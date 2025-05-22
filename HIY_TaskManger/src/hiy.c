
#include <sys/wait.h>
#include "hiy.h"
#include "parse.h"
#include "util.h"
#include "logging.h"

/* Constants */
#define DEBUG 0
#define STOP_SHELL  0
#define RUN_SHELL   1

static const char *task_path[] = {"./", "/usr/bin/", NULL};

#define NUM_PATHS (sizeof(task_path)/sizeof(const char*) - 1)

// Task structure
typedef struct task_st {
  unsigned int num; // task number
  pid_t pid; // process id
  unsigned short state; // task state
  int exit_code; // task exit code
  bool is_fg; // foreground flag
  char* cmd; // task command
  char** argv; // task command arguments
  struct task_st* next; // pointer to the next task
} Task;

Task* tasks = NULL; // initialize linked list for tasks
unsigned int task_id = 1; // set task id to 1 initially
unsigned int task_number = 0; // number of tasks
sigset_t mask, old_mask; // masks for signals
unsigned int task_to_promote = 0; // task number to promote to foreground

// Function is_busy - checks if a task is busy or not
// Input
//   unsigned short state - task state
// Output
//   true if busy, false if not
bool is_busy(unsigned short state) {
  return (state == LOG_STATE_RUN_FG) || (state == LOG_STATE_RUN_BG) || (state == LOG_STATE_SUSPENDED);
}

// Function free_task - frees allocated memory for a task
// Input
//   Task* task - pointer to a task to be freed
void free_task(Task* task) {
  free(task->cmd);
  free_argv(task->argv);
  free(task);
}

// Function free_tasks - frees allocated memory for the tasks list
void free_tasks() {
  Task* temp;
  // Free all tasks one by one until the head becomes NULL
  while (tasks != NULL) {
    temp = tasks;
    tasks = tasks->next;
    free_task(temp);
  }
}

// Function add_task - adds a task to the global linked list of tasks
// Input
//   char* cmd - task command
//   char** argv - task command arguments
void add_task(char* cmd, char** argv) {
  // Create a new task struct
  Task* new_task = (Task*)malloc(sizeof(Task));
  // If error, exit
  if (new_task == NULL) {
    exit(1);
  }
  // Otherwise, set all the fields
  new_task->num = task_id;
  new_task->pid = 0;
  new_task->state = STATE_READY;
  new_task->exit_code = 0;
  new_task->is_fg = false;
  new_task->cmd = string_copy(cmd);
  new_task->argv = clone_argv(argv);
  new_task->next = NULL;
  // And add it to the end of the list
  if (tasks == NULL) {
    tasks = new_task;
  }
  else {
    Task* temp = tasks;
    while (temp->next != NULL) {
      temp = temp->next;
    }
    temp->next = new_task;
  }
  // Log the event and update the counters
  log_hiy_task_init(task_id, cmd);
  task_id++;
  task_number++;
}

// Function delete_task - deletes a task from the global linked list of tasks
// Input
//   unsigned int num - task number
void delete_task(unsigned int num) {
  // If the list is empty, log task number error and return
  if (tasks == NULL) {
    log_hiy_task_num_error(num);
    return;
  }
  // If it matches the first element, check the state and remove it if possible
  if (tasks->num == num) {
    // If the task is busy, log status error and return
    if (is_busy(tasks->state)) {
      log_hiy_status_error(tasks->num, tasks->state);
      return;
    }
    // Otherwise, remove it, update the list head and task number,
    // and log the removal
    Task* temp = tasks;
    tasks = tasks->next;
    free_task(temp);
    log_hiy_delete(num);
    task_number--;
    return;
  }
  // If it was not the first, we need to iterate through the list
  Task* temp = tasks;
  while (temp->next != NULL) {
    // If the task number matches, check the state and remove it if possible
    if (temp->next->num == num) {
      // If the task is busy, log status error and return
      if (is_busy(temp->next->state)) {
        log_hiy_status_error(temp->next->num, temp->next->state);
        return;
      }
      // Otherwise, remove it, update the list pointers and task number,
      // and log the removal
      Task* to_delete = temp->next;
      temp->next = temp->next->next;
      free_task(to_delete);
      log_hiy_delete(num);
      task_number--;
      return;
    }
    // If the task number of the next task in the list is higher,
    // then we know for sure that it is not in the list because
    // the tasks are always sorted in ascending order by task number
    else if (temp->next->num > num) {
      log_hiy_task_num_error(num);
      return;
    }
    temp = temp->next;
  }
  // If not found in the list, report task number error and return
  log_hiy_task_num_error(num);
  return;
}

// Function get_task - searches for a task in the tasks list
// Input
//   unsigned int num - task number
// Output
//   pointer to the task if found, NULL if not
Task* get_task(unsigned int num) {
  Task* temp = tasks;
  // Iterate through the list and return the task if it matches the number
  while (temp != NULL) {
    if (temp->num == num) {
      return temp;
    }
    temp = temp->next;
  }
  return NULL;
}

// Function get_fg_task - searches for a foreground task in the tasks list
// Input
//   none
// Output
//   pointer to the foreground task if found, NULL if not
Task* get_fg_task() {
  Task* temp = tasks;
  // Iterate through the list and return the task if it matches the number
  while (temp != NULL) {
    if (temp->state == STATE_RUN_FG) {
      return temp;
    }
    temp = temp->next;
  }
  return NULL;
}

// Function get_task_by_pid - searches for a task in the tasks list
// Input
//   pid_t pid - process id of a task
// Output
//   pointer to the task if found, NULL if not
Task* get_task_by_pid(pid_t pid) {
  Task* temp = tasks;
  // Iterate through the list and return the task if it matches the number
  while (temp != NULL) {
    if (temp->pid == pid) {
      return temp;
    }
    temp = temp->next;
  }
  return NULL;
}

// Function log_task_list - logs the list of tasks
void log_task_list() {
  Task* temp = tasks;
  // Iterate through the linked list and log each task info
  while (temp != NULL) {
    log_hiy_task_info(temp->num, temp->cmd, temp->state, temp->pid, temp->exit_code);
    temp = temp->next;
  }
}

// Function task_input_redirect - redirects input for a task from the file
// Input
//   unsigned int num - task number
//   char* infile - name of the input file
void task_input_redirect(unsigned int num, char* infile) {
  // If the filename is valid, try to open the file and redirect the input
  if (infile != NULL) {
    int infd = open(infile, O_RDONLY);
    // If it cannot be opened, log an error and terminate the child
    if (infd == -1) {
      log_hiy_file_error(num, infile);
      exit(1);
    }
    // Otherwise, redirect the standard input and log the redirection
    dup2(infd, STDIN_FILENO);
    close(infd);
    log_hiy_redir(num, LOG_REDIR_IN, infile);
  }
}

// Function task_output_redirect - redirects output for a task to the file
// Input
//   unsigned int num - task number
//   char* outfile - name of the output file
void task_output_redirect(unsigned int num, char* outfile) {
  // If the filename is valid, try to open the file and redirect the output
  if (outfile != NULL) {
    // Create if it does not exist and overwrite if it does
    int outfd = open(outfile, O_WRONLY | O_CREAT | O_TRUNC, 0600);
    // If it cannot be opened, log an error and terminate the child
    if (outfd == -1) {
      log_hiy_file_error(num, outfile);
      exit(1);
    }
    // Otherwise, redirect the standard output and log the redirection
    dup2(outfd, STDOUT_FILENO);
    close(outfd);
    log_hiy_redir(num, LOG_REDIR_OUT, outfile);
  }
}

// Function task_execute - executes a task
// Input
//   Task* task - task to execute
void task_execute(Task* task) {
  int length;
  // Iterate through all the paths, append the program name
  // to each path and try to execute it
  for (int i = 0; i < NUM_PATHS; i++) {
    length = strlen(task_path[i]) + strlen(task->argv[0]) + 1;
    char path[length];
    strcpy(path, task_path[i]);
    strncat(path, task->argv[0], length - strlen(task_path[i]) - 1);
    execv(path, task->argv);
  }
  // If execv returned, it must be an error, report it and exit
  log_hiy_start_error(task->cmd);
  exit(1);
}

// Function start_task - starts a task from the tasks list
// Input
//   unsigned int num - task number
//   char* infile - input file for redirection
//   char* outfile - output file for redirection
//   unsigned short new_state - new task state (STATE_RUN_FG or STATE_RUN_BG)
//   int pipe_fds[] - pipe file descriptors, NULL if no piping required
void start_task(unsigned int num, char* infile, char* outfile, unsigned short new_state, int pipe_fds[]) {
  // First, try to find the task
  Task* task = get_task(num);
  // Log an error and return if not found
  if (task == NULL) {
    log_hiy_task_num_error(num);
    return;
  }
  // If the task is busy, log status error and return
  if (is_busy(task->state)) {
    log_hiy_status_error(task->num, task->state);
    return;
  }
  // Block the signals
  sigprocmask(SIG_BLOCK, &mask, &old_mask);
  // Else, everything is fine, fork a child process
  pid_t pid = fork();
  // If fork error, just exit
  if (pid < 0) {
    exit(1);
  }
  // Child process
  else if (pid == 0) {
    // Give the child a new process group id
    setpgid(0, 0);
    // Redirect the input, check if it should be connected to the pipe
    if (pipe_fds != NULL) {
      if (new_state == STATE_RUN_FG) {
        dup2(pipe_fds[0], STDIN_FILENO);
        close(pipe_fds[1]);
      }
    }
    else {
      task_input_redirect(num, infile);
    }
    // Redirect the output, check if it should be connected to the pipe
    if (pipe_fds != NULL) {
      if (new_state == STATE_RUN_BG) {
        dup2(pipe_fds[1], STDOUT_FILENO);
        close(pipe_fds[0]);
      }
    }
    else {
      task_output_redirect(num, outfile);
    }
    // Unblock the signals
    sigprocmask(SIG_SETMASK, &old_mask, NULL);
    // Execute the task command
    task_execute(task);
  }
  // Close the pipe ends if there are any open ones, and if this process is fg
  if ((pipe_fds != NULL) && (new_state == STATE_RUN_FG)) {
    close(pipe_fds[0]);
    close(pipe_fds[1]);
  }
  // In parent process, first log the status
  log_hiy_status(num, task->cmd, pid, task->state, new_state);
  // Set the pid, state and reset exit code if needed
  task->pid = pid;
  task->state = new_state;
  if (task->exit_code != 0) {
    task->exit_code = 0;
  }
  // Set the foreground flag appropriately
  task->is_fg = (new_state == STATE_RUN_FG);
  // Unblock the signals
  sigprocmask(SIG_SETMASK, &old_mask, NULL);
  // Wait for the process to end if it is foreground
  while (task->state == STATE_RUN_FG);
}

// Function pipe_tasks - starts two tasks, redirecting the output of the
//                       first to the input of the second task
// Input
//   unsigned int num1 - task 1 number
//   unsigned int num2 - task 2 number
void pipe_tasks(unsigned int num1, unsigned int num2) {
  Task* task1;
  Task* task2;
  // If the two task numbers are identical, log the error and return
  if (num1 == num2) {
    log_hiy_pipe_error(num1);
    return;
  }
  // Try to find the first task, check its state and log errors if needed
  task1 = get_task(num1);
  if (task1 == NULL) {
    log_hiy_task_num_error(num1);
    return;
  }
  if (is_busy(task1->state)) {
    log_hiy_status_error(task1->num, task1->state);
    return;
  }
  // Try to find the second task, check its state and log errors if needed
  task2 = get_task(num2);
  if (task2 == NULL) {
    log_hiy_task_num_error(num2);
    return;
  }
  if (is_busy(task2->state)) {
    log_hiy_status_error(task2->num, task2->state);
    return;
  }

  // If everything passed, try to create a pipe and log error if failed
  int pipe_fds[2];
  if (pipe(pipe_fds) == -1) {
    log_hiy_file_error(num1, LOG_FILE_PIPE);
    return;
  }

  // If passed, log the successful pipe creation
  log_hiy_pipe(num1, num2);

  // Start the first process in background
  start_task(num1, NULL, NULL, STATE_RUN_BG, pipe_fds);
  // Start the second process in foreground
  start_task(num2, NULL, NULL, STATE_RUN_FG, pipe_fds);
}

// Function run_fg - runs a task in the foreground
// Input
//   unsigned int num - task number
void run_fg(unsigned int num) {
  // First, try to find the task and report the error if not found
  Task* task = get_task(num);
  if (task == NULL) {
    log_hiy_task_num_error(num);
    return;
  }
  // If the task is idle (not busy), just call start function
  if (!is_busy(task->state)) {
    start_task(num, NULL, NULL, STATE_RUN_FG, NULL);
  }
  // Else, if the task is suspended, send a signal to wake it up
  else if (task->state == STATE_SUSPENDED) {
    // Set the foreground flag, so it resumes in the foreground
    task->is_fg = true;
    kill(task->pid, SIGCONT);
    log_hiy_sig_sent(LOG_CMD_RESUME, num, task->pid);
    // Wait for the process to finish (signal handler will update the flag)
    while (task->is_fg);
  }
  // Else, if the task is running in background, switch it to foreground
  else if (task->state == STATE_RUN_BG) {
    task->state = STATE_RUN_FG;
    task->is_fg = true;
    log_hiy_status(num, task->cmd, task->pid, STATE_RUN_BG, STATE_RUN_FG);
    // Wait for the process to finish (signal handler will update the flag)
    while (task->is_fg);
  }
  // Else, if the task is running in foreground, report error
  else if (task->state == STATE_RUN_FG) {
    log_hiy_status_error(num, STATE_RUN_FG);
  }
}

// Function run_bg - runs a task in the background
// Input
//   unsigned int num - task number
void run_bg(unsigned int num) {
  // First, try to find the task and report the error if not found
  Task* task = get_task(num);
  if (task == NULL) {
    log_hiy_task_num_error(num);
    return;
  }
  // If the task is idle (not busy), just call start function
  if (!is_busy(task->state)) {
    start_task(num, NULL, NULL, STATE_RUN_BG, NULL);
  }
  // Else, if the task is suspended, send a signal to wake it up
  else if (task->state == STATE_SUSPENDED) {
    // Reset the foreground flag
    task->is_fg = false;
    kill(task->pid, SIGCONT);
    log_hiy_sig_sent(LOG_CMD_RESUME, num, task->pid);
  }
  // Else, if the task is running in background, report error
  else if (task->state == STATE_RUN_BG) {
    log_hiy_status_error(num, STATE_RUN_BG);
  }
  // Else, if the task is running in foreground, report error
  else if (task->state == STATE_RUN_FG) {
    log_hiy_status_error(num, STATE_RUN_FG);
  }
}

// Function kill_task - kills a task by sending the signal SIGINT
// Input
//   unsigned int num - task number
void kill_task(unsigned int num) {
  // First, try to find the task and report the error if not found
  Task* task = get_task(num);
  if (task == NULL) {
    log_hiy_task_num_error(num);
    return;
  }
  // If the task is not running, report error
  if ((task->state != STATE_RUN_FG) && (task->state != STATE_RUN_BG)) {
    log_hiy_status_error(num, task->state);
    return;
  }
  // Else, send the signal and log the event
  kill(task->pid, SIGINT);
  log_hiy_sig_sent(LOG_CMD_KILL, num, task->pid);
}

// Function suspend_task - suspends a task by sending the signal SIGTSTP
// Input
//   unsigned int num - task number
void suspend_task(unsigned int num) {
  // First, try to find the task and report the error if not found
  Task* task = get_task(num);
  if (task == NULL) {
    log_hiy_task_num_error(num);
    return;
  }
  // If the task is idle (not busy), report error
  if (!is_busy(task->state)) {
    log_hiy_status_error(num, task->state);
    return;
  }
  // Else, send the signal and log the event
  kill(task->pid, SIGTSTP);
  log_hiy_sig_sent(LOG_CMD_SUSPEND, num, task->pid);
}

// Function handle_sigint - handles the SIGINT signal
// Input
//   int sig - signal code
void handle_sigint(int sig) {
  // Log the signal event
  log_hiy_ctrl_c();
  // Try to find the foreground task
  Task* fg_task = get_fg_task();
  // If found, pass the SIGINT signal to it
  if (fg_task != NULL) {
    kill(fg_task->pid, SIGINT);
    log_hiy_sig_sent(LOG_CMD_KILL, fg_task->num, fg_task->pid);
  }
}

// Function handle_sigtstp - handles the SIGTSTP signal
// Input
//   int sig - signal code
void handle_sigtstp(int sig) {
  // Log the signal event
  log_hiy_ctrl_z();
  // Try to find the foreground task
  Task* fg_task = get_fg_task();
  // If found, pass the SIGTSTP signal to it
  if (fg_task != NULL) {
    kill(fg_task->pid, SIGTSTP);
    log_hiy_sig_sent(LOG_CMD_SUSPEND, fg_task->num, fg_task->pid);
  }
}

// Function handle_sigquit - handles the SIGQUIT signal
// Input
//   int sig - signal code
void handle_sigquit(int sig) {
  // Log the signal event
  log_hiy_ctrl_bs();
  // Try to find the foreground task
  Task* fg_task = get_fg_task();
  // If no tasks, or only one fg task, just return
  if ((task_number == 0) || ((fg_task != NULL) && (task_number == 1))) {
    return;
  }
  // Otherwise if there is a foreground task, send SIGTSTP signal to it
  if (fg_task != NULL) {
    kill(fg_task->pid, SIGTSTP);
    log_hiy_sig_sent(LOG_CMD_SUSPEND, fg_task->num, fg_task->pid);
  }
  // Find the next task and run it in foreground
  Task* next_task = NULL;
  // If there is a foreground task, set it to the next task in the list
  if (fg_task != NULL) {
    next_task = fg_task->next;
  }
  // If it is still NULL, set it to the first task in the list
  if (next_task == NULL) {
    next_task = tasks;
  }
  // Set the task number to be promoted to foreground
  task_to_promote = next_task->num;
}

// Function handle_sigchld - handles the SIGCHLD signal
// Input
//   int sig - signal code
void handle_sigchld(int sig) {
  int status;
  pid_t pid;
  Task* task;

  // Reap the child processes using waitpid in a loop
  while (true) {
    pid = waitpid(-1, &status, WNOHANG | WUNTRACED | WCONTINUED);
    // Check the error code for interruption, and repeat the wait if needed
    if ((pid < 0) && (errno == EINTR)) {
      continue;
    }
    // If 0 or other error, break from the loop
    else if (pid <= 0) {
      break;
    }
    // If everything was fine, get the exit code and update state
    else {
      // First, find the task by its pid
      task = get_task_by_pid(pid);
      // If not NULL, check the status and update data
      if (task != NULL) {
        // If exited normally, get the status and exit code
        // and log the state change
        if (WIFEXITED(status)) {
          log_hiy_status(task->num, task->cmd, pid, task->state, STATE_FINISHED);
          task->exit_code = WEXITSTATUS(status);
          task->state = STATE_FINISHED;
          task->is_fg = false;
        }
        // If killed, just update the state and log the change
        else if (WIFSIGNALED(status)) {
          log_hiy_status(task->num, task->cmd, pid, task->state, STATE_KILLED);
          task->state = STATE_KILLED;
          task->is_fg = false;
        }
        // If suspended, just update the state and log the change
        else if (WIFSTOPPED(status)) {
          log_hiy_status(task->num, task->cmd, pid, task->state, STATE_SUSPENDED);
          task->state = STATE_SUSPENDED;
          task->is_fg = false;
        }
        // If continued, just update the state and log the change
        else if (WIFCONTINUED(status)) {
          // Check if it is supposed to continue in foreground or background
          if (task->is_fg) {
            log_hiy_status(task->num, task->cmd, pid, task->state, STATE_RUN_FG);
            task->state = STATE_RUN_FG;
          }
          else {
            log_hiy_status(task->num, task->cmd, pid, task->state, STATE_RUN_BG);
            task->state = STATE_RUN_BG;
          }
        }
      }
    }
  }
}

/*-------------------------------------------*/
/*  The entry of your task manager program   */
/*-------------------------------------------*/

int main() {
    char *cmd = NULL;
    int do_run_shell = RUN_SHELL;

    // Define sigaction structures
    struct sigaction sa_sigint;
    struct sigaction sa_sigtstp;
    struct sigaction sa_sigquit;
    struct sigaction sa_sigchld;

    // Initialize and connect the SIGINT signal handler
    memset(&sa_sigint, 0, sizeof(sa_sigint));
    sa_sigint.sa_handler = handle_sigint;
    sigaction(SIGINT, &sa_sigint, NULL);

    // Initialize and connect the SIGTSTP signal handler
    memset(&sa_sigtstp, 0, sizeof(sa_sigtstp));
    sa_sigtstp.sa_handler = handle_sigtstp;
    sigaction(SIGTSTP, &sa_sigtstp, NULL);

    // Initialize and connect the SIGQUIT signal handler
    memset(&sa_sigquit, 0, sizeof(sa_sigquit));
    sa_sigquit.sa_handler = handle_sigquit;
    sigaction(SIGQUIT, &sa_sigquit, NULL);

    // Initialize and connect the SIGCHLD signal handler
    memset(&sa_sigchld, 0, sizeof(sa_sigchld));
    sa_sigchld.sa_handler = handle_sigchld;
    sigaction(SIGCHLD, &sa_sigchld, NULL);

    // Initialize mask for blocking the signals
    sigaddset(&mask, SIGINT);
    sigaddset(&mask, SIGTSTP);
    sigaddset(&mask, SIGQUIT);
    sigaddset(&mask, SIGCHLD);

    /* Intial Prompt and Welcome */
    log_hiy_intro();
    log_hiy_help();

    /* Shell looping here to accept user command and execute */
    while (do_run_shell == RUN_SHELL) {
        char *argv[MAXARGS+1] = {0};        /* Argument list */
        Instruction inst = {0};           /* Instruction structure: check parse.h */

        // If there is a task to promote to foreground, do it now
        if (task_to_promote != 0) {
          run_fg(task_to_promote);
          task_to_promote = 0;
        }

        /* Print prompt */
        log_hiy_prompt();

        /* Get Input - Allocates memory for the cmd copy */
        cmd = get_input();
        /* If the input is whitespace/invalid, get new input from the user. */
        if(cmd == NULL) {
          continue;
        }

        /* Parse the Command and Populate the Instruction and Arguments */
        initialize_command(&inst, argv);    /* initialize arg lists and instruction */
        parse(cmd, &inst, argv);            /* call provided parse() */

        if (DEBUG) {  /* display parse result, redefine DEBUG to turn it off */
          debug_print_parse(cmd, &inst, argv, "Main, after parse");
        }

        /* After parsing: your code to continue from here */
        /*.===============================================.
         *| - The command has been parsed and you have cmd, inst, and argv filled with data
         *| - Very highly recommended to start calling your own functions after this point.
         *+===============================================*/


        /*==BUILT_IN: quit===*/
        if (strcmp(inst.instruct, "quit") == 0) {
          do_run_shell = STOP_SHELL;  /*set the main loop to exit when you finish processing it */
          log_hiy_quit();

        }
        /*==BUILT_IN: help===*/
        else if (strcmp(inst.instruct, "help") == 0) {
          log_hiy_help();
        }
        /*==BUILT_IN: list===*/
        else if (strcmp(inst.instruct, "list") == 0) {
          // Block the signals
          sigprocmask(SIG_BLOCK, &mask, &old_mask);
          // Log the number of tasks
          log_hiy_num_tasks(task_number);
          // Log the information for each task in the list
          log_task_list();
          // Unblock the signals
          sigprocmask(SIG_SETMASK, &old_mask, NULL);
        }
        /*==BUILT_IN: delete TASKNUM===*/
        else if (strcmp(inst.instruct, "delete") == 0) {
          // Block the signals
          sigprocmask(SIG_BLOCK, &mask, &old_mask);
          // Delete the task from the list
          delete_task(inst.num);
          // Unblock the signals
          sigprocmask(SIG_SETMASK, &old_mask, NULL);
        }
        /*==BUILT_IN: start TASKNUM [< INFILE] [> OUTFILE]===*/
        else if (strcmp(inst.instruct, "start") == 0) {
          start_task(inst.num, inst.infile, inst.outfile, STATE_RUN_FG, NULL);
        }
        /*==BUILT_IN: startbg TASKNUM [< INFILE] [> OUTFILE]===*/
        else if (strcmp(inst.instruct, "startbg") == 0) {
          start_task(inst.num, inst.infile, inst.outfile, STATE_RUN_BG, NULL);
        }
        /*==BUILT_IN: pipe TASKNUM1 TASKNUM2===*/
        else if (strcmp(inst.instruct, "pipe") == 0) {
          pipe_tasks(inst.num, inst.num2);
        }
        /*==BUILT_IN: fg TASKNUM===*/
        else if (strcmp(inst.instruct, "fg") == 0) {
          run_fg(inst.num);
        }
        /*==BUILT_IN: bg TASKNUM===*/
        else if (strcmp(inst.instruct, "bg") == 0) {
          run_bg(inst.num);
        }
        /*==BUILT_IN: kill TASKNUM===*/
        else if (strcmp(inst.instruct, "kill") == 0) {
          kill_task(inst.num);
        }
        /*==BUILT_IN: suspend TASKNUM===*/
        else if (strcmp(inst.instruct, "suspend") == 0) {
          suspend_task(inst.num);
        }
        /*==USER COMMAND===*/
        else {
          // Block the signals
          sigprocmask(SIG_BLOCK, &mask, &old_mask);
          // Add the new task to the list
          add_task(cmd, argv);
          // Unblock the signals
          sigprocmask(SIG_SETMASK, &old_mask, NULL);
        }

        /*.===============================================.
         *| After your code: We cleanup before Looping to the next command.
         *| free_command WILL free the cmd, inst and argv data.
         *| Make sure you've COPIED ANY INFORMATION YOU NEED first.
         *| Hint: You can use the util.c functions for copying this information.
         *+===============================================*/

        free_command(cmd, &inst, argv);
        cmd = NULL;
    }  // end while

    // Free the tasks list before exiting
    free_tasks();
    return 0;
}  // end main()

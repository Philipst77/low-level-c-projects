#include "listnode.h"
#include "exec.h"
#include <string.h>

#define MAX_LINE_SIZE 10000
#define MAX_ARGS 127 // for execute
#define MAX_CMDS 100

// struct which holds the command line input data
typedef struct CommandLine{
  char* command;
  char** args;
  int args_count;
}CommandLine;

// struct which holds the recent commands and their arguments
typedef struct HistoryList{
  int head;
  int count;
  CommandLine commands[MAX_CMDS];
}HistoryList;

// Function prototypes:
char* get_input(char*);
int parse(char*, CommandLine*);
void copy_command_line(CommandLine*, CommandLine*);
void free_command_line(CommandLine*);
void add_to_history(CommandLine*, HistoryList*);
void print_history_list(HistoryList*);
void clear_history_list(HistoryList*);
void clear_list_node(ListNode*);
void clean_up(HistoryList*, ListNode*);
void quit(CommandLine*, HistoryList*, ListNode*);
void cd(CommandLine*);
void history(CommandLine*, HistoryList*, ListNode**);
void new(CommandLine*, ListNode**);
void list(CommandLine*, ListNode*);
void open(CommandLine*, ListNode*);
void process_commands(CommandLine*, HistoryList*, ListNode**);
void execute(CommandLine*, ListNode*);

// Function get_input
// Description: prints the prompt and gets the command line input
// Input:
//   char* buffer - buffer which will be filled with the command line input
// Output:
//   return value of fgets, which is NULL if EOF
char* get_input(char* buffer) {
  fprintf(stdout, "262$");
  fflush(stdout);
  return fgets(buffer, MAX_LINE_SIZE, stdin);
}

// Function parse
// Description: parses command line input into command and arguments, and
//              fills the CommandLine struct with parsed data
// Input:
//   char* buffer - buffer which contains the command line input
//   CommandLine* cl - CommandLine struct which will be filled
// Output:
//   1 if success (not a blank line)
//   0 if no success (blank line)
int parse(char* buffer, CommandLine* cl) {
  int count = 0;

  // copy the buffer and count the number of tokens
  char* buffer_copy = strdup(buffer);
  char* token = strtok(buffer_copy, " \t\n");
  while (token != NULL) {
    token = strtok(NULL, " \t\n");
    count++;
  }
  free(buffer_copy);

  // if count is 0, it was a blank line, return 0
  if (count == 0) {
    return 0;
  }

  // now parse the original buffer
  token = strtok(buffer, " \t\n");
  // store the first token as a command
  cl->command = token;

  // set the argument count and allocate the data
  cl->args_count = count-1;
  cl->args = (char**) malloc(cl->args_count * sizeof(char*));

  // save the rest of the tokens as arguments
  token = strtok(NULL, " \t\n");
  count = 0;
  while (token != NULL) {
    cl->args[count] = token;
    count++;
    token = strtok(NULL, " \t\n");
  }

  return 1;
}

// Function copy_command_line
// Description: makes a deep copy of a CommandLine struct
// Input:
//   CommandLine* cl_src - CommandLine struct source
//   CommandLine* cl_dest - CommandLine struct destination
// Output:
//   none
void copy_command_line(CommandLine* cl_src, CommandLine* cl_dest) {
  // copy the command
  cl_dest->command = strdup(cl_src->command);
  // copy the arguments
  cl_dest->args = (char**) malloc(cl_src->args_count * sizeof(char*));
  for (int i = 0; i < cl_src->args_count; i++) {
    cl_dest->args[i] = strdup(cl_src->args[i]);
  }
  cl_dest->args_count = cl_src->args_count;
}

// Function free_command_line
// Description: frees the command line string and arguments strings
// Input:
//   CommandLine* cl - CommandLine struct to free
// Output:
//   none
void free_command_line(CommandLine* cl) {
  // free the command
  free(cl->command);
  // free the arguments
  for (int i = 0; i < cl->args_count; i++) {
    free(cl->args[i]);
  }
  free(cl->args);
  cl->args_count = 0;
}

// Function add_to_history
// Description: adds a command and its arguments to history list
// Input:
//   CommandLine* cl - CommandLine struct which contains input command and args
//                     of the most recent command which is not "history"
//   HistoryList* hl - HistoryList struct which will be updated with the
//                     most recent command
// Output:
//   none
void add_to_history(CommandLine* cl, HistoryList* hl) {
  // if the list is full, delete the oldest command
  if (hl->count == MAX_CMDS) {
    free_command_line(&hl->commands[hl->head]);
  }

  // add the most recent command to the list
  copy_command_line(cl, &hl->commands[hl->head]);
  hl->head = (hl->head + 1) % MAX_CMDS;

  // if the list is not full, increment the count
  if (hl->count < MAX_CMDS) {
    hl->count++;
  }
}

// Function print_history_list
// Description: prints all the recent commands from the history list
// Input:
//   HistoryList* hl - HistoryList struct which contains the list of recent
//                     commands and their arguments
// Output:
//   none
void print_history_list(HistoryList* hl) {
  int index = (hl->head - hl->count + MAX_CMDS) % MAX_CMDS;
  for (int i = 0; i < hl->count; i++) {
    fprintf(stdout, "%d: %s", i, hl->commands[index].command);
    for (int j = 0; j < hl->commands[index].args_count; j++) {
      fprintf(stdout, " %s", hl->commands[index].args[j]);
    }
    fprintf(stdout, "\n");
    index = (index + 1) % MAX_CMDS;
  }
}

// Function clear_history_list
// Description: frees all the allocated memory for the history list
// Input:
//   HistoryList* hl - HistoryList struct which contains the list of recent
//                     commands and their arguments
// Output:
//   none
void clear_history_list(HistoryList* hl) {
  for (int i = 0; i < hl->count; i++) {
    free_command_line(&hl->commands[i]);
  }
  hl->count = 0;
  hl->head = 0;
}

// Function clear_list_node
// Description: frees all the allocated memory for a list node
// Input:
//   ListNode* ln - ListNode struct which is to be cleared
// Output:
//   none
void clear_list_node(ListNode* ln) {
  free(ln->command);
  for (int i = 0; i < ln->arguments_length; i++) {
    free(ln->arguments[i]);
  }
  free(ln->arguments);
  if (ln->file_contents != NULL) {
    free(ln->file_contents);
  }
}

// Function clean_up
// Description: frees all the allocated memory
// Input:
//   HistoryList* hl - HistoryList struct which contains the list of recent
//                     commands and their arguments
//   ListNode* lhd - Head of the ListNode linked list of commands
// Output:
//   none
void clean_up(HistoryList* hl, ListNode* lhd) {
  // clean the history list
  clear_history_list(hl);
  // clean the linked list of ListNodes
  ListNode* temp;
  while (lhd != NULL) {
    temp = lhd;
    lhd = lhd->next;
    clear_list_node(temp);
    free(temp);
  }
}

// Function quit
// Description: performs "quit" built-in command
// Input:
//   CommandLine* cl - CommandLine struct which contains input command and args
//   HistoryList* hl - HistoryList struct which contains the list of recent
//   ListNode* lhd - Head of the ListNode linked list of commands
// Output:
//   none
void quit(CommandLine* cl, HistoryList* hl, ListNode* lhd) {
  if (cl->args_count != 0) {
    fprintf(stderr, "error: %s\n", "too many arguments");
    return;
  }

  // perform the clean up and exit
  clean_up(hl, lhd);
  free(cl->args);
  exit(0);
}

// Function cd
// Description: performs "cd" built-in command
// Input:
//   CommandLine* cl - CommandLine struct which contains input command and args
// Output:
//   none
void cd(CommandLine* cl) {
  if (cl->args_count == 1) {
    if (chdir(cl->args[0]) == -1) {
      fprintf(stderr, "error: %s\n", strerror(errno));
    }
  }
  else if (cl->args_count > 1) {
    fprintf(stderr, "error: too many arguments provided\n");
  }
}

// Function history
// Description: performs "history" built-in command
// Input:
//   CommandLine* cl - CommandLine struct which contains input command and args
//   HistoryList* hl - HistoryList struct which contains the list of recent
//                     commands and their arguments
//   ListNode** lhd - Head of the ListNode linked list of commands
// Output:
//   none
void history(CommandLine* cl, HistoryList* hl, ListNode** lhd) {
  // if no arguments, print the history list
  if (cl->args_count == 0) {
    print_history_list(hl);
  }
  // else, if one argument, see which one it is
  else if (cl->args_count == 1) {
    // if the argument is "-c", clear the list
    if (strcmp(cl->args[0], "-c") == 0) {
      clear_history_list(hl);
    }
    // else, it must be "history [index]"
    else {
      int index = atoi(cl->args[0]);
      if (index < 0 || index >= hl->count) {
        fprintf(stderr, "error: %s\n", "Index in history list does not exist");
        return;
      }
      // translate the index to real index value in history list
      int start = (hl->head - hl->count + MAX_CMDS) % MAX_CMDS;
      index = (start + index) % MAX_CMDS;
      // re-execute the command
      CommandLine new_cl;
      copy_command_line(&hl->commands[index], &new_cl);
      process_commands(&new_cl, hl, lhd);
      free_command_line(&new_cl);
    }
  }
  // else, if more arguments, it is too many
  else {
    fprintf(stderr, "error: %s\n", "too many arguments");
  }
}

// Function new
// Description: performs "new" built-in command
// Input:
//   CommandLine* cl - CommandLine struct which contains input command and args
//   ListNode** lhd - Head of the ListNode linked list of commands
// Output:
//   none
void new(CommandLine* cl, ListNode** lhd) {
  if (cl->args_count == 0) {
    fprintf(stderr, "error: too few arguments provided\n");
    return;
  }
  // allocate a new node
  ListNode* new_node = (ListNode*) malloc(sizeof(ListNode));
  new_node->command = strdup(cl->args[0]);
  new_node->next = NULL;
  // allocate space for the arguments, including NULL
  new_node->arguments = (char**) malloc((cl->args_count + 1) * sizeof(char*));
  // copy the arguments
  for (int i = 0; i < cl->args_count; i++) {
    new_node->arguments[i] = strdup(cl->args[i]);
  }
  new_node->arguments[cl->args_count] = NULL;
  // set arguments length
  new_node->arguments_length = cl->args_count;
  new_node->file_contents = NULL;
  // if the list is empty, set it to head
  if (*lhd == NULL) {
    new_node->id = 0;
    *lhd = new_node;
  }
  // otherwise, find the last node and append it to the end
  else {
    ListNode* temp = *lhd;
    while (temp->next != NULL) {
      temp = temp->next;
    }
    new_node->id = temp->id + 1;
    temp->next = new_node;
  }
}

// Function list
// Description: performs "list" built-in command
// Input:
//   CommandLine* cl - CommandLine struct which contains input command and args
//   ListNode* lhd - Head of the ListNode linked list of commands
// Output:
//   none
void list(CommandLine* cl, ListNode* lhd) {
  if (cl->args_count != 0) {
    fprintf(stderr, "error: %s\n", "too many arguments");
    return;
  }
  // iterate through the whole list and print the info
  ListNode* current = lhd;
  while (current != NULL) {
    // print ListNode id and command
    fprintf(stdout, "List Node %d\n", current->id);
    fprintf(stdout, "\tCommand: %s\n", current->command);
    // print file contents
    fprintf(stdout, "\tFile Contents:\n");
    if (current->file_contents != NULL) {
      // copy the file contents to a buffer, and then split it with strtok
      char *file_copy = strdup(current->file_contents);
      char *line = strtok(file_copy, "\n");

      while (line != NULL) {
        fprintf(stdout, "\t\t%s\n", line);
        line = strtok(NULL, "\n");
      }

      free(file_copy);
    }
    current = current->next;
  }
}

// Function open
// Description: performs "open" built-in command
// Input:
//   CommandLine* cl - CommandLine struct which contains input command and args
//   ListNode* lhd - Head of the ListNode linked list of commands
// Output:
//   none
void open(CommandLine* cl, ListNode* lhd) {
  if (cl->args_count != 2) {
    fprintf(stderr, "error: incorrect number of arguments\n");
    return;
  }

  int id = atoi(cl->args[0]);

  // try to find the node with this id
  ListNode* current = lhd;
  while (current != NULL && current->id != id) {
    current = current->next;
  }

  // if not found, print an error and return
  if (current == NULL) {
    fprintf(stderr, "error: %s\n", "Id does not exist");
    return;
  }

  // otherwise, try to open the file and copy its contents to the node
  FILE *file = fopen(cl->args[1], "r");
  if (file == NULL) {
    fprintf(stderr, "error: file cannot be opened\n");
    return;
  }

  // seek to the end of file in order to find out its size
  fseek(file, 0, SEEK_END);
  long file_size = ftell(file);

  // rewind it to the beginning
  rewind(file);

  // allocate a buffer to store the file contents, including terminating NULL
  char* buffer = (char*) malloc(file_size + 1);

  // read the contents and close the file
  fread(buffer, 1, file_size, file);
  buffer[file_size] = '\0';
  fclose(file);

  // set the list node contents to the buffer, free if not empty
  if (current->file_contents != NULL) {
    free(current->file_contents);
  }
  current->file_contents = buffer;
}

// Function execute
// Description: performs "execute" built-in command
// Input:
//   CommandLine* cl - CommandLine struct which contains input command and args
//   ListNode* lhd - Head of the ListNode linked list of commands
// Output:
//   none
void execute(CommandLine* cl, ListNode* lhd) {
  if (cl->args_count != 1) {
    fprintf(stderr, "error: incorrect number of arguments\n");
    return;
  }

  int id = atoi(cl->args[0]);

  // try to find the node with this id
  ListNode* current = lhd;
  while (current != NULL && current->id != id) {
    current = current->next;
  }

  // if not found, print an error and return
  if (current == NULL) {
    fprintf(stderr, "error: %s\n", "Id does not exist");
    return;
  }

  // if there are too many arguments, print an error and return
  if (current->arguments_length > MAX_ARGS) {
    fprintf(stderr, "error: %s\n", "too many arguments");
    return;
  }

  // run the command and get the exit status
  int status = run_command(current);
  int exit_status = (status >> 8) & 0xFF;

  if (exit_status != 0) {
    fprintf(stderr, "error: %s\n", strerror(exit_status));
  }
}

// Function process_commands
// Description: processes and executes the built-in commands, updates history
//              list with the most recent command
// Input:
//   CommandLine* cl - CommandLine struct which contains input command and args
//   HistoryList* hl - HistoryList struct which contains the list of recent
//   ListNode** lhd - Head of the ListNode linked list of commands
// Output:
//   none
void process_commands(CommandLine* cl, HistoryList* hl, ListNode** lhd) {
  // if the command is not "history", process it and add it to history list
  if (strcmp(cl->command, "history") != 0) {
    // process "quit" built-in command (7.1)
    if (strcmp(cl->command, "quit") == 0) {
      quit(cl, hl, *lhd);
    }
    // process "cd" built-in command (7.2)
    else if (strcmp(cl->command, "cd") == 0) {
      cd(cl);
    }
    // process "new" built-in command (7.4)
    else if (strcmp(cl->command, "new") == 0) {
      new(cl, lhd);
    }
    // process "list" built-in command (7.5)
    else if (strcmp(cl->command, "list") == 0) {
      list(cl, *lhd);
    }
    // process "open" built-in command (7.6)
    else if (strcmp(cl->command, "open") == 0) {
      open(cl, *lhd);
    }
    // process "execute" built-in command (7.7)
    else if (strcmp(cl->command, "execute") == 0) {
      execute(cl, *lhd);
    }
    add_to_history(cl, hl);
  }
  // else, process "history" built-in command (7.3)
  else {
    history(cl, hl, lhd);
  }
}

int main() {
  char buffer[MAX_LINE_SIZE];
  char* input;
  CommandLine cline;
  HistoryList hlist;
  ListNode* lhead = NULL;
  hlist.count = 0;
  hlist.head = 0;

  // the main loop which takes the command line input
  input = get_input(buffer);
  while (input) {
    // parse, and if not blank line, process the commands
    if (parse(buffer, &cline) != 0) {
      process_commands(&cline, &hlist, &lhead);
      // free the command line args array
      free(cline.args);
    }
    // get the command line input again
    input = get_input(buffer);
  }

  // clean up the memory
  clean_up(&hlist, lhead);

  return 0;
}


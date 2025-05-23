/* DO NOT EDIT THIS FILE

 *   Collection of common supporting functions for the StrawHat VM
 */

/* Standard Library Includes */
#include <stdio.h>
#include <stdlib.h>
/* Linux System API Includes */
#include <signal.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/time.h>
/* StrawHat Function Includes */
#include "strawhat.h"
#include "strawhat_cs.h"
#include "strawhat_settings.h"
#include "strawhat_shell.h"
#include "strawhat_printing.h"
#include "strawhat_support.h"

static int get_flag(int pos, unsigned short val) {
//  __asm__ ("movb %0, %%cl" : : "r"(flag));
//  __asm__ ("shrw %%cl, %1; andw $1, %1" : "=r"(state): "r"(state));
  return (val>>pos)&1;
}

static int is_critical(Scheduler_process_s *node) {
  return(node && get_flag(0xC,node->state));
}
static int is_ready(Scheduler_process_s *node) {
  return(node && get_flag(0xF,node->state));
}
static int is_running(Scheduler_process_s *node) {
  return(node && get_flag(0xE,node->state));
}
static int is_high(Scheduler_process_s *node) {
    return(node && get_flag(0xB,node->state));
}
// static int is_suspended(Scheduler_process_s *node) {
//   return(node && get_flag(0xC,node->state));
// }
static int is_terminated(Scheduler_process_s *node) {
  return(node && get_flag(0xD,node->state));
}

/* Quickly registers a new signal with the given signal number and handler
 * Exits the program on errors with signal registration.
 */
void register_signal(int sig, void (*handler)(int)) {
  if(handler == NULL) {
    ABORT_ERROR("Handler Function Needed for Registration\n");
  }

  // Register the handler with the OS
  struct sigaction sa = {0};
  sa.sa_handler = handler;
  sigaction(sig, &sa, NULL);
}

/* Print the Virtual System Prompt
 * PROMPT configurable in inc/strawhat_settings.h
 */
void print_prompt() {
  printf("%s(PID: %d)%s %s%s%s ", BLUE, getpid(), RST, GREEN, PROMPT, RST);
  fflush(stdout);
}

/* Special Error that also immediately exits the program. */
void abort_error(char *msg, char *src) {
  fprintf(stderr, "  %s[ERROR ] %s", RED, msg);
  fprintf(stderr, "  %s         (File %s)%s\n", RED, src, RST);
  fprintf(stderr, "  %s         Terminating Program%s\n", RED, RST);
  exit(EXIT_FAILURE);
}

/* Prints out the Opening Banner */
void print_strawHat_banner() {
  printf(
" %sGreen Imperial HEx:%s %sStrawHat Task Manager v1.5a%s %s*TRIAL EXPIRED*%s\n%s"
"               [o]                     \n"
"     .__________|__________.           \n"
"     |%s        _.--._       %s|			  \n"
"     |%s   _.-.'      `.-.   %s|			  \n"
"     |%s .' .%s/`--...--' \\%s `. %s|  \n"
"     |%s `.'.%s`--.._..--'%s  .' %s|	  \n"
"     |%s   ' -.._______..'   %s|	      \n"
"     |%s                     %s|       \n"
"     |%s %s[StrawHat] $%s _      |     \n"
"     ._____________________.           \n"
"               [  ]                    \n"
"       ____________________            \n"
"    _-'.-.-.-.-. .-.-.-.-. `-_    \n" // Keyboard and Hat Derived from Dan Strychalski's Work
"  _'.-.-.--------------.-.-.-.`-_ \n" // Original Keyboard at https://www.asciiart.eu/computers/keyboards
" :-------------------------------:%s\n", // Original Hat at https://www.asciiart.eu/clothing-and-accessories/hats
  GREEN, RST, BLUE, RST, RED, RST, WHITE,
  YELLOW, WHITE,
  YELLOW, WHITE,
  YELLOW, RED, YELLOW, WHITE,
  YELLOW, RED, YELLOW, WHITE,
  YELLOW, WHITE,
  YELLOW, WHITE,
  YELLOW, GREEN, WHITE,
  RST);
}

/* Prints the full Schedule of all processes being tracked. */
void print_scheduler_debug(Scheduler_s *schedule, Scheduler_process_s *on_cpu) {
  // If on_cpu is NULL, it won't be printed, but otherwise is legal
  
  // Only print if debug mode is enabled.
  if(g_debug_mode == 1) {
    print_schedule(schedule, on_cpu);
  }
}

/* Prints the full Schedule of all processes being tracked.
 * If on_cpu is NULL, simply won't print that anything is on CPU, but is legal */
void print_schedule(Scheduler_s *schedule, Scheduler_process_s *on_cpu) {
  // If no such schedule exists, print nothing.
  if(schedule == NULL) {
    return;
  }

  int queue_counts[3] = {0};

  // Collect the number of processes in StrawHat
  #if FEATURE_MLFQ
  queue_counts[0] = sched.scheduler_get_count(schedule->ready_queue_high);
  queue_counts[1] = sched.scheduler_get_count(schedule->ready_queue_normal);
  queue_counts[2] = sched.scheduler_get_count(schedule->terminated_queue);
  #else
  queue_counts[0] = sched.scheduler_get_count(schedule->ready_queue);
  queue_counts[1] = sched.scheduler_get_count(schedule->suspended_queue);
  queue_counts[2] = sched.scheduler_get_count(schedule->terminated_queue);
  #endif

  if(queue_counts[0] == -1 || queue_counts[1] == -1 || queue_counts[2] == -1) {
    ABORT_ERROR("The Scheduler Count function returned an Error Condition.\n");
  }

  int total_scheduled_processes = queue_counts[0] + queue_counts[1] + queue_counts[2];
  PRINT_STATUS("Printing the current Status...\n");
  PRINT_STATUS("Running Process (Note: Processes run briefly, so this is usually empty.)\n");

  // CPU - Add the On CPU Process to the count
  int count = 0;
  if(on_cpu != NULL) {
    count = 1;
  }

  // Now print all the information
  PRINT_STATUS("...[CPU Execution    - %2d Process%s]\n", count, count?"":"es");
  print_process_node(on_cpu);
  
  // Schedule
  PRINT_STATUS("Schedule - %d Processes across all Queues\n", total_scheduled_processes);
#if FEATURE_MLFQ
  // Ready Queue - High Pri
  count = queue_counts[0];
  if(count == -1) {
    ABORT_ERROR("The Scheduler Count function returned an Error Condition.\n");
  }
  PRINT_STATUS("...[Ready Queue - High   - %2d Process%s]\n", count, count==1?"":"es");
  print_scheduler_queue(schedule->ready_queue_high);
  // Ready Queue - Normal
  count = queue_counts[1];
  if(count == -1) {
    ABORT_ERROR("The Scheduler Count function returned an Error Condition.\n");
  }
  PRINT_STATUS("...[Ready Queue - Normal - %2d Process%s]\n", count, count==1?"":"es");
  print_scheduler_queue(schedule->ready_queue_normal);
#else  
  // Ready Queue - High Pri
  count = queue_counts[0];
  if(count == -1) {
    ABORT_ERROR("The Scheduler Ready Queue Count function returned an Error Condition.\n");
  }
  PRINT_STATUS("...[Ready Queue             - %2d Process%s]\n", count, count==1?"":"es");
  print_scheduler_queue(schedule->ready_queue);
  // Ready Queue - Normal
  count = queue_counts[1];
  if(count == -1) {
    ABORT_ERROR("The Scheduler Suspended Count function returned an Error Condition.\n");
  }
  PRINT_STATUS("...[Suspended Queue         - %2d Process%s]\n", count, count==1?"":"es");
  print_scheduler_queue(schedule->suspended_queue);
#endif

  // Defunct Queue
  count = sched.scheduler_get_count(schedule->terminated_queue);
  if(count == -1) {
    ABORT_ERROR("The Scheduler Terminated Count function returned an Error Condition.\n");
  }
  PRINT_STATUS("...[Terminated Queue     - %2d Process%s]\n", count, count==1?"":"es");
  print_scheduler_queue(schedule->terminated_queue);
}

/* Prints a single Scheduler Queue */
void print_scheduler_queue(Scheduler_queue_s *queue) {
  // If no such queue exists, print nothing.
  if(queue == NULL) {
    return;
  }

  // Iterate the queue and print each process
  Scheduler_process_s *walker = queue->head;
  while(walker != NULL) {
    print_process_node(walker);
    walker = walker->next;
  }
}

// Prints a schedule tracked process
void print_process_node(Scheduler_process_s *node) {
  // If no process exists, print nothing.
  if(node == NULL) {
    return;
  }
  // If Process has Terminated
  if(is_terminated(node)) {
    PRINT_STATUS("     [PID: %7d] %-26s ... Flags: [%c%c%c%c%c], Age: %2d, Exit Code: %d\n",
        node->pid, node->cmd,  
        is_ready(node)?       'R':' ',
        is_running(node)?     'U':' ',
        is_terminated(node)?  'T':' ',
        is_critical(node)?    'C':' ',
        is_high(node)?        'H':' ',
        node->age,
        sched.scheduler_get_ec(node));
  }
  // If Process has not Terminated Yet
  else {
    PRINT_STATUS("     [PID: %7d] %-26s ... Flags: [%c%c%c%c%c], Age: %2d\n",
        node->pid, node->cmd,  
        is_ready(node)?       'R':' ',
        is_running(node)?     'U':' ',
        is_terminated(node)?  'T':' ',
        is_critical(node)?    'C':' ',
        is_high(node)?        'H':' ',
        node->age);
  }
}


/* wlf CPU Scheduling Library
                     .
                    / V\
                  / `  /
                 <<   |
                 /    |
               /      |
             /        |
           /    \  \ /
          (      ) | |
  ________|   _/_  | |
<__________\______)\__)
*/
 
/* Standard Library Includes */
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <string.h>
/* Unix System Includes */
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/time.h>
#include <pthread.h>
#include <sched.h>
/* DO NOT CHANGE THE FOLLOWING INCLUDES - Local Includes 
 * If you change these, it will not build on Zeus with the Makefile
 * If you change these, it will not run in the grader
 */
#include "wlf_sched.h"
#include "strawhat_scheduler.h"
#include "strawhat_support.h"
#include "strawhat_process.h"
/* DO NOT CHANGE ABOVE INCLUDES - Local Includes */

/* Feel free to create any definitions or constants you like! */

/* Feel free to create any helper functions you like! */

/*** Wlf Library API Functions to Complete ***/

/* Initializes the Wlf_schedule_s Struct and all of the Wlf_queue_s Structs
 * Follow the project documentation for this function.
 * Returns a pointer to the new Wlf_schedule_s or NULL on any error.
 * - Hint: What does malloc return on an error?
 */
Wlf_schedule_s *wlf_initialize( ) {    
  Wlf_schedule_s *schedule =(Wlf_schedule_s*)malloc(sizeof(Wlf_schedule_s)); // allocating memory for schedule struct
  if(schedule == NULL){  // check if allocating went wrong meaning null
    printf("Error"); // print error message if so
    exit(1);  // exit out 
  }
  Wlf_queue_s *ready_queue_high =(Wlf_queue_s*)malloc(sizeof(Wlf_queue_s));   // allocating memory for ready high priority queue
  Wlf_queue_s  *ready_queue_normal =(Wlf_queue_s*)malloc(sizeof(Wlf_queue_s)); // allocating memory for ready normal priority queue
  Wlf_queue_s  *terminated_queue =(Wlf_queue_s*)malloc(sizeof(Wlf_queue_s));  // allocating memory for terminated queue

  
  if(ready_queue_high== NULL){   // check if allocating went wrong for ready queue high priority
    printf("Error "); // print error message if so 
    free(schedule);  // free schedule sturct first becasue we free in reverse order
    exit(1); // exit
  }
  if(ready_queue_normal== NULL){ // check if allocating went wrong for ready queue normal priority
    printf("Error"); // print error message if so
    free(schedule);  // free schedule struct becasue we free in reverse order
    free(ready_queue_high);  // free ready queue high priority 
    exit(1);  // exit 
  }
  if(terminated_queue == NULL){  // check if allocating went wrong for the terminated queue
    printf("Error");  // print error if so 
    free(schedule);  // free schedule struct
    free(ready_queue_high); // free ready queue high priorty beause we free in reverse order
    free(ready_queue_normal);  // free ready queue normal priority
    exit(1); //exit 
  }
  ready_queue_high ->head = NULL;  // setting head node of ready queue high priority  to null
  ready_queue_normal ->head = NULL;  // setting head node of ready queue normal priority to null
  terminated_queue ->head = NULL;  // setting head node of terminated queue to null

  ready_queue_high ->count= 0;  // setting count of ready queue high priority to 0
  ready_queue_normal ->count =0; // setting count of ready queue normal priority to 0
  terminated_queue -> count = 0; // seting count of terminated queue to 0

   schedule->ready_queue_high = ready_queue_high;  // linking ready queue high priority to schedule struct
    schedule->ready_queue_normal = ready_queue_normal; // linking ready queue normal priority to schedule sturct
    schedule->terminated_queue = terminated_queue; // linking terminated queue to schedule sturct 
  
  return schedule; // return struct instance 
}

/* Allocate and Initialize a new Wlf_process_s with the given information.
 * - Malloc and copy the command string, don't just assign it!
 * Follow the project documentation for this function.
 * - You may assume all arguments within data are Legal and Correct for this Function Only
 * Returns a pointer to the Wlf_process_s on success or a NULL on any error.
 */
Wlf_process_s *wlf_create(Wlf_create_data_s *data) {

  Wlf_process_s *process =(Wlf_process_s*)malloc(sizeof(Wlf_process_s)); // allocate memory for process 
  if(process == NULL){ // checking if allocation went wrong 
    printf("Error"); // print error if so 
    exit(1); // exit
  }
  process->state |=(1<<15);   // setting ready state bit to 1
  process->state  &= ~((1<<14)|(1<<13)); // setting bit 14 and 15 to on but when negating them with and to have them off so we only have ready bit set to 1 
  unsigned short critical = (1<<12);  // creating mask to turn on critical priority bit
  unsigned short high = (1 <<11);  // creating  mask to turn on high priority bit
  if(data->is_critical == 1){ // checking if current data critical priority bit is turned on 
    process->state |= critical;  // setting mask for turning on critical priority bit  with | bit wise operation
    process->state |= high; //setting mask for turning on high priority bit with | bit wise operation
  } else{  // if other condition then then the if statement 
    process->state &= ~critical;  // get process current state and apply with & bitwise operation to the negation of the mask for critical bit mask
  }
  if(data->is_high == 1){ // checking if condition for if high prority bit is on 
    process->state|= high; // setting high prority bit to on with bitwise | operation 
  } else{ // condition block to execute if other then the if condition above
    process->state &=  ~high; // applying the negation mask of high priority bitmask with & bitwise operaiton 
  }

  process->state &= 0xFF00; // settigng exit code bits to 0

  process->age =0;  //setting age of process to 0
  process->pid = data->pid; // setting pid to data pid of process
  process->cmd = malloc(strlen(data->original_cmd)+1);  // allocating memeory for string length of command entered +1 for the null terminator to be accounted 
  
   if (data->original_cmd == NULL) { // checking if data original cmd is null
        printf("Error \n");  // printing error message
        free(process); // freeing process
        process = NULL;  // setings process to null to avoid dangiling pointers
        exit(1);
    }

  if(process->cmd == NULL){  // checks if command entered of process is null
    printf("Error "); // prints error if so 
    free(process);  // frees process
    process = NULL;  // setings process to null to avoid dangiling pointers
    exit(1); //exit
  }
  strcpy(process->cmd, data->original_cmd);  // using strcpy to copy data original command into process command 
  process->next = NULL;  // setting process next to null because not linked to anything yet 

  return process; // return process
}

/* Inserts a process into the appropriate Ready Queue (singly linked list).
 * Follow the project documentation for this function.
 * - Do not create a new process to insert, insert the SAME process passed in.
 * Returns a 0 on success or a -1 on any error.
 */
int wlf_enqueue(Wlf_schedule_s *schedule, Wlf_process_s *process) {
    if (schedule == NULL || process == NULL) {  // checking if schedule sturct or process sturct are null
        printf("Error"); // print error if so
        return -1;  // return -1 becaue of error of null sturct or process
    }
  process->state |=(1<<15);   // setting ready state bit to 1
  process->state  &= ~((1<<14)|(1<<13)); // setting running and terminated bit to 1 using | operator but negating it so we set them to 0 we and that whole mask with state mask to have only running bit on 
    // Check for Critical priority High priority or Normal priority
    if (process->state & (1 << 12)) {  // checking if process state when bitwise aded with the bit mask has bit critical bit on
        Wlf_process_s *current = schedule->ready_queue_high->head;  // setting current as a holder for the head of the ready high priority queue to traverse linked list
        if (current == NULL) { // checking if current is NULL meaning that allcoating and setting went wrong some where
            schedule->ready_queue_high->head = process; // setting head of priority queue to process
            process->next = NULL;  // setting process next pointer to null
        } else { // else block of code to execute if condition of first if statment above is not met
            while (current->next != NULL) {   // while loop to execute code with in while statment until current next is not NULL 
                current = current->next; // iterating list
            }
            current->next = process;  // setting current next to process
            process->next = NULL; // setting process next to NULL
        }
        schedule->ready_queue_high->count++;  // incrementing the count of ready queue high count 
    } else if (process->state & (1 << 11)) {  // else if statment checking if process state applied with bitwise & of bit mask to access 11 bit high priority bit  is met
        Wlf_process_s *current = schedule->ready_queue_high->head; // then well do this which is setting current to be the pointer to head of ready queue high priority
        if (current == NULL) { // checking if allocating of current and setting is NULL
            schedule->ready_queue_high->head = process; // setting head to process
            process->next = NULL;  // setting next to NULL
        } else { // else statement block to execute if above if statment req was hit
            while (current->next != NULL) {  // while loop to keep executing code with in while condition if current ->next not equal to null
                current = current->next; // iteration through linked list
            }
            current->next = process;  // setting current next to process
            process->next = NULL; // setging process next to null
        }
        schedule->ready_queue_high->count++;  // incrementing ready queue high count 
    } else {  // else statment to execute else block
        Wlf_process_s *current = schedule->ready_queue_normal->head;  // setting current to head of ready queue normal 
        if (current == NULL) {  // checking if head is null
            schedule->ready_queue_normal->head = process;  //if so we just setting head to be process
            process->next = NULL; // setting process next to null
        } else {  // else statement
            while (current->next != NULL) {  // condition of while loop the code inside of while statment will keep executing until gaurd condition is violated
                current = current->next; // iterating through list
            }
            current->next = process; // setting current next to process
            process->next = NULL;  // setting process next to null
        }
        schedule->ready_queue_normal->count++;  // incrementing count of ready queue normal 
    }

    return 0;  // return
}


/* Returns the number of items in a given Wlf Queue (singly linked list).
 * Follow the project documentation for this function.
 * Returns the number of processes in the list or -1 on any errors.
 */
int wlf_count(Wlf_queue_s *queue) {  // this method really simple just return the queue coutn
 
    if(queue == NULL){   // checking if queue is NULL
      return -1;  // returns -1 if so means nothing in the queue
    } 
  return queue->count; // returns count of queue
}

/* Selects the best process to run from the Ready Queues (singly linked list).
 * Follow the project documentation for this function.
 * Returns a pointer to the process selected or NULL if none available or on any errors.
 * - Do not create a new process to return, return a pointer to the SAME process selected.
 * - Return NULL if the ready queues were both empty OR if there were any errors.
 */
Wlf_process_s *wlf_select(Wlf_schedule_s *schedule) {  
    if (schedule == NULL) {  // checking if schedule struct is null
        printf("Error");  // print error if so 
        return NULL;  // return NULL becuase of error
    }

    if (schedule->ready_queue_high->head == NULL && schedule->ready_queue_normal->head == NULL) {  // if statement that checks if ready queue high priority is null
    // also checkis if ready queue normal priority is null
        return NULL;  // return null because there null
    }

    Wlf_process_s *current = schedule->ready_queue_high->head;  // setting current to head of ready queue high priority 
    Wlf_process_s *prev = NULL;  // setting prev to null

    while (current != NULL && !(current->state & (1 << 13))) {   // while loop that executes code with in while loop as long as current not equal to null and state not bitmasked with termiated bit on
        prev = current;   // setting prev to current
        current = current->next;  // setting current to current 
        // so were iterating
    }

    if (current != NULL) {  // if statment that execute if current not equal to null
        if (prev == NULL) { // another if nested with in top on to check if prev equal to null
            schedule->ready_queue_high->head = current->next;  // setting head of ready queue high to current next
        } else {  //else statment to execute if first if statment doesnt hit
            prev->next = current->next;  // setting prev next to current next
            // basically removing the head by setting the new head to next node and setting prev next to next 
        }
      schedule->ready_queue_high->count--;  // decremeting ready queue high count
       current->age = 0;  // setting current age to 0
    current->state = (current->state & ~((1 << 15) | (1 << 13))) | (1 << 14); // setting only the runnign state bit to be running and having 
    // other 2 flags off because we are only allowed one at a time to be on and & to make sure that we dont effect the critical or high bits
        current->next = NULL; // setting current next to null
        return current;  // return current 
    }

    if (schedule->ready_queue_high->head != NULL) {  // checking if head of ready queue high is equal to null
        current = schedule->ready_queue_high->head;  // setting current head of ready queue high
       schedule->ready_queue_high->head = current->next;  // setting ready queue high head to current next
      schedule->ready_queue_high->count--;  // decrementing ready queue high count 
      current->age = 0; // setting current age to 0
        current->state = (current->state & ~((1 << 15) | (1 << 13))) | (1 << 14);  // setting only the runnign state bit to be running and having 
    // other 2 flags off because we are only allowed one at a time to be on and & to make sure that we dont effect the critical or high bits
        current->next = NULL;  // setting current next to null
        return current; // return current
    }
    Wlf_process_s *current2 = schedule->ready_queue_normal->head;  // setting current2 to head of ready queue normal head 
    if (current2 != NULL) {  // if statment checking if current2 is null if not null we excute code below
      schedule->ready_queue_normal->head = current2->next;  // setting head of ready queue normal to current2 next
      schedule->ready_queue_normal->count--;  // decrementing count of ready queue normal 
      current2->age = 0;  // setting current age to 0
       current2->state = (current2->state & ~((1 << 15) | (1 << 13))) | (1 << 14); // setting only the runnign state bit to be running and having 
    // other 2 flags off because we are only allowed one at a time to be on and & to make sure that we dont effect the critical or high bits
        current2->next = NULL;  // setting current2 next to null
        return current2; // returning current 2 
    }

    return NULL; // return null
}


/* Ages all Process nodes in the Ready Queue - Normal and Promotes any that are Starving.
 * If the Ready Queue - Normal is empty, return 0.  (Success if nothing to do)
 * Follow the specification for this function.
 * Returns a 0 on success or a -1 on any error.
 */
int wlf_promote(Wlf_schedule_s *schedule) {
    if (schedule == NULL) {  // checking if schedule struct is null
        printf("Error");  // print error if so 
        return -1; // return -1 for error 
    }
    Wlf_process_s *current = schedule->ready_queue_normal->head;  // setting current to head of ready queue normal 
    Wlf_process_s *prev = NULL; // setting prev to null
    int promoted_count = 0; // setting promoted count we keeps track of promotions to 0 because we need to initalize it to base value of 0 
    // no promotions yet

    while (current != NULL) { // while loop execute code inside while loop condition until current == NULL
        current->age++; // incrementing current age

        if (current->age >= STARVING_AGE) {  // if statement checking current age greater then or euqal to starving age macro so we know if nodes age are greater then starving age
            if (prev == NULL) { // nested if statement checking if prev == NULL
                schedule->ready_queue_normal->head = current->next; // setting head of ready queue normal to current next basically setting new head to remove old one so we can add 
                //it to high priority queue so it can get a chance to run because its age is to old it needs run its been waiting for to long
            } else { // else statment block to execute if top if statement not hit 
                prev->next = current->next;  // setting prev next to current next 
                // what ever node whos age has big age getting removed to get added to other queue so it gets a chance to run
            }
            schedule->ready_queue_normal->count--;  // decrementing count of ready queue normal

            Wlf_process_s *high_current = schedule->ready_queue_high->head; // setting high current pointer to head of ready queue high 
            if (high_current == NULL) {  // checking if high current allocating failed 
                schedule->ready_queue_high->head = current; // setting head of ready queue high to current node which is from other queue
                current->next = NULL;  // setting current next to null basically inserting the node
            } else { // else statement block to be hit if other conditions above are not hit
                while (high_current->next != NULL) {  // while loop to execute inside code as long as while loop gaurd condition valid 
                    high_current = high_current->next;  // iterating list
                }
                high_current->next = current;  // setting current next to current node which is from other qeue that had big age
                current->next = NULL;  // setting curent next to null inserting it 
            }
            schedule->ready_queue_high->count++;  // incrementing count of ready queue high

            promoted_count++; //incrementing promotion count 
          if (prev == NULL) {  // if statement checking if prev is null
           current = schedule->ready_queue_normal->head;  // setting current to ready queue normal head
          } else {  // else block
             current = prev->next;  // setting current to prev next
            }

        } else {  // else statement block
            prev = current;  // setting current to prev
            current = current->next;  // setting current to current next
        }
    }
    return promoted_count; // returns promoted count 
}


/* This is called when a process exits normally that was just Running.
 * Put the given node into the Terminated Queue and set the Exit Code 
 * - Do not create a new process to insert, insert the SAME process passed in.
 * Follow the project documentation for this function.
 * Returns a 0 on success or a -1 on any error.
 */
int wlf_exited(Wlf_schedule_s *schedule, Wlf_process_s *process, int exit_code) {
  if(schedule == NULL || process == NULL){  // if statement checks if scheudle struct or processs are NULL
    printf("Error"); // prints error if so 
    return -1; // returns -1 because of error
  }
  unsigned short mask1 = 1<<15 | 1 << 14; // bit mask to set ready and runnign state bit 
  unsigned short mask2 = 1<<13;  // bit mask to set terminated bit
  process ->state &= ~mask1; // applying bit and opertion with negation of mask 1 to turn bit 15 and 14 off
  process ->state |= mask2; // applying bit or with mask 2 to set terminated bit to on

     unsigned short mask3 = ~((1 << 8) - 1);   // 3rd mask with bit operation this to set exit code to 0
    process->state = (process->state & mask3) | (exit_code & ~mask3);  // setting state to state & bit wise operated with mask 2 and bit wise | with exit code & negated mask 3
    // this sets all bits 0-7 the exit code bits to 0 so we can then or it with the exit code to have the exit code on the bottom 8 of the byte
  
  if(schedule->terminated_queue->head ==NULL){  // if statement that checks if terminated queue head is null
    schedule->terminated_queue->head =process; // setting terminated queues head to process
    process->next = NULL;  // setting process next to null
  } else { // else block statement
    Wlf_process_s *current = schedule->terminated_queue->head;  // setting current to head of terminated queue
    while(current->next != NULL){ // while loop to iterate through terminated queue till this condition is not true anymore
      current = current->next;  // iterationg
    }
    current->next = process; // setting current next to process   bascially inserting the process node
    process->next = NULL; // setting process next to NULL
  }
    schedule->terminated_queue->count++;  // incrementing terminated queue count 
  return 0;  // return 0 

}




/* This is called when the OS terminates a process early. 
 * - This will either be in your Ready Queue - High or Ready Queue - Normal.
 * - The difference with wlf_exited is that this process is in one of your Queues already.
 * Remove the process with matching pid from either Ready Queue and add the Exit Code to it.
 * - You have to check both since it could be in either queue.
 * Follow the project documentation for this function.
 * Returns a 0 on success or a -1 on any error.
 */
int wlf_killed(Wlf_schedule_s *schedule, pid_t pid, int exit_code) {
    if (schedule == NULL || (schedule->ready_queue_high->head == NULL && schedule->ready_queue_normal->head == NULL)) {// checks if structs and struct head for high and normal priority are null
        printf("Error"); // prints error if null
        return -1;  // return -1 for error
    }


    Wlf_process_s *current = schedule->ready_queue_high->head;  // seting current to ready queue high head
    Wlf_process_s *prev = NULL;  // setting prev to null

    while (current != NULL && current->pid != pid) {  // while loop setting prev to current and curernt to curent next as long as current not equal to null and its pid isnt = to pid 
        prev = current;  // setting prev to current
        current = current->next; // setting current to current next
        // basically iterating
    }

    // If found in ready queue high
    if (current != NULL) {   // if statement checking if current != null
        if (prev == NULL) {  // nested if statement if prev equal to null
            schedule->ready_queue_high->head = current->next;  // if the head node the node we want were just moving the head of the ready 
            // queue high linke list to have a new head posistion 
        } else {  // else statement
            prev->next = current->next;  // setting prev next to current next skipping the old head node 
        }
        schedule->ready_queue_high->count--;  // decrementing count of ready queue high 
    } else {  // else 
        // Search ready queue normal
        current = schedule->ready_queue_normal->head;  // setting current to ready queue normal head 
        prev = NULL;  // setting prev to null

        while (current != NULL && current->pid != pid) {  // while condition for iteration 
            prev = current;  // prev to current           basically setting are node in 
            current = current->next; // current to current next
        }

        // If found in ready_queue_normal
        if (current != NULL) {  // if statement checking if current not equal to null
            if (prev == NULL) {  // if statment nested an if prev equal to null
                schedule->ready_queue_normal->head = current->next;  // setting head of ready queue normal to current next
            } else {  // else 
                prev->next = current->next;  // setting prev  next to current next 
                // so where skipping the node we found to remove it 
            }
            schedule->ready_queue_normal->count--;  // decremting ready queue normal count
        }
    }

    // if process not in either queue
    if (current == NULL) {  // if statemetn chekcing validity of current
        printf("Error:");  // if null print error
        return -1;  // return -1 for error
    }

    unsigned short mask1 = (1 << 15) | (1 << 14);  // mask to turn on ready and runnign bit on
    unsigned short mask2 = 1 << 13;  // setting mask for termianted bit activation
    unsigned short mask3 = ~((1 << 8) - 1);  // setting mask for exit code

    current->state &= ~mask1;  // using bit wise and with mask negated to turn off bit 15 and 14 running and ready
    current->state |= mask2;  // using bit wise or with mask terminated
    current->state = (current->state & mask3) | (exit_code & ~mask3);  // using bit mask operation to set exit code
//to the lower 8 bits and clearing them to 8 before we do that adn then adding the exit code in
    // Add to terminated_queue
    if (schedule->terminated_queue->head == NULL) {  // if statement checking head of terminated queue is null
        schedule->terminated_queue->head = current;  // setting head of terminated queue head to current
        current->next = NULL; // settin current next to null  bascially inserting are node in
    } else { // else
        Wlf_process_s *term_current = schedule->terminated_queue->head;  // setting term curent which is current node of terminated queue
        while (term_current->next != NULL) {  // while loop that iterations through current queue 
            term_current = term_current->next;  // setting term current to current next the iteration keep going till while loop condition is off
        }
        term_current->next = current;  // seting current next to current
        current->next = NULL;  // setting current next to NULL
    }
    schedule->terminated_queue->count++;  // updating count of terminated queue 

    return 0;  // return 
}


/* This is called when StrawHat reaps a Terminated (Defunct) Process.  (reap command).
 * Remove and free the process with matching pid from the Termainated Queue.
 * Follow the specification for this function.
 * Returns the exit_code on success or a -1 on any error (such as process not found).
 */
int wlf_reap(Wlf_schedule_s *schedule, pid_t pid) {
  if (schedule == NULL || schedule->terminated_queue->head == NULL) {  // if statement chekcing if struct is null and if hea dof termiate queue is null
      printf("Error ");// print error if so
      return -1;  // return -1 for erro 
  }

  Wlf_process_s *current = schedule->terminated_queue->head;  // setting current to head of termianted queue
  Wlf_process_s *prev = NULL;  // setting prev to null

  if (pid == 0) {  // if statement checking if pid ==0 
      Wlf_process_s *remove = schedule->terminated_queue->head; // setting remove to head of temrinate queue 
      if (remove == NULL) {  // if statement checking if remove is null
          printf("Error"); // if so print error
          return -1; // return -1 for error 
      }
      schedule->terminated_queue->head = remove->next; // setting terminated queue head to remove next
      schedule->terminated_queue->count--;  // decrementing terminated queue count 
      int exit_code = remove->state & ((1 << 8) - 1);  // setting exti code to remove state bit wise anded with this bit mask which only preserves teh first 7 bits
      // which is the error code 
      free(remove->cmd); // free remove cmd
      free(remove);  // free remove
      return exit_code;  // returns the exit code
  }

  while (current != NULL && current->pid != pid) {  // while loop that iterates as long asn current no null and current pid not pid as long as one of those not true
      prev = current;  // we will do execute the code in the while loopsetting prev to current 
      current = current->next; // setting current to prev next
  }

  if (current == NULL) {  // if pid not found meaning current is null
      printf("Error");  // we print error if so 
      return -1;  // return -1 for error 
  }

  if (prev == NULL) {  // if prev is null
  
      schedule->terminated_queue->head = current->next;  // setting terminatd queue head to current next remove the head basically
  } else { // else block
      prev->next = current->next; // setting prev next to current next skipping the current block effectivly removing it 
  }

  schedule->terminated_queue->count--;  // decrementing terminated queue count

  int exit_code = current->state & ((1 << 8) - 1);  // setting exit code to teh cucrrent state bit wise anded with the bit shift mask 
  // the mask bit wise with curent state only keep the exit code and we assing that to the exit code var
  free(current->cmd);  // freeing current cmd
  free(current); // freeing current

  return exit_code;  // returning exit code
}


/* Gets the exit code from a terminated process.
 * (All Linux exit codes are between 0 and 255)
 * Follow the project documentation for this function.
 * Returns the exit code if the process is terminated.
 * If the process is not terminated, return -1.
 */
int wlf_get_ec(Wlf_process_s *process) {
  if(process == NULL){  // if statement checking if process is null
    printf("Error");  // printing error if so 
    return -1;  // returning -1
  }
  if(! (process->state & (1<<13))){   // if the process state bitwise anded with the bit mask is not the state then we return -1
      return -1;
  }
  int exit_code = process->state & ((1<<8)-1);  // setting exit code to the process state bit wise adned with the bit mask whick toghther preserves only the bits 0-7
  //which are the exit code
  return exit_code; // reutrns the exit code
}

/* Frees all allocated memory in the Wlf_schedule_s, all of the Queues, and all of their Nodes.
 * Follow the project documentation for this function.
 * Returns void.
 */
void wlf_cleanup(Wlf_schedule_s *schedule) {


 if (schedule == NULL) {  // checking if schdule struct is null if so return null 
        return; 
    }
if(schedule->ready_queue_high != NULL){   // if statement checking if ready queue high not null
    Wlf_process_s *currRdyQueHigh = schedule->ready_queue_high->head; // setting currdyhigh to hea do ready queue high
    while (currRdyQueHigh != NULL) {  // while loop that execute while currRdyQueHigh not null
        Wlf_process_s *next = currRdyQueHigh->next; // setting net to currRdyquehighnext
        free(currRdyQueHigh->cmd);   // freeing curRdyQueHigh->cmd             
        free(currRdyQueHigh);          // freeing currRdyQeuHigh            
        currRdyQueHigh = next;         // settingn currRdyQueHigh to next            
    }
    free(schedule->ready_queue_high); // freeing ready queue high 
    schedule->ready_queue_high = NULL;
    //freeing in reverse
}
if(schedule->ready_queue_normal != NULL){ // if statement checking if ready queue normal is not null 
   Wlf_process_s *currRdyQueNormal = schedule->ready_queue_normal->head; // setting currRdyQueNomral to ready queue normal head 
    while (currRdyQueNormal != NULL) {  // while loop that executes while currRdyNomral not null
        Wlf_process_s *next = currRdyQueNormal->next;  // setting next to currdyquenormal next
        free(currRdyQueNormal->cmd);   // free currRdyQueNomral->cmd               
        free(currRdyQueNormal);       // free currRdyQueNomral              
        currRdyQueNormal = next;      // setting currRdyNomral to next               
    }
    free(schedule->ready_queue_normal);  // freeing ready queue normal struct
    schedule->ready_queue_normal = NULL;
    // freeing in reverse 
}
if(schedule->terminated_queue != NULL){ // checking if terminated queue is null
    Wlf_process_s *currTerQueue = schedule->terminated_queue->head;  // setting currTerQueue to head of terminatd queue
    while (currTerQueue != NULL) {  //  while loop that checks if currTerQueue is not null
        Wlf_process_s *next = currTerQueue->next; // setting next to currTerQueue next 
        free(currTerQueue->cmd);   // freeing currTerQueue->cmd              
        free(currTerQueue);            // freeing currTerQueue    
        currTerQueue = next;         // settig currTerqueue to next     
    }
    free(schedule->terminated_queue); // freeing terminated queue
    schedule->terminated_queue = NULL;
    //freeing in reverse
}

    free(schedule); // free schedule struct 

}

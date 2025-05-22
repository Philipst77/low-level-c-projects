# Wlf CPU Scheduler

A low-level C implementation of a multi-level feedback queue CPU scheduler used by the StrawHat task manager. This project demonstrates in-depth understanding of linked lists, bitwise operations, and systems programming. The code focuses on maintaining three singly linked lists to manage processes: High Priority Queue, Normal Priority Queue, and Terminated Queue.

---

## 📊 Project Overview

The Wlf Scheduler interacts with the StrawHat shell, which manages Linux processes using custom CPU scheduling strategies. The scheduler decides which process should run next based on a priority system (critical, high, or normal) and prevents starvation using an aging mechanism.

### This implementation includes:
- Process node creation and management  
- Singly linked list manipulation (insert, remove, search)  
- Multi-level feedback queue scheduling  
- Bitwise manipulation for state management  
- Memory management and cleanup  

---

## 🧠 Core Concepts

- **Linked Lists**: Used to represent process queues  
- **Bitwise Operators**: Used to encode and decode process state flags and exit codes  
- **Process Scheduling**: Implements aging, starvation prevention, and critical-first execution  
- **System-level C Programming**: Dynamic memory, pointer management, and process control  

---

## 🔧 Key Functions Implemented

- `wlf_initialize()` - Initializes the scheduler and all queues  
- `wlf_create()` - Creates and initializes a new process node  
- `wlf_enqueue()` - Adds a process to the appropriate ready queue  
- `wlf_count()` - Counts the number of processes in a queue  
- `wlf_select()` - Selects and removes the best process for execution  
- `wlf_promote()` - Increments process age and promotes starving processes  
- `wlf_exited()` - Moves a completed process to the terminated queue  
- `wlf_killed()` - Kills and moves a ready process to the terminated queue  
- `wlf_reap()` - Reaps (frees) a terminated process and returns its exit code  
- `wlf_get_ec()` - Retrieves a process's exit code from its state  
- `wlf_cleanup()` - Frees all dynamically allocated memory for the scheduler  

---

## 📁 Project Structure
```
project-root/
├── src/ # Source files
│ └── wlf_sched.c # Your implementation goes here
├── inc/ # Header files
│ └── wlf_sched.h # Struct and macro definitions
├── obj/ # Precompiled object files
├── Makefile # Build configuration
├── strawhat # Provided shell interface
└── test_wlf_sched.c # Optional: For local testing
```

---

## ⚙️ Build & Run

To compile the project 

```bash
Launch the task manager shell:
./strawhat

📄 Process Management Notes
Process state is stored in a 16-bit unsigned short:

Bits 13–15: R (Ready), U (Running), T (Terminated)

Bit 12: Critical

Bit 11: High Priority

Bits 0–7: Exit Code

Special behaviors:
Critical processes always run first and exclusively

Aging: Processes in the normal queue increase in age after each cycle

Promotion: Processes that reach STARVING_AGE are promoted to the high-priority queue

🚀 Highlights
Emphasizes systems-level C programming

Teaches practical use of memory, structs, and data structures

Simulates real-world operating system scheduling behavior





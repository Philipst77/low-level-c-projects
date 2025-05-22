# HIY Task Manager

## Overview

The **HIY Task Manager** is a fully interactive, Unix-style task management shell written in C. It allows users to launch, manage, and control multiple concurrent tasks (processes) in both foreground and background modes. This system simulates core features of traditional Unix shells such as Bash or Zsh.

HIY provides hands-on experience with low-level systems programming concepts including process forking, inter-process communication (pipes), I/O redirection, and signal handling.

## Features

- 🔁 **Interactive Shell Interface** – Accepts and processes one command per line with real-time user interaction.
- 🧠 **Task Lifecycle Management** – Supports task creation, execution, suspension, resumption, termination, and deletion.
- ⚙️ **Foreground & Background Execution** – Run tasks synchronously (`start`) or asynchronously (`startbg`).
- 🔗 **Process Piping** – Connect the output of one task directly into the input of another using Unix pipes (`pipe`).
- 📁 **I/O Redirection** – Redirect input and output streams from/to files using `< infile` and `> outfile`.
- 🧩 **Signal Handling** – Responds to `SIGINT`, `SIGTSTP`, `SIGCHLD`, and `SIGQUIT` for robust process control.
- 🧾 **Structured Logging** – All output is generated through logging functions to maintain consistent feedback.

## Supported Built-in Commands

| Command                                   | Description |
|-------------------------------------------|-------------|
| `help`                                    | Displays usage guide and available commands. |
| `quit`                                    | Exits the HIY shell cleanly. |
| `list`                                    | Lists all current tasks with PID, state, and exit codes. |
| `delete <TASK>`                           | Deletes a task that is idle (Ready, Finished, or Killed). |
| `start <TASK> [< infile] [> outfile]`     | Executes a task in the foreground. |
| `startbg <TASK> [< infile] [> outfile]`   | Executes a task in the background. |
| `pipe <TASK1> <TASK2>`                    | Pipes output from TASK1 into TASK2. |
| `fg <TASK>`                               | Moves a task to the foreground. |
| `bg <TASK>`                               | Moves a task to the background. |
| `kill <TASK>`                             | Sends `SIGINT` to terminate a running task. |
| `suspend <TASK>`                          | Sends `SIGTSTP` to suspend a running task. |

### Keyboard Controls

- `Ctrl-C`: Sends `SIGINT` to the current foreground process.
- `Ctrl-Z`: Sends `SIGTSTP` to suspend the current foreground process.
- `Ctrl-\`: Suspends the current foreground process and promotes the next task in the list.

## File Structure

```
p3_handout/
├── Makefile # Build system
├── fox.txt # Sample text file for input redirection
├── inc/ # Header files
│ ├── hiy.h
│ ├── logging.h
│ ├── parse.h
│ └── util.h
├── src/ # Source files
│ ├── hiy.c # Core shell logic (edit this file only)
│ ├── logging.c # Predefined logging utilities
│ ├── parse.c # Input parsing functions
│ ├── util.c # Helper functions for string duplication and cleanup
│ ├── my_echo.c # Utility to test exit statuses
│ ├── my_pause.c # Utility to test signal handling
│ └── slow_cooker.c # Utility to simulate long-running tasks
```

## How to Build & Run

### 🛠️ Compile the Project

```bash
make
This compiles all source files and produces the hiy executable.

🚀 Launch the HIY Task Manager
./hiy

Once launched, you'll see:

HIY$
From here, you can begin entering supported commands like help, list, start, or custom executable names.

🧹 Clean Up Build Files
make clean

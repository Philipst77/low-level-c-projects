

## Overview

This project implements a simplified Unix-like task manager shell in C, named **HIY Task Manager**. It allows users to create, manage, and control multiple tasks (processes), both in foreground and background. The system supports redirection, piping, and signal handling using Unix process management principles.

## Features

- Command-line interface with built-in command support.
- Ability to create, start, suspend, kill, delete, or resume tasks.
- Foreground (`start`) and background (`startbg`) task execution.
- Piping output of one task to another (`pipe`).
- I/O redirection from/to files (`< infile`, `> outfile`).
- Signal handling (SIGINT, SIGTSTP, SIGCHLD, SIGQUIT).
- Logging with provided functions to standardize output.

## Supported Built-in Commands

| Command       | Description |
|---------------|-------------|
| `help`        | Displays usage and list of available commands. |
| `quit`        | Exits the HIY shell. |
| `list`        | Lists all current tasks and their statuses. |
| `delete <TASK>` | Deletes an idle task. |
| `start <TASK> [< infile] [> outfile]` | Starts a task in the foreground. |
| `startbg <TASK> [< infile] [> outfile]` | Starts a task in the background. |
| `pipe <TASK1> <TASK2>` | Pipes output of TASK1 into TASK2. |
| `fg <TASK>`   | Moves a task to the foreground (starts or resumes it). |
| `bg <TASK>`   | Moves a task to the background (starts or resumes it). |
| `kill <TASK>` | Sends a SIGINT to a running task. |
| `suspend <TASK>` | Sends a SIGTSTP to suspend a task. |

### Keyboard Controls

- `Ctrl-C`: Sends SIGINT to foreground process.
- `Ctrl-Z`: Sends SIGTSTP to foreground process.
- `Ctrl-\`: Suspends the foreground process and promotes the next task.

## File Structure

```
p3_handout/
├── Makefile
├── fox.txt
├── inc/
│ ├── hiy.h
│ ├── logging.h
│ ├── parse.h
│ └── util.h
├── src/
│ ├── hiy.c <-- Edit this file only
│ ├── logging.c
│ ├── parse.c
│ ├── util.c
│ ├── my_echo.c
│ ├── my_pause.c
│ └── slow_cooker.c
```
How to Run
Launch the HIY Task Manager:
./hiy

Development Guidelines
All implementation must be in src/hiy.c.

Use only provided headers and libraries.

Use logging functions like log_hiy_prompt(), log_hiy_status(), etc., instead of printf.

Carefully manage process state transitions.

Handle child processes and signals correctly.

Use helper utilities in util.c and parse.c.

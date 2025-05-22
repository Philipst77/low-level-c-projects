# Node Manager Shell

A custom-built C shell program that mimics basic Unix shell behavior while managing user-defined command nodes. This project demonstrates proficiency in low-level programming, dynamic memory management, and linked data structures.

---

## 📌 Project Overview

This shell application supports built-in commands and manages a custom linked list of commands (`ListNodes`). Each node can be associated with a command, arguments, and file contents. The shell handles command history, file operations, memory safety, and process execution.

### Key Concepts Demonstrated:
- Linked lists and dynamic memory allocation  
- Command parsing and execution  
- Bitwise operations  
- Multi-file modular C programming  
- File handling and directory navigation  
- Defensive programming with error handling and memory checks  

---

## 🧠 Features

### Shell Capabilities
- `quit` – Exit the shell and deallocate all memory  
- `cd [dir]` – Change working directory (with error handling)  
- `history` – Display up to 100 past commands  
  - `history -c` – Clear history  
  - `history [index]` – Re-run command at index  
- `new CMD [args]` – Create a new `ListNode` with a command and arguments  
- `list` – Print all `ListNodes` and their associated file contents  
- `open [id] [filename]` – Load file contents into a specified node  
- `execute [id]` – Run the command stored in the specified node  

---

## 📁 Project Structure
```
project-root/
├── src/ # Source files
│ └── nodemngr.c
├── inc/ # Header files
│ ├── listnode.h
│ └── exec.h
├── obj/ # Precompiled object files
│ └── exec.o
├── inputs/ # Sample input files
│ └── data/
├── outputs/ # Expected outputs for testing
├── Makefile # Compilation script
├── driver.py # Automated test runner
├── typescript # Terminal log for verification
├── joestar.txt # Example input file
├── kakyoin.txt # Example input file
└── avdol.txt # Example input file

```
---

## ⚙️ Build Instructions

To build the program:
```bash
make clean
make
To run the test suite:
python3 driver.py
✅ Compiles cleanly without warnings
✅ No memory leaks (verified with Valgrind)
✅ Modular design using multiple header/source files

🧪 Example Usage
262$ new cat file.txt
262$ open 0 file.txt
262$ execute 0
262$ history
262$ quit

🛡️ Error Handling
All built-in commands include thorough error handling using strerror(errno)
and clear custom messages for invalid input, file errors, or improper usage.

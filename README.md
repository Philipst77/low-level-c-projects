# 🔬 MotifFinder: A Basic DNA Motif Search Tool in C

MotifFinder is a command-line C program that allows users to analyze DNA sequences and identify common motifs with allowed mismatches. It implements foundational algorithms using dynamic memory allocation, string handling, and low-level array operations.

This tool is ideal for students or researchers learning bioinformatics concepts such as Hamming distance, substring generation, and motif comparison in DNA sequences.

---

## 📌 Features

- User input validation for:
  - Number of DNA sequences (`n`)
  - Length of sequences (`l`)
  - Motif length (`m`)
  - Allowed mismatches (`h`)
- Dynamic generation of all possible motif candidates (e.g., 4^m)
- Substring extraction from DNA sequences
- Hamming distance-based motif matching with tolerance for mismatches
- Displays:
  - All candidate motifs
  - All valid substrings per sequence
  - Matching motifs for each sequence
  - Motifs common to all DNA sequences
- Defensive programming with memory management and error handling

---

## 🧠 Concepts Covered

- Dynamic memory allocation in C  
- 2D array manipulation and pointer arithmetic  
- String parsing and validation  
- Hamming distance algorithm  
- Modular function design  
- Bioinformatics basics (motif discovery)

---

## 🧪 Sample Execution

```bash
Basic Motif Search Program  
Please enter the number of input strings for motif search (n): 3  
Please enter the length of each input string (l): 10  
Please enter the length of motifs (m): 4  
Please enter the number of allowable mismatches (h): 1  
Please enter input string #1: ACGTAGCTAG  
Please enter input string #2: GCTAGTCAGT  
Please enter input string #3: TAGCTAGGAC  

🛠️ Compilation & Run Instructions
To compile the program:

gcc -o motif_finder motif_finder.c -lm

To run the program:

./motif_finder

📂 Project Files

motif_finder.c     # Main program source code
README.md          # Project documentation


🧹 Memory Management
All dynamic allocations are properly freed to prevent memory leaks.
The code has been tested with Valgrind for clean execution.

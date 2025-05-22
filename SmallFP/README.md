# 🧮 SmallFP: Software Floating-Point Library (CS 367 Project 2)

A low-level C implementation of a custom 11-bit floating-point library designed for embedded systems that lack native float or double support. This library is used by the MUAN (Micro-Ubiquitous Accounting Notary) programming language to support floating-point expressions.

---

## 📌 Project Overview

SmallFP implements encoding, decoding, and arithmetic operations on custom floating-point values without using the `float` or `double` types. All operations are performed using bitwise manipulation on 16-bit integers.

You will implement 6 core functions in `src/smallfp.c`:

- `toSmallFP()` – Encode integer & fraction into smallfp format  
- `toNumber()` – Decode a smallfp into a human-readable format  
- `addSmallFP()` – Perform addition  
- `subSmallFP()` – Perform subtraction  
- `mulSmallFP()` – Perform multiplication  
- `negSmallFP()` – Flip the sign bit  

---

## 🧠 Concepts Covered

- **Bitwise operations**: `&`, `|`, `^`, `~`, `<<`, `>>`
- **Floating-point emulation**: Custom 11-bit format (S:1, EXP:4, FRAC:6)
- **Binary scientific notation**
- **Rounding (round-to-nearest-even)**
- **Overflow/underflow handling**
- **Special values**: `NaN`, `Infinity`, `Zero`

---

## 📁 Project Structure

```
project-root/
├── src/
│ ├── smallfp.c # Your implementation file
│ └── test_smallfp.c # Optional unit test driver
├── inc/
│ ├── common_structs.h # Struct definitions
│ ├── smallfp_precision.h# Constants (do not modify)
│ └── MUAN_settings.h # Optional UI settings
├── scripts/
│ └── sample.muan # Sample script to run in MUAN
├── Makefile # For building MUAN + library
├── ref_all_values # Executable to see all valid encodings

yaml
Copy
Edit
```
---

## ⚙️ Build & Run

### 🔨 To compile everything:

```bash
make
▶️ Run MUAN interpreter:

./muan

Inside MUAN:
a = 3.25
display(a)       # Shows internal bit representation
print(a)         # Converts back to human-readable output

🧪 Unit Testing:
make tester
./tester


🔎 smallfp_s Format
+---+----------+------------+
| S | EXP (4b) | FRAC (6b)  |
+---+----------+------------+

S: Sign bit

EXP: Exponent in biased format

FRAC: 6-bit fraction

Zero: EXP=0 and FRAC=0

Infinity: EXP=0xF and FRAC=0

NaN: EXP=0xF and FRAC≠0


💡 Highlights
Simulates floating-point behavior on hardware without FPUs

Reinforces understanding of binary arithmetic

Provides practical embedded systems programming experience



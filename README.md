# C-Modern Compiler Front-End & TAC Generator

A robust, educational-focused compiler front-end for a modern subset of C. Built with **Flex** and **Bison**, this tool performs lexical analysis, syntax parsing, semantic validation, and generates Three-Address Code (TAC).

Designed as an **Educational Analyzer Tool**, it emphasizes plain-English error reporting to help beginners understand *why* their code is incorrect, bridging the gap between cryptic compiler faults and student learning.

---

## 🚀 Features

### 1. Lexical Analysis (Scanner)

* **Comprehensive Support:** Tokenizes standard C constructs and modern additions (e.g., `_Generic`, `nullptr`, `bool`, `_Alignas`).
* **Resilience:** Safely filters comments and whitespace while catching unterminated strings and illegal characters.

### 2. Syntax Analysis & AST Generation

* **Grammar Validation:** Parses expressions, declarations, loops (`for`, `while`), and conditionals (`if`).
* **Abstract Syntax Tree (AST):** Dynamically builds and visualizes an AST for structural representation.
* **Error Recovery:** Uses Bison's `error` token to gracefully skip malformed statements, allowing for multi-error reporting in a single pass.

### 3. Semantic Validation (Educational Feedback)

Includes a custom `[Friendly Compiler Notice]` system to guide novice programmers:

* **Scope & Declaration:** Detects undeclared identifiers and prevents redeclarations within the same scope.
* **Type Safety:** Warns against assigning decimals to integers to prevent silent data loss (truncation).
* **Clean Code:** Identifies variables that are declared but never used.
* **Logical Safety:** Blocks `void` variable declarations and intercepts division-by-zero.

### 4. Intermediate Code Generation (TAC)

* Generates **Three Address Code** for arithmetic, assignments, and control flow.
* Implements label-based branching for loops and conditionals.
* **Halt-on-Error:** Ensures TAC is only generated if the code passes all semantic checks.

---

## 📂 Project Structure

* `lexer.l`: Flex source file containing regular expressions and tokenization rules.
* `parser.y`: Bison file defining grammar, symbol table logic, and semantic checks.
* `ast.h` / `inter_code_gen.c`: Logic for traversing the AST and translating nodes into TAC.
* `Makefile`: Automates the build and clean processes.

---

## 🛠️ Build and Execution

### Prerequisites

* **Flex** (Fast Lexical Analyzer Generator)
* **Bison** (GNU Parser Generator)
* **GCC** (GNU Compiler Collection)

### Quick Start

1. **Build the Analyzer:**
Use the provided Makefile to compile the project automatically.
```bash
make

```


2. **Run the Compiler:**
Feed a C source file into the generated executable.
*On Windows (PowerShell):*
```powershell
Get-Content test_input.c | .\analyzer.exe

```


*On Linux/macOS:*
```bash
./analyzer < test_input.c

```


3. **Clean Build Files:**
To remove generated C files, headers, and the executable:
```bash
make clean

```



---

> **Note:** This tool is intended for educational purposes and covers a subset of the C language. It is not a replacement for production-grade compilers like GCC or Clang.

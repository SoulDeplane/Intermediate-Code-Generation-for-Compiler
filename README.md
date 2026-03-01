# C-Modern Compiler: Lexical Analyzer

A high-performance Lexical Analyzer (Scanner) built using **Flex**, designed to tokenize the **C23 standard** (and earlier) for a complete Compiler Design project.

## 1. Project Overview

This project is the first phase of a C compiler. The Lexer transforms raw C source code into a stream of meaningful tokens. Unlike basic student projects, this scanner is designed to be robust against modern C syntax, including C11/C23 keywords and complex numeric literals.

## 2. Key Features

* **Full Keyword Support:** Covers all 50+ keywords from ANSI C to C23 (including `_Generic`, `nullptr`, `bool`, etc.).
* **Modern Literals:** Handles Hexadecimal (`0x`), Octal (`0`), and Scientific notation (`1e-10`).
* **Advanced String Handling:** Supports wide-character strings (`L"..."`) and escape sequences.
* **Robust Error Handling:** * Tracks line numbers (`lno`) for precise error reporting.
* Detects unterminated comments and strings.
* Ignores Preprocessor directives (`#include`, `#define`) to prevent crashes.


* **Line Joining:** Correctly handles backslash line-splicing (`\`).

## 3. Token Categories

The lexer identifies and categorizes the following:
| Category | Examples |
| :--- | :--- |
| **Keywords** | `int`, `volatile`, `_Alignas`, `nullptr` |
| **Operators** | `++`, `->`, `<<=`, `&&`, `? :` |
| **Constants** | `1024`, `0xFF`, `3.14f`, `'A'` |
| **Identifiers** | Variable names, function names |
| **Strings** | `"Hello World"`, `L"Wide String"` |

## 4. Prerequisites

To build and run this project, you need:

* **Flex** (Fast Lexical Analyzer Generator)
* **Bison** (GNU Parser Generator) - *Required for the header file*
* **GCC** (C Compiler)

## 5. Build & Execution

Follow these steps to compile the scanner:

1. **Generate the Parser Header:**
```bash
bison -d parser.y

```


2. **Generate the Lexer:**
```bash
flex lexer.l

```


3. **Compile the Project:**
```bash
gcc lex.yy.c parser.tab.c -o compiler

```


4. **Run against a test file:**
```bash
./compiler test_input.c

```



## 6. Project Structure

* `lexer.l`: The Flex source file containing regular expressions and token rules.
* `parser.y`: The Bison file defining tokens and grammar (Phase 2).
* `parser.tab.h`: Generated header file linking the Lexer and Parser.
* `test_input.c`: Sample C code used for validation.

## 7. Error Reporting

The lexer provides descriptive feedback for illegal inputs:

```text
Lexical Error at line 12: Unterminated string '"'
Lexical Error at line 45: Unknown character '@'

```

---

**Would you like me to help you write a `test_input.c` file that includes all the "tricky" C features (like hex, scientific notation, and wide strings) to show off your lexer during your project demo?**

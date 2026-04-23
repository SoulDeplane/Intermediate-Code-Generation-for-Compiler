# Intermediate Code Generation for Compiler

A compiler front-end for a modern subset of C with a built-in auto-correction layer and a web UI for exploring every phase of compilation. Built with Flex, Bison, C, Python (Flask), and a small browser front-end.

The pipeline is modular — lexer, parser/semantic, intermediate code generator, code corrector, and optimizer are each independent stages. The web UI runs them in sequence and surfaces every artifact (tokens, AST, symbol table, errors, auto-corrected source, optimized TAC) in a tabbed view.

---

## Features

### Lexical Analysis
Tokenizes the standard C keyword set plus modern additions (_Generic, _Alignas, _Atomic, nullptr, bool, etc.). Handles decimal, octal, hexadecimal, and floating-point literals including exponent and suffix forms. Skips line and block comments, and reports unterminated comments, unterminated strings, malformed character literals, malformed numbers, and illegal characters via the shared diagnostics system.

### Syntax and Semantic Analysis
Bison grammar covering declarations, expressions, control flow (if, while, for), function definitions with parameter lists, function calls with argument count and type checking, pointer/address-of/dereference, and return statements. Builds an Abstract Syntax Tree during parsing and prints it to stdout. Maintains a scoped symbol table with kind tracking (variable, function, parameter), declaration/use status, and an is_allocated flag for malloc/free pairing. Semantic checks include undeclared identifiers, redeclarations in the same scope, void variables, division by zero, type mismatches in initialization and assignment, unused-variable warnings, possible memory leaks, missing returns in non-void functions, unsafe APIs, and argument count and type mismatches against function signatures. Bison error tokens with plain-English messages let parsing continue past malformed declarations, expressions, and conditionals so multiple errors can be reported in a single pass.

### Intermediate Code Generation
Walks the AST and emits Three-Address Code for arithmetic, assignment, comparisons, function calls (param/call), array indexing, address-of/dereference, returns, and label-based control flow for if/while/for. Writes to icg.txt only if no errors were recorded.

### Code Optimization
Reads icg.txt and applies constant propagation, constant folding, and dead code elimination (the last is run twice for fixed-point convergence). Writes the result to optimized_icg.txt. With --print, prints the IR after every pass for inspection.

### Auto-Correction Layer
A second-pass corrector that catches the most common beginner mistakes before the analyzer ever sees them. Runs as a separate executable invoked by the web server. All replacements are token-aware (skipping string literals, character literals, and comments) and idempotent.

Currently corrects: keyword typos (~40 entries), stdlib function typos (~60 entries), missing semicolons (parser-driven and locally detected), wrong terminator (trailing , or : in place of ;, with case/default/goto-label/initializer guards), missing # before include, missing .h on 16 standard headers, automatic #include injection when stdlib functions are used without their header (~70 functions tracked across stdio/stdlib/string/ctype/math/time), missing % in format specifiers (narrow rule guarded by surrounding context), unclosed " on a single line, unbalanced ) or ] when a call/index pattern is recognizable, smart quotes from paste-from-Word artifacts, void main → int main, missing int before main, and missing closing } at EOF.

### Diagnostics System
Single shared error pipeline used by every phase. Records phase, severity (warning/error/fatal), line, column, machine-readable code, human message, and a recovery hint. Supports a setjmp/longjmp fatal escape and prints a summary at end of compilation. The Flask server parses these structured records to drive the auto-correction step.

### Web UI
A Flask backend exposes /compile, which runs the analyzer on the user's source, runs the code corrector unconditionally (it is idempotent on clean code), re-runs the analyzer on the corrected source if it changed, runs the optimizer on the produced TAC, and returns lex tokens, AST, symbol table, raw diagnostics, corrected source, and optimized TAC as JSON. The browser front-end shows everything in five tabs: TAC & Errors, Lex Tokens, Parser Tree, Symbol Table, and Auto-Corrected. The Auto-Corrected tab is empty when no corrections were applied; otherwise it is highlighted with an asterisk.

---

## Build and Execution

### Prerequisites
- Flex (Fast Lexical Analyzer Generator)
- Bison (GNU Parser Generator)
- GCC
- Python 3 with Flask (only required for the web UI: pip install flask)

### Build
make — builds both analyzer.exe and code_correc.exe.

### Run from the command line
Pipe a C source file directly into the analyzer.

- Linux / macOS: ./analyzer < test_input.c
- Windows PowerShell: Get-Content test_input.c | .\analyzer.exe

The analyzer prints lex tokens, AST, symbol table, and a diagnostics summary to stdout/stderr, and writes TAC to icg.txt if compilation succeeds. To then optimize: make optimize (runs python optimiser.py icg.txt --print).

### Run the web UI
make then python server.py, and open <http://127.0.0.1:5000>. Paste C code in the input pane and click Process Data.

### Clean
make clean removes generated files (Flex/Bison output, executables, intermediate artifacts). The clean target uses Windows del; substitute rm -f on Linux/macOS.

---

> This tool is intended for educational use and covers a subset of C. It is not a replacement for production-grade compilers like GCC or Clang.

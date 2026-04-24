# Intermediate Code Generation for Compiler

A compiler front-end for a modern subset of C with a built-in auto-correction layer and a web UI for exploring every phase of compilation. Built with Flex, Bison, C, Python (Flask), and a small browser front-end.

The pipeline is modular — lexer, parser/semantic, intermediate code generator, code corrector, and optimizer are each independent stages. The web UI runs them in sequence and surfaces every artifact (lex tokens table, concrete parse tree, symbol table, plain-English diagnostics, auto-corrected source, optimized TAC) in a tabbed view.

---

## Features

### Lexical Analysis
Tokenizes the standard C keyword set plus modern additions (_Generic, _Alignas, _Atomic, nullptr, bool, etc.). Handles decimal, octal, hexadecimal, and floating-point literals including exponent and suffix forms. Captures preprocessor directives as a dedicated PREPROC token so they appear in the token stream and the parse tree. Skips line and block comments, and reports unterminated comments, unterminated strings, malformed character literals, malformed numbers, and illegal characters via the shared diagnostics system. Every token is also stored in a buffer for the parse-tree builder.

### Syntax and Semantic Analysis
Bison grammar covering declarations, expressions, control flow (if, while, for), function definitions with parameter lists, function calls with argument count and type checking, pointer/address-of/dereference, and return statements. Builds an Abstract Syntax Tree used internally for intermediate code generation. Maintains a scoped symbol table with kind tracking (variable, function, parameter), declaration/use status, and an is_allocated flag for malloc/free pairing. The symbol table view excludes the `main` function entry. Semantic checks include undeclared identifiers, redeclarations in the same scope, void variables (only for variables — `void main()` is allowed), division by zero, type mismatches in initialization and assignment, unused-variable warnings (excluding `main`), possible memory leaks, missing returns in non-void functions, unsafe APIs, and argument count and type mismatches against function signatures. Bison error tokens with plain-English messages let parsing continue past malformed declarations, expressions, and conditionals so multiple errors can be reported in a single pass.

### Concrete Parse Tree
A separate concrete parse tree is built by a hand-written recursive-descent walker over the captured token buffer. Its leaves are the actual lex tokens with their original text, so reading the leaves left-to-right reconstructs the source. Internal nodes carry meaningful labels (function_definition, block, declaration, expression_statement, if_statement, for_statement, etc.). The tree is emitted both as an ASCII diagram for terminal use and as JSON for the browser to render visually.

### Intermediate Code Generation
Walks the AST and emits Three-Address Code for arithmetic, assignment, comparisons, function calls (param/call), array indexing, address-of/dereference, returns, and label-based control flow for if/while/for. Writes to icg.txt only if no errors were recorded.

### Code Optimization
Reads icg.txt and applies constant propagation, constant folding, and dead code elimination (the last is run twice for fixed-point convergence). Writes the result to optimized_icg.txt. With --print, prints the IR after every pass for inspection.

### Auto-Correction Layer
A second-pass corrector that catches the most common beginner mistakes before the analyzer ever sees them. Runs as a separate executable invoked by the web server. All replacements are token-aware (skipping string literals, character literals, and comments) and idempotent.

Currently corrects: keyword typos (~40 entries), stdlib function typos (~60 entries), `include` typos so a malformed preprocessor line is fixed in place rather than supplemented, missing semicolons (parser-driven and locally detected, with a guard so a function header followed by `{` on the next line is not mistakenly closed with `;`), wrong terminator (trailing `,` or `:` in place of `;`, with case/default/goto-label/initializer guards), missing `#` before include, missing `.h` on 16 standard headers, automatic `#include` injection when stdlib functions are used without their header (~70 functions across stdio/stdlib/string/ctype/math/time), missing `%` in format specifiers (narrow rule guarded by surrounding context), unclosed `"` on a single line, unbalanced `)` or `]` when a call/index pattern is recognizable, smart quotes from paste-from-Word artifacts, missing `int` before bare `main()` (preserves `void main()` and any other typed `main`), missing closing `}` at EOF, automatic `int <name> = 0;` injection inside `main` for undeclared identifiers reported by the analyzer, and automatic `return 0;` for a non-void `main` that lacks one.

### Diagnostics System
Single shared error pipeline used by every phase. Records phase, severity (warning/error/fatal), line, column, machine-readable code, human message, and a recovery hint. Supports a setjmp/longjmp fatal escape and prints a summary at end of compilation. The Flask server parses these structured records to drive the auto-correction step and rewrites them as plain-English sentences for the user.

### Web UI
A Flask backend exposes /compile, which runs the analyzer on the user's source, runs the code corrector unconditionally (it is idempotent on clean code), re-runs the analyzer on the corrected source if it changed, runs the optimizer on the produced TAC, and returns lex tokens (text and JSON), the parse tree (text and JSON), the symbol table (text and JSON), corrected source, plain-English diagnostics, and optimized TAC as JSON. The browser front-end shows everything in five tabs: TAC & Errors, Lex Tokens, Parse Tree, Symbol Table, and Auto-Corrected. Lex Tokens and Symbol Table render as real HTML tables (styled grid with header row, alternating stripes, hover highlight). The Parse Tree renders as a visual tree with CSS-drawn connecting branch lines, internal nodes in the accent color and leaves in monospace. The Auto-Corrected tab is empty when no corrections were applied; otherwise it is highlighted with an asterisk. All diagnostics shown in TAC & Errors are plain-English sentences.

The input textarea inserts eight spaces when Tab is pressed (or indents every selected line); Shift+Tab outdents; Ctrl/Alt/Meta + Tab fall through to the browser default so focus navigation still works.

The layout is responsive. Above 900 px the input panel and output panel sit side by side. At 900 px or below, the layout collapses to a single column and an extra "Input" tab button appears in the tab bar; clicking it shows the input panel and clicking any other tab shows the corresponding output content.

---

## Build and Execution

### Prerequisites
- Flex (Fast Lexical Analyzer Generator)
- Bison (GNU Parser Generator)
- GCC
- Python 3 with Flask (only required for the web UI: pip install flask)

### Build
`make` — builds both `analyzer.exe` and `code_correc.exe`.

### Run from the command line
Pipe a C source file directly into the analyzer.

- Linux / macOS: `./analyzer < test_input.c`
- Windows PowerShell: `Get-Content test_input.c | .\analyzer.exe`

The analyzer prints the lex tokens table, the concrete parse tree, the symbol table, and a diagnostics summary to stdout/stderr, and writes TAC to icg.txt if compilation succeeds. To then optimize: `make optimize` (runs `python optimiser.py icg.txt --print`).

### Run the web UI
`make` then `python server.py`, and open <http://127.0.0.1:5000>. Paste C code in the input pane and click Process Data.

### Clean
`make clean` removes generated files (Flex/Bison output, executables, intermediate artifacts). The clean target uses Windows `del`; substitute `rm -f` on Linux/macOS.

---

> This tool is intended for educational use and covers a subset of C. It is not a replacement for production-grade compilers like GCC or Clang.

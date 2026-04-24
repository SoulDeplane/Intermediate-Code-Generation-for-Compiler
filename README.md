# Intermediate Code Generation for Compiler

---

## Features

### Lexical Analysis
* **Modern Token Set:** Supports standard C keywords plus modern additions like _Generic, _Atomic, nullptr, and bool.
* **Literal Parsing:** Handles decimal, octal, hexadecimal, and floating-point literals, including exponent and suffix variations.
* **Preprocessor Handling:** Captures directives as PREPROC tokens to include them in the visual parse tree without disrupting the grammar.
* **Robust Error Reporting:** Detects and reports unterminated comments/strings, malformed character literals, and illegal characters via a shared diagnostic system.
* **Buffering:** Stores every token in an in-memory buffer to support the construction of the Concrete Parse Tree.

### Syntax and Semantic Analysis
* **Broad Grammar:** Bison-based parser covering declarations, control flow (if, while, for), function definitions, and pointer operations.
* **Scoped Symbol Table:** Tracks identifiers by kind (variable, function, parameter) and manages scope-specific declaration/use status.
* **Safety & Integrity Checks:**
    * Validates void usage (disallowed for variables, allowed for void main()).
    * Detects division by zero and type mismatches in assignments/initializations.
    * Checks for unused variables, memory leaks (is_allocated tracking), and missing returns.
* **API & Signature Validation:** Flags unsafe APIs and enforces argument count/type matching against function signatures.
* **Resilient Parsing:** Utilizes Bison error tokens to report multiple errors in a single pass while providing plain-English diagnostics.

### Concrete Parse Tree
* **Manual Construction:** Built using a hand-written recursive-descent walker over the captured token buffer.
* **High Fidelity:** Leaves contain the original lex tokens, allowing the source to be reconstructed by reading leaves left-to-right.
* **Structural Labeling:** Nodes are labeled by construct to aid visualization.
* **Visual Output:** Emits an ASCII diagram for terminal users and a JSON structure for browser-based rendering.

### Intermediate Code Generation
* **TAC Generation:** Walks the AST to produce Three-Address Code for arithmetic, function calls, array indexing, and control flow.
* **Conditional Output:** Only writes to icg.txt if the source is free of errors, ensuring internal consistency.

### Code Optimization
* **Standard Passes:** Performs constant propagation, constant folding, and dead code elimination.
* **Fixed-Point Convergence:** Runs dead code elimination twice to ensure maximum efficiency.
* **Transparency:** Optional --print flag allows users to inspect the IR after every optimization pass.

### Auto-Correction Layer
* **Mistake Mitigation:** A separate executable catches common beginner errors before they reach the main analyzer.
* **Heuristic Fixes:** * Corrects ~100 common typos in keywords and standard library functions.
    * Repairs missing semicolons (with guards for function headers) and incorrect terminators.
    * Normalizes smart quotes and repairs unclosed strings or unbalanced parentheses.
* **Automatic Injection:** Adds missing #include directives for ~70 standard functions and injects return 0; or default variable declarations where necessary.
* **Safety:** Operates as a token-aware state machine to avoid modifying content within strings or comments.

### Diagnostics System
* **Shared Pipeline:** Provides unified error handling (LEX, SYN, SEM, ICG) with severity levels and recovery hints.
* **Flow Control:** Supports setjmp/longjmp for clean fatal-error escapes.
* **User-Centric:** Structured machine-readable records are humanized into plain-English sentences by the Flask server.

### Web UI
* **Tabbed Interface:** Surfaces the Lex Tokens, Parse Tree, Symbol Table, Auto-Corrected Source, and Optimized TAC.
* **Responsive Rendering:**
    * **Tables:** Styled HTML grids with hover highlights for tokens and symbols.
    * **Trees:** Visual trees using CSS-drawn branches and color-coded nodes.
* **Integrated Editor:** Support for smart Tab-key indentation and outdenting.
* **Adaptive Layout:** Side-by-side view for desktops (>900 px) and a tab-switched single column for mobile devices.

---

## Build and Execution

### Prerequisites
* **Compiler Tools:** Flex, Bison, and GCC.
* **Web Environment:** Python 3 with the Flask library (pip install flask).

### Build
* Use the command make to compile both analyzer.exe and code_correc.exe.

### Run from the command line
1. **Analyze:** Pipe C source into the analyzer:
   * **Linux/macOS:** ./analyzer < test_input.c
   * **Windows:** Get-Content test_input.c | .\analyzer.exe
2. **Optimize:** After analysis, run make optimize to process icg.txt.

### Run the web UI
1. Run make to prepare the binaries.
2. Start the server with python server.py.
3. Navigate to http://127.0.0.1:5000 to interact with the GUI.

### Clean
* Run make clean to remove intermediate artifacts, generated Flex/Bison source files, and executables.

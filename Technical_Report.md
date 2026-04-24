# Intermediate Code Generation for Compiler

---

## 1. Pipeline Overview
* **Sequential Execution:** Composed of independent stages (Analyzer, Corrector, Optimizer) communicating via files and standard I/O.
* **Single-Pass Analysis:** The analyzer performs lexical, syntax, and semantic analysis in one pass, generating a token buffer for UI visualization.
* **Diagnostic Flow:** Structured errors are written to stderr. If clean, Three-Address Code (TAC) is saved to icg.txt.
* **Feedback Loop:** The corrector reads errors and the original source to produce corrected_source.c. If changed, the analyzer re-runs on the fixed code.
* **Optimization:** A Python-based optimizer applies constant folding and dead code elimination to the final TAC.
* **Web Orchestration:** A Flask-based server manages the subprocesses and exposes results (JSON/Tables) to a responsive UI.

---

## 2. Lexical Analysis (lexer.l)
* **Token Recognition:** Processes standard C identifiers, complex literals (hex, float, string), and modern keywords.
* **Preprocessor Handling:** Captured as PREPROC tokens for visualization without affecting the core grammar.
* **Error Detection:** Emits specific codes for UNCLOSED_COMMENT, MALFORMED_NUMBER, and ILLEGAL_CHAR.
* **Dual Output:** Records every token in a global buffer for the Parse Tree and prints both tabular (terminal) and JSON (UI) formats.

---

## 3. Parser, AST, and Symbol Table (parser.y)
### 3.1 Grammar
* **C Subset:** Supports function definitions, declarations (pointers/arrays), and expressions following standard precedence.
### 3.2 Symbol Table
* **Scope Management:** Uses a linked-list structure with integer-numbered scopes; invalidates entries upon scope exit.
* **Validation:** Tracks is_used (for warnings) and is_allocated (to detect memory leaks).
### 3.3 Function Signatures
* **Type Safety:** Maintains a registry of functions to validate argument counts and types during calls.
### 3.4 AST
* **Internal Representation:** A four-slot node structure (left, mid1, mid2, right) optimized for for-loops and binary operations.
### 3.5 Semantic Checks
* **Rule-Based Validation:** Catches implicit truncations, division-by-zero, missing returns, and usage of unsafe APIs like gets.
### 3.6 Error Recovery
* **Resilience:** Emits plain-English messages and uses yyerrok to continue parsing despite syntax errors.

---

## 4. Concrete Parse Tree (parser.y, post-pass on token_buf)
* **CST vs. AST:** Built independently of the AST using the raw token stream to ensure the UI tree matches the user's literal code.
* **Recursive Descent:** A hand-written walker recognizes constructs like cst_parse_if or cst_parse_block.
* **Visualization:** Outputs an ASCII tree for terminals and a hierarchical JSON structure for the web UI's visual tree.

---

## 5. Intermediate Code Generation (inter_code_gen.c)
* **TAC Generation:** Walks the AST to emit instructions ($t = a \text{ op } b$, $if \text{ cond } goto \dots$).
* **Resource Management:** Uses new_temp and new_label with OOM (Out of Memory) safety checks.
* **Integrity Gate:** Only runs if the diagnostic layer reports zero errors, ensuring TAC is always valid.

---

## 6. TAC Optimizer (optimiser.py)
* **Constant Propagation:** Replaces variables with known constant values.
* **Constant Folding:** Computes math expressions at compile-time.
* **Dead Code Elimination:** Removes assignments to temporaries that are never used later.
* **Convergence:** Runs passes repeatedly until no further changes are detected.

---

## 7. Code Auto-Correction (code_correc.c)
* **Heuristic Fixes:** Automatically repairs #include syntax, normalizes smart quotes, and closes string literals.
* **Typo Correction:** Uses a 110-entry table to fix keywords and standard library functions.
* **Format/Syntax Fixes:** Adds missing % in format strings and repairs wrong terminators.
* **Error-Driven Fixes:** Injects ; or default int declarations based on specific analyzer error codes.
* **Safety Machine:** A state machine ensures no changes are made inside strings or comments.

---

## 8. Diagnostics System (errors.h / errors.c)
* **Unified Layer:** Shared across all phases with standard severity levels (WARNING, ERROR, FATAL).
* **Machine Readable:** Emits stable codes that allow the server to "humanize" messages for the UI.
* **Fatal Aborts:** Uses longjmp to cleanly exit a phase and return control to the main wrapper.

---

## 9. Web Orchestration (server.py, index.html)
* **Backend (Flask):** Drives the pipeline via subprocesses, handles character encoding, and compares source versions to detect if corrections were applied.
* **Frontend (UI):** Features a tabbed interface for TAC, Lexical Tables, visual Parse Trees (CSS-styled), and Symbol Tables.
* **Editor Features:** Custom Tab-key handling for indentation and a responsive layout that adapts to mobile/desktop views.

---

## 10. Build Topology (Makefile)
* **Compilation:** Compiles analyzer.exe and code_correc.exe using GCC, Flex, and Bison.
* **Maintenance:** Includes a clean target to wipe all temporary files and intermediate binaries.
* **Portability:** Designed for Windows/Unix environments with minor command adjustments.

---

## 11. End-to-End Behavior
* **Clean Code:** Passes through to optimized TAC and full visualization.
* **Fixable Code:** UI highlights corrections; the analyzer re-validates the fixed code before displaying results.
* **Unfixable Code:** Displays human-friendly error messages; suppresses TAC generation while still attempting to render the recovered Parse Tree.

# Project Proposal: Intermediate Code Generation for Compiler

## MOTIVATION

* **Importance of Static Analysis:** Detects errors before runtime, improving software reliability. Identifies memory leaks and security vulnerabilities in C code.
* **Challenges for Learners:** Compiler errors (GCC, Clang) are often cryptic for beginners. Logic and memory issues may go unnoticed until runtime failures.
* **Project Aim:** Build a static analysis tool that processes full C programs. Detect lexical, syntax, and semantic errors with clear explanations.
* **Educational Benefits:** Explains why code is incorrect, not just where. Reinforces scoping, type safety, and clean coding practices.

---

## STATE OF THE ART

* **Modern Compilers:** GCC and Clang focus on optimization and code generation. Limited educational feedback for beginners.
* **Existing Static Analyzers:** Tools like CppCheck and Splint target enterprise compliance. Lack beginner-friendly diagnostics or correction suggestions.
* **Gap in Current Solutions:** Few tools offer deep educational feedback. No accessible platform that teaches compiler design through hands-on error correction.

---

## PROJECT GOALS AND MILESTONES

### Primary Goal

* **Analyze full C programs** and generate detailed error reports.
* **Focus on source-level analysis** without the overhead of generating machine code.

### Initial Milestones

* **Lexical Analysis:** Tokenize source code and detect invalid tokens.
* **Syntax Analysis:** Validate grammar (detecting missing semicolons, unmatched braces, etc.).
* **Semantic Analysis:**
* Type compatibility checks.
* Scope and declaration validation.



### Stretch Goals

* **Suggest code improvements:** Identify and suggest the removal of unused variables.
* **Auto-Correction:** Generate corrected code snippets for simple, common errors.

### Progress Strategy

* **Modular development:** Follow a linear pipeline: Lexer → Parser → Analyzer → Reporter.

---

## PROJECT APPROACH

### Modular Development

The system is divided into separate, manageable components to ensure robust debugging and maintenance:

1. **Lexer**
2. **Parser**
3. **Semantic Analyzer**
4. **Error Reporter**

### Lexical Analysis

* Identify and categorize keywords, identifiers, and literals.
* Filter out comments and unnecessary whitespace to streamline further processing.

### Syntax Analysis

* Apply a defined grammar for a specific subset of the C language.
* Construct a **Parse Tree** or an **Abstract Syntax Tree (AST)** to represent the program structure.

### Semantic Analysis

* Maintain a **Symbol Table** to track variable types and their respective scopes.
* Perform deep type checking and logical validation (e.g., ensuring variables are declared before use).

### Error Reporting

* **Error Recovery:** Design the system to recover from an error and continue parsing to report multiple issues in a single pass.
* **User-Friendly Feedback:** Provide plain-English explanations instead of cryptic codes.

### Testing and Validation

* **Buggy Code Samples:** Test against beginner-level code containing common mistakes.
* **Advanced Programs:** Validate the analyzer using complex, clean C programs.
* **Diverse Constructs:** Ensure the tool handles various C constructs like loops, functions, and nested scopes.

---

## SYSTEM ARCHITECTURE

| Component | Responsibility |
| --- | --- |
| **Lexer (Flex)** | Converts source code into a stream of tokens. |
| **Parser (Bison)** | Builds a Syntax Tree and validates grammar rules. |
| **Semantic Analyzer** | Manages symbol tables and enforces type consistency. |
| **Intermediate Code Gen** | Produces a simplified, machine-independent representation. |

---

## PROJECT OUTCOME

* **Educational Analyzer Tool:** Parses C code and explains errors in plain English.
* **Technical Report:** Documents grammar rules and error-handling logic.
* **Demonstration:** Showcases tool’s ability to fix broken code.
* **Academic Contribution:** Bridges gap between compilers and student learning tools.

---

## ASSUMPTIONS

* Focus on core C subset (loops, functions, basic types).
* No object or assembly code generation.
* Prioritize clarity and correctness over performance.

---

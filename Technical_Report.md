# Technical Report: Compiler Pipeline, Diagnostics, and Auto-Correction

This document describes the architecture of the compiler front-end, the structured diagnostics layer shared across all phases, the auto-correction stage, the TAC optimizer, and the web orchestration layer that ties them together.

---

## 1. Pipeline Overview

The project is composed of independent stages that communicate through files and standard I/O.

The source file is fed into the analyzer, which performs lexical analysis, parsing, semantic checks, and intermediate code generation in a single pass. The analyzer writes structured diagnostics to stderr and, only if no errors were recorded, writes Three-Address Code to icg.txt.

The corrector (code_correc.exe) then reads the original source plus any structured error rows extracted from the analyzer's stderr and produces corrected_source.c. If the corrected source differs from the original, the analyzer is re-run on it to produce a fresh diagnostic stream and a fresh icg.txt.

Finally, the optimizer (optimiser.py) reads the latest icg.txt and writes the result to optimized_icg.txt after constant propagation, constant folding, and dead code elimination.

Two binaries are built: analyzer.exe (lex + parse + semantic + ICG) and code_correc.exe (auto-corrector). The optimizer is a Python script. The web UI (server.py + index.html) drives the whole pipeline through subprocess calls and exposes results as JSON.

---

## 2. Lexical Analysis (lexer.l)

Flex source defining the token set the parser consumes. It tokenizes identifiers, decimal/octal/hexadecimal/floating-point literals (with suffix and exponent forms), string and character literals (including the L prefix), the standard C keyword set plus modern additions (_Generic, _Alignas, _Atomic, _Noreturn, _Static_assert, _Thread_local, bool, true, false, nullptr, inline, restrict), and the full operator/punctuation set including compound assignments, comparisons, logical, bitwise, increment/decrement, arrow, and ellipsis.

Block comments are handled via a <COMMENT> start condition; line comments are skipped. Each rule maintains line and column counters and emits structured diagnostics on lexical errors:

| Code | Trigger |
|---|---|
| UNCLOSED_COMMENT | EOF while inside /* |
| UNCLOSED_STRING | newline inside string literal |
| MALFORMED_CHAR_LITERAL | malformed '...' |
| MALFORMED_NUMBER | e.g. 1.2.3 |
| ILLEGAL_CHAR | character not matched by any rule |

---

## 3. Parser, AST, and Symbol Table (parser.y)

### 3.1 Grammar

The grammar covers a practical subset of C: declarations, function definitions with parameter lists, statements (compound, expression, iteration, condition), and expressions stratified by standard precedence from assignment_expression down to primary_expression. Init-declarators support plain, array, and pointer (MULT IDENTIFIER) forms with optional initializers.

### 3.2 Symbol Table

A linked list of struct node entries with these fields:

| Field | Purpose |
|---|---|
| name, dtype, scope | identity |
| valid | active in current scope |
| is_used | flips on first lookup; powers unused-variable warnings |
| sym_kind | VAR / FUNC / PARAM |
| is_allocated | set when initialized from malloc; cleared on free |
| val | tracked constant value (for folding) |

Scopes are integer-numbered. Entering a compound_statement increments scope; leaving it walks the list and invalidates entries from that scope, emitting unused-variable and leak warnings as it goes.

### 3.3 Function Signatures

A separate function_sig linked list tracks declared functions and their parameter type lists (up to MAX_PARAMS = 32). Calls are checked against this table for argument count and type compatibility, producing ARG_COUNT_MISMATCH and ARG_TYPE_MISMATCH diagnostics.

### 3.4 AST

Built bottom-up via a tree stack (tree_stack). Each Node has left, mid1, mid2, right slots so for loops can store init/cond/inc/body without auxiliary nodes, plus token, addr (filled in by ICG), and eval_type. create_node(token, leaf) either pushes a leaf or pops two children and pushes a binary node. Statement-sequence, conditional, call, and pointer nodes are constructed directly in the relevant grammar actions. Two error sentinels exist: dummy (placeholder for missing operands) and __error__ (recorded after error token productions); both are skipped by the ICG and AST printer.

### 3.5 Semantic Checks

Run inline during parsing. Codes:

| Code | Severity | Meaning |
|---|---|---|
| IMPLICIT_TRUNCATION | warning | float assigned to int |
| TYPE_MISMATCH_INIT / TYPE_MISMATCH_ASSIGN | error | char into numeric, etc. |
| UNUSED_VARIABLE | warning | declared, never is_used |
| LEAK_POSSIBLE | warning | is_allocated still true at scope exit |
| MISSING_RETURN | error | non-void function without return value |
| UNSAFE_API | warning | use of gets |
| ARG_COUNT_MISMATCH / ARG_TYPE_MISMATCH | error | call vs. signature |

Type mismatches in arithmetic (e.g. multiplying strings) and division-by-zero are caught in the multiplicative production rules.

### 3.6 Error Recovery

Each major nonterminal carries one or more error productions that emit plain-English yyerror messages and call yyerrok to resume parsing. This covers missing semicolons after declarations, missing or invalid type specifiers, malformed if/while conditions (missing or unmatched parentheses), and statement-level syntax errors. yyerror routes through report_error → error_log, so every parser complaint becomes a structured diagnostic with line and column information.

---

## 4. Intermediate Code Generation (inter_code_gen.c)

A recursive AST walker that emits Three-Address Code to an open FILE *icg_out. The token at each AST node selects the emission rule:

| AST token | TAC emitted |
|---|---|
| leaf | addr := token (no emission) |
| +, -, *, /, comparisons | t = lhs OP rhs |
| = | lhs = rhs |
| if | if cond == 0 goto Lend … Lend: |
| while | Lstart: … if cond == 0 goto Lend … goto Lstart … Lend: |
| for | init + label + cond test + body + inc + back-edge |
| return | return [addr] |
| param / call | param x … t = call f |
| [] | t = a[i] |
| addr | t = &x |
| deref | t = *x |

Temporaries (t0, t1, …) and labels (L0, L1, …) are allocated by new_temp and new_label. Allocation failures route through the diagnostics layer with codes OOM_TEMP / OOM_LABEL and abort the ICG pass. The parser only opens icg.txt and calls generate_ICG if error_has_errors() returns false, guaranteeing TAC corresponds to a clean parse and semantic check.

---

## 5. TAC Optimizer (optimiser.py)

Three classical passes operate on the in-memory list of TAC lines:

1. **Constant Propagation** — for every t = c line where c is a constant, store t → c in a dictionary; later 5-token lines t = a OP b substitute known constants for a and b.
2. **Constant Folding** — for any 5-token line with both operands numeric, evaluate the expression and replace it with its computed value.
3. **Dead Code Elimination** — compute the set of temporaries that appear on the right-hand side of any later instruction; drop assignments to temporaries not in that set. Repeated until a fixed point (the entry point runs it twice for safety).

Output is written to optimized_icg.txt. With --print, the IR is dumped after each pass.

---

## 6. Code Auto-Correction (code_correc.c)

A standalone executable that takes (source.c, errors.txt, corrected.c) and produces a corrected source. Designed to be safe by default: every transformation is local, token-aware (skips strings, characters, and comments), and idempotent.

### 6.1 Per-Line Helpers

Run for each line in apply_heuristic_fixes() in this exact order:

1. fix_missing_hash_include — include<x> → #include<x>.
2. fix_common_header_extensions — table-driven <stdio> → <stdio.h> (16 headers, both <x> and < x > spacings).
3. normalize_smart_quotes — UTF-8 U+201C/D/18/19 → ASCII " / '.
4. fix_unclosed_string_literal — odd " count → close before trailing ; or EOL. Skips lines containing // or /*.
5. int main injection — bare main(...) → int main(...).
6. replace_identifier_safe(... "man" → "main") — only when man( or man ( is present.
7. replace_identifier_safe(... "print" → "printf").
8. apply_typo_table_to_line — ~100 entries covering keyword and stdlib-function typos.
9. fix_void_main — void main( → int main(. Runs after typos so chains like voi man() resolve correctly.
10. fix_format_string_missing_percent — within a recognized printf/scanf-family call, replace  <fmt-letter><tail> with  %<fmt-letter><tail>, where the letter is one of d/i/u/x/o/f/e/g/s/c/p (also ld, lf, lld, hd) and <tail> is \n, \t, ", or space.
11. fix_wrong_terminator — trailing , or : that isn't case X:, default:, a goto label, or inside a {...} line.
12. fix_unbalanced_parens_at_eol — appends a single missing ) or ] when a call or index pattern is recognizable; bails on multi-missing or ambiguous cases.

### 6.2 File-Level Helpers

Run once after the per-line pass:

- detect_and_fix_missing_semicolons_local — appends ; to every line is_semicolon_candidate_line flags.
- inject_missing_headers — for each (header, function-list) pair, if any function in the list is used anywhere (token-aware via line_uses_identifier) and no #include of that header is present, prepend #include <header> at the top via prepend_line. Maps stdio.h, stdlib.h, string.h, ctype.h, math.h, time.h to ~70 functions total.

After apply_heuristic_fixes() returns, main() reads errors.txt (rows of the form MISSING_SEMICOLON <line>), applies the parser-driven fix_missing_semicolon, then fix_missing_closing_braces_at_eof, then inject_missing_headers, then writes the result.

### 6.3 The String/Char/Comment Guard

replace_identifier_safe carries an explicit state machine while scanning each character: in_str, in_chr, esc, in_line_comment, in_block_comment. Substitution can only happen when none of those flags are set. This prevents the corrector from touching typos inside string or character literals, typos inside line or block comments, and identifiers where the typo appears as a substring (boundary checks via is_ident_char on both sides of the match). is_semicolon_candidate_line likewise rejects lines starting with // or /*, and line_uses_identifier tracks the same five states.

### 6.4 Idempotence

Re-running the corrector on its own output produces byte-identical results. This matters because the web server runs the corrector unconditionally and uses a string-equality test (after CRLF normalization) to decide whether to surface the corrected source in the UI.

---

## 7. Diagnostics System (errors.h / errors.c)

A single shared layer used by every phase. The public surface exposes a phase enum (LEX, SYN, SEM, ICG, SYS), a severity enum (WARNING, ERROR, FATAL), a reset function, a setjmp registration entry point, two logging functions (one variadic), error-state queries, and a summary printer.

Each call appends an ErrorEntry to a singly-linked list and immediately prints a structured line to stderr containing severity, phase, line, column, machine-readable code, message, and an optional recovery hint. Counts of warnings, errors, and fatals are maintained for fast error_has_errors() queries. Logging a fatal triggers a longjmp to the buffer registered in the parser's main, letting any phase abort cleanly without leaving partially-written files.

Because the code field is stable and machine-readable (e.g. MISSING_RETURN, TYPE_MISMATCH_INIT, LEAK_POSSIBLE, ARG_TYPE_MISMATCH, UNCLOSED_STRING, PARSER_ERROR), the web server's regex extractor can reliably pick out specific diagnostics — currently PARSER_ERROR lines whose message contains Missing semicolon — to drive the corrector's targeted MISSING_SEMICOLON rule.

---

## 8. Web Orchestration (server.py, index.html)

### 8.1 Backend

A Flask app exposing two routes. GET / serves index.html. POST /compile accepts a JSON code field and runs the full pipeline:

1. Write the source to temp_source.c.
2. Run analyzer.exe with the source on stdin and capture stdout / stderr.
3. Parse stderr with a regex matching the structured diagnostic format and extract MISSING_SEMICOLON <line> rows into errors.txt.
4. Run code_correc.exe temp_source.c errors.txt corrected_source.c unconditionally. The corrector is idempotent on clean code, so this is safe.
5. Compare corrected vs. original after CRLF and whitespace normalization. If different, set has_corrections = True and re-run the analyzer on the corrected source for the final stdout/stderr.
6. If icg.txt exists, run optimiser.py icg.txt and read optimized_icg.txt.
7. Extract sections (lex tokens, AST, symbol table) from analyzer stdout; filter the Variable 'main' is declared but never used warning from diagnostics shown to the user.
8. Return JSON: tac, lex_tokens, parser_tree, symbol_table, corrected_code, has_corrections, initial_stderr, final_stderr.

Temporary files (temp_source.c, errors.txt, corrected_source.c, icg.txt, optimized_icg.txt) are cleaned between requests.

### 8.2 Frontend

Single-page HTML with a left input pane and a right output panel containing five tabs:

- **TAC & Errors** — initial diagnostics, success banner if corrections were applied, then the optimized TAC. Renders error text in red when no TAC is produced.
- **Lex Tokens** — token stream from analyzer stdout.
- **Parser Tree** — text AST dump.
- **Symbol Table** — final symbol table after compilation of the corrected source.
- **Auto-Corrected** — the corrected source verbatim. Empty when has_corrections is false; the tab label gains a * marker when corrections were applied.

The execution button posts to /compile, populates each tab from the JSON response, and switches to the **TAC & Errors** tab.

---

## 9. Build Topology (Makefile)

The default target builds two binaries: analyzer.exe (from lex.yy.c, parser.tab.c, inter_code_gen.c, errors.c) and code_correc.exe (from code_correc.c alone). Flex generates lex.yy.c from lexer.l, and Bison generates parser.tab.c and parser.tab.h from parser.y. The optimize target runs python optimiser.py icg.txt --print against the most recent TAC.

CFLAGS = -Wall -Wno-int-conversion -Wno-implicit-function-declaration. The two warning suppressions accommodate generated Flex/Bison code; the project's own sources compile clean. The clean target uses Windows del; substitute rm -f on Unix.

### Run

1. make — compile both binaries.
2. python server.py — start the Flask web UI at <http://127.0.0.1:5000>.

### Clean

make clean — remove temporary and intermediate files (Flex/Bison output, executables, icg.txt, optimized_icg.txt, errors.txt, temp_source.c, corrected_source.c).

---

## 10. End-to-End Behavior

For a clean program, the analyzer emits TAC, the corrector returns byte-identical output, the server reports has_corrections = false, and the UI shows TAC with an empty Auto-Corrected tab.

For a program with fixable mistakes, the corrector produces a syntactically valid version, the analyzer re-runs on it cleanly, the optimizer rewrites the TAC, and the UI shows the corrected source alongside the optimized TAC.

For a program with errors the corrector cannot resolve (e.g. an undeclared identifier), the corrector returns an unmodified file, has_corrections stays false, the analyzer's errors are surfaced in the **TAC & Errors** tab, and no TAC is produced — preserving the contract that TAC only appears when the program is sound.

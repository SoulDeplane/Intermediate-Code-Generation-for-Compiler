# Technical Report: Compiler Pipeline, Diagnostics, and Auto-Correction

This document describes the architecture of the compiler front-end, the structured diagnostics layer shared across all phases, the auto-correction stage, the TAC optimizer, the concrete parse tree built for visualization, and the web orchestration layer that ties them together.

---

## 1. Pipeline Overview

The project is composed of independent stages that communicate through files and standard I/O.

The source file is fed into the analyzer, which performs lexical analysis, parsing, semantic checks, and intermediate code generation in a single pass. While tokenizing, the lexer also stores every token (type, text, line) into an in-memory buffer that is later used to build the concrete parse tree shown in the UI. The analyzer writes structured diagnostics to stderr and, only if no errors were recorded, writes Three-Address Code to icg.txt.

The corrector (code_correc.exe) reads the original source plus structured error rows extracted from the analyzer's stderr (missing semicolons, undeclared identifiers) and produces corrected_source.c. If the corrected source differs from the original, the analyzer is re-run on it to produce a fresh diagnostic stream and a fresh icg.txt.

Finally, the optimizer (optimiser.py) reads the latest icg.txt and writes the result to optimized_icg.txt after constant propagation, constant folding, and dead code elimination.

Two binaries are built: analyzer.exe (lex + parse + semantic + ICG + concrete-parse-tree print) and code_correc.exe (auto-corrector). The optimizer is a Python script. The web UI (server.py + index.html) drives the whole pipeline through subprocess calls and exposes results as JSON, including structured arrays for lex tokens and symbol-table rows and a structured tree for the parse tree.

---

## 2. Lexical Analysis (lexer.l)

Flex source defining the token set the parser consumes. It tokenizes identifiers, decimal/octal/hexadecimal/floating-point literals (with suffix and exponent forms), string and character literals (including the L prefix), the standard C keyword set plus modern additions (_Generic, _Alignas, _Atomic, _Noreturn, _Static_assert, _Thread_local, bool, true, false, nullptr, inline, restrict), preprocessor lines (captured as a single PREPROC token so they appear in the token stream and the parse tree without being passed to the grammar), and the full operator/punctuation set including compound assignments, comparisons, logical, bitwise, increment/decrement, arrow, and ellipsis.

Block comments are handled via a <COMMENT> start condition; line comments are skipped. Each rule maintains line and column counters and emits structured diagnostics on lexical errors:

| Code | Trigger |
|---|---|
| UNCLOSED_COMMENT | EOF while inside /* |
| UNCLOSED_STRING | newline inside string literal |
| MALFORMED_CHAR_LITERAL | malformed '...' |
| MALFORMED_NUMBER | e.g. 1.2.3 |
| ILLEGAL_CHAR | character not matched by any rule |

Every token returned by the lexer is also recorded in the global `token_buf[]` array (type, text, line) via the `RET` macro. This array is consumed later by the concrete parse-tree builder. The lexer also prints each token to stdout in a tabular form (LINE / TOKEN / TEXT columns) under the `--- Lexical Tokens ---` marker for terminal usage and emits the same data as a JSON array under `--- Lexical Tokens JSON ---` for the web UI.

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

Scopes are integer-numbered. Entering a compound_statement increments scope; leaving it walks the list and invalidates entries from that scope, emitting unused-variable and leak warnings as it goes. The unused-variable check explicitly skips `main`. The symbol table printer (`printsymtable`) skips the `main` function entry as well, since the user-facing table is only meaningful for variables, parameters, and user-defined functions. The printer emits both a tabular text version (NAME / KIND / TYPE / SCOPE / STATUS columns, with kind and type rendered as words via `sym_kind_name` / `sym_type_name`) under `--- Symbol Table ---` and a JSON array of records under `--- Symbol Table JSON ---` for the web UI.

The `insert()` routine rejects `void` only for variables, not for function declarations, so a `void main()` declaration is accepted.

### 3.3 Function Signatures

A separate function_sig linked list tracks declared functions and their parameter type lists (up to MAX_PARAMS = 32). Calls are checked against this table for argument count and type compatibility, producing ARG_COUNT_MISMATCH and ARG_TYPE_MISMATCH diagnostics.

### 3.4 AST

Built bottom-up via a tree stack (tree_stack). Each Node has left, mid1, mid2, right slots so for-loops can store init/cond/inc/body without auxiliary nodes, plus token, addr (filled in by ICG), and eval_type. `create_node(token, leaf)` either pushes a leaf or pops two children and pushes a binary node. Statement-sequence, conditional, call, and pointer nodes are constructed directly in the relevant grammar actions. Two error sentinels exist: dummy (placeholder for missing operands) and __error__ (recorded after error token productions); both are skipped by the ICG. The AST is now used internally only — for ICG generation and node lifetime — and is not displayed in the UI; the UI shows the concrete parse tree (Section 4) instead. `clear_tree` recurses into all four child slots so for-loop nodes do not leak.

### 3.5 Semantic Checks

Run inline during parsing. Codes:

| Code | Severity | Meaning |
|---|---|---|
| IMPLICIT_TRUNCATION | warning | float assigned to int |
| TYPE_MISMATCH_INIT / TYPE_MISMATCH_ASSIGN | error | char into numeric, etc. |
| UNUSED_VARIABLE | warning | declared, never is_used (excludes main) |
| LEAK_POSSIBLE | warning | is_allocated still true at scope exit |
| MISSING_RETURN | error | non-void function without return value |
| UNSAFE_API | warning | use of gets |
| ARG_COUNT_MISMATCH / ARG_TYPE_MISMATCH | error | call vs. signature |

Type mismatches in arithmetic (e.g. multiplying strings) and division-by-zero are caught in the multiplicative production rules.

### 3.6 Error Recovery

Each major nonterminal carries one or more error productions that emit plain-English `yyerror` messages and call `yyerrok` to resume parsing. This covers missing semicolons after declarations, missing or invalid type specifiers, malformed if/while conditions (missing or unmatched parentheses), and statement-level syntax errors. `yyerror` routes through `report_error` → `error_log`, so every parser complaint becomes a structured diagnostic with line and column information.

---

## 4. Concrete Parse Tree (parser.y, post-pass on token_buf)

The parse tree shown in the UI is a Concrete Syntax Tree built independently of the AST, using the token stream captured by the lexer. The leaves are the actual lex tokens (with their original text), so reading the leaves left-to-right reconstructs the source.

The CST builder is a small hand-written recursive-descent walker over `token_buf[]`:

| Function | Recognizes |
|---|---|
| cst_parse_program | top-level: preprocessor lines, function definitions, declarations |
| cst_parse_function_def | type, optional pointer stars, identifier, parameters, body |
| cst_parse_block | `{` … nested statements … `}` |
| cst_parse_statement | dispatches to if / while / for / return / declaration / expression-statement |
| cst_parse_if / while / for | keyword + condition (paren group) + body statement, plus `else` for if |
| cst_parse_paren_group | balances `(` … `)` |
| cst_parse_until_semi | consumes until matching `;` at depth zero |

Each recognized construct becomes an interior CSTNode with a meaningful label (function_definition, block, declaration, expression_statement, if_statement, for_statement, …) and its constituent tokens become leaf CSTNodes. The tree is printed twice: once as an ASCII tree (using `+--` / `\`--` / `|` connectors) under `--- Parser Tree ---` for terminal users, and once as JSON under `--- Parser Tree JSON ---` for the web UI to render visually.

---

## 5. Intermediate Code Generation (inter_code_gen.c)

A recursive AST walker that emits Three-Address Code to an open `FILE *icg_out`. The token at each AST node selects the emission rule:

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

Temporaries (t0, t1, …) and labels (L0, L1, …) are allocated by `new_temp` and `new_label`. Allocation failures route through the diagnostics layer with codes OOM_TEMP / OOM_LABEL and abort the ICG pass. The parser only opens `icg.txt` and calls `generate_ICG` if `error_has_errors()` returns false, guaranteeing TAC corresponds to a clean parse and semantic check.

---

## 6. TAC Optimizer (optimiser.py)

Three classical passes operate on the in-memory list of TAC lines:

1. **Constant Propagation** — for every `t = c` line where `c` is a constant, store `t → c` in a dictionary; later 5-token lines `t = a OP b` substitute known constants for `a` and `b`.
2. **Constant Folding** — for any 5-token line with both operands numeric, evaluate the expression and replace it with its computed value.
3. **Dead Code Elimination** — compute the set of temporaries that appear on the right-hand side of any later instruction; drop assignments to temporaries not in that set. Repeated until a fixed point (the entry point runs it twice for safety).

Output is written to `optimized_icg.txt`. With `--print`, the IR is dumped after each pass.

---

## 7. Code Auto-Correction (code_correc.c)

A standalone executable that takes `(source.c, errors.txt, corrected.c)` and produces a corrected source. Designed to be safe by default: every transformation is local, token-aware (skips strings, characters, and comments), and idempotent.

### 7.1 Per-Line Helpers

Run for each line in `apply_heuristic_fixes()` in this exact order:

1. fix_missing_hash_include — `include<x>` → `#include<x>`.
2. fix_common_header_extensions — table-driven `<stdio>` → `<stdio.h>` (16 headers, both `<x>` and `< x >` spacings).
3. normalize_smart_quotes — UTF-8 U+201C/D/18/19 → ASCII `"` / `'`.
4. fix_unclosed_string_literal — odd `"` count → close before trailing `;` or EOL. Skips lines containing `//` or `/*`.
5. `int main` injection — bare `main(...)` → `int main(...)` (does not touch `void main` or any other typed `main`).
6. replace_identifier_safe (man → main) — only when `man(` or `man (` is present.
7. replace_identifier_safe (print → printf).
8. apply_typo_table_to_line — ~110 entries covering keyword typos, stdlib-function typos, and common misspellings of `include` (so a malformed preprocessor line gets fixed in place rather than supplemented).
9. fix_format_string_missing_percent — within a recognized printf/scanf-family call, prefixes `%` to a bare format letter when the previous character is `"`, space, `:`, `=`, or `,` and the following character is `"`, `\n`, `\t`, or space. Letters covered: d, i, u, x, o, f, e, g, s, c, p (and ld, lf, lld, hd).
10. fix_wrong_terminator — trailing `,` or `:` that isn't `case X:`, `default:`, a goto label, or inside a `{...}` line.
11. fix_unbalanced_parens_at_eol — appends a single missing `)` or `]` when a call or index pattern is recognizable; bails on multi-missing or ambiguous cases.

### 7.2 File-Level Helpers

Run once after the per-line pass:

- detect_and_fix_missing_semicolons_local — appends `;` to every line `is_semicolon_candidate_line` flags. Skips lines whose next non-blank line begins with `{`, so a function header like `main()` followed by `{` on the next line is not turned into a forward declaration.
- inject_missing_headers — for each (header, function-list) pair, if any function in the list is used anywhere (token-aware via `line_uses_identifier`) and no `#include` of that header is present, prepend `#include <header>` at the top via `prepend_line`. Maps stdio.h, stdlib.h, string.h, ctype.h, math.h, time.h to ~70 functions total.

### 7.3 Errors-File-Driven Helpers

After `apply_heuristic_fixes()` returns, `main()` reads `errors.txt`, which the server populates from the analyzer's stderr:

- `MISSING_SEMICOLON <line>` rows trigger `fix_missing_semicolon`, which inserts `;` on or near the offending line (skipping function headers followed by `{`).
- `UNDECLARED_VARIABLE <line> <name>` rows trigger `inject_default_declaration`, which inserts `int <name> = 0;` indented inside `main()`'s body — provided the name is actually used in the corrected source and is not already declared. Without a `main`, the declaration is prepended to the file after any `#include`s.

After the errors-file pass, `fix_missing_closing_braces_at_eof` appends matching `}` for any unclosed open brace, `ensure_main_returns` appends `return 0;` before `main`'s closing `}` when the body lacks a return (skipped if `main` is `void`), and `inject_missing_headers` runs to add any newly required headers based on the now-corrected source.

### 7.4 The String/Char/Comment Guard

`replace_identifier_safe` carries an explicit state machine while scanning each character: in_str, in_chr, esc, in_line_comment, in_block_comment. Substitution can only happen when none of those flags are set. This prevents the corrector from touching typos inside string or character literals, typos inside line or block comments, and identifiers where the typo appears as a substring (boundary checks via `is_ident_char` on both sides of the match). `is_semicolon_candidate_line` likewise rejects lines starting with `//` or `/*`, and `line_uses_identifier` tracks the same five states.

### 7.5 Idempotence

Re-running the corrector on its own output produces byte-identical results. This matters because the web server runs the corrector unconditionally and uses a string-equality test (after CRLF normalization) to decide whether to surface the corrected source in the UI.

---

## 8. Diagnostics System (errors.h / errors.c)

A single shared layer used by every phase. The public surface exposes a phase enum (LEX, SYN, SEM, ICG, SYS), a severity enum (WARNING, ERROR, FATAL), a reset function, a setjmp registration entry point, two logging functions (one variadic, one fixed-format), an error-state query (`error_has_errors`), and a summary printer.

Each call prints a structured line to stderr containing severity, phase, line, column, machine-readable code, message, and an optional recovery hint, and bumps one of three counters. Logging a fatal triggers a longjmp to the buffer registered in the parser's `main`, letting any phase abort cleanly without leaving partially-written files.

Because the code field is stable and machine-readable (e.g. MISSING_RETURN, TYPE_MISMATCH_INIT, LEAK_POSSIBLE, ARG_TYPE_MISMATCH, UNCLOSED_STRING, PARSER_ERROR), the web server's regex extractor reliably picks out specific diagnostics — currently PARSER_ERROR lines containing `Missing semicolon` and PARSER_ERROR lines starting `Undeclared identifier` — to drive the corrector's targeted MISSING_SEMICOLON and UNDECLARED_VARIABLE rules. The same regex feeds a humanizer that rewrites every diagnostic as a plain-English sentence for the UI (for instance, an undeclared identifier becomes "Line N: 'X' is not declared. Check the spelling, or declare it before this line.").

---

## 9. Web Orchestration (server.py, index.html)

### 9.1 Backend

A Flask app exposing two routes. `GET /` serves index.html. `POST /compile` accepts a JSON `code` field and runs the full pipeline. All subprocess calls use UTF-8 encoding with `errors='replace'` so the analyzer's output (including any non-ASCII characters in user source) round-trips cleanly. The pipeline:

1. Remove any stale temp files left from a prior request.
2. Write the source to `temp_source.c`.
3. Run `analyzer.exe` with the source on stdin; capture stdout and stderr.
4. Parse stderr with the structured diagnostic regex; collect MISSING_SEMICOLON and UNDECLARED_VARIABLE rows into `errors.txt`.
5. Run `code_correc.exe temp_source.c errors.txt corrected_source.c` unconditionally. The corrector is idempotent on clean code, so this is safe.
6. Compare corrected vs. original after CRLF normalization. If different, set `has_corrections = True` and re-run the analyzer on the corrected source for the final stdout/stderr.
7. If `icg.txt` exists, run `optimiser.py icg.txt` and read `optimized_icg.txt`.
8. Extract sections from analyzer stdout: the text views (Lex Tokens, Parser Tree, Symbol Table) and the JSON views (Lex Tokens JSON, Parser Tree JSON, Symbol Table JSON), parsing the latter into Python objects.
9. Humanize both the initial and final stderr through the diagnostic regex + plain-English mapper.
10. Return JSON: tac, lex_tokens, lex_tokens_json, parser_tree, parser_tree_json, symbol_table, symbol_table_json, corrected_code, has_corrections, initial_stderr, final_stderr.

Temporary files are removed in a `finally` block so each request starts clean.

### 9.2 Frontend

Single-page HTML with an input pane and an output panel containing five tabs:

- **TAC & Errors** — initial diagnostics, success banner if corrections were applied, then the optimized TAC. All diagnostics shown here are plain-English sentences produced by the server's humanizer.
- **Lex Tokens** — rendered as a real HTML table with columns Line / Token / Text. The Line column is right-aligned numeric; the Text column uses a monospace font.
- **Parser Tree** — rendered as a visual tree built from the JSON CST: nested `<ul>`/`<li>` with CSS pseudo-elements drawing connecting branch lines, internal nodes styled in the accent color and leaves (the actual lex tokens) in monospace green.
- **Symbol Table** — rendered as a real HTML table with columns Name / Kind / Type / Scope / Status, with `main` excluded.
- **Auto-Corrected** — the corrected source verbatim. Empty when `has_corrections` is false; the tab label gains a `*` marker when corrections were applied.

The execution button posts to `/compile`, populates each tab from the JSON response, and switches to **TAC & Errors**.

The input textarea binds the Tab key locally: pressing Tab inserts eight spaces at the cursor (or indents every selected line by eight spaces); Shift+Tab outdents every selected line by up to eight spaces or one tab; Ctrl/Alt/Meta + Tab fall through to the browser default so focus navigation still works.

The layout is responsive. At widths above 900 px the input panel and output panel sit side by side. At 900 px or below, the layout collapses to a single column and an extra "Input" tab button (hidden on desktop) appears in the tab bar; clicking it shows the input panel and hides the output content area, while clicking any other tab does the reverse. The tab bar itself stays visible in both modes so the user can always switch back.

---

## 10. Build Topology (Makefile)

The default target builds two binaries: `analyzer.exe` (from `lex.yy.c`, `parser.tab.c`, `inter_code_gen.c`, `errors.c`) and `code_correc.exe` (from `code_correc.c` alone). Flex generates `lex.yy.c` from `lexer.l`, and Bison generates `parser.tab.c` and `parser.tab.h` from `parser.y`. The `optimize` target runs `python optimiser.py icg.txt --print` against the most recent TAC.

`CFLAGS = -Wall -Wno-int-conversion -Wno-implicit-function-declaration`. The two warning suppressions accommodate generated Flex/Bison code; the project's own sources compile clean. The `clean` target uses Windows `del`; substitute `rm -f` on Unix.

### Run

1. `make` — compile both binaries.
2. `python server.py` — start the Flask web UI at <http://127.0.0.1:5000>.

### Clean

`make clean` — remove temporary and intermediate files (Flex/Bison output, executables, icg.txt, optimized_icg.txt, errors.txt, temp_source.c, corrected_source.c).

---

## 11. End-to-End Behavior

For a clean program, the analyzer emits TAC, the corrector returns byte-identical output, the server reports `has_corrections = false`, and the UI shows TAC, the lex tokens table, the parse tree, the symbol table, and an empty Auto-Corrected tab.

For a program with fixable mistakes, the corrector produces a syntactically valid version (typo replacement, missing-semicolon insertion, missing-header injection, undeclared-variable defaulting inside `main`, automatic `return 0;` for a non-void `main` that lacks one), the analyzer re-runs on it cleanly, the optimizer rewrites the TAC, and the UI shows the corrected source alongside the optimized TAC, the parse tree built from the corrected token stream, and the symbol table populated from the corrected program.

For a program with errors the corrector cannot resolve, the analyzer's diagnostics are surfaced in plain English in the **TAC & Errors** tab, no TAC is produced, and the parse tree still reflects whatever structure the recursive-descent CST builder could recover from the token stream — preserving the contract that TAC only appears when the program is sound, while still giving the user as much structural feedback as possible.

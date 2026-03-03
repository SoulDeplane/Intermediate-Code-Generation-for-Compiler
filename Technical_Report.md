# Technical Report: Compiler Grammar and Error Handling
 
This document details the grammatical architecture and explicit error recovery paradigms utilized by our static analyzing compiler written in Flex and Bison.
 
## 1. Grammatical Subset & Tokens
 
### Lexical Analysis (`lexer.l`)
The tokenizer utilizes POSIX regular expressions mapping to explicit Bison (`parser.y`) tokens. It gracefully handles edge cases via specific rules:
- **Identifier**: `[a-zA-Z_][a-zA-Z0-9_]*`
- **Keywords**: Maps explicitly reserved constants (`int`, `char`, `malloc`, `for`, `if`, etc.) uniquely to Bison primitives.
- **Numbers**: Supports standard decimal (`[0-9]+`), octal (`0[0-7]+`), and hexadecimal (`0[xX][a-fA-F0-9]+`).
- **Floats**: `[0-9]*\.[0-9]*([eE][+-]?[0-9]+)?` safely extracts decimal notation logic natively.
- **Error Hooks**: Invalid or trailing strings (`L?\"([^"\\\n]|\\.)*`) missing closing quotes emit immediate feedback rather than silently discarding characters. Inverted, unrecognized characters match a `. ` fallback dropping an explicit `Lexical Error at line...`.
 
### Syntactical Grammar (`parser.y`)
The compiler digests a restricted subset of grammar mapped tightly to standard C implementation patterns, optimized for linear statement digestion across loops and assignment boundaries.
 
**Core Statements:**
* `program : translation_unit ;` serves as the entry root processing file elements consecutively.
* `statement : compound_statement | expression_statement | iteration_statement | condition_statement ;` isolates logical execution flows cleanly.
 
**Condition/Iteration Parsing:**
```yacc
iteration_statement : FOR LPAREN expression_statement expression_statement expression RPAREN statement
condition_statement : IF LPAREN relational_expression RPAREN statement
```
By mapping the elements strictly sequentially, four dynamic TAC branches inherently capture valid logic blocks for processing execution blocks.
 
**Advanced Expressions:**
* Evaluates strict algebraic hierarchy utilizing bottom-up parsing techniques spanning `additive_expression`, `multiplicative_expression`, down to `unary_expression` capturing address mappings natively (`&` and `*`).
 
## 2. Deep Semantic Analysis & Types
 
All valid expression matches return and propagate dynamic tracking wrappers (`struct node *ptr;`) dynamically up the tree natively securing strict Type compliance limits.
 
```yacc
multiplicative_expression MULT unary_expression
  if (($1->dtype == 4) || ($3->dtype == 4)) { 
      report_error(lno, "Type mismatch! Cannot perform multiplication on strings.");
  }
```
Expressions violating memory safety models explicitly halt downstream formatting logic generating friendly user alerts internally limiting silent execution breaks globally.
 
## 3. Explicit Error-Handling Mechanisms
 
Rather than throwing standard ambiguous `"Syntax Error"` warnings blocking compiling processing, deep recovery tracks intercept grammatical failure rules gracefully allowing execution models to format explicit English outputs while compiling subsequent chunks smoothly.
 
### Custom Punctuation Traps
The parser inherently predicts missed semicolons explicitly throwing user-actionable instructions naturally preventing code formatting failure loops:
```yacc
declaration : type_specifier init_declarator_list error 
   { yyerror("Missing semicolon (;) after variable declaration."); yyerrok; }
```
 
### Control Flow Mismatch Recovery
```yacc
iteration_statement : WHILE error RPAREN statement 
   { yyerror("Missing or malformed condition inside 'while'. Did you forget an opening '('?"); yyerrok; }
```
 
## 4. Static Memory & Security Warnings
 
The parser integrates active identifier evaluations mapping security warnings proactively natively across logical parsing blocks natively. 
 
- Buffer Warning Hooks: Any `.val` execution payload firing native `gets` logic blocks dynamically hooks a Warning formatting explicitly blocking deprecated execution formats natively!
- Explicit Memory Escape Diagnostics: The compiler injects explicit memory models tracing generic pointers bound sequentially through the ast using explicit `is_allocated` tags ensuring memory bounds are handled actively by developers directly tracking pointers dynamically.

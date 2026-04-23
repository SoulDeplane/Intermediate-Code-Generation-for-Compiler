%x COMMENT
%option noyywrap nounput noinput
%{
    #include <stdio.h>
    #include <stdlib.h>
    #include <string.h>
    #include "parser.tab.h"
    #include "errors.h"
    int lno = 1;
    int colno = 1;
    void printdebug(const char* msg, char c);
    #define YY_USER_ACTION colno += yyleng;
    #define RET(t) { printf("Token: %s | Text: %s\n", #t, yytext); return t; }
%}

%%
"/*"   {BEGIN(COMMENT);}
<COMMENT>\n {lno++; colno = 1;}
<COMMENT>.  ;
<COMMENT><<EOF>> { error_log(ERR_PHASE_LEX, ERR_SEV_ERROR, lno, colno, "UNCLOSED_COMMENT", "Auto-close comment at EOF and stop scanning.", "Unterminated comment. Missing closing */."); return 0; }
<COMMENT>"*/" {BEGIN(INITIAL);}
[ \t\r]+  ;
\n  {lno++; colno = 1;}
^#.* ;
\\\n {lno++; colno = 1;}
"auto"     {RET(AUTO);}
"break"    {RET(BREAK);}
"case"     {RET(CASE);}
"char"     {RET(CHAR);}
"const"    {RET(CONST);}
"continue" {RET(CONTINUE);}
"default"  {RET(DEFAULT);}
"do"       {RET(DO);}
"double"   {RET(DOUBLE);}
"else"     {RET(ELSE);}
"enum"     {RET(ENUM);}
"extern"   {RET(EXTERN);}
"float"    {RET(FLOAT);}
"for"      {RET(FOR);}
"goto"     {RET(GOTO);}
"if"       {RET(IF);}
"int"      {RET(INT);}
"long"     {RET(LONG);}
"register" {RET(REGISTER);}
"return"   {RET(RETURN);}
"short"    {RET(SHORT);}
"signed"   {RET(SIGNED);}
"sizeof"   {RET(SIZEOF);}
"static"   {RET(STATIC);}
"struct"   {RET(STRUCT);}
"switch"   {RET(SWITCH);}
"typedef"  {RET(TYPEDEF);}
"union"    {RET(UNION);}
"unsigned" {RET(UNSIGNED);}
"void"     {RET(VOID);}
"volatile" {RET(VOLATILE);}
"while"    {RET(WHILE);}
"inline"   {RET(INLINE);}
"restrict" {RET(RESTRICT);}
"_Bool"    {RET(BOOL);}
"_Complex" {RET(COMPLEX);}
"_Imaginary" {RET(IMAGINARY);}
"_Alignas"    {RET(ALIGNAS);}
"_Alignof"    {RET(ALIGNOF);}
"_Atomic"     {RET(ATOMIC);}
"_Generic"    {RET(GENERIC);}
"_Noreturn"   {RET(NORETURN);}
"_Static_assert" {RET(STATIC_ASSERT);}
"_Thread_local"  {RET(THREAD_LOCAL);}
"bool"        {RET(BOOL_KEYWORD);}
"true"        {RET(TRUE_KEYWORD);}
"false"       {RET(FALSE_KEYWORD);}
"nullptr"     {RET(NULLPTR_KEYWORD);}
"=="    {RET(EQ);}
"!="    {RET(NE);}
"<="    {RET(LE);}
">="    {RET(GE);}
"<"     {RET(LT);}
">"     {RET(GT);}
"&&"    {RET(AND);}
"||"    {RET(OR);}
"++"    {RET(INC);}
"--"    {RET(DEC);}
"+="    {RET(ADD_ASSIGN);}
"-="    {RET(SUB_ASSIGN);}
"*="    {RET(MUL_ASSIGN);}
"/="    {RET(DIV_ASSIGN);}
"%="    {RET(MOD_ASSIGN);}
"&="    {RET(AND_ASSIGN);}
"|="    {RET(OR_ASSIGN);}
"^="    {RET(XOR_ASSIGN);}
"<<="   {RET(LEFT_ASSIGN);}
">>="   {RET(RIGHT_ASSIGN);}
"<<"    {RET(LEFT_OP);}
">>"    {RET(RIGHT_OP);}
"..."   {RET(ELLIPSIS);}
"->"    {RET(ARROW);}
"="     {RET(ASSIGN);}
"+"     {RET(PLUS);}
"-"     {RET(MINUS);}
"*"     {RET(MULT);}
"/"     {RET(DIV);}
"%"     {RET(MOD);}
"&"     {RET(BIT_AND);}
"|"     {RET(BIT_OR);}
"^"     {RET(BIT_XOR);}
"~"     {RET(BIT_NOT);}
"!"     {RET(NOT);}
"?"     {RET(QUESTION);}
":"     {RET(COLON);}
";"     {RET(SEMICOLON);}
","     {RET(COMMA);}
"{"     {RET(LBRACE);}
"}"     {RET(RBRACE);}
"("     {RET(LPAREN);}
")"     {RET(RPAREN);}
"["     {RET(LBRACK);}
"]"     {RET(RBRACK);}
"."     {RET(DOT);}
L?\"([^"\\\n]|\\.)*\"     { yylval.sval = strdup(yytext); RET(STRING_LITERAL); }
L?\"([^"\\\n]|\\.)* { error_log(ERR_PHASE_LEX, ERR_SEV_ERROR, lno, colno, "UNCLOSED_STRING", "Auto-close string at line end and continue.", "Unterminated string literal."); }
L?'([^'\\\n]|\\.)+'       { yylval.sval = strdup(yytext); RET(CHAR_LITERAL); }
L?'([^'\\\n]|\\.)*        { error_log(ERR_PHASE_LEX, ERR_SEV_ERROR, lno, colno, "MALFORMED_CHAR_LITERAL", "Drop malformed char literal and continue.", "Malformed or unterminated character literal."); }
[a-zA-Z_][a-zA-Z0-9_]* { yylval.sval = strdup(yytext); RET(IDENTIFIER); }
[0-9]+\.[0-9]*\.[0-9]*   { error_log(ERR_PHASE_LEX, ERR_SEV_ERROR, lno, colno, "MALFORMED_NUMBER", "Skip malformed number token.", "Malformed floating-point literal."); }
0[xX][a-fA-F0-9]+[uUlL]* { yylval.ival = (int)strtol(yytext, NULL, 16); RET(NUMBER); }
0[0-7]+[uUlL]* { yylval.ival = (int)strtol(yytext, NULL, 8); RET(NUMBER); }
[0-9]+[uUlL]* { yylval.ival = atoi(yytext); RET(NUMBER); }
[0-9]*\.[0-9]*([eE][+-]?[0-9]+)?[fFlL]?|([0-9]+\.)(f|F|l|L)? { yylval.fval = atof(yytext); RET(FLOAT_VALUE); }
[0-9]+[eE][+-]?[0-9]+[fFlL]?            { yylval.fval = atof(yytext); RET(FLOAT_VALUE); }
<<EOF>> { return 0; }
.   { error_logf(ERR_PHASE_LEX, ERR_SEV_ERROR, lno, colno, "ILLEGAL_CHAR", "Drop illegal character and continue scanning.", "Unexpected character '%s'.", yytext); }
%%

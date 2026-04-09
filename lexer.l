%x COMMENT
%option noyywrap nounput noinput
%{
    #include <stdio.h>
    #include <stdlib.h>
    #include <string.h>
    #include "parser.tab.h"
    int lno = 1;
    void printdebug(const char* msg, char c);
    #define RET(t) { printf("Token: %s | Text: %s\n", #t, yytext); return t; }
%}

%%
"/*"   {BEGIN(COMMENT);}
<COMMENT>\n {lno++;}
<COMMENT>.  ;
<COMMENT><<EOF>> { printdebug("Unterminated comment. You forgot to close your comment with */", ' '); return 0; }
<COMMENT>"*/" {BEGIN(INITIAL);}
[ \t\r]+  ;
\n  {lno++;}
^#.* ;
\\\n {lno++;}
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
L?\"([^"\\\n]|\\.)* { printdebug("Unterminated string. You forgot to close your quotes before the end of the line.", '\"'); }
L?'([^'\\\n]|\\.)+'       { yylval.sval = strdup(yytext); RET(CHAR_LITERAL); }
[a-zA-Z_][a-zA-Z0-9_]* { yylval.sval = strdup(yytext); RET(IDENTIFIER); }
0[xX][a-fA-F0-9]+[uUlL]* { yylval.ival = (int)strtol(yytext, NULL, 16); RET(NUMBER); }
0[0-7]+[uUlL]* { yylval.ival = (int)strtol(yytext, NULL, 8); RET(NUMBER); }
[0-9]+[uUlL]* { yylval.ival = atoi(yytext); RET(NUMBER); }
[0-9]*\.[0-9]*([eE][+-]?[0-9]+)?[fFlL]?|([0-9]+\.)(f|F|l|L)? { yylval.fval = atof(yytext); RET(FLOAT_VALUE); }
[0-9]+[eE][+-]?[0-9]+[fFlL]?            { yylval.fval = atof(yytext); RET(FLOAT_VALUE); }
<<EOF>> { return 0; }
.   { printf("Lexical Error at line %d: Unexpected character '%s'. Check if you have a typo.\n", lno, yytext); }
%%

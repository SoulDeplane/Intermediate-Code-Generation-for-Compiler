%x COMMENT
%option noyywrap nounput noinput
%{
    #include <stdio.h>
    #include <stdlib.h>
    #include <string.h> 
    #include "parser.tab.h"
    int lno = 1;
    void printdebug(const char* msg, char c); 
%}

%%
"/*"    {BEGIN(COMMENT);}
<COMMENT>"*/"   {BEGIN(INITIAL);}
<COMMENT>\n {lno++;}
<COMMENT>.  ;
<COMMENT><<EOF>> { printdebug("Unterminated comment", ' '); return 0; }
"//".* ;
[ \t\r]+  ;
\n  {lno++;}
^#.* ;
\\\n {lno++;}
"auto"     {return AUTO;}
"break"    {return BREAK;}
"case"     {return CASE;}
"char"     {return CHAR;}
"const"    {return CONST;}
"continue" {return CONTINUE;}
"default"  {return DEFAULT;}
"do"       {return DO;}
"double"   {return DOUBLE;}
"else"     {return ELSE;}
"enum"     {return ENUM;}
"extern"   {return EXTERN;}
"float"    {return FLOAT;}
"for"      {return FOR;}
"goto"     {return GOTO;}
"if"       {return IF;}
"int"      {return INT;}
"long"     {return LONG;}
"register" {return REGISTER;}
"return"   {return RETURN;}
"short"    {return SHORT;}
"signed"   {return SIGNED;}
"sizeof"   {return SIZEOF;}
"static"   {return STATIC;}
"struct"   {return STRUCT;}
"switch"   {return SWITCH;}
"typedef"  {return TYPEDEF;}
"union"    {return UNION;}
"unsigned" {return UNSIGNED;}
"void"     {return VOID;}
"volatile" {return VOLATILE;}
"while"    {return WHILE;}
"inline"   {return INLINE;}
"restrict" {return RESTRICT;}
"_Bool"    {return BOOL;}
"_Complex" {return COMPLEX;}
"_Imaginary" {return IMAGINARY;}
"_Alignas"    {return ALIGNAS;}
"_Alignof"    {return ALIGNOF;}
"_Atomic"     {return ATOMIC;}
"_Generic"    {return GENERIC;}
"_Noreturn"   {return NORETURN;}
"_Static_assert" {return STATIC_ASSERT;}
"_Thread_local"  {return THREAD_LOCAL;}
"bool"        {return BOOL_KEYWORD;}
"true"        {return TRUE_KEYWORD;}
"false"       {return FALSE_KEYWORD;}
"nullptr"     {return NULLPTR_KEYWORD;}
"=="    {return EQ;}
"!="    {return NE;}
"<="    {return LE;}
">="    {return GE;}
"<"     {return LT;}
">"     {return GT;}
"&&"    {return AND;}
"||"    {return OR;}
"++"    {return INC;}
"--"    {return DEC;}
"+="    {return ADD_ASSIGN;}
"-="    {return SUB_ASSIGN;}
"*="    {return MUL_ASSIGN;}
"/="    {return DIV_ASSIGN;}
"%="    {return MOD_ASSIGN;}
"&="    {return AND_ASSIGN;}
"|="    {return OR_ASSIGN;}
"^="    {return XOR_ASSIGN;}
"<<="   {return LEFT_ASSIGN;}
">>="   {return RIGHT_ASSIGN;}
"<<"    {return LEFT_OP;}
">>"    {return RIGHT_OP;}
"..."   {return ELLIPSIS;}
"->"    {return ARROW;}
"="     {return ASSIGN;}
"+"     {return PLUS;}
"-"     {return MINUS;}
"*"     {return MULT;}
"/"     {return DIV;}
"%"     {return MOD;}
"&"     {return BIT_AND;}
"|"     {return BIT_OR;}
"^"     {return BIT_XOR;}
"~"     {return BIT_NOT;}
"!"     {return NOT;}
"?"     {return QUESTION;}
":"     {return COLON;}
";"     {return SEMICOLON;}
","     {return COMMA;}
"{"     {return LBRACE;}
"}"     {return RBRACE;}
"("     {return LPAREN;}
")"     {return RPAREN;}
"["     {return LBRACK;}
"]"     {return RBRACK;}
"."     {return DOT;}
L?\"([^"\\\n]|\\.)*\"     { yylval.sval = strdup(yytext); return STRING_LITERAL; }
L?\"([^"\\]|\\.)* { printdebug("Unterminated string", '\"'); }
L?'([^'\\\n]|\\.)+'       { yylval.sval = strdup(yytext); return CHAR_LITERAL; }
[a-zA-Z_][a-zA-Z0-9_]* { yylval.sval = strdup(yytext); return IDENTIFIER; }
0[xX][a-fA-F0-9]+[uUlL]* { yylval.ival = (int)strtol(yytext, NULL, 16); return NUMBER; }
0[0-7]+[uUlL]* { yylval.ival = (int)strtol(yytext, NULL, 8); return NUMBER; }
[0-9]+[uUlL]* { yylval.ival = atoi(yytext); return NUMBER; }
[0-9]*\.[0-9]*([eE][+-]?[0-9]+)?[fFlL]?|([0-9]+\.)(f|F|l|L)? { yylval.fval = atof(yytext); return FLOAT_VALUE; }
[0-9]+[eE][+-]?[0-9]+[fFlL]?            { yylval.fval = atof(yytext); return FLOAT_VALUE; }
<<EOF>> { return 0; }
.   {printdebug("Unknown character", yytext[0]);}
%%
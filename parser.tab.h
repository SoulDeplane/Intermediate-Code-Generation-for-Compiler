/* A Bison parser, made by GNU Bison 3.8.2.  */

/* Bison interface for Yacc-like parsers in C

   Copyright (C) 1984, 1989-1990, 2000-2015, 2018-2021 Free Software Foundation,
   Inc.

   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <https://www.gnu.org/licenses/>.  */

/* As a special exception, you may create a larger work that contains
   part or all of the Bison parser skeleton and distribute that work
   under terms of your choice, so long as that work isn't itself a
   parser generator using the skeleton or a modified version thereof
   as a parser skeleton.  Alternatively, if you modify or redistribute
   the parser skeleton itself, you may (at your option) remove this
   special exception, which will cause the skeleton and the resulting
   Bison output files to be licensed under the GNU General Public
   License without this special exception.

   This special exception was added by the Free Software Foundation in
   version 2.2 of Bison.  */

/* DO NOT RELY ON FEATURES THAT ARE NOT DOCUMENTED in the manual,
   especially those whose name start with YY_ or yy_.  They are
   private implementation details that can be changed or removed.  */

#ifndef YY_YY_PARSER_TAB_H_INCLUDED
# define YY_YY_PARSER_TAB_H_INCLUDED
/* Debug traces.  */
#ifndef YYDEBUG
# define YYDEBUG 0
#endif
#if YYDEBUG
extern int yydebug;
#endif

/* Token kinds.  */
#ifndef YYTOKENTYPE
# define YYTOKENTYPE
  enum yytokentype
  {
    YYEMPTY = -2,
    YYEOF = 0,                     /* "end of file"  */
    YYerror = 256,                 /* error  */
    YYUNDEF = 257,                 /* "invalid token"  */
    IDENTIFIER = 258,              /* IDENTIFIER  */
    STRING_LITERAL = 259,          /* STRING_LITERAL  */
    CHAR_LITERAL = 260,            /* CHAR_LITERAL  */
    NUMBER = 261,                  /* NUMBER  */
    FLOAT_VALUE = 262,             /* FLOAT_VALUE  */
    CHAR = 263,                    /* CHAR  */
    FLOAT = 264,                   /* FLOAT  */
    INT = 265,                     /* INT  */
    VOID = 266,                    /* VOID  */
    FOR = 267,                     /* FOR  */
    WHILE = 268,                   /* WHILE  */
    IF = 269,                      /* IF  */
    RETURN = 270,                  /* RETURN  */
    AUTO = 271,                    /* AUTO  */
    BREAK = 272,                   /* BREAK  */
    CASE = 273,                    /* CASE  */
    CONST = 274,                   /* CONST  */
    CONTINUE = 275,                /* CONTINUE  */
    DEFAULT = 276,                 /* DEFAULT  */
    DO = 277,                      /* DO  */
    DOUBLE = 278,                  /* DOUBLE  */
    ELSE = 279,                    /* ELSE  */
    ENUM = 280,                    /* ENUM  */
    EXTERN = 281,                  /* EXTERN  */
    GOTO = 282,                    /* GOTO  */
    LONG = 283,                    /* LONG  */
    REGISTER = 284,                /* REGISTER  */
    SHORT = 285,                   /* SHORT  */
    SIGNED = 286,                  /* SIGNED  */
    SIZEOF = 287,                  /* SIZEOF  */
    STATIC = 288,                  /* STATIC  */
    STRUCT = 289,                  /* STRUCT  */
    SWITCH = 290,                  /* SWITCH  */
    TYPEDEF = 291,                 /* TYPEDEF  */
    UNION = 292,                   /* UNION  */
    UNSIGNED = 293,                /* UNSIGNED  */
    VOLATILE = 294,                /* VOLATILE  */
    INLINE = 295,                  /* INLINE  */
    RESTRICT = 296,                /* RESTRICT  */
    BOOL = 297,                    /* BOOL  */
    COMPLEX = 298,                 /* COMPLEX  */
    IMAGINARY = 299,               /* IMAGINARY  */
    ALIGNAS = 300,                 /* ALIGNAS  */
    ALIGNOF = 301,                 /* ALIGNOF  */
    ATOMIC = 302,                  /* ATOMIC  */
    GENERIC = 303,                 /* GENERIC  */
    NORETURN = 304,                /* NORETURN  */
    STATIC_ASSERT = 305,           /* STATIC_ASSERT  */
    THREAD_LOCAL = 306,            /* THREAD_LOCAL  */
    BOOL_KEYWORD = 307,            /* BOOL_KEYWORD  */
    TRUE_KEYWORD = 308,            /* TRUE_KEYWORD  */
    FALSE_KEYWORD = 309,           /* FALSE_KEYWORD  */
    NULLPTR_KEYWORD = 310,         /* NULLPTR_KEYWORD  */
    EQ = 311,                      /* EQ  */
    NE = 312,                      /* NE  */
    LE = 313,                      /* LE  */
    GE = 314,                      /* GE  */
    INC = 315,                     /* INC  */
    DEC = 316,                     /* DEC  */
    ADD_ASSIGN = 317,              /* ADD_ASSIGN  */
    SUB_ASSIGN = 318,              /* SUB_ASSIGN  */
    MUL_ASSIGN = 319,              /* MUL_ASSIGN  */
    DIV_ASSIGN = 320,              /* DIV_ASSIGN  */
    MOD_ASSIGN = 321,              /* MOD_ASSIGN  */
    AND = 322,                     /* AND  */
    OR = 323,                      /* OR  */
    AND_ASSIGN = 324,              /* AND_ASSIGN  */
    OR_ASSIGN = 325,               /* OR_ASSIGN  */
    XOR_ASSIGN = 326,              /* XOR_ASSIGN  */
    LEFT_ASSIGN = 327,             /* LEFT_ASSIGN  */
    RIGHT_ASSIGN = 328,            /* RIGHT_ASSIGN  */
    LEFT_OP = 329,                 /* LEFT_OP  */
    RIGHT_OP = 330,                /* RIGHT_OP  */
    ELLIPSIS = 331,                /* ELLIPSIS  */
    ARROW = 332,                   /* ARROW  */
    BIT_AND = 333,                 /* BIT_AND  */
    BIT_OR = 334,                  /* BIT_OR  */
    BIT_XOR = 335,                 /* BIT_XOR  */
    ASSIGN = 336,                  /* ASSIGN  */
    PLUS = 337,                    /* PLUS  */
    MINUS = 338,                   /* MINUS  */
    MULT = 339,                    /* MULT  */
    DIV = 340,                     /* DIV  */
    MOD = 341,                     /* MOD  */
    NOT = 342,                     /* NOT  */
    BIT_NOT = 343,                 /* BIT_NOT  */
    QUESTION = 344,                /* QUESTION  */
    LT = 345,                      /* LT  */
    GT = 346,                      /* GT  */
    COLON = 347,                   /* COLON  */
    SEMICOLON = 348,               /* SEMICOLON  */
    COMMA = 349,                   /* COMMA  */
    LBRACE = 350,                  /* LBRACE  */
    RBRACE = 351,                  /* RBRACE  */
    LPAREN = 352,                  /* LPAREN  */
    RPAREN = 353,                  /* RPAREN  */
    LBRACK = 354,                  /* LBRACK  */
    RBRACK = 355,                  /* RBRACK  */
    DOT = 356                      /* DOT  */
  };
  typedef enum yytokentype yytoken_kind_t;
#endif

/* Value type.  */
#if ! defined YYSTYPE && ! defined YYSTYPE_IS_DECLARED
union YYSTYPE
{
#line 49 "parser.y"

    int ival;
    float fval;
    char *sval;
    struct node *ptr;

#line 172 "parser.tab.h"

};
typedef union YYSTYPE YYSTYPE;
# define YYSTYPE_IS_TRIVIAL 1
# define YYSTYPE_IS_DECLARED 1
#endif


extern YYSTYPE yylval;


int yyparse (void);


#endif /* !YY_YY_PARSER_TAB_H_INCLUDED  */

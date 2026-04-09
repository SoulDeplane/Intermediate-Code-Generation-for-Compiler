/* A Bison parser, made by GNU Bison 3.8.2.  */

/* Bison implementation for Yacc-like parsers in C

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

/* C LALR(1) parser skeleton written by Richard Stallman, by
   simplifying the original so-called "semantic" parser.  */

/* DO NOT RELY ON FEATURES THAT ARE NOT DOCUMENTED in the manual,
   especially those whose name start with YY_ or yy_.  They are
   private implementation details that can be changed or removed.  */

/* All symbols defined below should begin with yy or YY, to avoid
   infringing on user name space.  This should be done even for local
   variables, as they might otherwise be expanded by user macros.
   There are some unavoidable exceptions within include files to
   define necessary library symbols; they are noted "INFRINGES ON
   USER NAME SPACE" below.  */

/* Identify Bison output, and Bison version.  */
#define YYBISON 30802

/* Bison version string.  */
#define YYBISON_VERSION "3.8.2"

/* Skeleton name.  */
#define YYSKELETON_NAME "yacc.c"

/* Pure parsers.  */
#define YYPURE 0

/* Push parsers.  */
#define YYPUSH 0

/* Pull parsers.  */
#define YYPULL 1




/* First part of user prologue.  */
#line 1 "parser.y"

    #include <stdio.h>
    #include <stdlib.h>
    #include <string.h>
    #include <limits.h>
    #include "ast.h"

    void yyerror(const char*);
    int yylex();
    extern FILE * yyin, *yyout;
    extern int lno;
    FILE *icg_out = NULL;

    int scope = 0;
    int datatype = -1;
    char tempStr[100];
    int error_occurred = 0;

    struct node {
        char name[20];
        int dtype;
        int scope;
        int valid;
        int is_used;
        union value {
            float f;
            int i;
            char c;
        } val;
        int is_allocated;
        struct node *link;
    } *first = NULL;

    typedef struct tree_stack {
        Node *node;
        struct tree_stack *next;
    } tree_stack;
    tree_stack *tree_top = NULL;

    struct node* lookup(char *name);
    struct node* insert(char *name, int type);
    void printsymtable();
    void cleansymbol();

    void printAST(Node* root, int level);
    void clear_tree(Node* root);

#line 119 "parser.tab.c"

# ifndef YY_CAST
#  ifdef __cplusplus
#   define YY_CAST(Type, Val) static_cast<Type> (Val)
#   define YY_REINTERPRET_CAST(Type, Val) reinterpret_cast<Type> (Val)
#  else
#   define YY_CAST(Type, Val) ((Type) (Val))
#   define YY_REINTERPRET_CAST(Type, Val) ((Type) (Val))
#  endif
# endif
# ifndef YY_NULLPTR
#  if defined __cplusplus
#   if 201103L <= __cplusplus
#    define YY_NULLPTR nullptr
#   else
#    define YY_NULLPTR 0
#   endif
#  else
#   define YY_NULLPTR ((void*)0)
#  endif
# endif

#include "parser.tab.h"
/* Symbol kind.  */
enum yysymbol_kind_t
{
  YYSYMBOL_YYEMPTY = -2,
  YYSYMBOL_YYEOF = 0,                      /* "end of file"  */
  YYSYMBOL_YYerror = 1,                    /* error  */
  YYSYMBOL_YYUNDEF = 2,                    /* "invalid token"  */
  YYSYMBOL_IDENTIFIER = 3,                 /* IDENTIFIER  */
  YYSYMBOL_STRING_LITERAL = 4,             /* STRING_LITERAL  */
  YYSYMBOL_CHAR_LITERAL = 5,               /* CHAR_LITERAL  */
  YYSYMBOL_NUMBER = 6,                     /* NUMBER  */
  YYSYMBOL_FLOAT_VALUE = 7,                /* FLOAT_VALUE  */
  YYSYMBOL_CHAR = 8,                       /* CHAR  */
  YYSYMBOL_FLOAT = 9,                      /* FLOAT  */
  YYSYMBOL_INT = 10,                       /* INT  */
  YYSYMBOL_VOID = 11,                      /* VOID  */
  YYSYMBOL_FOR = 12,                       /* FOR  */
  YYSYMBOL_WHILE = 13,                     /* WHILE  */
  YYSYMBOL_IF = 14,                        /* IF  */
  YYSYMBOL_RETURN = 15,                    /* RETURN  */
  YYSYMBOL_AUTO = 16,                      /* AUTO  */
  YYSYMBOL_BREAK = 17,                     /* BREAK  */
  YYSYMBOL_CASE = 18,                      /* CASE  */
  YYSYMBOL_CONST = 19,                     /* CONST  */
  YYSYMBOL_CONTINUE = 20,                  /* CONTINUE  */
  YYSYMBOL_DEFAULT = 21,                   /* DEFAULT  */
  YYSYMBOL_DO = 22,                        /* DO  */
  YYSYMBOL_DOUBLE = 23,                    /* DOUBLE  */
  YYSYMBOL_ELSE = 24,                      /* ELSE  */
  YYSYMBOL_ENUM = 25,                      /* ENUM  */
  YYSYMBOL_EXTERN = 26,                    /* EXTERN  */
  YYSYMBOL_GOTO = 27,                      /* GOTO  */
  YYSYMBOL_LONG = 28,                      /* LONG  */
  YYSYMBOL_REGISTER = 29,                  /* REGISTER  */
  YYSYMBOL_SHORT = 30,                     /* SHORT  */
  YYSYMBOL_SIGNED = 31,                    /* SIGNED  */
  YYSYMBOL_SIZEOF = 32,                    /* SIZEOF  */
  YYSYMBOL_STATIC = 33,                    /* STATIC  */
  YYSYMBOL_STRUCT = 34,                    /* STRUCT  */
  YYSYMBOL_SWITCH = 35,                    /* SWITCH  */
  YYSYMBOL_TYPEDEF = 36,                   /* TYPEDEF  */
  YYSYMBOL_UNION = 37,                     /* UNION  */
  YYSYMBOL_UNSIGNED = 38,                  /* UNSIGNED  */
  YYSYMBOL_VOLATILE = 39,                  /* VOLATILE  */
  YYSYMBOL_INLINE = 40,                    /* INLINE  */
  YYSYMBOL_RESTRICT = 41,                  /* RESTRICT  */
  YYSYMBOL_BOOL = 42,                      /* BOOL  */
  YYSYMBOL_COMPLEX = 43,                   /* COMPLEX  */
  YYSYMBOL_IMAGINARY = 44,                 /* IMAGINARY  */
  YYSYMBOL_ALIGNAS = 45,                   /* ALIGNAS  */
  YYSYMBOL_ALIGNOF = 46,                   /* ALIGNOF  */
  YYSYMBOL_ATOMIC = 47,                    /* ATOMIC  */
  YYSYMBOL_GENERIC = 48,                   /* GENERIC  */
  YYSYMBOL_NORETURN = 49,                  /* NORETURN  */
  YYSYMBOL_STATIC_ASSERT = 50,             /* STATIC_ASSERT  */
  YYSYMBOL_THREAD_LOCAL = 51,              /* THREAD_LOCAL  */
  YYSYMBOL_BOOL_KEYWORD = 52,              /* BOOL_KEYWORD  */
  YYSYMBOL_TRUE_KEYWORD = 53,              /* TRUE_KEYWORD  */
  YYSYMBOL_FALSE_KEYWORD = 54,             /* FALSE_KEYWORD  */
  YYSYMBOL_NULLPTR_KEYWORD = 55,           /* NULLPTR_KEYWORD  */
  YYSYMBOL_EQ = 56,                        /* EQ  */
  YYSYMBOL_NE = 57,                        /* NE  */
  YYSYMBOL_LE = 58,                        /* LE  */
  YYSYMBOL_GE = 59,                        /* GE  */
  YYSYMBOL_INC = 60,                       /* INC  */
  YYSYMBOL_DEC = 61,                       /* DEC  */
  YYSYMBOL_ADD_ASSIGN = 62,                /* ADD_ASSIGN  */
  YYSYMBOL_SUB_ASSIGN = 63,                /* SUB_ASSIGN  */
  YYSYMBOL_MUL_ASSIGN = 64,                /* MUL_ASSIGN  */
  YYSYMBOL_DIV_ASSIGN = 65,                /* DIV_ASSIGN  */
  YYSYMBOL_MOD_ASSIGN = 66,                /* MOD_ASSIGN  */
  YYSYMBOL_AND = 67,                       /* AND  */
  YYSYMBOL_OR = 68,                        /* OR  */
  YYSYMBOL_AND_ASSIGN = 69,                /* AND_ASSIGN  */
  YYSYMBOL_OR_ASSIGN = 70,                 /* OR_ASSIGN  */
  YYSYMBOL_XOR_ASSIGN = 71,                /* XOR_ASSIGN  */
  YYSYMBOL_LEFT_ASSIGN = 72,               /* LEFT_ASSIGN  */
  YYSYMBOL_RIGHT_ASSIGN = 73,              /* RIGHT_ASSIGN  */
  YYSYMBOL_LEFT_OP = 74,                   /* LEFT_OP  */
  YYSYMBOL_RIGHT_OP = 75,                  /* RIGHT_OP  */
  YYSYMBOL_ELLIPSIS = 76,                  /* ELLIPSIS  */
  YYSYMBOL_ARROW = 77,                     /* ARROW  */
  YYSYMBOL_BIT_AND = 78,                   /* BIT_AND  */
  YYSYMBOL_BIT_OR = 79,                    /* BIT_OR  */
  YYSYMBOL_BIT_XOR = 80,                   /* BIT_XOR  */
  YYSYMBOL_ASSIGN = 81,                    /* ASSIGN  */
  YYSYMBOL_PLUS = 82,                      /* PLUS  */
  YYSYMBOL_MINUS = 83,                     /* MINUS  */
  YYSYMBOL_MULT = 84,                      /* MULT  */
  YYSYMBOL_DIV = 85,                       /* DIV  */
  YYSYMBOL_MOD = 86,                       /* MOD  */
  YYSYMBOL_NOT = 87,                       /* NOT  */
  YYSYMBOL_BIT_NOT = 88,                   /* BIT_NOT  */
  YYSYMBOL_QUESTION = 89,                  /* QUESTION  */
  YYSYMBOL_LT = 90,                        /* LT  */
  YYSYMBOL_GT = 91,                        /* GT  */
  YYSYMBOL_COLON = 92,                     /* COLON  */
  YYSYMBOL_SEMICOLON = 93,                 /* SEMICOLON  */
  YYSYMBOL_COMMA = 94,                     /* COMMA  */
  YYSYMBOL_LBRACE = 95,                    /* LBRACE  */
  YYSYMBOL_RBRACE = 96,                    /* RBRACE  */
  YYSYMBOL_LPAREN = 97,                    /* LPAREN  */
  YYSYMBOL_RPAREN = 98,                    /* RPAREN  */
  YYSYMBOL_LBRACK = 99,                    /* LBRACK  */
  YYSYMBOL_RBRACK = 100,                   /* RBRACK  */
  YYSYMBOL_DOT = 101,                      /* DOT  */
  YYSYMBOL_YYACCEPT = 102,                 /* $accept  */
  YYSYMBOL_S = 103,                        /* S  */
  YYSYMBOL_program = 104,                  /* program  */
  YYSYMBOL_translation_unit = 105,         /* translation_unit  */
  YYSYMBOL_ext_dec = 106,                  /* ext_dec  */
  YYSYMBOL_function_definition = 107,      /* function_definition  */
  YYSYMBOL_108_1 = 108,                    /* $@1  */
  YYSYMBOL_parameter_list = 109,           /* parameter_list  */
  YYSYMBOL_parameter_declaration = 110,    /* parameter_declaration  */
  YYSYMBOL_compound_statement = 111,       /* compound_statement  */
  YYSYMBOL_112_2 = 112,                    /* $@2  */
  YYSYMBOL_block_item_list = 113,          /* block_item_list  */
  YYSYMBOL_block_item = 114,               /* block_item  */
  YYSYMBOL_declaration = 115,              /* declaration  */
  YYSYMBOL_statement = 116,                /* statement  */
  YYSYMBOL_condition_statement = 117,      /* condition_statement  */
  YYSYMBOL_iteration_statement = 118,      /* iteration_statement  */
  YYSYMBOL_type_specifier = 119,           /* type_specifier  */
  YYSYMBOL_init_declarator_list = 120,     /* init_declarator_list  */
  YYSYMBOL_init_declarator = 121,          /* init_declarator  */
  YYSYMBOL_122_3 = 122,                    /* @3  */
  YYSYMBOL_123_4 = 123,                    /* @4  */
  YYSYMBOL_primary_expression = 124,       /* primary_expression  */
  YYSYMBOL_argument_expression_list = 125, /* argument_expression_list  */
  YYSYMBOL_postfix_expression = 126,       /* postfix_expression  */
  YYSYMBOL_unary_expression = 127,         /* unary_expression  */
  YYSYMBOL_multiplicative_expression = 128, /* multiplicative_expression  */
  YYSYMBOL_additive_expression = 129,      /* additive_expression  */
  YYSYMBOL_relational_expression = 130,    /* relational_expression  */
  YYSYMBOL_equality_expression = 131,      /* equality_expression  */
  YYSYMBOL_conditional_expression = 132,   /* conditional_expression  */
  YYSYMBOL_assignment_expression = 133,    /* assignment_expression  */
  YYSYMBOL_134_5 = 134,                    /* $@5  */
  YYSYMBOL_expression = 135,               /* expression  */
  YYSYMBOL_expression_statement = 136      /* expression_statement  */
};
typedef enum yysymbol_kind_t yysymbol_kind_t;




#ifdef short
# undef short
#endif

/* On compilers that do not define __PTRDIFF_MAX__ etc., make sure
   <limits.h> and (if available) <stdint.h> are included
   so that the code can choose integer types of a good width.  */

#ifndef __PTRDIFF_MAX__
# include <limits.h> /* INFRINGES ON USER NAME SPACE */
# if defined __STDC_VERSION__ && 199901 <= __STDC_VERSION__
#  include <stdint.h> /* INFRINGES ON USER NAME SPACE */
#  define YY_STDINT_H
# endif
#endif

/* Narrow types that promote to a signed type and that can represent a
   signed or unsigned integer of at least N bits.  In tables they can
   save space and decrease cache pressure.  Promoting to a signed type
   helps avoid bugs in integer arithmetic.  */

#ifdef __INT_LEAST8_MAX__
typedef __INT_LEAST8_TYPE__ yytype_int8;
#elif defined YY_STDINT_H
typedef int_least8_t yytype_int8;
#else
typedef signed char yytype_int8;
#endif

#ifdef __INT_LEAST16_MAX__
typedef __INT_LEAST16_TYPE__ yytype_int16;
#elif defined YY_STDINT_H
typedef int_least16_t yytype_int16;
#else
typedef short yytype_int16;
#endif

/* Work around bug in HP-UX 11.23, which defines these macros
   incorrectly for preprocessor constants.  This workaround can likely
   be removed in 2023, as HPE has promised support for HP-UX 11.23
   (aka HP-UX 11i v2) only through the end of 2022; see Table 2 of
   <https://h20195.www2.hpe.com/V2/getpdf.aspx/4AA4-7673ENW.pdf>.  */
#ifdef __hpux
# undef UINT_LEAST8_MAX
# undef UINT_LEAST16_MAX
# define UINT_LEAST8_MAX 255
# define UINT_LEAST16_MAX 65535
#endif

#if defined __UINT_LEAST8_MAX__ && __UINT_LEAST8_MAX__ <= __INT_MAX__
typedef __UINT_LEAST8_TYPE__ yytype_uint8;
#elif (!defined __UINT_LEAST8_MAX__ && defined YY_STDINT_H \
       && UINT_LEAST8_MAX <= INT_MAX)
typedef uint_least8_t yytype_uint8;
#elif !defined __UINT_LEAST8_MAX__ && UCHAR_MAX <= INT_MAX
typedef unsigned char yytype_uint8;
#else
typedef short yytype_uint8;
#endif

#if defined __UINT_LEAST16_MAX__ && __UINT_LEAST16_MAX__ <= __INT_MAX__
typedef __UINT_LEAST16_TYPE__ yytype_uint16;
#elif (!defined __UINT_LEAST16_MAX__ && defined YY_STDINT_H \
       && UINT_LEAST16_MAX <= INT_MAX)
typedef uint_least16_t yytype_uint16;
#elif !defined __UINT_LEAST16_MAX__ && USHRT_MAX <= INT_MAX
typedef unsigned short yytype_uint16;
#else
typedef int yytype_uint16;
#endif

#ifndef YYPTRDIFF_T
# if defined __PTRDIFF_TYPE__ && defined __PTRDIFF_MAX__
#  define YYPTRDIFF_T __PTRDIFF_TYPE__
#  define YYPTRDIFF_MAXIMUM __PTRDIFF_MAX__
# elif defined PTRDIFF_MAX
#  ifndef ptrdiff_t
#   include <stddef.h> /* INFRINGES ON USER NAME SPACE */
#  endif
#  define YYPTRDIFF_T ptrdiff_t
#  define YYPTRDIFF_MAXIMUM PTRDIFF_MAX
# else
#  define YYPTRDIFF_T long
#  define YYPTRDIFF_MAXIMUM LONG_MAX
# endif
#endif

#ifndef YYSIZE_T
# ifdef __SIZE_TYPE__
#  define YYSIZE_T __SIZE_TYPE__
# elif defined size_t
#  define YYSIZE_T size_t
# elif defined __STDC_VERSION__ && 199901 <= __STDC_VERSION__
#  include <stddef.h> /* INFRINGES ON USER NAME SPACE */
#  define YYSIZE_T size_t
# else
#  define YYSIZE_T unsigned
# endif
#endif

#define YYSIZE_MAXIMUM                                  \
  YY_CAST (YYPTRDIFF_T,                                 \
           (YYPTRDIFF_MAXIMUM < YY_CAST (YYSIZE_T, -1)  \
            ? YYPTRDIFF_MAXIMUM                         \
            : YY_CAST (YYSIZE_T, -1)))

#define YYSIZEOF(X) YY_CAST (YYPTRDIFF_T, sizeof (X))


/* Stored state numbers (used for stacks). */
typedef yytype_uint8 yy_state_t;

/* State numbers in computations.  */
typedef int yy_state_fast_t;

#ifndef YY_
# if defined YYENABLE_NLS && YYENABLE_NLS
#  if ENABLE_NLS
#   include <libintl.h> /* INFRINGES ON USER NAME SPACE */
#   define YY_(Msgid) dgettext ("bison-runtime", Msgid)
#  endif
# endif
# ifndef YY_
#  define YY_(Msgid) Msgid
# endif
#endif


#ifndef YY_ATTRIBUTE_PURE
# if defined __GNUC__ && 2 < __GNUC__ + (96 <= __GNUC_MINOR__)
#  define YY_ATTRIBUTE_PURE __attribute__ ((__pure__))
# else
#  define YY_ATTRIBUTE_PURE
# endif
#endif

#ifndef YY_ATTRIBUTE_UNUSED
# if defined __GNUC__ && 2 < __GNUC__ + (7 <= __GNUC_MINOR__)
#  define YY_ATTRIBUTE_UNUSED __attribute__ ((__unused__))
# else
#  define YY_ATTRIBUTE_UNUSED
# endif
#endif

/* Suppress unused-variable warnings by "using" E.  */
#if ! defined lint || defined __GNUC__
# define YY_USE(E) ((void) (E))
#else
# define YY_USE(E) /* empty */
#endif

/* Suppress an incorrect diagnostic about yylval being uninitialized.  */
#if defined __GNUC__ && ! defined __ICC && 406 <= __GNUC__ * 100 + __GNUC_MINOR__
# if __GNUC__ * 100 + __GNUC_MINOR__ < 407
#  define YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN                           \
    _Pragma ("GCC diagnostic push")                                     \
    _Pragma ("GCC diagnostic ignored \"-Wuninitialized\"")
# else
#  define YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN                           \
    _Pragma ("GCC diagnostic push")                                     \
    _Pragma ("GCC diagnostic ignored \"-Wuninitialized\"")              \
    _Pragma ("GCC diagnostic ignored \"-Wmaybe-uninitialized\"")
# endif
# define YY_IGNORE_MAYBE_UNINITIALIZED_END      \
    _Pragma ("GCC diagnostic pop")
#else
# define YY_INITIAL_VALUE(Value) Value
#endif
#ifndef YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
# define YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
# define YY_IGNORE_MAYBE_UNINITIALIZED_END
#endif
#ifndef YY_INITIAL_VALUE
# define YY_INITIAL_VALUE(Value) /* Nothing. */
#endif

#if defined __cplusplus && defined __GNUC__ && ! defined __ICC && 6 <= __GNUC__
# define YY_IGNORE_USELESS_CAST_BEGIN                          \
    _Pragma ("GCC diagnostic push")                            \
    _Pragma ("GCC diagnostic ignored \"-Wuseless-cast\"")
# define YY_IGNORE_USELESS_CAST_END            \
    _Pragma ("GCC diagnostic pop")
#endif
#ifndef YY_IGNORE_USELESS_CAST_BEGIN
# define YY_IGNORE_USELESS_CAST_BEGIN
# define YY_IGNORE_USELESS_CAST_END
#endif


#define YY_ASSERT(E) ((void) (0 && (E)))

#if !defined yyoverflow

/* The parser invokes alloca or malloc; define the necessary symbols.  */

# ifdef YYSTACK_USE_ALLOCA
#  if YYSTACK_USE_ALLOCA
#   ifdef __GNUC__
#    define YYSTACK_ALLOC __builtin_alloca
#   elif defined __BUILTIN_VA_ARG_INCR
#    include <alloca.h> /* INFRINGES ON USER NAME SPACE */
#   elif defined _AIX
#    define YYSTACK_ALLOC __alloca
#   elif defined _MSC_VER
#    include <malloc.h> /* INFRINGES ON USER NAME SPACE */
#    define alloca _alloca
#   else
#    define YYSTACK_ALLOC alloca
#    if ! defined _ALLOCA_H && ! defined EXIT_SUCCESS
#     include <stdlib.h> /* INFRINGES ON USER NAME SPACE */
      /* Use EXIT_SUCCESS as a witness for stdlib.h.  */
#     ifndef EXIT_SUCCESS
#      define EXIT_SUCCESS 0
#     endif
#    endif
#   endif
#  endif
# endif

# ifdef YYSTACK_ALLOC
   /* Pacify GCC's 'empty if-body' warning.  */
#  define YYSTACK_FREE(Ptr) do { /* empty */; } while (0)
#  ifndef YYSTACK_ALLOC_MAXIMUM
    /* The OS might guarantee only one guard page at the bottom of the stack,
       and a page size can be as small as 4096 bytes.  So we cannot safely
       invoke alloca (N) if N exceeds 4096.  Use a slightly smaller number
       to allow for a few compiler-allocated temporary stack slots.  */
#   define YYSTACK_ALLOC_MAXIMUM 4032 /* reasonable circa 2006 */
#  endif
# else
#  define YYSTACK_ALLOC YYMALLOC
#  define YYSTACK_FREE YYFREE
#  ifndef YYSTACK_ALLOC_MAXIMUM
#   define YYSTACK_ALLOC_MAXIMUM YYSIZE_MAXIMUM
#  endif
#  if (defined __cplusplus && ! defined EXIT_SUCCESS \
       && ! ((defined YYMALLOC || defined malloc) \
             && (defined YYFREE || defined free)))
#   include <stdlib.h> /* INFRINGES ON USER NAME SPACE */
#   ifndef EXIT_SUCCESS
#    define EXIT_SUCCESS 0
#   endif
#  endif
#  ifndef YYMALLOC
#   define YYMALLOC malloc
#   if ! defined malloc && ! defined EXIT_SUCCESS
void *malloc (YYSIZE_T); /* INFRINGES ON USER NAME SPACE */
#   endif
#  endif
#  ifndef YYFREE
#   define YYFREE free
#   if ! defined free && ! defined EXIT_SUCCESS
void free (void *); /* INFRINGES ON USER NAME SPACE */
#   endif
#  endif
# endif
#endif /* !defined yyoverflow */

#if (! defined yyoverflow \
     && (! defined __cplusplus \
         || (defined YYSTYPE_IS_TRIVIAL && YYSTYPE_IS_TRIVIAL)))

/* A type that is properly aligned for any stack member.  */
union yyalloc
{
  yy_state_t yyss_alloc;
  YYSTYPE yyvs_alloc;
};

/* The size of the maximum gap between one aligned stack and the next.  */
# define YYSTACK_GAP_MAXIMUM (YYSIZEOF (union yyalloc) - 1)

/* The size of an array large to enough to hold all stacks, each with
   N elements.  */
# define YYSTACK_BYTES(N) \
     ((N) * (YYSIZEOF (yy_state_t) + YYSIZEOF (YYSTYPE)) \
      + YYSTACK_GAP_MAXIMUM)

# define YYCOPY_NEEDED 1

/* Relocate STACK from its old location to the new one.  The
   local variables YYSIZE and YYSTACKSIZE give the old and new number of
   elements in the stack, and YYPTR gives the new location of the
   stack.  Advance YYPTR to a properly aligned location for the next
   stack.  */
# define YYSTACK_RELOCATE(Stack_alloc, Stack)                           \
    do                                                                  \
      {                                                                 \
        YYPTRDIFF_T yynewbytes;                                         \
        YYCOPY (&yyptr->Stack_alloc, Stack, yysize);                    \
        Stack = &yyptr->Stack_alloc;                                    \
        yynewbytes = yystacksize * YYSIZEOF (*Stack) + YYSTACK_GAP_MAXIMUM; \
        yyptr += yynewbytes / YYSIZEOF (*yyptr);                        \
      }                                                                 \
    while (0)

#endif

#if defined YYCOPY_NEEDED && YYCOPY_NEEDED
/* Copy COUNT objects from SRC to DST.  The source and destination do
   not overlap.  */
# ifndef YYCOPY
#  if defined __GNUC__ && 1 < __GNUC__
#   define YYCOPY(Dst, Src, Count) \
      __builtin_memcpy (Dst, Src, YY_CAST (YYSIZE_T, (Count)) * sizeof (*(Src)))
#  else
#   define YYCOPY(Dst, Src, Count)              \
      do                                        \
        {                                       \
          YYPTRDIFF_T yyi;                      \
          for (yyi = 0; yyi < (Count); yyi++)   \
            (Dst)[yyi] = (Src)[yyi];            \
        }                                       \
      while (0)
#  endif
# endif
#endif /* !YYCOPY_NEEDED */

/* YYFINAL -- State number of the termination state.  */
#define YYFINAL  60
/* YYLAST -- Last index in YYTABLE.  */
#define YYLAST   304

/* YYNTOKENS -- Number of terminals.  */
#define YYNTOKENS  102
/* YYNNTS -- Number of nonterminals.  */
#define YYNNTS  35
/* YYNRULES -- Number of rules.  */
#define YYNRULES  90
/* YYNSTATES -- Number of states.  */
#define YYNSTATES  156

/* YYMAXUTOK -- Last valid token kind.  */
#define YYMAXUTOK   356


/* YYTRANSLATE(TOKEN-NUM) -- Symbol number corresponding to TOKEN-NUM
   as returned by yylex, with out-of-bounds checking.  */
#define YYTRANSLATE(YYX)                                \
  (0 <= (YYX) && (YYX) <= YYMAXUTOK                     \
   ? YY_CAST (yysymbol_kind_t, yytranslate[YYX])        \
   : YYSYMBOL_YYUNDEF)

/* YYTRANSLATE[TOKEN-NUM] -- Symbol number corresponding to TOKEN-NUM
   as returned by yylex.  */
static const yytype_int8 yytranslate[] =
{
       0,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     1,     2,     3,     4,
       5,     6,     7,     8,     9,    10,    11,    12,    13,    14,
      15,    16,    17,    18,    19,    20,    21,    22,    23,    24,
      25,    26,    27,    28,    29,    30,    31,    32,    33,    34,
      35,    36,    37,    38,    39,    40,    41,    42,    43,    44,
      45,    46,    47,    48,    49,    50,    51,    52,    53,    54,
      55,    56,    57,    58,    59,    60,    61,    62,    63,    64,
      65,    66,    67,    68,    69,    70,    71,    72,    73,    74,
      75,    76,    77,    78,    79,    80,    81,    82,    83,    84,
      85,    86,    87,    88,    89,    90,    91,    92,    93,    94,
      95,    96,    97,    98,    99,   100,   101
};

#if YYDEBUG
/* YYRLINE[YYN] -- Source line where rule number YYN was defined.  */
static const yytype_int16 yyrline[] =
{
       0,    79,    79,   123,   124,   124,   125,   125,   125,   128,
     128,   132,   133,   134,   137,   144,   144,   160,   163,   163,
     164,   164,   165,   173,   183,   184,   185,   189,   190,   191,
     192,   193,   194,   198,   207,   208,   212,   225,   226,   235,
     236,   240,   241,   242,   243,   246,   246,   249,   249,   283,
     291,   299,   299,   322,   333,   353,   361,   369,   375,   381,
     385,   394,   412,   413,   419,   449,   490,   491,   503,   518,
     519,   523,   531,   538,   539,   543,   550,   551,   552,   553,
     554,   555,   556,   559,   560,   562,   563,   563,   597,   598,
     602
};
#endif

/** Accessing symbol of state STATE.  */
#define YY_ACCESSING_SYMBOL(State) YY_CAST (yysymbol_kind_t, yystos[State])

#if YYDEBUG || 0
/* The user-facing name of the symbol whose (internal) number is
   YYSYMBOL.  No bounds checking.  */
static const char *yysymbol_name (yysymbol_kind_t yysymbol) YY_ATTRIBUTE_UNUSED;

/* YYTNAME[SYMBOL-NUM] -- String name of the symbol SYMBOL-NUM.
   First, the terminals, then, starting at YYNTOKENS, nonterminals.  */
static const char *const yytname[] =
{
  "\"end of file\"", "error", "\"invalid token\"", "IDENTIFIER",
  "STRING_LITERAL", "CHAR_LITERAL", "NUMBER", "FLOAT_VALUE", "CHAR",
  "FLOAT", "INT", "VOID", "FOR", "WHILE", "IF", "RETURN", "AUTO", "BREAK",
  "CASE", "CONST", "CONTINUE", "DEFAULT", "DO", "DOUBLE", "ELSE", "ENUM",
  "EXTERN", "GOTO", "LONG", "REGISTER", "SHORT", "SIGNED", "SIZEOF",
  "STATIC", "STRUCT", "SWITCH", "TYPEDEF", "UNION", "UNSIGNED", "VOLATILE",
  "INLINE", "RESTRICT", "BOOL", "COMPLEX", "IMAGINARY", "ALIGNAS",
  "ALIGNOF", "ATOMIC", "GENERIC", "NORETURN", "STATIC_ASSERT",
  "THREAD_LOCAL", "BOOL_KEYWORD", "TRUE_KEYWORD", "FALSE_KEYWORD",
  "NULLPTR_KEYWORD", "EQ", "NE", "LE", "GE", "INC", "DEC", "ADD_ASSIGN",
  "SUB_ASSIGN", "MUL_ASSIGN", "DIV_ASSIGN", "MOD_ASSIGN", "AND", "OR",
  "AND_ASSIGN", "OR_ASSIGN", "XOR_ASSIGN", "LEFT_ASSIGN", "RIGHT_ASSIGN",
  "LEFT_OP", "RIGHT_OP", "ELLIPSIS", "ARROW", "BIT_AND", "BIT_OR",
  "BIT_XOR", "ASSIGN", "PLUS", "MINUS", "MULT", "DIV", "MOD", "NOT",
  "BIT_NOT", "QUESTION", "LT", "GT", "COLON", "SEMICOLON", "COMMA",
  "LBRACE", "RBRACE", "LPAREN", "RPAREN", "LBRACK", "RBRACK", "DOT",
  "$accept", "S", "program", "translation_unit", "ext_dec",
  "function_definition", "$@1", "parameter_list", "parameter_declaration",
  "compound_statement", "$@2", "block_item_list", "block_item",
  "declaration", "statement", "condition_statement", "iteration_statement",
  "type_specifier", "init_declarator_list", "init_declarator", "@3", "@4",
  "primary_expression", "argument_expression_list", "postfix_expression",
  "unary_expression", "multiplicative_expression", "additive_expression",
  "relational_expression", "equality_expression", "conditional_expression",
  "assignment_expression", "$@5", "expression", "expression_statement", YY_NULLPTR
};

static const char *
yysymbol_name (yysymbol_kind_t yysymbol)
{
  return yytname[yysymbol];
}
#endif

#define YYPACT_NINF (-81)

#define yypact_value_is_default(Yyn) \
  ((Yyn) == YYPACT_NINF)

#define YYTABLE_NINF (-87)

#define yytable_value_is_error(Yyn) \
  0

/* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
   STATE-NUM.  */
static const yytype_int16 yypact[] =
{
     157,    93,   -48,   -81,   -81,   -81,   -81,   -81,   -81,   -81,
     -81,     0,     2,     5,   193,   193,   -81,   -80,   207,    44,
     -81,    14,   -81,   -81,   -81,   -81,   -81,   -81,   -81,    33,
     -81,   -47,   -81,    -6,   -73,   -17,   -81,   -81,   -81,     7,
     -81,   -70,    52,   -81,   -59,   -81,   169,    -5,   109,   177,
     -40,   193,   -16,   193,   -14,   -81,   -81,   -81,   141,    -3,
     -81,   -81,   -43,    31,   207,   193,   193,   193,   193,   193,
     193,   193,   193,   193,   193,   193,   -81,   -81,    98,    27,
      60,   -81,    34,   -81,   -41,   -81,   207,    50,   -81,    64,
     177,   109,     3,   109,    29,   185,   125,   -81,   -81,   -81,
      34,   -81,    62,   -81,   -81,    85,   -81,   -81,   -81,    -6,
      -6,   -73,   -73,   -73,   -73,   -73,   -73,    94,   207,   114,
     -81,   207,   -81,   -81,   207,   -81,   -81,   109,   -81,   -81,
     109,   -81,   108,   -81,   -81,   234,   -81,   -81,   -81,   207,
     -81,   107,   -81,   -81,   -81,   -23,   -81,   204,   -81,   109,
     234,   120,   -81,   -81,   -81,   -81
};

/* YYDEFACT[STATE-NUM] -- Default reduction number in state STATE-NUM.
   Performed when YYTABLE does not specify something else to do.  Zero
   means the default is an error.  */
static const yytype_int8 yydefact[] =
{
       0,     0,    54,    57,    58,    55,    56,    43,    42,    41,
      44,     0,     0,     0,     0,     0,    89,    15,     0,     0,
       2,     0,     4,     7,    27,     6,     8,    30,    29,     0,
      62,    66,    69,    73,    76,    83,    84,    85,    88,     0,
      28,    49,     0,    31,     0,    45,     0,     0,     0,     0,
       0,     0,     0,     0,    54,    67,    68,    17,     0,     0,
       1,     5,    49,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    32,    90,     0,     0,
      53,    26,     0,    64,     0,    60,     0,     0,    37,     0,
       0,     0,     0,     0,     0,     0,     0,    18,    20,    21,
       0,    59,     0,    25,    24,     0,    70,    71,    72,    74,
      75,    81,    82,    77,    78,    79,    80,     0,     0,     0,
      46,     0,    65,    87,     0,    39,    40,     0,    34,    35,
       0,    23,     0,    16,    19,    11,    63,    50,    48,     0,
      61,     0,    38,    33,    22,     0,    12,     0,    52,     0,
       0,     0,    14,    36,    13,    10
};

/* YYPGOTO[NTERM-NUM].  */
static const yytype_int16 yypgoto[] =
{
     -81,   -81,   -81,   -81,   195,   -81,   -81,   -81,    67,    72,
     -81,   -81,   128,   -53,   -46,   -81,   -81,   -45,   225,   145,
     -81,   -81,   -81,   -81,   -81,   164,     1,    -7,    38,   -81,
     -81,   162,   -81,   -18,   -42
};

/* YYDEFGOTO[NTERM-NUM].  */
static const yytype_uint8 yydefgoto[] =
{
       0,    19,    20,    21,    22,    23,   102,   145,   146,    24,
      58,    96,    97,    25,    26,    27,    28,    29,    63,    45,
      79,   119,    30,    84,    31,    32,    33,    34,    35,    36,
      37,    38,    47,    39,    40
};

/* YYTABLE[YYPACT[STATE-NUM]] -- What to do in state STATE-NUM.  If
   positive, shift that token.  If negative, reduce the rule whose
   number is the opposite.  If YYTABLE_NINF, syntax error.  */
static const yytype_int16 yytable[] =
{
      59,    48,    88,    50,   126,    98,    52,    90,    76,    68,
      69,   -47,    99,   100,    -3,     1,    57,     2,     3,     4,
       5,     6,     7,     8,     9,    10,    11,    12,    13,    78,
     129,    89,   103,   -86,    81,    82,    62,    41,   -47,    70,
      71,    72,    73,    98,    60,   125,   105,   128,   124,    46,
      99,   100,    64,   121,    -9,    80,    78,   122,    91,    70,
      71,    72,    73,   111,   112,   113,   114,   115,   116,   109,
     110,   150,    89,    74,    75,   151,    86,   132,    65,    66,
      67,   142,    93,    46,   143,    70,    71,    72,    73,    92,
     147,    94,    14,    74,    75,   101,    41,    49,    15,    51,
      77,   127,    53,   153,   117,   147,   141,    16,   118,    17,
      87,    18,     2,     3,     4,     5,     6,    42,    42,    74,
      75,    11,    12,    13,   104,    82,     1,   130,     2,     3,
       4,     5,     6,     7,     8,     9,    10,    11,    12,    13,
      95,   -51,     1,    43,     2,     3,     4,     5,     6,     7,
       8,     9,    10,    11,    12,    13,    95,    77,     1,   135,
       2,     3,     4,     5,     6,     7,     8,     9,    10,    11,
      12,    13,     2,     3,     4,     5,     6,    42,    55,    56,
       2,     3,     4,     5,     6,   136,    43,    14,     2,     3,
       4,     5,     6,    15,   137,   139,    54,     3,     4,     5,
       6,   144,    16,    14,    17,   149,    18,   152,    85,    15,
       2,     3,     4,     5,     6,    17,    61,   154,    16,    14,
      17,   133,    18,   155,   134,    15,    44,   120,     0,   106,
     107,   108,     0,     0,    16,    14,    17,     0,    18,     0,
       0,    15,     7,     8,     9,    10,     0,    14,   123,     0,
      16,     0,    17,    15,    18,    14,     0,     0,     0,     0,
       0,    15,     0,    14,     0,     0,    18,    83,     0,    15,
      16,    14,     0,     0,    18,     0,     0,    15,   131,     0,
     138,     0,    18,   140,     0,    14,     0,     0,     0,     0,
      18,    15,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   148,     0,     0,    18
};

static const yytype_int16 yycheck[] =
{
      18,     1,    48,     1,     1,    58,     1,    49,     1,    82,
      83,    81,    58,    58,     0,     1,    96,     3,     4,     5,
       6,     7,     8,     9,    10,    11,    12,    13,    14,    99,
       1,    49,     1,    81,    93,    94,     3,     3,    81,    56,
      57,    58,    59,    96,     0,    91,    64,    93,    90,    97,
      96,    96,    99,    94,    97,     3,    99,    98,    98,    56,
      57,    58,    59,    70,    71,    72,    73,    74,    75,    68,
      69,    94,    90,    90,    91,    98,    81,    95,    84,    85,
      86,   127,    98,    97,   130,    56,    57,    58,    59,    51,
     135,    53,    78,    90,    91,    98,     3,    97,    84,    97,
      93,    98,    97,   149,     6,   150,   124,    93,    81,    95,
       1,    97,     3,     4,     5,     6,     7,    84,    84,    90,
      91,    12,    13,    14,    93,    94,     1,    98,     3,     4,
       5,     6,     7,     8,     9,    10,    11,    12,    13,    14,
      15,    81,     1,    93,     3,     4,     5,     6,     7,     8,
       9,    10,    11,    12,    13,    14,    15,    93,     1,    97,
       3,     4,     5,     6,     7,     8,     9,    10,    11,    12,
      13,    14,     3,     4,     5,     6,     7,    84,    14,    15,
       3,     4,     5,     6,     7,   100,    93,    78,     3,     4,
       5,     6,     7,    84,   100,    81,     3,     4,     5,     6,
       7,    93,    93,    78,    95,    98,    97,     3,    46,    84,
       3,     4,     5,     6,     7,    95,    21,   150,    93,    78,
      95,    96,    97,   151,    96,    84,     1,    82,    -1,    65,
      66,    67,    -1,    -1,    93,    78,    95,    -1,    97,    -1,
      -1,    84,     8,     9,    10,    11,    -1,    78,    86,    -1,
      93,    -1,    95,    84,    97,    78,    -1,    -1,    -1,    -1,
      -1,    84,    -1,    78,    -1,    -1,    97,    98,    -1,    84,
      93,    78,    -1,    -1,    97,    -1,    -1,    84,    93,    -1,
     118,    -1,    97,   121,    -1,    78,    -1,    -1,    -1,    -1,
      97,    84,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   139,    -1,    -1,    97
};

/* YYSTOS[STATE-NUM] -- The symbol kind of the accessing symbol of
   state STATE-NUM.  */
static const yytype_uint8 yystos[] =
{
       0,     1,     3,     4,     5,     6,     7,     8,     9,    10,
      11,    12,    13,    14,    78,    84,    93,    95,    97,   103,
     104,   105,   106,   107,   111,   115,   116,   117,   118,   119,
     124,   126,   127,   128,   129,   130,   131,   132,   133,   135,
     136,     3,    84,    93,   120,   121,    97,   134,     1,    97,
       1,    97,     1,    97,     3,   127,   127,    96,   112,   135,
       0,   106,     3,   120,    99,    84,    85,    86,    82,    83,
      56,    57,    58,    59,    90,    91,     1,    93,    99,   122,
       3,    93,    94,    98,   125,   133,    81,     1,   116,   135,
     136,    98,   130,    98,   130,    15,   113,   114,   115,   116,
     119,    98,   108,     1,    93,   135,   127,   127,   127,   128,
     128,   129,   129,   129,   129,   129,   129,     6,    81,   123,
     121,    94,    98,   133,   136,   116,     1,    98,   116,     1,
      98,    93,   135,    96,   114,    97,   100,   100,   133,    81,
     133,   135,   116,   116,    93,   109,   110,   119,   133,    98,
      94,    98,     3,   116,   110,   111
};

/* YYR1[RULE-NUM] -- Symbol kind of the left-hand side of rule RULE-NUM.  */
static const yytype_uint8 yyr1[] =
{
       0,   102,   103,   104,   105,   105,   106,   106,   106,   108,
     107,   109,   109,   109,   110,   112,   111,   111,   113,   113,
     114,   114,   114,   114,   115,   115,   115,   116,   116,   116,
     116,   116,   116,   117,   117,   117,   118,   118,   118,   118,
     118,   119,   119,   119,   119,   120,   120,   122,   121,   121,
     121,   123,   121,   121,   124,   124,   124,   124,   124,   124,
     125,   125,   126,   126,   126,   126,   127,   127,   127,   128,
     128,   128,   128,   129,   129,   129,   130,   130,   130,   130,
     130,   130,   130,   131,   132,   133,   134,   133,   135,   136,
     136
};

/* YYR2[RULE-NUM] -- Number of symbols on the right-hand side of rule RULE-NUM.  */
static const yytype_int8 yyr2[] =
{
       0,     2,     1,     1,     1,     2,     1,     1,     1,     0,
       7,     0,     1,     3,     2,     0,     4,     2,     1,     2,
       1,     1,     3,     2,     3,     3,     3,     1,     1,     1,
       1,     2,     2,     5,     4,     4,     7,     3,     5,     4,
       4,     1,     1,     1,     1,     1,     3,     0,     4,     1,
       4,     0,     5,     2,     1,     1,     1,     1,     1,     3,
       1,     3,     1,     4,     3,     4,     1,     2,     2,     1,
       3,     3,     3,     1,     3,     3,     1,     3,     3,     3,
       3,     3,     3,     1,     1,     1,     0,     4,     1,     1,
       2
};


enum { YYENOMEM = -2 };

#define yyerrok         (yyerrstatus = 0)
#define yyclearin       (yychar = YYEMPTY)

#define YYACCEPT        goto yyacceptlab
#define YYABORT         goto yyabortlab
#define YYERROR         goto yyerrorlab
#define YYNOMEM         goto yyexhaustedlab


#define YYRECOVERING()  (!!yyerrstatus)

#define YYBACKUP(Token, Value)                                    \
  do                                                              \
    if (yychar == YYEMPTY)                                        \
      {                                                           \
        yychar = (Token);                                         \
        yylval = (Value);                                         \
        YYPOPSTACK (yylen);                                       \
        yystate = *yyssp;                                         \
        goto yybackup;                                            \
      }                                                           \
    else                                                          \
      {                                                           \
        yyerror (YY_("syntax error: cannot back up")); \
        YYERROR;                                                  \
      }                                                           \
  while (0)

/* Backward compatibility with an undocumented macro.
   Use YYerror or YYUNDEF. */
#define YYERRCODE YYUNDEF


/* Enable debugging if requested.  */
#if YYDEBUG

# ifndef YYFPRINTF
#  include <stdio.h> /* INFRINGES ON USER NAME SPACE */
#  define YYFPRINTF fprintf
# endif

# define YYDPRINTF(Args)                        \
do {                                            \
  if (yydebug)                                  \
    YYFPRINTF Args;                             \
} while (0)




# define YY_SYMBOL_PRINT(Title, Kind, Value, Location)                    \
do {                                                                      \
  if (yydebug)                                                            \
    {                                                                     \
      YYFPRINTF (stderr, "%s ", Title);                                   \
      yy_symbol_print (stderr,                                            \
                  Kind, Value); \
      YYFPRINTF (stderr, "\n");                                           \
    }                                                                     \
} while (0)


/*-----------------------------------.
| Print this symbol's value on YYO.  |
`-----------------------------------*/

static void
yy_symbol_value_print (FILE *yyo,
                       yysymbol_kind_t yykind, YYSTYPE const * const yyvaluep)
{
  FILE *yyoutput = yyo;
  YY_USE (yyoutput);
  if (!yyvaluep)
    return;
  YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
  YY_USE (yykind);
  YY_IGNORE_MAYBE_UNINITIALIZED_END
}


/*---------------------------.
| Print this symbol on YYO.  |
`---------------------------*/

static void
yy_symbol_print (FILE *yyo,
                 yysymbol_kind_t yykind, YYSTYPE const * const yyvaluep)
{
  YYFPRINTF (yyo, "%s %s (",
             yykind < YYNTOKENS ? "token" : "nterm", yysymbol_name (yykind));

  yy_symbol_value_print (yyo, yykind, yyvaluep);
  YYFPRINTF (yyo, ")");
}

/*------------------------------------------------------------------.
| yy_stack_print -- Print the state stack from its BOTTOM up to its |
| TOP (included).                                                   |
`------------------------------------------------------------------*/

static void
yy_stack_print (yy_state_t *yybottom, yy_state_t *yytop)
{
  YYFPRINTF (stderr, "Stack now");
  for (; yybottom <= yytop; yybottom++)
    {
      int yybot = *yybottom;
      YYFPRINTF (stderr, " %d", yybot);
    }
  YYFPRINTF (stderr, "\n");
}

# define YY_STACK_PRINT(Bottom, Top)                            \
do {                                                            \
  if (yydebug)                                                  \
    yy_stack_print ((Bottom), (Top));                           \
} while (0)


/*------------------------------------------------.
| Report that the YYRULE is going to be reduced.  |
`------------------------------------------------*/

static void
yy_reduce_print (yy_state_t *yyssp, YYSTYPE *yyvsp,
                 int yyrule)
{
  int yylno = yyrline[yyrule];
  int yynrhs = yyr2[yyrule];
  int yyi;
  YYFPRINTF (stderr, "Reducing stack by rule %d (line %d):\n",
             yyrule - 1, yylno);
  /* The symbols being reduced.  */
  for (yyi = 0; yyi < yynrhs; yyi++)
    {
      YYFPRINTF (stderr, "   $%d = ", yyi + 1);
      yy_symbol_print (stderr,
                       YY_ACCESSING_SYMBOL (+yyssp[yyi + 1 - yynrhs]),
                       &yyvsp[(yyi + 1) - (yynrhs)]);
      YYFPRINTF (stderr, "\n");
    }
}

# define YY_REDUCE_PRINT(Rule)          \
do {                                    \
  if (yydebug)                          \
    yy_reduce_print (yyssp, yyvsp, Rule); \
} while (0)

/* Nonzero means print parse trace.  It is left uninitialized so that
   multiple parsers can coexist.  */
int yydebug;
#else /* !YYDEBUG */
# define YYDPRINTF(Args) ((void) 0)
# define YY_SYMBOL_PRINT(Title, Kind, Value, Location)
# define YY_STACK_PRINT(Bottom, Top)
# define YY_REDUCE_PRINT(Rule)
#endif /* !YYDEBUG */


/* YYINITDEPTH -- initial size of the parser's stacks.  */
#ifndef YYINITDEPTH
# define YYINITDEPTH 200
#endif

/* YYMAXDEPTH -- maximum size the stacks can grow to (effective only
   if the built-in stack extension method is used).

   Do not make this value too large; the results are undefined if
   YYSTACK_ALLOC_MAXIMUM < YYSTACK_BYTES (YYMAXDEPTH)
   evaluated with infinite-precision integer arithmetic.  */

#ifndef YYMAXDEPTH
# define YYMAXDEPTH 10000
#endif






/*-----------------------------------------------.
| Release the memory associated to this symbol.  |
`-----------------------------------------------*/

static void
yydestruct (const char *yymsg,
            yysymbol_kind_t yykind, YYSTYPE *yyvaluep)
{
  YY_USE (yyvaluep);
  if (!yymsg)
    yymsg = "Deleting";
  YY_SYMBOL_PRINT (yymsg, yykind, yyvaluep, yylocationp);

  YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
  YY_USE (yykind);
  YY_IGNORE_MAYBE_UNINITIALIZED_END
}


/* Lookahead token kind.  */
int yychar;

/* The semantic value of the lookahead symbol.  */
YYSTYPE yylval;
/* Number of syntax errors so far.  */
int yynerrs;




/*----------.
| yyparse.  |
`----------*/

int
yyparse (void)
{
    yy_state_fast_t yystate = 0;
    /* Number of tokens to shift before error messages enabled.  */
    int yyerrstatus = 0;

    /* Refer to the stacks through separate pointers, to allow yyoverflow
       to reallocate them elsewhere.  */

    /* Their size.  */
    YYPTRDIFF_T yystacksize = YYINITDEPTH;

    /* The state stack: array, bottom, top.  */
    yy_state_t yyssa[YYINITDEPTH];
    yy_state_t *yyss = yyssa;
    yy_state_t *yyssp = yyss;

    /* The semantic value stack: array, bottom, top.  */
    YYSTYPE yyvsa[YYINITDEPTH];
    YYSTYPE *yyvs = yyvsa;
    YYSTYPE *yyvsp = yyvs;

  int yyn;
  /* The return value of yyparse.  */
  int yyresult;
  /* Lookahead symbol kind.  */
  yysymbol_kind_t yytoken = YYSYMBOL_YYEMPTY;
  /* The variables used to return semantic value and location from the
     action routines.  */
  YYSTYPE yyval;



#define YYPOPSTACK(N)   (yyvsp -= (N), yyssp -= (N))

  /* The number of symbols on the RHS of the reduced rule.
     Keep to zero when no symbol should be popped.  */
  int yylen = 0;

  YYDPRINTF ((stderr, "Starting parse\n"));

  yychar = YYEMPTY; /* Cause a token to be read.  */

  goto yysetstate;


/*------------------------------------------------------------.
| yynewstate -- push a new state, which is found in yystate.  |
`------------------------------------------------------------*/
yynewstate:
  /* In all cases, when you get here, the value and location stacks
     have just been pushed.  So pushing a state here evens the stacks.  */
  yyssp++;


/*--------------------------------------------------------------------.
| yysetstate -- set current state (the top of the stack) to yystate.  |
`--------------------------------------------------------------------*/
yysetstate:
  YYDPRINTF ((stderr, "Entering state %d\n", yystate));
  YY_ASSERT (0 <= yystate && yystate < YYNSTATES);
  YY_IGNORE_USELESS_CAST_BEGIN
  *yyssp = YY_CAST (yy_state_t, yystate);
  YY_IGNORE_USELESS_CAST_END
  YY_STACK_PRINT (yyss, yyssp);

  if (yyss + yystacksize - 1 <= yyssp)
#if !defined yyoverflow && !defined YYSTACK_RELOCATE
    YYNOMEM;
#else
    {
      /* Get the current used size of the three stacks, in elements.  */
      YYPTRDIFF_T yysize = yyssp - yyss + 1;

# if defined yyoverflow
      {
        /* Give user a chance to reallocate the stack.  Use copies of
           these so that the &'s don't force the real ones into
           memory.  */
        yy_state_t *yyss1 = yyss;
        YYSTYPE *yyvs1 = yyvs;

        /* Each stack pointer address is followed by the size of the
           data in use in that stack, in bytes.  This used to be a
           conditional around just the two extra args, but that might
           be undefined if yyoverflow is a macro.  */
        yyoverflow (YY_("memory exhausted"),
                    &yyss1, yysize * YYSIZEOF (*yyssp),
                    &yyvs1, yysize * YYSIZEOF (*yyvsp),
                    &yystacksize);
        yyss = yyss1;
        yyvs = yyvs1;
      }
# else /* defined YYSTACK_RELOCATE */
      /* Extend the stack our own way.  */
      if (YYMAXDEPTH <= yystacksize)
        YYNOMEM;
      yystacksize *= 2;
      if (YYMAXDEPTH < yystacksize)
        yystacksize = YYMAXDEPTH;

      {
        yy_state_t *yyss1 = yyss;
        union yyalloc *yyptr =
          YY_CAST (union yyalloc *,
                   YYSTACK_ALLOC (YY_CAST (YYSIZE_T, YYSTACK_BYTES (yystacksize))));
        if (! yyptr)
          YYNOMEM;
        YYSTACK_RELOCATE (yyss_alloc, yyss);
        YYSTACK_RELOCATE (yyvs_alloc, yyvs);
#  undef YYSTACK_RELOCATE
        if (yyss1 != yyssa)
          YYSTACK_FREE (yyss1);
      }
# endif

      yyssp = yyss + yysize - 1;
      yyvsp = yyvs + yysize - 1;

      YY_IGNORE_USELESS_CAST_BEGIN
      YYDPRINTF ((stderr, "Stack size increased to %ld\n",
                  YY_CAST (long, yystacksize)));
      YY_IGNORE_USELESS_CAST_END

      if (yyss + yystacksize - 1 <= yyssp)
        YYABORT;
    }
#endif /* !defined yyoverflow && !defined YYSTACK_RELOCATE */


  if (yystate == YYFINAL)
    YYACCEPT;

  goto yybackup;


/*-----------.
| yybackup.  |
`-----------*/
yybackup:
  /* Do appropriate processing given the current state.  Read a
     lookahead token if we need one and don't already have one.  */

  /* First try to decide what to do without reference to lookahead token.  */
  yyn = yypact[yystate];
  if (yypact_value_is_default (yyn))
    goto yydefault;

  /* Not known => get a lookahead token if don't already have one.  */

  /* YYCHAR is either empty, or end-of-input, or a valid lookahead.  */
  if (yychar == YYEMPTY)
    {
      YYDPRINTF ((stderr, "Reading a token\n"));
      yychar = yylex ();
    }

  if (yychar <= YYEOF)
    {
      yychar = YYEOF;
      yytoken = YYSYMBOL_YYEOF;
      YYDPRINTF ((stderr, "Now at end of input.\n"));
    }
  else if (yychar == YYerror)
    {
      /* The scanner already issued an error message, process directly
         to error recovery.  But do not keep the error token as
         lookahead, it is too special and may lead us to an endless
         loop in error recovery. */
      yychar = YYUNDEF;
      yytoken = YYSYMBOL_YYerror;
      goto yyerrlab1;
    }
  else
    {
      yytoken = YYTRANSLATE (yychar);
      YY_SYMBOL_PRINT ("Next token is", yytoken, &yylval, &yylloc);
    }

  /* If the proper action on seeing token YYTOKEN is to reduce or to
     detect an error, take that action.  */
  yyn += yytoken;
  if (yyn < 0 || YYLAST < yyn || yycheck[yyn] != yytoken)
    goto yydefault;
  yyn = yytable[yyn];
  if (yyn <= 0)
    {
      if (yytable_value_is_error (yyn))
        goto yyerrlab;
      yyn = -yyn;
      goto yyreduce;
    }

  /* Count tokens shifted since error; after three, turn off error
     status.  */
  if (yyerrstatus)
    yyerrstatus--;

  /* Shift the lookahead token.  */
  YY_SYMBOL_PRINT ("Shifting", yytoken, &yylval, &yylloc);
  yystate = yyn;
  YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
  *++yyvsp = yylval;
  YY_IGNORE_MAYBE_UNINITIALIZED_END

  /* Discard the shifted token.  */
  yychar = YYEMPTY;
  goto yynewstate;


/*-----------------------------------------------------------.
| yydefault -- do the default action for the current state.  |
`-----------------------------------------------------------*/
yydefault:
  yyn = yydefact[yystate];
  if (yyn == 0)
    goto yyerrlab;
  goto yyreduce;


/*-----------------------------.
| yyreduce -- do a reduction.  |
`-----------------------------*/
yyreduce:
  /* yyn is the number of a rule to reduce with.  */
  yylen = yyr2[yyn];

  /* If YYLEN is nonzero, implement the default value of the action:
     '$$ = $1'.

     Otherwise, the following line sets YYVAL to garbage.
     This behavior is undocumented and Bison
     users should not rely upon it.  Assigning to YYVAL
     unconditionally makes the parser a bit smaller, and it avoids a
     GCC warning that YYVAL may be used uninitialized.  */
  yyval = yyvsp[1-yylen];


  YY_REDUCE_PRINT (yyn);
  switch (yyn)
    {
  case 2: /* S: program  */
#line 79 "parser.y"
            {
    if(!error_occurred) {
        printf("\n--- Intermediate Code (TAC) ---\n");
        if(tree_top != NULL) {
            icg_out = fopen("icg.txt", "w");
            if (icg_out) {
                generate_ICG(tree_top->node);
                fclose(icg_out);
                printf("Intermediate Code Successfully exported to icg.txt\n");
            } else {
                printf("Failed to open icg.txt for writing.\n");
            }
        }

        printf("\n--- Abstract Syntax Tree ---\n");
        tree_stack *temp = tree_top;
        while(temp != NULL) {
            printAST(temp->node, 0);
            temp = temp->next;
        }
    } else {
        printf("\n--- Compilation Halted Due to Errors ---\n");
    }

    while(tree_top != NULL) {
        Node *root = pop_tree();
        clear_tree(root);
    }
    struct node *ftp = first;
    while(ftp != NULL) {
        if(ftp->valid == 1) {
            if (ftp->is_used == 0) {
                fprintf(stderr, "[Friendly Compiler Notice] Warning: Variable '%s' is declared but never used. Consider removing it to clean up your code.\n", ftp->name);
            }
            if (ftp->is_allocated == 1) {
                fprintf(stderr, "[Friendly Compiler Notice] Warning: Memory leak detected! Variable '%s' was allocated but never freed before execution end.\n", ftp->name);
            }
        }
        ftp = ftp->link;
    }
    printsymtable();
    cleansymbol();
}
#line 1441 "parser.tab.c"
    break;

  case 5: /* translation_unit: translation_unit ext_dec  */
#line 124 "parser.y"
                                                      { create_node("stmt_seq", 0); }
#line 1447 "parser.tab.c"
    break;

  case 9: /* $@1: %empty  */
#line 128 "parser.y"
                                { insert((yyvsp[0].sval), datatype); }
#line 1453 "parser.tab.c"
    break;

  case 10: /* function_definition: type_specifier IDENTIFIER $@1 LPAREN parameter_list RPAREN compound_statement  */
#line 128 "parser.y"
                                                                                                          { create_node("func_def", 0); }
#line 1459 "parser.tab.c"
    break;

  case 14: /* parameter_declaration: type_specifier IDENTIFIER  */
#line 137 "parser.y"
                                                  {
    if (insert((yyvsp[0].sval), datatype) == NULL) {

    }
}
#line 1469 "parser.tab.c"
    break;

  case 15: /* $@2: %empty  */
#line 144 "parser.y"
             { scope++; }
#line 1475 "parser.tab.c"
    break;

  case 16: /* compound_statement: LBRACE $@2 block_item_list RBRACE  */
#line 144 "parser.y"
                                                 {
        struct node *ftp = first;
        while(ftp != NULL) {
            if(ftp->scope == scope && ftp->valid == 1) {
                if (ftp->is_used == 0) {
                    fprintf(stderr, "[Friendly Compiler Notice] Warning: Variable '%s' is declared but never used. Consider removing it to clean up your code.\n", ftp->name);
                }
                if (ftp->is_allocated == 1) {
                    fprintf(stderr, "[Friendly Compiler Notice] Warning: Memory leak detected! Variable '%s' was allocated but never freed before leaving scope.\n", ftp->name);
                }
                ftp->valid = 0;
            }
            ftp = ftp->link;
        }
        scope--;
    }
#line 1496 "parser.tab.c"
    break;

  case 19: /* block_item_list: block_item_list block_item  */
#line 163 "parser.y"
                                                          { create_node("stmt_seq", 0); }
#line 1502 "parser.tab.c"
    break;

  case 22: /* block_item: RETURN expression SEMICOLON  */
#line 165 "parser.y"
                                  {
        Node* expr = pop_tree();
        Node* retNode = (Node*)malloc(sizeof(Node));
        strcpy(retNode->token, "return");
        retNode->left = expr;
        retNode->right = NULL;
        push_tree(retNode);
    }
#line 1515 "parser.tab.c"
    break;

  case 23: /* block_item: RETURN SEMICOLON  */
#line 173 "parser.y"
                       {
        Node* retNode = (Node*)malloc(sizeof(Node));
        strcpy(retNode->token, "return");
        retNode->left = NULL;
        retNode->right = NULL;
        push_tree(retNode);
    }
#line 1527 "parser.tab.c"
    break;

  case 25: /* declaration: type_specifier init_declarator_list error  */
#line 184 "parser.y"
                                                { yyerror("Missing semicolon (;) after variable declaration."); yyerrok; }
#line 1533 "parser.tab.c"
    break;

  case 26: /* declaration: error init_declarator_list SEMICOLON  */
#line 185 "parser.y"
                                           { yyerror("Missing valid type specifier (int, float, etc) for declaration."); yyerrok; }
#line 1539 "parser.tab.c"
    break;

  case 31: /* statement: error SEMICOLON  */
#line 193 "parser.y"
                      { yyerror("Invalid statement; please check your syntax before the semicolon."); yyerrok; }
#line 1545 "parser.tab.c"
    break;

  case 32: /* statement: expression error  */
#line 194 "parser.y"
                       { yyerror("Missing semicolon immediately following expression block."); yyerrok; }
#line 1551 "parser.tab.c"
    break;

  case 33: /* condition_statement: IF LPAREN relational_expression RPAREN statement  */
#line 198 "parser.y"
                                                       {
        Node *stmt = pop_tree();
        Node *cond = pop_tree();
        Node *ifnode = (Node*)malloc(sizeof(Node));
        strcpy(ifnode->token, "if");
        ifnode->left = cond;
        ifnode->right = stmt;
        push_tree(ifnode);
    }
#line 1565 "parser.tab.c"
    break;

  case 34: /* condition_statement: IF error RPAREN statement  */
#line 207 "parser.y"
                                { yyerror("Missing or malformed condition inside 'if'. Did you forget an opening '('?"); yyerrok; }
#line 1571 "parser.tab.c"
    break;

  case 35: /* condition_statement: IF LPAREN relational_expression error  */
#line 208 "parser.y"
                                            { yyerror("Missing closing ')' for 'if' condition."); yyerrok; }
#line 1577 "parser.tab.c"
    break;

  case 36: /* iteration_statement: FOR LPAREN expression_statement expression_statement expression RPAREN statement  */
#line 212 "parser.y"
                                                                                       {
        Node *stmt = pop_tree();
        Node *inc = pop_tree();
        Node *cond = pop_tree();
        Node *init = pop_tree();
        Node *fornode = (Node*)malloc(sizeof(Node));
        strcpy(fornode->token, "for");
        fornode->left = init;
        fornode->mid1 = cond;
        fornode->mid2 = inc;
        fornode->right = stmt;
        push_tree(fornode);
    }
#line 1595 "parser.tab.c"
    break;

  case 37: /* iteration_statement: FOR error statement  */
#line 225 "parser.y"
                          { yyerror("Malformed 'for' loop declaration."); yyerrok; }
#line 1601 "parser.tab.c"
    break;

  case 38: /* iteration_statement: WHILE LPAREN relational_expression RPAREN statement  */
#line 226 "parser.y"
                                                          {
        Node *stmt = pop_tree();
        Node *cond = pop_tree();
        Node *whilenode = (Node*)malloc(sizeof(Node));
        strcpy(whilenode->token, "while");
        whilenode->left = cond;
        whilenode->right = stmt;
        push_tree(whilenode);
    }
#line 1615 "parser.tab.c"
    break;

  case 39: /* iteration_statement: WHILE error RPAREN statement  */
#line 235 "parser.y"
                                   { yyerror("Missing or malformed condition inside 'while'. Did you forget an opening '('?"); yyerrok; }
#line 1621 "parser.tab.c"
    break;

  case 40: /* iteration_statement: WHILE LPAREN relational_expression error  */
#line 236 "parser.y"
                                               { yyerror("Missing closing ')' for 'while' condition."); yyerrok; }
#line 1627 "parser.tab.c"
    break;

  case 41: /* type_specifier: INT  */
#line 240 "parser.y"
            { datatype = 0; }
#line 1633 "parser.tab.c"
    break;

  case 42: /* type_specifier: FLOAT  */
#line 241 "parser.y"
            { datatype = 1; }
#line 1639 "parser.tab.c"
    break;

  case 43: /* type_specifier: CHAR  */
#line 242 "parser.y"
            { datatype = 2; }
#line 1645 "parser.tab.c"
    break;

  case 44: /* type_specifier: VOID  */
#line 243 "parser.y"
            { datatype = 3; }
#line 1651 "parser.tab.c"
    break;

  case 46: /* init_declarator_list: init_declarator_list COMMA init_declarator  */
#line 246 "parser.y"
                                                                                    { create_node("stmt_seq", 0); }
#line 1657 "parser.tab.c"
    break;

  case 47: /* @3: %empty  */
#line 249 "parser.y"
                 {
        struct node* res = insert((yyvsp[0].sval), datatype);
        if (res) {
            create_node(res->name, 1);
        } else {
            create_node("dummy", 1);
        }
        (yyval.ptr) = res;
    }
#line 1671 "parser.tab.c"
    break;

  case 48: /* init_declarator: IDENTIFIER @3 ASSIGN assignment_expression  */
#line 257 "parser.y"
                                   {
        struct node* id_ptr = (yyvsp[-2].ptr);
        if (id_ptr) {
            if (id_ptr->dtype == 0 && (yyvsp[0].ptr)->dtype == 1) {
                fprintf(stderr, "[Friendly Compiler Notice] Warning at line %d: Possible loss of data. You are assigning a decimal value to the integer variable '%s'.\n", lno, id_ptr->name);
            } else if ((id_ptr->dtype == 0 || id_ptr->dtype == 1) && ((yyvsp[0].ptr)->dtype == 2)) {
                fprintf(stderr, "[Friendly Compiler Notice] Type Mismatch Error at line %d: Cannot initialize a numeric variable '%s' with a character.\n", lno, id_ptr->name);
                error_occurred = 1;
            } else if ((id_ptr->dtype == 0 || id_ptr->dtype == 1) && (yyvsp[0].ptr)->dtype == 4 && id_ptr->dtype != 4) {

            }
            if (strcmp((yyvsp[0].ptr)->name, "__malloc__") == 0) {
                id_ptr->is_allocated = 1;
            }

            if (id_ptr->dtype == 0) id_ptr->val.i = ((yyvsp[0].ptr)->dtype == 0) ? (yyvsp[0].ptr)->val.i : (int)(yyvsp[0].ptr)->val.f;
            else if (id_ptr->dtype == 1) id_ptr->val.f = ((yyvsp[0].ptr)->dtype == 1) ? (yyvsp[0].ptr)->val.f : (float)(yyvsp[0].ptr)->val.i;
            create_node("=", 0);
        } else {
            Node* expr = pop_tree();
            Node* dummy = pop_tree();
            clear_tree(expr);
            clear_tree(dummy);
            create_node("dummy", 1);
        }
    }
#line 1702 "parser.tab.c"
    break;

  case 49: /* init_declarator: IDENTIFIER  */
#line 283 "parser.y"
                 {
        struct node* res = insert((yyvsp[0].sval), datatype);
        if (res) {
            create_node(res->name, 1);
        } else {
            create_node("dummy", 1);
        }
    }
#line 1715 "parser.tab.c"
    break;

  case 50: /* init_declarator: IDENTIFIER LBRACK NUMBER RBRACK  */
#line 291 "parser.y"
                                      {
        struct node* res = insert((yyvsp[-3].sval), datatype);
        if (res) {
            create_node(res->name, 1);
        } else {
            create_node("dummy", 1);
        }
    }
#line 1728 "parser.tab.c"
    break;

  case 51: /* @4: %empty  */
#line 299 "parser.y"
                      {
        struct node* res = insert((yyvsp[0].sval), 4);
        if (res) {
            create_node(res->name, 1);
        } else {
            create_node("dummy", 1);
        }
        (yyval.ptr) = res;
    }
#line 1742 "parser.tab.c"
    break;

  case 52: /* init_declarator: MULT IDENTIFIER @4 ASSIGN assignment_expression  */
#line 307 "parser.y"
                                   {
        struct node* id_ptr = (yyvsp[-3].ptr);
        if (id_ptr) {
            if (strcmp((yyvsp[-1].ptr)->name, "__malloc__") == 0) {
                id_ptr->is_allocated = 1;
            }
            create_node("=", 0);
        } else {
            Node* expr = pop_tree();
            Node* dummy = pop_tree();
            clear_tree(expr);
            clear_tree(dummy);
            create_node("dummy", 1);
        }
    }
#line 1762 "parser.tab.c"
    break;

  case 53: /* init_declarator: MULT IDENTIFIER  */
#line 322 "parser.y"
                      {
        struct node* res = insert((yyvsp[0].sval), 4);
        if (res) {
            create_node(res->name, 1);
        } else {
            create_node("dummy", 1);
        }
    }
#line 1775 "parser.tab.c"
    break;

  case 54: /* primary_expression: IDENTIFIER  */
#line 333 "parser.y"
                 {
        struct node* res = lookup((yyvsp[0].sval));
        if(!res) {
            char errmsg[50];
            sprintf(errmsg, "Undeclared identifier %s", (yyvsp[0].sval));
            report_error(lno, errmsg);

            struct node* dummy = (struct node*)malloc(sizeof(struct node));
            dummy->dtype = 0; dummy->val.i = 0; strcpy(dummy->name, "dummy");
            (yyval.ptr) = dummy;
            create_node("dummy", 1);
        } else {
            res->is_used = 1;

            struct node* ret = (struct node*)malloc(sizeof(struct node));
            memcpy(ret, res, sizeof(struct node));
            (yyval.ptr) = ret;
            create_node(res->name, 1);
        }
    }
#line 1800 "parser.tab.c"
    break;

  case 55: /* primary_expression: NUMBER  */
#line 353 "parser.y"
             {
        struct node* num_node = (struct node*)malloc(sizeof(struct node));
        num_node->dtype = 0;
        num_node->val.i = (yyvsp[0].ival);
        (yyval.ptr) = num_node;
        sprintf(tempStr, "%d", (yyvsp[0].ival));
        create_node(tempStr, 1);
    }
#line 1813 "parser.tab.c"
    break;

  case 56: /* primary_expression: FLOAT_VALUE  */
#line 361 "parser.y"
                  {
        struct node* f_node = (struct node*)malloc(sizeof(struct node));
        f_node->dtype = 1;
        f_node->val.f = (yyvsp[0].fval);
        (yyval.ptr) = f_node;
        sprintf(tempStr, "%f", (yyvsp[0].fval));
        create_node(tempStr, 1);
    }
#line 1826 "parser.tab.c"
    break;

  case 57: /* primary_expression: STRING_LITERAL  */
#line 369 "parser.y"
                     {
        struct node* s_node = (struct node*)malloc(sizeof(struct node));
        s_node->dtype = 4;
        (yyval.ptr) = s_node;
        create_node("string_lit", 1);
    }
#line 1837 "parser.tab.c"
    break;

  case 58: /* primary_expression: CHAR_LITERAL  */
#line 375 "parser.y"
                   {
        struct node* c_node = (struct node*)malloc(sizeof(struct node));
        c_node->dtype = 2;
        (yyval.ptr) = c_node;
        create_node("char_lit", 1);
    }
#line 1848 "parser.tab.c"
    break;

  case 59: /* primary_expression: LPAREN expression RPAREN  */
#line 381 "parser.y"
                               { (yyval.ptr) = (yyvsp[-1].ptr); }
#line 1854 "parser.tab.c"
    break;

  case 60: /* argument_expression_list: assignment_expression  */
#line 385 "parser.y"
                            {
        Node* expr = pop_tree();
        Node* paramNode = (Node*)malloc(sizeof(Node));
        strcpy(paramNode->token, "param");
        paramNode->left = expr;
        paramNode->right = NULL;
        push_tree(paramNode);
        (yyval.ptr) = (yyvsp[0].ptr);
    }
#line 1868 "parser.tab.c"
    break;

  case 61: /* argument_expression_list: argument_expression_list COMMA assignment_expression  */
#line 394 "parser.y"
                                                           {
        Node* expr = pop_tree();
        Node* list = pop_tree();
        Node* paramNode = (Node*)malloc(sizeof(Node));
        strcpy(paramNode->token, "param");
        paramNode->left = expr;
        paramNode->right = NULL;

        Node* seq = (Node*)malloc(sizeof(Node));
        strcpy(seq->token, "param_list");
        seq->left = list;
        seq->right = paramNode;
        push_tree(seq);
        (yyval.ptr) = (yyvsp[-2].ptr);
    }
#line 1888 "parser.tab.c"
    break;

  case 63: /* postfix_expression: postfix_expression LBRACK expression RBRACK  */
#line 413 "parser.y"
                                                  {
        struct node* arr_node = (struct node*)malloc(sizeof(struct node));
        arr_node->dtype = 0;
        (yyval.ptr) = arr_node;
        create_node("[]", 0);
    }
#line 1899 "parser.tab.c"
    break;

  case 64: /* postfix_expression: IDENTIFIER LPAREN RPAREN  */
#line 419 "parser.y"
                               {
        if (strcmp((yyvsp[-2].sval), "gets") == 0) {
            fprintf(stderr, "[Friendly Compiler Notice] Warning at line %d: '%s' is unsafe; it can lead to buffer overflows. Consider using 'fgets' instead.\n", lno, (yyvsp[-2].sval));
        }

        struct node* res = lookup((yyvsp[-2].sval));
        if(!res &&
           strcmp((yyvsp[-2].sval), "gets") != 0 && strcmp((yyvsp[-2].sval), "malloc") != 0 && strcmp((yyvsp[-2].sval), "free") != 0 &&
           strcmp((yyvsp[-2].sval), "printf") != 0 && strcmp((yyvsp[-2].sval), "scanf") != 0) {
            char errmsg[50];
            sprintf(errmsg, "Undeclared identifier %s", (yyvsp[-2].sval));
            report_error(lno, errmsg);
        }

        Node* callNode = (Node*)malloc(sizeof(Node));
        strcpy(callNode->token, "call");

        Node* funcNode = (Node*)malloc(sizeof(Node));
        strcpy(funcNode->token, (yyvsp[-2].sval));
        funcNode->left = funcNode->right = NULL;

        callNode->left = funcNode;
        callNode->right = NULL;
        push_tree(callNode);

        struct node* call_res = (struct node*)malloc(sizeof(struct node));
        if (strcmp((yyvsp[-2].sval), "malloc") == 0) strcpy(call_res->name, "__malloc__");
        call_res->dtype = 0;
        (yyval.ptr) = call_res;
    }
#line 1934 "parser.tab.c"
    break;

  case 65: /* postfix_expression: IDENTIFIER LPAREN argument_expression_list RPAREN  */
#line 449 "parser.y"
                                                        {
        if (strcmp((yyvsp[-3].sval), "gets") == 0) {
            fprintf(stderr, "[Friendly Compiler Notice] Warning at line %d: '%s' is unsafe; it can lead to buffer overflows. Consider using 'fgets' instead.\n", lno, (yyvsp[-3].sval));
        }

        if (strcmp((yyvsp[-3].sval), "free") == 0) {
            if ((yyvsp[-1].ptr) && (yyvsp[-1].ptr)->is_allocated == 1) {

                struct node* free_target = lookup((yyvsp[-1].ptr)->name);
                if (free_target) free_target->is_allocated = 0;
            }
        }

        struct node* res = lookup((yyvsp[-3].sval));
        if(!res &&
           strcmp((yyvsp[-3].sval), "gets") != 0 && strcmp((yyvsp[-3].sval), "malloc") != 0 && strcmp((yyvsp[-3].sval), "free") != 0 &&
           strcmp((yyvsp[-3].sval), "printf") != 0 && strcmp((yyvsp[-3].sval), "scanf") != 0) {
            char errmsg[50];
            sprintf(errmsg, "Undeclared identifier %s", (yyvsp[-3].sval));
            report_error(lno, errmsg);
        }
        Node* args = pop_tree();
        Node* callNode = (Node*)malloc(sizeof(Node));
        strcpy(callNode->token, "call");

        Node* funcNode = (Node*)malloc(sizeof(Node));
        strcpy(funcNode->token, (yyvsp[-3].sval));
        funcNode->left = funcNode->right = NULL;

        callNode->left = funcNode;
        callNode->right = args;
        push_tree(callNode);

        struct node* call_res = (struct node*)malloc(sizeof(struct node));
        if (strcmp((yyvsp[-3].sval), "malloc") == 0) strcpy(call_res->name, "__malloc__");
        call_res->dtype = 0;
        (yyval.ptr) = call_res;
    }
#line 1977 "parser.tab.c"
    break;

  case 67: /* unary_expression: BIT_AND unary_expression  */
#line 491 "parser.y"
                               {
         struct node* res = (struct node*)malloc(sizeof(struct node));
         res->dtype = 4;
         (yyval.ptr) = res;

         Node* operand = pop_tree();
         Node* opNode = (Node*)malloc(sizeof(Node));
         strcpy(opNode->token, "addr");
         opNode->left = operand;
         opNode->right = NULL;
         push_tree(opNode);
    }
#line 1994 "parser.tab.c"
    break;

  case 68: /* unary_expression: MULT unary_expression  */
#line 503 "parser.y"
                            {
         struct node* res = (struct node*)malloc(sizeof(struct node));
         res->dtype = 4;
         (yyval.ptr) = res;

         Node* operand = pop_tree();
         Node* opNode = (Node*)malloc(sizeof(Node));
         strcpy(opNode->token, "deref");
         opNode->left = operand;
         opNode->right = NULL;
         push_tree(opNode);
    }
#line 2011 "parser.tab.c"
    break;

  case 70: /* multiplicative_expression: multiplicative_expression MULT unary_expression  */
#line 519 "parser.y"
                                                      {
        if (((yyvsp[-2].ptr)->dtype == 4 || (yyvsp[-2].ptr)->dtype == 2) || ((yyvsp[0].ptr)->dtype == 4 || (yyvsp[0].ptr)->dtype == 2)) { report_error(lno, "Type mismatch! Cannot perform multiplication on strings or characters."); }
        (yyval.ptr) = (struct node*)malloc(sizeof(struct node)); (yyval.ptr)->dtype = 0; create_node("*", 0);
    }
#line 2020 "parser.tab.c"
    break;

  case 71: /* multiplicative_expression: multiplicative_expression DIV unary_expression  */
#line 523 "parser.y"
                                                     {
        if (((yyvsp[-2].ptr)->dtype == 4 || (yyvsp[-2].ptr)->dtype == 2) || ((yyvsp[0].ptr)->dtype == 4 || (yyvsp[0].ptr)->dtype == 2)) { report_error(lno, "Type mismatch! Cannot perform division on strings or characters."); }
        float rhs = ((yyvsp[0].ptr)->dtype == 0) ? (float)(yyvsp[0].ptr)->val.i : (yyvsp[0].ptr)->val.f;
        if(rhs == 0.0) {
            report_error(lno, "Division by zero");
        }
        (yyval.ptr) = (struct node*)malloc(sizeof(struct node)); (yyval.ptr)->dtype = 0; create_node("/", 0);
    }
#line 2033 "parser.tab.c"
    break;

  case 72: /* multiplicative_expression: multiplicative_expression MOD unary_expression  */
#line 531 "parser.y"
                                                     {
        if (((yyvsp[-2].ptr)->dtype == 4 || (yyvsp[-2].ptr)->dtype == 2) || ((yyvsp[0].ptr)->dtype == 4 || (yyvsp[0].ptr)->dtype == 2)) { report_error(lno, "Type mismatch! Cannot modulo strings or characters."); }
        (yyval.ptr) = (struct node*)malloc(sizeof(struct node)); (yyval.ptr)->dtype = 0; create_node("%", 0);
    }
#line 2042 "parser.tab.c"
    break;

  case 74: /* additive_expression: additive_expression PLUS multiplicative_expression  */
#line 539 "parser.y"
                                                         {
        if (((yyvsp[-2].ptr)->dtype == 4 || (yyvsp[-2].ptr)->dtype == 2) || ((yyvsp[0].ptr)->dtype == 4 || (yyvsp[0].ptr)->dtype == 2)) { report_error(lno, "Type mismatch! Cannot explicitly add strings or characters."); }
        (yyval.ptr) = (struct node*)malloc(sizeof(struct node)); (yyval.ptr)->dtype = 0; create_node("+", 0);
    }
#line 2051 "parser.tab.c"
    break;

  case 75: /* additive_expression: additive_expression MINUS multiplicative_expression  */
#line 543 "parser.y"
                                                          {
        if (((yyvsp[-2].ptr)->dtype == 4 || (yyvsp[-2].ptr)->dtype == 2) || ((yyvsp[0].ptr)->dtype == 4 || (yyvsp[0].ptr)->dtype == 2)) { report_error(lno, "Type mismatch! Cannot explicitly subtract strings or characters."); }
        (yyval.ptr) = (struct node*)malloc(sizeof(struct node)); (yyval.ptr)->dtype = 0; create_node("-", 0);
    }
#line 2060 "parser.tab.c"
    break;

  case 77: /* relational_expression: relational_expression LE additive_expression  */
#line 551 "parser.y"
                                                   { (yyval.ptr) = (struct node*)malloc(sizeof(struct node)); (yyval.ptr)->dtype = 0; create_node("<=", 0); }
#line 2066 "parser.tab.c"
    break;

  case 78: /* relational_expression: relational_expression GE additive_expression  */
#line 552 "parser.y"
                                                   { (yyval.ptr) = (struct node*)malloc(sizeof(struct node)); (yyval.ptr)->dtype = 0; create_node(">=", 0); }
#line 2072 "parser.tab.c"
    break;

  case 79: /* relational_expression: relational_expression LT additive_expression  */
#line 553 "parser.y"
                                                   { (yyval.ptr) = (struct node*)malloc(sizeof(struct node)); (yyval.ptr)->dtype = 0; create_node("<", 0); }
#line 2078 "parser.tab.c"
    break;

  case 80: /* relational_expression: relational_expression GT additive_expression  */
#line 554 "parser.y"
                                                   { (yyval.ptr) = (struct node*)malloc(sizeof(struct node)); (yyval.ptr)->dtype = 0; create_node(">", 0); }
#line 2084 "parser.tab.c"
    break;

  case 81: /* relational_expression: relational_expression EQ additive_expression  */
#line 555 "parser.y"
                                                   { (yyval.ptr) = (struct node*)malloc(sizeof(struct node)); (yyval.ptr)->dtype = 0; create_node("==", 0); }
#line 2090 "parser.tab.c"
    break;

  case 82: /* relational_expression: relational_expression NE additive_expression  */
#line 556 "parser.y"
                                                   { (yyval.ptr) = (struct node*)malloc(sizeof(struct node)); (yyval.ptr)->dtype = 0; create_node("!=", 0); }
#line 2096 "parser.tab.c"
    break;

  case 86: /* $@5: %empty  */
#line 563 "parser.y"
                 { create_node((yyvsp[0].sval), 1); }
#line 2102 "parser.tab.c"
    break;

  case 87: /* assignment_expression: IDENTIFIER $@5 ASSIGN assignment_expression  */
#line 563 "parser.y"
                                                                      {
        struct node* res = lookup((yyvsp[-3].sval));
        if(!res) {
            char errmsg[50];
            sprintf(errmsg, "Undeclared identifier %s", (yyvsp[-3].sval));
            report_error(lno, errmsg);

            struct node* dummy = (struct node*)malloc(sizeof(struct node));
            dummy->dtype = 0; dummy->val.i = 0; strcpy(dummy->name, "dummy");
            (yyval.ptr) = dummy;
        } else {
            if (res->dtype == 0 && (yyvsp[0].ptr)->dtype == 1) {
                fprintf(stderr, "[Friendly Compiler Notice] Warning at line %d: Possible loss of data. You are assigning a decimal value to the integer variable '%s'.\n", lno, res->name);
            } else if ((res->dtype == 0 || res->dtype == 1) && ((yyvsp[0].ptr)->dtype == 2)) {
                fprintf(stderr, "[Friendly Compiler Notice] Type Mismatch Error at line %d: Cannot assign a character to a numeric variable '%s'.\n", lno, res->name);
                error_occurred = 1;
            } else if ((res->dtype == 0 || res->dtype == 1) && (yyvsp[0].ptr)->dtype == 4 && res->dtype != 4) {

            }
            if (strcmp((yyvsp[0].ptr)->name, "__malloc__") == 0) {
                res->is_allocated = 1;
            }
            if ((yyvsp[0].ptr)->is_allocated == 1 && strcmp((yyvsp[0].ptr)->name, res->name) != 0) {

                res->is_allocated = 1;
            }

            if(res->dtype == 0) res->val.i = ((yyvsp[0].ptr)->dtype == 0) ? (yyvsp[0].ptr)->val.i : (int)(yyvsp[0].ptr)->val.f;
            else if(res->dtype == 1) res->val.f = ((yyvsp[0].ptr)->dtype == 1) ? (yyvsp[0].ptr)->val.f : (float)(yyvsp[0].ptr)->val.i;
            (yyval.ptr) = (yyvsp[0].ptr);
        }
        create_node("=", 0);
    }
#line 2140 "parser.tab.c"
    break;

  case 89: /* expression_statement: SEMICOLON  */
#line 598 "parser.y"
                                 {
    struct node* dummy = (struct node*)malloc(sizeof(struct node));
    dummy->dtype = 0; dummy->val.i = 0; strcpy(dummy->name, "dummy");
    (yyval.ptr) = dummy;
}
#line 2150 "parser.tab.c"
    break;

  case 90: /* expression_statement: expression SEMICOLON  */
#line 602 "parser.y"
                         { (yyval.ptr) = (yyvsp[-1].ptr); }
#line 2156 "parser.tab.c"
    break;


#line 2160 "parser.tab.c"

      default: break;
    }
  /* User semantic actions sometimes alter yychar, and that requires
     that yytoken be updated with the new translation.  We take the
     approach of translating immediately before every use of yytoken.
     One alternative is translating here after every semantic action,
     but that translation would be missed if the semantic action invokes
     YYABORT, YYACCEPT, or YYERROR immediately after altering yychar or
     if it invokes YYBACKUP.  In the case of YYABORT or YYACCEPT, an
     incorrect destructor might then be invoked immediately.  In the
     case of YYERROR or YYBACKUP, subsequent parser actions might lead
     to an incorrect destructor call or verbose syntax error message
     before the lookahead is translated.  */
  YY_SYMBOL_PRINT ("-> $$ =", YY_CAST (yysymbol_kind_t, yyr1[yyn]), &yyval, &yyloc);

  YYPOPSTACK (yylen);
  yylen = 0;

  *++yyvsp = yyval;

  /* Now 'shift' the result of the reduction.  Determine what state
     that goes to, based on the state we popped back to and the rule
     number reduced by.  */
  {
    const int yylhs = yyr1[yyn] - YYNTOKENS;
    const int yyi = yypgoto[yylhs] + *yyssp;
    yystate = (0 <= yyi && yyi <= YYLAST && yycheck[yyi] == *yyssp
               ? yytable[yyi]
               : yydefgoto[yylhs]);
  }

  goto yynewstate;


/*--------------------------------------.
| yyerrlab -- here on detecting error.  |
`--------------------------------------*/
yyerrlab:
  /* Make sure we have latest lookahead translation.  See comments at
     user semantic actions for why this is necessary.  */
  yytoken = yychar == YYEMPTY ? YYSYMBOL_YYEMPTY : YYTRANSLATE (yychar);
  /* If not already recovering from an error, report this error.  */
  if (!yyerrstatus)
    {
      ++yynerrs;
      yyerror (YY_("syntax error"));
    }

  if (yyerrstatus == 3)
    {
      /* If just tried and failed to reuse lookahead token after an
         error, discard it.  */

      if (yychar <= YYEOF)
        {
          /* Return failure if at end of input.  */
          if (yychar == YYEOF)
            YYABORT;
        }
      else
        {
          yydestruct ("Error: discarding",
                      yytoken, &yylval);
          yychar = YYEMPTY;
        }
    }

  /* Else will try to reuse lookahead token after shifting the error
     token.  */
  goto yyerrlab1;


/*---------------------------------------------------.
| yyerrorlab -- error raised explicitly by YYERROR.  |
`---------------------------------------------------*/
yyerrorlab:
  /* Pacify compilers when the user code never invokes YYERROR and the
     label yyerrorlab therefore never appears in user code.  */
  if (0)
    YYERROR;
  ++yynerrs;

  /* Do not reclaim the symbols of the rule whose action triggered
     this YYERROR.  */
  YYPOPSTACK (yylen);
  yylen = 0;
  YY_STACK_PRINT (yyss, yyssp);
  yystate = *yyssp;
  goto yyerrlab1;


/*-------------------------------------------------------------.
| yyerrlab1 -- common code for both syntax error and YYERROR.  |
`-------------------------------------------------------------*/
yyerrlab1:
  yyerrstatus = 3;      /* Each real token shifted decrements this.  */

  /* Pop stack until we find a state that shifts the error token.  */
  for (;;)
    {
      yyn = yypact[yystate];
      if (!yypact_value_is_default (yyn))
        {
          yyn += YYSYMBOL_YYerror;
          if (0 <= yyn && yyn <= YYLAST && yycheck[yyn] == YYSYMBOL_YYerror)
            {
              yyn = yytable[yyn];
              if (0 < yyn)
                break;
            }
        }

      /* Pop the current state because it cannot handle the error token.  */
      if (yyssp == yyss)
        YYABORT;


      yydestruct ("Error: popping",
                  YY_ACCESSING_SYMBOL (yystate), yyvsp);
      YYPOPSTACK (1);
      yystate = *yyssp;
      YY_STACK_PRINT (yyss, yyssp);
    }

  YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
  *++yyvsp = yylval;
  YY_IGNORE_MAYBE_UNINITIALIZED_END


  /* Shift the error token.  */
  YY_SYMBOL_PRINT ("Shifting", YY_ACCESSING_SYMBOL (yyn), yyvsp, yylsp);

  yystate = yyn;
  goto yynewstate;


/*-------------------------------------.
| yyacceptlab -- YYACCEPT comes here.  |
`-------------------------------------*/
yyacceptlab:
  yyresult = 0;
  goto yyreturnlab;


/*-----------------------------------.
| yyabortlab -- YYABORT comes here.  |
`-----------------------------------*/
yyabortlab:
  yyresult = 1;
  goto yyreturnlab;


/*-----------------------------------------------------------.
| yyexhaustedlab -- YYNOMEM (memory exhaustion) comes here.  |
`-----------------------------------------------------------*/
yyexhaustedlab:
  yyerror (YY_("memory exhausted"));
  yyresult = 2;
  goto yyreturnlab;


/*----------------------------------------------------------.
| yyreturnlab -- parsing is finished, clean up and return.  |
`----------------------------------------------------------*/
yyreturnlab:
  if (yychar != YYEMPTY)
    {
      /* Make sure we have latest lookahead translation.  See comments at
         user semantic actions for why this is necessary.  */
      yytoken = YYTRANSLATE (yychar);
      yydestruct ("Cleanup: discarding lookahead",
                  yytoken, &yylval);
    }
  /* Do not reclaim the symbols of the rule whose action triggered
     this YYABORT or YYACCEPT.  */
  YYPOPSTACK (yylen);
  YY_STACK_PRINT (yyss, yyssp);
  while (yyssp != yyss)
    {
      yydestruct ("Cleanup: popping",
                  YY_ACCESSING_SYMBOL (+*yyssp), yyvsp);
      YYPOPSTACK (1);
    }
#ifndef yyoverflow
  if (yyss != yyssa)
    YYSTACK_FREE (yyss);
#endif

  return yyresult;
}

#line 604 "parser.y"

int main() {
    printf("Starting modern C parser...\n");
    printf("\n--- Lexical Tokens ---\n");
    return yyparse();
}

void report_error(int line, const char* msg) {
    if(!error_occurred) error_occurred = 1;
    fprintf(stderr, "[Friendly Compiler Notice] Error at line %d: %s\n", line, msg);
}

void yyerror(const char* s) {
    report_error(lno, s);
}

void printdebug(const char* msg, char c) {
    if (c != ' ') printf("Debug: %s '%c' at line %d\n", msg, c, lno);
    else printf("Debug: %s at line %d\n", msg, lno);
}

struct node* lookup(char *name) {
    struct node *ptr = first;
    while(ptr != NULL) {
        if(strcmp(ptr->name, name) == 0 && ptr->valid == 1) return ptr;
        ptr = ptr->link;
    }
    return NULL;
}

struct node* insert(char *name, int type) {
    if (type == 3) {
        char errmsg[100];
        sprintf(errmsg, "Variable '%s' cannot be declared as type 'void'", name);
        report_error(lno, errmsg);
        return NULL;
    }
    struct node *ptr = first;
    while(ptr != NULL) {
        if(strcmp(ptr->name, name) == 0 && ptr->scope == scope && ptr->valid == 1) {
            char errmsg[50];
            sprintf(errmsg, "Redeclaration of %s", name);
            report_error(lno, errmsg);
            return NULL;
        }
        ptr = ptr->link;
    }
    struct node *newnode = (struct node*)malloc(sizeof(struct node));
    strcpy(newnode->name, name);
    newnode->dtype = type;
    newnode->scope = scope;
    newnode->valid = 1;
    newnode->is_used = 0;
    newnode->is_allocated = 0;
    newnode->link = first;
    first = newnode;
    return newnode;
}

void printsymtable() {
    printf("\n--- Symbol Table ---\n");
    struct node *ptr = first;
    while(ptr != NULL) {
        printf("Name: %-10s | Scope: %d | Type: %d | Status: %s\n",
               ptr->name, ptr->scope, ptr->dtype, ptr->valid ? "Active" : "Dead");
        ptr = ptr->link;
    }
}

void cleansymbol() {
    struct node *ptr = first;
    while(ptr != NULL) {
        struct node *del = ptr;
        ptr = ptr->link;
        free(del);
    }
    first = NULL;
}

void push_tree(Node *newnode) {
    tree_stack *temp = (tree_stack*)malloc(sizeof(tree_stack));
    temp->node = newnode;
    temp->next = tree_top;
    tree_top = temp;
}

Node *pop_tree() {
    if(tree_top == NULL) return NULL;
    tree_stack *temp = tree_top;
    Node *ret = temp->node;
    tree_top = tree_top->next;
    free(temp);
    return ret;
}

void create_node(char *token, int leaf) {
    Node *newnode = (Node*)malloc(sizeof(Node));
    strcpy(newnode->token, token);
    newnode->mid1 = NULL;
    newnode->mid2 = NULL;
    if (leaf) {
        newnode->left = newnode->right = NULL;
    } else {
        newnode->right = pop_tree();
        newnode->left = pop_tree();
    }

    newnode->eval_type = tree_top && tree_top->node ? tree_top->node->eval_type : 0;
    push_tree(newnode);
}

void printAST(Node* root, int level) {
    if (root == NULL || strcmp(root->token, "dummy") == 0) return;
    for (int i = 0; i < level; i++) printf("  ");
    printf("-> %s\n", root->token);
    printAST(root->left, level + 1);
    printAST(root->right, level + 1);
}

void clear_tree(Node* root) {
    if (root == NULL) return;
    clear_tree(root->left);
    clear_tree(root->right);
    free(root);
}

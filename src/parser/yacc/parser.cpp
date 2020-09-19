/* A Bison parser, made by GNU Bison 3.0.4.  */

/* Bison implementation for Yacc-like parsers in C

   Copyright (C) 1984, 1989-1990, 2000-2015 Free Software Foundation, Inc.

   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.  */

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

/* All symbols defined below should begin with yy or YY, to avoid
   infringing on user name space.  This should be done even for local
   variables, as they might otherwise be expanded by user macros.
   There are some unavoidable exceptions within include files to
   define necessary library symbols; they are noted "INFRINGES ON
   USER NAME SPACE" below.  */

/* Identify Bison output.  */
#define YYBISON 1

/* Bison version.  */
#define YYBISON_VERSION "3.0.4"

/* Skeleton name.  */
#define YYSKELETON_NAME "yacc.c"

/* Pure parsers.  */
#define YYPURE 0

/* Push parsers.  */
#define YYPUSH 0

/* Pull parsers.  */
#define YYPULL 1




/* Copy the first part of user declarations.  */
#line 1 "parser.y" /* yacc.c:339  */

#include "token_type.h"
#include "../../tree/display_ast.h"
#include "../../env/env.h"
#include <vector> 
#include <string> 
#include <cstdio> 
#include <cassert>
#include "../../util/cuteprint.h"

// define semantic type here
#ifndef YYSTYPE_IS_DECLARED
typedef TokenData YYSTYPE;
#define YYSTYPE_IS_DECLARED 1
#endif

#include <iostream>
#include <string>
#include "../../tree/tree.h"

using std::cerr; 
using std::endl;
using std::vector; 

// var and func in `lexer.l`
extern int yylex();
extern int linum; 
extern FILE * yyin;
extern FILE * yyout;


static TopLevel *program;

static void msg(std::string str)
{
//    logger << "[Parser] Line " << linum << " :: " << str << endl; 
} 

void yyerror(const char *str)
{
//    cerr << "[Error] Syntax Error at line " << linum << ": " << str << endl; 
}


#line 111 "parser.cpp" /* yacc.c:339  */

# ifndef YY_NULLPTR
#  if defined __cplusplus && 201103L <= __cplusplus
#   define YY_NULLPTR nullptr
#  else
#   define YY_NULLPTR 0
#  endif
# endif

/* Enabling verbose error messages.  */
#ifdef YYERROR_VERBOSE
# undef YYERROR_VERBOSE
# define YYERROR_VERBOSE 1
#else
# define YYERROR_VERBOSE 0
#endif

/* In a future release of Bison, this section will be replaced
   by #include "parser.h".  */
#ifndef YY_YY_PARSER_H_INCLUDED
# define YY_YY_PARSER_H_INCLUDED
/* Debug traces.  */
#ifndef YYDEBUG
# define YYDEBUG 0
#endif
#if YYDEBUG
extern int yydebug;
#endif

/* Token type.  */
#ifndef YYTOKENTYPE
# define YYTOKENTYPE
  enum yytokentype
  {
    P_VOID = 258,
    P_INT = 259,
    P_CONST = 260,
    P_NUM = 261,
    P_STR = 262,
    P_WHILE = 263,
    P_IF = 264,
    P_ELSE = 265,
    P_RETURN = 266,
    P_BREAK = 267,
    P_CONTINUE = 268,
    P_IDENTIFIER = 269,
    P_AND = 270,
    P_OR = 271,
    P_LE = 272,
    P_GE = 273,
    P_EQ = 274,
    P_NE = 275,
    P_MINUS = 276,
    P_POS = 277
  };
#endif

/* Value type.  */
#if ! defined YYSTYPE && ! defined YYSTYPE_IS_DECLARED
typedef int YYSTYPE;
# define YYSTYPE_IS_TRIVIAL 1
# define YYSTYPE_IS_DECLARED 1
#endif


extern YYSTYPE yylval;

int yyparse (void);

#endif /* !YY_YY_PARSER_H_INCLUDED  */

/* Copy the second part of user declarations.  */

#line 185 "parser.cpp" /* yacc.c:358  */

#ifdef short
# undef short
#endif

#ifdef YYTYPE_UINT8
typedef YYTYPE_UINT8 yytype_uint8;
#else
typedef unsigned char yytype_uint8;
#endif

#ifdef YYTYPE_INT8
typedef YYTYPE_INT8 yytype_int8;
#else
typedef signed char yytype_int8;
#endif

#ifdef YYTYPE_UINT16
typedef YYTYPE_UINT16 yytype_uint16;
#else
typedef unsigned short int yytype_uint16;
#endif

#ifdef YYTYPE_INT16
typedef YYTYPE_INT16 yytype_int16;
#else
typedef short int yytype_int16;
#endif

#ifndef YYSIZE_T
# ifdef __SIZE_TYPE__
#  define YYSIZE_T __SIZE_TYPE__
# elif defined size_t
#  define YYSIZE_T size_t
# elif ! defined YYSIZE_T
#  include <stddef.h> /* INFRINGES ON USER NAME SPACE */
#  define YYSIZE_T size_t
# else
#  define YYSIZE_T unsigned int
# endif
#endif

#define YYSIZE_MAXIMUM ((YYSIZE_T) -1)

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

#ifndef YY_ATTRIBUTE
# if (defined __GNUC__                                               \
      && (2 < __GNUC__ || (__GNUC__ == 2 && 96 <= __GNUC_MINOR__)))  \
     || defined __SUNPRO_C && 0x5110 <= __SUNPRO_C
#  define YY_ATTRIBUTE(Spec) __attribute__(Spec)
# else
#  define YY_ATTRIBUTE(Spec) /* empty */
# endif
#endif

#ifndef YY_ATTRIBUTE_PURE
# define YY_ATTRIBUTE_PURE   YY_ATTRIBUTE ((__pure__))
#endif

#ifndef YY_ATTRIBUTE_UNUSED
# define YY_ATTRIBUTE_UNUSED YY_ATTRIBUTE ((__unused__))
#endif

#if !defined _Noreturn \
     && (!defined __STDC_VERSION__ || __STDC_VERSION__ < 201112)
# if defined _MSC_VER && 1200 <= _MSC_VER
#  define _Noreturn __declspec (noreturn)
# else
#  define _Noreturn YY_ATTRIBUTE ((__noreturn__))
# endif
#endif

/* Suppress unused-variable warnings by "using" E.  */
#if ! defined lint || defined __GNUC__
# define YYUSE(E) ((void) (E))
#else
# define YYUSE(E) /* empty */
#endif

#if defined __GNUC__ && 407 <= __GNUC__ * 100 + __GNUC_MINOR__
/* Suppress an incorrect diagnostic about yylval being uninitialized.  */
# define YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN \
    _Pragma ("GCC diagnostic push") \
    _Pragma ("GCC diagnostic ignored \"-Wuninitialized\"")\
    _Pragma ("GCC diagnostic ignored \"-Wmaybe-uninitialized\"")
# define YY_IGNORE_MAYBE_UNINITIALIZED_END \
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


#if ! defined yyoverflow || YYERROR_VERBOSE

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
#endif /* ! defined yyoverflow || YYERROR_VERBOSE */


#if (! defined yyoverflow \
     && (! defined __cplusplus \
         || (defined YYSTYPE_IS_TRIVIAL && YYSTYPE_IS_TRIVIAL)))

/* A type that is properly aligned for any stack member.  */
union yyalloc
{
  yytype_int16 yyss_alloc;
  YYSTYPE yyvs_alloc;
};

/* The size of the maximum gap between one aligned stack and the next.  */
# define YYSTACK_GAP_MAXIMUM (sizeof (union yyalloc) - 1)

/* The size of an array large to enough to hold all stacks, each with
   N elements.  */
# define YYSTACK_BYTES(N) \
     ((N) * (sizeof (yytype_int16) + sizeof (YYSTYPE)) \
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
        YYSIZE_T yynewbytes;                                            \
        YYCOPY (&yyptr->Stack_alloc, Stack, yysize);                    \
        Stack = &yyptr->Stack_alloc;                                    \
        yynewbytes = yystacksize * sizeof (*Stack) + YYSTACK_GAP_MAXIMUM; \
        yyptr += yynewbytes / sizeof (*yyptr);                          \
      }                                                                 \
    while (0)

#endif

#if defined YYCOPY_NEEDED && YYCOPY_NEEDED
/* Copy COUNT objects from SRC to DST.  The source and destination do
   not overlap.  */
# ifndef YYCOPY
#  if defined __GNUC__ && 1 < __GNUC__
#   define YYCOPY(Dst, Src, Count) \
      __builtin_memcpy (Dst, Src, (Count) * sizeof (*(Src)))
#  else
#   define YYCOPY(Dst, Src, Count)              \
      do                                        \
        {                                       \
          YYSIZE_T yyi;                         \
          for (yyi = 0; yyi < (Count); yyi++)   \
            (Dst)[yyi] = (Src)[yyi];            \
        }                                       \
      while (0)
#  endif
# endif
#endif /* !YYCOPY_NEEDED */

/* YYFINAL -- State number of the termination state.  */
#define YYFINAL  2
/* YYLAST -- Last index in YYTABLE.  */
#define YYLAST   355

/* YYNTOKENS -- Number of terminals.  */
#define YYNTOKENS  40
/* YYNNTS -- Number of nonterminals.  */
#define YYNNTS  22
/* YYNRULES -- Number of rules.  */
#define YYNRULES  69
/* YYNSTATES -- Number of states.  */
#define YYNSTATES  134

/* YYTRANSLATE[YYX] -- Symbol number corresponding to YYX as returned
   by yylex, with out-of-bounds checking.  */
#define YYUNDEFTOK  2
#define YYMAXUTOK   277

#define YYTRANSLATE(YYX)                                                \
  ((unsigned int) (YYX) <= YYMAXUTOK ? yytranslate[YYX] : YYUNDEFTOK)

/* YYTRANSLATE[TOKEN-NUM] -- Symbol number corresponding to TOKEN-NUM
   as returned by yylex, without out-of-bounds checking.  */
static const yytype_uint8 yytranslate[] =
{
       0,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,    31,     2,     2,     2,    27,     2,     2,
      34,    35,    25,    23,    32,    24,     2,    26,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,    33,
      29,    28,    30,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,    36,     2,    37,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,    38,     2,    39,     2,     2,     2,     2,
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
      15,    16,    17,    18,    19,    20,    21,    22
};

#if YYDEBUG
  /* YYRLINE[YYN] -- Source line where rule number YYN was defined.  */
static const yytype_uint16 yyrline[] =
{
       0,    72,    72,    76,    81,    90,    94,   108,   116,   124,
     130,   140,   147,   157,   167,   173,   179,   184,   191,   197,
     207,   215,   225,   230,   235,   241,   248,   253,   270,   280,
     286,   293,   299,   304,   309,   314,   319,   324,   329,   334,
     339,   344,   349,   358,   365,   369,   373,   378,   382,   386,
     390,   394,   398,   402,   406,   410,   414,   418,   422,   426,
     430,   434,   438,   442,   446,   452,   458,   463,   471,   476
};
#endif

#if YYDEBUG || YYERROR_VERBOSE || 0
/* YYTNAME[SYMBOL-NUM] -- String name of the symbol SYMBOL-NUM.
   First, the terminals, then, starting at YYNTOKENS, nonterminals.  */
static const char *const yytname[] =
{
  "$end", "error", "$undefined", "P_VOID", "P_INT", "P_CONST", "P_NUM",
  "P_STR", "P_WHILE", "P_IF", "P_ELSE", "P_RETURN", "P_BREAK",
  "P_CONTINUE", "P_IDENTIFIER", "P_AND", "P_OR", "P_LE", "P_GE", "P_EQ",
  "P_NE", "P_MINUS", "P_POS", "'+'", "'-'", "'*'", "'/'", "'%'", "'='",
  "'<'", "'>'", "'!'", "','", "';'", "'('", "')'", "'['", "']'", "'{'",
  "'}'", "$accept", "TopLevel", "Decl", "ConstDecl", "VarDecl",
  "VarDefList", "VarDef", "ArrayIndex", "InitVal", "InitValList",
  "FuncDef", "FuncFParamListE", "FuncFParamList", "FuncFParam", "Block",
  "StmtList", "Stmt", "LVal", "Expr", "FuncCall", "FuncRParamList",
  "FuncRParamListE", YY_NULLPTR
};
#endif

# ifdef YYPRINT
/* YYTOKNUM[NUM] -- (External) token number corresponding to the
   (internal) symbol number NUM (which must be that of a token).  */
static const yytype_uint16 yytoknum[] =
{
       0,   256,   257,   258,   259,   260,   261,   262,   263,   264,
     265,   266,   267,   268,   269,   270,   271,   272,   273,   274,
     275,   276,   277,    43,    45,    42,    47,    37,    61,    60,
      62,    33,    44,    59,    40,    41,    91,    93,   123,   125
};
# endif

#define YYPACT_NINF -79

#define yypact_value_is_default(Yystate) \
  (!!((Yystate) == (-79)))

#define YYTABLE_NINF -1

#define yytable_value_is_error(Yytable_value) \
  (!!((Yytable_value) == (-1)))

  /* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
     STATE-NUM.  */
static const yytype_int16 yypact[] =
{
     -79,   103,   -79,    11,    18,    47,   -79,   -79,   -79,   -79,
      25,    27,   -26,   -79,    54,    69,    69,   -18,    54,   -79,
     -79,    43,    65,    45,    51,   -79,    55,   117,   143,   -79,
     -79,    50,    60,    69,    60,   -79,   -79,    66,   143,   143,
     143,   143,    71,   -79,   -79,     4,   -79,   163,    62,   -79,
     -79,   -79,   -79,   143,    68,   -79,   -79,   -79,   179,   -79,
     -79,   -23,   143,   143,   143,   143,   143,   143,   143,   143,
     143,   143,   143,   143,   143,   -79,   -79,    58,     4,    83,
      85,   -79,   117,   -79,   311,   297,   325,   325,   325,   325,
     -22,   -22,   -79,   -79,   -79,   325,   325,    68,    54,    67,
      87,   138,    89,    92,   -79,   -79,   -79,   -79,   -79,    98,
     242,   143,   -79,   -79,   143,   143,   -79,   261,   -79,   -79,
     143,   -79,     4,   200,   221,   -79,   280,   105,   105,   -79,
     -79,   120,   105,   -79
};

  /* YYDEFACT[STATE-NUM] -- Default reduction number in state STATE-NUM.
     Performed when YYTABLE does not specify something else to do.  Zero
     means the default is an error.  */
static const yytype_uint8 yydefact[] =
{
       2,     0,     1,     0,     0,     0,     3,     5,     6,     4,
       0,    14,     0,     9,     0,    23,    23,    12,     0,     8,
      14,     0,     0,     0,    22,    25,     0,     0,     0,    10,
       7,    26,     0,     0,     0,    44,    45,    14,     0,     0,
       0,     0,     0,    11,    46,    15,    47,     0,     0,    31,
      21,    24,    20,    69,    43,    49,    50,    51,     0,    16,
      19,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    13,    14,     0,    66,    68,
       0,    48,     0,    17,    63,    64,    59,    60,    61,    62,
      52,    53,    54,    55,    56,    57,    58,    27,     0,     0,
       0,     0,     0,     0,    35,    28,    30,    33,    29,    46,
       0,     0,    65,    18,     0,     0,    41,     0,    39,    40,
       0,    34,    67,     0,     0,    42,     0,     0,     0,    32,
      38,    36,     0,    37
};

  /* YYPGOTO[NTERM-NUM].  */
static const yytype_int8 yypgoto[] =
{
     -79,   -79,    56,   -79,   -79,   113,   114,   -20,   -34,   -79,
     -79,   118,   -79,   102,    26,   -79,   -78,   -75,   -27,   -79,
     -79,   -79
};

  /* YYDEFGOTO[NTERM-NUM].  */
static const yytype_int8 yydefgoto[] =
{
      -1,     1,     6,     7,     8,    12,    13,    17,    43,    61,
       9,    23,    24,    25,   107,    77,   108,    44,   110,    46,
      79,    80
};

  /* YYTABLE[YYPACT[STATE-NUM]] -- What to do in state STATE-NUM.  If
     positive, shift that token.  If negative, reduce the rule whose
     number is the opposite.  If YYTABLE_NINF, syntax error.  */
static const yytype_int16 yytable[] =
{
      45,    47,   109,    70,    71,    72,    18,    19,    60,    82,
      27,    55,    56,    57,    58,    45,    83,    54,    28,    62,
      63,    64,    65,    66,    67,    10,    78,    68,    69,    70,
      71,    72,    11,    73,    74,    84,    85,    86,    87,    88,
      89,    90,    91,    92,    93,    94,    95,    96,   113,   130,
     131,    14,   109,   109,   133,    45,    97,   109,    50,    15,
      52,    16,    98,     5,    35,    36,    99,   100,    20,   101,
     102,   103,    37,    22,   117,    18,    30,    35,    36,    31,
      32,    38,    39,    33,   122,    37,    48,   123,   124,    40,
      34,   104,    41,   126,    38,    39,    49,   105,    49,    76,
      53,   114,    40,     2,    28,    41,     3,     4,     5,    42,
      59,    35,    36,    99,   100,   111,   101,   102,   103,    37,
     112,   115,   118,    35,    36,   119,   120,    21,    38,    39,
     132,    37,    29,   106,    26,    51,    40,     0,   104,    41,
      38,    39,     0,    49,    35,    36,     0,     0,    40,    35,
      36,    41,    37,     0,     0,    42,     0,    37,     0,     0,
       0,    38,    39,     0,     0,     0,    38,    39,     0,    40,
       0,   116,    41,     0,    40,     0,     0,    41,    62,    63,
      64,    65,    66,    67,     0,     0,    68,    69,    70,    71,
      72,     0,    73,    74,    62,    63,    64,    65,    66,    67,
      75,     0,    68,    69,    70,    71,    72,     0,    73,    74,
       0,     0,     0,     0,    81,    62,    63,    64,    65,    66,
      67,     0,     0,    68,    69,    70,    71,    72,     0,    73,
      74,     0,     0,     0,     0,   127,    62,    63,    64,    65,
      66,    67,     0,     0,    68,    69,    70,    71,    72,     0,
      73,    74,     0,     0,     0,     0,   128,    62,    63,    64,
      65,    66,    67,     0,     0,    68,    69,    70,    71,    72,
       0,    73,    74,     0,     0,   121,    62,    63,    64,    65,
      66,    67,     0,     0,    68,    69,    70,    71,    72,     0,
      73,    74,     0,     0,   125,    62,    63,    64,    65,    66,
      67,     0,     0,    68,    69,    70,    71,    72,     0,    73,
      74,     0,    62,   129,    64,    65,    66,    67,     0,     0,
      68,    69,    70,    71,    72,     0,    73,    74,    64,    65,
      66,    67,     0,     0,    68,    69,    70,    71,    72,     0,
      73,    74,    -1,    -1,    -1,    -1,     0,     0,    68,    69,
      70,    71,    72,     0,    -1,    -1
};

static const yytype_int16 yycheck[] =
{
      27,    28,    77,    25,    26,    27,    32,    33,    42,    32,
      28,    38,    39,    40,    41,    42,    39,    37,    36,    15,
      16,    17,    18,    19,    20,    14,    53,    23,    24,    25,
      26,    27,    14,    29,    30,    62,    63,    64,    65,    66,
      67,    68,    69,    70,    71,    72,    73,    74,    82,   127,
     128,     4,   127,   128,   132,    82,    76,   132,    32,    34,
      34,    34,     4,     5,     6,     7,     8,     9,    14,    11,
      12,    13,    14,     4,   101,    32,    33,     6,     7,    14,
      35,    23,    24,    32,   111,    14,    36,   114,   115,    31,
      35,    33,    34,   120,    23,    24,    38,    39,    38,    37,
      34,    34,    31,     0,    36,    34,     3,     4,     5,    38,
      39,     6,     7,     8,     9,    32,    11,    12,    13,    14,
      35,    34,    33,     6,     7,    33,    28,    14,    23,    24,
      10,    14,    18,    77,    16,    33,    31,    -1,    33,    34,
      23,    24,    -1,    38,     6,     7,    -1,    -1,    31,     6,
       7,    34,    14,    -1,    -1,    38,    -1,    14,    -1,    -1,
      -1,    23,    24,    -1,    -1,    -1,    23,    24,    -1,    31,
      -1,    33,    34,    -1,    31,    -1,    -1,    34,    15,    16,
      17,    18,    19,    20,    -1,    -1,    23,    24,    25,    26,
      27,    -1,    29,    30,    15,    16,    17,    18,    19,    20,
      37,    -1,    23,    24,    25,    26,    27,    -1,    29,    30,
      -1,    -1,    -1,    -1,    35,    15,    16,    17,    18,    19,
      20,    -1,    -1,    23,    24,    25,    26,    27,    -1,    29,
      30,    -1,    -1,    -1,    -1,    35,    15,    16,    17,    18,
      19,    20,    -1,    -1,    23,    24,    25,    26,    27,    -1,
      29,    30,    -1,    -1,    -1,    -1,    35,    15,    16,    17,
      18,    19,    20,    -1,    -1,    23,    24,    25,    26,    27,
      -1,    29,    30,    -1,    -1,    33,    15,    16,    17,    18,
      19,    20,    -1,    -1,    23,    24,    25,    26,    27,    -1,
      29,    30,    -1,    -1,    33,    15,    16,    17,    18,    19,
      20,    -1,    -1,    23,    24,    25,    26,    27,    -1,    29,
      30,    -1,    15,    33,    17,    18,    19,    20,    -1,    -1,
      23,    24,    25,    26,    27,    -1,    29,    30,    17,    18,
      19,    20,    -1,    -1,    23,    24,    25,    26,    27,    -1,
      29,    30,    17,    18,    19,    20,    -1,    -1,    23,    24,
      25,    26,    27,    -1,    29,    30
};

  /* YYSTOS[STATE-NUM] -- The (internal number of the) accessing
     symbol of state STATE-NUM.  */
static const yytype_uint8 yystos[] =
{
       0,    41,     0,     3,     4,     5,    42,    43,    44,    50,
      14,    14,    45,    46,     4,    34,    34,    47,    32,    33,
      14,    45,     4,    51,    52,    53,    51,    28,    36,    46,
      33,    14,    35,    32,    35,     6,     7,    14,    23,    24,
      31,    34,    38,    48,    57,    58,    59,    58,    36,    38,
      54,    53,    54,    34,    47,    58,    58,    58,    58,    39,
      48,    49,    15,    16,    17,    18,    19,    20,    23,    24,
      25,    26,    27,    29,    30,    37,    37,    55,    58,    60,
      61,    35,    32,    39,    58,    58,    58,    58,    58,    58,
      58,    58,    58,    58,    58,    58,    58,    47,     4,     8,
       9,    11,    12,    13,    33,    39,    42,    54,    56,    57,
      58,    32,    35,    48,    34,    34,    33,    58,    33,    33,
      28,    33,    58,    58,    58,    33,    58,    35,    35,    33,
      56,    56,    10,    56
};

  /* YYR1[YYN] -- Symbol number of symbol that rule YYN derives.  */
static const yytype_uint8 yyr1[] =
{
       0,    40,    41,    41,    41,    42,    42,    43,    44,    45,
      45,    46,    46,    47,    47,    48,    48,    48,    49,    49,
      50,    50,    51,    51,    52,    52,    53,    53,    54,    55,
      55,    55,    56,    56,    56,    56,    56,    56,    56,    56,
      56,    56,    56,    57,    58,    58,    58,    58,    58,    58,
      58,    58,    58,    58,    58,    58,    58,    58,    58,    58,
      58,    58,    58,    58,    58,    59,    60,    60,    61,    61
};

  /* YYR2[YYN] -- Number of symbols on the right hand side of rule YYN.  */
static const yytype_uint8 yyr2[] =
{
       0,     2,     0,     2,     2,     1,     1,     4,     3,     1,
       3,     4,     2,     4,     0,     1,     2,     3,     3,     1,
       6,     6,     1,     0,     3,     1,     2,     5,     3,     2,
       2,     0,     4,     1,     2,     1,     5,     7,     5,     2,
       2,     2,     3,     2,     1,     1,     1,     1,     3,     2,
       2,     2,     3,     3,     3,     3,     3,     3,     3,     3,
       3,     3,     3,     3,     3,     4,     1,     3,     1,     0
};


#define yyerrok         (yyerrstatus = 0)
#define yyclearin       (yychar = YYEMPTY)
#define YYEMPTY         (-2)
#define YYEOF           0

#define YYACCEPT        goto yyacceptlab
#define YYABORT         goto yyabortlab
#define YYERROR         goto yyerrorlab


#define YYRECOVERING()  (!!yyerrstatus)

#define YYBACKUP(Token, Value)                                  \
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

/* Error token number */
#define YYTERROR        1
#define YYERRCODE       256



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

/* This macro is provided for backward compatibility. */
#ifndef YY_LOCATION_PRINT
# define YY_LOCATION_PRINT(File, Loc) ((void) 0)
#endif


# define YY_SYMBOL_PRINT(Title, Type, Value, Location)                    \
do {                                                                      \
  if (yydebug)                                                            \
    {                                                                     \
      YYFPRINTF (stderr, "%s ", Title);                                   \
      yy_symbol_print (stderr,                                            \
                  Type, Value); \
      YYFPRINTF (stderr, "\n");                                           \
    }                                                                     \
} while (0)


/*----------------------------------------.
| Print this symbol's value on YYOUTPUT.  |
`----------------------------------------*/

static void
yy_symbol_value_print (FILE *yyoutput, int yytype, YYSTYPE const * const yyvaluep)
{
  FILE *yyo = yyoutput;
  YYUSE (yyo);
  if (!yyvaluep)
    return;
# ifdef YYPRINT
  if (yytype < YYNTOKENS)
    YYPRINT (yyoutput, yytoknum[yytype], *yyvaluep);
# endif
  YYUSE (yytype);
}


/*--------------------------------.
| Print this symbol on YYOUTPUT.  |
`--------------------------------*/

static void
yy_symbol_print (FILE *yyoutput, int yytype, YYSTYPE const * const yyvaluep)
{
  YYFPRINTF (yyoutput, "%s %s (",
             yytype < YYNTOKENS ? "token" : "nterm", yytname[yytype]);

  yy_symbol_value_print (yyoutput, yytype, yyvaluep);
  YYFPRINTF (yyoutput, ")");
}

/*------------------------------------------------------------------.
| yy_stack_print -- Print the state stack from its BOTTOM up to its |
| TOP (included).                                                   |
`------------------------------------------------------------------*/

static void
yy_stack_print (yytype_int16 *yybottom, yytype_int16 *yytop)
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
yy_reduce_print (yytype_int16 *yyssp, YYSTYPE *yyvsp, int yyrule)
{
  unsigned long int yylno = yyrline[yyrule];
  int yynrhs = yyr2[yyrule];
  int yyi;
  YYFPRINTF (stderr, "Reducing stack by rule %d (line %lu):\n",
             yyrule - 1, yylno);
  /* The symbols being reduced.  */
  for (yyi = 0; yyi < yynrhs; yyi++)
    {
      YYFPRINTF (stderr, "   $%d = ", yyi + 1);
      yy_symbol_print (stderr,
                       yystos[yyssp[yyi + 1 - yynrhs]],
                       &(yyvsp[(yyi + 1) - (yynrhs)])
                                              );
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
# define YYDPRINTF(Args)
# define YY_SYMBOL_PRINT(Title, Type, Value, Location)
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


#if YYERROR_VERBOSE

# ifndef yystrlen
#  if defined __GLIBC__ && defined _STRING_H
#   define yystrlen strlen
#  else
/* Return the length of YYSTR.  */
static YYSIZE_T
yystrlen (const char *yystr)
{
  YYSIZE_T yylen;
  for (yylen = 0; yystr[yylen]; yylen++)
    continue;
  return yylen;
}
#  endif
# endif

# ifndef yystpcpy
#  if defined __GLIBC__ && defined _STRING_H && defined _GNU_SOURCE
#   define yystpcpy stpcpy
#  else
/* Copy YYSRC to YYDEST, returning the address of the terminating '\0' in
   YYDEST.  */
static char *
yystpcpy (char *yydest, const char *yysrc)
{
  char *yyd = yydest;
  const char *yys = yysrc;

  while ((*yyd++ = *yys++) != '\0')
    continue;

  return yyd - 1;
}
#  endif
# endif

# ifndef yytnamerr
/* Copy to YYRES the contents of YYSTR after stripping away unnecessary
   quotes and backslashes, so that it's suitable for yyerror.  The
   heuristic is that double-quoting is unnecessary unless the string
   contains an apostrophe, a comma, or backslash (other than
   backslash-backslash).  YYSTR is taken from yytname.  If YYRES is
   null, do not copy; instead, return the length of what the result
   would have been.  */
static YYSIZE_T
yytnamerr (char *yyres, const char *yystr)
{
  if (*yystr == '"')
    {
      YYSIZE_T yyn = 0;
      char const *yyp = yystr;

      for (;;)
        switch (*++yyp)
          {
          case '\'':
          case ',':
            goto do_not_strip_quotes;

          case '\\':
            if (*++yyp != '\\')
              goto do_not_strip_quotes;
            /* Fall through.  */
          default:
            if (yyres)
              yyres[yyn] = *yyp;
            yyn++;
            break;

          case '"':
            if (yyres)
              yyres[yyn] = '\0';
            return yyn;
          }
    do_not_strip_quotes: ;
    }

  if (! yyres)
    return yystrlen (yystr);

  return yystpcpy (yyres, yystr) - yyres;
}
# endif

/* Copy into *YYMSG, which is of size *YYMSG_ALLOC, an error message
   about the unexpected token YYTOKEN for the state stack whose top is
   YYSSP.

   Return 0 if *YYMSG was successfully written.  Return 1 if *YYMSG is
   not large enough to hold the message.  In that case, also set
   *YYMSG_ALLOC to the required number of bytes.  Return 2 if the
   required number of bytes is too large to store.  */
static int
yysyntax_error (YYSIZE_T *yymsg_alloc, char **yymsg,
                yytype_int16 *yyssp, int yytoken)
{
  YYSIZE_T yysize0 = yytnamerr (YY_NULLPTR, yytname[yytoken]);
  YYSIZE_T yysize = yysize0;
  enum { YYERROR_VERBOSE_ARGS_MAXIMUM = 5 };
  /* Internationalized format string. */
  const char *yyformat = YY_NULLPTR;
  /* Arguments of yyformat. */
  char const *yyarg[YYERROR_VERBOSE_ARGS_MAXIMUM];
  /* Number of reported tokens (one for the "unexpected", one per
     "expected"). */
  int yycount = 0;

  /* There are many possibilities here to consider:
     - If this state is a consistent state with a default action, then
       the only way this function was invoked is if the default action
       is an error action.  In that case, don't check for expected
       tokens because there are none.
     - The only way there can be no lookahead present (in yychar) is if
       this state is a consistent state with a default action.  Thus,
       detecting the absence of a lookahead is sufficient to determine
       that there is no unexpected or expected token to report.  In that
       case, just report a simple "syntax error".
     - Don't assume there isn't a lookahead just because this state is a
       consistent state with a default action.  There might have been a
       previous inconsistent state, consistent state with a non-default
       action, or user semantic action that manipulated yychar.
     - Of course, the expected token list depends on states to have
       correct lookahead information, and it depends on the parser not
       to perform extra reductions after fetching a lookahead from the
       scanner and before detecting a syntax error.  Thus, state merging
       (from LALR or IELR) and default reductions corrupt the expected
       token list.  However, the list is correct for canonical LR with
       one exception: it will still contain any token that will not be
       accepted due to an error action in a later state.
  */
  if (yytoken != YYEMPTY)
    {
      int yyn = yypact[*yyssp];
      yyarg[yycount++] = yytname[yytoken];
      if (!yypact_value_is_default (yyn))
        {
          /* Start YYX at -YYN if negative to avoid negative indexes in
             YYCHECK.  In other words, skip the first -YYN actions for
             this state because they are default actions.  */
          int yyxbegin = yyn < 0 ? -yyn : 0;
          /* Stay within bounds of both yycheck and yytname.  */
          int yychecklim = YYLAST - yyn + 1;
          int yyxend = yychecklim < YYNTOKENS ? yychecklim : YYNTOKENS;
          int yyx;

          for (yyx = yyxbegin; yyx < yyxend; ++yyx)
            if (yycheck[yyx + yyn] == yyx && yyx != YYTERROR
                && !yytable_value_is_error (yytable[yyx + yyn]))
              {
                if (yycount == YYERROR_VERBOSE_ARGS_MAXIMUM)
                  {
                    yycount = 1;
                    yysize = yysize0;
                    break;
                  }
                yyarg[yycount++] = yytname[yyx];
                {
                  YYSIZE_T yysize1 = yysize + yytnamerr (YY_NULLPTR, yytname[yyx]);
                  if (! (yysize <= yysize1
                         && yysize1 <= YYSTACK_ALLOC_MAXIMUM))
                    return 2;
                  yysize = yysize1;
                }
              }
        }
    }

  switch (yycount)
    {
# define YYCASE_(N, S)                      \
      case N:                               \
        yyformat = S;                       \
      break
      YYCASE_(0, YY_("syntax error"));
      YYCASE_(1, YY_("syntax error, unexpected %s"));
      YYCASE_(2, YY_("syntax error, unexpected %s, expecting %s"));
      YYCASE_(3, YY_("syntax error, unexpected %s, expecting %s or %s"));
      YYCASE_(4, YY_("syntax error, unexpected %s, expecting %s or %s or %s"));
      YYCASE_(5, YY_("syntax error, unexpected %s, expecting %s or %s or %s or %s"));
# undef YYCASE_
    }

  {
    YYSIZE_T yysize1 = yysize + yystrlen (yyformat);
    if (! (yysize <= yysize1 && yysize1 <= YYSTACK_ALLOC_MAXIMUM))
      return 2;
    yysize = yysize1;
  }

  if (*yymsg_alloc < yysize)
    {
      *yymsg_alloc = 2 * yysize;
      if (! (yysize <= *yymsg_alloc
             && *yymsg_alloc <= YYSTACK_ALLOC_MAXIMUM))
        *yymsg_alloc = YYSTACK_ALLOC_MAXIMUM;
      return 1;
    }

  /* Avoid sprintf, as that infringes on the user's name space.
     Don't have undefined behavior even if the translation
     produced a string with the wrong number of "%s"s.  */
  {
    char *yyp = *yymsg;
    int yyi = 0;
    while ((*yyp = *yyformat) != '\0')
      if (*yyp == '%' && yyformat[1] == 's' && yyi < yycount)
        {
          yyp += yytnamerr (yyp, yyarg[yyi++]);
          yyformat += 2;
        }
      else
        {
          yyp++;
          yyformat++;
        }
  }
  return 0;
}
#endif /* YYERROR_VERBOSE */

/*-----------------------------------------------.
| Release the memory associated to this symbol.  |
`-----------------------------------------------*/

static void
yydestruct (const char *yymsg, int yytype, YYSTYPE *yyvaluep)
{
  YYUSE (yyvaluep);
  if (!yymsg)
    yymsg = "Deleting";
  YY_SYMBOL_PRINT (yymsg, yytype, yyvaluep, yylocationp);

  YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
  YYUSE (yytype);
  YY_IGNORE_MAYBE_UNINITIALIZED_END
}




/* The lookahead symbol.  */
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
    int yystate;
    /* Number of tokens to shift before error messages enabled.  */
    int yyerrstatus;

    /* The stacks and their tools:
       'yyss': related to states.
       'yyvs': related to semantic values.

       Refer to the stacks through separate pointers, to allow yyoverflow
       to reallocate them elsewhere.  */

    /* The state stack.  */
    yytype_int16 yyssa[YYINITDEPTH];
    yytype_int16 *yyss;
    yytype_int16 *yyssp;

    /* The semantic value stack.  */
    YYSTYPE yyvsa[YYINITDEPTH];
    YYSTYPE *yyvs;
    YYSTYPE *yyvsp;

    YYSIZE_T yystacksize;

  int yyn;
  int yyresult;
  /* Lookahead token as an internal (translated) token number.  */
  int yytoken = 0;
  /* The variables used to return semantic value and location from the
     action routines.  */
  YYSTYPE yyval;

#if YYERROR_VERBOSE
  /* Buffer for error messages, and its allocated size.  */
  char yymsgbuf[128];
  char *yymsg = yymsgbuf;
  YYSIZE_T yymsg_alloc = sizeof yymsgbuf;
#endif

#define YYPOPSTACK(N)   (yyvsp -= (N), yyssp -= (N))

  /* The number of symbols on the RHS of the reduced rule.
     Keep to zero when no symbol should be popped.  */
  int yylen = 0;

  yyssp = yyss = yyssa;
  yyvsp = yyvs = yyvsa;
  yystacksize = YYINITDEPTH;

  YYDPRINTF ((stderr, "Starting parse\n"));

  yystate = 0;
  yyerrstatus = 0;
  yynerrs = 0;
  yychar = YYEMPTY; /* Cause a token to be read.  */
  goto yysetstate;

/*------------------------------------------------------------.
| yynewstate -- Push a new state, which is found in yystate.  |
`------------------------------------------------------------*/
 yynewstate:
  /* In all cases, when you get here, the value and location stacks
     have just been pushed.  So pushing a state here evens the stacks.  */
  yyssp++;

 yysetstate:
  *yyssp = yystate;

  if (yyss + yystacksize - 1 <= yyssp)
    {
      /* Get the current used size of the three stacks, in elements.  */
      YYSIZE_T yysize = yyssp - yyss + 1;

#ifdef yyoverflow
      {
        /* Give user a chance to reallocate the stack.  Use copies of
           these so that the &'s don't force the real ones into
           memory.  */
        YYSTYPE *yyvs1 = yyvs;
        yytype_int16 *yyss1 = yyss;

        /* Each stack pointer address is followed by the size of the
           data in use in that stack, in bytes.  This used to be a
           conditional around just the two extra args, but that might
           be undefined if yyoverflow is a macro.  */
        yyoverflow (YY_("memory exhausted"),
                    &yyss1, yysize * sizeof (*yyssp),
                    &yyvs1, yysize * sizeof (*yyvsp),
                    &yystacksize);

        yyss = yyss1;
        yyvs = yyvs1;
      }
#else /* no yyoverflow */
# ifndef YYSTACK_RELOCATE
      goto yyexhaustedlab;
# else
      /* Extend the stack our own way.  */
      if (YYMAXDEPTH <= yystacksize)
        goto yyexhaustedlab;
      yystacksize *= 2;
      if (YYMAXDEPTH < yystacksize)
        yystacksize = YYMAXDEPTH;

      {
        yytype_int16 *yyss1 = yyss;
        union yyalloc *yyptr =
          (union yyalloc *) YYSTACK_ALLOC (YYSTACK_BYTES (yystacksize));
        if (! yyptr)
          goto yyexhaustedlab;
        YYSTACK_RELOCATE (yyss_alloc, yyss);
        YYSTACK_RELOCATE (yyvs_alloc, yyvs);
#  undef YYSTACK_RELOCATE
        if (yyss1 != yyssa)
          YYSTACK_FREE (yyss1);
      }
# endif
#endif /* no yyoverflow */

      yyssp = yyss + yysize - 1;
      yyvsp = yyvs + yysize - 1;

      YYDPRINTF ((stderr, "Stack size increased to %lu\n",
                  (unsigned long int) yystacksize));

      if (yyss + yystacksize - 1 <= yyssp)
        YYABORT;
    }

  YYDPRINTF ((stderr, "Entering state %d\n", yystate));

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

  /* YYCHAR is either YYEMPTY or YYEOF or a valid lookahead symbol.  */
  if (yychar == YYEMPTY)
    {
      YYDPRINTF ((stderr, "Reading a token: "));
      yychar = yylex ();
    }

  if (yychar <= YYEOF)
    {
      yychar = yytoken = YYEOF;
      YYDPRINTF ((stderr, "Now at end of input.\n"));
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

  /* Discard the shifted token.  */
  yychar = YYEMPTY;

  yystate = yyn;
  YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
  *++yyvsp = yylval;
  YY_IGNORE_MAYBE_UNINITIALIZED_END

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
| yyreduce -- Do a reduction.  |
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
        case 2:
#line 72 "parser.y" /* yacc.c:1646  */
    { 
                    (yyval).top_level = new TopLevel; 
                    program = (yyval).top_level; 
                }
#line 1397 "parser.cpp" /* yacc.c:1646  */
    break;

  case 3:
#line 77 "parser.y" /* yacc.c:1646  */
    { 
                    (yyval).top_level = (yyvsp[-1]).top_level; 
                    (yyval).top_level->addUnit((yyvsp[0]).decl); 
                }
#line 1406 "parser.cpp" /* yacc.c:1646  */
    break;

  case 4:
#line 82 "parser.y" /* yacc.c:1646  */
    {  
                    (yyval).top_level = (yyvsp[-1]).top_level;
                    (yyval).top_level->addUnit((yyvsp[0]).func_def); 
                }
#line 1415 "parser.cpp" /* yacc.c:1646  */
    break;

  case 5:
#line 91 "parser.y" /* yacc.c:1646  */
    {
                    (yyval).decl = (yyvsp[0]).decl; 
                }
#line 1423 "parser.cpp" /* yacc.c:1646  */
    break;

  case 6:
#line 95 "parser.y" /* yacc.c:1646  */
    {
                    (yyval).decl = (yyvsp[0]).decl; 
                }
#line 1431 "parser.cpp" /* yacc.c:1646  */
    break;

  case 7:
#line 109 "parser.y" /* yacc.c:1646  */
    { 
                    msg("Const Declaration");
                    (yyval).decl = (yyvsp[-1]).decl; 
                    (yyval).decl->setConst();  
                }
#line 1441 "parser.cpp" /* yacc.c:1646  */
    break;

  case 8:
#line 117 "parser.y" /* yacc.c:1646  */
    { 
                    msg("Variable Declaration"); 
                    (yyval).decl = (yyvsp[-1]).decl; 
                    (yyval).decl->setVar(); 
                }
#line 1451 "parser.cpp" /* yacc.c:1646  */
    break;

  case 9:
#line 125 "parser.y" /* yacc.c:1646  */
    {
                    msg("Start of VarDef List"); 
                    (yyval).decl = new Decl(); 
                    (yyval).decl->insert((yyvsp[0]).var_def);
                }
#line 1461 "parser.cpp" /* yacc.c:1646  */
    break;

  case 10:
#line 131 "parser.y" /* yacc.c:1646  */
    {
                    msg("Append VarDef List");
                    (yyval).decl = (yyvsp[-2]).decl; 
                    assert((yyval).decl != nullptr); 
                    (yyval).decl->insert((yyvsp[0]).var_def);
                }
#line 1472 "parser.cpp" /* yacc.c:1646  */
    break;

  case 11:
#line 141 "parser.y" /* yacc.c:1646  */
    { 
                    msg("VarDef with default value");  
                    (yyval).var_def = new VarDef(new Ident((yyvsp[-3]).str), 
                                            (yyvsp[-2]).array_index, 
                                            (yyvsp[0]).init_val, false);
                }
#line 1483 "parser.cpp" /* yacc.c:1646  */
    break;

  case 12:
#line 148 "parser.y" /* yacc.c:1646  */
    { 
                    msg("VarDef without default value");
                    (yyval).var_def = new VarDef(new Ident((yyvsp[-1]).str),
                                            (yyvsp[0]).array_index,
                                            nullptr, false);  
                }
#line 1494 "parser.cpp" /* yacc.c:1646  */
    break;

  case 13:
#line 158 "parser.y" /* yacc.c:1646  */
    {
                    msg("ArrayIndex list insert, size = " 
                        + std::to_string((yyvsp[-3]).array_index->get_size()) 
                        + " + 1");
                    (yyval).array_index = (yyvsp[-3]).array_index; 
                    assert((yyval).array_index != nullptr);
                    (yyval).array_index->insert((yyvsp[-1]).expr);
                }
#line 1507 "parser.cpp" /* yacc.c:1646  */
    break;

  case 14:
#line 167 "parser.y" /* yacc.c:1646  */
    {  
                    msg("Start of ArrayIndex list"); 
                    (yyval).array_index = new ArrayIndex(); 
                }
#line 1516 "parser.cpp" /* yacc.c:1646  */
    break;

  case 15:
#line 174 "parser.y" /* yacc.c:1646  */
    {  
                    msg("Init value as expression"); 
                    (yyval).init_val = new ExprInitVal((yyvsp[0]).expr);
                    // Init value of single element 
                }
#line 1526 "parser.cpp" /* yacc.c:1646  */
    break;

  case 16:
#line 180 "parser.y" /* yacc.c:1646  */
    {
                    msg("Init value as (empty) array assignment"); 
                    (yyval).init_val = new ArrayInitVal(); 
                }
#line 1535 "parser.cpp" /* yacc.c:1646  */
    break;

  case 17:
#line 185 "parser.y" /* yacc.c:1646  */
    {  
                    msg("Init value as (non-empty) array assignment"); 
                    (yyval).init_val = (yyvsp[-1]).array_init_val;
                }
#line 1544 "parser.cpp" /* yacc.c:1646  */
    break;

  case 18:
#line 192 "parser.y" /* yacc.c:1646  */
    {
                    msg("Append InitVal List");
                    (yyval).array_init_val = (yyvsp[-2]).array_init_val; 
                    (yyval).array_init_val->insert((yyvsp[0]).init_val);
                }
#line 1554 "parser.cpp" /* yacc.c:1646  */
    break;

  case 19:
#line 198 "parser.y" /* yacc.c:1646  */
    {
                    msg("Start of InitVal List"); 
                    (yyval).array_init_val = new ArrayInitVal();
                    (yyval).array_init_val->insert((yyvsp[0]).init_val);
                }
#line 1564 "parser.cpp" /* yacc.c:1646  */
    break;

  case 20:
#line 208 "parser.y" /* yacc.c:1646  */
    { 
                    msg("Function define with integer return value"); 
                    (yyval).func_def = new FuncDef(FuncDef::INT, 
                                            new Ident((yyvsp[-4]).str),
                                            (yyvsp[-2]).fparam_list, 
                                            (yyvsp[0]).block);
                }
#line 1576 "parser.cpp" /* yacc.c:1646  */
    break;

  case 21:
#line 216 "parser.y" /* yacc.c:1646  */
    { 
                    msg("Function define with no return value");  
                    (yyval).func_def = new FuncDef(FuncDef::VOID, 
                                            new Ident((yyvsp[-4]).str),
                                            (yyvsp[-2]).fparam_list, 
                                            (yyvsp[0]).block);
                }
#line 1588 "parser.cpp" /* yacc.c:1646  */
    break;

  case 22:
#line 226 "parser.y" /* yacc.c:1646  */
    { 
                        (yyval).fparam_list = (yyvsp[0]).fparam_list;
                    }
#line 1596 "parser.cpp" /* yacc.c:1646  */
    break;

  case 23:
#line 230 "parser.y" /* yacc.c:1646  */
    { 
                        (yyval).fparam_list.clear(); 
                    }
#line 1604 "parser.cpp" /* yacc.c:1646  */
    break;

  case 24:
#line 236 "parser.y" /* yacc.c:1646  */
    {  
                        msg("Append function formal param list");
                        (yyval).fparam_list = (yyvsp[-2]).fparam_list; 
                        (yyval).fparam_list.push_back((yyvsp[0]).fparam); 
                    }
#line 1614 "parser.cpp" /* yacc.c:1646  */
    break;

  case 25:
#line 242 "parser.y" /* yacc.c:1646  */
    {  
                        msg("Start of function formal param List"); 
                        (yyval).fparam_list.push_back((yyvsp[0]).fparam);
                    }
#line 1623 "parser.cpp" /* yacc.c:1646  */
    break;

  case 26:
#line 249 "parser.y" /* yacc.c:1646  */
    {  
                    msg("Integer as formal parameter");
                    (yyval).fparam = new FuncFParam(new Ident((yyvsp[0]).str), vector<Expr*>());
                }
#line 1632 "parser.cpp" /* yacc.c:1646  */
    break;

  case 27:
#line 254 "parser.y" /* yacc.c:1646  */
    {
                    msg("Array as formal parameter");
                    assert((yyvsp[0]).array_index != nullptr); 
                    vector<Expr*> ai_list = (yyvsp[0]).array_index->AIlist; 
                    delete (yyvsp[0]).array_index;  
                    // will not used 
                    vector<Expr*> ai_list_p = {nullptr};
                    for (auto expr : ai_list) {
                        ai_list_p.push_back(expr); 
                    }
                    (yyval).fparam = new FuncFParam(new Ident((yyvsp[-3]).str), ai_list_p); 
                }
#line 1649 "parser.cpp" /* yacc.c:1646  */
    break;

  case 28:
#line 271 "parser.y" /* yacc.c:1646  */
    { 
                    (yyval).block = (yyvsp[-1]).block;
                }
#line 1657 "parser.cpp" /* yacc.c:1646  */
    break;

  case 29:
#line 281 "parser.y" /* yacc.c:1646  */
    {
                    msg("Append statement list with statement");
                    (yyval).block = (yyvsp[-1]).block; 
                    (yyval).block->addStmt((yyvsp[0]).stmt);
                }
#line 1667 "parser.cpp" /* yacc.c:1646  */
    break;

  case 30:
#line 287 "parser.y" /* yacc.c:1646  */
    {
                    msg("Append statement list with declaration");
                    (yyval).block = (yyvsp[-1]).block;
                    (yyval).block->addDecl((yyvsp[0]).decl); 
                }
#line 1677 "parser.cpp" /* yacc.c:1646  */
    break;

  case 31:
#line 293 "parser.y" /* yacc.c:1646  */
    { 
                    msg("Start of statement list"); 
                    (yyval).block = new Block(); 
                }
#line 1686 "parser.cpp" /* yacc.c:1646  */
    break;

  case 32:
#line 300 "parser.y" /* yacc.c:1646  */
    {
                    msg("Assign statement");
                    (yyval).stmt = new AssignStmt((yyvsp[-3]).lval, (yyvsp[-1]).expr);
                }
#line 1695 "parser.cpp" /* yacc.c:1646  */
    break;

  case 33:
#line 305 "parser.y" /* yacc.c:1646  */
    {
                    msg("Block statement");
                    (yyval).stmt = (yyvsp[0]).block; 
                }
#line 1704 "parser.cpp" /* yacc.c:1646  */
    break;

  case 34:
#line 310 "parser.y" /* yacc.c:1646  */
    {
                    msg("Expr statement");
                    (yyval).stmt = new ExprStmt((yyvsp[-1]).expr); 
                }
#line 1713 "parser.cpp" /* yacc.c:1646  */
    break;

  case 35:
#line 315 "parser.y" /* yacc.c:1646  */
    {
                    msg("Empty statement");
                    (yyval).stmt = new EmptyStmt(); 
                }
#line 1722 "parser.cpp" /* yacc.c:1646  */
    break;

  case 36:
#line 320 "parser.y" /* yacc.c:1646  */
    { 
                    msg("If statement");
                    (yyval).stmt = new IfStmt((yyvsp[-2]).expr, (yyvsp[0]).stmt);
                }
#line 1731 "parser.cpp" /* yacc.c:1646  */
    break;

  case 37:
#line 325 "parser.y" /* yacc.c:1646  */
    { 
                    msg("If-else statement"); 
                    (yyval).stmt = new IfStmt((yyvsp[-4]).expr, (yyvsp[-2]).stmt, (yyvsp[0]).stmt);
                }
#line 1740 "parser.cpp" /* yacc.c:1646  */
    break;

  case 38:
#line 330 "parser.y" /* yacc.c:1646  */
    {
                    msg("While statement");
                    (yyval).stmt = new WhileStmt((yyvsp[-2]).expr, (yyvsp[0]).stmt);
                }
#line 1749 "parser.cpp" /* yacc.c:1646  */
    break;

  case 39:
#line 335 "parser.y" /* yacc.c:1646  */
    { 
                    msg("Break statement");
                    (yyval).stmt = new BreakStmt(); 
                }
#line 1758 "parser.cpp" /* yacc.c:1646  */
    break;

  case 40:
#line 340 "parser.y" /* yacc.c:1646  */
    {
                    msg("Continue statement");
                    (yyval).stmt = new ContinueStmt(); 
                }
#line 1767 "parser.cpp" /* yacc.c:1646  */
    break;

  case 41:
#line 345 "parser.y" /* yacc.c:1646  */
    { 
                    msg("Return statement with no return value");
                    (yyval).stmt = new ReturnStmt(); 
                }
#line 1776 "parser.cpp" /* yacc.c:1646  */
    break;

  case 42:
#line 350 "parser.y" /* yacc.c:1646  */
    {
                    msg("Return statement with integer value");
                    (yyval).stmt = new ReturnStmt((yyvsp[-1]).expr);
                }
#line 1785 "parser.cpp" /* yacc.c:1646  */
    break;

  case 43:
#line 359 "parser.y" /* yacc.c:1646  */
    { 
                    (yyval).lval = new Lval(new Ident((yyvsp[-1]).str));
                    (yyval).lval->AI = (yyvsp[0]).array_index; 
                }
#line 1794 "parser.cpp" /* yacc.c:1646  */
    break;

  case 44:
#line 366 "parser.y" /* yacc.c:1646  */
    { 
                    (yyval).expr = new IntConst((yyvsp[0]).val); 
                }
#line 1802 "parser.cpp" /* yacc.c:1646  */
    break;

  case 45:
#line 370 "parser.y" /* yacc.c:1646  */
    { 
                    (yyval).expr = new StringExpr((yyvsp[0]).str);
                }
#line 1810 "parser.cpp" /* yacc.c:1646  */
    break;

  case 46:
#line 374 "parser.y" /* yacc.c:1646  */
    {
                    // left value is a special expression
                    (yyval).expr = (yyvsp[0]).lval; 
                }
#line 1819 "parser.cpp" /* yacc.c:1646  */
    break;

  case 47:
#line 379 "parser.y" /* yacc.c:1646  */
    {
                    (yyval).expr = (yyvsp[0]).func_call;
                }
#line 1827 "parser.cpp" /* yacc.c:1646  */
    break;

  case 48:
#line 383 "parser.y" /* yacc.c:1646  */
    {
                    (yyval).expr = (yyvsp[-1]).expr; 
                }
#line 1835 "parser.cpp" /* yacc.c:1646  */
    break;

  case 49:
#line 387 "parser.y" /* yacc.c:1646  */
    {
                    (yyval).expr = new UnaryExpr(UnaryExpr::POS, (yyvsp[0]).expr); 
                }
#line 1843 "parser.cpp" /* yacc.c:1646  */
    break;

  case 50:
#line 391 "parser.y" /* yacc.c:1646  */
    {
                    (yyval).expr = new UnaryExpr(UnaryExpr::NEG, (yyvsp[0]).expr);
                }
#line 1851 "parser.cpp" /* yacc.c:1646  */
    break;

  case 51:
#line 395 "parser.y" /* yacc.c:1646  */
    {
                    (yyval).expr = new UnaryExpr(UnaryExpr::NOT ,(yyvsp[0]).expr);
                }
#line 1859 "parser.cpp" /* yacc.c:1646  */
    break;

  case 52:
#line 399 "parser.y" /* yacc.c:1646  */
    {
                    (yyval).expr = new BinaryExpr(BinaryExpr::ADD, (yyvsp[-2]).expr, (yyvsp[0]).expr); 
                }
#line 1867 "parser.cpp" /* yacc.c:1646  */
    break;

  case 53:
#line 403 "parser.y" /* yacc.c:1646  */
    {
                    (yyval).expr = new BinaryExpr(BinaryExpr::SUB, (yyvsp[-2]).expr, (yyvsp[0]).expr);
                }
#line 1875 "parser.cpp" /* yacc.c:1646  */
    break;

  case 54:
#line 407 "parser.y" /* yacc.c:1646  */
    { 
                    (yyval).expr = new BinaryExpr(BinaryExpr::MUL, (yyvsp[-2]).expr, (yyvsp[0]).expr);
                }
#line 1883 "parser.cpp" /* yacc.c:1646  */
    break;

  case 55:
#line 411 "parser.y" /* yacc.c:1646  */
    {
                    (yyval).expr = new BinaryExpr(BinaryExpr::DIV, (yyvsp[-2]).expr, (yyvsp[0]).expr);
                }
#line 1891 "parser.cpp" /* yacc.c:1646  */
    break;

  case 56:
#line 415 "parser.y" /* yacc.c:1646  */
    {
                    (yyval).expr = new BinaryExpr(BinaryExpr::MOD, (yyvsp[-2]).expr, (yyvsp[0]).expr);
                }
#line 1899 "parser.cpp" /* yacc.c:1646  */
    break;

  case 57:
#line 419 "parser.y" /* yacc.c:1646  */
    {
                    (yyval).expr = new BinaryExpr(BinaryExpr::LT, (yyvsp[-2]).expr, (yyvsp[0]).expr);
                }
#line 1907 "parser.cpp" /* yacc.c:1646  */
    break;

  case 58:
#line 423 "parser.y" /* yacc.c:1646  */
    {
                    (yyval).expr = new BinaryExpr(BinaryExpr::GT, (yyvsp[-2]).expr, (yyvsp[0]).expr);
                }
#line 1915 "parser.cpp" /* yacc.c:1646  */
    break;

  case 59:
#line 427 "parser.y" /* yacc.c:1646  */
    {
                    (yyval).expr = new BinaryExpr(BinaryExpr::LQ, (yyvsp[-2]).expr, (yyvsp[0]).expr);
                }
#line 1923 "parser.cpp" /* yacc.c:1646  */
    break;

  case 60:
#line 431 "parser.y" /* yacc.c:1646  */
    { 
                    (yyval).expr = new BinaryExpr(BinaryExpr::GQ, (yyvsp[-2]).expr, (yyvsp[0]).expr);
                }
#line 1931 "parser.cpp" /* yacc.c:1646  */
    break;

  case 61:
#line 435 "parser.y" /* yacc.c:1646  */
    {
                    (yyval).expr = new BinaryExpr(BinaryExpr::EQ, (yyvsp[-2]).expr, (yyvsp[0]).expr);
                }
#line 1939 "parser.cpp" /* yacc.c:1646  */
    break;

  case 62:
#line 439 "parser.y" /* yacc.c:1646  */
    {
                    (yyval).expr = new BinaryExpr(BinaryExpr::NEQ, (yyvsp[-2]).expr, (yyvsp[0]).expr);
                }
#line 1947 "parser.cpp" /* yacc.c:1646  */
    break;

  case 63:
#line 443 "parser.y" /* yacc.c:1646  */
    {
                    (yyval).expr = new BinaryExpr(BinaryExpr::AND, (yyvsp[-2]).expr, (yyvsp[0]).expr);
                }
#line 1955 "parser.cpp" /* yacc.c:1646  */
    break;

  case 64:
#line 447 "parser.y" /* yacc.c:1646  */
    {
                    (yyval).expr = new BinaryExpr(BinaryExpr::OR, (yyvsp[-2]).expr, (yyvsp[0]).expr);
                }
#line 1963 "parser.cpp" /* yacc.c:1646  */
    break;

  case 65:
#line 453 "parser.y" /* yacc.c:1646  */
    {
                    (yyval).func_call = new FuncCall(new Ident((yyvsp[-3]).str), (yyvsp[-1]).expr_list);
                }
#line 1971 "parser.cpp" /* yacc.c:1646  */
    break;

  case 66:
#line 459 "parser.y" /* yacc.c:1646  */
    { 
                        msg("Start of function real param List"); 
                        (yyval).expr_list = std::vector<Expr*>({(yyvsp[0]).expr});
                    }
#line 1980 "parser.cpp" /* yacc.c:1646  */
    break;

  case 67:
#line 464 "parser.y" /* yacc.c:1646  */
    {
                        msg("Append function real param list");
                        (yyval).expr_list = (yyvsp[-2]).expr_list; 
                        (yyval).expr_list.push_back((yyvsp[0]).expr);
                    }
#line 1990 "parser.cpp" /* yacc.c:1646  */
    break;

  case 68:
#line 472 "parser.y" /* yacc.c:1646  */
    { 
                        (yyval).expr_list = (yyvsp[0]).expr_list; 
                    }
#line 1998 "parser.cpp" /* yacc.c:1646  */
    break;

  case 69:
#line 476 "parser.y" /* yacc.c:1646  */
    { 
                        (yyval).expr_list = std::vector<Expr*>(); 
                    }
#line 2006 "parser.cpp" /* yacc.c:1646  */
    break;


#line 2010 "parser.cpp" /* yacc.c:1646  */
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
  YY_SYMBOL_PRINT ("-> $$ =", yyr1[yyn], &yyval, &yyloc);

  YYPOPSTACK (yylen);
  yylen = 0;
  YY_STACK_PRINT (yyss, yyssp);

  *++yyvsp = yyval;

  /* Now 'shift' the result of the reduction.  Determine what state
     that goes to, based on the state we popped back to and the rule
     number reduced by.  */

  yyn = yyr1[yyn];

  yystate = yypgoto[yyn - YYNTOKENS] + *yyssp;
  if (0 <= yystate && yystate <= YYLAST && yycheck[yystate] == *yyssp)
    yystate = yytable[yystate];
  else
    yystate = yydefgoto[yyn - YYNTOKENS];

  goto yynewstate;


/*--------------------------------------.
| yyerrlab -- here on detecting error.  |
`--------------------------------------*/
yyerrlab:
  /* Make sure we have latest lookahead translation.  See comments at
     user semantic actions for why this is necessary.  */
  yytoken = yychar == YYEMPTY ? YYEMPTY : YYTRANSLATE (yychar);

  /* If not already recovering from an error, report this error.  */
  if (!yyerrstatus)
    {
      ++yynerrs;
#if ! YYERROR_VERBOSE
      yyerror (YY_("syntax error"));
#else
# define YYSYNTAX_ERROR yysyntax_error (&yymsg_alloc, &yymsg, \
                                        yyssp, yytoken)
      {
        char const *yymsgp = YY_("syntax error");
        int yysyntax_error_status;
        yysyntax_error_status = YYSYNTAX_ERROR;
        if (yysyntax_error_status == 0)
          yymsgp = yymsg;
        else if (yysyntax_error_status == 1)
          {
            if (yymsg != yymsgbuf)
              YYSTACK_FREE (yymsg);
            yymsg = (char *) YYSTACK_ALLOC (yymsg_alloc);
            if (!yymsg)
              {
                yymsg = yymsgbuf;
                yymsg_alloc = sizeof yymsgbuf;
                yysyntax_error_status = 2;
              }
            else
              {
                yysyntax_error_status = YYSYNTAX_ERROR;
                yymsgp = yymsg;
              }
          }
        yyerror (yymsgp);
        if (yysyntax_error_status == 2)
          goto yyexhaustedlab;
      }
# undef YYSYNTAX_ERROR
#endif
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

  /* Pacify compilers like GCC when the user code never invokes
     YYERROR and the label yyerrorlab therefore never appears in user
     code.  */
  if (/*CONSTCOND*/ 0)
     goto yyerrorlab;

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

  for (;;)
    {
      yyn = yypact[yystate];
      if (!yypact_value_is_default (yyn))
        {
          yyn += YYTERROR;
          if (0 <= yyn && yyn <= YYLAST && yycheck[yyn] == YYTERROR)
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
                  yystos[yystate], yyvsp);
      YYPOPSTACK (1);
      yystate = *yyssp;
      YY_STACK_PRINT (yyss, yyssp);
    }

  YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
  *++yyvsp = yylval;
  YY_IGNORE_MAYBE_UNINITIALIZED_END


  /* Shift the error token.  */
  YY_SYMBOL_PRINT ("Shifting", yystos[yyn], yyvsp, yylsp);

  yystate = yyn;
  goto yynewstate;


/*-------------------------------------.
| yyacceptlab -- YYACCEPT comes here.  |
`-------------------------------------*/
yyacceptlab:
  yyresult = 0;
  goto yyreturn;

/*-----------------------------------.
| yyabortlab -- YYABORT comes here.  |
`-----------------------------------*/
yyabortlab:
  yyresult = 1;
  goto yyreturn;

#if !defined yyoverflow || YYERROR_VERBOSE
/*-------------------------------------------------.
| yyexhaustedlab -- memory exhaustion comes here.  |
`-------------------------------------------------*/
yyexhaustedlab:
  yyerror (YY_("memory exhausted"));
  yyresult = 2;
  /* Fall through.  */
#endif

yyreturn:
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
                  yystos[*yyssp], yyvsp);
      YYPOPSTACK (1);
    }
#ifndef yyoverflow
  if (yyss != yyssa)
    YYSTACK_FREE (yyss);
#endif
#if YYERROR_VERBOSE
  if (yymsg != yymsgbuf)
    YYSTACK_FREE (yymsg);
#endif
  return yyresult;
}
#line 480 "parser.y" /* yacc.c:1906  */


TopLevel* run_parser() {
    yyin = fopen(FILE_IN.c_str(), "r");
    yyout = fopen(FILE_OUT.c_str(), "w");
    program = nullptr; 
    int ret = yyparse();
    if (ret != 0)
        program = nullptr;  
    else if (PRINT_AST) {
        DisplayVisitor v(4, AST_NAME); 
        program->accept(&v);
    }
    fclose(yyin);
    fclose(yyout);
    return program;
}

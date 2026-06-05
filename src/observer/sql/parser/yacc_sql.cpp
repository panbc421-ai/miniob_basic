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
#define YYPURE 2

/* Push parsers.  */
#define YYPUSH 0

/* Pull parsers.  */
#define YYPULL 1




/* First part of user prologue.  */
#line 2 "yacc_sql.y"


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <algorithm>

#include "common/log/log.h"
#include "common/lang/string.h"
#include "sql/parser/parse_defs.h"
#include "sql/parser/yacc_sql.hpp"
#include "sql/parser/lex_sql.h"
#include "sql/expr/expression.h"
#include "sql/parser/value.h"

// 日期验证函数声明
static bool check_date(int y, int m, int d)
{
  // Validate year/month/day ranges. Accept years in [1, 9999].
  if (y < 1 || y > 9999) {
    return false;
  }
  if (m < 1 || m > 12 || d < 1) {
    return false;
  }

  static const int days_of_month[] = {0, 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
  bool leap = (y % 4 == 0 && y % 100 != 0) || (y % 400 == 0);
  int max_day = days_of_month[m];
  if (m == 2 && leap) {
    max_day = 29;
  }
  return d <= max_day;
}
using namespace std;

string token_name(const char *sql_string, YYLTYPE *llocp)
{
  return string(sql_string + llocp->first_column, llocp->last_column - llocp->first_column + 1);
}

int yyerror(YYLTYPE *llocp, const char *sql_string, ParsedSqlResult *sql_result, yyscan_t scanner, const char *msg)
{
  std::unique_ptr<ParsedSqlNode> error_sql_node = std::make_unique<ParsedSqlNode>(SCF_ERROR);
  error_sql_node->error.error_msg = msg;
  error_sql_node->error.line = llocp->first_line;
  error_sql_node->error.column = llocp->first_column;
  sql_result->add_sql_node(std::move(error_sql_node));
  return 0;
}

ArithmeticExpr *create_arithmetic_expression(ArithmeticExpr::Type type,
                                             Expression *left,
                                             Expression *right,
                                             const char *sql_string,
                                             YYLTYPE *llocp)
{
  ArithmeticExpr *expr = new ArithmeticExpr(type, left, right);
  expr->set_name(token_name(sql_string, llocp));
  return expr;
}


#line 135 "yacc_sql.cpp"

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

#include "yacc_sql.hpp"
/* Symbol kind.  */
enum yysymbol_kind_t
{
  YYSYMBOL_YYEMPTY = -2,
  YYSYMBOL_YYEOF = 0,                      /* "end of file"  */
  YYSYMBOL_YYerror = 1,                    /* error  */
  YYSYMBOL_YYUNDEF = 2,                    /* "invalid token"  */
  YYSYMBOL_SEMICOLON = 3,                  /* SEMICOLON  */
  YYSYMBOL_CREATE = 4,                     /* CREATE  */
  YYSYMBOL_DROP = 5,                       /* DROP  */
  YYSYMBOL_TABLE = 6,                      /* TABLE  */
  YYSYMBOL_TABLES = 7,                     /* TABLES  */
  YYSYMBOL_INDEX = 8,                      /* INDEX  */
  YYSYMBOL_CALC = 9,                       /* CALC  */
  YYSYMBOL_SELECT = 10,                    /* SELECT  */
  YYSYMBOL_DESC = 11,                      /* DESC  */
  YYSYMBOL_SHOW = 12,                      /* SHOW  */
  YYSYMBOL_SYNC = 13,                      /* SYNC  */
  YYSYMBOL_INSERT = 14,                    /* INSERT  */
  YYSYMBOL_DELETE = 15,                    /* DELETE  */
  YYSYMBOL_UPDATE = 16,                    /* UPDATE  */
  YYSYMBOL_LBRACE = 17,                    /* LBRACE  */
  YYSYMBOL_RBRACE = 18,                    /* RBRACE  */
  YYSYMBOL_COMMA = 19,                     /* COMMA  */
  YYSYMBOL_DATE_T = 20,                    /* DATE_T  */
  YYSYMBOL_TRX_BEGIN = 21,                 /* TRX_BEGIN  */
  YYSYMBOL_TRX_COMMIT = 22,                /* TRX_COMMIT  */
  YYSYMBOL_TRX_ROLLBACK = 23,              /* TRX_ROLLBACK  */
  YYSYMBOL_INT_T = 24,                     /* INT_T  */
  YYSYMBOL_STRING_T = 25,                  /* STRING_T  */
  YYSYMBOL_FLOAT_T = 26,                   /* FLOAT_T  */
  YYSYMBOL_HELP = 27,                      /* HELP  */
  YYSYMBOL_EXIT = 28,                      /* EXIT  */
  YYSYMBOL_DOT = 29,                       /* DOT  */
  YYSYMBOL_INTO = 30,                      /* INTO  */
  YYSYMBOL_VALUES = 31,                    /* VALUES  */
  YYSYMBOL_MAX = 32,                       /* MAX  */
  YYSYMBOL_MIN = 33,                       /* MIN  */
  YYSYMBOL_COUNT = 34,                     /* COUNT  */
  YYSYMBOL_AVG = 35,                       /* AVG  */
  YYSYMBOL_SUM = 36,                       /* SUM  */
  YYSYMBOL_FROM = 37,                      /* FROM  */
  YYSYMBOL_WHERE = 38,                     /* WHERE  */
  YYSYMBOL_AND = 39,                       /* AND  */
  YYSYMBOL_LIKE = 40,                      /* LIKE  */
  YYSYMBOL_NOT = 41,                       /* NOT  */
  YYSYMBOL_SET = 42,                       /* SET  */
  YYSYMBOL_INNER = 43,                     /* INNER  */
  YYSYMBOL_JOIN = 44,                      /* JOIN  */
  YYSYMBOL_ON = 45,                        /* ON  */
  YYSYMBOL_LOAD = 46,                      /* LOAD  */
  YYSYMBOL_DATA = 47,                      /* DATA  */
  YYSYMBOL_INFILE = 48,                    /* INFILE  */
  YYSYMBOL_EXPLAIN = 49,                   /* EXPLAIN  */
  YYSYMBOL_ORDER = 50,                     /* ORDER  */
  YYSYMBOL_BY = 51,                        /* BY  */
  YYSYMBOL_ASC = 52,                       /* ASC  */
  YYSYMBOL_AS = 53,                        /* AS  */
  YYSYMBOL_GROUP = 54,                     /* GROUP  */
  YYSYMBOL_HAVING = 55,                    /* HAVING  */
  YYSYMBOL_EQ = 56,                        /* EQ  */
  YYSYMBOL_LT = 57,                        /* LT  */
  YYSYMBOL_GT = 58,                        /* GT  */
  YYSYMBOL_LE = 59,                        /* LE  */
  YYSYMBOL_GE = 60,                        /* GE  */
  YYSYMBOL_NE = 61,                        /* NE  */
  YYSYMBOL_IN = 62,                        /* IN  */
  YYSYMBOL_EXISTS = 63,                    /* EXISTS  */
  YYSYMBOL_UNIQUE = 64,                    /* UNIQUE  */
  YYSYMBOL_NULL_T = 65,                    /* NULL_T  */
  YYSYMBOL_NULLABLE_T = 66,                /* NULLABLE_T  */
  YYSYMBOL_TEXT_T = 67,                    /* TEXT_T  */
  YYSYMBOL_IS = 68,                        /* IS  */
  YYSYMBOL_NUMBER = 69,                    /* NUMBER  */
  YYSYMBOL_FLOAT = 70,                     /* FLOAT  */
  YYSYMBOL_ID = 71,                        /* ID  */
  YYSYMBOL_SSS = 72,                       /* SSS  */
  YYSYMBOL_73_ = 73,                       /* '+'  */
  YYSYMBOL_74_ = 74,                       /* '-'  */
  YYSYMBOL_75_ = 75,                       /* '*'  */
  YYSYMBOL_76_ = 76,                       /* '/'  */
  YYSYMBOL_UMINUS = 77,                    /* UMINUS  */
  YYSYMBOL_YYACCEPT = 78,                  /* $accept  */
  YYSYMBOL_commands = 79,                  /* commands  */
  YYSYMBOL_command_wrapper = 80,           /* command_wrapper  */
  YYSYMBOL_exit_stmt = 81,                 /* exit_stmt  */
  YYSYMBOL_help_stmt = 82,                 /* help_stmt  */
  YYSYMBOL_sync_stmt = 83,                 /* sync_stmt  */
  YYSYMBOL_begin_stmt = 84,                /* begin_stmt  */
  YYSYMBOL_commit_stmt = 85,               /* commit_stmt  */
  YYSYMBOL_rollback_stmt = 86,             /* rollback_stmt  */
  YYSYMBOL_drop_table_stmt = 87,           /* drop_table_stmt  */
  YYSYMBOL_show_tables_stmt = 88,          /* show_tables_stmt  */
  YYSYMBOL_desc_table_stmt = 89,           /* desc_table_stmt  */
  YYSYMBOL_create_index_stmt = 90,         /* create_index_stmt  */
  YYSYMBOL_drop_index_stmt = 91,           /* drop_index_stmt  */
  YYSYMBOL_create_table_stmt = 92,         /* create_table_stmt  */
  YYSYMBOL_attr_def_list = 93,             /* attr_def_list  */
  YYSYMBOL_attr_def = 94,                  /* attr_def  */
  YYSYMBOL_number = 95,                    /* number  */
  YYSYMBOL_type = 96,                      /* type  */
  YYSYMBOL_insert_stmt = 97,               /* insert_stmt  */
  YYSYMBOL_value_list = 98,                /* value_list  */
  YYSYMBOL_id_list = 99,                   /* id_list  */
  YYSYMBOL_value = 100,                    /* value  */
  YYSYMBOL_delete_stmt = 101,              /* delete_stmt  */
  YYSYMBOL_update_stmt = 102,              /* update_stmt  */
  YYSYMBOL_update_assign_list = 103,       /* update_assign_list  */
  YYSYMBOL_update_assign = 104,            /* update_assign  */
  YYSYMBOL_alias_opt = 105,                /* alias_opt  */
  YYSYMBOL_select_stmt = 106,              /* select_stmt  */
  YYSYMBOL_calc_stmt = 107,                /* calc_stmt  */
  YYSYMBOL_expression_list = 108,          /* expression_list  */
  YYSYMBOL_expression = 109,               /* expression  */
  YYSYMBOL_agg_expr = 110,                 /* agg_expr  */
  YYSYMBOL_select_attr = 111,              /* select_attr  */
  YYSYMBOL_select_expr_item = 112,         /* select_expr_item  */
  YYSYMBOL_select_expr_list = 113,         /* select_expr_list  */
  YYSYMBOL_rel_attr = 114,                 /* rel_attr  */
  YYSYMBOL_join_clause = 115,              /* join_clause  */
  YYSYMBOL_where = 116,                    /* where  */
  YYSYMBOL_group_by_clause = 117,          /* group_by_clause  */
  YYSYMBOL_group_by_list = 118,            /* group_by_list  */
  YYSYMBOL_having_clause = 119,            /* having_clause  */
  YYSYMBOL_condition_list = 120,           /* condition_list  */
  YYSYMBOL_condition = 121,                /* condition  */
  YYSYMBOL_comp_op = 122,                  /* comp_op  */
  YYSYMBOL_load_data_stmt = 123,           /* load_data_stmt  */
  YYSYMBOL_explain_stmt = 124,             /* explain_stmt  */
  YYSYMBOL_set_variable_stmt = 125,        /* set_variable_stmt  */
  YYSYMBOL_order_by_clause = 126,          /* order_by_clause  */
  YYSYMBOL_order_by_list = 127,            /* order_by_list  */
  YYSYMBOL_order_by_item = 128,            /* order_by_item  */
  YYSYMBOL_opt_asc_desc = 129,             /* opt_asc_desc  */
  YYSYMBOL_opt_semicolon = 130             /* opt_semicolon  */
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
typedef yytype_int16 yy_state_t;

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

#if 1

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
#endif /* 1 */

#if (! defined yyoverflow \
     && (! defined __cplusplus \
         || (defined YYLTYPE_IS_TRIVIAL && YYLTYPE_IS_TRIVIAL \
             && defined YYSTYPE_IS_TRIVIAL && YYSTYPE_IS_TRIVIAL)))

/* A type that is properly aligned for any stack member.  */
union yyalloc
{
  yy_state_t yyss_alloc;
  YYSTYPE yyvs_alloc;
  YYLTYPE yyls_alloc;
};

/* The size of the maximum gap between one aligned stack and the next.  */
# define YYSTACK_GAP_MAXIMUM (YYSIZEOF (union yyalloc) - 1)

/* The size of an array large to enough to hold all stacks, each with
   N elements.  */
# define YYSTACK_BYTES(N) \
     ((N) * (YYSIZEOF (yy_state_t) + YYSIZEOF (YYSTYPE) \
             + YYSIZEOF (YYLTYPE)) \
      + 2 * YYSTACK_GAP_MAXIMUM)

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
#define YYFINAL  77
/* YYLAST -- Last index in YYTABLE.  */
#define YYLAST   411

/* YYNTOKENS -- Number of terminals.  */
#define YYNTOKENS  78
/* YYNNTS -- Number of nonterminals.  */
#define YYNNTS  53
/* YYNRULES -- Number of rules.  */
#define YYNRULES  156
/* YYNSTATES -- Number of states.  */
#define YYNSTATES  336

/* YYMAXUTOK -- Last valid token kind.  */
#define YYMAXUTOK   328


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
       2,     2,    75,    73,     2,    74,     2,    76,     2,     2,
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
      65,    66,    67,    68,    69,    70,    71,    72,    77
};

#if YYDEBUG
/* YYRLINE[YYN] -- Source line where rule number YYN was defined.  */
static const yytype_int16 yyrline[] =
{
       0,   239,   239,   247,   248,   249,   250,   251,   252,   253,
     254,   255,   256,   257,   258,   259,   260,   261,   262,   263,
     264,   265,   266,   270,   276,   281,   287,   293,   299,   305,
     312,   318,   326,   344,   364,   374,   393,   396,   409,   417,
     426,   435,   444,   453,   461,   470,   479,   488,   499,   502,
     503,   504,   505,   506,   509,   525,   528,   540,   543,   555,
     559,   563,   567,   571,   584,   592,   604,   624,   627,   639,
     648,   667,   668,   669,   673,   703,   752,   802,   812,   817,
     828,   831,   834,   837,   840,   844,   847,   859,   872,   877,
     882,   888,   896,   901,   906,   911,   917,   922,   930,   942,
     949,   967,   977,   983,   992,   997,  1004,  1008,  1012,  1016,
    1021,  1025,  1050,  1053,  1061,  1079,  1082,  1088,  1091,  1097,
    1103,  1116,  1119,  1126,  1129,  1134,  1141,  1149,  1157,  1172,
    1187,  1202,  1217,  1231,  1245,  1254,  1263,  1270,  1281,  1282,
    1283,  1284,  1285,  1286,  1290,  1303,  1311,  1323,  1326,  1333,
    1339,  1352,  1364,  1367,  1371,  1377,  1378
};
#endif

/** Accessing symbol of state STATE.  */
#define YY_ACCESSING_SYMBOL(State) YY_CAST (yysymbol_kind_t, yystos[State])

#if 1
/* The user-facing name of the symbol whose (internal) number is
   YYSYMBOL.  No bounds checking.  */
static const char *yysymbol_name (yysymbol_kind_t yysymbol) YY_ATTRIBUTE_UNUSED;

/* YYTNAME[SYMBOL-NUM] -- String name of the symbol SYMBOL-NUM.
   First, the terminals, then, starting at YYNTOKENS, nonterminals.  */
static const char *const yytname[] =
{
  "\"end of file\"", "error", "\"invalid token\"", "SEMICOLON", "CREATE",
  "DROP", "TABLE", "TABLES", "INDEX", "CALC", "SELECT", "DESC", "SHOW",
  "SYNC", "INSERT", "DELETE", "UPDATE", "LBRACE", "RBRACE", "COMMA",
  "DATE_T", "TRX_BEGIN", "TRX_COMMIT", "TRX_ROLLBACK", "INT_T", "STRING_T",
  "FLOAT_T", "HELP", "EXIT", "DOT", "INTO", "VALUES", "MAX", "MIN",
  "COUNT", "AVG", "SUM", "FROM", "WHERE", "AND", "LIKE", "NOT", "SET",
  "INNER", "JOIN", "ON", "LOAD", "DATA", "INFILE", "EXPLAIN", "ORDER",
  "BY", "ASC", "AS", "GROUP", "HAVING", "EQ", "LT", "GT", "LE", "GE", "NE",
  "IN", "EXISTS", "UNIQUE", "NULL_T", "NULLABLE_T", "TEXT_T", "IS",
  "NUMBER", "FLOAT", "ID", "SSS", "'+'", "'-'", "'*'", "'/'", "UMINUS",
  "$accept", "commands", "command_wrapper", "exit_stmt", "help_stmt",
  "sync_stmt", "begin_stmt", "commit_stmt", "rollback_stmt",
  "drop_table_stmt", "show_tables_stmt", "desc_table_stmt",
  "create_index_stmt", "drop_index_stmt", "create_table_stmt",
  "attr_def_list", "attr_def", "number", "type", "insert_stmt",
  "value_list", "id_list", "value", "delete_stmt", "update_stmt",
  "update_assign_list", "update_assign", "alias_opt", "select_stmt",
  "calc_stmt", "expression_list", "expression", "agg_expr", "select_attr",
  "select_expr_item", "select_expr_list", "rel_attr", "join_clause",
  "where", "group_by_clause", "group_by_list", "having_clause",
  "condition_list", "condition", "comp_op", "load_data_stmt",
  "explain_stmt", "set_variable_stmt", "order_by_clause", "order_by_list",
  "order_by_item", "opt_asc_desc", "opt_semicolon", YY_NULLPTR
};

static const char *
yysymbol_name (yysymbol_kind_t yysymbol)
{
  return yytname[yysymbol];
}
#endif

#define YYPACT_NINF (-258)

#define yypact_value_is_default(Yyn) \
  ((Yyn) == YYPACT_NINF)

#define YYTABLE_NINF (-92)

#define yytable_value_is_error(Yyn) \
  0

/* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
   STATE-NUM.  */
static const yytype_int16 yypact[] =
{
     316,     7,   128,   167,   144,   -45,    26,  -258,    12,    17,
     -26,  -258,  -258,  -258,  -258,  -258,   -10,    40,   316,    64,
      93,  -258,  -258,  -258,  -258,  -258,  -258,  -258,  -258,  -258,
    -258,  -258,  -258,  -258,  -258,  -258,  -258,  -258,  -258,  -258,
    -258,    38,    45,   114,    54,    59,     4,   116,   121,   131,
     136,   146,  -258,  -258,  -258,    78,  -258,   237,  -258,  -258,
      -8,  -258,   100,  -258,   -18,    98,   138,  -258,    25,  -258,
    -258,   110,   112,   143,   148,   162,  -258,  -258,  -258,  -258,
     195,   172,   169,  -258,   197,   235,    16,    66,    66,   211,
      66,    66,   167,   187,  -258,  -258,  -258,   167,   167,   167,
     167,   167,    48,   189,  -258,  -258,  -258,   190,   245,   196,
     194,   220,   244,   238,   212,   117,   213,   217,   218,   239,
     219,  -258,  -258,   275,   276,   277,   278,   279,   268,   280,
     281,   282,   283,   285,   286,   287,  -258,  -258,    71,    71,
    -258,  -258,  -258,  -258,   -30,  -258,   -30,   249,   296,   293,
      99,  -258,   297,   267,   269,   305,  -258,   284,   322,   126,
    -258,   312,     5,   326,   318,   288,  -258,    66,    66,   216,
      66,    66,   289,  -258,  -258,  -258,  -258,  -258,  -258,  -258,
       9,     9,   329,   337,   117,   -13,  -258,  -258,  -258,  -258,
    -258,  -258,   331,   -17,   167,   167,   194,    66,   194,   306,
     117,    80,   212,   238,  -258,  -258,   351,  -258,  -258,  -258,
    -258,  -258,    62,   217,   345,   299,   347,   353,   354,   355,
     356,   357,   358,  -258,   307,   333,   238,   238,   337,   361,
    -258,   117,   363,    70,   317,  -258,    92,    92,  -258,   362,
    -258,  -258,   332,  -258,   365,   337,  -258,   322,  -258,   314,
     319,   132,  -258,  -258,   326,  -258,   367,   320,  -258,  -258,
    -258,  -258,  -258,  -258,   -30,   321,   220,   220,   369,  -258,
    -258,    70,   365,   371,  -258,    66,    66,   117,   372,   375,
    -258,  -258,  -258,   376,  -258,  -258,  -258,   324,   378,   367,
       9,   -30,   269,   269,  -258,   365,   379,   380,  -258,  -258,
       8,  -258,   381,   365,  -258,  -258,   -19,   367,  -258,   383,
    -258,   359,   306,   306,   384,  -258,  -258,  -258,  -258,  -258,
      66,  -258,   155,  -258,  -258,  -258,  -258,   194,  -258,  -258,
    -258,  -258,  -258,  -258,     9,  -258
};

/* YYDEFACT[STATE-NUM] -- Default reduction number in state STATE-NUM.
   Performed when YYTABLE does not specify something else to do.  Zero
   means the default is an error.  */
static const yytype_uint8 yydefact[] =
{
       0,     0,     0,     0,     0,     0,     0,    25,     0,     0,
       0,    26,    27,    28,    24,    23,     0,     0,     0,     0,
     155,    22,    21,    14,    15,    16,    17,     9,    10,    11,
      12,    13,     8,     5,     7,     6,     4,     3,    18,    19,
      20,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    64,    59,    61,    89,    63,     0,    88,    77,
      78,    91,    89,    98,    71,    71,     0,   102,   115,    31,
      30,     0,     0,     0,     0,     0,   145,     1,   156,     2,
       0,     0,     0,    29,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    59,    61,    85,     0,     0,     0,
       0,     0,     0,     0,    72,    99,   100,     0,     0,     0,
     123,   117,     0,   115,     0,     0,     0,     0,     0,     0,
       0,    86,    84,     0,     0,     0,     0,     0,   104,     0,
       0,     0,     0,     0,     0,     0,    90,    79,    80,    81,
      82,    83,   101,    73,    71,   103,    71,     0,     0,     0,
      91,   116,   124,     0,   121,     0,    65,     0,    67,     0,
     146,     0,     0,    36,     0,     0,    34,     0,     0,     0,
       0,     0,     0,    92,    93,    95,    94,    96,    97,    87,
     112,   112,     0,     0,     0,     0,   138,   139,   140,   141,
     142,   143,     0,     0,     0,     0,   123,     0,   123,   147,
       0,     0,     0,   115,    60,    62,     0,    52,    49,    50,
      51,    53,    43,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   105,     0,     0,   115,   115,     0,     0,
     126,     0,     0,     0,     0,   134,   136,   137,   125,   119,
     118,   122,     0,    74,    55,     0,    69,    67,    66,     0,
       0,     0,    44,    46,    36,    35,    57,     0,   106,   107,
     109,   108,   110,   111,    71,     0,   117,   117,     0,   132,
     127,     0,    55,     0,   135,     0,     0,     0,     0,     0,
      68,   144,    48,     0,    45,    47,    37,     0,     0,    57,
     112,    71,   121,   121,   133,    55,     0,     0,   130,   120,
     152,   148,   149,    55,    54,    70,    38,    57,    33,     0,
     113,     0,   147,   147,     0,   131,   128,   154,   153,   151,
       0,    56,     0,    39,    41,    58,    32,   123,    76,    75,
     129,   150,    40,    42,   112,   114
};

/* YYPGOTO[NTERM-NUM].  */
static const yytype_int16 yypgoto[] =
{
    -258,  -258,   385,  -258,  -258,  -258,  -258,  -258,  -258,  -258,
    -258,  -258,  -258,  -258,  -258,   145,   192,  -258,  -258,  -258,
    -252,  -257,  -107,  -258,  -258,   159,   205,   -58,   -40,  -258,
      23,    13,    -4,  -258,   300,  -258,   -86,  -172,   -95,   -44,
     134,   -68,  -186,  -258,   260,  -258,  -258,  -258,   -57,    91,
    -258,  -258,  -258
};

/* YYDEFGOTO[NTERM-NUM].  */
static const yytype_int16 yydefgoto[] =
{
       0,    19,    20,    21,    22,    23,    24,    25,    26,    27,
      28,    29,    30,    31,    32,   214,   163,   283,   212,    33,
     278,   288,    58,    34,    35,   203,   158,   105,    36,    37,
      59,   149,    61,    66,    67,    68,   239,   226,   111,   154,
     240,   199,   151,   152,   194,    38,    39,    40,   243,   301,
     302,   319,    79
};

/* YYTABLE[YYPACT[STATE-NUM]] -- What to do in state STATE-NUM.  If
   positive, shift that token.  If negative, reduce the rule whose
   number is the opposite.  If YYTABLE_NINF, syntax error.  */
static const yytype_int16 yytable[] =
{
      65,   129,   130,   132,   133,   134,    85,   106,   160,   227,
     238,    97,   241,    41,     4,    42,    60,    64,   156,   317,
     297,    46,   322,   103,   234,   207,    69,   231,   224,   208,
     209,   210,   309,    70,   122,   103,    47,    48,    49,    50,
      51,   104,    71,   314,   108,    73,   323,   324,   235,   232,
     325,   321,   225,   104,    72,    98,    99,   100,   101,    86,
     318,    74,   109,   110,    77,    98,    99,   100,   101,    52,
      96,    43,   211,    53,    54,    55,    56,   230,    57,   250,
       4,   217,   218,   220,   221,   222,   180,    75,   181,    98,
      99,   100,   101,   244,   246,    92,    78,   245,   123,   124,
     125,   126,   127,   251,    65,    60,   150,    93,   248,    80,
      60,   138,   139,   140,   141,   135,    81,    92,   310,   136,
     137,    64,    82,   142,   270,    83,   272,   252,   253,   102,
      84,   266,   267,    87,    44,    52,    45,   128,    88,    53,
      54,   334,    56,   229,   159,    52,   100,   101,    89,    53,
      54,   103,    56,    90,   159,   186,   187,   188,   189,   190,
     191,    46,   335,    91,   295,    98,    99,   100,   101,   104,
     303,   -91,   -91,   -91,   -91,   107,    47,    48,    49,    50,
      51,   112,    52,   113,    46,   114,    53,    54,   268,    56,
     300,   159,   150,   273,   150,   204,   205,   284,   285,    47,
      48,    49,    50,    51,   115,   279,   290,   236,   237,    52,
     116,    46,   117,    53,    54,    62,    56,   118,    57,    63,
     332,   333,   292,   293,   312,   313,    47,    48,    49,    50,
      51,   296,    52,   311,   300,   147,    53,    54,    55,    56,
     119,    57,   120,   123,   124,   125,   126,   127,   123,   124,
     125,   126,   127,   121,    46,   328,   329,   148,   136,    52,
     143,   144,    46,    53,    54,    55,    56,   146,    57,    47,
      48,    49,    50,    51,   153,   155,   110,    47,    48,    49,
      50,    51,   128,   157,   165,   161,   131,   128,   162,   164,
     166,   219,   167,   168,   169,   170,   171,   172,   173,   174,
     175,   176,    52,   177,   178,   179,    94,    95,    55,    56,
      52,    57,   182,   183,    53,    54,    62,    56,   197,    57,
       1,     2,   200,   150,   198,     3,     4,     5,     6,     7,
       8,     9,    10,   184,   185,   215,   196,    11,    12,    13,
     201,   202,   206,    14,    15,   213,   228,     4,   233,   186,
     187,   188,   189,   190,   191,   192,   242,   249,    16,   216,
     223,   193,    17,   255,   257,    18,    98,    99,   100,   101,
     256,   258,   259,   260,   261,   262,   263,   265,   264,   269,
     271,   275,   274,   276,   277,   281,   287,   294,   282,   298,
     304,   289,   291,   305,   306,   307,   308,   315,   316,   286,
     320,   326,   330,    76,   327,   254,   280,   247,   145,   299,
     195,   331
};

static const yytype_int16 yycheck[] =
{
       4,    87,    88,    89,    90,    91,    46,    65,   115,   181,
     196,    19,   198,     6,    10,     8,     3,     4,   113,    11,
     272,    17,    41,    53,    41,    20,    71,    40,    19,    24,
      25,    26,   289,     7,    18,    53,    32,    33,    34,    35,
      36,    71,    30,   295,    19,    71,    65,    66,    65,    62,
     307,   303,    43,    71,    37,    73,    74,    75,    76,    46,
      52,    71,    37,    38,     0,    73,    74,    75,    76,    65,
      57,    64,    67,    69,    70,    71,    72,   184,    74,    17,
      10,   167,   168,   169,   170,   171,   144,    47,   146,    73,
      74,    75,    76,   200,   201,    17,     3,    17,    32,    33,
      34,    35,    36,    41,   108,    92,   110,    29,   203,    71,
      97,    98,    99,   100,   101,    92,    71,    17,   290,    71,
      97,   108,     8,    75,   231,    71,   233,    65,    66,    29,
      71,   226,   227,    17,     6,    65,     8,    71,    17,    69,
      70,   327,    72,   183,    74,    65,    75,    76,    17,    69,
      70,    53,    72,    17,    74,    56,    57,    58,    59,    60,
      61,    17,   334,    17,   271,    73,    74,    75,    76,    71,
     277,    73,    74,    75,    76,    37,    32,    33,    34,    35,
      36,    71,    65,    71,    17,    42,    69,    70,   228,    72,
     276,    74,   196,   233,   198,    69,    70,    65,    66,    32,
      33,    34,    35,    36,    56,   245,   264,   194,   195,    65,
      48,    17,    17,    69,    70,    71,    72,    45,    74,    75,
      65,    66,   266,   267,   292,   293,    32,    33,    34,    35,
      36,   271,    65,   291,   320,    41,    69,    70,    71,    72,
      71,    74,    45,    32,    33,    34,    35,    36,    32,    33,
      34,    35,    36,    18,    17,   312,   313,    63,    71,    65,
      71,    71,    17,    69,    70,    71,    72,    71,    74,    32,
      33,    34,    35,    36,    54,    31,    38,    32,    33,    34,
      35,    36,    71,    71,    45,    72,    75,    71,    71,    71,
      71,    75,    17,    17,    17,    17,    17,    29,    18,    18,
      18,    18,    65,    18,    18,    18,    69,    70,    71,    72,
      65,    74,    63,    17,    69,    70,    71,    72,    51,    74,
       4,     5,    17,   327,    55,     9,    10,    11,    12,    13,
      14,    15,    16,    40,    41,    17,    39,    21,    22,    23,
      56,    19,    30,    27,    28,    19,    17,    10,    17,    56,
      57,    58,    59,    60,    61,    62,    50,     6,    42,    71,
      71,    68,    46,    18,    17,    49,    73,    74,    75,    76,
      71,    18,    18,    18,    18,    18,    18,    44,    71,    18,
      17,    19,    65,    51,    19,    71,    19,    18,    69,    18,
      18,    71,    71,    18,    18,    71,    18,    18,    18,   254,
      19,    18,    18,    18,    45,   213,   247,   202,   108,   275,
     150,   320
};

/* YYSTOS[STATE-NUM] -- The symbol kind of the accessing symbol of
   state STATE-NUM.  */
static const yytype_uint8 yystos[] =
{
       0,     4,     5,     9,    10,    11,    12,    13,    14,    15,
      16,    21,    22,    23,    27,    28,    42,    46,    49,    79,
      80,    81,    82,    83,    84,    85,    86,    87,    88,    89,
      90,    91,    92,    97,   101,   102,   106,   107,   123,   124,
     125,     6,     8,    64,     6,     8,    17,    32,    33,    34,
      35,    36,    65,    69,    70,    71,    72,    74,   100,   108,
     109,   110,    71,    75,   109,   110,   111,   112,   113,    71,
       7,    30,    37,    71,    71,    47,    80,     0,     3,   130,
      71,    71,     8,    71,    71,   106,   109,    17,    17,    17,
      17,    17,    17,    29,    69,    70,   109,    19,    73,    74,
      75,    76,    29,    53,    71,   105,   105,    37,    19,    37,
      38,   116,    71,    71,    42,    56,    48,    17,    45,    71,
      45,    18,    18,    32,    33,    34,    35,    36,    71,   114,
     114,    75,   114,   114,   114,   108,    71,   108,   109,   109,
     109,   109,    75,    71,    71,   112,    71,    41,    63,   109,
     110,   120,   121,    54,   117,    31,   116,    71,   104,    74,
     100,    72,    71,    94,    71,    45,    71,    17,    17,    17,
      17,    17,    29,    18,    18,    18,    18,    18,    18,    18,
     105,   105,    63,    17,    40,    41,    56,    57,    58,    59,
      60,    61,    62,    68,   122,   122,    39,    51,    55,   119,
      17,    56,    19,   103,    69,    70,    30,    20,    24,    25,
      26,    67,    96,    19,    93,    17,    71,   114,   114,    75,
     114,   114,   114,    71,    19,    43,   115,   115,    17,   106,
     100,    40,    62,    17,    41,    65,   109,   109,   120,   114,
     118,   120,    50,   126,   100,    17,   100,   104,   116,     6,
      17,    41,    65,    66,    94,    18,    71,    17,    18,    18,
      18,    18,    18,    18,    71,    44,   116,   116,   106,    18,
     100,    17,   100,   106,    65,    19,    51,    19,    98,   106,
     103,    71,    69,    95,    65,    66,    93,    19,    99,    71,
     105,    71,   117,   117,    18,   100,   106,    98,    18,   118,
     114,   127,   128,   100,    18,    18,    18,    71,    18,    99,
     115,   105,   119,   119,    98,    18,    18,    11,    52,   129,
      19,    98,    41,    65,    66,    99,    18,    45,   126,   126,
      18,   127,    65,    66,   120,   115
};

/* YYR1[RULE-NUM] -- Symbol kind of the left-hand side of rule RULE-NUM.  */
static const yytype_uint8 yyr1[] =
{
       0,    78,    79,    80,    80,    80,    80,    80,    80,    80,
      80,    80,    80,    80,    80,    80,    80,    80,    80,    80,
      80,    80,    80,    81,    82,    83,    84,    85,    86,    87,
      88,    89,    90,    90,    91,    92,    93,    93,    94,    94,
      94,    94,    94,    94,    94,    94,    94,    94,    95,    96,
      96,    96,    96,    96,    97,    98,    98,    99,    99,   100,
     100,   100,   100,   100,   100,   101,   102,   103,   103,   104,
     104,   105,   105,   105,   106,   106,   106,   107,   108,   108,
     109,   109,   109,   109,   109,   109,   109,   109,   109,   109,
     109,   109,   110,   110,   110,   110,   110,   110,   111,   112,
     112,   112,   113,   113,   114,   114,   114,   114,   114,   114,
     114,   114,   115,   115,   115,   116,   116,   117,   117,   118,
     118,   119,   119,   120,   120,   120,   121,   121,   121,   121,
     121,   121,   121,   121,   121,   121,   121,   121,   122,   122,
     122,   122,   122,   122,   123,   124,   125,   126,   126,   127,
     127,   128,   129,   129,   129,   130,   130
};

/* YYR2[RULE-NUM] -- Number of symbols on the right-hand side of rule RULE-NUM.  */
static const yytype_int8 yyr2[] =
{
       0,     2,     2,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     3,
       2,     2,    10,     9,     5,     7,     0,     3,     5,     6,
       7,     6,     7,     2,     3,     4,     3,     4,     1,     1,
       1,     1,     1,     1,     8,     0,     3,     0,     3,     1,
       2,     1,     2,     1,     1,     4,     6,     0,     3,     3,
       5,     0,     1,     2,     6,    10,    10,     2,     1,     3,
       3,     3,     3,     3,     3,     2,     3,     4,     1,     1,
       3,     1,     4,     4,     4,     4,     4,     4,     1,     2,
       2,     3,     1,     3,     1,     3,     4,     4,     4,     4,
       4,     4,     0,     4,     7,     0,     2,     0,     3,     1,
       3,     0,     2,     0,     1,     3,     3,     4,     6,     7,
       5,     6,     4,     5,     3,     4,     3,     3,     1,     1,
       1,     1,     1,     1,     7,     2,     4,     0,     3,     1,
       3,     2,     0,     1,     1,     0,     1
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
        yyerror (&yylloc, sql_string, sql_result, scanner, YY_("syntax error: cannot back up")); \
        YYERROR;                                                  \
      }                                                           \
  while (0)

/* Backward compatibility with an undocumented macro.
   Use YYerror or YYUNDEF. */
#define YYERRCODE YYUNDEF

/* YYLLOC_DEFAULT -- Set CURRENT to span from RHS[1] to RHS[N].
   If N is 0, then set CURRENT to the empty location which ends
   the previous symbol: RHS[0] (always defined).  */

#ifndef YYLLOC_DEFAULT
# define YYLLOC_DEFAULT(Current, Rhs, N)                                \
    do                                                                  \
      if (N)                                                            \
        {                                                               \
          (Current).first_line   = YYRHSLOC (Rhs, 1).first_line;        \
          (Current).first_column = YYRHSLOC (Rhs, 1).first_column;      \
          (Current).last_line    = YYRHSLOC (Rhs, N).last_line;         \
          (Current).last_column  = YYRHSLOC (Rhs, N).last_column;       \
        }                                                               \
      else                                                              \
        {                                                               \
          (Current).first_line   = (Current).last_line   =              \
            YYRHSLOC (Rhs, 0).last_line;                                \
          (Current).first_column = (Current).last_column =              \
            YYRHSLOC (Rhs, 0).last_column;                              \
        }                                                               \
    while (0)
#endif

#define YYRHSLOC(Rhs, K) ((Rhs)[K])


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


/* YYLOCATION_PRINT -- Print the location on the stream.
   This macro was not mandated originally: define only if we know
   we won't break user code: when these are the locations we know.  */

# ifndef YYLOCATION_PRINT

#  if defined YY_LOCATION_PRINT

   /* Temporary convenience wrapper in case some people defined the
      undocumented and private YY_LOCATION_PRINT macros.  */
#   define YYLOCATION_PRINT(File, Loc)  YY_LOCATION_PRINT(File, *(Loc))

#  elif defined YYLTYPE_IS_TRIVIAL && YYLTYPE_IS_TRIVIAL

/* Print *YYLOCP on YYO.  Private, do not rely on its existence. */

YY_ATTRIBUTE_UNUSED
static int
yy_location_print_ (FILE *yyo, YYLTYPE const * const yylocp)
{
  int res = 0;
  int end_col = 0 != yylocp->last_column ? yylocp->last_column - 1 : 0;
  if (0 <= yylocp->first_line)
    {
      res += YYFPRINTF (yyo, "%d", yylocp->first_line);
      if (0 <= yylocp->first_column)
        res += YYFPRINTF (yyo, ".%d", yylocp->first_column);
    }
  if (0 <= yylocp->last_line)
    {
      if (yylocp->first_line < yylocp->last_line)
        {
          res += YYFPRINTF (yyo, "-%d", yylocp->last_line);
          if (0 <= end_col)
            res += YYFPRINTF (yyo, ".%d", end_col);
        }
      else if (0 <= end_col && yylocp->first_column < end_col)
        res += YYFPRINTF (yyo, "-%d", end_col);
    }
  return res;
}

#   define YYLOCATION_PRINT  yy_location_print_

    /* Temporary convenience wrapper in case some people defined the
       undocumented and private YY_LOCATION_PRINT macros.  */
#   define YY_LOCATION_PRINT(File, Loc)  YYLOCATION_PRINT(File, &(Loc))

#  else

#   define YYLOCATION_PRINT(File, Loc) ((void) 0)
    /* Temporary convenience wrapper in case some people defined the
       undocumented and private YY_LOCATION_PRINT macros.  */
#   define YY_LOCATION_PRINT  YYLOCATION_PRINT

#  endif
# endif /* !defined YYLOCATION_PRINT */


# define YY_SYMBOL_PRINT(Title, Kind, Value, Location)                    \
do {                                                                      \
  if (yydebug)                                                            \
    {                                                                     \
      YYFPRINTF (stderr, "%s ", Title);                                   \
      yy_symbol_print (stderr,                                            \
                  Kind, Value, Location, sql_string, sql_result, scanner); \
      YYFPRINTF (stderr, "\n");                                           \
    }                                                                     \
} while (0)


/*-----------------------------------.
| Print this symbol's value on YYO.  |
`-----------------------------------*/

static void
yy_symbol_value_print (FILE *yyo,
                       yysymbol_kind_t yykind, YYSTYPE const * const yyvaluep, YYLTYPE const * const yylocationp, const char * sql_string, ParsedSqlResult * sql_result, void * scanner)
{
  FILE *yyoutput = yyo;
  YY_USE (yyoutput);
  YY_USE (yylocationp);
  YY_USE (sql_string);
  YY_USE (sql_result);
  YY_USE (scanner);
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
                 yysymbol_kind_t yykind, YYSTYPE const * const yyvaluep, YYLTYPE const * const yylocationp, const char * sql_string, ParsedSqlResult * sql_result, void * scanner)
{
  YYFPRINTF (yyo, "%s %s (",
             yykind < YYNTOKENS ? "token" : "nterm", yysymbol_name (yykind));

  YYLOCATION_PRINT (yyo, yylocationp);
  YYFPRINTF (yyo, ": ");
  yy_symbol_value_print (yyo, yykind, yyvaluep, yylocationp, sql_string, sql_result, scanner);
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
yy_reduce_print (yy_state_t *yyssp, YYSTYPE *yyvsp, YYLTYPE *yylsp,
                 int yyrule, const char * sql_string, ParsedSqlResult * sql_result, void * scanner)
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
                       &yyvsp[(yyi + 1) - (yynrhs)],
                       &(yylsp[(yyi + 1) - (yynrhs)]), sql_string, sql_result, scanner);
      YYFPRINTF (stderr, "\n");
    }
}

# define YY_REDUCE_PRINT(Rule)          \
do {                                    \
  if (yydebug)                          \
    yy_reduce_print (yyssp, yyvsp, yylsp, Rule, sql_string, sql_result, scanner); \
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


/* Context of a parse error.  */
typedef struct
{
  yy_state_t *yyssp;
  yysymbol_kind_t yytoken;
  YYLTYPE *yylloc;
} yypcontext_t;

/* Put in YYARG at most YYARGN of the expected tokens given the
   current YYCTX, and return the number of tokens stored in YYARG.  If
   YYARG is null, return the number of expected tokens (guaranteed to
   be less than YYNTOKENS).  Return YYENOMEM on memory exhaustion.
   Return 0 if there are more than YYARGN expected tokens, yet fill
   YYARG up to YYARGN. */
static int
yypcontext_expected_tokens (const yypcontext_t *yyctx,
                            yysymbol_kind_t yyarg[], int yyargn)
{
  /* Actual size of YYARG. */
  int yycount = 0;
  int yyn = yypact[+*yyctx->yyssp];
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
        if (yycheck[yyx + yyn] == yyx && yyx != YYSYMBOL_YYerror
            && !yytable_value_is_error (yytable[yyx + yyn]))
          {
            if (!yyarg)
              ++yycount;
            else if (yycount == yyargn)
              return 0;
            else
              yyarg[yycount++] = YY_CAST (yysymbol_kind_t, yyx);
          }
    }
  if (yyarg && yycount == 0 && 0 < yyargn)
    yyarg[0] = YYSYMBOL_YYEMPTY;
  return yycount;
}




#ifndef yystrlen
# if defined __GLIBC__ && defined _STRING_H
#  define yystrlen(S) (YY_CAST (YYPTRDIFF_T, strlen (S)))
# else
/* Return the length of YYSTR.  */
static YYPTRDIFF_T
yystrlen (const char *yystr)
{
  YYPTRDIFF_T yylen;
  for (yylen = 0; yystr[yylen]; yylen++)
    continue;
  return yylen;
}
# endif
#endif

#ifndef yystpcpy
# if defined __GLIBC__ && defined _STRING_H && defined _GNU_SOURCE
#  define yystpcpy stpcpy
# else
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
# endif
#endif

#ifndef yytnamerr
/* Copy to YYRES the contents of YYSTR after stripping away unnecessary
   quotes and backslashes, so that it's suitable for yyerror.  The
   heuristic is that double-quoting is unnecessary unless the string
   contains an apostrophe, a comma, or backslash (other than
   backslash-backslash).  YYSTR is taken from yytname.  If YYRES is
   null, do not copy; instead, return the length of what the result
   would have been.  */
static YYPTRDIFF_T
yytnamerr (char *yyres, const char *yystr)
{
  if (*yystr == '"')
    {
      YYPTRDIFF_T yyn = 0;
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
            else
              goto append;

          append:
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

  if (yyres)
    return yystpcpy (yyres, yystr) - yyres;
  else
    return yystrlen (yystr);
}
#endif


static int
yy_syntax_error_arguments (const yypcontext_t *yyctx,
                           yysymbol_kind_t yyarg[], int yyargn)
{
  /* Actual size of YYARG. */
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
  if (yyctx->yytoken != YYSYMBOL_YYEMPTY)
    {
      int yyn;
      if (yyarg)
        yyarg[yycount] = yyctx->yytoken;
      ++yycount;
      yyn = yypcontext_expected_tokens (yyctx,
                                        yyarg ? yyarg + 1 : yyarg, yyargn - 1);
      if (yyn == YYENOMEM)
        return YYENOMEM;
      else
        yycount += yyn;
    }
  return yycount;
}

/* Copy into *YYMSG, which is of size *YYMSG_ALLOC, an error message
   about the unexpected token YYTOKEN for the state stack whose top is
   YYSSP.

   Return 0 if *YYMSG was successfully written.  Return -1 if *YYMSG is
   not large enough to hold the message.  In that case, also set
   *YYMSG_ALLOC to the required number of bytes.  Return YYENOMEM if the
   required number of bytes is too large to store.  */
static int
yysyntax_error (YYPTRDIFF_T *yymsg_alloc, char **yymsg,
                const yypcontext_t *yyctx)
{
  enum { YYARGS_MAX = 5 };
  /* Internationalized format string. */
  const char *yyformat = YY_NULLPTR;
  /* Arguments of yyformat: reported tokens (one for the "unexpected",
     one per "expected"). */
  yysymbol_kind_t yyarg[YYARGS_MAX];
  /* Cumulated lengths of YYARG.  */
  YYPTRDIFF_T yysize = 0;

  /* Actual size of YYARG. */
  int yycount = yy_syntax_error_arguments (yyctx, yyarg, YYARGS_MAX);
  if (yycount == YYENOMEM)
    return YYENOMEM;

  switch (yycount)
    {
#define YYCASE_(N, S)                       \
      case N:                               \
        yyformat = S;                       \
        break
    default: /* Avoid compiler warnings. */
      YYCASE_(0, YY_("syntax error"));
      YYCASE_(1, YY_("syntax error, unexpected %s"));
      YYCASE_(2, YY_("syntax error, unexpected %s, expecting %s"));
      YYCASE_(3, YY_("syntax error, unexpected %s, expecting %s or %s"));
      YYCASE_(4, YY_("syntax error, unexpected %s, expecting %s or %s or %s"));
      YYCASE_(5, YY_("syntax error, unexpected %s, expecting %s or %s or %s or %s"));
#undef YYCASE_
    }

  /* Compute error message size.  Don't count the "%s"s, but reserve
     room for the terminator.  */
  yysize = yystrlen (yyformat) - 2 * yycount + 1;
  {
    int yyi;
    for (yyi = 0; yyi < yycount; ++yyi)
      {
        YYPTRDIFF_T yysize1
          = yysize + yytnamerr (YY_NULLPTR, yytname[yyarg[yyi]]);
        if (yysize <= yysize1 && yysize1 <= YYSTACK_ALLOC_MAXIMUM)
          yysize = yysize1;
        else
          return YYENOMEM;
      }
  }

  if (*yymsg_alloc < yysize)
    {
      *yymsg_alloc = 2 * yysize;
      if (! (yysize <= *yymsg_alloc
             && *yymsg_alloc <= YYSTACK_ALLOC_MAXIMUM))
        *yymsg_alloc = YYSTACK_ALLOC_MAXIMUM;
      return -1;
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
          yyp += yytnamerr (yyp, yytname[yyarg[yyi++]]);
          yyformat += 2;
        }
      else
        {
          ++yyp;
          ++yyformat;
        }
  }
  return 0;
}


/*-----------------------------------------------.
| Release the memory associated to this symbol.  |
`-----------------------------------------------*/

static void
yydestruct (const char *yymsg,
            yysymbol_kind_t yykind, YYSTYPE *yyvaluep, YYLTYPE *yylocationp, const char * sql_string, ParsedSqlResult * sql_result, void * scanner)
{
  YY_USE (yyvaluep);
  YY_USE (yylocationp);
  YY_USE (sql_string);
  YY_USE (sql_result);
  YY_USE (scanner);
  if (!yymsg)
    yymsg = "Deleting";
  YY_SYMBOL_PRINT (yymsg, yykind, yyvaluep, yylocationp);

  YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
  YY_USE (yykind);
  YY_IGNORE_MAYBE_UNINITIALIZED_END
}






/*----------.
| yyparse.  |
`----------*/

int
yyparse (const char * sql_string, ParsedSqlResult * sql_result, void * scanner)
{
/* Lookahead token kind.  */
int yychar;


/* The semantic value of the lookahead symbol.  */
/* Default value used for initialization, for pacifying older GCCs
   or non-GCC compilers.  */
YY_INITIAL_VALUE (static YYSTYPE yyval_default;)
YYSTYPE yylval YY_INITIAL_VALUE (= yyval_default);

/* Location data for the lookahead symbol.  */
static YYLTYPE yyloc_default
# if defined YYLTYPE_IS_TRIVIAL && YYLTYPE_IS_TRIVIAL
  = { 1, 1, 1, 1 }
# endif
;
YYLTYPE yylloc = yyloc_default;

    /* Number of syntax errors so far.  */
    int yynerrs = 0;

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

    /* The location stack: array, bottom, top.  */
    YYLTYPE yylsa[YYINITDEPTH];
    YYLTYPE *yyls = yylsa;
    YYLTYPE *yylsp = yyls;

  int yyn;
  /* The return value of yyparse.  */
  int yyresult;
  /* Lookahead symbol kind.  */
  yysymbol_kind_t yytoken = YYSYMBOL_YYEMPTY;
  /* The variables used to return semantic value and location from the
     action routines.  */
  YYSTYPE yyval;
  YYLTYPE yyloc;

  /* The locations where the error started and ended.  */
  YYLTYPE yyerror_range[3];

  /* Buffer for error messages, and its allocated size.  */
  char yymsgbuf[128];
  char *yymsg = yymsgbuf;
  YYPTRDIFF_T yymsg_alloc = sizeof yymsgbuf;

#define YYPOPSTACK(N)   (yyvsp -= (N), yyssp -= (N), yylsp -= (N))

  /* The number of symbols on the RHS of the reduced rule.
     Keep to zero when no symbol should be popped.  */
  int yylen = 0;

  YYDPRINTF ((stderr, "Starting parse\n"));

  yychar = YYEMPTY; /* Cause a token to be read.  */

  yylsp[0] = yylloc;
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
        YYLTYPE *yyls1 = yyls;

        /* Each stack pointer address is followed by the size of the
           data in use in that stack, in bytes.  This used to be a
           conditional around just the two extra args, but that might
           be undefined if yyoverflow is a macro.  */
        yyoverflow (YY_("memory exhausted"),
                    &yyss1, yysize * YYSIZEOF (*yyssp),
                    &yyvs1, yysize * YYSIZEOF (*yyvsp),
                    &yyls1, yysize * YYSIZEOF (*yylsp),
                    &yystacksize);
        yyss = yyss1;
        yyvs = yyvs1;
        yyls = yyls1;
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
        YYSTACK_RELOCATE (yyls_alloc, yyls);
#  undef YYSTACK_RELOCATE
        if (yyss1 != yyssa)
          YYSTACK_FREE (yyss1);
      }
# endif

      yyssp = yyss + yysize - 1;
      yyvsp = yyvs + yysize - 1;
      yylsp = yyls + yysize - 1;

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
      yychar = yylex (&yylval, &yylloc, scanner);
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
      yyerror_range[1] = yylloc;
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
  *++yylsp = yylloc;

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

  /* Default location. */
  YYLLOC_DEFAULT (yyloc, (yylsp - yylen), yylen);
  yyerror_range[1] = yyloc;
  YY_REDUCE_PRINT (yyn);
  switch (yyn)
    {
  case 2: /* commands: command_wrapper opt_semicolon  */
#line 240 "yacc_sql.y"
  {
    std::unique_ptr<ParsedSqlNode> sql_node = std::unique_ptr<ParsedSqlNode>((yyvsp[-1].sql_node));
    sql_result->add_sql_node(std::move(sql_node));
  }
#line 1906 "yacc_sql.cpp"
    break;

  case 23: /* exit_stmt: EXIT  */
#line 270 "yacc_sql.y"
         {
      (void)yynerrs;  // 这么写为了消除yynerrs未使用的告警。如果你有更好的方法欢迎提PR
      (yyval.sql_node) = new ParsedSqlNode(SCF_EXIT);
    }
#line 1915 "yacc_sql.cpp"
    break;

  case 24: /* help_stmt: HELP  */
#line 276 "yacc_sql.y"
         {
      (yyval.sql_node) = new ParsedSqlNode(SCF_HELP);
    }
#line 1923 "yacc_sql.cpp"
    break;

  case 25: /* sync_stmt: SYNC  */
#line 281 "yacc_sql.y"
         {
      (yyval.sql_node) = new ParsedSqlNode(SCF_SYNC);
    }
#line 1931 "yacc_sql.cpp"
    break;

  case 26: /* begin_stmt: TRX_BEGIN  */
#line 287 "yacc_sql.y"
               {
      (yyval.sql_node) = new ParsedSqlNode(SCF_BEGIN);
    }
#line 1939 "yacc_sql.cpp"
    break;

  case 27: /* commit_stmt: TRX_COMMIT  */
#line 293 "yacc_sql.y"
               {
      (yyval.sql_node) = new ParsedSqlNode(SCF_COMMIT);
    }
#line 1947 "yacc_sql.cpp"
    break;

  case 28: /* rollback_stmt: TRX_ROLLBACK  */
#line 299 "yacc_sql.y"
                  {
      (yyval.sql_node) = new ParsedSqlNode(SCF_ROLLBACK);
    }
#line 1955 "yacc_sql.cpp"
    break;

  case 29: /* drop_table_stmt: DROP TABLE ID  */
#line 305 "yacc_sql.y"
                  {
      (yyval.sql_node) = new ParsedSqlNode(SCF_DROP_TABLE);
      (yyval.sql_node)->drop_table.relation_name = (yyvsp[0].string);
      free((yyvsp[0].string));
    }
#line 1965 "yacc_sql.cpp"
    break;

  case 30: /* show_tables_stmt: SHOW TABLES  */
#line 312 "yacc_sql.y"
                {
      (yyval.sql_node) = new ParsedSqlNode(SCF_SHOW_TABLES);
    }
#line 1973 "yacc_sql.cpp"
    break;

  case 31: /* desc_table_stmt: DESC ID  */
#line 318 "yacc_sql.y"
             {
      (yyval.sql_node) = new ParsedSqlNode(SCF_DESC_TABLE);
      (yyval.sql_node)->desc_table.relation_name = (yyvsp[0].string);
      free((yyvsp[0].string));
    }
#line 1983 "yacc_sql.cpp"
    break;

  case 32: /* create_index_stmt: CREATE UNIQUE INDEX ID ON ID LBRACE ID id_list RBRACE  */
#line 327 "yacc_sql.y"
    {
      (yyval.sql_node) = new ParsedSqlNode(SCF_CREATE_INDEX);
      CreateIndexSqlNode &create_index = (yyval.sql_node)->create_index;
      create_index.index_name = (yyvsp[-6].string);
      create_index.relation_name = (yyvsp[-4].string);
      create_index.attribute_name = (yyvsp[-2].string);
      create_index.attribute_names.push_back((yyvsp[-2].string));
      if ((yyvsp[-1].string_list) != nullptr) {
        create_index.attribute_names.insert(
            create_index.attribute_names.end(), (yyvsp[-1].string_list)->begin(), (yyvsp[-1].string_list)->end());
        delete (yyvsp[-1].string_list);
      }
      create_index.is_unique = true;
      free((yyvsp[-6].string));
      free((yyvsp[-4].string));
      free((yyvsp[-2].string));
    }
#line 2005 "yacc_sql.cpp"
    break;

  case 33: /* create_index_stmt: CREATE INDEX ID ON ID LBRACE ID id_list RBRACE  */
#line 345 "yacc_sql.y"
    {
      (yyval.sql_node) = new ParsedSqlNode(SCF_CREATE_INDEX);
      CreateIndexSqlNode &create_index = (yyval.sql_node)->create_index;
      create_index.index_name = (yyvsp[-6].string);
      create_index.relation_name = (yyvsp[-4].string);
      create_index.attribute_name = (yyvsp[-2].string);
      create_index.attribute_names.push_back((yyvsp[-2].string));
      if ((yyvsp[-1].string_list) != nullptr) {
        create_index.attribute_names.insert(
            create_index.attribute_names.end(), (yyvsp[-1].string_list)->begin(), (yyvsp[-1].string_list)->end());
        delete (yyvsp[-1].string_list);
      }
      free((yyvsp[-6].string));
      free((yyvsp[-4].string));
      free((yyvsp[-2].string));
    }
#line 2026 "yacc_sql.cpp"
    break;

  case 34: /* drop_index_stmt: DROP INDEX ID ON ID  */
#line 365 "yacc_sql.y"
    {
      (yyval.sql_node) = new ParsedSqlNode(SCF_DROP_INDEX);
      (yyval.sql_node)->drop_index.index_name = (yyvsp[-2].string);
      (yyval.sql_node)->drop_index.relation_name = (yyvsp[0].string);
      free((yyvsp[-2].string));
      free((yyvsp[0].string));
    }
#line 2038 "yacc_sql.cpp"
    break;

  case 35: /* create_table_stmt: CREATE TABLE ID LBRACE attr_def attr_def_list RBRACE  */
#line 375 "yacc_sql.y"
    {
      (yyval.sql_node) = new ParsedSqlNode(SCF_CREATE_TABLE);
      CreateTableSqlNode &create_table = (yyval.sql_node)->create_table;
      create_table.relation_name = (yyvsp[-4].string);
      free((yyvsp[-4].string));

      std::vector<AttrInfoSqlNode> *src_attrs = (yyvsp[-1].attr_infos);

      if (src_attrs != nullptr) {
        create_table.attr_infos.swap(*src_attrs);
      }
      create_table.attr_infos.emplace_back(*(yyvsp[-2].attr_info));
      std::reverse(create_table.attr_infos.begin(), create_table.attr_infos.end());
      delete (yyvsp[-2].attr_info);
    }
#line 2058 "yacc_sql.cpp"
    break;

  case 36: /* attr_def_list: %empty  */
#line 393 "yacc_sql.y"
    {
      (yyval.attr_infos) = nullptr;
    }
#line 2066 "yacc_sql.cpp"
    break;

  case 37: /* attr_def_list: COMMA attr_def attr_def_list  */
#line 397 "yacc_sql.y"
    {
      if ((yyvsp[0].attr_infos) != nullptr) {
        (yyval.attr_infos) = (yyvsp[0].attr_infos);
      } else {
        (yyval.attr_infos) = new std::vector<AttrInfoSqlNode>;
      }
      (yyval.attr_infos)->emplace_back(*(yyvsp[-1].attr_info));
      delete (yyvsp[-1].attr_info);
    }
#line 2080 "yacc_sql.cpp"
    break;

  case 38: /* attr_def: ID type LBRACE number RBRACE  */
#line 410 "yacc_sql.y"
    {
      (yyval.attr_info) = new AttrInfoSqlNode;
      (yyval.attr_info)->type = (AttrType)(yyvsp[-3].number);
      (yyval.attr_info)->name = (yyvsp[-4].string);
      (yyval.attr_info)->length = (yyvsp[-1].number);
      free((yyvsp[-4].string));
    }
#line 2092 "yacc_sql.cpp"
    break;

  case 39: /* attr_def: ID type LBRACE number RBRACE NULL_T  */
#line 418 "yacc_sql.y"
    {
      (yyval.attr_info) = new AttrInfoSqlNode;
      (yyval.attr_info)->type = (AttrType)(yyvsp[-4].number);
      (yyval.attr_info)->name = (yyvsp[-5].string);
      (yyval.attr_info)->length = (yyvsp[-2].number);
      (yyval.attr_info)->nullable = true;
      free((yyvsp[-5].string));
    }
#line 2105 "yacc_sql.cpp"
    break;

  case 40: /* attr_def: ID type LBRACE number RBRACE NOT NULL_T  */
#line 427 "yacc_sql.y"
    {
      (yyval.attr_info) = new AttrInfoSqlNode;
      (yyval.attr_info)->type = (AttrType)(yyvsp[-5].number);
      (yyval.attr_info)->name = (yyvsp[-6].string);
      (yyval.attr_info)->length = (yyvsp[-3].number);
      (yyval.attr_info)->nullable = false;
      free((yyvsp[-6].string));
    }
#line 2118 "yacc_sql.cpp"
    break;

  case 41: /* attr_def: ID type LBRACE number RBRACE NULLABLE_T  */
#line 436 "yacc_sql.y"
    {
      (yyval.attr_info) = new AttrInfoSqlNode;
      (yyval.attr_info)->type = (AttrType)(yyvsp[-4].number);
      (yyval.attr_info)->name = (yyvsp[-5].string);
      (yyval.attr_info)->length = (yyvsp[-2].number);
      (yyval.attr_info)->nullable = true;
      free((yyvsp[-5].string));
    }
#line 2131 "yacc_sql.cpp"
    break;

  case 42: /* attr_def: ID type LBRACE number RBRACE NOT NULLABLE_T  */
#line 445 "yacc_sql.y"
    {
      (yyval.attr_info) = new AttrInfoSqlNode;
      (yyval.attr_info)->type = (AttrType)(yyvsp[-5].number);
      (yyval.attr_info)->name = (yyvsp[-6].string);
      (yyval.attr_info)->length = (yyvsp[-3].number);
      (yyval.attr_info)->nullable = false;
      free((yyvsp[-6].string));
    }
#line 2144 "yacc_sql.cpp"
    break;

  case 43: /* attr_def: ID type  */
#line 454 "yacc_sql.y"
    {
      (yyval.attr_info) = new AttrInfoSqlNode;
      (yyval.attr_info)->type = (AttrType)(yyvsp[0].number);
      (yyval.attr_info)->name = (yyvsp[-1].string);
      (yyval.attr_info)->length = ((yyvsp[0].number) == TEXTS) ? 4096 : 4;
      free((yyvsp[-1].string));
    }
#line 2156 "yacc_sql.cpp"
    break;

  case 44: /* attr_def: ID type NULL_T  */
#line 462 "yacc_sql.y"
    {
      (yyval.attr_info) = new AttrInfoSqlNode;
      (yyval.attr_info)->type = (AttrType)(yyvsp[-1].number);
      (yyval.attr_info)->name = (yyvsp[-2].string);
      (yyval.attr_info)->length = ((yyvsp[-1].number) == TEXTS) ? 4096 : 4;
      (yyval.attr_info)->nullable = true;
      free((yyvsp[-2].string));
    }
#line 2169 "yacc_sql.cpp"
    break;

  case 45: /* attr_def: ID type NOT NULL_T  */
#line 471 "yacc_sql.y"
    {
      (yyval.attr_info) = new AttrInfoSqlNode;
      (yyval.attr_info)->type = (AttrType)(yyvsp[-2].number);
      (yyval.attr_info)->name = (yyvsp[-3].string);
      (yyval.attr_info)->length = ((yyvsp[-2].number) == TEXTS) ? 4096 : 4;
      (yyval.attr_info)->nullable = false;
      free((yyvsp[-3].string));
    }
#line 2182 "yacc_sql.cpp"
    break;

  case 46: /* attr_def: ID type NULLABLE_T  */
#line 480 "yacc_sql.y"
    {
      (yyval.attr_info) = new AttrInfoSqlNode;
      (yyval.attr_info)->type = (AttrType)(yyvsp[-1].number);
      (yyval.attr_info)->name = (yyvsp[-2].string);
      (yyval.attr_info)->length = ((yyvsp[-1].number) == TEXTS) ? 4096 : 4;
      (yyval.attr_info)->nullable = true;
      free((yyvsp[-2].string));
    }
#line 2195 "yacc_sql.cpp"
    break;

  case 47: /* attr_def: ID type NOT NULLABLE_T  */
#line 489 "yacc_sql.y"
    {
      (yyval.attr_info) = new AttrInfoSqlNode;
      (yyval.attr_info)->type = (AttrType)(yyvsp[-2].number);
      (yyval.attr_info)->name = (yyvsp[-3].string);
      (yyval.attr_info)->length = ((yyvsp[-2].number) == TEXTS) ? 4096 : 4;
      (yyval.attr_info)->nullable = false;
      free((yyvsp[-3].string));
    }
#line 2208 "yacc_sql.cpp"
    break;

  case 48: /* number: NUMBER  */
#line 499 "yacc_sql.y"
           {(yyval.number) = (yyvsp[0].number);}
#line 2214 "yacc_sql.cpp"
    break;

  case 49: /* type: INT_T  */
#line 502 "yacc_sql.y"
               { (yyval.number)=INTS; }
#line 2220 "yacc_sql.cpp"
    break;

  case 50: /* type: STRING_T  */
#line 503 "yacc_sql.y"
               { (yyval.number)=CHARS; }
#line 2226 "yacc_sql.cpp"
    break;

  case 51: /* type: FLOAT_T  */
#line 504 "yacc_sql.y"
               { (yyval.number)=FLOATS; }
#line 2232 "yacc_sql.cpp"
    break;

  case 52: /* type: DATE_T  */
#line 505 "yacc_sql.y"
               { (yyval.number)=DATES; }
#line 2238 "yacc_sql.cpp"
    break;

  case 53: /* type: TEXT_T  */
#line 506 "yacc_sql.y"
               { (yyval.number)=TEXTS; }
#line 2244 "yacc_sql.cpp"
    break;

  case 54: /* insert_stmt: INSERT INTO ID VALUES LBRACE value value_list RBRACE  */
#line 510 "yacc_sql.y"
    {
      (yyval.sql_node) = new ParsedSqlNode(SCF_INSERT);
      (yyval.sql_node)->insertion.relation_name = (yyvsp[-5].string);
      if ((yyvsp[-1].value_list) != nullptr) {
        (yyval.sql_node)->insertion.values.swap(*(yyvsp[-1].value_list));
      }
      (yyval.sql_node)->insertion.values.emplace_back(*(yyvsp[-2].value));
      std::reverse((yyval.sql_node)->insertion.values.begin(), (yyval.sql_node)->insertion.values.end());
      delete (yyvsp[-2].value);
      free((yyvsp[-5].string));
    }
#line 2260 "yacc_sql.cpp"
    break;

  case 55: /* value_list: %empty  */
#line 525 "yacc_sql.y"
    {
      (yyval.value_list) = nullptr;
    }
#line 2268 "yacc_sql.cpp"
    break;

  case 56: /* value_list: COMMA value value_list  */
#line 528 "yacc_sql.y"
                              {
      if ((yyvsp[0].value_list) != nullptr) {
        (yyval.value_list) = (yyvsp[0].value_list);
      } else {
        (yyval.value_list) = new std::vector<Value>;
      }
      (yyval.value_list)->emplace_back(*(yyvsp[-1].value));
      delete (yyvsp[-1].value);
    }
#line 2282 "yacc_sql.cpp"
    break;

  case 57: /* id_list: %empty  */
#line 540 "yacc_sql.y"
    {
      (yyval.string_list) = nullptr;
    }
#line 2290 "yacc_sql.cpp"
    break;

  case 58: /* id_list: COMMA ID id_list  */
#line 544 "yacc_sql.y"
    {
      if ((yyvsp[0].string_list) != nullptr) {
        (yyval.string_list) = (yyvsp[0].string_list);
      } else {
        (yyval.string_list) = new std::vector<std::string>;
      }
      (yyval.string_list)->emplace_back((yyvsp[-1].string));
      free((yyvsp[-1].string));
    }
#line 2304 "yacc_sql.cpp"
    break;

  case 59: /* value: NUMBER  */
#line 555 "yacc_sql.y"
           {
      (yyval.value) = new Value((int)(yyvsp[0].number));
      (yyloc) = (yylsp[0]);
    }
#line 2313 "yacc_sql.cpp"
    break;

  case 60: /* value: '-' NUMBER  */
#line 559 "yacc_sql.y"
                 {
      (yyval.value) = new Value(-(int)(yyvsp[0].number));
      (yyloc) = (yylsp[0]);
    }
#line 2322 "yacc_sql.cpp"
    break;

  case 61: /* value: FLOAT  */
#line 563 "yacc_sql.y"
           {
      (yyval.value) = new Value((float)(yyvsp[0].floats));
      (yyloc) = (yylsp[0]);
    }
#line 2331 "yacc_sql.cpp"
    break;

  case 62: /* value: '-' FLOAT  */
#line 567 "yacc_sql.y"
                {
      (yyval.value) = new Value(-(float)(yyvsp[0].floats));
      (yyloc) = (yylsp[0]);
    }
#line 2340 "yacc_sql.cpp"
    break;

  case 63: /* value: SSS  */
#line 571 "yacc_sql.y"
         {
  char *tmp = common::substr((yyvsp[0].string), 1, strlen((yyvsp[0].string))-2);
  int y, m, d;
  if (sscanf(tmp, "%d-%d-%d", &y, &m, &d) == 3 && check_date(y, m, d)) {
    int date_val = y * 10000 + m * 100 + d;
    (yyval.value) = new Value();
    (yyval.value)->set_date(date_val);
  } else {
    (yyval.value) = new Value(tmp);
  }
  free(tmp);
  free((yyvsp[0].string));
}
#line 2358 "yacc_sql.cpp"
    break;

  case 64: /* value: NULL_T  */
#line 584 "yacc_sql.y"
             {
      (yyval.value) = new Value();
      (yyval.value)->set_null(true);
      (yyloc) = (yylsp[0]);
    }
#line 2368 "yacc_sql.cpp"
    break;

  case 65: /* delete_stmt: DELETE FROM ID where  */
#line 593 "yacc_sql.y"
    {
      (yyval.sql_node) = new ParsedSqlNode(SCF_DELETE);
      (yyval.sql_node)->deletion.relation_name = (yyvsp[-1].string);
      if ((yyvsp[0].condition_list) != nullptr) {
        (yyval.sql_node)->deletion.conditions.swap(*(yyvsp[0].condition_list));
        delete (yyvsp[0].condition_list);
      }
      free((yyvsp[-1].string));
    }
#line 2382 "yacc_sql.cpp"
    break;

  case 66: /* update_stmt: UPDATE ID SET update_assign update_assign_list where  */
#line 605 "yacc_sql.y"
    {
      (yyval.sql_node) = new ParsedSqlNode(SCF_UPDATE);
      (yyval.sql_node)->update.relation_name = (yyvsp[-4].string);
      if ((yyvsp[-1].update_assign_list) != nullptr) {
        (yyval.sql_node)->update.assignments.swap(*(yyvsp[-1].update_assign_list));
        delete (yyvsp[-1].update_assign_list);
      }
      (yyval.sql_node)->update.assignments.emplace_back(std::move(*(yyvsp[-2].update_assign)));
      std::reverse((yyval.sql_node)->update.assignments.begin(), (yyval.sql_node)->update.assignments.end());
      delete (yyvsp[-2].update_assign);
      if ((yyvsp[0].condition_list) != nullptr) {
        (yyval.sql_node)->update.conditions.swap(*(yyvsp[0].condition_list));
        delete (yyvsp[0].condition_list);
      }
      free((yyvsp[-4].string));
    }
#line 2403 "yacc_sql.cpp"
    break;

  case 67: /* update_assign_list: %empty  */
#line 624 "yacc_sql.y"
    {
      (yyval.update_assign_list) = nullptr;
    }
#line 2411 "yacc_sql.cpp"
    break;

  case 68: /* update_assign_list: COMMA update_assign update_assign_list  */
#line 628 "yacc_sql.y"
    {
      if ((yyvsp[0].update_assign_list) != nullptr) {
        (yyval.update_assign_list) = (yyvsp[0].update_assign_list);
      } else {
        (yyval.update_assign_list) = new std::vector<UpdateAssignmentNode>;
      }
      (yyval.update_assign_list)->emplace_back(std::move(*(yyvsp[-1].update_assign)));
      delete (yyvsp[-1].update_assign);
    }
#line 2425 "yacc_sql.cpp"
    break;

  case 69: /* update_assign: ID EQ value  */
#line 640 "yacc_sql.y"
    {
      (yyval.update_assign) = new UpdateAssignmentNode;
      (yyval.update_assign)->attribute_name = (yyvsp[-2].string);
      (yyval.update_assign)->value = *(yyvsp[0].value);
      (yyval.update_assign)->is_subquery = false;
      free((yyvsp[-2].string));
      delete (yyvsp[0].value);
    }
#line 2438 "yacc_sql.cpp"
    break;

  case 70: /* update_assign: ID EQ LBRACE select_stmt RBRACE  */
#line 649 "yacc_sql.y"
    {
      auto *sel = new SelectSqlNode;
      sel->attributes.swap((yyvsp[-1].sql_node)->selection.attributes);
      sel->expressions.swap((yyvsp[-1].sql_node)->selection.expressions);
      sel->relations.swap((yyvsp[-1].sql_node)->selection.relations);
      sel->aliases.swap((yyvsp[-1].sql_node)->selection.aliases);
      sel->conditions.swap((yyvsp[-1].sql_node)->selection.conditions);
      sel->group_by.swap((yyvsp[-1].sql_node)->selection.group_by);
      sel->order_by.swap((yyvsp[-1].sql_node)->selection.order_by);
      (yyval.update_assign) = new UpdateAssignmentNode;
      (yyval.update_assign)->attribute_name = (yyvsp[-4].string);
      (yyval.update_assign)->is_subquery = true;
      (yyval.update_assign)->subquery = sel;
      free((yyvsp[-4].string));
      delete (yyvsp[-1].sql_node);
    }
#line 2459 "yacc_sql.cpp"
    break;

  case 71: /* alias_opt: %empty  */
#line 667 "yacc_sql.y"
                { (yyval.string) = nullptr; }
#line 2465 "yacc_sql.cpp"
    break;

  case 72: /* alias_opt: ID  */
#line 668 "yacc_sql.y"
         { (yyval.string) = (yyvsp[0].string); }
#line 2471 "yacc_sql.cpp"
    break;

  case 73: /* alias_opt: AS ID  */
#line 669 "yacc_sql.y"
            { (yyval.string) = (yyvsp[0].string); }
#line 2477 "yacc_sql.cpp"
    break;

  case 74: /* select_stmt: SELECT select_expr_list where group_by_clause having_clause order_by_clause  */
#line 674 "yacc_sql.y"
    {
      (yyval.sql_node) = new ParsedSqlNode(SCF_SELECT);
      if ((yyvsp[-4].select_expr_list) != nullptr) {
        (yyval.sql_node)->selection.expressions.swap(*(yyvsp[-4].select_expr_list));
        delete (yyvsp[-4].select_expr_list);
      }
      if ((yyvsp[-3].condition_list) != nullptr) {
        for (auto &c : *(yyvsp[-3].condition_list)) {
          (yyval.sql_node)->selection.conditions.emplace_back(std::move(c));
        }
        delete (yyvsp[-3].condition_list);
      }
      if ((yyvsp[-2].rel_attr_list) != nullptr) {
        std::reverse((yyvsp[-2].rel_attr_list)->begin(), (yyvsp[-2].rel_attr_list)->end());
        (yyval.sql_node)->selection.group_by.swap(*(yyvsp[-2].rel_attr_list));
        delete (yyvsp[-2].rel_attr_list);
      }
      if ((yyvsp[-1].condition_list) != nullptr) {
        for (auto &c : *(yyvsp[-1].condition_list)) {
          (yyval.sql_node)->selection.having.emplace_back(std::move(c));
        }
        delete (yyvsp[-1].condition_list);
      }
      if ((yyvsp[0].order_by_list) != nullptr) {
        std::reverse((yyvsp[0].order_by_list)->begin(), (yyvsp[0].order_by_list)->end());
        (yyval.sql_node)->selection.order_by.swap(*(yyvsp[0].order_by_list));
        delete (yyvsp[0].order_by_list);
      }
    }
#line 2511 "yacc_sql.cpp"
    break;

  case 75: /* select_stmt: SELECT select_expr_list FROM ID alias_opt join_clause where group_by_clause having_clause order_by_clause  */
#line 704 "yacc_sql.y"
    {
      (yyval.sql_node) = new ParsedSqlNode(SCF_SELECT);
      if ((yyvsp[-8].select_expr_list) != nullptr) {
        (yyval.sql_node)->selection.expressions.swap(*(yyvsp[-8].select_expr_list));
        delete (yyvsp[-8].select_expr_list);
      }
      if ((yyvsp[-4].join_clause) != nullptr) {
        for (auto &r : (yyvsp[-4].join_clause)->relations) {
          (yyval.sql_node)->selection.relations.push_back(r);
        }
        for (auto &c : (yyvsp[-4].join_clause)->conditions) {
          (yyval.sql_node)->selection.conditions.emplace_back(std::move(c));
        }
        for (auto &a : (yyvsp[-4].join_clause)->aliases) {
          (yyval.sql_node)->selection.aliases.push_back(a);
        }
        delete (yyvsp[-4].join_clause);
      }
      (yyval.sql_node)->selection.relations.push_back((yyvsp[-6].string));
      (yyval.sql_node)->selection.aliases.push_back((yyvsp[-5].string) ? (yyvsp[-5].string) : "");
      std::reverse((yyval.sql_node)->selection.relations.begin(), (yyval.sql_node)->selection.relations.end());
      std::reverse((yyval.sql_node)->selection.aliases.begin(), (yyval.sql_node)->selection.aliases.end());

      if ((yyvsp[-3].condition_list) != nullptr) {
        for (auto &c : *(yyvsp[-3].condition_list)) {
          (yyval.sql_node)->selection.conditions.emplace_back(std::move(c));
        }
        delete (yyvsp[-3].condition_list);
      }
      if ((yyvsp[-2].rel_attr_list) != nullptr) {
        std::reverse((yyvsp[-2].rel_attr_list)->begin(), (yyvsp[-2].rel_attr_list)->end());
        (yyval.sql_node)->selection.group_by.swap(*(yyvsp[-2].rel_attr_list));
        delete (yyvsp[-2].rel_attr_list);
      }
      if ((yyvsp[-1].condition_list) != nullptr) {
        for (auto &c : *(yyvsp[-1].condition_list)) {
          (yyval.sql_node)->selection.having.emplace_back(std::move(c));
        }
        delete (yyvsp[-1].condition_list);
      }
      if ((yyvsp[0].order_by_list) != nullptr) {
        std::reverse((yyvsp[0].order_by_list)->begin(), (yyvsp[0].order_by_list)->end());
        (yyval.sql_node)->selection.order_by.swap(*(yyvsp[0].order_by_list));
        delete (yyvsp[0].order_by_list);
      }
      free((yyvsp[-6].string));
      if ((yyvsp[-5].string)) free((yyvsp[-5].string));
    }
#line 2564 "yacc_sql.cpp"
    break;

  case 76: /* select_stmt: SELECT select_attr FROM ID alias_opt join_clause where group_by_clause having_clause order_by_clause  */
#line 753 "yacc_sql.y"
    {
      (yyval.sql_node) = new ParsedSqlNode(SCF_SELECT);
      if ((yyvsp[-8].rel_attr_list) != nullptr) {
        (yyval.sql_node)->selection.attributes.swap(*(yyvsp[-8].rel_attr_list));
        delete (yyvsp[-8].rel_attr_list);
      }
      if ((yyvsp[-4].join_clause) != nullptr) {
        for (auto &r : (yyvsp[-4].join_clause)->relations) {
          (yyval.sql_node)->selection.relations.push_back(r);
        }
        for (auto &c : (yyvsp[-4].join_clause)->conditions) {
          (yyval.sql_node)->selection.conditions.emplace_back(std::move(c));
        }
        for (auto &a : (yyvsp[-4].join_clause)->aliases) {
          (yyval.sql_node)->selection.aliases.push_back(a);
        }
        delete (yyvsp[-4].join_clause);
      }
      (yyval.sql_node)->selection.relations.push_back((yyvsp[-6].string));
      (yyval.sql_node)->selection.aliases.push_back((yyvsp[-5].string) ? (yyvsp[-5].string) : "");
      std::reverse((yyval.sql_node)->selection.relations.begin(), (yyval.sql_node)->selection.relations.end());
      std::reverse((yyval.sql_node)->selection.aliases.begin(), (yyval.sql_node)->selection.aliases.end());
      if ((yyvsp[-3].condition_list) != nullptr) {
        for (auto &c : *(yyvsp[-3].condition_list)) {
          (yyval.sql_node)->selection.conditions.emplace_back(std::move(c));
        }
        delete (yyvsp[-3].condition_list);
      }
      if ((yyvsp[-2].rel_attr_list) != nullptr) {
        std::reverse((yyvsp[-2].rel_attr_list)->begin(), (yyvsp[-2].rel_attr_list)->end());
        (yyval.sql_node)->selection.group_by.swap(*(yyvsp[-2].rel_attr_list));
        delete (yyvsp[-2].rel_attr_list);
      }
      if ((yyvsp[-1].condition_list) != nullptr) {
        for (auto &c : *(yyvsp[-1].condition_list)) {
          (yyval.sql_node)->selection.having.emplace_back(std::move(c));
        }
        delete (yyvsp[-1].condition_list);
      }
      if ((yyvsp[0].order_by_list) != nullptr) {
        std::reverse((yyvsp[0].order_by_list)->begin(), (yyvsp[0].order_by_list)->end());
        (yyval.sql_node)->selection.order_by.swap(*(yyvsp[0].order_by_list));
        delete (yyvsp[0].order_by_list);
      }
      free((yyvsp[-6].string));
      if ((yyvsp[-5].string)) free((yyvsp[-5].string));
    }
#line 2616 "yacc_sql.cpp"
    break;

  case 77: /* calc_stmt: CALC expression_list  */
#line 803 "yacc_sql.y"
    {
      (yyval.sql_node) = new ParsedSqlNode(SCF_CALC);
      std::reverse((yyvsp[0].expression_list)->begin(), (yyvsp[0].expression_list)->end());
      (yyval.sql_node)->calc.expressions.swap(*(yyvsp[0].expression_list));
      delete (yyvsp[0].expression_list);
    }
#line 2627 "yacc_sql.cpp"
    break;

  case 78: /* expression_list: expression  */
#line 813 "yacc_sql.y"
    {
      (yyval.expression_list) = new std::vector<Expression*>;
      (yyval.expression_list)->emplace_back((yyvsp[0].expression));
    }
#line 2636 "yacc_sql.cpp"
    break;

  case 79: /* expression_list: expression COMMA expression_list  */
#line 818 "yacc_sql.y"
    {
      if ((yyvsp[0].expression_list) != nullptr) {
        (yyval.expression_list) = (yyvsp[0].expression_list);
      } else {
        (yyval.expression_list) = new std::vector<Expression *>;
      }
      (yyval.expression_list)->emplace_back((yyvsp[-2].expression));
    }
#line 2649 "yacc_sql.cpp"
    break;

  case 80: /* expression: expression '+' expression  */
#line 828 "yacc_sql.y"
                              {
      (yyval.expression) = create_arithmetic_expression(ArithmeticExpr::Type::ADD, (yyvsp[-2].expression), (yyvsp[0].expression), sql_string, &(yyloc));
    }
#line 2657 "yacc_sql.cpp"
    break;

  case 81: /* expression: expression '-' expression  */
#line 831 "yacc_sql.y"
                                {
      (yyval.expression) = create_arithmetic_expression(ArithmeticExpr::Type::SUB, (yyvsp[-2].expression), (yyvsp[0].expression), sql_string, &(yyloc));
    }
#line 2665 "yacc_sql.cpp"
    break;

  case 82: /* expression: expression '*' expression  */
#line 834 "yacc_sql.y"
                                {
      (yyval.expression) = create_arithmetic_expression(ArithmeticExpr::Type::MUL, (yyvsp[-2].expression), (yyvsp[0].expression), sql_string, &(yyloc));
    }
#line 2673 "yacc_sql.cpp"
    break;

  case 83: /* expression: expression '/' expression  */
#line 837 "yacc_sql.y"
                                {
      (yyval.expression) = create_arithmetic_expression(ArithmeticExpr::Type::DIV, (yyvsp[-2].expression), (yyvsp[0].expression), sql_string, &(yyloc));
    }
#line 2681 "yacc_sql.cpp"
    break;

  case 84: /* expression: LBRACE expression RBRACE  */
#line 840 "yacc_sql.y"
                               {
      (yyval.expression) = (yyvsp[-1].expression);
      (yyval.expression)->set_name(token_name(sql_string, &(yyloc)));
    }
#line 2690 "yacc_sql.cpp"
    break;

  case 85: /* expression: '-' expression  */
#line 844 "yacc_sql.y"
                                  {
      (yyval.expression) = create_arithmetic_expression(ArithmeticExpr::Type::NEGATIVE, (yyvsp[0].expression), nullptr, sql_string, &(yyloc));
    }
#line 2698 "yacc_sql.cpp"
    break;

  case 86: /* expression: LBRACE select_stmt RBRACE  */
#line 847 "yacc_sql.y"
                                {
      auto *sel = new SelectSqlNode;
      sel->attributes.swap((yyvsp[-1].sql_node)->selection.attributes);
      sel->expressions.swap((yyvsp[-1].sql_node)->selection.expressions);
      sel->relations.swap((yyvsp[-1].sql_node)->selection.relations);
      sel->aliases.swap((yyvsp[-1].sql_node)->selection.aliases);
      sel->conditions.swap((yyvsp[-1].sql_node)->selection.conditions);
      sel->order_by.swap((yyvsp[-1].sql_node)->selection.order_by);
      (yyval.expression) = new SubQueryExpr(sel);
      (yyval.expression)->set_name(token_name(sql_string, &(yyloc)));
      delete (yyvsp[-1].sql_node);
    }
#line 2715 "yacc_sql.cpp"
    break;

  case 87: /* expression: ID LBRACE expression_list RBRACE  */
#line 859 "yacc_sql.y"
                                       {
      std::vector<std::unique_ptr<Expression>> args;
      if ((yyvsp[-1].expression_list) != nullptr) {
        std::reverse((yyvsp[-1].expression_list)->begin(), (yyvsp[-1].expression_list)->end());
        for (auto *e : *(yyvsp[-1].expression_list)) {
          args.emplace_back(e);
        }
        delete (yyvsp[-1].expression_list);
      }
      (yyval.expression) = new FunctionExpr((yyvsp[-3].string), std::move(args));
      (yyval.expression)->set_name(token_name(sql_string, &(yyloc)));
      free((yyvsp[-3].string));
    }
#line 2733 "yacc_sql.cpp"
    break;

  case 88: /* expression: value  */
#line 872 "yacc_sql.y"
            {
      (yyval.expression) = new ValueExpr(*(yyvsp[0].value));
      (yyval.expression)->set_name(token_name(sql_string, &(yyloc)));
      delete (yyvsp[0].value);
    }
#line 2743 "yacc_sql.cpp"
    break;

  case 89: /* expression: ID  */
#line 877 "yacc_sql.y"
         {
      (yyval.expression) = new UnboundFieldExpr("", (yyvsp[0].string));
      (yyval.expression)->set_name((yyvsp[0].string));
      free((yyvsp[0].string));
    }
#line 2753 "yacc_sql.cpp"
    break;

  case 90: /* expression: ID DOT ID  */
#line 882 "yacc_sql.y"
                {
      (yyval.expression) = new UnboundFieldExpr((yyvsp[-2].string), (yyvsp[0].string));
      (yyval.expression)->set_name((yyvsp[0].string));
      free((yyvsp[-2].string));
      free((yyvsp[0].string));
    }
#line 2764 "yacc_sql.cpp"
    break;

  case 91: /* expression: agg_expr  */
#line 888 "yacc_sql.y"
               {
      (yyval.expression) = new AggregationExpr((yyvsp[0].rel_attr)->aggregation_type, (yyvsp[0].rel_attr)->relation_name, (yyvsp[0].rel_attr)->attribute_name);
      delete (yyvsp[0].rel_attr);
    }
#line 2773 "yacc_sql.cpp"
    break;

  case 92: /* agg_expr: MAX LBRACE rel_attr RBRACE  */
#line 897 "yacc_sql.y"
    {
      (yyval.rel_attr) = (yyvsp[-1].rel_attr);
      (yyval.rel_attr)->aggregation_type = AGG_MAX;
    }
#line 2782 "yacc_sql.cpp"
    break;

  case 93: /* agg_expr: MIN LBRACE rel_attr RBRACE  */
#line 902 "yacc_sql.y"
    {
      (yyval.rel_attr) = (yyvsp[-1].rel_attr);
      (yyval.rel_attr)->aggregation_type = AGG_MIN;
    }
#line 2791 "yacc_sql.cpp"
    break;

  case 94: /* agg_expr: COUNT LBRACE rel_attr RBRACE  */
#line 907 "yacc_sql.y"
    {
      (yyval.rel_attr) = (yyvsp[-1].rel_attr);
      (yyval.rel_attr)->aggregation_type = AGG_COUNT;
    }
#line 2800 "yacc_sql.cpp"
    break;

  case 95: /* agg_expr: COUNT LBRACE '*' RBRACE  */
#line 912 "yacc_sql.y"
    {
      (yyval.rel_attr) = new RelAttrSqlNode;
      (yyval.rel_attr)->attribute_name = "*";
      (yyval.rel_attr)->aggregation_type = AGG_COUNT;
    }
#line 2810 "yacc_sql.cpp"
    break;

  case 96: /* agg_expr: AVG LBRACE rel_attr RBRACE  */
#line 918 "yacc_sql.y"
    {
      (yyval.rel_attr) = (yyvsp[-1].rel_attr);
      (yyval.rel_attr)->aggregation_type = AGG_AVG;
    }
#line 2819 "yacc_sql.cpp"
    break;

  case 97: /* agg_expr: SUM LBRACE rel_attr RBRACE  */
#line 923 "yacc_sql.y"
    {
      (yyval.rel_attr) = (yyvsp[-1].rel_attr);
      (yyval.rel_attr)->aggregation_type = AGG_SUM;
    }
#line 2828 "yacc_sql.cpp"
    break;

  case 98: /* select_attr: '*'  */
#line 930 "yacc_sql.y"
        {
      (yyval.rel_attr_list) = new std::vector<RelAttrSqlNode>;
      RelAttrSqlNode attr;
      attr.relation_name  = "";
      attr.attribute_name = "*";
      (yyval.rel_attr_list)->emplace_back(attr);
    }
#line 2840 "yacc_sql.cpp"
    break;

  case 99: /* select_expr_item: expression alias_opt  */
#line 943 "yacc_sql.y"
    {
      (yyval.select_expr) = new SelectExprNode;
      (yyval.select_expr)->expr = (yyvsp[-1].expression);
      (yyval.select_expr)->alias = (yyvsp[0].string) ? (yyvsp[0].string) : (yyvsp[-1].expression)->name();
      if ((yyvsp[0].string)) free((yyvsp[0].string));
    }
#line 2851 "yacc_sql.cpp"
    break;

  case 100: /* select_expr_item: agg_expr alias_opt  */
#line 950 "yacc_sql.y"
    {
      (yyval.select_expr) = new SelectExprNode;
      (yyval.select_expr)->agg_type = (yyvsp[-1].rel_attr)->aggregation_type;
      (yyval.select_expr)->agg_field = (yyvsp[-1].rel_attr)->attribute_name;
      (yyval.select_expr)->agg_table = (yyvsp[-1].rel_attr)->relation_name;
      if ((yyvsp[0].string)) {
        (yyval.select_expr)->alias = (yyvsp[0].string);
        free((yyvsp[0].string));
      } else if ((yyvsp[-1].rel_attr)->attribute_name == "*") {
        (yyval.select_expr)->alias = "count(*)";
      } else if (!common::is_blank((yyvsp[-1].rel_attr)->relation_name.c_str())) {
        (yyval.select_expr)->alias = (yyvsp[-1].rel_attr)->relation_name + "." + (yyvsp[-1].rel_attr)->attribute_name;
      } else {
        (yyval.select_expr)->alias = (yyvsp[-1].rel_attr)->attribute_name;
      }
      delete (yyvsp[-1].rel_attr);
    }
#line 2873 "yacc_sql.cpp"
    break;

  case 101: /* select_expr_item: ID DOT '*'  */
#line 968 "yacc_sql.y"
    {
      (yyval.select_expr) = new SelectExprNode;
      (yyval.select_expr)->is_star = true;
      (yyval.select_expr)->star_table = (yyvsp[-2].string);
      free((yyvsp[-2].string));
    }
#line 2884 "yacc_sql.cpp"
    break;

  case 102: /* select_expr_list: select_expr_item  */
#line 978 "yacc_sql.y"
    {
      (yyval.select_expr_list) = new std::vector<SelectExprNode>;
      (yyval.select_expr_list)->emplace_back(std::move(*(yyvsp[0].select_expr)));
      delete (yyvsp[0].select_expr);
    }
#line 2894 "yacc_sql.cpp"
    break;

  case 103: /* select_expr_list: select_expr_list COMMA select_expr_item  */
#line 984 "yacc_sql.y"
    {
      (yyval.select_expr_list) = (yyvsp[-2].select_expr_list);
      (yyval.select_expr_list)->emplace_back(std::move(*(yyvsp[0].select_expr)));
      delete (yyvsp[0].select_expr);
    }
#line 2904 "yacc_sql.cpp"
    break;

  case 104: /* rel_attr: ID  */
#line 992 "yacc_sql.y"
       {
      (yyval.rel_attr) = new RelAttrSqlNode;
      (yyval.rel_attr)->attribute_name = (yyvsp[0].string);
      free((yyvsp[0].string));
    }
#line 2914 "yacc_sql.cpp"
    break;

  case 105: /* rel_attr: ID DOT ID  */
#line 997 "yacc_sql.y"
                {
      (yyval.rel_attr) = new RelAttrSqlNode;
      (yyval.rel_attr)->relation_name  = (yyvsp[-2].string);
      (yyval.rel_attr)->attribute_name = (yyvsp[0].string);
      free((yyvsp[-2].string));
      free((yyvsp[0].string));
    }
#line 2926 "yacc_sql.cpp"
    break;

  case 106: /* rel_attr: MAX LBRACE rel_attr RBRACE  */
#line 1004 "yacc_sql.y"
                                 {
      (yyval.rel_attr) = (yyvsp[-1].rel_attr);
      (yyval.rel_attr)->aggregation_type = AGG_MAX;
    }
#line 2935 "yacc_sql.cpp"
    break;

  case 107: /* rel_attr: MIN LBRACE rel_attr RBRACE  */
#line 1008 "yacc_sql.y"
                                 {
      (yyval.rel_attr) = (yyvsp[-1].rel_attr);
      (yyval.rel_attr)->aggregation_type = AGG_MIN;
    }
#line 2944 "yacc_sql.cpp"
    break;

  case 108: /* rel_attr: COUNT LBRACE rel_attr RBRACE  */
#line 1012 "yacc_sql.y"
                                   {
      (yyval.rel_attr) = (yyvsp[-1].rel_attr);
      (yyval.rel_attr)->aggregation_type = AGG_COUNT;
    }
#line 2953 "yacc_sql.cpp"
    break;

  case 109: /* rel_attr: COUNT LBRACE '*' RBRACE  */
#line 1016 "yacc_sql.y"
                              {
      (yyval.rel_attr) = new RelAttrSqlNode;
      (yyval.rel_attr)->attribute_name = "*";
      (yyval.rel_attr)->aggregation_type = AGG_COUNT;
    }
#line 2963 "yacc_sql.cpp"
    break;

  case 110: /* rel_attr: AVG LBRACE rel_attr RBRACE  */
#line 1021 "yacc_sql.y"
                                 {
      (yyval.rel_attr) = (yyvsp[-1].rel_attr);
      (yyval.rel_attr)->aggregation_type = AGG_AVG;
    }
#line 2972 "yacc_sql.cpp"
    break;

  case 111: /* rel_attr: SUM LBRACE rel_attr RBRACE  */
#line 1025 "yacc_sql.y"
                                 {
      (yyval.rel_attr) = (yyvsp[-1].rel_attr);
      (yyval.rel_attr)->aggregation_type = AGG_SUM;
    }
#line 2981 "yacc_sql.cpp"
    break;

  case 112: /* join_clause: %empty  */
#line 1050 "yacc_sql.y"
    {
      (yyval.join_clause) = new JoinClauseNode;
    }
#line 2989 "yacc_sql.cpp"
    break;

  case 113: /* join_clause: COMMA ID alias_opt join_clause  */
#line 1054 "yacc_sql.y"
    {
      (yyval.join_clause) = (yyvsp[0].join_clause);
      (yyval.join_clause)->relations.push_back((yyvsp[-2].string));
      (yyval.join_clause)->aliases.push_back((yyvsp[-1].string) ? (yyvsp[-1].string) : "");
      free((yyvsp[-2].string));
      if ((yyvsp[-1].string)) free((yyvsp[-1].string));
    }
#line 3001 "yacc_sql.cpp"
    break;

  case 114: /* join_clause: INNER JOIN ID alias_opt ON condition_list join_clause  */
#line 1062 "yacc_sql.y"
    {
      (yyval.join_clause) = (yyvsp[0].join_clause);
      (yyval.join_clause)->relations.push_back((yyvsp[-4].string));
      (yyval.join_clause)->aliases.push_back((yyvsp[-3].string) ? (yyvsp[-3].string) : "");
      if ((yyvsp[-1].condition_list) != nullptr) {
        std::reverse((yyvsp[-1].condition_list)->begin(), (yyvsp[-1].condition_list)->end());
        for (auto &c : *(yyvsp[-1].condition_list)) {
          (yyval.join_clause)->conditions.emplace_back(std::move(c));
        }
        delete (yyvsp[-1].condition_list);
      }
      free((yyvsp[-4].string));
      if ((yyvsp[-3].string)) free((yyvsp[-3].string));
    }
#line 3020 "yacc_sql.cpp"
    break;

  case 115: /* where: %empty  */
#line 1079 "yacc_sql.y"
    {
      (yyval.condition_list) = nullptr;
    }
#line 3028 "yacc_sql.cpp"
    break;

  case 116: /* where: WHERE condition_list  */
#line 1082 "yacc_sql.y"
                           {
      (yyval.condition_list) = (yyvsp[0].condition_list);
    }
#line 3036 "yacc_sql.cpp"
    break;

  case 117: /* group_by_clause: %empty  */
#line 1088 "yacc_sql.y"
    {
      (yyval.rel_attr_list) = nullptr;
    }
#line 3044 "yacc_sql.cpp"
    break;

  case 118: /* group_by_clause: GROUP BY group_by_list  */
#line 1092 "yacc_sql.y"
    {
      (yyval.rel_attr_list) = (yyvsp[0].rel_attr_list);
    }
#line 3052 "yacc_sql.cpp"
    break;

  case 119: /* group_by_list: rel_attr  */
#line 1098 "yacc_sql.y"
    {
      (yyval.rel_attr_list) = new std::vector<RelAttrSqlNode>;
      (yyval.rel_attr_list)->emplace_back(*(yyvsp[0].rel_attr));
      delete (yyvsp[0].rel_attr);
    }
#line 3062 "yacc_sql.cpp"
    break;

  case 120: /* group_by_list: rel_attr COMMA group_by_list  */
#line 1104 "yacc_sql.y"
    {
      if ((yyvsp[0].rel_attr_list) != nullptr) {
        (yyval.rel_attr_list) = (yyvsp[0].rel_attr_list);
      } else {
        (yyval.rel_attr_list) = new std::vector<RelAttrSqlNode>;
      }
      (yyval.rel_attr_list)->emplace_back(*(yyvsp[-2].rel_attr));
      delete (yyvsp[-2].rel_attr);
    }
#line 3076 "yacc_sql.cpp"
    break;

  case 121: /* having_clause: %empty  */
#line 1116 "yacc_sql.y"
    {
      (yyval.condition_list) = nullptr;
    }
#line 3084 "yacc_sql.cpp"
    break;

  case 122: /* having_clause: HAVING condition_list  */
#line 1120 "yacc_sql.y"
    {
      (yyval.condition_list) = (yyvsp[0].condition_list);
    }
#line 3092 "yacc_sql.cpp"
    break;

  case 123: /* condition_list: %empty  */
#line 1126 "yacc_sql.y"
    {
      (yyval.condition_list) = nullptr;
    }
#line 3100 "yacc_sql.cpp"
    break;

  case 124: /* condition_list: condition  */
#line 1129 "yacc_sql.y"
                {
      (yyval.condition_list) = new std::vector<ConditionSqlNode>;
      (yyval.condition_list)->emplace_back(std::move(*(yyvsp[0].condition)));
      delete (yyvsp[0].condition);
    }
#line 3110 "yacc_sql.cpp"
    break;

  case 125: /* condition_list: condition AND condition_list  */
#line 1134 "yacc_sql.y"
                                   {
      (yyval.condition_list) = (yyvsp[0].condition_list);
      (yyval.condition_list)->emplace_back(std::move(*(yyvsp[-2].condition)));
      delete (yyvsp[-2].condition);
    }
#line 3120 "yacc_sql.cpp"
    break;

  case 126: /* condition: expression LIKE value  */
#line 1142 "yacc_sql.y"
    {
      (yyval.condition) = new ConditionSqlNode;
      (yyval.condition)->left_expr = (yyvsp[-2].expression);
      (yyval.condition)->right_expr = new ValueExpr(*(yyvsp[0].value));
      (yyval.condition)->comp = LIKE_OP;
      delete (yyvsp[0].value);
    }
#line 3132 "yacc_sql.cpp"
    break;

  case 127: /* condition: expression NOT LIKE value  */
#line 1150 "yacc_sql.y"
    {
      (yyval.condition) = new ConditionSqlNode;
      (yyval.condition)->left_expr = (yyvsp[-3].expression);
      (yyval.condition)->right_expr = new ValueExpr(*(yyvsp[0].value));
      (yyval.condition)->comp = NOT_LIKE;
      delete (yyvsp[0].value);
    }
#line 3144 "yacc_sql.cpp"
    break;

  case 128: /* condition: expression IN LBRACE value value_list RBRACE  */
#line 1158 "yacc_sql.y"
    {
      std::vector<Value> values;
      if ((yyvsp[-1].value_list) != nullptr) {
        values.swap(*(yyvsp[-1].value_list));
        delete (yyvsp[-1].value_list);
      }
      values.emplace_back(*(yyvsp[-2].value));
      std::reverse(values.begin(), values.end());
      (yyval.condition) = new ConditionSqlNode;
      (yyval.condition)->left_expr = (yyvsp[-5].expression);
      (yyval.condition)->right_expr = new InListExpr(std::move(values));
      (yyval.condition)->comp = IN_OP;
      delete (yyvsp[-2].value);
    }
#line 3163 "yacc_sql.cpp"
    break;

  case 129: /* condition: expression NOT IN LBRACE value value_list RBRACE  */
#line 1173 "yacc_sql.y"
    {
      std::vector<Value> values;
      if ((yyvsp[-1].value_list) != nullptr) {
        values.swap(*(yyvsp[-1].value_list));
        delete (yyvsp[-1].value_list);
      }
      values.emplace_back(*(yyvsp[-2].value));
      std::reverse(values.begin(), values.end());
      (yyval.condition) = new ConditionSqlNode;
      (yyval.condition)->left_expr = (yyvsp[-6].expression);
      (yyval.condition)->right_expr = new InListExpr(std::move(values));
      (yyval.condition)->comp = NOT_IN;
      delete (yyvsp[-2].value);
    }
#line 3182 "yacc_sql.cpp"
    break;

  case 130: /* condition: expression IN LBRACE select_stmt RBRACE  */
#line 1188 "yacc_sql.y"
    {
      auto *sel = new SelectSqlNode;
      sel->attributes.swap((yyvsp[-1].sql_node)->selection.attributes);
      sel->expressions.swap((yyvsp[-1].sql_node)->selection.expressions);
      sel->relations.swap((yyvsp[-1].sql_node)->selection.relations);
      sel->aliases.swap((yyvsp[-1].sql_node)->selection.aliases);
      sel->conditions.swap((yyvsp[-1].sql_node)->selection.conditions);
      sel->order_by.swap((yyvsp[-1].sql_node)->selection.order_by);
      (yyval.condition) = new ConditionSqlNode;
      (yyval.condition)->left_expr = (yyvsp[-4].expression);
      (yyval.condition)->right_expr = new SubQueryExpr(sel);
      (yyval.condition)->comp = IN_OP;
      delete (yyvsp[-1].sql_node);
    }
#line 3201 "yacc_sql.cpp"
    break;

  case 131: /* condition: expression NOT IN LBRACE select_stmt RBRACE  */
#line 1203 "yacc_sql.y"
    {
      auto *sel = new SelectSqlNode;
      sel->attributes.swap((yyvsp[-1].sql_node)->selection.attributes);
      sel->expressions.swap((yyvsp[-1].sql_node)->selection.expressions);
      sel->relations.swap((yyvsp[-1].sql_node)->selection.relations);
      sel->aliases.swap((yyvsp[-1].sql_node)->selection.aliases);
      sel->conditions.swap((yyvsp[-1].sql_node)->selection.conditions);
      sel->order_by.swap((yyvsp[-1].sql_node)->selection.order_by);
      (yyval.condition) = new ConditionSqlNode;
      (yyval.condition)->left_expr = (yyvsp[-5].expression);
      (yyval.condition)->right_expr = new SubQueryExpr(sel);
      (yyval.condition)->comp = NOT_IN;
      delete (yyvsp[-1].sql_node);
    }
#line 3220 "yacc_sql.cpp"
    break;

  case 132: /* condition: EXISTS LBRACE select_stmt RBRACE  */
#line 1218 "yacc_sql.y"
    {
      auto *sel = new SelectSqlNode;
      sel->attributes.swap((yyvsp[-1].sql_node)->selection.attributes);
      sel->expressions.swap((yyvsp[-1].sql_node)->selection.expressions);
      sel->relations.swap((yyvsp[-1].sql_node)->selection.relations);
      sel->aliases.swap((yyvsp[-1].sql_node)->selection.aliases);
      sel->conditions.swap((yyvsp[-1].sql_node)->selection.conditions);
      sel->order_by.swap((yyvsp[-1].sql_node)->selection.order_by);
      (yyval.condition) = new ConditionSqlNode;
      (yyval.condition)->right_expr = new SubQueryExpr(sel);
      (yyval.condition)->comp = EXISTS_OP;
      delete (yyvsp[-1].sql_node);
    }
#line 3238 "yacc_sql.cpp"
    break;

  case 133: /* condition: NOT EXISTS LBRACE select_stmt RBRACE  */
#line 1232 "yacc_sql.y"
    {
      auto *sel = new SelectSqlNode;
      sel->attributes.swap((yyvsp[-1].sql_node)->selection.attributes);
      sel->expressions.swap((yyvsp[-1].sql_node)->selection.expressions);
      sel->relations.swap((yyvsp[-1].sql_node)->selection.relations);
      sel->aliases.swap((yyvsp[-1].sql_node)->selection.aliases);
      sel->conditions.swap((yyvsp[-1].sql_node)->selection.conditions);
      sel->order_by.swap((yyvsp[-1].sql_node)->selection.order_by);
      (yyval.condition) = new ConditionSqlNode;
      (yyval.condition)->right_expr = new SubQueryExpr(sel);
      (yyval.condition)->comp = NOT_EXISTS;
      delete (yyvsp[-1].sql_node);
    }
#line 3256 "yacc_sql.cpp"
    break;

  case 134: /* condition: expression IS NULL_T  */
#line 1246 "yacc_sql.y"
    {
      Value null_value;
      null_value.set_null(true);
      (yyval.condition) = new ConditionSqlNode;
      (yyval.condition)->left_expr = (yyvsp[-2].expression);
      (yyval.condition)->right_expr = new ValueExpr(null_value);
      (yyval.condition)->comp = IS_NULL;
    }
#line 3269 "yacc_sql.cpp"
    break;

  case 135: /* condition: expression IS NOT NULL_T  */
#line 1255 "yacc_sql.y"
    {
      Value null_value;
      null_value.set_null(true);
      (yyval.condition) = new ConditionSqlNode;
      (yyval.condition)->left_expr = (yyvsp[-3].expression);
      (yyval.condition)->right_expr = new ValueExpr(null_value);
      (yyval.condition)->comp = IS_NOT_NULL;
    }
#line 3282 "yacc_sql.cpp"
    break;

  case 136: /* condition: expression comp_op expression  */
#line 1264 "yacc_sql.y"
    {
      (yyval.condition) = new ConditionSqlNode;
      (yyval.condition)->left_expr = (yyvsp[-2].expression);
      (yyval.condition)->right_expr = (yyvsp[0].expression);
      (yyval.condition)->comp = (yyvsp[-1].comp);
    }
#line 3293 "yacc_sql.cpp"
    break;

  case 137: /* condition: agg_expr comp_op expression  */
#line 1271 "yacc_sql.y"
    {
      (yyval.condition) = new ConditionSqlNode;
      (yyval.condition)->left_expr = new AggregationExpr((yyvsp[-2].rel_attr)->aggregation_type, (yyvsp[-2].rel_attr)->relation_name, (yyvsp[-2].rel_attr)->attribute_name);
      delete (yyvsp[-2].rel_attr);
      (yyval.condition)->comp = (yyvsp[-1].comp);
      (yyval.condition)->right_expr = (yyvsp[0].expression);
    }
#line 3305 "yacc_sql.cpp"
    break;

  case 138: /* comp_op: EQ  */
#line 1281 "yacc_sql.y"
         { (yyval.comp) = EQUAL_TO; }
#line 3311 "yacc_sql.cpp"
    break;

  case 139: /* comp_op: LT  */
#line 1282 "yacc_sql.y"
         { (yyval.comp) = LESS_THAN; }
#line 3317 "yacc_sql.cpp"
    break;

  case 140: /* comp_op: GT  */
#line 1283 "yacc_sql.y"
         { (yyval.comp) = GREAT_THAN; }
#line 3323 "yacc_sql.cpp"
    break;

  case 141: /* comp_op: LE  */
#line 1284 "yacc_sql.y"
         { (yyval.comp) = LESS_EQUAL; }
#line 3329 "yacc_sql.cpp"
    break;

  case 142: /* comp_op: GE  */
#line 1285 "yacc_sql.y"
         { (yyval.comp) = GREAT_EQUAL; }
#line 3335 "yacc_sql.cpp"
    break;

  case 143: /* comp_op: NE  */
#line 1286 "yacc_sql.y"
         { (yyval.comp) = NOT_EQUAL; }
#line 3341 "yacc_sql.cpp"
    break;

  case 144: /* load_data_stmt: LOAD DATA INFILE SSS INTO TABLE ID  */
#line 1291 "yacc_sql.y"
    {
      char *tmp_file_name = common::substr((yyvsp[-3].string), 1, strlen((yyvsp[-3].string)) - 2);
      
      (yyval.sql_node) = new ParsedSqlNode(SCF_LOAD_DATA);
      (yyval.sql_node)->load_data.relation_name = (yyvsp[0].string);
      (yyval.sql_node)->load_data.file_name = tmp_file_name;
      free((yyvsp[0].string));
      free(tmp_file_name);
    }
#line 3355 "yacc_sql.cpp"
    break;

  case 145: /* explain_stmt: EXPLAIN command_wrapper  */
#line 1304 "yacc_sql.y"
    {
      (yyval.sql_node) = new ParsedSqlNode(SCF_EXPLAIN);
      (yyval.sql_node)->explain.sql_node = std::unique_ptr<ParsedSqlNode>((yyvsp[0].sql_node));
    }
#line 3364 "yacc_sql.cpp"
    break;

  case 146: /* set_variable_stmt: SET ID EQ value  */
#line 1312 "yacc_sql.y"
    {
      (yyval.sql_node) = new ParsedSqlNode(SCF_SET_VARIABLE);
      (yyval.sql_node)->set_variable.name  = (yyvsp[-2].string);
      (yyval.sql_node)->set_variable.value = *(yyvsp[0].value);
      free((yyvsp[-2].string));
      delete (yyvsp[0].value);
    }
#line 3376 "yacc_sql.cpp"
    break;

  case 147: /* order_by_clause: %empty  */
#line 1323 "yacc_sql.y"
    {
      (yyval.order_by_list) = nullptr;
    }
#line 3384 "yacc_sql.cpp"
    break;

  case 148: /* order_by_clause: ORDER BY order_by_list  */
#line 1327 "yacc_sql.y"
    {
      (yyval.order_by_list) = (yyvsp[0].order_by_list);
    }
#line 3392 "yacc_sql.cpp"
    break;

  case 149: /* order_by_list: order_by_item  */
#line 1334 "yacc_sql.y"
    {
      (yyval.order_by_list) = new std::vector<OrderByNode>;
      (yyval.order_by_list)->emplace_back(*(yyvsp[0].order_by_node));
      delete (yyvsp[0].order_by_node);
    }
#line 3402 "yacc_sql.cpp"
    break;

  case 150: /* order_by_list: order_by_item COMMA order_by_list  */
#line 1340 "yacc_sql.y"
    {
      if ((yyvsp[0].order_by_list) != nullptr) {
        (yyval.order_by_list) = (yyvsp[0].order_by_list);
      } else {
        (yyval.order_by_list) = new std::vector<OrderByNode>;
      }
      (yyval.order_by_list)->emplace_back(*(yyvsp[-2].order_by_node));
      delete (yyvsp[-2].order_by_node);
    }
#line 3416 "yacc_sql.cpp"
    break;

  case 151: /* order_by_item: rel_attr opt_asc_desc  */
#line 1353 "yacc_sql.y"
    {
      (yyval.order_by_node) = new OrderByNode;
      (yyval.order_by_node)->relation_name = (yyvsp[-1].rel_attr)->relation_name;
      (yyval.order_by_node)->attribute_name = (yyvsp[-1].rel_attr)->attribute_name;
      (yyval.order_by_node)->is_asc = (yyvsp[0].boolean);
      delete (yyvsp[-1].rel_attr);
    }
#line 3428 "yacc_sql.cpp"
    break;

  case 152: /* opt_asc_desc: %empty  */
#line 1364 "yacc_sql.y"
    {
      (yyval.boolean) = true;
    }
#line 3436 "yacc_sql.cpp"
    break;

  case 153: /* opt_asc_desc: ASC  */
#line 1368 "yacc_sql.y"
    {
      (yyval.boolean) = true;
    }
#line 3444 "yacc_sql.cpp"
    break;

  case 154: /* opt_asc_desc: DESC  */
#line 1372 "yacc_sql.y"
    {
      (yyval.boolean) = false;
    }
#line 3452 "yacc_sql.cpp"
    break;


#line 3456 "yacc_sql.cpp"

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
  *++yylsp = yyloc;

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
      {
        yypcontext_t yyctx
          = {yyssp, yytoken, &yylloc};
        char const *yymsgp = YY_("syntax error");
        int yysyntax_error_status;
        yysyntax_error_status = yysyntax_error (&yymsg_alloc, &yymsg, &yyctx);
        if (yysyntax_error_status == 0)
          yymsgp = yymsg;
        else if (yysyntax_error_status == -1)
          {
            if (yymsg != yymsgbuf)
              YYSTACK_FREE (yymsg);
            yymsg = YY_CAST (char *,
                             YYSTACK_ALLOC (YY_CAST (YYSIZE_T, yymsg_alloc)));
            if (yymsg)
              {
                yysyntax_error_status
                  = yysyntax_error (&yymsg_alloc, &yymsg, &yyctx);
                yymsgp = yymsg;
              }
            else
              {
                yymsg = yymsgbuf;
                yymsg_alloc = sizeof yymsgbuf;
                yysyntax_error_status = YYENOMEM;
              }
          }
        yyerror (&yylloc, sql_string, sql_result, scanner, yymsgp);
        if (yysyntax_error_status == YYENOMEM)
          YYNOMEM;
      }
    }

  yyerror_range[1] = yylloc;
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
                      yytoken, &yylval, &yylloc, sql_string, sql_result, scanner);
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

      yyerror_range[1] = *yylsp;
      yydestruct ("Error: popping",
                  YY_ACCESSING_SYMBOL (yystate), yyvsp, yylsp, sql_string, sql_result, scanner);
      YYPOPSTACK (1);
      yystate = *yyssp;
      YY_STACK_PRINT (yyss, yyssp);
    }

  YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
  *++yyvsp = yylval;
  YY_IGNORE_MAYBE_UNINITIALIZED_END

  yyerror_range[2] = yylloc;
  ++yylsp;
  YYLLOC_DEFAULT (*yylsp, yyerror_range, 2);

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
  yyerror (&yylloc, sql_string, sql_result, scanner, YY_("memory exhausted"));
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
                  yytoken, &yylval, &yylloc, sql_string, sql_result, scanner);
    }
  /* Do not reclaim the symbols of the rule whose action triggered
     this YYABORT or YYACCEPT.  */
  YYPOPSTACK (yylen);
  YY_STACK_PRINT (yyss, yyssp);
  while (yyssp != yyss)
    {
      yydestruct ("Cleanup: popping",
                  YY_ACCESSING_SYMBOL (+*yyssp), yyvsp, yylsp, sql_string, sql_result, scanner);
      YYPOPSTACK (1);
    }
#ifndef yyoverflow
  if (yyss != yyssa)
    YYSTACK_FREE (yyss);
#endif
  if (yymsg != yymsgbuf)
    YYSTACK_FREE (yymsg);
  return yyresult;
}

#line 1380 "yacc_sql.y"

//_____________________________________________________________________
extern void scan_string(const char *str, yyscan_t scanner);

int sql_parse(const char *s, ParsedSqlResult *sql_result) {
  yyscan_t scanner;
  yylex_init(&scanner);
  scan_string(s, scanner);
  int result = yyparse(s, sql_result, scanner);
  yylex_destroy(scanner);
  return result;
}

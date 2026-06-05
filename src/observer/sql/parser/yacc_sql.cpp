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
  YYSYMBOL_OR = 40,                        /* OR  */
  YYSYMBOL_VIEW = 41,                      /* VIEW  */
  YYSYMBOL_LIKE = 42,                      /* LIKE  */
  YYSYMBOL_NOT = 43,                       /* NOT  */
  YYSYMBOL_SET = 44,                       /* SET  */
  YYSYMBOL_INNER = 45,                     /* INNER  */
  YYSYMBOL_JOIN = 46,                      /* JOIN  */
  YYSYMBOL_ON = 47,                        /* ON  */
  YYSYMBOL_LOAD = 48,                      /* LOAD  */
  YYSYMBOL_DATA = 49,                      /* DATA  */
  YYSYMBOL_INFILE = 50,                    /* INFILE  */
  YYSYMBOL_EXPLAIN = 51,                   /* EXPLAIN  */
  YYSYMBOL_ORDER = 52,                     /* ORDER  */
  YYSYMBOL_BY = 53,                        /* BY  */
  YYSYMBOL_ASC = 54,                       /* ASC  */
  YYSYMBOL_AS = 55,                        /* AS  */
  YYSYMBOL_GROUP = 56,                     /* GROUP  */
  YYSYMBOL_HAVING = 57,                    /* HAVING  */
  YYSYMBOL_EQ = 58,                        /* EQ  */
  YYSYMBOL_LT = 59,                        /* LT  */
  YYSYMBOL_GT = 60,                        /* GT  */
  YYSYMBOL_LE = 61,                        /* LE  */
  YYSYMBOL_GE = 62,                        /* GE  */
  YYSYMBOL_NE = 63,                        /* NE  */
  YYSYMBOL_IN = 64,                        /* IN  */
  YYSYMBOL_EXISTS = 65,                    /* EXISTS  */
  YYSYMBOL_UNIQUE = 66,                    /* UNIQUE  */
  YYSYMBOL_NULL_T = 67,                    /* NULL_T  */
  YYSYMBOL_NULLABLE_T = 68,                /* NULLABLE_T  */
  YYSYMBOL_TEXT_T = 69,                    /* TEXT_T  */
  YYSYMBOL_IS = 70,                        /* IS  */
  YYSYMBOL_NUMBER = 71,                    /* NUMBER  */
  YYSYMBOL_FLOAT = 72,                     /* FLOAT  */
  YYSYMBOL_ID = 73,                        /* ID  */
  YYSYMBOL_SSS = 74,                       /* SSS  */
  YYSYMBOL_75_ = 75,                       /* '+'  */
  YYSYMBOL_76_ = 76,                       /* '-'  */
  YYSYMBOL_77_ = 77,                       /* '*'  */
  YYSYMBOL_78_ = 78,                       /* '/'  */
  YYSYMBOL_UMINUS = 79,                    /* UMINUS  */
  YYSYMBOL_YYACCEPT = 80,                  /* $accept  */
  YYSYMBOL_commands = 81,                  /* commands  */
  YYSYMBOL_command_wrapper = 82,           /* command_wrapper  */
  YYSYMBOL_exit_stmt = 83,                 /* exit_stmt  */
  YYSYMBOL_help_stmt = 84,                 /* help_stmt  */
  YYSYMBOL_sync_stmt = 85,                 /* sync_stmt  */
  YYSYMBOL_begin_stmt = 86,                /* begin_stmt  */
  YYSYMBOL_commit_stmt = 87,               /* commit_stmt  */
  YYSYMBOL_rollback_stmt = 88,             /* rollback_stmt  */
  YYSYMBOL_drop_table_stmt = 89,           /* drop_table_stmt  */
  YYSYMBOL_show_tables_stmt = 90,          /* show_tables_stmt  */
  YYSYMBOL_desc_table_stmt = 91,           /* desc_table_stmt  */
  YYSYMBOL_create_index_stmt = 92,         /* create_index_stmt  */
  YYSYMBOL_drop_index_stmt = 93,           /* drop_index_stmt  */
  YYSYMBOL_create_table_stmt = 94,         /* create_table_stmt  */
  YYSYMBOL_create_view_stmt = 95,          /* create_view_stmt  */
  YYSYMBOL_attr_def_list = 96,             /* attr_def_list  */
  YYSYMBOL_attr_def = 97,                  /* attr_def  */
  YYSYMBOL_number = 98,                    /* number  */
  YYSYMBOL_type = 99,                      /* type  */
  YYSYMBOL_insert_stmt = 100,              /* insert_stmt  */
  YYSYMBOL_value_list = 101,               /* value_list  */
  YYSYMBOL_id_list = 102,                  /* id_list  */
  YYSYMBOL_value = 103,                    /* value  */
  YYSYMBOL_delete_stmt = 104,              /* delete_stmt  */
  YYSYMBOL_update_stmt = 105,              /* update_stmt  */
  YYSYMBOL_update_assign_list = 106,       /* update_assign_list  */
  YYSYMBOL_update_assign = 107,            /* update_assign  */
  YYSYMBOL_alias_opt = 108,                /* alias_opt  */
  YYSYMBOL_select_stmt = 109,              /* select_stmt  */
  YYSYMBOL_calc_stmt = 110,                /* calc_stmt  */
  YYSYMBOL_expression_list = 111,          /* expression_list  */
  YYSYMBOL_expression = 112,               /* expression  */
  YYSYMBOL_agg_expr = 113,                 /* agg_expr  */
  YYSYMBOL_select_attr = 114,              /* select_attr  */
  YYSYMBOL_select_expr_item = 115,         /* select_expr_item  */
  YYSYMBOL_select_expr_list = 116,         /* select_expr_list  */
  YYSYMBOL_rel_attr = 117,                 /* rel_attr  */
  YYSYMBOL_join_clause = 118,              /* join_clause  */
  YYSYMBOL_where = 119,                    /* where  */
  YYSYMBOL_group_by_clause = 120,          /* group_by_clause  */
  YYSYMBOL_group_by_list = 121,            /* group_by_list  */
  YYSYMBOL_having_clause = 122,            /* having_clause  */
  YYSYMBOL_condition_list = 123,           /* condition_list  */
  YYSYMBOL_condition = 124,                /* condition  */
  YYSYMBOL_comp_op = 125,                  /* comp_op  */
  YYSYMBOL_load_data_stmt = 126,           /* load_data_stmt  */
  YYSYMBOL_explain_stmt = 127,             /* explain_stmt  */
  YYSYMBOL_set_variable_stmt = 128,        /* set_variable_stmt  */
  YYSYMBOL_order_by_clause = 129,          /* order_by_clause  */
  YYSYMBOL_order_by_list = 130,            /* order_by_list  */
  YYSYMBOL_order_by_item = 131,            /* order_by_item  */
  YYSYMBOL_opt_asc_desc = 132,             /* opt_asc_desc  */
  YYSYMBOL_opt_semicolon = 133             /* opt_semicolon  */
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
#define YYFINAL  79
/* YYLAST -- Last index in YYTABLE.  */
#define YYLAST   420

/* YYNTOKENS -- Number of terminals.  */
#define YYNTOKENS  80
/* YYNNTS -- Number of nonterminals.  */
#define YYNNTS  54
/* YYNRULES -- Number of rules.  */
#define YYNRULES  160
/* YYNSTATES -- Number of states.  */
#define YYNSTATES  345

/* YYMAXUTOK -- Last valid token kind.  */
#define YYMAXUTOK   330


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
       2,     2,    77,    75,     2,    76,     2,    78,     2,     2,
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
      79
};

#if YYDEBUG
/* YYRLINE[YYN] -- Source line where rule number YYN was defined.  */
static const yytype_int16 yyrline[] =
{
       0,   242,   242,   250,   251,   252,   253,   254,   255,   256,
     257,   258,   259,   260,   261,   262,   263,   264,   265,   266,
     267,   268,   269,   270,   274,   280,   285,   291,   297,   303,
     309,   316,   322,   330,   348,   368,   378,   394,   407,   418,
     421,   434,   442,   451,   460,   469,   478,   486,   495,   504,
     513,   524,   527,   528,   529,   530,   531,   534,   550,   553,
     565,   568,   580,   584,   588,   592,   596,   609,   617,   629,
     649,   652,   664,   673,   692,   693,   694,   698,   728,   777,
     827,   837,   842,   853,   856,   859,   862,   865,   869,   872,
     884,   897,   902,   907,   913,   921,   926,   931,   936,   942,
     947,   955,   967,   974,   992,  1002,  1008,  1017,  1022,  1029,
    1033,  1037,  1041,  1046,  1050,  1075,  1078,  1086,  1104,  1107,
    1113,  1116,  1122,  1128,  1141,  1144,  1151,  1154,  1159,  1164,
    1178,  1186,  1194,  1209,  1224,  1239,  1254,  1268,  1282,  1291,
    1300,  1307,  1318,  1319,  1320,  1321,  1322,  1323,  1327,  1340,
    1348,  1360,  1363,  1370,  1376,  1389,  1401,  1404,  1408,  1414,
    1415
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
  "COUNT", "AVG", "SUM", "FROM", "WHERE", "AND", "OR", "VIEW", "LIKE",
  "NOT", "SET", "INNER", "JOIN", "ON", "LOAD", "DATA", "INFILE", "EXPLAIN",
  "ORDER", "BY", "ASC", "AS", "GROUP", "HAVING", "EQ", "LT", "GT", "LE",
  "GE", "NE", "IN", "EXISTS", "UNIQUE", "NULL_T", "NULLABLE_T", "TEXT_T",
  "IS", "NUMBER", "FLOAT", "ID", "SSS", "'+'", "'-'", "'*'", "'/'",
  "UMINUS", "$accept", "commands", "command_wrapper", "exit_stmt",
  "help_stmt", "sync_stmt", "begin_stmt", "commit_stmt", "rollback_stmt",
  "drop_table_stmt", "show_tables_stmt", "desc_table_stmt",
  "create_index_stmt", "drop_index_stmt", "create_table_stmt",
  "create_view_stmt", "attr_def_list", "attr_def", "number", "type",
  "insert_stmt", "value_list", "id_list", "value", "delete_stmt",
  "update_stmt", "update_assign_list", "update_assign", "alias_opt",
  "select_stmt", "calc_stmt", "expression_list", "expression", "agg_expr",
  "select_attr", "select_expr_item", "select_expr_list", "rel_attr",
  "join_clause", "where", "group_by_clause", "group_by_list",
  "having_clause", "condition_list", "condition", "comp_op",
  "load_data_stmt", "explain_stmt", "set_variable_stmt", "order_by_clause",
  "order_by_list", "order_by_item", "opt_asc_desc", "opt_semicolon", YY_NULLPTR
};

static const char *
yysymbol_name (yysymbol_kind_t yysymbol)
{
  return yytname[yysymbol];
}
#endif

#define YYPACT_NINF (-265)

#define yypact_value_is_default(Yyn) \
  ((Yyn) == YYPACT_NINF)

#define YYTABLE_NINF (-95)

#define yytable_value_is_error(Yyn) \
  0

/* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
   STATE-NUM.  */
static const yytype_int16 yypact[] =
{
     294,   117,    70,   200,   104,   -15,    36,  -265,    35,    47,
      30,  -265,  -265,  -265,  -265,  -265,    37,    67,   294,   124,
     123,  -265,  -265,  -265,  -265,  -265,  -265,  -265,  -265,  -265,
    -265,  -265,  -265,  -265,  -265,  -265,  -265,  -265,  -265,  -265,
    -265,  -265,    55,    57,    62,   133,    74,    86,    28,   126,
     135,   144,   156,   162,  -265,  -265,  -265,    15,  -265,   213,
    -265,  -265,    79,  -265,    88,  -265,   258,   273,   113,  -265,
      12,  -265,  -265,   109,   121,   142,   137,   147,  -265,  -265,
    -265,  -265,     2,   154,   148,   132,  -265,   160,   191,     5,
     227,   227,   131,   227,   227,   200,   143,  -265,  -265,  -265,
     200,   200,   200,   200,   200,   -36,   145,  -265,  -265,  -265,
     146,   221,   150,   155,   159,   179,   183,   152,   309,   163,
     166,   232,   170,   232,   197,   178,  -265,  -265,   235,   241,
     247,   248,   249,   239,   251,   252,   257,   259,   260,   261,
     263,  -265,  -265,   -42,   -42,  -265,  -265,  -265,  -265,   -34,
    -265,   -34,   217,   266,   297,   328,  -265,    80,   237,   234,
     279,  -265,   243,   283,   -20,  -265,   281,     0,   293,  -265,
     301,  -265,   246,  -265,   227,   227,   291,   227,   227,   256,
    -265,  -265,  -265,  -265,  -265,  -265,  -265,     8,     8,   303,
     232,   309,    -9,  -265,  -265,  -265,  -265,  -265,  -265,   313,
     -13,   200,   200,   155,   155,   227,   155,   262,   309,    77,
     152,   183,  -265,  -265,   331,  -265,  -265,  -265,  -265,  -265,
      -1,   166,   323,   270,   327,   329,   334,   335,   336,   344,
     345,  -265,   292,   320,   183,   183,   232,   351,  -265,   309,
     353,     1,   304,  -265,   317,   317,  -265,  -265,   358,  -265,
    -265,   325,  -265,   360,   232,  -265,   283,  -265,   311,   326,
      66,  -265,  -265,   293,  -265,   363,   330,  -265,  -265,  -265,
    -265,  -265,  -265,   -34,   332,   159,   159,   378,  -265,  -265,
       1,   360,   380,  -265,   227,   227,   309,   381,   382,  -265,
    -265,  -265,   383,  -265,  -265,  -265,   333,   384,   363,     8,
     -34,   234,   234,  -265,   360,   386,   389,  -265,  -265,    17,
    -265,   390,   360,  -265,  -265,   -21,   363,  -265,   392,  -265,
     361,   262,   262,   393,  -265,  -265,  -265,  -265,  -265,   227,
    -265,    78,  -265,  -265,  -265,  -265,   155,  -265,  -265,  -265,
    -265,  -265,  -265,     8,  -265
};

/* YYDEFACT[STATE-NUM] -- Default reduction number in state STATE-NUM.
   Performed when YYTABLE does not specify something else to do.  Zero
   means the default is an error.  */
static const yytype_uint8 yydefact[] =
{
       0,     0,     0,     0,     0,     0,     0,    26,     0,     0,
       0,    27,    28,    29,    25,    24,     0,     0,     0,     0,
     159,    23,    22,    15,    16,    17,    18,    10,    11,    12,
      13,    14,     8,     9,     5,     7,     6,     4,     3,    19,
      20,    21,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    67,    62,    64,    92,    66,     0,
      91,    80,    81,    94,    92,   101,    74,    74,     0,   105,
     118,    32,    31,     0,     0,     0,     0,     0,   149,     1,
     160,     2,     0,     0,     0,     0,    30,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    62,    64,    88,
       0,     0,     0,     0,     0,     0,     0,    75,   102,   103,
       0,     0,     0,   126,   120,     0,   118,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    89,    87,     0,     0,
       0,     0,     0,   107,     0,     0,     0,     0,     0,     0,
       0,    93,    82,    83,    84,    85,    86,   104,    76,    74,
     106,    74,     0,     0,     0,    94,   119,   127,     0,   124,
       0,    68,     0,    70,     0,   150,     0,     0,    39,    37,
       0,    38,     0,    35,     0,     0,     0,     0,     0,     0,
      95,    96,    98,    97,    99,   100,    90,   115,   115,     0,
       0,     0,     0,   142,   143,   144,   145,   146,   147,     0,
       0,     0,     0,   126,   126,     0,   126,   151,     0,     0,
       0,   118,    63,    65,     0,    55,    52,    53,    54,    56,
      46,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   108,     0,     0,   118,   118,     0,     0,   130,     0,
       0,     0,     0,   138,   140,   141,   128,   129,   122,   121,
     125,     0,    77,    58,     0,    72,    70,    69,     0,     0,
       0,    47,    49,    39,    36,    60,     0,   109,   110,   112,
     111,   113,   114,    74,     0,   120,   120,     0,   136,   131,
       0,    58,     0,   139,     0,     0,     0,     0,     0,    71,
     148,    51,     0,    48,    50,    40,     0,     0,    60,   115,
      74,   124,   124,   137,    58,     0,     0,   134,   123,   156,
     152,   153,    58,    57,    73,    41,    60,    34,     0,   116,
       0,   151,   151,     0,   135,   132,   158,   157,   155,     0,
      59,     0,    42,    44,    61,    33,   126,    79,    78,   133,
     154,    43,    45,   115,   117
};

/* YYPGOTO[NTERM-NUM].  */
static const yytype_int16 yypgoto[] =
{
    -265,  -265,   394,  -265,  -265,  -265,  -265,  -265,  -265,  -265,
    -265,  -265,  -265,  -265,  -265,  -265,   151,   192,  -265,  -265,
    -265,  -264,  -242,  -112,  -265,  -265,   161,   205,   -59,   -30,
    -265,   -66,    11,    -4,  -265,   305,  -265,   -89,  -181,  -103,
    -106,   134,  -117,  -194,  -265,   264,  -265,  -265,  -265,  -129,
      91,  -265,  -265,  -265
};

/* YYDEFGOTO[NTERM-NUM].  */
static const yytype_int16 yydefgoto[] =
{
       0,    19,    20,    21,    22,    23,    24,    25,    26,    27,
      28,    29,    30,    31,    32,    33,   222,   168,   292,   220,
      34,   287,   297,    60,    35,    36,   211,   163,   108,    37,
      38,    61,   154,    63,    68,    69,    70,   248,   234,   114,
     159,   249,   207,   156,   157,   201,    39,    40,    41,   252,
     310,   311,   328,    81
};

/* YYTABLE[YYPACT[STATE-NUM]] -- What to do in state STATE-NUM.  If
   positive, shift that token.  If negative, reduce the rule whose
   number is the opposite.  If YYTABLE_NINF, syntax error.  */
static const yytype_int16 yytable[] =
{
      67,   134,   135,   137,   138,   139,   165,   235,   109,   246,
     247,     4,   250,   161,    62,    66,   259,   306,    88,   120,
     215,   106,   331,   127,   216,   217,   218,   232,   326,   140,
     242,   111,    95,   239,   142,   103,   104,   141,     4,   107,
     323,   147,   260,    72,    96,    48,   332,   333,   330,   112,
     113,   212,   213,   233,   243,   240,   318,   121,    71,    89,
      49,    50,    51,    52,    53,    73,   261,   262,    54,   219,
      99,   327,    55,    56,   334,    58,    46,   164,    47,   238,
     101,   102,   103,   104,    74,   225,   226,   228,   229,   230,
     187,   169,   188,   171,   254,    54,   253,   255,   100,    55,
      56,    57,    58,    75,    59,    95,    62,    67,   257,   155,
      76,    62,   143,   144,   145,   146,    77,   105,   319,   203,
     204,    48,    66,    42,    79,    43,    80,   279,    82,   281,
      83,   275,   276,   293,   294,    84,    49,    50,    51,    52,
      53,    85,   343,    90,    54,   341,   342,    86,    55,    56,
     110,    58,    91,   164,   101,   102,   103,   104,    44,    87,
     237,    92,   344,   128,   129,   130,   131,   132,   304,   301,
     302,    54,    48,    93,   312,    55,    56,    64,    58,    94,
      59,    65,   115,    45,   321,   322,   117,    49,    50,    51,
      52,    53,   337,   338,   116,   118,   309,   119,   152,   155,
     155,   122,   155,   123,   133,   124,   277,   125,   136,   126,
     160,   282,   244,   245,   299,   158,   141,    48,   148,   149,
     153,   113,    54,   151,   288,   162,    55,    56,    57,    58,
      48,    59,    49,    50,    51,    52,    53,   166,    48,   167,
     309,   320,     4,   170,   172,    49,    50,    51,    52,    53,
     305,   173,   174,    49,    50,    51,    52,    53,   175,   128,
     129,   130,   131,   132,   176,   177,   178,    54,   179,   180,
     181,    55,    56,    57,    58,   182,    59,   183,   184,   185,
      54,   186,   189,   190,    97,    98,    57,    58,    54,    59,
     205,   206,    55,    56,    64,    58,   208,    59,     1,     2,
     133,   209,   210,     3,     4,     5,     6,     7,     8,     9,
      10,   214,   221,   106,   251,    11,    12,    13,   223,   224,
     236,    14,    15,   128,   129,   130,   131,   132,   106,   231,
     241,   107,   155,   101,   102,   103,   104,   258,    16,   191,
     192,   264,    17,   265,   266,    18,   107,   267,   -94,   -94,
     -94,   -94,   268,   269,   270,   193,   194,   195,   196,   197,
     198,   199,   271,   272,   133,   273,   274,   200,   227,   278,
     280,   283,   101,   102,   103,   104,    54,   284,   285,   286,
      55,    56,   296,    58,   290,   164,   193,   194,   195,   196,
     197,   198,   101,   102,   103,   104,   303,   291,   307,   313,
     314,   315,   317,   298,   324,   300,   316,   325,   336,   329,
     335,   339,    78,   263,   295,   256,   150,   289,   308,   202,
     340
};

static const yytype_int16 yycheck[] =
{
       4,    90,    91,    92,    93,    94,   118,   188,    67,   203,
     204,    10,   206,   116,     3,     4,    17,   281,    48,    17,
      20,    55,    43,    18,    24,    25,    26,    19,    11,    95,
      43,    19,    17,    42,   100,    77,    78,    73,    10,    73,
     304,    77,    43,     7,    29,    17,    67,    68,   312,    37,
      38,    71,    72,    45,    67,    64,   298,    55,    73,    48,
      32,    33,    34,    35,    36,    30,    67,    68,    67,    69,
      59,    54,    71,    72,   316,    74,     6,    76,     8,   191,
      75,    76,    77,    78,    37,   174,   175,   176,   177,   178,
     149,   121,   151,   123,    17,    67,   208,   209,    19,    71,
      72,    73,    74,    73,    76,    17,    95,   111,   211,   113,
      73,   100,   101,   102,   103,   104,    49,    29,   299,    39,
      40,    17,   111,     6,     0,     8,     3,   239,    73,   241,
      73,   234,   235,    67,    68,    73,    32,    33,    34,    35,
      36,     8,   336,    17,    67,    67,    68,    73,    71,    72,
      37,    74,    17,    76,    75,    76,    77,    78,    41,    73,
     190,    17,   343,    32,    33,    34,    35,    36,   280,   275,
     276,    67,    17,    17,   286,    71,    72,    73,    74,    17,
      76,    77,    73,    66,   301,   302,    44,    32,    33,    34,
      35,    36,   321,   322,    73,    58,   285,    50,    43,   203,
     204,    47,   206,    55,    73,    73,   236,    47,    77,    18,
      31,   241,   201,   202,   273,    56,    73,    17,    73,    73,
      65,    38,    67,    73,   254,    73,    71,    72,    73,    74,
      17,    76,    32,    33,    34,    35,    36,    74,    17,    73,
     329,   300,    10,    73,    47,    32,    33,    34,    35,    36,
     280,    73,    17,    32,    33,    34,    35,    36,    17,    32,
      33,    34,    35,    36,    17,    17,    17,    67,    29,    18,
      18,    71,    72,    73,    74,    18,    76,    18,    18,    18,
      67,    18,    65,    17,    71,    72,    73,    74,    67,    76,
      53,    57,    71,    72,    73,    74,    17,    76,     4,     5,
      73,    58,    19,     9,    10,    11,    12,    13,    14,    15,
      16,    30,    19,    55,    52,    21,    22,    23,    17,    73,
      17,    27,    28,    32,    33,    34,    35,    36,    55,    73,
      17,    73,   336,    75,    76,    77,    78,     6,    44,    42,
      43,    18,    48,    73,    17,    51,    73,    18,    75,    76,
      77,    78,    18,    18,    18,    58,    59,    60,    61,    62,
      63,    64,    18,    18,    73,    73,    46,    70,    77,    18,
      17,    67,    75,    76,    77,    78,    67,    19,    53,    19,
      71,    72,    19,    74,    73,    76,    58,    59,    60,    61,
      62,    63,    75,    76,    77,    78,    18,    71,    18,    18,
      18,    18,    18,    73,    18,    73,    73,    18,    47,    19,
      18,    18,    18,   221,   263,   210,   111,   256,   284,   155,
     329
};

/* YYSTOS[STATE-NUM] -- The symbol kind of the accessing symbol of
   state STATE-NUM.  */
static const yytype_uint8 yystos[] =
{
       0,     4,     5,     9,    10,    11,    12,    13,    14,    15,
      16,    21,    22,    23,    27,    28,    44,    48,    51,    81,
      82,    83,    84,    85,    86,    87,    88,    89,    90,    91,
      92,    93,    94,    95,   100,   104,   105,   109,   110,   126,
     127,   128,     6,     8,    41,    66,     6,     8,    17,    32,
      33,    34,    35,    36,    67,    71,    72,    73,    74,    76,
     103,   111,   112,   113,    73,    77,   112,   113,   114,   115,
     116,    73,     7,    30,    37,    73,    73,    49,    82,     0,
       3,   133,    73,    73,    73,     8,    73,    73,   109,   112,
      17,    17,    17,    17,    17,    17,    29,    71,    72,   112,
      19,    75,    76,    77,    78,    29,    55,    73,   108,   108,
      37,    19,    37,    38,   119,    73,    73,    44,    58,    50,
      17,    55,    47,    55,    73,    47,    18,    18,    32,    33,
      34,    35,    36,    73,   117,   117,    77,   117,   117,   117,
     111,    73,   111,   112,   112,   112,   112,    77,    73,    73,
     115,    73,    43,    65,   112,   113,   123,   124,    56,   120,
      31,   119,    73,   107,    76,   103,    74,    73,    97,   109,
      73,   109,    47,    73,    17,    17,    17,    17,    17,    29,
      18,    18,    18,    18,    18,    18,    18,   108,   108,    65,
      17,    42,    43,    58,    59,    60,    61,    62,    63,    64,
      70,   125,   125,    39,    40,    53,    57,   122,    17,    58,
      19,   106,    71,    72,    30,    20,    24,    25,    26,    69,
      99,    19,    96,    17,    73,   117,   117,    77,   117,   117,
     117,    73,    19,    45,   118,   118,    17,   109,   103,    42,
      64,    17,    43,    67,   112,   112,   123,   123,   117,   121,
     123,    52,   129,   103,    17,   103,   107,   119,     6,    17,
      43,    67,    68,    97,    18,    73,    17,    18,    18,    18,
      18,    18,    18,    73,    46,   119,   119,   109,    18,   103,
      17,   103,   109,    67,    19,    53,    19,   101,   109,   106,
      73,    71,    98,    67,    68,    96,    19,   102,    73,   108,
      73,   120,   120,    18,   103,   109,   101,    18,   121,   117,
     130,   131,   103,    18,    18,    18,    73,    18,   102,   118,
     108,   122,   122,   101,    18,    18,    11,    54,   132,    19,
     101,    43,    67,    68,   102,    18,    47,   129,   129,    18,
     130,    67,    68,   123,   118
};

/* YYR1[RULE-NUM] -- Symbol kind of the left-hand side of rule RULE-NUM.  */
static const yytype_uint8 yyr1[] =
{
       0,    80,    81,    82,    82,    82,    82,    82,    82,    82,
      82,    82,    82,    82,    82,    82,    82,    82,    82,    82,
      82,    82,    82,    82,    83,    84,    85,    86,    87,    88,
      89,    90,    91,    92,    92,    93,    94,    94,    95,    96,
      96,    97,    97,    97,    97,    97,    97,    97,    97,    97,
      97,    98,    99,    99,    99,    99,    99,   100,   101,   101,
     102,   102,   103,   103,   103,   103,   103,   103,   104,   105,
     106,   106,   107,   107,   108,   108,   108,   109,   109,   109,
     110,   111,   111,   112,   112,   112,   112,   112,   112,   112,
     112,   112,   112,   112,   112,   113,   113,   113,   113,   113,
     113,   114,   115,   115,   115,   116,   116,   117,   117,   117,
     117,   117,   117,   117,   117,   118,   118,   118,   119,   119,
     120,   120,   121,   121,   122,   122,   123,   123,   123,   123,
     124,   124,   124,   124,   124,   124,   124,   124,   124,   124,
     124,   124,   125,   125,   125,   125,   125,   125,   126,   127,
     128,   129,   129,   130,   130,   131,   132,   132,   132,   133,
     133
};

/* YYR2[RULE-NUM] -- Number of symbols on the right-hand side of rule RULE-NUM.  */
static const yytype_int8 yyr2[] =
{
       0,     2,     2,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       3,     2,     2,    10,     9,     5,     7,     5,     5,     0,
       3,     5,     6,     7,     6,     7,     2,     3,     4,     3,
       4,     1,     1,     1,     1,     1,     1,     8,     0,     3,
       0,     3,     1,     2,     1,     2,     1,     1,     4,     6,
       0,     3,     3,     5,     0,     1,     2,     6,    10,    10,
       2,     1,     3,     3,     3,     3,     3,     3,     2,     3,
       4,     1,     1,     3,     1,     4,     4,     4,     4,     4,
       4,     1,     2,     2,     3,     1,     3,     1,     3,     4,
       4,     4,     4,     4,     4,     0,     4,     7,     0,     2,
       0,     3,     1,     3,     0,     2,     0,     1,     3,     3,
       3,     4,     6,     7,     5,     6,     4,     5,     3,     4,
       3,     3,     1,     1,     1,     1,     1,     1,     7,     2,
       4,     0,     3,     1,     3,     2,     0,     1,     1,     0,
       1
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
#line 243 "yacc_sql.y"
  {
    std::unique_ptr<ParsedSqlNode> sql_node = std::unique_ptr<ParsedSqlNode>((yyvsp[-1].sql_node));
    sql_result->add_sql_node(std::move(sql_node));
  }
#line 1918 "yacc_sql.cpp"
    break;

  case 24: /* exit_stmt: EXIT  */
#line 274 "yacc_sql.y"
         {
      (void)yynerrs;  // 这么写为了消除yynerrs未使用的告警。如果你有更好的方法欢迎提PR
      (yyval.sql_node) = new ParsedSqlNode(SCF_EXIT);
    }
#line 1927 "yacc_sql.cpp"
    break;

  case 25: /* help_stmt: HELP  */
#line 280 "yacc_sql.y"
         {
      (yyval.sql_node) = new ParsedSqlNode(SCF_HELP);
    }
#line 1935 "yacc_sql.cpp"
    break;

  case 26: /* sync_stmt: SYNC  */
#line 285 "yacc_sql.y"
         {
      (yyval.sql_node) = new ParsedSqlNode(SCF_SYNC);
    }
#line 1943 "yacc_sql.cpp"
    break;

  case 27: /* begin_stmt: TRX_BEGIN  */
#line 291 "yacc_sql.y"
               {
      (yyval.sql_node) = new ParsedSqlNode(SCF_BEGIN);
    }
#line 1951 "yacc_sql.cpp"
    break;

  case 28: /* commit_stmt: TRX_COMMIT  */
#line 297 "yacc_sql.y"
               {
      (yyval.sql_node) = new ParsedSqlNode(SCF_COMMIT);
    }
#line 1959 "yacc_sql.cpp"
    break;

  case 29: /* rollback_stmt: TRX_ROLLBACK  */
#line 303 "yacc_sql.y"
                  {
      (yyval.sql_node) = new ParsedSqlNode(SCF_ROLLBACK);
    }
#line 1967 "yacc_sql.cpp"
    break;

  case 30: /* drop_table_stmt: DROP TABLE ID  */
#line 309 "yacc_sql.y"
                  {
      (yyval.sql_node) = new ParsedSqlNode(SCF_DROP_TABLE);
      (yyval.sql_node)->drop_table.relation_name = (yyvsp[0].string);
      free((yyvsp[0].string));
    }
#line 1977 "yacc_sql.cpp"
    break;

  case 31: /* show_tables_stmt: SHOW TABLES  */
#line 316 "yacc_sql.y"
                {
      (yyval.sql_node) = new ParsedSqlNode(SCF_SHOW_TABLES);
    }
#line 1985 "yacc_sql.cpp"
    break;

  case 32: /* desc_table_stmt: DESC ID  */
#line 322 "yacc_sql.y"
             {
      (yyval.sql_node) = new ParsedSqlNode(SCF_DESC_TABLE);
      (yyval.sql_node)->desc_table.relation_name = (yyvsp[0].string);
      free((yyvsp[0].string));
    }
#line 1995 "yacc_sql.cpp"
    break;

  case 33: /* create_index_stmt: CREATE UNIQUE INDEX ID ON ID LBRACE ID id_list RBRACE  */
#line 331 "yacc_sql.y"
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
#line 2017 "yacc_sql.cpp"
    break;

  case 34: /* create_index_stmt: CREATE INDEX ID ON ID LBRACE ID id_list RBRACE  */
#line 349 "yacc_sql.y"
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
#line 2038 "yacc_sql.cpp"
    break;

  case 35: /* drop_index_stmt: DROP INDEX ID ON ID  */
#line 369 "yacc_sql.y"
    {
      (yyval.sql_node) = new ParsedSqlNode(SCF_DROP_INDEX);
      (yyval.sql_node)->drop_index.index_name = (yyvsp[-2].string);
      (yyval.sql_node)->drop_index.relation_name = (yyvsp[0].string);
      free((yyvsp[-2].string));
      free((yyvsp[0].string));
    }
#line 2050 "yacc_sql.cpp"
    break;

  case 36: /* create_table_stmt: CREATE TABLE ID LBRACE attr_def attr_def_list RBRACE  */
#line 379 "yacc_sql.y"
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
#line 2070 "yacc_sql.cpp"
    break;

  case 37: /* create_table_stmt: CREATE TABLE ID AS select_stmt  */
#line 395 "yacc_sql.y"
    {
      (yyval.sql_node) = new ParsedSqlNode(SCF_CREATE_TABLE);
      CreateTableSqlNode &create_table = (yyval.sql_node)->create_table;
      create_table.relation_name = (yyvsp[-2].string);
      create_table.is_ctas = true;
      create_table.select_sql = new SelectSqlNode(std::move((yyvsp[0].sql_node)->selection));
      free((yyvsp[-2].string));
      delete (yyvsp[0].sql_node);
    }
#line 2084 "yacc_sql.cpp"
    break;

  case 38: /* create_view_stmt: CREATE VIEW ID AS select_stmt  */
#line 408 "yacc_sql.y"
    {
      (yyval.sql_node) = new ParsedSqlNode(SCF_CREATE_VIEW);
      (yyval.sql_node)->create_view.view_name = (yyvsp[-2].string);
      (yyval.sql_node)->create_view.select_sql = std::move((yyvsp[0].sql_node)->selection);
      free((yyvsp[-2].string));
      delete (yyvsp[0].sql_node);
    }
#line 2096 "yacc_sql.cpp"
    break;

  case 39: /* attr_def_list: %empty  */
#line 418 "yacc_sql.y"
    {
      (yyval.attr_infos) = nullptr;
    }
#line 2104 "yacc_sql.cpp"
    break;

  case 40: /* attr_def_list: COMMA attr_def attr_def_list  */
#line 422 "yacc_sql.y"
    {
      if ((yyvsp[0].attr_infos) != nullptr) {
        (yyval.attr_infos) = (yyvsp[0].attr_infos);
      } else {
        (yyval.attr_infos) = new std::vector<AttrInfoSqlNode>;
      }
      (yyval.attr_infos)->emplace_back(*(yyvsp[-1].attr_info));
      delete (yyvsp[-1].attr_info);
    }
#line 2118 "yacc_sql.cpp"
    break;

  case 41: /* attr_def: ID type LBRACE number RBRACE  */
#line 435 "yacc_sql.y"
    {
      (yyval.attr_info) = new AttrInfoSqlNode;
      (yyval.attr_info)->type = (AttrType)(yyvsp[-3].number);
      (yyval.attr_info)->name = (yyvsp[-4].string);
      (yyval.attr_info)->length = (yyvsp[-1].number);
      free((yyvsp[-4].string));
    }
#line 2130 "yacc_sql.cpp"
    break;

  case 42: /* attr_def: ID type LBRACE number RBRACE NULL_T  */
#line 443 "yacc_sql.y"
    {
      (yyval.attr_info) = new AttrInfoSqlNode;
      (yyval.attr_info)->type = (AttrType)(yyvsp[-4].number);
      (yyval.attr_info)->name = (yyvsp[-5].string);
      (yyval.attr_info)->length = (yyvsp[-2].number);
      (yyval.attr_info)->nullable = true;
      free((yyvsp[-5].string));
    }
#line 2143 "yacc_sql.cpp"
    break;

  case 43: /* attr_def: ID type LBRACE number RBRACE NOT NULL_T  */
#line 452 "yacc_sql.y"
    {
      (yyval.attr_info) = new AttrInfoSqlNode;
      (yyval.attr_info)->type = (AttrType)(yyvsp[-5].number);
      (yyval.attr_info)->name = (yyvsp[-6].string);
      (yyval.attr_info)->length = (yyvsp[-3].number);
      (yyval.attr_info)->nullable = false;
      free((yyvsp[-6].string));
    }
#line 2156 "yacc_sql.cpp"
    break;

  case 44: /* attr_def: ID type LBRACE number RBRACE NULLABLE_T  */
#line 461 "yacc_sql.y"
    {
      (yyval.attr_info) = new AttrInfoSqlNode;
      (yyval.attr_info)->type = (AttrType)(yyvsp[-4].number);
      (yyval.attr_info)->name = (yyvsp[-5].string);
      (yyval.attr_info)->length = (yyvsp[-2].number);
      (yyval.attr_info)->nullable = true;
      free((yyvsp[-5].string));
    }
#line 2169 "yacc_sql.cpp"
    break;

  case 45: /* attr_def: ID type LBRACE number RBRACE NOT NULLABLE_T  */
#line 470 "yacc_sql.y"
    {
      (yyval.attr_info) = new AttrInfoSqlNode;
      (yyval.attr_info)->type = (AttrType)(yyvsp[-5].number);
      (yyval.attr_info)->name = (yyvsp[-6].string);
      (yyval.attr_info)->length = (yyvsp[-3].number);
      (yyval.attr_info)->nullable = false;
      free((yyvsp[-6].string));
    }
#line 2182 "yacc_sql.cpp"
    break;

  case 46: /* attr_def: ID type  */
#line 479 "yacc_sql.y"
    {
      (yyval.attr_info) = new AttrInfoSqlNode;
      (yyval.attr_info)->type = (AttrType)(yyvsp[0].number);
      (yyval.attr_info)->name = (yyvsp[-1].string);
      (yyval.attr_info)->length = ((yyvsp[0].number) == TEXTS) ? 4096 : 4;
      free((yyvsp[-1].string));
    }
#line 2194 "yacc_sql.cpp"
    break;

  case 47: /* attr_def: ID type NULL_T  */
#line 487 "yacc_sql.y"
    {
      (yyval.attr_info) = new AttrInfoSqlNode;
      (yyval.attr_info)->type = (AttrType)(yyvsp[-1].number);
      (yyval.attr_info)->name = (yyvsp[-2].string);
      (yyval.attr_info)->length = ((yyvsp[-1].number) == TEXTS) ? 4096 : 4;
      (yyval.attr_info)->nullable = true;
      free((yyvsp[-2].string));
    }
#line 2207 "yacc_sql.cpp"
    break;

  case 48: /* attr_def: ID type NOT NULL_T  */
#line 496 "yacc_sql.y"
    {
      (yyval.attr_info) = new AttrInfoSqlNode;
      (yyval.attr_info)->type = (AttrType)(yyvsp[-2].number);
      (yyval.attr_info)->name = (yyvsp[-3].string);
      (yyval.attr_info)->length = ((yyvsp[-2].number) == TEXTS) ? 4096 : 4;
      (yyval.attr_info)->nullable = false;
      free((yyvsp[-3].string));
    }
#line 2220 "yacc_sql.cpp"
    break;

  case 49: /* attr_def: ID type NULLABLE_T  */
#line 505 "yacc_sql.y"
    {
      (yyval.attr_info) = new AttrInfoSqlNode;
      (yyval.attr_info)->type = (AttrType)(yyvsp[-1].number);
      (yyval.attr_info)->name = (yyvsp[-2].string);
      (yyval.attr_info)->length = ((yyvsp[-1].number) == TEXTS) ? 4096 : 4;
      (yyval.attr_info)->nullable = true;
      free((yyvsp[-2].string));
    }
#line 2233 "yacc_sql.cpp"
    break;

  case 50: /* attr_def: ID type NOT NULLABLE_T  */
#line 514 "yacc_sql.y"
    {
      (yyval.attr_info) = new AttrInfoSqlNode;
      (yyval.attr_info)->type = (AttrType)(yyvsp[-2].number);
      (yyval.attr_info)->name = (yyvsp[-3].string);
      (yyval.attr_info)->length = ((yyvsp[-2].number) == TEXTS) ? 4096 : 4;
      (yyval.attr_info)->nullable = false;
      free((yyvsp[-3].string));
    }
#line 2246 "yacc_sql.cpp"
    break;

  case 51: /* number: NUMBER  */
#line 524 "yacc_sql.y"
           {(yyval.number) = (yyvsp[0].number);}
#line 2252 "yacc_sql.cpp"
    break;

  case 52: /* type: INT_T  */
#line 527 "yacc_sql.y"
               { (yyval.number)=INTS; }
#line 2258 "yacc_sql.cpp"
    break;

  case 53: /* type: STRING_T  */
#line 528 "yacc_sql.y"
               { (yyval.number)=CHARS; }
#line 2264 "yacc_sql.cpp"
    break;

  case 54: /* type: FLOAT_T  */
#line 529 "yacc_sql.y"
               { (yyval.number)=FLOATS; }
#line 2270 "yacc_sql.cpp"
    break;

  case 55: /* type: DATE_T  */
#line 530 "yacc_sql.y"
               { (yyval.number)=DATES; }
#line 2276 "yacc_sql.cpp"
    break;

  case 56: /* type: TEXT_T  */
#line 531 "yacc_sql.y"
               { (yyval.number)=TEXTS; }
#line 2282 "yacc_sql.cpp"
    break;

  case 57: /* insert_stmt: INSERT INTO ID VALUES LBRACE value value_list RBRACE  */
#line 535 "yacc_sql.y"
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
#line 2298 "yacc_sql.cpp"
    break;

  case 58: /* value_list: %empty  */
#line 550 "yacc_sql.y"
    {
      (yyval.value_list) = nullptr;
    }
#line 2306 "yacc_sql.cpp"
    break;

  case 59: /* value_list: COMMA value value_list  */
#line 553 "yacc_sql.y"
                              {
      if ((yyvsp[0].value_list) != nullptr) {
        (yyval.value_list) = (yyvsp[0].value_list);
      } else {
        (yyval.value_list) = new std::vector<Value>;
      }
      (yyval.value_list)->emplace_back(*(yyvsp[-1].value));
      delete (yyvsp[-1].value);
    }
#line 2320 "yacc_sql.cpp"
    break;

  case 60: /* id_list: %empty  */
#line 565 "yacc_sql.y"
    {
      (yyval.string_list) = nullptr;
    }
#line 2328 "yacc_sql.cpp"
    break;

  case 61: /* id_list: COMMA ID id_list  */
#line 569 "yacc_sql.y"
    {
      if ((yyvsp[0].string_list) != nullptr) {
        (yyval.string_list) = (yyvsp[0].string_list);
      } else {
        (yyval.string_list) = new std::vector<std::string>;
      }
      (yyval.string_list)->emplace_back((yyvsp[-1].string));
      free((yyvsp[-1].string));
    }
#line 2342 "yacc_sql.cpp"
    break;

  case 62: /* value: NUMBER  */
#line 580 "yacc_sql.y"
           {
      (yyval.value) = new Value((int)(yyvsp[0].number));
      (yyloc) = (yylsp[0]);
    }
#line 2351 "yacc_sql.cpp"
    break;

  case 63: /* value: '-' NUMBER  */
#line 584 "yacc_sql.y"
                 {
      (yyval.value) = new Value(-(int)(yyvsp[0].number));
      (yyloc) = (yylsp[0]);
    }
#line 2360 "yacc_sql.cpp"
    break;

  case 64: /* value: FLOAT  */
#line 588 "yacc_sql.y"
           {
      (yyval.value) = new Value((float)(yyvsp[0].floats));
      (yyloc) = (yylsp[0]);
    }
#line 2369 "yacc_sql.cpp"
    break;

  case 65: /* value: '-' FLOAT  */
#line 592 "yacc_sql.y"
                {
      (yyval.value) = new Value(-(float)(yyvsp[0].floats));
      (yyloc) = (yylsp[0]);
    }
#line 2378 "yacc_sql.cpp"
    break;

  case 66: /* value: SSS  */
#line 596 "yacc_sql.y"
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
#line 2396 "yacc_sql.cpp"
    break;

  case 67: /* value: NULL_T  */
#line 609 "yacc_sql.y"
             {
      (yyval.value) = new Value();
      (yyval.value)->set_null(true);
      (yyloc) = (yylsp[0]);
    }
#line 2406 "yacc_sql.cpp"
    break;

  case 68: /* delete_stmt: DELETE FROM ID where  */
#line 618 "yacc_sql.y"
    {
      (yyval.sql_node) = new ParsedSqlNode(SCF_DELETE);
      (yyval.sql_node)->deletion.relation_name = (yyvsp[-1].string);
      if ((yyvsp[0].condition_list) != nullptr) {
        (yyval.sql_node)->deletion.conditions.swap(*(yyvsp[0].condition_list));
        delete (yyvsp[0].condition_list);
      }
      free((yyvsp[-1].string));
    }
#line 2420 "yacc_sql.cpp"
    break;

  case 69: /* update_stmt: UPDATE ID SET update_assign update_assign_list where  */
#line 630 "yacc_sql.y"
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
#line 2441 "yacc_sql.cpp"
    break;

  case 70: /* update_assign_list: %empty  */
#line 649 "yacc_sql.y"
    {
      (yyval.update_assign_list) = nullptr;
    }
#line 2449 "yacc_sql.cpp"
    break;

  case 71: /* update_assign_list: COMMA update_assign update_assign_list  */
#line 653 "yacc_sql.y"
    {
      if ((yyvsp[0].update_assign_list) != nullptr) {
        (yyval.update_assign_list) = (yyvsp[0].update_assign_list);
      } else {
        (yyval.update_assign_list) = new std::vector<UpdateAssignmentNode>;
      }
      (yyval.update_assign_list)->emplace_back(std::move(*(yyvsp[-1].update_assign)));
      delete (yyvsp[-1].update_assign);
    }
#line 2463 "yacc_sql.cpp"
    break;

  case 72: /* update_assign: ID EQ value  */
#line 665 "yacc_sql.y"
    {
      (yyval.update_assign) = new UpdateAssignmentNode;
      (yyval.update_assign)->attribute_name = (yyvsp[-2].string);
      (yyval.update_assign)->value = *(yyvsp[0].value);
      (yyval.update_assign)->is_subquery = false;
      free((yyvsp[-2].string));
      delete (yyvsp[0].value);
    }
#line 2476 "yacc_sql.cpp"
    break;

  case 73: /* update_assign: ID EQ LBRACE select_stmt RBRACE  */
#line 674 "yacc_sql.y"
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
#line 2497 "yacc_sql.cpp"
    break;

  case 74: /* alias_opt: %empty  */
#line 692 "yacc_sql.y"
                { (yyval.string) = nullptr; }
#line 2503 "yacc_sql.cpp"
    break;

  case 75: /* alias_opt: ID  */
#line 693 "yacc_sql.y"
         { (yyval.string) = (yyvsp[0].string); }
#line 2509 "yacc_sql.cpp"
    break;

  case 76: /* alias_opt: AS ID  */
#line 694 "yacc_sql.y"
            { (yyval.string) = (yyvsp[0].string); }
#line 2515 "yacc_sql.cpp"
    break;

  case 77: /* select_stmt: SELECT select_expr_list where group_by_clause having_clause order_by_clause  */
#line 699 "yacc_sql.y"
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
#line 2549 "yacc_sql.cpp"
    break;

  case 78: /* select_stmt: SELECT select_expr_list FROM ID alias_opt join_clause where group_by_clause having_clause order_by_clause  */
#line 729 "yacc_sql.y"
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
#line 2602 "yacc_sql.cpp"
    break;

  case 79: /* select_stmt: SELECT select_attr FROM ID alias_opt join_clause where group_by_clause having_clause order_by_clause  */
#line 778 "yacc_sql.y"
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
#line 2654 "yacc_sql.cpp"
    break;

  case 80: /* calc_stmt: CALC expression_list  */
#line 828 "yacc_sql.y"
    {
      (yyval.sql_node) = new ParsedSqlNode(SCF_CALC);
      std::reverse((yyvsp[0].expression_list)->begin(), (yyvsp[0].expression_list)->end());
      (yyval.sql_node)->calc.expressions.swap(*(yyvsp[0].expression_list));
      delete (yyvsp[0].expression_list);
    }
#line 2665 "yacc_sql.cpp"
    break;

  case 81: /* expression_list: expression  */
#line 838 "yacc_sql.y"
    {
      (yyval.expression_list) = new std::vector<Expression*>;
      (yyval.expression_list)->emplace_back((yyvsp[0].expression));
    }
#line 2674 "yacc_sql.cpp"
    break;

  case 82: /* expression_list: expression COMMA expression_list  */
#line 843 "yacc_sql.y"
    {
      if ((yyvsp[0].expression_list) != nullptr) {
        (yyval.expression_list) = (yyvsp[0].expression_list);
      } else {
        (yyval.expression_list) = new std::vector<Expression *>;
      }
      (yyval.expression_list)->emplace_back((yyvsp[-2].expression));
    }
#line 2687 "yacc_sql.cpp"
    break;

  case 83: /* expression: expression '+' expression  */
#line 853 "yacc_sql.y"
                              {
      (yyval.expression) = create_arithmetic_expression(ArithmeticExpr::Type::ADD, (yyvsp[-2].expression), (yyvsp[0].expression), sql_string, &(yyloc));
    }
#line 2695 "yacc_sql.cpp"
    break;

  case 84: /* expression: expression '-' expression  */
#line 856 "yacc_sql.y"
                                {
      (yyval.expression) = create_arithmetic_expression(ArithmeticExpr::Type::SUB, (yyvsp[-2].expression), (yyvsp[0].expression), sql_string, &(yyloc));
    }
#line 2703 "yacc_sql.cpp"
    break;

  case 85: /* expression: expression '*' expression  */
#line 859 "yacc_sql.y"
                                {
      (yyval.expression) = create_arithmetic_expression(ArithmeticExpr::Type::MUL, (yyvsp[-2].expression), (yyvsp[0].expression), sql_string, &(yyloc));
    }
#line 2711 "yacc_sql.cpp"
    break;

  case 86: /* expression: expression '/' expression  */
#line 862 "yacc_sql.y"
                                {
      (yyval.expression) = create_arithmetic_expression(ArithmeticExpr::Type::DIV, (yyvsp[-2].expression), (yyvsp[0].expression), sql_string, &(yyloc));
    }
#line 2719 "yacc_sql.cpp"
    break;

  case 87: /* expression: LBRACE expression RBRACE  */
#line 865 "yacc_sql.y"
                               {
      (yyval.expression) = (yyvsp[-1].expression);
      (yyval.expression)->set_name(token_name(sql_string, &(yyloc)));
    }
#line 2728 "yacc_sql.cpp"
    break;

  case 88: /* expression: '-' expression  */
#line 869 "yacc_sql.y"
                                  {
      (yyval.expression) = create_arithmetic_expression(ArithmeticExpr::Type::NEGATIVE, (yyvsp[0].expression), nullptr, sql_string, &(yyloc));
    }
#line 2736 "yacc_sql.cpp"
    break;

  case 89: /* expression: LBRACE select_stmt RBRACE  */
#line 872 "yacc_sql.y"
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
#line 2753 "yacc_sql.cpp"
    break;

  case 90: /* expression: ID LBRACE expression_list RBRACE  */
#line 884 "yacc_sql.y"
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
#line 2771 "yacc_sql.cpp"
    break;

  case 91: /* expression: value  */
#line 897 "yacc_sql.y"
            {
      (yyval.expression) = new ValueExpr(*(yyvsp[0].value));
      (yyval.expression)->set_name(token_name(sql_string, &(yyloc)));
      delete (yyvsp[0].value);
    }
#line 2781 "yacc_sql.cpp"
    break;

  case 92: /* expression: ID  */
#line 902 "yacc_sql.y"
         {
      (yyval.expression) = new UnboundFieldExpr("", (yyvsp[0].string));
      (yyval.expression)->set_name((yyvsp[0].string));
      free((yyvsp[0].string));
    }
#line 2791 "yacc_sql.cpp"
    break;

  case 93: /* expression: ID DOT ID  */
#line 907 "yacc_sql.y"
                {
      (yyval.expression) = new UnboundFieldExpr((yyvsp[-2].string), (yyvsp[0].string));
      (yyval.expression)->set_name((yyvsp[0].string));
      free((yyvsp[-2].string));
      free((yyvsp[0].string));
    }
#line 2802 "yacc_sql.cpp"
    break;

  case 94: /* expression: agg_expr  */
#line 913 "yacc_sql.y"
               {
      (yyval.expression) = new AggregationExpr((yyvsp[0].rel_attr)->aggregation_type, (yyvsp[0].rel_attr)->relation_name, (yyvsp[0].rel_attr)->attribute_name);
      delete (yyvsp[0].rel_attr);
    }
#line 2811 "yacc_sql.cpp"
    break;

  case 95: /* agg_expr: MAX LBRACE rel_attr RBRACE  */
#line 922 "yacc_sql.y"
    {
      (yyval.rel_attr) = (yyvsp[-1].rel_attr);
      (yyval.rel_attr)->aggregation_type = AGG_MAX;
    }
#line 2820 "yacc_sql.cpp"
    break;

  case 96: /* agg_expr: MIN LBRACE rel_attr RBRACE  */
#line 927 "yacc_sql.y"
    {
      (yyval.rel_attr) = (yyvsp[-1].rel_attr);
      (yyval.rel_attr)->aggregation_type = AGG_MIN;
    }
#line 2829 "yacc_sql.cpp"
    break;

  case 97: /* agg_expr: COUNT LBRACE rel_attr RBRACE  */
#line 932 "yacc_sql.y"
    {
      (yyval.rel_attr) = (yyvsp[-1].rel_attr);
      (yyval.rel_attr)->aggregation_type = AGG_COUNT;
    }
#line 2838 "yacc_sql.cpp"
    break;

  case 98: /* agg_expr: COUNT LBRACE '*' RBRACE  */
#line 937 "yacc_sql.y"
    {
      (yyval.rel_attr) = new RelAttrSqlNode;
      (yyval.rel_attr)->attribute_name = "*";
      (yyval.rel_attr)->aggregation_type = AGG_COUNT;
    }
#line 2848 "yacc_sql.cpp"
    break;

  case 99: /* agg_expr: AVG LBRACE rel_attr RBRACE  */
#line 943 "yacc_sql.y"
    {
      (yyval.rel_attr) = (yyvsp[-1].rel_attr);
      (yyval.rel_attr)->aggregation_type = AGG_AVG;
    }
#line 2857 "yacc_sql.cpp"
    break;

  case 100: /* agg_expr: SUM LBRACE rel_attr RBRACE  */
#line 948 "yacc_sql.y"
    {
      (yyval.rel_attr) = (yyvsp[-1].rel_attr);
      (yyval.rel_attr)->aggregation_type = AGG_SUM;
    }
#line 2866 "yacc_sql.cpp"
    break;

  case 101: /* select_attr: '*'  */
#line 955 "yacc_sql.y"
        {
      (yyval.rel_attr_list) = new std::vector<RelAttrSqlNode>;
      RelAttrSqlNode attr;
      attr.relation_name  = "";
      attr.attribute_name = "*";
      (yyval.rel_attr_list)->emplace_back(attr);
    }
#line 2878 "yacc_sql.cpp"
    break;

  case 102: /* select_expr_item: expression alias_opt  */
#line 968 "yacc_sql.y"
    {
      (yyval.select_expr) = new SelectExprNode;
      (yyval.select_expr)->expr = (yyvsp[-1].expression);
      (yyval.select_expr)->alias = (yyvsp[0].string) ? (yyvsp[0].string) : (yyvsp[-1].expression)->name();
      if ((yyvsp[0].string)) free((yyvsp[0].string));
    }
#line 2889 "yacc_sql.cpp"
    break;

  case 103: /* select_expr_item: agg_expr alias_opt  */
#line 975 "yacc_sql.y"
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
#line 2911 "yacc_sql.cpp"
    break;

  case 104: /* select_expr_item: ID DOT '*'  */
#line 993 "yacc_sql.y"
    {
      (yyval.select_expr) = new SelectExprNode;
      (yyval.select_expr)->is_star = true;
      (yyval.select_expr)->star_table = (yyvsp[-2].string);
      free((yyvsp[-2].string));
    }
#line 2922 "yacc_sql.cpp"
    break;

  case 105: /* select_expr_list: select_expr_item  */
#line 1003 "yacc_sql.y"
    {
      (yyval.select_expr_list) = new std::vector<SelectExprNode>;
      (yyval.select_expr_list)->emplace_back(std::move(*(yyvsp[0].select_expr)));
      delete (yyvsp[0].select_expr);
    }
#line 2932 "yacc_sql.cpp"
    break;

  case 106: /* select_expr_list: select_expr_list COMMA select_expr_item  */
#line 1009 "yacc_sql.y"
    {
      (yyval.select_expr_list) = (yyvsp[-2].select_expr_list);
      (yyval.select_expr_list)->emplace_back(std::move(*(yyvsp[0].select_expr)));
      delete (yyvsp[0].select_expr);
    }
#line 2942 "yacc_sql.cpp"
    break;

  case 107: /* rel_attr: ID  */
#line 1017 "yacc_sql.y"
       {
      (yyval.rel_attr) = new RelAttrSqlNode;
      (yyval.rel_attr)->attribute_name = (yyvsp[0].string);
      free((yyvsp[0].string));
    }
#line 2952 "yacc_sql.cpp"
    break;

  case 108: /* rel_attr: ID DOT ID  */
#line 1022 "yacc_sql.y"
                {
      (yyval.rel_attr) = new RelAttrSqlNode;
      (yyval.rel_attr)->relation_name  = (yyvsp[-2].string);
      (yyval.rel_attr)->attribute_name = (yyvsp[0].string);
      free((yyvsp[-2].string));
      free((yyvsp[0].string));
    }
#line 2964 "yacc_sql.cpp"
    break;

  case 109: /* rel_attr: MAX LBRACE rel_attr RBRACE  */
#line 1029 "yacc_sql.y"
                                 {
      (yyval.rel_attr) = (yyvsp[-1].rel_attr);
      (yyval.rel_attr)->aggregation_type = AGG_MAX;
    }
#line 2973 "yacc_sql.cpp"
    break;

  case 110: /* rel_attr: MIN LBRACE rel_attr RBRACE  */
#line 1033 "yacc_sql.y"
                                 {
      (yyval.rel_attr) = (yyvsp[-1].rel_attr);
      (yyval.rel_attr)->aggregation_type = AGG_MIN;
    }
#line 2982 "yacc_sql.cpp"
    break;

  case 111: /* rel_attr: COUNT LBRACE rel_attr RBRACE  */
#line 1037 "yacc_sql.y"
                                   {
      (yyval.rel_attr) = (yyvsp[-1].rel_attr);
      (yyval.rel_attr)->aggregation_type = AGG_COUNT;
    }
#line 2991 "yacc_sql.cpp"
    break;

  case 112: /* rel_attr: COUNT LBRACE '*' RBRACE  */
#line 1041 "yacc_sql.y"
                              {
      (yyval.rel_attr) = new RelAttrSqlNode;
      (yyval.rel_attr)->attribute_name = "*";
      (yyval.rel_attr)->aggregation_type = AGG_COUNT;
    }
#line 3001 "yacc_sql.cpp"
    break;

  case 113: /* rel_attr: AVG LBRACE rel_attr RBRACE  */
#line 1046 "yacc_sql.y"
                                 {
      (yyval.rel_attr) = (yyvsp[-1].rel_attr);
      (yyval.rel_attr)->aggregation_type = AGG_AVG;
    }
#line 3010 "yacc_sql.cpp"
    break;

  case 114: /* rel_attr: SUM LBRACE rel_attr RBRACE  */
#line 1050 "yacc_sql.y"
                                 {
      (yyval.rel_attr) = (yyvsp[-1].rel_attr);
      (yyval.rel_attr)->aggregation_type = AGG_SUM;
    }
#line 3019 "yacc_sql.cpp"
    break;

  case 115: /* join_clause: %empty  */
#line 1075 "yacc_sql.y"
    {
      (yyval.join_clause) = new JoinClauseNode;
    }
#line 3027 "yacc_sql.cpp"
    break;

  case 116: /* join_clause: COMMA ID alias_opt join_clause  */
#line 1079 "yacc_sql.y"
    {
      (yyval.join_clause) = (yyvsp[0].join_clause);
      (yyval.join_clause)->relations.push_back((yyvsp[-2].string));
      (yyval.join_clause)->aliases.push_back((yyvsp[-1].string) ? (yyvsp[-1].string) : "");
      free((yyvsp[-2].string));
      if ((yyvsp[-1].string)) free((yyvsp[-1].string));
    }
#line 3039 "yacc_sql.cpp"
    break;

  case 117: /* join_clause: INNER JOIN ID alias_opt ON condition_list join_clause  */
#line 1087 "yacc_sql.y"
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
#line 3058 "yacc_sql.cpp"
    break;

  case 118: /* where: %empty  */
#line 1104 "yacc_sql.y"
    {
      (yyval.condition_list) = nullptr;
    }
#line 3066 "yacc_sql.cpp"
    break;

  case 119: /* where: WHERE condition_list  */
#line 1107 "yacc_sql.y"
                           {
      (yyval.condition_list) = (yyvsp[0].condition_list);
    }
#line 3074 "yacc_sql.cpp"
    break;

  case 120: /* group_by_clause: %empty  */
#line 1113 "yacc_sql.y"
    {
      (yyval.rel_attr_list) = nullptr;
    }
#line 3082 "yacc_sql.cpp"
    break;

  case 121: /* group_by_clause: GROUP BY group_by_list  */
#line 1117 "yacc_sql.y"
    {
      (yyval.rel_attr_list) = (yyvsp[0].rel_attr_list);
    }
#line 3090 "yacc_sql.cpp"
    break;

  case 122: /* group_by_list: rel_attr  */
#line 1123 "yacc_sql.y"
    {
      (yyval.rel_attr_list) = new std::vector<RelAttrSqlNode>;
      (yyval.rel_attr_list)->emplace_back(*(yyvsp[0].rel_attr));
      delete (yyvsp[0].rel_attr);
    }
#line 3100 "yacc_sql.cpp"
    break;

  case 123: /* group_by_list: rel_attr COMMA group_by_list  */
#line 1129 "yacc_sql.y"
    {
      if ((yyvsp[0].rel_attr_list) != nullptr) {
        (yyval.rel_attr_list) = (yyvsp[0].rel_attr_list);
      } else {
        (yyval.rel_attr_list) = new std::vector<RelAttrSqlNode>;
      }
      (yyval.rel_attr_list)->emplace_back(*(yyvsp[-2].rel_attr));
      delete (yyvsp[-2].rel_attr);
    }
#line 3114 "yacc_sql.cpp"
    break;

  case 124: /* having_clause: %empty  */
#line 1141 "yacc_sql.y"
    {
      (yyval.condition_list) = nullptr;
    }
#line 3122 "yacc_sql.cpp"
    break;

  case 125: /* having_clause: HAVING condition_list  */
#line 1145 "yacc_sql.y"
    {
      (yyval.condition_list) = (yyvsp[0].condition_list);
    }
#line 3130 "yacc_sql.cpp"
    break;

  case 126: /* condition_list: %empty  */
#line 1151 "yacc_sql.y"
    {
      (yyval.condition_list) = nullptr;
    }
#line 3138 "yacc_sql.cpp"
    break;

  case 127: /* condition_list: condition  */
#line 1154 "yacc_sql.y"
                {
      (yyval.condition_list) = new std::vector<ConditionSqlNode>;
      (yyval.condition_list)->emplace_back(std::move(*(yyvsp[0].condition)));
      delete (yyvsp[0].condition);
    }
#line 3148 "yacc_sql.cpp"
    break;

  case 128: /* condition_list: condition AND condition_list  */
#line 1159 "yacc_sql.y"
                                   {
      (yyval.condition_list) = (yyvsp[0].condition_list);
      (yyval.condition_list)->emplace_back(std::move(*(yyvsp[-2].condition)));
      delete (yyvsp[-2].condition);
    }
#line 3158 "yacc_sql.cpp"
    break;

  case 129: /* condition_list: condition OR condition_list  */
#line 1164 "yacc_sql.y"
                                  {
      (yyval.condition_list) = (yyvsp[0].condition_list);
      if ((yyval.condition_list) == nullptr) {
        (yyval.condition_list) = new std::vector<ConditionSqlNode>;
      }
      ConditionSqlNode node = std::move(*(yyvsp[-2].condition));
      delete (yyvsp[-2].condition);
      if (!(yyval.condition_list)->empty()) {
        (*(yyval.condition_list))[0].connect_or = true;
      }
      (yyval.condition_list)->insert((yyval.condition_list)->begin(), std::move(node));
    }
#line 3175 "yacc_sql.cpp"
    break;

  case 130: /* condition: expression LIKE value  */
#line 1179 "yacc_sql.y"
    {
      (yyval.condition) = new ConditionSqlNode;
      (yyval.condition)->left_expr = (yyvsp[-2].expression);
      (yyval.condition)->right_expr = new ValueExpr(*(yyvsp[0].value));
      (yyval.condition)->comp = LIKE_OP;
      delete (yyvsp[0].value);
    }
#line 3187 "yacc_sql.cpp"
    break;

  case 131: /* condition: expression NOT LIKE value  */
#line 1187 "yacc_sql.y"
    {
      (yyval.condition) = new ConditionSqlNode;
      (yyval.condition)->left_expr = (yyvsp[-3].expression);
      (yyval.condition)->right_expr = new ValueExpr(*(yyvsp[0].value));
      (yyval.condition)->comp = NOT_LIKE;
      delete (yyvsp[0].value);
    }
#line 3199 "yacc_sql.cpp"
    break;

  case 132: /* condition: expression IN LBRACE value value_list RBRACE  */
#line 1195 "yacc_sql.y"
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
#line 3218 "yacc_sql.cpp"
    break;

  case 133: /* condition: expression NOT IN LBRACE value value_list RBRACE  */
#line 1210 "yacc_sql.y"
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
#line 3237 "yacc_sql.cpp"
    break;

  case 134: /* condition: expression IN LBRACE select_stmt RBRACE  */
#line 1225 "yacc_sql.y"
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
#line 3256 "yacc_sql.cpp"
    break;

  case 135: /* condition: expression NOT IN LBRACE select_stmt RBRACE  */
#line 1240 "yacc_sql.y"
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
#line 3275 "yacc_sql.cpp"
    break;

  case 136: /* condition: EXISTS LBRACE select_stmt RBRACE  */
#line 1255 "yacc_sql.y"
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
#line 3293 "yacc_sql.cpp"
    break;

  case 137: /* condition: NOT EXISTS LBRACE select_stmt RBRACE  */
#line 1269 "yacc_sql.y"
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
#line 3311 "yacc_sql.cpp"
    break;

  case 138: /* condition: expression IS NULL_T  */
#line 1283 "yacc_sql.y"
    {
      Value null_value;
      null_value.set_null(true);
      (yyval.condition) = new ConditionSqlNode;
      (yyval.condition)->left_expr = (yyvsp[-2].expression);
      (yyval.condition)->right_expr = new ValueExpr(null_value);
      (yyval.condition)->comp = IS_NULL;
    }
#line 3324 "yacc_sql.cpp"
    break;

  case 139: /* condition: expression IS NOT NULL_T  */
#line 1292 "yacc_sql.y"
    {
      Value null_value;
      null_value.set_null(true);
      (yyval.condition) = new ConditionSqlNode;
      (yyval.condition)->left_expr = (yyvsp[-3].expression);
      (yyval.condition)->right_expr = new ValueExpr(null_value);
      (yyval.condition)->comp = IS_NOT_NULL;
    }
#line 3337 "yacc_sql.cpp"
    break;

  case 140: /* condition: expression comp_op expression  */
#line 1301 "yacc_sql.y"
    {
      (yyval.condition) = new ConditionSqlNode;
      (yyval.condition)->left_expr = (yyvsp[-2].expression);
      (yyval.condition)->right_expr = (yyvsp[0].expression);
      (yyval.condition)->comp = (yyvsp[-1].comp);
    }
#line 3348 "yacc_sql.cpp"
    break;

  case 141: /* condition: agg_expr comp_op expression  */
#line 1308 "yacc_sql.y"
    {
      (yyval.condition) = new ConditionSqlNode;
      (yyval.condition)->left_expr = new AggregationExpr((yyvsp[-2].rel_attr)->aggregation_type, (yyvsp[-2].rel_attr)->relation_name, (yyvsp[-2].rel_attr)->attribute_name);
      delete (yyvsp[-2].rel_attr);
      (yyval.condition)->comp = (yyvsp[-1].comp);
      (yyval.condition)->right_expr = (yyvsp[0].expression);
    }
#line 3360 "yacc_sql.cpp"
    break;

  case 142: /* comp_op: EQ  */
#line 1318 "yacc_sql.y"
         { (yyval.comp) = EQUAL_TO; }
#line 3366 "yacc_sql.cpp"
    break;

  case 143: /* comp_op: LT  */
#line 1319 "yacc_sql.y"
         { (yyval.comp) = LESS_THAN; }
#line 3372 "yacc_sql.cpp"
    break;

  case 144: /* comp_op: GT  */
#line 1320 "yacc_sql.y"
         { (yyval.comp) = GREAT_THAN; }
#line 3378 "yacc_sql.cpp"
    break;

  case 145: /* comp_op: LE  */
#line 1321 "yacc_sql.y"
         { (yyval.comp) = LESS_EQUAL; }
#line 3384 "yacc_sql.cpp"
    break;

  case 146: /* comp_op: GE  */
#line 1322 "yacc_sql.y"
         { (yyval.comp) = GREAT_EQUAL; }
#line 3390 "yacc_sql.cpp"
    break;

  case 147: /* comp_op: NE  */
#line 1323 "yacc_sql.y"
         { (yyval.comp) = NOT_EQUAL; }
#line 3396 "yacc_sql.cpp"
    break;

  case 148: /* load_data_stmt: LOAD DATA INFILE SSS INTO TABLE ID  */
#line 1328 "yacc_sql.y"
    {
      char *tmp_file_name = common::substr((yyvsp[-3].string), 1, strlen((yyvsp[-3].string)) - 2);
      
      (yyval.sql_node) = new ParsedSqlNode(SCF_LOAD_DATA);
      (yyval.sql_node)->load_data.relation_name = (yyvsp[0].string);
      (yyval.sql_node)->load_data.file_name = tmp_file_name;
      free((yyvsp[0].string));
      free(tmp_file_name);
    }
#line 3410 "yacc_sql.cpp"
    break;

  case 149: /* explain_stmt: EXPLAIN command_wrapper  */
#line 1341 "yacc_sql.y"
    {
      (yyval.sql_node) = new ParsedSqlNode(SCF_EXPLAIN);
      (yyval.sql_node)->explain.sql_node = std::unique_ptr<ParsedSqlNode>((yyvsp[0].sql_node));
    }
#line 3419 "yacc_sql.cpp"
    break;

  case 150: /* set_variable_stmt: SET ID EQ value  */
#line 1349 "yacc_sql.y"
    {
      (yyval.sql_node) = new ParsedSqlNode(SCF_SET_VARIABLE);
      (yyval.sql_node)->set_variable.name  = (yyvsp[-2].string);
      (yyval.sql_node)->set_variable.value = *(yyvsp[0].value);
      free((yyvsp[-2].string));
      delete (yyvsp[0].value);
    }
#line 3431 "yacc_sql.cpp"
    break;

  case 151: /* order_by_clause: %empty  */
#line 1360 "yacc_sql.y"
    {
      (yyval.order_by_list) = nullptr;
    }
#line 3439 "yacc_sql.cpp"
    break;

  case 152: /* order_by_clause: ORDER BY order_by_list  */
#line 1364 "yacc_sql.y"
    {
      (yyval.order_by_list) = (yyvsp[0].order_by_list);
    }
#line 3447 "yacc_sql.cpp"
    break;

  case 153: /* order_by_list: order_by_item  */
#line 1371 "yacc_sql.y"
    {
      (yyval.order_by_list) = new std::vector<OrderByNode>;
      (yyval.order_by_list)->emplace_back(*(yyvsp[0].order_by_node));
      delete (yyvsp[0].order_by_node);
    }
#line 3457 "yacc_sql.cpp"
    break;

  case 154: /* order_by_list: order_by_item COMMA order_by_list  */
#line 1377 "yacc_sql.y"
    {
      if ((yyvsp[0].order_by_list) != nullptr) {
        (yyval.order_by_list) = (yyvsp[0].order_by_list);
      } else {
        (yyval.order_by_list) = new std::vector<OrderByNode>;
      }
      (yyval.order_by_list)->emplace_back(*(yyvsp[-2].order_by_node));
      delete (yyvsp[-2].order_by_node);
    }
#line 3471 "yacc_sql.cpp"
    break;

  case 155: /* order_by_item: rel_attr opt_asc_desc  */
#line 1390 "yacc_sql.y"
    {
      (yyval.order_by_node) = new OrderByNode;
      (yyval.order_by_node)->relation_name = (yyvsp[-1].rel_attr)->relation_name;
      (yyval.order_by_node)->attribute_name = (yyvsp[-1].rel_attr)->attribute_name;
      (yyval.order_by_node)->is_asc = (yyvsp[0].boolean);
      delete (yyvsp[-1].rel_attr);
    }
#line 3483 "yacc_sql.cpp"
    break;

  case 156: /* opt_asc_desc: %empty  */
#line 1401 "yacc_sql.y"
    {
      (yyval.boolean) = true;
    }
#line 3491 "yacc_sql.cpp"
    break;

  case 157: /* opt_asc_desc: ASC  */
#line 1405 "yacc_sql.y"
    {
      (yyval.boolean) = true;
    }
#line 3499 "yacc_sql.cpp"
    break;

  case 158: /* opt_asc_desc: DESC  */
#line 1409 "yacc_sql.y"
    {
      (yyval.boolean) = false;
    }
#line 3507 "yacc_sql.cpp"
    break;


#line 3511 "yacc_sql.cpp"

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

#line 1417 "yacc_sql.y"

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

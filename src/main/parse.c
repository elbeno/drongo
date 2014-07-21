#ifndef lint
static char yysccsid[] = "@(#)yaccpar	1.8 (Berkeley) 01/20/90";
#endif
#define YYBYACC 1
#line 2 "parse.y"
/*
Copyright(C) 1990, Marcus J. Ranum, All Rights Reserved.
This software may be freely used, modified, and redistributed,
as long as this copyright message is left intact, and this
software is not used to develop any commercial product, or used
in any product that is provided on a pay-for-use basis.
*/



/*
EXPECTED CONFLICTS::conflicts: 1 shift/reduce on the if/else. This is OK.

	grammar for ubermud compiler. there are lots of cases where 'cute'
tricks are played to minimize the number of instructions that get written
into a program - this is unfortunate, but necessary, since every byte that
can be saved out of an object will decrease the size of the universe.
frequently used ops, like EVAL, ASGN, BLTIN, and ELEVAL use this technique
as much as possible. it makes the code a bit more convoluted, but it
should prove worthwhile.

	WARNING: severely icky-poo looking code ahead. The debug statments
are a ROYAL pain in the butt, but you will come to love them if you ever
need to debug this guy. If you make ANY changes, keep the debug statements
up-to-date, or you will be killed in a horrible way.
*/


/*
#define	COMPILEDEBUG
*/

#include	<stdio.h>
#include	<ctype.h>

#include	"ubermud.h"
#include	"externs.h"


/*
scratch buffers in which to compile and assemble input.
this is done in a somewhat roundabout manner, to maintain a static area
for compilation.
*/


static	char	*currfunc;	/* kludge - used to return function name */
static	long	currenum;	/* kludge - used to return function elem # */
static	int	currfuncop;	/* used to return assign op of funcs */
static	int	stringnum;
static	int	stringoff;
static	char	stringbuf[STRBUFSIZ];
static	int	instbuf[INSBUFSIZ];

static	Prog	bprg = { 0, 0, instbuf, stringbuf, 0, INSBUFSIZ, 0 };

/* ya caught me! a GLOBAL! */
Prog	*yaccprog = &bprg;

static	int	infunct = 0;

/*
shortened name for the current instruction address - I suppose some
day someone might want to assemble 2 programs at once, by re-pointing
yaccprog, so we deref this through a pointer.
*/
#define	ASMPC	(yaccprog->p_pc)

/* compiler state exception flag */
static	int	compiler_state = COMPILE_OK;

static	char	*infuncmsg = "operation only permitted within function";
#line 79 "y.tab.c"
#define NUM 257
#define STR 258
#define IDENT 259
#define OUTOFSPACE 260
#define BLTIN 261
#define IF 262
#define IN 263
#define FOR 264
#define FORARG 265
#define ELSE 266
#define FUNC 267
#define RETURN 268
#define VNULL 269
#define ASGN 270
#define REF 271
#define OR 272
#define AND 273
#define GT 274
#define GTE 275
#define LT 276
#define LTE 277
#define EQ 278
#define NE 279
#define ADD 280
#define SUB 281
#define MUL 282
#define DIV 283
#define UNARY 284
#define NOT 285
#define YYERRCODE 256
short yylhs[] = {                                        -1,
    0,    0,    0,    3,    3,    3,    1,    1,    1,    1,
    1,    1,    1,    1,    1,    7,    8,   11,   11,   12,
   12,   12,   13,   13,   13,    5,    5,    5,    5,   14,
   15,   15,   15,    4,    4,    4,    4,    4,    4,    4,
    4,    4,    4,    4,    4,    4,    4,    4,    4,    4,
    4,    4,    4,    4,    4,    4,    4,    4,    9,   10,
    6,   16,    2,    2,    2,   17,   17,   17,   18,
};
short yylen[] = {                                         2,
    0,    2,    2,    4,    4,    3,    2,    2,    3,    1,
    4,    7,    7,    5,    3,    3,    0,    0,    2,    1,
    2,    2,    1,    2,    2,    3,    5,    3,    6,    0,
    1,    2,    2,    1,    1,    1,    2,    1,    1,    1,
    5,    4,    3,    3,    3,    3,    3,    2,    2,    3,
    3,    3,    3,    3,    3,    3,    3,    2,    1,    1,
    1,    0,    5,    6,    7,    0,    1,    3,    1,
};
short yydefred[] = {                                      1,
    0,   34,   35,   62,   61,   59,   60,   69,    0,   36,
    0,    0,    0,    0,    0,   10,   18,    0,   20,    0,
    2,    3,   40,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    8,    0,    0,    0,   23,   48,    0,
   58,    0,    0,   37,    0,    0,   21,   22,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    7,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    9,   25,   24,    0,    0,   15,   19,
   43,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,   46,   47,    0,   28,   30,    0,   17,    0,    0,
   26,    0,    0,    0,    0,    0,    0,    0,    5,    0,
    0,   16,    0,    0,   17,    0,   42,    0,    0,    0,
    0,   41,    0,    0,    0,   14,   27,    0,    0,   63,
    0,   29,   17,   17,    0,   64,   12,   13,   65,
};
short yydgoto[] = {                                       1,
   21,   22,   23,   24,   25,   26,   65,  113,   27,   28,
   45,   29,   30,  111,   31,   33,  104,   32,
};
short yysindex[] = {                                      0,
  -33,    0,    0,    0,    0,    0,    0,    0,   47,    0,
  -30,   53,  -30,   53,   31,    0,    0,   53,    0, -243,
    0,    0,    0,  401,  -27,  -29,  -28,  -19,    0,   -8,
    9,  -35,   23,    0,  424, -198,   -4,    0,    0,  -16,
    0,    0, -217,    0,   16,  152,    0,    0,   53,   53,
   53,   53,   53,   53,   53,   53,   53,   53,   53,   53,
    0,   53,  -36,   53,   38, -191, -187,  -20,   53,   41,
   33, -180,   53,    0,    0,    0,   53,   53,    0,    0,
    0,  -67,  301, -258, -258, -258, -258, -258, -258, -212,
 -212,    0,    0,  248,    0,    0,  164,    0, -182,   38,
    0,   53,  248,    2, -173, -174,   40,    3,    0,  248,
   53,    0, -175,  -29,    0,  176,    0,   53,   46,   38,
 -165,    0,  189,   38,   38,    0,    0,  248, -164,    0,
   38,    0,    0,    0,   38,    0,    0,    0,    0,
};
short yyrindex[] = {                                      0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,  104,    0,    0,    0,  116,    0,
  128,    0,    0,    0,    0,    0,  140,    0,    0,    0,
    0,   68,   88,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    4,    0,
    0,    0,    4,    0,    0,    0,    0,    0,    0,    0,
    0,  -26,  -32,  235,  271,  295,  314,  456,  475,  201,
  212,    0,    0,  -31,    0,    0,    0,    0,    0,    0,
    0,    0,   13,    0,    0,    0,    0,    0,    0,   -9,
    0,    0,    1,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,   14,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,
};
short yygindex[] = {                                      0,
   54,    0,    0,  545,   51,    0,  -18,  -94,    0,    0,
    0,   56,    0,    0,    0,    0,   25,    0,
};
#define YYTABLESIZE 754
short yytable[] = {                                      72,
   11,   20,   15,   96,   20,   36,   18,   66,   56,    6,
   64,   56,    6,   47,   57,   48,   67,   57,   63,  102,
  126,   57,   58,   59,   60,   16,   56,    6,   71,   63,
   19,    4,   57,   19,    4,   11,   11,   68,  137,  138,
   11,   63,  117,  122,   66,  118,  118,   66,   69,    4,
   20,   15,   78,   67,   68,   18,   67,   68,   75,   11,
   76,   37,   73,   40,   11,   44,   38,   99,   38,   59,
   60,  100,   20,   15,   16,  105,  107,   18,  106,   19,
  114,   20,   15,  119,  120,  121,   18,   20,   15,   17,
  124,  129,   18,  131,  135,  125,   16,  108,   80,    0,
    0,   19,    0,    0,    0,   34,    0,   33,   33,    0,
   19,   33,    0,   25,    0,    0,   19,    0,   98,    0,
    0,    0,    0,   11,    0,   11,   33,   32,   32,    0,
    0,   32,    0,   24,    0,    0,    0,    0,   17,    0,
   79,    0,    0,   31,   31,    0,   32,   31,    0,    0,
    0,    0,    0,  115,    0,    0,   38,    0,    0,   38,
   17,   23,   31,    0,    0,    0,    0,    0,   39,    0,
    0,   39,    0,  130,   38,    0,    0,  133,  134,    0,
   49,    0,    0,   49,  136,    0,   39,    0,  139,    0,
    0,    0,   81,    0,    0,    0,    0,    0,   49,    0,
    0,    0,    0,    0,  112,   50,   51,   52,   53,   54,
   55,   56,   57,   58,   59,   60,  127,    0,    0,    0,
    0,    0,   95,    2,    3,    0,    0,    4,    5,  132,
    6,    7,    0,    8,    9,   10,    0,   11,  101,   56,
   56,   44,   62,    0,   44,   57,   70,   12,   13,    0,
    0,   14,   45,   77,    0,   45,    0,   11,   11,   44,
    0,   11,   11,    0,   11,   11,    0,   11,   11,   11,
   45,   11,    2,    3,    0,   54,    4,    5,   54,    6,
    7,   11,   11,    9,   10,   11,   11,   42,    0,   43,
    0,    0,    0,   54,    2,    3,   12,   13,    4,    5,
   14,    6,    7,    2,    3,    9,   10,    4,   11,    2,
    3,   55,    0,    4,   55,   10,    0,   11,   12,   13,
    0,   10,   14,   11,    0,    0,    0,   12,   13,   55,
    0,   14,    0,   12,   13,   52,    0,   14,   52,   33,
   33,   33,   33,   33,   33,   33,   33,   33,   33,   33,
   33,    0,    0,   52,   53,    0,    0,   53,    0,   32,
   32,   32,   32,   32,   32,   32,   32,   32,   32,   32,
   32,    0,   53,    0,    0,   31,   31,   31,   31,   31,
   31,   31,   31,   31,   31,   31,   31,   38,   38,   38,
   38,   38,   38,   38,   38,   38,   38,   38,   38,   39,
   39,   39,   39,   39,   39,   39,   39,   39,   39,   39,
   39,   49,   49,   49,   49,   49,   49,   49,   49,   49,
   49,   49,   49,   49,   50,   51,   52,   53,   54,   55,
   56,   57,   58,   59,   60,   49,   50,   51,   52,   53,
   54,   55,   56,   57,   58,   59,   60,   49,   50,   51,
   52,   53,   54,   55,   56,   57,   58,   59,   60,   61,
   49,   50,   51,   52,   53,   54,   55,   56,   57,   58,
   59,   60,   44,   44,   44,   44,   44,   44,   44,   44,
   44,   44,   74,   45,   45,   45,   45,   45,   45,   45,
   45,   45,   45,    0,    0,    0,   50,    0,    0,   50,
    0,    0,    0,    0,    0,    0,   54,   54,   54,   54,
   54,   54,   54,   54,   50,   51,    0,    0,   51,   49,
   50,   51,   52,   53,   54,   55,   56,   57,   58,   59,
   60,    0,    0,   51,    0,    0,    0,    0,    0,    0,
    0,    0,   55,   55,   55,   55,   55,   55,   55,   55,
    0,    0,    0,   35,    0,    0,   39,    0,   41,    0,
    0,    0,   46,    0,    0,    0,   52,   52,   52,   52,
   52,   52,   52,   52,   51,   52,   53,   54,   55,   56,
   57,   58,   59,   60,    0,   53,   53,   53,   53,   53,
   53,   53,   53,   82,   83,   84,   85,   86,   87,   88,
   89,   90,   91,   92,   93,    0,   94,    0,   97,    0,
    0,    0,    0,  103,    0,    0,    0,  103,    0,    0,
    0,  109,  110,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,  116,    0,    0,    0,
    0,    0,    0,    0,    0,  123,    0,    0,    0,    0,
    0,    0,  128,    0,    0,    0,    0,    0,    0,    0,
    0,    0,   49,   50,   51,   52,   53,   54,   55,   56,
   57,   58,   59,   60,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,   49,   50,   51,   52,   53,
   54,   55,   56,   57,   58,   59,   60,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,   50,   50,   50,
   50,   50,   50,   50,   50,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,   51,   51,   51,   51,
   51,   51,   51,   51,
};
short yycheck[] = {                                      35,
    0,   35,   36,   40,   35,   36,   40,   36,   41,   41,
   40,   44,   44,  257,   41,  259,   36,   44,   46,   40,
  115,  280,  281,  282,  283,   59,   59,   59,   64,   46,
   64,   41,   59,   64,   44,   35,   36,   46,  133,  134,
   40,   46,   41,   41,   41,   44,   44,   44,   40,   59,
   35,   36,  270,   41,   41,   40,   44,   44,  257,   59,
  259,   11,   40,   13,   64,   35,   11,  259,   13,  282,
  283,  259,   35,   36,   59,   35,  257,   40,   46,   64,
  263,   35,   36,  257,  259,   46,   40,   35,   36,  123,
  266,   46,   40,  259,  259,  114,   59,   73,   45,   -1,
   -1,   64,   -1,   -1,   -1,   59,   -1,   40,   41,   -1,
   64,   44,   -1,   46,   -1,   -1,   64,   -1,   65,   -1,
   -1,   -1,   -1,  123,   -1,  125,   59,   40,   41,   -1,
   -1,   44,   -1,   46,   -1,   -1,   -1,   -1,  123,   -1,
  125,   -1,   -1,   40,   41,   -1,   59,   44,   -1,   -1,
   -1,   -1,   -1,  100,   -1,   -1,   41,   -1,   -1,   44,
  123,   46,   59,   -1,   -1,   -1,   -1,   -1,   41,   -1,
   -1,   44,   -1,  120,   59,   -1,   -1,  124,  125,   -1,
   41,   -1,   -1,   44,  131,   -1,   59,   -1,  135,   -1,
   -1,   -1,   41,   -1,   -1,   -1,   -1,   -1,   59,   -1,
   -1,   -1,   -1,   -1,   41,  273,  274,  275,  276,  277,
  278,  279,  280,  281,  282,  283,   41,   -1,   -1,   -1,
   -1,   -1,  259,  257,  258,   -1,   -1,  261,  262,   41,
  264,  265,   -1,  267,  268,  269,   -1,  271,  259,  272,
  273,   41,  270,   -1,   44,  272,  282,  281,  282,   -1,
   -1,  285,   41,  270,   -1,   44,   -1,  257,  258,   59,
   -1,  261,  262,   -1,  264,  265,   -1,  267,  268,  269,
   59,  271,  257,  258,   -1,   41,  261,  262,   44,  264,
  265,  281,  282,  268,  269,  285,  271,  257,   -1,  259,
   -1,   -1,   -1,   59,  257,  258,  281,  282,  261,  262,
  285,  264,  265,  257,  258,  268,  269,  261,  271,  257,
  258,   41,   -1,  261,   44,  269,   -1,  271,  281,  282,
   -1,  269,  285,  271,   -1,   -1,   -1,  281,  282,   59,
   -1,  285,   -1,  281,  282,   41,   -1,  285,   44,  272,
  273,  274,  275,  276,  277,  278,  279,  280,  281,  282,
  283,   -1,   -1,   59,   41,   -1,   -1,   44,   -1,  272,
  273,  274,  275,  276,  277,  278,  279,  280,  281,  282,
  283,   -1,   59,   -1,   -1,  272,  273,  274,  275,  276,
  277,  278,  279,  280,  281,  282,  283,  272,  273,  274,
  275,  276,  277,  278,  279,  280,  281,  282,  283,  272,
  273,  274,  275,  276,  277,  278,  279,  280,  281,  282,
  283,  272,  273,  274,  275,  276,  277,  278,  279,  280,
  281,  282,  283,  272,  273,  274,  275,  276,  277,  278,
  279,  280,  281,  282,  283,  272,  273,  274,  275,  276,
  277,  278,  279,  280,  281,  282,  283,  272,  273,  274,
  275,  276,  277,  278,  279,  280,  281,  282,  283,   59,
  272,  273,  274,  275,  276,  277,  278,  279,  280,  281,
  282,  283,  272,  273,  274,  275,  276,  277,  278,  279,
  280,  281,   59,  272,  273,  274,  275,  276,  277,  278,
  279,  280,  281,   -1,   -1,   -1,   41,   -1,   -1,   44,
   -1,   -1,   -1,   -1,   -1,   -1,  272,  273,  274,  275,
  276,  277,  278,  279,   59,   41,   -1,   -1,   44,  272,
  273,  274,  275,  276,  277,  278,  279,  280,  281,  282,
  283,   -1,   -1,   59,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,  272,  273,  274,  275,  276,  277,  278,  279,
   -1,   -1,   -1,    9,   -1,   -1,   12,   -1,   14,   -1,
   -1,   -1,   18,   -1,   -1,   -1,  272,  273,  274,  275,
  276,  277,  278,  279,  274,  275,  276,  277,  278,  279,
  280,  281,  282,  283,   -1,  272,  273,  274,  275,  276,
  277,  278,  279,   49,   50,   51,   52,   53,   54,   55,
   56,   57,   58,   59,   60,   -1,   62,   -1,   64,   -1,
   -1,   -1,   -1,   69,   -1,   -1,   -1,   73,   -1,   -1,
   -1,   77,   78,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,  102,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,  111,   -1,   -1,   -1,   -1,
   -1,   -1,  118,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,  272,  273,  274,  275,  276,  277,  278,  279,
  280,  281,  282,  283,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,  272,  273,  274,  275,  276,
  277,  278,  279,  280,  281,  282,  283,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,  272,  273,  274,
  275,  276,  277,  278,  279,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,  272,  273,  274,  275,
  276,  277,  278,  279,
};
#define YYFINAL 1
#ifndef YYDEBUG
#define YYDEBUG 0
#endif
#define YYMAXTOKEN 285
#if YYDEBUG
char *yyname[] = {
"end-of-file",0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,"'#'","'$'",0,0,0,"'('","')'",0,0,"','",0,"'.'",0,0,0,0,0,0,0,0,0,0,0,0,"';'",
0,0,0,0,"'@'",0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,"'{'",0,"'}'",0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,"NUM","STR","IDENT","OUTOFSPACE","BLTIN","IF","IN","FOR","FORARG","ELSE",
"FUNC","RETURN","VNULL","ASGN","REF","OR","AND","GT","GTE","LT","LTE","EQ","NE",
"ADD","SUB","MUL","DIV","UNARY","NOT",
};
char *yyrule[] = {
"$accept : program",
"program :",
"program : program statement",
"program : program funcdef",
"asgn : '$' IDENT ASGN expr",
"asgn : MUL element ASGN expr",
"asgn : element ASGN expr",
"statement : expr ';'",
"statement : RETURN ';'",
"statement : RETURN expr ';'",
"statement : ';'",
"statement : ifheader condition statement stop",
"statement : ifheader condition statement stop ELSE statement stop",
"statement : forheader '$' IDENT IN condition statement stop",
"statement : forargheader '$' IDENT statement stop",
"statement : '{' statements '}'",
"condition : '(' expr ')'",
"stop :",
"statements :",
"statements : statements statement",
"refnum : '@'",
"refnum : '#' NUM",
"refnum : '#' IDENT",
"element_head : refnum",
"element_head : '$' IDENT",
"element_head : '$' NUM",
"element : element_head '.' IDENT",
"element : element_head '.' '(' expr ')'",
"element : element '.' IDENT",
"element : element '.' '(' push_eval_kludge expr ')'",
"push_eval_kludge :",
"evaluated_element : element",
"evaluated_element : '$' IDENT",
"evaluated_element : '$' NUM",
"expr : NUM",
"expr : STR",
"expr : VNULL",
"expr : '$' '#'",
"expr : refnum",
"expr : evaluated_element",
"expr : asgn",
"expr : BLTIN beginargs '(' arglist ')'",
"expr : evaluated_element '(' arglist ')'",
"expr : '(' expr ')'",
"expr : expr ADD expr",
"expr : expr SUB expr",
"expr : expr MUL expr",
"expr : expr DIV expr",
"expr : SUB expr",
"expr : REF element",
"expr : expr EQ expr",
"expr : expr NE expr",
"expr : expr LT expr",
"expr : expr LTE expr",
"expr : expr GT expr",
"expr : expr GTE expr",
"expr : expr AND expr",
"expr : expr OR expr",
"expr : NOT expr",
"forheader : FOR",
"forargheader : FORARG",
"ifheader : IF",
"beginargs :",
"funcdef : funcheader '@' '.' IDENT statement",
"funcdef : funcheader '#' NUM '.' IDENT statement",
"funcdef : funcheader MUL '#' NUM '.' IDENT statement",
"arglist :",
"arglist : expr",
"arglist : arglist ',' expr",
"funcheader : FUNC",
};
#endif
#ifndef YYSTYPE
typedef int YYSTYPE;
#endif
#define yyclearin (yychar=(-1))
#define yyerrok (yyerrflag=0)
#ifdef YYSTACKSIZE
#ifndef YYMAXDEPTH
#define YYMAXDEPTH YYSTACKSIZE
#endif
#else
#ifdef YYMAXDEPTH
#define YYSTACKSIZE YYMAXDEPTH
#else
#define YYSTACKSIZE 500
#define YYMAXDEPTH 500
#endif
#endif
int yydebug;
int yynerrs;
int yyerrflag;
int yychar;
short *yyssp;
YYSTYPE *yyvsp;
YYSTYPE yyval;
YYSTYPE yylval;
short yyss[YYSTACKSIZE];
YYSTYPE yyvs[YYSTACKSIZE];
#define yystacksize YYSTACKSIZE
#line 773 "parse.y"
FILE	*yyin	= { stdin };

int	yylineno = 1;


void
setyyinput(f)
FILE	*f;
{
	yyin = f;
}


putinstringbuf(b)
char	*b;
{
	register char	*op = &stringbuf[stringoff];
	int		ooff = stringoff;

	while(*b && stringoff < sizeof(stringbuf)) {
		if(isprint(*b)) {
			*op++ = *b;
			stringoff++;
			if(stringoff >= sizeof(stringbuf))
				return(-1);
		}
		b++;
	}
	 stringoff++;
	*op = '\0';
	return(ooff);
}


assemble(p,op)
Prog	*p;
int	op;
{
	if(p->p_pc >= p->p_siz) {
		yyerror("program too big\n");
		compiler_state = COMPILE_OOE;
		return(0);
	}
	p->p_mem[p->p_pc] = op;
	p->p_pc++;
	return(p->p_pc - 1);
}



/*
return the element # of the currently defined function
*/
long
funcnum()
{
	return(currenum);
}



/*
return the assignment op type of the currently defined function
*/
funcop()
{
	return(currfuncop);
}




/*
return the name of the currently defined function
*/
char	*
funcname()
{
	return(currfunc);
}



/*
reset the string buffer pointers and so on. wipe the static memory
prior to a parser run.
*/
void
resetparser()
{
	infunct = 0;
	stringnum = 0;
	stringoff = 0;

	/* kludgy - reset the max size of the compiler's program buffer */
	ASMPC = 0;
	yaccprog->p_siz = INSBUFSIZ;
	yaccprog->s_siz = STRBUFSIZ;
	currfunc = 0;
	compiler_state = COMPILE_OK;
}



/*
search the string buffer for an existing copy of string. in case it is
already there, return the offset (cut down on duplication). if not,
return -1
*/
static	int
searchbuf(s)
char	*s;
{
	register	char	*p = stringbuf;
	register	char	*kp;
	int		strt;
	int		srch = 0;

	while(srch < stringoff) {
		kp = s;
		strt = srch;

		/* match as match can */
		while(*p == *kp && *kp != '\0' && srch < stringoff - 1) {
			p++;
			kp++;
			srch++;
		}

		/* is it a match ? */
		if(*p == '\0' && *kp == '\0' && srch < stringoff - 1) {
#ifdef	ASMDEBUG
			printf("string matched at %d\n",strt);
#endif
			return(strt);
		}

		/* it is not a match, skip to next NULL, and continue */
		while(*p != '\0') {
			p++;
			srch++;
		}

		/* and skip the NULL */
		srch++;
		if(srch < stringoff - 1)
			p++;
	}
	return(-1);
}



static	int
lookup(s,bval)
char	*s;
int	*bval;
{
	int	start	= 0;
	int	ret;

	static	struct	kwordz{
		char	*kw;
		int	rval;
		int	bltin;		/* # of builtin if builtin */
	} keyz[] = {
	/* MUST BE IN LEXICAL SORT ORDER !!!!!! */
	"NULL",			VNULL,		-1,
	"atoi",			BLTIN,		0,
	"atoobj",		BLTIN,		1,
	"catfile",		BLTIN,		2,
	"chmod",		BLTIN,		3,
	"chown",		BLTIN,		4,
	"closefile",            BLTIN,          42,
	"disconnect",		BLTIN,		5,
	"echo",			BLTIN,		6,
	"echoto",		BLTIN,		7,
	"else",			ELSE,		-1,
	"errno",		BLTIN,		8,
	"error",		BLTIN,		9,
	"foreach",		FOR,		-1,
	"foreacharg",		FORARG,		-1,
	"func",			FUNC,		-1,
	"geteuid",		BLTIN,		10,
	"getip",                BLTIN,          43,
	"getuid",		BLTIN,		11,
	"if",			IF,		-1,
	"in",			IN,		-1,
	"islist",		BLTIN,		12,
	"isnum",		BLTIN,		13,
	"isobj",		BLTIN,		14,
	"isstr",		BLTIN,		15,
	"listadd",		BLTIN,		16,
	"listappend",		BLTIN,		17,
	"listcount",		BLTIN,		18,
	"listdrop",		BLTIN,		19,
	"listelem",		BLTIN,		20,
	"listmerge",		BLTIN,		21,
	"listnew",		BLTIN,		22,
	"listprepend",		BLTIN,		16,	/* alias add/prepend */
	"listsearch",		BLTIN,		23,
	"listsetelem",		BLTIN,		24,
	"log",			BLTIN,		25,
	"match",		BLTIN,		26,
	"objectdestroy",	BLTIN,		27,
	"objectelements",	BLTIN,		28,
	"objectnew",		BLTIN,		29,
	"objectowner",		BLTIN,		30,
	"openfile",             BLTIN,          41,
	"random",		BLTIN,		31,
	"regcmp",		BLTIN,		32,
	"regexp",		BLTIN,		33,
	"return",		RETURN,		-1,
	"setruid",		BLTIN,		34,
	"setuid",		BLTIN,		35,
	"shutdown",		BLTIN,		36,
	"str",			BLTIN,		37,
	"strlen",		BLTIN,		38,
	"strtime",		BLTIN,		39,
	"time",			BLTIN,		40,
	0,0,0
	};

	int	end	= (sizeof(keyz)/sizeof(struct kwordz)) - 2;
	int	p	= end/2;

	*bval = -1;
	while(start <= end) {
		ret = strcmp(s,keyz[p].kw);
		if(ret == 0) {
			*bval = keyz[p].bltin;
			return(keyz[p].rval);
		}
		if(ret > 0)
			start = p + 1;
		else
			end = p - 1;

		p = start + ((end - start)/2);
	}
	return(-1);
}



yylex()
{
	char	in[BUFSIZ];
	char	*p = in;
	int	c;

	/* handle whitespace */
	while(isspace(c = fgetc(yyin)))
		if(c == '\n')
			yylineno++;
	
	/* handle EOF */
	if(c == EOF) {
		return(0);
	}

	/* save current char - it is valuable */
	*p++ = c;

	/* handle NUM */
	if(isdigit(c)) {
		int	num;

		num = c - '0';
		while(isdigit(c = fgetc(yyin)))
			num = (num * 10) + (c - '0');

		(void)ungetc(c,yyin);
		if(c == '\n')
			yylineno--;

		yylval = num;
		return(NUM);
	}

	/* handle keywords or idents/builtins */
	if(isalpha(c) || c == '_') {
		int	cnt = 0;
		int	rv;
		int	bltin;

		while((c = fgetc(yyin)) != EOF && (isalnum(c) || c == '_')) {
			if(++cnt + 1 >= MAXIDENTLEN)
				yyerror("identifier too long");
			*p++ = c;
		}

		(void)ungetc(c,yyin);

		*p = '\0';

		if((rv = lookup(in,&bltin)) != -1) {
			if(rv == BLTIN)
				yylval = bltin;
			return(rv);
		}
	
		/* if the string is already in the buffer, return that */
		if(stringoff > 0 && (yylval = searchbuf(in)) != -1) {
			return(IDENT);
		}

		if(stringoff + cnt > sizeof(stringbuf)) {
			yyerror("out of string space!");
			compiler_state = COMPILE_OOS;
			return(OUTOFSPACE);
		}
		bcopy(in,&stringbuf[stringoff],cnt + 2);
		yylval = stringoff;
		stringoff += cnt + 2;
		stringbuf[stringoff - 1] = '\0';
		return(IDENT);
	}

	/* handle quoted strings */
	if(c == '\"' || c == '\'') {
		int	cnt = 0;
		int	quot = c;

		/* strip start quote by resetting ptr */
		p = in;

		/* match quoted strings */
		while((c = fgetc(yyin)) != EOF && c != quot) {
			if(!isascii(c))
				continue;

			if(++cnt + 1 >= sizeof(in))
				yyerror("string too long");

			/* we have to guard the line count */
			if(c == '\n')
				yylineno++;

			if(c == '\\') {
				int	b;
				switch(b = fgetc(yyin)) {
					case	EOF:
						yyerror("EOF in quoted string");
						break;

					case	't':
						*p++ = '\t';
						break;

					case	';':
						*p++ = MATCH_CHAR;
						break;

					case	'n':
						*p++ = '\n';
						break;

					case	'a':
						*p++ = '\a';
						break;

					default:
						*p++ = b;
				}
			} else {
				*p++ = c;
			}
		}

		if(c == EOF)
			yyerror("EOF in quoted string");

		*p = '\0';
	
		/* if the string is already in the buffer, return that */
		if(stringoff > 0 && (yylval = searchbuf(in)) != -1) {
			return(STR);
		}

		if(stringoff + cnt > sizeof(stringbuf)) {
			yyerror("out of string space!");
			compiler_state = COMPILE_OOS;
			return(OUTOFSPACE);
		}
		bcopy(in,&stringbuf[stringoff],cnt + 2);
		yylval = stringoff;
		stringoff += cnt + 2;
		stringbuf[stringoff - 1] = '\0';
		return(STR);
	}

	switch(c) {
		case	'=':
			{
				int	t;
				t = fgetc(yyin);
				if(t != '=') {
					(void)ungetc(t,yyin);
					return(ASGN);
				}
			}
			return(EQ);

		case	'>':
			{
				int	t;
				t = fgetc(yyin);
				if(t != '=') {
					(void)ungetc(t,yyin);
					return(GT);
				}
			}
			return(GTE);

		case	'<':
			{
				int	t;
				t = fgetc(yyin);
				if(t != '=') {
					(void)ungetc(t,yyin);
					return(LT);
				}
			}
			return(LTE);

		case	'!':
			{
				int	t;
				t = fgetc(yyin);
				if(t != '=') {
					(void)ungetc(t,yyin);
					return(NOT);
				}
			}
			return(NE);

		case	'&':
			{
				int	t;
				t = fgetc(yyin);
				if(t != '&') {
					(void)ungetc(t,yyin);
					return(REF);
				}
			}
			return(AND);

		case	'|':
			{
				int	t;
				t = fgetc(yyin);
				if(t != '|') {
					(void)ungetc(t,yyin);
					return(t);
				}
			}
			return(OR);

		case	'/':
			return(DIV);

		case	'*':
			return(MUL);

		case	'-':
			return(SUB);

		case	'+':
			return(ADD);
	}

	/* punt */
	if(c == '\n')
		yylineno++;
	return(c);
}
#line 938 "y.tab.c"
#define YYABORT goto yyabort
#define YYACCEPT goto yyaccept
#define YYERROR goto yyerrlab
int
yyparse()
{
    register int yym, yyn, yystate;
#if YYDEBUG
    register char *yys;
    extern char *getenv();

    if (yys = getenv("YYDEBUG"))
    {
        yyn = *yys;
        if (yyn >= '0' && yyn <= '9')
            yydebug = yyn - '0';
    }
#endif

    yynerrs = 0;
    yyerrflag = 0;
    yychar = (-1);

    yyssp = yyss;
    yyvsp = yyvs;
    *yyssp = yystate = 0;

yyloop:
    if (yyn = yydefred[yystate]) goto yyreduce;
    if (yychar < 0)
    {
        if ((yychar = yylex()) < 0) yychar = 0;
#if YYDEBUG
        if (yydebug)
        {
            yys = 0;
            if (yychar <= YYMAXTOKEN) yys = yyname[yychar];
            if (!yys) yys = "illegal-symbol";
            printf("yydebug: state %d, reading %d (%s)\n", yystate,
                    yychar, yys);
        }
#endif
    }
    if ((yyn = yysindex[yystate]) && (yyn += yychar) >= 0 &&
            yyn <= YYTABLESIZE && yycheck[yyn] == yychar)
    {
#if YYDEBUG
        if (yydebug)
            printf("yydebug: state %d, shifting to state %d\n",
                    yystate, yytable[yyn]);
#endif
        if (yyssp >= yyss + yystacksize - 1)
        {
            goto yyoverflow;
        }
        *++yyssp = yystate = yytable[yyn];
        *++yyvsp = yylval;
        yychar = (-1);
        if (yyerrflag > 0)  --yyerrflag;
        goto yyloop;
    }
    if ((yyn = yyrindex[yystate]) && (yyn += yychar) >= 0 &&
            yyn <= YYTABLESIZE && yycheck[yyn] == yychar)
    {
        yyn = yytable[yyn];
        goto yyreduce;
    }
    if (yyerrflag) goto yyinrecovery;
#ifdef lint
    goto yynewerror;
#endif
yynewerror:
    yyerror("syntax error");
#ifdef lint
    goto yyerrlab;
#endif
yyerrlab:
    ++yynerrs;
yyinrecovery:
    if (yyerrflag < 3)
    {
        yyerrflag = 3;
        for (;;)
        {
            if ((yyn = yysindex[*yyssp]) && (yyn += YYERRCODE) >= 0 &&
                    yyn <= YYTABLESIZE && yycheck[yyn] == YYERRCODE)
            {
#if YYDEBUG
                if (yydebug)
                    printf("yydebug: state %d, error recovery shifting\
 to state %d\n", *yyssp, yytable[yyn]);
#endif
                if (yyssp >= yyss + yystacksize - 1)
                {
                    goto yyoverflow;
                }
                *++yyssp = yystate = yytable[yyn];
                *++yyvsp = yylval;
                goto yyloop;
            }
            else
            {
#if YYDEBUG
                if (yydebug)
                    printf("yydebug: error recovery discarding state %d\n",
                            *yyssp);
#endif
                if (yyssp <= yyss) goto yyabort;
                --yyssp;
                --yyvsp;
            }
        }
    }
    else
    {
        if (yychar == 0) goto yyabort;
#if YYDEBUG
        if (yydebug)
        {
            yys = 0;
            if (yychar <= YYMAXTOKEN) yys = yyname[yychar];
            if (!yys) yys = "illegal-symbol";
            printf("yydebug: state %d, error recovery discards token %d (%s)\n",
                    yystate, yychar, yys);
        }
#endif
        yychar = (-1);
        goto yyloop;
    }
yyreduce:
#if YYDEBUG
    if (yydebug)
        printf("yydebug: state %d, reducing by rule %d (%s)\n",
                yystate, yyn, yyrule[yyn]);
#endif
    yym = yylen[yyn];
    yyval = yyvsp[1-yym];
    switch (yyn)
    {
case 2:
#line 95 "parse.y"
{
		(void)assemble(yaccprog,OP_STOP);

		/*
		sizes here are used only for error control 
		when running the assembled static program.
		they get fixed during resetparser(), which
		should get called in the main loop.
		*/
		yaccprog->p_siz = ASMPC;
		yaccprog->s_siz = stringoff;

		return(compiler_state);
	}
break;
case 3:
#line 110 "parse.y"
{
		/*
		this instruction will only be reached if an
		explicit return; or return(expr) is not reached
		first. it ensures that something (a non-value)
		will be left on the stack for posterity
		*/
#ifdef	COMPILEDEBUG
		printf("OP_RETF@%d\n",ASMPC);
#endif
		(void)assemble(yaccprog,OP_RETF);

		/*
		sizes here are used to calculate how much space
		is allocated if this prog gets copied into a
		static function. this is very important, as it
		ensures minimal space wastage. these values must
		get reset in resetparser(), during the main loop.
		*/
		yaccprog->p_siz = ASMPC;
		yaccprog->p_siz = ASMPC;
		yaccprog->s_siz = stringoff;
		infunct = 0;

		if(compiler_state == COMPILE_OK)
			return(COMPILE_FUNC);
		return(compiler_state);
	}
break;
case 4:
#line 144 "parse.y"
{
		yyval = yyvsp[0];
#ifdef	COMPILEDEBUG
		printf("OP_ASGN@%d\n",ASMPC);
#endif
		(void)assemble(yaccprog,OP_ASGN);
#ifdef	COMPILEDEBUG
		printf("%s@%d\n",&stringbuf[yyvsp[-2]],ASMPC);
#endif
		(void)assemble(yaccprog,yyvsp[-2]);
	}
break;
case 5:
#line 157 "parse.y"
{
		/* *#44.bar = expr; */
		yyval = yyvsp[-2];
#ifdef	COMPILEDEBUG
		printf("OP_ECASGN@%d\n",ASMPC);
#endif
		(void)assemble(yaccprog,OP_ECASGN);
	}
break;
case 6:
#line 167 "parse.y"
{
		/* #44.bar = expr; */
#ifdef	COMPILEDEBUG
		printf("OP_EASGN@%d\n",ASMPC);
#endif
		(void)assemble(yaccprog,OP_EASGN);
	}
break;
case 7:
#line 180 "parse.y"
{
#ifdef	COMPILEDEBUG
		printf("OP_POP@%d\n",ASMPC);
#endif
		(void)assemble(yaccprog,OP_POP);
	}
break;
case 8:
#line 187 "parse.y"
{
		if(!infunct) {
			yyerror(infuncmsg);
			compiler_state = COMPILE_BAD;
		} else {
#ifdef	COMPILEDEBUG
			printf("OP_RETNV@%d\n",ASMPC);
#endif
			yyval = assemble(yaccprog,OP_RETNV);
		}
	}
break;
case 9:
#line 199 "parse.y"
{
		yyval = yyvsp[-1];
		if(!infunct) {
			yyerror(infuncmsg);
			compiler_state = COMPILE_BAD;
		} else {
#ifdef	COMPILEDEBUG
			printf("OP_RETV@%d\n",ASMPC);
#endif
			(void)assemble(yaccprog,OP_RETV);
		}
	}
break;
case 11:
#line 213 "parse.y"
{
		yaccprog->p_mem[yyvsp[-3] + 1] = yyvsp[-1];	/* if part */
		yaccprog->p_mem[yyvsp[-3] + 2] = 0;	/* else part (none) */
		yaccprog->p_mem[yyvsp[-3] + 3] = yyvsp[0];	/* continuation */
#ifdef	COMPILEDEBUG
printf("IF@%d backpatch, ifpart:%d@%d, else:%d@%d, cont:%d@%d\n",
		yyvsp[-3], yaccprog->p_mem[yyvsp[-3]+1],yyvsp[-3]+1,yaccprog->p_mem[yyvsp[-3]+2],yyvsp[-3]+2,
		yaccprog->p_mem[yyvsp[-3]+3],yyvsp[-3]+3);
#endif
	}
break;
case 12:
#line 224 "parse.y"
{
		yaccprog->p_mem[yyvsp[-6] + 1] = yyvsp[-4];	/* if part */
		yaccprog->p_mem[yyvsp[-6] + 2] = yyvsp[-1];	/* else part */
		yaccprog->p_mem[yyvsp[-6] + 3] = yyvsp[0];	/* continuation */
#ifdef	COMPILEDEBUG
printf("IF@%d backpatch, ifpart:%d@%d, else:%d@%d, cont:%d@%d\n",
		yyvsp[-6], yaccprog->p_mem[yyvsp[-6]+1],yyvsp[-6]+1,yaccprog->p_mem[yyvsp[-6]+2],yyvsp[-6]+2,
		yaccprog->p_mem[yyvsp[-6]+3],yyvsp[-6]+3);
#endif
	}
break;
case 13:
#line 235 "parse.y"
{
		yaccprog->p_mem[yyvsp[-6] + 1] = yyvsp[-4];	/* iterator sym name */
		yaccprog->p_mem[yyvsp[-6] + 2] = yyvsp[-1];	/* do part */
		yaccprog->p_mem[yyvsp[-6] + 3] = yyvsp[0];	/* continuation */
#ifdef	COMPILEDEBUG
printf("FOR@%d backpatch, sym:%s@%d, do:%d@%d, cont:%d@%d\n",
		yyvsp[-6], &stringbuf[yyvsp[-4]],yyvsp[-6]+1,yaccprog->p_mem[yyvsp[-6]+2],yyvsp[-6]+2,
		yaccprog->p_mem[yyvsp[-6]+3],yyvsp[-6]+3);
#endif
	}
break;
case 14:
#line 246 "parse.y"
{
		yaccprog->p_mem[yyvsp[-4] + 1] = yyvsp[-2];	/* iterator sym name */
		yaccprog->p_mem[yyvsp[-4] + 2] = yyvsp[0];	/* continuation */
#ifdef	COMPILEDEBUG
printf("FORARG@%d backpatch, iter:%s@%d, cont:%d@%d\n",
		yyvsp[-4], &stringbuf[yyvsp[-2]],yyvsp[-4]+1,yaccprog->p_mem[yyvsp[-4]+2],yyvsp[-4]+2);
#endif
	}
break;
case 15:
#line 255 "parse.y"
{
		yyval = yyvsp[-1];
	}
break;
case 16:
#line 264 "parse.y"
{
		yyval = yyvsp[-1];
#ifdef	COMPILEDEBUG
		printf("OP_STOP@%d\n",ASMPC);
#endif
		(void)assemble(yaccprog,OP_STOP);
	}
break;
case 17:
#line 277 "parse.y"
{
#ifdef	COMPILEDEBUG
		printf("OP_STOP@%d\n",ASMPC);
#endif
		(void)assemble(yaccprog,OP_STOP);
		yyval = ASMPC;
	}
break;
case 18:
#line 290 "parse.y"
{
		yyval = ASMPC;
	}
break;
case 20:
#line 300 "parse.y"
{
#ifdef	COMPILEDEBUG
		printf("OP_ROOTPUSH@%d\n",ASMPC);
#endif
		yyval = assemble(yaccprog,OP_ROOTPUSH);
	}
break;
case 21:
#line 307 "parse.y"
{
#ifdef	COMPILEDEBUG
		printf("OP_RPUSH@%d\n",ASMPC);
#endif
		yyval = assemble(yaccprog,OP_RPUSH);
#ifdef	COMPILEDEBUG
		printf("%d@%d\n",yyvsp[0],ASMPC);
#endif
		(void)assemble(yaccprog,yyvsp[0]);
	}
break;
case 22:
#line 318 "parse.y"
{
		if(!strcmp(&stringbuf[yyvsp[0]],"self")) {
#ifdef	COMPILEDEBUG
			printf("OP_SELFPUSH@%d\n",ASMPC);
#endif
			yyval = assemble(yaccprog,OP_SELFPUSH);
		} else
		if(!strcmp(&stringbuf[yyvsp[0]],"caller")) {
#ifdef	COMPILEDEBUG
			printf("OP_CALLPUSH@%d\n",ASMPC);
#endif
			yyval = assemble(yaccprog,OP_CALLPUSH);
		} else
		if(!strcmp(&stringbuf[yyvsp[0]],"actor")) {
#ifdef	COMPILEDEBUG
			printf("OP_ACTPUSH@%d\n",ASMPC);
#endif
			yyval = assemble(yaccprog,OP_ACTPUSH);
		} else {
			yyerror("error. must specify self|actor|caller\n");
			compiler_state = COMPILE_BAD;
		}
	}
break;
case 24:
#line 348 "parse.y"
{
#ifdef	COMPILEDEBUG
		printf("OP_EVAL@%d\n",ASMPC);
#endif
		yyval = assemble(yaccprog,OP_EVAL);
#ifdef	COMPILEDEBUG
		printf("%s@%d\n",&stringbuf[yyvsp[0]],ASMPC);
#endif
		(void)assemble(yaccprog,yyvsp[0]);
	}
break;
case 25:
#line 359 "parse.y"
{
		if(!infunct) {
			yyerror(infuncmsg);
			compiler_state = COMPILE_BAD;
		} else {
#ifdef	COMPILEDEBUG
			printf("OP_PPUSH@%d\n",ASMPC);
#endif
			yyval = assemble(yaccprog,OP_PPUSH);
#ifdef	COMPILEDEBUG
			printf("%d@%d\n",yyvsp[0],ASMPC);
#endif
			(void)assemble(yaccprog,yyvsp[0]);
		}
	}
break;
case 26:
#line 380 "parse.y"
{
#ifdef	COMPILEDEBUG
		printf("OP_ELPUSH@%d\n",ASMPC);
#endif
		(void)assemble(yaccprog,OP_ELPUSH);
#ifdef	COMPILEDEBUG
		printf("%s@%d\n",&stringbuf[yyvsp[0]],ASMPC);
#endif
		(void)assemble(yaccprog,yyvsp[0]);
	}
break;
case 27:
#line 391 "parse.y"
{
#ifdef	COMPILEDEBUG
		printf("OP_STRVAR@%d\n",ASMPC);
#endif
		(void)assemble(yaccprog,OP_STRVAR);
	}
break;
case 28:
#line 398 "parse.y"
{
#ifdef	COMPILEDEBUG
		printf("OP_EEVAL@%d\n",ASMPC);
#endif
		(void)assemble(yaccprog,OP_EEVAL);
#ifdef	COMPILEDEBUG
		printf("OP_ELPUSH@%d\n",ASMPC);
#endif
		(void)assemble(yaccprog,OP_ELPUSH);
#ifdef	COMPILEDEBUG
		printf("%s@%d\n",&stringbuf[yyvsp[0]],ASMPC);
#endif
		(void)assemble(yaccprog,yyvsp[0]);
	}
break;
case 29:
#line 413 "parse.y"
{
#ifdef	COMPILEDEBUG
		printf("OP_STRVAR@%d\n",ASMPC);
#endif
		(void)assemble(yaccprog,OP_STRVAR);
	}
break;
case 30:
#line 430 "parse.y"
{
#ifdef	COMPILEDEBUG
		printf("OP_EEVAL@%d\n",ASMPC);
#endif
		(void)assemble(yaccprog,OP_EEVAL);
	}
break;
case 31:
#line 442 "parse.y"
{
#ifdef	COMPILEDEBUG
		printf("OP_EEVAL@%d\n",ASMPC);
#endif
		(void)assemble(yaccprog,OP_EEVAL);
	}
break;
case 32:
#line 449 "parse.y"
{
#ifdef	COMPILEDEBUG
		printf("OP_EVAL@%d\n",ASMPC);
#endif
		yyval = assemble(yaccprog,OP_EVAL);	/* replace w/ contents */
#ifdef	COMPILEDEBUG
		printf("%s@%d\n",&stringbuf[yyvsp[0]],ASMPC);
#endif
		(void)assemble(yaccprog,yyvsp[0]);	/* variable name */
	}
break;
case 33:
#line 460 "parse.y"
{
		if(!infunct) {
			yyerror(infuncmsg);
			compiler_state = COMPILE_BAD;
		} else {
#ifdef	COMPILEDEBUG
			printf("OP_PPUSH@%d\n",ASMPC);
#endif
			yyval = assemble(yaccprog,OP_PPUSH);
#ifdef	COMPILEDEBUG
			printf("%d@%d\n",yyvsp[0],ASMPC);
#endif
			(void)assemble(yaccprog,yyvsp[0]);
		}
	}
break;
case 34:
#line 480 "parse.y"
{
#ifdef	COMPILEDEBUG
		printf("OP_NPUSH@%d\n",ASMPC);
#endif
		yyval = assemble(yaccprog,OP_NPUSH);
#ifdef	COMPILEDEBUG
		printf("%d@%d\n",yyvsp[0],ASMPC);
#endif
		(void)assemble(yaccprog,yyvsp[0]);
	}
break;
case 35:
#line 491 "parse.y"
{
#ifdef	COMPILEDEBUG
		printf("OP_SPUSH@%d\n",ASMPC);
#endif
		yyval = assemble(yaccprog,OP_SPUSH);
#ifdef	COMPILEDEBUG
		printf("%s@%d\n",&stringbuf[yyvsp[0]],ASMPC);
#endif
		(void)assemble(yaccprog,yyvsp[0]);
	}
break;
case 36:
#line 502 "parse.y"
{
#ifdef	COMPILEDEBUG
		printf("OP_NULPUSH@%d\n",ASMPC);
#endif
		yyval = assemble(yaccprog,OP_NULPUSH);
	}
break;
case 37:
#line 509 "parse.y"
{
		if(!infunct) {
			yyerror(infuncmsg);
			compiler_state = COMPILE_BAD;
		} else
#ifdef	COMPILEDEBUG
			printf("OP_CPUSH@%d\n",ASMPC);
#endif
			yyval = assemble(yaccprog,OP_CPUSH);
	}
break;
case 41:
#line 523 "parse.y"
{
		yyval = yyvsp[-3];
#ifdef	COMPILEDEBUG
		printf("OP_BLTIN@%d\n",ASMPC);
#endif
		(void)assemble(yaccprog,OP_BLTIN);
#ifdef	COMPILEDEBUG
		printf("bltin #%d@%d\n",yyvsp[-4],ASMPC);
#endif
		(void)assemble(yaccprog,yyvsp[-4]);
#ifdef	COMPILEDEBUG
		printf("%d args@%d\n",yyvsp[-1],ASMPC);
#endif
		(void)assemble(yaccprog,yyvsp[-1]);
	}
break;
case 42:
#line 539 "parse.y"
{
#ifdef	COMPILEDEBUG
		printf("OP_CALL@%d\n",ASMPC);
#endif
		(void)assemble(yaccprog,OP_CALL);
#ifdef	COMPILEDEBUG
		printf("%d args@%d\n",yyvsp[-1],ASMPC);
#endif
		(void)assemble(yaccprog,yyvsp[-1]);
	}
break;
case 43:
#line 550 "parse.y"
{
		yyval = yyvsp[-1];
	}
break;
case 44:
#line 554 "parse.y"
{
#ifdef	COMPILEDEBUG
		printf("OP_ADD@%d\n",ASMPC);
#endif
		(void)assemble(yaccprog,OP_ADD);
	}
break;
case 45:
#line 561 "parse.y"
{
#ifdef	COMPILEDEBUG
		printf("OP_SUB@%d\n",ASMPC);
#endif
		(void)assemble(yaccprog,OP_SUB);
	}
break;
case 46:
#line 568 "parse.y"
{
#ifdef	COMPILEDEBUG
		printf("OP_MUL@%d\n",ASMPC);
#endif
		(void)assemble(yaccprog,OP_MUL);
	}
break;
case 47:
#line 575 "parse.y"
{
#ifdef	COMPILEDEBUG
		printf("OP_DIV@%d\n",ASMPC);
#endif
		(void)assemble(yaccprog,OP_DIV);
	}
break;
case 48:
#line 582 "parse.y"
{
		yyval = yyvsp[0];
#ifdef	COMPILEDEBUG
		printf("OP_NEG@%d\n",ASMPC);
#endif
		(void)assemble(yaccprog,OP_NEG);	/* negate top of stack */
	}
break;
case 49:
#line 590 "parse.y"
{
		yyval = yyvsp[0];
#ifdef	COMPILEDEBUG
		printf("OP_REF@%d\n",ASMPC);
#endif
		(void)assemble(yaccprog,OP_REF);
	}
break;
case 50:
#line 598 "parse.y"
{
#ifdef	COMPILEDEBUG
		printf("OP_EQ@%d\n",ASMPC);
#endif
		(void)assemble(yaccprog,OP_EQ);
	}
break;
case 51:
#line 605 "parse.y"
{
#ifdef	COMPILEDEBUG
		printf("OP_NE@%d\n",ASMPC);
#endif
		(void)assemble(yaccprog,OP_NE);
	}
break;
case 52:
#line 612 "parse.y"
{
#ifdef	COMPILEDEBUG
		printf("OP_LT@%d\n",ASMPC);
#endif
		(void)assemble(yaccprog,OP_LT);
	}
break;
case 53:
#line 619 "parse.y"
{
#ifdef	COMPILEDEBUG
		printf("OP_LTE@%d\n",ASMPC);
#endif
		(void)assemble(yaccprog,OP_LTE);
	}
break;
case 54:
#line 626 "parse.y"
{
#ifdef	COMPILEDEBUG
		printf("OP_GT@%d\n",ASMPC);
#endif
		(void)assemble(yaccprog,OP_GT);
	}
break;
case 55:
#line 633 "parse.y"
{
#ifdef	COMPILEDEBUG
		printf("OP_GTE@%d\n",ASMPC);
#endif
		(void)assemble(yaccprog,OP_GTE);
	}
break;
case 56:
#line 640 "parse.y"
{
#ifdef	COMPILEDEBUG
		printf("OP_AND@%d\n",ASMPC);
#endif
		(void)assemble(yaccprog,OP_AND);
	}
break;
case 57:
#line 647 "parse.y"
{
#ifdef	COMPILEDEBUG
		printf("OP_OR@%d\n",ASMPC);
#endif
		(void)assemble(yaccprog,OP_OR);
	}
break;
case 58:
#line 654 "parse.y"
{
		yyval = yyvsp[0];
#ifdef	COMPILEDEBUG
		printf("OP_NOT@%d\n",ASMPC);
#endif
		(void)assemble(yaccprog,OP_NOT);	/* invert top of stack */
	}
break;
case 59:
#line 667 "parse.y"
{
#ifdef	COMPILEDEBUG
		printf("OP_FOR (header only)@%d\n",ASMPC);
#endif
		yyval = assemble(yaccprog,OP_FOR);
		(void)assemble(yaccprog,OP_STOP);
		(void)assemble(yaccprog,OP_STOP);
		(void)assemble(yaccprog,OP_STOP);
	}
break;
case 60:
#line 682 "parse.y"
{
#ifdef	COMPILEDEBUG
		printf("OP_FORARG (header only)@%d\n",ASMPC);
#endif
		yyval = assemble(yaccprog,OP_FORARG);
		(void)assemble(yaccprog,OP_STOP);
		(void)assemble(yaccprog,OP_STOP);
	}
break;
case 61:
#line 696 "parse.y"
{
#ifdef	COMPILEDEBUG
		printf("OP_IF (header only)@%d\n",ASMPC);
#endif
		yyval = assemble(yaccprog,OP_IF);
		(void)assemble(yaccprog,OP_STOP);
		(void)assemble(yaccprog,OP_STOP);
		(void)assemble(yaccprog,OP_STOP);
	}
break;
case 62:
#line 711 "parse.y"
{
		yyval = ASMPC;
	}
break;
case 63:
#line 720 "parse.y"
{
		/*
		this is gross, but the best way to handle it.
		store a pointer to the name of the currently
		defined function, to allow easy linking. this
		COULD be passed back in the compiled program,
		but then it would have to be stripped out.
		so, do it like this. Sue me.
		*/
		currfunc = &stringbuf[yyvsp[-1]];
		currenum = (long)0;
	}
break;
case 64:
#line 733 "parse.y"
{
		currfunc = &stringbuf[yyvsp[-1]];
		currenum = (long)yyvsp[-3];
		currfuncop = OP_EASGN;
	}
break;
case 65:
#line 739 "parse.y"
{
		currfunc = &stringbuf[yyvsp[-1]];
		currenum = (long)yyvsp[-3];
		currfuncop = OP_ECASGN;
	}
break;
case 66:
#line 750 "parse.y"
{
		yyval = 0;
	}
break;
case 67:
#line 754 "parse.y"
{
		yyval = 1;
	}
break;
case 68:
#line 758 "parse.y"
{
		yyval = yyvsp[-2] + 1;
	}
break;
case 69:
#line 767 "parse.y"
{
		infunct = 1;
	}
break;
#line 1776 "y.tab.c"
    }
    yyssp -= yym;
    yystate = *yyssp;
    yyvsp -= yym;
    yym = yylhs[yyn];
    if (yystate == 0 && yym == 0)
    {
#if YYDEBUG
        if (yydebug)
            printf("yydebug: after reduction, shifting from state 0 to\
 state %d\n", YYFINAL);
#endif
        yystate = YYFINAL;
        *++yyssp = YYFINAL;
        *++yyvsp = yyval;
        if (yychar < 0)
        {
            if ((yychar = yylex()) < 0) yychar = 0;
#if YYDEBUG
            if (yydebug)
            {
                yys = 0;
                if (yychar <= YYMAXTOKEN) yys = yyname[yychar];
                if (!yys) yys = "illegal-symbol";
                printf("yydebug: state %d, reading %d (%s)\n",
                        YYFINAL, yychar, yys);
            }
#endif
        }
        if (yychar == 0) goto yyaccept;
        goto yyloop;
    }
    if ((yyn = yygindex[yym]) && (yyn += yystate) >= 0 &&
            yyn <= YYTABLESIZE && yycheck[yyn] == yystate)
        yystate = yytable[yyn];
    else
        yystate = yydgoto[yym];
#if YYDEBUG
    if (yydebug)
        printf("yydebug: after reduction, shifting from state %d \
to state %d\n", *yyssp, yystate);
#endif
    if (yyssp >= yyss + yystacksize - 1)
    {
        goto yyoverflow;
    }
    *++yyssp = yystate;
    *++yyvsp = yyval;
    goto yyloop;
yyoverflow:
    yyerror("yacc stack overflow");
yyabort:
    return (1);
yyaccept:
    return (0);
}

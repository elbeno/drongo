
/*  A Bison parser, made from syncheck.y with Bison version GNU Bison version 1.22
  */

#define YYBISON 1  /* Identify Bison output.  */

#define	NUM	258
#define	STR	259
#define	IDENT	260
#define	OUTOFSPACE	261
#define	BLTIN	262
#define	IF	263
#define	IN	264
#define	FOR	265
#define	FORARG	266
#define	ELSE	267
#define	FUNC	268
#define	RETURN	269
#define	VNULL	270
#define	ASGN	271
#define	REF	272
#define	OR	273
#define	AND	274
#define	GT	275
#define	GTE	276
#define	LT	277
#define	LTE	278
#define	EQ	279
#define	NE	280
#define	ADD	281
#define	SUB	282
#define	MUL	283
#define	DIV	284
#define	UNARY	285
#define	NOT	286

#line 1 "syncheck.y"


/*
Copyright(C) 1990, Marcus J. Ranum, All Rights Reserved.
This software may be freely used, modified, and redistributed,
as long as this copyright message is left intact, and this
software is not used to develop any commercial product, or used
in any product that is provided on a pay-for-use basis.
*/

#include	<stdio.h>
#include	<ctype.h>

/* turn this on to run files through your C-preprocessor */
#define	USECPP
#define	DEFAULTCPP	"/lib/cpp -P"

int	infunct = 0;

int	yylineno = 1;
int	yyerrcnt = 0;
int	yywarcnt = 0;

char	lexbuf[BUFSIZ];

char	*fname;
FILE	*yyin = stdin;


#ifndef YYLTYPE
typedef
  struct yyltype
    {
      int timestamp;
      int first_line;
      int first_column;
      int last_line;
      int last_column;
      char *text;
   }
  yyltype;

#define YYLTYPE yyltype
#endif

#ifndef YYSTYPE
#define YYSTYPE int
#endif
#include <stdio.h>

#ifndef __cplusplus
#ifndef __STDC__
#define const
#endif
#endif



#define	YYFINAL		138
#define	YYFLAG		-32768
#define	YYNTBASE	42

#define YYTRANSLATE(x) ((unsigned)(x) <= 286 ? yytranslate[x] : 55)

static const char yytranslate[] = {     0,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,    35,    32,     2,     2,     2,    39,
    40,     2,     2,    41,     2,    34,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,    36,     2,
     2,     2,     2,    33,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,    37,     2,    38,     2,     2,     2,     2,     2,
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
     2,     2,     2,     2,     2,     1,     2,     3,     4,     5,
     6,     7,     8,     9,    10,    11,    12,    13,    14,    15,
    16,    17,    18,    19,    20,    21,    22,    23,    24,    25,
    26,    27,    28,    29,    30,    31
};

#if YYDEBUG != 0
static const short yyprhs[] = {     0,
     0,     1,     4,     7,    10,    15,    20,    24,    30,    37,
    43,    48,    52,    56,    59,    61,    64,    67,    71,    73,
    77,    83,    90,    95,    99,   100,   103,   107,   109,   112,
   115,   118,   120,   123,   126,   130,   136,   140,   146,   150,
   152,   155,   158,   161,   163,   165,   167,   169,   171,   174,
   176,   181,   184,   189,   193,   197,   201,   205,   209,   212,
   215,   218,   222,   226,   230,   234,   238,   242,   246,   250,
   253,   254,   256,   260
};

static const short yyrhs[] = {    -1,
    42,    46,     0,    42,    44,     0,    42,     1,     0,    32,
     5,    16,    53,     0,    28,    51,    16,    53,     0,    51,
    16,    53,     0,    45,    33,    34,     5,    46,     0,    45,
    35,     3,    34,     5,    46,     0,    45,    35,     3,    34,
     1,     0,    45,    35,     3,     1,     0,    45,    35,     1,
     0,    45,    33,     1,     0,    45,     1,     0,    13,     0,
    53,    36,     0,    14,    36,     0,    14,    53,    36,     0,
    36,     0,     8,    48,    46,     0,     8,    48,    46,    12,
    46,     0,    10,    32,     5,     9,    48,    46,     0,    11,
    32,     5,    46,     0,    37,    47,    38,     0,     0,    47,
    46,     0,    39,    53,    40,     0,    33,     0,    35,     3,
     0,    35,     5,     0,    35,     1,     0,    49,     0,    32,
     5,     0,    32,     3,     0,    50,    34,     5,     0,    50,
    34,    39,    53,    40,     0,    51,    34,     5,     0,    51,
    34,    39,    53,    40,     0,    51,    34,     1,     0,    51,
     0,    32,     5,     0,    32,     3,     0,    32,     1,     0,
     3,     0,     4,     0,    15,     0,    52,     0,    49,     0,
    32,    35,     0,    43,     0,     7,    39,    54,    40,     0,
     7,     1,     0,    52,    39,    54,    40,     0,    39,    53,
    40,     0,    53,    26,    53,     0,    53,    27,    53,     0,
    53,    28,    53,     0,    53,    29,    53,     0,    27,    53,
     0,    17,    51,     0,    17,     1,     0,    53,    24,    53,
     0,    53,    25,    53,     0,    53,    19,    53,     0,    53,
    18,    53,     0,    53,    22,    53,     0,    53,    23,    53,
     0,    53,    20,    53,     0,    53,    21,    53,     0,    31,
    53,     0,     0,    53,     0,    54,    41,    53,     0,    54,
     1,     0
};

#endif

#if YYDEBUG != 0
static const short yyrline[] = { 0,
    46,    48,    49,    50,    55,    57,    58,    63,    68,    72,
    77,    82,    87,    92,   100,   108,   110,   111,   112,   113,
   114,   115,   116,   117,   122,   124,   129,   135,   137,   138,
   139,   147,   149,   150,   159,   161,   162,   163,   164,   173,
   175,   176,   181,   190,   192,   193,   194,   195,   196,   202,
   203,   204,   208,   209,   210,   211,   212,   213,   214,   215,
   216,   220,   221,   222,   223,   224,   225,   226,   227,   228,
   233,   235,   236,   237
};

static const char * const yytname[] = {   "$","error","$illegal.","NUM","STR",
"IDENT","OUTOFSPACE","BLTIN","IF","IN","FOR","FORARG","ELSE","FUNC","RETURN",
"VNULL","ASGN","REF","OR","AND","GT","GTE","LT","LTE","EQ","NE","ADD","SUB",
"MUL","DIV","UNARY","NOT","'$'","'@'","'.'","'#'","';'","'{'","'}'","'('","')'",
"','","program","asgn","funcdef","funcheader","statement","statements","condition",
"refnum","element_head","element","evaluated_element","expr","arglist",""
};
#endif

static const short yyr1[] = {     0,
    42,    42,    42,    42,    43,    43,    43,    44,    44,    44,
    44,    44,    44,    44,    45,    46,    46,    46,    46,    46,
    46,    46,    46,    46,    47,    47,    48,    49,    49,    49,
    49,    50,    50,    50,    51,    51,    51,    51,    51,    52,
    52,    52,    52,    53,    53,    53,    53,    53,    53,    53,
    53,    53,    53,    53,    53,    53,    53,    53,    53,    53,
    53,    53,    53,    53,    53,    53,    53,    53,    53,    53,
    54,    54,    54,    54
};

static const short yyr2[] = {     0,
     0,     2,     2,     2,     4,     4,     3,     5,     6,     5,
     4,     3,     3,     2,     1,     2,     2,     3,     1,     3,
     5,     6,     4,     3,     0,     2,     3,     1,     2,     2,
     2,     1,     2,     2,     3,     5,     3,     5,     3,     1,
     2,     2,     2,     1,     1,     1,     1,     1,     2,     1,
     4,     2,     4,     3,     3,     3,     3,     3,     2,     2,
     2,     3,     3,     3,     3,     3,     3,     3,     3,     2,
     0,     1,     3,     2
};

static const short yydefact[] = {     1,
     0,     4,    44,    45,     0,     0,     0,     0,    15,     0,
    46,     0,     0,     0,     0,     0,    28,     0,    19,    25,
     0,    50,     3,     0,     2,    48,     0,    40,    47,     0,
    52,    71,     0,     0,     0,     0,    17,     0,    61,     0,
    32,    60,    59,     0,    70,    43,    42,    41,    49,    31,
    29,    30,     0,     0,    14,     0,     0,     0,     0,     0,
    71,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     0,     0,     0,    16,    72,     0,     0,    20,     0,     0,
    18,    34,    33,     0,     0,    24,    26,    54,    13,     0,
    12,     0,    35,     0,     7,    39,    37,     0,     0,    65,
    64,    68,    69,    66,    67,    62,    63,    55,    56,    57,
    58,    74,    51,     0,    27,     0,     0,    23,     6,     5,
     0,    11,     0,     0,     0,    53,    73,    21,     0,     8,
    10,     0,    36,    38,    22,     9,     0,     0
};

static const short yydefgoto[] = {     1,
    22,    23,    24,    25,    53,    34,    26,    27,    28,    29,
    30,    76
};

static const short yypact[] = {-32768,
   107,-32768,-32768,-32768,     6,   -15,    -1,    35,-32768,   181,
-32768,     1,   215,    93,   215,    12,-32768,    24,-32768,-32768,
   215,-32768,-32768,    15,-32768,    37,    44,    -6,    -4,   329,
-32768,   215,   215,    62,    81,    86,-32768,   348,-32768,    16,
-32768,    58,-32768,    -2,-32768,-32768,    69,     4,-32768,-32768,
-32768,-32768,   144,   237,-32768,     5,    84,    63,   215,     7,
   215,   215,   215,   215,   215,   215,   215,   215,   215,   215,
   215,   215,   215,-32768,   367,     0,   260,    94,    87,    62,
-32768,-32768,-32768,   215,   215,-32768,-32768,-32768,-32768,   104,
-32768,     8,-32768,   215,   367,-32768,-32768,   215,     3,   378,
   247,    54,    54,    54,    54,    54,    54,   108,   108,-32768,
-32768,-32768,-32768,   215,-32768,    62,   -15,-32768,-32768,   367,
    62,-32768,    25,   283,   306,-32768,   367,-32768,    62,-32768,
-32768,    62,-32768,-32768,-32768,-32768,   112,-32768
};

static const short yypgoto[] = {-32768,
-32768,-32768,-32768,   -16,-32768,     2,   115,-32768,   118,-32768,
   -10,    70
};


#define	YYLAST		407


static const short yytable[] = {    38,
   112,    39,    43,   112,    45,    89,    31,    96,   122,    59,
    54,    97,    46,    84,    47,    55,    48,    78,    82,    85,
    83,    75,    77,    33,    50,   131,    51,    60,    52,   132,
    35,    60,    40,    17,    61,    18,    87,   -33,    90,   113,
   114,   123,   126,   114,    32,    98,    49,    56,    95,    57,
    75,   100,   101,   102,   103,   104,   105,   106,   107,   108,
   109,   110,   111,   118,     3,     4,    36,    93,     5,     6,
   -32,     7,     8,   119,   120,    10,    11,    58,    12,    70,
    71,    72,    73,   124,    91,    79,    92,   125,    13,    14,
    80,    60,    15,    16,    17,   117,    18,    19,    20,   128,
    21,    94,   -34,   127,   130,   116,   137,     2,   121,     3,
     4,   138,   135,     5,     6,   136,     7,     8,   129,     9,
    10,    11,     0,    12,    40,    17,    41,    18,    41,    42,
    99,    44,     0,    13,    14,    72,    73,    15,    16,    17,
     0,    18,    19,    20,     0,    21,     3,     4,     0,     0,
     5,     6,     0,     7,     8,     0,     0,    10,    11,     0,
    12,     0,     0,     0,     0,     0,     0,     0,     0,     0,
    13,    14,     0,     0,    15,    16,    17,     0,    18,    19,
    20,    86,    21,     3,     4,     0,     0,     5,     0,     0,
     0,     0,     0,     0,     0,    11,     0,    12,     0,     0,
     0,     0,     0,     0,     0,     0,     0,    13,    14,     0,
     0,    15,    16,    17,     0,    18,    37,     3,     4,    21,
     0,     5,     0,     0,     0,     0,     0,     0,     0,    11,
     0,    12,     0,     0,     0,     0,     0,     0,     0,     0,
     0,    13,    14,     0,     0,    15,    16,    17,     0,    18,
     0,     0,     0,    21,    62,    63,    64,    65,    66,    67,
    68,    69,    70,    71,    72,    73,    64,    65,    66,    67,
    68,    69,    70,    71,    72,    73,    88,    62,    63,    64,
    65,    66,    67,    68,    69,    70,    71,    72,    73,     0,
     0,     0,     0,     0,     0,     0,     0,     0,     0,   115,
    62,    63,    64,    65,    66,    67,    68,    69,    70,    71,
    72,    73,     0,     0,     0,     0,     0,     0,     0,     0,
     0,     0,   133,    62,    63,    64,    65,    66,    67,    68,
    69,    70,    71,    72,    73,     0,     0,     0,     0,     0,
     0,     0,     0,     0,     0,   134,    62,    63,    64,    65,
    66,    67,    68,    69,    70,    71,    72,    73,     0,     0,
     0,     0,     0,     0,    74,    62,    63,    64,    65,    66,
    67,    68,    69,    70,    71,    72,    73,     0,     0,     0,
     0,     0,     0,    81,    62,    63,    64,    65,    66,    67,
    68,    69,    70,    71,    72,    73,    63,    64,    65,    66,
    67,    68,    69,    70,    71,    72,    73
};

static const short yycheck[] = {    10,
     1,     1,    13,     1,    15,     1,     1,     1,     1,    16,
    21,     5,     1,    16,     3,     1,     5,    34,     3,    16,
     5,    32,    33,    39,     1,     1,     3,    34,     5,     5,
    32,    34,    32,    33,    39,    35,    53,    34,    34,    40,
    41,    34,    40,    41,    39,    39,    35,    33,    59,    35,
    61,    62,    63,    64,    65,    66,    67,    68,    69,    70,
    71,    72,    73,    80,     3,     4,    32,     5,     7,     8,
    34,    10,    11,    84,    85,    14,    15,    34,    17,    26,
    27,    28,    29,    94,     1,     5,     3,    98,    27,    28,
     5,    34,    31,    32,    33,     9,    35,    36,    37,   116,
    39,    39,    34,   114,   121,    12,     0,     1,     5,     3,
     4,     0,   129,     7,     8,   132,    10,    11,   117,    13,
    14,    15,    -1,    17,    32,    33,    12,    35,    14,    12,
    61,    14,    -1,    27,    28,    28,    29,    31,    32,    33,
    -1,    35,    36,    37,    -1,    39,     3,     4,    -1,    -1,
     7,     8,    -1,    10,    11,    -1,    -1,    14,    15,    -1,
    17,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
    27,    28,    -1,    -1,    31,    32,    33,    -1,    35,    36,
    37,    38,    39,     3,     4,    -1,    -1,     7,    -1,    -1,
    -1,    -1,    -1,    -1,    -1,    15,    -1,    17,    -1,    -1,
    -1,    -1,    -1,    -1,    -1,    -1,    -1,    27,    28,    -1,
    -1,    31,    32,    33,    -1,    35,    36,     3,     4,    39,
    -1,     7,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    15,
    -1,    17,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
    -1,    27,    28,    -1,    -1,    31,    32,    33,    -1,    35,
    -1,    -1,    -1,    39,    18,    19,    20,    21,    22,    23,
    24,    25,    26,    27,    28,    29,    20,    21,    22,    23,
    24,    25,    26,    27,    28,    29,    40,    18,    19,    20,
    21,    22,    23,    24,    25,    26,    27,    28,    29,    -1,
    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    40,
    18,    19,    20,    21,    22,    23,    24,    25,    26,    27,
    28,    29,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
    -1,    -1,    40,    18,    19,    20,    21,    22,    23,    24,
    25,    26,    27,    28,    29,    -1,    -1,    -1,    -1,    -1,
    -1,    -1,    -1,    -1,    -1,    40,    18,    19,    20,    21,
    22,    23,    24,    25,    26,    27,    28,    29,    -1,    -1,
    -1,    -1,    -1,    -1,    36,    18,    19,    20,    21,    22,
    23,    24,    25,    26,    27,    28,    29,    -1,    -1,    -1,
    -1,    -1,    -1,    36,    18,    19,    20,    21,    22,    23,
    24,    25,    26,    27,    28,    29,    19,    20,    21,    22,
    23,    24,    25,    26,    27,    28,    29
};
/* -*-C-*-  Note some compilers choke on comments on `#line' lines.  */
#line 3 "/usr/lib/bison.simple"

/* Skeleton output parser for bison,
   Copyright (C) 1984, 1989, 1990 Bob Corbett and Richard Stallman

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 1, or (at your option)
   any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.  */


#ifndef alloca
#ifdef __GNUC__
#define alloca __builtin_alloca
#else /* not GNU C.  */
#if (!defined (__STDC__) && defined (sparc)) || defined (__sparc__) || defined (__sparc) || defined (__sgi)
#include <alloca.h>
#else /* not sparc */
#if defined (MSDOS) && !defined (__TURBOC__)
#include <malloc.h>
#else /* not MSDOS, or __TURBOC__ */
#if defined(_AIX)
#include <malloc.h>
 #pragma alloca
#else /* not MSDOS, __TURBOC__, or _AIX */
#ifdef __hpux
#ifdef __cplusplus
extern "C" {
void *alloca (unsigned int);
};
#else /* not __cplusplus */
void *alloca ();
#endif /* not __cplusplus */
#endif /* __hpux */
#endif /* not _AIX */
#endif /* not MSDOS, or __TURBOC__ */
#endif /* not sparc.  */
#endif /* not GNU C.  */
#endif /* alloca not defined.  */

/* This is the parser code that is written into each bison parser
  when the %semantic_parser declaration is not specified in the grammar.
  It was written by Richard Stallman by simplifying the hairy parser
  used when %semantic_parser is specified.  */

/* Note: there must be only one dollar sign in this file.
   It is replaced by the list of actions, each action
   as one case of the switch.  */

#define yyerrok		(yyerrstatus = 0)
#define yyclearin	(yychar = YYEMPTY)
#define YYEMPTY		-2
#define YYEOF		0
#define YYACCEPT	return(0)
#define YYABORT 	return(1)
#define YYERROR		goto yyerrlab1
/* Like YYERROR except do call yyerror.
   This remains here temporarily to ease the
   transition to the new meaning of YYERROR, for GCC.
   Once GCC version 2 has supplanted version 1, this can go.  */
#define YYFAIL		goto yyerrlab
#define YYRECOVERING()  (!!yyerrstatus)
#define YYBACKUP(token, value) \
do								\
  if (yychar == YYEMPTY && yylen == 1)				\
    { yychar = (token), yylval = (value);			\
      yychar1 = YYTRANSLATE (yychar);				\
      YYPOPSTACK;						\
      goto yybackup;						\
    }								\
  else								\
    { yyerror ("syntax error: cannot back up"); YYERROR; }	\
while (0)

#define YYTERROR	1
#define YYERRCODE	256

#ifndef YYPURE
#define YYLEX		yylex()
#endif

#ifdef YYPURE
#ifdef YYLSP_NEEDED
#define YYLEX		yylex(&yylval, &yylloc)
#else
#define YYLEX		yylex(&yylval)
#endif
#endif

/* If nonreentrant, generate the variables here */

#ifndef YYPURE

int	yychar;			/*  the lookahead symbol		*/
YYSTYPE	yylval;			/*  the semantic value of the		*/
				/*  lookahead symbol			*/

#ifdef YYLSP_NEEDED
YYLTYPE yylloc;			/*  location data for the lookahead	*/
				/*  symbol				*/
#endif

int yynerrs;			/*  number of parse errors so far       */
#endif  /* not YYPURE */

#if YYDEBUG != 0
int yydebug;			/*  nonzero means print parse trace	*/
/* Since this is uninitialized, it does not stop multiple parsers
   from coexisting.  */
#endif

/*  YYINITDEPTH indicates the initial size of the parser's stacks	*/

#ifndef	YYINITDEPTH
#define YYINITDEPTH 200
#endif

/*  YYMAXDEPTH is the maximum size the stacks can grow to
    (effective only if the built-in stack extension method is used).  */

#if YYMAXDEPTH == 0
#undef YYMAXDEPTH
#endif

#ifndef YYMAXDEPTH
#define YYMAXDEPTH 10000
#endif

/* Prevent warning if -Wstrict-prototypes.  */
#ifdef __GNUC__
int yyparse (void);
#endif

#if __GNUC__ > 1		/* GNU C and GNU C++ define this.  */
#define __yy_bcopy(FROM,TO,COUNT)	__builtin_memcpy(TO,FROM,COUNT)
#else				/* not GNU C or C++ */
#ifndef __cplusplus

/* This is the most reliable way to avoid incompatibilities
   in available built-in functions on various systems.  */
static void
__yy_bcopy (from, to, count)
     char *from;
     char *to;
     int count;
{
  register char *f = from;
  register char *t = to;
  register int i = count;

  while (i-- > 0)
    *t++ = *f++;
}

#else /* __cplusplus */

/* This is the most reliable way to avoid incompatibilities
   in available built-in functions on various systems.  */
static void
__yy_bcopy (char *from, char *to, int count)
{
  register char *f = from;
  register char *t = to;
  register int i = count;

  while (i-- > 0)
    *t++ = *f++;
}

#endif
#endif

#line 184 "/usr/lib/bison.simple"
int
yyparse()
{
  register int yystate;
  register int yyn;
  register short *yyssp;
  register YYSTYPE *yyvsp;
  int yyerrstatus;	/*  number of tokens to shift before error messages enabled */
  int yychar1 = 0;		/*  lookahead token as an internal (translated) token number */

  short	yyssa[YYINITDEPTH];	/*  the state stack			*/
  YYSTYPE yyvsa[YYINITDEPTH];	/*  the semantic value stack		*/

  short *yyss = yyssa;		/*  refer to the stacks thru separate pointers */
  YYSTYPE *yyvs = yyvsa;	/*  to allow yyoverflow to reallocate them elsewhere */

#ifdef YYLSP_NEEDED
  YYLTYPE yylsa[YYINITDEPTH];	/*  the location stack			*/
  YYLTYPE *yyls = yylsa;
  YYLTYPE *yylsp;

#define YYPOPSTACK   (yyvsp--, yyssp--, yylsp--)
#else
#define YYPOPSTACK   (yyvsp--, yyssp--)
#endif

  int yystacksize = YYINITDEPTH;

#ifdef YYPURE
  int yychar;
  YYSTYPE yylval;
  int yynerrs;
#ifdef YYLSP_NEEDED
  YYLTYPE yylloc;
#endif
#endif

  YYSTYPE yyval;		/*  the variable used to return		*/
				/*  semantic values from the action	*/
				/*  routines				*/

  int yylen;

#if YYDEBUG != 0
  if (yydebug)
    fprintf(stderr, "Starting parse\n");
#endif

  yystate = 0;
  yyerrstatus = 0;
  yynerrs = 0;
  yychar = YYEMPTY;		/* Cause a token to be read.  */

  /* Initialize stack pointers.
     Waste one element of value and location stack
     so that they stay on the same level as the state stack.
     The wasted elements are never initialized.  */

  yyssp = yyss - 1;
  yyvsp = yyvs;
#ifdef YYLSP_NEEDED
  yylsp = yyls;
#endif

/* Push a new state, which is found in  yystate  .  */
/* In all cases, when you get here, the value and location stacks
   have just been pushed. so pushing a state here evens the stacks.  */
yynewstate:

  *++yyssp = yystate;

  if (yyssp >= yyss + yystacksize - 1)
    {
      /* Give user a chance to reallocate the stack */
      /* Use copies of these so that the &'s don't force the real ones into memory. */
      YYSTYPE *yyvs1 = yyvs;
      short *yyss1 = yyss;
#ifdef YYLSP_NEEDED
      YYLTYPE *yyls1 = yyls;
#endif

      /* Get the current used size of the three stacks, in elements.  */
      int size = yyssp - yyss + 1;

#ifdef yyoverflow
      /* Each stack pointer address is followed by the size of
	 the data in use in that stack, in bytes.  */
#ifdef YYLSP_NEEDED
      /* This used to be a conditional around just the two extra args,
	 but that might be undefined if yyoverflow is a macro.  */
      yyoverflow("parser stack overflow",
		 &yyss1, size * sizeof (*yyssp),
		 &yyvs1, size * sizeof (*yyvsp),
		 &yyls1, size * sizeof (*yylsp),
		 &yystacksize);
#else
      yyoverflow("parser stack overflow",
		 &yyss1, size * sizeof (*yyssp),
		 &yyvs1, size * sizeof (*yyvsp),
		 &yystacksize);
#endif

      yyss = yyss1; yyvs = yyvs1;
#ifdef YYLSP_NEEDED
      yyls = yyls1;
#endif
#else /* no yyoverflow */
      /* Extend the stack our own way.  */
      if (yystacksize >= YYMAXDEPTH)
	{
	  yyerror("parser stack overflow");
	  return 2;
	}
      yystacksize *= 2;
      if (yystacksize > YYMAXDEPTH)
	yystacksize = YYMAXDEPTH;
      yyss = (short *) alloca (yystacksize * sizeof (*yyssp));
      __yy_bcopy ((char *)yyss1, (char *)yyss, size * sizeof (*yyssp));
      yyvs = (YYSTYPE *) alloca (yystacksize * sizeof (*yyvsp));
      __yy_bcopy ((char *)yyvs1, (char *)yyvs, size * sizeof (*yyvsp));
#ifdef YYLSP_NEEDED
      yyls = (YYLTYPE *) alloca (yystacksize * sizeof (*yylsp));
      __yy_bcopy ((char *)yyls1, (char *)yyls, size * sizeof (*yylsp));
#endif
#endif /* no yyoverflow */

      yyssp = yyss + size - 1;
      yyvsp = yyvs + size - 1;
#ifdef YYLSP_NEEDED
      yylsp = yyls + size - 1;
#endif

#if YYDEBUG != 0
      if (yydebug)
	fprintf(stderr, "Stack size increased to %d\n", yystacksize);
#endif

      if (yyssp >= yyss + yystacksize - 1)
	YYABORT;
    }

#if YYDEBUG != 0
  if (yydebug)
    fprintf(stderr, "Entering state %d\n", yystate);
#endif

  goto yybackup;
 yybackup:

/* Do appropriate processing given the current state.  */
/* Read a lookahead token if we need one and don't already have one.  */
/* yyresume: */

  /* First try to decide what to do without reference to lookahead token.  */

  yyn = yypact[yystate];
  if (yyn == YYFLAG)
    goto yydefault;

  /* Not known => get a lookahead token if don't already have one.  */

  /* yychar is either YYEMPTY or YYEOF
     or a valid token in external form.  */

  if (yychar == YYEMPTY)
    {
#if YYDEBUG != 0
      if (yydebug)
	fprintf(stderr, "Reading a token: ");
#endif
      yychar = YYLEX;
    }

  /* Convert token to internal form (in yychar1) for indexing tables with */

  if (yychar <= 0)		/* This means end of input. */
    {
      yychar1 = 0;
      yychar = YYEOF;		/* Don't call YYLEX any more */

#if YYDEBUG != 0
      if (yydebug)
	fprintf(stderr, "Now at end of input.\n");
#endif
    }
  else
    {
      yychar1 = YYTRANSLATE(yychar);

#if YYDEBUG != 0
      if (yydebug)
	{
	  fprintf (stderr, "Next token is %d (%s", yychar, yytname[yychar1]);
	  /* Give the individual parser a way to print the precise meaning
	     of a token, for further debugging info.  */
#ifdef YYPRINT
	  YYPRINT (stderr, yychar, yylval);
#endif
	  fprintf (stderr, ")\n");
	}
#endif
    }

  yyn += yychar1;
  if (yyn < 0 || yyn > YYLAST || yycheck[yyn] != yychar1)
    goto yydefault;

  yyn = yytable[yyn];

  /* yyn is what to do for this token type in this state.
     Negative => reduce, -yyn is rule number.
     Positive => shift, yyn is new state.
       New state is final state => don't bother to shift,
       just return success.
     0, or most negative number => error.  */

  if (yyn < 0)
    {
      if (yyn == YYFLAG)
	goto yyerrlab;
      yyn = -yyn;
      goto yyreduce;
    }
  else if (yyn == 0)
    goto yyerrlab;

  if (yyn == YYFINAL)
    YYACCEPT;

  /* Shift the lookahead token.  */

#if YYDEBUG != 0
  if (yydebug)
    fprintf(stderr, "Shifting token %d (%s), ", yychar, yytname[yychar1]);
#endif

  /* Discard the token being shifted unless it is eof.  */
  if (yychar != YYEOF)
    yychar = YYEMPTY;

  *++yyvsp = yylval;
#ifdef YYLSP_NEEDED
  *++yylsp = yylloc;
#endif

  /* count tokens shifted since error; after three, turn off error status.  */
  if (yyerrstatus) yyerrstatus--;

  yystate = yyn;
  goto yynewstate;

/* Do the default action for the current state.  */
yydefault:

  yyn = yydefact[yystate];
  if (yyn == 0)
    goto yyerrlab;

/* Do a reduction.  yyn is the number of a rule to reduce with.  */
yyreduce:
  yylen = yyr2[yyn];
  if (yylen > 0)
    yyval = yyvsp[1-yylen]; /* implement default value of the action */

#if YYDEBUG != 0
  if (yydebug)
    {
      int i;

      fprintf (stderr, "Reducing via rule %d (line %d), ",
	       yyn, yyrline[yyn]);

      /* Print the symbols being reduced, and their result.  */
      for (i = yyprhs[yyn]; yyrhs[i] > 0; i++)
	fprintf (stderr, "%s ", yytname[yyrhs[i]]);
      fprintf (stderr, " -> %s\n", yytname[yyr1[yyn]]);
    }
#endif


  switch (yyn) {

case 8:
#line 65 "syncheck.y"
{
		infunct = 0;
	;
    break;}
case 9:
#line 69 "syncheck.y"
{
		infunct = 0;
	;
    break;}
case 10:
#line 73 "syncheck.y"
{
		yyerror("invalid name field in function declaration");
		infunct = 0;
	;
    break;}
case 11:
#line 78 "syncheck.y"
{
		yyerror("function declaration missing '.' specifier");
		infunct = 0;
	;
    break;}
case 12:
#line 83 "syncheck.y"
{
		yyerror("function declaration missing object number");
		infunct = 0;
	;
    break;}
case 13:
#line 88 "syncheck.y"
{
		yyerror("invalid name specifier in system function declaration");
		infunct = 0;
	;
    break;}
case 14:
#line 93 "syncheck.y"
{
		yyerror("function declaration name incorrecly specified");
		infunct = 0;
	;
    break;}
case 15:
#line 102 "syncheck.y"
{
		infunct = 1;
	;
    break;}
case 31:
#line 140 "syncheck.y"
{
		yyerror("illegal object number specifier after \"#\"");
	;
    break;}
case 34:
#line 151 "syncheck.y"
{
		if(!infunct)
			yyerror("cannot use $ NUM outside of a function");
	;
    break;}
case 39:
#line 165 "syncheck.y"
{
		yyerror("illegal name specifier for element");
	;
    break;}
case 42:
#line 177 "syncheck.y"
{
		if(!infunct)
			yyerror("cannot use $ NUM outside of a function");
	;
    break;}
case 43:
#line 182 "syncheck.y"
{
		yyerror("illegal use of \"$\" to form a variable");
	;
    break;}
case 49:
#line 197 "syncheck.y"
{
		if(!infunct) {
			yyerror("cannot use $# outside of a function");
		}
	;
    break;}
case 52:
#line 205 "syncheck.y"
{
		yyerror("malformed call to built-in function");
	;
    break;}
case 61:
#line 217 "syncheck.y"
{
		yyerror("malformed reference");
	;
    break;}
case 74:
#line 238 "syncheck.y"
{
		yyerror("malformed function parameter list");
	;
    break;}
}
   /* the action file gets copied in in place of this dollarsign */
#line 465 "/usr/lib/bison.simple"

  yyvsp -= yylen;
  yyssp -= yylen;
#ifdef YYLSP_NEEDED
  yylsp -= yylen;
#endif

#if YYDEBUG != 0
  if (yydebug)
    {
      short *ssp1 = yyss - 1;
      fprintf (stderr, "state stack now");
      while (ssp1 != yyssp)
	fprintf (stderr, " %d", *++ssp1);
      fprintf (stderr, "\n");
    }
#endif

  *++yyvsp = yyval;

#ifdef YYLSP_NEEDED
  yylsp++;
  if (yylen == 0)
    {
      yylsp->first_line = yylloc.first_line;
      yylsp->first_column = yylloc.first_column;
      yylsp->last_line = (yylsp-1)->last_line;
      yylsp->last_column = (yylsp-1)->last_column;
      yylsp->text = 0;
    }
  else
    {
      yylsp->last_line = (yylsp+yylen-1)->last_line;
      yylsp->last_column = (yylsp+yylen-1)->last_column;
    }
#endif

  /* Now "shift" the result of the reduction.
     Determine what state that goes to,
     based on the state we popped back to
     and the rule number reduced by.  */

  yyn = yyr1[yyn];

  yystate = yypgoto[yyn - YYNTBASE] + *yyssp;
  if (yystate >= 0 && yystate <= YYLAST && yycheck[yystate] == *yyssp)
    yystate = yytable[yystate];
  else
    yystate = yydefgoto[yyn - YYNTBASE];

  goto yynewstate;

yyerrlab:   /* here on detecting error */

  if (! yyerrstatus)
    /* If not already recovering from an error, report this error.  */
    {
      ++yynerrs;

#ifdef YYERROR_VERBOSE
      yyn = yypact[yystate];

      if (yyn > YYFLAG && yyn < YYLAST)
	{
	  int size = 0;
	  char *msg;
	  int x, count;

	  count = 0;
	  /* Start X at -yyn if nec to avoid negative indexes in yycheck.  */
	  for (x = (yyn < 0 ? -yyn : 0);
	       x < (sizeof(yytname) / sizeof(char *)); x++)
	    if (yycheck[x + yyn] == x)
	      size += strlen(yytname[x]) + 15, count++;
	  msg = (char *) malloc(size + 15);
	  if (msg != 0)
	    {
	      strcpy(msg, "parse error");

	      if (count < 5)
		{
		  count = 0;
		  for (x = (yyn < 0 ? -yyn : 0);
		       x < (sizeof(yytname) / sizeof(char *)); x++)
		    if (yycheck[x + yyn] == x)
		      {
			strcat(msg, count == 0 ? ", expecting `" : " or `");
			strcat(msg, yytname[x]);
			strcat(msg, "'");
			count++;
		      }
		}
	      yyerror(msg);
	      free(msg);
	    }
	  else
	    yyerror ("parse error; also virtual memory exceeded");
	}
      else
#endif /* YYERROR_VERBOSE */
	yyerror("parse error");
    }

  goto yyerrlab1;
yyerrlab1:   /* here on error raised explicitly by an action */

  if (yyerrstatus == 3)
    {
      /* if just tried and failed to reuse lookahead token after an error, discard it.  */

      /* return failure if at end of input */
      if (yychar == YYEOF)
	YYABORT;

#if YYDEBUG != 0
      if (yydebug)
	fprintf(stderr, "Discarding token %d (%s).\n", yychar, yytname[yychar1]);
#endif

      yychar = YYEMPTY;
    }

  /* Else will try to reuse lookahead token
     after shifting the error token.  */

  yyerrstatus = 3;		/* Each real token shifted decrements this */

  goto yyerrhandle;

yyerrdefault:  /* current state does not do anything special for the error token. */

#if 0
  /* This is wrong; only states that explicitly want error tokens
     should shift them.  */
  yyn = yydefact[yystate];  /* If its default is to accept any token, ok.  Otherwise pop it.*/
  if (yyn) goto yydefault;
#endif

yyerrpop:   /* pop the current state because it cannot handle the error token */

  if (yyssp == yyss) YYABORT;
  yyvsp--;
  yystate = *--yyssp;
#ifdef YYLSP_NEEDED
  yylsp--;
#endif

#if YYDEBUG != 0
  if (yydebug)
    {
      short *ssp1 = yyss - 1;
      fprintf (stderr, "Error: state stack now");
      while (ssp1 != yyssp)
	fprintf (stderr, " %d", *++ssp1);
      fprintf (stderr, "\n");
    }
#endif

yyerrhandle:

  yyn = yypact[yystate];
  if (yyn == YYFLAG)
    goto yyerrdefault;

  yyn += YYTERROR;
  if (yyn < 0 || yyn > YYLAST || yycheck[yyn] != YYTERROR)
    goto yyerrdefault;

  yyn = yytable[yyn];
  if (yyn < 0)
    {
      if (yyn == YYFLAG)
	goto yyerrpop;
      yyn = -yyn;
      goto yyreduce;
    }
  else if (yyn == 0)
    goto yyerrpop;

  if (yyn == YYFINAL)
    YYACCEPT;

#if YYDEBUG != 0
  if (yydebug)
    fprintf(stderr, "Shifting error token, ");
#endif

  *++yyvsp = yylval;
#ifdef YYLSP_NEEDED
  *++yylsp = yylloc;
#endif

  yystate = yyn;
  goto yynewstate;
}
#line 242 "syncheck.y"



int
lookup(s)
char	*s;
{
	int	start	= 0;
	int	ret;

	static	struct	kwordz {
		char	*kw;
		int	rval;
	} keyz[] = {
	"NULL",			VNULL,
	"atoi",			BLTIN,
	"atoobj",		BLTIN,
	"catfile",		BLTIN,
	"chmod",		BLTIN,
	"chown",		BLTIN,
	"echo",			BLTIN,
	"echoto",		BLTIN,
	"else",			ELSE,
	"errno",		BLTIN,
	"error",		BLTIN,
	"foreach",		FOR,
	"foreacharg",		FORARG,
	"func",			FUNC,
	"geteuid",		BLTIN,
	"getip",		BLTIN,
	"getuid",		BLTIN,
	"if",			IF,
	"in",			IN,
	"islist",		BLTIN,
	"isnum",		BLTIN,
	"isobj",		BLTIN,
	"isstr",		BLTIN,
	"listadd",		BLTIN,
	"listappend",		BLTIN,
	"listcount",		BLTIN,
	"listdrop",		BLTIN,
	"listelem",		BLTIN,
	"listmerge",		BLTIN,
	"listnew",		BLTIN,
	"listprepend",		BLTIN,
	"listsearch",		BLTIN,
	"listsetelem",		BLTIN,
	"log",			BLTIN,
	"match",		BLTIN,
	"objectdestroy",	BLTIN,
	"objectelements",	BLTIN,
	"objectnew",		BLTIN,
	"objectowner",		BLTIN,
	"random",		BLTIN,
	"return",		RETURN,
	"setuid",		BLTIN,
	"str",			BLTIN,
	"strlen",		BLTIN,
	"strtime",		BLTIN,
	"time",			BLTIN,
	0,0
	};

	int	end	= (sizeof(keyz)/sizeof(struct kwordz)) - 2;
	int	p	= end/2;

	while(start <= end) {
		ret = strcmp(s,keyz[p].kw);
		if(ret == 0)
			return(keyz[p].rval);
		if(ret > 0)
			start = p + 1;
		else
			end = p - 1;

		p = start + ((end - start)/2);
	}
	return(-1);
}


/*
THIS IS NOT THE SAME LEXICAL ANALYSER AS IN THE U-COMPILER! 
Mix them and you will DIE!
*/
yylex()
{
	char	*p = lexbuf;
	int	c;

	*p = '\0';

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
	*p = '\0';

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
			if(++cnt + 1 >= 20)
				yyerror("identifier too long");
			*p++ = c;
		}

		(void)ungetc(c,yyin);

		*p = '\0';

		if((rv = lookup(lexbuf)) != -1)
			return(rv);
		return(IDENT);
	}

	/* handle quoted strings */
	if(c == '"' || c == '\'') {
		int	cnt = 0;
		int	quot = c;

		/* strip start quote by resetting ptr */
		p = lexbuf;
		*p = '\0';

		/* match quoted strings */
		while((c = fgetc(yyin)) != EOF && c != quot) {
			if(!isascii(c))
				continue;

			if(++cnt + 1 >= sizeof(lexbuf))
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
					case	';':
					case	'\'':
					case	'"':
					case	'n':
						break;

					default:
						yywarning("unrecognized escape");
				}
			} else {
				*p++ = c;
			}
		}

		if(c == EOF)
			yyerror("EOF in quoted string");

		*p = '\0';
	
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
				*p++ = t;
				*p++ = '\0';
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
				*p++ = t;
				*p++ = '\0';
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
				*p++ = t;
				*p++ = '\0';
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
				*p++ = t;
				*p++ = '\0';
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
				*p++ = t;
				*p++ = '\0';
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
				*p++ = t;
				*p++ = '\0';
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

	*p = '\0';
	return(c);
}


yyerror(s)
char	*s;
{
	fprintf(stderr,"%s:[line %d] Error: %s",fname,yylineno,s);
	if(lexbuf[0] != '\0');
		fprintf(stderr," near \"%s\"",lexbuf);
	fprintf(stderr,"\n");
	yyerrcnt++;
}


yywarning(s)
char	*s;
{
	fprintf(stderr,"%s:[line %d] Warning: %s",fname,yylineno,s);
	if(lexbuf[0] != '\0');
		fprintf(stderr," near \"%s\"",lexbuf);
	fprintf(stderr,"\n");
	yywarcnt++;
}


#ifdef	USECPP
/*
run a file through the pre-processor and return 0 or the resulting
file name
*/
char	*
preprocess_file(f)
char	*f;
{
	extern	char	*mktemp();
	static	char	*tmpf = (char *)0;

	int	sval;
	char	cbuf[600];

	if(tmpf == (char *)0) {
		if((tmpf = mktemp("/tmp/usynXXXXXX")) == NULL)
			return(NULL);
	}
	(void)strcpy(cbuf,DEFAULTCPP);
	(void)strcat(cbuf," ");
	(void)strcat(cbuf,f);
	(void)strcat(cbuf," ");
	(void)strcat(cbuf,tmpf);
	sval = system(cbuf);
	if(sval == 127) {
		fprintf(stderr,"cannot invoke c-preprocessor %s!\n",DEFAULTCPP);
		return((char *)0);
	}
	if(sval != 0)
		return((char *)0);
	return(tmpf);
}
#endif


crunch_file(name,ifd)
char	*name;
FILE	*ifd;
{
	if(ifd == NULL)
		return(0);

	yyin = ifd;
	fname = name;
	yylineno = 1;
	yyerrcnt = yywarcnt = 0;

	while(yyparse())
		;

	fprintf(stderr,"%s:%d errors, %d warnings\n",fname,yyerrcnt,yywarcnt);
	return(yyerrcnt);
}



main(ac,av)
int	ac;
char	*av[];
{
	int	acx;
	int	rv = 0;

	if(ac == 1)
		exit(crunch_file("standard input",stdin));

	for(acx = 1; acx < ac; acx++) {
#ifdef	USECPP
		char	*tmp;

		if((tmp = preprocess_file(av[acx])) == (char *)0) {
			perror(av[acx]);
			rv++;
			continue;
		}
		if((yyin = fopen(tmp,"r")) == NULL) {
			perror(tmp);
			(void)unlink(tmp);
			rv++;
			continue;
		}
		rv += crunch_file(av[acx],yyin);
		(void)unlink(tmp);
#else
		if((yyin = fopen(av[acx],"r")) == NULL) {
			perror(av[acx]);
			rv++;
			continue;
		}
		rv += crunch_file(av[acx],yyin);
#endif
	}
	exit(rv);
}

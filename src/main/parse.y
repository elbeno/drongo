%{
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
%}


%token	NUM STR IDENT
%token	OUTOFSPACE BLTIN
%token	IF IN FOR FORARG ELSE FUNC RETURN VNULL

%right	ASGN
%left	REF
%left	OR 
%left	AND
%left	GT GTE LT LTE EQ NE
%left	ADD SUB
%left	MUL DIV
%right	UNARY
%left	NOT

%%
program:
	/* nothing */
	| program statement
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
	| program funcdef
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
	;



asgn:
	'$' IDENT ASGN expr
	{
		$$ = $4;
#ifdef	COMPILEDEBUG
		printf("OP_ASGN@%d\n",ASMPC);
#endif
		(void)assemble(yaccprog,OP_ASGN);
#ifdef	COMPILEDEBUG
		printf("%s@%d\n",&stringbuf[$2],ASMPC);
#endif
		(void)assemble(yaccprog,$2);
	}
	|
	MUL element ASGN expr %prec UNARY
	{
		/* *#44.bar = expr; */
		$$ = $2;
#ifdef	COMPILEDEBUG
		printf("OP_ECASGN@%d\n",ASMPC);
#endif
		(void)assemble(yaccprog,OP_ECASGN);
	}
	|
	element ASGN expr
	{
		/* #44.bar = expr; */
#ifdef	COMPILEDEBUG
		printf("OP_EASGN@%d\n",ASMPC);
#endif
		(void)assemble(yaccprog,OP_EASGN);
	}
	;



statement:
	expr ';'
	{
#ifdef	COMPILEDEBUG
		printf("OP_POP@%d\n",ASMPC);
#endif
		(void)assemble(yaccprog,OP_POP);
	}
	| RETURN ';'
	{
		if(!infunct) {
			yyerror(infuncmsg);
			compiler_state = COMPILE_BAD;
		} else {
#ifdef	COMPILEDEBUG
			printf("OP_RETNV@%d\n",ASMPC);
#endif
			$$ = assemble(yaccprog,OP_RETNV);
		}
	}
	| RETURN expr ';'
	{
		$$ = $2;
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
	| ';'
	| ifheader condition statement stop
	{
		yaccprog->p_mem[$1 + 1] = $3;	/* if part */
		yaccprog->p_mem[$1 + 2] = 0;	/* else part (none) */
		yaccprog->p_mem[$1 + 3] = $4;	/* continuation */
#ifdef	COMPILEDEBUG
printf("IF@%d backpatch, ifpart:%d@%d, else:%d@%d, cont:%d@%d\n",
		$1, yaccprog->p_mem[$1+1],$1+1,yaccprog->p_mem[$1+2],$1+2,
		yaccprog->p_mem[$1+3],$1+3);
#endif
	}
	| ifheader condition statement stop ELSE statement stop
	{
		yaccprog->p_mem[$1 + 1] = $3;	/* if part */
		yaccprog->p_mem[$1 + 2] = $6;	/* else part */
		yaccprog->p_mem[$1 + 3] = $7;	/* continuation */
#ifdef	COMPILEDEBUG
printf("IF@%d backpatch, ifpart:%d@%d, else:%d@%d, cont:%d@%d\n",
		$1, yaccprog->p_mem[$1+1],$1+1,yaccprog->p_mem[$1+2],$1+2,
		yaccprog->p_mem[$1+3],$1+3);
#endif
	}
	| forheader '$' IDENT IN condition statement stop
	{
		yaccprog->p_mem[$1 + 1] = $3;	/* iterator sym name */
		yaccprog->p_mem[$1 + 2] = $6;	/* do part */
		yaccprog->p_mem[$1 + 3] = $7;	/* continuation */
#ifdef	COMPILEDEBUG
printf("FOR@%d backpatch, sym:%s@%d, do:%d@%d, cont:%d@%d\n",
		$1, &stringbuf[$3],$1+1,yaccprog->p_mem[$1+2],$1+2,
		yaccprog->p_mem[$1+3],$1+3);
#endif
	}
	| forargheader '$' IDENT statement stop
	{
		yaccprog->p_mem[$1 + 1] = $3;	/* iterator sym name */
		yaccprog->p_mem[$1 + 2] = $5;	/* continuation */
#ifdef	COMPILEDEBUG
printf("FORARG@%d backpatch, iter:%s@%d, cont:%d@%d\n",
		$1, &stringbuf[$3],$1+1,yaccprog->p_mem[$1+2],$1+2);
#endif
	}
	| '{' statements '}'
	{
		$$ = $2;
	}
	;



condition:
	'(' expr ')'
	{
		$$ = $2;
#ifdef	COMPILEDEBUG
		printf("OP_STOP@%d\n",ASMPC);
#endif
		(void)assemble(yaccprog,OP_STOP);
	}
	;



stop:
	/* nothing */
	{
#ifdef	COMPILEDEBUG
		printf("OP_STOP@%d\n",ASMPC);
#endif
		(void)assemble(yaccprog,OP_STOP);
		$$ = ASMPC;
	}
	;



statements:
	/* nothing */
	{
		$$ = ASMPC;
	}
	| statements statement
	;



refnum:
	'@'
	{
#ifdef	COMPILEDEBUG
		printf("OP_ROOTPUSH@%d\n",ASMPC);
#endif
		$$ = assemble(yaccprog,OP_ROOTPUSH);
	}
	| '#' NUM
	{
#ifdef	COMPILEDEBUG
		printf("OP_RPUSH@%d\n",ASMPC);
#endif
		$$ = assemble(yaccprog,OP_RPUSH);
#ifdef	COMPILEDEBUG
		printf("%d@%d\n",$2,ASMPC);
#endif
		(void)assemble(yaccprog,$2);
	}
	| '#' IDENT
	{
		if(!strcmp(&stringbuf[$2],"self")) {
#ifdef	COMPILEDEBUG
			printf("OP_SELFPUSH@%d\n",ASMPC);
#endif
			$$ = assemble(yaccprog,OP_SELFPUSH);
		} else
		if(!strcmp(&stringbuf[$2],"caller")) {
#ifdef	COMPILEDEBUG
			printf("OP_CALLPUSH@%d\n",ASMPC);
#endif
			$$ = assemble(yaccprog,OP_CALLPUSH);
		} else
		if(!strcmp(&stringbuf[$2],"actor")) {
#ifdef	COMPILEDEBUG
			printf("OP_ACTPUSH@%d\n",ASMPC);
#endif
			$$ = assemble(yaccprog,OP_ACTPUSH);
		} else {
			yyerror("error. must specify self|actor|caller\n");
			compiler_state = COMPILE_BAD;
		}
	}
	;



element_head:
	refnum
	| '$' IDENT
	{
#ifdef	COMPILEDEBUG
		printf("OP_EVAL@%d\n",ASMPC);
#endif
		$$ = assemble(yaccprog,OP_EVAL);
#ifdef	COMPILEDEBUG
		printf("%s@%d\n",&stringbuf[$2],ASMPC);
#endif
		(void)assemble(yaccprog,$2);
	}
	| '$' NUM
	{
		if(!infunct) {
			yyerror(infuncmsg);
			compiler_state = COMPILE_BAD;
		} else {
#ifdef	COMPILEDEBUG
			printf("OP_PPUSH@%d\n",ASMPC);
#endif
			$$ = assemble(yaccprog,OP_PPUSH);
#ifdef	COMPILEDEBUG
			printf("%d@%d\n",$2,ASMPC);
#endif
			(void)assemble(yaccprog,$2);
		}
	}
	;



element:
	element_head '.' IDENT
	{
#ifdef	COMPILEDEBUG
		printf("OP_ELPUSH@%d\n",ASMPC);
#endif
		(void)assemble(yaccprog,OP_ELPUSH);
#ifdef	COMPILEDEBUG
		printf("%s@%d\n",&stringbuf[$3],ASMPC);
#endif
		(void)assemble(yaccprog,$3);
	}
	| element_head '.' '(' expr ')'
	{
#ifdef	COMPILEDEBUG
		printf("OP_STRVAR@%d\n",ASMPC);
#endif
		(void)assemble(yaccprog,OP_STRVAR);
	}
	| element '.' IDENT
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
		printf("%s@%d\n",&stringbuf[$3],ASMPC);
#endif
		(void)assemble(yaccprog,$3);
	}
	| element '.' '(' push_eval_kludge expr ')'
	{
#ifdef	COMPILEDEBUG
		printf("OP_STRVAR@%d\n",ASMPC);
#endif
		(void)assemble(yaccprog,OP_STRVAR);
	}
	;



push_eval_kludge:
	/*
	nothing - what this does is ensures that the element is
	pushed AND evaluated BEFORE the expression is pushed and
	evaluated. this is somewhat gross, but then the whole
	string->variable hack is somewhat gross.
	*/
	{
#ifdef	COMPILEDEBUG
		printf("OP_EEVAL@%d\n",ASMPC);
#endif
		(void)assemble(yaccprog,OP_EEVAL);
	}
	;



evaluated_element:
	element
	{
#ifdef	COMPILEDEBUG
		printf("OP_EEVAL@%d\n",ASMPC);
#endif
		(void)assemble(yaccprog,OP_EEVAL);
	}
	| '$' IDENT
	{
#ifdef	COMPILEDEBUG
		printf("OP_EVAL@%d\n",ASMPC);
#endif
		$$ = assemble(yaccprog,OP_EVAL);	/* replace w/ contents */
#ifdef	COMPILEDEBUG
		printf("%s@%d\n",&stringbuf[$2],ASMPC);
#endif
		(void)assemble(yaccprog,$2);	/* variable name */
	}
	| '$' NUM
	{
		if(!infunct) {
			yyerror(infuncmsg);
			compiler_state = COMPILE_BAD;
		} else {
#ifdef	COMPILEDEBUG
			printf("OP_PPUSH@%d\n",ASMPC);
#endif
			$$ = assemble(yaccprog,OP_PPUSH);
#ifdef	COMPILEDEBUG
			printf("%d@%d\n",$2,ASMPC);
#endif
			(void)assemble(yaccprog,$2);
		}
	}
	;


expr:
	NUM
	{
#ifdef	COMPILEDEBUG
		printf("OP_NPUSH@%d\n",ASMPC);
#endif
		$$ = assemble(yaccprog,OP_NPUSH);
#ifdef	COMPILEDEBUG
		printf("%d@%d\n",$1,ASMPC);
#endif
		(void)assemble(yaccprog,$1);
	}
	| STR
	{
#ifdef	COMPILEDEBUG
		printf("OP_SPUSH@%d\n",ASMPC);
#endif
		$$ = assemble(yaccprog,OP_SPUSH);
#ifdef	COMPILEDEBUG
		printf("%s@%d\n",&stringbuf[$1],ASMPC);
#endif
		(void)assemble(yaccprog,$1);
	}
	| VNULL
	{
#ifdef	COMPILEDEBUG
		printf("OP_NULPUSH@%d\n",ASMPC);
#endif
		$$ = assemble(yaccprog,OP_NULPUSH);
	}
	| '$' '#'
	{
		if(!infunct) {
			yyerror(infuncmsg);
			compiler_state = COMPILE_BAD;
		} else
#ifdef	COMPILEDEBUG
			printf("OP_CPUSH@%d\n",ASMPC);
#endif
			$$ = assemble(yaccprog,OP_CPUSH);
	}
	| refnum
	| evaluated_element
	| asgn
	| BLTIN beginargs '(' arglist ')'
	{
		$$ = $2;
#ifdef	COMPILEDEBUG
		printf("OP_BLTIN@%d\n",ASMPC);
#endif
		(void)assemble(yaccprog,OP_BLTIN);
#ifdef	COMPILEDEBUG
		printf("bltin #%d@%d\n",$1,ASMPC);
#endif
		(void)assemble(yaccprog,$1);
#ifdef	COMPILEDEBUG
		printf("%d args@%d\n",$4,ASMPC);
#endif
		(void)assemble(yaccprog,$4);
	}
	| evaluated_element '(' arglist ')'
	{
#ifdef	COMPILEDEBUG
		printf("OP_CALL@%d\n",ASMPC);
#endif
		(void)assemble(yaccprog,OP_CALL);
#ifdef	COMPILEDEBUG
		printf("%d args@%d\n",$3,ASMPC);
#endif
		(void)assemble(yaccprog,$3);
	}
	| '(' expr ')'
	{
		$$ = $2;
	}
	| expr ADD expr
	{
#ifdef	COMPILEDEBUG
		printf("OP_ADD@%d\n",ASMPC);
#endif
		(void)assemble(yaccprog,OP_ADD);
	}
	| expr SUB expr
	{
#ifdef	COMPILEDEBUG
		printf("OP_SUB@%d\n",ASMPC);
#endif
		(void)assemble(yaccprog,OP_SUB);
	}
	| expr MUL expr
	{
#ifdef	COMPILEDEBUG
		printf("OP_MUL@%d\n",ASMPC);
#endif
		(void)assemble(yaccprog,OP_MUL);
	}
	| expr DIV expr
	{
#ifdef	COMPILEDEBUG
		printf("OP_DIV@%d\n",ASMPC);
#endif
		(void)assemble(yaccprog,OP_DIV);
	}
	| SUB expr %prec UNARY
	{
		$$ = $2;
#ifdef	COMPILEDEBUG
		printf("OP_NEG@%d\n",ASMPC);
#endif
		(void)assemble(yaccprog,OP_NEG);	/* negate top of stack */
	}
	| REF element %prec UNARY
	{
		$$ = $2;
#ifdef	COMPILEDEBUG
		printf("OP_REF@%d\n",ASMPC);
#endif
		(void)assemble(yaccprog,OP_REF);
	}
	| expr EQ expr
	{
#ifdef	COMPILEDEBUG
		printf("OP_EQ@%d\n",ASMPC);
#endif
		(void)assemble(yaccprog,OP_EQ);
	}
	| expr NE expr
	{
#ifdef	COMPILEDEBUG
		printf("OP_NE@%d\n",ASMPC);
#endif
		(void)assemble(yaccprog,OP_NE);
	}
	| expr LT expr
	{
#ifdef	COMPILEDEBUG
		printf("OP_LT@%d\n",ASMPC);
#endif
		(void)assemble(yaccprog,OP_LT);
	}
	| expr LTE expr
	{
#ifdef	COMPILEDEBUG
		printf("OP_LTE@%d\n",ASMPC);
#endif
		(void)assemble(yaccprog,OP_LTE);
	}
	| expr GT expr
	{
#ifdef	COMPILEDEBUG
		printf("OP_GT@%d\n",ASMPC);
#endif
		(void)assemble(yaccprog,OP_GT);
	}
	| expr GTE expr
	{
#ifdef	COMPILEDEBUG
		printf("OP_GTE@%d\n",ASMPC);
#endif
		(void)assemble(yaccprog,OP_GTE);
	}
	| expr AND expr
	{
#ifdef	COMPILEDEBUG
		printf("OP_AND@%d\n",ASMPC);
#endif
		(void)assemble(yaccprog,OP_AND);
	}
	| expr OR expr
	{
#ifdef	COMPILEDEBUG
		printf("OP_OR@%d\n",ASMPC);
#endif
		(void)assemble(yaccprog,OP_OR);
	}
	| NOT expr %prec UNARY
	{
		$$ = $2;
#ifdef	COMPILEDEBUG
		printf("OP_NOT@%d\n",ASMPC);
#endif
		(void)assemble(yaccprog,OP_NOT);	/* invert top of stack */
	}
	;



forheader:
	FOR
	{
#ifdef	COMPILEDEBUG
		printf("OP_FOR (header only)@%d\n",ASMPC);
#endif
		$$ = assemble(yaccprog,OP_FOR);
		(void)assemble(yaccprog,OP_STOP);
		(void)assemble(yaccprog,OP_STOP);
		(void)assemble(yaccprog,OP_STOP);
	}
	;



forargheader:
	FORARG
	{
#ifdef	COMPILEDEBUG
		printf("OP_FORARG (header only)@%d\n",ASMPC);
#endif
		$$ = assemble(yaccprog,OP_FORARG);
		(void)assemble(yaccprog,OP_STOP);
		(void)assemble(yaccprog,OP_STOP);
	}
	;



ifheader:
	IF
	{
#ifdef	COMPILEDEBUG
		printf("OP_IF (header only)@%d\n",ASMPC);
#endif
		$$ = assemble(yaccprog,OP_IF);
		(void)assemble(yaccprog,OP_STOP);
		(void)assemble(yaccprog,OP_STOP);
		(void)assemble(yaccprog,OP_STOP);
	}
	;



beginargs:
	/* nothing */
	{
		$$ = ASMPC;
	}
	;



funcdef:
	funcheader '@' '.' IDENT statement
	{
		/*
		this is gross, but the best way to handle it.
		store a pointer to the name of the currently
		defined function, to allow easy linking. this
		COULD be passed back in the compiled program,
		but then it would have to be stripped out.
		so, do it like this. Sue me.
		*/
		currfunc = &stringbuf[$4];
		currenum = (long)0;
	}
	| funcheader '#' NUM '.' IDENT statement
	{
		currfunc = &stringbuf[$5];
		currenum = (long)$3;
		currfuncop = OP_EASGN;
	}
	| funcheader MUL '#' NUM '.' IDENT statement
	{
		currfunc = &stringbuf[$6];
		currenum = (long)$4;
		currfuncop = OP_ECASGN;
	}
	;



arglist:
	/* nothing */
	{
		$$ = 0;
	}
	| expr
	{
		$$ = 1;
	}
	| arglist ',' expr
	{
		$$ = $1 + 1;
	}
	;



funcheader:
	FUNC 
	{
		infunct = 1;
	}
	;

%%
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

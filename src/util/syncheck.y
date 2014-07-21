%{

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
	| program funcdef
	| program error
	;



asgn:
	'$' IDENT ASGN expr
	| MUL element ASGN expr %prec UNARY
	| element ASGN expr
	;



funcdef:
	funcheader '@' '.' IDENT statement
	{
		infunct = 0;
	}
	| funcheader '#' NUM '.' IDENT statement
	{
		infunct = 0;
	}
	| funcheader '#' NUM '.' error
	{
		yyerror("invalid name field in function declaration");
		infunct = 0;
	}
	| funcheader '#' NUM error
	{
		yyerror("function declaration missing '.' specifier");
		infunct = 0;
	}
	| funcheader '#' error
	{
		yyerror("function declaration missing object number");
		infunct = 0;
	}
	| funcheader '@' error
	{
		yyerror("invalid name specifier in system function declaration");
		infunct = 0;
	}
	| funcheader error
	{
		yyerror("function declaration name incorrecly specified");
		infunct = 0;
	}
	;


funcheader:
	FUNC
	{
		infunct = 1;
	}
	;


statement:
	expr ';'
	| RETURN ';'
	| RETURN expr ';'
	| ';'
	| IF condition statement 
	| IF condition statement ELSE statement 
	| FOR '$' IDENT IN condition statement
	| FORARG '$' IDENT statement
	| '{' statements '}'
	;



statements:
	/* nothing */
	| statements statement
	;



condition:
	'(' expr ')'
	;



refnum:
	'@'
	| '#' NUM
	| '#' IDENT
	| '#' error
	{
		yyerror("illegal object number specifier after \"#\"");
	}
	;



element_head:
	refnum
	| '$' IDENT
	| '$' NUM
	{
		if(!infunct)
			yyerror("cannot use $ NUM outside of a function");
	}
	;



element:
	element_head '.' IDENT
	| element_head '.' '(' expr ')'
	| element '.' IDENT
	| element '.' '(' expr ')'
	| element '.' error
	{
		yyerror("illegal name specifier for element");
	}
	;




evaluated_element:
	element
	| '$' IDENT
	| '$' NUM
	{
		if(!infunct)
			yyerror("cannot use $ NUM outside of a function");
	}
	| '$' error
	{
		yyerror("illegal use of \"$\" to form a variable");
	}
	;




expr:
	NUM
	| STR
	| VNULL
	| evaluated_element
	| refnum
	| '$' '#'
	{
		if(!infunct) {
			yyerror("cannot use $# outside of a function");
		}
	}
	| asgn
	| BLTIN '(' arglist ')'
	| BLTIN error
	{
		yyerror("malformed call to built-in function");
	}
	| evaluated_element '(' arglist ')'
	| '(' expr ')'
	| expr ADD expr
	| expr SUB expr
	| expr MUL expr
	| expr DIV expr
	| SUB expr %prec UNARY
	| REF element %prec UNARY
	| REF error
	{
		yyerror("malformed reference");
	}
	| expr EQ expr
	| expr NE expr
	| expr AND expr
	| expr OR expr
	| expr LT expr
	| expr LTE expr
	| expr GT expr
	| expr GTE expr
	| NOT expr %prec UNARY
	;



arglist:
	/* nothing */
	| expr
	| arglist ',' expr
	| arglist error
	{
		yyerror("malformed function parameter list");
	}
	;
%%


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

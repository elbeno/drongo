#ifndef	_INCL_UBERMUD_H

/*
Copyright(C) 1990, Marcus J. Ranum, All Rights Reserved.
This software may be freely used, modified, and redistributed,
as long as this copyright message is left intact, and this
software is not included with any commercial product, or used
in any product that is provided on a pay-for-use basis.
*/

/* size of a communications buffer */
#define	MUDBUFSIZ	1024

/* max # of tokens that can be passed on a command line */
#define	MUDMAXARG	MUDBUFSIZ/2

/* max length of a field/identifier name. it should be reasonably small */
#define	MAXIDENTLEN	20

/* max # of lists that can be matched against at once. be reasonable */
#define	MAXMATCHLIST	20

/* max # of CPU "ticks" allowed for a run() before killing it */
#define	LIMITRUNCPU 1

#ifdef	LIMITRUNCPU
/* I have no idea what is a good number */
#define	MAXCPUTICKS	12000
#endif

/*
size of internal assembler buffers. this limits the max amount of string
space that can be used in a given object, as well as the max # of strings.
the instbuf is the buffer for virtual instructions. it need not be too
large. the STRBUF is also used to tokenize input into, so it MUST be
bigger than or equal to MUDBUFSIZ
*/
#define	STRBUFSIZ	2048
#define	INSBUFSIZ	2048

/*
default flags to the c-preprocessor - ONLY boot-files are pre-processed
not user-provided input. this is because a user's comments should all
already be stripped BEFORE it gets sent over the net!
*/
#define	USECPP
#ifdef	USECPP
#define	DEFAULTCPP	"/lib/cpp -traditional -P"
#endif


/*
size of the internal 'machine' memory. this limits program size and stack
depth, and should be set to a reasonable value.
*/
#define	MACHBUFSIZ	2048
#define	MACHSTKSIZ	1024
#define	MACHFRASIZ	1024


/*
magic cookie character used to separate strings for aliases. this can be
inserted into a string with the escape '\;' - look in the parser, in
match, and in the tcpio code for its usage. - ANYTHING but '\0' or a
printable char.
*/
#define	MATCH_CHAR	01


/* macro to return the size in bytes of a 'compiled' program/function */
#define	PROGSIZE(p)	(sizeof(Prog) + (p)->p_siz * sizeof(int) + (p)->s_siz)

/* macro to return the size in bytes of an object list */
#define	LISTSIZE(l)	(sizeof(ObjList) + (sizeof(long) * (l)->l_cnt))


/*
object permissions bits: note - the 'execute' semantic is irrelevant
in a type-flagged environment. making a function readable is the
same as making it executable.
*/
#define	PERM_OREAD	00200		/* owner r */
#define	PERM_OWRIT	00100		/* owner w */

#define	PERM_IREAD	00020		/* indirect r */
#define	PERM_IWRIT	00010		/* indirect w */

#define	PERM_WREAD	00002		/* world r */
#define	PERM_WWRIT	00001		/* world w */

#define	PERM_SUID	01000		/* setuid bit */
#define	PERM_CHAIN	02000		/* chain function bit */
#define	PERM_BLOCK	04000		/* block system function bit */

#define	PERM_READ	0
#define	PERM_WRITE	1

#define	PERM_DEFAULT	(PERM_OREAD|PERM_OWRIT|PERM_IREAD|PERM_IWRIT|PERM_WREAD)


/*
return codes from the parser:
*/
#define	COMPILE_EOF	0
#define	COMPILE_OK	111
#define	COMPILE_FUNC	112
#define	COMPILE_BAD	113	/* ugh. bad code */
#define	COMPILE_OOS	666
#define	COMPILE_OOE	667


/*
execution states
*/
#define	RUN_OK		0
#define	RUN_STACK	1
#define	RUN_FRAME	2
#define	RUN_ILLEGAL	3
#ifdef	LIMITRUNCPU
#define	RUN_CPU		4
#endif


/*
IO buffer states. - a simple state machine
*/
#define	INPUT_NEWCONN	0
#define	INPUT_PLAY	1
#define	INPUT_PROGRAM	2


/*
IO buffer flags
*/
#define	INPUT_DEFAULT	0
#define	INPUT_PROGAUTH	01
#define	INPUT_WIZAUTH	02


/*
virtual instructions. note that changing any of these values will render
existing databases invalid. you CAN add new opcodes, as long as old
values remain unchanged.
*/
#define	OP_STOP		0	/* halt execution */
#define	OP_POP		1	/* pop (toss) a value off the stack */
#define	OP_CPUSH	2	/* push parameter count on the stack */
#define	OP_ELPUSH	3	/* push element #.string pair */
#define	OP_SELPUSH	4	/* push system element #0.string pair */
#define	OP_NPUSH	5	/* push a number */
#define	OP_NULPUSH	6	/* push a NULL */
#define	OP_PPUSH	7	/* push a numbered parameter on the stack */
#define	OP_RPUSH	8	/* push a numbered object reference */
#define	OP_ROOTPUSH	9	/* push object #0 */
#define	OP_SPUSH	10	/* push a string from static table */
#define	OP_VPUSH	11	/* push a var name from static table */
#define	OP_SELFPUSH	12	/* push notion of current object */
#define	OP_CALLPUSH	13	/* push notion of current caller object */
#define	OP_ACTPUSH	14	/* push notion of current actor */
#define	OP_CALL		15	/* call a userdef func. */
#define	OP_BLTIN	16	/* call a builtin func. */
#define	OP_IF		17	/* if statement */
#define	OP_FOR		18	/* for statement */
#define	OP_FORARG	19	/* for statement across arg list */
#define	OP_REF		20	/* reference a base object */
#define	OP_RETF		21	/* funct. return by dropping off the end */
#define	OP_RETV		22	/* funct. return with a value */
#define	OP_RETNV	23	/* funct. return w/o a value */
#define	OP_EVAL		24	/* deref a var and stack it */
#define	OP_EEVAL	25	/* deref an element and stack it */
#define	OP_ASGN		26	/* assign to a var */
#define	OP_EASGN	27	/* assign to an element */
#define	OP_ECASGN	28	/* assign to contents of an element */
#define	OP_AND		29	/* and operator. (C style) */
#define	OP_OR		30	/* or operator. (C style) */
#define	OP_EQ		31	/* check equality of a numeric value */
#define	OP_NE		32	/* check non-equality of a numeric value */
#define	OP_GT		33	/* greater-than */
#define	OP_LT		34	/* less-than */
#define	OP_GTE		35	/* greater-than or equal to */
#define	OP_LTE		36	/* less-than or equal to */
#define	OP_NOT		37	/* invert sense of numeric value */
#define	OP_NEG		38	/* negate numeric value */
#define	OP_ADD		39	/* add top 2 numbers off stack */
#define	OP_SUB		40	/* subtract top 2 numbers off stack */
#define	OP_MUL		41	/* multiply top 2 numbers off stack */
#define	OP_DIV		42	/* divide top 2 numbers off stack */
#define	OP_STRVAR	43	/* controversial string->var operator */


/* data types we can find in the stack */
#define	TYP_NULL	0	/* NULL - magic value */
#define	TYP_UNDEF	1	/* we dont know what it is yet */
#define	TYP_NUM		2	/* an integer number */
#define	TYP_STR		3	/* a string */
#define	TYP_VAR		4	/* a variable name */
#define	TYP_OBJ		5	/* an object reference */
#define	TYP_ELEM	6	/* an object element */
#define	TYP_OLIST	7	/* an object reference list */
#define	TYP_FUNC	8	/* a function */
#define	TYP_BOBJHO	9	/* base obj 'root' (used only in store.c) */


/* error types we can find in the stack */
#define	ERR_NONE	0	/* no error */
#define	ERR_USER	1	/* user-defined error */
#define	ERR_OOM		2	/* out of memory */
#define	ERR_NUM		3	/* attempted numerical op on non-number */
#define	ERR_ZDIV	4	/* attempted division by zero */
#define	ERR_BADOBJ	5	/* badly formed object specifier */
#define	ERR_BADARG	6	/* invalid argument type */
#define	ERR_NOTHERE	7	/* no such object */
#define	ERR_REF		8	/* cannot dereference object type */
#define	ERR_NOVAL	9	/* function returned no value */
#define	ERR_NOPAR	10	/* attempt to use nonexistent parameter */
#define	ERR_DBASE	11	/* database error (disk problem!) */
#define	ERR_PERM	12	/* permission denied */
#define	ERR_NOTOWN	13	/* not owner */
#define	ERR_STACK	15	/* stack over/underflow */


/* macros to access parts of an element (packed into a string) */
#define	ELENUM(e)	(*(long *)(e))
#define	ELENAM(e)	((e) + sizeof(long))


/* a list of object references */
typedef	struct	{
	int	l_cnt;		/* total refs in list */
	int	l_hi;		/* high water mark */
	long	*l_data;	/* list data */
} ObjList;


/* an assembled program */
typedef	struct	{
	long	p_uid;		/* uid of prog owner (as above) */
	int	p_mode;		/* mode of prog (as above) */
	int	*p_mem;		/* program memory */
	char	*p_str;		/* start of string table */
	int	p_pc;		/* program instruction ptr */
	int	p_siz;		/* program memory size */
	int	s_siz;		/* string memory size */
} Prog;


/* a virtual stack memory data representation (one of everything) */
typedef	union	{
	char	*c;
	int	i;
	long	l;
	Prog	*p;
	ObjList	*ol;
} Oper;


/* temporary symbols */
typedef struct sym {
	char	*name;
	Oper	data;
	int	typ;
	struct	sym	*next;
} Sym;


/* a virtual stack memory component */
typedef	struct	{
	int	typ;		/* type of data object */
	Oper	oper;		/* data bucket */
	long	own;		/* the object root */
} MemOp;


/*
a stack frame for user-defined function calls.
*/
typedef	struct	{
	int	ret_sp;		/* where stack pointer should wind up */
	int	ret_ac;		/* # of args */
	long	cur_obj;	/* current object # of executing program */
	long	cur_uid;	/* current uid object # of executor */
	Sym	*syms;		/* symbol table */
} Frame;


/*
a runtime environment (stack + frames) - all done with pointers in
case this needs to be dynamic-sized someday.
*/
typedef	struct	{
	Frame	*m_fram;	/* funct call frames */
	MemOp	*m_mem;		/* machine memory */
	int	m_fp;		/* frame pointer */
	int	m_maxfp;	/* frame size */
	int	m_pc;		/* program counter */
	int	m_maxpc;	/* machine size */
} Machine;


/*
information block header used for storing objects on disk.
much of this layout is inspired by sys/stat.h
*/
typedef	struct	{
	int	mode;		/* permissions bits */
	long	owner;		/* object # of owner object */
	int	refcnt;		/* number of references to this object */
	int	typ;		/* type of the object */
	unsigned	dsiz;	/* size of the data in this object. */
	unsigned	msiz;	/* max allocation of this object */
	long	doff;		/* MAY store offset of more data */
	Oper	op;
} ObjIno;

#define	_INCL_UBERMUD_H
#endif

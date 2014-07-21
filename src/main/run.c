#include	"ubermud.h"
#include	"externs.h"


/*
Copyright(C) 1990, Marcus J. Ranum, All Rights Reserved.
This software may be freely used, modified, and redistributed,
as long as this copyright message is left intact, and this
software is not used to develop any commercial product, or used
in any product that is provided on a pay-for-use basis.
*/

/*
routines to handle running a compiled program in a virtual machine.
also handled are stack operations, calling of functions, stack
frames, and so on.
*/


/* some simple macros for manipulating machine addresses */
#define	CURRINST(p,pc)	((p)->p_mem[pc])
#define	CURRSTAK(m)	((m)->m_mem[(m)->m_pc])
#define	CURRSTAB(m)	((m)->m_fram[(m)->m_fp].syms)


/* used to flag a return/break out of if statement */
static	int	returning = 0;


/* used to flag a serious problem (stop the run) */
static	int	runstate = RUN_OK;

/* if CPU tick limiting is on, this is the limit counter */
#ifdef	LIMITRUNCPU
static	int	cputicks = 0;
#endif


/*
#define CPUTICKDEBUG
#define STACKDEBUG
#define RUNDEBUG
#define EXCEPTDEBUG
#define FRAMEDEBUG
*/



/*
modify the #self value on the machine stack - this SHOULD be built into
stack_push/stack_pop, but it'd be a royal pain.
*/
static	void
stack_setowner(m,own)
Machine	*m;
long	own;
{
	if(m->m_pc >= m->m_maxpc) {
		logf("stack_setowner: stack overflow!\n",0);
		runstate = RUN_STACK;
		return;
	}

	CURRSTAK(m).own = own;
#if	defined(RUNDEBUG) || defined(STACKDEBUG)
	printf("mark stacked element as object #%d\n",own);
#endif
}




/*
push a value on the machine stack
*/
static	void
stack_push(m,typ,val)
Machine	*m;
int	typ;
Oper	val;
{
	if(m->m_pc >= m->m_maxpc) {

		/*
		increment it ANYWAY! this is important
		since stack_pop will give an error when
		it is accessed off the top of the stack
		*/
		logf("stack_push: stack overflow: ",ltoa(CURRSTAK(m).own),
			"is the current object\n",0);
		m->m_pc++;
		runstate = RUN_STACK;
		return;
	}

	CURRSTAK(m).typ = typ;
	CURRSTAK(m).oper = val;
#if	defined(RUNDEBUG) || defined(STACKDEBUG)
	switch(typ) {
		case TYP_FUNC:
		printf("pushed function %d\n",val.p);
		break;

		case TYP_UNDEF:
		printf("pushed undef\n");
		break;

		case TYP_NULL:
		printf("pushed NULL (%d)\n",val.i);
		break;

		case TYP_NUM:
		printf("pushed num %d\n",val);
		break;

		case TYP_STR:
		printf("pushed str %s\n",val.c);
		break;

		case TYP_VAR:
		printf("pushed var %s\n",val.c);
		break;

		case TYP_OBJ:
		printf("pushed object reference %d\n",val.l);
		break;

		case TYP_OLIST:
		printf("pushed object list @%d\n",val.ol);
		break;

		case TYP_ELEM:
		printf("pushed elem %d.%s\n",ELENUM(val.c),ELENAM(val.c));
		break;

		default:
		printf("pushed unknown/illegal type!\n");
	}
#endif
	m->m_pc++;
}




/*
take a value off the machine stack
*/
static	int
stack_pop(m,op)
Machine	*m;
Oper	*op;
{
	if(--(m->m_pc) < 0) {
		logf("stack_pop: stack underflow!\n",0);
		op->i = ERR_STACK;
		runstate = RUN_STACK;
		return(TYP_NULL);
	}

	if(m->m_pc >= m->m_maxpc) {
		logf("stack_pop: stack overflow handled\n",0);
		op->i = ERR_STACK;
		runstate = RUN_STACK;
		return(TYP_NULL);
	}

	*op = CURRSTAK(m).oper;
#if	defined(RUNDEBUG) || defined(STACKDEBUG)
	switch(CURRSTAK(m).typ) {
		case TYP_FUNC:
		printf("popped function %d\n",CURRSTAK(m).oper.p);
		break;

		case TYP_UNDEF:
		printf("popped undef\n");
		break;

		case TYP_NULL:
		printf("popped NULL (%d)\n",CURRSTAK(m).oper.i);
		break;

		case TYP_NUM:
		printf("popped num %d\n",CURRSTAK(m).oper.i);
		break;

		case TYP_STR:
		printf("popped str %s\n",CURRSTAK(m).oper.c);
		break;

		case TYP_VAR:
		printf("popped var %s\n",CURRSTAK(m).oper.c);
		break;

		case TYP_OBJ:
		printf("popped object reference %d\n",CURRSTAK(m).oper.l);
		break;

		case TYP_OLIST:
		printf("popped object list @%d\n",CURRSTAK(m).oper.ol);
		break;

		case TYP_ELEM:
		printf("popped elem %d.%s\n",
			ELENUM(CURRSTAK(m).oper.c),
			ELENAM(CURRSTAK(m).oper.c));
		break;

		default:
		printf("popped unknown! object\n");
	}
#endif
	return(CURRSTAK(m).typ);
}




static	int
frame_push(m,ret_sp,ret_ac,onum)
Machine	*m;
int	ret_sp;
int	ret_ac;
long	onum;
{
	if(m->m_fp >= m->m_maxfp) {
		logf("frame_push: out of stack frames !\n",0);
		runstate = RUN_FRAME;
		return(-1);
	}
#ifdef	FRAMEDEBUG
	printf("push frame #%d, ret_sp=%d, ret_ac=%d, ",m->m_fp,ret_sp,ret_ac);
	printf("cur_obj=%ld, cur_uid=%ld, ",onum,*uid);
#endif
	m->m_fram[m->m_fp].ret_sp = ret_sp;
	m->m_fram[m->m_fp].ret_ac = ret_ac;
	m->m_fram[m->m_fp].cur_obj = onum;
	m->m_fram[m->m_fp].cur_uid = 0;
	m->m_fp++;
	CURRSTAB(m) = (Sym *)0;
	return(0);
}




static	Frame	*
frame_pop(m)
Machine	*m;
{
	if(CURRSTAB(m) != (Sym *)0) {
#ifdef	FRAMEDEBUG
		printf("free frame #%d symbols\n",m->m_fp);
#endif
		symfreelist(CURRSTAB(m));
	}

	if(--(m->m_fp) < 0) {
		logf("frame_pop: frame underflow (this is a disaster!)\n",0);
		runstate = RUN_FRAME;
		return((Frame *)0);
	}
#ifdef	FRAMEDEBUG
	printf("pop frame #%d\n",m->m_fp);
#endif
	return(&m->m_fram[m->m_fp]);
}




/*
handle comparision of equality or non-equality
*/
static	void
test_equal(d1t,d1,d2t,d2,op)
int	*d1t;
Oper	*d1;
int	*d2t;
Oper	*d2;
int	op;
{
	/* if one or the other is NULL... */
	if(*d1t == TYP_OBJ && *d2t == TYP_OBJ) {
		d1->i = (d1->l == d2->l);
		*d1t = TYP_NUM;
	} else
	if(*d1t == TYP_NULL || *d2t == TYP_NULL) {
		d1->i = (*d1t == *d2t);
		*d1t = TYP_NUM;
	} else
	if(*d1t == TYP_STR && *d2t == TYP_STR) {
		d1->i = !(strcmp(d1->c,d2->c));
		*d1t = TYP_NUM;
	} else
	if(*d1t != TYP_NUM || *d2t != TYP_NUM) {
		d1->i = 0;
		*d1t = TYP_NUM;
		return;
	} else
		d1->i = (d2->i == d1->i);

	if(op == OP_NE)
		d1->i = !(d1->i);

	*d1t = TYP_NUM;
}


/*
return a meaningful boolean value from an object
*/
static	int
test_bool(dt,d)
int	dt;
Oper	d;
{
	if(dt == TYP_NULL)
		return(0);
	if(dt == TYP_NUM && d.i == 0)
		return(0);
	if(dt == TYP_STR && *(d.c) == '\0')
		return(0);
	if(dt == TYP_OBJ && d.l == 0)
		return(0);
	if(dt == TYP_OLIST && d.ol->l_cnt == 0)
		return(0);
	return(1);
}



/*
handle basic binary numeric operations. returning nonzero indicates an
error condition (currently relevant only in division by zero).
*/
static	int
numeric_op(op,d1,d2)
int	op;
Oper	*d1;
Oper	*d2;
{
	switch(op) {
	case OP_ADD:
		d1->i = d2->i + d1->i;
		break;
	case OP_SUB:
		d1->i = d2->i - d1->i;
		break;
	case OP_DIV:
		if(d1->i == 0) {
			d2->i = ERR_ZDIV;
			return(1);
		} else {
			d1->i = d2->i / d1->i;
		}
		break;
	case OP_MUL:
		d1->i = d2->i * d1->i;
		break;
	case OP_LT:
		d1->i = (d2->i < d1->i);
		break;
	case OP_LTE:
		d1->i = (d2->i <= d1->i);
		break;
	case OP_GT:
		d1->i = (d2->i > d1->i);
		break;
	case OP_GTE:
		d1->i = (d2->i >= d1->i);
		break;
	}
	return(0);
}




/*
loop across a list of machine instructions, obeying them dutifully.
we *COULD* get some extra speed out of stuff like ASGN, EVAL, and
so on, by avoiding calls to stack_pop and stack_push, but that
kind of hackery can be done if it proves necessary.
*/
run(mach,prog,pc,uid,euid)
Machine	*mach;
Prog	*prog;
int	pc;
long	*uid;
long	*euid;
{
	Oper	d1;		/* stack data */
	Oper	d2;
	int	d1t;		/* types of stack data */
	int	d2t;
	Sym	*sp;		/* var ptr for locals. */
	ObjList	*lp;		/* misc. list pointer for for statements */
	long	rp;		/* misc. object ref */
	Frame	*framep;	/* misc. frame pointer */
	int	op;		/* saved operator */
	int	tmp1;
	int	tmp2;
	int	tmp3;
	int	runval;

#ifdef	RUNDEBUG
	printf("running prog %d at %d\n",prog,pc);
#endif

	/* unset returning */
	returning = 0;

	while(CURRINST(prog,pc) != OP_STOP) {
#ifdef	LIMITRUNCPU
		if(cputicks >= MAXCPUTICKS && *uid != (long)0 && *euid != (long)0) {
			runstate = RUN_CPU;
		}
		cputicks++;
#endif
		if(runstate != RUN_OK) {
#if	defined(RUNDEBUG) || defined(EXCEPTDEBUG)
			printf("RUN: bad run state, returning %d\n",runstate);
#endif
			return(runstate);
		}

		switch(op = CURRINST(prog,pc)) {


		/* handle binary numeric operations */
		case OP_ADD:
		case OP_SUB:
		case OP_DIV:
		case OP_MUL:
		case OP_LT:
		case OP_LTE:
		case OP_GT:
		case OP_GTE:
#ifdef	RUNDEBUG
		printf("num op %d at pc=%d in in prog %d\n",op,pc,prog);
#endif
		d1t = stack_pop(mach,&d1);
		d2t = stack_pop(mach,&d2);
		if(d1t != TYP_NUM || d2t != TYP_NUM) {
			d2.i = ERR_NUM;
			stack_push(mach,TYP_NULL,d2);
			break;
		}
		if(numeric_op(op,&d1,&d2)) {
			d2.i = ERR_NUM;
			stack_push(mach,TYP_NULL,d1);
		} else
			stack_push(mach,TYP_NUM,d1);
		break;



		/*
		assign to a local/temporary variable.
		*/
		case OP_ASGN:
#ifdef	RUNDEBUG
		printf("OP_ASGN at pc=%d in in prog %d\n",pc,prog);
#endif
		/* next instruction is name of variable */
		pc++;
		d1.c = prog->p_str + CURRINST(prog,pc);

		/* and take the value to assign to off the stack. */
		d2t = stack_pop(mach,&d2);

		sp = symlook(d1.c,CURRSTAB(mach));
		if(sp == 0) {
			sp = symnew(d1.c,d2t,d2);

			/* assume we are out of memory, or something */
			if(sp == (Sym *)0) {
				d2.i = ERR_OOM;
				stack_push(mach,TYP_NULL,d2);
				break;
			}

			CURRSTAB(mach) = symadd(sp,CURRSTAB(mach));
		}
		sp->typ = d2t;
		sp->data = d2;
		stack_push(mach,sp->typ,sp->data);
		break;



		case OP_CALL:
#ifdef	RUNDEBUG
		printf("OP_CALL at pc=%d in in prog %d\n",pc,prog);
#endif

		returning = 0;

		/* next instruction is the # of args */
		pc++;
		tmp2 = CURRINST(prog,pc);

		tmp3 = (mach->m_pc - tmp2) - 1;

#ifdef	RUNDEBUG
		printf("CALL - find func at %d\n",tmp3);
#endif

		/* stack our current running context */
		if(mach->m_mem[tmp3].typ != TYP_FUNC) {
#if	defined(RUNDEBUG) || defined(EXCEPTDEBUG)
			printf("CALL - function not found\n");
#endif
			/* not a function. manually fix up the stack */
			mach->m_mem[tmp3].typ = TYP_NULL;
			mach->m_mem[tmp3].oper.i = ERR_NOTHERE;
			mach->m_pc = tmp3 + 1;
		} else {
			if(frame_push(mach,tmp3,tmp2,mach->m_mem[tmp3].own)) {
#if	defined(RUNDEBUG) || defined(EXCEPTDEBUG)
				printf("CALL out of frames!\n");
#endif
				mach->m_mem[tmp3].typ = TYP_NULL;
				mach->m_mem[tmp3].oper.i = ERR_STACK;
				mach->m_pc = tmp3 + 1;
			} else {
				long	tmpu = *euid;
				long	savu = *uid;
				long	saveu = *euid;

				/* handle setuid functions */
				if(mach->m_mem[tmp3].oper.p->p_mode & PERM_SUID) {
					tmpu = mach->m_mem[tmp3].oper.p->p_uid;
#if	defined(RUNDEBUG) || defined(EXCEPTDEBUG)
					printf("CALL setuid as %d\n",tmpu);
#endif
				}

#ifdef	RUNDEBUG
			printf("CALL run(%d,%d,0)\n",mach,mach->m_mem[tmp3].oper.p);
#endif
				runval = run(mach,mach->m_mem[tmp3].oper.p,0,uid,&tmpu);
				if(runval != RUN_OK)
					return(runval);

				/* restore perms - at each level */
				*uid = savu;
				*euid = saveu;
			}
		}
		returning = 0;
		break;


	

		case OP_BLTIN:
#ifdef	RUNDEBUG
		printf("OP_BLTIN at pc=%d in in prog %d\n",pc,prog);
#endif

		/* next instruction is the # of the builtin */
		pc++;
		tmp1 = CURRINST(prog,pc);

		/* next instruction is the # of args */
		pc++;
		tmp2 = CURRINST(prog,pc);

		/* pretend it was a successful func. call */
		returning = 0;

		/* result goes in what is now the first arg */
		tmp3 = mach->m_pc - tmp2;
		(*bltintab[tmp1])(mach,tmp2,mach->m_pc - tmp2,tmp3,uid,euid);

		/* fake an increment of the stack pointer */
		mach->m_pc = tmp3 + 1;
		break;



		/*
		push the parameter count on the stack. this will only
		occur in a function(guaranteed). sneak a look at the
		stack frame, and push the arg count.
		*/
		case OP_CPUSH:
#ifdef	RUNDEBUG
		printf("OP_CPUSH at pc=%d in in prog %d\n",pc,prog);
#endif
		d2.i = mach->m_fram[mach->m_fp - 1].ret_ac;
		stack_push(mach,TYP_NUM,d2);
		break;



		case OP_ELPUSH:
		case OP_SELPUSH:
#ifdef	RUNDEBUG
		printf("OP_ELPUSH/SELPUSH at pc=%d in in prog %d\n",pc,prog);
#endif
		/* next instruction is name of variable */
		pc++;
		d1.c = prog->p_str + CURRINST(prog,pc);

		/* and the numeric part is on the stack */
		if(op == OP_ELPUSH) {
			d2t = stack_pop(mach,&d2);

			/* must be a number or an error */
			if(d2t != TYP_OBJ) {
				d2.i = ERR_BADOBJ;
				stack_push(mach,TYP_NULL,d2);
				break;
			}
			rp = d2.l;
		} else {
			rp = (long)0;
		}

		/* allocate storage */
		d2.c = tmpalloc(strlen(d1.c) + 1 + (int)sizeof(long));
		ELENUM(d2.c) = rp;
		(void)strcpy(ELENAM(d2.c),d1.c);
		stack_push(mach,TYP_ELEM,d2);
		break;



		/*
		the controversial string->varname operator 
		basically, just pull the front half of the
		element off the stack, and the back half,
		and type-check them, make sure they're a
		number and a string, and build an element
		name out of 'em. easy as that. (but a major
		issue in language design).
		*/
		case OP_STRVAR:
#ifdef	RUNDEBUG
		printf("OP_STRVAR at pc=%d in in prog %d\n",pc,prog);
#endif
		/* the string part is on the stack */
		d1t = stack_pop(mach,&d1);

		/* the numeric part is on the stack */
		d2t = stack_pop(mach,&d2);

		/* d2 must be a number or an error and d1 a string */
		if(d2t != TYP_OBJ || d1t != TYP_STR) {
			d2.i = ERR_BADOBJ;
			stack_push(mach,TYP_NULL,d2);
			break;
		}
		rp = d2.l;

		/* some insanity checking - no strings too long, please */
		if(strlen(d1.c) + 1 >= MAXIDENTLEN) {
			d2.i = ERR_BADOBJ;
			stack_push(mach,TYP_NULL,d2);
			break;
		}

		/* allocate storage and build the element buffer */
		d2.c = tmpalloc(strlen(d1.c) + 1 + (int)sizeof(long));
		ELENUM(d2.c) = rp;
		(void)strcpy(ELENAM(d2.c),d1.c);
		stack_push(mach,TYP_ELEM,d2);
		break;



		/*
		deref is a slimy (clever?) trick - by posing as a unary
		operator we allow the user to leave the name of an element
		on the stack. we simply check to ensure that the user
		has, in fact, provided us with a good element name.
		*/
		case OP_REF:
#ifdef	RUNDEBUG
		printf("OP_REF at pc=%d in in prog %d\n",pc,prog);
#endif
		d1t = stack_pop(mach,&d1);

		if(d1t != TYP_ELEM) {
			d2.i = ERR_REF;
			stack_push(mach,TYP_NULL,d2);
			break;
		}

		/* all is well, push it back */
		stack_push(mach,TYP_ELEM,d1);
		break;



		case OP_EEVAL:
#ifdef	RUNDEBUG
		printf("OP_EEVAL at pc=%d in in prog %d\n",pc,prog);
#endif
		d1t = stack_pop(mach,&d1);

		/* thaw out the element if it is one */
		if(d1t != TYP_ELEM) {
			d2.i = ERR_BADOBJ;
			stack_push(mach,TYP_NULL,d2);
			break;
		}
		if(ELENUM(d1.c) == 0) {
			int	thawval;

			if(thawval = sys_thaw(d1.c,&d2t,&d2,*uid,*euid)) {
				d2.i = thawval;
				stack_push(mach,TYP_NULL,d2);
				break;
			}
		} else {
			int	thawval;

			if(thawval = disk_thaw(d1.c,&d2t,&d2,*uid,*euid)) {
				d2.i = thawval;
				stack_push(mach,TYP_NULL,d2);
				break;
			}
		}

		/*
		MILD kludge.
		set the owner of the thing for #self.
		This MUST be done before the stack_push!
		*/
		stack_setowner(mach,ELENUM(d1.c));
		stack_push(mach,d2t,d2);
		break;



		case OP_EASGN:
		case OP_ECASGN:
#ifdef	RUNDEBUG
		printf("OP_EASGN/ECASGN at pc=%d in in prog %d\n",pc,prog);
#endif
		d1t = stack_pop(mach,&d1);
		d2t = stack_pop(mach,&d2);

		/* freeze the element if it is one */
		if(d2t != TYP_ELEM || (op == OP_ECASGN && ELENUM(d2.c) == (long)0)) {
			d2.i = ERR_BADOBJ;
			stack_push(mach,TYP_NULL,d2);
			break;
		}

		if(ELENUM(d2.c) == (long)0) {
			int	frzval;

			if(frzval = sys_freeze(d2.c,d1t,d1,*uid,*euid)) {
				d2.i = frzval;
				stack_push(mach,TYP_NULL,d2);
				break;
			}
		} else {
			int	frzval;

			if(frzval = disk_freeze(d2.c,d1t,d1,op,*uid,*euid)) {
				d2.i = frzval;
				stack_push(mach,TYP_NULL,d2);
				break;
			}
		}
		stack_push(mach,d1t,d1);
		break;



		case OP_EQ:
		case OP_NE:
#ifdef	RUNDEBUG
		printf("OP_EQ/NE at pc=%d in in prog %d\n",pc,prog);
#endif
		d1t = stack_pop(mach,&d1);
		d2t = stack_pop(mach,&d2);

		/* some operator overloading is done here */
		test_equal(&d1t,&d1,&d2t,&d2,op);
		stack_push(mach,d1t,d1);
		break;



		case OP_NULPUSH:
#ifdef	RUNDEBUG
		printf("NULPUSH at pc=%d in in prog %d\n",pc,prog);
#endif
		d2.i = ERR_USER;
		stack_push(mach,TYP_NULL,d2);
		break;



		case OP_EVAL:
#ifdef	RUNDEBUG
		printf("OP_EVAL at pc=%d in in prog %d\n",pc,prog);
#endif
		/* next instruction is name of variable */
		pc++;
		d1.c = prog->p_str + CURRINST(prog,pc);

		sp = symlook(d1.c,CURRSTAB(mach));
		if(sp == 0) {
			d2.i = ERR_NOTHERE;
			stack_push(mach,TYP_NULL,d2);
		} else {
			stack_push(mach,sp->typ,sp->data);
		}
		break;



		/*
		cope with an if statement by unrolling the address
		in the program to jump to, making a recursive (gag, choke)
		call to run() with that program counter, and returning
		to execute.
		*/
		case OP_IF:
#ifdef	RUNDEBUG
		printf("OP_IF at pc=%d in in prog %d\n",pc,prog);
#endif
		pc++;
		tmp1 = CURRINST(prog,pc);	/* if/true part */
		pc++;
		tmp2 = CURRINST(prog,pc);	/* if/else part */
		pc++;
		tmp3 = CURRINST(prog,pc);	/* continuation */
#ifdef	RUNDEBUG
		printf("IF cond=%d, ifpart=%d, else=%d, cont=%d\n",
		pc,tmp1,tmp2,tmp3);
#endif

		/*
		this is subtle. incrementing the program counter points it
		to the assembled code for the condition. run the condition
		and resume operation at the continuation branch.
		*/
		pc++;

		if((runval = run(mach,prog,pc,uid,euid)) != RUN_OK)
			return(runval);

		d1t = stack_pop(mach,&d1);
		if(test_bool(d1t,d1)) {
			if(tmp1 != 0) {
				runval = run(mach,prog,tmp1,uid,euid);
				if(runval != RUN_OK)
					return(runval);
			}
		} else {
			if(tmp2 != 0) {
				runval = run(mach,prog,tmp2,uid,euid);
				if(runval != RUN_OK)
					return(runval);
			}
		}

		if(returning)
			return(0);

		/* cheat. this will be post-incremented, so frob it */
		pc = tmp3 - 1;
		break;



		case OP_FOR:
#ifdef	RUNDEBUG
		printf("OP_FOR at pc=%d in in prog %d\n",pc,prog);
#endif
		/*
		dt2 is used here as a scratch to hold the name of
		the symbol we stuff the values into.
		*/
		pc++;
		d2.c = prog->p_str + CURRINST(prog,pc);	/* name of var */
		pc++;
		tmp1 = CURRINST(prog,pc);	/* do-part */
		pc++;
		tmp2 = CURRINST(prog,pc);	/* continuation */

		/*
		as in the OP_IF, the next value past the program 
		counter is the condition (in this case the loop
		list generator
		*/
		pc++;

		/* run the machine once to generate a list. */
		if((runval = run(mach,prog,pc,uid,euid)) != RUN_OK)
			return(runval);

		/* the thing on the stack had BETTER be a list */
		d1t = stack_pop(mach,&d1);
		if(d1t != TYP_OLIST) {
			pc = tmp2 - 1;
			break;
		}

		/* now locate - or create the iterator variable. */
		sp = symlook(d2.c,CURRSTAB(mach));
		if(sp == 0) {
			sp = symnew(d2.c,TYP_OBJ,d2);
			if(sp == (Sym *)0)
				break;
			CURRSTAB(mach) = symadd(sp,CURRSTAB(mach));
		}

		/* tmp3 is used as list iterator */
		if((lp = listtmpcopy(d1.ol)) == (ObjList *)0)
			return(1);
		
		tmp3 = 0;
		while(tmp3 < lp->l_cnt) {

			/* reset the iterator for each loop */
			sp->data.l = lp->l_data[tmp3];
#ifdef	RUNDEBUG
			printf("OP_FOR iterator %s now %d\n",sp->name,sp->data.l);
#endif
			if(tmp1 != 0) {
				runval = run(mach,prog,tmp1,uid,euid);
				if(runval != RUN_OK)
					return(runval);
				if(returning) {
					return(0);
				}
			}
			tmp3++;
		}

		/* now continue */
		pc = tmp2 - 1;
		break;



		case OP_FORARG:
#ifdef	RUNDEBUG
		printf("OP_FORARG at pc=%d in in prog %d\n",pc,prog);
#endif
		/*
		d2 is used here as a scratch to hold the name of
		the symbol we stuff the values into.
		*/
		pc++;
		d2.c = prog->p_str + CURRINST(prog,pc);	/* name of var */

		pc++;
		tmp2 = CURRINST(prog,pc);	/* continuation */

		pc++;

		sp = symlook(d2.c,CURRSTAB(mach));
		if(sp == 0) {
			sp = symnew(d2.c,TYP_OBJ,d2);
			if(sp == (Sym *)0)
				break;
			CURRSTAB(mach) = symadd(sp,CURRSTAB(mach));
		}

		tmp3 = 0;

		while(tmp3 < mach->m_fram[mach->m_fp - 1].ret_ac) {
			int	sa;

			/* reset the iterator for each loop */
			sa = mach->m_fram[mach->m_fp - 1].ret_sp + tmp3 + 1;
			sp->data = mach->m_mem[sa].oper;
			sp->typ = mach->m_mem[sa].typ;
#ifdef	RUNDEBUG
			printf("OP_FORARG iterator %s now %d, type=%d, val=%d\n",sp->name,tmp3,sp->typ,sp->data);
#endif
			runval = run(mach,prog,pc,uid,euid);
			if(runval != RUN_OK)
				return(runval);
			if(returning) {
				return(0);
			}
			tmp3++;
		}

		/* now continue */
		pc = tmp2 - 1;
		break;



		/*
		push a numeric value from program space onto the stack.
		*/
		case OP_NPUSH:
#ifdef	RUNDEBUG
		printf("OP_NPUSH at pc=%d in in prog %d\n",pc,prog);
#endif
		pc++;
		d2.i = CURRINST(prog,pc);
		stack_push(mach,TYP_NUM,d2);
		break;



		/*
		pop (and throw away) a value off the stack
		*/
		case OP_POP:
#ifdef	RUNDEBUG
		printf("OP_POP at pc=%d in in prog %d\n",pc,prog);
#endif
		(void)stack_pop(mach,&d1);
		break;





		/*
		drop off the end of a function - paste the stack together
		*/
		case OP_RETF:
#ifdef	RUNDEBUG
		printf("OP_RETF at pc=%d in in prog %d\n",pc,prog);
#endif
		if((framep = frame_pop(mach)) == 0)
			return(-1);
		mach->m_pc = framep->ret_sp;
		d2.i = ERR_NOVAL;
		stack_push(mach,TYP_NULL,d2);
		returning = 0;
		return(0);





		/*
		return from a function call.
		this is done by popping a stack frame, using it
		to reset the stack, and pushing a fake value
		onto the stack, or a real one, depending.
		*/
		case OP_RETNV:
		case OP_RETV:
#ifdef	RUNDEBUG
		printf("OP_RETNV/RETV at pc=%d in in prog %d\n",pc,prog);
#endif
		/* if returning a value, acquire a value to return. */
		if(op == OP_RETV)
			d1t = stack_pop(mach,&d1);

		if((framep = frame_pop(mach)) == 0)
			return(-1);
		mach->m_pc = framep->ret_sp;
		if(op == OP_RETNV) {
			d2.i = ERR_NOVAL;
			stack_push(mach,TYP_NULL,d2);
		} else {
			stack_push(mach,d1t,d1);
		}
		returning++;
		return(0);



		/*
		push #0
		*/
		case OP_ROOTPUSH:
#ifdef	RUNDEBUG
		printf("OP_ROOTPUSH at pc=%d in in prog %d\n",pc,prog);
#endif
		d2.l = (long)0;
		stack_push(mach,TYP_OBJ,d2);
		break;



		/*
		push the id# of an object
		*/
		case OP_RPUSH:
#ifdef	RUNDEBUG
		printf("OP_RPUSH at pc=%d in in prog %d\n",pc,prog);
#endif
		pc++;
		d2.l = CURRINST(prog,pc);
		stack_push(mach,TYP_OBJ,d2);
		break;



		case OP_ACTPUSH:
#ifdef	RUNDEBUG
		printf("OP_ACTPUSH at pc=%d in in prog %d\n",pc,prog);
#endif
		d1.l = *uid;
		stack_push(mach,TYP_OBJ,d1);
		break;



		case OP_SELFPUSH:
#ifdef	RUNDEBUG
		printf("OP_SELFPUSH at pc=%d in in prog %d\n",pc,prog);
#endif
		if(mach->m_fp == 0)
			d1.l = *uid;
		else
			d1.l = mach->m_fram[mach->m_fp - 1].cur_obj;
		stack_push(mach,TYP_OBJ,d1);
		break;



		case OP_CALLPUSH:
#ifdef	RUNDEBUG
		printf("OP_CALLPUSH at pc=%d in in prog %d\n",pc,prog);
#endif
		if(mach->m_fp < 2)
			d1.l = *uid;
		else
			d1.l = mach->m_fram[mach->m_fp - 2].cur_obj;
		stack_push(mach,TYP_OBJ,d1);
		break;




		/*
		push a parameter value onto the stack. this will only
		occur in a function(guaranteed). sneak a look at the
		stack frame, find the offset to the value in question,
		and push it again - unless it is invalid.
		*/
		case OP_PPUSH:
#ifdef	RUNDEBUG
		printf("OP_PPUSH at pc=%d in in prog %d\n",pc,prog);
#endif
		pc++;

		/* sanity chex */
		if(CURRINST(prog,pc) > mach->m_fram[mach->m_fp - 1].ret_ac || CURRINST(prog,pc) <= 0) {
			d2.i = ERR_NOPAR;
			stack_push(mach,TYP_NULL,d2);
		} else {
			int	sa;
			sa = mach->m_fram[mach->m_fp - 1].ret_sp + CURRINST(prog,pc);
			stack_push(mach,mach->m_mem[sa].typ,mach->m_mem[sa].oper);
		}
		break;



		/*
		push a string onto the stack from program space.
		use the index into the program string space
		(the next value in the program space) to get
		a pointer to the string, which is pushed onto
		the stack.
		*/
		case OP_SPUSH:
		case OP_VPUSH:
#ifdef	RUNDEBUG
		printf("OP_SPUSH at pc=%d in in prog %d\n",pc,prog);
#endif
		if(op == OP_SPUSH)
			d1t = TYP_STR;
		else
			d1t = TYP_VAR;
		pc++;
		d2.c = prog->p_str + CURRINST(prog,pc);
		stack_push(mach,d1t,d2);
		break;



		case OP_AND:
		case OP_OR:
#ifdef	RUNDEBUG
		printf("OP_OR/AND at pc=%d in in prog %d\n",pc,prog);
#endif
		d1t = stack_pop(mach,&d1);
		d2t = stack_pop(mach,&d2);

		if(op == OP_AND)
			d1.i = (test_bool(d1t,d1) && test_bool(d2t,d2));
		else
			d1.i = (test_bool(d1t,d1) || test_bool(d2t,d2));
		stack_push(mach,TYP_NUM,d1);
		break;





		/*
		negate/invert by popping top value, neg/inverting it, and
		pushing the result.
		*/
		case OP_NEG:
#ifdef	RUNDEBUG
		printf("OP_NEG at pc=%d in in prog %d\n",pc,prog);
#endif
		d1t = stack_pop(mach,&d1);
		if(d1t != TYP_NUM) {
			d2.i = ERR_NUM;
			stack_push(mach,TYP_NULL,d2);
			break;
		}
		d1.i = - (d1.i);
		stack_push(mach,d1t,d1);
		break;





		/*
		invert by popping top value, inverting it, and
		pushing the result.
		*/
		case OP_NOT:
#ifdef	RUNDEBUG
		printf("OP_NOT at pc=%d in in prog %d\n",pc,prog);
#endif
		d1t = stack_pop(mach,&d1);
		d2.i = !(test_bool(d1t,d1));
		stack_push(mach,TYP_NUM,d2);
		break;



		default:
		logf("run: unknown opcode ",itoa(CURRINST(prog,pc)),"\n",0);
		runstate = RUN_ILLEGAL;
		}
		pc++;
#ifdef	STACKDEBUG
		stack_dump(mach);
#endif
	}

#ifdef	RUNDEBUG
	/* reached when we hit OP_STOP */
	if(mach->m_pc != 0 && mach->m_fp == 0)
		printf("WARNING stack not drained!! ptr=%d\n",mach->m_pc);
	printf("returned from run, returning=%d\n",returning);
#endif
	returning = 0;
	return(0);
}



void
resetmachine(m)
Machine	*m;
{
#ifdef	RUNDEBUG
	printf("reset machine\n");
#endif

#ifdef	CPUTICKDEBUG
	if(cputicks != 0)
		printf("CPU ticks used: %d ticks\n",cputicks);
#endif
	/* clobber the symbols at lev. 0 if any */
	m->m_fp = 0;
	if(CURRSTAB(m) != (Sym *)0)
		symfreelist(CURRSTAB(m));
	CURRSTAB(m) = (Sym *)0;
	m->m_pc = 0;
	cputicks = 0;
	runstate = RUN_OK;
}


#ifdef	STACKDEBUG
stack_dump(m)
Machine	*m;
{
	int	f;
	
	printf("stack pointer is at ::%d\n",m->m_pc);
	for(f = m->m_pc - 1; f >= 0; f--) {
		switch(m->m_mem[f].typ) {
			case TYP_FUNC:
			printf("FUNC %d\t%d\n",f,m->m_mem[f].oper.p);
			break;

			case TYP_NULL:
			printf("NULL %d\n",f);
			break;

			case TYP_ELEM:
			printf("ELEM %d\t%d.%s\n",f,
				ELENUM(m->m_mem[f].oper.c),
				ELENAM(m->m_mem[f].oper.c));
			break;

			case TYP_OBJ:
			printf("OBJ %d\t%d\n",f,m->m_mem[f].oper.l);
			break;

			case TYP_OLIST:
			printf("OBJLIST %d\t#items%d\n",f,m->m_mem[f].oper.ol->l_cnt);
			break;

			case TYP_STR:
			printf("STR %d\t%s\n",f,m->m_mem[f].oper.c);
			break;

			case TYP_VAR:
			printf("VAR %d\t%s\n",f,m->m_mem[f].oper.c);
			break;

			case TYP_NUM:
			printf("NUM %d\t%d\n",f,m->m_mem[f].oper.i);
			break;

			default:
			printf("???(%d) %d\t%d\n",m->m_mem[f].typ,f,m->m_mem[f].oper.i);
		}
	}
}
#endif

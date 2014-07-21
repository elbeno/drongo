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
routines that handle copying stuff in memory, and other main
memory allocation shenanigans.
*/


/*
#define	ALLOCDEBUG
*/


/*
temporary data is managed by keeping a list of allocated stuff, which
can be traversed and freed. this is not too terribly efficient, but
is not a load-bearing section of code.
*/
struct	tmplist	{
	struct	tmplist	*next;
	char	*dat;
};


/* list of currently active temporaries */
static	struct	tmplist	*tmpl;

/* list of temp list structures that have been freed and stacked */
static	struct	tmplist	*freetmpl;



/* just like malloc() but keep track of the storage */
char	*
tmpalloc(siz)
int	siz;
{
	struct	tmplist	*lp;

	if(freetmpl != (struct tmplist *)0) {
		lp = freetmpl;
#ifdef	ALLOCDEBUG
		printf("take holder %d from tmp holder list\n",lp);
#endif
		freetmpl = freetmpl->next;
	} else {
		if((lp = (struct tmplist *)malloc(sizeof(struct tmplist))) == 0)
			return(0);
	}
	lp->dat = malloc((unsigned)siz);

	/* add to the chain */
	lp->next = tmpl;
	tmpl = lp;

#ifdef	ALLOCDEBUG
	printf("tmpalloc %d bytes at %d (holder is %d)\n",siz,lp->dat,lp);
#endif
	return(lp->dat);
}




/*
put something on the temp list to free later.
this results in what is basically a deferred free() - executed
at the end of each machine run.
*/
void
tmpputonfree(p)
char	*p;
{
	struct	tmplist	*lp;

	if(freetmpl != (struct tmplist *)0) {
		lp = freetmpl;
#ifdef	ALLOCDEBUG
		printf("take holder %d from tmp holder list\n",lp);
#endif
		freetmpl = freetmpl->next;
	} else {
		if((lp = (struct tmplist *)malloc(sizeof(struct tmplist))) == 0)
			return;
	}
	lp->dat = p;
	lp->next = tmpl;
	tmpl = lp;

#ifdef	ALLOCDEBUG
	printf("tmpputonfree ? bytes at %d (holder is %d)\n",lp->dat,lp);
#endif
}




/* free all the temporaries */
void
tmpfree()
{
	register struct	tmplist	*lp = tmpl;
	register struct	tmplist	*np;

	if(tmpl == (struct tmplist *)0)
		return;

	while(lp != (struct tmplist *)0) {
		if(lp->dat != 0)
			(void)free(lp->dat);
#ifdef	ALLOCDEBUG
		printf("tmpfree at %d (holder is %d)\n",lp->dat,lp);
#endif
		np = lp->next;
		lp->next = freetmpl;
		freetmpl = lp;
#ifdef	ALLOCDEBUG
		printf("stack holder %d on tmp holder list\n",lp);
#endif
		lp = np;
	}
	tmpl = (struct tmplist *)0;
}




/* allocate and copy a string that is NOT temporary */
char	*
copystr(s)
char	*s;
{
	int	alen;
	char	*ret;
	alen = strlen(s) + 1;
	ret = malloc((unsigned)alen);
	if(ret == (char *)0)
		return((char *)0);
	bcopy(s,ret,alen);
	return(ret);
}



/* freeze a copy of a program into an already allocated buffer.  */
Prog	*
progcopyinto(p,r)
Prog	*p;
char	*r;
{
	Prog	*ret = (Prog *)r;

	/* copy the prog in at the head of the buffer */
	bcopy((char *)p,r,sizeof(Prog));

	/* next in, the program memory... */
	bcopy((char *)p->p_mem,&r[sizeof(Prog)],p->p_siz * (int)sizeof(int));
	ret->p_mem = (int *)&r[sizeof(Prog)];
#ifdef	ALLOCDEBUG
	printf("...function memory is at %d\n",ret->p_mem);
#endif

	/* last in, the program string table... */
	bcopy(p->p_str,&r[sizeof(Prog) + (p->p_siz * sizeof(int))],p->s_siz);
	ret->p_str = &r[sizeof(Prog) + (p->p_siz * sizeof(int))];
#ifdef	ALLOCDEBUG
	printf("...function string table is at %d\n",ret->p_str);
#endif
	return(ret);
}



/* freeze a copy of a program into a contiguous buffer.  */
Prog	*
progcopy(p)
Prog	*p;
{
	char	*r;

	/*
	we do some wild and crazy mallocing and pasting here !
	it should be quite portable, however.
	*/
	if((r = malloc((unsigned)(PROGSIZE(p)))) == 0)
		return(0);
	return(progcopyinto(p,r));
}




ObjList	*
listcopyinto(l,p)
ObjList	*l;
char	*p;
{
	ObjList	*ret = (ObjList *)p;

	/* copy the list header in at the head of the buffer */
	bcopy((char *)l,p,sizeof(ObjList));

	bcopy((char *)l->l_data,&p[sizeof(ObjList)],l->l_cnt * (int)sizeof(long));
	ret->l_data = (long *)&p[sizeof(ObjList)];
	return(ret);
}




ObjList	*
listcopy(l)
ObjList	*l;
{
	char	*p;

	if((p = malloc((unsigned)LISTSIZE(l))) == (char *)0)
		return(0);
	return(listcopyinto(l,p));
}




ObjList	*
listtmpcopy(l)
ObjList	*l;
{
	char	*p;

	if((p = tmpalloc(LISTSIZE(l))) == (char *)0)
		return(0);

	return(listcopyinto(l,p));
}



/*
return the # of bytes of EXTERNAL storage needed.
numerical data doesn't need any, remember ?
*/
operdatasize(typ,op)
int	typ;
Oper	op;
{
	if(typ == TYP_STR)
		return(strlen(op.c) + 1);
	if(typ == TYP_OLIST)
		return(LISTSIZE(op.ol));
	if(typ == TYP_FUNC)
		return(PROGSIZE(op.p));
	return(0);
}




/*
copy an oper, allocating memory if needed.
*/
opercopy(old,otyp,new,ntyp,nsiz)
Oper		old;
int		otyp;
Oper		*new;
int		*ntyp;
unsigned	*nsiz;
{
	*ntyp = TYP_NULL;

	switch(otyp) {
	case TYP_STR:
		if((new->c = copystr(old.c)) == (char *)0)
			return(1);
		*nsiz = strlen(old.c) + 1;
		break;

	case TYP_OLIST:
		if((new->ol = listcopy(old.ol)) == (ObjList *)0)
			return(1);
		*nsiz = LISTSIZE(old.ol);
		break;

	case TYP_FUNC:
		if((new->p = progcopy(old.p)) == (Prog *)0)
			return(1);
		*nsiz = PROGSIZE(old.p);
		break;

	default:
		*new = old;
		*nsiz = 0;
	}
	*ntyp = otyp;
	return(0);
}

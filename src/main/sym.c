#include	"ubermud.h"
#include	"syssym.h"
#include	"externs.h"


/*
Copyright(C) 1990, Marcus J. Ranum, All Rights Reserved.
This software may be freely used, modified, and redistributed,
as long as this copyright message is left intact, and this
software is not used to develop any commercial product, or used
in any product that is provided on a pay-for-use basis.
*/

/*
	routines for managing local function symbol tables.
*/


/*
#define FREEZEDEBUG
#define THAWDEBUG
#define SYMDEBUG
*/

/* free entries are stacked, not freed */
static	Sym	*freel = (Sym *)0;


/* system symbol table */
static	SysSym	*systable[SYSSYMWIDTH];
static	SysSym	*syssymfree = (SysSym *)0;



Sym	*
symnew(name,typ,op)
char	*name;
int	typ;
Oper	op;
{
	Sym	*ret;
	if(freel != (Sym *)0) {
		ret = freel;
		freel = freel->next;
#ifdef	SYMDEBUG
		printf("symnew return Sym holder %d\n",ret);
#endif
	} else {
		if((ret = (Sym *)malloc(sizeof(Sym))) == 0)
			return(0);
#ifdef	SYMDEBUG
		printf("symnew malloc Sym holder %d\n",ret);
#endif
	}
	ret->data = op;
	ret->typ = typ;
	ret->name = name;
	return(ret);
}


Sym	*
symadd(sym,table)
Sym	*sym;
Sym	*table;
{
	sym->next = table;
	return(sym);
}


Sym	*
symlook(str,table)
char	*str;
Sym	*table;
{
	Sym	*ret;
	for(ret = table; ret != (Sym *)0; ret = ret->next)
		if(str != 0 && !strcmp(str,ret->name))
			return(ret);
	return(0);
}



void
symfreelist(l)
Sym	*l;
{
	Sym	*p;
	
	p = l;
	while(l != 0) {
		l = l->next;
#ifdef	SYMDEBUG
		printf("symfree stack Sym holder %d\n",p);
#endif
		p->next = freel;
		freel = p;
		p = l;
	}
}



int
cornedbeefhash(s,siz)
char	*s;
int	siz;
{
	register unsigned int	c = 0;
	register unsigned int	g;
	while(*s) {
		c = (c << 1) + *s++;
		if(g = c & 0xF0000000) {
			c = c ^ (g >> 24);
			c = c ^ g;
		}
	}
	c = c % 211;
	return(c % siz);
}



syssyminit()
{
	int	x;

	for(x = 0; x < SYSSYMWIDTH; x++)
		systable[x] = (SysSym *)0;
	return(0);
}



/*
chill a system variable.
*/
sys_freeze(ele,typ,data,uid,euid)
char	*ele;
int	typ;
Oper	data;
long	uid;
long	euid;
{

	register SysSym	*sp;
	int	hv;


#ifdef	FREEZEDEBUG
	printf("freeze:elem %d.%s\n",ELENUM(ele),ELENAM(ele));
#endif

	if(*ELENAM(ele) == '_' && uid != (long)0 && euid != (long)0)
		return(ERR_PERM);

	sp = systable[(hv = cornedbeefhash(ELENAM(ele),SYSSYMWIDTH))];
	while(sp != (SysSym *)0) {
		if(!strcmp(ELENAM(ele),sp->nam))
			break;
		sp = sp->next;
	}


	/*
	if the variable is not defined, simply create it
	otherwise we need to clean up the old version and
	adjust it.
	*/
	if(sp == (SysSym *)0 && typ != TYP_NULL) {

		/* only uid#0 can create in system table */
		if(uid != (long)0 && euid != (long)0)
			return(ERR_NOTOWN);

		if(syssymfree != (SysSym *)0) {
			sp = syssymfree;
			syssymfree = sp->next;
			sp->next = (SysSym *)0;
		} else {
			sp = (SysSym *)malloc(sizeof(SysSym));
			if(sp == (SysSym *)0)
				return(ERR_OOM);
		}
		if((sp->nam = copystr(ELENAM(ele))) == 0)
			return(ERR_OOM);

		sp->next = systable[hv];
		systable[hv] = sp;
#ifdef	FREEZEDEBUG
		printf("freeze:added elem to system sym table\n");
#endif
		sp->ino.mode = PERM_DEFAULT;
		sp->ino.owner = (long)0;
	} else {
		if(sp == (SysSym *)0 && typ == TYP_NULL)
			return(0);

		/*
		free any existing stuff.
		none of this may be allocated with tmpstr,etc.
		an implicit cast between char *, Prog *, and Objlist * 
		is made here. SO SUE ME!
		*/
		if(!permitted(PERM_WRITE,sp->ino.owner,sp->ino.mode,uid,euid))
			return(ERR_PERM);

		/*  free data if there is any. - a deferred free() */
		if(sp->ino.dsiz > 0)
			tmpputonfree(sp->ino.op.c);

		/* de-assign the sucker. */
		if(typ == TYP_NULL) {
			SysSym	*jp;

			free(sp->nam);
			jp = systable[hv];
			if(jp == sp) {
				systable[hv] = sp->next;
			} else {
				while(jp != (SysSym *)0 && jp->next != sp)
					jp = jp->next;
				if(jp != (SysSym *)0)
					jp->next = sp->next;
				sp->next = syssymfree;
				syssymfree = sp;
			}
			return(0);
		}
	}

	if(opercopy(data,typ,&(sp->ino.op),&(sp->ino.typ),&(sp->ino.dsiz)))
		return(1);

	/* back-patch functions */
	if(typ == TYP_FUNC) {
		sp->ino.op.p->p_uid = sp->ino.owner;
		sp->ino.op.p->p_mode = sp->ino.mode;
	}

#ifdef	FREEZEDEBUG
	printf("installed sys \"%s\", size=%d\n",sp->nam,sp->ino.dsiz);
#endif
	return(0);
}





sys_thaw(ele,typ,data,uid,euid)
char	*ele;
int	*typ;
Oper	*data;
long	uid;
long	euid;
{
	register SysSym	*sp;
#ifdef	THAWDEBUG
	printf("thaw:elem %d.%s\n",ELENUM(ele),ELENAM(ele));
#endif

	sp = systable[cornedbeefhash(ELENAM(ele),SYSSYMWIDTH)];
	while(sp != (SysSym *)0) {
		if(!strcmp(ELENAM(ele),sp->nam))
			break;
		sp = sp->next;
	}
	if(sp == (SysSym *)0)
		return(ERR_NOTHERE);

	if(!permitted(PERM_READ,sp->ino.owner,sp->ino.mode,uid,euid))
		return(ERR_PERM);

	*typ = sp->ino.typ;
	*data = sp->ino.op;
	return(0);
}



/*
used by blt_chmod and blt_chown in store.c
*/
SysSym	*
sys_getsym(ele)
char	*ele;
{
	register SysSym	*sp;
#ifdef	THAWDEBUG
	printf("sys_getsym:elem %d.%s\n",ELENUM(ele),ELENAM(ele));
#endif

	sp = systable[cornedbeefhash(ELENAM(ele),SYSSYMWIDTH)];
	while(sp != (SysSym *)0) {
		if(!strcmp(ELENAM(ele),sp->nam))
			break;
		sp = sp->next;
	}
	return(sp);
}

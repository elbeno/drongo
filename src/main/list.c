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
list manipulation functions. there are 2 more listcopy() and listfree() 
declared in alloc.c - which deal only with memory allocation type stuff.
*/



/*
note - in all these functions, you CANNOT assign a stack value
to the result argument, until AFTER you are done processing
all the stuff in the stack - because result is ALSO firstarg
in some cases, and assigning result will overwrite the argument
stack
*/


/*
since we use tmpalloc here, there is no need to worry about freeing
stuff up. Lists that get copied into permanent storage will need to
re-malloc this amount, and save it. allow a 'hint' as to the size
of the list data space that will be needed.
*/
static	ObjList	*
listnew(nmem)
int	nmem;
{
	ObjList	*ret;

	if((ret = (ObjList *)tmpalloc((int)sizeof(ObjList))) == 0)
		return(0);
	if(nmem > 0) {
		ret->l_data = (long *)tmpalloc((int)sizeof(long) * nmem);
		if(ret->l_data == 0)
			return(0);
	}
	ret->l_hi = nmem;
	ret->l_cnt = 0;
	return(ret);
}



/*
add elements to an existing list. first arg is (must be) list, remaining are
elements to add.
*/
void
blt_listappend(m,argc,firstarg,result,uid,euid)
Machine	*m;
int	argc;
int	firstarg;
int	result;
long	*uid;
long	*euid;
{
	ObjList	*ret;
	long	*refp;
	long	*olp;
	long	*olp2;
	int	oc;
	int	x;

	if(m->m_mem[firstarg].typ != TYP_OLIST && m->m_mem[firstarg].typ != TYP_NULL) {
		m->m_mem[result].oper.i = ERR_BADARG;
		m->m_mem[result].typ = TYP_NULL;
		return;
	}

	if(m->m_mem[firstarg].typ != TYP_NULL) {
		olp2 = olp = m->m_mem[firstarg].oper.ol->l_data;
		oc = m->m_mem[firstarg].oper.ol->l_cnt;
	} else {
		olp2 = 0;
		oc = 0;
	}

	if((ret = listnew(oc + argc - 1)) == (ObjList *)0) {
		m->m_mem[result].typ = TYP_NULL;
		m->m_mem[result].oper.i = ERR_OOM;
		return;
	}

	ret->l_cnt = 0;
	refp = ret->l_data;

	/* copy all the old list elems */
	for(x = 0; x < oc; x++) {
		*refp++ = *olp++;
		ret->l_cnt++;
	}

	/* skip first arg, remember */
	for(x = 1; x < argc; x++) {
		int	y;
		int	skip = 0;
		long	*xx = olp2;

		for(y = 0; y < oc; y++) {
			if(m->m_mem[firstarg + x].typ != TYP_OBJ ||
				m->m_mem[firstarg + x].oper.l == *xx) {
				skip++;
			}
			xx++;
		}

		if(!skip) {
			*refp++ = m->m_mem[firstarg + x].oper.l;
			ret->l_cnt++;
		}
	}

	m->m_mem[result].oper.ol = ret;
	m->m_mem[result].typ = TYP_OLIST;
}




/*
count the # of elements in a list.
*/
void
blt_listcount(m,argc,firstarg,result,uid,euid)
Machine	*m;
int	argc;
int	firstarg;
int	result;
long	*uid;
long	*euid;
{
	if(argc != 1 || m->m_mem[firstarg].typ != TYP_OLIST) {
		m->m_mem[result].oper.i = ERR_BADARG;
		m->m_mem[result].typ = TYP_NULL;
		return;
	}
	if(m->m_mem[firstarg].oper.ol->l_cnt == 0) {
		m->m_mem[result].oper.i = ERR_NOTHERE;
		m->m_mem[result].typ = TYP_NULL;
		return;
	}
	m->m_mem[result].oper.i = m->m_mem[firstarg].oper.ol->l_cnt;
	m->m_mem[result].typ = TYP_NUM;
}




/*
drop elements from an existing list. first arg is (must be) list, remaining
are elements to drop.
*/
void
blt_listdrop(m,argc,firstarg,result,uid,euid)
Machine	*m;
int	argc;
int	firstarg;
int	result;
long	*uid;
long	*euid;
{
	ObjList	*ret;
	long	*refp;
	long	*olp;
	int	oc;
	int	x;

	if(m->m_mem[firstarg].typ != TYP_OLIST) {
		m->m_mem[result].oper.i = ERR_BADARG;
		m->m_mem[result].typ = TYP_NULL;
		return;
	}

	/*
	semi-bummer. cannot shrink the list because the elements may
	not actually be on it. keep it the same size
	*/
	oc = m->m_mem[firstarg].oper.ol->l_cnt;
	if((ret = listnew(oc)) == (ObjList *)0) {
		m->m_mem[result].typ = TYP_NULL;
		m->m_mem[result].oper.i = ERR_OOM;
		return;
	}

	ret->l_cnt = 0;
	refp = ret->l_data;

	olp = m->m_mem[firstarg].oper.ol->l_data;
	for(x = 0; x < oc; x++) {
		int	y;
		int	skip = 0;

		for(y = 1; y < argc; y++) {
			if(m->m_mem[firstarg + y].typ != TYP_OBJ ||
				m->m_mem[firstarg + y].oper.l == *olp) {
				skip++;
			}
		}

		if(!skip) {
			*refp++ = *olp;
			ret->l_cnt++;
		}

		olp++;
	}

	if(ret->l_cnt == 0) {
		m->m_mem[result].oper.i = ERR_NONE;
		m->m_mem[result].typ = TYP_NULL;
	} else {
		m->m_mem[result].oper.ol = ret;
		m->m_mem[result].typ = TYP_OLIST;
	}
}




/*
return the N-th element of a list.
*/
void
blt_listelem(m,argc,firstarg,result,uid,euid)
Machine	*m;
int	argc;
int	firstarg;
int	result;
long	*uid;
long	*euid;
{
	int	x;

	if(argc != 2 || m->m_mem[firstarg].typ != TYP_OLIST || m->m_mem[firstarg + 1].typ != TYP_NUM) {
		m->m_mem[result].oper.i = ERR_BADARG;
		m->m_mem[result].typ = TYP_NULL;
		return;
	}
	x = m->m_mem[firstarg + 1].oper.i;
	if(x > m->m_mem[firstarg].oper.ol->l_cnt) {
		m->m_mem[result].oper.i = ERR_BADARG;
		m->m_mem[result].typ = TYP_NULL;
		return;
	}
	m->m_mem[result].oper.l = m->m_mem[firstarg].oper.ol->l_data[x - 1];
	m->m_mem[result].typ = TYP_OBJ;
}




/*
merge 2 lists into one. pack out dupes.
*/
void
blt_listmerge(m,argc,firstarg,result,uid,euid)
Machine	*m;
int	argc;
int	firstarg;
int	result;
long	*uid;
long	*euid;
{
	ObjList	*ret;
	ObjList	*lp1 = (ObjList *)0;
	ObjList	*lp2 = (ObjList *)0;
	long	*refp;
	long	*olp;
	int	l1c = 0;
	int	l2c = 0;

	if((m->m_mem[firstarg].typ != TYP_OLIST &&
	m->m_mem[firstarg].typ != TYP_NULL) ||
	(m->m_mem[firstarg + 1].typ != TYP_OLIST &&
	m->m_mem[firstarg + 1].typ != TYP_NULL)) {

		m->m_mem[result].oper.i = ERR_BADARG;
		m->m_mem[result].typ = TYP_NULL;
		return;
	}

	/* set pointers */
	if(m->m_mem[firstarg].typ != TYP_NULL) {
		lp1 = m->m_mem[firstarg].oper.ol;
		l1c = lp1->l_cnt;
		olp = lp1->l_data;
	}

	if(m->m_mem[firstarg + 1].typ != TYP_NULL) {
		lp2 = m->m_mem[firstarg + 1].oper.ol;
		l2c = lp2->l_cnt;
	}

	/* some memory wastage, but 'tis all temporary */
	if((ret = listnew(l1c + l2c)) == (ObjList *)0) {
		m->m_mem[result].typ = TYP_NULL;
		m->m_mem[result].oper.i = ERR_OOM;
		return;
	}

	ret->l_cnt = 0;
	refp = ret->l_data;

	/* copy all unique elements from list #1 */
	while(l1c--) {
		int	y;
		int	skip = 0;

		for(y = 0; y < l2c; y++) {
			if(lp2->l_data[y] == *olp)
				skip++;
		}

		if(!skip) {
			*refp++ = *olp;
			ret->l_cnt++;
		}

		olp++;
	}

	if(m->m_mem[firstarg + 1].typ != TYP_NULL)
		olp = lp2->l_data;

	/* by def. all elements in list #2 are unique, now. */
	while(l2c--) {
		*refp++ = *olp++;
		ret->l_cnt++;
	}

	m->m_mem[result].oper.ol = ret;
	m->m_mem[result].typ = TYP_OLIST;
}




/*
create a list. if arguments are provided, initialize it with them.
*/
void
blt_listnew(m,argc,firstarg,result,uid,euid)
Machine	*m;
int	argc;
int	firstarg;
int	result;
long	*uid;
long	*euid;
{
	ObjList	*ret;
	long	*refp;
	int	x;

	if((ret = listnew(argc)) == (ObjList *)0) {
		m->m_mem[result].typ = TYP_NULL;
		m->m_mem[result].oper.i = ERR_OOM;
		return;
	}

	ret->l_cnt = 0;
	refp = ret->l_data;

	for(x = 0; x < argc; x++) {
		if(m->m_mem[firstarg + x].typ == TYP_OBJ) {
			*refp++ = m->m_mem[firstarg + x].oper.l;
			ret->l_cnt++;
		}
	}
	m->m_mem[result].oper.ol = ret;
	m->m_mem[result].typ = TYP_OLIST;
}



/*
add elements to an existing list. first arg is (must be) list, remaining are
elements to add.
*/
void
blt_listprepend(m,argc,firstarg,result,uid,euid)
Machine	*m;
int	argc;
int	firstarg;
int	result;
long	*uid;
long	*euid;
{
	ObjList	*ret;
	long	*refp;
	long	*olp;
	int	oc;
	int	x;

	if(m->m_mem[firstarg].typ != TYP_OLIST && m->m_mem[firstarg].typ != TYP_NULL) {
		m->m_mem[result].oper.i = ERR_BADARG;
		m->m_mem[result].typ = TYP_NULL;
		return;
	}

	if(m->m_mem[firstarg].typ != TYP_NULL) {
		olp = m->m_mem[firstarg].oper.ol->l_data;
		oc = m->m_mem[firstarg].oper.ol->l_cnt;
	} else {
		olp = 0;
		oc = 0;
	}

	if((ret = listnew(oc + argc - 1)) == (ObjList *)0) {
		m->m_mem[result].typ = TYP_NULL;
		m->m_mem[result].oper.i = ERR_OOM;
		return;
	}

	ret->l_cnt = 0;
	refp = ret->l_data;

	/* skip first arg, remember */
	for(x = 1; x < argc; x++) {
		int	y;
		int	skip = 0;
		long	*p = olp;

		for(y = 0; y < oc; y++) {
			if(m->m_mem[firstarg + x].typ != TYP_OBJ ||
				m->m_mem[firstarg + x].oper.l == *p) {
				skip++;
			}
			p++;
		}

		if(!skip) {
			*refp++ = m->m_mem[firstarg + x].oper.l;
			ret->l_cnt++;
		}
	}

	for(x = 0; x < oc; x++) {
		*refp++ = *olp++;
		ret->l_cnt++;
	}

	m->m_mem[result].oper.ol = ret;
	m->m_mem[result].typ = TYP_OLIST;
}




/*
search the list, returning the # of the slot where the element is, or 0
if it is not in the list
*/
void
blt_listsearch(m,argc,firstarg,result,uid,euid)
Machine	*m;
int	argc;
int	firstarg;
int	result;
long	*uid;
long	*euid;
{
	long	*olp;
	int	x;

	if(argc != 2 || m->m_mem[firstarg].typ != TYP_OLIST ||
	m->m_mem[firstarg + 1].typ != TYP_OBJ) {
		m->m_mem[result].oper.i = ERR_BADARG;
		m->m_mem[result].typ = TYP_NULL;
		return;
	}

	
	olp = m->m_mem[firstarg].oper.ol->l_data;
	for(x = 0; x < m->m_mem[firstarg].oper.ol->l_cnt; x++) {
		if(*olp == m->m_mem[firstarg + 1].oper.l) {
			m->m_mem[result].oper.i = x + 1;
			m->m_mem[result].typ = TYP_NUM;
			return;
		}
		olp++;
	}

	m->m_mem[result].oper.i = ERR_NOTHERE;
	m->m_mem[result].typ = TYP_NULL;
}




/*
set the N-th element of a list to a value.
*/
void
blt_listsetelem(m,argc,firstarg,result,uid,euid)
Machine	*m;
int	argc;
int	firstarg;
int	result;
long	*uid;
long	*euid;
{
	ObjList	*ret;
	long	*olp;
	long	*nlp;
	int	oc;
	int	sv;
	int	x;
	

	if(argc != 3 || m->m_mem[firstarg].typ != TYP_OLIST ||
	m->m_mem[firstarg + 1].typ != TYP_NUM ||
	m->m_mem[firstarg + 2].typ != TYP_OBJ) {
		m->m_mem[result].oper.i = ERR_BADARG;
		m->m_mem[result].typ = TYP_NULL;
		return;
	}

	sv = m->m_mem[firstarg + 1].oper.i - 1;

	/* do not permit overruns or underruns */
	if(sv < 0 || sv >= m->m_mem[firstarg].oper.ol->l_cnt) {
		m->m_mem[result].oper.i = ERR_NOTHERE;
		m->m_mem[result].typ = TYP_NULL;
		return;
	}

	oc = m->m_mem[firstarg].oper.ol->l_cnt;

	if((ret = listnew(oc)) == (ObjList *)0) {
		m->m_mem[result].oper.i = ERR_OOM;
		m->m_mem[result].typ = TYP_NULL;
		return;
	}

	olp = m->m_mem[firstarg].oper.ol->l_data;
	nlp = ret->l_data;
	for(x = 0; x < oc; x++) {
		if(x != sv) {
			*nlp++ = *olp++;
		} else {
			*nlp++ = m->m_mem[firstarg + 2].oper.l;
			olp++;
		}
		ret->l_cnt++;
	}

	m->m_mem[result].oper.ol = ret;
	m->m_mem[result].typ = TYP_OLIST;
}

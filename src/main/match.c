#include	<ctype.h>

#include	"ubermud.h"
#include	"externs.h"

/*
#define	MATCHDEBUG
*/


/*
Copyright(C) 1990, Marcus J. Ranum, All Rights Reserved.
This software may be freely used, modified, and redistributed,
as long as this copyright message is left intact, and this
software is not used to develop any commercial product, or used
in any product that is provided on a pay-for-use basis.
*/

struct	mtch	{
	Oper	op;
	int	typ;
	char	*fp;
};

static	struct	mtch	mlist[MAXMATCHLIST];
static	int		tomatch;



/*
return the # of bytes of matching prefix, or -1 to indicate an
UN-match - this assumes s2 is the user-provided string.
*/
static	int
prefixlen(s1,s2,flg)
register char	*s1;
register char	*s2;
int	*flg;
{
	register int	rv = 0;

#ifdef	MATCHDEBUG
	printf("prefixlen compare \"%s\" and \"%s\"\n",s1,s2);
#endif

	/* match while match can */
	while(*s1 != '\0' && *s2 != '\0') {
		if((isupper(*s1) ? tolower((int)*s1) : *s1) !=
			(isupper(*s2) ? tolower((int)*s2) : *s2))
			return(-1);
		rv++, s1++, s2++;
	}

	/*
	if there is more to the user-provided string, and not in
	the string we are matching against, abort the match.
	*/
	if(*s1 == '\0' && *s2 != '\0')
		return(-1);

	/* flag complete matches */
	if(*s1 == '\0' && *s2 == '\0')
		*flg = *flg + 1;

#ifdef	MATCHDEBUG
	printf("prefixlen returns %d (flg=%d)\n",rv,*flg);
#endif
	return(rv);
}




/*
vectorise and match.

REMEMBER - you CANNOT modify the data in memory here, as it is a
pointer directly into the copy in cache, and you will effectively
alter the cache or stack storage if you touch this!!  fun - but
don't do it!!
*/
static	int
matchvector(s1,s2,eflg)
char	*s1;
char	*s2;
int	*eflg;
{
	int	rv = 0;

	/* buffers to tokenize into */
	char	mbuf1[MUDBUFSIZ];
	char	mbuf2[MUDBUFSIZ];
	char	*mvec1[MUDMAXARG];
	char	*mvec2[MUDMAXARG];
	int	mac1;
	int	mac2;
	char	**vp1;
	char	**vp2;
	int	exak = 0;

	/* chopper-pointers */
	char	*p1 = s1;
	char	*p2 = s1;

	/* tokenize the first input string */
	if((mac2 = enargv(s2,mvec2,MUDMAXARG,mbuf2,MUDBUFSIZ)) == 0)
		return(0);

	/* s1 is allowed to have aliases... */
	while(*p1 != '\0') {
		register int	mv;
		register int	tmv;

		/* reset the exact match counter */
		*eflg = 0;

		while(*p2 != '\0' && *p2 != MATCH_CHAR)
			p2++;

		/* EVIL! */
		if(*p2 == MATCH_CHAR) {
			/*
			kludge - temporarily NULL-terminate the buffer
			before passing it to enargv, then repair it.
			remember - this is what I just warned you NOT
			to do ? we're mangling bits in the stack...
			*/
			*p2 = '\0';
			mac1 = enargv(s1,mvec1,MUDMAXARG,mbuf1,MUDBUFSIZ);
			*p2 = MATCH_CHAR;
			p2++;
		} else {
			mac1 = enargv(s1,mvec1,MUDMAXARG,mbuf1,MUDBUFSIZ);
		}

		/* if there are no tokens, or the user has given more */
		if(mac1 == 0 || mac2 > mac1) {
#ifdef	MATCHDEBUG
			printf("vectormatch: token count mismatch, skip.\n");
#endif
			goto failed;
		}

		/* loop/match across the match vectors */
		for(vp1 = mvec1; *vp1 != (char *)0; vp1++) {
			char	**tmpvp = vp1;
			for(mv = 0, vp2 = mvec2; *vp2 != (char *)0 && *tmpvp != (char *)0; vp2++, tmpvp++) {
				if((tmv = prefixlen(*tmpvp,*vp2,eflg)) == -1) {
#ifdef	MATCHDEBUG
					printf("bad prefix, scrub.\n");
#endif
					mv = -1;
					*eflg = 0;
					break;
				}
				mv += tmv;
			}
			if(*tmpvp == (char *)0 && *vp2 != (char *)0) {
#ifdef	MATCHDEBUG
				printf("token count mismatch, scrub.\n");
#endif
				mv = -1;
				exak = 0;
				*eflg = 0;
			}
			if(mv > rv) {
#ifdef	MATCHDEBUG
				printf("take %d as new best byte count.\n",mv);
#endif
				exak = *eflg;
				rv = mv;
			}
		}

/* failed just drops through */
failed:
		/* ready to match next hunk */
		p1 = p2;
	}

	*eflg = exak;

#ifdef	MATCHDEBUG
	printf("match value is %d, (flg=%d)\n",rv,*eflg);
#endif
	return(rv);
}



/*
reset the stored match pointers
*/
void
matchreset()
{
	tomatch = 0;
}



/*
set a pointer into the match list for the next run. these pointers
cheerfully dangle when the objectlists are freed at the end of a 
run. make damn sure you do a matchreset between calls to match.
*/
matchset(op,typ,fld)
Oper	op;
int	typ;
char	*fld;
{
	if(tomatch >= MAXMATCHLIST)
		return(1);
	mlist[tomatch].op = op;
	mlist[tomatch].typ = typ;
	mlist[tomatch].fp = fld;
	tomatch++;
	return(0);
}



static	int
matchcompare(thing,nam,uid,euid,eflg)
char	*thing;
char	*nam;
long	uid;
long	euid;
int	*eflg;
{
	Oper	d1;
	int	d1t;
	int	tval;


	/* get the thing from disk */
	if(ELENUM(thing) == (long)0)
		tval = sys_thaw(thing,&d1t,&d1,uid,euid);
	else
		tval = disk_thaw(thing,&d1t,&d1,uid,euid);

	/* if it didn't thaw, or is ! a string skip it */
	if(tval != 0 || d1t != TYP_STR)
		return(0);

	return(matchvector(d1.c,nam,eflg));
}



/*
perform a match against the object lists by field in the match list
*/
long
match(nam,uid,euid)
char	*nam;
long	uid;
long	euid;
{
	static	char	*ibuf = (char *)0;
	int	x;
	int	best = 0;
	long	ret = (long)0;
	int	tval;

	/* flags to control exact matching - kinda kludgy */
	int	exact;
	int	exactcnt = 0;

	if(ibuf == (char *)0) {
		ibuf = malloc(MAXIDENTLEN * 2);
		if(ibuf == (char *)0)
			fatal("alloc ibuf in match failed: ",(char *)-1,"\n",0);
	}

	if(*nam == '\0')
		return((long)0);

	for(x = 0; x < tomatch; x++) {
		/* set field name */
		if(mlist[x].fp == (char *)0)
			continue;

		(void)strcpy(ELENAM(ibuf),mlist[x].fp);

		if(mlist[x].typ == TYP_OLIST) {
			int	lcnt = mlist[x].op.ol->l_cnt;
			long	*bobp = mlist[x].op.ol->l_data;

			while(lcnt--) {
				exact = 0;
				ELENUM(ibuf) = *bobp++;

				tval = matchcompare(ibuf,nam,uid,euid,&exact);

				if(tval == best && exact && exact > exactcnt) {
					best = tval;
					exactcnt = exact;
					ret = ELENUM(ibuf);
				}

				if(tval > best) {
					best = tval;
					ret = ELENUM(ibuf);
				}
			}
		} else
		if(mlist[x].typ == TYP_OBJ) {
			exact = 0;
			ELENUM(ibuf) = mlist[x].op.l;

			tval = matchcompare(ibuf,nam,uid,euid,&exact);
			if(tval == best && exact && exact > exactcnt) {
				best = tval;
				exactcnt = exact;
				ret = ELENUM(ibuf);
			}
			if(tval > best) {
				best = tval;
				ret = ELENUM(ibuf);
			}
		}
	}
	return(ret);
}



void
blt_match(m,argc,firstarg,result,uid,euid)
Machine	*m;
int	argc;
int	firstarg;
int	result;
long	*uid;
long	*euid;
{
	char	*np;
	int	x;
	long	matv;

	if(argc < 3 || m->m_mem[firstarg].typ != TYP_STR) {
		m->m_mem[result].oper.i = ERR_BADARG;
		m->m_mem[result].typ = TYP_NULL;
		return;
	}

	np = m->m_mem[firstarg].oper.c;

	matchreset();

	for(x = 1; x < argc; x += 2) {
		if(m->m_mem[firstarg + x + 1].typ != TYP_STR)
			continue;

		if(matchset(m->m_mem[firstarg + x].oper,
			m->m_mem[firstarg + x].typ,
			m->m_mem[firstarg + x + 1].oper.c)) {

			m->m_mem[result].oper.i = ERR_BADARG;
			m->m_mem[result].typ = TYP_NULL;
			return;
		}
	}

	matv = match(np,*uid,*euid);

#ifdef	MATCHDEBUG
	printf("blt_match:match returns %d\n",matv);
#endif

	if(matv == (long)0) {
		m->m_mem[result].oper.i = ERR_NOTHERE;
		m->m_mem[result].typ = TYP_NULL;
		return;
	}
	m->m_mem[result].oper.l = matv;
	m->m_mem[result].typ = TYP_OBJ;
}


/*
#ifdef	MATCHDEBUG
		printf("matchvector: matching across %s -> %s\n",p1,sp);
#endif

		while(*p1 != '\0' && *p1 != MATCH_CHAR && *sp != '\0') {
			c1 = isupper(*p1) ? tolower((int)*p1) : *p1;
			c2 = isupper(*sp) ? tolower((int)*sp) : *sp;

			if(c1 == c2) {
				mv++;
#ifdef	MATCHDEBUG
				printf("matchvector: match %d\n",mv);
#endif
			} else {
#ifdef	MATCHDEBUG
				printf("matchvector: stop matching\n");
#endif
				mv = 0;
				break;
			}
			sp++;
			p1++;
		}


		p1 = p2;

		if((*p1 == '\0' || *p1 == MATCH_CHAR) && *sp != '\0')
			continue;

		if(mv > rv) {
#ifdef	MATCHDEBUG
			printf("matchvector: new best match %d old, %d\n",mv,rv);
#endif
			rv = mv;
*/

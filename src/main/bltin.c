#include	<stdio.h>
#include	<sys/file.h>

/* henry spencer's regexp, not the system's. */
#include	"regexp.h"

#include	"ubermud.h"
#include	"externs.h"

extern char *tcpio_getip(int);
extern int tcpio_getidle(int);

/*
Copyright(C) 1990, Marcus J. Ranum, All Rights Reserved.
This software may be freely used, modified, and redistributed,
as long as this copyright message is left intact, and this
software is not used to develop any commercial product, or used
in any product that is provided on a pay-for-use basis.
*/

/*
built-in primitives - simple stuff like str(), and so on. more
built-ins reside in list.c - these are all called through the
dispatch table in blttab.c

note - in all these functions, you CANNOT assign a stack value
to the result argument, until AFTER you are done processing
all the stuff in the stack - because result is ALSO firstarg
in some cases, and assigning result will overwrite the argument
stack
*/

static FILE *old_logfile=0;

static	char	*liststr = "<list>";
static	char	*funcstr = "<function>";

/* error message table. in ubermud, an error message is a data type */
static	char	*errtbl[] = {
/* ERR_NONE	0 */	"no error",
/* ERR_USER	1 */	"error",
/* ERR_OOM	2 */	"out of memory",
/* ERR_NUM	3 */	"numeric operation on non-number",
/* ERR_ZDIV	4 */	"division by zero",
/* ERR_BADOBJ	5 */	"badly formed element specifier",
/* ERR_BADARG	6 */	"bad parameter type",
/* ERR_NOTHERE	7 */	"nonexistent object",
/* ERR_REF	8 */	"cannot reference object",
/* ERR_NOVAL	9 */	"function returned no value",
/* ERR_NOPAR	10 */	"no such parameter",
/* ERR_DBASE	11 */	"I/O error (this is bad!)",
/* ERR_PERM	12 */	"permission denied",
/* ERR_NOTOWN	13 */	"not owner",
/* ERR_STACK	14 */	"stack over/underflow",
0
};


char	*errmsg(x)
int	x;
{
	return(errtbl[x]);
}

void
blt_openfile(Machine *m, int argc, int firstarg, int result, long *uid, long *euid)
{
  extern FILE *main_logfil;

  char buf[MUDBUFSIZ], *p;

  if ((argc !=1) || old_logfile) {
    m->m_mem[result].typ = TYP_NULL;
    m->m_mem[result].oper.i = ERR_BADARG;
    return;
  }

  for (p=m->m_mem[firstarg].oper.c; *p != '\0'; p++)
    if (*p == '.' && *(p+1) == '.') {
      m->m_mem[result].oper.i = ERR_BADARG;
      m->m_mem[result].typ = TYP_NULL;
      return;
    }

  if (*(m->m_mem[firstarg].oper.c) == '/') {
    m->m_mem[result].oper.i = ERR_BADARG;
    m->m_mem[result].typ = TYP_NULL;
    return;
  }

  sprintf(buf, "write_files/%s", m->m_mem[firstarg].oper.c);

  old_logfile=main_logfil;

  main_logfil=fopen(buf, "w");
  if (!main_logfil) {
    main_logfil=old_logfile;
    m->m_mem[result].oper.i = ERR_BADARG;
    m->m_mem[result].typ = TYP_NULL;
    return;
  }

}

void
blt_closefile(Machine *m, int argc, int firstarg, int result, long *uid, long *euid)
{
  extern FILE *main_logfil;

  if (old_logfile) {
    fflush(main_logfil);
    fclose(main_logfil);
    main_logfil=old_logfile;
    old_logfile=NULL;

    m->m_mem[result].oper.i = 1;
    m->m_mem[result].typ = TYP_NUM;
    return;
  }
  m->m_mem[result].oper.i = ERR_BADARG;
  m->m_mem[result].typ = TYP_NULL;
}

/*
atoi. overload NUM and OBJ already.
*/
void
blt_atoi(m,argc,firstarg,result,uid,euid)
Machine	*m;
int	argc;
int	firstarg;
int	result;
long	*uid;
long	*euid;
{
	if(argc != 1) {
		m->m_mem[result].typ = TYP_NUM;
		m->m_mem[result].oper.i = 0;
		return;
	}

	switch(m->m_mem[firstarg].typ) {
		case	TYP_STR:
		m->m_mem[result].oper.i = atoi(m->m_mem[firstarg].oper.c);
		break;

		case	TYP_OBJ:
		m->m_mem[result].oper.i = (int)m->m_mem[firstarg].oper.l;
		break;

		case	TYP_NUM:
		m->m_mem[result].oper.i = m->m_mem[firstarg].oper.i;
		break;

		default:
		m->m_mem[result].oper.i = 0;
		break;
	}
	m->m_mem[result].typ = TYP_NUM;
}



/*
atoobj. overload NUM and OBJ already.
*/
void
blt_atoobj(m,argc,firstarg,result,uid,euid)
Machine	*m;
int	argc;
int	firstarg;
int	result;
long	*uid;
long	*euid;
{
	if(argc != 1) {
		m->m_mem[result].typ = TYP_NULL;
		m->m_mem[result].oper.i = ERR_BADARG;
		return;
	}

	switch(m->m_mem[firstarg].typ) {
		case	TYP_STR:
		if(*m->m_mem[firstarg].oper.c == '#') {
			m->m_mem[result].oper.l = atol(&m->m_mem[firstarg].oper.c[1]);
		} else {
			m->m_mem[result].oper.l = atol(m->m_mem[firstarg].oper.c);
		}
		m->m_mem[result].typ = TYP_OBJ;
		break;

		case	TYP_OBJ:
		m->m_mem[result].oper.l = m->m_mem[firstarg].oper.l;
		m->m_mem[result].typ = TYP_OBJ;
		break;

		case	TYP_NUM:
		m->m_mem[result].oper.l = (long)m->m_mem[firstarg].oper.i;
		m->m_mem[result].typ = TYP_OBJ;
		break;

		default:
		m->m_mem[result].oper.i = ERR_BADOBJ;
		m->m_mem[result].typ = TYP_NULL;
		break;
	}
	if(m->m_mem[result].typ != TYP_NULL && m->m_mem[result].oper.l == (long)0) {
		m->m_mem[result].oper.i = ERR_BADOBJ;
		m->m_mem[result].typ = TYP_NULL;
	}
}




/*
dump the contents of the file named to the object spec'd as first argument
*/
void
blt_catfile(m,argc,firstarg,result,uid,euid)
Machine	*m;
int	argc;
int	firstarg;
int	result;
long	*uid;
long	*euid;
{
	long	who;
	int	fd;
	int	red;
	char	buf[MUDBUFSIZ];
	char	*p;

	if(argc != 2 || m->m_mem[firstarg].typ != TYP_OBJ ||
		m->m_mem[firstarg + 1].typ != TYP_STR) {

		m->m_mem[result].oper.i = ERR_BADARG;
		m->m_mem[result].typ = TYP_NULL;
		return;
	}

	/* no files with '..' in them */
	for(p = m->m_mem[firstarg + 1].oper.c; *p != '\0'; p++)
		if(*p == '.' && *(p + 1) == '.') {
			m->m_mem[result].oper.i = ERR_BADARG;
			m->m_mem[result].typ = TYP_NULL;
			return;
		}

	/* no files starting with '/' */
	if(*m->m_mem[firstarg + 1].oper.c == '/') {
		m->m_mem[result].oper.i = ERR_BADARG;
		m->m_mem[result].typ = TYP_NULL;
		return;
	}

	/*
	heh. put them in a "files" directory - otherwise they
	can steal the password file.
	*/
	(void)sprintf(buf,"files/%s",m->m_mem[firstarg + 1].oper.c);
	who = m->m_mem[firstarg].oper.l;

	if((fd = open(buf,O_RDONLY)) < 0) {
/*		iobtell(who,"cannot open ",buf,": ",sys_errlist[errno],"\n",0);*/
		m->m_mem[result].oper.i = ERR_BADARG;
		m->m_mem[result].typ = TYP_NULL;
		return;
	      }

	while((red = read(fd,buf,MUDBUFSIZ - 1)) > 0) {
		buf[red] = '\0';
		iobtell(who,buf,0);
	}

	(void)close(fd);
	m->m_mem[result].oper.i = 0;
	m->m_mem[result].typ = TYP_NUM;
}




/*
disconnect connections owned by XX
*/
void
blt_disconnect(m,argc,firstarg,result,uid,euid)
Machine	*m;
int	argc;
int	firstarg;
int	result;
long	*uid;
long	*euid;
{
	if(*uid != (long)0 && *euid != (long)0) {
		m->m_mem[result].oper.i = ERR_PERM;
		m->m_mem[result].typ = TYP_NULL;
		return;
	}

	while(argc--) {
		if(m->m_mem[firstarg].typ == TYP_OBJ)
			iobdisconnect(m->m_mem[firstarg].oper.l);
		firstarg++;
	}
	m->m_mem[result].oper.i = 0;
	m->m_mem[result].typ = TYP_NUM;
}



/*
echo arguments. some operator overloading is done to handle
ints/strings intelligently.
*/
void
blt_echo(m,argc,firstarg,result,uid,euid)
Machine	*m;
int	argc;
int	firstarg;
int	result;
long	*uid;
long	*euid;
{
	while(argc--) {
		switch(m->m_mem[firstarg].typ) {
			case	TYP_STR:
			iobtell(*uid,m->m_mem[firstarg].oper.c,0);
			break;

			case	TYP_OBJ:
			iobtell(*uid,"#",ltoa(m->m_mem[firstarg].oper.l),0);
			break;

			case	TYP_FUNC:
			iobtell(*uid,funcstr,0);
			break;

			case	TYP_OLIST:
			iobtell(*uid,liststr,0);
			break;

			case	TYP_NUM:
			iobtell(*uid,itoa(m->m_mem[firstarg].oper.i),0);
			break;
		}
		firstarg++;
	}

	/* echo does not return a value. put a NULL on the stack */
	m->m_mem[result].typ = TYP_NULL;
	m->m_mem[result].oper.i = ERR_NONE;
}



/*
echo arguments TO someone. some operator overloading is done to handle
ints/strings intelligently.
*/
void
blt_echoto(m,argc,firstarg,result,uid,euid)
Machine	*m;
int	argc;
int	firstarg;
int	result;
long	*uid;
long	*euid;
{
	long	who;

	if(m->m_mem[firstarg].typ != TYP_OBJ) {
		m->m_mem[result].typ = TYP_NULL;
		m->m_mem[result].oper.i = ERR_BADARG;
		return;
	}

	who = m->m_mem[firstarg].oper.l;

	firstarg++;
	argc--;

	while(argc--) {
		switch(m->m_mem[firstarg].typ) {
			case	TYP_STR:
			iobtell(who,m->m_mem[firstarg].oper.c,0);
			break;

			case	TYP_OBJ:
			iobtell(who,"#",ltoa(m->m_mem[firstarg].oper.l),0);
			break;

			case	TYP_FUNC:
			iobtell(who,funcstr,0);
			break;

			case	TYP_OLIST:
			iobtell(who,liststr,0);
			break;

			case	TYP_NUM:
			iobtell(who,itoa(m->m_mem[firstarg].oper.i),0);
			break;
		}
		firstarg++;
	}

	/* echo does not return a value. put a NULL on the stack */
	m->m_mem[result].typ = TYP_NULL;
	m->m_mem[result].oper.i = ERR_NONE;
}



/*
return numerical error
*/
void
blt_errno(m,argc,firstarg,result,uid,euid)
Machine	*m;
int	argc;
int	firstarg;
int	result;
long	*uid;
long	*euid;
{
	if(argc < 1 || m->m_mem[firstarg].typ != TYP_NULL)
		m->m_mem[result].oper.i = ERR_NONE;
	else
	m->m_mem[result].oper.i = m->m_mem[firstarg].oper.i;
	m->m_mem[result].typ = TYP_NUM;
}



/*
return string error
*/
void
blt_error(m,argc,firstarg,result,uid,euid)
Machine	*m;
int	argc;
int	firstarg;
int	result;
long	*uid;
long	*euid;
{
	if(argc < 1 || m->m_mem[firstarg].typ != TYP_NULL)
		m->m_mem[result].oper.c = "";
	else
	m->m_mem[result].oper.c = errtbl[m->m_mem[firstarg].oper.i];
	m->m_mem[result].typ = TYP_STR;
}




/*
return the person's effective user-id.
*/
void
blt_geteuid(m,argc,firstarg,result,uid,euid)
Machine	*m;
int	argc;
int	firstarg;
int	result;
long	*uid;
long	*euid;
{
	m->m_mem[result].oper.i = *euid;
	m->m_mem[result].typ = TYP_OBJ;
}

/*
return the ip address
*/
void blt_getip(Machine *m, int argc, int firstarg, int result, long *uid, long *euid)
{
  long who;
  char *p, *op;

  if (m->m_mem[firstarg].typ != TYP_OBJ) {
    m->m_mem[result].typ = TYP_NULL;
    m->m_mem[result].oper.i = ERR_BADARG;
    return;
  }
  
  who=m->m_mem[firstarg].oper.l;

  if (!(p=tcpio_getip(who))) {
    m->m_mem[result].typ = TYP_NULL;
    m->m_mem[result].oper.i = ERR_BADARG;
    return;
  }

  op=m->m_mem[result].oper.c = tmpalloc(20);
  if (m->m_mem[result].oper.c == 0) {
    m->m_mem[result].typ = TYP_NULL;
    m->m_mem[result].oper.i = ERR_OOM;
    return;
  }

  strcpy(op, p);
  m->m_mem[result].typ = TYP_STR;
}

/*
return the person's real user-id.
*/
void
blt_getuid(m,argc,firstarg,result,uid,euid)
Machine	*m;
int	argc;
int	firstarg;
int	result;
long	*uid;
long	*euid;
{
	m->m_mem[result].oper.i = *uid;
	m->m_mem[result].typ = TYP_OBJ;
}




/*
return true if the argument is a list.
*/
void
blt_islist(m,argc,firstarg,result,uid,euid)
Machine	*m;
int	argc;
int	firstarg;
int	result;
long	*uid;
long	*euid;
{
	if(argc < 1 || m->m_mem[firstarg].typ != TYP_OLIST)
		m->m_mem[result].oper.i = 0;
	else
		m->m_mem[result].oper.i = 1;
	m->m_mem[result].typ = TYP_NUM;
}



/*
return true if the argument is a number.
*/
void
blt_isnum(m,argc,firstarg,result,uid,euid)
Machine	*m;
int	argc;
int	firstarg;
int	result;
long	*uid;
long	*euid;
{
	if(argc < 1 || m->m_mem[firstarg].typ != TYP_NUM)
		m->m_mem[result].oper.i = 0;
	else
		m->m_mem[result].oper.i = 1;
	m->m_mem[result].typ = TYP_NUM;
}



/*
return true if the argument is an object #.
*/
void
blt_isobj(m,argc,firstarg,result,uid,euid)
Machine	*m;
int	argc;
int	firstarg;
int	result;
long	*uid;
long	*euid;
{
	if(argc < 1 || m->m_mem[firstarg].typ != TYP_OBJ)
		m->m_mem[result].oper.i = 0;
	else
		m->m_mem[result].oper.i = 1;
	m->m_mem[result].typ = TYP_NUM;
}



/*
return true if the argument is a string.
*/
void
blt_isstr(m,argc,firstarg,result,uid,euid)
Machine	*m;
int	argc;
int	firstarg;
int	result;
long	*uid;
long	*euid;
{
	if(argc < 1 || m->m_mem[firstarg].typ != TYP_STR)
		m->m_mem[result].oper.i = 0;
	else
		m->m_mem[result].oper.i = 1;
	m->m_mem[result].typ = TYP_NUM;
}



/*
echo strings into system log
*/
void
blt_log(m,argc,firstarg,result,uid,euid)
Machine	*m;
int	argc;
int	firstarg;
int	result;
long	*uid;
long	*euid;
{
	while(argc--) {
		switch(m->m_mem[firstarg].typ) {
			case	TYP_STR:
			logf(m->m_mem[firstarg].oper.c,0);
			break;

			case	TYP_OBJ:
			logf("#",ltoa(m->m_mem[firstarg].oper.l),0);
			break;

			case	TYP_FUNC:
			logf(funcstr,0);
			break;

			case	TYP_OLIST:
			logf(liststr,0);
			break;

			case	TYP_NUM:
			logf(itoa(m->m_mem[firstarg].oper.i),0);
			break;
		}
		firstarg++;
	}

	m->m_mem[result].typ = TYP_NULL;
	m->m_mem[result].oper.i = ERR_NONE;
}




/*
return a random number between 0 and the arg given.
*/
void
blt_rand(m,argc,firstarg,result,uid,euid)
Machine	*m;
int	argc;
int	firstarg;
int	result;
long	*uid;
long	*euid;
{
	static	int	initd = 0;
	int	mod = 0;
	int	gotmod = 0;

	if(!initd) {
		(void)srandom(getpid() * getppid());
		initd++;
	}

	if(argc > 0) {
		if(m->m_mem[firstarg].typ == TYP_NUM) {
			mod = m->m_mem[firstarg].oper.i;
			gotmod++;
		} else {
			m->m_mem[result].oper.i = 0;
			return;
		}
	}

	m->m_mem[result].oper.i = (int)random();
	if(gotmod)
		m->m_mem[result].oper.i %= mod;
	m->m_mem[result].typ = TYP_NUM;
}




/*
perform a regular expression comparison.
*/
void
blt_regcmp(m,argc,firstarg,result,uid,euid)
Machine	*m;
int	argc;
int	firstarg;
int	result;
long	*uid;
long	*euid;
{
	regexp	*re;

	if(argc != 2 || m->m_mem[firstarg].typ != TYP_STR ||
		m->m_mem[firstarg + 1].typ != TYP_STR) {
		m->m_mem[result].oper.i = 0;
		m->m_mem[result].typ = TYP_NUM;
		return;
	}
	re = regcomp(m->m_mem[firstarg + 1].oper.c);
	if(re == (regexp *)0) {
		m->m_mem[result].oper.i = 0;
		m->m_mem[result].typ = TYP_NUM;
		return;
	}
	m->m_mem[result].oper.i = regexec(re,m->m_mem[firstarg].oper.c);
	m->m_mem[result].typ = TYP_NUM;
        free(re);                               /* no memory leak */
}




/*
return a section of a string matching the given expression.
expressions can be egrep-like. if there is no match, return
NULL.
*/
void
blt_regexp(m,argc,firstarg,result,uid,euid)
Machine	*m;
int	argc;
int	firstarg;
int	result;
long	*uid;
long	*euid;
{
	regexp	*re;
	int	rx;
	char	*nsp;
	char	*xp;
	char	*yp;
	char	*tp;
	int	l;

	if(argc != 2 || m->m_mem[firstarg].typ != TYP_STR ||
		m->m_mem[firstarg + 1].typ != TYP_STR) {
		m->m_mem[result].oper.i = ERR_BADARG;
		m->m_mem[result].typ = TYP_NULL;
		return;
	}
	re = regcomp(m->m_mem[firstarg + 1].oper.c);
	if(re == (regexp *)0) {
		m->m_mem[result].oper.i = ERR_BADARG;
		m->m_mem[result].typ = TYP_NULL;
		return;
	}
	rx = regexec(re,m->m_mem[firstarg].oper.c);
	if(rx == 0) {
		m->m_mem[result].oper.i = ERR_NOTHERE;
		m->m_mem[result].typ = TYP_NULL;
                free(re);                               /* no memory leak */
		return;
	}

	for(l = 0, tp = re->startp[0]; tp != re->endp[0]; tp++)
		l++;

	yp = nsp = tmpalloc((unsigned)l + 1);
	if(nsp == (char *)0) {
		m->m_mem[result].oper.i = ERR_OOM;
		m->m_mem[result].typ = TYP_NULL;
                free(re);                               /* no memory leak */
		return;
	}

	xp = re->startp[0];
	while(xp != re->endp[0])
		*yp++ = *xp++;
	*yp = '\0';

	m->m_mem[result].oper.c = nsp;
	m->m_mem[result].typ = TYP_STR;
        free(re);                               /* no memory leak */
}



/*
set uid, if permitted
*/
void
blt_setruid(m,argc,firstarg,result,uid,euid)
Machine	*m;
int	argc;
int	firstarg;
int	result;
long	*uid;
long	*euid;
{
	if(argc != 1 || m->m_mem[firstarg].typ != TYP_OBJ) {
		m->m_mem[result].typ = TYP_NUM;
		m->m_mem[result].oper.i = 1;
		return;
	}

	/* check perms */
	if(*uid != (long)0 && *euid != (long)0) {
		m->m_mem[result].typ = TYP_NUM;
		m->m_mem[result].oper.i = 1;
		return;
	}

	*uid = m->m_mem[firstarg].oper.l;
	m->m_mem[result].typ = TYP_NUM;
	m->m_mem[result].oper.i = 0;
	return;
}



/*
set user effective uid, if permitted
*/
void
blt_setuid(m,argc,firstarg,result,uid,euid)
Machine	*m;
int	argc;
int	firstarg;
int	result;
long	*uid;
long	*euid;
{
	if(argc != 1 || m->m_mem[firstarg].typ != TYP_OBJ) {
		m->m_mem[result].typ = TYP_NUM;
		m->m_mem[result].oper.i = 1;
		return;
	}

	/* check perms */
	if(*uid != m->m_mem[firstarg].oper.l && *uid != (long)0 && *euid != (long)0) {
		m->m_mem[result].typ = TYP_NUM;
		m->m_mem[result].oper.i = 1;
		return;
	}

	*euid = m->m_mem[firstarg].oper.l;
	m->m_mem[result].typ = TYP_NUM;
	m->m_mem[result].oper.i = 0;
	return;
}





/*
build a string from the arguments. numbers are converted into text
and concatenated together.
*/
void
blt_str(m,argc,firstarg,result,uid,euid)
Machine	*m;
int	argc;
int	firstarg;
int	result;
long	*uid;
long	*euid;
{
	int	totlen = 0;
	char	*ret;
	register char	*cp;
	register char	*ip;
	int	tac = argc;
	int	tag = firstarg;

	/*
	one problem with the way that tmpalloc() is written is that it
	does not elegantly handle realloc(). thus - we need to count
	the length of everything BEFORE we allocate the string.
	this is a trivial waste of CPU.
	*/
	if(tac == 0) {
		m->m_mem[result].typ = TYP_STR;
		m->m_mem[result].oper.c = "";
		return;
	}

	while(tac--) {
		switch(m->m_mem[tag].typ) {
			case	TYP_STR:
			totlen += strlen(m->m_mem[tag].oper.c);
			break;

			case	TYP_OBJ:
			totlen += strlen(itoa((int)m->m_mem[tag].oper.l)) + 1;
			break;

			case	TYP_FUNC:
			totlen += strlen(funcstr);
			break;

			case	TYP_OLIST:
			totlen += strlen(liststr);
			break;

			case	TYP_NUM:
			totlen += strlen(itoa(m->m_mem[tag].oper.i));
			break;
		}
		tag++;
	}

	ip = ret = tmpalloc(totlen + 1);
	if(ret == 0) {
		m->m_mem[result].typ = TYP_NULL;
		m->m_mem[result].oper.i = ERR_OOM;
		return;
	}

	/* now do the actual copy */
	tac = argc;
	tag = firstarg;
	while(tac--) {
		switch(m->m_mem[tag].typ) {
			case	TYP_STR:
			cp = m->m_mem[tag].oper.c;
			break;

			case	TYP_OBJ:
			*ip++ = '#';
			cp = itoa((int)m->m_mem[tag].oper.l);
			break;

			case	TYP_FUNC:
			cp = funcstr;
			break;

			case	TYP_OLIST:
			cp = liststr;
			break;

			case	TYP_NUM:
			cp = itoa(m->m_mem[tag].oper.i);
			break;

			default:
				cp = "";
		}
		while(*cp != '\0' && *cp != MATCH_CHAR)
			*ip++ = *cp++;
		tag++;
	}

	/* e finito! */
	*ip = '\0';
	m->m_mem[result].typ = TYP_STR;
	m->m_mem[result].oper.c = ret;
}



/*
return length of a string, or zero if non-string.
*/
void
blt_strlen(m,argc,firstarg,result,uid,euid)
Machine	*m;
int	argc;
int	firstarg;
int	result;
long	*uid;
long	*euid;
{

	if(argc < 1 || m->m_mem[firstarg].typ != TYP_STR)
		m->m_mem[result].oper.i = 0;
	else
		m->m_mem[result].oper.i = strlen(m->m_mem[firstarg].oper.c);
	m->m_mem[result].typ = TYP_NUM;
}



/*
stick the system's string notion of the time on the stack.
*/
void
blt_strtime(m,argc,firstarg,result,uid,euid)
Machine	*m;
int	argc;
int	firstarg;
int	result;
long	*uid;
long	*euid;
{
	char	*ip;
	char	*op;
	long	thistim;

	if(argc > 0 && m->m_mem[firstarg].typ == TYP_NUM)
		thistim = (long)m->m_mem[firstarg].oper.i;
	else
		(void)time(&thistim);

	/* if your ctime returns > 32 char, you will die! */
	m->m_mem[result].oper.c = tmpalloc(32);
	if(m->m_mem[result].oper.c == 0) {
		m->m_mem[result].typ = TYP_NULL;
		m->m_mem[result].oper.i = ERR_OOM;
		return;
	} 

	m->m_mem[result].typ = TYP_STR;

	ip = ctime(&thistim);
	op = m->m_mem[result].oper.c;
	while(*ip != '\0' && *ip != '\n')
		*op++ = *ip++;
	*op = '\0';
}



/*
stick the system's notion of the time on the stack. - the assumption is
made that a time_t is the same size as an int - not necessarily true.
to fix this, though, a whole new TYP_TIME datatype would need to be
added, and then it would be harder to manipulate with numeric ops. ick.
*/
void
blt_time(m,argc,firstarg,result,uid,euid)
Machine	*m;
int	argc;
int	firstarg;
int	result;
long	*uid;
long	*euid;
{
	m->m_mem[result].typ = TYP_NUM;
	m->m_mem[result].oper.i = (int)time((long *)0);
}

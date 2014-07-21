#include	<stdio.h>
#include	<ctype.h>
#include	<varargs.h>

#include	"ubermud.h"
#include	"externs.h"


/*
Copyright(C) 1990, Marcus J. Ranum, All Rights Reserved.
This software may be freely used, modified, and redistributed,
as long as this copyright message is left intact, and this
software is not used to develop any commercial product, or used
in any product that is provided on a pay-for-use basis.
*/

FILE	*main_logfil = stderr;

char	*
ltoa(num)
long	num;
{
	static	char	rbuf[64];
/*	return(sprintf(rbuf,"%ld",num)); */
	sprintf(rbuf,"%ld",num);
	return rbuf;
}

char	*
itoa(num)
int	num;
{
	static	char	rbuf[64];
/*	return(sprintf(rbuf,"%d",num)); */
	sprintf(rbuf,"%d",num);
	return rbuf;
}



void
logf_open(f)
char	*f;
{
	if(main_logfil != (FILE *)0)
		(void)fclose(main_logfil);
	main_logfil = fopen(f,"a");
}



void
logf_close()
{
	if(main_logfil != (FILE *)0)
		(void)fclose(main_logfil);
	main_logfil = (FILE *)0;
}




/*
fatal error handler. puts strings to stderr as given. if a
given string is == -1, then put the system error message instead.
*/
/* VARARGS */
void
fatal(va_alist)
va_dcl
{
	char	*p;
	va_list	ap;

	if(main_logfil == (FILE *)0)
		exit(1);

	va_start(ap);
	while((p = va_arg(ap,char *)) != (char *)0) {
		if(p == (char *)-1)
			p = sys_errlist[errno];
		(void)fprintf(main_logfil,"%s",p);
	}
	va_end(ap);
	(void)fclose(main_logfil);
	initiateshutdown();
}




/*
print a series of warnings - do not exit
*/
/* VARARGS */
void
logf(va_alist)
va_dcl
{
	char	*p;
	va_list	ap;

	if(main_logfil == (FILE *)0)
		return;

	va_start(ap);
	while((p = va_arg(ap,char *)) != (char *)0) {
		if(p == (char *)-1)
			p = sys_errlist[errno];
		(void)fprintf(main_logfil,"%s",p);
	}
	va_end(ap);
	(void)fflush(main_logfil);
}



/*
enargv tokenizes the provided (and null-terminated) buffer and
sets pointers to each argument in u_av[], returning u_ac
*/
enargv(buf,av,avsiz,tobuf,bsiz)
char	*buf;
char	**av;
int	avsiz;
char	*tobuf;
int	bsiz;
{
	register	char	*ip = buf;
	register	char	*op = tobuf;
	register	char	*sp = 0;
	int		u_ac = 0;
	int		quot = 0;
	int		start = 1;

	if (isspace(*ip) || !isprint(*ip) || *ip == ',') {
		start = 0;
		ip++;
	}

	while(isspace(*ip) || !isprint(*ip) || *ip == ',')
		ip++;

	while(*ip) {
		if(!isprint(*ip)) {
			ip++;
			continue;
		}

		if(!quot && (*ip == '\"' /* || *ip == '\'' */) && (*(ip-1) == ' ' || start)) {
			quot = *ip++;
			continue;
		}

		if(isspace(*ip) && !quot) {
			if(--bsiz < 0)
				return(0);
			*op++ = '\0';

			if(u_ac  + 1 >= avsiz)
				return(u_ac);

			av[u_ac++] = sp;
			sp = av[u_ac] = (char *)0;

			while(isspace(*ip))
				ip++;

			if(*ip == '\0')
				break;
			continue;
		}

		if(quot && *ip == quot && *(ip+1) == '\0') {
			quot = 0;
			ip++;
			continue;
		}

		if(*ip == '\\') {
			if(--bsiz < 0)
				return(0);
			switch(*++ip) {
			case	't':
				*op++ = '\t';
				break;

			case	';':
				*op++ = MATCH_CHAR;
				break;

			case	'n':
				*op++ = '\n';
				break;

			default:
				*op++ = *ip;
			}
			ip++;
			continue;
		}

		if(sp == (char *)0)
			sp = op;
		if(--bsiz < 0)
			return(0);
		*op++ = *ip++;
		if (start) start = 0;
	}

	if(sp != 0) {
		*op = '\0';

		if(u_ac  + 1 >= avsiz)
			return(u_ac);
		av[u_ac++] = sp;
		av[u_ac] = (char *)0;
	}
	return(u_ac);
}

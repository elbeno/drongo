#include	<stdio.h>
#include	<ctype.h>
#include	<sys/types.h>
#include	<sys/stat.h>


#include	"ubermud.h"
#include	"io.h"
#include	"externs.h"


/*
Copyright(C) 1990, Marcus J. Ranum, All Rights Reserved.
This software may be freely used, modified, and redistributed,
as long as this copyright message is left intact, and this
software is not used to develop any commercial product, or used
in any product that is provided on a pay-for-use basis.
*/


/*
#define	MONITORDEBUG
*/

/* our operating environment. */
static	MemOp	maincore[MACHBUFSIZ];
static	Frame	mainframes[MACHFRASIZ];
static	Machine	maincpu = {
	mainframes, maincore, 0, MACHFRASIZ, 0, MACHBUFSIZ
};


static	FILE	*progf = (FILE *)0;
static	long	proguid = (long)0;

/* strings */
static	char	*badwrt = "write failed:";
static	char	*badread = "open for read failed:";
static	char	*badstack = "stack over/underflow!!!\n";
static	char	*badstring = "string table out of space!!!\n";
static	char	*huhmsg = "Sorry, I don't understand.\n";
static	char	*oocsmsg = "out of code space. compilation ignored\n";
static	char	*oossmsg = "out of string space. compilation ignored\n";
static	char	*sffmsg = "function write to system table failed: ";
static	char	*dffmsg = "function write to object failed: ";

/* bad run messages */
static	char	*runfmsg = "function calls nested too deep (aborted)\n";
static	char	*runsmsg = "function requires too much stack (aborted)\n";
static	char	*runimsg = "function contains illegal operand (aborted)\n";
#ifdef	LIMITRUNCPU
static	char	*runcmsg = "function used too many CPU cycles (aborted)\n";
#endif

/* for yyerror - mild kludge */
static	Iob	*curriob;	


/*
scratch the block of data given to the temporary file named after
the user id owning the code. the file pointer is juggled around
here, as needed.
*/
program_in(b,iob)
char	*b;
Iob	*iob;
{

	/* close and re-open output file */
	if(proguid != iob->uid || progf == (FILE *)NULL) {
		char	f[500];

		(void)sprintf(f,"%ld.code",iob->uid);
		if(progf != (FILE *)NULL)
			(void)fclose(progf);
		if((progf = fopen(f,"a")) == (FILE *)NULL) {
			iobput(iob,badwrt,sys_errlist[errno],"\n",0);
			return(0);
		}
		proguid = iob->uid;
	}

	if(fprintf(progf,"%s",b) == EOF)
		iobput(iob,badwrt,sys_errlist[errno],"\n",0);
	return(0);
}


/*
print the size of the program scratch buffer.
*/
void
program_siz(iob)
Iob	*iob;
{

	char	f[500];
	struct	stat	stbuf;

	(void)sprintf(f,"%ld.code",iob->uid);
	if(stat(f,&stbuf))
		return;
	if(stbuf.st_size > 0)
		iobput(iob,"temporary file ",f," currently contains ",
		itoa(stbuf.st_size)," bytes\n",0);
}



/*
in the event of a change of the user's monitor state, make
sure their temp file is closed, written, and ready to go.
*/
void
program_stop(iob)
Iob	*iob;
{
	/* close output file if it is opened to us */
	if(proguid == iob->uid && progf != (FILE *)NULL) {
		(void)fclose(progf);
		progf = (FILE *)NULL;
		proguid = (long)0;
	}
}



/*
unlink the program buffer. >poof<
*/
void
program_flush(iob)
Iob	*iob;
{
	char	f[500];

	/* close output file if it is opened to us */
	if(proguid == iob->uid && progf != (FILE *)NULL) {
		(void)fclose(progf);
		progf = (FILE *)NULL;
		proguid = (long)0;
	}

	(void)sprintf(f,"%ld.code",iob->uid);
	(void)unlink(f);
}



static	void
compile_FILE(fdes,iob,uid,euid)
FILE	*fdes;
Iob	*iob;
long	uid;
long	euid;
{
	int	rv;
	int	runval;
	int	done = 0;
	Oper	op;
	char	*ep;
	long	tmpu = uid;
	long	tmpeu = euid;

	setyyinput(fdes);
	yylineno = 1;

	while(!done) {
		resetmachine(&maincpu);
		cache_reset();
		resetparser();

		rv = yyparse();
		switch(rv) {
			case	COMPILE_OOS:
				if(iob != (Iob *)0)
					iobput(iob,oossmsg,0);
				else
					logf(oossmsg,0);
				break;

			case	COMPILE_OOE:
				if(iob != (Iob *)0)
					iobput(iob,oocsmsg,0);
				else
					logf(oocsmsg,0);
				break;

			case	COMPILE_OK:
				runval = run(&maincpu,yaccprog,0,&tmpu,&tmpeu);
				if(runval != RUN_OK) {
					switch(runval) {
					case RUN_STACK:
						if(iob != (Iob *)0)
							iobput(iob,runsmsg,0);
						else
							logf(runsmsg,0);
						break;
					case RUN_FRAME:
						if(iob != (Iob *)0)
							iobput(iob,runfmsg,0);
						else
							logf(runfmsg,0);
						break;
					case RUN_ILLEGAL:
						if(iob != (Iob *)0)
							iobput(iob,runimsg,0);
						else
							logf(runimsg,0);
						break;
#ifdef	LIMITRUNCPU
					case RUN_CPU:
						if(iob != (Iob *)0)
							iobput(iob,runcmsg,0);
						else
							logf(runcmsg,0);
						break;
#endif
					}
				}
				if(runval == RUN_OK &&
				(maincpu.m_pc != 0 || maincpu.m_fp != 0))
					logf(runsmsg,0);
				break;

			case	COMPILE_FUNC:
				op.p = yaccprog;
				ep = tmpalloc(strlen(funcname())+1+(int)sizeof(long));
				if(ep == (char *)0) {
					fatal("memory allocation failed!\n",0);
					break;
				}
				ELENUM(ep) = funcnum();
				(void)strcpy(ELENAM(ep),funcname());
				if(ELENUM(ep) == 0) {
					int	fv;
					if((fv = sys_freeze(ep,TYP_FUNC,op,uid,euid)) != 0) {
						if(iob != (Iob *)0)
							iobput(iob,sffmsg,errmsg(fv),"\n",0);
						else
							logf(sffmsg,errmsg(fv),"\n",0);
					} else {
						iobput(iob,"compiled func #",
						ltoa(funcnum()),".",
						funcname()," (",
						itoa((int)(PROGSIZE(yaccprog))),
						" bytes)\n",0);
					}
				} else {
					int	fv;
					if((fv = disk_freeze(ep,TYP_FUNC,op,funcop(),uid,euid)) != 0) {
						if(iob != (Iob *)0)
							iobput(iob,dffmsg,errmsg(fv),"\n",0);
						else
							logf(dffmsg,errmsg(fv),"\n",0);
					} else {
						iobput(iob,"compiled func #",
						ltoa(funcnum()),".",
						funcname()," (",
						itoa((int)(PROGSIZE(yaccprog))),
						" bytes)\n",0);
					}
				}
				break;

			case	1:
			case	COMPILE_BAD:
				done++;
				if(iob != (Iob *)0)
					iobput(iob,"(remaining input discarded)\n",0);
				break;

			default:
				done++;
				break;
		}
		tmpfree();
	}
}




void
program_compile(iob)
Iob	*iob;
{

	curriob = iob;

	if(proguid != iob->uid || progf == (FILE *)NULL) {
		char	f[500];

		(void)sprintf(f,"%ld.code",iob->uid);
		if(progf != (FILE *)NULL)
			(void)fclose(progf);
		if((progf = fopen(f,"r")) == (FILE *)NULL) {
			iobput(iob,badread,sys_errlist[errno],"\n",0);
			return;
		}
		proguid = iob->uid;
	}
	compile_FILE(progf,iob,iob->uid,iob->euid);
	(void)fclose(progf);
	progf = (FILE *)NULL;
	curriob = (Iob *)0;
}


#ifdef	USECPP
/*
run a file through the pre-processor and return 0 or the resulting file
perforce, this is system specific!
*/
static	char	*
preprocess_file(f)
char	*f;
{
	extern	char	*mktemp();
	static	char	tmpdef[256] = "/tmp/usubXXXXXX";
	static	char	*tmpf;
	int	sval;
	char	cbuf[600];

	if((tmpf = mktemp(tmpdef)) == NULL)
		return(NULL);
	(void)strcpy(cbuf,DEFAULTCPP);
	(void)strcat(cbuf," ");
	(void)strcat(cbuf,f);
	(void)strcat(cbuf," ");
	(void)strcat(cbuf,tmpf);
	sval = system(cbuf);
	if(sval == 127) {
		logf("cannot invoke c-preprocessor!\n",0);
		return((char *)0);
	}
	if(sval != 0)
		return((char *)0);
	return(tmpf);
}
#endif



compile_file(name)
char	*name;
{
#ifdef	USECPP
	if((name = preprocess_file(name)) == (char *)0) {
		logf("preprocessor run failed\n",0);
		(void)unlink(name);
		return(1);
	}
#endif

	curriob = (Iob *)0;
	if((progf = fopen(name,"r")) == (FILE *)NULL) {
		logf("cannot open ",name,":",(char *)-1,"\n",0);
		return(1);
	}
	compile_FILE(progf,(Iob *)0,(long)0,(long)0);
	(void)fclose(progf);
	progf = (FILE *)NULL;
	curriob = (Iob *)0;

#ifdef	USECPP
	(void)unlink(name);
#endif

	return(0);
}



int
yyerror(s)
char	*s;
{
	if(curriob != (Iob *)0)
		iobput(curriob,s," line ",itoa(yylineno),"\n",0);
	else
		logf(s," line ",itoa(yylineno),"\n",0);
}



static	void
call_with_argv(iob,objno,ac,av,uid,euid)
Iob	*iob;
long	objno;
int	ac;
char	*av[];
long	uid;
long	euid;
{
	int	dt;
	int	strof;
	long	tmpu = uid;
	long	tmpeu = euid;

	/* REALLY important !*/
	resetparser();
	cache_reset();
	resetmachine(&maincpu);

#ifdef	MONITORDEBUG
	printf("calling #%d.%s with %d args\n",objno,av[0],ac - 1);
#endif

	(void)assemble(yaccprog,OP_RPUSH);
	(void)assemble(yaccprog,(int)objno);
	(void)assemble(yaccprog,OP_ELPUSH);

	if((strof = putinstringbuf(av[0])) == -1) {
		logf(badstring,0);
		return;
	}
	(void)assemble(yaccprog,strof);
	(void)assemble(yaccprog,OP_EEVAL);

	for(dt = 1; dt < ac; dt++) {
		(void)assemble(yaccprog,OP_SPUSH);
		if((strof = putinstringbuf(av[dt])) == -1) {
			logf(badstring,0);
			return;
		}
		(void)assemble(yaccprog,strof);
	}

	(void)assemble(yaccprog,OP_CALL);
	(void)assemble(yaccprog,ac - 1);
	(void)assemble(yaccprog,OP_POP);
	(void)assemble(yaccprog,OP_STOP);
	strof = run(&maincpu,yaccprog,0,&tmpu,&tmpeu);

	if(strof != RUN_OK) {
		switch(strof) {
		case RUN_STACK:
			if(iob != (Iob *)0)
				iobput(iob,runsmsg,0);
			logf(runsmsg,0);
			break;
		case RUN_FRAME:
			if(iob != (Iob *)0)
				iobput(iob,runfmsg,0);
			logf(runfmsg,0);
			break;
		case RUN_ILLEGAL:
			if(iob != (Iob *)0)
				iobput(iob,runimsg,0);
			logf(runimsg,0);
			break;
#ifdef	LIMITRUNCPU
		case RUN_CPU:
			if(iob != (Iob *)0)
				iobput(iob,runcmsg,0);
			logf(runcmsg,0);
			break;
#endif
		}
	}
	if(strof == RUN_OK && (maincpu.m_pc != 0 || maincpu.m_fp != 0))
		logf(badstack,0);
	tmpfree();
}



void
call_sysfunc(fname,uid,euid)
char	*fname;
long	uid;
long	euid;
{
	char	*av[2];

	av[0] = fname;
	av[1] = (char *)0;
	call_with_argv((Iob *)0,(long)0,1,av,uid,euid);
}




/*
read a buffer of stuff from the monkey at the other end of the keyboard
and try to resolve it into a function call.
*/
player_in(b,iob)
char	*b;
Iob	*iob;
{
	char	*p = b;
	static	char	*fnamb = 0;
	int	dt;
	Oper	d;
	int	ac;
	char	*av[MUDMAXARG];
	char	abuf[MUDBUFSIZ];
	long	ploc;
	int	fmode;


	if(fnamb == (char *)0) {
		fnamb = malloc(MAXIDENTLEN * 2);
		if(fnamb ==(char *)0)
			fatal("player_in: cannot allocate name buffer\n",0);
	}

	/* skip white-boy space */
	while(*b != '\0' && (isspace(*b) || !isprint(*b)))
		b++;

	/* handle this broken up into lines. */
	while(*b != '\0') {
		int	huh = 1;

		/* slide a pointer ahead and chop into lines */
		while(*p && *p != '\n' && *p != '\r')
			p++;

		while(*p == '\n' || *p == '\r')
			*p++ = '\0';


		/*
		this is a special case - we hot-wire the function call
		to emulate the tiny* say and emote syntax.
		we just hand-assemble our argv and argc, without a call
		to enargv().
		*/
		if(*b == '\"' || *b == ':' || *b == '>') {
			if(*b == '\"') {
				av[0] = "say";
#ifdef	MONITORDEBUG
				printf("hot-wired call to @.say();\n");
#endif
			} else
			if(*b == ':') {
				av[0] = "emote";
#ifdef	MONITORDEBUG
				printf("hot-wired call to @.emote();\n");
#endif
			} else
			if(*b == '>') {
				av[0] = "go";
#ifdef	MONITORDEBUG
				printf("hot-wired call to @.go();\n");
#endif
			}

			/* skip the hot-wire char */
			b++;

			/* skip white-boy space */
			while(*b != '\0' && (isspace(*b) || !isprint(*b)))
				b++;

			/* finish patching up the arg vector. */
			if(*b != '\0') {
				av[1] = b;
				ac = 2;
			} else {
				av[1] = (char *)0;
				ac = 1;
			}

		} else { 
			ac = enargv(b,av,MUDMAXARG,abuf,sizeof(abuf));
		}

		if(ac <= 0 || strlen(av[0]) >= MAXIDENTLEN)
				goto dropthrough;


		/*
		you CAN take this out, if you really want.
		it is where we prohibit calls to functions starting
		with a leading underscore. (except for the Wiz)
		*/
		/* HARDCODE */
		if(av[0][0] == '_' && iob->euid != (long)0)
			goto dropthrough;


#ifdef	MONITORDEBUG
		for(dt = 0; dt < ac; dt++)
			printf("argv[%d]= %s\n",dt,av[dt]);
#endif

		/*
		searching for and calling functions in order. I
		coded this section with gotos because it's that
		much easier to move the blocks of stuff around
		to change what gets called from where.
		*/

		call_sysfunc("_preTurn", iob->uid, iob->euid);

		/* SEARCH system table for matching function name */
		/*
		call it ONLY if it has BLOCK perm set. we search the
		system table again later for the same func, without
		BLOCK set.
		*/
		ELENUM(fnamb) = (long)0;
		(void)strcpy(ELENAM(fnamb),av[0]);
		if(sys_thaw(fnamb,&dt,&d,iob->uid,iob->euid) == 0 && dt == TYP_FUNC) {
			fmode = d.p->p_mode;
			if(fmode & PERM_BLOCK) {
#ifdef	MONITORDEBUG
				printf("call to block sys func\n");
#endif
				call_with_argv(iob,0,ac,av,iob->uid,iob->euid);

				huh = 0;
				if(!(fmode & PERM_CHAIN))
					goto	dropthrough;
			}
		}



		/* SEARCH in the room we are in for bound functions */
		ELENUM(fnamb) = iob->uid;
		/* HARDCODE */
		(void)strcpy(ELENAM(fnamb),"location");
#ifdef	MONITORDEBUG
		printf("determine player location...\n");
#endif
		if(disk_thaw(fnamb,&dt,&d,iob->uid,iob->euid) == 0 && dt == TYP_OBJ) {

			/* wherever you go... */
			ploc = d.l;
#ifdef	MONITORDEBUG
			printf("player is at #%d\n",ploc);
#endif

			/* scan here for func */
			ELENUM(fnamb) = ploc;
			(void)strcpy(ELENAM(fnamb),av[0]);
			if(disk_thaw(fnamb,&dt,&d,iob->uid,iob->euid) == 0 &&
				dt == TYP_FUNC) {
				fmode = d.p->p_mode;
#ifdef	MONITORDEBUG
				printf("call to #%d.%s();\n",ploc,av[0]);
#endif
				call_with_argv(iob,ploc,ac,av,iob->uid,iob->euid);
				huh = 0;

				if(!(fmode & PERM_CHAIN))
					goto	dropthrough;
			}
		}



		/* SEARCH what the player is using */
		/* this section here goes on the assumption that,
		for performance reasons, we only want players to
		be using one object at a time. If not, you will
		need to adapt the code to call match() with the
		list of objects, then probe the matched object
		for the function.
		*/
		ELENUM(fnamb) = iob->uid;
		/* HARDCODE */
		(void)strcpy(ELENAM(fnamb),"inUse");
#ifdef	MONITORDEBUG
		printf("determine what player is using...\n");
#endif
		if(disk_thaw(fnamb,&dt,&d,iob->uid,iob->euid) == 0 && dt == TYP_OBJ) {
#ifdef	MONITORDEBUG
			printf("player is using %d...\n",d.l);
#endif
			ELENUM(fnamb) = d.l;
			(void)strcpy(ELENAM(fnamb),av[0]);
			if(disk_thaw(fnamb,&dt,&d,iob->uid,iob->euid) == 0 &&
				dt == TYP_FUNC) {
				fmode = d.p->p_mode;
#ifdef	MONITORDEBUG
				printf("call to #%d.%s();\n",ELENUM(fnamb),av[0]);
#endif

				call_with_argv(iob,ELENUM(fnamb),ac,av,iob->uid,iob->euid);
				huh = 0;

				if(!(fmode & PERM_CHAIN))
					goto	dropthrough;
			}
		}



		/* SEARCH player for matching function name */
		ELENUM(fnamb) = (long)iob->uid;
		(void)strcpy(ELENAM(fnamb),av[0]);
		if(ELENUM(fnamb) != (long)0 &&
			disk_thaw(fnamb,&dt,&d,iob->uid,iob->euid) == 0 &&
			dt == TYP_FUNC) {

			fmode = d.p->p_mode;
#ifdef	MONITORDEBUG
			printf("call to player: %d.%s();\n",ELENUM(fnamb),av[0]);
#endif
			call_with_argv(iob,iob->uid,ac,av,iob->uid,iob->euid);
			huh = 0;

			if(!(fmode & PERM_CHAIN))
				goto	dropthrough;
		}



		/* SEARCH system table for matching function name */
		ELENUM(fnamb) = (long)0;
		(void)strcpy(ELENAM(fnamb),av[0]);
		if(sys_thaw(fnamb,&dt,&d,iob->uid,iob->euid) == 0 && dt == TYP_FUNC) {
			fmode = d.p->p_mode;

			/*
			if block was set before, we called it before
			so we don't call it again.
			*/
			if(!(fmode & PERM_BLOCK)) {
				call_with_argv(iob,0,ac,av,iob->uid,iob->euid);

				huh = 0;
				if(!(fmode & PERM_CHAIN))
					goto	dropthrough;
			}
		}


dropthrough:
		if(huh)
			iobput(iob,huhmsg,0);

		call_sysfunc("_postTurn", iob->uid, (long)0);

		/* next line if one. */
		b = p;

		/* skip white (again) */
		while(*b != '\0' && (isspace(*b) || !isprint(*b)))
			b++;
	}
	return(0);
}

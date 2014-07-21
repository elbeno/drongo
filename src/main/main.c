#include	<sys/signal.h>
#include	<stdio.h>

#include	"ubermud.h"
#include	"externs.h"


/*
Copyright(C) 1990, Marcus J. Ranum, All Rights Reserved.
This software may be freely used, modified, and redistributed,
as long as this copyright message is left intact, and this
software is not used to develop any commercial product, or used
in any product that is provided on a pay-for-use basis.
*/

#ifdef	BOB
static	int	bobs_unused_int;
#endif

/* set to shut down when runs are completed */
static	int	server_shutdown_now = 0;

static	void createpidfile();

main(ac,av)
int	ac;
char	*av[];
{
	int	x;
	char	*bootf = (char *)0;
	long	uptim;

	for(x = 1; x < ac; x++) {
		if(av[x][0] == '-')
			switch(av[x][1]) {

			case	'd':
				if(chdir(av[x+1])) {
					logf("cannot chdir to ",av[x+1],
						" : ",(char *)-1,"\n",0);
					exit(1);
				}
				break;

			case	'l':
				logf_open(av[x+1]);
				break;

			case	'f':
				bootf = av[x + 1];
				break;

			default:
				exit(usage());
			}
	}

	/* set interrupts and whatnot */
	setinterrupts();

	/* create a file with the process-id, for whatever reason. */
	createpidfile();

	/* open I/O system */
	if(ioinit())
		fatal("cannot open I/O channels!: ",(char *)-1,"\n",0);

	/* open databases and ready cache */
	if(db_open())
		fatal("cannot open database!: ",(char *)-1,"\n",0);

	/* initialize internal symbol tables */
	if(syssyminit())
		fatal("cannot initialize symbol tables: ",(char *)-1,"\n",0);

	/* compile and execute a boot file, if given */
	if(bootf != (char *)0 && compile_file(bootf))
		fatal("boot from \"",bootf,"\" failed!\n",0);

	(void)time(&uptim);
	logf("Server BOOTED: ",ctime(&uptim),0);

	/* loop on I/O */
	while(server_shutdown_now == 0 && ioloop() == 0)
		iosync();

	iowrap();

	db_close();

	logf("--down--\n",0);
	logf_close();

	exit(0);

#ifdef	lint
	return(0);
#endif
}



usage()
{
	(void)fprintf(stderr,"usage: um [-d home directory] [-f bootfile]\n");
	(void)fprintf(stderr,"\t[-l logfile]\n");
	return(1);
}


void
initiateshutdown()
{
	long	downtim;

	(void)time(&downtim);
	logf("Server shutdown request: ",ctime(&downtim),0);
	iobwall("Server going down: ",ctime(&downtim),0);
	server_shutdown_now++;
}


/*
return true if the argument is a string.
*/
void
blt_shutdown(m,argc,firstarg,result,uid,euid)
Machine	*m;
int	argc;
int	firstarg;
int	result;
long	*uid;
long	*euid;
{
	if(*uid == (long)0 || *euid == (long)0) {
		initiateshutdown();
		m->m_mem[result].oper.i = 0;
		m->m_mem[result].typ = TYP_NUM;
	} else {
		m->m_mem[result].oper.i = ERR_PERM;
		m->m_mem[result].typ = TYP_NULL;
	}
}




/*
this is a bizarre little hack. if the server catches a SIGCONT, it will
attempt to unlink the file. this is so that a program can test the
aliveness of the server, by creating the file, signalling the server,
and seeing if the file is still there. the choice to remove the file
rather than to create it is because of the possibility of there being
a dearth of file descriptors. it does not take an fd to remove a file.
*/
static	void
serverisalive()
{
	(void)unlink("server_lives");
}



void
setinterrupts()
{
#ifdef	ULTRIX_BUG
	int	abort();
#endif

#ifdef	HURIN
	/* in fond memory */
	(void)signal(SIGSEGV,SIG_IGN);
#endif
#ifdef	BOB
	/* bad to worse */
	(void)signal(SIGSEGV,main);
#endif
	(void)signal(SIGHUP,SIG_IGN);
	(void)signal(SIGINT,SIG_IGN);
/*
	(void)signal(SIGTSTP,SIG_IGN);
*/

#ifdef	ULTRIX_BUG
	(void)signal(SIGBUS,abort);
	(void)signal(SIGSEGV,abort);
#endif
	(void)signal(SIGPIPE,SIG_IGN);
	(void)signal(SIGWINCH,SIG_IGN);
	(void)signal(SIGSTOP,SIG_IGN);
	(void)signal(SIGIO,SIG_IGN);
	(void)signal(SIGQUIT,initiateshutdown);
	(void)signal(SIGTERM,initiateshutdown);
	(void)signal(SIGCONT,serverisalive);
}



/*
create a file in the server directory with the current process id
in it. this is also generally useful, costs nothing, etc.
*/
static	void
createpidfile()
{
	FILE	*ff;
	
	if((ff = fopen("server_pid","w")) == NULL) {
		logf("cannot open pid file:",(char *)-1,"\n",0);
		return;
	}
	(void)fprintf(ff,"%d\n",getpid());
	(void)fclose(ff);
}

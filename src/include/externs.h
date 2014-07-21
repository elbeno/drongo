/*
Copyright(C) 1990, Marcus J. Ranum, All Rights Reserved.
This software may be freely used, modified, and redistributed,
as long as this copyright message is left intact, and this
software is not used to develop any commercial product, or used
in any product that is provided on a pay-for-use basis.
*/

extern	ObjList	*listcopyinto();
extern	ObjList	*listcopy();
extern	ObjList	*listtmpcopy();

extern	Prog	*progcopyinto();
extern	Prog	*progcopy();
extern	Prog	*yaccprog;

extern	Sym	*symadd();
extern	Sym	*symlook();
extern	Sym	*symnew();

extern	char	*copystr();
extern	char	*ctime();
extern	char	*errmsg();
extern	char	*funcname();
extern	char	*index();
extern	char	*itoa();
extern	char	*ltoa();
extern	char	*malloc();
/*extern	int	sprintf(char *, char *, ...);*/
extern	char	*strcat();
extern	char	*strcpy();
extern	char	*sys_errlist[];
extern	char	*tmpalloc();

extern	int	errno;
extern	int	yychar;
extern	int	yylineno;

extern	long	atol();
extern	long	funcnum();
extern	long	lseek();
extern	long	match();
extern	long	random();
extern	long	time(long *);

extern	int	yyerrflag;
/* was an extern short, but y.tab.c seems to disagree */

extern	void	(*bltintab[])();
extern	void	bcopy();
extern	void	bzero();
extern	void	blt_atoi();
extern	void	blt_atoobj();
extern	void	blt_catfile();
extern	void	blt_chmod();
extern	void	blt_chown();
extern  void    blt_closefile();
extern	void	blt_disconnect();
extern	void	blt_echo();
extern	void	blt_echoto();
extern	void	blt_errno();
extern	void	blt_error();
extern	void	blt_geteuid();
extern  void    blt_getip();
extern	void	blt_getuid();
extern	void	blt_islist();
extern	void	blt_isnum();
extern	void	blt_isobj();
extern	void	blt_isstr();
extern	void	blt_listappend();
extern	void	blt_listcount();
extern	void	blt_listdrop();
extern	void	blt_listelem();
extern	void	blt_listmerge();
extern	void	blt_listnew();
extern	void	blt_listprepend();
extern	void	blt_listsearch();
extern	void	blt_listsetelem();
extern	void	blt_log();
extern	void	blt_match();
extern	void	blt_objectdestroy();
extern	void	blt_objectelements();
extern	void	blt_objectnew();
extern	void	blt_objectowner();
extern  void    blt_openfile();
extern	void	blt_rand();
extern	void	blt_regcmp();
extern	void	blt_regexp();
extern	void	blt_setruid();
extern	void	blt_setuid();
extern	void	blt_shutdown();
extern	void	blt_str();
extern	void	blt_strlen();
extern	void	blt_strtime();
extern	void	blt_time();
extern	void	cache_reset();
extern	void	call_sysfunc();
extern	void	db_close();
extern	void	exit();
/*VARARGS*/
extern	void	fatal();
extern	void	free();
extern	void	initiateshutdown();
extern	void	iobdisconnect();
extern	void	iobdrop();
/*VARARGS*/
extern	void	iobput();
/*VARARGS*/
extern	void	iobtell();
/*VARARGS*/
extern	void	iobwall();
extern	void	iosync();
extern	void	iowrap();
/*VARARGS*/
extern	void	logf();
extern	void	logf_close();
extern	void	logf_open();
extern	void	matchreset();
extern	void	program_compile();
extern	void	program_flush();
extern	void	program_siz();
extern	void	program_stop();
extern	void	resetmachine();
extern	void	resetparser();
extern	void	setinterrupts();
extern	void	setyyinput();
extern	void	srandom();
extern	void	symfreelist();
extern	void	tmpfree();
extern	void	tmpputonfree();

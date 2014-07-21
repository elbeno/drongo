#include	<stdio.h>
#include	<ctype.h>

#include	<sys/types.h>
#include	<sys/file.h>
#include	<sys/time.h>

#include	<sys/socket.h>
#include	<netdb.h>
#include	<netinet/in.h>

/*
	Code-submitter for Ubermud [eventually this should be replaced
        with an intelligent /load option in the client code, hint hint].

	Marcus J. Ranum, 1990, with mods by Random.
	Further mods by Andrew Bolt (-c option for running usubmit on
	same file system as server, to be used with 'sub' shell script
	which automatically creates fridge/ID.code (ID is 8 for Drongo).

	NOTE: To compile on your own system, change the DEFAULTCPP line
	so the -I points to your own include directory (for 'drogon.h')

	Also added UMUDSERVER and -s option. If server is set to UBER,
	@flush, @prog and @comp commands will be automatically generated.
	Otherwise only the contents of the file are dumped to the server.
   
	Use as:
	usubmit -l login -p password -f file
		other options:	-h hostname
				-n port #
				-d defines (for cpp)
				-c compile only (don't send file)
				-s set server name (UBER default)
*/



#define	USECPP

#ifdef	USECPP
#ifndef	DEFAULTCPP
#define	DEFAULTCPP	"/lib/cpp -traditional -P -I/home/bjd1000/drongo/boot"
#endif
char	*preprocess_file();
#endif


#ifndef	DEFAULTHOST
#define	DEFAULTHOST	"j-007.emma.cam.ac.uk"
#endif

/* yes - this is a STRING - it gets interpreted later. */
#ifndef	DEFAULTPORT
#define	DEFAULTPORT	"6666"
#endif

/* size of the largest buffer we can send to a client */
#define		MUDBUFSIZ	512


extern	char	*malloc();
extern	char	*getenv();
extern	char	*strcat();
extern	char	*mktemp();
extern	void	exit();
extern	void	bzero();
extern	void	bcopy();
extern	void	perror();

#ifdef	DGUX
extern	struct	in_addr	inet_addr();
#else
extern	unsigned long	inet_addr();
#endif


int	vflg = 1;	/* run with some verbosity */
char	defines[BUFSIZ];


static	void
sendto_server(fd,s)
int	fd;
char	*s;
{
	int	l = strlen(s);
	if(write(fd,s,l) != l) {
		if(vflg)
			(void)fprintf(stderr,"<<<cannot write to server>>>\n");
		exit(1);
	}
}


main(ac,av)
int	ac;
char	*av[];
{
	static	char	*cmmsg = "\n@compile\n";
	static	char	*prmsg = "\n@program\n";
	static	char	*flmsg = "\n@flush\n";
	static	char	*qmsg = "\n@quit\n";
	int	servfd;
	int	infd;
	int	sv;
	int     ubercompile;
	int	compileonly = 0;

	char	*host;
	char	*portp;
	char	*login;
	char	*pass;
	char    *mudserver;
	char	*fil = NULL;

	char	buf[MUDBUFSIZ];
	int	byts;

	/* look in the env. for starters  */
	if((host = getenv("UMUDHOST")) == NULL)
		host = "";
	if((portp = getenv("UMUDPORT")) == NULL)
		portp = "";
	if((login = getenv("UMUDLOGIN")) == NULL)
		login = "";
	if((pass = getenv("UMUDPASS")) == NULL)
		pass = "";
	if((mudserver = getenv("UMUDSERVER")) == NULL)
	        mudserver = "UBER";

/* 	printf ("You really really shouldn't know that %s's password is %s\n", login, pass);
      - silly diagnostic...
*/

	strcpy(defines,"");

	/* I hate getopt() - so sue me */
	for(byts = 1; byts < ac; byts++) {
		if(av[byts][0] == '-') {
			switch(av[byts][1]) {
				case	'f':	/* file */
					fil = av[byts + 1];
					break;

				case	'h':	/* hostname of mud is... */
					host = av[byts + 1];
					break;

				case	'l':	/* login as... */
					login = av[byts + 1];
					break;

				case	'n':	/* port # of mud is... */
					portp = av[byts + 1];
					break;

				case	'p':	/* password as... */
					pass = av[byts + 1];
					break;

				case	'q':	/* quiet down */
					vflg = 0;
					break;

				case	'd':	/* define passing */
					strcat(defines," -D");
					strcat(defines,av[byts + 1]);
					break;

			        case    's':    /* define server */
					mudserver = av[byts + 1];
					break;

				case    'c':    /* compile only */
					compileonly = 1;
					break;

				default:
					exit(usage());
			}
		}
	}
	
	/* no file set */
	if(fil == NULL && !compileonly) {
		(void)fprintf(stderr,"must provide the name of input file\n");
		exit(1);
	}
	
	/* no password given */
	if(pass == NULL) {
		(void)fprintf(stderr,"must provide a password\n");
		exit(1);
	} else {
		char	*junk;
	char	*pp;
		pp = malloc(strlen(pass) + 1);
		if(pp == (char *)0) {
			perror("malloc");
			exit(1);
		}
		(void)strcpy(pp,pass);
		for(junk = pass; *junk != '\0'; junk++)
			*junk = 'X';
		pass = pp;
	}

	ubercompile = (strcasecmp(mudserver,"UBER")==0);

	if (!compileonly)
	{
#ifdef	USECPP
		if((fil = preprocess_file(fil)) == NULL)
			exit(1);
#endif

		if((infd = open(fil,O_RDONLY,0600)) < 0) {
			perror(fil);
			exit(1);
		}
	
	}

	/* no host set - accept the default. */
	if(*host == '\0')
		host = DEFAULTHOST;

	/* no port set - accept the default. */
	if(*portp == '\0')
		portp = DEFAULTPORT;

	/* connect */
	if((servfd = connect_to(host,portp)) < 0) {
		if(vflg)
			(void)fprintf(stderr,"<<<connection failed>>>\n");
		exit(1);
	}

	if(fork()) {
		/* reader */
		while((byts = read(servfd,buf,MUDBUFSIZ - 1)) > 0) {
			buf[byts] = '\0';
			(void)fprintf(stdout,"%s",buf);
		}
		(void)close(servfd);
		exit(0);
	} else {
		/* writer */

		/* if a login/password was provided, try to use it */
		if(*login != '\0' && *pass != '\0') {
			if(login_user(servfd,login,pass)) {
				(void)fprintf(stderr,"<<<login failed>>>\n");
				exit(1);
			}
		}

                if (ubercompile && !compileonly)
		{
		  sendto_server(servfd,prmsg);
		  sleep(1);
		  sendto_server(servfd,flmsg);
		  sleep(1);
                }

		if (!compileonly)
		{
			/* now we are ready to commence sending data to the server. */
			while((byts = read(infd,buf,MUDBUFSIZ - 1)) > 0) {
				buf[byts] = '\0';
				sendto_server(servfd,buf);
			}

			sleep(4);
		}
		/* send compile directive */
                if (ubercompile)
		{
		  if(vflg)
			(void)fprintf(stderr,"<<<compiling>>>\n");
		  sendto_server(servfd,cmmsg);
		}
		sleep(1);
		/* send quit directive */
		if (ubercompile)
		{
		  sendto_server(servfd,qmsg);
		}
		(void)close(servfd);
		if(vflg)
			(void)fprintf(stderr,"<<<disconnecting>>>\n");
		sleep(4);
#ifdef	USECPP
		(void)unlink(fil);
#endif
		exit(0);
	}
}





int
connect_to(host,portstr)
char	*host;
char	*portstr;
{
	struct	sockaddr_in sin;
	struct	hostent	*hp;
	int	s;
	short	port;
	char	*p = host;

	if(p == 0 || *p == '\0') {
		(void)fprintf(stderr,"no host address provided\n");
		return(-1);
	}

	/*
	here we try to make some sense of the host address we were
	given. if the string is all digits and dots, assume it is a 
	'dot' format address. otherwise, assume it is a named host.
	*/
	while(*p != '\0' && (*p == '.' || isdigit(*p)))
		p++;

	if(*p != '\0') {
/*
		(void)fprintf(stderr,"hostname must be numeric\n");
		return(-1);
*/
		if((hp = gethostbyname(host)) == NULL) {
			(void)fprintf(stderr,"unknown host \"%s\"\n",host);
			return(-1);
		}
		(void)bcopy(hp->h_addr,(char *)&sin.sin_addr,hp->h_length);

	} else {

#ifdef	DGUX
		struct in_addr	f;

		f = inet_addr(host);
		if(f.S_un.S_addr == -1L) {
			(void)fprintf(stderr,"bad host addr \"%s\"\n",host);
			return(-1);
		}
		(void)bcopy((char *)&f.S_un.S_addr,(char *)&sin.sin_addr,sizeof(f.S_un.S_addr));
#else
		unsigned long	f;

		if((f = inet_addr(host)) == -1L) {
			(void)fprintf(stderr,"bad host addr \"%s\"\n",host);
			return(-1);
		}
		(void)bcopy((char *)&f,(char *)&sin.sin_addr,sizeof(f));
#endif

	}


	if(vflg)
		(void)fprintf(stderr,"<<<connecting to %s, port %s>>>\n",host,portstr);

	/* misc host connect setup */
	port = htons(atoi(portstr));
	sin.sin_port = port;
	sin.sin_family = AF_INET;

	/* open a socket */
	if((s = socket(AF_INET,SOCK_STREAM,0)) < 0) {
		perror("socket");
		return(-1);
	}

	/* hook it up */
	if(connect(s,(struct sockaddr *)&sin,sizeof(sin)) < 0) {
		perror("connect");
		return(-1);
	}
	if(vflg)
		(void)fprintf(stderr,"<<<connected>>>\n");
	return(s);
}



usage()
{
	(void)fprintf(stderr,"usage:: usubmit [options]\n");
	(void)fprintf(stderr,"where options are:\n");
        (void)fprintf(stderr,"\t-s\tset server: UBER=>send @program,etc.\n");
	(void)fprintf(stderr,"\t-v\tenable verbose output\n");
	(void)fprintf(stderr,"\t-h name\thost name to connect to\n");
	(void)fprintf(stderr,"\t-n #\tport number to connect to\n");
	(void)fprintf(stderr,"\t-c\tcompile only (don't submit file)\n");
	(void)fprintf(stderr,"\t-l name\tlogin as name\n");
	(void)fprintf(stderr,"\t-p password\tlogin with password\n");
	(void)fprintf(stderr,"\t\t(must provide both)\n");
	return(1);
}


login_user(fd,user,pass)
int	fd;
char	*user;
char	*pass;
{
	int	x;
	char	buf[500];

/* send the user id */
	*buf = '\0';
	(void)strcat(buf,user);
	if(vflg)
		(void)fprintf(stderr,"<<<attempting login '%s'>>>\n",buf);
	(void)strcat(buf,"\n");
	sendto_server(fd,buf);
	sleep(1);

/* send the password */
        *buf = '\0';
	(void)strcat(buf,pass);
	(void)strcat(buf,"\n");
	sendto_server(fd,buf);
	sleep(1);
	return(0);
}



#ifdef	USECPP
/*
run a file through the pre-processor and return 0 or the resulting file
perforce, this is system specific!
*/
char	*
preprocess_file(f)
char	*f;
{
	static	char	*tmpf = "/tmp/usubXXXXXX";
	int	sval;
	char	cbuf[600];

	if((tmpf = mktemp(tmpf)) == NULL)
		return(NULL);
	(void)strcpy(cbuf,DEFAULTCPP);
	if(*defines)
		(void)strcat(cbuf,defines);
	(void)strcat(cbuf," ");
	(void)strcat(cbuf,f);
	(void)strcat(cbuf," ");
	(void)strcat(cbuf,tmpf);
	if(vflg)
		(void)fprintf(stderr,"<<<calling preprocessor:%s>>>\n",cbuf);
	sval = system(cbuf);
	if(sval == 127) {
		(void)fprintf(stderr,"<<<system() - no shell>>>\n");
		return(NULL);
	}
	if(sval != 0) {
		(void)fprintf(stderr,"<<<cpp returns non-zero>>>\n");
		return(NULL);
	}
	return(tmpf);
}
#endif









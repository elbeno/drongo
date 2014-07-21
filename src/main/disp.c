#include	<stdio.h>

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

#ifdef	UNIXLIKE
static	char	*logmsg = "Ubermud 0.9 login: ";
#else
static	char	*logmsg = "Please enter your player ID: ";
#endif

static	char	*badlogmsg = "Login incorrect.\n";
static	char	*twologmsg = "User is already connected.\n";
static	char	*permsg = "permission denied.\n";
static	char	*flsmsg = "flushing temporary file...\n";
static	char	*cmpmsg = "compiling...\n";
static	char	*plymsg = "play mode set.\n";
static	char	*prgmsg = "programming mode set.\n";

extern char echoOn[10];
extern char echoOff[10];

static char userinp[100];

/*
dispatch a buffer of text correctly, depending on the state
of the connection.
*/
dispatch(b,iob)
char	*b;
Iob	*iob;
{
	int	ac;
	char	*av[MUDMAXARG];
	char	abuf[MUDBUFSIZ];

	if(b == (char *)0 || iob == (Iob *)0)
		return(0);

	/* update the idle time */
	call_sysfunc("_updateidle", iob->uid, (long)0);

	/* check for mode switches based on magical escape character */
	if(*b == '@') {
		ac = enargv(&b[1],av,MUDMAXARG,abuf,sizeof(abuf));

		if(ac == 0) {
			iobput(iob,"use: compile, flush, play, program, login, quit\n",0);
			return(0);
		}

		/*
		before changing state, close the temp file, if we have
		it open for this uid.
		*/
		if(iob->typ == INPUT_PROGRAM)
			program_stop(iob);

		if(!strncmp(av[0],"flush",strlen(av[0]))) {
			if((iob->flg & INPUT_PROGAUTH) == 0) {
				iobput(iob,permsg,0);
				return(0);
			}
			iobput(iob,flsmsg,0);
			program_flush(iob);
			return(0);
		}

		if(!strncmp(av[0],"compile",strlen(av[0]))) {
			if((iob->flg & INPUT_PROGAUTH) == 0) {
				iobput(iob,permsg,0);
				return(0);
			}
			iobput(iob,cmpmsg,0);
			program_siz(iob);
			program_compile(iob);
/*			iobput(iob,flsmsg,0);
			program_flush(iob);*/
			return(0);
		}


		if(!strncmp(av[0],"play",strlen(av[0]))) {
			iobput(iob,plymsg,0);
			iob->typ = INPUT_PLAY;
			return(0);
		}


		if(!strncmp(av[0],"program",strlen(av[0]))) {
			if((iob->flg & INPUT_PROGAUTH) != 0) {
				iobput(iob,prgmsg,0);
				program_siz(iob);
				iob->typ = INPUT_PROGRAM;
			} else {
				iobput(iob,permsg,0);
			}
			return(0);
		}


		if(!strncmp(av[0],"login",strlen(av[0]))) {
			if(iob->typ != INPUT_NEWCONN)
				call_sysfunc("_quit",iob->uid,(long)0);
			iob->typ = INPUT_NEWCONN;
			iob->uid = (long)-1;
			iob->euid = (long)-1;
			iobput(iob,logmsg,0);
			return(0);
		}


		if(!strncmp(av[0],"quit",strlen(av[0]))) {
			if(iob->typ != INPUT_NEWCONN)
				call_sysfunc("_quit",iob->uid,(long)0);
			iobdrop(iob);
			return(0);
		}
	}

	switch(iob->typ) {
	case 95:
	case INPUT_NEWCONN:

			if(b[0] == '\0' || b[1] == '\0') {
				iobput(iob,badlogmsg,logmsg,0);
				break;
			}
/*			if (index(b, ' ')) {
			  strcpy(userinp, b);
			  *(index(userinp, ' '))='\0';
			  if (!check_login(index(b, ' ')+1, iob)) {
			    iobput(iob,badlogmsg,logmsg,0);
			    iob->typ=95;
			    break;
			  }
			  iob->typ = INPUT_PLAY;
			  call_sysfunc("_welcome", iob->uid, (long)0);
			  break;
			}
*/
			iob->typ=96;
			strcpy(userinp, b);
			iobput(iob,"          and your password: ", 0);
                        iobput(iob,echoOff,0);
			return(0);
			break;
      case 96:
			if(!check_login(b,iob)) {
				iobput(iob,badlogmsg,logmsg,0);
                                iobput(iob,echoOn,0);
				iob->typ=95;
				break;
			}

			iob->typ = INPUT_PLAY;
                        iobput(iob,echoOn,0);
			call_sysfunc("_welcome",iob->uid,(long)0);
			break;
			
		case INPUT_PLAY:
			if(player_in(b,iob))
				return(1);
			break;

		case INPUT_PROGRAM:
			if(program_in(b,iob))
				return(1);
			break;

		default:
			fatal("bad switch in dispatch()\n",0);
	}
	return(0);
}



greet(iob)
Iob	*iob;
{
	iobput(iob,logmsg,0);
}



/*
check for a user's valid login in the password file.
some attempt is made to correctly ignore TinyTalk if it sends "connect"
instead of something more reasonable.
*/
check_login(s,iob)
char	*s;
Iob	*iob;
{
	FILE	*pwf;
	char	pbuf[BUFSIZ];
	char	*x;
	char	*nam;
	char	*pass;
	char	*uip;
	char	*av[MUDMAXARG];
	int	ac;
	char	*unp;
	char	*upp;
	char	abuf[MUDBUFSIZ];

	ac = enargv(s,av,MUDMAXARG,abuf,sizeof(abuf));
#ifdef RWS_REINSERT
	if(ac < 2 || ac > 3)
		return(0);
#endif
	if((pwf = fopen("passwd","r")) == NULL)
		return(0);

	upp=av[0];

#ifdef RWS_REINSERT
	/* hack for TinyTalk users autologin */
	if(ac == 3 && !strncmp(av[0],"connect",strlen(av[0]))) {
		unp = av[1];
		upp = av[2];
	} else {
		unp = av[0];
		upp = av[1];
	}
#endif
	unp=userinp;

#ifdef RWS_INSERT
	printf("Data: Userid(%s), PassWord(%s)\n", unp, upp);
#endif
	while(fgets(pbuf,BUFSIZ,pwf) != NULL) {
		if(*pbuf == '#' || *pbuf == '\n' || *pbuf == '\0')
			continue;

		/* a pointer to the name */
		nam = pbuf;

		if((x = index(nam,':')) == NULL)
			continue;
		*x++ = '\0';

		/* check the name */
		if(strcmp(nam,unp))
			continue;

		/* a pointer to the password */
		pass = x;

		if((x = index(x,':')) == NULL)
			break;
		*x++ = '\0';

		/* check the password */
		if(pass == NULL || *pass == '\0' || strcmp(pass,upp)) {
			logf("bad password from: ",nam,"\n",0);
			continue;
		}

		/* a pointer to the (ascii) uid */
		uip = x;

		if((x = index(x,':')) == NULL)
			break;
		*x++ = '\0';

		/* set the uid */
		if(uip == NULL || *uip == '\0')
			continue;

		if(iobconnected(atol(uip))) {
			iobput(iob,twologmsg,0);
			logf("dupe connect: ",nam," id#",ltoa(iob->uid),"\n",0);
			continue;
		}

		iob->uid = iob->euid = atol(uip);
		

		/* x now points at what may be the programmer flag */
		if(x != NULL && *x == '1')
			iob->flg = iob->flg | INPUT_PROGAUTH;
		else
		if(x != NULL && *x == 'W') {
			iob->flg = iob->flg | INPUT_PROGAUTH | INPUT_WIZAUTH;
			iob->euid = (long)0;
		}

		/* found: done. */
		(void)fclose(pwf);
		logf("connect: ",nam," id#",ltoa(iob->uid),
			"@",itoa(iob->fd),"\n",0);
		return(1);
	}
	(void)fclose(pwf);
	return(0);
}

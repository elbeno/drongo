#define INPUT_LOGIN 95
#define INPUT_PASS 96
 
#define TELOPTS
#include <arpa/telnet.h>
 
#include <sys/types.h>
#include <ctype.h>
#include <varargs.h>
#include <fcntl.h>
#include <sys/time.h>
#include <sys/errno.h>
#include <sys/socket.h>
#include <netinet/in.h>
 
/** Gordons flags **/
 
#define CRON
#undef CHECK_NEW_MALLOC
#define GNEW_INPUT 1
#undef GBUG
#define TRY_ECHO_OFF_STUFF 1
 
/*
Copyright(C) 1990, Marcus J. Ranum, All Rights Reserved.
This software may be freely used, modified, and redistributed,
as long as this copyright message is left intact, and this
software is not used to develop any commercial product, or used
in any product that is provided on a pay-for-use basis.
*/
 
/*
network layer library for Ubermud. this comprises a completely
'hidden' network interface for TCP/IP. All I/O function calls
are contained in this file.
*/
 
/*
 * Other stuff added by Gordon Henderson:
 *
 */

void checkho(int); 
 
#include "ubermud.h"
#include "io.h"
#include "externs.h"
 
static void iobflush() ;
 
/*
this arguably should be in ubermud.h, but is not, because it is
network-implementation-specific.
*/
 
#define MUDPORT 6666
#define ALTPORT 6999
 
/*
Routines to handle multiplexing I/O. You can plug anything you like into
the I/O routines, if you're not on a BSD machine, but follow this
interface. All you need to know is the following:
 
1) you will get called with ioinit() once at startup.
2) you will get called with ioloop() as long as ioloop() returns 0. each
 call to ioloop() is expected to handle calling dispatch() where
 appropriate.
3) after ioloop() returns 0, iosync() is called.
4) when ioloop() returns 1, iowrap() is called.
*/
 

#undef IODEBUG
 
 
/* old iob handles are kept around, since we may go through these fast */
static Iob *freeiob = (Iob *)0;
 
/* pointer to active Iobs by file descriptor */
static Iob **iobindex;
static int iobindexsiz;
 
/* a real hack, but fun. - figure it out. */
static Iob *lastiob = (Iob *)0;
static int onewrt = 0;
 
/* TCP specific stuff. */
static int  serfd;
struct sockaddr_in addr;
 
char echoOn  [10] ;
char echoOff [10] ;
 
/*
set everything up for I/O. only call ONCE! Need I add 'or else' ?
*/
ioinit(defaultPort)
int defaultPort ;
{
 int x;
 
 sprintf (echoOn,  "%c%c%c%c%c%c%c%c%c",
  IAC, DO,   TELOPT_ECHO, IAC, WONT, TELOPT_ECHO,0) ;
#ifdef TRY_ECHO_OFF_STUFF
 sprintf (echoOff, "%c%c%c%c%c%c%c%c%c",
  IAC, DONT, TELOPT_ECHO, IAC, WILL, TELOPT_ECHO,0) ;
#else
 echoOff [0] = '\0' ;
#endif
 
 
 /*
 if you ain't got dtablesize(), fudge this with whatever
 your systems max file descriptor value is. erring on the
 high side will waste a little memory.
 */
 iobindexsiz = 128 ; /*getdtablesize();*/
 
 
 /* prep index */
 iobindex = (Iob **)malloc((unsigned)(sizeof(Iob) * iobindexsiz));
 if(iobindex == (Iob **)0)
  return(1);
 
 for(x = 0; x < iobindexsiz; x++)
  iobindex[x] = (Iob *)0;
 
 if((serfd = socket(AF_INET,SOCK_STREAM,0)) < 0)
  return(1);
 
 x = 1;
 setsockopt(serfd,SOL_SOCKET,SO_REUSEADDR,(char *)&x,sizeof(x));
 
 addr.sin_family = AF_INET;
 addr.sin_addr.s_addr = INADDR_ANY;
 if (defaultPort)
  addr.sin_port = htons(MUDPORT);
 else
  addr.sin_port = htons(ALTPORT);
 
 if(bind(serfd,(struct sockaddr *)&addr,sizeof(addr))) {
  logf("cannot bind socket: ",(char *)-1,"\n",0);
  return(1);
 }
 
 if(listen(serfd,5) == -1) {
  logf("cannot listen at socket: ",(char *)-1,"\n",0);
  return(1);
 }
 
#ifdef IODEBUG
 printf("I/O system ready\n");
#endif
 return(0);
}

char *tcpio_getip(int uid)
{
  int x;

  for (x=0; x<iobindexsiz; x++) {
    if (iobindex[x] != (Iob *)0 && iobindex[x]->uid == uid &&
	iobindex[x]->state == IOSTATE_OK) {
      return iobindex[x]->ipAddress;
    }
  }
  return (char *)0;
} 

/*
drop an Iob from the active list, stack it on the free list, and
make sure everything is cleaned. if we reach here, we are DAMN
sure not to flush it - who knows what state the connection is in.
*/
void
iobdrop(iob)
Iob *iob;
{
 int x;
 
 for(x = 0; x < iobindexsiz; x++) {
  if(iobindex[x] == iob) {
#ifdef IODEBUG
   printf("close connection on fd %d\n",x);
#endif
   logf("close connect @",itoa(x),"\n",0);
   (void)shutdown(x,2);
   (void)close(x);
 
   /*
   put the Iob on free chain
   pressing the buf-ptr as a list ptr
   */
   iob->oBufp = (char *)freeiob;
   freeiob = iob;
 
   /* mark it as dead */
   iob->state = IOSTATE_HUNGUP;
 
   /* de-index (very important) */
   iobindex[x] = (Iob *)0;
  }
 }
}
 
 
 
/*
iowrap is responsible for shutting down and cleaning everything up.
*/
void
iowrap()
{
 int x;
 Iob *bp;
 
 /* shut down bound socket */
 (void)close(serfd);
 
 /* shut down any I/O ports open */
 for(x = 0; x < iobindexsiz; x++)
  if(iobindex[x] != (Iob *)0)
   iobdrop(iobindex[x]);
 
 for(bp = freeiob; bp != (Iob *)0;) {
  Iob *np = (Iob *)bp->oBufp;
  (void)free((char *)bp);
  bp = np;
 }
 (void)free((char *)iobindex);
 (void)close(serfd);
}
 
 
 
/*
main loop. run through all the active FDs once, handle new connections,
and return.
*/
ioloop()
{
 fd_set ready;
 fd_set xcpt;
 Iob *bp;
 register int n;
 int bcnt;
 char buf[MUDBUFSIZ+1];
 struct timeval timo;
 int scnt;
 struct sockaddr_in sin ;
 int sinLen = sizeof (sin) ;
 
 
#ifdef CRON
 timo.tv_sec  =      0 ;
 timo.tv_usec = 900000 ; /** Just more than once a second **/
#else
 timo.tv_sec = 666;
 timo.tv_usec = 5;
#endif
 
 /* check for new connections */
 FD_ZERO(&ready);
 FD_ZERO(&xcpt);
 
 /* set the fds to check */
 FD_SET(serfd,&ready);
 for(n = 0; n < iobindexsiz; n++) {
  if(iobindex[n] != (Iob *)0) {
   FD_SET(n,&ready);
   FD_SET(n,&xcpt);
  }
 }
 
 if((scnt = select(iobindexsiz,&ready,0,&xcpt,&timo)) < 0) {
  if(errno != EINTR) {
   logf("select in loop failed: ",(char *)-1,"\n",0);
   return(1);
  }
 }
 
 /* not a very interesting run, eh ? */
 if(scnt <= 0)
  return(0);
 
 /* check new connections */
 if(FD_ISSET(serfd,&ready)) {
  n = accept(serfd,(struct sockaddr *)&sin,&sinLen);
  if(n != -1) {
 
   /* allocate a buffer */
   if(freeiob != (Iob *)0) {
    bp = freeiob;
    freeiob = (Iob *)freeiob->oBufp;
   } else {
    bp = (Iob *)malloc(sizeof(Iob));
#ifdef CHECK_NEW_MALLOC
  logf ("-- mallocing Iob: ", itoa (sizeof (Iob)), " bytes.\n", 0) ;
#endif
    if(bp == (Iob *)0) {
     logf("cannot malloc a new Iob: ",
      (char *)-1,"\n",0);
     return(1);
    }
   }
 
   /* initialize it */
   bp->typ     = INPUT_LOGIN;
   bp->uid     = bp->euid = (long)-1;
   bp->flg     = INPUT_DEFAULT;
   bp->state   = IOSTATE_OK;
   bp->fd      = n;
   *bp->oBuf   = '\0';
   *bp->iBuf   = '\0';
   bp->oBufp   = bp->oBuf;
   bp->iBufp   = bp->iBuf;
   bp->oBufcnt = sizeof(bp->oBuf);
   bp->iBufcnt = 0;
   strcpy (bp->ipAddress, (char *) inet_ntoa (sin.sin_addr)) ;
   logf ("New connect @", bp->ipAddress, "\n", 0) ;
 
   /* refuse to talk if we cannot set no blocking */
   if(fcntl(bp->fd,F_SETFL,O_NONBLOCK /*FNDELAY*/) != -1 /*==  0*/) {
 
#ifdef IODEBUG
    printf("new connection fd on %d\n",n);
#endif
 
    iobindex[n] = bp;
 
    /* introduce ourselves */
    iobflush (bp) ;
    iobput (bp, echoOn, 0) ;
    iobflush (bp) ;
    greet(bp);
   } else {
    logf("could not set non-blocking:",(char *)-1,"\n",0);
    iobdrop(bp);
   }
  }
 }
 
 /* check input on existing fds. */
 for(n = 0; n < iobindexsiz; n++) {
 
  /* first check needed to keep from checking serfd */
  if( (iobindex[n] != (Iob *)0) && FD_ISSET(n,&ready)) {
 
#ifdef GNEW_INPUT
 
/*
 * Mods by GH: We may be getting characters from a single-char at a time
 * (Geds braindead PAD) source. We need to suck these until we get a CR or
 * NL, then dispatch ... (This sometimes happens in non-echo mode too,
 * I think)
 *
 */
 
 
  if ((bcnt = read (n, buf, sizeof (buf) - 1)) <= 0) /* Hangup */
  {
    long oid = iobindex[n]->uid;
    int  fcf = iobindex[n]->typ;
 
    iobdrop(iobindex[n]);
    if( (fcf != INPUT_LOGIN) && (fcf != INPUT_PASS) )
      call_sysfunc("_quitHangup",oid,(long)0);
  }
  else
  {
    register Iob  *iob = iobindex [n] ;
    register char *p1  = iob->iBufp ;
    register char *p2  = buf ;
    register char *p3  = iob->iBuf + MUDBUFSIZ - 1 ;
 
    buf [bcnt] = '\0' ;
 
#ifdef GBUG
    printf ("Got %d bytes: ", bcnt) ;
    {
      register int i ;
      for (i = 0 ; i < bcnt ; ++i)
        printf (" %.2X", ((int)buf [i]) & 0xFF) ;
      printf ("\n");
    }
    printf ("iBuf: %.8X, iBufp: %.8X\n", iob->iBuf, iob->iBufp);
#endif
    /** If it's a TELOPT packet, ignore it **/
    if ((buf [0] & 0xFF) == IAC)
#ifdef GBUG
    {
      printf ("Ignored a TELOPT packet, len: %d\n", bcnt) ;
      continue ;
    }
#else
      continue ;
#endif
 
    /** Add it to our buffer, truncate any overflow **/
 
#ifdef GBUG
printf ("adding to buffer\n") ;
#endif
    p1 = iob->iBufp ;
    while ( (*p2 != '\0') && (p1 != p3) )
      *p1++ = *p2++ ;
    *p1 = '\0' ;
 
    iob->iBufp = p1 ;
 
#ifdef GBUG
    printf ("iBuf now: ") ;
    {
      register char *p = iob->iBuf ;
      while (*p != '\0')
      {
        printf (" %.2X", ((int)*p) & 0xFF) ;
        if (p++ == iob->iBufp)
   printf ("<") ;
      }
      printf ("\n");
    }
    printf ("iBuf: %.8X, iBufp: %.8X\n", iob->iBuf, iob->iBufp);
#endif
 
    /** Scan for an NL or CR **/
 
#ifdef GBUG
printf ("scanning for CR or NL\n") ;
#endif

    p1 = iob->iBuf ;
    while (*p1 != '\0')
    {
 
      if ( (*p1 == '\n') || (*p1 == '\r') )
      {
        /** Just gotten a password? **/
        if (iobindex [n]->typ == INPUT_PASS)
        {
   iobput (iobindex [n],   "\n", 0) ;
   iobput (iobindex [n], echoOn, 0) ;
        }
 
#ifdef GBUG
printf ("copying chars\n") ;
#endif
        /** Copy chars to the 1st NL into buf **/
        p2 = iob->iBuf ;
        p3 = buf ;
        while (p2 != p1)
   *p3++ = *p2++ ;
        *p3 = '\0' ;
 
#ifdef GBUG
printf ("dispatch\n") ;
#endif
        /** Dispatch the buf **/
	{
	  register int t;
	  t=dispatch(buf, iobindex[n]);
	  if (t) return 1 ;
	}
 
        /** We may have quit ... **/
        if (iobindex[n] == (Iob *)0)
   break ;
 
#ifdef GBUG
printf ("skip\n") ;
#endif
        /** Skip the NLs & CRs **/
        while ( (*p1 == '\n') || (*p1 == '\r') )
   ++p1 ;
 
#ifdef GBUG
printf ("shuffle\n") ;
#endif
        /** Shuffle iBuf down **/
        p2 = iob->iBuf ;
        while (*p1 != '\0')
   *p2++ = *p1++ ;
        *p2 = '\0' ;
 
        p1 = iob->iBuf ; /** We do it again **/
 
        iob->iBufp = p2 ;
      } /* if p1 */
      else
        ++p1 ;
 
    } /* while */
 
 
#else
    printf("TCPIO read\n");
   bcnt = read(n,buf,sizeof(buf) - 1);
 
   if ((*buf & 0xFF) == 0xFF)
    return 0 ;
 
   if (iobindex [n]->typ == INPUT_PASS) {
     printf("Have had password\n");
    iobput (iobindex [n], "\n", 0) ;
    iobput (iobindex [n], echoOn, 0) ;
   }
 
   /* check hangups */
   if(bcnt <= 0) {
    long oid = iobindex[n]->uid;
    int fcf = iobindex[n]->typ;
 
    iobdrop(iobindex[n]);
    if((fcf != INPUT_LOGIN) && (fcf != INPUT_PASS))
     call_sysfunc("_quitHangup",oid,(long)0);
   } else {
    buf[bcnt] = '\0';
 
    /*
    dispatch branches on the type
    of buffer we are dealing with.
    the Iob pointer is passed, too,
    since it contains uid information
    and whatnot.
 
    for those of you using this outside
    of Ubermud, the iob->typ flag can
    be used in dispatch() to provide a
    simple state machine for a login
    sequence. that is all completely
    irrelevant to this code.
    */

    printf("dispatch()ing (%d)\n", iobindex[n]->typ);

    if (dispatch(buf,iobindex[n])) 
      return(1);

#endif
   }
  } else
  if(iobindex[n] != (Iob *)0 && FD_ISSET(n,&xcpt)) {
   long oid = iobindex[n]->uid;
   int fcf = iobindex[n]->typ;
 
   logf("dropped due to exception: ",(char *)-1,"\n",0);
   iobdrop(iobindex[n]);
   if((fcf != INPUT_LOGIN) && (fcf != INPUT_PASS))
    call_sysfunc("_quit",oid,(long)0);
  }
 }
 return(0);
}
 
 
 
/*
try to flush an Iob. if the IOB would block, avoid the flush and
leave the data in it for later, if we can.
*/
static void
iobflush(iob)
Iob *iob;
{
 int rv;
 
 if(iob->state != IOSTATE_OK)
  return;
 
 rv = write(iob->fd,iob->oBuf,iob->oBufp - iob->oBuf);
 
#ifdef IODEBUG
 printf("attempted write %d bytes, fd %d\n",iob->oBufp - iob->oBuf,iob->fd);
#endif
 if(rv < 0 && errno != EWOULDBLOCK) {
  long oid = iob->uid;
  int fcf = iob->typ;
 
  logf("dropped fd due to bad write: ",(char *)-1,"\n",0);
  iobdrop(iob);
 
  if((fcf != INPUT_LOGIN) && (fcf != INPUT_PASS))
   call_sysfunc("_quit",oid,(long)0);
  return;
 }
 
 /* ignore partial writes and whatnot if we can. <shrug> */
 if(rv > 0 && rv != iob->oBufp - iob->oBuf && iob->oBufcnt > 0)
  return;
 
 iob->oBufp = iob->oBuf;
 iob->oBufcnt = sizeof(iob->oBuf);
}
 
 
 
/*
put text into an Iob. if it overflows, make a half-hearted attempt to
flush it, but don't get carried away, or anything.
all arguments to iobput must be strings, and the list must be terminated
with a (char *)0. warning - some machines have limited lengths of arg
lists that can be passed with varargs, so some sense is required.
*/
/* VARARGS1 */
void
iobput(iob,va_alist)
Iob *iob;
va_dcl
{
 register char *s;
 va_list ap;
 
 if(iob == (Iob *)0 || iob->state != IOSTATE_OK)
  return;
 
 if(lastiob != iob) {
  lastiob = iob;
  onewrt++;
 }
 
 va_start(ap);
 while((s = va_arg(ap,char *)) != (char *)0) {
  while(*s /** && *s != MATCH_CHAR **/ && iob != (Iob *)0) {
 
/** Do a CR for every NL ... GH **/
 
   if (*s == '\n') {
    if(--iob->oBufcnt)
     *iob->oBufp++ = '\r' ;
    else {
     iobflush(iob);
     continue ;
    }
   }
 
   if(--iob->oBufcnt)
    *iob->oBufp++ = *s++;
   else
    iobflush(iob);
  }
 }
 va_end(ap);
}
 
 
 
 
/*
wall a message to all the Iobs and then flush them
*/
/* VARARGS */
void
iobwall(va_alist)
va_dcl
{
 char *s;
 int x;
 va_list ap;
 
 va_start(ap);
 while((s = va_arg(ap,char *)) != (char *)0) {
  for(x = 0; x < iobindexsiz; x++)
   if(iobindex[x] != (Iob *)0)
    iobput(iobindex[x],s,0);
 }
 va_end(ap);
}
 
 
 
 
/*
toast connects belonging to #XX
*/
void
iobdisconnect(uid)
long uid;
{
 int x;
 for(x = 0; x < iobindexsiz; x++)
  if(iobindex[x] != (Iob *)0 && iobindex[x]->uid == uid) {
   iobflush(iobindex[x]);
   iobdrop(iobindex[x]);
  }
}
 
 
 
 
/*
let us know if user #XX is connected.
*/
int
iobconnected(uid)
long uid;
{
 int x;
 if(uid == 0L)
  return(0);
 for(x = 0; x < iobindexsiz; x++)
  if(iobindex[x] != (Iob *)0 && iobindex[x]->uid == uid)
   return(1);
 return(0);
}
 
 
 
 
/*
put text into an Iob by user-id.
it the user-id is connected more than once, they get more than one
copy of the output. as with iobput, all arguments must be strings,
and the list must be terminated with (char *)0.
*/
/* VARARGS1 */
void
iobtell(uid,va_alist)
long uid;
va_dcl
{
 register char *s;
 int x;
 va_list ap;
 
 for(x = 0; x < iobindexsiz; x++) {
  if(iobindex[x] != (Iob *)0 && iobindex[x]->uid == uid &&
   iobindex[x]->state == IOSTATE_OK) {
 
   if(lastiob != iobindex[x]) {
    lastiob = iobindex[x];
    onewrt++;
   }
 
   va_start(ap);
   while((s = va_arg(ap,char *)) != (char *)0) {
    while(*s && *s != MATCH_CHAR && iobindex[x] != (Iob *)0) {
 
/** Do a CR for every NL ... GH **/
 
     if (*s == '\n') {
      if(--iobindex[x]->oBufcnt)
       *iobindex[x]->oBufp++ = '\r';
      else {
       iobflush(iobindex[x]);
       continue;
      }
     }
     if(--iobindex[x]->oBufcnt)
      *iobindex[x]->oBufp++ = *s++;
     else
      iobflush(iobindex[x]);
    }
   }
   va_end(ap);
  }
 }
}
 
 
 
 
/*
flush all the Iobs.
*/
void
iosync()
{
 int n;
 
 if(!onewrt)
  return;
 
 if(onewrt == 1 && lastiob != (Iob *)0 && lastiob->oBufp != lastiob->oBuf) {
#ifdef IODEBUG
  printf("only one iob to flush\n");
#endif
  iobflush(lastiob);
  return;
 }
 
 for(n = 0; n < iobindexsiz; n++) {
  if(iobindex[n] != (Iob *)0 && iobindex[n]->oBufp != iobindex[n]->oBuf) {
#ifdef IODEBUG
   printf("sync connection on fd %d\n",n);
#endif
   iobflush(iobindex[n]);
  }
 }
 
 onewrt = 1;
 lastiob = (Iob *)0;
}

void checkho(int n)
{
  if (iobindex[n]->typ==INPUT_PASS) {
    iobput(iobindex[n], echoOff, 0);
  }
}

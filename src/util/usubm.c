#include <stdio.h>
#include <fcntl.h>
#include <ctype.h>
 
#include <sys/types.h>
#include <sys/file.h>
#include <sys/time.h>
 
#include <sys/socket.h>
#include <netdb.h>
#include <netinet/in.h>

#include <stdlib.h>

#define HENDOBODGE
 
#ifndef DEFAULTHOST
#define DEFAULTHOST "j-007.emma.cam.ac.uk"
#endif
 
/* yes - this is a STRING - it gets interpreted later. */
#ifndef DEFAULTPORT
#define DEFAULTPORT "6666"
#endif
 
/* size of the largest buffer we can send to a client */
#define  MUDBUFSIZ 512
 
 
extern void exit();
#define bcopy(s,d,l) memcpy(d,s,l)
extern void perror();
 
#ifdef DGUX
extern struct in_addr inet_addr();
#else
extern unsigned long inet_addr();
#endif
 
 
int vflg = 1; /* run with some verbosity */
char defines[BUFSIZ];
 
 
static void
sendto_server(fd,s)
int fd;
char *s;
{
 int l = strlen(s);
 if(write(fd,s,l) != l) {
  if(vflg)
   (void)fprintf(stderr,"<<<cannot write to server>>>\n");
  exit(1);
 }
}
 
 
main(ac,av)
int ac;
char *av[];
{
 static char *cmmsg = "@compile\n";
 static char *prmsg = "@program\n";
 static char *flmsg = "@flush\n";
 static char *qmsg  = "@quit\n";
 int servfd;
 FILE   *infd;
 int sv;
 
 char *host;
 char *portp;
 char *login;
 char *pass;
 char *fil = NULL;
 
 char buf[MUDBUFSIZ];
 int byts;
 
 /* look in the env. for starters  */
 if((host = getenv("UMUDHOST")) == NULL)
  host = "";
 if((portp = getenv("UMUDPORT")) == NULL)
  portp = "";
 if((login = getenv("UMUDLOGIN")) == NULL)
  login = "";
 if((pass = getenv("UMUDPASS")) == NULL)
  pass = "";
 
 strcpy(defines,"");
 
 /* I hate getopt() - so sue me */
 for(byts = 1; byts < ac; byts++) {
  if(av[byts][0] == '-') {
   switch(av[byts][1]) {
    case 'f': /* file */
     fil = av[byts + 1];
     break;
 
    case 'h': /* hostname of mud is... */
     host = av[byts + 1];
     break;
 
    case 'l': /* login as... */
     login = av[byts + 1];
     break;
 
    case 'n': /* port # of mud is... */
     portp = av[byts + 1];
     break;
 
    case 'p': /* password as... */
     pass = av[byts + 1];
     break;
 
    case 'q': /* quiet down */
     vflg = 0;
     break;
 
    case 'd': /* define passing */
     strcat(defines," -D");
     strcat(defines,av[byts + 1]);
     break;
 
    default:
     exit(usage());
   }
  }
 }
 
 /* no password given */
 if(pass == NULL) {
  (void)fprintf(stderr,"must provide a password\n");
  exit(1);
 } else {
  char *junk;
 char *pp;
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
   (void)fprintf(stdout,"%s",buf); fflush(stdout);
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
  sendto_server(servfd,prmsg);
  sleep(1);
 
  /* send compile directive */
  if(vflg)
   (void)fprintf(stderr,"<<<compiling>>>\n");

  sendto_server(servfd,cmmsg);
  
  if(vflg)
   (void)fprintf(stderr,"<<<disconnecting>>>\n");
  sendto_server(servfd,qmsg);
  (void)close(servfd);

  exit(0);
 }
}
 
 
 
 
 
int
connect_to(host,portstr)
char *host;
char *portstr;
{
 struct sockaddr_in sin;
 struct hostent *hp;
 int s;
 short port;
 char *p = host;
 
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
  if((hp = gethostbyname(host)) == NULL) {
   (void)fprintf(stderr,"unknown host \"%s\"\n",host);
   return(-1);
  }
  (void)bcopy(hp->h_addr,(char *)&sin.sin_addr,hp->h_length);
 } else {
 
#ifdef DGUX
  struct in_addr f;
 
  f = inet_addr(host);
  if(f.S_un.S_addr == -1L) {
   (void)fprintf(stderr,"bad host addr \"%s\"\n",host);
   return(-1);
  }
  (void)bcopy((char *)&f.S_un.S_addr,(char *)&sin.sin_addr,sizeof(f.S_un.S_addr));
#else
  unsigned long f;
 
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
 (void)fprintf(stderr,"\t-v\tenable verbose output\n");
 (void)fprintf(stderr,"\t-h name\thost name to connect to\n");
 (void)fprintf(stderr,"\t-n #\tport number to connect to\n");
 (void)fprintf(stderr,"\t-l name\tlogin as name\n");
 (void)fprintf(stderr,"\t-p password\tlogin with password\n");
 (void)fprintf(stderr,"\t\t(must provide both)\n");
 return(1);
}
 
 
login_user(fd,user,pass)
int fd;
char *user;
char *pass;
{
 int x;
 char buf[500];
 
 sprintf(buf, "\"%s\" %s\n", user, pass);

 sleep(2);
 if (vflg)
   (void)fprintf(stderr,"<<<sending '%s'>>>\n", buf);
 sendto_server(fd,buf);
 sleep(2);
 return(0);

}

 
#ifndef _INCL_BUFFER_H
 
 
/*
Copyright(C) 1990, Marcus J. Ranum, All Rights Reserved.
This software may be freely used, modified, and redistributed,
as long as this copyright message is left intact, and this
software is not used to develop any commercial product, or used
in any product that is provided on a pay-for-use basis.
*/
 
#define IOSTATE_OK 0
#define IOSTATE_ERR 1
#define IOSTATE_HUNGUP 2
 
/*
an Iob is a per-user I/O buffer with file descriptor, type flag,
etc. Ubermud specific fields are 'typ' and 'uid'.
*/
typedef struct {
 int  typ; /* defined in ubermud.h */
 long  uid;
 long  euid;
 int  flg; /* programmer/wiz bit */
 int  state; /* connection status */
 
 int  fd;
 
 char  *oBufp,          *iBufp;
 char   oBuf[MUDBUFSIZ], iBuf [MUDBUFSIZ];
 int   oBufcnt,         iBufcnt;
 
 char  ipAddress [20]; /* nnn.nnn.nnn.nnn == 15, should be enough */

 char  login[MUDBUFSIZ] ;
 
} Iob;
 
#define _INCL_BUFFER_H
#endif

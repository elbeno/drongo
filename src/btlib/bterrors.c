#include	<sys/types.h>
#include	<stdio.h>
#include	"btconf.h"
#include	"btree.h"

/*
         (C) Copyright, 1988, 1989 Marcus J. Ranum
                    All rights reserved


          This software, its documentation,  and  supporting
          files  are  copyrighted  material  and may only be
          distributed in accordance with the terms listed in
          the COPYRIGHT document.

	$Log:	bterrors.c,v $
 * Revision 1.1  90/06/08  16:11:44  mjr
 *  
 * 
 * Revision 1.1  90/06/06  15:03:42  mjr
 *  
 * 
 * Revision 1.1  90/06/03  16:23:08  mjr
 *  
 * 
 * Revision 1.1  90/05/18  23:20:35  mjr
 *  
 * 
 * Revision 1.1  90/05/15  13:18:28  mjr
 * Initial revision
 * 
 * Revision 1.1  90/05/05  15:04:33  mjr
 * Initial revision
 * 
 * Revision 1.1  90/03/23  15:03:41  mjr
 * Initial revision
 * 
*/

#ifndef	lint
static	char	*rcsid = "$Header: /atreus/mjr/hacks/mud/btlib/RCS/bterrors.c,v 1.1 90/06/08 16:11:44 mjr Exp $";
#endif


char	*bt_errs[] = {
/* BT_NOERROR */		"no btree error",
/* BT_KTOBIG */			"btree key too big",
/* BT_ZEROKEY */		"zero sized btree key",
/* BT_DUPKEY */			"duplicate btree key",
/* BT_PTRINVAL */		"invalid btree pointer",
/* BT_NOBUFFERS */		"insufficient btree buffers",
/* BT_LTOOBIG */		"btree label too big",
/* BT_BTOOSMALL */		"btree buffer too small",
/* BT_BADPAGE */		"btree page is corrupt",
/* BT_PAGESRCH */		"cannot search btree page",
/* BT_BADUSERARG */		"invalid btree argument",
0};

/* system error # */
extern	int	errno;

void
bt_perror(b,s)
BT_INDEX	*b;
char		*s;
{
	static	char	*cmesg = "cannot open";
	static	char	*fmt1 = "%s\n";
	static	char	*fmt2 = "%s: %s\n";

	if(b == NULL) {
		if(s == NULL || *s == '\0')
			(void)fprintf(stderr,fmt1,cmesg);
		else
			(void)fprintf(stderr,fmt2,s,cmesg);
		return;
	}
	if(bt_errno(b) == BT_NOERROR && errno != 0) {
		perror(s);
	} else {
		if(s == NULL || *s == '\0')
			(void)fprintf(stderr,fmt1,bt_errs[bt_errno(b)]);
		else
			(void)fprintf(stderr,fmt2,s,bt_errs[bt_errno(b)]);
	}
}

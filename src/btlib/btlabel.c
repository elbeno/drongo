#include	<sys/types.h>
#include	<sys/file.h>
#include	"btconf.h"
#include	"btree.h"

/*
         (C) Copyright, 1988, 1989 Marcus J. Ranum
                    All rights reserved


          This software, its documentation,  and  supporting
          files  are  copyrighted  material  and may only be
          distributed in accordance with the terms listed in
          the COPYRIGHT document.

	$Log:	btlabel.c,v $
 * Revision 1.1  90/06/08  16:11:47  mjr
 *  
 * 
 * Revision 1.1  90/06/06  15:03:45  mjr
 *  
 * 
 * Revision 1.1  90/06/03  16:23:12  mjr
 *  
 * 
 * Revision 1.1  90/05/18  23:20:38  mjr
 *  
 * 
 * Revision 1.1  90/05/15  13:18:31  mjr
 * Initial revision
 * 
 * Revision 1.1  90/05/05  15:04:35  mjr
 * Initial revision
 * 
 * Revision 1.1  90/03/23  15:03:44  mjr
 * Initial revision
 * 
*/


#ifndef	lint
static char *rcsid = "$Header: /atreus/mjr/hacks/mud/btlib/RCS/btlabel.c,v 1.1 90/06/08 16:11:47 mjr Exp $";
#endif

extern	off_t	lseek();

bt_wlabel(b,buf,siz)
BT_INDEX	*b;
bt_chrp		buf;
int		siz;
{
	if(siz > BT_LABSIZ(b)) {
		bt_errno(b) = BT_LTOOBIG;
		return(BT_ERR);
	}

	if((lseek(bt_fileno(b),(off_t)sizeof(struct bt_super),SEEK_SET) !=
		(off_t)sizeof(struct bt_super)) ||
		(write(bt_fileno(b),(char *)buf,siz) != siz))
			return(BT_ERR);
	bt_clearerr(b);
	return(BT_OK);
}

bt_rlabel(b,buf,siz)
BT_INDEX	*b;
bt_chrp		buf;
int		siz;
{
	if(siz < BT_LABSIZ(b)) {
		bt_errno(b) = BT_BTOOSMALL;
		return(BT_ERR);
	}

	if((lseek(bt_fileno(b),(off_t)sizeof(struct bt_super),SEEK_SET) !=
		(off_t)sizeof(struct bt_super)) ||
		(read(bt_fileno(b),(char *)buf,BT_LABSIZ(b)) != BT_LABSIZ(b)))
			return(BT_ERR);
	bt_clearerr(b);
	return(BT_OK);
}

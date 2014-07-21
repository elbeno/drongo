#include	<sys/types.h>
#include	<stdio.h>
#include	"btconf.h"
#include	"btree.h"
#include	"btintern.h"

/*
         (C) Copyright, 1988, 1989 Marcus J. Ranum
                    All rights reserved


          This software, its documentation,  and  supporting
          files  are  copyrighted  material  and may only be
          distributed in accordance with the terms listed in
          the COPYRIGHT document.

	$Log:	btfind.c,v $
 * Revision 1.1  90/06/08  16:11:44  mjr
 *  
 * 
 * Revision 1.1  90/06/06  15:03:42  mjr
 *  
 * 
 * Revision 1.1  90/06/03  16:23:09  mjr
 *  
 * 
 * Revision 1.1  90/05/18  23:20:36  mjr
 *  
 * 
 * Revision 1.1  90/05/15  13:18:29  mjr
 * Initial revision
 * 
 * Revision 1.1  90/05/05  15:04:33  mjr
 * Initial revision
 * 
 * Revision 1.2  90/05/01  12:08:39  mjr
 * *** empty log message ***
 * 
 * Revision 1.1  90/03/23  15:03:41  mjr
 * Initial revision
 * 
*/


#ifndef	lint
static char *rcsid = "$Header: /atreus/mjr/hacks/mud/btlib/RCS/btfind.c,v 1.1 90/06/08 16:11:44 mjr Exp $";
#endif

bt_find(b,key,len,rrn)
BT_INDEX	*b;
bt_chrp		key;
int		len;
off_t		*rrn;
{
	struct	bt_cache *op;	/* old page */
	int	sr;

	if(len > BT_MAXK(b)) {
		bt_errno(b) = BT_KTOOBIG;
		return(BT_ERR);
	}

	if(len <= 0) {
		bt_errno(b) = BT_ZEROKEY;
		return(BT_ERR);
	}

	if(bt_seekdown(b,key,len) == BT_ERR)
		return(BT_ERR);

	if((op = bt_rpage(b,b->stack[b->sblk.levs - 1].pg)) == NULL)
		return(BT_ERR);

	b->cpag = op->num;

	/* mark all as well with tree */
	bt_clearerr(b);

	sr = bt_srchpg(key,len,bt_dtype(b),b->cmpfn,rrn,&b->cky,op->p);
	if(sr == BT_OK)
		return(BT_OK);
	if(sr == BT_ERR) {
		bt_errno(b) = BT_PAGESRCH;
		return(BT_ERR);
	}
	b->cky--;
	return(BT_NF);
}

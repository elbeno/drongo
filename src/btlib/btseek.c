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

	$Log:	btseek.c,v $
 * Revision 1.1  90/06/08  16:11:51  mjr
 *  
 * 
 * Revision 1.1  90/06/06  15:03:51  mjr
 *  
 * 
 * Revision 1.1  90/06/03  16:23:14  mjr
 *  
 * 
 * Revision 1.1  90/05/18  23:20:41  mjr
 *  
 * 
 * Revision 1.1  90/05/15  13:18:38  mjr
 * Initial revision
 * 
 * Revision 1.1  90/05/05  15:04:38  mjr
 * Initial revision
 * 
 * Revision 1.1  90/03/23  15:04:00  mjr
 * Initial revision
 * 
*/


#ifndef	lint
static char *rcsid = "$Header: /atreus/mjr/hacks/mud/btlib/RCS/btseek.c,v 1.1 90/06/08 16:11:51 mjr Exp $";
#endif

extern	char	*realloc();

bt_seekdown(b,key,len)
BT_INDEX	*b;
bt_chrp		key;
int		len;
{
	/* bt_find just seeks down to leaf level, sets the stack, ends */

	int	l = 0;
	int	sr;
	struct	bt_cache *p;

	b->stack[0].pg = b->sblk.root;

	while(1) {
		if((p = bt_rpage(b,b->stack[l].pg)) == NULL)
			return(BT_ERR);
		if(HIPT(p->p) == BT_NULL) {
			return(BT_OK);
		}
		sr = bt_srchpg(key,len,bt_dtype(b),b->cmpfn,&b->stack[l + 1].pg,&b->stack[l].ky,p->p);
		if(sr == BT_ERR) {
			bt_errno(b) = BT_PAGESRCH;
			return(BT_ERR);
		}

		/* dynamically deal w/ stack overruns */
		if(l == b->shih - 2) {
			b->shih += 3;
			b->stack = (struct bt_stack *)realloc((char *)b->stack,(unsigned)(b->shih * sizeof(struct bt_stack)));
			if(b->stack == NULL)
				return(BT_ERR);
		}

		l++;
	}
}

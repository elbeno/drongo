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

	$Log:	btravrs.c,v $
 * Revision 1.1  90/06/08  16:11:50  mjr
 *  
 * 
 * Revision 1.1  90/06/06  15:03:51  mjr
 *  
 * 
 * Revision 1.1  90/06/03  16:23:14  mjr
 *  
 * 
 * Revision 1.1  90/05/18  23:20:40  mjr
 *  
 * 
 * Revision 1.1  90/05/15  13:18:36  mjr
 * Initial revision
 * 
 * Revision 1.1  90/05/05  15:04:37  mjr
 * Initial revision
 * 
 * Revision 1.1  90/03/23  15:03:59  mjr
 * Initial revision
 * 
*/


#ifndef	lint
static char *rcsid = "$Header: /atreus/mjr/hacks/mud/btlib/RCS/btravrs.c,v 1.1 90/06/08 16:11:50 mjr Exp $";
#endif

bt_traverse(b,d,kbuf,maxlen,retlen,retrrv)
BT_INDEX	*b;
int		d;
bt_chrp		kbuf;
int		maxlen;
int		*retlen;
off_t		*retrrv;
{
	struct	bt_cache *p;
	off_t	savp;

	/* if the current page is not set, just zing up to the head */
	/* or the tail - the opposite side of the tree, whatever it is */
	if(b->cpag == BT_NULL)
		if(bt_goto(b,d == BT_EOF ? BT_BOF:BT_EOF) == BT_ERR) 
			return(BT_ERR);

	/* save */
	savp = b->cpag;

	if((p = bt_rpage(b,b->cpag)) == NULL)
		return(BT_ERR);

	/* adjust current key # */
	if(d == BT_EOF)
		b->cky++;
	else
		b->cky--;

	/* have we run out of keys in the page ? */
	while((d == BT_EOF && b->cky >=  KEYCNT(p->p)) || 
		(d == BT_BOF && b->cky < 0)) {

		if(d == BT_EOF)
			b->cpag = RSIB(p->p);
		else
			b->cpag = LSIB(p->p);

		/* we are there - wherever there is */
		if(b->cpag != BT_NULL) {
			if((p = bt_rpage(b,b->cpag)) == NULL)
				return(BT_ERR);

			/* reset current key */
			if(d == BT_EOF)
				b->cky = 0;
			else
				b->cky = KEYCNT(p->p) - 1;
		} else {
			/* we have actually HIT the end */
			/* restore current page */
			b->cpag = savp;
			/* and point the key just past the end of the page */
			if(d == BT_EOF)
				b->cky = KEYCNT(p->p);
			else
				b->cky = -1;
			*retlen = 0;
			*retrrv = 0;
			return(d);
		}
	}

	if(bt_keyof(b->cky,p->p,kbuf,retlen,retrrv,maxlen) == BT_ERR)
		return(BT_ERR);

	/* mark all as well with tree */
	bt_clearerr(b);
	return(BT_OK);
}

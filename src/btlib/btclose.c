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

	$Log:	btclose.c,v $
 * Revision 1.1  90/06/08  16:11:42  mjr
 *  
 * 
 * Revision 1.1  90/06/06  15:03:40  mjr
 *  
 * 
 * Revision 1.1  90/06/03  16:23:07  mjr
 *  
 * 
 * Revision 1.1  90/05/18  23:20:34  mjr
 *  
 * 
 * Revision 1.1  90/05/15  13:18:27  mjr
 * Initial revision
 * 
 * Revision 1.1  90/05/05  15:03:54  mjr
 * Initial revision
 * 
 * Revision 1.1  90/03/23  15:03:38  mjr
 * Initial revision
 * 
*/

#ifndef	lint
static char *rcsid = "$Header: /atreus/mjr/hacks/mud/btlib/RCS/btclose.c,v 1.1 90/06/08 16:11:42 mjr Exp $";
#endif

bt_close(b)
BT_INDEX	*b;
{
	struct	bt_cache *p1;
	struct	bt_cache *p2;
	int	rv;

	rv = bt_sync(b);

	/* close fd */
	if(bt_fileno(b) != -1)
		if(close(bt_fileno(b)) < 0)
			rv = BT_ERR;

	/* free stack */
	if(b->stack != NULL)
		(void)free((char *)b->stack);

	/* free cache */
	for(p1 = b->lru; p1 != NULL;) {

		/* K&R say not to use free()d stuff */
		p2 = p1->next;

		/* free it */
		(void)free((char *)p1->p);
		(void)free((char *)p1);
		p1 = p2;
	}

	/* free handle itself */
	(void)free((char *)b);
	return(rv);
}

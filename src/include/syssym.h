#ifndef	_INCL_SYS_SYM_H

#define	SYSSYMWIDTH	97

/*
Copyright(C) 1990, Marcus J. Ranum, All Rights Reserved.
This software may be freely used, modified, and redistributed,
as long as this copyright message is left intact, and this
software is not included with any commercial product, or used
in any product that is provided on a pay-for-use basis.
*/

/* holder for a single system symbol */
typedef	struct	syssym {
	char	*nam;
	ObjIno	ino;
	Oper	op;
	struct	syssym	*next;
} SysSym;

extern	SysSym	*sys_getsym();

#define	_INCL_SYS_SYM_H
#endif

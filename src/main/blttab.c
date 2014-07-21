#include	"ubermud.h"
#include	"externs.h"


/*
Copyright(C) 1990, Marcus J. Ranum, All Rights Reserved.
This software may be freely used, modified, and redistributed,
as long as this copyright message is left intact, and this
software is not used to develop any commercial product, or used
in any product that is provided on a pay-for-use basis.
*/

/*
function dispatch table for built-ins. each builtin is given a unique
number in parse.y/lookup() which is used as an index into this table.
NOTE - because these values get compiled into stored objects, changing
the ordinal numbers of builtins will destroy any existing databases!
*/

/* these must match the order/number given in parse.y/lookup() */
void (*bltintab[])() = {
	blt_atoi,
	blt_atoobj,
	blt_catfile,
	blt_chmod,
	blt_chown,
	blt_disconnect,
	blt_echo,
	blt_echoto,
	blt_errno,
	blt_error,
	blt_geteuid,
	blt_getuid,
	blt_islist,
	blt_isnum,
	blt_isobj,
	blt_isstr,
	blt_listprepend,
	blt_listappend,
	blt_listcount,
	blt_listdrop,
	blt_listelem,
	blt_listmerge,
	blt_listnew,
	blt_listsearch,
	blt_listsetelem,
	blt_log,
	blt_match,
	blt_objectdestroy,
	blt_objectelements,
	blt_objectnew,
	blt_objectowner,
	blt_rand,
	blt_regcmp,
	blt_regexp,
	blt_setruid,
	blt_setuid,
	blt_shutdown,
	blt_str,
	blt_strlen,
	blt_strtime,
	blt_time,
	blt_openfile,
	blt_closefile,
	blt_getip,
	0
};

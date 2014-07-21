#ifndef	_INCL_STORE_H

#define	CACHEWIDTH	173
#define	CACHEDEPTH	21

/*
the amount of wasted space to permit in object storage. make this
a reasonable number. the larger it is, the more disk space you
waste, but the less I/O you do. setting this to more than 12 is
probably a bad idea.
*/
#define	STORESLOP	8

/*
Copyright(C) 1990, Marcus J. Ranum, All Rights Reserved.
This software may be freely used, modified, and redistributed,
as long as this copyright message is left intact, and this
software is not included with any commercial product, or used
in any product that is provided on a pay-for-use basis.
*/


/*
WARNING: assumptions are made here::
these FreeEnts are stuffed into a b+tree to maintain the free list.
This is no problem, EXCEPT that since the FreeEnts are numeric
data, on a machine that requires strict alignment, a FreeEnt MUST
be sized to a multiple of the system word size. Since this is an
int and a long, it will almost always work, BUT if you start
getting code dumps when freeing things, check the size of this
data structure.
*/
typedef	struct	{
	int	siz;
	long	offt;
} FreeEnt;


/*
a file containing blocks of inodes
*/
typedef	struct	{
	struct	{
		long	free;
		long	high;
	} sblk;
	int	fd;
} InoFile;

/* extern */ static	InoFile	*inoopen();


/*
a file of variable-length chunks.
*/
typedef	struct	{
	struct	{
		long	free;
		long	high;
	} sblk;
	BT_INDEX	*fbt;
	int		fd;
} ChunkFile;

/* extern */ static	ChunkFile	*chunkopen();


/* holder for a single cache element */
typedef	struct	cachelem {
	long	onum;
	ObjIno	ino;
	struct	cachelem	*next;
	struct	cachelem	*prev;
} CacheElem;

/*
a cache chain is laid out with 2 pairs of pointers: the ohead points to
the head of the 'old' side of the cache, the 'otail' to the last element
in the old side. the 'ahead' points to the first element in the 'active'
side of the cache, and the 'atail' to its tail.
when an object is accessed out of the cache, the ahead list is searched,
and if the object is found, the data is returned directly. if it is NOT
found in the active side, the old side is searched. a hit on the 'old' 
side causes the object to be unlinked from the old side and prepended 
to the active side. this ensures that things that are being used by the
interpreter do NOT get flushed at an inconvenient time. if the object
is still unfound, the tail element of the old cache is flushed and the
data is brought in off disk. the element is then added to the active
side. 
when an interpreter run is completed, the active cache chains are all
automagically prepended to the old cache side (effecting an LRU of
sorts) - an operation that is relatively cheap.
*/

typedef	struct	{
	CacheElem *ohead, *otail, *ahead, *atail;
} CacheChain;

#define	_INCL_STORE_H
#endif

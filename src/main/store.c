#include	<sys/types.h>
#include	<sys/file.h>

#include	"btree.h"

#include	"ubermud.h"
#include	"store.h"
#include	"syssym.h"
#include	"externs.h"


/*
Copyright(C) 1990, Marcus J. Ranum, All Rights Reserved.
This software may be freely used, modified, and redistributed,
as long as this copyright message is left intact, and this
software is not used to develop any commercial product, or used
in any product that is provided on a pay-for-use basis.
*/

/*
routines involved with storing data to and from disk (mostly called from
the freeze/thaw code). support is included for InoFiles (files of base
objects) as well as ChunkFiles (files of compiled programs and strings)

In fact, keeping your paws OUT of this code is probably the smartest
thing you'll do in a long time.

COOL THINGS THAT CAN STILL BE DONE TO MAKE THIS FASTER::
1) put a level of caching for hits in front of the b+tree routines
	using a wide hash table, say 5 deep, with updating as needed.
	this MIGHT speed things about 5%.
2) make chunkalloc() split large chunks into smaller chunks when the
	only avail. chunk is large. this results in some storage fragging
	but would probably be better in the long run.
*/


/*
#define	FREEZEDEBUG
#define	THAWDEBUG
#define	CHUNKDEBUG
#define	ALLOCDEBUG
#define	CACHEDEBUG
*/


/* the system base object repository */
static	InoFile		*sys_ino = (InoFile *)0;

/* b+tree index of object names */
static	BT_INDEX	*sys_idx = (BT_INDEX *)0;

/* repository for random chunks of other data */
static	ChunkFile	*sys_chunk = (ChunkFile *)0;

/* system cache headers */
static	CacheChain	sys_cache[CACHEWIDTH];



/*
return the length of an elem padded UP to the nearest word-boundary
this is vitally necessary, or the b+tree will die a screaming death.
if you run this under Sabre-C, you'll get errors, because the b+tree
code will take stuff past the end of the element name. this is not a
problem.
*/
static	int
elelen(elem)
char	*elem;
{
	int	l;
	l = strlen(ELENAM(elem)) + sizeof(long) + 1;
	return(l + (sizeof(unsigned long) - (l % sizeof(unsigned long))));
}




/*
free block comparison funct. passed to b+tree routines
*/
static	int
freecompare(f1,l1,f2,l2)
FreeEnt	*f1;
int	l1;
FreeEnt	*f2;
int	l2;
{
	if(f1->offt == f2->offt && f1->siz == f2->siz)
		return(0);
	if(f1->siz != f2->siz)
		return(f1->siz - f2->siz);
	return(f1->offt - f2->offt);
}




/*
element name comparison funct. passed to b+tree routines
*/
static	int
elecompare(e1,l1,e2,l2)
char	*e1;
int	l1;
char	*e2;
int	l2;
{
	register char	*p1;
	register char	*p2;

	if(ELENUM(e1) != ELENUM(e2))
		return(ELENUM(e1) - ELENUM(e2));
	p1 = ELENAM(e1);
	p2 = ELENAM(e2);

	for(;*p1 == *p2 && *p1 != '\0' && *p2 != '\0'; p1++, p2++)
		;

	if(*p1 != *p2)
		return(*p1 - *p2);
	return(0);
}




/*
open a ChunkFile file and return a pointer to it
*/
static	ChunkFile	*
chunkopen(file)
char	*file;
{
	ChunkFile	*ret;
	int		nred;

	if((ret = (ChunkFile *)malloc(sizeof(ChunkFile))) == 0)
		return(0);
	if((ret->fd = open(file,O_RDWR|O_CREAT,0600)) < 0) {
		free((char *)ret);
		return(0);
	}
	nred = read(ret->fd,(char *)&(ret->sblk),sizeof(ret->sblk));
	if(nred == 0) {
		/* empty - initialize */
		ret->sblk.free = 0L;	/* presently unused */
		ret->sblk.high = (long)sizeof(ret->sblk);
		if((lseek(ret->fd,0L,0) != 0L)) {
			(void)close(ret->fd);
			free(ret);
			return(0);
		}
		nred = write(ret->fd,(char *)&ret->sblk,sizeof(ret->sblk));
	}
	if(nred != sizeof(ret->sblk)) {
		(void)close(ret->fd);
		free(ret);
		return(0);
	}


	/* open free block index */
	ret->fbt = bt_optopen(
		BT_PATH,	"free_index",
		BT_OMODE,	O_CREAT,
		BT_DTYPE,	BT_USRDEF,	freecompare,
	0);

	if(ret->fbt == (BT_INDEX *)0) {
		(void)close(ret->fd);
		free(ret);
		return(0);
	}

	return(ret);
}




/*
close a chunk file
*/
static	void
chunkclose(cf)
ChunkFile	*cf;
{
	if(cf == (ChunkFile *)0)
		return;

	(void)close(cf->fd);
	(void)bt_close(cf->fbt);
	free((char *)cf);
}




/*
open an ObjIno file and return a pointer to it
*/
static	InoFile	*
inoopen(file)
char	*file;
{
	InoFile	*ret;
	int	nred;

	if((ret = (InoFile *)malloc(sizeof(InoFile))) == 0)
		return(0);
	if((ret->fd = open(file,O_RDWR|O_CREAT,0600)) < 0) {
		free((char *)ret);
		return(0);
	}
	nred = read(ret->fd,(char *)&(ret->sblk),sizeof(ret->sblk));
	if(nred == 0) {
		/* empty - initialize */
		ret->sblk.free = 0L;
		ret->sblk.high = 1L;
		if((lseek(ret->fd,0L,0) != 0L)) {
			(void)close(ret->fd);
			free(ret);
			return(0);
		}
		nred = write(ret->fd,(char *)&ret->sblk,sizeof(ret->sblk));
	}
	if(nred != sizeof(ret->sblk)) {
		(void)close(ret->fd);
		free(ret);
		return(0);
	}
	return(ret);
}




/*
close an ObjIno file
*/
static	void
inoclose(ino)
InoFile	*ino;
{
	if(ino == (InoFile *)0)
		return;

	(void)close(ino->fd);
	free((char *)ino);
}




/*
read the requested ObjIno into buf
*/
static	int
inoread(inof,num,buf)
InoFile	*inof;
long	num;
ObjIno	*buf;
{
	long	o;
	o = (long)((num * sizeof(ObjIno)) + sizeof(inof->sblk));
	if(lseek(inof->fd,o,0) != o)
		return(1);
	if(read(inof->fd,(char *)buf,sizeof(ObjIno)) != sizeof(ObjIno))
		return(1);
	return(0);
}




/*
write the requested ObjIno from buf
*/
static	int
inowrite(inof,num,buf)
InoFile	*inof;
long	num;
ObjIno	*buf;
{
	long	o;
	o = (long)((num * sizeof(ObjIno)) + sizeof(inof->sblk));
	if(lseek(inof->fd,o,0) != o)
		return(1);
	if(write(inof->fd,(char *)buf,sizeof(ObjIno)) != sizeof(ObjIno))
		return(1);
	return(0);
}




/*
free an ObjIno
*/
static	int
inofree(nof,i)
InoFile	*nof;
long	i;
{
	ObjIno	ibuf;

	if(i <= 0 || i >= nof->sblk.high)
		return(1);

	if((inoread(nof,i,&ibuf)) != 0)
		return(1);

	ibuf.typ = TYP_NULL;
	ibuf.doff = nof->sblk.free;

	if((inowrite(nof,i,&ibuf)) != 0)
		return(1);

	nof->sblk.free = i;

	/* repair and re-write the header */
	if(lseek(nof->fd,0L,0) != 0L)
		return(1);
	if(write(nof->fd,(char *)&(nof->sblk),sizeof(nof->sblk)) !=
		sizeof(nof->sblk))
		return(1);
	return(0);
}




/*
allocate a new ObjIno and return its #
*/
static	long
inonew(nof)
InoFile	*nof;
{
	long	ret = (long)0;
	ObjIno	ibuf;

	if(nof->sblk.free == (long)0) {
		ret = (long)nof->sblk.high;
		nof->sblk.high++;
	} else {
		/* read in the Ino at the head of the free list */
		if((inoread(nof,nof->sblk.free,&ibuf)) != 0)
			return((long)0);

		/* real problems! */
		if(ibuf.typ != TYP_NULL) {
			logf("WARNING ! bad free list!!\n",0);
			ret = (long)nof->sblk.high;
			nof->sblk.high++;
		} else {
			/* that will be the one to return */
			ret = nof->sblk.free;
			nof->sblk.free = ibuf.doff;
		}
	}

	/* repair and re-write the header */
	if(lseek(nof->fd,0L,0) != 0L)
		return((long)0);
	if(write(nof->fd,(char *)&(nof->sblk),sizeof(nof->sblk)) !=
		sizeof(nof->sblk))
		return((long)0);
	return(ret);
}




/*
really allocate a chunk from a chunk file. only called from chunknew
*/
static	long
chunkallocate(cof,byts,realsiz)
ChunkFile	*cof;
unsigned	byts;
unsigned	*realsiz;
{
	long	ret = (long)0;

	ret = (long)cof->sblk.high;
	cof->sblk.high += byts;
#ifdef	CHUNKDEBUG
	printf("chunkallocate: %d bytes at %d, new high %d\n",byts,ret,cof->sblk.high);
#endif
	*realsiz = byts;

	/* repair and re-write the header */
	if(lseek(cof->fd,0L,0) != 0L)
		return((long)0);
	if(write(cof->fd,(char *)&(cof->sblk),sizeof(cof->sblk)) !=
		sizeof(cof->sblk))
		return((long)0);
	return(ret);
}




/*
allocate a new chunk and return its offset, after first checking for
a reasonable-sized free hunk in the free list. (the free list is
maintained in a b+tree, because it was easy, and the ordering
properties of the tree permit an interesting best-fit approach)
*/
static	long
chunknew(cof,byts,realsiz)
ChunkFile	*cof;
unsigned	byts;
unsigned	*realsiz;
{
	FreeEnt	fe;
	int	btret;
	long	bf;
	int	jnk;

	fe.offt = (long)0;
	fe.siz = byts;

	/*
	scan tree. we had BETTER not find anything, since the offset we
	are looking for is at 0, and HAS to be less than anything of
	comparable size in the tree.
	*/
	btret = bt_find(cof->fbt,(char *)&fe,sizeof(fe),&bf);
	if(btret != BT_NF)
		return(chunkallocate(cof,byts,realsiz));

	/*
	get the next entry, which, by definition, will be the best fit,
	since the size is the next highest. anything else (such as BT_ERR
	or BT_EOF) as a return value means we are out of luck.
	*/
	btret = bt_traverse(cof->fbt,BT_EOF,(char *)&fe,sizeof(fe),&jnk,&bf);
	if(btret != BT_OK)
		return(chunkallocate(cof,byts,realsiz));

#ifdef	CHUNKDEBUG
	printf("chunknew: %d bytes at %d in free tree\n",fe.siz,fe.offt);
#endif

	/*
	now, fe should have a nicely matching chunk of free disk.
	make sure there is not too much slop - to avoid fragging.
	*/
	if(fe.siz - byts > STORESLOP)
		return(chunkallocate(cof,byts,realsiz));

	/*
	looks like we have a decent fit, now, so delete fe (it HAD
	BETTER BE THERE!) from the tree, and patch up a return
	value.
	*/
	if(bt_delete(cof->fbt,(char *)&fe,sizeof(fe)) != BT_OK)
		return(chunkallocate(cof,byts,realsiz));

	/* fe's chunk may be bigger, remember */
	*realsiz = fe.siz;
#ifdef	CHUNKDEBUG
	printf("chunknew: returning %d bytes(really %d) at %d\n",byts,fe.siz,fe.offt);
#endif
	return(fe.offt);
}




/*
free a chunk, by inserting it in the b+tree free list. if it doesn't
work, it doesn't work.
*/
static	void
chunkfree(cof,off,byts)
ChunkFile	*cof;
long		off;
unsigned	byts;
{
	FreeEnt	fe;

	fe.offt = off;
	fe.siz = byts;
	(void)bt_insert(cof->fbt,(char *)&fe,sizeof(fe),off,1);
#ifdef	CHUNKDEBUG
	printf("chunkfree: free %d bytes at %d\n",byts,off);
#endif
}




/*
write a list header and its data separately (we don't know that it is
contiguous in memory) when it is re-materialized it will be.
*/
static	int
freezelist(cof,off,l)
ChunkFile	*cof;
long		off;
ObjList		*l;
{

	if(lseek(cof->fd,off,0) != off)
		return(1);

	/* write the header */
	if(write(cof->fd,(char *)l,sizeof(ObjList)) != sizeof(ObjList))
		return(1);
#ifdef	FREEZEDEBUG
	printf("freeze:wrote list header\n");
#endif

	/* write the data */
	if(write(cof->fd,(char *)l->l_data,(int)(sizeof(long) * l->l_cnt))
		!= (int)(sizeof(long) * l->l_cnt)) 
		return(1);
#ifdef	FREEZEDEBUG
	printf("freeze:wrote list data\n");
#endif
	return(0);
}




/*
write a function to disk.
*/
static	int
freezeprog(cof,off,p)
ChunkFile	*cof;
long		off;
Prog		*p;
{

	if(lseek(cof->fd,off,0) != off)
		return(1);

	/* unsetuid bit when copying functs, just in case */
	p->p_mode = (p->p_mode & ~(PERM_SUID));

	/* write the header */
	if(write(cof->fd,(char *)p,sizeof(Prog)) != sizeof(Prog))
		return(1);
#ifdef	FREEZEDEBUG
	printf("freeze:wrote func header\n");
#endif

	/* write the executable segment */
	if(write(cof->fd,(char *)p->p_mem,(int)sizeof(int) * p->p_siz)
		!= (int)sizeof(int) * p->p_siz) 
		return(1);
#ifdef	FREEZEDEBUG
	printf("freeze:wrote func exec. code (%d)\n",sizeof(int) * p->p_siz);
#endif

	/* write the string table */
	if(write(cof->fd,(char *)p->p_str,p->s_siz) != p->s_siz) 
		return(1);
#ifdef	FREEZEDEBUG
	printf("freeze:wrote func str. table (%d)\n",p->s_siz);
#endif

	return(0);
}




/*
open all the databases used
*/
db_open()
{
	CacheChain	*cp;
	CacheElem	*ep;


	/* open i-block file */
	if((sys_ino = inoopen("inodes")) == (InoFile *)0) {
		fatal("cannot open inodes: ",(char *)-1,"\n",0);
		return(1);
	}


	/* open chunk store */
	if((sys_chunk = chunkopen("chunks")) == (ChunkFile *)0) {
		fatal("cannot open chunks: ",(char *)-1,"\n",0);
		return(1);
	}


	/* open primary index with a BIG cache */
	sys_idx = bt_optopen(
		BT_PATH,	"index",
		BT_OMODE,	O_CREAT,
		BT_DTYPE,	BT_USRDEF,	elecompare,
		BT_CACHE,	12,
	0);
	if(sys_idx == (BT_INDEX *)0) {
		fatal("cannot open index: ",(char *)-1,"\n",0);
		return(1);
	}


	/* init cache - zero everything */
	for(cp = sys_cache; cp < sys_cache + CACHEWIDTH; cp++) {
		cp->ahead = (CacheElem *)0;
		cp->atail = (CacheElem *)0;

		cp->ohead = (CacheElem *)malloc(sizeof(CacheElem) * CACHEDEPTH);
		if(cp->ohead == (CacheElem *)0)
			return(1);

		for(ep = cp->ohead; ep < cp->ohead + CACHEDEPTH; ep++) {
			ep->next = (CacheElem *)0;
			if(ep != cp->ohead) {
				ep->prev = ep - 1;
				ep->prev->next = ep;
			} else
				ep->prev = (CacheElem *)0;
			ep->onum = (long)0;
			ep->ino.dsiz = 0;
		}
		cp->otail = cp->ohead + (CACHEDEPTH - 1);
		cp->ohead->prev = cp->otail->next = (CacheElem *)0;
	}
	return(0);
}




/*
close all the databases
*/
void
db_close()
{
	/*
	WE DO NOT FREE THE CACHE! why ? because we have no idea 
	where the pointers to the originally allocated memory are
	nowadays. fix this someday by allocating the whole cache
	in one call to malloc, and saving a pointer to it.
	*/
	if(sys_ino != (InoFile *)0)
		inoclose(sys_ino);
	if(sys_chunk != (ChunkFile *)0)
		chunkclose(sys_chunk);
	if(sys_idx != (BT_INDEX *)0)
		(void)bt_close(sys_idx);
}




/*
return the base object # of an element, or 0
*/
static	long
elegetbase(ele)
char	*ele;
{
	long	bob;

	switch(bt_find(sys_idx,ele,elelen(ele),&bob)) {
		case BT_NF:
			bob = (long)0;
			break;

		case BT_ERR:
			logf("elegetbase: btree error ",
			bt_errs[bt_errno(sys_idx)]," cannot find\n",0);
			bob = (long)0;
			break;
	}
	return(bob);
}




/*
allocate a new base object and make an index entry for it. actually,
index entries may already exist, but are overwritten by the new one.
this is correct behaviour.
*/
static	long
elenewbase(ele)
char	*ele;
{
	long	ret;

	/* allocate a new base object Ino */
	if((ret = inonew(sys_ino)) == (long)0)
		return(1);

	/*
	add the new entry to the index b+tree
	NOTE - the '1' flag implies an overwrite of existing entries.
	this means that existing base element pointers will be overwritten
	which is exactly what we want, of course, but if you modify
	this code, you have to be a bit careful of that.
	*/
	if(bt_insert(sys_idx,ele,elelen(ele),ret,1)== BT_ERR) {
		logf("elenewbase: btree error ",bt_errs[bt_errno(sys_idx)],
		" cannot insert\n",0);
		return(1);
	}
#ifdef	FREEZEDEBUG
	printf("freeze:add %d.%s as #%d in index\n",ELENUM(ele),ELENAM(ele),ret);
#endif
	return(ret);
}




/*
reset the cache - move all active objects back to the head of the old
list.
*/
void
cache_reset()
{
	CacheChain	*cp;

	for(cp = sys_cache; cp < sys_cache + CACHEWIDTH; cp++) {
		if(cp->ahead != 0) {
#ifdef	CACHEDEBUG
			printf("cache:reset chain %d\n",cp);
#endif
			cp->atail->next = cp->ohead;
#ifdef	CACHEDEBUG
			printf("cache:tail of active is %d\n",cp->atail->next);
#endif
			if(cp->ohead != (CacheElem *)0)
				cp->ohead->prev = cp->atail;
			cp->ohead = cp->ahead;
#ifdef	CACHEDEBUG
			printf("cache:new head of old is %d\n",cp->ohead);
#endif
			cp->ahead = (CacheElem *)0;
		}
	}
}





/*
remove an object from the old cache chain
*/
static	void
cache_dropold(cp,ep)
CacheChain	*cp;
CacheElem	*ep;
{
	/* is this the list tail ? */
	if(ep->next == (CacheElem *)0)
		cp->otail = ep->prev;
	else
		ep->next->prev = ep->prev;

	/* is this the list head ? */
	if(ep->prev == (CacheElem *)0)
		cp->ohead = ep->next;
	else
		ep->prev->next = ep->next;
#ifdef	CACHEDEBUG
	printf("cache:dechained %d from chain %d\n",ep,cp);
#endif
}




/*
insert an element at the head of the active list for its chain
*/
static	void
cache_insactiv(num,ep)
long		num;
CacheElem	*ep;
{
	CacheChain	*cp;

	cp = sys_cache + (num % CACHEWIDTH);
	ep->next = cp->ahead;
	if(cp->ahead == (CacheElem *)0)
		cp->atail = ep;
	cp->ahead = ep;
	ep->prev = (CacheElem *)0;
	if(ep->next != (CacheElem *)0)
		ep->next->prev = ep;
#ifdef	CACHEDEBUG
	printf("cache:%d now on active chain\n",ep);
#endif
}




/*
search the cache and return the thing. If it is NOT there,
return 0 - aflg is set iff the thing is in the currently
active cache.
*/
static	CacheElem	*
cache_search(num,aflg)
long	num;
int	*aflg;
{
	CacheChain	*cp;
	CacheElem	*ep;

	*aflg = 0;

	/* pointer math - you gotta LOVE it */
	cp = sys_cache + (num % CACHEWIDTH);
#ifdef	CACHEDEBUG
	printf("cache:searching cache holder %d\n",num % CACHEWIDTH);
#endif

	ep = cp->ahead;
#ifdef	CACHEDEBUG
	printf("cache:scan active chain %d\n",cp->ahead);
#endif
	/* first - search the active chain */
	while(ep != (CacheElem *)0) {
		if(num == ep->onum) {	/* hit ? quit! */
#ifdef	CACHEDEBUG
		printf("cache:return %d from active chain\n",ep);
#endif
			*aflg = 1;
			return(ep);
		}
		ep = ep->next;
	}

#ifdef	CACHEDEBUG
	printf("cache:scan old chain %d\n",cp->ahead);
#endif
	/* second - search the old chain */
	ep = cp->ohead;
	while(ep != (CacheElem *)0) {
		/*
		hits are trickier, here. we need to dechain it
		and re-chain it on the active list.
		*/
		if(num == ep->onum) {
			cache_dropold(cp,ep);
			cache_insactiv(num,ep);

			/* and return the thing */
#ifdef	CACHEDEBUG
			printf("cache:return %d from old chain\n",ep);
#endif
			return(ep);
		}
		ep = ep->next;
	}
	return((CacheElem *)0);
}




/*
get an empty holder from the cache chain needed.
*/
static	CacheElem	*
cache_getholder(num)
long	num;
{
	CacheChain	*cp;
	CacheElem	*ep;

	/* pointer math - you gotta LOVE it */
	cp = sys_cache + (num % CACHEWIDTH);
	if(cp->otail != (CacheElem *)0) {
		ep = cp->otail;
#ifdef	CACHEDEBUG
		printf("cache:flushing %d from chain\n",ep);
#endif
		cache_dropold(cp,cp->otail);

		/* if there was cached data, free it */
		if(ep->ino.dsiz != 0) {
#ifdef	ALLOCDEBUG
			printf("free %d bytes at %d\n",ep->ino.dsiz,ep->ino.op.c);
#endif
			free(ep->ino.op.c);
		}
	} else {
		ep = (CacheElem *)malloc(sizeof(CacheElem));
		if(ep == (CacheElem *)0)
			return((CacheElem *)0);
#ifdef	CACHEDEBUG
		printf("cache:out of holders in chain! allocated %d\n",ep);
#endif
		ep->next = ep->prev = (CacheElem *)0;
	}

	ep->onum = (long)0;
	ep->ino.op.c = (char *)0;
	return(ep);
}




/*
search the cache and return the materialized thing. if it is NOT in
the cache, materialize it quietly.
*/
static	ObjIno	*
cache_get(num)
long	num;
{
	CacheElem	*ep;
	int		aflg;

	/* if the thing is already in the cache, life is good and pure. */
	if((ep = cache_search(num,&aflg)) != (CacheElem *)0)
		return(&(ep->ino));

	/* get an empty holder */
	if((ep = cache_getholder(num)) == (CacheElem *)0)
		return((ObjIno *)0);

	/* that done, activate it */
	cache_insactiv(num,ep);


	/* bring in the object's I-block */
	if(inoread(sys_ino,num,&(ep->ino)))
		return((ObjIno *)0);

#ifdef	CACHEDEBUG
	printf("cache:materialize %d from disk\n",num);
#endif

	/* allocate memory if dsize indicates there is some */
	if(ep->ino.dsiz != 0) {
		char	*p;
		int	rv;

		if((p = malloc(ep->ino.dsiz)) == (char *)0)
			return((ObjIno *)0);

#ifdef	ALLOCDEBUG
		printf("malloced %d bytes at %d\n",ep->ino.dsiz,p);
#endif
#ifdef	CACHEDEBUG
		printf("cache:read %d at %d\n",ep->ino.dsiz,ep->ino.doff);
#endif
		if(lseek(sys_chunk->fd,ep->ino.doff,0) != ep->ino.doff)
			return((ObjIno *)0);
		if((rv = read(sys_chunk->fd,p,(int)ep->ino.dsiz)) != (int)ep->ino.dsiz) {
			logf("cache_get: read@",ltoa(ep->ino.doff),
				" failed:",(char *)-1," got ",itoa(rv),"\n",0);
			logf("cache_get: inode #",ltoa(num),"\n",0);
			return((ObjIno *)0);
		}

		/* implicit cast between char * and whatever here! */
		ep->ino.op.c = p;

		/*
		some types may need some internal pointer frobbing
		because they contain absolute pointers. at this point
		we KNOW they are contiguous in memory. the rest is easy.
		*/
		if(ep->ino.typ == TYP_OLIST) {
			ep->ino.op.ol->l_data = (long *)&p[sizeof(ObjList)];
		} else
		if(ep->ino.typ == TYP_FUNC) {
			/* executable segment */
			ep->ino.op.p->p_mem = (int *)&p[sizeof(Prog)];
			/* string table */
			ep->ino.op.p->p_str = &p[sizeof(Prog) + (ep->ino.op.p->p_siz * sizeof(int))];
		}
	}

	/* now the object is valid and can be used. */
	ep->onum = num;

#ifdef	CACHEDEBUG
	printf("cache:return%d\n",ep);
#endif
	return(&(ep->ino));
}




/*
put the thing back into the cache, updating an in-memory copy if there
is any, freeing secondary storage, reassigning, etc, etc, etc.
changed flags that there has been a change to the secondary data.
If there has NOT been such a change, only the inode is updated.
If there HAS been, then a little more leg-work is necessary.
*/
static	int
cache_put(num,ino,changed,typ,op)
long	num;
ObjIno	*ino;
int	changed;
int	typ;
Oper	op;
{
	CacheElem	*ep;
	int		aflg;

	/* STAGE ONE: fix the in-core version */
	/* if the thing is already in the cache, we must needs update it. */
	if((ep = cache_search(num,&aflg)) != (CacheElem *)0) {
		Oper	oop;

		/* this shuts up lint. */
		oop.i = 0;

		/* free the old stuff if there is secondary core */
		if(changed && ep->ino.dsiz > 0) {
			if(aflg) {
				/*
				put the pointer on the free list to
				be freed after the run is over
				*/
#ifdef	ALLOCDEBUG
				printf("put %d bytes at %d on tmp free list for later removal\n",ep->ino.dsiz,ep->ino.op.c);
#endif
				tmpputonfree(ep->ino.op.c);
			} else {
#ifdef	ALLOCDEBUG
				printf("free %d bytes at %d\n",ep->ino.dsiz,ep->ino.op.c);
#endif
				free(ep->ino.op.c);
			}
		}

		/*
		update the cached inode, saving its data pointer in case.
		*/
		oop = ep->ino.op;

		if(ino != &(ep->ino))
			bcopy((char *)ino,(char *)&(ep->ino),sizeof(ObjIno));

		if(changed) {
			/*
			copy the new thing into the old. this may allocate
			memory as needed!
			*/
			if(opercopy(op,typ,&(ep->ino.op),&(ep->ino.typ),&(ep->ino.dsiz)))
				return(ERR_OOM);
		} else {
			/*
			restore the original pointer!
			*/
			ep->ino.op = oop;
		}

		/* somewhat of a kludge */
		if(typ == TYP_FUNC) {
			ep->ino.op.p->p_uid = ino->owner;
			ep->ino.op.p->p_mode = ino->mode;
		}
	}

	/* STAGE TWO: fix the on-disk version */
	/* chapter the first: if the secondary data has changed... */
	if(changed) {
		int	ns;

		ns = operdatasize(typ,op);

		if(ns == 0 && ino->msiz > 0) {
			/* no need to allocate any at all - free it */
			chunkfree(sys_chunk,ino->doff,ino->msiz);
		} else
		if(ns > 0 && (ino->msiz < ns || ino->msiz - ns > STORESLOP)) {
			long		noff;
			unsigned	rsiz;
			
			/* need to allocate more */
			noff = chunknew(sys_chunk,(unsigned)ns,&rsiz);
			if(noff == (long)0)
				return(ERR_DBASE);

			/* don't free it until the allocation works */
			if(ino->msiz > 0)
				chunkfree(sys_chunk,ino->doff,ino->msiz);

			ino->msiz = rsiz;
			ino->dsiz = ns;
			ino->doff = noff;
		}

		/*
		if the above 2 cases missed, there is enough room for
		any needed secondary storage already. Huzzah.
		do the store!
		*/
		if(typ == TYP_STR) {
			if(lseek(sys_chunk->fd,ino->doff,0) != ino->doff)
				return(ERR_DBASE);

			if(write(sys_chunk->fd,op.c,(int)ino->dsiz) != (int)ino->dsiz)
				return(ERR_DBASE);
		} else
		if(typ == TYP_OLIST) {
			if(freezelist(sys_chunk,ino->doff,op.ol))
				return(ERR_DBASE);
		} else
		if(typ == TYP_FUNC) {
			op.p->p_uid = ino->owner;
			op.p->p_mode = ino->mode;
			if(freezeprog(sys_chunk,ino->doff,op.p))
				return(ERR_DBASE);
		} else {
			/* fall through. just update the inode. */
			ino->op = op;
		}
		ino->typ = typ;
	}

	/* chapter the second: write out the inode... */
	return(inowrite(sys_ino,num,ino));
}




/*
this is brutal - MUST be a long, because of alignment - if it's a char,
the compiler may not word align it, and you are in a world of pain.
an alternate form would use bzero, I guess.
*/
static	char	*
eleroot(enm)
long	enm;
{
	static	long	xx[2];
	xx[0] = enm;
	*((char *)(&xx[1])) = '\0';
	return((char *)xx);
}




/*
check permissions on an object, by operator.
*/
permitted(op,owner,mode,uid,euid)
int	op;
long	owner;
int	mode;
long	uid;
long	euid;
{
	/* wiz */
	if(uid == (long)0 || euid == (long)0)
		return(1);

	/* world */
	if(op == PERM_WRITE && (mode & PERM_WWRIT))
		return(1);
	if(op == PERM_READ && (mode & PERM_WREAD))
		return(1);

	/* indirect */
	if(euid == owner && uid != euid) {
		if(op == PERM_WRITE && (mode & PERM_IWRIT))
			return(1);
		if(op == PERM_READ && (mode & PERM_IREAD))
			return(1);
	}

	/* owner */
	if(euid == owner && uid == euid) {
		if(op == PERM_WRITE && (mode & PERM_OWRIT))
			return(1);
		if(op == PERM_READ && (mode & PERM_OREAD))
			return(1);
	}
	return(0);
}




/*
add a reference count, and add it to the index
*/
static	int
eleaddref(bob,ele)
long	bob;
char	*ele;
{
	ObjIno	*ino;
	Oper	junk;

	/* increment link count */
	if((ino = cache_get(bob)) == (ObjIno *)0)
		return(ERR_DBASE);

	(ino->refcnt)++;

	junk.i = 0;
	if(cache_put(bob,ino,0,TYP_NULL,junk) != 0)
		return(ERR_DBASE);

	/* add an index entry */
	if(bt_insert(sys_idx,ele,elelen(ele),bob,1) == BT_ERR) {
		logf("eleaddref: btree error ",bt_errs[bt_errno(sys_idx)],
		" cannot insert\n",0);
		return(ERR_DBASE);
	}

#ifdef	FREEZEDEBUG
	printf("freeze:added ref %d.%s->%d, refcnt = %d\n",ELENUM(ele),ELENAM(ele),bob,ino->refcnt);
#endif
	return(0);
}




/*
decrement a reference count, and kill it from the index if appropriate
*/
static	int
eledropref(bob,ele)
long	bob;
char	*ele;
{
	ObjIno	*ino;

#ifdef	FREEZEDEBUG
	printf("eledropref: %d.%s base object %d\n",ELENUM(ele),ELENAM(ele),bob);
#endif
	/* first delete index entry */
	if(bt_delete(sys_idx,ele,elelen(ele)) == BT_ERR) {
		logf("eledropref: btree error ",bt_errs[bt_errno(sys_idx)],
		" cannot delete reference\n",0);
		return(ERR_DBASE);
	}

	/* decrement link count */
	if((ino = cache_get(bob)) == (ObjIno *)0)
		return(ERR_DBASE);

	(ino->refcnt)--;

	/* nothing linked to it anymore, free it completely */
	if(ino->refcnt <= 0) {
		CacheElem	*ep;
		int		aflg;
#ifdef	FREEZEDEBUG
		printf("free base object %d\n",bob);
#endif
		/* free any secondary storage */
		if(ino->dsiz > 0) {
			chunkfree(sys_chunk,ino->doff,ino->msiz);
			ino->dsiz = 0;
			ino->msiz = 0;
		}

		/* free the inode */
		if(inofree(sys_ino,bob))
			return(ERR_DBASE);

		/* make sure it is hors de combat */
		if((ep = cache_search(bob,&aflg)) != (CacheElem *)0)
			ep->onum = (long)0;
	} else {
		Oper	junk;

		junk.i = 0;
		if(cache_put(bob,ino,0,TYP_NULL,junk) != 0)
			return(ERR_DBASE);
	}

#ifdef	FREEZEDEBUG
	printf("freeze:decr ref %d.%s->%d, refcnt = %d\n",ELENUM(ele),ELENAM(ele),bob,ino->refcnt);
#endif
	return(0);
}




/*
chill a non-system var.
*/
disk_freeze(ele,typ,data,op,uid,euid)
char	*ele;
int	typ;
Oper	data;
int	op;
long	uid;
long	euid;
{
	ObjIno		i;
	ObjIno		*ci;
	ObjIno		*newip;
	long		bob;

#ifdef	FREEZEDEBUG
	printf("freeze:elem %d.%s\n",ELENUM(ele),ELENAM(ele));
#endif

	if(*ELENAM(ele) == '_' && uid != (long)0 && euid != (long)0)
		return(ERR_PERM);

	bob = elegetbase(ele);

	if(bob == (long)0) {
		/* if the assign is to NULL, we are done */
		if(typ == TYP_NULL)
			return(0);

		/*
		before permitting someone to add a variable to
		a base object, make sure they have write permission
		on it.
		*/
		if(uid != (long)0 && euid != (long)0) {
			if((bob = elegetbase(eleroot(ELENUM(ele)))) == (long)0) {
#ifdef	FREEZEDEBUG
				printf("freeze:no root obj %d\n",ELENUM(ele));
#endif
				return(ERR_NOTHERE);
			}

			if((ci = cache_get(bob)) == (ObjIno *)0)
				return(ERR_DBASE);

			/* check perms */
			if(!permitted(PERM_WRITE,ci->owner,ci->mode,uid,euid)) {
#ifdef	FREEZEDEBUG
				printf("freeze:perm create in %d denied\n",ELENUM(ele));
#endif
				return(ERR_NOTOWN);
			}
		}

		/* if the assign is to ELEM, add a ref, and the index */
		if(typ == TYP_ELEM) {
			long	txx;

			if((txx = elegetbase(data.c)) == (long)0) {
#ifdef	FREEZEDEBUG
				printf("freeze:link %d.%s:not found\n",ELENUM(ele),ELENAM(ele));
#endif
				return(ERR_NOTHERE);
			}

			return(eleaddref(txx,ele));
		}

		/* permission granted */
		if((bob = elenewbase(ele)) == (long)0)
			return(ERR_DBASE);

		/* initialize new object */
		i.mode = PERM_DEFAULT;
		i.owner = euid;
		i.refcnt = 1;
		i.dsiz = i.msiz = 0;
		newip = &i;
	} else {
		/* permissions checks - sigh - waste some time */
		if((newip = ci = cache_get(bob)) == (ObjIno *)0)
			return(ERR_DBASE);

		if(!permitted(PERM_WRITE,ci->owner,ci->mode,uid,euid)) {
#ifdef	FREEZEDEBUG
			printf("freeze:modify %d.%s denied\n",ELENUM(ele),ELENAM(ele));
#endif
			return(ERR_PERM);
		}


		/* if the assign is to NULL, drop a ref, and the index */
		if(typ == TYP_NULL)
			return(eledropref(bob,ele));

		/* adding a reference */
		if(typ == TYP_ELEM) {
			long	txx;

			if((txx = elegetbase(data.c)) == (long)0) {
#ifdef	FREEZEDEBUG
				printf("freeze:link %d.%s:not found\n",ELENUM(ele),ELENAM(ele));
#endif
				return(ERR_NOTHERE);
			}
			return(eleaddref(txx,ele));
		}

		/*
		this is in the case where the object exists, BUT 
		there is more than one reference to it. a reference
		must be dropped, and a new base object created, then
		the index modified.
		*/
		if(op == OP_EASGN) {
			if((ci = cache_get(bob)) == (ObjIno *)0)
				return(ERR_DBASE);

			if(ci->refcnt > 1) {
				if(eledropref(bob,ele))
					return(ERR_DBASE);
#ifdef	FREEZEDEBUG
	printf("freeze:decr ref %d.%s->%d, refcnt = %d\n",ELENUM(ele),ELENAM(ele),bob,ci->refcnt);
#endif

				/*
				allocate a new base object and modify index
				*/
				if((bob = elenewbase(ele)) == (long)0)
					return(ERR_DBASE);
				i.mode = PERM_DEFAULT;
				i.owner = euid;
				i.refcnt = 1;
				i.dsiz  = i.msiz = 0;
				newip = &i;
			}
		}
#ifdef	FREEZEDEBUG
		else {
	printf("freeze:assigning to *%d.%s->%d\n",ELENUM(ele),ELENAM(ele),bob);
		}
#endif
	}


#ifdef	FREEZEDEBUG
	printf("freeze:updated elem %d\n",bob);
#endif
	return(cache_put(bob,newip,1,typ,data));
}




/*
restore a non-system variable from disk.
*/
disk_thaw(ele,typ,data,uid,euid)
char	*ele;
int	*typ;
Oper	*data;
long	uid;
long	euid;
{
	ObjIno		*ci;
	long		bob;

	/* search the primary index for the object by name */
	switch(bt_find(sys_idx,ele,elelen(ele),&bob)) {

		case BT_NF:
#ifdef	THAWDEBUG
		printf("thaw:index empty for %d.%s\n",ELENUM(ele),ELENAM(ele));
#endif
		/* not there. oops */
		*typ = TYP_NULL;
		data->i = 0;
		return(ERR_NOTHERE);

		case BT_ERR:
		/* internal error - BAD news */
		logf("disk_thaw: btree error ",bt_errs[bt_errno(sys_idx)],
		" cannot find\n",0);
		return(ERR_DBASE);
	}

#ifdef	THAWDEBUG
	printf("thaw:index gives object #%d\n",bob);
#endif
	if((ci = cache_get(bob)) == (ObjIno *)0)
		return(ERR_DBASE);

	if(!permitted(PERM_READ,ci->owner,ci->mode,uid,euid)) {
#ifdef	THAWDEBUG
		printf("thaw:#%d mode %d,owner %d,perm denied\n",bob,ci->mode,ci->owner);
#endif
		return(ERR_PERM);
	}

	*typ = ci->typ;
	*data = ci->op;

	/* somewhat of a kludge */
	if(*typ == TYP_FUNC) {
		data->p->p_uid = ci->owner;
		data->p->p_mode = ci->mode;
	}

	return(0);
}




/*
this obtains an allocates a new base object holder, incrementing
the system's notion of how many have been allocated so far. the
system's idea is in a long, so when maxlong things have been
allocated, the database is in a world of hurtin.

we assume, further, that if there is NO #0.0, we should create one.
this will utterly annihilate the database, if something mangles #0.0,
as already existing base objects will get re-allocated. OW!
*/
void
blt_objectnew(m,argc,firstarg,result,uid,euid)
Machine	*m;
int	argc;
int	firstarg;
int	result;
long	*uid;
long	*euid;
{
	ObjIno	ibuf;
	long	bob;
	long	nin;
	int	errv;

	/*
	errv is used similarly in the other blt_ store functions.
	since all the errors in objectnew are disk-related, is is
	kind of redundant here, but is left in to ease eventual
	changes in code.
	*/

	/* this is absolutely awful, but necessary */
	switch(bt_find(sys_idx,eleroot((long)0),elelen(eleroot((long)0)),&bob)) {
		case BT_NF:
			bob = (long)0;
			break;

		case BT_ERR:
			logf("baseobjnew: btree error ",
			bt_errs[bt_errno(sys_idx)]," cannot insert\n",0);
			bob = (long)0;
			break;
	}

	/* now, increment it! (thus we will never hit 0!) */
	bob++;


	/* replace it in the index */
	if(bt_insert(sys_idx,eleroot((long)0),elelen(eleroot((long)0)),bob,1)== BT_ERR) {
		logf("baseobjnew: btree error ",bt_errs[bt_errno(sys_idx)],
		" cannot insert\n",0);
		errv = ERR_DBASE;
		goto fail;
	}

	/* allocate a new base object Ino */
	if((nin = inonew(sys_ino)) == (long)0) {
		errv = ERR_DBASE;
		goto fail;
	}

	ibuf.mode = PERM_DEFAULT;
	ibuf.owner = *euid;
	ibuf.refcnt = 1;
	ibuf.typ = TYP_BOBJHO;
	ibuf.dsiz = 0;
	ibuf.msiz = 0;
	ibuf.doff = (long)0;
	ibuf.op.l = (long)0;

	ibuf.refcnt = 1;

	/* initialize it and save it */
	if((inowrite(sys_ino,nin,&ibuf)) != 0) {
		errv = ERR_DBASE;
		goto fail;
	}

	/* make an entry in the index. */
	if(bt_insert(sys_idx,eleroot(bob),elelen(eleroot(bob)),nin,1)== BT_ERR) {
		logf("baseobjnew: btree error ",bt_errs[bt_errno(sys_idx)],
		" cannot insert\n",0);
		errv = ERR_DBASE;
		goto fail;
	}
#ifdef	FREEZEDEBUG
	printf("created new obj #%d, owner %d, mode %o\n",bob,*uid,ibuf.mode);
#endif

	m->m_mem[result].typ = TYP_OBJ;
	m->m_mem[result].oper.l = bob;
	return;

fail:
	m->m_mem[result].typ = TYP_NULL;
	m->m_mem[result].oper.i = errv;
	return;
}




/*
change the ownership of an object or base object. (some operator
overloading never hurt anyone)
*/
void
blt_chown(m,argc,firstarg,result,uid,euid)
Machine	*m;
int	argc;
int	firstarg;
int	result;
long	*uid;
long	*euid;
{
	ObjIno	*ip;
	SysSym	*sp;
	long	bob;
	int	errv;

	/* type check the cthoogies out of it */
	if(argc < 2 ||
		(m->m_mem[firstarg].typ != TYP_ELEM &&
		m->m_mem[firstarg].typ != TYP_OBJ) ||
		m->m_mem[firstarg + 1].typ != TYP_OBJ) {
#ifdef	FREEZEDEBUG
		printf("chown: bad args\n");
#endif
		errv = ERR_BADARG;
		goto fail;
	}

 
	if(m->m_mem[firstarg].typ == TYP_ELEM) {
		if(ELENUM(m->m_mem[firstarg].oper.c) == (long)0) {

			sp = sys_getsym(m->m_mem[firstarg].oper.c);
			if(sp == (SysSym *)0) {
				errv = ERR_NOTHERE;
				goto fail;
			}

			ip = &(sp->ino);
			bob = 0;
		} else {
			bob = elegetbase(m->m_mem[firstarg].oper.c);
			if(bob == (long)0) {
#ifdef	FREEZEDEBUG
				printf("chown: nonexistent object\n");
#endif
				errv = ERR_NOTHERE;
				goto fail;
			}

			if((ip = cache_get(bob)) == (ObjIno *)0) {
				errv = ERR_DBASE;
				goto fail;
			}
		}
	} else {
		
		bob = elegetbase(eleroot(m->m_mem[firstarg].oper.l));
		if(bob == (long)0) {
#ifdef	FREEZEDEBUG
			printf("chown: nonexistent object\n");
#endif
			errv = ERR_NOTHERE;
			goto fail;
		}

		if((ip = cache_get(bob)) == (ObjIno *)0) {
			errv = ERR_DBASE;
			goto fail;
		}
	}

	/* check perms */
	if(*uid != (long)0 && *euid != (long)0 && ip->owner != *euid) {
#ifdef	FREEZEDEBUG
		printf("chown: permission denied\n");
#endif
		errv = ERR_NOTOWN;
		goto fail;
	}

	/* really f***ing ugly. - MUST patch it in the func */
	if(ip->typ == TYP_FUNC) {
		ip->op.p->p_uid = ip->owner;
		ip->op.p->p_mode = ip->mode;
	}

	/* ok, do it */
	ip->owner = m->m_mem[firstarg + 1].oper.l;
	ip->mode = (ip->mode & ~(PERM_SUID));

	if(m->m_mem[firstarg].typ != TYP_ELEM || ELENUM(m->m_mem[firstarg].oper.c) != (long)0) {
		Oper	junk;

		junk.i = 0;
		if(cache_put(bob,ip,0,TYP_NULL,junk)) {
			errv = ERR_DBASE;
			goto fail;
		}
	}

	m->m_mem[result].oper.i = 0;
	m->m_mem[result].typ = TYP_NUM;
#ifdef	FREEZEDEBUG
	printf("chown: base object %d belongs to %d\n",bob,ip->owner);
#endif
	return;

fail:
	m->m_mem[result].typ = TYP_NULL;
	m->m_mem[result].oper.i = errv;
	return;
}




/*
change the permissions bits of an object.
*/
void
blt_chmod(m,argc,firstarg,result,uid,euid)
Machine	*m;
int	argc;
int	firstarg;
int	result;
long	*uid;
long	*euid;
{
	ObjIno	*ip;
	SysSym	*sp;
	int	errv;
	long	bob;
	char	*mp;
	int	nm = 0;

	/* type check the cthoogies out of it */
	if(argc < 2 ||
		(m->m_mem[firstarg].typ != TYP_ELEM &&
		m->m_mem[firstarg].typ != TYP_OBJ) ||
		m->m_mem[firstarg + 1].typ != TYP_STR) {
#ifdef	FREEZEDEBUG
		printf("chmod: bad argument\n");
#endif
		errv = ERR_BADARG;
		goto fail;
	}

 
	if(m->m_mem[firstarg].typ == TYP_ELEM) {
		if(ELENUM(m->m_mem[firstarg].oper.c) == (long)0) {
			sp = sys_getsym(m->m_mem[firstarg].oper.c);
			if(sp == (SysSym *)0) {
				errv = ERR_NOTHERE;
				goto fail;
			}
			ip = &(sp->ino);
			bob = 0;
		} else {
			bob = elegetbase(m->m_mem[firstarg].oper.c);
			if(bob == (long)0) {
#ifdef	FREEZEDEBUG
				printf("chmod: no such object\n");
#endif
				errv = ERR_NOTHERE;
				goto fail;
			}

			if((ip = cache_get(bob)) == (ObjIno *)0) {
				errv = ERR_DBASE;
				goto fail;
			}
		}
	} else {

		bob = elegetbase(eleroot(m->m_mem[firstarg].oper.l));
		if(bob == (long)0) {
			errv = ERR_NOTHERE;
			goto fail;
		}
		if((ip = cache_get(bob)) == (ObjIno *)0) {
			errv = ERR_DBASE;
			goto fail;
		}
	}


	/* check perms */
	if(*uid != (long)0 && *euid != (long)0 && ip->owner != *euid && ip->owner != *uid) {
#ifdef	FREEZEDEBUG
		printf("chmod: base object %d perm denied\n",bob);
#endif
		errv = ERR_NOTOWN;
		goto fail;
	}

#ifdef	FREEZEDEBUG
	printf("chmod: base object %d mode is %o\n",bob,ip->mode);
#endif
	mp = m->m_mem[firstarg + 1].oper.c;

	while(mp && *mp) {
		switch(*mp++) {
		case	'O':
			if(*mp++ != ':')
				break;

			for(;*mp != '\0' && *mp != ' ' && *mp != '\t' && *mp != ','; mp++) {
				if(*mp == 'w')
					nm |= PERM_OWRIT;
				else
				if(*mp == 'r')
					nm |= PERM_OREAD;
			}
			break;

		case	'I':
			if(*mp++ != ':')
				break;

			for(;*mp != '\0' && *mp != ' ' && *mp != '\t' && *mp != ','; mp++) {
				if(*mp == 'w')
					nm |= PERM_IWRIT;
				else
				if(*mp == 'r')
					nm |= PERM_IREAD;
			}
			break;

		case	'W':
			if(*mp++ != ':')
				break;

			for(;*mp != '\0' && *mp != ' ' && *mp != '\t' && *mp != ','; mp++) {
				if(*mp == 'w')
					nm |= PERM_WWRIT;
				else
				if(*mp == 'r')
					nm |= PERM_WREAD;
				else
				if(*mp == 's')
					nm |= PERM_SUID;
				else
				if(*mp == 'c')
					nm |= PERM_CHAIN;
				else
				if(*mp == 'b')
					nm |= PERM_BLOCK;
			}
			break;
		}
	}

	ip->mode = nm;

	/* really f***ing ugly. - MUST patch it in the func */
	if(ip->typ == TYP_FUNC)
		ip->op.p->p_mode = nm;

	/* if disk-based, write it */
	if(m->m_mem[firstarg].typ != TYP_ELEM || ELENUM(m->m_mem[firstarg].oper.c) != (long)0) {
		Oper	junk;

		junk.i = 0;
		if(cache_put(bob,ip,0,TYP_NULL,junk)) {
			errv = ERR_DBASE;
			goto fail;
		}
	}

#ifdef	FREEZEDEBUG
	printf("chmod: base object %d new mode is %o\n",bob,nm);
#endif

	m->m_mem[result].oper.i = 0;
	m->m_mem[result].typ = TYP_NUM;
	return;

fail:
	m->m_mem[result].typ = TYP_NULL;
	m->m_mem[result].oper.i = errv;
	return;
}




/*
destroy a base object.
*/
void
blt_objectdestroy(m,argc,firstarg,result,uid,euid)
Machine	*m;
int	argc;
int	firstarg;
int	result;
long	*uid;
long	*euid;
{
	static	char	*idlbuf = (char *)0;
	int	errv;
	ObjIno	*ip;
	long	bob;
	long	jnk;
	CacheElem	*ep;
	int	aflg;

	if(idlbuf == (char *)0) {
		idlbuf = malloc(MAXIDENTLEN * 2);
		if(idlbuf == (char *)0) {
			logf("alloc idlbuf in objectdestroy failed !\n",0);
			errv = ERR_OOM;
			goto fail;
		}
	}

	/* type check the cthoogies out of it */
	if(argc != 1 || m->m_mem[firstarg].typ != TYP_OBJ) {
#ifdef	FREEZEDEBUG
		printf("objectdestroy: bad argument\n");
#endif
		errv = ERR_BADARG;
		goto fail;
	}

	if(m->m_mem[firstarg].oper.l == (long)0) {
#ifdef	FREEZEDEBUG
		printf("objectdestroy: some joker tried to destroy #0\n");
#endif
		errv = ERR_BADARG;
		goto fail;
	}

	bob = elegetbase(eleroot(m->m_mem[firstarg].oper.l));
	if(bob == (long)0) {
		errv = ERR_NOTHERE;
		goto fail;
	}

	if((ip = cache_get(bob)) == (ObjIno *)0) {
		errv = ERR_DBASE;
		goto fail;
	}

#define	ONLYWIZCANDESTROY
#ifdef	ONLYWIZCANDESTROY
	if(*uid != (long)0 && *euid != (long)0) {
		errv = ERR_PERM;
		goto fail;
	}
#else
	if(!permitted(PERM_WRITE,ip->owner,ip->mode,*uid,*euid)) {
#ifdef	THAWDEBUG
		printf("objectdestroy:#%d mode %d,owner %d,perm denied\n",
			bob,ip->mode,ip->owner);
#endif
		errv = ERR_PERM;
		goto fail;
	}
#endif

	bob = m->m_mem[firstarg].oper.l;

	/* prepare to attack with chainsaw! */
	if(bt_find(sys_idx,eleroot(bob),elelen(eleroot(bob)),&jnk) != BT_OK) {
#ifdef	THAWDEBUG
		printf("objectdestroy:#%d root not found\n",bob);
#endif
		errv = ERR_DBASE;
		goto fail;
	}

	/* first delete index entry of root object */
	if(bt_delete(sys_idx,eleroot(bob),elelen(eleroot(bob))) == BT_ERR) {
		logf("objectdestroy: btree error ",
		bt_errs[bt_errno(sys_idx)]," cannot delete\n",0);
		errv = ERR_DBASE;
		goto fail;
	}

	/* free root object's ino */
	if(inofree(sys_ino,jnk)) {
		errv = ERR_DBASE;
		goto fail;
	}

	if((ep = cache_search(jnk,&aflg)) != (CacheElem *)0)
		ep->onum = (long)0;

#ifdef	THAWDEBUG
	printf("objectdestroy:#%d root %d freed\n",bob,jnk);
#endif

	while(1) {
		int	x;
		int	bv;

		bv = bt_traverse(sys_idx,BT_EOF,idlbuf,MAXIDENTLEN * 2,&x,&jnk);
		if(bv == BT_ERR) {
			logf("b+tree error: objectdestroy traverse\n",0);
			errv = ERR_DBASE;
			goto fail;
		}
		if(bv != BT_OK)
			break;

		if(ELENUM(idlbuf) != bob)
			break;

#ifdef	FREEZEDEBUG
		printf("objectdestroy: elem %d.%s\n",ELENUM(idlbuf),ELENAM(idlbuf));
#endif
		if(eledropref(jnk,idlbuf)) {
			errv = ERR_DBASE;
			goto fail;
		}
	}

	m->m_mem[result].oper.i = 0;
	m->m_mem[result].typ = TYP_NUM;
	return;

fail:
	m->m_mem[result].typ = TYP_NULL;
	m->m_mem[result].oper.i = errv;
}




/*
list the elements in a base object
*/
void
blt_objectelements(m,argc,firstarg,result,uid,euid)
Machine	*m;
int	argc;
int	firstarg;
int	result;
long	*uid;
long	*euid;
{
	static	char	*idlbuf = (char *)0;
	int	errv;
	ObjIno	*ip;
	long	bob;
	long	jnk;

	if(idlbuf == (char *)0) {
		idlbuf = malloc(MAXIDENTLEN * 2);
		if(idlbuf == (char *)0) {
			logf("alloc idlbuf in objectelements failed !\n",0);
			errv = ERR_OOM;
			goto fail;
		}
	}

	if(argc != 1 || m->m_mem[firstarg].typ != TYP_OBJ) {
#ifdef	FREEZEDEBUG
		printf("objectelements: bad argument\n");
#endif
		errv = ERR_BADARG;
		goto fail;
	}

	if(m->m_mem[firstarg].oper.l == (long)0) {
#ifdef	FREEZEDEBUG
		printf("objectelements: some idiot tried to list #0\n");
#endif
		errv = ERR_BADARG;
		goto fail;
	}

	bob = elegetbase(eleroot(m->m_mem[firstarg].oper.l));
	if(bob == (long)0) {
		errv = ERR_NOTHERE;
		goto fail;
	}

	if((ip = cache_get(bob)) == (ObjIno *)0) {
		errv = ERR_DBASE;
		goto fail;
	}

	if(!permitted(PERM_READ,ip->owner,ip->mode,*uid,*euid)) {
#ifdef	THAWDEBUG
		printf("objectelements:#%d mode %d,owner %d,perm denied\n",
			bob,ip->mode,ip->owner);
#endif
		errv = ERR_PERM;
		goto fail;
	}

	bob = m->m_mem[firstarg].oper.l;
	if(bt_find(sys_idx,eleroot(bob),elelen(eleroot(bob)),&jnk) != BT_OK) {
#ifdef	THAWDEBUG
		printf("objectelements:#%d root not found\n",bob);
#endif
		errv = ERR_DBASE;
		goto fail;
	}

	iobtell(*uid,"elements in #",ltoa(bob),":",0);

	while(1) {
		int	x;
		int	bv;

		bv = bt_traverse(sys_idx,BT_EOF,idlbuf,MAXIDENTLEN * 2,&x,&jnk);
		if(bv == BT_ERR) {
			logf("b+tree error: objectdestroy traverse\n",0);
			errv = ERR_DBASE;
			goto fail;
		}

		if(bv != BT_OK)
			break;

		if(ELENUM(idlbuf) != bob)
			break;

#ifdef	FREEZEDEBUG
		printf("objectelement: elem %d.%s\n",ELENUM(idlbuf),ELENAM(idlbuf));
#endif
		iobtell(*uid," .",ELENAM(idlbuf),0);
	}
	iobtell(*uid,"\n",0);

	m->m_mem[result].oper.i = 0;
	m->m_mem[result].typ = TYP_NUM;
	return;

fail:
	m->m_mem[result].typ = TYP_NULL;
	m->m_mem[result].oper.i = errv;
}




/*
return the ownership of an object
*/
void
blt_objectowner(m,argc,firstarg,result,uid,euid)
Machine	*m;
int	argc;
int	firstarg;
int	result;
long	*uid;
long	*euid;
{
	ObjIno	*ip;
	SysSym	*sp;
	int	errv;
	long	bob;

	/* type check the cthoogies out of it */
	if(argc != 1 || (m->m_mem[firstarg].typ != TYP_ELEM &&
		m->m_mem[firstarg].typ != TYP_OBJ)) {
#ifdef	FREEZEDEBUG
		printf("objectowner: bad argument\n");
#endif
		errv = ERR_BADARG;
		goto fail;
	}

 
	if(m->m_mem[firstarg].typ == TYP_ELEM) {

		if(ELENUM(m->m_mem[firstarg].oper.c) == (long)0) {
			sp = sys_getsym(m->m_mem[firstarg].oper.c);
			if(sp == (SysSym *)0) {
				errv = ERR_NOTHERE;
				goto fail;
			}
			ip = &(sp->ino);
			bob = 0;
		} else {
			bob = elegetbase(m->m_mem[firstarg].oper.c);
			if(bob == (long)0) {
#ifdef	FREEZEDEBUG
				printf("objectowner: no such object\n");
#endif
				errv = ERR_NOTHERE;
				goto fail;
			}

			if((ip = cache_get(bob)) == (ObjIno *)0) {
				errv = ERR_DBASE;
				goto fail;
			}
		}
	} else {

		bob = elegetbase(eleroot(m->m_mem[firstarg].oper.l));
		if(bob == (long)0) {
			errv = ERR_NOTHERE;
			goto fail;
		}

		if((ip = cache_get(bob)) == (ObjIno *)0) {
			errv = ERR_DBASE;
			goto fail;
		}
	}


	m->m_mem[result].oper.l = ip->owner;
	m->m_mem[result].typ = TYP_OBJ;
#ifdef	FREEZEDEBUG
	printf("objectowner: base object %d belongs to %d\n",bob,ip->owner);
#endif
	return;

fail:
	m->m_mem[result].typ = TYP_NULL;
	m->m_mem[result].oper.i = errv;
	return;
}

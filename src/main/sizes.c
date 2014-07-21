#include	<sys/types.h>

#include	"btree.h"

#include	"ubermud.h"
#include	"store.h"
#include	"syssym.h"

main()
{
	/* from ubermud.h */
	printf("size of an ObjList header: %d\n",sizeof(ObjList));
	printf("size of a Prog header: %d\n",sizeof(Prog));
	printf("size of an Oper union: %d\n",sizeof(Oper));
	printf("size of a temporary symbol: %d\n",sizeof(Sym));
	printf("size of a stack component: %d\n",sizeof(MemOp));
	printf("size of a stack frame: %d\n",sizeof(Frame));
	printf("size of a machine header: %d\n",sizeof(Machine));
	printf("size of an object inode: %d\n",sizeof(ObjIno));

	/* from store.h */
	printf("size of an Inode file: %d\n",sizeof(InoFile));
	printf("size of a Chunk file: %d\n",sizeof(ChunkFile));
	printf("size of a CacheElem: %d\n",sizeof(CacheElem));
	printf("\t(there are %d of these allocated: total: %d)\n",
		CACHEWIDTH * CACHEDEPTH,
		CACHEWIDTH * CACHEDEPTH * sizeof(CacheElem));
	printf("\t(not counting dynamically allocated memory for the objects)\n");
	printf("size of a CacheChain: %d\n",sizeof(CacheChain));
	printf("\t(there are %d of these allocated: total: %d)\n",
		CACHEWIDTH, CACHEWIDTH * sizeof(CacheChain));


	/* from syssym.h */
	printf("size of a SysSym: %d\n",sizeof(SysSym));
	printf("\t(there are %d pointers to these allocated: total: %d)\n",
		SYSSYMWIDTH, CACHEWIDTH * sizeof(SysSym *));
}

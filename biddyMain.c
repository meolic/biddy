/***************************************************************************//*!
\file biddyMain.c
\brief File biddyMain.c contains main functions for representation and
manipulation of boolean functions with ROBDDs.

### Description

    PackageName [Biddy]
    Synopsis    [Biddy provides data structures and algorithms for the
                 representation and manipulation of Boolean functions with
                 ROBDDs. A hash table is used for quick search of nodes.
                 Complement edges decreases the number of nodes. An automatic
                 garbage collection with a formulae counter is implemented.
                 Variable swapping and sifting are implemented.]

    FileName    [biddyMain.c]
    Revision    [$Revision: 103 $]
    Date        [$Date: 2015-10-15 14:51:19 +0200 (čet, 15 okt 2015) $]
    Authors     [Robert Meolic (robert.meolic@um.si),
                 Ales Casar (ales@homemade.net)]

### Copyright

Copyright (C) 2006, 2015 UM-FERI, Smetanova ulica 17, SI-2000 Maribor, Slovenia

Biddy is free software; you can redistribute it and/or modify it under the terms
of the GNU General Public License as published by the Free Software Foundation;
either version 2 of the License, or (at your option) any later version.

Biddy is distributed in the hope that it will be useful, but WITHOUT ANY
WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A
PARTICULAR PURPOSE. See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License along with
this program; if not, write to the Free Software Foundation, Inc., 51 Franklin
Street, Fifth Floor, Boston, MA 02110-1301 USA.

### More info

See also: biddy.h, biddyInt.h

*******************************************************************************/

#include "biddyInt.h"

#ifdef UNIX
#include <unistd.h> /* used for sysconf(), UNIX only */
#include <sys/resource.h> /* used for setrlimit(), UNIX only */
#endif

#ifdef UNIXXX
#include <malloc.h> /* experimental */
#endif

/*----------------------------------------------------------------------------*/
/* Constant declarations                                                      */
/*----------------------------------------------------------------------------*/

/* ALL SIZES ARE unsigned int                                      */
/* Size of node and cache tables must be 2^N-1                     */
/* e.g. 2^10-1 = 1023, 2^16-1 = 65535, 2^20-1 = 1048575            */
/*      2^24-1 = 16777215                                          */

/* If BDD node is 24B (e.g. 32-bit GNU/Linux systems)              */
/* 256 nodes is 6 kB                                               */
/* 65536 nodes is 1.5 MB                                           */
/* 262144 nodes is 6 MB                                            */
/* 524288 nodes is 12 MB                                           */
/* 1048576 nodes is 24.0 MB                                        */
/* 2097152 nodes is 48.0 MB                                        */
/* 4194304 nodes is 96.0 MB                                        */
/* 8388608 nodes is 192.0 MB                                       */
/* 16777216 nodes is 384.0 MB                                      */

/* If BDD node is 48B (e.g. 64-bit GNU/Linux systems)              */
/* 256 nodes is 12 kB                                              */
/* 65536 nodes is 3.0 MB                                           */
/* 262144 nodes is 12 MB                                           */
/* 524288 nodes is 24 MB                                           */
/* 1048576 nodes is 48.0 MB                                        */
/* 2097152 nodes is 96.0 MB                                        */
/* 4194304 nodes is 192.0 MB                                       */
/* 8388608 nodes is 384.0 MB                                       */
/* 16777216 nodes is 768.0 MB                                      */

#define TINY_TABLE       1023
#define SMALL_TABLE     65535
#define BIG_TABLE     1048575
#define XBIG_TABLE    2097151
#define XXBIG_TABLE   4194303
#define XXXBIG_TABLE  8388607
#define HUGE_TABLE   16777215

/*----------------------------------------------------------------------------*/
/* Variable declarations                                                      */
/*----------------------------------------------------------------------------*/

/* EXPORTED VARIABLES */

#ifdef __cplusplus
extern "C" {
#endif

#ifdef __cplusplus
}
#endif

/* INTERNAL VARIABLES */

Biddy_Manager biddyAnonymousManager /* anonymous manager */
       = NULL;

BiddyLocalInfo *biddyLocalInfo /* local info table */
       = NULL;

/*----------------------------------------------------------------------------*/
/* Static function prototypes                                                 */
/*----------------------------------------------------------------------------*/

static unsigned int nodeTableHash(Biddy_Variable v, Biddy_Edge pf, Biddy_Edge pt, unsigned int biddyNodeTableSize);

static unsigned int ITEHash(Biddy_Edge a, Biddy_Edge b, Biddy_Edge c, unsigned int biddyIteCacheSize);

static void exchangeEdges(Biddy_Edge *f, Biddy_Edge *g);

static void complExchangeEdges(Biddy_Edge *f, Biddy_Edge *g);

static void addNodeTable(Biddy_Manager MNG, unsigned int hash, BiddyNode *node, BiddyNode *sup1);
             
static BiddyNode *findNodeTable(Biddy_Manager MNG, Biddy_Variable v, Biddy_Edge pf, Biddy_Edge pt, BiddyNode **thesup);

static void addITECache(Biddy_Manager MNG, Biddy_Edge a, Biddy_Edge b, Biddy_Edge c, Biddy_Edge r);

static Biddy_Boolean findITECache(Biddy_Manager MNG, Biddy_Edge a, Biddy_Edge b, Biddy_Edge c, Biddy_Edge *r);

static void ITEGarbage(Biddy_Manager MNG);

static void addEACache(Biddy_Manager MNG, Biddy_Edge f, Biddy_Variable v, Biddy_Edge r);

static Biddy_Boolean findEACache(Biddy_Manager MNG, Biddy_Edge f, Biddy_Variable v, Biddy_Edge *r);

static void EAGarbage(Biddy_Manager MNG);

static void addRCCache(Biddy_Manager MNG, Biddy_Edge f, Biddy_Edge g, Biddy_Variable v, Biddy_Edge r);

static Biddy_Boolean findRCCache(Biddy_Manager MNG, Biddy_Edge f, Biddy_Edge g, Biddy_Variable v, Biddy_Edge *r);

static void RCGarbage(Biddy_Manager MNG);

static Biddy_Variable swapWithHigher(Biddy_Manager MNG, Biddy_Variable v);

static Biddy_Variable swapWithLower(Biddy_Manager MNG, Biddy_Variable v);

static void swapVariables(Biddy_Manager MNG, Biddy_Variable low, Biddy_Variable high);

static void nullOrdering(BiddyOrderingTable table);

static void nodeNumberOrdering(Biddy_Edge f, unsigned int *i, BiddyOrderingTable ordering);

static void warshall(BiddyOrderingTable table, Biddy_Variable varnum);

static void writeBDD(Biddy_Manager MNG, Biddy_Edge f);

static void writeORDERING(Biddy_Manager MNG, BiddyOrderingTable table, Biddy_Variable varnum);

static void writeORDER(Biddy_Manager MNG);

static BiddyLocalInfo * createLocalInfo(Biddy_Edge f, BiddyLocalInfo *c);

static void deleteLocalInfo(Biddy_Edge f);

/*----------------------------------------------------------------------------*/
/* Definition of exported functions                                           */
/*----------------------------------------------------------------------------*/

/***************************************************************************//*!
\brief Function Biddy_Init initialize BDD package.

### Description
    Biddy_Init creates anonymous manager which consists of creating
    node table (biddyNodeTable), variable table (biddyVariableTable),
    formula table (biddyFormulaTable), three basic caches
    (biddyIteCache, biddyEACache and biddyRCCache), and cache list
    (biddyCacheList).
    Biddy_Init also initializes constant nodes (biddyOne, biddyZero),
    memory management and automatic garbage collection.
### Side effects
    Allocate a lot of memory.
### More info
*******************************************************************************/

#ifdef __cplusplus
extern "C" {
#endif

void
Biddy_Init()
{
  int i,j;
  Biddy_Manager MNG;

#ifdef UNIX
  struct rlimit rl;
#endif

#ifdef UNIX
  /* printf("The page size for this system is %ld bytes.\n", sysconf(_SC_PAGESIZE)); */
  rl.rlim_max  = rl.rlim_cur = RLIM_INFINITY;
  if (setrlimit(RLIMIT_STACK, &rl)) {
    fprintf(stderr,"ERROR: setrlimit\n");
    perror("setrlimit");
    exit(1);
  }
#endif

#ifdef UNIXXX
  mallopt(M_MMAP_MAX,0);  
  mallopt(M_TRIM_THRESHOLD,-1);
#endif

  /* CREATE ANONYMOUS MANAGER */
  if (!(MNG = (Biddy_Manager)
        malloc(sizeof(BiddyManager)))) {
    fprintf(stderr,"Biddy_Init (Anonymous manager): Out of memoy!\n");
    exit(1);
  }
  MNG[0] = strdup("anonymous"); /* biddyManagerName */
  MNG[1] = strdup("ROBDD WITH COMPLEMENT EDGES"); /* biddyManagerType */
  MNG[2] = NULL; /* biddyZero */
  MNG[3] = NULL; /* biddyOne */
  if (!(MNG[4] = (BiddyNodeTable *)
        malloc(sizeof(BiddyNodeTable)))) {
    fprintf(stderr,"Biddy_Init (Anonymous manager): Out of memoy!\n");
    exit(1);
  }
  biddyNodeTable.table = NULL;
  biddyNodeTable.size = HUGE_TABLE;
  biddyNodeTable.blocktable = NULL;
  biddyNodeTable.foa = 0;
  biddyNodeTable.compare = 0;
  biddyNodeTable.add = 0;
  biddyNodeTable.max = 0;
  biddyNodeTable.num = 0;
  biddyNodeTable.numf = 0;
  biddyNodeTable.garbage = 0;
  biddyNodeTable.generated = 0;
  biddyNodeTable.blocknumber = 0;
  biddyNodeTable.swap = 0;
  biddyNodeTable.sifting = 0;
  if (!(MNG[5] = (BiddyVariableTable *)
        malloc(sizeof(BiddyVariableTable)))) {
    fprintf(stderr,"Biddy_Init (Anonymous manager): Out of memoy!\n");
    exit(1);
  }
  biddyVariableTable.table = NULL;
  biddyVariableTable.size = BIDDYVARMAX;
  biddyVariableTable.num = 0;
  biddyVariableTable.numnum = 0;
  if (!(MNG[6] = (BiddyFormulaTable *)
        malloc(sizeof(BiddyFormulaTable)))) {
    fprintf(stderr,"Biddy_Init (Anonymous manager): Out of memoy!\n");
    exit(1);
  }
  biddyFormulaTable.table = NULL;
  biddyFormulaTable.size = 0;
  biddyFormulaTable.numOrdered = 0;
  biddyFormulaTable.deletedName = NULL;
  if (!(MNG[7] = (BiddyOp3CacheTable *)
        malloc(sizeof(BiddyOp3CacheTable)))) {
    fprintf(stderr,"Biddy_Init (Anonymous manager): Out of memoy!\n");
    exit(1);
  }
  biddyIteCache.table = NULL;
  biddyIteCache.size = BIG_TABLE;
  biddyIteCache.search = 0;
  biddyIteCache.find = 0;
  biddyIteCache.overwrite = 0;
  if (!(MNG[8] = (BiddyEACacheTable *)
        malloc(sizeof(BiddyEACacheTable)))) {
    fprintf(stderr,"Biddy_Init (Anonymous manager): Out of memoy!\n");
    exit(1);
  }
  biddyEACache.table = NULL;
  biddyEACache.size = SMALL_TABLE;
  biddyEACache.search = 0;
  biddyEACache.find = 0;
  biddyEACache.overwrite = 0;
  if (!(MNG[9] = (BiddyRCCacheTable *)
        malloc(sizeof(BiddyRCCacheTable)))) {
    fprintf(stderr,"Biddy_Init (Anonymous manager): Out of memoy!\n");
    exit(1);
  }
  biddyRCCache.table = NULL;
  biddyRCCache.size = SMALL_TABLE;
  biddyRCCache.search = 0;
  biddyRCCache.find = 0;
  biddyRCCache.overwrite = 0;
  if (!(MNG[10] = (BiddyCacheList* *)
        malloc(sizeof(BiddyCacheList*)))) {
    fprintf(stderr,"Biddy_Init (Anonymous manager): Out of memoy!\n");
    exit(1);
  }
  biddyCacheList = NULL;
  if (!(MNG[11] = (unsigned int *)
        malloc(sizeof(unsigned int)))) {
    fprintf(stderr,"Biddy_Init (Anonymous manager): Out of memoy!\n");
    exit(1);
  }
  biddyBlockSize = 0;
  if (!(MNG[12] = (BiddyNode* *)
        malloc(sizeof(BiddyNode*)))) {
    fprintf(stderr,"Biddy_Init (Anonymous manager): Out of memoy!\n");
    exit(1);
  }
  biddyFreeNodes = NULL;
  if (!(MNG[13] = (BiddyOrderingTable *)
        malloc(sizeof(BiddyOrderingTable)))) {
    fprintf(stderr,"Biddy_Init (Anonymous manager): Out of memoy!\n");
    exit(1);
  }
  if (!(MNG[14] = (int *)
        malloc(sizeof(int)))) {
    fprintf(stderr,"Biddy_Init (Anonymous manager): Out of memoy!\n");
    exit(1);
  }
  biddyCount = 0;
  biddyAnonymousManager = MNG;

  /* CREATE AND INITIALIZE NODE TABLE IN ANONYMOUS MANAGER */
  /* THE SIZE OF NODE TABLE IS biddyNodeTable.size+2 */
  /* BECAUSE OF USED TRICKS, HASH FUNCTION MUST NEVER RETURN ZERO! */
  /* USEFUL INDICES ARE FROM [1] TO [biddyNodeTable.size+1] */
  if (!(biddyNodeTable.table = (BiddyNode **)
        malloc((biddyNodeTable.size+2) * sizeof(BiddyNode *)))) {
    fprintf(stderr,"Biddy_Init (Node table): Out of memoy!\n");
    exit(1);
  }
  for (i=0;i<biddyNodeTable.size+2;i++) {
    biddyNodeTable.table[i] = NULL;
  }

  /* CREATE AND INITIALIZE VARIABLE TABLE IN ANONYMOUS MANAGER */
  /* VARIABLES ARE ORDERED BY INDEX IN THE VARIABLE TABLE */
  /* IN THE BDD, SMALLER VARIABLES ARE ABOVE THE GREATER ONES */
  /* VARIABLE "1" HAS INDEX [0] */
  /* VARIABLE "1" MUST ALWAYS HAVE MAX ORDER (IT IS BOTTOMMOST) */
  /* THUS WE HAVE: [1] < [2] < ... < [0] */
  biddyVariableTable.table = (BiddyVariable *)
                malloc(biddyVariableTable.size * sizeof(BiddyVariable));
  nullOrdering(biddyOrderingTable);
  biddyVariableTable.table[0].name = strdup("1");
  biddyVariableTable.table[0].num = 1;
  for (i=1; i<biddyVariableTable.size; i++) {
    biddyVariableTable.table[i].num = 0;
    biddyVariableTable.table[i].name = NULL;
    biddyVariableTable.table[i].selected = FALSE;
    SET_ORDER(biddyOrderingTable,i,0);
    for (j=i+1; j<biddyVariableTable.size; j++) {
      SET_ORDER(biddyOrderingTable,i,j);
    }
  }
  biddyVariableTable.num = 1; /* ONLY VARIABLE '1' IS CREATED */
  biddyVariableTable.numnum = 1; /* VARIABLE '1' IS NUMBERED VARIABLE */

  /* DEBUGGING */
  /*
  writeORDER(MNG);
  */

  /* DEBUGGING */
  /*
  warshall(biddyOrderingTable,Biddy_Managed_VariableTableNum(MNG));
  writeORDER(MNG);
  */

  /* INITIALIZATION OF MEMORY MANAGEMENT IN ANONYMOUS MANAGER */
  /* ALLOCATE FIRST CHUNK OF NODES */
  biddyBlockSize = 524288; /* this should be multiples of pagesize */
  if (!(biddyFreeNodes = (BiddyNode *)
        malloc((biddyBlockSize) * sizeof(BiddyNode)))) {
    fprintf(stderr,"Biddy_Init (Nodes): Out of memoy!\n");
    exit(1);
  }
   
  /* EXPERIMENTAL */
  /*
  {
  uintptr_t commonbits;
  unsigned int numcb=0;
  commonbits = (uintptr_t) biddyFreeNodes ^ (uintptr_t) (&biddyFreeNodes[biddyBlockSize-1]);
  while (commonbits) {
    numcb++;
    commonbits = commonbits >> 1;
  }
  fprintf(stderr,"Biddy_Init: Nodes in the inital memory block have addresses with common %u upper bits,\n",numcb);
  }
  */
      
  biddyNodeTable.blocktable = (BiddyNode **) malloc(sizeof(BiddyNode *));
  biddyNodeTable.blocktable[0] = biddyFreeNodes;
  biddyNodeTable.table[0] = biddyFreeNodes;
  biddyFreeNodes = &biddyFreeNodes[1];
  for (i=0; i<biddyBlockSize-2; i++) {
    biddyFreeNodes[i].list = (void *) &biddyFreeNodes[i+1];
  }
  biddyFreeNodes[biddyBlockSize-2].list = NULL;

  /* MAKE CONSTANT NODE "1" AND EDGES TO CONSTANT NODE IN IN ANONYMOUS MANAGER */
  /* SINCE BIDDY V1.2: CONSTANT NODE "1" IS AT INDEX [0] */
  biddyNodeTable.table[0]->prev = NULL;
  biddyNodeTable.table[0]->next = NULL;
  biddyNodeTable.table[0]->list = NULL;
  biddyNodeTable.table[0]->f = biddyNull;
  biddyNodeTable.table[0]->t = biddyNull;
  biddyNodeTable.table[0]->count = 0; /* constant node is fortified */
  biddyNodeTable.table[0]->v = 0;
  MNG[3] = (void *) biddyNodeTable.table[0]; /* biddyOne */
  MNG[2] = (void *) ((uintptr_t) biddyOne | (uintptr_t) 1); /* biddyZero */

  /* INITIALIZATION OF VARIABLE'S VALUES */
  biddyVariableTable.table[0].value = biddyOne;
  for (i=1; i<biddyVariableTable.size; i++) {
    biddyVariableTable.table[i].value = biddyZero;
  }

  /* INITIALIZE FORMULA TABLE IN ANONYMOUS MANAGER */
  biddyFormulaTable.deletedName = strdup("BIDDY_DELETED_FORMULAE");
  biddyFormulaTable.size = 2;
  biddyFormulaTable.numOrdered = 2;
  if (!(biddyFormulaTable.table = (BiddyFormula *)
        malloc(biddyFormulaTable.size*sizeof(BiddyFormula))))
  {
    fprintf(stderr,"Biddy_Init (Formula Table): Out of memoy!\n");
    exit(1);
  }
  biddyFormulaTable.table[0].name = strdup("0");
  biddyFormulaTable.table[0].f = biddyZero;
  biddyFormulaTable.table[0].count = 0;
  biddyFormulaTable.table[0].deleted = FALSE;
  biddyFormulaTable.table[1].name = strdup("1");
  biddyFormulaTable.table[1].f = biddyOne;
  biddyFormulaTable.table[1].count = 0;
  biddyFormulaTable.table[1].deleted = FALSE;

  /* INITIALIZATION OF NODE TABLE IN ANONYMOUS MANAGER */
  biddyNodeTable.max = 1;
  biddyNodeTable.num = 1;
  biddyNodeTable.numf = 1;
  biddyNodeTable.foa = 0;
  biddyNodeTable.compare = 0;
  biddyNodeTable.add = 0;
  biddyNodeTable.garbage = 0;
  biddyNodeTable.generated = biddyBlockSize;
  biddyNodeTable.blocknumber = 1;
  biddyNodeTable.swap = 0;
  biddyNodeTable.sifting = 0;

  /* INITIALIZATION OF GARBAGE COLLECTION IN ANONYMOUS MANAGER */
  /* MINIMAL VALUE FOR biddyCount IS 2 (SEE IMPLEMENTATION OF SIFTING) */
  biddyCount = 2;

  /* INITIALIZATION OF CACHE LIST IN ANONYMOUS MANAGER */
  biddyCacheList = NULL;

  /* INITIALIZATION OF DEFAULT ITE CACHE - USED FOR ITE OPERATION */
  biddyIteCache.search = biddyIteCache.find = biddyIteCache.overwrite = 0;
  if (!(biddyIteCache.table = (BiddyOp3Cache *)
  malloc((biddyIteCache.size+1) * sizeof(BiddyOp3Cache)))) {
    fprintf(stderr,"Biddy_Init (ITE CACHE): Out of memoy!\n");
    exit(1);
  }
  for (i=0;i<=biddyIteCache.size;i++) {
    biddyIteCache.table[i].result = biddyNull;
  }
  Biddy_Managed_AddCache(MNG,ITEGarbage);

  /* INITIALIZATION OF DEFAULT EA CACHE - USED FOR QUANTIFICATIONS */
  biddyEACache.search = biddyEACache.find = biddyEACache.overwrite = 0;
  if (!(biddyEACache.table = (BiddyEACache *)
  malloc((biddyEACache.size+1) * sizeof(BiddyEACache)))) {
    fprintf(stderr,"Biddy_Init (EA CACHE): Out of memoy!\n");
    exit(1);
  }
  for (i=0;i<=biddyEACache.size;i++) {
    biddyEACache.table[i].result = biddyNull;
  }
  Biddy_Managed_AddCache(MNG,EAGarbage);

  /* INITIALIZATION OF DEFAULT RC CACHE - USED FOR RESTRICT AND COMPOSE */
  biddyRCCache.search = biddyRCCache.find = biddyRCCache.overwrite = 0;
  if (!(biddyRCCache.table = (BiddyRCCache *)
  malloc((biddyRCCache.size+1) * sizeof(BiddyRCCache)))) {
    fprintf(stderr,"Biddy_Init (RC CACHE): Out of memoy!\n");
    exit(1);
  }
  for (i=0;i<=biddyRCCache.size;i++) {
    biddyRCCache.table[i].result = biddyNull;
  }
  Biddy_Managed_AddCache(MNG,RCGarbage);

  /* DEBUGGING */
  /*
  printf("\n");
  printf("DEBUG: FALSE = %x, TRUE = %x\n",FALSE,TRUE);
  printf("DEBUG: biddyOne = %p\n",biddyOne);
  printf("DEBUG: biddyZero = %p\n",biddyZero);
#if UINTPTR_MAX == 0xffffffffffffffff
  printf("DEBUG: sizeof(BiddyNode) = %lu\n",sizeof(BiddyNode));
  printf("DEBUG: sizeof(BiddyNode *) = %lu\n",sizeof(BiddyNode *));
#else
  printf("DEBUG: sizeof(BiddyNode) = %u\n",sizeof(BiddyNode));
  printf("DEBUG: sizeof(BiddyNode *) = %u\n",sizeof(BiddyNode *));
#endif
  printf("\n");
  */

  /* DEBUGGING WITH GDB AND DDD */
  /* YOU MAY REPLACE biddyOne WITH biddy_termTrue */
  /* USE "up" TO CHANGE CONTEXT */
  /* use "print sizeof(BiddyNode)" */
  /* use "print sizeof(((BiddyNode *) biddyOne).f)" */
  /* use "graph display *((BiddyNode *) biddyOne)" */
  /* use "print ((BiddyNode *) biddyOne)" */
  /* use "print ((BiddyNode *) (((uintptr_t) f) & (~((uintptr_t) 1))))->v */
  /* use "print ((BiddyNode *) ((BiddyNode *) (((uintptr_t) f) & (~((uintptr_t) 1))))->f)->v" */
  /* use "print ((BiddyNode *) ((BiddyNode *) (((uintptr_t) f) & (~((uintptr_t) 1))))->t)->v" */
  /* use "print biddyVariableTable.table[0].name" */

}

#ifdef __cplusplus
}
#endif

/***************************************************************************//*!
\brief Function Biddy_Exit deletes anonymous manager.

### Description
    Deallocates all memory allocated by Biddy_Init, Biddy_FoaVariable,
    Biddy_FoaNode etc.
### Side effects
### More info
*******************************************************************************/

#ifdef __cplusplus
extern "C" {
#endif

void
Biddy_Exit()
{
  int i;
  BiddyCacheList *sup1, *sup2;
  Biddy_Manager MNG;

  MNG = biddyAnonymousManager;

  /*
  printf("Delete name and type...\n");
  */
  free(biddyManagerName); /* MNG[0] is not derefenced by macro */
  free(biddyManagerType);  /* MNG[1] is not derefenced by macro */

  MNG[2] = NULL; /* biddyZero */
  MNG[3] = NULL; /* biddyOne */

  /*
  printf("Delete nodes, block table, and node table...\n");
  */
  for (i=0; i<biddyNodeTable.blocknumber; i++) {
    free(biddyNodeTable.blocktable[i]);
  }
  free(biddyNodeTable.blocktable);
  free(biddyNodeTable.table);
  free((BiddyNodeTable*)(MNG[4]));

  /*
  printf("Delete variable table...\n");
  */
  free(biddyVariableTable.table);
  free((BiddyVariableTable*)(MNG[5]));

  /*
  printf("Delete formula table...\n");
  */
  free(biddyFormulaTable.table);
  free(biddyFormulaTable.deletedName);
  free((BiddyFormulaTable*)(MNG[6]));

  /*
  printf("Delete ITE cache...\n");
  */
  free(biddyIteCache.table);
  free((BiddyOp3CacheTable*)(MNG[7]));

  /*
  printf("Delete EA cache...\n");
  */
  free(biddyEACache.table);
  free((BiddyEACacheTable*)(MNG[8]));

  /*
  printf("Delete RC cache...\n");
  */
  free(biddyRCCache.table);
  free((BiddyRCCacheTable*)(MNG[9]));

  /*
  printf("Delete cache list...\n");
  */
  sup1 = biddyCacheList;
  while (sup1) {
    sup2 = sup1->next;
    free(sup1);
    sup1 = sup2;
  }
  free((BiddyCacheList**)(MNG[10]));

  /*
  printf("Delete blocksize...\n");
  */
  free((unsigned int*)(MNG[11]));

  /*
  printf("Delete pointer biddyFreeNodes...\n");
  */
  free((BiddyNode**)(MNG[12]));

  /*
  printf("Delete Ordering table...\n");
  */
  free((BiddyOrderingTable*)(MNG[13]));

  /*
  printf("Delete counter...\n");
  */
  free((int*)(MNG[14]));

  /*
  printf("And finally, delete manager...\n");
  */
  free((BiddyManager*)MNG);

  /* USER SHOULD DELETE HIS OWN CACHES */
}

#ifdef __cplusplus
}
#endif

/***************************************************************************//*!
\brief Function Biddy_About reports version of Biddy package.

### Description
### Side effects
### More info
*******************************************************************************/

#ifdef __cplusplus
extern "C" {
#endif

Biddy_String
Biddy_About() {
  return strdup(BIDDYVERSION);
}

#ifdef __cplusplus
}
#endif

/***************************************************************************//*!
\brief Function Biddy_GetThen returns THEN successor.

### Description
    Input mark is not considered! External use, only.
### Side effects
### More info
    Macro BiddyT(f) is defined for internal use.
*******************************************************************************/

#ifdef __cplusplus
extern "C" {
#endif

Biddy_Edge
Biddy_GetThen(Biddy_Edge f)
{
  return BiddyT(f);
}

#ifdef __cplusplus
}
#endif

/***************************************************************************//*!
\brief Function Biddy_GetElse returns ELSE successor.

### Description
    Input mark is not considered! External use, only.
### Side effects
### More info
    Macro BiddyE(f) is defined for internal use.
*******************************************************************************/

#ifdef __cplusplus
extern "C" {
#endif

Biddy_Edge
Biddy_GetElse(Biddy_Edge f)
{
  return BiddyE(f);
}

#ifdef __cplusplus
}
#endif

/***************************************************************************//*!
\brief Function Biddy_GetTopVariable returns the top variable.

### Description
### Side effects
### More info
*******************************************************************************/

#ifdef __cplusplus
extern "C" {
#endif

Biddy_Variable
Biddy_GetTopVariable(Biddy_Edge f)
{
  return ((BiddyNode *) Biddy_Regular(f))->v;
}

#ifdef __cplusplus
}
#endif

/***************************************************************************//*!
\brief Function Biddy_SelectNode selects the top node of the given function.

### Description
### Side effects
    This would be faster but more danger to implement without checking if
    already selected.
### More info
*******************************************************************************/

#ifdef __cplusplus
extern "C" {
#endif

void
Biddy_SelectNode(Biddy_Edge f)
{
  if (((BiddyNode *) Biddy_Regular(f))->count >= 0)
    ((BiddyNode *) Biddy_Regular(f))->count =
      ~((BiddyNode *) Biddy_Regular(f))->count;
}

#ifdef __cplusplus
}
#endif

/***************************************************************************//*!
\brief Function Biddy_DeselectNode deselects the top node of the given function.

### Description
### Side effects
    This would be faster but more danger to implement without checking if
    already deselected.
### More info
*******************************************************************************/

#ifdef __cplusplus
extern "C" {
#endif

void
Biddy_DeselectNode(Biddy_Edge f)
{
  if (((BiddyNode *) Biddy_Regular(f))->count < 0)
    ((BiddyNode *) Biddy_Regular(f))->count =
      ~((BiddyNode *) Biddy_Regular(f))->count;
}

#ifdef __cplusplus
}
#endif

/***************************************************************************//*!
\brief Function Biddy_IsSelected returns TRUE iff the top node of the given
       function is selected.

### Description
### Side effects
### More info
*******************************************************************************/

#ifdef __cplusplus
extern "C" {
#endif

Biddy_Boolean
Biddy_IsSelected(Biddy_Edge f)
{
  return (((BiddyNode *) Biddy_Regular(f))->count < 0);
}

#ifdef __cplusplus
}
#endif

/***************************************************************************//*!
\brief Function Biddy_NodeSelect recursively selects all nodes of a
       given function.

### Description
### Side effects
    Constant node must be selected before starting this function!
### More info
*******************************************************************************/

#ifdef __cplusplus
extern "C" {
#endif

void
Biddy_NodeSelect(Biddy_Edge f)
{
  if (!Biddy_IsSelected(f)) {
    Biddy_SelectNode(f);
    Biddy_NodeSelect(BiddyE(f));
    Biddy_NodeSelect(BiddyT(f));
  }
}

#ifdef __cplusplus
}
#endif

/***************************************************************************//*!
\brief Function Biddy_NodeRepair recursively repairs (deselects) all nodes of a
       given function.

### Description
    Selection of nodes (used for statistics, reordering and garbage collection
    techniques etc.) creates invalid counts that have to be repaired before any
    operation on BDDs.
### Side effects
    Constant node must not be selected before starting this function!
### More info
*******************************************************************************/

#ifdef __cplusplus
extern "C" {
#endif

void
Biddy_NodeRepair(Biddy_Edge f)
{
  if (Biddy_IsSelected(f)) {
    Biddy_DeselectNode(f);
    Biddy_NodeRepair(BiddyE(f));
    Biddy_NodeRepair(BiddyT(f));
  }
}

#ifdef __cplusplus
}
#endif

/***************************************************************************//*!
\brief Function Biddy_NOT complements Boolean function.

### Description
    Deprecated! Use macro Biddy_Not.
### Side effects
### More info
*******************************************************************************/

#ifdef __cplusplus
extern "C" {
#endif

Biddy_Edge
Biddy_NOT(Biddy_Edge f)
{
  Biddy_InvertMark(f);
  return f;
}

#ifdef __cplusplus
}
#endif

/***************************************************************************//*!
\brief Function Biddy_TransferMark complements Boolean function if
       given parameter mark is TRUE.

### Description
    Deprecated! Use macro Biddy_NotCond.
### Side effects
### More info
*******************************************************************************/

#ifdef __cplusplus
extern "C" {
#endif

Biddy_Edge
Biddy_TransferMark(Biddy_Edge f, Biddy_Boolean mark)
{
  if (Biddy_GetMark(f) != mark) Biddy_SetMark(f); else Biddy_ClearMark(f);
  return f;
}

#ifdef __cplusplus
}
#endif

/***************************************************************************//*!
\brief Function Biddy_Managed_GetConstantZero returns constant 0.

### Description
    Constants 0 and 1 depend on a manager.
### Side effects
### More info
    Internally, use macro biddyZero.
    Macro Biddy_GetConstantZero() is defined for use with anonymous manager.
*******************************************************************************/

Biddy_Edge
Biddy_Managed_GetConstantZero(Biddy_Manager MNG)
{
  if (!MNG) MNG = biddyAnonymousManager;

  return biddyZero;
}

/***************************************************************************//*!
\brief Function Biddy_Managed_GetConstantOne returns constant 1.

### Description
    Constants 0 and 1 depend on a manager.
### Side effects
### More info
    Internally, use macro biddyOne.
    Macro Biddy_GetConstantOne() is defined for use with anonymous manager.
*******************************************************************************/

Biddy_Edge
Biddy_Managed_GetConstantOne(Biddy_Manager MNG)
{
  if (!MNG) MNG = biddyAnonymousManager;

  return biddyOne;
}

/***************************************************************************//*!
\brief Function Biddy_Managed_GetVariableName returns the name of a variable.

### Description
### Side effects
### More info
    Macro Biddy_GetVariableName(v) is defined for use with anonymous manager.
*******************************************************************************/

#ifdef __cplusplus
extern "C" {
#endif

Biddy_String
Biddy_Managed_GetVariableName(Biddy_Manager MNG, Biddy_Variable v)
{
  if (!MNG) MNG = biddyAnonymousManager;

  return biddyVariableTable.table[v].name;
}

#ifdef __cplusplus
}
#endif

/***************************************************************************//*!
\brief Function Biddy_Managed_GetTopVariableName returns the name of top
       variable.

### Description
### Side effects
### More info
    Macro Biddy_GetTopVariableName(f) is defined for use with anonymous manager.
*******************************************************************************/

#ifdef __cplusplus
extern "C" {
#endif

Biddy_String
Biddy_Managed_GetTopVariableName(Biddy_Manager MNG, Biddy_Edge f)
{
  if (!MNG) MNG = biddyAnonymousManager;

  return biddyVariableTable.table[((BiddyNode *) Biddy_Regular(f))->v].name;
}

#ifdef __cplusplus
}
#endif

/***************************************************************************//*!
\brief Function Biddy_Managed_GetTopVariableChar returns the first character in
       the name of top variable.

### Description
### Side effects
### More info
    Macro Biddy_GetTopVariableChar(f) is defined for use with anonymous manager.
*******************************************************************************/

#ifdef __cplusplus
extern "C" {
#endif

char
Biddy_Managed_GetTopVariableChar(Biddy_Manager MNG, Biddy_Edge f)
{
  if (!MNG) MNG = biddyAnonymousManager;

  return biddyVariableTable.table[((BiddyNode *) Biddy_Regular(f))->v].name[0];
}

#ifdef __cplusplus
}
#endif

/***************************************************************************//*!
\brief Function Biddy_Managed_ResetVariablesValue sets all variable's value to
       biddyZero, except constant node which gets value biddyOne.

### Description
### Side effects
    Only active (used) variables are reinitialized.
### More info
    Macro Biddy_ResetVariablesValue() is defined for use with anonymous manager.
*******************************************************************************/

#ifdef __cplusplus
extern "C" {
#endif

void
Biddy_Managed_ResetVariablesValue(Biddy_Manager MNG)
{
  Biddy_Variable v;

  if (!MNG) MNG = biddyAnonymousManager;

  biddyVariableTable.table[0].value = biddyOne;
  for (v=1; v<biddyVariableTable.num; v++) {
    biddyVariableTable.table[v].value = biddyZero;
  }
}

#ifdef __cplusplus
}
#endif

/***************************************************************************//*!
\brief Function Biddy_Managed_SetVariableValue sets variable's value.

### Description
### Side effects
### More info
    Macro Biddy_SetVariableValue(v,f) is defined for use with anonymous manager.
*******************************************************************************/

#ifdef __cplusplus
extern "C" {
#endif

void
Biddy_Managed_SetVariableValue(Biddy_Manager MNG, Biddy_Variable v, Biddy_Edge f)
{
  if (!MNG) MNG = biddyAnonymousManager;

  biddyVariableTable.table[v].value = f;
}

#ifdef __cplusplus
}
#endif

/***************************************************************************//*!
\brief Function Biddy_Managed_IsSmaller returns TRUE if the first variable is
       smaller (= lower = topmore).

### Description
### Side effects
### More info
    Macro Biddy_IsSmaller(fv,gv) is defined for use with anonymous manager.
*******************************************************************************/

#ifdef __cplusplus
extern "C" {
#endif

Biddy_Boolean
Biddy_Managed_IsSmaller(Biddy_Manager MNG, Biddy_Variable fv, Biddy_Variable gv)
{
  if (!MNG) MNG = biddyAnonymousManager;

  return GET_ORDER(biddyOrderingTable,fv,gv);
}

#ifdef __cplusplus
}
#endif

/***************************************************************************//*!
\brief Function Biddy_Managed_AddVariable adds a numbered variable.

### Description
    Numbered variables have only digits in its name.
    The current number of numbered variables is stored in numnum.
    Function increments numnum and always creates non-existing
    variable. Function returns variable edge.
### Side effects
### More info
    Macro Biddy_AddVariable() is defined for use with anonymous manager.
*******************************************************************************/

#ifdef __cplusplus
extern "C" {
#endif

Biddy_Edge
Biddy_Managed_AddVariable(Biddy_Manager MNG)
{
  Biddy_String x;
  Biddy_Edge f;

  if (!MNG) MNG = biddyAnonymousManager;

  biddyVariableTable.numnum++;

  x = (Biddy_String) malloc(15);
  sprintf(x,"%u",biddyVariableTable.numnum);
  f = Biddy_Managed_FoaVariable(MNG,x);
  free(x);

  return f;
}

#ifdef __cplusplus
}
#endif

/***************************************************************************//*!
\brief Function Biddy_Managed_AddVariableBelow adds a numbered variable.

### Description
    Biddy_Managed_AddVariableBelow uses Biddy_Managed_AddVariable to add
    numbered variable. Then, the order of the new variable is changed to become
    immediately below the given variable (below = topmore in BDD)
    Function returns variable edge.
### Side effects
### More info
    Macro Biddy_AddVariableBelow(v) is defined for use with anonymous manager.
*******************************************************************************/

#ifdef __cplusplus
extern "C" {
#endif

Biddy_Edge
Biddy_Managed_AddVariableBelow(Biddy_Manager MNG, Biddy_Variable v)
{
  Biddy_Edge f;
  Biddy_Variable x;
  Biddy_Variable i;

  if (!MNG) MNG = biddyAnonymousManager;

  f = Biddy_Managed_AddVariable(MNG);
  x = Biddy_GetTopVariable(f);

  for (i=0; i<biddyVariableTable.size; i++) {
    if (GET_ORDER(biddyOrderingTable,i,v)) {
      SET_ORDER(biddyOrderingTable,i,x);
      CLEAR_ORDER(biddyOrderingTable,x,i);
    } else {
      SET_ORDER(biddyOrderingTable,x,i);
      CLEAR_ORDER(biddyOrderingTable,i,x);
    }
  }

  return f;
}

#ifdef __cplusplus
}
#endif

/***************************************************************************//*!
\brief Function Biddy_Managed_AddVariableAbove adds a numbered variable.

### Description
    Biddy_Managed_AddVariableAbove uses Biddy_Managed_AddVariable to add
    numbered variable. Then, the order of the new variable is changed to become
    immediately above the given variable (above = bottommore in BDD)
    Function returns variable edge.
### Side effects
### More info
    Macro Biddy_AddVariableAbove(v) is defined for use with anonymous manager.
*******************************************************************************/

#ifdef __cplusplus
extern "C" {
#endif

Biddy_Edge
Biddy_Managed_AddVariableAbove(Biddy_Manager MNG, Biddy_Variable v)
{
  Biddy_Edge f;
  Biddy_Variable x;
  Biddy_Variable i;

  if (!MNG) MNG = biddyAnonymousManager;

  if (v == 0) {
    fprintf(stderr,"WARNING: variable cannot be added above constant node 1\n");
    return biddyNull;
  }

  f = Biddy_Managed_AddVariable(MNG);
  x = Biddy_GetTopVariable(f);

  for (i=0; i<biddyVariableTable.size; i++) {
    if (GET_ORDER(biddyOrderingTable,v,i)) {
      SET_ORDER(biddyOrderingTable,x,i);
      CLEAR_ORDER(biddyOrderingTable,i,x);
    } else {
      SET_ORDER(biddyOrderingTable,i,x);
      CLEAR_ORDER(biddyOrderingTable,x,i);
    }
  }

  return f;
}

#ifdef __cplusplus
}
#endif

/***************************************************************************//*!
\brief Function Biddy_Managed_FoaVariable finds or adds variable edge.

### Description
    Variable edge points to a node with successors biddyZero and biddyOne.
    If such variable edge already exists, function returns it and does not
    create the new one.
### Side effects
    Inefficient! We should use some kind of searching tree.
### More info
    Macro Biddy_FoaVariable(x) is defined for use with anonymous manager.
*******************************************************************************/

#ifdef __cplusplus
extern "C" {
#endif

Biddy_Edge
Biddy_Managed_FoaVariable(Biddy_Manager MNG, Biddy_String x)
{
  Biddy_Variable v;
  Biddy_Boolean find;
  Biddy_Edge result;

  if (!MNG) MNG = biddyAnonymousManager;

  /* VARIABLE TABLE IS NEVER EMPTY. AT LEAST, THERE IS ELEMENT '1' AT INDEX [0] */
  v = 0;
  find = FALSE;
  while (!find && v<biddyVariableTable.num) {
    if (!strcmp(x,biddyVariableTable.table[v].name)) {
      find = TRUE;
    } else {
      v++;
    }
  }

  if (!find) {

    /* IF THE ELEMENT WAS NOT FOUND CREATE NEW VARIABLE */

    if (v == biddyVariableTable.size) {
      fprintf(stderr,"\nBIDDY (Biddy_Managed_FoaVariable): Size error!\n");
      fprintf(stderr,"Currently, there exist %d variables.\n",biddyVariableTable.num);
      exit(1);
    } else {
      biddyVariableTable.num++; /* v IS NOW THE INDEX OF THE LAST ELEMENT! */
    }

    /* LIST OF NEW NODES FOR ANY [v] MUST NEVER BE EMPTY */
    /* YOU MUST NEVER ASSUME THAT [v]->lastNewNode->list = NULL */
    /* FIRST ELEMENT OF LIST OF NEW NODES IS firstNewNode */
    /* LAST ELEMENT OF LIST OF NEW NODES IS lastNewNode */
    /* FIRST ELEMENT OF LIST OF FORTIFIED NODES IS fortifiedNodes */
    /* LAST ELEMENT OF LIST OF FORTIFIED NODES HAS list = NULL */
    /* THE NODE CREATED HERE IS ALWAYS FIRST IN THE LIST OF NEW NODES */
    /* THE NODE CREATED HERE IS NEVER MOVED TO THE LIST OF FORTIFIED NODES */
    /* CONSTANT NODE "1" IS USED HERE BUT IT IS NOT ADDED TO ANY LISTS */

    biddyVariableTable.table[v].name = strdup(x);
    biddyVariableTable.table[v].lastNewNode = biddyNodeTable.table[0];
    result = Biddy_Managed_FoaNode(MNG,v,biddyZero,biddyOne,FALSE);
    biddyVariableTable.table[v].firstNewNode = (BiddyNode *) result;
    biddyVariableTable.table[v].freshNodes = (BiddyNode *) result;
    biddyVariableTable.table[v].fortifiedNodes = NULL;
    biddyNodeTable.table[0]->list = NULL; /* REPAIR CONSTANT NODE */

  } else {

    result = Biddy_Managed_FoaNode(MNG,v,biddyZero,biddyOne,FALSE);

  }

  /* ALL VARIABLES MUST BE STORED IN FORMULA TABLE */
  /* THIS IS IMPORTANT EVEN IF VARIABLE ALREADY EXISTS */
  /* CURRENTLY, VARIABLE MAY BE HIDDEN BY SOME FORMULA */
  /* THIS WILL ALSO FORTIFY NODE */
  Biddy_Managed_AddPersistentFormula(MNG,x,result);
 
  /* RETURN THE EDGE, WHICH HAS BEEN LOOKED FOR */
  return result;
}


#ifdef __cplusplus
}
#endif

/***************************************************************************//*!
\brief Function Biddy_Managed_FoaNode finds or adds new node with the given
       variable and successors.

### Description
    If such node already exists, function returns it and does not create the
    new one.
### Side effects
    Using Biddy_Managed_FoaNode you can create node with arbitrary ordering.
    It is much more safe to use Biddy_Managed_ITE. To enable efficient
    implementation of e.g. sifting the function started with the returned
    node is not refreshed!
### More info
    Macro Biddy_FoaNode(v,pf,pt,garbageAllowed) is defined for use with
    anonymous manager.
*******************************************************************************/

#ifdef __cplusplus
extern "C" {
#endif

Biddy_Edge
Biddy_Managed_FoaNode(Biddy_Manager MNG, Biddy_Variable v, Biddy_Edge pf,
                      Biddy_Edge pt, Biddy_Boolean garbageAllowed)
{
  unsigned int hash;
  Biddy_Edge edge;
  BiddyNode *sup, *sup1;
  Biddy_Boolean complementedResult;
  int i;

  if (!MNG) MNG = biddyAnonymousManager;

  biddyNodeTable.foa++;

  /* MINIMIZATION */

  if (Biddy_IsEqv(pf,pt)) return pf;

  /* NORMALIZATION - EDGE 'THEN' MAY NOT BE COMPLEMENTED ! */
  if (Biddy_GetMark(pt)) {
    Biddy_InvertMark(pf);
    Biddy_ClearMark(pt);
    complementedResult = TRUE;
  } else {
    complementedResult = FALSE;
  }

  /* THIS STATEMENT DISABLES GARBAGE COLLECTION */
  /*
  garbageAllowed = FALSE;
  */

  /* IF ALL GENERATED NODES ARE USED THEN TRY GARBAGE COLLECTION */
  if (garbageAllowed && !biddyFreeNodes) Biddy_Managed_Garbage(MNG);

  /* THIS IS HASH FUNCTION FOR NODE TABLE */
  /* BECAUSE OF USED TRICKS, HASH FUNCTION MUST NEVER RETURN ZERO! */

  hash = nodeTableHash(v,pf,pt,biddyNodeTable.size);

  /* DEBUGGING */
  /*
  printf("FoaNode: v=%u, pf=%p, pt=%p, hash=%u\n",v,(void*)pf,(void*)pt,hash);
  */

  /* FIND OR ADD - THERE IS A HASH TABLE WITH CHAINING */

  sup = biddyNodeTable.table[hash];
  sup1 = findNodeTable(MNG,v,pf,pt,&sup);

  if ((!sup) || (v != sup->v)) {

    /* NEW NODE MUST BE ADDED - IT WILL BE FRESH */
    biddyNodeTable.num++;
    biddyNodeTable.add++;
    (biddyVariableTable.table[v].num)++;
    if (biddyNodeTable.num > biddyNodeTable.max)
      biddyNodeTable.max = biddyNodeTable.num;

    /* THESE LINES PREVENT ALLOCATION OF MEMORY AFTER INITIALIZATION */
    /*
    if (!biddyFreeNodes) {
      fprintf(stderr,"\nBIDDY (Biddy_Managed_FoaNode): Allocated node table is full!\n");
      fprintf(stderr,"Currently, there exist %d nodes.\n",biddyNodeTable.num);
      exit(1);
    }
    */

    /* THIS COULD BE USEFUL */
    /*
    if (!biddyFreeNodes || (biddyNodeTable.num >= 0.9*biddyNodeTable.generated)) {...}
    */

    if (!biddyFreeNodes) {

      if (!(biddyFreeNodes = (BiddyNode *)
            malloc((biddyBlockSize) * sizeof(BiddyNode))))
      {
        fprintf(stderr,"\nBIDDY (Biddy_Managed_FoaNode): Out of memory error!\n");
        fprintf(stderr,"Currently, there exist %d nodes.\n",biddyNodeTable.num);
        exit(1);
      }

      biddyNodeTable.blocknumber++;
      if (!(biddyNodeTable.blocktable = (BiddyNode **) realloc(biddyNodeTable.blocktable,
            biddyNodeTable.blocknumber * sizeof(BiddyNode *))))
      {
        fprintf(stderr,"\nBIDDY (Biddy_Managed_FoaNode): Out of memory error!\n");
        fprintf(stderr,"Currently, there exist %d nodes.\n",biddyNodeTable.num);
        exit(1);
      }
      biddyNodeTable.blocktable[biddyNodeTable.blocknumber-1] = biddyFreeNodes;

      for (i=0; i<biddyBlockSize-1; i++) {
        biddyFreeNodes[i].list = (void *) &biddyFreeNodes[i+1];
      }
      biddyFreeNodes[biddyBlockSize-1].list = NULL;

      biddyNodeTable.generated = biddyNodeTable.generated + biddyBlockSize;
    }

    sup = biddyFreeNodes;
    biddyFreeNodes = (BiddyNode *) sup->list;
 
    sup->f = pf; /* BE CAREFULL !!!! */
    sup->t = pt; /* you can create node with an arbitrary (wrong!) ordering */
    sup->count = biddyCount;
    sup->v = v;

    /* add new node at the end of list */
    /* lastNewNode->list IS NOT DEFINED! */
    /* YOU MUST NEVER ASSUME THAT lastNewNode->list = NULL */
  
    addNodeTable(MNG,hash,sup,sup1);
    biddyVariableTable.table[v].lastNewNode->list = (void *) sup;
    biddyVariableTable.table[v].lastNewNode = sup;

  }

  /* SINCE BIDDY V1.1 */
  edge = sup;
  if (complementedResult) Biddy_SetMark(edge);

  /* (Biddy v1.5) To enable efficient implementation of e.g. sifting */
  /* the function started with the returned node is not refreshed! */
  /* BiddyRefresh(MNG,edge); */

  return edge;
}

#ifdef __cplusplus
}
#endif

/***************************************************************************//*!
\brief Function Biddy_Managed_ITE calculates ITE operation of three Boolean
       functions.

### Description
### Side Effects
### More info
    Macro Biddy_ITE(f,g,h) is defined for use with anonymous manager.
*******************************************************************************/

#ifdef __cplusplus
extern "C" {
#endif

Biddy_Edge
Biddy_Managed_ITE(Biddy_Manager MNG, Biddy_Edge f, Biddy_Edge g, Biddy_Edge h)
{
  Biddy_Boolean negation,simple;
  Biddy_Edge r, T, E, Fv, Gv, Hv, Fneg_v, Gneg_v, Hneg_v;
  Biddy_Variable v;

  static Biddy_Boolean terminal; /* CAN BE STATIC, WHAT IS BETTER? */
  static Biddy_Variable topF;    /* CAN BE STATIC, WHAT IS BETTER? */
  static Biddy_Variable topG;    /* CAN BE STATIC, WHAT IS BETTER? */
  static Biddy_Variable topH;    /* CAN BE STATIC, WHAT IS BETTER? */

  if (!MNG) MNG = biddyAnonymousManager;

  /* DEBUGGING */
  /*
  static int mm = 0;
  int nn;
  nn = ++mm;
  fprintf(stdout,"Biddy_Managed_ITE (%d) F\n",nn);
  writeBDD(MNG,f);
  fprintf(stdout,"\n");
  fprintf(stdout,"Biddy_Managed_ITE (%d) G\n",nn);
  writeBDD(MNG,g);
  fprintf(stdout,"\n");
  fprintf(stdout,"Biddy_Managed_ITE (%d) H\n",nn);
  writeBDD(MNG,h);
  fprintf(stdout,"\n");
  mm = nn;
  */

  /* DEBUGGING */
  /**/
  if (!f) {
    fprintf(stdout,"ERROR (Biddy_Managed_ITE): f = NULL!\n");
    free((void *)1); /* BREAKPOINT */
  }
  if (!g) {
    fprintf(stdout,"ERROR (Biddy_Managed_ITE): g = NULL!\n");
    free((void *)1); /* BREAKPOINT */
  }
  if (!h) {
    fprintf(stdout,"ERROR (Biddy_Managed_ITE): h = NULL!\n");
    free((void *)1); /* BREAKPOINT */
  }
  /**/

  /**/
  if (!BiddyIsOK(f)) {
    fprintf(stdout,"ERROR (Biddy_Managed_ITE): Bad f\n");
    free((void *)1); /* BREAKPOINT */
  }
  if (!BiddyIsOK(g)) {
    fprintf(stdout,"ERROR (Biddy_Managed_ITE): Bad g\n");
    free((void *)1); /* BREAKPOINT */
  }
  if (!BiddyIsOK(h)) {
    fprintf(stdout,"ERROR (Biddy_Managed_ITE): Bad h\n");
    free((void *)1); /* BREAKPOINT */
  }
  /**/

  /* NORMALIZATION - STEP 1 */

  /* CURRENT IMPLEMENTATION - PROBABLY THE BEST */
  /**/
  if (Biddy_IsEqvPointer(f,g)) {
    if (!Biddy_IsEqv(f,g)) g = biddyZero; else g = biddyOne;
  }

  if (Biddy_IsEqvPointer(f,h)) {
    if (Biddy_IsEqv(f,h)) h = biddyZero; else h = biddyOne;
  }
  /**/

  /* ALTERNATIVE IMPLEMENTATION */
  /*
  if (Biddy_IsEqv(f,g)) {
    g = biddyOne;
  } else if (Biddy_IsEqvPointer(f,g)) {
    g = biddyZero;
  }

  if (Biddy_IsEqv(f,h)) {
    h = biddyZero;
  } else if (Biddy_IsEqvPointer(f,h)) {
    h = biddyOne;
  }
  */

  negation = FALSE;

  /* LOOKING FOR TERMINAL CASE */

  terminal = FALSE;
  if (Biddy_IsConstant(f)) {
    terminal = TRUE;
    /* r = Biddy_GetMark(f) ? h : g; */
    r = Biddy_IsEqv(f,biddyOne) ? g : h;
  } else {
    if (Biddy_IsEqvPointer(g,h)) {
      if (Biddy_IsEqv(g,h)) {
        terminal = TRUE;
        r = g;
      } else {
        if (Biddy_IsConstant(g)) {
          terminal = TRUE;
          r = f;
          negation = Biddy_GetMark(g);
        }
      }
    }
  }

  /* IF NOT TERMINAL CASE */
  if (!terminal) {

    /* NORMALIZATION - STEP 2 */

    if (Biddy_IsConstant(g)) {
      if (((uintptr_t) f) > ((uintptr_t) h)) {
        if (Biddy_GetMark(g)) {
          complExchangeEdges(&f, &h);
        } else {
          exchangeEdges(&f, &h);
        }
      }
    }

    if (Biddy_IsConstant(h)) {
      if (((uintptr_t) f) > ((uintptr_t) g)) {
        if (Biddy_GetMark(h)) {
          exchangeEdges(&f, &g);
        } else {
          complExchangeEdges(&f, &g);
        }
      }
    }

    /* WE KNOW, THAT G AND H ARE NOT EQUAL, BUT THEY CAN BE INVERTED! */
    if (Biddy_IsEqvPointer(g,h)) {
      if (((uintptr_t) f) > ((uintptr_t) g)) {
        h = f;
        f = g;
        g = h;
        Biddy_InvertMark(h);
      }
    }

    /* NORMALIZATION - STEP 3 */

    if (Biddy_GetMark(f)) {
      if (Biddy_GetMark(h)) {
        Biddy_ClearMark(f);
        negation = TRUE;
        complExchangeEdges(&g, &h);
      } else {
        Biddy_ClearMark(f);
        exchangeEdges(&g, &h);
      }
    } else {
      if (Biddy_GetMark(g)) {
        negation = TRUE;
        Biddy_ClearMark(g);
        Biddy_InvertMark(h);
      }
    }

    /* IF RESULT IS NOT IN THE CACHE TABLE... */
    if (!findITECache(MNG,f,g,h,&r))
    {

      /* LOOKING FOR THE SMALLES TOP VARIABLE */
      topF = Biddy_GetTopVariable(f);
      topG = Biddy_GetTopVariable(g);
      topH = Biddy_GetTopVariable(h);

      /* SINCE BIDDY V1.2 */
      /* CONSTANT NODE MUST HAVE MAX ORDER */
      if (Biddy_Managed_IsSmaller(MNG,topF,topG)) {
        v = Biddy_Managed_IsSmaller(MNG,topF,topH) ? topF : topH;
      } else {
        v = Biddy_Managed_IsSmaller(MNG,topH,topG) ? topH : topG;
      }

      /* DETERMINING PARAMETERS FOR RECURSIVE CALLS */
      if (topF == v) {
        if (Biddy_GetMark(f)) {
          Fv = Biddy_Not(BiddyT(f));
          Fneg_v = Biddy_Not(BiddyE(f));
        } else {
          Fv = BiddyT(f);
          Fneg_v = BiddyE(f);
        }
      } else {
        Fneg_v = Fv = f;
      }

      if (topG == v) {
        if (Biddy_GetMark(g)) {
          Gv = Biddy_Not(BiddyT(g));
          Gneg_v = Biddy_Not(BiddyE(g));
        } else {
          Gv = BiddyT(g);
          Gneg_v = BiddyE(g);
        }
      } else {
         Gneg_v = Gv = g;
      }

      if (topH == v) {
        if (Biddy_GetMark(h)) {
          Hv = Biddy_Not(BiddyT(h));
          Hneg_v = Biddy_Not(BiddyE(h));
        } else {
          Hv = BiddyT(h);
          Hneg_v = BiddyE(h);
        }
      } else {
        Hneg_v = Hv = h;
      }

      /* RECURSIVE CALLS */

      simple = FALSE;
      if (Biddy_IsConstant(Fv)) {
        T = Biddy_IsEqv(Fv,biddyOne) ? Gv : Hv;
        simple = TRUE;
      } else  {
        T = Biddy_Managed_ITE(MNG,Fv,Gv,Hv);
      }

      if (Biddy_IsConstant(Fneg_v)) {
        E = Biddy_IsEqv(Fneg_v,biddyOne) ? Gneg_v : Hneg_v;
        simple = TRUE;
      } else {
        E = Biddy_Managed_ITE(MNG,Fneg_v,Gneg_v,Hneg_v);
      }

      r = Biddy_Managed_FoaNode(MNG,v,E,T,TRUE);
      BiddyRefresh(MNG,r); /* FoaNode will not refresh function! */

      /* CACHE EVERYTHING */
      /**/
      addITECache(MNG,f,g,h,r);
      /**/

      /* THIS DOES NOT PRODUCE BETTER RESULTS */
      /*
      if (!simple) addITECache(MNG,f,g,h,r);
      */

    } else {

      /* IF THE RESULT IS FROM CACHE TABLE, FRESH IT! */
      BiddyRefresh(MNG,r);

    }
  }

  if (negation) {
    Biddy_InvertMark(r);
  }

  /* DEBUGGING */
  /*
  fprintf(stdout,"Biddy_ITE (%d) R\n",nn);
  if (terminal) fprintf(stdout,"TERMINAL CASE\n");
  writeBDD(MNG,r);
  fprintf(stdout,"\n");
  */

  return r;
}

#ifdef __cplusplus
}
#endif

/***************************************************************************//*!
\brief Function Biddy_Managed_And calculates Boolean function AND (conjunction).

### Description
### Side Effects
    Uses ITE. Less efficient as direct implementation in CUDD.
### More Info
    Macro Biddy_And(f,g) is defined for use with anonymous manager.
*******************************************************************************/

#ifdef __cplusplus
extern "C" {
#endif

Biddy_Edge
Biddy_Managed_And(Biddy_Manager MNG, Biddy_Edge f, Biddy_Edge g)
{
  if (!MNG) MNG = biddyAnonymousManager;

  return Biddy_Managed_ITE(MNG,f,g,biddyZero);
}

#ifdef __cplusplus
}
#endif

/***************************************************************************//*!
\brief Function Biddy_Managed_Or calculates Boolean function OR (disjunction).

### Description
### Side Effects
    Uses ITE. Less efficient as direct implementation in CUDD.
### More Info
    Macro Biddy_Or(f,g) is defined for use with anonymous manager.
*******************************************************************************/

#ifdef __cplusplus
extern "C" {
#endif

Biddy_Edge
Biddy_Managed_Or(Biddy_Manager MNG, Biddy_Edge f, Biddy_Edge g)
{
  if (!MNG) MNG = biddyAnonymousManager;

  return Biddy_Managed_ITE(MNG,f,biddyOne,g);
}

#ifdef __cplusplus
}
#endif

/***************************************************************************//*!
\brief Function Biddy_Managed_Nand calculates Boolean function NAND (Sheffer).

### Description
### Side Effects
    Uses ITE. Less efficient as direct implementation in CUDD.
### More Info
    Macro Biddy_Nand(f,g) is defined for use with anonymous manager.
*******************************************************************************/

#ifdef __cplusplus
extern "C" {
#endif

Biddy_Edge
Biddy_Managed_Nand(Biddy_Manager MNG, Biddy_Edge f, Biddy_Edge g)
{
  if (!MNG) MNG = biddyAnonymousManager;

  return Biddy_Managed_ITE(MNG,f,Biddy_Not(g),biddyOne);
}

#ifdef __cplusplus
}
#endif

/***************************************************************************//*!
\brief Function Biddy_Managed_Nor calculates Boolean function NOR (Peirce).

### Description
### Side Effects
    Uses ITE. Less efficient as direct implementation in CUDD.
### More Info
    Macro Biddy_Nor(f,g) is defined for use with anonymous manager.
*******************************************************************************/

#ifdef __cplusplus
extern "C" {
#endif

Biddy_Edge
Biddy_Managed_Nor(Biddy_Manager MNG, Biddy_Edge f, Biddy_Edge g)
{
  if (!MNG) MNG = biddyAnonymousManager;

  return Biddy_Managed_ITE(MNG,f,biddyZero,Biddy_Not(g));
}

#ifdef __cplusplus
}
#endif

/***************************************************************************//*!
\brief Function Biddy_Managed_Xor calculates Boolean function XOR.

### Description
### Side Effects
    Uses ITE. Less efficient as direct implementation in CUDD.
### More Info
    Macro Biddy_Xor(f,g) is defined for use with anonymous manager.
*******************************************************************************/

#ifdef __cplusplus
extern "C" {
#endif

Biddy_Edge
Biddy_Managed_Xor(Biddy_Manager MNG, Biddy_Edge f, Biddy_Edge g)
{
  if (!MNG) MNG = biddyAnonymousManager;

  return Biddy_Managed_ITE(MNG,f,Biddy_Not(g),g);
}

#ifdef __cplusplus
}
#endif

/***************************************************************************//*!
\brief Function Biddy_Managed_Xnor calculates Boolean function XNOR.

### Description
### Side Effects
    Uses ITE. Less efficient as direct implementation in CUDD.
### More Info
    Macro Biddy_Xnor(f,g) is defined for use with anonymous manager.
*******************************************************************************/

#ifdef __cplusplus
extern "C" {
#endif

Biddy_Edge
Biddy_Managed_Xnor(Biddy_Manager MNG, Biddy_Edge f, Biddy_Edge g)
{
  if (!MNG) MNG = biddyAnonymousManager;

  return Biddy_Managed_ITE(MNG,f,g,Biddy_Not(g));
}

#ifdef __cplusplus
}
#endif

/***************************************************************************//*!
\brief Function Biddy_Managed_Leq returns TRUE iff function f is included in
       function g.

### Description
### Side Effects
    Implemented by calculating full implication which is less efficient as
    implementation in CUDD.
### More Info
    Macro Biddy_Leq(f,g) is defined for use with anonymous manager.
*******************************************************************************/

#ifdef __cplusplus
extern "C" {
#endif

Biddy_Boolean
Biddy_Managed_Leq(Biddy_Manager MNG, Biddy_Edge f, Biddy_Edge g)
{
  Biddy_Edge r;

  if (!MNG) MNG = biddyAnonymousManager;

  r = Biddy_Managed_ITE(MNG,f,g,biddyOne);
  return Biddy_IsEqv(r,biddyOne);
}

#ifdef __cplusplus
}
#endif

/***************************************************************************//*!
\brief Biddy_Managed_Intersect calculates a function included in the
       intersection of f and g.

### Description
    If the result is not constant 0 then it is a witness that the intersection
    is not empty. The result should be calculated with as few new nodes as
    possible, and the result may not be the same as conjunction between
    functions! If the only result of interest is whether f and g intersect,
    Biddy_Leq should be used instead (return (f<=g')).
### Side Effects
    Implemented by calculating full conjunction which is less efficient as
    implementation in CUDD.
### More Info
    Macro Biddy_Intersect(f,g) is defined for use with anonymous manager.
*******************************************************************************/

#ifdef __cplusplus
extern "C" {
#endif

Biddy_Edge
Biddy_Managed_Intersect(Biddy_Manager MNG, Biddy_Edge f, Biddy_Edge g)
{
  if (!MNG) MNG = biddyAnonymousManager;

  return Biddy_Managed_ITE(MNG,f,g,biddyZero);
}

#ifdef __cplusplus
}
#endif

/***************************************************************************//*!
\brief Function Biddy_Managed_Restrict calculates a restriction of Boolean
       function.

### Description
    Value must be constant 0 or constant 1.
    It uses optimization: F(a=x) == NOT((NOT F)(a=x)).
### Side effects
    This is not Coudert and Madre's restrict function.
    Use Biddy_Simplify if you need that one.
    Cache table is not used, yet.
### More info
    Macro Biddy_Restrict(f,v,value) is defined for use with anonymous manager.
*******************************************************************************/

#ifdef __cplusplus
extern "C" {
#endif

Biddy_Edge
Biddy_Managed_Restrict(Biddy_Manager MNG, Biddy_Edge f, Biddy_Variable v,
                       Biddy_Edge value)
{
  Biddy_Edge e, t, r;
  Biddy_Variable fv;

  if (!MNG) MNG = biddyAnonymousManager;

  if (!f) {
    fprintf(stdout,"ERROR (Biddy_Managed_Restrict): f = NULL!\n");
    free((void *)1); /* BREAKPOINT */
  }

  if (!BiddyIsOK(f)) {
    fprintf(stdout,"ERROR (Biddy_Managed_Restrict): Bad f\n");
    free((void *)1); /* BREAKPOINT */
  }

  if (Biddy_IsConstant(f)) return f;

  if ((fv=Biddy_GetTopVariable(f)) == v) {
    if (Biddy_IsEqv(value,biddyOne)) {
      return Biddy_NotCond(BiddyT(f),Biddy_GetMark(f));
    } else {
      return Biddy_NotCond(BiddyE(f),Biddy_GetMark(f));
    }
  }

  if (Biddy_Managed_IsSmaller(MNG,v,fv)) return f;

  e = Biddy_Managed_Restrict(MNG,BiddyE(f),v,value);
  t = Biddy_Managed_Restrict(MNG,BiddyT(f),v,value);
  r = Biddy_NotCond(Biddy_Managed_FoaNode(MNG,fv,e,t,TRUE),Biddy_GetMark(f));
  BiddyRefresh(MNG,r); /* FoaNode will not refresh function! */

  return r;
}

#ifdef __cplusplus
}
#endif

/***************************************************************************//*!
\brief Function Biddy_Managed_Compose calculates a composition of two Boolean
       functions.

### Description
    It uses optimization: F(a=G) == NOT((NOT F)(a=G)).
### Side effects
    Cache table is not used, yet.
### More info
    Macro Biddy_Compose(f,v,g) is defined for use with anonymous manager.
*******************************************************************************/

#ifdef __cplusplus
extern "C" {
#endif

Biddy_Edge
Biddy_Managed_Compose(Biddy_Manager MNG, Biddy_Edge f, Biddy_Variable v,
                      Biddy_Edge g)
{
  Biddy_Edge e, t, r;
  Biddy_Variable fv;

  if (!MNG) MNG = biddyAnonymousManager;

  if (!f) {
    fprintf(stdout,"ERROR (Biddy_Managed_Compose): f = NULL!\n");
    free((void *)1); /* BREAKPOINT */
  }

  if (!g) {
    fprintf(stdout,"ERROR (Biddy_Managed_Compose): g = NULL!\n");
    free((void *)1); /* BREAKPOINT */
  }

  if (!BiddyIsOK(f)) {
    fprintf(stdout,"ERROR (Biddy_Managed_Compose): Bad f\n");
    free((void *)1); /* BREAKPOINT */
  }

  if (!BiddyIsOK(g)) {
    fprintf(stdout,"ERROR (Biddy_Managed_Compose): Bad g\n");
    free((void *)1); /* BREAKPOINT */
  }

  if (Biddy_IsConstant(f)) return f;

  if ((fv=Biddy_GetTopVariable(f)) == v) {
    return Biddy_NotCond(Biddy_Managed_ITE(MNG,g,BiddyT(f),BiddyE(f)),Biddy_GetMark(f));
  }

  if (Biddy_Managed_IsSmaller(MNG,v,fv)) return f;

  e = Biddy_Managed_Compose(MNG,BiddyE(f),v,g);
  t = Biddy_Managed_Compose(MNG,BiddyT(f),v,g);
  r = Biddy_NotCond(Biddy_Managed_FoaNode(MNG,fv,e,t,TRUE),Biddy_GetMark(f));
  BiddyRefresh(MNG,r); /* FoaNode will not refresh function! */

  return r;
}

#ifdef __cplusplus
}
#endif

/***************************************************************************//*!
\brief Function Biddy_Managed_E calculates an existential quantification of
       Boolean function.

### Description
    Be careful: ExA F != NOT(ExA (NOT F)).
    Counterexample: Exb (AND (NOT a) b c).
### Side effects
    Cache table is not used, yet.
### More info
    Macro Biddy_E(f,v) is defined for use with anonymous manager.
*******************************************************************************/

#ifdef __cplusplus
extern "C" {
#endif

Biddy_Edge
Biddy_Managed_E(Biddy_Manager MNG, Biddy_Edge f, Biddy_Variable v)
{
  Biddy_Edge e, t, r;
  Biddy_Variable fv;

  if (!MNG) MNG = biddyAnonymousManager;

  if (!f) {
    fprintf(stdout,"ERROR (Biddy_Managed_E): f = NULL!\n");
    free((void *)1); /* BREAKPOINT */
  }

  if (!BiddyIsOK(f)) {
    fprintf(stdout,"ERROR (Biddy_Managed_E): Bad f\n");
    free((void *)1); /* BREAKPOINT */
  }

  /* LOOKING FOR TERMINAL CASE */
  if (Biddy_IsConstant(f)) return f;

  /* THE USAGE OF EAX CACHE IS NOT IMPLEMENTED, YET */

  if ((fv=Biddy_GetTopVariable(f)) == v) {
    return Biddy_Managed_ITE(MNG,
                     Biddy_NotCond(BiddyT(f),Biddy_GetMark(f)),
                     biddyOne,
                     Biddy_NotCond(BiddyE(f),Biddy_GetMark(f)));
  }

  if (Biddy_Managed_IsSmaller(MNG,v,fv)) return f;

  e = Biddy_Managed_E(MNG,Biddy_NotCond(BiddyE(f),Biddy_GetMark(f)),v);
  t = Biddy_Managed_E(MNG,Biddy_NotCond(BiddyT(f),Biddy_GetMark(f)),v);
  r = Biddy_Managed_ITE(MNG,Biddy_Managed_FoaNode(MNG,fv,biddyZero,biddyOne,TRUE),t,e);
  BiddyRefresh(MNG,r); /* FoaNode will not refresh function! */

  return r;
}

#ifdef __cplusplus
}
#endif

/***************************************************************************//*!
\brief Function Biddy_Managed_A calculates an universal quantification of
       Boolean function.

### Description
    Be careful: AxA F != NOT(AxA (NOT F)).
    Counterexample: Axb (AND (NOT a) b c).
### Side effects
    Cache table is not used, yet.
### More info
    Macro Biddy_A(f,v) is defined for use with anonymous manager.
*******************************************************************************/

#ifdef __cplusplus
extern "C" {
#endif

Biddy_Edge
Biddy_Managed_A(Biddy_Manager MNG, Biddy_Edge f, Biddy_Variable v)
{
  Biddy_Edge e, t, r;
  Biddy_Variable fv;

  if (!MNG) MNG = biddyAnonymousManager;

  if (!f) {
    fprintf(stdout,"ERROR (Biddy_Managed_A): f = NULL!\n");
    free((void *)1); /* BREAKPOINT */
  }

  if (!BiddyIsOK(f)) {
    fprintf(stdout,"ERROR (Biddy_Managed_A): Bad f\n");
    free((void *)1); /* BREAKPOINT */
  }

  /* LOOKING FOR TERMINAL CASE */
  if (Biddy_IsConstant(f)) return f;

  /* THE USAGE OF EAX CACHE IS NOT IMPLEMENTED, YET */

  if ((fv=Biddy_GetTopVariable(f)) == v) {
    return Biddy_Managed_ITE(MNG,
                     Biddy_NotCond(BiddyT(f),Biddy_GetMark(f)),
                     Biddy_NotCond(BiddyE(f),Biddy_GetMark(f)),
                     biddyZero);
  }

  if (Biddy_Managed_IsSmaller(MNG,v,fv)) return f;

  e = Biddy_Managed_A(MNG,Biddy_NotCond(BiddyE(f),Biddy_GetMark(f)),v);
  t = Biddy_Managed_A(MNG,Biddy_NotCond(BiddyT(f),Biddy_GetMark(f)),v);
  r = Biddy_Managed_ITE(MNG,Biddy_Managed_FoaNode(MNG,fv,biddyZero,biddyOne,TRUE),t,e);
  BiddyRefresh(MNG,r); /* FoaNode will not refresh function! */

  return r;
}

#ifdef __cplusplus
}
#endif

/***************************************************************************//*!
\brief Function Biddy_Managed_IsVariableDependent returns TRUE iff variable is
       dependent on others in a function.

### Description
    A variable is dependent on others in a function iff universal
    quantification of this variable returns constant FALSE.
### Side effects
    Implemented by calculating full universal quantification
    which is less efficient as direct implementation in CUDD.
### More info
    Macro Biddy_IsVariableDependent(f,v) is defined for use with anonymous
    manager.
*******************************************************************************/

#ifdef __cplusplus
extern "C" {
#endif

Biddy_Boolean
Biddy_Managed_IsVariableDependent(Biddy_Manager MNG, Biddy_Edge f, Biddy_Variable v)
{
  Biddy_Edge r;

  if (!MNG) MNG = biddyAnonymousManager;

  r = Biddy_Managed_A(MNG,f,v);
  return Biddy_IsEqv(r,biddyZero);
}

#ifdef __cplusplus
}
#endif

/***************************************************************************//*!
\brief Function Biddy_Managed_ExistAbstract existentially abstracts all the
       variables in cube from f.

### Description
### Side effects
    Cache table is not implemented, yet.
### More info
    Macro Biddy_ExistAbstract(f,cube) is defined for use with anonymous manager.
*******************************************************************************/

#ifdef __cplusplus
extern "C" {
#endif

Biddy_Edge
Biddy_Managed_ExistAbstract(Biddy_Manager MNG, Biddy_Edge f, Biddy_Edge cube)
{
  Biddy_Edge e, t, r;
  Biddy_Variable fv,cv;

  if (!MNG) MNG = biddyAnonymousManager;

  if (!f) {
    fprintf(stdout,"ERROR (Biddy_Managed_ExistAbstract): f = NULL!\n");
    free((void *)1); /* BREAKPOINT */
  }

  if (!BiddyIsOK(f)) {
    fprintf(stdout,"ERROR (Biddy_Managed_ExistAbstract): Bad f\n");
    free((void *)1); /* BREAKPOINT */
  }

  /* LOOKING FOR TERMINAL CASE */
  if (Biddy_IsConstant(f)) return f;

  fv = Biddy_GetTopVariable(f);
  cv = Biddy_GetTopVariable(cube);

  while (!Biddy_IsConstant(cube) && Biddy_Managed_IsSmaller(MNG,cv,fv)) {
    cube = BiddyT(cube);
  }
  if (Biddy_IsConstant(cube)) return f;

  if (cv == fv) {
    
    e = Biddy_Managed_ExistAbstract(MNG,
          Biddy_NotCond(BiddyE(f),Biddy_GetMark(f)),BiddyT(cube));
    t = Biddy_Managed_ExistAbstract(MNG,
          Biddy_NotCond(BiddyT(f),Biddy_GetMark(f)),BiddyT(cube));
    r = Biddy_Managed_ITE(MNG,t,biddyOne,e);

  } else {

    e = Biddy_Managed_ExistAbstract(MNG,
          Biddy_NotCond(BiddyE(f),Biddy_GetMark(f)),cube);
    t = Biddy_Managed_ExistAbstract(MNG,
          Biddy_NotCond(BiddyT(f),Biddy_GetMark(f)),cube);
    r = Biddy_Managed_FoaNode(MNG,fv,biddyZero,biddyOne,TRUE);
    BiddyRefresh(MNG,r); /* FoaNode will not refresh function! */
    r = Biddy_Managed_ITE(MNG,r,t,e);

  }

  return r;
}

#ifdef __cplusplus
}
#endif

/***************************************************************************//*!
\brief Function Biddy_Managed_UnivAbstract universally abstracts all the
       variables in cube from f.

### Description
### Side effects
    Cache table is not implemented, yet.
### More info
    Macro Biddy_UnivAbstract(f,cube) is defined for use with anonymous manager.
*******************************************************************************/

#ifdef __cplusplus
extern "C" {
#endif

Biddy_Edge
Biddy_Managed_UnivAbstract(Biddy_Manager MNG, Biddy_Edge f, Biddy_Edge cube)
{
  Biddy_Edge e, t, r;
  Biddy_Variable fv,cv;

  if (!MNG) MNG = biddyAnonymousManager;

  if (!f) {
    fprintf(stdout,"ERROR (Biddy_Managed_UnivAbstract): f = NULL!\n");
    free((void *)1); /* BREAKPOINT */
  }

  if (!BiddyIsOK(f)) {
    fprintf(stdout,"ERROR (Biddy_Managed_UnivAbstract): Bad f\n");
    free((void *)1); /* BREAKPOINT */
  }

  /* LOOKING FOR TERMINAL CASE */
  if (Biddy_IsConstant(f)) return f;

  fv = Biddy_GetTopVariable(f);
  cv = Biddy_GetTopVariable(cube);

  while (!Biddy_IsConstant(cube) && Biddy_Managed_IsSmaller(MNG,cv,fv)) {
    cube = BiddyT(cube);
  }
  if (Biddy_IsConstant(cube)) return f;

  if (cv == fv) {
    
    e = Biddy_Managed_UnivAbstract(MNG,
          Biddy_NotCond(BiddyE(f),Biddy_GetMark(f)),BiddyT(cube));
    t = Biddy_Managed_UnivAbstract(MNG,
          Biddy_NotCond(BiddyT(f),Biddy_GetMark(f)),BiddyT(cube));
    r = Biddy_Managed_ITE(MNG,t,e,biddyZero);

  } else {

    e = Biddy_Managed_UnivAbstract(MNG,
          Biddy_NotCond(BiddyE(f),Biddy_GetMark(f)),cube);
    t = Biddy_Managed_UnivAbstract(MNG,
          Biddy_NotCond(BiddyT(f),Biddy_GetMark(f)),cube);
    r = Biddy_Managed_FoaNode(MNG,fv,biddyZero,biddyOne,TRUE);
    BiddyRefresh(MNG,r); /* FoaNode will not refresh function! */
    r = Biddy_Managed_ITE(MNG,r,t,e);

  }

  return r;
}

#ifdef __cplusplus
}
#endif

/***************************************************************************//*!
\brief Function Biddy_Managed_AndAbstract calculates the AND of two BDDs and
       simultaneously (existentially) abstracts the variables in cube.

### Description
### Side effects
    Cache table is not implemented, yet.
### More info
    Macro Biddy_AndAbstract(f,g,cube) is defined for use with anonymous manager.
*******************************************************************************/

#ifdef __cplusplus
extern "C" {
#endif

Biddy_Edge
Biddy_Managed_AndAbstract(Biddy_Manager MNG, Biddy_Edge f, Biddy_Edge g,
                          Biddy_Edge cube)
{
  Biddy_Edge f0,f1,g0,g1;
  Biddy_Edge e, t, r;
  Biddy_Variable fv,gv,minv,cv;

  if (!MNG) MNG = biddyAnonymousManager;

  if (!f) {
    fprintf(stdout,"ERROR (Biddy_Managed_AndAbstract): f = NULL!\n");
    free((void *)1); /* BREAKPOINT */
  }

  if (!g) {
    fprintf(stdout,"ERROR (Biddy_Managed_AndAbstract): g = NULL!\n");
    free((void *)1); /* BREAKPOINT */
  }

  if (!BiddyIsOK(f)) {
    fprintf(stdout,"ERROR (Biddy_Managed_AndAbstract): Bad f\n");
    free((void *)1); /* BREAKPOINT */
  }

  if (!BiddyIsOK(g)) {
    fprintf(stdout,"ERROR (Biddy_Managed_AndAbstract): Bad g\n");
    free((void *)1); /* BREAKPOINT */
  }

  /* LOOKING FOR TERMINAL CASE */
  if (Biddy_IsEqv(f,biddyZero) || Biddy_IsEqv(g,biddyZero)) {
    return biddyZero;
  }
  if (Biddy_IsEqv(f,biddyOne) && Biddy_IsEqv(g,biddyOne)) {
    return biddyOne;
  }

  fv = Biddy_GetTopVariable(f);
  gv = Biddy_GetTopVariable(g);

  if (Biddy_Managed_IsSmaller(MNG,fv,gv)) {
    f0 = Biddy_NotCond(BiddyE(f),Biddy_GetMark(f));
    f1 = Biddy_NotCond(BiddyT(f),Biddy_GetMark(f));
    g0 = g;
    g1 = g;
    minv = fv;
  } else if (Biddy_Managed_IsSmaller(MNG,gv,fv)) {
    f0 = f;
    f1 = f;
    g0 = Biddy_NotCond(BiddyE(g),Biddy_GetMark(g));
    g1 = Biddy_NotCond(BiddyT(g),Biddy_GetMark(g));
    minv = gv;
  } else {
    f0 = Biddy_NotCond(BiddyE(f),Biddy_GetMark(f));
    f1 = Biddy_NotCond(BiddyT(f),Biddy_GetMark(f));
    g0 = Biddy_NotCond(BiddyE(g),Biddy_GetMark(g));
    g1 = Biddy_NotCond(BiddyT(g),Biddy_GetMark(g));
    minv = fv;
  }
  
  cv = Biddy_GetTopVariable(cube);

  while (!Biddy_IsConstant(cube) && Biddy_Managed_IsSmaller(MNG,cv,minv)) {
    cube = BiddyT(cube);
  }

  if (Biddy_IsConstant(cube)) {
    return Biddy_Managed_ITE(MNG,f,g,biddyZero);
  }

  if (minv == cv) {

    /* Tricky optimizations are from: */
    /* B. Yang et al. A Performance Study of BDD-Based Model Checking. 1998. */
    /* http://fmv.jku.at/papers/YangEtAl-FMCAD98.pdf */

    e = Biddy_Managed_AndAbstract(MNG,f0,g0,BiddyT(cube));
    if (Biddy_IsEqv(e,biddyOne)) return biddyOne;
    if (Biddy_IsEqv(e,f1) || Biddy_IsEqv(e,g1)) return e;
    if (Biddy_IsEqv(e,Biddy_Not(f1))) f1 = biddyOne;
    if (Biddy_IsEqv(e,Biddy_Not(g1))) g1 = biddyOne;
    t = Biddy_Managed_AndAbstract(MNG,f1,g1,BiddyT(cube));
    r = Biddy_Managed_ITE(MNG,t,biddyOne,e);

  } else {

    e = Biddy_Managed_AndAbstract(MNG,f0,g0,cube);
    t = Biddy_Managed_AndAbstract(MNG,f1,g1,cube);
    r = Biddy_Managed_FoaNode(MNG,minv,biddyZero,biddyOne,TRUE);
    BiddyRefresh(MNG,r); /* FoaNode will not refresh function! */
    r = Biddy_Managed_ITE(MNG,r,t,e);

  }

  return r;
}

#ifdef __cplusplus
}
#endif

/***************************************************************************//*!
\brief Function Biddy_Managed_Constrain calculates Coudert and Madre's constrain
       function.

### Description
    Coudert and Madre's constrain function is also called a generalized cofactor
    of function f with respect to function c.
### Side effects
    Cache table is not implemented, yet.
### More info
    Macro Biddy_Constrain(f,c) is defined for use with anonymous manager.
*******************************************************************************/

/* SOME REFERENCES FOR Biddy_Constrain AND Biddy_Simplify */
/* O. Coudert et al. Verification of synchronous sequential machines based on symbolic execution, 1989 */
/* O. Coudert et al. A unified framework for the formal verification of sequential circuits, 1990 */
/* B. Lin et al.  Don't care minimization of multi-level sequential logic networks. 1990. */
/* Thomas R. Shiple et al. Heuristic minimization of BDDs using don't cares, 1994 */
/* Mark D. Aagaard. et al. Formal verification using parametric representations of Boolean constraints, 1999 */

#ifdef __cplusplus
extern "C" {
#endif

Biddy_Edge
Biddy_Managed_Constrain(Biddy_Manager MNG, Biddy_Edge f, Biddy_Edge c)
{
  Biddy_Edge f0,f1,c0,c1;
  Biddy_Edge e, t, r;
  Biddy_Variable fv,cv,minv;

  if (!MNG) MNG = biddyAnonymousManager;

  if (!f) {
    fprintf(stdout,"ERROR (Biddy_Managed_Constrain): f = NULL!\n");
    free((void *)1); /* BREAKPOINT */
  }

  if (!c) {
    fprintf(stdout,"ERROR (Biddy_Managed_Constrain): c = NULL!\n");
    free((void *)1); /* BREAKPOINT */
  }

  if (!BiddyIsOK(f)) {
    fprintf(stdout,"ERROR (Biddy_Managed_Constrain): Bad f\n");
    free((void *)1); /* BREAKPOINT */
  }

  if (!BiddyIsOK(c)) {
    fprintf(stdout,"ERROR (Biddy_Managed_Constrain): Bad c\n");
    free((void *)1); /* BREAKPOINT */
  }

  /* LOOKING FOR TERMINAL CASE */
  if (Biddy_IsEqv(c,biddyZero)) return biddyZero;
  if (Biddy_IsEqv(c,biddyOne)) return f;
  if (Biddy_IsConstant(f)) return f;

  fv = Biddy_GetTopVariable(f);
  cv = Biddy_GetTopVariable(c);

  if (Biddy_Managed_IsSmaller(MNG,fv,cv)) {
    f0 = Biddy_NotCond(BiddyE(f),Biddy_GetMark(f));
    f1 = Biddy_NotCond(BiddyT(f),Biddy_GetMark(f));
    c0 = c;
    c1 = c;
    minv = fv;
  } else if (Biddy_Managed_IsSmaller(MNG,cv,fv)) {
    f0 = f;
    f1 = f;
    c0 = Biddy_NotCond(BiddyE(c),Biddy_GetMark(c));
    c1 = Biddy_NotCond(BiddyT(c),Biddy_GetMark(c));
    minv = cv;
  } else {
    f0 = Biddy_NotCond(BiddyE(f),Biddy_GetMark(f));
    f1 = Biddy_NotCond(BiddyT(f),Biddy_GetMark(f));
    c0 = Biddy_NotCond(BiddyE(c),Biddy_GetMark(c));
    c1 = Biddy_NotCond(BiddyT(c),Biddy_GetMark(c));
    minv = cv;
  }

  if (Biddy_IsEqv(c0,biddyZero)) return Biddy_Managed_Constrain(MNG,f1,c1);
  if (Biddy_IsEqv(c1,biddyZero)) return Biddy_Managed_Constrain(MNG,f0,c0);

  e = Biddy_Managed_Constrain(MNG,f0,c0);
  t = Biddy_Managed_Constrain(MNG,f1,c1);
  r = Biddy_Managed_FoaNode(MNG,minv,biddyZero,biddyOne,TRUE);
  BiddyRefresh(MNG,r); /* FoaNode will not refresh function! */
  r = Biddy_Managed_ITE(MNG,r,t,e);

  return r;
}

#ifdef __cplusplus
}
#endif

/***************************************************************************//*!
\brief Function Biddy_Managed_Simplify calculates Coudert and Madre's restrict
       function.

### Description
    Coudert and Madre's restrict function tries to simplify function f by
    restricting it to the domain covered by function c.
### Side effects
    No checks are done to see if the result is actually smaller than the input.
    Cache table is not implemented, yet.
### More info
    Macro Biddy_Simplify(f,c) is defined for use with anonymous manager.
*******************************************************************************/

#ifdef __cplusplus
extern "C" {
#endif

Biddy_Edge
Biddy_Managed_Simplify(Biddy_Manager MNG, Biddy_Edge f, Biddy_Edge c)
{
  Biddy_Edge f0,f1,c0,c1;
  Biddy_Edge e, t, r;
  Biddy_Variable fv,cv,minv;

  if (!MNG) MNG = biddyAnonymousManager;

  if (!f) {
    fprintf(stdout,"ERROR (Biddy_Managed_Simplify): f = NULL!\n");
    free((void *)1); /* BREAKPOINT */
  }

  if (!c) {
    fprintf(stdout,"ERROR (Biddy_Managed_Simplify): c = NULL!\n");
    free((void *)1); /* BREAKPOINT */
  }

  if (!BiddyIsOK(f)) {
    fprintf(stdout,"ERROR (Biddy_Managed_Simplify): Bad f\n");
    free((void *)1); /* BREAKPOINT */
  }

  if (!BiddyIsOK(c)) {
    fprintf(stdout,"ERROR (Biddy_Managed_Simplify): Bad c\n");
    free((void *)1); /* BREAKPOINT */
  }

  /* LOOKING FOR TERMINAL CASE */
  if (Biddy_IsEqv(c,biddyZero)) return biddyNull;
  if (Biddy_IsEqv(c,biddyOne)) return f;
  if (Biddy_IsConstant(f)) return f;

  /* THIS IS FROM 20-CS-626-001 */
  /* http://gauss.ececs.uc.edu/Courses/c626/lectures/BDD/bdd-desc.pdf */
  if (Biddy_IsEqv(f,c)) return biddyOne;
  if (Biddy_IsEqv(f,Biddy_Not(c))) return biddyZero;

  fv = Biddy_GetTopVariable(f);
  cv = Biddy_GetTopVariable(c);

  /* SPECIAL CASE: f/~a == f/a */
  if (Biddy_Managed_IsSmaller(MNG,cv,fv)) {
    return Biddy_Managed_Simplify(MNG,f,Biddy_Managed_E(MNG,c,cv));
  }

  if (Biddy_Managed_IsSmaller(MNG,fv,cv)) {
    f0 = Biddy_NotCond(BiddyE(f),Biddy_GetMark(f));
    f1 = Biddy_NotCond(BiddyT(f),Biddy_GetMark(f));
    c0 = c;
    c1 = c;
    minv = fv;
  } else {
    f0 = Biddy_NotCond(BiddyE(f),Biddy_GetMark(f));
    f1 = Biddy_NotCond(BiddyT(f),Biddy_GetMark(f));
    c0 = Biddy_NotCond(BiddyE(c),Biddy_GetMark(c));
    c1 = Biddy_NotCond(BiddyT(c),Biddy_GetMark(c));
    minv = cv;
  }

  if (Biddy_IsEqv(c0,biddyZero)) return Biddy_Managed_Simplify(MNG,f1,c1);
  if (Biddy_IsEqv(c1,biddyZero)) return Biddy_Managed_Simplify(MNG,f0,c0);

  e = Biddy_Managed_Simplify(MNG,f0,c0);
  t = Biddy_Managed_Simplify(MNG,f1,c1);
  r = Biddy_Managed_FoaNode(MNG,minv,biddyZero,biddyOne,TRUE);
  BiddyRefresh(MNG,r); /* FoaNode will not refresh function! */
  r = Biddy_Managed_ITE(MNG,r,t,e);

  return r;
}

#ifdef __cplusplus
}
#endif

/***************************************************************************//*!
\brief Function Biddy_Managed_Support calculates a product of all dependent
       variables.

### Description
### Side effects
### More info
    Macro Biddy_Support(f) is defined for use with anonymous manager.
*******************************************************************************/

#ifdef __cplusplus
extern "C" {
#endif

Biddy_Edge
Biddy_Managed_Support(Biddy_Manager MNG, Biddy_Edge f)
{
  Biddy_Variable v;
  unsigned int n;
  Biddy_Edge s,r;

  if (!MNG) MNG = biddyAnonymousManager;

  if (!f) {
    fprintf(stdout,"ERROR (Biddy_Managed_Support): f = NULL!\n");
    free((void *)1); /* BREAKPOINT */
  }

  if (!BiddyIsOK(f)) {
    fprintf(stdout,"ERROR (Biddy_Managed_Support): Bad f\n");
    free((void *)1); /* BREAKPOINT */
  }

  if (Biddy_IsNull(f)) return biddyNull;
  if (Biddy_IsConstant(f)) return biddyZero;

  for (v=1;v<biddyVariableTable.num;v++) {
    biddyVariableTable.table[v].selected = FALSE;
  }

  n = 1; /* NOT NEEDED, BUT NODES ARE COUNTED BY BiddyNodeVarNumber */
  Biddy_SelectNode(biddyOne); /* needed for BiddyNodeVarNumber */
  BiddyNodeVarNumber(MNG,f,&n); /* VARIABLES ARE MARKED */
  Biddy_DeselectNode(biddyOne); /* needed for Biddy_NodeRepair */
  Biddy_NodeRepair(f);

  s = biddyOne;
  for (v=1;v<biddyVariableTable.num;v++) {
    if (biddyVariableTable.table[v].selected == TRUE) {
      r = Biddy_Managed_FoaNode(MNG,v,biddyZero,biddyOne,FALSE);
      BiddyRefresh(MNG,r); /* FoaNode will not refresh function! */
      s = Biddy_Managed_ITE(MNG,s,r,biddyZero);
      biddyVariableTable.table[v].selected = FALSE;
    }
  }

  return s;
}

#ifdef __cplusplus
}
#endif

/***************************************************************************//*!
\brief Function Biddy_Managed_Replace calculates BDD with one or more variables
       replaced.

### Description
    Replacing is controlled by variable's values (which are edges!).
    Use Biddy_Managed_ResetVariablesValue and Biddy_Managed_SetVariableValue
    to prepare replacing. Current and new variables should be disjoint sets.
### Side effects
    Cache table is not implemented, yet.
### More info
    Macro Biddy_Replace(f) is defined for use with anonymous manager.
*******************************************************************************/

#ifdef __cplusplus
extern "C" {
#endif

Biddy_Edge
Biddy_Managed_Replace(Biddy_Manager MNG, Biddy_Edge f)
{
  Biddy_Edge e,t,r;
  Biddy_Variable fv;

  if (!MNG) MNG = biddyAnonymousManager;

  if (!f) {
    fprintf(stdout,"ERROR (Biddy_Managed_Replace): f = NULL!\n");
    free((void *)1); /* BREAKPOINT */
  }

  if (!BiddyIsOK(f)) {
    fprintf(stdout,"ERROR (Biddy_Managed_Replace): Bad f\n");
    free((void *)1); /* BREAKPOINT */
  }

  if (Biddy_IsConstant(f)) return f;

  fv=Biddy_GetTopVariable(f);
  if (!Biddy_IsEqv((r=biddyVariableTable.table[fv].value),biddyZero)) {
    fv = Biddy_GetTopVariable(r);

    /* DEBUGGING */
    /*
    fprintf(stdout,"REPLACE: %s(%u) -> %s(%u)\n",
            Biddy_Managed_GetTopVariableName(MNG,f),
            Biddy_GetTopVariable(f),
            Biddy_Managed_GetVariableName(MNG,fv),
            fv
    );
    */
  }

  e = Biddy_Managed_Replace(MNG,Biddy_NotCond(BiddyE(f),Biddy_GetMark(f)));
  t = Biddy_Managed_Replace(MNG,Biddy_NotCond(BiddyT(f),Biddy_GetMark(f)));
  r = Biddy_Managed_FoaNode(MNG,fv,biddyZero,biddyOne,TRUE);
  BiddyRefresh(MNG,r); /* FoaNode will not refresh function! */
  r = Biddy_Managed_ITE(MNG,r,t,e);

  return r;
}

#ifdef __cplusplus
}
#endif

/***************************************************************************//*!
\brief Function Biddy_Managed_IsOK returns TRUE iff given node is not obsolete.

### Description
    External use, only.
### Side effects
### More info
    Macro BiddyIsOK(f) is defined for internal use.
    Macro Biddy_IsOK(f) is defined for use with anonymous manager.
*******************************************************************************/

#ifdef __cplusplus
extern "C" {
#endif

Biddy_Boolean
Biddy_Managed_IsOK(Biddy_Manager MNG, Biddy_Edge f)
{
  int c;

  if (!MNG) MNG = biddyAnonymousManager;

  return
    (((c = ((BiddyNode *) Biddy_Regular(f))->count) == 0) ||
     (c >= biddyCount));
}

#ifdef __cplusplus
}
#endif

/***************************************************************************//*!
\brief Function Biddy_Managed_Garbage performs garbage collection.

### Description
    Part of List of new nodes is checked (until freshNodes), bad nodes are
    deleted whilst fortified nodes in this part are moved to List of fortified
    nodes.
### Side effects
    The first element of each chain in a node table should have a special value
    for its 'prev' element to allow tricky but efficient deleting. Moreover,
    'prev' and 'next' should be the first and the second element in the
    structure BiddyNode, respectively. Not all fortified nodes are moved to
    List of fortified nodes, the ones after 'freshNode' remains! Garbage
    collecion is reported by biddyNodeTable.garbage only if some bad nodes are
    purged!
### More info
    Macro Biddy_Garbage() is defined for use with anonymous manager.
*******************************************************************************/

#ifdef __cplusplus
extern "C" {
#endif

void
Biddy_Managed_Garbage(Biddy_Manager MNG)
{
  BiddyNode *first;
  BiddyNode *sup;
  BiddyCacheList *c;
  Biddy_Boolean cacheOK;
  Biddy_Variable v;

  if (!MNG) MNG = biddyAnonymousManager;

  /*
  {
  static int n = 0;
  fprintf(stdout,"Garbage (%d), count = %u, there are %u BDD nodes...\n",++n,biddyCount,biddyNodeTable.num);
  }
  */

  cacheOK = TRUE;
  for (v=1; v<biddyVariableTable.num; v++) {

    /* BE CAREFULLY WITH TYPE BiddyNode !!!! */
    /* THIS COMPUTATION USE SOME TRICKS! */
    /* FIELDS 'prev' AND 'next' HAVE TO BE THE FIRST AND THE SECOND! */

    if (biddyVariableTable.table[v].firstNewNode !=
        biddyVariableTable.table[v].freshNodes) {

      /* UPDATING NODE TABLE */
      /* After initialization the only element in the list is a fortified */
      /* constant node. Because List of New Nodes must not be empty */
      /* this element is never moved to the list of fortified nodes. */
      first = biddyVariableTable.table[v].firstNewNode;
      biddyVariableTable.table[v].firstNewNode = (BiddyNode *) biddyVariableTable.table[v].firstNewNode->list;
      while (biddyVariableTable.table[v].firstNewNode !=
             biddyVariableTable.table[v].freshNodes) {
        if (!biddyVariableTable.table[v].firstNewNode->count) { /* fortified node */
          sup = biddyVariableTable.table[v].firstNewNode;
          biddyVariableTable.table[v].firstNewNode =
            (BiddyNode *) biddyVariableTable.table[v].firstNewNode->list;
          sup->list = (void *) biddyVariableTable.table[v].fortifiedNodes;
          biddyVariableTable.table[v].fortifiedNodes = sup;
        } else {
          if (biddyVariableTable.table[v].firstNewNode->count >= biddyCount) { /* fresh or prolonged node */
            biddyVariableTable.table[v].lastNewNode->list = (void *) biddyVariableTable.table[v].firstNewNode;
            biddyVariableTable.table[v].lastNewNode = biddyVariableTable.table[v].firstNewNode;
            biddyVariableTable.table[v].firstNewNode = (BiddyNode *) biddyVariableTable.table[v].firstNewNode->list;
          } else { /* obsolete node */
            cacheOK = FALSE;
            biddyNodeTable.num--;
            (biddyVariableTable.table[v].num)--;
            /* update node table chain */
            biddyVariableTable.table[v].firstNewNode->prev->next =
              biddyVariableTable.table[v].firstNewNode->next;
            if (biddyVariableTable.table[v].firstNewNode->next) {
              biddyVariableTable.table[v].firstNewNode->next->prev =
                biddyVariableTable.table[v].firstNewNode->prev;
            }
            /* update lists */
            sup = (BiddyNode *) biddyVariableTable.table[v].firstNewNode->list;
            biddyVariableTable.table[v].firstNewNode->list = (void *) biddyFreeNodes;
            biddyFreeNodes = biddyVariableTable.table[v].firstNewNode;
            biddyVariableTable.table[v].firstNewNode = sup;
          }
        }
      }

      /* NOW, FIRST ELEMENT MUST BE BACK TO THE LIST */
      /* BUT, ELEMENT CURRENTLY REFERENCED BY firstNewNode */
      /* MAY BE OBSOLETE (if there were no fresh nodes in the list) */
      /* if it is obsolete it must be removed */

      if ((biddyVariableTable.table[v].firstNewNode->count) &&
          (biddyVariableTable.table[v].firstNewNode->count < biddyCount))
      {
        
        /* ELEMENT REFERENCED BY firstNewNode is OBSOLETE */

        cacheOK = FALSE;
        biddyNodeTable.num--;
        (biddyVariableTable.table[v].num)--;
        /* update node table chain */
        biddyVariableTable.table[v].firstNewNode->prev->next =
          biddyVariableTable.table[v].firstNewNode->next;
        if (biddyVariableTable.table[v].firstNewNode->next) {
          biddyVariableTable.table[v].firstNewNode->next->prev =
            biddyVariableTable.table[v].firstNewNode->prev;
        }
        /* update list */
        if (biddyVariableTable.table[v].firstNewNode ==
              biddyVariableTable.table[v].lastNewNode)
        {
          /* this is the only one element in the list */
          biddyVariableTable.table[v].firstNewNode->list = (void *) biddyFreeNodes;
          biddyFreeNodes = biddyVariableTable.table[v].firstNewNode;
          biddyVariableTable.table[v].firstNewNode =
            biddyVariableTable.table[v].freshNodes =
              biddyVariableTable.table[v].lastNewNode = first;
        } else {
          /* there are many elements in the list */
          first->list = (BiddyNode *) biddyVariableTable.table[v].firstNewNode->list;
          biddyVariableTable.table[v].firstNewNode->list = (void *) biddyFreeNodes;
          biddyFreeNodes = biddyVariableTable.table[v].firstNewNode;
          biddyVariableTable.table[v].firstNewNode =
            biddyVariableTable.table[v].freshNodes = first;
        }

      } else {

        /* ELEMENT REFERENCED BY firstNewNode is NOT OBSOLETE */

        first->list = (BiddyNode *) biddyVariableTable.table[v].firstNewNode;
        biddyVariableTable.table[v].firstNewNode =
          biddyVariableTable.table[v].freshNodes = first;

      }

      /* DEBUG */
      /*
      sup = biddyVariableTable.table[v].firstNewNode;
      if (!Biddy_IsEqv(sup->f,biddyZero) || !Biddy_IsEqv(sup->t,biddyOne)) {
        fprintf(stderr,"WARNING (Biddy_Managed_Garbage): bad first element in the list\n");
        free((void *)1);
      }
      if ((sup->count) && (sup->count < biddyCount))
      {
        printf("WARNING (Biddy_Managed_Garbage): obsolete nodes not completely removed!\n");
      }
      */

    }

  }

  /* Updating cache tables - only if needed */
  if (!cacheOK) {
    biddyNodeTable.garbage++; /* if some bad nodes are purged, only! */
    c = biddyCacheList;
    while (c) {
      c->gc(MNG);
      c = c->next;
    }
  }
}

#ifdef __cplusplus
}
#endif

/***************************************************************************//*!
\brief Function Biddy_Managed_Clean performs cleaning.

### Description
    Discard all nodes which are not preserved or which are not preserved
    anymore. Obsolete nodes are not immediately removed, they will be removed
    during the first garbage collection.
### Side effects
    Tag deleted is not considered and thus no fortified node is removed.
### More info
    Macro Biddy_Clean() is defined for use with anonymous manager.
*******************************************************************************/

#ifdef __cplusplus
extern "C" {
#endif

void
Biddy_Managed_Clean(Biddy_Manager MNG)
{
  Biddy_Variable v;
  BiddyNode *sup;
  unsigned int i,j,n;

  if (!MNG) MNG = biddyAnonymousManager;

  /* 1. DISCARD ALL NODES WHICH ARE NOT PRESERVED */
  /* implemented by simple increasing BiddyCount */

  BiddyIncCounter(MNG);

  /* 2. REMOVE ALL FORMULAE WHICH ARE NOT PRESERVED ANYMORE */
  /* the first two formulae ("0" and "1") are never deleted */

  i = 0;
  for (j = 0; j < biddyFormulaTable.size; j++) {
    if (!biddyFormulaTable.table[j].count || biddyFormulaTable.table[j].count > biddyCount) {
      if (i != j) {
        biddyFormulaTable.table[i] = biddyFormulaTable.table[j];
      }
      if (biddyFormulaTable.table[i].name) biddyFormulaTable.numOrdered = i+1;
      i++;
    } else {
      if (biddyFormulaTable.table[j].name) free(biddyFormulaTable.table[j].name);
    }
  }
  if (i != biddyFormulaTable.size) {
    biddyFormulaTable.size = i;
    if (!(biddyFormulaTable.table = (BiddyFormula *)
       realloc(biddyFormulaTable.table,biddyFormulaTable.size*sizeof(BiddyFormula))))
    {
      fprintf(stderr,"Biddy_Managed_Clean: Out of memoy!\n");
      exit(1);
    }
  }

}

#ifdef __cplusplus
}
#endif


/***************************************************************************//*!
\brief Function Biddy_Managed_ClearAll performs exhaustive garbage collection.

### Description
    Immediately remove all nodes except the permanently preserved ones
    which were not made obsolete by Biddy_DeleteFormula.
### Side effects
### More info
    Macro Biddy_ClearAll() is defined for use with anonymous manager.
*******************************************************************************/

#ifdef __cplusplus
extern "C" {
#endif

void
Biddy_Managed_ClearAll(Biddy_Manager MNG)
{
  Biddy_Variable v;
  BiddyNode *sup;
  unsigned int i,j,n;

  if (!MNG) MNG = biddyAnonymousManager;

  /* DEBUGGING */
  /*
  printf("Biddy_Managed_ClearAll START biddyFormulaTable.size = %u, numf=%u\n",
         biddyFormulaTable.size,biddyNodeTable.numf);
  */

  /* 1. REMOVE NON-PERMANENTLY PRESERVED AND DELETED FORMULAE */
  /* the first two formulae ("0" and "1") are never deleted */

  i = 0;
  for (j = 0; j < biddyFormulaTable.size; j++) {
    if (!biddyFormulaTable.table[j].count && !biddyFormulaTable.table[j].deleted) {
      if (i != j) {
        biddyFormulaTable.table[i] = biddyFormulaTable.table[j];
      }
      if (biddyFormulaTable.table[i].name) biddyFormulaTable.numOrdered = i+1;
      i++;
    } else {
      if (biddyFormulaTable.table[j].name) free(biddyFormulaTable.table[j].name);
    }
  }
  if (i != biddyFormulaTable.size) {
    biddyFormulaTable.size = i;
    if (!(biddyFormulaTable.table = (BiddyFormula *)
       realloc(biddyFormulaTable.table,biddyFormulaTable.size*sizeof(BiddyFormula))))
    {
      fprintf(stderr,"Biddy_Managed_ClearAll: Out of memoy!\n");
      exit(1);
    }
  }

  /* 2. FRESH ALL NODES INCLUDING PROLONGED AND FORTIFIED NODES */

  for (v=1; v<biddyVariableTable.num; v++) {
    biddyVariableTable.table[v].lastNewNode->list =
      (void *)biddyVariableTable.table[v].fortifiedNodes;
    sup = biddyVariableTable.table[v].firstNewNode;
    while (sup) {
      sup->count = biddyCount;
      if (!sup->list) {
        biddyVariableTable.table[v].lastNewNode = sup;
      }
      sup = (BiddyNode *) sup->list;
    }
    biddyVariableTable.table[v].fortifiedNodes = NULL;
  }
  biddyNodeTable.numf = 1;

  /* DEBUGGING */
  /*
  printf("Biddy_Managed_ClearAll STEP2 numf=%u\n",biddyNodeTable.numf);
  */

  /* 3. FORTIFY NODES FROM EXTERNAL FORMULAE */

  for (n = 0; n < biddyFormulaTable.size; n++) {
    if (!Biddy_IsNull(biddyFormulaTable.table[n].f))
    {
      BiddyFortify(MNG,biddyFormulaTable.table[n].f);
    }
  }

  /* DEBUGGING */
  /*
  printf("Biddy_Managed_ClearAll STEP3 numf=%u\n",biddyNodeTable.numf);
  */

  /* 4. REMOVE ALL FRESH NODES */

  BiddyIncCounter(MNG);
  Biddy_Managed_Garbage(MNG);
  biddyCount = 2; /* there are no obsolete, fresh, and prolonged nodes, anymore */

  /* DEBUGGING */
  /*
  printf("Biddy_Managed_ClearAll FINISH biddyFormulaTable.size = %u, numf=%u\n",
         biddyFormulaTable.size,biddyNodeTable.numf);
  */

}

#ifdef __cplusplus
}
#endif

/***************************************************************************//*!
\brief Function Biddy_Managed_AddCache adds cache to the end of Cache list.

### Description
    If Cache list does not exist, function creates it.
### Side effects
### More info
    Macro Biddy_AddCache(gc) is defined for use with anonymous manager.
*******************************************************************************/

#ifdef __cplusplus
extern "C" {
#endif

void
Biddy_Managed_AddCache(Biddy_Manager MNG, Biddy_GCFunction gc)
{
  BiddyCacheList *sup;

  if (!MNG) MNG = biddyAnonymousManager;

  if (!biddyCacheList) {

    if (!(biddyCacheList = (BiddyCacheList *) malloc(sizeof(BiddyCacheList)))) {
      fprintf(stdout,"Biddy_Managed_AddCache: Out of memoy!\n");
      exit(1);
    }

    biddyCacheList->gc = gc;
    biddyCacheList->next = NULL;
  } else {
    sup = biddyCacheList;
    while (sup->next) sup = sup->next;

    if (!(sup->next = (BiddyCacheList *) malloc(sizeof(BiddyCacheList)))) {
      fprintf(stdout,"Biddy_Managed_AddCache: Out of memoy!\n");
      exit(1);
    }

    sup = sup->next;
    sup->gc = gc;
    sup->next = NULL;
  }
}

#ifdef __cplusplus
}
#endif

/***************************************************************************//*!
\brief Function Biddy_Managed_FindFormula find formula in Formula table.

### Description
### Side effects
### More info
    Macro Biddy_FindFormula(x,f) is defined for use with anonymous manager.
*******************************************************************************/

#ifdef __cplusplus
extern "C" {
#endif

Biddy_Boolean
Biddy_Managed_FindFormula(Biddy_Manager MNG, Biddy_String x, Biddy_Edge *f)
{
  int i;
  Biddy_Boolean OK;
  int cc;

  /* TO DO: USE BINARY SEARCH BECAUSE FORMULAE ARE OREDERED BY NAME!! */

  if (!MNG) MNG = biddyAnonymousManager;

  *f = biddyNull;
  if (!biddyFormulaTable.table) return FALSE;
  if (!x) return FALSE;

  OK = FALSE;
  for (i = 0; !OK && (i < biddyFormulaTable.size); i++) {
    if (biddyFormulaTable.table[i].name) {
      cc = strcmp(x,biddyFormulaTable.table[i].name);
      if ((cc == 0) && !biddyFormulaTable.table[i].deleted) {
        OK = TRUE;
        break;
      }
      if (cc < 0) break;
    } else {
      break;
    }
  }

  if (OK) {

     /* element found */
     *f = biddyFormulaTable.table[i].f;

     /* DEBUGGING */
     /*
     printf("\nBiddy_FindFormula: i=%d, x=%s, f=%p\n",i,x,*f);
     */

  } else {

     /* element not found */

     /* DEBUGGING */
     /*
     printf("\nBiddy_FindFormula: %s NOT FOUND!\n",x);
     */

  }

  return OK;
}

#ifdef __cplusplus
}
#endif

/***************************************************************************//*!
\brief Function Biddy_Managed_AddFormula adds formula to Formula table.

### Description
    Nodes of the given BDD will be preserved for the given number of clearings.
    If (name != NULL) then formula is accessible by its name. If (count == 0)
    then formula is persistently preserved and you have to use
    Biddy_DeleteFormula and Biddy_ClearAll to remove its nodes. There are two
    macros defined to simplify formulae management.
    Macro Biddy_Managed_AddTmpFormula(mng,bdd,count) is
    defined as Biddy_Manged_AddFormula(mng,NULL,bdd,count) and macro
    Biddy_Managed_AddPersistentFormula(mng,name,bdd) is defined as
    Biddy_Managed_AddFormula(mng,name,bdd,0).
### Side effects
    Function is prolonged or fortified. Formulae with name are ordered by name.
    If formula with same name already exists, it will be overwritten
    (preserved and persistently preserved formulae, too)! Obsolete nodes
    of preserved and persistently preserved formulae will not be immediately
    removed.
### More info
    Macro Biddy_AddFormula(x,f) is defined for use with anonymous manager.
*******************************************************************************/

#ifdef __cplusplus
extern "C" {
#endif

void
Biddy_Managed_AddFormula(Biddy_Manager MNG, Biddy_String x, Biddy_Edge f,
                         int count)
{
  unsigned int i,j;
  Biddy_Boolean OK;
  int cc;

  if (!MNG) MNG = biddyAnonymousManager;

  /* DEBUGGING */
  /*
  if (x) printf("Biddy_Managed_AddFormula: Formula %s\n",x);
  */

  if (count) count += biddyCount;

  /* if formula does not have a name and it is known to be already */
  /* persistently preserved then it is not neccessary to add it */
  if (!x && (Biddy_IsNull(f) || Biddy_IsConstant(f))) {
    return;
  } 

  if (!x) {
    OK = FALSE;
    i = biddyFormulaTable.size;
  } else {
    OK = FALSE;
    if (!biddyFormulaTable.table) {
      i = 0;
    } else {
      for (i = 0; !OK && (i < biddyFormulaTable.size); i++) {
        if (biddyFormulaTable.table[i].name) {
          cc = strcmp(x,biddyFormulaTable.table[i].name);
          if ((cc == 0) && !biddyFormulaTable.table[i].deleted) {
            OK = TRUE;
            break;
          }
          if (cc < 0) break;
        } else {
          break;
        }
      }
    }
  }

  /* the first two formulae ("0" and "1") must not be changed */
  if (OK && (i<2))
  {
    return;
  }

  /* formulae representing variables must not be changed */
  if (OK && (biddyFormulaTable.table[i].name
         && !Biddy_IsNull(biddyFormulaTable.table[i].f)
         && !strcmp(biddyFormulaTable.table[i].name,
                Biddy_GetTopVariableName(biddyFormulaTable.table[i].f))))
  {
    return;
  }
  
  /* element found */
  if (OK) {

    if (!biddyFormulaTable.table[i].count) {
      if (!biddyFormulaTable.table[i].deleted) {
        biddyFormulaTable.table[i].deleted = TRUE;
        i++;
        OK = FALSE;
      } else {
        fprintf(stderr,"Biddy_Managed_AddFormula: Problem with deleted persistent function!\n");
        exit(1);
      }
    }

    if (biddyFormulaTable.table[i].count > biddyCount) {
      if (!biddyFormulaTable.table[i].deleted) {
        biddyFormulaTable.table[i].deleted = TRUE;
        i++;
        OK = FALSE;
      } else {
        fprintf(stderr,"Biddy_Managed_AddFormula: Problem with deleted preserved function!\n");
        exit(1);
      }
    }

  }

  /* element can be reused */
  if (OK) {

    /*
    printf("Biddy_Managed_AddFormula: Formula %s OVERWRITTEN!\n",x);
    */

    biddyFormulaTable.table[i].f = f;
    biddyFormulaTable.table[i].count = count;
    biddyFormulaTable.table[i].deleted = FALSE;
    if (!Biddy_IsNull(f)) {
      if (!count) {
        BiddyFortify(MNG,f);
      } else {
        BiddyProlong(MNG,f,count);
      }
    }
    return;
  }

  /* element must be added */
  biddyFormulaTable.size++;
  if (!(biddyFormulaTable.table = (BiddyFormula *)
     realloc(biddyFormulaTable.table,biddyFormulaTable.size*sizeof(BiddyFormula))))
  {
    fprintf(stderr,"Biddy_Managed_AddFormula: Out of memoy!\n");
    exit(1);
  }

  for (j = biddyFormulaTable.size-1; j > i; j--) {
    biddyFormulaTable.table[j] = biddyFormulaTable.table[j-1];
  }

  if (x) {
    biddyFormulaTable.table[i].name = strdup(x);
    biddyFormulaTable.numOrdered++;
  } else {
    biddyFormulaTable.table[i].name = NULL;
  }
  biddyFormulaTable.table[i].f = f;
  biddyFormulaTable.table[i].count = count;
  biddyFormulaTable.table[i].deleted = FALSE;
  if (!Biddy_IsNull(f)) {
    if (!count) {
      BiddyFortify(MNG,f);
    } else {
      BiddyProlong(MNG,f,count);
    }
  }

  /* DEBUGGING */
  /*
  printf("Biddy_Managed_AddFormula: i=%d/%d, x=%s, f=%p\n",i,biddyFormulaTable.size-1,x,f);
  */

}

#ifdef __cplusplus
}
#endif


/***************************************************************************//*!
\brief Function Biddy_Managed_DeleteFormula delete formula from Formula table.

### Description
    Nodes of the given formula are tagged as not needed.
    Formula is not accessible by its name anymore.
    Regular cleaning with Biddy_Clean is not considered this tag.
    Obsolete nodes from preserved formulae can be immediately removed
    in explicite call to Biddy_ClearAll (this is the only way to remove
    obsolete nodes from permanently preserved formulae).
### Side effects
    Function representing a variable cannot be removed.
    Formula is labelled and not immediately removed.
    Nodes are not immediately removed.
### More info
    Macro Biddy_DeleteFormula(x) is defined for use with anonymous manager.
*******************************************************************************/

#ifdef __cplusplus
extern "C" {
#endif

Biddy_Boolean
Biddy_Managed_DeleteFormula(Biddy_Manager MNG, Biddy_String x)
{
  unsigned int i,j;
  Biddy_Boolean OK;
  int cc;
  Biddy_Edge f;

  if (!MNG) MNG = biddyAnonymousManager;

  if (!x) {
    printf("WARNING (Biddy_Managed_DeleteFormula): empty name!\n");
    return FALSE;
  }

  OK = FALSE;

  if (!biddyFormulaTable.table) {
    i = 0;
  } else {
    for (i = 0; !OK && (i < biddyFormulaTable.size); i++) {
      if (biddyFormulaTable.table[i].name) {
        cc = strcmp(x,biddyFormulaTable.table[i].name);
        if ((cc == 0) && !biddyFormulaTable.table[i].deleted) {
          OK = TRUE;
          break;
        }
        if (cc < 0) break;
      } else {
        break;
      }
    }
  }

  if (OK) {
    OK = Biddy_Managed_DeleteIthFormula(MNG,i);
  }

  return OK;
}

#ifdef __cplusplus
}
#endif


/***************************************************************************//*!
\brief Function Biddy_Managed_DeleteIthFormula deletes formula from the table.

### Description
    Nodes of the given formula are tagged as not needed.
    Formula is not accessible by its name anymore.
    Regular cleaning with Biddy_Clean is not considered this tag.
    Obsolete nodes from preserved formulae can be immediately removed
    in explicite call to Biddy_ClearAll (this is the only way to remove
    obsolete nodes from permanently preserved formulae).
### Side effects
    Function representing a variable cannot be removed.
    Formula is labelled and not immediately removed.
    Nodes are not immediately removed.
### More info
    Macro Biddy_DeleteIthFormula(x) is defined for use with anonymous manager.
*******************************************************************************/

#ifdef __cplusplus
extern "C" {
#endif

Biddy_Boolean
Biddy_Managed_DeleteIthFormula(Biddy_Manager MNG, unsigned int i)
{
  Biddy_Boolean OK;
  Biddy_Edge f;
  Biddy_String name;

  if (!MNG) MNG = biddyAnonymousManager;

  /* the first two formulae ("0" and "1") must not be deleted */
  if (i<2) {
    OK = FALSE;
  } else {
    OK = TRUE;
  }

  if (OK) {
    /* formulae representing variables must not be deleted */
    if (Biddy_IsNull(biddyFormulaTable.table[i].f) ||
        !biddyFormulaTable.table[i].name ||
        strcmp(biddyFormulaTable.table[i].name,
               Biddy_GetTopVariableName(biddyFormulaTable.table[i].f)))
    {

      /* DEBUGGING */
      /*
      if (biddyFormulaTable.table[i].name) {
        fprintf(stderr,"Biddy_Managed_DeleteIthFormula: Deleted %s\n",biddyFormulaTable.table[i].name);
        exit(1);
      }
      */

      /* this can be removed */
      biddyFormulaTable.table[i].deleted = TRUE;
    } else {
      OK = FALSE;
    }
  }

  return OK;
}

#ifdef __cplusplus
}
#endif

/***************************************************************************//*!
\brief Function Biddy_Managed_GetIthFormula returns ith formula in a Formula
       table.

### Description
    Return biddyNull if ith formulae does not exist.
### Side effects
    After addding new formula the index of others may change!
### More info
    Macro Biddy_GetIthFormula(i) is defined for use with anonymous manager.
*******************************************************************************/

#ifdef __cplusplus
extern "C" {
#endif

Biddy_Edge
Biddy_Managed_GetIthFormula(Biddy_Manager MNG, unsigned int i)
{
  if (!MNG) MNG = biddyAnonymousManager;

  if (!biddyFormulaTable.table) return biddyNull;
  if (i >= biddyFormulaTable.size) return biddyNull;

  return biddyFormulaTable.table[i].f;
}

#ifdef __cplusplus
}
#endif

/***************************************************************************//*!
\brief Function Biddy_Managed_GetIthFormulaName returns name of the ith formula
       in a Formula table.

### Description
    Return NULL if ith formulae does not exist.
### Side effects
    After addding new formula the index of others may change!
### More info
    Macro Biddy_GetIthFormulaName(i) is defined for use with anonymous manager.
*******************************************************************************/

#ifdef __cplusplus
extern "C" {
#endif

Biddy_String
Biddy_Managed_GetIthFormulaName(Biddy_Manager MNG, unsigned int i)
{
  Biddy_String name;

  if (!MNG) MNG = biddyAnonymousManager;

  if (!biddyFormulaTable.table) return NULL;
  if (i >= biddyFormulaTable.size) return NULL;

  if (biddyFormulaTable.table[i].deleted) {
   name = biddyFormulaTable.deletedName;
  } else {
   name = biddyFormulaTable.table[i].name;
  }

  return name;
}

#ifdef __cplusplus
}
#endif

/***************************************************************************//*!
\brief Function Biddy_Managed_SwapWithHigher swaps two adjacent variables.

### Description
    Higher (greater) variable is the bottommore one!
    The highest element is constant "1".
    Constant '1' has global ordering numUsedVariables (not smaller than anyone).
    Global ordering is the number of 0 in corresponding line of orderingTable.
### Side effects
### More info
    Macro Biddy_SwapWithHigher(v) is defined for use with anonymous manager.
*******************************************************************************/

#ifdef __cplusplus
extern "C" {
#endif

Biddy_Variable
Biddy_Managed_SwapWithHigher(Biddy_Manager MNG, Biddy_Variable v)
{
  Biddy_Variable vhigh;

  if (!MNG) MNG = biddyAnonymousManager;

  /* OBSOLETE AND FRESH NODES ARE REMOVED BEFORE SWAPPING */
  BiddyIncCounter(MNG);
  Biddy_Managed_Garbage(MNG);

  vhigh = swapWithHigher(MNG,v);

  BiddyDecCounter(MNG); /* revert fresh nodes back to prolonged nodes */

  return vhigh;
}

#ifdef __cplusplus
}
#endif

/***************************************************************************//*!
\brief Function Biddy_Managed_SwapWithLower swaps two adjacent variables.

### Description
    Lower (smaller) variable is the topmore one!
    The lowest (topmost) element is not fixed.
    Topmost variable has global ordering 1 (smaller than all except itself).
    Global ordering is the number of 0 in corresponding line of orderingTable.
### Side effects
### More info
    Macro Biddy_SwapWithLower(v) is defined for use with anonymous manager.
*******************************************************************************/

#ifdef __cplusplus
extern "C" {
#endif

Biddy_Variable
Biddy_Managed_SwapWithLower(Biddy_Manager MNG, Biddy_Variable v)
{
  Biddy_Variable vlow;

  if (!MNG) MNG = biddyAnonymousManager;

  /* OBSOLETE AND FRESH NODES ARE REMOVED BEFORE SWAPPING */
  BiddyIncCounter(MNG);
  Biddy_Managed_Garbage(MNG);

  vlow = swapWithLower(MNG,v);

  BiddyDecCounter(MNG); /* revert fresh nodes back to prolonged nodes */

  return vlow;
}

#ifdef __cplusplus
}
#endif

/***************************************************************************//*!
\brief Function Biddy_Managed_Sifting reorders variables to minimize node number
       for the whole system (if f = NULL) or for the given function
       (if f != NULL) using Rudell's sifting algorithm.

### Description
    Variables are reordered globally!
    All fresh nodes are removed
### Side effects
    It is hard to implement sifting if fresh nodes cannot be removed.
### More info
    Macro Biddy_Sifting(f) is defined for use with anonymous manager.
*******************************************************************************/

#ifdef __cplusplus
extern "C" {
#endif

Biddy_Boolean
Biddy_Managed_Sifting(Biddy_Manager MNG, Biddy_Edge f)
{
  unsigned int num,min;
  unsigned int n,best1,best2;
  Biddy_Variable v,k,minvar,maxvar,varOrder,minvarOrder,maxvarOrder;
  Biddy_Boolean highfirst,stop,finish;
  BiddyOrderingTable *fOrdering;
  Biddy_Edge spr;
  unsigned int varTable[BIDDYVARMAX];
  
  /* varTable is used in the following way: */
  /* varTable[i] == 0 iff variable is not in f */
  /* varTable[i] == 1 iff variable is in f and has not been sifted, yet */
  /* varTable[i] == 2 iff variable is in f and has already been sifted */

  if (!MNG) MNG = biddyAnonymousManager;
  
  if (FALSE) {
    /* sifting will not be performed */ 
    return FALSE;
  }

  if (!f) {

    /* MINIMIZING NODE NUMBER FOR THE WHOLE SYSTEM */

    varTable[0] = 2; /* variable '1' is not swapped */
    for (k=1; k<Biddy_Managed_VariableTableNum(MNG); k++) {
      varTable[k] = 1;
    }

    /* determine minvar and maxvar */
    /* minvar is topmost variable and has global ordering 1 */
    /* maxvar is bottommost variable '1' and has global ordering numUsedVariables */
    /* global ordering is the number of 0 in corresponding line of orderingTable */
    minvar = maxvar = 0;
    for (v=1; v<Biddy_Managed_VariableTableNum(MNG); v++) {
      varOrder = Biddy_Managed_VariableTableNum(MNG);
      for (k=0; k<Biddy_Managed_VariableTableNum(MNG); k++) {
        if (GET_ORDER(biddyOrderingTable,v,k)) varOrder--;
      }
      if (varOrder == 1) minvar = v;
      if (varOrder == Biddy_Managed_VariableTableNum(MNG)) maxvar = v;
      if ((minvar != 0) && (maxvar != 0)) break;
    }
    fOrdering =  &biddyOrderingTable;
    num = Biddy_Managed_NodeTableNumF(MNG);

  } else {

    /* MINIMIZING NODE NUMBER FOR THE GIVEN FUNCTION */

    varTable[0] = 2; /* variable '1' is not swapped */
    for (k=1; k<Biddy_Managed_VariableTableNum(MNG); k++) {
      varTable[k] = 0;
    }
    spr = Biddy_Managed_Support(MNG,f);
    while (!Biddy_IsConstant(spr)) {
      varTable[Biddy_GetTopVariable(spr)] = 1;
      spr = BiddyT(spr);
    }

    fOrdering = (BiddyOrderingTable *) malloc(sizeof(BiddyOrderingTable));

    /* determine node number and variable ordering for the specific function */
    /* fOrdering is the same ordering as biddyOrderingTable, but it */
    /* also contains information about which variables are neightbours in f */
    nullOrdering(*fOrdering);
    num = 1;
    nodeNumberOrdering(f,&num,*fOrdering);
    Biddy_NodeRepair(f);

    /* determine minvar (topmost) and maxvar (bottommost) variable */
    minvar = maxvar = Biddy_GetTopVariable(f);
    stop = FALSE;
    while (!stop) {
      stop = TRUE;
      for (k=1; k<Biddy_VariableTableNum(); k++) {
        if (GET_ORDER(*fOrdering,maxvar,k)) {
          maxvar = k;
          stop = FALSE;
          break;
        }
      }
    }
  }

  /* determine minvar's and maxvar's global ordering */
  /* in this global ordering, highest (topmost) variable has ordering 1 */
  /* and smallest (bottommost) variable '1' has ordering numUsedVariables */
  minvarOrder = maxvarOrder = Biddy_Managed_VariableTableNum(MNG);
  for (k=0; k<Biddy_Managed_VariableTableNum(MNG); k++) {
    if (GET_ORDER(biddyOrderingTable,minvar,k)) minvarOrder--;
    if (GET_ORDER(biddyOrderingTable,maxvar,k)) maxvarOrder--;
  }

  if (!f) {
    /* MINIMIZING NODE NUMBER FOR THE WHOLE SYSTEM */
    if (minvarOrder != 1) {
      fprintf(stdout,"ERROR (Biddy_Managed_Sifting): minvarOrder %u is wrong\n",minvarOrder);
      free((void *)1); /* BREAKPOINT */
    }
    if (maxvarOrder != Biddy_Managed_VariableTableNum(MNG)) {
      fprintf(stdout,"ERROR (Biddy_Managed_Sifting): maxvarOrder %u is wrong\n",maxvarOrder);
      free((void *)1); /* BREAKPOINT */
    }
  }

  /* DEBUGGING */
  /*
  printf("\n========================================\n");
  printf("SIFTING START: num sifting: %u\n",Biddy_Managed_NodeTableSiftingNumber(MNG));
  printf("SIFTING START: num swapping: %u\n",Biddy_Managed_NodeTableSwapNumber(MNG));
  printf("SIFTING START: num garbage collection: %u\n",Biddy_Managed_NodeTableGarbage(MNG));
  BiddySystemReport(MNG);
  if (f) {
    BiddyFormula formula;
    printf("MINIMIZING NODE NUMBER FOR THE GIVEN FUNCTION\n");
    for (n=0;n<biddyFormulaTable.size;n++) {
      formula = biddyFormulaTable.table[n];
      if (Biddy_IsEqv(formula.f,f)) {
        if (formula.name) {
           printf("Formula name: %s\n",formula.name);
        } else {
           printf("Formula name: NULL\n");
        }
      }
    }
    printf("Initial number of variables: %u\n",Biddy_VariableNumber(f));
    printf("Initial number of minterms: %.0f\n",Biddy_CountMinterm(f,0));
    printf("Initial number of nodes: %u\n",num);
    printf("Top edge: %p\n",(void *) f);
    printf("Top variable: %s\n",Biddy_Managed_GetTopVariableName(MNG,f));
    printf("Count of top node: %d\n",((BiddyNode *) Biddy_Regular(f))->count);
    printf("minvar: %s (id=%u, order=%u), maxvar: %s (id=%u, order=%u)\n",
            Biddy_Managed_GetVariableName(MNG,minvar),minvar,minvarOrder,
            Biddy_Managed_GetVariableName(MNG,maxvar),maxvar,maxvarOrder);
    printf("SIFTING: initial table\n");
    Biddy_Managed_WriteTable(MNG,f);
  }
  printf("SIFTING: initial order\n");
  writeORDER(MNG);
  printf("========================================\n");
  */

  /* SIFTING ALGORITHM */
  /* EXAMINE ALL VARIABLES ONCE! */

  biddyNodeTable.sifting++;

  /* OBSOLETE AND FRESH NODES ARE REMOVED BEFORE SIFTING */
  BiddyIncCounter(MNG);
  Biddy_Managed_Garbage(MNG);
  BiddyDecCounter(MNG);

  v = minvar; /* start with smallest (topmost) variable */
  finish = (varTable[v] == 2); /* maybe, we are sifting constant '1' */
  min = num;
  while (!finish) {
    varTable[v] = 2;

    /* DEBUGGING */
    /*
    printf("SIFTING: variable: %s (id=%u, order=%u, num=%u)\n",
           Biddy_Managed_GetVariableName(MNG,v),v,n,biddyVariableTable.table[v].num);
    BiddySystemReport(MNG);
    */

    /* DETERMINE FIRST DIRECTION */
    if (v == minvar) { /* TOPMOST VARIABLE */
      n = minvarOrder; /* used for debugging, only */
      stop = TRUE; /* skip first direction */
      highfirst = FALSE; /* swapWithHigher will be used in second direction */
    } else if (v == maxvar) { /* BOTTOMMOST VARIABLE */
      n = maxvarOrder; /* used for debugging, only */
      stop = TRUE; /* skip first direction */
      highfirst = TRUE; /* swapWithLower will be used in second direction */
    } else {
      stop = FALSE;
      /* determine v's global ordering */
      /* it is better to start with shorter direction */
      n = Biddy_Managed_VariableTableNum(MNG);
      for (k=0; k<Biddy_Managed_VariableTableNum(MNG); k++) {
        if (GET_ORDER(biddyOrderingTable,v,k)) n--;
      }
      if (n > (minvarOrder+maxvarOrder)/2) highfirst = TRUE; else highfirst = FALSE;
    }

    /* DEBUGGING */
    /*
    printf("SIFTING: FIRST DIRECTION\n");
    */

    /* ONE DIRECTION */
    n = 0;
    best1 = 0;
    while (!stop && (highfirst ? 
                      (k=swapWithHigher(MNG,v)) :
                      (k=swapWithLower(MNG,v))) && k)
    {

      n++;

      /* DEBUGGING */
      /*
      printf("SIFTING: ");
      if (highfirst) printf("Swaped With Higher\n"); else printf("Swaped With Lower\n");
      */

      /* IF (!f) THEN THIS if IS ALWAYS TRUE */
      /* IF (f) THEN THIS if IS TRUE IF k AND v ARE NEIGHTBOURS IN f */
      if (GET_ORDER(*fOrdering,v,k) || GET_ORDER(*fOrdering,k,v)) {

        if (!f) {

          /* DETERMINE THE NUMBER OF USEFUL NODES */
          num = Biddy_Managed_NodeTableNumF(MNG);

        } else {

          /* update node number and variable ordering for the specific function */
          /* both is neccessary for the algorithm */
          /* *fOrdering is the same ordering as biddyOrderingTable, but it */
          /* also contains information about which variables are neightbours */
          nullOrdering(*fOrdering);
          num = 1;
          nodeNumberOrdering(f,&num,*fOrdering);
          Biddy_NodeRepair(f);

        }

        /* DEBUGGING */
        /*
        printf("SIFTING: first direction, v=%s, step=%u, min=%u, num=%u\n",
               Biddy_Managed_GetVariableName(MNG,minvar),n,min,num);
        */

        if (num <= min) {
          min = num;
          best1 = n;
        }

        if (highfirst) {
          if (v == minvar) {
            minvar = k;  /* update minvar */
            /* minvarOrder is the same */
          }
          if (k == maxvar) {
            maxvar = v; /* update maxvar */
            /* maxvarOrder is the same */
            stop = TRUE;
          }
        } else {
          if (k == minvar) {
            minvar = v; /* update minvar */
            /* minvarOrder is the same */
            stop = TRUE;
          }
          if (v == maxvar) {
            maxvar = k;  /* update maxvar */
            /* maxvarOrder is the same */
          }
        }

        if (num > (12 * min / 10)) { /* THIS IS TRESHOLD FOR SIFTING! */
          stop = TRUE;
        }

      } else {

        /* DEBUGGING */
        /*
        printf("SIFTING: NOT NEIGHBOURS IN F!\n");
        */

        /* THEY ARE NOT NEIGHBOURS BUT THEY MAY BE BOTH IN F !! */

        if (highfirst) {
          if (v == minvar) {
            /* if k is in f then minvar = k else minvarOrder++ */
            if (varTable[k]) {
              minvar = k;
            } else {
              minvarOrder++;
            }
          }
          if (k == maxvar) {
            /* if v is in f then maxvar = v else maxvarOrder-- */
            if (varTable[v]) {
              maxvar = v;
            } else {
              maxvarOrder--;
            }
            stop = TRUE;
          }
        } else {
          if (k == minvar) {
            /* if v is in f then minvar = v else minvarOrder++ */
            if (varTable[v]) {
              minvar = v;
            } else {
              minvarOrder++;
            }
            stop = TRUE;
          }
          if (v == maxvar) {
            /* if k is in f then maxvar = k else maxvarOrder-- */
            if (varTable[k]) {
              maxvar = k;
            } else {
              maxvarOrder--;
            }
          }
        }

      }

      /* IF OBSOLETE NODES ARE REMOVED THE AGORITHM MAY BE MORE EFFICIENT */
      /*
      Biddy_Managed_Garbage(MNG);
      */

    } /* while (!stop ...) */

    /* DEBUGGING */
    /*
    printf("SIFTING: GO BACK!\n");
    */

    /* GO BACK */
    while (n) {
      if (highfirst) {
        k = swapWithLower(MNG,v);
        if (v == maxvar) {
          /* if k is in f then maxvar = k else maxvarOrder-- */
          if (varTable[k]) {
            maxvar = k;
          } else {
            maxvarOrder--;
          }
        }
        if (k == minvar) {
          /* if v is in f then minvar = v else minvarOrder++ */
          if (varTable[v]) {
            minvar = v;
          } else {
            minvarOrder++;
          }
        }
      } else {
        k = swapWithHigher(MNG,v);
        if (v == minvar) {
          /* if k is in f then minvar = k else minvarOrder++ */
          if (varTable[k]) {
            minvar = k;
          } else {
            minvarOrder++;
          }
        }
        if (k == maxvar) {
          /* if v is in f then maxvar = v else maxvarOrder-- */
          if (varTable[v]) {
            maxvar = v;
          } else {
            maxvarOrder--;
          }
        }
      }
      n--;

      /* IF OBSOLETE NODES ARE REMOVED THE AGORITHM MAY BE MORE EFFICIENT */
      /*
      Biddy_Managed_Garbage(MNG);
      */

    }

    /* DEBUGGING */
    /*
    printf("SIFTING: SECOND DIRECTION\n");
    */

    /* OPPOSITE DIRECTION */
    n = 0;
    best2 = 0;
    stop = FALSE;
    while (!stop && (highfirst ?
                      (k=swapWithLower(MNG,v)) :
                      (k=swapWithHigher(MNG,v))) && k)
    {

      n++;

      /* DEBUGGING */
      /*
      printf("SIFTING: ");
      if (highfirst) printf("Swaped With Lower\n"); else printf("Swaped With Higher\n");
      */

      /* IF (!f) THEN THIS if IS ALWAYS TRUE */
      /* IF (f) THEN THIS if IS TRUE IF k AND v ARE NEIGHTBOURS IN f */
      if (GET_ORDER(*fOrdering,v,k) || GET_ORDER(*fOrdering,k,v)) {

        if (!f) {

          /* DETERMINE THE NUMBER OF USEFUL NODES */
          num = Biddy_Managed_NodeTableNumF(MNG);

        } else {

          /* update node number and variable ordering for the specific function */
          /* both is neccessary for the algorithm */
          /* *fOrdering is the same ordering as biddyOrderingTable, but it */
          /* also contains information about which variables are neightbours */
          nullOrdering(*fOrdering);
          num = 1;
          nodeNumberOrdering(f,&num,*fOrdering);
          Biddy_NodeRepair(f);

        }

        /* DEBUGGING */
        /*
        printf("SIFTING: second direction, v=%s, step=%u, min=%u, num=%u\n",
               Biddy_Managed_GetVariableName(MNG,minvar),n,min,num);
        */

        if (num <= min) {
          min = num;
          best2 = n;
        }

        if (highfirst) {
          if (k == minvar) {
            minvar = v; /* update minvar */
            stop = TRUE;
          }
          if (v == maxvar) {
            maxvar = k;  /* update maxvar */
            /* maxvarOrder is the same */
          }
        } else {
          if (v == minvar) {
            minvar = k;  /* update minvar */
            /* minvarOrder is the same */
          }
          if (k == maxvar) {
            maxvar = v; /* update maxvar */
            stop = TRUE;
          }
        }

        if (num > (12 * min / 10)) { /* THIS IS TRESHOLD FOR SIFTING! */
          stop = TRUE;
        }

      } else {

        /* DEBUGGING */
        /*
        printf("SIFTING: NOT NEIGHBOURS IN F!\n");
        */

        /* THEY ARE NOT NEIGHBOURS BUT THEY MAY BE BOTH IN F !! */

        if (highfirst) {
          if (k == minvar) {
            /* if v is in f then minvar = v else minvarOrder++ */
            if (varTable[v]) {
              minvar = v;
            } else {
              minvarOrder++;
            }
            stop = TRUE;
          }
          if (v == maxvar) {
            /* if k is in f then maxvar = k else maxvarOrder-- */
            if (varTable[k]) {
              maxvar = k;
            } else {
              maxvarOrder--;
            }
          }
        } else {
          if (v == minvar) {
            /* if k is in f then minvar = k else minvarOrder++ */
            if (varTable[k]) {
              minvar = k;
            } else {
              minvarOrder++;
            }
          }
          if (k == maxvar) {
            /* if v is in f then maxvar = v else maxvarOrder-- */
            if (varTable[v]) {
              maxvar = v;
            } else {
              maxvarOrder--;
            }
            stop = TRUE;
          }
        }

      }

      /* IF OBSOLETE NODES ARE REMOVED THE AGORITHM MAY BE MORE EFFICIENT */
      /*
      Biddy_Managed_Garbage(MNG);
      */

    } /* while (!stop ...) */

    /* DEBUGGING */
    /*
    printf("SIFTING: GO FINAL!\n");
    */

    /* IF best2==0 THEN MINIMUM WAS REACHED IN THE FIRST WHILE */
    /* YOU HAVE TO GO BACK AND THEN MAKE best1 STEPS */
    /* IF best2!=0 THEN MINIMUM WAS REACHED IN THE SECOND WHILE */
    /* YOU HAVE TO GO BACK UNTIL n == best2 */

    if (!best2) {
      while (n) {
        if (highfirst) {
          k = swapWithHigher(MNG,v);
          if (v == minvar) {
            /* if k is in f then minvar = k else minvarOrder++ */
            if (varTable[k]) {
              minvar = k;
            } else {
              minvarOrder++;
            }
          }
          if (k == maxvar) {
            /* if v is in f then maxvar = v else maxvarOrder-- */
            if (varTable[v]) {
              maxvar = v;
            } else {
              maxvarOrder--;
            }
          }
        } else {
          k = swapWithLower(MNG,v);
          if (v == maxvar) {
            /* if k is in f then maxvar = k else maxvarOrder-- */
            if (varTable[k]) {
              maxvar = k;
            } else {
              maxvarOrder--;
            }
          }
          if (k == minvar) {
            /* if v is in f then minvar = v else minvarOrder++ */
            if (varTable[v]) {
              minvar = v;
            } else {
              minvarOrder++;
            }
          }
        }
        n--;

        /* IF OBSOLETE NODES ARE REMOVED THE AGORITHM MAY BE MORE EFFICIENT */
        /*
        Biddy_Managed_Garbage(MNG);
        */

      }
      while (n!=best1) {
        if (highfirst) {
          k = swapWithHigher(MNG,v);
          if (v == minvar) {
            /* if k is in f then minvar = k else minvarOrder++ */
            if (varTable[k]) {
              minvar = k;
            } else {
              minvarOrder++;
            }
          }
          if (k == maxvar) {
            /* if v is in f then maxvar = v else maxvarOrder-- */
            if (varTable[v]) {
              maxvar = v;
            } else {
              maxvarOrder--;
            }
          }
        } else {
          k = swapWithLower(MNG,v);
          if (v == maxvar) {
            /* if k is in f then maxvar = k else maxvarOrder-- */
            if (varTable[k]) {
              maxvar = k;
            } else {
              maxvarOrder--;
            }
          }
          if (k == minvar) {
            /* if v is in f then minvar = v else minvarOrder++ */
            if (varTable[v]) {
              minvar = v;
            } else {
              minvarOrder++;
            }
          }
        }
        n++;

        /* IF OBSOLETE NODES ARE REMOVED THE AGORITHM MAY BE MORE EFFICIENT */
        /*
        Biddy_Managed_Garbage(MNG);
        */

      }
    } else {
      while (n!=best2) {
        if (highfirst) {
          k = swapWithHigher(MNG,v);
          if (v == minvar) {
            /* if k is in f then minvar = k else minvarOrder++ */
            if (varTable[k]) {
              minvar = k;
            } else {
              minvarOrder++;
            }
          }
          if (k == maxvar) {
            /* if v is in f then maxvar = v else maxvarOrder-- */
            if (varTable[v]) {
              maxvar = v;
            } else {
              maxvarOrder--;
            }
          }
        } else {
          k = swapWithLower(MNG,v);
          if (v == maxvar) {
            /* if k is in f then maxvar = k else maxvarOrder-- */
            if (varTable[k]) {
              maxvar = k;
            } else {
              maxvarOrder--;
            }
          }
          if (k == minvar) {
            /* if v is in f then minvar = v else minvarOrder++ */
            if (varTable[v]) {
              minvar = v;
            } else {
              minvarOrder++;
            }
          }
        }
        n--;

        /* IF OBSOLETE NODES ARE REMOVED THE AGORITHM MAY BE MORE EFFICIENT */
        /*
        Biddy_Managed_Garbage(MNG);
        */

      }
    }

    /* OBSOLETE AND FRESH NODES ARE REMOVED AFTER SIFTING ONE VARIABLE */
    /* THIS IS NEEDED FOR EFFICIENT ALGORITHM */
    /**/
    Biddy_Managed_Garbage(MNG);
    /**/

    /* DEBUGGING */
    /*
    varOrder = Biddy_Managed_VariableTableNum(MNG);
    for (k=0; k<Biddy_Managed_VariableTableNum(MNG); k++) {
      if (GET_ORDER(biddyOrderingTable,v,k)) varOrder--;
    }
    printf("SIFTING - VARIABLE %s, new order = %u, minvar = %s, maxvar = %s, swap: %u, gc: %u\n",
            Biddy_Managed_GetVariableName(MNG,v),
            varOrder,
            Biddy_Managed_GetVariableName(MNG,minvar),
            Biddy_Managed_GetVariableName(MNG,maxvar),
            Biddy_Managed_NodeTableSwapNumber(MNG),
            Biddy_Managed_NodeTableGarbage(MNG));
    BiddySystemReport(MNG);
    */

    /* CHOOSE NEXT VARIABLE OR FINISH */
    /* IF f IS GIVEN THEN VARIABLES NOT IN f WILL BE SKIPPED */
    finish = TRUE;
    v = minvar; /* try current topmost variable */
    if (varTable[v] == 1) finish = FALSE;
    if (finish == TRUE) {
      v = 1; /* check all variables - ordering is simple by id */
      while ((v < Biddy_Managed_VariableTableNum(MNG)) && finish) {
        if (varTable[v] != 1) v++; else finish = FALSE;
      }
    }

  }

  /* END OF SIFTING ALGORITHM */

  /* DEBUGGING */
  /*
  printf("========================================\n");
  printf("SIFTING FINISH: num sifting: %u\n",Biddy_Managed_NodeTableSiftingNumber(MNG));
  printf("SIFTING FINISH: num swapping: %u\n",Biddy_Managed_NodeTableSwapNumber(MNG));
  printf("SIFTING FINISH: num garbage collection: %u\n",Biddy_Managed_NodeTableGarbage(MNG));
  BiddySystemReport(MNG);
  if (f) {
    minvarOrder = maxvarOrder = Biddy_Managed_VariableTableNum(MNG);
    for (k=0; k<Biddy_Managed_VariableTableNum(MNG); k++) {
      if (GET_ORDER(biddyOrderingTable,minvar,k)) minvarOrder--;
      if (GET_ORDER(biddyOrderingTable,maxvar,k)) maxvarOrder--;
    }
    printf("Final number of variables: %u\n",Biddy_VariableNumber(f));
    printf("Final number of minterms: %.0f\n",Biddy_CountMinterm(f,0));
    printf("Final number of nodes: %u\n",Biddy_NodeNumber(f));
    printf("Top edge: %p\n",(void *) f);
    printf("Top variable: %s\n",Biddy_Managed_GetTopVariableName(MNG,f));
    printf("Count of top node: %d\n",((BiddyNode *) Biddy_Regular(f))->count);
    printf("minvar: %s (id=%u, order=%u), maxvar: %s (id=%u, order=%u)\n",
            Biddy_Managed_GetVariableName(MNG,minvar),minvar,minvarOrder,
            Biddy_Managed_GetVariableName(MNG,maxvar),maxvar,maxvarOrder);
    printf("SIFTING: final table\n");
    Biddy_Managed_WriteTable(MNG,f);
  }
  printf("SIFTING: final order\n");
  writeORDER(MNG);
  printf("========================================\n");
  */

  if (f) free(fOrdering);
  
  return TRUE; /* sifting has been performed */

}

#ifdef __cplusplus
}
#endif

/***************************************************************************//*!
\brief Function Biddy_Managed_Purge denotes that one step of
       calculation is finished.

### Description
    Immediately remove all nodes which were not preserved or which are not
    preserved anymore. Call to Biddy_Purge does not count as clearing and
    thus all preserved formulae remains preserved for the same number of
    clearings.
### Side effects
    Tag deleted is not considered and thus no fortified node is removed.
### More info
    Macro Biddy_Purge(f) is defined for use with anonymous manager.
*******************************************************************************/

#ifdef __cplusplus
extern "C" {
#endif

void
Biddy_Managed_Purge(Biddy_Manager MNG)
{
  if (!MNG) MNG = biddyAnonymousManager;

  BiddyIncCounter(MNG);
  Biddy_Managed_Garbage(MNG);
  BiddyDecCounter(MNG);
}

#ifdef __cplusplus
}
#endif

/***************************************************************************//*!
\brief Function Biddy_Managed_PurgeAndReorder denotes that one step of
       calculation is finished.

### Description
    Immediately remove all nodes which were not preserved or which are not
    preserved anymore. Call to Biddy_PurgeAndReorder does not count as clearing
    and thus all preserved formulae remains preserved for the same number of
    clearings. Moreover, Biddy_PurgeAndReorder trigger reordering on function
    (if BDD is given) or global reordering (if NULL is given).
### Side effects
    Tag deleted is not considered and thus no fortified node is removed.
    Removing nodes is done during sifting.
### More info
    Macro Biddy_PurgeAndReorder(f) is defined for use with anonymous manager.
*******************************************************************************/

#ifdef __cplusplus
extern "C" {
#endif

void
Biddy_Managed_PurgeAndReorder(Biddy_Manager MNG, Biddy_Edge f)
{
  if (!MNG) MNG = biddyAnonymousManager;

  Biddy_Managed_Sifting(MNG,f);
}

#ifdef __cplusplus
}
#endif

/***************************************************************************//*!
\brief Function Biddy_Managed_Refresh refreshes all obsolete nodes in a
       function.

### Description
    It is supposed that childrens of not-obsolete nodes are all not obsolete.
### Side effects
    This is an external variant of internal function BiddyRefresh.
    It is needed to implement user caches.
### More info
*******************************************************************************/

#ifdef __cplusplus
extern "C" {
#endif

void
Biddy_Managed_Refresh(Biddy_Manager MNG, Biddy_Edge f)
{
  if (!MNG) MNG = biddyAnonymousManager;

  BiddyRefresh(MNG,f);
}

#ifdef __cplusplus
}
#endif

/*----------------------------------------------------------------------------*/
/* Definition of internal functions                                           */
/*----------------------------------------------------------------------------*/

/***************************************************************************//*!
\brief Function BiddyIncCounter increments formulae counter by 1.

### Description
    All prolonged nodes become fresh nodes, all fresh nodes become obsolete
    nodes.
### Side effects
### More info
*******************************************************************************/

void
BiddyIncCounter(Biddy_Manager MNG)
{
  Biddy_Variable v;

  if (++biddyCount <= 0) {
    printf("WARNING: biddyCount is back to start");
    biddyCount--;
    /* TO DO: change count of all obsolete and fresh nodes to 1 */
    /* TO DO: change count of all prolonged nodes to 2 */
    biddyCount = 2;
  }

  for (v=1; v<biddyVariableTable.size; v++) {
    biddyVariableTable.table[v].freshNodes = biddyVariableTable.table[v].lastNewNode;
  }
}

/***************************************************************************//*!
\brief Function BiddyDecCounter decrements formulae counter by 1.

### Description
    All fresh nodes become prolonged nodes.
### Side effects
### More info
*******************************************************************************/

void
BiddyDecCounter(Biddy_Manager MNG)
{
  Biddy_Variable v;

  if (--biddyCount < 1) {
    printf("ERROR (BiddyDecCounter): biddyCount is wring");
  }
}

/***************************************************************************//*!
\brief Function BiddyRefresh refreshes all obsolete nodes in a function.

### Description
    It is supposed that childrens of not-obsolete nodes are all not obsolete.
### Side effects
### More info
*******************************************************************************/

void
BiddyRefresh(Biddy_Manager MNG, Biddy_Edge f)
{
  if (!BiddyIsOK(f)) {
    ((BiddyNode *) Biddy_Regular(f))->count = biddyCount;
    BiddyRefresh(MNG,BiddyE(f));
    BiddyRefresh(MNG,BiddyT(f));
  }
}

/***************************************************************************//*!
\brief Function BiddyProlong prolonges all non-fortified nodes in a function.

### Description
### Side effects
### More info
*******************************************************************************/

void
BiddyProlong(Biddy_Manager MNG, Biddy_Edge f, unsigned int count)
{
  if (((BiddyNode *) Biddy_Regular(f))->count &&
      (((BiddyNode *) Biddy_Regular(f))->count < count))
  {
    ((BiddyNode *) Biddy_Regular(f))->count = count;
    BiddyProlong(MNG,BiddyE(f),count);
    BiddyProlong(MNG,BiddyT(f),count);
  }
}

/***************************************************************************//*!
\brief Function BiddyFortify fortifies all nodes in a function.

### Description
    It is supposed that childrens of fortified node are all fortified.
### Side effects
### More info
*******************************************************************************/

void
BiddyFortify(Biddy_Manager MNG, Biddy_Edge f)
{
  if (((BiddyNode *) Biddy_Regular(f))->count) {
    ((BiddyNode *) Biddy_Regular(f))->count = 0;
    biddyNodeTable.numf++;
    BiddyFortify(MNG,BiddyE(f));
    BiddyFortify(MNG,BiddyT(f));
  }
}


/***************************************************************************//*!
\brief Function BiddyRepair repairs nodes.

### Description
    Successors of a fortified node must be fortified.
    Successors of a fresh node must be fresh iff not fortified.
    For efficiency, we assume no prolonged nodes are present.
    Only nodes with variables 'var' are repaired.
### Side effects
### More info
*******************************************************************************/

void
BiddyRepair1(Biddy_Manager MNG, Biddy_Variable var)
{
  Biddy_Variable v;
  BiddyNode *sup;
  unsigned int n;

  for (v=1; v<biddyVariableTable.num; v++) {
    if (GET_ORDER(biddyOrderingTable,v,var)) {
      /* two lists are connected to make implementation simple, only */
      /* lastNewNode and fortifiedNodes are not changed! */
      biddyVariableTable.table[v].lastNewNode->list =
        (void *)biddyVariableTable.table[v].fortifiedNodes;
      sup = biddyVariableTable.table[v].firstNewNode;
      while (sup) {
        if (sup->count == 0) {
          
          if (((BiddyNode *) Biddy_Regular(sup->f))->count) {

            /* DEBUGGING */
            /*
            if (((BiddyNode *) Biddy_Regular(sup->f))->v != var) {
              fprintf(stderr,"ERROR (BiddyRepair1): invalid obsolete node");
              exit(1);
            }
            */

            ((BiddyNode *) Biddy_Regular(sup->f))->count = 0;
            biddyNodeTable.numf++;
          }
          
          if (((BiddyNode *) sup->t)->count) {

            /* DEBUGGING */
            /*
            if (((BiddyNode *) sup->t)->v != var) {
              fprintf(stderr,"ERROR (BiddyRepair1): invalid obsolete node");
              exit(1);
            }
            */

            ((BiddyNode *) sup->t)->count = 0;
            biddyNodeTable.numf++;
          }
 
        } else {
 
          if (((BiddyNode *) Biddy_Regular(sup->f))->count &&
              (((BiddyNode *) Biddy_Regular(sup->f))->count < sup->count)) {

            /* DEBUGGING */
            /*
            if (((BiddyNode *) Biddy_Regular(sup->f))->v != var) {
              fprintf(stderr,"ERROR (BiddyRepair1): invalid obsolete node");
              exit(1);
            }
            */

            ((BiddyNode *) Biddy_Regular(sup->f))->count = sup->count;
          }
          
          if (((BiddyNode *) sup->t)->count &&
              (((BiddyNode *) sup->t)->count < sup->count)) {

            /* DEBUGGING */
            /*
            if (((BiddyNode *) Biddy_Regular(sup->t))->v != var) {
              fprintf(stderr,"ERROR (BiddyRepair1): invalid obsolete node\n");
              exit(1);
            }
            */

            ((BiddyNode *) sup->t)->count = sup->count;
          }

        }

        sup = (BiddyNode *) sup->list;

      }
    }
  }

  /* DEBUGGING */
  /*
  printf("REPAIR1 FINISHED: variable = %s\n",Biddy_Managed_GetVariableName(MNG,var));
  BiddySystemReport(MNG);
  */

}

void
BiddyRepair2(Biddy_Manager MNG, Biddy_Edge f, Biddy_Variable var, int c)
{
  if (((BiddyNode *) Biddy_Regular(f))->count >= 0) {

    if (((BiddyNode *) Biddy_Regular(f))->count == 0) {

      /* FORTIFIED NODE, NOT VISITED, YET */

      ((BiddyNode *) Biddy_Regular(f))->count = ~0;
      if (GET_ORDER(biddyOrderingTable,Biddy_GetTopVariable(f),var)) {
        BiddyRepair2(MNG,BiddyE(f),var,0);
        BiddyRepair2(MNG,BiddyT(f),var,0);
      }

    } else if (((BiddyNode *) Biddy_Regular(f))->count >= biddyCount) {

      /* FRESH PROLONGED NODE, NOT VISITED, YET */

      /* DEBUGGING */
      /*
      if (!c) {
        fprintf(stderr,"ERROR (BiddyRepair2): invalid node");
        exit(1);
      }
      */

      if (((BiddyNode *) Biddy_Regular(f))->count >= c) {

        /* THIS NODE DOES NOT NEED THE REPAIR */
        /* successors do not need repair */

        ((BiddyNode *) Biddy_Regular(f))->count = ~(((BiddyNode *) Biddy_Regular(f))->count);

      } else {

        ((BiddyNode *) Biddy_Regular(f))->count = ~c;
        if (GET_ORDER(biddyOrderingTable,Biddy_GetTopVariable(f),var)) {
          BiddyRepair2(MNG,BiddyE(f),var,c);
          BiddyRepair2(MNG,BiddyT(f),var,c);
        }

      }

    } else {

      /* OBSOLETE NODE, NOT VISITED, YET */
      /* successors should not need repair, because only "var" are broken */

      /* DEBUGGING */
      /*
      if (Biddy_GetTopVariable(f) != var) {
        fprintf(stderr,"ERROR (BiddyRepair2): invalid obsolete node");
        exit(1);
      }
      */

      if (!c) {
        biddyNodeTable.numf++;
      } else {
      }
      ((BiddyNode *) Biddy_Regular(f))->count = ~c;

    }

  } else {

    if (~(((BiddyNode *) Biddy_Regular(f))->count) == 0) {

      /* FORTIFIED NODE, ALREADY VISITED */
      /* successors do not need repair */

    } else if (~(((BiddyNode *) Biddy_Regular(f))->count) >= biddyCount) {

      /* FRESH OR PROLONGED NODE, ALREADY VISITED */

      /* DEBUGGING */
      /*
      if (!c) {
        fprintf(stderr,"ERROR (BiddyRepair2): invalid node");
        exit(1);
      }
      */

      if (~(((BiddyNode *) Biddy_Regular(f))->count) >= c) {

        /* THIS NODE DOES NOT NEED THE REPAIR */
        /* successors do not need repair */

      } else {

        ((BiddyNode *) Biddy_Regular(f))->count = ~c;
        if (GET_ORDER(biddyOrderingTable,Biddy_GetTopVariable(f),var)) {
          BiddyRepair2(MNG,BiddyE(f),var,c);
          BiddyRepair2(MNG,BiddyT(f),var,c);
        }

      }

    } else {

      /* OBSOLETE NODE, ALREADY VISITED */

      /* DEBUGGING */
      /*
      fprintf(stderr,"ERROR (BiddyRepair2): invalid situation");
      exit(1);
      */

    }
  }
}

/*******************************************************************************
\brief Function BiddyCreateLocalInfo creates local info table for the given
       function.

### Description
    Only one local info table may exists! It is not allowed to add new BDD nodes
    if local info table exists. It is not allowed to start GC if local info
    table exists. Function returns number of dependent variables.
### Side effects
    Local info for constant node is not created!
### More info
*******************************************************************************/

Biddy_Variable
BiddyCreateLocalInfo(Biddy_Manager MNG, Biddy_Edge f)
{
  unsigned int num;
  BiddyLocalInfo *c;
  Biddy_Variable i,var;

  if (Biddy_IsNull(f)) return -1;
  if (Biddy_IsConstant(f)) return 0;
  if (Biddy_IsSelected(f)) {
    fprintf(stderr,"ERROR: BiddyCreateLocalInfo got selected node\n");
    exit(1);
  }
  num = 0;  /* CONSTANT NODE IS NOT COUNTED HERE */

  for (i=0;i<biddyVariableTable.num;i++) {
    biddyVariableTable.table[i].selected = FALSE;
  }

  Biddy_SelectNode(biddyOne);
  BiddyNodeVarNumber(MNG,f,&num); /* NOW, ALL NODES ARE SELECTED */
  Biddy_DeselectNode(biddyOne); /* REQUIRED FOR createLocalCache */

  var = 0;
  for (i=0;i<biddyVariableTable.num;i++) {
    if (biddyVariableTable.table[i].selected == TRUE) {
      var++;
      biddyVariableTable.table[i].selected = FALSE;
    }
  }

  if (!(biddyLocalInfo = (BiddyLocalInfo *)
        malloc(num * sizeof(BiddyLocalInfo)))) {
    fprintf(stderr,"BiddyCreateLocalInfo: Out of memoy!\n");
    exit(1);
  }
  c = createLocalInfo(f,biddyLocalInfo);
  if (!c) {
    fprintf(stderr,"ERROR: createLocalInfo returns NULL\n");
    exit(1);
  }
  
  return var;
}

/*******************************************************************************
\brief Function BiddyDeleteLocalCache deletes local info table for the given
       function.

### Description
    Only one local info table may exists! After deleting local info table,
    Biddy is ready for any operation.
### Side effects
    All nodes except constant node must be selected.
    Local info for constant node should not be created!
### More info
*******************************************************************************/

void
BiddyDeleteLocalInfo(Biddy_Manager MNG, Biddy_Edge f)
{
  if (Biddy_IsNull(f)) return;
  if (Biddy_IsConstant(f)) return;
  if (!Biddy_IsSelected(f)) {
    fprintf(stderr,"ERROR: BiddyDeleteLocalInfo got unselected node\n");
    exit(1);
  }
  deleteLocalInfo(f); /* this will deselect all nodes */
  free(biddyLocalInfo);
  biddyLocalInfo = NULL;
}

/*******************************************************************************
\brief Function BiddySetMintermCount is used in mintermCount().

### Description
### Side effects
    This should not be called for constant node because constant node does
    not have local info!
### More info
*******************************************************************************/

void
BiddySetMintermCount(Biddy_Edge f, double value)
{
  ((BiddyLocalInfo *)
    (((BiddyNode *) Biddy_Regular(f))->list))->data.mintermCount = value;
}

/*******************************************************************************
\brief Function BiddyGetMintermCount is used in mintermCount().

### Description
### Side effects
    This should not be called for constant node because constant node does
    not have local info!
### More info
*******************************************************************************/

double
BiddyGetMintermCount(Biddy_Edge f)
{
  return ((BiddyLocalInfo *)
    (((BiddyNode *) Biddy_Regular(f))->list))->data.mintermCount;
}

/*******************************************************************************
\brief Function BiddySetNodePlainSelected is used in nodePlainNumber().

### Description
### Side effects
    This should not be called for constant node because constant node does
    not have local info!
### More info
*******************************************************************************/

void
BiddySetNodePlainSelected(Biddy_Edge f)
{
  if (Biddy_GetMark(f)) {
    ((BiddyLocalInfo *)
      (((BiddyNode *) Biddy_Regular(f))->list))->negativeSelected = TRUE;
  } else {
    ((BiddyLocalInfo *)
      (((BiddyNode *) Biddy_Regular(f))->list))->positiveSelected = TRUE;
  }
}

/*******************************************************************************
\brief Function BiddyGetNodePlainSelected is used in nodePlainNumber().

### Description
### Side effects
    This should not be called for constant node because constant node does
    not have local info!
### More info
*******************************************************************************/

Biddy_Boolean
BiddyGetNodePlainSelected(Biddy_Edge f)
{
  Biddy_Boolean r;
  
  if (Biddy_GetMark(f)) {
    r = ((BiddyLocalInfo *)
      (((BiddyNode *) Biddy_Regular(f))->list))->negativeSelected;
  } else {
    r = ((BiddyLocalInfo *)
      (((BiddyNode *) Biddy_Regular(f))->list))->positiveSelected;
  }
  
  return r;
}

/*******************************************************************************
\brief Function BiddySetNodePlainCount is used in nodePlainNumber().

### Description
### Side effects
    This should not be called for constant node because constant node does
    not have local info!
### More info
*******************************************************************************/

void
BiddySetNodePlainCount(Biddy_Edge f, unsigned int n)
{
  ((BiddyLocalInfo *)
    (((BiddyNode *) Biddy_Regular(f))->list))->data.nodePlainCount = n;
}

/*******************************************************************************
\brief Function BiddyGetNodePlainCount is used in nodePlainNumber().

### Description
### Side effects
    This should not be called for constant node because constant node does
    not have local info!
### More info
*******************************************************************************/

unsigned int
BiddyGetNodePlainCount(Biddy_Edge f)
{
  return ((BiddyLocalInfo *)
    (((BiddyNode *) Biddy_Regular(f))->list))->data.nodePlainCount;
}

/*******************************************************************************
\brief Function BiddySystemReport is intended for debugging.

### Description
### Side effects
### More info
*******************************************************************************/

#define SYSTEMREPORTVERBOSE
#undef SYSTEMREPORTVERBOSE

#define SYSTEMREPORTDETAILS
#undef SYSTEMREPORTDETAILS

void
BiddySystemReport(Biddy_Manager MNG)
{
  Biddy_Variable v;
  unsigned int i;
  unsigned int count1,count2;
  unsigned int countfortified,countprolonged,countfresh,countobsolete;

  /* VARIABLES */

#ifdef SYSTEMREPORTVERBOSE
  printf("SYSTEM REPORT (VARIABLES)\n");
#endif

#ifdef SYSTEMREPORTDETAILS
  for (v=0; v<Biddy_Managed_VariableTableNum(MNG); v++) {
    printf("%s(%d),",
           Biddy_Managed_GetVariableName(MNG,v),
           Biddy_Managed_NodeTableNumVar(MNG,v)
    );
  }
  printf("\n");
  printf("unused variables: %d\n",BIDDYVARMAX-Biddy_Managed_VariableTableNum(MNG));
#endif

#ifdef SYSTEMREPORTVERBOSE
  printf("nuber of used varaibles: %d\n",Biddy_Managed_VariableTableNum(MNG));
#endif

  /* BDD SYSTEM BY VARIABLE LISTS */

#ifdef SYSTEMREPORTVERBOSE
  printf("SYSTEM REPORT (VARIABLE LISTS)\n");
#endif

  countfortified = 1;
  countprolonged = countfresh = countobsolete = 0;
  for (v=1; v<biddyVariableTable.num; v++) {

    BiddyNode *sup;
    Biddy_Boolean fresh,end;

    count1 = 0;
    count2 = 0;

#ifdef SYSTEMREPORTVERBOSE
    printf("VARIABLE: variable %s, num=%u\n",
      biddyVariableTable.table[v].name,biddyVariableTable.table[v].num);
#endif

    /* LIST OF NODES */

    sup = biddyVariableTable.table[v].firstNewNode;

    if (!sup) {
      fprintf(stderr,"ERROR (BiddySystemReport): Empty list\n");
      free((void *)1); /* BREAKPOINT */
    }

    if (!Biddy_IsEqv(sup->f,biddyZero) || !Biddy_IsEqv(sup->t,biddyOne)) {
      fprintf(stderr,"ERROR (BiddySystemReport): Bad first element in the list\n");
      free((void *)1); /* BREAKPOINT */
    }

    if (sup == biddyVariableTable.table[v].freshNodes) {
      fresh = TRUE;
    } else {
      fresh = FALSE;
    }

    do {

      if (sup->v != v) {
        fprintf(stderr,"ERROR (BiddySystemReport): Bad variable in list\n");
        fprintf(stderr,"VARIABLE: variable %s\n",biddyVariableTable.table[v].name);
        fprintf(stderr,"NODE: pointer=%p, variable %s, else=%s, then=%s\n",
          (void *) sup,
          Biddy_Managed_GetTopVariableName(MNG,sup),
          Biddy_Managed_GetTopVariableName(MNG,sup->f),
          Biddy_Managed_GetTopVariableName(MNG,sup->t));
        free((void *)1); /* BREAKPOINT */
      }

      if (fresh && sup->count && (sup->count < biddyCount)) {
        fprintf(stderr,"ERROR (BiddySystemReport): Obsolete node after freshNodes\n");
        fprintf(stderr,"VARIABLE: variable %s\n",biddyVariableTable.table[v].name);
        fprintf(stderr,"NODE: pointer=%p, variable %s, else=%s, then=%s\n",
          (void *) sup,
          Biddy_Managed_GetTopVariableName(MNG,sup),
          Biddy_Managed_GetTopVariableName(MNG,sup->f),
          Biddy_Managed_GetTopVariableName(MNG,sup->t));
        free((void *)1); /* BREAKPOINT */
      }

      if (( !(sup->count) &&
            (((BiddyNode *) Biddy_Regular(sup->f))->count ||
             ((BiddyNode *) sup->t)->count)
          ) || (
            (sup->count) &&
            ( (((BiddyNode *) Biddy_Regular(sup->f))->count &&
                (((BiddyNode *) Biddy_Regular(sup->f))->count < sup->count)) ||
              (((BiddyNode *) sup->t)->count &&
                (((BiddyNode *) sup->t)->count < sup->count)))
          ))
      {
        fprintf(stderr,"ERROR (BiddySystemReport): Bad node in list\n");
        fprintf(stderr,"biddyCount = %u, NODE: pointer=%p, variable %s (%d), else=%s (%d), then=%s (%d)\n",
          biddyCount,
          (void *) sup,
          Biddy_Managed_GetTopVariableName(MNG,sup),sup->count,
          Biddy_Managed_GetTopVariableName(MNG,sup->f),((BiddyNode *) Biddy_Regular(sup->f))->count,
          Biddy_Managed_GetTopVariableName(MNG,sup->t),((BiddyNode *) sup->t)->count);
        free((void *)1); /* BREAKPOINT */
      }

#ifdef SYSTEMREPORTDETAILS
      printf("NODE: pointer=%p, count=%d, variable %s, else=%s, then=%s\n",
        (void *) sup,sup->count,
        Biddy_Managed_GetTopVariableName(MNG,sup),
        Biddy_Managed_GetTopVariableName(MNG,sup->f),
        Biddy_Managed_GetTopVariableName(MNG,sup->t));
#endif

      if (!(sup->count)) {
        countfortified++;
      } else if (sup->count == biddyCount) {
        countfresh++;
      } else if (sup->count > biddyCount) {
        countprolonged++;
      } else {
        countobsolete++;
      }

      count1++;

      end = (sup == biddyVariableTable.table[v].lastNewNode);
      if (!end) sup = (BiddyNode *) sup->list;
    } while (!end);

    /* LIST OF FORTIFIED NODES */

    sup = biddyVariableTable.table[v].fortifiedNodes;
    while (sup) {

      if (sup->v != v) {
        fprintf(stderr,"ERROR (BiddySystemReport): Bad variable in fortified list\n");
        fprintf(stderr,"VARIABLE: variable %s\n",biddyVariableTable.table[v].name);
        fprintf(stderr,"NODE: pointer=%p, variable %s, else=%s, then=%s\n",
          (void *) sup,
          Biddy_Managed_GetTopVariableName(MNG,sup),
          Biddy_Managed_GetTopVariableName(MNG,sup->f),
          Biddy_Managed_GetTopVariableName(MNG,sup->t));
        free((void *)1); /* BREAKPOINT */
      }

      if (sup->count) {
        fprintf(stderr,"ERROR (BiddySystemReport): Bad node in list of fortified nodes\n");
        fprintf(stderr,"NODE: pointer=%p, variable %s (%d), else=%s (%d), then=%s (%d)\n",
          (void *) sup,
          Biddy_Managed_GetTopVariableName(MNG,sup),sup->count,
          Biddy_Managed_GetTopVariableName(MNG,sup->f),((BiddyNode *) Biddy_Regular(sup->f))->count,
          Biddy_Managed_GetTopVariableName(MNG,sup->t),((BiddyNode *) sup->t)->count);
        free((void *)1); /* BREAKPOINT */
      }

      if (((BiddyNode *) Biddy_Regular(sup->f))->count || ((BiddyNode *) sup->t)->count) {
        fprintf(stderr,"ERROR (BiddySystemReport): Bad node in list of fortified nodes\n");
        fprintf(stderr,"biddyCount = %u, NODE: pointer=%p, variable %s (%d), else=%s (%d), then=%s (%d)\n",
          biddyCount,
          (void *) sup,
          Biddy_Managed_GetTopVariableName(MNG,sup),sup->count,
          Biddy_Managed_GetTopVariableName(MNG,sup->f),((BiddyNode *) Biddy_Regular(sup->f))->count,
          Biddy_Managed_GetTopVariableName(MNG,sup->t),((BiddyNode *) sup->t)->count);
        free((void *)1); /* BREAKPOINT */
      }

      if (sup == biddyVariableTable.table[v].freshNodes) {
        fprintf(stderr,"ERROR (BiddySystemReport): Bad freshNodes pointer - inside fortified list\n");
        fprintf(stderr,"VARIABLE: variable %s\n",biddyVariableTable.table[v].name);
        fprintf(stderr,"NODE: pointer=%p, variable %s, else=%s, then=%s\n",
          (void *) sup,
          Biddy_Managed_GetTopVariableName(MNG,sup),
          Biddy_Managed_GetTopVariableName(MNG,sup->f),
          Biddy_Managed_GetTopVariableName(MNG,sup->t));
        free((void *)1); /* BREAKPOINT */
      }

#ifdef SYSTEMREPORTDETAILS
      printf("NODE: pointer=%p, count=%d, variable %s, else=%s, then=%s\n",
        (void *) sup,sup->count,
        Biddy_Managed_GetTopVariableName(MNG,sup),
        Biddy_Managed_GetTopVariableName(MNG,sup->f),
        Biddy_Managed_GetTopVariableName(MNG,sup->t));
#endif

      if (!(sup->count)) {
        countfortified++;
      } else if (sup->count == biddyCount) {
        countfresh++;
      } else if (sup->count > biddyCount) {
        countprolonged++;
      } else {
        countobsolete++;
      }

      count2++;

      sup = (BiddyNode *) sup->list;
    }

#ifdef SYSTEMREPORTDETAILS
    printf("firstNewNode: pointer=%p, freshNodes=%p, lastNewNode=%p, fortifiedNodes=%p\n",
      (void *) biddyVariableTable.table[v].firstNewNode,
      (void *) biddyVariableTable.table[v].freshNodes,
      (void *) biddyVariableTable.table[v].lastNewNode,
      (void *) biddyVariableTable.table[v].fortifiedNodes
    );
#endif

    if (biddyVariableTable.table[v].num != count1+count2) {
      fprintf(stderr,"ERROR (BiddySystemReport): wrong node number for a variable\n");
      fprintf(stderr,"VARIABLE: variable %s, num=%u\n",
        biddyVariableTable.table[v].name,biddyVariableTable.table[v].num);
      fprintf(stderr,"COUNT list= %u, fortified = %u, all = %u\n",count1,count2,count1+count2);
      free((void *)1); /* BREAKPOINT */
    }

#ifdef SYSTEMREPORTVERBOSE
    printf("COUNT listOfNewNodes = %u, listOfFortifiedNodes = %u, all = %u\n",count1,count2,count1+count2);
#endif

  }

#ifdef SYSTEMREPORTVERBOSE
  printf("numFortified: %u, numProlonged: %u, numFresh: %u, numBad: %u, total: %u\n",
                 countfortified,countprolonged,countfresh,countobsolete,
                 countfortified+countprolonged+countfresh+countobsolete
  );
#endif

  if (biddyNodeTable.num != countfortified+countprolonged+countfresh+countobsolete) {
    fprintf(stderr,"ERROR (BiddySystemReport): total number of nodes (%u by lists) differs from num = %u\n",
            countfortified+countprolonged+countfresh+countobsolete,
            biddyNodeTable.num);
    free((void *)1); /* BREAKPOINT */
  }
  if (biddyNodeTable.numf != countfortified) {
    fprintf(stderr,"ERROR (BiddySystemReport): number of fortified nodes (%u by lists) differs from numF = %u\n",
            countfortified,
            biddyNodeTable.numf);
    free((void *)1); /* BREAKPOINT */
  }

  /* BDD SYSTEM BY NODE TABLE */

#ifdef SYSTEMREPORTVERBOSE
  printf("SYSTEM REPORT (NODE TABLE)\n");
#endif

  count1 = 0;
  count2 = 1;
  countfortified = 1;
  countprolonged = countfresh = countobsolete = 0;
  for (i=1; i<biddyNodeTable.size+2; i++) {
    BiddyNode *sup;

    sup = biddyNodeTable.table[i];
    if (sup != NULL) {

      count1++;
      while (sup) {
        if (Biddy_GetTopVariable(sup) < biddyVariableTable.num) {
          count2++;
          if (!(sup->count)) {
            countfortified++;
          } else if (sup->count == biddyCount) {
            countfresh++;
          } else if (sup->count > biddyCount) {
            countprolonged++;
          } else {
            countobsolete++;
          }
        }

#ifdef SYSTEMREPORTDETAILS
        if (Biddy_GetTopVariable(sup) < biddyVariableTable.num) {
          printf("NODE: i=%u, pointer=%p, count=%d, variable %s, else=%s, then=%s\n",
            i, (void *) sup,sup->count,
            Biddy_Managed_GetTopVariableName(MNG,sup),
            Biddy_Managed_GetTopVariableName(MNG,sup->f),
            Biddy_Managed_GetTopVariableName(MNG,sup->t));
        } else {
          printf("NODE: i=%u, pointer=%p, count=%d, variable %u, else=%s, then=%s\n",
            i, (void *) sup,sup->count,
            Biddy_GetTopVariable(sup),
            Biddy_Managed_GetTopVariableName(MNG,sup->f),
            Biddy_Managed_GetTopVariableName(MNG,sup->t));
        }
#endif

        sup = sup->next;
      }
    }
  }

#ifdef SYSTEMREPORTVERBOSE
  printf("NODE TABLE HAS %u NON-EMPTY LISTS\n",count1);
  printf("NODE TABLE HAS %u NODES\n",count2);
#endif

  printf("numFortified: %u, numProlonged: %u, numFresh: %u, numBad: %u, total: %u\n",
         countfortified,countprolonged,countfresh,countobsolete,
         countfortified+countprolonged+countfresh+countobsolete
  );

  if (biddyNodeTable.num != countfortified+countprolonged+countfresh+countobsolete) {
    fprintf(stderr,"ERROR (BiddySystemReport): total number of nodes (%u by table) differs from num = %u\n",
            countfortified+countprolonged+countfresh+countobsolete,
            biddyNodeTable.num);
    free((void *)1); /* BREAKPOINT */
  }
  if (biddyNodeTable.numf != countfortified) {
    fprintf(stderr,"ERROR (BiddySystemReport): number of fortified nodes (%u by table) differs from numF = %u\n",
            countfortified,
            biddyNodeTable.numf);
    free((void *)1); /* BREAKPOINT */
  }

}

/*******************************************************************************
\brief Function BiddyFunctionReport is intended for debugging.

### Description
### Side effects
### More info
*******************************************************************************/

#define FUNCTIONREPORTVERBOSE
#undef FUNCTIONREPORTVERBOSE

#define FUNCTIONREPORTDETAILS
#undef FUNCTIONREPORTDETAILS

void
BiddyFunctionReport(Biddy_Manager MNG, Biddy_Edge f)
{
  unsigned int num;
  Biddy_Variable k,minvar,maxvar,minvarOrder,maxvarOrder;
  Biddy_Boolean stop;
  BiddyOrderingTable fOrdering;

  if (Biddy_IsNull(f)) {
    printf("NULL FUNCTION");
    return;
  }

  /* determine function ordering and node number */
  nullOrdering(fOrdering);
  num = 1;
  nodeNumberOrdering(f,&num,fOrdering);
  Biddy_NodeRepair(f);

  printf("FUNCTION: number of nodes: %d, count of top node: %d/%d\n",num,
          ((BiddyNode *) Biddy_Regular(f))->count,biddyCount);

#ifdef FUNCTIONREPORTVERBOSE
  printf("REPORT ON FUNCTION\n");
  /* determine highest (bottommost) variable */
  minvar = maxvar = Biddy_GetTopVariable(f);
  stop = FALSE;
  while (!stop) {
    stop = TRUE;
    for (k=1; k<biddyVariableTable.num; k++) {
      if (GET_ORDER(fOrdering,maxvar,k)) {
        maxvar = k;
        stop = FALSE;
        break;
      }
    }
  }
  minvarOrder = maxvarOrder = Biddy_VariableTableNum();
  for (k=0; k<Biddy_VariableTableNum(); k++) {
    if (GET_ORDER(biddyOrderingTable,minvar,k)) --minvarOrder;
    if (GET_ORDER(biddyOrderingTable,maxvar,k)) --maxvarOrder;
  }
  printf("FUNCTION: minvar: %s (id=%u, order=%u), maxvar: %s (id=%u, order=%u)\n",
          Biddy_Managed_GetVariableName(MNG,minvar),minvar,minvarOrder,
          Biddy_Managed_GetVariableName(MNG,maxvar),maxvar,maxvarOrder);
#endif

#ifdef FUNCTIONREPORTDETAILS
  printf("HERE IS BDD\n");
  writeBDD(f);
#endif

}

/*----------------------------------------------------------------------------*/
/* Definition of static functions                                             */
/*----------------------------------------------------------------------------*/

/*******************************************************************************
\brief Function nodeTableHash.

### Description
### Side effects
### More info
*******************************************************************************/

static unsigned int
nodeTableHash(Biddy_Variable v, Biddy_Edge pf, Biddy_Edge pt,
              unsigned int biddyNodeTableSize)
{
  unsigned int hash;

  /* THIS IS HASH FUNCTION FOR NODE TABLE */
  /* THE SIZE OF NODE TABLE IS biddyNodeTable.size+2 */
  /* BECAUSE OF USED TRICKS, HASH FUNCTION MUST NEVER RETURN ZERO! */
  /* USEFUL INDICES ARE FROM [1] TO [biddyNodeTable.size+1] */

  /* NAIVE 64-bit HASH FUNCTION */
  /*
  hash = 1 + ((((uintptr_t) v >> 0) +
               ((uintptr_t) pf >> 3) +
               ((uintptr_t) pt >> 5)
              ) & biddyNodeTableSize);
  */


#if UINTPTR_MAX == 0xffffffffffffffff

  /* MURMUR3 (A KIND OF) 64-bit HASH FUNCTION */
  /* https://code.google.com/p/smhasher/ */
  {
  const uint64_t C1 = 0x87c37b91114253d5L;
  const uint64_t C2 = 0x4cf5ad432745937fL;
  uintptr_t k1,k2,k;
  k = v;
  k1 = (uintptr_t) pf;
  k1 *= C1;
  k1 = (k1 << 31) | (k1 >> 33);
  k1 *= C2;
  k ^= k1;
  k = (k << 27) | (k >> 37);
  k = k*5+0x52dce729;
  k2 = (uintptr_t) pt;
  k2 *= C2;
  k2 = (k2 << 33) | (k2 >> 31);
  k2 *= C1;
  k ^= k2;
  k = (k << 31) | (k >> 33);
  k = k*5+0x38495ab5;
  k ^= k >> 33;
  k *= 0xff51afd7ed558ccdL;
  k ^= k >> 33;
  k *= 0xc4ceb9fe1a85ec53L;
  k ^= k >> 33;
  hash = 1 + (k & biddyNodeTableSize);
  }

#else

  /* MURMUR3 (A KIND OF) 32-bit HASH FUNCTION */
  /* https://code.google.com/p/smhasher/ */
  {
  const uint32_t C1 = 0xcc9e2d51;
  const uint32_t C2 = 0x1b873593;
  uintptr_t k1,k2,k;
  k = v;
  k1 = (uintptr_t) pf;
  k1 *= C1;
  k1 = (k1 << 15) | (k1 >> 17);
  k1 *= C2;
  k ^= k1;
  k = (k << 13) | (k >> 19);
  k = k*5+0xe6546b64;
  k2 = (uintptr_t) pt;
  k2 *= C1;
  k2 = (k2 << 15) | (k2 >> 17);
  k2 *= C2;
  k ^= k2;
  k = (k << 13) | (k >> 19);
  k = k*5+0xe6546b64;
  k ^= k >> 16;
  k *= 0x85ebca6b;
  k ^= k >> 13;
  k *= 0xc2b2ae35;
  k ^= k >> 16;
  hash = 1 + (k & biddyNodeTableSize);
  }

#endif

  return hash;

}

/*******************************************************************************
\brief Function ITEHash.

### Description
### Side effects
### More info
*******************************************************************************/

static unsigned int
ITEHash(Biddy_Edge a, Biddy_Edge b, Biddy_Edge c,
        unsigned int biddyIteCacheSize)
{

  unsigned int indeks;

  /* NAIVE 64-bit HASH FUNCTION */
  /*
  indeks = ( (uintptr_t) a +
             ((uintptr_t) b >> 5) +
             ((uintptr_t) c >> 7)
           ) & biddyIteCacheSize;
  */

#if UINTPTR_MAX == 0xffffffffffffffff

  /* MURMUR3 (A KIND OF) 64-bit HASH FUNCTION */
  /* https://code.google.com/p/smhasher/ */
  {
  const uint64_t C1 = 0x87c37b91114253d5L;
  const uint64_t C2 = 0x4cf5ad432745937fL;
  uintptr_t k1,k2,k;
  k = (uintptr_t) a;
  k1 = (uintptr_t) b;
  k1 *= C1;
  k1 = (k1 << 31) | (k1 >> 33);
  k1 *= C2;
  k ^= k1;
  k = (k << 27) | (k >> 37);
  k = k*5+0x52dce729;
  k2 = (uintptr_t) c;
  k2 *= C2;
  k2 = (k2 << 33) | (k2 >> 31);
  k2 *= C1;
  k ^= k2;
  k = (k << 31) | (k >> 33);
  k = k*5+0x38495ab5;
  k ^= k >> 33;
  k *= 0xff51afd7ed558ccdL;
  k ^= k >> 33;
  k *= 0xc4ceb9fe1a85ec53L;
  k ^= k >> 33;
  indeks = k & biddyIteCacheSize;
  }

#else

  /* MURMUR3 (A KIND OF) 32-bit HASH FUNCTION */
  /* https://code.google.com/p/smhasher/ */
  {
  const uint32_t C1 = 0xcc9e2d51;
  const uint32_t C2 = 0x1b873593;
  uintptr_t k1,k2,k;
  k = (uintptr_t) a;
  k1 = (uintptr_t) b;
  k1 *= C1;
  k1 = (k1 << 15) | (k1 >> 17);
  k1 *= C2;
  k ^= k1;
  k = (k << 13) | (k >> 19);
  k = k*5+0xe6546b64;
  k2 = (uintptr_t) c;
  k2 *= C1;
  k2 = (k2 << 15) | (k2 >> 17);
  k2 *= C2;
  k ^= k2;
  k = (k << 13) | (k >> 19);
  k = k*5+0xe6546b64;
  k ^= k >> 16;
  k *= 0x85ebca6b;
  k ^= k >> 13;
  k *= 0xc2b2ae35;
  k ^= k >> 16;
  indeks = k & biddyIteCacheSize;
  }

#endif

  return indeks;
}

/*******************************************************************************
\brief Function exchangeEdges exchanges two functions.

### Description
### Side effects
### More info
*******************************************************************************/

static void
exchangeEdges(Biddy_Edge *f, Biddy_Edge *g)
{
  Biddy_Edge sup;

  sup = *f;
  *f = *g;
  *g = sup;
}

/*******************************************************************************
\brief Function complExchangeEdges complements and exchanges two functions.

### Description
### Side effects
### More info
*******************************************************************************/

static void
complExchangeEdges(Biddy_Edge *f, Biddy_Edge *g)
{
  Biddy_Edge sup;

  sup = *f;
  *f = *g;
  *g = sup;
  Biddy_InvertMark((*f));
  Biddy_InvertMark((*g));
}

/*******************************************************************************
\brief Function addNodeTable adds node to the table, node itself is not
       changed in any way.

### Description
    There are two options: the node can be inserted at the beginning or
    immediately after sup1 (in this case lists can be ordered).
### Side effects
    This is low level functon. It is not checked, if the same node already
    exists. Sifting may require insertion after sup1!
### More info
*******************************************************************************/

static void
addNodeTable(Biddy_Manager MNG, unsigned int hash, BiddyNode *node,
             BiddyNode *sup1)
{

    /* ******************************************************************* */
    /* OPTION                                                              */
    /* USE THIS TO INSERT NODE AFTER sup1                                  */
    /* IF sup1 = NULL THEN NEW NODE WILL BE INSERTED AT THE BEGINNING      */
    /* IF sup1 = LAST NODE THEN NEW NODE WILL BE INSERTED AT THE END       */
    /* ******************************************************************* */

    /* THE FIRST ELEMENT OF EACH CHAIN SHOULD HAVE A SPECIAL VALUE */
    /* FOR ITS PREV ELEMENT TO ALLOW TRICKY BUT EFFICIENT DELETING */

    node->prev = sup1 ? sup1 : (BiddyNode *) &biddyNodeTable.table[hash-1];
    node->next = node->prev->next;
    node->prev->next = node;
    if (node->next) node->next->prev = node;

    /* ******************************************************************* */
    /* OPTION                                                              */
    /* USE THIS TO ALWAYS INSERT NODE AT THE BEGINNING                     */
    /* ******************************************************************* */

    /* THE FIRST ELEMENT OF EACH CHAIN SHOULD HAVE A SPECIAL VALUE */
    /* FOR ITS PREV ELEMENT TO ALLOW TRICKY BUT EFFICIENT DELETING */

    /*
    node->prev = (BiddyNode *) &biddyNodeTable.table[hash-1];
    node->next = biddyNodeTable.table[hash];
    biddyNodeTable.table[hash] = node;
    if (node->next) node->next->prev = node;
    */

    /* ******************************************************************* */
    /* END OF OPTIONS                                                      */
    /* ******************************************************************* */
}

/*******************************************************************************
\brief Function findNodeTable checks for the node in the chain referenced
       by thesup.

### Description
    Return NULL and (*thesup = NULL) iff node does not exists and
           chain is empty.
    Return lastNodeInChain and (*thesup = NULL) or
           nodeBeforeNodeWithWrongVariable and (*thesup = nodeWithWrongVariable)
           iff node does not exists and chain is not empty.
    Return NULL and (*thesup = theNode) iff node exists and
           it is first element in the chain.
    Return nodeBeforeTheNode and (*thesup = theNode) iff node exists and
           it is not first element in the chain.
### Side effects
### More info
*******************************************************************************/

static BiddyNode *
findNodeTable(Biddy_Manager MNG, Biddy_Variable v, Biddy_Edge pf, Biddy_Edge pt,
              BiddyNode **thesup)
{
  BiddyNode *sup,*sup1;

  sup = *thesup;
  sup1 = NULL;

  /* ********************************************************************* */
  /* OPTION                                                                */
  /* USE THIS IF YOU HAVE ORDERED CHAINS IN HASH TABLE                     */
  /* IT MUST BE COMBINED WITH OPTION "INSERTING NODES AFTER sup1"          */
  /* ********************************************************************* */

  /**/
  while (sup && (v < sup->v))
  {
    biddyNodeTable.compare++;
    sup1 = sup;
    sup = sup->next;
  }

  while (sup &&
          (v == sup->v) &&
          (!Biddy_IsEqv(pf,sup->f) || !Biddy_IsEqv(pt,sup->t))
        )
  /**/

  /* ********************************************************************* */
  /* OPTION                                                                */
  /* USE THIS IF YOU ALLOW CHAINS TO BE PURE RANDOM                        */
  /* FOR EXAMPLE, IF YOU ALWAYS INSERT AT THE BEGINNING OF A CHAIN         */
  /* ********************************************************************* */

  /*
  while (sup && (
          !Biddy_IsEqv(pf,sup->f) ||
          !Biddy_IsEqv(pt,sup->t) ||
          (v != sup->v)
        ))
  */

  /* ********************************************************************* */
  /* END OF OPTIONS                                                        */
  /* ********************************************************************* */

  {
    biddyNodeTable.compare++;
    sup1 = sup;
    sup = sup->next;
  }

  *thesup = sup;
  return sup1;
}

/*******************************************************************************
\brief Function addIteCache adds a result to the ITE cache.

### Description
### Side effects
### More info
*******************************************************************************/

static void
addITECache(Biddy_Manager MNG, Biddy_Edge a, Biddy_Edge b, Biddy_Edge c,
            Biddy_Edge r)
{
  unsigned int indeks;
  BiddyOp3Cache *p;

  /* THIS IS HASH FUNCTION */
  /* MUST BE THE SAME AS IN FindIteCache! */

  indeks = ITEHash(a,b,c,biddyIteCache.size);

  p = &biddyIteCache.table[indeks];
  if (!Biddy_IsNull(p->result)) {
    /* THE CELL IS VALID, THUS THIS IS OVERWRITING */
    biddyIteCache.overwrite++;
  }

  p->f = a;
  p->g = b;
  p->h = c;
  p->result = r;
}

/*******************************************************************************
\brief Function findIteCache looks for the result in the ITE cache.

### Description
### Side effects
### More info
*******************************************************************************/

static Biddy_Boolean
findITECache(Biddy_Manager MNG, Biddy_Edge a, Biddy_Edge b, Biddy_Edge c,
             Biddy_Edge *r)
{
  unsigned int indeks;
  BiddyOp3Cache *p;

  /* THIS IS HASH FUNCTION */
  /* MUST BE THE SAME AS IN AddIteCache! */

  indeks = ITEHash(a,b,c,biddyIteCache.size);

  biddyIteCache.search++;
  p = &biddyIteCache.table[indeks];

  if (!Biddy_IsNull(p->result) &&
      Biddy_IsEqv(p->f,a) &&
      Biddy_IsEqv(p->g,b) &&
      Biddy_IsEqv(p->h,c)
     )
  {
    biddyIteCache.find++;
    *r = p->result;
    return TRUE;
  }

  return FALSE;
}

/*******************************************************************************
\brief Function ITEGarbage performs Garbage Collection for the ITE cache.

### Description
### Side effects
### More info
*******************************************************************************/

static void
ITEGarbage(Biddy_Manager MNG)
{
  int j;
  BiddyOp3Cache *c;

  for (j=0; j<=biddyIteCache.size; j++) {
    if (!Biddy_IsNull(biddyIteCache.table[j].result)) {
      c = &biddyIteCache.table[j];
      if (!BiddyIsOK(c->f) ||
          !BiddyIsOK(c->g) ||
          !BiddyIsOK(c->h) ||
          !BiddyIsOK(c->result))
      {
        c->result = biddyNull;
      }
    }
  }
}

/*******************************************************************************
\brief Function addEACache adds a result to the EA cache.

### Description
### Side effects
### More info
*******************************************************************************/

static void
addEACache(Biddy_Manager MNG, Biddy_Edge f, Biddy_Variable v, Biddy_Edge r)
{
  /* NOT IMPLEMENTED, YET */
}

/*******************************************************************************
\brief Function findEACache looks for the result in the EA cache.

### Description
### Side effects
### More info
*******************************************************************************/

static Biddy_Boolean
findEACache(Biddy_Manager MNG, Biddy_Edge f, Biddy_Variable v, Biddy_Edge *r)
{
  /* NOT IMPLEMENTED, YET */
  return FALSE;
}

/*******************************************************************************
\brief Function EAGarbage performs Garbage Collection for the EA cache.

### Description
### Side effects
### More info
*******************************************************************************/

static void
EAGarbage(Biddy_Manager MNG)
{
  int j;
  BiddyEACache *c;

  for (j=0; j<=biddyEACache.size; j++) {
    if (!Biddy_IsNull(biddyEACache.table[j].result)) {
      c = &biddyEACache.table[j];
      if (!BiddyIsOK(c->f) ||
          !BiddyIsOK(c->result))
      {
        c->result = biddyNull;
      }
    }
  }
}

/*******************************************************************************
\brief Function addRCCache adds a result to the RC cache.

### Description
### Side effects
### More info
*******************************************************************************/

static void
addRCCache(Biddy_Manager MNG, Biddy_Edge f, Biddy_Edge g, Biddy_Variable v,
           Biddy_Edge r)
{
  /* NOT IMPLEMENTED, YET */
}

/*******************************************************************************
\brief Function findRCCache looks for the result in the RC cache.

### Description
### Side effects
### More info
*******************************************************************************/

static Biddy_Boolean
findRCCache(Biddy_Manager MNG, Biddy_Edge f, Biddy_Edge g, Biddy_Variable v,
            Biddy_Edge *r)
{
  /* NOT IMPLEMENTED, YET */
  return FALSE;
}

/*******************************************************************************
\brief Function RCGarbage performs Garbage Collection for the RC cache.

### Description
### Side effects
### More info
*******************************************************************************/

static void
RCGarbage(Biddy_Manager MNG)
{
  int j;
  BiddyRCCache *c;

  for (j=0; j<=biddyRCCache.size; j++) {
    if (!Biddy_IsNull(biddyRCCache.table[j].result)) {
      c = &biddyRCCache.table[j];
      if (!BiddyIsOK(c->f) ||
          !BiddyIsOK(c->g) ||
          !BiddyIsOK(c->result))
      {
        c->result = biddyNull;
      }
    }
  }
}

/*******************************************************************************
\brief Function swapWithHigher determines higher variable and calls swapping.

### Description
### Side effects
### More info
*******************************************************************************/

static Biddy_Variable
swapWithHigher(Biddy_Manager MNG, Biddy_Variable v)
{
  Biddy_Variable vhigh;
  Biddy_Variable i;

  vhigh = 0;
  for (i=1; i<biddyVariableTable.num; i++) {
    if (Biddy_Managed_IsSmaller(MNG,v,i) &&
        Biddy_Managed_IsSmaller(MNG,i,vhigh)) vhigh = i;
  }

  /* DEBUGGING */
  /*
  printf("swapWithHigher: name=%s, v=%u, vlow=%u\n",biddyVariableTable.table[v].name,v,vhigh);
  */

  if (vhigh) {
    swapVariables(MNG,v,vhigh);
  }

  /* DEBUGGING */
  /*
  BiddySystemReport(MNG);
  */

  return vhigh;
}

/*******************************************************************************
\brief Function swapWithLower determines lower variable and calls swapping.

### Description
### Side effects
### More info
*******************************************************************************/

static Biddy_Variable
swapWithLower(Biddy_Manager MNG, Biddy_Variable v)
{
  Biddy_Variable vlow;
  Biddy_Variable i;
  Biddy_Boolean first;

  vlow = 0;
  first = TRUE;
  for (i=1; i<biddyVariableTable.num; i++) {
    if (first) {
      if (Biddy_Managed_IsSmaller(MNG,i,v)) {
        vlow = i;
        first = FALSE;
      }
    } else {
      if (Biddy_Managed_IsSmaller(MNG,i,v) &&
          Biddy_Managed_IsSmaller(MNG,vlow,i)) vlow = i;
    }
  }

  /* DEBUGGING */
  /*
  printf("swapWithLower: name=%s, v=%u, vlow=%u\n",biddyVariableTable.table[v].name,v,vlow);
  */

  if (vlow) {
    swapVariables(MNG,vlow,v);
  }

  /* DEBUGGING */
  /*
  BiddySystemReport(MNG);
  */

  return vlow;
}

/*******************************************************************************
\brief Function swapVariables swaps two variables in a BDD system.

### Description
    If variable low is above (above = topmore) variable high then these two
    variables are swapped.
### Side effects
    It is not checked that low is next to high.
    The result is wrong if low is not next to high!
### More info
*******************************************************************************/

static void
swapVariables(Biddy_Manager MNG, Biddy_Variable low, Biddy_Variable high)
{
  BiddyNode *sup,*tmp,*tmp1;
  BiddyNode *newBegin,*newEnd,*newFresh;
  Biddy_Edge f00,f01,f10,f11,u0,u1;
  unsigned int hash;
  Biddy_Variable v;
  unsigned int n;
  Biddy_Boolean repairNeeded;
  int i;

  if (!low || !high) return;
  biddyNodeTable.swap++;

  /* DEBUGGING */
  /*
  printf("SWAPPING START: low =  %s, high = %s\n",
         Biddy_Managed_GetVariableName(MNG,low),
         Biddy_Managed_GetVariableName(MNG,high));
  BiddySystemReport(MNG);
  */

  /* update active ordering */
  CLEAR_ORDER(biddyOrderingTable,low,high);
  SET_ORDER(biddyOrderingTable,high,low);

  sup = newBegin = biddyVariableTable.table[low].firstNewNode;
  newEnd = newFresh = NULL;

  /* connect two lists */
  biddyVariableTable.table[low].lastNewNode->list =
    (void *) biddyVariableTable.table[low].fortifiedNodes;

  /* PREPARE NEW LIST FOR 'LOW' */
  /* CONSTANT NODE "1" IS USED HERE BUT ONLY AS TMP */
  /* DO NOT CHANGE freshNodes HERE, ITS CURRENT VALUE IS USED IN THE LOOP */
  biddyVariableTable.table[low].firstNewNode =
    biddyVariableTable.table[low].lastNewNode = biddyNodeTable.table[0];

  repairNeeded = FALSE;
  while (sup) {

    /* ALL NODES WITH VARIABLE low AND SUCCESSOR high ARE SWAPPED */
    if ((Biddy_GetTopVariable(sup->f) == high) || (Biddy_GetTopVariable(sup->t) == high)) {

      /* SWAP */
      /* this will add zero, one or two new nodes with variable 'low' */
      /* if FoaNode adds new node it will be added to new list for 'low' */
      /* if FoaNode returns existing node with label 'low', */
      /* then it will be relisted to new list for 'low' when it will be checked */
      /* (all existing variables are checked!) */
      
      if (Biddy_GetTopVariable(sup->f) == high) {
        f00 = BiddyE(sup->f);
        f01 = BiddyT(sup->f);
        if (Biddy_GetMark(sup->f)) {
          Biddy_InvertMark(f00);
          Biddy_InvertMark(f01);
        }
        /* sup->f is not needed in sup, anymore */
        /* we will mark it as obsolete and we are expecting, that */
        /* formulae are repaired before garbage collection */
        if (((BiddyNode *) Biddy_Regular(sup->f))->count == 0) {
          biddyNodeTable.numf--;
        }
        ((BiddyNode *) Biddy_Regular(sup->f))->count = biddyCount-1;
        repairNeeded = TRUE;
      } else {
        f00 = f01 = sup->f;
      }

      if (Biddy_GetTopVariable(sup->t) == high) {
        f10 = BiddyE(sup->t);
        f11 = BiddyT(sup->t);
        /* sup->t is not needed in sup, anymore */
        /* we will mark it as obsolete and we are expecting, that */
        /* formulae are repaired before garbage collection */
        if (((BiddyNode *) sup->t)->count == 0) {
          biddyNodeTable.numf--;
        }
        ((BiddyNode *) sup->t)->count = biddyCount-1;
        repairNeeded = TRUE;
      } else {
        f10 = f11 = sup->t;
      }

      if (Biddy_IsEqv(f00,f10)) {
        u0 = f00;
      } else {
        /* u0 is a new node or an existing node */
        /* in any case, it is a needed node */
        /* in any case, GC will not be called during FoaNode */
        u0 = Biddy_Managed_FoaNode(MNG,low,f00,f10,FALSE);
        if (!(sup->count)) {

          if (((BiddyNode *) Biddy_Regular(u0))->count) {

            /* DEBUGGING */
            /*
            if (((BiddyNode *) Biddy_Regular(u0))->v != low) {
              fprintf(stdout,"ERROR (swapVariables): LOW!!!\n");
              exit(1);
            }
            */

            ((BiddyNode *) Biddy_Regular(u0))->count = 0;
            biddyNodeTable.numf++;

          }

        } else {

          if (((BiddyNode *) Biddy_Regular(u0))->count &&
              (((BiddyNode *) Biddy_Regular(u0))->count < sup->count)) {

            /* DEBUGGING */
            /*
            if (((BiddyNode *) Biddy_Regular(u0))->v != low) {
              fprintf(stdout,"ERROR (swapVariables): LOW!!!\n");
              exit(1);
            }
            */

            ((BiddyNode *) Biddy_Regular(u0))->count = sup->count;
          }

        } 

      }

      if (Biddy_IsEqv(f01,f11)) {
        u1 = f01;
      } else {
        /* u1 is a new node or an existing node */
        /* in any case, it is a needed node */
        /* in any case, GC will not be called during FoaNode */
        u1 = Biddy_Managed_FoaNode(MNG,low,f01,f11,FALSE);

        if (!(sup->count)) {

          if (((BiddyNode *) Biddy_Regular(u1))->count) {

            /* DEBUGGING */
            /*
            if (((BiddyNode *) Biddy_Regular(u1))->v != low) {
              fprintf(stdout,"ERROR (swapVariables): LOW!!!\n");
              exit(1);
            }
            */

            ((BiddyNode *) Biddy_Regular(u1))->count = 0;
            biddyNodeTable.numf++;

          }

        } else {

          if (((BiddyNode *) Biddy_Regular(u1))->count &&
              (((BiddyNode *) Biddy_Regular(u1))->count < sup->count)) {

            /* DEBUGGING */
            /*
            if (((BiddyNode *) Biddy_Regular(u1))->v != low) {
              fprintf(stdout,"ERROR (swapVariables): LOW!!!\n");
              exit(1);
            }
            */

            ((BiddyNode *) Biddy_Regular(u1))->count = sup->count;
          }

        }

      }

      sup->v = high;
      sup->f = u0;
      sup->t = u1;

      (biddyVariableTable.table[low].num)--;
      (biddyVariableTable.table[high].num)++;

      /* rehash this node */
      sup->prev->next = sup->next; /* remove node from the old chain */
      if (sup->next) sup->next->prev = sup->prev; /* remove node from the old chain */
      hash = nodeTableHash(high,u0,u1,biddyNodeTable.size);
      tmp = biddyNodeTable.table[hash]; /* the beginning of new chain */
      tmp1 = findNodeTable(MNG,high,u0,u1,&tmp); /* such node cannot exists! */
      addNodeTable(MNG,hash,sup,tmp1); /* add node to hash table */

      newEnd = sup;

      /* when node pointed by freshNodes is swapped then remember it */
      /* it will be used to optimize the position of new freshNodes pointer */
      if (biddyVariableTable.table[low].freshNodes == sup) newFresh = sup;

    } else {

      /* this node with variable 'low' has not been changed to 'high' */
      /* because it does not have successors with variable 'high' */

      /* remove this node from list of renamed nodes (newBegin,newEnd) */
      if (!newEnd) {
        newBegin = (BiddyNode *) sup->list;
      } else {
        newEnd->list = sup->list;
      }
      /* relink to the end of list for low */
      biddyVariableTable.table[low].lastNewNode->list = (void *) sup;
      biddyVariableTable.table[low].lastNewNode = sup;

    }

    sup = (BiddyNode *) sup->list;
  }

  /* all nodes in the list from newBegin to newEnd (inclusively) are renamed */
  /* thus insert the complete list into list for 'high' */
  if (newEnd) {
    if (biddyVariableTable.table[high].freshNodes == biddyVariableTable.table[high].lastNewNode) {
      biddyVariableTable.table[high].lastNewNode = newEnd;
    } else {
      newEnd->list = (biddyVariableTable.table[high].freshNodes)->list;
    }
    (biddyVariableTable.table[high].freshNodes)->list = (void *) newBegin;
    if (newFresh) {
      biddyVariableTable.table[high].freshNodes = newFresh;
    } else {
      biddyVariableTable.table[high].freshNodes = newEnd;
    }
  }

  /* manage new lists for 'low' */
  /* at least one new node has been added to new list for 'low' */
  /* e.g. node representing variable should not be renamed to 'high' */
  if (biddyVariableTable.table[low].lastNewNode == biddyNodeTable.table[0]) {
    fprintf(stdout,"ERROR (swapVariables): nodes not added to new list for low = %s\n",
            Biddy_Managed_GetVariableName(MNG,low));
    exit(1);
  }

  biddyVariableTable.table[low].firstNewNode = 
    (BiddyNode *) (biddyVariableTable.table[low].firstNewNode)->list;
  biddyVariableTable.table[low].freshNodes = biddyVariableTable.table[low].lastNewNode;
  biddyVariableTable.table[low].fortifiedNodes = NULL; /* wait for next garbage */

  /* REPAIR CONSTANT NODE */
  biddyNodeTable.table[0]->list = NULL;
  
  /* REMOVE BROKEN LIST OF FORTIFIED NODES AND REPAIR BROKEN NODES */
  /* YOU HAVE TO PRESERVE FIRST NODE IN THE LIST */
  if (repairNeeded) {
    if (biddyVariableTable.table[high].fortifiedNodes) {
      tmp = biddyVariableTable.table[high].fortifiedNodes;
      while (tmp->list) {
        tmp = (BiddyNode *) tmp->list;
      }
      biddyVariableTable.table[high].lastNewNode->list =
        biddyVariableTable.table[high].fortifiedNodes;
      biddyVariableTable.table[high].lastNewNode = tmp;
      biddyVariableTable.table[high].fortifiedNodes = NULL;
    }
    biddyVariableTable.table[high].freshNodes =
      biddyVariableTable.table[high].lastNewNode;

    /* VARIANT 1: BY USING ITERATIVE CHECKING */
    /* THIS SEEMS TO BE FASTER */
    /**/
    for (n = 0; n < biddyFormulaTable.size; n++) {
      if (!Biddy_IsNull(biddyFormulaTable.table[n].f)) {
        if (((BiddyNode *) Biddy_Regular(biddyFormulaTable.table[n].f))->count) {
          if (!biddyFormulaTable.table[n].count) {
            biddyNodeTable.numf++;
          }
          ((BiddyNode *) Biddy_Regular(biddyFormulaTable.table[n].f))->count =
                           biddyFormulaTable.table[n].count;
        }
      }
    }
    BiddyRepair1(MNG,high);
    /**/

    /* VARIANT 2: BY USING RECURSIVE CHECKING */
    /* THIS SEEMS TO BE SLOWER */
    /*
    Biddy_SelectNode(biddyOne);
    for (n = 0; n < biddyFormulaTable.size; n++) {
      if (!Biddy_IsNull(biddyFormulaTable.table[n].f)) {
        BiddyRepair2(MNG,biddyFormulaTable.table[n].f,high,
                       biddyFormulaTable.table[n].count);
      }
    }
    Biddy_DeselectNode(biddyOne);
    for (n = 0; n < biddyFormulaTable.size; n++) {
      if (!Biddy_IsNull(biddyFormulaTable.table[n].f)) {
        Biddy_NodeRepair(biddyFormulaTable.table[n].f);
      }
    }
    */

  }

  /* DEBUGGING */
  /*
  printf("SWAPPING FINISH: low =  %s, high = %s\n",
         Biddy_Managed_GetVariableName(MNG,low),
         Biddy_Managed_GetVariableName(MNG,high));
  BiddySystemReport(MNG);
  */

}

/*******************************************************************************
\brief Function nullOrdering.

### Description
### Side effects
### More info
*******************************************************************************/

static void
nullOrdering(BiddyOrderingTable table)
{
  int i,j;

  for (i=0; i<BIDDYVARMAX; i++) {
    for (j=0; j<=(BIDDYVARMAX-1)/UINTPTRSIZE; j++) {
      table[i][j]= (UINTPTR) 0;
    }
  }
}

/*******************************************************************************
\brief Function nodeNumberOrdering.

### Description
### Side effects
    Ordering of constant node is not included!
    All nodes except constant node will be selected.
### More info
*******************************************************************************/

static void
nodeNumberOrdering(Biddy_Edge f, unsigned int *i, BiddyOrderingTable ordering)
{
  Biddy_Edge sup;

  if (!Biddy_IsConstant(f)) {
    Biddy_SelectNode(f);
    (*i)++;
    if (!Biddy_IsConstant((sup=BiddyE(f)))) {
      SET_ORDER(ordering,Biddy_GetTopVariable(f),Biddy_GetTopVariable(sup));
      if (!Biddy_IsSelected(sup)) nodeNumberOrdering(sup,i,ordering);
    }
    if (!Biddy_IsConstant((sup=BiddyT(f)))) {
      SET_ORDER(ordering,Biddy_GetTopVariable(f),Biddy_GetTopVariable(sup));
      if (!Biddy_IsSelected(sup)) nodeNumberOrdering(sup,i,ordering);
    }
  }
}

/*******************************************************************************
\brief Function warshall.

### Description
    Transitive closure of directed graphs (Warshall's algorithm).
    http://en.wikipedia.org/wiki/Floyd%E2%80%93Warshall_algorithm
    sizeof(table) = [BIDDYVARMAX] [1+(BIDDYVARMAX-1)/UINTPTRSIZE]
    table[X,Y]==1 iff variabe X is smaller than variable Y
### Side effects
### More info
*******************************************************************************/

static void
warshall(BiddyOrderingTable table, Biddy_Variable varnum)
{
  Biddy_Variable i,j,k;
  UINTPTR jbit,kbit;
  kbit = (UINTPTR) 1;
  for (k=0; k<varnum; k++) {
    for (i=0; i<varnum; i++) {
      jbit = (UINTPTR) 1;
      for (j=0; j<varnum; j++) {
        if (!(table[i][j/UINTPTRSIZE]&jbit)) {
          if ((table[i][k/UINTPTRSIZE]&kbit) &&
              (table[k][j/UINTPTRSIZE]&jbit))
          {
            //printf("set bit [%d][%d]\n",i,j);
            table[i][j/UINTPTRSIZE] |= jbit;
          }
        }
        if (!(jbit <<= 1)) jbit = (UINTPTR) 1;
      }
    }
    if (!(kbit <<= 1)) kbit = (UINTPTR) 1;
  }
}

/*******************************************************************************
\brief Function writeBDD.

### Description
### Side effects
### More info
*******************************************************************************/

static void
writeBDD(Biddy_Manager MNG, Biddy_Edge f)
{
  if (Biddy_GetMark(f)) {
    fprintf(stdout,"* ");
  }
  fprintf(stdout,"%s",Biddy_Managed_GetTopVariableName(MNG,f));
  if (!Biddy_IsConstant(f)) {
    fprintf(stdout," (");
    writeBDD(MNG,BiddyE(f));
    fprintf(stdout,") (");
    writeBDD(MNG,BiddyT(f));
    fprintf(stdout,")");
  }
}

/*******************************************************************************
\brief Function writeORDERING.

### Description
### Side effects
### More info
*******************************************************************************/

static void
writeORDERING(Biddy_Manager MNG, BiddyOrderingTable table,
              Biddy_Variable varnum)
{
  Biddy_Variable i,j;
  UINTPTR jbit;
  for (i=0; i<varnum; i++) {
    printf("\"%s\"",biddyVariableTable.table[i].name);
    jbit = (UINTPTR) 1;
    for (j=0; j<varnum; j++) {
      if (!(table[i][j/UINTPTRSIZE]&jbit)) {
        printf(" 0");
      } else {
        printf(" 1");
      }
      if (!(jbit <<= 1)) jbit = (UINTPTR) 1;
    }
    printf("\n");
  }
}

/*******************************************************************************
\brief Function writeORDER.

### Description
### Side effects
### More info
*******************************************************************************/

static void
writeORDER(Biddy_Manager MNG)
{
  Biddy_Variable varTable[BIDDYVARMAX];
  Biddy_Variable k,v,varOrder;

  for (k=0; k<Biddy_Managed_VariableTableNum(MNG); k++) {
    varTable[k] = 0;
  }

  for (v=0; v<Biddy_Managed_VariableTableNum(MNG); v++) {
    varOrder = Biddy_Managed_VariableTableNum(MNG);
    for (k=0; k<Biddy_Managed_VariableTableNum(MNG); k++) {
      if (GET_ORDER(biddyOrderingTable,v,k)) varOrder--;
    }
    varTable[varOrder-1] = v;
  }

  for (k=0; k<Biddy_Managed_VariableTableNum(MNG); k++) {
    printf("%s",Biddy_Managed_GetVariableName(MNG,varTable[k]));
    if (k!=Biddy_Managed_VariableTableNum(MNG)-1) printf(",");
  }
  printf("\n");
}

/*******************************************************************************
\brief Function createLocalInfo recursively creates local info for all nodes
       except constant node.

### Description
### Side effects
    All nodes except constant node must be selected!
    Function createLocalInfo will deselect them.
### More info
*******************************************************************************/

static BiddyLocalInfo *
createLocalInfo(Biddy_Edge f, BiddyLocalInfo *c)
{
  /* FIELD data.* IS NOT INITIALIED! */

  if (Biddy_IsSelected(f)) {
    Biddy_DeselectNode(f);
    c->org = (BiddyNode *)((BiddyNode *) Biddy_Regular(f))->list;
    ((BiddyNode *) Biddy_Regular(f))->list = (void *) c;
    c->positiveSelected = FALSE;
    c->negativeSelected = FALSE;
    c = &c[1]; /* next field in the array */
    c = createLocalInfo(BiddyE(f),c);
    c = createLocalInfo(BiddyT(f),c);
  }
  return c;
}

/*******************************************************************************
\brief Function deleteLocalInfo recursively deletes local info for all nodes
       except constant node.

### Description
### Side effects
    All nodes except constant node must be selected!
    Function deleteLocalInfo will deselect them.
### More info
*******************************************************************************/

static void
deleteLocalInfo(Biddy_Edge f)
{
  if (Biddy_IsSelected(f)) {
    Biddy_DeselectNode(f);
    ((BiddyNode *) Biddy_Regular(f))->list = (void *)
      ((BiddyLocalInfo *)((BiddyNode *) Biddy_Regular(f))->list)->org;
    deleteLocalInfo(BiddyE(f));
    deleteLocalInfo(BiddyT(f));
  }
}

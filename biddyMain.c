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
    Revision    [$Revision: 174 $]
    Date        [$Date: 2016-07-03 16:54:17 +0200 (ned, 03 jul 2016) $]
    Authors     [Robert Meolic (robert.meolic@um.si),
                 Ales Casar (ales@homemade.net)]

### Copyright

Copyright (C) 2006, 2016 UM-FERI, Smetanova ulica 17, SI-2000 Maribor, Slovenia

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
/* e.g. 2^16-1 = 65535, 2^20-1 = 1048575, 2^24-1 = 16777215        */

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

/* If BDD node is 48B (e.g. 64-bit GNU/Linux systems), then:       */
/* 256 nodes is 12 kB                                              */
/* 65536 nodes is 3.0 MB                                           */
/* 262144 nodes is 12 MB                                           */
/* 524288 nodes is 24 MB                                           */
/* 1048576 nodes is 48.0 MB                                        */
/* 2097152 nodes is 96.0 MB                                        */
/* 4194304 nodes is 192.0 MB                                       */
/* 8388608 nodes is 384.0 MB                                       */
/* 16777216 nodes is 768.0 MB                                      */

#define TINY_SIZE        1023
#define SMALL_SIZE      65535
#define MEDIUM_SIZE    262143
#define LARGE_SIZE    1048575
#define XLARGE_SIZE   2097151
#define XXLARGE_SIZE  4194303
#define XXXLARGE_SIZE 8388607
#define HUGE_SIZE    16777215

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

Biddy_Edge debug_edge = NULL; /* debugging, only */

/*----------------------------------------------------------------------------*/
/* Static function prototypes                                                 */
/*----------------------------------------------------------------------------*/

static unsigned int nodeTableHash(Biddy_Variable v, Biddy_Edge pf, Biddy_Edge pt, unsigned int size);

static unsigned int op3Hash(Biddy_Edge a, Biddy_Edge b, Biddy_Edge c, unsigned int size);

static void exchangeEdges(Biddy_Edge *f, Biddy_Edge *g);

static void complExchangeEdges(Biddy_Edge *f, Biddy_Edge *g);

static void addNodeTable(Biddy_Manager MNG, unsigned int hash, BiddyNode *node, BiddyNode *sup1);

static BiddyNode *findNodeTable(Biddy_Manager MNG, Biddy_Variable v, Biddy_Edge pf, Biddy_Edge pt, BiddyNode **thesup);

static void addOp3Cache(Biddy_Manager MNG, BiddyOp3CacheTable cache, Biddy_Edge a, Biddy_Edge b, Biddy_Edge c, Biddy_Edge r, unsigned int index);

static Biddy_Boolean findOp3Cache(Biddy_Manager MNG, BiddyOp3CacheTable cache, Biddy_Edge a, Biddy_Edge b, Biddy_Edge c, Biddy_Edge *r, unsigned int *index);

static void ITEGarbage(Biddy_Manager MNG);

static void EAGarbage(Biddy_Manager MNG);

static void RCGarbage(Biddy_Manager MNG);

static Biddy_Variable swapWithHigher(Biddy_Manager MNG, Biddy_Variable v, Biddy_Boolean *active);

static Biddy_Variable swapWithLower(Biddy_Manager MNG, Biddy_Variable v, Biddy_Boolean *active);

static void swapVariables(Biddy_Manager MNG, Biddy_Variable low, Biddy_Variable high, Biddy_Boolean *active);

static void nullOrdering(BiddyOrderingTable table);

static void nodeNumberOrdering(Biddy_Manager MNG, Biddy_Edge f, unsigned int *i, BiddyOrderingTable ordering);

static void warshall(BiddyOrderingTable table, Biddy_Variable varnum);

static void writeBDD(Biddy_Manager MNG, Biddy_Edge f);

static void writeORDERING(Biddy_Manager MNG, BiddyOrderingTable table, Biddy_Variable varnum);

static void writeORDER(Biddy_Manager MNG);

static BiddyLocalInfo * createLocalInfo(Biddy_Manager MNG, Biddy_Edge f, BiddyLocalInfo *c);

static void deleteLocalInfo(Biddy_Manager MNG, Biddy_Edge f);

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
  unsigned int i,j;
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

  /* CREATE ANONYMOUS MANAGER - VALUES ARE NOT INITIALIZED IN THIS BLOCK OF CODE */
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
  biddyNodeTable.blocktable = NULL;
  biddyNodeTable.initsize = 0;
  biddyNodeTable.size = 0;
  biddyNodeTable.limitsize = 0;
  biddyNodeTable.blocknumber = 0;
  biddyNodeTable.initblocksize = 0;
  biddyNodeTable.blocksize = 0;
  biddyNodeTable.limitblocksize = 0;
  biddyNodeTable.generated = 0;
  biddyNodeTable.max = 0;
  biddyNodeTable.num = 0;
  biddyNodeTable.garbage = 0;
  biddyNodeTable.swap = 0;
  biddyNodeTable.sifting = 0;
  biddyNodeTable.nodetableresize = 0;
  biddyNodeTable.funite = 0;
  biddyNodeTable.funand = 0;
  biddyNodeTable.gctime = 0;
  biddyNodeTable.drtime = 0;
  biddyNodeTable.gcratio = 0.0;
  biddyNodeTable.resizeratio = 0.0;
  biddyNodeTable.newblockratio = 0.0;
  biddyNodeTable.siftingtreshold = 0.0;
  biddyNodeTable.fsiftingtreshold = 0.0;
  biddyNodeTable.convergesiftingtreshold = 0.0;
  biddyNodeTable.fconvergesiftingtreshold = 0.0;
#ifdef BIDDYEXTENDEDSTATS_YES
  biddyNodeTable.iterecursive = 0;
  biddyNodeTable.andrecursive = 0;
  biddyNodeTable.foa = 0;
  biddyNodeTable.find = 0;
  biddyNodeTable.compare = 0;
  biddyNodeTable.add = 0;
#endif
  if (!(MNG[5] = (BiddyVariableTable *)
        malloc(sizeof(BiddyVariableTable)))) {
    fprintf(stderr,"Biddy_Init (Anonymous manager): Out of memoy!\n");
    exit(1);
  }
  biddyVariableTable.table = NULL;
  biddyVariableTable.lookup = NULL;
  biddyVariableTable.size = 0;
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
  biddyIteCache.size = 0;
  if (!(biddyIteCache.search = (unsigned long long int *) malloc(sizeof(unsigned long long int)))) {
    fprintf(stderr, "Biddy_Init (Anonymous manager): Out of memoy!\n");
    exit(1);
  }
  if (!(biddyIteCache.find = (unsigned long long int *) malloc(sizeof(unsigned long long int)))) {
    fprintf(stderr, "Biddy_Init (Anonymous manager): Out of memoy!\n");
    exit(1);
  }
  if (!(biddyIteCache.insert = (unsigned long long int *) malloc(sizeof(unsigned long long int)))) {
    fprintf(stderr, "Biddy_Init (Anonymous manager): Out of memoy!\n");
    exit(1);
  }
  if (!(biddyIteCache.overwrite = (unsigned long long int *) malloc(sizeof(unsigned long long int)))) {
    fprintf(stderr, "Biddy_Init (Anonymous manager): Out of memoy!\n");
    exit(1);
  }
  *(biddyIteCache.search) = 0;
  *(biddyIteCache.find) = 0;
  *(biddyIteCache.insert) = 0;
  *(biddyIteCache.overwrite) = 0;
  if (!(MNG[8] = (BiddyOp3CacheTable *)
        malloc(sizeof(BiddyOp3CacheTable)))) {
    fprintf(stderr,"Biddy_Init (Anonymous manager): Out of memoy!\n");
    exit(1);
  }
  biddyEACache.table = NULL;
  biddyEACache.size = 0;
  if (!(biddyEACache.search = (unsigned long long int *) malloc(sizeof(unsigned long long int)))) {
    fprintf(stderr, "Biddy_Init (Anonymous manager): Out of memoy!\n");
    exit(1);
  }
  if (!(biddyEACache.find = (unsigned long long int *) malloc(sizeof(unsigned long long int)))) {
    fprintf(stderr, "Biddy_Init (Anonymous manager): Out of memoy!\n");
    exit(1);
  }
  if (!(biddyEACache.insert = (unsigned long long int *) malloc(sizeof(unsigned long long int)))) {
    fprintf(stderr, "Biddy_Init (Anonymous manager): Out of memoy!\n");
    exit(1);
  }
  if (!(biddyEACache.overwrite = (unsigned long long int *) malloc(sizeof(unsigned long long int)))) {
    fprintf(stderr, "Biddy_Init (Anonymous manager): Out of memoy!\n");
    exit(1);
  }
  *(biddyEACache.search) = 0;
  *(biddyEACache.find) = 0;
  *(biddyEACache.insert) = 0;
  *(biddyEACache.overwrite) = 0;
  if (!(MNG[9] = (BiddyOp3CacheTable *)
        malloc(sizeof(BiddyOp3CacheTable)))) {
    fprintf(stderr,"Biddy_Init (Anonymous manager): Out of memoy!\n");
    exit(1);
  }
  biddyRCCache.table = NULL;
  biddyRCCache.size = 0;
  if (!(biddyRCCache.search = (unsigned long long int *) malloc(sizeof(unsigned long long int)))) {
    fprintf(stderr, "Biddy_Init (Anonymous manager): Out of memoy!\n");
    exit(1);
  }
  if (!(biddyRCCache.find = (unsigned long long int *) malloc(sizeof(unsigned long long int)))) {
    fprintf(stderr, "Biddy_Init (Anonymous manager): Out of memoy!\n");
    exit(1);
  }
  if (!(biddyRCCache.insert = (unsigned long long int *) malloc(sizeof(unsigned long long int)))) {
    fprintf(stderr, "Biddy_Init (Anonymous manager): Out of memoy!\n");
    exit(1);
  }
  if (!(biddyRCCache.overwrite = (unsigned long long int *) malloc(sizeof(unsigned long long int)))) {
    fprintf(stderr, "Biddy_Init (Anonymous manager): Out of memoy!\n");
    exit(1);
  }
  *(biddyRCCache.search) = 0;
  *(biddyRCCache.find) = 0;
  *(biddyRCCache.insert) = 0;
  *(biddyRCCache.overwrite) = 0;
  if (!(MNG[10] = (BiddyCacheList* *)
        malloc(sizeof(BiddyCacheList*)))) {
    fprintf(stderr,"Biddy_Init (Anonymous manager): Out of memoy!\n");
    exit(1);
  }
  biddyCacheList = NULL;
  if (!(MNG[11] = (BiddyNode* *)
        malloc(sizeof(BiddyNode*)))) {
    fprintf(stderr,"Biddy_Init (Anonymous manager): Out of memoy!\n");
    exit(1);
  }
  biddyFreeNodes = NULL;
  if (!(MNG[12] = (BiddyOrderingTable *)
        malloc(sizeof(BiddyOrderingTable)))) {
    fprintf(stderr,"Biddy_Init (Anonymous manager): Out of memoy!\n");
    exit(1);
  }
  if (!(MNG[13] = (unsigned int *)
        malloc(sizeof(int)))) {
    fprintf(stderr,"Biddy_Init (Anonymous manager): Out of memoy!\n");
    exit(1);
  }
  biddyCount = 0;
  if (!(MNG[14] = (unsigned short int *)
        malloc(sizeof(int)))) {
    fprintf(stderr,"Biddy_Init (Anonymous manager): Out of memoy!\n");
    exit(1);
  }
  biddySelect = 0;
  biddyAnonymousManager = MNG;

  /* PROFILING, ONLY */
  /*
  srand(time(NULL));
  */

  /* THESE ARE DEFAULT SIZES */
  biddyVariableTable.size = BIDDYVARMAX;
  biddyNodeTable.blocksize = MEDIUM_SIZE;
  biddyNodeTable.limitblocksize = LARGE_SIZE;
  biddyNodeTable.size = SMALL_SIZE;
  biddyNodeTable.limitsize = HUGE_SIZE;
  biddyIteCache.size = MEDIUM_SIZE;
  biddyEACache.size = SMALL_SIZE;
  biddyRCCache.size = SMALL_SIZE;

  /* these values are experimentally determined */
  /* very efficient combination is (0.85,0.80,0.10) */
  /* other interesting combinations are: */
  /* (0.95,0.83,0.22), (0.65,0.68,0.77) */
  biddyNodeTable.gcratio = 0.85; /* do not start GC if the number of nodes is to low */
  biddyNodeTable.resizeratio = 0.80; /* resize Node table if there are enough nodes */
  biddyNodeTable.newblockratio = 0.10; /* create newblock of nodes if there are not enough free nodes */

  /* these values are experimentally determined */
  biddyNodeTable.siftingtreshold = 1.04;  /* stop sifting if the size of the system grows to much */
  biddyNodeTable.fsiftingtreshold = 1.04; /* stop sifting if the size of the function grows to much */
  biddyNodeTable.convergesiftingtreshold = 1.02;  /* stop one step of converging sifting if the size of the system grows to much */
  biddyNodeTable.fconvergesiftingtreshold = 1.02; /* stop one step of converging sifting if the size of the function grows to much */

  /* CREATE AND INITIALIZE NODE TABLE IN ANONYMOUS MANAGER */
  /* THE ACTUAL SIZE OF NODE TABLE IS biddyNodeTable.size+2 */
  /* BECAUSE OF USED TRICKS, HASH FUNCTION MUST NEVER RETURN ZERO! */
  /* USEFUL INDICES ARE FROM [1] TO [biddyNodeTable.size+1] */
  if (!(biddyNodeTable.table = (BiddyNode **)
  calloc((biddyNodeTable.size+2),sizeof(BiddyNode *)))) {
    fprintf(stderr,"Biddy_Init (Node table): Out of memoy!\n");
    exit(1);
  }
  /* for (i=0;i<=biddyNodeTable.size+1;i++) biddyNodeTable.table[i] = NULL; */

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
  biddyVariableTable.table[0].variable = biddyNull;
  biddyVariableTable.table[0].num = 1;
  for (i=1; i<biddyVariableTable.size; i++) {
    biddyVariableTable.table[i].num = 0;
    biddyVariableTable.table[i].name = NULL;
    biddyVariableTable.table[i].variable = biddyNull;
    biddyVariableTable.table[i].selected = FALSE;
    SET_ORDER(biddyOrderingTable,i,0);
    for (j=i+1; j<biddyVariableTable.size; j++) {
      SET_ORDER(biddyOrderingTable,i,j);
    }
  }
  biddyVariableTable.num = 1; /* ONLY VARIABLE '1' IS CREATED */
  biddyVariableTable.numnum = 1; /* VARIABLE '1' IS NUMBERED VARIABLE */

  /* LOOKUP TABLE FOR FASTER SEARCHING OF VARIABLES */
  /* CURRENTLY, THIS IS NOT USED */
  /*
  biddyVariableTable.lookup = (BiddyLookupVariable *)
                malloc(biddyVariableTable.size * sizeof(BiddyLookupVariable));
  biddyVariableTable.lookup[0].name = strdup("1");
  biddyVariableTable.lookup[0].v = 0;
  */

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
  if (!(biddyFreeNodes = (BiddyNode *)
        malloc((biddyNodeTable.blocksize) * sizeof(BiddyNode)))) {
    fprintf(stderr,"Biddy_Init (Nodes): Out of memoy!\n");
    exit(1);
  }
  if (!(biddyNodeTable.blocktable = (BiddyNode **)malloc(sizeof(BiddyNode *)))) {
    fprintf(stderr, "Biddy_Init (blocktable): Out of memoy!\n");
    exit(1);
  }
  biddyNodeTable.blocktable[0] = biddyFreeNodes;
  biddyNodeTable.table[0] = biddyFreeNodes;
  biddyFreeNodes = &biddyFreeNodes[1];
  for (i=0; i<biddyNodeTable.blocksize-2; i++) {
    biddyFreeNodes[i].list = (void *) &biddyFreeNodes[i+1];
  }
  biddyFreeNodes[biddyNodeTable.blocksize-2].list = NULL;
  biddyNodeTable.generated = biddyNodeTable.blocksize;
  biddyNodeTable.blocknumber = 1;

  /* MAKE CONSTANT NODE "1" AND EDGES TO CONSTANT NODE IN IN ANONYMOUS MANAGER */
  /* SINCE BIDDY V1.2: CONSTANT NODE "1" IS AT INDEX [0] */
  biddyNodeTable.table[0]->prev = NULL;
  biddyNodeTable.table[0]->next = NULL;
  biddyNodeTable.table[0]->list = NULL;
  biddyNodeTable.table[0]->f = biddyNull;
  biddyNodeTable.table[0]->t = biddyNull;
  biddyNodeTable.table[0]->v = 0;
  biddyNodeTable.table[0]->count = 0; /* constant node is fortified */
  biddyNodeTable.table[0]->select = 0; /* initialy it is not selected */
  MNG[3] = (void *) biddyNodeTable.table[0]; /* biddyOne */
  MNG[2] = (void *) ((uintptr_t) biddyOne | (uintptr_t) 1); /* biddyZero */

  /* INITIALIZATION OF VARIABLE'S VALUES */
  biddyVariableTable.table[0].value = biddyOne;
  for (i=1; i<biddyVariableTable.size; i++) {
#pragma warning(suppress: 6386)
    biddyVariableTable.table[i].value = biddyZero;
  }

  /* INITIALIZE FORMULA TABLE IN ANONYMOUS MANAGER */
  biddyFormulaTable.deletedName = strdup("BIDDY_DELETED_FORMULA");
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
  biddyNodeTable.garbage = 0;
  biddyNodeTable.swap = 0;
  biddyNodeTable.sifting = 0;
  biddyNodeTable.nodetableresize = 0;
  biddyNodeTable.funite = 0;
  biddyNodeTable.funand = 0;
  biddyNodeTable.gctime = 0;
  biddyNodeTable.drtime = 0;
#ifdef BIDDYEXTENDEDSTATS_YES
  biddyNodeTable.iterecursive = 0;
  biddyNodeTable.andrecursive = 0;
  biddyNodeTable.foa = 0;
  biddyNodeTable.find = 0;
  biddyNodeTable.compare = 0;
  biddyNodeTable.add = 0;
#endif

  /* INITIALIZATION OF NODE SELECTION IN ANONYMOUS MANAGER */
  biddySelect = 1;

  /* INITIALIZATION OF GARBAGE COLLECTION IN ANONYMOUS MANAGER */
  /* MINIMAL VALUE FOR biddyCount IS 2 (SEE IMPLEMENTATION OF SIFTING) */
  biddyCount = 2;

  /* INITIALIZATION OF CACHE LIST IN ANONYMOUS MANAGER */
  biddyCacheList = NULL;

  /* INITIALIZATION OF DEFAULT ITE CACHE - USED FOR ITE OPERATION */
#pragma warning(suppress: 6011)
  *(biddyIteCache.search) = *(biddyIteCache.find) = *(biddyIteCache.overwrite) = 0;
  if (!(biddyIteCache.table = (BiddyOp3Cache *)
  calloc((biddyIteCache.size+1),sizeof(BiddyOp3Cache)))) {
    fprintf(stderr,"Biddy_Init (ITE CACHE): Out of memoy!\n");
    exit(1);
  }
  /* for (i=0;i<=biddyIteCache.size;i++) biddyIteCache.table[i].result = biddyNull; */
  Biddy_Managed_AddCache(MNG,ITEGarbage);

  /* INITIALIZATION OF DEFAULT EA CACHE - USED FOR QUANTIFICATIONS */
  *(biddyEACache.search) = *(biddyEACache.find) = *(biddyEACache.overwrite) = 0;
  if (!(biddyEACache.table = (BiddyOp3Cache *)
  calloc((biddyEACache.size+1),sizeof(BiddyOp3Cache)))) {
    fprintf(stderr,"Biddy_Init (EA CACHE): Out of memoy!\n");
    exit(1);
  }
  /* for (i=0;i<=biddyEACache.size;i++) biddyEACache.table[i].result = biddyNull; */
  Biddy_Managed_AddCache(MNG,EAGarbage);

  /* INITIALIZATION OF DEFAULT RC CACHE - USED FOR RESTRICT AND COMPOSE */
#pragma warning(suppress: 6011)
  *(biddyRCCache.search) = *(biddyRCCache.find) = *(biddyRCCache.overwrite) = 0;
  if (!(biddyRCCache.table = (BiddyOp3Cache *)
  calloc((biddyRCCache.size+1),sizeof(BiddyOp3Cache)))) {
    fprintf(stderr,"Biddy_Init (RC CACHE): Out of memoy!\n");
    exit(1);
  }
  /* for (i=0;i<=biddyRCCache.size;i++) biddyRCCache.table[i].result = biddyNull; */
  Biddy_Managed_AddCache(MNG,RCGarbage);

  biddyNodeTable.initsize = biddyNodeTable.size;
  biddyNodeTable.initblocksize = biddyNodeTable.blocksize;

  /* DEBUGGING */
  /*
  BiddySystemReport(MNG);
  */

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
  unsigned int i;
  BiddyCacheList *sup1, *sup2;
  Biddy_Manager MNG;

  MNG = biddyAnonymousManager;

  /* REPORT ABOUT THE SYSTEM */
  /* CHECKING THE CORRECTNESS AND THE EFFICIENCY OF MEMORY MANAGEMENT */

#ifdef BIDDYEXTENDEDSTATS_YES
  BiddySystemReport(MNG);
  printf("Number of ITE calls: %u (internal direct and recursive calls: %llu)\n",biddyNodeTable.funite,biddyNodeTable.iterecursive);
  printf("Number of AND calls: %u (internal direct and recursive calls: %llu)\n",biddyNodeTable.funand,biddyNodeTable.andrecursive);
#ifdef MINGW
  printf("Memory in use: %I64u bytes\n",Biddy_Managed_ReadMemoryInUse(MNG));
#else
  printf("Memory in use: %llu bytes\n",Biddy_Managed_ReadMemoryInUse(MNG));
#endif
  printf("Garbage collections so far: %u (node table resizing so far: %u)\n",biddyNodeTable.garbage,biddyNodeTable.nodetableresize);
  printf("Total time for garbage collections so far: %.3fs\n",biddyNodeTable.gctime / (1.0 * CLOCKS_PER_SEC));
  printf("Used buckets in node table: %u (%.2f%%)\n",
                                   Biddy_Managed_ListUsed(MNG),
                                   (100.0*Biddy_Managed_ListUsed(MNG)/(biddyNodeTable.size+1)));
  printf("Peak number of live BDD nodes: %u\n",biddyNodeTable.max);
  printf("Number of live BDD nodes: %u\n",biddyNodeTable.num);
  printf("Number of compared nodes: %llu (%.2f per findNodeTable call)\n",
                                   biddyNodeTable.compare,
                                   biddyNodeTable.find ?
                                   (1.0*biddyNodeTable.compare/biddyNodeTable.find):0);
#endif

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
  if ((BiddyNodeTable*)(MNG[4])) {
    for (i = 0; i < biddyNodeTable.blocknumber; i++) {
      free(biddyNodeTable.blocktable[i]);
    }
    free(biddyNodeTable.blocktable);
    free(biddyNodeTable.table);
#pragma warning(suppress: 6001)
    free((BiddyNodeTable*)(MNG[4]));
  }

  /* TO DO: REMOVE VARIABLE NAMES! */
  /*
  printf("Delete variable table...\n");
  */
  if ((BiddyVariableTable*)(MNG[5])) {
    free(biddyVariableTable.table);
    free(biddyVariableTable.lookup);
#pragma warning(suppress: 6001)
    free((BiddyVariableTable*)(MNG[5]));
  }

  /* TO DO: REMOVE FORMULAE NAMES! */
  /*
  printf("Delete formula table...\n");
  */
  if ((BiddyFormulaTable*)(MNG[6])) {
    free(biddyFormulaTable.table);
    free(biddyFormulaTable.deletedName);
#pragma warning(suppress: 6001)
    free((BiddyFormulaTable*)(MNG[6]));
  }

  /*
  printf("Delete ITE cache...\n");
  */
  if ((BiddyOp3CacheTable*)(MNG[7])) {
    free(biddyIteCache.table);
    free(biddyIteCache.search);
    free(biddyIteCache.find);
    free(biddyIteCache.insert);
    free(biddyIteCache.overwrite);
#pragma warning(suppress: 6001)
    free((BiddyOp3CacheTable*)(MNG[7]));
  }

  /*
  printf("Delete EA cache...\n");
  */
  if ((BiddyOp3CacheTable*)(MNG[8])) {
    free(biddyEACache.table);
    free(biddyEACache.search);
    free(biddyEACache.find);
    free(biddyEACache.insert);
    free(biddyEACache.overwrite);
#pragma warning(suppress: 6001)
    free((BiddyOp3CacheTable*)(MNG[8]));
  }

  /*
  printf("Delete RC cache...\n");
  */
  if ((BiddyOp3CacheTable*)(MNG[9])) {
    free(biddyRCCache.table);
    free(biddyRCCache.search);
    free(biddyRCCache.find);
    free(biddyRCCache.insert);
    free(biddyRCCache.overwrite);
#pragma warning(suppress: 6001)
    free((BiddyOp3CacheTable*)(MNG[9]));
  }

  /*
  printf("Delete cache list...\n");
  */
  if ((BiddyCacheList**)(MNG[10])) {
#pragma warning(suppress: 6001)
    sup1 = biddyCacheList;
    while (sup1) {
      sup2 = sup1->next;
      free(sup1);
      sup1 = sup2;
    }
#pragma warning(suppress: 6001)
    free((BiddyCacheList**)(MNG[10]));
  }

  /*
  printf("Delete pointer biddyFreeNodes...\n");
  */
#pragma warning(suppress: 6001)
  free((BiddyNode**)(MNG[11]));

  /*
  printf("Delete Ordering table...\n");
  */
#pragma warning(suppress: 6001)
  free((BiddyOrderingTable*)(MNG[12]));

  /*
  printf("Delete counter...\n");
  */
  free((unsigned int*)(MNG[13]));

  /*
  printf("Delete selector...\n");
  */
  free((unsigned short int*)(MNG[14]));

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
    Macro BiddyT(fun) is defined for internal use.
*******************************************************************************/

#ifdef __cplusplus
extern "C" {
#endif

Biddy_Edge
Biddy_GetThen(Biddy_Edge fun)
{
  return BiddyT(fun);
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
    Macro BiddyE(fun) is defined for internal use.
*******************************************************************************/

#ifdef __cplusplus
extern "C" {
#endif

Biddy_Edge
Biddy_GetElse(Biddy_Edge fun)
{
  return BiddyE(fun);
}

#ifdef __cplusplus
}
#endif

/***************************************************************************//*!
\brief Function Biddy_GetTopVariable returns the top variable.

### Description
    External use, only.
### Side effects
### More info
    Macro BiddyV(fun) is defined for internal use.
*******************************************************************************/

#ifdef __cplusplus
extern "C" {
#endif

Biddy_Variable
Biddy_GetTopVariable(Biddy_Edge fun)
{
  return BiddyV(fun);
}

#ifdef __cplusplus
}
#endif

/***************************************************************************//*!
\brief Function Biddy_Managed_SelectNode selects the top node of the given
       function.

### Description
### Side effects
### More info
    Macro Biddy_SelectNode(f) is defined for use with anonymous manager.
*******************************************************************************/

#ifdef __cplusplus
extern "C" {
#endif

void
Biddy_Managed_SelectNode(Biddy_Manager MNG, Biddy_Edge f)
{
  if (!MNG) MNG = biddyAnonymousManager;

  ((BiddyNode *) Biddy_Regular(f))->select = biddySelect;
}

#ifdef __cplusplus
}
#endif

/***************************************************************************//*!
\brief Function Biddy_Managed_DeselectNode deselects the top node of the given
       function.

### Description
### Side effects
### More info
    Macro Biddy_DeselectNode(f) is defined for use with anonymous manager.
*******************************************************************************/

#ifdef __cplusplus
extern "C" {
#endif

void
Biddy_Managed_DeselectNode(Biddy_Manager MNG, Biddy_Edge f)
{
  if (!MNG) MNG = biddyAnonymousManager;

  ((BiddyNode *) Biddy_Regular(f))->select = 0;
}

#ifdef __cplusplus
}
#endif

/***************************************************************************//*!
\brief Function Biddy_Managed_IsSelected returns TRUE iff the top node of the
       given function is selected.

### Description
### Side effects
### More info
    Macro Biddy_IsSelected(f) is defined for use with anonymous manager.
*******************************************************************************/

#ifdef __cplusplus
extern "C" {
#endif

Biddy_Boolean
Biddy_Managed_IsSelected(Biddy_Manager MNG, Biddy_Edge f)
{
  if (!MNG) MNG = biddyAnonymousManager;

  return (((BiddyNode *) Biddy_Regular(f))->select == biddySelect);
}

#ifdef __cplusplus
}
#endif

/***************************************************************************//*!
\brief Function Biddy_Managed_SelectFunction recursively selects all nodes of a
       given function.

### Description
### Side effects
    Constant node must be selected before starting this function!
### More info
    Macro Biddy_SelectFunction(f) is defined for use with anonymous manager.
*******************************************************************************/

#ifdef __cplusplus
extern "C" {
#endif

void
Biddy_Managed_SelectFunction(Biddy_Manager MNG, Biddy_Edge f)
{
  if (!MNG) MNG = biddyAnonymousManager;

  /* VARIANT 1 - ITERATIVE */
  /*
  BiddyNode **stack;
  BiddyNode **SP;
  BiddyNode *p;
  if ((p=(BiddyNode *) Biddy_Regular(f))->select != biddySelect) {
    SP = stack = (BiddyNode **)
                  malloc(biddyNodeTable.num * sizeof(BiddyNode *));
    *(SP++) = p;
    while (SP != stack) {
      p = (*(--SP));
      p->select = biddySelect;
      if (((BiddyNode *) Biddy_Regular(p->f))->select != biddySelect) {
        *(SP++) = (BiddyNode *) Biddy_Regular(p->f);
      }
      if (((BiddyNode *) p->t)->select != biddySelect) {
        *(SP++) = (BiddyNode *) p->t;
      }
    }
    free(stack);
  }
  */

  /* VARIANT 2 - RECURSIVE */
  /**/
  if (((BiddyNode *)Biddy_Regular(f))->select != biddySelect) {
    ((BiddyNode *)Biddy_Regular(f))->select = biddySelect;
    Biddy_Managed_SelectFunction(MNG, BiddyE(f));
    Biddy_Managed_SelectFunction(MNG, BiddyT(f));
  }
  /**/

}

#ifdef __cplusplus
}
#endif

/***************************************************************************//*!
\brief Function Biddy_Managed_DeselectAll deselects all nodes.

### Description
    Deselect all nodes.
### Side effects
### More info
    Macro Biddy_DeselectAll() is defined for use with anonymous manager.
*******************************************************************************/

#ifdef __cplusplus
extern "C" {
#endif

void
Biddy_Managed_DeselectAll(Biddy_Manager MNG)
{
  if (!MNG) MNG = biddyAnonymousManager;

  if (++biddySelect == 0) {
    printf("WARNING: biddySelect is back to start");
    biddySelect--;
    /* TO DO: change select of all non-selected nodes to 0 */
    /* TO DO: change select of all selected nodes to 1 */
    biddySelect = 1;
  }
}

#ifdef __cplusplus
}
#endif

/***************************************************************************//*!
\brief Function Biddy_NOT complements Boolean function.

### Description
    DEPRECATED! Use macro Biddy_Not.
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
    DEPRECATED! Use macro Biddy_NotCond.
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

  if (!(x = (Biddy_String)malloc(15))) {
    fprintf(stderr, "Biddy_Managed_AddVariable: Out of memoy!\n");
    exit(1);
  }
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
  x = BiddyV(f);

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
  x = BiddyV(f);

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

  /* SEARCH FOR THE VARIABLE IN THE ORIGINAL TABLE */
  /**/
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
    if (biddyVariableTable.num == biddyVariableTable.size) {
      fprintf(stderr,"\nBIDDY (Biddy_Managed_FoaVariable): Size error!\n");
      fprintf(stderr,"Currently, there exist %d variables.\n",biddyVariableTable.num);
      exit(1);
    }
    biddyVariableTable.num++;
  }
  /**/

  /* SEARCH FOR THE VARIABLE IN THE LOOKUP TABLE INSTEAD IN THE ORIGINAL TABLE */
  /* TO DO: USE BINARY SEARCH! */
  /*
  {
  int cc;
  Biddy_Variable i;
  if (!strcmp(x,biddyVariableTable.lookup[0].name)) {
    v = 0;
    find = TRUE;
  } else {
    find = FALSE;
    for (v = 1; !find && (v < biddyVariableTable.num); v++) {
      cc = strcmp(x,biddyVariableTable.lookup[v].name);
      if (cc == 0) {
        find = TRUE;
        break;
      }
      if (cc < 0) break;
    }
  }
  if (!find) {
    if (biddyVariableTable.num == biddyVariableTable.size) {
      fprintf(stderr,"\nBIDDY (Biddy_Managed_FoaVariable): Size error!\n");
      fprintf(stderr,"Currently, there exist %d variables.\n",biddyVariableTable.num);
      exit(1);
    }
    for (i = biddyVariableTable.num-1; i >= v;  i--) {
       biddyVariableTable.lookup[i+1].name = biddyVariableTable.lookup[i].name;
       biddyVariableTable.lookup[i+1].v = biddyVariableTable.lookup[i].v;
    }
    biddyVariableTable.lookup[v].name = strdup(x);
    biddyVariableTable.lookup[v].v = biddyVariableTable.num;
    biddyVariableTable.num++;
  }
  v = biddyVariableTable.lookup[v].v;
  }
  */

  /* v IS NOW THE INDEX OF THE CORRECT ELEMENT! */

  if (!find) {

    /* IF THE ELEMENT IS A NEW ONE THEN CREATE NEW VARIABLE */
    /* LIST OF ACTIVE NODES FOR ANY [v] MUST NEVER BE EMPTY */
    /* FIRST ELEMENT OF LIST OF ACTIVE NODES IS biddyFirstNode */
    /* LAST ELEMENT OF LIST OF ACTIVE NODES IS biddyLastNode */
    /* YOU MUST NEVER ASSUME THAT [v]->biddyLastNode->list == NULL */
    /* THE NODE CREATED HERE IS ALWAYS FIRST IN THE LIST OF ACTIVE NODES */
    /* CONSTANT NODE biddyNodeTable.table[0] IS USED HERE AS TMP NODE, ONLY */

    biddyVariableTable.table[v].name = strdup(x);
    biddyVariableTable.table[v].num = 0;
    biddyVariableTable.table[v].lastNewNode = biddyNodeTable.table[0];
    result = Biddy_Managed_FoaNode(MNG,v,NULL,NULL,FALSE); /* FALSE is important! */
    biddyVariableTable.table[v].variable = result;
    biddyVariableTable.table[v].firstNewNode = (BiddyNode *) result;
    biddyNodeTable.table[0]->list = NULL; /* REPAIR CONSTANT NODE */

  } else {

    result = biddyVariableTable.table[v].variable;

  }

  /* ALL VARIABLES MUST BE STORED IN FORMULA TABLE */
  /* THIS IS IMPORTANT EVEN IF VARIABLE ALREADY EXISTS */
  /* VARIABLE MAY BE HIDDEN BY SOME FORMULA */
  /* AddPersistentFormula WILL FORTIFY NODE */
  Biddy_Managed_AddPersistentFormula(MNG,x,result);

  /* DEBUGGING */
  /*
  printf("FOAVARIABLE COMPLETE");
  BiddySystemReport(MNG);
  */

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
    new one. You have to use pf = pt = NULL to add variable.
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
  unsigned int i;
  BiddyNode ** tmp;
  Biddy_Boolean newblockRequired;
  Biddy_Boolean newvariableRequired;

  static BiddyNode *newFreeNodes;

  if (!MNG) MNG = biddyAnonymousManager;

#ifdef BIDDYEXTENDEDSTATS_YES
  biddyNodeTable.foa++;
#endif

  newblockRequired = FALSE;

  /* SPECIAL CASE */
  newvariableRequired = FALSE;
  if (!pf) {
    pf = biddyZero;
    pt = biddyOne;
    newvariableRequired = TRUE;
  } else {
    assert( Biddy_Managed_IsSmaller(MNG,v,BiddyV(pf)) );
    assert( Biddy_Managed_IsSmaller(MNG,v,BiddyV(pt)) );
  }

  /* TERMINAL CASE */
  if (pf == pt) return pf;

  /* NORMALIZATION - EDGE 'THEN' MAY NOT BE COMPLEMENTED ! */
  if (Biddy_GetMark(pt)) {
    Biddy_InvertMark(pf);
    Biddy_ClearMark(pt);
    complementedResult = TRUE;
  } else {
    complementedResult = FALSE;
  }

  /* TERMINAL CASE - VARIABLES ARE NOT HASHED IN THE USUAL WAY */
  if ((pf == biddyZero) && (pt == biddyOne) && !newvariableRequired) {
    if (complementedResult) {
      return Biddy_Not(biddyVariableTable.table[v].variable);
    } else {
      return biddyVariableTable.table[v].variable;
    }
  }

  /* THIS STATEMENT DISABLES GARBAGE COLLECTION IF THE NUMBER OF NODES IS TO LOW */
  garbageAllowed = garbageAllowed && (biddyNodeTable.num > biddyNodeTable.gcratio * biddyNodeTable.size);

  /* FIND OR ADD - THERE IS A HASH TABLE WITH CHAINING */
  /* BECAUSE OF USED TRICKS, HASH FUNCTION MUST NEVER RETURN ZERO! */
  /* VARIABLES ARE NOT HASHED IN THE USUAL WAY */

  if (newvariableRequired) {
    sup = sup1 = NULL;
    hash = 0;
  } else {
    hash = nodeTableHash(v,pf,pt,biddyNodeTable.size);
    sup = biddyNodeTable.table[hash];
    sup1 = findNodeTable(MNG,v,pf,pt,&sup);
  }

  if ((!sup) || (v != sup->v)) {

    /* NEW NODE MUST BE ADDED */

    /* IF ALL GENERATED NODES ARE USED THEN TRY GARBAGE COLLECTION */
    if (!biddyFreeNodes && garbageAllowed) {

      /* PROFILING */
      /*
      fprintf(stderr,"GC IN: biddyNodeTable.num = %u, biddyNodeTable.generated = %u\n",biddyNodeTable.num,biddyNodeTable.generated);
      */

      Biddy_Managed_FullGC(MNG);
      
      /* PROFILING */
      /*
      fprintf(stderr,"GC OUT: biddyNodeTable.num = %u, biddyNodeTable.generated = %u, free = %u (%.2f)\n",
              biddyNodeTable.num,biddyNodeTable.generated,biddyNodeTable.generated-biddyNodeTable.num,100.0*biddyNodeTable.num/biddyNodeTable.generated);
      */

      /* IF NOT ENOUGH FREE NODES ARE FOUND THEN CREATE NEW BLOCK OF NODES */
      newblockRequired = (
        ((biddyNodeTable.generated - biddyNodeTable.num) < (biddyNodeTable.newblockratio * biddyNodeTable.blocksize)) &&
        ((biddyNodeTable.generated - biddyNodeTable.num) < (biddyNodeTable.newblockratio *biddyNodeTable.size))
      );

      /* the table may be resized, thus the element must be rehashed */
      hash = nodeTableHash(v,pf,pt,biddyNodeTable.size);
      sup = biddyNodeTable.table[hash];
      sup1 = findNodeTable(MNG,v,pf,pt,&sup);

    }

    if (newblockRequired || !biddyFreeNodes) {

      /* the size of a memory block is increased until the limit is reached */
      if (biddyNodeTable.blocksize < biddyNodeTable.limitblocksize)
      {
        biddyNodeTable.blocksize = biddyNodeTable.blocksize + biddyNodeTable.size;
        if (biddyNodeTable.blocksize > biddyNodeTable.limitblocksize)
          biddyNodeTable.blocksize = biddyNodeTable.limitblocksize;
      }

      if (!(newFreeNodes = (BiddyNode *)
            malloc((biddyNodeTable.blocksize) * sizeof(BiddyNode))))
      {
        fprintf(stderr,"\nBIDDY (Biddy_Managed_FoaNode): Out of memory error!\n");
        fprintf(stderr,"Currently, there exist %d nodes.\n",biddyNodeTable.num);
        exit(1);
      }

      biddyNodeTable.blocknumber++;
      if (!(tmp = (BiddyNode **) realloc(biddyNodeTable.blocktable,
            biddyNodeTable.blocknumber * sizeof(BiddyNode *))))
      {
        fprintf(stderr,"\nBIDDY (Biddy_Managed_FoaNode): Out of memory error!\n");
        fprintf(stderr,"Currently, there exist %d nodes.\n",biddyNodeTable.num);
        exit(1);
      }
      biddyNodeTable.blocktable = tmp;
      biddyNodeTable.blocktable[biddyNodeTable.blocknumber-1] = newFreeNodes;
      biddyNodeTable.generated = biddyNodeTable.generated + biddyNodeTable.blocksize;

      newFreeNodes[biddyNodeTable.blocksize-1].list = biddyFreeNodes;
      biddyFreeNodes = newFreeNodes;
      for (i=0; i<biddyNodeTable.blocksize-1; i++) {
        newFreeNodes = (BiddyNode*) (newFreeNodes->list = &newFreeNodes[1]);
      }

    }

    /* ADDING NEW NODE */
    (biddyNodeTable.num)++;
    if (biddyNodeTable.num > biddyNodeTable.max) biddyNodeTable.max = biddyNodeTable.num;
    (biddyVariableTable.table[v].num)++;

    sup = biddyFreeNodes;
    biddyFreeNodes = (BiddyNode *) sup->list;

    assert( Biddy_Managed_IsSmaller(MNG,v,BiddyV(pf)) );
    assert( Biddy_Managed_IsSmaller(MNG,v,BiddyV(pt)) );

    sup->f = pf; /* BE CAREFULL !!!! */
    sup->t = pt; /* you can create node with an arbitrary (wrong!) ordering */
    sup->count = 1; /* new node is not refreshed! */
    sup->select = 0;
    sup->v = v;

    /* add new node to Node table */
    /* variables are not stored in Node table */
    if (!newvariableRequired) {
      addNodeTable(MNG,hash,sup,sup1);
    }

    /* add new node at the end of list */
    /* lastNewNode->list IS NOT FIXED! */
    /* YOU MUST NEVER ASSUME THAT lastNewNode->list = NULL */
    biddyVariableTable.table[v].lastNewNode->list = (void *) sup;
    biddyVariableTable.table[v].lastNewNode = sup;

  }

  /* SINCE BIDDY V1.1 */
  edge = sup;
  if (complementedResult) Biddy_SetMark(edge);

  /* (Biddy v1.5) To enable efficient memory management (e.g. sifting) */
  /* the function started with the returned node is not recursively refreshed! */

  /* DEBUGGING */
  /*
  printf("FOANODE COMPLETE");
  if (newvariableRequired) {
    printf("\nADDED VARIABLE\n");
  } else {
    BiddySystemReport(MNG);
  }
  */

  return edge;
}

#ifdef __cplusplus
}
#endif

Biddy_Edge BiddyManagedITE(Biddy_Manager MNG, Biddy_Edge f, Biddy_Edge g, Biddy_Edge h);
Biddy_Edge BiddyManagedAnd(const Biddy_Manager MNG, const Biddy_Edge f, const Biddy_Edge g);
Biddy_Edge BiddyManagedOr(const Biddy_Manager MNG, const Biddy_Edge f, const Biddy_Edge g);
Biddy_Edge BiddyManagedNand(const Biddy_Manager MNG, const Biddy_Edge f, const Biddy_Edge g);
Biddy_Edge BiddyManagedNor(const Biddy_Manager MNG, const Biddy_Edge f, const Biddy_Edge g);
Biddy_Edge BiddyManagedXor(const Biddy_Manager MNG, const Biddy_Edge f, const Biddy_Edge g);
Biddy_Edge BiddyManagedXnor(const Biddy_Manager MNG, const Biddy_Edge f, const Biddy_Edge g);

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
  Biddy_Edge r;

  assert( f != NULL );
  assert( g != NULL );
  assert( h != NULL );

  if (!MNG) MNG = biddyAnonymousManager;
  ZF_LOGI("Biddy_ITE");

  assert( BiddyIsOK(f) == TRUE );
  assert( BiddyIsOK(g) == TRUE );
  assert( BiddyIsOK(h) == TRUE );

  biddyNodeTable.funite++;

  r = BiddyManagedITE(MNG,f,g,h);

  return r;
}

#ifdef __cplusplus
}
#endif

Biddy_Edge
BiddyManagedITE(Biddy_Manager MNG, Biddy_Edge f, Biddy_Edge g, Biddy_Edge h)
{
  Biddy_Boolean negation;
  /* Biddy_Boolean simple; */ /* TESTING, ONLY */
  Biddy_Edge r, T, E, Fv, Gv, Hv, Fneg_v, Gneg_v, Hneg_v;
  Biddy_Variable v;
  unsigned cindex;

  static Biddy_Boolean terminal; /* CAN BE STATIC, WHAT IS BETTER? */
  static Biddy_Variable topF;    /* CAN BE STATIC, WHAT IS BETTER? */
  static Biddy_Variable topG;    /* CAN BE STATIC, WHAT IS BETTER? */
  static Biddy_Variable topH;    /* CAN BE STATIC, WHAT IS BETTER? */

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

  assert( MNG != NULL );
  assert( f != NULL );
  assert( g != NULL );
  assert( h != NULL );

#ifdef BIDDYEXTENDEDSTATS_YES
  biddyNodeTable.iterecursive++;
#endif

  /* NORMALIZATION - STEP 1 */

  if (Biddy_IsEqvPointer(f,g)) {
    if (f != g) g = biddyZero; else g = biddyOne;
  }

  if (Biddy_IsEqvPointer(f,h)) {
    if (f == h) h = biddyZero; else h = biddyOne;
  }

  negation = FALSE;

  /* LOOKING FOR TERMINAL CASE */

  terminal = FALSE;

  if (f == biddyOne) {
    terminal = TRUE;
    r = g;
    BiddyProlongOne(MNG,r,biddyCount);
  } else if (f == biddyZero) {
    terminal = TRUE;
    r = h;
    BiddyProlongOne(MNG,r,biddyCount);
  } else {
    if (Biddy_IsEqvPointer(g,h)) {
      if (g == h) {
        terminal = TRUE;
        r = g;
        BiddyProlongOne(MNG,r,biddyCount);
      } else {
        if (Biddy_IsConstant(g)) {
          terminal = TRUE;
          r = f;
          negation = Biddy_GetMark(g);
          BiddyProlongOne(MNG,r,biddyCount);
        }
      }
    }
  }

  if (!terminal && Biddy_IsConstant(g)) {
    terminal = TRUE;
    if (Biddy_GetMark(g)) {
      r = BiddyManagedAnd(MNG,Biddy_Not(f),h);
    } else {
      negation = TRUE;
      r = BiddyManagedAnd(MNG,Biddy_Not(f),Biddy_Not(h));
    }
  }

  if (!terminal && Biddy_IsConstant(h)) {
    terminal = TRUE;
    if (Biddy_GetMark(h)) {
      r = BiddyManagedAnd(MNG,f,g);
    } else {
      negation = TRUE;
      r = BiddyManagedAnd(MNG,f,Biddy_Not(g));
    }
  }

  /* IF NOT TERMINAL CASE */
  if (!terminal) {

    /* NORMALIZATION - STEP 2 */

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
    if (!findOp3Cache(MNG,biddyIteCache,f,g,h,&r,&cindex))
    {

      /* LOOKING FOR THE SMALLES TOP VARIABLE */
      topF = BiddyV(f);
      topG = BiddyV(g);
      topH = BiddyV(h);

      /* SINCE BIDDY V1.2 */
      /* CONSTANT NODE MUST HAVE MAX ORDER */
      if (Biddy_Managed_IsSmaller(MNG,topF,topG)) {
        v = Biddy_Managed_IsSmaller(MNG,topF,topH) ? topF : topH;
      } else {
        v = Biddy_Managed_IsSmaller(MNG,topH,topG) ? topH : topG;
      }

      /* DETERMINING PARAMETERS FOR RECURSIVE CALLS */

      /* HERE, f IS ALWAYS POSITIVE */
      if (topF == v) {
        Fv = BiddyT(f);
        Fneg_v = BiddyE(f);
      } else {
        Fneg_v = Fv = f;
      }

      /* HERE, g IS ALWAYS POSITIVE */
      if (topG == v) {
        Gv = BiddyT(g);
        Gneg_v = BiddyE(g);
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

      /* THIS COULD BE USED FOR SMARTER INSERTION INTO ITE CACHE TABLE */
      /* simple = FALSE; */

      if (Fv == biddyOne) {
        T = Gv;
        /* simple = TRUE; */
      } else if (Fv == biddyZero) {
        T = Hv;
        /* simple = TRUE; */
      } else  {
        T = BiddyManagedITE(MNG,Fv,Gv,Hv);
      }

      if (Fneg_v == biddyOne) {
        E = Gneg_v;
        /* simple = TRUE; */
      } else if (Fneg_v == biddyZero) {
        E = Hneg_v;
        /* simple = TRUE; */
      } else  {
        E = BiddyManagedITE(MNG,Fneg_v,Gneg_v,Hneg_v);
      }

      r = Biddy_Managed_FoaNode(MNG,v,E,T,TRUE); /* FoaNode will not refresh function! */
      BiddyProlongOne(MNG,r,biddyCount);

      /* CACHE EVERYTHING */
      /**/
      addOp3Cache(MNG,biddyIteCache,f,g,h,r,cindex);
      /**/

      /* THIS DOES NOT PRODUCE BETTER RESULTS */
      /*
      if (!simple) addOp3Cache(MNG,biddyIteCache,f,g,h,r);
      */

      /* DEBUGGING */
      /*
      printf("ite has not used cache\n");
      */

    } else {

      /* DEBUGGING */
      /*
      printf("ITE HAS USED CACHE\n");
      */

      /* IF THE RESULT IS FROM CACHE TABLE, REFRESH IT! */
      BiddyProlongOne(MNG,r,biddyCount);

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

/***************************************************************************//*!
\brief Function Biddy_Managed_And calculates Boolean function AND
       (conjunction).

### Description
### Side Effects
    Used by ITE.
### More Info
    Macro Biddy_And(f,g) is defined for use with anonymous manager.
*******************************************************************************/

#ifdef __cplusplus
extern "C" {
#endif

Biddy_Edge
Biddy_Managed_And(Biddy_Manager MNG, Biddy_Edge f, Biddy_Edge g)
{
  Biddy_Edge r;

  assert( f != NULL );
  assert( g != NULL );

  if (!MNG) MNG = biddyAnonymousManager;
  ZF_LOGI("Biddy_And");

  assert( BiddyIsOK(f) == TRUE );
  assert( BiddyIsOK(g) == TRUE );

  biddyNodeTable.funand++;

  r = BiddyManagedAnd(MNG,f,g);

  return r;
}

#ifdef __cplusplus
}
#endif

Biddy_Edge
BiddyManagedAnd(const Biddy_Manager MNG, const Biddy_Edge f, const Biddy_Edge g)
{
  Biddy_Boolean negation;
  Biddy_Edge r, T, E, Fv, Gv, Fneg_v, Gneg_v;
  Biddy_Edge FF, GG, HH;
  Biddy_Variable v;
  unsigned int cindex;

  static Biddy_Variable topF;    /* CAN BE STATIC, WHAT IS BETTER? */
  static Biddy_Variable topG;    /* CAN BE STATIC, WHAT IS BETTER? */

  assert( MNG != NULL );
  assert( f != NULL );
  assert( g != NULL );

#ifdef BIDDYEXTENDEDSTATS_YES
  biddyNodeTable.andrecursive++;
#endif

  /* NORMALIZATION - STEP 1 */

  if (f == g) {
    r = f;
    BiddyProlongOne(MNG,r,biddyCount);
    return r;
  }
  if (f == Biddy_Not(g)) {
    return biddyZero;
  }

  /* LOOKING FOR TERMINAL CASE */

  if (f == biddyOne) {
    r = g;
    BiddyProlongOne(MNG,r,biddyCount);
    return r;
  } else if (f == biddyZero) {
    return biddyZero;
  } else if (g == biddyOne) {
    r = f;
    BiddyProlongOne(MNG,r,biddyCount);
    return r;
  } else if (g == biddyZero) {
    return biddyZero;
  }

  /* THIS IS NOT A TERMINAL CASE */

  negation = FALSE;

  /* NORMALIZATION - STEP 2 AND 3 */

  if (((uintptr_t) f) > ((uintptr_t) g)) {
    if (Biddy_GetMark(f)) {
      negation = TRUE;
      FF = Biddy_Not(f);
      GG = biddyOne; /* Biddy_Not(h) */
      HH = Biddy_Not(g);
    } else {
      if (Biddy_GetMark(g)) {
        negation = TRUE;
        FF = f;
        GG = Biddy_Not(g);
        HH = biddyOne; /* Biddy_Not(h) */
      } else {
        FF = f;
        GG = g;
        HH = biddyZero; /* h */
      }
    }
  } else {
    if (Biddy_GetMark(g)) {
      negation = TRUE;
      FF = Biddy_Not(g);
      GG = biddyOne; /* Biddy_Not(h) */
      HH = Biddy_Not(f);
    } else {
      if (Biddy_GetMark(f)) {
        negation = TRUE;
        FF = g;
        GG = Biddy_Not(f);
        HH = biddyOne; /* Biddy_Not(h) */
      } else {
        FF = g;
        GG = f;
        HH = biddyZero; /* h */
      }
    }
  }

  /* IF RESULT IS NOT IN THE CACHE TABLE... */
  if (!findOp3Cache(MNG,biddyIteCache,FF,GG,HH,&r,&cindex))
  {

    /* LOOKING FOR THE SMALLES TOP VARIABLE */
    topF = BiddyV(f);
    topG = BiddyV(g);

    /* CONSTANT NODE MUST HAVE MAX ORDER */
    v = Biddy_Managed_IsSmaller(MNG,topF,topG) ? topF : topG;

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

    /* RECURSIVE CALLS */
    if (Fv == biddyOne) {
      T = Gv;
    } else if (Fv == biddyZero) {
      T = biddyZero;
    } else  {
      T = BiddyManagedAnd(MNG,Fv,Gv);
    }

    if (Fneg_v == biddyOne) {
      E = Gneg_v;
    } else if (Fneg_v == biddyZero) {
      E = biddyZero;
    } else  {
      E = BiddyManagedAnd(MNG,Fneg_v,Gneg_v);
    }

    r = Biddy_Managed_FoaNode(MNG,v,E,T,TRUE); /* FoaNode will not refresh/prolong function! */

    if (negation) {
      addOp3Cache(MNG,biddyIteCache,FF,GG,HH,Biddy_Not(r),cindex);
    } else {
      addOp3Cache(MNG,biddyIteCache,FF,GG,HH,r,cindex);
    }

    BiddyProlongOne(MNG,r,biddyCount);

  } else {

    if (negation) {
      Biddy_InvertMark(r);
    }

    /* IF THE RESULT IS FROM CACHE TABLE, PROLONG IT! */
    BiddyProlongOne(MNG,r,biddyCount);

  }

  return r;

}

/***************************************************************************//*!
\brief Function Biddy_Managed_Or calculates Boolean function OR
       (disjunction).

### Description
### Side Effects
### More Info
    Macro Biddy_Or(f,g) is defined for use with anonymous manager.
*******************************************************************************/

#ifdef __cplusplus
extern "C" {
#endif

Biddy_Edge
Biddy_Managed_Or(Biddy_Manager MNG, Biddy_Edge f, Biddy_Edge g)
{
  Biddy_Edge r;

  assert( f != NULL );
  assert( g != NULL );

  if (!MNG) MNG = biddyAnonymousManager;
  ZF_LOGI("Biddy_Or");

  assert( BiddyIsOK(f) == TRUE );
  assert( BiddyIsOK(g) == TRUE );

  r = BiddyManagedOr(MNG,f,g);

  return r;
}

#ifdef __cplusplus
}
#endif

Biddy_Edge
BiddyManagedOr(const Biddy_Manager MNG, const Biddy_Edge f, const Biddy_Edge g)
{
  assert( MNG != NULL );
  assert( f != NULL );
  assert( g != NULL );

  return Biddy_Not(BiddyManagedAnd(MNG,Biddy_Not(f),Biddy_Not(g)));
}

/***************************************************************************//*!
\brief Function Biddy_Managed_Nand calculates Boolean function NAND
       (Sheffer).

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
  Biddy_Edge r;

  assert( f != NULL );
  assert( g != NULL );

  if (!MNG) MNG = biddyAnonymousManager;
  ZF_LOGI("Biddy_Nand");

  assert( BiddyIsOK(f) == TRUE );
  assert( BiddyIsOK(g) == TRUE );

  r = BiddyManagedNand(MNG,f,g);

  return r;
}

#ifdef __cplusplus
}
#endif

Biddy_Edge
BiddyManagedNand(const Biddy_Manager MNG, const Biddy_Edge f,
                 const Biddy_Edge g)
{
  assert( MNG != NULL );
  assert( f != NULL );
  assert( g != NULL );

  return BiddyManagedITE(MNG,f,Biddy_Not(g),biddyOne);
}

/***************************************************************************//*!
\brief Function Biddy_Managed_Nor calculates Boolean function NOR
       (Peirce).

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
  Biddy_Edge r;

  assert( f != NULL );
  assert( g != NULL );

  if (!MNG) MNG = biddyAnonymousManager;
  ZF_LOGI("Biddy_Nor");

  assert( BiddyIsOK(f) == TRUE );
  assert( BiddyIsOK(g) == TRUE );

  r = BiddyManagedNor(MNG,f,g);

  return r;
}

#ifdef __cplusplus
}
#endif

Biddy_Edge
BiddyManagedNor(const Biddy_Manager MNG, const Biddy_Edge f,
                const Biddy_Edge g)
{
  assert( MNG != NULL );
  assert( f != NULL );
  assert( g != NULL );

  return BiddyManagedITE(MNG,f,biddyZero,Biddy_Not(g));
}

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
  Biddy_Edge r;

  assert( f != NULL );
  assert( g != NULL );

  if (!MNG) MNG = biddyAnonymousManager;
  ZF_LOGI("Biddy_Xor");

  assert( BiddyIsOK(f) == TRUE );
  assert( BiddyIsOK(g) == TRUE );

  r = BiddyManagedXor(MNG,f,g);

  return r;
}

#ifdef __cplusplus
}
#endif

Biddy_Edge
BiddyManagedXor(const Biddy_Manager MNG, const Biddy_Edge f,
                const Biddy_Edge g)
{
  assert( MNG != NULL );
  assert( f != NULL );
  assert( g != NULL );

  return BiddyManagedITE(MNG,f,Biddy_Not(g),g);
}

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
  Biddy_Edge r;

  assert( f != NULL );
  assert( g != NULL );

  if (!MNG) MNG = biddyAnonymousManager;
  ZF_LOGI("Biddy_Xnor");

  assert( BiddyIsOK(f) == TRUE );
  assert( BiddyIsOK(g) == TRUE );

  r = BiddyManagedXnor(MNG,f,g);

  return r;
}

#ifdef __cplusplus
}
#endif

Biddy_Edge
BiddyManagedXnor(const Biddy_Manager MNG, const Biddy_Edge f,
                 const Biddy_Edge g)
{
  assert( MNG != NULL );
  assert( f != NULL );
  assert( g != NULL );

  return BiddyManagedITE(MNG,f,g,Biddy_Not(g));
}

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
  ZF_LOGI("Biddy_Leq");

  r = BiddyManagedITE(MNG,f,g,biddyOne);
  return (r == biddyOne);
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
  Biddy_Edge r;

  if (!MNG) MNG = biddyAnonymousManager;
  ZF_LOGI("Biddy_Intersect");

  r = BiddyManagedITE(MNG,f,g,biddyZero);
  return r;
}

#ifdef __cplusplus
}
#endif

/***************************************************************************//*!
\brief Function Biddy_Managed_Restrict calculates a restriction of Boolean
       function.

### Description
    Original BDD is not changed!
    Value must be constant 0 or constant 1.
    It uses optimization: F(a=x) == NOT((NOT F)(a=x)).
### Side effects
    This is not Coudert and Madre's restrict function.
    Use Biddy_Simplify if you need that one.
    Cache table is not used, yet.
### More info
    Macro Biddy_Restrict(f,v,value) is defined for use with anonymous manager.
*******************************************************************************/

Biddy_Edge BiddyManagedRestrict(Biddy_Manager MNG, Biddy_Edge f, Biddy_Variable v, Biddy_Edge value);

#ifdef __cplusplus
extern "C" {
#endif

Biddy_Edge
Biddy_Managed_Restrict(Biddy_Manager MNG, Biddy_Edge f, Biddy_Variable v,
                       Biddy_Edge value)
{
  Biddy_Edge r;

  assert( f != NULL );
  assert( value != NULL );

  if (!MNG) MNG = biddyAnonymousManager;
  ZF_LOGI("Biddy_Restrict");

  assert( BiddyIsOK(f) == TRUE );
  assert( BiddyIsOK(value) == TRUE );

  r = BiddyManagedRestrict(MNG,f,v,value);

  return r;
}

#ifdef __cplusplus
}
#endif

Biddy_Edge
BiddyManagedRestrict(Biddy_Manager MNG, Biddy_Edge f, Biddy_Variable v,
                     Biddy_Edge value)
{
  Biddy_Edge h;
  Biddy_Edge e, t, r;
  Biddy_Variable fv;
  unsigned int cindex;

  assert( MNG != NULL );
  assert( f != NULL );
  assert( value != NULL );

  if (Biddy_IsConstant(f)) return f;

  h = biddyVariableTable.table[v].variable;

  /* IF RESULT IS NOT IN THE CACHE TABLE... */
  if (!findOp3Cache(MNG,biddyRCCache,f,value,h,&r,&cindex))
  {

    if ((fv=BiddyV(f)) == v) {
      if (value == biddyOne) {
        return Biddy_NotCond(BiddyT(f),Biddy_GetMark(f));
      } else {
        return Biddy_NotCond(BiddyE(f),Biddy_GetMark(f));
      }
    }

    if (Biddy_Managed_IsSmaller(MNG,v,fv)) return f;

    e = BiddyManagedRestrict(MNG,BiddyE(f),v,value);
    t = BiddyManagedRestrict(MNG,BiddyT(f),v,value);
    r = Biddy_NotCond(Biddy_Managed_FoaNode(MNG,fv,e,t,TRUE),Biddy_GetMark(f));
    BiddyProlongOne(MNG,r,biddyCount); /* FoaNode will not refresh function! */

    /* CACHE EVERYTHING */
    addOp3Cache(MNG,biddyRCCache,f,value,h,r,cindex);

  } else {

    /* IF THE RESULT IS FROM CACHE TABLE, REFRESH IT! */
    BiddyProlongOne(MNG,r,biddyCount);

  }

  return r;
}

/***************************************************************************//*!
\brief Function Biddy_Managed_Compose calculates a composition of two Boolean
       functions.

### Description
    Original BDDs are not changed!
    It uses optimization: F(a=G) == NOT((NOT F)(a=G)).
### Side effects
    Cache table is not used, yet.
### More info
    Macro Biddy_Compose(f,v,g) is defined for use with anonymous manager.
*******************************************************************************/

Biddy_Edge BiddyManagedCompose(Biddy_Manager MNG, Biddy_Edge f, Biddy_Variable v, Biddy_Edge g);

#ifdef __cplusplus
extern "C" {
#endif

Biddy_Edge
Biddy_Managed_Compose(Biddy_Manager MNG, Biddy_Edge f, Biddy_Variable v,
                      Biddy_Edge g)
{
  Biddy_Edge r;

  assert( f != NULL );
  assert( g != NULL );

  if (!MNG) MNG = biddyAnonymousManager;
  ZF_LOGI("Biddy_Compose");

  assert( BiddyIsOK(f) == TRUE );
  assert( BiddyIsOK(g) == TRUE );

  r = BiddyManagedCompose(MNG,f,v,g);

  return r;
}

#ifdef __cplusplus
}
#endif

Biddy_Edge
BiddyManagedCompose(Biddy_Manager MNG, Biddy_Edge f, Biddy_Variable v,
                    Biddy_Edge g)
{
  Biddy_Edge h;
  Biddy_Edge e, t, r;
  Biddy_Variable fv;
  unsigned int cindex;

  assert( MNG != NULL );
  assert( f != NULL );
  assert( g != NULL );

  if (Biddy_IsConstant(f)) return f;

  h = biddyVariableTable.table[v].variable;

  /* IF RESULT IS NOT IN THE CACHE TABLE... */
  if (!findOp3Cache(MNG,biddyRCCache,f,g,h,&r,&cindex))
  {

    if ((fv=BiddyV(f)) == v) {
      return Biddy_NotCond(BiddyManagedITE(MNG,g,BiddyT(f),BiddyE(f)),Biddy_GetMark(f));
    }

    if (Biddy_Managed_IsSmaller(MNG,v,fv)) return f;

    e = BiddyManagedCompose(MNG,BiddyE(f),v,g);
    t = BiddyManagedCompose(MNG,BiddyT(f),v,g);
    r = Biddy_NotCond(Biddy_Managed_FoaNode(MNG,fv,e,t,TRUE),Biddy_GetMark(f));
    BiddyProlongOne(MNG,r,biddyCount); /* FoaNode will not refresh function! */

    /* CACHE EVERYTHING */
    addOp3Cache(MNG,biddyRCCache,f,g,h,r,cindex);

  } else {

    /* IF THE RESULT IS FROM CACHE TABLE, FRESH IT! */
    BiddyProlongOne(MNG,r,biddyCount);

  }

  return r;
}

/***************************************************************************//*!
\brief Function Biddy_Managed_E calculates an existential quantification of
       Boolean function.

### Description
    Original BDD is not changed!
    Be careful: ExA F != NOT(ExA (NOT F)).
    Counterexample: Exb (AND (NOT a) b c).
### Side effects
### More info
    Macro Biddy_E(f,v) is defined for use with anonymous manager.
*******************************************************************************/

Biddy_Edge BiddyManagedE(Biddy_Manager MNG, Biddy_Edge f, Biddy_Variable v);

#ifdef __cplusplus
extern "C" {
#endif

Biddy_Edge
Biddy_Managed_E(Biddy_Manager MNG, Biddy_Edge f, Biddy_Variable v)
{
  Biddy_Edge r;

  assert( f != NULL );

  if (!MNG) MNG = biddyAnonymousManager;
  ZF_LOGI("Biddy_E");

  assert( BiddyIsOK(f) == TRUE );

  r = BiddyManagedE(MNG,f,v);

  return r;
}

#ifdef __cplusplus
}
#endif

Biddy_Edge
BiddyManagedE(Biddy_Manager MNG, Biddy_Edge f, Biddy_Variable v)
{
  Biddy_Edge h;
  Biddy_Edge e, t, r;
  Biddy_Variable fv;
  unsigned int cindex;

  assert( MNG != NULL );
  assert( f != NULL );

  /* LOOKING FOR TERMINAL CASE */
  if (Biddy_IsConstant(f)) return f;

  h = biddyVariableTable.table[v].variable;

  /* IF RESULT IS NOT IN THE CACHE TABLE... */
  if ((((uintptr_t) f) > ((uintptr_t) biddyOne)) ?
      !findOp3Cache(MNG,biddyEACache,biddyOne,f,h,&r,&cindex) :
      !findOp3Cache(MNG,biddyEACache,f,biddyOne,h,&r,&cindex))
  {

    if ((fv=BiddyV(f)) == v) {
      return BiddyManagedITE(MNG,
                       Biddy_NotCond(BiddyT(f),Biddy_GetMark(f)),
                       biddyOne,
                       Biddy_NotCond(BiddyE(f),Biddy_GetMark(f)));
    }

    if (Biddy_Managed_IsSmaller(MNG,v,fv)) return f;

    e = BiddyManagedE(MNG,Biddy_NotCond(BiddyE(f),Biddy_GetMark(f)),v);
    t = BiddyManagedE(MNG,Biddy_NotCond(BiddyT(f),Biddy_GetMark(f)),v);
    r = BiddyManagedITE(MNG,biddyVariableTable.table[fv].variable,t,e);

    /* CACHE EVERYTHING */
    if (((uintptr_t) f) > ((uintptr_t) biddyOne)) {
      addOp3Cache(MNG,biddyEACache,biddyOne,f,h,r,cindex);
    } else {
      addOp3Cache(MNG,biddyEACache,f,biddyOne,h,r,cindex);
    }

  } else {

    /* IF THE RESULT IS FROM CACHE TABLE, FRESH IT! */
    BiddyProlongOne(MNG,r,biddyCount);

  }

  return r;
}

/***************************************************************************//*!
\brief Function Biddy_Managed_A calculates an universal quantification of
       Boolean function.

### Description
    Original BDD is not changed!
    Be careful: AxA F != NOT(AxA (NOT F)).
    Counterexample: Axb (AND (NOT a) b c).
### Side effects
### More info
    Macro Biddy_A(f,v) is defined for use with anonymous manager.
*******************************************************************************/

Biddy_Edge BiddyManagedA(Biddy_Manager MNG, Biddy_Edge f, Biddy_Variable v);

#ifdef __cplusplus
extern "C" {
#endif

Biddy_Edge
Biddy_Managed_A(Biddy_Manager MNG, Biddy_Edge f, Biddy_Variable v)
{
  Biddy_Edge r;

  assert( f != NULL );

  if (!MNG) MNG = biddyAnonymousManager;
  ZF_LOGI("Biddy_A");

  assert( BiddyIsOK(f) == TRUE );

  r = BiddyManagedA(MNG,f,v);

  return r;
}

#ifdef __cplusplus
}
#endif

Biddy_Edge
BiddyManagedA(Biddy_Manager MNG, Biddy_Edge f, Biddy_Variable v)
{
  Biddy_Edge r;

  assert( MNG != NULL );
  assert( f != NULL );

  /* LOOKING FOR TERMINAL CASE */
  if (Biddy_IsConstant(f)) return f;

  /* PROCEED BY CALCULATING NOT (EX v NOT f) */
  r = BiddyManagedE(MNG,Biddy_Not(f),v);
  r = Biddy_Not(r);

  return r;
}

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
  ZF_LOGI("Biddy_IsVariableDependent");

  r = BiddyManagedA(MNG,f,v);
  return (r == biddyZero);
}

#ifdef __cplusplus
}
#endif

/***************************************************************************//*!
\brief Function Biddy_Managed_ExistAbstract existentially abstracts all the
       variables in cube from f.

### Description
### Side effects
    Original BDD is not changed!
### More info
    Macro Biddy_ExistAbstract(f,cube) is defined for use with anonymous manager.
*******************************************************************************/

Biddy_Edge BiddyManagedExistAbstract(Biddy_Manager MNG, Biddy_Edge f, Biddy_Edge cube);

#ifdef __cplusplus
extern "C" {
#endif

Biddy_Edge
Biddy_Managed_ExistAbstract(Biddy_Manager MNG, Biddy_Edge f, Biddy_Edge cube)
{
  Biddy_Edge r;

  assert( f != NULL );
  assert( cube != NULL );

  if (!MNG) MNG = biddyAnonymousManager;
  ZF_LOGI("Biddy_ExistAbstract");

  assert( BiddyIsOK(f) == TRUE );
  assert( BiddyIsOK(cube) == TRUE );

  r = BiddyManagedExistAbstract(MNG,f,cube);

  return r;
}

#ifdef __cplusplus
}
#endif

Biddy_Edge
BiddyManagedExistAbstract(Biddy_Manager MNG, Biddy_Edge f, Biddy_Edge cube)
{
  Biddy_Edge e, t, r;
  Biddy_Variable fv,cv;
  unsigned int cindex;

  assert( MNG != NULL );
  assert( f != NULL );
  assert( cube != NULL );

  /* LOOKING FOR TERMINAL CASE */
  if (Biddy_IsConstant(f)) return f;

  /* IF RESULT IS NOT IN THE CACHE TABLE... */
  if ((((uintptr_t) f) > ((uintptr_t) biddyOne)) ?
      !findOp3Cache(MNG,biddyEACache,biddyOne,f,cube,&r,&cindex) :
      !findOp3Cache(MNG,biddyEACache,f,biddyOne,cube,&r,&cindex))
  {

    fv = BiddyV(f);
    cv = BiddyV(cube);

    while (!Biddy_IsConstant(cube) && Biddy_Managed_IsSmaller(MNG,cv,fv)) {
      cube = BiddyT(cube);
      cv = BiddyV(cube);
    }
    if (Biddy_IsConstant(cube)) return f;

    if (cv == fv) {

      e = BiddyManagedExistAbstract(MNG,
            Biddy_NotCond(BiddyE(f),Biddy_GetMark(f)),BiddyT(cube));
      t = BiddyManagedExistAbstract(MNG,
            Biddy_NotCond(BiddyT(f),Biddy_GetMark(f)),BiddyT(cube));
      r = BiddyManagedITE(MNG,t,biddyOne,e);

    } else {

      e = BiddyManagedExistAbstract(MNG,
            Biddy_NotCond(BiddyE(f),Biddy_GetMark(f)),cube);
      t = BiddyManagedExistAbstract(MNG,
            Biddy_NotCond(BiddyT(f),Biddy_GetMark(f)),cube);
      r = BiddyManagedITE(MNG,biddyVariableTable.table[fv].variable,t,e);

    }

    /* CACHE EVERYTHING */
    if (((uintptr_t) f) > ((uintptr_t) biddyOne)) {
      addOp3Cache(MNG,biddyEACache,biddyOne,f,cube,r,cindex);
    } else {
      addOp3Cache(MNG,biddyEACache,f,biddyOne,cube,r,cindex);
    }

  } else {

    /* IF THE RESULT IS FROM CACHE TABLE, FRESH IT! */
    BiddyProlongOne(MNG,r,biddyCount);

  }

  return r;
}

/***************************************************************************//*!
\brief Function Biddy_Managed_UnivAbstract universally abstracts all the
       variables in cube from f.

### Description
### Side effects
    Original BDD is not changed!
### More info
    Macro Biddy_UnivAbstract(f,cube) is defined for use with anonymous manager.
*******************************************************************************/

Biddy_Edge BiddyManagedUnivAbstract(Biddy_Manager MNG, Biddy_Edge f, Biddy_Edge cube);

#ifdef __cplusplus
extern "C" {
#endif

Biddy_Edge
Biddy_Managed_UnivAbstract(Biddy_Manager MNG, Biddy_Edge f, Biddy_Edge cube)
{
  Biddy_Edge r;

  assert( f != NULL );
  assert( cube != NULL );

  if (!MNG) MNG = biddyAnonymousManager;
  ZF_LOGI("Biddy_UnivAbstract");

  assert( BiddyIsOK(f) == TRUE );
  assert( BiddyIsOK(cube) == TRUE );

  r = BiddyManagedUnivAbstract(MNG,f,cube);

  return r;
}

#ifdef __cplusplus
}
#endif

Biddy_Edge
BiddyManagedUnivAbstract(Biddy_Manager MNG, Biddy_Edge f, Biddy_Edge cube)
{
  Biddy_Edge r;

  assert( MNG != NULL );
  assert( f != NULL );
  assert( cube != NULL );

  /* LOOKING FOR TERMINAL CASE */
  if (Biddy_IsConstant(f)) return f;

  /* PROCEED BY CALCULATING NOT (EX cube NOT f) */
  r = BiddyManagedExistAbstract(MNG,Biddy_Not(f),cube);
  r = Biddy_Not(r);

  return r;
}

/***************************************************************************//*!
\brief Function Biddy_Managed_AndAbstract calculates the AND of two BDDs and
       simultaneously (existentially) abstracts the variables in cube.

### Description
### Side effects
### More info
    Macro Biddy_AndAbstract(f,g,cube) is defined for use with anonymous manager.
*******************************************************************************/

Biddy_Edge BiddyManagedAndAbstract(Biddy_Manager MNG, Biddy_Edge f, Biddy_Edge g, Biddy_Edge cube);

#ifdef __cplusplus
extern "C" {
#endif

Biddy_Edge
Biddy_Managed_AndAbstract(Biddy_Manager MNG, Biddy_Edge f, Biddy_Edge g,
                          Biddy_Edge cube)
{
  Biddy_Edge r;

  assert( f != NULL );
  assert( g != NULL );
  assert( cube != NULL );

  if (!MNG) MNG = biddyAnonymousManager;
  ZF_LOGI("Biddy_AndAbstract");

  assert( BiddyIsOK(f) == TRUE );
  assert( BiddyIsOK(g) == TRUE );
  assert( BiddyIsOK(cube) == TRUE );

  r = BiddyManagedAndAbstract(MNG,f,g,cube);

  return r;
}

#ifdef __cplusplus
}
#endif

Biddy_Edge
BiddyManagedAndAbstract(Biddy_Manager MNG, Biddy_Edge f, Biddy_Edge g,
                        Biddy_Edge cube)
{
  Biddy_Edge f0,f1,g0,g1;
  Biddy_Edge e, t, r;
  Biddy_Variable fv,gv,minv,cv;
  unsigned int cindex;

  assert( MNG != NULL );
  assert( f != NULL );
  assert( g != NULL );
  assert( cube != NULL );

  /* LOOKING FOR TERMINAL CASE */
  if ((f == biddyZero) || (g == biddyZero)) {
    return biddyZero;
  }

  if (f == biddyOne) {
    return BiddyManagedExistAbstract(MNG,g,cube);
  }

  if (g == biddyOne) {
    return BiddyManagedExistAbstract(MNG,f,cube);
  }

  /* NORMALIZATION */

  if (((uintptr_t) f) > ((uintptr_t) g)) {
    exchangeEdges(&f,&g);
  }

  /* IF RESULT IS NOT IN THE CACHE TABLE... */
  if (!findOp3Cache(MNG,biddyEACache,f,g,cube,&r,&cindex))
  {

    fv = BiddyV(f);
    gv = BiddyV(g);

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

    cv = BiddyV(cube);

    while (!Biddy_IsConstant(cube) && Biddy_Managed_IsSmaller(MNG,cv,minv)) {
      cube = BiddyT(cube);
      cv = BiddyV(cube);
    }

    if (Biddy_IsConstant(cube)) {
      return BiddyManagedITE(MNG,f,g,biddyZero);
    }

    if (minv == cv) {

      /* Tricky optimizations are from: */
      /* B. Yang et al. A Performance Study of BDD-Based Model Checking. 1998. */
      /* http://fmv.jku.at/papers/YangEtAl-FMCAD98.pdf */

      e = BiddyManagedAndAbstract(MNG,f0,g0,BiddyT(cube));
      if (e == biddyOne) return biddyOne;
      if ((e == f1) || (e == g1)) return e;
      if (e == Biddy_Not(f1)) f1 = biddyOne;
      if (e == Biddy_Not(g1)) g1 = biddyOne;
      t = BiddyManagedAndAbstract(MNG,f1,g1,BiddyT(cube));
      r = BiddyManagedITE(MNG,t,biddyOne,e);

    } else {

      e = BiddyManagedAndAbstract(MNG,f0,g0,cube);
      t = BiddyManagedAndAbstract(MNG,f1,g1,cube);
      r = BiddyManagedITE(MNG,biddyVariableTable.table[minv].variable,t,e);

    }

    /* CACHE EVERYTHING */
    addOp3Cache(MNG,biddyEACache,f,g,cube,r,cindex);

  } else {

    /* IF THE RESULT IS FROM CACHE TABLE, FRESH IT! */
    BiddyProlongOne(MNG,r,biddyCount);

  }

  return r;
}

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

Biddy_Edge BiddyManagedConstrain(Biddy_Manager MNG, Biddy_Edge f, Biddy_Edge c);

#ifdef __cplusplus
extern "C" {
#endif

Biddy_Edge
Biddy_Managed_Constrain(Biddy_Manager MNG, Biddy_Edge f, Biddy_Edge c)
{
  Biddy_Edge r;

  assert( f != NULL );
  assert( c != NULL );

  if (!MNG) MNG = biddyAnonymousManager;
  ZF_LOGI("Biddy_Constrain");

  assert( BiddyIsOK(f) == TRUE );
  assert( BiddyIsOK(c) == TRUE );

  r = BiddyManagedConstrain(MNG,f,c);

  return r;
}

#ifdef __cplusplus
}
#endif

Biddy_Edge
BiddyManagedConstrain(Biddy_Manager MNG, Biddy_Edge f, Biddy_Edge c)
{
  Biddy_Edge f0,f1,c0,c1;
  Biddy_Edge e, t, r;
  Biddy_Variable fv,cv,minv;

  assert( MNG != NULL );
  assert( f != NULL );
  assert( c != NULL );

  /* LOOKING FOR TERMINAL CASE */
  if (c == biddyZero) return biddyZero;
  if (c == biddyOne) return f;
  if (Biddy_IsConstant(f)) return f;

  fv = BiddyV(f);
  cv = BiddyV(c);

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

  if (c0 == biddyZero) return BiddyManagedConstrain(MNG,f1,c1);
  if (c1 == biddyZero) return BiddyManagedConstrain(MNG,f0,c0);

  e = BiddyManagedConstrain(MNG,f0,c0);
  t = BiddyManagedConstrain(MNG,f1,c1);
  r = BiddyManagedITE(MNG,biddyVariableTable.table[minv].variable,t,e);

  return r;
}

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

Biddy_Edge BiddyManagedSimplify(Biddy_Manager MNG, Biddy_Edge f, Biddy_Edge c);

#ifdef __cplusplus
extern "C" {
#endif

Biddy_Edge
Biddy_Managed_Simplify(Biddy_Manager MNG, Biddy_Edge f, Biddy_Edge c)
{
  Biddy_Edge r;

  assert( f != NULL );
  assert( c != NULL );

  if (!MNG) MNG = biddyAnonymousManager;
  ZF_LOGI("Biddy_Simplify");

  assert( BiddyIsOK(f) == TRUE );
  assert( BiddyIsOK(c) == TRUE );

  r = BiddyManagedSimplify(MNG,f,c);

  return r;
}

#ifdef __cplusplus
}
#endif

Biddy_Edge
BiddyManagedSimplify(Biddy_Manager MNG, Biddy_Edge f, Biddy_Edge c)
{
  Biddy_Edge f0,f1,c0,c1;
  Biddy_Edge e, t, r;
  Biddy_Variable fv,cv,minv;

  assert( MNG != NULL );
  assert( f != NULL );
  assert( c != NULL );

  /* LOOKING FOR TERMINAL CASE */
  if (c == biddyZero) return biddyNull;
  if (c == biddyOne) return f;
  if (Biddy_IsConstant(f)) return f;

  /* THIS IS FROM 20-CS-626-001 */
  /* http://gauss.ececs.uc.edu/Courses/c626/lectures/BDD/bdd-desc.pdf */
  if (f == c) return biddyOne;
  if (f == Biddy_Not(c)) return biddyZero;

  fv = BiddyV(f);
  cv = BiddyV(c);

  /* SPECIAL CASE: f/~a == f/a */
  if (Biddy_Managed_IsSmaller(MNG,cv,fv)) {
    return BiddyManagedSimplify(MNG,f,BiddyManagedE(MNG,c,cv));
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

  if (c0 == biddyZero) return BiddyManagedSimplify(MNG,f1,c1);
  if (c1 == biddyZero) return BiddyManagedSimplify(MNG,f0,c0);

  e = BiddyManagedSimplify(MNG,f0,c0);
  t = BiddyManagedSimplify(MNG,f1,c1);
  r = BiddyManagedITE(MNG,biddyVariableTable.table[minv].variable,t,e);

  return r;
}

/***************************************************************************//*!
\brief Function Biddy_Managed_Support calculates a product of all dependent
       variables.

### Description
### Side effects
### More info
    Macro Biddy_Support(f) is defined for use with anonymous manager.
*******************************************************************************/

Biddy_Edge BiddyManagedSupport(Biddy_Manager MNG, Biddy_Edge f);

#ifdef __cplusplus
extern "C" {
#endif

Biddy_Edge
Biddy_Managed_Support(Biddy_Manager MNG, Biddy_Edge f)
{
  Biddy_Edge r;

  if (!MNG) MNG = biddyAnonymousManager;
  ZF_LOGI("Biddy_Support");

  assert( (f == NULL) || (BiddyIsOK(f) == TRUE) );

  r = BiddyManagedSupport(MNG,f);

  return r;
}

#ifdef __cplusplus
}
#endif

Biddy_Edge
BiddyManagedSupport(Biddy_Manager MNG, Biddy_Edge f)
{
  Biddy_Variable v;
  unsigned int n;
  Biddy_Edge s;

  assert( MNG != NULL );

  if (Biddy_IsNull(f)) return biddyNull;
  if (Biddy_IsConstant(f)) return biddyZero;

  for (v=1;v<biddyVariableTable.num;v++) {
    biddyVariableTable.table[v].selected = FALSE;
  }

  n = 1; /* NOT NEEDED, BUT NODES ARE COUNTED BY BiddyNodeVarNumber */
  Biddy_Managed_SelectNode(MNG,biddyOne); /* needed for BiddyNodeVarNumber */
  BiddyNodeVarNumber(MNG,f,&n); /* VARIABLES ARE MARKED */
  Biddy_Managed_DeselectAll(MNG);

  s = biddyOne;
  for (v=1;v<biddyVariableTable.num;v++) {
    if (biddyVariableTable.table[v].selected == TRUE) {
      s = BiddyManagedITE(MNG,s,biddyVariableTable.table[v].variable,biddyZero);
      biddyVariableTable.table[v].selected = FALSE;
    }
  }

  return s;
}

/***************************************************************************//*!
\brief Function Biddy_Managed_Replace calculates BDD with one or more variables
       replaced.

### Description
    Original BDD is not changed!
    Replacing is controlled by variable's values (which are edges!).
    Use Biddy_Managed_ResetVariablesValue and Biddy_Managed_SetVariableValue
    to prepare replacing. Current and new variables should be disjoint sets.
### Side effects
    Cache table is not implemented, yet.
### More info
    Macro Biddy_Replace(f) is defined for use with anonymous manager.
*******************************************************************************/

Biddy_Edge BiddyManagedReplace(Biddy_Manager MNG, Biddy_Edge f);

#ifdef __cplusplus
extern "C" {
#endif

Biddy_Edge
Biddy_Managed_Replace(Biddy_Manager MNG, Biddy_Edge f)
{
  Biddy_Edge r;

  assert( f != NULL );

  if (!MNG) MNG = biddyAnonymousManager;
  ZF_LOGI("Biddy_Replace");

  assert( BiddyIsOK(f) == TRUE );

  r = BiddyManagedReplace(MNG,f);

  return r;
}

#ifdef __cplusplus
}
#endif

Biddy_Edge
BiddyManagedReplace(Biddy_Manager MNG, Biddy_Edge f)
{
  Biddy_Edge e,t,r;
  Biddy_Variable fv;

  assert( MNG != NULL );
  assert( f != NULL );

  if (Biddy_IsConstant(f)) return f;

  fv = BiddyV(f);
  if ((r=biddyVariableTable.table[fv].value) != biddyZero) {
    fv = BiddyV(r);

    /* DEBUGGING */
    /*
    fprintf(stdout,"REPLACE: %s(%u) -> %s(%u)\n",
            Biddy_Managed_GetTopVariableName(MNG,f),
            BiddyV(f),
            Biddy_Managed_GetVariableName(MNG,fv),
            fv
    );
    */
  }

  e = BiddyManagedReplace(MNG,Biddy_NotCond(BiddyE(f),Biddy_GetMark(f)));
  t = BiddyManagedReplace(MNG,Biddy_NotCond(BiddyT(f),Biddy_GetMark(f)));
  r = BiddyManagedITE(MNG,biddyVariableTable.table[fv].variable,t,e);

  return r;
}

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
  if (!MNG) MNG = biddyAnonymousManager;

  return
    ((((BiddyNode *) Biddy_Regular(f))->count == 0) ||
     (((BiddyNode *)Biddy_Regular(f))->count >= biddyCount));
}

#ifdef __cplusplus
}
#endif

/***************************************************************************//*!
\brief Function Biddy_Managed_GC performs garbage collection.

### Description
    Obsolete nodes are deleted.
### Side effects
    The first element of each chain in a node table should have a special value
    for its 'prev' element to allow tricky but efficient deleting. Moreover,
    'prev' and 'next' should be the first and the second element in the
    structure BiddyNode, respectively. Garbage collecion is reported by
    biddyNodeTable.garbage only if some bad nodes are purged!
### More info
    Macro Biddy_GC() is defined for use with anonymous manager.
    Macros Biddy_Managed_FullGC and Biddy_FullGC are useful variants.
*******************************************************************************/

#ifdef __cplusplus
extern "C" {
#endif

void
Biddy_Managed_GC(Biddy_Manager MNG, Biddy_Variable target, Biddy_Boolean cupdate)
{
  unsigned int i,j;
  BiddyFormula *tmp;
  BiddyNode *tmpnode1,*tmpnode2;
  BiddyNode *sup;
  BiddyCacheList *c;
  Biddy_Boolean cacheOK;
  Biddy_Boolean deletedFormulae;
  Biddy_Boolean resizeRequired;
  Biddy_Variable v;
  unsigned int hash;
  clock_t starttime;

  if (!MNG) MNG = biddyAnonymousManager;
  ZF_LOGI("Biddy_Garbage");

  /* DEBUGGING */
  /*
  {
  static int n = 0;
  fprintf(stdout,"\nGarbage (%d), count = %u, there are %u BDD nodes...\n",++n,biddyCount,biddyNodeTable.num);
  BiddySystemReport(MNG);
  }
  */

  biddyNodeTable.garbage++;

  starttime = clock();

  /* REMOVE ALL FORMULAE WHICH ARE NOT PRESERVED ANYMORE */
  /* the first two formulae ("0" and "1") are never deleted */

  i = 0;
  deletedFormulae = FALSE;
  for (j = 0; j < biddyFormulaTable.size; j++) {
    if (!biddyFormulaTable.table[j].deleted &&
        (!biddyFormulaTable.table[j].count || biddyFormulaTable.table[j].count > biddyCount)) {

      /* THIS FORMULA IS OK */
      if (i != j) {
        biddyFormulaTable.table[i] = biddyFormulaTable.table[j];
      }
      if (biddyFormulaTable.table[i].name) biddyFormulaTable.numOrdered = i+1;
      i++;

    } else {

      /* THIS FORMULA IS DELETED */
      if (!biddyFormulaTable.table[j].count || biddyFormulaTable.table[j].count > biddyCount) {
        deletedFormulae = TRUE;
      }
      if (biddyFormulaTable.table[j].name) free(biddyFormulaTable.table[j].name);

    }
  }

  if (i != biddyFormulaTable.size) {

    /* UPDATE FORMULA TABLE */

    biddyFormulaTable.size = i;
    if (!(tmp = (BiddyFormula *)
      realloc(biddyFormulaTable.table,biddyFormulaTable.size*sizeof(BiddyFormula))))
    {
      fprintf(stderr,"Biddy_Managed_GC: Out of memoy!\n");
      exit(1);
    }
    biddyFormulaTable.table = tmp;

    if (deletedFormulae) {

      /* THERE EXIST DELETED BUT NON-OBSOLETE FORMULAE */
      /* THIS REQUIRES A LOT OF CALCULATION! */

      /* FRESH ALL NODES INCLUDING PROLONGED AND FORTIFIED NODES */
      /* TO DO: VISIT ALL NODES USING memory block AND NOT list */

      for (v=1; v<biddyVariableTable.num; v++) {
        biddyVariableTable.table[v].lastNewNode->list = NULL;
        sup = biddyVariableTable.table[v].firstNewNode;
        while (sup) {
          sup->count = biddyCount;
          sup = (BiddyNode *) sup->list;
        }
      }

    }
  }

  /* RESTORE count FOR TOP NODE OF ALL EXTERNAL FORMULAE */
  /* DURING SIFTING, THIS IS NEEDED EVEN IF THERE ARE NO DELETED FORMULAE */
  for (j = 0; j < biddyFormulaTable.size; j++) {
    if (!Biddy_IsNull(biddyFormulaTable.table[j].f)) {
      BiddyProlongOne(MNG,biddyFormulaTable.table[j].f,biddyFormulaTable.table[j].count);
    }
  }

  /* REFRESH OBSOLETE NODES WHICH SHOULD NOT BE OBSOLETE */
  /* ALSO, COUNT HOW MANY OBSOLETE NODES PER VARIABLE EXIST */

  for (v=1; v<biddyVariableTable.num; v++) {
    biddyVariableTable.table[v].lastNewNode->list = NULL;
    biddyVariableTable.table[v].numobsolete = 0;
  }
  for (v=1; v<biddyVariableTable.num; v++) {
    /* during sifting we use target != 0 */
    if (target == v) {
      sup = biddyVariableTable.table[v].firstNewNode;
      assert( sup && (sup->count == 0) );
      while (sup) {
        assert(sup->v == v) ;
        if ((sup->count) && (sup->count < biddyCount)) {
          (biddyVariableTable.table[v].numobsolete)++;
        }
        sup = (BiddyNode *) sup->list;
      }
    }
    else if ((target == 0) || Biddy_Managed_IsSmaller(MNG,v,target)) {
      /* first node is always variable node and it is never obsolete */
      sup = biddyVariableTable.table[v].firstNewNode;
      assert( sup && (sup->count == 0) );
      while (sup) {
        assert(sup->v == v) ;
        if (!(sup->count) || (sup->count >= biddyCount)) {
          BiddyProlongRecursively(MNG,BiddyE(sup),sup->count,v);
          BiddyProlongRecursively(MNG,BiddyT(sup),sup->count,v);
        } else {
          (biddyVariableTable.table[v].numobsolete)++;
        }
        sup = (BiddyNode *) sup->list;
      }
    }
  }

  /* DEBUGGING */
  /*
  for (v=1; v<biddyVariableTable.num; v++) {
    printf("VARIABLE %s HAS %u OBSOLETE NODES\n",biddyVariableTable.table[v].name,biddyVariableTable.table[v].numobsolete);
  }
  */

  /* REMOVE ALL OBSOLETE NODES */
  /* RESIZE NODE TABLE IF IT SEEMS TO BE TO SMALL */
  
  i = biddyNodeTable.num;
  resizeRequired = FALSE;
  for (v=1; v<biddyVariableTable.num; v++) {
    i = i - biddyVariableTable.table[v].numobsolete;
  }
  resizeRequired = (
     (i > biddyNodeTable.resizeratio * biddyNodeTable.size) && 
     (biddyNodeTable.size < biddyNodeTable.limitsize)
  );

  if (resizeRequired) {
    /* REMOVE OLD NODE TABLE */
    free(biddyNodeTable.table);
    biddyNodeTable.nodetableresize++;

    /* DETERMINE NEW SIZE OF THE NODE TABLE */
    biddyNodeTable.size = 2 * biddyNodeTable.size + 1;

    /* CREATE NEW NODE TABLE */
    if (!(biddyNodeTable.table = (BiddyNode **)
    calloc((biddyNodeTable.size+2),sizeof(BiddyNode *)))) {
      fprintf(stderr,"Biddy_Managed_GC: Out of memoy!\n");
      exit(1);
    }
    biddyNodeTable.table[0] = (BiddyNode *) biddyOne;
  }

  /* BECAUSE OF REHASHING (NODES ARE STORED IN NODE TABLE IN OREDERED LISTS) */
  /* IT IS REQUIRED TO VISIT NODES STARTING WITH THE LAST VARIABLE! */
  cacheOK = TRUE;
  for (v=biddyVariableTable.num-1; v>0; v--) {

    /* IF RESIZE IS REQUIRED THEN ALL NODES EXCEPT THE FIRST ONE MUST BE VISITED */
    if (resizeRequired) {
      biddyVariableTable.table[v].numobsolete = biddyVariableTable.table[v].num - 1;
    }

    /* BE CAREFULLY WITH TYPE BiddyNode !!!! */
    /* FIELDS 'prev' AND 'next' HAVE TO BE THE FIRST AND THE SECOND! */
    /* THIS COMPUTATION USE SOME TRICKS! */

    if (biddyVariableTable.table[v].numobsolete) {

      /* UPDATING NODE TABLE */
      /* first node is always variable node and it is never obsolete */
      /* variable nodes are not stored in Node table and they never need rehasing */
      tmpnode1 = biddyVariableTable.table[v].firstNewNode;
      biddyVariableTable.table[v].lastNewNode->list = NULL;
      assert( tmpnode1 && (tmpnode1->count == 0) );

      /* DEBUGGING */
      /*
      printf("GC FOR %u (OBSOLETE NODES: %u)\n",v,biddyVariableTable.table[v].numobsolete);
      */

      while (biddyVariableTable.table[v].numobsolete) {
        tmpnode2 = (BiddyNode *) tmpnode1->list;
        assert( tmpnode2 != NULL) ;
        assert( tmpnode2->v == v ) ;

        if (!(tmpnode2->count) || (tmpnode2->count >= biddyCount)) {
          /* fortified, fresh or prolonged  node */
          tmpnode1 = tmpnode2;

          /* if resize is required then this node must be rehashed and stored into new Node table */
          if (resizeRequired) {
            hash = nodeTableHash(v,tmpnode2->f,tmpnode2->t,biddyNodeTable.size);
            addNodeTable(MNG,hash,tmpnode2,NULL); /* add node to hash table */
            (biddyVariableTable.table[v].numobsolete)--;
          }

        } else {

          /* DEBUGGING */
          /*
          printf("REMOVE %p FOR %s (%u)\n",tmpnode2,biddyVariableTable.table[v].name,biddyVariableTable.table[v].numobsolete);
          */

          /* obsolete node */
          cacheOK = FALSE;
          (biddyNodeTable.num)--;
          (biddyVariableTable.table[v].num)--;
          (biddyVariableTable.table[v].numobsolete)--;
          /* if resize is not required then remove from Node table */
          if (!resizeRequired) {
            tmpnode2->prev->next = tmpnode2->next;
            if (tmpnode2->next) {
              tmpnode2->next->prev = tmpnode2->prev;
            }
          }
          /* update list of active nodes */
          tmpnode1->list = (BiddyNode *) tmpnode2->list;
          /* insert obsolete node to the list of free nodes */
          tmpnode2->list = (void *) biddyFreeNodes;
          biddyFreeNodes = tmpnode2;
        }

      }
      if (!tmpnode1->list) {
        biddyVariableTable.table[v].lastNewNode = tmpnode1;
      }
    }

  }

  /* Updating cache tables - only if enabled and needed */
  if (cupdate && !cacheOK) {
    c = biddyCacheList;
    while (c) {
      c->gc(MNG);
      c = c->next;
    }
  }

  biddyNodeTable.gctime += clock() - starttime;

  /* DEBUGGING */
  /*
  printf("GC COMPLETE");
  BiddySystemReport(MNG);
  */

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
    Tag deleted is not considered and thus no fortified node and no
    prolonged node is discarded.
### More info
    Macro Biddy_Clean() is defined for use with anonymous manager.
*******************************************************************************/

#ifdef __cplusplus
extern "C" {
#endif

void
Biddy_Managed_Clean(Biddy_Manager MNG)
{
  if (!MNG) MNG = biddyAnonymousManager;

  /* DISCARD ALL NODES WHICH ARE NOT PRESERVED */
  /* implemented by simple increasing BiddyCount */
  BiddyIncCounter(MNG);

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
  unsigned int i;
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
    Biddy_DeleteFormula to remove its nodes. There are two
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
    Macros Biddy_AddFormula(x,f), Biddy_AddTmpFormula(f,count), and
    Biddy_AddPersistentFormula(x,f) are defined for use with anonymous manager.
*******************************************************************************/

#ifdef __cplusplus
extern "C" {
#endif

void
Biddy_Managed_AddFormula(Biddy_Manager MNG, Biddy_String x, Biddy_Edge f,
                         unsigned int count)
{
  unsigned int i,j;
  Biddy_Boolean OK;
  int cc;
  BiddyFormula *tmp;

  if (!MNG) MNG = biddyAnonymousManager;

  /* DEBUGGING */
  /*
  if (x) {
    printf("Biddy_Managed_AddFormula: Formula %s\n",x);
  } else {
    printf("Biddy_Managed_AddFormula: Anonymous formula with top variable %s\n",Biddy_Managed_GetTopVariableName(MNG,f));
  }
  */

  /*
  if (f && !BiddyIsOK(f)) {
    fprintf(stdout,"ERROR (Biddy_Managed_AddFormula): Bad f\n");
    free((void *)1);
    exit(1);
  }
  */

  if (count) {
    if ((count+biddyCount) < biddyCount) {
      fprintf(stdout,"ERROR (Biddy_Managed_AddFormula): count is to big \n");
      free((void *)1);
      exit(1);
    }
    count += biddyCount;
  }

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
                Biddy_Managed_GetTopVariableName(MNG,biddyFormulaTable.table[i].f))))
  {
    return;
  }

  /* element found */
  if (OK) {

    /* need to change persistent formula - new formula will be created */
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

    /* need to change preserved formula - new formula will be created */
    else if (biddyFormulaTable.table[i].count > biddyCount) {
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

    /* DEBUGGING */
    /*
    printf("Biddy_Managed_AddFormula: Formula %s OVERWRITTEN!\n",x);
    */

    biddyFormulaTable.table[i].f = f;
    biddyFormulaTable.table[i].count = count;
    biddyFormulaTable.table[i].deleted = FALSE;
    if (!Biddy_IsNull(f)) {
      BiddyProlongOne(MNG,f,count);
    }
    return;
  }

  /* element must be added */

  biddyFormulaTable.size++;
  if (!(tmp = (BiddyFormula *)
     realloc(biddyFormulaTable.table,biddyFormulaTable.size*sizeof(BiddyFormula))))
  {
    fprintf(stderr,"Biddy_Managed_AddFormula: Out of memoy!\n");
    exit(1);
  }
  biddyFormulaTable.table = tmp;

  for (j = biddyFormulaTable.size-1; j > i; j--) {
#pragma warning(suppress: 6386)
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
    BiddyProlongOne(MNG,f,count);
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
    Formula is labelled but not immediately removed.
    Nodes of the given formula are not immediately removed.
### Side effects
    Formula is not accessible by its name anymore.
    Formulae representing constants and variables will not be deleted.
### More info
    Macro Biddy_DeleteFormula(x) is defined for use with anonymous manager.
*******************************************************************************/

#ifdef __cplusplus
extern "C" {
#endif

Biddy_Boolean
Biddy_Managed_DeleteFormula(Biddy_Manager MNG, Biddy_String x)
{
  unsigned int i;
  Biddy_Boolean OK;
  int cc;

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
    Formula is labelled but not immediately removed.
    Nodes of the given formula are not immediately removed.
### Side effects
    Formula is not accessible by its name anymore.
    The first two formulae ("0" and "1") will not be deleted.
    Formulae representing variables will not be deleted.
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
               Biddy_Managed_GetTopVariableName(MNG,biddyFormulaTable.table[i].f)))
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
  Biddy_Boolean active;
  Biddy_Variable vhigh;

  if (!MNG) MNG = biddyAnonymousManager;

  /* OBSOLETE AND FRESH NODES ARE REMOVED BEFORE SWAPPING */
  BiddyIncCounter(MNG);
  Biddy_Managed_FullGC(MNG);

  vhigh = swapWithHigher(MNG,v,&active);

  /* revert fresh nodes back to prolonged nodes */
  BiddyDecCounter(MNG);

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
  Biddy_Boolean active;
  Biddy_Variable vlow;

  if (!MNG) MNG = biddyAnonymousManager;

  /* OBSOLETE AND FRESH NODES ARE REMOVED BEFORE SWAPPING */
  BiddyIncCounter(MNG);
  Biddy_Managed_FullGC(MNG);

  vlow = swapWithLower(MNG,v,&active);

  /* revert fresh nodes back to prolonged nodes */
  BiddyDecCounter(MNG);

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
    Variables are reordered globally.
    All obsolete nodes and all fresh nodes must be removed before call to
    Biddy_Managed_Sifting!
### Side effects
### More info
    Macro Biddy_Sifting(f) is defined for use with anonymous manager.
*******************************************************************************/

#ifdef __cplusplus
extern "C" {
#endif

Biddy_Boolean
Biddy_Managed_Sifting(Biddy_Manager MNG, Biddy_Edge f, Biddy_Boolean converge)
{
  unsigned int num,min,vnum,vv,totalbest;
  unsigned int best1,best2;
  Biddy_Variable v,vmax,k,minvar,maxvar,varOrder,n,minvarOrder,maxvarOrder;
  Biddy_Boolean highfirst,stop,finish,active;
  BiddyOrderingTable *fOrdering;
  unsigned int varTable[BIDDYVARMAX];
  float treshold;
  BiddyCacheList *c;
  clock_t starttime;

  assert( MNG != NULL );
  assert( (f == NULL) || (BiddyIsOK(f) == TRUE) );

  if (!MNG) MNG = biddyAnonymousManager;
  ZF_LOGI("Biddy_Sifting");

  starttime = clock();

  if (f && (((BiddyNode *) Biddy_Regular(f))->count == biddyCount)) {
    fprintf(stdout,"ERROR (Biddy_Managed_Sifting): Sifting on fresh function is wrong!\n");
    return FALSE;
  }

  if (FALSE) {
    /* sifting will not be performed */
    return FALSE;
  }

  if (!f) {

    /* MINIMIZING NODE NUMBER FOR THE WHOLE SYSTEM */

    /* determine minvar and maxvar */
    /* minvar is topmost variable and has global ordering 1 */
    /* maxvar is bottommost variable '1' and has global ordering numUsedVariables */
    /* global ordering is the number of zeros in corresponding line of orderingTable */
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

  } else {

    /* MINIMIZING NODE NUMBER FOR THE GIVEN FUNCTION */

    /* determine node number and variable ordering for the specific function */
    /* fOrdering is the same ordering as biddyOrderingTable, but it */
    /* also contains information about which variables are neighbours in f */
    /* function nodeNumberOrdering also counts the number of each variable in f */
    if (!(fOrdering = (BiddyOrderingTable *)malloc(sizeof(BiddyOrderingTable)))) {
      fprintf(stderr, "Biddy_Managed_Sifting: Out of memoy!\n");
      exit(1);
    }
    for (k=1; k<Biddy_Managed_VariableTableNum(MNG); k++) {
      biddyVariableTable.table[k].numone = 0;
    }
    nullOrdering(*fOrdering);
    num = 1;
    nodeNumberOrdering(MNG,f,&num,*fOrdering);
    Biddy_Managed_DeselectAll(MNG);

    /* determine minvar (topmost) and maxvar (bottommost) variable */
    minvar = maxvar = BiddyV(f);
    stop = FALSE;
    while (!stop) {
      stop = TRUE;
      for (k=1; k<Biddy_Managed_VariableTableNum(MNG); k++) {
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
      exit(1);
    }
    if (maxvarOrder != Biddy_Managed_VariableTableNum(MNG)) {
      fprintf(stdout,"ERROR (Biddy_Managed_Sifting): maxvarOrder %u is wrong\n",maxvarOrder);
      free((void *)1); /* BREAKPOINT */
      exit(1);
    }
  }

  /* DEBUGGING */
  /*
  printf("SIFTING START: num sifting: %u\n",Biddy_Managed_NodeTableSiftingNumber(MNG));
  */

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
      if (formula.f == f) {
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

  /* WE ARE STARTING WITH A CURRENT NUMBER OF NODES */
  if (!f) {
    if (converge) {
      treshold = biddyNodeTable.convergesiftingtreshold;
    } else {
      treshold = biddyNodeTable.siftingtreshold;
    }
    num = Biddy_Managed_NodeTableNum(MNG);
    min = num;
  } else {
    if (converge) {
      treshold = biddyNodeTable.fconvergesiftingtreshold;
    } else {
      treshold = biddyNodeTable.fsiftingtreshold;
    }
    min = num;
  }

  totalbest = min+1;
  while (min < totalbest) {
  totalbest = min;

  biddyNodeTable.sifting++;

  /* INIT varTable */
  /* varTable is used in the following way: */
  /* varTable[i] == 0 iff variable is not in f */
  /* varTable[i] == 1 iff variable is in f and has not been sifted, yet */
  /* varTable[i] == 2 iff variable is in f and has already been sifted */

  if (!f) {
    varTable[0] = 2; /* variable '1' is not swapped */
    for (k=1; k<Biddy_Managed_VariableTableNum(MNG); k++) {
      varTable[k] = 1;
    }
  } else {
    varTable[0] = 2; /* variable '1' is not swapped */
    for (k=1; k<Biddy_Managed_VariableTableNum(MNG); k++) {
      if (biddyVariableTable.table[k].numone == 0) {
        varTable[k] = 0;
      } else {
        varTable[k] = 1;
      }
    }
  }

  /* CHOOSE THE VARIABLE THAT WILL BE SIFTED FIRST */
  /* 1. it has not been sifted, yet */
  /* 2. it has max number of nodes */

  finish = (varTable[minvar] == 2); /* maybe, we are sifting constant '1' */
  if (!finish) {
    if (!f) {
      /* VARIANT 1 */
      /* MINIMIZING NODE NUMBER FOR THE WHOLE SYSTEM */
      /* choose a variable that has the max number of nodes */
      vmax = 1;
      vnum = Biddy_Managed_NodeTableNumVar(MNG,1);
      v = 2;
      while (v < Biddy_Managed_VariableTableNum(MNG)) {
        if ((vv=Biddy_Managed_NodeTableNumVar(MNG,v)) > vnum) {
          vmax = v;
          vnum = vv;
        }
        v++;
      }
      v = vmax;
    } else {
      /* VARIANT 2 */
      /* choose a variable that has the max number of nodes */
      vmax = 1;
      vnum = biddyVariableTable.table[1].numone;
      v = 2;
      while (v < Biddy_Managed_VariableTableNum(MNG)) {
        if ((vv=biddyVariableTable.table[v].numone) > vnum) {
          vmax = v;
          vnum = vv;
        }
        v++;
      }
      v = vmax;
      /* VARIANT 3 */
      /* simplyy choose the smallest (topmost) variable */
      /*
      v = minvar;
      */
    }
  }

  while (!finish) {
    varTable[v] = 2;

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
                      (k=swapWithHigher(MNG,v,&active)) :
                      (k=swapWithLower(MNG,v,&active))))
    {

      n++;

      /* DEBUGGING */
      /*
      printf("SIFTING: ");
      if (highfirst) {
        printf("%u swaped with higher %u\n",v,k);
      } else {
        printf("%u swaped with lower %u\n",v,k);
      }
      */

      /* IF v AND k HAVE BEEN SWAPPED */
      if (active) {

        if (!f) {

          /* DETERMINE THE NUMBER OF USEFUL NODES */
          Biddy_Managed_GC(MNG,Biddy_Managed_IsSmaller(MNG,k,v)?v:k,FALSE);
          num = Biddy_Managed_NodeTableNum(MNG);

        } else {

          /* update node number and variable ordering for the specific function */
          /* both is neccessary for the algorithm */
          /* garbage collection is not neccessary here! */
          /* *fOrdering is the same ordering as biddyOrderingTable, but it */
          /* also contains information about which variables are neighbours */
          for (vmax=1; vmax<Biddy_Managed_VariableTableNum(MNG); vmax++) {
            biddyVariableTable.table[vmax].numone = 0;
          }
          nullOrdering(*fOrdering);
          num = 1;
          nodeNumberOrdering(MNG,f,&num,*fOrdering);
          Biddy_Managed_DeselectAll(MNG);

        }

        /* DEBUGGING */
        /*
        printf("SIFTING: first direction, v=%s, step=%u, min=%u, num=%u\n",
               Biddy_Managed_GetVariableName(MNG,minvar),n,min,num);
        */

        if (num < min) {
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

        if (num > (treshold * min)) {
          stop = TRUE;
        }

      } else {

        /* DEBUGGING */
        /*
        printf("SIFTING: v AND k NOT SWAPPED!\n");
        */

        /* THEY ARE NOT NEIGHBOURS BUT THEY MAY BE BOTH IN f !! */

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

    } /* while (!stop ...) */

    /* DEBUGGING */
    /*
    printf("SIFTING FIRST DIRECTION: variable: %s (id=%u), n=%u\n",
           Biddy_Managed_GetVariableName(MNG,v),v,n);
    */

    /* DEBUGGING */
    /*
    printf("SIFTING: GO BACK!\n");
    */

    /* GO BACK */
    while (n) {
      if (highfirst) {
        k = swapWithLower(MNG,v,&active);
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
        k = swapWithHigher(MNG,v,&active);
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

    }

    if (!f) {
      /* due to gc optimizations used further, garbage collection is required here! */
      Biddy_Managed_GC(MNG,0,FALSE);
    } else {
      /* garbage collection is not needed here */
      /* however, it seems to be a good idea */
      Biddy_Managed_GC(MNG,0,FALSE);
    }

    /* update variable ordering for the specific function */
    if (f) {
      for (vmax=1; vmax<Biddy_Managed_VariableTableNum(MNG); vmax++) {
        biddyVariableTable.table[vmax].numone = 0;
      }
      nullOrdering(*fOrdering);
      num = 1;
      nodeNumberOrdering(MNG,f,&num,*fOrdering);
      Biddy_Managed_DeselectAll(MNG);
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
                      (k=swapWithLower(MNG,v,&active)) :
                      (k=swapWithHigher(MNG,v,&active))))
    {

      n++;

      /* DEBUGGING */
      /*
      printf("SIFTING: ");
      if (highfirst) {
        printf("%u swaped with lower %u\n",v,k);
      } else {
        printf("%u swaped with higher %u\n",v,k);
      }
      */

      /* IF v AND k HAVE BEEN SWAPPED */
      if (active) {

        if (!f) {

          /* DETERMINE THE NUMBER OF USEFUL NODES */
          Biddy_Managed_GC(MNG,Biddy_Managed_IsSmaller(MNG,k,v)?v:k,FALSE);
          num = Biddy_Managed_NodeTableNum(MNG);

        } else {

          /* update node number and variable ordering for the specific function */
          /* both is neccessary for the algorithm */
          /* garbage collection is not neccessary here! */
          /* *fOrdering is the same ordering as biddyOrderingTable, but it */
          /* also contains information about which variables are neighbours */
          for (vmax=1; vmax<Biddy_Managed_VariableTableNum(MNG); vmax++) {
            biddyVariableTable.table[vmax].numone = 0;
          }
          nullOrdering(*fOrdering);
          num = 1;
          nodeNumberOrdering(MNG,f,&num,*fOrdering);
          Biddy_Managed_DeselectAll(MNG);

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

        if (num > (treshold * min)) {
          stop = TRUE;
        }

      } else {

        /* DEBUGGING */
        /*
        printf("SIFTING: v AND k NOT SWAPPED!\n");
        */

        /* THEY ARE NOT NEIGHBOURS BUT THEY MAY BE BOTH IN f !! */

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

    } /* while (!stop ...) */

    /* DEBUGGING */
    /*
    printf("SIFTING SECOND DIRECTION: variable: %s (id=%u), best2=%u, n=%u\n",
           Biddy_Managed_GetVariableName(MNG,v),v,best2,n);
    */

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
          k = swapWithHigher(MNG,v,&active);
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
          k = swapWithLower(MNG,v,&active);
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
      }
      while (n!=best1) {
        if (highfirst) {
          k = swapWithHigher(MNG,v,&active);
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
          k = swapWithLower(MNG,v,&active);
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
      }
    } else {
      while (n!=best2) {
        if (highfirst) {
          k = swapWithHigher(MNG,v,&active);
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
          k = swapWithLower(MNG,v,&active);
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
      }
    }

    if (!f) {
      /* garbage collection is required here! */
      Biddy_Managed_GC(MNG,0,FALSE);
    } else {
      /* garbage collection is not needed here */
      /* however, it seems to be a good idea */
      Biddy_Managed_GC(MNG,0,FALSE);
    }

    /* CHOOSE NEXT VARIABLE OR FINISH */
    /* IF f IS GIVEN THEN VARIABLES NOT IN f WILL BE SKIPPED */

    if (!f) {
      num = Biddy_Managed_NodeTableNum(MNG);
    } else {
      for (vmax=1; vmax<Biddy_Managed_VariableTableNum(MNG); vmax++) {
        biddyVariableTable.table[vmax].numone = 0;
      }
      nullOrdering(*fOrdering);
      num = 1;
      nodeNumberOrdering(MNG,f,&num,*fOrdering);
      Biddy_Managed_DeselectAll(MNG);
    }

    /* DEBUGGING */
    /*
    varOrder = Biddy_Managed_VariableTableNum(MNG);
    for (k=0; k<Biddy_Managed_VariableTableNum(MNG); k++) {
      if (GET_ORDER(biddyOrderingTable,v,k)) varOrder--;
    }
    printf("SIFTING FINAL: variable: %s (id=%u), finalOrder=%u, numNodes=%u\n",
           Biddy_Managed_GetVariableName(MNG,v),v,varOrder,num);
    */

    finish = TRUE;
    if (!f) {
      /* choose a variable that has not been sifted, yet, and has the max number of nodes */
      vmax = 0;
      vnum = 0;
      v = 1;
      while (v < Biddy_Managed_VariableTableNum(MNG)) {
        if ((varTable[v] == 1) && ((vv=Biddy_Managed_NodeTableNumVar(MNG,v)) > vnum)) {
          vmax = v;
          vnum = vv;
          finish = FALSE;
        }
        v++;
      }
      v = vmax;
    } else {
      /* choose a variable that has not been sifted, yet, and has the max number of nodes */
      vmax = 0;
      vnum = 0;
      v = 1;
      while (v < Biddy_Managed_VariableTableNum(MNG)) {
        if ((varTable[v] == 1) && ((vv=biddyVariableTable.table[v].numone) > vnum)) {
          vmax = v;
          vnum = vv;
          finish = FALSE;
        }
        v++;
      }
      v = vmax;
    }


  }  /* while (!finish) */

  /* THIS LINE DISABLES FURTHER CALLS IN THE CASE OF NOT-CONVERGING SIFTING */
  /* IN THE CASE OF CONVERGING SIFTING, TRESHOLD IS INCREASED IN EVERY STEP */
  if (!converge) {
    totalbest = min;
  } else {
    if (!f) {
      treshold = treshold * treshold;
    } else {
      treshold = treshold * treshold;
    }
  }

  } /*   while (min < totalbest) */

  /* UPDATING CACHE TABLES IS REQUIRED BECAUSE OF GC OPTIMIZATIONS USED BEFORE */
  c = biddyCacheList;
  while (c) {
    c->gc(MNG);
    c = c->next;
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

  biddyNodeTable.drtime += clock() - starttime;

  return TRUE; /* sifting has been performed */

}

#ifdef __cplusplus
}
#endif

/***************************************************************************//*!
\brief Function Biddy_Managed_Purge immediately removes all nodes which were
       not preserved or which are not preserved anymore.

### Description
    All obsolete nodes are immediatelly removed. Moreover, nodes from deleted
    prolonged formulae and nodes from deleted fortified formulae are removed
    if they are not needed by other formulae.
    Call to Biddy_Purge does not count as clearing and thus all preserved
    formulae remains preserved for the same number of clearings.
### Side effects
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
  Biddy_Managed_FullGC(MNG);
  BiddyDecCounter(MNG);
}

#ifdef __cplusplus
}
#endif

/***************************************************************************//*!
\brief Function Biddy_Managed_PurgeAndReorder immediately removes non-preserved
       nodes and triggers reordering on function.

### Description
    All obsolete nodes are immediatelly removed. Moreover, nodes from deleted
    prolonged formulae and nodes from deleted fortified formulae are removed
    if they are not needed by other formulae.
    If BDD is given (f != NULL), reordering on function is used.
    Otherwise (f == NULL) global reordering is used.
    Call to Biddy_PurgeAndReorder does not count as clearing and thus all
    preserved formulae remains preserved for the same number of clearings.
### Side effects
### More info
    Macro Biddy_PurgeAndReorder(f) is defined for use with anonymous manager.
*******************************************************************************/

#ifdef __cplusplus
extern "C" {
#endif

void
Biddy_Managed_PurgeAndReorder(Biddy_Manager MNG, Biddy_Edge f,
                              Biddy_Boolean converge)
{
  if (!MNG) MNG = biddyAnonymousManager;

  Biddy_Managed_Purge(MNG);
  Biddy_Managed_Sifting(MNG,f,converge);
}

#ifdef __cplusplus
}
#endif

/***************************************************************************//*!
\brief Function Biddy_Managed_Refresh refreshes top node in a given function.

### Description
### Side effects
    This is an external variant of internal macro BiddyProlongOne
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

  BiddyProlongOne(MNG,f,biddyCount);
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
  if (++biddyCount <= 0) {
    printf("WARNING: biddyCount is back to start");
    biddyCount--;
    /* TO DO: change count of all obsolete and fresh nodes to 1 */
    /* TO DO: change count of all prolonged nodes to 2 */
    biddyCount = 2;
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
  if (--biddyCount < 1) {
    printf("ERROR (BiddyDecCounter): biddyCount is wrong");
  }
}

/***************************************************************************//*!
\brief Function BiddyProlongRecursively prolonges obsolete nodes in a function.

### Description
### Side effects
    The goal is to make nodes non-obsolete and not to gve them correct count.
    Nodes are prolonged recursively until a non-obsolete node is reached.
### More info
*******************************************************************************/

void
BiddyProlongRecursively(Biddy_Manager MNG, Biddy_Edge f, unsigned int count,
                        Biddy_Variable v)
{
  if (((BiddyNode *) Biddy_Regular(f))->count &&
      (((BiddyNode *) Biddy_Regular(f))->count < biddyCount))
  {
    BiddyProlongOne(MNG,f,count);
    if (v > BiddyV(f)) {
      (biddyVariableTable.table[BiddyV(f)].numobsolete)--;
    }
    BiddyProlongRecursively(MNG,BiddyE(f),count,v);
    BiddyProlongRecursively(MNG,BiddyT(f),count,v);
  }
}

/*******************************************************************************
\brief Function BiddyCreateLocalInfo creates local info table for the given
       function.

### Description
    Only one local info table may exists! It is not allowed to add new BDD nodes
    if local info table exists. It is not allowed to start GC if local info
    table exists. Function returns number of dependent variables.
    The last element and only the last element in the table has back = NULL.
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
  if (Biddy_Managed_IsSelected(MNG,f)) {
    fprintf(stderr,"ERROR: BiddyCreateLocalInfo got selected node\n");
    exit(1);
  }
  num = 0;  /* CONSTANT NODE IS NOT COUNTED HERE */

  for (i=0;i<biddyVariableTable.num;i++) {
    biddyVariableTable.table[i].selected = FALSE;
  }

  Biddy_Managed_SelectNode(MNG,biddyOne);
  BiddyNodeVarNumber(MNG,f,&num); /* NOW, ALL NODES ARE SELECTED */
  Biddy_Managed_DeselectNode(MNG,biddyOne); /* REQUIRED FOR createLocalInfo */

  var = 0;
  for (i=0;i<biddyVariableTable.num;i++) {
    if (biddyVariableTable.table[i].selected == TRUE) {
      var++;
      biddyVariableTable.table[i].selected = FALSE;
    }
  }

  if (!(biddyLocalInfo = (BiddyLocalInfo *)
        malloc((num+1) * sizeof(BiddyLocalInfo)))) {
    fprintf(stderr,"BiddyCreateLocalInfo: Out of memoy!\n");
    exit(1);
  }
  c = createLocalInfo(MNG,f,biddyLocalInfo);
  if (!c) {
    fprintf(stderr,"ERROR: createLocalInfo returns NULL\n");
    exit(1);
  }
  biddyLocalInfo[num].back = NULL;

  return var;
}

/*******************************************************************************
\brief Function BiddyDeleteLocalInfo deletes local info table for the given
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
  if (!Biddy_Managed_IsSelected(MNG,f)) {
    fprintf(stderr,"ERROR: BiddyDeleteLocalInfo got unselected node\n");
    exit(1);
  }
  deleteLocalInfo(MNG,f); /* this will deselect all nodes */
  free(biddyLocalInfo);
  biddyLocalInfo = NULL;
}

/*******************************************************************************
\brief Function BiddyGetSeqByNode returns seq number of a given node.

### Description
### Side effects
    Local info must be created before using this function.
    This should not be called for constant node because constant node does
    not have local info!
### More info
*******************************************************************************/

unsigned int
BiddyGetSeqByNode(Biddy_Edge root, Biddy_Edge f)
{
  unsigned int n;

  n = (unsigned int)
      (((uintptr_t)(((BiddyNode *)Biddy_Regular(f))->list)) -
       ((uintptr_t)(((BiddyNode *)Biddy_Regular(root))->list))
      ) / sizeof(BiddyLocalInfo);

  return n;
}

/*******************************************************************************
\brief Function BiddyGetNodeBySeq returns node with a given seq number.

### Description
### Side effects
    Local info must be created before using this function.
    Function fails if node with a given seq number does not exist.
### More info
*******************************************************************************/

Biddy_Edge
BiddyGetNodeBySeq(Biddy_Edge root, unsigned int n)
{
  return (Biddy_Edge)(((BiddyLocalInfo *)
    (((BiddyNode *)Biddy_Regular(root))->list))[n].back);
}

/*******************************************************************************
\brief Function BiddySetEnumerator is used in enumerateNodes.

### Description
### Side effects
    Local info must be created before using this function.
    This should not be called for constant node because constant node does
    not have local info!
### More info
*******************************************************************************/

void
BiddySetEnumerator(Biddy_Edge f, unsigned int n)
{
  ((BiddyLocalInfo *)(((BiddyNode *)Biddy_Regular(f))->list))->data.enumerator = n;
}

/*******************************************************************************
\brief Function BiddyGetEnumerator is used in enumerateNodes.

### Description
### Side effects
    Local info must be created before using this function.
    This should not be called for constant node because constant node does
    not have local info!
### More info
*******************************************************************************/

unsigned int
BiddyGetEnumerator(Biddy_Edge f)
{
  return ((BiddyLocalInfo *)(((BiddyNode *)Biddy_Regular(f))->list))->data.enumerator;
}

/*******************************************************************************
\brief Function BiddySetMintermCount is used in mintermCount().

### Description
### Side effects
    Local info must be created before using this function.
    We are using GNU Multiple Precision Arithmetic Library (GMP).
    This should not be called for constant node because constant node does
    not have local info!
### More info
*******************************************************************************/

void
BiddySetMintermCount(Biddy_Edge f, mpz_t value)
{
  mpz_init(((BiddyLocalInfo *)
    (((BiddyNode *) Biddy_Regular(f))->list))->data.mintermCount);
  mpz_set(((BiddyLocalInfo *)
    (((BiddyNode *) Biddy_Regular(f))->list))->data.mintermCount,value);
}

/*******************************************************************************
\brief Function BiddyGetMintermCount is used in mintermCount().

### Description
### Side effects
    Local info must be created before using this function.
    We are using GNU Multiple Precision Arithmetic Library (GMP).
    This should not be called for constant node because constant node does
    not have local info!
### More info
*******************************************************************************/

void
BiddyGetMintermCount(Biddy_Edge f, mpz_t result)
{
  mpz_set(result,((BiddyLocalInfo *)
    (((BiddyNode *) Biddy_Regular(f))->list))->data.mintermCount);
}

/*******************************************************************************
\brief Function BiddySetNodePlainSelected is used in nodePlainNumber().

### Description
### Side effects
    Local info must be created before using this function.
    This should not be called for constant node because constant node does
    not have local info!
### More info
*******************************************************************************/

void
BiddySetNodePlainSelected(Biddy_Edge f)
{
  if (Biddy_GetMark(f)) {
    (((BiddyLocalInfo *)
      (((BiddyNode *) Biddy_Regular(f))->list))->data.npSelected) |= (unsigned int) 2;
  } else {
    (((BiddyLocalInfo *)
      (((BiddyNode *) Biddy_Regular(f))->list))->data.npSelected) |= (unsigned int) 1;
  }
}

/*******************************************************************************
\brief Function BiddyGetNodePlainSelected is used in nodePlainNumber().

### Description
### Side effects
    Local info must be created before using this function.
    This should not be called for constant node because constant node does
    not have local info!
### More info
*******************************************************************************/

Biddy_Boolean
BiddyGetNodePlainSelected(Biddy_Edge f)
{
  Biddy_Boolean r;

  if (Biddy_GetMark(f)) {
    r = ((((BiddyLocalInfo *)
          (((BiddyNode *) Biddy_Regular(f))->list))->data.npSelected
         ) & (unsigned int) 2
        ) != 0;
  } else {
    r = ((((BiddyLocalInfo *)
          (((BiddyNode *) Biddy_Regular(f))->list))->data.npSelected
         ) & (unsigned int) 1
        ) != 0;
  }

  return r;
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

  printf("\n");

#ifdef SYSTEMREPORTVERBOSE
  printf("SYSTEM REPORT (VARIABLES)\n");
#endif

#ifdef SYSTEMREPORTDETAILS
  for (v=0; v<biddyVariableTable.num; v++) {
    printf("%s(%d),",
           Biddy_Managed_GetVariableName(MNG,v),
           biddyVariableTable.num
    );
  }
  printf("\n");
  printf("unused variables: %d\n",BIDDYVARMAX-biddyVariableTable.num);
#endif

#ifdef SYSTEMREPORTVERBOSE
  printf("nuber of used variables: %d\n",biddyVariableTable.num);
#endif

  /* BDD SYSTEM BY VARIABLE'S LISTS */

#ifdef SYSTEMREPORTVERBOSE
  printf("SYSTEM REPORT (VARIABLE LISTS)\n");
#endif

  countfortified = 1;
  countprolonged = countfresh = countobsolete = 0;
  for (v=1; v<biddyVariableTable.num; v++) {

    BiddyNode *sup;
    BiddyNode *sup1;
    Biddy_Boolean end;
    unsigned int hash;

    count1 = 0;

#ifdef SYSTEMREPORTVERBOSE
    printf("VARIABLE: variable %s, num=%u\n",
      biddyVariableTable.table[v].name,biddyVariableTable.table[v].num);
#endif

    sup = biddyVariableTable.table[v].firstNewNode;

    if (!sup) {
      fprintf(stderr,"ERROR (BiddySystemReport): Empty list for variable %s\n",biddyVariableTable.table[v].name);
      free((void *)1); /* BREAKPOINT */
    }

    if ((sup->f != biddyZero) || (sup->t != biddyOne)) {
      fprintf(stderr,"ERROR (BiddySystemReport): Bad first element in the list for variable %s\n",biddyVariableTable.table[v].name);
      free((void *)1); /* BREAKPOINT */
    }

    do {

      if (!sup) {
        fprintf(stderr,"ERROR (BiddySystemReport): Problem with lastNewNode for variable %s\n",biddyVariableTable.table[v].name);
        free((void *)1); /* BREAKPOINT */
      }

      if (sup->v != v) {
        fprintf(stderr,"ERROR (BiddySystemReport): Node with bad variable in list for variable %s\n",biddyVariableTable.table[v].name);
        fprintf(stderr,"NODE: pointer=%p, variable %s, else=%s, then=%s\n",
          (void *) sup,
          Biddy_Managed_GetTopVariableName(MNG,sup),
          Biddy_Managed_GetTopVariableName(MNG,sup->f),
          Biddy_Managed_GetTopVariableName(MNG,sup->t));
        free((void *)1); /* BREAKPOINT */
      }

      if ((!Biddy_Managed_IsSmaller(MNG,v,BiddyV(sup->f))) ||
          (!Biddy_Managed_IsSmaller(MNG,v,BiddyV(sup->t)))
         )
      {
        fprintf(stderr,"ERROR (BiddySystemReport): Node with bad ordering in list for variable %s\n",biddyVariableTable.table[v].name);
        fprintf(stderr,"NODE: pointer=%p, variable %s, else=%s, then=%s\n",
          (void *) sup,
          Biddy_Managed_GetTopVariableName(MNG,sup),
          Biddy_Managed_GetTopVariableName(MNG,sup->f),
          Biddy_Managed_GetTopVariableName(MNG,sup->t));
        free((void *)1); /* BREAKPOINT */
      }
  
      if (sup != biddyVariableTable.table[v].firstNewNode) {
        hash = nodeTableHash(v,sup->f,sup->t,biddyNodeTable.size);
        sup1 = biddyNodeTable.table[hash];
        findNodeTable(MNG,v,sup->f,sup->t,&sup1);
        if (!sup1) {
          fprintf(stderr,"ERROR (BiddySystemReport): Node not correctly inserted into Node table\n");
          printf("NODE: pointer=%p, count=%d, variable %s, else=%s, then=%s\n",
            (void *) sup,sup->count,
            Biddy_Managed_GetTopVariableName(MNG,sup),
            Biddy_Managed_GetTopVariableName(MNG,sup->f),
            Biddy_Managed_GetTopVariableName(MNG,sup->t));
          free((void *)1); /* BREAKPOINT */
        }
        sup1 = (BiddyNode *) sup1->next;
        if (sup1) {
          findNodeTable(MNG,v,sup->f,sup->t,&sup1);
          if (sup1 && (v == sup1->v)) {
            fprintf(stderr,"ERROR (BiddySystemReport): The same node multiple times in Node table\n");
            printf("NODE: pointer=%p, count=%d, variable %s, else=%s (%p), then=%s (%p)\n",
              (void *) sup,sup->count,
              Biddy_Managed_GetTopVariableName(MNG,sup),
              Biddy_Managed_GetTopVariableName(MNG,sup->f),sup->f,
              Biddy_Managed_GetTopVariableName(MNG,sup->t),sup->t);
            free((void *)1); /* BREAKPOINT */
          }
        }
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

#ifdef SYSTEMREPORTDETAILS
    printf("firstNewNode: pointer=%p, lastNewNode=%p\n",
      (void *) biddyVariableTable.table[v].firstNewNode,
      (void *) biddyVariableTable.table[v].lastNewNode
    );
#endif

    if (biddyVariableTable.table[v].num != count1) {
      fprintf(stderr,"ERROR (BiddySystemReport): wrong number of nodes for a variable\n");
      fprintf(stderr,"VARIABLE: variable %s, num=%u, count1=%u\n",
        biddyVariableTable.table[v].name,biddyVariableTable.table[v].num,count1);
      free((void *)1); /* BREAKPOINT */
    }

#ifdef SYSTEMREPORTVERBOSE
    printf("COUNT listOfNewNodes = %u\n",count1);
#endif

  }

#ifdef SYSTEMREPORTVERBOSE
  printf("numFortified: %u, numProlonged: %u, numFresh: %u, numBad: %u, total: %u\n",
                 countfortified,countprolonged,countfresh,countobsolete,
                 countfortified+countprolonged+countfresh+countobsolete
  );
#endif

  /* FINAL REPORT FOR LISTS */
  printf("BiddySystemReport (lists): numFortified: %u, numProlonged: %u, numFresh: %u, numBad: %u, total: %u\n",
         countfortified,countprolonged,countfresh,countobsolete,
         countfortified+countprolonged+countfresh+countobsolete
  );

  if (biddyNodeTable.num != countfortified+countprolonged+countfresh+countobsolete) {
    fprintf(stderr,"ERROR (BiddySystemReport): total number of nodes (%u by lists) differs from num = %u\n",
            countfortified+countprolonged+countfresh+countobsolete,
            biddyNodeTable.num);
    free((void *)1); /* BREAKPOINT */
  }

  /* BDD SYSTEM BY NODE TABLE */

#ifdef SYSTEMREPORTVERBOSE
  printf("SYSTEM REPORT (NODE TABLE)\n");
#endif

  count1 = 0; /* biddyNodeTable.table[0] is not considered */
  count2 = 0; /* biddyNodeTable.table[0] is counted as a variable */
  countfortified = biddyVariableTable.num; /* variables are not stored in Node table */
  countprolonged = countfresh = countobsolete = 0;
  for (i=1; i<=biddyNodeTable.size+1; i++) {
    BiddyNode *sup;

    sup = biddyNodeTable.table[i];
    if (sup != NULL) {

      count1++;
      while (sup) {
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

#ifdef SYSTEMREPORTDETAILS
        if (BiddyV(sup) < biddyVariableTable.num) {
          printf("NODE: i=%u, pointer=%p, count=%d, variable %s, else=%s, then=%s\n",
            i, (void *) sup,sup->count,
            Biddy_Managed_GetTopVariableName(MNG,sup),
            Biddy_Managed_GetTopVariableName(MNG,sup->f),
            Biddy_Managed_GetTopVariableName(MNG,sup->t));
        } else {
          printf("NODE: i=%u, pointer=%p, count=%d, variable %u, else=%s, then=%s\n",
            i, (void *) sup,sup->count,
            BiddyV(sup),
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
  printf("NODE TABLE HAS %u NODES (+variables = %u)\n",count2,count2+biddyVariableTable.num);
#endif

  /* FINAL REPORT FOR TABLE */
  printf("BiddySystemReport (table): numFortified: %u, numProlonged: %u, numFresh: %u, numBad: %u, total: %u\n",
         countfortified,countprolonged,countfresh,countobsolete,
         countfortified+countprolonged+countfresh+countobsolete
  );

  if (biddyNodeTable.num != countfortified+countprolonged+countfresh+countobsolete) {
    fprintf(stderr,"ERROR (BiddySystemReport): total number of nodes (%u by table) differs from num = %u\n",
            countfortified+countprolonged+countfresh+countobsolete,
            biddyNodeTable.num);
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
  BiddyOrderingTable *fOrdering;
  Biddy_Variable k;
#ifdef FUNCTIONREPORTVERBOSE
  Biddy_Boolean stop;
  Biddy_Variable minvar,maxvar,minvarOrder,maxvarOrder;
#endif

  if (Biddy_IsNull(f)) {
    printf("NULL FUNCTION");
    return;
  }

  if (!(fOrdering = (BiddyOrderingTable *)malloc(sizeof(BiddyOrderingTable)))) {
    fprintf(stderr, "BiddyFunctionReport: Out of memoy!\n");
    exit(1);
  }

  /* determine function ordering and node number */
  for (k=1; k<Biddy_Managed_VariableTableNum(MNG); k++) {
    biddyVariableTable.table[k].numone = 0;
  }
  nullOrdering(*fOrdering);
  num = 1;
  nodeNumberOrdering(MNG,f,&num,*fOrdering);
  Biddy_Managed_DeselectAll(MNG);

  printf("FUNCTION: number of nodes: %d, count of top node: %d/%d\n",num,
          ((BiddyNode *) Biddy_Regular(f))->count,biddyCount);

#ifdef FUNCTIONREPORTVERBOSE
  printf("REPORT ON FUNCTION\n");
  /* determine highest (bottommost) variable */
  minvar = maxvar = BiddyV(f);
  stop = FALSE;
  while (!stop) {
    stop = TRUE;
    for (k=1; k<biddyVariableTable.num; k++) {
      if (GET_ORDER(*fOrdering,maxvar,k)) {
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

  free(fOrdering);

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
nodeTableHash(Biddy_Variable v, Biddy_Edge pf, Biddy_Edge pt, unsigned int size)
{
  unsigned int hash;

  /* THIS IS HASH FUNCTION FOR NODE TABLE */
  /* THE SIZE OF NODE TABLE IS biddyNodeTable.size+2 */
  /* BECAUSE OF USED TRICKS, HASH FUNCTION MUST NEVER RETURN ZERO! */
  /* USEFUL INDICES ARE FROM [1] TO [biddyNodeTable.size+1] */

  /* this is very simple but quite efficient, not using v */
  /*
  {
  uintptr_t k;
  k = ((uintptr_t) pt >> 6) + ((uintptr_t) pf >> 5);
  hash = 1 + (k & size);
  return hash;
  }
  */

  /* this is very simple but quite efficient, using v seems to be good for sifting */
  /*
  {
  uintptr_t k;
  k = ((uintptr_t) pt >> 6) + ((uintptr_t) pf >> 2) + (uintptr_t) v;
  hash = 1 + (k & size);
  return hash;
  }
  */

#if UINTPTR_MAX == 0xffffffffffffffff

  /* this is experimentally determined, not using v */
  /**/
  {
  uintptr_t k;
  k = ((uintptr_t) pt * 352241492 + (uintptr_t) pf) * 231629123;
  hash = 1 + ((k >> 32) & size);
  return hash;
  }
  /**/

  /* this is experimentally determined, using v seems to be good for sifting */
  /*
  {
  uintptr_t k;
  k = ((uintptr_t) pt * 103406078 + (uintptr_t) pf + (uintptr_t) v) * 1186577883;
  hash = 1 + ((k >> 24) & size);
  return hash;
  }
  */

  /* this is using ideas from MURMUR3, see https://code.google.com/p/smhasher/ */
  /*
  {
  uintptr_t k,k1,k2;
  k = (uintptr_t) v;
  k1 = (uintptr_t) pf;
  k1 = (k1 << 31) | (k1 >> 33);
  k ^= k1;
  k2 = (uintptr_t) pt;
  k2 = (k2 << 35) | (k2 >> 29);
  k ^= k2;
  k *= 0xff51afd7ed558ccdL;
  k ^= k >> 32;
  k *= 0xc4ceb9fe1a85ec53L;
  k ^= k >> 32;
  hash = 1 + (k & size);
  return hash;
  }
  */

#else

  /* MURMUR3 (A KIND OF) 32-bit HASH FUNCTION */
  /* https://code.google.com/p/smhasher/ */
  {
  uintptr_t k,k1,k2;
  k = (uintptr_t) v;
  k1 = (uintptr_t) pf;
  k1 = (k1 << 15) | (k1 >> 17);
  k ^= k1;
  k = (k << 19) | (k >> 13);
  k2 = (uintptr_t) pt;
  k ^= k2;
  k *= 0x85ebca6b;
  k ^= k >> 16;
  k *= 0xc2b2ae35;
  k ^= k >> 16;
  hash = 1 + (k & size);
  return hash;
  }

#endif

  return 0;

}

/*******************************************************************************
\brief Function op3Hash.

### Description
### Side effects
### More info
*******************************************************************************/

static unsigned int
op3Hash(Biddy_Edge a, Biddy_Edge b, Biddy_Edge c, unsigned int size)
{

  unsigned int hash;

  /* this is very simple but quite efficient */
  /*
  {
  uintptr_t k;
  k = ((uintptr_t) a >> 4) + ((uintptr_t) b >> 2) + ((uintptr_t) c >> 4);
  hash = k & size;
  return hash;
  }
  */

#if UINTPTR_MAX == 0xffffffffffffffff

  /* this is experimentally determined */
  /* rndc1 = 12582917, rndc2 = 4256249, >> 1,1,2,8, calls: 33415082 */
  /* rndc1 = 1982250849, rndc2 = 423214982, >> 0,0,0,16, calls: 33382146 */
  /* rndc1 = 1327958971, rndc2=516963069, >> 1,2,0,19, calls: 33374477 */
  /**/
  {
  uintptr_t k;
  k = ((((uintptr_t) b >> 1) + ((uintptr_t) c >> 2)) * 1327958971 + (uintptr_t) a) * 516963069;
  hash = (k >> 19) & size;
  return hash;
  }
  /**/

  /* this is using ideas from MURMUR3, see https://code.google.com/p/smhasher/ */
  /*
  {
  uintptr_t k,k1,k2;
  k = (uintptr_t) a;
  k1 = (uintptr_t) b;
  k1 = (k1 << 31) | (k1 >> 33);
  k ^= k1;
  k2 = (uintptr_t) c;
  k2 = (k2 << 35) | (k2 >> 29);
  k ^= k2;
  k *= 0xff51afd7ed558ccdL;
  k ^= k >> 32;
  k *= 0xc4ceb9fe1a85ec53L;
  k ^= k >> 32;
  hash = k & size;
  return hash;
  }
  */

#else

  /* MURMUR3 (A KIND OF) 32-bit HASH FUNCTION */
  /* https://code.google.com/p/smhasher/ */
  {
  uintptr_t k,k1,k2;
  k = (uintptr_t) a;
  k1 = (uintptr_t) b;
  k1 = (k1 << 15) | (k1 >> 17);
  k ^= k1;
  k2 = (uintptr_t) c;
  k2 = (k2 << 19) | (k2 >> 13);
  k ^= k2;
  k *= 0x85ebca6b;
  k ^= k >> 16;
  k *= 0xc2b2ae35;
  k ^= k >> 16;
  hash = k & size;
  return hash;
  }

#endif

  return 0;
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
### Side effects
    This is low level functon. It is not checked, if the same node already
    exists.
### More info
*******************************************************************************/

static void
addNodeTable(Biddy_Manager MNG, unsigned int hash, BiddyNode *node,
             BiddyNode *sup1)
{

    /* DEBUGGING */
    /*
    fprintf(stderr,"\n[ADD NODE: variable %s, else=%s (%p), then=%s (%p)]",
      Biddy_Managed_GetTopVariableName(MNG,node),
      Biddy_Managed_GetTopVariableName(MNG,node->f),node->f,
      Biddy_Managed_GetTopVariableName(MNG,node->t),node->t);
    */

#ifdef BIDDYEXTENDEDSTATS_YES
    biddyNodeTable.add++;
#endif

    /* ******************************************************************* */
    /* NODES ARE INSERTED AFTER sup1                                       */
    /* IF sup1 = NULL THEN NEW NODE WILL BE INSERTED AT THE BEGINNING      */
    /* IF sup1 = lastNode THEN NEW NODE WILL BE INSERTED AT THE END        */
    /* ******************************************************************* */

    /* THE FIRST ELEMENT OF EACH CHAIN SHOULD HAVE A SPECIAL VALUE */
    /* FOR ITS PREV ELEMENT TO ALLOW TRICKY BUT EFFICIENT DELETING */

    node->prev = sup1 ? sup1 : (BiddyNode *) &biddyNodeTable.table[hash-1];
    node->next = node->prev->next;
    node->prev->next = node;
    if (node->next) node->next->prev = node;
}

/*******************************************************************************
\brief Function findNodeTable checks for the node in the chain referenced
       by thesup.

### Description
    Return NULL and (*thesup = NULL) iff node does not exists and
           chain is empty.
    Return NULL and (*thesup = theNode) iff node exists and
           it is first element in the chain.
    Return lastNodeInChain and (*thesup = NULL) or
           nodeBeforeNodeWithWrongVariable and (*thesup = nodeWithWrongVariable)
           iff node does not exists and chain is not empty.
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

  /* DEBUGGING */
  /*
  printf("FIND NODE TABLE (*thesup=%p, v=%u, f=%p, t=%p, next=%p): v=%s (%u), pf=%s (%p), pt=%s (%p)\n",
    (void *) *thesup,
    (*thesup) ? (*thesup)->v : 0,
    (*thesup) ? (void *) (*thesup)->f : NULL,
    (*thesup) ? (void *) (*thesup)->t : NULL,
    (*thesup) ? (void *) (*thesup)->next : NULL,
    Biddy_Managed_GetVariableName(MNG,v),v,
    Biddy_Managed_GetTopVariableName(MNG,pf),pf,
    Biddy_Managed_GetTopVariableName(MNG,pt),pt);
  */

#ifdef BIDDYEXTENDEDSTATS_YES
  biddyNodeTable.find++;
#endif

  if (!(*thesup)) return NULL;

  sup = *thesup;
  sup1 = NULL;

#ifdef BIDDYEXTENDEDSTATS_YES
  biddyNodeTable.compare++;
#endif

  /* ********************************************************************* */
  /* WE HAVE ORDERED CHAINS IN NODE TABLE                                  */
  /* ********************************************************************* */

  while (sup && (v > sup->v))
  {
    sup1 = sup;
    sup = sup->next;

#ifdef BIDDYEXTENDEDSTATS_YES
    if (sup) biddyNodeTable.compare++;
#endif

  }

  while (sup &&
          (v == sup->v) &&
          ((pf != sup->f) || (pt != sup->t))
        )
  {
    sup1 = sup;
    sup = sup->next;

#ifdef BIDDYEXTENDEDSTATS_YES
    if (sup) biddyNodeTable.compare++;
#endif

  }

  *thesup = sup;
  return sup1;
}

/*******************************************************************************
\brief Function addOp3Cache adds a result to the cache table for
       three-arguments operations.

### Description
### Side effects
### More info
*******************************************************************************/

static void
addOp3Cache(Biddy_Manager MNG, BiddyOp3CacheTable cache, Biddy_Edge a,
            Biddy_Edge b, Biddy_Edge c, Biddy_Edge r, unsigned int index)
{
  BiddyOp3Cache *p;

  p = &cache.table[index];

  if (!Biddy_IsNull(p->result)) {

    /* this is experimental - not usable */
    /*
    if (Biddy_IsSmaller(Biddy_GetTopVariable(r),Biddy_GetTopVariable(p->result))) return;
    if (BiddyIsOK(r)) return;
    */
  
    /* THE CELL IS NOT EMPTY, THUS THIS IS OVERWRITING */
    (*cache.overwrite)++;
  }

  (*cache.insert)++;
  p->f = a;
  p->g = b;
  p->h = c;
  p->result = r;
}

/*******************************************************************************
\brief Function findOp3Cache looks for the result in the cache table for
       three-arguments operations.

### Description
### Side effects
### More info
*******************************************************************************/

static Biddy_Boolean
findOp3Cache(Biddy_Manager MNG, BiddyOp3CacheTable cache, Biddy_Edge a,
             Biddy_Edge b, Biddy_Edge c, Biddy_Edge *r, unsigned int *index)
{
  int q;
  BiddyOp3Cache *p;

  q = FALSE;

  *index = op3Hash(a,b,c,cache.size);
  p = &cache.table[*index];

  (*cache.search)++;

  if ((p->f == a) &&
      (p->g == b) &&
      (p->h == c) &&
      !Biddy_IsNull(p->result)
     )
  {
    (*cache.find)++;
    *r = p->result;
    q = TRUE;
  }

  return q;
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
  unsigned int j;
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
\brief Function EAGarbage performs Garbage Collection for the EA cache.

### Description
### Side effects
### More info
*******************************************************************************/

static void
EAGarbage(Biddy_Manager MNG)
{
  unsigned int j;
  BiddyOp3Cache *c;

  for (j=0; j<=biddyEACache.size; j++) {
    if (!Biddy_IsNull(biddyEACache.table[j].result)) {
      c = &biddyEACache.table[j];
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
\brief Function RCGarbage performs Garbage Collection for the RC cache.

### Description
### Side effects
### More info
*******************************************************************************/

static void
RCGarbage(Biddy_Manager MNG)
{
  unsigned int j;
  BiddyOp3Cache *c;

  for (j=0; j<=biddyRCCache.size; j++) {
    if (!Biddy_IsNull(biddyRCCache.table[j].result)) {
      c = &biddyRCCache.table[j];
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
\brief Function swapWithHigher determines higher variable and calls swapping.

### Description
### Side effects
### More info
*******************************************************************************/

static Biddy_Variable
swapWithHigher(Biddy_Manager MNG, Biddy_Variable v, Biddy_Boolean *active)
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
    swapVariables(MNG,v,vhigh,active);
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
swapWithLower(Biddy_Manager MNG, Biddy_Variable v, Biddy_Boolean *active)
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
    swapVariables(MNG,vlow,v,active);
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
    variables are swapped. Argument active is set to TRUE if some variables
    have been swapped.
### Side effects
    It is not checked that low is next to high.
    The result is wrong if low is not next to high!
### More info
*******************************************************************************/

static void
swapVariables(Biddy_Manager MNG, Biddy_Variable low, Biddy_Variable high,
              Biddy_Boolean *active)
{
  BiddyNode *sup,*tmp,*tmp1;
  BiddyNode *newBegin,*newEnd;
  Biddy_Edge f00,f01,f10,f11,u0,u1;
  unsigned int hash;
  unsigned int n;

  if (!low || !high) return;
  biddyNodeTable.swap++;

  /* DEBUGGING */
  /*
  printf("SWAPPING START: low =  %s, high = %s",
         Biddy_Managed_GetVariableName(MNG,low),
         Biddy_Managed_GetVariableName(MNG,high));
  BiddySystemReport(MNG);
  */

  /* update active ordering */
  CLEAR_ORDER(biddyOrderingTable,low,high);
  SET_ORDER(biddyOrderingTable,high,low);

  sup = newBegin = biddyVariableTable.table[low].firstNewNode;
  newEnd = NULL;

  /* PREPARE NEW LIST FOR 'LOW' */
  /* CONSTANT NODE "1" IS USED HERE BUT ONLY AS TMP */
  biddyVariableTable.table[low].lastNewNode->list = NULL;
  biddyVariableTable.table[low].firstNewNode =
    biddyVariableTable.table[low].lastNewNode = biddyNodeTable.table[0];

  *active = FALSE; /* no swapping has been made, yet */
  while (sup) {

    assert( BiddyV(sup) == low );

    /* ALL NODES WITH VARIABLE low AND SUCCESSOR high ARE SWAPPED */
    if ((BiddyV(sup->f) == high) || (BiddyV(sup->t) == high)) {

      assert( (BiddyV(sup->f)==high)?TRUE:Biddy_Managed_IsSmaller(MNG,low,BiddyV(sup->f)) );
      assert( (BiddyV(sup->t)==high)?TRUE:Biddy_Managed_IsSmaller(MNG,low,BiddyV(sup->t)) );

      /* ONE SWAP */
      /* this will add zero, one or two new nodes with variable 'low' */
      /* if FoaNode adds new node it will be added to new list for 'low' */
      /* if FoaNode returns existing node with label 'low', */
      /* then it will be relisted to new list for 'low' when it will be checked */
      /* (all existing variables are checked!) */

      if (BiddyV(sup->f) == high) {
        f00 = BiddyE(sup->f);
        f01 = BiddyT(sup->f);
        if (Biddy_GetMark(sup->f)) {
          Biddy_InvertMark(f00);
          Biddy_InvertMark(f01);
        }

        assert( Biddy_Managed_IsSmaller(MNG,low,BiddyV(f00)) );
        assert( Biddy_Managed_IsSmaller(MNG,low,BiddyV(f01)) );

        /* sup->f is not needed in sup, anymore */
        /* we will mark it as obsolete and we are expecting, that */
        /* formulae are repaired before garbage collection */
        ((BiddyNode *) Biddy_Regular(sup->f))->count = 1;
        *active = TRUE; /* swapping has been made */
      } else {
        f00 = f01 = sup->f;
      }

      if (BiddyV(sup->t) == high) {
        f10 = BiddyE(sup->t);
        f11 = BiddyT(sup->t);

        assert( Biddy_Managed_IsSmaller(MNG,low,BiddyV(f10)) );
        assert( Biddy_Managed_IsSmaller(MNG,low,BiddyV(f11)) );

        /* sup->t is not needed in sup, anymore */
        /* we will mark it as obsolete and we are expecting, that */
        /* formulae are repaired before garbage collection */
        ((BiddyNode *) sup->t)->count = 1;
        *active = TRUE; /* swapping has been made */
      } else {
        f10 = f11 = sup->t;
      }

      if (f00 == f10) {
        u0 = f00;
      } else {
        /* u0 is a new node or an existing node */
        /* in any case, it is a needed node */
        /* here, GC will not be called during FoaNode */
        u0 = Biddy_Managed_FoaNode(MNG,low,f00,f10,FALSE);

/* THIS IS NOT NECESSARY BUT MAYBE IT IS USEFUL */
/**/
        assert( ((BiddyNode *) Biddy_Regular(u0))->v == low );
        BiddyProlongOne(MNG,u0,sup->count);
/**/
      }

      if (f01 == f11) {
        u1 = f01;
      } else {
        /* u1 is a new node or an existing node */
        /* in any case, it is a needed node */
        /* here, GC will not be called during FoaNode */
        u1 = Biddy_Managed_FoaNode(MNG,low,f01,f11,FALSE);

/* THIS IS NOT NECESSARY BUT MAYBE IT IS USEFUL */
/**/
        assert( ((BiddyNode *) Biddy_Regular(u1))->v == low );
        BiddyProlongOne(MNG,u1,sup->count);
/**/
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
      tmp1 = findNodeTable(MNG,high,u0,u1,&tmp); /* to calculate tmp1, such node cannot exists! */
      addNodeTable(MNG,hash,sup,tmp1); /* add node to hash table */

      newEnd = sup;

    } else {

      /* this node with variable 'low' has not been changed to 'high' */
      /* because it does not have successors with variable 'high' */

      /* do not add this node to the list of renamed nodes (newBegin,newEnd) */
      if (!newEnd) {
        newBegin = (BiddyNode *) sup->list;
      } else {
        newEnd->list = sup->list;
      }
      /* relink to the end of list for low */
      assert( BiddyV(sup) == low );
      biddyVariableTable.table[low].lastNewNode->list = (void *) sup;
      biddyVariableTable.table[low].lastNewNode = sup;

    }

    sup = (BiddyNode *) sup->list;
  }

  /* all nodes in the list from newBegin to newEnd (inclusively) are renamed */
  /* thus insert the complete list into list for 'high' */
  if (newEnd) {
    (biddyVariableTable.table[high].lastNewNode)->list = (void *) newBegin;
    biddyVariableTable.table[high].lastNewNode = newEnd;
  }

  /* manage new lists for 'low' */
  /* at least one new node has been added to new list for 'low' */
  /* e.g. node representing variable is never renamed to 'high' */
  /* please note, that the first element is still a temporary one */
  assert(biddyVariableTable.table[low].lastNewNode != biddyNodeTable.table[0] );
  biddyVariableTable.table[low].firstNewNode =
    (BiddyNode *) (biddyVariableTable.table[low].firstNewNode)->list;

  /* REPAIR CONSTANT NODE */
  biddyNodeTable.table[0]->list = NULL;

  /* DEBUGGING */
  /*
  printf("SWAPPING COMPLETE");
  BiddySystemReport(MNG);
  */

  /* REPAIR BROKEN NODES */
  /* YOU HAVE TO PRESERVE FIRST NODE IN THE LIST */
  /* this is need only if swaps have been made */
  if (*active) {

    for (n = 0; n < biddyFormulaTable.size; n++) {
      if (!Biddy_IsNull(biddyFormulaTable.table[n].f)) {
        BiddyProlongOne(MNG,biddyFormulaTable.table[n].f,biddyFormulaTable.table[n].count);
      }
    }

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
  unsigned int i,j;

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
    For each variable its number is counted in numone.
### More info
*******************************************************************************/

static void
nodeNumberOrdering(Biddy_Manager MNG, Biddy_Edge f, unsigned int *i,
                   BiddyOrderingTable ordering)
{
  Biddy_Edge sup;

  if (!Biddy_IsConstant(f)) {
    biddyVariableTable.table[BiddyV(f)].numone++;
    Biddy_Managed_SelectNode(MNG,f);
    (*i)++;
    if (!Biddy_IsConstant((sup=BiddyE(f)))) {
      SET_ORDER(ordering,BiddyV(f),BiddyV(sup));
      if (!Biddy_Managed_IsSelected(MNG,sup)) nodeNumberOrdering(MNG,sup,i,ordering);
    }
    if (!Biddy_IsConstant((sup=BiddyT(f)))) {
      SET_ORDER(ordering,BiddyV(f),BiddyV(sup));
      if (!Biddy_Managed_IsSelected(MNG,sup)) nodeNumberOrdering(MNG,sup,i,ordering);
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
#pragma warning(suppress: 6001)
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
    data.mintermCount is using GNU Multiple Precision Arithmetic Library (GMP).
### More info
*******************************************************************************/

static BiddyLocalInfo *
createLocalInfo(Biddy_Manager MNG, Biddy_Edge f, BiddyLocalInfo *c)
{
  if (Biddy_Managed_IsSelected(MNG,f)) {
    Biddy_Managed_DeselectNode(MNG,f);
    c->back = (BiddyNode *)Biddy_Regular(f);
    c->org = (BiddyNode *)(((BiddyNode *) Biddy_Regular(f))->list);
    ((BiddyNode *) Biddy_Regular(f))->list = (void *) c;
    c->data.enumerator = 0;
    c = &c[1]; /* next field in the array */
    c = createLocalInfo(MNG,BiddyE(f),c);
    c = createLocalInfo(MNG,BiddyT(f),c);
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
    TO DO: memory reserved for data.mintermCount is not removed!
### More info
*******************************************************************************/

static void
deleteLocalInfo(Biddy_Manager MNG, Biddy_Edge f)
{
  if (Biddy_Managed_IsSelected(MNG,f)) {
    Biddy_Managed_DeselectNode(MNG,f);
    ((BiddyNode *) Biddy_Regular(f))->list = (void *)
      ((BiddyLocalInfo *)((BiddyNode *) Biddy_Regular(f))->list)->org;
    deleteLocalInfo(MNG,BiddyE(f));
    deleteLocalInfo(MNG,BiddyT(f));
  }
}

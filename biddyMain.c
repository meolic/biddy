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
                 garbage collection with a system age is implemented.
                 Variable swapping and sifting are implemented.]

    FileName    [biddyMain.c]
    Revision    [$Revision: 319 $]
    Date        [$Date: 2017-09-30 22:37:26 +0200 (sob, 30 sep 2017) $]
    Authors     [Robert Meolic (robert.meolic@um.si),
                 Ales Casar (ales@homemade.net)]

### Copyright

Copyright (C) 2006, 2017 UM FERI, Koroska cesta 46, SI-2000 Maribor, Slovenia

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

static Biddy_Boolean checkFunctionOrdering(Biddy_Manager MNG, Biddy_Edge f);

static Biddy_Boolean isEqv(Biddy_Manager MNG1, Biddy_Edge f1, Biddy_Manager MNG2, Biddy_Edge f2);

static unsigned int nodeTableHash(Biddy_Variable v, Biddy_Edge pf, Biddy_Edge pt, unsigned int size);

static unsigned int op3Hash(Biddy_Edge a, Biddy_Edge b, Biddy_Edge c, unsigned int size);

static void exchangeEdges(Biddy_Edge *f, Biddy_Edge *g);

static void complExchangeEdges(Biddy_Edge *f, Biddy_Edge *g);

static void addNodeTable(Biddy_Manager MNG, unsigned int hash, BiddyNode *node, BiddyNode *sup1);

static BiddyNode *findNodeTable(Biddy_Manager MNG, Biddy_Variable v, Biddy_Edge pf, Biddy_Edge pt, BiddyNode **thesup);

static void addOp3Cache(Biddy_Manager MNG, BiddyOp3CacheTable cache, Biddy_Edge a, Biddy_Edge b, Biddy_Edge c, Biddy_Edge r, unsigned int index);

static Biddy_Boolean findOp3Cache(Biddy_Manager MNG, BiddyOp3CacheTable cache, Biddy_Edge a, Biddy_Edge b, Biddy_Edge c, Biddy_Edge *r, unsigned int *index);

static void OPGarbage(Biddy_Manager MNG);

static void OPGarbageDeleteAll(Biddy_Manager MNG);

static void EAGarbage(Biddy_Manager MNG);

static void EAGarbageDeleteAll(Biddy_Manager MNG);

static void RCGarbage(Biddy_Manager MNG);

static void RCGarbageDeleteAll(Biddy_Manager MNG);

static Biddy_Edge addVariableElement(Biddy_Manager MNG, Biddy_String x);

static Biddy_Variable swapWithHigher(Biddy_Manager MNG, Biddy_Variable v, Biddy_Boolean *active);

static Biddy_Variable swapWithLower(Biddy_Manager MNG, Biddy_Variable v, Biddy_Boolean *active);

static Biddy_Boolean isSmaller(BiddyOrderingTable *orderingTable, Biddy_Variable fv, Biddy_Variable gv);

static Biddy_Variable prevVariable(BiddyOrderingTable *orderingTable, Biddy_Variable num, Biddy_Variable v);

static Biddy_Variable nextVariable(BiddyOrderingTable *orderingTable, Biddy_Variable num, Biddy_Variable v);

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
\brief Function Biddy_InitMNG initialize a manager.

### Description
    Biddy_InitMNG creates and initializes a manager.
    Initialization consists of creating manager structure (MNG),
    node table (biddyNodeTable), variable table (biddyVariableTable),
    formula table (biddyFormulaTable), three basic caches (biddyOPCache,
    biddyEACache and biddyRCCache), and cache list (biddyCacheList).
    Biddy_InitMNG also initializes constant edges (biddyOne, biddyZero),
    memory management and automatic garbage collection.
### Side effects
    Allocates a lot of memory. Parameter gddtype is ignored.
### More info
    Macro Biddy_Init() will initialize anonymous manager.
*******************************************************************************/

#ifdef __cplusplus
extern "C" {
#endif

void
Biddy_InitMNG(Biddy_Manager *mng, int gddtype)
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

  /* RANDOM NUMBERS GENERATOR */
  srand((unsigned int) time(NULL));

  if (!mng) {
    if (!(biddyAnonymousManager = (Biddy_Manager) malloc(sizeof(BiddyManager)))) {
      fprintf(stderr,"Biddy_InitMNG: Out of memoy!\n");
      exit(1);
    }
    MNG = biddyAnonymousManager;
  } else {
    if (!(*mng = (Biddy_Manager) malloc(sizeof(BiddyManager)))) {
      fprintf(stderr,"Biddy_InitMNG: Out of memoy!\n");
      exit(1);
    }
    MNG = *mng;
  }

  /* CREATION OF MANAGER'S STRUCTURES - VALUES ARE NOT INITIALIZED HERE */

  MNG[0] = NULL; /* biddyManagerName */
  if (!(MNG[1] = (short int *) malloc(sizeof(short int)))) {
    fprintf(stderr,"Biddy_InitMNG: Out of memoy!\n");
    exit(1);
  }
  biddyManagerType = 0;
  MNG[2] = NULL; /* biddyTerminal */
  MNG[3] = NULL; /* biddyZero */
  MNG[4] = NULL; /* biddyOne */
  if (!(MNG[5] = (BiddyNodeTable *) malloc(sizeof(BiddyNodeTable)))) {
    fprintf(stderr,"Biddy_InitMNG: Out of memoy!\n");
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
  biddyNodeTable.funandor = 0;
  biddyNodeTable.funxor = 0;
  biddyNodeTable.gctime = 0;
  biddyNodeTable.drtime = 0;
  biddyNodeTable.gcratio = 0.0;
  biddyNodeTable.gcratioF = 0.0;
  biddyNodeTable.gcratioX = 0.0;
  biddyNodeTable.resizeratio = 0.0;
  biddyNodeTable.resizeratioF = 0.0;
  biddyNodeTable.resizeratioX = 0.0;
  biddyNodeTable.siftingtreshold = 0.0;
  biddyNodeTable.fsiftingtreshold = 0.0;
  biddyNodeTable.convergesiftingtreshold = 0.0;
  biddyNodeTable.fconvergesiftingtreshold = 0.0;
#ifdef BIDDYEXTENDEDSTATS_YES
  biddyNodeTable.foa = 0;
  biddyNodeTable.find = 0;
  biddyNodeTable.compare = 0;
  biddyNodeTable.add = 0;
  biddyNodeTable.iterecursive = 0;
  biddyNodeTable.andorrecursive = 0;
  biddyNodeTable.xorrecursive = 0;
  biddyNodeTable.gcobsolete = NULL;
#endif
  if (!(MNG[6] = (BiddyVariableTable *) malloc(sizeof(BiddyVariableTable)))) {
    fprintf(stderr,"Biddy_InitMNG: Out of memoy!\n");
    exit(1);
  }
  biddyVariableTable.table = NULL;
  biddyVariableTable.lookup = NULL;
  biddyVariableTable.size = 0;
  biddyVariableTable.num = 0;
  biddyVariableTable.numnum = 0;
  if (!(MNG[7] = (BiddyFormulaTable *) malloc(sizeof(BiddyFormulaTable)))) {
    fprintf(stderr,"Biddy_InitMNG: Out of memoy!\n");
    exit(1);
  }
  biddyFormulaTable.table = NULL;
  biddyFormulaTable.size = 0;
  biddyFormulaTable.numOrdered = 0;
  biddyFormulaTable.deletedName = NULL;
  if (!(MNG[8] = (BiddyOp3CacheTable *) malloc(sizeof(BiddyOp3CacheTable)))) {
    fprintf(stderr,"Biddy_InitMNG: Out of memoy!\n");
    exit(1);
  }
  biddyOPCache.table = NULL;
  biddyOPCache.size = 0;
  biddyOPCache.disabled = FALSE;
  if (!(biddyOPCache.search = (unsigned long long int *) malloc(sizeof(unsigned long long int)))) {
    fprintf(stderr, "Biddy_InitMNG: Out of memoy!\n");
    exit(1);
  }
  if (!(biddyOPCache.find = (unsigned long long int *) malloc(sizeof(unsigned long long int)))) {
    fprintf(stderr, "Biddy_InitMNG: Out of memoy!\n");
    exit(1);
  }
#ifdef BIDDYEXTENDEDSTATS_YES
  if (!(biddyOPCache.insert = (unsigned long long int *) malloc(sizeof(unsigned long long int)))) {
    fprintf(stderr, "Biddy_InitMNG: Out of memoy!\n");
    exit(1);
  }
  if (!(biddyOPCache.overwrite = (unsigned long long int *) malloc(sizeof(unsigned long long int)))) {
    fprintf(stderr, "Biddy_InitMNG: Out of memoy!\n");
    exit(1);
  }
#endif
  *(biddyOPCache.search) = 0;
  *(biddyOPCache.find) = 0;
#ifdef BIDDYEXTENDEDSTATS_YES
  *(biddyOPCache.insert) = 0;
  *(biddyOPCache.overwrite) = 0;
#endif
  if (!(MNG[9] = (BiddyOp3CacheTable *) malloc(sizeof(BiddyOp3CacheTable)))) {
    fprintf(stderr,"Biddy_InitMNG: Out of memoy!\n");
    exit(1);
  }
  biddyEACache.table = NULL;
  biddyEACache.size = 0;
  biddyEACache.disabled = FALSE;
  if (!(biddyEACache.search = (unsigned long long int *) malloc(sizeof(unsigned long long int)))) {
    fprintf(stderr, "Biddy_InitMNG: Out of memoy!\n");
    exit(1);
  }
  if (!(biddyEACache.find = (unsigned long long int *) malloc(sizeof(unsigned long long int)))) {
    fprintf(stderr, "Biddy_InitMNG: Out of memoy!\n");
    exit(1);
  }
#ifdef BIDDYEXTENDEDSTATS_YES
  if (!(biddyEACache.insert = (unsigned long long int *) malloc(sizeof(unsigned long long int)))) {
    fprintf(stderr, "Biddy_InitMNG: Out of memoy!\n");
    exit(1);
  }
  if (!(biddyEACache.overwrite = (unsigned long long int *) malloc(sizeof(unsigned long long int)))) {
    fprintf(stderr, "Biddy_InitMNG: Out of memoy!\n");
    exit(1);
  }
#endif
  *(biddyEACache.search) = 0;
  *(biddyEACache.find) = 0;
#ifdef BIDDYEXTENDEDSTATS_YES
  *(biddyEACache.insert) = 0;
  *(biddyEACache.overwrite) = 0;
#endif
  if (!(MNG[10] = (BiddyOp3CacheTable *) malloc(sizeof(BiddyOp3CacheTable)))) {
    fprintf(stderr,"Biddy_InitMNG: Out of memoy!\n");
    exit(1);
  }
  biddyRCCache.table = NULL;
  biddyRCCache.size = 0;
  biddyRCCache.disabled = FALSE;
  if (!(biddyRCCache.search = (unsigned long long int *) malloc(sizeof(unsigned long long int)))) {
    fprintf(stderr, "Biddy_InitMNG: Out of memoy!\n");
    exit(1);
  }
  if (!(biddyRCCache.find = (unsigned long long int *) malloc(sizeof(unsigned long long int)))) {
    fprintf(stderr, "Biddy_InitMNG: Out of memoy!\n");
    exit(1);
  }
#ifdef BIDDYEXTENDEDSTATS_YES
  if (!(biddyRCCache.insert = (unsigned long long int *) malloc(sizeof(unsigned long long int)))) {
    fprintf(stderr, "Biddy_InitMNG: Out of memoy!\n");
    exit(1);
  }
  if (!(biddyRCCache.overwrite = (unsigned long long int *) malloc(sizeof(unsigned long long int)))) {
    fprintf(stderr, "Biddy_InitMNG: Out of memoy!\n");
    exit(1);
  }
#endif
  *(biddyRCCache.search) = 0;
  *(biddyRCCache.find) = 0;
#ifdef BIDDYEXTENDEDSTATS_YES
  *(biddyRCCache.insert) = 0;
  *(biddyRCCache.overwrite) = 0;
#endif
  if (!(MNG[11] = (BiddyCacheList* *) malloc(sizeof(BiddyCacheList*)))) {
    fprintf(stderr,"Biddy_InitMNG: Out of memoy!\n");
    exit(1);
  }
  biddyCacheList = NULL;
  if (!(MNG[12] = (BiddyNode* *) malloc(sizeof(BiddyNode*)))) {
    fprintf(stderr,"Biddy_InitMNG: Out of memoy!\n");
    exit(1);
  }
  biddyFreeNodes = NULL;
  if (!(MNG[13] = (BiddyOrderingTable *) malloc(sizeof(BiddyOrderingTable)))) {
    fprintf(stderr,"Biddy_InitMNG: Out of memoy!\n");
    exit(1);
  }
  if (!(MNG[14] = (unsigned int *) malloc(sizeof(unsigned int)))) {
    fprintf(stderr,"Biddy_InitMNG: Out of memoy!\n");
    exit(1);
  }
  biddySystemAge = 0;
  if (!(MNG[15] = (unsigned short int *) malloc(sizeof(unsigned short int)))) {
    fprintf(stderr,"Biddy_InitMNG: Out of memoy!\n");
    exit(1);
  }
  biddySelect = 0;

  /* INITIALIZATION OF MANAGER'S STRUCTURES  - VALUES ARE NOW INITIALIZED */
  MNG[0] = strdup("ROBDD WITH COMPLEMENTED EDGES"); /* biddyManagerName */
  biddyManagerType = BIDDYTYPEOBDD;

  /* THESE ARE DEFAULT SIZES */
  biddyVariableTable.size = BIDDYVARMAX;
  biddyNodeTable.initblocksize = MEDIUM_SIZE; /* 12.0 MB on 64-bit */
  biddyNodeTable.limitblocksize = XLARGE_SIZE; /* 96.0 MB on 64-bit */
  biddyNodeTable.initsize = SMALL_SIZE;
  biddyNodeTable.limitsize = HUGE_SIZE;
  biddyOPCache.size = MEDIUM_SIZE;
  biddyEACache.size = SMALL_SIZE;
  biddyRCCache.size = SMALL_SIZE;

  /* these values are experimentally determined */
  /* gcr=1.67, gcrF=1.20, gcrX=0.91, rr=0.01, rrF=1.45, rrX=0.98 */ /* used in v1.7.1 */
  /* gcr=1.62, gcrF=1.13, gcrX=0.98, rr=0.01, rrF=1.15, rrX=0.95 */ /* candidate for v1.7.2 */
  /* gcr=1.64, gcrF=1.40, gcrX=0.80, rr=0.01, rrF=0.76, rrX=0.86 */ /* candidate for v1.7.2 */
  biddyNodeTable.gcratio = (float) 1.64; /* do not delete nodes if the effect is to small, gcratio */
  biddyNodeTable.gcratioF = (float) 1.40; /* do not delete nodes if the effect is to small, gcratio */
  biddyNodeTable.gcratioX = (float) 0.80; /* do not delete nodes if the effect is to small, gcratio */
  biddyNodeTable.resizeratio = (float) 0.01; /* resize Node table if there are to many nodes */
  biddyNodeTable.resizeratioF = (float) 0.76; /* resize Node table if there are to many nodes */
  biddyNodeTable.resizeratioX = (float) 0.86; /* resize Node table if there are to many nodes */

  /* these values are experimentally determined */
  biddyNodeTable.siftingtreshold = 1.05;  /* stop sifting if the size of the system grows to much */
  biddyNodeTable.fsiftingtreshold = 1.05; /* stop sifting if the size of the function grows to much */
  biddyNodeTable.convergesiftingtreshold = 1.02;  /* stop one step of converging sifting if the size of the system grows to much */
  biddyNodeTable.fconvergesiftingtreshold = 1.02; /* stop one step of converging sifting if the size of the function grows to much */

  /* CREATE AND INITIALIZE NODE TABLE */
  /* THE ACTUAL SIZE OF NODE TABLE IS biddyNodeTable.size+2 */
  /* BECAUSE OF USED TRICKS, HASH FUNCTION MUST NEVER RETURN ZERO! */
  /* USEFUL INDICES ARE FROM [1] TO [biddyNodeTable.size+1] */
  biddyNodeTable.size = biddyNodeTable.initsize;
  if (!(biddyNodeTable.table = (BiddyNode **)
  calloc((biddyNodeTable.size+2),sizeof(BiddyNode *)))) {
    fprintf(stderr,"Biddy_InitMNG (node table): Out of memoy!\n");
    exit(1);
  }
  /* for (i=0;i<=biddyNodeTable.size+1;i++) biddyNodeTable.table[i] = NULL; */

  /* CREATE AND INITIALIZE VARIABLE TABLE */
  /* VARIABLE "1" HAS INDEX [0] */
  /* VARIABLES ARE ORDERED BY INDEX IN THE VARIABLE TABLE */
  /* IN THE BDD, SMALLER (PREV) VARIABLES ARE ABOVE THE GREATER (NEXT) ONES */
  /* VARIABLE "1" MUST ALWAYS HAVE MAX GLOBAL ORDER (IT IS BOTTOMMOST) */
  /* THIS PART OF CODE DETERMINES DEFAULT ORDERING FOR NEW VARIABLES: */
  /* [1] < [2] < ... < [size-1] < [0] */
  biddyVariableTable.table = (BiddyVariable *)
                malloc(biddyVariableTable.size * sizeof(BiddyVariable));
  nullOrdering(biddyOrderingTable);
  biddyVariableTable.table[0].name = strdup("1");
  biddyVariableTable.table[0].variable = biddyNull;
  biddyVariableTable.table[0].element = biddyNull;
  biddyVariableTable.table[0].num = 1;
  biddyVariableTable.table[0].prev = biddyVariableTable.size;
  biddyVariableTable.table[0].next = 0; /* fixed */
  for (i=1; i<biddyVariableTable.size; i++) {
    biddyVariableTable.table[i].num = 0;
    biddyVariableTable.table[i].name = NULL;
    biddyVariableTable.table[i].variable = biddyNull;
    biddyVariableTable.table[i].element = biddyNull;
    biddyVariableTable.table[i].selected = FALSE;
    SET_ORDER(biddyOrderingTable,i,0);
    biddyVariableTable.table[i].prev = i-1;
    biddyVariableTable.table[i].next = i+1;
    for (j=i+1; j<biddyVariableTable.size; j++) {
      SET_ORDER(biddyOrderingTable,i,j);
    }
  }
  biddyVariableTable.num = 1; /* ONLY VARIABLE '1' IS CREATED */
  biddyVariableTable.numnum = 1; /* VARIABLE '1' IS A NUMBERED VARIABLE */

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

  /* INITIALIZATION OF MEMORY MANAGEMENT */
  /* ALLOCATE FIRST CHUNK OF NODES */
  biddyNodeTable.blocksize = biddyNodeTable.initblocksize;
  if (!(biddyFreeNodes = (BiddyNode *)
        malloc((biddyNodeTable.blocksize) * sizeof(BiddyNode)))) {
    fprintf(stderr,"Biddy_InitMNG (nodes): Out of memoy!\n");
    exit(1);
  }
  if (!(biddyNodeTable.blocktable = (BiddyNode **)malloc(sizeof(BiddyNode *)))) {
    fprintf(stderr, "Biddy_InitMNG (blocktable): Out of memoy!\n");
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

  /* MAKE CONSTANT NODE "1" */
  /* SINCE BIDDY V1.2: CONSTANT NODE "1" IS AT INDEX [0] */
  biddyNodeTable.table[0]->prev = NULL;
  biddyNodeTable.table[0]->next = NULL;
  biddyNodeTable.table[0]->list = NULL;
  biddyNodeTable.table[0]->f = biddyNull;
  biddyNodeTable.table[0]->t = biddyNull;
  biddyNodeTable.table[0]->v = 0;
  biddyNodeTable.table[0]->expiry = 0; /* constant node is fortified */
  biddyNodeTable.table[0]->select = 0; /* initialy it is not selected */

  /* MAKE TERMINAL EDGE AND EDGES REPRESENTING CONSTANTS */
  /* FOR THE EMPTY DOMAIN, biddyZero AND biddyOne ARE THE SAME FOR ALL GDD TYPES */
  MNG[2] = (void *) biddyNodeTable.table[0]; /* biddyTerminal */
  MNG[3] = Biddy_Complement(biddyTerminal); /* biddyZero */
  MNG[4] = biddyTerminal; /* biddyOne */

  /* INITIALIZATION OF VARIABLE'S VALUES */
  biddyVariableTable.table[0].value = biddyZero;
  for (i=1; i<biddyVariableTable.size; i++) {
#pragma warning(suppress: 6386)
    biddyVariableTable.table[i].value = biddyZero;
  }

  /* INITIALIZE FORMULA TABLE */
  biddyFormulaTable.deletedName = strdup("BIDDY_DELETED_FORMULA");
  biddyFormulaTable.size = 2;
  biddyFormulaTable.numOrdered = 2;
  if (!(biddyFormulaTable.table = (BiddyFormula *)
        malloc(biddyFormulaTable.size*sizeof(BiddyFormula))))
  {
    fprintf(stderr,"Biddy_InitMNG (formula table): Out of memoy!\n");
    exit(1);
  }
  biddyFormulaTable.table[0].name = strdup("0");
  biddyFormulaTable.table[0].f = biddyZero;
  biddyFormulaTable.table[0].expiry = 0;
  biddyFormulaTable.table[0].deleted = FALSE;
  biddyFormulaTable.table[1].name = strdup("1");
  biddyFormulaTable.table[1].f = biddyOne;
  biddyFormulaTable.table[1].expiry = 0;
  biddyFormulaTable.table[1].deleted = FALSE;

  /* INITIALIZATION OF NODE TABLE */
  biddyNodeTable.max = 1;
  biddyNodeTable.num = 1;
  biddyNodeTable.garbage = 0;
  biddyNodeTable.swap = 0;
  biddyNodeTable.sifting = 0;
  biddyNodeTable.nodetableresize = 0;
  biddyNodeTable.funite = 0;
  biddyNodeTable.funandor = 0;
  biddyNodeTable.funxor = 0;
  biddyNodeTable.gctime = 0;
  biddyNodeTable.drtime = 0;
#ifdef BIDDYEXTENDEDSTATS_YES
  biddyNodeTable.foa = 0;
  biddyNodeTable.find = 0;
  biddyNodeTable.compare = 0;
  biddyNodeTable.add = 0;
  biddyNodeTable.iterecursive = 0;
  biddyNodeTable.andorrecursive = 0;
  biddyNodeTable.xorrecursive = 0;
  biddyNodeTable.gcobsolete = NULL;
#endif

  /* INITIALIZATION OF NODE SELECTION */
  biddySelect = 1;

  /* INITIALIZATION OF GARBAGE COLLECTION */
  /* MINIMAL VALUE FOR biddySystemAge IS 2 (SEE IMPLEMENTATION OF SIFTING) */
  biddySystemAge = 2;

  /* INITIALIZATION OF CACHE LIST */
  biddyCacheList = NULL;

  /* INITIALIZATION OF DEFAULT ITE CACHE - USED FOR ITE OPERATION */
  biddyOPCache.disabled = FALSE;
  *(biddyOPCache.search) = *(biddyOPCache.find) = 0;
#ifdef BIDDYEXTENDEDSTATS_YES
  *(biddyOPCache.insert) = *(biddyOPCache.overwrite) = 0;
#endif
  if (!(biddyOPCache.table = (BiddyOp3Cache *)
  calloc((biddyOPCache.size+1),sizeof(BiddyOp3Cache)))) {
    fprintf(stderr,"Biddy_InitMNG (OP cache): Out of memoy!\n");
    exit(1);
  }
  /* for (i=0;i<=biddyOPCache.size;i++) biddyOPCache.table[i].result = biddyNull; */
  Biddy_Managed_AddCache(MNG,OPGarbage);

  /* INITIALIZATION OF DEFAULT EA CACHE - USED FOR QUANTIFICATIONS */
  biddyEACache.disabled = FALSE;
  *(biddyEACache.search) = *(biddyEACache.find) = 0;
#ifdef BIDDYEXTENDEDSTATS_YES
  *(biddyEACache.insert) = *(biddyEACache.overwrite) = 0;
#endif
  if (!(biddyEACache.table = (BiddyOp3Cache *)
  calloc((biddyEACache.size+1),sizeof(BiddyOp3Cache)))) {
    fprintf(stderr,"Biddy_InitMNG (EA cache): Out of memoy!\n");
    exit(1);
  }
  /* for (i=0;i<=biddyEACache.size;i++) biddyEACache.table[i].result = biddyNull; */
  Biddy_Managed_AddCache(MNG,EAGarbage);

  /* INITIALIZATION OF DEFAULT RC CACHE - USED FOR RESTRICT AND COMPOSE */
  biddyRCCache.disabled = FALSE;
  *(biddyRCCache.search) = *(biddyRCCache.find) = 0;
#ifdef BIDDYEXTENDEDSTATS_YES
  *(biddyRCCache.insert) = *(biddyRCCache.overwrite) = 0;
#endif
  if (!(biddyRCCache.table = (BiddyOp3Cache *)
  calloc((biddyRCCache.size+1),sizeof(BiddyOp3Cache)))) {
    fprintf(stderr,"Biddy_InitMNG (RC cache): Out of memoy!\n");
    exit(1);
  }
  /* for (i=0;i<=biddyRCCache.size;i++) biddyRCCache.table[i].result = biddyNull; */
  Biddy_Managed_AddCache(MNG,RCGarbage);

  /* DEBUGGING */
  /*
  BiddySystemReport(MNG);
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
\brief Function Biddy_ExitMNG deletes a manager.

### Description
    Deallocates all memory allocated by Biddy_InitMNG, Biddy_FoaVariable,
    Biddy_FoaNode etc.
### Side effects
### More info
    Macro Biddy_Exit() will delete anonymous manager.
*******************************************************************************/

#ifdef __cplusplus
extern "C" {
#endif

void
Biddy_ExitMNG(Biddy_Manager *mng)
{
  unsigned int i;
  BiddyCacheList *sup1, *sup2;
  Biddy_Manager MNG;

  if (!mng) {
    MNG = biddyAnonymousManager;
  } else {
    MNG = *mng;
  }

  /* REPORT ABOUT THE SYSTEM */
  /* CHECKING THE CORRECTNESS AND THE EFFICIENCY OF MEMORY MANAGEMENT */

#ifdef BIDDYEXTENDEDSTATS_YES
  BiddySystemReport(MNG);
  printf("Number of ITE calls: %u (internal direct and recursive calls: %llu)\n",biddyNodeTable.funite,biddyNodeTable.iterecursive);
  printf("Number of AND and OR calls: %u (internal direct and recursive calls: %llu)\n",biddyNodeTable.funandor,biddyNodeTable.andorrecursive);
  printf("Number of XOR calls: %u (internal direct and recursive calls: %llu)\n",biddyNodeTable.funxor,biddyNodeTable.xorrecursive);
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

  /*
  printf("Delete GDD type ...\n");
  */
  free((short int*)(MNG[1]));

  MNG[2] = NULL; /* biddyTerminal */
  MNG[3] = NULL; /* biddyZero */
  MNG[4] = NULL; /* biddyOne */

  /*
  printf("Delete nodes, block table, and node table...\n");
  */
  if ((BiddyNodeTable*)(MNG[5])) {
    for (i = 0; i < biddyNodeTable.blocknumber; i++) {
      free(biddyNodeTable.blocktable[i]);
    }
    free(biddyNodeTable.blocktable);
    free(biddyNodeTable.table);
#pragma warning(suppress: 6001)
    free((BiddyNodeTable*)(MNG[5]));
  }

  /* TO DO: REMOVE VARIABLE NAMES! */
  /*
  printf("Delete variable table...\n");
  */
  if ((BiddyVariableTable*)(MNG[6])) {
    free(biddyVariableTable.table);
    free(biddyVariableTable.lookup);
#pragma warning(suppress: 6001)
    free((BiddyVariableTable*)(MNG[6]));
  }

  /* TO DO: REMOVE FORMULAE NAMES! */
  /*
  printf("Delete formula table...\n");
  */
  if ((BiddyFormulaTable*)(MNG[7])) {
    free(biddyFormulaTable.table);
    free(biddyFormulaTable.deletedName);
#pragma warning(suppress: 6001)
    free((BiddyFormulaTable*)(MNG[7]));
  }

  /*
  printf("Delete OP cache...\n");
  */
  if ((BiddyOp3CacheTable*)(MNG[8])) {
    free(biddyOPCache.table);
    free(biddyOPCache.search);
    free(biddyOPCache.find);
#ifdef BIDDYEXTENDEDSTATS_YES
    free(biddyOPCache.insert);
    free(biddyOPCache.overwrite);
#endif
#pragma warning(suppress: 6001)
    free((BiddyOp3CacheTable*)(MNG[8]));
  }

  /*
  printf("Delete EA cache...\n");
  */
  if ((BiddyOp3CacheTable*)(MNG[9])) {
    free(biddyEACache.table);
    free(biddyEACache.search);
    free(biddyEACache.find);
#ifdef BIDDYEXTENDEDSTATS_YES
    free(biddyEACache.insert);
    free(biddyEACache.overwrite);
#endif
#pragma warning(suppress: 6001)
    free((BiddyOp3CacheTable*)(MNG[9]));
  }

  /*
  printf("Delete RC cache...\n");
  */
  if ((BiddyOp3CacheTable*)(MNG[10])) {
    free(biddyRCCache.table);
    free(biddyRCCache.search);
    free(biddyRCCache.find);
#ifdef BIDDYEXTENDEDSTATS_YES
    free(biddyRCCache.insert);
    free(biddyRCCache.overwrite);
#endif
#pragma warning(suppress: 6001)
    free((BiddyOp3CacheTable*)(MNG[10]));
  }

  /*
  printf("Delete cache list...\n");
  */
  if ((BiddyCacheList**)(MNG[11])) {
#pragma warning(suppress: 6001)
    sup1 = biddyCacheList;
    while (sup1) {
      sup2 = sup1->next;
      free(sup1);
      sup1 = sup2;
    }
#pragma warning(suppress: 6001)
    free((BiddyCacheList**)(MNG[11]));
  }

  /*
  printf("Delete pointer biddyFreeNodes...\n");
  */
#pragma warning(suppress: 6001)
  free((BiddyNode**)(MNG[12]));

  /*
  printf("Delete Ordering table...\n");
  */
#pragma warning(suppress: 6001)
  free((BiddyOrderingTable*)(MNG[13]));

  /*
  printf("Delete age...\n");
  */
  free((unsigned int*)(MNG[14]));

  /*
  printf("Delete selector...\n");
  */
  free((unsigned short int*)(MNG[15]));

  /*
  printf("Delete statistic vectors...\n");
  */
#ifdef BIDDYEXTENDEDSTATS_YES
  if (biddyNodeTable.gcobsolete) free(biddyNodeTable.gcobsolete);
#endif

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
\brief Function Biddy_Managed_GetManagerType reports BDD type used in the
       manager.

### Description
### Side effects
### More info
    Macro Biddy_GetManagerType() is defined for use with anonymous manager.
*******************************************************************************/

#ifdef __cplusplus
extern "C" {
#endif

int Biddy_Managed_GetManagerType(Biddy_Manager MNG)
{
  if (!MNG) MNG = biddyAnonymousManager;

  return (*((short int*)(MNG[1])));
}

#ifdef __cplusplus
}
#endif

/***************************************************************************//*!
\brief Function Biddy_Managed_SetManagerParameters set modifiable parameters.

### Description
    Function expect 6 float values. If the value is < 0 then the parameter
    is not modified. The parameters are:
    biddyNodeTable.gcratio (do not delete nodes if the effect is to small),
    biddyNodeTable.gcratioF (do not delete nodes if the effect is to small),
    biddyNodeTable.gcratioX (do not delete nodes if the effect is to small),
    biddyNodeTable.resizeratio (resize Node table if there are to many nodes),
    biddyNodeTable.resizeratioF (resize Node table if there are to many nodes),
    biddyNodeTable.resizeratioX (resize Node table if there are to many nodes),
    biddyNodeTable.siftingtreshold (stop sifting if the size of the system
                                    grows to much),
    biddyNodeTable.fsiftingtreshold (stop sifting if the size of the function
                                     grows to much),
    biddyNodeTable.convergesiftingtreshold (stop one step of converging sifting
                                     if the size of the system grows to much),
    biddyNodeTable.fconvergesiftingtreshold (stop one step of converging sifting
                                     if the size of the function grows to much).
### Side effects
    Initial values are given in Biddy_InitMNG.
### More info
    Macro Biddy_SetManagerParameters() is defined for use with anonymous
    manager.
*******************************************************************************/

#ifdef __cplusplus
extern "C" {
#endif

void
Biddy_Managed_SetManagerParameters(Biddy_Manager MNG, float gcr, float gcrF,
  float gcrX, float rr, float rrF, float rrX, float st, float fst,
  float cst, float fcst)
{
  if (!MNG) MNG = biddyAnonymousManager;

  if (gcr >= 0.0) biddyNodeTable.gcratio = gcr;
  if (gcrF >= 0.0) biddyNodeTable.gcratioF = gcrF;
  if (gcrX >= 0.0) biddyNodeTable.gcratioX = gcrX;
  if (rr >= 0.0) biddyNodeTable.resizeratio = rr;
  if (rrF >= 0.0) biddyNodeTable.resizeratioF = rrF;
  if (rrX >= 0.0) biddyNodeTable.resizeratioX = rrX;
  if (st >= 0.0) biddyNodeTable.siftingtreshold = st;
  if (fst >= 0.0) biddyNodeTable.fsiftingtreshold = fst;
  if (cst >= 0.0) biddyNodeTable.convergesiftingtreshold = cst;
  if (fcst >= 0.0) biddyNodeTable.fconvergesiftingtreshold = fcst;
}

#ifdef __cplusplus
}
#endif

/***************************************************************************//*!
\brief Function Biddy_GetThen returns THEN successor.

### Description
    Input mark is not transfered! External use, only.
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
  Biddy_Edge r;

  r = BiddyT(fun);

  /* EXTERNAL FUNCTIONS MUST RETURN NON-OBSOLETE NODE */
  if ((BiddyN(r)->expiry) && 
      ((BiddyN(r)->expiry<BiddyN(fun)->expiry) || !(BiddyN(fun)->expiry))
     )
  {
    BiddyN(r)->expiry = BiddyN(fun)->expiry;
  }

  return r;
}

#ifdef __cplusplus
}
#endif

/***************************************************************************//*!
\brief Function Biddy_GetElse returns ELSE successor.

### Description
    Input mark is not transfered! External use, only.
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
  Biddy_Edge r;

  r = BiddyE(fun);

  /* EXTERNAL FUNCTIONS MUST RETURN NON-OBSOLETE NODE */
  if ((BiddyN(r)->expiry) && 
      ((BiddyN(r)->expiry<BiddyN(fun)->expiry) || !(BiddyN(fun)->expiry))
     )
  {
    BiddyN(r)->expiry = BiddyN(fun)->expiry;
  }

  return r;
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
\brief Function Biddy_Managed_IsEqv returns TRUE iff two BDDs are equal.

### Description
    It is assumed that f1 and f2 have the same ordering.
### Side effects
### More info
    Macro Biddy_IsEqv(f1,MNG2,f2) is defined for use with anonymous manager.
*******************************************************************************/

#ifdef __cplusplus
extern "C" {
#endif

Biddy_Boolean
Biddy_Managed_IsEqv(Biddy_Manager MNG1, Biddy_Edge f1, Biddy_Manager MNG2,
                    Biddy_Edge f2)
{
  Biddy_Boolean r;
  if (!MNG1) MNG1 = biddyAnonymousManager;

  assert( biddyManagerType1 == biddyManagerType2 );

  if (MNG1 == MNG2) return (f1 == f2);

  r = isEqv(MNG1,f1,MNG2,f2);
  Biddy_Managed_DeselectAll(MNG1);

  return r;
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

  ((BiddyNode *) BiddyP(f))->select = biddySelect;
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

  ((BiddyNode *) BiddyP(f))->select = 0;
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

  return (((BiddyNode *) BiddyP(f))->select == biddySelect);
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
  if ((p=(BiddyNode *) BiddyP(f))->select != biddySelect) {
    SP = stack = (BiddyNode **)
                  malloc(biddyNodeTable.num * sizeof(BiddyNode *));
    *(SP++) = p;
    while (SP != stack) {
      p = (*(--SP));
      p->select = biddySelect;
      if (((BiddyNode *) BiddyP(p->f))->select != biddySelect) {
        *(SP++) = (BiddyNode *) BiddyP(p->f);
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
  if (((BiddyNode *)BiddyP(f))->select != biddySelect) {
    ((BiddyNode *)BiddyP(f))->select = biddySelect;
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
\brief Function Biddy_Managed_GetTerminal returns unmarked edge pointing to
       the constant node 1.

### Description
    Terminal node depends on a manager.
### Side effects
### More info
    Internally, use macro biddyTerminal.
    Macro Biddy_GetTerminal() is defined for use with anonymous manager.
*******************************************************************************/

#ifdef __cplusplus
extern "C" {
#endif

Biddy_Edge
Biddy_Managed_GetTerminal(Biddy_Manager MNG)
{
  if (!MNG) MNG = biddyAnonymousManager;

  return biddyTerminal;
}

#ifdef __cplusplus
}
#endif

/***************************************************************************//*!
\brief Function Biddy_Managed_GetConstantZero returns constant 0.

### Description
    Constants 0 and 1 depend on a manager.
    For combination sets, constant 0 coincides with empty set.
### Side effects
### More info
    Internally, use macro biddyZero.
    Macro Biddy_GetConstantZero() is defined for use with anonymous manager.
    Macros Biddy_Managed_GetEmptySet(MNG) and Biddy_GetEmptySet() are defined
    for manipulation of combination sets.
*******************************************************************************/

#ifdef __cplusplus
extern "C" {
#endif

Biddy_Edge
Biddy_Managed_GetConstantZero(Biddy_Manager MNG)
{
  if (!MNG) MNG = biddyAnonymousManager;

  return biddyZero;
}

#ifdef __cplusplus
}
#endif

/***************************************************************************//*!
\brief Function Biddy_Managed_GetConstantOne returns constant 1.

### Description
    Constants 0 and 1 depend on a manager.
    For combination sets, constant 1 coincides with universal set.
### Side effects
### More info
    Internally, use macro biddyOne.
    Macro Biddy_GetConstantOne() is defined for use with anonymous manager.
    Macros Biddy_Managed_GetUniversalSet(MNG) and Biddy_GetUniversalSet() are
    defined for manipulation of combination sets.
*******************************************************************************/

#ifdef __cplusplus
extern "C" {
#endif

Biddy_Edge
Biddy_Managed_GetConstantOne(Biddy_Manager MNG)
{
  if (!MNG) MNG = biddyAnonymousManager;

  return biddyOne;
}

#ifdef __cplusplus
}
#endif

/***************************************************************************//*!
\brief Function Biddy_Managed_GetBaseSet returns set containing only a null
       combination, i.e. it returns {{}}.

### Description
### Side effects
### More info
    Macro Biddy_GetBaseSet() is defined for use with anonymous manager.
*******************************************************************************/

#ifdef __cplusplus
extern "C" {
#endif

Biddy_Edge
Biddy_Managed_GetBaseSet(Biddy_Manager MNG)
{
  Biddy_Edge r;
  Biddy_Variable v;

  if (!MNG) MNG = biddyAnonymousManager;

  r = biddyOne;
  for (v=1;v<biddyVariableTable.num;v++) {
    r = Biddy_Managed_ITE(MNG,biddyVariableTable.table[v].variable,biddyZero,r);
  }

  return r;
}

#ifdef __cplusplus
}
#endif

/***************************************************************************//*!
\brief Function Biddy_Managed_GetVariable returns variable with the given
       name.

### Description
### Side effects
    If variable is not found function returns 0!
### More info
    Macro Biddy_GetVariable(x) is defined for use with anonymous manager.
*******************************************************************************/

#ifdef __cplusplus
extern "C" {
#endif

Biddy_Variable
Biddy_Managed_GetVariable(Biddy_Manager MNG, Biddy_String x)
{
  Biddy_Variable v;
  Biddy_Boolean find;

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
    v = 0;
  }

  return v;
}

#ifdef __cplusplus
}
#endif

/***************************************************************************//*!
\brief Function Biddy_Managed_GetPrevVariable returns previous variable in
       the global ordering (lower, topmore).

### Description
### Side effects
### More info
    Macro Biddy_GetPrevVariable(v) is defined for use with anonymous manager.
*******************************************************************************/

#ifdef __cplusplus
extern "C" {
#endif

Biddy_Variable
Biddy_Managed_GetPrevVariable(Biddy_Manager MNG, Biddy_Variable v)
{
  if (!MNG) MNG = biddyAnonymousManager;

  if (v >= biddyVariableTable.num) {
    return 0;
  } else {
    return biddyVariableTable.table[v].prev;
  }
}

#ifdef __cplusplus
}
#endif

/***************************************************************************//*!
\brief Function Biddy_Managed_GetNextVariable returns next variable in
       the global ordering (higher, bottommore).

### Description
### Side effects
### More info
    Macro Biddy_GetNextVariable(v) is defined for use with anonymous manager.
*******************************************************************************/

#ifdef __cplusplus
extern "C" {
#endif

Biddy_Variable
Biddy_Managed_GetNextVariable(Biddy_Manager MNG, Biddy_Variable v)
{
  if (!MNG) MNG = biddyAnonymousManager;

  if (v >= biddyVariableTable.num) {
    return 0;
  } else {
    return biddyVariableTable.table[v].next;
  }
}

#ifdef __cplusplus
}
#endif

/***************************************************************************//*!
\brief Function Biddy_Managed_GetVariableEdge returns variable's edge.

### Description
### Side effects
### More info
    Macro Biddy_GetVariableEdge(v) is defined for use with anonymous manager.
*******************************************************************************/

#ifdef __cplusplus
extern "C" {
#endif

Biddy_Edge
Biddy_Managed_GetVariableEdge(Biddy_Manager MNG, Biddy_Variable v)
{
  if (!MNG) MNG = biddyAnonymousManager;

  return biddyVariableTable.table[v].variable;
}

#ifdef __cplusplus
}
#endif

/***************************************************************************//*!
\brief Function Biddy_Managed_GetElementEdge returns element's edge.

### Description
### Side effects
### More info
    Macro Biddy_GetElementEdge(v) is defined for use with anonymous manager.
*******************************************************************************/

#ifdef __cplusplus
extern "C" {
#endif

Biddy_Edge
Biddy_Managed_GetElementEdge(Biddy_Manager MNG, Biddy_Variable v)
{
  /* NOT IMPLEMENTED, YET */
  return biddyNull;
}

#ifdef __cplusplus
}
#endif

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
\brief Function Biddy_Managed_GetTopVariableEdge returns variable's edge of
       top variable.

### Description
### Side effects
    TO DO: For ZBDDs, element edge is sometimes preffered over variable edge.
### More info
    Macro Biddy_GetTopVariableEdge(f) is defined for use with anonymous
    manager.
*******************************************************************************/

#ifdef __cplusplus
extern "C" {
#endif

Biddy_Edge
Biddy_Managed_GetTopVariableEdge(Biddy_Manager MNG, Biddy_Edge f)
{
  if (!MNG) MNG = biddyAnonymousManager;

  assert( f != NULL );

  return biddyVariableTable.table[BiddyV(f)].variable;
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

  assert( f != NULL );

  return biddyVariableTable.table[BiddyV(f)].name;
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

  assert( f != NULL );

  return biddyVariableTable.table[BiddyV(f)].name[0];
}

#ifdef __cplusplus
}
#endif

/***************************************************************************//*!
\brief Function Biddy_Managed_ResetVariablesValue sets all variable's value to
       biddyZero.

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

  for (v=0; v<biddyVariableTable.num; v++) {
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
       smaller (= lower = previous = above = topmore).

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
\brief Function Biddy_Managed_FoaVariable finds variable/element or adds
       new variable (i.e. Boolean function f = x) and new element (i.e. it
       creates set {{x}}).

### Description
    If variable/element already exists, function returns the existing one.
    If x == NULL then numbered variable/element is added. Numbered
    variables/elements have only digits in its name. The current number of
    numbered variables/elements is stored in numnum. If numbered
    variable/element is requested then function increments numnum and creates
    a new (non-existing) variable/element.
    Parameter varelem is used to determine how to adapt the existing BDD base
    to keep the current formula valid (use varelem = TRUE if formulae represent
    Boolean functions and varelem = FALSE if they represent combination sets).
    The ordering of the new variable/element is determined in Biddy_InitMNG.
    Function always returns variable.
### Side effects
    Adding new variable/element may change the meaning of the existing BDDs.
    Variables and elements are repaired. Moreover, formulae are repaired with
    regards to the parameter varelem.
    For OBDDs, it is safe to add new variables/elements if BDDs are used to
    represent Boolean functions.
    User should not add numbered variables/elements with some other function.
    TO DO: Formulae in user's formula tables are not repaired, yet!
### More info
    Macro Biddy_FoaVariable(x) is defined for use with anonymous manager.
*******************************************************************************/

#ifdef __cplusplus
extern "C" {
#endif

Biddy_Variable
Biddy_Managed_FoaVariable(Biddy_Manager MNG, Biddy_String x,
                          Biddy_Boolean varelem)
{
  unsigned int i;
  Biddy_Variable v;
  int cc;
  Biddy_Boolean isNumbered,find;

  if (!MNG) MNG = biddyAnonymousManager;

  v = 0;
  isNumbered = FALSE;
  if (!x) {

    biddyVariableTable.numnum++;
    if (!(x = (Biddy_String)malloc(15))) {
      fprintf(stderr, "Biddy_Managed_FoaVariable: Out of memoy!\n");
      exit(1);
    }
    sprintf(x,"%u",biddyVariableTable.numnum);
    v = biddyVariableTable.num;
    find = FALSE;
    isNumbered = TRUE;

  } else {

    /* SEARCH FOR THE EXISTING VARIABLE WITH THE GIVEN NAME */
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
  }

  /* IF find THEN v IS THE INDEX OF THE CORRECT VARIABLE! */
  if (!find) {

    /* SEARCH FOR THE EXISTING FORMULA WITH THE SAME NAME */
    /* FORMULAE IN FORMULA TABLE ARE ORDERED BY NAME */
    for (i = 0; !find && (i < biddyFormulaTable.size); i++) {
      if (biddyFormulaTable.table[i].name) {
        cc = strcmp(x,biddyFormulaTable.table[i].name);
        if ((cc == 0) && !biddyFormulaTable.table[i].deleted) {
          find = TRUE;
          break;
        }
        if (cc < 0) break;
      } else {
        break;
      }
    }

    if (find) {
      printf("WARNING (Biddy_Managed_FoaVariable): new variable %s has the same name as en existing formula, formula is not accessible anymore!\n",x);
    }

    /* addVariableElement creates variable and element, prolongs results, and returns variable edge */
    if (!addVariableElement(MNG,x)) {
      printf("ERROR (Biddy_Managed_FoaVariable): addVariableElement returns NULL\n");
    }

    assert ( v == biddyVariableTable.num-1 );

    /* EXISTING ELEMENTS MUST BE ADAPTED */
    /* NOT IMPLEMENTED, YET */

    /* FORMULAE MUST BE ADAPTED IF THEY REPRESENT COMBINATION SETS */
    /* biddyZero AND biddyOne ALWAYS REPRESENT A BOOLEAN FORMULA! */
    if (!varelem) {
      /* NOT IMPLEMENTED, YET */
    }

  } else {

    /* assert( printf("WARNING (Biddy_Managed_FoaVariable): variable %s already exists\n",x) ); */

  }

  if (isNumbered) {
    free(x);
  }

  /* RETURN VARIABLE, WHICH HAS BEEN LOOKED FOR */
  return v;
}

#ifdef __cplusplus
}
#endif

/***************************************************************************//*!
\brief Function Biddy_Managed_AddVariableByName adds variable.

### Description
    Biddy_Managed_AddVariableByName uses Biddy_Managed_FoaVariable to find or
    add variable. Function returns variable edge. If variable already exists,
    function returns the existing variable edge. For more details see
    Biddy_Managed_FoaVariable.
### Side effects
    See Biddy_Managed_FoaVariable.
### More info
    Macro Biddy_AddVariableByName(x) is defined for use with anonymous manager.
    Macros Biddy_Managed_AddVariable(MNG) and Biddy_AddVariable() are defined
    for creating numbered variables.
*******************************************************************************/

#ifdef __cplusplus
extern "C" {
#endif

Biddy_Edge
Biddy_Managed_AddVariableByName(Biddy_Manager MNG, Biddy_String x)
{
  Biddy_Variable v;

  if (!MNG) MNG = biddyAnonymousManager;

  v = Biddy_Managed_FoaVariable(MNG,x,TRUE);
  return biddyVariableTable.table[v].variable;
}

#ifdef __cplusplus
}
#endif

/***************************************************************************//*!
\brief Function Biddy_Managed_AddElementByName adds element.

### Description
    Biddy_Managed_AddElementByName uses Biddy_Managed_FoaVariable to find or
    add element. Function returns element edge. If element already exists,
    function returns the existing element edge. For more details see
    Biddy_Managed_FoaVariable.
### Side effects
    See Biddy_Managed_FoaVariable.
### More info
    Macro Biddy_AddElementByName(x) is defined for use with anonymous manager.
    Macros Biddy_Managed_AddElement(MNG) and Biddy_AddElement() are defined for
    creating numbered elements.
*******************************************************************************/

#ifdef __cplusplus
extern "C" {
#endif

Biddy_Edge
Biddy_Managed_AddElementByName(Biddy_Manager MNG, Biddy_String x)
{
  Biddy_Variable v;

  if (!MNG) MNG = biddyAnonymousManager;

  v = Biddy_Managed_FoaVariable(MNG,x,FALSE);
  return biddyVariableTable.table[v].element;
}

#ifdef __cplusplus
}
#endif

/***************************************************************************//*!
\brief Function Biddy_Managed_AddVariableBelow adds a numbered variable.

### Description
    Biddy_Managed_AddVariableBelow uses Biddy_Managed_AddVariableByName to add
    numbered variable. Then, the order of the new variable is changed to become
    immediately below the given variable (below = next = bottommore in BDD)
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

  assert( v > 0 );
  assert( v < biddyVariableTable.num );

  if (v == 0) {
    fprintf(stderr,"WARNING: variable cannot be added below constant node 1\n");
    return biddyNull;
  }

  if (v >= biddyVariableTable.num ) {
    fprintf(stderr,"WARNING: variable cannot be added below the non-existing node\n");
    return biddyNull;
  }

  f = Biddy_Managed_AddVariableByName(MNG,NULL);
  x = biddyVariableTable.num - 1;

  if (v == biddyVariableTable.table[0].prev) return f; /* variable shifting is not needed */
  biddyVariableTable.table[0].prev = biddyVariableTable.table[x].prev;
  biddyVariableTable.table[biddyVariableTable.table[x].prev].next = 0;
  biddyVariableTable.table[biddyVariableTable.table[v].next].prev = x;
  biddyVariableTable.table[x].next = biddyVariableTable.table[v].next;
  biddyVariableTable.table[x].prev = v;
  biddyVariableTable.table[v].next = x;

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
\brief Function Biddy_Managed_AddVariableAbove adds a numbered variable.

### Description
    Biddy_Managed_AddVariableAbove uses Biddy_Managed_AddVariableByName to add
    numbered variable. Then, the order of the new variable is changed to become
    immediately above the given variable (above = previous = topmore in BDD)
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

  assert( v < biddyVariableTable.num );

  if (v >= biddyVariableTable.num ) {
    fprintf(stderr,"WARNING: variable cannot be added above the non-existing node\n");
    return biddyNull;
  }

  f = Biddy_Managed_AddVariableByName(MNG,NULL);
  x = biddyVariableTable.num - 1;

  if (v == 0) return f; /* variable shifting is not needed */
  biddyVariableTable.table[0].prev = biddyVariableTable.table[x].prev;
  biddyVariableTable.table[biddyVariableTable.table[x].prev].next = 0;
  if (biddyVariableTable.table[v].prev != biddyVariableTable.size) {
    biddyVariableTable.table[biddyVariableTable.table[v].prev].next = x;
  }
  biddyVariableTable.table[x].prev = biddyVariableTable.table[v].prev;
  biddyVariableTable.table[x].next = v;
  biddyVariableTable.table[v].prev = x;

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
\brief Function Biddy_Managed_TransferMark returns edge with inverted
       complement bit iff the second parameter is TRUE and normalization
       rules require this.

### Description
    It is better to use macro Biddy_InvCond.
    Parameter leftright is ignored.
### Side effects
### More info
    Macro Biddy_TransferMark() is defined for use with anonymous manager.
*******************************************************************************/

#ifdef __cplusplus
extern "C" {
#endif

Biddy_Edge
Biddy_Managed_TransferMark(Biddy_Manager MNG, Biddy_Edge f, Biddy_Boolean mark,
                           Biddy_Boolean leftright)
{
  if (!MNG) MNG = biddyAnonymousManager;

  if (Biddy_GetMark(f) != mark) Biddy_SetMark(f); else Biddy_ClearMark(f);
  return f;
}

#ifdef __cplusplus
}
#endif

/***************************************************************************//*!
\brief Function Biddy_Managed_IncTag returns edge with an incremented tag.

### Description
    This function is not used for OBDDs.
### Side effects
### More info
    Macro Biddy_IncTag() is defined for use with anonymous manager.
*******************************************************************************/

#ifdef __cplusplus
extern "C" {
#endif

Biddy_Edge
Biddy_Managed_IncTag(Biddy_Manager MNG, Biddy_Edge f)
{
  return f;
}

#ifdef __cplusplus
}
#endif


/***************************************************************************//*!
\brief Function Biddy_Managed_TaggedFoaNode finds or adds new node with the
       given variable and successors.

### Description
    If such node already exists, function returns it and does not create the
    new one. If pf = pt = NULL then new variable is created.
    This function should not be called directly to add new variables,
    you must use Biddy_Managed_FoaVariable and Biddy_Managed_AddVariableByName.
### Side effects
    Parameter ptag is ignored.
    Using Biddy_Managed_FoaNode you can create node with arbitrary ordering.
    It is much more safe to use Biddy_Managed_ITE. To enable efficient
    implementation of sifting the function started with the returned
    node is not refreshed!
### More info
    Macro Biddy_Managed_FoaNode(MNG,v,pf,pt,garbageAllowed) is defined for
    use without tags.
    Macros Biddy_TaggedFoaNode(v,pf,pt,tag,garbageAllowed) and
    Biddy_FoaNode(v,pf,pt,garbageAllowed) are defined for use with
    anonymous manager.
*******************************************************************************/

#ifdef __cplusplus
extern "C" {
#endif

Biddy_Edge
Biddy_Managed_TaggedFoaNode(Biddy_Manager MNG, Biddy_Variable v, Biddy_Edge pf,
                            Biddy_Edge pt, Biddy_Variable ptag,
                            Biddy_Boolean garbageAllowed)
{
  Biddy_Edge r;

  if (!MNG) MNG = biddyAnonymousManager;

  assert( (pf == NULL) || (BiddyIsOK(pf) == TRUE) );
  assert( (pt == NULL) || (BiddyIsOK(pt) == TRUE) );

  r = BiddyManagedFoaNode(MNG,v,pf,pt,garbageAllowed);
  BiddyRefresh(r); /* FoaNode returns an obsolete node! */

  return r;
}

#ifdef __cplusplus
}
#endif

Biddy_Edge
BiddyManagedTaggedFoaNode(Biddy_Manager MNG, Biddy_Variable v, Biddy_Edge pf,
                          Biddy_Edge pt, Biddy_Variable ptag,
                          Biddy_Boolean garbageAllowed)
{
  unsigned int hash;
  Biddy_Edge edge;
  BiddyNode *sup, *sup1;
  Biddy_Boolean complementedResult;
  unsigned int i;
  BiddyNode ** tmp;
  Biddy_Boolean newvariableRequired;

  static BiddyNode *newFreeNodes;

  assert( MNG != NULL );

#ifdef BIDDYEXTENDEDSTATS_YES
  biddyNodeTable.foa++;
#endif

  /* SPECIAL CASE */
  newvariableRequired = FALSE;
  if (!pf) {
    pf = biddyZero;
    pt = biddyTerminal;
    newvariableRequired = TRUE;
  } else {
    assert( Biddy_Managed_IsSmaller(MNG,v,BiddyV(pf)) );
    assert( Biddy_Managed_IsSmaller(MNG,v,BiddyV(pt)) );
  }

  /* TERMINAL CASE */
  if (pf == pt) return pf;

  /* NORMALIZATION OF COMPLEMENTED EDGES - 'THEN' SHOULD NOT BE COMPLEMENTED ! */
  complementedResult = FALSE;
  if (Biddy_GetMark(pt)) {
    Biddy_InvertMark(pf);
    Biddy_ClearMark(pt);
    complementedResult = TRUE;
  } else {
    complementedResult = FALSE;
  }

  /* TERMINAL CASE - VARIABLES ARE NOT HASHED IN THE USUAL WAY */
  if ((pf == biddyZero) && (pt == biddyTerminal) && !newvariableRequired) {
    if (complementedResult) {
      return Biddy_Inv(biddyVariableTable.table[v].variable);
    } else {
      return biddyVariableTable.table[v].variable;
    }
  }

  /* FIND OR ADD - THERE IS A HASH TABLE WITH CHAINING */
  /* THE CREATED NODE WILL ALWAYS BE THE FIRST NODE IN THE CHAIN */
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
    if (garbageAllowed && !biddyFreeNodes) {

      Biddy_Managed_AutoGC(MNG);

      /* the table may be resized, thus the element must be rehashed */
      hash = nodeTableHash(v,pf,pt,biddyNodeTable.size);
      sup = biddyNodeTable.table[hash];
      sup1 = findNodeTable(MNG,v,pf,pt,&sup);

    }

    /* IF NO FREE NODES ARE FOUND THEN CREATE NEW BLOCK OF NODES */
    if (!biddyFreeNodes) {

      /* the size of a memory block is increased until the limit is reached */
      if (biddyNodeTable.blocksize < biddyNodeTable.limitblocksize)
      {
        biddyNodeTable.blocksize = biddyNodeTable.resizeratio * biddyNodeTable.size;
        if (biddyNodeTable.blocksize < biddyNodeTable.initblocksize)
          biddyNodeTable.blocksize = biddyNodeTable.initblocksize;
        if (biddyNodeTable.blocksize > biddyNodeTable.limitblocksize)
          biddyNodeTable.blocksize = biddyNodeTable.limitblocksize;
      }

      if (!(newFreeNodes = (BiddyNode *)
            malloc((biddyNodeTable.blocksize) * sizeof(BiddyNode))))
      {
        fprintf(stderr,"\nBIDDY (BiddyManagedTaggedFoaNode): Out of memory error!\n");
        fprintf(stderr,"Currently, there exist %d nodes.\n",biddyNodeTable.num);
        exit(1);
      }

      biddyNodeTable.blocknumber++;
      if (!(tmp = (BiddyNode **) realloc(biddyNodeTable.blocktable,
            biddyNodeTable.blocknumber * sizeof(BiddyNode *))))
      {
        fprintf(stderr,"\nBIDDY (BiddyManagedTaggedFoaNode): Out of memory error!\n");
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
    sup->expiry = 1; /* new node is not refreshed! */
    sup->select = 0;
    sup->v = v;

    /* add new node to Node table */
    /* variables are not stored in Node table */
    if (!newvariableRequired) {
      addNodeTable(MNG,hash,sup,sup1);
    }

    /* add new node to the list */
    /* all nodes are added to list, also variables which are not added to Node table */
    /* lastNode->list IS NOT FIXED! */
    /* YOU MUST NEVER ASSUME THAT lastNode->list = NULL */
    biddyVariableTable.table[v].lastNode->list = (void *) sup;
    biddyVariableTable.table[v].lastNode = sup;

  }

  /* SINCE BIDDY V1.1 */
  edge = sup;

  if (complementedResult) {
    Biddy_SetMark(edge);
  }

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

/***************************************************************************//*!
\brief Function Biddy_Managed_Not calculates Boolean function NOT.

### Description
    It is better to use macro Biddy_Inv.
### Side effects
### More info
    Macro Biddy_Not() is defined for use with anonymous manager.
*******************************************************************************/

#ifdef __cplusplus
extern "C" {
#endif

Biddy_Edge
Biddy_Managed_Not(Biddy_Manager MNG, Biddy_Edge f)
{
  assert( f != NULL );

  if (!MNG) MNG = biddyAnonymousManager;
  ZF_LOGI("Biddy_Not");

  assert( BiddyIsOK(f) == TRUE );

  Biddy_InvertMark(f);
  /* BiddyRefresh(f); */

  return f;
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
BiddyManagedITE(const Biddy_Manager MNG, Biddy_Edge f, Biddy_Edge g, Biddy_Edge h)
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
    /* BiddyRefresh(r); */
  } else if (f == biddyZero) {
    terminal = TRUE;
    r = h;
    /* BiddyRefresh(r); */
  } else {
    if (Biddy_IsEqvPointer(g,h)) {
      if (g == h) {
        terminal = TRUE;
        r = g;
        /* BiddyRefresh(r); */
      } else {
        if (Biddy_IsConstant(g)) {
          terminal = TRUE;
          r = f;
          negation = Biddy_GetMark(g);
          /* BiddyRefresh(r); */
        }
      }
    }
  }

  /* LOOKING FOR SIMPLE CASE */

  if (!terminal && Biddy_IsConstant(g)) {
    terminal = TRUE;
    if (Biddy_GetMark(g)) {
      r = BiddyManagedAnd(MNG,Biddy_Inv(f),h);
    } else {
      negation = TRUE;
      r = BiddyManagedAnd(MNG,Biddy_Inv(f),Biddy_Inv(h));
    }
  }

  if (!terminal && Biddy_IsConstant(h)) {
    terminal = TRUE;
    if (Biddy_GetMark(h)) {
      r = BiddyManagedAnd(MNG,f,g);
    } else {
      negation = TRUE;
      r = BiddyManagedAnd(MNG,f,Biddy_Inv(g));
    }
  }

  /* WE KNOW, THAT G AND H ARE NOT EQUAL, BUT THEY CAN BE INVERTED! */
  if (!terminal && Biddy_IsEqvPointer(g,h)) {
    terminal = TRUE;
    r = BiddyManagedXor(MNG,f,h);
  }

  /* IF NOT TERMINAL CASE AND NOT SIMPLE CASE */
  if (!terminal) {

    /* NORMALIZATION - STEP 2 */

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
    if (!findOp3Cache(MNG,biddyOPCache,f,g,h,&r,&cindex))
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
          Hv = Biddy_Inv(BiddyT(h));
          Hneg_v = Biddy_Inv(BiddyE(h));
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

      r = BiddyManagedFoaNode(MNG,v,E,T,TRUE);
      BiddyRefresh(r); /* FoaNode will not refresh function! */

      /* CACHE EVERYTHING */
      /**/
      addOp3Cache(MNG,biddyOPCache,f,g,h,r,cindex);
      /**/

      /* THIS DOES NOT PRODUCE BETTER RESULTS */
      /*
      if (!simple) addOp3Cache(MNG,biddyOPCache,f,g,h,r);
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
      BiddyRefresh(r);

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
    For combination sets, this function coincides with Intersection.
### Side Effects
    Used by ITE.
### More Info
    Macro Biddy_And(f,g) is defined for use with anonymous manager.
    Macros Biddy_Managed_Intersect(MNG,f,g) and Biddy_Intersect(f,g) are
    defined for manipulation of combination sets.
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

  biddyNodeTable.funandor++;

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
  biddyNodeTable.andorrecursive++;
#endif

  /* NORMALIZATION - STEP 1 */

  if (f == g) {
    /* BiddyRefresh(f); */
    return f;
  }
  if (f == Biddy_Inv(g)) {
    return biddyZero;
  }

  /* LOOKING FOR TERMINAL CASE */

  if (f == biddyOne) {
    /* BiddyRefresh(g); */
    return g;
  } else if (f == biddyZero) {
    return biddyZero;
  } else if (g == biddyOne) {
    /* BiddyRefresh(f); */
    return f;
  } else if (g == biddyZero) {
    return biddyZero;
  }

  /* THIS IS NOT A TERMINAL CASE */

  negation = FALSE;

  /* NORMALIZATION - STEP 2 - (uintptr_t) f) SHOULD BE GREATER THAN ((uintptr_t) g) */
  /* FF, GG, and HH ARE USED FOR CACHE LOOKUP, ONLY */

  if (((uintptr_t) f) > ((uintptr_t) g)) {
    if (Biddy_GetMark(f)) {
      negation = TRUE;
      FF = Biddy_Inv(f);
      GG = biddyOne; /* Biddy_Inv(h) */
      HH = Biddy_Inv(g);
    } else {
      if (Biddy_GetMark(g)) {
        negation = TRUE;
        FF = f;
        GG = Biddy_Inv(g);
        HH = biddyOne; /* Biddy_Inv(h) */
      } else {
        FF = f;
        GG = g;
        HH = biddyZero; /* h */
      }
    }
  } else {
    if (Biddy_GetMark(g)) {
      negation = TRUE;
      FF = Biddy_Inv(g);
      GG = biddyOne; /* Biddy_Inv(h) */
      HH = Biddy_Inv(f);
    } else {
      if (Biddy_GetMark(f)) {
        negation = TRUE;
        FF = g;
        GG = Biddy_Inv(f);
        HH = biddyOne; /* Biddy_Inv(h) */
      } else {
        FF = g;
        GG = f;
        HH = biddyZero; /* h */
      }
    }
  }

  /* IF RESULT IS NOT IN THE CACHE TABLE... */
  if (!findOp3Cache(MNG,biddyOPCache,FF,GG,HH,&r,&cindex))
  {

    /* LOOKING FOR THE SMALLES TOP VARIABLE */
    topF = BiddyV(f);
    topG = BiddyV(g);

    /* CONSTANT NODE MUST HAVE MAX ORDER */
    v = Biddy_Managed_IsSmaller(MNG,topF,topG) ? topF : topG;

    /* DETERMINING PARAMETERS FOR RECURSIVE CALLS */
    if (topF == v) {
      if (Biddy_GetMark(f)) {
        Fv = Biddy_Inv(BiddyT(f));
        Fneg_v = Biddy_Inv(BiddyE(f));
      } else {
        Fv = BiddyT(f);
        Fneg_v = BiddyE(f);
      }
    } else {
      Fneg_v = Fv = f;
    }

    if (topG == v) {
      if (Biddy_GetMark(g)) {
        Gv = Biddy_Inv(BiddyT(g));
        Gneg_v = Biddy_Inv(BiddyE(g));
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
    } else if (Gv == biddyOne) {
      T = Fv;
    } else if (Gv == biddyZero) {
      T = biddyZero;
    } else  {
      T = BiddyManagedAnd(MNG,Fv,Gv);
    }

    if (Fneg_v == biddyOne) {
      E = Gneg_v;
    } else if (Fneg_v == biddyZero) {
      E = biddyZero;
    } else if (Gneg_v == biddyOne) {
      E = Fneg_v;
    } else if (Gneg_v == biddyZero) {
      E = biddyZero;
    } else  {
      E = BiddyManagedAnd(MNG,Fneg_v,Gneg_v);
    }

    r = BiddyManagedFoaNode(MNG,v,E,T,TRUE);
    BiddyRefresh(r); /* FoaNode will not refresh function! */

    if (negation) {
      addOp3Cache(MNG,biddyOPCache,FF,GG,HH,Biddy_Inv(r),cindex);
    } else {
      addOp3Cache(MNG,biddyOPCache,FF,GG,HH,r,cindex);
    }

  } else {

    if (negation) {
      Biddy_InvertMark(r);
    }

    /* IF THE RESULT IS FROM CACHE TABLE, REFRESH IT! */
    BiddyRefresh(r);

  }

  return r;

}

/***************************************************************************//*!
\brief Function Biddy_Managed_Or calculates Boolean function OR
       (disjunction).

### Description
    For combination sets, this function coincides with Union.
### Side Effects
### More Info
    Macro Biddy_Or(f,g) is defined for use with anonymous manager.
    Macros Biddy_Managed_Union(MNG,f,g) and Biddy_Union(f,g) are defined
    for manipulation of combination sets.
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

  biddyNodeTable.funandor++;

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

  /* TESTING, ONLY */
  /*
  return BiddyManagedXor(MNG,BiddyManagedXor(MNG,f,g),BiddyManagedAnd(MNG,f,g));
  */

  /* in the statistics, only a single andor call is counted */
  return Biddy_Inv(BiddyManagedAnd(MNG,Biddy_Inv(f),Biddy_Inv(g)));
}

/***************************************************************************//*!
\brief Function Biddy_Managed_Nand calculates Boolean function NAND
       (Sheffer).

### Description
### Side Effects
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

  return Biddy_Inv(BiddyManagedAnd(MNG,f,g));
}

/***************************************************************************//*!
\brief Function Biddy_Managed_Nor calculates Boolean function NOR
       (Peirce).

### Description
### Side Effects
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

  return BiddyManagedAnd(MNG,Biddy_Inv(f),Biddy_Inv(g));
}

/***************************************************************************//*!
\brief Function Biddy_Managed_Xor calculates Boolean function XOR.

### Description
### Side Effects
    Used by ITE.
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

  biddyNodeTable.funxor++;
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
  biddyNodeTable.xorrecursive++;
#endif

  /* NORMALIZATION - STEP 1 */

  if (f == g) {
    return biddyZero;
  }
  if (f == Biddy_Inv(g)) {
    return biddyOne;
  }

  /* LOOKING FOR TERMINAL CASE */

  if (f == biddyOne) {
    r = Biddy_Inv(g);
    /* BiddyRefresh(r); */
    return r;
  } else if (f == biddyZero) {
    /* BiddyRefresh(g); */
    return g;
  } else if (g == biddyOne) {
    r = Biddy_Inv(f);
    /* BiddyRefresh(r); */
    return r;
  } else if (g == biddyZero) {
    /* BiddyRefresh(f); */
    return f;
  }

  /* THIS IS NOT A TERMINAL CASE */

  negation = FALSE;

  /* NORMALIZATION - STEP 2  - (uintptr_t) f) SHOULD BE GREATER THAN ((uintptr_t) g) */
  /* FF, GG, and HH ARE USED FOR CACHE LOOKUP, ONLY */

  if (((uintptr_t) f) > ((uintptr_t) g)) {
    if (Biddy_GetMark(f)) {
      if (Biddy_GetMark(g)) {
        FF = Biddy_Inv(f);
        GG = Biddy_Inv(g); /* h */
        HH = g;
      } else {
        negation = TRUE;
        FF = Biddy_Inv(f);
        GG = g;
        HH = Biddy_Inv(g); /* h */
      }
    } else {
      if (Biddy_GetMark(g)) {
        negation = TRUE;
        FF = f;
        GG = Biddy_Inv(g); /* h */
        HH = g;
      } else {
        FF = f;
        GG = g;
        HH = Biddy_Inv(g); /* h */
      }
    }
  } else {
    if (Biddy_GetMark(g)) {
      if (Biddy_GetMark(f)) {
        FF = Biddy_Inv(g);
        GG = Biddy_Inv(f); /* h */
        HH = f;
      } else {
        negation = TRUE;
        FF = Biddy_Inv(g);
        GG = f;
        HH = Biddy_Inv(f); /* h */
      }
    } else {
      if (Biddy_GetMark(f)) {
        negation = TRUE;
        FF = g;
        GG = Biddy_Inv(f); /* h */
        HH = f;
      } else {
        FF = g;
        GG = f;
        HH = Biddy_Inv(f); /* h */
      }
    }
  }

  /* IF RESULT IS NOT IN THE CACHE TABLE... */
  if (!findOp3Cache(MNG,biddyOPCache,FF,GG,HH,&r,&cindex))
  {

    /* LOOKING FOR THE SMALLES TOP VARIABLE */
    topF = BiddyV(f);
    topG = BiddyV(g);

    /* CONSTANT NODE MUST HAVE MAX ORDER */
    v = Biddy_Managed_IsSmaller(MNG,topF,topG) ? topF : topG;

    /* DETERMINING PARAMETERS FOR RECURSIVE CALLS */
    if (topF == v) {
      if (Biddy_GetMark(f)) {
        Fv = Biddy_Inv(BiddyT(f));
        Fneg_v = Biddy_Inv(BiddyE(f));
      } else {
        Fv = BiddyT(f);
        Fneg_v = BiddyE(f);
      }
    } else {
      Fneg_v = Fv = f;
    }

    if (topG == v) {
      if (Biddy_GetMark(g)) {
        Gv = Biddy_Inv(BiddyT(g));
        Gneg_v = Biddy_Inv(BiddyE(g));
      } else {
        Gv = BiddyT(g);
        Gneg_v = BiddyE(g);
      }
    } else {
       Gneg_v = Gv = g;
    }

    /* RECURSIVE CALLS */
    if (Fv == biddyOne) {
      T = Biddy_Inv(Gv);
    } else if (Fv == biddyZero) {
      T = Gv;
    } else if (Gv == biddyOne) {
      T = Biddy_Inv(Fv);
    } else if (Gv == biddyZero) {
      T = Fv;
    } else  {
      T = BiddyManagedXor(MNG,Fv,Gv);
    }

    if (Fneg_v == biddyOne) {
      E = Biddy_Inv(Gneg_v);
    } else if (Fneg_v == biddyZero) {
      E = Gneg_v;
    } else if (Gneg_v == biddyOne) {
      E = Biddy_Inv(Fneg_v);
    } else if (Gneg_v == biddyZero) {
      E = Fneg_v;
    } else  {
      E = BiddyManagedXor(MNG,Fneg_v,Gneg_v);
    }

    r = BiddyManagedFoaNode(MNG,v,E,T,TRUE);
    BiddyRefresh(r); /* FoaNode will not refresh function! */

    if (negation) {
      addOp3Cache(MNG,biddyOPCache,FF,GG,HH,Biddy_Inv(r),cindex);
    } else {
      addOp3Cache(MNG,biddyOPCache,FF,GG,HH,r,cindex);
    }

  } else {

    if (negation) {
      Biddy_InvertMark(r);
    }

    /* IF THE RESULT IS FROM CACHE TABLE, REFRESH IT! */
    BiddyRefresh(r);

  }

  return r;
}

/***************************************************************************//*!
\brief Function Biddy_Managed_Xnor calculates Boolean function XNOR.

### Description
### Side Effects
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

  return Biddy_Inv(BiddyManagedXor(MNG,f,g));
}

/***************************************************************************//*!
\brief Function Biddy_Managed_Leq calculates Boolean implication.

### Description
### Side Effects
### More Info
    Macro Biddy_Leq(f,g) is defined for use with anonymous manager.
*******************************************************************************/

#ifdef __cplusplus
extern "C" {
#endif

Biddy_Edge
Biddy_Managed_Leq(Biddy_Manager MNG, Biddy_Edge f, Biddy_Edge g)
{
  Biddy_Edge r;

  assert( f != NULL );
  assert( g != NULL );

  if (!MNG) MNG = biddyAnonymousManager;
  ZF_LOGI("Biddy_Leq");

  assert( BiddyIsOK(f) == TRUE );
  assert( BiddyIsOK(g) == TRUE );

  r = BiddyManagedLeq(MNG,f,g);

  return r;
}

#ifdef __cplusplus
}
#endif

Biddy_Edge
BiddyManagedLeq(const Biddy_Manager MNG, const Biddy_Edge f, const Biddy_Edge g)
{
  assert( MNG != NULL );
  assert( f != NULL );
  assert( g != NULL );

  return Biddy_Inv(BiddyManagedAnd(MNG,f,Biddy_Inv(g)));
}

/***************************************************************************//*!
\brief Function Biddy_Managed_Gt calculates the negation of Boolean implication.

### Description
    For combination sets, this function coincides with Diff.
### Side Effects
### More Info
    Macro Biddy_Gt(f,g) is defined for use with anonymous manager.
    Macros Biddy_Managed_Diff(MNG,f,g) and Biddy_Diff(f,g) are defined for
    manipulation of combination sets.
*******************************************************************************/

#ifdef __cplusplus
extern "C" {
#endif

Biddy_Edge
Biddy_Managed_Gt(Biddy_Manager MNG, Biddy_Edge f, Biddy_Edge g)
{
  Biddy_Edge r;

  assert( f != NULL );
  assert( g != NULL );

  if (!MNG) MNG = biddyAnonymousManager;
  ZF_LOGI("Biddy_Gt");

  assert( BiddyIsOK(f) == TRUE );
  assert( BiddyIsOK(g) == TRUE );

  r = BiddyManagedGt(MNG,f,g);

  return r;
}

#ifdef __cplusplus
}
#endif

Biddy_Edge
BiddyManagedGt(const Biddy_Manager MNG, const Biddy_Edge f, const Biddy_Edge g)
{
  assert( MNG != NULL );
  assert( f != NULL );
  assert( g != NULL );

  return BiddyManagedAnd(MNG,f,Biddy_Inv(g));
}

/***************************************************************************//*!
\brief Function Biddy_Managed_IsLeq returns TRUE iff function f is included in
       function g.

### Description
### Side Effects
    Implemented by calculating full implication which is less efficient as
    implementation in CUDD.
### More Info
    Macro Biddy_IsLeq(f,g) is defined for use with anonymous manager.
*******************************************************************************/

#ifdef __cplusplus
extern "C" {
#endif

Biddy_Boolean
Biddy_Managed_IsLeq(Biddy_Manager MNG, Biddy_Edge f, Biddy_Edge g)
{
  Biddy_Edge r;

  if (!MNG) MNG = biddyAnonymousManager;
  ZF_LOGI("Biddy_IsLeq");

  r = BiddyManagedITE(MNG,f,g,biddyOne);
  return (r == biddyOne);
}

#ifdef __cplusplus
}
#endif

/***************************************************************************//*!
\brief Biddy_Managed_SubIntersect calculates a function included in the
       intersection of f and g.

### Description
    If the result is not constant 0 then it is a witness that the intersection
    is not empty. The result should be calculated with as few new nodes as
    possible, and the result may not be the same as conjunction between
    functions! If the only result of interest is whether f and g intersect,
    Biddy_IsLeq should be used (which returns TRUE iff f*g' == 0).
### Side Effects
    Implemented by calculating full conjunction which is less efficient as
    implementation in CUDD.
### More Info
    Macro Biddy_SubIntersect(f,g) is defined for use with anonymous manager.
*******************************************************************************/

#ifdef __cplusplus
extern "C" {
#endif

Biddy_Edge
Biddy_Managed_SubIntersect(Biddy_Manager MNG, Biddy_Edge f, Biddy_Edge g)
{
  Biddy_Edge r;

  if (!MNG) MNG = biddyAnonymousManager;
  ZF_LOGI("Biddy_SubIntersect");

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
    Original BDD is not changed.
    This is not Coudert and Madre's restrict function (use Biddy_Simplify if
    you need that one).
### Side effects
    Recursive calls use optimization: F(a=x) == NOT((NOT F)(a=x)).
### More info
    Macro Biddy_Restrict(f,v,value) is defined for use with anonymous manager.
*******************************************************************************/

#ifdef __cplusplus
extern "C" {
#endif

Biddy_Edge
Biddy_Managed_Restrict(Biddy_Manager MNG, Biddy_Edge f, Biddy_Variable v,
                       Biddy_Boolean value)
{
  Biddy_Edge r;

  assert( f != NULL );

  if (!MNG) MNG = biddyAnonymousManager;
  ZF_LOGI("Biddy_Restrict");

  assert( BiddyIsOK(f) == TRUE );

  r = BiddyManagedRestrict(MNG,f,v,value);

  return r;
}

#ifdef __cplusplus
}
#endif

Biddy_Edge
BiddyManagedRestrict(Biddy_Manager MNG, Biddy_Edge f, Biddy_Variable v,
                     Biddy_Boolean value)
{
  Biddy_Edge e, t, r;
  Biddy_Edge FF, GG, HH;
  Biddy_Variable fv;
  unsigned int cindex;

  assert( MNG != NULL );
  assert( f != NULL );

  if (Biddy_IsConstant(f)) return f;

  FF = f;
  GG = value?Biddy_Managed_GetConstantZero(MNG):Biddy_Managed_GetConstantOne(MNG);
  HH = biddyVariableTable.table[v].variable;

  /* IF RESULT IS NOT IN THE CACHE TABLE... */
  if (!findOp3Cache(MNG,biddyRCCache,FF,GG,HH,&r,&cindex))
  {

    if ((fv=BiddyV(f)) == v) {
      if (value) {
        r = Biddy_InvCond(BiddyT(f),Biddy_GetMark(f));
        BiddyRefresh(r);
      } else {
        r = Biddy_InvCond(BiddyE(f),Biddy_GetMark(f));
        BiddyRefresh(r);
      }
    }
    else if (Biddy_Managed_IsSmaller(MNG,v,fv)) {
      r = f;
      /* BiddyRefresh(r); */
    }
    else {
      e = BiddyManagedRestrict(MNG,BiddyE(f),v,value);
      t = BiddyManagedRestrict(MNG,BiddyT(f),v,value);
      r = Biddy_InvCond(BiddyManagedFoaNode(MNG,fv,e,t,TRUE),Biddy_GetMark(f));
      BiddyRefresh(r); /* FoaNode will not refresh function! */
      addOp3Cache(MNG,biddyRCCache,FF,GG,HH,r,cindex);
    }

  } else {

    /* IF THE RESULT IS FROM CACHE TABLE, REFRESH IT! */
    BiddyRefresh(r);

  }

  return r;
}

/***************************************************************************//*!
\brief Function Biddy_Managed_Compose calculates a composition of two Boolean
       functions.

### Description
    Original BDDs are not changed.
### Side effects
    It uses optimization: F(a=G) == NOT((NOT F)(a=G)).
### More info
    Macro Biddy_Compose(f,g,v) is defined for use with anonymous manager.
*******************************************************************************/

#ifdef __cplusplus
extern "C" {
#endif

Biddy_Edge
Biddy_Managed_Compose(Biddy_Manager MNG, Biddy_Edge f, Biddy_Edge g,
                      Biddy_Variable v)
{
  Biddy_Edge r;

  assert( f != NULL );
  assert( g != NULL );

  if (!MNG) MNG = biddyAnonymousManager;
  ZF_LOGI("Biddy_Compose");

  assert( BiddyIsOK(f) == TRUE );
  assert( BiddyIsOK(g) == TRUE );

  r = BiddyManagedCompose(MNG,f,g,v);

  return r;
}

#ifdef __cplusplus
}
#endif

Biddy_Edge
BiddyManagedCompose(Biddy_Manager MNG, Biddy_Edge f, Biddy_Edge g,
                    Biddy_Variable v)
{
  Biddy_Edge e, t, r;
  Biddy_Edge FF, GG, HH;
  Biddy_Variable fv;
  unsigned int cindex;

  assert( MNG != NULL );
  assert( f != NULL );
  assert( g != NULL );

  if (Biddy_IsConstant(f)) return f;

  FF = f;
  GG = g;
  HH = biddyVariableTable.table[v].variable;

  /* IF RESULT IS NOT IN THE CACHE TABLE... */
  if (!findOp3Cache(MNG,biddyRCCache,FF,GG,HH,&r,&cindex))
  {

    if ((fv=BiddyV(f)) == v) {
      r = Biddy_InvCond(BiddyManagedITE(MNG,g,BiddyT(f),BiddyE(f)),Biddy_GetMark(f));
    }
    else if (Biddy_Managed_IsSmaller(MNG,v,fv)) {
      r = f;
      /* BiddyRefresh(r); */
    }
    else {
      e = BiddyManagedCompose(MNG,BiddyE(f),g,v);
      t = BiddyManagedCompose(MNG,BiddyT(f),g,v);
      if (Biddy_Managed_IsSmaller(MNG,fv,Biddy_GetTopVariable(e)) && Biddy_Managed_IsSmaller(MNG,fv,Biddy_GetTopVariable(t))) {
        r = Biddy_InvCond(BiddyManagedFoaNode(MNG,fv,e,t,TRUE),Biddy_GetMark(f));
        BiddyRefresh(r); /* FoaNode will not refresh function! */
      } else {
        r = Biddy_InvCond(BiddyManagedITE(MNG,biddyVariableTable.table[fv].variable,t,e),Biddy_GetMark(f));
      }
      addOp3Cache(MNG,biddyRCCache,FF,GG,HH,r,cindex);
    }

  } else {

    /* IF THE RESULT IS FROM CACHE TABLE, REFRESH IT! */
    BiddyRefresh(r);

  }

  return r;
}

/***************************************************************************//*!
\brief Function Biddy_Managed_E calculates an existential quantification of
       Boolean function.

### Description
    Original BDD is not changed.
### Side effects
    Be careful: ExA F != NOT(ExA (NOT F)).
    Counterexample: Exb (AND (NOT a) b c).
### More info
    Macro Biddy_E(f,v) is defined for use with anonymous manager.
*******************************************************************************/

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
                       Biddy_InvCond(BiddyT(f),Biddy_GetMark(f)),
                       biddyOne,
                       Biddy_InvCond(BiddyE(f),Biddy_GetMark(f)));
    }

    if (Biddy_Managed_IsSmaller(MNG,v,fv)) {
      /* BiddyRefresh(f); */
      return f;
    }

    e = BiddyManagedE(MNG,Biddy_InvCond(BiddyE(f),Biddy_GetMark(f)),v);
    t = BiddyManagedE(MNG,Biddy_InvCond(BiddyT(f),Biddy_GetMark(f)),v);
    r = BiddyManagedITE(MNG,biddyVariableTable.table[fv].variable,t,e);

    /* CACHE EVERYTHING */
    if (((uintptr_t) f) > ((uintptr_t) biddyOne)) {
      addOp3Cache(MNG,biddyEACache,biddyOne,f,h,r,cindex);
    } else {
      addOp3Cache(MNG,biddyEACache,f,biddyOne,h,r,cindex);
    }

  } else {

    /* IF THE RESULT IS FROM CACHE TABLE, REFRESH IT! */
    BiddyRefresh(r);

  }

  return r;
}

/***************************************************************************//*!
\brief Function Biddy_Managed_A calculates an universal quantification of
       Boolean function.

### Description
    Original BDD is not changed.
### Side effects
    Be careful: AxA F != NOT(AxA (NOT F)).
    Counterexample: Axb (AND (NOT a) b c).
### More info
    Macro Biddy_A(f,v) is defined for use with anonymous manager.
*******************************************************************************/

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
  r = Biddy_Inv(BiddyManagedE(MNG,Biddy_Inv(f),v));

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
    Original BDD is not changed.
### Side effects
### More info
    Macro Biddy_ExistAbstract(f,cube) is defined for use with anonymous manager.
*******************************************************************************/

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
    if (Biddy_IsConstant(cube)) {
      /* BiddyRefresh(f); */
      return f;
    }

    if (cv == fv) {

      e = BiddyManagedExistAbstract(MNG,
            Biddy_InvCond(BiddyE(f),Biddy_GetMark(f)),BiddyT(cube));
      t = BiddyManagedExistAbstract(MNG,
            Biddy_InvCond(BiddyT(f),Biddy_GetMark(f)),BiddyT(cube));
      r = BiddyManagedITE(MNG,t,biddyOne,e);

    } else {

      e = BiddyManagedExistAbstract(MNG,
            Biddy_InvCond(BiddyE(f),Biddy_GetMark(f)),cube);
      t = BiddyManagedExistAbstract(MNG,
            Biddy_InvCond(BiddyT(f),Biddy_GetMark(f)),cube);
      r = BiddyManagedITE(MNG,biddyVariableTable.table[fv].variable,t,e);

    }

    /* CACHE EVERYTHING */
    if (((uintptr_t) f) > ((uintptr_t) biddyOne)) {
      addOp3Cache(MNG,biddyEACache,biddyOne,f,cube,r,cindex);
    } else {
      addOp3Cache(MNG,biddyEACache,f,biddyOne,cube,r,cindex);
    }

  } else {

    /* IF THE RESULT IS FROM CACHE TABLE, REFRESH IT! */
    BiddyRefresh(r);

  }

  return r;
}

/***************************************************************************//*!
\brief Function Biddy_Managed_UnivAbstract universally abstracts all the
       variables in cube from f.

### Description
    Original BDD is not changed.
### Side effects
### More info
    Macro Biddy_UnivAbstract(f,cube) is defined for use with anonymous manager.
*******************************************************************************/

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
  r = Biddy_Inv(BiddyManagedExistAbstract(MNG,Biddy_Inv(f),cube));

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
      f0 = Biddy_InvCond(BiddyE(f),Biddy_GetMark(f));
      f1 = Biddy_InvCond(BiddyT(f),Biddy_GetMark(f));
      g0 = g;
      g1 = g;
      minv = fv;
    } else if (Biddy_Managed_IsSmaller(MNG,gv,fv)) {
      f0 = f;
      f1 = f;
      g0 = Biddy_InvCond(BiddyE(g),Biddy_GetMark(g));
      g1 = Biddy_InvCond(BiddyT(g),Biddy_GetMark(g));
      minv = gv;
    } else {
      f0 = Biddy_InvCond(BiddyE(f),Biddy_GetMark(f));
      f1 = Biddy_InvCond(BiddyT(f),Biddy_GetMark(f));
      g0 = Biddy_InvCond(BiddyE(g),Biddy_GetMark(g));
      g1 = Biddy_InvCond(BiddyT(g),Biddy_GetMark(g));
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
      if ((e == f1) || (e == g1)) {
        /* BiddyRefresh(e); */
        return e;
      }
      if (e == Biddy_Inv(f1)) f1 = biddyOne;
      if (e == Biddy_Inv(g1)) g1 = biddyOne;
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

    /* IF THE RESULT IS FROM CACHE TABLE, REFRESH IT! */
    BiddyRefresh(r);

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
  if (c == biddyOne) {
    /* BiddyRefresh(f); */
    return f;
  }
  if (Biddy_IsConstant(f)) return f;

  fv = BiddyV(f);
  cv = BiddyV(c);

  if (Biddy_Managed_IsSmaller(MNG,fv,cv)) {
    f0 = Biddy_InvCond(BiddyE(f),Biddy_GetMark(f));
    f1 = Biddy_InvCond(BiddyT(f),Biddy_GetMark(f));
    c0 = c;
    c1 = c;
    minv = fv;
  } else if (Biddy_Managed_IsSmaller(MNG,cv,fv)) {
    f0 = f;
    f1 = f;
    c0 = Biddy_InvCond(BiddyE(c),Biddy_GetMark(c));
    c1 = Biddy_InvCond(BiddyT(c),Biddy_GetMark(c));
    minv = cv;
  } else {
    f0 = Biddy_InvCond(BiddyE(f),Biddy_GetMark(f));
    f1 = Biddy_InvCond(BiddyT(f),Biddy_GetMark(f));
    c0 = Biddy_InvCond(BiddyE(c),Biddy_GetMark(c));
    c1 = Biddy_InvCond(BiddyT(c),Biddy_GetMark(c));
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
    restricting it to the domain covered by function c. No checks are done
    to see if the result is actually smaller than the input.
### Side effects
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
  if (c == biddyOne) {
    /* BiddyRefresh(f); */
    return f;
  }
  if (Biddy_IsConstant(f)) return f;

  /* THIS IS FROM 20-CS-626-001 */
  /* http://gauss.ececs.uc.edu/Courses/c626/lectures/BDD/bdd-desc.pdf */
  if (f == c) return biddyOne;
  if (f == Biddy_Inv(c)) return biddyZero;

  fv = BiddyV(f);
  cv = BiddyV(c);

  /* SPECIAL CASE: f/~a == f/a */
  if (Biddy_Managed_IsSmaller(MNG,cv,fv)) {
    return BiddyManagedSimplify(MNG,f,BiddyManagedE(MNG,c,cv));
  }

  if (Biddy_Managed_IsSmaller(MNG,fv,cv)) {
    f0 = Biddy_InvCond(BiddyE(f),Biddy_GetMark(f));
    f1 = Biddy_InvCond(BiddyT(f),Biddy_GetMark(f));
    c0 = c;
    c1 = c;
    minv = fv;
  } else {
    f0 = Biddy_InvCond(BiddyE(f),Biddy_GetMark(f));
    f1 = Biddy_InvCond(BiddyT(f),Biddy_GetMark(f));
    c0 = Biddy_InvCond(BiddyE(c),Biddy_GetMark(c));
    c1 = Biddy_InvCond(BiddyT(c),Biddy_GetMark(c));
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
  Biddy_Managed_SelectNode(MNG,biddyTerminal); /* needed for BiddyNodeVarNumber */
  BiddyNodeVarNumber(MNG,f,&n); /* VARIABLES ARE MARKED */
  Biddy_Managed_DeselectAll(MNG);

  s = biddyOne;
  for (v=1;v<biddyVariableTable.num;v++) {
    if (biddyVariableTable.table[v].selected == TRUE) {
      s = BiddyManagedAnd(MNG,s,biddyVariableTable.table[v].variable);
      biddyVariableTable.table[v].selected = FALSE;
    }
  }

  return s;
}

/***************************************************************************//*!
\brief Function Biddy_Managed_Replace calculates BDD with one or more variables
       replaced.

### Description
    Original BDD is not changed.
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

  e = BiddyManagedReplace(MNG,Biddy_InvCond(BiddyE(f),Biddy_GetMark(f)));
  t = BiddyManagedReplace(MNG,Biddy_InvCond(BiddyT(f),Biddy_GetMark(f)));
  r = BiddyManagedITE(MNG,biddyVariableTable.table[fv].variable,t,e);

  return r;
}

/***************************************************************************//*!
\brief Function Biddy_Managed_Change change the form of the given variable
       (positive literal becomes negative and vice versa).

### Description
### Side effects
### More info
    Macro Biddy_Change() is defined for use with anonymous manager.
*******************************************************************************/

#ifdef __cplusplus
extern "C" {
#endif

Biddy_Edge
Biddy_Managed_Change(Biddy_Manager MNG, Biddy_Edge f, Biddy_Variable v)
{
  Biddy_Edge r;

  assert( f != NULL );

  if (!MNG) MNG = biddyAnonymousManager;
  ZF_LOGI("Biddy_Change");

  assert( BiddyIsOK(f) == TRUE );

  r = BiddyManagedChange(MNG,f,v);

  return r;
}

#ifdef __cplusplus
}
#endif

Biddy_Edge
BiddyManagedChange(Biddy_Manager MNG, Biddy_Edge f, Biddy_Variable v)
{
  Biddy_Edge e,t,r;
  Biddy_Variable fv;

  assert( MNG != NULL );
  assert( f != NULL );

  if (f == biddyZero) return biddyZero;

  fv = BiddyV(f);
  if (v == fv) {
    e = Biddy_InvCond(BiddyT(f),Biddy_GetMark(f));
    t = Biddy_InvCond(BiddyE(f),Biddy_GetMark(f));
    r = BiddyManagedFoaNode(MNG,fv,e,t,TRUE);
    BiddyRefresh(r); /* FoaNode will not refresh function! */
  } else if (Biddy_Managed_IsSmaller(MNG,v,fv)) {
    r = f;
    /* BiddyRefresh(r); */
  } else {
    e = BiddyManagedChange(MNG,Biddy_InvCond(BiddyE(f),Biddy_GetMark(f)),v);
    t = BiddyManagedChange(MNG,Biddy_InvCond(BiddyT(f),Biddy_GetMark(f)),v);
    r = BiddyManagedFoaNode(MNG,fv,e,t,TRUE);
    BiddyRefresh(r); /* FoaNode will not refresh function! */
  }
  return r;
}

/***************************************************************************//*!
\brief Function Biddy_Managed_Subset calculates a division of Boolean
       function with a literal.

### Description
    Original BDD is not changed.
    For combination sets, this function coincides with Subset0 and Subset1.
### Side effects
### More info
    Macro Biddy_Subset(f,v,value) is defined for use with anonymous manager.
    Macros Biddy_Managed_Subset0(MNG,f,v), Biddy_Subset0(f,v),
    Biddy_Managed_Subset1(MNG,f,v), and Biddy_Subset1(f,v) are defined for
    manipulation of combination sets.
*******************************************************************************/

#ifdef __cplusplus
extern "C" {
#endif

Biddy_Edge
Biddy_Managed_Subset(Biddy_Manager MNG, Biddy_Edge f, Biddy_Variable v,
                     Biddy_Boolean value)
{
  Biddy_Edge r;

  assert( f != NULL );

  if (!MNG) MNG = biddyAnonymousManager;
  ZF_LOGI("Biddy_Subset");

  assert( BiddyIsOK(f) == TRUE );

  r = BiddyManagedSubset(MNG,f,v,value);

  return r;
}

#ifdef __cplusplus
}
#endif

Biddy_Edge
BiddyManagedSubset(Biddy_Manager MNG, Biddy_Edge f, Biddy_Variable v,
                   Biddy_Boolean value)
{
  Biddy_Edge h;
  Biddy_Edge e, t, r;
  Biddy_Edge FF, GG, HH;
  Biddy_Variable fv;
  unsigned int cindex;

  assert( MNG != NULL );
  assert( f != NULL );

  if (f == biddyZero) return biddyZero;

  h = biddyVariableTable.table[v].variable;
  if (!value) {
    h = Biddy_Inv(h);
  }

  if (((uintptr_t) f) > ((uintptr_t) h)) {
    FF = f;
    GG = h;
    HH = biddyZero;
  } else {
    FF = h;
    GG = f;
    HH = biddyZero;
  }

  /* IF RESULT IS NOT IN THE CACHE TABLE... */
  if (!findOp3Cache(MNG,biddyOPCache,FF,GG,HH,&r,&cindex))
  {
    if ((fv=BiddyV(f)) == v) {
      if (value) {
        r = BiddyManagedFoaNode(MNG,v,biddyZero,Biddy_InvCond(BiddyT(f),Biddy_GetMark(f)),TRUE);
        BiddyRefresh(r); /* FoaNode will not refresh function! */
      } else {
        r = BiddyManagedFoaNode(MNG,v,Biddy_InvCond(BiddyE(f),Biddy_GetMark(f)),biddyZero,TRUE);
        BiddyRefresh(r); /* FoaNode will not refresh function! */
      }
    }
    else if (Biddy_Managed_IsSmaller(MNG,v,fv)) {
      if (value) {
        r = BiddyManagedFoaNode(MNG,v,biddyZero,f,TRUE);
        BiddyRefresh(r); /* FoaNode will not refresh function! */
      } else {
        r = BiddyManagedFoaNode(MNG,v,f,biddyZero,TRUE);
        BiddyRefresh(r); /* FoaNode will not refresh function! */
      }
    } else {
      e = BiddyManagedSubset(MNG,Biddy_InvCond(BiddyE(f),Biddy_GetMark(f)),v,value);
      t = BiddyManagedSubset(MNG,Biddy_InvCond(BiddyT(f),Biddy_GetMark(f)),v,value);
      r = BiddyManagedFoaNode(MNG,fv,e,t,TRUE);
      BiddyRefresh(r); /* FoaNode will not refresh function! */
      addOp3Cache(MNG,biddyOPCache,FF,GG,HH,r,cindex);
    }

  } else {

    /* IF THE RESULT IS FROM CACHE TABLE, REFRESH IT! */
    BiddyRefresh(r);

  }

  return r;
}

/***************************************************************************//*!
\brief Function Biddy_Managed_IsOK returns TRUE iff given node is not obsolete.

### Description
    This is needed for implementation of user caches.
### Side effects
### More info
    Macro BiddyIsOK(f) is defined for debugging. It will check more properties
    and not only the expiry value.
    Macro Biddy_IsOK(f) is defined for use with anonymous manager.
*******************************************************************************/

#ifdef __cplusplus
extern "C" {
#endif

Biddy_Boolean
Biddy_Managed_IsOK(Biddy_Manager MNG, Biddy_Edge f)
{
  if (!MNG) MNG = biddyAnonymousManager;

  return (!(BiddyN(f)->expiry) || ((BiddyN(f)->expiry) >= biddySystemAge));
}

#ifdef __cplusplus
}
#endif

/***************************************************************************//*!
\brief Function Biddy_Managed_GC performs garbage collection.

### Description
    All obsolete nodes are deleted.
    Parameter target is used during sifting.
    If parameter total is true than all obsolete nodes are deleted,
    otherwise nodes are deleted only if there are enough obsolete nodes.
    Nodes from deleted non-obsolete formulae are immediately removed
    only if parameter purge is true (this should not be used during the
    automatic garbage collection), otherwise these nodes only become fresh.
### Side effects
    The first element of each chain in a node table should have a special value
    for its 'prev' element to allow tricky but efficient deleting. Moreover,
    'prev' and 'next' should be the first and the second element in the
    structure BiddyNode, respectively. Garbage collecion is reported by
    biddyNodeTable.garbage only if some bad nodes are purged!
### More info
    Macro Biddy_GC(target,purge,total) is defined for use with anonymous
    manager.
    Macros Biddy_Managed_AutoGC(MNG) and Biddy_AutoGC() are useful variants
    with target = 0, purge = FALSE, and total = FALSE.
*******************************************************************************/

#ifdef __cplusplus
extern "C" {
#endif

void
Biddy_Managed_GC(Biddy_Manager MNG, Biddy_Variable target, Biddy_Boolean purge,
                 Biddy_Boolean total)
{
  unsigned int i,j;
  unsigned int k;
  BiddyFormula *tmp;
  BiddyNode *tmpnode1,*tmpnode2;
  BiddyNode *sup;
  BiddyCacheList *c;
  Biddy_Boolean cacheOK;
  Biddy_Boolean deletedFormulae;
  Biddy_Boolean resizeRequired;
  Biddy_Boolean gcUseful;
  Biddy_Variable v;
  unsigned int hash;
  clock_t starttime;

  if (!MNG) MNG = biddyAnonymousManager;
  ZF_LOGI("Biddy_Garbage");

  /* DEBUGGING */
  /*
  {
  static int n = 0;
  fprintf(stdout,"\nGarbage (%d), systemAge = %u, there are %u BDD nodes, NodeTableSize = %u...\n",
                 ++n,biddySystemAge,biddyNodeTable.num,biddyNodeTable.size);
  }
  */

  biddyNodeTable.garbage++;

#ifdef BIDDYEXTENDEDSTATS_YES
  if (!(biddyNodeTable.gcobsolete = (unsigned long long int *) realloc(biddyNodeTable.gcobsolete,
        biddyNodeTable.garbage * sizeof(unsigned long long int))))
  {
    fprintf(stderr,"Biddy_Managed_GC: Out of memoy!\n");
    exit(1);
  }
  biddyNodeTable.gcobsolete[biddyNodeTable.garbage-1] = 0;
#endif

  starttime = clock();

  /* REMOVE ALL FORMULAE WHICH ARE NOT PRESERVED ANYMORE */
  /* the first two formulae ("0" and "1") are never deleted */

  i = 0;
  deletedFormulae = FALSE;
  for (j = 0; j < biddyFormulaTable.size; j++) {
    if (!biddyFormulaTable.table[j].deleted &&
        (!biddyFormulaTable.table[j].expiry || biddyFormulaTable.table[j].expiry >= biddySystemAge)) {

      /* THIS FORMULA IS OK */
      if (i != j) {
        biddyFormulaTable.table[i] = biddyFormulaTable.table[j];
      }
      if (biddyFormulaTable.table[i].name) biddyFormulaTable.numOrdered = i+1;
      i++;

    } else {

      /* THIS FORMULA IS DELETED */
      if (!biddyFormulaTable.table[j].expiry || biddyFormulaTable.table[j].expiry >= biddySystemAge) {
        /* DELETED BUT NON-OBSOLETE FORMULAE */
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

      /* ALL FRESH, PROLONGED, AND FORTIFIED NODES BECOME */
      /* FRESH (deletenow = FALSE) OR OBSOLETE (deletenow = TRUE) */
      /* NOT EFFICIENT! ONLY NODES FROM DELETED FORMULAE SHOULD BE VISITED! */

      if (purge) {
        k = 1;
      } else {
        k = biddySystemAge;
      }
      for (v=1; v<biddyVariableTable.num; v++) {
        biddyVariableTable.table[v].lastNode->list = NULL;
        sup = biddyVariableTable.table[v].firstNode;
        while (sup) {
          if (!sup->expiry || sup->expiry >= biddySystemAge) {
            sup->expiry = k;
          }
          sup = (BiddyNode *) sup->list;
        }
      }

    }
  }

  /* RESTORE expiry VALUE FOR TOP NODE OF ALL VARIABLES, ELEMENTS AND EXTERNAL FORMULAE */
  /* THIS IS NEEDED EVEN IF THERE ARE NO DELETED FORMULAE (SEE E.G. SIFTING)*/
  for (v = 1; v < biddyVariableTable.num; v++) {
    BiddyN(biddyVariableTable.table[v].variable)->expiry = 0; /* variable is fortified */
    /* restoring elements is not implemented, yet */
  }
  for (j = 0; j < biddyFormulaTable.size; j++) {
    if (!Biddy_IsNull(biddyFormulaTable.table[j].f)) {
      BiddyProlongOne(MNG,biddyFormulaTable.table[j].f,biddyFormulaTable.table[j].expiry);
    }
  }

  /* REFRESH OBSOLETE NODES WHICH SHOULD NOT BE OBSOLETE */
  /* ALSO, COUNT HOW MANY OBSOLETE NODES PER VARIABLE EXIST */
  for (v=1; v<biddyVariableTable.num; v++) {
    biddyVariableTable.table[v].lastNode->list = NULL;
    biddyVariableTable.table[v].numobsolete = 0;
  }
  for (v=1; v<biddyVariableTable.num; v++) {
    /* during sifting we use target != 0 */
    if (target == v) {
      /* first node is always variable node and it is never obsolete */
      sup = biddyVariableTable.table[v].firstNode;
      assert( BiddyIsOK(sup) );
      while (sup) {
        assert(sup->v == v) ;
        if ((sup->expiry) && (sup->expiry < biddySystemAge)) {
          (biddyVariableTable.table[v].numobsolete)++;
        }
        sup = (BiddyNode *) sup->list;
      }
    }
    else if ((target == 0) || Biddy_Managed_IsSmaller(MNG,v,target)) {
      /* first node is always variable node and it is never obsolete */
      sup = biddyVariableTable.table[v].firstNode;
      assert( BiddyIsOK(sup) );
      while (sup) {
        assert(sup->v == v) ;
        if (!(sup->expiry) || (sup->expiry >= biddySystemAge)) {
          BiddyProlongRecursively(MNG,BiddyE(sup),sup->expiry);
          BiddyProlongRecursively(MNG,BiddyT(sup),sup->expiry);
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

  /* CHECK IF THE RESIZING OF NODE TABLE IS NECESSARY */
  i = biddyNodeTable.num; /* we will count how many nodes are non-obsolete */
  resizeRequired = FALSE;
  for (v=1; v<biddyVariableTable.num; v++) {

    /* there should be at least one non-obsolete node for each variable */
    assert ( biddyVariableTable.table[v].numobsolete < biddyVariableTable.table[v].num );

    i = i - biddyVariableTable.table[v].numobsolete;
  }
  resizeRequired = (
     (biddyNodeTable.size < biddyNodeTable.limitsize) &&
     (i > biddyNodeTable.resizeratio * biddyNodeTable.size)
  );

  /* CHECK IF THE DELETION OF NODES IS USEFUL - how many nodes will be deleted */
  gcUseful = ((biddyNodeTable.num - i) > (biddyNodeTable.gcratio * biddyNodeTable.blocksize));
  if (gcUseful) resizeRequired = FALSE;
  if (total && (i < biddyNodeTable.num)) gcUseful = TRUE;

  /* REMOVE ALL OBSOLETE NODES */
  /* RESIZE NODE TABLE IF IT SEEMS TO BE TO SMALL */
  cacheOK = TRUE;
  if (gcUseful || resizeRequired) {

#ifdef BIDDYEXTENDEDSTATS_YES
    biddyNodeTable.gcobsolete[biddyNodeTable.garbage-1]  = (biddyNodeTable.num - i);
#endif

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
      biddyNodeTable.table[0] = (BiddyNode *) biddyTerminal;

      /* ADAPT PARAMETERS */
      biddyNodeTable.gcratio = (1.0 - biddyNodeTable.gcratioX) + (biddyNodeTable.gcratio / biddyNodeTable.gcratioF);
      biddyNodeTable.resizeratio = (1.0 - biddyNodeTable.resizeratioX) + (biddyNodeTable.resizeratio * biddyNodeTable.resizeratioF);
      if (biddyNodeTable.gcratio < 0.0) biddyNodeTable.gcratio = 0.0;
      if (biddyNodeTable.resizeratio < 0.0) biddyNodeTable.resizeratio = 0.0;
    }

    /* BECAUSE OF REHASHING (NODES ARE STORED IN NODE TABLE IN 'ORDERED' LISTS) */
    /* IT IS REQUIRED TO VISIT NODES STARTING WITH THE LAST VARIABLE! */
    /* LAST VARIABLE IS NOT THE SAME AS THE SMALLEST/LARGEST IN THE VARIABLE ORDERING! */
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
        /* first node in the list is always variable node and it is always fortified */
        /* variable nodes are not stored in Node table and they never need rehasing */
        tmpnode1 = biddyVariableTable.table[v].firstNode;
        assert( BiddyIsOK(tmpnode1) );

        biddyVariableTable.table[v].lastNode->list = NULL;

        /* DEBUGGING */
        /*
        printf("GC FOR %u (OBSOLETE NODES: %u)\n",v,biddyVariableTable.table[v].numobsolete);
        */

        while (biddyVariableTable.table[v].numobsolete) {
          tmpnode2 = (BiddyNode *) tmpnode1->list;
          assert( tmpnode2 != NULL) ;
          assert( tmpnode2->v == v ) ;

          if (!(tmpnode2->expiry) || (tmpnode2->expiry >= biddySystemAge)) {
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
            /* update list of live nodes */
            tmpnode1->list = (BiddyNode *) tmpnode2->list;
            /* insert obsolete node to the list of free nodes */
            tmpnode2->list = (void *) biddyFreeNodes;
            biddyFreeNodes = tmpnode2;
          }

        }
        if (!tmpnode1->list) {
          biddyVariableTable.table[v].lastNode = tmpnode1;
        }
      }

    }
  }

  /* Updating cache tables - only if needed */
  if (!cacheOK) {
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
    Constants and variables should be fortified!
    Restoring elements is not implemented, yet.
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
  /* implemented by simple increasing of biddySystemAge */
  BiddyIncSystemAge(MNG);
}

#ifdef __cplusplus
}
#endif

/***************************************************************************//*!
\brief Function Biddy_Managed_Purge immediately removes all nodes which were
       not preserved or which are not preserved anymore.

### Description
    All fresh and obsolete nodes are immediatelly removed. Moreover, nodes
    from deleted prolonged formulae and nodes from deleted fortified formulae
    are removed if they are not needed by other formulae.
    Call to Biddy_Purge does not count as clearing and thus all preserved
    formulae remains preserved for the same number of clearings.
### Side effects
    Removes all fresh nodes!
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

  BiddyIncSystemAge(MNG);
  Biddy_Managed_GC(MNG,0,TRUE,TRUE);
  BiddyDecSystemAge(MNG);
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
    Removes all fresh nodes.
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
    This is an external variant of internal macro BiddyRefresh
    This is needed for implementing user caches.
### Side effects
### More info
    Macro Biddy_Refresh(f) is defined for use with anonymous manager.
*******************************************************************************/

#ifdef __cplusplus
extern "C" {
#endif

void
Biddy_Managed_Refresh(Biddy_Manager MNG, Biddy_Edge f)
{
  if (!MNG) MNG = biddyAnonymousManager;

  BiddyRefresh(f);
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
\brief Function Biddy_Managed_AddFormula adds formula to Formula table.

### Description
    Nodes of the given BDD will be preserved for the given number of clearings.
    If (x != NULL) then formula is accessible by its name. If (c == -1)
    then formula is not preserved. If (c == 0) then formula is persistently
    preserved and you have to use Biddy_DeleteFormula to remove its nodes.
    There are two macros defined to simplify formulae management.
    Macro Biddy_Managed_AddTmpFormula(mng,bdd,c) is
    defined as Biddy_Managed_AddFormula(mng,NULL,bdd,c) and macro
    Biddy_Managed_AddPersistentFormula(mng,name,bdd) is defined as
    Biddy_Managed_AddFormula(mng,name,bdd,0).
### Side effects
    Function is prolonged or fortified. Formulae with name are ordered by name.
    If formula with the same name already exists, it will be overwritten
    (preserved and persistently preserved formulae, too)!
### More info
    Macros Biddy_AddFormula(x,f), Biddy_AddTmpFormula(f,c), and
    Biddy_AddPersistentFormula(x,f) are defined for use with anonymous manager.
*******************************************************************************/

#ifdef __cplusplus
extern "C" {
#endif

unsigned int
Biddy_Managed_AddFormula(Biddy_Manager MNG, Biddy_String x, Biddy_Edge f, int c)
{
  unsigned int i,j;
  Biddy_Boolean OK,find;
  int cc;
  BiddyFormula *tmp;
  Biddy_Variable v;

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

  if (c == -1) {
    c = biddySystemAge;
  }
  else if (c) {
    if ((c+biddySystemAge) < biddySystemAge) {
      fprintf(stdout,"ERROR (Biddy_Managed_AddFormula): expiry value is to large\n");
      free((void *)1);
      exit(1);
    }
    c += biddySystemAge;
  }

  /* if formula does not have a name and it is known to be already */
  /* persistently preserved then it is not neccessary to add it */
  if (!x && (Biddy_IsNull(f) || Biddy_IsConstant(f))) {
    return 0;
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
    printf("WARNING (Biddy_Managed_AddFormula): formula %s must not be changed!\n",x);
    return 0;
  }

  /* IF FORMULA HAS A NAME SEARCH FOR THE VARIABLE WITH THE SAME NAME */
  find = FALSE;
  if (x) {
    v = 1;
    while (!find && v<biddyVariableTable.num) {
      if (!strcmp(x,biddyVariableTable.table[v].name)) {
        find = TRUE;
      } else {
        v++;
      }
    }
  }
  if (find) {
    printf("WARNING (Biddy_Managed_AddFormula): formula %s not added because it has the same name as an existing variable!\n",x);
    return 0;
  }

  /* element found */
  if (OK) {

    /* need to change persistent formula - new formula will be created */
    if (!biddyFormulaTable.table[i].expiry) {
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
    else if (biddyFormulaTable.table[i].expiry > biddySystemAge) {
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
    biddyFormulaTable.table[i].expiry = (unsigned int) c;
    biddyFormulaTable.table[i].deleted = FALSE;
    if (!Biddy_IsNull(f)) {
      BiddyProlongOne(MNG,f,(unsigned int) c);
    }
    return i;
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
  biddyFormulaTable.table[i].expiry = (unsigned int) c;
  biddyFormulaTable.table[i].deleted = FALSE;
  if (!Biddy_IsNull(f)) {
    BiddyProlongOne(MNG,f,(unsigned int) c);
  }

  /* DEBUGGING */
  /*
  printf("Biddy_Managed_AddFormula: i=%d/%d, x=%s, f=%p\n",i,biddyFormulaTable.size-1,x,f);
  */

  return i;
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
  Biddy_Variable v;
  Biddy_Boolean OK;
  int cc;

  /* TO DO: USE BINARY SEARCH BECAUSE FORMULAE ARE ORDERED BY NAME!! */

  if (!MNG) MNG = biddyAnonymousManager;

  *f = biddyNull;
  if (!x) return FALSE;

  if (!strcmp(x,biddyFormulaTable.table[0].name)) { /* [0] = biddyZero */
    *f = biddyZero;
    return TRUE;
  }

  if (!strcmp(x,biddyFormulaTable.table[1].name)) { /* [1] = biddyOne */
    *f = biddyOne;
    return TRUE;
  }

  /* VARIABLES ARE CONSIDERED TO BE FORMUALE, [0] = constant 1 */
  v = 1;
  OK = FALSE;
  while (!OK && v<biddyVariableTable.num) {
    if (!strcmp(x,biddyVariableTable.table[v].name)) {
      OK = TRUE;
    } else {
      v++;
    }
  }

  if (OK) {
    *f = biddyVariableTable.table[v].variable;
    return TRUE;
  }

  if (!biddyFormulaTable.table) return FALSE;

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
    Global ordering is the number of zeros in corresponding line of orderingTable.
### Side effects
    All obsolete nodes will be removed.
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

  /* OBSOLETE NODES ARE REMOVED BEFORE SWAPPING */
  Biddy_Managed_AutoGC(MNG);

  vhigh = swapWithHigher(MNG,v,&active);

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
    Global ordering is the number of zeros in corresponding line of orderingTable.
### Side effects
    All obsolete nodes will be removed.
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

  /* OBSOLETE NODES ARE REMOVED BEFORE SWAPPING */
  Biddy_Managed_AutoGC(MNG);

  vlow = swapWithLower(MNG,v,&active);

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
    All obsolete nodes will be removed.
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
  clock_t starttime;

  assert( (f == NULL) || (BiddyIsOK(f) == TRUE) );

  if (!MNG) MNG = biddyAnonymousManager;
  ZF_LOGI("Biddy_Sifting");

  starttime = clock();

  if (f && (((BiddyNode *) BiddyP(f))->expiry == biddySystemAge)) {
    fprintf(stdout,"ERROR (Biddy_Managed_Sifting): Sifting on fresh function is wrong!\n");
    return FALSE;
  }

  if (FALSE) {
    /* sifting will not be performed */
    return FALSE;
  }

  if (!f) {

    /* MINIMIZING NODE NUMBER FOR THE WHOLE SYSTEM */

    Biddy_Managed_GC(MNG,0,TRUE,TRUE);

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

    /* garbage collection is not needed here */
    /* however, it seems to be a good idea */
    Biddy_Managed_GC(MNG,0,TRUE,TRUE);

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

  /* DELETE ALL CACHE TABLES AND DISABLE THEM */
  /**/
  OPGarbageDeleteAll(MNG);
  EAGarbageDeleteAll(MNG);
  RCGarbageDeleteAll(MNG);
  biddyOPCache.disabled = TRUE;
  biddyEACache.disabled = TRUE;
  biddyRCCache.disabled = TRUE;
  /**/

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
    printf("Expiry value of top node: %d\n",((BiddyNode *) BiddyP(f))->expiry);
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
  v = 0;
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
          Biddy_Managed_GC(MNG,Biddy_Managed_IsSmaller(MNG,k,v)?v:k,FALSE,TRUE);
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

        if (num < min) { /* first direction should use strict less */
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
      /* this garbage collection is required here */
      /* due to gc optimizations used further (parameter target!) */
      Biddy_Managed_GC(MNG,0,FALSE,TRUE);
    } else {
      /* garbage collection is not needed here */
      /* however, it seems to be a good idea */
      Biddy_Managed_GC(MNG,0,FALSE,TRUE);
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
          Biddy_Managed_GC(MNG,Biddy_Managed_IsSmaller(MNG,k,v)?v:k,FALSE,TRUE);
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

        if (num <= min) { /* second direction should use less or equal */
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
      Biddy_Managed_GC(MNG,0,FALSE,TRUE);
    } else {
      /* garbage collection is not needed here */
      /* however, it seems to be a good idea */
      Biddy_Managed_GC(MNG,0,FALSE,TRUE);
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

    /* EVERY NEXT VARIABLE HAS A LITTLE BIT SMALLER TRESHOLD */
    if (!f) {
      treshold = treshold + (biddyNodeTable.siftingtreshold - 1) / (2 * Biddy_Managed_VariableTableNum(MNG));
    }

    /* DEBUGGING */
    /*
    printf("TRESHOLD = %.3f\n",treshold);
    */

  }  /* while (!finish) */

  /* THIS LINE DISABLES FURTHER CALLS IN THE CASE OF NOT-CONVERGING SIFTING */
  /* IN THE CASE OF CONVERGING SIFTING, TRESHOLD IS ADAPTED IN EVERY STEP */
  if (!converge) {
    totalbest = min;
  } else {
    if (!f) {
      /* treshold = (treshold + 1) / 2; */ /* decrease treshold */
      /* treshold = treshold * treshold; */ /* increase treshold */
      treshold = 2 * treshold - 1; /* increase treshold */
      if (treshold > 2 * biddyNodeTable.convergesiftingtreshold)
        treshold = 2 * biddyNodeTable.convergesiftingtreshold;
    } else {
      /* treshold = (treshold + 1) / 2; */ /* decrease treshold */
      /* treshold = treshold * treshold; */ /* increase treshold */
      treshold = 2 * treshold - 1; /* increase treshold */
      if (treshold > 2 * biddyNodeTable.fconvergesiftingtreshold)
        treshold = 2 * biddyNodeTable.fconvergesiftingtreshold;
    }
  }

  } /*   while (min < totalbest) */

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
    printf("Expiry value of top node: %d\n",((BiddyNode *) BiddyP(f))->expiry);
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

  /* ENABLE ALL CACHE TABLES */
  /**/
  biddyOPCache.disabled = FALSE;
  biddyEACache.disabled = FALSE;
  biddyRCCache.disabled = FALSE;
  /**/

  biddyNodeTable.drtime += clock() - starttime;

  return TRUE; /* sifting has been performed */

}

#ifdef __cplusplus
}
#endif

/***************************************************************************//*!
\brief Function Biddy_Managed_Copy copies a graph from one manager to
       another manager.
### Description
    The function takes a graph from one manager and creates the same graph
    in another manager. The resulting graph will represent the same Boolean
    function assuming the domain from the target manager.
    If f = biddyZero then only the domain is copied.
### Side effects
    If source and target manager are the same then function does nothing.
    The variable ordering of created BDD is adapted to the target manager. 
### More info
    Macro Biddy_Copy(MNG2,f) is defined for use with anonymous manager.
*******************************************************************************/

#ifdef __cplusplus
extern "C" {
#endif

Biddy_Edge
Biddy_Managed_Copy(Biddy_Manager MNG1, Biddy_Manager MNG2, Biddy_Edge f)
{
  Biddy_Edge r;
  Biddy_Variable v,k;
  Biddy_String vname;
  Biddy_Boolean simplecopy;
  
  if (!MNG1) MNG1 = biddyAnonymousManager;
  ZF_LOGI("Biddy_Copy");

  assert ( biddyManagerType1 == biddyManagerType2 );
  if (MNG1 == MNG2) {
    return f;
  }

  /* CHECK IF THE TARGET MNG IS EMPTY (NO PROBLEMS WITH ORDERING) */
  simplecopy = FALSE;
  if (biddyVariableTable2.num == 1) {
    simplecopy = TRUE;
  }
  /* COPY THE DOMAIN */
  /* the ordering of new variable is determined in Biddy_InitMNG */
  /* FOR OBDD, OFDD, TZBDD, AND TZFDD: [1] < [2] < ... < [size-1] < [0] */

  v = 0;
  for (k = 1; k < biddyVariableTable1.num; k++) {
      v = biddyVariableTable1.table[v].prev;
  }
  for (k = 1; k < biddyVariableTable1.num; k++) {
    vname = Biddy_Managed_GetVariableName(MNG1,v);

    /* DEBUGGING */
    /*
    printf("Variable %s (%u), next variable is %s (%u)\n",
            vname,v,Biddy_Managed_GetVariableName(MNG1,biddyVariableTable1.table[v].next),biddyVariableTable1.table[v].next);
    */

    r = Biddy_Managed_AddVariableByName(MNG2,vname);
    v = biddyVariableTable1.table[v].next;
  }

  if (f == biddyZero1) return biddyZero2; /* domain has been already copied */

  /* IF THE TARGET MNG IS EMPTY (NO PROBLEMS WITH ORDERING) */
  if (simplecopy) {
    r = BiddyCopy(MNG1,MNG2,f);
  } else {
    r = BiddyCopyOBDD(MNG1,MNG2,f);
  }
  
  return r;
}

#ifdef __cplusplus
}
#endif

/***************************************************************************//*!
\brief Function Biddy_Managed_CopyFormula uses Biddy_Managed_Copy to copy a
       graph from one manager to another manager.
### Description
    See Biddy_Managed_Copy.
### Side effects
    If source and target manager are the same then function does nothing.
    The variable ordering of created BDD is adapted to the target manager. 
    The created formula is not preserved.
### More info
    Macro Biddy_CopyFormula(MNG2,x) is defined for use with anonymous manager.
*******************************************************************************/

#ifdef __cplusplus
extern "C" {
#endif

void
Biddy_Managed_CopyFormula(Biddy_Manager MNG1, Biddy_Manager MNG2, Biddy_String x)
{
  Biddy_Boolean OK;
  Biddy_Edge src,dst;

  if (!MNG1) MNG1 = biddyAnonymousManager;

  if (!x) return;

  OK = Biddy_Managed_FindFormula(MNG1,x,&src);

  if (OK) {
    OK = Biddy_Managed_FindFormula(MNG2,x,&dst);
    if (OK) {
      printf("WARNING (Biddy_Managed_CopyFormula): formula overwritten!\n");
    }
    dst = Biddy_Managed_Copy(MNG1,MNG2,src);
    Biddy_Managed_AddFormula(MNG2,x,dst,-1);
  }
}

#ifdef __cplusplus
}
#endif

/***************************************************************************//*!
\brief Function Biddy_Managed_Eval returns the value of a Boolean function
       for a given variable assignment.
### Description
### Side effects
### More info
    Macro Biddy_Eval(f) is defined for use with anonymous manager.
*******************************************************************************/

#ifdef __cplusplus
extern "C" {
#endif

Biddy_Boolean
Biddy_Managed_Eval(Biddy_Manager MNG, Biddy_Edge f)
{
  Biddy_Boolean r;

  assert( f != NULL );

  if (!MNG) MNG = biddyAnonymousManager;

  r = FALSE;

  /* NOT IMPLEMENTED, YET */
  return r;
}

#ifdef __cplusplus
}
#endif

/***************************************************************************//*!
\brief Function Biddy_Managed_Random generates a random BDD.

### Description
    The represented Boolean function depends on the variables given with
    parameter support whilst the parameter r determines the ratio between
    the number of function's minterms and the number of all possible minterms.
    Parameter support is a product of positive varaiables.
### Side effects
    Parameter r must be a number from [0,1]. Otherwise, function returns
    biddyNull.
### More info
    Macro Biddy_Random(support,r) is defined for use with anonymous manager.
*******************************************************************************/

#ifdef __cplusplus
extern "C" {
#endif

Biddy_Edge
Biddy_Managed_Random(Biddy_Manager MNG, Biddy_Edge support, double r)
{
  long long unsigned int n,m,x,z;
  Biddy_Edge p,q,one,tmp,result;
  Biddy_Boolean opposite;

  if (!MNG) MNG = biddyAnonymousManager;
  ZF_LOGI("Biddy_Random");

  if ((r < 0.0) || (r > 1.0)) {
    return biddyNull;
  }

  n = 1;
  p = support;
  while (!Biddy_IsConstant(p)) {

    assert( n < 2 * n );

    n = 2 * n;
    p = BiddyT(p);
  }

  opposite = FALSE;
  if (r > 0.5) {
    r = 1.0 - r;
    opposite = TRUE;
  }

  m = (long long unsigned int) (0.5 + n * r);

  if (m == 0) {
    if (opposite) {
      return biddyOne;
    } else {
      return biddyZero;
    }
  }

  result = biddyZero;
  while (m != 0) {

    /* GENERATE RANDOM NUMBER */
    x = rand();
    x = (n*x)/RAND_MAX;

    /* DEBUGGING */
    /*
    printf("RANDOM NUMBER = %llu\n",x);
    */

    p = support;
    z = n;
    q = biddyOne;
    while (z != 1) {
      one = biddyVariableTable.table[BiddyV(p)].variable;

      assert( !Biddy_IsConstant(one) );

      p = BiddyT(p);
      if (x <= ((z - 0.5) / 2)) {
        z = (long long unsigned int) (z + 0.5) / 2;
        q = BiddyManagedITE(MNG,one,biddyZero,q);
      } else {
        z = (long long unsigned int) (z + 0.5) / 2;
        x = x - z;
        q = BiddyManagedITE(MNG,one,q,biddyZero);
      }
    }

    tmp = BiddyManagedOr(MNG,result,q);
    if (tmp != result) {
      result = tmp;
      m--;
    }

  }

  if (opposite) {
    result = Biddy_Inv(result);
  }

  BiddyRefresh(result);

  return result;
}

#ifdef __cplusplus
}
#endif

/***************************************************************************//*!
\brief Function Biddy_Managed_RandomSet generates a random BDD.

### Description
    The represented set is a random combination set determined by the
    parameter unit whilst the parameter r determines the ratio between
    the number of set's subsets and the number of all possible subsets.
    Parameter set is a set containing only one subset which consist
    of all elements, i.e. it is a set {{x1,x2,...,xn}}.
### Side effects
    Parameter r must be a number from [0,1]. Otherwise, function returns
    biddyNull.
### More info
    Macro Biddy_RandomSet(unit,r) is defined for use with anonymous manager.
*******************************************************************************/

#ifdef __cplusplus
extern "C" {
#endif

Biddy_Edge
Biddy_Managed_RandomSet(Biddy_Manager MNG, Biddy_Edge unit, double r)
{
  long long unsigned int n,m,x,z;
  Biddy_Edge p,q,tmp,result;
  Biddy_Boolean opposite;
  Biddy_Variable one;

  if (!MNG) MNG = biddyAnonymousManager;
  ZF_LOGI("Biddy_RandomSet");

  if ((r < 0.0) || (r > 1.0)) {
    return biddyNull;
  }

  n = 1;
  p = unit;
  while (!Biddy_IsConstant(p)) {

    assert( n < 2 * n );

    n = 2 * n;
    p = BiddyT(p);
  }

  opposite = FALSE;
  if (r > 0.5) {
    r = 1.0 - r;
    opposite = TRUE;
  }

  m = (long long unsigned int) (0.5 + n * r);

  if (m == 0) {
    if (opposite) {
      return biddyOne;
    } else {
      return biddyZero;
    }
  }

  result = biddyZero;
  while (m != 0) {

    /* DEBUGGING */
    /*
    printf("RANDOM SET: m=%llu\n",m);
    */

    /* GENERATE RANDOM NUMBER */
    x = rand();
    x = (n*x)/RAND_MAX;

    /* DEBUGGING */
    /*
    printf("RANDOM NUMBER = %llu\n",x);
    */

    p = unit;
    z = n;
    q = Biddy_Managed_GetBaseSet(MNG);
    while (z != 1) {
      one = BiddyV(p);
      p = BiddyT(p);
      if (x <= ((z - 0.5) / 2)) {
        z = (long long unsigned int) (z + 0.5) / 2;
      } else {
        z = (long long unsigned int) (z + 0.5) / 2;
        x = x - z;
        q = BiddyManagedChange(MNG,q,one);
      }
    }

    tmp = BiddyManagedUnion(MNG,result,q);
    if (tmp != result) {
      result = tmp;
      m--;
    }

  }

  if (opposite) {
    result = Biddy_Inv(result);
  }

  BiddyRefresh(result);

  return result;
}

#ifdef __cplusplus
}
#endif

/*----------------------------------------------------------------------------*/
/* Definition of internal functions                                           */
/*----------------------------------------------------------------------------*/

/***************************************************************************//*!
\brief Function BiddyIncSystemAge increments system age by 1.

### Description
    All prolonged nodes become fresh nodes, all fresh nodes become obsolete
    nodes.
### Side effects
### More info
*******************************************************************************/

void
BiddyIncSystemAge(Biddy_Manager MNG)
{
  if (++biddySystemAge <= 0) {
    printf("WARNING: biddySystemAge is back to the start");
    biddySystemAge--;
    /* TO DO: change expiry value of all obsolete and fresh nodes to 1 */
    /* TO DO: change expiry value of all prolonged nodes to 2 */
    biddySystemAge = 2;
  }
}

/***************************************************************************//*!
\brief Function BiddyDecSystemAge decrements system age by 1.

### Description
    All fresh nodes become prolonged nodes.
### Side effects
### More info
*******************************************************************************/

void
BiddyDecSystemAge(Biddy_Manager MNG)
{
  if (--biddySystemAge < 1) {
    printf("ERROR (BiddyDecSystemAge): biddySystemAge is wrong");
  }
}

/***************************************************************************//*!
\brief Function BiddyProlongRecursively prolonges obsolete nodes in a function.

### Description
### Side effects
    The goal is to make nodes non-obsolete and not to give them correct expiry
    value. Nodes are prolonged recursively until a non-obsolete node is reached.
### More info
*******************************************************************************/

void
BiddyProlongRecursively(Biddy_Manager MNG, Biddy_Edge f, unsigned int c)
{
  if (((BiddyNode *) BiddyP(f))->expiry &&
      (((BiddyNode *) BiddyP(f))->expiry < biddySystemAge))
  {
    BiddyProlongOne(MNG,f,c);
    if (biddyVariableTable.table[BiddyV(f)].numobsolete != 0) {
      (biddyVariableTable.table[BiddyV(f)].numobsolete)--;
    }
    BiddyProlongRecursively(MNG,BiddyE(f),c);
    BiddyProlongRecursively(MNG,BiddyT(f),c);
  }
}

/*******************************************************************************
\brief Function BiddyCreateLocalInfo creates local info table for the given
       function.

### Description
    Only one local info table may exists! It is not allowed to add new BDD
    nodes if local info table exists. It is not allowed to start GC if local
    info table exists. The last element and only the last element in the table
    has back = NULL. Function returns number of noticeable variables.
    For OBDD, noticeable variables are all variables existing in the graph.
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

  if (Biddy_IsNull(f)) return 0;
  if (Biddy_IsConstant(f)) return 0;

  assert( !Biddy_Managed_IsSelected(MNG,f) );

  num = 0;  /* CONSTANT NODE IS NOT COUNTED HERE */

  for (i=1;i<biddyVariableTable.num;i++) {
    biddyVariableTable.table[i].selected = FALSE;
  }

  Biddy_Managed_SelectNode(MNG,biddyTerminal);
  BiddyNodeVarNumber(MNG,f,&num); /* NOW, ALL NODES ARE SELECTED */
  Biddy_Managed_DeselectNode(MNG,biddyTerminal); /* REQUIRED FOR createLocalInfo */

  var = 0;
  for (i=1;i<biddyVariableTable.num;i++) {
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
      (((uintptr_t)(((BiddyNode *)BiddyP(f))->list)) -
       ((uintptr_t)(((BiddyNode *)BiddyP(root))->list))
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
    (((BiddyNode *)BiddyP(root))->list))[n].back);
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
  ((BiddyLocalInfo *)(((BiddyNode *)BiddyP(f))->list))->data.enumerator = n;
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
  return ((BiddyLocalInfo *)(((BiddyNode *)BiddyP(f))->list))->data.enumerator;
}

/*******************************************************************************
\brief Function BiddySetNodeCount is used in nodePlainNumber().

### Description
### Side effects
    Local info must be created before using this function.
    This should not be called for constant node because constant node does
    not have local info!
### More info
*******************************************************************************/

void
BiddySetNodeCount(Biddy_Edge f, unsigned int value)
{
  ((BiddyLocalInfo *)(((BiddyNode *) BiddyP(f))->list))->data.nodeCount = value;
}

/*******************************************************************************
\brief Function BiddyGetNodeCount is used in nodePlainNumber().

### Description
### Side effects
    Local info must be created before using this function.
    This should not be called for constant node because constant node does
    not have local info!
### More info
*******************************************************************************/

unsigned int
BiddyGetNodeCount(Biddy_Edge f)
{
  return ((BiddyLocalInfo *)(((BiddyNode *) BiddyP(f))->list))->data.nodeCount;
}

/*******************************************************************************
\brief Function BiddySetPath1Count is used in pathCount().

### Description
### Side effects
    Local info must be created before using this function.
    This should not be called for constant node because constant node does
    not have local info!
### More info
*******************************************************************************/

void
BiddySetPath1Count(Biddy_Edge f, unsigned long long int value)
{
  ((BiddyLocalInfo *)(((BiddyNode *) BiddyP(f))->list))->data.path1Count = value;
}

/*******************************************************************************
\brief Function BiddyGetPath1Count is used in pathCount().

### Description
### Side effects
    Local info must be created before using this function.
    This should not be called for constant node because constant node does
    not have local info!
### More info
*******************************************************************************/

unsigned long long int
BiddyGetPath1Count(Biddy_Edge f)
{
  return ((BiddyLocalInfo *)(((BiddyNode *) BiddyP(f))->list))->data.path1Count;
}

/*******************************************************************************
\brief Function BiddySetPath0Count is used in pathCount().

### Description
### Side effects
    Local info must be created before using this function.
    This should not be called for constant node because constant node does
    not have local info!
### More info
*******************************************************************************/

void
BiddySetPath0Count(Biddy_Edge f, unsigned long long int value)
{
  ((BiddyLocalInfo *)(((BiddyNode *) BiddyP(f))->list))->datax.path0Count = value;
}

/*******************************************************************************
\brief Function BiddyGetPath0Count is used in pathCount().

### Description
### Side effects
    Local info must be created before using this function.
    This should not be called for constant node because constant node does
    not have local info!
### More info
*******************************************************************************/

unsigned long long int
BiddyGetPath0Count(Biddy_Edge f)
{
  return ((BiddyLocalInfo *)(((BiddyNode *) BiddyP(f))->list))->datax.path0Count;
}

/*******************************************************************************
\brief Function BiddySetLeftmost is used in pathCount().

### Description
### Side effects
    Local info must be created before using this function.
    This should not be called for constant node because constant node does
    not have local info!
### More info
*******************************************************************************/

void
BiddySetLeftmost(Biddy_Edge f, Biddy_Boolean value)
{
  ((BiddyLocalInfo *)(((BiddyNode *) BiddyP(f))->list))->datax.leftmost = value;
}

/*******************************************************************************
\brief Function BiddyGetLeftmost is used in pathCount().

### Description
### Side effects
    Local info must be created before using this function.
    This should not be called for constant node because constant node does
    not have local info!
### More info
*******************************************************************************/

Biddy_Boolean
BiddyGetLeftmost(Biddy_Edge f)
{
  return ((BiddyLocalInfo *)(((BiddyNode *) BiddyP(f))->list))->datax.leftmost;
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
    (((BiddyNode *) BiddyP(f))->list))->data.mintermCount);
  mpz_set(((BiddyLocalInfo *)
    (((BiddyNode *) BiddyP(f))->list))->data.mintermCount,value);
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
    (((BiddyNode *) BiddyP(f))->list))->data.mintermCount);
}

/*******************************************************************************
\brief Function BiddySelectNP is used in nodePlainNumber().

### Description
### Side effects
    Local info must be created before using this function.
    This should not be called for constant node because constant node does
    not have local info!
### More info
*******************************************************************************/

void
BiddySelectNP(Biddy_Edge f)
{
  if (Biddy_GetMark(f)) {
    (((BiddyLocalInfo *)
      (((BiddyNode *) BiddyP(f))->list))->data.npSelected) |= (unsigned int) 2;
  } else {
    (((BiddyLocalInfo *)
      (((BiddyNode *) BiddyP(f))->list))->data.npSelected) |= (unsigned int) 1;
  }
}

/*******************************************************************************
\brief Function BiddyIsSelectedNP is used in nodePlainNumber().

### Description
### Side effects
    Local info must be created before using this function.
    This should not be called for constant node because constant node does
    not have local info!
### More info
*******************************************************************************/

Biddy_Boolean
BiddyIsSelectedNP(Biddy_Edge f)
{
  Biddy_Boolean r;

  if (Biddy_GetMark(f)) {
    r = ((((BiddyLocalInfo *)
          (((BiddyNode *) BiddyP(f))->list))->data.npSelected
         ) & (unsigned int) 2
        ) != 0;
  } else {
    r = ((((BiddyLocalInfo *)
          (((BiddyNode *) BiddyP(f))->list))->data.npSelected
         ) & (unsigned int) 1
        ) != 0;
  }

  return r;
}

/*******************************************************************************
\brief Function BiddyCopy create a copy of BDD in another manager.

### Description
### Side effects
    Expiry value of the original nodes is not copied.
    All created nodes are refreshed in the new manager.
    It is assumed that the target manager has the same or larger domain and
    the same ordering for all variables from the source manager
### More info
*******************************************************************************/

Biddy_Edge
BiddyCopy(Biddy_Manager MNG1, Biddy_Manager MNG2, Biddy_Edge f)
{
  Biddy_Edge e,t;
  Biddy_Boolean mark;
  Biddy_String top;
  Biddy_Variable v;
  Biddy_Edge r;

  assert( MNG1 != MNG2 );
  assert( biddyManagerType1 == biddyManagerType2 );

  if (f == biddyNull) return biddyNull;
  if (f == biddyZero1) return biddyZero2;
  if (f == biddyOne1) return biddyOne2;

  mark = Biddy_GetMark(f); /* complement bit of the given edge */
  top = Biddy_Managed_GetTopVariableName(MNG1,f); /* top variable of the given edge */

  v = Biddy_Managed_GetVariable(MNG2,top);

  e = BiddyCopy(MNG1,MNG2,Biddy_Managed_TransferMark(MNG1,BiddyE(f),mark,TRUE)); /* TRUE = left */
  t = BiddyCopy(MNG1,MNG2,Biddy_Managed_TransferMark(MNG1,BiddyT(f),mark,FALSE)); /* FALSE = right */
  
  r = BiddyManagedFoaNode(MNG2,v,e,t,TRUE);
  BiddyProlongOne(MNG2,r,biddySystemAge2);

  return r;
}

/*******************************************************************************
\brief Function BiddyCopyOBDD create a copy of a graph using BIDDYTYPEOBDD in
       another manager.

### Description
### Side effects
    Expiry value of the original nodes is not copied.
    All created nodes are refreshed in the new manager.
    The variable ordering of created BDD is adapted to the target manager.
    It is assumed that the target manager has the same or larger domain.
### More info
*******************************************************************************/

Biddy_Edge
BiddyCopyOBDD(Biddy_Manager MNG1, Biddy_Manager MNG2, Biddy_Edge f)
{
  Biddy_String topname;
  Biddy_Variable v;
  Biddy_Boolean mark;
  Biddy_Edge e,t,r;

  assert ( biddyManagerType1 == BIDDYTYPEOBDD );

  if (f == biddyNull) return biddyNull;
  if (f == biddyZero1) return biddyZero2;
  if (f == biddyOne1) return biddyOne2;

  topname = Biddy_Managed_GetTopVariableName(MNG1,f);
  mark = Biddy_GetMark(f);
  v = Biddy_Managed_GetVariable(MNG2,topname); /* variable must exist */
  e = BiddyCopyOBDD(MNG1,MNG2,Biddy_Managed_TransferMark(MNG1,BiddyE(f),mark,TRUE)); /* TRUE = left */
  t = BiddyCopyOBDD(MNG1,MNG2,Biddy_Managed_TransferMark(MNG1,BiddyT(f),mark,FALSE)); /* FALSE = right */
  r = Biddy_Managed_ITE(MNG2,Biddy_Managed_GetVariableEdge(MNG2,v),t,e);
  
  return r;
}

/*******************************************************************************
\brief Function BiddyCopyZBDD is not implemented.

### Description
### Side effects
    NOT IMPLEMENTED
### More info
*******************************************************************************/

Biddy_Edge
BiddyCopyZBDD(Biddy_Manager MNG1, Biddy_Manager MNG2, Biddy_Edge f)
{
  return biddyNull;
}

/*******************************************************************************
\brief Function BiddyCopyTZBDD is not implemented.

### Description
### Side effects
    NOT IMPLEMENTED
### More info
*******************************************************************************/

Biddy_Edge
BiddyCopyTZBDD(Biddy_Manager MNG1, Biddy_Manager MNG2, Biddy_Edge f)
{
  return biddyNull;
}

/*******************************************************************************
\brief Function BiddyConvertDirect is not implemented.

### Description
### Side effects
    NOT IMPLEMENTED
### More info
*******************************************************************************/

Biddy_Edge
BiddyConvertDirect(Biddy_Manager MNG1, Biddy_Manager MNG2, Biddy_Edge f)
{
  return biddyNull;
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

#ifdef SYSTEMREPORTDETAILS
    printf("firstNode: pointer=%p, lastNode=%p\n",
      (void *) biddyVariableTable.table[v].firstNode,
      (void *) biddyVariableTable.table[v].lastNode
    );
#endif

    sup = biddyVariableTable.table[v].firstNode;

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
        fprintf(stderr,"ERROR (BiddySystemReport): Problem with lastNode for variable %s\n",biddyVariableTable.table[v].name);
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
  
      if (sup != biddyVariableTable.table[v].firstNode) {
        hash = nodeTableHash(v,sup->f,sup->t,biddyNodeTable.size);
        sup1 = biddyNodeTable.table[hash];
        findNodeTable(MNG,v,sup->f,sup->t,&sup1);
        if (!sup1) {
          fprintf(stderr,"ERROR (BiddySystemReport): Node not correctly inserted into Node table\n");
          printf("NODE: pointer=%p, expiry=%d, variable %s, else=%s, then=%s\n",
            (void *) sup,sup->expiry,
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
            printf("NODE: pointer=%p, expiry=%d, variable %s, else=%s (%p), then=%s (%p)\n",
              (void *) sup,sup->expiry,
              Biddy_Managed_GetTopVariableName(MNG,sup),
              Biddy_Managed_GetTopVariableName(MNG,sup->f),sup->f,
              Biddy_Managed_GetTopVariableName(MNG,sup->t),sup->t);
            free((void *)1); /* BREAKPOINT */
          }
        }
      }

#ifdef SYSTEMREPORTDETAILS
      printf("NODE: pointer=%p, ",(void *) sup);
      printf("expiry=%d, variable %s, else=%s, then=%s\n",
        sup->expiry,
        Biddy_Managed_GetTopVariableName(MNG,sup),
        Biddy_Managed_GetTopVariableName(MNG,sup->f),
        Biddy_Managed_GetTopVariableName(MNG,sup->t));
#endif

      if (!(sup->expiry)) {
        countfortified++;
      } else if (sup->expiry == biddySystemAge) {
        countfresh++;
      } else if (sup->expiry > biddySystemAge) {
        countprolonged++;
      } else {
        countobsolete++;
      }

      count1++;

      end = (sup == biddyVariableTable.table[v].lastNode);
      if (!end) sup = (BiddyNode *) sup->list;

    } while (!end);

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
        if (!(sup->expiry)) {
          countfortified++;
        } else if (sup->expiry == biddySystemAge) {
          countfresh++;
        } else if (sup->expiry > biddySystemAge) {
          countprolonged++;
        } else {
          countobsolete++;
      }

#ifdef SYSTEMREPORTDETAILS
        if (BiddyV(sup) < biddyVariableTable.num) {
          printf("NODE: i=%u, pointer=%p, expiry=%d, variable %s, else=%s, then=%s\n",
            i, (void *) sup,sup->expiry,
            Biddy_Managed_GetTopVariableName(MNG,sup),
            Biddy_Managed_GetTopVariableName(MNG,sup->f),
            Biddy_Managed_GetTopVariableName(MNG,sup->t));
        } else {
          printf("NODE: i=%u, pointer=%p, expiry=%d, variable %u, else=%s, then=%s\n",
            i, (void *) sup,sup->expiry,
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
  Biddy_Boolean ok;
#ifdef FUNCTIONREPORTVERBOSE
  unsigned int num;
  BiddyOrderingTable *fOrdering;
  Biddy_Variable k;
  Biddy_Boolean stop;
  Biddy_Variable minvar,maxvar,minvarOrder,maxvarOrder;
#endif

#ifdef FUNCTIONREPORTVERBOSE
  printf("REPORT ON FUNCTION\n");
#endif

  if (Biddy_IsNull(f)) {
#ifdef FUNCTIONREPORTVERBOSE
    printf("FUNCTION IS NULL\n");
#endif
    return;
  }

  ok = checkFunctionOrdering(MNG,f);
  if (!ok) {
    printf("FUNCTION: wrong ordering!\n");
  }

#ifdef FUNCTIONREPORTVERBOSE
  /* determine function ordering and node number */
  if (!(fOrdering = (BiddyOrderingTable *)malloc(sizeof(BiddyOrderingTable)))) {
    fprintf(stderr, "BiddyFunctionReport: Out of memoy!\n");
    exit(1);
  }
  for (k=1; k<Biddy_Managed_VariableTableNum(MNG); k++) {
    biddyVariableTable.table[k].numone = 0;
  }
  nullOrdering(*fOrdering);
  num = 1;
  nodeNumberOrdering(MNG,f,&num,*fOrdering);
  Biddy_Managed_DeselectAll(MNG);
  printf("FUNCTION: number of nodes: %d, expiry value of top node: %d/%d\n",num,
          ((BiddyNode *) BiddyP(f))->expiry,biddySystemAge);

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
  free(fOrdering);
#endif

#ifdef FUNCTIONREPORTDETAILS
  printf("FUNCTION: here is BDD:\n");
  writeBDD(f);
#endif

}

/*----------------------------------------------------------------------------*/
/* Definition of static functions                                             */
/*----------------------------------------------------------------------------*/

/*******************************************************************************
\brief Function checkFunctionOrdering.

### Description
    Check if the given function has correct orderring.
### Side effects
    Function will select all nodes except constant node.
### More info
*******************************************************************************/

static Biddy_Boolean
checkFunctionOrderingR(Biddy_Manager MNG, Biddy_Edge f)
{
  if (Biddy_IsConstant(f) || Biddy_Managed_IsSelected(MNG,f)) return TRUE;
  Biddy_Managed_SelectNode(MNG,f);
  return
    Biddy_Managed_IsSmaller(MNG,BiddyV(f),BiddyV(BiddyE(f))) &&
    Biddy_Managed_IsSmaller(MNG,BiddyV(f),BiddyV(BiddyT(f))) &&
    checkFunctionOrderingR(MNG,BiddyE(f)) &&
    checkFunctionOrderingR(MNG,BiddyT(f));
}

static Biddy_Boolean
checkFunctionOrdering(Biddy_Manager MNG, Biddy_Edge f)
{
  Biddy_Boolean r;

  r = checkFunctionOrderingR(MNG,f);
  Biddy_Managed_DeselectAll(MNG);
  return r;
}

/*******************************************************************************
\brief Function isEqv.

### Description
    It is assumed that f1 and f2 have the same ordering.
### Side effects
### More info
*******************************************************************************/

static Biddy_Boolean
isEqv(Biddy_Manager MNG1, Biddy_Edge f1, Biddy_Manager MNG2, Biddy_Edge f2)
{
  Biddy_Boolean r;

  if (Biddy_IsConstant(f1)) {
    r = Biddy_IsConstant(f2) &&
        (Biddy_GetMark(f1) == Biddy_GetMark(f2)) &&
        !strcmp(
          Biddy_Managed_GetVariableName(MNG1,Biddy_GetTag(f1)),
          Biddy_Managed_GetVariableName(MNG2,Biddy_GetTag(f2)));
  } else {
    r = !Biddy_IsConstant(f2) &&
        (Biddy_GetMark(f1) == Biddy_GetMark(f2)) &&
        !strcmp(
          Biddy_Managed_GetTopVariableName(MNG1,f1),
          Biddy_Managed_GetTopVariableName(MNG2,f2)) &&
        !strcmp(
          Biddy_Managed_GetVariableName(MNG1,Biddy_GetTag(f1)),
          Biddy_Managed_GetVariableName(MNG2,Biddy_GetTag(f2))) &&
        isEqv(MNG1,BiddyE(f1),MNG2,BiddyE(f2)) &&
        isEqv(MNG1,BiddyT(f1),MNG2,BiddyT(f2));
  }

  return r;
}

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

  /* this is very simple but quite efficient, using v */
  /*
  {
  uintptr_t k;
  k = ((uintptr_t) pt >> 6) + ((uintptr_t) pf >> 2) + (uintptr_t) v;
  hash = 1 + (k & size);
  return hash;
  }
  */

#if UINTPTR_MAX == 0xffffffffffffffff

  /* this is used to experimentally determine parameters */
  /* for i in `seq 10`; do ./biddy-example-8queens 8; done */
  /*
  {
  uintptr_t k;
  static unsigned int GG=0;
  static unsigned int rnd1=1214272557,rnd2=1422347099;
  if (!GG) {GG=1;rnd1=rand();rnd2=rand();printf("<rnd1=%u><rnd2=%u>\n",rnd1,rnd2);}
  k = ((uintptr_t) pt * rnd1 + (uintptr_t) pf + (uintptr_t) v) * rnd2;
  hash = 1 + ((k >> 24) & size);
  return hash;
  }
  */

  /* this is experimentally determined, not using v */
  /**/
  {
  uintptr_t k;
  k = ((uintptr_t) pt * 352241492 + (uintptr_t) pf) * 231629123;
  hash = 1 + ((k >> 32) & size);
  return hash;
  }
  /**/

  /* this is experimentally determined, using v */
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

  if (cache.disabled) return;

  p = &cache.table[index];

#ifdef BIDDYEXTENDEDSTATS_YES
  if (!Biddy_IsNull(p->result)) {
    /* THE CELL IS NOT EMPTY, THUS THIS IS OVERWRITING */
    (*cache.overwrite)++;
  }
#endif

#ifdef BIDDYEXTENDEDSTATS_YES
  (*cache.insert)++;
#endif

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
  Biddy_Boolean q;
  BiddyOp3Cache *p;

  if (cache.disabled) return FALSE;

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
\brief Function OPGarbage performs Garbage Collection for the OP cache.

### Description
### Side effects
### More info
*******************************************************************************/

static void
OPGarbage(Biddy_Manager MNG)
{
  unsigned int j;
  BiddyOp3Cache *c;

  if (biddyOPCache.disabled) return;

  for (j=0; j<=biddyOPCache.size; j++) {
    if (!Biddy_IsNull(biddyOPCache.table[j].result)) {
      c = &biddyOPCache.table[j];
      if (((BiddyN(c->f)->expiry) && ((BiddyN(c->f)->expiry) < biddySystemAge)) ||
          ((BiddyN(c->g)->expiry) && ((BiddyN(c->g)->expiry) < biddySystemAge)) ||
          ((BiddyN(c->h)->expiry) && ((BiddyN(c->h)->expiry) < biddySystemAge)) ||
          ((BiddyN(c->result)->expiry) && ((BiddyN(c->result)->expiry) < biddySystemAge)))
      {
        c->result = biddyNull;
      }
    }
  }
}

/*******************************************************************************
\brief Function OPGarbageDeleteAll deletes all entries in the OP cache.

### Description
### Side effects
### More info
*******************************************************************************/

static void
OPGarbageDeleteAll(Biddy_Manager MNG)
{
  unsigned int j;
  BiddyOp3Cache *c;

  if (biddyOPCache.disabled) return;

  for (j=0; j<=biddyOPCache.size; j++) {
    c = &biddyOPCache.table[j];
    c->result = biddyNull;
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

  if (biddyEACache.disabled) return;

  for (j=0; j<=biddyEACache.size; j++) {
    if (!Biddy_IsNull(biddyEACache.table[j].result)) {
      c = &biddyEACache.table[j];
      if (((BiddyN(c->f)->expiry) && ((BiddyN(c->f)->expiry) < biddySystemAge)) ||
          ((BiddyN(c->g)->expiry) && ((BiddyN(c->g)->expiry) < biddySystemAge)) ||
          ((BiddyN(c->h)->expiry) && ((BiddyN(c->h)->expiry) < biddySystemAge)) ||
          ((BiddyN(c->result)->expiry) && ((BiddyN(c->result)->expiry) < biddySystemAge)))
      {
        c->result = biddyNull;
      }
    }
  }
}

/*******************************************************************************
\brief Function EAGarbageDeleteAll deletes all entries in the EA cache.

### Description
### Side effects
### More info
*******************************************************************************/

static void
EAGarbageDeleteAll(Biddy_Manager MNG)
{
  unsigned int j;
  BiddyOp3Cache *c;

  if (biddyEACache.disabled) return;

  for (j=0; j<=biddyEACache.size; j++) {
    c = &biddyEACache.table[j];
    c->result = biddyNull;
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

  if (biddyRCCache.disabled) return;

  for (j=0; j<=biddyRCCache.size; j++) {
    if (!Biddy_IsNull(biddyRCCache.table[j].result)) {
      c = &biddyRCCache.table[j];
      if (((BiddyN(c->f)->expiry) && ((BiddyN(c->f)->expiry) < biddySystemAge)) ||
          ((BiddyN(c->g)->expiry) && ((BiddyN(c->g)->expiry) < biddySystemAge)) ||
          ((BiddyN(c->h)->expiry) && ((BiddyN(c->h)->expiry) < biddySystemAge)) ||
          ((BiddyN(c->result)->expiry) && ((BiddyN(c->result)->expiry) < biddySystemAge)))
      {
        c->result = biddyNull;
      }
    }
  }
}

/*******************************************************************************
\brief Function RCGarbageDeleteAll deletes all entries in the RC cache.

### Description
### Side effects
### More info
*******************************************************************************/

static void
RCGarbageDeleteAll(Biddy_Manager MNG)
{
  unsigned int j;
  BiddyOp3Cache *c;

  if (biddyRCCache.disabled) return;

  for (j=0; j<=biddyRCCache.size; j++) {
    c = &biddyRCCache.table[j];
    c->result = biddyNull;
  }
}

/*******************************************************************************
\brief Function addVariableElement adds new variable and new element.

### Description
    Function returns new variable.
### Side effects
    This will always create a new variable/element. It is not checked that
    the same variable/element does not already exist. The ordering of the new
    variable is determined in Biddy_InitMNG. Variable and element are prolonged.
### More info
*******************************************************************************/

static Biddy_Edge
addVariableElement(Biddy_Manager MNG, Biddy_String x)
{
  Biddy_Variable v;
  Biddy_Edge r,el;
  
  /* DEBUGGING */
  /*
  printf("addVariableElement: name = %s\n",x);
  */

  if (biddyVariableTable.num == biddyVariableTable.size) {
    fprintf(stderr,"\nERROR (addVariableElement): Size exceeded!\n");
    fprintf(stderr,"Currently, there exist %d variables.\n",biddyVariableTable.num);
    exit(1);
  }
  v = biddyVariableTable.num++;

  if (biddyVariableTable.table[0].prev != biddyVariableTable.size) {
    biddyVariableTable.table[biddyVariableTable.table[0].prev].next = v;
  }
  biddyVariableTable.table[v].prev = biddyVariableTable.table[0].prev;
  biddyVariableTable.table[0].prev = v;
  biddyVariableTable.table[v].next = 0;

  /* LIST OF LIVE NODES FOR ANY [v] MUST NEVER BE EMPTY */
  /* FIRST ELEMENT OF LIST OF LIVE NODES IS [v]->firstNode */
  /* LAST ELEMENT OF LIST OF LIVE NODES IS [v]->lastNode */
  /* YOU MUST NEVER ASSUME THAT [v]->lastNode->list == NULL */
  /* THE NODE CREATED HERE IS ALWAYS FIRST IN THE LIST OF LIVE NODES */
  /* CONSTANT NODE biddyNodeTable.table[0] IS USED HERE AS TMP NODE, ONLY */

  biddyVariableTable.table[v].name = strdup(x);
  biddyVariableTable.table[v].num = 0;
  biddyVariableTable.table[v].firstNode = biddyNodeTable.table[0];
  biddyVariableTable.table[v].lastNode = biddyNodeTable.table[0];
  r = BiddyManagedFoaNode(MNG,v,biddyNull,biddyNull,FALSE); /* FALSE is important! */
  biddyVariableTable.table[v].firstNode = BiddyN(r);
  biddyNodeTable.table[0]->list = NULL; /* REPAIR CONSTANT NODE */

  /* NOW YOU HAVE A VARIABLE BUT AN ELEMENT IS ALSO CREATED */
  BiddyN(r)->expiry = 0; /* result is fortified */
  biddyVariableTable.table[v].variable = r;
  el = biddyZero; /* elements are not supported, yet */
  biddyVariableTable.table[v].element = el;

  return r;
}

/*******************************************************************************
\brief Function swapWithHigher determines higher variable and calls swapping.

### Description
    Higher (greater) variable is the bottommore one!
    The highest element is constant "1".
    Constant '1' has global ordering numUsedVariables (not smaller than anyone).
    Global ordering is the number of zeros in corresponding line of orderingTable.
### Side effects
### More info
*******************************************************************************/

static Biddy_Variable
swapWithHigher(Biddy_Manager MNG, Biddy_Variable v, Biddy_Boolean *active)
{
  Biddy_Variable vhigh;

  if (v == 0) {
    vhigh = 0; /* constant cannot be swapped */
  } else {
    vhigh = biddyVariableTable.table[v].next;
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
    Lower (smaller) variable is the topmore one!
    The lowest (topmost) element is not fixed.
    Topmost variable has global ordering 1 (smaller than all except itself).
    Global ordering is the number of zeros in corresponding line of orderingTable.
### Side effects
### More info
*******************************************************************************/

static Biddy_Variable
swapWithLower(Biddy_Manager MNG, Biddy_Variable v, Biddy_Boolean *active)
{
  Biddy_Variable vlow;

  if (v == 0) {
    vlow = 0; /* constant cannot be swapped */
  } else {
    vlow = biddyVariableTable.table[v].prev;
    if (vlow == biddyVariableTable.size) vlow = 0;
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

/***************************************************************************//*!
\brief Function isSmaller returns TRUE if the first variable is
       smaller (= lower = previous = above = topmore) in a custom
       ordering table.

### Description
### Side effects
    NOT IMPLEMENTED
### More info
*******************************************************************************/

static Biddy_Boolean
isSmaller(BiddyOrderingTable *orderingTable, Biddy_Variable fv, Biddy_Variable gv)
{
  return FALSE;
}

/*******************************************************************************
\brief Function prevVariable returns the first lower variable in a custom
       ordering table.

### Description
    Lower (smaller) variable is the topmore one!
    The lowest (topmost) element is not fixed.
### Side effects
### More info
*******************************************************************************/

static Biddy_Variable
prevVariable(BiddyOrderingTable *orderingTable, Biddy_Variable num, Biddy_Variable v)
{
  Biddy_Variable vlow;
  Biddy_Variable i;
  Biddy_Boolean first;

  vlow = 0;
  first = TRUE;
  for (i=1; i<num; i++) {
    if (first) {
      if (isSmaller(orderingTable,i,v)) {
        vlow = i;
        first = FALSE;
      }
    } else {
      if (isSmaller(orderingTable,i,v) &&
          isSmaller(orderingTable,vlow,i)) vlow = i;
    }
  }

  return vlow;
}

/*******************************************************************************
\brief Function nextVariable returns the first higher variable in a custom
       ordering table.

### Description
    Higher (greater) variable is the bottommore one!
    The highest element is constant "1".
### Side effects
### More info
*******************************************************************************/

static Biddy_Variable
nextVariable(BiddyOrderingTable *orderingTable, Biddy_Variable num, Biddy_Variable v)
{
  Biddy_Variable i;
  Biddy_Variable vhigh;

  vhigh = 0;
  for (i=1; i<num; i++) {
    if (isSmaller(orderingTable,v,i) &&
        isSmaller(orderingTable,i,vhigh)) vhigh = i;
  }

  return vhigh;
}

/*******************************************************************************
\brief Function swapVariables swaps two variables in a BDD system.

### Description
    If variable low is above (above = topmore) variable high then these two
    variables are swapped. Argument active is set to TRUE if some variables
    have been swapped.
### Side effects
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

  assert( biddyVariableTable.table[low].next == high );
  assert( biddyVariableTable.table[high].prev == low );

  biddyNodeTable.swap++;

  /* DEBUGGING */
  /*
  printf("SWAPPING START: low =  %s, high = %s",
         Biddy_Managed_GetVariableName(MNG,low),
         Biddy_Managed_GetVariableName(MNG,high));
  BiddySystemReport(MNG);
  */

  /* update active ordering */
  if (biddyVariableTable.table[low].prev != biddyVariableTable.size) {
    biddyVariableTable.table[biddyVariableTable.table[low].prev].next = high;
  }
  biddyVariableTable.table[biddyVariableTable.table[high].next].prev = low;
  biddyVariableTable.table[low].next = biddyVariableTable.table[high].next;
  biddyVariableTable.table[high].prev = biddyVariableTable.table[low].prev;
  biddyVariableTable.table[low].prev = high;
  biddyVariableTable.table[high].next = low;
  CLEAR_ORDER(biddyOrderingTable,low,high);
  SET_ORDER(biddyOrderingTable,high,low);

  sup = newBegin = biddyVariableTable.table[low].firstNode;
  newEnd = NULL;

  /* PREPARE NEW LIST FOR 'LOW' */
  /* CONSTANT NODE "1" IS USED HERE BUT ONLY AS TMP */
  biddyVariableTable.table[low].lastNode->list = NULL;
  biddyVariableTable.table[low].firstNode =
    biddyVariableTable.table[low].lastNode = biddyNodeTable.table[0];

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
        ((BiddyNode *) BiddyP(sup->f))->expiry = 1;
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
        ((BiddyNode *) sup->t)->expiry = 1;
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
        u0 = BiddyManagedFoaNode(MNG,low,f00,f10,FALSE);

/* THIS IS NOT NECESSARY BUT MAYBE IT IS USEFUL */
/**/
        assert( ((BiddyNode *) BiddyP(u0))->v == low );
        BiddyProlongOne(MNG,u0,sup->expiry);
/**/
      }

      if (f01 == f11) {
        u1 = f01;
      } else {
        /* u1 is a new node or an existing node */
        /* in any case, it is a needed node */
        /* here, GC will not be called during FoaNode */
        u1 = BiddyManagedFoaNode(MNG,low,f01,f11,FALSE);

/* THIS IS NOT NECESSARY BUT MAYBE IT IS USEFUL */
/**/
        assert( ((BiddyNode *) BiddyP(u1))->v == low );
        BiddyProlongOne(MNG,u1,sup->expiry);
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

      sup = (BiddyNode *) sup->list;

    } else {

      assert( BiddyV(sup) == low );

      /* this node with variable 'low' has not been changed to 'high' */
      /* because it does not have successors with variable 'high' */

      /* do not add this node to the list of renamed nodes (newBegin,newEnd) */
      if (!newEnd) {
        newBegin = (BiddyNode *) sup->list;
      } else {
        newEnd->list = sup->list;
      }
      /* relink to the end of list for low */
      /* first node added to the new list for low is always a node representing variable */
      /* it is added to new list before any other node will be added */

      biddyVariableTable.table[low].lastNode->list = (void *) sup;
      biddyVariableTable.table[low].lastNode = sup;

      sup = (BiddyNode *) sup->list;
    }

  }

  /* all nodes in the list from newBegin to newEnd (inclusively) are renamed */
  /* thus insert the complete list into list for 'high' */
  if (newEnd) {
    (biddyVariableTable.table[high].lastNode)->list = (void *) newBegin;
    biddyVariableTable.table[high].lastNode = newEnd;
  }

  /* at least one new node has been added to new list for 'low' */
  /* e.g. node representing variable is never renamed to 'high' */

  /* please note, that the first element is still a temporary one */
  biddyVariableTable.table[low].firstNode =
    (BiddyNode *) (biddyVariableTable.table[low].firstNode)->list;

  /* REPAIR CONSTANT NODE */
  biddyNodeTable.table[0]->list = NULL;

  /* DEBUGGING */
  /*
  printf("SWAPPING COMPLETE");
  BiddySystemReport(MNG);
  */

  /* REPAIR BROKEN NODES */
  /* this is need only if swaps have been made */
  /* FORMULAE CAN BE REPAIRED BECAUSE THEY ARE ALL COLLECTED IN FORMULA TABLE */
  /* FRESH NODES FROM CURRENT CALCULATION ARE NOT COLLECTED AND THUS THEY CANNOT BE REPAIRED! */
  if (*active) {

    for (n = 0; n < biddyFormulaTable.size; n++) {
      if (!Biddy_IsNull(biddyFormulaTable.table[n].f)) {
        BiddyProlongOne(MNG,biddyFormulaTable.table[n].f,biddyFormulaTable.table[n].expiry);
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
    c->back = (BiddyNode *)BiddyP(f);
    c->org = (BiddyNode *)(((BiddyNode *) BiddyP(f))->list);
    ((BiddyNode *) BiddyP(f))->list = (void *) c;
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
    ((BiddyNode *) BiddyP(f))->list = (void *)
      ((BiddyLocalInfo *)((BiddyNode *) BiddyP(f))->list)->org;
    deleteLocalInfo(MNG,BiddyE(f));
    deleteLocalInfo(MNG,BiddyT(f));
  }
}

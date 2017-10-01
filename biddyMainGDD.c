/***************************************************************************//*!
\file biddyMainGDD.c
\brief File biddyMainGDD.c contains main functions for representation and
manipulation of boolean functions with various types of Binary Decision
Diagrams (GDD = general decision diagrams).

### Description

    PackageName [Biddy]
    Synopsis    [Biddy provides data structures and algorithms for the
                 representation and manipulation of Boolean functions with
                 ROBDDs, 0-sup-BDDs, and TZBDDs. A hash table is used for quick
             		 search of nodes. Complement edges decreases the number of
		             nodes. An automatic garbage collection with a system age is
                 implemented. Variable swapping and sifting are implemented.]

    FileName    [biddyMainGDD.c]
    Revision    [$Revision: 319 $]
    Date        [$Date: 2017-09-30 22:37:26 +0200 (sob, 30 sep 2017) $]
    Authors     [Robert Meolic (robert.meolic@um.si)]

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


/* Biddy_Boolean biddySiftingActive = FALSE; */ /* debugging, only */

/*----------------------------------------------------------------------------*/
/* Static function prototypes                                                 */
/*----------------------------------------------------------------------------*/

static Biddy_Boolean checkFunctionOrdering(Biddy_Manager MNG, Biddy_Edge f);

static Biddy_Boolean isEqv(Biddy_Manager MNG1, Biddy_Edge f1, Biddy_Manager MNG2, Biddy_Edge f2);

static inline unsigned int nodeTableHash(Biddy_Variable v, Biddy_Edge pf, Biddy_Edge pt, unsigned int size);

static inline unsigned int op3Hash(Biddy_Edge a, Biddy_Edge b, Biddy_Edge c, unsigned int size);

static inline unsigned int keywordHash(Biddy_Edge a, unsigned int k, unsigned int size);

static void exchangeEdges(Biddy_Edge *f, Biddy_Edge *g);

static void complExchangeEdges(Biddy_Edge *f, Biddy_Edge *g);

static inline void addNodeTable(Biddy_Manager MNG, unsigned int hash, BiddyNode *node, BiddyNode *sup1);

static inline BiddyNode *findNodeTable(Biddy_Manager MNG, Biddy_Variable v, Biddy_Edge pf, Biddy_Edge pt, BiddyNode **thesup);

static inline void addOp3Cache(Biddy_Manager MNG, BiddyOp3CacheTable cache, Biddy_Edge a, Biddy_Edge b, Biddy_Edge c, Biddy_Edge r, unsigned int index);

static inline Biddy_Boolean findOp3Cache(Biddy_Manager MNG, BiddyOp3CacheTable cache, Biddy_Edge a, Biddy_Edge b, Biddy_Edge c, Biddy_Edge *r, unsigned int *index);

static inline void addKeywordCache(Biddy_Manager MNG, BiddyKeywordCacheTable cache, Biddy_Edge a, unsigned int k, Biddy_Edge r, unsigned int index);

static inline Biddy_Boolean findKeywordCache(Biddy_Manager MNG, BiddyKeywordCacheTable cache, Biddy_Edge a, unsigned int k, Biddy_Edge *r, unsigned int *index);

static void OPGarbage(Biddy_Manager MNG);

static void OPGarbageNewVariable(Biddy_Manager MNG, Biddy_Variable v);

static void OPGarbageDeleteAll(Biddy_Manager MNG);

static void EAGarbage(Biddy_Manager MNG);

static void EAGarbageNewVariable(Biddy_Manager MNG, Biddy_Variable v);

static void EAGarbageDeleteAll(Biddy_Manager MNG);

static void RCGarbage(Biddy_Manager MNG);

static void RCGarbageNewVariable(Biddy_Manager MNG, Biddy_Variable v);

static void RCGarbageDeleteAll(Biddy_Manager MNG);

static void ReplaceGarbage(Biddy_Manager MNG);

static void ReplaceGarbageNewVariable(Biddy_Manager MNG, Biddy_Variable v);

static void ReplaceGarbageDeleteAll(Biddy_Manager MNG);

static Biddy_Edge addVariableElement(Biddy_Manager MNG, Biddy_String x);

static Biddy_Variable swapWithHigher(Biddy_Manager MNG, Biddy_Variable v, Biddy_Boolean *active);

static Biddy_Variable swapWithLower(Biddy_Manager MNG, Biddy_Variable v, Biddy_Boolean *active);

static Biddy_Boolean isSmaller(BiddyOrderingTable *orderingTable, Biddy_Variable fv, Biddy_Variable gv);

static Biddy_Variable prevVariable(BiddyOrderingTable *orderingTable, Biddy_Variable num, Biddy_Variable v);

static Biddy_Variable nextVariable(BiddyOrderingTable *orderingTable, Biddy_Variable num, Biddy_Variable v);

static void swapVariables(Biddy_Manager MNG, Biddy_Variable low, Biddy_Variable high, Biddy_Boolean *active);

static void oneSwap(Biddy_Manager MNG, BiddyNode *sup, Biddy_Variable low, Biddy_Variable high, Biddy_Edge *u0, Biddy_Edge *u1);

static void oneSwapEdge(Biddy_Manager MNG, Biddy_Edge sup, Biddy_Variable low, Biddy_Variable high, Biddy_Edge *u, Biddy_Boolean *active);
            
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
    formula table (biddyFormulaTable), four basic caches (biddyOPCache,
    biddyEACache, biddyRCCache, and biddyReplaceCache), and cache list
    (biddyCacheList). Biddy_InitMNG also initializes constant edges (biddyOne,
    biddyZero), memory management and automatic garbage collection.
### Side effects
    Allocates a lot of memory.
### More info
    Macro Biddy_InitAnonymous() will initialize anonymous manager.
    Macro Biddy_Init() will initialize anonymous manager for ROBDDs.
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

  /* DEBUGGING */
  /*
  printf("DEBUG (Biddy_InitMNG): Biddy_InitMNG STARTED\n");
  printf("DEBUG (Biddy_InitMNG): mng = %p\n",mng);
  printf("DEBUG (Biddy_InitMNG): gddtype = %d\n",gddtype);
  printf("DEBUG (Biddy_InitMNG): MNG = %p\n",MNG);
  */

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
  if (!(MNG[11] = (BiddyKeywordCacheTable *) malloc(sizeof(BiddyKeywordCacheTable)))) {
    fprintf(stderr,"Biddy_InitMNG: Out of memoy!\n");
    exit(1);
  }
  biddyReplaceCache.table = NULL;
  biddyReplaceCache.size = 0;
  biddyReplaceCache.keywordList = NULL;
  biddyReplaceCache.keyList = NULL;
  biddyReplaceCache.keywordNum = 0;
  biddyReplaceCache.disabled = FALSE;
  if (!(biddyReplaceCache.search = (unsigned long long int *) malloc(sizeof(unsigned long long int)))) {
    fprintf(stderr, "Biddy_InitMNG: Out of memoy!\n");
    exit(1);
  }
  if (!(biddyReplaceCache.find = (unsigned long long int *) malloc(sizeof(unsigned long long int)))) {
    fprintf(stderr, "Biddy_InitMNG: Out of memoy!\n");
    exit(1);
  }
#ifdef BIDDYEXTENDEDSTATS_YES
  if (!(biddyReplaceCache.insert = (unsigned long long int *) malloc(sizeof(unsigned long long int)))) {
    fprintf(stderr, "Biddy_InitMNG: Out of memoy!\n");
    exit(1);
  }
  if (!(biddyReplaceCache.overwrite = (unsigned long long int *) malloc(sizeof(unsigned long long int)))) {
    fprintf(stderr, "Biddy_InitMNG: Out of memoy!\n");
    exit(1);
  }
#endif
  *(biddyReplaceCache.search) = 0;
  *(biddyReplaceCache.find) = 0;
#ifdef BIDDYEXTENDEDSTATS_YES
  *(biddyReplaceCache.insert) = 0;
  *(biddyReplaceCache.overwrite) = 0;
#endif
  if (!(MNG[12] = (BiddyCacheList* *) malloc(sizeof(BiddyCacheList*)))) {
    fprintf(stderr,"Biddy_InitMNG: Out of memoy!\n");
    exit(1);
  }
  biddyCacheList = NULL;
  if (!(MNG[13] = (BiddyNode* *) malloc(sizeof(BiddyNode*)))) {
    fprintf(stderr,"Biddy_InitMNG: Out of memoy!\n");
    exit(1);
  }
  biddyFreeNodes = NULL;
  if (!(MNG[14] = (BiddyOrderingTable *) malloc(sizeof(BiddyOrderingTable)))) {
    fprintf(stderr,"Biddy_InitMNG: Out of memoy!\n");
    exit(1);
  }
  if (!(MNG[15] = (unsigned int *) malloc(sizeof(unsigned int)))) {
    fprintf(stderr,"Biddy_InitMNG: Out of memoy!\n");
    exit(1);
  }
  biddySystemAge = 0;
  if (!(MNG[16] = (unsigned short int *) malloc(sizeof(unsigned short int)))) {
    fprintf(stderr,"Biddy_InitMNG: Out of memoy!\n");
    exit(1);
  }
  biddySelect = 0;

  /* INITIALIZATION OF MANAGER'S STRUCTURES  - VALUES ARE NOW INITIALIZED */
  if (gddtype == BIDDYTYPEOBDD) {
    MNG[0] = strdup(BIDDYTYPENAMEOBDD); /* biddyManagerName */
  } else if (gddtype == BIDDYTYPEOBDDC) {
    MNG[0] = strdup(BIDDYTYPENAMEOBDDC); /* biddyManagerName */
  } else if (gddtype == BIDDYTYPEZBDD) {
    MNG[0] = strdup(BIDDYTYPENAMEZBDD); /* biddyManagerName */
    fprintf(stderr,"Biddy_InitMNG: %s is not supported, yet!\n",BIDDYTYPENAMEZBDD);
    exit(1);
  } else if (gddtype == BIDDYTYPEZBDDC) {
    MNG[0] = strdup(BIDDYTYPENAMEZBDDC); /* biddyManagerName */
  } else if (gddtype == BIDDYTYPETZBDD) {
#if UINTPTR_MAX == 0xffffffffffffffff
#else
    fprintf(stderr,"Biddy_InitMNG: TZBDDs are supported on 64-bit architecture, only!\n");
    exit(1);
#endif
    MNG[0] = strdup(BIDDYTYPENAMETZBDD); /* biddyManagerName */
  } else if (gddtype == BIDDYTYPETZBDDC) {
#if UINTPTR_MAX == 0xffffffffffffffff
#else
    fprintf(stderr,"Biddy_InitMNG: TZBDDs are supported on 64-bit architecture, only!\n");
    exit(1);
#endif
    MNG[0] = strdup(BIDDYTYPENAMETZBDDC); /* biddyManagerName */
    fprintf(stderr,"Biddy_InitMNG: %s is not supported, yet!\n",BIDDYTYPENAMETZBDDC);
    exit(1);
  } else if (gddtype == BIDDYTYPEOFDD) {
    MNG[0] = strdup(BIDDYTYPENAMEOFDD); /* biddyManagerName */
    fprintf(stderr,"Biddy_InitMNG: %s is not supported, yet!\n",BIDDYTYPENAMEOFDD);
    exit(1);
  } else if (gddtype == BIDDYTYPEOFDDC) {
    MNG[0] = strdup(BIDDYTYPENAMEOFDDC); /* biddyManagerName */
    fprintf(stderr,"Biddy_InitMNG: %s is not supported, yet!\n",BIDDYTYPENAMEOFDDC);
    exit(1);
  } else if (gddtype == BIDDYTYPEZFDD) {
    MNG[0] = strdup(BIDDYTYPENAMEZFDD); /* biddyManagerName */
    fprintf(stderr,"Biddy_InitMNG: %s is not supported, yet!\n",BIDDYTYPENAMEZFDD);
    exit(1);
  } else if (gddtype == BIDDYTYPEZFDDC) {
    MNG[0] = strdup(BIDDYTYPENAMEZFDDC); /* biddyManagerName */
    fprintf(stderr,"Biddy_InitMNG: %s is not supported, yet!\n",BIDDYTYPENAMEZFDDC);
    exit(1);
  } else if (gddtype == BIDDYTYPETZFDD) {
#if UINTPTR_MAX == 0xffffffffffffffff
#else
    fprintf(stderr,"Biddy_InitMNG: TZFDDs are supported on 64-bit architecture, only!\n");
    exit(1);
#endif
    MNG[0] = strdup(BIDDYTYPENAMETZFDD); /* biddyManagerName */
    fprintf(stderr,"Biddy_InitMNG: %s is not supported, yet!\n",BIDDYTYPENAMETZFDD);
    exit(1);
  } else if (gddtype == BIDDYTYPETZFDDC) {
#if UINTPTR_MAX == 0xffffffffffffffff
#else
    fprintf(stderr,"Biddy_InitMNG: TZFDDs are supported on 64-bit architecture, only!\n");
    exit(1);
#endif
    MNG[0] = strdup(BIDDYTYPENAMETZFDDC); /* biddyManagerName */
    fprintf(stderr,"Biddy_InitMNG: %s is not supported, yet!\n",BIDDYTYPENAMETZFDDC);
    exit(1);
  } else {
    fprintf(stderr,"Biddy_InitMNG: Unsupported GDD type!\n");
    exit(1);
  }

  biddyManagerType = gddtype;

  /* THESE ARE DEFAULT SIZES */
  biddyVariableTable.size = BIDDYVARMAX;
  biddyNodeTable.initblocksize = MEDIUM_SIZE; /* 12.0 MB on 64-bit */
  biddyNodeTable.limitblocksize = XLARGE_SIZE; /* 96.0 MB on 64-bit */
  biddyNodeTable.initsize = SMALL_SIZE;
  biddyNodeTable.limitsize = HUGE_SIZE;
  biddyOPCache.size = MEDIUM_SIZE;
  biddyEACache.size = SMALL_SIZE;
  biddyRCCache.size = SMALL_SIZE;
  biddyReplaceCache.size = SMALL_SIZE;

  /* TESTING */
  /*
  biddyOPCache.size = LARGE_SIZE;
  biddyEACache.size = MEDIUM_SIZE;
  biddyRCCache.size = MEDIUM_SIZE;
  biddyReplaceCache.size = MEDIUM_SIZE;
  */

  /* these values are experimentally determined */
  /* gcr=1.67, gcrF=1.20, gcrX=0.91, rr=0.01, rrF=1.45, rrX=0.98 */ /* used in v1.7.1 */
  /* gcr=1.32, gcrF=0.99, gcrX=1.10, rr=0.01, rrF=0.89, rrX=0.91 */ /* used in v1.7.2 */
  biddyNodeTable.gcratio = (float) 1.32; /* do not delete nodes if the effect is to small, gcratio */
  biddyNodeTable.gcratioF = (float) 0.99; /* do not delete nodes if the effect is to small, gcratio */
  biddyNodeTable.gcratioX = (float) 1.10; /* do not delete nodes if the effect is to small, gcratio */
  biddyNodeTable.resizeratio = (float) 0.01; /* resize Node table if there are to many nodes */
  biddyNodeTable.resizeratioF = (float) 0.89; /* resize Node table if there are to many nodes */
  biddyNodeTable.resizeratioX = (float) 0.91; /* resize Node table if there are to many nodes */

  /* these values are experimentally determined */
  biddyNodeTable.siftingtreshold = (float) 0.95;  /* stop sifting if the size of the system grows to much */
  biddyNodeTable.fsiftingtreshold = (float) 0.95; /* stop sifting if the size of the function grows to much */
  biddyNodeTable.convergesiftingtreshold = (float) 1.01;  /* stop one step of converging sifting if the size of the system grows to much */
  biddyNodeTable.fconvergesiftingtreshold = (float) 1.01; /* stop one step of converging sifting if the size of the function grows to much */

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
  /* ORDER FOR OBDDs AND OFDDs: [1] < [2] < ... < [size-1] < [0] */
  /* ORDER FOR ZBDDs, ZFDDs, TZBDDs, AND TZFDDs: [size-1] < [size-2] < ... < [1] < [0] */
  /* FOR OBDDs AND OFDDs: the topmost variable always has prev=biddyVariableTable.size */
  /* FOR ZBDDs, ZFDDs, TZBDDs, AND TZFDDs: the topmost variable always has prev=biddyVariableTable.num */
  biddyVariableTable.table = (BiddyVariable *)
                malloc(biddyVariableTable.size * sizeof(BiddyVariable));
  nullOrdering(biddyOrderingTable);
  biddyVariableTable.table[0].name = strdup("1");
  biddyVariableTable.table[0].variable = biddyNull;
  biddyVariableTable.table[0].element = biddyNull;
  biddyVariableTable.table[0].num = 1;
  if ((biddyManagerType == BIDDYTYPEOBDDC) || (biddyManagerType == BIDDYTYPEOBDD) ||
      (biddyManagerType == BIDDYTYPEOFDDC) || (biddyManagerType == BIDDYTYPEOFDD))
  {
    biddyVariableTable.table[0].prev = biddyVariableTable.size;
    biddyVariableTable.table[0].next = 0; /* fixed */
  }
  else if ((biddyManagerType == BIDDYTYPEZBDDC) || (biddyManagerType == BIDDYTYPEZBDD) ||
           (biddyManagerType == BIDDYTYPEZFDDC) || (biddyManagerType == BIDDYTYPEZFDD) ||
           (biddyManagerType == BIDDYTYPETZBDDC) || (biddyManagerType == BIDDYTYPETZBDD) ||
           (biddyManagerType == BIDDYTYPETZFDDC) || (biddyManagerType == BIDDYTYPETZFDD))
  {
    biddyVariableTable.table[0].prev = 1;
    biddyVariableTable.table[0].next = 0; /* fixed */
  }
  for (i=1; i<biddyVariableTable.size; i++) {
    biddyVariableTable.table[i].num = 0;
    biddyVariableTable.table[i].name = NULL;
    biddyVariableTable.table[i].variable = biddyNull;
    biddyVariableTable.table[i].element = biddyNull;
    biddyVariableTable.table[i].selected = FALSE;
    SET_ORDER(biddyOrderingTable,i,0);
    if ((biddyManagerType == BIDDYTYPEOBDDC) || (biddyManagerType == BIDDYTYPEOBDD) ||
        (biddyManagerType == BIDDYTYPEOFDDC) || (biddyManagerType == BIDDYTYPEOFDD))
    {
      biddyVariableTable.table[i].prev = i-1;
      biddyVariableTable.table[i].next = i+1;
      for (j=i+1; j<biddyVariableTable.size; j++) {
        SET_ORDER(biddyOrderingTable,i,j);
      }
    }
    else if ((biddyManagerType == BIDDYTYPEZBDDC) || (biddyManagerType == BIDDYTYPEZBDD) ||
             (biddyManagerType == BIDDYTYPEZFDDC) || (biddyManagerType == BIDDYTYPEZFDD) ||
             (biddyManagerType == BIDDYTYPETZBDDC) || (biddyManagerType == BIDDYTYPETZBDD) ||
             (biddyManagerType == BIDDYTYPETZFDDC) || (biddyManagerType == BIDDYTYPETZFDD))
    {
      biddyVariableTable.table[i].prev = i+1;
      biddyVariableTable.table[i].next = i-1;
      for (j=1; j<i; j++) {
        SET_ORDER(biddyOrderingTable,i,j);
      }
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
  /* CONSTANT NODE IS "1" FOR ALL BDD TYPES AND IT IS AT INDEX [0] */
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
  /* biddyZero IS COMPLEMENTED FOR ALL GDD TYPES, INCLUDING OBDD, ZBDD, AND TZBDD */
  MNG[2] = (void *) biddyNodeTable.table[0]; /* biddyTerminal */
  MNG[3] = Biddy_Complement(biddyTerminal); /* biddyZero */
  MNG[4] = biddyTerminal; /* biddyOne */

  /* DEBUGGING */
  /*
  printf("DEBUG (Biddy_InitMNG): biddyOne = %p\n",biddyOne);
  printf("DEBUG (Biddy_InitMNG): biddyZero = %p\n",biddyZero);
  */

  /* INITIALIZATION OF VARIABLE'S VALUES */
  biddyVariableTable.table[0].value = biddyZero;
  for (i=1; i<biddyVariableTable.size; i++) {
#pragma warning(suppress: 6386)
    biddyVariableTable.table[i].value = biddyZero;
  }

  /* INITIALIZATION OF NODE SELECTION */
  biddySelect = 1;

  /* INITIALIZATION OF GARBAGE COLLECTION */
  /* MINIMAL VALUE FOR biddySystemAge IS 2 (SEE IMPLEMENTATION OF SIFTING) */
  biddySystemAge = 2;

  /* INITIALIZE FORMULA TABLE */
  /* first formula is 0, it is always a single terminal node */
  /* second formula is 1, it could be a large graph, e.g. for ZBDD */
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
  biddyFormulaTable.table[1].expiry = biddySystemAge;
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

  /* INITIALIZATION OF DEFAULT REPLACE CACHE - USED FOR REPLACE */
  biddyReplaceCache.disabled = FALSE;
  biddyReplaceCache.keywordList = NULL;
  biddyReplaceCache.keyList = NULL;
  biddyReplaceCache.keywordNum = 0;
  *(biddyReplaceCache.search) = *(biddyReplaceCache.find) = 0;
#ifdef BIDDYEXTENDEDSTATS_YES
  *(biddyReplaceCache.insert) = *(biddyReplaceCache.overwrite) = 0;
#endif
  if (!(biddyReplaceCache.table = (BiddyKeywordCache *)
  calloc((biddyReplaceCache.size+1),sizeof(BiddyKeywordCache)))) {
    fprintf(stderr,"Biddy_InitMNG (Replace cache): Out of memoy!\n");
    exit(1);
  }
  /* for (i=0;i<=biddyReplaceCache.size;i++) biddyReplaceCache.table[i].result = biddyNull; */
  Biddy_Managed_AddCache(MNG,ReplaceGarbage);

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

  /*
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
  */

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
  printf("Delete Replace cache...\n");
  */
  if ((BiddyKeywordCacheTable*)(MNG[11])) {
    for (i = 0; i < biddyReplaceCache.keywordNum; i++) {
      free(biddyReplaceCache.keywordList[i]);
    }
    free(biddyReplaceCache.keywordList);
    free(biddyReplaceCache.keyList);
    free(biddyReplaceCache.table);
    free(biddyReplaceCache.search);
    free(biddyReplaceCache.find);
#ifdef BIDDYEXTENDEDSTATS_YES
    free(biddyReplaceCache.insert);
    free(biddyReplaceCache.overwrite);
#endif
#pragma warning(suppress: 6001)
    free((BiddyKeywordCacheTable*)(MNG[11]));
  }

  /*
  printf("Delete cache list...\n");
  */
  if ((BiddyCacheList**)(MNG[12])) {
#pragma warning(suppress: 6001)
    sup1 = biddyCacheList;
    while (sup1) {
      sup2 = sup1->next;
      free(sup1);
      sup1 = sup2;
    }
#pragma warning(suppress: 6001)
    free((BiddyCacheList**)(MNG[12]));
  }

  /*
  printf("Delete pointer biddyFreeNodes...\n");
  */
#pragma warning(suppress: 6001)
  free((BiddyNode**)(MNG[13]));

  /*
  printf("Delete Ordering table...\n");
  */
#pragma warning(suppress: 6001)
  free((BiddyOrderingTable*)(MNG[14]));

  /*
  printf("Delete age...\n");
  */
  free((unsigned int*)(MNG[15]));

  /*
  printf("Delete selector...\n");
  */
  free((unsigned short int*)(MNG[16]));

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

int
Biddy_Managed_GetManagerType(Biddy_Manager MNG)
{
  if (!MNG) MNG = biddyAnonymousManager;

  return biddyManagerType;
}

#ifdef __cplusplus
}
#endif

/***************************************************************************//*!
\brief Function Biddy_Managed_GetManagerName reports the name of the BDD type
        used in the manager.

### Description
### Side effects
### More info
    Macro Biddy_GetManagerName() is defined for use with anonymous manager.
*******************************************************************************/

#ifdef __cplusplus
extern "C" {
#endif

Biddy_String
Biddy_Managed_GetManagerName(Biddy_Manager MNG)
{
  if (!MNG) MNG = biddyAnonymousManager;

  return biddyManagerName;
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

  if (gcr > 0.0) biddyNodeTable.gcratio = gcr;
  if (gcrF > 0.0) biddyNodeTable.gcratioF = gcrF;
  if (gcrX > 0.0) biddyNodeTable.gcratioX = gcrX;
  if (rr > 0.0) biddyNodeTable.resizeratio = rr;
  if (rrF > 0.0) biddyNodeTable.resizeratioF = rrF;
  if (rrX > 0.0) biddyNodeTable.resizeratioX = rrX;
  if (st > 0.0) biddyNodeTable.siftingtreshold = st;
  if (fst > 0.0) biddyNodeTable.fsiftingtreshold = fst;
  if (cst > 0.0) biddyNodeTable.convergesiftingtreshold = cst;
  if (fcst > 0.0) biddyNodeTable.fconvergesiftingtreshold = fcst;

  /* PROFILING */
  /*
  fprintf(stderr,"gcr=%.2f, ",biddyNodeTable.gcratio);
  fprintf(stderr,"gcrF=%.2f, ",biddyNodeTable.gcratioF);
  fprintf(stderr,"gcrX=%.2f, ",biddyNodeTable.gcratioX);
  fprintf(stderr,"rr=%.2f, ",biddyNodeTable.resizeratio);
  fprintf(stderr,"rrF=%.2f, ",biddyNodeTable.resizeratioF);
  fprintf(stderr,"rrX=%.2f, ",biddyNodeTable.resizeratioX);
  */
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
\brief Function Biddy_Managed_GetTerminal returns unmarked and untagged
       edge pointing to the constant node 1.

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
    For ZBDDs and ZFDDs, you should always obtain constant 1 through the call
    of this function!
### More info
    Internally, use macro biddyOne (also for ZBDDs and ZFDDs!).
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
  Biddy_Variable v,top;

  if (!MNG) MNG = biddyAnonymousManager;

  if ((biddyManagerType == BIDDYTYPEOBDDC) || (biddyManagerType == BIDDYTYPEOBDD)) {
    /* IMPLEMENTED */
    r = biddyOne;
    for (v=1;v<biddyVariableTable.num;v++) {
      r = BiddyManagedITE(MNG,biddyVariableTable.table[v].variable,biddyZero,r);
    }
    BiddyRefresh(r);

  } else if ((biddyManagerType == BIDDYTYPEZBDDC) || (biddyManagerType == BIDDYTYPEZBDD)) {
    /* IMPLEMENTED */

    r = biddyTerminal;

  } else if ((biddyManagerType == BIDDYTYPETZBDDC) || (biddyManagerType == BIDDYTYPETZBDD)) {
    /* IMPLEMENTED */

    /* find topmost variable */
    top = 0;
    for (v = 1; v < biddyVariableTable.num; v++) {
      top = biddyVariableTable.table[top].prev;
    }
    r = biddyTerminal;
    Biddy_SetTag(r,top);

  } else if ((biddyManagerType == BIDDYTYPEOFDDC) || (biddyManagerType == BIDDYTYPEOFDD) ||
              (biddyManagerType == BIDDYTYPEZFDDC) || (biddyManagerType == BIDDYTYPEZFDD) ||
              (biddyManagerType == BIDDYTYPETZFDDC) || (biddyManagerType == BIDDYTYPETZFDD))
  {
    fprintf(stderr,"Biddy_GetBaseSet: this GDD type is not supported, yet!\n");
    return biddyNull;
  } else {
    fprintf(stderr,"Biddy_GetBaseSet: Unsupported GDD type!\n");
    return biddyNull;
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
  /**/

  /* TO DO: SEARCH FOR THE VARIABLE IN THE LOOKUP TABLE INSTEAD IN THE ORIGINAL TABLE */

  /* IF find THEN v IS THE INDEX OF THE CORRECT ELEMENT! */
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
  if (!MNG) MNG = biddyAnonymousManager;

  return biddyVariableTable.table[v].element;
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
    Macro BiddyIsSmaller(fv,gv) is defined for internal use.
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
    BDDs without external references are not repaired.
    For OBDDs, OFDDs, TZBDDS, and TZFDDs, it is safe to add new
    variables/elements if BDDs are used to represent Boolean functions.
    For ZBDDs and ZFDDs, it is safe to add new variables/elements if BDDs are
    used to represent combination sets.
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
  Biddy_Variable v,w;
  int cc;
  Biddy_Boolean isNumbered,find;
  Biddy_Edge result;

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

    /* SEARCH FOR THE EXISTING VARIABLE/ELEMENT/FORMULA WITH THE GIVEN NAME */
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

    /* SEARCH FOR THE VARIABLE/ELEMENT IN THE LOOKUP TABLE INSTEAD IN THE ORIGINAL TABLE */

  }

  /* IF find THEN v IS THE INDEX OF THE CORRECT VARIABLE/ELEMENT! */
  if (!find) {

    /* SEARCH FOR THE EXISTING FORMULA WITH THE SAME NAME */
    /* FORMULAE IN FORMULA TABLE ARE ORDERED BY NAME */
    /* TO DO: USE THE SAME METHOD AS IN FindFormula */
    for (i = 0; !find && (i < biddyFormulaTable.numOrdered); i++) {
      if (biddyFormulaTable.table[i].name) {
        cc = strcmp(x,biddyFormulaTable.table[i].name);
        if ((cc == 0) && !biddyFormulaTable.table[i].deleted) {
          find = TRUE;
          break;
        }
        if (cc < 0) break;
      } else {
        fprintf(stderr,"Biddy_Managed_FoaVariable: Problem with the formula table!\n");
        exit(1);
      }
    }

    if (find) {
      printf("WARNING (Biddy_Managed_FoaVariable): new variable/element %s has the same name as an existing formula!\n",x);
    }

    /* addVariableElement creates variable, prolongs results, and returns variable edge */
    result = addVariableElement(MNG,x);

    assert ( v == biddyVariableTable.num-1 );

    /* *** ADAPTING CACHE TABLES *** */

    /* FOR OBDDs AND OFDDs, CACHE DOES NOT NEED TO BE ADAPTED */

    /* FOR ZBDDs AND ZFDDs, CACHE DOES NOT NEED TO BE ADAPTED */

    /* FOR TZBDDs AND TZFDDs, ALL RESULTS IN CACHE WHERE TOP VARIABLE IS ABOVE THE NEW VARIABLE MAY BE WRONG! */
    if ((biddyManagerType == BIDDYTYPETZBDDC) || (biddyManagerType == BIDDYTYPETZBDD) ||
        (biddyManagerType == BIDDYTYPETZFDDC) || (biddyManagerType == BIDDYTYPETZFDD))
    {
      OPGarbageNewVariable(MNG,v);
      EAGarbageNewVariable(MNG,v);
      RCGarbageNewVariable(MNG,v);
      ReplaceGarbageNewVariable(MNG,v);
    }

    /* *** ADAPTING FORMULAE *** */

    /* FOR OBDDs AND OFDDs, FORMULAE MUST BE ADAPTED IF THEY REPRESENT COMBINATION SETS */
    /* biddyZero AND biddyOne ALWAYS REPRESENT A BOOLEAN FORMULA! */
    if ((biddyManagerType == BIDDYTYPEOBDDC) || (biddyManagerType == BIDDYTYPEOBDD) ||
        (biddyManagerType == BIDDYTYPEOFDDC) || (biddyManagerType == BIDDYTYPEOFDD)) {
      if (!varelem) {
        /* NOT IMPLEMENTED, YET */
      }
    }

    /* FOR ZBDDs AND ZFDDs, FORMULAE MUST BE ADAPTED IF THEY REPRESENT BOOLEAN FUNCTIONS */
    /* biddyZero AND biddyOne ALWAYS REPRESENT A BOOLEAN FORMULA! */
    /* first formula is 0, it is always a single terminal node */
    /* second formula is 1, it could be a large graph, e.g. for ZBDD */
    if ((biddyManagerType == BIDDYTYPEZBDDC) || (biddyManagerType == BIDDYTYPEZBDD) ||
        (biddyManagerType == BIDDYTYPEZFDDC) || (biddyManagerType == BIDDYTYPEZFDD)) {

      if (varelem) {

        for (i = 1; i < biddyFormulaTable.size; i++) {
          if (!biddyFormulaTable.table[i].deleted && (biddyFormulaTable.table[i].f != result)) {
            if (BiddyIsSmaller(BiddyV(biddyFormulaTable.table[i].f),v)) {
              /* NOT IMPLEMENTED, YET */
              printf("Biddy_Managed_FoaVariable: For ZBDDs, adding new variable below the existing ones is not supported, yet!\n");
              return 0;
            }
          }
        }

        for (i = 1; i < biddyFormulaTable.size; i++) {
          if (!biddyFormulaTable.table[i].deleted && (biddyFormulaTable.table[i].f != result)) {

            /* DEBUGGING */
            /*
            printf("Biddy_FoaVariable: updating formula %s\n",biddyFormulaTable.table[i].name);
            */

            /* formulae must be explicitly refreshed, here */
            biddyFormulaTable.table[i].f =
              BiddyManagedTaggedFoaNode(MNG,v,biddyFormulaTable.table[i].f,
                biddyFormulaTable.table[i].f,0,TRUE); /* FoaNode returns an obsolete node! */
            BiddyRefresh(biddyFormulaTable.table[i].f);

          }
        }

        /* MNG[3] = biddyFormulaTable.table[0].f; */ /* biddyZero does not need to be updated */
        MNG[4] = biddyFormulaTable.table[1].f; /* biddyOne must be updated, e.g. for ZBDD */

      } else {

        for (i = 0; i < 2; i++) {
          /* formulae must be explicitly refreshed, here */
          biddyFormulaTable.table[i].f =
            BiddyManagedTaggedFoaNode(MNG,v,biddyFormulaTable.table[i].f,
              biddyFormulaTable.table[i].f,0,TRUE); /* FoaNode returns an obsolete node! */
          BiddyRefresh(biddyFormulaTable.table[i].f);
        }

        MNG[3] = biddyFormulaTable.table[0].f; /* biddyZero must be updated */
        MNG[4] = biddyFormulaTable.table[1].f; /* biddyOne must be updated */

      }
    }

    /* FOR TZBDDs AND TZFDDs, FORMULAE MUST BE ADAPTED REGARDLESS OF WHAT THEY REPRESENT */
    /* biddyZero AND biddyOne ALWAYS REPRESENT A BOOLEAN FORMULA! */
    else if ((biddyManagerType == BIDDYTYPETZBDDC) || (biddyManagerType == BIDDYTYPETZBDD) ||
              (biddyManagerType == BIDDYTYPETZFDDC) || (biddyManagerType == BIDDYTYPETZFDD)) {

      if (varelem) {
        /* FOR BOOLEAN FUNCTIONS, ALL FORMULAE WHERE TOP TAG IS ABOVE THE NEW VARIABLE MUST BE ADAPTED */
        if (biddyNodeTable.num > 1) {
          for (i = 1; i < biddyFormulaTable.size; i++) {
            if (!biddyFormulaTable.table[i].deleted && (biddyFormulaTable.table[i].f != result)) {
              if (BiddyIsSmaller(Biddy_GetTag(biddyFormulaTable.table[i].f),v)) {
                printf("Biddy_Managed_FoaVariable: For TZBDDs, adding new variable below the existing ones is not supported, yet!\n");
                return 0;
              }
            }
          }
        }

      } else {
        /* FOR COMBINATION SETS, ... */
        /* NOT IMPLEMENTED, YET */
      }

    }

    /* *** ADAPTING VARIABLES AND ELEMENTS *** */

    /* FOR OBDDs AND OFDDs, ALL ELEMENTS MUST BE ADAPTED */
    if ((biddyManagerType == BIDDYTYPEOBDDC) || (biddyManagerType == BIDDYTYPEOBDD) ||
        (biddyManagerType == BIDDYTYPEOFDDC) || (biddyManagerType == BIDDYTYPEOFDD)) {
      /* NOT IMPLEMENTED, YET */
    }

    /* FOR ZBDDs AND ZFDDs, ALL VARIABLES MUST BE ADAPTED */
    if ((biddyManagerType == BIDDYTYPEZBDDC) || (biddyManagerType == BIDDYTYPEZBDD) ||
        (biddyManagerType == BIDDYTYPEZFDDC) || (biddyManagerType == BIDDYTYPEZFDD)) {
      for (w = 1; w < biddyVariableTable.num-1; w++) {

        /* DEBUGGING */
        /*
        printf("Biddy_FoaVariable: updating variable %s (w=%u)\n",Biddy_Managed_GetVariableName(MNG,w),w);
        printf("top node before updating: %s\n",Biddy_Managed_GetTopVariableName(MNG,biddyVariableTable.table[w].variable));
        */

        biddyVariableTable.table[w].variable =
          BiddyManagedTaggedFoaNode(MNG,v,biddyVariableTable.table[w].variable,
            biddyVariableTable.table[w].variable,0,TRUE); /* FoaNode returns an obsolete node! */
        BiddyRefresh(biddyVariableTable.table[w].variable);

        /* DEBUGGING */
        /*
        printf("top node of variable after updating: %s\n",Biddy_Managed_GetTopVariableName(MNG,biddyVariableTable.table[w].variable));
        */
      }
    }

    /* FOR TZBDDs AND TZFDDs, ELEMENTS MUST BE ADAPTED IF NEW VARIABLE IS ADDED ABOVE ALL THE EXISTING VARIABLES */
    if ((biddyManagerType == BIDDYTYPETZBDDC) || (biddyManagerType == BIDDYTYPETZBDD) ||
        (biddyManagerType == BIDDYTYPETZFDDC) || (biddyManagerType == BIDDYTYPETZFDD)) {
      /* NOT IMPLEMENTED, YET */
    }

  } else {

    /* assert( printf("WARNING (Biddy_Managed_FoaVariable): variable %s already exists\n",x) ); */

  }

  if (isNumbered) {
    free(x);
  }

  /* DEBUGGING */
  /**/
  if ((biddyManagerType == BIDDYTYPETZBDDC) || (biddyManagerType == BIDDYTYPETZBDD)) {
    if (biddyVariableTable.table[2].next != 1) printf("HEJ2, ERROR!\n");
  }
  /**/

  /* RETURN VARIABLE, WHICH HAS BEEN LOOKED FOR */
  return v;
}

#ifdef __cplusplus
}
#endif

/***************************************************************************//*!
\brief Function Biddy_Managed_ChangeVariableName set new name to the given
        variable/element.

### Description
### Side effects
    It is not checked that the same variable/element does not already exist.
    The ordering of the variable is not changed.
### More info
    Macro Biddy_ChangeVariableName(v,x) is defined for use with anonymous
    manager.
*******************************************************************************/

#ifdef __cplusplus
extern "C" {
#endif

void
Biddy_Managed_ChangeVariableName(Biddy_Manager MNG, Biddy_Variable v, Biddy_String x)
{
  if (!MNG) MNG = biddyAnonymousManager;

  if (v == 0) return; /* constant cannot be renamed */
  if (v >= biddyVariableTable.num) return; /* unexisting varible */

  free(biddyVariableTable.table[v].name);
  biddyVariableTable.table[v].name = strdup(x);
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
\brief Function Biddy_Managed_AddVariableBelow adds variable.

### Description
    Biddy_Managed_AddVariableBelow uses Biddy_Managed_AddVariableByName to add
    new variable. Then, the order of the new variable is changed to become
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

  if ((biddyManagerType == BIDDYTYPEOBDDC) || (biddyManagerType == BIDDYTYPEOBDD) ||
      (biddyManagerType == BIDDYTYPEOFDDC) || (biddyManagerType == BIDDYTYPEOFDD))
  {
    if (v == biddyVariableTable.table[0].prev) return f; /* variable shifting is not needed */
    biddyVariableTable.table[0].prev = biddyVariableTable.table[x].prev;
    biddyVariableTable.table[biddyVariableTable.table[x].prev].next = 0;
    biddyVariableTable.table[biddyVariableTable.table[v].next].prev = x;
    biddyVariableTable.table[x].next = biddyVariableTable.table[v].next;
    biddyVariableTable.table[x].prev = v;
    biddyVariableTable.table[v].next = x;
  }
  else if ((biddyManagerType == BIDDYTYPEZBDDC) || (biddyManagerType == BIDDYTYPEZBDD) ||
            (biddyManagerType == BIDDYTYPEZFDDC) || (biddyManagerType == BIDDYTYPEZFDD) ||
            (biddyManagerType == BIDDYTYPETZBDDC) || (biddyManagerType == BIDDYTYPETZBDD) ||
            (biddyManagerType == BIDDYTYPETZFDDC) || (biddyManagerType == BIDDYTYPETZFDD))
  {
    /* NOT IMPLEMENTED, YET */
    fprintf(stderr,"Biddy_AddVariableBelow:: this GDD type is not supported, yet!\n");
    return biddyNull;
  } else {
    fprintf(stderr,"Biddy_AddVariableBelow: Unsupported GDD type!\n");
    return biddyNull;
  }

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
\brief Function Biddy_Managed_AddVariableAbove adds variable.

### Description
    Biddy_Managed_AddVariableAbove uses Biddy_Managed_AddVariableByName to add
    new variable. Then, the order of the new variable is changed to become
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

  if ((biddyManagerType == BIDDYTYPEOBDDC) || (biddyManagerType == BIDDYTYPEOBDD) ||
      (biddyManagerType == BIDDYTYPEOFDDC) || (biddyManagerType == BIDDYTYPEOFDD))
  {
    if (v == 0) return f; /* variable shifting is not needed */
    biddyVariableTable.table[0].prev = biddyVariableTable.table[x].prev;
    biddyVariableTable.table[biddyVariableTable.table[x].prev].next = 0;
    if (biddyVariableTable.table[v].prev != biddyVariableTable.size) {
      biddyVariableTable.table[biddyVariableTable.table[v].prev].next = x;
    }
    biddyVariableTable.table[x].prev = biddyVariableTable.table[v].prev;
    biddyVariableTable.table[x].next = v;
    biddyVariableTable.table[v].prev = x;
  }
  else if ((biddyManagerType == BIDDYTYPEZBDDC) || (biddyManagerType == BIDDYTYPEZBDD) ||
            (biddyManagerType == BIDDYTYPEZFDDC) || (biddyManagerType == BIDDYTYPEZFDD) ||
            (biddyManagerType == BIDDYTYPETZBDDC) || (biddyManagerType == BIDDYTYPETZBDD) ||
            (biddyManagerType == BIDDYTYPETZFDDC) || (biddyManagerType == BIDDYTYPETZFDD))
  {
    /* NOT IMPLEMENTED, YET */
    fprintf(stderr,"Biddy_AddVariableAbove: this GDD type is not supported, yet!\n");
    return biddyNull;
  } else {
    fprintf(stderr,"Biddy_AddVariableAbove: Unsupported GDD type!\n");
    return biddyNull;
  }

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
    Parameter leftright should be TRUE for left and FALSE for right.
    For OBDDC, it is better to use macro Biddy_InvCond.
    For OBDDC, parameter leftright is ignored.
### Side effects
    TO DO: swap the meaning of parameter leftright (left should be FALSE)
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

  assert (
    (biddyManagerType == BIDDYTYPEOBDD) || (biddyManagerType == BIDDYTYPEOBDDC) ||
    (biddyManagerType == BIDDYTYPEZBDD) || (biddyManagerType == BIDDYTYPEZBDDC) ||
    (biddyManagerType == BIDDYTYPETZBDD) || (biddyManagerType == BIDDYTYPETZBDDC) ||
    (biddyManagerType == BIDDYTYPEOFDD) || (biddyManagerType == BIDDYTYPEOFDDC) ||
    (biddyManagerType == BIDDYTYPEZFDD) || (biddyManagerType == BIDDYTYPEZFDDC) ||
    (biddyManagerType == BIDDYTYPETZFDD) || (biddyManagerType == BIDDYTYPETZFDDC)
  );

  if (biddyManagerType == BIDDYTYPEOBDDC) {
    /* IMPLEMENTED */
    if (Biddy_GetMark(f) != mark) Biddy_SetMark(f); else Biddy_ClearMark(f);
  } else if (biddyManagerType == BIDDYTYPEZBDDC) {
    /* IMPLEMENTED */
    if (leftright) {
      if (Biddy_GetMark(f) != mark) Biddy_SetMark(f); else Biddy_ClearMark(f);
    }
  } else if (biddyManagerType == BIDDYTYPETZBDDC) {
    /* NOT IMPLEMENTED, YET */
    fprintf(stderr,"Biddy_TransferMark: this GDD type is not supported, yet!\n");
    return biddyNull;
  } else if ((biddyManagerType == BIDDYTYPEOFDDC) ||
              (biddyManagerType == BIDDYTYPEZFDDC) ||
              (biddyManagerType == BIDDYTYPETZFDDC))
  {
    fprintf(stderr,"Biddy_TransferMark: this GDD type is not supported, yet!\n");
    return biddyNull;
  } else if ((biddyManagerType == BIDDYTYPEOBDD) ||
              (biddyManagerType == BIDDYTYPEZBDD) ||
              (biddyManagerType == BIDDYTYPETZBDD) ||
              (biddyManagerType == BIDDYTYPEOFDD) ||
              (biddyManagerType == BIDDYTYPEZFDD) ||
              (biddyManagerType == BIDDYTYPETZFDD))
  {
    /* NOTHING TO TRANSFER */
  }

  return f;
}

#ifdef __cplusplus
}
#endif

/***************************************************************************//*!
\brief Function Biddy_Managed_IncTag returns edge with an incremented tag.

### Description
    Used for TZBDDs and TZFDDs, only.
### Side effects
    It is not checked, that the resulting tag is not greater than top variable.
    Function may return non-fresh node even if f is fresh.
### More info
    Macro Biddy_IncTag() is defined for use with anonymous manager.
*******************************************************************************/

#ifdef __cplusplus
extern "C" {
#endif

Biddy_Edge
Biddy_Managed_IncTag(Biddy_Manager MNG, Biddy_Edge f)
{
  Biddy_Variable tag,var;

  if (!MNG) MNG = biddyAnonymousManager;

  assert(
    (biddyManagerType == BIDDYTYPETZBDD) ||
    (biddyManagerType == BIDDYTYPETZBDDC) ||
    (biddyManagerType == BIDDYTYPETZFDD) ||
    (biddyManagerType == BIDDYTYPETZFDDC)
  );

  tag = Biddy_GetTag(f);
  var = BiddyV(f);

  assert( BiddyIsSmaller(tag,var) );

  tag = biddyVariableTable.table[tag].next;

  if (biddyManagerType == BIDDYTYPETZBDD) {

    /* WE HAVE TO CHECK IF MINIMIZATION RULE 3 CAN BE APPLIED */
    if (var && (var == tag) && (BiddyE(f) == BiddyT(f))) {
      f = BiddyE(f);
    } else {
      Biddy_SetTag(f,tag);
    }

    assert ( (Biddy_GetTag(f) == BiddyV(f)) || BiddyIsSmaller(Biddy_GetTag(f),BiddyV(f)) );

  } else if (biddyManagerType == BIDDYTYPETZBDDC) {
    /* NOT IMPLEMENTED, YET */
    fprintf(stderr,"Biddy_IncTag: this GDD type is not supported, yet!\n");
    return biddyNull;
  } else if ((biddyManagerType == BIDDYTYPETZFDDC) || (biddyManagerType == BIDDYTYPETZFDD)) {
    /* NOT IMPLEMENTED, YET */
    fprintf(stderr,"Biddy_IncTag: this GDD type is not supported, yet!\n");
    return biddyNull;
  }

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
    new one. If pf = pt = NULL (and ptag = v) then new variable (for
    OBDD, OFDD, TZBDD, and TZFDD) or new element (for ZBDD and ZFDD) is
    created. For OBDD, ZBDD, OFDD, and ZFDD, parameter ptag is ignored. For
    TZBDD and TZFDD, the returned edge is tagged with the given ptag. This
    function should not be called directly to add new variables and elements,
    you must use Biddy_Managed_FoaVariable, Biddy_Managed_AddVariableByName, or
    Biddy_Managed_AddElementByName.
### Side effects
    Using Biddy_Managed_TaggedFoaNode you can create node with an arbitrary
    ordering. It is much more safe to use Boolean operators, e.g.
    Biddy_Managed_ITE.
### More info
    Macro Biddy_Managed_FoaNode(MNG,v,pf,pt,garbageAllowed) is defined for
    use with implicit tags or without tags.
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

  if (biddyManagerType == BIDDYTYPEOBDD) {
    /* IMPLEMENTED */
    r = BiddyManagedTaggedFoaNode(MNG,v,pf,pt,ptag,garbageAllowed);
    BiddyRefresh(r); /* FoaNode returns an obsolete node! */
  } else if (biddyManagerType == BIDDYTYPEOBDDC) {
    /* IMPLEMENTED */
    r = BiddyManagedTaggedFoaNode(MNG,v,pf,pt,ptag,garbageAllowed);
    BiddyRefresh(r); /* FoaNode returns an obsolete node! */
  } else if (biddyManagerType == BIDDYTYPEZBDDC) {
    /* IMPLEMENTED */
    r = BiddyManagedTaggedFoaNode(MNG,v,pf,pt,ptag,garbageAllowed);
    BiddyRefresh(r); /* FoaNode returns an obsolete node! */
  } else if (biddyManagerType == BIDDYTYPETZBDD) {
    /* IMPLEMENTED */
    r = BiddyManagedTaggedFoaNode(MNG,v,pf,pt,ptag,garbageAllowed);
    BiddyRefresh(r); /* FoaNode returns an obsolete node! */
  } else if ((biddyManagerType == BIDDYTYPEZBDD) ||
              (biddyManagerType == BIDDYTYPETZBDDC))
  {
    fprintf(stderr,"Biddy_TaggedFoaNode: this GDD type is not supported, yet!\n");
    return biddyNull;
  } else if ((biddyManagerType == BIDDYTYPEOFDDC) || (biddyManagerType == BIDDYTYPEOFDD) ||
              (biddyManagerType == BIDDYTYPEZFDDC) || (biddyManagerType == BIDDYTYPEZFDD) ||
              (biddyManagerType == BIDDYTYPETZFDDC) || (biddyManagerType == BIDDYTYPETZFDD))
  {
    fprintf(stderr,"Biddy_TaggedFoaNode: this GDD type is not supported, yet!\n");
    return biddyNull;
  } else {
    fprintf(stderr,"Biddy_TaggedFoaNode: Unsupported GDD type!\n");
    return biddyNull;
  }

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
  Biddy_Boolean addNodeSpecial;
  Biddy_Variable tag;

  static BiddyNode *newFreeNodes;

  assert( MNG != NULL );

  /* IMPLEMENTED FOR OBDD, OBDDC, ZBDDC, AND TZBDD */
  assert(
    (biddyManagerType == BIDDYTYPEOBDD) ||
    (biddyManagerType == BIDDYTYPEOBDDC) ||
    (biddyManagerType == BIDDYTYPEZBDDC) ||
    (biddyManagerType == BIDDYTYPETZBDD)
  );

#ifdef BIDDYEXTENDEDSTATS_YES
  biddyNodeTable.foa++;
#endif

  /* DEBUGGING */
  /*
  printf("FOANODE: v=%u, pf = %p, pt = %p, biddyNull = %p, NULL = %ld\n",v,pf,pt,biddyNull,NULL);
  */

  /* DISABLE GC */
  /*
  garbageAllowed = FALSE;
  */

  /* SPECIAL CASE */
  addNodeSpecial = FALSE;
  if (!pf) {
    pf = biddyZero;
    pt = biddyTerminal;
    addNodeSpecial = TRUE;
  } else {
    assert( BiddyIsSmaller(v,BiddyV(pf)) );
    assert( BiddyIsSmaller(v,BiddyV(pt)) );
  }

  /*
  if (biddyNodeTable.num > 262140) {
    printf("FOANODE: START\n");
    BiddySystemReport(MNG);
  }
  */

  /* DEBUGGING */
  /*
  printf("FOANODE");
  if (addNodeSpecial) {
    printf("(addNodeSpecial==TRUE)");
  }
  printf(": v = <%s>%s, pf = <%s>%s, pt = <%s>%s\n",
         biddyVariableTable.table[ptag].name,
         biddyVariableTable.table[v].name,
         biddyVariableTable.table[Biddy_GetTag(pf)].name,
         Biddy_GetMark(pf)?"0":Biddy_Managed_GetTopVariableName(MNG,pf),
         biddyVariableTable.table[Biddy_GetTag(pt)].name,
         Biddy_GetMark(pt)?"0":Biddy_Managed_GetTopVariableName(MNG,pt));
  */

  /* SIMPLE CASES */

  if ((biddyManagerType == BIDDYTYPEOBDDC) || (biddyManagerType == BIDDYTYPEOBDD)) {
    if (pf == pt) {
      return pf;
    }
  }

  else if (biddyManagerType == BIDDYTYPEZBDDC) {
    if (pt == biddyZero) {
      return pf;
    }
  }

  else if (biddyManagerType == BIDDYTYPETZBDD) {
    if (BiddyR(pt) == biddyZero) {
      if (BiddyR(pf) == biddyZero) {
        return biddyZero;
      }
      tag = Biddy_GetTag(pf);
      v = biddyVariableTable.table[v].next;
      if (tag == v) {
        return Biddy_SetTag(pf,ptag);
      } else {
        return BiddyManagedTaggedFoaNode(MNG,v,pf,pf,ptag,garbageAllowed);
      }
    }
    if (pf == pt) {
      if (ptag == v) {
        return pf;
      }
    }
  }

  /* NORMALIZATION OF COMPLEMENTED EDGES */
  complementedResult = FALSE;

  if (biddyManagerType == BIDDYTYPEOBDDC) {
    if (Biddy_GetMark(pt)) {
      Biddy_InvertMark(pf);
      Biddy_ClearMark(pt);
      complementedResult = TRUE;
    } else {
      complementedResult = FALSE;
    }
  }

  else if (biddyManagerType == BIDDYTYPEZBDDC) {
    if (Biddy_GetMark(pf)) {
      Biddy_ClearMark(pf);
      complementedResult = TRUE;
    } else {
      complementedResult = FALSE;
    }
  }

  else if (biddyManagerType == BIDDYTYPETZBDD) {
    /* COMPLEMENTED EDGES ARE NOT USED */
  }

  /* NODE (v,0,1) IS NOT HASHED IN THE USUAL WAY */
  /* THE RESULTING EDGE IS STORED ONLY AS PART OF BiddyVariable */
  if ((biddyManagerType == BIDDYTYPEOBDDC) || (biddyManagerType == BIDDYTYPEOBDD) ||
      (biddyManagerType == BIDDYTYPEOFDDC) || (biddyManagerType == BIDDYTYPEOFDD))
  {
    if ((pf == biddyZero) && (pt == biddyTerminal) && !addNodeSpecial) {
      if (complementedResult) {
        return Biddy_Inv(biddyVariableTable.table[v].variable);
      } else {
        return biddyVariableTable.table[v].variable;
      }
    }
  }
  if ((biddyManagerType == BIDDYTYPEZBDDC) || (biddyManagerType == BIDDYTYPEZBDD) ||
      (biddyManagerType == BIDDYTYPEZFDDC) || (biddyManagerType == BIDDYTYPEZFDD))
  {
    if ((pf == biddyTerminal) && (pt == biddyTerminal) && !addNodeSpecial) {
      if (complementedResult) {
        return biddyVariableTable.table[v].element;
      } else {
        return Biddy_Inv(biddyVariableTable.table[v].element);
      }
    }
  }
  if ((biddyManagerType == BIDDYTYPETZBDDC) || (biddyManagerType == BIDDYTYPETZBDD) ||
      (biddyManagerType == BIDDYTYPETZFDDC) || (biddyManagerType == BIDDYTYPETZFDD))
  {
    if ((pf == biddyZero) && (pt == biddyTerminal) && !addNodeSpecial) {
      edge = biddyVariableTable.table[v].variable;
      Biddy_SetTag(edge,ptag);
      return edge;
    }
  }

  /* FIND OR ADD - THERE IS A HASH TABLE WITH CHAINING */
  /* THE CREATED NODE WILL ALWAYS BE THE FIRST NODE IN THE CHAIN */
  /* BECAUSE OF USED TRICKS, HASH FUNCTION MUST NEVER RETURN ZERO! */
  /* VARIABLES/ELEMENTS (v,0,1) ARE NOT HASHED IN THE USUAL WAY */

  if (addNodeSpecial) {
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

      /* PROFILING */
      /*
      fprintf(stderr,"AutoGC IN: biddyNodeTable.num = %u, biddyNodeTable.generated = %u\n",biddyNodeTable.num,biddyNodeTable.generated);
      */

      Biddy_Managed_AutoGC(MNG);

      /* PROFILING */
      /*
      fprintf(stderr,"AutoGC OUT: biddyNodeTable.num = %u, biddyNodeTable.generated = %u, free = %u (%.2f)\n",
              biddyNodeTable.num,biddyNodeTable.generated,biddyNodeTable.generated-biddyNodeTable.num,100.0*biddyNodeTable.num/biddyNodeTable.generated);
      */

      /* the table may be resized, thus the element must be rehashed */
      hash = nodeTableHash(v, pf, pt, biddyNodeTable.size);
      sup = biddyNodeTable.table[hash];
      sup1 = findNodeTable(MNG, v, pf, pt, &sup);

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

        /* PROFILING */
        /*
        printf(">");
        */
      }

      /* PROFILING */
      /*
      else {
        printf("=");
      }
      */

      if (!(newFreeNodes = (BiddyNode *)
        malloc((biddyNodeTable.blocksize) * sizeof(BiddyNode))))
      {
        fprintf(stderr, "\nBIDDY (BiddyManagedTaggedFoaNode): Out of memory error!\n");
        fprintf(stderr, "Currently, there exist %d nodes.\n", biddyNodeTable.num);
        exit(1);
      }

      biddyNodeTable.blocknumber++;
      if (!(tmp = (BiddyNode **)realloc(biddyNodeTable.blocktable,
        biddyNodeTable.blocknumber * sizeof(BiddyNode *))))
      {
        fprintf(stderr, "\nBIDDY (BiddyManagedTaggedFoaNode): Out of memory error!\n");
        fprintf(stderr, "Currently, there exist %d nodes.\n", biddyNodeTable.num);
        exit(1);
      }
      biddyNodeTable.blocktable = tmp;
      biddyNodeTable.blocktable[biddyNodeTable.blocknumber - 1] = newFreeNodes;
      biddyNodeTable.generated = biddyNodeTable.generated + biddyNodeTable.blocksize;

      newFreeNodes[biddyNodeTable.blocksize - 1].list = biddyFreeNodes;
      biddyFreeNodes = newFreeNodes;
      for (i = 0; i < biddyNodeTable.blocksize - 1; i++) {
        newFreeNodes = (BiddyNode*)(newFreeNodes->list = &newFreeNodes[1]);
      }

      /* PROFILING */
      /*
      fprintf(stderr,"FOANODE NEW BLOCK: biddyNodeTable.num = %u, biddyNodeTable.generated = %u, free = %u (%.2f)\n",
              biddyNodeTable.num,biddyNodeTable.generated,biddyNodeTable.generated-biddyNodeTable.num,100.0*biddyNodeTable.num/biddyNodeTable.generated);
      */

    }

    /* ADDING NEW NODE */
    (biddyNodeTable.num)++;
    if (biddyNodeTable.num > biddyNodeTable.max) biddyNodeTable.max = biddyNodeTable.num;
    (biddyVariableTable.table[v].num)++;

    sup = biddyFreeNodes;
    biddyFreeNodes = (BiddyNode *)sup->list;

    /* DEBUGGING */
    /*
    printf("FOANODE after the normalization: (%s,%s,%s)\n",
            Biddy_Managed_GetVariableName(MNG,v),
            Biddy_Managed_GetVariableName(MNG,BiddyV(pf)),
            Biddy_Managed_GetVariableName(MNG,BiddyV(pt))
          );
    */

    assert(BiddyIsSmaller(v,BiddyV(pf)));
    assert(BiddyIsSmaller(v,BiddyV(pt)));

    sup->f = pf; /* BE CAREFULL !!!! */
    sup->t = pt; /* you can create node with an arbitrary (wrong!) ordering */
    if (addNodeSpecial) {
      sup->expiry = biddySystemAge; /* variable/element node is refreshed! */
    } else {
      sup->expiry = 1; /* other nodes are not refreshed! */
    }
    sup->select = 0;
    sup->v = v;

    /* add new node to Node table */
    /* nodes (v,0,1) are not stored in Node table */
    if (!addNodeSpecial) {
      addNodeTable(MNG, hash, sup, sup1);
    }

    /* add new node to the list */
    /* all nodes are added to list, also variables/elements which are not added to Node table */
    /* lastNode->list IS NOT FIXED! */
    /* YOU MUST NEVER ASSUME THAT lastNode->list = NULL */
    biddyVariableTable.table[v].lastNode->list = (void *)sup;
    biddyVariableTable.table[v].lastNode = sup;

  }

  edge = sup;

  if (complementedResult) {
    Biddy_SetMark(edge);
  }

  if (biddyManagerType == BIDDYTYPETZBDD) {
    Biddy_SetTag(edge,ptag);
  }

  /* (Biddy v1.5) To enable efficient memory management (e.g. sifting) */
  /* the function started with the returned node is not recursively refreshed! */

  /* DEBUGGING */
  /*
  printf("FOANODE COMPLETE");
  if (addNodeSpecial) {
    printf("(addNodeSpecial==TRUE)");
  }
  printf(": <");
  if (!Biddy_IsNull(edge) && Biddy_GetMark(edge)) printf("*");
  printf("<\"%s\">\"%s\",",Biddy_Managed_GetVariableName(MNG,Biddy_GetTag(edge)),Biddy_Managed_GetTopVariableName(MNG,edge)); 
  if (!Biddy_IsNull(BiddyE(edge)) && Biddy_GetMark(BiddyE(edge))) printf("*");
  if (Biddy_IsNull(BiddyE(edge))) printf("NULL,");
    else printf("<\"%s\">\"%s\",",Biddy_Managed_GetVariableName(MNG,Biddy_GetTag(BiddyE(edge))),Biddy_Managed_GetTopVariableName(MNG,BiddyE(edge)));
  if (!Biddy_IsNull(BiddyT(edge)) && Biddy_GetMark(BiddyT(edge))) printf("*");
  if (Biddy_IsNull(BiddyT(edge))) printf("NULL>");
    else printf("<\"%s\">\"%s\">",Biddy_Managed_GetVariableName(MNG,Biddy_GetTag(BiddyT(edge))),Biddy_Managed_GetTopVariableName(MNG,BiddyT(edge)));
  printf("@%u",BiddyN(edge)->expiry);
  printf("\n");
  */

  assert( BiddyIsSmaller(BiddyV(edge),BiddyV(BiddyE(edge))) );
  assert( BiddyIsSmaller(BiddyV(edge),BiddyV(BiddyT(edge))) );
  assert( (biddyManagerType != BIDDYTYPETZBDD) || (Biddy_GetTag(edge) == BiddyV(edge)) || BiddyIsSmaller(Biddy_GetTag(edge),BiddyV(edge)) );
  assert( (biddyManagerType != BIDDYTYPETZBDD) || (Biddy_GetTag(BiddyE(edge)) == BiddyV(BiddyE(edge))) || BiddyIsSmaller(Biddy_GetTag(BiddyE(edge)),BiddyV(BiddyE(edge))) );
  assert( (biddyManagerType != BIDDYTYPETZBDD) || (Biddy_GetTag(BiddyT(edge)) == BiddyV(BiddyT(edge))) || BiddyIsSmaller(Biddy_GetTag(BiddyT(edge)),BiddyV(BiddyT(edge))) );

  /* if (!addNodeSpecial) BiddySystemReport(MNG); */

  /*
  if (biddyNodeTable.num > 262140) {
    printf("FOANODE: END\n");
    BiddySystemReport(MNG);
  }
  */

  return edge;
}

/***************************************************************************//*!
\brief Function Biddy_Managed_Not calculates Boolean function NOT.

### Description
### Side effects
    Implemented for OBDDs, ZBDDs, and TZBDDs.
    For OBDD and OFDD, it is better to use macro Biddy_Inv.
    For OBDDC, cache table is not needed.
    For ZBDDC, recursive calls are via Xor and thus its cache table is used.
    For all other BDD types, results are cached as (f,biddyZero,biddyOne).
### More info
    Macro Biddy_Not() is defined for use with anonymous manager.
*******************************************************************************/

#ifdef __cplusplus
extern "C" {
#endif

Biddy_Edge
Biddy_Managed_Not(Biddy_Manager MNG, Biddy_Edge f)
{
  Biddy_Edge r;

  assert( f != NULL );

  if (!MNG) MNG = biddyAnonymousManager;
  ZF_LOGI("Biddy_Not");

  assert( BiddyIsOK(f) == TRUE );

  r = biddyNull;

  if (biddyManagerType == BIDDYTYPEOBDD) {
    /* IMPLEMENTED */
    r = BiddyManagedNot(MNG,f);
    BiddyRefresh(r); /* not always refreshed by BiddyManagedNot */
  } else if (biddyManagerType == BIDDYTYPEOBDDC) {
    /* IMPLEMENTED */
    r = Biddy_Inv(f);
    /* BiddyRefresh() */ /* not needed because the same node is returned */
  } else if (biddyManagerType == BIDDYTYPEZBDDC) {
    /* IMPLEMENTED */
    r = BiddyManagedNot(MNG,f);
    BiddyRefresh(r); /* not always refreshed by BiddyManagedNot */
  } else if (biddyManagerType == BIDDYTYPETZBDD) {
    /* IMPLEMENTED */
    r = BiddyManagedNot(MNG,f);
    BiddyRefresh(r); /* not always refreshed by BiddyManagedNot */
  } else if ((biddyManagerType == BIDDYTYPEZBDD) ||
              (biddyManagerType == BIDDYTYPETZBDDC))
  {
    fprintf(stderr,"Biddy_Not: this GDD type is not supported, yet!\n");
    return biddyNull;
  } else if ((biddyManagerType == BIDDYTYPEOFDDC) || (biddyManagerType == BIDDYTYPEOFDD) ||
              (biddyManagerType == BIDDYTYPEZFDDC) || (biddyManagerType == BIDDYTYPEZFDD) ||
              (biddyManagerType == BIDDYTYPETZFDDC) || (biddyManagerType == BIDDYTYPETZFDD))
  {
    fprintf(stderr,"Biddy_Not: this GDD type is not supported, yet!\n");
    return biddyNull;
  } else {
    fprintf(stderr,"Biddy_Not: Unsupported GDD type!\n");
    return biddyNull;
  }

  return r;
}

#ifdef __cplusplus
}
#endif

Biddy_Edge
BiddyManagedNot(const Biddy_Manager MNG, const Biddy_Edge f)
{
  Biddy_Edge r, T, E, Fv, Gv, Fneg_v, Gneg_v;
  Biddy_Edge FF, GG, HH;
  Biddy_Variable v, nt;
  unsigned int cindex;

  assert( MNG != NULL );
  assert( f != NULL );

  /* IMPLEMENTED FOR OBDD, OBDDC, ZBDDC, AND TZBDD */
  assert(
    (biddyManagerType == BIDDYTYPEOBDD) ||
    (biddyManagerType == BIDDYTYPEOBDDC) ||
    (biddyManagerType == BIDDYTYPEZBDDC) ||
    (biddyManagerType == BIDDYTYPETZBDD)
  );

  r = biddyNull;

  /* LOOKING FOR SIMPLE CASE */

  if (f == biddyZero) {
    return biddyOne;
  }
  if (f == biddyOne) {
    return biddyZero;
  }

  /* THIS IS NOT A SIMPLE CASE */

  FF = GG = HH = biddyNull;
  Fneg_v = Fv = biddyNull;
  Gneg_v = Gv = biddyNull;
  v = 0;

  if (biddyManagerType == BIDDYTYPEOBDDC) {

    /* FOR OBDDC, CACHE TABLE IS NOT NEEDED */

    r = Biddy_Inv(f);

  }

  else if (biddyManagerType == BIDDYTYPEOBDD) {

    /* FOR OBDD, 'NOT' USES CACHE TABLE DIRECTLY */
    /* THIS IS NOT ITE CACHE! */

    FF = f;
    GG = biddyZero;
    HH = biddyOne;

    /* IF RESULT IS NOT IN THE CACHE TABLE... */
    cindex = 0;
    if (!findOp3Cache(MNG,biddyOPCache,FF,GG,HH,&r,&cindex))
    {

      /* DETERMINING PARAMETERS FOR RECURSIVE CALLS */
      v = BiddyV(f);
      Fneg_v = BiddyE(f);
      Fv = BiddyT(f);

      /* RECURSIVE CALLS */
      if (Fneg_v == biddyZero) {
        E = biddyOne;
      } else if (Fneg_v == biddyOne) {
        E = biddyZero;
      } else  {
        E = BiddyManagedNot(MNG,Fneg_v);
      }
      if (Fv == biddyZero) {
        T = biddyOne;
      } else if (Fv == biddyOne) {
        T = biddyZero;
      } else  {
        T = BiddyManagedNot(MNG,Fv);
      }

      r = BiddyManagedTaggedFoaNode(MNG,v,E,T,v,TRUE);
      BiddyRefresh(r); /* FoaNode returns an obsolete node! */

      addOp3Cache(MNG,biddyOPCache,FF,GG,HH,r,cindex);

    } else {

      /* IF THE RESULT IS FROM CACHE TABLE, REFRESH IT! */
      BiddyRefresh(r);

    }

  }

  else if (biddyManagerType == BIDDYTYPEZBDDC) {

    /* FOR ZBDD, RECURSIVE CALLS ARE VIA 'XOR' AND THUS ITS CACHE TABLE IS USED */

    v = BiddyV(biddyOne); /* biddyOne includes all variables */

    /* DETERMINING PARAMETERS FOR RECURSIVE CALLS */
    /* COMPLEMENTED EDGES MUST BE TRANSFERED */
    if (BiddyV(f) == v) {
      Fneg_v = Biddy_InvCond(BiddyE(f),Biddy_GetMark(f));
      Fv = BiddyT(f);
    } else {
      Fneg_v = f;
      Fv = biddyZero;
    }
    Gneg_v = BiddyE(biddyOne);
    Gv = BiddyT(biddyOne);

    /* RECURSIVE CALLS */
    if (Fneg_v == biddyZero) {
      E = Gneg_v;
    } else if (Gneg_v == biddyZero) {
      E = Fneg_v;
    } else  {
      E = BiddyManagedXor(MNG,Fneg_v,Gneg_v);
    }
    if (Fv == biddyZero) {
      T = Gv;
    } else if (Gv == biddyZero) {
      T = Fv;
    } else  {
      T = BiddyManagedXor(MNG,Fv,Gv);
    }

    r = BiddyManagedTaggedFoaNode(MNG,v,E,T,0,TRUE);
    BiddyRefresh(r); /* FoaNode returns an obsolete node! */

  }

  else if (biddyManagerType == BIDDYTYPETZBDD) {

    /* FOR TZBDD, 'NOT' USES CACHE TABLE DIRECTLY */
    /* THIS IS NOT ITE CACHE! */

    FF = f;
    GG = biddyZero;
    HH = biddyOne;

    assert ( f != biddyZero );
    assert ( f != biddyOne );

    /* IF RESULT IS NOT IN THE CACHE TABLE... */
    cindex = 0;
    if (!findOp3Cache(MNG,biddyOPCache,FF,GG,HH,&r,&cindex))
    {

      v = BiddyV(f);
      nt = Biddy_GetTag(f);
      if (!v) {
        r = biddyZero;
      } else {
        Fneg_v = BiddyE(f);
        Fv = BiddyT(f);

        /* RECURSIVE CALLS */
        if (Fneg_v == biddyZero) {
          E = biddyOne;
        } else if (Fneg_v == biddyOne) {
          E = biddyZero;
        } else  {
          E = BiddyManagedNot(MNG,Fneg_v);
        }
        if (Fv == biddyZero) {
          T = biddyOne;
        } else if (Fv == biddyOne) {
          T = biddyZero;
        } else  {
          T = BiddyManagedNot(MNG,Fv);
        }

        r = BiddyManagedTaggedFoaNode(MNG,v,E,T,v,TRUE);
        BiddyRefresh(r); /* FoaNode returns an obsolete node! */
      }

      while (v != nt) {
        v = biddyVariableTable.table[v].prev;
        r = BiddyManagedTaggedFoaNode(MNG,v,r,biddyOne,v,TRUE);
        BiddyRefresh(r); /* FoaNode returns an obsolete node! */
      }

      addOp3Cache(MNG,biddyOPCache,FF,GG,HH,r,cindex);

    } else {

      /* IF THE RESULT IS FROM CACHE TABLE, REFRESH IT! */
      BiddyRefresh(r);

    }

  }

  return r;
}

/***************************************************************************//*!
\brief Function Biddy_Managed_ITE calculates ITE operation of three Boolean
       functions.

### Description
### Side Effects
    Implemented for OBDDs, ZBDDs, and TZBDDs.
    For OBDDC, results are cached as parameters to ITE(F,G,H)= F*G XOR F'*H.
    For all other BDD types, results are cached as (f,g,h) where
    f,g,h != 0, f != g, f != h, and g != h.
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

  r = biddyNull;

  if (biddyManagerType == BIDDYTYPEOBDD) {
    /* IMPLEMENTED */
    r = BiddyManagedITE(MNG,f,g,h);
    BiddyRefresh(r); /* not always refreshed by BiddyManagedITE */
  } else if (biddyManagerType == BIDDYTYPEOBDDC) {
    /* IMPLEMENTED */
    r = BiddyManagedITE(MNG,f,g,h);
    BiddyRefresh(r); /* not always refreshed by BiddyManagedITE */
  } else if (biddyManagerType == BIDDYTYPEZBDDC) {
    /* IMPLEMENTED */
    r = BiddyManagedITE(MNG,f,g,h);
    BiddyRefresh(r); /* not always refreshed by BiddyManagedITE */
  } else if (biddyManagerType == BIDDYTYPETZBDD) {
    /* IMPLEMENTED */
    r = BiddyManagedITE(MNG,f,g,h);
    BiddyRefresh(r); /* not always refreshed by BiddyManagedITE */
  } else if ((biddyManagerType == BIDDYTYPEZBDD) ||
              (biddyManagerType == BIDDYTYPETZBDDC))
  {
    fprintf(stderr,"Biddy_ITE: this GDD type is not supported, yet!\n");
    return biddyNull;
  } else if ((biddyManagerType == BIDDYTYPEOFDDC) || (biddyManagerType == BIDDYTYPEOFDD) ||
              (biddyManagerType == BIDDYTYPEZFDDC) || (biddyManagerType == BIDDYTYPEZFDD) ||
              (biddyManagerType == BIDDYTYPETZFDDC) || (biddyManagerType == BIDDYTYPETZFDD))
  {
    fprintf(stderr,"Biddy_ITE: this GDD type is not supported, yet!\n");
    return biddyNull;
  } else {
    fprintf(stderr,"Biddy_ITE: Unsupported GDD type!\n");
    return biddyNull;
  }

  return r;
}

#ifdef __cplusplus
}
#endif

Biddy_Edge
BiddyManagedITE(const Biddy_Manager MNG, Biddy_Edge f, Biddy_Edge g, Biddy_Edge h)
{
  Biddy_Edge r, T, E, Fv, Gv, Hv, Fneg_v, Gneg_v, Hneg_v;
  Biddy_Edge FF, GG, HH;
  Biddy_Boolean NN;
  Biddy_Variable v,rtag;
  unsigned cindex;

  static Biddy_Variable topF;    /* CAN BE STATIC, WHAT IS BETTER? */
  static Biddy_Variable topG;    /* CAN BE STATIC, WHAT IS BETTER? */
  static Biddy_Variable topH;    /* CAN BE STATIC, WHAT IS BETTER? */
  static Biddy_Variable tagF;    /* CAN BE STATIC, WHAT IS BETTER? */
  static Biddy_Variable tagG;    /* CAN BE STATIC, WHAT IS BETTER? */
  static Biddy_Variable tagH;    /* CAN BE STATIC, WHAT IS BETTER? */

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

  /* IMPLEMENTED FOR OBDD, OBDDC, ZBDDC, AND TZBDD */
  assert(
    (biddyManagerType == BIDDYTYPEOBDD) ||
    (biddyManagerType == BIDDYTYPEOBDDC) ||
    (biddyManagerType == BIDDYTYPEZBDDC) ||
    (biddyManagerType == BIDDYTYPETZBDD)
  );

#ifdef BIDDYEXTENDEDSTATS_YES
  biddyNodeTable.iterecursive++;
#endif

  r = biddyNull;

  /* LOOKING FOR SIMPLE CASE */

  if (f == biddyOne) {
    return g;
  } else if (f == biddyZero) {
    return h;
  } else if (g == biddyOne) {
    if (h == biddyOne) {
      return biddyOne;
    } else if (h == biddyZero) {
      return f;
    } else {
      return BiddyManagedOr(MNG,f,h);
    }
  } else if (g == biddyZero) {
    if (h == biddyOne) {
      return BiddyManagedNot(MNG,f);
    } else if (h == biddyZero) {
      return biddyZero;
    } else {
      return BiddyManagedGt(MNG,h,f);
    }
  } else if (h == biddyOne) {
    return BiddyManagedLeq(MNG,f,g);
  } else if (h == biddyZero) {
    return BiddyManagedAnd(MNG,f,g);
  } else if (g == h) {
    return g;
  } else if (f == g) {
    return BiddyManagedOr(MNG,f,h);
  } else if (f == h) {
    return BiddyManagedAnd(MNG,f,g);
  }

  if (biddyManagerType == BIDDYTYPEOBDDC) {
    if (Biddy_IsEqvPointer(g,h)) {
      return BiddyManagedXor(MNG,f,h);
    } else if (Biddy_IsEqvPointer(f,g)) {
      return BiddyManagedAnd(MNG,g,h);
    } else if (Biddy_IsEqvPointer(f,h)) {
      return BiddyManagedOr(MNG,g,h);
    }
  }

  /* THIS IS NOT A SIMPLE CASE */

  FF = GG = HH = biddyNull;
  NN = FALSE;
  Fneg_v = Fv = biddyNull;
  Gneg_v = Gv = biddyNull;
  Hneg_v = Hv = biddyNull;
  rtag = 0;
  v = 0;

  /* NORMALIZATION OF COMPLEMENTED EDGES */
  /* APLICABLE ONLY TO OBDDC */
  /* FF, GG, HH, and NN ARE USED FOR CACHE LOOKUP, ONLY */

  if (biddyManagerType == BIDDYTYPEOBDDC) {
    if (Biddy_GetMark(f)) {
      if (Biddy_GetMark(h)) {
        NN = TRUE;
        FF = Biddy_Inv(f);
        GG = Biddy_Inv(h);
        HH = Biddy_Inv(g);
      } else {
        FF = Biddy_Inv(f);
        GG = h;
        HH = g;
      }
    } else if (Biddy_GetMark(g)) {
      NN = TRUE;
      FF = f;
      GG = Biddy_Inv(g);
      HH = Biddy_Inv(h);
    } else {
      FF = f;
      GG = g;
      HH = h;
    }
  }

  else {

    /* ITE CACHE IS NOT SHARED WITH OTHER OPERATIONS! */

    FF = f;
    GG = g;
    HH = h;
  }

  /* IF RESULT IS NOT IN THE CACHE TABLE... */
  cindex = 0;
  if (!findOp3Cache(MNG,biddyOPCache,FF,GG,HH,&r,&cindex))
  {

    if ((biddyManagerType == BIDDYTYPEOBDDC) || (biddyManagerType == BIDDYTYPEOBDD)) {

      /* LOOKING FOR THE SMALLEST TOP VARIABLE */
      topF = BiddyV(f);
      topG = BiddyV(g);
      topH = BiddyV(h);

      /* CONSTANT NODE MUST HAVE MAX ORDER */
      if (BiddyIsSmaller(topF,topG)) {
        v = BiddyIsSmaller(topF,topH) ? topF : topH;
      } else {
        v = BiddyIsSmaller(topH,topG) ? topH : topG;
      }

      /* DETERMINING PARAMETERS FOR RECURSIVE CALLS */
      if (topF == v) {
        Fneg_v = Biddy_InvCond(BiddyE(f),Biddy_GetMark(f));
        Fv = Biddy_InvCond(BiddyT(f),Biddy_GetMark(f));
      } else {
        Fneg_v = Fv = f;
      }
      if (topG == v) {
        Gneg_v = Biddy_InvCond(BiddyE(g),Biddy_GetMark(g));
        Gv = Biddy_InvCond(BiddyT(g),Biddy_GetMark(g));
      } else {
        Gneg_v = Gv = g;
      }
      if (topH == v) {
        Hneg_v = Biddy_InvCond(BiddyE(h),Biddy_GetMark(h));
        Hv = Biddy_InvCond(BiddyT(h),Biddy_GetMark(h));
      } else {
        Hneg_v = Hv = h;
      }

      rtag = v;

    }

    else if (biddyManagerType == BIDDYTYPEZBDDC) {

      /* LOOKING FOR THE SMALLEST TOP VARIABLE */
      topF = BiddyV(f);
      topG = BiddyV(g);
      topH = BiddyV(h);

      /* CONSTANT NODE MUST HAVE MAX ORDER */
      if (BiddyIsSmaller(topF,topG)) {
        v = BiddyIsSmaller(topF,topH) ? topF : topH;
      } else {
        v = BiddyIsSmaller(topH,topG) ? topH : topG;
      }

      /* DETERMINING PARAMETERS FOR RECURSIVE CALLS */
      if (topF == v) {
        Fneg_v = Biddy_InvCond(BiddyE(f),Biddy_GetMark(f));
        Fv = BiddyT(f);
      } else {
        Fv = biddyZero;
        Fneg_v = f;
      }
      if (topG == v) {
        Gneg_v = Biddy_InvCond(BiddyE(g),Biddy_GetMark(g));
        Gv = BiddyT(g);
      } else {
        Gv = biddyZero;
        Gneg_v = g;
      }
      if (topH == v) {
        Hneg_v = Biddy_InvCond(BiddyE(h),Biddy_GetMark(h));
        Hv = BiddyT(h);
      } else {
        Hv = biddyZero;
        Hneg_v = h;
      }

      rtag = 0;

    }

    else if (biddyManagerType == BIDDYTYPETZBDD) {

      /* LOOKING FOR THE SMALLEST TAG AND THE SMALLEST TOP VARIABLE */
      /* CONSTANT NODE MUST HAVE MAX ORDER */
      tagF = Biddy_GetTag(f);
      tagG = Biddy_GetTag(g);
      tagH = Biddy_GetTag(h);
      topF = BiddyV(f);
      topG = BiddyV(g);
      topH = BiddyV(h);

      /* CONSTANT NODE MUST HAVE MAX ORDER */
      if (BiddyIsSmaller(tagF,tagG)) {
        rtag = BiddyIsSmaller(tagF,tagH) ? tagF : tagH;
      } else {
        rtag = BiddyIsSmaller(tagH,tagG) ? tagH : tagG;
      }
      if (BiddyIsSmaller(topF,topG)) {
        v = BiddyIsSmaller(topF,topH) ? topF : topH;
      } else {
        v = BiddyIsSmaller(topH,topG) ? topH : topG;
      }

      if ((tagF != tagG) || (tagF != tagH)) v = rtag;

      if (tagF == rtag) {
        if (topF == v) {
          Fneg_v = BiddyE(f);
          Fv = BiddyT(f);
        } else {
          Fneg_v = f;
          Biddy_SetTag(Fneg_v,v);
          Fneg_v = Biddy_Managed_IncTag(MNG,Fneg_v);
          Fv = biddyZero;
        }
      } else {
        Fneg_v = Fv = f;
      }
      if (tagG == rtag) {
        if (topG == v) {
          Gneg_v = BiddyE(g);
          Gv = BiddyT(g);
        } else {
          Gneg_v = g;
          Biddy_SetTag(Gneg_v,v);
          Gneg_v = Biddy_Managed_IncTag(MNG,Gneg_v);
          Gv = biddyZero;
        }
      } else {
        Gneg_v = Gv = g;
      }
      if (tagH == rtag) {
        if (topH == v) {
          Hneg_v = BiddyE(h);
          Hv = BiddyT(h);
        } else {
          Hneg_v = h;
          Biddy_SetTag(Hneg_v,v);
          Hneg_v = Biddy_Managed_IncTag(MNG,Hneg_v);
          Hv = biddyZero;
        }
      } else {
        Hneg_v = Hv = h;
      }

    }

    /* RECURSIVE CALLS */
    if (Fneg_v == biddyOne) {
      E = Gneg_v;
    } else if (Fneg_v == biddyZero) {
      E = Hneg_v;
    } else if (Gneg_v == Hneg_v) {
      E = Gneg_v;
    } else {
      E = BiddyManagedITE(MNG,Fneg_v,Gneg_v,Hneg_v);
    }
    if (Fv == biddyOne) {
      T = Gv;
    } else if (Fv == biddyZero) {
      T = Hv;
    } else if (Gv == Hv) {
      T = Gv;
    } else {
      T = BiddyManagedITE(MNG,Fv,Gv,Hv);
    }

    r = BiddyManagedTaggedFoaNode(MNG,v,E,T,rtag,TRUE);
    BiddyRefresh(r); /* FoaNode returns an obsolete node! */

    /* CACHE EVERYTHING */
    if (NN) {
      addOp3Cache(MNG,biddyOPCache,FF,GG,HH,Biddy_Inv(r),cindex);
    } else {
      addOp3Cache(MNG,biddyOPCache,FF,GG,HH,r,cindex);
    }

    /* DEBUGGING */
    /*
    printf("ITE has not used cache\n");
    */

  } else {

    if (NN) {
      Biddy_InvertMark(r);
    }

    /* IF THE RESULT IS FROM CACHE TABLE, REFRESH IT! */
    BiddyRefresh(r);

  }

  /* DEBUGGING */
  /*
  fprintf(stdout,"Biddy_ITE (%d) R\n",nn);
  writeBDD(MNG,r);
  */

  return r;
}

/***************************************************************************//*!
\brief Function Biddy_Managed_And calculates Boolean function AND
       (conjunction).

### Description
    For combination sets, this function coincides with Intersection.
### Side Effects
    Used by ITE (for OBDDs).
    Implemented for OBDDs, ZBDDs, and TZBDDs.
    For OBDDC, results are cached as parameters to ITE(F,G,H)= F*G XOR F'*H.
    For all other BDD types, results are cached as (f,g,biddyZero).
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

  r = biddyNull;

  if (biddyManagerType == BIDDYTYPEOBDD) {
    /* IMPLEMENTED */
    r = BiddyManagedAnd(MNG,f,g);
    BiddyRefresh(r); /* not always refreshed by BiddyManagedAnd */
  } else if (biddyManagerType == BIDDYTYPEOBDDC) {
    /* IMPLEMENTED */
    r = BiddyManagedAnd(MNG,f,g);
    BiddyRefresh(r); /* not always refreshed by BiddyManagedAnd */
  } else if (biddyManagerType == BIDDYTYPEZBDDC) {
    /* IMPLEMENTED */
    r = BiddyManagedAnd(MNG,f,g);
    BiddyRefresh(r); /* not always refreshed by BiddyManagedAnd */
  } else if (biddyManagerType == BIDDYTYPETZBDD) {
    /* IMPLEMENTED */
    r = BiddyManagedAnd(MNG,f,g);
    BiddyRefresh(r); /* not always refreshed by BiddyManagedAnd */
  } else if ((biddyManagerType == BIDDYTYPEZBDD) ||
              (biddyManagerType == BIDDYTYPETZBDDC))
  {
    fprintf(stderr,"Biddy_And: this GDD type is not supported, yet!\n");
    return biddyNull;
  } else if ((biddyManagerType == BIDDYTYPEOFDDC) || (biddyManagerType == BIDDYTYPEOFDD) ||
              (biddyManagerType == BIDDYTYPEZFDDC) || (biddyManagerType == BIDDYTYPEZFDD) ||
              (biddyManagerType == BIDDYTYPETZFDDC) || (biddyManagerType == BIDDYTYPETZFDD))
  {
    fprintf(stderr,"Biddy_And: this GDD type is not supported, yet!\n");
    return biddyNull;
  } else {
    fprintf(stderr,"Biddy_And: Unsupported GDD type!\n");
    return biddyNull;
  }

  return r;
}

#ifdef __cplusplus
}
#endif

Biddy_Edge
BiddyManagedAnd(const Biddy_Manager MNG, const Biddy_Edge f, const Biddy_Edge g)
{
  Biddy_Edge r, T, E, Fv, Gv, Fneg_v, Gneg_v;
  Biddy_Edge FF, GG, HH;
  Biddy_Boolean NN;
  Biddy_Variable v,rtag;
  unsigned int cindex;

  static Biddy_Variable topF;    /* CAN BE STATIC, WHAT IS BETTER? */
  static Biddy_Variable topG;    /* CAN BE STATIC, WHAT IS BETTER? */
  static Biddy_Variable tagF;    /* CAN BE STATIC, WHAT IS BETTER? */
  static Biddy_Variable tagG;    /* CAN BE STATIC, WHAT IS BETTER? */

  assert( MNG != NULL );
  assert( f != NULL );
  assert( g != NULL );

  /* IMPLEMENTED FOR OBDD, OBDDC, ZBDDC, AND TZBDD */
  assert(
    (biddyManagerType == BIDDYTYPEOBDD) ||
    (biddyManagerType == BIDDYTYPEOBDDC) ||
    (biddyManagerType == BIDDYTYPEZBDDC) ||
    (biddyManagerType == BIDDYTYPETZBDD)
  );

#ifdef BIDDYEXTENDEDSTATS_YES
  biddyNodeTable.andorrecursive++;
#endif

  /* LOOKING FOR SIMPLE CASE */

  if (f == biddyZero) {
    return biddyZero;
  } else if (g == biddyZero) {
    return biddyZero;
  } else if (f == biddyOne) {
    return g;
  } else if (g == biddyOne) {
    return f;
  } else if (f == g) {
    return f;
  }

  if (biddyManagerType == BIDDYTYPEOBDDC) {
    if (Biddy_IsEqvPointer(f,g)) {
      return biddyZero;
    }
  }

  else if (biddyManagerType == BIDDYTYPETZBDD) {
    if (BiddyR(f) == BiddyR(g)) {
      if (BiddyIsSmaller(Biddy_GetTag(f),Biddy_GetTag(g))) {
        return f;
      } else {
        return g;
      }
    }
  }

  /* THIS IS NOT A SIMPLE CASE */

  FF = GG = HH = biddyNull;
  NN = FALSE;
  Fneg_v = Fv = biddyNull;
  Gneg_v = Gv = biddyNull;
  rtag = 0;
  v = 0;

  /* NORMALIZATION OF COMPLEMENTED EDGES */
  /* FF, GG, and HH ARE USED FOR CACHE LOOKUP, ONLY */
  /* FF AND GG SHOULD NOT BE COMPLEMENTED */
  /* ((uintptr_t) f) SHOULD BE GREATER THAN ((uintptr_t) g) */

  if (biddyManagerType == BIDDYTYPEOBDDC) {

    if (((uintptr_t) f) > ((uintptr_t) g)) {
      if (Biddy_GetMark(f)) {
        NN = TRUE;
        FF = Biddy_Inv(f);
        GG = biddyOne; /* Biddy_Inv(h) */
        HH = Biddy_Inv(g);
      } else {
        if (Biddy_GetMark(g)) {
          NN = TRUE;
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
        NN = TRUE;
        FF = Biddy_Inv(g);
        GG = biddyOne; /* Biddy_Inv(h) */
        HH = Biddy_Inv(f);
      } else {
        if (Biddy_GetMark(f)) {
          NN = TRUE;
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

  }

  else {

    /* THIS IS NOT ITE CACHE! */

    if (((uintptr_t) f) > ((uintptr_t) g)) {
      FF = f;
      GG = g;
      HH = biddyZero;
    } else {
      FF = g;
      GG = f;
      HH = biddyZero;
    }

  }

  assert ( f != biddyZero );
  assert ( f != biddyOne );
  assert ( g != biddyZero );
  assert ( g != biddyOne );

  /* IF RESULT IS NOT IN THE CACHE TABLE... */
  /* FOR ZBDDs, NOT EVERYTHING IS CACHED */
  cindex = 0;
  if (((biddyManagerType == BIDDYTYPEZBDDC) && ((topF = BiddyV(f)) != (topG = BiddyV(g))))
      || !findOp3Cache(MNG,biddyOPCache,FF,GG,HH,&r,&cindex))
  {

    if ((biddyManagerType == BIDDYTYPEOBDDC) || (biddyManagerType == BIDDYTYPEOBDD)) {

      /* LOOKING FOR THE SMALLEST TOP VARIABLE */
      /* CONSTANT NODE MUST HAVE MAX ORDER */
      topF = BiddyV(f);
      topG = BiddyV(g);
      v = BiddyIsSmaller(topF,topG) ? topF : topG;

      /* DETERMINING PARAMETERS FOR RECURSIVE CALLS */
      /* COMPLEMENTED EDGES MUST BE TRANSFERED */

      if (topF == v) {
        Fneg_v = Biddy_InvCond(BiddyE(f),Biddy_GetMark(f));
        Fv = Biddy_InvCond(BiddyT(f),Biddy_GetMark(f));
      } else {
        Fneg_v = Fv = f;
      }

      if (topG == v) {
        Gneg_v = Biddy_InvCond(BiddyE(g),Biddy_GetMark(g));
        Gv = Biddy_InvCond(BiddyT(g),Biddy_GetMark(g));
      } else {
        Gneg_v = Gv = g;
      }

      rtag = v;

    }

    else if (biddyManagerType == BIDDYTYPEZBDDC) {

      /* LOOKING FOR THE SMALLEST TOP VARIABLE */
      /* CONSTANT NODE MUST HAVE MAX ORDER */
      /* FOR ZBDD, topF AND topG HAVE BEEN ALREADY CALCULATED */
      /* topF = BiddyV(f); */ /* ALREADY CALCULATED */
      /* topG = BiddyV(g); */ /* ALREADY CALCULATED */
      v = BiddyIsSmaller(topF,topG) ? topF : topG;

      /* THIS CALL IS NOT INTRODUCING NEW NODES */
      /* COMPLEMENTED EDGES MUST BE TRANSFERED */
      /* IT SEEMS THAT CACHING THIS RESULTS IS NOT A GOOD IDEA */
      if (topF != topG) {
        if (topF == v) {
          r = BiddyManagedAnd(MNG,Biddy_InvCond(BiddyE(f),Biddy_GetMark(f)),g);
          return r;
        } else {
          r = BiddyManagedAnd(MNG,f,Biddy_InvCond(BiddyE(g),Biddy_GetMark(g)));
          return r;
        }
      }

      /* DETERMINING PARAMETERS FOR RECURSIVE CALLS, topF == topG */
      /* COMPLEMENTED EDGES MUST BE TRANSFERED */
      Fneg_v = Biddy_InvCond(BiddyE(f),Biddy_GetMark(f));
      Fv = BiddyT(f);
      Gneg_v = Biddy_InvCond(BiddyE(g),Biddy_GetMark(g));
      Gv = BiddyT(g);

      rtag = 0;

    }

    else if (biddyManagerType == BIDDYTYPETZBDD) {

      /* LOOKING FOR THE SMALLEST TAG AND SMALLEST TOP VARIABLE */
      /* CONSTANT NODE MUST HAVE MAX ORDER */
      tagF = Biddy_GetTag(f);
      tagG = Biddy_GetTag(g);
      topF = BiddyV(f);
      topG = BiddyV(g);
      rtag = BiddyIsSmaller(tagF,tagG) ? tagF : tagG;
      v = BiddyIsSmaller(topF,topG) ? topF : topG;

      /* VARIANT 1 */
      /*
      if (BiddyIsSmaller(topF,tagG)) {
        Fneg_v = BiddyE(f);
        Fv = BiddyT(f);
        Gneg_v = Gv = g;
      } else if (BiddyIsSmaller(topG,tagF)) {
        Fneg_v = Fv = f;
        Gneg_v = BiddyE(g);
        Gv = BiddyT(g);
      } else {
        if (topF == v) {
          Fneg_v = BiddyE(f);
          Fv = BiddyT(f);
        } else {
          Fneg_v = f;
          Biddy_SetTag(Fneg_v,v);
          Fneg_v = Biddy_Managed_IncTag(MNG,Fneg_v);
          Fv = biddyZero;
        }
        if (topG == v) {
          Gneg_v = BiddyE(g);
          Gv = BiddyT(g);
        } else {
          Gneg_v = g;
          Biddy_SetTag(Gneg_v,v);
          Gneg_v = Biddy_Managed_IncTag(MNG,Gneg_v);
          Gv = biddyZero;
        }
      }
      */

      /* VARIANT 2 */
      /*
      if (topF == v) {
        Fneg_v = BiddyE(f);
        Fv = BiddyT(f);
      } else {
        if (BiddyIsSmaller(v,tagF)) {
          Fneg_v = Fv = f;
        } else {
          Fneg_v = f;
          Biddy_SetTag(Fneg_v,v);
          Fneg_v = Biddy_Managed_IncTag(MNG,Fneg_v);
          Fv = biddyZero;
        }
      }
      if (topG == v) {
        Gneg_v = BiddyE(g);
        Gv = BiddyT(g);
      } else {
        if (BiddyIsSmaller(v,tagG)) {
          Gneg_v = Gv = g;
        } else {
          Gneg_v = g;
          Biddy_SetTag(Gneg_v,v);
          Gneg_v = Biddy_Managed_IncTag(MNG,Gneg_v);
          Gv = biddyZero;
        }
      }
      */

      /* VARIANT 3 */
      /**/
      if (BiddyIsSmaller(v,tagF)) {
        Fneg_v = Fv = f;
      } else if (v == topF) {
        Fneg_v = BiddyE(f);
        Fv = BiddyT(f);
      } else {
        Fneg_v = f;
        Biddy_SetTag(Fneg_v,v);
        Fneg_v = Biddy_Managed_IncTag(MNG,Fneg_v);
        Fv = biddyZero;
      }
      if (BiddyIsSmaller(v,tagG)) {
        Gneg_v = Gv = g;
      } else if (v == topG) {
        Gneg_v = BiddyE(g);
        Gv = BiddyT(g);
      } else {
        Gneg_v = g;
        Biddy_SetTag(Gneg_v,v);
        Gneg_v = Biddy_Managed_IncTag(MNG,Gneg_v);
        Gv = biddyZero;
      }
      /**/

    }

    /* RECURSIVE CALLS */
    if (Fneg_v == biddyZero) {
      E = biddyZero;
    } else if (Gneg_v == biddyZero) {
      E = biddyZero;
    } else if (Fneg_v == biddyOne) {
      E = Gneg_v;
    } else if (Gneg_v == biddyOne) {
      E = Fneg_v;
    } else  {
      E = BiddyManagedAnd(MNG,Fneg_v,Gneg_v);
    }
    if (Fv == biddyZero) {
      T = biddyZero;
    } else if (Gv == biddyZero) {
      T = biddyZero;
    } else if (Fv == biddyOne) {
      T = Gv;
    } else if (Gv == biddyOne) {
      T = Fv;
    } else  {
      T = BiddyManagedAnd(MNG,Fv,Gv);
    }

    r = BiddyManagedTaggedFoaNode(MNG,v,E,T,rtag,TRUE);
    BiddyRefresh(r); /* FoaNode returns an obsolete node! */

    if (NN) {
      addOp3Cache(MNG,biddyOPCache,FF,GG,HH,Biddy_Inv(r),cindex);
    } else {
      addOp3Cache(MNG,biddyOPCache,FF,GG,HH,r,cindex);
    }

  } else {

    if (NN) {
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
    Implemented for OBDDs, ZBDDs, and TZBDDs.
    For OBDDs, results are cached as parameters to ITE(F,G,H)= F*G XOR F'*H.
    For all other BDD types, results are cached as (biddyZero,f,g).
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

  r = biddyNull;

  if (biddyManagerType == BIDDYTYPEOBDD) {
    /* PROCEED BY CALCULATING NOT-AND */
    /*
    r = BiddyManagedNot(MNG,BiddyManagedAnd(MNG,BiddyManagedNot(MNG,f),BiddyManagedNot(MNG,g)));
    */
    /* PROCEED BY CALCULATING XOR-AND */
    /*
    r = BiddyManagedXor(MNG,BiddyManagedXor(MNG,f,g),BiddyManagedAnd(MNG,f,g));
    */
    /* IMPLEMENTED */
    r = BiddyManagedOr(MNG,f,g);
    BiddyRefresh(r); /* not always refreshed by BiddyManagedOr */
  } else if (biddyManagerType == BIDDYTYPEOBDDC) {
    /* PROCEED BY CALCULATING NOT-AND */
    /*
    r = Biddy_Inv(BiddyManagedAnd(MNG,Biddy_Inv(f),Biddy_Inv(g)));
    */
    /* PROCEED BY CALCULATING XOR-AND */
    /*
    r = BiddyManagedXor(MNG,BiddyManagedXor(MNG,f,g),BiddyManagedAnd(MNG,f,g));
    */
    /* IMPLEMENTED */
    r = BiddyManagedOr(MNG,f,g);
    BiddyRefresh(r); /* not always refreshed by BiddyManagedOr */
  } else if (biddyManagerType == BIDDYTYPEZBDDC) {
    /* PROCEED BY CALCULATING NOT-AND */
    /*
    r = BiddyManagedNot(MNG,BiddyManagedAnd(MNG,BiddyManagedNot(MNG,f),BiddyManagedNot(MNG,g)));
    */
    /* PROCEED BY CALCULATING XOR-AND */
    /*
    r = BiddyManagedXor(MNG,BiddyManagedXor(MNG,f,g),BiddyManagedAnd(MNG,f,g));
    */
    /* IMPLEMENTED */
    r = BiddyManagedOr(MNG,f,g);
    BiddyRefresh(r); /* not always refreshed by BiddyManagedOr */
  } else if (biddyManagerType == BIDDYTYPETZBDD) {
    /* PROCEED BY CALCULATING NOT-AND */
    /*
    r = BiddyManagedNot(MNG,BiddyManagedAnd(MNG,BiddyManagedNot(MNG,f),BiddyManagedNot(MNG,g)));
    */
    /* PROCEED BY CALCULATING XOR-AND */
    /*
    r = BiddyManagedXor(MNG,BiddyManagedXor(MNG,f,g),BiddyManagedAnd(MNG,f,g));
    */
    /* IMPLEMENTED */
    r = BiddyManagedOr(MNG,f,g);
    BiddyRefresh(r); /* not always refreshed by BiddyManagedOr */
  } else if ((biddyManagerType == BIDDYTYPEZBDD) ||
              (biddyManagerType == BIDDYTYPETZBDDC))
  {
    fprintf(stderr,"Biddy_Or: this GDD type is not supported, yet!\n");
    return biddyNull;
  } else if ((biddyManagerType == BIDDYTYPEOFDDC) || (biddyManagerType == BIDDYTYPEOFDD) ||
              (biddyManagerType == BIDDYTYPEZFDDC) || (biddyManagerType == BIDDYTYPEZFDD) ||
              (biddyManagerType == BIDDYTYPETZFDDC) || (biddyManagerType == BIDDYTYPETZFDD))
  {
    fprintf(stderr,"Biddy_Or: this GDD type is not supported, yet!\n");
    return biddyNull;
  } else {
    fprintf(stderr,"Biddy_Or: Unsupported GDD type!\n");
    return biddyNull;
  }

  return r;
}

#ifdef __cplusplus
}
#endif

Biddy_Edge
BiddyManagedOr(const Biddy_Manager MNG, const Biddy_Edge f, const Biddy_Edge g)
{
  Biddy_Edge r, T, E, Fv, Gv, Fneg_v, Gneg_v;
  Biddy_Edge FF, GG, HH;
  Biddy_Boolean NN;
  Biddy_Variable v,rtag;
  unsigned int cindex;

  static Biddy_Variable topF;    /* CAN BE STATIC, WHAT IS BETTER? */
  static Biddy_Variable topG;    /* CAN BE STATIC, WHAT IS BETTER? */
  static Biddy_Variable tagF;    /* CAN BE STATIC, WHAT IS BETTER? */
  static Biddy_Variable tagG;    /* CAN BE STATIC, WHAT IS BETTER? */

  assert( MNG != NULL );
  assert( f != NULL );
  assert( g != NULL );

  /* IMPLEMENTED FOR OBDD, OBDDC, ZBDDC, AND TZBDD */
  assert(
    (biddyManagerType == BIDDYTYPEOBDD) ||
    (biddyManagerType == BIDDYTYPEOBDDC) ||
    (biddyManagerType == BIDDYTYPEZBDDC) ||
    (biddyManagerType == BIDDYTYPETZBDD)
  );

#ifdef BIDDYEXTENDEDSTATS_YES
  biddyNodeTable.andorrecursive++;
#endif

  r = biddyNull;

  /* LOOKING FOR SIMPLE CASE */

  if (f == biddyZero) {
    return g;
  } else if (g == biddyZero) {
    return f;
  } else if (f == biddyOne) {
    return biddyOne;
  } else if (g == biddyOne) {
    return biddyOne;
  } else if (f == g) {
    return f;
  }

  if (biddyManagerType == BIDDYTYPEOBDDC) {
    if (Biddy_IsEqvPointer(f,g)) {
      return biddyOne;
    }
  }

  if (biddyManagerType == BIDDYTYPETZBDD) {
    if (BiddyR(f) == BiddyR(g)) {
      if (BiddyIsSmaller(Biddy_GetTag(f),Biddy_GetTag(g))) {
        return g;
      } else {
        return f;
      }
    }
  }

  /* THIS IS NOT A SIMPLE CASE */

  FF = GG = HH = biddyNull;
  NN = FALSE;
  Fneg_v = Fv = biddyNull;
  Gneg_v = Gv = biddyNull;
  rtag = 0;
  v = 0;

  /* NORMALIZATION OF COMPLEMENTED EDGES */
  /* FF, GG, and HH ARE USED FOR CACHE LOOKUP, ONLY */
  /* FF AND GG SHOULD NOT BE COMPLEMENTED */
  /* ((uintptr_t) f) SHOULD BE GREATER THAN ((uintptr_t) g) */

  if (biddyManagerType == BIDDYTYPEOBDDC) {

    if (((uintptr_t) f) > ((uintptr_t) g)) {
      if (Biddy_GetMark(f)) {
        if (Biddy_GetMark(g)) {
          NN = TRUE;
          FF = Biddy_Inv(f);
          GG = Biddy_Inv(g);
          HH = biddyZero;  /* Biddy_Inv(g) */
        } else {
          FF = Biddy_Inv(f);
          GG = g;
          HH = biddyOne; /* g */
        }
      } else {
        FF = f;
        GG = biddyOne;
        HH = g;
      }
    } else {
      if (Biddy_GetMark(g)) {
        if (Biddy_GetMark(f)) {
          NN = TRUE;
          FF = Biddy_Inv(g);
          GG = Biddy_Inv(f);
          HH = biddyZero;  /* Biddy_Inv(g) */
        } else {
          FF = Biddy_Inv(g);
          GG = f;
          HH = biddyOne; /* g */
        }
      } else {
        FF = g;
        GG = biddyOne;
        HH = f;
      }
    }
  }

  else {

    /* THIS IS NOT ITE CACHE! */

    if (((uintptr_t) f) > ((uintptr_t) g)) {
      FF = biddyZero;
      GG = f;
      HH = g;
    } else {
      FF = biddyZero;
      GG = g;
      HH = f;
    }

  }

  assert ( f != biddyZero );
  assert ( f != biddyOne );
  assert ( g != biddyZero );
  assert ( g != biddyOne );

  /* IF RESULT IS NOT IN THE CACHE TABLE... */
  cindex = 0;
  if (!findOp3Cache(MNG,biddyOPCache,FF,GG,HH,&r,&cindex)) {

    if ((biddyManagerType == BIDDYTYPEOBDDC) || (biddyManagerType == BIDDYTYPEOBDD)) {

      /* LOOKING FOR THE SMALLEST TOP VARIABLE */
      /* CONSTANT NODE MUST HAVE MAX ORDER */
      topF = BiddyV(f);
      topG = BiddyV(g);
      v = BiddyIsSmaller(topF,topG) ? topF : topG;

      /* DETERMINING PARAMETERS FOR RECURSIVE CALLS */
      /* COMPLEMENTED EDGES MUST BE TRANSFERED */
      if (topF == v) {
        Fneg_v = Biddy_InvCond(BiddyE(f),Biddy_GetMark(f));
        Fv = Biddy_InvCond(BiddyT(f),Biddy_GetMark(f));
      } else {
        Fneg_v = Fv = f;
      }

      if (topG == v) {
        Gneg_v = Biddy_InvCond(BiddyE(g),Biddy_GetMark(g));
        Gv = Biddy_InvCond(BiddyT(g),Biddy_GetMark(g));
      } else {
        Gneg_v = Gv = g;
      }

      rtag = v;

    }

    else if (biddyManagerType == BIDDYTYPEZBDDC) {

      /* LOOKING FOR THE SMALLEST TOP VARIABLE */
      /* CONSTANT NODE MUST HAVE MAX ORDER */
      topF = BiddyV(f);
      topG = BiddyV(g);
      v = BiddyIsSmaller(topF,topG) ? topF : topG;

      /* DETERMINING PARAMETERS FOR RECURSIVE CALLS */
      /* COMPLEMENTED EDGES MUST BE TRANSFERED */
      if (topF == v) {
        Fneg_v = Biddy_InvCond(BiddyE(f),Biddy_GetMark(f));
        Fv = BiddyT(f);
      } else {
        Fneg_v = f;
        Fv = biddyZero;
      }
      if (topG == v) {
        Gneg_v = Biddy_InvCond(BiddyE(g),Biddy_GetMark(g));
        Gv = BiddyT(g);
      } else {
         Gneg_v = g;
         Gv = biddyZero;
      }

      rtag = 0;

    }

    else if (biddyManagerType == BIDDYTYPETZBDD) {

      /* VARIANT 1 */
      /*
      tagF = Biddy_GetTag(f);
      tagG = Biddy_GetTag(g);
      if (tagF == tagG) {
        / * THE SAME TAG * /
        topF = BiddyV(f);
        topG = BiddyV(g);
        v = BiddyIsSmaller(topF,topG) ? topF : topG;
        rtag = tagF;
        w = v;
        if (topF == v) {
          Fneg_v = BiddyE(f);
          Fv = BiddyT(f);
        } else {
          Fneg_v = f;
          Biddy_SetTag(Fneg_v,v);
          Fneg_v = Biddy_Managed_IncTag(MNG,Fneg_v);
          Fv = biddyZero;
        }
        if (topG == v) {
          Gneg_v = BiddyE(g);
          Gv = BiddyT(g);
        } else {
          Gneg_v = g;
          Biddy_SetTag(Gneg_v,v);
          Gneg_v = Biddy_Managed_IncTag(MNG,Gneg_v);
          Gv = biddyZero;
        }
      } else {
        / * DIFFERENT TAG * /
        rtag = BiddyIsSmaller(tagF,tagG) ? tagF : tagG;
        w = rtag;
        if (tagF == rtag) {
          if (BiddyV(f) == rtag) {
            / * TAG AND TOP VARIABLE ARE EQUAL * /
            Fneg_v = BiddyE(f);
            Fv = BiddyT(f);
          } else {
            / * TAG AND TOP VARIABLE ARE NOT EQUAL * /
            Fneg_v = f;
            Biddy_SetTag(Fneg_v,rtag);
            Fneg_v = Biddy_Managed_IncTag(MNG,Fneg_v);
            Fv = biddyZero;
          }
          Gneg_v = Gv = g;
        } else {
          if (BiddyV(g) == rtag) {
            / * TAG AND TOP VARIABLE ARE EQUAL * /
            Gneg_v = BiddyE(g);
            Gv = BiddyT(g);
          } else {
            / * TAG AND TOP VARIABLE ARE NOT EQUAL * /
            Gneg_v = g;
            Biddy_SetTag(Gneg_v,rtag);
            Gneg_v = Biddy_Managed_IncTag(MNG,Gneg_v);
            Gv = biddyZero;
          }
          Fneg_v = Fv = f;
        }
      }
      */

      /* VARIANT 2 */
      /**/
      tagF = Biddy_GetTag(f);
      tagG = Biddy_GetTag(g);
      topF = BiddyV(f);
      topG = BiddyV(g);
      rtag = BiddyIsSmaller(tagF,tagG) ? tagF : tagG;
      v = BiddyIsSmaller(topF,topG) ? topF : topG;
      if (tagF != tagG) v = rtag;
      if (tagF == rtag) {
        if (topF == v) {
          Fneg_v = BiddyE(f);
          Fv = BiddyT(f);
        } else {
          Fneg_v = f;
          Biddy_SetTag(Fneg_v,v);
          Fneg_v = Biddy_Managed_IncTag(MNG,Fneg_v);
          Fv = biddyZero;
        }
      } else {
        Fneg_v = Fv = f;
      }
      if (tagG == rtag) {
        if (topG == v) {
          Gneg_v = BiddyE(g);
          Gv = BiddyT(g);
        } else {
          Gneg_v = g;
          Biddy_SetTag(Gneg_v,v);
          Gneg_v = Biddy_Managed_IncTag(MNG,Gneg_v);
          Gv = biddyZero;
        }
      } else {
        Gneg_v = Gv = g;
      }
      /**/

    }

    /* RECURSIVE CALLS */
    if (Fneg_v == biddyZero) {
      E = Gneg_v;
    } else if (Gneg_v == biddyZero) {
      E = Fneg_v;
    } else if (Fneg_v == biddyOne) {
      E = biddyOne;
    } else if (Gneg_v == biddyOne) {
      E = biddyOne;
    } else  {
      E = BiddyManagedOr(MNG,Fneg_v,Gneg_v);
    }
    if (Fv == biddyZero) {
      T = Gv;
    } else if (Gv == biddyZero) {
      T = Fv;
    } else if (Fv == biddyOne) {
      T = biddyOne;
    } else if (Gv == biddyOne) {
      T = biddyOne;
    } else  {
      T = BiddyManagedOr(MNG,Fv,Gv);
    }

    r = BiddyManagedTaggedFoaNode(MNG,v,E,T,rtag,TRUE);
    BiddyRefresh(r); /* FoaNode returns an obsolete node! */

    if (NN) {
      addOp3Cache(MNG,biddyOPCache,FF,GG,HH,Biddy_Inv(r),cindex);
    } else {
      addOp3Cache(MNG,biddyOPCache,FF,GG,HH,r,cindex);
    }

  } else {

    if (NN) {
      Biddy_InvertMark(r);
    }

    /* IF THE RESULT IS FROM CACHE TABLE, REFRESH IT! */
    BiddyRefresh(r);

  }

  return r;
}

/***************************************************************************//*!
\brief Function Biddy_Managed_Nand calculates Boolean function NAND
       (Sheffer).

### Description
### Side Effects
    Implemented for OBDDC. Prototyped for OBDD.
    Prototyped for ZBDDs and TZBDDs (via and-not).
    For OBDDs, results are cached as parameters to ITE(F,G,H)= F*G XOR F'*H.
    For ZBDDs and TZBDDs, results could be cached as (f,g,biddyNull).
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

  r = biddyNull;

  if (biddyManagerType == BIDDYTYPEOBDD) {
    /* PROTOTYPED */
    r = BiddyManagedNot(MNG,BiddyManagedAnd(MNG,f,g));
    BiddyRefresh(r); /* not always refreshed by BiddyManagedNot */
  } else if (biddyManagerType == BIDDYTYPEOBDDC) {
    /* IMPLEMENTED */
    r = BiddyManagedNand(MNG,f,g);
    BiddyRefresh(r); /* not always refreshed by BiddyManagedNand */
  } else if (biddyManagerType == BIDDYTYPEZBDDC) {
    /* PROTOTYPED */
    r = BiddyManagedNot(MNG,BiddyManagedAnd(MNG,f,g));
    BiddyRefresh(r); /* not always refreshed by BiddyManagedNot */
  } else if (biddyManagerType == BIDDYTYPETZBDD) {
    /* PROTOTYPED */
    r = BiddyManagedNot(MNG,BiddyManagedAnd(MNG,f,g));
    BiddyRefresh(r); /* not always refreshed by BiddyManagedNot */
  } else if ((biddyManagerType == BIDDYTYPEZBDD) ||
              (biddyManagerType == BIDDYTYPETZBDDC))
  {
    fprintf(stderr,"Biddy_Nand: this GDD type is not supported, yet!\n");
    return biddyNull;
  } else if ((biddyManagerType == BIDDYTYPEOFDDC) || (biddyManagerType == BIDDYTYPEOFDD) ||
              (biddyManagerType == BIDDYTYPEZFDDC) || (biddyManagerType == BIDDYTYPEZFDD) ||
              (biddyManagerType == BIDDYTYPETZFDDC) || (biddyManagerType == BIDDYTYPETZFDD))
  {
    fprintf(stderr,"Biddy_Nand: this GDD type is not supported, yet!\n");
    return biddyNull;
  } else {
    fprintf(stderr,"Biddy_Nand: Unsupported GDD type!\n");
    return biddyNull;
  }

  return r;
}

#ifdef __cplusplus
}
#endif

Biddy_Edge
BiddyManagedNand(const Biddy_Manager MNG, const Biddy_Edge f,
                 const Biddy_Edge g)
{
  Biddy_Edge r;

  assert( MNG != NULL );
  assert( f != NULL );
  assert( g != NULL );

  /* IMPLEMENTED FOR OBDDC, ONLY */
  assert( biddyManagerType == BIDDYTYPEOBDDC );

  r = biddyNull;

  if (biddyManagerType == BIDDYTYPEOBDDC) {
    r = Biddy_Inv(BiddyManagedAnd(MNG,f,g));
  }

  return r;
}

/***************************************************************************//*!
\brief Function Biddy_Managed_Nor calculates Boolean function NOR
       (Peirce).

### Description
### Side Effects
    Implemented for OBDDC. Prototyped for OBDD.
    Prototyped for ZBDDs and TZBDDs (via or-not).
    For OBDDs, results are cached as parameters to ITE(F,G,H)= F*G XOR F'*H.
    For ZBDDs and TZBDDs, results could be cached as (biddyNull,f,g).
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

  r = biddyNull;

  if (biddyManagerType == BIDDYTYPEOBDD) {
    /* PROTOTYPED */
    r = BiddyManagedNot(MNG,BiddyManagedOr(MNG,f,g));
    BiddyRefresh(r); /* not always refreshed by BiddyManagedNot */
  } else if (biddyManagerType == BIDDYTYPEOBDDC) {
    /* IMPLEMENTED */
    r = BiddyManagedNor(MNG,f,g);
    BiddyRefresh(r); /* not always refreshed by BiddyManagedNor */
  } else if (biddyManagerType == BIDDYTYPEZBDDC) {
    /* PROTOTYPED */
    r = BiddyManagedNot(MNG,BiddyManagedOr(MNG,f,g));
    BiddyRefresh(r); /* not always refreshed by BiddyManagedNot */
  } else if (biddyManagerType == BIDDYTYPETZBDD) {
    /* PROTOTYPED */
    r = BiddyManagedNot(MNG,BiddyManagedOr(MNG,f,g));
    BiddyRefresh(r); /* not always refreshed by BiddyManagedNot */
  } else if ((biddyManagerType == BIDDYTYPEZBDD) ||
              (biddyManagerType == BIDDYTYPETZBDDC))
  {
    fprintf(stderr,"Biddy_Nor: this GDD type is not supported, yet!\n");
    return biddyNull;
  } else if ((biddyManagerType == BIDDYTYPEOFDDC) || (biddyManagerType == BIDDYTYPEOFDD) ||
              (biddyManagerType == BIDDYTYPEZFDDC) || (biddyManagerType == BIDDYTYPEZFDD) ||
              (biddyManagerType == BIDDYTYPETZFDDC) || (biddyManagerType == BIDDYTYPETZFDD))
  {
    fprintf(stderr,"Biddy_Nor: this GDD type is not supported, yet!\n");
    return biddyNull;
  } else {
    fprintf(stderr,"Biddy_Nor: Unsupported GDD type!\n");
    return biddyNull;
  }

  return r;
}

#ifdef __cplusplus
}
#endif

Biddy_Edge
BiddyManagedNor(const Biddy_Manager MNG, const Biddy_Edge f,
                const Biddy_Edge g)
{
  Biddy_Edge r;

  assert( MNG != NULL );
  assert( f != NULL );
  assert( g != NULL );

  /* IMPLEMENTED FOR OBDDC, ONLY */
  assert( biddyManagerType == BIDDYTYPEOBDDC );

  r = biddyNull;

  if (biddyManagerType == BIDDYTYPEOBDDC) {
    r = BiddyManagedAnd(MNG,Biddy_Inv(f),Biddy_Inv(g));
  }

  return r;
}

/***************************************************************************//*!
\brief Function Biddy_Managed_Xor calculates Boolean function XOR.

### Description
### Side Effects
    Used by ITE (for OBDDC).
    Implemented for OBDDs, ZBDDs, and TZBDDs.
    For OBDD, results are cached as parameters to ITE(F,G,H)= F*G XOR F'*H.
    For all other BDD types, results are cached as (f,biddyZero,g).
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

  r = biddyNull;

  if (biddyManagerType == BIDDYTYPEOBDD) {
    /* IMPLEMENTED */
    r = BiddyManagedXor(MNG,f,g);
    BiddyRefresh(r); /* not always refreshed by BiddyManagedXor */
  } else if (biddyManagerType == BIDDYTYPEOBDDC) {
    /* IMPLEMENTED */
    r = BiddyManagedXor(MNG,f,g);
    BiddyRefresh(r); /* not always refreshed by BiddyManagedXor */
  } else if (biddyManagerType == BIDDYTYPEZBDDC) {
    /* IMPLEMENTED */
    r = BiddyManagedXor(MNG,f,g);
    BiddyRefresh(r); /* not always refreshed by BiddyManagedXor */
  } else if (biddyManagerType == BIDDYTYPETZBDD) {
    /* IMPLEMENTED */
    r = BiddyManagedXor(MNG,f,g);
    BiddyRefresh(r); /* not always refreshed by BiddyManagedXor */
  } else if ((biddyManagerType == BIDDYTYPEZBDD) ||
              (biddyManagerType == BIDDYTYPETZBDDC))
  {
    fprintf(stderr,"Biddy_Xor: this GDD type is not supported, yet!\n");
    return biddyNull;
  } else if ((biddyManagerType == BIDDYTYPEOFDDC) || (biddyManagerType == BIDDYTYPEOFDD) ||
              (biddyManagerType == BIDDYTYPEZFDDC) || (biddyManagerType == BIDDYTYPEZFDD) ||
              (biddyManagerType == BIDDYTYPETZFDDC) || (biddyManagerType == BIDDYTYPETZFDD))
  {
    fprintf(stderr,"Biddy_Xor: this GDD type is not supported, yet!\n");
    return biddyNull;
  } else {
    fprintf(stderr,"Biddy_Xor: Unsupported GDD type!\n");
    return biddyNull;
  }

  return r;
}

#ifdef __cplusplus
}
#endif

Biddy_Edge
BiddyManagedXor(const Biddy_Manager MNG, const Biddy_Edge f,
                const Biddy_Edge g)
{
  Biddy_Edge r, T, E, Fv, Gv, Fneg_v, Gneg_v;
  Biddy_Edge FF, GG, HH;
  Biddy_Boolean NN;
  Biddy_Variable v,rtag;
  unsigned int cindex;

  static Biddy_Variable topF;    /* CAN BE STATIC, WHAT IS BETTER? */
  static Biddy_Variable topG;    /* CAN BE STATIC, WHAT IS BETTER? */
  static Biddy_Variable tagF;    /* CAN BE STATIC, WHAT IS BETTER? */
  static Biddy_Variable tagG;    /* CAN BE STATIC, WHAT IS BETTER? */

  assert( MNG != NULL );
  assert( f != NULL );
  assert( g != NULL );

  /* IMPLEMENTED FOR OBDD, OBDDC, ZBDDC, AND TZBDD */
  assert(
    (biddyManagerType == BIDDYTYPEOBDD) ||
    (biddyManagerType == BIDDYTYPEOBDDC) ||
    (biddyManagerType == BIDDYTYPEZBDDC) ||
    (biddyManagerType == BIDDYTYPETZBDD)
  );

#ifdef BIDDYEXTENDEDSTATS_YES
  biddyNodeTable.xorrecursive++;
#endif

  /* LOOKING FOR SIMPLE CASE */

  if (f == biddyZero) {
    return g;
  } else if (g == biddyZero) {
    return f;
  } else if (f == g) {
    return biddyZero;
  }

  if (biddyManagerType == BIDDYTYPEOBDDC) {
    if (f == biddyOne) {
      r = Biddy_Inv(g);
      return r;
    } else if (g == biddyOne) {
      r = Biddy_Inv(f);
      return r;
    } else if (Biddy_IsEqvPointer(f,g)) {
      return biddyOne;
    }
  }

  else if (biddyManagerType == BIDDYTYPEOBDD) {
    if (f == biddyOne) {
      r = BiddyManagedNot(MNG,g);
      return r;
    } else if (g == biddyOne) {
      r = BiddyManagedNot(MNG,f);
      return r;
    }
  }

  else if (biddyManagerType == BIDDYTYPEZBDDC) {
    if (f == biddyOne) {
      r = BiddyManagedNot(MNG,g);
      return r;
    } else if (g == biddyOne) {
      r = BiddyManagedNot(MNG,f);
      return r;
    }
  }

  else if (biddyManagerType == BIDDYTYPETZBDD) {
    if (f == biddyOne) {
      r = BiddyManagedNot(MNG,g);
      return r;
    } else if (g == biddyOne) {
      r = BiddyManagedNot(MNG,f);
      return r;
    }
  }

  /* THIS IS NOT A SIMPLE CASE */

  FF = GG = HH = biddyNull;
  NN = FALSE;
  Fneg_v = Fv = biddyNull;
  Gneg_v = Gv = biddyNull;
  rtag = 0;
  v = 0;

  /* NORMALIZATION OF COMPLEMENTED EDGES */
  /* FF, GG, and HH ARE USED FOR CACHE LOOKUP, ONLY */
  /* FF AND GG SHOULD NOT BE COMPLEMENTED */
  /* ((uintptr_t) f) SHOULD BE GREATER THAN ((uintptr_t) g) */

  if (biddyManagerType == BIDDYTYPEOBDDC) {

    if (((uintptr_t) f) > ((uintptr_t) g)) {
      if (Biddy_GetMark(f)) {
        if (Biddy_GetMark(g)) {
          FF = Biddy_Inv(f);
          GG = Biddy_Inv(g); /* h */
          HH = g;
        } else {
          NN = TRUE;
          FF = Biddy_Inv(f);
          GG = g;
          HH = Biddy_Inv(g); /* h */
        }
      } else {
        if (Biddy_GetMark(g)) {
          NN = TRUE;
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
          NN = TRUE;
          FF = Biddy_Inv(g);
          GG = f;
          HH = Biddy_Inv(f); /* h */
        }
      } else {
        if (Biddy_GetMark(f)) {
          NN = TRUE;
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

  }

  else {

    /* THIS IS NOT ITE CACHE! */

    if (((uintptr_t) f) > ((uintptr_t) g)) {
      FF = f;
      GG = biddyZero;
      HH = g;
    } else {
      FF = g;
      GG = biddyZero;
      HH = f;
    }

  }

  assert ( f != biddyZero );
  assert ( f != biddyOne );
  assert ( g != biddyZero );
  assert ( g != biddyOne );

  /* IF RESULT IS NOT IN THE CACHE TABLE... */
  cindex = 0;
  if (!findOp3Cache(MNG,biddyOPCache,FF,GG,HH,&r,&cindex))
  {

    if ((biddyManagerType == BIDDYTYPEOBDDC) || (biddyManagerType == BIDDYTYPEOBDD)) {

      /* LOOKING FOR THE SMALLEST TOP VARIABLE */
      /* CONSTANT NODE MUST HAVE MAX ORDER */
      topF = BiddyV(f);
      topG = BiddyV(g);
      v = BiddyIsSmaller(topF,topG) ? topF : topG;

      /* DETERMINING PARAMETERS FOR RECURSIVE CALLS */
      /* COMPLEMENTED EDGES MUST BE TRANSFERED */
      if (topF == v) {
        Fneg_v = Biddy_InvCond(BiddyE(f),Biddy_GetMark(f));
        Fv = Biddy_InvCond(BiddyT(f),Biddy_GetMark(f));
      } else {
        Fneg_v = Fv = f;
      }
      if (topG == v) {
        Gneg_v = Biddy_InvCond(BiddyE(g),Biddy_GetMark(g));
        Gv = Biddy_InvCond(BiddyT(g),Biddy_GetMark(g));
      } else {
        Gneg_v = Gv = g;
      }

      rtag = v;

    }

    else if (biddyManagerType == BIDDYTYPEZBDDC) {

      /* LOOKING FOR THE SMALLEST TOP VARIABLE */
      /* CONSTANT NODE MUST HAVE MAX ORDER */
      topF = BiddyV(f);
      topG = BiddyV(g);
      v = BiddyIsSmaller(topF,topG) ? topF : topG;

      /* DETERMINING PARAMETERS FOR RECURSIVE CALLS */
      /* COMPLEMENTED EDGES MUST BE TRANSFERED */
      if (topF == v) {
        Fneg_v = Biddy_InvCond(BiddyE(f),Biddy_GetMark(f));
        Fv = BiddyT(f);
      } else {
        Fneg_v = f;
        Fv = biddyZero;
      }
      if (topG == v) {
        Gneg_v = Biddy_InvCond(BiddyE(g),Biddy_GetMark(g));
        Gv = BiddyT(g);
      } else {
        Gneg_v = g;
        Gv = biddyZero;
      }

      rtag = 0;

    }

    else if (biddyManagerType == BIDDYTYPETZBDD) {

      /* VARIANT 1 */
      /*
      tagF = Biddy_GetTag(f);
      tagG = Biddy_GetTag(g);
      if (tagF == tagG) {
        / * THE SAME TAG * /
        topF = BiddyV(f);
        topG = BiddyV(g);
        v = BiddyIsSmaller(topF,topG) ? topF : topG;
        rtag = tagF;
        w = v;
        if (topF == v) {
          Fneg_v = BiddyE(f);
          Fv = BiddyT(f);
        } else {
          Fneg_v = f;
          Biddy_SetTag(Fneg_v,v);
          Fneg_v = Biddy_Managed_IncTag(MNG,Fneg_v);
          Fv = biddyZero;
        }
        if (topG == v) {
          Gneg_v = BiddyE(g);
          Gv = BiddyT(g);
        } else {
          Gneg_v = g;
          Biddy_SetTag(Gneg_v,v);
          Gneg_v = Biddy_Managed_IncTag(MNG,Gneg_v);
          Gv = biddyZero;
        }
      } else {
        / * DIFFERENT TAG * /
        rtag = BiddyIsSmaller(tagF,tagG) ? tagF : tagG;
        w = rtag;
        if (tagF == rtag) {
          if (BiddyV(f) == rtag) {
            / * TAG AND TOP VARIABLE ARE EQUAL * /
            Fneg_v = BiddyE(f);
            Fv = BiddyT(f);
          } else {
            / * TAG AND TOP VARIABLE ARE NOT EQUAL * /
            Fneg_v = f;
            Biddy_SetTag(Fneg_v,rtag);
            Fneg_v = Biddy_Managed_IncTag(MNG,Fneg_v);
            Fv = biddyZero;
          }
          Gneg_v = Gv = g;
        } else {
          if (BiddyV(g) == rtag) {
            / * TAG AND TOP VARIABLE ARE EQUAL * /
            Gneg_v = BiddyE(g);
            Gv = BiddyT(g);
          } else {
            / * TAG AND TOP VARIABLE ARE NOT EQUAL * /
            Gneg_v = g;
            Biddy_SetTag(Gneg_v,rtag);
            Gneg_v = Biddy_Managed_IncTag(MNG,Gneg_v);
            Gv = biddyZero;
          }
          Fneg_v = Fv = f;
        }
      }
      */

      /* VARIANT 2 */
      /**/
      tagF = Biddy_GetTag(f);
      tagG = Biddy_GetTag(g);
      topF = BiddyV(f);
      topG = BiddyV(g);
      rtag = BiddyIsSmaller(tagF,tagG) ? tagF : tagG;
      v = BiddyIsSmaller(topF,topG) ? topF : topG;
      if (tagF != tagG) v = rtag;
      if (tagF == rtag) {
        if (topF == v) {
          Fneg_v = BiddyE(f);
          Fv = BiddyT(f);
        } else {
          Fneg_v = f;
          Biddy_SetTag(Fneg_v,v);
          Fneg_v = Biddy_Managed_IncTag(MNG,Fneg_v);
          Fv = biddyZero;
        }
      } else {
        Fneg_v = Fv = f;
      }
      if (tagG == rtag) {
        if (topG == v) {
          Gneg_v = BiddyE(g);
          Gv = BiddyT(g);
        } else {
          Gneg_v = g;
          Biddy_SetTag(Gneg_v,v);
          Gneg_v = Biddy_Managed_IncTag(MNG,Gneg_v);
          Gv = biddyZero;
        }
      } else {
        Gneg_v = Gv = g;
      }
      /**/

    }

    /* RECURSIVE CALLS */
    if (Fneg_v == biddyZero) {
      E = Gneg_v;
    } else if (Gneg_v == biddyZero) {
      E = Fneg_v;
    } else if ((Fneg_v == biddyOne) && (biddyManagerType == BIDDYTYPEOBDDC)) {
      E = Biddy_Inv(Gneg_v);
    } else if ((Gneg_v == biddyOne) && (biddyManagerType == BIDDYTYPEOBDDC)) {
      E = Biddy_Inv(Fneg_v);
    } else  {
      E = BiddyManagedXor(MNG,Fneg_v,Gneg_v);
    }
    if (Fv == biddyZero) {
      T = Gv;
    } else if (Gv == biddyZero) {
      T = Fv;
    } else if ((Fv == biddyOne) && (biddyManagerType == BIDDYTYPEOBDDC)) {
      T = Biddy_Inv(Gv);
    } else if ((Gv == biddyOne) && (biddyManagerType == BIDDYTYPEOBDDC)) {
      T = Biddy_Inv(Fv);
    } else  {
      T = BiddyManagedXor(MNG,Fv,Gv);
    }

    r = BiddyManagedTaggedFoaNode(MNG,v,E,T,rtag,TRUE);
    BiddyRefresh(r); /* FoaNode returns an obsolete node! */

    if (NN) {
      addOp3Cache(MNG,biddyOPCache,FF,GG,HH,Biddy_Inv(r),cindex);
    } else {
      addOp3Cache(MNG,biddyOPCache,FF,GG,HH,r,cindex);
    }

  } else {

    if (NN) {
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
    Implemented for OBDDs.
    Prototyped for ZBDDs and TZBDDs (via xor-not).
    For OBDD, results are cached as parameters to ITE(F,G,H)= F*G XOR F'*H.
    For ZBDDs and TZBDDs, results could be cached as (f,biddyNull,g).
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

  r = biddyNull;

  if (biddyManagerType == BIDDYTYPEOBDD) {
    /* PROTOTYPED */
    r = BiddyManagedNot(MNG,BiddyManagedXor(MNG,f,g));
    BiddyRefresh(r); /* not always refreshed by BiddyManagedNot */
  } else if (biddyManagerType == BIDDYTYPEOBDDC) {
    /* IMPLEMENTED */
    r = BiddyManagedXnor(MNG,f,g);
    BiddyRefresh(r); /* not always refreshed by BiddyManagedXnor */
  } else if (biddyManagerType == BIDDYTYPEZBDDC) {
    /* PROTOTYPED */
    r = BiddyManagedNot(MNG,BiddyManagedXor(MNG,f,g));
    BiddyRefresh(r); /* not always refreshed by BiddyManagedNot */
  } else if (biddyManagerType == BIDDYTYPETZBDD) {
    /* PROTOTYPED */
    r = BiddyManagedNot(MNG,BiddyManagedXor(MNG,f,g));
    BiddyRefresh(r); /* not always refreshed by BiddyManagedNot */
  } else if ((biddyManagerType == BIDDYTYPEZBDD) ||
              (biddyManagerType == BIDDYTYPETZBDDC))
  {
    fprintf(stderr,"Biddy_Xnor: this GDD type is not supported, yet!\n");
    return biddyNull;
  } else if ((biddyManagerType == BIDDYTYPEOFDDC) || (biddyManagerType == BIDDYTYPEOFDD) ||
              (biddyManagerType == BIDDYTYPEZFDDC) || (biddyManagerType == BIDDYTYPEZFDD) ||
              (biddyManagerType == BIDDYTYPETZFDDC) || (biddyManagerType == BIDDYTYPETZFDD))
  {
    fprintf(stderr,"Biddy_Xnor: this GDD type is not supported, yet!\n");
    return biddyNull;
  } else {
    fprintf(stderr,"Biddy_Xnor: Unsupported GDD type!\n");
    return biddyNull;
  }

  return r;
}

#ifdef __cplusplus
}
#endif

Biddy_Edge
BiddyManagedXnor(const Biddy_Manager MNG, const Biddy_Edge f,
                 const Biddy_Edge g)
{
  Biddy_Edge r;

  assert( MNG != NULL );
  assert( f != NULL );
  assert( g != NULL );

  r = biddyNull;

  /* IMPLEMENTED FOR OBDDC, ONLY */
  assert( biddyManagerType == BIDDYTYPEOBDDC );

  if (biddyManagerType == BIDDYTYPEOBDDC) {
    r = Biddy_Inv(BiddyManagedXor(MNG,f,g));
  }

  return r;
}

/***************************************************************************//*!
\brief Function Biddy_Managed_Leq calculates Boolean implication.

### Description
    Boolean function leq(f,g) = or(not(f),g) = not(gt(f,g)).
    This function coincides with implication f->g.
### Side Effects
    Implemented for OBDDC. Prototyped for OBDD.
    Prototyped for ZBDDs and TZBDDs (via gt-not).
    For OBDDC, results are cached as parameters to ITE(F,G,H)= F*G XOR F'*H.
    For all other BDD types, results are cached as (f,f,g).
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

  r = biddyNull;

  if (biddyManagerType == BIDDYTYPEOBDD) {
    /* IMPLEMENTED */
    r = BiddyManagedLeq(MNG,f,g);
    BiddyRefresh(r); /* not always refreshed by BiddyManagedLeq */
  } else if (biddyManagerType == BIDDYTYPEOBDDC) {
    /* IMPLEMENTED */
    r = BiddyManagedLeq(MNG,f,g);
    BiddyRefresh(r); /* not always refreshed by BiddyManagedLeq */
  } else if (biddyManagerType == BIDDYTYPEZBDDC) {
    /* PROTOTYPED */
    r = BiddyManagedNot(MNG,BiddyManagedGt(MNG,f,g));
    BiddyRefresh(r); /* not always refreshed by BiddyManagedNot */
  } else if (biddyManagerType == BIDDYTYPETZBDD) {
    /* PROTOTYPED */
    r = BiddyManagedNot(MNG,BiddyManagedGt(MNG,f,g));
    BiddyRefresh(r); /* not always refreshed by BiddyManagedNot */
  } else if ((biddyManagerType == BIDDYTYPEZBDD) ||
              (biddyManagerType == BIDDYTYPETZBDDC))
  {
    fprintf(stderr,"Biddy_Leq: this GDD type is not supported, yet!\n");
    return biddyNull;
  } else if ((biddyManagerType == BIDDYTYPEOFDDC) || (biddyManagerType == BIDDYTYPEOFDD) ||
              (biddyManagerType == BIDDYTYPEZFDDC) || (biddyManagerType == BIDDYTYPEZFDD) ||
              (biddyManagerType == BIDDYTYPETZFDDC) || (biddyManagerType == BIDDYTYPETZFDD))
  {
    fprintf(stderr,"Biddy_Leq: this GDD type is not supported, yet!\n");
    return biddyNull;
  } else {
    fprintf(stderr,"Biddy_Leq: Unsupported GDD type!\n");
    return biddyNull;
  }

  return r;
}

#ifdef __cplusplus
}
#endif

Biddy_Edge
BiddyManagedLeq(const Biddy_Manager MNG, const Biddy_Edge f, const Biddy_Edge g)
{
  Biddy_Edge r, T, E, Fv, Gv, Fneg_v, Gneg_v;
  Biddy_Edge FF, GG, HH;
  Biddy_Variable v,rtag;
  unsigned int cindex;

  static Biddy_Variable topF; /* CAN BE STATIC, WHAT IS BETTER? */
  static Biddy_Variable topG; /* CAN BE STATIC, WHAT IS BETTER? */
  /* static Biddy_Variable tagF; */ /* CAN BE STATIC, WHAT IS BETTER? */
  /* static Biddy_Variable tagG; */ /* CAN BE STATIC, WHAT IS BETTER? */

  assert( MNG != NULL );
  assert( f != NULL );
  assert( g != NULL );

/* THIS SHOULD BE ADDED WHEN DIRECTLY IMPLEMENTED */
/*
#ifdef BIDDYEXTENDEDSTATS_YES
  biddyNodeTable.andorrecursive++;
#endif
*/

  r = biddyNull;

  /* IMPLEMENTED FOR OBDD and OBDDC */
  assert(
    (biddyManagerType == BIDDYTYPEOBDD) ||
    (biddyManagerType == BIDDYTYPEOBDDC)
  );

  if (biddyManagerType == BIDDYTYPEOBDDC) {

    /* FOR OBDDC, PROCEED BY CALCULATING NOT-AND */

    r = Biddy_Inv(BiddyManagedAnd(MNG,f,Biddy_Inv(g)));
    return r;

  }

  /* LOOKING FOR SIMPLE CASE */

  if (f == biddyZero) {
    return biddyOne;
  } else if (g == biddyZero) {
    return BiddyManagedNot(MNG,f);
  } else if (f == biddyOne) {
    return g;
  } else if (g == biddyOne) {
    return biddyOne;
  } else if (f == g) {
    return biddyOne;
  }

  /* THIS IS NOT A SIMPLE CASE */

  FF = GG = HH = biddyNull;
  Fneg_v = Fv = biddyNull;
  Gneg_v = Gv = biddyNull;
  rtag = 0;
  v = 0;

  if (biddyManagerType == BIDDYTYPEOBDDC) {
    /* ALREADY CALCULATED */
  }

  else {

#ifdef BIDDYEXTENDEDSTATS_YES
    biddyNodeTable.andorrecursive++;
#endif

    /* FF, GG, and HH ARE USED FOR CACHE LOOKUP, ONLY */
    /* THIS IS NOT ITE CACHE! */

    FF = f;
    GG = f;
    HH = g;

  }

  assert ( f != biddyZero );
  assert ( f != biddyOne );
  assert ( g != biddyZero );
  assert ( g != biddyOne );

  /* IF RESULT IS NOT IN THE CACHE TABLE... */
  cindex = 0;
  if (!findOp3Cache(MNG,biddyOPCache,FF,GG,HH,&r,&cindex)) {

    if (biddyManagerType == BIDDYTYPEOBDDC) {
      /* ALREADY CALCULATED */
    }

    else if (biddyManagerType == BIDDYTYPEOBDD) {

      /* LOOKING FOR THE SMALLEST TOP VARIABLE */
      /* CONSTANT NODE MUST HAVE MAX ORDER */
      topF = BiddyV(f);
      topG = BiddyV(g);
      v = BiddyIsSmaller(topF,topG) ? topF : topG;

      /* DETERMINING PARAMETERS FOR RECURSIVE CALLS */
      /* COMPLEMENTED EDGES MUST BE TRANSFERED */
      if (topF == v) {
        Fneg_v = BiddyE(f);
        Fv = BiddyT(f);
      } else {
        Fneg_v = Fv = f;
      }

      if (topG == v) {
        Gneg_v = BiddyE(g);
        Gv = BiddyT(g);
      } else {
        Gneg_v = Gv = g;
      }

      rtag = v;

    }

    /* RECURSIVE CALLS */
    if (Fneg_v == biddyZero) {
      E = biddyOne;
    } else if (Gneg_v == biddyZero) {
      E = BiddyManagedNot(MNG,Fneg_v);
    } else if (Fneg_v == biddyOne) {
      E = Gneg_v;
    } else if (Gneg_v == biddyOne) {
      E = biddyOne;
    } else if (Fneg_v == Gneg_v) {
      E = biddyOne;
    } else  {
      E = BiddyManagedLeq(MNG,Fneg_v,Gneg_v);
    }
    if (Fv == biddyZero) {
      T = biddyOne;
    } else if (Gv == biddyZero) {
      T = BiddyManagedNot(MNG,Fv);
    } else if (Fv == biddyOne) {
      T = Gv;
    } else if (Gv == biddyOne) {
      T = biddyOne;
    } else if (Fv == Gv) {
      T = biddyOne;
    } else  {
      T = BiddyManagedLeq(MNG,Fv,Gv);
    }

    r = BiddyManagedTaggedFoaNode(MNG,v,E,T,rtag,TRUE);
    BiddyRefresh(r); /* FoaNode returns an obsolete node! */

    addOp3Cache(MNG,biddyOPCache,FF,GG,HH,r,cindex);

  } else {

    /* IF THE RESULT IS FROM CACHE TABLE, REFRESH IT! */
    BiddyRefresh(r);

  }

  return r;
}

/***************************************************************************//*!
\brief Function Biddy_Managed_Gt calculates the negation of Boolean implication.

### Description
    Boolean function gt(f,g) = and(f,not(g)).
    For combination sets, this function coincides with Diff.
### Side Effects
    Implemented for OBDDs, ZBDDs, and TZBDDs.
    For OBDD, results are cached as parameters to ITE(F,G,H)= F*G XOR F'*H.
    For ZBDDs and TZBDDs, results are cached as (f,g,g).
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

  r = biddyNull;

  if (biddyManagerType == BIDDYTYPEOBDD) {
    /* IMPLEMENTED */
    r = BiddyManagedGt(MNG,f,g);
    BiddyRefresh(r); /* not always refreshed by BiddyManagedGt */
  } else if (biddyManagerType == BIDDYTYPEOBDDC) {
    /* IMPLEMENTED */
    r = BiddyManagedGt(MNG,f,g);
    BiddyRefresh(r); /* not always refreshed by BiddyManagedGt */
  } else if (biddyManagerType == BIDDYTYPEZBDDC) {
    /* IMPLEMENTED */
    r = BiddyManagedGt(MNG,f,g);
    BiddyRefresh(r); /* not always refreshed by BiddyManagedGt */
  } else if (biddyManagerType == BIDDYTYPETZBDD) {
    /* IMPLEMENTED */
    r = BiddyManagedGt(MNG,f,g);
    BiddyRefresh(r); /* not always refreshed by BiddyManagedGt */
  } else if ((biddyManagerType == BIDDYTYPEZBDD) ||
              (biddyManagerType == BIDDYTYPETZBDDC))
  {
    fprintf(stderr,"Biddy_Gt: this GDD type is not supported, yet!\n");
    return biddyNull;
  } else if ((biddyManagerType == BIDDYTYPEOFDDC) || (biddyManagerType == BIDDYTYPEOFDD) ||
              (biddyManagerType == BIDDYTYPEZFDDC) || (biddyManagerType == BIDDYTYPEZFDD) ||
              (biddyManagerType == BIDDYTYPETZFDDC) || (biddyManagerType == BIDDYTYPETZFDD))
  {
    fprintf(stderr,"Biddy_Gt: this GDD type is not supported, yet!\n");
    return biddyNull;
  } else {
    fprintf(stderr,"Biddy_Gt: Unsupported GDD type!\n");
    return biddyNull;
  }

  return r;
}

#ifdef __cplusplus
}
#endif

Biddy_Edge
BiddyManagedGt(const Biddy_Manager MNG, const Biddy_Edge f, const Biddy_Edge g)
{
  Biddy_Edge r, T, E, Fv, Gv, Fneg_v, Gneg_v;
  Biddy_Edge FF, GG, HH;
  Biddy_Variable v,rtag;
  unsigned int cindex;

  static Biddy_Variable topF; /* CAN BE STATIC, WHAT IS BETTER? */
  static Biddy_Variable topG; /* CAN BE STATIC, WHAT IS BETTER? */
  static Biddy_Variable tagF; /* CAN BE STATIC, WHAT IS BETTER? */
  static Biddy_Variable tagG; /* CAN BE STATIC, WHAT IS BETTER? */

  assert( MNG != NULL );
  assert( f != NULL );
  assert( g != NULL );

  r = biddyNull;

  /* IMPLEMENTED FOR OBDD, OBDDC, ZBDDC, AND TZBDDC */
  assert(
    (biddyManagerType == BIDDYTYPEOBDD) ||
    (biddyManagerType == BIDDYTYPEOBDDC) ||
    (biddyManagerType == BIDDYTYPEZBDDC) ||
    (biddyManagerType == BIDDYTYPETZBDD)
  );

  if (biddyManagerType == BIDDYTYPEOBDDC) {

    /* FOR OBDDC, PROCEED BY CALCULATING NOT-AND */

    r = BiddyManagedAnd(MNG,f,Biddy_Inv(g));
    return r;

  }

  /* LOOKING FOR SIMPLE CASE */

  if (f == biddyZero) {
    return biddyZero;
  } else if (g == biddyZero) {
    return f;
  } else if (f == biddyOne) {
    return BiddyManagedNot(MNG,g);
  } else if (g == biddyOne) {
    return biddyZero;
  } else if (f == g) {
    return biddyZero;
  }

  if (biddyManagerType == BIDDYTYPETZBDD) {
    if (BiddyR(f) == BiddyR(g)) {
      if (BiddyIsSmaller(Biddy_GetTag(f),Biddy_GetTag(g))) {
        return biddyZero;
      }
    }
  }

  /* THIS IS NOT A SIMPLE CASE */

  FF = GG = HH = biddyNull;
  Fneg_v = Fv = biddyNull;
  Gneg_v = Gv = biddyNull;
  rtag = 0;
  v = 0;

  if (biddyManagerType == BIDDYTYPEOBDDC) {
    /* ALREADY CALCULATED */
  }

  else {

#ifdef BIDDYEXTENDEDSTATS_YES
    biddyNodeTable.andorrecursive++;
#endif

    /* FF, GG, and HH ARE USED FOR CACHE LOOKUP, ONLY */
    /* THIS IS NOT ITE CACHE! */

    FF = f;
    GG = g;
    HH = g;

  }

  assert ( f != biddyZero );
  assert ( f != biddyOne );
  assert ( g != biddyZero );
  assert ( g != biddyOne );

  /* IF RESULT IS NOT IN THE CACHE TABLE... */
  cindex = 0;
  if (((biddyManagerType == BIDDYTYPEZBDDC) && ((topF = BiddyV(f)) != v))
      || !findOp3Cache(MNG,biddyOPCache,FF,GG,HH,&r,&cindex))
  {

    if (biddyManagerType == BIDDYTYPEOBDDC) {
      /* ALREADY CALCULATED */
    }

    else if (biddyManagerType == BIDDYTYPEOBDD) {

      /* LOOKING FOR THE SMALLEST TOP VARIABLE */
      /* CONSTANT NODE MUST HAVE MAX ORDER */
      topF = BiddyV(f);
      topG = BiddyV(g);
      v = BiddyIsSmaller(topF,topG) ? topF : topG;

      /* DETERMINING PARAMETERS FOR RECURSIVE CALLS */
      /* COMPLEMENTED EDGES MUST BE TRANSFERED */
      if (topF == v) {
        Fneg_v = BiddyE(f);
        Fv = BiddyT(f);
      } else {
        Fneg_v = Fv = f;
      }

      if (topG == v) {
        Gneg_v = BiddyE(g);
        Gv = BiddyT(g);
      } else {
        Gneg_v = Gv = g;
      }

      rtag = v;

    }

    else if (biddyManagerType == BIDDYTYPEZBDDC) {

      /* LOOKING FOR THE SMALLEST TOP VARIABLE */
      /* CONSTANT NODE MUST HAVE MAX ORDER */
      /* FOR ZBDD, topF HAS BEEN ALREADY CALCULATED */
      /* topF = BiddyV(f); */ /* ALREADY CALCULATED */
      topG = BiddyV(g);
      v = BiddyIsSmaller(topF,topG) ? topF : topG;

      /* THIS CALL IS NOT INTRODUCING NEW NODES */
      /* COMPLEMENTED EDGES MUST BE TRANSFERED */
      /* IT SEEMS THAT CACHING THIS RESULTS IS NOT A GOOD IDEA */
      if (topF != v) {
        r = BiddyManagedGt(MNG,f,Biddy_InvCond(BiddyE(g),Biddy_GetMark(g)));
        return r;
      }

      /* DETERMINING PARAMETERS FOR RECURSIVE CALLS */
      /* IT IS ALREADY KNOWN THAT topF == v */
      /* COMPLEMENTED EDGES MUST BE TRANSFERED */
      Fneg_v = Biddy_InvCond(BiddyE(f),Biddy_GetMark(f));
      Fv = BiddyT(f);
      if (topG == v) {
        Gneg_v = Biddy_InvCond(BiddyE(g),Biddy_GetMark(g));
        Gv = BiddyT(g);
      } else {
         Gneg_v = g;
         Gv = biddyZero;
      }

      rtag = 0;

    }

    else if (biddyManagerType == BIDDYTYPETZBDD) {

      /* LOOKING FOR THE SMALLEST TAG AND SMALLEST TOP VARIABLE */
      /* CONSTANT NODE MUST HAVE MAX ORDER */
      tagF = Biddy_GetTag(f);
      tagG = Biddy_GetTag(g);
      topF = BiddyV(f);
      topG = BiddyV(g);
      rtag = BiddyIsSmaller(tagF,tagG) ? tagF : tagG;
      v = BiddyIsSmaller(topF,topG) ? topF : topG;

      if (BiddyIsSmaller(tagG,tagF)) v = rtag;

      if (BiddyIsSmaller(v,tagF)) {
        Fneg_v = Fv = f;
      } else if (BiddyIsSmaller(v,topF)) {
        Fneg_v = f;
        Biddy_SetTag(Fneg_v,v);
        Fneg_v = Biddy_Managed_IncTag(MNG,Fneg_v);
        Fv = biddyZero;
      } else {
        /* HERE topF == v */
        Fneg_v = BiddyE(f);
        Fv = BiddyT(f);
      }
      if (BiddyIsSmaller(v,tagG)) {
        Gneg_v = Gv = g;
      } else if (BiddyIsSmaller(v,topG)) {
        Gneg_v = g;
        Biddy_SetTag(Gneg_v,v);
        Gneg_v = Biddy_Managed_IncTag(MNG,Gneg_v);
        Gv = biddyZero;
      } else {
        /* HERE topG == v */
        Gneg_v = BiddyE(g);
        Gv = BiddyT(g);
      }
      
    }

    /* RECURSIVE CALLS */
    if (Fneg_v == biddyZero) {
      E = biddyZero;
    } else if (Gneg_v == biddyZero) {
      E = Fneg_v;
    } else if (Fneg_v == biddyOne) {
      E = BiddyManagedNot(MNG,Gneg_v);
    } else if (Gneg_v == biddyOne) {
      E = biddyZero;
    } else if (Fneg_v == Gneg_v) {
      E = biddyZero;
    } else  {
      E = BiddyManagedGt(MNG,Fneg_v,Gneg_v);
    }
    if (Fv == biddyZero) {
      T = biddyZero;
    } else if (Gv == biddyZero) {
      T = Fv;
    } else if (Fv == biddyOne) {
      T = BiddyManagedNot(MNG,Gv);
    } else if (Gv == biddyOne) {
      T = biddyZero;
    } else if (Fv == Gv) {
      T = biddyZero;
    } else  {
      T = BiddyManagedGt(MNG,Fv,Gv);
    }

    r = BiddyManagedTaggedFoaNode(MNG,v,E,T,rtag,TRUE);
    BiddyRefresh(r); /* FoaNode returns an obsolete node! */

    addOp3Cache(MNG,biddyOPCache,FF,GG,HH,r,cindex);

  } else {

    /* IF THE RESULT IS FROM CACHE TABLE, REFRESH IT! */
    BiddyRefresh(r);

  }

  return r;
}

/***************************************************************************//*!
\brief Function Biddy_Managed_IsLeq returns TRUE iff function f is included in
       function g.

### Description
### Side Effects
    Prototyped for OBDDs, ZBDDs, and TZBDDs (via calculating
    full implication, this is less efficient as implementation in CUDD).
### More Info
    Macro Biddy_IsLeq(f,g) is defined for use with anonymous manager.
*******************************************************************************/

#ifdef __cplusplus
extern "C" {
#endif

Biddy_Boolean
Biddy_Managed_IsLeq(Biddy_Manager MNG, Biddy_Edge f, Biddy_Edge g)
{
  Biddy_Edge imp;
  Biddy_Boolean r;

  if (!MNG) MNG = biddyAnonymousManager;
  ZF_LOGI("Biddy_IsLeq");

  r = FALSE;

  if (biddyManagerType == BIDDYTYPEOBDD) {
    /* PROTOTYPED */
    imp = BiddyManagedLeq(MNG,f,g);
    r = (imp == biddyOne);
  } else if (biddyManagerType == BIDDYTYPEOBDDC) {
    /* PROTOTYPED */
    imp = BiddyManagedLeq(MNG,f,g);
    r = (imp == biddyOne);
  } else if (biddyManagerType == BIDDYTYPEZBDDC) {
    /* PROTOTYPED */
    imp = BiddyManagedLeq(MNG,f,g);
    r = (imp == biddyOne);
  } else if (biddyManagerType == BIDDYTYPETZBDD) {
    /* PROTOTYPED */
    imp = BiddyManagedLeq(MNG,f,g);
    r = (imp == biddyOne);
  } else if ((biddyManagerType == BIDDYTYPEZBDD) ||
              (biddyManagerType == BIDDYTYPETZBDDC))
  {
    fprintf(stderr,"Biddy_IsLeq: this GDD type is not supported, yet!\n");
    return FALSE;
  } else if ((biddyManagerType == BIDDYTYPEOFDDC) || (biddyManagerType == BIDDYTYPEOFDD) ||
              (biddyManagerType == BIDDYTYPEZFDDC) || (biddyManagerType == BIDDYTYPEZFDD) ||
              (biddyManagerType == BIDDYTYPETZFDDC) || (biddyManagerType == BIDDYTYPETZFDD))
  {
    fprintf(stderr,"Biddy_IsLeq: this GDD type is not supported, yet!\n");
    return FALSE;
  } else {
    fprintf(stderr,"Biddy_IsLeq: Unsupported GDD type!\n");
    return FALSE;
  }

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
    For OBDDs, recursive calls use optimization: F(a=x) == NOT((NOT F)(a=x)).
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

  r = biddyNull;

  if (biddyManagerType == BIDDYTYPEOBDD) {
    /* IMPLEMENTED */
    r = BiddyManagedRestrict(MNG,f,v,value);
    BiddyRefresh(r); /* not always refreshed by BiddyManagedRestrict */
  } else if (biddyManagerType == BIDDYTYPEOBDDC) {
    /* IMPLEMENTED */
    r = BiddyManagedRestrict(MNG,f,v,value);
    BiddyRefresh(r); /* not always refreshed by BiddyManagedRestrict */
  } else if (biddyManagerType == BIDDYTYPEZBDDC) {
    /* IMPLEMENTED */
    r = BiddyManagedRestrict(MNG,f,v,value);
    BiddyRefresh(r); /* not always refreshed by BiddyManagedRestrict */
  } else if (biddyManagerType == BIDDYTYPETZBDD) {
    /* IMPLEMENTED */
    r = BiddyManagedRestrict(MNG,f,v,value);
    BiddyRefresh(r); /* not always refreshed by BiddyManagedRestrict */
  } else if ((biddyManagerType == BIDDYTYPEZBDD) ||
              (biddyManagerType == BIDDYTYPETZBDDC))
  {
    fprintf(stderr,"Biddy_Restrict: this GDD type is not supported, yet!\n");
    return biddyNull;
  } else if ((biddyManagerType == BIDDYTYPEOFDDC) || (biddyManagerType == BIDDYTYPEOFDD) ||
              (biddyManagerType == BIDDYTYPEZFDDC) || (biddyManagerType == BIDDYTYPEZFDD) ||
              (biddyManagerType == BIDDYTYPETZFDDC) || (biddyManagerType == BIDDYTYPETZFDD))
  {
    fprintf(stderr,"Biddy_Restrict: this GDD type is not supported, yet!\n");
    return biddyNull;
  } else {
    fprintf(stderr,"Biddy_Restrict: Unsupported GDD type!\n");
    return biddyNull;
  }

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
  Biddy_Variable fv,tag;
  Biddy_Edge FF, GG, HH;
  unsigned int cindex;

  assert( MNG != NULL );
  assert( f != NULL );

  /* IMPLEMENTED FOR OBDD, OBDDC, ZBDDC, AND TZBDD */
  assert(
    (biddyManagerType == BIDDYTYPEOBDD) ||
    (biddyManagerType == BIDDYTYPEOBDDC) ||
    (biddyManagerType == BIDDYTYPEZBDDC) ||
    (biddyManagerType == BIDDYTYPETZBDD)
  );

  r = biddyNull;

  if (f == biddyZero) return biddyZero;

  FF = f;
  GG = value?biddyZero:biddyOne;
  HH = biddyVariableTable.table[v].variable;

  /* IF RESULT IS NOT IN THE CACHE TABLE... */
  /* Result is stored in cache only if recursive calls are needed */
  /* I am not sure if this is a good strategy! */
  cindex = 0;
  if (!findOp3Cache(MNG,biddyRCCache,FF,GG,HH,&r,&cindex))
  {

    if ((biddyManagerType == BIDDYTYPEOBDDC) || (biddyManagerType == BIDDYTYPEOBDD)) {
      if ((fv=BiddyV(f)) == v) {
        if (value) {
          r = Biddy_InvCond(BiddyT(f),Biddy_GetMark(f));
        } else {
          r = Biddy_InvCond(BiddyE(f),Biddy_GetMark(f));
        }
      }
      else if (BiddyIsSmaller(v,fv)) {
        r = f;
      }
      else {
        e = BiddyManagedRestrict(MNG,BiddyE(f),v,value);
        t = BiddyManagedRestrict(MNG,BiddyT(f),v,value);
        r = Biddy_InvCond(BiddyManagedTaggedFoaNode(MNG,fv,e,t,fv,TRUE),Biddy_GetMark(f));
        BiddyRefresh(r); /* FoaNode returns an obsolete node! */
        addOp3Cache(MNG,biddyRCCache,FF,GG,HH,r,cindex);
      }
    }

    else if (biddyManagerType == BIDDYTYPEZBDDC) {
      if ((fv=BiddyV(f)) == v) {
        if (value) {
          t = BiddyT(f);
          r = BiddyManagedTaggedFoaNode(MNG,v,t,t,0,TRUE);
          BiddyRefresh(r); /* FoaNode returns an obsolete node! */
        } else {
          e = Biddy_InvCond(BiddyE(f),Biddy_GetMark(f));
          r = BiddyManagedTaggedFoaNode(MNG,v,e,e,0,TRUE);
          BiddyRefresh(r); /* FoaNode returns an obsolete node! */
        }
      }
      else if (BiddyIsSmaller(v,fv)) {
        if (value) {
          r = biddyZero;
        } else {
          r = BiddyManagedTaggedFoaNode(MNG,v,f,f,0,TRUE);
          BiddyRefresh(r); /* FoaNode returns an obsolete node! */
        }
      }
      else {
        e = BiddyManagedRestrict(MNG,Biddy_InvCond(BiddyE(f),Biddy_GetMark(f)),v,value);
        t = BiddyManagedRestrict(MNG,BiddyT(f),v,value);
        r = BiddyManagedTaggedFoaNode(MNG,fv,e,t,0,TRUE);
        BiddyRefresh(r); /* FoaNode returns an obsolete node! */
        addOp3Cache(MNG,biddyRCCache,FF,GG,HH,r,cindex);
      }
    }

    else if (biddyManagerType == BIDDYTYPETZBDD) {
      tag = Biddy_GetTag(f);
      if (BiddyIsSmaller(v,tag)) {
        r = f;
      } else {
        fv = BiddyV(f);
        if (v == fv) {
          if (v == tag) {
            if (value) {
              r = BiddyT(f);
            } else {
              r = BiddyE(f);
            }
          } else {
            if (value) {
              t = BiddyT(f);
              r = BiddyManagedTaggedFoaNode(MNG,v,t,t,tag,TRUE);
              BiddyRefresh(r); /* FoaNode returns an obsolete node! */
            } else {
              e = BiddyE(f);
              r = BiddyManagedTaggedFoaNode(MNG,v,e,e,tag,TRUE);
              BiddyRefresh(r); /* FoaNode returns an obsolete node! */
            }
          }
        } else if (BiddyIsSmaller(v,fv)) {
          if (value) {
            r = biddyZero;
          } else {
            if (v == tag) {
              r = Biddy_Managed_IncTag(MNG,f);
            } else {
              r = f;
              Biddy_SetTag(r,v);
              r = Biddy_Managed_IncTag(MNG,r);
              r = BiddyManagedTaggedFoaNode(MNG,v,r,r,tag,TRUE);
              BiddyRefresh(r); /* FoaNode returns an obsolete node! */
            }
          }
        } else {
          e = BiddyManagedRestrict(MNG,BiddyE(f),v,value);
          t = BiddyManagedRestrict(MNG,BiddyT(f),v,value);
          r = BiddyManagedTaggedFoaNode(MNG,fv,e,t,tag,TRUE);
          BiddyRefresh(r); /* FoaNode returns an obsolete node! */
          addOp3Cache(MNG,biddyRCCache,FF,GG,HH,r,cindex);
        }
      }
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
    Original BDD is not changed.
### Side effects
    For OBDDs, recursive calls use optimization: F(a=G) == NOT((NOT F)(a=G)).
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

  r = biddyNull;

  if (biddyManagerType == BIDDYTYPEOBDD) {
    /* IMPLEMENTED */
    r = BiddyManagedCompose(MNG,f,g,v);
    BiddyRefresh(r); /* not always refreshed by BiddyManagedCompose */
  } else if (biddyManagerType == BIDDYTYPEOBDDC) {
    /* IMPLEMENTED */
    r = BiddyManagedCompose(MNG,f,g,v);
    BiddyRefresh(r); /* not always refreshed by BiddyManagedCompose */
  } else if (biddyManagerType == BIDDYTYPEZBDDC) {
    /* IMPLEMENTED */
    r = BiddyManagedCompose(MNG,f,g,v);
    BiddyRefresh(r); /* not always refreshed by BiddyManagedCompose */
  } else if (biddyManagerType == BIDDYTYPETZBDD) {
    /* IMPLEMENTED */
    r = BiddyManagedCompose(MNG,f,g,v);
    BiddyRefresh(r); /* not always refreshed by BiddyManagedCompose */
  } else if ((biddyManagerType == BIDDYTYPEZBDD) ||
              (biddyManagerType == BIDDYTYPETZBDDC))
  {
    fprintf(stderr,"Biddy_Compose: this GDD type is not supported, yet!\n");
    return biddyNull;
  } else if ((biddyManagerType == BIDDYTYPEOFDDC) || (biddyManagerType == BIDDYTYPEOFDD) ||
              (biddyManagerType == BIDDYTYPEZFDDC) || (biddyManagerType == BIDDYTYPEZFDD) ||
              (biddyManagerType == BIDDYTYPETZFDDC) || (biddyManagerType == BIDDYTYPETZFDD))
  {
    fprintf(stderr,"Biddy_Compose: this GDD type is not supported, yet!\n");
    return biddyNull;
  } else {
    fprintf(stderr,"Biddy_Compose: Unsupported GDD type!\n");
    return biddyNull;
  }

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
  Biddy_Variable fv,tag;
  Biddy_Edge FF, GG, HH;
  unsigned int cindex;

  assert( MNG != NULL );
  assert( f != NULL );
  assert( g != NULL );

  /* IMPLEMENTED FOR OBDD, OBDDC, ZBDDC, AND TZBDD */
  assert(
    (biddyManagerType == BIDDYTYPEOBDD) ||
    (biddyManagerType == BIDDYTYPEOBDDC) ||
    (biddyManagerType == BIDDYTYPEZBDDC) ||
    (biddyManagerType == BIDDYTYPETZBDD)
  );

  r = biddyNull;

  if (f == biddyZero) return f;

  FF = f;
  GG = g;
  HH = biddyVariableTable.table[v].variable;

  /* IF RESULT IS NOT IN THE CACHE TABLE... */
  /* TO DO: CHECK ONLY IF IT IS POSSIBLE TO EXIST IN THE CACHE */
  cindex = 0;
  if (!findOp3Cache(MNG,biddyRCCache,FF,GG,HH,&r,&cindex))
  {

    if ((biddyManagerType == BIDDYTYPEOBDDC) || (biddyManagerType == BIDDYTYPEOBDD)) {
      if ((fv=BiddyV(f)) == v) {
        r = Biddy_InvCond(BiddyManagedITE(MNG,g,BiddyT(f),BiddyE(f)),Biddy_GetMark(f));
      }
      else if (BiddyIsSmaller(v,fv)) {
        r = f;
      }
      else {
        e = BiddyManagedCompose(MNG,BiddyE(f),g,v);
        t = BiddyManagedCompose(MNG,BiddyT(f),g,v);
        /* VARIANT A: USE TRUE FOR THE FIRST CONDITION */
        /* VARIANT B: USE FALSE FOR THE FIRST CONDITION */
        if (TRUE && BiddyIsSmaller(fv,Biddy_GetTopVariable(e)) && BiddyIsSmaller(fv,Biddy_GetTopVariable(t))) {
          r = Biddy_InvCond(BiddyManagedTaggedFoaNode(MNG,fv,e,t,fv,TRUE),Biddy_GetMark(f));
          BiddyRefresh(r); /* FoaNode returns an obsolete node! */
        } else {
          r = Biddy_InvCond(BiddyManagedITE(MNG,biddyVariableTable.table[fv].variable,t,e),Biddy_GetMark(f));
        }
        addOp3Cache(MNG,biddyRCCache,FF,GG,HH,r,cindex);
      }
    }

    else if (biddyManagerType == BIDDYTYPEZBDDC) {
      if ((fv=BiddyV(f)) == v) {
        e = Biddy_InvCond(BiddyE(f),Biddy_GetMark(f));
        e = BiddyManagedTaggedFoaNode(MNG,v,e,e,0,TRUE);
        BiddyRefresh(e); /* FoaNode returns an obsolete node! */
        t = BiddyT(f);
        t = BiddyManagedTaggedFoaNode(MNG,v,t,t,0,TRUE);
        BiddyRefresh(t); /* FoaNode returns an obsolete node! */
        r = BiddyManagedITE(MNG,g,t,e);
        addOp3Cache(MNG,biddyRCCache,FF,GG,HH,r,cindex);
      }
      else if (BiddyIsSmaller(v,fv)) {
        r = BiddyManagedTaggedFoaNode(MNG,v,f,f,0,TRUE);
        BiddyRefresh(r); /* FoaNode returns an obsolete node! */
        r = BiddyManagedITE(MNG,g,biddyZero,r);
        addOp3Cache(MNG,biddyRCCache,FF,GG,HH,r,cindex);
      }
      else {
        e = BiddyManagedCompose(MNG,Biddy_InvCond(BiddyE(f),Biddy_GetMark(f)),g,v);
        t = BiddyManagedCompose(MNG,BiddyT(f),BiddyManagedChange(MNG,g,fv),v);
        t = BiddyManagedChange(MNG,t,fv);
        r = BiddyManagedXor(MNG,e,t);
        addOp3Cache(MNG,biddyRCCache,FF,GG,HH,r,cindex);
      }
    }

    else if (biddyManagerType == BIDDYTYPETZBDD) {
      tag = Biddy_GetTag(f);
      if (BiddyIsSmaller(v,tag)) {
        r = f;
      } else {
        if ((fv=BiddyV(f)) == v) {
          /* VARIANT 1 */
          /**/
          if (v == tag) {
            e = BiddyE(f);
            t = BiddyT(f);
          } else {
            e = BiddyManagedTaggedFoaNode(MNG,v,BiddyE(f),BiddyE(f),tag,TRUE);
            BiddyRefresh(e); /* FoaNode returns an obsolete node! */
            t = BiddyManagedTaggedFoaNode(MNG,v,BiddyT(f),BiddyT(f),tag,TRUE);
            BiddyRefresh(t); /* FoaNode returns an obsolete node! */
          }
          r = BiddyManagedITE(MNG,g,t,e);
          /**/
          /* VARIANT 2 - THIS COULD BE BETTER FOR SOME EXAMPLES */
          /*
          r = BiddyManagedTaggedFoaNode(MNG,v,biddyOne,biddyOne,tag,TRUE);
          BiddyRefresh(r);
          e = BiddyE(f);
          t = BiddyT(f);
          r = BiddyManagedAnd(MNG,r,BiddyManagedITE(MNG,g,t,e));
          */
          addOp3Cache(MNG,biddyRCCache,FF,GG,HH,r,cindex);
        }
        else if (BiddyIsSmaller(v,fv)) {
          r = BiddyManagedRestrict(MNG,f,v,FALSE);
          r = BiddyManagedITE(MNG,g,biddyZero,r);
          addOp3Cache(MNG,biddyRCCache,FF,GG,HH,r,cindex);
        } else {
          /* VARIANT 1 */
          /**/
          r = BiddyManagedTaggedFoaNode(MNG,fv,biddyOne,biddyOne,tag,TRUE);
          BiddyRefresh(r); /* FoaNode returns an obsolete node! */
          e = BiddyManagedCompose(MNG,BiddyE(f),g,v);
          t = BiddyManagedCompose(MNG,BiddyT(f),g,v);
          r = BiddyManagedAnd(MNG,r,BiddyManagedITE(MNG,biddyVariableTable.table[fv].variable,t,e));
          /**/
          /* VARIANT 2 - THIS SEEMS TO BE SLOWER */
          /*
          e = BiddyManagedTaggedFoaNode(MNG,fv,biddyOne,biddyZero,tag,TRUE);
          BiddyRefresh(e);
          t = BiddyManagedTaggedFoaNode(MNG,fv,biddyZero,biddyOne,tag,TRUE);
          BiddyRefresh(t);
          e = BiddyManagedAnd(MNG,e,BiddyManagedCompose(MNG,BiddyE(f),g,v));
          t = BiddyManagedAnd(MNG,t,BiddyManagedCompose(MNG,BiddyT(f),g,v));
          r = BiddyManagedXor(MNG,e,t);
          */
          addOp3Cache(MNG,biddyRCCache,FF,GG,HH,r,cindex);
        }
      }
    }

  } else {

    /* IF THE RESULT IS FROM CACHE TABLE, FRESH IT! */
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

  r = biddyNull;

  if (biddyManagerType == BIDDYTYPEOBDD) {
    /* IMPLEMENTED */
    r = BiddyManagedE(MNG,f,v);
    BiddyRefresh(r); /* not always refreshed by BiddyManagedE */
  } else if (biddyManagerType == BIDDYTYPEOBDDC) {
    /* IMPLEMENTED */
    r = BiddyManagedE(MNG,f,v);
    BiddyRefresh(r); /* not always refreshed by BiddyManagedE */
  } else if (biddyManagerType == BIDDYTYPEZBDDC) {
    /* IMPLEMENTED */
    r = BiddyManagedE(MNG,f,v);
    BiddyRefresh(r); /* not always refreshed by BiddyManagedE */
  } else if (biddyManagerType == BIDDYTYPETZBDD) {
    /* IMPLEMENTED */
    r = BiddyManagedE(MNG,f,v);
    BiddyRefresh(r); /* not always refreshed by BiddyManagedE */
  } else if ((biddyManagerType == BIDDYTYPEZBDD) ||
              (biddyManagerType == BIDDYTYPETZBDDC))
  {
    fprintf(stderr,"Biddy_E: this GDD type is not supported, yet!\n");
    return biddyNull;
  } else if ((biddyManagerType == BIDDYTYPEOFDDC) || (biddyManagerType == BIDDYTYPEOFDD) ||
              (biddyManagerType == BIDDYTYPEZFDDC) || (biddyManagerType == BIDDYTYPEZFDD) ||
              (biddyManagerType == BIDDYTYPETZFDDC) || (biddyManagerType == BIDDYTYPETZFDD))
  {
    fprintf(stderr,"Biddy_E: this GDD type is not supported, yet!\n");
    return biddyNull;
  } else {
    fprintf(stderr,"Biddy_E: Unsupported GDD type!\n");
    return biddyNull;
  }

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
  Biddy_Variable fv,tag;
  unsigned int cindex;

  assert( MNG != NULL );
  assert( f != NULL );

  /* IMPLEMENTED FOR OBDD, OBDDC, ZBDDC, AND TZBDD */
  assert(
    (biddyManagerType == BIDDYTYPEOBDD) ||
    (biddyManagerType == BIDDYTYPEOBDDC) ||
    (biddyManagerType == BIDDYTYPEZBDDC) ||
    (biddyManagerType == BIDDYTYPETZBDD)
  );

  r = biddyNull;

  /* LOOKING FOR SIMPLE CASE */
  if (f == biddyZero) return f;

  h = biddyNull;

  /* h MUST BE COMPATIBLE WITH CUBE */
  if ((biddyManagerType == BIDDYTYPEOBDDC) || (biddyManagerType == BIDDYTYPEOBDD)) {
    h = biddyVariableTable.table[v].variable;
  }
  else if (biddyManagerType == BIDDYTYPEZBDDC) {
    h = biddyVariableTable.table[v].element;
  }
  else if (biddyManagerType == BIDDYTYPETZBDD) {
    h = biddyVariableTable.table[v].variable;
  }

  /* IF RESULT IS NOT IN THE CACHE TABLE... */
  cindex = 0;
  if ((((uintptr_t) f) > ((uintptr_t) biddyOne)) ?
      !findOp3Cache(MNG,biddyEACache,biddyOne,f,h,&r,&cindex) :
      !findOp3Cache(MNG,biddyEACache,f,biddyOne,h,&r,&cindex))
  {

    if ((biddyManagerType == BIDDYTYPEOBDDC) || (biddyManagerType == BIDDYTYPEOBDD)) {
      if ((fv=BiddyV(f)) == v) {
        r = BiddyManagedOr(MNG,
                           Biddy_InvCond(BiddyE(f),Biddy_GetMark(f)),
                           Biddy_InvCond(BiddyT(f),Biddy_GetMark(f)));
      }
      else if (BiddyIsSmaller(v,fv)) {
        r = f;
      }
      else {
        e = BiddyManagedE(MNG,Biddy_InvCond(BiddyE(f),Biddy_GetMark(f)),v);
        t = BiddyManagedE(MNG,Biddy_InvCond(BiddyT(f),Biddy_GetMark(f)),v);
        r = BiddyManagedTaggedFoaNode(MNG,fv,e,t,fv,TRUE);
        BiddyRefresh(r); /* FoaNode returns an obsolete node! */
      }
    }

    else if (biddyManagerType == BIDDYTYPEZBDDC) {
      if ((fv=BiddyV(f)) == v) {
        r = BiddyManagedOr(MNG,
                           Biddy_InvCond(BiddyE(f),Biddy_GetMark(f)),
                           BiddyT(f));
        r = BiddyManagedTaggedFoaNode(MNG,v,r,r,0,TRUE);
        BiddyRefresh(r); /* FoaNode returns an obsolete node! */
      }
      else if (BiddyIsSmaller(v,fv)) {
        r = BiddyManagedTaggedFoaNode(MNG,v,f,f,0,TRUE);
        BiddyRefresh(r); /* FoaNode returns an obsolete node! */
      }
      else {
        e = BiddyManagedE(MNG,Biddy_InvCond(BiddyE(f),Biddy_GetMark(f)),v);
        t = BiddyManagedE(MNG,BiddyT(f),v);
        r = BiddyManagedTaggedFoaNode(MNG,fv,e,t,0,TRUE);
        BiddyRefresh(r); /* FoaNode returns an obsolete node! */
      }
    }

    else if (biddyManagerType == BIDDYTYPETZBDD) {
      tag = Biddy_GetTag(f);
      if (BiddyIsSmaller(v,tag)) {
        r = f;
      } else {
        fv = BiddyV(f);
        if (v == fv) {
          r = BiddyManagedOr(MNG,BiddyE(f),BiddyT(f));
          if (v != tag) {
            r = BiddyManagedTaggedFoaNode(MNG,v,r,r,tag,TRUE);
            BiddyRefresh(r); /* FoaNode returns an obsolete node! */
          }
        } else if (BiddyIsSmaller(v,fv)) {
          if (v == tag) {
            r = Biddy_Managed_IncTag(MNG,f);
          } else {
            r = f;
            Biddy_SetTag(r,v);
            r = Biddy_Managed_IncTag(MNG,r);
            r = BiddyManagedTaggedFoaNode(MNG,v,r,r,tag,TRUE);
            BiddyRefresh(r); /* FoaNode returns an obsolete node! */
          }
        } else {
          e = BiddyManagedE(MNG,BiddyE(f),v);
          t = BiddyManagedE(MNG,BiddyT(f),v);
          r = BiddyManagedTaggedFoaNode(MNG,fv,e,t,tag,TRUE);
          BiddyRefresh(r); /* FoaNode returns an obsolete node! */
        }
      }
    }

    /* CACHE FOR Ex(v)(f*1), f*1 <=> 1*f */
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
    Implemented for OBDDC. Prototyped for OBDD.
    Prototyped for ZBDDs and TZBDDs.
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

  r = biddyNull;

  if (biddyManagerType == BIDDYTYPEOBDD) {
    /* PROTOTYPED */
    r = BiddyManagedNot(MNG,BiddyManagedE(MNG,BiddyManagedNot(MNG,f),v));
    BiddyRefresh(r); /* not always refreshed by BiddyManagedNot */
  } else if (biddyManagerType == BIDDYTYPEOBDDC) {
    /* IMPLEMENTED */
    r = BiddyManagedA(MNG,f,v);
    BiddyRefresh(r); /* not always refreshed by BiddyManagedA */
  } else if (biddyManagerType == BIDDYTYPEZBDDC) {
    /* PROTOTYPED */
    r = BiddyManagedNot(MNG,BiddyManagedE(MNG,BiddyManagedNot(MNG,f),v));
    BiddyRefresh(r); /* not always refreshed by BiddyManagedNot */
  } else if (biddyManagerType == BIDDYTYPETZBDD) {
    /* PROTOTYPED */
    r = BiddyManagedNot(MNG,BiddyManagedE(MNG,BiddyManagedNot(MNG,f),v));
    BiddyRefresh(r); /* not always refreshed by BiddyManagedNot */
  } else if ((biddyManagerType == BIDDYTYPEZBDD) ||
              (biddyManagerType == BIDDYTYPETZBDDC))
  {
    fprintf(stderr,"Biddy_A: this GDD type is not supported, yet!\n");
    return biddyNull;
  } else if ((biddyManagerType == BIDDYTYPEOFDDC) || (biddyManagerType == BIDDYTYPEOFDD) ||
              (biddyManagerType == BIDDYTYPEZFDDC) || (biddyManagerType == BIDDYTYPEZFDD) ||
              (biddyManagerType == BIDDYTYPETZFDDC) || (biddyManagerType == BIDDYTYPETZFDD))
  {
    fprintf(stderr,"Biddy_A: this GDD type is not supported, yet!\n");
    return biddyNull;
  } else {
    fprintf(stderr,"Biddy_A: Unsupported GDD type!\n");
    return biddyNull;
  }

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

  /* IMPLEMENTED FOR OBDDC, ONLY */
  assert( biddyManagerType == BIDDYTYPEOBDDC );

  /* LOOKING FOR SIMPLE CASE */
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
    Prototyped for OBDDs (via xA, calculating full universal quantification
    is less efficient as direct implementation in CUDD).
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
  Biddy_Edge xa;
  Biddy_Boolean r;

  if (!MNG) MNG = biddyAnonymousManager;
  ZF_LOGI("Biddy_IsVariableDependent");

  r = FALSE;

  if (biddyManagerType == BIDDYTYPEOBDD) {
    /* PROTOTYPED */
    xa = BiddyManagedA(MNG,f,v);
    r = (xa == biddyZero);
  } else if (biddyManagerType == BIDDYTYPEOBDDC) {
    /* PROTOTYPED */
    xa = BiddyManagedA(MNG,f,v);
    r = (xa == biddyZero);
  } else if ((biddyManagerType == BIDDYTYPEZBDDC) ||
              (biddyManagerType == BIDDYTYPETZBDD))
  {
    fprintf(stderr,"Biddy_IsVariableDependent: this GDD type is not supported, yet!\n");
    return FALSE;
  } else if ((biddyManagerType == BIDDYTYPEZBDD) ||
              (biddyManagerType == BIDDYTYPETZBDDC))
  {
    fprintf(stderr,"Biddy_IsVariableDependent: this GDD type is not supported, yet!\n");
    return FALSE;
  } else if ((biddyManagerType == BIDDYTYPEOFDDC) || (biddyManagerType == BIDDYTYPEOFDD) ||
              (biddyManagerType == BIDDYTYPEZFDDC) || (biddyManagerType == BIDDYTYPEZFDD) ||
              (biddyManagerType == BIDDYTYPETZFDDC) || (biddyManagerType == BIDDYTYPETZFDD))
  {
    fprintf(stderr,"Biddy_IsVariableDependent: this GDD type is not supported, yet!\n");
    return FALSE;
  } else {
    fprintf(stderr,"Biddy_IsVariableDependent: Unsupported GDD type!\n");
    return FALSE;
  }

  return r;
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

  r = biddyNull;

  if (biddyManagerType == BIDDYTYPEOBDD) {
    /* IMPLEMENTED */
    r = BiddyManagedExistAbstract(MNG,f,cube);
    BiddyRefresh(r); /* not always refreshed by BiddyManagedExistAbstract */
  } else if (biddyManagerType == BIDDYTYPEOBDDC) {
    /* IMPLEMENTED */
    r = BiddyManagedExistAbstract(MNG,f,cube);
    BiddyRefresh(r); /* not always refreshed by BiddyManagedExistAbstract */
  } else if (biddyManagerType == BIDDYTYPEZBDDC) {
    /* IMPLEMENTED */
    r = BiddyManagedExistAbstract(MNG,f,cube);
    BiddyRefresh(r); /* not always refreshed by BiddyManagedExistAbstract */
  } else if (biddyManagerType == BIDDYTYPETZBDD) {
    /* IMPLEMENTED */
    r = BiddyManagedExistAbstract(MNG,f,cube);
    BiddyRefresh(r); /* not always refreshed by BiddyManagedExistAbstract */
  } else if ((biddyManagerType == BIDDYTYPEZBDD) ||
              (biddyManagerType == BIDDYTYPETZBDDC))
  {
    fprintf(stderr,"Biddy_ExistAbstract: this GDD type is not supported, yet!\n");
    return biddyNull;
  } else if ((biddyManagerType == BIDDYTYPEOFDDC) || (biddyManagerType == BIDDYTYPEOFDD) ||
              (biddyManagerType == BIDDYTYPEZFDDC) || (biddyManagerType == BIDDYTYPEZFDD) ||
              (biddyManagerType == BIDDYTYPETZFDDC) || (biddyManagerType == BIDDYTYPETZFDD))
  {
    fprintf(stderr,"Biddy_ExistAbstract: this GDD type is not supported, yet!\n");
    return biddyNull;
  } else {
    fprintf(stderr,"Biddy_ExistAbstract: Unsupported GDD type!\n");
    return biddyNull;
  }

  return r;
}

#ifdef __cplusplus
}
#endif

Biddy_Edge
BiddyManagedExistAbstract(Biddy_Manager MNG, Biddy_Edge f, Biddy_Edge cube)
{
  Biddy_Edge e, t, r;
  Biddy_Variable fv,cv,tag;
  unsigned int cindex;

  assert( MNG != NULL );
  assert( f != NULL );
  assert( cube != NULL );

  /* IMPLEMENTED FOR OBDD, OBDDC, ZBDDC, AND TZBDD */
  assert(
    (biddyManagerType == BIDDYTYPEOBDD) ||
    (biddyManagerType == BIDDYTYPEOBDDC) ||
    (biddyManagerType == BIDDYTYPEZBDDC) ||
    (biddyManagerType == BIDDYTYPETZBDD)
  );

  r = biddyNull;

  /* LOOKING FOR SIMPLE CASE */
  if (f == biddyZero) return f;

  /* IF RESULT IS NOT IN THE CACHE TABLE... */
  cindex = 0;
  if ((((uintptr_t) f) > ((uintptr_t) biddyOne)) ?
      !findOp3Cache(MNG,biddyEACache,biddyOne,f,cube,&r,&cindex) :
      !findOp3Cache(MNG,biddyEACache,f,biddyOne,cube,&r,&cindex))
  {

    if ((biddyManagerType == BIDDYTYPEOBDDC) || (biddyManagerType == BIDDYTYPEOBDD)) {
      fv = BiddyV(f);
      cv = BiddyV(cube);
      while (!Biddy_IsConstant(cube) && BiddyIsSmaller(cv,fv)) {
        cube = BiddyT(cube);
        cv = BiddyV(cube);
      }
      if (Biddy_IsConstant(cube)) {
        return f;
      }
      if (cv == fv) {
        e = BiddyManagedExistAbstract(MNG,
              Biddy_InvCond(BiddyE(f),Biddy_GetMark(f)),BiddyT(cube));
        t = BiddyManagedExistAbstract(MNG,
              Biddy_InvCond(BiddyT(f),Biddy_GetMark(f)),BiddyT(cube));
        r = BiddyManagedOr(MNG,e,t);
      } else {
        e = BiddyManagedExistAbstract(MNG,
              Biddy_InvCond(BiddyE(f),Biddy_GetMark(f)),cube);
        t = BiddyManagedExistAbstract(MNG,
              Biddy_InvCond(BiddyT(f),Biddy_GetMark(f)),cube);
        r = BiddyManagedTaggedFoaNode(MNG,fv,e,t,fv,TRUE);
        BiddyRefresh(r); /* FoaNode returns an obsolete node! */
      }
    }

    else if (biddyManagerType == BIDDYTYPEZBDDC) {
      if (Biddy_IsConstant(cube)) {
        return f;
      }
      cv = BiddyV(cube);
      fv = BiddyV(f);
      if (BiddyIsSmaller(cv,fv)) {
        r = BiddyManagedExistAbstract(MNG,f,BiddyT(cube));
        r = BiddyManagedTaggedFoaNode(MNG,cv,r,r,0,TRUE);
        BiddyRefresh(r); /* FoaNode returns an obsolete node! */
      }
      else if (cv == fv) {
        e = BiddyManagedExistAbstract(MNG,
              Biddy_InvCond(BiddyE(f),Biddy_GetMark(f)),BiddyT(cube));
        t = BiddyManagedExistAbstract(MNG,BiddyT(f),BiddyT(cube));
        r = BiddyManagedOr(MNG,e,t);
        r = BiddyManagedTaggedFoaNode(MNG,cv,r,r,0,TRUE);
        BiddyRefresh(r); /* FoaNode returns an obsolete node! */
      } else {
        e = BiddyManagedExistAbstract(MNG,
              Biddy_InvCond(BiddyE(f),Biddy_GetMark(f)),cube);
        t = BiddyManagedExistAbstract(MNG,BiddyT(f),cube);
        r = BiddyManagedTaggedFoaNode(MNG,fv,e,t,0,TRUE);
        BiddyRefresh(r); /* FoaNode returns an obsolete node! */
      }
    }

    else if (biddyManagerType == BIDDYTYPETZBDD) {
      tag = Biddy_GetTag(f);
      cv = BiddyV(cube);
      while (!Biddy_IsConstant(cube) && BiddyIsSmaller(cv,tag)) {
        cube = BiddyT(cube);
        cv = BiddyV(cube);
      }
      if (Biddy_IsConstant(cube)) {
        return f;
      }
      fv = BiddyV(f);

      /* VARIANT 1 */
      /*
      if (cv == tag) {
        if (tag == fv) {
          e = BiddyManagedExistAbstract(MNG,BiddyE(f),BiddyT(cube));
          t = BiddyManagedExistAbstract(MNG,BiddyT(f),BiddyT(cube));
          r = BiddyManagedOr(MNG,e,t);
        } else {
          r = Biddy_Managed_IncTag(MNG,f);
          r = BiddyManagedExistAbstract(MNG,r,BiddyT(cube));
        }
      } else if (BiddyIsSmaller(cv,fv)) {
        r = f;
        Biddy_SetTag(r,cv);
        r = Biddy_Managed_IncTag(MNG,r);
        r = BiddyManagedExistAbstract(MNG,r,BiddyT(cube));
        r = BiddyManagedTaggedFoaNode(MNG,cv,r,r,tag,TRUE);
        BiddyRefresh(r); / * FoaNode returns an obsolete node! * /
      } else if (cv == fv) {
        e = BiddyManagedExistAbstract(MNG,BiddyE(f),BiddyT(cube));
        t = BiddyManagedExistAbstract(MNG,BiddyT(f),BiddyT(cube));
        r = BiddyManagedOr(MNG,e,t);
        r = BiddyManagedTaggedFoaNode(MNG,cv,r,r,tag,TRUE);
        BiddyRefresh(r); / * FoaNode returns an obsolete node! * /
      } else {
        e = BiddyManagedExistAbstract(MNG,BiddyE(f),cube);
        t = BiddyManagedExistAbstract(MNG,BiddyT(f),cube);
        r = BiddyManagedTaggedFoaNode(MNG,fv,e,t,tag,TRUE);
        BiddyRefresh(r); / * FoaNode returns an obsolete node! * /
      }
      */

      /* VARIANT 2 */
      /**/
      if (cv == fv) {
        e = BiddyManagedExistAbstract(MNG,BiddyE(f),BiddyT(cube));
        t = BiddyManagedExistAbstract(MNG,BiddyT(f),BiddyT(cube));
        r = BiddyManagedOr(MNG,e,t);
        if (cv != tag) {
          r = BiddyManagedTaggedFoaNode(MNG,cv,r,r,tag,TRUE);
          BiddyRefresh(r); /* FoaNode returns an obsolete node! */
        }
      } else if (BiddyIsSmaller(cv,fv)) {
        if (cv == tag) {
          r = Biddy_Managed_IncTag(MNG,f);
          r = BiddyManagedExistAbstract(MNG,r,BiddyT(cube));
        } else {
          r = f;
          Biddy_SetTag(r,cv);
          r = Biddy_Managed_IncTag(MNG,r);
          r = BiddyManagedExistAbstract(MNG,r,BiddyT(cube));
          r = BiddyManagedTaggedFoaNode(MNG,cv,r,r,tag,TRUE);
          BiddyRefresh(r); /* FoaNode returns an obsolete node! */
        }
      } else {
        e = BiddyManagedExistAbstract(MNG,BiddyE(f),cube);
        t = BiddyManagedExistAbstract(MNG,BiddyT(f),cube);
        r = BiddyManagedTaggedFoaNode(MNG,fv,e,t,tag,TRUE);
        BiddyRefresh(r); /* FoaNode returns an obsolete node! */
      }
      /**/

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
    Implemented for OBDDC. Prototyped for OBDD.
    Prototyped for ZBDDs and TZBDDs.
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

  r = biddyNull;

  if (biddyManagerType == BIDDYTYPEOBDD) {
    /* PROTOTYPED */
    r = BiddyManagedNot(MNG,BiddyManagedExistAbstract(MNG,BiddyManagedNot(MNG,f),cube));
    BiddyRefresh(r); /* not always refreshed by BiddyManagedNot */
  } else if (biddyManagerType == BIDDYTYPEOBDDC) {
    /* IMPLEMENTED */
    r = BiddyManagedUnivAbstract(MNG,f,cube);
    BiddyRefresh(r); /* not always refreshed by BiddyManagedUnivAbstract */
  } else if (biddyManagerType == BIDDYTYPEZBDDC) {
    /* PROTOTYPED */
    r = BiddyManagedNot(MNG,BiddyManagedExistAbstract(MNG,BiddyManagedNot(MNG,f),cube));
    BiddyRefresh(r); /* not always refreshed by BiddyManagedNot */
  } else if (biddyManagerType == BIDDYTYPETZBDD) {
    /* PROTOTYPED */
    r = BiddyManagedNot(MNG,BiddyManagedExistAbstract(MNG,BiddyManagedNot(MNG,f),cube));
    BiddyRefresh(r); /* not always refreshed by BiddyManagedNot */
  } else if ((biddyManagerType == BIDDYTYPEZBDD) ||
              (biddyManagerType == BIDDYTYPETZBDDC))
  {
    fprintf(stderr,"Biddy_UnivAbstract: this GDD type is not supported, yet!\n");
    return biddyNull;
  } else if ((biddyManagerType == BIDDYTYPEOFDDC) || (biddyManagerType == BIDDYTYPEOFDD) ||
              (biddyManagerType == BIDDYTYPEZFDDC) || (biddyManagerType == BIDDYTYPEZFDD) ||
              (biddyManagerType == BIDDYTYPETZFDDC) || (biddyManagerType == BIDDYTYPETZFDD))
  {
    fprintf(stderr,"Biddy_UnivAbstract: this GDD type is not supported, yet!\n");
    return biddyNull;
  } else {
    fprintf(stderr,"Biddy_UnivAbstract: Unsupported GDD type!\n");
    return biddyNull;
  }

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

  /* IMPLEMENTED FOR OBDDC, ONLY */
  assert( biddyManagerType == BIDDYTYPEOBDDC );

  /* LOOKING FOR SIMPLE CASE */
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

  r = biddyNull;

  if (biddyManagerType == BIDDYTYPEOBDD) {
    /* IMPLEMENTED */
    r = BiddyManagedAndAbstract(MNG,f,g,cube);
    BiddyRefresh(r); /* not always refreshed by BiddyManagedAndAbstract */
  } else if (biddyManagerType == BIDDYTYPEOBDDC) {
    /* IMPLEMENTED */
    r = BiddyManagedAndAbstract(MNG,f,g,cube);
    BiddyRefresh(r); /* not always refreshed by BiddyManagedAndAbstract */
  } else if (biddyManagerType == BIDDYTYPEZBDDC) {
    /* IMPLEMENTED */
    r = BiddyManagedAndAbstract(MNG,f,g,cube);
    BiddyRefresh(r); /* not always refreshed by BiddyManagedAndAbstract */
  } else if (biddyManagerType == BIDDYTYPETZBDD) {
    /* IMPLEMENTED */
    r = BiddyManagedAndAbstract(MNG,f,g,cube);
    BiddyRefresh(r); /* not always refreshed by BiddyManagedAndAbstract */
  } else if ((biddyManagerType == BIDDYTYPEZBDD) ||
              (biddyManagerType == BIDDYTYPETZBDDC))
  {
    fprintf(stderr,"Biddy_AndAbstract: this GDD type is not supported, yet!\n");
    return biddyNull;
  } else if ((biddyManagerType == BIDDYTYPEOFDDC) || (biddyManagerType == BIDDYTYPEOFDD) ||
              (biddyManagerType == BIDDYTYPEZFDDC) || (biddyManagerType == BIDDYTYPEZFDD) ||
              (biddyManagerType == BIDDYTYPETZFDDC) || (biddyManagerType == BIDDYTYPETZFDD))
  {
    fprintf(stderr,"Biddy_AndAbstract: this GDD type is not supported, yet!\n");
    return biddyNull;
  } else {
    fprintf(stderr,"Biddy_AndAbstract: Unsupported GDD type!\n");
    return biddyNull;
  }

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
  Biddy_Variable ftag,gtag,mintag;
  Biddy_Boolean cgt;
  unsigned int cindex;

  assert( MNG != NULL );
  assert( f != NULL );
  assert( g != NULL );
  assert( cube != NULL );

  /* IMPLEMENTED FOR OBDD, OBDDC, ZBDDC, AND TZBDDC */
  assert(
    (biddyManagerType == BIDDYTYPEOBDD) ||
    (biddyManagerType == BIDDYTYPEOBDDC) ||
    (biddyManagerType == BIDDYTYPEZBDDC) ||
    (biddyManagerType == BIDDYTYPETZBDD)
  );

  r = biddyNull;

  /* LOOKING FOR SIMPLE CASE */
  if ((f == biddyZero) || (g == biddyZero)) {
    return biddyZero;
  } else if (f == biddyOne) {
    return BiddyManagedExistAbstract(MNG,g,cube);
  } else if (g == biddyOne) {
    return BiddyManagedExistAbstract(MNG,f,cube);
  }

  /* NORMALIZATION */
  if (((uintptr_t) f) > ((uintptr_t) g)) {
    exchangeEdges(&f,&g);
  }

  /* IF RESULT IS NOT IN THE CACHE TABLE... */
  cindex = 0;
  if (!findOp3Cache(MNG,biddyEACache,f,g,cube,&r,&cindex))
  {

    if ((biddyManagerType == BIDDYTYPEOBDDC) || (biddyManagerType == BIDDYTYPEOBDD)) {
      fv = BiddyV(f);
      gv = BiddyV(g);
      if (BiddyIsSmaller(fv,gv)) {
        f0 = Biddy_InvCond(BiddyE(f),Biddy_GetMark(f));
        f1 = Biddy_InvCond(BiddyT(f),Biddy_GetMark(f));
        g0 = g;
        g1 = g;
        minv = fv;
      } else if (BiddyIsSmaller(gv,fv)) {
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
      while (!Biddy_IsConstant(cube) && BiddyIsSmaller(cv,minv)) {
        cube = BiddyT(cube);
        cv = BiddyV(cube);
      }
      if (Biddy_IsConstant(cube)) {
        return BiddyManagedAnd(MNG,f,g);
        /* r = BiddyManagedAnd(MNG,f,g); */  /* I AM NOT SURE IF THIS SHOULD BE CACHED */
      } else {
        if (minv == cv) {
          /* Tricky optimizations are from: */
          /* B. Yang et al. A Performance Study of BDD-Based Model Checking. 1998. */
          /* http://fmv.jku.at/papers/YangEtAl-FMCAD98.pdf */
          e = BiddyManagedAndAbstract(MNG,f0,g0,BiddyT(cube));
          if (e == biddyOne) return biddyOne;
          if ((e == f1) || (e == g1)) {
            return e;
            /* r = e */ /* I AM NOT SURE IF THIS SHOULD BE CACHED */
          } else {
            if (e == Biddy_Inv(f1)) f1 = biddyOne; /* this is useful only for OBDDC, but not wrong for OBDD */
            if (e == Biddy_Inv(g1)) g1 = biddyOne; /* this is useful only for OBDDC, but not wrong for OBDD */
            t = BiddyManagedAndAbstract(MNG,f1,g1,BiddyT(cube));
            r = BiddyManagedOr(MNG,e,t);
          }
        } else {
          e = BiddyManagedAndAbstract(MNG,f0,g0,cube);
          t = BiddyManagedAndAbstract(MNG,f1,g1,cube);
          r = BiddyManagedTaggedFoaNode(MNG,minv,e,t,minv,TRUE);
          BiddyRefresh(r); /* FoaNode returns an obsolete node! */
        }
      }
    }

    else if (biddyManagerType == BIDDYTYPEZBDDC) {
      if (Biddy_IsConstant(cube)) {
        return BiddyManagedAnd(MNG,f,g);
        /* r = BiddyManagedAnd(MNG,f,g); */  /* I AM NOT SURE IF THIS SHOULD BE CACHED */
      } else {
        fv = BiddyV(f);
        gv = BiddyV(g);
        minv = BiddyIsSmaller(fv,gv) ? fv : gv;
        if (fv != gv) {
          if (fv == minv) {
            if (Biddy_GetMark(f)) {
              /* return BiddyManagedAndAbstract(MNG,Biddy_Inv(BiddyE(f)),g,cube); */
              r = BiddyManagedAndAbstract(MNG,Biddy_Inv(BiddyE(f)),g,cube);
            } else {
              /* return BiddyManagedAndAbstract(MNG,BiddyE(f),g,cube); */
              r = BiddyManagedAndAbstract(MNG,BiddyE(f),g,cube);
            }
          } else {
            if (Biddy_GetMark(g)) {
              /* return BiddyManagedAndAbstract(MNG,f,Biddy_Inv(BiddyE(g)),cube); */
              r = BiddyManagedAndAbstract(MNG,f,Biddy_Inv(BiddyE(g)),cube);
            } else {
              /* return BiddyManagedAndAbstract(MNG,f,BiddyE(g),cube); */
              r = BiddyManagedAndAbstract(MNG,f,BiddyE(g),cube);
            }
          }
        } else {
          cv = BiddyV(cube);
          if (BiddyIsSmaller(cv,minv)) {
            r = BiddyManagedAndAbstract(MNG,f,g,BiddyT(cube));
            r = BiddyManagedTaggedFoaNode(MNG,cv,r,r,0,TRUE);
            BiddyRefresh(r); /* FoaNode returns an obsolete node! */
          }
          else if (cv == minv) {
            f0 = Biddy_InvCond(BiddyE(f),Biddy_GetMark(f));
            f1 = BiddyT(f);
            g0 = Biddy_InvCond(BiddyE(g),Biddy_GetMark(g));
            g1 = BiddyT(g);
            e = BiddyManagedAndAbstract(MNG,f0,g0,BiddyT(cube));
            t = BiddyManagedAndAbstract(MNG,f1,g1,BiddyT(cube));
            r = BiddyManagedOr(MNG,e,t);
            r = BiddyManagedTaggedFoaNode(MNG,cv,r,r,0,TRUE);
            BiddyRefresh(r); /* FoaNode returns an obsolete node! */
          } else {
            f0 = Biddy_InvCond(BiddyE(f),Biddy_GetMark(f));
            f1 = BiddyT(f);
            g0 = Biddy_InvCond(BiddyE(g),Biddy_GetMark(g));
            g1 = BiddyT(g);
            e = BiddyManagedAndAbstract(MNG,f0,g0,cube);
            t = BiddyManagedAndAbstract(MNG,f1,g1,cube);
            r = BiddyManagedTaggedFoaNode(MNG,minv,e,t,0,TRUE);
            BiddyRefresh(r); /* FoaNode returns an obsolete node! */
          }
        }
      }
    }

    else if (biddyManagerType == BIDDYTYPETZBDD) {
      ftag = Biddy_GetTag(f);
      gtag = Biddy_GetTag(g);
      mintag = BiddyIsSmaller(ftag,gtag) ? ftag : gtag;
      cv = BiddyV(cube);
      while (!Biddy_IsConstant(cube) && BiddyIsSmaller(cv,mintag)) {
        cube = BiddyT(cube);
        cv = BiddyV(cube);
      }
      if (Biddy_IsConstant(cube)) {
        return BiddyManagedAnd(MNG,f,g);
        /* r = BiddyManagedAnd(MNG,f,g); */  /* I AM NOT SURE IF THIS SHOULD BE CACHED */
      } else {

        /* VARIANT 1 */
        /*
        fv = BiddyV(f);
        gv = BiddyV(g);
        minv = BiddyIsSmaller(fv,gv) ? fv : gv;
        if (BiddyIsSmaller(fv,gtag)) {
          f0 = BiddyE(f);
          f1 = BiddyT(f);
          g0 = g1 = g;
        } else if (BiddyIsSmaller(gv,ftag)) {
          f0 = f1 = f;
          g0 = BiddyE(g);
          g1 = BiddyT(g);
        } else {
          if (minv == fv) {
            f0 = BiddyE(f);
            f1 = BiddyT(f);
          } else {
            f0 = f;
            Biddy_SetTag(f0,minv);
            f0 = Biddy_Managed_IncTag(MNG,f0);
            f1 = biddyZero;
          }
          if (minv == gv) {
            g0 = BiddyE(g);
            g1 = BiddyT(g);
          } else {
            g0 = g;
            Biddy_SetTag(g0,minv);
            g0 = Biddy_Managed_IncTag(MNG,g0);
            g1 = biddyZero;
          }
        }
        if (BiddyIsSmaller(minv,cv)) {
          e = BiddyManagedAndAbstract(MNG,f0,g0,cube);
          t = BiddyManagedAndAbstract(MNG,f1,g1,cube);
        } else {
          e = BiddyManagedAndAbstract(MNG,f0,g0,BiddyT(cube));
          t = BiddyManagedAndAbstract(MNG,f1,g1,BiddyT(cube));
        }
        if (cv == mintag) {
          if (minv == mintag) {
            r = BiddyManagedOr(MNG,e,t);
          } else {
            r = BiddyManagedTaggedFoaNode(MNG,minv,e,t,mintag,TRUE);
            BiddyRefresh(r); / * FoaNode returns an obsolete node! * /
            if (r != biddyZero) r = Biddy_Managed_IncTag(MNG,r);
          }
        } else if (BiddyIsSmaller(cv,minv)) {
          r = BiddyManagedTaggedFoaNode(MNG,minv,e,t,cv,TRUE);
          r = Biddy_Managed_IncTag(MNG,r);
          r = BiddyManagedTaggedFoaNode(MNG,cv,r,r,mintag,TRUE);
          BiddyRefresh(r); / * FoaNode returns an obsolete node! * /
        } else if (cv == minv) {
          r = BiddyManagedOr(MNG,e,t);
          r = BiddyManagedTaggedFoaNode(MNG,cv,r,r,mintag,TRUE);
          BiddyRefresh(r); / * FoaNode returns an obsolete node! * /
        } else {
          r = BiddyManagedTaggedFoaNode(MNG,minv,e,t,mintag,TRUE);
          BiddyRefresh(r); / * FoaNode returns an obsolete node! * /
        }
        */

        /* VARIANT 2 */
        /**/
        fv = BiddyV(f);
        gv = BiddyV(g);
        minv = BiddyIsSmaller(fv,gv) ? fv : gv;
        if (BiddyIsSmaller(minv,ftag)) {
          f0 = f1 = f;
        } else if (minv == fv) {
          f0 = BiddyE(f);
          f1 = BiddyT(f);
        } else {
          f0 = f;
          Biddy_SetTag(f0,minv);
          f0 = Biddy_Managed_IncTag(MNG,f0);
          f1 = biddyZero;
        }
        if (BiddyIsSmaller(minv,gtag)) {
          g0 = g1 = g;
        } else if (minv == gv) {
          g0 = BiddyE(g);
          g1 = BiddyT(g);
        } else {
          g0 = g;
          Biddy_SetTag(g0,minv);
          g0 = Biddy_Managed_IncTag(MNG,g0);
          g1 = biddyZero;
        }
        cgt = BiddyIsSmaller(minv,cv);
        if (f0 == biddyZero) {
          e = biddyZero;
        } else if (g0 == biddyZero) {
          e = biddyZero;
        } else {
          e = BiddyManagedAndAbstract(MNG,f0,g0,cgt?cube:BiddyT(cube));
        }
        if (f1 == biddyZero) {
          t = biddyZero;
        } else if (g1 == biddyZero) {
          t = biddyZero;
        } else {
          t = BiddyManagedAndAbstract(MNG,f1,g1,cgt?cube:BiddyT(cube));
        }

        /* VARIANT 2A */
        /*
        if (cv == mintag) {
          if (minv == mintag) {
            r = BiddyManagedOr(MNG,e,t);
          } else {
            r = BiddyManagedTaggedFoaNode(MNG,minv,e,t,mintag,TRUE);
            BiddyRefresh(r); / * FoaNode returns an obsolete node! * /
            if (r != biddyZero) r = Biddy_Managed_IncTag(MNG,r);
          }
        } else if (BiddyIsSmaller(cv,minv)) {
          r = BiddyManagedTaggedFoaNode(MNG,minv,e,t,cv,TRUE);
          BiddyRefresh(r); / * FoaNode returns an obsolete node! * /
          if (r != biddyZero) r = Biddy_Managed_IncTag(MNG,r);
          r = BiddyManagedTaggedFoaNode(MNG,cv,r,r,mintag,TRUE);
          BiddyRefresh(r); / * FoaNode returns an obsolete node! * /
        } else if (cv == minv) {
          r = BiddyManagedOr(MNG,e,t);
          r = BiddyManagedTaggedFoaNode(MNG,cv,r,r,mintag,TRUE);
          BiddyRefresh(r); / * FoaNode returns an obsolete node! * /
        } else {
          r = BiddyManagedTaggedFoaNode(MNG,minv,e,t,mintag,TRUE);
          BiddyRefresh(r); / * FoaNode returns an obsolete node! * /
        }
        */

        /* VARIANT 2B */
        if (cv == minv) {
          r = BiddyManagedOr(MNG,e,t);
          if (cv != mintag) {
            r = BiddyManagedTaggedFoaNode(MNG,cv,r,r,mintag,TRUE);
            BiddyRefresh(r); /* FoaNode returns an obsolete node! */
          }
        } else if (BiddyIsSmaller(cv,minv)) {
          r = BiddyManagedTaggedFoaNode(MNG,minv,e,t,cv,TRUE);
          BiddyRefresh(r); /* FoaNode returns an obsolete node! */
          if (r != biddyZero) r = Biddy_Managed_IncTag(MNG,r);
          if (cv != mintag) {
            r = BiddyManagedTaggedFoaNode(MNG,cv,r,r,mintag,TRUE);
            BiddyRefresh(r); /* FoaNode returns an obsolete node! */
          }
        } else {
          r = BiddyManagedTaggedFoaNode(MNG,minv,e,t,mintag,TRUE);
          BiddyRefresh(r); /* FoaNode returns an obsolete node! */
        }
        /**/

      }
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
    Implemented for OBDDs.
    Cache table is not used.
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

  r = biddyNull;

  if (biddyManagerType == BIDDYTYPEOBDD) {
    /* IMPLEMENTED */
    r = BiddyManagedConstrain(MNG,f,c);
    BiddyRefresh(r); /* not always refreshed by BiddyManagedConstrain */
  } else if (biddyManagerType == BIDDYTYPEOBDDC) {
    /* IMPLEMENTED */
    r = BiddyManagedConstrain(MNG,f,c);
    BiddyRefresh(r); /* not always refreshed by BiddyManagedConstrain */
  } else if ((biddyManagerType == BIDDYTYPEZBDDC) ||
              (biddyManagerType == BIDDYTYPETZBDD))
  {
    fprintf(stderr,"Biddy_Constrain: this GDD type is not supported, yet!\n");
    return biddyNull;
  } else if ((biddyManagerType == BIDDYTYPEZBDD) ||
              (biddyManagerType == BIDDYTYPETZBDDC))
  {
    fprintf(stderr,"Biddy_Constrain: this GDD type is not supported, yet!\n");
    return biddyNull;
  } else if ((biddyManagerType == BIDDYTYPEOFDDC) || (biddyManagerType == BIDDYTYPEOFDD) ||
              (biddyManagerType == BIDDYTYPEZFDDC) || (biddyManagerType == BIDDYTYPEZFDD) ||
              (biddyManagerType == BIDDYTYPETZFDDC) || (biddyManagerType == BIDDYTYPETZFDD))
  {
    fprintf(stderr,"Biddy_Constrain: this GDD type is not supported, yet!\n");
    return biddyNull;
  } else {
    fprintf(stderr,"Biddy_Constrain: Unsupported GDD type!\n");
    return biddyNull;
  }

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

  /* IMPLEMENTED FOR OBDD AND OBDDC */
  assert(
    (biddyManagerType == BIDDYTYPEOBDD) ||
    (biddyManagerType == BIDDYTYPEOBDDC)
  );

  r = biddyNull;

  /* LOOKING FOR SIMPLE CASE */
  if (c == biddyZero) {
    return biddyZero;
  } else if (c == biddyOne) {
    return f;
  } else if (Biddy_IsConstant(f)) {
    return f;
  }

  fv = BiddyV(f);
  cv = BiddyV(c);

  if (BiddyIsSmaller(fv,cv)) {
    f0 = Biddy_InvCond(BiddyE(f),Biddy_GetMark(f));
    f1 = Biddy_InvCond(BiddyT(f),Biddy_GetMark(f));
    c0 = c;
    c1 = c;
    minv = fv;
  } else if (BiddyIsSmaller(cv,fv)) {
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
    Implemented for OBDDs.
    Cache table is not used.
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

  r = biddyNull;

  if (biddyManagerType == BIDDYTYPEOBDD) {
    /* IMPLEMENTED */
    r = BiddyManagedSimplify(MNG,f,c);
    BiddyRefresh(r); /* not always refreshed by BiddyManagedSimplify */
  } else if (biddyManagerType == BIDDYTYPEOBDDC) {
    /* IMPLEMENTED */
    r = BiddyManagedSimplify(MNG,f,c);
    BiddyRefresh(r); /* not always refreshed by BiddyManagedSimplify */
  } else if ((biddyManagerType == BIDDYTYPEZBDDC) ||
              (biddyManagerType == BIDDYTYPETZBDD))
  {
    fprintf(stderr,"Biddy_Simplify: this GDD type is not supported, yet!\n");
    return biddyNull;
  } else if ((biddyManagerType == BIDDYTYPEOBDD) ||
              (biddyManagerType == BIDDYTYPEZBDD) ||
              (biddyManagerType == BIDDYTYPETZBDDC))
  {
    fprintf(stderr,"Biddy_Simplify: this GDD type is not supported, yet!\n");
    return biddyNull;
  } else if ((biddyManagerType == BIDDYTYPEOFDDC) || (biddyManagerType == BIDDYTYPEOFDD) ||
              (biddyManagerType == BIDDYTYPEZFDDC) || (biddyManagerType == BIDDYTYPEZFDD) ||
              (biddyManagerType == BIDDYTYPETZFDDC) || (biddyManagerType == BIDDYTYPETZFDD))
  {
    fprintf(stderr,"Biddy_Simplify: this GDD type is not supported, yet!\n");
    return biddyNull;
  } else {
    fprintf(stderr,"Biddy_Simplify: Unsupported GDD type!\n");
    return biddyNull;
  }

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

  /* IMPLEMENTED FOR OBDD AND OBDDC */
  assert(
    (biddyManagerType == BIDDYTYPEOBDD) ||
    (biddyManagerType == BIDDYTYPEOBDDC)
  );

  r = biddyNull;

  /* LOOKING FOR SIMPLE CASE */
  if (c == biddyZero) return biddyNull;
  if (c == biddyOne) {
    return f;
  } else if (Biddy_IsConstant(f)) {
    return f;
  }

  /* THIS IS FROM 20-CS-626-001 */
  /* http://gauss.ececs.uc.edu/Courses/c626/lectures/BDD/bdd-desc.pdf */
  if (f == c) return biddyOne;
  if (biddyManagerType == BIDDYTYPEOBDDC) {
    if (Biddy_IsEqvPointer(f,c)) {
      return biddyZero;
    }
  }

  fv = BiddyV(f);
  cv = BiddyV(c);

  /* SPECIAL CASE: f/~a == f/a */
  if (BiddyIsSmaller(cv,fv)) {
    return BiddyManagedSimplify(MNG,f,BiddyManagedE(MNG,c,cv));
  }

  if (BiddyIsSmaller(fv,cv)) {
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
       variables (OBDD and TZBDD) or the combination set containing a subset
       which includes all dependent variables (ZBDD).

### Description
    Implemented for OBDDs.
    Prototyped for ZBDDs and TZBDDs.
    For OBDD, dependent variables are all variables existing in the graph.
    For ZBDD and TZBDD, this is not true.
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

  r = biddyNull;

  if (biddyManagerType == BIDDYTYPEOBDD) {
    /* IMPLEMENTED */
    r = BiddyManagedSupport(MNG,f);
    BiddyRefresh(r); /* not always refreshed by BiddyManagedSupport */
  } else if (biddyManagerType == BIDDYTYPEOBDDC) {
    /* IMPLEMENTED */
    r = BiddyManagedSupport(MNG,f);
    BiddyRefresh(r); /* not always refreshed by BiddyManagedSupport */
  } else if (biddyManagerType == BIDDYTYPEZBDDC) {
    /* PROTOTYPED USING Restrict */
    /*
    Biddy_Variable v;
    r = biddyTerminal;
    for (v=1;v<biddyVariableTable.num;v++) {
      if (BiddyManagedRestrict(MNG,f,v,FALSE) != BiddyManagedRestrict(MNG,f,v,TRUE)) {
        r = BiddyManagedChange(MNG,r,v);
      }
    }
    */
    /* IMPLEMENTED */
    r = BiddyManagedSupport(MNG,f);
    BiddyRefresh(r); /* not always refreshed by BiddyManagedSupport */
  } else if (biddyManagerType == BIDDYTYPETZBDD) {
    /* PROTOTYPED USING Restrict */
    /*
    Biddy_Variable v;
    r = biddyOne;
    for (v=1;v<biddyVariableTable.num;v++) {
      if (BiddyManagedRestrict(MNG,f,v,FALSE) != BiddyManagedRestrict(MNG,f,v,TRUE)) {
        r = BiddyManagedAnd(MNG,r,biddyVariableTable.table[v].variable);
      }
    }
    */
    r = BiddyManagedSupport(MNG,f);
    BiddyRefresh(r); /* not always refreshed by BiddyManagedSupport */
  } else if ((biddyManagerType == BIDDYTYPEZBDD) ||
              (biddyManagerType == BIDDYTYPETZBDDC))
  {
    fprintf(stderr,"Biddy_Support: this GDD type is not supported, yet!\n");
    return biddyNull;
  } else if ((biddyManagerType == BIDDYTYPEOFDDC) || (biddyManagerType == BIDDYTYPEOFDD) ||
              (biddyManagerType == BIDDYTYPEZFDDC) || (biddyManagerType == BIDDYTYPEZFDD) ||
              (biddyManagerType == BIDDYTYPETZFDDC) || (biddyManagerType == BIDDYTYPETZFDD))
  {
    fprintf(stderr,"Biddy_Support: this GDD type is not supported, yet!\n");
    return biddyNull;
  } else {
    fprintf(stderr,"Biddy_Support: Unsupported GDD type!\n");
    return biddyNull;
  }

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

  /* IMPLEMENTED FOR OBDD, OBDDC, ZBDDC, and TZBDD */
  assert(
    (biddyManagerType == BIDDYTYPEOBDD) ||
    (biddyManagerType == BIDDYTYPEOBDDC) ||
    (biddyManagerType == BIDDYTYPEZBDDC) ||
    (biddyManagerType == BIDDYTYPETZBDD)
  );

  if (Biddy_IsNull(f)) return biddyNull;
  if (Biddy_IsConstant(f)) return biddyZero;

  /* MAKE VARIABLES UNSELECTED */
  /*
  for (v=1;v<biddyVariableTable.num;v++) {
    biddyVariableTable.table[v].selected = FALSE;
  }
  */

  if ((biddyManagerType == BIDDYTYPEOBDDC) || (biddyManagerType == BIDDYTYPEOBDD)) {
    n = 1; /* NUMBER OF NODES NOT NEEDED, BUT NODES ARE COUNTED BY BiddyNodeVarNumber */
    Biddy_Managed_SelectNode(MNG,biddyTerminal); /* for OBDDs, this is needed for BiddyNodeVarNumber */
    BiddyNodeVarNumber(MNG,f,&n); /* DEPENDENT VARIABLES ARE MARKED, all nodes are selected */
    Biddy_Managed_DeselectAll(MNG);
  }
  else if (biddyManagerType == BIDDYTYPEZBDDC) {
    BiddyCreateLocalInfo(MNG,f); /* FOR ZBDDs, localinfo IS USED BY BiddyNodeVarNumber */
    n = 1; /* NUMBER OF NODES NOT NEEDED, BUT NODES ARE COUNTED BY BiddyNodeVarNumber */
    BiddyNodeVarNumber(MNG,f,&n); /* DEPENDENT VARIABLES ARE MARKED, selects all except terminal node */
    BiddyDeleteLocalInfo(MNG,f); /* FOR ZBDDs, localinfo IS USED BY BiddyNodeVarNumber */
  }
  else if (biddyManagerType == BIDDYTYPETZBDD) {
    n = 1; /* NUMBER OF NODES NOT NEEDED, BUT NODES ARE COUNTED BY BiddyNodeVarNumber */
    Biddy_Managed_SelectNode(MNG,biddyTerminal); /* for TZBDDs, this is needed for BiddyNodeVarNumber */
    BiddyNodeVarNumber(MNG,f,&n); /* DEPENDENT VARIABLES ARE MARKED, all nodes are selected */
    Biddy_Managed_DeselectAll(MNG);
  }

  s = biddyNull;
  if ((biddyManagerType == BIDDYTYPEOBDDC) || (biddyManagerType == BIDDYTYPEOBDD)) {
    s = biddyOne;
    for (v=1;v<biddyVariableTable.num;v++) {
      if (biddyVariableTable.table[v].selected == TRUE) {
        s = BiddyManagedAnd(MNG,s,biddyVariableTable.table[v].variable);
        biddyVariableTable.table[v].selected = FALSE;
      }
    }
  }
  else if (biddyManagerType == BIDDYTYPEZBDDC) {
    /* NOTE: for ZBDDs, variables above the top variable (which are always all dependent) */
    /* are not selected by BiddyNodeVarNumber! */
    s = biddyTerminal; /* this is base set */
    for (v=1;v<biddyVariableTable.num;v++) {
      if ((biddyVariableTable.table[v].selected == TRUE) || BiddyIsSmaller(v,BiddyV(f)))
      {
        s = BiddyManagedChange(MNG,s,v);
        biddyVariableTable.table[v].selected = FALSE;
      }
    }
  }
  else if (biddyManagerType == BIDDYTYPETZBDD) {
    s = biddyOne;
    for (v=1;v<biddyVariableTable.num;v++) {
      if (biddyVariableTable.table[v].selected == TRUE) {
        s = BiddyManagedAnd(MNG,s,biddyVariableTable.table[v].variable);
        biddyVariableTable.table[v].selected = FALSE;
      }
    }
  }

  return s;
}

/***************************************************************************//*!
\brief Function Biddy_Managed_ReplaceByKeyword calculates Boolean function
        with one or more variables replaced.

### Description
    Implemented for OBDDs.
    Prototyped for ZBDDs and TZBDDs (via And-Xor-Not-Restrict).
    Original BDD is not changed.
    The sets of current and new variables should be disjoint.
    Replacing is controlled by variable's values (which are edges!).
    Use Biddy_Managed_ResetVariablesValue and Biddy_Managed_SetVariableValue
    to prepare replacing.
    Parameter keyword is used to maintain cache table. If (keyword == NULL)
    then entries in the cache table from previous calculations are deleted.
### Side effects
### More info
    Macro Biddy_ReplaceByKeyword(f,keyword) is defined for use with anonymous
    manager. Macros Biddy_Managed_Replace(MNG,f) and Biddy_Replace(f) are
    variants with less effective cache table.
*******************************************************************************/

#ifdef __cplusplus
extern "C" {
#endif

Biddy_Edge
Biddy_Managed_ReplaceByKeyword(Biddy_Manager MNG, Biddy_Edge f,
                               Biddy_String keyword)
{
  Biddy_Edge e,t,tmp,r;
  Biddy_Variable v;
  unsigned int key;
  Biddy_Boolean OK;
  int cc;
  const Biddy_Boolean disableCache = FALSE;

  assert( f != NULL );

  if (!MNG) MNG = biddyAnonymousManager;
  ZF_LOGI("Biddy_Replace");

  assert( BiddyIsOK(f) == TRUE );

  r = biddyNull;

  /* find keyword in keywordList */

  /* VARIANT A: LINEAR SEARCH */
  if (!disableCache && keyword) {
    OK = FALSE;
    if (!biddyReplaceCache.keywordList) {
      key = 0;
    } else {
      for (key = 0; !OK && (key < biddyReplaceCache.keywordNum); key++) {
        cc = strcmp(keyword,biddyReplaceCache.keywordList[key]);
        if (cc == 0) {
          OK = TRUE;
          break;
        }
        if (cc < 0) break;
      }
    }
  }

  /* TO DO: BINARY SEARCH */
  /* NOT IMPORTANT BECAUSE NUMBER OF KEYWORDS IS VERY SMALL IN THE PRACTICE */

  /* keyword not found - it will be added */
  if (disableCache || !keyword) {
    key = 0;
  } else {
    if (!OK) {
      Biddy_String *tmp1;
      unsigned int *tmp2;

      biddyReplaceCache.keywordNum++;
      if (!(tmp1 = (Biddy_String *)
         realloc(biddyReplaceCache.keywordList,biddyReplaceCache.keywordNum*sizeof(Biddy_String))))
      {
        fprintf(stderr,"Biddy_Managed_ReplaceByKeyword: Out of memoy!\n");
        exit(1);
      }
      biddyReplaceCache.keywordList = tmp1;
      if (!(tmp2 = (unsigned int *)
         realloc(biddyReplaceCache.keyList,biddyReplaceCache.keywordNum*sizeof(unsigned int))))
      {
        fprintf(stderr,"Biddy_Managed_ReplaceByKeyword: Out of memoy!\n");
        exit(1);
      }
      biddyReplaceCache.keyList = tmp2;

      memmove(&biddyReplaceCache.keywordList[key+1],&biddyReplaceCache.keywordList[key],(biddyReplaceCache.keywordNum-key-1)*sizeof(Biddy_String));
      memmove(&biddyReplaceCache.keyList[key+1],&biddyReplaceCache.keyList[key],(biddyReplaceCache.keywordNum-key-1)*sizeof(unsigned int));
      /*
      for (i = biddyReplaceCache.keywordNum-1; i > key; i--) {
        biddyReplaceCache.keywordList[i] = biddyReplaceCache.keywordList[i-1];
        biddyReplaceCache.keyList[i] = biddyReplaceCache.keyList[i-1];
      }
      */

      biddyReplaceCache.keywordList[key] = strdup(keyword);
      biddyReplaceCache.keyList[key] = biddyReplaceCache.keywordNum;
    }
    key = biddyReplaceCache.keyList[key];
  }

  /* DEBUGGING */
  /*
  fprintf(stderr,"ReplaceByKeyword key = %u (%s)\n",key,keyword);
  */

  if (biddyManagerType == BIDDYTYPEOBDD) {
    /* IMPLEMENTED */
    if (!disableCache && !key) ReplaceGarbageDeleteAll(MNG);
    r = BiddyManagedReplaceByKeyword(MNG,f,key);
    BiddyRefresh(r); /* not always refreshed by BiddyManagedReplaceByKeyword */
    Biddy_Managed_DeselectAll(MNG);
  } else if (biddyManagerType == BIDDYTYPEOBDDC) {
    /* IMPLEMENTED */
    if (!disableCache && !key) ReplaceGarbageDeleteAll(MNG);
    r = BiddyManagedReplaceByKeyword(MNG,f,key);
    BiddyRefresh(r); /* not always refreshed by BiddyManagedReplaceByKeyword */
    Biddy_Managed_DeselectAll(MNG);
  } else if (biddyManagerType == BIDDYTYPEZBDDC) {
    /* PROTOTYPED */
    r = f;
    for (v=1;v<biddyVariableTable.num;v++) {
      if ((tmp=biddyVariableTable.table[v].value) != biddyZero) {
        e = BiddyManagedRestrict(MNG,r,v,FALSE);
        t = BiddyManagedRestrict(MNG,r,v,TRUE);
        r = BiddyManagedXor(MNG,
              BiddyManagedAnd(MNG,tmp,t),
              BiddyManagedAnd(MNG,BiddyManagedNot(MNG,tmp),e));
      }
    }
    BiddyRefresh(r); /* not always refreshed by BiddyManagedXor */
  } else if (biddyManagerType == BIDDYTYPETZBDD) {
    /* PROTOTYPED */
    r = f;
    for (v=1;v<biddyVariableTable.num;v++) {
      if ((tmp=biddyVariableTable.table[v].value) != biddyZero) {
        e = BiddyManagedRestrict(MNG,r,v,FALSE);
        t = BiddyManagedRestrict(MNG,r,v,TRUE);
        r = BiddyManagedXor(MNG,
              BiddyManagedAnd(MNG,tmp,t),
              BiddyManagedAnd(MNG,BiddyManagedNot(MNG,tmp),e));
      }
    }
    BiddyRefresh(r); /* not always refreshed by BiddyManagedXor */
  } else if ((biddyManagerType == BIDDYTYPEOBDD) ||
              (biddyManagerType == BIDDYTYPEZBDD) ||
              (biddyManagerType == BIDDYTYPETZBDDC))
  {
    fprintf(stderr,"Biddy_ReplaceByKeyword: this GDD type is not supported, yet!\n");
    return biddyNull;
  } else if ((biddyManagerType == BIDDYTYPEOFDDC) || (biddyManagerType == BIDDYTYPEOFDD) ||
              (biddyManagerType == BIDDYTYPEZFDDC) || (biddyManagerType == BIDDYTYPEZFDD) ||
              (biddyManagerType == BIDDYTYPETZFDDC) || (biddyManagerType == BIDDYTYPETZFDD))
  {
    fprintf(stderr,"Biddy_ReplaceByKeyword: this GDD type is not supported, yet!\n");
    return biddyNull;
  } else {
    fprintf(stderr,"Biddy_ReplaceByKeyword: Unsupported GDD type!\n");
    return biddyNull;
  }

  return r;
}

#ifdef __cplusplus
}
#endif

Biddy_Edge
BiddyManagedReplaceByKeyword(Biddy_Manager MNG, Biddy_Edge f, const unsigned int key)
{
  Biddy_Edge e,t,r;
  Biddy_Variable fv;
  Biddy_Edge FF;
  Biddy_Boolean NN;
  unsigned int cindex;

  assert( MNG != NULL );
  assert( f != NULL );

  /* IMPLEMENTED FOR OBDD AND OBDDC */
  assert(
    (biddyManagerType == BIDDYTYPEOBDD) ||
    (biddyManagerType == BIDDYTYPEOBDDC)
  );

  if (Biddy_IsConstant(f)) return f;

  if (key && Biddy_GetMark(f)) {
    NN = TRUE;
    FF = Biddy_Inv(f);
  } else {
    NN = FALSE;
    FF = f;
  }

  /* IF RESULT IS NOT IN THE CACHE TABLE... */
  cindex = 0;
  if (!key || !findKeywordCache(MNG,biddyReplaceCache,FF,key,&r,&cindex))
  {
    fv = BiddyV(f);
    if ((r=biddyVariableTable.table[fv].value) != biddyZero) {

      assert( r != NULL );

      fv = BiddyV(r);

      assert( fv != 0 );

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

    e = BiddyManagedReplaceByKeyword(MNG,Biddy_InvCond(BiddyE(f),Biddy_GetMark(f)),key);
    t = BiddyManagedReplaceByKeyword(MNG,Biddy_InvCond(BiddyT(f),Biddy_GetMark(f)),key);

    /* VARIANT A */
    /* THIS IS NOT SLOW BECAUSE CACHE TABLE FOR ITE RESOLVES A LOT OF CALLS */
    /**/
    r = BiddyManagedITE(MNG,biddyVariableTable.table[fv].variable,t,e);
    /**/

    /* VARIANT B */
    /* THIS IS NOT AS FAST AS WE WANT IT */
    /*
    if (BiddyIsSmaller(fv,BiddyV(e)) && BiddyIsSmaller(fv,BiddyV(t))) {
      r = BiddyManagedTaggedFoaNode(MNG,fv,e,t,fv,TRUE);
      BiddyRefresh(r);
    } else {
      r = BiddyManagedITE(MNG,biddyVariableTable.table[fv].variable,t,e);
    }
    */

    if (key) {
      if (NN) {
        addKeywordCache(MNG,biddyReplaceCache,FF,key,Biddy_Inv(r),cindex);
      } else {
        addKeywordCache(MNG,biddyReplaceCache,FF,key,r,cindex);
      }
    }

  } else {

    if (NN) {
      Biddy_InvertMark(r);
    }

    /* IF THE RESULT IS FROM CACHE TABLE, REFRESH IT! */
    BiddyRefresh(r);

  }

  return r;
}

/***************************************************************************//*!
\brief Function Biddy_Managed_Change change the form of the given variable
       (positive literal becomes negative and vice versa).

### Description
### Side effects
    RC Cache is used with parameters (f,biddyNull,v).
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

  r = biddyNull;

  if (biddyManagerType == BIDDYTYPEOBDD) {
    /* IMPLEMENTED */
    r = BiddyManagedChange(MNG,f,v);
    BiddyRefresh(r); /* not always refreshed by BiddyManagedChange */
  } else if (biddyManagerType == BIDDYTYPEOBDDC) {
    /* IMPLEMENTED */
    r = BiddyManagedChange(MNG,f,v);
    BiddyRefresh(r); /* not always refreshed by BiddyManagedChange */
  } else if (biddyManagerType == BIDDYTYPEZBDDC) {
    /* IMPLEMENTED */
    r = BiddyManagedChange(MNG,f,v);
    BiddyRefresh(r); /* not always refreshed by BiddyManagedChange */
  } else if (biddyManagerType == BIDDYTYPETZBDD) {
    /* IMPLEMENTED */
    r = BiddyManagedChange(MNG,f,v);
    BiddyRefresh(r); /* not always refreshed by BiddyManagedChange */
  } else if ((biddyManagerType == BIDDYTYPEZBDD) ||
              (biddyManagerType == BIDDYTYPETZBDDC))
  {
    fprintf(stderr,"Biddy_Change: this GDD type is not supported, yet!\n");
    return biddyNull;
  } else if ((biddyManagerType == BIDDYTYPEOFDDC) || (biddyManagerType == BIDDYTYPEOFDD) ||
              (biddyManagerType == BIDDYTYPEZFDDC) || (biddyManagerType == BIDDYTYPEZFDD) ||
              (biddyManagerType == BIDDYTYPETZFDDC) || (biddyManagerType == BIDDYTYPETZFDD))
  {
    fprintf(stderr,"Biddy_Change: this GDD type is not supported, yet!\n");
    return biddyNull;
  } else {
    fprintf(stderr,"Biddy_Change: Unsupported GDD type!\n");
    return biddyNull;
  }

  return r;
}

#ifdef __cplusplus
}
#endif

Biddy_Edge
BiddyManagedChange(Biddy_Manager MNG, Biddy_Edge f, Biddy_Variable v)
{
  Biddy_Edge e,t,r;
  Biddy_Variable fv,tag;
  Biddy_Edge FF, GG, HH;
  unsigned int cindex;

  assert( MNG != NULL );
  assert( f != NULL );

  /* IMPLEMENTED FOR OBDD, OBDDC, ZBDDC, AND TZBDD */
  assert(
    (biddyManagerType == BIDDYTYPEOBDD) ||
    (biddyManagerType == BIDDYTYPEOBDDC) ||
    (biddyManagerType == BIDDYTYPEZBDDC) ||
    (biddyManagerType == BIDDYTYPETZBDD)
  );

  if (f == biddyZero) return biddyZero;

  r = biddyNull;

  FF = f;
  GG = biddyNull;
  HH = biddyVariableTable.table[v].variable;

  /* IF RESULT IS NOT IN THE CACHE TABLE... */
  /* TO DO: CHECK ONLY IF IT IS POSSIBLE TO EXIST IN THE CACHE */
  cindex = 0;
  if (!findOp3Cache(MNG,biddyRCCache,FF,GG,HH,&r,&cindex))
  {

    if ((biddyManagerType == BIDDYTYPEOBDDC) || (biddyManagerType == BIDDYTYPEOBDD)) {
      fv = BiddyV(f);
      if (v == fv) {
        e = Biddy_InvCond(BiddyT(f),Biddy_GetMark(f));
        t = Biddy_InvCond(BiddyE(f),Biddy_GetMark(f));
        r = BiddyManagedTaggedFoaNode(MNG,v,e,t,v,TRUE);
        BiddyRefresh(r); /* FoaNode returns an obsolete node! */
      } else if (BiddyIsSmaller(v,fv)) {
        r = f;
      } else {
        e = BiddyManagedChange(MNG,Biddy_InvCond(BiddyE(f),Biddy_GetMark(f)),v);
        t = BiddyManagedChange(MNG,Biddy_InvCond(BiddyT(f),Biddy_GetMark(f)),v);
        r = BiddyManagedTaggedFoaNode(MNG,fv,e,t,fv,TRUE);
        BiddyRefresh(r); /* FoaNode returns an obsolete node! */
        addOp3Cache(MNG,biddyRCCache,FF,GG,HH,r,cindex);
      }
    }

    else if (biddyManagerType == BIDDYTYPEZBDDC) {
      fv = BiddyV(f);
      if (v == fv) {
        e = BiddyT(f);
        t = Biddy_InvCond(BiddyE(f),Biddy_GetMark(f));
        r = BiddyManagedTaggedFoaNode(MNG,v,e,t,0,TRUE);
        BiddyRefresh(r); /* FoaNode returns an obsolete node! */
      } else if (BiddyIsSmaller(v,fv)) {
        r = BiddyManagedTaggedFoaNode(MNG,v,biddyZero,f,0,TRUE);
        BiddyRefresh(r); /* FoaNode returns an obsolete node! */
      } else {
        e = BiddyManagedChange(MNG,Biddy_InvCond(BiddyE(f),Biddy_GetMark(f)),v);
        t = BiddyManagedChange(MNG,BiddyT(f),v);
        r = BiddyManagedTaggedFoaNode(MNG,fv,e,t,0,TRUE);
        BiddyRefresh(r); /* FoaNode returns an obsolete node! */
        addOp3Cache(MNG,biddyRCCache,FF,GG,HH,r,cindex);
      }
    }

    else if (biddyManagerType == BIDDYTYPETZBDD) {
      tag = Biddy_GetTag(f);
      if (BiddyIsSmaller(v,tag)) {
        r = f;
      } else {
        fv = BiddyV(f);
        if (v == fv) {
          e = BiddyT(f);
          t = BiddyE(f);
          r = BiddyManagedTaggedFoaNode(MNG,v,e,t,tag,TRUE);
          BiddyRefresh(r); /* FoaNode returns an obsolete node! */
        } else if (BiddyIsSmaller(v,fv)) {
          r = f;
          Biddy_SetTag(r,v);
          r = Biddy_Managed_IncTag(MNG,r);
          r = BiddyManagedTaggedFoaNode(MNG,v,biddyZero,r,tag,TRUE);
          BiddyRefresh(r); /* FoaNode returns an obsolete node! */
        } else {
          e = BiddyManagedChange(MNG,BiddyE(f),v);
          t = BiddyManagedChange(MNG,BiddyT(f),v);
          r = BiddyManagedTaggedFoaNode(MNG,fv,e,t,tag,TRUE);
          BiddyRefresh(r); /* FoaNode returns an obsolete node! */
          addOp3Cache(MNG,biddyRCCache,FF,GG,HH,r,cindex);
        }
      }
    }

  } else {

    /* IF THE RESULT IS FROM CACHE TABLE, REFRESH IT! */
    BiddyRefresh(r);

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
    Cache table for AND is used.
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

  r = biddyNull;

  if (biddyManagerType == BIDDYTYPEOBDD) {
    /* IMPLEMENTED */
    r = BiddyManagedSubset(MNG,f,v,value);
    BiddyRefresh(r); /* not always refreshed by BiddyManagedSubset */
  } else if (biddyManagerType == BIDDYTYPEOBDDC) {
    /* IMPLEMENTED */
    r = BiddyManagedSubset(MNG,f,v,value);
    BiddyRefresh(r); /* not always refreshed by BiddyManagedSubset */
  } else if (biddyManagerType == BIDDYTYPEZBDDC) {
    /* IMPLEMENTED */
    r = BiddyManagedSubset(MNG,f,v,value);
    BiddyRefresh(r); /* not always refreshed by BiddyManagedSubset */
  } else if (biddyManagerType == BIDDYTYPETZBDD) {
    /* IMPLEMENTED */
    r = BiddyManagedSubset(MNG,f,v,value);
    BiddyRefresh(r); /* not always refreshed by BiddyManagedSubset */
  } else if ((biddyManagerType == BIDDYTYPEZBDD) ||
              (biddyManagerType == BIDDYTYPETZBDDC))
  {
    fprintf(stderr,"Biddy_Subset: this GDD type is not supported, yet!\n");
    return biddyNull;
  } else if ((biddyManagerType == BIDDYTYPEOFDDC) || (biddyManagerType == BIDDYTYPEOFDD) ||
              (biddyManagerType == BIDDYTYPEZFDDC) || (biddyManagerType == BIDDYTYPEZFDD) ||
              (biddyManagerType == BIDDYTYPETZFDDC) || (biddyManagerType == BIDDYTYPETZFDD))
  {
    fprintf(stderr,"Biddy_Subset: this GDD type is not supported, yet!\n");
    return biddyNull;
  } else {
    fprintf(stderr,"Biddy_Subset: Unsupported GDD type!\n");
    return biddyNull;
  }

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
  Biddy_Variable fv,tag,ntag;
  Biddy_Edge FF, GG, HH;
  unsigned int cindex;

  assert( MNG != NULL );
  assert( f != NULL );

  /* IMPLEMENTED FOR OBDD, OBDDC, ZBDDC, AND TZBDD */
  assert(
    (biddyManagerType == BIDDYTYPEOBDD) ||
    (biddyManagerType == BIDDYTYPEOBDDC) ||
    (biddyManagerType == BIDDYTYPEZBDDC) ||
    (biddyManagerType == BIDDYTYPETZBDD)
  );

  if (f == biddyZero) return biddyZero;

  FF = biddyNull;
  GG = biddyNull;
  HH = biddyNull;

  h = biddyVariableTable.table[v].variable;
  if (!value) {
    h = BiddyManagedNot(MNG,h);
  }

  /* TO DO: CACHE TABLE FOR OBDDC COULD BE OPTIMIZED BY USING COMPLEMENTED EDGES */
  if (biddyManagerType == BIDDYTYPEOBDDC) {
    if (((uintptr_t) f) > ((uintptr_t) h)) {
      FF = f;
      GG = h;
      HH = biddyZero;
    } else {
      FF = h;
      GG = f;
      HH = biddyZero;
    }
  }

  else {
    if (((uintptr_t) f) > ((uintptr_t) h)) {
      FF = f;
      GG = h;
      HH = biddyZero;
    } else {
      FF = h;
      GG = f;
      HH = biddyZero;
    }
  }

  /* IF RESULT IS NOT IN THE CACHE TABLE... */
  /* TO DO: CHECK ONLY IF IT IS POSSIBLE TO EXIST IN THE CACHE */
  cindex = 0;
  if (!findOp3Cache(MNG,biddyOPCache,FF,GG,HH,&r,&cindex))
  {
    if ((biddyManagerType == BIDDYTYPEOBDDC) || (biddyManagerType == BIDDYTYPEOBDD)) {
      if ((fv=BiddyV(f)) == v) {
        if (value) {
          r = BiddyManagedTaggedFoaNode(MNG,v,biddyZero,Biddy_InvCond(BiddyT(f),Biddy_GetMark(f)),v,TRUE);
          BiddyRefresh(r); /* FoaNode returns an obsolete node! */
        } else {
          r = BiddyManagedTaggedFoaNode(MNG,v,Biddy_InvCond(BiddyE(f),Biddy_GetMark(f)),biddyZero,v,TRUE);
          BiddyRefresh(r); /* FoaNode returns an obsolete node! */
        }
      }
      else if (BiddyIsSmaller(v,fv)) {
        if (value) {
          r = BiddyManagedTaggedFoaNode(MNG,v,biddyZero,f,v,TRUE);
          BiddyRefresh(r); /* FoaNode returns an obsolete node! */
        } else {
          r = BiddyManagedTaggedFoaNode(MNG,v,f,biddyZero,v,TRUE);
          BiddyRefresh(r); /* FoaNode returns an obsolete node! */
        }
      } else {
        e = BiddyManagedSubset(MNG,Biddy_InvCond(BiddyE(f),Biddy_GetMark(f)),v,value);
        t = BiddyManagedSubset(MNG,Biddy_InvCond(BiddyT(f),Biddy_GetMark(f)),v,value);
        r = BiddyManagedTaggedFoaNode(MNG,fv,e,t,fv,TRUE);
        BiddyRefresh(r); /* FoaNode returns an obsolete node! */
        addOp3Cache(MNG,biddyOPCache,FF,GG,HH,r,cindex);
      }
    }

    else if (biddyManagerType == BIDDYTYPEZBDDC) {
      if ((fv=BiddyV(f)) == v) {
        if (value) {
          r = BiddyManagedTaggedFoaNode(MNG,v,biddyZero,BiddyT(f),0,TRUE);
          BiddyRefresh(r); /* FoaNode returns an obsolete node! */
        } else {
          r = Biddy_InvCond(BiddyE(f),Biddy_GetMark(f));
        }
      }
      else if (BiddyIsSmaller(v,fv)) {
        if (value) {
          r = biddyZero;
        } else {
          r = f;
        }
      } else {
        e = BiddyManagedSubset(MNG,Biddy_InvCond(BiddyE(f),Biddy_GetMark(f)),v,value);
        t = BiddyManagedSubset(MNG,BiddyT(f),v,value);
        r = BiddyManagedTaggedFoaNode(MNG,fv,e,t,0,TRUE);
        BiddyRefresh(r); /* FoaNode returns an obsolete node! */
        addOp3Cache(MNG,biddyOPCache,FF,GG,HH,r,cindex);
      }
    }

    else if (biddyManagerType == BIDDYTYPETZBDD) {
      tag = Biddy_GetTag(f);
      if (BiddyIsSmaller(v,tag)) {
        if (value) {
          r = BiddyManagedTaggedFoaNode(MNG,v,biddyZero,f,v,TRUE);
          BiddyRefresh(r); /* FoaNode returns an obsolete node! */
        } else {
          ntag = biddyVariableTable.table[v].next;
          if (tag == ntag) {
            r = f;
            Biddy_SetTag(r,v);
          } else {
            r = BiddyManagedTaggedFoaNode(MNG,ntag,f,f,v,TRUE);
            BiddyRefresh(r); /* FoaNode returns an obsolete node! */
          }
        }
      } else {
        fv = BiddyV(f);
        if (v == fv) {
          if (value) {
            r = BiddyManagedTaggedFoaNode(MNG,v,biddyZero,BiddyT(f),tag,TRUE);
            BiddyRefresh(r); /* FoaNode returns an obsolete node! */
          } else {
            ntag = biddyVariableTable.table[v].next;
            e = BiddyE(f);
            if (ntag == Biddy_GetTag(e)) {
              r = e;
              if (r != biddyZero) {
                Biddy_SetTag(r,tag);
              }
            } else {
              r = BiddyManagedTaggedFoaNode(MNG,ntag,e,e,tag,TRUE);
              BiddyRefresh(r); /* FoaNode returns an obsolete node! */
            }
          }
        } else if (BiddyIsSmaller(v,fv)) {
          if (value) {
            r = biddyZero;
          } else {
            r = f;
          }
        } else {
          e = BiddyManagedSubset(MNG,BiddyE(f),v,value);
          t = BiddyManagedSubset(MNG,BiddyT(f),v,value);
          r = BiddyManagedTaggedFoaNode(MNG,fv,e,t,tag,TRUE);
          BiddyRefresh(r); /* FoaNode returns an obsolete node! */
        }
      }
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
    Parameter target is used during sifting, in other cases 0 is used.
    Iff parameter purge is true then all formulae without name are deleted.
    Iff parameter purge is true then all nodes which are not part of any
    non-obsolete non-deleted formulae are removed even if they are fresh or
    fortified (this should not be used during automatic garbage collection!).
    If parameter total is true than unnecessary nodes are guaranted to be
    deleted, otherwise they are deleted only when there are enough of them.
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
  BiddyFormula *tmp;
  BiddyNode *tmpnode1,*tmpnode2;
  BiddyNode *sup;
  BiddyCacheList *c;
  Biddy_Boolean cacheOK;
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

  /* DEBUGGING - REPORT ALL NODES WITH A SPECIFIC VARIABLE */
  /* TO DO: CHANGE THIS INTO INTERNAL FUNCTION */
  /*
  {
  BiddyNode *sup;
  biddyVariableTable.table[3].lastNode->list = NULL;
  sup = biddyVariableTable.table[3].firstNode;
  while (sup) {
    printf("<\"%s\",",Biddy_Managed_GetTopVariableName(MNG,sup));
    if (Biddy_GetMark(sup->f)) printf("*");
    printf("\"%s\",",Biddy_Managed_GetTopVariableName(MNG,sup->f));
    if (Biddy_GetMark(sup->t)) printf("*");
    printf("\"%s\">",Biddy_Managed_GetTopVariableName(MNG,sup->t));
    printf("@%u",sup->expiry);
    sup = (BiddyNode *) sup->list;
  }
  printf("\n");
  }
  */

  starttime = clock();

  /* REMOVE ALL FORMULAE WHICH ARE NOT PRESERVED ANYMORE */
  /* The first two formulae ("0" and "1") are never deleted. */
  /* Iff parameter purge is true then all formulae without name are deleted. */

  i = 2;
  for (j = 2; j < biddyFormulaTable.size; j++) {
    
    /* DEBUGGING */
    /*
    printf("[FORM:%s@%u]",biddyFormulaTable.table[j].name,biddyFormulaTable.table[j].expiry);
    */

    if (purge && !biddyFormulaTable.table[j].name) {
      biddyFormulaTable.table[j].deleted = TRUE;
    }
   
    if (!biddyFormulaTable.table[j].deleted &&
        (!biddyFormulaTable.table[j].expiry || biddyFormulaTable.table[j].expiry >= biddySystemAge))
    {

      /* THIS FORMULA IS OK */
      if (i != j) {
        biddyFormulaTable.table[i] = biddyFormulaTable.table[j];
      }
      if (biddyFormulaTable.table[i].name) biddyFormulaTable.numOrdered = i+1;
      i++;

    } else {

      /* THIS FORMULA IS BAD */
      
      /* DEBUGGING */
      /*
      fprintf(stderr,"Biddy_Managed_GC: Formula %s will be deleted!\n",biddyFormulaTable.table[j].name);
      if (biddyFormulaTable.table[j].deleted) fprintf(stderr,"Biddy_Managed_GC: Formula has DELETED flag!\n");
      if (biddyFormulaTable.table[j].expiry && biddyFormulaTable.table[j].expiry < biddySystemAge)
        fprintf(stderr,"Biddy_Managed_GC: Formula is obsolete!\n");
      */

      if (biddyFormulaTable.table[j].name) free(biddyFormulaTable.table[j].name);

    }
  }

  /* UPDATE FORMULA TABLE */
  if (i != biddyFormulaTable.size) {
    biddyFormulaTable.size = i;
    if (!(tmp = (BiddyFormula *)
      realloc(biddyFormulaTable.table,biddyFormulaTable.size*sizeof(BiddyFormula))))
    {
      fprintf(stderr,"Biddy_Managed_GC: Out of memoy!\n");
      exit(1);
    }
    biddyFormulaTable.table = tmp;
  }

  /* if parameter purge is true than nodes which are not part of any non-obsolete */
  /* non-deleted formulae are removed even if they are fresh or fortified */
  /* (this should not be used during the automatic garbage collection!) */

  if (purge) {
    for (v=1; v<biddyVariableTable.num; v++) {
      biddyVariableTable.table[v].lastNode->list = NULL;
      sup = biddyVariableTable.table[v].firstNode;
      while (sup) {
        sup->expiry = 1;
        sup = (BiddyNode *) sup->list;
      }
    }
  }

  /* RESTORE expiry VALUE FOR TOP NODE OF ALL CONSTANTS, VARIABLES, ELEMENTS, AND EXTERNAL FORMULAE */
  /* THIS IS NEEDED EVEN IF PURGE == FALSE (E.G. SIFTING) */
  /* first formula is 0, it is always a single terminal node */
  /* second formula is 1, it could be a large graph, e.g. for ZBDD */
  BiddyRefresh(biddyZero);
  BiddyRefresh(biddyOne);
  for (v = 1; v < biddyVariableTable.num; v++) {
    BiddyRefresh(biddyVariableTable.table[v].variable);
    BiddyRefresh(biddyVariableTable.table[v].element);
  }
  for (j = 1; j < biddyFormulaTable.size; j++) {
    if (!Biddy_IsNull(biddyFormulaTable.table[j].f) &&
        !Biddy_IsConstant(biddyFormulaTable.table[j].f))
    {
      if ((biddyManagerType == BIDDYTYPEOBDDC) || (biddyManagerType == BIDDYTYPEOBDD) ||
          (biddyManagerType == BIDDYTYPEOFDDC) || (biddyManagerType == BIDDYTYPEOFDD))
      {
        if ((BiddyE(biddyFormulaTable.table[j].f) == biddyZero) && (BiddyT(biddyFormulaTable.table[j].f) == biddyTerminal)) {
          /* variable/element already refreshed */
        } else {
          BiddyProlongOne(biddyFormulaTable.table[j].f,biddyFormulaTable.table[j].expiry);
        }
      }
      if ((biddyManagerType == BIDDYTYPEZBDDC) || (biddyManagerType == BIDDYTYPEZBDD) ||
          (biddyManagerType == BIDDYTYPEZFDDC) || (biddyManagerType == BIDDYTYPEZFDD))
      {
        if ((BiddyE(biddyFormulaTable.table[j].f) == biddyTerminal) && (BiddyT(biddyFormulaTable.table[j].f) == biddyTerminal)) {
          /* variable/element already refreshed */
        } else {
          BiddyProlongOne(biddyFormulaTable.table[j].f,biddyFormulaTable.table[j].expiry);
        }
      }
      if ((biddyManagerType == BIDDYTYPETZBDDC) || (biddyManagerType == BIDDYTYPETZBDD) ||
          (biddyManagerType == BIDDYTYPETZFDDC) || (biddyManagerType == BIDDYTYPETZFDD))
      {
        if ((BiddyE(biddyFormulaTable.table[j].f) == biddyZero) && (BiddyT(biddyFormulaTable.table[j].f) == biddyTerminal)) {
          /* variable/element already refreshed */
        } else {
          BiddyProlongOne(biddyFormulaTable.table[j].f,biddyFormulaTable.table[j].expiry);
        }
      }
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
    /* if target != 0 then nodes equal or higher than v will not be prolonged */
    if (target == v) {
      /* there is always at least one node in the list */
      sup = biddyVariableTable.table[v].firstNode;
      assert( sup != NULL );
      while (sup) {
        assert(sup->v == v) ;
        if ((sup->expiry) && (sup->expiry < biddySystemAge)) {
          (biddyVariableTable.table[v].numobsolete)++;
        }
        sup = (BiddyNode *) sup->list;
      }
    }
    else if ((target == 0) || BiddyIsSmaller(v,target)) {
      /* there is always at least one node in the list */
      sup = biddyVariableTable.table[v].firstNode;
      assert( sup != NULL );
      assert( sup->v == v );
      /* successors of the first node in the list does not need to be prolonged */
      sup = (BiddyNode *) sup->list;
      while (sup) {
        assert( sup->v == v );
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

  /* DEBUGGING */
  /*
  printf("GC CHECK: %u obsolete nodes ",biddyNodeTable.num - i);
  if (gcUseful) printf("(GC useful) "); else printf("(GC NOT useful) ");
  if (resizeRequired) printf("(Resize required)"); else printf("Resize NOT required)");
  printf("\n");
  */

  /* REMOVE ALL OBSOLETE NODES */
  /* RESIZE NODE TABLE IF IT SEEMS TO BE TO SMALL */
  cacheOK = TRUE;
  if (gcUseful || resizeRequired) {

#ifdef BIDDYEXTENDEDSTATS_YES
    biddyNodeTable.gcobsolete[biddyNodeTable.garbage-1]  = (biddyNodeTable.num - i);
#endif

    if (resizeRequired) {

      /* DEBUGGING */
      /*
      printf("Biddy_Managed_GC: resizeRequired == TRUE\n");
      */

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
#pragma warning(suppress: 4244)
      biddyNodeTable.gcratio = (1.0 - biddyNodeTable.gcratioX) + (biddyNodeTable.gcratio / biddyNodeTable.gcratioF);
#pragma warning(suppress: 4244)
      biddyNodeTable.resizeratio = (1.0 - biddyNodeTable.resizeratioX) + (biddyNodeTable.resizeratio * biddyNodeTable.resizeratioF);
      if (biddyNodeTable.gcratio < 0.0) biddyNodeTable.gcratio = 0.0;
      if (biddyNodeTable.resizeratio < 0.0) biddyNodeTable.resizeratio = 0.0;

      /* PROFILING */
      /*
      printf("(size=%u)(gcr=%.2f)(rr=%.2f)",biddyNodeTable.size,biddyNodeTable.gcratio,biddyNodeTable.resizeratio);
      */
    }
    
    /* PROFILING */
    /*
    else {
      printf("+");
    }
    */

    /* BECAUSE OF REHASHING (NODES ARE STORED IN NODE TABLE IN 'ORDERED' LISTS) */
    /* IT IS REQUIRED TO VISIT NODES STARTING WITH THE LAST VARIABLE! */
    /* LAST VARIABLE IS NOT THE SAME AS THE SMALLEST/LARGEST IN THE VARIABLE ORDERING! */
    for (v=biddyVariableTable.num-1; v>0; v--) {

      /* DEBUGGING */
      /*
      printf("GC VARIABLE %u (NODES: %u, OBSOLETE NODES: %u)\n",v,biddyVariableTable.table[v].num,biddyVariableTable.table[v].numobsolete);
      */

      /* FOR EVER VARIABLE, ALL NODES EXCEPT THE FIRST ONE MUST BE VISITED */
      if (resizeRequired) {
        biddyVariableTable.table[v].numobsolete = biddyVariableTable.table[v].num - 1;
      }

      /* BE CAREFULLY WITH TYPE BiddyNode !!!! */
      /* FIELDS 'prev' AND 'next' HAVE TO BE THE FIRST AND THE SECOND! */
      /* THIS COMPUTATION USE SOME TRICKS! */

      if (biddyVariableTable.table[v].numobsolete) {

        /* UPDATING NODE TABLE */
        /* there is always at least one node in the list and the first node is never obsolete */
        tmpnode1 = biddyVariableTable.table[v].firstNode;
        assert( BiddyIsOK(tmpnode1) );

        biddyVariableTable.table[v].lastNode->list = NULL;

        /* the first node in the list is variable/element */
        /* and because it is not stored in Node table it does not need rehashing */
        tmpnode2 = (BiddyNode *) tmpnode1->list;

        while (biddyVariableTable.table[v].numobsolete) {

          /* DEBUGGING */
          /*
          printf("  GC WHILE OBSOLETE NODES = %u\n",biddyVariableTable.table[v].numobsolete);
          */

          assert( tmpnode2 != NULL) ;
          assert( tmpnode2->v == v ) ;

          if (!(tmpnode2->expiry) || (tmpnode2->expiry >= biddySystemAge)) {
            /* fortified, fresh or prolonged  node */
            tmpnode1 = tmpnode2;

            /* if resize is required then this node must be rehashed and stored into new Node table */
            if (resizeRequired) {
              hash = nodeTableHash(v,tmpnode2->f,tmpnode2->t,biddyNodeTable.size);
              addNodeTable(MNG,hash,tmpnode2,NULL); /* add node to hash table */
              (biddyVariableTable.table[v].numobsolete)--; /* if resize is required then all nodes are counted with this counter */
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

          tmpnode2 = (BiddyNode *) tmpnode1->list;
        }
        if (!tmpnode1->list) {
          biddyVariableTable.table[v].lastNode = tmpnode1;
        }
      }

    }
  }
  
  /* DEBUGGING */
  /*
  else {
    printf("#");
  }
  */

  /* Updating cache tables - only if needed */
  if (!cacheOK) {
    c = biddyCacheList;
    while (c) {
      c->gc(MNG);
      c = c->next;
    }
  }

  biddyNodeTable.gctime += clock() - starttime;
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
    Field deleted is not considered and thus no fortified node and no
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
  Biddy_Variable v;

  if (!MNG) MNG = biddyAnonymousManager;

  /* DISCARD ALL NODES WHICH ARE NOT PRESERVED */
  /* implemented by simple increasing of biddySystemAge */
  BiddyIncSystemAge(MNG);

  /* to ensure that parameters in external functions are never obsolete, */
  /* constants must be explicitly refreshed, here */
  BiddyRefresh(biddyZero);
  BiddyRefresh(biddyOne);

  /* to ensure that parameters in external functions are never obsolete, */
  /* all variables and elements must be explicitly refreshed, here */
  for (v = 1; v < biddyVariableTable.num; v++) {
    BiddyRefresh(biddyVariableTable.table[v].variable);
    BiddyRefresh(biddyVariableTable.table[v].element);
  }

}

#ifdef __cplusplus
}
#endif

/***************************************************************************//*!
\brief Function Biddy_Managed_Purge immediately removes all nodes which were
       not preserved or which are not preserved anymore.

### Description
    All fresh and obsolete nodes are immediatelly removed.
    All formulae without name are deleted.
    Moreover, nodes from deleted prolonged formulae and nodes from deleted
    fortified formulae are removed if they are not needed by other formulae.
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
  Biddy_Variable v;

  if (!MNG) MNG = biddyAnonymousManager;

  BiddyIncSystemAge(MNG);
  Biddy_Managed_GC(MNG,0,TRUE,TRUE); /* purge = TRUE, total = TRUE */
  BiddyDecSystemAge(MNG);

  /* constants must be explicitly repaired, here */
  BiddyDefresh(biddyZero);
  BiddyDefresh(biddyOne);

  /* all variables and elements must be explicitly repaired, here */
  for (v = 1; v < biddyVariableTable.num; v++) {
    BiddyDefresh(biddyVariableTable.table[v].variable);
    BiddyDefresh(biddyVariableTable.table[v].element);
  }
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
    If (x != NULL) then formula is accessible by its name.
    If (c == -1) then formula is refreshed but not preserved.
    If (c == 0) then formula is persistently preserved and you have to use
    Biddy_DeleteFormula to remove its nodes.
    There are two macros defined to simplify formulae management.
    Macro Biddy_Managed_AddTmpFormula(mng,bdd,c) is
    defined as Biddy_Managed_AddFormula(mng,NULL,bdd,c) and macro
    Biddy_Managed_AddPersistentFormula(mng,name,bdd) is defined as
    Biddy_Managed_AddFormula(mng,name,bdd,0).
### Side effects
    Function is prolonged or fortified. Formulae with name are ordered by name.
    If formula with the same name already exists, it will be overwritten
    (preserved and persistently preserved formulae, too!).
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
  unsigned int i;
  Biddy_Boolean OK;
  BiddyFormula *tmp;
  Biddy_Edge old;

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

  /* if formula does not have a name and is NULL or constant, then it will not be added */
  if (!x && (Biddy_IsNull(f) || Biddy_IsConstant(f))) {
    return 0;
  }

  /* if formula does not have a name and is a basic variable/element, then it will not be changed */
  if (!x && !Biddy_IsNull(f)) {
    if ((biddyManagerType == BIDDYTYPEOBDDC) || (biddyManagerType == BIDDYTYPEOBDD) ||
        (biddyManagerType == BIDDYTYPEOFDDC) || (biddyManagerType == BIDDYTYPEOFDD))
    {
      if ((BiddyE(f) == biddyZero) && (BiddyT(f) == biddyTerminal)) {
        return 0;
      }
    }
    if ((biddyManagerType == BIDDYTYPEZBDDC) || (biddyManagerType == BIDDYTYPEZBDD) ||
        (biddyManagerType == BIDDYTYPEZFDDC) || (biddyManagerType == BIDDYTYPEZFDD))
    {
      if ((BiddyE(f) == biddyTerminal) && (BiddyT(f) == biddyTerminal)) {
        return 0;
      }
    }
    if ((biddyManagerType == BIDDYTYPETZBDDC) || (biddyManagerType == BIDDYTYPETZBDD) ||
        (biddyManagerType == BIDDYTYPETZFDDC) || (biddyManagerType == BIDDYTYPETZFDD))
    {
      if ((BiddyE(f) == biddyZero) && (BiddyT(f) == biddyTerminal)) {
        return 0;
      }
    }
  }

  /* SEARCH IN THE FORMULA TABLE */
  if (!x) {
    OK = FALSE;
    i = biddyFormulaTable.size; /* tmp formulae are added on the end of the table */
  } else {
    OK = Biddy_Managed_FindFormula(MNG,x,&i,&old);
  }

  /* the first two formulae ("0" and "1") must not be changed */
  if (OK && (i<2))
  {
    printf("WARNING (Biddy_Managed_AddFormula): formula %s not added because it has the same name as an existing constant/variable!\n",x);
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
      if ((biddyManagerType == BIDDYTYPEOBDDC) || (biddyManagerType == BIDDYTYPEOBDD) ||
          (biddyManagerType == BIDDYTYPEOFDDC) || (biddyManagerType == BIDDYTYPEOFDD))
      {
        if ((BiddyE(f) == biddyZero) && (BiddyT(f) == biddyTerminal)) {
          /* variable/element not changed */
        } else {
          BiddyProlongOne(f,(unsigned int) c);
        }
      }
      if ((biddyManagerType == BIDDYTYPEZBDDC) || (biddyManagerType == BIDDYTYPEZBDD) ||
          (biddyManagerType == BIDDYTYPEZFDDC) || (biddyManagerType == BIDDYTYPEZFDD))
      {
        if ((BiddyE(f) == biddyTerminal) && (BiddyT(f) == biddyTerminal)) {
          /* variable/element not changed */
        } else {
          BiddyProlongOne(f,(unsigned int) c);
        }
      }
      if ((biddyManagerType == BIDDYTYPETZBDDC) || (biddyManagerType == BIDDYTYPETZBDD) ||
          (biddyManagerType == BIDDYTYPETZFDDC) || (biddyManagerType == BIDDYTYPETZFDD))
      {
        if ((BiddyE(f) == biddyZero) && (BiddyT(f) == biddyTerminal)) {
          /* variable/element not changed */
        } else {
          BiddyProlongOne(f,(unsigned int) c);
        }
      }
    }
    return i;
  }

  /* formula must be added, first two formulae must be unchanged */
  if (i < 2) {
    fprintf(stderr,"Biddy_Managed_AddFormula: Name of the formula must be after \"1\"!\n");
    exit(1);
  }

  biddyFormulaTable.size++;
  if (!(tmp = (BiddyFormula *)
     realloc(biddyFormulaTable.table,biddyFormulaTable.size*sizeof(BiddyFormula))))
  {
    fprintf(stderr,"Biddy_Managed_AddFormula: Out of memoy!\n");
    exit(1);
  }
  biddyFormulaTable.table = tmp;

  memmove(&biddyFormulaTable.table[i+1],&biddyFormulaTable.table[i],(biddyFormulaTable.size-i-1)*sizeof(BiddyFormula));
  /*
  for (j = biddyFormulaTable.size-1; j > i; j--) {
    biddyFormulaTable.table[j] = biddyFormulaTable.table[j-1];
  }
  */

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
    if ((biddyManagerType == BIDDYTYPEOBDDC) || (biddyManagerType == BIDDYTYPEOBDD) ||
        (biddyManagerType == BIDDYTYPEOFDDC) || (biddyManagerType == BIDDYTYPEOFDD))
    {
      if ((BiddyE(f) == biddyZero) && (BiddyT(f) == biddyTerminal)) {
        /* variable/element not changed */
      } else {
        BiddyProlongOne(f,(unsigned int) c);
      }
    }
    if ((biddyManagerType == BIDDYTYPEZBDDC) || (biddyManagerType == BIDDYTYPEZBDD) ||
        (biddyManagerType == BIDDYTYPEZFDDC) || (biddyManagerType == BIDDYTYPEZFDD))
    {
      if ((BiddyE(f) == biddyTerminal) && (BiddyT(f) == biddyTerminal)) {
        /* variable/element not changed */
      } else {
        BiddyProlongOne(f,(unsigned int) c);
      }
    }
    if ((biddyManagerType == BIDDYTYPETZBDDC) || (biddyManagerType == BIDDYTYPETZBDD) ||
        (biddyManagerType == BIDDYTYPETZFDDC) || (biddyManagerType == BIDDYTYPETZFDD))
    {
      if ((BiddyE(f) == biddyZero) && (BiddyT(f) == biddyTerminal)) {
        /* variable/element not changed */
      } else {
        BiddyProlongOne(f,(unsigned int) c);
      }
    }
  }

  /* DEBUGGING */
  /*
  printf("Biddy_Managed_AddFormula: i=%d/%d, x=%s, f=%p\n",i,biddyFormulaTable.size-1,x,f);
  BiddySystemReport(MNG);
  */

  return i;
}

#ifdef __cplusplus
}
#endif

/***************************************************************************//*!
\brief Function Biddy_Managed_FindFormula find formula in Formula table.

### Description
    Return TRUE/FALSE, index, and the formula.
    If formula is constant or variable then idx = 0 and f != biddyNull.
    If formula is not found then idx is a position where the formulae should
    exist and f == biddyNull.
### Side effects
### More info
    Macro Biddy_FindFormula(x,f) is defined for use with anonymous manager.
*******************************************************************************/

#ifdef __cplusplus
extern "C" {
#endif

Biddy_Boolean
Biddy_Managed_FindFormula(Biddy_Manager MNG, Biddy_String x,
                          unsigned int *idx, Biddy_Edge *f)
{
  unsigned int i,min,max;
  Biddy_Variable v;
  Biddy_Boolean OK;
  int cc;

  if (!MNG) MNG = biddyAnonymousManager;

  *idx = 0;
  *f = biddyNull;
  if (!x) return FALSE;

  assert ( biddyFormulaTable.table );

  if (!strcmp(x,biddyFormulaTable.table[0].name)) { /* [0] = biddyZero */
    *idx = 0;
    *f = biddyZero;
    return TRUE;
  }

  if (!strcmp(x,biddyFormulaTable.table[1].name)) { /* [1] = biddyOne */
    *idx = 1;
    *f = biddyOne;
    return TRUE;
  }

  /* VARIABLES ARE CONSIDERED TO BE FORMULAE, [0] = constant 1 */
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

  if (biddyFormulaTable.numOrdered == 2) {
    *idx = 2;
    return FALSE;
  }

  /* VARIANT A: LINEAR SEARCH */
  /*
  OK = FALSE;
  for (i = 2; !OK && (i < biddyFormulaTable.numOrdered); i++) {
    if (biddyFormulaTable.table[i].name) {
      cc = strcmp(x,biddyFormulaTable.table[i].name);
      if ((cc == 0) && !biddyFormulaTable.table[i].deleted) {
        OK = TRUE;
        break;
      }
      if (cc < 0) break;
    } else {
      fprintf(stderr,"Biddy_Managed_FindFormula: Problem with the formula table!\n");
      exit(1);
    }
  }
  j = i;
  */

  /* VARIANT B: BINARY SEARCH */
  min = 2;
  max = biddyFormulaTable.numOrdered - 1;

  /* DEBUGGING */
  /*
  fprintf(stderr,"search for %s, min = %u (%s), max = %u (%s)\n",x,min,biddyFormulaTable.table[min].name,max,biddyFormulaTable.table[max].name);
  */

  cc = strcmp(x,biddyFormulaTable.table[min].name);
  if ((cc == 0) && !biddyFormulaTable.table[min].deleted) {
    i = min;
    OK = TRUE;
  } else if (cc < 0) {
    i = min;
    OK = FALSE;
  } else {
    cc = strcmp(x,biddyFormulaTable.table[max].name);
    if ((cc == 0) && !biddyFormulaTable.table[max].deleted) {
      i = max;
      OK = TRUE;
    } else if (cc > 0) {
      i = max+1;
      OK = FALSE;
    } else {
      i = (min + max) / 2;
      while (i != min) {
        cc = strcmp(x,biddyFormulaTable.table[i].name);
        if ((cc == 0) && !biddyFormulaTable.table[i].deleted) {
          min = max = i;
          OK = TRUE;
        } else if (cc < 0) {
          max = i;
        } else {
          min = i;
        }
        i = (min + max) / 2;
      }
      if (!OK) i++;
    }
  }

  /* DEBUGGING */
  /*
  if (i != j) {
    fprintf(stderr,"ERROR, i=%u, j=%u\n",i,j);
    for (i = 2; !OK && (i < biddyFormulaTable.numOrdered); i++) {
      fprintf(stderr,"%i. %s\n",i,biddyFormulaTable.table[i].name);
    }
    exit(1);
  }
  */

  if (OK) {

    /* element found */
    *idx = i;
    *f = biddyFormulaTable.table[i].f;

    /* DEBUGGING */
    /*
    printf("\nBiddy_FindFormula: i=%d, x=%s, f=%p\n",i,x,*f);
    */

  } else {

    /* element not found */
    *idx = i;

    /* DEBUGGING */
    /*
    printf("\nBiddy_FindFormula: <%s> NOT FOUND!\n",x);
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
    for (i = 0; !OK && (i < biddyFormulaTable.numOrdered); i++) {
      if (biddyFormulaTable.table[i].name) {
        cc = strcmp(x,biddyFormulaTable.table[i].name);
        if ((cc == 0) && !biddyFormulaTable.table[i].deleted) {
          OK = TRUE;
          break;
        }
        if (cc < 0) break;
      } else {
        fprintf(stderr,"Biddy_Managed_DeleteFormula: Problem with the formula table!\n");
        exit(1);
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

  if (biddyManagerType == BIDDYTYPEOBDD) {
    /* IMPLEMENTED */
    vhigh = swapWithHigher(MNG,v,&active);
  } else if (biddyManagerType == BIDDYTYPEOBDDC) {
    /* IMPLEMENTED */
    vhigh = swapWithHigher(MNG,v,&active);
  } else if (biddyManagerType == BIDDYTYPEZBDDC) {
    /* IMPLEMENTED */
    vhigh = swapWithHigher(MNG,v,&active);
  } else if (biddyManagerType == BIDDYTYPETZBDD) {
    /* IMPLEMENTED */
    vhigh = swapWithHigher(MNG,v,&active);
  } else if ((biddyManagerType == BIDDYTYPEZBDD) ||
              (biddyManagerType == BIDDYTYPETZBDDC))
  {
    fprintf(stderr,"Biddy_SwapWithHigher: this GDD type is not supported, yet!\n");
    return 0;
  } else if ((biddyManagerType == BIDDYTYPEOFDDC) || (biddyManagerType == BIDDYTYPEOFDD) ||
              (biddyManagerType == BIDDYTYPEZFDDC) || (biddyManagerType == BIDDYTYPEZFDD) ||
              (biddyManagerType == BIDDYTYPETZFDDC) || (biddyManagerType == BIDDYTYPETZFDD))
  {
    fprintf(stderr,"Biddy_SwapWithHigher: this GDD type is not supported, yet!\n");
    return 0;
  } else {
    fprintf(stderr,"Biddy_SwapWithHigher: Unsupported GDD type!\n");
    return 0;
  }

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

  if (biddyManagerType == BIDDYTYPEOBDD) {
    /* IMPLEMENTED */
    vlow = swapWithLower(MNG,v,&active);
  } else if (biddyManagerType == BIDDYTYPEOBDDC) {
    /* IMPLEMENTED */
    vlow = swapWithLower(MNG,v,&active);
  } else if (biddyManagerType == BIDDYTYPEZBDDC) {
    /* IMPLEMENTED */
    vlow = swapWithLower(MNG,v,&active);
  } else if (biddyManagerType == BIDDYTYPETZBDD) {
    /* IMPLEMENTED */
    vlow = swapWithLower(MNG,v,&active);
  } else if ((biddyManagerType == BIDDYTYPEZBDD) ||
              (biddyManagerType == BIDDYTYPETZBDDC))
  {
    fprintf(stderr,"Biddy_SwapWithLower: this GDD type is not supported, yet!\n");
    return 0;
  } else if ((biddyManagerType == BIDDYTYPEOFDDC) || (biddyManagerType == BIDDYTYPEOFDD) ||
              (biddyManagerType == BIDDYTYPEZFDDC) || (biddyManagerType == BIDDYTYPEZFDD) ||
              (biddyManagerType == BIDDYTYPETZFDDC) || (biddyManagerType == BIDDYTYPETZFDD))
  {
    fprintf(stderr,"Biddy_SwapWithLower: this GDD type is not supported, yet!\n");
    return 0;
  } else {
    fprintf(stderr,"Biddy_SwapWithLower: Unsupported GDD type!\n");
    return 0;
  }

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
    In the current implementation for TZBDD, sifting may change top node of
    functions - this is a problem, because functions referenced by local
    variables may become wrong. Consequently, for the current implementation
    for TZBDDs, sifting is not safe to start automatically!
### More info
    Macro Biddy_Sifting(f) is defined for use with anonymous manager.
*******************************************************************************/

#ifdef __cplusplus
extern "C" {
#endif

Biddy_Boolean
Biddy_Managed_Sifting(Biddy_Manager MNG, Biddy_Edge f, Biddy_Boolean converge)
{
  unsigned int num,min,min2,vnum,vv,totalbest;
  unsigned int best1,best2;
  Biddy_Variable v,vmax,k,minvar,maxvar,varOrder,n,minvarOrder,maxvarOrder;
  Biddy_Boolean highfirst,stop,finish,active;
  BiddyOrderingTable *fOrdering;
  unsigned int varTable[BIDDYVARMAX];
  float treshold;
  clock_t starttime;

  assert( (f == NULL) || (BiddyIsOK(f) == TRUE) );

  /* biddySiftingActive = TRUE; */

  if (!MNG) MNG = biddyAnonymousManager;
  ZF_LOGI("Biddy_Sifting");

  if (biddyManagerType == BIDDYTYPEOBDD) {
    /* IMPLEMENTED */
  } else if (biddyManagerType == BIDDYTYPEOBDDC) {
    /* IMPLEMENTED */
  } else if (biddyManagerType == BIDDYTYPEZBDDC) {
    /* IMPLEMENTED */
    if (f) return FALSE; /* is sifting on function correct ?? */
  } else if (biddyManagerType == BIDDYTYPETZBDD) {
    /* IMPLEMENTED */
    if (f) return FALSE; /* is sifting on function correct ?? */
  } else if ((biddyManagerType == BIDDYTYPEZBDD) ||
              (biddyManagerType == BIDDYTYPETZBDDC))
  {
    fprintf(stderr,"Biddy_Sifting: this GDD type is not supported, yet!\n");
    return FALSE;
  } else if ((biddyManagerType == BIDDYTYPEOFDDC) || (biddyManagerType == BIDDYTYPEOFDD) ||
              (biddyManagerType == BIDDYTYPEZFDDC) || (biddyManagerType == BIDDYTYPEZFDD) ||
              (biddyManagerType == BIDDYTYPETZFDDC) || (biddyManagerType == BIDDYTYPETZFDD))
  {
    fprintf(stderr,"Biddy_Sifting: this GDD type is not supported, yet!\n");
    return FALSE;
  } else {
    fprintf(stderr,"Biddy_Sifting: Unsupported GDD type!\n");
    return FALSE;
  }

  starttime = clock();

  if (f && (((BiddyNode *) BiddyP(f))->expiry == biddySystemAge)) {
    fprintf(stdout,"ERROR (Biddy_Managed_Sifting): Sifting on fresh function is wrong!\n");
    return FALSE;
  }

  if (!f) {

    /* MINIMIZING NODE NUMBER FOR THE WHOLE SYSTEM */

    /* TO GET THE PRECISE NUMBER OF NODES IN EVERY STEP */
    /* IT IS IMPORTANT, THAT WE REMOVE ALL OBSOLETE FORMULAE AND NODES */
    /* AND ALSO ALL NODES WHICH ARE NOT PART OF ANY REMAINING FORMULA */
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
    /* however, a rigorous GC seems to be a good idea before sifting on function */
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

    if ((biddyManagerType == BIDDYTYPEZBDDC) || (biddyManagerType == BIDDYTYPEZBDD) ||
        (biddyManagerType == BIDDYTYPETZBDDC) || (biddyManagerType == BIDDYTYPETZBDD))
    {
      fOrdering =  &biddyOrderingTable;
    }

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
  /* TO DO: user cache tables are not considered, yet */
  OPGarbageDeleteAll(MNG);
  EAGarbageDeleteAll(MNG);
  RCGarbageDeleteAll(MNG);
  ReplaceGarbageDeleteAll(MNG);
  biddyOPCache.disabled = TRUE;
  biddyEACache.disabled = TRUE;
  biddyRCCache.disabled = TRUE;
  biddyReplaceCache.disabled = TRUE;

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
  printf("========================================\n");
  printf("SIFTING START: num sifting: %u\n",Biddy_Managed_NodeTableSiftingNumber(MNG));
  printf("SIFTING START: num swapping: %u\n",Biddy_Managed_NodeTableSwapNumber(MNG));
  printf("SIFTING START: num garbage collection: %u\n",Biddy_Managed_NodeTableGCNumber(MNG));
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
    printf("Initial number of dependent variables: %u\n",Biddy_Managed_DependentVariableNumber(MNG,f));
    printf("Initial number of minterms: %.0f\n",Biddy_Managed_CountMinterm(MNG,f,0));
    printf("Initial number of nodes: %u\n",Biddy_Managed_NodeNumber(MNG,f));
    printf("Top edge: %p\n",(void *) f);
    printf("Top variable: %s\n",Biddy_Managed_GetTopVariableName(MNG,f));
    printf("Expiry value of top node: %d\n",((BiddyNode *) BiddyP(f))->expiry);
    printf("minvar: %s (id=%u, order=%u), maxvar: %s (id=%u, order=%u)\n",
            Biddy_Managed_GetVariableName(MNG,minvar),minvar,minvarOrder,
            Biddy_Managed_GetVariableName(MNG,maxvar),maxvar,maxvarOrder);
  }
  printf("SIFTING: initial order\n");
  writeORDER(MNG);
  BiddySystemReport(MNG);
  for (n = 1; n < biddyFormulaTable.size; n++) {
    if (!Biddy_IsNull(biddyFormulaTable.table[n].f) &&
        !Biddy_IsConstant(biddyFormulaTable.table[n].f))
    {
      assert( BiddyIsOK(biddyFormulaTable.table[n].f) );
    }
  }
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

      /* VARIANT A:  start with shorter direction, thus calculate v's global ordering */
      /**/
      n = Biddy_Managed_VariableTableNum(MNG);
      for (k=0; k<Biddy_Managed_VariableTableNum(MNG); k++) {
        if (GET_ORDER(biddyOrderingTable,v,k)) n--;
      }
      if (n > (minvarOrder+maxvarOrder)/2) highfirst = TRUE; else highfirst = FALSE;
      /**/

      /* VARIANT B:  start with highfirst = TRUE */
      /*
      highfirst = TRUE;
      */
    }

    /* PROFILING */
    /*
    varOrder = Biddy_Managed_VariableTableNum(MNG);
    for (k=0; k<Biddy_Managed_VariableTableNum(MNG); k++) {
      if (GET_ORDER(biddyOrderingTable,v,k)) varOrder--;
    }
    printf("SIFTING START: variable: \"%s\" (v=%u), startOrder=%u, min=%u, numNodes=%u, systemAge=%u, ",
           Biddy_Managed_GetVariableName(MNG,v),v,varOrder,min,Biddy_Managed_NodeTableNum(MNG),biddySystemAge);
    if (highfirst) printf("HIGHFIRST\n"); else printf("LOWFIRST\n");
    */

    /* DEBUGGING */
    /*
    for (k=0; k<Biddy_Managed_VariableTableNum(MNG); k++) {
      printf("[\"%s\"@%u]",biddyVariableTable.table[k].name,biddyVariableTable.table[k].num);
    }
    printf("\n");
    */

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

      n++; /* number of steps performed*/

      /* DEBUGGING */
      /*
      printf("SIFTING: ");
      if (highfirst) {
        printf("%u swaped with higher %u",v,k);
      } else {
        printf("%u swaped with lower %u",v,k);
      }
      if (active) printf(" (ACTIVE)");
      printf("\n");
      */

      /* IF v AND k HAVE BEEN SWAPPED */
      if (active) {

        if (!f) {

          /* DETERMINE THE NUMBER OF USEFUL NODES */
          if ((biddyManagerType == BIDDYTYPEOBDDC) || (biddyManagerType == BIDDYTYPEOBDD)) {
            /* Biddy_Managed_GC(MNG,0,FALSE,TRUE); */
            Biddy_Managed_GC(MNG,BiddyIsSmaller(k,v)?v:k,FALSE,TRUE);
            num = Biddy_Managed_NodeTableNum(MNG);
          }
          else if ((biddyManagerType == BIDDYTYPEZBDDC) || (biddyManagerType == BIDDYTYPEZBDD)) {
            /* Biddy_Managed_GC(MNG,0,FALSE,TRUE); */
            Biddy_Managed_GC(MNG,BiddyIsSmaller(k,v)?v:k,FALSE,TRUE);
            num = Biddy_Managed_NodeTableNum(MNG);
          }
          else if ((biddyManagerType == BIDDYTYPETZBDDC) || (biddyManagerType == BIDDYTYPETZBDD)) {
            Biddy_Managed_GC(MNG,0,FALSE,TRUE);
            /* Biddy_Managed_GC(MNG,BiddyIsSmaller(k,v)?biddyVariableTable.table[v].next:biddyVariableTable.table[k].next,FALSE,TRUE); */
            num = Biddy_Managed_NodeTableNum(MNG);
          }
          else {
            Biddy_Managed_GC(MNG,0,FALSE,TRUE);
            num = Biddy_Managed_NodeTableNum(MNG);
          }

          /* THIS IS NECESSARY IF NOT ALL OBSOLETE NODES ARE REMOVED BY GC */
          /* if last parameter == TRUE then all obsolete nodes are removed */
          /*
          for (vmax=1; vmax<biddyVariableTable.num; vmax++) {
            num = num - biddyVariableTable.table[vmax].numobsolete;
          }
          */

        } else {

          /* update node number and variable ordering for the specific function */
          /* both is neccessary for the algorithm */
          /* *fOrdering is the same ordering as biddyOrderingTable, but it */
          /* also contains information about which variables are neighbours */
          /* garbage collection is not neccessary here! */
          for (vmax=1; vmax<Biddy_Managed_VariableTableNum(MNG); vmax++) {
            biddyVariableTable.table[vmax].numone = 0;
          }
          nullOrdering(*fOrdering);
          num = 1;
          nodeNumberOrdering(MNG,f,&num,*fOrdering);
          Biddy_Managed_DeselectAll(MNG);

          if ((biddyManagerType == BIDDYTYPEZBDDC) || (biddyManagerType == BIDDYTYPEZBDD) ||
              (biddyManagerType == BIDDYTYPETZBDDC) || (biddyManagerType == BIDDYTYPETZBDD))
          {
            fOrdering =  &biddyOrderingTable;
          }

        }

        /* DEBUGGING */
        /*
        printf("SIFTING: first direction, step=%u, min=%u, num=%u\n",n,min,num);
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

        /* THIS IS EXPERIMENTAL !!! */
        /* IN THE FIRST DIRECTION, PERFORM LIMIT NUMBER OF STEPS */
        /* THIS COULD RESULT IN SHORT TIME AND NOT VERY BAD SIZE */
        /*
        if (n >= 1) {
          stop = TRUE;
        }
        */

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
     
    /* IF (!f) THEN NO OBSOLETE NODES EXIST IN THE SYSTEM */
    /* BECAUSE GC WAS CALLED AFTER EVERY SWAPPING */

    /* PROFILING */
    /*
    varOrder = Biddy_Managed_VariableTableNum(MNG);
    for (k=0; k<Biddy_Managed_VariableTableNum(MNG); k++) {
      if (GET_ORDER(biddyOrderingTable,v,k)) varOrder--;
    }
    printf("SIFTING BEFORE GO-BACK: variable: \"%s\" (v=%u), currentOrder=%u, min=%u, numNodes=%u\n",
           Biddy_Managed_GetVariableName(MNG,v),v,varOrder,min,Biddy_Managed_NodeTableNum(MNG));
    */

    /* DEBUGGING */
    /*
    for (k=0; k<Biddy_Managed_VariableTableNum(MNG); k++) {
      printf("[\"%s\"@%u]",biddyVariableTable.table[k].name,biddyVariableTable.table[k].num);
    }
    printf("\n");
    */

    /* DEBUGGING */
    /*
    printf("SIFTING: GO-BACK!\n");
    */

    /* GO BACK */
    while (n) {
      if (highfirst) {
        k = swapWithLower(MNG,v,&active);

        if ((biddyManagerType == BIDDYTYPETZBDDC) || (biddyManagerType == BIDDYTYPETZBDD)) {
          Biddy_Managed_GC(MNG,0,FALSE,TRUE);
        }

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

        if ((biddyManagerType == BIDDYTYPETZBDDC) || (biddyManagerType == BIDDYTYPETZBDD)) {
          Biddy_Managed_GC(MNG,0,FALSE,TRUE);
        }

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

    /* IF GO-BACK WAS MOVING VARIABLE UP (highfirst == TRUE) */
    /* OBDD, ZBDD: ONLY OBSOLETE NODES WITH var == v EXIST IN THE SYSTEM (NO GC WAS CALLED FOR GO-BACK) */
    /* TZBDD: OBSOLETE NODES WITH var >= v EXIST IN THE SYSTEM (NO GC WAS CALLED FOR GO-BACK) */

    /* IF GO-BACK WAS MOVING VARIABLE DOWN (highfirst == FALSE) */
    /* OBDD, ZBDD: OBSOLETE NODES WITH var < v EXIST IN THE SYSTEM (NO GC WAS CALLED FOR GO-BACK) */
    /* TZBDD: OBSOLETE NODES WITH var < v EXIST IN THE SYSTEM (NO GC WAS CALLED FOR GO-BACK) */

    if (!f) {
      /* this garbage collection is required here to remove all obsolete nodes */
      /* next GC call may use parameter target and may not remove old obsolete nodes! */
      if ((biddyManagerType == BIDDYTYPEOBDDC) || (biddyManagerType == BIDDYTYPEOBDD)) {
        Biddy_Managed_GC(MNG,v,FALSE,TRUE);
      }
      else if ((biddyManagerType == BIDDYTYPEZBDDC) || (biddyManagerType == BIDDYTYPEZBDD)) {
        Biddy_Managed_GC(MNG,v,FALSE,TRUE);
      }
      else if ((biddyManagerType == BIDDYTYPETZBDDC) || (biddyManagerType == BIDDYTYPETZBDD)) {
        /* TO DO: this can be optimized in the case of moving down */
        Biddy_Managed_GC(MNG,0,FALSE,TRUE);
      }
      else {
        Biddy_Managed_GC(MNG,0,FALSE,TRUE);
      }

    }

    if (f) {
      /* garbage collection is not needed here */
      /* however, it seems to be a good idea */
      if ((biddyManagerType == BIDDYTYPEOBDDC) || (biddyManagerType == BIDDYTYPEOBDD)) {
        Biddy_Managed_GC(MNG,v,FALSE,TRUE);
      }
      else if ((biddyManagerType == BIDDYTYPEZBDDC) || (biddyManagerType == BIDDYTYPEZBDD)) {
        Biddy_Managed_GC(MNG,v,FALSE,TRUE);
      }
      else if ((biddyManagerType == BIDDYTYPETZBDDC) || (biddyManagerType == BIDDYTYPETZBDD)) {
        /* TO DO: this can be optimized in the case of moving down */
        Biddy_Managed_GC(MNG,0,FALSE,TRUE);
      }
      else {
        Biddy_Managed_GC(MNG,0,FALSE,TRUE);
      }
    }

    /* update variable ordering for the specific function */
    /* to update which variables are neighbours */
    if (f) {
      for (vmax=1; vmax<Biddy_Managed_VariableTableNum(MNG); vmax++) {
        biddyVariableTable.table[vmax].numone = 0;
      }
      nullOrdering(*fOrdering);
      num = 1;
      nodeNumberOrdering(MNG,f,&num,*fOrdering);
      Biddy_Managed_DeselectAll(MNG);

      if ((biddyManagerType == BIDDYTYPEZBDDC) || (biddyManagerType == BIDDYTYPEZBDD) ||
          (biddyManagerType == BIDDYTYPETZBDDC) || (biddyManagerType == BIDDYTYPETZBDD))
      {
        fOrdering =  &biddyOrderingTable;
      }
    }

    /* PROFILING */
    /*
    varOrder = Biddy_Managed_VariableTableNum(MNG);
    for (k=0; k<Biddy_Managed_VariableTableNum(MNG); k++) {
      if (GET_ORDER(biddyOrderingTable,v,k)) varOrder--;
    }
    printf("SIFTING AFTER FIRST DIRECTION: variable: \"%s\" (v=%u), currentOrder=%u, min=%u, numNodes=%u\n",
           Biddy_Managed_GetVariableName(MNG,v),v,varOrder,min,Biddy_Managed_NodeTableNum(MNG));
    */

    /* DEBUGGING */
    /*
    for (k=0; k<Biddy_Managed_VariableTableNum(MNG); k++) {
      printf("[\"%s\"@%u]",biddyVariableTable.table[k].name,biddyVariableTable.table[k].num);
    }
    printf("\n");
    */

    /* DEBUGGING */
    /*
    printf("SIFTING: SECOND DIRECTION\n");
    */

    /* OPPOSITE DIRECTION */

    n = 0;
    best2 = 0;
    stop = FALSE;
    min2 = Biddy_Managed_NodeTableNum(MNG); /* refresh node number after go-back */

    if (converge) {

      /* VARIANT C1: min from the first direction is not used within second direction */
      /* no code is needed in this variant */

      /* VARIANT C2: min from the first direction is used within second direction */
      /*
      min2 = min;
      */

      /* VARIANT C3: if there is min in the first direction do not check second direction */
      /*
      if (best1) {
        stop = TRUE;
      }
      */

    } else {

      /* VARIANT NC1: min from the first direction is not used within second direction */
      /* no code is needed in this variant */

      /* VARIANT NC2: min from the first direction is used within second direction */
      /*
      min2 = min;
      */

      /* VARIANT NC3: if there is min in the first direction do not check second direction */
      /* THIS SEEMS TO BE BAD FOR REGULAR SIFTING */

    }

    while (!stop && (highfirst ?
                      (k=swapWithLower(MNG,v,&active)) :
                      (k=swapWithHigher(MNG,v,&active))))
    {

      n++;

      /* DEBUGGING */
      /*
      printf("SIFTING: ");
      if (highfirst) {
        printf("%u swaped with lower %u",v,k);
      } else {
        printf("%u swaped with higher %u",v,k);
      }
      if (active) printf(" (ACTIVE)");
      printf("\n");
      */

      /* IF v AND k HAVE BEEN SWAPPED */
      if (active) {

        if (!f) {

          /* DETERMINE THE NUMBER OF USEFUL NODES */
          if ((biddyManagerType == BIDDYTYPEOBDDC) || (biddyManagerType == BIDDYTYPEOBDD)) {
            /* Biddy_Managed_GC(MNG,0,FALSE,TRUE); */
            Biddy_Managed_GC(MNG,BiddyIsSmaller(k,v)?v:k,FALSE,TRUE);
            num = Biddy_Managed_NodeTableNum(MNG);
          }
          else if ((biddyManagerType == BIDDYTYPEZBDDC) || (biddyManagerType == BIDDYTYPEZBDD)) {
            /* Biddy_Managed_GC(MNG,0,FALSE,TRUE); */
            Biddy_Managed_GC(MNG,BiddyIsSmaller(k,v)?v:k,FALSE,TRUE);
            num = Biddy_Managed_NodeTableNum(MNG);
          }
          else if ((biddyManagerType == BIDDYTYPETZBDDC) || (biddyManagerType == BIDDYTYPETZBDD)) {
            Biddy_Managed_GC(MNG,0,FALSE,TRUE);
            /* Biddy_Managed_GC(MNG,BiddyIsSmaller(k,v)?biddyVariableTable.table[v].next:biddyVariableTable.table[k].next,FALSE,TRUE); */
            num = Biddy_Managed_NodeTableNum(MNG);
          }
          else {
            Biddy_Managed_GC(MNG,0,FALSE,TRUE);
            num = Biddy_Managed_NodeTableNum(MNG);
          }

          /* THIS IS NECESSARY IF NOT ALL OBSOLETE NODES ARE REMOVED BY GC */
          /* if last paramter == TRUE then all obsolete nodes are removed */
          /*
          for (vmax=1; vmax<biddyVariableTable.num; vmax++) {
            num = num - biddyVariableTable.table[vmax].numobsolete;
          }
          */

        } else {

          /* update node number and variable ordering for the specific function */
          /* both is neccessary for the algorithm */
          /* *fOrdering is the same ordering as biddyOrderingTable, but it */
          /* also contains information about which variables are neighbours */
          /* garbage collection is not neccessary here! */
          for (vmax=1; vmax<Biddy_Managed_VariableTableNum(MNG); vmax++) {
            biddyVariableTable.table[vmax].numone = 0;
          }
          nullOrdering(*fOrdering);
          num = 1;
          nodeNumberOrdering(MNG,f,&num,*fOrdering);
          Biddy_Managed_DeselectAll(MNG);

          if ((biddyManagerType == BIDDYTYPEZBDDC) || (biddyManagerType == BIDDYTYPEZBDD) ||
              (biddyManagerType == BIDDYTYPETZBDDC) || (biddyManagerType == BIDDYTYPETZBDD))
          {
            fOrdering =  &biddyOrderingTable;
          }
        
        }

        /* DEBUGGING */
        /*
        printf("SIFTING: second direction, step=%u, min2=%u, num=%u\n",n,min2,num);
        */

        if (num <= min2) { /* second direction should use less or equal */
          min2 = num;
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

        if (num > (treshold * min2)) {
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

    if (min2 <= min) { /* second direction should use less or equal */
      min = min2;
    } else {
      best2 = 0;
    }

    /* IF best2==0 THEN MINIMUM WAS REACHED IN THE FIRST WHILE */
    /* YOU HAVE TO GO-BACK AND THEN MAKE ADDITIONAL best1 BACK STEPS */
    /* IF best2!=0 THEN MINIMUM WAS REACHED IN THE SECOND WHILE */
    /* YOU HAVE TO GO-BACK UNTIL n == best2 */

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

    /* IF SECOND GO-BACK WAS MOVING VARIABLE UP (highfirst == FALSE) */
    /* OBDD, ZBDD: ONLY OBSOLETE NODES WITH var == v EXIST IN THE SYSTEM (NO GC WAS CALLED FOR GO-BACK) */
    /* TZBDD: OBSOLETE NODES WITH var >= v EXIST IN THE SYSTEM (NO GC WAS CALLED FOR GO-BACK) */

    /* IF SECOND GO-BACK WAS MOVING VARIABLE DOWN (highfirst == TRUE) */
    /* OBDD, ZBDD: OBSOLETE NODES WITH var < v EXIST IN THE SYSTEM (NO GC WAS CALLED FOR GO-BACK) */
    /* TZBDD: OBSOLETE NODES WITH var < v EXIST IN THE SYSTEM (NO GC WAS CALLED FOR GO-BACK) */

    if (!f) {
      /* this garbage collection is required here to remove all obsolete nodes */
      /* next GC call may use parameter target and may not remove old obsolete nodes! */
      if ((biddyManagerType == BIDDYTYPEOBDDC) || (biddyManagerType == BIDDYTYPEOBDD)) {
        Biddy_Managed_GC(MNG,v,FALSE,TRUE);
      }
      else if ((biddyManagerType == BIDDYTYPEZBDDC) || (biddyManagerType == BIDDYTYPEZBDD)) {
        Biddy_Managed_GC(MNG,v,FALSE,TRUE);
      }
      else if ((biddyManagerType == BIDDYTYPETZBDDC) || (biddyManagerType == BIDDYTYPETZBDD)) {
        /* TO DO: this can be optimized in the case of moving down */
        Biddy_Managed_GC(MNG,0,FALSE,TRUE);
      }
      else {
        Biddy_Managed_GC(MNG,0,FALSE,TRUE);
      }
    }

    if (f) {
      /* garbage collection is not needed here */
      /* however, it seems to be a good idea */
      if ((biddyManagerType == BIDDYTYPEOBDDC) || (biddyManagerType == BIDDYTYPEOBDD)) {
        Biddy_Managed_GC(MNG,v,FALSE,TRUE);
      }
      else if ((biddyManagerType == BIDDYTYPEZBDDC) || (biddyManagerType == BIDDYTYPEZBDD)) {
        Biddy_Managed_GC(MNG,v,FALSE,TRUE);
      }
      else if ((biddyManagerType == BIDDYTYPETZBDDC) || (biddyManagerType == BIDDYTYPETZBDD)) {
        /* TO DO: this can be optimized in the case of moving down */
        Biddy_Managed_GC(MNG,0,FALSE,TRUE);
      }
      else {
        Biddy_Managed_GC(MNG,0,FALSE,TRUE);
      }
    }

    /* CHOOSE NEXT VARIABLE OR FINISH */
    /* IF f IS GIVEN THEN VARIABLES NOT IN f WILL BE SKIPPED */

    if (!f) {
      if ((biddyManagerType == BIDDYTYPEOBDDC) || (biddyManagerType == BIDDYTYPEOBDD)) {
        assert( min == Biddy_Managed_NodeTableNum(MNG) );
        num = min;
      }
      else if ((biddyManagerType == BIDDYTYPEZBDDC) || (biddyManagerType == BIDDYTYPEZBDD)) {
        assert( min == Biddy_Managed_NodeTableNum(MNG) );
        num = min;
      }
      else if ((biddyManagerType == BIDDYTYPETZBDDC) || (biddyManagerType == BIDDYTYPETZBDD)) {
        assert( min == Biddy_Managed_NodeTableNum(MNG) );
        num = min;
      }
      else {
        num = Biddy_Managed_NodeTableNum(MNG);
      }
      
    }

    /* IF f IS GIVEN you have to update variable ordering for the function */
    /* to update which variables are neighbours */
    if (f) {
      for (vmax=1; vmax<Biddy_Managed_VariableTableNum(MNG); vmax++) {
        biddyVariableTable.table[vmax].numone = 0;
      }
      nullOrdering(*fOrdering);
      num = 1;
      nodeNumberOrdering(MNG,f,&num,*fOrdering);
      Biddy_Managed_DeselectAll(MNG);

      if ((biddyManagerType == BIDDYTYPEZBDDC) || (biddyManagerType == BIDDYTYPEZBDD) ||
          (biddyManagerType == BIDDYTYPETZBDDC) || (biddyManagerType == BIDDYTYPETZBDD))
      {
        fOrdering =  &biddyOrderingTable;
      }
    }

    /* PROFILING */
    /*
    varOrder = Biddy_Managed_VariableTableNum(MNG);
    for (k=0; k<Biddy_Managed_VariableTableNum(MNG); k++) {
      if (GET_ORDER(biddyOrderingTable,v,k)) varOrder--;
    }
    printf("SIFTING FINAL: variable: \"%s\" (v=%u), finalOrder=%u, best1=%u, best2=%u, min=%u, num=%u, numNodes=%u\n",
           Biddy_Managed_GetVariableName(MNG,v),v,varOrder,best1,best2,min,num,Biddy_Managed_NodeTableNum(MNG));
    */

    /* DEBUGGING */
    /*
    for (k=0; k<Biddy_Managed_VariableTableNum(MNG); k++) {
      printf("[\"%s\"@%u]",biddyVariableTable.table[k].name,biddyVariableTable.table[k].num);
    }
    printf("\n");
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

    /* FOR NON-CONVERGING SIFTING, EVERY NEXT VARIABLE HAS A LITTLE BIT GREATER TRESHOLD */
    if (!f) {
      if (!converge) {
        if (treshold < 1.25 * biddyNodeTable.siftingtreshold) {
          treshold = treshold + 1 / (3.14 * Biddy_Managed_VariableTableNum(MNG));
        }
      }
    }

    /* PROFILING */
    /*
    printf("TRESHOLD = %.3f\n",treshold);
    */

  }  /* while (!finish) */

  /* THIS CODE DISABLES FURTHER CALLS IN THE CASE OF NOT-CONVERGING SIFTING */
  /* IN THE CASE OF CONVERGING SIFTING, TRESHOLD IS ADAPTED IN EVERY STEP */
  if (!converge) {
    totalbest = min;
  } else {
    if (!f) {
      treshold = treshold * biddyNodeTable.convergesiftingtreshold;
    } else {
      treshold = treshold * biddyNodeTable.fconvergesiftingtreshold;
    }
  }

  } /*   while (min < totalbest) */

  /* END OF SIFTING ALGORITHM */

  if (f) free(fOrdering);

  /* ENABLE ALL CACHE TABLES */
  /* TO DO: user caches are not considered, yet */
  biddyOPCache.disabled = FALSE;
  biddyEACache.disabled = FALSE;
  biddyRCCache.disabled = FALSE;
  biddyReplaceCache.disabled = FALSE;

  biddyNodeTable.drtime += clock() - starttime;

  /* DEBUGGING */
  /*
  printf("========================================\n");
  printf("SIFTING FINISH: num sifting: %u\n",Biddy_Managed_NodeTableSiftingNumber(MNG));
  printf("SIFTING FINISH: num swapping: %u\n",Biddy_Managed_NodeTableSwapNumber(MNG));
  printf("SIFTING FINISH: num garbage collection: %u\n",Biddy_Managed_NodeTableGCNumber(MNG));
  if (f) {
    minvarOrder = maxvarOrder = Biddy_Managed_VariableTableNum(MNG);
    for (k=0; k<Biddy_Managed_VariableTableNum(MNG); k++) {
      if (GET_ORDER(biddyOrderingTable,minvar,k)) minvarOrder--;
      if (GET_ORDER(biddyOrderingTable,maxvar,k)) maxvarOrder--;
    }
    printf("Final number of dependent variables: %u\n",Biddy_Managed_DependentVariableNumber(MNG,f));
    printf("Final number of minterms: %.0f\n",Biddy_Managed_CountMinterm(MNG,f,0));
    printf("Final number of nodes: %u\n",Biddy_Managed_NodeNumber(MNG,f));
    printf("Top edge: %p\n",(void *) f);
    printf("Top variable: %s\n",Biddy_Managed_GetTopVariableName(MNG,f));
    printf("Expiry value of top node: %d\n",((BiddyNode *) BiddyP(f))->expiry);
    printf("minvar: %s (id=%u, order=%u), maxvar: %s (id=%u, order=%u)\n",
            Biddy_Managed_GetVariableName(MNG,minvar),minvar,minvarOrder,
            Biddy_Managed_GetVariableName(MNG,maxvar),maxvar,maxvarOrder);
  }
  printf("SIFTING: final order\n");
  writeORDER(MNG);
  BiddySystemReport(MNG);
  for (n = 1; n < biddyFormulaTable.size; n++) {
    if (!Biddy_IsNull(biddyFormulaTable.table[n].f) &&
        !Biddy_IsConstant(biddyFormulaTable.table[n].f))
    {
      assert( BiddyIsOK(biddyFormulaTable.table[n].f) );
    }
  }
  printf("========================================\n");
  */

  /* biddySiftingActive = FALSE; */

  return TRUE; /* sifting has been performed */
}

#ifdef __cplusplus
}
#endif

/***************************************************************************//*!
\brief Function Biddy_Managed_Copy copies a graph from one manager to
       another manager which can use the same or different BDD type.
### Description
    The function takes a graph from one manager and creates the same graph
    in another manager. If the managers do not use the same BDD type then
    a graph is converted. The resulting graph will represent the same Boolean
    function assuming the domain from the target manager.
    If f = biddyZero then only the domain is copied.
### Side effects
    If source and target manager are the same then function does nothing.
    The variable ordering of the created BDD is trying to follow the original
    ordering, but if some variables already exist in the target manager then
    the final ordering is adapted to the target manager.
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
  Biddy_Variable top,prevvar;
  Biddy_String vname,topname;
  Biddy_Boolean simplecopy;

  if (!MNG1) MNG1 = biddyAnonymousManager;
  ZF_LOGI("Biddy_Copy");

  if (MNG1 == MNG2) {
    return f;
  }

  /* CHECK IF THE TARGET MNG IS EMPTY */
  simplecopy = FALSE;
  if (biddyVariableTable2.num == 1) {
    simplecopy = TRUE;
  }

  /* COPY THE DOMAIN */
  /* the ordering of the new variable is determined in Biddy_InitMNG */
  /* FOR OBDDs AND OFDDs: new variable is added below (bottommore) all others */
  /* FOR ZBDDs, ZFDDs, TZBDDs, AND TZFDDs: new variable is added above (topmore) all others */
  /* FINAL ORDER FOR OBDDs AND OFDDs: [1] < [2] < ... < [size-1] < [0] */
  /* FINAL ORDER FOR ZBDDs, ZFDDs, TZBDDs, AND TZFDDs: [size-1] < [size-2] < ... < [1] < [0] */
  if ((biddyManagerType2 == BIDDYTYPEOBDDC) || (biddyManagerType2 == BIDDYTYPEOBDD) ||
      (biddyManagerType2 == BIDDYTYPEOFDDC) || (biddyManagerType2 == BIDDYTYPEOFDD))
  {
    v = 0;
    for (k = 1; k < biddyVariableTable1.num; k++) {
      v = biddyVariableTable1.table[v].prev;
    }
    for (k = 1; k < biddyVariableTable1.num; k++) {
      vname = Biddy_Managed_GetVariableName(MNG1,v);
      r = Biddy_Managed_AddVariableByName(MNG2,vname);
      v = biddyVariableTable1.table[v].next;
    }
  }

  else if ((biddyManagerType2 == BIDDYTYPEZBDDC) || (biddyManagerType2 == BIDDYTYPEZBDD) ||
            (biddyManagerType2 == BIDDYTYPEZFDDC) || (biddyManagerType2 == BIDDYTYPEZFDD) ||
            (biddyManagerType2 == BIDDYTYPETZBDDC) || (biddyManagerType2 == BIDDYTYPETZBDD) ||
            (biddyManagerType2 == BIDDYTYPETZFDDC) || (biddyManagerType2 == BIDDYTYPETZFDD))
  {
    v = 0;
    for (k = 1; k < biddyVariableTable1.num; k++) {
      v = biddyVariableTable1.table[v].prev;
      vname = Biddy_Managed_GetVariableName(MNG1,v);
      r = Biddy_Managed_AddVariableByName(MNG2,vname);
    }
  }

  else {
    fprintf(stderr,"Biddy_Managed_Copy: Unsupported GDD type for MNG2!\n");
  }

  /* DEBUGGING - PRINT BOTH DOMAINS */
  /*
  printf("SOURCE DOMAIN\n");
  v = 0;
  for (k = 1; k < biddyVariableTable1.num; k++) {
    vname = Biddy_Managed_GetVariableName(MNG1,k);
    printf("(%u)%s,",k,vname);
    v = biddyVariableTable1.table[v].prev;
  }
  printf("\n");
  for (k = 1; k < biddyVariableTable1.num; k++) {
    vname = Biddy_Managed_GetVariableName(MNG1,v);
    printf("%s,",vname);
    v = biddyVariableTable1.table[v].next;
  }
  printf("\n");
  printf("TARGET DOMAIN\n");
  v = 0;
  for (k = 1; k < biddyVariableTable2.num; k++) {
    vname = Biddy_Managed_GetVariableName(MNG2,k);
    printf("(%u)%s,",k,vname);
    v = biddyVariableTable2.table[v].prev;
  }
  printf("\n");
  for (k = 1; k < biddyVariableTable2.num; k++) {
    vname = Biddy_Managed_GetVariableName(MNG2,v);
    printf("%s,",vname);
    v = biddyVariableTable2.table[v].next;
  }
  printf("\n");
  */

  if (f == biddyZero1) return biddyZero2; /* domain has been already copied */

  r = biddyNull;

  if (simplecopy) {

    if (biddyManagerType1 == biddyManagerType2) {

      /* MANAGERS ARE OF THE SAME TYPE AND THE TARGET MANAGER IS EMPTY */
      r = BiddyCopy(MNG1,MNG2,f);

    } else {

      /* MANAGERS ARE OF DIFFERENT TYPE BUT TARGET MNG IS EMPTY */
      /* WE USE DIRECT CONVERSION BECAUSE IT IS MORE EFFICIENT */
      r = BiddyConvertDirect(MNG1,MNG2,f);

      if (((biddyManagerType1 == BIDDYTYPEOBDDC) || (biddyManagerType1 == BIDDYTYPEOBDD))
          &&
          ((biddyManagerType2 == BIDDYTYPEOBDDC) || (biddyManagerType2 == BIDDYTYPEOBDD)))
      {
        /* RESULT IS ALREADY CORRECT */
      }

      else if (((biddyManagerType1 == BIDDYTYPEOBDDC) || (biddyManagerType1 == BIDDYTYPEOBDD))
                &&
                ((biddyManagerType2 == BIDDYTYPEZBDDC) || (biddyManagerType2 == BIDDYTYPEZBDD)))
      {
        /* BiddyConvertDirect does not create the topmost variables which are not present in OBDD */
        /* MNG2 was empty on the beginning and the domain have been correctly copied */
        /* thus, we can assume that MNG1 and MNG2 have the same ordering */
        /* NOTE: this implementation allows that MNG2 contains extra variables */        
        /* NOTE: this implementation allows that the same variable has different indices in MNG1 and MNG2 */

        /* determine the topmost variable in the domain of MNG2 (for ZBDDs, it has prev=biddyVariableTable.num) */
        top = BiddyV(r); /* not always the best start, but it is correct */
        while (biddyVariableTable2.table[top].prev != biddyVariableTable2.num) {
          top = biddyVariableTable2.table[top].prev;
        }
        /* determine topmost variable of f in MNG2 considered by BiddyConvertDirect (by checking the topmost variable of f in MNG1) */
        topname = Biddy_Managed_GetTopVariableName(MNG1,f);
        prevvar = Biddy_Managed_GetVariable(MNG2,topname);
        /* add all missing variables to f in MNG2 which are missing in MNG2 */
        while (prevvar != top) {
          prevvar = biddyVariableTable2.table[prevvar].prev;
          r = BiddyManagedTaggedFoaNode(MNG2,prevvar,r,r,0,TRUE);
          BiddyProlongOne(r,biddySystemAge2);
        }
      }

      else if (((biddyManagerType1 == BIDDYTYPEOBDDC) || (biddyManagerType1 == BIDDYTYPEOBDD))
                &&
                ((biddyManagerType2 == BIDDYTYPETZBDDC) || (biddyManagerType2 == BIDDYTYPETZBDD)))
      {
        /* RESULT IS ALREADY CORRECT */
      }

      else if (((biddyManagerType1 == BIDDYTYPEZBDDC) || (biddyManagerType1 == BIDDYTYPEZBDD))
                &&
                ((biddyManagerType2 == BIDDYTYPEZBDDC) || (biddyManagerType2 == BIDDYTYPEZBDD)))
      {
        /* IF MNG1 AND MNG2 CONTAINS EXACTLY THE SAME VARIABLES THEN THE RESULT IS ALREADY CORRECT */
        /* TO DO: */
        /* extend for the case where MNG2 contains extra variables and */
        /* the same variable may have different indices in MNG1 and MNG2 */
      }

      else if (((biddyManagerType1 == BIDDYTYPEZBDDC) || (biddyManagerType1 == BIDDYTYPEZBDD))
                &&
                ((biddyManagerType2 == BIDDYTYPEOBDDC) || (biddyManagerType2 == BIDDYTYPEOBDD)))
      {
        /* BiddyConvertDirect does not create the topmost variables which are not present in ZBDD */
        /* MNG2 was empty on the beginning and the domain have been correctly copied */
        /* thus, we can assume that MNG1 and MNG2 have the same ordering */
        /* NOTE: this implementation allows that MNG2 contains extra variables */        
        /* NOTE: this implementation allows that the same variable has different indices in MNG1 and MNG2 */

        /* determine the topmost variable in the domain of MNG1 (for ZBDDs, it has prev=biddyVariableTable.num) */
        top = BiddyV(f);
        while (biddyVariableTable1.table[top].prev != biddyVariableTable1.num) {
          top = biddyVariableTable1.table[top].prev;
        }
        /* determine topmost variable of f in MNG1 considered by BiddyConvertDirect (by checking the topmost variable of f in MNG1) */
        prevvar = BiddyV(f);
        /* add all variables to f in MNG2 which are missing in MNG1 */
        while (prevvar != top) {
          prevvar = biddyVariableTable1.table[prevvar].prev;
          topname = Biddy_Managed_GetVariableName(MNG1,prevvar);
          v = Biddy_Managed_GetVariable(MNG2,topname);
          r = BiddyManagedTaggedFoaNode(MNG2,v,r,biddyZero2,v,TRUE);
          BiddyProlongOne(r,biddySystemAge2);
        }

      }

      else if (((biddyManagerType1 == BIDDYTYPEZBDDC) || (biddyManagerType1 == BIDDYTYPEZBDD))
                &&
                ((biddyManagerType2 == BIDDYTYPETZBDDC) || (biddyManagerType2 == BIDDYTYPETZBDD)))
      {
        /* BiddyConvertDirect does not create the topmost variables which are not present in ZBDD */
        /* MNG2 was empty on the beginning and the domain have been correctly copied */
        /* thus, we can assume that MNG1 and MNG2 have the same ordering */
        /* this implementation is correct only if MNG1 and MNG2 contain exactly the same variables */        
        /* TO DO: */
        /* extend for the case where MNG2 contains extra variables and */
        /* the same variable may have different indices in MNG1 and MNG2 */

        /* determine the topmost variable in the domain of MNG1 (for ZBDDs, it has prev=biddyVariableTable.num) */
        top = BiddyV(f);
        while (biddyVariableTable1.table[top].prev != biddyVariableTable1.num) {
          top = biddyVariableTable1.table[top].prev;
        }
        /* add all variables to f in MNG2 which are missing in MNG1 */
        topname = Biddy_Managed_GetVariableName(MNG1,top);
        v = Biddy_Managed_GetVariable(MNG2,topname);
        Biddy_SetTag(r,v);
      }

      else if (((biddyManagerType1 == BIDDYTYPETZBDDC) || (biddyManagerType1 == BIDDYTYPETZBDD))
                &&
                ((biddyManagerType2 == BIDDYTYPEOBDDC) || (biddyManagerType2 == BIDDYTYPEOBDD)))
      {
        /* RESULT IS ALREADY CORRECT */
      }

      else if (((biddyManagerType1 == BIDDYTYPETZBDDC) || (biddyManagerType1 == BIDDYTYPETZBDD))
                &&
                ((biddyManagerType2 == BIDDYTYPEZBDDC) || (biddyManagerType2 == BIDDYTYPEZBDD)))
      {
        /* BiddyConvertDirect does not create the topmost variables which are not present in TZBDD */
        /* MNG2 was empty on the beginning and the domain have been correctly copied */
        /* thus, we can assume that MNG1 and MNG2 have the same ordering */
        /* NOTE: this implementation allows that MNG2 contains extra variables */        
        /* NOTE: this implementation allows that the same variable has different indices in MNG1 and MNG2 */

        /* determine the topmost variable in the domain of MNG2 (for ZBDDs, it has prev=biddyVariableTable.num) */
        top = BiddyV(r); /* not always the best start, but it is correct */
        while (biddyVariableTable2.table[top].prev != biddyVariableTable2.num) {
          top = biddyVariableTable2.table[top].prev;
        }
        /* determine topmost variable of f in MNG2 considered by BiddyConvertDirect (by checking the topmost tag of f in MNG1) */
        topname = Biddy_Managed_GetVariableName(MNG1,Biddy_GetTag(f));
        prevvar = Biddy_Managed_GetVariable(MNG2,topname);
        /* add all missing variables to f in MNG2 which are missing in MNG2 */
        while (prevvar != top) {
          prevvar = biddyVariableTable2.table[prevvar].prev;
          r = BiddyManagedTaggedFoaNode(MNG2,prevvar,r,r,0,TRUE);
          BiddyProlongOne(r,biddySystemAge2);
        }
      }

      else if (((biddyManagerType1 == BIDDYTYPETZBDDC) || (biddyManagerType1 == BIDDYTYPETZBDD))
                &&
                ((biddyManagerType2 == BIDDYTYPETZBDDC) || (biddyManagerType2 == BIDDYTYPETZBDD)))
      {
        /* IF MNG1 AND MNG2 CONTAINS EXACTLY THE SAME VARIABLES THEN THE RESULT IS ALREADY CORRECT */
        /* TO DO: */
        /* extend for the case where MNG2 contains extra variables and */
        /* the same variable may have different indices in MNG1 and MNG2 */
      }

      else {
        fprintf(stderr,"Biddy_Managed_Copy: Unsupported GDD type combination for MNG1 or MNG2!\n");
        assert ( FALSE );
      }

    }

  } else {

    if ((biddyManagerType1 == BIDDYTYPEOBDDC) || (biddyManagerType1 == BIDDYTYPEOBDD)) {
      r = BiddyCopyOBDD(MNG1,MNG2,f);
    }
    else if ((biddyManagerType1 == BIDDYTYPEZBDDC) || (biddyManagerType1 == BIDDYTYPEZBDD)) {
      r = BiddyCopyZBDD(MNG1,MNG2,f);
    }
    else if ((biddyManagerType1 == BIDDYTYPETZBDD) || (biddyManagerType1 == BIDDYTYPETZBDDC)) {
      r = BiddyCopyTZBDD(MNG1,MNG2,f);
    }
    else {
      fprintf(stderr,"Biddy_Managed_Copy: Unsupported GDD type for MNG1!\n");
    }

  }

  return r;
}

#ifdef __cplusplus
}
#endif

/***************************************************************************//*!
\brief Function Biddy_Managed_CopyFormula uses Biddy_Managed_Copy to copy a
       graph from one manager to another manager which can use the same or
       different BDD type.
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
  unsigned int i;
  Biddy_Boolean OK;
  Biddy_Edge src,dst;

  if (!x) return;

  if (!MNG1) MNG1 = biddyAnonymousManager;

  OK = Biddy_Managed_FindFormula(MNG1,x,&i,&src);

  if (OK) {
    OK = Biddy_Managed_FindFormula(MNG2,x,&i,&dst);
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
    Variables must have values assigned. Variable is considered to be
    FALSE iff variable.value == biddyZero, whilst it is considered to be TRUE,
    otherwise.
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
  Biddy_Variable fv,ftop;

  assert( f != NULL );

  if (!MNG) MNG = biddyAnonymousManager;

  if (f == biddyNull) return FALSE;

  ftop = 0;
  if ((biddyManagerType == BIDDYTYPEOBDDC) || (biddyManagerType == BIDDYTYPEOBDD)) {
    /* IMPLEMENTED */
    ftop = BiddyV(f);
  } else if ((biddyManagerType == BIDDYTYPEZBDDC) || (biddyManagerType == BIDDYTYPEZBDD)) {
    /* IMPLEMENTED */
    /* determine the topmost variable (for ZBDDs, it has prev=biddyVariableTable.num) */
    ftop = BiddyV(f);
    while (biddyVariableTable.table[ftop].prev != biddyVariableTable.num) {
      ftop = biddyVariableTable.table[ftop].prev;
    }
  } else if ((biddyManagerType == BIDDYTYPETZBDDC) || (biddyManagerType == BIDDYTYPETZBDD)) {
    /* IMPLEMENTED */
    ftop = Biddy_GetTag(f);
  } else if ((biddyManagerType == BIDDYTYPEOFDDC) || (biddyManagerType == BIDDYTYPEOFDD) ||
              (biddyManagerType == BIDDYTYPEZFDDC) || (biddyManagerType == BIDDYTYPEZFDD) ||
              (biddyManagerType == BIDDYTYPETZFDDC) || (biddyManagerType == BIDDYTYPETZFDD))
  {
    fprintf(stderr,"Biddy_Eval: this GDD type is not supported, yet!\n");
    return FALSE;
  } else {
    fprintf(stderr,"Biddy_Eval: Unsupported GDD type!\n");
    return FALSE;
  }

  r = !(Biddy_GetMark(f));
  while (ftop != 0) {

    fv = BiddyV(f);
    assert( biddyVariableTable.table[fv].value != NULL );

    if ((biddyManagerType == BIDDYTYPEOBDDC) || (biddyManagerType == BIDDYTYPEOBDD)) {

      if (biddyVariableTable.table[fv].value == biddyZero) {
        f = BiddyE(f);
        /* for OBDDC, 'else' successor can be marked */
        if (Biddy_GetMark(f)) r = !r;
      } else {
        f = BiddyT(f);
        /* for OBDD, biddyZero is a marked edge to 'then' successor! */
        if (Biddy_GetMark(f)) r = !r;
      }
      ftop = BiddyV(f);

    } else if ((biddyManagerType == BIDDYTYPEZBDDC) || (biddyManagerType == BIDDYTYPEZBDD)) {

      while (ftop != fv) {
        assert( biddyVariableTable.table[ftop].value != NULL );
        if (biddyVariableTable.table[ftop].value == biddyZero) {
          ftop = Biddy_Managed_GetNextVariable(MNG,ftop);
        } else {
          r = FALSE;
          ftop = fv = 0;
        }
      }
      if (fv != 0) {
        if (biddyVariableTable.table[fv].value == biddyZero) {
          f = BiddyE(f);
          /* for ZBDDs, 'else' successor is never marked */
        } else {
          f = BiddyT(f);
          /* for ZBDDC, mark is transfered only to the left successor */
          r = TRUE;
          /* for ZBDDC, 'then' successor can be marked */
          if (Biddy_GetMark(f)) r = !r;
        }
        ftop = Biddy_Managed_GetNextVariable(MNG,fv);
      }

    } else if ((biddyManagerType == BIDDYTYPETZBDDC) || (biddyManagerType == BIDDYTYPETZBDD)) {

      while (ftop != fv) {
        assert( biddyVariableTable.table[ftop].value != NULL );
        if (biddyVariableTable.table[ftop].value == biddyZero) {
          ftop = Biddy_Managed_GetNextVariable(MNG,ftop);
        } else {
          r = FALSE;
          ftop = fv = 0;
        }
      }
      if (fv != 0) {
        if (biddyVariableTable.table[fv].value == biddyZero) {
          f = BiddyE(f);
          /* for TZBDD, marks are not transfered but used only to denote 0 */
          if (Biddy_GetMark(f)) r = FALSE;
        } else {
          f = BiddyT(f);
          /* for TZBDD, marks are not transfered but used only to denote 0 */
          if (Biddy_GetMark(f)) r = FALSE;
        }
        ftop = Biddy_GetTag(f);
      }
    }
  }

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

  if (biddyManagerType == BIDDYTYPEOBDD) {
    /* IMPLEMENTED */
    /* assert( printf("Biddy_Random: OBDD\n") ); */
  } else if (biddyManagerType == BIDDYTYPEOBDDC) {
    /* IMPLEMENTED */
    /* assert( printf("Biddy_Random: OBDD\n") ); */
  } else if (biddyManagerType == BIDDYTYPEZBDDC) {
    /* IMPLEMENTED */
    /* assert( printf("Biddy_Random: ZBDD\n") ); */
  } else if (biddyManagerType == BIDDYTYPETZBDD) {
    /* IMPLEMENTED */
    /* assert( printf("Biddy_Random: TZBDD\n") ); */
  } else if ((biddyManagerType == BIDDYTYPEZBDD) ||
              (biddyManagerType == BIDDYTYPETZBDDC))
  {
    fprintf(stderr,"Biddy_Random: this GDD type is not supported, yet!\n");
    return biddyNull;
  } else if ((biddyManagerType == BIDDYTYPEOFDDC) || (biddyManagerType == BIDDYTYPEOFDD) ||
              (biddyManagerType == BIDDYTYPEZFDDC) || (biddyManagerType == BIDDYTYPEZFDD) ||
              (biddyManagerType == BIDDYTYPETZFDDC) || (biddyManagerType == BIDDYTYPETZFDD))
  {
    fprintf(stderr,"Biddy_Random: this GDD type is not supported, yet!\n");
    return biddyNull;
  } else {
    fprintf(stderr,"Biddy_Random: Unsupported GDD type!\n");
    return biddyNull;
  }

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

    /* DEBUGGING */
    /*
    printf("RANDOM: m=%llu\n",m);
    */

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
      assert( BiddyManagedNot(MNG,BiddyManagedNot(MNG,q)) == q );
    }

    tmp = BiddyManagedOr(MNG,result,q);
    if (tmp != result) {
      result = tmp;
      m--;
    }

  }

  if (opposite) {
    result = BiddyManagedNot(MNG,result);
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

  if (biddyManagerType == BIDDYTYPEOBDD) {
    /* IMPLEMENTED */
    /* assert( printf("Biddy_RandomSet: OBDD\n") ); */
  } else if (biddyManagerType == BIDDYTYPEOBDDC) {
    /* IMPLEMENTED */
    /* assert( printf("Biddy_RandomSet: OBDD\n") ); */
  } else if (biddyManagerType == BIDDYTYPEZBDDC) {
    /* IMPLEMENTED */
    /* assert( printf("Biddy_RandomSet: ZBDD\n") ); */
  } else if (biddyManagerType == BIDDYTYPETZBDD) {
    /* IMPLEMENTED */
    /* assert( printf("Biddy_RandomSet: TZBDD\n") ); */
  } else if ((biddyManagerType == BIDDYTYPEZBDD) ||
              (biddyManagerType == BIDDYTYPETZBDDC))
  {
    fprintf(stderr,"Biddy_RandomSet: this GDD type is not supported, yet!\n");
    return biddyNull;
  } else if ((biddyManagerType == BIDDYTYPEOFDDC) || (biddyManagerType == BIDDYTYPEOFDD) ||
              (biddyManagerType == BIDDYTYPEZFDDC) || (biddyManagerType == BIDDYTYPEZFDD) ||
              (biddyManagerType == BIDDYTYPETZFDDC) || (biddyManagerType == BIDDYTYPETZFDD))
  {
    fprintf(stderr,"Biddy_RandomSet: this GDD type is not supported, yet!\n");
    return biddyNull;
  } else {
    fprintf(stderr,"Biddy_RandomSet: Unsupported GDD type!\n");
    return biddyNull;
  }

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
    result = BiddyManagedNot(MNG,result);
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
    Nodes are prolonged recursively until a non-obsolete node is reached.
### Side effects
    The goal is to make nodes non-obsolete and not to give them correct expiry
    value.
### More info
*******************************************************************************/

void
BiddyProlongRecursively(Biddy_Manager MNG, Biddy_Edge f, unsigned int c)
{
  if (((BiddyNode *) BiddyP(f))->expiry &&
      (((BiddyNode *) BiddyP(f))->expiry < biddySystemAge))
  {
    BiddyProlongOne(f,c);
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
    For ZBDD, all variables are noticeable variables.
    For TZBDD, noticeable variables are all variables equal or below a top
    variable (considering the tag).
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

  /* DETERMINE NUMBER OF NODES */
  Biddy_Managed_SelectNode(MNG,biddyTerminal);
  if ((biddyManagerType == BIDDYTYPEOBDDC) || (biddyManagerType == BIDDYTYPEOBDD)) {
    BiddyNodeVarNumber(MNG,f,&num); /* NOW, ALL NODES ARE SELECTED, NOTICEABLE VARS COUNTED */
  }
  else if ((biddyManagerType == BIDDYTYPEZBDDC) || (biddyManagerType == BIDDYTYPEZBDD)) {
    BiddyNodeNumber(MNG,f,&num); /* NOW, ALL NODES ARE SELECTED, NOTICEABLE VARS DOES NOT NEED TO BE COUNTED */
  }
  else if ((biddyManagerType == BIDDYTYPETZBDDC) || (biddyManagerType == BIDDYTYPETZBDD)) {
    BiddyNodeNumber(MNG,f,&num); /* NOW, ALL NODES ARE SELECTED, NOTICEABLE VARS DOES NOT NEED TO BE COUNTED */
  }
  else if ((biddyManagerType == BIDDYTYPEOFDDC) || (biddyManagerType == BIDDYTYPEOFDD) ||
              (biddyManagerType == BIDDYTYPEZFDDC) || (biddyManagerType == BIDDYTYPEZFDD) ||
              (biddyManagerType == BIDDYTYPETZFDDC) || (biddyManagerType == BIDDYTYPETZFDD))
  {
    fprintf(stderr,"BiddyCreateLocalInfo: this GDD type is not supported, yet!\n");
    return 0;
  }
  else {
    fprintf(stderr,"BiddyCreateLocalInfo: Unsupported GDD type!\n");
    return 0;
  }

  Biddy_Managed_DeselectNode(MNG,biddyTerminal); /* REQUIRED FOR createLocalInfo */

  /* DETERMINE NUMBER OF NOTICEABLE VARIABLES */
  var = 0;
  if ((biddyManagerType == BIDDYTYPEOBDDC) || (biddyManagerType == BIDDYTYPEOBDD)) {
    for (i=1;i<biddyVariableTable.num;i++) {
      if (biddyVariableTable.table[i].selected == TRUE) {
        var++;
        biddyVariableTable.table[i].selected = FALSE; /* deselect variable */
      }
    }
  }
  else if ((biddyManagerType == BIDDYTYPEZBDDC) || (biddyManagerType == BIDDYTYPEZBDD)) {
    var = biddyVariableTable.num-1;
  }
  else if ((biddyManagerType == BIDDYTYPETZBDDC) || (biddyManagerType == BIDDYTYPETZBDD)) {
    for (i=1;i<biddyVariableTable.num;i++) {
      if ((Biddy_GetTag(f) == i) || BiddyIsSmaller(Biddy_GetTag(f),i)) {
        var++;
      }
    }
  }
  else if ((biddyManagerType == BIDDYTYPEOFDDC) || (biddyManagerType == BIDDYTYPEOFDD) ||
              (biddyManagerType == BIDDYTYPEZFDDC) || (biddyManagerType == BIDDYTYPEZFDD) ||
              (biddyManagerType == BIDDYTYPETZFDDC) || (biddyManagerType == BIDDYTYPETZFDD))
  {
    fprintf(stderr,"BiddyCreateLocalInfo: this GDD type is not supported, yet!\n");
    return 0;
  }
  else {
    fprintf(stderr,"BiddyCreateLocalInfo: Unsupported GDD type!\n");
    return 0;
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
\brief Function BiddyCopy create a copy of BDD in another manager of the same
        BDD type.

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
  Biddy_Variable tag;
  Biddy_Boolean mark;
  Biddy_String top;
  Biddy_Variable v;
  Biddy_Edge r;

  assert( MNG1 != MNG2 );
  assert( biddyManagerType1 == biddyManagerType2 );

  tag = Biddy_GetTag(f); /* tag of the given edge */
  mark = Biddy_GetMark(f); /* complement bit of the given edge */

  /* TO DO: use cache to make it more efficient */

  if (f == biddyNull) return biddyNull;
  if (Biddy_IsConstant(f)) {
    if (mark) {
      r = Biddy_Inv(biddyTerminal2);
    } else {
      r = biddyTerminal2;
    }
    if (tag) Biddy_SetTag(r,tag);
  } else {
    top = Biddy_Managed_GetTopVariableName(MNG1,f); /* top variable of the given edge */
    v = Biddy_Managed_GetVariable(MNG2,top);
    e = BiddyCopy(MNG1,MNG2,Biddy_Managed_TransferMark(MNG1,BiddyE(f),mark,TRUE)); /* TRUE = left */
    t = BiddyCopy(MNG1,MNG2,Biddy_Managed_TransferMark(MNG1,BiddyT(f),mark,FALSE)); /* FALSE = right */
    r = BiddyManagedTaggedFoaNode(MNG2,v,e,t,tag,TRUE);
    BiddyProlongOne(r,biddySystemAge2);
  }

  return r;
}

/*******************************************************************************
\brief Function BiddyCopyOBDD copies a BIDDYTYPEOBDD or BIDDYTYPEOBDDC
        into another manager.

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

  assert ( (biddyManagerType1 == BIDDYTYPEOBDDC) || (biddyManagerType1 == BIDDYTYPEOBDD) );

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
\brief Function BiddyCopyZBDD copies a BIDDYTYPEZBDD or BIDDYTYPEZBDDC
        into another manager.

### Description
### Side effects
    Expiry value of the original nodes is not copied.
    All created nodes are refreshed in the new manager.
    The variable ordering of created BDD is adapted to the target manager.
### More info
*******************************************************************************/

Biddy_Edge
BiddyCopyZBDD(Biddy_Manager MNG1, Biddy_Manager MNG2, Biddy_Edge f)
{
  Biddy_String topname;
  Biddy_Variable k;
  Biddy_Variable v;
  Biddy_Boolean mark;
  Biddy_Edge e,t,r;

  assert ( (biddyManagerType1 == BIDDYTYPEZBDDC) || (biddyManagerType1 == BIDDYTYPEZBDD) );

  if (f == biddyNull) return biddyNull;
  if (f == biddyZero1) return biddyZero2;
  if (f == biddyTerminal1) {
    r = biddyOne2;
    for (k = 1; k < biddyVariableTable1.num; k++) {
      topname = Biddy_Managed_GetVariableName(MNG1,k);
      v = Biddy_Managed_GetVariable(MNG2,topname); /* variable must exist */
      r = Biddy_Managed_ITE(MNG2,Biddy_Managed_GetVariableEdge(MNG2,v),biddyZero2,r);
    }
    return r;
  }

  topname = Biddy_Managed_GetTopVariableName(MNG1,f);
  mark = Biddy_GetMark(f);
  v = Biddy_Managed_GetVariable(MNG2,topname); /* variable must exist */
  e = BiddyCopyZBDD(MNG1,MNG2,Biddy_Managed_TransferMark(MNG1,BiddyE(f),mark,TRUE)); /* TRUE = left */
  t = BiddyCopyZBDD(MNG1,MNG2,Biddy_Managed_TransferMark(MNG1,BiddyT(f),mark,FALSE)); /* FALSE = right */
  t = Biddy_Managed_Change(MNG2,t,v);
  r = Biddy_Managed_Or(MNG2,e,t); /* or/xor, both is correct, what is faster? */

  return r;
}

/*******************************************************************************
\brief Function BiddyCopyTZBDD copies a BIDDYTYPETZBDD or BIDDYTYPETZBDDC
        into another manager.

### Description
### Side effects
    Expiry value of the original nodes is not copied.
    All created nodes are refreshed in the new manager.
    The variable ordering of created BDD is adapted to the target manager.
### More info
*******************************************************************************/

Biddy_Edge
BiddyCopyTZBDD(Biddy_Manager MNG1, Biddy_Manager MNG2, Biddy_Edge f)
{
  Biddy_String topname;
  Biddy_Variable top,tag;
  Biddy_Variable v;
  Biddy_Edge e,t,r;

  assert ( (biddyManagerType1 == BIDDYTYPETZBDD) || (biddyManagerType1 == BIDDYTYPETZBDDC) );

  if (f == biddyNull) return biddyNull;
  if (f == biddyZero1) return biddyZero2;

  top = BiddyV(f);
  if (!top) {
    r = biddyOne2;
  } else {
    topname = Biddy_Managed_GetVariableName(MNG1,top);
    v = Biddy_Managed_GetVariable(MNG2,topname); /* variable must exist */
    e = BiddyCopyTZBDD(MNG1,MNG2,BiddyE(f));
    t = BiddyCopyTZBDD(MNG1,MNG2,BiddyT(f)); 
    r = Biddy_Managed_ITE(MNG2,Biddy_Managed_GetVariableEdge(MNG2,v),t,e);
  }

  tag = Biddy_GetTag(f);
  while (top != tag) {
    top = biddyVariableTable1.table[top].prev;
    topname = Biddy_Managed_GetVariableName(MNG1,top);
    v = Biddy_Managed_GetVariable(MNG2,topname); /* variable must exist */
    r = Biddy_Managed_ITE(MNG2,Biddy_Managed_GetVariableEdge(MNG2,v),biddyZero2,r);
  }
  
  return r;
}

/*******************************************************************************
\brief Function BiddyConvertDirect converts a graph from one BDD type to
       another BDD type assuming that the target manager is empty.

### Description
### Side effects
    This should be faster than a general copy functions.
### More info
*******************************************************************************/

Biddy_Edge
BiddyConvertDirect(Biddy_Manager MNG1, Biddy_Manager MNG2, Biddy_Edge f)
{
  Biddy_Edge e,t;
  Biddy_Variable tag,top;
  Biddy_Boolean mark;
  Biddy_String topname;
  Biddy_Variable v;
  Biddy_Variable prevvar;
  Biddy_Edge r;

  assert( biddyManagerType1 != BIDDYTYPEOFDD ); /* NOT IMPLEMENTED, YET */
  assert( biddyManagerType1 != BIDDYTYPEOFDDC ); /* NOT IMPLEMENTED, YET */
  assert( biddyManagerType1 != BIDDYTYPEZFDD ); /* NOT IMPLEMENTED, YET */
  assert( biddyManagerType1 != BIDDYTYPEZFDDC ); /* NOT IMPLEMENTED, YET */
  assert( biddyManagerType1 != BIDDYTYPETZFDD ); /* NOT IMPLEMENTED, YET */
  assert( biddyManagerType1 != BIDDYTYPETZFDDC ); /* NOT IMPLEMENTED, YET */
  assert( biddyManagerType2 != BIDDYTYPEOFDD ); /* NOT IMPLEMENTED, YET */
  assert( biddyManagerType2 != BIDDYTYPEOFDDC ); /* NOT IMPLEMENTED, YET */
  assert( biddyManagerType2 != BIDDYTYPEZFDD ); /* NOT IMPLEMENTED, YET */
  assert( biddyManagerType2 != BIDDYTYPEZFDDC ); /* NOT IMPLEMENTED, YET */
  assert( biddyManagerType2 != BIDDYTYPETZFDD ); /* NOT IMPLEMENTED, YET */
  assert( biddyManagerType2 != BIDDYTYPETZFDDC ); /* NOT IMPLEMENTED, YET */

  assert( MNG1 != MNG2 );

  /* DEBUGGING */
  /*
  printf("Convert direct: f = %s (%p)\n",Biddy_Managed_GetTopVariableName(MNG1,f),f);
  */

  /* TO DO: use cache to get more efficient solution! */

  if (f == biddyNull) return biddyNull;

  r = biddyNull;

  if ((biddyManagerType1 == BIDDYTYPEOBDDC) || (biddyManagerType1 == BIDDYTYPEOBDD)) {

    if (Biddy_IsConstant(f)) {
      if (Biddy_GetMark(f)) {
        return Biddy_Complement(biddyTerminal2);
      } else {
        return biddyTerminal2;
      }
    }

    mark = Biddy_GetMark(f); /* complement bit of the given edge */
    top = Biddy_GetTopVariable(f); /* top variable of the given edge */
    topname = Biddy_Managed_GetVariableName(MNG1,top); /* name of the top variable of the given edge */
    v = Biddy_Managed_GetVariable(MNG2,topname);

    if ((biddyManagerType2 == BIDDYTYPEZBDDC) || (biddyManagerType2 == BIDDYTYPEZBDD)) {
      top = Biddy_Managed_GetVariable(MNG2,topname);
    }

    e = BiddyConvertDirect(MNG1,MNG2,Biddy_Managed_TransferMark(MNG1,BiddyE(f),mark,TRUE)); /* TRUE = left */

    if ((biddyManagerType2 == BIDDYTYPEZBDDC) || (biddyManagerType2 == BIDDYTYPEZBDD)) {
      topname = Biddy_Managed_GetTopVariableName(MNG1,BiddyE(f));
      prevvar = Biddy_Managed_GetVariable(MNG2,topname);
      prevvar = biddyVariableTable2.table[prevvar].prev;
      while (prevvar != top) {
        e = BiddyManagedTaggedFoaNode(MNG2,prevvar,e,e,0,TRUE);
        BiddyProlongOne(e,biddySystemAge2);
        prevvar = biddyVariableTable2.table[prevvar].prev;
      }
    }

    t = BiddyConvertDirect(MNG1,MNG2,Biddy_Managed_TransferMark(MNG1,BiddyT(f),mark,FALSE)); /* FALSE = right */

    if ((biddyManagerType2 == BIDDYTYPEZBDDC) || (biddyManagerType2 == BIDDYTYPEZBDD)) {
      topname = Biddy_Managed_GetTopVariableName(MNG1,BiddyT(f));
      prevvar = Biddy_Managed_GetVariable(MNG2,topname);
      prevvar = biddyVariableTable2.table[prevvar].prev;
      while (prevvar != top) {
        t = BiddyManagedTaggedFoaNode(MNG2,prevvar,t,t,0,TRUE);
        BiddyProlongOne(t,biddySystemAge2);
        prevvar = biddyVariableTable2.table[prevvar].prev;
      }
    }

    r = BiddyManagedTaggedFoaNode(MNG2,v,e,t,v,TRUE);
    BiddyProlongOne(r,biddySystemAge2);

  }

  else if ((biddyManagerType1 == BIDDYTYPEZBDDC) || (biddyManagerType1 == BIDDYTYPEZBDD)) {

    if ((biddyManagerType2 == BIDDYTYPETZBDDC) || (biddyManagerType2 == BIDDYTYPETZBDD)) {
      /* TO DO: DIRECT CONVERSION ZBDD -> TZBDD IS NOT IMPLEMENTED, YET */
      Biddy_Manager MNGTMP;
      Biddy_Edge tmp;
      Biddy_InitMNG(&MNGTMP,BIDDYTYPEOBDDC);
      Biddy_Managed_Copy(MNG1,MNGTMP,biddyZero1); /* copy domain */
      tmp = BiddyConvertDirect(MNG1,MNGTMP,f);
      r = BiddyConvertDirect(MNGTMP,MNG2,tmp);
      Biddy_ExitMNG(&MNGTMP);
      return r;
    }

    if (Biddy_IsConstant(f)) {
      if (Biddy_GetMark(f)) {
        return Biddy_Complement(biddyTerminal2);
      } else {
        return biddyTerminal2;
      }
    }

    mark = Biddy_GetMark(f); /* complement bit of the given edge */
    top = Biddy_GetTopVariable(f); /* top variable of the given edge */
    topname = Biddy_Managed_GetVariableName(MNG1,top); /* name of the top variable of the given edge */
    v = Biddy_Managed_GetVariable(MNG2,topname);

    if ((biddyManagerType2 == BIDDYTYPEOBDDC) || (biddyManagerType2 == BIDDYTYPEOBDD)){
      top = Biddy_Managed_GetVariable(MNG2,topname);
    }

    e = BiddyConvertDirect(MNG1,MNG2,Biddy_Managed_TransferMark(MNG1,BiddyE(f),mark,TRUE)); /* TRUE = left */

    if ((biddyManagerType2 == BIDDYTYPEOBDDC) || (biddyManagerType2 == BIDDYTYPEOBDD)) {
      topname = Biddy_Managed_GetTopVariableName(MNG1,BiddyE(f));
      prevvar = Biddy_Managed_GetVariable(MNG2,topname);
      prevvar = biddyVariableTable2.table[prevvar].prev;
      while (prevvar != top) {
        e = BiddyManagedTaggedFoaNode(MNG2,prevvar,e,biddyZero2,prevvar,TRUE);
        BiddyProlongOne(e,biddySystemAge2);
        prevvar = biddyVariableTable2.table[prevvar].prev;
      }
    }

    t = BiddyConvertDirect(MNG1,MNG2,Biddy_Managed_TransferMark(MNG1,BiddyT(f),mark,FALSE)); /* FALSE = right */

    if ((biddyManagerType2 == BIDDYTYPEOBDDC) || (biddyManagerType2 == BIDDYTYPEOBDD)) {
      topname = Biddy_Managed_GetTopVariableName(MNG1,BiddyT(f));
      prevvar = Biddy_Managed_GetVariable(MNG2,topname);
      prevvar = biddyVariableTable2.table[prevvar].prev;
      while (prevvar != top) {
        t = BiddyManagedTaggedFoaNode(MNG2,prevvar,t,biddyZero2,prevvar,TRUE);
        BiddyProlongOne(t,biddySystemAge2);
        prevvar = biddyVariableTable2.table[prevvar].prev;
      }
    }

    r = BiddyManagedTaggedFoaNode(MNG2,v,e,t,v,TRUE);
    BiddyProlongOne(r,biddySystemAge2);

  }

  else if ((biddyManagerType1 == BIDDYTYPETZBDDC) || (biddyManagerType1 == BIDDYTYPETZBDD)) {

    if ((biddyManagerType2 == BIDDYTYPEZBDDC) || (biddyManagerType2 == BIDDYTYPEZBDD)) {
      /* TO DO: DIRECT CONVERSION TZBDD -> ZBDD IS NOT IMPLEMENTED, YET */
      Biddy_Manager MNGTMP;
      Biddy_Edge tmp;
      Biddy_InitMNG(&MNGTMP,BIDDYTYPEOBDDC);
      Biddy_Managed_Copy(MNG1,MNGTMP,biddyZero1); /* copy domain */
      tmp = BiddyConvertDirect(MNG1,MNGTMP,f);
      r = BiddyConvertDirect(MNGTMP,MNG2,tmp);
      Biddy_ExitMNG(&MNGTMP);
      return r;
    }

    tag = Biddy_GetTag(f); /* tag of the given edge */
    mark = Biddy_GetMark(f); /* complement bit of the given edge */
    top = Biddy_GetTopVariable(f); /* top variable of the given edge */

    if (Biddy_IsConstant(f)) {
      if (Biddy_GetMark(f)) {
        r = Biddy_Complement(biddyTerminal2); /* this should not be reached */
      } else {
        r = biddyTerminal2;
      }
    } else {
      topname = Biddy_Managed_GetVariableName(MNG1,top); /* name of the top variable of the given edge */
      v = Biddy_Managed_GetVariable(MNG2,topname);
      e = BiddyConvertDirect(MNG1,MNG2,Biddy_Managed_TransferMark(MNG1,BiddyE(f),mark,TRUE)); /* TRUE = left */
      t = BiddyConvertDirect(MNG1,MNG2,Biddy_Managed_TransferMark(MNG1,BiddyT(f),mark,FALSE)); /* FALSE = right */
      r = BiddyManagedTaggedFoaNode(MNG2,v,e,t,v,TRUE);
      BiddyProlongOne(r,biddySystemAge2);
    }

    while (tag != top) {
      top = biddyVariableTable1.table[top].prev;
      topname = Biddy_Managed_GetVariableName(MNG1,top);
      v = Biddy_Managed_GetVariable(MNG2,topname);
      r = BiddyManagedTaggedFoaNode(MNG2,v,r,biddyZero2,v,TRUE);
      BiddyProlongOne(r,biddySystemAge2);
    }

  }

  /* DEBUGGING */
  /*
  printf("Result: f = %s (%p)\n",Biddy_Managed_GetTopVariableName(MNG2,r),r);
  */

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

  /* FORMULAE */
  if ((biddyManagerType == BIDDYTYPETZBDDC) || (biddyManagerType == BIDDYTYPETZBDD) ||
      (biddyManagerType == BIDDYTYPETZFDDC) || (biddyManagerType == BIDDYTYPETZFDD))
  {
    for (i = 1; i < biddyFormulaTable.size; i++) {
      if (!Biddy_IsNull(biddyFormulaTable.table[i].f) &&
          !Biddy_IsConstant(biddyFormulaTable.table[i].f))
      {
        printf("CHECKING (BiddySystemReport): %s\n",biddyFormulaTable.table[i].name);
        if (BiddyIsSmaller(BiddyV(biddyFormulaTable.table[i].f),Biddy_GetTag(biddyFormulaTable.table[i].f))) {
          fprintf(stderr,"ERROR (BiddySystemReport): Formula %s has bad tag for its top edge\n",biddyFormulaTable.table[i].name);
          fprintf(stderr,"TOP EDGE: tag=\"%s\", variable \"%s\", elsetag=\"%s\", else=\"%s\", thentag=\"%s\", then=\"%s\"\n",
            Biddy_Managed_GetVariableName(MNG,Biddy_GetTag(biddyFormulaTable.table[i].f)),
            Biddy_Managed_GetTopVariableName(MNG,biddyFormulaTable.table[i].f),
            Biddy_Managed_GetVariableName(MNG,Biddy_GetTag(BiddyE(biddyFormulaTable.table[i].f))),
            Biddy_Managed_GetTopVariableName(MNG,BiddyE(biddyFormulaTable.table[i].f)),
            Biddy_Managed_GetVariableName(MNG,Biddy_GetTag(BiddyT(biddyFormulaTable.table[i].f))),
            Biddy_Managed_GetTopVariableName(MNG,BiddyT(biddyFormulaTable.table[i].f)));
          free((void *)1); /* BREAKPOINT */
        }

        /* this will check the structure of the graph */
        /* this should not be used if some nodes are selected */
        /*
        {
        unsigned int n = 1;
        Biddy_Managed_SelectNode(MNG,biddyTerminal);
        BiddyNodeNumber(MNG,biddyFormulaTable.table[i].f,&n); 
        Biddy_Managed_DeselectAll(MNG);
        }
        */
      }
    }
  }

  /* BDD SYSTEM BY VARIABLE'S LISTS */

#ifdef SYSTEMREPORTVERBOSE
  printf("SYSTEM REPORT (VARIABLE LISTS)\n");
#endif

  countfresh = 1; /* all variables, including constant 1, are fresh */
  countfortified = countprolonged = countobsolete = 0;
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
      fprintf(stderr,"ERROR (BiddySystemReport): Empty list for variable \"%s\"\n",biddyVariableTable.table[v].name);
      free((void *)1); /* BREAKPOINT */
    }

    if ((biddyManagerType == BIDDYTYPEOBDDC) || (biddyManagerType == BIDDYTYPEOBDD) ||
        (biddyManagerType == BIDDYTYPEOFDDC) || (biddyManagerType == BIDDYTYPEOFDD))
    {
      if ((sup->f != biddyZero) || (sup->t != biddyTerminal)) {
        fprintf(stderr,"ERROR (BiddySystemReport): Bad first element in the list for variable \"%s\"\n",biddyVariableTable.table[v].name);
        fprintf(stderr,"NODE: pointer=%p, variable \"%s\", else=\"%s\", then=\"%s\"\n",
          (void *) sup,
          sup?Biddy_Managed_GetTopVariableName(MNG,sup):"NULL",
          sup->f?Biddy_Managed_GetTopVariableName(MNG,sup->f):"NULL",
          sup->t?Biddy_Managed_GetTopVariableName(MNG,sup->t):"NULL");
        free((void *)1); /* BREAKPOINT */
      }
    }

    if ((biddyManagerType == BIDDYTYPEZBDDC) || (biddyManagerType == BIDDYTYPEZBDD) ||
        (biddyManagerType == BIDDYTYPEZFDDC) || (biddyManagerType == BIDDYTYPEZFDD))
    {
      if ((sup->f != biddyTerminal) || (sup->t != biddyTerminal)) {
        fprintf(stderr,"ERROR (BiddySystemReport): Bad first element in the list for variable \"%s\"\n",biddyVariableTable.table[v].name);
        fprintf(stderr,"NODE: pointer=%p, variable \"%s\", else=\"%s\", then=\"%s\"\n",
          (void *) sup,
          sup?Biddy_Managed_GetTopVariableName(MNG,sup):"NULL",
          sup->f?Biddy_Managed_GetTopVariableName(MNG,sup->f):"NULL",
          sup->t?Biddy_Managed_GetTopVariableName(MNG,sup->t):"NULL");
        free((void *)1); /* BREAKPOINT */
      }
    }

    if ((biddyManagerType == BIDDYTYPETZBDDC) || (biddyManagerType == BIDDYTYPETZBDD) ||
        (biddyManagerType == BIDDYTYPETZFDDC) || (biddyManagerType == BIDDYTYPETZFDD))
    {
      if ((sup->f != biddyZero) || (sup->t != biddyTerminal)) {
        fprintf(stderr,"ERROR (BiddySystemReport): Bad first element in the list for variable \"%s\"\n",biddyVariableTable.table[v].name);
        fprintf(stderr,"NODE: pointer=%p, variable \"%s\", else=\"%s\", then=\"%s\"\n",
          (void *) sup,
          sup?Biddy_Managed_GetTopVariableName(MNG,sup):"NULL",
          sup->f?Biddy_Managed_GetTopVariableName(MNG,sup->f):"NULL",
          sup->t?Biddy_Managed_GetTopVariableName(MNG,sup->t):"NULL");
        free((void *)1); /* BREAKPOINT */
      }
    }

    if (sup->expiry != biddySystemAge) {
      fprintf(stderr,"ERROR (BiddySystemReport): First element in the list for variable \"%s\" is not fresh (expiry = %u, biddySystemAge = %u)\n",
              biddyVariableTable.table[v].name,sup->expiry,biddySystemAge);
      fprintf(stderr,"NODE: pointer=%p, variable \"%s\", else=\"%s\", then=\"%s\"\n",
        (void *) sup,
        sup?Biddy_Managed_GetTopVariableName(MNG,sup):"NULL",
        sup->f?Biddy_Managed_GetTopVariableName(MNG,sup->f):"NULL",
        sup->t?Biddy_Managed_GetTopVariableName(MNG,sup->t):"NULL");
      free((void *)1); /* BREAKPOINT */
    }

    do {

      if (!sup) {
        fprintf(stderr,"ERROR (BiddySystemReport): Problem with lastNode for variable \"%s\"\n",biddyVariableTable.table[v].name);
        free((void *)1); /* BREAKPOINT */
      }

      if (sup->v != v) {
        fprintf(stderr,"ERROR (BiddySystemReport): Node with bad variable in list for variable \"%s\"\n",biddyVariableTable.table[v].name);
        fprintf(stderr,"NODE: pointer=%p, variable \"%s\", else=\"%s\", then=\"%s\"\n",
          (void *) sup,
          Biddy_Managed_GetTopVariableName(MNG,sup),
          Biddy_Managed_GetTopVariableName(MNG,sup->f),
          Biddy_Managed_GetTopVariableName(MNG,sup->t));
        free((void *)1); /* BREAKPOINT */
      }

      if ((!BiddyIsSmaller(v,BiddyV(sup->f))) ||
          (!BiddyIsSmaller(v,BiddyV(sup->t)))
         )
      {
        fprintf(stderr,"ERROR (BiddySystemReport): Node with bad ordering in list for variable \"%s\"\n",biddyVariableTable.table[v].name);
        fprintf(stderr,"NODE: pointer=%p, variable \"%s\", else=\"%s\", then=\"%s\"\n",
          (void *) sup,
          Biddy_Managed_GetTopVariableName(MNG,sup),
          Biddy_Managed_GetTopVariableName(MNG,sup->f),
          Biddy_Managed_GetTopVariableName(MNG,sup->t));
        free((void *)1); /* BREAKPOINT */
      }

      if ((biddyManagerType == BIDDYTYPETZBDDC) || (biddyManagerType == BIDDYTYPETZBDD) ||
          (biddyManagerType == BIDDYTYPETZFDDC) || (biddyManagerType == BIDDYTYPETZFDD))
      {
        if (BiddyIsSmaller(BiddyV(sup->f),Biddy_GetTag(sup->f)) ||
            BiddyIsSmaller(BiddyV(sup->t),Biddy_GetTag(sup->t)))
        {
          fprintf(stderr,"ERROR (BiddySystemReport): Node \"%s\" has bad tag for its successor\n",biddyVariableTable.table[v].name);
          fprintf(stderr,"NODE: pointer=%p, expiry=%d, variable \"%s\", elsetag=\"%s\", else=\"%s\", thentag=\"%s\", then=\"%s\"\n",
            (void *) sup,sup->expiry,
            Biddy_Managed_GetTopVariableName(MNG,sup),
            Biddy_Managed_GetVariableName(MNG,Biddy_GetTag(sup->f)),
            Biddy_Managed_GetTopVariableName(MNG,sup->f),
            Biddy_Managed_GetVariableName(MNG,Biddy_GetTag(sup->t)),
            Biddy_Managed_GetTopVariableName(MNG,sup->t));
          free((void *)1); /* BREAKPOINT */
        }
      }

      if (sup != biddyVariableTable.table[v].firstNode) {
        hash = nodeTableHash(v,sup->f,sup->t,biddyNodeTable.size);
        sup1 = biddyNodeTable.table[hash];
        findNodeTable(MNG,v,sup->f,sup->t,&sup1);
        if (!sup1) {
          fprintf(stderr,"ERROR (BiddySystemReport): Node not correctly inserted into Node table\n");
          fprintf(stderr,"NODE: pointer=%p, variable \"%s\", else=\"%s\", then=\"%s\"\n",
            (void *) sup,
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
            fprintf(stderr,"NODE: pointer=%p, expiry=%d, variable \"%s\", else=\"%s\" (%p), then=\"%s\" (%p)\n",
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
      printf("expiry=%d, variable \"%s\", else=\"%s\", then=\"%s\"\n",
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
      fprintf(stderr,"VARIABLE: variable \"%s\", num=%u, count1=%u\n",
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
  countfresh = biddyVariableTable.num; /* variables are not stored in Node table, they are fresh */
  countprolonged = countfortified = countobsolete = 0;
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
    BiddyIsSmaller(BiddyV(f),BiddyV(BiddyE(f))) &&
    BiddyIsSmaller(BiddyV(f),BiddyV(BiddyT(f))) &&
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

static inline unsigned int
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

  /* this is experimentally determined, not using v, efficient for OBDDs, only */
  /*
  {
  uintptr_t k;
  k = ((uintptr_t) pt * 352241492 + (uintptr_t) pf) * 231629123;
  hash = 1 + ((k >> 32) & size);
  return hash;
  }
  */

  /* this is experimentally determined, using v, seems to be better for TZBDDs */
  /**/
  {
  uintptr_t k;
  k = ((uintptr_t) pt * 103406078 + (uintptr_t) pf + (uintptr_t) v) * 1186577883;
  hash = 1 + ((k >> 24) & size);
  return hash;
  }
  /**/

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

static inline unsigned int
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
\brief Function keywordHash.

### Description
### Side effects
### More info
*******************************************************************************/

static inline unsigned int
keywordHash(Biddy_Edge a, unsigned int k, unsigned int size)
{

  unsigned int hash;

  /* this is very simple */
  /**/
  {
  uintptr_t key;
  key = (uintptr_t) a >> 3;
  hash = key & size;
  return hash;
  }
  /**/

#if UINTPTR_MAX == 0xffffffffffffffff

  /* this is experimentally determined */
  {
  uintptr_t key;
  key = ((uintptr_t) a >> 3) * 1327958971;
  hash = (key >> 19) & size;
  return hash;
  }

#else

  /* this is experimentally determined */
  {
  uintptr_t key;
  key = ((uintptr_t) a >> 3) * 7958971;
  hash = (key >> 13) & size;
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

static inline void
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

static inline BiddyNode *
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

static inline void
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

static inline Biddy_Boolean
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
\brief Function addKeywordCache adds a result to the cache table for
       one-arguments operation and labels the entry with a given keyword.

### Description
### Side effects
### More info
*******************************************************************************/

static inline void
addKeywordCache(Biddy_Manager MNG, BiddyKeywordCacheTable cache,
                Biddy_Edge a, unsigned int k, Biddy_Edge r,
                unsigned int index)
{
  BiddyKeywordCache *p;

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
  p->keyword = k;
  p->result = r;
}

/*******************************************************************************
\brief Function findKeywordCache looks for the result in the cache table for
       one-argument operation which is labelled with a given keyword.

### Description
### Side effects
### More info
*******************************************************************************/

static inline Biddy_Boolean
findKeywordCache(Biddy_Manager MNG, BiddyKeywordCacheTable cache,
                 Biddy_Edge a, unsigned int k, Biddy_Edge *r,
                 unsigned int *index)
{
  Biddy_Boolean q;
  BiddyKeywordCache *p;

  if (cache.disabled) return FALSE;

  q = FALSE;

  *index = keywordHash(a,k,cache.size);
  p = &cache.table[*index];

  (*cache.search)++;

  if ((p->f == a) &&
      (p->keyword == k) &&
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
      /* VARIANT A */
      /*
      if (((BiddyN(c->f)->expiry) && ((BiddyN(c->f)->expiry) < biddySystemAge)) ||
          ((BiddyN(c->g)->expiry) && ((BiddyN(c->g)->expiry) < biddySystemAge)) ||
          ((BiddyN(c->h)->expiry) && ((BiddyN(c->h)->expiry) < biddySystemAge)) ||
          ((BiddyN(c->result)->expiry) && ((BiddyN(c->result)->expiry) < biddySystemAge)))
      */
      /* VARIANT B */
      /*
      if (((BiddyN(c->result)->expiry) && ((BiddyN(c->result)->expiry) < biddySystemAge)) ||
          ((BiddyN(c->f)->expiry) && ((BiddyN(c->f)->expiry) < biddySystemAge)) ||
          ((BiddyN(c->g)->expiry) && ((BiddyN(c->g)->expiry) < biddySystemAge)) ||
          ((BiddyN(c->h)->expiry) && ((BiddyN(c->h)->expiry) < biddySystemAge)))
      */
      /* VARIANT C */
      /**/
      if (((BiddyN(c->f)->expiry) && ((BiddyN(c->f)->expiry) < biddySystemAge)) ||
          ((BiddyN(c->result)->expiry) && ((BiddyN(c->result)->expiry) < biddySystemAge)) ||
          ((BiddyN(c->g)->expiry) && ((BiddyN(c->g)->expiry) < biddySystemAge)) ||
          ((BiddyN(c->h)->expiry) && ((BiddyN(c->h)->expiry) < biddySystemAge)))
      /**/
      {
        c->result = biddyNull;
      }
    }
  }
}

/*******************************************************************************
\brief Function OPGarbageNewVariable is used to clean the OP cache after the
       adding new variable.

### Description
    This function is needed for BDD types where adding new variable below
    the existing one changes the meaning of the existing formulae.
### Side effects
### More info
*******************************************************************************/

static void
OPGarbageNewVariable(Biddy_Manager MNG, Biddy_Variable v)
{
  unsigned int j;
  BiddyOp3Cache *c;

  if (biddyOPCache.disabled) return;

  for (j=0; j<=biddyOPCache.size; j++) {
    if (!Biddy_IsNull(biddyOPCache.table[j].result)) {
      c = &biddyOPCache.table[j];
      if ((BiddyIsSmaller(BiddyN(c->f)->v,v)) ||
          (BiddyIsSmaller(BiddyN(c->g)->v,v)) ||
          (BiddyIsSmaller(BiddyN(c->h)->v,v)) ||
          (BiddyIsSmaller(BiddyN(c->result)->v,v)))
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
\brief Function EAGarbageNewVariable is used to clean the EA cache after the
       adding new variable.

### Description
    This function is needed for BDD types where adding new variable below
    the existing one changes the meaning of the existing formulae.
### Side effects
### More info
*******************************************************************************/

static void
EAGarbageNewVariable(Biddy_Manager MNG, Biddy_Variable v)
{
  unsigned int j;
  BiddyOp3Cache *c;

  if (biddyEACache.disabled) return;

  for (j=0; j<=biddyEACache.size; j++) {
    if (!Biddy_IsNull(biddyEACache.table[j].result)) {
      c = &biddyEACache.table[j];
      if ((BiddyIsSmaller(BiddyN(c->f)->v,v)) ||
          (BiddyIsSmaller(BiddyN(c->g)->v,v)) ||
          (BiddyIsSmaller(BiddyN(c->h)->v,v)) ||
          (BiddyIsSmaller(BiddyN(c->result)->v,v)))
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
          (c->g && (BiddyN(c->g)->expiry) && ((BiddyN(c->g)->expiry) < biddySystemAge)) ||
          ((BiddyN(c->h)->expiry) && ((BiddyN(c->h)->expiry) < biddySystemAge)) ||
          ((BiddyN(c->result)->expiry) && ((BiddyN(c->result)->expiry) < biddySystemAge)))
      {
        c->result = biddyNull;
      }
    }
  }
}

/*******************************************************************************
\brief Function RCGarbageNewVariable is used to clean the RC cache after the
       adding new variable.

### Description
    This function is needed for BDD types where adding new variable below
    the existing one changes the meaning of the existing formulae.
### Side effects
### More info
*******************************************************************************/

static void
RCGarbageNewVariable(Biddy_Manager MNG, Biddy_Variable v)
{
  unsigned int j;
  BiddyOp3Cache *c;

  if (biddyRCCache.disabled) return;

  for (j=0; j<=biddyRCCache.size; j++) {
    if (!Biddy_IsNull(biddyRCCache.table[j].result)) {
      c = &biddyRCCache.table[j];
      if ((BiddyIsSmaller(BiddyN(c->f)->v,v)) ||
          (c->g && BiddyIsSmaller(BiddyN(c->g)->v,v)) ||
          (BiddyIsSmaller(BiddyN(c->h)->v,v)) ||
          (BiddyIsSmaller(BiddyN(c->result)->v,v)))
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
\brief Function ReplaceGarbage performs Garbage Collection for the Replace
        cache.

### Description
### Side effects
### More info
*******************************************************************************/

static void
ReplaceGarbage(Biddy_Manager MNG)
{
  unsigned int j;
  BiddyKeywordCache *c;

  if (biddyReplaceCache.disabled) return;

  for (j=0; j<=biddyReplaceCache.size; j++) {
    if (!Biddy_IsNull(biddyReplaceCache.table[j].result)) {
      c = &biddyReplaceCache.table[j];
      if (((BiddyN(c->f)->expiry) && ((BiddyN(c->f)->expiry) < biddySystemAge)) ||
          ((BiddyN(c->result)->expiry) && ((BiddyN(c->result)->expiry) < biddySystemAge)))
      {
        c->result = biddyNull;
      }
    }
  }
}

/*******************************************************************************
\brief Function ReplaceGarbageNewVariable is used to clean the Replace cache
       after the adding new variable.

### Description
    This function is needed for BDD types where adding new variable below
    the existing one changes the meaning of the existing formulae.
### Side effects
### More info
*******************************************************************************/

static void
ReplaceGarbageNewVariable(Biddy_Manager MNG, Biddy_Variable v)
{
  unsigned int j;
  BiddyKeywordCache *c;

  if (biddyReplaceCache.disabled) return;

  for (j=0; j<=biddyReplaceCache.size; j++) {
    if (!Biddy_IsNull(biddyReplaceCache.table[j].result)) {
      c = &biddyReplaceCache.table[j];
      if ((BiddyIsSmaller(BiddyN(c->f)->v,v)) ||
          (BiddyIsSmaller(BiddyN(c->result)->v,v)))
      {
        c->result = biddyNull;
      }
    }
  }
}

/*******************************************************************************
\brief Function ReplaceGarbageDeleteAll deletes all entries in the Replace
        cache.

### Description
### Side effects
### More info
*******************************************************************************/

static void
ReplaceGarbageDeleteAll(Biddy_Manager MNG)
{
  unsigned int j;
  BiddyKeywordCache *c;

  if (biddyReplaceCache.disabled) return;

  for (j=0; j<=biddyReplaceCache.size; j++) {
    c = &biddyReplaceCache.table[j];
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
  Biddy_Variable v,k,top;
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

  /* ENSURE THE CORRECT ORDERING OF THE NEW VARIABLE */
  if ((biddyManagerType == BIDDYTYPEOBDDC) || (biddyManagerType == BIDDYTYPEOBDD) ||
      (biddyManagerType == BIDDYTYPEOFDDC) || (biddyManagerType == BIDDYTYPEOFDD))
  {
    if (biddyVariableTable.table[0].prev != biddyVariableTable.size) {
      biddyVariableTable.table[biddyVariableTable.table[0].prev].next = v;
    }
    biddyVariableTable.table[v].prev = biddyVariableTable.table[0].prev;
    biddyVariableTable.table[0].prev = v;
    biddyVariableTable.table[v].next = 0;
  }
  else if ((biddyManagerType == BIDDYTYPEZBDDC) || (biddyManagerType == BIDDYTYPEZBDD) ||
           (biddyManagerType == BIDDYTYPEZFDDC) || (biddyManagerType == BIDDYTYPEZFDD) ||
           (biddyManagerType == BIDDYTYPETZBDDC) || (biddyManagerType == BIDDYTYPETZBDD) ||
           (biddyManagerType == BIDDYTYPETZFDDC) || (biddyManagerType == BIDDYTYPETZFDD))
  {
    /* all variables should already have correct prev and next value */
  }

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
  r = BiddyManagedTaggedFoaNode(MNG,v,biddyNull,biddyNull,v,FALSE); /* FALSE is important! */
  biddyVariableTable.table[v].firstNode = BiddyN(r);
  biddyNodeTable.table[0]->list = NULL; /* REPAIR CONSTANT NODE */

  /* FOR OBDD AND OFDD, NOW YOU HAVE A VARIABLE BUT AN ELEMENT IS ALSO CREATED */
  if ((biddyManagerType == BIDDYTYPEOBDDC) || (biddyManagerType == BIDDYTYPEOBDD) ||
      (biddyManagerType == BIDDYTYPEOFDDC) || (biddyManagerType == BIDDYTYPEOFDD))
  {
    /* BiddyRefresh(r); */
    biddyVariableTable.table[v].variable = r;
    el = biddyZero; /* for OBDDs and OFDDs, elements are not supported, yet */
    /* BiddyRefresh(el); */
    biddyVariableTable.table[v].element = el;
  }

  /* FOR ZBDD AND ZFDD, NOW YOU HAVE AN ELEMENT BUT A VARIABLE IS ALSO CREATED */
  /* THIS CALCULATION ASSUMES THAT NEW VARIABLE IS ADDED ABOVE ALL THE EXISTING ONES */
  else if ((biddyManagerType == BIDDYTYPEZBDDC) || (biddyManagerType == BIDDYTYPEZBDD) ||
            (biddyManagerType == BIDDYTYPEZFDDC) || (biddyManagerType == BIDDYTYPEZFDD))
  {
    /* BiddyRefresh(r); */
    biddyVariableTable.table[v].element = r;
    r = BiddyManagedTaggedFoaNode(MNG,v,biddyZero,biddyOne,0,TRUE);
    BiddyRefresh(r);
    biddyVariableTable.table[v].variable = r;
  }

  /* FOR TZBDD AND TZFDD, NOW YOU HAVE A VARIABLE BUT AN ELEMENT IS ALSO CREATED */
  else if ((biddyManagerType == BIDDYTYPETZBDDC) || (biddyManagerType == BIDDYTYPETZBDD) ||
            (biddyManagerType == BIDDYTYPETZFDDC) || (biddyManagerType == BIDDYTYPETZFDD))
  {
    /* BiddyRefresh(r); */
    biddyVariableTable.table[v].variable = r;
    el = biddyOne;
    Biddy_SetTag(el,biddyVariableTable.table[v].next);
    top = 0;
    for (k = 1; k < biddyVariableTable.num; k++) {
      top = biddyVariableTable.table[top].prev;
    }
    el = BiddyManagedTaggedFoaNode(MNG,v,biddyZero,el,top,TRUE);
    BiddyRefresh(el); /* NOT SURE IF THIS IS NEEDED */
    biddyVariableTable.table[v].element = el;
  }

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
    NOT IMPLEMENTED
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
    NOT IMPLEMENTED
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
    variables are swapped. Argument active is set to TRUE if some nodes
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
  Biddy_Variable var,minvar,maxvar,v;
  Biddy_Edge u,u0,u1;
  unsigned int n;
  unsigned int hash;

  if (!low || !high) return;

  assert( biddyVariableTable.table[low].next == high );
  assert( biddyVariableTable.table[high].prev == low );

  biddyNodeTable.swap++;

  /* DEBUGGING */
  /*
  printf("SWAPPING: low =  \"%s\", high = \"%s\"\n",
         Biddy_Managed_GetVariableName(MNG,low),
         Biddy_Managed_GetVariableName(MNG,high));
  */
 
  /* update active ordering */
  /* this is the same for all BDD types */
  /* IMPORTANT: For TZBDD it is important that variables are reordered before node manipulation! */
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

  *active = FALSE; /* swapping has not introduced brooken nodes, yet */
  newBegin = newEnd = NULL;
  minvar = 1;
  maxvar = biddyVariableTable.num-1;

  if ((biddyManagerType == BIDDYTYPEOBDDC) || (biddyManagerType == BIDDYTYPEOBDD) ||
      (biddyManagerType == BIDDYTYPEZBDDC) || (biddyManagerType == BIDDYTYPEZBDD))
  {
    /* ONLY NODES WITH var == low ARE AFFECTED */
    minvar = low;
    maxvar = low;
  }

  for (var = minvar; var <= maxvar; var++) {

    if ((biddyManagerType == BIDDYTYPEOBDDC) || (biddyManagerType == BIDDYTYPEOBDD) ||
        (biddyManagerType == BIDDYTYPEZBDDC) || (biddyManagerType == BIDDYTYPEZBDD))
    {

      sup = newBegin = biddyVariableTable.table[var].firstNode;

      /* PREPARE NEW LIST FOR 'LOW' */
      /* CONSTANT NODE "1" IS USED HERE BUT ONLY AS TMP */
      biddyVariableTable.table[var].lastNode->list = NULL;
      biddyVariableTable.table[var].firstNode =
        biddyVariableTable.table[var].lastNode = biddyNodeTable.table[0];

    }

    else if ((biddyManagerType == BIDDYTYPETZBDDC) || (biddyManagerType == BIDDYTYPETZBDD))
    {

      /* variables already reordered, now high < low !! */
      if (BiddyIsSmaller(var,high)) {
        biddyVariableTable.table[var].lastNode->list = NULL;
        sup = biddyVariableTable.table[var].firstNode;
      } else {
        sup = NULL;
      }
    }
    
    else if ((biddyManagerType == BIDDYTYPEOFDDC) || (biddyManagerType == BIDDYTYPEOFDD) ||
                (biddyManagerType == BIDDYTYPEZFDDC) || (biddyManagerType == BIDDYTYPEZFDD) ||
                (biddyManagerType == BIDDYTYPETZFDDC) || (biddyManagerType == BIDDYTYPETZFDD))
    {
      fprintf(stderr,"swapVariables: this GDD type is not supported, yet!\n");
      return;
    }
    
    else {
      fprintf(stderr,"swapVariables: Unsupported GDD type!\n");
      return;
    }

    while (sup) {

      assert( BiddyV(sup) == var );

      /* DEBUGGING */
      /*
      printf("NODE:");
      printf("<\"%s\",",Biddy_Managed_GetTopVariableName(MNG,sup));
      if (Biddy_GetMark(sup->f)) printf("*");
      printf("\"%s\",",Biddy_Managed_GetTopVariableName(MNG,sup->f));
      if (Biddy_GetMark(sup->t)) printf("*");
      printf("\"%s\">",Biddy_Managed_GetTopVariableName(MNG,sup->t));
      printf("@%u",sup->expiry);
      */

      if ((biddyManagerType == BIDDYTYPEOBDDC) || (biddyManagerType == BIDDYTYPEOBDD) ||
          (biddyManagerType == BIDDYTYPEZBDDC) || (biddyManagerType == BIDDYTYPEZBDD))
      {

        /* IF ((VARIABLE == low) AND (AT LEAST ONE SUCCESSOR == high)) THEN NODE IS AFFECTED */
        if ((BiddyV(sup->f) == high) || (BiddyV(sup->t) == high)) {

          /* DEBUGGING */
          /*
          printf("NODE-CHANGED:");
          printf("<\"%s\",",Biddy_Managed_GetTopVariableName(MNG,sup));
          if (Biddy_GetMark(sup->f)) printf("*");
          printf("\"%s\",",Biddy_Managed_GetTopVariableName(MNG,sup->f));
          if (Biddy_GetMark(sup->t)) printf("*");
          printf("\"%s\">",Biddy_Managed_GetTopVariableName(MNG,sup->t));
          printf("@%u",sup->expiry);
          */

          assert( (BiddyV(sup->f)==high)?TRUE:BiddyIsSmaller(low,BiddyV(sup->f)) );
          assert( (BiddyV(sup->t)==high)?TRUE:BiddyIsSmaller(low,BiddyV(sup->t)) );

          /* ONE SWAP for OBDD and ZBDD */
          /* this will add zero, one or two new nodes with variable 'low' */
          /* if FoaNode adds new node it will be added to new list for 'low' */
          /* if FoaNode returns existing node with label 'low', */
          /* then it will be relisted to new list for 'low' when it will be checked */
          /* (all existing variables are checked!) */

          oneSwap(MNG,sup,low,high,&u0,&u1);

          sup->v = high;
          sup->f = u0;
          sup->t = u1;

          (biddyVariableTable.table[low].num)--;
          (biddyVariableTable.table[high].num)++;

          *active = TRUE; /* swapping has introduced brooken nodes */

          /* DEBUGGING */
          /*
          printf("NODE-CREATED:");
          printf("<\"%s\",",Biddy_Managed_GetTopVariableName(MNG,sup));
          if (Biddy_GetMark(sup->f)) printf("*");
          printf("\"%s\",",Biddy_Managed_GetTopVariableName(MNG,sup->f));
          if (Biddy_GetMark(sup->t)) printf("*");
          printf("\"%s\">",Biddy_Managed_GetTopVariableName(MNG,sup->t));
          printf("@%u",sup->expiry);
          */

          /* rehash this node */
          sup->prev->next = sup->next; /* remove node from the old chain */
          if (sup->next) sup->next->prev = sup->prev; /* remove node from the old chain */
          hash = nodeTableHash(high,sup->f,sup->t,biddyNodeTable.size);
          tmp = biddyNodeTable.table[hash]; /* the beginning of new chain */
          tmp1 = findNodeTable(MNG,high,sup->f,sup->t,&tmp); /* to calculate tmp1, such node cannot exists! */
          addNodeTable(MNG,hash,sup,tmp1); /* add node to hash table */

          /* add this node to the list of renamed nodes (newBegin,newEnd) */
          newEnd = sup;

          sup = (BiddyNode *) sup->list;

        } else {

          /* DEBUGGING */
          /*
          printf("NODE-NOT-CHANGED:");
          printf("<\"%s\",",Biddy_Managed_GetTopVariableName(MNG,sup));
          if (Biddy_GetMark(BiddyE(sup))) printf("*");
          printf("\"%s\",",Biddy_Managed_GetTopVariableName(MNG,sup->f));
          if (Biddy_GetMark(BiddyT(sup))) printf("*");
          printf("\"%s\">",Biddy_Managed_GetTopVariableName(MNG,sup->t));
          printf("@%u",sup->expiry);
          */

          /* this node with variable 'low' has not been changed to 'high' */
          /* because it does not have successors with variable 'high' */

          assert( BiddyV(sup) == low );
          tmp = (BiddyNode *) sup->list;

          /* do not add this node to the list of renamed nodes (newBegin,newEnd) */
          if (!newEnd) {
            newBegin = (BiddyNode *) sup->list;
          } else {
            newEnd->list = sup->list;
          }

          /* relink to the new list for low */
          /* first node added to the new list for low is always a node representing variable */
          /* it is added to new list before any other node will be added */

          /* VARIANT A */
          /**/
          biddyVariableTable.table[low].lastNode->list = (void *) sup;
          biddyVariableTable.table[low].lastNode = sup;
          /**/

          /* VARIANT B */
          /*
          if (biddyVariableTable.table[low].firstNode == biddyVariableTable.table[low].lastNode) {
            biddyVariableTable.table[low].lastNode->list = (void *) sup;
            biddyVariableTable.table[low].lastNode = sup;
            if (biddyVariableTable.table[low].firstNode == biddyNodeTable.table[0]) {
              biddyVariableTable.table[low].firstNode = sup;
            }
          } else {
            sup->list = biddyVariableTable.table[low].firstNode->list;
            biddyVariableTable.table[low].firstNode->list = (void *) sup;
          }
          */

          sup = tmp;
        }

      }

      else if ((biddyManagerType == BIDDYTYPETZBDDC) || (biddyManagerType == BIDDYTYPETZBDD))
      {

        oneSwapEdge(MNG,sup->f,low,high,&u0,active);
        oneSwapEdge(MNG,sup->t,low,high,&u1,active);

        if ((sup->f != u0) || (sup->t != u1)) {
          /* this node is affected */

          /* DEBUGGING */
          /*
          if (BiddyV(sup) == 3) {
            printf("NODE-CHANGED:");
            printf("<\"%s\",",Biddy_Managed_GetTopVariableName(MNG,sup));
            if (!Biddy_IsNull(sup->f) && Biddy_GetMark(sup->f)) printf("*");
            if (Biddy_IsNull(sup->f)) printf("NULL,");
              else printf("<\"%s\">\"%s\",",Biddy_Managed_GetVariableName(MNG,Biddy_GetTag(sup->f)),Biddy_Managed_GetTopVariableName(MNG,sup->f));
            if (!Biddy_IsNull(sup->t) && Biddy_GetMark(sup->t)) printf("*");
            if (Biddy_IsNull(sup->t)) printf("NULL>");
              else printf("<\"%s\">\"%s\">",Biddy_Managed_GetVariableName(MNG,Biddy_GetTag(sup->t)),Biddy_Managed_GetTopVariableName(MNG,sup->t));
            printf("@%u",BiddyN(sup)->expiry);
            printf("\n");
          }
          */

          sup->f = u0;
          sup->t = u1;
          sup->prev->next = sup->next; /* remove node from the old chain */
          if (sup->next) sup->next->prev = sup->prev; /* remove node from the old chain */
          hash = nodeTableHash(sup->v,sup->f,sup->t,biddyNodeTable.size);
          tmp = biddyNodeTable.table[hash]; /* the beginning of new chain */
          tmp1 = findNodeTable(MNG,sup->v,sup->f,sup->t,&tmp); /* to calculate tmp1 */
          /* the same node may currently exist, but all the existing node will be changed before the end of loop */
          addNodeTable(MNG,hash,sup,tmp1); /* add node to hash table */
          /* if the same node currently exist then the new node is inserted before it */

          /* DEBUGGING */
          /*
          if (BiddyV(sup) == 3) {
            printf("NODE-CREATED:");
            printf("<\"%s\",",Biddy_Managed_GetTopVariableName(MNG,sup));
            if (!Biddy_IsNull(sup->f) && Biddy_GetMark(sup->f)) printf("*");
            if (Biddy_IsNull(sup->f)) printf("NULL,");
              else printf("<\"%s\">\"%s\",",Biddy_Managed_GetVariableName(MNG,Biddy_GetTag(sup->f)),Biddy_Managed_GetTopVariableName(MNG,sup->f));
            if (!Biddy_IsNull(sup->t) && Biddy_GetMark(sup->t)) printf("*");
            if (Biddy_IsNull(sup->t)) printf("NULL>");
              else printf("<\"%s\">\"%s\">",Biddy_Managed_GetVariableName(MNG,Biddy_GetTag(sup->t)),Biddy_Managed_GetTopVariableName(MNG,sup->t));
            printf("@%u",BiddyN(sup)->expiry);
            printf("\n");
          }
          */

        }
       
        sup = (BiddyNode *) sup->list;
      }

      else if ((biddyManagerType == BIDDYTYPEOFDDC) || (biddyManagerType == BIDDYTYPEOFDD) ||
                  (biddyManagerType == BIDDYTYPEZFDDC) || (biddyManagerType == BIDDYTYPEZFDD) ||
                  (biddyManagerType == BIDDYTYPETZFDDC) || (biddyManagerType == BIDDYTYPETZFDD))
      {
        fprintf(stderr,"swapVariables: this GDD type is not supported, yet!\n");
        return;
      }
      
      else {
        fprintf(stderr,"swapVariables: Unsupported GDD type!\n");
        return;
      }

    } /* while (sup) */
  } /* for (var) */

  /* FINAL CALCULATION */

  if ((biddyManagerType == BIDDYTYPEOBDDC) || (biddyManagerType == BIDDYTYPEOBDD) ||
      (biddyManagerType == BIDDYTYPEZBDDC) || (biddyManagerType == BIDDYTYPEZBDD))
  {

    /* all nodes in the list from newBegin to newEnd (inclusively) are renamed */
    /* thus insert the complete list into list for 'high' */
    if (newEnd) {

      /* DEBUGGING */
      /*
      {
      BiddyNode *sup;
      sup = newBegin;
      printf("LIST OF RENAMED NODES\n");
      while (sup) {
        printf("<\"%s\",",Biddy_Managed_GetTopVariableName(MNG,sup));
        if (Biddy_GetMark(sup->f)) printf("*");
        printf("\"%s\",",Biddy_Managed_GetTopVariableName(MNG,sup->f));
        if (Biddy_GetMark(sup->t)) printf("*");
        printf("\"%s\">",Biddy_Managed_GetTopVariableName(MNG,sup->t));
        printf("@%u",sup->expiry);
        if (sup == newEnd) {
          sup = NULL;
        } else {
          sup = (BiddyNode *) sup->list;
        }
      }
      printf("\n");
      }
      */

      (biddyVariableTable.table[high].lastNode)->list = (void *) newBegin;
      biddyVariableTable.table[high].lastNode = newEnd;
    }

    /* at least one new node has been added to new list for 'low' */
    /* e.g. node representing variable is never renamed to 'high' */

    /* use this with variant A, because the first element is still a temporary one */
    /**/
    biddyVariableTable.table[low].firstNode =
      (BiddyNode *) (biddyVariableTable.table[low].firstNode)->list;
    /**/

    /* REPAIR CONSTANT NODE */
    biddyNodeTable.table[0]->list = NULL;

  }

  else if ((biddyManagerType == BIDDYTYPETZBDDC) || (biddyManagerType == BIDDYTYPETZBDD))
  {

    /* WE HAVE TO CHECK ALL NON-CONSTANT FORMULAE AS WELL */
    /* for TZBDD, first two formulae are constants */
    for (n = 2; n < biddyFormulaTable.size; n++) {
      if (!Biddy_IsNull(biddyFormulaTable.table[n].f) &&
          !Biddy_IsConstant(biddyFormulaTable.table[n].f))
      {
        oneSwapEdge(MNG,biddyFormulaTable.table[n].f,low,high,&u,active);
        biddyFormulaTable.table[n].f = u;
      }
    } 
  }

  else if ((biddyManagerType == BIDDYTYPEOFDDC) || (biddyManagerType == BIDDYTYPEOFDD) ||
              (biddyManagerType == BIDDYTYPEZFDDC) || (biddyManagerType == BIDDYTYPEZFDD) ||
              (biddyManagerType == BIDDYTYPETZFDDC) || (biddyManagerType == BIDDYTYPETZFDD))
  {
    fprintf(stderr,"swapVariables: this GDD type is not supported, yet!\n");
    return;
  }
  
  else {
    fprintf(stderr,"swapVariables: Unsupported GDD type!\n");
    return;
  }

  /* REPAIR BROKEN NODES */
  /* this is need only if swaps have been made */
  /* FORMULAE CAN BE REPAIRED BECAUSE THEY ARE ALL COLLECTED IN FORMULA TABLE */
  /* FRESH NODES FROM CURRENT CALCULATION ARE NOT COLLECTED AND THUS THEY CANNOT BE REPAIRED! */
  /* first formula is 0, it is always a single terminal node */
  /* second formula is 1, it could be a large graph, e.g. for ZBDD */
  if (*active) {
    for (v = 1; v < biddyVariableTable.num; v++) {
      BiddyRefresh(biddyVariableTable.table[v].variable);
      BiddyRefresh(biddyVariableTable.table[v].element);
    }
    for (n = 1; n < biddyFormulaTable.size; n++) {
      if (!Biddy_IsNull(biddyFormulaTable.table[n].f) &&
          !Biddy_IsConstant(biddyFormulaTable.table[n].f))
      {
        if ((biddyManagerType == BIDDYTYPEOBDDC) || (biddyManagerType == BIDDYTYPEOBDD) ||
            (biddyManagerType == BIDDYTYPEOFDDC) || (biddyManagerType == BIDDYTYPEOFDD))
        {
          if ((BiddyE(biddyFormulaTable.table[n].f) == biddyZero) && (BiddyT(biddyFormulaTable.table[n].f) == biddyTerminal)) {
            /* variable/element already refreshed */
          } else {
            BiddyProlongOne(biddyFormulaTable.table[n].f,biddyFormulaTable.table[n].expiry);
          }
        }
        else if ((biddyManagerType == BIDDYTYPEZBDDC) || (biddyManagerType == BIDDYTYPEZBDD) ||
            (biddyManagerType == BIDDYTYPEZFDDC) || (biddyManagerType == BIDDYTYPEZFDD))
        {
          if ((BiddyE(biddyFormulaTable.table[n].f) == biddyTerminal) && (BiddyT(biddyFormulaTable.table[n].f) == biddyTerminal)) {
            /* variable/element already refreshed */
          } else {
            BiddyProlongOne(biddyFormulaTable.table[n].f,biddyFormulaTable.table[n].expiry);
          }
        }
        else if ((biddyManagerType == BIDDYTYPETZBDDC) || (biddyManagerType == BIDDYTYPETZBDD) ||
            (biddyManagerType == BIDDYTYPETZFDDC) || (biddyManagerType == BIDDYTYPETZFDD))
        {
          if ((BiddyE(biddyFormulaTable.table[n].f) == biddyZero) && (BiddyT(biddyFormulaTable.table[n].f) == biddyTerminal)) {
            /* variable/element already refreshed */
          } else {
            BiddyProlongOne(biddyFormulaTable.table[n].f,biddyFormulaTable.table[n].expiry);
          }
        }
        else if ((biddyManagerType == BIDDYTYPEOFDDC) || (biddyManagerType == BIDDYTYPEOFDD) ||
                    (biddyManagerType == BIDDYTYPEZFDDC) || (biddyManagerType == BIDDYTYPEZFDD) ||
                    (biddyManagerType == BIDDYTYPETZFDDC) || (biddyManagerType == BIDDYTYPETZFDD))
        {
          fprintf(stderr,"swapVariables: this GDD type is not supported, yet!\n");
          return;
        }
        else {
          fprintf(stderr,"swapVariables: Unsupported GDD type!\n");
          return;
        }
      }
    }
  }

  /* DEBUGGING */
  /*
  printf("SWAPPING FINISH: low =  \"%s\", high = \"%s\"\n",
         Biddy_Managed_GetVariableName(MNG,low),
         Biddy_Managed_GetVariableName(MNG,high));
  for (n = 1; n < biddyFormulaTable.size; n++) {
    if (!Biddy_IsNull(biddyFormulaTable.table[n].f) &&
        !Biddy_IsConstant(biddyFormulaTable.table[n].f))
    {
      assert( BiddyIsOK(biddyFormulaTable.table[n].f) );
    }
  }
  */
}

/*******************************************************************************
\brief Function oneSwap calculates u0 and u1 which are needed within variable
        swapping.

### Description
### Side effects
    It is assumed, that the given node is affected by swapping low and high.
### More info
*******************************************************************************/

static void
oneSwap(Biddy_Manager MNG, BiddyNode *sup, Biddy_Variable low,
        Biddy_Variable high, Biddy_Edge *u0, Biddy_Edge *u1)
{
  Biddy_Edge f00,f01,f10,f11;

  /* DEBUGGING */
  /*
  {
  BiddyNode *sup1;
  printf("ONESWAP:");
  printf("SUP<\"%s\"",Biddy_Managed_GetTopVariableName(MNG,sup));
  printf("@%u",sup->expiry);
  if (sup->f == sup->t) printf("DOUBLE");
  printf("ELSE");
  if (Biddy_GetMark(sup->f)) printf("*");
  sup1 = (BiddyNode *) BiddyP(sup->f);
  printf("<<\"%s\">\"%s\",",Biddy_Managed_GetVariableName(MNG,Biddy_GetTag(sup->f)),Biddy_Managed_GetTopVariableName(MNG,sup1));
  if (!Biddy_IsNull(sup1->f) && Biddy_GetMark(sup1->f)) printf("*");
  if (Biddy_IsNull(sup1->f)) printf("NULL,");
    else printf("<\"%s\">\"%s\",",Biddy_Managed_GetVariableName(MNG,Biddy_GetTag(sup1->f)),Biddy_Managed_GetTopVariableName(MNG,sup1->f));
  if (!Biddy_IsNull(sup1->t) && Biddy_GetMark(sup1->t)) printf("*");
  if (Biddy_IsNull(sup1->t)) printf("NULL>");
    else printf("<\"%s\">\"%s\">",Biddy_Managed_GetVariableName(MNG,Biddy_GetTag(sup1->t)),Biddy_Managed_GetTopVariableName(MNG,sup1->t));
  printf("@%u",sup1->expiry);
  if (sup1->f == sup1->t) printf("DOUBLE");
  printf("THEN");
  if (Biddy_GetMark(sup->t)) printf("*");
  sup1 = (BiddyNode *) BiddyP(sup->t);
  printf("<<\"%s\">\"%s\",",Biddy_Managed_GetVariableName(MNG,Biddy_GetTag(sup->t)),Biddy_Managed_GetTopVariableName(MNG,sup1));
  if (!Biddy_IsNull(sup1->f) && Biddy_GetMark(sup1->f)) printf("*");
  if (Biddy_IsNull(sup1->f)) printf("NULL,");
    else printf("<\"%s\">\"%s\",",Biddy_Managed_GetVariableName(MNG,Biddy_GetTag(sup1->f)),Biddy_Managed_GetTopVariableName(MNG,sup1->f));
  if (!Biddy_IsNull(sup1->t) && Biddy_GetMark(sup1->t)) printf("*");
  if (Biddy_IsNull(sup1->t)) printf("NULL>");
    else printf("<\"%s\">\"%s\">",Biddy_Managed_GetVariableName(MNG,Biddy_GetTag(sup1->t)),Biddy_Managed_GetTopVariableName(MNG,sup1->t));
  printf("@%u",sup1->expiry);
  if (sup1->f == sup1->t) printf("DOUBLE");
  printf("\n");
  }
  */

  /* for OBDD and ZBDD: */
  /* this will add zero, one or two new nodes with variable 'low' */
  /* if FoaNode adds new node it will be added to new list for 'low' */
  /* if FoaNode returns existing node with label 'low', */
  /* then it will be relisted to new list for 'low' when it will be checked */
  /* (all existing variables are checked!) */

  /* for TZBDD: */
  /* this will add zero, one or two new nodes with variable 'low' */
  /* and zero or one new node with variable high */

  if (BiddyV(sup->f) == high) {

    /* FOR OBDDC, sup->f CAN BE MARKED, MARK IS TRANSFERED TO BOTH SUCCESSORS */
    /* FOR ZBDDC, sup->f CANNOT BE MARKED */
    /* FOR OBDD, ZBDD, and TZBDD, sup->f CAN BE MARKED BUT MARK IS NOT TRANSFERED TO SUCCESSORS */

    f00 = BiddyE(sup->f);
    f01 = BiddyT(sup->f);

    if (biddyManagerType == BIDDYTYPEOBDDC)
    {
      if (Biddy_GetMark(sup->f)) {
        Biddy_InvertMark(f00);
        Biddy_InvertMark(f01);
      }
    }

    assert( BiddyIsSmaller(low,BiddyV(f00)) );
    assert( BiddyIsSmaller(low,BiddyV(f01)) );

    /* sup->f is not needed in sup, anymore */
    /* we will mark it as obsolete and we are expecting, that */
    /* formulae are repaired before garbage collection */
    BiddyN(sup->f)->expiry = 1;

    /* DEBUGGING */
    /*
    {
    BiddyNode *sup1;
    sup1 = (BiddyNode *) BiddyP(sup->f);
    printf("NODE-OBSOLETE:");
    printf("<\"%s\",",Biddy_Managed_GetTopVariableName(MNG,sup1));
    if (Biddy_GetMark(sup1->f)) printf("*");
    printf("\"%s\",",Biddy_Managed_GetTopVariableName(MNG,sup1->f));
    if (Biddy_GetMark(sup1->t)) printf("*");
    printf("\"%s\">",Biddy_Managed_GetTopVariableName(MNG,sup1->t));
    printf("@%u",sup1->expiry);
    }
    */

  } else {

    if ((biddyManagerType == BIDDYTYPEOBDDC) || (biddyManagerType == BIDDYTYPEOBDD)) {
      f00 = f01 = sup->f;
    }
    else if ((biddyManagerType == BIDDYTYPEZBDDC) || (biddyManagerType == BIDDYTYPEZBDD)) {
      f00 = sup->f;
      f01 = biddyZero;
    }
    else if ((biddyManagerType == BIDDYTYPETZBDDC) || (biddyManagerType == BIDDYTYPETZBDD)) {
      if (Biddy_GetTag(sup->f) == high) {
        f00 = sup->f;
        /* ordering already changed and now low is higher */
        Biddy_SetTag(f00,low);
        f00 = Biddy_Managed_IncTag(MNG,f00);
        if (BiddyN(sup->f) != BiddyN(f00)) {
          /* sup->f is not needed in sup, anymore */
          /* we will mark it as obsolete and we are expecting, that */
          /* formulae are repaired before garbage collection */
          BiddyN(sup->f)->expiry = 1;
        }
        f01 = biddyZero;
      } else {
        f00 = f01 = sup->f;
      }
    }
    else if ((biddyManagerType == BIDDYTYPEOFDDC) || (biddyManagerType == BIDDYTYPEOFDD) ||
                (biddyManagerType == BIDDYTYPEZFDDC) || (biddyManagerType == BIDDYTYPEZFDD) ||
                (biddyManagerType == BIDDYTYPETZFDDC) || (biddyManagerType == BIDDYTYPETZFDD))
    {
      fprintf(stderr,"oneSwap: this GDD type is not supported, yet!\n");
      f00 = f01 = biddyNull;
      return;
    }
    else {
      fprintf(stderr,"oneSwap: Unsupported GDD type!\n");
      f00 = f01 = biddyNull;
      return;
    }
  }

  if (BiddyV(sup->t) == high) {

    /* FOR OBDDC, sup->t CANNOT BE MARKED */
    /* FOR ZBDDC, sup->t CAN BE MARKED, MARK IS TRANSFERED TO LEFT SUCCESSOR, ONLY */
    /* FOR OBDD, ZBDD, and TZBDD, sup->f CAN BE MARKED BUT MARK IS NOT TRANSFERED TO SUCCESSORS */

    f10 = BiddyE(sup->t);
    f11 = BiddyT(sup->t);

    if (biddyManagerType == BIDDYTYPEZBDDC)
    {
      if (Biddy_GetMark(sup->t)) {
        Biddy_InvertMark(f10);
      }
    }

    assert( BiddyIsSmaller(low,BiddyV(f10)) );
    assert( BiddyIsSmaller(low,BiddyV(f11)) );

    /* sup->t is not needed in sup, anymore */
    /* we will mark it as obsolete and we are expecting, that */
    /* formulae are repaired before garbage collection */
    BiddyN(sup->t)->expiry = 1;

    /* DEBUGGING */
    /*
    {
    BiddyNode *sup1;
    sup1 = (BiddyNode *) BiddyP(sup->t);
    printf("NODE-OBSOLETE:");
    printf("<\"%s\",",Biddy_Managed_GetTopVariableName(MNG,sup1));
    if (Biddy_GetMark(sup1->f)) printf("*");
    printf("\"%s\",",Biddy_Managed_GetTopVariableName(MNG,sup1->f));
    if (Biddy_GetMark(sup1->t)) printf("*");
    printf("\"%s\">",Biddy_Managed_GetTopVariableName(MNG,sup1->t));
    printf("@%u",sup1->expiry);
    }
    */

  } else {

    if ((biddyManagerType == BIDDYTYPEOBDDC) || (biddyManagerType == BIDDYTYPEOBDD))
    {
      f10 = f11 = sup->t;
    }
    else if ((biddyManagerType == BIDDYTYPEZBDDC) || (biddyManagerType == BIDDYTYPEZBDD)) {
      f10 = sup->t;
      f11 = biddyZero;
    }
    else if ((biddyManagerType == BIDDYTYPETZBDDC) || (biddyManagerType == BIDDYTYPETZBDD)) {
      if (Biddy_GetTag(sup->t) == high) {
        f10 = sup->t;
        /* ordering already changed and now low is higher */
        Biddy_SetTag(f10,low);
        f10 = Biddy_Managed_IncTag(MNG,f10);
        if (BiddyN(sup->t) != BiddyN(f10)) {
          /* sup->t is not needed in sup, anymore */
          /* we will mark it as obsolete and we are expecting, that */
          /* formulae are repaired before garbage collection */
          BiddyN(sup->t)->expiry = 1;
        }
        f11 = biddyZero;
      } else {
        f10 = f11 = sup->t;
      }
    }
    else if ((biddyManagerType == BIDDYTYPEOFDDC) || (biddyManagerType == BIDDYTYPEOFDD) ||
                (biddyManagerType == BIDDYTYPEZFDDC) || (biddyManagerType == BIDDYTYPEZFDD) ||
                (biddyManagerType == BIDDYTYPETZFDDC) || (biddyManagerType == BIDDYTYPETZFDD))
    {
      fprintf(stderr,"oneSwap: this GDD type is not supported, yet!\n");
      f00 = f01 = biddyNull;
      return;
    }
    else {
      fprintf(stderr,"oneSwap: Unsupported GDD type!\n");
      f00 = f01 = biddyNull;
      return;
    }
  }

  *u0 = *u1 = biddyNull;
  if ((biddyManagerType == BIDDYTYPEOBDDC) || (biddyManagerType == BIDDYTYPEOBDD))
  {

    if (f00 == f10) {
      *u0 = f00;
    } else {
      /* u0 is a new node or an existing node */
      /* in any case, it is a needed node */
      /* here, GC will not be called during FoaNode */
      *u0 = BiddyManagedTaggedFoaNode(MNG,low,f00,f10,low,FALSE);
    }

    if (f01 == f11) {
      *u1 = f01;
    } else {
      /* u1 is a new node or an existing node */
      /* in any case, it is a needed node */
      /* here, GC will not be called during FoaNode */
      *u1 = BiddyManagedTaggedFoaNode(MNG,low,f01,f11,low,FALSE);
    }

  }

  else if ((biddyManagerType == BIDDYTYPEZBDDC) || (biddyManagerType == BIDDYTYPEZBDD))
  {

    if (f10 == biddyZero) {
      *u0 = f00;
    } else {
      /* u0 is a new node or an existing node */
      /* in any case, it is a needed node */
      /* here, GC will not be called during FoaNode */
      *u0 = BiddyManagedTaggedFoaNode(MNG,low,f00,f10,low,FALSE);
    }

    if (f11 == biddyZero) {
      *u1 = f01;
    } else {
      /* u1 is a new node or an existing node */
      /* in any case, it is a needed node */
      /* here, GC will not be called during FoaNode */
      *u1 = BiddyManagedTaggedFoaNode(MNG,low,f01,f11,low,FALSE);
    }

  }

  else if ((biddyManagerType == BIDDYTYPETZBDDC) || (biddyManagerType == BIDDYTYPETZBDD))
  {

    /* u0 is a new node or an existing node */
    /* it is not always needed, sometimes it is reduced in further calculation */
    /* here, GC will not be called during FoaNode */
    *u0 = BiddyManagedTaggedFoaNode(MNG,low,f00,f10,low,FALSE);

    /* u1 is a new node or an existing node */
    /* it is not always needed, sometimes it is reduced in further calculation */
    /* in any case, it is a needed node */
    /* here, GC will not be called during FoaNode */
    *u1 = BiddyManagedTaggedFoaNode(MNG,low,f01,f11,low,FALSE);

  }

  else if ((biddyManagerType == BIDDYTYPEOFDDC) || (biddyManagerType == BIDDYTYPEOFDD) ||
              (biddyManagerType == BIDDYTYPEZFDDC) || (biddyManagerType == BIDDYTYPEZFDD) ||
              (biddyManagerType == BIDDYTYPETZFDDC) || (biddyManagerType == BIDDYTYPETZFDD))
  {
    fprintf(stderr,"oneSwap: this GDD type is not supported, yet!\n");
    f00 = f01 = biddyNull;
    return;
  }

  else {
    fprintf(stderr,"oneSwap: Unsupported GDD type!\n");
    f00 = f01 = biddyNull;
    return;
  }

}

/*******************************************************************************
\brief Function oneSwapEdge calculates u which is needed within variable
        swapping for TZBDDs, only.

### Description
### Side effects
### More info
*******************************************************************************/

static void
oneSwapEdge(Biddy_Manager MNG, Biddy_Edge sup, Biddy_Variable low,
            Biddy_Variable high, Biddy_Edge *u, Biddy_Boolean *active)
{
  Biddy_Edge u0,u1;

  assert( (biddyManagerType == BIDDYTYPETZBDDC) || (biddyManagerType == BIDDYTYPETZBDD) );

  *u = sup;

  if (BiddyV(sup) == low) {

    /* ONE SWAP for TZBDD */
    /* this will add zero, one or two new nodes with variable 'low' */
    /* and zero or one new node with variable high */

    oneSwap(MNG,BiddyN(sup),low,high,&u0,&u1);

    if (Biddy_GetTag(sup) == low) {
       *u = BiddyManagedTaggedFoaNode(MNG,high,u0,u1,high,FALSE);
    } else {
       *u = BiddyManagedTaggedFoaNode(MNG,high,u0,u1,Biddy_GetTag(sup),FALSE);
    }

    /* node pointed by sup is not needed, anymore */
    /* we will mark it as obsolete and we are expecting, that */
    /* formulae are repaired before garbage collection */
    BiddyN(sup)->expiry = 1;

    *active = TRUE; /* swapping has introduced brooken nodes */

    /* DEBUGGING */
    /*
    printf("EDGE-CREATED:");
    printf("<<\"%s\">\"%s\",",Biddy_Managed_GetVariableName(MNG,Biddy_GetTag(*u)),Biddy_Managed_GetTopVariableName(MNG,*u));
    if (Biddy_GetMark(BiddyE(*u))) printf("*");
    printf("\"%s\",",Biddy_Managed_GetTopVariableName(MNG,BiddyE(*u)));
    if (Biddy_GetMark(BiddyT(*u))) printf("*");
    printf("\"%s\">",Biddy_Managed_GetTopVariableName(MNG,BiddyT(*u)));
    printf("@%u",BiddyN(*u)->expiry);
    */

  } else if (((Biddy_GetTag(sup) == low) || BiddyIsSmaller(Biddy_GetTag(sup),high)) && (BiddyV(sup) == high)) {
    /* variables already reordered, now high < low !! */

    u0 = BiddyManagedTaggedFoaNode(MNG,low,BiddyE(sup),biddyZero,low,FALSE);
    u1 = BiddyManagedTaggedFoaNode(MNG,low,BiddyT(sup),biddyZero,low,FALSE);

    if (Biddy_GetTag(sup) == low) {
       *u = BiddyManagedTaggedFoaNode(MNG,high,u0,u1,high,FALSE);
    } else {
       *u = BiddyManagedTaggedFoaNode(MNG,high,u0,u1,Biddy_GetTag(sup),FALSE);
    }

    /* node pointed by sup is not needed, anymore */
    /* we will mark it as obsolete and we are expecting, that */
    /* formulae are repaired before garbage collection */
    BiddyN(sup)->expiry = 1;

    *active = TRUE; /* swapping has introduced brooken nodes */

    /* DEBUGGING */
    /*
    printf("EDGE-CREATED:");
    printf("<<\"%s\">\"%s\",",Biddy_Managed_GetVariableName(MNG,Biddy_GetTag(*u)),Biddy_Managed_GetTopVariableName(MNG,*u));
    if (Biddy_GetMark(BiddyE(*u))) printf("*");
    printf("\"%s\",",Biddy_Managed_GetTopVariableName(MNG,BiddyE(*u)));
    if (Biddy_GetMark(BiddyT(*u))) printf("*");
    printf("\"%s\">",Biddy_Managed_GetTopVariableName(MNG,BiddyT(*u)));
    printf("@%u",BiddyN(*u)->expiry);
    */

  } else if ((Biddy_GetTag(sup) == high) && BiddyIsSmaller(low,BiddyV(sup))) {

    /* variables already reordered, now high < low !! */
    Biddy_SetTag(*u,low);
    *u = Biddy_Managed_IncTag(MNG,*u);
    if (BiddyN(sup) != BiddyN(*u)) {
      /* old sup is not needed, anymore */
      /* we will mark it as obsolete and we are expecting, that */
      /* formulae are repaired before garbage collection */
      BiddyN(sup)->expiry = 1;
    }
    *u = BiddyManagedTaggedFoaNode(MNG,low,*u,*u,high,FALSE);

  } else if (Biddy_GetTag(sup) == low) {
    /* NOW, TARGET NODE IS CHANGED IF NECESSARY, FOR OTHERS TAG MAY NEED A CHANGE */

    /* CHANGE TAG TO high - THIS CAN BE DONE WITHOUT ANY CHECKING */
    Biddy_SetTag(*u,high);

  }

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

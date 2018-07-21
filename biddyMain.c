/***************************************************************************//*!
\file biddyMain.c
\brief File biddyMain.c contains main functions for representation and
manipulation of boolean functions with various types of Binary Decision
Diagrams.

### Description

    PackageName [Biddy]
    Synopsis    [Biddy provides data structures and algorithms for the
                 representation and manipulation of Boolean functions with
                 ROBDDs, 0-sup-BDDs, and TZBDDs. A hash table is used for quick
                 search of nodes. Complement edges decreases the number of
                 nodes. An automatic garbage collection with a system age is
                 implemented. Variable swapping and sifting are implemented.]

    FileName    [biddyMain.c]
    Revision    [$Revision: 458 $]
    Date        [$Date: 2018-07-18 13:53:04 +0200 (sre, 18 jul 2018) $]
    Authors     [Robert Meolic (robert.meolic@um.si)]

### Copyright

Copyright (C) 2006, 2018 UM FERI, Koroska cesta 46, SI-2000 Maribor, Slovenia

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

/* Biddy_Edge debug_edge = NULL; */ /* debugging, only */
/* Biddy_Boolean biddySiftingActive = FALSE; */ /* debugging, only */

/*----------------------------------------------------------------------------*/
/* Static function prototypes                                                 */
/*----------------------------------------------------------------------------*/

static Biddy_Boolean isEqv(Biddy_Manager MNG1, Biddy_Edge f1, Biddy_Manager MNG2, Biddy_Edge f2);
static BiddyLocalInfo * createLocalInfo(Biddy_Manager MNG, Biddy_Edge f, BiddyLocalInfo *c);
static void deleteLocalInfo(Biddy_Manager MNG, Biddy_Edge f);
static inline unsigned int nodeTableHash(Biddy_Variable v, Biddy_Edge pf, Biddy_Edge pt, unsigned int size);
static inline void addNodeTable(Biddy_Manager MNG, unsigned int hash, BiddyNode *node, BiddyNode *sup1);
static inline BiddyNode *findNodeTable(Biddy_Manager MNG, Biddy_Variable v, Biddy_Edge pf, Biddy_Edge pt, BiddyNode **thesup);
static Biddy_Edge addVariableElement(Biddy_Manager MNG, Biddy_String x);
static void evalProbability(Biddy_Manager MNG, Biddy_Edge f,  double *c1, double *c0, Biddy_Boolean *leftmost);
static Biddy_Boolean checkFunctionOrdering(Biddy_Manager MNG, Biddy_Edge f);
static Biddy_Variable getGlobalOrdering(Biddy_Manager MNG, Biddy_Variable v);
static Biddy_Variable swapWithHigher(Biddy_Manager MNG, Biddy_Variable v, Biddy_Boolean *active);
static Biddy_Variable swapWithLower(Biddy_Manager MNG, Biddy_Variable v, Biddy_Boolean *active);
static void swapVariables(Biddy_Manager MNG, Biddy_Variable low, Biddy_Variable high, Biddy_Boolean *active);
static void oneSwap(Biddy_Manager MNG, BiddyNode *sup, Biddy_Variable low, Biddy_Variable high, Biddy_Edge *u0, Biddy_Edge *u1);
static void oneSwapEdge(Biddy_Manager MNG, Biddy_Edge sup, Biddy_Variable low, Biddy_Variable high, Biddy_Edge *u, Biddy_Boolean *active);
static void nullOrdering(BiddyOrderingTable table);
static void alphabeticOrdering(Biddy_Manager MNG, BiddyOrderingTable table);
static void nodeNumberOrdering(Biddy_Manager MNG, Biddy_Edge f, unsigned int *i, BiddyOrderingTable ordering);
static void reorderVariables(Biddy_Manager MNG, int numV, BiddyVariableOrder *tableV, int numN, BiddyNodeList *tableN, int id);
static void constructBDD(Biddy_Manager MNG, int numN, BiddyNodeList *tableN, int id);
static void warshall(BiddyOrderingTable table, Biddy_Variable varnum);
static void writeBDD(Biddy_Manager MNG, Biddy_Edge f);
static void writeORDER(Biddy_Manager MNG);
static void debugNode(Biddy_Manager MNG, BiddyNode *node);
static void debugEdge(Biddy_Manager MNG, Biddy_Edge edge);
static void debugORDERING(Biddy_Manager MNG, BiddyOrderingTable table, Biddy_Variable varnum);

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
    (biddyCacheList). Biddy_InitMNG also initializes constant edges
    (biddyOne, biddyZero), memory management and automatic garbage collection.
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
Biddy_InitMNG(Biddy_Manager *mng, int bddtype)
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
  printf("DEBUG (Biddy_InitMNG): bddtype = %d\n",bddtype);
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
  biddyNodeTable.convergesiftingtreshold = 0.0;
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
  if (!(biddyOPCache.notusedyet = (Biddy_Boolean *) malloc(sizeof(Biddy_Boolean)))) {
    fprintf(stderr, "Biddy_InitMNG: Out of memoy!\n");
    exit(1);
  }
  *(biddyOPCache.notusedyet) = FALSE;
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
  if (!(biddyEACache.notusedyet = (Biddy_Boolean *) malloc(sizeof(Biddy_Boolean)))) {
    fprintf(stderr, "Biddy_InitMNG: Out of memoy!\n");
    exit(1);
  }
  *(biddyEACache.notusedyet) = FALSE;
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
  if (!(biddyRCCache.notusedyet = (Biddy_Boolean *) malloc(sizeof(Biddy_Boolean)))) {
    fprintf(stderr, "Biddy_InitMNG: Out of memoy!\n");
    exit(1);
  }
  *(biddyRCCache.notusedyet) = FALSE;
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
  biddyReplaceCache.keyNum = 0;
  biddyReplaceCache.keywordNum = 0;
  biddyReplaceCache.disabled = FALSE;
  if (!(biddyReplaceCache.notusedyet = (Biddy_Boolean *) malloc(sizeof(Biddy_Boolean)))) {
    fprintf(stderr, "Biddy_InitMNG: Out of memoy!\n");
    exit(1);
  }
  *(biddyReplaceCache.notusedyet) = FALSE;
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
  if (bddtype == BIDDYTYPEOBDD) {
    MNG[0] = strdup(BIDDYTYPENAMEOBDD); /* biddyManagerName */
  } else if (bddtype == BIDDYTYPEOBDDC) {
    MNG[0] = strdup(BIDDYTYPENAMEOBDDC); /* biddyManagerName */
  } else if (bddtype == BIDDYTYPEZBDD) {
    MNG[0] = strdup(BIDDYTYPENAMEZBDD); /* biddyManagerName */
  } else if (bddtype == BIDDYTYPEZBDDC) {
    MNG[0] = strdup(BIDDYTYPENAMEZBDDC); /* biddyManagerName */
  } else if (bddtype == BIDDYTYPETZBDD) {
#if UINTPTR_MAX == 0xffffffffffffffff
#else
    fprintf(stderr,"Biddy_InitMNG: TZBDDs are supported on 64-bit architecture, only!\n");
    exit(1);
#endif
    MNG[0] = strdup(BIDDYTYPENAMETZBDD); /* biddyManagerName */
  } else if (bddtype == BIDDYTYPETZBDDC) {
#if UINTPTR_MAX == 0xffffffffffffffff
#else
    fprintf(stderr,"Biddy_InitMNG: TZBDDs are supported on 64-bit architecture, only!\n");
    exit(1);
#endif
    MNG[0] = strdup(BIDDYTYPENAMETZBDDC); /* biddyManagerName */
    fprintf(stderr,"Biddy_InitMNG: %s is not supported, yet!\n",BIDDYTYPENAMETZBDDC);
    exit(1);
  } else if (bddtype == BIDDYTYPEOFDD) {
    MNG[0] = strdup(BIDDYTYPENAMEOFDD); /* biddyManagerName */
    fprintf(stderr,"Biddy_InitMNG: %s is not supported, yet!\n",BIDDYTYPENAMEOFDD);
    exit(1);
  } else if (bddtype == BIDDYTYPEOFDDC) {
    MNG[0] = strdup(BIDDYTYPENAMEOFDDC); /* biddyManagerName */
    fprintf(stderr,"Biddy_InitMNG: %s is not supported, yet!\n",BIDDYTYPENAMEOFDDC);
    exit(1);
  } else if (bddtype == BIDDYTYPEZFDD) {
    MNG[0] = strdup(BIDDYTYPENAMEZFDD); /* biddyManagerName */
    fprintf(stderr,"Biddy_InitMNG: %s is not supported, yet!\n",BIDDYTYPENAMEZFDD);
    exit(1);
  } else if (bddtype == BIDDYTYPEZFDDC) {
    MNG[0] = strdup(BIDDYTYPENAMEZFDDC); /* biddyManagerName */
    fprintf(stderr,"Biddy_InitMNG: %s is not supported, yet!\n",BIDDYTYPENAMEZFDDC);
    exit(1);
  } else if (bddtype == BIDDYTYPETZFDD) {
#if UINTPTR_MAX == 0xffffffffffffffff
#else
    fprintf(stderr,"Biddy_InitMNG: TZFDDs are supported on 64-bit architecture, only!\n");
    exit(1);
#endif
    MNG[0] = strdup(BIDDYTYPENAMETZFDD); /* biddyManagerName */
    fprintf(stderr,"Biddy_InitMNG: %s is not supported, yet!\n",BIDDYTYPENAMETZFDD);
    exit(1);
  } else if (bddtype == BIDDYTYPETZFDDC) {
#if UINTPTR_MAX == 0xffffffffffffffff
#else
    fprintf(stderr,"Biddy_InitMNG: TZFDDs are supported on 64-bit architecture, only!\n");
    exit(1);
#endif
    MNG[0] = strdup(BIDDYTYPENAMETZFDDC); /* biddyManagerName */
    fprintf(stderr,"Biddy_InitMNG: %s is not supported, yet!\n",BIDDYTYPENAMETZFDDC);
    exit(1);
  } else {
    fprintf(stderr,"Biddy_InitMNG: Unsupported BDD type!\n");
    exit(1);
  }

  biddyManagerType = bddtype;

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

  /* USED FOR LARGER PROBLEMS, E.G. DICTIONARY EXAMPLE */
  /*
  biddyOPCache.size = LARGE_SIZE;
  */

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
  biddyNodeTable.convergesiftingtreshold = (float) 1.01;  /* stop one step of converging sifting if the size of the system grows to much */

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
  biddyVariableTable.table[0].num = 1;
  biddyVariableTable.table[0].firstNode = NULL;
  biddyVariableTable.table[0].lastNode = NULL;
  biddyVariableTable.table[0].variable = biddyNull;
  biddyVariableTable.table[0].element = biddyNull;
  biddyVariableTable.table[0].selected = FALSE;
  biddyVariableTable.table[0].data = NULL;
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
    biddyVariableTable.table[i].name = NULL;
    biddyVariableTable.table[i].num = 0;
    biddyVariableTable.table[i].firstNode = NULL;
    biddyVariableTable.table[i].lastNode = NULL;
    biddyVariableTable.table[i].variable = biddyNull;
    biddyVariableTable.table[i].element = biddyNull;
    biddyVariableTable.table[i].selected = FALSE;
    biddyVariableTable.table[i].data = NULL;
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

  /* MAKE TERMINAL NODE "1" */
  /* TERMINAL NODE IS "1" FOR ALL BDD TYPES AND IT IS AT INDEX [0] */
  biddyNodeTable.table[0]->prev = NULL;
  biddyNodeTable.table[0]->next = NULL;
  biddyNodeTable.table[0]->list = NULL;
  biddyNodeTable.table[0]->f = biddyNull;
  biddyNodeTable.table[0]->t = biddyNull;
  biddyNodeTable.table[0]->v = 0; /* terminal node is the only one with v = 0 */
  biddyNodeTable.table[0]->expiry = 0; /* terminal node is fortified */
  biddyNodeTable.table[0]->select = 0; /* initialy it is not selected */

  /* MAKE TERMINAL EDGE AND EDGES REPRESENTING CONSTANTS */
  /* FOR THE EMPTY DOMAIN, biddyZero AND biddyOne ARE THE SAME FOR ALL BDD TYPES */
  /* biddyZero IS COMPLEMENTED FOR ALL BDD TYPES, INCLUDING OBDD, ZBDD, AND TZBDD */
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

  /* INITIALIZATION OF DEFAULT OPERATION CACHE - USED FOR ITE AND OTHER BOOLEAN OPERATIONS */
  biddyOPCache.disabled = FALSE;
  *(biddyOPCache.notusedyet) = TRUE;
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
  Biddy_Managed_AddCache(MNG,BiddyOPGarbage);

  /* INITIALIZATION OF DEFAULT EA CACHE - USED FOR QUANTIFICATIONS */
  biddyEACache.disabled = FALSE;
  *(biddyEACache.notusedyet) = TRUE;
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
  Biddy_Managed_AddCache(MNG,BiddyEAGarbage);

  /* INITIALIZATION OF DEFAULT RC CACHE - USED FOR RESTRICT AND COMPOSE */
  biddyRCCache.disabled = FALSE;
  *(biddyRCCache.notusedyet) = TRUE;
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
  Biddy_Managed_AddCache(MNG,BiddyRCGarbage);

  /* INITIALIZATION OF DEFAULT REPLACE CACHE - USED FOR REPLACE */
  biddyReplaceCache.keywordList = NULL;
  biddyReplaceCache.keyList = NULL;
  biddyReplaceCache.keyNum = 0;
  biddyReplaceCache.keywordNum = 0;
  biddyReplaceCache.disabled = FALSE;
  *(biddyReplaceCache.notusedyet) = TRUE;
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
  Biddy_Managed_AddCache(MNG,BiddyReplaceGarbage);

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
  printf("Delete BDD type ...\n");
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
#ifdef BIDDYEXTENDEDSTATS_YES
    free(biddyNodeTable.gcobsolete);
#endif
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
    free(biddyOPCache.notusedyet);
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
    free(biddyEACache.notusedyet);
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
    free(biddyRCCache.notusedyet);
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
    free(biddyReplaceCache.notusedyet);
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
  float gcrX, float rr, float rrF, float rrX, float st, float cst)
{
  if (!MNG) MNG = biddyAnonymousManager;

  if (gcr > 0.0) biddyNodeTable.gcratio = gcr;
  if (gcrF > 0.0) biddyNodeTable.gcratioF = gcrF;
  if (gcrX > 0.0) biddyNodeTable.gcratioX = gcrX;
  if (rr > 0.0) biddyNodeTable.resizeratio = rr;
  if (rrF > 0.0) biddyNodeTable.resizeratioF = rrF;
  if (rrX > 0.0) biddyNodeTable.resizeratioX = rrX;
  if (st > 0.0) biddyNodeTable.siftingtreshold = st;
  if (cst > 0.0) biddyNodeTable.convergesiftingtreshold = cst;

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
    For terminal nodes, function returns the same node.
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

  if (Biddy_IsTerminal(fun)) return fun;

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
    For terminal nodes, function returns the same node.
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

  if (Biddy_IsTerminal(fun)) return fun;

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
    Terminal node must be selected before starting this function!
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
    /* biddySelect is back to start - explicite deselection is required */
    Biddy_Variable v;
    BiddyNode *sup;
    biddyNodeTable.table[0]->select = 0;
    for (v=1; v<biddyVariableTable.num; v++) {
      biddyVariableTable.table[v].lastNode->list = NULL;
      sup = biddyVariableTable.table[v].firstNode;
      assert( sup != NULL );
      while (sup) {
        assert(sup->v == v) ;
        sup->select = 0;
        sup = (BiddyNode *) sup->list;
      }
    }
    biddySelect = 1;
  }
}

#ifdef __cplusplus
}
#endif

/***************************************************************************//*!
\brief Function Biddy_Managed_GetTerminal returns unmarked and untagged
       edge pointing to terminal node 1.

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
    BiddyRefresh(r); /* not always refreshed by ITE */

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
    fprintf(stderr,"Biddy_GetBaseSet: this BDD type is not supported, yet!\n");
    return biddyNull;
  } else {
    fprintf(stderr,"Biddy_GetBaseSet: Unsupported BDD type!\n");
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
    if ((x[0] == biddyVariableTable.table[v].name[0]) &&
        (!strcmp(x,biddyVariableTable.table[v].name)))
    {
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
\brief Function Biddy_Managed_GetLowestVariable returns the lowest variable
        in the current ordering.

### Description
    The lowest variable is the tompost variable.
### Side effects
### More info
    Macro Biddy_GetLowestVariable(x) is defined for use with anonymous manager.
*******************************************************************************/

#ifdef __cplusplus
extern "C" {
#endif

Biddy_Variable
Biddy_Managed_GetLowestVariable(Biddy_Manager MNG)
{
  Biddy_Variable v,k;

  if (!MNG) MNG = biddyAnonymousManager;

  v = 0;
  for (k = 1; k < biddyVariableTable.num; k++) {
    v = biddyVariableTable.table[v].prev;
  }

  return v;
}

#ifdef __cplusplus
}
#endif

/***************************************************************************//*!
\brief Function Biddy_Managed_GetIthVariable returns ith variable
        in the current global ordering.

### Description
    The lowest (topmost) variable has global ordering 1.
    The highest (bottommost) variable is '1' and has global ordering equal
    to numUsedVariables.
### Side effects
    If argument is 0, function returns 0.
    If argument is larger than the number of variables, function returns 0.
### More info
    Macro Biddy_GetIthVariable(x) is defined for use with anonymous manager.
*******************************************************************************/

#ifdef __cplusplus
extern "C" {
#endif

Biddy_Variable
Biddy_Managed_GetIthVariable(Biddy_Manager MNG, Biddy_Variable i)
{
  Biddy_Variable v;

  if (!MNG) MNG = biddyAnonymousManager;

  if ((i == 0) || (i > biddyVariableTable.num)) {
    return 0;
  } else {
    v = 0;
    while (i < biddyVariableTable.num) {
      v = Biddy_Managed_GetPrevVariable(MNG,v);
      i++;
    }
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
    It is not checked that the given variable is valid.
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
\brief Function Biddy_Managed_GetVariableValue gets variable's value.

### Description
### Side effects
    It is not checked that the given variable is valid.
### More info
    Macro Biddy_GetVariableValue(v) is defined for use with anonymous manager.
*******************************************************************************/

#ifdef __cplusplus
extern "C" {
#endif

Biddy_Edge
Biddy_Managed_GetVariableValue(Biddy_Manager MNG, Biddy_Variable v)
{
  if (!MNG) MNG = biddyAnonymousManager;

  return biddyVariableTable.table[v].value;
}

#ifdef __cplusplus
}
#endif

/***************************************************************************//*!
\brief Function Biddy_Managed_ClearVariablesData free memory used for all
        variable's data.

### Description
### Side effects
    Only active (used) variables are considered.
### More info
    Macro Biddy_ClearVariablesData() is defined for use with anonymous manager.
*******************************************************************************/

#ifdef __cplusplus
extern "C" {
#endif

void
Biddy_Managed_ClearVariablesData(Biddy_Manager MNG)
{
  Biddy_Variable v;

  if (!MNG) MNG = biddyAnonymousManager;

  for (v=0; v<biddyVariableTable.num; v++) {
    if (biddyVariableTable.table[v].data) {
      free(biddyVariableTable.table[v].data);
      biddyVariableTable.table[v].data = NULL;
    }
  }
}

#ifdef __cplusplus
}
#endif

/***************************************************************************//*!
\brief Function Biddy_Managed_SetVariableData sets variable's data.

### Description
### Side effects
    It is not checked that the given variable is valid.
### More info
    Macro Biddy_SetVariableData(v,x) is defined for use with anonymous manager.
*******************************************************************************/

#ifdef __cplusplus
extern "C" {
#endif

void
Biddy_Managed_SetVariableData(Biddy_Manager MNG, Biddy_Variable v, void *x)
{
  if (!MNG) MNG = biddyAnonymousManager;

  biddyVariableTable.table[v].data = x;
}

#ifdef __cplusplus
}
#endif

/***************************************************************************//*!
\brief Function Biddy_Managed_GetVariableData gets variable's data.

### Description
### Side effects
    It is not checked that the given variable is valid.
### More info
    Macro Biddy_GetVariableData(v) is defined for use with anonymous manager.
*******************************************************************************/

#ifdef __cplusplus
extern "C" {
#endif

void *
Biddy_Managed_GetVariableData(Biddy_Manager MNG, Biddy_Variable v)
{
  if (!MNG) MNG = biddyAnonymousManager;

  return biddyVariableTable.table[v].data;
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
    fprintf(stderr,"Biddy_Eval: this BDD type is not supported, yet!\n");
    return FALSE;
  } else {
    fprintf(stderr,"Biddy_Eval: Unsupported BDD type!\n");
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
\brief Function Biddy_Managed_EvalProbability evaluates BDD.

### Description
    Each variable should be associated with a probability q (0-1) via
    data element in BiddyVariable. For each 1-path a product of variable's
    probability is calculated (q for 'then' successor and (1-q) for 'else'
    successor). The result of the function is the sum of all such products.
### Side effects
    Implemented for OBDD, ZBDD, and TZBDD.
### More info
    Macro Biddy_EvalProbability(f) is defined for use with anonymous manager.
*******************************************************************************/

#ifdef __cplusplus
extern "C" {
#endif

double
Biddy_Managed_EvalProbability(Biddy_Manager MNG, Biddy_Edge f)
{
  double r1,r0;
  Biddy_Boolean rf;

  if (!MNG) MNG = biddyAnonymousManager;

  if (biddyManagerType == BIDDYTYPEOBDD) {
    /* IMPLEMENTED */
  } else if (biddyManagerType == BIDDYTYPEOBDDC) {
    /* IMPLEMENTED */
  } else if (biddyManagerType == BIDDYTYPEZBDD) {
    /* IMPLEMENTED */
  } else if (biddyManagerType == BIDDYTYPEZBDDC) {
    /* IMPLEMENTED */
  } else if (biddyManagerType == BIDDYTYPETZBDD) {
    /* IMPLEMENTED */
  } else if (biddyManagerType == BIDDYTYPETZBDDC)
  {
    fprintf(stderr,"Biddy_EvalProbability: this BDD type is not supported, yet!\n");
    return 0;
  } else if ((biddyManagerType == BIDDYTYPEOFDD) || (biddyManagerType == BIDDYTYPEOFDDC) ||
              (biddyManagerType == BIDDYTYPEZFDD) || (biddyManagerType == BIDDYTYPEZFDDC) ||
              (biddyManagerType == BIDDYTYPETZFDD) || (biddyManagerType == BIDDYTYPETZFDDC))
  {
    fprintf(stderr,"Biddy_EvalProbability: this BDD type is not supported, yet!\n");
    return 0;
  } else {
    fprintf(stderr,"Biddy_EvalProbability: Unsupported BDD type!\n");
    return 0;
  }

  if (Biddy_IsNull(f)) return 0;
  if (f == biddyZero) return 0;
  if (Biddy_IsTerminal(f)) return 1;

  BiddyCreateLocalInfo(MNG,f);
  evalProbability(MNG,f,&r1,&r0,&rf); /* all nodes except terminal node are selected */
  BiddyDeleteLocalInfo(MNG,f);

  return r1;
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
\brief Function Biddy_Managed_IsLowest returns TRUE if the variable is
        the lowest one (lowest == topmost).

### Description
### Side effects
### More info
    Macro BiddyIsLowest(v) is defined for internal use.
    Macro Biddy_IsLowest(v) is defined for use with anonymous manager.
*******************************************************************************/

#ifdef __cplusplus
extern "C" {
#endif

Biddy_Boolean
Biddy_Managed_IsLowest(Biddy_Manager MNG, Biddy_Variable v)
{
  /* lowest (topmost) variable has global ordering 1 */
  /* global ordering is the number of zeros in corresponding line of orderingTable */

  if (!MNG) MNG = biddyAnonymousManager;

  /* FOR OBDDs AND OFDDs: the lowest (topmost) variable always has prev=biddyVariableTable.size */
  /* FOR ZBDDs, ZFDDs, TZBDDs, AND TZFDDs: the topmost variable always has prev=biddyVariableTable.num */

  if (v >= biddyVariableTable.num) {
    return FALSE;
  } else {
    return (biddyVariableTable.table[v].prev >= biddyVariableTable.num);
  }
}

#ifdef __cplusplus
}
#endif

/***************************************************************************//*!
\brief Function Biddy_Managed_IsHighest returns TRUE if the variable is
        the highest one if terminal node is ignored (highest == bottommost).

### Description
### Side effects
### More info
    Macro BiddyIsHighest(v) is defined for internal use.
    Macro Biddy_IsHighest(v) is defined for use with anonymous manager.
*******************************************************************************/

#ifdef __cplusplus
extern "C" {
#endif

Biddy_Boolean
Biddy_Managed_IsHighest(Biddy_Manager MNG, Biddy_Variable v)
{
  /* bottommost variable is '1' and has global ordering numUsedVariables */
  /* global ordering is the number of zeros in corresponding line of orderingTable */

  if (!MNG) MNG = biddyAnonymousManager;

  /* ALWAYS: the bottommost variable has next=0 */

  if ((biddyVariableTable.num == 1) && (v == 0)) return TRUE;
  if ((v == 0) || (v >= biddyVariableTable.num)) {
    return FALSE;
  } else {
    return (biddyVariableTable.table[v].next == 0);
  }
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
    Variables and elements are always repaired. Formulae are repaired with
    regards to the parameter varelem. BDDs without external references are not
    repaired. For OBDDs, OFDDs, TZBDDS, and TZFDDs, it is safe to add new
    variables/elements if BDDs are used to represent Boolean functions.
    For ZBDDs and ZFDDs, it is safe to add new variables/elements if BDDs are
    used to represent combination sets.
    User should not add numbered variables/elements with some other function.
    TO DO: Formulae in user's formula tables are not repaired, yet!
    TO DO: Variables cannot be deleted, yet!
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
      /* DEBUGGING */
      /*
      printf("Biddy_Managed_FoaVariable: variable \"%s\"(%u) added, cleaning cache tables\n",x,v);
      */

      /* VARIANT 1: DELETE ONLY INVALID RESULTS */
      /*
      BiddyOPGarbageNewVariable(MNG,v);
      BiddyEAGarbageNewVariable(MNG,v);
      BiddyRCGarbageNewVariable(MNG,v);
      BiddyReplaceGarbageNewVariable(MNG,v);
      */

      /* VARIANT 2: SIMPLY DELETE ALL RESULTS */
      /**/
      BiddyOPGarbageDeleteAll(MNG);
      BiddyEAGarbageDeleteAll(MNG);
      BiddyRCGarbageDeleteAll(MNG);
      BiddyReplaceGarbageDeleteAll(MNG);
      /**/
    }

    /* *** ADAPTING FORMULAE *** */

    /* FOR OBDDs AND OFDDs, FORMULAE MUST BE ADAPTED IF THEY REPRESENT COMBINATION SETS */
    /* biddyZero AND biddyOne ALWAYS REPRESENT A BOOLEAN FORMULA! */
    if ((biddyManagerType == BIDDYTYPEOBDDC) || (biddyManagerType == BIDDYTYPEOBDD) ||
        (biddyManagerType == BIDDYTYPEOFDDC) || (biddyManagerType == BIDDYTYPEOFDD))
    {
      if (!varelem) {
        /* NOT IMPLEMENTED, YET */
      }
    }

    /* FOR ZBDDs AND ZFDDs, FORMULAE MUST BE ADAPTED IF THEY REPRESENT BOOLEAN FUNCTIONS */
    /* biddyZero AND biddyOne ALWAYS REPRESENT A BOOLEAN FORMULA! */
    /* first formula is 0, it is always a single terminal node */
    /* second formula is 1, it could be a large graph, e.g. for ZBDD */
    if ((biddyManagerType == BIDDYTYPEZBDDC) || (biddyManagerType == BIDDYTYPEZBDD) ||
        (biddyManagerType == BIDDYTYPEZFDDC) || (biddyManagerType == BIDDYTYPEZFDD))
    {

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

            biddyFormulaTable.table[i].f =
              BiddyManagedTaggedFoaNode(MNG,v,biddyFormulaTable.table[i].f,
                biddyFormulaTable.table[i].f,v,TRUE); /* FoaNode returns an obsolete node! */
            BiddyRefresh(biddyFormulaTable.table[i].f);

          }
        }

        /* MNG[3] = biddyFormulaTable.table[0].f; */ /* biddyZero does not need to be updated */
        MNG[4] = biddyFormulaTable.table[1].f; /* biddyOne must be updated, e.g. for ZBDD */

      } else {

        for (i = 0; i < 2; i++) {
          biddyFormulaTable.table[i].f =
            BiddyManagedTaggedFoaNode(MNG,v,biddyFormulaTable.table[i].f,
              biddyFormulaTable.table[i].f,v,TRUE); /* FoaNode returns an obsolete node! */
          BiddyRefresh(biddyFormulaTable.table[i].f);
        }

        MNG[3] = biddyFormulaTable.table[0].f; /* biddyZero must be updated */
        MNG[4] = biddyFormulaTable.table[1].f; /* biddyOne must be updated */

      }
    }

    /* FOR TZBDDs AND TZFDDs, FORMULAE MUST BE ADAPTED REGARDLESS OF WHAT THEY REPRESENT */
    /* biddyZero AND biddyOne ALWAYS REPRESENT A BOOLEAN FORMULA! */
    else if ((biddyManagerType == BIDDYTYPETZBDDC) || (biddyManagerType == BIDDYTYPETZBDD) ||
              (biddyManagerType == BIDDYTYPETZFDDC) || (biddyManagerType == BIDDYTYPETZFDD))
    {

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

    /* FOR OBDDs AND OFDDs, NEW VARIABLE IS ADDED BELOW ALL THE EXISTING ONES */
    /* FOR OBDDs AND OFDDs, ALL ELEMENTS MUST BE ADAPTED */
    if ((biddyManagerType == BIDDYTYPEOBDDC) || (biddyManagerType == BIDDYTYPEOBDD) ||
        (biddyManagerType == BIDDYTYPEOFDDC) || (biddyManagerType == BIDDYTYPEOFDD))
    {
      /* NOT IMPLEMENTED, YET */
    }

    /* FOR ZBDDs AND ZFDDs, NEW VARIABLE IS ADDED ABOVE ALL THE EXISTING ONES */
    /* FOR ZBDDs AND ZFDDs, ALL VARIABLES MUST BE ADAPTED */
    if ((biddyManagerType == BIDDYTYPEZBDDC) || (biddyManagerType == BIDDYTYPEZBDD) ||
        (biddyManagerType == BIDDYTYPEZFDDC) || (biddyManagerType == BIDDYTYPEZFDD))
    {
      for (w = 1; w < biddyVariableTable.num-1; w++) {

        /* DEBUGGING */
        /*
        printf("Biddy_FoaVariable: updating variable %s (w=%u)\n",Biddy_Managed_GetVariableName(MNG,w),w);
        printf("top node before updating: %s\n",Biddy_Managed_GetTopVariableName(MNG,biddyVariableTable.table[w].variable));
        */

        biddyVariableTable.table[w].variable =
          BiddyManagedTaggedFoaNode(MNG,v,biddyVariableTable.table[w].variable,
            biddyVariableTable.table[w].variable,v,TRUE); /* FoaNode returns an obsolete node! */
        BiddyRefresh(biddyVariableTable.table[w].variable);

        /* DEBUGGING */
        /*
        printf("top node of variable after updating: %s\n",Biddy_Managed_GetTopVariableName(MNG,biddyVariableTable.table[w].variable));
        */
      }
    }

    /* FOR TZBDDs AND TZFDDs, NEW VARIABLE IS ADDED ABOVE ALL THE EXISTING ONES */
    /* FOR TZBDDs AND TZFDDs, ELEMENTS MUST BE ADAPTED IF NEW VARIABLE IS ADDED ABOVE ALL THE EXISTING VARIABLES */
    if ((biddyManagerType == BIDDYTYPETZBDDC) || (biddyManagerType == BIDDYTYPETZBDD) ||
        (biddyManagerType == BIDDYTYPETZFDDC) || (biddyManagerType == BIDDYTYPETZFDD))
    {
      for (w = 1; w < biddyVariableTable.num-1; w++) {
        Biddy_SetTag(biddyVariableTable.table[w].element,v);
      }
    }

    /* FOR TZBDDs AND TZFDDs, ELEMENT MUST BE ADAPTED IF NEW VARIABLE IS ADDED IMMEDIATELLY BELLOW IT */
    if ((biddyManagerType == BIDDYTYPETZBDDC) || (biddyManagerType == BIDDYTYPETZBDD) ||
        (biddyManagerType == BIDDYTYPETZFDDC) || (biddyManagerType == BIDDYTYPETZFDD))
    {
      /* NOT NEEDED */
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

  if (v == 0) return; /* constant variable cannot be renamed */
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

Biddy_Variable
Biddy_Managed_AddVariableByName(Biddy_Manager MNG, Biddy_String x)
{
  if (!MNG) MNG = biddyAnonymousManager;

  return Biddy_Managed_FoaVariable(MNG,x,TRUE);
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

Biddy_Variable
Biddy_Managed_AddElementByName(Biddy_Manager MNG, Biddy_String x)
{
  if (!MNG) MNG = biddyAnonymousManager;

  return Biddy_Managed_FoaVariable(MNG,x,FALSE);
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
    fprintf(stderr,"WARNING: variable cannot be added below constant variable\n");
    return biddyNull;
  }

  if (v >= biddyVariableTable.num ) {
    fprintf(stderr,"WARNING: variable cannot be added below the non-existing node\n");
    return biddyNull;
  }

  f = Biddy_Managed_GetVariableEdge(MNG,Biddy_Managed_AddVariableByName(MNG,NULL));
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
    fprintf(stderr,"Biddy_AddVariableBelow:: this BDD type is not supported, yet!\n");
    return biddyNull;
  } else {
    fprintf(stderr,"Biddy_AddVariableBelow: Unsupported BDD type!\n");
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

  f = Biddy_Managed_GetVariableEdge(MNG,Biddy_Managed_AddVariableByName(MNG,NULL));
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
    fprintf(stderr,"Biddy_AddVariableAbove: this BDD type is not supported, yet!\n");
    return biddyNull;
  } else {
    fprintf(stderr,"Biddy_AddVariableAbove: Unsupported BDD type!\n");
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
    (biddyManagerType == BIDDYTYPEOBDDC) || (biddyManagerType == BIDDYTYPEOBDD) ||
    (biddyManagerType == BIDDYTYPEZBDDC) || (biddyManagerType == BIDDYTYPEZBDD) ||
    (biddyManagerType == BIDDYTYPETZBDDC) || (biddyManagerType == BIDDYTYPETZBDD) ||
    (biddyManagerType == BIDDYTYPEOFDDC) || (biddyManagerType == BIDDYTYPEOFDD) ||
    (biddyManagerType == BIDDYTYPEZFDDC) || (biddyManagerType == BIDDYTYPEZFDD) ||
    (biddyManagerType == BIDDYTYPETZFDDC) || (biddyManagerType == BIDDYTYPETZFDD)
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
    fprintf(stderr,"Biddy_TransferMark: this BDD type is not supported, yet!\n");
    return biddyNull;
  } else if ((biddyManagerType == BIDDYTYPEOFDDC) ||
              (biddyManagerType == BIDDYTYPEZFDDC) ||
              (biddyManagerType == BIDDYTYPETZFDDC))
  {
    fprintf(stderr,"Biddy_TransferMark: this BDD type is not supported, yet!\n");
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
    fprintf(stderr,"Biddy_IncTag: this BDD type is not supported, yet!\n");
    return biddyNull;
  } else if ((biddyManagerType == BIDDYTYPETZFDDC) || (biddyManagerType == BIDDYTYPETZFDD)) {
    /* NOT IMPLEMENTED, YET */
    fprintf(stderr,"Biddy_IncTag: this BDD type is not supported, yet!\n");
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
    new one.  For OBDDs, ZBDDs, OFDDs, and ZFDDs, the returned edge is not
    tagged (i.e. tag == 0). For TZBDDs and TZFDDs, the returned edge is
    tagged with the given ptag. There are two special cases:
    1. If (pf == pt == NULL) then new variable (for OBDDs, OFDDs, TZBDDs,
    and TZFDDs) or new element (for ZBDDs and ZFDDs) is created.
    2. (If ptag == 0) then the reduction rule and the normalization of
    complemented edges is not used and the node is added exactly as
    specified (be careful, this may create a wrong node!).
### Side effects
    This function should not be called directly to add new variables and
    elements, you must use Biddy_Managed_FoaVariable,
    Biddy_Managed_AddVariableByName, or Biddy_Managed_AddElementByName.
    Using Biddy_Managed_TaggedFoaNode you can create node with an arbitrary
    ordering. It is much more safe to use Boolean operators, e.g.
    Biddy_Managed_ITE.
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

  if (biddyManagerType == BIDDYTYPEOBDD) {
    /* IMPLEMENTED */
    r = BiddyManagedTaggedFoaNode(MNG,v,pf,pt,ptag,garbageAllowed);
    BiddyRefresh(r); /* FoaNode returns an obsolete node! */
  } else if (biddyManagerType == BIDDYTYPEOBDDC) {
    /* IMPLEMENTED */
    r = BiddyManagedTaggedFoaNode(MNG,v,pf,pt,ptag,garbageAllowed);
    BiddyRefresh(r); /* FoaNode returns an obsolete node! */
  } else if (biddyManagerType == BIDDYTYPEZBDD) {
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
  } else if (biddyManagerType == BIDDYTYPETZBDDC)
  {
    fprintf(stderr,"Biddy_TaggedFoaNode: this BDD type is not supported, yet!\n");
    return biddyNull;
  } else if ((biddyManagerType == BIDDYTYPEOFDDC) || (biddyManagerType == BIDDYTYPEOFDD) ||
              (biddyManagerType == BIDDYTYPEZFDDC) || (biddyManagerType == BIDDYTYPEZFDD) ||
              (biddyManagerType == BIDDYTYPETZFDDC) || (biddyManagerType == BIDDYTYPETZFDD))
  {
    fprintf(stderr,"Biddy_TaggedFoaNode: this BDD type is not supported, yet!\n");
    return biddyNull;
  } else {
    fprintf(stderr,"Biddy_TaggedFoaNode: Unsupported BDD type!\n");
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

  /* DEBUGGING */
  /*
  printf("BiddyManagedTaggedFoaNode: %p, %d\n",MNG,Biddy_Managed_GetManagerType(MNG));
  */

  assert( MNG != NULL );

  /* IMPLEMENTED FOR OBDD, OBDDC, ZBDD, ZBDDC, AND TZBDD */
  assert(
    (biddyManagerType == BIDDYTYPEOBDD) ||
    (biddyManagerType == BIDDYTYPEOBDDC) ||
    (biddyManagerType == BIDDYTYPEZBDD) ||
    (biddyManagerType == BIDDYTYPEZBDDC) ||
    (biddyManagerType == BIDDYTYPETZBDD)
  );

  assert( v != 0 );

  /* CURRENTLY, SPECIAL CASE 2 IS ONLY INTENDED FOR USE WITH TAGGED BDDs */
  /* TO DO: USE SPECIAL CASE 2 FOR ALL BDD TYPES TO IMPROVE GRAPH COPYING/CONSTRUCTING */
  assert( (biddyManagerType == BIDDYTYPETZBDD) ||
          (biddyManagerType == BIDDYTYPETZBDDC) ||
          (ptag != 0) );

#ifdef BIDDYEXTENDEDSTATS_YES
  biddyNodeTable.foa++;
#endif

  /* DEBUGGING */
  /*
  printf("FOANODE: v=%u, pf = %p, pt = %p, tag = %u, biddyNull = %p\n",v,pf,pt,biddyNull,NULL);
  */

  /* DISABLE GC */
  /*
  garbageAllowed = FALSE;
  */

  addNodeSpecial = FALSE;
  complementedResult = FALSE;

  /* SPECIAL CASE1: pf == pt == NULL */

  if (!pf) {
    if ((biddyManagerType == BIDDYTYPEOBDDC) || (biddyManagerType == BIDDYTYPEOBDD)) {
      pf = biddyZero;
      pt = biddyTerminal;
    }
    else if (biddyManagerType == BIDDYTYPEZBDD) {
      pf = biddyZero;
      pt = biddyTerminal;
    }
    else if (biddyManagerType == BIDDYTYPEZBDDC) {
      pf = biddyTerminal;
      pt = biddyTerminal;
      complementedResult = TRUE;
    }
    else if (biddyManagerType == BIDDYTYPETZBDD) {
      pf = biddyZero;
      pt = biddyTerminal;
    }
    else if (biddyManagerType == BIDDYTYPETZBDDC) {
      pf = biddyTerminal;
      pt = biddyTerminal;
      complementedResult = TRUE;
    }
    ptag = v;
    addNodeSpecial = TRUE;
  } else {
    assert( BiddyIsSmaller(v,BiddyV(pf)) );
    assert( BiddyIsSmaller(v,BiddyV(pt)) );
  }

  /* DEBUGGING */
  /*
  printf("FOANODE (%d)",Biddy_Managed_GetManagerType(MNG));
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

 /* SPECIAL CASE 2: ptag == 0 */

  if (!addNodeSpecial && ptag) {

    /* SIMPLE CASES */

    if ((biddyManagerType == BIDDYTYPEOBDDC) || (biddyManagerType == BIDDYTYPEOBDD)) {
      if (pf == pt) {
        return pf;
      }
    }

    else if ((biddyManagerType == BIDDYTYPEZBDDC) || (biddyManagerType == BIDDYTYPEZBDD)) {
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

  }

  /* SOME NODES ARE NOT HASHED IN THE USUAL WAY */
  /* FOR THEM, THE RESULTING EDGE IS STORED ONLY AS PART OF BiddyVariable */
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
  if ((biddyManagerType == BIDDYTYPEZBDD) || (biddyManagerType == BIDDYTYPEZFDD))
  {
    if ((pf == biddyZero) && (pt == biddyTerminal) && !addNodeSpecial) {
      return biddyVariableTable.table[v].element;
    }
  }
  if ((biddyManagerType == BIDDYTYPEZBDDC) || (biddyManagerType == BIDDYTYPEZFDDC))
  {
    if ((pf == biddyTerminal) && (pt == biddyTerminal) && !addNodeSpecial) {
      if (complementedResult) {
        return biddyVariableTable.table[v].element;
      }
      else {
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

  if ((biddyManagerType == BIDDYTYPETZBDD) && ptag) {
    Biddy_SetTag(edge,ptag);
  }

  /* (Biddy v1.5) To enable efficient memory management (e.g. sifting) */
  /* the function started with the returned node is not recursively refreshed! */

  /* DEBUGGING */
  /*
  printf("FOANODE COMPLETE ");
  if (addNodeSpecial) printf("(addNodeSpecial==TRUE) ");
  debugEdge(MNG,edge);
  printf("\n");
  */

  /* DEBUGGING */
  /*
  if (!((biddyManagerType != BIDDYTYPETZBDD) || (Biddy_GetTag(edge) == BiddyV(edge)) || BiddyIsSmaller(Biddy_GetTag(edge),BiddyV(edge))))
  {
    printf("FOANODE ERROR");
    if (addNodeSpecial) printf(" (addNodeSpecial==TRUE)");
    printf("!\n");
    writeORDER(MNG);
    debugEdge(MNG,edge);
    printf("\n");
  }
  */

  assert( BiddyIsSmaller(BiddyV(edge),BiddyV(BiddyE(edge))) );
  assert( BiddyIsSmaller(BiddyV(edge),BiddyV(BiddyT(edge))) );
  assert( (biddyManagerType != BIDDYTYPETZBDD) || !ptag || (Biddy_GetTag(edge) == BiddyV(edge)) || BiddyIsSmaller(Biddy_GetTag(edge),BiddyV(edge)) );
  assert( (biddyManagerType != BIDDYTYPETZBDD) || !ptag || (Biddy_GetTag(BiddyE(edge)) == BiddyV(BiddyE(edge))) || BiddyIsSmaller(Biddy_GetTag(BiddyE(edge)),BiddyV(BiddyE(edge))) );
  assert( (biddyManagerType != BIDDYTYPETZBDD) || !ptag || (Biddy_GetTag(BiddyT(edge)) == BiddyV(BiddyT(edge))) || BiddyIsSmaller(Biddy_GetTag(BiddyT(edge)),BiddyV(BiddyT(edge))) );

  return edge;
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
    Iff parameter purge is true then all formulae without name are deleted.
    Iff parameter purge is true then all nodes which are not part of
    non-obsolete non-deleted formulae are removed even if they are fresh or
    fortified (this should not be used during automatic garbage collection!).
    If parameter total is true than all unnecessary nodes are immediately
    deleted, otherwise they are deleted only when there are enough of them.
    If (targetLT != 0) then node table resizing is disabled.
    If (targetLT != 0) then there should not exist obsolete formulae.
    If (targetLT != 0) then there should not exist obsolete nodes which are not
    part of any non-obsolete non-deleted formulae.
    If (targetLT != 0) then there should not exist obsolete nodes with variable
    equal or higher (bottom-more) than target and smaller (top-more) than
    targetGEQ.
### Side effects
    The first element of each chain in a node table should have a special value
    for its 'prev' element to allow tricky but efficient deleting. Moreover,
    'prev' and 'next' should be the first and the second element in the
    structure BiddyNode, respectively. Garbage collecion is reported by
    biddyNodeTable.garbage only if some bad nodes are purged!
    Parameters targetLT and targetGEQ are used during sifting, only,
    in all other cases 0 is used.
### More info
    Macro Biddy_GC(targetLT,targetGEQ,purge,total) is defined for use with
    anonymous manager.
    Macros Biddy_Managed_AutoGC(MNG) and Biddy_AutoGC() are useful variants
    with targetLT = targetGEQ = 0, purge = FALSE, and total = FALSE.
    Macros Biddy_Managed_ForceGC(MNG) and Biddy_ForceGC() are useful variants
    with targetLT = targetGEQ = 0, purge = FALSE, and total = TRUE.
*******************************************************************************/

#ifdef __cplusplus
extern "C" {
#endif

void
Biddy_Managed_GC(Biddy_Manager MNG, Biddy_Variable targetLT,
                 Biddy_Variable targetGEQ,Biddy_Boolean purge,
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
  fprintf(stdout,"\nGarbage (%d), targetLT=%u, targetGEQ=%u, systemAge=%u, there are %u BDD nodes, NodeTableSize=%u\n",
                 ++n,targetLT,targetGEQ,biddySystemAge,biddyNodeTable.num,biddyNodeTable.size);
  }
  */

  assert( (targetLT == 0) || (targetGEQ != 0) );

  /* GC calls used during sifting are not counted */
  if (targetLT == 0) {
    biddyNodeTable.garbage++;
  }

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
  /*
  {
  BiddyNode *sup;
  biddyVariableTable.table[3].lastNode->list = NULL;
  sup = biddyVariableTable.table[3].firstNode;
  while (sup) {
    debugNode(MNG,sup);
    sup = (BiddyNode *) sup->list;
  }
  printf("\n");
  }
  */

  starttime = clock();

  /* REMOVE ALL FORMULAE WHICH ARE NOT PRESERVED ANYMORE */
  /* The first two formulae ("0" and "1") are never removed. */
  /* Iff parameter purge is true then all formulae without name are removed. */
  /* Iff parameter purge is true then all deleted formulae (even if fortified/preserved) are removed. */

  if (targetLT == 0) {
    i = 2;
    biddyFormulaTable.numOrdered = 2;
  } else {
    /* if (targetLT != 0) then there should not obsolete formulae */
    i = biddyFormulaTable.size;
  }
  for (j = i; j < biddyFormulaTable.size; j++) {

    /* DEBUGGING */
    /*
    printf("[FORM:%s@%u]",biddyFormulaTable.table[j].name,biddyFormulaTable.table[j].expiry);
    */

    if (purge && !biddyFormulaTable.table[j].name) {
      biddyFormulaTable.table[j].deleted = TRUE;
    }

    if ((!purge || !biddyFormulaTable.table[j].deleted)
        &&
        (!biddyFormulaTable.table[j].expiry || biddyFormulaTable.table[j].expiry >= biddySystemAge))
    {

      /* THIS FORMULA IS OK */
      if (i != j) {
        biddyFormulaTable.table[i] = biddyFormulaTable.table[j];
      }
      if (biddyFormulaTable.table[i].name) biddyFormulaTable.numOrdered++;
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

  /* if parameter purge is true then nodes which are not part of any non-obsolete */
  /* non-deleted formulae are removed even if they are fresh or fortified */
  /* (this should not be used during the automatic garbage collection!) */

  if ((targetLT == 0) && purge) {
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
        !Biddy_IsTerminal(biddyFormulaTable.table[j].f))
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
    /* if (targetLT != 0) then nodes equal or higher (bottom-more) than targetLT */
    /* and smaller (top-more) than targetGEQ will not be prolonged */
    if (v == targetLT) {
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
    else if ((targetLT == 0) || (BiddyIsSmaller(v,targetLT))) {
      /* (targetLT == 0) used in the above 'if' for the evaluation speed, only */
      /* there is always at least one node in the list */
      sup = biddyVariableTable.table[v].firstNode;
      assert( sup != NULL );
      assert( sup->v == v );
      /* successors of the first node in the list does not need to be prolonged */
      sup = (BiddyNode *) sup->list;
      while (sup) {
        assert( sup->v == v );
        if (!(sup->expiry) || (sup->expiry >= biddySystemAge)) {
          assert( !Biddy_IsNull(BiddyE(sup)) );
          assert( !Biddy_IsNull(BiddyT(sup)) );
          BiddyProlongRecursively(MNG,BiddyE(sup),sup->expiry,targetLT);
          BiddyProlongRecursively(MNG,BiddyT(sup),sup->expiry,targetLT);
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

  /* IF (targetLT != 0) THEN NODE TABLE RESIZING IS DISABLED */
  if (targetLT != 0) resizeRequired = FALSE;

  /* CHECK IF THE DELETION OF NODES IS USEFUL - how many nodes will be deleted */
  gcUseful = ((biddyNodeTable.num - i) > (biddyNodeTable.gcratio * biddyNodeTable.blocksize));
  if (gcUseful) resizeRequired = FALSE;

  /* IF total AND THERE IS SOMETHING TO DELETE THEN gcUseful must be TRUE */
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
      while (resizeRequired) {
        biddyNodeTable.size = 2 * biddyNodeTable.size + 1;
        /* ADAPT PARAMETERS */
  #pragma warning(suppress: 4244)
        biddyNodeTable.gcratio = (1.0 - biddyNodeTable.gcratioX) + (biddyNodeTable.gcratio / biddyNodeTable.gcratioF);
  #pragma warning(suppress: 4244)
        biddyNodeTable.resizeratio = (1.0 - biddyNodeTable.resizeratioX) + (biddyNodeTable.resizeratio * biddyNodeTable.resizeratioF);
        if (biddyNodeTable.gcratio < 0.0) biddyNodeTable.gcratio = 0.0;
        if (biddyNodeTable.resizeratio < 0.0) biddyNodeTable.resizeratio = 0.0;
        resizeRequired = (
          (biddyNodeTable.size < biddyNodeTable.limitsize) &&
          (i > biddyNodeTable.resizeratio * biddyNodeTable.size)
        );
      }
      resizeRequired = TRUE; /* reset variable resizeRequired */

      /* CREATE NEW NODE TABLE */
      if (!(biddyNodeTable.table = (BiddyNode **)
      calloc((biddyNodeTable.size+2),sizeof(BiddyNode *)))) {
        fprintf(stderr,"Biddy_Managed_GC: Out of memoy!\n");
        exit(1);
      }
      biddyNodeTable.table[0] = (BiddyNode *) biddyTerminal;

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

      /* DEBUGGING - !!! PRINTF THE NUMBER OF OBSOLETE NODES PER VARIABLE !!! */
      /*
      printf("GC VARIABLE %u (NODES: %u, OBSOLETE NODES: %u)\n",v,biddyVariableTable.table[v].num,biddyVariableTable.table[v].numobsolete);
      */

      /* if (v >= targetLT) or (v < targetGEQ) then no action is required because */
      /* there should be no obsolete nodes with (v >= targetLT) or (v < targetGEQ) */
      /* please note, that resize is not allowed if (targetLT != 0) */
      /* (targetLT != 0) is used in the next 'if' for evaluation speed, only */
      if ((targetLT != 0) && (!BiddyIsSmaller(v,targetLT) || BiddyIsSmaller(v,targetGEQ))) continue;

      /* IF RESIZE THEN ALL NODES EXCEPT THE FIRST ONE MUST BE VISITED */
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

            /* if targetLT is used then */
            /* there should not exist obsolete nodes with variable equal or higher than targetLT */
            /* and variable smaller than targetGEQ */
            /* in fact, nodes with obsolete 'expiry' may exist but only because */
            /* they have not been prolonged - they must not be removed! */
            assert( (targetLT == 0) || (BiddyIsSmaller(v,targetLT) && !BiddyIsSmaller(v,targetGEQ)) ) ;

            /* DEBUGGING */
            /*
            printf("REMOVE %p\n",(void *)tmpnode2);
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

  /* DEBUGGING */
  /*
  printf("GC FINISHED: systemAge = %u, there are %u BDD nodes, NodeTableSize = %u\n",
          biddySystemAge,biddyNodeTable.num,biddyNodeTable.size);
  */

  /* DEBUGGING */
  /*
  if (purge) {
    for (v=1; v<biddyVariableTable.num; v++) {
      biddyVariableTable.table[v].lastNode->list = NULL;
      sup = biddyVariableTable.table[v].firstNode;
      while (sup) {
        if ((sup->expiry) && (sup->expiry < biddySystemAge)) {
          printf("Biddy_Managed_GC:: INVALID OBSOLETE NODE!\n");
          exit(1);
        }
        sup = (BiddyNode *) sup->list;
      }
    }
  }
  */

  /* GC calls used during sifting are not counted */
  if (targetLT == 0) {
    biddyNodeTable.gctime += clock() - starttime;
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
  /* constant functions must be explicitly refreshed, here */
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
    All formulae without name are deleted.
    All deleted formulae (including prolonged/fortified formulae) are removed.
    All fresh and obsolete nodes are immediatelly removed.
    Moreover, all prolonged and fortified nodes are removed if they are not
    needed by some of the remaining formula.
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
  Biddy_Managed_GC(MNG,0,0,TRUE,TRUE); /* purge = TRUE, total = TRUE */
  BiddyDecSystemAge(MNG);

  /* constant functions must be explicitly repaired, here */
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
    Given BDD becomes a formula. If (x != NULL) then formula is accessible by
    its name. If (x != NULL) then index of the formula in the Formulae Table
    is returned, otherwise function returns 0.
    Nodes of the given BDD will be preserved for the given number of clearings.
    If (c == -1) then formula is refreshed but not preserved.
    If (c == 0) then formula is persistently preserved.
    You have to use Biddy_DeleteFormula and Biddy_Purge to remove nodes
    of persistently preserved formulae.
    There are five macros defined to simplify formulae management:
    Biddy_Managed_AddTmpFormula(mng,name,bdd)
      := Biddy_Managed_AddFormula(mng,name,bdd,-1)
    Biddy_Managed_AddPersistentFormula(mng,name,bdd)
      := Biddy_Managed_AddFormula(mng,name,bdd,0)
    Biddy_Managed_KeepFormula(mng,bdd)
      := Biddy_Managed_AddFormula(mng,NULL,bdd,1)
    Biddy_Managed_KeepFormulaProlonged(mng,bdd,c)
      := Biddy_Managed_AddFormula(mng,NULL,bdd,c)
    Biddy_Managed_KeepFormulaUntilPurge(mng,bdd)
      := Biddy_Managed_AddFormula(mng,NULL,bdd,0)
### Side effects
    Function is prolonged or fortified. Formulae with name are ordered by name.
    If formula with the same name already exists, it will be overwritten -
    preserved (i.e. not obsolete and not fresh) and persistently preserved
    formulae will be deleted at the original index and recreated at new index!
### More info
    Macros Biddy_AddFormula(x,f,c), Biddy_AddTmpFormula(x,f),
    Biddy_AddPersistentFormula(x,f),  Biddy_KeepFormula(f),
    Biddy_KeepFormulaProlonged(f,c), and Biddy_KeepFormulaUntilPurge(f)
    are defined for use with anonymous manager.
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
      BiddyCompactSystemAge(MNG);
    }
    c += biddySystemAge;
  }

  /* if formula does not have a name and is NULL or constant function, */
  /* then it will not be added, such BDD is already fortified, redundancy not needed */
  if (!x && (Biddy_IsNull(f) || (f == biddyZero) || (f == biddyOne))) {
    return 0;
  }

  /* if formula does not have a name and it is the same as a basic variable/element, */
  /* then it will not be added, such BDD is already fortified, redundancy not needed */
  if (!x && !Biddy_IsNull(f)) {
    if ((biddyManagerType == BIDDYTYPEOBDDC) || (biddyManagerType == BIDDYTYPEOBDD) ||
        (biddyManagerType == BIDDYTYPEOFDDC) || (biddyManagerType == BIDDYTYPEOFDD))
    {
      if ((BiddyE(f) == biddyZero) && (BiddyT(f) == biddyTerminal)) {
        return 0;
      }
    }
    if ((biddyManagerType == BIDDYTYPEZBDD) || (biddyManagerType == BIDDYTYPEZFDD))
    {
      if ((BiddyE(f) == biddyZero) && (BiddyT(f) == biddyTerminal)) {
        return 0;
      }
    }
    if ((biddyManagerType == BIDDYTYPEZBDDC) || (biddyManagerType == BIDDYTYPEZFDDC))
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

  /* DEBUGGING */
  /*
  printf("Biddy_Managed_AddFormula: biddyFormulaTable.size = %u, findFormula returned %u\n",biddyFormulaTable.size,i);
  */

  /* the first two formulae ("0" and "1") must not be changed */
  if (OK && (i<2))
  {
    printf("WARNING (Biddy_Managed_AddFormula): formula %s not added because it has the same name as the existing constant!\n",x);
    return i;
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
          /* this is variable, not changed */
        } else {
          BiddyProlongOne(f,(unsigned int) c);
        }
      }
      if ((biddyManagerType == BIDDYTYPEZBDD) || (biddyManagerType == BIDDYTYPEZFDD))
      {
        if ((BiddyE(f) == biddyZero) && (BiddyT(f) == biddyTerminal)) {
          /* this is element, not changed */
        } else {
          BiddyProlongOne(f,(unsigned int) c);
        }
      }
      if ((biddyManagerType == BIDDYTYPEZBDDC) || (biddyManagerType == BIDDYTYPEZFDDC))
      {
        if ((BiddyE(f) == biddyTerminal) && (BiddyT(f) == biddyTerminal)) {
          /* this is element, not changed */
        }
        else {
          BiddyProlongOne(f, (unsigned int)c);
        }
      }
      if ((biddyManagerType == BIDDYTYPETZBDDC) || (biddyManagerType == BIDDYTYPETZBDD) ||
          (biddyManagerType == BIDDYTYPETZFDDC) || (biddyManagerType == BIDDYTYPETZFDD))
      {
        if ((BiddyE(f) == biddyZero) && (BiddyT(f) == biddyTerminal)) {
          /* this is variable/element, not changed */
        } else {
          BiddyProlongOne(f,(unsigned int) c);
        }
      }
    }
    return i;
  }

  /* formula must be added, indices of the first two formulae must not be changed */
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

  /* DEBUGGING */
  /*
  printf("Biddy_Managed_AddFormula: biddyFormulaTable.size = %u, i = %u\n",biddyFormulaTable.size,i);
  */

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
        /* this is variable, not changed */
      } else {
        BiddyProlongOne(f,(unsigned int) c);
      }
    }
    if ((biddyManagerType == BIDDYTYPEZBDD) || (biddyManagerType == BIDDYTYPEZFDD))
    {
      if ((BiddyE(f) == biddyZero) && (BiddyT(f) == biddyTerminal)) {
        /* this is element, not changed */
      } else {
        BiddyProlongOne(f,(unsigned int) c);
      }
    }
    if ((biddyManagerType == BIDDYTYPEZBDDC) || (biddyManagerType == BIDDYTYPEZFDDC))
    {
      if ((BiddyE(f) == biddyTerminal) && (BiddyT(f) == biddyTerminal)) {
        /* this is element, not changed */
      }
      else {
        BiddyProlongOne(f, (unsigned int)c);
      }
    }
    if ((biddyManagerType == BIDDYTYPETZBDDC) || (biddyManagerType == BIDDYTYPETZBDD) ||
        (biddyManagerType == BIDDYTYPETZFDDC) || (biddyManagerType == BIDDYTYPETZFDD))
    {
      if ((BiddyE(f) == biddyZero) && (BiddyT(f) == biddyTerminal)) {
        /* this is variable/element, not changed */
      } else {
        BiddyProlongOne(f,(unsigned int) c);
      }
    }
  }

  /* DEBUGGING */
  /*
  printf("Biddy_Managed_AddFormula: i=%d/%d, x=%s, f=%p\n",i,biddyFormulaTable.size-1,x,f);
  */

  if (!x) i = 0; /* if formula does not have a name return 0 */

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

  /* VARIABLES ARE CONSIDERED TO BE FORMULAE, [0] = constant variable 1 */
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

  /* THERE ARE ONLY TWO FORMULAE, I.E. 0 AND 1 */
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

  assert ( max <= biddyFormulaTable.size );

  /* DEBUGGING */
  /*
  fprintf(stderr,"biddyFormulaTable.size = %u, search for %s, min = %u (%s), max = %u (%s)\n",
          biddyFormulaTable.size,x,min,biddyFormulaTable.table[min].name,max,biddyFormulaTable.table[max].name);
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

  assert ( *idx <= biddyFormulaTable.size );

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
\brief Function Biddy_Managed_SetAlphabeticOrdering use variable swapping
        to create an alphabetic ordering.

### Description
    Named variables are ordered according to their names. Numbered variables
    are ordered according to their numbers and are always smaller (topmore) as
    any named variable.
### Side effects
### More info
    Macro Biddy_SetAlphabeticOrdering() is defined for use with anonymous
    manager.
*******************************************************************************/

#ifdef __cplusplus
extern "C" {
#endif

void
Biddy_Managed_SetAlphabeticOrdering(Biddy_Manager MNG)
{
  BiddyOrderingTable table;

  if (!MNG) MNG = biddyAnonymousManager;

  alphabeticOrdering(MNG,table);
  BiddySetOrdering(MNG,table);
}

#ifdef __cplusplus
}
#endif

/***************************************************************************//*!
\brief Function Biddy_Managed_SwapWithHigher swaps two adjacent variables.

### Description
    Higher (greater) variable is the bottommore one!
    The highest variable is constant variable "1".
    Global ordering is number of zeros in corresponding line of orderingTable.
    Constant variable '1' has global ordering greater than all others.
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
  } else if (biddyManagerType == BIDDYTYPEZBDD) {
    /* IMPLEMENTED */
    vhigh = swapWithHigher(MNG,v,&active);
  } else if (biddyManagerType == BIDDYTYPEZBDDC) {
    /* IMPLEMENTED */
    vhigh = swapWithHigher(MNG,v,&active);
  } else if (biddyManagerType == BIDDYTYPETZBDD) {
    /* IMPLEMENTED */
    vhigh = swapWithHigher(MNG,v,&active);
  } else if (biddyManagerType == BIDDYTYPETZBDDC)
  {
    fprintf(stderr,"Biddy_SwapWithHigher: this BDD type is not supported, yet!\n");
    return 0;
  } else if ((biddyManagerType == BIDDYTYPEOFDDC) || (biddyManagerType == BIDDYTYPEOFDD) ||
              (biddyManagerType == BIDDYTYPEZFDDC) || (biddyManagerType == BIDDYTYPEZFDD) ||
              (biddyManagerType == BIDDYTYPETZFDDC) || (biddyManagerType == BIDDYTYPETZFDD))
  {
    fprintf(stderr,"Biddy_SwapWithHigher: this BDD type is not supported, yet!\n");
    return 0;
  } else {
    fprintf(stderr,"Biddy_SwapWithHigher: Unsupported BDD type!\n");
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
  } else if (biddyManagerType == BIDDYTYPEZBDD) {
    /* IMPLEMENTED */
    vlow = swapWithLower(MNG,v,&active);
  } else if (biddyManagerType == BIDDYTYPEZBDDC) {
    /* IMPLEMENTED */
    vlow = swapWithLower(MNG,v,&active);
  } else if (biddyManagerType == BIDDYTYPETZBDD) {
    /* IMPLEMENTED */
    vlow = swapWithLower(MNG,v,&active);
  } else if (biddyManagerType == BIDDYTYPETZBDDC)
  {
    fprintf(stderr,"Biddy_SwapWithLower: this BDD type is not supported, yet!\n");
    return 0;
  } else if ((biddyManagerType == BIDDYTYPEOFDDC) || (biddyManagerType == BIDDYTYPEOFDD) ||
              (biddyManagerType == BIDDYTYPEZFDDC) || (biddyManagerType == BIDDYTYPEZFDD) ||
              (biddyManagerType == BIDDYTYPETZFDDC) || (biddyManagerType == BIDDYTYPETZFDD))
  {
    fprintf(stderr,"Biddy_SwapWithLower: this BDD type is not supported, yet!\n");
    return 0;
  } else {
    fprintf(stderr,"Biddy_SwapWithLower: Unsupported BDD type!\n");
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
    For TZBDD, all unreferenced nodes (not part of registered formulae)
    will be removed. For TZBDD, sifting may change top edge or even
    a top node of any function/formula - this is a problem, because functions
    referenced by local variables only may become wrong. Consequently, for
    TZBDDs, sifting is not safe to start automatically!
### More info
    Macro Biddy_Sifting(f) is defined for use with anonymous manager.
*******************************************************************************/

#ifdef __cplusplus
extern "C" {
#endif

Biddy_Boolean
Biddy_Managed_Sifting(Biddy_Manager MNG, Biddy_Edge f, Biddy_Boolean converge)
{
  Biddy_Boolean status;
  clock_t starttime;

  assert( (f == NULL) || (BiddyIsOK(f) == TRUE) );

  /* biddySiftingActive = TRUE; */

  if (!MNG) MNG = biddyAnonymousManager;
  ZF_LOGI("Biddy_Sifting");

  if (biddyManagerType == BIDDYTYPEOBDD) {
    /* IMPLEMENTED */
  } else if (biddyManagerType == BIDDYTYPEOBDDC) {
    /* IMPLEMENTED */
  } else if (biddyManagerType == BIDDYTYPEZBDD) {
    /* IMPLEMENTED */
  } else if (biddyManagerType == BIDDYTYPEZBDDC) {
    /* IMPLEMENTED */
  } else if (biddyManagerType == BIDDYTYPETZBDD) {
    /* IMPLEMENTED */
  } else if (biddyManagerType == BIDDYTYPETZBDDC)
  {
    fprintf(stderr,"Biddy_Sifting: this BDD type is not supported, yet!\n");
    return FALSE;
  } else if ((biddyManagerType == BIDDYTYPEOFDDC) || (biddyManagerType == BIDDYTYPEOFDD) ||
              (biddyManagerType == BIDDYTYPEZFDDC) || (biddyManagerType == BIDDYTYPEZFDD) ||
              (biddyManagerType == BIDDYTYPETZFDDC) || (biddyManagerType == BIDDYTYPETZFDD))
  {
    fprintf(stderr,"Biddy_Sifting: this BDD type is not supported, yet!\n");
    return FALSE;
  } else {
    fprintf(stderr,"Biddy_Sifting: Unsupported BDD type!\n");
    return FALSE;
  }

  /* DEBUGGING */
  /*
  printf("SIFTING START: num sifting: %u\n",Biddy_Managed_NodeTableSiftingNumber(MNG));
  */

  /* DEBUGGING */
  /*
  {
  Biddy_Variable n;
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
    printf("Initial number of dependent variables: %u\n",Biddy_Managed_DependentVariableNumber(MNG,f,FALSE));
    printf("Initial number of minterms: %.0f\n",Biddy_Managed_CountMinterms(MNG,f,0));
    printf("Initial number of nodes: %u\n",Biddy_Managed_CountNodes(MNG,f));
    printf("Top edge: %p\n",(void *) f);
    printf("Top variable: %s\n",Biddy_Managed_GetTopVariableName(MNG,f));
    printf("Expiry value of top node: %d\n",((BiddyNode *) BiddyP(f))->expiry);
  }
  printf("SIFTING: initial order\n");
  writeORDER(MNG);
  BiddySystemReport(MNG);
  for (n = 1; n < biddyFormulaTable.size; n++) {
    if (!Biddy_IsNull(biddyFormulaTable.table[n].f) &&
        !Biddy_IsTerminal(biddyFormulaTable.table[n].f))
    {
      assert( BiddyIsOK(biddyFormulaTable.table[n].f) );
    }
  }
  printf("========================================\n");
  }
  */

  biddyNodeTable.sifting++;
  starttime = clock();

  /* DELETE ALL CACHE TABLES AND DISABLE THEM */
  /* TO DO: user cache tables are not considered, yet */
  BiddyOPGarbageDeleteAll(MNG);
  BiddyEAGarbageDeleteAll(MNG);
  BiddyRCGarbageDeleteAll(MNG);
  BiddyReplaceGarbageDeleteAll(MNG);
  biddyOPCache.disabled = TRUE;
  biddyEACache.disabled = TRUE;
  biddyRCCache.disabled = TRUE;
  biddyReplaceCache.disabled = TRUE;

  if (!f) {
    /* MINIMIZING NODE NUMBER FOR THE WHOLE SYSTEM */
    /* WE REMOVE OBSOLETE NODES BECAUSE THEY SHOULD NOT BE CONSIDERED */
    /* WE DO NOT WANT TO USE PURGE HERE BECAUSE IT LIMITS THE USABILITY OF SIFTING */
    Biddy_Managed_ForceGC(MNG);
    status = BiddyGlobalSifting(MNG,converge);
  } else {
    /* MINIMIZING NODE NUMBER FOR THE GIVEN FUNCTION */
    status = BiddySiftingOnFunction(MNG,f,converge);
  }

  /* ENABLE ALL CACHE TABLES */
  /* TO DO: user caches are not considered, yet */
  biddyOPCache.disabled = FALSE;
  biddyEACache.disabled = FALSE;
  biddyRCCache.disabled = FALSE;
  biddyReplaceCache.disabled = FALSE;

  biddyNodeTable.drtime += clock() - starttime;

  /* DEBUGGING */
  /*
  {
  Biddy_Variable n;
  printf("========================================\n");
  printf("SIFTING FINISH: num sifting: %u\n",Biddy_Managed_NodeTableSiftingNumber(MNG));
  printf("SIFTING FINISH: num swapping: %u\n",Biddy_Managed_NodeTableSwapNumber(MNG));
  printf("SIFTING FINISH: num garbage collection: %u\n",Biddy_Managed_NodeTableGCNumber(MNG));
  if (f) {
    printf("Final number of dependent variables: %u\n",Biddy_Managed_DependentVariableNumber(MNG,f,FALSE));
    printf("Final number of minterms: %.0f\n",Biddy_Managed_CountMinterms(MNG,f,0));
    printf("Final number of nodes: %u\n",Biddy_Managed_CountNodes(MNG,f));
    printf("Top edge: %p\n",(void *) f);
    printf("Top variable: %s\n",Biddy_Managed_GetTopVariableName(MNG,f));
    printf("Expiry value of top node: %d\n",((BiddyNode *) BiddyP(f))->expiry);
  }
  printf("SIFTING: final order\n");
  writeORDER(MNG);
  BiddySystemReport(MNG);
  for (n = 1; n < biddyFormulaTable.size; n++) {
    if (!Biddy_IsNull(biddyFormulaTable.table[n].f) &&
        !Biddy_IsTerminal(biddyFormulaTable.table[n].f))
    {
      assert( BiddyIsOK(biddyFormulaTable.table[n].f) );
    }
  }
  printf("========================================\n");
  }
  */

  /* biddySiftingActive = FALSE; */

  return status; /* sifting has been performed */
}

#ifdef __cplusplus
}
#endif

/***************************************************************************//*!
\brief Function Biddy_Managed_MinimizeBDD reorders variables to minimize
        the node number of the given formula using an exhaustive search
        over all possible orderings.

### Description
    Steinhaus–Johnson–Trotter algorithm is used to generate all possible
    permutations. An optimized version of Bubble Sort is used to
    setup the final ordering. Variables are reordered globally. All obsolete
    nodes will be removed.
### Side effects
    Indeed, this function may take a lot of time!
    For TZBDD, all unreferenced nodes (not part of registered formulae)
    will be removed. For TZBDD, this function may change top edge or even
    a top node of any function/formula - this is a problem, because functions
    referenced by local variables only may become wrong. Consequently, for
    TZBDDs, sifting is not safe to start automatically!
### More info
    Macro Biddy_MinimizeBDD(f) is defined for use with anonymous manager.
*******************************************************************************/

#ifdef __cplusplus
extern "C" {
#endif

void
Biddy_Managed_MinimizeBDD(Biddy_Manager MNG, Biddy_String name)
{
  Biddy_Edge f;
  unsigned int idx;
  unsigned int n,MIN;
  BiddyOrderingTable finalOrdering;
  Biddy_Boolean first;

  /* DEBUGGING */
  /*
  if (Biddy_Managed_FindFormula(MNG,name,&idx,&f)) {
    printf("MINIMIZE FOR %s STARTED\n",name);
    n = Biddy_Managed_CountNodes(MNG,f);
    printf("START: n=%u\n",n);
  } else {
    printf("MINIMIZE: FORMULA %s NOT FOUND\n",name);
  }
  */

  BiddySjtInit(MNG);

  MIN = 0;
  first = TRUE;
  do {
    Biddy_Managed_FindFormula(MNG,name,&idx,&f);

    /* use this to minimize the number of internal nodes */
    /*
    n = 0;
    Biddy_Managed_SelectNode(MNG,biddyTerminal);
    BiddyNodeNumber(MNG,f,&n);
    Biddy_Managed_DeselectAll(MNG);
    */

    /* use this to minimize the number of all nodes */
    /**/
    n = Biddy_Managed_CountNodes(MNG,f);
    /**/

    /* DEBUGGING */
    /*
    printf("MINIMIZATION STEP: n=%u\n",n);
    */

    if (first || (n < MIN)) {
      MIN = n;
      memcpy(finalOrdering,biddyOrderingTable,sizeof(BiddyOrderingTable));
    }

    first = FALSE;

  } while(BiddySjtStep(MNG));

  BiddySjtExit(MNG);

  BiddySetOrdering(MNG,finalOrdering);

  /* DEBUGGING */
  /*
  Biddy_Managed_FindFormula(MNG,name,&idx,&f);
  n = Biddy_Managed_CountNodes(MNG,f);
  printf("FINAL: n=%u\n",n);
  */
}

#ifdef __cplusplus
}
#endif

/***************************************************************************//*!
\brief Function Biddy_Managed_MaximizeBDD reorders variables to maximize
        the node number of the given function using an exhaustive search
        over all possible orderings.

### Description
    Steinhaus–Johnson–Trotter algorithm is used to generate all possible
    permutations. An optimized version of Bubble Sort is used to
    setup the final ordering. Variables are reordered globally. All obsolete
    nodes will be removed.
### Side effects
    Indeed, this function may take a lot of time!
    For TZBDD, all unreferenced nodes (not part of registered formulae)
    will be removed. For TZBDD, this function may change top edge or even
    a top node of any function/formula - this is a problem, because functions
    referenced by local variables only may become wrong. Consequently, for
    TZBDDs, sifting is not safe to start automatically!
### More info
    Macro Biddy_MaximizeBDD(f) is defined for use with anonymous manager.
*******************************************************************************/

#ifdef __cplusplus
extern "C" {
#endif

void
Biddy_Managed_MaximizeBDD(Biddy_Manager MNG, Biddy_String name)
{
  Biddy_Edge f;
  unsigned int idx;
  unsigned int n,MAX;
  BiddyOrderingTable finalOrdering;

  /* DEBUGGING */
  /*
  if (Biddy_Managed_FindFormula(MNG,name,&idx,&f)) {
    printf("MAXIMIZE FOR %s STARTED\n",name);
    n = Biddy_Managed_CountNodes(MNG,f);
    printf("START: n=%u\n",n);
  } else {
    printf("MAXIMIZE: FORMULA %s NOT FOUND\n",name);
  }
  */

  BiddySjtInit(MNG);
  MAX = 0;
  do {
    Biddy_Managed_FindFormula(MNG,name,&idx,&f);

    /* use this to maximize the number of internal nodes */
    /*
    n = 0;
    Biddy_Managed_SelectNode(MNG,biddyTerminal);
    BiddyNodeNumber(MNG,f,&n);
    Biddy_Managed_DeselectAll(MNG);
    */

    /* use this to maximize the number of all nodes */
    /**/
    n = Biddy_Managed_CountNodes(MNG,f);
    /**/

    /* DEBUGGING */
    /*
    printf("MAXIMIZATION STEP: n=%u\n",n);
    */

    if (!MAX || (n > MAX)) {
      MAX = n;
      memcpy(finalOrdering,biddyOrderingTable,sizeof(BiddyOrderingTable));
    }
  } while(BiddySjtStep(MNG));

  BiddySjtExit(MNG);

  BiddySetOrdering(MNG,finalOrdering);

  /* DEBUGGING */
  /*
  Biddy_Managed_FindFormula(MNG,name,&idx,&f);
  n = Biddy_Managed_CountNodes(MNG,f);
  printf("FINAL: n=%u\n",n);
  */
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
    If (f == biddyZero) then only the complete domain (all variables) is
    copied.
### Side effects
    If source and target manager are the same then function does nothing.
    The variable ordering of the created BDD is trying to follow the original
    ordering, but if some variables already exist in the target manager then
    the final ordering is adapted to the target manager.
    Please note, that indices of variables in the target manager may not be
    the same as in the source manager (for example, if source managet does not
    use initial ordering the indices in the target manager will follow the
    variable's ordering and not variable's original indices)
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
  Biddy_Variable v1,v2,k;
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
    v1 = Biddy_Managed_GetLowestVariable(MNG1); /* lowest = topmost */
    for (k = 1; k < biddyVariableTable1.num; k++) {
      vname = Biddy_Managed_GetVariableName(MNG1,v1);
      v2 = Biddy_Managed_AddVariableByName(MNG2,vname);
      Biddy_Managed_SetVariableData(MNG2,v2,Biddy_Managed_GetVariableData(MNG1,v1));
      v1 = biddyVariableTable1.table[v1].next;
    }
  }

  else if ((biddyManagerType2 == BIDDYTYPEZBDDC) || (biddyManagerType2 == BIDDYTYPEZBDD) ||
            (biddyManagerType2 == BIDDYTYPEZFDDC) || (biddyManagerType2 == BIDDYTYPEZFDD) ||
            (biddyManagerType2 == BIDDYTYPETZBDDC) || (biddyManagerType2 == BIDDYTYPETZBDD) ||
            (biddyManagerType2 == BIDDYTYPETZFDDC) || (biddyManagerType2 == BIDDYTYPETZFDD))
  {
    v1 = 0;
    for (k = 1; k < biddyVariableTable1.num; k++) {
      v1 = biddyVariableTable1.table[v1].prev;
      vname = Biddy_Managed_GetVariableName(MNG1,v1);
      v2 = Biddy_Managed_AddVariableByName(MNG2,vname);
      Biddy_Managed_SetVariableData(MNG2,v2,Biddy_Managed_GetVariableData(MNG1,v1));
    }
  }

  else {
    fprintf(stderr,"Biddy_Managed_Copy: Unsupported BDD type for MNG2!\n");
  }

  /* DEBUGGING - PRINT BOTH DOMAINS */
  /*
  printf("Biddy_Copy: SOURCE DOMAIN FOR %s\n",Biddy_Managed_GetManagerName(MNG1));
  v1 = 0;
  for (k = 1; k < biddyVariableTable1.num; k++) {
    vname = Biddy_Managed_GetVariableName(MNG1,k);
    printf("(%u)%s,",k,vname);
    v1 = biddyVariableTable1.table[v1].prev;
  }
  printf("\n");
  for (k = 1; k < biddyVariableTable1.num; k++) {
    vname = Biddy_Managed_GetVariableName(MNG1,v1);
    printf("%s,",vname);
    v1 = biddyVariableTable1.table[v1].next;
  }
  printf("\n");
  printf("Biddy_Copy: TARGET DOMAIN FOR %s\n",Biddy_Managed_GetManagerName(MNG2));
  v1 = 0;
  for (k = 1; k < biddyVariableTable2.num; k++) {
    vname = Biddy_Managed_GetVariableName(MNG2,k);
    printf("(%u)%s,",k,vname);
    v1 = biddyVariableTable2.table[v1].prev;
  }
  printf("\n");
  for (k = 1; k < biddyVariableTable2.num; k++) {
    vname = Biddy_Managed_GetVariableName(MNG2,v1);
    printf("%s,",vname);
    v1 = biddyVariableTable2.table[v1].next;
  }
  printf("\n");
  if (simplecopy) printf("Biddy_Copy: SIMPLE COPY!\n");
  if (f == biddyZero1) printf("Biddy_Copy: DOMAIN COPIED AND FINISHED!\n");
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
          r = BiddyManagedTaggedFoaNode(MNG2,prevvar,r,r,prevvar,TRUE);
          BiddyProlongOne(r,biddySystemAge2); /* FoaNode returns an obsolete node! */
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
          v2 = Biddy_Managed_GetVariable(MNG2,topname);
          r = BiddyManagedTaggedFoaNode(MNG2,v2,r,biddyZero2,v2,TRUE);
          BiddyProlongOne(r,biddySystemAge2); /* FoaNode returns an obsolete node! */
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
        /* determine topmost variable of f in MNG1 considered by BiddyConvertDirect (by checking the topmost variable of f in MNG1) */
        prevvar = BiddyV(f);
        /* add all variables to f in MNG2 which are missing in MNG1 */
        if (prevvar != top) {
          topname = Biddy_Managed_GetVariableName(MNG1,top);
          v2 = Biddy_Managed_GetVariable(MNG2,topname);
          Biddy_SetTag(r,v2);
        }
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
          r = BiddyManagedTaggedFoaNode(MNG2,prevvar,r,r,prevvar,TRUE);
          BiddyProlongOne(r,biddySystemAge2); /* FoaNode returns an obsolete node! */
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
        fprintf(stderr,"Biddy_Managed_Copy: Unsupported BDD type combination for MNG1 or MNG2!\n");
        assert ( FALSE );
      }

    }

  } else {

    if ((biddyManagerType1 == BIDDYTYPEOBDDC) || (biddyManagerType1 == BIDDYTYPEOBDD)) {
      r = BiddyCopyOBDD(MNG1,MNG2,f);
      BiddyProlongOne(r,biddySystemAge2); /* not always refreshed by BiddyCopy */
    }
    else if ((biddyManagerType1 == BIDDYTYPEZBDDC) || (biddyManagerType1 == BIDDYTYPEZBDD)) {
      r = BiddyCopyZBDD(MNG1,MNG2,f);
      BiddyProlongOne(r,biddySystemAge2); /* not always refreshed by BiddyCopy */
    }
    else if ((biddyManagerType1 == BIDDYTYPETZBDDC) || (biddyManagerType1 == BIDDYTYPETZBDD)) {
      r = BiddyCopyTZBDD(MNG1,MNG2,f);
      BiddyProlongOne(r,biddySystemAge2); /* not always refreshed by BiddyCopy */
    }
    else {
      fprintf(stderr,"Biddy_Managed_Copy: Unsupported BDD type for MNG1!\n");
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
    The created formula is refreshed but not preserved.
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

  /* DEBUGGING */
  /*
  printf("Biddy_Managed_CopyFormula STARTED: %s\n",x);
  */

  OK = Biddy_Managed_FindFormula(MNG1,x,&i,&src);

  if (OK) {
    OK = Biddy_Managed_FindFormula(MNG2,x,&i,&dst);
    if (OK) {
      printf("WARNING (Biddy_Managed_CopyFormula): formula overwritten!\n");
    }
    if (src == biddyZero1) {
      dst = biddyZero2;
    } else {
      dst = Biddy_Managed_Copy(MNG1,MNG2,src);
    }
    Biddy_Managed_AddFormula(MNG2,x,dst,-1);
  }

  /* DEBUGGING */
  /*
  printf("Biddy_Managed_CopyFormula FINISHED: %s\n",x);
  */
}

#ifdef __cplusplus
}
#endif

/**Function****************************************************************
  Synopsis    [Function Biddy_Managed_ConstructBDD.]
  Description [Biddy_Managed_ConstructBDD constructs BDD from lists of
               nodes and edges. In both lists, elements are separated
               with spaces. Elements of node list has the following format:
               terminal node zero = (type=0,l=-1,r=-1),
               terminal node one = (type=1,l=-1,r=-1),
               regular label = (type=2,r=-1),
               complemented label = (type=3,r=-1),
               regular node = (type=4),
               node with complemented successor(s) = (type=5)
               The first element in nodelist is ignored. An example node
               list is: 'Biddy 0 Biddy 2 1 -1 1 B 4 2 3 2 0 0 -1 -1 3 i 4 4
               9 4 d 4 5 6 5 0 0 -1 -1 6 y 4 7 8 7 0 0 -1 -1 8 1 1 -1 -1 9
               d 4 6 10 10 1 1 -1 -1' which is constructed as: '{Biddy}
               {0 Biddy 2 1 -1} {1 B 4 2 3} {2 0 0 -1 -1} {3 i 4 4 9}
               {4 d 4 5 6} {5 0 0 -1 -1} {6 y 4 7 8} {7 0 0 -1 -1}
               {8 1 1 -1 -1} {9 d 4 6 10} {10 1 1 -1 -1}'.
               ]
  SideEffects [If variable ordering in the file is not compatible
               with the active ordering then the result will be wrong.]
  SeeAlso     []
  ************************************************************************/

#ifdef __cplusplus
extern "C" {
#endif

Biddy_Edge
Biddy_Managed_ConstructBDD(Biddy_Manager MNG, int numV, Biddy_String varlist, int numN,
                           Biddy_String nodelist)
{
  BiddyVariableOrder *tableV;
  BiddyNodeList *tableN;
  Biddy_Boolean usetags;
  Biddy_Edge r;
  Biddy_String word;
  int i; /* must be signed because of used for loop */

  if (!MNG) MNG = biddyAnonymousManager;

  /* DEBUGGING */
  /*
  printf("Biddy_ConstructBDD\n");
  printf("%d variables\n",numV);
  printf("%s\n",varlist);
  printf("%d nodes\n",numN);
  printf("%s\n",nodelist);
  */

  tableV = (BiddyVariableOrder *) malloc(numV * sizeof(BiddyVariableOrder));
  if (!tableV) return biddyNull;
  tableN = (BiddyNodeList *) malloc(numN * sizeof(BiddyNodeList));
  if (!tableN) return biddyNull;

  /* VARIABLES */
  if (numV) {
    word = strtok(varlist," "); /* parsing list of variables */
    tableV[0].name = strdup(word);
    tableV[0].order = 0;
    for (i=1; i<numV; i++) {
      word = strtok(NULL," ");
      tableV[i].name = strdup(word);
      tableV[i].order = 0;
    }
  }

  /* DEBUGGING */
  /*
  printf("VARIABLES %d:\n",numV);
  for (i=0; i<numV; i++) {
    printf("%s\n",tableV[i].name);
  }
  */

  usetags = FALSE;
  if ((biddyManagerType == BIDDYTYPEOBDDC) || (biddyManagerType == BIDDYTYPEOBDD))
  {
    for (i=0; i<numV; i++) {
      Biddy_Managed_AddVariableByName(MNG,tableV[i].name);
    }
  }
  else if ((biddyManagerType == BIDDYTYPEZBDDC) || (biddyManagerType == BIDDYTYPEZBDD))
  {
    for (i=numV-1; i>=0; i--) {
      Biddy_Managed_AddVariableByName(MNG,tableV[i].name);
    }
  }
  else if ((biddyManagerType == BIDDYTYPETZBDDC) || (biddyManagerType == BIDDYTYPETZBDD))
  {
    usetags = TRUE;
    for (i=numV-1; i>=0; i--) {
      Biddy_Managed_AddVariableByName(MNG,tableV[i].name);
    }
  }
  else {
    fprintf(stderr,"Biddy_ConstructBDD: Unknown BDD type!\n");
    return biddyNull;
  }

  /* NODES AND EDGES */
  word = strtok(nodelist," "); /* ignored */
  for (i=0; i<numN; i++) {
    word = strtok(NULL," "); /* node id */
    tableN[i].id = atoi(word);
    word = strtok(NULL," "); /* node name */
    tableN[i].name = strdup(word);
    word = strtok(NULL," "); /* node type */
    tableN[i].type = atoi(word);
    word = strtok(NULL," "); /* node left successor */
    tableN[i].l = atoi(word);
    if (usetags) {
      word = strtok(NULL," "); /* left successor's tag */
      if (tableN[i].l != -1) {
        tableN[i].ltag = Biddy_Managed_GetVariable(MNG,word);
      } else {
        tableN[i].ltag = 0;
      }
    } else {
      tableN[i].ltag = 0;
    }
    word = strtok(NULL," "); /* node right successor */
    tableN[i].r = atoi(word);
    if (usetags) {
      word = strtok(NULL," "); /* right successor's tag */
      if (tableN[i].r != -1) {
        tableN[i].rtag = Biddy_Managed_GetVariable(MNG,word);
      } else {
        tableN[i].rtag = 0;
      }
    } else {
      tableN[i].rtag = 0;
    }
  }

  r = BiddyConstructBDD(MNG,numN,tableN);

  for (i=0; i<numV; ++i) {
    free(tableV[i].name);
  }
  for (i=0; i<numN; ++i) {
    free(tableN[i].name);
  }

  free(tableV);
  free(tableN);

  return r;
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
    biddySystemAge--;
    BiddyCompactSystemAge(MNG);
    biddySystemAge++;
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
    exit(1);
  }
}

/***************************************************************************//*!
\brief Function BiddyCompactSystemAge make system age to be represented with
        as small number as possible.

### Description
    Statuses of nodes are not changed.
### Side effects
### More info
*******************************************************************************/

void
BiddyCompactSystemAge(Biddy_Manager MNG)
{
  Biddy_Variable v;
  BiddyNode *sup;
  unsigned int i;

  /* DEBUGGING */
  /**/
  printf("NOTE FROM BIDDY: BiddyCompactSystemAge\n");
  /**/

  /* CHANGE expiry VALUE OF ALL NODES */
  for (v=1; v<biddyVariableTable.num; v++) {
    sup = biddyVariableTable.table[v].firstNode;
    if (sup->expiry) {
      if (sup->expiry < biddySystemAge) {
        sup->expiry = 1;
      } else {
        sup->expiry = 2+sup->expiry-biddySystemAge;
      }
    }
    sup = (BiddyNode *) sup->list;
  }

  /* CHANGE expiry VALUE OF ALL FORMULAE */
  for (i = 2; i < biddyFormulaTable.size; i++) {
    if (biddyFormulaTable.table[i].expiry) {
      if (biddyFormulaTable.table[i].expiry) {
        biddyFormulaTable.table[i].expiry = biddyFormulaTable.table[i].expiry;
      } else {
        biddyFormulaTable.table[i].expiry = 2+biddyFormulaTable.table[i].expiry-biddySystemAge;
      }
    }
  }

  biddySystemAge = 2;
}

/***************************************************************************//*!
\brief Function BiddyProlongRecursively prolonges obsolete nodes in a function.

### Description
    Nodes are prolonged recursively until a non-obsolete node is reached.
### Side effects
    The goal is to make nodes non-obsolete and not to give them correct expiry
    value.
    If (target != 0) then not all obsolete nodes are prolonged - it is
    expected that GC will not delete nodes which are skipped for optimization.
### More info
*******************************************************************************/

void
BiddyProlongRecursively(Biddy_Manager MNG, Biddy_Edge f, unsigned int c,
                        Biddy_Variable target)
{
  if ((target != 0) && !BiddyIsSmaller(BiddyV(f),target)) return;
  if (((BiddyNode *) BiddyP(f))->expiry &&
      (((BiddyNode *) BiddyP(f))->expiry < biddySystemAge))
  {
    BiddyProlongOne(f,c);
    if (biddyVariableTable.table[BiddyV(f)].numobsolete != 0) {
      (biddyVariableTable.table[BiddyV(f)].numobsolete)--;
    }
    BiddyProlongRecursively(MNG,BiddyE(f),c,target);
    BiddyProlongRecursively(MNG,BiddyT(f),c,target);
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
    Local info for terminal node is not created!
### More info
*******************************************************************************/

Biddy_Variable
BiddyCreateLocalInfo(Biddy_Manager MNG, Biddy_Edge f)
{
  unsigned int num;
  BiddyLocalInfo *c;
  Biddy_Variable i,var;

  if (Biddy_IsNull(f)) return 0;
  if (Biddy_IsTerminal(f)) return 0;

  assert( !Biddy_Managed_IsSelected(MNG,f) );

  num = 0;  /* TERMINAL NODE IS NOT COUNTED HERE */

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
    fprintf(stderr,"BiddyCreateLocalInfo: this BDD type is not supported, yet!\n");
    return 0;
  }
  else {
    fprintf(stderr,"BiddyCreateLocalInfo: Unsupported BDD type!\n");
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
    fprintf(stderr,"BiddyCreateLocalInfo: this BDD type is not supported, yet!\n");
    return 0;
  }
  else {
    fprintf(stderr,"BiddyCreateLocalInfo: Unsupported BDD type!\n");
    return 0;
  }

  /* calloc is used here because we want initialization to zero */
  if (!(biddyLocalInfo = (BiddyLocalInfo *)
        calloc((num+1),sizeof(BiddyLocalInfo)))) {
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
    All nodes except terminal node must be selected.
    Local info for terminal node should not be created!
### More info
*******************************************************************************/

void
BiddyDeleteLocalInfo(Biddy_Manager MNG, Biddy_Edge f)
{
  if (Biddy_IsNull(f)) return;
  if (Biddy_IsTerminal(f)) return;
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
    This should not be called for terminal node because terminal node does
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
    This should not be called for terminal node because terminal node does
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
    This should not be called for terminal node because terminal node does
    not have local info!
### More info
*******************************************************************************/

unsigned int
BiddyGetEnumerator(Biddy_Edge f)
{
  return ((BiddyLocalInfo *)(((BiddyNode *)BiddyP(f))->list))->data.enumerator;
}

/*******************************************************************************
\brief Function BiddySetCopy is used in BiddyCopy.

### Description
### Side effects
    Local info must be created before using this function.
    This should not be called for terminal node because terminal node does
    not have local info!
### More info
*******************************************************************************/

void
BiddySetCopy(Biddy_Edge f, BiddyNode *n)
{
  ((BiddyLocalInfo *)(((BiddyNode *)BiddyP(f))->list))->data.copy = n;
}

/*******************************************************************************
\brief Function BiddyGetCopy is used in BiddyCopy.

### Description
### Side effects
    Local info must be created before using this function.
    This should not be called for terminal node because terminal node does
    not have local info!
### More info
*******************************************************************************/

BiddyNode *
BiddyGetCopy(Biddy_Edge f)
{
  return ((BiddyLocalInfo *)(((BiddyNode *)BiddyP(f))->list))->data.copy;
}

/*******************************************************************************
\brief Function BiddySetPath1Count is used in pathCount().

### Description
### Side effects
    Local info must be created before using this function.
    This should not be called for terminal node because terminal node does
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
    This should not be called for terminal node because terminal node does
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
    This should not be called for terminal node because terminal node does
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
    This should not be called for terminal node because terminal node does
    not have local info!
### More info
*******************************************************************************/

unsigned long long int
BiddyGetPath0Count(Biddy_Edge f)
{
  return ((BiddyLocalInfo *)(((BiddyNode *) BiddyP(f))->list))->datax.path0Count;
}

/*******************************************************************************
\brief Function BiddySetPath1ProbabilitySum is used in pathProbabilitySum().

### Description
### Side effects
    Local info must be created before using this function.
    This should not be called for terminal node because terminal node does
    not have local info!
### More info
*******************************************************************************/

void
BiddySetPath1ProbabilitySum(Biddy_Edge f, double value)
{
  ((BiddyLocalInfo *)(((BiddyNode *) BiddyP(f))->list))->data.path1Probability = value;
}

/*******************************************************************************
\brief Function BiddyGetPath1ProbabilitySum is used in pathProbabilitySum().

### Description
### Side effects
    Local info must be created before using this function.
    This should not be called for terminal node because terminal node does
    not have local info!
### More info
*******************************************************************************/

double
BiddyGetPath1ProbabilitySum(Biddy_Edge f)
{
  return ((BiddyLocalInfo *)(((BiddyNode *) BiddyP(f))->list))->data.path1Probability;
}

/*******************************************************************************
\brief Function BiddySetPath0ProbabilitySum is used in pathProbabilitySum().

### Description
### Side effects
    Local info must be created before using this function.
    This should not be called for terminal node because terminal node does
    not have local info!
### More info
*******************************************************************************/

void
BiddySetPath0ProbabilitySum(Biddy_Edge f, double value)
{
  ((BiddyLocalInfo *)(((BiddyNode *) BiddyP(f))->list))->datax.path0Probability = value;
}

/*******************************************************************************
\brief Function BiddyGetPath0ProbabilitySum is used in pathProbabilitySum().

### Description
### Side effects
    Local info must be created before using this function.
    This should not be called for terminal node because terminal node does
    not have local info!
### More info
*******************************************************************************/

double
BiddyGetPath0ProbabilitySum(Biddy_Edge f)
{
  return ((BiddyLocalInfo *)(((BiddyNode *) BiddyP(f))->list))->datax.path0Probability;
}

/*******************************************************************************
\brief Function BiddySetLeftmost is used in pathCount().

### Description
### Side effects
    Local info must be created before using this function.
    This should not be called for terminal node because terminal node does
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
    This should not be called for terminal node because terminal node does
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
    This should not be called for terminal node because terminal node does
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
    This should not be called for terminal node because terminal node does
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
    This should not be called for terminal node because terminal node does
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
    This should not be called for terminal node because terminal node does
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

/***************************************************************************//*!
\brief Function BiddyGlobalSifting reorders variables to minimize node number
       for the whole system using Rudell's sifting algorithm.

### Description
    Variables are reordered globally.
    All obsolete nodes will be removed.
### Side effects
    It is required that all obsolete formula and nodes and also all nodes which
    are not part of the remaining formula are removed before calling this
    function. For TZBDD, sifting may change top edge or even a top node of
    any function/formula - this is a problem, because functions referenced by
    local variables only may become wrong. Consequently, for TZBDDs, sifting
    is not safe to start automatically!
### More info
*******************************************************************************/

Biddy_Boolean
BiddyGlobalSifting(Biddy_Manager MNG, Biddy_Boolean converge)
{
  unsigned int num,min,min2,vnum,vv,totalbest;
  unsigned int best1,best2;
  Biddy_Variable v,vmax,k,n,minvar,maxvar;
  Biddy_Boolean highfirst,stop,finish,active,goback;
  unsigned int varTable[BIDDYVARMAX];
  float treshold;

  /* DEBUGGING */
  /*
  printf("SIFTING START: numNodes=%u, systemAge=%u\n",Biddy_Managed_NodeTableNum(MNG),biddySystemAge);
  BiddySystemReport(MNG);
  */

  /* determine current number of nodes in the global system */
  num = Biddy_Managed_NodeTableNum(MNG);

  /* determine minvar (lowest == topmost) and maxvar (highest == bottommost) */
  minvar = Biddy_Managed_GetLowestVariable(MNG);
  maxvar = 0;

  /* DEBUGGING - CHECKING THE SOUNDNESS OF THE ACTIVE ORDERING */
  /*
  {
  Biddy_Variable k,minvarOrder,maxvarOrder;
  minvarOrder = maxvarOrder = Biddy_Managed_VariableTableNum(MNG);
  for (k=0; k<Biddy_Managed_VariableTableNum(MNG); k++) {
    if (GET_ORDER(biddyOrderingTable,minvar,k)) minvarOrder--;
    if (GET_ORDER(biddyOrderingTable,maxvar,k)) maxvarOrder--;
  }
  if (minvarOrder != 1) {
    fprintf(stdout,"ERROR (BiddyGlobalSifting): minvarOrder %u is wrong\n",minvarOrder);
    exit(1);
  }
  if (maxvarOrder != Biddy_Managed_VariableTableNum(MNG)) {
    fprintf(stdout,"ERROR (BiddyGlobalSifting): maxvarOrder %u is wrong\n",maxvarOrder);
    exit(1);
  }
  }
  */

  /* SIFTING ALGORITHM */
  /* EXAMINE EVERY VARIABLE ONCE! */

  /* WE ARE STARTING WITH A CURRENT NUMBER OF NODES */
  if (converge) {
    treshold = biddyNodeTable.convergesiftingtreshold;
  } else {
    treshold = biddyNodeTable.siftingtreshold;
  }
  min = num;

  totalbest = min+1;
  while (min < totalbest) {
  totalbest = min;

  /* INIT varTable */
  /* varTable is used in the following way: */
  /* varTable[i] == 1 iff variable has not been sifted, yet */
  /* varTable[i] == 2 iff variable has already been sifted */

  varTable[0] = 2; /* variable '1' is not swapped */
  for (k=1; k<biddyVariableTable.num; k++) {
    varTable[k] = 1;
  }

  /* CHOOSE THE VARIABLE THAT WILL BE SIFTED FIRST */
  /* 1. it has not been sifted, yet */
  /* 2. it has max number of nodes */

  finish = (varTable[minvar] == 2); /* maybe, we are sifting constant variable '1' */
  v = 0;
  if (!finish) {
    /* VARIANT 1 */
    /* choose a variable that has the max number of nodes */
    vmax = 1;
    vnum = biddyVariableTable.table[1].num;
    v = 2;
    while (v < biddyVariableTable.num) {
      if ((vv=biddyVariableTable.table[v].num) > vnum) {
        vmax = v;
        vnum = vv;
      }
      v++;
    }
    v = vmax;
    /* VARIANT 2 */
    /* simply choose the smallest (topmost) variable */
    /*
    v = minvar;
    */
  }

  while (!finish) {
    varTable[v] = 2;

    /* DETERMINE FIRST DIRECTION */
    if (v == minvar) { /* TOPMOST VARIABLE */
      stop = TRUE; /* skip first direction */
      highfirst = FALSE; /* swapWithHigher will be used in second direction */
    } else if (v == maxvar) { /* BOTTOMMOST VARIABLE */
      stop = TRUE; /* skip first direction */
      highfirst = TRUE; /* swapWithLower will be used in second direction */
    } else {
      stop = FALSE;

      /* VARIANT A: start with shorter direction, thus calculate v's global ordering */
      /**/
      n = getGlobalOrdering(MNG,v);
      if (n > (1+biddyVariableTable.num)/2) highfirst = TRUE; else highfirst = FALSE;
      /**/

      /* VARIANT B: start with highfirst = TRUE */
      /*
      highfirst = TRUE;
      */
    }

    /* PROFILING */
    /*
    {
    Biddy_Variable varOrder;
    varOrder = getGlobalOrdering(MNG,v);
    printf("SIFTING LOOP START WITH variable \"%s\" (v=%u): startOrder=%u, min=%u, numNodes=%u, systemAge=%u, ",
           Biddy_Managed_GetVariableName(MNG,v),v,varOrder,min,Biddy_Managed_NodeTableNum(MNG),biddySystemAge);
    if (highfirst) printf("HIGHFIRST\n"); else printf("LOWFIRST\n");
    }
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

      if (highfirst) {
        if (v == minvar) {
          minvar = k;  /* update minvar */
        }
        if (k == maxvar) {
          maxvar = v; /* update maxvar */
          stop = TRUE;
        }
      } else {
        if (k == minvar) {
          minvar = v; /* update minvar */
          stop = TRUE;
        }
        if (v == maxvar) {
          maxvar = k;  /* update maxvar */
        }
      }

      /* NUMBER OF NODES CAN CHANGE ONLY IF v AND k HAVE BEEN ACTIVELY SWAPPED */
      /* FOR OBDDs AND ZBDDs, ONLY NODES WITH (v == smaller(k,v)) CAN BE OBSOLETE */
      /* FOR TZBDs, ALL NODES CAN BE OBSOLETE ?? */
      if (active) {

        /* DETERMINE THE NUMBER OF USEFUL NODES */
        if ((biddyManagerType == BIDDYTYPEOBDDC) || (biddyManagerType == BIDDYTYPEOBDD)) {
          /* Biddy_Managed_GC(MNG,0,0,FALSE,TRUE); */ /* try this if there are strange problems */
          Biddy_Managed_GC(MNG,
                           BiddyIsSmaller(k,v)?v:k,
                           BiddyIsSmaller(k,v)?k:v,
                           FALSE,TRUE);
          num = biddyNodeTable.num;
        }
        else if ((biddyManagerType == BIDDYTYPEZBDDC) || (biddyManagerType == BIDDYTYPEZBDD)) {
          /* Biddy_Managed_GC(MNG,0,0,FALSE,TRUE); */ /* try this if there are strange problems */
          Biddy_Managed_GC(MNG,
                           BiddyIsSmaller(k,v)?v:k,
                           BiddyIsSmaller(k,v)?k:v,
                           FALSE,TRUE);
          num = biddyNodeTable.num;
        }
        else if ((biddyManagerType == BIDDYTYPETZBDDC) || (biddyManagerType == BIDDYTYPETZBDD)) {
          Biddy_Managed_GC(MNG,0,0,FALSE,TRUE);
          num = biddyNodeTable.num;
        }
        else {
          Biddy_Managed_GC(MNG,0,0,FALSE,TRUE);
          num = biddyNodeTable.num;
        }

        /* DEBUGGING */
        /*
        printf("SIFTING: first direction, step=%u, min=%u, num=%u\n",n,min,num);
        */

        if (num < min) { /* first direction should use strict less */
          min = num;
          best1 = n;
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

      }
    } /* while (!stop ...) */

    /* PROFILING */
    /*
    {
    Biddy_Variable varOrder;
    varOrder = getGlobalOrdering(MNG,v);
    printf("SIFTING BEFORE GO-BACK: variable: \"%s\" (v=%u), currentOrder=%u, min=%u, numNodes=%u\n",
           Biddy_Managed_GetVariableName(MNG,v),v,varOrder,min,Biddy_Managed_NodeTableNum(MNG));
    }
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
    goback = (n > 0);
    if (goback) vmax = biddyVariableTable.table[v].next; /* save the initial position of v */
    while (n) {
      if (highfirst) {
        k = swapWithLower(MNG,v,&active);

        if ((biddyManagerType == BIDDYTYPETZBDDC) || (biddyManagerType == BIDDYTYPETZBDD)) {
          Biddy_Managed_GC(MNG,0,0,FALSE,TRUE);
        }

        if (v == maxvar) {
          maxvar = k;
        }
        if (k == minvar) {
          minvar = v;
        }
      } else {
        k = swapWithHigher(MNG,v,&active);

        if ((biddyManagerType == BIDDYTYPETZBDDC) || (biddyManagerType == BIDDYTYPETZBDD)) {
          Biddy_Managed_GC(MNG,0,0,FALSE,TRUE);
        }

        if (v == minvar) {
          minvar = k;
        }
        if (k == maxvar) {
          maxvar = v;
        }
      }
      n--;
    }

    /* PROFILING */
    /*
    {
    Biddy_Variable varOrder;
    varOrder = getGlobalOrdering(MNG,v);
    printf("SIFTING AFTER GO BACK: variable: \"%s\" (v=%u), currentOrder=%u, min=%u, numNodes=%u\n",
           Biddy_Managed_GetVariableName(MNG,v),v,varOrder,min,Biddy_Managed_NodeTableNum(MNG));
    }
    */

    /* IF GO-BACK WAS MOVING VARIABLE UP (highfirst == TRUE) */
    /* OBDD, ZBDD: ONLY OBSOLETE NODES WITH (var == v) EXIST IN THE SYSTEM (NO GC WAS CALLED FOR GO-BACK) */
    /* TZBDD: OBSOLETE NODES WITH ANY var EXIST IN THE SYSTEM (NO GC WAS CALLED FOR GO-BACK) ?? */

    /* IF GO-BACK WAS MOVING VARIABLE DOWN (highfirst == FALSE) */
    /* OBDD, ZBDD: OBSOLETE NODES WITH (vmax <= var < v) EXIST IN THE SYSTEM (NO GC WAS CALLED FOR GO-BACK) */
    /* TZBDD: OBSOLETE NODES WITH ANY var EXIST IN THE SYSTEM (NO GC WAS CALLED FOR GO-BACK) ?? */

    /* this garbage collection is required here to remove all obsolete nodes */
    /* next GC call may use parameter target and may not remove old obsolete nodes! */
    if (goback) {

      if ((biddyManagerType == BIDDYTYPEOBDDC) || (biddyManagerType == BIDDYTYPEOBDD)) {
        /* Biddy_Managed_GC(MNG,0,0,FALSE,TRUE); */ /* try this if there are strange problems */
        if (highfirst) {
          Biddy_Managed_GC(MNG,k,v,FALSE,TRUE);
        } else {
          Biddy_Managed_GC(MNG,v,vmax,FALSE,TRUE);
        }
      }
      else if ((biddyManagerType == BIDDYTYPEZBDDC) || (biddyManagerType == BIDDYTYPEZBDD)) {
        /* Biddy_Managed_GC(MNG,0,0,FALSE,TRUE); */ /* try this if there are strange problems */
        if (highfirst) {
          Biddy_Managed_GC(MNG,k,v,FALSE,TRUE);
        } else {
          Biddy_Managed_GC(MNG,v,vmax,FALSE,TRUE);
        }
      }
      else if ((biddyManagerType == BIDDYTYPETZBDDC) || (biddyManagerType == BIDDYTYPETZBDD)) {
        Biddy_Managed_GC(MNG,0,0,FALSE,TRUE);
      }
      else {
        Biddy_Managed_GC(MNG,0,0,FALSE,TRUE);
      }

    }

    /* PROFILING */
    /*
    {
    Biddy_Variable varOrder;
    varOrder = getGlobalOrdering(MNG,v);
    printf("SIFTING AFTER FIRST DIRECTION: variable: \"%s\" (v=%u), currentOrder=%u, min=%u, numNodes=%u\n",
           Biddy_Managed_GetVariableName(MNG,v),v,varOrder,min,Biddy_Managed_NodeTableNum(MNG));
    }
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
    min2 = biddyNodeTable.num; /* recalculate node number after go-back */

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

      if (highfirst) {
        if (k == minvar) {
          minvar = v; /* update minvar */
          stop = TRUE;
        }
        if (v == maxvar) {
          maxvar = k;  /* update maxvar */
        }
      } else {
        if (v == minvar) {
          minvar = k;  /* update minvar */
        }
        if (k == maxvar) {
          maxvar = v; /* update maxvar */
          stop = TRUE;
        }
      }

      /* NUMBER OF NODES CAN CHANGE ONLY IF v AND k HAVE BEEN ACTIVELY SWAPPED */
      /* FOR OBDDs AND ZBDDs, ONLY NODES WITH (v == smaller(k,v)) CAN BE OBSOLETE */
      /* FOR TZBDs, ALL NODES CAN BE OBSOLETE ?? */
      if (active) {

        /* DETERMINE THE NUMBER OF USEFUL NODES */
        if ((biddyManagerType == BIDDYTYPEOBDDC) || (biddyManagerType == BIDDYTYPEOBDD)) {
          /* Biddy_Managed_GC(MNG,0,0,FALSE,TRUE); */ /* try this if there are strange problems */
          Biddy_Managed_GC(MNG,
                           BiddyIsSmaller(k,v)?v:k,
                           BiddyIsSmaller(k,v)?k:v,
                           FALSE,TRUE);
          num = biddyNodeTable.num;
        }
        else if ((biddyManagerType == BIDDYTYPEZBDDC) || (biddyManagerType == BIDDYTYPEZBDD)) {
          /* Biddy_Managed_GC(MNG,0,0,FALSE,TRUE); */ /* try this if there are strange problems */
          Biddy_Managed_GC(MNG,
                           BiddyIsSmaller(k,v)?v:k,
                           BiddyIsSmaller(k,v)?k:v,
                           FALSE,TRUE);
          num = biddyNodeTable.num;
        }
        else if ((biddyManagerType == BIDDYTYPETZBDDC) || (biddyManagerType == BIDDYTYPETZBDD)) {
          Biddy_Managed_GC(MNG,0,0,FALSE,TRUE);
          num = biddyNodeTable.num;
        }
        else {
          Biddy_Managed_GC(MNG,0,0,FALSE,TRUE);
          num = biddyNodeTable.num;
        }

        /* DEBUGGING */
        /*
        printf("SIFTING: second direction, step=%u, min2=%u, num=%u\n",n,min2,num);
        */

        if (num <= min2) { /* second direction should use less or equal */
          min2 = num;
          best2 = n;
        }

        if (num > (treshold * min2)) {
          stop = TRUE;
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

    goback = ((!best2 && (n+best1 > 0)) || (best2 && (n != best2)));
    if (goback) vmax = biddyVariableTable.table[v].next; /* save the initial position of v */
    if (!best2) {
      while (n) {
        if (highfirst) {
          k = swapWithHigher(MNG,v,&active);
          if (v == minvar) {
            minvar = k;
          }
          if (k == maxvar) {
            maxvar = v;
          }
        } else {
          k = swapWithLower(MNG,v,&active);
          if (v == maxvar) {
            maxvar = k;
          }
          if (k == minvar) {
            minvar = v;
          }
        }
        n--;
      }
      while (n!=best1) {
        if (highfirst) {
          k = swapWithHigher(MNG,v,&active);
          if (v == minvar) {
            minvar = k;
          }
          if (k == maxvar) {
            maxvar = v;
          }
        } else {
          k = swapWithLower(MNG,v,&active);
          if (v == maxvar) {
            maxvar = k;
          }
          if (k == minvar) {
            minvar = v;
          }
        }
        n++;
      }
    } else {
      while (n!=best2) {
        if (highfirst) {
          k = swapWithHigher(MNG,v,&active);
          if (v == minvar) {
            minvar = k;
          }
          if (k == maxvar) {
            maxvar = v;
          }
        } else {
          k = swapWithLower(MNG,v,&active);
          if (v == maxvar) {
            maxvar = k;
          }
          if (k == minvar) {
            minvar = v;
          }
        }
        n--;
      }
    }

    /* IF SECOND GO-BACK WAS MOVING VARIABLE UP (highfirst == FALSE) */
    /* OBDD, ZBDD: ONLY OBSOLETE NODES WITH (var == v) EXIST IN THE SYSTEM (NO GC WAS CALLED FOR GO-BACK) */
    /* TZBDD: OBSOLETE NODES WITH var >= v EXIST IN THE SYSTEM (NO GC WAS CALLED FOR GO-BACK) */

    /* IF SECOND GO-BACK WAS MOVING VARIABLE DOWN (highfirst == TRUE) */
    /* OBDD, ZBDD: OBSOLETE NODES WITH (vmax <= var < v) EXIST IN THE SYSTEM (NO GC WAS CALLED FOR GO-BACK) */
    /* TZBDD: OBSOLETE NODES WITH ANY var EXIST IN THE SYSTEM (NO GC WAS CALLED FOR GO-BACK) ?? */

    /* this garbage collection is required here to remove all obsolete nodes */
    /* next GC call may use parameter target and may not remove old obsolete nodes! */
    if (goback) {

      if ((biddyManagerType == BIDDYTYPEOBDDC) || (biddyManagerType == BIDDYTYPEOBDD)) {
        /* Biddy_Managed_GC(MNG,0,0,FALSE,TRUE); */ /* try this if there are strange problems */
        if (highfirst) {
          Biddy_Managed_GC(MNG,v,vmax,FALSE,TRUE);
        } else {
          Biddy_Managed_GC(MNG,k,v,FALSE,TRUE);
        }
      }
      else if ((biddyManagerType == BIDDYTYPEZBDDC) || (biddyManagerType == BIDDYTYPEZBDD)) {
        /* Biddy_Managed_GC(MNG,0,0,FALSE,TRUE); */ /* try this if there are strange problems */
        if (highfirst) {
          Biddy_Managed_GC(MNG,v,vmax,FALSE,TRUE);
        } else {
          Biddy_Managed_GC(MNG,k,v,FALSE,TRUE);
        }
      }
      else if ((biddyManagerType == BIDDYTYPETZBDDC) || (biddyManagerType == BIDDYTYPETZBDD)) {
        Biddy_Managed_GC(MNG,0,0,FALSE,TRUE);
      }
      else {
        Biddy_Managed_GC(MNG,0,0,FALSE,TRUE);
      }

    }

    /* CHOOSE NEXT VARIABLE OR FINISH */

    if ((biddyManagerType == BIDDYTYPEOBDDC) || (biddyManagerType == BIDDYTYPEOBDD)) {
      assert( min == biddyNodeTable.num );
      num = min;
    }
    else if ((biddyManagerType == BIDDYTYPEZBDDC) || (biddyManagerType == BIDDYTYPEZBDD)) {
      assert( min == biddyNodeTable.num );
      num = min;
    }
    else if ((biddyManagerType == BIDDYTYPETZBDDC) || (biddyManagerType == BIDDYTYPETZBDD)) {
      assert( min == biddyNodeTable.num );
      num = min;
    }
    else {
      num = Biddy_Managed_NodeTableNum(MNG);
    }

    /* PROFILING */
    /*
    {
    Biddy_Variable varOrder;
    varOrder = getGlobalOrdering(MNG,v);
    printf("SIFTING LOOP FINISH WITH variable \"%s\" (v=%u): finalOrder=%u, best1=%u, best2=%u, min=%u, num=%u, numNodes=%u\n",
           Biddy_Managed_GetVariableName(MNG,v),v,varOrder,best1,best2,min,num,Biddy_Managed_NodeTableNum(MNG));
    }
    */

    /* DEBUGGING */
    /*
    for (k=0; k<Biddy_Managed_VariableTableNum(MNG); k++) {
      printf("[\"%s\"@%u]",biddyVariableTable.table[k].name,biddyVariableTable.table[k].num);
    }
    printf("\n");
    */

    finish = TRUE;
    /* choose a variable that has not been sifted, yet, and has the max number of nodes */
    vmax = 0;
    vnum = 0;
    v = 1;
    while (v < biddyVariableTable.num) {
      if ((varTable[v] == 1) && ((vv=biddyVariableTable.table[v].num) > vnum)) {
        vmax = v;
        vnum = vv;
        finish = FALSE;
      }
      v++;
    }
    v = vmax;

    /* FOR NON-CONVERGING SIFTING, EVERY NEXT VARIABLE HAS A LITTLE BIT GREATER TRESHOLD */
    if (!converge) {
      if (treshold < 1.25 * biddyNodeTable.siftingtreshold) {
        treshold = treshold + 1 / ((float)3.14 * biddyVariableTable.num);
      }
    }

    /* PROFILING */
    /*
    printf("TRESHOLD = %.3f\n",treshold);
    */

  } /* while (!finish) */

  /* THIS CODE DISABLES FURTHER CALLS IN THE CASE OF NOT-CONVERGING SIFTING */
  /* IN THE CASE OF CONVERGING SIFTING, TRESHOLD IS ADAPTED IN EVERY STEP */
  if (!converge) {
    totalbest = min;
  } else {
    treshold = treshold * biddyNodeTable.convergesiftingtreshold;
  }

  } /* while (min < totalbest) */

  /* END OF SIFTING ALGORITHM */

  /* DEBUGGING */
  /*
  printf("SIFTING FINISH: numNodes=%u, systemAge=%u\n",Biddy_Managed_NodeTableNum(MNG),biddySystemAge);
  BiddySystemReport(MNG);
  */

  return TRUE; /* sifting has been performed */
}

/***************************************************************************//*!
\brief Function BiddySiftingOnFunction reorders variables to minimize node
       number for the given function using Rudell's sifting algorithm.

### Description
    Implemented by creating tmp manager and sifting on isolated function.
    Variables are reordered globally. All obsolete nodes will be removed.
### Side effects
### More info
*******************************************************************************/

Biddy_Boolean
BiddySiftingOnFunction(Biddy_Manager MNG, Biddy_Edge f, Biddy_Boolean converge)
{
  Biddy_Manager MNG2;
  Biddy_Edge fnew;
  Biddy_Variable v;
  Biddy_Boolean status;

  /* PROFILING */
  /*
  clock_t elapsedtime;
  BiddyOrderingTable initialOrdering;
  memcpy(initialOrdering,biddyOrderingTable,sizeof(BiddyOrderingTable));
  elapsedtime = clock();
  */

  /* DEBUGGING */
  /*
  printf("SIFTING ON FUNCTION: Initial number of nodes for f = %u\n",Biddy_Managed_CountNodes(MNG,f));
  printf("INITIAL ORDERING:\n");
  writeORDER(MNG);
  */

  Biddy_InitMNG(&MNG2,Biddy_Managed_GetManagerType(MNG));

  /* FOR MNG2, DISABLE ALL CACHE TABLES - COPY AND SIFTING IS FASTER WITHOUT THEM */
  /* TO DO: user cache tables are not considered, yet */
  biddyOPCache2.disabled = TRUE;
  biddyEACache2.disabled = TRUE;
  biddyRCCache2.disabled = TRUE;
  biddyReplaceCache2.disabled = TRUE;

  fnew = Biddy_Managed_Copy(MNG,MNG2,f);
  Biddy_Managed_AddPersistentFormula(MNG2,(Biddy_String) "_BIDDY_SIFTING_ON_FUNCTION",fnew); /* sifting uses purge */

  /* PROFILING */
  /*
  printf("COPY TIME = %.2f\n",(clock()-elapsedtime)/(1.0*CLOCKS_PER_SEC));
  */

  /* DEBUGGING */
  /*
  printf("SIFTING ON FUNCTION: Initial number of nodes for fnew = %u\n",Biddy_Managed_CountNodes(MNG2,fnew));
  printf("INITIAL ORDERING FOR fnew:\n");
  writeORDER(MNG);
  */

  status = BiddyGlobalSifting(MNG2,converge);

  /* PROFILING */
  /*
  printf("GLOBAL SIFTING TIME = %.2f\n",(clock()-elapsedtime)/(1.0*CLOCKS_PER_SEC));
  */

  /* DEBUGGING */
  /*
  printf("SIFTING ON FUNCTION: Resulting number of nodes for fnew = %u\n",Biddy_Managed_CountNodes(MNG2,fnew));
  printf("RESULTING ORDERING FOR fnew:\n");
  writeORDER(MNG2);
  */

  /* ADAPT FINAL ORDERING TO THE INITIAL MANAGER */
  for (v = 1; v < biddyVariableTable.num; v++) {
    Biddy_Variable *data = (Biddy_Variable *) malloc(sizeof(Biddy_Variable));
    if ((biddyManagerType == BIDDYTYPEOBDDC) || (biddyManagerType == BIDDYTYPEOBDD))
    {
      *data = getGlobalOrdering(MNG,v);
    }
    else if ((biddyManagerType == BIDDYTYPEZBDDC) || (biddyManagerType == BIDDYTYPEZBDD) ||
              (biddyManagerType == BIDDYTYPETZBDDC) || (biddyManagerType == BIDDYTYPETZBDD))
    {
      /* for ZBDD and TZBDD variables are added in the reverse ordering */
      *data = biddyVariableTable.num - getGlobalOrdering(MNG,v);
    }
    else {
      exit(1);
    }
    Biddy_Managed_SetVariableData(MNG,v,(void *) data);
  }
  BiddySetOrderingByData(MNG,biddyOrderingTable2);
  Biddy_Managed_ClearVariablesData(MNG);

  /* PROFILING */
  /*
  printf("ADAPTING TIME = %.2f\n",(clock()-elapsedtime)/(1.0*CLOCKS_PER_SEC));
  */

  /* DEBUGGING */
  /*
  printf("SIFTING ON FUNCTION: Resulting number of nodes for f = %u\n",Biddy_Managed_CountNodes(MNG,f));
  printf("RESULTING ORDERING:\n");
  writeORDER(MNG);
  */

  Biddy_ExitMNG(&MNG2);

  /* PROFILING */
  /*
  BiddySetOrdering(MNG,initialOrdering);
  elapsedtime = clock();
  BiddySiftingOnFunctionDirect(MNG,f,converge);
  printf("LOCAL SIFTING TIME = %.2f\n",(clock()-elapsedtime)/(1.0*CLOCKS_PER_SEC));
  printf("SIFTING ON FUNCTION: Resulting number of nodes for f = %u\n",Biddy_Managed_CountNodes(MNG,f));
  */

  return status;
}

/***************************************************************************//*!
\brief Function BiddySiftingOnFunctionDirect reorders variables to minimize
       node number for the given function using Rudell's sifting algorithm.

### Description
    Variables are reordered globally. Purge is used and thus all obsolete
    formulae and nodes will be removed.
### Side effects
    Implemented for OBDD and OBDDC, only.
### More info
*******************************************************************************/

Biddy_Boolean
BiddySiftingOnFunctionDirect(Biddy_Manager MNG, Biddy_Edge f,
                             Biddy_Boolean converge)
{
  unsigned int num,min,min2,vnum,vv,totalbest;
  unsigned int best1,best2;
  Biddy_Variable v,vmax,k,n,minvar,maxvar;
  Biddy_Boolean highfirst,stop,finish,active;
  unsigned int varTable[BIDDYVARMAX];
  float treshold;

  Biddy_Variable minvarOrder,maxvarOrder;
  BiddyOrderingTable *fOrdering;

  assert( (f != NULL) && (BiddyIsOK(f) == TRUE) );

  if (f && (((BiddyNode *) BiddyP(f))->expiry == biddySystemAge)) {
    fprintf(stdout,"ERROR (BiddySiftingOnFunction): Sifting on fresh function is wrong!\n");
    return FALSE;
  }

  /* garbage collection is not needed here */
  /* however, using GC before sifting on function seems to be a good idea */
  Biddy_Managed_GC(MNG,0,0,FALSE,TRUE);

  /* determine node number and variable ordering for the specific function */
  /* fOrdering is the same ordering as biddyOrderingTable, but it */
  /* also contains information about which variables are neighbours in f */
  /* function nodeNumberOrdering also counts the number of each variable in f */
  if (!(fOrdering = (BiddyOrderingTable *) malloc(sizeof(BiddyOrderingTable)))) {
    fprintf(stderr,"ERROR (BiddySiftingOnFunction): Out of memoy!\n");
    exit(1);
  }
  for (k=1; k<Biddy_Managed_VariableTableNum(MNG); k++) {
    biddyVariableTable.table[k].numone = 0;
  }
  nullOrdering(*fOrdering);
  num = 1;
  nodeNumberOrdering(MNG,f,&num,*fOrdering);
  Biddy_Managed_DeselectAll(MNG);

  /* determine minvar (lowest == topmost) and maxvar (highest == bottommost) for function f */
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

  /* determine minvar's and maxvar's global ordering */
  /* in this global ordering, highest (topmost) variable has ordering 1 */
  /* and smallest (bottommost) variable '1' has ordering numUsedVariables */
  minvarOrder = maxvarOrder = Biddy_Managed_VariableTableNum(MNG);
  for (k=0; k<Biddy_Managed_VariableTableNum(MNG); k++) {
    if (GET_ORDER(biddyOrderingTable,minvar,k)) minvarOrder--;
    if (GET_ORDER(biddyOrderingTable,maxvar,k)) maxvarOrder--;
  }

  /* DEBUGGING */
  /*
  printf("========================================\n");
  printf("minvar: %s (id=%u, order=%u), maxvar: %s (id=%u, order=%u)\n",
          Biddy_Managed_GetVariableName(MNG,minvar),minvar,minvarOrder,
          Biddy_Managed_GetVariableName(MNG,maxvar),maxvar,maxvarOrder);
  printf("========================================\n");
  */

  /* SIFTING ALGORITHM */
  /* EXAMINE EVERY VARIABLE ONCE! */

  /* WE ARE STARTING WITH A CURRENT NUMBER OF NODES */
  if (converge) {
    treshold = biddyNodeTable.convergesiftingtreshold;
  } else {
    treshold = biddyNodeTable.siftingtreshold;
  }
  min = num;

  totalbest = min+1;
  while (min < totalbest) {
  totalbest = min;

  /* INIT varTable */
  /* varTable is used in the following way: */
  /* varTable[i] == 0 iff variable is not in f */
  /* varTable[i] == 1 iff variable is in f and has not been sifted, yet */
  /* varTable[i] == 2 iff variable is in f and has already been sifted */

  varTable[0] = 2; /* variable '1' is not swapped */
  for (k=1; k<Biddy_Managed_VariableTableNum(MNG); k++) {
    if (biddyVariableTable.table[k].numone == 0) {
      varTable[k] = 0;
    } else {
      varTable[k] = 1;
    }
  }

  /* FOR ZBDD WITHOUT COMPLEMENTED EDGES */
  /* do not swap variables which are not dependent in any function */
  /*
  if (biddyManagerType == BIDDYTYPEZBDD) {
    for (k=1; k<biddyVariableTable.num; k++) {
      BiddyNode *sup;
      Biddy_Boolean end,redundant;
      end = FALSE;
      redundant = TRUE;
      sup = biddyVariableTable.table[k].firstNode;
      do {
        if (sup->f != sup->t) {
          redundant = FALSE;
          end = TRUE;
        }
        end = end || (sup == biddyVariableTable.table[k].lastNode);
        if (!end) sup = (BiddyNode *) sup->list;
      } while (!end);
      if (redundant) varTable[k] == 2;
    }
  }
  */

  /* CHOOSE THE VARIABLE THAT WILL BE SIFTED FIRST */
  /* 1. it has not been sifted, yet */
  /* 2. it has max number of nodes */

  finish = (varTable[minvar] == 2); /* maybe, we are sifting constant variable '1' */
  v = 0;
  if (!finish) {
    /* VARIANT 1 */
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
    /* VARIANT 2 */
    /* simply choose the smallest (topmost) variable */
    /*
    v = minvar;
    */
  }

  while (!finish) {
    varTable[v] = 2;

    /* DETERMINE FIRST DIRECTION */
    if (v == minvar) { /* TOPMOST VARIABLE */
      stop = TRUE; /* skip first direction */
      highfirst = FALSE; /* swapWithHigher will be used in second direction */
    } else if (v == maxvar) { /* BOTTOMMOST VARIABLE */
      stop = TRUE; /* skip first direction */
      highfirst = TRUE; /* swapWithLower will be used in second direction */
    } else {
      stop = FALSE;

      /* VARIANT A: start with shorter direction, thus calculate v's global ordering */
      /**/
      n = getGlobalOrdering(MNG,v);
      if (n > (minvarOrder+maxvarOrder)/2) highfirst = TRUE; else highfirst = FALSE;
      /**/

      /* VARIANT B: start with highfirst = TRUE */
      /*
      highfirst = TRUE;
      */
    }

    /* PROFILING */
    /*
    {
    Biddy_Variable varOrder;
    varOrder = getGlobalOrdering(MNG,v);
    printf("SIFTING START: variable: \"%s\" (v=%u), startOrder=%u, min=%u, numNodes=%u, systemAge=%u, ",
           Biddy_Managed_GetVariableName(MNG,v),v,varOrder,min,Biddy_Managed_NodeTableNum(MNG),biddySystemAge);
    if (highfirst) printf("HIGHFIRST\n"); else printf("LOWFIRST\n");
    }
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

      if (active) {
        /* IF v AND k HAVE BEEN SWAPPED */

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

      } else {
        /* IF v AND k HAVE NOT BEEN SWAPPED */
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

      /* NUMBER OF NODES CAN CHANGE ONLY IF v AND k HAVE BEEN ACTIVELY SWAPPED */
      if (active) {

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

        /* DEBUGGING */
        /*
        printf("SIFTING: first direction, step=%u, min=%u, num=%u\n",n,min,num);
        */

        if (num < min) { /* first direction should use strict less */
          min = num;
          best1 = n;
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

      }
    } /* while (!stop ...) */

    /* PROFILING */
    /*
    {
    Biddy_Variable varOrder;
    varOrder = getGlobalOrdering(MNG,v);
    printf("SIFTING BEFORE GO-BACK: variable: \"%s\" (v=%u), currentOrder=%u, min=%u, numNodes=%u\n",
           Biddy_Managed_GetVariableName(MNG,v),v,varOrder,min,Biddy_Managed_NodeTableNum(MNG));
    }
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
          Biddy_Managed_GC(MNG,0,0,FALSE,TRUE);
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
          Biddy_Managed_GC(MNG,0,0,FALSE,TRUE);
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

    /* garbage collection is not needed here */
    /* however, it seems to be a good idea */
    Biddy_Managed_GC(MNG,0,0,FALSE,TRUE);

    /* update variable ordering for the specific function */
    /* to update which variables are neighbours */
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

    /* PROFILING */
    /*
    {
    Biddy_Variable varOrder;
    varOrder = getGlobalOrdering(MNG,v);
    printf("SIFTING AFTER FIRST DIRECTION: variable: \"%s\" (v=%u), currentOrder=%u, min=%u, numNodes=%u\n",
           Biddy_Managed_GetVariableName(MNG,v),v,varOrder,min,Biddy_Managed_NodeTableNum(MNG));
    }
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
    min2 = Biddy_Managed_NodeTableNum(MNG); /* recalculate node number after go-back */

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

      if (active) {
        /* IF v AND k HAVE BEEN SWAPPED */

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

      } else {
        /* IF v AND k HAVE NOT BEEN SWAPPED */
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

      /* NUMBER OF NODES CAN CHANGE ONLY IF v AND k HAVE BEEN ACTIVELY SWAPPED */
      if (active) {

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

        /* DEBUGGING */
        /*
        printf("SIFTING: second direction, step=%u, min2=%u, num=%u\n",n,min2,num);
        */

        if (num <= min2) { /* second direction should use less or equal */
          min2 = num;
          best2 = n;
        }

        if (num > (treshold * min2)) {
          stop = TRUE;
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

    /* garbage collection is not needed here */
    /* however, it seems to be a good idea */
    Biddy_Managed_GC(MNG,0,0,FALSE,TRUE);

    /* CHOOSE NEXT VARIABLE OR FINISH */
    /* VARIABLES NOT IN f WILL BE SKIPPED */
    /* ou have to update variable ordering for the function to update which variables are neighbours */

    for (vmax=1; vmax<Biddy_Managed_VariableTableNum(MNG); vmax++) {
      biddyVariableTable.table[vmax].numone = 0;
    }
    nullOrdering(*fOrdering);
    num = 1;
    nodeNumberOrdering(MNG,f,&num,*fOrdering);
    Biddy_Managed_DeselectAll(MNG);

    /* PROFILING */
    /*
    {
    Biddy_Variable varOrder;
    varOrder = getGlobalOrdering(MNG,v);
    printf("SIFTING FINAL: variable: \"%s\" (v=%u), finalOrder=%u, best1=%u, best2=%u, min=%u, num=%u, numNodes=%u\n",
           Biddy_Managed_GetVariableName(MNG,v),v,varOrder,best1,best2,min,num,Biddy_Managed_NodeTableNum(MNG));
    }
    */

    /* DEBUGGING */
    /*
    for (k=0; k<Biddy_Managed_VariableTableNum(MNG); k++) {
      printf("[\"%s\"@%u]",biddyVariableTable.table[k].name,biddyVariableTable.table[k].num);
    }
    printf("\n");
    */

    finish = TRUE;
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

    /* FOR NON-CONVERGING SIFTING, EVERY NEXT VARIABLE HAS A LITTLE BIT GREATER TRESHOLD */
    if (!converge) {
      if (treshold < 1.25 * biddyNodeTable.siftingtreshold) {
        treshold = treshold + 1 / ((float)3.14 * Biddy_Managed_VariableTableNum(MNG));
      }
    }

    /* PROFILING */
    /*
    printf("TRESHOLD = %.3f\n",treshold);
    */

  } /* while (!finish) */

  /* THIS CODE DISABLES FURTHER CALLS IN THE CASE OF NOT-CONVERGING SIFTING */
  /* IN THE CASE OF CONVERGING SIFTING, TRESHOLD IS ADAPTED IN EVERY STEP */
  if (!converge) {
    totalbest = min;
  } else {
    treshold = treshold * biddyNodeTable.convergesiftingtreshold;
  }

  } /* while (min < totalbest) */

  /* END OF SIFTING ALGORITHM */

  free(fOrdering);

  return TRUE; /* sifting has been performed */
}

/*******************************************************************************
\brief Initialization of Steinhaus–Johnson–Trotter algorithm.

### Description
    Create data needed for Steinhaus–Johnson–Trotter algorithm.
### Side effects
### More info
*******************************************************************************/

void
BiddySjtInit(Biddy_Manager MNG)
{
  unsigned int i;
  Biddy_Variable v;

  struct userdata {
    unsigned int value;
    Biddy_Boolean direction;
  } *ud;

  /* OBSOLETE NODES ARE REMOVED TO MAKE SWAPPING FASTER */
  Biddy_Managed_AutoGC(MNG);

  v = Biddy_Managed_GetLowestVariable(MNG);
  i = 0;
  do {
    ud = (struct userdata *) malloc(sizeof(struct userdata));
    ud->value = i;
    ud->direction = FALSE; /* FALSE = left = toTop = toLowest, TRUE = right = toBottom = toHighest */
    Biddy_Managed_SetVariableData(MNG,v,(void *) ud);
    v = biddyVariableTable.table[v].next;
    i++;
  } while (v != 0);
}

/*******************************************************************************
\brief Remove data needed for Steinhaus–Johnson–Trotter algorithm.

### Description
### Side effects
### More info
*******************************************************************************/

void
BiddySjtExit(Biddy_Manager MNG)
{
  Biddy_Managed_ClearVariablesData(MNG);
}

/*******************************************************************************
\brief One step of Steinhaus–Johnson–Trotter algorithm.

### Description
    Returns TRUE if a step was made, otherwise it returns FALSE.
### Side effects
### More info
*******************************************************************************/

Biddy_Boolean
BiddySjtStep(Biddy_Manager MNG)
{
  Biddy_Variable v,mm;
  unsigned int mmvalue;
  Biddy_Boolean STOP;

  struct userdata {
    unsigned int value;
    Biddy_Boolean direction;
  } *ud1,*ud2;

  /* find the largest mobile element */
  mm = 0;
  mmvalue = 0;
  STOP = TRUE;
  v = 0;
  do {
    v = biddyVariableTable.table[v].prev;
    ud1 = (struct userdata *) Biddy_Managed_GetVariableData(MNG,v);
    if (ud1->direction && !(Biddy_Managed_IsHighest(MNG,v)))
    {
      ud2 = (struct userdata *) Biddy_Managed_GetVariableData(MNG,biddyVariableTable.table[v].next);
      if (ud1->value > ud2->value)
      {
        STOP = FALSE; /* there exists a mobile element */
        if (ud1->value > mmvalue) {
          mm = v;
          mmvalue = ud1->value;
        }
      }
    }
    if (!(ud1->direction) && !(Biddy_Managed_IsLowest(MNG,v)))
    {
      ud2 = (struct userdata *) Biddy_Managed_GetVariableData(MNG,biddyVariableTable.table[v].prev);
      if (ud1->value > ud2->value)
      {
        STOP = FALSE; /* there exists a mobile element */
        if (ud1->value > mmvalue) {
          mm = v;
          mmvalue = ud1->value;
        }
      }
    }
  } while (!(Biddy_Managed_IsLowest(MNG,v)));

  if (!(STOP)) {
    /* swap mm and the adjacent element it is looking at */
    ud1 = (struct userdata *) Biddy_Managed_GetVariableData(MNG,mm);
    if (ud1->direction) {
      Biddy_Managed_SwapWithHigher(MNG,mm);
      /* swapWithHigher(MNG,mm,&active); */ /* not sure about this */
    } else {
      Biddy_Managed_SwapWithLower(MNG,mm);
      /* swapWithLower(MNG,mm,&active); */ /* not sure about this */
    }
    /* reverse the direction of all elements larger than mm */
    ud2 = (struct userdata *) Biddy_Managed_GetVariableData(MNG,mm);
    for (v = 1; v < Biddy_Managed_VariableTableNum(MNG); v++) {
      ud1 = (struct userdata *) Biddy_Managed_GetVariableData(MNG,v);
      if (ud1->value > ud2->value)
      {
        if (ud1->direction) {
          ud1->direction = FALSE;
        } else {
          ud1->direction = TRUE;
        }
      }
    }
  }

  return !STOP;
}

/*******************************************************************************
\brief Function BiddySetOrdering use variable swapping to create a given
        variable ordering.

### Description
    An optimized version of Bubble Sort is used.
### Side effects
    Variables are reordered globally. All obsolete nodes will be removed.
    For TZBDD, all unreferenced nodes (not part of registered formulae)
    will be removed. For TZBDD, this function may change top edge or even
    a top node of any function/formula - this is a problem, because functions
    referenced by local variables only may become wrong. Consequently, for
    TZBDDs, sifting is not safe to start automatically!
### More info
*******************************************************************************/

void
BiddySetOrdering(Biddy_Manager MNG, BiddyOrderingTable ordering)
{
  Biddy_Variable v;
  Biddy_Boolean swapped,OK;

  /* THIS IS FROM http://www.geeksforgeeks.org/bubble-sort/ */
  if (biddyVariableTable.num == 1) return;
  do {
    v = biddyVariableTable.table[0].prev;
    swapped = FALSE;
    while (!Biddy_Managed_IsLowest(MNG,v)) {
      if (GET_ORDER(ordering,v,biddyVariableTable.table[v].prev)) {
        /* v IS SMALLER THAN v.lower */
        swapWithLower(MNG,v,&OK);
        swapped = TRUE;
      } else {
        v = biddyVariableTable.table[v].prev;
      }
    }
  } while (swapped);
}

/*******************************************************************************
\brief Function BiddySetOrderingByData use variable swapping to create a given
        variable ordering.

### Description
    This is the same as BiddySetOrdering() except that variables' data are
    used instead of variable's indices.
    An optimized version of Bubble Sort is used.
### Side effects
    Variables are reordered globally. All obsolete nodes will be removed.
    For TZBDD, all unreferenced nodes (not part of registered formulae)
    will be removed. For TZBDD, this function may change top edge or even
    a top node of any function/formula - this is a problem, because functions
    referenced by local variables only may become wrong. Consequently, for
    TZBDDs, sifting is not safe to start automatically!
### More info
*******************************************************************************/

void
BiddySetOrderingByData(Biddy_Manager MNG, BiddyOrderingTable ordering)
{
  Biddy_Variable v;
  Biddy_Boolean swapped,OK;

  /* THIS IS FROM http://www.geeksforgeeks.org/bubble-sort/ */
  if (biddyVariableTable.num == 1) return;
  do {
    v = biddyVariableTable.table[0].prev;
    swapped = FALSE;
    while (!Biddy_Managed_IsLowest(MNG,v)) {
      if (GET_ORDER(ordering,
                    *((Biddy_Variable *)(Biddy_Managed_GetVariableData(MNG,v))),
                    *((Biddy_Variable *)(Biddy_Managed_GetVariableData(MNG,biddyVariableTable.table[v].prev)))))
      {
        /* v.data IS SMALLER THAN (v.lower).data */
        swapWithLower(MNG,v,&OK);
        swapped = TRUE;
      } else {
        v = biddyVariableTable.table[v].prev;
      }
    }
  } while (swapped);
}

/*******************************************************************************
\brief Function BiddyCopy create a copy of BDD in another manager of the same
        BDD type.

### Description
### Side effects
    Expiry value of the original nodes is not copied.
    It is assumed that the target manager has the same or larger domain and
    the same ordering for all variables from the source manager.
### More info
*******************************************************************************/

static Biddy_Edge BiddyCopyR(Biddy_Manager MNG1, Biddy_Manager MNG2, Biddy_Edge f);

Biddy_Edge
BiddyCopy(Biddy_Manager MNG1, Biddy_Manager MNG2, Biddy_Edge f)
{
  Biddy_Edge r;

  assert( MNG1 != MNG2 );
  assert( biddyManagerType1 == biddyManagerType2 );

  /* TO DO: use Special case 2 of taggedFoaNode to simplify this function! */

  BiddyCreateLocalInfo(MNG1,f);
  r = BiddyCopyR(MNG1,MNG2,f);
  BiddyDeleteLocalInfo(MNG1,f);

  return r;
}

static Biddy_Edge
BiddyCopyR(Biddy_Manager MNG1, Biddy_Manager MNG2, Biddy_Edge f)
{
  Biddy_Edge e,t,r;
  Biddy_Variable tag;
  Biddy_Boolean mark;
  Biddy_String name;
  Biddy_Variable v;

  /* TO DO: use Special case 2 of taggedFoaNode to simplify this function! */

  tag = Biddy_GetTag(f); /* tag of the given edge */
  if (tag) {
    name = Biddy_Managed_GetVariableName(MNG1,tag);
    tag = Biddy_Managed_GetVariable(MNG2,name);
  }
  mark = Biddy_GetMark(f); /* complement bit of the given edge */

  if (f == biddyNull) return biddyNull;
  if (Biddy_IsTerminal(f)) {
    if (mark) {
      r = Biddy_Complement(biddyTerminal2);
    } else {
      r = biddyTerminal2;
    }
    if (tag) Biddy_SetTag(r,tag);
  } else {
    if (Biddy_Managed_IsSelected(MNG1,f)) {
      r = BiddyGetCopy(f);
      Biddy_SetTag(r,tag);
      if (mark) Biddy_SetMark(r);
    } else {
      Biddy_Managed_SelectNode(MNG1,f);
      name = Biddy_Managed_GetTopVariableName(MNG1,f); /* top variable of the given edge */
      v = Biddy_Managed_GetVariable(MNG2,name);
      e = BiddyCopyR(MNG1,MNG2,Biddy_Managed_TransferMark(MNG1,BiddyE(f),mark,TRUE)); /* TRUE = left */
      t = BiddyCopyR(MNG1,MNG2,Biddy_Managed_TransferMark(MNG1,BiddyT(f),mark,FALSE)); /* FALSE = right */
      if ((biddyManagerType1 == BIDDYTYPETZBDDC) || (biddyManagerType1 == BIDDYTYPETZBDD)) {
        r = BiddyManagedTaggedFoaNode(MNG2,v,e,t,tag,TRUE);
      } else {
        r = BiddyManagedTaggedFoaNode(MNG2,v,e,t,v,TRUE);
      }
      BiddyProlongOne(r,biddySystemAge2); /* FoaNode returns an obsolete node! */
      BiddySetCopy(f,BiddyN(r));
    }
  }

  return r;
}

/*******************************************************************************
\brief Function BiddyCopyOBDD copies a BIDDYTYPEOBDD or BIDDYTYPEOBDDC
        into another manager.

### Description
### Side effects
    Expiry value of the original nodes is not copied.
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
  r = BiddyManagedITE(MNG2,Biddy_Managed_GetVariableEdge(MNG2,v),t,e);

  return r;
}

/*******************************************************************************
\brief Function BiddyCopyZBDD copies a BIDDYTYPEZBDD or BIDDYTYPEZBDDC
        into another manager.

### Description
### Side effects
    Expiry value of the original nodes is not copied.
    The variable ordering of created BDD is adapted to the target manager.
    It is assumed that the target manager has the same or larger domain.
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
      r = BiddyManagedITE(MNG2,Biddy_Managed_GetVariableEdge(MNG2,v),biddyZero2,r);
    }
    return r;
  }

  topname = Biddy_Managed_GetTopVariableName(MNG1,f);
  mark = Biddy_GetMark(f);
  v = Biddy_Managed_GetVariable(MNG2,topname); /* variable must exist */
  e = BiddyCopyZBDD(MNG1,MNG2,Biddy_Managed_TransferMark(MNG1,BiddyE(f),mark,TRUE)); /* TRUE = left */
  t = BiddyCopyZBDD(MNG1,MNG2,Biddy_Managed_TransferMark(MNG1,BiddyT(f),mark,FALSE)); /* FALSE = right */
  t = BiddyManagedChange(MNG2,t,v);
  r = BiddyManagedOr(MNG2,e,t); /* or/xor, both is correct, what is faster? */

  return r;
}

/*******************************************************************************
\brief Function BiddyCopyTZBDD copies a BIDDYTYPETZBDD or BIDDYTYPETZBDDC
        into another manager.

### Description
### Side effects
    Expiry value of the original nodes is not copied.
    The variable ordering of created BDD is adapted to the target manager.
    It is assumed that the target manager has the same or larger domain.
### More info
*******************************************************************************/

Biddy_Edge
BiddyCopyTZBDD(Biddy_Manager MNG1, Biddy_Manager MNG2, Biddy_Edge f)
{
  Biddy_String topname;
  Biddy_Variable top,tag;
  Biddy_Variable v;
  Biddy_Edge e,t,r;

  assert ( (biddyManagerType1 == BIDDYTYPETZBDDC) || (biddyManagerType1 == BIDDYTYPETZBDD) );

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
    r = BiddyManagedITE(MNG2,Biddy_Managed_GetVariableEdge(MNG2,v),t,e);
  }

  tag = Biddy_GetTag(f);
  while (top != tag) {
    top = biddyVariableTable1.table[top].prev;
    topname = Biddy_Managed_GetVariableName(MNG1,top);
    v = Biddy_Managed_GetVariable(MNG2,topname); /* variable must exist */
    r = BiddyManagedITE(MNG2,Biddy_Managed_GetVariableEdge(MNG2,v),biddyZero2,r);
  }

  return r;
}

/*******************************************************************************
\brief Function BiddyConvertDirect copies a graph from one manager to
       another manager which can use the same or different BDD type.

### Description
    This should be faster than a general copy functions. But, it is not
    implemented for all BDD type conversions. However, it is safe to use this
    function for any BDD type conversion because in the case the conversion
    is not directly implemented a regular copy will be called.
### Side effects
    It is assumed, that domain has already been copied.
    For conversion from OBDD/TZBDD into ZBDD this function does not create
    the topmost variables which are not present in OBDD/TZBDD.
    For conversion from ZBDD into OBDD/TZBDD this function does not create
    the topmost variables which are not present in ZBDD.
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

    if (Biddy_IsTerminal(f)) {
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
        e = BiddyManagedTaggedFoaNode(MNG2,prevvar,e,e,prevvar,TRUE);
        BiddyProlongOne(e,biddySystemAge2); /* FoaNode returns an obsolete node! */
        prevvar = biddyVariableTable2.table[prevvar].prev;
      }
    }

    t = BiddyConvertDirect(MNG1,MNG2,Biddy_Managed_TransferMark(MNG1,BiddyT(f),mark,FALSE)); /* FALSE = right */

    if ((biddyManagerType2 == BIDDYTYPEZBDDC) || (biddyManagerType2 == BIDDYTYPEZBDD)) {
      topname = Biddy_Managed_GetTopVariableName(MNG1,BiddyT(f));
      prevvar = Biddy_Managed_GetVariable(MNG2,topname);
      prevvar = biddyVariableTable2.table[prevvar].prev;
      while (prevvar != top) {
        t = BiddyManagedTaggedFoaNode(MNG2,prevvar,t,t,prevvar,TRUE);
        BiddyProlongOne(t,biddySystemAge2); /* FoaNode returns an obsolete node! */
        prevvar = biddyVariableTable2.table[prevvar].prev;
      }
    }

    r = BiddyManagedTaggedFoaNode(MNG2,v,e,t,v,TRUE);
    BiddyProlongOne(r,biddySystemAge2); /* FoaNode returns an obsolete node! */

  }

  else if ((biddyManagerType1 == BIDDYTYPEZBDDC) || (biddyManagerType1 == BIDDYTYPEZBDD)) {

    if ((biddyManagerType2 == BIDDYTYPETZBDDC) || (biddyManagerType2 == BIDDYTYPETZBDD)) {
      /* TO DO: DIRECT CONVERSION ZBDD -> TZBDD IS NOT IMPLEMENTED, YET */
      Biddy_Manager MNGTMP;
      Biddy_Edge tmp;
      Biddy_InitMNG(&MNGTMP,BIDDYTYPEOBDDC);
      Biddy_Managed_Copy(MNG1,MNGTMP,biddyZero1); /* copy domain from MNG1 to MNGTMP */
      tmp = BiddyConvertDirect(MNG1,MNGTMP,f);
      r = BiddyConvertDirect(MNGTMP,MNG2,tmp);
      Biddy_ExitMNG(&MNGTMP);
      return r;
    }

    if (Biddy_IsTerminal(f)) {
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
        BiddyProlongOne(e,biddySystemAge2); /* FoaNode returns an obsolete node! */
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
        BiddyProlongOne(t,biddySystemAge2); /* FoaNode returns an obsolete node! */
        prevvar = biddyVariableTable2.table[prevvar].prev;
      }
    }

    r = BiddyManagedTaggedFoaNode(MNG2,v,e,t,v,TRUE);
    BiddyProlongOne(r,biddySystemAge2); /* FoaNode returns an obsolete node! */

  }

  else if ((biddyManagerType1 == BIDDYTYPETZBDDC) || (biddyManagerType1 == BIDDYTYPETZBDD)) {

    if ((biddyManagerType2 == BIDDYTYPEZBDDC) || (biddyManagerType2 == BIDDYTYPEZBDD)) {
      /* TO DO: DIRECT CONVERSION TZBDD -> ZBDD IS NOT IMPLEMENTED, YET */
      Biddy_Manager MNGTMP;
      Biddy_Edge tmp;
      Biddy_InitMNG(&MNGTMP,BIDDYTYPEOBDDC);
      Biddy_Managed_Copy(MNG1,MNGTMP,biddyZero1); /* copy domain from MNG1 to MNGTMP */
      tmp = BiddyConvertDirect(MNG1,MNGTMP,f);
      r = BiddyConvertDirect(MNGTMP,MNG2,tmp);
      Biddy_ExitMNG(&MNGTMP);
      return r;
    }

    tag = Biddy_GetTag(f); /* tag of the given edge */
    mark = Biddy_GetMark(f); /* complement bit of the given edge */
    top = Biddy_GetTopVariable(f); /* top variable of the given edge */

    if (Biddy_IsTerminal(f)) {
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
      BiddyProlongOne(r,biddySystemAge2); /* FoaNode returns an obsolete node! */
    }

    while (tag != top) {
      top = biddyVariableTable1.table[top].prev;
      topname = Biddy_Managed_GetVariableName(MNG1,top);
      v = Biddy_Managed_GetVariable(MNG2,topname);
      r = BiddyManagedTaggedFoaNode(MNG2,v,r,biddyZero2,v,TRUE);
      BiddyProlongOne(r,biddySystemAge2); /* FoaNode returns an obsolete node! */
    }

  }

  /* DEBUGGING */
  /*
  printf("Result: f = %s (%p)\n",Biddy_Managed_GetTopVariableName(MNG2,r),r);
  */

  return r;
}

/*******************************************************************************
\brief Function BiddyConstructBDD constructs BDD.

### Description
### Side effects
### More info
*******************************************************************************/

Biddy_Edge
BiddyConstructBDD(Biddy_Manager MNG, int numN, BiddyNodeList *tableN)
{
  Biddy_Boolean usetags;
  int i; /* must be signed because of used for loop */
  Biddy_Edge r;

  r = biddyNull;

  usetags = FALSE;
  if ((biddyManagerType == BIDDYTYPETZBDDC) || (biddyManagerType == BIDDYTYPETZBDD))
  {
    usetags = TRUE;
  }

  /* CONSTRUCT TERMINAL NODES */
  /* for all BDD types, constant zero is always represented by a single node */
  /* in fact, for all BDD types, terminal zero is represented as a complemented terminal node */
  /* however, constant one may not be the same as terminal node */
  for (i=0; i<numN; i++) {
    if (tableN[i].type == 0) {
      tableN[i].created = TRUE;
      tableN[i].f = biddyZero;
    } else if (tableN[i].type == 1) {
      tableN[i].created = TRUE;
      tableN[i].f = biddyTerminal;
    } else {
      tableN[i].created = FALSE;
      tableN[i].f = biddyNull;
    }
  }

  /* THIS CODE IS TESTED ONLY WITH ONE LABEL PER NODELIST */
  /* TO DO: implement and use reorderVariables() for better results */
  for (i=0; i<numN; ++i) {
    /* regular label: type = 2, r = -1 */
    if (tableN[i].type == 2) {
      constructBDD(MNG,numN,tableN,tableN[i].l);
      tableN[i].f = tableN[tableN[i].l].f;
      if (usetags) {
        Biddy_SetTag(tableN[i].f,tableN[i].ltag);
      }
      r = tableN[i].f;
    }
    /* complemented label: type = 3, r = -1 */
    if (tableN[i].type == 3) {
      constructBDD(MNG,numN,tableN,tableN[i].l);
      tableN[i].f = Biddy_Complement(tableN[tableN[i].l].f);
      if (usetags) {
        Biddy_SetTag(tableN[i].f,tableN[i].ltag);
      }
      r = tableN[i].f;
    }
  }

  return r;
}

/*******************************************************************************
\brief Function BiddySystemReport is intended for debugging.

### Description
### Side effects
### More info
*******************************************************************************/

/* USE THIS TO GET MORE STATISTICS */
#define SYSTEMREPORTVERBOSE
#undef SYSTEMREPORTVERBOSE

/* USE THIS TO GET LISTING OF ALL NODES  IN THE SYSTEM */
#define SYSTEMREPORTDETAILS
#undef SYSTEMREPORTDETAILS

void
BiddySystemReport(Biddy_Manager MNG)
{
  Biddy_Variable v;
  unsigned int i;
  unsigned int count1,count2;
  unsigned int countfortified,countprolonged,countfresh,countobsolete;

#ifdef SYSTEMREPORTDETAILS
#undef SYSTEMREPORTVERBOSE
#define SYSTEMREPORTVERBOSE
#endif

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

#ifdef SYSTEMREPORTDETAILS
  printf("list of variables: ");
  v = 0;
  do {
    v = biddyVariableTable.table[v].prev;
  } while (biddyVariableTable.table[v].prev < biddyVariableTable.num);
  do {
    printf("[%s]",biddyVariableTable.table[v].name);
    v = biddyVariableTable.table[v].next;
  } while (v != 0);
  printf("\n");
#endif

  /* FORMULAE */

#ifdef SYSTEMREPORTVERBOSE
  printf("SYSTEM REPORT (FORMULAE)\n");
#endif

#ifdef SYSTEMREPORTVERBOSE
  printf("number of formulae: %d\n",biddyFormulaTable.size);
#endif

  if ((biddyManagerType == BIDDYTYPETZBDDC) || (biddyManagerType == BIDDYTYPETZBDD) ||
      (biddyManagerType == BIDDYTYPETZFDDC) || (biddyManagerType == BIDDYTYPETZFDD))
  {
    for (i = 1; i < biddyFormulaTable.size; i++) {
#ifdef SYSTEMREPORTDETAILS
      printf("CHECKING FORMULA: %s\n",biddyFormulaTable.table[i].name);
#endif
      if (!Biddy_IsNull(biddyFormulaTable.table[i].f) &&
          !Biddy_IsTerminal(biddyFormulaTable.table[i].f))
      {
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

  countfresh = 1; /* all variables, including terminal node, are fresh */
  countfortified = countprolonged = countobsolete = 0;
  for (v=1; v<biddyVariableTable.num; v++) {

    BiddyNode *sup;
    BiddyNode *sup1,*sup2;
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
        fprintf(stderr,"NODE: pointer = %p\n",(void *) sup);
        fprintf(stderr,"NODE: variable \"%s\", else=\"%s\", then=\"%s\"\n",
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
        fprintf(stderr,"NODE: pointer = %p\n",(void *) sup);
        fprintf(stderr,"NODE: variable \"%s\", else=\"%s\", then=\"%s\"\n",
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
        fprintf(stderr,"NODE: pointer = %p\n",(void *) sup);
        fprintf(stderr,"NODE: variable \"%s\", else=\"%s\", then=\"%s\"\n",
          sup?Biddy_Managed_GetTopVariableName(MNG,sup):"NULL",
          sup->f?Biddy_Managed_GetTopVariableName(MNG,sup->f):"NULL",
          sup->t?Biddy_Managed_GetTopVariableName(MNG,sup->t):"NULL");
        free((void *)1); /* BREAKPOINT */
      }
    }

    if (sup->expiry != biddySystemAge) {
      fprintf(stderr,"ERROR (BiddySystemReport): First element in the list for variable \"%s\" is not fresh (expiry = %u, biddySystemAge = %u)\n",
              biddyVariableTable.table[v].name,sup->expiry,biddySystemAge);
      fprintf(stderr,"NODE: pointer = %p\n",(void *) sup);
      fprintf(stderr,"NODE: variable \"%s\", else=\"%s\", then=\"%s\"\n",
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

      if (!sup->f || !sup->t) {
        fprintf(stderr,"ERROR (BiddySystemReport): Node with NULL successor \"%s\"\n",biddyVariableTable.table[v].name);
        debugNode(MNG,sup);
        free((void *)1); /* BREAKPOINT */
      }

      if (sup->v != v) {
        fprintf(stderr,"ERROR (BiddySystemReport): Node with bad variable in list for variable \"%s\"\n",biddyVariableTable.table[v].name);
        fprintf(stderr,"NODE: pointer = %p\n",(void *) sup);
        fprintf(stderr,"NODE: variable \"%s\", else=\"%s\", then=\"%s\"\n",
          Biddy_Managed_GetTopVariableName(MNG,sup),
          Biddy_Managed_GetTopVariableName(MNG,sup->f),
          Biddy_Managed_GetTopVariableName(MNG,sup->t));
        free((void *)1); /* BREAKPOINT */
      }

      if ((sup->f && !BiddyIsSmaller(v,BiddyV(sup->f))) || (sup->t && !BiddyIsSmaller(v,BiddyV(sup->t))))
      {
        fprintf(stderr,"ERROR (BiddySystemReport): Node with bad ordering in list for variable \"%s\"\n",biddyVariableTable.table[v].name);
        fprintf(stderr,"NODE: pointer = %p\n",(void *) sup);
        fprintf(stderr,"NODE: variable \"%s\", else=\"%s\", then=\"%s\", @%u\n",
          Biddy_Managed_GetTopVariableName(MNG,sup),
          Biddy_Managed_GetTopVariableName(MNG,sup->f),
          Biddy_Managed_GetTopVariableName(MNG,sup->t),
          sup->expiry);
        free((void *)1); /* BREAKPOINT */
      }

      if ((biddyManagerType == BIDDYTYPETZBDDC) || (biddyManagerType == BIDDYTYPETZBDD) ||
          (biddyManagerType == BIDDYTYPETZFDDC) || (biddyManagerType == BIDDYTYPETZFDD))
      {
        if ((sup->f && BiddyIsSmaller(BiddyV(sup->f),Biddy_GetTag(sup->f))) ||
            (sup->t && BiddyIsSmaller(BiddyV(sup->t),Biddy_GetTag(sup->t))))
        {
          fprintf(stderr,"ERROR (BiddySystemReport): Node \"%s\" has bad tag for its successor\n",biddyVariableTable.table[v].name);
          fprintf(stderr,"NODE: pointer = %p\n",(void *) sup);
          fprintf(stderr,"NODE: variable \"%s\", elsetag=\"%s\", else=\"%s\", thentag=\"%s\", then=\"%s\"\n",
            Biddy_Managed_GetTopVariableName(MNG,sup),
            Biddy_Managed_GetVariableName(MNG,Biddy_GetTag(sup->f)),
            Biddy_Managed_GetTopVariableName(MNG,sup->f),
            Biddy_Managed_GetVariableName(MNG,Biddy_GetTag(sup->t)),
            Biddy_Managed_GetTopVariableName(MNG,sup->t));
          free((void *)1); /* BREAKPOINT */
        }
      }

      if (sup->f && sup->t && (sup != biddyVariableTable.table[v].firstNode)) {
        hash = nodeTableHash(v,sup->f,sup->t,biddyNodeTable.size);
        sup1 = biddyNodeTable.table[hash];
        findNodeTable(MNG,v,sup->f,sup->t,&sup1);
        if (!sup1 || (v != sup1->v)) {
          fprintf(stderr,"ERROR (BiddySystemReport): Node not correctly inserted into Node table\n");
          fprintf(stderr,"NODE: pointer = %p\n",(void *) sup);
          fprintf(stderr,"NODE: variable \"%s\", else=\"%s\", then=\"%s\"\n",
            Biddy_Managed_GetTopVariableName(MNG,sup),
            Biddy_Managed_GetTopVariableName(MNG,sup->f),
            Biddy_Managed_GetTopVariableName(MNG,sup->t));
          free((void *)1); /* BREAKPOINT */
        }
        sup2 = (BiddyNode *) sup1->next;
        if (sup2) {
          findNodeTable(MNG,v,sup->f,sup->t,&sup2);
          if (sup2 && (v == sup2->v)) {
            fprintf(stderr,"ERROR (BiddySystemReport): The same node multiple times in Node table\n");
            fprintf(stderr,"NODE: pointer = %p\n",(void *) sup1);
            fprintf(stderr,"NODE: variable \"%s\", else=\"%s\" (%p), then=\"%s\" (%p)\n",
              Biddy_Managed_GetTopVariableName(MNG,sup1),
              Biddy_Managed_GetTopVariableName(MNG,sup1->f),sup1->f,
              Biddy_Managed_GetTopVariableName(MNG,sup1->t),sup1->t);
            fprintf(stderr,"NODE: pointer = %p\n",(void *) sup1);
            fprintf(stderr,"NODE: variable \"%s\", else=\"%s\" (%p), then=\"%s\" (%p)\n",
              Biddy_Managed_GetTopVariableName(MNG,sup2),
              Biddy_Managed_GetTopVariableName(MNG,sup2->f),sup2->f,
              Biddy_Managed_GetTopVariableName(MNG,sup2->t),sup2->t);
            free((void *)1); /* BREAKPOINT */
          }
        }
      }

#ifdef SYSTEMREPORTDETAILS
      printf("NODE ");
      debugNode(MNG,sup);
      printf("\n");
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

  /* FINAL REPORT FOR LISTS */
#ifdef SYSTEMREPORTVERBOSE
  printf("BiddySystemReport (lists): numFortified: %u, numProlonged: %u, numFresh: %u, numBad: %u, total: %u\n",
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
        printf("NODE %u-",i);
        debugNode(MNG,sup);
        printf("\n");
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
#ifdef SYSTEMREPORTVERBOSE
  printf("BiddySystemReport (table): numFortified: %u, numProlonged: %u, numFresh: %u, numBad: %u, total: %u\n",
         countfortified,countprolonged,countfresh,countobsolete,
         countfortified+countprolonged+countfresh+countobsolete
  );
#endif

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

/*******************************************************************************
\brief Function BiddyDebugNode.

### Description
### Side effects
### More info
*******************************************************************************/

void
BiddyDebugNode(Biddy_Manager MNG, BiddyNode *node)
{
  debugNode(MNG,node);
}

/*******************************************************************************
\brief Function BiddyDebugEdge.

### Description
### Side effects
### More info
*******************************************************************************/

void
BiddyDebugEdge(Biddy_Manager MNG, Biddy_Edge edge)
{
  debugEdge(MNG,edge);
}

/*----------------------------------------------------------------------------*/
/* Definition of static functions                                             */
/*----------------------------------------------------------------------------*/

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

  if (Biddy_IsTerminal(f1)) {
    r = Biddy_IsTerminal(f2) &&
        (Biddy_GetMark(f1) == Biddy_GetMark(f2)) &&
        !strcmp(
          Biddy_Managed_GetVariableName(MNG1,Biddy_GetTag(f1)),
          Biddy_Managed_GetVariableName(MNG2,Biddy_GetTag(f2)));
  } else {
    r = !Biddy_IsTerminal(f2) &&
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
\brief Function createLocalInfo recursively creates local info for all nodes
       except terminal node.

### Description
### Side effects
    All nodes except terminal node must be selected!
    Function createLocalInfo will deselect them.
    data.mintermCount is using GNU Multiple Precision Arithmetic Library (GMP).
    No initialization of elements is performed here.
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
    c = &c[1]; /* next field in the array */
    c = createLocalInfo(MNG,BiddyE(f),c);
    c = createLocalInfo(MNG,BiddyT(f),c);
  }
  return c;
}

/*******************************************************************************
\brief Function deleteLocalInfo recursively deletes local info for all nodes
       except terminal node.

### Description
### Side effects
    All nodes except terminal node must be selected!
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

  /* ENSURE THE CORRECT INFO ABOUT THE ORDERING OF THE NEW VARIABLE */
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
  /* TERMINAL NODE biddyNodeTable.table[0] IS USED HERE AS TMP NODE, ONLY */

  biddyVariableTable.table[v].name = strdup(x);
  biddyVariableTable.table[v].num = 0;
  biddyVariableTable.table[v].firstNode = biddyNodeTable.table[0];
  biddyVariableTable.table[v].lastNode = biddyNodeTable.table[0];
  r = BiddyManagedTaggedFoaNode(MNG,v,biddyNull,biddyNull,v,FALSE); /* FALSE is important! */
  biddyVariableTable.table[v].firstNode = BiddyN(r);
  biddyNodeTable.table[0]->list = NULL; /* REPAIR TERMINAL NODE */

  /* FOR OBDD AND OFDD, NOW YOU HAVE A VARIABLE BUT AN ELEMENT IS ALSO CREATED */
  /* FOR OBDD AND OFDD, NEW VARIABLE IS ADDED BELOW ALL THE EXISTING ONES */
  if ((biddyManagerType == BIDDYTYPEOBDDC) || (biddyManagerType == BIDDYTYPEOBDD) ||
      (biddyManagerType == BIDDYTYPEOFDDC) || (biddyManagerType == BIDDYTYPEOFDD))
  {
    biddyVariableTable.table[v].variable = r;
    el = biddyZero; /* for OBDDs and OFDDs, elements are not supported, yet */
    biddyVariableTable.table[v].element = el;
  }

  /* FOR ZBDD AND ZFDD, NOW YOU HAVE AN ELEMENT BUT A VARIABLE IS ALSO CREATED */
  /* FOR ZBDD AND ZFDD, NEW VARIABLE IS ADDED ABOVE ALL THE EXISTING ONES */
  /* THIS CALCULATION REQUIRES THAT NEW VARIABLE IS ADDED ABOVE ALL THE EXISTING ONES */
  else if ((biddyManagerType == BIDDYTYPEZBDDC) || (biddyManagerType == BIDDYTYPEZBDD) ||
            (biddyManagerType == BIDDYTYPEZFDDC) || (biddyManagerType == BIDDYTYPEZFDD))
  {
    biddyVariableTable.table[v].element = r;
    r = BiddyManagedTaggedFoaNode(MNG,v,biddyZero,biddyOne,v,TRUE);
    BiddyRefresh(r);
    biddyVariableTable.table[v].variable = r;
  }

  /* FOR TZBDD AND TZFDD, NOW YOU HAVE A VARIABLE BUT AN ELEMENT IS ALSO CREATED */
  /* FOR TZBDD AND TZFDD, NEW VARIABLE IS ADDED ABOVE ALL THE EXISTING ONES */
  /* HOWEVER, THIS CALCULATION IS GENERAL */
  else if ((biddyManagerType == BIDDYTYPETZBDDC) || (biddyManagerType == BIDDYTYPETZBDD) ||
            (biddyManagerType == BIDDYTYPETZFDDC) || (biddyManagerType == BIDDYTYPETZFDD))
  {
    biddyVariableTable.table[v].variable = r;
    el = biddyOne;
    Biddy_SetTag(el,biddyVariableTable.table[v].next);
    top = 0;
    for (k = 1; k < biddyVariableTable.num; k++) {
      top = biddyVariableTable.table[top].prev;
    }
    el = BiddyManagedTaggedFoaNode(MNG,v,biddyZero,el,top,TRUE);
    BiddyRefresh(el);
    biddyVariableTable.table[v].element = el;
  }

  return r;
}

/*******************************************************************************
\brief Function evalProbability.

### Description
### Side effects
    Function will select all nodes except terminal node.
### More info
*******************************************************************************/

static void
evalProbability(Biddy_Manager MNG, Biddy_Edge f,  double *c1,
                double *c0, Biddy_Boolean *leftmost)
{
  double r1,r0,z,zthen,zelse;
  double *q;
  Biddy_Variable v;

  assert( f != NULL );

  assert((
    (biddyManagerType == BIDDYTYPEOBDD) ||
    (biddyManagerType == BIDDYTYPEOBDDC) ||
    (biddyManagerType == BIDDYTYPEZBDD) ||
    (biddyManagerType == BIDDYTYPEZBDDC) ||
    (biddyManagerType == BIDDYTYPETZBDD)
  ));

  /* DEBUGGING */
  /*
  printf("evalProbability: f = %s\n",Biddy_Managed_GetTopVariableName(MNG,f));
  */

  /* For OBDDs, c1 is path 1 sum, c0 is path 0 sum, leftmost is not used */
  /* For ZBDDs, c1 is path 1 sum, c0 is not used, leftmost is the leftmost value */
  /* For TZBDDs, c1 is path 1 sum, c0 is not used, leftmost is not used, yet */
  if (f == biddyZero) {
    *c1 = 0.0;
    *c0 = 1.0;
    *leftmost = FALSE;
    return;
  }
  if (Biddy_IsTerminal(f)) {
    *c1 = 1.0;
    *c0 = 0.0;
    *leftmost = TRUE;
    return;
  }

  if (Biddy_Managed_IsSelected(MNG,f)) {
    /* ALREADY VISITED */
    if (Biddy_GetMark(f)) {
      if (biddyManagerType == BIDDYTYPEOBDDC) {
        *c1 = BiddyGetPath0ProbabilitySum(f); /* positive edge is stored but actual edge is returned */
        *c0 = BiddyGetPath1ProbabilitySum(f); /* positive edge is stored but actual edge is returned */
      }
      else if (biddyManagerType == BIDDYTYPEZBDDC) {
        *c1 = BiddyGetPath1ProbabilitySum(f); /* positive edge is stored */
        *leftmost = !(BiddyGetLeftmost(f)); /* actual edge is returned */
        v = Biddy_Managed_GetTopVariable(MNG,f);
        z = 1.0;
        while (v) {
          q = (double *) Biddy_Managed_GetVariableData(MNG,v);
          if (q) {
            z = z * (1 - *q);
          }
          v = biddyVariableTable.table[v].next;
        }
        if (*leftmost) {
          (*c1) += z; /* actual edge is returned */
        } else {
          (*c1) -= z; /* actual edge is returned */
        }
      }
      else if (biddyManagerType == BIDDYTYPETZBDDC) {
        /* COMPLEMENTED EDGES ARE NOT IMPLEMENTED FOR TZBDDs, YET */
      }
    } else {
      if ((biddyManagerType == BIDDYTYPEOBDDC) || (biddyManagerType == BIDDYTYPEOBDD)) {
        *c1 = BiddyGetPath1ProbabilitySum(f);
        *c0 = BiddyGetPath0ProbabilitySum(f);
      }
      else if ((biddyManagerType == BIDDYTYPEZBDDC) || (biddyManagerType == BIDDYTYPEZBDD)) {
        *c1 = BiddyGetPath1ProbabilitySum(f);
        *leftmost = BiddyGetLeftmost(f);
      }
      else if ((biddyManagerType == BIDDYTYPETZBDDC) || (biddyManagerType == BIDDYTYPETZBDD)) {
        *c1 = BiddyGetPath1ProbabilitySum(f);
      }
    }
    return;
  }

  /* NOT VISITED, YET */
  Biddy_Managed_SelectNode(MNG,f);

  /* SUM PROBABILITY FOR BOTH SUCCESSORS */
  /* because of parameter leftmost it is important to calculate BiddyT first! */
  evalProbability(MNG,BiddyT(f),c1,c0,leftmost);
  evalProbability(MNG,BiddyE(f),&r1,&r0,leftmost);

  if ((biddyManagerType == BIDDYTYPEOBDDC) || (biddyManagerType == BIDDYTYPEOBDD)) {
    q = (double *) Biddy_Managed_GetVariableData(MNG,Biddy_Managed_GetTopVariable(MNG,f));
    if (q) {
      *c1 = (*c1 * *q) + (r1 * (1.0 - *q));
      *c0 = (*c0 * *q) + (r0 * (1.0 - *q));
    } else {
      /* empty value means 100% probability, i.e. *q = 0 */
      *c1 = r1;
      *c0 = r0;
    }
  }
  else if ((biddyManagerType == BIDDYTYPEZBDDC) || (biddyManagerType == BIDDYTYPEZBDD)) {
    v = Biddy_Managed_GetTopVariable(MNG,f);
    v = biddyVariableTable.table[v].next;
    zthen = 1.0;
    while (v != Biddy_Managed_GetTopVariable(MNG,BiddyT(f))) {
      q = (double *) Biddy_Managed_GetVariableData(MNG,v);
      if (q) {
        zthen = zthen * (1 - *q);
      }
      v = biddyVariableTable.table[v].next;
    }
    v = Biddy_Managed_GetTopVariable(MNG,f);
    v = biddyVariableTable.table[v].next;
    zelse = 1.0;
    while (v != Biddy_Managed_GetTopVariable(MNG,BiddyE(f))) {
      q = (double *) Biddy_Managed_GetVariableData(MNG,v);
      if (q) {
        zelse = zelse * (1 - *q);
      }
      v = biddyVariableTable.table[v].next;
    }
    v = Biddy_Managed_GetTopVariable(MNG,f);
    q = (double *) Biddy_Managed_GetVariableData(MNG,v);
    if (q) {
      *c1 = (*c1 * zthen * *q) + (r1 * zelse * (1.0 - *q));
    } else {
      /* empty value means 100% probability, i.e. *q = 0 */
      *c1 = r1 * zelse;
    }
  }
  else if ((biddyManagerType == BIDDYTYPETZBDDC) || (biddyManagerType == BIDDYTYPETZBDD)) {
    v = Biddy_GetTag(BiddyT(f));
    zthen = 1.0;
    while (v != Biddy_Managed_GetTopVariable(MNG,BiddyT(f))) {
      q = (double *) Biddy_Managed_GetVariableData(MNG,v);
      if (q) {
        zthen = zthen * (1 - *q);
      }
      v = biddyVariableTable.table[v].next;
    }
    v = Biddy_GetTag(BiddyE(f));
    zelse = 1.0;
    while (v != Biddy_Managed_GetTopVariable(MNG,BiddyE(f))) {
      q = (double *) Biddy_Managed_GetVariableData(MNG,v);
      if (q) {
        zelse = zelse * (1 - *q);
      }
      v = biddyVariableTable.table[v].next;
    }
    v = Biddy_Managed_GetTopVariable(MNG,f);
    q = (double *) Biddy_Managed_GetVariableData(MNG,v);
    if (q) {
      *c1 = (*c1 * zthen * *q) + (r1 * zelse * (1.0 - *q));
    } else {
      /* empty value means 100% probability, i.e. *q = 0 */
      *c1 = r1 * zelse;
    }
  }

  /* DEBUGGING */
  /*
  printf("evalProbability: c1 = %.4lf, c0 = %.4lf\n",*c1,*c0);
  */

  /* STORE PATHS INTO LOCALINFO */
  if ((biddyManagerType == BIDDYTYPEOBDDC) || (biddyManagerType == BIDDYTYPEOBDD)) {
    BiddySetPath1ProbabilitySum(f,*c1); /* positive edge is stored */
    BiddySetPath0ProbabilitySum(f,*c0); /* positive edge is stored */
    if (Biddy_GetMark(f)) {
      *c1 = BiddyGetPath0ProbabilitySum(f); /* actual edge is returned */
      *c0 = BiddyGetPath1ProbabilitySum(f); /* actual edge is returned */
    }
  }
  else if ((biddyManagerType == BIDDYTYPEZBDDC) || (biddyManagerType == BIDDYTYPEZBDD)) {
    BiddySetPath1ProbabilitySum(f,*c1); /* positive edge is stored */
    BiddySetLeftmost(f,*leftmost);  /* positive edge is stored */
    if (Biddy_GetMark(f)) {
      *leftmost = !(*leftmost); /* actual edge is returned */
      v = Biddy_Managed_GetTopVariable(MNG,f);
      z = 1.0;
      while (v) {
        q = (double *) Biddy_Managed_GetVariableData(MNG,v);
        if (q) {
          z = z * (1 - *q);
        }
        v = biddyVariableTable.table[v].next;
      }
      if (*leftmost) {
        (*c1) += z; /* actual edge is returned */
      } else {
        (*c1) -= z; /* actual edge is returned */
      }
    }
  }
  else if ((biddyManagerType == BIDDYTYPETZBDDC) || (biddyManagerType == BIDDYTYPETZBDD)) {
    BiddySetPath1ProbabilitySum(f, *c1);
    if (Biddy_GetMark(f)) {
      /* not implemented, yet */
    }
  }

  /* DEBUGGING */
  /*
  printf("evalProbability finished, results returned: c1=%.4lf, c0=%.4lf\n",*c1,*c0);
  */

  return;
}

/*******************************************************************************
\brief Function checkFunctionOrdering.

### Description
    Check if the given function has correct orderring.
### Side effects
    Function will select all nodes except terminal node.
### More info
*******************************************************************************/

static Biddy_Boolean
checkFunctionOrderingR(Biddy_Manager MNG, Biddy_Edge f)
{
  if (Biddy_IsTerminal(f) || Biddy_Managed_IsSelected(MNG,f)) return TRUE;
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
\brief Function getGlobalOrdering.

### Description
    Global ordering is the number of zeros in corresponding line of
    orderingTable. Topmost variable has global ordering 1. Bottommost variable
    is '1' and has global ordering equal to numUsedVariables.
### Side effects
### More info
*******************************************************************************/

static Biddy_Variable
getGlobalOrdering(Biddy_Manager MNG, Biddy_Variable v)
{
  Biddy_Variable k,n;

  /* VARIANT 1 */
  n = Biddy_Managed_VariableTableNum(MNG);
  for (k=0; k<Biddy_Managed_VariableTableNum(MNG); k++) {
    if (GET_ORDER(biddyOrderingTable,v,k)) n--;
  }

  /* TO DO: VARIANT 2 */
  /* start with 0 and use .prev to find the variable */

  return n;
}

/*******************************************************************************
\brief Function swapWithHigher determines higher variable and calls swapping.

### Description
    Higher (greater) variable is the bottommore one!
    The highest variable is constant variable "1".
    Global ordering is number of zeros in corresponding line of orderingTable.
    Constant variable '1' has global ordering greater than all others.
    Returns the higher element which has been used in swapping.
### Side effects
### More info
*******************************************************************************/

static Biddy_Variable
swapWithHigher(Biddy_Manager MNG, Biddy_Variable v, Biddy_Boolean *active)
{
  Biddy_Variable vhigh;

  if (v == 0) {
    vhigh = 0; /* constant variable cannot be swapped */
  } else {
    vhigh = biddyVariableTable.table[v].next;
  }

  /* DEBUGGING */
  /*
  printf("swapWithHigher: v=%s(%u), vlow=%s(%u)\n",
         biddyVariableTable.table[v].name,v,
         biddyVariableTable.table[vhigh].name,vhigh);
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
    Returns the lower element which has been used in swapping.
### Side effects
### More info
*******************************************************************************/

static Biddy_Variable
swapWithLower(Biddy_Manager MNG, Biddy_Variable v, Biddy_Boolean *active)
{
  Biddy_Variable vlow;

  if (v == 0) {
    vlow = 0; /* constant variable cannot be swapped */
  } else if (BiddyIsLowest(v)) {
    vlow = 0; /* v is the lowest variable */
  } else {
    vlow = biddyVariableTable.table[v].prev;
  }

  /* DEBUGGING */
  /*
  printf("swapWithLower: v=%s(%u), vlow=%s(%u)\n",
         biddyVariableTable.table[v].name,v,
         biddyVariableTable.table[vlow].name,vlow);
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
    variables are swapped. Argument active is set to TRUE if some nodes
    have been swapped.
### Side effects
    The result is wrong if low is not next to high!
    For TZBDDs, node selection is used and thus any existing selection
    is deleted before swapping.
    For TZBDDs, GC is called at the end because algorithm is not able
    to keep the system clean.
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
  BiddySystemReport(MNG);
  */

  /* update active ordering */
  /* this is the same for all BDD types */
  /* IMPORTANT: For TZBDD it is explicitely assumed that variables are reordered before node manipulation! */
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

  minvar = 0;
  maxvar = 0;
  if ((biddyManagerType == BIDDYTYPEOBDDC) || (biddyManagerType == BIDDYTYPEOBDD) ||
      (biddyManagerType == BIDDYTYPEZBDDC) || (biddyManagerType == BIDDYTYPEZBDD))
  {
    /* ONLY NODES WITH var == low ARE AFFECTED */
    minvar = low;
    maxvar = biddyVariableTable.table[low].next;
  }
  else if ((biddyManagerType == BIDDYTYPETZBDDC) || (biddyManagerType == BIDDYTYPETZBDD))
  {
    /* NODES WITH var < low ARE AFFECTED */
    minvar = low;
    do {
      minvar = biddyVariableTable.table[minvar].prev;
    } while (!(Biddy_Managed_IsLowest(MNG,minvar)));
    maxvar = high; /* low and high has already been reordered */

    /* node selection is used and thus any existing selection is deleted before swapping */
    Biddy_Managed_DeselectAll(MNG);
  }

  *active = FALSE; /* swapping has not introduced brooken nodes, yet */
  newBegin = newEnd = NULL;

  var = maxvar;
  while (var != minvar) {
    var = biddyVariableTable.table[var].prev;

    if ((biddyManagerType == BIDDYTYPEOBDDC) || (biddyManagerType == BIDDYTYPEOBDD) ||
        (biddyManagerType == BIDDYTYPEZBDDC) || (biddyManagerType == BIDDYTYPEZBDD))
    {

      sup = newBegin = biddyVariableTable.table[var].firstNode;

      /* PREPARE NEW LIST FOR 'LOW' */
      /* TERMINAL NODE biddyNodeTable.table[0] IS USED HERE BUT ONLY AS TMP */
      biddyVariableTable.table[var].lastNode->list = NULL;
      biddyVariableTable.table[var].firstNode =
        biddyVariableTable.table[var].lastNode = biddyNodeTable.table[0];

    }

    else if ((biddyManagerType == BIDDYTYPETZBDDC) || (biddyManagerType == BIDDYTYPETZBDD))
    {

      biddyVariableTable.table[var].lastNode->list = NULL;
      sup = biddyVariableTable.table[var].firstNode;

    }

    else if ((biddyManagerType == BIDDYTYPEOFDDC) || (biddyManagerType == BIDDYTYPEOFDD) ||
                (biddyManagerType == BIDDYTYPEZFDDC) || (biddyManagerType == BIDDYTYPEZFDD) ||
                (biddyManagerType == BIDDYTYPETZFDDC) || (biddyManagerType == BIDDYTYPETZFDD))
    {
      fprintf(stderr,"swapVariables: this BDD type is not supported, yet!\n");
      return;
    }

    else {
      fprintf(stderr,"swapVariables: Unsupported BDD type!\n");
      return;
    }

    while (sup) {

      assert( BiddyV(sup) == var );

      /* DEBUGGING */
      /*
      printf("SWAP ");
      debugNode(MNG,sup);
      printf("\n");
      Biddy_Managed_PrintfBDD(MNG,sup);
      */

      if ((biddyManagerType == BIDDYTYPEOBDDC) || (biddyManagerType == BIDDYTYPEOBDD) ||
          (biddyManagerType == BIDDYTYPEZBDDC) || (biddyManagerType == BIDDYTYPEZBDD))
      {

        /* IF ((VARIABLE == low) AND (AT LEAST ONE SUCCESSOR == high)) THEN NODE IS AFFECTED */
        if ((BiddyV(sup->f) == high) || (BiddyV(sup->t) == high)) {

          /* DEBUGGING */
          /*
          printf("NODE-TO-BE-CHANGED ");
          debugNode(MNG,sup);
          printf("\n");
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
          printf("NODE-CHANGED ");
          debugNode(MNG,sup);
          printf("\n");
          */

          /* rehash this node */
          sup->prev->next = sup->next; /* remove node from the old chain */
          if (sup->next) sup->next->prev = sup->prev; /* remove node from the old chain */
          hash = nodeTableHash(high,sup->f,sup->t,biddyNodeTable.size);
          tmp = biddyNodeTable.table[hash]; /* the beginning of new chain */
          tmp1 = findNodeTable(MNG,high,sup->f,sup->t,&tmp); /* to calculate tmp1, such node cannot exists! */
          assert( !tmp || (sup->v != tmp->v) );
          addNodeTable(MNG,hash,sup,tmp1); /* add node to hash table */

          /* add this node to the list of renamed nodes (newBegin,newEnd) */
          newEnd = sup;

          sup = (BiddyNode *) sup->list;

        } else {

          /* DEBUGGING */
          /*
          printf("NODE-NOT-CHANGED ");
          debugNode(MNG,sup);
          printf("\n");
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

        /* THIS ALGORITHM IS VERY DIFFERENT AND MUCH LESS EFFFICIENT */
        /* THE WORST: ALGORITHM IS NOT ABLE TO KEEP THE SYSTEM CLEAN WITH A SINGLE PASS */
        /* THUS, A GC IS REQUIRED AT THE END OF EVERY SWAP TO REPAIR THE BROKEN SYSTEM */
        /* IS IT POSSIBLE TO IMPROVE THIS ??? */

        if (!sup->f && !sup->t) {
          sup = (BiddyNode *) sup->list;
          continue;
        }

        /* DEBUGGING */
        /*
        printf("ONE-SWAP-EDGE FOR ELSE %p\n",(void *)sup->f);
        */
        oneSwapEdge(MNG,sup->f,low,high,&u0,active);
        Biddy_Managed_SelectNode(MNG,u0);
        if (BiddyE(u0)) Biddy_Managed_SelectNode(MNG,BiddyE(u0)); /* maybe not necessary but not wrong */
        if (BiddyT(u0)) Biddy_Managed_SelectNode(MNG,BiddyT(u0)); /* maybe not necessary but not wrong */

        /* DEBUGGING */
        /*
        printf("ONE-SWAP-EDGE FOR THEN %p\n",(void *)sup->t);
        */
        oneSwapEdge(MNG,sup->t,low,high,&u1,active);
        Biddy_Managed_SelectNode(MNG,u1);
        if (BiddyE(u1)) Biddy_Managed_SelectNode(MNG,BiddyE(u1)); /* maybe not necessary but not wrong */
        if (BiddyT(u1)) Biddy_Managed_SelectNode(MNG,BiddyT(u1)); /* maybe not necessary but not wrong */

        if ((sup->f != u0) || (sup->t != u1)) {
          /* this node is affected */

          /* DEBUGGING */
          /*
          printf("NODE-TO-BE-CHANGED ");
          debugNode(MNG,sup);
          printf("\n");
          Biddy_Managed_PrintfBDD(MNG,sup);
          */

          sup->f = u0;
          sup->t = u1;

          /* DEBUGGING */
          /*
          printf("NODE-CHANGED ");
          debugNode(MNG,sup);
          printf("\n");
          Biddy_Managed_PrintfBDD(MNG,sup);
          */

          sup->prev->next = sup->next; /* remove node from the old chain */
          if (sup->next) sup->next->prev = sup->prev; /* remove node from the old chain */
          hash = nodeTableHash(sup->v,sup->f,sup->t,biddyNodeTable.size);
          tmp = biddyNodeTable.table[hash]; /* the beginning of new chain */
          tmp1 = findNodeTable(MNG,sup->v,sup->f,sup->t,&tmp); /* to calculate tmp1 */

          /* no problem if the same node already exists, the existing one will be changed  */
          /* if the same node already exists then new node is inserted immediately before it */

          /* DEBUGGING */
          /*
          if (tmp && (sup->v == tmp->v)) {
            printf("EXISTING NODE ");
            debugNode(MNG,tmp);
            printf("\n");
            Biddy_Managed_PrintfBDD(MNG,sup);
            Biddy_Managed_PrintfBDD(MNG,tmp);
          }
          */

          addNodeTable(MNG,hash,sup,tmp1); /* add new node to hash table */

        } else {

          /* DEBUGGING */
          /*
          printf("NODE-NOT-CHANGED ");
          debugNode(MNG,sup);
          printf("\n");
          */

        }

        sup = (BiddyNode *) sup->list;
      }

      else if ((biddyManagerType == BIDDYTYPEOFDDC) || (biddyManagerType == BIDDYTYPEOFDD) ||
                  (biddyManagerType == BIDDYTYPEZFDDC) || (biddyManagerType == BIDDYTYPEZFDD) ||
                  (biddyManagerType == BIDDYTYPETZFDDC) || (biddyManagerType == BIDDYTYPETZFDD))
      {
        fprintf(stderr,"swapVariables: this BDD type is not supported, yet!\n");
        return;
      }

      else {
        fprintf(stderr,"swapVariables: Unsupported BDD type!\n");
        return;
      }

    } /* while (sup) */

  } /*   while (var != minvar)  */

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
        debugNode(MNG,sup);
        printf("\n");
        if (sup == newEnd) {
          sup = NULL;
        } else {
          sup = (BiddyNode *) sup->list;
        }
      }
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

    /* REPAIR TERMINAL NODE */
    biddyNodeTable.table[0]->list = NULL;

  }

  else if ((biddyManagerType == BIDDYTYPETZBDDC) || (biddyManagerType == BIDDYTYPETZBDD))
  {

    /* WE HAVE TO CHECK ALL ELEMENTS AND NON-CONSTANT FORMULAE AS WELL */
    /* VARIABLES DO NOT NEED TO BE CHECKED, BUT THEY SHOULD BE SELECTED */
    /* for TZBDD, first two formulae are constants */

    for (n = 1; n < biddyVariableTable.num; n++) {

      /* DEBUGGING */
      /*
      printf("ONE-SWAP-EDGE FOR ELEMENT %s (%p)\n",
             Biddy_Managed_GetVariableName(MNG,n),(void *)biddyVariableTable.table[n].element);
      */

      Biddy_Managed_SelectNode(MNG,biddyVariableTable.table[n].variable);

      oneSwapEdge(MNG,biddyVariableTable.table[n].element,low,high,&u,active);
      biddyVariableTable.table[n].element = u;
      Biddy_Managed_SelectNode(MNG,u);
      if (BiddyE(u)) Biddy_Managed_SelectNode(MNG,BiddyE(u)); /* maybe not necessary but not wrong */
      if (BiddyT(u)) Biddy_Managed_SelectNode(MNG,BiddyT(u)); /* maybe not necessary but not wrong */
    }

    for (n = 2; n < biddyFormulaTable.size; n++) {
      if (!Biddy_IsNull(biddyFormulaTable.table[n].f) &&
          (biddyFormulaTable.table[n].f != biddyTerminal))
      {
        /* DEBUGGING */
        /*
        printf("ONE-SWAP-EDGE FOR FORMULA %s (%p)\n",
               Biddy_Managed_GetIthFormulaName(MNG,n),(void *)biddyFormulaTable.table[n].f);
        */

        oneSwapEdge(MNG,biddyFormulaTable.table[n].f,low,high,&u,active);
        biddyFormulaTable.table[n].f = u;
        Biddy_Managed_SelectNode(MNG,u);
        if (BiddyE(u)) Biddy_Managed_SelectNode(MNG,BiddyE(u)); /* maybe not necessary but not wrong */
        if (BiddyT(u)) Biddy_Managed_SelectNode(MNG,BiddyT(u)); /* maybe not necessary but not wrong */
      }
    }

    /* WE HAVE TO CHECK ALL NODES WITH var == low AND var == high FOR UNSELECTED NODES */
    /* UNSELECTED NODES ARE NOT REFERENCED WITH EXTERNAL REFERENCES */
    /* THEY MAY HAVE ONE OR MULTIPLE INTERNAL REFERENCES BUT WE CANNOT CHECK AND REPAIR THIS HERE  */
    /* IT IS THE BEST TO MAKE SUCH NODES IRREGULAR TO NOTICE THE USER ASAP */
    biddyVariableTable.table[low].lastNode->list = biddyVariableTable.table[high].firstNode;
    biddyVariableTable.table[high].lastNode->list = NULL;
    sup = biddyVariableTable.table[low].firstNode;
    while (sup) {
      if (!Biddy_Managed_IsSelected(MNG,(Biddy_Edge)sup)) {

        /* DEBUGGING */
        /*
        printf("UNSELECTED NODE ");
        debugNode(MNG,sup);
        printf("\n");
        Biddy_Managed_PrintfBDD(MNG,sup);
        */

        BiddyN(sup)->expiry = 1;
        sup->f = sup->t = NULL;
        *active = TRUE; /* swapping has introduced brooken nodes */

      }
      sup = (BiddyNode *) sup->list;
    }

    /* WE HAVE TO CHECK FOR NODES WITH BAD ORDERING - TZBDD ONLY! */
    /* THESE NODES EXIST BUT THEY ARE NOT REFERENCED FROM ANY USER-VISIBLE FUNCTION */
    /* IT IS THE BEST TO MAKE SUCH NODES IRREGULAR AND CALL GC ASAP */
    biddyVariableTable.table[low].lastNode->list = NULL;
    sup = biddyVariableTable.table[low].firstNode;
    while (sup) {
      if ((sup->f && !BiddyIsSmaller(low,BiddyV(sup->f))) ||
          (sup->t && !BiddyIsSmaller(low,BiddyV(sup->t))) ||
          (sup->f && !BiddyIsSmaller(low,Biddy_GetTag(sup->f))) ||
          (sup->t && !BiddyIsSmaller(low,Biddy_GetTag(sup->t))))
      {

        /* DEBUGGING */
        /**/
        printf("!!!CONFIRMED-4\n");
        printf("BAD-ORDERING-TO-BE-REMOVED ");
        debugNode(MNG,sup);
        printf("\n");
        /**/

        BiddyN(sup)->expiry = 1;
        sup->f = sup->t = NULL;
        *active = TRUE; /* swapping has introduced brooken nodes */

      }
      sup = (BiddyNode *) sup->list;
    }

    /* node selection was used */
    Biddy_Managed_DeselectAll(MNG);
  }

  else if ((biddyManagerType == BIDDYTYPEOFDDC) || (biddyManagerType == BIDDYTYPEOFDD) ||
              (biddyManagerType == BIDDYTYPEZFDDC) || (biddyManagerType == BIDDYTYPEZFDD) ||
              (biddyManagerType == BIDDYTYPETZFDDC) || (biddyManagerType == BIDDYTYPETZFDD))
  {
    fprintf(stderr,"swapVariables: this BDD type is not supported, yet!\n");
    return;
  }

  else {
    fprintf(stderr,"swapVariables: Unsupported BDD type!\n");
    return;
  }

  /* REPAIR BROKEN NODES - AFTER SWAPPING EXTERNAL REFERENCES SHOULD NOT BE OBSOLETE */
  /* NOT NEEDED FOR TZBDDs, BECASUE WE WILL CALL GC BEFORE THE END OF THIS FUNCTION */
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
          !Biddy_IsTerminal(biddyFormulaTable.table[n].f))
      {
        if ((biddyManagerType == BIDDYTYPEOBDDC) || (biddyManagerType == BIDDYTYPEOBDD) ||
            (biddyManagerType == BIDDYTYPEOFDDC) || (biddyManagerType == BIDDYTYPEOFDD))
        {
          if ((BiddyE(biddyFormulaTable.table[n].f) == biddyZero) && (BiddyT(biddyFormulaTable.table[n].f) == biddyTerminal)) {
            /* this is variable, already refreshed */
          } else {
            BiddyProlongOne(biddyFormulaTable.table[n].f,biddyFormulaTable.table[n].expiry);
          }
        }
        else if ((biddyManagerType == BIDDYTYPEZBDD) || (biddyManagerType == BIDDYTYPEZFDD))
        {
          if ((BiddyE(biddyFormulaTable.table[n].f) == biddyZero) && (BiddyT(biddyFormulaTable.table[n].f) == biddyTerminal)) {
            /* this is element, already refreshed */
          } else {
            BiddyProlongOne(biddyFormulaTable.table[n].f,biddyFormulaTable.table[n].expiry);
          }
        }
        else if ((biddyManagerType == BIDDYTYPEZBDDC) || (biddyManagerType == BIDDYTYPEZFDDC))
        {
          if ((BiddyE(biddyFormulaTable.table[n].f) == biddyTerminal) && (BiddyT(biddyFormulaTable.table[n].f) == biddyTerminal)) {
            /* this is element, already refreshed */
          } else {
            BiddyProlongOne(biddyFormulaTable.table[n].f,biddyFormulaTable.table[n].expiry);
          }
        }
        else if ((biddyManagerType == BIDDYTYPETZBDDC) || (biddyManagerType == BIDDYTYPETZBDD) ||
                 (biddyManagerType == BIDDYTYPETZFDDC) || (biddyManagerType == BIDDYTYPETZFDD))
        {
          /* NOT NEEDED, BECAUSE GC CALLED BELOW WILL DO THE WORK */
        }
        else if ((biddyManagerType == BIDDYTYPEOFDDC) || (biddyManagerType == BIDDYTYPEOFDD) ||
                 (biddyManagerType == BIDDYTYPEZFDDC) || (biddyManagerType == BIDDYTYPEZFDD))
        {
          fprintf(stderr,"swapVariables: this BDD type is not supported, yet!\n");
          return;
        }
        else {
          fprintf(stderr,"swapVariables: Unsupported BDD type!\n");
          return;
        }
      }
    }
  }

  /* For TZBDDs, GC is called at the end because algorithm is not able to keep the system clean */
  /* nodes with both successors NULL (except terminal node 1) must be immediately removed */
  if (*active) {
    if ((biddyManagerType == BIDDYTYPETZBDDC) || (biddyManagerType == BIDDYTYPETZBDD))
    {
      Biddy_Managed_ForceGC(MNG);
    }
  }

  /* DEBUGGING */
  /*
  printf("SWAPPING FINISH: low =  \"%s\", high = \"%s\"\n",
         Biddy_Managed_GetVariableName(MNG,low),
         Biddy_Managed_GetVariableName(MNG,high));
  printf("MNG = %p\n",MNG);
  debugORDERING(MNG,biddyOrderingTable,biddyVariableTable.num);
  */
}

/*******************************************************************************
\brief Function oneSwap calculates u0 and u1 needed within variable swapping.

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
  printf("ONESWAP ");
  printf("NODE<%p:\"%s\"",(void *)sup,Biddy_Managed_GetTopVariableName(MNG,sup));
  printf("@%u",sup->expiry);
  if (sup->f == sup->t) printf("DOUBLE");
  printf("> ");
  printf("ELSE<");
  debugEdge(MNG,sup->f);
  sup1 = (BiddyNode *) BiddyP(sup->f);
  if (sup1->f == sup1->t) printf("DOUBLE");
  printf("> ");
  printf("THEN<");
  if (Biddy_GetMark(sup->t)) printf("*");
  debugEdge(MNG,sup->t);
  sup1 = (BiddyNode *) BiddyP(sup->t);
  if (sup1->f == sup1->t) printf("DOUBLE");
  printf(">\n");
  Biddy_Managed_PrintfBDD(MNG,sup);
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
    printf("NODE-OBSOLETE ");
    debugNode(MNG,sup1);
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
      fprintf(stderr,"oneSwap: this BDD type is not supported, yet!\n");
      f00 = f01 = biddyNull;
      return;
    }
    else {
      fprintf(stderr,"oneSwap: Unsupported BDD type!\n");
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
    printf("NODE-OBSOLETE ");
    debugNode(MNG,sup1);
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
      fprintf(stderr,"oneSwap: this BDD type is not supported, yet!\n");
      f00 = f01 = biddyNull;
      return;
    }
    else {
      fprintf(stderr,"oneSwap: Unsupported BDD type!\n");
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
    fprintf(stderr,"oneSwap: this BDD type is not supported, yet!\n");
    f00 = f01 = biddyNull;
    return;
  }

  else {
    fprintf(stderr,"oneSwap: Unsupported BDD type!\n");
    f00 = f01 = biddyNull;
    return;
  }

}

/*******************************************************************************
\brief Function oneSwapEdge calculates u needed within variable swapping.

### Description
    This function is used by TZBDDs, only.
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

    /* DEBUGGING */
    /*
    printf("TZBDD SWAP: SITUATION 1\n");
    */

    /* ONE SWAP for TZBDD */
    /* this will add zero, one or two new nodes with variable 'low' */
    /* and zero or one new node with variable high */

    oneSwap(MNG,BiddyN(sup),low,high,&u0,&u1);

    /* the result is not refreshed and we are expecting, that */
    /* formulae are repaired before garbage collection */
    if (Biddy_GetTag(sup) == low) {
       *u = BiddyManagedTaggedFoaNode(MNG,high,u0,u1,high,FALSE);
    } else {
       *u = BiddyManagedTaggedFoaNode(MNG,high,u0,u1,Biddy_GetTag(sup),FALSE);
    }

    if (BiddyN(sup) != BiddyN(*u)) {
      /* node pointed by sup cannot be changed or deleted because it can be referenced many times */
      /* however, after proceesing all referee nodes a node pointed by sup will not be needed, anymore */
      /* we will mark it as obsolete and we are expecting, that */
      /* formulae are repaired before garbage collection */
      BiddyN(sup)->expiry = 1;
      *active = TRUE; /* swapping has introduced brooken nodes */
    }

    /* DEBUGGING */
    /*
    printf("EDGE-CREATED ");
    debugEdge(MNG,*u);
    printf("\n");
    */

  } else if (((Biddy_GetTag(sup) == low) || BiddyIsSmaller(Biddy_GetTag(sup),high)) && (BiddyV(sup) == high)) {
    /* variables already reordered, now high < low !! */

    /* DEBUGGING */
    /*
    printf("TZBDD SWAP: SITUATION 2\n");
    */

    u0 = BiddyManagedTaggedFoaNode(MNG,low,BiddyE(sup),biddyZero,low,FALSE);
    u1 = BiddyManagedTaggedFoaNode(MNG,low,BiddyT(sup),biddyZero,low,FALSE);

    /* the result is not refreshed and we are expecting, that */
    /* formulae are repaired before garbage collection */
    if (Biddy_GetTag(sup) == low) {
       *u = BiddyManagedTaggedFoaNode(MNG,high,u0,u1,high,FALSE);
    } else {
       *u = BiddyManagedTaggedFoaNode(MNG,high,u0,u1,Biddy_GetTag(sup),FALSE);
    }

    if (BiddyN(sup) != BiddyN(*u)) {
      /* node pointed by sup cannot be changed or deleted because it can be referenced many times */
      /* however, after proceesing all referee nodes a node pointed by sup will not be needed, anymore */
      /* we will mark it as obsolete and we are expecting, that */
      /* formulae are repaired before garbage collection */
      BiddyN(sup)->expiry = 1;
      *active = TRUE; /* swapping has introduced brooken nodes */
    } else {

      /* DEBUGGING */
      /**/
      printf("!!!CONFIRMED-2\n");
      /**/

    }

    /* DEBUGGING */
    /*
    printf("EDGE-CREATED ");
    debugEdge(MNG,*u);
    printf("\n");
    */

  } else if ((Biddy_GetTag(sup) == high) && BiddyIsSmaller(low,BiddyV(sup))) {

    /* DEBUGGING */
    /*
    printf("TZBDD SWAP: SITUATION 3\n");
    */

    /* variables already reordered, now high < low !! */
    Biddy_SetTag(*u,low);
    *u = Biddy_Managed_IncTag(MNG,*u);
    if (BiddyN(sup) != BiddyN(*u)) {
      /* old sup is not needed, anymore */
      /* we will mark it as obsolete and we are expecting, that */
      /* formulae are repaired before garbage collection */
      BiddyN(sup)->expiry = 1;
      *active = TRUE; /* swapping has introduced brooken nodes */
    }

    *u = BiddyManagedTaggedFoaNode(MNG,low,*u,*u,high,FALSE);

  } else if (Biddy_GetTag(sup) == low) {

    /* DEBUGGING */
    /*
    printf("TZBDD SWAP: SITUATION 4\n");
    */

    /* CHANGE TAG TO high - THIS CAN BE DONE HERE WITHOUT ANY CHECKING */
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
\brief Function alphabeticOrdering.

### Description
    Create an alphabetic ordering using all the variables in Variable table.
    Named variables are ordered according to their names. Numbered variables
    are ordered according to their numbers and are always smaller (topmore) as
    any named variable.
### Side effects
### More info
*******************************************************************************/

static void
alphabeticOrdering(Biddy_Manager MNG, BiddyOrderingTable table)
{
  Biddy_Variable i,j;

  nullOrdering(table);
  for (i=1; i<biddyVariableTable.num; i++) {
    for (j=1; j<biddyVariableTable.num; j++) {
      if ((i != j) && biddyVariableTable.table[i].name && biddyVariableTable.table[j].name &&
          (strcmp(biddyVariableTable.table[i].name,biddyVariableTable.table[j].name) < 0))
      {
        SET_ORDER(table,i,j);
        CLEAR_ORDER(table,j,i);
      }
    }
  }

  /* DEBUGGING */
  /*
  printf("CURRENT ORDERING\n");
  debugORDERING(MNG,biddyOrderingTable,biddyVariableTable.num);
  printf("ALPHABETIC ORDERING\n");
  debugORDERING(MNG,table,biddyVariableTable.num);
  */
}

/*******************************************************************************
\brief Function nodeNumberOrdering.

### Description
    Calculates the node number and the actual variable ordering.
### Side effects
    Ordering of constant variable is not included!
    All nodes except terminal node will be selected.
    For each variable its number is counted in numone.
### More info
*******************************************************************************/

static void
nodeNumberOrdering(Biddy_Manager MNG, Biddy_Edge f, unsigned int *i,
                   BiddyOrderingTable ordering)
{
  Biddy_Edge sup;

  if (!Biddy_IsTerminal(f)) {
    biddyVariableTable.table[BiddyV(f)].numone++;
    Biddy_Managed_SelectNode(MNG,f);
    (*i)++;
    if (!Biddy_IsTerminal((sup=BiddyE(f)))) {
      SET_ORDER(ordering,BiddyV(f),BiddyV(sup));
      if (!Biddy_Managed_IsSelected(MNG,sup)) nodeNumberOrdering(MNG,sup,i,ordering);
    }
    if (!Biddy_IsTerminal((sup=BiddyT(f)))) {
      SET_ORDER(ordering,BiddyV(f),BiddyV(sup));
      if (!Biddy_Managed_IsSelected(MNG,sup)) nodeNumberOrdering(MNG,sup,i,ordering);
    }
  }
}

/*******************************************************************************
\brief Function reorderVariables.

### Description
    Used in Biddy_Managed_ConstructBDD.
### Side effects
### More info
*******************************************************************************/

static void
reorderVariables(Biddy_Manager MNG, int numV, BiddyVariableOrder *tableV,
                 int numN, BiddyNodeList *tableN, int id)
{
  /* NOT IMPLEMENTED, YET */
}

/*******************************************************************************
\brief Function constructBDD.

### Description
    Used in Biddy_Managed_ConstructBDD.
    Elements of node list has the following format:
    terminal node zero = (type=0,l=-1,r=-1),
    terminal node one = (type=1,l=-1,r=-1),
    regular label = (type=2,r=-1),
    complemented label = (type=3,r=-1),
    regular node = (type=4),
    node with complemented successor(s) = (type=5)
    The first element in nodelist is ignored. An example node
    list is: 'Biddy 0 Biddy 2 1 -1 1 B 4 2 3 2 0 0 -1 -1 3 i 4 4
    9 4 d 4 5 6 5 0 0 -1 -1 6 y 4 7 8 7 0 0 -1 -1 8 1 1 -1 -1 9
    d 4 6 10 10 1 1 -1 -1'
### Side effects
### More info
*******************************************************************************/

static void
constructBDD(Biddy_Manager MNG, int numN, BiddyNodeList *tableN, int id)
{
  int i,idx,lidx,ridx;
  Biddy_Edge l,r;
  Biddy_Variable v;

  /* NODE'S id MAY NOT BE THE SAME AS NODE'S INDEX !! */
  idx = -1;
  if (tableN[id].id == id) {
    idx = id;
  }
  i = 0;
  while (i<numN && (idx == -1))
  {
    if (tableN[i].id == id) idx = i;
    i++;
  }
  /* if (id != idx) printf("constructBDD: id=%d,idx=%d\n",id,idx); */
  if (idx == -1) {
    printf("constructBDD: wrong index, numN=%d, id=%d\n",numN,id);
    tableN[idx].f = biddyNull;
    exit(1);
  }
  lidx = -1;
  if (tableN[idx].l != -1) {
    if (tableN[tableN[idx].l].id == tableN[idx].l) {
      lidx = tableN[idx].l;
    }
  }
  ridx = -1;
  if (tableN[idx].r != -1) {
    if (tableN[tableN[idx].r].id == tableN[idx].r) {
      ridx = tableN[idx].r;
    }
  }
  i = 0;
  while (i<numN && (
         ((tableN[idx].l != -1) && (lidx == -1)) ||
         ((tableN[idx].r != -1) && (ridx == -1))))
  {
    if ((tableN[idx].l != -1) && (tableN[i].id == tableN[idx].l)) lidx = i;
    if ((tableN[idx].r != -1) && (tableN[i].id == tableN[idx].r)) ridx = i;
    i++;
  }
  if (tableN[idx].l != -1) {
    if (lidx == -1) {
      printf("constructBDD: wrong left successor index\n");
      tableN[idx].f = biddyNull;
      exit(1);
    }
  }
  if (tableN[idx].r != -1) {
    if (ridx == -1) {
      printf("constructBDD: wrong right successor index\n");
      tableN[idx].f = biddyNull;
      exit(1);
    }
  }

  /* DEBUGGING */
  /*
  printf("constructBDD: id=%d, idx=%d\n",id,idx);
  printf("TABLE[%d]: %s, type:%d, l:%d, ltag:%u, r:%d, rtag:%u\n",
         tableN[idx].id,tableN[idx].name,tableN[idx].type,
         tableN[idx].l,tableN[idx].ltag,
         tableN[idx].r,tableN[idx].rtag);
  */

  if (!tableN[idx].created) {

    if (!tableN[lidx].created) {
      constructBDD(MNG,numN,tableN,tableN[idx].l);
    }

    if ((ridx != -1) && (!tableN[ridx].created)) {
      constructBDD(MNG,numN,tableN,tableN[idx].r);
    }

    if (tableN[idx].type == 4) {
      /* both edges are regular */
      v = Biddy_Managed_GetVariable(MNG,tableN[idx].name);
      if (!v) {
        printf("constructBDD ERROR: variable <%s> not found!\n",tableN[idx].name);
        exit(1);
      }
      l = tableN[lidx].f;
      r = tableN[ridx].f;
      if ((biddyManagerType == BIDDYTYPETZBDDC) || (biddyManagerType == BIDDYTYPETZBDD))
      {
        /* using Special case 2 of BiddyManagedTaggedFoaNode */
        Biddy_SetTag(l,tableN[idx].ltag);
        Biddy_SetTag(r,tableN[idx].rtag);
        tableN[idx].f = Biddy_Managed_TaggedFoaNode(MNG,v,l,r,0,TRUE);
      } else {
        tableN[idx].f = Biddy_Managed_TaggedFoaNode(MNG,v,l,r,v,TRUE);
      }
    }

    if (tableN[idx].type == 5) {
      v = Biddy_Managed_GetVariable(MNG,tableN[idx].name);
      if (biddyManagerType == BIDDYTYPEOBDDC) {
        /* left edge is complemented */
        l = Biddy_Complement(tableN[lidx].f);
        r = tableN[ridx].f;
      }
      else if (biddyManagerType == BIDDYTYPEZBDDC) {
        /* right edge is complemented */
        l = tableN[lidx].f;
        r = Biddy_Complement(tableN[ridx].f);
      }
      else if (biddyManagerType == BIDDYTYPETZBDDC) {
        /* right edge is complemented */
        l = tableN[lidx].f;
        r = Biddy_Complement(tableN[ridx].f);
      }
      else {
        printf("constructBDD: wrong complemented edge\n");
        tableN[idx].f = biddyNull;
        return;
      }
      if (biddyManagerType == BIDDYTYPETZBDDC) {
        /* using Special case 2 of BiddyManagedTaggedFoaNode */
        Biddy_SetTag(l,tableN[idx].ltag);
        Biddy_SetTag(r,tableN[idx].rtag);
        tableN[idx].f = Biddy_Managed_TaggedFoaNode(MNG,v,l,r,0,TRUE);
      } else {
        tableN[idx].f = Biddy_Managed_TaggedFoaNode(MNG,v,l,r,v,TRUE);
      }
    }

    tableN[idx].created = TRUE;
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
  if (!Biddy_IsTerminal(f)) {
    fprintf(stdout," (");
    writeBDD(MNG,BiddyE(f));
    fprintf(stdout,") (");
    writeBDD(MNG,BiddyT(f));
    fprintf(stdout,")");
  }
}

/*******************************************************************************
\brief Function writeORDER.

### Description
    Result is produced by checking Ordering table and not by using
    prev/next fields.
### Side effects
    Slower than using prev/next fields but suitable for debugging.
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
    printf("\"%s\"",Biddy_Managed_GetVariableName(MNG,varTable[k]));
    if (k!=Biddy_Managed_VariableTableNum(MNG)-1) printf(",");
  }
  printf("\n");
}

/*******************************************************************************
\brief Function debugNode.

### Description
### Side effects
### More info
*******************************************************************************/

static void
debugNode(Biddy_Manager MNG, BiddyNode *node)
{
  printf("%p:",(void *) node);
  if (node->v < biddyVariableTable.num) {
    printf("\"%s\",",Biddy_Managed_GetVariableName(MNG,node->v));
  } else {
    printf("\"ERROR-%u\",",node->v);
  }
  if (Biddy_IsNull(node->f)) printf("NULL,");
  else {
    if (Biddy_GetMark(node->f)) printf("*");
    printf("%p:",(void *) node->f);
    if ((biddyManagerType == BIDDYTYPETZBDDC) || (biddyManagerType == BIDDYTYPETZBDD) ||
        (biddyManagerType == BIDDYTYPETZFDDC) || (biddyManagerType == BIDDYTYPETZFDD))
    {
      printf("<\"%s\">\"%s\",",Biddy_Managed_GetVariableName(MNG,Biddy_GetTag(node->f)),Biddy_Managed_GetTopVariableName(MNG,node->f));
    } else {
      printf("\"%s\",",Biddy_Managed_GetTopVariableName(MNG,node->f));
    }
  }
  if (Biddy_IsNull(node->t)) printf("NULL,");
  else {
    if (Biddy_GetMark(node->t)) printf("*");
    printf("%p:",(void *) node->t);
    if ((biddyManagerType == BIDDYTYPETZBDDC) || (biddyManagerType == BIDDYTYPETZBDD) ||
        (biddyManagerType == BIDDYTYPETZFDDC) || (biddyManagerType == BIDDYTYPETZFDD))
    {
      printf("<\"%s\">\"%s\",",Biddy_Managed_GetVariableName(MNG,Biddy_GetTag(node->t)),Biddy_Managed_GetTopVariableName(MNG,node->t));
    } else {
      printf("\"%s\">",Biddy_Managed_GetTopVariableName(MNG,node->t));
    }
  }
  printf("@%u\n",node->expiry);
}

/*******************************************************************************
\brief Function debugEdge.

### Description
### Side effects
### More info
*******************************************************************************/

static void
debugEdge(Biddy_Manager MNG, Biddy_Edge edge)
{
  printf("%p:",(void *) edge);
  if (Biddy_IsNull(edge)) {
    printf("NULL,");
    return;
  }
  if (Biddy_GetMark(edge)) printf("*");
  if ((biddyManagerType == BIDDYTYPETZBDDC) || (biddyManagerType == BIDDYTYPETZBDD) ||
      (biddyManagerType == BIDDYTYPETZFDDC) || (biddyManagerType == BIDDYTYPETZFDD))
  {
    printf("<\"%s\">\"%s\",",Biddy_Managed_GetVariableName(MNG,Biddy_GetTag(edge)),Biddy_Managed_GetTopVariableName(MNG,edge));
  } else {
    printf("\"%s\",",Biddy_Managed_GetTopVariableName(MNG,edge));
  }
  if (Biddy_IsNull(BiddyE(edge))) printf("NULL,");
  else {
    if (Biddy_GetMark(BiddyE(edge))) printf("*");
    printf("%p:",(void *) BiddyE(edge));
    if ((biddyManagerType == BIDDYTYPETZBDDC) || (biddyManagerType == BIDDYTYPETZBDD) ||
        (biddyManagerType == BIDDYTYPETZFDDC) || (biddyManagerType == BIDDYTYPETZFDD))
    {
      printf("<\"%s\">\"%s\",",Biddy_Managed_GetVariableName(MNG,Biddy_GetTag(BiddyE(edge))),Biddy_Managed_GetTopVariableName(MNG,BiddyE(edge)));
    } else {
      printf("\"%s\",",Biddy_Managed_GetTopVariableName(MNG,BiddyE(edge)));
    }
  }
  if (Biddy_IsNull(BiddyT(edge))) printf("NULL,");
  else {
    if (Biddy_GetMark(BiddyT(edge))) printf("*");
    printf("%p:",(void *) BiddyT(edge));
    if ((biddyManagerType == BIDDYTYPETZBDDC) || (biddyManagerType == BIDDYTYPETZBDD) ||
        (biddyManagerType == BIDDYTYPETZFDDC) || (biddyManagerType == BIDDYTYPETZFDD))
    {
      printf("<\"%s\">\"%s\",",Biddy_Managed_GetVariableName(MNG,Biddy_GetTag(BiddyT(edge))),Biddy_Managed_GetTopVariableName(MNG,BiddyT(edge)));
    } else {
      printf("\"%s\",",Biddy_Managed_GetTopVariableName(MNG,BiddyT(edge)));
    }
  }
  printf("@%u\n",BiddyN(edge)->expiry);
}

/*******************************************************************************
\brief Function debugORDERING.

### Description
### Side effects
### More info
*******************************************************************************/

static void
debugORDERING(Biddy_Manager MNG, BiddyOrderingTable table,
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

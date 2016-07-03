/***************************************************************************//*!
\file biddyInt.h
\brief File biddyInt.h contains declaration of internal data structures.

### Description

    PackageName [Biddy]
    Synopsis    [Biddy provides data structures and algorithms for the
                 representation and manipulation of Boolean functions with
                 ROBDDs. A hash table is used for quick search of nodes.
                 Complement edges decreases the number of nodes. An automatic
                 garbage collection with a formulae counter is implemented.
                 Variable swapping and sifting are implemented.]

    FileName    [biddyInt.h]
    Revision    [$Revision: 168 $]
    Date        [$Date: 2016-06-28 22:44:56 +0200 (tor, 28 jun 2016) $]
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

See also: biddy.h

*******************************************************************************/

#ifndef _BIDDYINT
#define _BIDDYINT

#include "biddy.h"
#include <assert.h>
#include <string.h>
#include <ctype.h>
#include <gmp.h>

/* sleep() is not included in modern version of MINGW */
/* #define sleep(sec) (Sleep ((sec) * 1000), 0) */

#ifdef MINGW
#define sleep(sec) 0
#endif

/* this is from */
/* http://stackoverflow.com/questions/3694723/error-c3861-strcasecmp-identifier-not-found-in-visual-studio-2008 */
/* not #if defined(_WIN32) || defined(_WIN64) because we have strncasecmp in mingw */
#ifdef _MSC_VER
#define strncasecmp _strnicmp
#define strcasecmp _stricmp
#endif

/* extended stats (YES or NO) */
#define BIDDYEXTENDEDSTATS_NO

/* event log (YES or NO) */
#define BIDDYEVENTLOG_NO
#ifdef BIDDYEVENTLOG_YES
#define ZF_LOGI(x) {static unsigned int biddystat = 0;printf(x);printf("#%u,%.2f\n",++biddystat,clock()/(1.0*CLOCKS_PER_SEC));}
#else
#define ZF_LOGI(x)
#endif


/*----------------------------------------------------------------------------*/
/* Constant declarations                                                      */
/*----------------------------------------------------------------------------*/

/* UINTPTR and UINTPTRSIZE are used for BiddyOrdering */
#define UINTPTR uintptr_t
#define UINTPTRSIZE (8*sizeof(UINTPTR))

/* Max number of variables - this is hardcoded for better performance. */
/* for optimal space reservation use VARMAX =  32*N */
/* variable "1" is one of these variables */
/* BIDDY IS NOT CAPABLE TO EFFICIENTLY WORK WITH TOO MANY VARIABLES! */
#define BIDDYVARMAX 2048

/*----------------------------------------------------------------------------*/
/* Macro definitions                                                          */
/*----------------------------------------------------------------------------*/

/* Null edge, this is hardcoded since Biddy v1.4 */
#define biddyNull ((BiddyNode *) NULL)

/* BiddyAddress returns the pointer used with the given edge */

#define BiddyAddress(f) (f)

/* BiddyT returns THEN successor, since Biddy v1.4 */

#define BiddyT(fun) (((BiddyNode *) Biddy_Regular(fun))->t)

/* BiddyE returns ELSE successor, since Biddy v1.4 */

#define BiddyE(fun) (((BiddyNode *) Biddy_Regular(fun))->f)

/* BiddyV returns top variable, since Biddy v1.6 */

#define BiddyV(fun) (((BiddyNode *) Biddy_Regular(fun))->v)

/* orderingtable[X,Y]==1 iff variabe X is smaller than variable Y */
/* IN THE BDD, SMALLER VARIABLES ARE ABOVE THE GREATER ONES */
/* GET_ORDER check if variabe X is smaller (topmore) than variable Y */
/* SET_ORDER set variabe X to be smaller (topmore) than variable Y */
/* CLEAR_ORDER set variabe X not to be smaller (topmore) than variable Y */
/* SET_ORDER and CLEAR_ORDER should be used simultaneously */
#define GET_ORDER(orderingtable,X,Y) ((orderingtable)[X][Y/UINTPTRSIZE]&(((UINTPTR) 1)<<(Y%UINTPTRSIZE)))!=0
#define SET_ORDER(orderingtable,X,Y) (orderingtable)[X][Y/UINTPTRSIZE] |= (((UINTPTR) 1)<<(Y%UINTPTRSIZE))
#define CLEAR_ORDER(orderingtable,X,Y) (orderingtable)[X][Y/UINTPTRSIZE] &= (~(((UINTPTR) 1)<<(Y%UINTPTRSIZE)))

/* The name of manager MNG, since Biddy v1.4. */
#define biddyManagerName ((Biddy_String)(MNG[0]))

/* Type of manager MNG, since Biddy v1.4. */
#define biddyManagerType ((Biddy_String)(MNG[1]))

/* Constant 0 in manager MNG, since Biddy v1.4. */
#define biddyZero ((Biddy_Edge)(MNG[2]))

/* Constant 1 in manager MNG, since Biddy v1.4. */
#define biddyOne ((Biddy_Edge)(MNG[3]))

/* Node table in manager MNG, since Biddy v1.4. */
/* this is typecasted to (BiddyNodeTable*) and dereferenced */
#define biddyNodeTable (*((BiddyNodeTable*)(MNG[4])))

/* Variable table in manager MNG, since Biddy v1.4. */
/* this is typecasted to (BiddyVariableTable*) and dereferenced */
#define biddyVariableTable (*((BiddyVariableTable*)(MNG[5])))

/* Formula table in manager MNG, since Biddy v1.4. */
/* this is typecasted to (BiddyFormulaTable*) and dereferenced */
#define biddyFormulaTable (*((BiddyFormulaTable*)(MNG[6])))

/* ITE Cache in manager MNG, since Biddy v1.4. */
/* this is typecasted to (BiddyOp3CacheTable*) and dereferenced */
#define biddyIteCache (*((BiddyOp3CacheTable*)(MNG[7])))

/* EA Cache in manager MNG, since Biddy v1.4. */
/* this is typecasted to (BiddyOp3CacheTable*) and dereferenced */
#define biddyEACache (*((BiddyOp3CacheTable*)(MNG[8])))

/* RC Cache in manager MNG, since Biddy v1.4. */
/* this is typecasted to (BiddyOp3CacheTable*) and dereferenced */
#define biddyRCCache (*((BiddyOp3CacheTable*)(MNG[9])))

/* Cache list in manager MNG, since Biddy v1.4. */
/* this is typecasted to (BiddyCacheList**) and dereferenced */
#define biddyCacheList (*((BiddyCacheList**)(MNG[10])))

/* List of free nodes in manager MNG, since Biddy v1.4. */
/* this is typecasted to (BiddyNode**) and dereferenced */
#define biddyFreeNodes (*((BiddyNode**)(MNG[11])))

/* Variable ordering in manager MNG, since Biddy v1.4. */
/* this is typecasted to (BiddyOrderingTable*) and dereferenced */
#define biddyOrderingTable (*((BiddyOrderingTable*)(MNG[12])))

/* Formulae counter in manager MNG, since Biddy v1.4. */
/* this is typecasted to (int*) and dereferenced */
#define biddyCount (*((unsigned int*)(MNG[13])))

/* Node selector in manager MNG, since Biddy v1.6 */
/* this is typecasted to (int*) and dereferenced */
#define biddySelect (*((unsigned short int*)(MNG[14])))

/* BiddyIsOK returns TRUE if the top node of given BDD is not obsolete, since Biddy v1.5 */
#define BiddyIsOK(f) (!(((BiddyNode *) Biddy_Regular(f))->count) || ((((BiddyNode *) Biddy_Regular(f))->count) >= biddyCount))

/* BiddyProlongOne prolonges top node of the given function, since Biddy v1.6 */
#define BiddyProlongOne(MNG,f,c) if((!(c))||(((BiddyNode*)Biddy_Regular(f))->count&&(((BiddyNode *)Biddy_Regular(f))->count<(c))))((BiddyNode*)Biddy_Regular(f))->count=(c)

/*----------------------------------------------------------------------------*/
/* Type declarations                                                          */
/*----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------*/
/* Structure declarations                                                     */
/*----------------------------------------------------------------------------*/

/* NODE TABLE (UNIQUE TABLE) = a fixed-size hash table with chaining */
/* TYPE Biddy_Variable IS DEFINED IN biddy.h */
/* IF Biddy_Variable IS unsigned short int (2B) THEN: */
/* THE SIZE OF BiddyNode on 32-bit systems is 28 Bytes */
/* THE SIZE OF BiddyNode on 64-bit systems is 48 Bytes */
typedef struct BiddyNode {
  struct BiddyNode *prev, *next; /* !!!MUST BE FIRST AND SECOND */
  Biddy_Edge f, t; /* f = else, t = then, !!!MUST BE THIRD AND FOURTH */
  void *list; /* list of nodes (various purposes) */
  unsigned int count; /* formulae counter */
  unsigned short int select; /* used to select node */
  Biddy_Variable v; /* index in variable table */
} BiddyNode;

typedef struct {
  BiddyNode **table;
  BiddyNode **blocktable; /* table of allocated memory blocks */
  unsigned int initsize; /* initial size of Node table */
  unsigned int size; /* current size of Node table */
  unsigned int limitsize; /* limit for the size of Node table */
  unsigned int blocknumber; /* number of allocated memory blocks */
  unsigned int initblocksize; /* the size of the first block of nodes */
  unsigned int blocksize; /* the size of the last block of nodes */
  unsigned int limitblocksize; /* limit for the size of the block of nodes */
  unsigned int generated; /* total number of generated nodes  */
  unsigned int max; /* maximal (peek) number of nodes in node table */
  unsigned int num; /* number of nodes currently in node table */
  unsigned int garbage; /* number of garbage collections */
  unsigned int swap; /* number of performed variable swapping */
  unsigned int sifting; /* number of performed dynamic reordering */
  unsigned int nodetableresize; /* number of performed node table resizing */
  unsigned int funite; /* number of calls of function Biddy_ITE */
  unsigned int funand; /* number of calls of function Biddy_And */
  clock_t gctime; /* total time spent for garbage collections */
  clock_t drtime; /* total time spent for dynamic reordering */
  float gcratio; /* do not start GC if the number of nodes is to low */
  float resizeratio; /* resize Node table if there are enough nodes */
  float newblockratio; /* create newblock of nodes if there are not enough free nodes */
  float siftingtreshold; /* stop sifting if the size of the system grows to much */
  float fsiftingtreshold; /* stop sifting if the size of the function grows to much */
  float convergesiftingtreshold;  /* stop one step of converging sifting if the size of the system grows to much */
  float fconvergesiftingtreshold; /* stop one step of converging sifting if the size of the function grows to much */

#ifdef BIDDYEXTENDEDSTATS_YES
  unsigned long long int iterecursive; /* number of calls to BiddyITE (direct and recursive) */
  unsigned long long int andrecursive; /*  number of calls to BiddyAnd (direct and recursive) */
  unsigned long long int foa; /* number of calls to Biddy_FoaNode */
  unsigned long long int find; /* number of calls to findNodeTable */
  unsigned long long int compare; /* num of compared nodes in findNodeTable */
  unsigned long long int add; /* number of calls to addNodeTable (node table insertions) */
#endif
} BiddyNodeTable;

/* VARIABLE TABLE (SYMBOL TREE) = dynamicaly allocated table */
/* variable "1" has id == 0 */
/* size of variable table must be compatible with Biddy_Variable */
/* value is used for evaluation and also for renaming variables */
/* counting variables is also used in BiddyCreateLocalInfo */
/* lastNewNode->list IS NOT DEFINED! */
/* YOU MUST NEVER ASSUME THAT lastNewNode->list = NULL */
typedef struct {
  Biddy_String name; /* name of variable */
  unsigned int num; /* number of nodes with this variable */
  unsigned int numone; /* used to count number of nodes with this variable in one function */
  unsigned int numobsolete; /* used to count number of obsolete nodes with this variable */
  BiddyNode *firstNewNode;
  BiddyNode *lastNewNode;
  Biddy_Edge variable; /* bdd representing a single positive variable */
  Biddy_Edge value; /* value: biddy_zero = 0, biddy_one = 1. etc. */
  Biddy_Boolean selected; /* used to count variables */
} BiddyVariable;

typedef struct {
  Biddy_String name; /* name of variable */
  Biddy_Variable v; /* variable corresponding to the name */
} BiddyLookupVariable;

typedef struct {
  BiddyVariable *table;
  BiddyLookupVariable *lookup;
  Biddy_Variable size; /* size = Biddy_VARMAX */
  Biddy_Variable num; /* number of all variables, inc. 1 */
  Biddy_Variable numnum; /* number of numbered variables, inc. 1 */
} BiddyVariableTable;

/* ORDERING TABLE = TWO-DIMENSIONAL MATRIX OF BITS */
/* ordering table is used to define variable ordering. */
/* constants 0 and 1 have max order */
/* USABLE ONLY AS AN IRREFLEXIVE TRANSITIVE RELATION */
/* MATRIX'S DIMENSION IN BITS = (VARMAX) x (VARMAX) */
/* orderingtable[X,Y]==1 iff variabe X is smaller than variable Y */
/* IN THE BDD, SMALLER VARIABLES ARE ABOVE THE GREATER ONES */
typedef UINTPTR BiddyOrderingTable[BIDDYVARMAX][1+(BIDDYVARMAX-1)/UINTPTRSIZE];

/* FORMULA TABLE (FORMULAE TREE) = dynamicaly allocated table */
/* count = 0, deleted = FALSE -> permanently preserved formula */
/* count = 0, deleted = TRUE -> deleted permanently preserved formula */
/* count >= biddyCount, deleted = FALSE -> preserved formula */
/* count >= biddyCount, deleted = TRUE -> deleted depreserved formula */
typedef struct {
  Biddy_Edge f;
  Biddy_String name;
  unsigned int count;
  Biddy_Boolean deleted;
} BiddyFormula;

typedef struct {
  BiddyFormula *table;
  unsigned int size;
  unsigned int numOrdered; /* formulae with name are ordered */
  Biddy_String deletedName; /* used to report deleted formulae */
} BiddyFormulaTable;

/* CACHE LIST = unidirectional list */
typedef struct BiddyCacheList {
  struct BiddyCacheList *next;
  Biddy_GCFunction gc;
} BiddyCacheList;

/* OP1 Cache = a fixed-size cache table for one-argument operations */
typedef struct {
  Biddy_Edge f;
  Biddy_Edge result; /* biddy_null = not valid record! */
} BiddyOp1Cache;

typedef struct {
  BiddyOp1Cache *table;
  unsigned int size;
  unsigned long long int *search;
  unsigned long long int *find;
  unsigned long long int *insert;
  unsigned long long int *overwrite;
} BiddyOp1CacheTable;

/* OP2 Cache = a fixed-size cache table for two-arguments operations */
typedef struct {
  Biddy_Edge f,g;
  Biddy_Edge result; /* biddy_null = not valid record! */
} BiddyOp2Cache;

typedef struct {
  BiddyOp2Cache *table;
  unsigned int size;
  unsigned long long int *search;
  unsigned long long int *find;
  unsigned long long int *insert;
  unsigned long long int *overwrite;
} BiddyOp2CacheTable;

/* OP3 Cache = a fixed-size cache table for three-arguments operations */
typedef struct {
  Biddy_Edge f,g,h;
  Biddy_Edge result; /* biddy_null = not valid record! */
} BiddyOp3Cache;

typedef struct {
  BiddyOp3Cache *table;
  unsigned int size;
  unsigned long long int *search;
  unsigned long long int *find;
  unsigned long long int *insert;
  unsigned long long int *overwrite;
} BiddyOp3CacheTable;

/* EA Cache = a fixed-size cache table intended for Biddy_E and Biddy_A */
/* Since Biddy v1.5, EA Cache is BiddyOp3Cache where variable is represented by h */

/* RC Cache = a fixed-size cache table intended for Biddy_Restrict and Biddy_Compose */
/* Since Biddy v1.5, RC Cache is BiddyOp3Cache where variable is represented by h */

/* Manager = configuration of a BDD system, since Biddy v1.4 */
/* All fields in BiddyManager must be pointers or arrays */
/* Anonymous manager (i.e. anonymous namespace) is created by Biddy_Init */
/* BDD systems managed by different managers are completely independent */
/* (different node table, different caches, different formulae counter etc.) */
/* User can create its own manager and use it instead of anonymous manager */
/* All managers are of type void** but internally they have the structure */
/* given here (see function Biddy_Init for details). */
/* NOTE: names of elements given here are not used in the program! */
typedef struct {
  void *name; /* this is typecasted to Biddy_String */
  void *type; /* this is typecasted to Biddy_String */
  void *constantZero; /* this is typecasted to Biddy_Edge */
  void *constantOne; /* this is typecasted to Biddy_Edge */
  void *nodeTable; /* this is typecasted to (BiddyNodeTable*) */
  void *variableTable; /* this is typecasted to (BiddyVariableTable*) */
  void *formulaTable; /* this is typecasted to (BiddyFormulaTable*) */
  void *ITEcache; /* this is typecasted to (BiddyOp3CacheTable*) */
  void *EAcache; /* this is typecasted to (BiddyOp3CacheTable*) */
  void *RCcache; /* this is typecasted to (BiddyOp3CacheTable*) */
  void *cacheList; /* this is typecasted to (BiddyCacheList*) */
  void *freeNodes; /* this is typecasted to (BiddyNode*) */
  void *ordering; /* this is typecasted to (BiddyOrderingTable*) */
  void *counter; /* this is typecasted to (unsigned int*) */
  void *selector; /* this is typecasted to (unsigned short int*) */
} BiddyManager;

/* LocalInfo = a table for additional info about nodes, since Biddy v1.4 */
/* Created only if needed and deleted before normal operations */
/* It reuses pointer which is normally used for garbage collection! */
/* Only one element of the union can be used! */
typedef struct {
  BiddyNode *back; /* reference to node */
  BiddyNode *org; /* original pointer, used when deleting local cache */
  union {
    unsigned int enumerator; /* used in enumerateNodes */
    unsigned int npSelected; /* used in nodePlainNumber() */
    /* NOTE: positiveSelected iff (data.npSelected & 1 != 0) */
    /* NOTE: negativeSelected iff (data.npSelected & 2 != 0) */
    mpz_t mintermCount; /* used in mintermCount() */
    /* NOTE: mintermCount is a number represented by using GMP library */
  } data;
} BiddyLocalInfo;

/*----------------------------------------------------------------------------*/
/* Variable declarations                                                      */
/*----------------------------------------------------------------------------*/

extern Biddy_Manager biddyAnonymousManager; /* anonymous manager */

extern BiddyLocalInfo *biddyLocalInfo; /* reference to Local Info */

/*----------------------------------------------------------------------------*/
/* Prototypes for internal functions defined in biddyMain.c                   */
/*----------------------------------------------------------------------------*/

void BiddyIncCounter(Biddy_Manager MNG);

void BiddyDecCounter(Biddy_Manager MNG);

extern void BiddyProlongRecursively(Biddy_Manager MNG, Biddy_Edge f, unsigned int count, Biddy_Variable v);

extern Biddy_Variable BiddyCreateLocalInfo(Biddy_Manager MNG, Biddy_Edge f);

extern void BiddyDeleteLocalInfo(Biddy_Manager MNG, Biddy_Edge f);

extern unsigned int BiddyGetSeqByNode(Biddy_Edge root, Biddy_Edge f);

extern Biddy_Edge BiddyGetNodeBySeq(Biddy_Edge root, unsigned int n);

extern void BiddySetEnumerator(Biddy_Edge f, unsigned int n);

extern unsigned int BiddyGetEnumerator(Biddy_Edge f);

extern void BiddySetMintermCount(Biddy_Edge f, mpz_t value);

extern void BiddyGetMintermCount(Biddy_Edge f, mpz_t result);

extern void BiddySetNodePlainSelected(Biddy_Edge f);

extern Biddy_Boolean BiddyGetNodePlainSelected(Biddy_Edge f);

extern void BiddySystemReport(Biddy_Manager MNG);

extern void BiddyFunctionReport(Biddy_Manager MNG, Biddy_Edge f);

/*----------------------------------------------------------------------------*/
/* Prototypes for internal functions defined in biddyStat.c                   */
/*----------------------------------------------------------------------------*/

extern void BiddyNodeNumber(Biddy_Manager MNG, Biddy_Edge f, unsigned int *n);

extern void BiddyNodeVarNumber(Biddy_Manager MNG, Biddy_Edge f, unsigned int *n);

/*----------------------------------------------------------------------------*/
/* Prototypes for internal functions defined in biddyInOut.c                  */
/*----------------------------------------------------------------------------*/

#endif  /* _BIDDYINT */

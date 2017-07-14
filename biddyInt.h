/***************************************************************************//*!
\file biddyInt.h
\brief File biddyInt.h contains declaration of internal data structures.

### Description

    PackageName [Biddy]
    Synopsis    [Biddy provides data structures and algorithms for the
                 representation and manipulation of Boolean functions with
                 ROBDDs. A hash table is used for quick search of nodes.
                 Complement edges decreases the number of nodes. An automatic
                 garbage collection with a system age is implemented.
                 Variable swapping and sifting are implemented.]

    FileName    [biddyInt.h]
    Revision    [$Revision: 284 $]
    Date        [$Date: 2017-07-11 23:55:56 +0200 (tor, 11 jul 2017) $]
    Authors     [Robert Meolic (robert.meolic@um.si),
                 Ales Casar (ales@homemade.net)]

### Copyright

Copyright (C) 2006, 2017 UM-FERI, Smetanova ulica 17, SI-2000 Maribor, Slovenia

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
/* if YES then you have to use -lm for linking */
#define BIDDYEXTENDEDSTATS_NO

/* event log (YES or NO) */
#define BIDDYEVENTLOG_NO
#ifdef BIDDYEVENTLOG_YES
#define ZF_LOGI(x) {static unsigned int biddystat = 0;printf(x);printf("#%u,%.2f\n",++biddystat,clock()/(1.0*CLOCKS_PER_SEC));}
#else
#define ZF_LOGI(x)
#endif

/* use this to use CUDD style for edges: */
/* solid line: regular THEN arcs */
/* bold line: complemented THEN arcs */
/* dashed line: regular ELSE arcs */
/* dotted line: complemented ELSE arcs */
/* without LEGACY_DOT, an experimental style for edges is used */
/* this setting is NOT USED for CUDD-like generation of dot files */
#define LEGACY_DOT

/*----------------------------------------------------------------------------*/
/* Constant declarations                                                      */
/*----------------------------------------------------------------------------*/

/* UINTPTR and UINTPTRSIZE are used for BiddyOrdering */
#define UINTPTR uintptr_t
#define UINTPTRSIZE (8*sizeof(UINTPTR))

/* Max number of variables - this is hardcoded for better performance. */
/* for optimal space reservation use VARMAX =  32*N */
/* variable "1" is one of these variables */
/* For TZBDDs and TZFDDs, the limit is 65536 variables on 64-bit architecture */
/* If there are more than 32768 variables then some macros in biddy.h must be changed */
/* BIDDY IS NOT EFFICIENT WITH MANY VARIABLES! */
#define BIDDYVARMAX 2048

/*----------------------------------------------------------------------------*/
/* Macro definitions                                                          */
/*----------------------------------------------------------------------------*/

/* Null edge, this is hardcoded since Biddy v1.4 */
#define biddyNull ((Biddy_Edge)NULL)

/* BiddyP returns the pointer used with the given edge (without tag and complement bit), since Biddy v1.7. */
#if UINTPTR_MAX == 0xffffffffffffffff
#define BiddyP(fun) ((void *)((uintptr_t) fun & 0x0000fffffffffffe))
#else
#define BiddyP(fun) ((void *)((uintptr_t) fun & ~((uintptr_t) 1)))
#endif

/* BiddyR returns not-tagged version of edge, since Biddy v1.7. */
/* tagged edges assumes 64-bit architecture */
#if UINTPTR_MAX == 0xffffffffffffffff
#define BiddyR(fun) ((void *)((uintptr_t) fun & 0x0000ffffffffffff))
#else
#define BiddyR(fun) (fun)
#endif

/* BiddyN returns the node pointed by the given edge, since Biddy v1.7. */
#if UINTPTR_MAX == 0xffffffffffffffff
#define BiddyN(fun) ((BiddyNode*)((uintptr_t) fun & 0x0000fffffffffffe))
#else
#define BiddyN(fun) ((BiddyNode*)((uintptr_t) fun & ~((uintptr_t) 1)))
#endif

/* BiddyT returns THEN successor, since Biddy v1.4 */

#define BiddyT(fun) (BiddyN(fun)->t)

/* BiddyE returns ELSE successor, since Biddy v1.4 */

#define BiddyE(fun) (BiddyN(fun)->f)

/* BiddyV returns top variable, since Biddy v1.6 */

#define BiddyV(fun) (BiddyN(fun)->v)

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
#define biddyManagerName1 ((Biddy_String)(MNG1[0]))
#define biddyManagerName2 ((Biddy_String)(MNG2[0]))

/* Type of manager MNG, since Biddy v1.7 this is used for GDD type */
#define biddyManagerType (*((short int*)(MNG[1])))
#define biddyManagerType1 (*((short int*)(MNG1[1])))
#define biddyManagerType2 (*((short int*)(MNG2[1])))

/* Terminal node in manager MNG, since Biddy v1.7. */
#define biddyTerminal ((void *)(MNG[2]))
#define biddyTerminal1 ((void *)(MNG1[2]))
#define biddyTerminal2 ((void *)(MNG2[2]))

/* Constant 0 in manager MNG, since Biddy v1.4. */
#define biddyZero ((Biddy_Edge)(MNG[3]))
#define biddyZero1 ((Biddy_Edge)(MNG1[3]))
#define biddyZero2 ((Biddy_Edge)(MNG2[3]))

/* Constant 1 in manager MNG, since Biddy v1.4. */
#define biddyOne ((Biddy_Edge)(MNG[4]))
#define biddyOne1 ((Biddy_Edge)(MNG1[4]))
#define biddyOne2 ((Biddy_Edge)(MNG2[4]))

/* Node table in manager MNG, since Biddy v1.4. */
/* this is typecasted to (BiddyNodeTable*) and dereferenced */
/* gdb -g: p *((BiddyNodeTable*)(MNG[5])) */
/* gdb -g: p ((BiddyNodeTable*)(MNG[5])).table[i] */
/* gdb -g3: p biddyNodeTable */
/* gdb -g3: p biddyNodeTable.table[i] */
#define biddyNodeTable (*((BiddyNodeTable*)(MNG[5])))
#define biddyNodeTable1 (*((BiddyNodeTable*)(MNG1[5])))
#define biddyNodeTable2 (*((BiddyNodeTable*)(MNG2[5])))

/* Variable table in manager MNG, since Biddy v1.4. */
/* this is typecasted to (BiddyVariableTable*) and dereferenced */
/* gdb -g: p *((BiddyVariableTable*)(MNG[6])) */
/* gdb -g: p ((BiddyVariableTable*)(MNG[6])).table[i] */
/* gdb -g3: p biddyVariableTable */
/* gdb -g3: p biddyVariableTable.table[i] */
#define biddyVariableTable (*((BiddyVariableTable*)(MNG[6])))
#define biddyVariableTable1 (*((BiddyVariableTable*)(MNG1[6])))
#define biddyVariableTable2 (*((BiddyVariableTable*)(MNG2[6])))

/* Formula table in manager MNG, since Biddy v1.4. */
/* this is typecasted to (BiddyFormulaTable*) and dereferenced */
#define biddyFormulaTable (*((BiddyFormulaTable*)(MNG[7])))
#define biddyFormulaTable1 (*((BiddyFormulaTable*)(MNG1[7])))
#define biddyFormulaTable2 (*((BiddyFormulaTable*)(MNG2[7])))

/* OP Cache in manager MNG, since Biddy v1.4. */
/* this is typecasted to (BiddyOp3CacheTable*) and dereferenced */
#define biddyOPCache (*((BiddyOp3CacheTable*)(MNG[8])))
#define biddyOPCache1 (*((BiddyOp3CacheTable*)(MNG1[8])))
#define biddyOPCache2 (*((BiddyOp3CacheTable*)(MNG2[8])))

/* EA Cache in manager MNG, since Biddy v1.4. */
/* this is typecasted to (BiddyOp3CacheTable*) and dereferenced */
#define biddyEACache (*((BiddyOp3CacheTable*)(MNG[9])))
#define biddyEACache1 (*((BiddyOp3CacheTable*)(MNG1[9])))
#define biddyEACache2 (*((BiddyOp3CacheTable*)(MNG2[9])))

/* RC Cache in manager MNG, since Biddy v1.4. */
/* this is typecasted to (BiddyOp3CacheTable*) and dereferenced */
#define biddyRCCache (*((BiddyOp3CacheTable*)(MNG[10])))
#define biddyRCCache1 (*((BiddyOp3CacheTable*)(MNG1[10])))
#define biddyRCCache2 (*((BiddyOp3CacheTable*)(MNG2[10])))

/* Cache list in manager MNG, since Biddy v1.4. */
/* this is typecasted to (BiddyCacheList**) and dereferenced */
#define biddyCacheList (*((BiddyCacheList**)(MNG[11])))
#define biddyCacheList1 (*((BiddyCacheList**)(MNG1[11])))
#define biddyCacheList2 (*((BiddyCacheList**)(MNG2[11])))

/* List of free nodes in manager MNG, since Biddy v1.4. */
/* this is typecasted to (BiddyNode**) and dereferenced */
#define biddyFreeNodes (*((BiddyNode**)(MNG[12])))
#define biddyFreeNodes1 (*((BiddyNode**)(MNG1[12])))
#define biddyFreeNodes2 (*((BiddyNode**)(MNG2[12])))

/* Variable ordering in manager MNG, since Biddy v1.4. */
/* this is typecasted to (BiddyOrderingTable*) and dereferenced */
#define biddyOrderingTable (*((BiddyOrderingTable*)(MNG[13])))
#define biddyOrderingTable1 (*((BiddyOrderingTable*)(MNG1[13])))
#define biddyOrderingTable2 (*((BiddyOrderingTable*)(MNG2[13])))

/* System age in manager MNG, since Biddy v1.4. */
/* this is typecasted to (int*) and dereferenced */
#define biddySystemAge (*((unsigned int*)(MNG[14])))
#define biddySystemAge1 (*((unsigned int*)(MNG1[14])))
#define biddySystemAge2 (*((unsigned int*)(MNG2[14])))

/* Node selector in manager MNG, since Biddy v1.6 */
/* this is typecasted to (int*) and dereferenced */
#define biddySelect (*((unsigned short int*)(MNG[15])))
#define biddySelect1 (*((unsigned short int*)(MNG1[15])))
#define biddySelect2 (*((unsigned short int*)(MNG2[15])))

/* BiddyProlongOne prolonges top node of the given function, since Biddy v1.6 */
#define BiddyProlongOne(f,c) if((!(c))||(BiddyN(f)->expiry&&(BiddyN(f)->expiry<(c))))BiddyN(f)->expiry=(c)

/* BiddyRefresh prolonges top node of the given function, manager MNG is assumed, since Biddy v1.7 */
#define BiddyRefresh(f) if(BiddyN(f)->expiry&&(BiddyN(f)->expiry<(biddySystemAge)))BiddyN(f)->expiry=(biddySystemAge)
#define BiddyDefresh(f) if(BiddyN(f)->expiry&&(BiddyN(f)->expiry>(biddySystemAge)))BiddyN(f)->expiry=(biddySystemAge)

/* BiddyIsSmaller returns TRUE if the first variable is smaller (= lower = previous = above = topmore), manager MNG is assumed, since Biddy v1.7 */
#define BiddyIsSmaller(fv,gv) GET_ORDER(biddyOrderingTable,fv,gv)

/* BiddyIsOK is intended for debugging, only, manager MNG is assumed, since Biddy v1.7 */
#define BiddyIsOK(f) (!(BiddyN(f)->expiry) || ((BiddyN(f)->expiry) >= biddySystemAge))
/* #define BiddyIsOK(f) ((!(BiddyN(f)->expiry) || ((BiddyN(f)->expiry) >= biddySystemAge)) && checkFunctionOrdering(MNG,f)) */

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
  Biddy_Edge f, t; /* f = left = else, t = right = then, !!!MUST BE THIRD AND FOURTH */
  void *list; /* list of nodes (various purposes) */
  unsigned int expiry; /* expiry value */
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
  unsigned int funandor; /* number of calls of function Biddy_And and Biddy_Or */
  unsigned int funxor; /* number of calls of function Xor */
  clock_t gctime; /* total time spent for garbage collections */
  clock_t drtime; /* total time spent for dynamic reordering */
  float gcratio; /* do not delete nodes if the effect is to small */
  float gcratioF; /* do not delete nodes if the effect is to small */
  float gcratioX; /* do not delete nodes if the effect is to small */
  float resizeratio; /* resize Node table if there are to many nodes */
  float resizeratioF; /* resize Node table if there are to many nodes */
  float resizeratioX; /* resize Node table if there are to many nodes */
  float siftingtreshold; /* stop sifting if the size of the system grows to much */
  float fsiftingtreshold; /* stop sifting if the size of the function grows to much */
  float convergesiftingtreshold;  /* stop one step of converging sifting if the size of the system grows to much */
  float fconvergesiftingtreshold; /* stop one step of converging sifting if the size of the function grows to much */

#ifdef BIDDYEXTENDEDSTATS_YES
  unsigned long long int foa; /* number of calls to Biddy_FoaNode */
  unsigned long long int find; /* number of calls to findNodeTable */
  unsigned long long int compare; /* num of compared nodes in findNodeTable */
  unsigned long long int add; /* number of calls to addNodeTable (node table insertions) */
  unsigned long long int iterecursive; /* number of calls to BiddyITE (direct and recursive) */
  unsigned long long int andorrecursive; /*  number of calls to BiddyAnd and BiddyOr (direct and recursive) */
  unsigned long long int xorrecursive; /*  number of calls to BiddyXor (direct and recursive) */
  unsigned long long int *gcobsolete; /* number of obsolete nodes removed by GC */
#endif
} BiddyNodeTable;

/* VARIABLE TABLE (SYMBOL TREE) = dynamicaly allocated table */
/* variable "1" has id == 0 */
/* size of variable table must be compatible with Biddy_Variable */
/* value is used for evaluation and also for renaming variables */
/* counting variables is also used in BiddyCreateLocalInfo */
/* lastNode->list IS NOT DEFINED! */
/* YOU MUST NEVER ASSUME THAT lastNode->list = NULL */
typedef struct {
  Biddy_String name; /* name of variable */
  unsigned int num; /* number of nodes with this variable */
  unsigned int numone; /* used to count number of nodes with this variable in one function */
  unsigned int numobsolete; /* used to count number of obsolete nodes with this variable */
  BiddyNode *firstNode;
  BiddyNode *lastNode;
  Biddy_Variable prev; /* previous variable in the global ordering (lower, topmore) */
  Biddy_Variable next; /* next variable in the global ordering (higher, bottommore) */
  Biddy_Edge variable; /* bdd representing a single positive variable */
  Biddy_Edge element; /* bdd representing a set with a single element, i.e. {{x}} */
  Biddy_Edge value; /* value: biddyZero = 0, biddyOne = 1, reused in some algorithms */
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
/* expiry = 0, deleted = FALSE -> permanently preserved formula */
/* expiry = 0, deleted = TRUE -> deleted permanently preserved formula */
/* expiry >= biddySystemAge, deleted = FALSE -> preserved formula */
/* expiry >= biddySystemAge, deleted = TRUE -> deleted depreserved formula */
typedef struct {
  Biddy_Edge f;
  Biddy_String name;
  unsigned int expiry;
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
  Biddy_Boolean disabled;
  unsigned long long int *search;
  unsigned long long int *find;
#ifdef BIDDYEXTENDEDSTATS_YES
  unsigned long long int *insert;
  unsigned long long int *overwrite;
#endif
} BiddyOp1CacheTable;

/* OP2 Cache = a fixed-size cache table for two-arguments operations */
typedef struct {
  Biddy_Edge f,g;
  Biddy_Edge result; /* biddy_null = not valid record! */
} BiddyOp2Cache;

typedef struct {
  BiddyOp2Cache *table;
  unsigned int size;
  Biddy_Boolean disabled;
  unsigned long long int *search;
  unsigned long long int *find;
#ifdef BIDDYEXTENDEDSTATS_YES
  unsigned long long int *insert;
  unsigned long long int *overwrite;
#endif
} BiddyOp2CacheTable;

/* OP3 Cache = a fixed-size cache table for three-arguments operations */
typedef struct {
  Biddy_Edge f,g,h;
  Biddy_Edge result; /* biddy_null = not valid record! */
} BiddyOp3Cache;

typedef struct {
  BiddyOp3Cache *table;
  unsigned int size;
  Biddy_Boolean disabled;
  unsigned long long int *search;
  unsigned long long int *find;
#ifdef BIDDYEXTENDEDSTATS_YES
  unsigned long long int *insert;
  unsigned long long int *overwrite;
#endif
} BiddyOp3CacheTable;

/* EA Cache = a fixed-size cache table intended for Biddy_E and Biddy_A */
/* Since Biddy v1.5, EA Cache is BiddyOp3Cache where variable is represented by h */

/* RC Cache = a fixed-size cache table intended for Biddy_Restrict and Biddy_Compose */
/* Since Biddy v1.5, RC Cache is BiddyOp3Cache where variable is represented by h */

/* Manager = configuration of a BDD system, since Biddy v1.4 */
/* All fields in BiddyManager must be pointers or arrays */
/* Anonymous manager (i.e. anonymous namespace) is created by Biddy_Init */
/* BDD systems managed by different managers are completely independent */
/* (different node table, different caches, different system age etc.) */
/* User can create its own manager and use it instead of anonymous manager */
/* All managers are of type void** but internally they have the structure */
/* given here (see function Biddy_Init for details). */
/* NOTE: names of elements given here are not used in the program! */
typedef struct {
  void *name; /* this is typecasted to Biddy_String */
  void *type; /* this is typecasted to (short int*) */
  void *terminalEdge; /* this is typecasted to (void *) */
  void *constantZero; /* this is typecasted to Biddy_Edge */
  void *constantOne; /* this is typecasted to Biddy_Edge */
  void *nodeTable; /* this is typecasted to (BiddyNodeTable*) */
  void *variableTable; /* this is typecasted to (BiddyVariableTable*) */
  void *formulaTable; /* this is typecasted to (BiddyFormulaTable*) */
  void *OPcache; /* this is typecasted to (BiddyOp3CacheTable*) */
  void *EAcache; /* this is typecasted to (BiddyOp3CacheTable*) */
  void *RCcache; /* this is typecasted to (BiddyOp3CacheTable*) */
  void *cacheList; /* this is typecasted to (BiddyCacheList*) */
  void *freeNodes; /* this is typecasted to (BiddyNode*) */
  void *ordering; /* this is typecasted to (BiddyOrderingTable*) */
  void *age; /* this is typecasted to (unsigned int*) */
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
    unsigned int npSelected; /* used in nodePlainNumber() and pathCount() */
    /* NOTE: positiveSelected iff (data.npSelected & 1 != 0) */
    /* NOTE: negativeSelected iff (data.npSelected & 2 != 0) */
    unsigned int nodeCount; /* used in nodePlainNumber() */
    unsigned long long int path1Count; /* used in pathCount() */
    mpz_t mintermCount; /* used in mintermCount(), represented using GMP library */
  } data;
  union {
    unsigned long long int path0Count; /* used in pathCount() */
    Biddy_Boolean leftmost; /* used in pathCount() */
  } datax;
} BiddyLocalInfo;

/*----------------------------------------------------------------------------*/
/* Variable declarations                                                      */
/*----------------------------------------------------------------------------*/

extern Biddy_Manager biddyAnonymousManager; /* anonymous manager */

extern BiddyLocalInfo *biddyLocalInfo; /* reference to Local Info */

/*----------------------------------------------------------------------------*/
/* Prototypes for internal functions defined in biddyMain.c                   */
/*----------------------------------------------------------------------------*/

#define BiddyManagedFoaNode(MNG,v,pf,pt,garbageAllowed) BiddyManagedTaggedFoaNode(MNG,v,pf,pt,v,garbageAllowed)
extern Biddy_Edge BiddyManagedTaggedFoaNode(Biddy_Manager MNG, Biddy_Variable v, Biddy_Edge pf, Biddy_Edge pt, Biddy_Variable ptag, Biddy_Boolean garbageAllowed);

extern Biddy_Edge BiddyManagedNot(const Biddy_Manager MNG, const Biddy_Edge f);
extern Biddy_Edge BiddyManagedITE(const Biddy_Manager MNG, Biddy_Edge f, Biddy_Edge g, Biddy_Edge h);
extern Biddy_Edge BiddyManagedAnd(const Biddy_Manager MNG, const Biddy_Edge f, const Biddy_Edge g);
extern Biddy_Edge BiddyManagedOr(const Biddy_Manager MNG, const Biddy_Edge f, const Biddy_Edge g);
extern Biddy_Edge BiddyManagedNand(const Biddy_Manager MNG, const Biddy_Edge f, const Biddy_Edge g);
extern Biddy_Edge BiddyManagedNor(const Biddy_Manager MNG, const Biddy_Edge f, const Biddy_Edge g);
extern Biddy_Edge BiddyManagedXor(const Biddy_Manager MNG, const Biddy_Edge f, const Biddy_Edge g);
extern Biddy_Edge BiddyManagedXnor(const Biddy_Manager MNG, const Biddy_Edge f, const Biddy_Edge g);
extern Biddy_Edge BiddyManagedLeq(const Biddy_Manager MNG, const Biddy_Edge f, const Biddy_Edge g);
extern Biddy_Edge BiddyManagedGt(const Biddy_Manager MNG, const Biddy_Edge f, const Biddy_Edge g);

extern Biddy_Edge BiddyManagedRestrict(Biddy_Manager MNG, Biddy_Edge f, Biddy_Variable v, Biddy_Boolean value);
extern Biddy_Edge BiddyManagedCompose(Biddy_Manager MNG, Biddy_Edge f, Biddy_Edge g, Biddy_Variable v);
extern Biddy_Edge BiddyManagedE(Biddy_Manager MNG, Biddy_Edge f, Biddy_Variable v);
extern Biddy_Edge BiddyManagedA(Biddy_Manager MNG, Biddy_Edge f, Biddy_Variable v);
extern Biddy_Edge BiddyManagedExistAbstract(Biddy_Manager MNG, Biddy_Edge f, Biddy_Edge cube);
extern Biddy_Edge BiddyManagedUnivAbstract(Biddy_Manager MNG, Biddy_Edge f, Biddy_Edge cube);
extern Biddy_Edge BiddyManagedAndAbstract(Biddy_Manager MNG, Biddy_Edge f, Biddy_Edge g, Biddy_Edge cube);
extern Biddy_Edge BiddyManagedConstrain(Biddy_Manager MNG, Biddy_Edge f, Biddy_Edge c);
extern Biddy_Edge BiddyManagedSimplify(Biddy_Manager MNG, Biddy_Edge f, Biddy_Edge c);
extern Biddy_Edge BiddyManagedSupport(Biddy_Manager MNG, Biddy_Edge f);
extern Biddy_Edge BiddyManagedReplace(Biddy_Manager MNG, Biddy_Edge f);
extern Biddy_Edge BiddyManagedChange(Biddy_Manager MNG, Biddy_Edge f, Biddy_Variable v);
extern Biddy_Edge BiddyManagedSubset(Biddy_Manager MNG, Biddy_Edge f, Biddy_Variable v, Biddy_Boolean value);

#define BiddyManagedUnion(MNG,f,g) BiddyManagedOr(MNG,f,g)
#define BiddyManagedIntersect(MNG,f,g) BiddyManagedAnd(MNG,f,g)
#define BiddyManagedDiff(MNG,f,g) BiddyManagedGt(MNG,g,f)

extern void BiddyIncSystemAge(Biddy_Manager MNG);
extern void BiddyDecSystemAge(Biddy_Manager MNG);
extern void BiddyProlongRecursively(Biddy_Manager MNG, Biddy_Edge f, unsigned int c);
extern Biddy_Variable BiddyCreateLocalInfo(Biddy_Manager MNG, Biddy_Edge f);
extern void BiddyDeleteLocalInfo(Biddy_Manager MNG, Biddy_Edge f);
extern unsigned int BiddyGetSeqByNode(Biddy_Edge root, Biddy_Edge f);
extern Biddy_Edge BiddyGetNodeBySeq(Biddy_Edge root, unsigned int n);
extern void BiddySetEnumerator(Biddy_Edge f, unsigned int n);
extern unsigned int BiddyGetEnumerator(Biddy_Edge f);
extern void BiddySetNodeCount(Biddy_Edge f, unsigned int value);
extern unsigned int BiddyGetNodeCount(Biddy_Edge f);
extern void BiddySetPath0Count(Biddy_Edge f, unsigned long long int value);
extern unsigned long long int BiddyGetPath0Count(Biddy_Edge f);
extern void BiddySetPath1Count(Biddy_Edge f, unsigned long long int value);
extern unsigned long long int BiddyGetPath1Count(Biddy_Edge f);
extern void BiddySetLeftmost(Biddy_Edge f, Biddy_Boolean value);
extern Biddy_Boolean BiddyGetLeftmost(Biddy_Edge f);
extern void BiddySetMintermCount(Biddy_Edge f, mpz_t value);
extern void BiddyGetMintermCount(Biddy_Edge f, mpz_t result);
extern void BiddySelectNP(Biddy_Edge f);
extern Biddy_Boolean BiddyIsSelectedNP(Biddy_Edge f);
extern Biddy_Edge BiddyCopy(Biddy_Manager MNG1, Biddy_Manager MNG2, Biddy_Edge f);
extern Biddy_Edge BiddyCopyOBDD(Biddy_Manager MNG1, Biddy_Manager MNG2, Biddy_Edge f);
extern Biddy_Edge BiddyCopyZBDD(Biddy_Manager MNG1, Biddy_Manager MNG2, Biddy_Edge f);
extern Biddy_Edge BiddyCopyTZBDD(Biddy_Manager MNG1, Biddy_Manager MNG2, Biddy_Edge f);
extern Biddy_Edge BiddyConvertDirect(Biddy_Manager MNG1, Biddy_Manager MNG2, Biddy_Edge f);

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

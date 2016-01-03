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
    Revision    [$Revision: 124 $]
    Date        [$Date: 2015-12-30 17:27:05 +0100 (sre, 30 dec 2015) $]
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

See also: biddy.h

*******************************************************************************/

#ifndef _BIDDYINT
#define _BIDDYINT

#include "biddy.h"

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

/*----------------------------------------------------------------------------*/
/* Constant declarations                                                      */
/*----------------------------------------------------------------------------*/

/* UINTPTR and UINTPTRSIZE are used for BiddyOrdering */
#define UINTPTR uintptr_t
#define UINTPTRSIZE (8*sizeof(UINTPTR))

/* Max number of variables - this is hardcoded for better performance. */
/* for optimal space reservation use VARMAX =  32*N */
/* this must be compatible with Biddy_Variable (<65536) */
/* variable "1" is one of these variables */
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

/* The size of memory block in manager MNG, since Biddy v1.4. */
/* this is typecasted to (unsigned int*) and dereferenced */
#define biddyBlockSize (*((unsigned int*)(MNG[11])))

/* List of free nodes in manager MNG, since Biddy v1.4. */
/* this is typecasted to (BiddyNode**) and dereferenced */
#define biddyFreeNodes (*((BiddyNode**)(MNG[12])))

/* Variable ordering in manager MNG, since Biddy v1.4. */
/* this is typecasted to (BiddyOrderingTable*) and dereferenced */
#define biddyOrderingTable (*((BiddyOrderingTable*)(MNG[13])))

/* Formulae counter in manager MNG, since Biddy v1.4. */
/* this is typecasted to (int*) and dereferenced */
#define biddyCount (*((int*)(MNG[14])))

/* BiddyIsOK returns TRUE if the top node of given BDD is not obsolete, since Biddy v1.5 */
#define BiddyIsOK(f) (!(((BiddyNode *) Biddy_Regular(f))->count) || ((((BiddyNode *) Biddy_Regular(f))->count) >= biddyCount))

/*----------------------------------------------------------------------------*/
/* Type declarations                                                          */
/*----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------*/
/* Structure declarations                                                     */
/*----------------------------------------------------------------------------*/

/* NODE TABLE (UNIQUE TABLE) = a fixed-size hash table with chaining */
/* TYPE Biddy_Variable IS DEFINED IN biddy.h */
/* IF Biddy_Variable IS unsigned int (4B) THEN: */
/* THE SIZE OF BiddyNode on 32-bit systems is 28 Bytes */
/* THE SIZE OF BiddyNode on 64-bit systems is 48 Bytes */
typedef struct BiddyNode {
  struct BiddyNode *prev, *next; /* !!!MUST BE FIRST AND SECOND */
  Biddy_Edge f, t; /* f = else, t = then, !!!MUST BE THIRD AND FOURTH */
  void *list; /* list of nodes (various purposes) */
  int count; /* formulae counter, !!!MUST BE SIGNED */
  Biddy_Variable v; /* index in variable table */
} BiddyNode;

typedef struct {
  BiddyNode **table;
  unsigned int size; /* size of node table */
  BiddyNode **blocktable; /* table of allocated memory blocks */
  unsigned long long int foa; /* number of calls to Biddy_FoaNode */
  unsigned long long int compare; /* num of cmp made by Biddy_FoaNode */
  unsigned long long int add; /* number of adds made by Biddy_FoaNode */
  unsigned int max; /* maximal number of nodes in node table */
  unsigned int num; /* number of nodes currently in node table */
  unsigned int numf; /* number of fortified nodes */
  unsigned int garbage; /* number of garbage collections */
  unsigned int generated; /* number of free nodes generated */
  unsigned int blocknumber; /* number of allocated memory blocks */
  unsigned int swap; /* number of allocated memory blocks */
  unsigned int sifting; /* number of allocated memory blocks */
} BiddyNodeTable;

/* VARIABLE TABLE (SYMBOL TREE) = dynamicaly allocated table */
/* variable "1" has id == 0 */
/* size of variable table must be compatible with Biddy_Variable */
/* value is used for evaluation and also for renaming variables */
/* counting variables is also used in BiddyCreateLocalInfo */
typedef struct {
  Biddy_String name; /* name of variable */
  BiddyNode *firstNewNode;
  BiddyNode *lastNewNode;
  BiddyNode *freshNodes;
  BiddyNode *fortifiedNodes;
  unsigned int num; /* number of nodes with this variable */
  Biddy_Edge terminal; /* bdd representing a single positive variable */
  Biddy_Edge value; /* value: biddy_zero = 0, biddy_one = 1. etc. */
  Biddy_Boolean selected; /* used to count variables */
} BiddyVariable;

typedef struct {
  BiddyVariable *table;
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
  int count;
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
  void *blocksize; /* this is typecasted to (unsigned int*) */
  void *freeNodes; /* this is typecasted to (BiddyNode*) */
  void *ordering; /* this is typecasted to (BiddyOrderingTable*) */
  void *counter; /* this is typecasted to (int*) */
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
    /* positiveSelected iff (data.npSelected & 1 != 0) */
    /* negativeSelected iff (data.npSelected & 2 != 0) */
    mpz_t mintermCount; /* used in mintermCount() */
    /* mintermCount is a number represented by using GMP library */
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

extern void BiddyRefresh(Biddy_Manager MNG, Biddy_Edge f);

extern void BiddyProlong(Biddy_Manager MNG, Biddy_Edge f, int count);

extern void BiddyFortify(Biddy_Manager MNG, Biddy_Edge f);

extern void BiddyRepair1(Biddy_Manager MNG, Biddy_Variable var);

extern void BiddyRepair2(Biddy_Manager MNG, Biddy_Edge f, Biddy_Variable var, int c);

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

extern void BiddyNodeNumber(Biddy_Edge f, unsigned int *n);

extern void BiddyNodeVarNumber(Biddy_Manager MNG, Biddy_Edge f, unsigned int *n);

/*----------------------------------------------------------------------------*/
/* Prototypes for internal functions defined in biddyInOut.c                  */
/*----------------------------------------------------------------------------*/

#endif  /* _BIDDYINT */

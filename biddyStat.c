/***************************************************************************//*!
\file biddyStat.c
\brief File biddyStat.c contains statistical functions.

### Description

    PackageName [Biddy]
    Synopsis    [Biddy provides data structures and algorithms for the
                 representation and manipulation of Boolean functions with
                 ROBDDs, 0-sup-BDDs, and TZBDDs. A hash table is used for quick
                 search of nodes. Complement edges decreases the number of
                 nodes. An automatic garbage collection with a system age is
                 implemented. Variable swapping and sifting are implemented.]

    FileName    [biddyStat.c]
    Revision    [$Revision: 320 $]
    Date        [$Date: 2017-10-01 12:02:23 +0200 (ned, 01 okt 2017) $]
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

#ifdef BIDDYEXTENDEDSTATS_YES
#include "math.h"
#endif

/*----------------------------------------------------------------------------*/
/* Static function prototypes                                                 */
/*----------------------------------------------------------------------------*/

static void MaxLevel(Biddy_Edge f, unsigned int *max, unsigned int i);

static void AvgLevel(Biddy_Edge f, float *sum, unsigned int *n, unsigned int i);

static void pathCount(Biddy_Manager MNG, Biddy_Edge f,  unsigned long long int *c1, unsigned long long int *c0, Biddy_Boolean *cf);

static void mintermCount(Biddy_Manager MNG, Biddy_Edge f, mpz_t max, mpz_t result, Biddy_Boolean *leftmost);

static unsigned int nodePlainNumber(Biddy_Manager MNG, Biddy_Edge f);

static float calculateSD(unsigned long long int *data, unsigned int n);

/*----------------------------------------------------------------------------*/
/* Definition of exported functions                                           */
/*----------------------------------------------------------------------------*/

/***************************************************************************//*!
\brief Function Biddy_Managed_NodeNumber.

### Description
    Count number of nodes in a BDD.
### Side effects
    This function must be managed because node selection is used.
### More info
    Macro Biddy_Managed_NodeNumber(f) is defined for use with anonymous manager.
*******************************************************************************/

#ifdef __cplusplus
extern "C" {
#endif

unsigned int
Biddy_Managed_NodeNumber(Biddy_Manager MNG, Biddy_Edge f)
{
  unsigned int n;

  if (!MNG) MNG = biddyAnonymousManager;

  if (Biddy_IsNull(f)) return 0;
  if (Biddy_IsConstant(f)) return 1;

  if (biddyManagerType == BIDDYTYPEOBDD) {
    /* IMPLEMENTED */
    /* terminal 0 is represented by complemented edge to terminal 1 */
    n = 2; /* BOTH CONSTANT NODES ARE COUNTED HERE */
  } else if (biddyManagerType == BIDDYTYPEOBDDC) {
    /* IMPLEMENTED */
    n = 1; /* CONSTANT NODE IS COUNTED HERE */
  } else if (biddyManagerType == BIDDYTYPEZBDD) {
    /* IMPLEMENTED */
    /* terminal 0 is represented by complemented edge to terminal 1 */
    /* TO DO: we do not need exact number of complemented edges here */
    n = 0;
    Biddy_Managed_SelectNode(MNG, biddyTerminal); /* needed for BiddyComplementedEdgeNumber */
    BiddyComplementedEdgeNumber(MNG, f, &n);
    Biddy_Managed_DeselectAll(MNG);
    if (n == 0) {
      n = 1; /* THE CONSTANT NODE IS COUNTED HERE */
    }
    else {
      n = 2; /* BOTH CONSTANT NODES ARE COUNTED HERE */
    }
  } else if (biddyManagerType == BIDDYTYPEZBDDC) {
    /* IMPLEMENTED */
    n = 1; /* CONSTANT NODE IS COUNTED HERE */
  } else if (biddyManagerType == BIDDYTYPETZBDD) {
    /* IMPLEMENTED */
    /* terminal 0 is represented by complemented edge to terminal 1 */
    /* TO DO: we do not need exact number of complemented edges here */
    n = 0;
    Biddy_Managed_SelectNode(MNG, biddyTerminal); /* needed for BiddyComplementedEdgeNumber */
    BiddyComplementedEdgeNumber(MNG, f, &n);
    Biddy_Managed_DeselectAll(MNG);
    if (n == 0) {
      n = 1; /* THE CONSTANT NODE IS COUNTED HERE */
    } else {
      n = 2; /* BOTH CONSTANT NODES ARE COUNTED HERE */
    }
  } else if (biddyManagerType == BIDDYTYPETZBDDC) {
    /* IMPLEMENTED */
    n = 1; /* CONSTANT NODE IS COUNTED HERE */
  } else {
    fprintf(stderr,"Biddy_NodeNumberPlain: Unsupported GDD type!\n");
    return 0;
  }

  Biddy_Managed_SelectNode(MNG,biddyTerminal); /* needed for BiddyNodeNumber */
  BiddyNodeNumber(MNG,f,&n);
  Biddy_Managed_DeselectAll(MNG);

  return n;
}

#ifdef __cplusplus
}
#endif

/***************************************************************************//*!
\brief Function Biddy_NodeMaxLevel.

### Description
### Side effects
### More info
*******************************************************************************/

#ifdef __cplusplus
extern "C" {
#endif

unsigned int
Biddy_NodeMaxLevel(Biddy_Edge f)
{
  unsigned int max;

  if (Biddy_IsNull(f)) return 0;

  max = 0;
  MaxLevel(f,&max,0);
  return max;
}

#ifdef __cplusplus
}
#endif

/***************************************************************************//*!
\brief Function Biddy_NodeAvgLevel.

### Description
### Side effects
### More info
*******************************************************************************/

#ifdef __cplusplus
extern "C" {
#endif

float
Biddy_NodeAvgLevel(Biddy_Edge f)
{
  float sum;
  unsigned int n;

  if (Biddy_IsNull(f)) return 0;

  sum = 0;
  n = 0;
  AvgLevel(f,&sum,&n,0);
  return (sum/n);
}

#ifdef __cplusplus
}
#endif

/***************************************************************************//*!
\brief Function Biddy_Managed_VariableTableNum returns number of used variables.

### Description
### Side effects
    Variable '1' is included.
### More info
    Macro Biddy_VariableTableNum() is defined for use with anonymous manager.
*******************************************************************************/

#ifdef __cplusplus
extern "C" {
#endif

Biddy_Variable
Biddy_Managed_VariableTableNum(Biddy_Manager MNG)
{
  if (!MNG) MNG = biddyAnonymousManager;

  return biddyVariableTable.num;
}

#ifdef __cplusplus
}
#endif

/***************************************************************************//*!
\brief Function Biddy_Managed_NodeTableSize returns the size of node table.

### Description
### Side effects
### More info
    Macro Biddy_NodeTableSize() is defined for use with anonymous manager.
*******************************************************************************/

#ifdef __cplusplus
extern "C" {
#endif

unsigned int
Biddy_Managed_NodeTableSize(Biddy_Manager MNG)
{
  if (!MNG) MNG = biddyAnonymousManager;

  /* THE REAL SIZE OF NODE TABLE IS biddyNodeTable.size+2. */
  /* THE FIRST INDEX IS NOT USED, */
  /* THUS WE HAVE biddyNodeTable.size+1 USEFUL ELEMENTS. */
               
  return biddyNodeTable.size+1;
}

#ifdef __cplusplus
}
#endif

/***************************************************************************//*!
\brief Function Biddy_Managed_NodeTableBlockNumber.

### Description
### Side effects
### More info
    Macro Biddy_NodeTableBlockNumber() is defined for use with anonymous
    manager.
*******************************************************************************/

#ifdef __cplusplus
extern "C" {
#endif

unsigned int
Biddy_Managed_NodeTableBlockNumber(Biddy_Manager MNG)
{
  if (!MNG) MNG = biddyAnonymousManager;

  return biddyNodeTable.blocknumber;
}

#ifdef __cplusplus
}
#endif

/***************************************************************************//*!
\brief Function Biddy_Managed_NodeTableGenerated.

### Description
### Side effects
### More info
    Macro Biddy_NodeTableGenerated() is defined for use with anonymous manager.
*******************************************************************************/

#ifdef __cplusplus
extern "C" {
#endif

unsigned int
Biddy_Managed_NodeTableGenerated(Biddy_Manager MNG)
{
  if (!MNG) MNG = biddyAnonymousManager;

  return biddyNodeTable.generated;
}

#ifdef __cplusplus
}
#endif

/***************************************************************************//*!
\brief Function Biddy_Managed_NodeTableMax returns maximal (peek) number of
       nodes in node table.

### Description
### Side effects
### More info
    Macro Biddy_NodeTableMax() is defined for use with anonymous manager.
*******************************************************************************/

#ifdef __cplusplus
extern "C" {
#endif

unsigned int
Biddy_Managed_NodeTableMax(Biddy_Manager MNG)
{
  if (!MNG) MNG = biddyAnonymousManager;

  return biddyNodeTable.max;
}

#ifdef __cplusplus
}
#endif

/***************************************************************************//*!
\brief Function Biddy_Managed_NodeTableNum returns number of all nodes
       currently in node table.

### Description
### Side effects
### More info
    Macro Biddy_NodeTableNum() is defined for use with anonymous manager.
*******************************************************************************/

#ifdef __cplusplus
extern "C" {
#endif

unsigned int
Biddy_Managed_NodeTableNum(Biddy_Manager MNG)
{
  if (!MNG) MNG = biddyAnonymousManager;

  return biddyNodeTable.num;
}

#ifdef __cplusplus
}
#endif

/***************************************************************************//*!
\brief Function Biddy_Managed_NodeTableNumVar returns number of nodes with
       a given variable currently in node table.

### Description
### Side effects
### More info
    Macro Biddy_NodeTableNumVar(v) is defined for use with anonymous manager.
*******************************************************************************/

#ifdef __cplusplus
extern "C" {
#endif

unsigned int
Biddy_Managed_NodeTableNumVar(Biddy_Manager MNG, Biddy_Variable v)
{
  if (!MNG) MNG = biddyAnonymousManager;

  return biddyVariableTable.table[v].num;
}

#ifdef __cplusplus
}
#endif

/***************************************************************************//*!
\brief Function Biddy_Managed_NodeTableResizeNumber.

### Description
### Side effects
### More info
    Macro Biddy_NodeTableResizeNumber() is defined for use with anonymous
    manager.
*******************************************************************************/

#ifdef __cplusplus
extern "C" {
#endif

unsigned int
Biddy_Managed_NodeTableResizeNumber(Biddy_Manager MNG)
{
  if (!MNG) MNG = biddyAnonymousManager;

  return biddyNodeTable.nodetableresize;
}

#ifdef __cplusplus
}
#endif

/***************************************************************************//*!
\brief Function Biddy_Managed_NodeTableFoaNumber.

### Description
### Side effects
### More info
    Macro Biddy_NodeTableFoaNumber() is defined for use with anonymous
    manager.
*******************************************************************************/

#ifdef __cplusplus
extern "C" {
#endif

unsigned long long int
Biddy_Managed_NodeTableFoaNumber(Biddy_Manager MNG)
{
  unsigned long long int foa = 0;

  if (!MNG) MNG = biddyAnonymousManager;

#ifdef BIDDYEXTENDEDSTATS_YES
  foa = biddyNodeTable.foa;
#endif

  return foa;
}

#ifdef __cplusplus
}
#endif

/***************************************************************************//*!
\brief Function Biddy_Managed_NodeTableFindNumber.

### Description
### Side effects
### More info
    Macro Biddy_NodeTableFindNumber() is defined for use with anonymous
    manager.
*******************************************************************************/

#ifdef __cplusplus
extern "C" {
#endif

unsigned long long int
Biddy_Managed_NodeTableFindNumber(Biddy_Manager MNG)
{
  unsigned long long int find = 0;

  if (!MNG) MNG = biddyAnonymousManager;

#ifdef BIDDYEXTENDEDSTATS_YES
  find = biddyNodeTable.find;
#endif

  return find;
}

#ifdef __cplusplus
}
#endif

/***************************************************************************//*!
\brief Function Biddy_Managed_NodeTableCompareNumber.

### Description
### Side effects
### More info
    Macro Biddy_NodeTableCompareNumber() is defined for use with anonymous
    manager.
*******************************************************************************/

#ifdef __cplusplus
extern "C" {
#endif

unsigned long long int
Biddy_Managed_NodeTableCompareNumber(Biddy_Manager MNG)
{
  unsigned long long int compare = 0;

  if (!MNG) MNG = biddyAnonymousManager;

#ifdef BIDDYEXTENDEDSTATS_YES
  compare = biddyNodeTable.compare;
#endif

  return compare;
}

#ifdef __cplusplus
}
#endif

/***************************************************************************//*!
\brief Function Biddy_Managed_NodeTableAddNumber.

### Description
### Side effects
### More info
    Macro Biddy_NodeTableAddNumber() is defined for use with anonymous
    manager.
*******************************************************************************/

#ifdef __cplusplus
extern "C" {
#endif

unsigned long long int
Biddy_Managed_NodeTableAddNumber(Biddy_Manager MNG)
{
  unsigned long long int add = 0;

  if (!MNG) MNG = biddyAnonymousManager;

#ifdef BIDDYEXTENDEDSTATS_YES
  add = biddyNodeTable.add;
#endif

  return add;
}

#ifdef __cplusplus
}
#endif

/***************************************************************************//*!
\brief Function Biddy_Managed_NodeTableGCNumber.

### Description
### Side effects
### More info
    Macro Biddy_NodeTableGCNumber() is defined for use with anonymous manager.
*******************************************************************************/

#ifdef __cplusplus
extern "C" {
#endif

unsigned int
Biddy_Managed_NodeTableGCNumber(Biddy_Manager MNG)
{
  if (!MNG) MNG = biddyAnonymousManager;

  return biddyNodeTable.garbage;
}

#ifdef __cplusplus
}
#endif

/***************************************************************************//*!
\brief Function Biddy_Managed_NodeTableGCTime.

### Description
### Side effects
### More info
    Macro Biddy_NodeTableGCTime() is defined for use with anonymous manager.
*******************************************************************************/

#ifdef __cplusplus
extern "C" {
#endif

clock_t
Biddy_Managed_NodeTableGCTime(Biddy_Manager MNG)
{
  if (!MNG) MNG = biddyAnonymousManager;

  return biddyNodeTable.gctime;
}

#ifdef __cplusplus
}
#endif

/***************************************************************************//*!
\brief Function Biddy_Managed_NodeTableGCObsoleteNumber.

### Description
    Return the number of nodes deleted by GC.
### Side effects
    Obsolete nodes deleted by GC are counted only if Biddy is compiled using
    directive BIDDYEXTENDEDSTATS_YES.
### More info
    Macro Biddy_NodeTableGCObsoleteNumber() is defined for use with anonymous
    manager.
*******************************************************************************/

#ifdef __cplusplus
extern "C" {
#endif

unsigned long long int
Biddy_Managed_NodeTableGCObsoleteNumber(Biddy_Manager MNG)
{
  unsigned long long int gcobsolete = 0;

#ifdef BIDDYEXTENDEDSTATS_YES
  unsigned int i;
  unsigned long long int sum;

  if (!MNG) MNG = biddyAnonymousManager;

  sum = 0;
  for (i = 0; i < biddyNodeTable.garbage; i++) {
    sum += biddyNodeTable.gcobsolete[i];
  }
  gcobsolete = sum;
  calculateSD(biddyNodeTable.gcobsolete,biddyNodeTable.garbage);
#endif

  return gcobsolete;
}

#ifdef __cplusplus
}
#endif

/***************************************************************************//*!
\brief Function Biddy_Managed_NodeTableSwapNumber.

### Description
### Side effects
### More info
    Macro Biddy_NodeTableSwapNumber() is defined for use with anonymous manager.
*******************************************************************************/

#ifdef __cplusplus
extern "C" {
#endif

unsigned int
Biddy_Managed_NodeTableSwapNumber(Biddy_Manager MNG)
{
  if (!MNG) MNG = biddyAnonymousManager;

  return biddyNodeTable.swap;
}

#ifdef __cplusplus
}
#endif

/***************************************************************************//*!
\brief Function Biddy_Managed_NodeTableSiftingNumber.

### Description
### Side effects
### More info
    Macro Biddy_NodeTableSiftingNumber() is defined for use with anonymous
    manager.
*******************************************************************************/

#ifdef __cplusplus
extern "C" {
#endif

unsigned int
Biddy_Managed_NodeTableSiftingNumber(Biddy_Manager MNG)
{
  if (!MNG) MNG = biddyAnonymousManager;

  return biddyNodeTable.sifting;
}

#ifdef __cplusplus
}
#endif

/***************************************************************************//*!
\brief Function Biddy_Managed_NodeTableDRTime.

### Description
### Side effects
### More info
    Macro Biddy_NodeTableDRTime() is defined for use with anonymous manager.
*******************************************************************************/

#ifdef __cplusplus
extern "C" {
#endif

clock_t
Biddy_Managed_NodeTableDRTime(Biddy_Manager MNG)
{
  if (!MNG) MNG = biddyAnonymousManager;

  return biddyNodeTable.drtime;
}

#ifdef __cplusplus
}
#endif

/***************************************************************************//*!
\brief Function Biddy_Managed_NodeTableITENumber.

### Description
### Side effects
### More info
    Macro Biddy_NodeTableITENumber() is defined for use with anonymous manager.
*******************************************************************************/

#ifdef __cplusplus
extern "C" {
#endif

unsigned int
Biddy_Managed_NodeTableITENumber(Biddy_Manager MNG)
{
  if (!MNG) MNG = biddyAnonymousManager;

  return biddyNodeTable.funite;
}

#ifdef __cplusplus
}
#endif

/***************************************************************************//*!
\brief Function Biddy_Managed_NodeTableITERecursiveNumber.

### Description
### Side effects
    Recursive ITE calls are counted only if Biddy is compiled using directive
    BIDDYEXTENDEDSTATS_YES.
### More info
    Macro Biddy_NodeTableITERecursiveNumber() is defined for use with anonymous
    manager.
*******************************************************************************/

#ifdef __cplusplus
extern "C" {
#endif

unsigned long long int
Biddy_Managed_NodeTableITERecursiveNumber(Biddy_Manager MNG)
{
  unsigned long long int iterecursive = 0;

  if (!MNG) MNG = biddyAnonymousManager;

#ifdef BIDDYEXTENDEDSTATS_YES
  iterecursive = biddyNodeTable.iterecursive;
#endif

  return iterecursive;
}

#ifdef __cplusplus
}
#endif

/***************************************************************************//*!
\brief Function Biddy_Managed_NodeTableANDORNumber.

### Description
### Side effects
### More info
    Macro Biddy_NodeTableANDORNumber() is defined for use with anonymous
    manager.
*******************************************************************************/

#ifdef __cplusplus
extern "C" {
#endif

unsigned int
Biddy_Managed_NodeTableANDORNumber(Biddy_Manager MNG)
{
  if (!MNG) MNG = biddyAnonymousManager;

  return biddyNodeTable.funandor;
}

#ifdef __cplusplus
}
#endif

/***************************************************************************//*!
\brief Function Biddy_Managed_NodeTableANDORRecursiveNumber.

### Description
### Side effects
    Recursive AND/OR calls are counted only if Biddy is compiled using directive
    BIDDYEXTENDEDSTATS_YES.
### More info
    Macro Biddy_NodeTableANDORRecursiveNumber() is defined for use with
    anonymous manager.
*******************************************************************************/

#ifdef __cplusplus
extern "C" {
#endif

unsigned long long int
Biddy_Managed_NodeTableANDORRecursiveNumber(Biddy_Manager MNG)
{
  unsigned long long int andorrecursive = 0;

  if (!MNG) MNG = biddyAnonymousManager;

#ifdef BIDDYEXTENDEDSTATS_YES
  andorrecursive = biddyNodeTable.andorrecursive;
#endif

  return andorrecursive;
}

#ifdef __cplusplus
}
#endif

/***************************************************************************//*!
\brief Function Biddy_Managed_NodeTableXORNumber.

### Description
### Side effects
### More info
    Macro Biddy_NodeTableXORNumber() is defined for use with anonymous manager.
*******************************************************************************/

#ifdef __cplusplus
extern "C" {
#endif

unsigned int
Biddy_Managed_NodeTableXORNumber(Biddy_Manager MNG)
{
  if (!MNG) MNG = biddyAnonymousManager;

  return biddyNodeTable.funxor;
}

#ifdef __cplusplus
}
#endif

/***************************************************************************//*!
\brief Function Biddy_Managed_NodeTableXORRecursiveNumber.

### Description
### Side effects
    Recursive XOR calls are counted only if Biddy is compiled using directive
    BIDDYEXTENDEDSTATS_YES.
### More info
    Macro Biddy_NodeTableXORRecursiveNumber() is defined for use with
    anonymous manager.
*******************************************************************************/

#ifdef __cplusplus
extern "C" {
#endif

unsigned long long int
Biddy_Managed_NodeTableXORRecursiveNumber(Biddy_Manager MNG)
{
  unsigned long long int xorrecursive = 0;

  if (!MNG) MNG = biddyAnonymousManager;

#ifdef BIDDYEXTENDEDSTATS_YES
  xorrecursive = biddyNodeTable.xorrecursive;
#endif

  return xorrecursive;
}

#ifdef __cplusplus
}
#endif

/***************************************************************************//*!
\brief Function Biddy_Managed_FormulaTableNum returns number of known formulae.

### Description
### Side effects
    Formulae '0' and '1' are included.
### More info
    Macro Biddy_FormulaTableNum() is defined for use with anonymous manager.
*******************************************************************************/

#ifdef __cplusplus
extern "C" {
#endif

unsigned int
Biddy_Managed_FormulaTableNum(Biddy_Manager MNG)
{
  if (!MNG) MNG = biddyAnonymousManager;

  return biddyFormulaTable.size;
}

#ifdef __cplusplus
}
#endif

/***************************************************************************//*!
\brief Function Biddy_Managed_ListUsed.

### Description
### Side effects
### More info
    Macro Biddy_ListUsed() is defined for use with anonymous manager.
*******************************************************************************/

#ifdef __cplusplus
extern "C" {
#endif

unsigned int
Biddy_Managed_ListUsed(Biddy_Manager MNG)
{
  unsigned int i,n;

  if (!MNG) MNG = biddyAnonymousManager;

  n = 0;
  for (i=1; i<=biddyNodeTable.size+1; i++) {
    if (biddyNodeTable.table[i] != NULL) {
      n++;
    }
  }
  return n;
}

#ifdef __cplusplus
}
#endif

/***************************************************************************//*!
\brief Function Biddy_Managed_ListMaxLength.

### Description
### Side effects
### More info
    Macro Biddy_ListMaxLength() is defined for use with anonymous manager.
*******************************************************************************/

#ifdef __cplusplus
extern "C" {
#endif

unsigned int
Biddy_Managed_ListMaxLength(Biddy_Manager MNG)
{
  unsigned int i,n,max;
  BiddyNode *sup;

  /* DEBUG */
  /*
  unsigned maxi;
  */

  if (!MNG) MNG = biddyAnonymousManager;

  max = 0;
  for (i=1; i<=biddyNodeTable.size+1; i++) {
    if (biddyNodeTable.table[i] != NULL) {
      n = 0;
      sup = biddyNodeTable.table[i];
      while (sup != NULL) {
        n++;
        assert( sup != sup->next );
        sup = sup->next;
      }
      if (n > max) {
        max = n;

        /* DEBUG */
        /*
        maxi = i;
        */
      }
    }
  }

  /* DEBUG */
  /*
  sup = biddyNodeTable.table[maxi];
  printf("MAX LIST (%u):\n",maxi);
  while (sup != NULL) {
    printf("v=%u, else=%p, then=%p\n",sup->v,(void*)sup->f,(void*)sup->t);
    sup = sup->next;
  }
  */

  return max;
}

#ifdef __cplusplus
}
#endif

/***************************************************************************//*!
\brief Function Biddy_Managed_ListAvgLength.

### Description
### Side effects
### More info
    Macro Biddy_ListAvgLength() is defined for use with anonymous manager.
*******************************************************************************/

#ifdef __cplusplus
extern "C" {
#endif

float
Biddy_Managed_ListAvgLength(Biddy_Manager MNG)
{
  float sum;
  unsigned int i,n;
  BiddyNode *sup;

  if (!MNG) MNG = biddyAnonymousManager;

  sum = 0;
  n = 0;
  for (i=1; i<=biddyNodeTable.size+1; i++) {
    if (biddyNodeTable.table[i] != NULL) {
      n++;
      sup = biddyNodeTable.table[i];
      while (sup != NULL) {
        sum = sum + 1;
        assert( sup != sup->next );
        sup = sup->next;
      }
    }
  }
  return (sum/n);
}

#ifdef __cplusplus
}
#endif

/***************************************************************************//*!
\brief Function Biddy_Managed_OPCacheSearch.

### Description
### Side effects
### More info
    Macro Biddy_OPCacheSearch() is defined for use with anonymous manager.
*******************************************************************************/

#ifdef __cplusplus
extern "C" {
#endif

unsigned long long int
Biddy_Managed_OPCacheSearch(Biddy_Manager MNG)
{
  if (!MNG) MNG = biddyAnonymousManager;

  return *(biddyOPCache.search);
}

#ifdef __cplusplus
}
#endif

/***************************************************************************//*!
\brief Function Biddy_Managed_OPCacheFind.

### Description
### Side effects
### More info
    Macro Biddy_OPCacheFind() is defined for use with anonymous manager.
*******************************************************************************/

#ifdef __cplusplus
extern "C" {
#endif

unsigned long long int
Biddy_Managed_OPCacheFind(Biddy_Manager MNG)
{
  if (!MNG) MNG = biddyAnonymousManager;

  return *(biddyOPCache.find);
}

#ifdef __cplusplus
}
#endif

/***************************************************************************//*!
\brief Function Biddy_Managed_OPCacheInsert.

### Description
### Side effects
### More info
    Macro Biddy_OPCacheInsert() is defined for use with anonymous manager.
*******************************************************************************/

#ifdef __cplusplus
extern "C" {
#endif

unsigned long long int
Biddy_Managed_OPCacheInsert(Biddy_Manager MNG)
{
  unsigned long long int insert = 0;

  if (!MNG) MNG = biddyAnonymousManager;

#ifdef BIDDYEXTENDEDSTATS_YES
  insert = *(biddyOPCache.insert);
#endif

  return insert;
}

#ifdef __cplusplus
}
#endif

/***************************************************************************//*!
\brief Function Biddy_Managed_OPCacheOverwrite.

### Description
### Side effects
### More info
    Macro Biddy_OPCacheOverwrite() is defined for use with anonymous manager.
*******************************************************************************/

#ifdef __cplusplus
extern "C" {
#endif

unsigned long long int
Biddy_Managed_OPCacheOverwrite(Biddy_Manager MNG)
{
  unsigned long long int overwrite = 0;

  if (!MNG) MNG = biddyAnonymousManager;

#ifdef BIDDYEXTENDEDSTATS_YES
  overwrite = *(biddyOPCache.overwrite);
#endif

  return overwrite;
}

#ifdef __cplusplus
}
#endif

/***************************************************************************//*!
\brief Function Biddy_Managed_NodeNumberPlain.

### Description
    Count number of nodes in a corresponding BDD without complement edges.
### Side effects
### More info
    Macro Biddy_Managed_NodeNumberPlain(f) is defined for use with anonymous manager.
*******************************************************************************/

#ifdef __cplusplus
extern "C" {
#endif

unsigned int
Biddy_Managed_NodeNumberPlain(Biddy_Manager MNG, Biddy_Edge f)
{
  unsigned int n;

  if (!MNG) MNG = biddyAnonymousManager;

  if (Biddy_IsNull(f)) return 0;

  if (biddyManagerType == BIDDYTYPEOBDD) {
    /* IMPLEMENTED */
    /* terminal 0 is represented by complemented edge to terminal 1 */
    return Biddy_Managed_NodeNumber(MNG,f);
  } else if (biddyManagerType == BIDDYTYPEOBDDC) {
    /* IMPLEMENTED */
  } else if (biddyManagerType == BIDDYTYPEZBDD) {
    /* IMPLEMENTED */
    /* terminal 0 is represented by complemented edge to terminal 1 */
    return Biddy_Managed_NodeNumber(MNG,f);
  } else if (biddyManagerType == BIDDYTYPEZBDDC) {
    /* IMPLEMENTED */
  } else if (biddyManagerType == BIDDYTYPETZBDD) {
    /* IMPLEMENTED */
    /* terminal 0 is represented by complemented edge to terminal 1 */
    return Biddy_Managed_NodeNumber(MNG,f);
  } else if (biddyManagerType == BIDDYTYPETZBDDC) {
    /* NOT IMPLEMENTED, YET */
    return 0;
  } else {
    fprintf(stderr,"Biddy_NodeNumberPlain: Unsupported GDD type!\n");
    return 0;
  }

  if (Biddy_IsConstant(f)) return 1;

  n = 2; /* BOTH CONSTANT NODES ARE COUNTED HERE */
#pragma warning(suppress: 6031)
  BiddyCreateLocalInfo(MNG,f);
  n += nodePlainNumber(MNG,f); /* select all nodes except constant node */
  BiddyDeleteLocalInfo(MNG,f);

  return n;
}

#ifdef __cplusplus
}
#endif

/***************************************************************************//*!
\brief Function Biddy_Managed_DependentVariableNumber.

### Description
    Count number of dependent variables.
    For OBDD, the number of dependent variables is the same as the number
    of variables in the graph. For ZBDD and TZBDD, this is not true.
### Side effects
### More info
    Macro Biddy_DependentVariableNumber(f) is defined for use with anonymous
    manager.
*******************************************************************************/

#ifdef __cplusplus
extern "C" {
#endif

unsigned int
Biddy_Managed_DependentVariableNumber(Biddy_Manager MNG, Biddy_Edge f)
{
  Biddy_Variable i;
  unsigned int n;
  unsigned int v;

  if (!MNG) MNG = biddyAnonymousManager;

  if (Biddy_IsNull(f)) return 0;
  if (Biddy_IsConstant(f)) return 0;

  /* variables should not be selected */
  /*
  for (i=1;i<biddyVariableTable.num;i++) {
    biddyVariableTable.table[i].selected = FALSE;
  }
  */

  v = 0;
  if ((biddyManagerType == BIDDYTYPEOBDDC) || (biddyManagerType == BIDDYTYPEOBDD)) {
    /* IMPLEMENTED */
    n = 1; /* NOT NEEDED, BUT NODES ARE ALSO COUNTED BY BiddyNodeVarNumber */
    Biddy_Managed_SelectNode(MNG,biddyTerminal); /* for OBDDs, this is needed for BiddyNodeVarNumber */
    BiddyNodeVarNumber(MNG,f,&n); /* DEPENDENT VARIABLES ARE MARKED, all nodes are selected */
    Biddy_Managed_DeselectAll(MNG); /* deselect all nodes */
    for (i=1;i<biddyVariableTable.num;i++) {
      if (biddyVariableTable.table[i].selected == TRUE) {
        v++;
        biddyVariableTable.table[i].selected = FALSE; /* deselect variable */
      }
    }
  }
  else if ((biddyManagerType == BIDDYTYPEZBDDC) || (biddyManagerType == BIDDYTYPEZBDD)) {
    /* PROTOTYPED USING Restrict */
    /*
    for (i=1;i<biddyVariableTable.num;i++) {
      if (BiddyManagedRestrict(MNG,f,i,FALSE) != BiddyManagedRestrict(MNG,f,i,TRUE)) {
        v++;
      }
    }
    */
    /* IMPLEMENTED */
    BiddyCreateLocalInfo(MNG,f); /* FOR ZBDDs, localinfo IS USED BY BiddyNodeVarNumber */
    n = 1; /* NOT NEEDED, BUT NODES ARE ALSO COUNTED BY BiddyNodeVarNumber */
    BiddyNodeVarNumber(MNG,f,&n); /* DEPENDENT VARIABLES ARE MARKED, ALL NODES ARE SELECTED */
    BiddyDeleteLocalInfo(MNG,f); /* FOR ZBDDs, localinfo IS USED BY BiddyNodeVarNumber */
    v = 0;
    for (i=1;i<biddyVariableTable.num;i++) {
      if (BiddyIsSmaller(i,BiddyV(f)) ||
          (biddyVariableTable.table[i].selected == TRUE))
      {
        v++;
        biddyVariableTable.table[i].selected = FALSE; /* deselect variable */
      }
    }
  }
  else if ((biddyManagerType == BIDDYTYPETZBDDC) || (biddyManagerType == BIDDYTYPETZBDD)) {
    /* PROTOTYPED USING Restrict */
    /*
    for (i=1;i<biddyVariableTable.num;i++) {
      if (BiddyManagedRestrict(MNG,f,i,FALSE) != BiddyManagedRestrict(MNG,f,i,TRUE)) {
        v++;
      }
    }
    */
    /* IMPLEMENTED */
    n = 1; /* NOT NEEDED, BUT NODES ARE ALSO COUNTED BY BiddyNodeVarNumber */
    Biddy_Managed_SelectNode(MNG,biddyTerminal); /* for TZBDDs, this is needed for BiddyNodeVarNumber */
    BiddyNodeVarNumber(MNG,f,&n); /* DEPENDENT VARIABLES ARE MARKED, all nodes are selected */
    Biddy_Managed_DeselectAll(MNG); /* deselect all nodes */
    for (i=1;i<biddyVariableTable.num;i++) {
      if (biddyVariableTable.table[i].selected == TRUE) {
        v++;
        biddyVariableTable.table[i].selected = FALSE; /* deselect variable */
      }
    }
  } else if ((biddyManagerType == BIDDYTYPEOFDD) || (biddyManagerType == BIDDYTYPEOFDDC) ||
              (biddyManagerType == BIDDYTYPEZFDD) || (biddyManagerType == BIDDYTYPEZFDDC) ||
              (biddyManagerType == BIDDYTYPETZFDD) || (biddyManagerType == BIDDYTYPETZFDDC))
  {
    fprintf(stderr,"Biddy_DependentVariableNumber: this GDD type is not supported, yet!\n");
    return 0;
  } else {
    fprintf(stderr,"Biddy_DependentVariableNumber: Unsupported GDD type!\n");
    return 0;
  }

  return v;
}

#ifdef __cplusplus
}
#endif

/***************************************************************************//*!
\brief Function Biddy_Managed_NodeVarNumber.

### Description
    Count number of nodes and number of dependent variables.
    For OBDD, the number of variables existing in the graph is the same as
    the number of dependent variables. For ZBDD and TZBDD, this is not true.
### Side effects
### More info
    Macro Biddy_NodeVarNumber(f,n,v) is defined for use with anonymous manager.
*******************************************************************************/

#ifdef __cplusplus
extern "C" {
#endif

void
Biddy_Managed_NodeVarNumber(Biddy_Manager MNG, Biddy_Edge f, unsigned int *n,
                            unsigned int *v)
{
  Biddy_Variable i;

  if (!MNG) MNG = biddyAnonymousManager;

  (*n) = 0;
  (*v) = 0;

  if (Biddy_IsNull(f)) return;

  (*n) = 1; /* CONSTANT NODE IS COUNTED HERE */

  if (Biddy_IsConstant(f)) return;

  /* VARIABLES SHOULD NOT BE SELECTED */
  /*
  for (i=1;i<biddyVariableTable.num;i++) {
    biddyVariableTable.table[i].selected = FALSE;
  }
  */

  if ((biddyManagerType == BIDDYTYPEOBDDC) || (biddyManagerType == BIDDYTYPEOBDD))
  {
    Biddy_Managed_SelectNode(MNG,biddyTerminal);  /* for OBDDs, this is needed for BiddyNodeVarNumber */
    BiddyNodeVarNumber(MNG,f,n); /* DEPENDENT VARIABLES ARE MARKED, all nodes are selected */
    Biddy_Managed_DeselectAll(MNG);
  }
  else if ((biddyManagerType == BIDDYTYPEZBDDC) || (biddyManagerType == BIDDYTYPEZBDD))
  {
    BiddyCreateLocalInfo(MNG,f); /* FOR ZBDDs, localinfo IS USED BY BiddyNodeVarNumber */
    BiddyNodeVarNumber(MNG,f,n); /* DEPENDENT VARIABLES ARE MARKED, selects all except terminal node */
    BiddyDeleteLocalInfo(MNG,f); /* FOR ZBDDs, localinfo IS USED BY BiddyNodeVarNumber */
  }
  else if ((biddyManagerType == BIDDYTYPETZBDDC) || (biddyManagerType == BIDDYTYPETZBDD))
  {
    Biddy_Managed_SelectNode(MNG,biddyTerminal);  /* for TZBDDs, this is needed for BiddyNodeVarNumber */
    BiddyNodeVarNumber(MNG,f,n); /* DEPENDENT VARIABLES ARE MARKED, all nodes are selected */
    Biddy_Managed_DeselectAll(MNG);
  }
  else if ((biddyManagerType == BIDDYTYPEOFDD) || (biddyManagerType == BIDDYTYPEOFDDC) ||
              (biddyManagerType == BIDDYTYPEZFDD) || (biddyManagerType == BIDDYTYPEZFDDC) ||
              (biddyManagerType == BIDDYTYPETZFDD) || (biddyManagerType == BIDDYTYPETZFDDC))
  {
    fprintf(stderr,"Biddy_NodeVarNumber: this GDD type is not supported, yet!\n");
    return;
  }
  else {
    fprintf(stderr,"Biddy_NodeVarNumber: Unsupported GDD type!\n");
    return;
  }

  for (i=1;i<biddyVariableTable.num;i++) {
    if (biddyVariableTable.table[i].selected == TRUE) {
      (*v)++;
      biddyVariableTable.table[i].selected = FALSE; /* deselect variable */
    }
  }
}

#ifdef __cplusplus
}
#endif

/***************************************************************************//*!
\brief Function Biddy_Managed_CountComplemented count the number of
        complemented edges.

### Description
    Count number of complemented edges in a given BDD.
### Side effects
    Terminal 0 is represented by complemented edge to terminal 1 with all
    BDD types but this edge is counted as a complemented one only if
    complemented edges are explicitly used.
### More info
    Macro Biddy_Managed_CountComplemented(f) is defined for use with anonymous
    manager.
*******************************************************************************/

#ifdef __cplusplus
extern "C" {
#endif

unsigned int
Biddy_Managed_CountComplemented(Biddy_Manager MNG, Biddy_Edge f)
{
  unsigned int n;

  if (!MNG) MNG = biddyAnonymousManager;

  if (Biddy_IsNull(f)) return 0;

  if (biddyManagerType == BIDDYTYPEOBDD) {
    /* IMPLEMENTED */
    return 0;
  } else if (biddyManagerType == BIDDYTYPEOBDDC) {
    /* IMPLEMENTED */
  } else if (biddyManagerType == BIDDYTYPEZBDD) {
    /* IMPLEMENTED */
    return 0;
  } else if (biddyManagerType == BIDDYTYPEZBDDC) {
    /* IMPLEMENTED */
  } else if (biddyManagerType == BIDDYTYPETZBDD) {
    /* IMPLEMENTED */
    return 0;
  } else if (biddyManagerType == BIDDYTYPETZBDDC) {
    /* IMPLEMENTED */
  } else {
    fprintf(stderr,"Biddy_CountComplemented: Unsupported GDD type!\n");
    return 0;
  }

  if (Biddy_GetMark(f)) {
    n = 1;
  } else {
    n = 0;
  }

  if (Biddy_IsConstant(f)) return n;

  Biddy_Managed_SelectNode(MNG,biddyTerminal); /* needed for BiddyComplementedEdgeNumber */
  BiddyComplementedEdgeNumber(MNG,f,&n);
  Biddy_Managed_DeselectAll(MNG);

  return n;
}

#ifdef __cplusplus
}
#endif

/***************************************************************************//*!
\brief Function Biddy_Managed_CountPaths count the number of 1-paths.

### Description
### Side effects
    Implemented for OBDD, ZBDD, and TZBDD.
    TO DO: implement this using GNU Multiple Precision Arithmetic Library (GMP).
### More info
    Macro Biddy_CountPaths(f) is defined for use with anonymous manager.
*******************************************************************************/

#ifdef __cplusplus
extern "C" {
#endif

unsigned long long int
Biddy_Managed_CountPaths(Biddy_Manager MNG, Biddy_Edge f)
{
  unsigned long long int r1,r0;
  Biddy_Boolean rf;

  if (!MNG) MNG = biddyAnonymousManager;

  if (biddyManagerType == BIDDYTYPEOBDD) {
    /* IMPLEMENTED */
  } else if (biddyManagerType == BIDDYTYPEOBDDC) {
    /* IMPLEMENTED */
  } else if (biddyManagerType == BIDDYTYPEZBDDC) {
    /* IMPLEMENTED */
  } else if (biddyManagerType == BIDDYTYPETZBDD) {
    /* IMPLEMENTED */
  } else if ((biddyManagerType == BIDDYTYPEZBDD) ||
              (biddyManagerType == BIDDYTYPETZBDDC))
  {
    fprintf(stderr,"Biddy_CountPaths: this GDD type is not supported, yet!\n");
    return 0;
  } else if ((biddyManagerType == BIDDYTYPEOFDD) || (biddyManagerType == BIDDYTYPEOFDDC) ||
              (biddyManagerType == BIDDYTYPEZFDD) || (biddyManagerType == BIDDYTYPEZFDDC) ||
              (biddyManagerType == BIDDYTYPETZFDD) || (biddyManagerType == BIDDYTYPETZFDDC))
  {
    fprintf(stderr,"Biddy_CountPaths: this GDD type is not supported, yet!\n");
    return 0;
  } else {
    fprintf(stderr,"Biddy_CountPaths: Unsupported GDD type!\n");
    return 0;
  }

  if (Biddy_IsNull(f)) return 0;
  if (f == biddyZero) return 0;
  if (Biddy_IsConstant(f)) return 1;

  BiddyCreateLocalInfo(MNG,f);
  pathCount(MNG,f,&r1,&r0,&rf); /* all nodes except constant are selected */
  BiddyDeleteLocalInfo(MNG,f); 
  
  return r1;
}

#ifdef __cplusplus
}
#endif

/***************************************************************************//*!
\brief Function Biddy_Managed_CountMinterm.

### Description
    Parameter nvars is a user-defined number of dependent variables.
    If nvars == 0 then number of variables existing in the graph is used.
    For combination sets, this function coincides with combination counting.
### Side effects
    We are using GNU Multiple Precision Arithmetic Library (GMP).
    For ZBDDs, this function coincides with the 1-path count.
### More info
    Macro Biddy_CountMinterm(f,nvars) is defined for use with anonymous manager.
    Macros Biddy_Managed_CountCombination(MNG,f,nvars) and
    Biddy_CountCombination(f,nvars) are defined for use with combination sets.
*******************************************************************************/

#ifdef __cplusplus
extern "C" {
#endif

double
Biddy_Managed_CountMinterm(Biddy_Manager MNG, Biddy_Edge f, unsigned int nvars)
{
  unsigned int var,depvar;
  mpz_t max;
  mpz_t result;
  double resultd;
  Biddy_Boolean leftmost;

  if (!MNG) MNG = biddyAnonymousManager;

  if (biddyManagerType == BIDDYTYPEOBDD) {
    /* IMPLEMENTED */
  } else if (biddyManagerType == BIDDYTYPEOBDDC) {
    /* IMPLEMENTED */
  } else if (biddyManagerType == BIDDYTYPEZBDDC) {
    /* IMPLEMENTED */
  } else if (biddyManagerType == BIDDYTYPETZBDD) {
    /* IMPLEMENTED */
  } else if ((biddyManagerType == BIDDYTYPEZBDD) ||
              (biddyManagerType == BIDDYTYPETZBDDC))
  {
    fprintf(stderr,"Biddy_CountMinterm: this GDD type is not supported, yet!\n");
    return 0.0;
  } else if ((biddyManagerType == BIDDYTYPEOFDD) || (biddyManagerType == BIDDYTYPEOFDDC) ||
              (biddyManagerType == BIDDYTYPEZFDD) || (biddyManagerType == BIDDYTYPEZFDDC) ||
              (biddyManagerType == BIDDYTYPETZFDD) || (biddyManagerType == BIDDYTYPETZFDDC))
  {
    fprintf(stderr,"Biddy_CountMinterm: this GDD type is not supported, yet!\n");
    return 0.0;
  } else {
    fprintf(stderr,"Biddy_CountMinterm: Unsupported GDD type!\n");
    return 0.0;
  }

  if (Biddy_IsNull(f)) return 0;
  if (f == biddyZero) return 0;

  var = 0;
  depvar = 0;
  if ((biddyManagerType == BIDDYTYPEOBDDC) || (biddyManagerType == BIDDYTYPEOBDD)) {
    var = BiddyCreateLocalInfo(MNG,f); /* returns the number of noticeable variables */
    depvar = var; /* number of dependent variables is the same as the number of noticeable variables */
  }
  else if ((biddyManagerType == BIDDYTYPEZBDDC) || (biddyManagerType == BIDDYTYPEZBDD)) {
    depvar = Biddy_Managed_DependentVariableNumber(MNG,f); /* for ZBDDs, this should be called before CreateLocalInfo! */
    var = BiddyCreateLocalInfo(MNG,f); /* returns the number of noticeable variables */
  }
  else if ((biddyManagerType == BIDDYTYPETZBDDC) || (biddyManagerType == BIDDYTYPETZBDD)) {
    depvar = Biddy_Managed_DependentVariableNumber(MNG,f); /* for TZBDDs, this should be called before CreateLocalInfo! */
    var = BiddyCreateLocalInfo(MNG,f); /* returns the number of noticeable variables */
  }

  /* nvars is the requested number of variables */
  if (nvars == 0) nvars = depvar;
  if (nvars < depvar) {
    nvars = depvar;
    fprintf(stderr,"WARNING (Biddy_Managed_CountMinterm): nvars < depvar\n");
  }

  if ((biddyManagerType == BIDDYTYPEOBDDC) || (biddyManagerType == BIDDYTYPEOBDD)) {
    mpz_init(max);
    mpz_ui_pow_ui(max,2,var); /* initialized using the number of noticeable variables */
    leftmost = FALSE; /* it is not used */
  }
  else if ((biddyManagerType == BIDDYTYPEZBDDC) || (biddyManagerType == BIDDYTYPEZBDD)) {
    mpz_init(max); /* it is not used */
    leftmost = FALSE;
  }
  else if ((biddyManagerType == BIDDYTYPETZBDDC) ||(biddyManagerType == BIDDYTYPETZBDD)) {
    mpz_init(max);
    mpz_ui_pow_ui(max,2,var); /* initialized using the number of noticeable variables */
    leftmost = FALSE; /* it is not used */
  }

  mpz_init(result);
  mintermCount(MNG,f,max,result,&leftmost); /* select all nodes except constant node */

  /* the obtained result considers the number of noticeable variables (var) */
  /* the result should be adapted  to consider the requested number of variables (nvar) */
  if ((biddyManagerType == BIDDYTYPEOBDDC) || (biddyManagerType == BIDDYTYPEOBDD)) {
    /* the requested number of variables cannot be smaller than the number of noticeable variables */
    if (nvars > var) {
      while (nvars > var) {
        mpz_mul_ui(result,result,2);
        nvars--;
      }
    }
  }
  else if ((biddyManagerType == BIDDYTYPEZBDDC) || (biddyManagerType == BIDDYTYPEZBDD)) {
    /* the requested number of variables cannot be greater than the number of noticeable variables */
    if (nvars < var) {
      while (nvars < var) {
        mpz_divexact_ui(result,result,2);
        nvars++;
      }
    }
  }
  else if ((biddyManagerType == BIDDYTYPETZBDDC) || (biddyManagerType == BIDDYTYPETZBDD)) {
    if (nvars > var) {
      while (nvars > var) {
        mpz_mul_ui(result,result,2);
        nvars--;
      }
    }
    else if (nvars < var) {
      while (nvars < var) {
        mpz_divexact_ui(result,result,2);
        nvars++;
      }
    }
  }

  resultd = mpz_get_d(result);
  mpz_clear(max);
  mpz_clear(result);

  /* all nodes except constant are selected */
  BiddyDeleteLocalInfo(MNG,f);

  return resultd;
}

#ifdef __cplusplus
}
#endif

/***************************************************************************//*!
\brief Function Biddy_Managed_DensityFunction calculates the ratio of the
       number of on-set minterms to the number of all minterms.

### Description
    If nvars == 0 then number of dependent variables is used.
### Side effects
### More info
    Macro Biddy_DensityFunction(f,nvars) is defined for use with anonymous
    manager.
*******************************************************************************/

#ifdef __cplusplus
extern "C" {
#endif

double
Biddy_Managed_DensityFunction(Biddy_Manager MNG, Biddy_Edge f,
                              unsigned int nvars)
{
  double n;
  double m;

  if (!MNG) MNG = biddyAnonymousManager;

  if (biddyManagerType == BIDDYTYPEOBDD) {
    /* IMPLEMENTED */
  } else if (biddyManagerType == BIDDYTYPEOBDDC) {
    /* IMPLEMENTED */
  } else if (biddyManagerType == BIDDYTYPEZBDDC) {
    /* IMPLEMENTED */
  } else if (biddyManagerType == BIDDYTYPETZBDD) {
    /* IMPLEMENTED */
  } else if ((biddyManagerType == BIDDYTYPEZBDD) ||
              (biddyManagerType == BIDDYTYPETZBDDC))
  {
    fprintf(stderr,"Biddy_DensityFunction: this GDD type is not supported, yet!\n");
    return 0.0;
  } else if ((biddyManagerType == BIDDYTYPEOFDD) || (biddyManagerType == BIDDYTYPEOFDDC) ||
              (biddyManagerType == BIDDYTYPEZFDD) || (biddyManagerType == BIDDYTYPEZFDDC) ||
              (biddyManagerType == BIDDYTYPETZFDD) || (biddyManagerType == BIDDYTYPETZFDDC))
  {
    fprintf(stderr,"Biddy_DensityFunction: this GDD type is not supported, yet!\n");
    return 0.0;
  } else {
    fprintf(stderr,"Biddy_DensityFunction: Unsupported GDD type!\n");
    return 0.0;
  }

  if (nvars == 0) {
    nvars = Biddy_Managed_DependentVariableNumber(MNG,f);
  }
  m = Biddy_Managed_CountMinterm(MNG,f,nvars);

  n = 1.0;
  while (nvars) {

    assert( n < 2 * n );

    n = 2 * n;
    nvars--;
  }

  return m/n;
}

#ifdef __cplusplus
}
#endif

/***************************************************************************//*!
\brief Function Biddy_Managed_DensityBDD calculates the ratio of the
       number of on-set minterms to the number of nodes.

### Description
    If nvars == 0 then number of dependent variables is used.
### Side effects
### More info
    Macro Biddy_DensityBDD(f,nvars) is defined for use with anonymous manager.
*******************************************************************************/

#ifdef __cplusplus
extern "C" {
#endif

double
Biddy_Managed_DensityBDD(Biddy_Manager MNG, Biddy_Edge f, unsigned int nvars)
{
  unsigned int n;
  double m;

  if (!MNG) MNG = biddyAnonymousManager;

  if (biddyManagerType == BIDDYTYPEOBDD) {
    /* IMPLEMENTED */
  } else if (biddyManagerType == BIDDYTYPEOBDDC) {
    /* IMPLEMENTED */
  } else if (biddyManagerType == BIDDYTYPEZBDDC) {
    /* IMPLEMENTED */
  } else if (biddyManagerType == BIDDYTYPETZBDD) {
    /* IMPLEMENTED */
  } else if ((biddyManagerType == BIDDYTYPEZBDD) ||
              (biddyManagerType == BIDDYTYPETZBDDC))
  {
    fprintf(stderr,"Biddy_DensityBDD: this GDD type is not supported, yet!\n");
    return 0.0;
  } else if ((biddyManagerType == BIDDYTYPEOFDD) || (biddyManagerType == BIDDYTYPEOFDDC) ||
              (biddyManagerType == BIDDYTYPEZFDD) || (biddyManagerType == BIDDYTYPEZFDDC) ||
              (biddyManagerType == BIDDYTYPETZFDD) || (biddyManagerType == BIDDYTYPETZFDDC))
  {
    fprintf(stderr,"Biddy_DensityBDD: this GDD type is not supported, yet!\n");
    return 0.0;
  } else {
    fprintf(stderr,"Biddy_DensityBDD: Unsupported GDD type!\n");
    return 0.0;
  }

  if (nvars == 0) {
    Biddy_Managed_NodeVarNumber(MNG,f,&n,&nvars);
  } else {
    n = Biddy_Managed_NodeNumber(MNG,f);
  }

  m = Biddy_Managed_CountMinterm(MNG,f,nvars);
  return m/n;
}

#ifdef __cplusplus
}
#endif

/***************************************************************************//*!
\brief Function Biddy_Managed_ReadMemoryInUse report memory consumption
       of main data strucutures (nodes, node table, variable table,
       ordering table, formula table, ITE cache, EA cache, RC cache) in bytes.

### Description
### Side effects
### More info
    Macro Biddy_ReadMemoryInUse() is defined for use with anonymous manager.
*******************************************************************************/

#ifdef __cplusplus
extern "C" {
#endif

unsigned long long int
Biddy_Managed_ReadMemoryInUse(Biddy_Manager MNG) {
  Biddy_Variable v;
  unsigned i;
  BiddyCacheList *c;
  unsigned long long int n;

  if (!MNG) MNG = biddyAnonymousManager;
  
  n = 0;

  /* nodes */
  n += biddyNodeTable.generated * sizeof(BiddyNode);

  /* node table */
  n += sizeof(BiddyNodeTable) +
       biddyNodeTable.blocknumber * sizeof(BiddyNode *) +
       (biddyNodeTable.size+2) * sizeof(BiddyNode *);

  /* variable table */
  n += sizeof(BiddyVariableTable) +
       biddyVariableTable.size * sizeof(BiddyVariable);
  /* n += biddyVariableTable.size * sizeof(BiddyLookupVariable); */ /* currently not used */
  for (v=0; v<biddyVariableTable.num; v++) {
    if (biddyVariableTable.table[v].name) {
      n += strlen(biddyVariableTable.table[v].name) + 1;
    }
  }

  /* ordering table */
  n += sizeof(BiddyOrderingTable);

  /* formula table */
  n += sizeof(BiddyFormulaTable) +
       biddyFormulaTable.size * sizeof(BiddyFormula);
  if (biddyFormulaTable.deletedName) {
    n += strlen(biddyFormulaTable.deletedName) + 1;
  }
  for (i=0; i<biddyFormulaTable.size; i++) {
    if (biddyFormulaTable.table[i].name) {
      n += strlen(biddyFormulaTable.table[i].name) + 1;
    }
  }

  /* cache list*/
  c = biddyCacheList;
  while (c) {
    n += sizeof(BiddyCacheList);
    c = c->next;
  }

  /* OP cache */
  n += sizeof(BiddyOp3CacheTable) +
       (biddyOPCache.size+1) * sizeof(BiddyOp3Cache);
  n += 4 * sizeof(unsigned long long int); /* counters */

  /* EA cache */
  n += sizeof(BiddyOp3CacheTable) +
       (biddyEACache.size+1) * sizeof(BiddyOp3Cache);
  n += 4 * sizeof(unsigned long long int); /* counters */

  /* RC cache */
  n += sizeof(BiddyOp3CacheTable) +
       (biddyRCCache.size+1) * sizeof(BiddyOp3Cache);
  n += 4 * sizeof(unsigned long long int); /* counters */
  
  return n;
}

#ifdef __cplusplus
}
#endif

/***************************************************************************//*!
\brief Function Biddy_Managed_PrintInfo prepare a file with stats.

### Description
### Side effects
### More info
    Macro Biddy_PrintInfo(f) is defined for use with anonymous manager.
*******************************************************************************/

#ifdef __cplusplus
extern "C" {
#endif

void
Biddy_Managed_PrintInfo(Biddy_Manager MNG, FILE *f) {

  if (!MNG) MNG = biddyAnonymousManager;

  /* we have problems with passing stdout between mingw-based main and visual-studio-based dll */
  if (!f) f = stdout;

  fprintf(f,"**** System parameters ****\n");
  fprintf(f,"BDD TYPE: %s\n",Biddy_Managed_GetManagerName(MNG));
  fprintf(f,"UINTPTRSIZE = %u bits\n",(unsigned int) UINTPTRSIZE);
  fprintf(f,"sizeof(clock_t): %u bytes\n",(unsigned int)sizeof(clock_t));
  fprintf(f,"sizeof(BiddyNode *): %u bytes\n",(unsigned int)sizeof(BiddyNode *));
  fprintf(f,"sizeof(BiddyNode): %u bytes\n",(unsigned int)sizeof(BiddyNode));
  fprintf(f,"sizeof(BiddyVariable): %u bytes\n",(unsigned int)sizeof(BiddyVariable));
  fprintf(f,"sizeof(BiddyFormula): %u bytes\n",(unsigned int)sizeof(BiddyFormula));
  fprintf(f,"sizeof(BiddyOp3Cache): %u bytes\n",(unsigned int)sizeof(BiddyOp3Cache));
  fprintf(f,"sizeof(BiddyOrderingTable): %u bytes\n",(unsigned int)sizeof(BiddyOrderingTable));
  fprintf(f,"**** Biddy modifiable parameters ****\n");
  fprintf(f,"Limit for number of variables: %u\n",biddyVariableTable.size);
  fprintf(f,"Initial number of buckets in Node table: %u\n",biddyNodeTable.initsize+1);
  fprintf(f,"Number of nodes in the initial memory block: %u\n",biddyNodeTable.initblocksize);
  fprintf(f,"Initial OP cache size: %u\n",biddyOPCache.size);
  fprintf(f,"Initial EA cache size: %u\n",biddyEACache.size);
  fprintf(f,"Initial RC cache size: %u\n",biddyRCCache.size);
  fprintf(f,"Initial Replace cache size: %u\n",biddyReplaceCache.size);
  fprintf(f,"**** Biddy non-modifiable parameters ****\n");
#ifdef MINGW
  fprintf(f, "Memory in use: %I64u bytes\n",Biddy_Managed_ReadMemoryInUse(MNG));
#else
  fprintf(f, "Memory in use: %llu bytes\n",Biddy_Managed_ReadMemoryInUse(MNG));
#endif
  fprintf(f,"Number of memory blocks: %u\n",biddyNodeTable.blocknumber);
  fprintf(f,"Number of nodes in the last memory block: %u\n",biddyNodeTable.blocksize);
  fprintf(f,"Number of variables: %u\n",biddyVariableTable.num);
  fprintf(f,"Number of formulae: %u\n",biddyFormulaTable.size); 
  fprintf(f,"Peak number of BDD nodes: %u\n",biddyNodeTable.generated);
  fprintf(f,"Peak number of live BDD nodes: %u\n",biddyNodeTable.max);
  fprintf(f,"Number of live BDD nodes: %u\n",biddyNodeTable.num);
  fprintf(f,"Garbage collections so far: %u (node table resizing was used in %u of them)\n",biddyNodeTable.garbage,biddyNodeTable.nodetableresize);
#ifdef BIDDYEXTENDEDSTATS_YES
#ifdef MINGW
  fprintf(f,"Total number of obsolete nodes deleted: %I64u\n",Biddy_Managed_NodeTableGCObsoleteNumber(MNG));
#else
  fprintf(f,"Total number of obsolete nodes deleted: %llu\n",Biddy_Managed_NodeTableGCObsoleteNumber(MNG));
#endif
#endif
  fprintf(f,"Total time for garbage collections so far: %.3fs\n",biddyNodeTable.gctime / (1.0 * CLOCKS_PER_SEC));
  fprintf(f,"Dynamic reorderings so far: %u\n",biddyNodeTable.sifting);
  fprintf(f,"Node swaps in dynamic reorderings: %u\n",biddyNodeTable.swap);
  fprintf(f,"Total time for dynamic reorderings so far: %.3fs\n",biddyNodeTable.drtime / (1.0 * CLOCKS_PER_SEC));
  fprintf(f,"**** Node Table stats ****\n");
  fprintf(f,"Number of buckets in Node table: %u\n",biddyNodeTable.size+1);
  fprintf(f,"Used buckets in Node table: %u (%.2f%%)\n",
                                   Biddy_Managed_ListUsed(MNG),
                                   (100.0*Biddy_Managed_ListUsed(MNG)/(biddyNodeTable.size+1)));
  fprintf(f,"Max bucket's size in Node table: %u\n",Biddy_Managed_ListMaxLength(MNG));
  fprintf(f,"Avg bucket's size in Node table: %f\n",Biddy_Managed_ListAvgLength(MNG));
#ifdef BIDDYEXTENDEDSTATS_YES
#ifdef MINGW
  fprintf(f, "Number of ITE calls: %u (internal and recursive calls: %I64u)\n",biddyNodeTable.funite,biddyNodeTable.iterecursive);
  fprintf(f, "Number of AND or OR calls: %u (internal and recursive calls: %I64u)\n",biddyNodeTable.funandor,biddyNodeTable.andorrecursive);
  fprintf(f, "Number of XOR calls: %u (internal and recursive calls: %I64u)\n",biddyNodeTable.funxor,biddyNodeTable.xorrecursive);
  fprintf(f, "Number of node table FOA calls: %I64u\n",biddyNodeTable.foa);
  fprintf(f, "Number of node table insertions: %I64u\n",biddyNodeTable.add);
  fprintf(f, "Number of compared nodes: %I64u (%.2f per findNodeTable call)\n",
                                   biddyNodeTable.compare,
                                   biddyNodeTable.find ?
                                   (1.0*biddyNodeTable.compare/biddyNodeTable.find):0);
#else
  fprintf(f, "Number of ITE calls: %u (internal and recursive calls: %llu)\n",biddyNodeTable.funite,biddyNodeTable.iterecursive);
  fprintf(f, "Number of AND and OR calls: %u (internal and recursive calls: %llu)\n",biddyNodeTable.funandor,biddyNodeTable.andorrecursive);
  fprintf(f, "Number of XOR calls: %u (internal and recursive calls: %llu)\n",biddyNodeTable.funxor,biddyNodeTable.xorrecursive);
  fprintf(f, "Number of node table FOA calls: %llu\n",biddyNodeTable.foa);
  fprintf(f, "Number of node table insertions: %llu\n",biddyNodeTable.add);
  fprintf(f, "Number of compared nodes: %llu (%.2f per findNodeTable call)\n",
                                   biddyNodeTable.compare,
                                   biddyNodeTable.find ?
                                   (1.0*biddyNodeTable.compare/biddyNodeTable.find):0);
#endif
#endif
  fprintf(f,"**** OP cache stats ****\n");
  fprintf(f,"Size of OP cache: %u\n",biddyOPCache.size);
#ifdef MINGW
  fprintf(f, "Number of OP cache look-ups: %I64u\n",*(biddyOPCache.search));
  fprintf(f, "Number of OP cache hits: %I64u (%.2f%% of all calls)\n",
                                   *(biddyOPCache.find),
                                   *(biddyOPCache.search) ?
                                   (100.0*(*(biddyOPCache.find))/(*(biddyOPCache.search))):0);
#ifdef BIDDYEXTENDEDSTATS_YES
  fprintf(f,"Number of OP cache insertions: %I64u (%.2f%% of all calls)\n",
                                   *(biddyOPCache.insert),
                                   *(biddyOPCache.search) ?
                                   (100.0*(*(biddyOPCache.insert))/(*(biddyOPCache.search))):0);
  fprintf(f, "Number of OP cache collisions: %I64u (%.2f%% of all insertions, %.2f%% of all calls)\n",
                                   *(biddyOPCache.overwrite),
                                   (*(biddyOPCache.insert)) ?
                                   (100.0*(*(biddyOPCache.overwrite))/(*(biddyOPCache.insert))):0,
                                   *(biddyOPCache.search) ?
                                   (100.0*(*(biddyOPCache.overwrite))/(*(biddyOPCache.search))):0);
#endif
#else
  fprintf(f, "Number of OP cache look-ups: %llu\n",*(biddyOPCache.search));
  fprintf(f, "Number of OP cache hits: %llu (%.2f%% of all calls)\n",
                                   *(biddyOPCache.find),
                                   *(biddyOPCache.search) ?
                                   (100.0*(*(biddyOPCache.find))/(*(biddyOPCache.search))):0);
#ifdef BIDDYEXTENDEDSTATS_YES
  fprintf(f,"Number of OP cache insertions: %llu (%.2f%% of all calls)\n",
                                   *(biddyOPCache.insert),
                                   *(biddyOPCache.search) ?
                                   (100.0*(*(biddyOPCache.insert))/(*(biddyOPCache.search))):0);
  fprintf(f, "Number of OP cache collisions: %llu (%.2f%% of all insertions, %.2f%% of all calls)\n",
                                   *(biddyOPCache.overwrite),
                                   (*(biddyOPCache.insert)) ?
                                   (100.0*(*(biddyOPCache.overwrite))/(*(biddyOPCache.insert))):0,
                                   *(biddyOPCache.search) ?
                                   (100.0*(*(biddyOPCache.overwrite))/(*(biddyOPCache.search))):0);
#endif
#endif
  fprintf(f,"**** EA cache stats ****\n");
  fprintf(f,"Size of EA cache: %u\n",biddyEACache.size);
#ifdef MINGW
  fprintf(f, "Number of EA cache look-ups: %I64u\n",*(biddyEACache.search));
  fprintf(f, "Number of EA cache hits: %I64u (%.2f%% of all calls)\n",
                                   *(biddyEACache.find),
                                   (*(biddyEACache.search)) ?
                                   (100.0*(*(biddyEACache.find))/(*(biddyEACache.search))):0);
#ifdef BIDDYEXTENDEDSTATS_YES
  fprintf(f,"Number of EA cache insertions: %I64u (%.2f%% of all calls)\n",
                                   *(biddyEACache.insert),
                                   (*(biddyEACache.search)) ?
                                   (100.0*(*(biddyEACache.insert))/(*(biddyEACache.search))):0);
  fprintf(f, "Number of EA cache collisions: %I64u (%.2f%% of all insertions, %.2f%% of all calls)\n",
                                   *(biddyEACache.overwrite),
                                   (*(biddyEACache.insert)) ?
                                   (100.0*(*(biddyEACache.overwrite))/(*(biddyEACache.insert))):0,
                                   (*(biddyEACache.search)) ?
                                   (100.0*(*(biddyEACache.overwrite))/(*(biddyEACache.search))):0);
#endif
#else
  fprintf(f, "Number of EA cache look-ups: %llu\n",*(biddyEACache.search));
  fprintf(f, "Number of EA cache hits: %llu (%.2f%% of all calls)\n",
                                   *(biddyEACache.find),
                                   (*(biddyEACache.search)) ?
                                   (100.0*(*(biddyEACache.find))/(*(biddyEACache.search))):0);
#ifdef BIDDYEXTENDEDSTATS_YES
  fprintf(f,"Number of EA cache insertions: %llu (%.2f%% of all calls)\n",
                                   *(biddyEACache.insert),
                                   (*(biddyEACache.search)) ?
                                   (100.0*(*(biddyEACache.insert))/(*(biddyEACache.search))):0);
  fprintf(f, "Number of EA cache collisions: %llu (%.2f%% of all insertions, %.2f%% of all calls)\n",
                                   *(biddyEACache.overwrite),
                                   (*(biddyEACache.insert)) ?
                                   (100.0*(*(biddyEACache.overwrite))/(*(biddyEACache.insert))):0,
                                   (*(biddyEACache.search)) ?
                                   (100.0*(*(biddyEACache.overwrite))/(*(biddyEACache.search))):0);
#endif
#endif
  fprintf(f,"**** RC cache stats ****\n");
  fprintf(f,"Size of RC cache: %u\n",biddyRCCache.size);
#ifdef MINGW
  fprintf(f, "Number of RC cache look-ups: %I64u\n",*(biddyRCCache.search));
  fprintf(f, "Number of RC cache hits: %I64u (%.2f%% of all calls)\n",
                                   *(biddyRCCache.find),
                                   (*(biddyRCCache.search)) ?
                                   (100.0*(*(biddyRCCache.find))/(*(biddyRCCache.search))):0);
#ifdef BIDDYEXTENDEDSTATS_YES
  fprintf(f,"Number of RC cache insertions: %I64u (%.2f%% of all calls)\n",
                                   *(biddyRCCache.insert),
                                   (*(biddyRCCache.search)) ?
                                   (100.0*(*(biddyRCCache.insert))/(*(biddyRCCache.search))):0);
  fprintf(f, "Number of RC cache collisions: %I64u (%.2f%% of all insertions, %.2f%% of all calls)\n",
                                   *(biddyRCCache.overwrite),
                                   (*(biddyRCCache.insert)) ?
                                   (100.0*(*(biddyRCCache.overwrite))/(*(biddyRCCache.insert))):0,
                                   (*(biddyRCCache.search)) ?
                                   (100.0*(*(biddyRCCache.overwrite))/(*(biddyRCCache.search))):0);
#endif
#else
  fprintf(f, "Number of RC cache look-ups: %llu\n",*(biddyRCCache.search));
  fprintf(f, "Number of RC cache hits: %llu (%.2f%% of all calls)\n",
                                   *(biddyRCCache.find),
                                   (*(biddyRCCache.search)) ?
                                   (100.0*(*(biddyRCCache.find))/(*(biddyRCCache.search))):0);
#ifdef BIDDYEXTENDEDSTATS_YES
  fprintf(f,"Number of RC cache insertions: %llu (%.2f%% of all calls)\n",
                                   *(biddyRCCache.insert),
                                   (*(biddyRCCache.search)) ?
                                   (100.0*(*(biddyRCCache.insert))/(*(biddyRCCache.search))):0);
  fprintf(f, "Number of RC cache collisions: %llu (%.2f%% of all insertions, %.2f%% of all calls)\n",
                                   *(biddyRCCache.overwrite),
                                   (*(biddyRCCache.insert)) ?
                                   (100.0*(*(biddyRCCache.overwrite))/(*(biddyRCCache.insert))):0,
                                   (*(biddyRCCache.search)) ?
                                   (100.0*(*(biddyRCCache.overwrite))/(*(biddyRCCache.search))):0);
#endif
#endif
  fprintf(f,"**** Replace cache stats ****\n");
  fprintf(f,"Size of Replace cache: %u\n",biddyReplaceCache.size);
#ifdef MINGW
  fprintf(f, "Number of Replace cache look-ups: %I64u\n",*(biddyReplaceCache.search));
  fprintf(f, "Number of Replace cache hits: %I64u (%.2f%% of all calls)\n",
                                   *(biddyReplaceCache.find),
                                   (*(biddyReplaceCache.search)) ?
                                   (100.0*(*(biddyReplaceCache.find))/(*(biddyReplaceCache.search))):0);
#ifdef BIDDYEXTENDEDSTATS_YES
  fprintf(f,"Number of Replace cache insertions: %I64u (%.2f%% of all calls)\n",
                                   *(biddyReplaceCache.insert),
                                   (*(biddyReplaceCache.search)) ?
                                   (100.0*(*(biddyReplaceCache.insert))/(*(biddyReplaceCache.search))):0);
  fprintf(f, "Number of Replace cache collisions: %I64u (%.2f%% of all insertions, %.2f%% of all calls)\n",
                                   *(biddyReplaceCache.overwrite),
                                   (*(biddyReplaceCache.insert)) ?
                                   (100.0*(*(biddyReplaceCache.overwrite))/(*(biddyReplaceCache.insert))):0,
                                   (*(biddyReplaceCache.search)) ?
                                   (100.0*(*(biddyReplaceCache.overwrite))/(*(biddyReplaceCache.search))):0);
#endif
#else
  fprintf(f, "Number of Replace cache look-ups: %llu\n",*(biddyReplaceCache.search));
  fprintf(f, "Number of Replace cache hits: %llu (%.2f%% of all calls)\n",
                                   *(biddyReplaceCache.find),
                                   (*(biddyReplaceCache.search)) ?
                                   (100.0*(*(biddyReplaceCache.find))/(*(biddyReplaceCache.search))):0);
#ifdef BIDDYEXTENDEDSTATS_YES
  fprintf(f,"Number of Replace cache insertions: %llu (%.2f%% of all calls)\n",
                                   *(biddyReplaceCache.insert),
                                   (*(biddyReplaceCache.search)) ?
                                   (100.0*(*(biddyReplaceCache.insert))/(*(biddyReplaceCache.search))):0);
  fprintf(f, "Number of Replace cache collisions: %llu (%.2f%% of all insertions, %.2f%% of all calls)\n",
                                   *(biddyReplaceCache.overwrite),
                                   (*(biddyReplaceCache.insert)) ?
                                   (100.0*(*(biddyReplaceCache.overwrite))/(*(biddyReplaceCache.insert))):0,
                                   (*(biddyReplace¸Cache.search)) ?
                                   (100.0*(*(biddyReplaceCache.overwrite))/(*(biddyReplaceCache.search))):0);
#endif
#endif
  fprintf(f,"**** END OF STATS ****\n");
}

#ifdef __cplusplus
}
#endif

/*----------------------------------------------------------------------------*/
/* Definition of internal functions                                           */
/*----------------------------------------------------------------------------*/

/*******************************************************************************
\brief Function BiddyNodeNumber.

### Description
### Side effects
    Constant node must be selected! Function will select all other nodes.
### More info
*******************************************************************************/

void
BiddyNodeNumber(Biddy_Manager MNG, Biddy_Edge f, unsigned int *n)
{
  if (!Biddy_Managed_IsSelected(MNG,f)) {
    Biddy_Managed_SelectNode(MNG,f);
    (*n)++;
    BiddyNodeNumber(MNG,BiddyE(f),n);
    BiddyNodeNumber(MNG,BiddyT(f),n);
  }
}


/*******************************************************************************
\brief Function BiddyComplementedEdgeNumber.

### Description
### Side effects
    Constant node must be selected! Function will select all other nodes.
### More info
*******************************************************************************/

void
BiddyComplementedEdgeNumber(Biddy_Manager MNG, Biddy_Edge f, unsigned int *n)
{
  if (!Biddy_Managed_IsSelected(MNG,f)) {
    Biddy_Managed_SelectNode(MNG,f);
    if (Biddy_GetMark(BiddyE(f))) (*n)++;
    if (Biddy_GetMark(BiddyT(f))) (*n)++;
    BiddyComplementedEdgeNumber(MNG,BiddyE(f),n);
    BiddyComplementedEdgeNumber(MNG,BiddyT(f),n);
  }
}

/*******************************************************************************
\brief Function BiddyNodeVarNumber.

### Description
### Side effects
    Constant node must be selected! Function will select all other nodes.
    Function will select all dependent variables (for OBDDs, present variables
    are the same as dependent variables, for ZBDDs and TZBDDs this is not true).
    IMPORTANT: For ZBDDs, variables above the top variable (which are always
    all dependent) are not selected!
### More info
*******************************************************************************/

void
BiddyNodeVarNumber(Biddy_Manager MNG, Biddy_Edge f, unsigned int *n)
{
  Biddy_Edge e,t;
  Biddy_Variable v,w;


  if ((biddyManagerType == BIDDYTYPEOBDDC) || (biddyManagerType == BIDDYTYPEOBDD))
  {
    if (Biddy_Managed_IsSelected(MNG,f)) {
      /* ALREADY VISITED */
      return;
    } else {
      assert( BiddyIsSmaller(BiddyV(f),BiddyV(BiddyE(f))) );
      assert( BiddyIsSmaller(BiddyV(f),BiddyV(BiddyT(f))) );
      Biddy_Managed_SelectNode(MNG,f);
      (*n)++;
      v = BiddyV(f);
      e = BiddyE(f); /* here, mark is not important */
      t = BiddyT(f); /* here, mark is not important */
      biddyVariableTable.table[v].selected = TRUE;
      BiddyNodeVarNumber(MNG,e,n);
      BiddyNodeVarNumber(MNG,t,n);
    }
  }

  else if ((biddyManagerType == BIDDYTYPEZBDDC) || (biddyManagerType == BIDDYTYPEZBDD))
  {
    if (Biddy_IsConstant(f)) return;
    if (Biddy_Managed_IsSelected(MNG,f)) {
      if (BiddyIsSelectedNP(f)) {
        /* ALREADY VISITED WITH THE SAME MARK */
        return;
      } else {
        /* ALREADY VISITED, BUT WITH DIFFERENT MARK */
        BiddySelectNP(f);
      }
    } else {
      /* NOT VISITED */
      assert( BiddyIsSmaller(BiddyV(f),BiddyV(BiddyE(f))) );
      assert( BiddyIsSmaller(BiddyV(f),BiddyV(BiddyT(f))) );
      Biddy_Managed_SelectNode(MNG,f);
      BiddySelectNP(f);
      (*n)++;
    }
    v = BiddyV(f);
    e = Biddy_Managed_TransferMark(MNG,BiddyE(f),Biddy_GetMark(f),TRUE); /* TRUE = left */
    t = Biddy_Managed_TransferMark(MNG,BiddyT(f),Biddy_GetMark(f),FALSE); /* FALSE = right */
    if (e != t) {
      biddyVariableTable.table[v].selected = TRUE;
    }
    v = biddyVariableTable.table[v].next;
    w = BiddyV(t); /* for ZBDD, t cannot be biddyZero */
    if ((e != biddyZero) && BiddyIsSmaller(w,BiddyV(e))) w = BiddyV(e);
    while ((v != 0) && (v != w)) {
      biddyVariableTable.table[v].selected = TRUE;
      v = biddyVariableTable.table[v].next;
    }
    BiddyNodeVarNumber(MNG,e,n);
    BiddyNodeVarNumber(MNG,t,n);
  }

  else if ((biddyManagerType == BIDDYTYPETZBDDC) || (biddyManagerType == BIDDYTYPETZBDD))
  {
    if (Biddy_Managed_IsSelected(MNG,f)) {
      /* ALREADY VISITED */
      return;
    } else {
      
      assert( (Biddy_GetTag(f) == BiddyV(f)) || BiddyIsSmaller(Biddy_GetTag(f),BiddyV(f)) );
      assert( BiddyIsSmaller(BiddyV(f),BiddyV(BiddyE(f))) );
      assert( BiddyIsSmaller(BiddyV(f),BiddyV(BiddyT(f))) );
      assert( (Biddy_GetTag(BiddyE(f)) == BiddyV(BiddyE(f))) || BiddyIsSmaller(Biddy_GetTag(BiddyE(f)),BiddyV(BiddyE(f))) );
      assert( (Biddy_GetTag(BiddyT(f)) == BiddyV(BiddyT(f))) || BiddyIsSmaller(Biddy_GetTag(BiddyT(f)),BiddyV(BiddyT(f))) );

      /* DEBUGGING */
      /*
      if (BiddyIsSmaller(BiddyV(f),Biddy_GetTag(f)) ||
          !BiddyIsSmaller(BiddyV(f),BiddyV(BiddyE(f))) ||
          !BiddyIsSmaller(BiddyV(f),BiddyV(BiddyT(f))))
      {
        printf("ERROR:");
        printf("<\"%s\",",Biddy_Managed_GetTopVariableName(MNG,f));
        if (!Biddy_IsNull(BiddyE(f)) && Biddy_GetMark(BiddyE(f))) printf("*");
        if (Biddy_IsNull(BiddyE(f))) printf("NULL,");
          else printf("<\"%s\">\"%s\",",Biddy_Managed_GetVariableName(MNG,Biddy_GetTag(BiddyE(f))),Biddy_Managed_GetTopVariableName(MNG,BiddyE(f)));
        if (!Biddy_IsNull(BiddyT(f)) && Biddy_GetMark(BiddyT(f))) printf("*");
        if (Biddy_IsNull(BiddyT(f))) printf("NULL>");
          else printf("<\"%s\">\"%s\">",Biddy_Managed_GetVariableName(MNG,Biddy_GetTag(BiddyT(f))),Biddy_Managed_GetTopVariableName(MNG,BiddyT(f)));
        printf("@%u",BiddyN(f)->expiry);
        printf("\n");
      }
      assert( (Biddy_GetTag(f) == BiddyV(f)) || BiddyIsSmaller(Biddy_GetTag(f),BiddyV(f)) );
      assert( BiddyIsSmaller(BiddyV(f),BiddyV(BiddyE(f))) );
      assert( BiddyIsSmaller(BiddyV(f),BiddyV(BiddyT(f))) );
      */

      Biddy_Managed_SelectNode(MNG,f);
      (*n)++;
      v = BiddyV(f);
      e = BiddyE(f); /* marks are not used */
      t = BiddyT(f); /* marks are not used */
      if (e != t) {
        biddyVariableTable.table[v].selected = TRUE;
      }

      /* DEBUGGING */
      /*
      if (BiddyIsSmaller(BiddyV(e),Biddy_GetTag(e))) {
        printf("ERROR:");
        printf("<\"%s\",",Biddy_Managed_GetTopVariableName(MNG,f));
        if (!Biddy_IsNull(e) && Biddy_GetMark(e)) printf("*");
        if (Biddy_IsNull(e)) printf("NULL,");
          else printf("<\"%s\">\"%s\",",Biddy_Managed_GetVariableName(MNG,Biddy_GetTag(e)),Biddy_Managed_GetTopVariableName(MNG,e));
        if (!Biddy_IsNull(t) && Biddy_GetMark(t)) printf("*");
        if (Biddy_IsNull(t)) printf("NULL>");
          else printf("<\"%s\">\"%s\">",Biddy_Managed_GetVariableName(MNG,Biddy_GetTag(t)),Biddy_Managed_GetTopVariableName(MNG,t));
        printf("@%u",BiddyN(f)->expiry);
        printf("\n");
      }
      assert ( (Biddy_GetTag(e) == BiddyV(e)) || BiddyIsSmaller(Biddy_GetTag(e),BiddyV(e)) );
      */

      w = Biddy_GetTag(e);
      while (w != BiddyV(e)) {
        biddyVariableTable.table[w].selected = TRUE;
        w = biddyVariableTable.table[w].next;
      }

      /* DEBUGGING */
      /*
      if (BiddyIsSmaller(BiddyV(t),Biddy_GetTag(t))) {
        printf("ERROR:");
        printf("<\"%s\",",Biddy_Managed_GetTopVariableName(MNG,f));
        if (!Biddy_IsNull(e) && Biddy_GetMark(e)) printf("*");
        if (Biddy_IsNull(e)) printf("NULL,");
          else printf("<\"%s\">\"%s\",",Biddy_Managed_GetVariableName(MNG,Biddy_GetTag(e)),Biddy_Managed_GetTopVariableName(MNG,e));
        if (!Biddy_IsNull(t) && Biddy_GetMark(t)) printf("*");
        if (Biddy_IsNull(t)) printf("NULL>");
          else printf("<\"%s\">\"%s\">",Biddy_Managed_GetVariableName(MNG,Biddy_GetTag(t)),Biddy_Managed_GetTopVariableName(MNG,t));
        printf("@%u",BiddyN(f)->expiry);
        printf("\n");
      }
      assert ( (Biddy_GetTag(t) == BiddyV(t)) || BiddyIsSmaller(Biddy_GetTag(t),BiddyV(t)) );
      */

      w = Biddy_GetTag(t);
      while (w != BiddyV(t)) {
        biddyVariableTable.table[w].selected = TRUE;
        w = biddyVariableTable.table[w].next;
      }
      BiddyNodeVarNumber(MNG,e,n);
      BiddyNodeVarNumber(MNG,t,n);
    }
  }

  else if ((biddyManagerType == BIDDYTYPEOFDD) || (biddyManagerType == BIDDYTYPEOFDDC) ||
              (biddyManagerType == BIDDYTYPEZFDD) || (biddyManagerType == BIDDYTYPEZFDDC) ||
              (biddyManagerType == BIDDYTYPETZFDD) || (biddyManagerType == BIDDYTYPETZFDDC))
  {
    fprintf(stderr,"BiddyNodeVarNumber: this GDD type is not supported, yet!\n");
    return;
  }
  
  else {
    fprintf(stderr,"BiddyNodeVarNumber: Unsupported GDD type!\n");
    return;
  }

}

/*----------------------------------------------------------------------------*/
/* Definition of static functions                                             */
/*----------------------------------------------------------------------------*/

/*******************************************************************************
\brief Function MaxLevel.

### Description
### Side effects
### More info
*******************************************************************************/

static void
MaxLevel(Biddy_Edge f, unsigned int *max, unsigned int i)
{
  i++;
  if (i > *max) *max = i;
  if (!Biddy_IsNull(BiddyE(f))) MaxLevel(BiddyE(f),max,i);
  if (!Biddy_IsNull(BiddyT(f))) MaxLevel(BiddyT(f),max,i);
}

/*******************************************************************************
\brief Function AvgLevel.

### Description
### Side effects
### More info
*******************************************************************************/

static void
AvgLevel(Biddy_Edge f, float *sum, unsigned int *n, unsigned int i)
{
  i++;
  (*n)++;
  (*sum) = (*sum) + i;
  if (!Biddy_IsNull(BiddyE(f))) AvgLevel(BiddyE(f),sum,n,i);
  if (!Biddy_IsNull(BiddyT(f))) AvgLevel(BiddyT(f),sum,n,i);
}

/*******************************************************************************
\brief Function pathCount.

### Description
### Side effects
    Function will select all nodes except constant node.
    TO DO: implement this using GNU Multiple Precision Arithmetic Library (GMP).
### More info
*******************************************************************************/

static void
pathCount(Biddy_Manager MNG, Biddy_Edge f,  unsigned long long int *c1,
          unsigned long long int *c0, Biddy_Boolean *leftmost)
{
  unsigned long long int r1,r0;

  assert( f != NULL );

  assert((
    (biddyManagerType == BIDDYTYPEOBDD) ||
    (biddyManagerType == BIDDYTYPEOBDDC) ||
    (biddyManagerType == BIDDYTYPEZBDDC) ||
    (biddyManagerType == BIDDYTYPETZBDD)
  ));

  /* DEBUGGING */
  /*
  printf("pathCount: f = %s (%p), c1 = %llu, c0 = %llu\n",Biddy_Managed_GetTopVariableName(MNG,f),f,*c1,*c0);
  */

  /* For OBDDs, c1 is number of ones, c0 is number of zeros, leftmost is not used */
  /* For ZBDDs, c1 is number of ones, c0 is not used, leftmost is the leftmost value */
  /* For TZBDDs, c1 is number of ones, c0 is not used, leftmost is not used */
  if (f == biddyZero) {
    *c1 = 0;
    *c0 = 1;
    *leftmost = FALSE;
    return;
  }
  if (Biddy_IsConstant(f)) {
    *c1 = 1;
    *c0 = 0;
    *leftmost = TRUE;
    return;
  }

  if (Biddy_Managed_IsSelected(MNG,f)) {
    /* ALREADY VISITED */
    if (Biddy_GetMark(f)) {
      if ((biddyManagerType == BIDDYTYPEOBDDC) || (biddyManagerType == BIDDYTYPEOBDD)) {
        *c1 = BiddyGetPath0Count(f); /* positive edge is stored but actual edge is returned */
        *c0 = BiddyGetPath1Count(f); /* positive edge is stored but actual edge is returned */
      }
      else if ((biddyManagerType == BIDDYTYPEZBDDC) || (biddyManagerType == BIDDYTYPEZBDD)) {
        *c1 = BiddyGetPath1Count(f); /* positive edge is stored */
        *leftmost = !(BiddyGetLeftmost(f)); /* actual edge is returned */
        if (*leftmost) {
          (*c1)++; /* actual edge is returned */
        } else {
          (*c1)--; /* actual edge is returned */
        }
      }
      else if ((biddyManagerType == BIDDYTYPETZBDDC) || (biddyManagerType == BIDDYTYPETZBDD)) {
        /* COMPLEMENTED EDGES ARE NOT USED FOR TZBDDs */
      }
    } else {
      if ((biddyManagerType == BIDDYTYPEOBDDC) || (biddyManagerType == BIDDYTYPEOBDD)) {
        *c1 = BiddyGetPath1Count(f);
        *c0 = BiddyGetPath0Count(f);
      }
      else if ((biddyManagerType == BIDDYTYPEZBDDC) || (biddyManagerType == BIDDYTYPEZBDD)) {
        *c1 = BiddyGetPath1Count(f);
        *leftmost = BiddyGetLeftmost(f);
      }
      else if ((biddyManagerType == BIDDYTYPETZBDDC) || (biddyManagerType == BIDDYTYPETZBDD)) {
        *c1 = BiddyGetPath1Count(f);
      }
    }
    return;
  }

  /* NOT VISITED, YET */
  Biddy_Managed_SelectNode(MNG,f);

  /* COUNT PATHS FOR BOTH SUCCESSORS */
  /* because of parameter leftmost it is important to calculate BiddyT first! */
  pathCount(MNG,BiddyT(f),c1,c0,leftmost);
  pathCount(MNG,BiddyE(f),&r1,&r0,leftmost);
  *c1 += r1;
  *c0 += r0;

  /* STORE PATHS INTO LOCALINFO */
  if ((biddyManagerType == BIDDYTYPEOBDDC) || (biddyManagerType == BIDDYTYPEOBDD)) {
    BiddySetPath1Count(f,*c1); /* positive edge is stored */
    BiddySetPath0Count(f,*c0); /* positive edge is stored */
    if (Biddy_GetMark(f)) {
      *c1 = BiddyGetPath0Count(f); /* actual edge is returned */
      *c0 = BiddyGetPath1Count(f); /* actual edge is returned */
    }
  }
  else if ((biddyManagerType == BIDDYTYPEZBDDC) || (biddyManagerType == BIDDYTYPEZBDD)) {
    BiddySetPath1Count(f,*c1); /* positive edge is stored */
    BiddySetLeftmost(f,*leftmost);  /* positive edge is stored */
    if (Biddy_GetMark(f)) {
      *leftmost = !(*leftmost); /* actual edge is returned */
      if (*leftmost) {
        (*c1)++; /* actual edge is returned */
      } else {
        (*c1)--; /* actual edge is returned */
      }
    }
  }
  else if ((biddyManagerType == BIDDYTYPETZBDDC) || (biddyManagerType == BIDDYTYPETZBDD)) {
    BiddySetPath1Count(f,*c1);
  }

  /* DEBUGGING */
  /*
  printf("pathCount finished, results returned: c1=%llu, c0=%llu\n",*c1,*c0);
  */

  return;
}

/*******************************************************************************
\brief Function mintermCount.

### Description
### Side effects
    We are using GNU Multiple Precision Arithmetic Library (GMP).
### More info
*******************************************************************************/

static void
mintermCount(Biddy_Manager MNG, Biddy_Edge f, mpz_t max, mpz_t result,
             Biddy_Boolean *leftmost)
{
  mpz_t countE,countT;
  Biddy_Variable v;

  assert((
    (biddyManagerType == BIDDYTYPEOBDD) ||
    (biddyManagerType == BIDDYTYPEOBDDC) ||
    (biddyManagerType == BIDDYTYPEZBDDC) ||
    (biddyManagerType == BIDDYTYPETZBDD)
  ));

  /* For OBDD and TZBDD, leftmost is not used */
  /* For ZBDD, max is not used */
  if (f == biddyZero) {
    mpz_set_ui(result,0);
    *leftmost = FALSE;
    return;
  }
  if (Biddy_IsConstant(f)) {
    if ((biddyManagerType == BIDDYTYPEOBDDC) || (biddyManagerType == BIDDYTYPEOBDD)) {
      mpz_set(result,max);
    }
    else if ((biddyManagerType == BIDDYTYPEZBDDC) || (biddyManagerType == BIDDYTYPEZBDD)) {
      mpz_set_ui(result,1);
    }
    else if ((biddyManagerType == BIDDYTYPETZBDDC) || (biddyManagerType == BIDDYTYPETZBDD)) {
      mpz_set(result,max);
      v = BiddyV(f);
      while (v != Biddy_GetTag(f)) {
        mpz_divexact_ui(result,result,2); /* actual edge is returned */
        v = biddyVariableTable.table[v].prev;
      }
    }
    *leftmost = TRUE;
    return;
  }

  if (Biddy_Managed_IsSelected(MNG,f)) {
    /* ALREADY VISITED */
    if ((biddyManagerType == BIDDYTYPEOBDDC) || (biddyManagerType == BIDDYTYPEOBDD)) {
      BiddyGetMintermCount(f,result); /* positive edge is stored */
      if (Biddy_GetMark(f)) {
        mpz_sub(result,max,result); /* actual edge is returned */
      }
    }
    else if ((biddyManagerType == BIDDYTYPEZBDDC) || (biddyManagerType == BIDDYTYPEZBDD)) {
      BiddyGetMintermCount(f,result); /* positive edge is stored */
      *leftmost = BiddyGetLeftmost(f); /* positive edge is stored */
      if (Biddy_GetMark(f)) {
        *leftmost = !(*leftmost); /* actual edge is returned */
        if (*leftmost) {
          mpz_add_ui(result,result,1); /* actual edge is returned */
        } else {
          mpz_sub_ui(result,result,1); /* actual edge is returned */
        }
      }
    }
    else if ((biddyManagerType == BIDDYTYPETZBDDC) || (biddyManagerType == BIDDYTYPETZBDD)) {
      BiddyGetMintermCount(f,result); /* untagged edge is stored */
      v = BiddyV(f);
      while (v != Biddy_GetTag(f)) {
        mpz_divexact_ui(result,result,2);
        v = biddyVariableTable.table[v].prev;
      }
    }      
    return;
  }

  /* NOT VISITED, YET */
  Biddy_Managed_SelectNode(MNG,f);
  
  mpz_init(countE);
  mpz_init(countT);

  /* because of parameter leftmost it is important to calculate countT first! */
  mintermCount(MNG,BiddyT(f),max,countT,leftmost);
  mintermCount(MNG,BiddyE(f),max,countE,leftmost);

  mpz_add(result,countE,countT);

  if ((biddyManagerType == BIDDYTYPEOBDDC) || (biddyManagerType == BIDDYTYPEOBDD)) {
    mpz_divexact_ui(result,result,2);
    BiddySetMintermCount(f,result); /* positive edge is stored */
    if (Biddy_GetMark(f)) {
      mpz_sub(result,max,result); /* actual edge is returned */
    }
  }
  else if ((biddyManagerType == BIDDYTYPEZBDDC) || (biddyManagerType == BIDDYTYPEZBDD)) {
    BiddySetMintermCount(f,result); /* positive edge is stored */
    BiddySetLeftmost(f,*leftmost); /* positive edge is stored */
    if (Biddy_GetMark(f)) {
      *leftmost = !(*leftmost); /* actual edge is returned */
      if (*leftmost) {
        mpz_add_ui(result,result,1); /* actual edge is returned */
      } else {
        mpz_sub_ui(result,result,1); /* actual edge is returned */
      }
    }
  }
  else if ((biddyManagerType == BIDDYTYPETZBDDC) || (biddyManagerType == BIDDYTYPETZBDD)) {
    mpz_divexact_ui(result,result,2);
    BiddySetMintermCount(f,result); /* untagged edge is stored */
    v = BiddyV(f);
    while (v != Biddy_GetTag(f)) {
      mpz_divexact_ui(result,result,2); /* actual edge is returned */
      v = biddyVariableTable.table[v].prev;
    }
  }      

  mpz_clear(countE);
  mpz_clear(countT);
}

/*******************************************************************************
\brief Function nodePlainNumber.

### Description
### Side effects
### More info
*******************************************************************************/

static unsigned int
nodePlainNumber(Biddy_Manager MNG, Biddy_Edge f)
{
  unsigned int n;

  assert( f != NULL );

  assert((
    (biddyManagerType == BIDDYTYPEOBDDC) ||
    (biddyManagerType == BIDDYTYPEZBDDC)
  ));

  /* DEBUGGING */
  /*
  printf("nodePlainNumber: f = %s (%p)\n",Biddy_Managed_GetTopVariableName(MNG,f),f);
  */

  if (Biddy_IsConstant(f)) return 0;
  if (Biddy_Managed_IsSelected(MNG,f)) {
    if (BiddyIsSelectedNP(f)) {
      /* ALREADY VISITED */
      return 0;
    } else {
      if (biddyManagerType == BIDDYTYPEOBDDC) {
        BiddySelectNP(f);
        /* COUNTING MUST BE REPEATED */
        /* mark is inverted */
        /* I do not know how to calculate plain number of the negated function from the existing info */
        /* this seems to be impossible */
      }
      else if (biddyManagerType == BIDDYTYPEZBDDC) {
        /* COUNTING MUST BE REPEATED */
      }
    }
  }

  Biddy_Managed_SelectNode(MNG,f);
  BiddySelectNP(f);

  n = 1;
  n += nodePlainNumber(MNG,
         Biddy_Managed_TransferMark(MNG,BiddyE(f),Biddy_GetMark(f),TRUE));
  n += nodePlainNumber(MNG,
         Biddy_Managed_TransferMark(MNG,BiddyT(f),Biddy_GetMark(f),FALSE));

  /* DEBUGGING */
  /*
  printf("nodePlainNumber: return %u\n",n);
  */

  return n;
}

/*******************************************************************************
\brief Function calculateSD.

### Description
### Side effects
### More info
*******************************************************************************/

static float
calculateSD(unsigned long long int *data, unsigned int n)
{
  float standardDeviation = 0.0;

#ifdef BIDDYEXTENDEDSTATS_YES
  unsigned int i;
  unsigned long long int sum;
  float mean;

  sum = 0;
  for (i = 0; i < n; i++) {
    sum += data[i];
  }

  mean = (float) sum / (float) n;

  standardDeviation = 0;
  for (i=0; i<n; ++i) {
    standardDeviation += ((float) (data[i] - mean)) * ((float) (data[i] - mean));
  }
  standardDeviation = sqrt(standardDeviation/n);

  /*
  printf("(data: ");
  for (i=0; i<n; ++i) {
    printf("%llu",data[i]);
    if (i < n-1 ) printf(" "); else printf("), ");
  }
  printf("%.0f, %.0f, ",mean,standardDeviation);
  */
#endif

  return standardDeviation;
}

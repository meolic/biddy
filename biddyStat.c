/***************************************************************************//*!
\file biddyStat.c
\brief File biddyStat.c contains statistical functions.

### Description

    PackageName [Biddy]
    Synopsis    [Biddy provides data structures and algorithms for the
                 representation and manipulation of Boolean functions with
                 ROBDDs. A hash table is used for quick search of nodes.
                 Complement edges decreases the number of nodes. An automatic
                 garbage collection with a formulae counter is implemented.
                 Variable swapping and sifting are implemented.]

    FileName    [biddyStat.c]
    Revision    [$Revision: 167 $]
    Date        [$Date: 2016-06-26 23:02:13 +0200 (ned, 26 jun 2016) $]
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

/*----------------------------------------------------------------------------*/
/* Static function prototypes                                                 */
/*----------------------------------------------------------------------------*/

static void MaxLevel(Biddy_Edge f, unsigned int *max, unsigned int i);

static void AvgLevel(Biddy_Edge f, float *sum, unsigned int *n, unsigned int i);

static void mintermCount(Biddy_Manager MNG, Biddy_Edge f, mpz_t max, mpz_t result);

static unsigned int nodePlainNumber(Biddy_Manager MNG, Biddy_Edge f);

/*----------------------------------------------------------------------------*/
/* Definition of exported functions                                           */
/*----------------------------------------------------------------------------*/

/***************************************************************************//*!
\brief Function Biddy_Managed_NodeNumber.

### Description
    Count number of nodes in a BDD.
### Side effects
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

  /* DEBUGGING */
  /*
  BiddyFunctionReport(biddyAnonymousManager,f);
  BiddySystemReport(biddyAnonymousManager);
  */

  n = 1; /* CONSTANT NODE IS COUNTED HERE */

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
\brief Function Biddy_Managed_NodeTableANDNumber.

### Description
### Side effects
### More info
    Macro Biddy_NodeTableANDNumber() is defined for use with anonymous manager.
*******************************************************************************/

#ifdef __cplusplus
extern "C" {
#endif

unsigned int
Biddy_Managed_NodeTableANDNumber(Biddy_Manager MNG)
{
  if (!MNG) MNG = biddyAnonymousManager;

  return biddyNodeTable.funand;
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
\brief Function Biddy_Managed_IteCacheSearch.

### Description
### Side effects
### More info
    Macro Biddy_IteCacheSearch() is defined for use with anonymous manager.
*******************************************************************************/

#ifdef __cplusplus
extern "C" {
#endif

unsigned long long int
Biddy_Managed_IteCacheSearch(Biddy_Manager MNG)
{
  if (!MNG) MNG = biddyAnonymousManager;

  return *(biddyIteCache.search);
}

#ifdef __cplusplus
}
#endif

/***************************************************************************//*!
\brief Function Biddy_Managed_IteCacheFind.

### Description
### Side effects
### More info
    Macro Biddy_IteCacheFind() is defined for use with anonymous manager.
*******************************************************************************/

#ifdef __cplusplus
extern "C" {
#endif

unsigned long long int
Biddy_Managed_IteCacheFind(Biddy_Manager MNG)
{
  if (!MNG) MNG = biddyAnonymousManager;

  return *(biddyIteCache.find);
}

#ifdef __cplusplus
}
#endif

/***************************************************************************//*!
\brief Function Biddy_Managed_IteCacheOverwrite.

### Description
### Side effects
### More info
    Macro Biddy_IteCacheOverwrite() is defined for use with anonymous manager.
*******************************************************************************/

#ifdef __cplusplus
extern "C" {
#endif

unsigned long long int
Biddy_Managed_IteCacheOverwrite(Biddy_Manager MNG)
{
  if (!MNG) MNG = biddyAnonymousManager;

  return *(biddyIteCache.overwrite);
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
  if (Biddy_IsConstant(f)) return 1;

  n = 2; /* BOTH CONSTANT NODES ARE COUNTED HERE */
#pragma warning(suppress: 6031)
  BiddyCreateLocalInfo(MNG,f); /* returned number of variables is not used */
  n += nodePlainNumber(MNG,f); /* select all nodes except constant node */
  BiddyDeleteLocalInfo(MNG,f);

  return n;
}

#ifdef __cplusplus
}
#endif

/***************************************************************************//*!
\brief Function Biddy_Managed_VariableNumber.

### Description
    Count number of dependent variables in a function.
### Side effects
### More info
    Macro Biddy_VariableNumber(f) is defined for use with anonymous manager.
*******************************************************************************/

#ifdef __cplusplus
extern "C" {
#endif

unsigned int
Biddy_Managed_VariableNumber(Biddy_Manager MNG, Biddy_Edge f)
{
  Biddy_Variable i;
  unsigned int n;
  unsigned int v;

  if (!MNG) MNG = biddyAnonymousManager;

  v = 0;

  if (Biddy_IsNull(f)) return 0;
  if (Biddy_IsConstant(f)) return 0;

  for (i=1;i<biddyVariableTable.num;i++) {
    biddyVariableTable.table[i].selected = FALSE;
  }

  n = 1; /* NOT NEEDED, BUT NODES ARE COUNTED BY BiddyNodeVarNumber */
  Biddy_Managed_SelectNode(MNG,biddyOne); /* needed for BiddyNodeVarNumber */
  BiddyNodeVarNumber(MNG,f,&n); /* VARIABLES ARE MARKED */
  Biddy_Managed_DeselectAll(MNG);

  for (i=1;i<biddyVariableTable.num;i++) {
    if (biddyVariableTable.table[i].selected == TRUE) {
      v++;
      biddyVariableTable.table[i].selected = FALSE;
    }
  }

  return v;
}

#ifdef __cplusplus
}
#endif

/***************************************************************************//*!
\brief Function Biddy_Managed_NodeVarNumber.

### Description
    Count number of nodes and number of dependent variables in a function.
### Side effects
    This is faster as counting separately.
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

  for (i=0;i<biddyVariableTable.num;i++) {
    biddyVariableTable.table[i].selected = FALSE;
  }

  Biddy_Managed_SelectNode(MNG,biddyOne); /* needed for BiddyNodeVarNumber */
  BiddyNodeVarNumber(MNG,f,n); /* VARIABLES ARE MARKED */
  Biddy_Managed_DeselectAll(MNG);

  for (i=0;i<biddyVariableTable.num;i++) {
    if (biddyVariableTable.table[i].selected == TRUE) {
      (*v)++;
      biddyVariableTable.table[i].selected = FALSE;
    }
  }

}

#ifdef __cplusplus
}
#endif

/***************************************************************************//*!
\brief Function Biddy_Managed_CountMinterm.

### Description
    If nvars == 0 then number of dependent variables is used.
### Side effects
    We are using GNU Multiple Precision Arithmetic Library (GMP).
### More info
    Macro Biddy_CountMinterm(f,nvars) is defined for use with anonymous manager.
*******************************************************************************/

#ifdef __cplusplus
extern "C" {
#endif

double
Biddy_Managed_CountMinterm(Biddy_Manager MNG, Biddy_Edge f, unsigned int nvars)
{
  unsigned int var;
  mpz_t max;
  mpz_t result;
  double resultd;

  if (!MNG) MNG = biddyAnonymousManager;

  if (Biddy_IsNull(f)) return 0;
  if (Biddy_IsConstant(f)) return 0; /* 0 OR 1, NOT SURE ABOUT THIS! */

  var = BiddyCreateLocalInfo(MNG,f);
  if (nvars == 0) nvars = var;
  if (var > nvars) {
    fprintf(stderr,"WARNING (Biddy_CountMinterm): var > nvars\n");
  }

  mpz_init(max);
  mpz_ui_pow_ui(max,2,nvars);
  mpz_init(result);
  mintermCount(MNG,f,max,result); /* select all nodes except constant node */
  if (Biddy_GetMark(f)) {
    mpz_sub(result,max,result);
  }
  resultd = mpz_get_d(result);
  mpz_clear(max);
  mpz_clear(result);

  BiddyDeleteLocalInfo(MNG,f);

  return resultd;
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

  /* ITE cache */
  n += sizeof(BiddyOp3CacheTable) +
       (biddyIteCache.size+1) * sizeof(BiddyOp3Cache);
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
  fprintf(f,"Initial ITE cache size: %u\n",biddyIteCache.size);
  fprintf(f,"Initial EA cache size: %u\n",biddyEACache.size);
  fprintf(f,"Initial RC cache size: %u\n",biddyRCCache.size);
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
  fprintf(f, "Number of ITE calls: %u (internal direct and recursive calls: %I64u)\n",biddyNodeTable.funite,biddyNodeTable.iterecursive);
  fprintf(f, "Number of AND calls: %u (internal direct and recursive calls: %I64u)\n",biddyNodeTable.funand,biddyNodeTable.andrecursive);
  fprintf(f, "Number of node table FOA calls: %I64u\n",biddyNodeTable.foa);
  fprintf(f, "Number of node table insertions: %I64u\n",biddyNodeTable.add);
  fprintf(f, "Number of compared nodes: %I64u (%.2f per findNodeTable call)\n",
                                   biddyNodeTable.compare,
                                   biddyNodeTable.find ?
                                   (1.0*biddyNodeTable.compare/biddyNodeTable.find):0);
#else
  fprintf(f, "Number of ITE calls: %u (internal direct and recursive calls: %llu)\n",biddyNodeTable.funite,biddyNodeTable.iterecursive);
  fprintf(f, "Number of AND calls: %u (internal direct and recursive calls: %llu)\n",biddyNodeTable.funand,biddyNodeTable.andrecursive);
  fprintf(f, "Number of node table FOA calls: %llu\n",biddyNodeTable.foa);
  fprintf(f, "Number of node table insertions: %llu\n",biddyNodeTable.add);
  fprintf(f, "Number of compared nodes: %llu (%.2f per findNodeTable call)\n",
                                   biddyNodeTable.compare,
                                   biddyNodeTable.find ?
                                   (1.0*biddyNodeTable.compare/biddyNodeTable.find):0);
#endif
#endif
  fprintf(f,"**** ITE cache stats ****\n");
  fprintf(f,"Size of ITE cache: %u\n",biddyIteCache.size);
#ifdef MINGW
  fprintf(f, "Number of ITE cache look-ups: %I64u\n",*(biddyIteCache.search));
  fprintf(f, "Number of ITE cache hits: %I64u (%.2f%% of all calls)\n",
                                   *(biddyIteCache.find),
                                   *(biddyIteCache.search) ?
                                   (100.0*(*(biddyIteCache.find))/(*(biddyIteCache.search))):0);
  fprintf(f,"Number of ITE cache insertions: %I64u (%.2f%% of all calls)\n",
                                   *(biddyIteCache.insert),
                                   *(biddyIteCache.search) ?
                                   (100.0*(*(biddyIteCache.insert))/(*(biddyIteCache.search))):0);
  fprintf(f, "Number of ITE cache collisions: %I64u (%.2f%% of all insertions, %.2f%% of all calls)\n",
                                   *(biddyIteCache.overwrite),
                                   (*(biddyIteCache.insert)) ?
                                   (100.0*(*(biddyIteCache.overwrite))/(*(biddyIteCache.insert))):0,
                                   *(biddyIteCache.search) ?
                                   (100.0*(*(biddyIteCache.overwrite))/(*(biddyIteCache.search))):0);
#else
  fprintf(f, "Number of ITE cache look-ups: %llu\n",*(biddyIteCache.search));
  fprintf(f, "Number of ITE cache hits: %llu (%.2f%% of all calls)\n",
                                   *(biddyIteCache.find),
                                   *(biddyIteCache.search) ?
                                   (100.0*(*(biddyIteCache.find))/(*(biddyIteCache.search))):0);
  fprintf(f,"Number of ITE cache insertions: %llu (%.2f%% of all calls)\n",
                                   *(biddyIteCache.insert),
                                   *(biddyIteCache.search) ?
                                   (100.0*(*(biddyIteCache.insert))/(*(biddyIteCache.search))):0);
  fprintf(f, "Number of ITE cache collisions: %llu (%.2f%% of all insertions, %.2f%% of all calls)\n",
                                   *(biddyIteCache.overwrite),
                                   (*(biddyIteCache.insert)) ?
                                   (100.0*(*(biddyIteCache.overwrite))/(*(biddyIteCache.insert))):0,
                                   *(biddyIteCache.search) ?
                                   (100.0*(*(biddyIteCache.overwrite))/(*(biddyIteCache.search))):0);
#endif
  fprintf(f,"**** EA cache stats ****\n");
  fprintf(f,"Size of EA cache: %u\n",biddyEACache.size);
#ifdef MINGW
  fprintf(f, "Number of EA cache look-ups: %I64u\n",*(biddyEACache.search));
  fprintf(f, "Number of EA cache hits: %I64u (%.2f%% of all calls)\n",
                                   *(biddyEACache.find),
                                   (*(biddyEACache.search)) ?
                                   (100.0*(*(biddyEACache.find))/(*(biddyEACache.search))):0);
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
#else
  fprintf(f, "Number of EA cache look-ups: %llu\n",*(biddyEACache.search));
  fprintf(f, "Number of EA cache hits: %llu (%.2f%% of all calls)\n",
                                   *(biddyEACache.find),
                                   (*(biddyEACache.search)) ?
                                   (100.0*(*(biddyEACache.find))/(*(biddyEACache.search))):0);
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
  fprintf(f,"**** RC cache stats ****\n");
  fprintf(f,"Size of RC cache: %u\n",biddyRCCache.size);
#ifdef MINGW
  fprintf(f, "Number of RC cache look-ups: %I64u\n",*(biddyRCCache.search));
  fprintf(f, "Number of RC cache hits: %I64u (%.2f%% of all calls)\n",
                                   *(biddyRCCache.find),
                                   (*(biddyRCCache.search)) ?
                                   (100.0*(*(biddyRCCache.find))/(*(biddyRCCache.search))):0);
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
#else
  fprintf(f, "Number of RC cache look-ups: %llu\n",*(biddyRCCache.search));
  fprintf(f, "Number of RC cache hits: %llu (%.2f%% of all calls)\n",
                                   *(biddyRCCache.find),
                                   (*(biddyRCCache.search)) ?
                                   (100.0*(*(biddyRCCache.find))/(*(biddyRCCache.search))):0);
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
    Function will select all nodes except constant node.
### More info
*******************************************************************************/

void
BiddyNodeNumber(Biddy_Manager MNG, Biddy_Edge f, unsigned int *n)
{
  if (!Biddy_IsConstant(f) && !Biddy_Managed_IsSelected(MNG,f)) {
    Biddy_Managed_SelectNode(MNG,f);
    (*n)++;
    BiddyNodeNumber(MNG,BiddyE(f),n);
    BiddyNodeNumber(MNG,BiddyT(f),n);
  }
}

/*******************************************************************************
\brief Function BiddyNodeVarNumber.

### Description
### Side effects
    Constant node must be selected! Function will select all other nodes. All
    dependent variables are selected.
### More info
*******************************************************************************/

void
BiddyNodeVarNumber(Biddy_Manager MNG, Biddy_Edge f, unsigned int *n)
{
  if (!Biddy_Managed_IsSelected(MNG,f)) {
    Biddy_Managed_SelectNode(MNG,f);
    (*n)++;
    biddyVariableTable.table[((BiddyNode *) Biddy_Regular(f))->v].selected = TRUE;
    BiddyNodeVarNumber(MNG,BiddyE(f),n);
    BiddyNodeVarNumber(MNG,BiddyT(f),n);
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
\brief Function mintermCount.

### Description
### Side effects
    We are using GNU Multiple Precision Arithmetic Library (GMP).
### More info
*******************************************************************************/

static void mintermCount(Biddy_Manager MNG, Biddy_Edge f, mpz_t max, mpz_t result)
{
  mpz_t countE,countT;

  if (Biddy_IsConstant(f)) {
    mpz_set(result,max);
    return;
  }

  if (Biddy_Managed_IsSelected(MNG,f)) {
    BiddyGetMintermCount(f,result);
    return;
  }
  
  mpz_init(countE);
  mpz_init(countT);
  Biddy_Managed_SelectNode(MNG,f);

  mintermCount(MNG,BiddyE(f),max,countE);
  if (Biddy_GetMark(BiddyE(f))) {
    mpz_sub(countE,max,countE);
  }
  mintermCount(MNG,BiddyT(f),max,countT);

  mpz_add(result,countE,countT);
  mpz_divexact_ui(result,result,2);

  mpz_clear(countE);
  mpz_clear(countT);

  BiddySetMintermCount(f,result);

  /* DEBUGGING */
  /*
  printf("MINTERM COUNT: ");
  if (Biddy_GetMark(f)) printf(" marked ");
  printf("edge to %s (%s, %s) represents %.0f minterms\n",
         Biddy_Managed_GetTopVariableName(MNG,f),
         Biddy_Managed_GetTopVariableName(MNG,BiddyE(f)),
         Biddy_Managed_GetTopVariableName(MNG,BiddyT(f)),
         count
        );
  */
}

/*******************************************************************************
\brief Function nodePlainNumber.

### Description
### Side effects
### More info
*******************************************************************************/

static unsigned int nodePlainNumber(Biddy_Manager MNG, Biddy_Edge f)
{
  unsigned int n;

  if (Biddy_IsConstant(f)) return 0;
  if (Biddy_Managed_IsSelected(MNG,f)) {
    if (BiddyGetNodePlainSelected(f)) {
      /* ALREADY VISITED */
      return 0;
    } else {
      /* ALREADY VISITED BUT WITH A COMPLEMENTED EDGE */
      /* COUNTING MUST BE REPEATED */
    }
  }

  Biddy_Managed_SelectNode(MNG,f);
  BiddySetNodePlainSelected(f);

  n = 1;
  n += nodePlainNumber(MNG,Biddy_NotCond(BiddyE(f),Biddy_GetMark(f)));
  n += nodePlainNumber(MNG,Biddy_NotCond(BiddyT(f),Biddy_GetMark(f)));

  return n;
}

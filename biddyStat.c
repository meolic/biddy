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
    Revision    [$Revision: 132 $]
    Date        [$Date: 2016-01-02 09:59:22 +0100 (sob, 02 jan 2016) $]
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

/*----------------------------------------------------------------------------*/
/* Static function prototypes                                                 */
/*----------------------------------------------------------------------------*/

static void MaxLevel(Biddy_Edge f, unsigned int *max, unsigned int i);

static void AvgLevel(Biddy_Edge f, float *sum, unsigned int *n, unsigned int i);

static void mintermCount(Biddy_Edge f, mpz_t max, Biddy_Edge constantOne, mpz_t result);

static unsigned int nodePlainNumber(Biddy_Edge f);

/*----------------------------------------------------------------------------*/
/* Definition of exported functions                                           */
/*----------------------------------------------------------------------------*/

/***************************************************************************//*!
\brief Function Biddy_NodeNumber.

### Description
    Count number of nodes in a BDD.
### Side effects
### More info
*******************************************************************************/

#ifdef __cplusplus
extern "C" {
#endif

unsigned int
Biddy_NodeNumber(Biddy_Edge f)
{
  unsigned int n;

  if (Biddy_IsNull(f)) return 0;
  if (Biddy_IsConstant(f)) return 1;

  /* DEBUGGING */
  /*
  BiddyFunctionReport(biddyAnonymousManager,f);
  BiddySystemReport(biddyAnonymousManager);
  */

  n = 1; /* CONSTANT NODE IS COUNTED HERE */

  BiddyNodeNumber(f,&n);
  Biddy_NodeRepair(f);

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
\brief Function Biddy_Managed_NodeTableMax.

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
\brief Function Biddy_Managed_NodeTableNum.

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
\brief Function Biddy_Managed_NodeTableNumVar.

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
\brief Function Biddy_Managed_NodeTableNumF.

### Description
### Side effects
### More info
    Macro Biddy_NodeTableNumF() is defined for use with anonymous manager.
*******************************************************************************/

#ifdef __cplusplus
extern "C" {
#endif

unsigned int
Biddy_Managed_NodeTableNumF(Biddy_Manager MNG)
{
  if (!MNG) MNG = biddyAnonymousManager;

  return biddyNodeTable.numf;
}

#ifdef __cplusplus
}
#endif

/***************************************************************************//*!
\brief Function Biddy_Managed_NodeTableFOA.

### Description
### Side effects
### More info
    Macro Biddy_NodeTableFOA() is defined for use with anonymous manager.
*******************************************************************************/

#ifdef __cplusplus
extern "C" {
#endif

unsigned long long int
Biddy_Managed_NodeTableFOA(Biddy_Manager MNG)
{
  if (!MNG) MNG = biddyAnonymousManager;

  return biddyNodeTable.foa;
}

#ifdef __cplusplus
}
#endif

/***************************************************************************//*!
\brief Function Biddy_Managed_NodeTableCompare.

### Description
### Side effects
### More info
    Macro Biddy_NodeTableCompare() is defined for use with anonymous manager.
*******************************************************************************/

#ifdef __cplusplus
extern "C" {
#endif

unsigned long long int
Biddy_Managed_NodeTableCompare(Biddy_Manager MNG)
{
  if (!MNG) MNG = biddyAnonymousManager;

  return biddyNodeTable.compare;
}

#ifdef __cplusplus
}
#endif

/***************************************************************************//*!
\brief Function Biddy_Managed_NodeTableAdd.

### Description
### Side effects
### More info
    Macro Biddy_NodeTableAdd() is defined for use with anonymous manager.
*******************************************************************************/

#ifdef __cplusplus
extern "C" {
#endif

unsigned long long int
Biddy_Managed_NodeTableAdd(Biddy_Manager MNG)
{
  if (!MNG) MNG = biddyAnonymousManager;

  return biddyNodeTable.add;
}

#ifdef __cplusplus
}
#endif

/***************************************************************************//*!
\brief Function Biddy_Managed_NodeTableGarbage.

### Description
### Side effects
### More info
    Macro Biddy_NodeTableGarbage() is defined for use with anonymous manager.
*******************************************************************************/

#ifdef __cplusplus
extern "C" {
#endif

unsigned int
Biddy_Managed_NodeTableGarbage(Biddy_Manager MNG)
{
  if (!MNG) MNG = biddyAnonymousManager;

  return biddyNodeTable.garbage;
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
  for (i=1; i<biddyNodeTable.size+2; i++) {
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
  for (i=1; i<biddyNodeTable.size+2; i++) {
    if (biddyNodeTable.table[i] != NULL) {
      n = 0;
      sup = biddyNodeTable.table[i];
      while (sup != NULL) {
        n++;
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
  for (i=1; i<biddyNodeTable.size+2; i++) {
    if (biddyNodeTable.table[i] != NULL) {
      n++;
      sup = biddyNodeTable.table[i];
      while (sup != NULL) {
        sum = sum + 1;
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
    Macro Biddy_NodeNumberPlain(f) is defined for use with anonymous manager.
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
  n += nodePlainNumber(f); /* select all nodes except constant node */
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
  Biddy_SelectNode(biddyOne); /* needed for BiddyNodeVarNumber */
  BiddyNodeVarNumber(MNG,f,&n); /* VARIABLES ARE MARKED */
  Biddy_DeselectNode(biddyOne); /* needed for Biddy_NodeRepair */
  Biddy_NodeRepair(f);

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

  Biddy_SelectNode(biddyOne); /* needed for BiddyNodeVarNumber */
  BiddyNodeVarNumber(MNG,f,n); /* VARIABLES ARE MARKED */
  Biddy_DeselectNode(biddyOne); /* needed for Biddy_NodeRepair */
  Biddy_NodeRepair(f);

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
  mintermCount(f,max,biddyOne,result); /* select all nodes except constant node */
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
  n += biddyNodeTable.blocknumber * biddyBlockSize * sizeof(BiddyNode);

  /* node table */
  n += sizeof(BiddyNodeTable) +
       biddyNodeTable.blocknumber * sizeof(BiddyNode *) +
       (biddyNodeTable.size+2) * sizeof(BiddyNode *);

  /* variable table */
  n += sizeof(BiddyVariableTable) +
       biddyVariableTable.size * sizeof(BiddyVariable);
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
  n += 3 * sizeof(unsigned long long int);

  /* EA cache */
  n += sizeof(BiddyOp3CacheTable) +
       (biddyEACache.size+1) * sizeof(BiddyOp3Cache);
  n += 3 * sizeof(unsigned long long int);

  /* RC cache */
  n += sizeof(BiddyOp3CacheTable) +
       (biddyRCCache.size+1) * sizeof(BiddyOp3Cache);
  n += 3 * sizeof(unsigned long long int);
  
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
  fprintf(f,"sizeof(BiddyNode *): %u bytes\n",(unsigned int)sizeof(BiddyNode *));
  fprintf(f,"sizeof(BiddyNode): %u bytes\n",(unsigned int)sizeof(BiddyNode));
  fprintf(f,"sizeof(BiddyVariable): %u bytes\n",(unsigned int)sizeof(BiddyVariable));
  fprintf(f,"sizeof(BiddyFormula): %u bytes\n",(unsigned int)sizeof(BiddyFormula));
  fprintf(f,"sizeof(BiddyOp3Cache): %u bytes\n",(unsigned int)sizeof(BiddyOp3Cache));
  fprintf(f,"sizeof(BiddyOrderingTable): %u bytes\n",(unsigned int)sizeof(BiddyOrderingTable));
  fprintf(f,"**** Biddy modifiable parameters ****\n");
  fprintf(f,"Nodes per one memory block: %u\n",biddyBlockSize);
  fprintf(f,"Limit for number of variables: %u\n",biddyVariableTable.size);
  fprintf(f,"Limit for number of buckets in node table: %u\n",biddyNodeTable.size);
  fprintf(f,"ITE cache size: %u\n",biddyIteCache.size);
  fprintf(f,"EA cache size: %u\n",biddyEACache.size);
  fprintf(f,"RC cache size: %u\n",biddyRCCache.size);
  fprintf(f,"**** Biddy non-modifiable parameters ****\n");
#ifdef MINGW
  fprintf(f, "Memory in use: %I64u bytes\n", Biddy_Managed_ReadMemoryInUse(MNG));
#else
  fprintf(f, "Memory in use: %llu bytes\n", Biddy_Managed_ReadMemoryInUse(MNG));
#endif
  fprintf(f,"Number of memory blocks: %u\n",Biddy_Managed_NodeTableBlockNumber(MNG));
  fprintf(f,"Number of variables: %u\n",Biddy_Managed_VariableTableNum(MNG));
  fprintf(f,"Number of formulae: %u\n",biddyFormulaTable.size); /* TO DO: report externally */
  fprintf(f,"Peak number of BDD nodes: %u\n",Biddy_Managed_NodeTableGenerated(MNG));
  fprintf(f,"Peak number of live BDD nodes: %u\n",Biddy_Managed_NodeTableMax(MNG));
  fprintf(f,"Number of live BDD nodes: %u\n",Biddy_Managed_NodeTableNum(MNG));
  fprintf(f,"Number of fortified BDD nodes: %u\n",Biddy_Managed_NodeTableNumF(MNG));
#ifdef MINGW
  fprintf(f, "Number of node table FOA calls: %I64u\n", Biddy_Managed_NodeTableFOA(MNG));
#else
  fprintf(f, "Number of node table FOA calls: %llu\n", Biddy_Managed_NodeTableFOA(MNG));
#endif
#ifdef MINGW
  fprintf(f, "Number of node table compare calls: %I64u\n", Biddy_Managed_NodeTableCompare(MNG));
#else
  fprintf(f, "Number of node table compare calls: %llu\n", Biddy_Managed_NodeTableCompare(MNG));
#endif
#ifdef MINGW
  fprintf(f, "Number of node table insertions: %I64u\n", Biddy_Managed_NodeTableAdd(MNG));
#else
  fprintf(f, "Number of node table insertions: %llu\n", Biddy_Managed_NodeTableAdd(MNG));
#endif
  fprintf(f,"Garbage collections so far: %u\n",Biddy_Managed_NodeTableGarbage(MNG));
  fprintf(f,"Reorderings so far: %u\n",Biddy_Managed_NodeTableSiftingNumber(MNG));
  fprintf(f,"Node swaps in reordering: %u\n",Biddy_Managed_NodeTableSwapNumber(MNG));
  fprintf(f,"Number of buckets in node table: %u\n",Biddy_Managed_NodeTableSize(MNG));
  fprintf(f,"Used buckets in node table: %u (%.2f%%)\n",Biddy_Managed_ListUsed(MNG),(100.0*Biddy_Managed_ListUsed(MNG)/Biddy_Managed_NodeTableSize(MNG)));
  fprintf(f,"Max bucket's size in node table: %u\n",Biddy_Managed_ListMaxLength(MNG));
  fprintf(f,"Avg bucket's size in node table: %f\n",Biddy_Managed_ListAvgLength(MNG));
  fprintf(f,"Size of ITE cache: %u\n",biddyIteCache.size); /* TO DO: report externally */
#ifdef MINGW
  fprintf(f, "Number of ITE cache look-ups: %I64u\n", Biddy_Managed_IteCacheSearch(MNG));
#else
  fprintf(f, "Number of ITE cache look-ups: %llu\n", Biddy_Managed_IteCacheSearch(MNG));
#endif
#ifdef MINGW
  fprintf(f, "Number of ITE cache hits: %I64u\n", Biddy_Managed_IteCacheFind(MNG));
#else
  fprintf(f, "Number of ITE cache hits: %llu\n", Biddy_Managed_IteCacheFind(MNG));
#endif
  fprintf(f,"Number of ITE cache insertions: NOT IMPLEMENTED, YET\n");
#ifdef MINGW
  fprintf(f, "Number of ITE cache collisions: %I64u\n", Biddy_Managed_IteCacheOverwrite(MNG));
#else
  fprintf(f, "Number of ITE cache collisions: %llu\n", Biddy_Managed_IteCacheOverwrite(MNG));
#endif
  fprintf(f,"Size of EA cache: %u\n",biddyEACache.size); /* TO DO: report externally */
#ifdef MINGW
  fprintf(f, "Number of EA cache look-ups: %I64u\n", *(biddyEACache.search)); /* TO DO: report externally */
#else
  fprintf(f, "Number of EA cache look-ups: %llu\n", *(biddyEACache.search)); /* TO DO: report externally */
#endif
#ifdef MINGW
  fprintf(f, "Number of EA cache hits: %I64u\n", *(biddyEACache.find)); /* TO DO: report externally */
#else
  fprintf(f, "Number of EA cache hits: %llu\n", *(biddyEACache.find)); /* TO DO: report externally */
#endif
  fprintf(f,"Number of EA cache insertions: NOT IMPLEMENTED, YET\n");
#ifdef MINGW
  fprintf(f, "Number of EA cache collisions: %I64u\n", *(biddyEACache.overwrite)); /* TO DO: report externally */
#else
  fprintf(f, "Number of EA cache collisions: %llu\n", *(biddyEACache.overwrite)); /* TO DO: report externally */
#endif
  fprintf(f,"Size of RC cache: %u\n",biddyRCCache.size); /* TO DO: report externally */
#ifdef MINGW
  fprintf(f, "Number of RC cache look-ups: %I64u\n", *(biddyRCCache.search)); /* TO DO: report externally */
#else
  fprintf(f, "Number of RC cache look-ups: %llu\n", *(biddyRCCache.search)); /* TO DO: report externally */
#endif
#ifdef MINGW
  fprintf(f, "Number of RC cache hits: %I64u\n", *(biddyRCCache.find)); /* TO DO: report externally */
#else
  fprintf(f, "Number of RC cache hits: %llu\n", *(biddyRCCache.find)); /* TO DO: report externally */
#endif
  fprintf(f,"Number of RC cache insertions: NOT IMPLEMENTED, YET\n");
#ifdef MINGW
  fprintf(f, "Number of RC cache collisions: %I64u\n", *(biddyRCCache.overwrite)); /* TO DO: report externally */
#else
  fprintf(f, "Number of RC cache collisions: %llu\n", *(biddyRCCache.overwrite)); /* TO DO: report externally */
#endif
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
BiddyNodeNumber(Biddy_Edge f, unsigned int *n)
{
  if (!Biddy_IsConstant(f) && !Biddy_IsSelected(f)) {
    Biddy_SelectNode(f);
    (*n)++;
    BiddyNodeNumber(BiddyE(f),n);
    BiddyNodeNumber(BiddyT(f),n);
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
  if (!Biddy_IsSelected(f)) {
    Biddy_SelectNode(f);
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
    We are using argument constantOne to avoid dependency on a manager.
    We are using GNU Multiple Precision Arithmetic Library (GMP).
### More info
*******************************************************************************/

static void mintermCount(Biddy_Edge f, mpz_t max, Biddy_Edge constantOne, mpz_t result)
{
  mpz_t countE,countT;

  if (Biddy_IsEqvPointer(f,constantOne)) {
    mpz_set(result,max);
    return;
  }

  if (Biddy_IsSelected(f)) {
    BiddyGetMintermCount(f,result);
    return;
  }
  
  mpz_init(countE);
  mpz_init(countT);
  Biddy_SelectNode(f);

  mintermCount(BiddyE(f),max,constantOne,countE);
  if (Biddy_GetMark(BiddyE(f))) {
    mpz_sub(countE,max,countE);
  }
  mintermCount(BiddyT(f),max,constantOne,countT);

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
         Biddy_GetTopVariableName(f),
         Biddy_GetTopVariableName(BiddyE(f)),
         Biddy_GetTopVariableName(BiddyT(f)),
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

static unsigned int nodePlainNumber(Biddy_Edge f)
{
  unsigned int n;

  if (Biddy_IsConstant(f)) return 0;
  if (Biddy_IsSelected(f)) {
    if (BiddyGetNodePlainSelected(f)) {
      /* ALREADY VISITED */
      return 0;
    } else {
      /* ALREADY VISITED BUT WITH A COMPLEMENTED EDGE */
      /* COUNTING MUST BE REPEATED */
    }
  }

  Biddy_SelectNode(f);
  BiddySetNodePlainSelected(f);

  n = 1;
  n += nodePlainNumber(Biddy_NotCond(BiddyE(f),Biddy_GetMark(f)));
  n += nodePlainNumber(Biddy_NotCond(BiddyT(f),Biddy_GetMark(f)));

  return n;
}

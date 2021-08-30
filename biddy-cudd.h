/* $Revision: 652 $ */
/* $Date: 2021-08-28 09:52:46 +0200 (sob, 28 avg 2021) $ */
/* This file (biddy-cudd.h) is a C header file */
/* Author: Robert Meolic (robert@meolic.com) */
/* This file has been released into the public domain by the author. */

/* This example is compatible with Biddy v1.8 (and laters) and CUDD v3.0.0 */
/* See Biddy examples (e.g. biddy-example-dictionary.c) to see how this file is used */

/* COMPILE WITH: */
/* gcc -DUNIX -DBIDDY -DOBDDC -O2 -o output input.c -I. -L./bin -static -lbiddy -lgmp */
/* gcc -DCUDD -DOBDDC -O2 -o output input.c -I ../cudd/include/ -L ../cudd/lib/ -lcudd -lm */

#ifdef BIDDY
#  include "biddy.h"
#  define BDDNULL NULL
#endif

#ifdef CUDD
#  include <stdlib.h>
#  include <stdint.h>
#  include <stdarg.h>
#  include <math.h>
#  include "cudd.h"
#  define BDDNULL NULL
#  ifndef TRUE
#    define TRUE (0 == 0)
#  endif
#  ifndef FALSE
#    define FALSE !TRUE
#  endif
typedef char *Biddy_String;
typedef char Biddy_Boolean;
typedef DdNode *Biddy_Edge;
DdManager *manager;
#endif

/* OBDD is not supported by CUDD */
/* ZBDDC is not supported by CUDD */
#ifdef CUDD
#  ifdef OBDD
#    undef OBDD
#    define OBDDC
#  endif
#  ifdef ZBDDC
#    undef ZBDDC
#    define ZBDD
#  endif
#  define BIDDYTYPEOBDD 1
#  define BIDDYTYPEOBDDC 2
#  define BIDDYTYPEZBDD 3
#  define BIDDYTYPEZBDDC 4
#  define BIDDYTYPETZBDD 5
#  define BIDDYTYPETZBDDC 6
#endif

#if !defined(OBDD) && !defined(OBDDC) && !defined(ZBDD) && !defined(ZBDDC) && !defined(TZBDD) && !defined(TZBDDC)
#  define OBDDC
#endif

#ifdef OBDD
#  define BDDTYPE BIDDYTYPEOBDD
#endif
#ifdef OBDDC
#  define BDDTYPE BIDDYTYPEOBDDC
#endif
#ifdef ZBDD
#  define BDDTYPE BIDDYTYPEZBDD
#endif
#ifdef ZBDDC
#  define BDDTYPE BIDDYTYPEZBDDC
#endif
#ifdef TZBDD
#  define BDDTYPE BIDDYTYPETZBDD
#endif
#ifdef TZBDDC
#  define BDDTYPE BIDDYTYPETZBDDC
#endif

#ifdef BIDDY
#define REF(f) 0
#define DEREF(f) 0
#define INITGC 0
#ifdef DISABLEGC
#  define MARK(f) 0
#  define MARK_0(f) 0
#  define MARK_N(f,n) 0
#  define SWEEP() 0
#else
#  define MARK(f) Biddy_KeepFormula(f)
#  define MARK_0(f) Biddy_KeepFormulaUntilPurge(f)
#  define MARK_N(f,n) Biddy_KeepFormulaProlonged(f,n)
#  define MARK_X(x,f) Biddy_AddPersistentFormula((Biddy_String)x,f)
#  define SWEEP() Biddy_Clean()
#endif
#define Biddy_NodeTableNumLive() (unsigned long int) 0 /* not implemented */
#define Biddy_GlobalPosition(v) (Biddy_Variable) 0 /* not implemented */
#define Biddy_GlobalSifting() Biddy_Sifting(NULL,FALSE)
#define Biddy_GlobalConvergedSifting() Biddy_Sifting(NULL,TRUE)
#endif

#ifdef CUDD
#  define MARK(f) 0
#  define MARK_0(f) 0
#  define MARK_N(f,n) 0
#  define MARK_X(x,f) 0
#  define SWEEP() 0
#  define REF(f) Cudd_Ref(f)
#ifdef DISABLEGC
#  define INITGC Cudd_DisableGarbageCollection(manager)
#else
#  define INITGC 0
#endif
#  ifdef OBDDC
#    define DEREF(f) Cudd_RecursiveDeref(manager,f)
#    define Biddy_GlobalPosition(v) Cudd_ReadPerm(manager,v)
#    define Biddy_GetIthVariable(i) Cudd_ReadInvPerm(manager,i)
#    define Biddy_GetConstantZero() Cudd_ReadLogicZero(manager)
#    define Biddy_GetConstantOne() Cudd_ReadOne(manager)
#    define Biddy_AddVariable() Cudd_NodeReadIndex(Cudd_bddNewVar(manager))
#    define Biddy_AddVariableEdge() Cudd_bddNewVar(manager)
#    define Biddy_GetVariableEdge(i) Cudd_bddIthVar(manager,i)
#    define Biddy_Not(f) Cudd_Not(f)
#    define Biddy_And(f,g) Cudd_bddAnd(manager,f,g)
#    define Biddy_Or(f,g) Cudd_bddOr(manager,f,g)
#    define Biddy_Xor(f,g) Cudd_bddXor(manager,f,g)
#    define Biddy_Xnor(f,g) Cudd_bddXnor(manager,f,g)
#    define Biddy_Compose(f,g,v) Cudd_bddCompose(manager,f,g,v)
#    define Biddy_AndAbstract(f,g,c) Cudd_bddAndAbstract(manager,f,g,c)
#    define Biddy_VariableTableNum() Cudd_ReadSize(manager)
#    define Biddy_NodeTableNum() Cudd_ReadKeys(manager)
#    define Biddy_NodeTableNumLive() Cudd_ReadNodeCount(manager)
#    define Biddy_CountNodes(f) Cudd_DagSize(f)
#    define Biddy_DependentVariableNumber(f,mustbefalse) Cudd_SupportSize(manager,f)
#    define Biddy_CountMinterms(f,n) Cudd_CountMinterm(manager,f,n)
#    define Biddy_GlobalSifting() Cudd_ReduceHeap(manager,CUDD_REORDER_SIFT,0)
#    define Biddy_GlobalConvergedSifting() Cudd_ReduceHeap(manager,CUDD_REORDER_SIFT_CONVERGE,0)
#  elif defined(ZBDD)
#    define DEREF(f) Cudd_RecursiveDerefZdd(manager,f)
#    define Biddy_GlobalPosition(v) Cudd_ReadPermZdd(manager,v)
#    define Biddy_GetIthVariable(i) Cudd_ReadInvPermZdd(manager,i)
#    define Biddy_GetConstantZero() Cudd_ReadZero(manager)
#    define Biddy_GetConstantOne() Cudd_ReadZddOne(manager,0)
#    define Biddy_AddVariable() 0 /* problematic because Cudd_zddIthVar recalculate result and then return unreferenced edge */
#    define Biddy_AddVariableEdge() Cudd_zddIthVar(manager,Cudd_ReadZddSize(manager))
#    define Biddy_GetVariableEdge(i) Cudd_zddIthVar(manager,i)
#    define Biddy_Not(f) Cudd_zddDiff(manager,Cudd_ReadZddOne(manager,0),f)
#    define Biddy_And(f,g) Cudd_zddIntersect(manager,f,g)
#    define Biddy_Or(f,g) Cudd_zddUnion(manager,f,g)
#    define Biddy_Xor(f,g) 0 /* not implemented */ /* check Extra_zddUnionExor */
#    define Biddy_Xnor(f,g) 0 /* not implemented */
#    define Biddy_Compose(f,g,v) 0 /* not implemented */
#    define Biddy_AndAbstract(f,g,c) 0 /* not implemented */
#    define Biddy_VariableTableNum() Cudd_ReadZddSize(manager)
#    define Biddy_NodeTableNum() 0 /* not implemented */
#    define Biddy_NodeTableNumLive() Cudd_zddReadNodeCount(manager)
#    define Biddy_CountNodes(f) ((Cudd_zddDagSize(f)==0)?1:Cudd_zddDagSize(f)+2) /* this is not always valid but there is no better option */
#    define Biddy_DependentVariableNumber(f,mustbefalse) Cudd_ReadZddSize(manager) /* this is not always valid but there is no better option */
#    define Biddy_CountMinterms(f,n) Cudd_zddCountMinterm(manager,f,n)
#    define Biddy_GlobalSifting() Cudd_zddReduceHeap(manager,CUDD_REORDER_SIFT,0)
#    define Biddy_GlobalConvergedSifting() Cudd_zddReduceHeap(manager,CUDD_REORDER_SIFT_CONVERGE,0)
#  endif
#  define Biddy_GetManagerType() BDDTYPE
#  define Biddy_GetManagerName() "CUDD"
#  define Biddy_ChangeVariableName(v,name) 0
#  define Biddy_DensityOfBDD(f,n) Cudd_Density(manager,f,n)
#  define Biddy_CountComplementedEdges(f) 0 /* not implemented */
#  define Biddy_NodeTableSize() Cudd_ReadSlots(manager)
#  define Biddy_NodeTableGCNumber() Cudd_ReadGarbageCollections(manager)
#  define Biddy_NodeTableGCTime() Cudd_ReadGarbageCollectionTime(manager)
#  define Biddy_NodeTableSiftingNumber() Cudd_ReadReorderings(manager)
#  define Biddy_NodeTableDRTime() Cudd_ReadReorderingTime(manager)
#  define Biddy_OPCacheSearch() (unsigned long int) 0
#  define Biddy_ReadMemoryInUse() Cudd_ReadMemoryInUse(manager)
#  define Biddy_PrintInfo(s) Cudd_PrintInfo(manager,s)
#  define Biddy_Exit() Cudd_Quit(manager)

void Biddy_WriteDot(const char filename[], Biddy_Edge f, const char ignore1[], int ignore2, Biddy_Boolean ignore3)
{
  FILE * fp;
  fp = fopen(filename,"w");
  if ((Biddy_GetManagerType() == BIDDYTYPEOBDD) || (Biddy_GetManagerType() == BIDDYTYPEOBDDC)) Cudd_DumpDot(manager,1,&f,NULL,NULL,fp);
  if ((Biddy_GetManagerType() == BIDDYTYPEZBDD) || (Biddy_GetManagerType() == BIDDYTYPEZBDDC)) Cudd_zddDumpDot(manager,1,&f,NULL,NULL,fp);
  fclose(fp);
}
#endif

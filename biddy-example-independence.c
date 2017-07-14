/* $Revision: 290 $ */
/* $Date: 2017-07-13 16:28:31 +0200 (čet, 13 jul 2017) $ */
/* This file (biddy-example-independence.c) is a C file */
/* Author: Robert Meolic (robert.meolic@um.si) */
/* This file has been released into the public domain by the author. */

/* This example is compatible with Biddy v1.7 and CUDD v3.0.0 */

/* COMPILE WITH: */
/* gcc -DNOCONVERGE -DNOPROFILE -DUNIX -DUSE_BIDDY -O2 -o biddy-example-independence biddy-example-independence.c biddy-example-independence-usa.c biddy-example-independence-europe.c -I. -L./bin -lbiddy -lgmp */
/* gcc -DNOCONVERGE -DNOPROFILE -DUSE_CUDD -O2 -o cudd-example-independence biddy-example-independence.c biddy-example-independence-usa.c biddy-example-independence-europe.c -I ../cudd/include/ -L ../cudd/lib/ -lcudd -lm */

/* define CALCULATE_KERNELS 0 OR 1 */
/* define USA_NO or USA_YES */
/* define EUROPE_NO or EUROPE_YES */
#define CALCULATE_KERNELS 0
#define USA_YES
#define EUROPE_YES

#ifdef USE_BIDDY
#  include "biddy.h"
#  include <string.h>
#  define BDDNULL NULL
/* #define Biddy_Inv(f) Biddy_NOT(f) */ /* for compatibility with Biddy v1.6 */
#endif

#ifdef USE_CUDD
#  include <stdio.h>
#  include <stdlib.h>
#  include <stdint.h>
#  include <stdarg.h>
#  include <ctype.h>
#  include <string.h>
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

#include <time.h>

#ifdef USE_CUDD
#define Biddy_GetConstantZero() Cudd_ReadLogicZero(manager)
#define Biddy_GetConstantOne() Cudd_ReadOne(manager)
#define Biddy_GetElse(f) Cudd_E(f)
#define Biddy_GetThen(f) Cudd_T(f)
#define Biddy_AddVariable() Cudd_bddNewVar(manager)
#define Biddy_Not(f) Cudd_Not(f)
#define Biddy_Inv(f) Cudd_Not(f)
#define Biddy_And(f,g) Cudd_bddAnd(manager,f,g)
#define Biddy_Or(f,g) Cudd_bddOr(manager,f,g)
#define Biddy_Xnor(f,g) Cudd_bddXnor(manager,f,g)
#define Biddy_Support(f) Cudd_Support(manager,f)
#define Biddy_DependentVariableNumber(f) Cudd_SupportSize(manager,f)
#define Biddy_CountMinterm(f,n) Cudd_CountMinterm(manager,f,n)
#define Biddy_NodeNumber(f) Cudd_DagSize(f)
#define Biddy_PrintInfo(s) Cudd_PrintInfo(manager,s)
#endif

extern void setDataUSA(unsigned int *size, unsigned int ***usa, unsigned int **order, char **codes);
extern void setDataEurope(unsigned int *size, unsigned int ***europe, unsigned int **order, char **codes);
static void createGraph(unsigned int size, unsigned int **edge, Biddy_Edge *state, Biddy_Edge *graph);
static Biddy_Edge calculateIndependence(unsigned int size, Biddy_Edge *state, Biddy_Edge *graph);
static Biddy_Edge calculateKernels(unsigned int size, Biddy_Edge *state, Biddy_Edge *graph);
static void writeOrder(Biddy_Edge f, char *codes, unsigned int *order, unsigned int size);

int main(int argc, char** argv) {
  unsigned int i;
  Biddy_Boolean complete;
  Biddy_Edge tmp;
  Biddy_Edge r1,r2;
  unsigned int n1,n2;
  char *userinput;

  clock_t elapsedtime;

#ifdef USA_YES
  unsigned int usaSize = 0;
  unsigned int **usaEdge;
  unsigned int *usaOrder;
  char *usaCodes;
  Biddy_Edge *usaState;
  Biddy_Edge *usaGraph;
#endif

#ifdef EUROPE_YES
  unsigned int europeSize = 0;
  unsigned int **europeEdge;
  unsigned int *europeOrder;
  char *europeCodes;
  Biddy_Edge *europeState;
  Biddy_Edge *europeGraph;
#endif

  setbuf(stdout, NULL);

#ifdef USA_YES
  setDataUSA(&usaSize,&usaEdge,&usaOrder,&usaCodes);
#endif

#ifdef EUROPE_YES
  setDataEurope(&europeSize,&europeEdge,&europeOrder,&europeCodes);
#endif

#ifdef USE_BIDDY
  /* There is only one unique table in Biddy */
  /* There are three caches in Biddy */
  /* Unique table grows over the time */
  /* The max number of variables is hardcoded in biddyInt. h */
  /* biddyVariableTable.usaSize = BIDDYVARMAX = 2048 */
  /* The following constants are hardcoded in biddyMain.c */
  /* biddyIteCache.usaSize = MEDIUM_TABLE = 262143 */
  /* biddyEACache.usaSize = SMALL_TABLE = 65535 */
  /* biddyRCCache.usaSize = SMALL_TABLE = 65535 */
  /* DEFAULT INIT CALL: Biddy_InitAnonymous(BIDDYTYPEOBDD) */
  Biddy_InitAnonymous(BIDDYTYPEOBDD);
  if (argc == 1) {
    Biddy_SetManagerParameters(-1,-1,-1,-1,-1,-1,-1,-1,-1,-1);
  }
  else if (argc == 7) {
    int gcr,gcrF,gcrX;
    int rr,rrF,rrX;
    sscanf(argv[1],"%d",&gcr);
    sscanf(argv[2],"%d",&gcrF);
    sscanf(argv[3],"%d",&gcrX);
    sscanf(argv[4],"%d",&rr);
    sscanf(argv[5],"%d",&rrF);
    sscanf(argv[6],"%d",&rrX);
    Biddy_SetManagerParameters(gcr/100.0,gcrF/100.0,gcrX/100.0,rr/100.0,rrF/100.0,rrX/100.0,-1,-1,-1,-1);
  }
  else if (argc == 11) {
    int gcr,gcrF,gcrX;
    int rr,rrF,rrX;
    int st;
    int fst;
    int cst;
    int fcst;
    sscanf(argv[1],"%d",&gcr);
    sscanf(argv[2],"%d",&gcrF);
    sscanf(argv[3],"%d",&gcrX);
    sscanf(argv[4],"%d",&rr);
    sscanf(argv[5],"%d",&rrF);
    sscanf(argv[6],"%d",&rrX);
    sscanf(argv[7],"%d",&st);
    sscanf(argv[8],"%d",&fst);
    sscanf(argv[9],"%d",&cst);
    sscanf(argv[10],"%d",&fcst);
    Biddy_SetManagerParameters(gcr/100.0,gcrF/100.0,gcrX/100.0,rr/100.0,rrF/100.0,rrX/100.0,st/100.0,fst/100.0,cst/100.0,fcst/100.0);
  }
  else {
    printf("Wrong number of parameters!\n");
    exit(0);
  }
#endif

#ifdef USE_CUDD
  /* In CUDD each variable has its own subtable in the unique table */
  /* There is only one cache in CUDD */
  /* Subtables grow over the time, you can set limit for fast unique table growth */
  /* Cudd_SetLooseUpTo(manager,1048576) */
  /* Cache can grow over the time, you can set the max usaSize */
  /* Cudd_SetMaxCacheHard(manager,262144) */
  /* These two constants are hardcoded in v3.0.0 */
  /* CUDD_UNIQUE_SLOTS = 256 (default initial usaSize of each subtable) */
  /* CUDD_CACHE_SLOTS = 262144 (default initial usaSize of cache table) */
  /* DEFAULT INIT CALL: Cudd_Init(0,0,CUDD_UNIQUE_SLOTS,CUDD_CACHE_SLOTS,0) */
  manager = Cudd_Init(0,0,CUDD_UNIQUE_SLOTS,CUDD_CACHE_SLOTS,0);
  Cudd_SetMaxCacheHard(manager,262144);
#endif

#ifdef USA_YES
  usaGraph = (Biddy_Edge *) malloc(usaSize*sizeof(Biddy_Edge *));
  usaState = (Biddy_Edge *) malloc(usaSize*sizeof(Biddy_Edge *));
#endif

#ifdef EUROPE_YES
  europeGraph = (Biddy_Edge *) malloc(europeSize*sizeof(Biddy_Edge *));
  europeState = (Biddy_Edge *) malloc(europeSize*sizeof(Biddy_Edge *));
#endif

  i = 0;
  complete = FALSE;
  while (!complete) {
    complete = TRUE;
    tmp = Biddy_AddVariable();
#ifdef USA_YES
    if (i < usaSize) {
      usaState[usaOrder[i]] = tmp;
    }
    complete = complete && (i >= (usaSize-1));
#endif
#ifdef EUROPE_YES
    if (i < europeSize) {
      europeState[europeOrder[i]] = tmp;
    }
    complete = complete && (i >= (europeSize-1));
#endif
    i++;
  }

  /* for ZBDDs, by adding new variables all the existing ones are changed */
  /* after adding the last variable we have to refresh them all */
  /* Additionaly, for ZBDD the order of variables is reverted to get consistent results */
  /* usaState[i] and europeState[i] are single variable, not necessary to explicitly preserve */
  /* NOTE: user variables in Biddy start with index 1 */
#ifdef USE_BIDDY
  if (Biddy_GetManagerType() == BIDDYTYPEZBDD) {
    i = 0;
    complete = FALSE;
    while (!complete) {
      complete = TRUE;
#ifdef USA_YES
      if (i < usaSize) {
        usaState[usaOrder[i]] = Biddy_GetVariableEdge(usaSize-i);
        /* Biddy_AddTmpFormula(usaState[usaOrder[i]],0); */
      }
      complete = complete && (i >= (usaSize-1));
#endif
#ifdef EUROPE_YES
      if (i < europeSize) {
        europeState[europeOrder[i]] = Biddy_GetVariableEdge(europeSize-i);
        /* Biddy_AddTmpFormula(europeState[europeOrder[i]],0); */
      }
      complete = complete && (i >= (europeSize-1));
#endif
      i++;
    }
    Biddy_Purge(); /* remove unnecessary nodes */
  }
#endif

  /* HERE, BDD OPERATIONS START AND THUS WE START TO MEASURE THE TIME */
  elapsedtime = clock();

#ifdef USA_YES
  createGraph(usaSize,usaEdge,usaState,usaGraph);
#endif

#ifdef EUROPE_YES
  createGraph(europeSize,europeEdge,europeState,europeGraph);
#endif

  r1 = Biddy_GetConstantZero();
  r2 = Biddy_GetConstantZero();
#ifdef USE_CUDD
  Cudd_Ref(r1);
  Cudd_Ref(r2);
#endif

#ifdef USA_YES
  if (!CALCULATE_KERNELS) {
    /* CALCULATING INDEPENDENCE SETS FOR USA */
#ifdef USE_CUDD
    Cudd_RecursiveDeref(manager,r1);
#endif
    r1 = calculateIndependence(usaSize,usaState,usaGraph);
  } else {
    /* CALCULATING KERNELS (MAXIMUM INDEPENDENCE SETS) FOR USA */
#ifdef USE_CUDD
    Cudd_RecursiveDeref(manager,r1);
#endif
    r1 = calculateKernels(usaSize,usaState,usaGraph);
  }
#ifdef USE_BIDDY
  Biddy_AddPersistentFormula((Biddy_String)"usa",r1);
#endif
#ifdef USE_CUDD
  for (i=0; i<usaSize; i++) {
    Cudd_RecursiveDeref(manager,usaGraph[i]);
  }
#endif
#endif

#ifdef EUROPE_YES
  if (!CALCULATE_KERNELS) {
    /* CALCULATING INDEPENDENCE SETS FOR EUROPE */
#ifdef USE_CUDD
    Cudd_RecursiveDeref(manager,r2);
#endif
    r2 = calculateIndependence(europeSize,europeState,europeGraph);
  } else {
    /* CALCULATING KERNELS (MAXIMUM INDEPENDENCE SETS) FOR EUROPE */
#ifdef USE_CUDD
    Cudd_RecursiveDeref(manager,r2);
#endif
    r2 = calculateKernels(europeSize,europeState,europeGraph);
  }
#ifdef USE_BIDDY
  Biddy_AddPersistentFormula((Biddy_String)"europe",r2);
#endif
#ifdef USE_CUDD
  for (i=0; i<europeSize; i++) {
    Cudd_RecursiveDeref(manager,europeGraph[i]);
  }
#endif
#endif

#ifdef USE_BIDDY
  /* TO GET RELEVANT TIME FOR SIFTING WE HAVE TO REMOVE UNNECESSARY NODES */
  /* this will remove all tmp formulae created by createGraph() */
  Biddy_Purge();
#endif

  userinput = strdup("...");
  while (userinput[0] != 'x') {

    /* We have problems with passing stdout in the case you compile this file */
    /* with MINGW and use biddy.dll generated with Visual Studio. */
    /* In such cases, please, use Biddy_PrintInfo(NULL) */
    if (userinput[0] == 'r') Biddy_PrintInfo(stdout);

    /* SIFTING  */
#ifdef USE_BIDDY
#ifdef CONVERGE
    if (userinput[0] == 's') Biddy_Sifting(NULL,TRUE);
#ifdef USA_YES
    if (userinput[0] == 'u') Biddy_Sifting(r1,TRUE);
#endif
#ifdef EUROPE_YES
    if (userinput[0] == 'e') Biddy_Sifting(r2,TRUE);
#endif
#else
    if (userinput[0] == 's') Biddy_Sifting(NULL,FALSE);
#ifdef USA_YES
    if (userinput[0] == 'u') Biddy_Sifting(r1,FALSE);
#endif
#ifdef EUROPE_YES
    if (userinput[0] == 'e') Biddy_Sifting(r2,FALSE);
#endif
#endif
    /* for TZBDD, sifting may change top node! */
#ifdef USA_YES
    if (!Biddy_FindFormula((Biddy_String)"usa",&r1)) exit(99);
#endif
#ifdef EUROPE_YES
    if (!Biddy_FindFormula((Biddy_String)"europe",&r2)) exit(99);
#endif
#endif
#ifdef USE_CUDD
#ifdef CONVERGE
    if (userinput[0] == 's') Cudd_ReduceHeap(manager,CUDD_REORDER_SIFT_CONVERGE,0);
#else
    if (userinput[0] == 's') Cudd_ReduceHeap(manager,CUDD_REORDER_SIFT,0);
#endif
#endif

    n1 = Biddy_DependentVariableNumber(r1);
    n2 = Biddy_DependentVariableNumber(r2);

    if ((userinput[0] == 's')
#ifdef USA_YES
        || (userinput[0] == 'u')
#endif
#ifdef EUROPE_YES
        || (userinput[0] == 'e')
#endif
    ) {
#ifdef CONVERGE
      printf("(CONVERGING SIFTING");
#ifdef USE_BIDDY
      if (userinput[0] == 'u') {
        printf(" ON FUNCTION FOR USA");
      }
      if (userinput[0] == 'e') {
        printf(" ON FUNCTION FOR EUROPE");
      }
#endif
#ifdef PROFILE
      printf("), ");
#else
      printf(")\n");
#endif
#else
      printf("(SIFTING");
#ifdef USE_BIDDY
      if (userinput[0] == 'u') {
        printf(" ON FUNCTION FOR USA");
      }
      if (userinput[0] == 'e') {
        printf(" ON FUNCTION FOR EUROPE");
      }
#endif
#ifdef PROFILE
      printf("), ");
#else
      printf(")\n");
#endif
#endif
    }

#ifndef PROFILE
    printf("Resulting function r1/r2 depends on %u/%u variables.\n",n1,n2);
    printf("Resulting function r1/r2 has %.0f/%.0f minterms.\n",Biddy_CountMinterm(r1,n1),Biddy_CountMinterm(r2,n2));
    printf("BDD system has %u nodes.\n",Biddy_NodeTableNum());
#ifdef USE_BIDDY
    if (Biddy_GetManagerType() == BIDDYTYPEOBDD) {
      printf("OBDD");
    } else if (Biddy_GetManagerType() == BIDDYTYPEZBDD) {
      printf("ZBDD");
    } else if (Biddy_GetManagerType() == BIDDYTYPETZBDD) {
      printf("TZBDD");
    }
    printf(" for resulting function r1/r2 has %u/%u nodes (%u/%u nodes if using complement edges).\n",
           Biddy_NodeNumberPlain(r1),Biddy_NodeNumberPlain(r2),Biddy_NodeNumber(r1),Biddy_NodeNumber(r2));
#endif
#ifdef USE_CUDD
    printf("BDD for resulting function r1/r2 has %u/%u nodes (using complement edges).\n",Biddy_NodeNumber(r1),Biddy_NodeNumber(r2));
#endif
#ifdef USE_BIDDY
    printf("Variable swaps performed so far: %u\n",Biddy_NodeTableSwapNumber());
#endif
#ifdef USE_CUDD
    printf("Variable swaps performed so far: %.0f\n",Cudd_ReadSwapSteps(manager));
#endif
#endif
#ifndef PROFILE
#ifdef USA_YES
    printf("Order for USA: ");
    writeOrder(r1,usaCodes,usaOrder,usaSize);
#endif
#ifdef EUROPE_YES
    printf("Order for Europe: ");
    writeOrder(r2,europeCodes,europeOrder,europeSize);
#endif
#endif
#ifndef PROFILE
#ifdef USE_BIDDY
    printf("E[x]it or [r]eport or ");
#ifdef CONVERGE
    printf("Converging sifting ");
#else
    printf("Sifting ");
#endif
#if (defined USA_YES) || (defined EUROPE_YES)
    printf("on [s]ystem or on function for");
#ifdef USA_YES
    printf(" [u]SA");
#endif
#ifdef EUROPE_YES
    printf(" [e]urope");
#endif
#endif
#endif
#ifdef USE_CUDD
    printf("E[x]it or [r]eport or ");
#ifdef CONVERGE
    printf("Converging sifting ");
#else
    printf("Sifting ");
#endif
    printf("on [s]ystem");
#endif
    printf(": ");
#endif
#ifdef PROFILE
    if (!strcmp(userinput,"r")) userinput = strdup("x");
    if (!strcmp(userinput,"s")) userinput = strdup("x");
    if (!strcmp(userinput,"...")) userinput = strdup("s");
#else
    if (!scanf("%s",userinput)) printf("ERROR\n");
#endif
  }

  elapsedtime = clock()-elapsedtime;

  free(userinput);

  /* PROFILING */
  /**/
#ifdef USE_BIDDY
#ifdef MINGW
  fprintf(stderr,"%I64u, %.2f",Biddy_ReadMemoryInUse(),Biddy_NodeTableDRTime())/(1.0*CLOCKS_PER_SEC);
#else
  fprintf(stderr,"%llu, %.2f",Biddy_ReadMemoryInUse(),Biddy_NodeTableDRTime()/(1.0*CLOCKS_PER_SEC));
#endif
  printf(", %u/%u",Biddy_NodeNumber(r1),Biddy_NodeNumber(r2));
  fprintf(stderr,"\n");
#endif

  /* EXIT */
#ifdef USE_BIDDY
  Biddy_Exit();
#endif
#ifdef USE_CUDD
  Cudd_RecursiveDeref(manager,r1);
  Cudd_RecursiveDeref(manager,r2);
  printf("CUDD: nodes with non-zero reference counts: %d\n",Cudd_CheckZeroRef(manager));
  Cudd_Quit(manager);
#endif

  return 0;
}

/* CREATE GRAPH */
static void createGraph(unsigned int size, unsigned int **edge, Biddy_Edge *state, Biddy_Edge *graph) {
  unsigned int i,j;
  Biddy_Edge tmp;
  /*
  printf("CREATE GRAPH... ");
  */
  for (i=0; i<size; i++) {
    graph[i] = Biddy_GetConstantZero();
#ifdef USE_CUDD
    Cudd_Ref(graph[i]);
#endif
    for (j=0; j<size; j++) {
      if (edge[i][j]) {
        tmp = Biddy_Or(graph[i],state[j]);
#ifdef USE_CUDD
        Cudd_Ref(tmp);
        Cudd_RecursiveDeref(manager,graph[i]);
#endif
        graph[i] = tmp;
      }
    }
#ifdef USE_BIDDY
    Biddy_AddTmpFormula(graph[i],0);
#endif
  }
  /*
  printf("OK\n");
  */
}

/* CALCULATE INDEPENDENCE SETS */
static Biddy_Edge calculateIndependence(unsigned int size, Biddy_Edge *state, Biddy_Edge *graph) {
  unsigned int i;
  Biddy_Edge p,r,tmp;
  /*
  printf("CALCULATE INDEPENDENCE SETS...\n");
  */
  r = Biddy_GetConstantZero();
#ifdef USE_CUDD
  Cudd_Ref(r);
#endif
  for (i=0; i<size; i++) {
    p = Biddy_And(graph[i],state[i]);
#ifdef USE_CUDD
    Cudd_Ref(p);
#endif
    tmp = Biddy_Or(r,p);
#ifdef USE_BIDDY
    Biddy_AddTmpFormula(tmp,1);
    Biddy_Clean();
#endif
#ifdef USE_CUDD
    Cudd_Ref(tmp);
    Cudd_RecursiveDeref(manager,r);
    Cudd_RecursiveDeref(manager,p);
    /* Cudd_ReduceHeap(manager,CUDD_REORDER_SIFT,0); */
#endif
    r = tmp;
  }
  return Biddy_Not(r);
}

/* CALCULATE KERNELS (MAXIMUM INDEPENDENCE SETS) */
static Biddy_Edge calculateKernels(unsigned int size, Biddy_Edge *state, Biddy_Edge *graph) {
  unsigned int i;
  Biddy_Edge p,r,tmp;
  /*
  printf("CALCULATE KERNELS (MAXIMUM INDEPENDENCE SETS)...\n");
  */
  r = Biddy_GetConstantZero();
#ifdef USE_CUDD
  Cudd_Ref(r);
#endif
  for (i=0; i<size; i++) {
    p = Biddy_Xnor(graph[i],state[i]);
#ifdef USE_CUDD
    Cudd_Ref(p);
#endif
    tmp = Biddy_Or(r,p);
#ifdef USE_BIDDY
    Biddy_AddTmpFormula(tmp,1);
    Biddy_Clean();
#endif
#ifdef USE_CUDD
    Cudd_Ref(tmp);
    Cudd_RecursiveDeref(manager,r);
    Cudd_RecursiveDeref(manager,p);
    /* Cudd_ReduceHeap(manager,CUDD_REORDER_SIFT,0); */
#endif
    r = tmp;
  }
  return Biddy_Inv(r);
}

static void writeOrder(Biddy_Edge f, char *codes, unsigned int *order, unsigned int size) {
  char ch;
#ifdef USE_CUDD
  int i, j;
#endif
  int k;
  unsigned int ivar = 0;

#ifdef USE_BIDDY
  /* TO DO: IMPLEMENT Biddy_ReadPerm() AND ITS INVERSE */
  {
  Biddy_Edge tmp;
  tmp = Biddy_Support(f);
  while (!Biddy_IsConstant(tmp)) {
    /* In Biddy, the first user variable is at index [1] */
    if (Biddy_GetManagerType() == BIDDYTYPEOBDD) {
      ivar = Biddy_GetTopVariable(tmp)-1;
    } else if (Biddy_GetManagerType() == BIDDYTYPEZBDD) {
      ivar = size - Biddy_GetTopVariable(tmp);
    } else if (Biddy_GetManagerType() == BIDDYTYPETZBDD) {
      ivar = Biddy_GetTopVariable(tmp)-1;
    }
    k = 3*(order[ivar]);
    ch = codes[k+2];
    codes[k+2] = 0;
    printf("%s",&codes[k]);
    codes[k+2] = ch;
    tmp = Biddy_GetThen(tmp);
    if (!Biddy_IsConstant(tmp)) printf(","); else printf("\n");
  }
  Biddy_Purge(); /* we do not want that removing tmp nodes is part of benchmarks */
  }
#endif

#ifdef USE_CUDD
  /* HOW TO AVOID THIS DOUBLE LOOP ??? */
  for (i=0; i<size; i++) {
    /* In CUDD, the first user variable is at index [0] */
    for (j=0; j<size; j++) {
      if (i == Cudd_ReadPerm(manager,j)) ivar = j;
    }
    k = 3*(order[ivar]);
    ch = codes[k+2];
    codes[k+2] = 0;
    printf("%s", &codes[k]);
    codes[k+2] = ch;
    if (i != (size-1)) printf(","); else printf("\n");
  }
#endif

}

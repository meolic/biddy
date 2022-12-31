/* $Revision: 658 $ */
/* $Date: 2021-09-12 20:08:44 +0200 (ned, 12 sep 2021) $ */
/* This file (biddy-example-independence.c) is a C file */
/* Author: Robert Meolic (robert@meolic.com) */
/* This file has been released into the public domain by the author. */

/* This example is compatible with Biddy v2.0.2 (and laters) and CUDD v3.0.0 */

/* COMPILE WITH (ADD -lgmp IF USING STATIC BIDDY LIBRARY): */
/* gcc -DNOCONVERGE -DNOPROFILE -DUNIX -DBIDDY -DOBDDC -O2 -o biddy-example-independence biddy-example-independence.c biddy-example-independence-usa.c biddy-example-independence-europe.c -I. -L./bin -lbiddy */
/* gcc -DCONVERGE -DNOPROFILE -DUNIX -DBIDDY -DOBDDC -O2 -o biddy-example-independence-converge biddy-example-independence.c biddy-example-independence-usa.c biddy-example-independence-europe.c -I. -L./bin -lbiddy */
/* gcc -DNOCONVERGE -DNOPROFILE -DCUDD -DOBDDC -O2 -o cudd-example-independence biddy-example-independence.c biddy-example-independence-usa.c biddy-example-independence-europe.c -I ../cudd/include/ -L ../cudd/lib/ -lcudd -lm */
/* gcc -DCONVERGE -DNOPROFILE -DCUDD -DOBDDC -O2 -o cudd-example-independence-converge biddy-example-independence.c biddy-example-independence-usa.c biddy-example-independence-europe.c -I ../cudd/include/ -L ../cudd/lib/ -lcudd -lm */

/* HERE ARE SOME RESULTS FROM THE PROGRAM */
/* Resulting function r1/r2 depends on 49/46 variables. */
/* Resulting function r1/r2 has 211954906/546220896 minterms. */

/* SYSTEM BEST */
/* System has 5164 nodes / 0 live nodes. */
/* ROBDD for resulting function r1/r2 has 1878/1984 nodes (1878/1984 plain nodes). */
/* Order for USA: KS,OH,NE,SD,IA,WI,IL,WY,ND,MN,KY,MI,LA,IN,MO,MT,ID,UT,OR,WA,AZ,OK,AL,NM,CO,MS,TN,TX,GA,FL,SC,NC,WV,DE,AR,NH,ME,CA,NY,NV,CT,PA,NJ,VT,DC,VA,MD,RI,MA */
/* Order for Europe: FI,NO,LV,SE,DE,EE,LU,IE,FR,HU,GB,ES,IT,LI,DK,SM,PT,VA,AT,PL,AD,ME,BA,KZ,SI,SK,CZ,CH,RU,LT,BY,AL,MC,HR,AZ,NL,MK,BE,RO,MD,UA,BG,TR,GR,RS,GE */

/* EUROPE BEST */
/* System has 92419 nodes / 0 live nodes. */
/* ROBDD for resulting function r1/r2 has 90887/229 nodes (90887/229 plain nodes). */
/* Order for USA: WI,MN,LA,IN,KY,AL,OR,WY,ND,NY,IA,CT,FL,ID,MO,NH,AZ,MT,TN,WA,UT,GA,TX,OK,VT,ME,MI,DE,NE,IL,NC,OH,KS,SD,CO,NJ,PA,DC,AR,NM,NV,MD,RI,WV,SC,VA,MS,CA,MA */
/* Order for Europe: IE,GB,ES,FR,AD,PT,LU,NL,DE,BE,CH,DK,IT,MC,AT,LI,SI,VA,SM,CZ,SK,PL,UA,HR,HU,BY,LV,EE,LT,NO,FI,SE,BA,MD,RO,BG,AL,ME,MK,GR,RS,RU,TR,KZ,AZ,GE */

/* USA BEST */
/* System has 36344 nodes / 0 live nodes. */
/* ROBDD for resulting function r1/r2 has 341/34692 nodes (341/34692 plain nodes). */
/* Order for USA: ID,NV,OR,WA,AZ,CA,UT,NM,WY,CO,MT,SD,MN,ND,IA,WI,MO,KS,NE,TX,AR,OK,LA,TN,KY,MI,IN,IL,AL,MS,FL,NC,SC,GA,OH,WV,MD,DC,VA,PA,NJ,DE,MA,VT,ME,NH,CT,RI,NY */
/* Order for Europe: DK,MK,PT,VA,AT,AZ,SM,ME,BA,LI,SE,IE,LU,DE,IT,FI,LV,SK,AL,PL,GB,SI,FR,HU,ES,EE,AD,KZ,CH,LT,RU,CZ,NO,GR,BG,TR,RO,MD,BY,GE,UA,HR,MC,BE,RS,NL */

/* define CALCULATE_KERNELS 0 OR 1 */
/* define USA_NO or USA_YES */
/* define EUROPE_NO or EUROPE_YES */
#define CALCULATE_KERNELS 0
#define USA_YES
#define EUROPE_YES

#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <time.h>

#include "biddy-cudd.h"

extern void setDataUSA(unsigned int *size, unsigned int ***usa, unsigned int **order, char **codes);
extern void setDataEurope(unsigned int *size, unsigned int ***europe, unsigned int **order, char **codes);
static void createGraph(unsigned int size, unsigned int **edge, Biddy_Edge *state, Biddy_Edge *graph);
static Biddy_Edge calculateIndependence(unsigned int size, Biddy_Edge *state, Biddy_Edge *graph);
static Biddy_Edge calculateKernels(unsigned int size, Biddy_Edge *state, Biddy_Edge *graph);
static void writeOrder(char *codes, unsigned int *order, unsigned int size);

int main(int argc, char** argv) {
  unsigned int i;
  Biddy_Boolean complete;
  Biddy_Edge tmp;
  Biddy_Edge r1,r2;
  unsigned int n1,n2;
  char *userinput;
  long long unsigned memoryinuse;

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

#include "biddy-cudd.c" /* this will initialize BDD manager */

#ifdef BIDDY
  if (argc == 1) {
    Biddy_SetManagerParameters(-1,-1,-1,-1,-1,-1,-1,-1);
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
    Biddy_SetManagerParameters(gcr/100.0,gcrF/100.0,gcrX/100.0,rr/100.0,rrF/100.0,rrX/100.0,-1,-1);
    printf("%d, %d, %d, %d, %d, %d, ",gcr,gcrF,gcrX,rr,rrF,rrX);
  }
  else if (argc == 10) {
    int gcr,gcrF,gcrX;
    int rr,rrF,rrX;
    int st,cst;
    sscanf(argv[1],"%d",&gcr);
    sscanf(argv[2],"%d",&gcrF);
    sscanf(argv[3],"%d",&gcrX);
    sscanf(argv[4],"%d",&rr);
    sscanf(argv[5],"%d",&rrF);
    sscanf(argv[6],"%d",&rrX);
    sscanf(argv[7],"%d",&st);
    sscanf(argv[8],"%d",&cst);
    Biddy_SetManagerParameters(gcr/100.0,gcrF/100.0,gcrX/100.0,rr/100.0,rrF/100.0,rrX/100.0,st/100.0,cst/100.0);
    printf("%d, %d, %d, %d, %d, %d, ",gcr,gcrF,gcrX,rr,rrF,rrX);
  }
  else {
    printf("Wrong number of parameters!\n");
    exit(0);
  }
#endif

#ifdef CUDD
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

  /* DEBUGGING */
  /*
  printf("Before creating variables:\n");
  printf("  biddyZero has %u nodes.\n",Biddy_CountNodes(Biddy_GetConstantZero()));
  printf("  biddyZero has %u complemented edges.\n",Biddy_CountComplementedEdges(Biddy_GetConstantZero()));
  printf("  biddyOne has %u nodes.\n",Biddy_CountNodes(Biddy_GetConstantOne()));
  printf("  biddyOne has %u complemented edges.\n",Biddy_CountComplementedEdges(Biddy_GetConstantOne()));
  */

  /* CREATE OBDD VARIABLES */
  if ((Biddy_GetManagerType() == BIDDYTYPEOBDD) || (Biddy_GetManagerType() == BIDDYTYPEOBDDC))
  {
    i = 0;
    complete = FALSE;
    while (!complete) {
      complete = TRUE;
      tmp = Biddy_AddVariableEdge();
#ifdef USA_YES
      if (i < usaSize) {
        usaState[usaOrder[i]] = tmp;
        REF(usaState[usaOrder[i]]);
      }
      complete = complete && (i >= (usaSize-1));
#endif
#ifdef EUROPE_YES
      if (i < europeSize) {
        europeState[europeOrder[i]] = tmp;
        REF(europeState[europeOrder[i]]);
      }
      complete = complete && (i >= (europeSize-1));
#endif
      i++;
    }
  }

  /* CREATE ZBDD AND TZBDD VARIABLES */
  /* NOTE for Biddy: ZBDDs and TZBDDs variables are added in the reverse order by default */
  /* NOTE for Biddy: usaState[0] = europeState[0] and it is last added */
  /* NOTE for CUDD: usaState[0] = europeState[0] and it is first added */
  if ((Biddy_GetManagerType() == BIDDYTYPEZBDD) || (Biddy_GetManagerType() == BIDDYTYPEZBDDC) ||
      (Biddy_GetManagerType() == BIDDYTYPETZBDD) || (Biddy_GetManagerType() == BIDDYTYPETZBDD))
  {
    i = 0;
    complete = FALSE;
    while (!complete) {
      complete = TRUE;
      tmp = Biddy_AddVariableEdge();
#ifdef USA_YES
      if (i < usaSize) {
#ifdef CUDD
        usaState[usaOrder[i]] = tmp;
        REF(usaState[usaOrder[i]]);
#endif
#ifdef BIDDY
        usaState[usaOrder[usaSize-i-1]] = tmp;
#endif
      }
      complete = complete && (i >= (usaSize-1));
#endif
#ifdef EUROPE_YES
      if (i < europeSize) {
#ifdef CUDD
        europeState[europeOrder[i]] = tmp;
        REF(europeState[europeOrder[i]]);
#endif
#ifdef BIDDY
        europeState[europeOrder[europeSize-i-1]] = tmp;
#endif
      }
      complete = complete && (i >= (europeSize-1));
#endif
      i++;
    }
  }

  /* For ZBDDs, by adding new variables all the existing ones are changed. */
  /* Thus, after adding the last variable we have to refresh the array of variables. */
  /* NOTE: Biddy_GetVariableEdge returns variables by the creation time, not by the order used in the BDD */
  /* NOTE for Biddy: user variables start with index 1 */
  /* NOTE for Biddy: variables have been created in the reverse order */
  /* NOTE for CUDD: user variables start with index 0 */
  if ((Biddy_GetManagerType() == BIDDYTYPEZBDD) || (Biddy_GetManagerType() == BIDDYTYPEZBDDC))
  {

#ifdef BIDDY
    i = 0;
    complete = FALSE;
    while (!complete) {
      complete = TRUE;
#ifdef USA_YES
      if (i < usaSize) {
        usaState[usaOrder[i]] = Biddy_GetVariableEdge(usaSize-i);
      }
      complete = complete && (i >= (usaSize-1));
#endif
#ifdef EUROPE_YES
      if (i < europeSize) {
        europeState[europeOrder[i]] = Biddy_GetVariableEdge(europeSize-i);
      }
      complete = complete && (i >= (europeSize-1));
#endif
      i++;
    }
    Biddy_Purge(); /* remove unnecessary nodes, this is useful only for ZBDDs */
#endif

#ifdef CUDD
    i = 0;
    complete = FALSE;
    while (!complete) {
      complete = TRUE;
#ifdef USA_YES
      if (i < usaSize) {
        DEREF(usaState[usaOrder[i]]);
        usaState[usaOrder[i]] = Biddy_GetVariableEdge(i);
        REF(usaState[usaOrder[i]]);
      }
      complete = complete && (i >= (usaSize-1));
#endif
#ifdef EUROPE_YES
      if (i < europeSize) {
        DEREF(europeState[europeOrder[i]]);
        europeState[europeOrder[i]] = Biddy_GetVariableEdge(i);
        REF(europeState[europeOrder[i]]);
      }
      complete = complete && (i >= (europeSize-1));
#endif
      i++;
    }
#endif

  }
  
  /* DEBUGGING */
  /*
  printf("After creating variables:\n");
  printf("  biddyZero has %u nodes.\n",Biddy_CountNodes(Biddy_GetConstantZero()));
  printf("  biddyZero has %u complemented edges.\n",Biddy_CountComplementedEdges(Biddy_GetConstantZero()));
  printf("  biddyOne has %u nodes.\n",Biddy_CountNodes(Biddy_GetConstantOne()));
  printf("  biddyOne has %u complemented edges.\n",Biddy_CountComplementedEdges(Biddy_GetConstantOne()));
  */

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
  REF(r1);
  REF(r2);

#ifdef USA_YES
  if (!CALCULATE_KERNELS) {
    /* CALCULATING INDEPENDENCE SETS FOR USA */
    DEREF(r1);
    r1 = calculateIndependence(usaSize,usaState,usaGraph);
  } else {
    /* CALCULATING KERNELS (MAXIMUM INDEPENDENCE SETS) FOR USA */
    DEREF(r1);
    r1 = calculateKernels(usaSize,usaState,usaGraph);
  }
  MARK_X("usa",r1);
  for (i=0; i<usaSize; i++) {
    DEREF(usaGraph[i]);
  }
#endif

#ifdef EUROPE_YES
  if (!CALCULATE_KERNELS) {
    /* CALCULATING INDEPENDENCE SETS FOR EUROPE */
    DEREF(r2);
    r2 = calculateIndependence(europeSize,europeState,europeGraph);
  } else {
    /* CALCULATING KERNELS (MAXIMUM INDEPENDENCE SETS) FOR EUROPE */
    DEREF(r2);
    r2 = calculateKernels(europeSize,europeState,europeGraph);
  }
  MARK_X("europe",r2);
  for (i=0; i<europeSize; i++) {
    DEREF(europeGraph[i]);
  }
#endif

  /* TO GET RELEVANT TIME FOR SIFTING WE REMOVE UNNECESSARY NODES */
  /* this will remove all tmp formulae created by createGraph() */
#ifdef BIDDY
  Biddy_Purge();
#endif
#ifdef CUDD
  /* TO DO */
#endif

  userinput = strdup("...");
  while (userinput[0] != 'x') {

    /* We have problems with passing stdout in the case you compile this file */
    /* with MINGW and use biddy.dll generated with Visual Studio. */
    /* In such cases, please, use Biddy_PrintInfo(NULL) */
    if (userinput[0] == 'r') {
      Biddy_PrintInfo(stdout);
    }

    /* SIFTING  */
#ifdef CONVERGE
    if (userinput[0] == 's') Biddy_GlobalConvergedSifting();
#ifdef BIDDY
#ifdef USA_YES
    if (userinput[0] == 'u') Biddy_Sifting(r1,TRUE);
#endif
#ifdef EUROPE_YES
    if (userinput[0] == 'e') Biddy_Sifting(r2,TRUE);
#endif
#endif
#else
    if (userinput[0] == 's') Biddy_GlobalSifting();
#ifdef BIDDY
#ifdef USA_YES
    if (userinput[0] == 'u') Biddy_Sifting(r1,FALSE);
#endif
#ifdef EUROPE_YES
    if (userinput[0] == 'e') Biddy_Sifting(r2,FALSE);
#endif
#endif
#endif
    /* Biddy only: for TZBDDs, sifting may change top node! */
#ifdef BIDDY
#ifdef USA_YES
    if (!Biddy_FindFormula((Biddy_String)"usa",&i,&r1)) exit(99);
#endif
#ifdef EUROPE_YES
    if (!Biddy_FindFormula((Biddy_String)"europe",&i,&r2)) exit(99);
#endif
#endif

    n1 = Biddy_DependentVariableNumber(r1,FALSE);
    n2 = Biddy_DependentVariableNumber(r2,FALSE);

    if ((userinput[0] == 's')
#ifdef BIDDY
#ifdef USA_YES
        || (userinput[0] == 'u')
#endif
#ifdef EUROPE_YES
        || (userinput[0] == 'e')
#endif
#endif
    ) {
#ifdef CONVERGE
      printf("(CONVERGING SIFTING");
#ifdef BIDDY
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
#ifdef BIDDY
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

    printf("Time = %.2f\n",(clock()-elapsedtime)/(1.0*CLOCKS_PER_SEC));

    /* DEBUGGING */
    /*
    printf("Starting the interactive part... use 'x' to finish the program.\n");
    */

    printf("Resulting function r1/r2 depends on %u/%u variables.\n",n1,n2);
    printf("Resulting function r1/r2 has %.0f/%.0f minterms.\n",Biddy_CountMinterms(r1,n1),Biddy_CountMinterms(r2,n2));
    printf("System has %u total nodes and %lu live nodes.\n",Biddy_NodeTableNum(),Biddy_NodeTableNumLive());
#ifdef BIDDY
    printf("%s",Biddy_GetManagerName());
    printf(" for resulting function r1/r2 has %u/%u nodes (%u/%u plain nodes).\n",
           Biddy_CountNodes(r1),Biddy_CountNodes(r2),Biddy_CountNodesPlain(r1),Biddy_CountNodesPlain(r2));
    if ((Biddy_GetManagerType() == BIDDYTYPEOBDDC) || (Biddy_GetManagerType() == BIDDYTYPEZBDDC))
    {
      printf("%s",Biddy_GetManagerName());
      printf(" for resulting function r1/r2 has %u/%u complemented edges.\n",
             Biddy_CountComplementedEdges(r1),Biddy_CountComplementedEdges(r2));
    }
#endif
#ifdef CUDD
    printf("BDD for resulting function r1/r2 has %u/%u nodes (using complement edges).\n",
           Biddy_CountNodes(r1),Biddy_CountNodes(r2));
#endif
#ifdef BIDDY
    printf("Variable swaps performed so far: %u\n",Biddy_NodeTableSwapNumber());
#endif
#ifdef CUDD
    printf("Variable swaps performed so far: %.0f\n",Cudd_ReadSwapSteps(manager));
#endif
#endif
#ifndef PROFILE
#ifdef USA_YES
    printf("Order for USA: ");
    writeOrder(usaCodes,usaOrder,usaSize);
#endif
#ifdef EUROPE_YES
    printf("Order for Europe: ");
    writeOrder(europeCodes,europeOrder,europeSize);
#endif
#endif
#ifndef PROFILE
#ifdef BIDDY
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
#ifdef CUDD
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
#ifdef PROFILE
  memoryinuse = Biddy_ReadMemoryInUse();
#ifdef MINGW
  printf("%I64u, %u",memoryinuse,Biddy_NodeTableDRTime());
#else
  printf("%llu, %u",memoryinuse,(unsigned int)Biddy_NodeTableDRTime());
#endif
  printf(", %u/%u",Biddy_CountNodes(r1),Biddy_CountNodes(r2));
  printf("\n");
#endif

  /* EXIT */

  DEREF(r1);
  DEREF(r2);
#ifdef USA_YES
  for (i=0; i<usaSize; i++) {
    DEREF(usaState[i]);
  }
#endif
#ifdef EUROPE_YES
  for (i=0; i<europeSize; i++) {
    DEREF(europeState[i]);
  }
#endif

#ifdef CUDD
  printf("CUDD: nodes with non-zero reference counts: %d\n",Cudd_CheckZeroRef(manager));
#endif

  Biddy_Exit();
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
    REF(graph[i]);
    for (j=0; j<size; j++) {
      if (edge[i][j]) {
        tmp = Biddy_Or(graph[i],state[j]);
        REF(tmp);
        DEREF(graph[i]);
        graph[i] = tmp;
      }
    }
    MARK_0(graph[i]);
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
  REF(r);
  for (i=0; i<size; i++) {
    p = Biddy_And(graph[i],state[i]);
    REF(p);
    tmp = Biddy_Or(r,p);
    MARK(tmp);
    SWEEP();
    REF(tmp);
    DEREF(r);
    DEREF(p);
    r = tmp;
  }
  tmp = Biddy_Not(r);
  MARK(tmp);
  SWEEP();
  REF(tmp);
  DEREF(r);
  r = tmp;
  return r;
}

/* CALCULATE KERNELS (MAXIMUM INDEPENDENCE SETS) */
static Biddy_Edge calculateKernels(unsigned int size, Biddy_Edge *state, Biddy_Edge *graph) {
  unsigned int i;
  Biddy_Edge p,r,tmp;
  /*
  printf("CALCULATE KERNELS (MAXIMUM INDEPENDENCE SETS)...\n");
  */
  r = Biddy_GetConstantZero();
  REF(r);
  for (i=0; i<size; i++) {
    p = Biddy_Xnor(graph[i],state[i]);
    REF(p);
    tmp = Biddy_Or(r,p);
    MARK(tmp);
    SWEEP();
    REF(tmp);
    DEREF(r);
    DEREF(p);
    r = tmp;
  }
  tmp = Biddy_Not(r);
  MARK(tmp);
  SWEEP();
  REF(tmp);
  DEREF(r);
  r = tmp;
  return r;
}

static void writeOrder(char *codes, unsigned int *order, unsigned int size) {
  char ch;
  unsigned int i;
#ifdef CUDD
  int j;
#endif
  int k;
  unsigned int numvar,ivar;
  unsigned int first;

  numvar = Biddy_VariableTableNum();
  first = 1;

  /* In Biddy, the first user variable is at index [1] */
  /* In CUDD, the first user variable is at index [0] */
#ifdef BIDDY
  for (i=1; i<numvar; i++) {
    ivar = Biddy_GetIthVariable(i)-1;
#endif
#ifdef CUDD
  for (i=0; i<numvar; i++) {
    ivar = Biddy_GetIthVariable(i);
#endif
    /*
    printf("(ith:%u=%u)",i,ivar);
    */
    if (ivar >= size) continue;

#ifdef BIDDY
    if ((Biddy_GetManagerType() == BIDDYTYPEOBDD) || (Biddy_GetManagerType() == BIDDYTYPEOBDDC))
    {
      k = 3*(order[ivar]);
    }
    else if ((Biddy_GetManagerType() == BIDDYTYPEZBDD) || (Biddy_GetManagerType() == BIDDYTYPEZBDDC) ||
        (Biddy_GetManagerType() == BIDDYTYPETZBDD) || (Biddy_GetManagerType() == BIDDYTYPETZBDD))
    {
      k = 3*(order[size-ivar-1]);
    }
    else {
      printf("ERROR: Unsupported BDD type\n");
      exit(1);
    }
#endif

#ifdef CUDD
    k = 3*(order[ivar]);
#endif

    ch = codes[k+2];
    codes[k+2] = 0;
    if (first) {
      first = 0;
    } else {
      printf(",");
    }
    printf("%s",&codes[k]);
    codes[k+2] = ch;
  }
  printf("\n");

}

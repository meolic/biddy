/* $Revision: 558 $ */
/* $Date: 2019-10-14 09:42:56 +0200 (pon, 14 okt 2019) $ */
/* This file (biddy-example-ta.c) is a C file */
/* Author: Robert Meolic (robert@meolic.com) */
/* This file has been released into the public domain by the author. */

/* This example is compatible with Biddy v1.8 */

/* COMPILE WITH (ADD -lgmp IF USING STATIC BIDDY LIBRARY): */
/* gcc -DUNIX -O2 -o biddy-example-ta biddy-example-ta.c -I. -L./bin -lbiddy */

/* this example is about the Task Allocation problem */

#include "biddy.h"
#include "string.h"
#include <time.h>

/* choose primary BDD type */
#define BDDTYPE BIDDYTYPEZBDDC

#define MAXDAYS 31
#define MAXTASKS 100

/* define PAPER to have variable names usable for the paper */
/* define LATEX to have variable names compatible with LaTeX (otherwise MS Word Equation style is used) */
#define PAPER
#define NOLATEX

/* define CHECKOPERATIONS for debugging single operations */
#define NOCHECKOPERATIONS

/* define INTERNALREPORTS for internal reports - they can be huge! */
#define NOINTERNALREPORTS

/* MAIN DATA STRUCTURES */

typedef struct LIMIT {
  unsigned int task;
  unsigned int day;
  unsigned int w;
} LIMIT;

typedef struct {
  unsigned int n[MAXDAYS+1];
  Biddy_Edge f;
  Biddy_Edge r; /* NULL = not valid node! */
} PermitEffortCache;

typedef struct {
  PermitEffortCache *table;
  unsigned int size;
} PermitEffortCacheTable;

/* const unsigned int permitEffortCacheSize = 1048573; */
const unsigned int permitEffortCacheSize = 4194304;
/* const unsigned int permitEffortCacheSize = 16777216; */

PermitEffortCacheTable permitEffortCache = {NULL,0};
long long unsigned int effortCacheSearch = 0;
long long unsigned int effortCacheHit = 0;

typedef struct VARIABLE {
  Biddy_String name;
  Biddy_String papername;
  LIMIT limit;
} VARIABLE;

unsigned int variableTableSize;
VARIABLE *variableTable;

char foavarname[128]; /* used to create variable name in find-or-add functions */
char papervarname[128]; /* used to create variable name using paper style */

/* USEFUL MACROS TO ENABLE NICE LATEX REPORT */

#ifdef LATEX
#define PRINTCOMBINATIONS(X) \
{\
printf("\\begin{small}\\begin{verbatim}");\
printf("%s has %u nodes (including terminals) and %.0f cubes",Biddy_GetManagerName(),Biddy_CountNodes(X),Biddy_CountMinterms(X,0));\
printf("\\end{verbatim}\\end{small}");\
printf("\\vspace*{-1em}\\begin{align*} ");\
if (Biddy_CountMinterms(X,0) > 99) printf("to large...\n"); else Biddy_PrintfMinterms(X,FALSE);\
printf("\\end{align*}");\
}
#else
#define PRINTCOMBINATIONS(X) \
{\
printf("%s has %u nodes (including terminals) and %.0f cubes\n",Biddy_GetManagerName(),Biddy_CountNodes(X),Biddy_CountMinterms(X,0));\
if (Biddy_CountMinterms(X,0) > 99) printf("to large...\n"); else Biddy_PrintfMinterms(X,FALSE);\
}
#endif

#ifdef LATEX
#define STATCOMBINATIONS(X) \
{\
printf("%s has %u nodes (including terminals) and %.0f cubes\n",Biddy_GetManagerName(),Biddy_CountNodes(X),Biddy_CountMinterms(X,0));\
}
#else
#define STATCOMBINATIONS(X) \
{\
printf("%s has %u nodes (including terminals) and %.0f cubes\n",Biddy_GetManagerName(),Biddy_CountNodes(X),Biddy_CountMinterms(X,0));\
}
#endif

static void
concat(Biddy_String *s1, const Biddy_String s2)
{
   *s1 = (Biddy_String) realloc(*s1,strlen(*s1)+strlen(s2)+1);
   strcat(*s1,s2);
}

static void
concatNumber(Biddy_String *s1, const int n)
{
   char s2[32];
   sprintf(s2,"%d",n);
   *s1 = (Biddy_String) realloc(*s1,strlen(*s1)+strlen(s2)+1);
   strcat(*s1,s2);
}

static void
concatStatic(Biddy_String s1, const Biddy_String s2)
{
   strcat(s1,s2);
}

static void
reportOrdering()
{
  Biddy_Boolean uselatex;
  Biddy_String activeordering;

  uselatex = FALSE;
#ifdef PAPER
#ifdef LATEX
  uselatex = TRUE;
#endif
#endif

  printf("\nVariable ordering:\n");
  activeordering = Biddy_GetOrdering();
  if (uselatex) printf("\\begin{align*}\n");
  printf("%s\n",activeordering);
  if (uselatex) printf("\\end{align*}\n");
  free(activeordering);
}

/* ************************************************************************** */
/* MANIPULATION OF VARIABLES  */
/* ************************************************************************** */

#define foaTvariable(i,d) findoraddTvariable(i,d,FALSE,FALSE)
#define findTvariable(i,d) findoraddTvariable(i,d,FALSE,TRUE)

#define ADDVARIABLE(i,d) \
{\
findoraddTvariable(i,d,TRUE,FALSE);\
}

void
addVariableTable(unsigned int *variableTableSize, VARIABLE **variableTable, VARIABLE var)
{
  unsigned int i;
  Biddy_Boolean FIND;
  int cc;

  FIND = FALSE;
  if (*variableTableSize == 0) {
    *variableTable = (VARIABLE *) malloc(sizeof(VARIABLE));
    (*variableTable)[0].name = strdup(var.name);
    (*variableTable)[0].papername = strdup(var.papername);
    (*variableTable)[0].limit = var.limit;
    (*variableTableSize)++;
    FIND = TRUE;
  } else {

    /* SEARCH FOR THE EXISTING VARIABLE WITH THE GIVEN NAME */
    for (i=0; !FIND && (i < *variableTableSize); i++) {
      cc = strcmp(var.name,(*variableTable)[i].name);
      if (cc == 0) {
        FIND = TRUE;
        break;
      }
      if (cc < 0) break; /* > means increasing order, < means decreasing order */
    }
  }

  /* IF FIND THEN i IS THE INDEX OF THE CORRECT VARIABLE! */
  if (!FIND) {
    (*variableTableSize)++;
    *variableTable = (VARIABLE *) realloc(*variableTable,(*variableTableSize)*sizeof(VARIABLE));
    memmove(&(*variableTable)[i+1],&(*variableTable)[i],((*variableTableSize)-i-1)*sizeof(VARIABLE));
    (*variableTable)[i].name = strdup(var.name);
    (*variableTable)[i].papername = strdup(var.papername);
    (*variableTable)[i].limit = var.limit;
  }
}

Biddy_Variable
foaVariable(VARIABLE var, Biddy_Boolean findOnly)
{
  Biddy_String name;
  Biddy_Variable v;
  LIMIT *data;

#ifdef PAPER
  name = var.papername;
#else
  name = var.name;
#endif

  if (!(v = Biddy_GetVariable(name))) {
    if (!findOnly) {

      /*
      printf("WARNING (foaVariable): variable %s added!\n",name);
      */

      v = Biddy_FoaVariable(name,FALSE); /* FALSE means addElement */
      if (var.limit.day) {
        data = (LIMIT *) malloc(sizeof(LIMIT));
        data->task = var.limit.task;
        data->day = var.limit.day;
        data->w = var.limit.w;
        Biddy_SetVariableData(v,(void *) data);
      }
    }
  }

  return v;
}

Biddy_Variable
findoraddTvariable(unsigned int i, unsigned int d, 
                   Biddy_Boolean addTable, Biddy_Boolean findOnly)
{
  VARIABLE var;
  char seq[10] = "E..T..D..";
  Biddy_Variable v;

  var.limit.task = i;
  var.limit.day = d;
  var.limit.w = 0;

  foavarname[0] = 0;
  papervarname[0] = 0;
  var.name = foavarname;
  var.papername = papervarname;

#ifdef PAPER
#ifdef LATEX
    sprintf(var.papername,"T_{%u,%u}",d,i);
#else
    sprintf(var.papername,"T_%u,%u",d,i);
#endif
#endif

  /* used for calculation, r must be 1 .. 99 */
  seq[1] = (i/10) + '0';
  seq[2] = (i%10) + '0';
  seq[4] = (d/10) + '0';
  seq[5] = (d%10) + '0';
  concatStatic(var.name,seq);

  v = 0;

  if (addTable) {
    addVariableTable(&variableTableSize,&variableTable,var);
  } else {
    v = foaVariable(var,findOnly);
  }

  return v;
}

void createVariables()
{
  int i; /* should not be unsigned */
  Biddy_String name;
  Biddy_Variable v;
  LIMIT *data;

  for (i=0; i < variableTableSize; i++)
  /* for (i=variableTableSize-1; i>=0; i--) */
  {
#ifdef PAPER
    name = variableTable[i].papername;
#else
    name = variableTable[i].name;
#endif
    /* printf("(%s)",name); */
    v = Biddy_FoaVariable(name,FALSE); /* FALSE means addElement */
    if (variableTable[i].limit.day) {
      data = (LIMIT *) malloc(sizeof(LIMIT));
      data->task = variableTable[i].limit.task;
      data->day = variableTable[i].limit.day;
      data->w = variableTable[i].limit.w;
      Biddy_SetVariableData(v,(void *) data);
    }
  }

  /*
  printf("\n");
  */
}

int cmpVarByEffort (const void * a, const void * b) {
   return ( (*(VARIABLE*)a).limit.w - (*(VARIABLE*)b).limit.w );
}

/* ************************************************************************** */
/* HEURISTICS  */
/* ************************************************************************** */

Biddy_Edge
permitEffort(const Biddy_Edge f, const unsigned int *effortlimits, const unsigned int days)
{
  Biddy_Edge e, t, r, Fv, Fneg_v;
  Biddy_Variable v;
  Biddy_Boolean OK;
  unsigned int s;
  LIMIT limit;
  unsigned int *effortlimits2;
  void *data;
  PermitEffortCache *p;
  uintptr_t cidx;

  /* this function is correct only for ZBDD and ZBDDC */

  v = Biddy_GetTopVariable(f);
  if (v == 0) return f;

  /* LOOKUP IN CACHE TABLE */
  effortCacheSearch++;
  cidx = (uintptr_t) f;
  for (s = 1; s <= days; s++) cidx = cidx + s * effortlimits[s];
  p = &permitEffortCache.table[cidx % permitEffortCache.size];
  if ((p->r) && (p->f == f))
  {
    OK = TRUE;
    for (s = 1; OK && (s <= days); s++) {
      if (p->n[s] != effortlimits[s]) OK = FALSE;
    }
    if (OK) {
      r = p->r;
      Biddy_Refresh(r);
      effortCacheHit++;
      return r;
    }
  }

  /* DETERMINING PARAMETERS FOR RECURSIVE CALLS */
  /* COMPLEMENTED EDGES MUST BE TRANSFERED */
  Fneg_v = Biddy_InvCond(Biddy_GetElse(f),Biddy_GetMark(f));
  Fv = Biddy_GetThen(f);

  data = Biddy_GetVariableData(v);
  limit = *((LIMIT *) data);

  effortlimits2 = (unsigned int *) malloc((1+days)*sizeof(unsigned int));
  effortlimits2[0] = 0;
  for (s = 1; s <= days; s++) {
    effortlimits2[s] = effortlimits[s];
  }

  e = permitEffort(Fneg_v,effortlimits,days);
  if (effortlimits2[limit.day] < limit.w) {
    t = Biddy_GetConstantZero();
  } else {
    effortlimits2[limit.day] = effortlimits2[limit.day] - limit.w;
    t = permitEffort(Fv,effortlimits2,days);
  }

  free(effortlimits2);

  r = Biddy_TaggedFoaNode(v,e,t,v,TRUE);

  /* STORE IN CACHE TABLE */
  p->f = f;
  for (s = 1; s <= days; s++) {
    p->n[s] = effortlimits[s];
  }
  p->r = r;

  return r;
}

void
permitEffortGarbage(Biddy_Manager MNG)
{
  int i;
  PermitEffortCache *p;

  for (i=0; i<=permitEffortCache.size; i++) {
    if (permitEffortCache.table[i].r) {
      p = &permitEffortCache.table[i];
      if (!Biddy_Managed_IsOK(MNG,p->f) || !Biddy_Managed_IsOK(MNG,p->r))
      {
        p->r = NULL;
      }
    }
  }
}

Biddy_Edge
applyEffortLimit(Biddy_Edge X, unsigned int e, unsigned int days)
{
  unsigned int s;
  unsigned int i;
  Biddy_Edge f;
  unsigned int *effortlimits;

  effortlimits = (unsigned int *) malloc((1+days)*sizeof(unsigned int));

  effortlimits[0] = 0;
  for (s = 1; s <= days; s++) {
    effortlimits[s] = e;
  }

  f = permitEffort(X,effortlimits,days);

  free(effortlimits);

  return f;
}

/* ************************************************************************** */
/* REPORT RESULTS */
/* ************************************************************************** */

void
reportResult(Biddy_Edge f, unsigned int numDays, unsigned int numTasks)
{
  unsigned int i,j;
  unsigned int t,total;
  Biddy_Variable v;
  LIMIT limit;
  void *data;

  total = 0;
  for (i=1; i<=numDays; i++) {
    printf("DAY %u ",i);
    t = 0;
    for (j=0; j<numTasks; j++) {
      if (Biddy_Subset1(f,v=findTvariable(j,i)) != Biddy_GetEmptySet()) {
        data = Biddy_GetVariableData(v);
        limit = *((LIMIT *) data);
        t = t + limit.w;
      }
    }
    printf("(effort = %u): ",t);

    total = total + t;

    for (j=0; j<numTasks; j++) {
      if (Biddy_Subset1(f,v=findTvariable(j,i)) != Biddy_GetEmptySet()) {
        data = Biddy_GetVariableData(v);
        limit = *((LIMIT *) data);
        printf("T%.2u<%u> ",limit.task,limit.w);
      }
    }
    printf("\n");
  }
  printf("TOTAL EFFORT = %u\n",total);
}

/* ************************************************************************** */
/* MAIN PROGRAM */
/* ************************************************************************** */

/* CHECKOPERATIONS IS FOR DEBUGGING, ONLY */
#ifdef CHECKOPERATIONS
#include "biddy-example-pp-checkoperations.c"
#endif

int main(int argc, char **argv) {
  clock_t elapsedtime;
  Biddy_String activeordering;
  unsigned int i,j,t;
  unsigned int i1,i2,j1,j2;
  Biddy_Edge T[MAXTASKS];
  Biddy_Edge f,schedule,restriction;
  Biddy_Boolean taskCompleted[MAXTASKS];

  /* SYSTEM DEFINITION */
  unsigned int numDays = 0;
  unsigned int numTasks = 0;
  unsigned int maxEffortPerDay = 0;
  unsigned int maxAhead = 0;
  unsigned int maxProlonged = 0;
  unsigned int taskEffort[MAXTASKS] = {2,2,1,2,5,5,5,5,3,3,5,2,3,3,2,1,5,1,2,1,2,1,4,3,2,2,1,3,4,3,4,2,2,4,3,2,3,1,1,2,1,1,2,3,2,2,4,2,1,3,4,1,3,1,2,2,2,4,1,2};

  setbuf(stdout,NULL);

  /* CHECK OPERATIONS ON COMBINATION SETS-  DEBUGGING ONLY */
#ifdef CHECKOPERATIONS
  Biddy_InitAnonymous(BDDTYPE);
  printf("Using %s...\n",Biddy_GetManagerName());
  checkOperations();
  Biddy_Exit();
  exit(0);
#endif

  variableTableSize = 0;
  variableTable = NULL;

  /* CHECK ARGUMENTS */
  i = 1;
  while (i < argc) {

    if (!strcmp(argv[i],"-h") || !strcmp(argv[i],"-help")) {
      printf("[ -task  | -t ]\n");
      printf("[ -effort | -e ]\n");
      printf("[ -ahead | -a ]\n");
      printf("[ -prolonged | -p ]\n");
      exit(0);
    }

    if (!strncmp(argv[i],"-t",2) || !strcmp(argv[i],"-task")) {
      sscanf(&(argv[i][2]),"%u",&numTasks);
      if (!numTasks && strncmp(argv[i],"-t0",3)) {
        sscanf(argv[++i],"%u",&numTasks);
      }
    }

    if (!strncmp(argv[i],"-e",2) || !strcmp(argv[i],"-effort")) {
      sscanf(&(argv[i][2]),"%u",&maxEffortPerDay);
      if (!maxEffortPerDay && strncmp(argv[i],"-e0",3)) {
        sscanf(argv[++i],"%u",&maxEffortPerDay);
      }
    }

    if (!strncmp(argv[i],"-a",2) || !strcmp(argv[i],"-ahead")) {
      sscanf(&(argv[i][2]),"%u",&maxAhead);
      if (!maxAhead && strncmp(argv[i],"-a0",3)) {
        sscanf(argv[++i],"%u",&maxAhead);
      }
    }

    if (!strncmp(argv[i],"-p",2) || !strcmp(argv[i],"-prolonged")) {
      sscanf(&(argv[i][2]),"%u",&maxProlonged);
      if (!maxProlonged && strncmp(argv[i],"-p0",3)) {
        sscanf(argv[++i],"%u",&maxProlonged);
      }
    }

    i++;
  }

  /* INIT VARIABLES */

  printf("\nINITIALIZATION\n\n");

  if (numTasks == 0) {
    printf("Error: numTasks = %u\n",numTasks);
    exit(1);
  }

  if (maxEffortPerDay == 0) {
    printf("Error: maxEffortPerDay = %u\n",maxEffortPerDay);
    exit(1);
  }

  if (numTasks > MAXTASKS) {
    printf("Error: numTasks = %u, MAXTASKS = %u\n",numTasks,MAXTASKS);
    exit(1);
  }

  for (i=0; i<MAXTASKS; i++) {
    taskCompleted[i] = FALSE;
  }

  t = 0;
  for (i=0; i<numTasks; i++) {
    t = t + taskEffort[i];
  }
  numDays = t / maxEffortPerDay;
  if (t > numDays * maxEffortPerDay) numDays++;
  numDays = numDays + maxProlonged;

  if (numDays > MAXDAYS) {
    printf("Error: numDays = %u, MAXDAYS = %u\n",numDays,MAXDAYS);
    exit(1);
  }

  if (maxProlonged) {
    printf("numDays = %u + %u\n",numDays-maxProlonged,maxProlonged);
  } else {
    printf("numDays = %u\n",numDays);
  }
  printf("numTasks = %u\n",numTasks);
  printf("totalEffort = %u\n",t);
  printf("maxEffortPerDay = %u\n",maxEffortPerDay);
  printf("maxAhead = %u\n",maxAhead);
  printf("\n");
  for (i=0; i<numTasks; i++) {
    printf("<T%u:%u>",i,taskEffort[i]);
    for (j=1; j<=numDays; j++) {
      ADDVARIABLE(i,j);
    }
  }
  printf("\n");

  /* set maxEffortPerDay for all created variables */
  for (i=0; i<variableTableSize; i++) {
    variableTable[i].limit.w = taskEffort[variableTable[i].limit.task];
  }

  /* reorder variables by effort */
  /**/
  qsort(variableTable,variableTableSize,sizeof(VARIABLE),cmpVarByEffort);
  /**/

  /* INIT BDD SYSTEM */

  Biddy_InitAnonymous(BDDTYPE);

  printf("\nSTARTING CALCULATION USING %s\n",Biddy_GetManagerName());

  elapsedtime = clock();

  /* permitEffort cache */
  permitEffortCache.size = permitEffortCacheSize;
  permitEffortCache.table = (PermitEffortCache *) malloc((permitEffortCache.size+1) * sizeof(PermitEffortCache));
  for (i=0; i<=permitEffortCache.size; i++) {
    permitEffortCache.table[i].r = NULL;
  }
  Biddy_AddCache((Biddy_GCFunction) permitEffortGarbage);

  /* CREATE ALL VARIABLES BEFORE THE REAL CALCULATION START */
  /* THIS IS REQUIRED FOR OBDDs AND TZBDDs, IT IS OPTIONAL FOR ZBDDs */
  /* (MANIPULATING SETS WITH OBDDs AND TZBDDs IS NOT FULLY SUPPORTED IN BIDDY, YET) */
  /* THIS IS REQUIRED IF CUSTOM VARIABLE ORDERING IS NEEDED */

  createVariables();

  /**/
  reportOrdering();
  /**/

  restriction = Biddy_GetEmptySet();
  printf("\nCREATE RESTRICTION FUNCTION\n");
  for (i1=0; i1<numTasks; i1++) {
    for (i2=0; i2<numTasks; i2++) {
      for (j1=1; j1<=numDays; j1++) {
        for (j2=1; j2<=numDays; j2++) {
          if ((i1 < i2) && (j1 > (j2 + maxAhead)) && (taskEffort[i1] == taskEffort[i2])) {
            f = Biddy_GetBaseSet();
            f = Biddy_Change(f,findTvariable(i1,j1));
            f = Biddy_Change(f,findTvariable(i2,j2));
            restriction = Biddy_Union(restriction,f);
          }
        }
      }
    }
  }
  Biddy_KeepFormulaUntilPurge(restriction);

  printf("clock() TIME = %.2f\n",(clock()-elapsedtime)/(1.0*CLOCKS_PER_SEC));
  printf("restriction function has %u nodes (including terminals) and %.0f minterms\n",Biddy_CountNodes(restriction),Biddy_CountMinterms(restriction,0));

  printf("\nSCHEDULING\n");

  for (i=0; i<numTasks; i++) {
    T[i] = Biddy_GetEmptySet();
    for (j=1; j<=numDays; j++) {
      T[i] = Biddy_Union(T[i],Biddy_GetElementEdge(findTvariable(i,j)));
    }
    Biddy_KeepFormulaUntilPurge(T[i]);
  }

  schedule = Biddy_GetBaseSet();
  for (i=1; i<=numTasks; i++) {
    /* find task with max effort among those which are not scheduled, yet */
    t = 0;
    while (taskCompleted[t]) t++;
    for (j=t; j<numTasks; j++) {
      if (!taskCompleted[j] && (taskEffort[j] > taskEffort[t])) {
        t = j;
      }
    }
    taskCompleted[t] = TRUE;
    printf("STEP %u, scheduling task %u (effort = %u)...\n",i,t,taskEffort[t]);
    schedule = Biddy_Product(schedule,T[t]);
    Biddy_KeepFormula(schedule);
    Biddy_Clean();

    /* apply restriction after every step */
    /**/
    printf("(%.2f s) Before restriction, ",(clock()-elapsedtime)/(1.0*CLOCKS_PER_SEC));
    STATCOMBINATIONS(schedule);
    f = Biddy_Supset(schedule,restriction);
    schedule = Biddy_Diff(schedule,f);
    Biddy_KeepFormula(schedule);
    Biddy_Clean();
    printf("(%.2f s)  After restriction, ",(clock()-elapsedtime)/(1.0*CLOCKS_PER_SEC));
    STATCOMBINATIONS(schedule);
    /**/

    /* apply limit after every step */
    /**/
    printf("(%.2f s) Before limit, ",(clock()-elapsedtime)/(1.0*CLOCKS_PER_SEC));
    STATCOMBINATIONS(schedule);
    schedule = applyEffortLimit(schedule,maxEffortPerDay,numDays);
    Biddy_KeepFormula(schedule);
    Biddy_Clean();
    printf("(%.2f s)  After limit, ",(clock()-elapsedtime)/(1.0*CLOCKS_PER_SEC));
    STATCOMBINATIONS(schedule);
    /**/

    printf("\n");
  }

  /* apply restriction after all steps */
  /*
  printf("(%.2f s) Before restriction, ",(clock()-elapsedtime)/(1.0*CLOCKS_PER_SEC));
  STATCOMBINATIONS(schedule);
  f = Biddy_Supset(schedule,restriction);
  schedule = Biddy_Diff(schedule,f);
  printf("(%.2f s)  After restriction, ",(clock()-elapsedtime)/(1.0*CLOCKS_PER_SEC));
  STATCOMBINATIONS(schedule);
  */

  /* apply limit after all steps */
  /*
  printf("\nAPPLY LIMIT\n");
  printf("(before limit) ");
  STATCOMBINATIONS(schedule);
  schedule = applyEffortLimit(schedule,maxEffortPerDay);
  printf("( after limit) ");
  STATCOMBINATIONS(schedule);
  */

  printf("\n");
  printf("clock() TIME = %.2f\n",(clock()-elapsedtime)/(1.0*CLOCKS_PER_SEC));
  printf("Cache hits = %llu/%llu\n",effortCacheHit,effortCacheSearch);
  if (schedule == Biddy_GetEmptySet()) {
    printf("NO SOLUTIONS\n");
  } else {
    printf("%s FOR FINAL SCHEDULE HAS %u nodes (including terminals)\n",Biddy_GetManagerName(),Biddy_CountNodes(schedule));
    printf("REPORTING ONE SOLUTION OUT OF %.0f POSSIBLE\n",Biddy_CountMinterms(schedule,0));
    f = Biddy_ExtractMinterm(NULL,schedule);
    reportResult(f,numDays,numTasks);
  }

  /**/
  Biddy_PrintInfo(NULL);
  /**/

  Biddy_Exit();
}

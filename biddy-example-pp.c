/* $Revision: 558 $ */
/* $Date: 2019-10-14 09:42:56 +0200 (pon, 14 okt 2019) $ */
/* This file (biddy-example-pp.c) is a C file */
/* Author: Robert Meolic (robert@meolic.com) */
/* This file has been released into the public domain by the author. */

/* This example is compatible with Biddy v1.8 */

/* COMPILE WITH (ADD -lgmp IF USING STATIC BIDDY LIBRARY): */
/* gcc -DUNIX -O2 -o biddy-example-pp biddy-example-pp.c -I. -L./bin -lbiddy */

/* this example is about the production planning problem */

#include "biddy.h"
#include "string.h"
#include <time.h>

/* choose primary BDD type */
#define BDDTYPE BIDDYTYPEZBDD

/* MAX NUMBER OF PARTS/JOBS - used for PermitMakespanCache, only */
/* MAX NUMBER OF MACHINES - used for PermitMakespanCache, only */
#define MAXPARTS 16
#define MAXMACHINES 16

/* define PAPER to have variable names usable for the paper */
/* define LATEX to have variable names compatible with LaTeX (otherwise MS Word Equation style is used) */
#define NOPAPER
#define NOLATEX

/* define CHECKOPERATIONS for debugging single operations */
#define NOCHECKOPERATIONS

/* define NOHARDCODEDDATA to avoid loading hardcoded data from biddy-example-pp-data.c */
#define HARDCODEDDATA

/* define INTERNALREPORTS for internal reports - they can be huge! */
#define NOINTERNALREPORTS

/* define ONEPASS for calculation which creates the solution in one pass - less efficient */
#define NOTONEPASS

/* choose VARIANTA (from the paper), VARIANTB (the most efficient, used for our paper), or VARIANTC (mixed) */
#define VARIANTB

/* define LIMITSINGLE if you want to limit calculation to a single process */
/* define LIMITSINGLE and LIMITDOUBLE if you want to limit calculation to two processes */
/* example: -DLIMITSINGLE=3 -DLIMITDOUBLE=6 */

enum PPTYPE {NONE, DEFAULT, PERMUTATION, ALTERNATIVE};

const unsigned int PPCOLOR[MAXPARTS+1] = {0,0,60,140,180,240,300,25,85,145,205,265,325,40,160,280,100};

typedef struct OPERATION {
  unsigned int idx;
  Biddy_String name;
} OPERATION;

typedef struct SEQUENCEITEM {
  enum PPTYPE type;
  void *ITEM;
} SEQUENCEITEM;

typedef struct SEQUENCE {
  unsigned int n; /* number of operations in the sequence */
  SEQUENCEITEM *O; /* table of items */
} SEQUENCE;

typedef struct PART {
  unsigned int idx;
  unsigned int part;
  Biddy_String name;
  unsigned int color; /* hue component used to show on Gantt chart */
  unsigned int n; /* number of sequences */
  SEQUENCE *X; /* table of sequences */
  Biddy_Edge code; /* BDD representing the set of all cubes for this part */
  Biddy_Edge feasible; /* BDD representing all the feasible process sequences for this part */
} PART;

typedef struct MACHINE {
  unsigned int idx;
  Biddy_String name;
  unsigned int n; /* number of instances */
} MACHINE;

typedef struct LIMIT {
  unsigned int part;
  unsigned int machine;
  unsigned int w;
} LIMIT;

typedef struct VARIABLE {
  Biddy_String name;
  Biddy_String papername;
  LIMIT limit;
} VARIABLE;

typedef struct GANTTITEM {
  unsigned int part;
  unsigned int r;
  unsigned int t;
} GANTTITEM;

typedef struct GANTT {
  unsigned int n; /* number of items */
  Biddy_String *content;
  unsigned int *id;
  Biddy_String *group;
  unsigned int *start;
  unsigned int *end;
  Biddy_String *color;
} GANTT;

typedef SEQUENCEITEM *PSEQUENCEITEM;
typedef PART *PPART;

/* SYSTEM IS STORED IN THE GLOBAL VARIABLES */
/* THOROUGHLY disables DYNAMICLIMIT */

unsigned int DATASET = 1;
Biddy_String CSVNAME = NULL;
Biddy_String DATANAME = NULL;
unsigned int MAXSEQUENCELENGTH = 0;
unsigned int NUMPARTS = 0;
unsigned int NUMOPERATIONS = 0;
unsigned int NUMMACHINES = 0;
unsigned int FACTORYCAPACITY = 0;
Biddy_Boolean FIRSTSOLUTIONONLY = FALSE;
Biddy_Boolean FEASIBLEONLY = FALSE;
Biddy_Boolean MAXFACTORYONLY = FALSE;
Biddy_Boolean FULLSTATESPACE = FALSE;
unsigned int MAKESPANLIMIT = 0;
unsigned int TIMEBOUND = 0;

Biddy_Boolean THOROUGHLY = FALSE;
Biddy_Boolean VERBOSE = FALSE;

Biddy_Boolean AUTOBOUND = FALSE;
unsigned int BOUNDINTERVAL = 0;
Biddy_Boolean DYNAMICLIMIT = FALSE;
unsigned int LIMITINTERVAL = 0;

unsigned int operationTableSize;
OPERATION *operationTable;

unsigned int machineTableSize;
MACHINE *machineTable;

unsigned int *operationMatrix;

unsigned int productionTableSize;
PPART *productionTable;

unsigned int partTableSize;
PART *partTable;

unsigned int variableTableSize;
VARIABLE *variableTable;

char foavarname[128]; /* used to create variable name in find-or-add functions */
char papervarname[128]; /* used to create variable name using paper style */

Biddy_Edge *RESULT = NULL;
unsigned int RESULTSIZE = 0;
unsigned int MINRESULT = 0;

typedef struct {
  unsigned int n[MAXPARTS];
  unsigned int m[MAXMACHINES];
  Biddy_Edge f;
  Biddy_Edge r; /* NULL = not valid node! */
} PermitMakespanCache;

typedef struct {
  PermitMakespanCache *table;
  unsigned int size;
} PermitMakespanCacheTable;

const unsigned int permitMakespanCacheSize = 1048573;
PermitMakespanCacheTable permitMakespanCache = {NULL,0};

typedef struct {
  unsigned int n;
  Biddy_Edge f;
  Biddy_Edge r; /* NULL = not valid node! */
} PermitMachineTimeCache;

typedef struct {
  PermitMachineTimeCache *table;
  unsigned int size;
} PermitMachineTimeCacheTable;

const unsigned int permitMachineTimeCacheSize = 1048573;
PermitMachineTimeCacheTable permitMachineTimeCache = {NULL,0};

Biddy_Edge trace;

/* USEFUL MACROS TO ENABLE NICE LATEX REPORT */
#ifdef LATEX
#define PRINTCOMBINATIONS(X) \
{\
if (!trace) {\
printf("\\begin{small}\\begin{verbatim}");\
printf("%s has %u nodes (including terminals) and %.0f cubes",Biddy_GetManagerName(),Biddy_CountNodes(X),Biddy_CountMinterms(X,0));\
printf("\\end{verbatim}\\end{small}");\
printf("\\vspace*{-1em}\\begin{align*} ");\
if (Biddy_CountMinterms(X,0) > 99) printf("to large...\n"); else Biddy_PrintfMinterms(X,FALSE);\
printf("\\end{align*}");\
}}
#else
#define PRINTCOMBINATIONS(X) \
{\
if (!trace) {\
printf("%s has %u nodes (including terminals) and %.0f cubes\n",Biddy_GetManagerName(),Biddy_CountNodes(X),Biddy_CountMinterms(X,0));\
if (Biddy_CountMinterms(X,0) > 99) printf("to large...\n"); else Biddy_PrintfMinterms(X,FALSE);\
}}
#endif

#ifdef LATEX
#define STATCOMBINATIONS(X) \
{\
if (!trace) {\
printf("clock() TIME = %.2f\n",(clock()-elapsedtime)/(1.0*CLOCKS_PER_SEC));\
printf("%s has %u nodes (including terminals) and %.0f cubes\n",Biddy_GetManagerName(),Biddy_CountNodes(X),Biddy_CountMinterms(X,0));\
}}
#else
#define STATCOMBINATIONS(X) \
{\
if (!trace) {\
printf("clock() TIME = %.2f\n",(clock()-elapsedtime)/(1.0*CLOCKS_PER_SEC));\
printf("%s has %u nodes (including terminals) and %.0f cubes\n",Biddy_GetManagerName(),Biddy_CountNodes(X),Biddy_CountMinterms(X,0));\
}}
#endif

/* USEFUL MACROS TO ENABLE SHORT DESCRIPTIONS OF DATA */

#define foaOvariable(s,w,r) findoraddOvariable(s,w,r,FALSE,FALSE)
#define findOvariable(s,w,r) findoraddOvariable(s,w,r,FALSE,TRUE)

#define foaMvariable(s,w,r,i) findoraddMvariable(s,w,r,i,FALSE,FALSE)
#define findMvariable(s,w,r,i) findoraddMvariable(s,w,r,i,FALSE,TRUE)

#define foaMXvariable(i,x) findoraddMXvariable(i,x,FALSE,FALSE)
#define findMXvariable(i,x) findoraddMXvariable(i,x,FALSE,TRUE)

/* THESE ARE USED FOR SCHEDULING */
/* variable W[part,r+1] is used to denote final state - it must not be removed! */

#define foaWvariable(s,r) findoraddWvariable(s,r,FALSE,FALSE)
#define findWvariable(s,r) findoraddWvariable(s,r,FALSE,TRUE)

#define foaRvariable(s,r) findoraddRvariable(s,r,FALSE,FALSE)
#define findRvariable(s,r) findoraddRvariable(s,r,FALSE,TRUE)

#define foaBvariable(i,t) findoraddBvariable(i,t,FALSE,FALSE)
#define findBvariable(i,t) findoraddBvariable(i,t,FALSE,TRUE)

#define foaTvariable(s,t) findoraddTvariable(s,t,FALSE,FALSE)
#define findTvariable(s,t) findoraddTvariable(s,t,FALSE,TRUE)

#define foaSvariable(s,r,i,j) findoraddSvariable(s,r,i,j,FALSE,FALSE)
#define findSvariable(s,r,i,j) findoraddSvariable(s,r,i,j,FALSE,TRUE)

#define foaFSvariable(i) findoraddFSvariable(i,FALSE,FALSE)
#define findFSvariable(i) findoraddFSvariable(i,FALSE,TRUE)

/* THESE ARE USED FOR GANTT CHART */

#define foaGvariable(s,r,t) findoraddGvariable(s,r,t,FALSE,FALSE)
#define findGvariable(s,r,t) findoraddGvariable(s,r,t,FALSE,TRUE)

/* VARIABLES */

#ifdef LIMITSINGLE
#ifdef LIMITDOUBLE
#define ADDVARIABLE(w,r)\
{unsigned int i,j;\
if((part==LIMITSINGLE)||(part==LIMITDOUBLE)){\
if(w>NUMOPERATIONS)NUMOPERATIONS=w;\
findoraddOvariable(part,w,r,TRUE,FALSE);\
findoraddWvariable(part,r,TRUE,FALSE);\
findoraddWvariable(part,r+1,TRUE,FALSE);\
findoraddRvariable(part,r,TRUE,FALSE);\
for(i=1;i<=NUMMACHINES;i++)\
if(operationMatrix[w+i*(1+operationTableSize)]){\
  findoraddMvariable(part,w,r,i,TRUE,FALSE);\
  for(j=0;j<=operationMatrix[w+i*(1+operationTableSize)];j++) findoraddTvariable(part,j,TRUE,FALSE);\
  for(j=1;j<=machineTable[i].n;j++){\
    findoraddMXvariable(i,j,TRUE,FALSE);\
    findoraddBvariable(i,j,TRUE,FALSE);\
    findoraddSvariable(part,r,i,j,TRUE,FALSE);\
}}}}
#else
#define ADDVARIABLE(w,r)\
{unsigned int i,j;\
if(part==LIMITSINGLE){\
if(w>NUMOPERATIONS)NUMOPERATIONS=w;\
findoraddOvariable(part,w,r,TRUE,FALSE);\
findoraddWvariable(part,r,TRUE,FALSE);\
findoraddWvariable(part,r+1,TRUE,FALSE);\
findoraddRvariable(part,r,TRUE,FALSE);\
for(i=1;i<=NUMMACHINES;i++)\
if(operationMatrix[w+i*(1+operationTableSize)]){\
  findoraddMvariable(part,w,r,i,TRUE,FALSE);\
  for(j=0;j<=operationMatrix[w+i*(1+operationTableSize)];j++) findoraddTvariable(part,j,TRUE,FALSE);\
  for(j=1;j<=machineTable[i].n;j++){\
    findoraddMXvariable(i,j,TRUE,FALSE);\
    findoraddBvariable(i,j,TRUE,FALSE);\
    findoraddSvariable(part,r,i,j,TRUE,FALSE);\
}}}}
#endif
#else
#define ADDVARIABLE(w,r) \
{unsigned int i,j; \
if(part<=NUMPARTS){\
if(w>NUMOPERATIONS)NUMOPERATIONS=w;\
findoraddOvariable(part,w,r,TRUE,FALSE);\
findoraddWvariable(part,r,TRUE,FALSE);\
findoraddWvariable(part,r+1,TRUE,FALSE);\
findoraddRvariable(part,r,TRUE,FALSE);\
for(i=1;i<=NUMMACHINES;i++)\
if(operationMatrix[w+i*(1+operationTableSize)]){\
  findoraddMvariable(part,w,r,i,TRUE,FALSE);\
  for(j=0;j<=operationMatrix[w+i*(1+operationTableSize)];j++) findoraddTvariable(part,j,TRUE,FALSE);\
  for(j=1;j<=machineTable[i].n;j++){\
    findoraddMXvariable(i,j,TRUE,FALSE);\
    findoraddBvariable(i,j,TRUE,FALSE);\
    findoraddSvariable(part,r,i,j,TRUE,FALSE);\
}}}}
#endif

void
concat(Biddy_String *s1, const Biddy_String s2)
{
   *s1 = (Biddy_String) realloc(*s1,strlen(*s1)+strlen(s2)+1);
   strcat(*s1,s2);
}

void
concatNumber(Biddy_String *s1, const int n)
{
   char s2[32];
   sprintf(s2,"%d",n);
   *s1 = (Biddy_String) realloc(*s1,strlen(*s1)+strlen(s2)+1);
   strcat(*s1,s2);
}

void
concatStatic(Biddy_String s1, const Biddy_String s2)
{
   strcat(s1,s2);
}

void
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

void reportSystem()
{
  unsigned int i;
  /* unsigned int j,k; */
  Biddy_Edge f;

  /*
  printf("\nOPERATION TABLE\n");
  for (i = 1; i <= NUMOPERATIONS; i++) {
    printf("%u. %s\n",i,operationTable[i].name);
  }
  */

  /*
  printf("PART TABLE\n");
  for (i = 1; i <= partTableSize; i++) {
    printf("%u. %s\n",i,partTable[i].name);
    for (j = 1; j <= partTable[i].n; j++) {
      printf("SEQUENCE %u:",j);
      for (k = 1; k <= partTable[i].X[j].n; k++) {
        switch (partTable[i].X[j].O[k].type) {
          case DEFAULT:
            printf(" %s",((OPERATION *)(partTable[i].X[j].O[k].ITEM))->name);
            break;
          case PERMUTATION:
            printf(" [%s]",((OPERATION *)(partTable[i].X[j].O[k].ITEM))->name);
            break;
          case ALTERNATIVE:
            printf(" %s",((PART *)(partTable[i].X[j].O[k].ITEM))->name);
            break;
        }
      }
      printf("\n");
    }
  }
  */

  /*
  printf("\nMACHINE TABLE\n");
  for (i = 1; i <= NUMMACHINES; i++) {
    printf("%u. %s (%u)\n",i,machineTable[i].name,machineTable[i].n);
  }
  */

  /*
  printf("\nOPERATION MATRIX\n");
  for (i = 1; i <= NUMOPERATIONS; i++) {
    for (j = 1; j <= NUMMACHINES; j++) {
      if (operationMatrix[i + j * (1 + operationTableSize)]) {
        printf("O%u @ m%u = %u\n",i,j,operationMatrix[i + j * (1 + operationTableSize)]);
      }
    }
  }
  */

  printf("THERE ARE %u nodes\n",Biddy_NodeTableNum());
  for (i = 1; i < Biddy_FormulaTableNum(); i++) {
    f = Biddy_GetIthFormula(i);
    if (f != NULL) {
      printf("CHECKING FORMULA: %s has %u nodes\n",Biddy_GetIthFormulaName(i),Biddy_CountNodes(f));
    }
  }

}

void
addVariableTable(unsigned int s, unsigned int *variableTableSize,
                 VARIABLE **variableTable, VARIABLE var)
{
  unsigned int i;
  Biddy_Boolean FIND;
  int cc;

  if (s > NUMPARTS) return;

#ifdef LIMITSINGLE
#ifdef LIMITDOUBLE
    if (s && (s != LIMITSINGLE) && (s != LIMITDOUBLE)) return; /* limit to two processes */
#else
    if (s && s != LIMITSINGLE) return; /* limit to a single process */
#endif
#endif

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
      if (cc > 0) break;
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

  /* DEBUGGING */
  /*
  printf("addVariableTable: VARIABLE TABLE, SIZE = %u, var %s ",*variableTableSize,var.name);
  if (var.limit.part) printf("(limit.part = %u, limit.machine = %u, limit.w = %u) ",var.limit.part,var.limit.machine,var.limit.w);
  if (!FIND) printf("ADDED\n"); else printf("FOUND\n");
  */
  /*
  for (i=0; i < *variableTableSize; i++) {
    printf("%u. %s\n",i,(*variableTable)[i].name);
  }
  */
}

Biddy_Variable
foaVariable(unsigned int s, VARIABLE var, Biddy_Boolean findOnly)
{
  Biddy_String name;
  Biddy_Variable v;
  LIMIT *data;

  if (s > NUMPARTS) return 0;

#ifdef LIMITSINGLE
#ifdef LIMITDOUBLE
    if (s && (s != LIMITSINGLE) && (s != LIMITDOUBLE)) return 0; /* limit to two processes */
#else
    if (s && s != LIMITSINGLE) return 0; /* limit to a single process */
#endif
#endif

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

      /*
      if ((Biddy_GetManagerType() == BIDDYTYPETZBDD) || (Biddy_GetManagerType() == BIDDYTYPETZBDDC)) {
        printf("WARNING (foaVariable): variable %s added!\n",name);
      }
      */

      v = Biddy_FoaVariable(name,FALSE); /* FALSE means addElement */
      if (var.limit.part) {
        data = (LIMIT *) malloc(sizeof(LIMIT));
        data->part = var.limit.part;
        data->machine = var.limit.machine;
        data->w = var.limit.w;
        Biddy_SetVariableData(v,(void *) data);
      }
    }
  }

  return v;
}

Biddy_Variable
findoraddOvariable(unsigned int s, unsigned int w, unsigned int r,
                   Biddy_Boolean addTable, Biddy_Boolean findOnly)
{
  VARIABLE var;
  char seq[3] = "R.";
  Biddy_Variable v;

  if (s > NUMPARTS) return 0;
  if (w > NUMOPERATIONS) return 0;

  var.limit.part = 0;
  var.limit.machine = 0;
  var.limit.w = 0;

  foavarname[0] = 0;
  papervarname[0] = 0;
  var.name = foavarname;
  var.papername = papervarname;

#ifdef PAPER
#ifdef LATEX
    sprintf(var.papername,"O^{%u}_{%u,%u}",s,w,r);
#else
    sprintf(var.papername,"O^%u_%u,%u",s,w,r);
#endif
#endif

  /* used for calculation, r must be 1 .. 9 */
  seq[1] = r + '0';
  concatStatic(var.name,(*(productionTable[s])).name);
  concatStatic(var.name,seq);
  concatStatic(var.name,operationTable[w].name);

  v = 0;

  if (addTable) {
    addVariableTable(s,&variableTableSize,&variableTable,var);
  } else {
    v = foaVariable(s,var,findOnly);
  }

  return v;
}

Biddy_Variable
findoraddMvariable(unsigned int s, unsigned int w, unsigned int r, unsigned int i,
                   Biddy_Boolean addTable, Biddy_Boolean findOnly)
{
  VARIABLE var;
  char seq[3] = "R.";
  Biddy_Variable v;

  if (s > NUMPARTS) return 0;
  if (w > NUMOPERATIONS) return 0;
  if (i > NUMMACHINES) return 0;

  var.limit.part = s;
  var.limit.machine = i;
  var.limit.w = operationMatrix[w+i*(1+operationTableSize)];

  foavarname[0] = 0;
  papervarname[0] = 0;
  var.name = foavarname;
  var.papername = papervarname;

#ifdef PAPER
#ifdef LATEX
  sprintf(var.papername,"M^{%u,%u,%u}_{%u}",s,w,r,i);
#else
  sprintf(var.papername,"M^%u,%u,%u_%u",s,w,r,i);
#endif
#endif

  /* used for calculation, r must be 1 .. 9 */
  seq[1] = r + '0';
  concatStatic(var.name,(*(productionTable[s])).name);
  concatStatic(var.name,seq);
  concatStatic(var.name,operationTable[w].name);
  concatStatic(var.name,machineTable[i].name);

  v = 0;

  if (addTable) {
    addVariableTable(s,&variableTableSize,&variableTable,var);
  } else {
    v = foaVariable(s,var,findOnly);
  }

  return v;
}

Biddy_Variable
findoraddMXvariable(unsigned int i, unsigned int j,
                    Biddy_Boolean addTable, Biddy_Boolean findOnly)
{
  VARIABLE var;
  char seq[3] = "J.";
  Biddy_Variable v;

  if (i > NUMMACHINES) return 0;

  var.limit.part = NUMPARTS+1;
  var.limit.machine = i;
  var.limit.w = 0;

  foavarname[0] = 0;
  papervarname[0] = 0;
  var.name = foavarname;
  var.papername = papervarname;

#ifdef PAPER
#ifdef LATEX
  sprintf(var.papername,"M_{%u,%u}",i,j);
#else
  sprintf(var.papername,"M_%u,%u",i,j);
#endif
#endif

  /* used for calculation, j must be 1 .. 9 */
  concatStatic(var.name,"X");
  seq[1] = j + '0';
  concatStatic(var.name,seq);
  concatStatic(var.name,machineTable[i].name);

  v = 0;

  if (addTable) {
    addVariableTable(0,&variableTableSize,&variableTable,var);
  } else {
    v = foaVariable(0,var,findOnly);
  }

  return v;
}

Biddy_Variable
findoraddBvariable(unsigned int i, unsigned int j,
                   Biddy_Boolean addTable, Biddy_Boolean findOnly)
{
  VARIABLE var;
  char seq[3] = "J.";
  Biddy_Variable v;

  if (i > NUMMACHINES) return 0;

  var.limit.part = 0;
  var.limit.machine = 0;
  var.limit.w = 0;

  foavarname[0] = 0;
  papervarname[0] = 0;
  var.name = foavarname;
  var.papername = papervarname;

#ifdef PAPER
#ifdef LATEX
  sprintf(var.papername,"B_{%u,%u}",i,j);
#else
  sprintf(var.papername,"B_%u,%u",i,j);
#endif
#endif

  /* used for calculation, j must be 1 .. 9 */
  seq[1] = j + '0';
  concatStatic(var.name,"B");
  concatStatic(var.name,machineTable[i].name);
  concatStatic(var.name,seq);

  v = 0;

  if (addTable) {
    addVariableTable(0,&variableTableSize,&variableTable,var);
  } else {
    v = foaVariable(0,var,findOnly);
  }

  return v;
}

Biddy_Variable
findoraddWvariable(unsigned int s, unsigned int r,
                   Biddy_Boolean addTable, Biddy_Boolean findOnly)
{
  VARIABLE var;
  char seq[3] = "R.";
  Biddy_Variable v;

  if (s > NUMPARTS) return 0;

  var.limit.part = 0;
  var.limit.machine = 0;
  var.limit.w = 0;

  foavarname[0] = 0;
  papervarname[0] = 0;
  var.name = foavarname;
  var.papername = papervarname;

#ifdef PAPER
#ifdef LATEX
  sprintf(var.papername,"W^{%u,%u}",s,r);
#else
  sprintf(var.papername,"W^%u,%u",s,r);
#endif
#endif

  /* used for calculation, r must be 1 .. 9 */
  seq[1] = r + '0';
  concatStatic(var.name,(*(productionTable[s])).name);
  concatStatic(var.name,seq);

  v = 0;

  if (addTable) {
    addVariableTable(s,&variableTableSize,&variableTable,var);
  } else {
    v = foaVariable(s,var,findOnly);
  }

  return v;
}

Biddy_Variable
findoraddRvariable(unsigned int s, unsigned int r,
                   Biddy_Boolean addTable, Biddy_Boolean findOnly)
{
  VARIABLE var;
  char seq[4] = "R.B";
  Biddy_Variable v;

  if (s > NUMPARTS) return 0;

  var.limit.part = 0;
  var.limit.machine = 0;
  var.limit.w = 0;

  foavarname[0] = 0;
  papervarname[0] = 0;
  var.name = foavarname;
  var.papername = papervarname;

#ifdef PAPER
#ifdef LATEX
  sprintf(var.papername,"R^{%u,%u}",s,r);
#else
  sprintf(var.papername,"R^%u,%u",s,r);
#endif
#endif

  /* used for calculation, r must be 1 .. 9 */
  seq[1] = r + '0';
  concatStatic(var.name,(*(productionTable[s])).name);
  concatStatic(var.name,seq);

  v = 0;

  if (addTable) {
    addVariableTable(s,&variableTableSize,&variableTable,var);
  } else {
    v = foaVariable(s,var,findOnly);
  }

  return v;
}

Biddy_Variable
findoraddTvariable(unsigned int s, unsigned int t,
                   Biddy_Boolean addTable, Biddy_Boolean findOnly)
{
  VARIABLE var;
  char seq[4] = "C..";
  Biddy_Variable v;

  if (s > NUMPARTS) return 0;

  var.limit.part = 0;
  var.limit.machine = 0;
  var.limit.w = 0;

  foavarname[0] = 0;
  papervarname[0] = 0;
  var.name = foavarname;
  var.papername = papervarname;

#ifdef PAPER
#ifdef LATEX
  sprintf(var.papername,"T^{%u}_{%u}",s,t);
#else
  sprintf(var.papername,"T^%u_%u",s,t);
#endif
#endif

  /* used for calculation, t must be 1 .. 99 */
  seq[1] = (t/10) + '0';
  seq[2] = (t%10) + '0';
  concatStatic(var.name,(*(productionTable[s])).name);
  concatStatic(var.name,seq);

  v = 0;

  if (addTable) {
    addVariableTable(s,&variableTableSize,&variableTable,var);
  } else {
    v = foaVariable(s,var,findOnly);
  }

  return v;
}

Biddy_Variable
findoraddSvariable(unsigned int s, unsigned int r, unsigned int i, unsigned int j,
                   Biddy_Boolean addTable, Biddy_Boolean findOnly)
{
  VARIABLE var;
  char seq1[3] = "R.";
  char seq2[3] = "J.";
  Biddy_Variable v;

  if (s > NUMPARTS) return 0;
  if (i > NUMMACHINES) return 0;

  var.limit.part = 0;
  var.limit.machine = 0;
  var.limit.w = 0;

  foavarname[0] = 0;
  papervarname[0] = 0;
  var.name = foavarname;
  var.papername = papervarname;

#ifdef PAPER
#ifdef LATEX
  sprintf(var.papername,"S^{%u,%u}_{%u,%u}",s,r,i,j);
#else
  sprintf(var.papername,"S^%u,%u_%u,%u",s,r,i,j);
#endif
#endif

  /* used for calculation, r and j must be 1 .. 9 */
  seq1[1] = r + '0';
  seq2[1] = j + '0';
  concatStatic(var.name,(*(productionTable[s])).name);
  concatStatic(var.name,seq1);
  concatStatic(var.name,seq2);
  concatStatic(var.name,machineTable[i].name);

  v = 0;

  if (addTable) {
    addVariableTable(s,&variableTableSize,&variableTable,var);
  } else {
    v = foaVariable(s,var,findOnly);
  }

  return v;
}

Biddy_Variable
findoraddFSvariable(unsigned int i,
                    Biddy_Boolean addTable, Biddy_Boolean findOnly)
{
  VARIABLE var;
  char seq[5] = "YS..";
  Biddy_Variable v;

  var.limit.part = 0;
  var.limit.machine = 0;
  var.limit.w = 0;

  foavarname[0] = 0;
  papervarname[0] = 0;
  var.name = foavarname;
  var.papername = papervarname;

#ifdef PAPER
#ifdef LATEX
  sprintf(var.papername,"FS_{%u}",i);
#else
  sprintf(var.papername,"FS_%u",i);
#endif
#endif

  /* used for calculation, i must be 1 .. 99 */
  seq[2] = (i/10) + '0';
  seq[3] = (i%10) + '0';
  concatStatic(var.name,seq);

  v = 0;

  if (addTable) {
    addVariableTable(0,&variableTableSize,&variableTable,var);
  } else {
    v = foaVariable(0,var,findOnly);
  }

  return v;
}

/* name of G-variable must start with 'G' */
Biddy_Variable
findoraddGvariable(unsigned int s, unsigned int r, unsigned int t,
                   Biddy_Boolean addTable, Biddy_Boolean findOnly)
{
  VARIABLE var;
  char seq1[5] = "G...";
  char seq2[3] = "R.";
  Biddy_Variable v;
  GANTTITEM *data;

  if (s > NUMPARTS) return 0;

  var.limit.part = 0;
  var.limit.machine = 0;
  var.limit.w = 0;

  foavarname[0] = 0;
  papervarname[0] = 0;
  var.name = foavarname;
  var.papername = papervarname;

#ifdef PAPER
#ifdef LATEX
  sprintf(var.papername,"G^{%u,%u}_{%u}",s,r,t);
#else
  sprintf(var.papername,"G^%u,%u_%u",s,r,t);
#endif
#endif

  /* used for calculation, t must be 0 .. 999 */
  /* used for calculation, r must be 1 .. 9 */
  seq1[1] = (t/100) + '0';
  seq1[2] = ((t/10)%10) + '0';
  seq1[3] = (t%10) + '0';
  seq2[1] = r + '0';
  concatStatic(var.name,seq1);
  concatStatic(var.name,(*(productionTable[s])).name);
  concatStatic(var.name,seq2);

  v = 0;

  if (addTable) {
    printf("G variables should not be included into the variable table\n");
    exit(1);
  } else {
    v = foaVariable(s,var,findOnly);
    data = (GANTTITEM *) malloc(sizeof(GANTTITEM));
    data->part = s;
    data->r = r;
    data->t = t;
    Biddy_SetVariableData(v,(void *) data);
  }

  return v;
}

void createVariables()
{
  int i; /* should not be unsigned */
  Biddy_String name;
  Biddy_Variable v;
  LIMIT *data;

  printf("CREATE VARIABLES\n");

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
    if (variableTable[i].limit.part) {
      data = (LIMIT *) malloc(sizeof(LIMIT));
      data->part = variableTable[i].limit.part;
      data->machine = variableTable[i].limit.machine;
      data->w = variableTable[i].limit.w;
      Biddy_SetVariableData(v,(void *) data);
    }
  }

  /*
  printf("\n");
  */
}

Biddy_Edge encodePart(PART *part, unsigned int activepart, unsigned int r);

Biddy_Edge encodeItem(SEQUENCEITEM item, unsigned int activepart, unsigned int r)
{
  Biddy_Edge code = NULL;

  switch (item.type) {
    case NONE:
      break;
    case PERMUTATION:
      /*
      printf("[]");
      */
      break;
    case DEFAULT:
      /*
      printf("%s (idx = %u)\n",((OPERATION *)(item.ITEM))->name,((OPERATION *)(item.ITEM))->idx);
      */
      if (((OPERATION *)(item.ITEM))->idx <= NUMOPERATIONS) {
        code = Biddy_GetElementEdge(foaOvariable(activepart,((OPERATION *)(item.ITEM))->idx,r));
      } else {
        code = Biddy_GetBaseSet();
      }
      break;
    case ALTERNATIVE:
      code = encodePart((PART *)(item.ITEM),activepart,r);
      break;
  }
  return code;
}

void addPermutationTable(unsigned int *size, PSEQUENCEITEM **table, PSEQUENCEITEM item)
{
  (*size)++;
  if (*size == 1) {
    (*table) = (PSEQUENCEITEM *) malloc(sizeof(PSEQUENCEITEM));
  } else {
    (*table) = (PSEQUENCEITEM *) realloc((*table),(*size) * sizeof(PSEQUENCEITEM));
  }
  (*table)[(*size)-1] = item;
}

Biddy_Edge encodePart(PART *part, unsigned int activepart, unsigned int r)
{
  unsigned int i,j,num;
  unsigned int permutationTableSize;
  PSEQUENCEITEM *permutationTable;
  Biddy_Boolean permutationComplete;
  Biddy_Edge code,code1,code2,code3,sequencecode,itemcode;

  if (!activepart) {
    /*
    printf("ENCODE PART %s\n",part->name);
    */
    activepart = part->part;
  }
  /* for all sequences */
  code = Biddy_GetEmptySet();
  permutationTable = NULL;
  permutationTableSize = 0;
  permutationComplete = FALSE;
  for (i = 1; i <= part->n; i++) {
    num = r;
    /* for all items */
    sequencecode = Biddy_GetBaseSet();
    for (j = 1; j <= part->X[i].n; j++) {
      if (part->X[i].O[j].type == PERMUTATION) {
        /* just add to the Permutation table - it will be encoded later */
        addPermutationTable(&permutationTableSize,&permutationTable,&part->X[i].O[j]);
        if ((j == part->X[i].n) || (part->X[i].O[j+1].type != PERMUTATION)) permutationComplete = TRUE;
      } else {
        itemcode = encodeItem(part->X[i].O[j],activepart,num);
        sequencecode = Biddy_Product(sequencecode,itemcode);
        /*
        printf("ITEM: ");
        Biddy_PrintfMinterms(itemcode,FALSE);
        */
      }
      if (permutationComplete) {
        itemcode = Biddy_GetEmptySet();
        if (permutationTableSize == 0) {
          printf("INTERNAL ERROR (permutationTableSize == 0)\n");
          exit(0);
        } else if (permutationTableSize == 1) {
          itemcode = encodeItem(*(permutationTable[0]),activepart,num);
        } else if (permutationTableSize == 2) {
          /* (0,1) */
          code1 = encodeItem(*(permutationTable[0]),activepart,num-1);
          code2 = encodeItem(*(permutationTable[1]),activepart,num);
          itemcode = Biddy_Union(itemcode,Biddy_Product(code1,code2));
          /* (1,0) */
          code1 = encodeItem(*(permutationTable[1]),activepart,num-1);
          code2 = encodeItem(*(permutationTable[0]),activepart,num);
          itemcode = Biddy_Union(itemcode,Biddy_Product(code1,code2));
        } else if (permutationTableSize == 3) {
          /* (0,1,2) */
          code1 = encodeItem(*(permutationTable[0]),activepart,num-2);
          code2 = encodeItem(*(permutationTable[1]),activepart,num-1);
          code3 = encodeItem(*(permutationTable[2]),activepart,num);
          itemcode = Biddy_Union(itemcode,Biddy_Product(Biddy_Product(code1,code2),code3));
          /* (0,2,1) */
          code1 = encodeItem(*(permutationTable[0]),activepart,num-2);
          code2 = encodeItem(*(permutationTable[2]),activepart,num-1);
          code3 = encodeItem(*(permutationTable[1]),activepart,num);
          itemcode = Biddy_Union(itemcode,Biddy_Product(Biddy_Product(code1,code2),code3));
          /* (1,0,2) */
          code1 = encodeItem(*(permutationTable[1]),activepart,num-2);
          code2 = encodeItem(*(permutationTable[0]),activepart,num-1);
          code3 = encodeItem(*(permutationTable[2]),activepart,num);
          itemcode = Biddy_Union(itemcode,Biddy_Product(Biddy_Product(code1,code2),code3));
          /* (1,2,0) */
          code1 = encodeItem(*(permutationTable[1]),activepart,num-2);
          code2 = encodeItem(*(permutationTable[2]),activepart,num-1);
          code3 = encodeItem(*(permutationTable[0]),activepart,num);
          itemcode = Biddy_Union(itemcode,Biddy_Product(Biddy_Product(code1,code2),code3));
          /* (2,0,1) */
          code1 = encodeItem(*(permutationTable[2]),activepart,num-2);
          code2 = encodeItem(*(permutationTable[0]),activepart,num-1);
          code3 = encodeItem(*(permutationTable[1]),activepart,num);
          itemcode = Biddy_Union(itemcode,Biddy_Product(Biddy_Product(code1,code2),code3));
          /* (2,1,0) */
          code1 = encodeItem(*(permutationTable[2]),activepart,num-2);
          code2 = encodeItem(*(permutationTable[1]),activepart,num-1);
          code3 = encodeItem(*(permutationTable[0]),activepart,num);
          itemcode = Biddy_Union(itemcode,Biddy_Product(Biddy_Product(code1,code2),code3));
        } else {
          /* https://www.geeksforgeeks.org/write-a-c-program-to-print-all-permutations-of-a-given-string/ */
          printf("PERMUTATION WITH MORE THAN 3 ITEMS IS NOT IMPLEMENTED, YET!\n");
          exit(0);
        }          
        /*
        printf("PERMUTATION: ");
        Biddy_PrintfMinterms(itemcode,FALSE);
        */
        sequencecode = Biddy_Product(sequencecode,itemcode);
        free(permutationTable);
        permutationTable = NULL;
        permutationTableSize = 0;
        permutationComplete = FALSE;
      }
      num++;
    }
    code = Biddy_Union(code,sequencecode);
  }
  part->code = code;
  /*
  printf("CODE: ");
  Biddy_PrintfMinterms(code,FALSE);
  */
  return code;
}

Biddy_Edge createZ(unsigned int s, unsigned int w, unsigned int r)
{
  unsigned int i;
  Biddy_Edge z;
  Biddy_Variable O,M;

  O = findOvariable(s,w,r);
  if (!O) {
    printf("createZ: internal error!\n");
    exit(1);
  }

  z = Biddy_GetEmptySet();
  for (i = 1; i <= NUMMACHINES; i++) {
    if (operationMatrix[w + i * (1 + operationTableSize)]) {
      M = foaMvariable(s,w,r,i);
      z = Biddy_Union(z,Biddy_GetElementEdge(M));
    }
  }
  z = Biddy_Change(z,O);
  
  return z;
}

void deleteSystem()
{
  unsigned int i,j;

  for (i = 1; i <= NUMOPERATIONS; i++) {
    free(operationTable[i].name);
  }
  free(operationTable);

  for (i = 1; i <= partTableSize; i++) {
    free(partTable[i].name);
    for (j = 1; j <= partTable[i].n; j++) {
      free(partTable[i].X[j].O);
    }
    free(partTable[i].X);
  }
  free(partTable);

  free(productionTable);

  for (i = 1; i <= NUMMACHINES; i++) {
    free(machineTable[i].name);
  }
  free(machineTable);

  free(operationMatrix);

  for (i=0; i < variableTableSize; i++) {
    free(variableTable[i].name);
#ifdef PAPER
    free(variableTable[i].papername);
#endif
  }
  free(variableTable);
}

void
adaptOrdering(Biddy_String *ordering)
{
  unsigned int i;
  Biddy_String varname,newvarname,newordering;
  Biddy_Boolean first;

  printf("ADAPT ORDERING\n");

#ifdef PAPER
  newordering = strdup("");
#else
  return;
#endif

  varname = strtok(*ordering,"\"");
  first = TRUE;
  while (varname) {
    if (varname) {
      /* printf("ADAPT ORDERING: %s -> ",varname); */
      newvarname = NULL;
      for (i=0; !newvarname && i < variableTableSize; i++) {
        if (!strcmp(varname,variableTable[i].name)) {
          newvarname = variableTable[i].papername;
        }
      }
      if (!newvarname) newvarname = varname;
      /* printf("%s\n",newvarname); */
      if (first) first = FALSE; else concat(&newordering,",");
      concat(&newordering,"\"");
      concat(&newordering,newvarname);
      concat(&newordering,"\"");
    }
    varname = strtok(NULL,"\""); varname = strtok(NULL,"\"");
  }
  free(*ordering);
  *ordering = newordering;
}

Biddy_Edge
permitMakespan(Biddy_Edge f, unsigned int *partlimits, unsigned int *machinelimits, unsigned int n)
{
  Biddy_Edge e, t, r, Fv, Fneg_v;
  Biddy_Variable v;
  Biddy_Boolean OK,islimited;
  unsigned int s;
  unsigned int i;
  LIMIT limit;
  unsigned int *partlimits2;
  unsigned int *machinelimits2;
  void *data;
  PermitMakespanCache *p;
  uintptr_t cidx;

  /* this function is correct only for ZBDD and ZBDDC */

  v = Biddy_GetTopVariable(f);
  if (v == 0) return f;

  /* LOOKUP IN CACHE TABLE */
  cidx = ((uintptr_t) f << 8);
  for (s = 1; s <= NUMPARTS; s++) cidx = cidx + ((uintptr_t) partlimits[s] << 0);
  if (machinelimits) for (i = 1; i <= NUMMACHINES; i++) cidx = cidx + ((uintptr_t) machinelimits[i] << 0);
  p = &permitMakespanCache.table[cidx % permitMakespanCache.size];
  if ((p->r) && (p->f == f))
  {
    OK = TRUE;
    for (s = 1; OK && (s <= NUMPARTS); s++) {
      if (p->n[s-1] != partlimits[s]) OK = FALSE;
    }
    for (i = 1; OK && (i <= NUMMACHINES); i++) {
      if (machinelimits) {
        if (p->m[i-1] != machinelimits[i]) OK = FALSE;
      } else {
        if (p->m[i-1]) OK = FALSE;
      }
    }
    if (OK) {
      r = p->r;
      Biddy_Refresh(r);
      return r;
    }
  }

  /* DETERMINING PARAMETERS FOR RECURSIVE CALLS */
  /* COMPLEMENTED EDGES MUST BE TRANSFERED */
  Fneg_v = Biddy_InvCond(Biddy_GetElse(f),Biddy_GetMark(f));
  Fv = Biddy_GetThen(f);

  islimited = FALSE;
  if (Biddy_GetTopVariableChar(f) == 'G') {
    islimited = FALSE;
  } else {
    if ((data = Biddy_GetVariableData(v))) {
      islimited = TRUE;
      limit = *((LIMIT *) data);
    } else {
      islimited = FALSE;
    }
  }

  partlimits2 = (unsigned int *) malloc((1+NUMPARTS)*sizeof(unsigned int));
  partlimits2[0] = 0;
  for (s = 1; s <= NUMPARTS; s++) {
    partlimits2[s] = partlimits[s];
  }
  if (machinelimits) {
    machinelimits2 = (unsigned int *) malloc((1+NUMMACHINES)*sizeof(unsigned int));
    machinelimits2[0] = 0;
    for (i = 1; i <= NUMMACHINES; i++) {
      machinelimits2[i] = machinelimits[i];
    }
  } else {
    machinelimits2 = NULL;
  }

  e = permitMakespan(Fneg_v,partlimits,machinelimits,n);
  if (islimited) {
    if (limit.w) {
      /* this is M-variable */
      if ((partlimits2[limit.part] < limit.w) || (machinelimits2 && (machinelimits2[limit.machine] < limit.w))) {
        t = Biddy_GetConstantZero();
      } else {
        partlimits2[limit.part] = partlimits2[limit.part] - limit.w;
        if (machinelimits2) machinelimits2[limit.machine] = machinelimits2[limit.machine] - limit.w;
        t = permitMakespan(Fv,partlimits2,machinelimits2,n);
      }
    } else {
      /* this is MX-variable */
      /* this is needed only when MX-variables are required to be before M-variables */
      /* if (machinelimits2) machinelimits2[limit.machine] = machinelimits2[limit.machine] + n; */
      t = permitMakespan(Fv,partlimits2,machinelimits2,n);
    }
  } else {
    t = permitMakespan(Fv,partlimits2,machinelimits2,n);
  }

  free(partlimits2);
  free(machinelimits2);

  r = Biddy_TaggedFoaNode(v,e,t,v,TRUE);

  /* STORE IN CACHE TABLE */
  p->f = f;
  for (s = 1; s <= NUMPARTS; s++) {
    p->n[s-1] = partlimits[s];
  }
  for (i = 1; i <= NUMMACHINES; i++) {
    if (machinelimits) {
      p->m[i-1] = machinelimits[i];
    } else {
      p->m[i-1] = 0;
    }
  }
  p->r = r;

  return r;
}

void
permitMakespanGarbage(Biddy_Manager MNG)
{
  int i;
  PermitMakespanCache *p;

  for (i=0; i<=permitMakespanCache.size; i++) {
    if (permitMakespanCache.table[i].r) {
      p = &permitMakespanCache.table[i];
      if (!Biddy_Managed_IsOK(MNG,p->f) || !Biddy_Managed_IsOK(MNG,p->r))
      {
        p->r = NULL;
      }
    }
  }
}

Biddy_Edge
applyMakespanLimit(Biddy_Edge X, unsigned int partlimit, unsigned int machinelimit, unsigned int *partlimits, unsigned int *machinelimits)
{
  unsigned int s;
  unsigned int i;
  Biddy_Edge f;

  /* if (machinelimit == 0) then this function is correct only if MX-variables are before M-variables */

  if (partlimits) {
    partlimits[0] = 0;
    for (s = 1; s <= NUMPARTS; s++) {
      partlimits[s] = partlimit;
    }
  } else {
    return X;
  }
  if (machinelimits) {
    machinelimits[0] = 0;
    for (i = 1; i <= NUMMACHINES; i++) {
      machinelimits[i] = machinelimit * machineTable[i].n;
    }
  }

  f = permitMakespan(X,partlimits,machinelimits,partlimit);

  return f;
}

Biddy_Edge
permitMachineTime(Biddy_Edge f, unsigned int n)
{
  Biddy_Edge e, t, r, Fv, Fneg_v;
  Biddy_Variable v;
  Biddy_Boolean OK,islimited;
  LIMIT limit;
  void *data;
  PermitMachineTimeCache *p;
  uintptr_t cidx;

  /* this function is correct only for ZBDD and ZBDDC */

  v = Biddy_GetTopVariable(f);
  if (v == 0) return f;

  /* LOOKUP IN CACHE TABLE */
  cidx = ((uintptr_t) f << 8);
  cidx = cidx + ((uintptr_t) n << 0);
  p = &permitMachineTimeCache.table[cidx % permitMachineTimeCache.size];
  if ((p->r) && (p->f == f))
  {
    OK = TRUE;
    if (p->n != n) OK = FALSE;
    if (OK) {
      r = p->r;
      Biddy_Refresh(r);
      return r;
    }
  }

  /* DETERMINING PARAMETERS FOR RECURSIVE CALLS */
  /* COMPLEMENTED EDGES MUST BE TRANSFERED */
  Fneg_v = Biddy_InvCond(Biddy_GetElse(f),Biddy_GetMark(f));
  Fv = Biddy_GetThen(f);

  islimited = FALSE;
  if (Biddy_GetTopVariableChar(f) == 'G') {
    islimited = FALSE;
  } else {
    if ((data = Biddy_GetVariableData(v))) {
      islimited = TRUE;
      limit = *((LIMIT *) data);
    } else {
      islimited = FALSE;
    }
  }

  e = permitMachineTime(Fneg_v,n);
  if (islimited) {
    if (n < limit.w) {
      t = Biddy_GetConstantZero();
    } else {
      t = permitMachineTime(Fv,n-limit.w);
    }
  } else {
    t = permitMachineTime(Fv,n);
  }

  r = Biddy_TaggedFoaNode(v,e,t,v,TRUE);

  /* STORE IN CACHE TABLE */
  p->f = f;
  p->n = n;
  p->r = r;

  return r;
}

void
permitMachineTimeGarbage(Biddy_Manager MNG)
{
  int i;
  PermitMachineTimeCache *p;

  for (i=0; i<=permitMachineTimeCache.size; i++) {
    if (permitMachineTimeCache.table[i].r) {
      p = &permitMachineTimeCache.table[i];
      if (!Biddy_Managed_IsOK(MNG,p->f) || !Biddy_Managed_IsOK(MNG,p->r))
      {
        p->r = NULL;
      }
    }
  }
}

Biddy_Edge
restrictToMinMachineTime(Biddy_Edge X, unsigned int *i)
{
  Biddy_Edge Y;

  *i = 0;
  if (X == Biddy_GetEmptySet()) return X;

  Y = Biddy_GetEmptySet();
  while (Y == Biddy_GetEmptySet()) {
    (*i)++;
    Y = permitMachineTime(X,*i);
  }

  return Y;
}

Biddy_Edge
restrictToMinFS(Biddy_Edge X, unsigned int *fs)
{
  Biddy_Edge Y;
  Biddy_Variable v;

  *fs = 0;
  if (X == Biddy_GetEmptySet()) return X;

  Y = Biddy_GetEmptySet();
  while (Y == Biddy_GetEmptySet()) {
    (*fs)++;
    if (*fs > FACTORYCAPACITY) {
      printf("INTERNAL ERROR: restrictToMinFS got wrong X!\n");
      exit(1);
    }
    v = findFSvariable(*fs);
    Y = Biddy_Subset1(X,v);
  }

  return Y;
}

Biddy_Edge
revertSolution(Biddy_Edge solution, Biddy_Edge *restricted)
{
  unsigned int s,w,r,i,j;
  Biddy_Variable O,M,v;
  Biddy_Edge f,g;
  Biddy_Boolean complete;

#ifdef ONEPASS
  for (v=1; v < Biddy_VariableTableNum(); v++) {
    if ((Biddy_GetVariableName(v))[0] == 'G') {
      solution = Biddy_ElementAbstract(solution,v);
    }
  }
#endif

  for (s = 1; s <= NUMPARTS; s++) {
    for (w = 1; w <= NUMOPERATIONS; w++) {
      r = 1;
      complete = FALSE;
      while (!complete) {
        O = findOvariable(s,w,r);
        if (O) {
          f = Biddy_Subset1(solution,O);
          if (f != Biddy_GetEmptySet()) {
            solution = Biddy_Diff(solution,f);
            for (i = 1; i <= NUMMACHINES; i++) {
              M = findMvariable(s,w,r,i);
              if (M) {
                for (j = 1; j <= machineTable[i].n; j++) {
                  v = findSvariable(s,r,i,j);
                  g = Biddy_Subset1(f,v);
                  if (g != Biddy_GetEmptySet()) {
                    f = Biddy_Diff(f,g);
                    g = Biddy_Change(g,v);
                    g = Biddy_Change(g,M);
                    if (*restricted) *restricted = Biddy_Change(*restricted,v);
                    f = Biddy_Union(f,g);
                  }
                }
              }
            }
            solution = Biddy_Union(solution,f);
          }
        }
        if (r == MAXSEQUENCELENGTH) complete = TRUE;
        r++;
      }
    }
  }

  return solution;
}

/* ************************************************************************** */
/* NEEDED FOR DEBUGGING  */
/* ************************************************************************** */

void
check(Biddy_Edge X)
{
  unsigned int s,w,r,i,j;
  Biddy_Variable O,M,v;
  Biddy_Edge f,g;
  Biddy_Boolean complete;

  /* cubes with R[s,r] should not include M[s,w,r,i] */
  for (s = 1; s <= NUMPARTS; s++) {
    for (w = 1; w <= NUMOPERATIONS; w++) {
      r = 1;
      complete = FALSE;
      while (!complete) {
        v = findRvariable(s,r);
        for (i = 1; i <= NUMMACHINES; i++) {
          M = findMvariable(s,w,r,i);
          if (M) {
            f = Biddy_Subset1(X,v);
            f = Biddy_Subset1(f,M);
            if (f != Biddy_GetEmptySet()) {
              printf("INTERNAL ERROR - WRONG M VARIABLE!\n");
              PRINTCOMBINATIONS(f);
              exit(1);
            }
          }
        }
        if (r == MAXSEQUENCELENGTH) complete = TRUE;
        r++;
      }
    }
  }

  /* find cubes including B[i,j] */
  f = Biddy_GetEmptySet();
  for (s = 1; s <= NUMPARTS; s++) {
    for (w = 1; w <= NUMOPERATIONS; w++) {
      r = 1;
      complete = FALSE;
      while (!complete) {
        for (i = 1; i <= NUMMACHINES; i++) {
          M = findMvariable(s,w,r,i);
          if (M) {
            for (j = 1; j <= machineTable[i].n; j++) {
              g = Biddy_Subset1(X,findBvariable(i,j));
              f = Biddy_Union(f,g);
            }
          }
        }
        if (r == MAXSEQUENCELENGTH) complete = TRUE;
        r++;
      }
    }
  }
  
  /* if there is no O[s,w,r] included in all cubes then this is an error */
  for (s = 1; s <= NUMPARTS; s++) {
    for (w = 1; w <= NUMOPERATIONS; w++) {
      r = 1;
      complete = FALSE;
      while (!complete) {
        O = findOvariable(s,w,r);
        if (O) {
          g = Biddy_Subset1(f,O);
          f = Biddy_Diff(f,g);
        }
        if (r == MAXSEQUENCELENGTH) complete = TRUE;
        r++;
      }
    }
  }
  
  if (f != Biddy_GetEmptySet()) {
    printf("INTERNAL ERROR - MISSING O VARIABLE!\n");
    PRINTCOMBINATIONS(f);
    exit(1);
  }
}

/* ************************************************************************** */
/* NEEDED FOR GANTT CHART  */
/* ************************************************************************** */

/* this will extend GANTT with new data */
void addGanttItem(GANTT *g, unsigned int part, Biddy_String c1, Biddy_String c2, unsigned int gr, unsigned int s, unsigned int e, unsigned int h)
{
  unsigned int n;

  n = g->n;
  if (g->n) {
    g->content = (Biddy_String *) realloc(g->content,(n+1)*sizeof(Biddy_String));
    g->id = (unsigned int *) realloc(g->id,(n+1)*sizeof(unsigned int));
    g->group = (Biddy_String *) realloc(g->group,(n+1)*sizeof(Biddy_String));
    g->start = (unsigned int *) realloc(g->start,(n+1)*sizeof(unsigned int));
    g->end = (unsigned int *) realloc(g->end,(n+1)*sizeof(unsigned int));
    g->color = (Biddy_String *) realloc(g->color,(n+1)*sizeof(Biddy_String));
  } else {
    g->content = (Biddy_String *) malloc(sizeof(Biddy_String));
    g->id = (unsigned int *) malloc(sizeof(unsigned int));
    g->group = (Biddy_String *) malloc(sizeof(Biddy_String));
    g->start = (unsigned int *) malloc(sizeof(unsigned int));
    g->end = (unsigned int *) malloc(sizeof(unsigned int));
    g->color = (Biddy_String *) malloc(sizeof(Biddy_String));
  }

  g->n = n+1;
  g->content[n] = NULL;
  if (c1) g->content[n] = strdup(c1);
  if (c1 && c2) concat(&(g->content[n]),"@");
  if (c1 && c2) concat(&(g->content[n]),c2);
  g->id[n] = n;
  g->group[n] = strdup(Biddy_GetVariableName(gr));
/*
  g->group[n] = strdup("\"");
  g->group[n] = concat(&(g->group[n]),Biddy_GetVariableName(gr));
  g->group[n] = concat(&(g->group[n]),"\"");
*/
  g->start[n] = s;
  g->end[n] = e;
  g->color[n] = strdup("\"background-color:hsl(");
  concatNumber(&(g->color[n]),h);
  if (part < 7) {
    concat(&(g->color[n]),",80%,80%);\"");
  } else if (part < 13) {
    concat(&(g->color[n]),",40%,85%);\"");
  } else {
    concat(&(g->color[n]),",20%,90%);\"");
  }

  /* DEBUGGING */
  /*
  for (i=0; i < g->n; i++) {
    printf("<%s,%u,%u,%u,%u>\n",g->content[i],g->id[i],g->group[i],g->start[i],g->end[i]);
  }
  */
}

/* ************************************************************************** */
/* PRODUCTION PLANING  */
/* ************************************************************************** */

Biddy_Edge feasibleCombinations()
{
  clock_t elapsedtime;
  unsigned int s,w,r;
  unsigned int i,j;
  /* unsigned int count; */
  Biddy_Edge X,Z,Q,R,Y,Mall,Mng;
  Biddy_Edge *MI;
  Biddy_Variable O,M;
  Biddy_Boolean complete;
  Biddy_Edge f;
  unsigned int *partlimits;
  unsigned int *machinelimits;

  printf("\nPROCESS PLANS\n");

  elapsedtime = clock();

  if (VERBOSE) {
    for (s = 1; s <= NUMPARTS; s++) {
#ifdef LIMITSINGLE
#ifdef LIMITDOUBLE
      if ((s != LIMITSINGLE) && (s != LIMITDOUBLE)) continue;
#else
      if (s != LIMITSINGLE) continue;
#endif
#endif
      X = (*(productionTable[s])).code;
#ifdef INTERNALREPORTS
      printf("Xinit(%u): \n",s);
      PRINTCOMBINATIONS(X);
#else
      printf("Xinit(%u): \n",s);
      STATCOMBINATIONS(X);
#endif
#ifdef LIMITSINGLE
      if (s == LIMITSINGLE) {
        Biddy_WriteDot("Xinit.dot",X,"Xinit",-1,FALSE);
        printf("Use 'dot -y -Tpng -O Xinit.dot' to visualize process for %s\n",(*(productionTable[s])).name);
        Biddy_WriteBddview("Xinit.bddview",X,"Xinit",NULL);
        printf("Use BDD Scout to open Xinit.bddview\n");
      }
#endif
    }
    printf("\n");
  }

  for (s = 1; s <= NUMPARTS; s++) {
#ifdef LIMITSINGLE
#ifdef LIMITDOUBLE
    if ((s != LIMITSINGLE) && (s != LIMITDOUBLE)) continue;
#else
    if (s != LIMITSINGLE) continue;
#endif
#endif

    X = (*(productionTable[s])).code;

    elapsedtime = clock();

    for (w = 1; w <= NUMOPERATIONS; w++) {

      r = 1;
      complete = FALSE;
      while (!complete) {
        O = findOvariable(s,w,r);
        if (O) {
          /*
          printf("*************\n");
          printf("s = %u, w = %u, r = %u\n",s,w,r);
          */

          Z = createZ(s,w,r);
          /*
          printf("Z(%u,%u,%u): ",s,w,r);
          Biddy_PrintfMinterms(Z,FALSE);
          */

          Q = Biddy_Quotient(X,O);
          /*
          printf("X(%u)/O(%u,%u,%u): ",s,s,w,r);
          Biddy_PrintfMinterms(Q,FALSE);
          */

          Q = Biddy_Product(Q,Z);
          /*
          printf("QxZ: ");
          Biddy_PrintfMinterms(Q,FALSE);
          */

          R = Biddy_Remainder(X,O);
          /*
          printf("X(%u)%%O(%u,%u,%u): ",s,s,w,r);
          Biddy_PrintfMinterms(R,FALSE);
          */

          X = Biddy_Union(Q,R);
          /*
          printf("X(%u): ",s);
          Biddy_PrintfMinterms(X,FALSE);
          */
        }

        if (r == MAXSEQUENCELENGTH) complete = TRUE;
        r++;
      }
    }

    (*(productionTable[s])).feasible = X;

    /* REPORT RESULT AND ELAPSED TIME */
    /**/
    printf("X(%u): ",s);
    STATCOMBINATIONS(X);
    /**/

    if (VERBOSE) {
#ifdef INTERNALREPORTS
      printf("X(%u): \n",s);
      PRINTCOMBINATIONS(X);
#endif
#ifdef LIMITSINGLE
      if (s == LIMITSINGLE) {
        Biddy_WriteDot("X.dot",X,"X",-1,FALSE);
        printf("Use 'dot -y -Tpng -O X.dot' to visualize process plan for %s\n",(*(productionTable[s])).name);
        Biddy_WriteBddview("X.bddview",X,"X",NULL);
        printf("Use BDD Scout to open X.bddview\n");
      }
#endif
    }

    Biddy_KeepFormulaUntilPurge((*(productionTable[s])).feasible);
    Biddy_Clean();
  }

  /* CALCULATE BDD FOR THE COMPREHENSIVE PROCESS PLANS */

  elapsedtime = clock();

  X = Biddy_GetBaseSet();
  for (s = 1; s <= NUMPARTS; s++)
  {

#ifdef LIMITSINGLE
#ifdef LIMITDOUBLE
    if ((s != LIMITSINGLE) && (s != LIMITDOUBLE)) continue; /* limit to two processes */
#else
    if (s != LIMITSINGLE) continue; /* limit to a single process */
#endif
#endif

    f = (*(productionTable[s])).feasible;

    /*
    printf("(BEFORE PRODUCT) ");
    printf("%s for X(%u): %u nodes (including terminals), %.0f cubes, ",
           Biddy_GetManagerName(),s,Biddy_CountNodes(X),Biddy_CountMinterms(X,0));
    printf("clock() TIME = %.2f\n",(clock()-elapsedtime)/(1.0*CLOCKS_PER_SEC));
    */

    X = Biddy_Product(X,f);
    Biddy_KeepFormula(X);
    Biddy_Clean();

    /* RESTRICT TO THE GIVEN BOUND FOR TOTAL MACHINE TIME */

    if (TIMEBOUND) {
      X = permitMachineTime(X,TIMEBOUND);
      Biddy_KeepFormula(X);
      Biddy_Clean();
    }

    /* RESTRICT TO THE GIVEN BOUND FOR MAKESPAN */
    /* this seems to be wrong idea, number of cubes is not reduced */
    /* but the calculation is much slower */
    /*
    if (MAKESPANLIMIT) {
      partlimits = (unsigned int *) malloc((1+NUMPARTS)*sizeof(unsigned int));
      machinelimits = (unsigned int *) malloc((1+NUMMACHINES)*sizeof(unsigned int));
      X = applyMakespanLimit(X,MAKESPANLIMIT,MAKESPANLIMIT,partlimits,machinelimits);
      free(partlimits);
      free(machinelimits);
      Biddy_KeepFormula(X);
      Biddy_Clean();
    }
    */
    
    /* SIFTING ON PARTIAL RESULTS */
    /*
    printf("(BEFORE SIFTING) ");
    printf("%s for X(%u): %u nodes (including terminals), %.0f cubes, ",
           Biddy_GetManagerName(),s,Biddy_CountNodes(X),Biddy_CountMinterms(X,0));
    printf("clock() TIME = %.2f\n",(clock()-elapsedtime)/(1.0*CLOCKS_PER_SEC));
    Biddy_Sifting(X,FALSE);
    */

    /* CONVERGING SIFTING ON PARTIAL RESULTS */
    /*
    printf("(BEFORE CONVERGING SIFTING) ");
    printf("%s for X(%u): %u nodes (including terminals), %.0f cubes, ",
           Biddy_GetManagerName(),s,Biddy_CountNodes(X),Biddy_CountMinterms(X,0));
    printf("clock() TIME = %.2f\n",(clock()-elapsedtime)/(1.0*CLOCKS_PER_SEC));
    Biddy_Sifting(X,TRUE);
    */

    /* REPORT PARTIAL RESULTS */
    /*
    printf("%s for X(%u): %u nodes (including terminals), %.0f cubes, ",
           Biddy_GetManagerName(),s,Biddy_CountNodes(X),Biddy_CountMinterms(X,0));
    printf("clock() TIME = %.2f\n",(clock()-elapsedtime)/(1.0*CLOCKS_PER_SEC));
    */

  }

  /* REPORT RESULT AND ELAPSED TIME */
  /**/
  printf("\nCOMPREHENSIVE PROCESS PLAN");
  if (TIMEBOUND) printf(" (RESTRICTED TO TOTAL MACHINE TIME = %u)",TIMEBOUND);
  printf("\n");
  STATCOMBINATIONS(X);
  /**/

#ifdef INTERNALREPORTS
  printf("\nCOMPREHENSIVE PROCESS PLAN");
  if (TIMEBOUND) printf(" (RESTRICTED TO TOTAL MACHINE TIME = %u)",TIMEBOUND);
  printf("\n");
  PRINTCOMBINATIONS(X);
#endif

  /* SIFTING ON RESULT */
  /*
  printf("\nSIFTING ON X\n");
  elapsedtime = clock();
  Biddy_Sifting(X,FALSE);
  Biddy_Sifting(X,TRUE);
  STATCOMBINATIONS(X);
  reportOrdering();
  */

  /* SAVE FINAL BDD TO FILE*/
  /*
  Biddy_WriteBddview("X.bddview",X,"X",NULL);
  */

  /* CALCULATE BDD FOR THE PROCESS AND RESOURCE PLANS */

  elapsedtime = clock();

#ifdef VARIANTA
  /* VARIANT A - USED IN THE 2015 PAPER */
  MI = (Biddy_Edge *) malloc((1 + NUMMACHINES) * sizeof(Biddy_Edge));
  MI[0] = NULL;
  for (i = 1; i <= NUMMACHINES; i++) {
    MI[i] = Biddy_GetBaseSet();
    for (j = 1; j <= machineTable[i].n; j++) {
      if (!findMXvariable(i,j)) {
        /* foaMXvariable(i,j); */ /* use this if initialization is not complete */
      }
      if (findMXvariable(i,j)) {
        MI[i] = Biddy_Product(MI[i],Biddy_Union(Biddy_GetBaseSet(),Biddy_GetElementEdge(foaMXvariable(i,j))));
      }
    }
    MI[i] = Biddy_Diff(MI[i],Biddy_GetBaseSet());
    Biddy_KeepFormulaUntilPurge(MI[i]);
  }
#endif

#ifdef VARIANTB
  /* VARIANT B - APPLY LIMITS EARLIER USING THE MOST EFFICIENT METHOD */
  MI = (Biddy_Edge *) malloc((1 + NUMMACHINES) * sizeof(Biddy_Edge));
  MI[0] = NULL;
  for (i = 1; i <= NUMMACHINES; i++) {
    MI[i] = NULL;
    if (!findMXvariable(i,machineTable[i].n)) {
      /* foaMXvariable(i,machineTable[i].n); */ /* use this if initialization is not complete */
    }
    if (findMXvariable(i,machineTable[i].n)) {
      MI[i] = Biddy_GetElementEdge(foaMXvariable(i,machineTable[i].n));
      for (j = machineTable[i].n-1; j>0; j--) {
        MI[i] = Biddy_Product(Biddy_Union(MI[i],Biddy_GetBaseSet()),Biddy_GetElementEdge(foaMXvariable(i,j)));
      }
    }
    if (MI[i]) Biddy_KeepFormulaUntilPurge(MI[i]);
  }
#endif

#ifdef VARIANTC
  /* VARIANT C - APPLY LIMITS EARLIER BUT KEEP THE SAME COMBINATIONS AS VARIANT A */
  MI = (Biddy_Edge *) malloc((1 + NUMMACHINES) * sizeof(Biddy_Edge));
  MI[0] = NULL;
  for (i = 1; i <= NUMMACHINES; i++) {
    MI[i] = Biddy_GetBaseSet();
    for (j = 1; j <= machineTable[i].n; j++) {
      if (!findMXvariable(i,j)) {
        /* foaMXvariable(i,j); */ /* use this if initialization is not complete */
      }
      if (findMXvariable(i,j)) {
        MI[i] = Biddy_Product(MI[i],Biddy_Union(Biddy_GetBaseSet(),Biddy_GetElementEdge(foaMXvariable(i,j))));
      }
    }
    MI[i] = Biddy_Diff(MI[i],Biddy_GetBaseSet());
    if (machineTable[i].n > 1) {
      for (count = 1; count <= machineTable[i].n; count++) {
        f = Biddy_Diff(Biddy_Permitsym(MI[i],count),Biddy_Permitsym(MI[i],count-1));
        if (Biddy_CountMinterms(f,0) > 1) {
          MI[i] = Biddy_Diff(MI[i],f);
          f = Biddy_ExtractMinterm(NULL,f);
          MI[i] = Biddy_Union(MI[i],f);
        }
      }
    }
    Biddy_KeepFormulaUntilPurge(MI[i]);
  }
#endif

  for (s = 1; s <= NUMPARTS; s++) {
    for (w = 1; w <= NUMOPERATIONS; w++) {
      r = 1;
      complete = FALSE;
      while (!complete) {
        for (i = 1; i <= NUMMACHINES; i++) {
          M = findMvariable(s,w,r,i);
          if (M) {

            Y = Biddy_Supset(X,Biddy_GetElementEdge(M));
            if (Y == Biddy_GetEmptySet()) continue;

            /*
            printf("*************\n");
            printf("s = %u, w = %u, r = %u, i = %u\\\\\n",s,w,r,i);
            */

            /*
            printf("$Y$: ");
            Biddy_PrintfMinterms(Y,FALSE);
            */

            f = Biddy_Supset(Y,MI[i]);

            /*
            printf("$Restrict(Y,M_%u)$: ",i);
            Biddy_PrintfMinterms(f,FALSE);
            */

            Y = Biddy_Diff(Y,f);

            /*
            printf("$Y - Restrict(Y,M_%u)$: ",i);
            Biddy_PrintfMinterms(Y,FALSE);
            */

            Y = Biddy_Product(Y,MI[i]);

            /*
            printf("$(Y - Restrict(Y,M_%u)) x M_%u$: ",i,i);
            Biddy_PrintfMinterms(Y,FALSE);
            */

            Y = Biddy_Union(Y,f);
            f = Biddy_Remainder(X,M);

            /*
            printf("$X\\%%M_{%u}^{%u,%u,%u}$: ",i,s,w,r);
            Biddy_PrintfMinterms(f,FALSE);
            */

            X = Biddy_Union(Y,f);

            /*
            printf("$X$: ");
            Biddy_PrintfMinterms(X,FALSE);
            */

            Biddy_KeepFormula(X);
            Biddy_Clean();
          }
        }
        if (r == MAXSEQUENCELENGTH) complete = TRUE;
        r++;
      }
    }
  }

  /**/
  printf("\nPROCESS AND RESOURCE PLANS\n");
  STATCOMBINATIONS(X);
  /**/

#ifdef INTERNALREPORTS
  printf("\nPROCESS AND RESOURCE PLANS\n");
  PRINTCOMBINATIONS(X);
#endif

  /* RESTRICT FOR MAX NUMBER OF INSTALLABLE MACHINES (variable FACTORYCAPACITY) */

  elapsedtime = clock();

  Mall = Biddy_GetBaseSet();
  for (i = 1; i <= NUMMACHINES; i++) {
    if (MI[i]) Mall = Biddy_Product(Mall,Biddy_Union(Biddy_GetBaseSet(),MI[i]));
  }

  /* Mall = Biddy_Diff(Mall,Biddy_GetBaseSet()); */ /* not needed */
  Mng = Biddy_Diff(Mall,Biddy_Permitsym(Mall,FACTORYCAPACITY));
  X = Biddy_Diff(X,Biddy_Supset(X,Mng));

  if (MAXFACTORYONLY) {
    Mall = Biddy_Diff(Mall,Biddy_Permitsym(Mall,FACTORYCAPACITY-1));
    X = Biddy_Supset(X,Mall);
  }

  Biddy_KeepFormula(X);
  Biddy_Clean();

  /**/
  printf("\nRESTRICTED FOR MAX NUMBER OF INSTALLABLE MACHINES\n");
  STATCOMBINATIONS(X);
  /**/

#ifdef INTERNALREPORTS
  printf("\nRESTRICTED FOR MAX NUMBER OF INSTALLABLE MACHINES\n");
  PRINTCOMBINATIONS(X);
#endif
  
#ifdef VARIANTA
  /* RESTRICT FOR EFFICIENCY - NOT NEEDED IF NO REDUNDANCE HAS BEEN INTRODUCED BEFORE */
  Y = X;
  elapsedtime = clock();
  for (i = 1; i <= NUMMACHINES; i++) {
    if (machineTable[i].n > 1) {
      for (count = 1; count <= machineTable[i].n; count++) {
        f = Biddy_Diff(Biddy_Permitsym(MI[i],count),Biddy_Permitsym(MI[i],count-1));
        if (Biddy_CountMinterms(f,0) > 1) {
          g = Biddy_Supset(X,f);
          X = Biddy_Diff(X,g);
          f = Biddy_ExtractMinterm(NULL,f);
          g = Biddy_Supset(g,f);
          X = Biddy_Union(X,g);
        }
      }
    }
  }
  Biddy_KeepFormula(X);
  Biddy_Clean();
  printf("\nRESTRICTED FOR EFFICIENCY\n");
  if (X == Y) {
    printf("Result is the same function!\n");
  } else {
    STATCOMBINATIONS(X);
  }
#endif

  /* RESTRICT TO MINIMAL TOTAL MACHINE TIME */

  if (AUTOBOUND && (X != Biddy_GetEmptySet())) {
    elapsedtime = clock();

    Y = restrictToMinMachineTime(X,&i);
    if (BOUNDINTERVAL) {
      i = i + BOUNDINTERVAL;
      Y = permitMachineTime(X,i);
    }
    X = Y;
    TIMEBOUND = i;
    Biddy_KeepFormula(X);
    Biddy_Clean();

    /**/
    printf("\nRESTRICTED TO MINIMAL TOTAL MACHINE TIME: %u\n",i);
    STATCOMBINATIONS(X);
    /**/

#ifdef INTERNALREPORTS
    printf("\nRESTRICTED TO MINIMAL TOTAL MACHINE TIME: %u\n",i);
    PRINTCOMBINATIONS(X);
#endif
  }

  /* RESTRICT TO MAKESPAN */

  if (DYNAMICLIMIT && (X != Biddy_GetEmptySet())) {
    i = 0;
    partlimits = (unsigned int *) malloc((1+NUMPARTS)*sizeof(unsigned int));
    machinelimits = (unsigned int *) malloc((1+NUMMACHINES)*sizeof(unsigned int));
    Y = Biddy_GetEmptySet();
    while (Y == Biddy_GetEmptySet()) {
      i++;
      /* this can be used only if MX-variables are before M-variables */
      /* Y = applyMakespanLimit(X,i,0,partlimits,machinelimits); */
      /* this is less efficient only if machines have more than one instance */
      /* Y = applyMakespanLimit(X,i,i,partlimits,machinelimits); */
      /* this should be worse than the previous solution but we get better results! */
      Y = applyMakespanLimit(X,i,i,partlimits,NULL);
    }
    if (LIMITINTERVAL) {
      i = i + LIMITINTERVAL;
      /* this can be used only if MX-variables are before M-variables */
      /* Y = applyMakespanLimit(X,i,0,partlimits,machinelimits); */
      /* this is less efficient only if machines have more than one instance */
      /* Y = applyMakespanLimit(X,i,i,partlimits,machinelimits); */
      /* this should be worse than the previous solution but we get better results! */
      Y = applyMakespanLimit(X,i,i,partlimits,NULL);
    }
    X = Y;
    free(partlimits);
    free(machinelimits);
    Biddy_KeepFormula(X);
    Biddy_Clean();

    /**/
    printf("\nRESTRICTED TO MAKESPAN: %u\n",i);
    STATCOMBINATIONS(X);
    /**/

#ifdef INTERNALREPORTS
    printf("\nRESTRICTED TO MAKESPAN: %u\n",i);
    PRINTCOMBINATIONS(X);
#endif
  }

  return X;
}

/* ************************************************************************** */
/* SCHEDULING  */
/* ************************************************************************** */

/* here are given limits and bounds which are applied after every scheduling step */
Biddy_Edge
stepLimitsBounds(Biddy_Edge X, unsigned int limit, Biddy_Boolean createG, unsigned int count)
{
  Biddy_Edge f;
  unsigned int dlimit;
  unsigned int *partlimits;
  unsigned int *machinelimits;

  /* TOTAL MACHINE TIME BOUND IS NOT CHANGED DURING SCHEDULING */
  /* THUS, IT IS NOT NECCESSARY TO APPLY THIS BOUND IN EVERY STEP */

  /* APPLY DYNAMIC MAKESPAN LIMIT */
  /* ONLY THE SCHEDULES WITH CURRENTLY MINIMAL MAKESPAN ARE KEPT */
  /* machinelimit is not given, it will be obtained from MX-variables */

  if (DYNAMICLIMIT && (X != Biddy_GetEmptySet()))
  {
    dlimit = 0;
    partlimits = (unsigned int *) malloc((1+NUMPARTS)*sizeof(unsigned int));
    machinelimits = (unsigned int *) malloc((1+NUMMACHINES)*sizeof(unsigned int));
    f = Biddy_GetEmptySet();
    while ((!limit || (dlimit < limit-count)) && (f == Biddy_GetEmptySet())) {
      dlimit++;
      /* this can be used only if MX-variables are before M-variables */
      /* f = applyMakespanLimit(X,dlimit,0,partlimits,machinelimits); */
      /* this is less efficient only if machines have more than one instance */
      /**/ f = applyMakespanLimit(X,dlimit,dlimit,partlimits,machinelimits); /**/
      /* this should be worse than the previous solution but we get better results! */
      /* f = applyMakespanLimit(X,dlimit,dlimit,partlimits,NULL); */
    }
    if (LIMITINTERVAL && (X != Biddy_GetEmptySet())) {
      dlimit = dlimit + LIMITINTERVAL;
      if (limit && (dlimit > limit - count)) dlimit = limit - count;
      /* this can be used only if MX-variables are before M-variables */
      /* f = applyMakespanLimit(X,dlimit,0,partlimits,machinelimits); */
      /* this is less efficient only if machines have more than one instance */
      /**/ f = applyMakespanLimit(X,dlimit,dlimit,partlimits,machinelimits); /**/
      /* this should be worse than the previous solution but we get better results! */
      /* f = applyMakespanLimit(X,dlimit,dlimit,partlimits,NULL); */
    }
    if (f != X) {
      if (createG) {
        /*
        printf("\nDYNAMIC MAKESPAN LIMIT: %u\n",dlimit);
        printf("KEPT AFTER APPLYING DYNAMIC LIMIT: %.0f/%.0f cubes\n",Biddy_CountMinterms(f,0),Biddy_CountMinterms(X,0));
        */
      } else {
        printf("\nDYNAMIC MAKESPAN LIMIT: %u\n",dlimit);
        printf("KEPT AFTER APPLYING DYNAMIC LIMIT: %.0f/%.0f cubes\n",Biddy_CountMinterms(f,0),Biddy_CountMinterms(X,0));
      }
    }
    if (f == Biddy_GetEmptySet()) {
      if (!createG) {
        printf("UNDO APPLYING DYNAMIC MAKESPAN LIMIT BECAUSE IT WOULD REMOVE ALL SOLUTIONS\n");
      }
    } else {
      X = f;
    }
    Biddy_KeepFormula(X);
    Biddy_Clean();
    free(partlimits);
    free(machinelimits);
  }

  /* APPLY MAKESPAN LIMIT */
  /* ONLY THE SCHEDULES WITH GIVEN MAKESPAN ARE KEPT */

  if (!DYNAMICLIMIT && limit && (X != Biddy_GetEmptySet())) {
    partlimits = (unsigned int *) malloc((1+NUMPARTS)*sizeof(unsigned int));
    machinelimits = (unsigned int *) malloc((1+NUMMACHINES)*sizeof(unsigned int));
    /* this can be used only if MX-variables are before M-variables */
    /* f = applyMakespanLimit(X,limit-count,0,partlimits,machinelimits); */
    /* this is less efficient only if machines have more than one instance */
    /**/ f = applyMakespanLimit(X,limit-count,limit-count,partlimits,machinelimits); /**/
    /* this should be worse than the previous solution but we get better results! */
    /* f = applyMakespanLimit(X,limit-count,limit-count,partlimits,NULL); */
    if (f != X) {
      if (createG) {
        /*
        printf("\nKEPT AFTER APPLYING MAKESPAN LIMIT: %.0f/%.0f cubes\n",Biddy_CountMinterms(f,0),Biddy_CountMinterms(X,0));
        */
      } else {
        printf("\nKEPT AFTER APPLYING MAKESPAN LIMIT: %.0f/%.0f cubes\n",Biddy_CountMinterms(f,0),Biddy_CountMinterms(X,0));
      }
    }
    if (createG) {
      if (f == Biddy_GetEmptySet()) {
        /*
        printf("UNDO APPLYING DYNAMIC LIMIT BECAUSE IT WOULD REMOVE ALL SOLUTIONS\n");
        */
      } else {
        X = f;
      }
    } else {
      X = f;
    }
    Biddy_KeepFormula(X);
    Biddy_Clean();
    free(partlimits);
    free(machinelimits);
  }

  return X;
}

/* limit = limit number of steps */
/* limit == 0 means no limit */
/* restricted = a set of machine instances, specified with S variables */
/* restricted == NULL means no restriction */
/* restricted should be used for X with a single cube, only, */
/* restricted should make calculation more efficient by reusing the previously obtained result */
/* function stores results in global variables */
void runScheduling(Biddy_Edge X, unsigned int limit, Biddy_Edge restricted, Biddy_Boolean createG)
{
  clock_t elapsedtime;
  unsigned int s,w,r;
  unsigned int i,j,k,count;
  Biddy_Variable M,v,v1,v2,v3;
  Biddy_Boolean complete,scheduling;
  Biddy_Edge f,g,Y;
  Biddy_Edge tr1,tr1cube;

  elapsedtime = clock();

  free(RESULT);
  RESULT = NULL;
  RESULTSIZE = 0;

  /* TESTING - VARIABLE restricted IS NOT CORRECT, YET! */
  /**/
  restricted = NULL;
  /**/

  if (restricted && (Biddy_CountMinterms(X,0) != 1)) {
    printf("\nINTERNAL ERROR: wrong X in runScheduling\n");
    PRINTCOMBINATIONS(X);
    exit(1);
  }

  if (createG) {
    /*
    printf("\n********************* SCHEDULING *********************\\\\\n");
    printf("\nX: ");
#ifdef INTERNALREPORTS
    PRINTCOMBINATIONS(X);
#else
    STATCOMBINATIONS(X);
#endif
    */
  } else {
    /**/
    printf("\n********************* SCHEDULING *********************\\\\\n");
    printf("\nX: ");
#ifdef INTERNALREPORTS
    PRINTCOMBINATIONS(X);
#else
    STATCOMBINATIONS(X);
#endif
    /**/
  }

  /* SIFTING ON X */
  /*
  if (!createG) {
    printf("\nSIFTING ON X\n");
    elapsedtime = clock();
    Biddy_Sifting(X,FALSE);
    Biddy_Sifting(X,TRUE);
    STATCOMBINATIONS(X);
    reportOrdering();
  }
  */

  /* SCHEDULING - CREATE tr1 AND tr1cube */
  /* prepare tr1 - M[s,w,r,i]*W[s,r]*MX[i,j]*^B[i,j]*^R[s,r]*S[s,r,i,j]*T[s,n] */
  /* in tr1cube you must use product instead of change */
  /* if restricted is given, then tr1 is much smaller */
  /* if DYNAMICLIMIT is used then restricted should be ignored - it is to aggressive */
  tr1 = Biddy_GetEmptySet();
  tr1cube = Biddy_GetBaseSet();
  for (s = 1; s <= NUMPARTS; s++) {
    for (w = 1; w <= NUMOPERATIONS; w++) {
      r = 1;
      complete = FALSE;
      while (!complete) {
        for (i = 1; i <= NUMMACHINES; i++) {
          M = findMvariable(s,w,r,i);
          if (M) {
            for (j = 1; j <= machineTable[i].n; j++) {
              v = findSvariable(s,r,i,j);
              if (DYNAMICLIMIT || !restricted || (Biddy_Subset1(restricted,v) != Biddy_GetEmptySet())) {
                f = Biddy_GetBaseSet();
                g = Biddy_GetElementEdge(M); tr1cube = Biddy_Product(tr1cube,g); f = Biddy_Change(f,M);
                g = Biddy_GetElementEdge(v=findWvariable(s,r)); tr1cube = Biddy_Product(tr1cube,g); f = Biddy_Change(f,v);
                g = Biddy_GetElementEdge(v=findMXvariable(i,j)); tr1cube = Biddy_Product(tr1cube,g); f = Biddy_Change(f,v);
                v = findBvariable(i,j); tr1cube = Biddy_E(tr1cube,v); f = Biddy_Change(f,v);
                v = findRvariable(s,r); tr1cube = Biddy_E(tr1cube,v); f = Biddy_Change(f,v);
                v = findSvariable(s,r,i,j); f = Biddy_Change(f,v);
                v = findTvariable(s,operationMatrix[w+i*(1+operationTableSize)]); f = Biddy_Change(f,v);
                tr1 = Biddy_Union(tr1,f);
              }
            }
          }
        }
        if (r == MAXSEQUENCELENGTH) complete = TRUE;
        r++;
      }
    }
  }
  Biddy_KeepFormulaUntilPurge(tr1);
  Biddy_KeepFormulaUntilPurge(tr1cube);

  /*
  printf("\nTR: ");
  STATCOMBINATIONS(tr1);
  */

#ifdef INTERNALREPORTS
  if (!createG) {
    printf("\nTR: ");
    PRINTCOMBINATIONS(tr1);
  }
#endif

  for (s = 1; s <= NUMPARTS; s++) {
    for (w = 1; w <= NUMOPERATIONS; w++) {
      r = 1; /* activate the first operation, only */
      for (i = 1; i <= NUMMACHINES; i++) {
        M = findMvariable(s,w,r,i);
        if (M) {
          /* printf("CALCULATE FOR M(%u,%u,%u,%u)\n",s,w,r,i); */
          f = X;
          f = Biddy_Subset1(f,M);
          X = Biddy_Diff(X,f);
          f = Biddy_Product(f,Biddy_GetElementEdge(findWvariable(s,r)));
          X = Biddy_Union(X,f);
        }
      }
    }
  }
  Biddy_KeepFormula(X);
  Biddy_Clean();

  /*
  printf("\nAFTER THE INITIALIZATION: ");
  STATCOMBINATIONS(X);
  */

#ifdef INTERNALREPORTS
  printf("\nAFTER THE INITIALIZATION: ");
  PRINTCOMBINATIONS(X);
#endif

  trace = NULL; /* this is required, do not delete */

  /* DEBUGGING */
  /*
  trace = Biddy_GetBaseSet();
  trace = Biddy_Change(trace,findOvariable(1,3,1));
  trace = Biddy_Change(trace,findOvariable(1,1,2));
  trace = Biddy_Change(trace,findOvariable(1,4,3));
  trace = Biddy_Change(trace,findOvariable(2,7,1));
  trace = Biddy_Change(trace,findOvariable(2,5,2));
  trace = Biddy_Change(trace,findOvariable(2,8,3));
  trace = Biddy_Change(trace,findMvariable(1,3,1,3));
  trace = Biddy_Change(trace,findMvariable(1,1,2,1));
  trace = Biddy_Change(trace,findMvariable(1,4,3,1));
  trace = Biddy_Change(trace,findMvariable(2,7,1,1));
  trace = Biddy_Change(trace,findMvariable(2,5,2,1));
  trace = Biddy_Change(trace,findMvariable(2,8,3,3));
  trace = Biddy_Change(trace,findSvariable(1,1,3,1));
  trace = Biddy_Change(trace,findSvariable(1,2,1,1));
  trace = Biddy_Change(trace,findSvariable(1,3,1,1));
  trace = Biddy_Change(trace,findSvariable(2,1,1,1));
  trace = Biddy_Change(trace,findSvariable(2,2,1,1));
  trace = Biddy_Change(trace,findSvariable(2,3,3,1));
  */

  /* it is much faster to disable FULLSTATESPACE, */
  /* but then the program may not be able to construct the Gantt chart */
  /**/
  if (FULLSTATESPACE && createG && !THOROUGHLY) {
    printf("WARNING: FULLSTATESPACE ignored for this stage\n\n");
    FULLSTATESPACE = FALSE;
  }
  /**/

  scheduling = TRUE; /* to stop scheduling after the first solution or in a given step */
  count = 1;
  while (scheduling && (X != Biddy_GetEmptySet())) {

    if (createG) {
      printf("[%u]",count);
      /*
      printf("\n********************* STEP %u *********************\\\\\n",count);
      printf("\nSTEP %u - INITIAL X: ",count);
      STATCOMBINATIONS(X);
      */
    } else {
      /**/
      printf("\n********************* STEP %u *********************\\\\\n",count);
      printf("\nSTEP %u - INITIAL X: ",count);
#ifdef INTERNALREPORTS
      PRINTCOMBINATIONS(X);
#else
      STATCOMBINATIONS(X);
#endif
      /**/
    }

    /* SCHEDULING - APPLY STEP-BY-STEP LIMITS AND BOUNDS */
    X = stepLimitsBounds(X,limit,createG,count-1); /* count-1 because this step has not been executed, yet */

    /*
    printf("\nSTEP %u (AFTER LIMITS): ",count);
    STATCOMBINATIONS(X);
    */

    /* DEBUGGING */
    /*
    if (trace) {
      printf("INITIAL");
#ifdef LATEX
      printf("\\begin{align*} ");
      Biddy_PrintfMinterms(Biddy_Supset(X,trace),FALSE);
      printf("\\end{align*}");
#else
      Biddy_PrintfMinterms(Biddy_Supset(X,trace),FALSE);
#endif
    }
    */

    /* SCHEDULING - TR1 */
    /* first, apply tr1 - M[s,w,r,i]*W[s,r]*MX[i,j]*^B[i,j]*^R[s,r]*S[s,r,i,j]*T[s,n] */
    /* second, restrict for efficiency */
    /* third, find R[s,r], then remove W[s,r] and M[s,w,r,i] (and add G[s,t] for Gantt chart) */
    Y = Biddy_SelectiveProduct(X,tr1,tr1cube);
    /* VARIANT A - IT SEEMS TO BE MORE EFFICIENT */
    X = Biddy_Intersect(X,Biddy_Stretch(Biddy_Union(X,Y)));
    /* VARIANT  B - IT SEEMS TO BE LESS EFFICIENT */
    /* X = Biddy_Diff(X,Biddy_Subset(X,Y)); */
    Biddy_KeepFormula(X);
    Biddy_KeepFormula(Y);
    Biddy_Clean();
    f = Y;
    while (f != Biddy_GetEmptySet())
    {
      f = Biddy_SelectiveProduct(f,tr1,tr1cube);
      Y = Biddy_Union(Y,f);
      Biddy_KeepFormula(X);
      Biddy_KeepFormula(Y);
      Biddy_KeepFormula(f);
      Biddy_Clean();
    }
    /* Biddy_ForceGC(); */ /* minimize nodes */
    /*
    printf("\nSTEP %u (AFTER SELECTIVE PRODUCT): ",count);
    STATCOMBINATIONS(X);
    */
    /* full state space is achieved by disabling Strech operation */
    /* TO DO: using FULLSTATESPACE is to complex for createG for large examples */
    if (!FULLSTATESPACE)
    {
      f = Y;
      Y = Biddy_Stretch(Y);
      /**/
      if (VERBOSE && !createG) {
        k = Biddy_CountMinterms(Biddy_Diff(f,Y),0);
        if (k) {
          printf("\nREMOVED BY STRETCH: %u/%.0f\n",k,Biddy_CountMinterms(f,0));
        }
      }
      /**/
      Biddy_KeepFormula(X);
      Biddy_KeepFormula(Y);
      Biddy_Clean();
      /* Biddy_ForceGC(); */ /* minimize nodes */
    }
    for (i = 1; i <= NUMMACHINES; i++) {
      for (j = 2; j <= machineTable[i].n; j++) {
        v1 = findBvariable(i,j);
        if (v1) {
          v2 = findBvariable(i,j-1);
          f = Y;
          f = Biddy_Subset1(f,v1);
          Y = Biddy_Diff(Y,f);
          f = Biddy_Subset1(f,v2);
          Y = Biddy_Union(Y,f);
          Biddy_KeepFormula(X);
          Biddy_KeepFormula(Y);
          Biddy_Clean();
        }
      }
    }
    /* Biddy_ForceGC(); */ /* minimize nodes */
    for (s = 1; s <= NUMPARTS; s++) {
      r = 1;
      complete = FALSE;
      while (!complete) {
        v1 = findRvariable(s,r);
        if (v1) {
          f = Y;
          f = Biddy_Subset1(f,v1);
          if (f != Biddy_GetEmptySet()) {
            Y = Biddy_Diff(Y,f);
            v2 = findWvariable(s,r);
#ifdef ONEPASS
            if (TRUE)
#else
            if (createG)
#endif
              {
              g = Biddy_Subset1(f,v2);
              if (g != Biddy_GetEmptySet()) {
                f = Biddy_Diff(f,g);
                v = foaGvariable(s,r,count-1); /* -1 because Gantt chart starts with zero */
                g = Biddy_Change(g,v);
                g = Biddy_Change(g,v2);
                f = Biddy_Union(f,g);
              }
            } else {
              f = Biddy_ElementAbstract(f,v2);
            }
            for (w = 1; w <= NUMOPERATIONS; w++) {
              for (i = 1; i <= NUMMACHINES; i++) {
                M = findMvariable(s,w,r,i);
                if (M) {
                  f = Biddy_ElementAbstract(f,M);
                }  
              }
            }
            Y = Biddy_Union(Y,f);
            Biddy_KeepFormula(X);
            Biddy_KeepFormula(Y);
            Biddy_Clean();
          }
        }
        if (r == MAXSEQUENCELENGTH) complete = TRUE;
        r++;
      }
    }
    X = Biddy_Union(X,Y);
    Biddy_KeepFormula(X);
    Biddy_Clean();
    /* Biddy_ForceGC(); */ /* minimize nodes */

    /*
    printf("\nSTEP %u (TR1): ",count);
    STATCOMBINATIONS(X);
    */

#ifdef INTERNALREPORTS
    printf("\nSTEP %u (TR1): ",count);
    PRINTCOMBINATIONS(X);
#endif

    /* if G-variables are added and not THOROUGHLY then keep only one branch for joined branch */
    /* however, then the program may not be able to report all solutions */
    /* TO DO: not very good but seems to be needed for large problems */
    if (createG && !THOROUGHLY) {
      f = X;
      for (v=1; v < Biddy_VariableTableNum(); v++) {
        if ((Biddy_GetVariableName(v))[0] == 'G') {
          /* printf("<%s>",Biddy_GetVariableName(v)); */
          f = Biddy_ElementAbstract(f,v);
        }
      }
      /* printf("NOTE: X includes %.0f cubes, f includes %.0f cubes\n",Biddy_CountMinterms(X,0),Biddy_CountMinterms(f,0)); */
      if (Biddy_CountMinterms(X,0) != Biddy_CountMinterms(f,0)) {
        Y = Biddy_GetEmptySet();
        while (f != Biddy_GetEmptySet()) {
          g = Biddy_ExtractMinterm(NULL,f);
          f = Biddy_Diff(f,g);
          g = Biddy_Supset(X,g);
          /* printf("<%.0f>",Biddy_CountMinterms(g,0)); */
          g = Biddy_ExtractMinterm(NULL,g);
          Y = Biddy_Union(Y,g);
        }
        /* printf("NOTE: Y includes %.0f cubes\n",Biddy_CountMinterms(Y,0)); */
        X = Y;
      }
    }

    /*
    printf("\nSTEP %u (AFTER JOIN): ",count);
    STATCOMBINATIONS(X);
    */

#ifdef INTERNALREPORTS
    printf("\nSTEP %u (AFTER JOIN): ",count);
    PRINTCOMBINATIONS(X);
#endif

    /* SCHEDULING - TR2 */
    /* find T[s,k] then remove it and add T[s,k-1] */
    /*
    k = 1;
    complete = FALSE;
    while (!complete) {
      for (s = 1; s <= NUMPARTS; s++) {
        v1 = findTvariable(s,k);
        if (v1) {
          f = Biddy_Subset1(X,v1);
          if (f != Biddy_GetEmptySet()) complete = TRUE;
        }
      }
      k++;
    }
    printf("FOUND K = %u\n",k-1);
    */
    k = 1;
    complete = FALSE;
    while (!complete) {
      complete = TRUE;
      for (s = 1; s <= NUMPARTS; s++) {
        v1 = findTvariable(s,k);
        if (v1) {
          complete = FALSE;
          v2 = findTvariable(s,k-1);
          f = X;
          f = Biddy_Subset1(f,v1);
          X = Biddy_Diff(X,f);
          f = Biddy_Change(f,v1); /* instead of ElementAbstract because of Subset1 before */
          f = Biddy_Change(f,v2);
          X = Biddy_Union(X,f);
          Biddy_KeepFormula(X);
          Biddy_Clean();
        }
      }
      k++;
    }
    /* Biddy_ForceGC(); */ /* minimize nodes */

    /*
    printf("\nSTEP %u (TR2): ",count);
    STATCOMBINATIONS(X);
    */

#ifdef INTERNALREPORTS
    printf("\nSTEP %u (TR2): ",count);
    PRINTCOMBINATIONS(X);
#endif

    /* SCHEDULING - TR3 */
    /* find T[s,0]*R[s,r]*S[s,r,i,j] */
    /* then remove T[s,0], R[s,r], B[i,j], and add W[s,r+1] */
    for (s = 1; s <= NUMPARTS; s++) {
      v1 = findTvariable(s,0);
      r = 1;
      complete = FALSE;
      while (!complete) {
        g = Biddy_GetEmptySet();
        v2 = findRvariable(s,r);
        if (v2) {
          for (i = 1; i <= NUMMACHINES; i++) {
            for (j = 1; j <= machineTable[i].n; j++) {
              v3 = findSvariable(s,r,i,j);
              if (v3) {
                f = X;
                f = Biddy_Subset1(f,v1);
                f = Biddy_Subset1(f,v2);
                f = Biddy_Subset1(f,v3);
                X = Biddy_Diff(X,f);
                f = Biddy_Change(f,v1); /* instead of ElementAbstract because of Subset1 before */
                f = Biddy_Change(f,v2); /* instead of ElementAbstract because of Subset1 before */
                f = Biddy_ElementAbstract(f,findBvariable(i,j));
                g = Biddy_Union(g,f);
              }
            }
          }
        }
        if (g != Biddy_GetEmptySet()) {
          v3 = findWvariable(s,r+1);
          if (v3) g = Biddy_Change(g,v3);
          X = Biddy_Union(X,g);
          Biddy_KeepFormula(X);
          Biddy_Clean();
        }
        if (r == MAXSEQUENCELENGTH) complete = TRUE;
        r++;
      }
    }
    /* Biddy_ForceGC(); */ /* minimize nodes */

    /*
    printf("\nSTEP %u (TR3): ",count);
    STATCOMBINATIONS(X);
    */

#ifdef INTERNALREPORTS
    printf("\nSTEP %u (TR3): ",count);
    PRINTCOMBINATIONS(X);
#endif

    /* SCHEDULING - CHECK FOR SOLUTIONS - THEY DO NOT HAVE M AND R VARIABLES*/
    /* remove W variables before storing the result into the global table RESULT */
    f = X;
    for (s = 1; s <= NUMPARTS; s++) {
      for (w = 1; w <= NUMOPERATIONS; w++) {
        r = 1;
        complete = FALSE;
        while (!complete) {
          v1 = findRvariable(s,r);
          v2 = findWvariable(s,r+1);
          for (i = 1; i <= NUMMACHINES; i++) {
            M = findMvariable(s,w,r,i);
            if (M) {
              f = Biddy_Subset0(f,M);
              f = Biddy_Subset0(f,v1);
              if (v2) f = Biddy_ElementAbstract(f,v2);
            }
          }
          if (r == MAXSEQUENCELENGTH) complete = TRUE;
          r++;
        }
      }
    }
    if (f != Biddy_GetEmptySet()) Biddy_KeepFormulaUntilPurge(f);

    /* STORE RESULT */
    if (!MINRESULT && (Biddy_CountMinterms(f,0) != 0)) MINRESULT = count;
    RESULTSIZE++;
    if (!RESULT) {
      RESULT = (Biddy_Edge *)  malloc(sizeof(Biddy_Edge));
    } else {
      RESULT = (Biddy_Edge *) realloc(RESULT, RESULTSIZE * sizeof(Biddy_Edge));
    }
    RESULT[RESULTSIZE-1] = f;

    if (createG) {
      /*
      printf("\nNUMBER OF SOLUTIONS WITH LENGTH %u: %.0f\n",count,Biddy_CountMinterms(f,0));
      */
    } else {
      /**/
      printf("\nNUMBER OF SOLUTIONS WITH LENGTH %u: %.0f\n",count,Biddy_CountMinterms(f,0));
      /**/
#ifdef INTERNALREPORTS
      if (Biddy_CountMinterms(f,0)) PRINTCOMBINATIONS(f);
#endif
    }
 
    /* SIFTING ON X */
    /*
    if (count == 19) {
      printf("\nSIFTING ON X\n");
      elapsedtime = clock();
      Biddy_Sifting(X,FALSE);
      Biddy_Sifting(X,TRUE);
      STATCOMBINATIONS(X);
      reportOrdering();
    }
    */

    /* REMOVE SOLUTIONS */
    /* remove all those which do not have M and R variables */
    f = Biddy_Supset(X,f);
    for (s = 1; s <= NUMPARTS; s++) {
      for (w = 1; w <= NUMOPERATIONS; w++) {
        r = 1;
        complete = FALSE;
        while (!complete) {
          v = findRvariable(s,r);
          for (i = 1; i <= NUMMACHINES; i++) {
            M = findMvariable(s,w,r,i);
            if (M) {
              f = Biddy_Subset0(f,M);
              f = Biddy_Subset0(f,v);
            }
          }
          if (r == MAXSEQUENCELENGTH) complete = TRUE;
          r++;
        }
      }
    }
    /**/
    if (!createG) {
      if (Biddy_CountMinterms(f,0)) {
        printf("NUMBER OF REMOVED SOLUTIONS (SHOULD BE THE SAME): %.0f\n",Biddy_CountMinterms(f,0));
        if (FALSE) STATCOMBINATIONS(f);
      }
    }
    /**/
    X = Biddy_Diff(X,f);
    Biddy_KeepFormula(X);
    Biddy_Clean();
    /* Biddy_ForceGC(); */ /* minimize nodes */

    /**/
    if (createG) {
      /*
      printf("\nSTEP %u - FINAL X: ",count);
#ifdef INTERNALREPORTS
      PRINTCOMBINATIONS(X);
#else
      STATCOMBINATIONS(X);
#endif
      */
    } else {
      printf("\nSTEP %u - FINAL X: ",count);
#ifdef INTERNALREPORTS
      PRINTCOMBINATIONS(X);
#else
      STATCOMBINATIONS(X);
#endif
    }
    /**/

    /* FINISH AFTER THE FIRST SOLUTION */
    if (FIRSTSOLUTIONONLY) {
      if (Biddy_CountMinterms(f,0) != 0) {
        scheduling = FALSE;
      }
    }

    if (count == limit) scheduling = FALSE;

    if (scheduling) count++;
  }

  /* SIFTING ON RESULT */
  /*
  printf("\nSIFTING ON X\n");
  STATCOMBINATIONS(X);
  elapsedtime = clock();
  Biddy_Sifting(X,FALSE);
  Biddy_Sifting(X,TRUE);
  STATCOMBINATIONS(X);
  reportOrdering();
  */

  /* SIFTING ON SYSTEM */
  /*
  printf("\nSIFTING ON SYSTEM\n");
  STATCOMBINATIONS(X);
  elapsedtime = clock();
  Biddy_Sifting(NULL,FALSE);
  Biddy_Sifting(NULL,TRUE);
  STATCOMBINATIONS(X);
  reportOrdering();
  */
}

/* ************************************************************************** */
/* GENERATE Rmd FOR GANTT CHART - THE GENERATED FILE IS FOR Rstudio SOFTWARE  */
/* ************************************************************************** */

/* this approach is using vistime library */
Biddy_Boolean
generateGanttVistime(const char filename[], Biddy_Edge solution, unsigned int limit)
{
  return FALSE;
}

/* this approach is using timevis library */
Biddy_Boolean
generateGanttTimevis(const char filename[], Biddy_Edge solution, unsigned int limit, Biddy_Boolean gready, Biddy_Edge restricted)
{
  FILE *F;
  unsigned int i,j;
  unsigned int s,r,w;
  Biddy_Variable O,v;
  Biddy_Boolean first;
  Biddy_Edge f;
  GANTT g;
  GANTTITEM *data;

  if (filename) {
    F = fopen(filename,"w");
  } else {
    F = stdout;
  }
  if (!F) return FALSE;

  if (!solution) {
    fprintf(F,"NULL\n");
    fclose(F);
    return FALSE;
  }

  if (solution == Biddy_GetEmptySet()) {
    fprintf(F,"NO SOLUTION\n");
    fclose(F);
    return FALSE;
  }

  if (solution == Biddy_GetBaseSet()) {
    printf("\nINTERNAL ERROR: generateGanttTimevis got base set\n");
    fprintf(F,"\nINTERNAL ERROR: generateGanttTimevis got base set\n");
    fclose(F);
    return FALSE;
  }

  /* add G variables, global variables for scheduling are modified */
  /* runScheduling will create many minterms even if given just one minterm */
  if (!gready) {
    solution = revertSolution(solution,&restricted);
    /* TO DO: restricted is not correct, and thus it is not used */
    runScheduling(solution,limit,restricted,TRUE); /* TRUE = create G variables */
    solution = Biddy_ExtractMinterm(NULL,RESULT[limit-1]);

    if (solution == Biddy_GetEmptySet()) {
      printf("\nWARNING: not able to generate Gantt chart, please use -t");
      fprintf(F,"\nWARNING: not able to generate Gantt chart, please use -t");
      fclose(F);
      return FALSE;
    }
  }

  /* make sure you have only one solution */
  if (Biddy_CountMinterms(solution,0) != 1) {
    printf("\nINTERNAL ERROR: generateGanttTimevis got solution with multiple minterms\n");
    fprintf(F,"\nINTERNAL ERROR: generateGanttTimevis got solution with multiple minterms\n");
    fclose(F);
    return FALSE;
  }

  /*
  printf("\n\nSOLUTION:\n");
  PRINTCOMBINATIONS(solution);
  */

  g.n = 0;
  addGanttItem(&g,0,NULL,NULL,0,0,0,0);

  f = solution;
  while (!Biddy_IsTerminal(f)) {
    if (Biddy_GetTopVariableChar(f) == 'G') {
      v = Biddy_GetTopVariable(f);
      data = (GANTTITEM *) Biddy_GetVariableData(v);
      s = data->part;
      r = data->r;
      for (w = 1; w <= NUMOPERATIONS; w++) {
        O = findOvariable(s,w,r);
        /* the following condition is OK because we have only one minterm */
        if (O && (Biddy_Subset1(solution,O) != Biddy_GetEmptySet())) {
          for (i = 1; i <= NUMMACHINES; i++) {
            for (j = 1; j <= machineTable[i].n; j++) {
              v = findSvariable(s,r,i,j);
              /* the following condition is OK because we have only one minterm */
              if (v && (Biddy_Subset1(solution,v) != Biddy_GetEmptySet())) {
                addGanttItem(&g,s,(*(productionTable[s])).name,
                      operationTable[w].name,
                      findMXvariable(i,j),
                      data->t,
                      data->t+operationMatrix[w+i*(1+operationTableSize)],
                      (*(productionTable[s])).color);
              }
            }
          }
        }
      }
    }
    f = Biddy_GetThen(f);
  }

  /* DEBUGGING */
  /*
  for (i = g.n-1; g.n && (i > 0); i--) {
    printf("[[%s,%u,%u,%u,%u]]\n",g.content[i],g.id[i],g.group[i],g.start[i],g.end[i]);
  }
  */

  /* starting part */
  fprintf(F,"---\n");
  fprintf(F,"title: \"The Production Planning example\"\n");
  fprintf(F,"author: \"R. Meolic, Z. Brezocnik\"\n");
  /* fprintf(F,"date: '`r format(Sys.Date(), \"%%d\\\\. %%m\\\\. %%Y\")`'\n"); */
  fprintf(F,"---\n");
  fprintf(F,"\n");
  fprintf(F,"Calculated with Biddy (biddy.meolic.com).\n");
  fprintf(F,"\n");
  fprintf(F,"```{r, echo=FALSE}\n");
  fprintf(F,"library(timevis)\n");

  /* here is the diagram */
  fprintf(F,"groups = data.frame(\n");
  fprintf(F,"  content = c(");
  first = TRUE;
  for (i = 1; i <= NUMMACHINES; i++) {
    for (j = 1; j <= machineTable[i].n; j++) {
      v = findMXvariable(i,j);
      if (v && (Biddy_Subset1(solution,v) != Biddy_GetEmptySet())) {
        if (first) first = FALSE; else fprintf(F,",");
        fprintf(F,"\"%s#%u\"",machineTable[i].name,j);
      }
    }
  }
  fprintf(F,"),\n");
  fprintf(F,"  id      = c(");
  first = TRUE;
  for (i = 1; i <= NUMMACHINES; i++) {
    for (j = 1; j <= machineTable[i].n; j++) {
      v = findMXvariable(i,j);
      if (v && (Biddy_Subset1(solution,v) != Biddy_GetEmptySet())) {
        if (first) first = FALSE; else fprintf(F,",");
        fprintf(F,"\"%s\"",Biddy_GetVariableName(v));
      }
    }
  }
  fprintf(F,")\n");
  fprintf(F,")\n");
  fprintf(F,"data <- data.frame(\n");
  fprintf(F,"  content = c(");
  for (i = g.n-1; g.n && (i > 0); i--) {
    fprintf(F,"\"%s\"",g.content[i]);
    if (i > 1) fprintf(F,",");
  }
  fprintf(F,"),\n");
  fprintf(F,"  id      = c(");
  for (i=1; i < g.n; i++) {
    fprintf(F,"%u",i);
    if (i < (g.n-1)) fprintf(F,",");
  }
  fprintf(F,"),\n");
  fprintf(F,"  group   = c(");
  for (i = g.n-1; g.n && (i > 0); i--) {
    fprintf(F,"\"%s\"",g.group[i]);
    if (i > 1) fprintf(F,",");
  }
  fprintf(F,"),\n");
  fprintf(F,"  start   = c(");
  for (i = g.n-1; g.n && (i > 0); i--) {
    fprintf(F,"%u",g.start[i]);
    if (i > 1) fprintf(F,",");
  }
  fprintf(F,"),\n");
  fprintf(F,"  end     = c(");
  for (i = g.n-1; g.n && (i > 0); i--) {
    fprintf(F,"%u",g.end[i]);
    if (i > 1) fprintf(F,",");
  }
  fprintf(F,"),\n");
  fprintf(F,"  style   = c(");
  for (i = g.n-1; g.n && (i > 0); i--) {
    fprintf(F,"%s",g.color[i]);
    if (i > 1) fprintf(F,",");
  }
  fprintf(F,")\n");
  fprintf(F,")\n");

  /* final part */
  fprintf(F,"timevis(data,groups,showZoom=FALSE,width=\"100%%\",options=list(margin=0,stack=FALSE))\n");
  fprintf(F,"```\n");
  if (filename) {
    fclose(F);
  }

  return TRUE;
}


/* ************************************************************************** */
/* GENERATE THE SYSTEM FROM CSV FILE */
/* ************************************************************************** */

void
trim(Biddy_String source)
{
  unsigned int sr_length;

  sr_length = (unsigned int) strlen(source);
  while (sr_length && ((source[sr_length-1] == ' ') ||
                       (source[sr_length-1] == '\t') ||
                       (source[sr_length-1] == 10) ||
                       (source[sr_length-1] == 13)))
  {
    source[sr_length-1] = 0;
    sr_length = (unsigned int) strlen(source);
  }
}

void
generateCSVSystem(Biddy_String csvname, Biddy_String dataname)
{
  FILE *csvfile;
  char buffer[255];
  char delchar[2];
  Biddy_String word;
  unsigned int wordnum;
  Biddy_Boolean OK,DONE;
  unsigned int N;
  unsigned int part,pnum,xnum,onum;
  unsigned int i,j;
  unsigned int w,r;

  if (!csvname || !dataname) {
    printf("PLEASE, SPECIFY THE CSV NAME AND THE PROBLEM NAME\n");
    exit(1);
  }

  delchar[0] = delchar[1] = 0;
  j = 0;
  pnum = 0;
  N = 0;

  csvfile = fopen(csvname,"r");
  if (!csvfile) {
    printf("FILE %s DOES NOT EXIST\n",csvname);
    exit(1);
  } else {
    OK = FALSE;
    while (fgets(buffer,sizeof(buffer),csvfile) != NULL) {
      trim(buffer); /* to assure Win/Unix EOL independecy */
      if (buffer[0] == '#') continue;
      if (!strncmp(buffer,"NAME",4)) {
        i = 4;
        while (buffer[i] == ' ') i++;
        delchar[0] = buffer[i];
      }
      wordnum = 0;
      for (i=0; i<strlen(buffer); i++) {
        if ((buffer[i] == delchar[0]) && buffer[i+1] && (buffer[i+1] != delchar[0])) wordnum++;
      }
      word = strtok(buffer,delchar);
      if (word && !strlen(word)) {
        while ((word=strtok(NULL,delchar))&&!strlen(word));
      }
      if (word && !strcmp(word,"NAME")) {
        word = strtok(NULL,delchar);
        if (!strcmp(word,dataname)) {
          OK = TRUE;
          N = 1;
        } else {
          OK = FALSE;
        }
      } else {
        if (OK && word) {
          DONE = FALSE;
          switch (N) {
            case 1: /* <NUMPARTS><x> */
              if (strcmp(word,"NUMPARTS") && strcmp(word,"NUMJOBS")) {
                printf("FILE %s IS NOT CORRECT\n",csvname);
                exit(1);
              } else {
                while ((word=strtok(NULL,delchar))&&!strlen(word));
                sscanf(word,"%u",&NUMPARTS);
                /* printf("NUMPARTS = %u\n",NUMPARTS); */
              }
              DONE = TRUE;
              break;
            case 2: /* <operationTable>... */
              if (strcmp(word,"operationTable")) {
                printf("FILE %s IS NOT CORRECT\n",csvname);
                exit(1);
              } else {
                /* printf("<%s>",word); */
                /* --- */
                operationTableSize = wordnum;
                operationTable = (OPERATION *) malloc((1 + operationTableSize) * sizeof(OPERATION));
                (operationTable)[0].name = NULL;
                for (i = 1; i <= operationTableSize; i++)  (operationTable)[i].idx = i;
                /* --- */
                while ((word=strtok(NULL,delchar))&&!strlen(word));
                j = 0;
                while (word) {
                  j++;
                  /* printf("<%s>",word); */
                  /* --- */
                  (operationTable)[j].name = strdup(word);
                  /* --- */
                  while ((word=strtok(NULL,delchar))&&!strlen(word));
                }
                /* printf("\n"); */
              }
              DONE = TRUE;
              break;
            case 3: /* <machineTable>... */
              if (strcmp(word,"machineTable")) {
                printf("FILE %s IS NOT CORRECT\n",csvname);
                exit(1);
              } else {
                /* printf("<%s>",word); */
                /* --- */
                NUMMACHINES = machineTableSize = wordnum;
                machineTable = (MACHINE *) malloc((1 + machineTableSize) * sizeof(MACHINE));
                machineTable[0].name = NULL;
                for (i = 1; i <= machineTableSize; i++) machineTable[i].idx = i;
                /* --- */
                while ((word=strtok(NULL,delchar))&&!strlen(word));
                j = 0;
                while (word) {
                  j++;
                  /* printf("<%s>",word); */
                  /* --- */
                  machineTable[j].name = strdup(word); machineTable[j].n = 1;
                  /* --- */
                  while ((word=strtok(NULL,delchar))&&!strlen(word));
                }
                /* printf("\n"); */
              }
              DONE = TRUE;
              break;
            case 4: /* <operationMatrix> */
              if (strcmp(word,"operationMatrix")) {
                printf("FILE %s IS NOT CORRECT\n",csvname);
                exit(1);
              }
              /* --- */
              /* printf("<%s>\n",word); */
              operationMatrix = (unsigned int *)
                malloc((1 + machineTableSize) * (1 + operationTableSize) * sizeof(unsigned int));
              for (j = 0; j <= operationTableSize; j++) {
                for (i = 0; i <= machineTableSize; i++) {
                  (operationMatrix)[j + i * (1 + operationTableSize)] = 0; /* Oj @ Mi */
                }
              }
              /* --- */
              j = 0;
              DONE = TRUE;
              break;
            case 5: /* ... */
              j++;
              i = 0;
              while ((word=strtok(NULL,delchar))&&!strlen(word)); /* first column is ignored */
              while (word) {
                i++;
                if (i > NUMMACHINES) {
                  printf("FILE %s IS NOT CORRECT\n",csvname);
                  exit(1);
                }
                sscanf(word,"%u",&r);
                /* printf("<%u>",r); */
                /* --- */
                operationMatrix[j + i * (1 + operationTableSize)] = r; /* Oj @ Mi */
                /* --- */
                while ((word=strtok(NULL,delchar))&&!strlen(word));
              }
              /* printf("\n"); */
              if (j == operationTableSize) DONE = TRUE;
              break;
            case 6: /* <partTable> */
              if (strcmp(word,"partTable") && strcmp(word,"jobTable")) {
                printf("FILE %s IS NOT CORRECT\n",csvname);
                exit(1);
              }
              /* printf("<%s>\n",word); */
              /* --- */
              MAXSEQUENCELENGTH = 0;
              pnum = 0;
              productionTableSize = partTableSize = NUMPARTS;
              productionTable = (PPART *) malloc((1 + productionTableSize) * sizeof(PPART));
              productionTable[0] = NULL;
              partTable = (PART *) malloc((1 + partTableSize) * sizeof(PART));
              partTable[0].name = NULL; partTable[0].n = 0; partTable[0].X = NULL;
              for (i = 1; i <= partTableSize; i++)  {
                partTable[i].idx = i;
                partTable[i].color = 0;
              }
              /* --- */
              j = 0;
              DONE = TRUE;
              break;
            case 7: /* ... */
              j++;
              part = j;
              /* printf("<%s>",word); */
              /* --- */
              pnum++; xnum = 0; onum = 0;
              productionTable[part] = &(partTable[pnum]);
              partTable[pnum].name = strdup(word);
              partTable[pnum].part = part;
              partTable[pnum].n = 1; /* number of sequences */
              partTable[pnum].code = NULL; partTable[pnum].feasible = NULL;
              partTable[pnum].X = (SEQUENCE *) malloc((1 + partTable[pnum].n) * sizeof(SEQUENCE));
              partTable[pnum].X[0].n = 0; partTable[pnum].X[0].O = NULL;
              partTable[pnum].color = PPCOLOR[part];
              xnum++; onum = 0; /* next sequence */
              if (MAXSEQUENCELENGTH < wordnum) MAXSEQUENCELENGTH = wordnum;
              partTable[pnum].X[xnum].n = wordnum; /* number of items in the sequence */
              partTable[pnum].X[xnum].O = (SEQUENCEITEM *) malloc((1 + partTable[pnum].X[xnum].n) * sizeof(SEQUENCEITEM));
              partTable[pnum].X[xnum].O[0].type = NONE; partTable[pnum].X[xnum].O[0].ITEM = NULL;
              /* --- */
              while ((word=strtok(NULL,delchar))&&!strlen(word));
              r = 0;
              while (word) {
                r++;
                sscanf(word,"%u",&w);
                /* printf("<%u>",w); */
                /* --- */
                partTable[pnum].X[xnum].O[++onum].type = DEFAULT; partTable[pnum].X[xnum].O[onum].ITEM = (void *) &(operationTable[w]);
                ADDVARIABLE(w,r);
                /* --- */
                while ((word=strtok(NULL,delchar))&&!strlen(word));
              }
              /* printf("\n"); */
              if (j == partTableSize) DONE = TRUE;
              break;
            case 8: /* ERROR */
              printf("FILE %s IS NOT CORRECT\n",csvname);
              exit(1);
              DONE = TRUE;
              break;
          }
          if (DONE) N++;
        }
      }
    }
    if (feof(csvfile)) {
      printf("File %s OK.\n\n",csvname);
      fclose(csvfile);
    } else {
      printf("FILE %s IS NOT CORRECT\n",csvname);
      exit(1);
    }
  }
}

/* ************************************************************************** */
/* MAIN PROGRAM */
/* ************************************************************************** */

/* choose data set */
/* CHECKOPERATIONS: debugging, only */
/* NOHARDCODEDDATA: do not load hardcoded data from biddy-example-pp-data.c */

#ifdef CHECKOPERATIONS
#include "biddy-example-pp-checkoperations.c"
#else
#ifdef NOHARDCODEDDATA
void generateSystemTakahashi(){}
void generateSystemKacem4x5(){}
void generateSystemKacem8x8(){}
void generateSystemKacem10x7(){}
void generateSystemKacem10x10(){}
void generateSystemKacem15x10(){}
#else
#include "biddy-example-pp-data.c"
#endif
#endif

int main(int argc, char **argv) {
  clock_t elapsedtime;
  /* Biddy_String activeordering; */
  Biddy_String ganttname;
  unsigned int s,w,r;
  unsigned int i,j,k,l;
  Biddy_Edge X,Z,Q,Y,FC;
  Biddy_Variable O,M,v;
  Biddy_Boolean complete;
  Biddy_Edge f,g,restricted;
  Biddy_Edge S1,S2;
  unsigned int S;

  setbuf(stdout,NULL);

  /* CHECK OPERATIONS ON COMBINATION SETS */
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
      printf("You can select one of the hardcoded systems:\n");
      printf("-data takahashi (DEFAULT)\n");
      printf("-data kacem4x5\n");
      printf("-data kacem8x8\n");
      printf("-data kacem10x7\n");
      printf("-data kacem10x10\n");
      printf("-data kacem15x10\n");
      printf("\n");
      printf("To use the csv file instead of the hardcoded data:\n");
      printf("-csv file_name -data problem_name\n");
      printf("\n");
      printf("You can choose a subsystem by defining the following arguments:\n");
      printf("[ -processes    | -p ] num_processes\n");
      printf("[ -machines     | -m ] num_machines\n");
      printf("[ -capacity     | -c ] factory_capacity\n");
      printf("\n");
      printf("You can adjust scheduling with the following parameters:\n");
      printf("[ -feasibleonly | -0 ]\n");
      printf("[ -firstonly    | -1 ]\n");
      printf("[ -maxonly      | -x ]\n");
      printf("[ -full         | -f ]\n");
      printf("[ -bound        | -b ] machine_time_bound\n");
      printf("[ -limit        | -l ] makespan_limit\n");
      printf("\n");
      printf("Here are the advanced parameters:\n");
      printf("[ -BOUND        | -B ]\n");
      printf("[ -BOUND+       | -B+ ] machine_time_interval\n");
      printf("[ -LIMIT        | -L ]\n");
      printf("[ -LIMIT+       | -L+ ] makespan_interval\n");
      printf("\n");
      printf("You can adjust the amount of generated results with the following parameters:\n");
      printf("[ -thoroughly   | -t ]\n");
      printf("[ -verbose      | -v ]\n");
      printf("\n");
      printf("Example ([Takahashi2014]): biddy-example-pp.exe -p 8 -m 8 -c 8\n");
      printf("Example ([Takahashi2015]): biddy-example-pp.exe -p 4 -m 5 -c 4\n");
      exit(0);
    }

    if (!strcmp(argv[i],"-csv")) {
      i++;
      CSVNAME = strdup(argv[i]);
    }

    if (!strcmp(argv[i],"-data")) {
      i++;
      DATANAME = strdup(argv[i]);
      if (!strcmp(argv[i],"takahashi")) DATASET = 1;
      else if (!strcmp(argv[i],"kacem4x5")) DATASET = 2;
      else if (!strcmp(argv[i],"kacem8x8")) DATASET = 3;
      else if (!strcmp(argv[i],"kacem10x7")) DATASET = 4;
      else if (!strcmp(argv[i],"kacem10x10")) DATASET = 5;
      else if (!strcmp(argv[i],"kacem15x10")) DATASET = 6;
      else DATASET = 0;
    }

    if (!CSVNAME && !DATASET) {
      printf("ERROR: UNKNOWN DATASET\n");
      exit(1);
    }

    if (!strncmp(argv[i],"-p",2) || !strcmp(argv[i],"-process")) {
      sscanf(&(argv[i][2]),"%u",&NUMPARTS);
      if (!NUMPARTS) {
        sscanf(argv[++i],"%u",&NUMPARTS);
      }
#ifdef LIMITSINGLE
    if (NUMPARTS < LIMITSINGLE) NUMPARTS = LIMITSINGLE;
#endif
#ifdef LIMITDOUBLE
      if (NUMPARTS < LIMITDOUBLE) NUMPARTS = LIMITDOUBLE;
#endif
    }

    if ((!strncmp(argv[i],"-m",2) && strcmp(argv[i],"-maxonly")) || !strcmp(argv[i],"-machines")) {
      sscanf(&(argv[i][2]),"%u",&NUMMACHINES);
      if (!NUMMACHINES) {
        sscanf(argv[++i],"%u",&NUMMACHINES);
      }
    }

    if ((!strncmp(argv[i],"-c",2) && strcmp(argv[i],"-csv")) || !strcmp(argv[i],"-capacity"))  {
      sscanf(&(argv[i][2]),"%u",&FACTORYCAPACITY);
      if (!FACTORYCAPACITY) {
        sscanf(argv[++i],"%u",&FACTORYCAPACITY);
      }
    }

    if (!strcmp(argv[i],"-0") || !strcmp(argv[i],"-feasibleonly")) {
      FEASIBLEONLY = TRUE;
    }

    if (!strcmp(argv[i],"-1") || !strcmp(argv[i],"-firstonly")) {
      FIRSTSOLUTIONONLY = TRUE;
    }

    if (!strcmp(argv[i],"-x") || !strcmp(argv[i],"-maxonly")) {
      MAXFACTORYONLY = TRUE;
    }

    if (!strcmp(argv[i],"-f") || !strcmp(argv[i],"-full")) {
      FULLSTATESPACE = TRUE;
    }

    if (!strncmp(argv[i],"-b",2) || !strcmp(argv[i],"-bound")) {
      sscanf(&(argv[i][2]),"%u",&TIMEBOUND);
      if (!TIMEBOUND) {
        sscanf(argv[++i],"%u",&TIMEBOUND);
      }
    }

    if (!strncmp(argv[i],"-l",2) || !strcmp(argv[i],"-limit")) {
      sscanf(&(argv[i][2]),"%u",&MAKESPANLIMIT);
      if (!MAKESPANLIMIT) {
        sscanf(argv[++i],"%u",&MAKESPANLIMIT);
      }
    }

    if (!strcmp(argv[i],"-t") || !strcmp(argv[i],"-thoroughly")) {
      THOROUGHLY = TRUE;
    }

    if (!strcmp(argv[i],"-v") || !strcmp(argv[i],"-verbose")) {
      VERBOSE = TRUE;
    }

    if (!strcmp(argv[i],"-B") || !strcmp(argv[i],"-BOUND")) {
      AUTOBOUND = TRUE;
    }

    if (!strncmp(argv[i],"-B+",3) || !strcmp(argv[i],"-BOUND+")) {
      AUTOBOUND = TRUE;
      sscanf(&(argv[i][3]),"%u",&BOUNDINTERVAL);
      if (!BOUNDINTERVAL) {
        sscanf(argv[++i],"%u",&BOUNDINTERVAL);
      }
    }

    if (!strcmp(argv[i],"-L") || !strcmp(argv[i],"-LIMIT")) {
      DYNAMICLIMIT = TRUE;
    }

    if (!strncmp(argv[i],"-L+",3) || !strcmp(argv[i],"-LIMIT+")) {
      DYNAMICLIMIT = TRUE;
      sscanf(&(argv[i][3]),"%u",&LIMITINTERVAL);
      if (!LIMITINTERVAL) {
        sscanf(argv[++i],"%u",&LIMITINTERVAL);
      }
    }

    i++;
  }

#ifdef ONEPASS
  THOROUGHLY = TRUE;
#endif

#ifdef INTERNALREPORTS
  VERBOSE = TRUE;
#endif

  if (THOROUGHLY) DYNAMICLIMIT = FALSE;

  /* GENERATE SYSTEM */
  if (CSVNAME) {
    generateCSVSystem(CSVNAME,DATANAME);
  } else {
#ifdef NOHARDCODEDDATA
    printf("PLEASE, SPECIFY THE CSV NAME AND THE PROBLEM NAME\n");
    exit(1);
#endif
    if (DATASET == 1) generateSystemTakahashi();
    else if (DATASET == 2) generateSystemKacem4x5();
    else if (DATASET == 3) generateSystemKacem8x8();
    else if (DATASET == 4) generateSystemKacem10x7();
    else if (DATASET == 5) generateSystemKacem10x10();
    else if (DATASET == 6) generateSystemKacem15x10();
    else {
      printf("ERROR: UNKNOWN DATASET\n");
      exit(1);
    }
  }

  if (NUMPARTS > MAXPARTS) {
    printf("ERROR: NUMPARTS: %u\n",NUMPARTS);
    printf("ERROR: MAXPARTS: %u\n",MAXPARTS);
    exit(1);
  }

  if (!FACTORYCAPACITY) {
    FACTORYCAPACITY = NUMMACHINES;
  }

  /* REPORT PARAMETERS */
  if (CSVNAME) {
    printf("DATASET: %s\n",DATANAME);
  } else {
    printf("DATASET: ");
    if (DATASET == 1) printf("takahashi\n");
    if (DATASET == 2) printf("kacem4x5\n");
    if (DATASET == 3) printf("kacem8x8\n");
    if (DATASET == 4) printf("kacem10x7\n");
    if (DATASET == 5) printf("kacem10x10\n");
    if (DATASET == 6) printf("kacem15x10\n");
  }

  /**/
  printf("INTERNALREPORTS: ");
#ifdef INTERNALREPORTS
  printf("TRUE\n");
#else
  printf("FALSE\n");
#endif
  printf("ONEPASS: ");
#ifdef ONEPASS
  printf("TRUE\n");
#else
  printf("FALSE\n");
#endif
  printf("FINETUNING: ");
#ifdef FINETUNING
  printf("TRUE\n");
#else
  printf("FALSE\n");
#endif
  /**/

  /**/
  printf("NUMPARTS: %u\n",NUMPARTS);
#ifdef LIMITSINGLE
  printf("LIMITSINGLE: %u\n",LIMITSINGLE);
#endif
#ifdef LIMITDOUBLE
  printf("LIMITDOUBLE: %u\n",LIMITDOUBLE);
#endif
  printf("NUMOPERATIONS: %u\n",NUMOPERATIONS);
  printf("NUMMACHINES: %u\n",NUMMACHINES);
  printf("FACTORYCAPACITY: %u\n",FACTORYCAPACITY);
  printf("FEASIBLEONLY: ");
  if (FEASIBLEONLY) printf("TRUE\n"); else printf("FALSE\n");
  printf("FIRSTSOLUTIONONLY: ");
  if (FIRSTSOLUTIONONLY) printf("TRUE\n"); else printf("FALSE\n");
  printf("MAXFACTORYONLY: ");
  if (MAXFACTORYONLY) printf("TRUE\n"); else printf("FALSE\n");
  printf("FULLSTATESPACE: ");
  if (FULLSTATESPACE) printf("TRUE\n"); else printf("FALSE\n");
  printf("TIMEBOUND: %u\n",TIMEBOUND);
  printf("MAKESPANLIMIT: %u\n",MAKESPANLIMIT);
  printf("THOROUGHLY: ");
  if (THOROUGHLY) printf("TRUE\n"); else printf("FALSE\n");
  printf("VERBOSE: ");
  if (VERBOSE) printf("TRUE\n"); else printf("FALSE\n");
  /**/

  /**/
  printf("AUTOBOUND: ");
  if (AUTOBOUND) {if (BOUNDINTERVAL) printf("%u\n",BOUNDINTERVAL); else printf("TRUE\n");} else printf("FALSE\n");
  printf("DYNAMICLIMIT: ");
  if (DYNAMICLIMIT) {if (LIMITINTERVAL) printf("%u\n",LIMITINTERVAL); else printf("TRUE\n");} else printf("FALSE\n");
  /**/

  /* REPORT SYSTEM */
  /* reportSystem(); */

  /* N variables are used for scheduling */
  for (i=1; i<=FACTORYCAPACITY; i++) findoraddFSvariable(i,TRUE,FALSE);\

  printf("\nCALCULATION START\n\n");

  /* CALCULATION */

  Biddy_InitAnonymous(BDDTYPE);

  printf("Using %s...\n\n",Biddy_GetManagerName());

  /* permitMakespan cache */
  permitMakespanCache.size = permitMakespanCacheSize;
  permitMakespanCache.table = (PermitMakespanCache *) malloc((permitMakespanCache.size+1) * sizeof(PermitMakespanCache));
  for (i=0;i<=permitMakespanCache.size;i++) {
    permitMakespanCache.table[i].r = NULL;
  }
  Biddy_AddCache((Biddy_GCFunction) permitMakespanGarbage);

  /* permitMachineTime cache */
  permitMachineTimeCache.size = permitMachineTimeCacheSize;
  permitMachineTimeCache.table = (PermitMachineTimeCache *) malloc((permitMachineTimeCache.size+1) * sizeof(PermitMachineTimeCache));
  for (i=0;i<=permitMachineTimeCache.size;i++) {
    permitMachineTimeCache.table[i].r = NULL;
  }
  Biddy_AddCache((Biddy_GCFunction) permitMachineTimeGarbage);

  /* CREATE ALL VARIABLES BEFORE THE REAL CALCULATION START */
  /* THIS IS REQUIRED FOR OBDDs AND TZBDDs, IT IS OPTIONAL FOR ZBDDs */
  /* (MANIPULATING SETS WITH OBDDs AND TZBDDs IS NOT FULLY SUPPORTED IN BIDDY, YET) */
  /* THIS IS REQUIRED IF CUSTOM VARIABLE ORDERING IS NEEDED */

  /* createVariables() uses variableTable created by generateSystem() */
  createVariables();

  /* APPLY ALPHABETIC ORDERING ORDERING */
  /* this option is not needed if createVariables() creates all variables (only G-variables are not important) */
  /* if Biddy_SetAlphabeticOrdering() is used you have to call createVariables() again to refresh variables weigths */
  /*
  Biddy_ClearVariablesData();
  Biddy_SetAlphabeticOrdering();
  createVariables();
  */
  
  /* APPLY USER VARIABLE ORDERING */
  /* if Biddy_SetOrdering() is used you have to call createVariables() again to refresh variables weigths */
  /*
  activeordering = strdup("\"NULL\"");
  concat(&activeordering,"");
  concat(&activeordering,"");
  concat(&activeordering,"");
  adaptOrdering(&activeordering);
  Biddy_ClearVariablesData();
  Biddy_SetOrdering(activeordering);
  free(activeordering);
  createVariables();
  */

  /*
  reportOrdering();
  */

  /* ENCODE SYSTEM */

  for (s = 1; s <= NUMPARTS; s++) {
#ifdef LIMITSINGLE
#ifdef LIMITDOUBLE
    if ((s != LIMITSINGLE) && (s != LIMITDOUBLE)) continue; /* limit to two processes */
#else
    if (s != LIMITSINGLE) continue; /* limit to a single process */
#endif
#endif
    encodePart(productionTable[s],0,1);
    Biddy_KeepFormulaUntilPurge((*(productionTable[s])).code);
    Biddy_Clean();
  }
  
  /* PRODUCTION PLANNING */

  X = feasibleCombinations();

  if (FEASIBLEONLY) {
    /*
    reportOrdering();
    */
    deleteSystem();
    Biddy_Exit();
    return 0;
  }

  /* ADD INFORMATION ABOUT FACTORY SIZE (I.E. NUMBER OF MACHINE INSTANCES) */
  /* FC = FACTORY CONFIGURATIONS (I.E. THE SETS OF MACHINE INSTANCES) */
  FC = X;
  elapsedtime = clock();
  for (s = 1; s <= NUMPARTS; s++) {
    for (w = 1; w <= NUMOPERATIONS; w++) {
      r = 1;
      complete = FALSE;
      while (!complete) {
        O = findOvariable(s,w,r);
        if (O) {
          FC = Biddy_ElementAbstract(FC,O);
        }
        for (i = 1; i <= NUMMACHINES; i++) {
          M = findMvariable(s,w,r,i);
          if (M) {
            FC = Biddy_ElementAbstract(FC,M);
          }
        }
        if (r == MAXSEQUENCELENGTH) complete = TRUE;
        r++;
      }
    }
  }
  Y = Biddy_GetEmptySet();
  for (i=FACTORYCAPACITY; i>0; i--) {
    f = Biddy_Diff(Biddy_Permitsym(FC,i),Biddy_Permitsym(FC,i-1));
    g = Biddy_Supset(X,f);
    Y = Biddy_Union(Y,Biddy_Change(g,findFSvariable(i)));
    X = Biddy_Diff(X,g);
  }
  X = Y;

  /**/
  printf("\nDIFFERENT FACTORY CONFIGURATIONS: ");
  STATCOMBINATIONS(FC);
  /**/

  Biddy_AddFormula("X",X,2);
  Biddy_Clean();
  Biddy_Purge();

  /* SCHEDULING */
  /* results are stored into global variables */

  runScheduling(X,MAKESPANLIMIT,NULL,FALSE);

  /* REPORT FINAL RESULTS */

  if (MINRESULT > 0) {
    printf("\n********************* RESULTS AFTER %u steps*********************\\\\\n\n",RESULTSIZE);
  } else {
    printf("\n************************** NO SOLUTION **************************\\\\\n\n");
  }

  printf("clock() TIME = %.2f\n",(clock()-elapsedtime)/(1.0*CLOCKS_PER_SEC));

  /* TO DO: restricted is calculated but not correct, and thus it will not be used */
  S = 0;
  restricted = NULL;
  if (MINRESULT > 0) {
    X = NULL; /* RESULT[MINRESULT-1] will be stored in X */
    Y = NULL; /* reverted RESULT[MINRESULT-1] will be created in Y */
    restricted = NULL; /* restricted for RESULT[MINRESULT-1] will be created */
    S = MINRESULT;
    for (i=0; i<RESULTSIZE; i++) {
      if (Biddy_CountMinterms(RESULT[i],0) != 0) {
        printf("\n%.0f ARRANGEMENTS FOUND WITH MAKESPAN = %u",Biddy_CountMinterms(RESULT[i],0),i+1);
        if (TIMEBOUND) printf(" AND MAX TOTAL MACHINE TIME = %u",TIMEBOUND);
        printf("\n");
        Z = restrictToMinFS(RESULT[i],&j);
        printf("MIN NUMBER OF MACHINES = %u, ",j);
        if (restricted) {
          f = NULL;
        } else {
          f = Biddy_GetBaseSet();
        }
        Z = revertSolution(RESULT[i],&f);
        if (!restricted) {
          /* IF NOT FIRSTSOLUTIONONLY AND MAKESPANLIMIT THEN THE FEASIBLE SOLUTIONS WITH THE GIVEN MAKESPANLIMIT ARE CHOOSEN */
          /* OTHERWISE, THE FEASIBLE SOLUTIONS WITH THE MINIMAL MAKESPANLIMIT ARE CHOOSEN */
          if (FIRSTSOLUTIONONLY || !MAKESPANLIMIT || (i == MAKESPANLIMIT-1)) {
            S = i+1;
            X = RESULT[i];
            Y = Z;
            restricted = f;
            Biddy_KeepFormulaUntilPurge(Y);
            Biddy_KeepFormulaUntilPurge(restricted);
          }
        }
        Z = restrictToMinMachineTime(Z,&k);
        printf("MIN TOTAL MACHINE TIME = %u\n",k);
      }
    }
  }

  /* IF TIMEBOUND IS GIVEN THEN RESULT IS RESTRICTED EXACTLY TO IT */
  /* X is not adapted, because it is not used in further calculations! */
  if (TIMEBOUND) {
    Y = Biddy_Diff(Y,permitMachineTime(Y,TIMEBOUND-1));
    Biddy_KeepFormulaUntilPurge(Y);
  }

  if (Y == Biddy_GetEmptySet()) {
    printf("\nNO SOLUTIONS WITH MAKESPAN = %u AND TOTAL MACHINE TIME = %u\n",S,TIMEBOUND);
  }

#ifdef ONEPASS
  /* do nothing here, different schedules are already counted */
#else
  if (THOROUGHLY && (Y != Biddy_GetEmptySet()) && (MINRESULT > 0)) {
    printf("\nCOUNTING ALL SCHEDULES WITH MAKESPAN = %u",S);
    if (MAXFACTORYONLY) printf(", FACTORY SIZE = %u",FACTORYCAPACITY);
    if (TIMEBOUND) printf(", TOTAL MACHINE TIME = %u",TIMEBOUND);
    printf("\n");
    elapsedtime = clock();
    /* MINRESULT will be recalculated */
    /* TO DO: restricted is not correct, and thus it is not used */
    runScheduling(Y,S,restricted,TRUE); /* TRUE = create G variables */
    printf("\nclock() TIME = %.2f\n",(clock()-elapsedtime)/(1.0*CLOCKS_PER_SEC));
    printf("\nNUMBER OF SCHEDULES FOUND: %.0f\n",Biddy_CountMinterms(RESULT[S-1],0));
  }
#endif

  /* FIND THE FEASIBLE SOLUTIONS WITH MINIMAL NUMBER OF MACHINES AND MINIMAL TOTAL MACHINE TIME */
  S1 = NULL;
  S2 = NULL;
  if ((Y != Biddy_GetEmptySet()) && (MINRESULT > 0)) {
    Z = restrictToMinFS(Y,&k); /* k is minimal number of machines of the solutions with the choosen makespan */
    if (TIMEBOUND) {
      j = TIMEBOUND;
      Q = Y;
    } else {
      Q = restrictToMinMachineTime(Y,&j); /* j is minimal total machine time of the solutions with the choosen makespan */
    }
    S1 = restrictToMinFS(Q,&i); /* i is minimal number of machines of the solutions from Q */
    Biddy_KeepFormulaUntilPurge(S1);
    if (i == k) {
      /* there exist solutions which are optimal regarding minimal number of machines (i) and minimal total machine time (j) */
    } else {
      /* S1 = solutions which are optimal regarding minimal total machine time (j) but not minimal number of machines (i) */
      /* S2 = solutions which are optimal regarding minimal number of machines (k) but not minimal total machine time (l) */
      if (TIMEBOUND) {
        l = TIMEBOUND;
        S2 = Z;
      } else {
        S2 = restrictToMinMachineTime(Z,&l); /* l is minimal total machine time of the solutions from Z */
      }
      Biddy_KeepFormulaUntilPurge(S2);
    }
  }

  /* REPORT ONE OR BOTH SOLUTIONS */
  if (!VERBOSE && S1 && (MINRESULT > 0)) {
    /*
    PRINTCOMBINATIONS(S1);
    printf("\n");
    */
    printf("\nREPORTING ONE SCHEDULE WITH MAKESPAN = %u, NUMBER OF MACHINES = %u, TOTAL MACHINE TIME = %u\n",S,i,j);
    printf("(found %.0f feasible solutions with the same parameters)\n",Biddy_CountMinterms(S1,0));
    elapsedtime = clock();
    if (generateGanttTimevis("PP-GanttChart-MinTime.Rmd",Biddy_ExtractMinterm(NULL,S1),S,FALSE,restricted)) /* FALSE because G-variables were removed */
    {
      printf("\nclock() TIME = %.2f\n",(clock()-elapsedtime)/(1.0*CLOCKS_PER_SEC));\
      printf("\nGantt chart is in file PP-GanttChart-MinTime.Rmd\n");
    } else {
      printf("\nclock() TIME = %.2f\n",(clock()-elapsedtime)/(1.0*CLOCKS_PER_SEC));\
      printf("\nGantt chart not created\n");
    }

    if (S2) {
      /*
      PRINTCOMBINATIONS(Y);
      printf("\n");
      */
      printf("\nREPORTING ONE SCHEDULE WITH MAKESPAN = %u, NUMBER OF MACHINES = %u, TOTAL MACHINE TIME = %u\n",S,k,l);
      printf("(found %.0f feasible solutions with the same properties)\n",Biddy_CountMinterms(S2,0));
      elapsedtime = clock();
      if (generateGanttTimevis("PP-GanttChart-MinNumber.Rmd",Biddy_ExtractMinterm(NULL,S2),S,FALSE,restricted))  /* FALSE because G-variables were removed */
      {
        printf("\nclock() TIME = %.2f\n",(clock()-elapsedtime)/(1.0*CLOCKS_PER_SEC));\
        printf("\nGantt chart is in file PP-GanttChart-MinNumber.Rmd\n");
      } else {
        printf("\nclock() TIME = %.2f\n",(clock()-elapsedtime)/(1.0*CLOCKS_PER_SEC));\
        printf("\nGantt chart not created\n");
      }
    }
  }

  /* IN VERBOSE MODE REPORT ALL INTERESTING FEASIBLE SOLUTIONS */
  if (VERBOSE && S1 && (MINRESULT > 0)) {
    if (S2) {
      S1 = Biddy_Union(S1,S2);
      Biddy_KeepFormulaUntilPurge(S1);
    }
    k = Biddy_CountMinterms(S1,0);
    printf("\nREPORTING ONE SCHEDULE FOR ALL %u INTERESTING FEASIBLE SOLUTIONS FOUND IN THIS RUN\n",k);
    if (k > 132) {
      k = 132; /* MAX NUMBER OF REPORTED COMBINATIONS */
      printf("(too many cubes, reporting only the first %u of them, starting with those with the minimal number of machines)\n",k);
    }
    j = 0;
    printf("\n");
    while (j < k) {
      for (i=1; (j < k) && (i <= FACTORYCAPACITY); i++) {
        v = findFSvariable(i);
        f = Biddy_Subset1(S1,v);
        if (Biddy_CountMinterms(f,0)) {
          while ((j < k) && (f != Biddy_GetEmptySet())) {
            g = Biddy_ExtractMinterm(NULL,f);
            f = Biddy_Diff(f,g);
            Biddy_KeepFormulaUntilPurge(f);
#ifdef PAPER
#ifdef LATEX
            /*
            PRINTCOMBINATIONS(g);
            printf("\n");
            */
#else
            /*
            Biddy_PrintfMinterms(g,FALSE);
            printf("\n");
            */
#endif
#endif
            j++;
            ganttname = strdup("PP-GanttChart");
            concatNumber(&ganttname,j);
            concat(&ganttname,".Rmd");
            if (generateGanttTimevis(ganttname,g,S,FALSE,restricted))  /* FALSE because G-variables were removed */
            {
              printf("\nGANTT CHART IS STORED IN FILE %s\n\n",ganttname);
            } else {
              printf("\nError while creating Gantt chart\n\n");
            }
            free(ganttname);
          }
        }
      }
    }
  }

  /* SIFTING ON SYSTEM */
  /*
  printf("\nSIFTING ON SYSTEM\n");
  Biddy_Sifting(NULL,FALSE);
  Biddy_Sifting(NULL,TRUE);
  reportOrdering();
  */

  /* FINAL REPORT ON SYSTEM */

  /*
  reportOrdering();
  */

  printf("\nNUMBER OF VARIABLES NEEDED TO FINISH THE CALCULATION: %u\n",Biddy_VariableTableNum());

  /**/
  if (!trace) {
    printf("\n");
    Biddy_PrintInfo(NULL);
  }
  /**/

  /* DELETE SYSTEM */
  deleteSystem();

  Biddy_Exit();
}

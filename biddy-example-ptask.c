/* $Revision$ */
/* $Date$ */
/* This file (biddy-example-ptask.c) is a C file */
/* Author: Robert Meolic (robert@meolic.com) */
/* This file has been released into the public domain by the author. */

/* This example is compatible with Biddy v2.0 and laters */

/* COMPILE WITH (ADD -lgmp IF USING STATIC BIDDY LIBRARY): */
/* gcc -DUNIX -O2 -o biddy-example-ptask biddy-example-ptask.c -I. -L./bin -lbiddy */

/* this example is about the Shift Minimization Personnel Task Scheduling problem (SMPTSP) */

/*
filename = strdup("/home/meolic/biddy/ptask/data_122_422_1358_66.dat");
ZBDD for result after step 1172: 27416604 nodes (including terminals), clock() TIME = 16028.67
ZBDD for result after step 1173: 27460205 nodes (including terminals), clock() TIME = 18279.63
ZBDD for result after step 1174: 27478275 nodes (including terminals), clock() TIME = 20124.18
*/

#include "biddy.h"
#include "string.h"
#include <time.h>

/* choose primary BDD type */
#define BDDTYPE BIDDYTYPEZBDD

/* define PAPER to have variable names usable for the paper */
/* define LATEX to have variable names compatible with LaTeX (otherwise MS Word Equation style is used) */
#define NOPAPER
#define NOLATEX

/* define CHECKOPERATIONS for debugging single operations */
#define NOCHECKOPERATIONS

/* define INTERNALREPORTS for internal reports - they can be huge! */
#define NOINTERNALREPORTS

/* use sifting during the calculation: 0 = NO, 1 = YES */
#define SIFTING 0

/* MAIN DATA STRUCTURES */

typedef struct LIMIT {
  unsigned int job;
  unsigned int shift;
  unsigned int w;
} LIMIT;

typedef struct {
  unsigned int n;
  Biddy_Edge f;
  Biddy_Edge r; /* NULL = not valid node! */
} PermitEffortCache;

typedef struct {
  PermitEffortCache *table;
  unsigned int size;
} PermitEffortCacheTable;

const unsigned int permitEffortCacheSize = 1048573;

PermitEffortCacheTable permitEffortCache = {NULL,0};

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

#define foaTvariable(j,w) findoraddTvariable(j,w,FALSE,FALSE)
#define findTvariable(j,w) findoraddTvariable(j,w,FALSE,TRUE)

#define foaRvariable(w) findoraddRvariable(w,FALSE,FALSE)
#define findRvariable(w) findoraddRvariable(w,FALSE,TRUE)

#define foaSvariable(w) findoraddSvariable(w,FALSE,FALSE)
#define findSvariable(w) findoraddSvariable(w,FALSE,TRUE)

#define ADDVARIABLE(j,w) \
{\
findoraddTvariable(j,w,TRUE,FALSE);\
findoraddRvariable(w,TRUE,FALSE);\
findoraddSvariable(w,TRUE,FALSE);\
}

void
addVariableTable(unsigned int *variableTableSize, VARIABLE **variableTable, VARIABLE var)
{
  unsigned int i,min,max;
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
    /* VARIANT A: LINEAR SEARCH */
    /*
    for (i=0; !FIND && (i < *variableTableSize); i++) {
      cc = strcmp(var.name,(*variableTable)[i].name);
      if (cc == 0) {
        FIND = TRUE;
        break;
      }
      if (cc > 0) break; / * > means increasing order, < means decreasing order * /
    }
    */

    /* VARIANT B: BINARY SEARCH - ordered from smallest to greatest */
    /**/
    min = 0;
    max = *variableTableSize - 1;
    cc = strcmp(var.name,(*variableTable)[min].name);
    if (cc == 0) {
      i = min;
      FIND = TRUE;
    } else if (cc < 0) {
      i = min;
      FIND = FALSE;
    } else {
      cc = strcmp(var.name,(*variableTable)[max].name);
      if (cc == 0) {
        i = max;
        FIND = TRUE;
      } else if (cc > 0) {
        i = max+1;
        FIND = FALSE;
      } else {
        i = (min + max) / 2;
        while (i != min) {
          cc = strcmp(var.name,(*variableTable)[i].name);
          if (cc == 0) {
            min = max = i;
            FIND = TRUE;
          } else if (cc < 0) {
            max = i;
          } else {
            min = i;
          }
          i = (min + max) / 2;
        }
        if (!FIND) i++;
      }
    }
    /**/

  }

  /* IF (FIND == TRUE) THEN i IS THE INDEX OF THE CORRECT VARIABLE! */
  
  /* OTHERWISE, ADD NEW VARIABLE */
  if (!FIND) {
    (*variableTableSize)++;
    *variableTable = (VARIABLE *) realloc(*variableTable,(*variableTableSize)*sizeof(VARIABLE));
    memmove(&(*variableTable)[i+1],&(*variableTable)[i],((*variableTableSize)-i-1)*sizeof(VARIABLE));
    (*variableTable)[i].name = strdup(var.name);
    (*variableTable)[i].papername = strdup(var.papername);
    (*variableTable)[i].limit = var.limit;
  }
}

void
statVariableTable(unsigned int variableTableSize, VARIABLE *variableTable)
{
}

void
optimizeVariableTable(unsigned int *variableTableSize, VARIABLE **variableTable)
{
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

      v = Biddy_AddElementByName(name);
      if (var.limit.job) {
        data = (LIMIT *) malloc(sizeof(LIMIT));
        data->job = var.limit.job;
        data->shift = var.limit.shift;
        data->w = var.limit.w;
        Biddy_SetVariableData(v,(void *) data);
      }
    }
  }

  return v;
}

Biddy_Variable
findoraddTvariable(unsigned int j, unsigned int w, 
                   Biddy_Boolean addTable, Biddy_Boolean findOnly)
{
  VARIABLE var;
  /* char seq[10] = "W...J...."; */
  char seq[10] = "J....W...";
  Biddy_Variable v;

  var.limit.job = j;
  var.limit.shift = w;
  var.limit.w = 0;

  foavarname[0] = 0;
  papervarname[0] = 0;
  var.name = foavarname;
  var.papername = papervarname;

#ifdef PAPER
#ifdef LATEX
  sprintf(var.papername,"T^{%u}_{%u}",j,w);
#else
  sprintf(var.papername,"T^%u_%u",j,w);
#endif
#endif

  /* used for calculation, j must be 1 .. 9999, w must be 1 .. 999 */
  seq[1] = (j/1000) + '0';
  seq[2] = ((j/100)%10) + '0';
  seq[3] = ((j/10)%10) + '0';
  seq[4] = (j%10) + '0';
  seq[6] = (w/100) + '0';
  seq[7] = ((w/10)%10) + '0';
  seq[8] = (w%10) + '0';
  concatStatic(var.name,seq);

  v = 0;

  if (addTable) {
    addVariableTable(&variableTableSize,&variableTable,var);
  } else {
    v = foaVariable(var,findOnly);
  }

  return v;
}

Biddy_Variable
findoraddRvariable(unsigned int w, 
                   Biddy_Boolean addTable, Biddy_Boolean findOnly)
{
  VARIABLE var;
  char seq[5] = "R...";
  Biddy_Variable v;

  var.limit.job = 0;
  var.limit.shift = w;
  var.limit.w = 0;

  foavarname[0] = 0;
  papervarname[0] = 0;
  var.name = foavarname;
  var.papername = papervarname;

#ifdef PAPER
#ifdef LATEX
  sprintf(var.papername,"R_{%u}",w);
#else
  sprintf(var.papername,"R_%u",w);
#endif
#endif

  /* used for calculation, w must be 1 .. 999 */
  seq[1] = (w/100) + '0';
  seq[2] = ((w/10)%10) + '0';
  seq[3] = (w%10) + '0';
  concatStatic(var.name,seq);

  v = 0;

  if (addTable) {
    addVariableTable(&variableTableSize,&variableTable,var);
  } else {
    v = foaVariable(var,findOnly);
  }

  return v;
}

Biddy_Variable
findoraddSvariable(unsigned int w, 
                   Biddy_Boolean addTable, Biddy_Boolean findOnly)
{
  VARIABLE var;
  char seq[5] = "S...";
  Biddy_Variable v;

  var.limit.job = 0;
  var.limit.shift = w;
  var.limit.w = 0;

  foavarname[0] = 0;
  papervarname[0] = 0;
  var.name = foavarname;
  var.papername = papervarname;

#ifdef PAPER
#ifdef LATEX
  sprintf(var.papername,"S_{%u}",w);
#else
  sprintf(var.papername,"S_%u",w);
#endif
#endif

  /* used for calculation, w must be 1 .. 999 */
  seq[1] = (w/100) + '0';
  seq[2] = ((w/10)%10) + '0';
  seq[3] = (w%10) + '0';
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

  /**/ for (i=0; i < variableTableSize; i++) /**/ /* REVERSE ORDERING FOR ZBDDs - WE GET BETTER RESULTS */
  /* for (i=variableTableSize-1; i>=0; i--) */
  {
#ifdef PAPER
    name = variableTable[i].papername;
#else
    name = variableTable[i].name;
#endif
    /* printf("(%s)",name); */
    v = Biddy_AddElementByName(name);
    if (variableTable[i].limit.job) {
      data = (LIMIT *) malloc(sizeof(LIMIT));
      data->job = variableTable[i].limit.job;
      data->shift = variableTable[i].limit.shift;
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


/* ************************************************************************** */
/* REPORT RESULTS */
/* ************************************************************************** */


/* ************************************************************************** */
/* MAIN PROGRAM */
/* ************************************************************************** */

int main(int argc, char **argv) {
  clock_t elapsedtime;
  FILE *infile;
  char *filename,*buffer,*curline,*nextline;
  size_t index,len;
  ssize_t bytes_read;
  unsigned int i,j,w;
  Biddy_String word;

  /* SYSTEM DEFINITION */
  unsigned int numJobs = 0;
  unsigned int numShifts = 0;

  /* BDD variables */
  Biddy_Variable v;
  Biddy_Edge set1,set2,result;

  setbuf(stdout,NULL);

  variableTableSize = 0;
  variableTable = NULL;

  /* CHECK ARGUMENTS */
  i = 1;
  while (i < argc) {

    if (!strcmp(argv[i],"-h") || !strcmp(argv[i],"-help")) {
      exit(0);
    }

    i++;
  }

  /* READ dat FILE FROM ptask FOLDER */

  printf("READ INPUT FILE\n");

  /* filename = strdup("/home/meolic/biddy/ptask/data_1_23_40_66.dat"); */
  /* filename = strdup("/home/meolic/biddy/ptask/data_23_74_180_66.dat"); */
  /* filename = strdup("/home/meolic/biddy/ptask/data_25_120_200_33.dat"); */
  /* filename = strdup("/home/meolic/biddy/ptask/data_26_116_203_66.dat"); */
  /* filename = strdup("/home/meolic/biddy/ptask/data_44_121_400_33.dat"); */
  /**/ filename = strdup("/home/meolic/biddy/ptask/data_122_422_1358_66.dat"); /**/
  infile = fopen(filename,"r");
  if (!infile) {
    printf("ERROR: File %s does not exists!\n",filename);
    return 1;
  }
  free(filename);

  /* this is from */
  /* https://stackoverflow.com/questions/174531/how-to-read-the-content-of-a-file-to-a-string-in-c */
  buffer = NULL;
  len = 0;
  bytes_read = getdelim(&buffer,&len,'\0',infile);
  if (bytes_read == -1) {
    printf("ERROR: File %s cannot be read!\n",filename);
    free(buffer);
    return 1;
  }

  /* DEBUGGING */
  /*
  printf("%s",buffer);
  */

  /* this is from */
  /* https://stackoverflow.com/questions/17983005/c-how-to-read-a-string-line-by-line */
  curline = buffer;
  word = malloc(256); /* max length of one word, all words are number */
  while(curline) {
    nextline = strchr(curline,'\n');
    if (nextline) *nextline = '\0';

    /* DEBUGGING */
    /*
    printf("%s\n",curline);
    */

    index = 0;
    len = strlen(curline);
    while (index != len) {
      while ((index < len) && (curline[index] == ' ' || curline[index] == '\t')) index++;
      if (index == len) break;
      if (curline[index] == '#') break;

      /* first word */
      i = index;
      while ((index < len) && (curline[index] != ' ') && (curline[index] != '\t')) index++;
      memcpy(word,&curline[i],index-i);
      word[index-i] = 0;

      if (!strcmp(word,"Type")) {
        printf("\n%s =",word); /* DEBUGGING */
        while ((index < len) && (curline[index] == ' ' || curline[index] == '\t' || curline[index] == '=')) index++;
        i = index;
        while ((index < len) && (curline[index] != ' ') && (curline[index] != '\t')) index++;
        memcpy(word,&curline[i],index-i);
        word[index-i] = 0;
        printf(" %s\n",word); /* DEBUGGING */
      } else if (!strcmp(word,"Jobs")) {
        printf("%s =",word); /* DEBUGGING */
        while ((index < len) && (curline[index] == ' ' || curline[index] == '\t' || curline[index] == '=')) index++;
        i = index;
        while ((index < len) && (curline[index] != ' ') && (curline[index] != '\t')) index++;
        memcpy(word,&curline[i],index-i);
        word[index-i] = 0;
        printf(" %s\n",word); /* DEBUGGING */
      } else if (!strcmp(word,"Qualifications")) {
        printf("\n%s =",word); /* DEBUGGING */
        while ((index < len) && (curline[index] == ' ' || curline[index] == '\t' || curline[index] == '=')) index++;
        i = index;
        while ((index < len) && (curline[index] != ' ') && (curline[index] != '\t')) index++;
        memcpy(word,&curline[i],index-i);
        word[index-i] = 0;
        printf(" %s\n",word); /* DEBUGGING */
      } else if (word[strlen(word)-1] == ':') {
        /* printf("SHIFT :"); */ /* DEBUGGING */
        sscanf(word,"%u",&w);
        printf("[%u]",numShifts);
        numShifts++;
        for (; w>0; w--) {
          while ((index < len) && (curline[index] == ' ' || curline[index] == '\t' || curline[index] == '=')) index++;
          if (index == len) break;
          i = index;
          while ((index < len) && (curline[index] != ' ') && (curline[index] != '\t')) index++;
          memcpy(word,&curline[i],index-i);
          word[index-i] = 0;
          /* printf(" %s",word); */ /* DEBUGGING */
          sscanf(word,"%u",&j);
          ADDVARIABLE(j,numShifts-1);
        }
        /* printf("\n"); */ /* DEBUGGING */
      } else {
        /* printf("JOB START %s",word); */ /* DEBUGGING */
        printf("[%u]",numJobs);
        numJobs++;
        while ((index < len) && (curline[index] == ' ' || curline[index] == '\t' || curline[index] == '=')) index++;
        i = index;
        while ((index < len) && (curline[index] != ' ') && (curline[index] != '\t')) index++;
        memcpy(word,&curline[i],index-i);
        word[index-i] = 0;
        /* printf(" STOP %s\n",word); */ /* DEBUGGING */
      }

      break;
    }

    if (nextline) *nextline = '\n';
    curline = nextline ? (nextline+1) : NULL;
  }
  free(word);

  printf("\n\nINITIALIZATION (numJobs = %u, numShifts = %u)\n\n",numJobs,numShifts);

  if (numJobs == 0) {
    printf("Error: numTasks = %u\n",numJobs);
    exit(1);
  }

  if (numShifts == 0) {
    printf("Error: numTasks = %u\n",numShifts);
    exit(1);
  }

  /* START CALCULATION */

  Biddy_InitAnonymous(BDDTYPE);

  elapsedtime = clock();

  printf("CREATE VARIABLES\n");

  /* CREATE ALL VARIABLES BEFORE THE REAL CALCULATION START */
  /* THIS IS REQUIRED FOR OBDDs AND TZBDDs, IT IS OPTIONAL FOR ZBDDs */
  /* THIS IS ALSO REQUIRED IF CUSTOM VARIABLE ORDERING IS NEEDED */

  createVariables(); /* this uses variableTable filled by macro ADDVARIABLE and function addVariableTable() */

  /* DEBUGGING */
  /*
  printf("INITIAL ORDERING\n");
  reportOrdering();
  */

  printf("\nSTARTING CALCULATION USING %s\n",Biddy_GetManagerName());

  /* CREATE THE COMPREHENSIVE SET OF PLANS */
  /* NO RESTRICTIONS ARE APPLIED, YET */

  printf("\nCREATE THE COMPREHENSIVE SET OF PLANS\n");

  result = Biddy_GetBaseSet();
  for (j=0; j<numJobs; j++) {
    set1 = Biddy_GetEmptySet();
    for (w=0; w<numShifts; w++) {
      if ((v = findTvariable(j,w))) {
        set2= Biddy_GetBaseSet();
        set2 = Biddy_Change(set2,v);
        set1 = Biddy_Union(set1,set2);
      }
    }
    result = Biddy_Product(result,set1);
    Biddy_KeepFormula(result);
    Biddy_Clean();

    /* SHORT DEBUGGING */
    /**/
    printf("%s for result after step %u: %u nodes (including terminals), ",
           Biddy_GetManagerName(),j,Biddy_CountNodes(result));
    printf("clock() TIME = %.2f\n",(clock()-elapsedtime)/(1.0*CLOCKS_PER_SEC));
    /**/

    /* LONG DEBUGGING */
    /*
    printf("\nCREATE THE COMPREHENSIVE SET OF PLANS (j = %u)\n",j);
    printf("%s for result after step %u: %u nodes (including terminals), %.0f cubes, ",
           Biddy_GetManagerName(),j,Biddy_CountNodes(result),Biddy_CountCombinations(result));
    printf("clock() TIME = %.2f\n",(clock()-elapsedtime)/(1.0*CLOCKS_PER_SEC));
    */

    /* SIFTING ON RESULT - use argument TRUE for converge variant */
    if (SIFTING && Biddy_CountNodes(result) > 10000) {
      printf("SIFTING ON RESULT (j = %u)\n",j);
      Biddy_Sifting(result,FALSE);
      printf("AFTER SIFTING (j = %u)\n",j);
      printf("%s for result after step %u: %u nodes (including terminals), ",
             Biddy_GetManagerName(),j,Biddy_CountNodes(result));
      printf("clock() TIME = %.2f\n",(clock()-elapsedtime)/(1.0*CLOCKS_PER_SEC));
    }
    if (SIFTING) {
      printf("ORDERING\n");
      reportOrdering();
    }

  }

  if (SIFTING && Biddy_CountNodes(result) > 10000) {
    printf("STARTED FINAL SIFTING\n");
    Biddy_Sifting(result,FALSE); /* TRUE = CONVERGE SIFTING */
    printf("%s for result after step %u: %u nodes (including terminals), ",
           Biddy_GetManagerName(),j,Biddy_CountNodes(result));
    printf("clock() TIME = %.2f\n",(clock()-elapsedtime)/(1.0*CLOCKS_PER_SEC));
  }
  if (SIFTING) {
    printf("\nFINAL ORDERING\n");
    reportOrdering();
  }
  
  /* DEBUGGING */
  /*
  printf("\nFINAL ORDERING\n");
  reportOrdering();
  */

  printf("\nCALCULATE THE SET OF FEASIBLE SOLUTIONS\n\n");

  /* SHORT DEBUGGING */
  /**/
  printf("FINAL %s for result after step %u: %u nodes (including terminals), ",
         Biddy_GetManagerName(),j,Biddy_CountNodes(result));
  printf("clock() TIME = %.2f\n",(clock()-elapsedtime)/(1.0*CLOCKS_PER_SEC));
  /**/

  /* LONG DEBUGGING */
  /*
  printf("FINAL %s for result after step %u: %u nodes (including terminals), %.0f cubes, ",
         Biddy_GetManagerName(),j,Biddy_CountNodes(result),Biddy_CountCombinations(result));
  printf("clock() TIME = %.2f\n",(clock()-elapsedtime)/(1.0*CLOCKS_PER_SEC));
  */

  printf("\n");
  printf("clock() TIME = %.2f\n",(clock()-elapsedtime)/(1.0*CLOCKS_PER_SEC));
  printf("NO SOLUTIONS, YET\n");

  /**/
  Biddy_PrintInfo(NULL);
  /**/

  Biddy_Exit();
}

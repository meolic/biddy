/**CFile*******************************************************************
  PackageName [Bdd Scout]
  Synopsis    [Bdd Scout]

  FileName    [bddscout.c]
  Revision    [$Revision: 260 $]
  Date        [$Date: 2017-03-20 23:01:02 +0100 (pon, 20 mar 2017) $]
  Authors     [Robert Meolic (robert.meolic@um.si)]
  Description []
  SeeAlso     [bddscout.h]

  Copyright   [This file is part of Bdd Scout package.
               Copyright (C) 2008, 2017 UM-FERI
               UM-FERI, Smetanova ulica 17, SI-2000 Maribor, Slovenia

               Bdd Scout is free software; you can redistribute it and/or modify
               it under the terms of the GNU General Public License as
               published by the Free Software Foundation; either version 2
               of the License, or (at your option) any later version.

               Bdd Scout is distributed in the hope that it will be useful,
               but WITHOUT ANY WARRANTY; without even the implied warranty of
               MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
               GNU General Public License for more details.

               You should have received a copy of the GNU General Public
               License along with this program; if not, write to the Free
               Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
               Boston, MA 02110-1301 USA.]
  ************************************************************************/

#include "bddscout.h"

#ifdef UNIX
#include <sys/wait.h>
#endif

/* used for STUBS mechanism */
extern BddscoutStubs bddscoutStubs;

/* on tcl 8.3 use #define USECONST */
/* on tcl 8.4 use #define USECONST const*/
/* this is defined in Makefile */

/*-----------------------------------------------------------------------*/
/* Type declarations                                                     */
/*-----------------------------------------------------------------------*/

/*-----------------------------------------------------------------------*/
/* Structure declarations                                                */
/*-----------------------------------------------------------------------*/

/*-----------------------------------------------------------------------*/
/* Variable declarations                                                 */
/*-----------------------------------------------------------------------*/

int SCAN_RESULT; /* used in readln macro */

Biddy_Manager MNGROBDD,MNGZBDD,MNGTZBDD;
Biddy_Manager MNGACTIVE;

/*-----------------------------------------------------------------------*/
/* Static function prototypes                                            */
/*-----------------------------------------------------------------------*/

static unsigned int MyNodeNumber(Biddy_Edge f);

static unsigned int MyNodeMaxLevel(Biddy_Edge f);

static float MyNodeAvgLevel(Biddy_Edge f);

static unsigned int MyPathNumber(Biddy_Edge f);

static float MyMintermNumber(Biddy_Edge f);

static void concat(char **s1, const char *s2);

/*-----------------------------------------------------------------------*/
/* External functions                                                    */
/*-----------------------------------------------------------------------*/

/*-----------------------------------------------------------------------*/
/* Internal functions acessible via TCL mechanism                        */
/*-----------------------------------------------------------------------*/

/**Function****************************************************************
  Synopsis    [Function Bddscout_GetActiveManager]
  Description [Bddscout_GetActiveManager is used in BDD Scout extensions.]
  SideEffects []
  SeeAlso     []
  ************************************************************************/

Biddy_Manager
Bddscout_GetActiveManager()
{
  return MNGACTIVE;
}

/*-----------------------------------------------------------------------*/
/* Internal functions                                                    */
/*-----------------------------------------------------------------------*/

/**Function****************************************************************
  Synopsis    [Function BddscoutListVariablesByPosition]
  Description [BddscoutListVariablesByPosition creates a list of all BDD
               variables sorted by position in Variable table.]
  SideEffects []
  SeeAlso     []
  ************************************************************************/

void
BddscoutListVariablesByPosition(Biddy_String *list)
{
  Biddy_Variable v;
  Biddy_String name;

  if (!list) return;

  for (v=0; v<Biddy_Managed_VariableTableNum(MNGACTIVE); v++) {
    name = Biddy_Managed_GetVariableName(MNGACTIVE,v);
    concat(list,name);
  }
}

/**Function****************************************************************
  Synopsis    [Function BddscoutListVariablesByName]
  Description [BddscoutListVariablesByName creates a list of all BDD
               variables sorted by name.]
  SideEffects [SORTING NOT IMPLEMENTED, YET]
  SeeAlso     []
  ************************************************************************/

void
BddscoutListVariablesByName(Biddy_String *list)
{
  Biddy_Variable v;
  Biddy_String name;

  if (!list) return;

  for (v=0; v<Biddy_Managed_VariableTableNum(MNGACTIVE); v++) {
    name = Biddy_Managed_GetVariableName(MNGACTIVE,v);
    concat(list,name);
  }
}

/**Function****************************************************************
  Synopsis    [Function BddscoutListVariablesByOrder]
  Description [BddscoutListVariablesByOrder creates a list of all BDD
               variables sorted by order in active ordering.]
  SideEffects []
  SeeAlso     []
  ************************************************************************/

void
BddscoutListVariablesByOrder(Biddy_String *list)
{
  Biddy_Variable v;
  Biddy_Variable k;
  Biddy_String name;

  if (!list) return;

  v = 0;
  for (k = 1; k < Biddy_Managed_VariableTableNum(MNGACTIVE); k++) {
      v = Biddy_Managed_GetPrevVariable(MNGACTIVE,v);
  }
  for (k = 1; k < Biddy_Managed_VariableTableNum(MNGACTIVE); k++) {
    name = Biddy_Managed_GetVariableName(MNGACTIVE,v);
    concat(list,name);
    v = Biddy_Managed_GetNextVariable(MNGACTIVE,v);
  }
}

/**Function****************************************************************
  Synopsis    [Function BddscoutListVariablesByNumber]
  Description [BddscoutListVariablesByNumber creates a list of all BDD
               variables sorted by number of BDD nodes.]
  SideEffects [SORTING NOT IMPLEMENTED, YET]
  SeeAlso     []
  ************************************************************************/

void
BddscoutListVariablesByNumber(Biddy_String *list)
{
  Biddy_Variable v;
  Biddy_String name;

  if (!list) return;

  for (v=0; v<Biddy_Managed_VariableTableNum(MNGACTIVE); v++) {
    name = Biddy_Managed_GetVariableName(MNGACTIVE,v);
    concat(list,name);
  }
}

/**Function****************************************************************
  Synopsis    [Function BddscoutListFormulaByName]
  Description [BddscoutListFormulaByName creates a sorted list of all
               formulae in Biddy Formulae tree.]
  SideEffects []
  SeeAlso     []
  ************************************************************************/

void
BddscoutListFormulaByName(Biddy_String *list)
{
  Biddy_Edge formula;
  Biddy_String formulaname;
  unsigned int i;

  if (!list) return;

  i = 0;
  formula = Biddy_Managed_GetIthFormula(MNGACTIVE,i);
  formulaname = Biddy_Managed_GetIthFormulaName(MNGACTIVE,i);
  while (formulaname) {
    if (!Biddy_IsNull(formula) &&
        strcmp(formulaname,"BIDDY_DELETED_FORMULA") &&
        (!strcmp(formulaname,"1") ||
         strcmp(formulaname,Biddy_Managed_GetTopVariableName(MNGACTIVE,formula))
        )
       )
    {
      if (i == 0) {
        free(*list);
        *list = strdup(formulaname);
      } else {
        concat(list,formulaname);
      }
    }

    i++;
    formula = Biddy_Managed_GetIthFormula(MNGACTIVE,i);
    formulaname = Biddy_Managed_GetIthFormulaName(MNGACTIVE,i);
  }
}

/**Function****************************************************************
  Synopsis    [Function BddscoutListFormulaByUIntParameter]
  Description [Bddscout_ListFormulaByUIntParameter creates a sorted list
               of all formulae in Biddy Formulae tree.
               Parameter is a function used for sorting.]
  SideEffects []
  SeeAlso     []
  ************************************************************************/

void
BddscoutListFormulaByUIntParameter(Biddy_String *list, unsigned int (*PF)(Biddy_Edge))
{
  typedef struct {
    Biddy_Edge t;
    Biddy_String name;
    unsigned int parameter;
  } FT;

  FT *sortingTable;
  FT *tmp;
  unsigned int tableSize;
  Biddy_String newlist;
  Biddy_String number;
  Biddy_Edge formula;
  Biddy_String formulaname;
  unsigned int i,j;
  unsigned int n;

  if (!list) return;

  tableSize = 0;
  sortingTable = NULL;
  i = 0;
  formula = Biddy_Managed_GetIthFormula(MNGACTIVE,i);
  formulaname = Biddy_Managed_GetIthFormulaName(MNGACTIVE,i);
  while (formulaname) {
    if (!Biddy_IsNull(formula) &&
        strcmp(formulaname,"BIDDY_DELETED_FORMULA") &&
        (!strcmp(formulaname,"1") ||
         strcmp(formulaname,Biddy_Managed_GetTopVariableName(MNGACTIVE,formula))
        )
       )
    {

      if (!strcmp(*list,"NULL")) {

        free(*list);
        *list = strdup("");
        sortingTable = (FT *) malloc(sizeof(FT));
        if (!sortingTable) return;
        tableSize = 1;
        sortingTable[0].t = formula;
        sortingTable[0].name = formulaname;
        sortingTable[0].parameter = PF(formula);

      } else {

        tableSize++;
        tmp = (FT *) realloc(sortingTable,sizeof(FT)*tableSize);
        if (!tmp) return;
        sortingTable = tmp;

        if (tableSize > 2) j = tableSize - 2; else j = 0;
        n = PF(formula);
        while ((j>=0) && (sortingTable[j].parameter > n)) {
          sortingTable[j+1].t = sortingTable[j].t;
          sortingTable[j+1].name = sortingTable[j].name;
          sortingTable[j+1].parameter = sortingTable[j].parameter;
          j--;
        }
#pragma warning(suppress: 6386)
        sortingTable[j+1].t = formula;
        sortingTable[j+1].name = formulaname;
        sortingTable[j+1].parameter = n;
      }

    }

    i++;
    formula = Biddy_Managed_GetIthFormula(MNGACTIVE,i);
    formulaname = Biddy_Managed_GetIthFormulaName(MNGACTIVE,i);
  }

  for (i=0;i<tableSize;++i) {
    number = (Biddy_String) malloc(255);
    if (!number) return;
    sprintf(number,"(%d)",sortingTable[i].parameter);
    newlist = (Biddy_String)
      malloc(strlen(*list)+ strlen(sortingTable[i].name)+strlen(number)+2);
    if (!newlist) return;
    sprintf(newlist,"%s %s%s",*list,sortingTable[i].name,number);
    free(number);
    free(*list);
    *list = newlist;
  }
}

/**Function****************************************************************
  Synopsis    [Function BddscoutListFormulaByFloatParameter]
  Description [BddscoutListFormulaByFloatParameter creates a sorted list
               of all formulae in Biddy Formulae tree.
               Parameter is a function used for sorting.]
  SideEffects []
  SeeAlso     []
  ************************************************************************/

void
BddscoutListFormulaByFloatParameter(Biddy_String *list, float (*PF)(Biddy_Edge))
{
  typedef struct {
    Biddy_Edge t;
    Biddy_String name;
    float parameter;
  } FT;

  FT *sortingTable;
  FT *tmp;
  int tableSize;
  Biddy_String newlist;
  Biddy_String number;
  Biddy_Edge formula;
  Biddy_String formulaname;
  int i,j;
  float n;

  if (!list) return;

  tableSize = 0;
  sortingTable = NULL;
  i = 0;
  formula = Biddy_Managed_GetIthFormula(MNGACTIVE,i);
  formulaname = Biddy_Managed_GetIthFormulaName(MNGACTIVE,i);
  while (formulaname) {
    if (!Biddy_IsNull(formula) &&
        strcmp(formulaname,"BIDDY_DELETED_FORMULA") &&
        (!strcmp(formulaname,"1") ||
         strcmp(formulaname,Biddy_Managed_GetTopVariableName(MNGACTIVE,formula))
        )
       )
    {

      if (!strcmp(*list,"NULL")) {

        free(*list);
        *list = strdup("");
        sortingTable = (FT *) malloc(sizeof(FT));
        if (!sortingTable) return;
        tableSize = 1;
        sortingTable[0].t = formula;
        sortingTable[0].name = formulaname;
        sortingTable[0].parameter = PF(formula);

      } else {

        tableSize++;
        tmp = (FT *) realloc(sortingTable,sizeof(FT)*tableSize);
        if (!tmp) return;
        sortingTable = tmp;

        j = tableSize-2;
        n = PF(formula);
        while ((j>=0) && (sortingTable[j].parameter > n)) {
          sortingTable[j+1].t = sortingTable[j].t;
          sortingTable[j+1].name = sortingTable[j].name;
          sortingTable[j+1].parameter = sortingTable[j].parameter;
          j--;
        }
        sortingTable[j+1].t = formula;
        sortingTable[j+1].name = formulaname;
        sortingTable[j+1].parameter = n;
      }

    }

    i++;
    formula = Biddy_Managed_GetIthFormula(MNGACTIVE,i);
    formulaname = Biddy_Managed_GetIthFormulaName(MNGACTIVE,i);
  }

  for (i=0;i<tableSize;++i) {
    number = (Biddy_String) malloc(255);
    if (!number) return;
    sprintf(number,"(%.2f)",sortingTable[i].parameter);
    newlist = (Biddy_String)
      malloc(strlen(*list)+ strlen(sortingTable[i].name)+strlen(number)+2);
    if (!newlist) return;
    sprintf(newlist,"%s %s%s",*list,sortingTable[i].name,number);
    free(number);
    free(*list);
    *list = newlist;
  }
}

/**Function****************************************************************
  Synopsis    [Function BddscoutWriteBddview.]
  Description []
  SideEffects [Uses Biddy_WriteBddview and graphviz package]
  SeeAlso     []
  ************************************************************************/

static void parseDot(FILE *dotfile, Biddy_XY *table, int *xsize, int *ysize);

int
BddscoutWriteBddview(const char filename[], Biddy_String dotexe, Biddy_String name)
{
  FILE *xdotfile;
  int n,m;
  Biddy_XY *table;
  int xsize,ysize;
  Biddy_Edge f;
#if defined(MINGW) || defined(_MSC_VER)
#else
  pid_t childpid;
  int status;
#endif
  char* dotname;
  char* xdotname;
#if defined(MINGW) || defined(_MSC_VER)
  char* docdir;
  char* appdir;
  docdir = getenv("USERPROFILE");
  appdir = strdup("\\AppData\\Local\\");
#endif

  if (!Biddy_Managed_FindFormula(MNGACTIVE,name,&f)) {
    fprintf(stderr,"BddscoutWriteBddview: Function %s does not exists!\n",name);
    return 0;
  }

  if (Biddy_IsNull(f)) {
    fprintf(stderr,"BddscoutWriteBddview: Problem with Biddy_WriteDot for function %s (f==NULL) !\n",name);
    return 0 ;
  }

#if defined(MINGW) || defined(_MSC_VER)
  dotname = (Biddy_String) malloc(strlen(docdir)+strlen(appdir)+7+1);
  dotname[0] = 0;
  strcat(dotname,docdir);
  strcat(dotname,appdir);
  strcat(dotname,"bdd.dot");
  xdotname = (Biddy_String) malloc(strlen(docdir)+strlen(appdir)+8+1);
  if (!xdotname) return -1;
  xdotname[0] = 0;
  strcat(xdotname,docdir);
  strcat(xdotname,appdir);
  strcat(xdotname,"xbdd.dot");
  free(appdir);
#elif UNIX
  dotname = strdup("/tmp/XXXXXX");
  close(mkstemp(dotname));
  xdotname = strdup("/tmp/XXXXXX");
  close(mkstemp(xdotname));
#else
  dotname = strdup("bdd.dot");
  xdotname = strdup("xbdd.dot");
#endif

  n = Biddy_Managed_WriteDot(MNGACTIVE,dotname,f,name,-1,FALSE); /* FALSE is important */

  if (n == 0) {
    fprintf(stderr,"BddscoutWriteBddview: Problem with Biddy_WriteDot for function %s (n==0) !\n",name);
    return(0);
  }

#if defined(MINGW) || defined(_MSC_VER)
  _spawnlp(_P_WAIT, dotexe, "dot.exe", "-y", "-o",xdotname,dotname,NULL);
#else
  childpid = fork();
  if (childpid == 0) {
    execlp(dotexe, "dot", "-y", "-o",xdotname, dotname,NULL);
  } else if (childpid > 0) {
    waitpid(childpid,&status,0);
  }
#endif

  xdotfile = fopen(xdotname,"r");
  if (!xdotfile) {
    fprintf(stderr,"BddscoutWriteBddview: File error (%s)!\n",xdotname);
    return(0);
  }

  table = (Biddy_XY *) malloc((n+1) * sizeof(Biddy_XY)); /* n = nodes, add one label */
  if (!table) return -1;
  parseDot(xdotfile,table,&xsize,&ysize);
  fclose(xdotfile);

  free(dotname);
  free(xdotname);

  m = Biddy_Managed_WriteBddview(MNGACTIVE,filename,f,name,table);

  free(table);

  if (m != n) {
    fprintf(stderr,"Bddscout_WriteBddview: Problem with Biddy_WriteBddview for function %s (m!=n)!\n",name);
    return(0);
  }

  return n;
}

static void
parseDot(FILE *dotfile, Biddy_XY *table, int *xsize, int *ysize)
{
  char line[255];
  Biddy_String r,label,defaultlabel,pos;
  int id,x,y;
  float xf,yf;

  r = fgets(line,255,dotfile);

  /* BOUNDING BOX */
  while (r && !strstr(line,"bb=")) r = fgets(line,255,dotfile);
  if (r) {
    pos = (Biddy_String) malloc(strlen(line));
#pragma warning(suppress: 6031)
    sscanf(&(strstr(line,"bb=")[4]),"%s",pos);
#pragma warning(suppress: 6031)
    sscanf(pos,"%d,%d,%d,%d",&x,&y,xsize,ysize);
    free(pos);
  }

  /* NODES AND EDGES - ONLY INFO ABOUT NODES IS PARSED */
  defaultlabel = NULL;
  while (r) {

    while (r && !strstr(line,"height=") && !strstr(line,"node")) r = fgets(line,255,dotfile);
    if (r && strstr(line,"node")) {
      while (r && !strstr(line,"label=")) r = fgets(line,255,dotfile);
      if (defaultlabel) free(defaultlabel);
      defaultlabel = (Biddy_String) malloc(strlen(line));
#pragma warning(suppress: 6031)
      sscanf(&(strstr(line,"label=")[6]),"%s",defaultlabel);
      defaultlabel[strlen(defaultlabel)-1] = 0;
      while (r && !strstr(line,"height=")) r = fgets(line,255,dotfile);
    }
    if (r) {
#pragma warning(suppress: 6031)
      sscanf(line,"%d",&id);
      table[id].id = id;
    }

    while (r && !strstr(line,"label=") && !strstr(line,"pos=")) r = fgets(line,255,dotfile);
    if (r) {
      if (strstr(line,"label=")) {
        label = (Biddy_String) malloc(strlen(line));
        if (!label) return;
#pragma warning(suppress: 6011)
#pragma warning(suppress: 6031)
        sscanf(&(strstr(line,"label=")[6]),"%s",label);
        label[strlen(label)-1] = 0;
      } else {
        label = strdup(defaultlabel);
      }
    }

    while (r && !strstr(line,"pos=")) r = fgets(line,255,dotfile);
    if (r) {
      pos = (Biddy_String) malloc(strlen(line));
      if (!pos) return;
#pragma warning(suppress: 6011)
#pragma warning(suppress: 6031)
      sscanf(&(strstr(line,"pos=")[5]),"%s",pos);
#pragma warning(suppress: 6031)
      sscanf(pos,"%f,%f",&xf,&yf);

      table[id].label = strdup(label);
      table[id].x = (int)xf; /* float -> int */
      table[id].y = (int)yf; /* float -> int */
      table[id].isConstant = FALSE;
      if (!strcmp(table[id].label,"0")||!strcmp(table[id].label,"1")) {
        table[id].isConstant = TRUE;
      }
      free(label);
      free(pos);

      /*
      fprintf(stderr,"DEBUG: id = %d, label = %s, x = %d, y = %d\n",table[id].id,table[id].label,table[id].x,table[id].y);
      */

      /* TWO OR MORE NODES IN ONE LINE ARE NOT ALLOWED! */
      r = fgets(line,255,dotfile);

    }

  }
  if (defaultlabel) free(defaultlabel);

}

/*-----------------------------------------------------------------------*/
/* Other internal functions (mostly from 1995)                           */
/*-----------------------------------------------------------------------*/

/**Function****************************************************************
  Synopsis    [Function BddscoutParseFile]
  Description []
  SideEffects [Implemented long time ago (1995). Not comprehensible.]
  SeeAlso     []
  ************************************************************************/

#define readln(f,s) SCAN_RESULT = fscanf(f," %[^\n] ",s)

static Biddy_String makeFunction(FILE *f, Biddy_String line);

Biddy_String
BddscoutParseFile(FILE *f)
{
  Biddy_String line,name;
  Biddy_String s1;
  Biddy_String var;
  unsigned int n,x;
  Biddy_String tmp;

  line = (Biddy_String) malloc(4096);
  if (!line) return NULL;

  readln(f,line);
  x = (unsigned int)strcspn(line,"=");
  if (x != strlen(line)) {
    printf("WARNING (BddscoutParseFile): Variable ordering not specified.\n");
  } else {
    s1 = strdup(line);
    readln(f,line);
    x = (unsigned int)strcspn(line,"=");
    while (x == strlen(line))
    {
      n = (unsigned int)strlen(s1)+(unsigned int)strlen(line)+1+1;
      tmp = (Biddy_String) realloc(s1,n);
      if (!tmp) return NULL;
      s1 = tmp;
#pragma warning(suppress: 6386)
      strcat(s1," ");
      strcat(s1,line);
      readln(f,line);
      x = (unsigned int)strcspn(line,"=");
    }

    var = strtok(&s1[1]," \t)");
    while (var) {

      /*
      printf("VARIABLE: <%s>\n",var);
      */

      Biddy_Managed_AddVariableByName(MNGACTIVE,var);
      var = strtok(NULL," \t)");
    }

    free(s1);
  }

  name = NULL;
  while (!feof(f)) {
    if (name) free(name);
    name = makeFunction(f,line);
  }
  if (name) free(name);
  name = makeFunction(f,line);

  free(line);

  return name;
}

static Biddy_String
makeFunction(FILE *f, Biddy_String line)
{
  Biddy_Edge fun;
  Biddy_String s1,s2,name;
  unsigned int len;
  unsigned int x,y;
  Biddy_String tmp;

  if (!line) return NULL;

  x = (unsigned int)strcspn(line, "=");
  if (x == strlen(line)) {
    s1 = NULL;                 /* s1 = left part of the expression */
    s2 = strdup(line);         /* s2 = right part of the expression */
  } else {
    s1 = (Biddy_String) strdup(line);
    s1[x] = 0; /* strndup is not supported everywhere */
    if (x != strlen(line)-1) {
      y = (unsigned int)strspn(&line[x+1], " \t");
      s2 = strdup(&line[x+1+y]);
    } else {
      s2 = NULL;
    }
  }

  if (s1) {
    x = (unsigned int)strspn(s1," \t");
    y = (unsigned int)strcspn(&s1[x]," \t\n");
    name = (Biddy_String) strdup(&s1[x]);
    name[y] = 0; /* strndup is not supported everywhere */
    free(s1);
  } else {
    name = strdup("F");
  }

  if (!feof(f)) {
    readln(f,line);
    x = (unsigned int)strcspn(line,"=");
  } else {
    line[0] = 0;
    line = NULL;
  }

  while (line && (x == strlen(line)))
  {
    if (s2 != NULL) {
      len = (unsigned int)strlen(s2)+(unsigned int)strlen(line)+1;
      tmp = (Biddy_String) realloc(s2, len+1);
      if (!tmp) return NULL;
      s2 = tmp;
#pragma warning(suppress: 6386)
      strcat(s2," ");
      strcat(s2,line);
    } else {
      s2 = strdup(line);
    }

    if (!feof(f)) {
      readln(f,line);
      x = (unsigned int)strcspn(line, "=");
    } else {
      line[0] = 0;
      line = NULL;
    }
  }

  /*
  printf("FUNCTION NAME: <%s>\n",name);
  printf("FUNCTION DEF: <%s>\n",s2);
  */

  fun = Biddy_Managed_Eval1x(MNGACTIVE,s2,(Biddy_LookupFunction) NULL);
  free(s2);

  if (!Biddy_IsNull(fun)) {
    Biddy_Managed_AddPersistentFormula(MNGACTIVE,name,fun);
  }

  return name;
}

/**Function****************************************************************
  Synopsis    [Function BddscoutConstructBDD.]
  Description []
  SideEffects [Implemented only for OBDDs.]
  SeeAlso     []
  ************************************************************************/

typedef struct {
  Biddy_String name;
  int order;
} BddscoutVariableOrder;

typedef struct {
  Biddy_String name;
  int type;
  int l,r;
  Biddy_Edge f;
  Biddy_Boolean created;
} BddscoutTclBDD;

static void reorderVariables(BddscoutVariableOrder *tableV, BddscoutTclBDD *tableN, int id);

static void constructBDD(BddscoutTclBDD *table, int id);

void
BddscoutConstructBDD(int numV, Biddy_String s2, int numN, Biddy_String s4)
{
  BddscoutVariableOrder *tableV;
  BddscoutTclBDD *tableN;
  Biddy_String word;
  int i,id;

  /*
  fprintf(stderr,"Construct BDD\n");
  fprintf(stderr,"%d variables\n",numV);
  fprintf(stderr,"%s\n",s2);
  fprintf(stderr,"%d nodes\n",numN);
  fprintf(stderr,"%s\n",s4);
  */

  tableV = (BddscoutVariableOrder *) malloc(numV * sizeof(BddscoutVariableOrder));
  if (!tableV) return;
  tableN = (BddscoutTclBDD *) malloc(numN * sizeof(BddscoutTclBDD));
  if (!tableN) return;

  if (numV) {
    word = strtok(s2," "); /* parsing list of variables */
    tableV[0].name = strdup(word);
    tableV[0].order = 0;
    for (i=1; i<numV; ++i) {
      word = strtok(NULL," ");
#pragma warning(suppress: 6386)
      tableV[i].name = strdup(word);
      tableV[i].order = 0;
    }
  }

  /* type == 0 : regular label, r = -1 */
  /* type == 1 : complemented label, r = -1 */
  /* type == 2 : terminal node, l = -1, r = -1 */
  /* type == 3 : regular node */
  /* type == 4 : node with complemented "else" */

  word = strtok(s4," "); /* the name of BDD */
  for (i=0; i<numN; ++i) {
    word = strtok(NULL," "); /* node id */
    id = atoi(word);
    word = strtok(NULL," ");
    tableN[id].name = strdup(word);
    word = strtok(NULL," ");
    tableN[id].type = atoi(word);
    word = strtok(NULL," ");
    tableN[id].l = atoi(word);
    word = strtok(NULL," ");
    tableN[id].r = atoi(word);

    if (tableN[id].type == 2) {
      tableN[id].created = TRUE;
      tableN[id].f = Biddy_Managed_GetConstantOne(MNGACTIVE);
    } else {
      tableN[id].created = FALSE;
      tableN[id].f = BDDNULL;
    }
  }

  for (i=0; i<numV; ++i) {
#pragma warning(suppress: 6385)
    Biddy_Managed_AddVariableByName(MNGACTIVE,tableV[i].name); /* PROBABLY NOT CORRECT FOR ZBDDs AND ZFDDs! */
  }

  for (i=0; i<numN; ++i) {
#pragma warning(suppress: 6385)
    if (tableN[i].type == 0) {
      reorderVariables(tableV,tableN,tableN[i].l);
      constructBDD(tableN,tableN[i].l);
      Biddy_Managed_AddPersistentFormula(MNGACTIVE,tableN[i].name,tableN[tableN[i].l].f);
    }
    if (tableN[i].type == 1) {
      reorderVariables(tableV,tableN,tableN[i].l);
      constructBDD(tableN,tableN[i].l);
      Biddy_Managed_AddPersistentFormula(MNGACTIVE,tableN[i].name,Biddy_Managed_Not(MNGACTIVE,tableN[tableN[i].l].f));
    }
  }

  for (i=0; i<numV; ++i) {
    free(tableV[i].name);
  }
  for (i=0; i<numN; ++i) {
    free(tableN[i].name);
  }

  free(tableV);
  free(tableN);
}

static void
reorderVariables(BddscoutVariableOrder *tableV, BddscoutTclBDD *tableN, int id)
{
  /* NOT IMPLEMENTED, YET */
}

static void
constructBDD(BddscoutTclBDD *tableN, int id)
{
  /*
  printf("Construct %d\n",id);
  printf("TABLE[%d]: %s, l:%d, r:%d\n",
          id,tableN[id].name,tableN[id].l,tableN[id].r);
  */

  if (!tableN[id].created) {

    if (!tableN[tableN[id].l].created) {
      constructBDD(tableN,tableN[id].l);
    }

    if ((tableN[id].r != -1) && (!tableN[tableN[id].r].created)) {
      constructBDD(tableN,tableN[id].r);
    }

    if (tableN[id].type == 3) {
      tableN[id].f = Biddy_Managed_ITE(MNGACTIVE,Biddy_Managed_AddVariableByName(MNGACTIVE,tableN[id].name),
                              tableN[tableN[id].r].f,tableN[tableN[id].l].f);
    }

    if (tableN[id].type == 4) {
      tableN[id].f = Biddy_Managed_ITE(MNGACTIVE,Biddy_Managed_AddVariableByName(MNGACTIVE,tableN[id].name),
                              tableN[tableN[id].r].f,Biddy_Managed_Not(MNGACTIVE,tableN[tableN[id].l].f));
    }

    tableN[id].created = TRUE;
  }

}

/**Function****************************************************************
  Synopsis    [Function BddscoutReadBDD.]
  Description []
  SideEffects []
  SeeAlso     []
  ************************************************************************/

Biddy_String
BddscoutReadBDD(FILE *s)
{
  Biddy_String bdd;
  unsigned int len,n;
  Biddy_String name;
  Biddy_String tmp;

  len = 0;
  bdd = strdup("");
  do {
    tmp = (Biddy_String) realloc(bdd,len+255);
    if (!tmp) return NULL;
    bdd = tmp;
    n = (unsigned int)fread(&bdd[len],1,255,s);
    len = len + n;
  } while (n == 255);

  name = Biddy_Managed_Eval0(MNGACTIVE,bdd);
  free(bdd);

  if (!strcmp(name,"")) {
    printf("Bddscout_ReadBDD: ERROR in file\n");
    return("");
  }

  return(name);
}

/**Function****************************************************************
  Synopsis    [Function BddscoutWriteBDD.]
  Description []
  SideEffects []
  SeeAlso     []
  ************************************************************************/

void
BddscoutWriteBDD(char *filename, Biddy_String name)
{
  Biddy_Edge f;

  if (!Biddy_Managed_FindFormula(MNGACTIVE,name,&f)) {
    printf("BddscoutWriteBDD: Function %s does not exists!\n",name);
    return;
  } else {
    Biddy_Managed_WriteBDD(MNGACTIVE,filename,f,name);
  }
}

/*-----------------------------------------------------------------------*/
/* Definition of static functions                                        */
/*-----------------------------------------------------------------------*/

static unsigned int MyNodeNumber(Biddy_Edge f)
{
  return Biddy_Managed_NodeNumber(MNGACTIVE,f);
}

static unsigned int MyNodeMaxLevel(Biddy_Edge f)
{
  return Biddy_Managed_NodeMaxLevel(MNGACTIVE,f);
}

static float MyNodeAvgLevel(Biddy_Edge f)
{
  return Biddy_Managed_NodeAvgLevel(MNGACTIVE,f);
}

static unsigned int MyPathNumber(Biddy_Edge f)
{
  unsigned int n;

  n = (unsigned int) Biddy_Managed_CountPaths(MNGACTIVE,f); /* returns unsigned long long int */
  return n;
}

static float MyMintermNumber(Biddy_Edge f)
{
  float n;

  n = (float) Biddy_Managed_CountMinterm(MNGACTIVE,f,0); /* returns double */
  return n;
}

static void concat(char **s1, const char *s2)
{
   if (s2) {
     *s1 = (char*) realloc(*s1,strlen(*s1)+strlen(s2)+1+1);
     strcat(*s1," ");
     strcat(*s1,s2);
   }
}

/*-----------------------------------------------------------------------*/
/* TCL related functions                                                 */
/*-----------------------------------------------------------------------*/

/**Function****************************************************************
  Synopsis    [Function Bddscout_Init.]
  Description [This function is used by Tcl/Tk only.]
  SideEffects []
  SeeAlso     []
  ************************************************************************/

static int BddscoutInitPkgCmd(ClientData clientData, Tcl_Interp *interp,
                          int argc, USECONST char **argv);

static int BddscoutExitPkgCmd(ClientData clientData, Tcl_Interp *interp,
                          int argc, USECONST char **argv);

static int BddscoutAboutPkgCmd(ClientData clientData, Tcl_Interp *interp,
                          int argc, USECONST char **argv);

static int BddscoutChangeBddTypeCmd(
                          ClientData clientData, Tcl_Interp *interp,
                          int argc, USECONST char **argv);

static int BddscoutParseInfixInputCmd(
                          ClientData clientData, Tcl_Interp *interp,
                          int argc, USECONST char **argv);

static int BddscoutListVariablesByPositionCmd(
                          ClientData clientData, Tcl_Interp *interp,
                          int argc, USECONST char **argv);

static int BddscoutListVariablesByNameCmd(
                          ClientData clientData, Tcl_Interp *interp,
                          int argc, USECONST char **argv);

static int BddscoutListVariablesByOrderCmd(
                          ClientData clientData, Tcl_Interp *interp,
                          int argc, USECONST char **argv);

static int BddscoutListVariablesByNumberCmd(
                          ClientData clientData, Tcl_Interp *interp,
                          int argc, USECONST char **argv);

static int BddscoutListFormulaeByNameCmd(
                          ClientData clientData, Tcl_Interp *interp,
                          int argc, USECONST char **argv);

static int BddscoutListFormulaeByNodeNumberCmd(
                          ClientData clientData, Tcl_Interp *interp,
                          int argc, USECONST char **argv);

static int BddscoutListFormulaeByNodeMaxLevelCmd(
                          ClientData clientData, Tcl_Interp *interp,
                          int argc, USECONST char **argv);

static int BddscoutListFormulaeByNodeAvgLevelCmd(
                          ClientData clientData, Tcl_Interp *interp,
                          int argc, USECONST char **argv);

static int BddscoutListFormulaeByPathNumberCmd(
                          ClientData clientData, Tcl_Interp *interp,
                          int argc, USECONST char **argv);

static int BddscoutListFormulaeByMintermNumberCmd(
                          ClientData clientData, Tcl_Interp *interp,
                          int argc, USECONST char **argv);

static int BddscoutReadBDDCmd(
                          ClientData clientData, Tcl_Interp *interp,
                          int argc, USECONST char **argv);

static int BddscoutReadBFCmd(
                          ClientData clientData, Tcl_Interp *interp,
                          int argc, USECONST char **argv);

static int BddscoutConstructBDDCmd(
                          ClientData clientData, Tcl_Interp *interp,
                          int argc, USECONST char **argv);

static int BddscoutCheckFormulaCmd(
                          ClientData clientData, Tcl_Interp *interp,
                          int argc, USECONST char **argv);

static int BddscoutCopyFormulaCmd(
                          ClientData clientData, Tcl_Interp *interp,
                          int argc, USECONST char **argv);

static int BddscoutWriteBDDCmd(
                          ClientData clientData, Tcl_Interp *interp,
                          int argc, USECONST char **argv);

static int BddscoutWriteBddviewCmd(
                          ClientData clientData, Tcl_Interp *interp,
                          int argc, USECONST char **argv);

static int BddscoutSwapWithHigherCmd(
                          ClientData clientData, Tcl_Interp *interp,
                          int argc, USECONST char **argv);

static int BddscoutSwapWithLowerCmd(
                          ClientData clientData, Tcl_Interp *interp,
                          int argc, USECONST char **argv);

static int BddscoutSiftingCmd(
                          ClientData clientData, Tcl_Interp *interp,
                          int argc, USECONST char **argv);

static int BddscoutSiftingOnFunctionCmd(
                          ClientData clientData, Tcl_Interp *interp,
                          int argc, USECONST char **argv);

static int BiddyVariableNumCmd(
                          ClientData clientData, Tcl_Interp *interp,
                          int argc, USECONST char **argv);

static int BiddyTableSizeCmd(
                          ClientData clientData, Tcl_Interp *interp,
                          int argc, USECONST char **argv);

static int BiddyTableMaxCmd(
                          ClientData clientData, Tcl_Interp *interp,
                          int argc, USECONST char **argv);

static int BiddyTableNumberCmd(
                          ClientData clientData, Tcl_Interp *interp,
                          int argc, USECONST char **argv);

static int BiddyVariableNumberCmd(
                          ClientData clientData, Tcl_Interp *interp,
                          int argc, USECONST char **argv);

static int BiddyGarbageNumberCmd(
                          ClientData clientData, Tcl_Interp *interp,
                          int argc, USECONST char **argv);

static int BiddyTableGeneratedCmd(
                          ClientData clientData, Tcl_Interp *interp,
                          int argc, USECONST char **argv);

static int BiddyBlockNumberCmd(
                          ClientData clientData, Tcl_Interp *interp,
                          int argc, USECONST char **argv);

static int BiddySwapNumberCmd(
                          ClientData clientData, Tcl_Interp *interp,
                          int argc, USECONST char **argv);

static int BiddySiftingNumberCmd(
                          ClientData clientData, Tcl_Interp *interp,
                          int argc, USECONST char **argv);

static int BiddyListUsedCmd(
                          ClientData clientData, Tcl_Interp *interp,
                          int argc, USECONST char **argv);

static int BiddyListMaxLengthCmd(
                          ClientData clientData, Tcl_Interp *interp,
                          int argc, USECONST char **argv);

static int BiddyListAvgLengthCmd(
                          ClientData clientData, Tcl_Interp *interp,
                          int argc, USECONST char **argv);

static int BiddyOPCacheSearchCmd(
                          ClientData clientData, Tcl_Interp *interp,
                          int argc, USECONST char **argv);

static int BiddyOPCacheFindCmd(
                          ClientData clientData, Tcl_Interp *interp,
                          int argc, USECONST char **argv);

static int BiddyOPCacheOverwriteCmd(
                          ClientData clientData, Tcl_Interp *interp,
                          int argc, USECONST char **argv);

#ifdef __cplusplus
extern "C" {
#endif

EXTERN int
Bddscout_Init(Tcl_Interp *interp)
{

#ifdef USE_TCL_STUBS
  if (Tcl_InitStubs(interp, "8.1", 0) == NULL) {
    return TCL_ERROR;
  }
#endif

  Tcl_CreateCommand(interp, "bddscout_initPkg", BddscoutInitPkgCmd,
                     (ClientData) NULL, (Tcl_CmdDeleteProc *) NULL);

  Tcl_CreateCommand(interp, "bddscout_exitPkg", BddscoutExitPkgCmd,
                     (ClientData) NULL, (Tcl_CmdDeleteProc *) NULL);

  Tcl_CreateCommand(interp, "bddscout_aboutPkg", BddscoutAboutPkgCmd,
                     (ClientData) NULL, (Tcl_CmdDeleteProc *) NULL);

  Tcl_CreateCommand(interp, "bddscout_changeBddType", BddscoutChangeBddTypeCmd,
                     (ClientData) NULL, (Tcl_CmdDeleteProc *) NULL);

  Tcl_CreateCommand(interp, "bddscout_parseInfixInput",
                     BddscoutParseInfixInputCmd,
                     (ClientData) NULL, (Tcl_CmdDeleteProc *) NULL);

  Tcl_CreateCommand(interp, "bddscout_listVariablesByPositon",
                     BddscoutListVariablesByPositionCmd,
                     (ClientData) NULL, (Tcl_CmdDeleteProc *) NULL);

  Tcl_CreateCommand(interp, "bddscout_listVariablesByName",
                     BddscoutListVariablesByNameCmd,
                     (ClientData) NULL, (Tcl_CmdDeleteProc *) NULL);

  Tcl_CreateCommand(interp, "bddscout_listVariablesByOrder",
                     BddscoutListVariablesByOrderCmd,
                     (ClientData) NULL, (Tcl_CmdDeleteProc *) NULL);

  Tcl_CreateCommand(interp, "bddscout_listVariablesByNumber",
                     BddscoutListVariablesByNumberCmd,
                     (ClientData) NULL, (Tcl_CmdDeleteProc *) NULL);

  Tcl_CreateCommand(interp, "bddscout_listFormulaeByName",
                     BddscoutListFormulaeByNameCmd,
                     (ClientData) NULL, (Tcl_CmdDeleteProc *) NULL);

  Tcl_CreateCommand(interp, "bddscout_listFormulaeByNodeNumber",
                     BddscoutListFormulaeByNodeNumberCmd,
                     (ClientData) NULL, (Tcl_CmdDeleteProc *) NULL);

  Tcl_CreateCommand(interp, "bddscout_listFormulaeByNodeMaxLevel",
                     BddscoutListFormulaeByNodeMaxLevelCmd,
                     (ClientData) NULL, (Tcl_CmdDeleteProc *) NULL);

  Tcl_CreateCommand(interp, "bddscout_listFormulaeByNodeAvgLevel",
                     BddscoutListFormulaeByNodeAvgLevelCmd,
                     (ClientData) NULL, (Tcl_CmdDeleteProc *) NULL);

  Tcl_CreateCommand(interp, "bddscout_listFormulaeByPathNumber",
                     BddscoutListFormulaeByPathNumberCmd,
                     (ClientData) NULL, (Tcl_CmdDeleteProc *) NULL);

  Tcl_CreateCommand(interp, "bddscout_listFormulaeByMintermNumber",
                     BddscoutListFormulaeByMintermNumberCmd,
                     (ClientData) NULL, (Tcl_CmdDeleteProc *) NULL);

  Tcl_CreateCommand(interp, "bddscout_readBDD", BddscoutReadBDDCmd,
                     (ClientData) NULL, (Tcl_CmdDeleteProc *) NULL);

  Tcl_CreateCommand(interp, "bddscout_readBF", BddscoutReadBFCmd,
                     (ClientData) NULL, (Tcl_CmdDeleteProc *) NULL);

  Tcl_CreateCommand(interp, "bddscout_constructBDD", BddscoutConstructBDDCmd,
                     (ClientData) NULL, (Tcl_CmdDeleteProc *) NULL);

  Tcl_CreateCommand(interp, "bddscout_checkFormula", BddscoutCheckFormulaCmd,
                     (ClientData) NULL, (Tcl_CmdDeleteProc *) NULL);

  Tcl_CreateCommand(interp, "bddscout_copyFormula", BddscoutCopyFormulaCmd,
                     (ClientData) NULL, (Tcl_CmdDeleteProc *) NULL);

  Tcl_CreateCommand(interp, "bddscout_writeBDD", BddscoutWriteBDDCmd,
                     (ClientData) NULL, (Tcl_CmdDeleteProc *) NULL);

  Tcl_CreateCommand(interp, "bddscout_writeBDDview", BddscoutWriteBddviewCmd,
                     (ClientData) NULL, (Tcl_CmdDeleteProc *) NULL);

  Tcl_CreateCommand(interp, "bddscout_swap_with_higher", BddscoutSwapWithHigherCmd,
                     (ClientData) NULL, (Tcl_CmdDeleteProc *) NULL);

  Tcl_CreateCommand(interp, "bddscout_swap_with_lower", BddscoutSwapWithLowerCmd,
                     (ClientData) NULL, (Tcl_CmdDeleteProc *) NULL);

  Tcl_CreateCommand(interp, "bddscout_sifting", BddscoutSiftingCmd,
                     (ClientData) NULL, (Tcl_CmdDeleteProc *) NULL);

  Tcl_CreateCommand(interp, "bddscout_sifting_on_function", BddscoutSiftingOnFunctionCmd,
                     (ClientData) NULL, (Tcl_CmdDeleteProc *) NULL);

  Tcl_CreateCommand(interp, "biddy_variable_num", BiddyVariableNumCmd,
                     (ClientData) NULL, (Tcl_CmdDeleteProc *) NULL);

  Tcl_CreateCommand(interp, "biddy_table_size", BiddyTableSizeCmd,
                     (ClientData) NULL, (Tcl_CmdDeleteProc *) NULL);

  Tcl_CreateCommand(interp, "biddy_table_max", BiddyTableMaxCmd,
                     (ClientData) NULL, (Tcl_CmdDeleteProc *) NULL);

  Tcl_CreateCommand(interp, "biddy_table_number", BiddyTableNumberCmd,
                     (ClientData) NULL, (Tcl_CmdDeleteProc *) NULL);

  Tcl_CreateCommand(interp, "biddy_variable_number", BiddyVariableNumberCmd,
                     (ClientData) NULL, (Tcl_CmdDeleteProc *) NULL);

  Tcl_CreateCommand(interp, "biddy_garbage_number", BiddyGarbageNumberCmd,
                     (ClientData) NULL, (Tcl_CmdDeleteProc *) NULL);

  Tcl_CreateCommand(interp, "biddy_table_generated", BiddyTableGeneratedCmd,
                     (ClientData) NULL, (Tcl_CmdDeleteProc *) NULL);

  Tcl_CreateCommand(interp, "biddy_block_number", BiddyBlockNumberCmd,
                     (ClientData) NULL, (Tcl_CmdDeleteProc *) NULL);

  Tcl_CreateCommand(interp, "biddy_swap_number", BiddySwapNumberCmd,
                     (ClientData) NULL, (Tcl_CmdDeleteProc *) NULL);

  Tcl_CreateCommand(interp, "biddy_sifting_number", BiddySiftingNumberCmd,
                     (ClientData) NULL, (Tcl_CmdDeleteProc *) NULL);

  Tcl_CreateCommand(interp, "biddy_list_used", BiddyListUsedCmd,
                     (ClientData) NULL, (Tcl_CmdDeleteProc *) NULL);

  Tcl_CreateCommand(interp, "biddy_list_max_length", BiddyListMaxLengthCmd,
                     (ClientData) NULL, (Tcl_CmdDeleteProc *) NULL);

  Tcl_CreateCommand(interp, "biddy_list_avg_length", BiddyListAvgLengthCmd,
                     (ClientData) NULL, (Tcl_CmdDeleteProc *) NULL);

  Tcl_CreateCommand(interp, "biddy_cache_ITE_search", BiddyOPCacheSearchCmd,
                     (ClientData) NULL, (Tcl_CmdDeleteProc *) NULL);

  Tcl_CreateCommand(interp, "biddy_cache_ITE_find", BiddyOPCacheFindCmd,
                     (ClientData) NULL, (Tcl_CmdDeleteProc *) NULL);

  Tcl_CreateCommand(interp, "biddy_cache_ITE_overwrite", BiddyOPCacheOverwriteCmd,
                     (ClientData) NULL, (Tcl_CmdDeleteProc *) NULL);

#ifdef USE_TCL_STUBS
  return Tcl_PkgProvideEx(interp, "bddscout-lib", "1.0", &bddscoutStubs);
#else
  return Tcl_PkgProvide(interp, "bddscout-lib", "1.0");
#endif
}

#ifdef __cplusplus
}
#endif

static int
BddscoutInitPkgCmd(ClientData clientData, Tcl_Interp *interp, int argc,
                   USECONST char **argv)
{
  if (argc != 1) {
    Tcl_SetResult(interp, (char *) "wrong # args", TCL_STATIC);
    return TCL_ERROR;
  }

  Biddy_InitMNG(&MNGROBDD,BIDDYTYPEOBDD);
  Biddy_InitMNG(&MNGZBDD,BIDDYTYPEZBDD);
  Biddy_InitMNG(&MNGTZBDD,BIDDYTYPETZBDD);
  MNGACTIVE = MNGROBDD;

  /* USE THIS TO USE ANONYMOUS MANAGER */
  /*
  MNGACTIVE = NULL;
  Biddy_InitAnonymous(BIDDYTYPEOBDD);
  */

  Tcl_SetResult(interp, (char *) "", TCL_STATIC);
  return TCL_OK;
}

static int
BddscoutExitPkgCmd(ClientData clientData, Tcl_Interp *interp, int argc,
                   USECONST char **argv)
{
  if (argc != 1) {
    Tcl_SetResult(interp, (char *) "wrong # args", TCL_STATIC);
    return TCL_ERROR;
  }

  Biddy_ExitMNG(&MNGROBDD);
  Biddy_ExitMNG(&MNGZBDD);
  Biddy_ExitMNG(&MNGTZBDD);

  if (!MNGACTIVE) Biddy_Exit();

  Tcl_SetResult(interp, (char *) "", TCL_STATIC);
  return TCL_OK;
}

static int
BddscoutAboutPkgCmd(ClientData clientData, Tcl_Interp *interp, int argc,
                    USECONST char **argv)
{
  char *bv;

  if (argc != 1) {
    Tcl_SetResult(interp, (char *) "wrong # args", TCL_STATIC);
    return TCL_ERROR;
  }

  printf("This is BDD Scout library");
  bv = Biddy_About();
  printf("Bdd package (Biddy v%s)",bv);

  free(bv);

  printf("\n");
  Tcl_SetResult(interp, (char *) "", TCL_STATIC);
  return TCL_OK;
}

static int BddscoutChangeBddTypeCmd(ClientData clientData, Tcl_Interp *interp,
                    int argc, USECONST char **argv)
{
  Biddy_String type;

  if (argc != 2) {
    Tcl_SetResult(interp, (char *) "wrong # args", TCL_STATIC);
    return TCL_ERROR;
  }

  type = strdup(argv[1]);

  if (!strcmp(type,"BIDDYTYPEOBDD")) {
    MNGACTIVE =  MNGROBDD;
  }
  else if (!strcmp(type,"BIDDYTYPEZBDD")) {
    MNGACTIVE =  MNGZBDD;
  }
  else if (!strcmp(type,"BIDDYTYPETZBDD")) {
    MNGACTIVE =  MNGTZBDD;
  }

  Tcl_SetResult(interp, (char *) "", TCL_STATIC);
  free(type);

  return TCL_OK;
}

static int BddscoutParseInfixInputCmd(ClientData clientData, Tcl_Interp *interp,
                    int argc, USECONST char **argv)
{
  Biddy_String expr,name,form;
  unsigned int n,n1,n2,n3,n4;
  Biddy_Edge tmp,bdd;

  if (argc != 2) {
    Tcl_SetResult(interp, (char *) "wrong # args", TCL_STATIC);
    return TCL_ERROR;
  }

  expr = strdup(argv[1]);

  /* DEBUGGING */
  /*
  printf("PARSE: <%s>\n",expr);
  */

  name = NULL;
  form = NULL;
  if (strcmp(expr,"")) {

    n1 = (unsigned int) strspn(expr," ");
    n2 = (unsigned int) strcspn(&expr[n1]," =");
    n3 = (unsigned int) strspn(&expr[n1+n2]," ");

    if (expr[n1] != '=') {
      if ( (n1+n2+n3) < strlen(expr) ) {
        if (expr[n1+n2+n3] == '=') {
          expr[n1+n2] = 0;
          name = strdup(&expr[n1]);
          n4 = (unsigned int) strspn(&expr[n1+n2+n3+1]," ");
          form = strdup(&expr[n1+n2+n3+1+n4]);
        } else {
          /* name = strdup("NONAME"); */
          form = strdup(&expr[n1]);
        }
      } else {
        /* name = strdup("NONAME"); */
        form = strdup(&expr[n1]);
      }
    } else {
      /* name = strdup("NONAME"); */
      n4 = (unsigned int) strspn(&expr[n1+1]," ");
      form = strdup(&expr[n1+1+n4]);
    }

    if (!name) {
      n = Biddy_Managed_FormulaTableNum(MNGACTIVE);
      printf("NUM: %u\n",n);
      name = strdup("F12345678"); /* max is 8-digit number */
      sprintf(name,"F%u",n);
    }

    /* DEBUGGING */
    /*
    printf("NAME: <%s>\n",name);
    printf("FORM: <%s>\n",form);
    */

    bdd = Biddy_Managed_Eval2(MNGACTIVE,form);
    if (Biddy_Managed_FindFormula(MNGACTIVE,name,&tmp)) {
      if (bdd != tmp) {
        Biddy_Managed_AddPersistentFormula(MNGACTIVE,name,bdd);
      }
    } else {
      Biddy_Managed_AddPersistentFormula(MNGACTIVE,name,bdd);
    }

    /* ADAPT FORMULA WITH THE SAME NAME IN OTHER MANAGERS */
    /* KNOWN MANAGERS: MNGROBDD, MNGZBDD, MNGTZBDD */

    if (MNGACTIVE != MNGROBDD) {
      if (Biddy_Managed_FindFormula(MNGROBDD,name,&tmp)) {
        bdd = Biddy_Managed_Eval2(MNGROBDD,form); /* TO DO: use convert instread of eval2 */
        if (bdd != tmp) {
          Biddy_Managed_AddPersistentFormula(MNGROBDD,name,bdd);
        }
      }
    }

    if (MNGACTIVE != MNGZBDD) {
      if (Biddy_Managed_FindFormula(MNGZBDD,name,&tmp)) {
        bdd = Biddy_Managed_Eval2(MNGZBDD,form); /* TO DO: use convert instread of eval2 */
        if (bdd != tmp) {
          Biddy_Managed_AddPersistentFormula(MNGZBDD,name,bdd);
        }
      }
    }

    if (MNGACTIVE != MNGTZBDD) {
      if (Biddy_Managed_FindFormula(MNGTZBDD,name,&tmp)) {
        bdd = Biddy_Managed_Eval2(MNGTZBDD,form); /* TO DO: use convert instread of eval2 */
        if (bdd != tmp) {
          Biddy_Managed_AddPersistentFormula(MNGTZBDD,name,bdd);
        }
      }
    }

  }

  Tcl_SetResult(interp, name, TCL_VOLATILE);

  if (name) free(name);
  if (form) free(form);
  free(expr);

  return TCL_OK;
}

static int
BddscoutListVariablesByPositionCmd(ClientData clientData, Tcl_Interp *interp,
                    int argc, USECONST char **argv)
{
  Biddy_String list;

  list = strdup("");

  BddscoutListVariablesByPosition(&list);

  Tcl_SetResult(interp, list, TCL_VOLATILE);
  free(list);

  return TCL_OK;
}

static int
BddscoutListVariablesByNameCmd(ClientData clientData, Tcl_Interp *interp,
                    int argc, USECONST char **argv)
{
  Biddy_String list;

  list = strdup("");

  BddscoutListVariablesByName(&list);

  Tcl_SetResult(interp, list, TCL_VOLATILE);
  free(list);

  return TCL_OK;
}

static int
BddscoutListVariablesByOrderCmd(ClientData clientData, Tcl_Interp *interp,
                    int argc, USECONST char **argv)
{
  Biddy_String list;

  list = strdup("");

  BddscoutListVariablesByOrder(&list);

  Tcl_SetResult(interp, list, TCL_VOLATILE);
  free(list);

  return TCL_OK;
}

static int
BddscoutListVariablesByNumberCmd(ClientData clientData, Tcl_Interp *interp,
                    int argc, USECONST char **argv)
{
  Biddy_String list;

  list = strdup("");

  BddscoutListVariablesByNumber(&list);

  Tcl_SetResult(interp, list, TCL_VOLATILE);
  free(list);

  return TCL_OK;
}

static int
BddscoutListFormulaeByNameCmd(ClientData clientData, Tcl_Interp *interp, int argc,
                    USECONST char **argv)
{
  Biddy_String list;

  list = strdup("NULL"); /* not required, but nice */

  BddscoutListFormulaByName(&list);

  Tcl_SetResult(interp, list, TCL_VOLATILE);
  free(list);

  return TCL_OK;
}

static int
BddscoutListFormulaeByNodeNumberCmd(ClientData clientData,
             Tcl_Interp *interp, int argc, USECONST char **argv)
{
  Biddy_String list;

  list = strdup("NULL"); /* required! */

  BddscoutListFormulaByUIntParameter(&list,MyNodeNumber);

  Tcl_SetResult(interp, list, TCL_VOLATILE);
  free(list);

  return TCL_OK;
}

static int
BddscoutListFormulaeByNodeMaxLevelCmd(ClientData clientData,
             Tcl_Interp *interp, int argc, USECONST char **argv)
{
  Biddy_String list;

  list = strdup("NULL"); /* required! */

  BddscoutListFormulaByUIntParameter(&list,MyNodeMaxLevel);

  Tcl_SetResult(interp, list, TCL_VOLATILE);
  free(list);

  return TCL_OK;
}

static int
BddscoutListFormulaeByNodeAvgLevelCmd(ClientData clientData,
             Tcl_Interp *interp, int argc, USECONST char **argv)
{
  Biddy_String list;

  list = strdup("NULL"); /* required! */

  BddscoutListFormulaByFloatParameter(&list,MyNodeAvgLevel);

  Tcl_SetResult(interp, list, TCL_VOLATILE);
  free(list);

  return TCL_OK;
}

static int
BddscoutListFormulaeByPathNumberCmd(ClientData clientData,
             Tcl_Interp *interp, int argc, USECONST char **argv)
{
  Biddy_String list;

  list = strdup("NULL"); /* required! */

  BddscoutListFormulaByUIntParameter(&list,MyPathNumber);

  Tcl_SetResult(interp, list, TCL_VOLATILE);
  free(list);

  return TCL_OK;
}

static int
BddscoutListFormulaeByMintermNumberCmd(ClientData clientData,
             Tcl_Interp *interp, int argc, USECONST char **argv)
{
  Biddy_String list;

  list = strdup("NULL"); /* required! */

  BddscoutListFormulaByFloatParameter(&list,MyMintermNumber);

  Tcl_SetResult(interp, list, TCL_VOLATILE);
  free(list);

  return TCL_OK;
}

static int
BddscoutReadBDDCmd(ClientData clientData, Tcl_Interp *interp, int argc,
                  USECONST char **argv)
{
  Biddy_String s1;
  FILE *funfile;
  Biddy_String name;

  if (argc != 2) {
    Tcl_SetResult(interp, (char *) "wrong # args", TCL_STATIC);
    return TCL_ERROR;
  }

  s1 = strdup(argv[1]);

  funfile = fopen(s1,"r");
  if (!funfile) {
    printf("BddscoutReadBDDCmd: File error (%s)!\n",s1);
    free(s1);
    return TCL_ERROR;
  }

  name = BddscoutReadBDD(funfile);

  fclose(funfile);

  free(s1);

  Tcl_SetResult(interp, name, TCL_VOLATILE);
  free(name);

  return TCL_OK;
}

static int
BddscoutReadBFCmd(ClientData clientData, Tcl_Interp *interp, int argc,
                  USECONST char **argv)
{
  Biddy_String s1;
  FILE *funfile;
  Biddy_String name;

  if (argc != 2) {
    Tcl_SetResult(interp, (char *) "wrong # args", TCL_STATIC);
    return TCL_ERROR;
  }

  s1 = strdup(argv[1]);

  funfile = fopen(s1,"r");
  if (!funfile) {
    printf("BddscoutReadBFCmd: File error (%s)!\n",s1);
    free(s1);
    return TCL_ERROR;
  }

  name = BddscoutParseFile(funfile);

  fclose(funfile);

  free(s1);

  Tcl_SetResult(interp, name, TCL_VOLATILE);
  free(name);

  return TCL_OK;
}

static int
BddscoutConstructBDDCmd(ClientData clientData, Tcl_Interp *interp, int argc,
                  USECONST char **argv)
{
  Biddy_String s1,s2,s3,s4;
  int numV,numN;

  if (argc != 5) {
    Tcl_SetResult(interp, (char *) "wrong # args", TCL_STATIC);
    return TCL_ERROR;
  }

  s1 = strdup(argv[1]); /* number of variables */
  s2 = strdup(argv[2]); /* list of variables */
  s3 = strdup(argv[3]); /* number of nodes */
  s4 = strdup(argv[4]); /* BDD */

  numV = atoi(s1);
  numN = atoi(s3);

  BddscoutConstructBDD(numV,s2,numN,s4);

  free(s1);
  free(s2);
  free(s3);
  free(s4);

  Tcl_SetResult(interp, (char *) "", TCL_STATIC);
  return TCL_OK;
}

static int
BddscoutCheckFormulaCmd(ClientData clientData, Tcl_Interp *interp, int argc,
                  USECONST char **argv)
{
  Biddy_String type,fname,r;
  Biddy_Manager mng;
  Biddy_Edge tmp;
  Biddy_Boolean OK;

  if (argc != 3) {
    Tcl_SetResult(interp, (char *) "wrong # args", TCL_STATIC);
    return TCL_ERROR;
  }

  type = strdup(argv[1]);
  fname = strdup(argv[2]);

  mng = NULL;
  if (!strcmp(type,"BIDDYTYPEOBDD")) {
    mng =  MNGROBDD;
  }
  else if (!strcmp(type,"BIDDYTYPEZBDD")) {
    mng =  MNGZBDD;
  }
  else if (!strcmp(type,"BIDDYTYPETZBDD")) {
    mng =  MNGTZBDD;
  }

  OK = FALSE;
  if (mng) {
    OK = Biddy_Managed_FindFormula(mng,fname,&tmp);
  }

  r = NULL;
  if (OK) {
    r = strdup("1");
  } else {
    r = strdup("0");
  }

  Tcl_SetResult(interp, r, TCL_VOLATILE);

  free(type);
  free(fname);
  free(r);

  return TCL_OK;
}

static int
BddscoutCopyFormulaCmd(ClientData clientData, Tcl_Interp *interp, int argc,
                  USECONST char **argv)
{
  Biddy_String fname,type1,type2;
  Biddy_Manager mng1,mng2;

  if (argc != 4) {
    Tcl_SetResult(interp, (char *) "wrong # args", TCL_STATIC);
    return TCL_ERROR;
  }

  fname = strdup(argv[1]);
  type1 = strdup(argv[2]);
  type2 = strdup(argv[3]);

  mng1 = NULL;
  if (!strcmp(type1,"BIDDYTYPEOBDD")) {
    mng1 =  MNGROBDD;
  }
  else if (!strcmp(type1,"BIDDYTYPEZBDD")) {
    mng1 =  MNGZBDD;
  }
  else if (!strcmp(type1,"BIDDYTYPETZBDD")) {
    mng1 =  MNGTZBDD;
  }

  mng2 = NULL;
  if (!strcmp(type2,"BIDDYTYPEOBDD")) {
    mng2 =  MNGROBDD;
  }
  else if (!strcmp(type2,"BIDDYTYPEZBDD")) {
    mng2 =  MNGZBDD;
  }
  else if (!strcmp(type2,"BIDDYTYPETZBDD")) {
    mng2 =  MNGTZBDD;
  }

  if (mng1 && mng2) {
    Biddy_Managed_CopyFormula(mng1,mng2,fname);
  }

  free(fname);
  free(type1);
  free(type2);

  Tcl_SetResult(interp, (char *) "", TCL_STATIC);
  return TCL_OK;
}

static int
BddscoutWriteBDDCmd(ClientData clientData, Tcl_Interp *interp, int argc,
                  USECONST char **argv)
{
  Biddy_String s1,s2;

  if (argc != 3) {
    Tcl_SetResult(interp, (char *) "wrong # args", TCL_STATIC);
    return TCL_ERROR;
  }

  s1 = strdup(argv[1]);
  s2 = strdup(argv[2]);

  BddscoutWriteBDD(s1,s2);

  free(s1);
  free(s2);

  Tcl_SetResult(interp, (char *) "", TCL_STATIC);
  return TCL_OK;
}

static int
BddscoutWriteBddviewCmd(ClientData clientData, Tcl_Interp *interp, int argc,
                  USECONST char **argv)
{
  Biddy_String s1,s2,s2x,s3;
  int OK;

#if defined(MINGW) || defined(_MSC_VER)
  char* docdir;
  char* appdir;
  docdir = getenv("USERPROFILE");
  appdir = strdup("\\AppData\\Local\\");
#endif

  if (argc != 4) {
    Tcl_SetResult(interp, (char *) "wrong # args", TCL_STATIC);
    return TCL_ERROR;
  }

  s1 = strdup(argv[1]);
  s2 = strdup(argv[2]);
  s3 = strdup(argv[3]);

#if defined(MINGW) || defined(_MSC_VER)
  s2x = (Biddy_String) malloc(strlen(docdir)+strlen(appdir)+strlen(s2)+1);
  if (!s2x) return TCL_ERROR;
  s2x[0] = 0;
  strcat(s2x,docdir);
  strcat(s2x,appdir);
  strcat(s2x,s2);
  free(appdir);
#elif UNIX
  s2x = (Biddy_String) malloc(strlen(s2)+6);
  s2x[0] = 0;
  strcat(s2x,"/tmp/");
  strcat(s2x,s2);
#else
  s2x = strdup(s2);
#endif

  OK = BddscoutWriteBddview(s2x,s3,s1);

  free(s1);
  free(s2);
  free(s3);

  if (!OK) {
    printf("BddscoutWriteBddview error\n");
  }

  Tcl_SetResult(interp, s2x, TCL_VOLATILE);
  free(s2x);

  return TCL_OK;
}

static int
BddscoutSwapWithHigherCmd(ClientData clientData, Tcl_Interp *interp, int argc,
                  USECONST char **argv)
{
  Biddy_String s1;
  Biddy_Variable v;
  Biddy_Boolean find;

  if (argc != 2) {
    Tcl_SetResult(interp, (char *) "wrong # args", TCL_STATIC);
    return TCL_ERROR;
  }

  s1 = strdup(argv[1]);
  v = 0;
  find = FALSE;
  while (!find && v<Biddy_Managed_VariableTableNum(MNGACTIVE)) {
    if (!strcmp(s1,Biddy_Managed_GetVariableName(MNGACTIVE,v))) {
      find = TRUE;
    } else {
      v++;
    }
  }
  free(s1);

  /* IF THE ELEMENT WAS NOT FOUND... */
  if (find) {
    Biddy_Managed_SwapWithHigher(MNGACTIVE,v);
  }

  Tcl_SetResult(interp, (char *) "", TCL_STATIC);
  return TCL_OK;
}

static int
BddscoutSwapWithLowerCmd(ClientData clientData, Tcl_Interp *interp, int argc,
                  USECONST char **argv)
{
  Biddy_String s1;
  Biddy_Variable v;
  Biddy_Boolean find;

  if (argc != 2) {
    Tcl_SetResult(interp, (char *) "wrong # args", TCL_STATIC);
    return TCL_ERROR;
  }

  s1 = strdup(argv[1]);
  v = 0;
  find = FALSE;
  while (!find && v<Biddy_Managed_VariableTableNum(MNGACTIVE)) {
    if (!strcmp(s1,Biddy_Managed_GetVariableName(MNGACTIVE,v))) {
      find = TRUE;
    } else {
      v++;
    }
  }
  free(s1);

  /* IF THE ELEMENT WAS NOT FOUND... */
  if (find) {
    Biddy_Managed_SwapWithLower(MNGACTIVE,v);
  }

  Tcl_SetResult(interp, (char *) "", TCL_STATIC);
  return TCL_OK;
}

static int
BddscoutSiftingCmd(ClientData clientData, Tcl_Interp *interp, int argc,
                  USECONST char **argv)
{
  if (argc != 1) {
    Tcl_SetResult(interp, (char *) "wrong # args", TCL_STATIC);
    return TCL_ERROR;
  }

  if (Biddy_Managed_GetManagerType(MNGACTIVE) == BIDDYTYPEOBDD) {
    Biddy_Managed_Sifting(MNGACTIVE,NULL,FALSE);
  }

  Tcl_SetResult(interp, (char *) "", TCL_STATIC);
  return TCL_OK;

}

static int
BddscoutSiftingOnFunctionCmd(ClientData clientData, Tcl_Interp *interp, int argc,
                  USECONST char **argv)
{
  Biddy_String s1;
  Biddy_Edge f;

  if (argc != 2) {
    Tcl_SetResult(interp, (char *) "wrong # args", TCL_STATIC);
    return TCL_ERROR;
  }

  s1 = strdup(argv[1]);

  if (Biddy_Managed_GetManagerType(MNGACTIVE) == BIDDYTYPEOBDD) {
    if (Biddy_Managed_FindFormula(MNGACTIVE,s1,&f)) {
      Biddy_Managed_Sifting(MNGACTIVE,f,FALSE);
    }
  }

  free(s1);

  Tcl_SetResult(interp, (char *) "", TCL_STATIC);
  return TCL_OK;

}

static int
BiddyVariableNumCmd(ClientData clientData, Tcl_Interp *interp, int argc,
                  USECONST char **argv)
{
  Biddy_String sup;

  if (argc != 1) {
    Tcl_SetResult(interp, (char *) "wrong # args", TCL_STATIC);
    return TCL_ERROR;
  }

  sup = (Biddy_String) malloc(127);
  if (!sup) return TCL_ERROR;
  sprintf(sup,"%u",Biddy_Managed_VariableTableNum(MNGACTIVE));
  Tcl_SetResult(interp, sup, TCL_VOLATILE);
  free(sup);

  return TCL_OK;
}

static int
BiddyTableSizeCmd(ClientData clientData, Tcl_Interp *interp, int argc,
                  USECONST char **argv)
{
  Biddy_String sup;

  if (argc != 1) {
    Tcl_SetResult(interp, (char *) "wrong # args", TCL_STATIC);
    return TCL_ERROR;
  }

  sup = (Biddy_String) malloc(127);
  if (!sup) return TCL_ERROR;
  sprintf(sup,"%u",Biddy_Managed_NodeTableSize(MNGACTIVE));
  Tcl_SetResult(interp, sup, TCL_VOLATILE);
  free(sup);

  return TCL_OK;
}

static int
BiddyTableMaxCmd(ClientData clientData, Tcl_Interp *interp, int argc,
                  USECONST char **argv)
{
  Biddy_String sup;

  if (argc != 1) {
    Tcl_SetResult(interp, (char *) "wrong # args", TCL_STATIC);
    return TCL_ERROR;
  }

  sup = (Biddy_String) malloc(127);
  if (!sup) return TCL_ERROR;
  sprintf(sup,"%u",Biddy_Managed_NodeTableMax(MNGACTIVE));
  Tcl_SetResult(interp, sup, TCL_VOLATILE);
  free(sup);

  return TCL_OK;
}

static int
BiddyTableNumberCmd(ClientData clientData, Tcl_Interp *interp, int argc,
                  USECONST char **argv)
{
  Biddy_String sup;

  if (argc != 1) {
    Tcl_SetResult(interp, (char *) "wrong # args", TCL_STATIC);
    return TCL_ERROR;
  }

  sup = (Biddy_String) malloc(127);
  if (!sup) return TCL_ERROR;
  sprintf(sup,"%u",Biddy_Managed_NodeTableNum(MNGACTIVE));
  Tcl_SetResult(interp, sup, TCL_VOLATILE);
  free(sup);

  return TCL_OK;
}

static int
BiddyVariableNumberCmd(ClientData clientData, Tcl_Interp *interp, int argc,
                  USECONST char **argv)
{
  Biddy_String s1;
  Biddy_Variable v;
  Biddy_Boolean find;
  Biddy_String sup;

  if (argc != 2) {
    Tcl_SetResult(interp, (char *) "wrong # args", TCL_STATIC);
    return TCL_ERROR;
  }

  s1 = strdup(argv[1]);
  v = 0;
  find = FALSE;
  while (!find && v<Biddy_Managed_VariableTableNum(MNGACTIVE)) {
    if (!strcmp(s1,Biddy_Managed_GetVariableName(MNGACTIVE,v))) {
      find = TRUE;
    } else {
      v++;
    }
  }
  free(s1);

  /* IF THE ELEMENT WAS NOT FOUND... */
  if (!find) {
    sup = strdup("0");
  } else {
    sup = (Biddy_String) malloc(127);
    if (!sup) return TCL_ERROR;
    sprintf(sup,"%u",Biddy_Managed_NodeTableNumVar(MNGACTIVE,v));
  }

  Tcl_SetResult(interp, sup, TCL_VOLATILE);
  free(sup);

  return TCL_OK;
}

static int
BiddyGarbageNumberCmd(ClientData clientData, Tcl_Interp *interp, int argc,
                  USECONST char **argv)
{
  Biddy_String sup;

  if (argc != 1) {
    Tcl_SetResult(interp, (char *) "wrong # args", TCL_STATIC);
    return TCL_ERROR;
  }

  sup = (Biddy_String) malloc(127);
  if (!sup) return TCL_ERROR;
  sprintf(sup,"%u",Biddy_Managed_NodeTableGCNumber(MNGACTIVE));
  Tcl_SetResult(interp, sup, TCL_VOLATILE);
  free(sup);

  return TCL_OK;
}

static int
BiddyTableGeneratedCmd(ClientData clientData, Tcl_Interp *interp, int argc,
                  USECONST char **argv)
{
  Biddy_String sup;

  if (argc != 1) {
    Tcl_SetResult(interp, (char *) "wrong # args", TCL_STATIC);
    return TCL_ERROR;
  }

  sup = (Biddy_String) malloc(127);
  if (!sup) return TCL_ERROR;
  sprintf(sup,"%u",Biddy_Managed_NodeTableGenerated(MNGACTIVE));
  Tcl_SetResult(interp, sup, TCL_VOLATILE);
  free(sup);

  return TCL_OK;
}

static int
BiddyBlockNumberCmd(ClientData clientData, Tcl_Interp *interp, int argc,
                  USECONST char **argv)
{
  Biddy_String sup;

  if (argc != 1) {
    Tcl_SetResult(interp, (char *) "wrong # args", TCL_STATIC);
    return TCL_ERROR;
  }

  sup = (Biddy_String) malloc(127);
  if (!sup) return TCL_ERROR;
  sprintf(sup,"%u",Biddy_Managed_NodeTableBlockNumber(MNGACTIVE));
  Tcl_SetResult(interp, sup, TCL_VOLATILE);
  free(sup);

  return TCL_OK;
}

static int
BiddySwapNumberCmd(ClientData clientData, Tcl_Interp *interp, int argc,
                  USECONST char **argv)
{
  Biddy_String sup;

  if (argc != 1) {
    Tcl_SetResult(interp, (char *) "wrong # args", TCL_STATIC);
    return TCL_ERROR;
  }

  sup = (Biddy_String) malloc(127);
  if (!sup) return TCL_ERROR;
  sprintf(sup,"%u",Biddy_Managed_NodeTableSwapNumber(MNGACTIVE));
  Tcl_SetResult(interp, sup, TCL_VOLATILE);
  free(sup);

  return TCL_OK;
}

static int
BiddySiftingNumberCmd(ClientData clientData, Tcl_Interp *interp, int argc,
                  USECONST char **argv)
{
  Biddy_String sup;

  if (argc != 1) {
    Tcl_SetResult(interp, (char *) "wrong # args", TCL_STATIC);
    return TCL_ERROR;
  }

  sup = (Biddy_String) malloc(127);
  if (!sup) return TCL_ERROR;
  sprintf(sup,"%u",Biddy_Managed_NodeTableSiftingNumber(MNGACTIVE));
  Tcl_SetResult(interp, sup, TCL_VOLATILE);
  free(sup);

  return TCL_OK;
}

static int
BiddyListUsedCmd(ClientData clientData, Tcl_Interp *interp, int argc,
                  USECONST char **argv)
{
  Biddy_String sup;

  if (argc != 1) {
    Tcl_SetResult(interp, (char *) "wrong # args", TCL_STATIC);
    return TCL_ERROR;
  }

  sup = (Biddy_String) malloc(127);
  if (!sup) return TCL_ERROR;
  sprintf(sup,"%u",Biddy_Managed_ListUsed(MNGACTIVE));
  Tcl_SetResult(interp, sup, TCL_VOLATILE);
  free(sup);

  return TCL_OK;
}

static int
BiddyListMaxLengthCmd(ClientData clientData, Tcl_Interp *interp, int argc,
                  USECONST char **argv)
{
  Biddy_String sup;

  if (argc != 1) {
    Tcl_SetResult(interp, (char *) "wrong # args", TCL_STATIC);
    return TCL_ERROR;
  }

  sup = (Biddy_String) malloc(127);
  if (!sup) return TCL_ERROR;
  sprintf(sup,"%u",Biddy_Managed_ListMaxLength(MNGACTIVE));
  Tcl_SetResult(interp, sup, TCL_VOLATILE);
  free(sup);

  return TCL_OK;
}

static int
BiddyListAvgLengthCmd(ClientData clientData, Tcl_Interp *interp, int argc,
                  USECONST char **argv)
{
  Biddy_String sup;

  if (argc != 1) {
    Tcl_SetResult(interp, (char *) "wrong # args", TCL_STATIC);
    return TCL_ERROR;
  }

  sup = (Biddy_String) malloc(127);
  if (!sup) return TCL_ERROR;
  sprintf(sup,"%.2f",Biddy_Managed_ListAvgLength(MNGACTIVE));
  Tcl_SetResult(interp, sup, TCL_VOLATILE);
  free(sup);

  return TCL_OK;
}

static int
BiddyOPCacheSearchCmd(ClientData clientData, Tcl_Interp *interp, int argc,
                  USECONST char **argv)
{
  Biddy_String sup;

  if (argc != 1) {
    Tcl_SetResult(interp, (char *) "wrong # args", TCL_STATIC);
    return TCL_ERROR;
  }

  sup = (Biddy_String) malloc(127);
  if (!sup) return TCL_ERROR;

#if defined(MINGW) || defined(_MSC_VER)
  sprintf(sup,"%I64u",Biddy_Managed_OPCacheSearch(MNGACTIVE)); /* MINGW AND VS?? REQUIRE "%I64u" */
#else
  sprintf(sup,"%llu",Biddy_Managed_OPCacheSearch(MNGACTIVE));
#endif

  Tcl_SetResult(interp, sup, TCL_VOLATILE);
  free(sup);

  return TCL_OK;
}

static int
BiddyOPCacheFindCmd(ClientData clientData, Tcl_Interp *interp, int argc,
                  USECONST char **argv)
{
  Biddy_String sup;

  if (argc != 1) {
    Tcl_SetResult(interp, (char *) "wrong # args", TCL_STATIC);
    return TCL_ERROR;
  }

  sup = (Biddy_String) malloc(127);
  if (!sup) return TCL_ERROR;

#if defined(MINGW) || defined(_MSC_VER)
  sprintf(sup,"%I64u",Biddy_Managed_OPCacheFind(MNGACTIVE)); /* MINGW AND VS?? REQUIRE "%I64u" */
#else
  sprintf(sup,"%llu",Biddy_Managed_OPCacheFind(MNGACTIVE));
#endif

  Tcl_SetResult(interp, sup, TCL_VOLATILE);
  free(sup);

  return TCL_OK;
}

static int
BiddyOPCacheOverwriteCmd(ClientData clientData, Tcl_Interp *interp, int argc,
                  USECONST char **argv)
{
  Biddy_String sup;

  if (argc != 1) {
    Tcl_SetResult(interp, (char *) "wrong # args", TCL_STATIC);
    return TCL_ERROR;
  }

  sup = (Biddy_String) malloc(127);
  if (!sup) return TCL_ERROR;

#if defined(MINGW) || defined(_MSC_VER)
  sprintf(sup,"%I64u",Biddy_Managed_OPCacheOverwrite(MNGACTIVE)); /* MINGW AND VS?? REQUIRE "%I64u" */
#else
  sprintf(sup,"%llu",Biddy_Managed_OPCacheOverwrite(MNGACTIVE));
#endif

  Tcl_SetResult(interp, sup, TCL_VOLATILE);
  free(sup);

  return TCL_OK;
}

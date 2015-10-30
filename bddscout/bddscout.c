/**CFile*******************************************************************
  PackageName [Bdd Scout]
  Synopsis    [Bdd Scout]

  FileName    [bddscout.c]
  Revision    [$Revision: 101 $]
  Date        [$Date: 2015-10-06 15:51:07 +0200 (tor, 06 okt 2015) $]
  Authors     [Robert Meolic (robert.meolic@um.si)]
  Description []
  SeeAlso     [bddscout.h]

  Copyright   [This file is part of Bdd Scout package.
               Copyright (C) 2008, 2015 UM-FERI
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

/*-----------------------------------------------------------------------*/
/* External functions                                                    */
/*-----------------------------------------------------------------------*/

/*-----------------------------------------------------------------------*/
/* Internal functions                                                    */
/*-----------------------------------------------------------------------*/

/**Function****************************************************************
  Synopsis    [Function Bddscout_ListVariablesByPosition]
  Description [Bddscout_ListVariablesByPosition creates a list of all BDD
               variables sorted by position in Variable table.]
  SideEffects []
  SeeAlso     []
  ************************************************************************/

void
Bddscout_ListVariablesByPosition(Biddy_String *list)
{
  Biddy_Variable v;
  Biddy_String newlist;
  Biddy_String name;

  for (v=0; v<Biddy_VariableTableNum(); v++) {
    name = Biddy_GetVariableName(v);
    newlist = (Biddy_String) malloc(strlen(*list)+strlen(name)+2);
    sprintf(newlist,"%s %s",*list,name);
    free(*list);
    *list = newlist;
  }
}

/**Function****************************************************************
  Synopsis    [Function Bddscout_ListVariablesByName]
  Description [Bddscout_ListVariablesByName creates a list of all BDD
               variables sorted by name.]
  SideEffects [SORTING NOT IMPLEMENTED, YET]
  SeeAlso     []
  ************************************************************************/

void
Bddscout_ListVariablesByName(Biddy_String *list)
{
  Biddy_Variable v;
  Biddy_String newlist;
  Biddy_String name;

  for (v=0; v<Biddy_VariableTableNum(); v++) {
    name = Biddy_GetVariableName(v);
    newlist = (Biddy_String) malloc(strlen(*list)+strlen(name)+2);
    sprintf(newlist,"%s %s",*list,name);
    free(*list);
    *list = newlist;
  }
}

/**Function****************************************************************
  Synopsis    [Function Bddscout_ListVariablesByOrder]
  Description [Bddscout_ListVariablesByOrder creates a list of all BDD
               variables sorted by order in active ordering.]
  SideEffects [SORTING NOT IMPLEMENTED, YET]
  SeeAlso     []
  ************************************************************************/

void
Bddscout_ListVariablesByOrder(Biddy_String *list)
{
  Biddy_Variable v;
  Biddy_String newlist;
  Biddy_String name;

  for (v=0; v<Biddy_VariableTableNum(); v++) {
    name = Biddy_GetVariableName(v);
    newlist = (Biddy_String) malloc(strlen(*list)+strlen(name)+2);
    sprintf(newlist,"%s %s",*list,name);
    free(*list);
    *list = newlist;
  }
}

/**Function****************************************************************
  Synopsis    [Function Bddscout_ListVariablesByNumber]
  Description [Bddscout_ListVariablesByNumber creates a list of all BDD
               variables sorted by number of BDD nodes.]
  SideEffects [SORTING NOT IMPLEMENTED, YET]
  SeeAlso     []
  ************************************************************************/

void
Bddscout_ListVariablesByNumber(Biddy_String *list)
{
  Biddy_Variable v;
  Biddy_String newlist;
  Biddy_String name;

  for (v=0; v<Biddy_VariableTableNum(); v++) {
    name = Biddy_GetVariableName(v);
    newlist = (Biddy_String) malloc(strlen(*list)+strlen(name)+2);
    sprintf(newlist,"%s %s",*list,name);
    free(*list);
    *list = newlist;
  }
}

/**Function****************************************************************
  Synopsis    [Function Bddscout_ListFormulaByName]
  Description [Bddscout_ListFormulaByName creates a sorted list of all
               formulae in Biddy Formulae tree.]
  SideEffects []
  SeeAlso     []
  ************************************************************************/

void
Bddscout_ListFormulaByName(Biddy_String *list)
{
  Biddy_String newlist;
  Biddy_Edge formula;
  Biddy_String formulaname;
  unsigned int i;

  i = 0;
  formula = Biddy_GetIthFormula(i);
  formulaname = Biddy_GetIthFormulaName(i);
  while (formulaname) {
    if (!Biddy_IsNull(formula) &&
        strcmp(formulaname,"BIDDY_DELETED_FORMULAE") &&
        strcmp(formulaname,Biddy_GetTopVariableName(formula)))
    {
      if (i == 0) {
        free(*list);
        *list = strdup("");
      }
      newlist = (Biddy_String) malloc(strlen(*list)+strlen(formulaname)+2);
      sprintf(newlist,"%s %s",*list,formulaname);
      free(*list);
      *list = newlist;
    }

    i++;
    formula = Biddy_GetIthFormula(i);
    formulaname = Biddy_GetIthFormulaName(i);
  }
}

/**Function****************************************************************
  Synopsis    [Function Bddscout_ListFormulaByUIntParameter]
  Description [Bddscout_ListFormulaByUIntParameter creates a sorted list
               of all formulae in Biddy Formulae tree.
               Parameter is a function used for sorting.]
  SideEffects []
  SeeAlso     []
  ************************************************************************/

void
Bddscout_ListFormulaByUIntParameter(Biddy_String *list, unsigned int (*PF)(Biddy_Edge))
{
  typedef struct {
    Biddy_Edge t;
    Biddy_String name;
    unsigned int parameter;
  } FT;

  FT *sortingTable;
  int tableSize;
  Biddy_String newlist;
  Biddy_String number;
  Biddy_Edge formula;
  Biddy_String formulaname;
  int i,j;
  int n;

  tableSize = 0;
  sortingTable = NULL;
  i = 0;
  formula = Biddy_GetIthFormula(i);
  formulaname = Biddy_GetIthFormulaName(i);
  while (formulaname) {
    if (!Biddy_IsNull(formula) &&
        strcmp(formulaname,"BIDDY_DELETED_FORMULAE") &&
        strcmp(formulaname,Biddy_GetTopVariableName(formula)))
    {

      if (!strcmp(*list,"NULL")) {

        free(*list);
        *list = strdup("");
        sortingTable = (FT *) malloc(sizeof(FT));
        tableSize = 1;
        sortingTable[0].t = formula;
        sortingTable[0].name = formulaname;
        sortingTable[0].parameter = PF(formula);

      } else {

        tableSize++;
        sortingTable = (FT *) realloc(sortingTable,sizeof(FT)*tableSize);

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
    formula = Biddy_GetIthFormula(i);
    formulaname = Biddy_GetIthFormulaName(i);
  }

  for (i=0;i<tableSize;++i) {
    number = (Biddy_String) malloc(255);
    sprintf(number,"(%d)",sortingTable[i].parameter);
    newlist = (Biddy_String)
      malloc(strlen(*list)+ strlen(sortingTable[i].name)+strlen(number)+2);
    sprintf(newlist,"%s %s%s",*list,sortingTable[i].name,number);
    free(number);
    free(*list);
    *list = newlist;
  }
}

/**Function****************************************************************
  Synopsis    [Function Bddscout_ListFormulaByFloatParameter]
  Description [Bddscout_ListFormulaByFloatParameter creates a sorted list
               of all formulae in Biddy Formulae tree.
               Parameter is a function used for sorting.]
  SideEffects []
  SeeAlso     []
  ************************************************************************/

void
Bddscout_ListFormulaByFloatParameter(Biddy_String *list, float (*PF)(Biddy_Edge))
{
  typedef struct {
    Biddy_Edge t;
    Biddy_String name;
    float parameter;
  } FT;

  FT *sortingTable;
  int tableSize;
  Biddy_String newlist;
  Biddy_String number;
  Biddy_Edge formula;
  Biddy_String formulaname;
  int i,j;
  int n;

  tableSize = 0;
  sortingTable = NULL;
  i = 0;
  formula = Biddy_GetIthFormula(i);
  formulaname = Biddy_GetIthFormulaName(i);
  while (formulaname) {
    if (!Biddy_IsNull(formula) &&
        strcmp(formulaname,"BIDDY_DELETED_FORMULAE") &&
        strcmp(formulaname,Biddy_GetTopVariableName(formula)))
    {

      if (!strcmp(*list,"NULL")) {

        free(*list);
        *list = strdup("");
        sortingTable = (FT *) malloc(sizeof(FT));
        tableSize = 1;
        sortingTable[0].t = formula;
        sortingTable[0].name = formulaname;
        sortingTable[0].parameter = PF(formula);

      } else {

        tableSize++;
        sortingTable = (FT *) realloc(sortingTable,sizeof(FT)*tableSize);

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
    formula = Biddy_GetIthFormula(i);
    formulaname = Biddy_GetIthFormulaName(i);
  }

  for (i=0;i<tableSize;++i) {
    number = (Biddy_String) malloc(255);
    sprintf(number,"(%.2f)",sortingTable[i].parameter);
    newlist = (Biddy_String)
      malloc(strlen(*list)+ strlen(sortingTable[i].name)+strlen(number)+2);
    sprintf(newlist,"%s %s%s",*list,sortingTable[i].name,number);
    free(number);
    free(*list);
    *list = newlist;
  }
}

/*-----------------------------------------------------------------------*/
/* Other internal functions (mostly from 1995)                           */
/*-----------------------------------------------------------------------*/

/**Function****************************************************************
  Synopsis    [Function Bddscout_ParseFile]
  Description []
  SideEffects [Implemented long time ago (1995). Not comprehensible.]
  SeeAlso     []
  ************************************************************************/

#define readln(f,s) SCAN_RESULT = fscanf(f," %[^\n] ",s)

static Biddy_String makeFunction(FILE *f, Biddy_String line);

Biddy_String
Bddscout_ParseFile(FILE *f)
{
  Biddy_String line,name;
  Biddy_String s1;
  Biddy_String var;
  unsigned int n,x;

  line = (Biddy_String) malloc(4096);

  readln(f,line);
  x = strcspn(line,"=");
  if (x != strlen(line)) {
    printf("WARNING: Variable ordering not specified.\n");
  } else {
    s1 = strdup(line);
    readln(f,line);
    x = strcspn(line,"=");
    while (x == strlen(line))
    {
      n = strlen(s1)+strlen(line)+1+1;
      s1 = (Biddy_String) realloc(s1,n);
      strcat(s1," ");
      strcat(s1,line);
      readln(f,line);
      x = strcspn(line,"=");
    }

    var = strtok(&s1[1]," \t)");
    while (var) {

      /*
      printf("VARIABLE: <%s>\n",var);
      */

      Biddy_FoaVariable(var);
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

  if (!line) return NULL;

  x = strcspn(line, "=");
  if (x == strlen(line)) {
    s1 = NULL;                 /* s1 = left part of the expression */
    s2 = strdup(line);         /* s2 = right part of the expression */
  } else {
    s1 = (Biddy_String) strdup(line);
    s1[x] = 0; /* strndup is not supported everywhere */
    if (x != strlen(line)-1) {
      y = strspn(&line[x+1], " \t");
      s2 = strdup(&line[x+1+y]);
    } else {
      s2 = NULL;
    }
  }

  if (s1) {
    x = strspn(s1," \t");
    y = strcspn(&s1[x]," \t\n");
    name = (Biddy_String) strdup(&s1[x]);
    name[y] = 0; /* strndup is not supported everywhere */
    free(s1);
  } else {
    name = strdup("F");
  }

  if (!feof(f)) {
    readln(f,line);
    x = strcspn(line,"=");
  } else {
    line[0] = 0;
    line = NULL;
  }

  while (line && (x == strlen(line)))
  {
    if (s2 != NULL) {
      len = strlen(s2)+strlen(line)+1;
      s2 = (Biddy_String) realloc(s2, len+1);
      strcat(s2," ");
      strcat(s2,line);
    } else {
      s2 = strdup(line);
    }

    if (!feof(f)) {
      readln(f,line);
      x = strcspn(line, "=");
    } else {
      line[0] = 0;
      line = NULL;
    }
  }

  /*
  printf("FUNCTION NAME: <%s>\n",name);
  printf("FUNCTION DEF: <%s>\n",s2);
  */

  fun = Biddy_Eval1x(s2,(Biddy_LookupFunction) NULL);
  free(s2);

  if (!Biddy_IsNull(fun)) {
    Biddy_AddPersistentFormula(name,fun);
  }

  return name;
}

/**Function****************************************************************
  Synopsis    [Function Bddscout_WriteBDDView.]
  Description []
  SideEffects [Uses Biddy_WriteDot and graphviz package]
  SeeAlso     []
  ************************************************************************/

typedef struct {
  int n;
  void *p;
  int t;
} tNode;

typedef struct {
  int id;
  Biddy_String label;
  int x;
  int y;
} tableXY;

static void EnumerateNodes(Biddy_Edge f, tNode **tnode, int *t, int *n);
static void addNode(tNode **tnode, int n, void *p, int t);
static int findNode(tNode *tnode, int n, void *p, int t);
static void WriteDotConnections(FILE *funfile, Biddy_Edge f, tNode *tnode,
                                 int *t, int n);
static void ParseDot(FILE *dotfile, tableXY *table, int *xsize, int *ysize);

int
Bddscout_WriteBDDView(FILE *s, Biddy_String dotexe, Biddy_String name)
{
  FILE *xdotfile;
  int i,t,n;
  tNode *tn;
  tableXY *table;
  int xsize,ysize;
  Biddy_Edge f;
#ifndef WINDOWS
  pid_t childpid;
  int status;
#endif
  char* dotname;
  char* xdotname;
#ifdef WINDOWS
  char* docdir;
  char* appdir;
  docdir = getenv("USERPROFILE");
  appdir = strdup("\\AppData\\Local\\");
#endif

  if (!Biddy_FindFormula(name,&f)) {
    printf("Function %s does not exists!\n",name);
    return -1;
  } else {

#ifdef WINDOWS
    dotname = (Biddy_String) malloc(strlen(docdir)+strlen(appdir)+7+1);
    dotname[0] = 0;
    strcat(dotname,docdir);
    strcat(dotname,appdir);
    strcat(dotname,"bdd.dot");
    xdotname = (Biddy_String) malloc(strlen(docdir)+strlen(appdir)+8+1);
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

    n = 0;
    if (!Biddy_FindFormula(name,&f)) {
      printf("Bddscout_WriteBDDView: Function %s does not exists!\n",name);
      return(0);
    } else {
      n = Biddy_WriteDot(dotname,f,name);
    }

    if (n == 0) {
      printf("Bddscout_WriteBDDView: Problem with function %s!\n",name);
      return(0);
    }

#ifdef WINDOWS
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
      printf("Bddscout_WriteBDDView: File error (%s)!\n",xdotname);
      return -1;
    }

    table = (tableXY *) malloc((n+1) * sizeof(tableXY)); /* n = nodes, add one label */
    ParseDot(xdotfile,table,&xsize,&ysize);
    fclose(xdotfile);

    free(dotname);
    free(xdotname);

    t = 0;
    n = 0;
    tn = NULL;
    EnumerateNodes(f,&tn,&t,&n);
    Biddy_DeselectNode(Biddy_GetConstantOne()); /* needed for Biddy_NodeRepair */
    Biddy_NodeRepair(f);

    fprintf(s,"label %d %s %d %d\n",table[0].id,table[0].label,table[0].x,table[0].y);
    for (i=1;i<(n+1);++i) {
      if (tn[i-1].t == 0) {
        fprintf(s,"node %d %s %d %d\n",table[i].id,table[i].label,table[i].x,table[i].y);
      } else {
        fprintf(s,"terminal %d %s %d %d\n",table[i].id,table[i].label,table[i].x,table[i].y);
      }
      free(table[i].label);
    }
    free(table);

    fprintf(s,"connect 0 1 ");
    if (Biddy_GetMark(f)) {
      fprintf(s,"si\n");
    } else {
      fprintf(s,"s\n");
    }

    t = 1;
    WriteDotConnections(s,f,tn,&t,n); /* this will select all nodes except constant node */
    Biddy_NodeRepair(f);

    free(tn);

  }

  return 0;
}

static void
EnumerateNodes(Biddy_Edge f, tNode **tnode, int *t, int *n)
{
  if (!Biddy_IsSelected(f)) {
    Biddy_SelectNode(f);
    if (Biddy_IsConstant(f)) {
      (*n)++;
      (*t)++;
      addNode(tnode,(*n),Biddy_Regular(f),(*t));
    } else {
      (*n)++;
      addNode(tnode,(*n),Biddy_Regular(f),0);
      if (Biddy_IsConstant(Biddy_GetElse(f)) || Biddy_IsConstant(Biddy_GetThen(f))) {
        (*n)++;
        (*t)++;
        addNode(tnode,(*n),Biddy_Regular(f),(*t));
      }
      if (!Biddy_IsConstant(Biddy_GetElse(f))) {
        EnumerateNodes(Biddy_GetElse(f),tnode,t,n);
      }
      if (!Biddy_IsConstant(Biddy_GetThen(f))) {
        EnumerateNodes(Biddy_GetThen(f),tnode,t,n);
      }
    }
  }
}

static void
addNode(tNode **tnode, int n, void *p, int t)
{
  if (!(*tnode)) {
    (*tnode) = (tNode *) malloc(n * sizeof(tNode));
  } else {
    (*tnode) = (tNode *) realloc((*tnode), n * sizeof(tNode));
  }
  (*tnode)[n-1].n = n;
  (*tnode)[n-1].p = p;
  (*tnode)[n-1].t = t;
}

static int
findNode(tNode *tnode, int n, void *p, int t)
{
  int i;

  i=0;
  while (i<n) {
    if ((tnode[i].p == p) && (tnode[i].t == t)) {
      return tnode[i].n;
    }
    i++;
  }

  return -1;
}

static void
WriteDotConnections(FILE *funfile, Biddy_Edge f, tNode *tnode, int *t, int n)
{
  int n1,n2;

  if (!Biddy_IsConstant(f) && !Biddy_IsSelected(f)) {
    Biddy_SelectNode(f);
    n1 = findNode(tnode,n,Biddy_Regular(f),0);

    if (Biddy_Regular(Biddy_GetThen(f)) == Biddy_Regular(Biddy_GetElse(f))) {

      if (Biddy_IsConstant(Biddy_GetThen(f))) {
        n2 = findNode(tnode,n,Biddy_Regular(f),(*t));
      } else {
        n2 = findNode(tnode,n,Biddy_Regular(Biddy_GetThen(f)),0);
      }
      fprintf(funfile,"connect %d %d d\n",n1,n2);

    } else {

      if (Biddy_IsConstant(Biddy_GetElse(f))) {
        n2 = findNode(tnode,n,Biddy_Regular(f),(*t));
      } else {
        n2 = findNode(tnode,n,Biddy_Regular(Biddy_GetElse(f)),0);
      }
      fprintf(funfile,"connect %d %d ",n1,n2);
      if (Biddy_GetMark((Biddy_GetElse(f)))) {
        fprintf(funfile,"li\n");
      } else {
        fprintf(funfile,"l\n");
      }

      if (Biddy_IsConstant(Biddy_GetThen(f))) {
        n2 = findNode(tnode,n,Biddy_Regular(f),(*t));
      } else {
        n2 = findNode(tnode,n,Biddy_Regular(Biddy_GetThen(f)),0);
      }
      fprintf(funfile,"connect %d %d r\n",n1,n2);

    }

    if (Biddy_IsConstant(Biddy_GetElse(f)) || Biddy_IsConstant(Biddy_GetThen(f))) (*t)++;
    WriteDotConnections(funfile,Biddy_GetElse(f),tnode,t,n);
    WriteDotConnections(funfile,Biddy_GetThen(f),tnode,t,n);
  }
}

static void
ParseDot(FILE *dotfile, tableXY *table, int *xsize, int *ysize)
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
    sscanf(&(strstr(line,"bb=")[4]),"%s",pos);
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
      sscanf(&(strstr(line,"label=")[6]),"%s",defaultlabel);
      defaultlabel[strlen(defaultlabel)-1] = 0;
      while (r && !strstr(line,"height=")) r = fgets(line,255,dotfile);
    }
    if (r) {
      sscanf(line,"%d",&id);
      table[id].id = id;
    }

    while (r && !strstr(line,"label=") && !strstr(line,"pos=")) r = fgets(line,255,dotfile);
    if (r) {
      if (strstr(line,"label=")) {
        label = (Biddy_String) malloc(strlen(line));
        sscanf(&(strstr(line,"label=")[6]),"%s",label);
        label[strlen(label)-1] = 0;
      } else {
        label = strdup(defaultlabel);
      }
    }

    while (r && !strstr(line,"pos=")) r = fgets(line,255,dotfile);
    if (r) {
      pos = (Biddy_String) malloc(strlen(line));
      sscanf(&(strstr(line,"pos=")[5]),"%s",pos);
      sscanf(pos,"%f,%f",&xf,&yf);

      table[id].label = strdup(label);
      table[id].x = xf; /* float -> int */
      table[id].y = yf; /* float -> int */
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

/**Function****************************************************************
  Synopsis    [Function Bddscout_ConstructBDD.]
  Description []
  SideEffects []
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
Bddscout_ConstructBDD(int numV, Biddy_String s2, int numN, Biddy_String s4)
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
  tableN = (BddscoutTclBDD *) malloc(numN * sizeof(BddscoutTclBDD));

  if (numV) {
    word = strtok(s2," "); /* parsing list of variables */
    tableV[0].name = strdup(word);
    tableV[0].order = 0;
    for (i=1; i<numV; ++i) {
      word = strtok(NULL," ");
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
      tableN[id].f = Biddy_GetConstantOne();
    } else {
      tableN[id].created = FALSE;
      tableN[id].f = BDDNULL;
    }
  }

  for (i=0; i<numV; ++i) {
    Biddy_FoaVariable(tableV[i].name);
  }

  /* DEBUGGING */
  /*
  for (i=0; i<numV; ++i) {
    fprintf(stderr,"Variable: %s\n",tableV[i].name);
    Biddy_SwapWithHigher(Biddy_GetTopVariable(Biddy_FoaVariable(tableV[i].name)));
    Biddy_SwapWithLower(Biddy_GetTopVariable(Biddy_FoaVariable(tableV[i].name)));
  }
  */

  for (i=0; i<numN; ++i) {
    if (tableN[i].type == 0) {
      reorderVariables(tableV,tableN,tableN[i].l);
      constructBDD(tableN,tableN[i].l);
      Biddy_AddPersistentFormula(tableN[i].name,tableN[tableN[i].l].f);
    }
    if (tableN[i].type == 1) {
      reorderVariables(tableV,tableN,tableN[i].l);
      constructBDD(tableN,tableN[i].l);
      Biddy_AddPersistentFormula(tableN[i].name,Biddy_Not(tableN[tableN[i].l].f));
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
      tableN[id].f = Biddy_ITE(Biddy_FoaVariable(tableN[id].name),
                              tableN[tableN[id].r].f,tableN[tableN[id].l].f);
    }

    if (tableN[id].type == 4) {
      tableN[id].f = Biddy_ITE(Biddy_FoaVariable(tableN[id].name),
                              tableN[tableN[id].r].f,Biddy_Not(tableN[tableN[id].l].f));
    }

    tableN[id].created = TRUE;
  }

}

/**Function****************************************************************
  Synopsis    [Function Bddscout_ReadBDD.]
  Description []
  SideEffects []
  SeeAlso     []
  ************************************************************************/

Biddy_String
Bddscout_ReadBDD(FILE *s)
{
  Biddy_String bdd;
  int len,n;
  Biddy_String name;

  len = 0;
  bdd = strdup("");
  do {
    bdd = (Biddy_String) realloc(bdd,len+255);
    n = fread(&bdd[len],1,255,s);
    len = len + n;
  } while (n == 255);

  name = Biddy_Eval0(bdd);
  free(bdd);

  if (!strcmp(name,"")) {
    printf("Bddscout_ReadBDD: ERROR in file\n");
    return("");
  }

  return(name);
}

/**Function****************************************************************
  Synopsis    [Function Bddscout_WriteBDD.]
  Description []
  SideEffects []
  SeeAlso     []
  ************************************************************************/

void
Bddscout_WriteBDD(char *filename, Biddy_String name)
{
  Biddy_Edge f;

  if (!Biddy_FindFormula(name,&f)) {
    printf("Function %s does not exists!\n",name);
    return;
  } else {
    Biddy_WriteBDDx(filename,f,name);
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

static int BddscoutWriteBDDviewCmd(ClientData clientData, Tcl_Interp *interp,
                          int argc, USECONST char **argv);

static int BddscoutConstructBDDCmd(ClientData clientData, Tcl_Interp *interp,
                          int argc, USECONST char **argv);

static int BddscoutReadBDDCmd(ClientData clientData, Tcl_Interp *interp,
                          int argc, USECONST char **argv);

static int BddscoutReadBFCmd(ClientData clientData, Tcl_Interp *interp,
                          int argc, USECONST char **argv);

static int BddscoutWriteBDDCmd(ClientData clientData, Tcl_Interp *interp,
                          int argc, USECONST char **argv);

static int BddscoutSiftingCmd(ClientData clientData, Tcl_Interp *interp,
                                int argc, USECONST char **argv);

static int BddscoutSiftingOnFunctionCmd(ClientData clientData, Tcl_Interp *interp,
                                int argc, USECONST char **argv);

static int BiddyVariableNumCmd(ClientData clientData, Tcl_Interp *interp,
                                int argc, USECONST char **argv);

static int BiddyTableSizeCmd(ClientData clientData, Tcl_Interp *interp,
                                int argc, USECONST char **argv);

static int BiddyTableMaxCmd(ClientData clientData, Tcl_Interp *interp,
                                int argc, USECONST char **argv);

static int BiddyTableNumberCmd(ClientData clientData, Tcl_Interp *interp,
                                int argc, USECONST char **argv);

static int BiddyVariableNumberCmd(ClientData clientData, Tcl_Interp *interp,
                                int argc, USECONST char **argv);

static int BiddyTableFortifiedCmd(ClientData clientData, Tcl_Interp *interp,
                                int argc, USECONST char **argv);

static int BiddyTableFoaCmd(ClientData clientData, Tcl_Interp *interp,
                                int argc, USECONST char **argv);

static int BiddyTableCompareCmd(ClientData clientData, Tcl_Interp *interp,
                                int argc, USECONST char **argv);

static int BiddyTableAddCmd(ClientData clientData, Tcl_Interp *interp,
                                int argc, USECONST char **argv);

static int BiddyGarbageNumberCmd(ClientData clientData, Tcl_Interp *interp,
                                int argc, USECONST char **argv);

static int BiddyTableGeneratedCmd(ClientData clientData, Tcl_Interp *interp,
                                int argc, USECONST char **argv);

static int BiddyBlockNumberCmd(ClientData clientData, Tcl_Interp *interp,
                                int argc, USECONST char **argv);

static int BiddySwapNumberCmd(ClientData clientData, Tcl_Interp *interp,
                                int argc, USECONST char **argv);

static int BiddySiftingNumberCmd(ClientData clientData, Tcl_Interp *interp,
                                int argc, USECONST char **argv);

static int BiddyListUsedCmd(ClientData clientData, Tcl_Interp *interp,
                                int argc, USECONST char **argv);

static int BiddyListMaxLengthCmd(ClientData clientData, Tcl_Interp *interp,
                                int argc, USECONST char **argv);

static int BiddyListAvgLengthCmd(ClientData clientData, Tcl_Interp *interp,
                                int argc, USECONST char **argv);

static int BiddyIteCacheSearchCmd(ClientData clientData, Tcl_Interp *interp,
                                int argc, USECONST char **argv);

static int BiddyIteCacheFindCmd(ClientData clientData, Tcl_Interp *interp,
                                int argc, USECONST char **argv);

static int BiddyIteCacheOverwriteCmd(ClientData clientData, Tcl_Interp *interp,
                                int argc, USECONST char **argv);

#ifdef __cplusplus
extern "C" {
#endif

int
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

  Tcl_CreateCommand(interp, "bddscout_writeBDDview", BddscoutWriteBDDviewCmd,
                     (ClientData) NULL, (Tcl_CmdDeleteProc *) NULL);

  Tcl_CreateCommand(interp, "bddscout_constructBDD", BddscoutConstructBDDCmd,
                     (ClientData) NULL, (Tcl_CmdDeleteProc *) NULL);

  Tcl_CreateCommand(interp, "bddscout_readBDD", BddscoutReadBDDCmd,
                     (ClientData) NULL, (Tcl_CmdDeleteProc *) NULL);

  Tcl_CreateCommand(interp, "bddscout_readBF", BddscoutReadBFCmd,
                     (ClientData) NULL, (Tcl_CmdDeleteProc *) NULL);

  Tcl_CreateCommand(interp, "bddscout_writeBDD", BddscoutWriteBDDCmd,
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

  Tcl_CreateCommand(interp, "biddy_table_fortified", BiddyTableFortifiedCmd,
                     (ClientData) NULL, (Tcl_CmdDeleteProc *) NULL);

  Tcl_CreateCommand(interp, "biddy_table_foa", BiddyTableFoaCmd,
                     (ClientData) NULL, (Tcl_CmdDeleteProc *) NULL);

  Tcl_CreateCommand(interp, "biddy_table_compare", BiddyTableCompareCmd,
                     (ClientData) NULL, (Tcl_CmdDeleteProc *) NULL);

  Tcl_CreateCommand(interp, "biddy_table_add", BiddyTableAddCmd,
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

  Tcl_CreateCommand(interp, "biddy_cache_ITE_search", BiddyIteCacheSearchCmd,
                     (ClientData) NULL, (Tcl_CmdDeleteProc *) NULL);

  Tcl_CreateCommand(interp, "biddy_cache_ITE_find", BiddyIteCacheFindCmd,
                     (ClientData) NULL, (Tcl_CmdDeleteProc *) NULL);

  Tcl_CreateCommand(interp, "biddy_cache_ITE_overwrite", BiddyIteCacheOverwriteCmd,
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

  Biddy_Init();

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

  Biddy_Exit();

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

static int
BddscoutListVariablesByPositionCmd(ClientData clientData, Tcl_Interp *interp,
                    int argc, USECONST char **argv)
{
  Biddy_String list;

  list = strdup("");

  Bddscout_ListVariablesByPosition(&list);

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

  Bddscout_ListVariablesByName(&list);

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

  Bddscout_ListVariablesByOrder(&list);

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

  Bddscout_ListVariablesByNumber(&list);

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

  Bddscout_ListFormulaByName(&list);

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

  Bddscout_ListFormulaByUIntParameter(&list,Biddy_NodeNumber);

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

  Bddscout_ListFormulaByUIntParameter(&list,Biddy_NodeMaxLevel);

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

  Bddscout_ListFormulaByFloatParameter(&list,Biddy_NodeAvgLevel);

  Tcl_SetResult(interp, list, TCL_VOLATILE);
  free(list);

  return TCL_OK;
}

static int
BddscoutWriteBDDviewCmd(ClientData clientData, Tcl_Interp *interp, int argc,
                  USECONST char **argv)
{
  Biddy_String s1,s2,s2x,s3;
  FILE *funfile;
  int ERR;

#ifdef WINDOWS
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

#ifdef WINDOWS
  s2x = (Biddy_String) malloc(strlen(docdir)+strlen(appdir)+strlen(s2)+1);
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
  
  funfile = fopen(s2x,"w");
  if (!funfile) {
    printf("BddscoutWriteBDDviewCmd: File error (%s)!\n",s2x);
    free(s1);
    free(s2);
    free(s2x);
    free(s3);
    return TCL_ERROR;
  }

  ERR = Bddscout_WriteBDDView(funfile,s3,s1);
  fclose(funfile);

  free(s1);
  free(s2);
  free(s3);

  if (ERR) {
    printf("Bddscout_WriteBDDView error\n");
  }

  Tcl_SetResult(interp, s2x, TCL_VOLATILE);
  free(s2x);

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

  Bddscout_ConstructBDD(numV,s2,numN,s4);

  free(s1);
  free(s2);
  free(s3);
  free(s4);

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

  Bddscout_WriteBDD(s1,s2);

  free(s1);
  free(s2);

  Tcl_SetResult(interp, (char *) "", TCL_STATIC);
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

  name = Bddscout_ReadBDD(funfile);

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

  name = Bddscout_ParseFile(funfile);

  fclose(funfile);

  free(s1);

  Tcl_SetResult(interp, name, TCL_VOLATILE);
  free(name);

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

  Biddy_Sifting(NULL);

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

  if (Biddy_FindFormula(s1,&f)) {
    Biddy_Sifting(f);
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
  sprintf(sup,"%u",Biddy_VariableTableNum());
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
  sprintf(sup,"%u",Biddy_NodeTableSize());
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
  sprintf(sup,"%u",Biddy_NodeTableMax());
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
  sprintf(sup,"%u",Biddy_NodeTableNum());
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
  while (!find && v<Biddy_VariableTableNum()) {
    if (!strcmp(s1,Biddy_GetVariableName(v))) {
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
    sprintf(sup,"%u",Biddy_NodeTableNumVar(v));
  }

  Tcl_SetResult(interp, sup, TCL_VOLATILE);
  free(sup);

  return TCL_OK;
}

static int
BiddyTableFortifiedCmd(ClientData clientData, Tcl_Interp *interp, int argc,
                  USECONST char **argv)
{
  Biddy_String sup;

  if (argc != 1) {
    Tcl_SetResult(interp, (char *) "wrong # args", TCL_STATIC);
    return TCL_ERROR;
  }

  sup = (Biddy_String) malloc(127);
  sprintf(sup,"%u",Biddy_NodeTableNumF());
  Tcl_SetResult(interp, sup, TCL_VOLATILE);
  free(sup);

  return TCL_OK;
}

static int
BiddyTableFoaCmd(ClientData clientData, Tcl_Interp *interp, int argc,
                  USECONST char **argv)
{
  Biddy_String sup;

  if (argc != 1) {
    Tcl_SetResult(interp, (char *) "wrong # args", TCL_STATIC);
    return TCL_ERROR;
  }

  sup = (Biddy_String) malloc(127);

#ifdef WINDOWS
  sprintf(sup,"%I64u",Biddy_NodeTableFOA()); /* MINGW REQUIRES "%I64u" */
#else
  sprintf(sup,"%llu",Biddy_NodeTableFOA());
#endif

  Tcl_SetResult(interp, sup, TCL_VOLATILE);
  free(sup);

  return TCL_OK;
}

static int
BiddyTableCompareCmd(ClientData clientData, Tcl_Interp *interp, int argc,
                  USECONST char **argv)
{
  Biddy_String sup;

  if (argc != 1) {
    Tcl_SetResult(interp, (char *) "wrong # args", TCL_STATIC);
    return TCL_ERROR;
  }

  sup = (Biddy_String) malloc(127);

#ifdef WINDOWS
  sprintf(sup,"%I64u",Biddy_NodeTableCompare()); /* MINGW REQUIRES "%I64u" */
#else
  sprintf(sup,"%llu",Biddy_NodeTableCompare());
#endif

  Tcl_SetResult(interp, sup, TCL_VOLATILE);
  free(sup);

  return TCL_OK;
}

static int
BiddyTableAddCmd(ClientData clientData, Tcl_Interp *interp, int argc,
                  USECONST char **argv)
{
  Biddy_String sup;

  if (argc != 1) {
    Tcl_SetResult(interp, (char *) "wrong # args", TCL_STATIC);
    return TCL_ERROR;
  }

  sup = (Biddy_String) malloc(127);

#ifdef WINDOWS
  sprintf(sup,"%I64u",Biddy_NodeTableAdd()); /* MINGW REQUIRES "%I64u" */
#else
  sprintf(sup,"%llu",Biddy_NodeTableAdd());
#endif

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
  sprintf(sup,"%u",Biddy_NodeTableGarbage());
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
  sprintf(sup,"%u",Biddy_NodeTableGenerated());
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
  sprintf(sup,"%u",Biddy_NodeTableBlockNumber());
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
  sprintf(sup,"%u",Biddy_NodeTableSwapNumber());
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
  sprintf(sup,"%u",Biddy_NodeTableSiftingNumber());
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
  sprintf(sup,"%u",Biddy_ListUsed());
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
  sprintf(sup,"%u",Biddy_ListMaxLength());
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
  sprintf(sup,"%.2f",Biddy_ListAvgLength());
  Tcl_SetResult(interp, sup, TCL_VOLATILE);
  free(sup);

  return TCL_OK;
}

static int
BiddyIteCacheSearchCmd(ClientData clientData, Tcl_Interp *interp, int argc,
                  USECONST char **argv)
{
  Biddy_String sup;

  if (argc != 1) {
    Tcl_SetResult(interp, (char *) "wrong # args", TCL_STATIC);
    return TCL_ERROR;
  }

  sup = (Biddy_String) malloc(127);

#ifdef WINDOWS
  sprintf(sup,"%I64u",Biddy_IteCacheSearch()); /* MINGW REQUIRES "%I64u" */
#else
  sprintf(sup,"%llu",Biddy_IteCacheSearch());
#endif

  Tcl_SetResult(interp, sup, TCL_VOLATILE);
  free(sup);

  return TCL_OK;
}

static int
BiddyIteCacheFindCmd(ClientData clientData, Tcl_Interp *interp, int argc,
                  USECONST char **argv)
{
  Biddy_String sup;

  if (argc != 1) {
    Tcl_SetResult(interp, (char *) "wrong # args", TCL_STATIC);
    return TCL_ERROR;
  }

  sup = (Biddy_String) malloc(127);

#ifdef WINDOWS
  sprintf(sup,"%I64u",Biddy_IteCacheFind()); /* MINGW REQUIRES "%I64u" */
#else
  sprintf(sup,"%llu",Biddy_IteCacheFind());
#endif

  Tcl_SetResult(interp, sup, TCL_VOLATILE);
  free(sup);

  return TCL_OK;
}

static int
BiddyIteCacheOverwriteCmd(ClientData clientData, Tcl_Interp *interp, int argc,
                  USECONST char **argv)
{
  Biddy_String sup;

  if (argc != 1) {
    Tcl_SetResult(interp, (char *) "wrong # args", TCL_STATIC);
    return TCL_ERROR;
  }

  sup = (Biddy_String) malloc(127);

#ifdef WINDOWS
  sprintf(sup,"%I64u",Biddy_IteCacheOverwrite()); /* MINGW REQUIRES "%I64u" */
#else
  sprintf(sup,"%llu",Biddy_IteCacheOverwrite());
#endif

  Tcl_SetResult(interp, sup, TCL_VOLATILE);
  free(sup);

  return TCL_OK;
}
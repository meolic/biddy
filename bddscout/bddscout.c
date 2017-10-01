/**CFile*******************************************************************
  PackageName [Bdd Scout]
  Synopsis    [Bdd Scout]

  FileName    [bddscout.c]
  Revision    [$Revision: 319 $]
  Date        [$Date: 2017-09-30 22:37:26 +0200 (sob, 30 sep 2017) $]
  Authors     [Robert Meolic (robert.meolic@um.si)]
  Description []
  SeeAlso     [bddscout.h]

  Copyright   [This file is part of Bdd Scout package.
               Copyright (C) 2008, 2017 UM FERI
               UM FERI, Koroska cesta 46, SI-2000 Maribor, Slovenia

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

#define readln(f,s) SCAN_RESULT = fscanf(f," %[^\n] ",s)

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

Biddy_Manager MNGROBDD,MNGROBDDCE,MNGZBDDCE,MNGTZBDD;
Biddy_Manager MNGACTIVE = NULL;

/*-----------------------------------------------------------------------*/
/* Static function prototypes                                            */
/*-----------------------------------------------------------------------*/

static void concat(char **s1, const char *s2);

/*-----------------------------------------------------------------------*/
/* External functions                                                    */
/*-----------------------------------------------------------------------*/

/**Function****************************************************************
  Synopsis    [Function Bddscout_GetActiveManager]
  Description [Bddscout_GetActiveManager is used in BDD Scout extensions.]
  SideEffects []
  SeeAlso     []
  ************************************************************************/

#ifdef __cplusplus
extern "C" {
#endif

Biddy_Manager
Bddscout_GetActiveManager()
{
  return MNGACTIVE;
}

#ifdef __cplusplus
}
#endif

/**Function****************************************************************
  Synopsis    [Function Bddscout_ClearActiveManager]
  Description [Bddscout_ClearActiveManager is used in BDD Scout extensions.]
  SideEffects []
  SeeAlso     []
  ************************************************************************/

#ifdef __cplusplus
extern "C" {
#endif

void
Bddscout_ClearActiveManager()
{
  if (MNGACTIVE == MNGROBDD) {
    Biddy_ExitMNG(&MNGROBDD);
    Biddy_InitMNG(&MNGROBDD,BIDDYTYPEOBDD);
    MNGACTIVE = MNGROBDD;
  }
  else if (MNGACTIVE == MNGROBDDCE) {
    Biddy_ExitMNG(&MNGROBDDCE);
    Biddy_InitMNG(&MNGROBDDCE,BIDDYTYPEOBDDC);
    MNGACTIVE = MNGROBDDCE;
  }
  else if (MNGACTIVE == MNGZBDDCE) {
    Biddy_ExitMNG(&MNGZBDDCE);
    Biddy_InitMNG(&MNGZBDDCE,BIDDYTYPEZBDDC);
    MNGACTIVE = MNGZBDDCE;
  }
  else if (MNGACTIVE == MNGTZBDD) {
    Biddy_ExitMNG(&MNGTZBDD);
    Biddy_InitMNG(&MNGTZBDD,BIDDYTYPETZBDD);
    MNGACTIVE = MNGTZBDD;
  }
}

#ifdef __cplusplus
}
#endif

/**Function****************************************************************
  Synopsis    []
  Description []
  SideEffects []
  SeeAlso     []
  ************************************************************************/

#ifdef __cplusplus
extern "C" {
#endif

void
Bddscout_ChangeActiveManager(Biddy_String type)
{
  if (!strcmp(type,"BIDDYTYPEOBDD")) {
    MNGACTIVE = MNGROBDD;
  }
  else if (!strcmp(type,"BIDDYTYPEOBDDC")) {
    MNGACTIVE = MNGROBDDCE;
  }
  else if (!strcmp(type,"BIDDYTYPEZBDDC")) {
    MNGACTIVE = MNGZBDDCE;
  }
  else if (!strcmp(type,"BIDDYTYPETZBDD")) {
    MNGACTIVE = MNGTZBDD;
  }
}

#ifdef __cplusplus
}
#endif

/*-----------------------------------------------------------------------*/
/* Internal functions                                                    */
/*-----------------------------------------------------------------------*/

/**Function****************************************************************
  Synopsis    [Function BddscoutInit]
  Description []
  SideEffects []
  SeeAlso     []
  ************************************************************************/

void
BddscoutInit()
{
  Biddy_InitMNG(&MNGROBDD,BIDDYTYPEOBDD);
  Biddy_InitMNG(&MNGROBDDCE,BIDDYTYPEOBDDC);
  Biddy_InitMNG(&MNGZBDDCE,BIDDYTYPEZBDDC);
  Biddy_InitMNG(&MNGTZBDD,BIDDYTYPETZBDD);
  MNGACTIVE = MNGROBDD; /* this should be compatible with ACTIVEBDDTYPE in bddscout.tcl */
  
  /* USE THIS TO USE ANONYMOUS MANAGER */
  /*
  MNGACTIVE = NULL;
  Biddy_InitAnonymous(BIDDYTYPEOBDDC);
  */
}

/**Function****************************************************************
  Synopsis    [Function BddscoutExit]
  Description []
  SideEffects []
  SeeAlso     []
  ************************************************************************/

void
BddscoutExit()
{
  Biddy_ExitMNG(&MNGROBDD);
  Biddy_ExitMNG(&MNGROBDDCE);
  Biddy_ExitMNG(&MNGZBDDCE);
  Biddy_ExitMNG(&MNGTZBDD);

  if (!MNGACTIVE) Biddy_Exit();
}

/**Function****************************************************************
  Synopsis    [Function BddscoutCheckFormula]
  Description [BddscoutCheckFormula check if formula with the given fname
               exists in the given manager.]
  SideEffects []
  SeeAlso     []
  ************************************************************************/

Biddy_Boolean
BddscoutCheckFormula(Biddy_String type, Biddy_String fname)
{
  Biddy_Manager mng;
  Biddy_Edge tmp;
  unsigned int idx;
  Biddy_Boolean OK;

  mng = NULL;
  if (!strcmp(type,"BIDDYTYPEOBDD")) {
    mng = MNGROBDD;
  }
  else if (!strcmp(type,"BIDDYTYPEOBDDC")) {
    mng = MNGROBDDCE;
  }
  else if (!strcmp(type,"BIDDYTYPEZBDDC")) {
    mng = MNGZBDDCE;
  }
  else if (!strcmp(type,"BIDDYTYPETZBDD")) {
    mng = MNGTZBDD;
  }

  OK = FALSE;
  if (mng) {
    OK = Biddy_Managed_FindFormula(mng,fname,&idx,&tmp);
  }

  /* VARIABLES ARE CONSIDERED AS FORMULAE BY BIDDY! */
  if (idx == 0) OK = FALSE;

  return OK;
}

/**Function****************************************************************
  Synopsis    [Function BddscoutCopyFormula]
  Description [BddscoutCopyFormula copies the given formula from one
               manager to another.]
  SideEffects []
  SeeAlso     []
  ************************************************************************/

void
BddscoutCopyFormula(Biddy_String fname, Biddy_String type1,
                    Biddy_String type2)
{
  Biddy_Manager mng1,mng2;

  mng1 = NULL;
  if (!strcmp(type1,"BIDDYTYPEOBDD")) {
    mng1 = MNGROBDD;
  }
  else if (!strcmp(type1,"BIDDYTYPEOBDDC")) {
    mng1 = MNGROBDDCE;
  }
  else if (!strcmp(type1,"BIDDYTYPEZBDDC")) {
    mng1 = MNGZBDDCE;
  }
  else if (!strcmp(type1,"BIDDYTYPETZBDD")) {
    mng1 = MNGTZBDD;
  }

  mng2 = NULL;
  if (!strcmp(type2,"BIDDYTYPEOBDD")) {
    mng2 = MNGROBDD;
  }
  else if (!strcmp(type2,"BIDDYTYPEOBDDC")) {
    mng2 = MNGROBDDCE;
  }
  else if (!strcmp(type2,"BIDDYTYPEZBDDC")) {
    mng2 = MNGZBDDCE;
  }
  else if (!strcmp(type2,"BIDDYTYPETZBDD")) {
    mng2 = MNGTZBDD;
  }

  if (mng1 && mng2) {
    Biddy_Managed_CopyFormula(mng1,mng2,fname);
  }
}

/**Function****************************************************************
  Synopsis    [Function BddscoutSyncFormula]
  Description [BddscoutSyncFormula adapt or create formula with the given
               name in all managers. Formula is given with the string form.]
  SideEffects [TO DO: works only for form that could be parsed by eval2.]
  SeeAlso     []
  ************************************************************************/

void
BddscoutSyncFormula(Biddy_Edge form, Biddy_String name)
{
  Biddy_Edge tmp,bdd;
  unsigned int idx;

  if (MNGACTIVE != MNGROBDD) {
    if (Biddy_Managed_FindFormula(MNGROBDD,name,&idx,&tmp)) {
      bdd = Biddy_Managed_Eval2(MNGROBDD,form); /* TO DO: use convert instead of eval2 */
      if (bdd != tmp) {
        Biddy_Managed_AddPersistentFormula(MNGROBDD,name,bdd);
      }
    }
  }

  if (MNGACTIVE != MNGROBDDCE) {
    if (Biddy_Managed_FindFormula(MNGROBDDCE,name,&idx,&tmp)) {
      bdd = Biddy_Managed_Eval2(MNGROBDDCE,form); /* TO DO: use convert instead of eval2 */
      if (bdd != tmp) {
        Biddy_Managed_AddPersistentFormula(MNGROBDDCE,name,bdd);
      }
    }
  }

  if (MNGACTIVE != MNGZBDDCE) {
    if (Biddy_Managed_FindFormula(MNGZBDDCE,name,&idx,&tmp)) {
      bdd = Biddy_Managed_Eval2(MNGZBDDCE,form); /* TO DO: use convert instead of eval2 */
      if (bdd != tmp) {
        Biddy_Managed_AddPersistentFormula(MNGZBDDCE,name,bdd);
      }
    }
  }

  if (MNGACTIVE != MNGTZBDD) {
    if (Biddy_Managed_FindFormula(MNGTZBDD,name,&idx,&tmp)) {
      bdd = Biddy_Managed_Eval2(MNGTZBDD,form); /* TO DO: use convert instread of eval2 */
      if (bdd != tmp) {
        Biddy_Managed_AddPersistentFormula(MNGTZBDD,name,bdd);
      }
    }
  }
}

/**Function****************************************************************
  Synopsis    [Function BddscoutConstructBDD.]
  Description [BddscoutConstructBDD is used to construct BDD from lists
               of nodes and edges created by tcl functions.]
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
  unsigned int idx;
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

  /* DEBUGGING */
  /*
  printf("\nBddscoutWriteBddview started for %s (MNGACTIVE = %p)\n", Biddy_Managed_GetManagerName(MNGACTIVE), MNGACTIVE);
  */

  if (!Biddy_Managed_FindFormula(MNGACTIVE,name,&idx,&f)) {
    fprintf(stderr,"BddscoutWriteBddview: (%p) Function %s does not exists!\n",MNGACTIVE,name);
    return 0;
  }

  if (Biddy_IsNull(f)) {
    fprintf(stderr,"BddscoutWriteBddview: (%p) Function %s is NULL!\n",MNGACTIVE,name);
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

  /* DEBUGGING */
  /*
  printf("Biddy_Managed_WriteDot STARTED\n");
  n = Biddy_Managed_WriteDot(MNGACTIVE,NULL,f,name,-1,FALSE);
  printf("Biddy_Managed_WriteDot FINISHED\n");
  */
  
  n = Biddy_Managed_WriteDot(MNGACTIVE,dotname,f,name,-1,FALSE); /* FALSE is important */

  if (n == 0) {
    fprintf(stderr,"BddscoutWriteBddview: Problem with function %s (n==0) !\n",name);
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
  if (!table) {
    fclose(xdotfile);
    return -1;
  }
  parseDot(xdotfile,table,&xsize,&ysize);
  fclose(xdotfile);

  free(dotname);
  free(xdotname);

  /* DEBUGGING */
  /*
  printf("Biddy_Managed_WriteBddview STARTED\n");
  m = Biddy_Managed_WriteBddview(MNGACTIVE,NULL,f,name,table);
  printf("Biddy_Managed_WriteBddview FINISHED\n");
  */

  m = Biddy_Managed_WriteBddview(MNGACTIVE,filename,f,name,table);

  free(table);

  if (m != n) {
    fprintf(stderr,"BddscoutWriteBddview: Problem with function %s (m!=n)!\n",name);
    return(0);
  }

  return n;
}

static void
parseDot(FILE *dotfile, Biddy_XY *table, int *xsize, int *ysize)
{
  char line[256];
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
    printf("BddscoutReadBDD: ERROR in file\n");
    return("");
  }

  return(name);
}

/**Function****************************************************************
  Synopsis    [Function BddscoutReadBF]
  Description []
  SideEffects [Implemented long time ago (1995). Not comprehensible.]
  SeeAlso     []
  ************************************************************************/

static Biddy_String makeFunction(FILE *f, Biddy_String line);

Biddy_String
BddscoutReadBF(FILE *f)
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
    printf("WARNING (BddscoutReadBF): Variable ordering not specified.\n");
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

/*-----------------------------------------------------------------------*/
/* Definition of static functions                                        */
/*-----------------------------------------------------------------------*/

static void
concat(char **s1, const char *s2)
{
   if (s2) {
     *s1 = (char*) realloc(*s1,strlen(*s1)+strlen(s2)+1+1);
     strcat(*s1," ");
     strcat(*s1,s2);
   }
}

/**CFile*******************************************************************
  PackageName [Bdd Scout]
  Synopsis    [Bdd Scout]

  FileName    [bddscoutIFIP.c]
  Revision    [$Revision: 118 $]
  Date        [$Date: 2015-12-24 10:15:47 +0100 (čet, 24 dec 2015) $]
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

/* on tcl 8.3 use #define USECONST */
/* on tcl 8.4 use #define USECONST const*/
/* this is defined in Makefile */

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
  Synopsis    [Function Bddscout_BenchmarkIFIP]
  Description []
  SideEffects [Implemented long time ago (1995). Not comprehensible.]
  SeeAlso     []
  ************************************************************************/

#define readln(f,s) SCAN_RESULT = fscanf(f," %[^\n] ",s)

static Biddy_String makeFunction(FILE *f, Biddy_String line, Biddy_String prefix);
static Biddy_String upCase(Biddy_String s);

Biddy_String
BddscoutBenchmarkIFIP(FILE *f, Biddy_String filename)
{
  Biddy_String line;
  Biddy_String s1;
  Biddy_String var;
  Biddy_String name,tmpname;
  Biddy_String *nameTable;
  int nameNumber;
  Biddy_Boolean *eqvTable;
  Biddy_Boolean ok,usedcs;
  Biddy_Edge bdd1,bdd2,dcs;
  Biddy_String report;
  unsigned int len;
  int i,n;

  for (i=0; i<strlen(filename); i++) filename[i]=toupper(filename[i]);

  /*
  printf("FILENAME: <%s>\n",filename);
  */

  line = (Biddy_String) malloc(4096);

  /* PREFIX - SHOULD BE EMPTY */

  do {
    readln(f,line);
  } while (strstr(line, "@BE1") != line);

  /* @BE1 - JUST A LABEL */

  do {
    readln(f,line);
  } while (strstr(line,"@invar") != line);

  /* @invar  */

  readln(f,line);
  s1 = strdup(line);
  readln(f,line);

  while (strstr(line,"@sub") != line &&
         strstr(line,"@out") != line)
  {
    n = (unsigned int)strlen(s1)+(unsigned int)strlen(line)+1+1;
    s1 = (char *) realloc(s1, sizeof(char) * n);
    strcat(s1," ");
    strcat(s1,line);
    readln(f,line);
  }

  upCase(s1);

  var = strtok(&s1[1]," \t)");
  while (var) {

    /*
    printf("VARIABLE: <%s>\n",var);
    */

    Biddy_FoaVariable(var);
    var = strtok(NULL," \t)");
  }

  free(s1);

  /* @sub */

  if (strstr(line,"@sub") == line) {
    readln(f,line);
    do {
      name = makeFunction(f,line,NULL);
      free(name);
    } while (strstr(line,"@out") != line);
  }

  /* @out */

  nameTable = NULL;
  nameNumber = 0;

  readln(f,line);
  do {
    tmpname = strdup("BE1_");
    len = 4+(unsigned int)strlen(filename)+1;
    tmpname = (Biddy_String) realloc(tmpname,len);
    strcat(tmpname,filename);

    name = makeFunction(f,line,tmpname);

    if (!nameTable) {
      nameTable = (Biddy_String *) malloc(sizeof(Biddy_String));
    } else {
      nameTable = (Biddy_String *) realloc(nameTable,sizeof(Biddy_String)*(nameNumber+1));
    }
    nameTable[nameNumber] = strdup(&name[len]);
    nameNumber++;
    free(name);

  } while (strstr(line,"@end") != line);

  /* FINALIZER FOR @BE1 - SHOULD BE EMPTY */
  
  do {
    readln(f,line);
  } while (strstr(line,"@BE2") != line);

  /* @BE2 - JUST A LABEL */

  do {
    readln(f,line);
  } while (strstr(line,"@invar") != line);

  /* @invar  */

  readln(f,line);
  s1 = strdup(line);
  readln(f,line);

  while (strstr(line,"@sub") != line &&
         strstr(line,"@out") != line)
  {
    n = (unsigned int)strlen(s1)+(unsigned int)strlen(line)+1+1;
    s1 = (char *) realloc(s1, sizeof(char) * n);
    strcat(s1," ");
    strcat(s1,line);
    readln(f,line);
  }

  upCase(s1);

  var = strtok(&s1[1]," \t)");
  while (var) {

    /*
    printf("VARIABLE: <%s>\n",var);
    */

    Biddy_FoaVariable(var);
    var = strtok(NULL," \t)");
  }

  free(s1);

  /* @sub */

  if (strstr(line,"@sub") == line) {
    readln(f,line);
    do {
      name = makeFunction(f,line,NULL);
      free(name);
    } while (strstr(line,"@out") != line);
  }

  /* @out */

  readln(f,line);
  do {
    tmpname = strdup("BE2_");
    len = 4+(unsigned int)strlen(filename)+1;
    tmpname = (Biddy_String) realloc(tmpname,len);
    strcat(tmpname,filename);

    name = makeFunction(f,line,tmpname);

    ok = FALSE;
    for (i=0; (i<nameNumber) && (!ok); i++) {
      if (!strcmp(&name[len],nameTable[i])) {
        ok = TRUE;
      }
    }

    free(name);
    if (!ok) printf("ERROR: missing function in @BE2");

  } while (strstr(line, "@end") != line);

  /* FINALIZER FOR @BE2 - SHOULD BE EMPTY */

  usedcs = FALSE;
  if (!feof(f)) {
    do {
      readln(f, line);
    } while (!feof(f) && strstr(line,"@DCS") != line);
  }

  /* @DCS */

  if (strstr(line, "@DCS") == line) {
    readln(f,line);
    name = makeFunction(f,line,(Biddy_String) "DCS");
    free(name);
    usedcs = TRUE;
  }

  /* FINALIZER */

  if (line) free(line);

  /* CHECK EQUIVALENCE */

  if (usedcs) {
    if (!Biddy_FindFormula((Biddy_String) "DCS",&dcs)) {
      printf("ERROR: missing DCS!\n");
    }
  } else {
    dcs = Biddy_GetConstantZero();
  }

  ok = TRUE;
  eqvTable = (Biddy_Boolean *) malloc(sizeof(Biddy_Boolean)*nameNumber);
  for (i=0; i<nameNumber; i++) {
    len = (unsigned int)strlen(filename)+(unsigned int)strlen(nameTable[i])+5+1;
    name = strdup("BE1_");
    name = (Biddy_String) realloc(name,len);
    strcat(name,filename);
    strcat(name,"_");
    strcat(name,nameTable[i]);
    Biddy_FindFormula(name,&bdd1);
    name[2] = '2';
    Biddy_FindFormula(name,&bdd2);
    free(name);
    eqvTable[i] = (
      Biddy_ITE(dcs,Biddy_GetConstantZero(),bdd1) ==
        Biddy_ITE(dcs,Biddy_GetConstantZero(),bdd2)
    );
    if (!eqvTable[i]) ok = FALSE;
  }

  /* GARBAGE COLLECTION IS USED TO KEEP ONLY OUTPUT FUNCTIONS */
  /* YOU CAN ENABLE VARIANT 1 OR VARIANT 2, OR DISABLE BOTH */

  /* VARIANT 1: USING Biddy_AddFormula AND Biddy_Clean */
  /* you can use Biddy_Purge() or Biddy_PurgeAndReorder(NULL) before Biddy_Clean */
  /**/
  Biddy_Clean();
  /**/

  /* VARIANT 2: USING Biddy_AddPersistentFormula AND Biddy_ClearAll */
  /*
  i = 2;
  name = Biddy_GetIthFormulaName(i);
  while (name) {
    if (strncmp(name,"BE1",3) && strncmp(name,"BE2",3) ) {
      Biddy_DeleteIthFormula(i);
    }
    i++;
    name = Biddy_GetIthFormulaName(i);
  }
  Biddy_ClearAll();
  */

  report = (Biddy_String) malloc(255);
  if (!ok) {
    sprintf(report,"%s 0 %u %u %u %u ",Tcl_GetHostName(),Biddy_VariableTableNum(),Biddy_NodeTableMax(),Biddy_NodeTableNumF(),Biddy_NodeTableGarbage());
  } else {
    sprintf(report,"%s 1 %u %u %u %u ",Tcl_GetHostName(),Biddy_VariableTableNum(),Biddy_NodeTableMax(),Biddy_NodeTableNumF(),Biddy_NodeTableGarbage());
  }

  for (i=0; i<nameNumber; i++) {
    len = (unsigned int)strlen(report)+(unsigned int)strlen(nameTable[i])+3+1;
    report = (Biddy_String) realloc(report,len);
    strcat(report,nameTable[i]);
    strcat(report," ");
    if (eqvTable[i]) {
      strcat(report,"1");
    } else {
      strcat(report,"0");
    }
    strcat(report," ");
  }

  free(nameTable);
  free(eqvTable);

  /* DELETE ALL BDDS - FILES ARE BENCHMARKED INDEPENDENTLY */
  /*
  Biddy_Exit();
  Biddy_Init();
  */

  return report;
}

static Biddy_String
makeFunction(FILE *f, Biddy_String line, Biddy_String prefix)
{
  Biddy_Edge fun;
  Biddy_String s1,s2,name;
  unsigned int len;
  unsigned int x,y;

  /*
  printf("MAKEFUNCTION: %s\n",line);
  */

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

  if (!s1) {
    name = (Biddy_String) strdup(prefix);
  } else {
    x = (unsigned int)strspn(s1," \t");
    y = (unsigned int)strcspn(&s1[x]," \t\n");

    if (!prefix) {
      name = (Biddy_String) strdup(&s1[x]);
      name[y] = 0; /* strndup is not supported everywhere */
    } else {
      len = (unsigned int)strlen(prefix)+1+y+1;
      name = strdup(prefix);
      name = (Biddy_String) realloc(name,len);
      strcat(name,"_");
      strncat(name,&s1[x],y);
      free(s1);
    }
  }

  upCase(name);

  if (!feof(f)) {
    readln(f,line);
    x = (unsigned int)strcspn(line,"=");
  } else {
    strcpy(line,"@end");
    x = 0;
  }
  while (x == strlen(line) &&
         strstr(line, "@out") != line &&
         strstr(line, "@end") != line)
  {
    if (s2 != NULL) {
      len = (unsigned int)strlen(s2)+(unsigned int)strlen(line)+1;
      s2 = (Biddy_String) realloc(s2, len+1);
      strcat(s2," ");
      strcat(s2,line);
    } else {
      s2 = strdup(line);
    }

    if (!feof(f)) {
      readln(f,line);
      x = (unsigned int)strcspn(line, "=");
    } else {
      strcpy(line,"@end");
      x = 0;
    }
  }

  upCase(s2);

  /*
  printf("FUNCTION NAME: <%s>\n",name);
  printf("FUNCTION DEF: <%s>\n",s2);
  */

  fun = Biddy_Eval1x(s2,(Biddy_LookupFunction) NULL);
  
  if (Biddy_IsNull(fun)) {
    printf("WARNING (bddscoutIFIP): NULL FUNCTION\n");
    printf("FUNCTION NAME: <%s>\n",name);
    printf("FUNCTION DEF: <%s>\n",s2);
  }

  /* YOU CAN USE Biddy_AddPersistentFormula INSTEAD OF Biddy_AddFormula */
  /* YOU CAN USE Biddy_Purge() OR Biddy_PurgeAndReorder(fun) AFTER ADDING A FORMULA */

  if (!prefix) {
    Biddy_AddFormula(name,fun,1);
  } else {
    if (!strncmp(prefix,"BE1",3) || !strncmp(prefix,"BE2",3) ) {
      Biddy_AddPersistentFormula(name,fun);
    } else {
      Biddy_AddFormula(name,fun,1);
    }
  }

  free(s2);

  return name;
}

static Biddy_String
upCase(Biddy_String s)
{
  Biddy_String p;

  p = s;
  while (p[0]) {
    p[0] = toupper(p[0]);
    p = &p[1];
  }
  return s;
}

/*-----------------------------------------------------------------------*/
/* TCL related functions                                                 */
/*-----------------------------------------------------------------------*/

/**Function****************************************************************
  Synopsis    [Function Bddscoutifip_Init.]
  Description [This function is used by Tcl/Tk only.]
  SideEffects []
  SeeAlso     []
  ************************************************************************/

static int BddscoutParseIfipCmd(ClientData clientData, Tcl_Interp *interp,
                          int argc, USECONST char **argv);

#ifdef __cplusplus
extern "C" {
#endif

EXTERN int
Bddscoutifip_Init(Tcl_Interp *interp)
{

#ifdef USE_TCL_STUBS
  if (Tcl_InitStubs(interp, "8.1", 0) == NULL) {
    return TCL_ERROR;
  }
#endif

#ifdef USE_BDDSCOUT_STUBS
  if (Bddscout_InitStubs(interp, (char *) "1.0", 0) == NULL) {
    return TCL_ERROR;
  }
#endif

  Tcl_CreateCommand(interp, "bddscout_parseIFIP", BddscoutParseIfipCmd,
                     (ClientData) NULL, (Tcl_CmdDeleteProc *) NULL);

  return Tcl_PkgProvide(interp, "bddscoutIFIP", "1.0");
}

#ifdef __cplusplus
}
#endif

static int
BddscoutParseIfipCmd(ClientData clientData, Tcl_Interp *interp, int argc,
                    USECONST char **argv)
{
  Biddy_String s1,s2;
  FILE *funfile;
  Biddy_String report;

  if (argc != 3) {
    Tcl_SetResult(interp, (char *) "wrong # args", TCL_STATIC);
    return TCL_ERROR;
  }

  s1 = strdup(argv[1]);
  s2 = strdup(argv[2]);

  funfile = fopen(s1,"r");
  if (!funfile) {
    printf("BddscoutParseIfipCmd: File error (%s)!\n",s1);
    return TCL_ERROR;
  }

  report = BddscoutBenchmarkIFIP(funfile,s2);

  fclose(funfile);

  free(s1);
  free(s2);

  Tcl_SetResult(interp, report, TCL_VOLATILE);
  free(report);

  return TCL_OK;
}

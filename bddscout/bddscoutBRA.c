/**CFile*******************************************************************
  PackageName [Bdd Scout]
  Synopsis    [Bdd Scout]

  FileName    [bddscoutBRA.c]
  Revision    [$Revision: 114 $]
  Date        [$Date: 2015-12-21 16:04:00 +0100 (pon, 21 dec 2015) $]
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

/*-----------------------------------------------------------------------*/
/* External functions                                                    */
/*-----------------------------------------------------------------------*/

/*-----------------------------------------------------------------------*/
/* Internal functions                                                    */
/*-----------------------------------------------------------------------*/

Biddy_String
BddscoutBRAExhaustive(Biddy_String fname)
{
  Biddy_Edge f;
  Biddy_Boolean ok;
  Biddy_String report;

  Biddy_Edge *table;
  int i;

  int numstat = 1; /* THE NUMBER OF CHECKED ORDERING */

  /* THIS FUNCTON IS NOT IMPLEMENTED, YET */
  /* NOW, IT WILL ONLY PRESENT STATISTICS FOR CURRENT ORDERING */

  ok = Biddy_FindFormula(fname,&f);
  if (ok) {

    table = (Biddy_Edge *)  malloc(numstat * sizeof(Biddy_Edge));
    for (i=0; i<numstat; i++) {
      table[i] = f;
    }

    report = strdup("");
    for (i=0; i<numstat; i++) {
      Biddy_String line;
      line = (Biddy_String) malloc(255);

      /* HERE, WE OBTAIN STATISTICS */
      sprintf(line,"%s-%02d %d %d %.4f ",fname,i,
              Biddy_NodeNumber(table[i]),
              Biddy_NodeMaxLevel(table[i]),
              Biddy_NodeAvgLevel(table[i]));
      report = (Biddy_String) realloc(report,strlen(report)+strlen(line)+1);
      strcat(report,line);
      free(line);

      /* HERE, WE SHOUD MAKE SOME REORDERING */
      /* NOT IMPLEMENTED, YET */

    }

    free(table);

  } else {
    report = strdup("ERROR");
  }

  return report;
}

/*-----------------------------------------------------------------------*/
/* TCL related functions                                                 */
/*-----------------------------------------------------------------------*/

/**Function****************************************************************
  Synopsis    [Function Bddscoutbra_Init.]
  Description [This function is used by Tcl/Tk only.]
  SideEffects []
  SeeAlso     []
  ************************************************************************/

static int BddscoutSwapWithHigherCmd(ClientData clientData, Tcl_Interp *interp,
                          int argc, USECONST char **argv);

static int BddscoutSwapWithLowerCmd(ClientData clientData, Tcl_Interp *interp,
                          int argc, USECONST char **argv);

static int BddscoutBraExhaustiveCmd(ClientData clientData, Tcl_Interp *interp,
                          int argc, USECONST char **argv);

#ifdef __cplusplus
extern "C" {
#endif

EXTERN int
Bddscoutbra_Init(Tcl_Interp *interp)
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

  Tcl_CreateCommand(interp, "bddscout_swap_with_higher", BddscoutSwapWithHigherCmd,
                     (ClientData) NULL, (Tcl_CmdDeleteProc *) NULL);

  Tcl_CreateCommand(interp, "bddscout_swap_with_lower", BddscoutSwapWithLowerCmd,
                     (ClientData) NULL, (Tcl_CmdDeleteProc *) NULL);

  Tcl_CreateCommand(interp, "bddscout_bra_exhaustive", BddscoutBraExhaustiveCmd,
                     (ClientData) NULL, (Tcl_CmdDeleteProc *) NULL);

  return Tcl_PkgProvide(interp, "bddscoutBRA", "1.0");
}

#ifdef __cplusplus
}
#endif

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
  while (!find && v<Biddy_VariableTableNum()) {
    if (!strcmp(s1,Biddy_GetVariableName(v))) {
      find = TRUE;
    } else {
      v++;
    }
  }
  free(s1);

  /* IF THE ELEMENT WAS NOT FOUND... */
  if (find) {
    Biddy_SwapWithHigher(v);
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
  while (!find && v<Biddy_VariableTableNum()) {
    if (!strcmp(s1,Biddy_GetVariableName(v))) {
      find = TRUE;
    } else {
      v++;
    }
  }
  free(s1);

  /* IF THE ELEMENT WAS NOT FOUND... */
  if (find) {
    Biddy_SwapWithLower(v);
  }

  Tcl_SetResult(interp, (char *) "", TCL_STATIC);
  return TCL_OK;
}

static int
BddscoutBraExhaustiveCmd(ClientData clientData, Tcl_Interp *interp, int argc,
                    USECONST char **argv)
{
  Biddy_String s1;
  Biddy_String report;

  if (argc != 2) {
    Tcl_SetResult(interp, (char *) "wrong # args", TCL_STATIC);
    return TCL_ERROR;
  }

  s1 = strdup(argv[1]);

  report = BddscoutBRAExhaustive(s1);

  free(s1);

  Tcl_SetResult(interp, report, TCL_VOLATILE);
  free(report);

  return TCL_OK;
}

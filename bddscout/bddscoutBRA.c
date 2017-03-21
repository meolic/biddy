/**CFile*******************************************************************
  PackageName [Bdd Scout]
  Synopsis    [Bdd Scout]

  FileName    [bddscoutBRA.c]
  Revision    [$Revision: 244 $]
  Date        [$Date: 2017-02-14 23:23:32 +0100 (tor, 14 feb 2017) $]
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
  Biddy_Manager MNG;
  Biddy_Edge f;
  Biddy_Boolean ok;
  Biddy_String report;

  Biddy_Edge *table;
  int i;

  int numstat = 1; /* THE NUMBER OF CHECKED ORDERING */

  MNG = Bddscout_GetActiveManager();

  /* THIS FUNCTON IS NOT IMPLEMENTED, YET */
  /* NOW, IT WILL ONLY PRESENT STATISTICS FOR CURRENT ORDERING */

  ok = Biddy_Managed_FindFormula(MNG,fname,&f);
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
              Biddy_Managed_NodeNumber(MNG,table[i]),
              Biddy_Managed_NodeMaxLevel(MNG,table[i]),
              Biddy_Managed_NodeAvgLevel(MNG,table[i]));
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

  Tcl_CreateCommand(interp, "bddscout_bra_exhaustive", BddscoutBraExhaustiveCmd,
                     (ClientData) NULL, (Tcl_CmdDeleteProc *) NULL);

  return Tcl_PkgProvide(interp, "bddscoutBRA", "1.0");
}

#ifdef __cplusplus
}
#endif

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

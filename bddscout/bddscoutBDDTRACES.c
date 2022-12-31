/**CFile*******************************************************************
  PackageName [Bdd Scout]
  Synopsis    [Bdd Scout]

  FileName    [bddscoutBDDTRACES.c]
  Revision    [$Revision: 673 $]
  Date        [$Date: 2022-12-29 15:08:11 +0100 (čet, 29 dec 2022) $]
  Authors     [Robert Meolic (robert@meolic.com)]
  Description []
  SeeAlso     [bddscout.h]

  Copyright   [This file is part of Bdd Scout package.
               Copyright (C) 2008, 2019 UM FERI, Koroska cesta 46, SI-2000 Maribor, Slovenia
               Copyright (C) 2019, 2022 Robert Meolic, SI-2000 Maribor, Slovenia

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

/* USEFUL LINKS */
/* http://www.cs.cmu.edu/~bwolen/software/ */
/* http://javabdd.sourceforge.net/ */
/* http://javabdd.sourceforge.net/xref-test/trace/TraceDriver.html */

#ifndef BDDSCOUTPROFILE
#include "bddscoutTcl.h"
#else
#include "bddscout.h"
#endif

#define NODEBUG

/*-----------------------------------------------------------------------*/
/* Functions                                                             */
/*-----------------------------------------------------------------------*/

#include "./bddtraces-BIDDY.c"

/*-----------------------------------------------------------------------*/
/* TCL related functions                                                 */
/*-----------------------------------------------------------------------*/

#ifndef BDDSCOUTPROFILE

extern const char *Bddscout_InitStubs(Tcl_Interp *interp, char *version, int exact);

/**Function****************************************************************
  Synopsis    [Function Bddscoutbddtraces_Init.]
  Description [This function is used by Tcl/Tk only.]
  SideEffects []
  SeeAlso     []
  ************************************************************************/

static int BddscoutRunBddTraceCmd(ClientData clientData, Tcl_Interp *interp,
                          int argc, const char **argv);

#ifdef __cplusplus
extern "C" {
#endif

int
Bddscoutbddtraces_Init(Tcl_Interp *interp)
{

  if (Tcl_InitStubs(interp, "8.1", 0) == NULL) {
    return TCL_ERROR;
  }

  if (Bddscout_InitStubs(interp, (char *) "1.0", 0) == NULL) {
    return TCL_ERROR;
  }

  Tcl_CreateCommand(interp, "bddscout_runBddTrace", BddscoutRunBddTraceCmd,
                     (ClientData) NULL, (Tcl_CmdDeleteProc *) NULL);

  return Tcl_PkgProvide(interp, "bddscoutBDDTRACES", "1.0");
}

#ifdef __cplusplus
}
#endif

static int
BddscoutRunBddTraceCmd(ClientData clientData, Tcl_Interp *interp, int argc,
                       const char **argv)
{
  Biddy_String s1;
  FILE *funfile;
  Biddy_String report;

  if (argc != 2) {
    Tcl_SetResult(interp, (char *) "wrong # args", TCL_STATIC);
    return TCL_ERROR;
  }

  s1 = strdup(argv[1]);

  funfile = fopen(s1,"r");
  if (!funfile) {
    printf("BddscoutRunBddTraceCmd: File error (%s)!\n",s1);
    return TCL_ERROR;
  }

  report = BddscoutRunBddTrace(funfile);

  fclose(funfile);

  free(s1);

  Tcl_SetResult(interp, report, TCL_VOLATILE);
  free(report);

  return TCL_OK;
}

#endif

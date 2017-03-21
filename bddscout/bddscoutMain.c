/**CFile*******************************************************************
  PackageName [Bdd Scout]
  Synopsis    [Bdd Scout]

  FileName    [bddscoutMain.c]
  Revision    [$Revision: 226 $]
  Date        [$Date: 2017-01-04 15:45:57 +0100 (sre, 04 jan 2017) $]
  Authors     [Robert Meolic (robert.meolic@um.si)]
  Description [File bddscoutMain.c initializes and start Tcl/Tk GUI.]
  SeeAlso     []

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

#undef USE_TCL_STUBS

#include <stdio.h>
#include <stdlib.h>
#include <tcl.h>
#include <tk.h>

int AppInit(Tcl_Interp *interp) {
  if(Tcl_Init(interp) == TCL_ERROR) return TCL_ERROR;
  Tk_InitConsoleChannels(interp);
  if(Tk_Init(interp) == TCL_ERROR) return TCL_ERROR;
  Tcl_Eval(interp,"set auto_path [linsert $auto_path 0 /usr/lib/bddscout]");
  Tcl_Eval(interp,"set auto_path [linsert $auto_path 0 [file dirname [info nameofexecutable]]]");
  Tcl_PkgRequire(interp,"bddscout","1",0);
  return TCL_OK;
}

int main(int argc, char *argv[]) {
  Tk_Main(argc, argv, AppInit);
  return 0;
}

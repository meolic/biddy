/**CFile*******************************************************************
  PackageName [Bdd Scout]
  Synopsis    [Bdd Scout]

  FileName    [bddscoutMain.c]
  Revision    [$Revision: 548 $]
  Date        [$Date: 2019-06-25 21:51:07 +0200 (tor, 25 jun 2019) $]
  Authors     [Robert Meolic (robert@meolic.com)]
  Description [File bddscoutMain.c initializes and start Tcl/Tk GUI.]
  SeeAlso     []

  Copyright   [This file is part of Bdd Scout package.
               Copyright (C) 2008, 2019 UM FERI, Koroska cesta 46, SI-2000 Maribor, Slovenia
               Copyright (C) 2019 Robert Meolic, SI-2000 Maribor, Slovenia

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

  /* DEBUGGING */
  /*
  Tcl_Eval(interp,"set chan [open log.log a]");
  Tcl_Eval(interp,"puts $chan $auto_path");
  Tcl_Eval(interp,"close $chan");
  */

  Tcl_PkgRequire(interp,"bddscout","1",0);
  return TCL_OK;
}

int main(int argc, char *argv[]) {
  Tk_Main(argc, argv, AppInit);
  return 0;
}

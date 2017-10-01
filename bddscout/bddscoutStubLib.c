/**CFile*******************************************************************
  PackageName [Bdd Scout]
  Synopsis    [Bdd Scout]

  FileName    [bddscoutStubLib.c]
  Revision    [$Revision: 319 $]
  Date        [$Date: 2017-09-30 22:37:26 +0200 (sob, 30 sep 2017) $]
  Authors     [Robert Meolic (robert.meolic@um.si)]
  Description [Adapted from http://wiki.tcl.tk/3358]
  SeeAlso     [bddscout.h, bddscout.decls, bddscoutDecls.h, bddscoutStubInit.c]

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

#include "bddscoutTcl.h"

const BddscoutStubs *bddscoutStubsPtr;

const char *
Bddscout_InitStubs (Tcl_Interp *interp, char *version, int exact)
{
  const char *actualVersion;
  ClientData clientData = NULL;

  actualVersion = Tcl_PkgRequireEx(interp, "bddscout-lib", version, exact, (ClientData*) &clientData);
  bddscoutStubsPtr = (BddscoutStubs *) clientData;

  if (!actualVersion) {
    Tcl_SetResult(interp, (char *) "ERROR while Tcl_PkgRequireEx(\"bddscout-lib\")", TCL_STATIC);
    return NULL;
  }

  if (!bddscoutStubsPtr) {
    Tcl_SetResult(interp, (char *) "This implementation of bddscout does not support stubs", TCL_STATIC);
    return NULL;
  }

  return actualVersion;
}

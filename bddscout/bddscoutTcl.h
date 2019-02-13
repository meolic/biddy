/**CHeaderFile*************************************************************
  PackageName [Bdd Scout]
  Synopsis    [Bdd Scout]

  FileName    [bddscoutTcl.h]
  Revision    [$Revision: 545 $]
  Date        [$Date: 2019-02-11 14:07:50 +0100 (pon, 11 feb 2019) $]
  Authors     [Robert Meolic (robert@meolic.com)]
  Description [The file bddscoutTcl.h contains declarations.]
  SeeAlso     []

  Copyright   [This file is part of Bdd Scout package.
               Copyright (C) 2008, 2019 UM FERI
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

/* To inspect DLL use Visual Studio Command Prompt + DUMPBIN /EXPORTS */

#ifndef _BDDSCOUTTCL
#define _BDDSCOUTTCL

#include <tcl.h>
#include "bddscout.h"

#ifndef BUILD_BDDSCOUTINIT
#include "bddscoutDecls.h"
#endif

/*-----------------------------------------------------------------------*/
/* Constant definitions                                                  */
/*-----------------------------------------------------------------------*/

/*-----------------------------------------------------------------------*/
/* Macro definitions                                                     */
/*-----------------------------------------------------------------------*/

/*-----------------------------------------------------------------------*/
/* Type declarations                                                     */
/*-----------------------------------------------------------------------*/

/*-----------------------------------------------------------------------*/
/* Structure declarations                                                */
/*-----------------------------------------------------------------------*/

/*-----------------------------------------------------------------------*/
/* Variable declarations                                                 */
/*-----------------------------------------------------------------------*/

/*-----------------------------------------------------------------------*/
/* External functions prototypes                                         */
/*-----------------------------------------------------------------------*/

EXTERN int Bddscout_Init(Tcl_Interp *interp);
EXTERN int Bddscoutbddtraces_Init(Tcl_Interp *interp);
EXTERN int Bddscoutbra_Init(Tcl_Interp *interp);
EXTERN int Bddscoutifip_Init(Tcl_Interp *interp);

/*-----------------------------------------------------------------------*/
/* Internal functions prototypes                                         */
/*-----------------------------------------------------------------------*/

#endif  /* _BDDSCOUTTCL */

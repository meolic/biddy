/**CHeaderFile*************************************************************
  PackageName [Bdd Scout]
  Synopsis    [Bdd Scout]

  FileName    [bddscout.h]
  Revision    [$Revision: 114 $]
  Date        [$Date: 2015-12-21 16:04:00 +0100 (pon, 21 dec 2015) $]
  Authors     [Robert Meolic (robert.meolic@um.si)]
  Description [The file bddscout.h contains some declarations.]
  SeeAlso     []

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

/* To inspect DLL use Visual Studio Command Prompt + DUMPBIN /EXPORTS */

#ifndef _BDDSCOUT
#define _BDDSCOUT

#include <tcl.h>
#include <biddy.h>

/* ON MS WINDOWS + MINGW THERE HAS TO BE DEFINED MINGW */
/* ON GNU/LINUX THERE HAS TO BE DEFINED UNIX */
/* ON MACOSX THERE HAS TO BE DEFINED MACOSX */

#ifdef UNIX
#  undef EXTERN
#  define EXTERN
#  undef DATAEXTERN
#  define DATAEXTERN
#endif

#ifdef MACOSX
#  undef EXTERN
#  define EXTERN
#  undef DATAEXTERN
#  define DATAEXTERN
#endif

#if defined(MINGW) || defined(_MSC_VER)
#  undef EXTERN
#  define EXTERN __declspec (dllexport)
#  undef DATAEXTERN
#  define DATAEXTERN extern __declspec (dllexport)
#endif

#ifdef _MSC_VER
#  include <io.h>
#  include <process.h>
#else
#  include <unistd.h>
#endif

#if TCL_MAJOR_VERSION < 8
#  error "We need Tcl 8.1 or greater to build this"
#elif defined(USE_TCL_STUBS) && TCL_MAJOR_VERSION == 8 && \
   (TCL_MINOR_VERSION == 0 || \
   (TCL_MINOR_VERSION == 1 && TCL_RELEASE_LEVEL != TCL_FINAL_RELEASE))
#  error "Stubs interface doesn't work in 8.0 and alpha/beta 8.1"
#endif

/*-----------------------------------------------------------------------*/
/* Constant definitions                                                  */
/*-----------------------------------------------------------------------*/

/*-----------------------------------------------------------------------*/
/* Macro definitions                                                     */
/*-----------------------------------------------------------------------*/

#define BDDNULL NULL

/*-----------------------------------------------------------------------*/
/* Type declarations                                                     */
/*-----------------------------------------------------------------------*/

typedef Biddy_Boolean (*Bddscout_LookupFunction)(Biddy_String,Biddy_Edge*);

/*-----------------------------------------------------------------------*/
/* Structure declarations                                                */
/*-----------------------------------------------------------------------*/

/*-----------------------------------------------------------------------*/
/* Variable declarations                                                 */
/*-----------------------------------------------------------------------*/

/*-----------------------------------------------------------------------*/
/* Function prototypes                                                   */
/*-----------------------------------------------------------------------*/

extern CONST char *Bddscout_InitStubs (Tcl_Interp *interp, char *version, int exact);

#include "bddscoutDecls.h"

#endif  /* _BDDSCOUT */

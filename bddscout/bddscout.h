/**CHeaderFile*************************************************************
  PackageName [Bdd Scout]
  Synopsis    [Bdd Scout]

  FileName    [bddscout.h]
  Revision    [$Revision: 652 $]
  Date        [$Date: 2021-08-28 09:52:46 +0200 (sob, 28 avg 2021) $]
  Authors     [Robert Meolic (robert@meolic.com)]
  Description [The file bddscout.h contains declarations.]
  SeeAlso     []

  Copyright   [This file is part of Bdd Scout package.
               Copyright (C) 2008, 2019 UM FERI, Koroska cesta 46, SI-2000 Maribor, Slovenia
               Copyright (C) 2019, 2021 Robert Meolic, SI-2000 Maribor, Slovenia

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

#include <biddy.h>
#include <string.h>
#include <ctype.h>

#ifdef _MSC_VER
#  include <io.h>
#  include <process.h>
#else
#  include <unistd.h>
#endif

/* ON MS WINDOWS + MINGW THERE HAS TO BE DEFINED MINGW */
/* ON GNU/LINUX THERE HAS TO BE DEFINED UNIX */
/* ON MACOSX THERE HAS TO BE DEFINED MACOSX */

#ifdef UNIX
#  undef EXTERN
#  define EXTERN extern
#  undef DATAEXTERN
#  define DATAEXTERN extern
#endif

#ifdef MACOSX
#  undef EXTERN
#  define EXTERN extern
#  undef DATAEXTERN
#  define DATAEXTERN extern
#endif

#if defined(MINGW) || defined(_MSC_VER)
#  undef EXTERN
#  define EXTERN __declspec (dllexport)
#  undef DATAEXTERN
#  define DATAEXTERN extern __declspec (dllexport)
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

/* this should be identical as defined in biddyInt.h */
/* we have to define it because it is not exported from biddyInt.h */
typedef struct {
  int id;
  Biddy_String label;
  int x;
  int y;
  Biddy_Boolean isConstant;
} BiddyXY;

/*-----------------------------------------------------------------------*/
/* Variable declarations                                                 */
/*-----------------------------------------------------------------------*/

/*-----------------------------------------------------------------------*/
/* External functions prototypes                                         */
/*-----------------------------------------------------------------------*/

#ifdef __cplusplus
extern "C" {
#endif

EXTERN Biddy_Manager Bddscout_GetActiveManager();
EXTERN void Bddscout_ClearActiveManager();
EXTERN void Bddscout_ChangeActiveManager(Biddy_String type);

#ifdef __cplusplus
}
#endif

/*-----------------------------------------------------------------------*/
/* Internal functions prototypes                                         */
/*-----------------------------------------------------------------------*/

void BddscoutInit();
void BddscoutExit();
Biddy_Boolean BddscoutCheckFormula(Biddy_String type, Biddy_String fname);
void BddscoutCopyFormula(Biddy_String fname, Biddy_String type1, Biddy_String type2);
void BddscoutSyncFormula(Biddy_String name);
void BddscoutSyncVariable(Biddy_String name);
void BddscoutConstructBDD(int numV, Biddy_String s2, int numN, Biddy_String s4);
int BddscoutWriteBddview(const char filename[], Biddy_String dotexe, Biddy_String name);
void BddscoutListVariablesByPosition(Biddy_String *list);
void BddscoutListVariablesByName(Biddy_String *list);
void BddscoutListVariablesByOrder(Biddy_String *list);
void BddscoutListVariablesByNumber(Biddy_String *list);
void BddscoutListFormulaByName(Biddy_String *list);
void BddscoutListFormulaByUIntParameter(Biddy_String *list, unsigned int (*PF)(Biddy_Edge));
void BddscoutListFormulaByFloatParameter(Biddy_String *list, float (*PF)(Biddy_Edge));
Biddy_String BddscoutReadBDD(FILE *s);
Biddy_String BddscoutReadBF(FILE *f);

#endif  /* _BDDSCOUT */

/***************************************************************************//*!
\file biddy.h
\brief File biddy.h contains declaration of all external data structures.

### Description

    PackageName [Biddy]
    Synopsis    [Biddy provides data structures and algorithms for the
                 representation and manipulation of Boolean functions with
                 ROBDDs. A hash table is used for quick search of nodes.
                 Complement edges decreases the number of nodes. An automatic
                 garbage collection with a formulae counter is implemented.
                 Variable swapping and sifting are implemented.]

    FileName    [biddy.h]
    Revision    [$Revision: 103 $]
    Date        [$Date: 2015-10-15 14:51:19 +0200 (čet, 15 okt 2015) $]
    Authors     [Robert Meolic (robert.meolic@um.si),
                 Ales Casar (ales@homemade.net)]

### Copyright

Copyright (C) 2006, 2015 UM-FERI, Smetanova ulica 17, SI-2000 Maribor, Slovenia

Biddy is free software; you can redistribute it and/or modify it under the terms
of the GNU General Public License as published by the Free Software Foundation;
either version 2 of the License, or (at your option) any later version.

Biddy is distributed in the hope that it will be useful, but WITHOUT ANY
WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A
PARTICULAR PURPOSE. See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License along with
this program; if not, write to the Free Software Foundation, Inc., 51 Franklin
Street, Fifth Floor, Boston, MA 02110-1301 USA.

### More info

See also: biddyInt.h

*******************************************************************************/

#ifndef _BIDDY
#define _BIDDY

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <ctype.h>
#include <stdarg.h>
#include <math.h>

/* ON MS WINDOWS THERE HAS TO BE DEFINED WINDOWS */
/* ON GNU/LINUX THERE HAS TO BE DEFINED UNIX */
/* ON MACOSX THERE HAS TO BE DEFINED MACOSX */

#ifdef UNIX
#  ifndef EXTERN
#    define EXTERN extern
#  endif
#  ifndef DATAEXTERN
#    define DATAEXTERN extern
#  endif
#endif

#ifdef MACOSX
#  ifndef EXTERN
#    define EXTERN extern
#  endif
#  ifndef DATAEXTERN
#    define DATAEXTERN extern
#  endif
#endif

#ifdef WINDOWS
#  ifdef BUILD_BIDDY
#    undef EXTERN
#    define EXTERN __declspec (dllexport)
#    undef DATAEXTERN
#    define DATAEXTERN extern __declspec (dllexport)
#  else
#    ifdef USE_BIDDY
#      undef EXTERN
#      define EXTERN __declspec (dllimport)
#      undef DATAEXTERN
#      define DATAEXTERN __declspec (dllimport)
#    else
#      undef EXTERN
#      define EXTERN extern
#      undef DATAEXTERN
#      define DATAEXTERN extern
#    endif
#  endif
#endif

/*----------------------------------------------------------------------------*/
/* Constant definitions                                                       */
/*----------------------------------------------------------------------------*/

#ifndef TRUE
#  define TRUE (0 == 0)
#endif
#ifndef FALSE
#  define FALSE !TRUE
#endif

/*----------------------------------------------------------------------------*/
/* Macro definitions                                                          */
/*----------------------------------------------------------------------------*/

/*! Biddy_IsNull returns TRUE iff given BDD is a null edge */
/* biddy_null is hardcoded since Biddy v1.4 */
#define Biddy_IsNull(f) (f == NULL)

/*! Biddy_IsConstant returns TRUE iff given BDD is a constant 0 or 1 */
/* succesors should be the third and the fourth field in BiddyNode */
/* non-constant nodes should not have null edges as successors */
#define Biddy_IsConstant(f) ((((void**)((uintptr_t) f & ~((uintptr_t) 1)))[2] == NULL) && (((void**)((uintptr_t) f & ~((uintptr_t) 1)))[3] == NULL))

/*! Biddy_IsEqv returns TRUE iff given BDDs are equal. */
#define Biddy_IsEqv(f,g) (f == g)

/*! Biddy_IsEqvPointer returns TRUE iff given BDDs are equal or inverted. */
#define Biddy_IsEqvPointer(f,g) (((uintptr_t) f & ~((uintptr_t) 1)) == ((uintptr_t) g & ~((uintptr_t) 1)))

/*! Biddy_GetMark returns TRUE iff given edge is complemented. */
#define Biddy_GetMark(f) (((uintptr_t) f & (uintptr_t) 1) != 0)

/*! Biddy_SetMark makes given edge complemented. */
#define Biddy_SetMark(f) (f = (void *) ((uintptr_t) f | (uintptr_t) 1))

/*! Biddy_ClearMark makes given edge not-complemented. */
#define Biddy_ClearMark(f) (f = (void *) ((uintptr_t) f & ~((uintptr_t) 1)))

/*! Biddy_InvertMark changes complement bit of the given edge. */
#define Biddy_InvertMark(f) (f = (void *) ((uintptr_t) f ^ (uintptr_t) 1))

/*! Biddy_Not returns edge with changed complemented bit, since Biddy v1.4. */
#define Biddy_Not(f) ((void *) ((uintptr_t) f ^ (uintptr_t) 1))

/*! Biddy_NotCond returns edge with conditinonaly changed complemented bit, since Biddy v1.4. */
#define Biddy_NotCond(f,c) (c ? ((void *) ((uintptr_t) f ^ (uintptr_t) 1)) : f)

/*! Biddy_Regular returns not-complemented version of edge, since Biddy v1.4. */
/* This was called Biddy_GetPointer in the previous versions of Biddy. */
#define Biddy_Regular(f) ((void *) ((uintptr_t) f & ~((uintptr_t) 1)))

/*! Biddy_Complement returns complemented version of edge, since Biddy v1.4. */
#define Biddy_Complement(f) ((void *) ((uintptr_t) f | (uintptr_t) 1))

/*! Biddy_String2Variable finds or adds variable with a given name, since Biddy v1.4. */
#define Biddy_String2Variable(MNG,x) (Biddy_GetTopVariable(Biddy_Managed_FoaVariable(MNG,(Biddy_String)x)))

/*----------------------------------------------------------------------------*/
/* Type declarations                                                          */
/*----------------------------------------------------------------------------*/

/*! Biddy_Boolean is used for boolean values. */
typedef char Biddy_Boolean;

/*! Biddy_String is used for strings. */
typedef char *Biddy_String;

/*! Biddy_Manager is used to specify manager.
    Manager is a pointer to BiddyManager. A manager includes Node Table,
    Variable Table, Formulae Table, Ordering Table, three basic caches
    (ITE Cache, EA Cache and RC Cache), list of user's caches, formulae
    counter and some other structures needed for memory management.
    Internal structure of BiddyManager is not exported but
    must be imitated to create user's managers */
typedef void **Biddy_Manager;

/*! Biddy_Cache is used to specify user's cache table.
    Caches for different operations are different and the user is
    responsible for the correct internal structure. */
typedef void *Biddy_Cache;

/*! Biddy_Variable is used for indices in variable table. */
/* BIDDY IS NOT CAPABLE TO EFFICIENTLY USE MANY VARIABLES! */
typedef unsigned int Biddy_Variable;

/*! Biddy_Edge is a marked edge (i.e. a marked pointer to BiddyNode).
    Mark is encoded as the value of the last significant bit.
    Internal structure of BiddyNode is not visible to the user. */
typedef void* Biddy_Edge;

/*! Biddy_GCFunction is used in Biddy_AddCache to specify user's function
    which will performs garbage collection. */
typedef void (*Biddy_GCFunction)(Biddy_Manager);

/*! Biddy_LookupFunction is used in Biddy_Eval1x to specify user's function
    which will lookups in a user's formula table. */
typedef Biddy_Boolean (*Biddy_LookupFunction)(Biddy_String,Biddy_Edge*);

/*----------------------------------------------------------------------------*/
/* Structure declarations                                                     */
/*----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------*/
/* Variable declarations                                                      */
/*----------------------------------------------------------------------------*/

#ifdef __cplusplus
extern "C" {
#endif

#ifdef __cplusplus
}
#endif

/*----------------------------------------------------------------------------*/
/* Prototypes for functions exported from biddyMain.c                         */
/*----------------------------------------------------------------------------*/

#ifdef __cplusplus
extern "C" {
#endif

EXTERN void Biddy_Init();

EXTERN void Biddy_Exit();

EXTERN Biddy_String Biddy_About();

EXTERN Biddy_Edge Biddy_GetThen(Biddy_Edge f);

EXTERN Biddy_Edge Biddy_GetElse(Biddy_Edge f);

EXTERN Biddy_Variable Biddy_GetTopVariable(Biddy_Edge f);

EXTERN void Biddy_SelectNode(Biddy_Edge f);

EXTERN void Biddy_DeselectNode(Biddy_Edge f);

EXTERN Biddy_Boolean Biddy_IsSelected(Biddy_Edge f);

EXTERN void Biddy_NodeSelect(Biddy_Edge f);

EXTERN void Biddy_NodeRepair(Biddy_Edge f);

EXTERN Biddy_Edge Biddy_NOT(Biddy_Edge f);

EXTERN Biddy_Edge Biddy_TransferMark(Biddy_Edge f, Biddy_Boolean mark);

/*! Macro Biddy_GetConstantZero is defined for use with anonymous manager. */
#define Biddy_GetConstantZero() Biddy_Managed_GetConstantZero(NULL)
EXTERN Biddy_Edge Biddy_Managed_GetConstantZero(Biddy_Manager MNG);

/*! Macro Biddy_GetConstantOne is defined for use with anonymous manager. */
#define Biddy_GetConstantOne() Biddy_Managed_GetConstantOne(NULL)
EXTERN Biddy_Edge Biddy_Managed_GetConstantOne(Biddy_Manager MNG);

/*! Macro Biddy_GetVariableName is defined for use with anonymous manager. */
#define Biddy_GetVariableName(v) Biddy_Managed_GetVariableName(NULL,v)
EXTERN Biddy_String Biddy_Managed_GetVariableName(Biddy_Manager MNG, Biddy_Variable v);

/*! Macro Biddy_GetTopVariableName is defined for use with anonymous manager. */
#define Biddy_GetTopVariableName(f) Biddy_Managed_GetTopVariableName(NULL,f)
EXTERN Biddy_String Biddy_Managed_GetTopVariableName(Biddy_Manager MNG, Biddy_Edge f);

/*! Macro Biddy_GetTopVariableChar is defined for use with anonymous manager. */
#define Biddy_GetTopVariableChar(f) Biddy_Managed_GetTopVariableChar(NULL,f)
EXTERN char Biddy_Managed_GetTopVariableChar(Biddy_Manager MNG, Biddy_Edge f);

/*! Macro Biddy_ResetVariablesValue is defined for use with anonymous manager. */
#define Biddy_ResetVariablesValue() Biddy_Managed_ResetVariablesValue(NULL)
EXTERN void Biddy_Managed_ResetVariablesValue(Biddy_Manager MNG);

/*! Macro Biddy_SetVariableValue is defined for use with anonymous manager. */
#define Biddy_SetVariableValue(v,f) Biddy_Managed_SetVariableValue(NULL,v,f)
EXTERN void Biddy_Managed_SetVariableValue(Biddy_Manager MNG, Biddy_Variable v, Biddy_Edge f);

/*! Macro Biddy_IsSmaller is defined for use with anonymous manager. */
#define Biddy_IsSmaller(fv,gv) Biddy_Managed_IsSmaller(NULL,fv,gv)
EXTERN Biddy_Boolean Biddy_Managed_IsSmaller(Biddy_Manager MNG, Biddy_Variable fv, Biddy_Variable gv);

/*! Macro Biddy_AddVariable is defined for use with anonymous manager. */
#define Biddy_AddVariable() Biddy_Managed_AddVariable(NULL)
EXTERN Biddy_Edge Biddy_Managed_AddVariable(Biddy_Manager MNG);

/*! Macro Biddy_AddVariableBelow is defined for use with anonymous manager. */
#define Biddy_AddVariableBelow(v) Biddy_Managed_AddVariableBelow(NULL,v)
EXTERN Biddy_Edge Biddy_Managed_AddVariableBelow(Biddy_Manager MNG, Biddy_Variable v);

/*! Macro Biddy_AddVariableAbove is defined for use with anonymous manager. */
#define Biddy_AddVariableAbove(v) Biddy_Managed_AddVariableAbove(NULL,v)
EXTERN Biddy_Edge Biddy_Managed_AddVariableAbove(Biddy_Manager MNG, Biddy_Variable v);

/*! Macro Biddy_FoaVariable is defined for use with anonymous manager. */
#define Biddy_FoaVariable(x) Biddy_Managed_FoaVariable(NULL,x)
EXTERN Biddy_Edge Biddy_Managed_FoaVariable(Biddy_Manager MNG, Biddy_String x);

/*! Macro Biddy_FoaNode is defined for use with anonymous manager. */
#define Biddy_FoaNode(v,pf,pt,garbageAllowed) Biddy_Managed_FoaNode(NULL,v,pf,pt,garbageAllowed)
EXTERN Biddy_Edge Biddy_Managed_FoaNode(Biddy_Manager MNG, Biddy_Variable v, Biddy_Edge pf, Biddy_Edge pt, Biddy_Boolean garbageAllowed);

/*! Macro Biddy_ITE is defined for use with anonymous manager. */
#define Biddy_ITE(f,g,h) Biddy_Managed_ITE(NULL,f,g,h)
EXTERN Biddy_Edge Biddy_Managed_ITE(Biddy_Manager MNG, Biddy_Edge f, Biddy_Edge g, Biddy_Edge h);

/*! Macro Biddy_And is defined for use with anonymous manager. */
#define Biddy_And(f,g) Biddy_Managed_And(NULL,f,g)
EXTERN Biddy_Edge Biddy_Managed_And(Biddy_Manager MNG, Biddy_Edge f, Biddy_Edge g);

/*! Macro Biddy_Or is defined for use with anonymous manager. */
#define Biddy_Or(f,g) Biddy_Managed_Or(NULL,f,g)
EXTERN Biddy_Edge Biddy_Managed_Or(Biddy_Manager MNG, Biddy_Edge f, Biddy_Edge g);

/*! Macro Biddy_Nand is defined for use with anonymous manager. */
#define Biddy_Nand(f,g) Biddy_Managed_Nand(NULL,f,g)
EXTERN Biddy_Edge Biddy_Managed_Nand(Biddy_Manager MNG, Biddy_Edge f, Biddy_Edge g);

/*! Macro Biddy_Nor is defined for use with anonymous manager. */
#define Biddy_Nor(f,g) Biddy_Managed_Nor(NULL,f,g)
EXTERN Biddy_Edge Biddy_Managed_Nor(Biddy_Manager MNG, Biddy_Edge f, Biddy_Edge g);

/*! Macro Biddy_Xor is defined for use with anonymous manager. */
#define Biddy_Xor(f,g) Biddy_Managed_Xor(NULL,f,g)
EXTERN Biddy_Edge Biddy_Managed_Xor(Biddy_Manager MNG, Biddy_Edge f, Biddy_Edge g);

/*! Macro Biddy_Xnor is defined for use with anonymous manager. */
#define Biddy_Xnor(f,g) Biddy_Managed_Xnor(NULL,f,g)
EXTERN Biddy_Edge Biddy_Managed_Xnor(Biddy_Manager MNG, Biddy_Edge f, Biddy_Edge g);

/*! Macro Biddy_Leq is defined for use with anonymous manager. */
#define Biddy_Leq(f,g) Biddy_Managed_Leq(NULL,f,g)
EXTERN Biddy_Boolean Biddy_Managed_Leq(Biddy_Manager MNG, Biddy_Edge f, Biddy_Edge g);

/*! Macro Biddy_Intersect is defined for use with anonymous manager. */
#define Biddy_Intersect(f,g) Biddy_Managed_Intersect(NULL,f,g)
EXTERN Biddy_Edge Biddy_Managed_Intersect(Biddy_Manager MNG, Biddy_Edge f, Biddy_Edge g);

/*! Macro Biddy_Restrict is defined for use with anonymous manager. */
#define Biddy_Restrict(f,v,value) Biddy_Managed_Restrict(NULL,f,v,value)
EXTERN Biddy_Edge Biddy_Managed_Restrict(Biddy_Manager MNG, Biddy_Edge f, Biddy_Variable v, Biddy_Edge value);

/*! Macro Biddy_Compose is defined for use with anonymous manager. */
#define Biddy_Compose(f,v,g) Biddy_Managed_Compose(NULL,f,v,g)
EXTERN Biddy_Edge Biddy_Managed_Compose(Biddy_Manager MNG, Biddy_Edge f, Biddy_Variable v, Biddy_Edge g);

/*! Macro Biddy_E is defined for use with anonymous manager. */
#define Biddy_E(f,v) Biddy_Managed_E(NULL,f,v)
EXTERN Biddy_Edge Biddy_Managed_E(Biddy_Manager MNG, Biddy_Edge f, Biddy_Variable v);

/*! Macro Biddy_A is defined for use with anonymous manager. */
#define Biddy_A(f,v) Biddy_Managed_A(NULL,f,v)
EXTERN Biddy_Edge Biddy_Managed_A(Biddy_Manager MNG, Biddy_Edge f, Biddy_Variable v);

/*! Macro Biddy_IsVariableDependent is defined for use with anonymous manager. */
#define Biddy_IsVariableDependent(f,v) Biddy_Managed_IsVariableDependent(NULL,f,v)
EXTERN Biddy_Boolean Biddy_Managed_IsVariableDependent(Biddy_Manager MNG, Biddy_Edge f, Biddy_Variable v);

/*! Macro Biddy_ExistAbstract is defined for use with anonymous manager. */
#define Biddy_ExistAbstract(f,cube) Biddy_Managed_ExistAbstract(NULL,f,cube)
EXTERN Biddy_Edge Biddy_Managed_ExistAbstract(Biddy_Manager MNG, Biddy_Edge f, Biddy_Edge cube);

/*! Macro Biddy_UnivAbstract is defined for use with anonymous manager. */
#define Biddy_UnivAbstract(f,cube) Biddy_Managed_UnivAbstract(NULL,f,cube)
EXTERN Biddy_Edge Biddy_Managed_UnivAbstract(Biddy_Manager MNG, Biddy_Edge f, Biddy_Edge cube);

/*! Macro Biddy_AndAbstract is defined for use with anonymous manager. */
#define Biddy_AndAbstract(f,g,cube) Biddy_Managed_AndAbstract(NULL,f,g,cube)
EXTERN Biddy_Edge Biddy_Managed_AndAbstract(Biddy_Manager MNG, Biddy_Edge f, Biddy_Edge g, Biddy_Edge cube);

/*! Macro Biddy_Constrain is defined for use with anonymous manager. */
#define Biddy_Constrain(f,c) Biddy_Managed_Constrain(NULL,f,c)
EXTERN Biddy_Edge Biddy_Managed_Constrain(Biddy_Manager MNG, Biddy_Edge f, Biddy_Edge c);

/*! Macro Biddy_Simplify is defined for use with anonymous manager. */
#define Biddy_Simplify(f,c) Biddy_Managed_Simplify(NULL,f,c)
EXTERN Biddy_Edge Biddy_Managed_Simplify(Biddy_Manager MNG, Biddy_Edge f, Biddy_Edge c);

/*! Macro Biddy_Support is defined for use with anonymous manager. */
#define Biddy_Support(f) Biddy_Managed_Support(NULL,f)
EXTERN Biddy_Edge Biddy_Managed_Support(Biddy_Manager MNG, Biddy_Edge f);

/*! Macro Biddy_Replace is defined for use with anonymous manager. */
#define Biddy_Replace(f) Biddy_Managed_Replace(NULL,f)
EXTERN Biddy_Edge Biddy_Managed_Replace(Biddy_Manager MNG, Biddy_Edge f);

/*! Macro Biddy_IsOK is defined for use with anonymous manager. */
#define Biddy_IsOK(f) Biddy_Managed_IsOK(NULL,f)
EXTERN Biddy_Boolean Biddy_Managed_IsOK(Biddy_Manager MNG, Biddy_Edge f);

/*! Macro Biddy_Garbage is defined for use with anonymous manager. */
#define Biddy_Garbage() Biddy_Managed_Garbage(NULL)
EXTERN void Biddy_Managed_Garbage(Biddy_Manager MNG);

/*! Macro Biddy_Clean is defined for use with anonymous manager. */
#define Biddy_Clean() Biddy_Managed_Clean(NULL)
EXTERN void Biddy_Managed_Clean(Biddy_Manager MNG);

/*! Macro Biddy_Purge is defined for use with anonymous manager. */
#define Biddy_Purge() Biddy_Managed_Purge(NULL)
EXTERN void Biddy_Managed_Purge(Biddy_Manager MNG);

/*! Macro Biddy_PurgeAndReorder is defined for use with anonymous manager. */
#define Biddy_PurgeAndReorder(f) Biddy_Managed_PurgeAndReorder(NULL,f)
EXTERN void Biddy_Managed_PurgeAndReorder(Biddy_Manager MNG, Biddy_Edge f);

/*! Macro Biddy_Refresh is defined for use with anonymous manager. */
#define Biddy_Refresh(f) Biddy_Managed_Refresh(NULL,f)
EXTERN void Biddy_Managed_Refresh(Biddy_Manager MNG, Biddy_Edge f);

/*! Macro Biddy_ClearAll is defined for use with anonymous manager. */
#define Biddy_ClearAll() Biddy_Managed_ClearAll(NULL)
EXTERN void Biddy_Managed_ClearAll(Biddy_Manager MNG);

/*! Macro Biddy_AddCache is defined for use with anonymous manager. */
#define Biddy_AddCache(gc) Biddy_Managed_AddCache(NULL,gc)
EXTERN void Biddy_Managed_AddCache(Biddy_Manager MNG, Biddy_GCFunction gc);

/*! Macro Biddy_AddFormula is defined for use with anonymous manager. */
#define Biddy_AddFormula(x,f,count) Biddy_Managed_AddFormula(NULL,x,f,count)
EXTERN void Biddy_Managed_AddFormula(Biddy_Manager MNG, Biddy_String x, Biddy_Edge f, int count);
#define Biddy_Managed_AddTmpFormula(mng,f,count) Biddy_Managed_AddFormula(mng,NULL,f,count)
#define Biddy_Managed_AddPersistentFormula(mng,x,f) Biddy_Managed_AddFormula(mng,x,f,0)
#define Biddy_AddTmpFormula(f,count) Biddy_Managed_AddFormula(NULL,NULL,f,count)
#define Biddy_AddPersistentFormula(x,f) Biddy_Managed_AddFormula(NULL,x,f,0)

/*! Macro Biddy_FindFormula is defined for use with anonymous manager. */
#define Biddy_FindFormula(x,f) Biddy_Managed_FindFormula(NULL,x,f)
EXTERN Biddy_Boolean Biddy_Managed_FindFormula(Biddy_Manager MNG, Biddy_String x, Biddy_Edge *f);

/*! Macro Biddy_DeleteFormula is defined for use with anonymous manager. */
#define Biddy_DeleteFormula(x) Biddy_Managed_DeleteFormula(NULL,x)
EXTERN Biddy_Boolean Biddy_Managed_DeleteFormula(Biddy_Manager MNG, Biddy_String x);

/*! Macro Biddy_DeleteIthFormula is defined for use with anonymous manager. */
#define Biddy_DeleteIthFormula(x) Biddy_Managed_DeleteIthFormula(NULL,x)
EXTERN Biddy_Boolean Biddy_Managed_DeleteIthFormula(Biddy_Manager MNG, unsigned int i);

/*! Macro Biddy_GetIthFormula is defined for use with anonymous manager. */
#define Biddy_GetIthFormula(i) Biddy_Managed_GetIthFormula(NULL,i)
EXTERN Biddy_Edge Biddy_Managed_GetIthFormula(Biddy_Manager MNG, unsigned int i);

/*! Macro Biddy_GetIthFormulaName is defined for use with anonymous manager. */
#define Biddy_GetIthFormulaName(i) Biddy_Managed_GetIthFormulaName(NULL,i)
EXTERN Biddy_String Biddy_Managed_GetIthFormulaName(Biddy_Manager MNG, unsigned int i);

/*! Macro Biddy_SwapWithHigher is defined for use with anonymous manager. */
#define Biddy_SwapWithHigher(v) Biddy_Managed_SwapWithHigher(NULL,v)
EXTERN Biddy_Variable Biddy_Managed_SwapWithHigher(Biddy_Manager MNG, Biddy_Variable v);

/*! Macro Biddy_SwapWithLower is defined for use with anonymous manager. */
#define Biddy_SwapWithLower(v) Biddy_Managed_SwapWithLower(NULL,v)
EXTERN Biddy_Variable Biddy_Managed_SwapWithLower(Biddy_Manager MNG, Biddy_Variable v);

/*! Macro Biddy_Sifting is defined for use with anonymous manager. */
#define Biddy_Sifting(f) Biddy_Managed_Sifting(NULL,f)
EXTERN Biddy_Boolean Biddy_Managed_Sifting(Biddy_Manager MNG, Biddy_Edge f);

#ifdef __cplusplus
}
#endif

/* TO DO: unique */
/* Unique quantification of variables. */
/* Similar to existential quantification but uses XOR instead of OR. */

/*----------------------------------------------------------------------------*/
/* Prototypes for functions exported from biddyStat.c                         */
/*----------------------------------------------------------------------------*/

#ifdef __cplusplus
extern "C" {
#endif

EXTERN unsigned int Biddy_NodeNumber(Biddy_Edge f);

EXTERN unsigned int Biddy_NodeMaxLevel(Biddy_Edge f);

EXTERN float Biddy_NodeAvgLevel(Biddy_Edge f);

/*! Macro Biddy_VariableTableNum is defined for use with anonymous manager. */
#define Biddy_VariableTableNum() Biddy_Managed_VariableTableNum(NULL)
EXTERN Biddy_Variable Biddy_Managed_VariableTableNum(Biddy_Manager MNG);

/*! Macro Biddy_NodeTableSize is defined for use with anonymous manager. */
#define Biddy_NodeTableSize() Biddy_Managed_NodeTableSize(NULL)
EXTERN unsigned int Biddy_Managed_NodeTableSize(Biddy_Manager MNG);

/*! Macro Biddy_NodeTableMax is defined for use with anonymous manager. */
#define Biddy_NodeTableMax() Biddy_Managed_NodeTableMax(NULL)
EXTERN unsigned int Biddy_Managed_NodeTableMax(Biddy_Manager MNG);

/*! Macro Biddy_NodeTableNum is defined for use with anonymous manager. */
#define Biddy_NodeTableNum() Biddy_Managed_NodeTableNum(NULL)
EXTERN unsigned int Biddy_Managed_NodeTableNum(Biddy_Manager MNG);

/*! Macro Biddy_NodeTableNumVar is defined for use with anonymous manager. */
#define Biddy_NodeTableNumVar(v) Biddy_Managed_NodeTableNumVar(NULL,v)
EXTERN unsigned int Biddy_Managed_NodeTableNumVar(Biddy_Manager MNG, Biddy_Variable v);

/*! Macro Biddy_NodeTableNumF is defined for use with anonymous manager. */
#define Biddy_NodeTableNumF() Biddy_Managed_NodeTableNumF(NULL)
EXTERN unsigned int Biddy_Managed_NodeTableNumF(Biddy_Manager MNG);

/*! Macro Biddy_NodeTableFOA is defined for use with anonymous manager. */
#define Biddy_NodeTableFOA() Biddy_Managed_NodeTableFOA(NULL)
EXTERN unsigned long long int Biddy_Managed_NodeTableFOA(Biddy_Manager MNG);

/*! Macro Biddy_NodeTableCompare is defined for use with anonymous manager. */
#define Biddy_NodeTableCompare() Biddy_Managed_NodeTableCompare(NULL)
EXTERN unsigned long long int Biddy_Managed_NodeTableCompare(Biddy_Manager MNG);

/*! Macro Biddy_NodeTableAdd is defined for use with anonymous manager. */
#define Biddy_NodeTableAdd() Biddy_Managed_NodeTableAdd(NULL)
EXTERN unsigned long long int Biddy_Managed_NodeTableAdd(Biddy_Manager MNG);

/*! Macro Biddy_NodeTableGarbage is defined for use with anonymous manager. */
#define Biddy_NodeTableGarbage() Biddy_Managed_NodeTableGarbage(NULL)
EXTERN unsigned int Biddy_Managed_NodeTableGarbage(Biddy_Manager MNG);

/*! Macro Biddy_NodeTableGenerated is defined for use with anonymous manager. */
#define Biddy_NodeTableGenerated() Biddy_Managed_NodeTableGenerated(NULL)
EXTERN unsigned int Biddy_Managed_NodeTableGenerated(Biddy_Manager MNG);

/*! Macro Biddy_NodeTableBlockNumber is defined for use with anonymous manager. */
#define Biddy_NodeTableBlockNumber() Biddy_Managed_NodeTableBlockNumber(NULL)
EXTERN unsigned int Biddy_Managed_NodeTableBlockNumber(Biddy_Manager MNG);

/*! Macro Biddy_NodeTableSwapNumber is defined for use with anonymous manager. */
#define Biddy_NodeTableSwapNumber() Biddy_Managed_NodeTableSwapNumber(NULL)
EXTERN unsigned int Biddy_Managed_NodeTableSwapNumber(Biddy_Manager MNG);

/*! Macro Biddy_NodeTableSiftingNumber is defined for use with anonymous manager. */
#define Biddy_NodeTableSiftingNumber() Biddy_Managed_NodeTableSiftingNumber(NULL)
EXTERN unsigned int Biddy_Managed_NodeTableSiftingNumber(Biddy_Manager MNG);

/*! Macro Biddy_ListUsed is defined for use with anonymous manager. */
#define Biddy_ListUsed() Biddy_Managed_ListUsed(NULL)
EXTERN unsigned int Biddy_Managed_ListUsed(Biddy_Manager MNG);

/*! Macro Biddy_ListMaxLength is defined for use with anonymous manager. */
#define Biddy_ListMaxLength() Biddy_Managed_ListMaxLength(NULL)
EXTERN unsigned int Biddy_Managed_ListMaxLength(Biddy_Manager MNG);

/*! Macro Biddy_ListAvgLength is defined for use with anonymous manager. */
#define Biddy_ListAvgLength() Biddy_Managed_ListAvgLength(NULL)
EXTERN float Biddy_Managed_ListAvgLength(Biddy_Manager MNG);

/*! Macro Biddy_IteCacheSearch is defined for use with anonymous manager. */
#define Biddy_IteCacheSearch() Biddy_Managed_IteCacheSearch(NULL)
EXTERN unsigned long long int Biddy_Managed_IteCacheSearch(Biddy_Manager MNG);

/*! Macro Biddy_IteCacheFind is defined for use with anonymous manager. */
#define Biddy_IteCacheFind() Biddy_Managed_IteCacheFind(NULL)
EXTERN unsigned long long int Biddy_Managed_IteCacheFind(Biddy_Manager MNG);

/*! Macro Biddy_IteCacheOverwrite is defined for use with anonymous manager. */
#define Biddy_IteCacheOverwrite() Biddy_Managed_IteCacheOverwrite(NULL)
EXTERN unsigned long long int Biddy_Managed_IteCacheOverwrite(Biddy_Manager MNG);

/*! Macro Biddy_NodeNumberPlain is defined for use with anonymous manager. */
#define Biddy_NodeNumberPlain(f) Biddy_Managed_NodeNumberPlain(NULL,f)
EXTERN unsigned int Biddy_Managed_NodeNumberPlain(Biddy_Manager MNG, Biddy_Edge f);

/*! Macro Biddy_VariableNumber is defined for use with anonymous manager. */
#define Biddy_VariableNumber(f) Biddy_Managed_VariableNumber(NULL,f)
EXTERN unsigned int Biddy_Managed_VariableNumber(Biddy_Manager MNG, Biddy_Edge f);

/*! Macro Biddy_NodeVarNumber is defined for use with anonymous manager. */
#define Biddy_NodeVarNumber(f,n,v) Biddy_Managed_NodeVarNumber(NULL,f,n,v)
EXTERN void Biddy_Managed_NodeVarNumber(Biddy_Manager MNG, Biddy_Edge f, unsigned int *n, unsigned int *v);

/*! Macro Biddy_CountMinterm is defined for use with anonymous manager. */
#define Biddy_CountMinterm(f,nvars) Biddy_Managed_CountMinterm(NULL,f,nvars)
EXTERN double Biddy_Managed_CountMinterm(Biddy_Manager MNG, Biddy_Edge f, int nvars);

#ifdef __cplusplus
}
#endif

/*----------------------------------------------------------------------------*/
/* Prototypes for functions exported from biddyInOut.c                        */
/*----------------------------------------------------------------------------*/

#ifdef __cplusplus
extern "C" {
#endif

/*! Macro Biddy_Eval0 is defined for use with anonymous manager. */
#define Biddy_Eval0(s) Biddy_Managed_Eval0(NULL,s)
EXTERN Biddy_String Biddy_Managed_Eval0(Biddy_Manager MNG, Biddy_String s);

/*! Macro Biddy_Eval1 is defined for use with anonymous manager. */
#define Biddy_Eval1(s) Biddy_Managed_Eval1(NULL,s)
EXTERN Biddy_Edge Biddy_Managed_Eval1(Biddy_Manager MNG, Biddy_String s);

/*! Macro Biddy_Eval1x is defined for use with anonymous manager. */
#define Biddy_Eval1x(s,f) Biddy_Managed_Eval1x(NULL,s,f)
EXTERN Biddy_Edge Biddy_Managed_Eval1x(Biddy_Manager MNG, Biddy_String s, Biddy_LookupFunction lf);

/*! Macro Biddy_Eval2 is defined for use with anonymous manager. */
#define Biddy_Eval2(boolFunc) Biddy_Managed_Eval2(NULL,boolFunc)
EXTERN Biddy_Edge Biddy_Managed_Eval2(Biddy_Manager MNG, Biddy_String boolFunc);

/*! Macro Biddy_WriteBDD is defined for use with anonymous manager. */
#define Biddy_WriteBDD(f) Biddy_Managed_WriteBDD(NULL,f)
EXTERN void Biddy_Managed_WriteBDD(Biddy_Manager MNG, Biddy_Edge f);

/*! Macro Biddy_WriteBDDx is defined for use with anonymous manager. */
#define Biddy_WriteBDDx(filename,f,label) Biddy_Managed_WriteBDDx(NULL,filename,f,label)
EXTERN void Biddy_Managed_WriteBDDx(Biddy_Manager MNG, const char filename[], Biddy_Edge f, Biddy_String label);

/*! Macro Biddy_WriteDot is defined for use with anonymous manager. */
#define Biddy_WriteDot(filename,f,label) Biddy_Managed_WriteDot(NULL,filename,f,label)
EXTERN int Biddy_Managed_WriteDot(Biddy_Manager MNG, const char filename[], Biddy_Edge f, const char label[]);

/*! Macro Biddy_WriteDotx is defined for use with anonymous manager. */
#define Biddy_WriteDotx(filename,f,label,id) Biddy_Managed_WriteDotx(NULL,filename,f,label,id)
EXTERN int Biddy_Managed_WriteDotx(Biddy_Manager MNG, const char filename[], Biddy_Edge f, const char label[], int id);

/*! Macro Biddy_WriteTable is defined for use with anonymous manager. */
#define Biddy_WriteTable(f) Biddy_Managed_WriteTable(NULL,f)
EXTERN void Biddy_Managed_WriteTable(Biddy_Manager MNG, Biddy_Edge f);

#ifdef __cplusplus
}
#endif

/* TO DO: CNF + DIMACS BENCHMARKS */
/* http://people.sc.fsu.edu/~jburkardt/data/cnf/cnf.html */
/* http://www.dwheeler.com/essays/minisat-user-guide.html */
/* http://www.miroslav-velev.com/sat_benchmarks.html */
/* http://www.satcompetition.org/ */

#endif  /* _BIDDY */

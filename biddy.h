/***************************************************************************//*!
\file biddy.h
\brief File biddy.h contains declaration of all external data structures.

### Description

    PackageName [Biddy]
    Synopsis    [Biddy provides data structures and algorithms for the
                 representation and manipulation of Boolean functions with
                 ROBDDs, 0-sup-BDDs, and TZBDDs. A hash table is used for quick
                 search of nodes. Complement edges decreases the number of
                 nodes. An automatic garbage collection with a system age is
                 implemented. Variable swapping and sifting are implemented.]

    FileName    [biddy.h]
    Revision    [$Revision: 694 $]
    Date        [$Date: 2024-06-30 20:28:55 +0200 (ned, 30 jun 2024) $]
    Authors     [Robert Meolic (robert@meolic.com),
                 Ales Casar (ales@homemade.net)]

### Copyright

Copyright (C) 2006, 2019 UM FERI, Koroska cesta 46, SI-2000 Maribor, Slovenia.
Copyright (C) 2019, 2024 Robert Meolic, SI-2000 Maribor, Slovenia.

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
#include <stdarg.h>
#include <time.h>

/* ON MS WINDOWS + MINGW THERE HAS TO BE DEFINED MINGW */
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

#ifdef MINGW
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

#ifdef _MSC_VER
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

#ifndef EXTERN
#  define EXTERN extern
#endif
#ifndef DATAEXTERN
#  define DATAEXTERN extern
#endif

#ifndef BIDDYVERSION
#define BIDDYVERSION "2.4.1"
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

/* Supported BDD types */

#define BIDDYTYPEOBDD 1
#define BIDDYTYPENAMEOBDD "ROBDD"

#define BIDDYTYPEOBDDC 2
#define BIDDYTYPENAMEOBDDC "ROBDD WITH COMPLEMENTED EDGES"

#define BIDDYTYPEZBDD 3
#define BIDDYTYPENAMEZBDD "ZBDD"

#define BIDDYTYPEZBDDC 4
#define BIDDYTYPENAMEZBDDC "ZBDD WITH COMPLEMENTED EDGES"

#define BIDDYTYPETZBDD 5
#define BIDDYTYPENAMETZBDD "TAGGED ZBDD"

#define BIDDYTYPETZBDDC 6
#define BIDDYTYPENAMETZBDDC "TAGGED ZBDD WITH COMPLEMENTED EDGES"

#define BIDDYTYPEOFDD 7
#define BIDDYTYPENAMEOFDD "ROFDD"

#define BIDDYTYPEOFDDC 8
#define BIDDYTYPENAMEOFDDC "ROFDD WITH COMPLEMENTED EDGES"

#define BIDDYTYPEZFDD 9
#define BIDDYTYPENAMEZFDD "ZFDD"

#define BIDDYTYPEZFDDC 10
#define BIDDYTYPENAMEZFDDC "ZFDD WITH COMPLEMENTED EDGES"

#define BIDDYTYPETZFDD 11
#define BIDDYTYPENAMETZFDD "TZFDD"

#define BIDDYTYPETZFDDC 12
#define BIDDYTYPENAMETZFDDC "TZFDD WITH COMPLEMENTED EDGES"

/* Supported system statistics types */

/* unsigned int Biddy_Managed_SystemStat(Biddy_Manager MNG, unsigned int stat) */
#define BIDDYSTATVARIABLETABLENUM 1
#define BIDDYSTATFORMULATABLENUM 2
#define BIDDYSTATNODETABLESIZE 3
#define BIDDYSTATNODETABLEBLOCKNUMBER 4
#define BIDDYSTATNODETABLEGENERATED 5
#define BIDDYSTATNODETABLEMAX 6
#define BIDDYSTATNODETABLENUM 7
#define BIDDYSTATNODETABLERESIZENUMBER 8
#define BIDDYSTATNODETABLEGCNUMBER 9
#define BIDDYSTATNODETABLEGCTIME 10
#define BIDDYSTATNODETABLESWAPNUMBER 11
#define BIDDYSTATNODETABLESIFTINGNUMBER 12
#define BIDDYSTATNODETABLEDRTIME 13
#define BIDDYSTATNODETABLEITENUMBER 14
#define BIDDYSTATNODETABLEANDORNUMBER 15
#define BIDDYSTATNODETABLEXORNUMBER 16

/* unsigned long long int Biddy_Managed_SystemLongStat(...) */
#define BIDDYLONGSTATNODETABLEFOANUMBER 101
#define BIDDYLONGSTATNODETABLEFINDNUMBER 102
#define BIDDYLONGSTATNODETABLECOMPARENUMBER 103
#define BIDDYLONGSTATNODETABLEADDNUMBER 104
#define BIDDYLONGSTATNODETABLEITERECURSIVENUMBER 105
#define BIDDYLONGSTATNODETABLEANDORRECURSIVENUMBER 106
#define BIDDYLONGSTATNODETABLEXORRECURSIVENUMBER 107
#define BIDDYLONGSTATOPCACHESEARCH 108
#define BIDDYLONGSTATOPCACHEFIND 109
#define BIDDYLONGSTATOPCACHEINSERT 110
#define BIDDYLONGSTATOPCACHEOVERWRITE 111

/*----------------------------------------------------------------------------*/
/* Macro definitions                                                          */
/*----------------------------------------------------------------------------*/

/*! Biddy_IsNull returns TRUE iff given BDD is a null edge. */
/* null edge is hardcoded since Biddy v1.4 */
#define Biddy_IsNull(f) (f == NULL)

/*! Biddy_IsTerminal returns TRUE iff given edge points to the terminal node. */
/* succesors should be the third and the fourth field in BiddyNode */
/* non-terminal nodes should not have null edges as successors */
/* for reduced (minimal) OBDD and OFDD this means that the represented function is 0 or 1 */
/* for TZBDD, true is returned also for tagged edges to terminal node! */
#if UINTPTR_MAX == 0xffffffffffffffff
#define Biddy_IsTerminal(f) ((((void**)((uintptr_t) f & 0x0000fffffffffffe))[2] == NULL) && (((void**)((uintptr_t) f & 0x0000fffffffffffe))[3] == NULL))
#else
#define Biddy_IsTerminal(f) ((((void**)((uintptr_t) f & ~((uintptr_t) 1)))[2] == NULL) && (((void**)((uintptr_t) f & ~((uintptr_t) 1)))[3] == NULL))
#endif

/*! Biddy_IsEqvPointer returns TRUE iff given edges points to the same node. */
/* for OBDDs and OFDDs this means that represented functions are equal or inverted */
#define Biddy_IsEqvPointer(f,g) (((uintptr_t) f & ~((uintptr_t) 1)) == ((uintptr_t) g & ~((uintptr_t) 1)))

/*! Biddy_GetMark returns TRUE iff given edge is complemented. */
#define Biddy_GetMark(f) (((uintptr_t) f & (uintptr_t) 1) != 0)

/*! Biddy_SetMark makes given edge complemented. */
#define Biddy_SetMark(f) (f = (Biddy_Edge) ((uintptr_t) f | (uintptr_t) 1))

/*! Biddy_ClearMark makes given edge not-complemented. */
#define Biddy_ClearMark(f) (f = (Biddy_Edge) ((uintptr_t) f & ~((uintptr_t) 1)))

/*! Biddy_InvertMark changes complement bit of the given edge. */
#define Biddy_InvertMark(f) (f = (Biddy_Edge) ((uintptr_t) f ^ (uintptr_t) 1))

/*! Biddy_Inv returns edge with changed complement bit. */
/* for OBDD and OFDD this is the same as Biddy_Not */
#define Biddy_Inv(f) ((Biddy_Edge) ((uintptr_t) f ^ (uintptr_t) 1))

/*! Biddy_InvCond returns edge with conditinonaly changed complement bit. */
#define Biddy_InvCond(f,c) (c ? ((Biddy_Edge) ((uintptr_t) f ^ (uintptr_t) 1)) : f)

/*! Biddy_Regular returns not-complemented version of edge, since Biddy v1.4. */
#define Biddy_Regular(f) ((Biddy_Edge) ((uintptr_t) f & ~((uintptr_t) 1)))

/*! Biddy_Complement returns complemented version of edge, since Biddy v1.4. */
#define Biddy_Complement(f) ((Biddy_Edge) ((uintptr_t) f | (uintptr_t) 1))

/*! Biddy_GetTag returns tag used for the given edge, since Biddy v1.7. */
/* use the extended version if there are more than 32768 variables */
/* this macro assumes 64-bit architecture */
#if UINTPTR_MAX == 0xffffffffffffffff
#define Biddy_GetTag(f) ((Biddy_Variable) ((uintptr_t) f >> 48))
/* #define Biddy_GetTag(f) ((Biddy_Variable) (((uintptr_t) f >> 48) & 0x000000000000ffff)) */
#else
#define Biddy_GetTag(f) ((Biddy_Variable) 0)
#endif

/*! Biddy_SetTag adds tag to the given edge, since Biddy v1.7. */
/* CAREFULLY: you can create an invalid node! */
/* this macro assumes 64-bit architecture */
#if UINTPTR_MAX == 0xffffffffffffffff
#define Biddy_SetTag(f,t) (f = (Biddy_Edge) (((uintptr_t) f & 0x0000ffffffffffff) | ((uintptr_t) t << 48)))
#else
#define Biddy_SetTag(f,t) (f = (Biddy_Edge) 0)
#endif

/*! Biddy_ClearTag removes tag from the given edge, since Biddy v1.7. */
/* this macro assumes 64-bit architecture */
#if UINTPTR_MAX == 0xffffffffffffffff
#define Biddy_ClearTag(f) (f = (Biddy_Edge) ((uintptr_t) f & 0x0000ffffffffffff))
#else
#define Biddy_ClearTag(f) (f = (Biddy_Edge) 0)
#endif

/*! Biddy_Untagged returns untagged version of edge, since Biddy v1.7. */
/* this macro assumes 64-bit architecture */
#if UINTPTR_MAX == 0xffffffffffffffff
#define Biddy_Untagged(f) ((Biddy_Edge) ((uintptr_t) f & 0x0000ffffffffffff))
#else
#define Biddy_Untagged(f) ((Biddy_Edge) 0)
#endif

/*----------------------------------------------------------------------------*/
/* Type declarations                                                          */
/*----------------------------------------------------------------------------*/

/*! \class Biddy_Boolean
    \brief Biddy_Boolean is used for boolean values. */
typedef char Biddy_Boolean;

/*! \class Biddy_String
    \brief Biddy_String is used for strings. */
typedef char *Biddy_String;

/*! \class Biddy_Manager
    \brief Biddy_Manager is used to specify manager.

    Manager is a pointer to BiddyManager. A manager includes Node Table,
    Variable Table, Formulae Table, Ordering Table, three basic caches
    (ITE Cache, EA Cache and RC Cache), list of user's caches, system
    age and some other structures needed for memory management.
    Internal structure of BiddyManager is not exported but
    must be imitated to create user's managers */
typedef void **Biddy_Manager;

/*! \class Biddy_Cache
    \brief Biddy_Cache is used to specify user's cache table.

    Caches for different operations are different and the user is
    responsible for the correct internal structure. */
typedef void *Biddy_Cache;

/*! \class Biddy_Variable
    \brief Biddy_Variable is used for indices in variable table. */
/* do not used PLAIN for tagged graphs */
/* for tagged graphs sizeof(Biddy_Variable) must not be larger than 16 bits = unsigned short int */
#ifdef PLAIN
typedef unsigned int Biddy_Variable;
#else
typedef unsigned short int Biddy_Variable;
#endif

/*! \class Biddy_Edge
    \brief Biddy_Edge is a marked edge (i.e. a marked pointer to BiddyNode).

    Mark is encoded as the value of the last significant bit.
    For TZBDDs and TZFDDs, edges are tagged. Tag is a 16 bit number
    (unsigned short int) whish is stored in the highest part of
    the pointer (this is safe because only 48 bits are used).
    TZBDDs and TZFDDs are supported only on 64-bits architectures.
    Internal structure of BiddyNode is not visible to the user. */
typedef void* Biddy_Edge;

/*! \class Biddy_GCFunction
    \brief Biddy_GCFunction is used in Biddy_AddCache to specify user's function
            which will performs garbage collection. */
typedef void (*Biddy_GCFunction)(Biddy_Manager);

/*! \class Biddy_LookupFunction
    \brief Biddy_LookupFunction is used in Biddy_Eval1x to specify user's function
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

/* 1 */
/*! Macros Biddy_Init and Biddy_InitAnonymous will initialize anonymous manager.*/
EXTERN void Biddy_InitMNG(Biddy_Manager *mng, int bddtype);
#define Biddy_Init() Biddy_InitMNG(NULL,BIDDYTYPEOBDD)
#define Biddy_InitAnonymous(bddtype) Biddy_InitMNG(NULL,bddtype)

/* 2 */
/*! Macro Biddy_Exit will delete anonymous manager. */
EXTERN void Biddy_ExitMNG(Biddy_Manager *mng);
#define Biddy_Exit() Biddy_ExitMNG(NULL)

/* 3 */
EXTERN Biddy_String Biddy_About();

/* 4 */
/*! Macro Biddy_GetManagerType is defined for use with anonymous manager. */
#define Biddy_GetManagerType() Biddy_Managed_GetManagerType(NULL)
EXTERN int Biddy_Managed_GetManagerType(Biddy_Manager MNG);

/* 5 */
/*! Macro Biddy_GetManagerName is defined for use with anonymous manager. */
#define Biddy_GetManagerName() Biddy_Managed_GetManagerName(NULL)
EXTERN Biddy_String Biddy_Managed_GetManagerName(Biddy_Manager MNG);

/* 6 */
/*! Macro Biddy_SetManagerParameters is defined for use with anonymous manager. */
#define Biddy_SetManagerParameters(gcr,gcrF,gcrX,rr,rrF,rrX,st,cst) Biddy_Managed_SetManagerParameters(NULL,gcr,gcrF,gcrX,rr,rrF,rrX,st,cst)
EXTERN void Biddy_Managed_SetManagerParameters(Biddy_Manager MNG, float gcr, float gcrF, float gcrX, float rr, float rrF, float rrX, float st, float cst);

/* 7 */
/*! Macro Biddy_Managed_GetThen is defined for your convenience. */
#define Biddy_Managed_GetThen(MNG,f) Biddy_GetThen(f)
EXTERN Biddy_Edge Biddy_GetThen(Biddy_Edge f);

/* 8 */
/*! Macro Biddy_Managed_GetElse is defined for your convenience. */
#define Biddy_Managed_GetElse(MNG,f) Biddy_GetElse(f)
EXTERN Biddy_Edge Biddy_GetElse(Biddy_Edge f);

/* 9 */
/*! Macro Biddy_Managed_GetTopVariable is defined for your convenience. */
#define Biddy_Managed_GetTopVariable(MNG,f) Biddy_GetTopVariable(f)
EXTERN Biddy_Variable Biddy_GetTopVariable(Biddy_Edge f);

/* 10 */
/*! Macro Biddy_IsEqv is defined for use with anonymous manager. */
#define Biddy_IsEqv(f1,MNG2,f2) Biddy_Managed_IsEqv(NULL,f1,MNG2,f2)
EXTERN Biddy_Boolean Biddy_Managed_IsEqv(Biddy_Manager MNG1, Biddy_Edge f1, Biddy_Manager MNG2, Biddy_Edge f2);

/* 11 */
/*! Macro Biddy_SelectNode is defined for use with anonymous manager. */
#define Biddy_SelectNode(f) Biddy_Managed_SelectNode(NULL,f)
EXTERN void Biddy_Managed_SelectNode(Biddy_Manager MNG, Biddy_Edge f);

/* 12 */
/*! Macro Biddy_DeselectNode is defined for use with anonymous manager. */
#define Biddy_DeselectNode(f) Biddy_Managed_DeselectNode(NULL,f)
EXTERN void Biddy_Managed_DeselectNode(Biddy_Manager MNG, Biddy_Edge f);

/* 13 */
/*! Macro Biddy_IsSelected is defined for use with anonymous manager. */
#define Biddy_IsSelected(f) Biddy_Managed_IsSelected(NULL,f)
EXTERN Biddy_Boolean Biddy_Managed_IsSelected(Biddy_Manager MNG, Biddy_Edge f);

/* 14 */
/*! Macro Biddy_SelectFunction is defined for use with anonymous manager. */
#define Biddy_SelectFunction(f) Biddy_Managed_SelectFunction(NULL,f)
EXTERN void Biddy_Managed_SelectFunction(Biddy_Manager MNG, Biddy_Edge f);

/* 15 */
/*! Macro Biddy_DeselectAll is defined for use with anonymous manager. */
#define Biddy_DeselectAll() Biddy_Managed_DeselectAll(NULL)
EXTERN void Biddy_Managed_DeselectAll(Biddy_Manager MNG);

/* 16 */
/*! Macro Biddy_GetTerminal is defined for use with anonymous manager. */
#define Biddy_GetTerminal() Biddy_Managed_GetTerminal(NULL)
EXTERN Biddy_Edge Biddy_Managed_GetTerminal(Biddy_Manager MNG);

/* 17 */
/*! Macro Biddy_GetConstantZero is defined for use with anonymous manager. */
#define Biddy_GetConstantZero() Biddy_Managed_GetConstantZero(NULL)
EXTERN Biddy_Edge Biddy_Managed_GetConstantZero(Biddy_Manager MNG);
#define Biddy_Managed_GetEmptySet(MNG) Biddy_Managed_GetConstantZero(MNG)
#define Biddy_GetEmptySet() Biddy_Managed_GetConstantZero(NULL)

/* 18 */
/*! Macro Biddy_GetConstantOne is defined for use with anonymous manager. */
#define Biddy_GetConstantOne() Biddy_Managed_GetConstantOne(NULL)
EXTERN Biddy_Edge Biddy_Managed_GetConstantOne(Biddy_Manager MNG);
#define Biddy_Managed_GetUniversalSet(MNG) Biddy_Managed_GetConstantOne(MNG)
#define Biddy_GetUniversalSet() Biddy_Managed_GetConstantOne(NULL)

/* 19 */
/*! Macro Biddy_GetBaseSet is defined for use with anonymous manager. */
#define Biddy_GetBaseSet() Biddy_Managed_GetBaseSet(NULL)
EXTERN Biddy_Edge Biddy_Managed_GetBaseSet(Biddy_Manager MNG);

/* 20 */
/*! Macro Biddy_GetVariable is defined for use with anonymous manager. */
#define Biddy_GetVariable(x) Biddy_Managed_GetVariable(NULL,x)
EXTERN Biddy_Variable Biddy_Managed_GetVariable(Biddy_Manager MNG, Biddy_String x);

/* 21 */
/*! Macro Biddy_GetLowestVariable is defined for use with anonymous manager. */
#define Biddy_GetLowestVariable() Biddy_Managed_GetLowestVariable(NULL)
EXTERN Biddy_Variable Biddy_Managed_GetLowestVariable(Biddy_Manager MNG);

/* 22 */
/*! Macro Biddy_GetIthVariable is defined for use with anonymous manager. */
#define Biddy_GetIthVariable(i) Biddy_Managed_GetIthVariable(NULL,i)
EXTERN Biddy_Variable Biddy_Managed_GetIthVariable(Biddy_Manager MNG, Biddy_Variable i);

/* 23 */
/*! Macro Biddy_GetPrevVariable is defined for use with anonymous manager. */
#define Biddy_GetPrevVariable(v) Biddy_Managed_GetPrevVariable(NULL,v)
EXTERN Biddy_Variable Biddy_Managed_GetPrevVariable(Biddy_Manager MNG, Biddy_Variable v);

/* 24 */
/*! Macro Biddy_GetNextVariable is defined for use with anonymous manager. */
#define Biddy_GetNextVariable(v) Biddy_Managed_GetNextVariable(NULL,v)
EXTERN Biddy_Variable Biddy_Managed_GetNextVariable(Biddy_Manager MNG, Biddy_Variable v);

/* 25 */
/*! Macro Biddy_GetVariableEdge is defined for use with anonymous manager. */
#define Biddy_GetVariableEdge(v) Biddy_Managed_GetVariableEdge(NULL,v)
EXTERN Biddy_Edge Biddy_Managed_GetVariableEdge(Biddy_Manager MNG, Biddy_Variable v);

/* 26 */
/*! Macro Biddy_GetElementEdge is defined for use with anonymous manager. */
#define Biddy_GetElementEdge(v) Biddy_Managed_GetElementEdge(NULL,v)
EXTERN Biddy_Edge Biddy_Managed_GetElementEdge(Biddy_Manager MNG, Biddy_Variable v);

/* 27 */
/*! Macro Biddy_GetVariableName is defined for use with anonymous manager. */
#define Biddy_GetVariableName(v) Biddy_Managed_GetVariableName(NULL,v)
EXTERN Biddy_String Biddy_Managed_GetVariableName(Biddy_Manager MNG, Biddy_Variable v);

/* 28 */
/*! Macro Biddy_GetTopVariableEdge is defined for use with anonymous manager. */
#define Biddy_GetTopVariableEdge(f) Biddy_Managed_GetTopVariableEdge(NULL,f)
EXTERN Biddy_Edge Biddy_Managed_GetTopVariableEdge(Biddy_Manager MNG, Biddy_Edge f);

/* 29 */
/*! Macro Biddy_GetTopVariableName is defined for use with anonymous manager. */
#define Biddy_GetTopVariableName(f) Biddy_Managed_GetTopVariableName(NULL,f)
EXTERN Biddy_String Biddy_Managed_GetTopVariableName(Biddy_Manager MNG, Biddy_Edge f);

/* 30 */
/*! Macro Biddy_GetTopVariableChar is defined for use with anonymous manager. */
#define Biddy_GetTopVariableChar(f) Biddy_Managed_GetTopVariableChar(NULL,f)
EXTERN char Biddy_Managed_GetTopVariableChar(Biddy_Manager MNG, Biddy_Edge f);

/* 31 */
/*! Macro Biddy_ResetVariablesValue is defined for use with anonymous manager. */
#define Biddy_ResetVariablesValue() Biddy_Managed_ResetVariablesValue(NULL)
EXTERN void Biddy_Managed_ResetVariablesValue(Biddy_Manager MNG);

/* 32 */
/*! Macro Biddy_SetVariableValue is defined for use with anonymous manager. */
#define Biddy_SetVariableValue(v,f) Biddy_Managed_SetVariableValue(NULL,v,f)
EXTERN void Biddy_Managed_SetVariableValue(Biddy_Manager MNG, Biddy_Variable v, Biddy_Edge f);

/* 33 */
/*! Macro Biddy_GetVariableValue is defined for use with anonymous manager. */
#define Biddy_GetVariableValue(v) Biddy_Managed_GetVariableValue(NULL,v)
EXTERN Biddy_Edge Biddy_Managed_GetVariableValue(Biddy_Manager MNG, Biddy_Variable v);

/* 34 */
/*! Macro Biddy_ClearVariablesData is defined for use with anonymous manager. */
#define Biddy_ClearVariablesData() Biddy_Managed_ClearVariablesData(NULL)
EXTERN void Biddy_Managed_ClearVariablesData(Biddy_Manager MNG);

/* 35 */
/*! Macro Biddy_SetVariableData is defined for use with anonymous manager. */
#define Biddy_SetVariableData(v,x) Biddy_Managed_SetVariableData(NULL,v,x)
EXTERN void Biddy_Managed_SetVariableData(Biddy_Manager MNG, Biddy_Variable v, void *x);

/* 36 */
/*! Macro Biddy_GetVariableData is defined for use with anonymous manager. */
#define Biddy_GetVariableData(v) Biddy_Managed_GetVariableData(NULL,v)
EXTERN void *Biddy_Managed_GetVariableData(Biddy_Manager MNG, Biddy_Variable v);

/* 37 */
/*! Macro Biddy_Eval is defined for use with anonymous manager. */
#define Biddy_Eval(f) Biddy_Managed_Eval(NULL,f)
EXTERN Biddy_Boolean Biddy_Managed_Eval(Biddy_Manager MNG, Biddy_Edge f);

/* 38 */
/*! Macro Biddy_EvalProbability is defined for use with anonymous manager. */
#define Biddy_EvalProbability(f) Biddy_Managed_EvalProbability(NULL,f)
EXTERN double Biddy_Managed_EvalProbability(Biddy_Manager MNG, Biddy_Edge f);

/* 39 */
/*! Macro Biddy_IsSmaller is defined for use with anonymous manager. */
#define Biddy_IsSmaller(fv,gv) Biddy_Managed_IsSmaller(NULL,fv,gv)
EXTERN Biddy_Boolean Biddy_Managed_IsSmaller(Biddy_Manager MNG, Biddy_Variable fv, Biddy_Variable gv);

/* 40 */
/*! Macro Biddy_IsLowest is defined for use with anonymous manager. */
#define Biddy_IsLowest(v) Biddy_Managed_IsLowest(NULL,v)
EXTERN Biddy_Boolean Biddy_Managed_IsLowest(Biddy_Manager MNG, Biddy_Variable v);

/* 41 */
/*! Macro Biddy_IsHighest is defined for use with anonymous manager. */
#define Biddy_IsHighest(v) Biddy_Managed_IsHighest(NULL,v)
EXTERN Biddy_Boolean Biddy_Managed_IsHighest(Biddy_Manager MNG, Biddy_Variable v);

/* 42 */
/*! Macro Biddy_FoaVariable is defined for use with anonymous manager. */
#define Biddy_FoaVariable(x,varelem) Biddy_Managed_FoaVariable(NULL,x,varelem)
EXTERN Biddy_Variable Biddy_Managed_FoaVariable(Biddy_Manager MNG, Biddy_String x, Biddy_Boolean varelem);

/* 43 */
/*! Macro Biddy_ChangeVariableName is defined for use with anonymous manager. */
#define Biddy_ChangeVariableName(v,x) Biddy_Managed_ChangeVariableName(NULL,v,x)
EXTERN void Biddy_Managed_ChangeVariableName(Biddy_Manager MNG, Biddy_Variable v, Biddy_String x);

/* 44 */
/*! Macro Biddy_AddVariableByName is defined for use with anonymous manager. */
/*! Macros Biddy_Managed_AddVariable and Biddy_AddVariable are defined for creating numbered variables. */
/*! Macros Biddy_Managed_AddVariableEdge and Biddy_AddVariableEdge also create numbered variables but return the variable edge. */
#define Biddy_AddVariableByName(x) Biddy_Managed_AddVariableByName(NULL,x)
EXTERN Biddy_Variable Biddy_Managed_AddVariableByName(Biddy_Manager MNG, Biddy_String x);
#define Biddy_Managed_AddVariable(MNG) Biddy_Managed_AddVariableByName(MNG,NULL)
#define Biddy_AddVariable() Biddy_Managed_AddVariableByName(NULL,NULL)
#define Biddy_Managed_AddVariableEdge(MNG) Biddy_Managed_GetVariableEdge(MNG,Biddy_Managed_AddVariableByName(MNG,NULL))
#define Biddy_AddVariableEdge() Biddy_Managed_GetVariableEdge(NULL,Biddy_Managed_AddVariableByName(NULL,NULL))

/* 45 */
/*! Macro Biddy_AddElementByName is defined for use with anonymous manager. */
/*! Macros Biddy_Managed_AddElement and Biddy_AddElement are defined for creating numbered elements. */
/*! Macros Biddy_Managed_AddElementEdge and Biddy_AddElementEdge also create numbered elements but return the element edge. */
#define Biddy_AddElementByName(x) Biddy_Managed_AddElementByName(NULL,x)
EXTERN Biddy_Variable Biddy_Managed_AddElementByName(Biddy_Manager MNG, Biddy_String x);
#define Biddy_Managed_AddElement(MNG) Biddy_Managed_AddElementByName(MNG,NULL)
#define Biddy_AddElement() Biddy_Managed_AddElementByName(NULL,NULL)
#define Biddy_Managed_AddElementEdge(MNG) Biddy_Managed_GetElementEdge(MNG,Biddy_Managed_AddElementByName(MNG,NULL))
#define Biddy_AddElementEdge() Biddy_Managed_GetElementEdge(NULL,Biddy_Managed_AddElementByName(NULL,NULL))

/* 46 */
/*! Macro Biddy_AddVariableBelow is defined for use with anonymous manager. */
#define Biddy_AddVariableBelow(v) Biddy_Managed_AddVariableBelow(NULL,v)
EXTERN Biddy_Edge Biddy_Managed_AddVariableBelow(Biddy_Manager MNG, Biddy_Variable v);

/* 47 */
/*! Macro Biddy_AddVariableAbove is defined for use with anonymous manager. */
#define Biddy_AddVariableAbove(v) Biddy_Managed_AddVariableAbove(NULL,v)
EXTERN Biddy_Edge Biddy_Managed_AddVariableAbove(Biddy_Manager MNG, Biddy_Variable v);

/* 48 */
/*! Macro Biddy_TransferMark is defined for use with anonymous manager. */
/*! For OBDD, use macro Biddy_InvCond. */
#define Biddy_TransferMark(f,mark,leftright) Biddy_Managed_TransferMark(NULL,f,mark,leftright)
EXTERN Biddy_Edge Biddy_Managed_TransferMark(Biddy_Manager MNG, Biddy_Edge f, Biddy_Boolean mark, Biddy_Boolean leftright);

/* 49 */
/*! Macro Biddy_IncTag is defined for use with anonymous manager. */
#define Biddy_IncTag(f) Biddy_Managed_IncTag(NULL,f)
EXTERN Biddy_Edge Biddy_Managed_IncTag(Biddy_Manager MNG, Biddy_Edge f);

/* 50 */
/*! Macro Biddy_TaggedFoaNode is defined for use with anonymous manager. */
#define Biddy_TaggedFoaNode(v,pf,pt,ptag,garbageAllowed) Biddy_Managed_TaggedFoaNode(NULL,v,pf,pt,ptag,garbageAllowed)
EXTERN Biddy_Edge Biddy_Managed_TaggedFoaNode(Biddy_Manager MNG, Biddy_Variable v, Biddy_Edge pf, Biddy_Edge pt, Biddy_Variable ptag, Biddy_Boolean garbageAllowed);
#define Biddy_Managed_FoaNode(MNG,v,pf,pt,garbageAllowed) Biddy_Managed_TaggedFoaNode(MNG,v,pf,pt,v,garbageAllowed)
#define Biddy_FoaNode(v,pf,pt,garbageAllowed) Biddy_Managed_TaggedFoaNode(NULL,v,pf,pt,v,garbageAllowed)

/* 51 */
/*! Macro Biddy_IsOK is defined for use with anonymous manager. */
#define Biddy_IsOK(f) Biddy_Managed_IsOK(NULL,f)
EXTERN Biddy_Boolean Biddy_Managed_IsOK(Biddy_Manager MNG, Biddy_Edge f);

/* 52 */
/*! Macro Biddy_GC is defined for use with anonymous manager. */
/*! Macros Biddy_Managed_AutoGC, Biddy_AutoGC, Biddy_Managed_ForceGC, and Biddy_ForceGC are useful variants. */
#define Biddy_GC(targetLT,targetGEQ,purge,total) Biddy_Managed_GC(NULL,targetLT,targetGEQ,purge,total)
EXTERN void Biddy_Managed_GC(Biddy_Manager MNG, Biddy_Variable targetLT, Biddy_Variable targetGEQ, Biddy_Boolean purge, Biddy_Boolean total);
#define Biddy_Managed_AutoGC(MNG) Biddy_Managed_GC(MNG,0,0,FALSE,FALSE)
#define Biddy_AutoGC() Biddy_Managed_GC(NULL,0,0,FALSE,FALSE)
#define Biddy_Managed_ForceGC(MNG) Biddy_Managed_GC(MNG,0,0,FALSE,TRUE)
#define Biddy_ForceGC() Biddy_Managed_GC(NULL,0,0,FALSE,TRUE)

/* 53 */
/*! Macro Biddy_Clean is defined for use with anonymous manager. */
#define Biddy_Clean() Biddy_Managed_Clean(NULL)
EXTERN void Biddy_Managed_Clean(Biddy_Manager MNG);

/* 54 */
/*! Macro Biddy_Purge is defined for use with anonymous manager. */
#define Biddy_Purge() Biddy_Managed_Purge(NULL)
EXTERN void Biddy_Managed_Purge(Biddy_Manager MNG);

/* 55 */
/*! Macro Biddy_PurgeAndReorder is defined for use with anonymous manager. */
#define Biddy_PurgeAndReorder(f,c) Biddy_Managed_PurgeAndReorder(NULL,f,c)
EXTERN void Biddy_Managed_PurgeAndReorder(Biddy_Manager MNG, Biddy_Edge f, Biddy_Boolean converge);

/* 56 */
/*! Macro Biddy_Refresh is defined for use with anonymous manager. */
#define Biddy_Refresh(f) Biddy_Managed_Refresh(NULL,f)
EXTERN void Biddy_Managed_Refresh(Biddy_Manager MNG, Biddy_Edge f);

/* 57 */
/*! Macro Biddy_AddCache is defined for use with anonymous manager. */
#define Biddy_AddCache(gc) Biddy_Managed_AddCache(NULL,gc)
EXTERN void Biddy_Managed_AddCache(Biddy_Manager MNG, Biddy_GCFunction gc);

/* 58 */
/*! Macro Biddy_AddFormula is defined for use with anonymous manager. */
/*! Macros Biddy_Managed_AddTmpFormula, Biddy_AddTmpFormula, */
/*! Biddy_Managed_AddPersistentFormula, Biddy_AddPersistentFormula, */
/*! Biddy_Managed_KeepFormula, Biddy_KeepFormula, */
/*! Biddy_Managed_KeepFormulaProlonged, Biddy_KeepFormulaProlonged, */
/*! Biddy_Managed_KeepFormulaUntilPurge, and Biddy_KeepFormulaUntilPurge */
/*! are defined to simplify formulae management. */
#define Biddy_AddFormula(x,f,c) Biddy_Managed_AddFormula(NULL,x,f,c)
EXTERN unsigned int Biddy_Managed_AddFormula(Biddy_Manager MNG, Biddy_String x, Biddy_Edge f, int c);
#define Biddy_Managed_AddTmpFormula(MNG,x,f) Biddy_Managed_AddFormula(MNG,x,f,-1)
#define Biddy_Managed_AddPersistentFormula(MNG,x,f) Biddy_Managed_AddFormula(MNG,x,f,0)
#define Biddy_Managed_KeepFormula(MNG,f) Biddy_Managed_AddFormula(MNG,NULL,f,1)
#define Biddy_Managed_KeepFormulaProlonged(MNG,f,c) Biddy_Managed_AddFormula(MNG,NULL,f,c)
#define Biddy_Managed_KeepFormulaUntilPurge(MNG,f) Biddy_Managed_AddFormula(MNG,NULL,f,0)
#define Biddy_AddTmpFormula(x,f) Biddy_Managed_AddFormula(NULL,x,f,-1)
#define Biddy_AddPersistentFormula(x,f) Biddy_Managed_AddFormula(NULL,x,f,0)
#define Biddy_KeepFormula(f) Biddy_Managed_AddFormula(NULL,NULL,f,1)
#define Biddy_KeepFormulaProlonged(f,c) Biddy_Managed_AddFormula(NULL,NULL,f,c)
#define Biddy_KeepFormulaUntilPurge(f) Biddy_Managed_AddFormula(NULL,NULL,f,0)

/* 59 */
/*! Macro Biddy_FindFormula is defined for use with anonymous manager. */
#define Biddy_FindFormula(x,idx,f) Biddy_Managed_FindFormula(NULL,x,idx,f)
EXTERN Biddy_Boolean Biddy_Managed_FindFormula(Biddy_Manager MNG, Biddy_String x, unsigned int *idx, Biddy_Edge *f);

/* 60 */
/*! Macro Biddy_DeleteFormula is defined for use with anonymous manager. */
#define Biddy_DeleteFormula(x) Biddy_Managed_DeleteFormula(NULL,x)
EXTERN Biddy_Boolean Biddy_Managed_DeleteFormula(Biddy_Manager MNG, Biddy_String x);

/* 61 */
/*! Macro Biddy_DeleteIthFormula is defined for use with anonymous manager. */
#define Biddy_DeleteIthFormula(x) Biddy_Managed_DeleteIthFormula(NULL,x)
EXTERN Biddy_Boolean Biddy_Managed_DeleteIthFormula(Biddy_Manager MNG, unsigned int i);

/* 62 */
/*! Macro Biddy_GetIthFormula is defined for use with anonymous manager. */
#define Biddy_GetIthFormula(i) Biddy_Managed_GetIthFormula(NULL,i)
EXTERN Biddy_Edge Biddy_Managed_GetIthFormula(Biddy_Manager MNG, unsigned int i);

/* 63 */
/*! Macro Biddy_GetIthFormulaName is defined for use with anonymous manager. */
#define Biddy_GetIthFormulaName(i) Biddy_Managed_GetIthFormulaName(NULL,i)
EXTERN Biddy_String Biddy_Managed_GetIthFormulaName(Biddy_Manager MNG, unsigned int i);

/* 64 */
/*! Macro Biddy_GetOrdering is defined for use with anonymous manager. */
#define Biddy_GetOrdering() Biddy_Managed_GetOrdering(NULL)
EXTERN Biddy_String Biddy_Managed_GetOrdering(Biddy_Manager MNG);

/* 65 */
/*! Macro Biddy_SetOrdering is defined for use with anonymous manager. */
#define Biddy_SetOrdering(ordering) Biddy_Managed_SetOrdering(NULL,ordering)
EXTERN void Biddy_Managed_SetOrdering(Biddy_Manager MNG, Biddy_String ordering);

/* 66 */
/*! Macro Biddy_SetAlphabeticOrdering is defined for use with anonymous manager. */
#define Biddy_SetAlphabeticOrdering() Biddy_Managed_SetAlphabeticOrdering(NULL)
EXTERN void Biddy_Managed_SetAlphabeticOrdering(Biddy_Manager MNG);

/* 67 */
/*! Macro Biddy_SwapWithHigher is defined for use with anonymous manager. */
#define Biddy_SwapWithHigher(v) Biddy_Managed_SwapWithHigher(NULL,v)
EXTERN Biddy_Variable Biddy_Managed_SwapWithHigher(Biddy_Manager MNG, Biddy_Variable v);

/* 68 */
/*! Macro Biddy_SwapWithLower is defined for use with anonymous manager. */
#define Biddy_SwapWithLower(v) Biddy_Managed_SwapWithLower(NULL,v)
EXTERN Biddy_Variable Biddy_Managed_SwapWithLower(Biddy_Manager MNG, Biddy_Variable v);

/* 69 */
/*! Macro Biddy_Sifting is defined for use with anonymous manager. */
#define Biddy_Sifting(f,c) Biddy_Managed_Sifting(NULL,f,c)
EXTERN Biddy_Boolean Biddy_Managed_Sifting(Biddy_Manager MNG, Biddy_Edge f, Biddy_Boolean converge);

/* 70 */
/*! Macro Biddy_MinimizeBDD is defined for use with anonymous manager. */
#define Biddy_MinimizeBDD(f) Biddy_Managed_MinimizeBDD(NULL,f)
EXTERN void Biddy_Managed_MinimizeBDD(Biddy_Manager MNG, Biddy_String name);

/* 71 */
/*! Macro Biddy_MaximizeBDD is defined for use with anonymous manager. */
#define Biddy_MaximizeBDD(f) Biddy_Managed_MaximizeBDD(NULL,f)
EXTERN void Biddy_Managed_MaximizeBDD(Biddy_Manager MNG, Biddy_String name);

/* 72 */
/*! Macros Biddy_Copy and Biddy_CopyFrom are defined for use with anonymous manager. */
#define Biddy_Copy(MNG2,f) Biddy_Managed_Copy(NULL,MNG2,f)
#define Biddy_CopyFrom(MNG1,f) Biddy_Managed_Copy(MNG1,NULL,f)
EXTERN Biddy_Edge Biddy_Managed_Copy(Biddy_Manager MNG1, Biddy_Manager MNG2, Biddy_Edge f);

/* 73 */
/*! Macros Biddy_CopyFormulaTo and Biddy_CopyFormulaFrom are defined for use */
/*! with anonymous manager. */
#define Biddy_CopyFormulaTo(MNG2,x) Biddy_Managed_CopyFormula(NULL,MNG2,x)
#define Biddy_CopyFormulaFrom(MNG1,x) Biddy_Managed_CopyFormula(MNG1,NULL,x)
EXTERN void Biddy_Managed_CopyFormula(Biddy_Manager MNG1, Biddy_Manager MNG2, Biddy_String x);

/* 74 */
/*! Macro Biddy_ConstructBDD is defined for use with anonymous manager. */
#define Biddy_ConstructBDD(numV,varlist,numN,nodelist) Biddy_Managed_ConstructBDD(NULL,numV,varlist,numV,nodelist)
EXTERN Biddy_Edge Biddy_Managed_ConstructBDD(Biddy_Manager MNG, int numV, Biddy_String varlist, int numN, Biddy_String nodelist);

#ifdef __cplusplus
}
#endif

/*----------------------------------------------------------------------------*/
/* Prototypes for functions exported from biddyOp.c                           */
/*----------------------------------------------------------------------------*/

#ifdef __cplusplus
extern "C" {
#endif

/* 75 */
/*! Macro Biddy_Not is defined for use with anonymous manager. */
/*! For OBDD and OFDD, use macro Biddy_Inv. */
#define Biddy_Not(f) Biddy_Managed_Not(NULL,f)
EXTERN Biddy_Edge Biddy_Managed_Not(Biddy_Manager MNG, Biddy_Edge f);

/* 76 */
/*! Macro Biddy_ITE is defined for use with anonymous manager. */
#define Biddy_ITE(f,g,h) Biddy_Managed_ITE(NULL,f,g,h)
EXTERN Biddy_Edge Biddy_Managed_ITE(Biddy_Manager MNG, Biddy_Edge f, Biddy_Edge g, Biddy_Edge h);

/* 77 */
/*! Macro Biddy_And is defined for use with anonymous manager. */
/*! Macros Biddy_Managed_Intersect and Biddy_Intersect are defined for set manipulation. */
#define Biddy_And(f,g) Biddy_Managed_And(NULL,f,g)
EXTERN Biddy_Edge Biddy_Managed_And(Biddy_Manager MNG, Biddy_Edge f, Biddy_Edge g);
#define Biddy_Managed_Intersect(MNG,f,g) Biddy_Managed_And(MNG,f,g)
#define Biddy_Intersect(f,g) Biddy_Managed_And(NULL,f,g)

/* 78 */
/*! Macro Biddy_Or is defined for use with anonymous manager. */
/*! Macros Biddy_Managed_Union and Biddy_Union are defined for set manipulation. */
#define Biddy_Or(f,g) Biddy_Managed_Or(NULL,f,g)
EXTERN Biddy_Edge Biddy_Managed_Or(Biddy_Manager MNG, Biddy_Edge f, Biddy_Edge g);
#define Biddy_Managed_Union(MNG,f,g) Biddy_Managed_Or(MNG,f,g)
#define Biddy_Union(f,g) Biddy_Managed_Or(NULL,f,g)

/* 79 */
/*! Macro Biddy_Nand is defined for use with anonymous manager. */
#define Biddy_Nand(f,g) Biddy_Managed_Nand(NULL,f,g)
EXTERN Biddy_Edge Biddy_Managed_Nand(Biddy_Manager MNG, Biddy_Edge f, Biddy_Edge g);

/* 80 */
/*! Macro Biddy_Nor is defined for use with anonymous manager. */
#define Biddy_Nor(f,g) Biddy_Managed_Nor(NULL,f,g)
EXTERN Biddy_Edge Biddy_Managed_Nor(Biddy_Manager MNG, Biddy_Edge f, Biddy_Edge g);

/* 81 */
/*! Macro Biddy_Xor is defined for use with anonymous manager. */
#define Biddy_Xor(f,g) Biddy_Managed_Xor(NULL,f,g)
EXTERN Biddy_Edge Biddy_Managed_Xor(Biddy_Manager MNG, Biddy_Edge f, Biddy_Edge g);

/* 82 */
/*! Macro Biddy_Xnor is defined for use with anonymous manager. */
#define Biddy_Xnor(f,g) Biddy_Managed_Xnor(NULL,f,g)
EXTERN Biddy_Edge Biddy_Managed_Xnor(Biddy_Manager MNG, Biddy_Edge f, Biddy_Edge g);

/* 83 */
/*! Macro Biddy_Leq is defined for use with anonymous manager. */
#define Biddy_Leq(f,g) Biddy_Managed_Leq(NULL,f,g)
EXTERN Biddy_Edge Biddy_Managed_Leq(Biddy_Manager MNG, Biddy_Edge f, Biddy_Edge g);

/* 84 */
/*! Macro Biddy_Gt is defined for use with anonymous manager. */
#define Biddy_Gt(f,g) Biddy_Managed_Gt(NULL,f,g)
EXTERN Biddy_Edge Biddy_Managed_Gt(Biddy_Manager MNG, Biddy_Edge f, Biddy_Edge g);
#define Biddy_Managed_Diff(MNG,f,g) Biddy_Managed_Gt(MNG,f,g)
#define Biddy_Diff(f,g) Biddy_Managed_Gt(NULL,f,g)

/* 85 */
/*! Macro Biddy_IsLeq is defined for use with anonymous manager. */
#define Biddy_IsLeq(f,g) Biddy_Managed_IsLeq(NULL,f,g)
EXTERN Biddy_Boolean Biddy_Managed_IsLeq(Biddy_Manager MNG, Biddy_Edge f, Biddy_Edge g);

/* 86 */
/* This is used to calculate cofactors f|{v=0} and f|{v=1}. */
/*! Macro Biddy_Restrict is defined for use with anonymous manager. */
#define Biddy_Restrict(f,v,value) Biddy_Managed_Restrict(NULL,f,v,value)
EXTERN Biddy_Edge Biddy_Managed_Restrict(Biddy_Manager MNG, Biddy_Edge f, Biddy_Variable v, Biddy_Boolean value);

/* 87 */
/*! Macro Biddy_Compose is defined for use with anonymous manager. */
#define Biddy_Compose(f,g,v) Biddy_Managed_Compose(NULL,f,g,v)
EXTERN Biddy_Edge Biddy_Managed_Compose(Biddy_Manager MNG, Biddy_Edge f, Biddy_Edge g, Biddy_Variable v);

/* 88 */
/*! Macro Biddy_XYCompose is defined for use with anonymous manager. */
#define Biddy_XYCompose(f) Biddy_Managed_XYCompose(NULL,f)
EXTERN Biddy_Edge Biddy_Managed_XYCompose(Biddy_Manager MNG, Biddy_Edge f);

/* 89 */
/*! Macro Biddy_E is defined for use with anonymous manager. */
#define Biddy_E(f,v) Biddy_Managed_E(NULL,f,v)
EXTERN Biddy_Edge Biddy_Managed_E(Biddy_Manager MNG, Biddy_Edge f, Biddy_Variable v);

/* 90 */
/*! Macro Biddy_A is defined for use with anonymous manager. */
#define Biddy_A(f,v) Biddy_Managed_A(NULL,f,v)
EXTERN Biddy_Edge Biddy_Managed_A(Biddy_Manager MNG, Biddy_Edge f, Biddy_Variable v);

/* 91 */
/*! Macro Biddy_IsVariableDependent is defined for use with anonymous manager. */
#define Biddy_IsVariableDependent(f,v) Biddy_Managed_IsVariableDependent(NULL,f,v)
EXTERN Biddy_Boolean Biddy_Managed_IsVariableDependent(Biddy_Manager MNG, Biddy_Edge f, Biddy_Variable v);

/* 92 */
/*! Macro Biddy_ExistAbstract is defined for use with anonymous manager. */
#define Biddy_ExistAbstract(f,cube) Biddy_Managed_ExistAbstract(NULL,f,cube)
EXTERN Biddy_Edge Biddy_Managed_ExistAbstract(Biddy_Manager MNG, Biddy_Edge f, Biddy_Edge cube);

/* 93 */
/*! Macro Biddy_ExistAndAbstract is defined for use with anonymous manager. */
#define Biddy_ExistAndAbstract(f,g,cube) Biddy_Managed_ExistAndAbstract(NULL,f,g,cube)
EXTERN Biddy_Edge Biddy_Managed_ExistAndAbstract(Biddy_Manager MNG, Biddy_Edge f, Biddy_Edge g, Biddy_Edge cube);

/* 94 */
/*! Macro Biddy_UnivAbstract is defined for use with anonymous manager. */
#define Biddy_UnivAbstract(f,cube) Biddy_Managed_UnivAbstract(NULL,f,cube)
EXTERN Biddy_Edge Biddy_Managed_UnivAbstract(Biddy_Manager MNG, Biddy_Edge f, Biddy_Edge cube);

/* 95 */
/*! Macro Biddy_DiffAbstract is defined for use with anonymous manager. */
#define Biddy_DiffAbstract(f,cube) Biddy_Managed_DiffAbstract(NULL,f,cube)
EXTERN Biddy_Edge Biddy_Managed_DiffAbstract(Biddy_Manager MNG, Biddy_Edge f, Biddy_Edge cube);

/* 96 */
/*! Macro Biddy_YesNoAbstract is defined for use with anonymous manager. */
#define Biddy_YesNoAbstract(type,f,cube) Biddy_Managed_YesNoAbstract(NULL,type,f,cube)
EXTERN Biddy_Edge Biddy_Managed_YesNoAbstract(Biddy_Manager MNG, Biddy_Boolean type, Biddy_Edge f, Biddy_Edge cube);

/* 97 */
/*! Macro Biddy_Constrain is defined for use with anonymous manager. */
#define Biddy_Constrain(f,c) Biddy_Managed_Constrain(NULL,f,c)
EXTERN Biddy_Edge Biddy_Managed_Constrain(Biddy_Manager MNG, Biddy_Edge f, Biddy_Edge c);

/* 98 */
/* This is Coudert and Madre's restrict function */
/*! Macro Biddy_Simplify is defined for use with anonymous manager. */
#define Biddy_Simplify(f,c) Biddy_Managed_Simplify(NULL,f,c)
EXTERN Biddy_Edge Biddy_Managed_Simplify(Biddy_Manager MNG, Biddy_Edge f, Biddy_Edge c);

/* 99 */
/*! Macro Biddy_Median is defined for use with anonymous manager. */
#define Biddy_Median(f,g,h) Biddy_Managed_Median(NULL,f,g,h)
EXTERN Biddy_Edge Biddy_Managed_Median(Biddy_Manager MNG, Biddy_Edge f, Biddy_Edge g, Biddy_Edge h);

/* 100 */
/*! Macro Biddy_Support is defined for use with anonymous manager. */
#define Biddy_Support(f) Biddy_Managed_Support(NULL,f)
EXTERN Biddy_Edge Biddy_Managed_Support(Biddy_Manager MNG, Biddy_Edge f);

/* 101 */
/*! Macro Biddy_ReplaceByKeyword is defined for use with anonymous manager. */
/*! Macros Biddy_Managed_Replace and Biddy_Replace are variants */
/*! with less effective cache table */
#define Biddy_ReplaceByKeyword(f,keyword) Biddy_Managed_ReplaceByKeyword(NULL,f,keyword)
EXTERN Biddy_Edge Biddy_Managed_ReplaceByKeyword(Biddy_Manager MNG, Biddy_Edge f, Biddy_String keyword);
#define Biddy_Managed_Replace(MNG,f) Biddy_Managed_ReplaceByKeyword(MNG,f,NULL)
#define Biddy_Replace(f) Biddy_Managed_ReplaceByKeyword(NULL,f,NULL)

/* 102 */
/*! Macro Biddy_Change is defined for use with anonymous manager. */
#define Biddy_Change(f,v) Biddy_Managed_Change(NULL,f,v)
EXTERN Biddy_Edge Biddy_Managed_Change(Biddy_Manager MNG, Biddy_Edge f, Biddy_Variable v);

/* 103 */
/* This is used to calculate f*v and f*(-v) */
/* Using the provided macros, Biddy_Managed_Quotient and Biddy_Quotient are not implemented optimally */
/*! Macro Biddy_VarSubset is defined for use with anonymous manager. */
#define Biddy_VarSubset(f,v,value) Biddy_Managed_VarSubset(NULL,f,v,value)
EXTERN Biddy_Edge Biddy_Managed_VarSubset(Biddy_Manager MNG, Biddy_Edge f, Biddy_Variable v, Biddy_Boolean value);
#define Biddy_Managed_Subset0(MNG,f,v) Biddy_Managed_VarSubset(MNG,f,v,FALSE)
#define Biddy_Subset0(f,v) Biddy_Managed_VarSubset(NULL,f,v,FALSE)
#define Biddy_Managed_Subset1(MNG,f,v) Biddy_Managed_VarSubset(MNG,f,v,TRUE)
#define Biddy_Subset1(f,v) Biddy_Managed_VarSubset(NULL,f,v,TRUE)
#define Biddy_Managed_Quotient(MNG,f,v) Biddy_Managed_Change(MNG,Biddy_Managed_VarSubset(MNG,f,v,TRUE),v)
#define Biddy_Quotient(f,v) Biddy_Change(Biddy_VarSubset(f,v,TRUE),v)
#define Biddy_Managed_Remainder(MNG,f,v) Biddy_Managed_VarSubset(MNG,f,v,FALSE)
#define Biddy_Remainder(f,v) Biddy_Managed_VarSubset(NULL,f,v,FALSE)

/* 104 */
/*! Macro Biddy_ElementAbstract is defined for use with anonymous manager. */
#define Biddy_ElementAbstract(f,v) Biddy_Managed_ElementAbstract(NULL,f,v)
EXTERN Biddy_Edge Biddy_Managed_ElementAbstract(Biddy_Manager MNG, Biddy_Edge f, Biddy_Variable v);

/* 105 */
/*! Macro Biddy_Product is defined for use with anonymous manager. */
#define Biddy_Product(f,g) Biddy_Managed_Product(NULL,f,g)
EXTERN Biddy_Edge Biddy_Managed_Product(Biddy_Manager MNG, Biddy_Edge f, Biddy_Edge g);

/* 106 */
/*! Macro Biddy_SelectiveProduct is defined for use with anonymous manager. */
#define Biddy_SelectiveProduct(f,g,pncube) Biddy_Managed_SelectiveProduct(NULL,f,g,pncube)
EXTERN Biddy_Edge Biddy_Managed_SelectiveProduct(Biddy_Manager MNG, Biddy_Edge f, Biddy_Edge g, Biddy_Edge pncube);

/* 107 */
/* This is restriction operation for combination sets. */
/*! Macro Biddy_Supset is defined for use with anonymous manager. */
#define Biddy_Supset(f,g) Biddy_Managed_Supset(NULL,f,g)
EXTERN Biddy_Edge Biddy_Managed_Supset(Biddy_Manager MNG, Biddy_Edge f, Biddy_Edge g);

/* 108 */
/* This is permission operation for combination sets. */
/*! Macro Biddy_Subset is defined for use with anonymous manager. */
#define Biddy_Subset(f,g) Biddy_Managed_Subset(NULL,f,g)
EXTERN Biddy_Edge Biddy_Managed_Subset(Biddy_Manager MNG, Biddy_Edge f, Biddy_Edge g);

/* 109 */
/*! Macro Biddy_Permitsym is defined for use with anonymous manager. */
#define Biddy_Permitsym(f,n) Biddy_Managed_Permitsym(NULL,f,n)
EXTERN Biddy_Edge Biddy_Managed_Permitsym(Biddy_Manager MNG, Biddy_Edge f, unsigned int n);

/* 110 */
/*! Macro Biddy_Stretch is defined for use with anonymous manager. */
#define Biddy_Stretch(f) Biddy_Managed_Stretch(NULL,f)
EXTERN Biddy_Edge Biddy_Managed_Stretch(Biddy_Manager MNG, Biddy_Edge f);

/* 111 */
/*! Macro Biddy_CreateMinterm is defined for use with anonymous manager. */
#define Biddy_CreateMinterm(support,x) Biddy_Managed_CreateMinterm(NULL,support,x)
EXTERN Biddy_Edge Biddy_Managed_CreateMinterm(Biddy_Manager MNG, Biddy_Edge support, long long unsigned int x);

/* 112 */
/*! Macro Biddy_CreateFunction is defined for use with anonymous manager. */
#define Biddy_CreateFunction(support,x) Biddy_Managed_CreateFunction(NULL,support,x)
EXTERN Biddy_Edge Biddy_Managed_CreateFunction(Biddy_Manager MNG, Biddy_Edge support, long long unsigned int x);

/* 113 */
/*! Macro Biddy_RandomFunction is defined for use with anonymous manager. */
#define Biddy_RandomFunction(support,ratio) Biddy_Managed_RandomFunction(NULL,support,ratio)
EXTERN Biddy_Edge Biddy_Managed_RandomFunction(Biddy_Manager MNG, Biddy_Edge support, double ratio);

/* 114 */
/*! Macro Biddy_RandomSet is defined for use with anonymous manager. */
#define Biddy_RandomSet(unit,ratio) Biddy_Managed_RandomSet(NULL,unit,ratio)
EXTERN Biddy_Edge Biddy_Managed_RandomSet(Biddy_Manager MNG, Biddy_Edge unit, double ratio);

/* 115 */
/*! Macro Biddy_ExtractMinterm and Biddy_ExtractMintermWithSupport are defined for use with anonymous manager. */
#define Biddy_ExtractMinterm(f) Biddy_Managed_ExtractMinterm(NULL,NULL,f)
#define Biddy_ExtractMintermWithSupport(support,f) Biddy_Managed_ExtractMinterm(NULL,support,f)
EXTERN Biddy_Edge Biddy_Managed_ExtractMinterm(Biddy_Manager MNG, Biddy_Edge support, Biddy_Edge f);

/* 116 */
/*! Macro Biddy_Dual is defined for use with anonymous manager. */
#define Biddy_Dual(f,neg) Biddy_Managed_Dual(NULL,f,neg)
EXTERN Biddy_Edge Biddy_Managed_Dual(Biddy_Manager MNG, Biddy_Edge f, Biddy_Boolean neg);

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

/* 117 */
/*! Macro Biddy_CountNodes(f) is defined for use with anonymous manager. */
#define Biddy_CountNodes(f) Biddy_Managed_CountNodes(NULL,f)
EXTERN unsigned int Biddy_Managed_CountNodes(Biddy_Manager MNG, Biddy_Edge f);

/* 118 */
/*! Macro Biddy_Managed_MaxLevel(MNG,f) is defined for your convenience. */
#define Biddy_Managed_MaxLevel(MNG,f) Biddy_MaxLevel(f)
EXTERN unsigned int Biddy_MaxLevel(Biddy_Edge f);

/* 119 */
/*! Macro Biddy_Managed_AvgLevel(MNG,f) is defined for your convenience. */
#define Biddy_Managed_AvgLevel(MNG,f) Biddy_AvgLevel(f)
EXTERN float Biddy_AvgLevel(Biddy_Edge f);

/* 120 */
/*! Macro Biddy_SystemStat is defined for use with anonymous manager. */
#define Biddy_SystemStat(stat) Biddy_Managed_SystemStat(NULL,stat)
EXTERN unsigned int Biddy_Managed_SystemStat(Biddy_Manager MNG, unsigned int stat);
#define Biddy_Managed_VariableTableNum(MNG) Biddy_Managed_SystemStat(MNG,BIDDYSTATVARIABLETABLENUM)
#define Biddy_Managed_FormulaTableNum(MNG) Biddy_Managed_SystemStat(MNG,BIDDYSTATFORMULATABLENUM)
#define Biddy_Managed_NodeTableSize(MNG) Biddy_Managed_SystemStat(MNG,BIDDYSTATNODETABLESIZE)
#define Biddy_Managed_NodeTableBlockNumber(MNG) Biddy_Managed_SystemStat(MNG,BIDDYSTATNODETABLEBLOCKNUMBER)
#define Biddy_Managed_NodeTableGenerated(MNG) Biddy_Managed_SystemStat(MNG,BIDDYSTATNODETABLEGENERATED)
#define Biddy_Managed_NodeTableMax(MNG) Biddy_Managed_SystemStat(MNG,BIDDYSTATNODETABLEMAX)
#define Biddy_Managed_NodeTableNum(MNG) Biddy_Managed_SystemStat(MNG,BIDDYSTATNODETABLENUM)
#define Biddy_Managed_NodeTableResizeNumber(MNG) Biddy_Managed_SystemStat(MNG,BIDDYSTATNODETABLERESIZENUMBER)
#define Biddy_Managed_NodeTableGCNumber(MNG) Biddy_Managed_SystemStat(MNG,BIDDYSTATNODETABLEGCNUMBER)
#define Biddy_Managed_NodeTableGCTime(MNG) Biddy_Managed_SystemStat(MNG,BIDDYSTATNODETABLEGCTIME)
#define Biddy_Managed_NodeTableSwapNumber(MNG) Biddy_Managed_SystemStat(MNG,BIDDYSTATNODETABLESWAPNUMBER)
#define Biddy_Managed_NodeTableSiftingNumber(MNG) Biddy_Managed_SystemStat(MNG,BIDDYSTATNODETABLESIFTINGNUMBER)
#define Biddy_Managed_NodeTableDRTime(MNG) Biddy_Managed_SystemStat(MNG,BIDDYSTATNODETABLEDRTIME)
#define Biddy_Managed_NodeTableITENumber(MNG) Biddy_Managed_SystemStat(MNG,BIDDYSTATNODETABLEITENUMBER)
#define Biddy_Managed_NodeTableANDORNumber(MNG) Biddy_Managed_SystemStat(MNG,BIDDYSTATNODETABLEANDORNUMBER)
#define Biddy_Managed_NodeTableXORNumber(MNG) Biddy_Managed_SystemStat(MNG,BIDDYSTATNODETABLEXORNUMBER)
#define Biddy_VariableTableNum() Biddy_Managed_SystemStat(NULL,BIDDYSTATVARIABLETABLENUM)
#define Biddy_FormulaTableNum() Biddy_Managed_SystemStat(NULL,BIDDYSTATFORMULATABLENUM)
#define Biddy_NodeTableSize() Biddy_Managed_SystemStat(NULL,BIDDYSTATNODETABLESIZE)
#define Biddy_NodeTableBlockNumber() Biddy_Managed_SystemStat(NULL,BIDDYSTATNODETABLEBLOCKNUMBER)
#define Biddy_NodeTableGenerated() Biddy_Managed_SystemStat(NULL,BIDDYSTATNODETABLEGENERATED)
#define Biddy_NodeTableMax() Biddy_Managed_SystemStat(NULL,BIDDYSTATNODETABLEMAX)
#define Biddy_NodeTableNum() Biddy_Managed_SystemStat(NULL,BIDDYSTATNODETABLENUM)
#define Biddy_NodeTableResizeNumber() Biddy_Managed_SystemStat(NULL,BIDDYSTATNODETABLERESIZENUMBER)
#define Biddy_NodeTableGCNumber() Biddy_Managed_SystemStat(NULL,BIDDYSTATNODETABLEGCNUMBER)
#define Biddy_NodeTableGCTime() Biddy_Managed_SystemStat(NULL,BIDDYSTATNODETABLEGCTIME)
#define Biddy_NodeTableSwapNumber() Biddy_Managed_SystemStat(NULL,BIDDYSTATNODETABLESWAPNUMBER)
#define Biddy_NodeTableSiftingNumber() Biddy_Managed_SystemStat(NULL,BIDDYSTATNODETABLESIFTINGNUMBER)
#define Biddy_NodeTableDRTime() Biddy_Managed_SystemStat(NULL,BIDDYSTATNODETABLEDRTIME)
#define Biddy_NodeTableITENumber() Biddy_Managed_SystemStat(NULL,BIDDYSTATNODETABLEITENUMBER)
#define Biddy_NodeTableANDORNumber() Biddy_Managed_SystemStat(NULL,BIDDYSTATNODETABLEANDORNUMBER)
#define Biddy_NodeTableXORNumber() Biddy_Managed_SystemStat(NULL,BIDDYSTATNODETABLEXORNUMBER)

/* 121 */
/*! Macro Biddy_SystemLongStat is defined for use with anonymous manager. */
#define Biddy_SystemLongStat(longstat) Biddy_Managed_SystemLongStat(NULL,longstat)
EXTERN unsigned long long int Biddy_Managed_SystemLongStat(Biddy_Manager MNG, unsigned int longstat);
#define Biddy_Managed_NodeTableFoaNumber(MNG) Biddy_Managed_SystemLongStat(MNG,BIDDYLONGSTATNODETABLEFOANUMBER)
#define Biddy_Managed_NodeTableFindNumber(MNG) Biddy_Managed_SystemLongStat(MNG,BIDDYLONGSTATNODETABLEFINDNUMBER)
#define Biddy_Managed_NodeTableCompareNumber(MNG) Biddy_Managed_SystemLongStat(MNG,BIDDYLONGSTATNODETABLECOMPARENUMBER)
#define Biddy_Managed_NodeTableAddNumber(MNG) Biddy_Managed_SystemLongStat(MNG,BIDDYLONGSTATNODETABLEADDNUMBER)
#define Biddy_Managed_NodeTableITERecursiveNumber(MNG) Biddy_Managed_SystemLongStat(MNG,BIDDYLONGSTATNODETABLEITERECURSIVENUMBER)
#define Biddy_Managed_NodeTableANDORRecursiveNumber(MNG) Biddy_Managed_SystemLongStat(MNG,BIDDYLONGSTATNODETABLEANDORRECURSIVENUMBER)
#define Biddy_Managed_NodeTableXORRecursiveNumber(MNG) Biddy_Managed_SystemLongStat(MNG,BIDDYLONGSTATNODETABLEXORRECURSIVENUMBER)
#define Biddy_Managed_OPCacheSearch(MNG) Biddy_Managed_SystemLongStat(MNG,BIDDYLONGSTATOPCACHESEARCH)
#define Biddy_Managed_OPCacheFind(MNG) Biddy_Managed_SystemLongStat(MNG,BIDDYLONGSTATOPCACHEFIND)
#define Biddy_Managed_OPCacheInsert(MNG) Biddy_Managed_SystemLongStat(MNG,BIDDYLONGSTATOPCACHEINSERT)
#define Biddy_Managed_OPCacheOverwrite(MNG) Biddy_Managed_SystemLongStat(MNG,BIDDYLONGSTATOPCACHEOVERWRITE)
#define Biddy_NodeTableFoaNumber() Biddy_Managed_SystemLongStat(NULL,BIDDYLONGSTATNODETABLEFOANUMBER)
#define Biddy_NodeTableFindNumber() Biddy_Managed_SystemLongStat(NULL,BIDDYLONGSTATNODETABLEFINDNUMBER)
#define Biddy_NodeTableCompareNumber() Biddy_Managed_SystemLongStat(NULL,BIDDYLONGSTATNODETABLECOMPARENUMBER)
#define Biddy_NodeTableAddNumber() Biddy_Managed_SystemLongStat(NULL,BIDDYLONGSTATNODETABLEADDNUMBER)
#define Biddy_NodeTableITERecursiveNumber() Biddy_Managed_SystemLongStat(NULL,BIDDYLONGSTATNODETABLEITERECURSIVENUMBER)
#define Biddy_NodeTableANDORRecursiveNumber() Biddy_Managed_SystemLongStat(NULL,BIDDYLONGSTATNODETABLEANDORRECURSIVENUMBER)
#define Biddy_NodeTableXORRecursiveNumber() Biddy_Managed_SystemLongStat(NULL,BIDDYLONGSTATNODETABLEXORRECURSIVENUMBER)
#define Biddy_OPCacheSearch() Biddy_Managed_SystemLongStat(NULL,BIDDYLONGSTATOPCACHESEARCH)
#define Biddy_OPCacheFind() Biddy_Managed_SystemLongStat(NULL,BIDDYLONGSTATOPCACHEFIND)
#define Biddy_OPCacheInsert() Biddy_Managed_SystemLongStat(NULL,BIDDYLONGSTATOPCACHEINSERT)
#define Biddy_OPCacheOverwrite() Biddy_Managed_SystemLongStat(NULL,BIDDYLONGSTATOPCACHEOVERWRITE)

/* 122 */
/*! Macro Biddy_NodeTableNumVar is defined for use with anonymous manager. */
#define Biddy_NodeTableNumVar(v) Biddy_Managed_NodeTableNumVar(NULL,v)
EXTERN unsigned int Biddy_Managed_NodeTableNumVar(Biddy_Manager MNG, Biddy_Variable v);

/* 123 */
/*! Macro Biddy_NodeTableGCObsoleteNumber is defined for use with anonymous manager. */
#define Biddy_NodeTableGCObsoleteNumber() Biddy_Managed_NodeTableGCObsoleteNumber(NULL)
EXTERN unsigned long long int Biddy_Managed_NodeTableGCObsoleteNumber(Biddy_Manager MNG);

/* 124 */
/*! Macro Biddy_ListUsed is defined for use with anonymous manager. */
#define Biddy_ListUsed() Biddy_Managed_ListUsed(NULL)
EXTERN unsigned int Biddy_Managed_ListUsed(Biddy_Manager MNG);

/* 125 */
/*! Macro Biddy_ListMaxLength is defined for use with anonymous manager. */
#define Biddy_ListMaxLength() Biddy_Managed_ListMaxLength(NULL)
EXTERN unsigned int Biddy_Managed_ListMaxLength(Biddy_Manager MNG);

/* 126 */
/*! Macro Biddy_ListAvgLength is defined for use with anonymous manager. */
#define Biddy_ListAvgLength() Biddy_Managed_ListAvgLength(NULL)
EXTERN float Biddy_Managed_ListAvgLength(Biddy_Manager MNG);

/* 127 */
/*! Macro Biddy_CountNodesPlain is defined for use with anonymous manager. */
#define Biddy_CountNodesPlain(f) Biddy_Managed_CountNodesPlain(NULL,f)
EXTERN unsigned int Biddy_Managed_CountNodesPlain(Biddy_Manager MNG, Biddy_Edge f);

/* 128 */
/*! Macro Biddy_DependentVariableNumber is defined for use with anonymous manager. */
#define Biddy_DependentVariableNumber(f,select) Biddy_Managed_DependentVariableNumber(NULL,f,select)
EXTERN unsigned int Biddy_Managed_DependentVariableNumber(Biddy_Manager MNG, Biddy_Edge f, Biddy_Boolean select);

/* 129 */
/*! Macro Biddy_CountComplementedEdges is defined for use with anonymous manager. */
#define Biddy_CountComplementedEdges(f) Biddy_Managed_CountComplementedEdges(NULL,f)
EXTERN unsigned int Biddy_Managed_CountComplementedEdges(Biddy_Manager MNG, Biddy_Edge f);

/* 130 */
/*! Macro Biddy_CountPaths is defined for use with anonymous manager. */
#define Biddy_CountPaths(f) Biddy_Managed_CountPaths(NULL,f)
EXTERN unsigned long long int Biddy_Managed_CountPaths(Biddy_Manager MNG, Biddy_Edge f);

/* 131 */
/*! Macro Biddy_CountMinterms is defined for use with anonymous manager. */
#define Biddy_CountMinterms(f,nvars) Biddy_Managed_CountMinterms(NULL,f,nvars)
EXTERN double Biddy_Managed_CountMinterms(Biddy_Manager MNG, Biddy_Edge f, int nvars);
#define Biddy_Managed_CountCombinations(MNG,f) Biddy_Managed_CountMinterms(MNG,f,-1)
#define Biddy_CountCombinations(f) Biddy_Managed_CountMinterms(NULL,f,-1)

/* 132 */
/*! Macro Biddy_DensityOfFunction is defined for use with anonymous manager. */
#define Biddy_DensityOfFunction(f,nvars) Biddy_Managed_DensityOfFunction(NULL,f,nvars)
EXTERN double Biddy_Managed_DensityOfFunction(Biddy_Manager MNG, Biddy_Edge f, unsigned int nvars);

/* 133 */
/*! Macro Biddy_DensityOfBDD is defined for use with anonymous manager. */
#define Biddy_DensityOfBDD(f,nvars) Biddy_Managed_DensityOfBDD(NULL,f,nvars)
EXTERN double Biddy_Managed_DensityOfBDD(Biddy_Manager MNG, Biddy_Edge f, unsigned int nvars);

/* 134 */
/*! Macro Biddy_MinNodes(f) is defined for use with anonymous manager. */
#define Biddy_MinNodes(f) Biddy_Managed_MinNodes(NULL,f)
EXTERN unsigned int Biddy_Managed_MinNodes(Biddy_Manager MNG, Biddy_Edge f);

/* 135 */
/*! Macro Biddy_MaxNodes(f) is defined for use with anonymous manager. */
#define Biddy_MaxNodes(f) Biddy_Managed_MaxNodes(NULL,f)
EXTERN unsigned int Biddy_Managed_MaxNodes(Biddy_Manager MNG, Biddy_Edge f);

/* 136 */
/*! Macro Biddy_ReadMemoryInUse is defined for use with anonymous manager. */
#define Biddy_ReadMemoryInUse() Biddy_Managed_ReadMemoryInUse(NULL)
EXTERN unsigned long long int Biddy_Managed_ReadMemoryInUse(Biddy_Manager MNG);

/* 137 */
/*! Macro Biddy_PrintInfo is defined for use with anonymous manager. */
#define Biddy_PrintInfo(f) Biddy_Managed_PrintInfo(NULL,f)
EXTERN void Biddy_Managed_PrintInfo(Biddy_Manager MNG, FILE *f);

#ifdef __cplusplus
}
#endif

/*----------------------------------------------------------------------------*/
/* Prototypes for functions exported from biddyInOut.c                        */
/*----------------------------------------------------------------------------*/

#ifdef __cplusplus
extern "C" {
#endif

/* 138 */
/*! Macro Biddy_Eval0 is defined for use with anonymous manager. */
#define Biddy_Eval0(s) Biddy_Managed_Eval0(NULL,s)
EXTERN Biddy_String Biddy_Managed_Eval0(Biddy_Manager MNG, Biddy_String s);

/* 139 */
/*! Macro Biddy_Eval1x is defined for use with anonymous manager. */
#define Biddy_Eval1x(s,lf) Biddy_Managed_Eval1x(NULL,s,lf)
EXTERN Biddy_Edge Biddy_Managed_Eval1x(Biddy_Manager MNG, Biddy_String s, Biddy_LookupFunction lf);
#define Biddy_Managed_Eval1(MNG,s) Biddy_Managed_Eval1x(MNG,s,NULL)
#define Biddy_Eval1(s) Biddy_Managed_Eval1x(NULL,s,NULL)

/* 140 */
/*! Macro Biddy_Eval2 is defined for use with anonymous manager. */
#define Biddy_Eval2(boolFunc) Biddy_Managed_Eval2(NULL,boolFunc)
EXTERN Biddy_Edge Biddy_Managed_Eval2(Biddy_Manager MNG, Biddy_String boolFunc);

/* 141 */
/*! Macro Biddy_Eval3 is defined for use with anonymous manager. */
#define Biddy_Eval3(boolFunc) Biddy_Managed_Eval3(NULL,bddlString)
EXTERN Biddy_Edge Biddy_Managed_Eval3(Biddy_Manager MNG, Biddy_String *name, Biddy_String bddlString);

/* 142 */
/*! Macro Biddy_ReadBddview is defined for use with anonymous manager. */
#define Biddy_ReadBddview(filename,name) Biddy_Managed_ReadBddview(NULL,filename,name)
EXTERN Biddy_String Biddy_Managed_ReadBddview(Biddy_Manager MNG, const char filename[], Biddy_String name);

/* 143 */
/*! Macro Biddy_ReadVerilogFile is defined for use with anonymous manager. */
#define Biddy_ReadVerilogFile(filename,prefix) Biddy_Managed_ReadVerilogFile(NULL,filename,prefix)
EXTERN void Biddy_Managed_ReadVerilogFile(Biddy_Manager MNG, const char filename[], Biddy_String prefix);

/* 144 */
/*! Macro Biddy_ReadBDDLFile is defined for use with anonymous manager. */
#define Biddy_ReadBDDLFile(filename) Biddy_Managed_ReadBDDLFile(NULL,filename)
EXTERN Biddy_String Biddy_Managed_ReadBDDLFile(Biddy_Manager MNG, const char filename[]);

/* 145 */
/*! Macro Biddy_PrintBDD is defined for use with anonymous manager. */
#define Biddy_PrintBDD(var,filename,f,label) Biddy_Managed_PrintBDD(NULL,var,filename,f,label)
EXTERN void Biddy_Managed_PrintBDD(Biddy_Manager MNG, Biddy_String *var, const char filename[], Biddy_Edge f, Biddy_String label);
#define Biddy_Managed_PrintfBDD(MNG,f) Biddy_Managed_PrintBDD(MNG,NULL,"stdout",f,NULL)
#define Biddy_Managed_SprintfBDD(MNG,var,f) Biddy_Managed_PrintBDD(MNG,var,"",f,NULL)
#define Biddy_Managed_WriteBDD(MNG,filename,f,label) Biddy_Managed_PrintBDD(MNG,NULL,filename,f,label)
#define Biddy_PrintfBDD(f) Biddy_Managed_PrintBDD(NULL,NULL,"stdout",f,NULL)
#define Biddy_SprintfBDD(var,f) Biddy_Managed_PrintBDD(NULL,var,"",f,NULL)
#define Biddy_WriteBDD(filename,f,label) Biddy_Managed_PrintBDD(NULL,NULL,filename,f,label)

/* 146 */
/*! Macro Biddy_PrintTable is defined for use with anonymous manager. */
#define Biddy_PrintTable(var,filename,f) Biddy_Managed_PrintTable(NULL,var,filename,f)
EXTERN void Biddy_Managed_PrintTable(Biddy_Manager MNG, Biddy_String *var, const char filename[], Biddy_Edge f);
#define Biddy_Managed_PrintfTable(MNG,f) Biddy_Managed_PrintTable(MNG,NULL,"stdout",f)
#define Biddy_Managed_SprintfTable(MNG,var,f) Biddy_Managed_PrintTable(MNG,var,"",f)
#define Biddy_Managed_WriteTable(MNG,filename,f) Biddy_Managed_PrintTable(MNG,NULL,filename,f)
#define Biddy_PrintfTable(f) Biddy_Managed_PrintTable(NULL,NULL,"stdout",f)
#define Biddy_SprintfTable(var,f) Biddy_Managed_PrintTable(NULL,var,"",f)
#define Biddy_WriteTable(filename,f) Biddy_Managed_PrintTable(NULL,NULL,filename,f)

/* 147 */
/*! Macro Biddy_PrintSOP is defined for use with anonymous manager. */
#define Biddy_PrintSOP(var,filename,f) Biddy_Managed_PrintSOP(NULL,var,filename,f)
EXTERN void Biddy_Managed_PrintSOP(Biddy_Manager MNG, Biddy_String *var, const char filename[], Biddy_Edge f);
#define Biddy_Managed_PrintfSOP(MNG,f) Biddy_Managed_PrintSOP(MNG,NULL,"stdout",f)
#define Biddy_Managed_SprintfSOP(MNG,var,f) Biddy_Managed_PrintSOP(MNG,var,"",f)
#define Biddy_Managed_WriteSOP(MNG,filename,f) Biddy_Managed_PrintSOP(MNG,NULL,filename,f)
#define Biddy_PrintfSOP(f) Biddy_Managed_PrintSOP(NULL,NULL,"stdout",f)
#define Biddy_SprintfSOP(var,f) Biddy_Managed_PrintSOP(NULL,var,"",f)
#define Biddy_WriteSOP(filename,f) Biddy_Managed_PrintSOP(NULL,NULL,filename,f)

/* 148 */
/*! Macro Biddy_PrintMinterms is defined for use with anonymous manager. */
#define Biddy_PrintMinterms(var,filename,f,negative) Biddy_Managed_PrintMinterms(NULL,var,filename,f,negative)
EXTERN void Biddy_Managed_PrintMinterms(Biddy_Manager MNG, Biddy_String *var, const char filename[], Biddy_Edge f, Biddy_Boolean negative);
#define Biddy_Managed_PrintfMinterms(MNG,f,negative) Biddy_Managed_PrintMinterms(MNG,NULL,"stdout",f,negative)
#define Biddy_Managed_SprintfMinterms(MNG,var,f,negative) Biddy_Managed_PrintMinterms(MNG,var,"",f,negative)
#define Biddy_Managed_WriteMinterms(MNG,filename,f,negative) Biddy_Managed_PrintMinterms(MNG,NULL,filename,f,negative)
#define Biddy_PrintfMinterms(f,negative) Biddy_Managed_PrintMinterms(NULL,NULL,"stdout",f,negative)
#define Biddy_SprintfMinterms(var,f,negative) Biddy_Managed_PrintMinterms(NULL,var,"",f,negative)
#define Biddy_WriteMinterms(filename,f,negative) Biddy_Managed_PrintMinterms(NULL,NULL,filename,f,negative)

/* 149 */
/*! Macro Biddy_WriteDot is defined for use with anonymous manager. */
#define Biddy_WriteDot(filename,f,label,id,cudd) Biddy_Managed_WriteDot(NULL,filename,f,label,id,cudd)
EXTERN unsigned int Biddy_Managed_WriteDot(Biddy_Manager MNG, const char filename[], Biddy_Edge f, const char label[], int id, Biddy_Boolean cudd);

/* 150 */
/*! Macro Biddy_WriteBddview is defined for use with anonymous manager. */
#define Biddy_WriteBddview(filename,f,label,table) Biddy_Managed_WriteBddview(NULL,filename,f,label,table)
EXTERN unsigned int Biddy_Managed_WriteBddview(Biddy_Manager MNG, const char filename[], Biddy_Edge f, const char label[], void *xytable);

#ifdef __cplusplus
}
#endif

/* TO DO: CNF + DIMACS BENCHMARKS */
/* http://people.sc.fsu.edu/~jburkardt/data/cnf/cnf.html */
/* http://www.dwheeler.com/essays/minisat-user-guide.html */
/* http://www.miroslav-velev.com/sat_benchmarks.html */
/* http://www.satcompetition.org/ */

#endif  /* _BIDDY */

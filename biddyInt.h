/***************************************************************************//*!
\file biddyInt.h
\brief File biddyInt.h contains declaration of internal data structures.

### Description

    PackageName [Biddy]
    Synopsis    [Biddy provides data structures and algorithms for the
                 representation and manipulation of Boolean functions with
                 ROBDDs, 0-sup-BDDs, and TZBDDs. A hash table is used for quick
                 search of nodes. Complement edges decreases the number of
                 nodes. An automatic garbage collection with a system age is
                 implemented. Variable swapping and sifting are implemented.]

    FileName    [biddyInt.h]
    Revision    [$Revision: 618 $]
    Date        [$Date: 2020-03-28 12:38:59 +0100 (sob, 28 mar 2020) $]
    Authors     [Robert Meolic (robert@meolic.com),
                 Ales Casar (ales@homemade.net)]

### Copyright

Copyright (C) 2006, 2019 UM FERI, Koroska cesta 46, SI-2000 Maribor, Slovenia.
Copyright (C) 2019, 2020 Robert Meolic, SI-2000 Maribor, Slovenia.

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

See also: biddy.h

*******************************************************************************/

#ifndef _BIDDYINT
#define _BIDDYINT

/*----------------------------------------------------------------------------*/
/* START OF USER SETTINGS                                                     */
/*----------------------------------------------------------------------------*/

/* define COMPACT for variant with only ROBDDs */
#define NOCOMPACT

/* define GMPFREE for variant without gmp library - it has limited functionality */
#define NOGMPFREE

/* define LOWMEMORY to use settings which try to lower memory requirements */
#define NOLOWMEMORY

/* define ESTPROJECT to use optimal settings for EST */
#define NOESTPROJECT

/* define COMPREHENSIVE to use settings for larger problems, e.g. dictionary example, pp example */
#define NOCOMPREHENSIVE

/* EST project */
#ifdef ESTPROJECT
#ifdef GMPFREE
#undef GMPFREE
#endif
#ifdef LOWMEMORY
#undef LOWMEMORY
#endif
#ifdef COMPREHENSIVE
#undef COMPREHENSIVE
#endif
#endif

/* extended stats (YES or NO) */
/* if YES then you have to use -lm for linking */
#define BIDDYEXTENDEDSTATS_NO

/* event log (YES or NO) */
#define BIDDYEVENTLOG_NO

/* define LEGACY_DOT to use CUDD style for edges: */
/* solid line: regular THEN arcs */
/* bold line: complemented THEN arcs */
/* dashed line: regular ELSE arcs */
/* dotted line: complemented ELSE arcs */
/* without LEGACY_DOT, an experimental style for edges is used */
/* this setting is NOT USED for CUDD-like generation of dot files */
#define LEGACY_DOT

/* refreshing variant during operatins on BDDs (to test the efficiency) */
/* MACRO USING IS NOT IMPLEMENTED YET */
/* VARIANT B WAS USED FROM 1.6 UNTIL 1.9, VARIANT A IS USED IN 2.0 */
/* variant A: created and reused nodes are immediately refreshed */
/* variant B: created nodes are immediately refreshed, */
/*   reused nodes are not refreshed, top node is refreshed when op finishes */
/*   this could be problematic if BDD operations are paralelized on multiple cores */
/* TO DO: check if sifting and BDD copying is correctly implemented */
#define REFRESH_VARIANT_A

/*----------------------------------------------------------------------------*/
/* END OF USER SETTINGS                                                       */
/*----------------------------------------------------------------------------*/

#include "biddy.h"
#include <assert.h>
#include <string.h>
#include <ctype.h>

/*----------------------------------------------------------------------------*/
/* Constant declarations                                                      */
/*----------------------------------------------------------------------------*/

/* max number of variables - this is hardcoded for better performance */
/* for optimal space reservation use VARMAX =  32*N */
/* variable "1" is one of these variables */
/* for TZBDDs and TZFDDs, the limit is 65536 variables on 64-bit architecture */
/* if there are more than 32768 variables then some macros in biddy.h must be changed */
/* BIDDY IS NOT EFFICIENT WITH MANY VARIABLES! */
#ifdef LOWMEMORY
#define BIDDYVARMAX 1024
#else
/* #define BIDDYVARMAX 1024 */
/* #define BIDDYVARMAX 2048 */
#define BIDDYVARMAX 4096
/* #define BIDDYVARMAX 6144 */
/* #define BIDDYVARMAX 8192 */
#endif

/* THE FOLLOWING SIZES ARE unsigned int                            */
/* Size of node and cache tables must be 2^N-1                     */
/* e.g. 2^16-1 = 65535, 2^20-1 = 1048575, 2^24-1 = 16777215        */

/* If BDD node is 24B (e.g. 32-bit GNU/Linux systems)              */
/* 256 nodes is 6 kB                                               */
/* 65536 nodes is 1.5 MB                                           */
/* 262144 nodes is 6 MB                                            */
/* 524288 nodes is 12 MB                                           */
/* 1048576 nodes is 24.0 MB                                        */
/* 2097152 nodes is 48.0 MB                                        */
/* 4194304 nodes is 96.0 MB                                        */
/* 8388608 nodes is 192.0 MB                                       */
/* 16777216 nodes is 384.0 MB                                      */

/* If BDD node is 48B (e.g. 64-bit GNU/Linux systems), then:       */
/* 256 nodes is 12 kB                                              */
/* 65536 nodes is 3.0 MB                                           */
/* 262144 nodes is 12 MB                                           */
/* 524288 nodes is 24 MB                                           */
/* 1048576 nodes is 48.0 MB                                        */
/* 2097152 nodes is 96.0 MB                                        */
/* 4194304 nodes is 192.0 MB                                       */
/* 8388608 nodes is 384.0 MB                                       */
/* 16777216 nodes is 768.0 MB                                      */

#define TINY_SIZE        1023
#define SMALL_SIZE      65535
#define MEDIUM_SIZE    262143
#define LARGE_SIZE    1048575
#define XLARGE_SIZE   2097151
#define XXLARGE_SIZE  4194303
#define XXXLARGE_SIZE 8388607
#define HUGE_SIZE    16777215

#if defined(LOWMEMORY)
/* THESE ARE SIZES FOR LOW MEMORY SETTINGS, , E.G. FOR MOBILE APPS */
#define BIDDYVARIABLETABLESIZE BIDDYVARMAX
#define BIDDYNODETABLEINITBLOCKSIZE SMALL_SIZE
#define BIDDYNODETABLELIMITBLOCKSIZE SMALL_SIZE
#define BIDDYNODETABLEINITSIZE SMALL_SIZE
#define BIDDYNODETABLELIMITSIZE MEDIUM_SIZE
#define BIDDYOPCACHESIZE SMALL_SIZE
#define BIDDYEACACHESIZE SMALL_SIZE
#define BIDDYRCCACHESIZE SMALL_SIZE
#define BIDDYREPLACECACHESIZE SMALL_SIZE
#elif defined(COMPREHENSIVE)
/* THESE ARE SIZES FOR LARGER PROBLEMS, E.G. DICTIONARY EXAMPLE, PP EXAMPLE */
#define BIDDYVARIABLETABLESIZE BIDDYVARMAX
#define BIDDYNODETABLEINITBLOCKSIZE LARGE_SIZE
#define BIDDYNODETABLELIMITBLOCKSIZE LARGE_SIZE
#define BIDDYNODETABLEINITSIZE LARGE_SIZE
#define BIDDYNODETABLELIMITSIZE HUGE_SIZE
#define BIDDYOPCACHESIZE XLARGE_SIZE
#define BIDDYEACACHESIZE XLARGE_SIZE
#define BIDDYRCCACHESIZE XLARGE_SIZE
#define BIDDYREPLACECACHESIZE SMALL_SIZE
#elif defined(ESTPROJECT)
/* THESE ARE SIZES IN EST PROJECT */
#define BIDDYVARIABLETABLESIZE BIDDYVARMAX
#define BIDDYNODETABLEINITBLOCKSIZE LARGE_SIZE
#define BIDDYNODETABLELIMITBLOCKSIZE XLARGE_SIZE
#define BIDDYNODETABLEINITSIZE LARGE_SIZE
#define BIDDYNODETABLELIMITSIZE HUGE_SIZE
#define BIDDYOPCACHESIZE LARGE_SIZE
#define BIDDYEACACHESIZE SMALL_SIZE
#define BIDDYRCCACHESIZE SMALL_SIZE
#define BIDDYREPLACECACHESIZE SMALL_SIZE
#else
/* THESE ARE DEFAULT SIZES */
#define BIDDYVARIABLETABLESIZE BIDDYVARMAX
#define BIDDYNODETABLEINITBLOCKSIZE MEDIUM_SIZE
#define BIDDYNODETABLELIMITBLOCKSIZE XLARGE_SIZE
#define BIDDYNODETABLEINITSIZE SMALL_SIZE
#define BIDDYNODETABLELIMITSIZE HUGE_SIZE
#define BIDDYOPCACHESIZE MEDIUM_SIZE
#define BIDDYEACACHESIZE SMALL_SIZE
#define BIDDYRCCACHESIZE SMALL_SIZE
#define BIDDYREPLACECACHESIZE SMALL_SIZE
#endif

/* THE FOLLOWING TRESHOLDS ARE float */
/* all values are experimentally determined */
/* gcr=1.67, gcrF=1.20, gcrX=0.91, rr=0.01, rrF=1.45, rrX=0.98 */ /* used in v1.7.1 */
/* gcr=1.32, gcrF=0.99, gcrX=1.10, rr=0.01, rrF=0.89, rrX=0.91 */ /* used in v1.7.2, v1.7.3, v1.7.4, v1.8.1, v1.8.2, v1.9.1 */
/* gcr=1.32, gcrF=0.99, gcrX=1.10, rr=0.01, rrF=0.89, rrX=0.91 */ /* used in v2.0.1 */

#define BIDDYNODETABLEGCRATIO 1.32
#define BIDDYNODETABLEGCRATIOF 0.99
#define BIDDYNODETABLEGCRATIOX 1.10
#define BIDDYNODETABLERESIZERATIO 0.01
#define BIDDYNODETABLERESIZERATIOF 0.89
#define BIDDYNODETABLERESIZERATIOX 0.91

/* THE FOLLOWING TRESHOLDS ARE float */
/* all values are experimentally determined */

#define BIDDYNODETABLESIFTINGTRESHOLD 0.95
#define BIDDYNODETABLECONVERGESIFTINGTRESHOLD 1.01
#define BIDDYNODETABLESIFTINGFACTOR 3.14

/* UINTPTR and UINTPTRSIZE are used for BiddyOrdering */
#define UINTPTR uintptr_t
#define UINTPTRSIZE (8*sizeof(UINTPTR))

/* the following constants are used in Biddy_ReadVerilogFile */
#define LINESIZE 999 /* maximum length of each input line read */
#define BUFSIZE 99999 /* maximum length of a buffer */
#define INOUTNUM 999 /* maximum number of inputs and outputs */
#define REGNUM 9999 /* maximum number of registers */
#define TOKENNUM 9999 /* maximum number of tokens */
#define GATENUM 9999 /* maximum number of gates */
#define LINENUM 99999 /* maximum number of lines */
#define WIRENUM 99999 /* maximum number of wires */

/*----------------------------------------------------------------------------*/
/* Definitions of platform or variant dependent macros                        */
/*----------------------------------------------------------------------------*/

#ifdef GMPFREE
#define mpz_t unsigned long long int *
#define mpz_init(x) x = malloc(sizeof(unsigned long long int)); *(x) = 0
#define mpz_clear(x) free(x)
#define mpz_set(x,y) *(x) = *(y)
#define mpz_set_ui(x,y) *(x) = y
#define mpz_get_d(x) GetD(*(x))
#define mpz_mul_ui(x,a,b) *(x) = *(a) * b
#define mpz_divexact_ui(x,a,b) *(x) = *(a) / b
#define mpz_add(x,a,b) *(x) = *(a) + *(b)
#define mpz_add_ui(x,a,b) *(x) = *(a) + b
#define mpz_sub(x,a,b) *(x) = *(a) - *(b)
#define mpz_sub_ui(x,a,b) *(x) = *(a) - b
#define mpz_ui_pow_ui(x,a,b) *(x) = PowUI(a,b)
#else
#include <gmp.h>
#endif

#ifdef BIDDYEVENTLOG_YES
#define ZF_LOGI(x) {static unsigned int biddystat = 0;printf(x);printf("#%u,%.2f\n",++biddystat,clock()/(1.0*CLOCKS_PER_SEC));}
#else
#define ZF_LOGI(x)
#endif

/* sleep() is not included in modern version of MINGW */
/* #define sleep(sec) (Sleep ((sec) * 1000), 0) */

#ifdef MINGW
#define sleep(sec) 0
#endif

/* this is from */
/* http://stackoverflow.com/questions/3694723/error-c3861-strcasecmp-identifier-not-found-in-visual-studio-2008 */
/* not #if defined(_WIN32) || defined(_WIN64) because we have strncasecmp in mingw */
#ifdef _MSC_VER
#define strncasecmp _strnicmp
#define strcasecmp _stricmp
#endif

/*----------------------------------------------------------------------------*/
/* Shortcuts to external macros                                               */
/*----------------------------------------------------------------------------*/

#define BiddyIsNull(f) Biddy_IsNull(f)
#define BiddyIsTerminal(f) Biddy_IsTerminal(f)
#define BiddyIsEqvPointer(f,g) Biddy_IsEqvPointer(f,g)
#define BiddyGetMark(f) Biddy_GetMark(f)
#define BiddySetMark(f) Biddy_SetMark(f)
#define BiddyClearMark(f) Biddy_ClearMark(f)
#define BiddyInvertMark(f) Biddy_InvertMark(f)
#define BiddyInv(f) Biddy_Inv(f)
#define BiddyInvCond(f,c) Biddy_InvCond(f,c)
#define BiddyRegular(f) Biddy_Regular(f)
#define BiddyComplement(f) Biddy_Complement(f)
#define BiddyGetTag(f) Biddy_GetTag(f)
#define BiddySetTag(f,t) Biddy_SetTag(f,t)
#define BiddyClearTag(f) Biddy_ClearTag(f)
#define BiddyUntagged(f) Biddy_Untagged(f)

/*----------------------------------------------------------------------------*/
/* Definitions of internal macros                                             */
/*----------------------------------------------------------------------------*/

/* Null edge, this is hardcoded since Biddy v1.4 */
#define biddyNull ((Biddy_Edge)NULL)

/* BiddyP returns the pointer used with the given edge (without tag and complement bit), since Biddy v1.7. */
#if UINTPTR_MAX == 0xffffffffffffffff
#define BiddyP(fun) ((void *)((uintptr_t) fun & 0x0000fffffffffffe))
#else
#define BiddyP(fun) ((void *)((uintptr_t) fun & ~((uintptr_t) 1)))
#endif

/* BiddyR returns not-tagged version of edge, since Biddy v1.7. */
/* tagged edges assumes 64-bit architecture */
#if UINTPTR_MAX == 0xffffffffffffffff
#define BiddyR(fun) ((void *)((uintptr_t) fun & 0x0000ffffffffffff))
#else
#define BiddyR(fun) (fun)
#endif

/* BiddyN returns the node pointed by the given edge, since Biddy v1.7. */
#if UINTPTR_MAX == 0xffffffffffffffff
#define BiddyN(fun) ((BiddyNode*)((uintptr_t) fun & 0x0000fffffffffffe))
#else
#define BiddyN(fun) ((BiddyNode*)((uintptr_t) fun & ~((uintptr_t) 1)))
#endif

/* BiddyT returns THEN successor, since Biddy v1.4 */

#define BiddyT(fun) (BiddyN(fun)->t)

/* BiddyE returns ELSE successor, since Biddy v1.4 */

#define BiddyE(fun) (BiddyN(fun)->f)

/* BiddyV returns top variable, since Biddy v1.6 */

#define BiddyV(fun) (BiddyN(fun)->v)

/* orderingtable[X,Y]==1 iff variabe X is smaller than variable Y */
/* IN THE BDD, SMALLER VARIABLES ARE ABOVE THE GREATER ONES */
/* GET_ORDER check if variabe X is smaller (topmore) than variable Y */
/* SET_ORDER set variabe X to be smaller (topmore) than variable Y */
/* CLEAR_ORDER set variabe X not to be smaller (topmore) than variable Y */
/* SET_ORDER and CLEAR_ORDER should be used simultaneously */
#define GET_ORDER(orderingtable,X,Y) ((orderingtable)[X][Y/UINTPTRSIZE]&(((UINTPTR) 1)<<(Y%UINTPTRSIZE)))!=0
#define SET_ORDER(orderingtable,X,Y) (orderingtable)[X][Y/UINTPTRSIZE] |= (((UINTPTR) 1)<<(Y%UINTPTRSIZE))
#define CLEAR_ORDER(orderingtable,X,Y) (orderingtable)[X][Y/UINTPTRSIZE] &= (~(((UINTPTR) 1)<<(Y%UINTPTRSIZE)))

/* The name of manager MNG, since Biddy v1.4. */
#define biddyManagerName ((Biddy_String)(MNG[0]))
#define biddyManagerName1 ((Biddy_String)(MNG1[0]))
#define biddyManagerName2 ((Biddy_String)(MNG2[0]))

/* Type of manager MNG, since Biddy v1.7 this is used for BDD type */
#define biddyManagerType (*((short int*)(MNG[1])))
#define biddyManagerType1 (*((short int*)(MNG1[1])))
#define biddyManagerType2 (*((short int*)(MNG2[1])))

/* Terminal node in manager MNG, since Biddy v1.7. */
#define biddyTerminal ((void *)(MNG[2]))
#define biddyTerminal1 ((void *)(MNG1[2]))
#define biddyTerminal2 ((void *)(MNG2[2]))

/* Constant 0 in manager MNG, since Biddy v1.4. */
#define biddyZero ((Biddy_Edge)(MNG[3]))
#define biddyZero1 ((Biddy_Edge)(MNG1[3]))
#define biddyZero2 ((Biddy_Edge)(MNG2[3]))

/* Constant 1 in manager MNG, since Biddy v1.4. */
#define biddyOne ((Biddy_Edge)(MNG[4]))
#define biddyOne1 ((Biddy_Edge)(MNG1[4]))
#define biddyOne2 ((Biddy_Edge)(MNG2[4]))

/* Node table in manager MNG, since Biddy v1.4. */
/* this is typecasted to (BiddyNodeTable*) and dereferenced */
/* gdb -g: p *((BiddyNodeTable*)(MNG[5])) */
/* gdb -g: p ((BiddyNodeTable*)(MNG[5])).table[i] */
/* gdb -g3: p biddyNodeTable */
/* gdb -g3: p biddyNodeTable.table[i] */
#define biddyNodeTable (*((BiddyNodeTable*)(MNG[5])))
#define biddyNodeTable1 (*((BiddyNodeTable*)(MNG1[5])))
#define biddyNodeTable2 (*((BiddyNodeTable*)(MNG2[5])))

/* Variable table in manager MNG, since Biddy v1.4. */
/* this is typecasted to (BiddyVariableTable*) and dereferenced */
/* gdb -g: p *((BiddyVariableTable*)(MNG[6])) */
/* gdb -g: p ((BiddyVariableTable*)(MNG[6])).table[i] */
/* gdb -g3: p biddyVariableTable */
/* gdb -g3: p biddyVariableTable.table[i] */
#define biddyVariableTable (*((BiddyVariableTable*)(MNG[6])))
#define biddyVariableTable1 (*((BiddyVariableTable*)(MNG1[6])))
#define biddyVariableTable2 (*((BiddyVariableTable*)(MNG2[6])))

/* Formula table in manager MNG, since Biddy v1.4. */
/* this is typecasted to (BiddyFormulaTable*) and dereferenced */
#define biddyFormulaTable (*((BiddyFormulaTable*)(MNG[7])))
#define biddyFormulaTable1 (*((BiddyFormulaTable*)(MNG1[7])))
#define biddyFormulaTable2 (*((BiddyFormulaTable*)(MNG2[7])))

/* OP Cache in manager MNG, since Biddy v1.4. */
/* this is typecasted to (BiddyOp3CacheTable*) and dereferenced */
#define biddyOPCache (*((BiddyOp3CacheTable*)(MNG[8])))
#define biddyOPCache1 (*((BiddyOp3CacheTable*)(MNG1[8])))
#define biddyOPCache2 (*((BiddyOp3CacheTable*)(MNG2[8])))

/* EA Cache in manager MNG, since Biddy v1.4. */
/* this is typecasted to (BiddyOp3CacheTable*) and dereferenced */
#define biddyEACache (*((BiddyOp3CacheTable*)(MNG[9])))
#define biddyEACache1 (*((BiddyOp3CacheTable*)(MNG1[9])))
#define biddyEACache2 (*((BiddyOp3CacheTable*)(MNG2[9])))

/* RC Cache in manager MNG, since Biddy v1.4. */
/* this is typecasted to (BiddyOp3CacheTable*) and dereferenced */
#define biddyRCCache (*((BiddyOp3CacheTable*)(MNG[10])))
#define biddyRCCache1 (*((BiddyOp3CacheTable*)(MNG1[10])))
#define biddyRCCache2 (*((BiddyOp3CacheTable*)(MNG2[10])))

/* Replace Cache in manager MNG, since Biddy v1.7. */
/* this is typecasted to (BiddyKeywordCacheTable*) and dereferenced */
#define biddyReplaceCache (*((BiddyKeywordCacheTable*)(MNG[11])))
#define biddyReplaceCache1 (*((BiddyKeywordCacheTable*)(MNG1[11])))
#define biddyReplaceCache2 (*((BiddyKeywordCacheTable*)(MNG2[11])))

/* Cache list in manager MNG, since Biddy v1.4. */
/* this is typecasted to (BiddyCacheList**) and dereferenced */
#define biddyCacheList (*((BiddyCacheList**)(MNG[12])))
#define biddyCacheList1 (*((BiddyCacheList**)(MNG1[12])))
#define biddyCacheList2 (*((BiddyCacheList**)(MNG2[12])))

/* List of free nodes in manager MNG, since Biddy v1.4. */
/* this is typecasted to (BiddyNode**) and dereferenced */
#define biddyFreeNodes (*((BiddyNode**)(MNG[13])))
#define biddyFreeNodes1 (*((BiddyNode**)(MNG1[13])))
#define biddyFreeNodes2 (*((BiddyNode**)(MNG2[13])))

/* Variable ordering in manager MNG, since Biddy v1.4. */
/* this is typecasted to (BiddyOrderingTable*) and dereferenced */
#define biddyOrderingTable (*((BiddyOrderingTable*)(MNG[14])))
#define biddyOrderingTable1 (*((BiddyOrderingTable*)(MNG1[14])))
#define biddyOrderingTable2 (*((BiddyOrderingTable*)(MNG2[14])))

/* System age in manager MNG, since Biddy v1.4. */
/* this is typecasted to (int*) and dereferenced */
#define biddySystemAge (*((unsigned int*)(MNG[15])))
#define biddySystemAge1 (*((unsigned int*)(MNG1[15])))
#define biddySystemAge2 (*((unsigned int*)(MNG2[15])))

/* Node selector in manager MNG, since Biddy v1.6 */
/* this is typecasted to (int*) and dereferenced */
#define biddySelect (*((unsigned short int*)(MNG[16])))
#define biddySelect1 (*((unsigned short int*)(MNG1[16])))
#define biddySelect2 (*((unsigned short int*)(MNG2[16])))

/* BiddyProlongOne prolonges top node of the given function, since Biddy v1.6 */
#define BiddyProlongOne(f,c) if((!(c))||(BiddyN(f)->expiry&&(BiddyN(f)->expiry<(c))))BiddyN(f)->expiry=(c)

/* BiddyRefresh make top node of the given function equal to biddySystemAge if it was smaller, manager MNG is assumed, since Biddy v1.7 */
/* BiddyDefresh make top node of the given function equal to biddySystemAge if it was greater, manager MNG is assumed, since Biddy v1.7 */
#define BiddyRefresh(f) if(BiddyN(f)->expiry&&(BiddyN(f)->expiry<(biddySystemAge)))BiddyN(f)->expiry=(biddySystemAge)
#define BiddyDefresh(f) if(BiddyN(f)->expiry&&(BiddyN(f)->expiry>(biddySystemAge)))BiddyN(f)->expiry=(biddySystemAge)

/* BiddyIsSmaller returns TRUE if the first variable is smaller (= lower = previous = above = topmore), manager MNG is assumed, since Biddy v1.7 */
#define BiddyIsSmaller(fv,gv) GET_ORDER(biddyOrderingTable,fv,gv)

/* BiddyIsLowest returns TRUE if the variable is the lowest one (lowest = topmost), manager MNG is assumed, since Biddy v1.7 */
#define BiddyIsLowest(v) (biddyVariableTable.table[v].prev >= biddyVariableTable.num)

/* BiddyIsHighest returns TRUE if the variable is the highest one ignoring the terminal node (highest = bottommost), manager MNG is assumed, since Biddy v1.7 */
#define BiddyIsHighest(v) (biddyVariableTable.table[v].next == 0)

/* BiddyIsOK is intended for debugging, only, manager MNG is assumed, since Biddy v1.7 */
#define BiddyIsOK(f) (!(BiddyN(f)->expiry) || ((BiddyN(f)->expiry) >= biddySystemAge))
#define BiddyIsOKK(f) ((!(BiddyN(f)->expiry) || ((BiddyN(f)->expiry) >= biddySystemAge)) && checkFunctionOrdering(MNG,f))

/*----------------------------------------------------------------------------*/
/* Type declarations                                                          */
/*----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------*/
/* Structure declarations                                                     */
/*----------------------------------------------------------------------------*/

/* NODE TABLE (UNIQUE TABLE) = a fixed-size hash table with chaining */
/* TYPE Biddy_Variable IS DEFINED IN biddy.h */
/* IF Biddy_Variable IS unsigned short int (2B) THEN: */
/* THE SIZE OF BiddyNode on 32-bit systems is 28 Bytes */
/* THE SIZE OF BiddyNode on 64-bit systems is 48 Bytes */
typedef struct BiddyNode {
  struct BiddyNode *prev, *next; /* !!!MUST BE FIRST AND SECOND */
  Biddy_Edge f, t; /* f = left = else, t = right = then, !!!MUST BE THIRD AND FOURTH */
  void *list; /* list of nodes (various purposes) */
  unsigned int expiry; /* expiry value */
  unsigned short int select; /* used to select node */
  Biddy_Variable v; /* index in variable table */
} BiddyNode;

typedef struct {
  BiddyNode **table;
  BiddyNode **blocktable; /* table of allocated memory blocks */
  unsigned int initsize; /* initial size of Node table */
  unsigned int size; /* current size of Node table */
  unsigned int limitsize; /* limit for the size of Node table */
  unsigned int blocknumber; /* number of allocated memory blocks */
  unsigned int initblocksize; /* the size of the first block of nodes */
  unsigned int blocksize; /* the size of the last block of nodes */
  unsigned int limitblocksize; /* limit for the size of the block of nodes */
  unsigned int generated; /* total number of generated nodes  */
  unsigned int max; /* maximal (peek) number of nodes in node table */
  unsigned int num; /* number of nodes currently in node table */
  unsigned int garbage; /* number of garbage collections */
  unsigned int swap; /* number of performed variable swapping */
  unsigned int sifting; /* number of performed dynamic reordering */
  unsigned int nodetableresize; /* number of performed node table resizing */
  unsigned int funite; /* number of calls of function Biddy_ITE */
  unsigned int funandor; /* number of calls of function Biddy_And and Biddy_Or */
  unsigned int funxor; /* number of calls of function Xor */
  clock_t gctime; /* total time spent for garbage collections */
  clock_t drtime; /* total time spent for dynamic reordering */
  float gcratio; /* do not delete nodes if the effect is to small */
  float gcratioF; /* do not delete nodes if the effect is to small */
  float gcratioX; /* do not delete nodes if the effect is to small */
  float resizeratio; /* resize Node table if there are to many nodes */
  float resizeratioF; /* resize Node table if there are to many nodes */
  float resizeratioX; /* resize Node table if there are to many nodes */
  float siftingtreshold; /* stop sifting if the size of the system grows to much */
  float convergesiftingtreshold;  /* stop one step of converging sifting if the size of the system grows to much */
  float siftingfactor; /* sifting heuristics */

#ifdef BIDDYEXTENDEDSTATS_YES
  unsigned long long int foa; /* number of calls to Biddy_FoaNode */
  unsigned long long int find; /* number of calls to findNodeTable */
  unsigned long long int compare; /* num of compared nodes in findNodeTable */
  unsigned long long int add; /* number of calls to addNodeTable (node table insertions) */
  unsigned long long int iterecursive; /* number of calls to BiddyITE (direct and recursive) */
  unsigned long long int andorrecursive; /*  number of calls to BiddyAnd and BiddyOr (direct and recursive) */
  unsigned long long int xorrecursive; /*  number of calls to BiddyXor (direct and recursive) */
  unsigned long long int *gcobsolete; /* number of obsolete nodes removed by GC */
#endif
} BiddyNodeTable;

/* VARIABLE TABLE (SYMBOL TREE) = dynamicaly allocated table */
/* variable "1" has id == 0 */
/* size of variable table must be compatible with Biddy_Variable */
/* value is used for evaluation and also for renaming variables */
/* counting variables is also used in BiddyCreateLocalInfo */
/* lastNode->list IS NOT DEFINED! */
/* YOU MUST NEVER ASSUME THAT lastNode->list = NULL */
typedef struct {
  Biddy_String name; /* name of variable */
  unsigned int num; /* number of nodes with this variable */
  unsigned int numone; /* used to count nodes with this variable in a particular function */
  unsigned int numobsolete; /* used to count obsolete nodes with this variable */
  BiddyNode *firstNode;
  BiddyNode *lastNode;
  Biddy_Variable prev; /* previous variable in the global ordering (lower, topmore) */
  Biddy_Variable next; /* next variable in the global ordering (higher, bottommore) */
  Biddy_Edge variable; /* bdd representing a single positive variable */
  Biddy_Edge element; /* bdd representing a set with a single element, i.e. {{x}} */
  Biddy_Edge value; /* value: biddyZero = 0, biddyOne = 1, reused in some algorithms */
  Biddy_Boolean selected; /* used to count variables */
  void *data; /* can be used to associate various user data with every variable */
} BiddyVariable;

typedef struct {
  Biddy_String name; /* name of variable */
  Biddy_Variable v; /* variable corresponding to the name */
} BiddyLookupVariable;

typedef struct {
  BiddyVariable *table;
  BiddyLookupVariable *lookup;
  Biddy_Variable size; /* size = Biddy_VARMAX */
  Biddy_Variable num; /* number of all variables, inc. 1 */
  Biddy_Variable numnum; /* number of numbered variables, inc. 1 */
} BiddyVariableTable;

/* ORDERING TABLE = TWO-DIMENSIONAL MATRIX OF BITS */
/* ordering table is used to define variable ordering. */
/* constant variable 1 has max order */
/* USABLE ONLY AS AN IRREFLEXIVE TRANSITIVE RELATION */
/* MATRIX'S DIMENSION IN BITS = (VARMAX) x (VARMAX) */
/* orderingtable[X,Y]==1 iff variabe X is smaller than variable Y */
/* IN THE BDD, SMALLER VARIABLES ARE ABOVE THE GREATER ONES */
typedef UINTPTR BiddyOrderingTable[BIDDYVARMAX][1+(BIDDYVARMAX-1)/UINTPTRSIZE];

/* FORMULA TABLE (FORMULAE TREE) = dynamicaly allocated table */
/* expiry = 0, deleted = FALSE -> permanently preserved formula */
/* expiry = 0, deleted = TRUE -> deleted permanently preserved formula */
/* expiry >= biddySystemAge, deleted = FALSE -> preserved formula */
/* expiry >= biddySystemAge, deleted = TRUE -> deleted depreserved formula */
typedef struct {
  Biddy_Edge f;
  Biddy_String name;
  unsigned int expiry;
  Biddy_Boolean deleted;
} BiddyFormula;

typedef struct {
  BiddyFormula *table;
  unsigned int size;
  unsigned int numOrdered; /* formulae with name are ordered */
  Biddy_String deletedName; /* used to report deleted formulae */
} BiddyFormulaTable;

/* CACHE LIST = unidirectional list */
typedef struct BiddyCacheList {
  struct BiddyCacheList *next;
  Biddy_GCFunction gc;
} BiddyCacheList;

/* OP1 Cache = a fixed-size cache table for one-argument operations */
typedef struct {
  Biddy_Edge result; /* biddy_null = not valid record! */
  Biddy_Edge f;
} BiddyOp1Cache;

typedef struct {
  BiddyOp1Cache *table;
  unsigned int size;
  Biddy_Boolean disabled;
  Biddy_Boolean notusedyet;
  unsigned long long int *search;
  unsigned long long int *find;
#ifdef BIDDYEXTENDEDSTATS_YES
  unsigned long long int *insert;
  unsigned long long int *overwrite;
#endif
} BiddyOp1CacheTable;

/* OP2 Cache = a fixed-size cache table for two-arguments operations */
typedef struct {
  Biddy_Edge result; /* biddy_null = not valid record! */
  Biddy_Edge f,g;
} BiddyOp2Cache;

typedef struct {
  BiddyOp2Cache *table;
  unsigned int size;
  Biddy_Boolean disabled;
  Biddy_Boolean notusedyet;
  unsigned long long int *search;
  unsigned long long int *find;
#ifdef BIDDYEXTENDEDSTATS_YES
  unsigned long long int *insert;
  unsigned long long int *overwrite;
#endif
} BiddyOp2CacheTable;

/* OP3 Cache = a fixed-size cache table for three-arguments operations */
typedef struct {
  Biddy_Edge result; /* biddy_null = not valid record! */
  Biddy_Edge f,g,h;
} BiddyOp3Cache;

typedef struct {
  BiddyOp3Cache *table;
  unsigned int size;
  Biddy_Boolean disabled;
  Biddy_Boolean *notusedyet;
  unsigned long long int *search;
  unsigned long long int *find;
#ifdef BIDDYEXTENDEDSTATS_YES
  unsigned long long int *insert;
  unsigned long long int *overwrite;
#endif
} BiddyOp3CacheTable;

/* EA Cache = a fixed-size cache table intended for Biddy_E and Biddy_A */
/* Since Biddy v1.5, EA Cache is BiddyOp3Cache where variable is represented by h */

/* RC Cache = a fixed-size cache table intended for Biddy_Restrict and Biddy_Compose */
/* Since Biddy v1.5, RC Cache is BiddyOp3Cache where variable is represented by h */

/* Keyword Cache = a fixed-size cache table for one-argument operations */
/* Since Biddy v1.7, KeywordCache is used for Biddy_ReplaceByKeyword */
/* TO DO: use it also for other functions, e.g. copyBDD */
typedef struct {
  Biddy_Edge result; /* biddy_null = not valid record! */
  Biddy_Edge f;
  unsigned int keyword;
} BiddyKeywordCache;

typedef struct {
  BiddyKeywordCache *table;
  Biddy_String *keywordList;
  unsigned int *keyList;
  unsigned int keywordNum;
  unsigned int keyNum;
  unsigned int size;
  Biddy_Boolean disabled;
  Biddy_Boolean *notusedyet;
  unsigned long long int *search;
  unsigned long long int *find;
#ifdef BIDDYEXTENDEDSTATS_YES
  unsigned long long int *insert;
  unsigned long long int *overwrite;
#endif
} BiddyKeywordCacheTable;

/* Manager = configuration of a BDD system, since Biddy v1.4 */
/* All fields in BiddyManager must be pointers or arrays */
/* Anonymous manager (i.e. anonymous namespace) is created by Biddy_Init */
/* BDD systems managed by different managers are completely independent */
/* (different node table, different caches, different system age etc.) */
/* User can create its own manager and use it instead of anonymous manager */
/* All managers are of type void** but internally they have the structure */
/* given here (see function Biddy_Init for details). */
/* NOTE: element names given here are placeholders not used in the program! */
typedef struct {
  void *name; /* this is typecasted to Biddy_String */
  void *type; /* this is typecasted to (short int*) */
  void *terminalEdge; /* this is typecasted to (void *) */
  void *constantZero; /* this is typecasted to Biddy_Edge */
  void *constantOne; /* this is typecasted to Biddy_Edge */
  void *nodeTable; /* this is typecasted to (BiddyNodeTable*) */
  void *variableTable; /* this is typecasted to (BiddyVariableTable*) */
  void *formulaTable; /* this is typecasted to (BiddyFormulaTable*) */
  void *OPcache; /* this is typecasted to (BiddyOp3CacheTable*) */
  void *EAcache; /* this is typecasted to (BiddyOp3CacheTable*) */
  void *RCcache; /* this is typecasted to (BiddyOp3CacheTable*) */
  void *REPLACEcache; /* this is typecasted to (BiddyKeywordCacheTable*) */
  void *cacheList; /* this is typecasted to (BiddyCacheList*) */
  void *freeNodes; /* this is typecasted to (BiddyNode*) */
  void *ordering; /* this is typecasted to (BiddyOrderingTable*) */
  void *age; /* this is typecasted to (unsigned int*) */
  void *selector; /* this is typecasted to (unsigned short int*) */
} BiddyManager;

/* LocalInfo = a table for additional info about nodes, since Biddy v1.4 */
/* Created only if needed and deleted before normal operations */
/* It reuses pointer which is normally used for garbage collection! */
/* Only one element of the union can be used! */
typedef struct {
  BiddyNode *back; /* reference to node */
  BiddyNode *org; /* original pointer, used when deleting local cache */
  union {
    unsigned int enumerator; /* used in enumerateNodes */
    BiddyNode *copy; /* used in BiddyCopy */
    unsigned int npSelected; /* used in nodePlainNumber and nodeVarNumber */
    /* NOTE: positiveSelected iff (data.npSelected & 1 != 0) */
    /* NOTE: negativeSelected iff (data.npSelected & 2 != 0) */
    unsigned long long int path1Count; /* used in pathCount */
    double path1Probability; /* used in pathProbabilitySum */
    mpz_t mintermCount; /* used in mintermCount, represented using GMP library */
  } data;
  union {
    Biddy_Boolean leftmost; /* used in pathCount and pathProbabilitySum for ZBDDs, only */
    unsigned long long int path0Count; /* used in pathCount for OBDDs, only */
    double path0Probability; /* used in pathProbabilitySum for OBDDs, only */
  } datax;
} BiddyLocalInfo;

/* BiddyVariableOrder is used in Biddy_ConstructBDD, since Biddy v1.8 */
typedef struct {
  Biddy_String name;
  int order;
} BiddyVariableOrder;

/* BiddyNodeList is used in Biddy_ConstructBDD, since Biddy v1.8 */
typedef struct {
  Biddy_String name;
  int id;
  int type;
  int l,r;
  Biddy_Variable ltag,rtag;
  Biddy_Edge f;
  Biddy_Boolean created;
} BiddyNodeList;

/* BiddyXY is used in Biddy_WriteBDDView, since Biddy v1.8 */
/* this is used to pass node coordinates */
typedef struct {
  int id;
  Biddy_String label;
  int x;
  int y;
  Biddy_Boolean isConstant;
} BiddyXY;

/* BiddyBTreeNode and BiddyBTreeContainer are used in Biddy_Eval2, since Biddy v1.8 */
typedef struct {
  int parent;
  int left;
  int right;
  int index;
  unsigned char op;
  Biddy_String name;
} BiddyBTreeNode;

typedef struct {
  BiddyBTreeNode *tnode;
  int availableNode;
} BiddyBTreeContainer;

/* BiddyVarList is used in in Biddy_PrintfSOP and Biddy_WriteSOP, since Biddy v1.8 */
typedef struct BiddyVarList {
  struct BiddyVarList *next;
  Biddy_Variable v;
  Biddy_Boolean negative;
} BiddyVarList;

/* BiddyVerilogLine, BiddyVerilogNode, BiddyVerilogWire, BiddyVerilogModule, and */
/* BiddyVerilogCircuit are used in Biddy_ReadVerilogFile, since Biddy v1.8 */

typedef struct { /* table of lines is used to store acceptable lines */
  Biddy_String keyword;
  Biddy_String line;
} BiddyVerilogLine;

typedef struct { /* table of nodes is used to store names of all symbols */
  Biddy_String type; /* input, output, wire, gate, extern */
  Biddy_String name; /* node name */
} BiddyVerilogNode;

typedef struct { /* wires are primary inputs and gates */
  int id; /* input/gate ID number */
  Biddy_String type; /* type of a signal driving this wire, "I" for primary input, "W" for internal wire */
  unsigned int inputcount; /* number of input/gate inputs */
  int inputs[INOUTNUM]; /* array of inputs */
  unsigned int outputcount; /* number of input/gate outputs - currently not used */
  int outputs[INOUTNUM]; /* array of outputs - currently not used */
  int fanout; /* how many times this wire is used as an input to some gate or another wire */
  int ttl; /* tmp nuber used to guide GC during the creation of BDD */
  Biddy_Edge bdd; /* BDD associated with the wire/gate */
} BiddyVerilogWire;

typedef struct {
  char *name; /* name of the module */
  Biddy_Boolean outputFirst; /* TRUE iff "nand NAND2_1 (N1, N2, N3);" defines N1 = NAND(N2,N3) */
  unsigned int inputcount, outputcount; /* count of primary inputs and primary outputs. */
  unsigned int wirecount, regcount, gatecount; /* count of wires ,regs, gates */
  Biddy_String inputs[INOUTNUM], outputs[INOUTNUM]; /* name list of primary inputs and primary outputs in the netlist */
  Biddy_String wires[WIRENUM]; /* name list of wires in the netlist */
  Biddy_String regs[REGNUM]; /* name list of regs in the netlist */
  Biddy_String gates[GATENUM]; /* name list of gates in the netlist */
} BiddyVerilogModule;

typedef struct {
  Biddy_String name; /* name of the circuit */
  unsigned int inputcount, outputcount; /* number of primary inputs and primary outputs */
  unsigned int nodecount, wirecount; /* number of nodes and wires */
  Biddy_String inputs[INOUTNUM], outputs[INOUTNUM]; /* name list of primary inputs and primary outputs in the netlist */
  BiddyVerilogWire **wires; /* array of all wires */
  BiddyVerilogNode **nodes; /* array of nodes */
} BiddyVerilogCircuit;

/*----------------------------------------------------------------------------*/
/* Variable declarations                                                      */
/*----------------------------------------------------------------------------*/

extern Biddy_Manager biddyAnonymousManager; /* anonymous manager */

extern BiddyLocalInfo *biddyLocalInfo; /* reference to Local Info */

/*----------------------------------------------------------------------------*/
/* Prototypes for internal functions defined in biddyMain.c                   */
/*----------------------------------------------------------------------------*/

extern void BiddyInitMNG(Biddy_Manager *mng, int bddtype);
extern void BiddyExitMNG(Biddy_Manager *mng);
extern Biddy_String BiddyAbout();
#define BiddyManagedGetEmptySet(MNG) biddyZero
#define BiddyManagedGetUniversalSet(MNG) biddyOne
extern Biddy_Boolean BiddyManagedIsOK(Biddy_Manager MNG, Biddy_Edge f); /* TO DO: replace with macro */
extern int BiddyManagedGetManagerType(Biddy_Manager MNG);
extern Biddy_String BiddyManagedGetManagerName(Biddy_Manager MNG);
extern void BiddyManagedSetManagerParameters(Biddy_Manager MNG, float gcr, float gcrF, float gcrX, float rr, float rrF, float rrX, float st, float cst);
extern Biddy_Edge BiddyManagedGetBaseSet(Biddy_Manager MNG);
extern Biddy_Edge BiddyManagedTransferMark(Biddy_Manager MNG, Biddy_Edge f, Biddy_Boolean mark,Biddy_Boolean leftright);
extern Biddy_Boolean BiddyManagedIsEqv(Biddy_Manager MNG1, Biddy_Edge f1, Biddy_Manager MNG2, Biddy_Edge f2);
extern void BiddyManagedSelectNode(Biddy_Manager MNG, Biddy_Edge f);
extern void BiddyManagedDeselectNode(Biddy_Manager MNG, Biddy_Edge f);
extern Biddy_Boolean BiddyManagedIsSelected(Biddy_Manager MNG, Biddy_Edge f);
extern void BiddyManagedSelectFunction(Biddy_Manager MNG, Biddy_Edge f);
extern void BiddyManagedDeselectAll(Biddy_Manager MNG);
extern Biddy_Variable BiddyManagedGetVariable(Biddy_Manager MNG, Biddy_String x);
extern Biddy_Variable BiddyManagedGetLowestVariable(Biddy_Manager MNG);
extern Biddy_Variable BiddyManagedGetIthVariable(Biddy_Manager MNG, Biddy_Variable i);
extern Biddy_Variable BiddyManagedGetPrevVariable(Biddy_Manager MNG, Biddy_Variable v);
extern Biddy_Variable BiddyManagedGetNextVariable(Biddy_Manager MNG, Biddy_Variable v);
extern Biddy_Edge BiddyManagedGetVariableEdge(Biddy_Manager MNG, Biddy_Variable v);
extern Biddy_Edge BiddyManagedGetElementEdge(Biddy_Manager MNG, Biddy_Variable v);
extern Biddy_String BiddyManagedGetVariableName(Biddy_Manager MNG, Biddy_Variable v);
extern Biddy_Edge BiddyManagedGetTopVariableEdge(Biddy_Manager MNG, Biddy_Edge f);
extern Biddy_String BiddyManagedGetTopVariableName(Biddy_Manager MNG, Biddy_Edge f);
extern char BiddyManagedGetTopVariableChar(Biddy_Manager MNG, Biddy_Edge f);
extern void BiddyManagedResetVariablesValue(Biddy_Manager MNG);
extern void BiddyManagedSetVariableValue(Biddy_Manager MNG, Biddy_Variable v, Biddy_Edge f);
extern Biddy_Edge BiddyManagedGetVariableValue(Biddy_Manager MNG, Biddy_Variable v);
extern void BiddyManagedClearVariablesData(Biddy_Manager MNG);
extern void BiddyManagedSetVariableData(Biddy_Manager MNG, Biddy_Variable v, void *x);
extern void *BiddyManagedGetVariableData(Biddy_Manager MNG, Biddy_Variable v);
extern Biddy_Boolean BiddyManagedEval(Biddy_Manager MNG, Biddy_Edge f);
extern double BiddyManagedEvalProbability(Biddy_Manager MNG, Biddy_Edge f);
extern Biddy_Variable BiddyManagedFoaVariable(Biddy_Manager MNG, Biddy_String x, Biddy_Boolean varelem, Biddy_Boolean complete);
extern void BiddyManagedChangeVariableName(Biddy_Manager MNG, Biddy_Variable v, Biddy_String x);
extern Biddy_Variable BiddyManagedAddVariableByName(Biddy_Manager MNG, Biddy_String x, Biddy_Boolean complete);
#define BiddyManagedAddVariable(MNG) BiddyManagedAddVariableByName(MNG,NULL,TRUE)
#define BiddyManagedAddVariableEdge(MNG) BiddyManagedGetVariableEdge(MNG,BiddyManagedAddVariableByName(MNG,NULL,TRUE))
extern Biddy_Variable BiddyManagedAddElementByName(Biddy_Manager MNG, Biddy_String x, Biddy_Boolean complete);
#define BiddyManagedAddElement(MNG) BiddyManagedAddElementByName(MNG,NULL,TRUE)
extern Biddy_Edge BiddyManagedAddVariableBelow(Biddy_Manager MNG, Biddy_Variable v);
extern Biddy_Edge BiddyManagedAddVariableAbove(Biddy_Manager MNG, Biddy_Variable v);
extern Biddy_Edge BiddyManagedIncTag(Biddy_Manager MNG, Biddy_Edge f);
extern Biddy_Edge BiddyManagedTaggedFoaNode(Biddy_Manager MNG, Biddy_Variable v, Biddy_Edge pf, Biddy_Edge pt, Biddy_Variable ptag, Biddy_Boolean garbageAllowed);
#define BiddyManagedFoaNode(MNG,v,pf,pt,garbageAllowed) BiddyManagedTaggedFoaNode(MNG,v,pf,pt,v,garbageAllowed)
extern void BiddyManagedGC(Biddy_Manager MNG, Biddy_Variable targetLT, Biddy_Variable targetGEQ, Biddy_Boolean purge, Biddy_Boolean total);
#define BiddyManagedAutoGC(MNG) BiddyManagedGC(MNG,0,0,FALSE,FALSE)
#define BiddyManagedForceGC(MNG) BiddyManagedGC(MNG,0,0,FALSE,TRUE)
extern void BiddyManagedClean(Biddy_Manager MNG);
extern void BiddyManagedPurge(Biddy_Manager MNG);
extern void BiddyManagedPurgeAndReorder(Biddy_Manager MNG, Biddy_Edge f, Biddy_Boolean converge);
extern void BiddyManagedAddCache(Biddy_Manager MNG, Biddy_GCFunction gc);
extern unsigned int BiddyManagedAddFormula(Biddy_Manager MNG, Biddy_String x, Biddy_Edge f, int c);
#define BiddyManagedAddTmpFormula(MNG,x,f) BiddyManagedAddFormula(MNG,x,f,-1)
#define BiddyManagedAddPersistentFormula(MNG,x,f) BiddyManagedAddFormula(MNG,x,f,0)
#define BiddyManagedKeepFormula(MNG,f) BiddyManagedAddFormula(MNG,NULL,f,1)
#define BiddyManagedKeepFormulaProlonged(MNG,f,c) BiddyManagedAddFormula(MNG,NULL,f,c)
#define BiddyManagedKeepFormulaUntilPurge(MNG,f) BiddyManagedAddFormula(MNG,NULL,f,0)
extern Biddy_Boolean BiddyManagedFindFormula(Biddy_Manager MNG, Biddy_String x, unsigned int *idx, Biddy_Edge *f);
extern Biddy_Boolean BiddyManagedDeleteFormula(Biddy_Manager MNG, Biddy_String x);
extern Biddy_Boolean BiddyManagedDeleteIthFormula(Biddy_Manager MNG, unsigned int i);
extern Biddy_Edge BiddyManagedGetIthFormula(Biddy_Manager MNG, unsigned int i);
extern Biddy_String BiddyManagedGetIthFormulaName(Biddy_Manager MNG, unsigned int i);
extern Biddy_String BiddyManagedGetOrdering(Biddy_Manager MNG);
extern void BiddyManagedSetOrdering(Biddy_Manager MNG, Biddy_String ordering);
extern void BiddyManagedSetAlphabeticOrdering(Biddy_Manager MNG);
extern Biddy_Variable BiddyManagedSwapWithHigher(Biddy_Manager MNG, Biddy_Variable v);
extern Biddy_Variable BiddyManagedSwapWithLower(Biddy_Manager MNG, Biddy_Variable v);
extern Biddy_Boolean BiddyManagedSifting(Biddy_Manager MNG, Biddy_Edge f, Biddy_Boolean converge);
extern void BiddyManagedMinimizeBDD(Biddy_Manager MNG, Biddy_String name);
extern void BiddyManagedMaximizeBDD(Biddy_Manager MNG, Biddy_String name);
extern Biddy_Edge BiddyManagedCopy(Biddy_Manager MNG1, Biddy_Manager MNG2, Biddy_Edge f, Biddy_Boolean complete);
extern void BiddyManagedCopyFormula(Biddy_Manager MNG1, Biddy_Manager MNG2, Biddy_String x);
extern Biddy_Edge BiddyManagedConstructBDD(Biddy_Manager MNG, int numV, Biddy_String varlist, int numN, Biddy_String nodelist);

extern void BiddyIncSystemAge(Biddy_Manager MNG);
extern void BiddyDecSystemAge(Biddy_Manager MNG);
void BiddyCompactSystemAge(Biddy_Manager MNG);
extern void BiddyProlongRecursively(Biddy_Manager MNG, Biddy_Edge f, unsigned int c, Biddy_Variable target);

extern Biddy_Variable BiddyCreateLocalInfo(Biddy_Manager MNG, Biddy_Edge f);
extern void BiddyDeleteLocalInfo(Biddy_Manager MNG, Biddy_Edge f);
extern unsigned int BiddyGetSeqByNode(Biddy_Edge root, Biddy_Edge f);
extern Biddy_Edge BiddyGetNodeBySeq(Biddy_Edge root, unsigned int n);
extern void BiddySetEnumerator(Biddy_Edge f, unsigned int n);
extern unsigned int BiddyGetEnumerator(Biddy_Edge f);
extern void BiddySetCopy(Biddy_Edge f, BiddyNode *n);
extern BiddyNode * BiddyGetCopy(Biddy_Edge f);
extern void BiddySetPath1Count(Biddy_Edge f, unsigned long long int value);
extern unsigned long long int BiddyGetPath1Count(Biddy_Edge f);
extern void BiddySetPath0Count(Biddy_Edge f, unsigned long long int value);
extern unsigned long long int BiddyGetPath0Count(Biddy_Edge f);
extern void BiddySetPath1ProbabilitySum(Biddy_Edge f, double value);
extern double BiddyGetPath1ProbabilitySum(Biddy_Edge f);
extern void BiddySetPath0ProbabilitySum(Biddy_Edge f, double value);
extern double BiddyGetPath0ProbabilitySum(Biddy_Edge f);
extern void BiddySetLeftmost(Biddy_Edge f, Biddy_Boolean value);
extern Biddy_Boolean BiddyGetLeftmost(Biddy_Edge f);
extern void BiddySetMintermCount(Biddy_Edge f, mpz_t value);
extern void BiddyGetMintermCount(Biddy_Edge f, mpz_t result);
extern void BiddySelectNP(Biddy_Edge f);
extern Biddy_Boolean BiddyIsSelectedNP(Biddy_Edge f);

extern Biddy_Boolean BiddyGlobalSifting(Biddy_Manager MNG, Biddy_Boolean converge);
extern Biddy_Boolean BiddySiftingOnFunction(Biddy_Manager MNG, Biddy_Edge f, Biddy_Boolean converge);
extern Biddy_Boolean BiddySiftingOnFunctionDirect(Biddy_Manager MNG, Biddy_Edge f, Biddy_Boolean converge);
extern void BiddySjtInit(Biddy_Manager MNG);
extern void BiddySjtExit(Biddy_Manager MNG);
extern Biddy_Boolean BiddySjtStep(Biddy_Manager MNG);
extern void BiddySetOrdering(Biddy_Manager MNG, BiddyOrderingTable ordering);
extern void BiddySetOrderingByData(Biddy_Manager MNG);

extern Biddy_Edge BiddyCopy(Biddy_Manager MNG1, Biddy_Manager MNG2, Biddy_Edge f);
extern Biddy_Edge BiddyCopyOBDD(Biddy_Manager MNG1, Biddy_Manager MNG2, Biddy_Edge f);
extern Biddy_Edge BiddyCopyZBDD(Biddy_Manager MNG1, Biddy_Manager MNG2, Biddy_Edge f);
extern Biddy_Edge BiddyCopyTZBDD(Biddy_Manager MNG1, Biddy_Manager MNG2, Biddy_Edge f);
extern Biddy_Edge BiddyConvertDirect(Biddy_Manager MNG1, Biddy_Manager MNG2, Biddy_Edge f);
Biddy_Edge BiddyConstructBDD(Biddy_Manager MNG, int numN, BiddyNodeList *tableN);

extern void BiddyOPGarbage(Biddy_Manager MNG);
extern void BiddyOPGarbageNewVariable(Biddy_Manager MNG, Biddy_Variable v);
extern void BiddyOPGarbageDeleteAll(Biddy_Manager MNG);
extern void BiddyEAGarbage(Biddy_Manager MNG);
extern void BiddyEAGarbageNewVariable(Biddy_Manager MNG, Biddy_Variable v);
extern void BiddyEAGarbageDeleteAll(Biddy_Manager MNG);
extern void BiddyRCGarbage(Biddy_Manager MNG);
extern void BiddyRCGarbageNewVariable(Biddy_Manager MNG, Biddy_Variable v);
extern void BiddyRCGarbageDeleteAll(Biddy_Manager MNG);
extern void BiddyReplaceGarbage(Biddy_Manager MNG);
extern void BiddyReplaceGarbageNewVariable(Biddy_Manager MNG, Biddy_Variable v);
extern void BiddyReplaceGarbageDeleteAll(Biddy_Manager MNG);

extern void BiddySystemReport(Biddy_Manager MNG);
extern void BiddyFunctionReport(Biddy_Manager MNG, Biddy_Edge f);
extern void BiddyDebugNode(Biddy_Manager MNG, BiddyNode *node);
extern void BiddyDebugEdge(Biddy_Manager MNG, Biddy_Edge edge);

/*----------------------------------------------------------------------------*/
/* Prototypes for internal functions defined in biddyOp.c                     */
/*----------------------------------------------------------------------------*/

extern Biddy_Edge BiddyManagedNot(const Biddy_Manager MNG, const Biddy_Edge f);
extern Biddy_Edge BiddyManagedITE(const Biddy_Manager MNG, Biddy_Edge f, Biddy_Edge g, Biddy_Edge h);
extern Biddy_Edge BiddyManagedAnd(const Biddy_Manager MNG, const Biddy_Edge f, const Biddy_Edge g);
#define BiddyManagedIntersect(MNG,f,g) BiddyManagedAnd(MNG,f,g)
extern Biddy_Edge BiddyManagedOr(const Biddy_Manager MNG, const Biddy_Edge f, const Biddy_Edge g);
#define BiddyManagedUnion(MNG,f,g) BiddyManagedOr(MNG,f,g)
extern Biddy_Edge BiddyManagedNand(const Biddy_Manager MNG, const Biddy_Edge f, const Biddy_Edge g);
extern Biddy_Edge BiddyManagedNor(const Biddy_Manager MNG, const Biddy_Edge f, const Biddy_Edge g);
extern Biddy_Edge BiddyManagedXor(const Biddy_Manager MNG, const Biddy_Edge f, const Biddy_Edge g);
extern Biddy_Edge BiddyManagedXnor(const Biddy_Manager MNG, const Biddy_Edge f, const Biddy_Edge g);
extern Biddy_Edge BiddyManagedLeq(const Biddy_Manager MNG, const Biddy_Edge f, const Biddy_Edge g);
extern Biddy_Edge BiddyManagedGt(const Biddy_Manager MNG, const Biddy_Edge f, const Biddy_Edge g);
#define BiddyManagedDiff(MNG,f,g) BiddyManagedGt(MNG,f,g)

extern Biddy_Edge BiddyManagedRestrict(Biddy_Manager MNG, Biddy_Edge f, Biddy_Variable v, Biddy_Boolean value);
extern Biddy_Edge BiddyManagedCompose(Biddy_Manager MNG, Biddy_Edge f, Biddy_Edge g, Biddy_Variable v);
extern Biddy_Edge BiddyManagedE(Biddy_Manager MNG, Biddy_Edge f, Biddy_Variable v);
extern Biddy_Edge BiddyManagedA(Biddy_Manager MNG, Biddy_Edge f, Biddy_Variable v);
extern Biddy_Edge BiddyManagedExistAbstract(Biddy_Manager MNG, Biddy_Edge f, Biddy_Edge cube);
extern Biddy_Edge BiddyManagedUnivAbstract(Biddy_Manager MNG, Biddy_Edge f, Biddy_Edge cube);
extern Biddy_Edge BiddyManagedAndAbstract(Biddy_Manager MNG, Biddy_Edge f, Biddy_Edge g, Biddy_Edge cube);
extern Biddy_Edge BiddyManagedConstrain(Biddy_Manager MNG, Biddy_Edge f, Biddy_Edge c);
extern Biddy_Edge BiddyManagedSimplify(Biddy_Manager MNG, Biddy_Edge f, Biddy_Edge c);
extern Biddy_Edge BiddyManagedSupport(Biddy_Manager MNG, Biddy_Edge f);
extern Biddy_Edge BiddyManagedReplaceByKeyword(Biddy_Manager MNG, Biddy_Edge f, Biddy_String keyword);
#define BiddyManagedReplace(MNG,f) BiddyManagedReplaceByKeyword(MNG,f,NULL)
extern Biddy_Edge BiddyManagedChange(Biddy_Manager MNG, Biddy_Edge f, Biddy_Variable v);
extern Biddy_Edge BiddyManagedVarSubset(Biddy_Manager MNG, Biddy_Edge f, Biddy_Variable v, Biddy_Boolean value);
#define BiddyManagedSubset0(MNG,f,v) BiddyManagedVarSubset(MNG,f,v,FALSE)
#define BiddyManagedSubset1(MNG,f,v) BiddyManagedVarSubset(MNG,f,v,TRUE)
#define BiddyManagedQuotient(MNG,f,v) BiddyManagedChange(MNG,Biddy_Managed_VarSubset(MNG,f,v,TRUE),v)
#define BiddyManagedRemainder(MNG,f,v) BiddyManagedVarSubset(MNG,f,v,FALSE)
extern Biddy_Edge BiddyManagedElementAbstract(Biddy_Manager MNG, Biddy_Edge f, Biddy_Variable v);
extern Biddy_Edge BiddyManagedProduct(Biddy_Manager MNG, Biddy_Edge f, Biddy_Edge g);
extern Biddy_Edge BiddyManagedSelectiveProduct(Biddy_Manager MNG, Biddy_Edge f, Biddy_Edge g, Biddy_Edge cube);
extern Biddy_Edge BiddyManagedSupset(Biddy_Manager MNG, Biddy_Edge f, Biddy_Edge g);
extern Biddy_Edge BiddyManagedSubset(Biddy_Manager MNG, Biddy_Edge f, Biddy_Edge g);
extern Biddy_Edge BiddyManagedPermitsym(Biddy_Manager MNG, Biddy_Edge f, Biddy_Variable lowest, unsigned int n);
extern Biddy_Edge BiddyManagedStretch(Biddy_Manager MNG, Biddy_Edge support, Biddy_Edge f);

extern Biddy_Edge BiddyManagedCreateMinterm(Biddy_Manager MNG, Biddy_Edge support, long long unsigned int x);
extern Biddy_Edge BiddyManagedCreateFunction(Biddy_Manager MNG, Biddy_Edge support, long long unsigned int x);
extern Biddy_Edge BiddyManagedRandomFunction(Biddy_Manager MNG, Biddy_Edge support, double ratio);
extern Biddy_Edge BiddyManagedRandomSet(Biddy_Manager MNG, Biddy_Edge unit, double ratio);
extern Biddy_Edge BiddyManagedExtractMinterm(Biddy_Manager MNG, Biddy_Edge support, Biddy_Edge f);

/*----------------------------------------------------------------------------*/
/* Prototypes for internal functions defined in biddyStat.c                   */
/*----------------------------------------------------------------------------*/

extern unsigned int BiddyManagedCountNodes(Biddy_Manager MNG, Biddy_Edge f);
extern unsigned int BiddyMaxLevel(Biddy_Edge f);
extern float BiddyAvgLevel(Biddy_Edge f);
extern unsigned int BiddyManagedSystemStat(Biddy_Manager MNG, unsigned int stat);
#define BiddyManagedVariableTableNum(MNG) BiddyManagedSystemStat(MNG,BIDDYSTATVARIABLETABLENUM)
#define BiddyManagedFormulaTableNum(MNG) BiddyManagedSystemStat(MNG,BIDDYSTATFORMULATABLENUM)
#define BiddyManagedNodeTableSize(MNG) BiddyManagedSystemStat(MNG,BIDDYSTATNODETABLESIZE)
#define BiddyManagedNodeTableBlockNumber(MNG) BiddyManagedSystemStat(MNG,BIDDYSTATNODETABLEBLOCKNUMBER)
#define BiddyManagedNodeTableGenerated(MNG) BiddyManagedSystemStat(MNG,BIDDYSTATNODETABLEGENERATED)
#define BiddyManagedNodeTableMax(MNG) BiddyManagedSystemStat(MNG,BIDDYSTATNODETABLEMAX)
#define BiddyManagedNodeTableNum(MNG) BiddyManagedSystemStat(MNG,BIDDYSTATNODETABLENUM)
#define BiddyManagedNodeTableResizeNumber(MNG) BiddyManagedSystemStat(MNG,BIDDYSTATNODETABLERESIZENUMBER)
#define BiddyManagedNodeTableGCNumber(MNG) BiddyManagedSystemStat(MNG,BIDDYSTATNODETABLEGCNUMBER)
#define BiddyManagedNodeTableGCTime(MNG) BiddyManagedSystemStat(MNG,BIDDYSTATNODETABLEGCTIME)
#define BiddyManagedNodeTableSwapNumber(MNG) BiddyManagedSystemStat(MNG,BIDDYSTATNODETABLESWAPNUMBER)
#define BiddyManagedNodeTableSiftingNumber(MNG) BiddyManagedSystemStat(MNG,BIDDYSTATNODETABLESIFTINGNUMBER)
#define BiddyManagedNodeTableDRTime(MNG) BiddyManagedSystemStat(MNG,BIDDYSTATNODETABLEDRTIME)
#define BiddyManagedNodeTableITENumber(MNG) BiddyManagedSystemStat(MNG,BIDDYSTATNODETABLEITENUMBER)
#define BiddyManagedNodeTableANDORNumber(MNG) BiddyManagedSystemStat(MNG,BIDDYSTATNODETABLEANDORNUMBER)
#define BiddyManagedNodeTableXORNumber(MNG) BiddyManagedSystemStat(MNG,BIDDYSTATNODETABLEXORNUMBER)
extern unsigned long long int BiddyManagedSystemLongStat(Biddy_Manager MNG, unsigned int longstat);
#define BiddyManagedNodeTableFoaNumber(MNG) BiddyManagedSystemLongStat(MNG,BIDDYLONGSTATNODETABLEFOANUMBER)
#define BiddyManagedNodeTableFindNumber(MNG) BiddyManagedSystemLongStat(MNG,BIDDYLONGSTATNODETABLEFINDNUMBER)
#define BiddyManagedNodeTableCompareNumber(MNG) BiddyManagedSystemLongStat(MNG,BIDDYLONGSTATNODETABLECOMPARENUMBER)
#define BiddyManagedNodeTableAddNumber(MNG) BiddyManagedSystemLongStat(MNG,BIDDYLONGSTATNODETABLEADDNUMBER)
#define BiddyManagedNodeTableITERecursiveNumber(MNG) BiddyManagedSystemLongStat(MNG,BIDDYLONGSTATNODETABLEITERECURSIVENUMBER)
#define BiddyManagedNodeTableANDORRecursiveNumber(MNG) BiddyManagedSystemLongStat(MNG,BIDDYLONGSTATNODETABLEANDORRECURSIVENUMBER)
#define BiddyManagedNodeTableXORRecursiveNumber(MNG) BiddyManagedSystemLongStat(MNG,BIDDYLONGSTATNODETABLEXORRECURSIVENUMBER)
#define BiddyManagedOPCacheSearch(MNG) BiddyManagedSystemLongStat(MNG,BIDDYLONGSTATOPCACHESEARCH)
#define BiddyManagedOPCacheFind(MNG) BiddyManagedSystemLongStat(MNG,BIDDYLONGSTATOPCACHEFIND)
#define BiddyManagedOPCacheInsert(MNG) BiddyManagedSystemLongStat(MNG,BIDDYLONGSTATOPCACHEINSERT)
#define BiddyManagedOPCacheOverwrite(MNG) BiddyManagedSystemLongStat(MNG,BIDDYLONGSTATOPCACHEOVERWRITE)
extern unsigned int BiddyManagedNodeTableNumVar(Biddy_Manager MNG, Biddy_Variable v);
extern unsigned long long int BiddyManagedNodeTableGCObsoleteNumber(Biddy_Manager MNG);
extern unsigned int BiddyManagedListUsed(Biddy_Manager MNG);
extern unsigned int BiddyManagedListMaxLength(Biddy_Manager MNG);
extern float BiddyManagedListAvgLength(Biddy_Manager MNG);
extern unsigned int BiddyManagedCountNodesPlain(Biddy_Manager MNG, Biddy_Edge f);
extern unsigned int BiddyManagedDependentVariableNumber(Biddy_Manager MNG, Biddy_Edge f, Biddy_Boolean select);
extern unsigned int BiddyManagedCountComplementedEdges(Biddy_Manager MNG, Biddy_Edge f);
extern unsigned long long int BiddyManagedCountPaths(Biddy_Manager MNG, Biddy_Edge f);
extern double BiddyManagedCountMinterms(Biddy_Manager MNG, Biddy_Edge f, int nvars);
#define BiddyManagedCountCombinations(MNG,f,nvars) BiddyManagedCountMinterms(MNG,f,nvars)
extern double BiddyManagedDensityOfFunction(Biddy_Manager MNG, Biddy_Edge f, unsigned int nvars);
extern double BiddyManagedDensityOfBDD(Biddy_Manager MNG, Biddy_Edge f, unsigned int nvars);
extern unsigned int BiddyManagedMinNodes(Biddy_Manager MNG, Biddy_Edge f);
extern unsigned int BiddyManagedMaxNodes(Biddy_Manager MNG, Biddy_Edge f);
extern unsigned long long int BiddyManagedReadMemoryInUse(Biddy_Manager MNG);
extern void BiddyManagedPrintInfo(Biddy_Manager MNG, FILE *f);

extern void BiddyNodeNumber(Biddy_Manager MNG, Biddy_Edge f, unsigned int *n);
extern void BiddyComplementedEdgeNumber(Biddy_Manager MNG, Biddy_Edge f, unsigned int *n);
extern void BiddyNodeVarNumber(Biddy_Manager MNG, Biddy_Edge f, unsigned int *n);

/*----------------------------------------------------------------------------*/
/* Prototypes for internal functions defined in biddyInOut.c                  */
/*----------------------------------------------------------------------------*/

Biddy_String BiddyManagedEval0(Biddy_Manager MNG, Biddy_String s);
Biddy_Edge BiddyManagedEval1x(Biddy_Manager MNG, Biddy_String s, Biddy_LookupFunction lf);
#define BiddyManagedEval1(MNG,s) BiddyManagedEval1x(MNG,s,NULL)
Biddy_Edge BiddyManagedEval2(Biddy_Manager MNG, Biddy_String boolFunc);
Biddy_String BiddyManagedReadBddview(Biddy_Manager MNG, const char filename[],Biddy_String name);
void BiddyManagedReadVerilogFile(Biddy_Manager MNG, const char filename[],Biddy_String prefix);
void BiddyManagedPrintBDD(Biddy_Manager MNG, Biddy_String *var, const char filename[],Biddy_Edge f, Biddy_String label);
#define BiddyManagedPrintfBDD(MNG,f) BiddyManagedPrintBDD(MNG,NULL,"stdout",f,NULL)
#define BiddyManagedSprintfBDD(MNG,var,f) BiddyManagedPrintBDD(MNG,var,"",f,NULL)
#define BiddyManagedWriteBDD(MNG,filename,f,label) BiddyManagedPrintBDD(MNG,NULL,filename,f,label)
void BiddyManagedPrintTable(Biddy_Manager MNG, Biddy_String *var, const char filename[],Biddy_Edge f);
#define BiddyManagedPrintfTable(MNG,f) BiddyManagedPrintTable(MNG,NULL,"stdout",f)
#define BiddyManagedSprintfTable(MNG,var,f) BiddyManagedPrintTable(MNG,var,"",f)
#define BiddyManagedWriteTable(MNG,filename,f) BiddyManagedPrintTable(MNG,NULL,filename,f)
void BiddyManagedPrintSOP(Biddy_Manager MNG, Biddy_String *var, const char filename[],Biddy_Edge f);
#define BiddyManagedPrintfSOP(MNG,f) BiddyManagedPrintSOP(MNG,NULL,"stdout",f)
#define BiddyManagedSprintfSOP(MNG,var,f) BiddyManagedPrintSOP(MNG,var,"",f)
#define BiddyManagedWriteSOP(MNG,filename,f) BiddyManagedPrintSOP(MNG,NULL,filename,f)
void BiddyManagedPrintMinterms(Biddy_Manager MNG, Biddy_String *var, const char filename[],Biddy_Edge f, Biddy_Boolean negative);
#define BiddyManagedPrintfMinterms(MNG,f,negative) BiddyManagedPrintMinterms(MNG,NULL,"stdout",f,negative)
#define BiddyManagedSprintfMinterms(MNG,var,f,negative) BiddyManagedPrintMinterms(MNG,var,"",f,negative)
#define BiddyManagedWriteMinterms(MNG,filename,f,negative) BiddyManagedPrintMinterms(MNG,NULL,filename,f,negative)
unsigned int BiddyManagedWriteDot(Biddy_Manager MNG, const char filename[], Biddy_Edge f,const char label[], int id, Biddy_Boolean cudd);
unsigned int BiddyManagedWriteBddview(Biddy_Manager MNG, const char filename[],Biddy_Edge f, const char label[], void *xytable);

#endif  /* _BIDDYINT */

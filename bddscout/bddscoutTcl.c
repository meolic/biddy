/**CFile*******************************************************************
  PackageName [Bdd Scout]
  Synopsis    [Bdd Scout]

  FileName    [bddscoutTcl.c]
  Revision    [$Revision: 320 $]
  Date        [$Date: 2017-10-01 12:02:23 +0200 (ned, 01 okt 2017) $]
  Authors     [Robert Meolic (robert.meolic@um.si)]
  Description [File bddscoutTcl.c contains definitions of Tcl commands,
               which can be used for manipulating with BDDs from
               Tcl interpreter (e.g. tclsh or wish).]
  SeeAlso     [bddscout.h, bddscout.c, bddscout.tcl]

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

#define BUILD_BDDSCOUTSTUBS
#include "bddscoutTcl.h"

/*-----------------------------------------------------------------------*/
/* Type declarations                                                     */
/*-----------------------------------------------------------------------*/

/*-----------------------------------------------------------------------*/
/* Structure declarations                                                */
/*-----------------------------------------------------------------------*/

/*-----------------------------------------------------------------------*/
/* Variable declarations                                                 */
/*-----------------------------------------------------------------------*/

/* used for STUBS mechanism */
extern BddscoutStubs bddscoutStubs;

/*-----------------------------------------------------------------------*/
/* Static function prototypes                                            */
/*-----------------------------------------------------------------------*/

static unsigned int MyNodeNumber(Biddy_Edge f);

static unsigned int MyNodeMaxLevel(Biddy_Edge f);

static float MyNodeAvgLevel(Biddy_Edge f);

static unsigned int MyPathNumber(Biddy_Edge f);

static float MyMintermNumber(Biddy_Edge f);

static void concat(char **s1, const char *s2);

/*-----------------------------------------------------------------------*/
/* Prototypes of static functions defining TCL commands                  */
/*-----------------------------------------------------------------------*/

/* **************** */
/* general commands */
/* **************** */

static int BddscoutInitPkgCmd(ClientData clientData, Tcl_Interp *interp, int argc, const char **argv);

static int BddscoutExitPkgCmd(ClientData clientData, Tcl_Interp *interp, int argc, const char **argv);

static int BddscoutClearPkgCmd(ClientData clientData, Tcl_Interp *interp, int argc, const char **argv);

static int BddscoutAboutPkgCmd(ClientData clientData, Tcl_Interp *interp, int argc, const char **argv);

static int BddscoutChangeTypeCmd(ClientData clientData, Tcl_Interp *interp, int argc, const char **argv);

static int BddscoutCheckFormulaCmd(ClientData clientData, Tcl_Interp *interp, int argc, const char **argv);

static int BddscoutCopyFormulaCmd(ClientData clientData, Tcl_Interp *interp, int argc, const char **argv);

static int BddscoutConstructCmd(ClientData clientData, Tcl_Interp *interp, int argc, const char **argv);

static int BddscoutWriteBddviewCmd(ClientData clientData, Tcl_Interp *interp, int argc, const char **argv);

/* *************************************************** */
/* commands to prepare lists used in menus and dialogs */
/* *************************************************** */

static int BddscoutListVariablesByPositionCmd(ClientData clientData, Tcl_Interp *interp, int argc, const char **argv);

static int BddscoutListVariablesByNameCmd(ClientData clientData, Tcl_Interp *interp, int argc, const char **argv);

static int BddscoutListVariablesByOrderCmd(ClientData clientData, Tcl_Interp *interp, int argc, const char **argv);

static int BddscoutListVariablesByNumberCmd(ClientData clientData, Tcl_Interp *interp, int argc, const char **argv);

static int BddscoutListFormulaeByNameCmd(ClientData clientData, Tcl_Interp *interp, int argc, const char **argv);

static int BddscoutListFormulaeByNodeNumberCmd(ClientData clientData, Tcl_Interp *interp, int argc, const char **argv);

static int BddscoutListFormulaeByNodeMaxLevelCmd(ClientData clientData, Tcl_Interp *interp, int argc, const char **argv);

static int BddscoutListFormulaeByNodeAvgLevelCmd(ClientData clientData, Tcl_Interp *interp, int argc, const char **argv);

static int BddscoutListFormulaeByPathNumberCmd(ClientData clientData, Tcl_Interp *interp, int argc, const char **argv);

static int BddscoutListFormulaeByMintermNumberCmd(ClientData clientData, Tcl_Interp *interp, int argc, const char **argv);

/* ************************************************ */
/* commands used to import files of various formats */
/* ************************************************ */

static int BddscoutReadBDDCmd(ClientData clientData, Tcl_Interp *interp, int argc, const char **argv);

static int BddscoutReadBFCmd(ClientData clientData, Tcl_Interp *interp, int argc, const char **argv);

/* ********************************************** */
/* commands used to parse strings from input line */
/* ********************************************** */

static int BddscoutParseInputInfixCmd(ClientData clientData, Tcl_Interp *interp, int argc, const char **argv);

/* ****************************************************************** */
/* commands used to perform Biddy functions exported from biddyMain.c */
/* ****************************************************************** */

/* 79 Biddy_SwapWithHigher */
static int BiddySwapWithHigherCmd(ClientData clientData, Tcl_Interp *interp, int argc, const char **argv);

/* 80 Biddy_SwapWithLower */
static int BiddySwapWithLowerCmd(ClientData clientData, Tcl_Interp *interp, int argc, const char **argv);

/* 81 Biddy_Sifting */
static int BiddySiftingCmd(ClientData clientData, Tcl_Interp *interp, int argc, const char **argv);

/* 81 Macro Biddy_Sifting */
static int BiddySiftingOnFunctionCmd(ClientData clientData, Tcl_Interp *interp, int argc, const char **argv);

/* ****************************************************************** */
/* commands used to perform Biddy functions exported from biddyStat.c */
/* ****************************************************************** */

/* 87 Biddy_Managed_NodeNumber */
static int BiddyNodeNumberCmd(ClientData clientData, Tcl_Interp *interp, int argc, const char **argv);

/* 88 Biddy_NodeMaxLevel */
static int BiddyNodeMaxLevelCmd(ClientData clientData, Tcl_Interp *interp, int argc, const char **argv);

/* 89 Biddy_NodeAvgLevel */
static int BiddyNodeAvgLevelCmd(ClientData clientData, Tcl_Interp *interp, int argc, const char **argv);

/* 90 Biddy_Managed_VariableTableNum */
static int BiddyVariableTableNumCmd(ClientData clientData, Tcl_Interp *interp, int argc, const char **argv);

/* 91 Biddy_Managed_NodeTableSize */
static int BiddyNodeTableSizeCmd(ClientData clientData, Tcl_Interp *interp, int argc, const char **argv);

/* 92 Biddy_Managed_NodeTableBlockNumber */
static int BiddyNodeTableBlockNumberCmd(ClientData clientData, Tcl_Interp *interp, int argc, const char **argv);

/* 93 Biddy_Managed_NodeTableGenerated */
static int BiddyNodeTableGeneratedCmd(ClientData clientData, Tcl_Interp *interp, int argc, const char **argv);

/* 94 Biddy_Managed_NodeTableMax */
static int BiddyNodeTableMaxCmd(ClientData clientData, Tcl_Interp *interp, int argc, const char **argv);

/* 95 Biddy_Managed_NodeTableNum */
static int BiddyNodeTableNumCmd(ClientData clientData, Tcl_Interp *interp, int argc, const char **argv);

/* 96 Biddy_Managed_NodeTableNumVar */
static int BiddyNodeTableNumVarCmd(ClientData clientData, Tcl_Interp *interp, int argc, const char **argv);

/* 97 Biddy_Managed_NodeTableResizeNumber */
static int BiddyNodeTableResizeNumberCmd(ClientData clientData, Tcl_Interp *interp, int argc, const char **argv);

/* 98 Biddy_Managed_NodeTableFoaNumber */
static int BiddyNodeTableFoaNumberCmd(ClientData clientData, Tcl_Interp *interp, int argc, const char **argv);

/* 99 Biddy_Managed_NodeTableFindNumber */
static int BiddyNodeTableFindNumberCmd(ClientData clientData, Tcl_Interp *interp, int argc, const char **argv);

/* 100 Biddy_Managed_NodeTableCompareNumber */
static int BiddyNodeTableCompareNumberCmd(ClientData clientData, Tcl_Interp *interp, int argc, const char **argv);

/* 101 Biddy_Managed_NodeTableAddNumber */
static int BiddyNodeTableAddNumberCmd(ClientData clientData, Tcl_Interp *interp, int argc, const char **argv);

/* 102 Biddy_Managed_NodeTableGCNumber */
static int BiddyNodeTableGCNumberCmd(ClientData clientData, Tcl_Interp *interp, int argc, const char **argv);

/* 103 Biddy_Managed_NodeTableGCTime */
static int BiddyNodeTableGCTimeCmd(ClientData clientData, Tcl_Interp *interp, int argc, const char **argv);

/* 104 Biddy_Managed_NodeTableGCObsoleteNumber */
static int BiddyNodeTableGCObsoleteNumberCmd(ClientData clientData, Tcl_Interp *interp, int argc, const char **argv);

/* 105 Biddy_Managed_NodeTableSwapNumber */
static int BiddyNodeTableSwapNumberCmd(ClientData clientData, Tcl_Interp *interp, int argc, const char **argv);

/* 106 Biddy_Managed_NodeTableSiftingNumber */
static int BiddyNodeTableSiftingNumberCmd(ClientData clientData, Tcl_Interp *interp, int argc, const char **argv);

/* 107 Biddy_Managed_NodeTableDRTime */
static int BiddyNodeTableDRTimeCmd(ClientData clientData, Tcl_Interp *interp, int argc, const char **argv);

/* 108 Biddy_Managed_NodeTableITENumber */
static int BiddyNodeTableITENumberCmd(ClientData clientData, Tcl_Interp *interp, int argc, const char **argv);

/* 109 Biddy_Managed_NodeTableITERecursiveNumber */
static int BiddyNodeTableITERecursiveNumberCmd(ClientData clientData, Tcl_Interp *interp, int argc, const char **argv);

/* 110 Biddy_Managed_NodeTableANDORNumber */
static int BiddyNodeTableANDORNumberCmd(ClientData clientData, Tcl_Interp *interp, int argc, const char **argv);

/* 111 Biddy_Managed_NodeTableANDORRecursiveNumber */
static int BiddyNodeTableANDORRecursiveNumberCmd(ClientData clientData, Tcl_Interp *interp, int argc, const char **argv);

/* 112 Biddy_Managed_NodeTableXORNumber */
static int BiddyNodeTableXORNumberCmd(ClientData clientData, Tcl_Interp *interp, int argc, const char **argv);

/* 113 Biddy_Managed_NodeTableXORRecursiveNumber */
static int BiddyNodeTableXORRecursiveNumberCmd(ClientData clientData, Tcl_Interp *interp, int argc, const char **argv);

/* 114 Biddy_Managed_FormulaTableNum */
static int BiddyFormulaTableNumCmd(ClientData clientData, Tcl_Interp *interp, int argc, const char **argv);

/* 115 Biddy_Managed_ListUsed */
static int BiddyListUsedCmd(ClientData clientData, Tcl_Interp *interp, int argc, const char **argv);

/* 116 Biddy_Managed_ListMaxLength */
static int BiddyListMaxLengthCmd(ClientData clientData, Tcl_Interp *interp, int argc, const char **argv);

/* 117 Biddy_Managed_ListAvgLength */
static int BiddyListAvgLengthCmd(ClientData clientData, Tcl_Interp *interp, int argc, const char **argv);

/* 118 Biddy_Managed_OPCacheSearch */
static int BiddyOPCacheSearchCmd(ClientData clientData, Tcl_Interp *interp, int argc, const char **argv);

/* 119 Biddy_Managed_OPCacheFind */
static int BiddyOPCacheFindCmd(ClientData clientData, Tcl_Interp *interp, int argc, const char **argv);

/* 120 Biddy_Managed_OPCacheInsert */
static int BiddyOPCacheInsertCmd(ClientData clientData, Tcl_Interp *interp, int argc, const char **argv);

/* 121 Biddy_Managed_OPCacheOverwrite */
static int BiddyOPCacheOverwriteCmd(ClientData clientData, Tcl_Interp *interp, int argc, const char **argv);

/* 122 Biddy_Managed_NodeNumberPlain */
static int BiddyNodeNumberPlainCmd(ClientData clientData, Tcl_Interp *interp, int argc, const char **argv);

/* 123 Biddy_Managed_DependentVariableNumber */
static int BiddyDependentVariableNumberCmd(ClientData clientData, Tcl_Interp *interp, int argc, const char **argv);

/* 124 Biddy_Managed_NodeVarNumber */
static int BiddyNodeVarNumberCmd(ClientData clientData, Tcl_Interp *interp, int argc, const char **argv);

/* 125 Biddy_Managed_CountComplemented */
static int BiddyCountComplementedCmd(ClientData clientData, Tcl_Interp *interp, int argc, const char **argv);

/* 126 Biddy_Managed_CountPaths */
static int BiddyCountPathsCmd(ClientData clientData, Tcl_Interp *interp, int argc, const char **argv);

/* 127 Biddy_Managed_CountMinterm */
static int BiddyCountMintermCmd(ClientData clientData, Tcl_Interp *interp, int argc, const char **argv);

/* 128 Biddy_Managed_DensityFunction */
static int BiddyDensityFunctionCmd(ClientData clientData, Tcl_Interp *interp, int argc, const char **argv);

/* 129 Biddy_Managed_DensityBDD */
static int BiddyDensityBDDCmd(ClientData clientData, Tcl_Interp *interp, int argc, const char **argv);

/* 130 Biddy_Managed_ReadMemoryInUse */
static int BiddyReadMemoryInUseCmd(ClientData clientData, Tcl_Interp *interp, int argc, const char **argv);

/* 131 Biddy_Managed_PrintInfo */
static int BiddyPrintInfoCmd(ClientData clientData, Tcl_Interp *interp, int argc, const char **argv);

/* ******************************************************************* */
/* commands used to perform Biddy functions exported from biddyInOut.c */
/* ******************************************************************* */

/* 132 Biddy_Managed_Eval0 */
static int BiddyEval0Cmd(ClientData clientData, Tcl_Interp *interp, int argc, const char **argv);

/* 133 Biddy_Managed_Eval1 */
static int BiddyEval1Cmd(ClientData clientData, Tcl_Interp *interp, int argc, const char **argv);

/* 134 Biddy_Managed_Eval2 */
static int BiddyEval2Cmd(ClientData clientData, Tcl_Interp *interp, int argc, const char **argv);

/* 135 Biddy_Managed_ReadVerilogFile */
static int BiddyReadVerilogFileCmd(ClientData clientData, Tcl_Interp *interp, int argc, const char **argv);

/* 136 Biddy_Managed_PrintfBDD */
static int BiddyPrintfBDDCmd(ClientData clientData, Tcl_Interp *interp, int argc, const char **argv);

/* 137 Biddy_Managed_WriteBDD */
static int BiddyWriteBDDCmd(ClientData clientData, Tcl_Interp *interp, int argc, const char **argv);

/* 138 Biddy_Managed_PrintfTable */
static int BiddyPrintfTableCmd(ClientData clientData, Tcl_Interp *interp, int argc, const char **argv);

/* 139 Biddy_Managed_WriteTable */
static int BiddyWriteTableCmd(ClientData clientData, Tcl_Interp *interp, int argc, const char **argv);

/* 140 Biddy_Managed_PrintfSOP */
static int BiddyPrintfSOPCmd(ClientData clientData, Tcl_Interp *interp, int argc, const char **argv);

/* 141 Biddy_Managed_WriteSOP */
static int BiddyWriteSOPCmd(ClientData clientData, Tcl_Interp *interp, int argc, const char **argv);

/* 142 Biddy_Managed_WriteDot */
static int BiddyWriteDotCmd(ClientData clientData, Tcl_Interp *interp, int argc, const char **argv);

/* 143 Biddy_Managed_WriteBddview */
static int BiddyWriteBddviewCmd(ClientData clientData, Tcl_Interp *interp, int argc, const char **argv);

/*-----------------------------------------------------------------------*/
/* External functions                                                    */
/*-----------------------------------------------------------------------*/

/**Function****************************************************************
  Synopsis    [Function Bddscout_Init.]
  Description [This function is used by Tcl/Tk only.]
  SideEffects []
  SeeAlso     []
  ************************************************************************/

#ifdef __cplusplus
extern "C" {
#endif

int
Bddscout_Init(Tcl_Interp *interp)
{

  if (Tcl_InitStubs(interp, "8.1", 0) == NULL) {
    return TCL_ERROR;
  }

/* **************** */
/* general commands */
/* **************** */

  Tcl_CreateCommand(interp, "bddscout_initPkg", BddscoutInitPkgCmd,
                     (ClientData) NULL, (Tcl_CmdDeleteProc *) NULL);

  Tcl_CreateCommand(interp, "bddscout_exitPkg", BddscoutExitPkgCmd,
                     (ClientData) NULL, (Tcl_CmdDeleteProc *) NULL);

  Tcl_CreateCommand(interp, "bddscout_clearPkg", BddscoutClearPkgCmd,
                     (ClientData) NULL, (Tcl_CmdDeleteProc *) NULL);

  Tcl_CreateCommand(interp, "bddscout_aboutPkg", BddscoutAboutPkgCmd,
                     (ClientData) NULL, (Tcl_CmdDeleteProc *) NULL);

  Tcl_CreateCommand(interp, "bddscout_change_type", BddscoutChangeTypeCmd,
                     (ClientData) NULL, (Tcl_CmdDeleteProc *) NULL);

  Tcl_CreateCommand(interp, "bddscout_check_formula", BddscoutCheckFormulaCmd,
                     (ClientData) NULL, (Tcl_CmdDeleteProc *) NULL);

  Tcl_CreateCommand(interp, "bddscout_copy_formula", BddscoutCopyFormulaCmd,
                     (ClientData) NULL, (Tcl_CmdDeleteProc *) NULL);

  Tcl_CreateCommand(interp, "bddscout_construct", BddscoutConstructCmd,
                     (ClientData) NULL, (Tcl_CmdDeleteProc *) NULL);

  Tcl_CreateCommand(interp, "bddscout_write_bddview", BddscoutWriteBddviewCmd,
                     (ClientData) NULL, (Tcl_CmdDeleteProc *) NULL);

/* *************************************************** */
/* commands to prepare lists used in menus and dialogs */
/* *************************************************** */

  Tcl_CreateCommand(interp, "bddscout_list_variables_by_positon", BddscoutListVariablesByPositionCmd,
                     (ClientData) NULL, (Tcl_CmdDeleteProc *) NULL);

  Tcl_CreateCommand(interp, "bddscout_list_variables_by_name", BddscoutListVariablesByNameCmd,
                     (ClientData) NULL, (Tcl_CmdDeleteProc *) NULL);

  Tcl_CreateCommand(interp, "bddscout_list_variables_by_order", BddscoutListVariablesByOrderCmd,
                     (ClientData) NULL, (Tcl_CmdDeleteProc *) NULL);

  Tcl_CreateCommand(interp, "bddscout_list_variables_by_number", BddscoutListVariablesByNumberCmd,
                     (ClientData) NULL, (Tcl_CmdDeleteProc *) NULL);

  Tcl_CreateCommand(interp, "bddscout_list_formulae_by_name", BddscoutListFormulaeByNameCmd,
                     (ClientData) NULL, (Tcl_CmdDeleteProc *) NULL);

  Tcl_CreateCommand(interp, "bddscout_list_formulae_by_node_number", BddscoutListFormulaeByNodeNumberCmd,
                     (ClientData) NULL, (Tcl_CmdDeleteProc *) NULL);

  Tcl_CreateCommand(interp, "bddscout_list_formulae_by_node_max_level", BddscoutListFormulaeByNodeMaxLevelCmd,
                     (ClientData) NULL, (Tcl_CmdDeleteProc *) NULL);

  Tcl_CreateCommand(interp, "bddscout_list_formulae_by_node_avg_level", BddscoutListFormulaeByNodeAvgLevelCmd,
                     (ClientData) NULL, (Tcl_CmdDeleteProc *) NULL);

  Tcl_CreateCommand(interp, "bddscout_list_formulae_by_path_number", BddscoutListFormulaeByPathNumberCmd,
                     (ClientData) NULL, (Tcl_CmdDeleteProc *) NULL);

  Tcl_CreateCommand(interp, "bddscout_list_formulae_by_minterm_number", BddscoutListFormulaeByMintermNumberCmd,
                     (ClientData) NULL, (Tcl_CmdDeleteProc *) NULL);

/* ************************************************ */
/* commands used to import files of various formats */
/* ************************************************ */

  Tcl_CreateCommand(interp, "bddscout_read_bdd", BddscoutReadBDDCmd,
                     (ClientData) NULL, (Tcl_CmdDeleteProc *) NULL);

  Tcl_CreateCommand(interp, "bddscout_read_bf", BddscoutReadBFCmd,
                     (ClientData) NULL, (Tcl_CmdDeleteProc *) NULL);

/* ********************************************** */
/* commands used to parse strings from input line */
/* ********************************************** */

  Tcl_CreateCommand(interp, "bddscout_parse_input_infix", BddscoutParseInputInfixCmd,
                     (ClientData) NULL, (Tcl_CmdDeleteProc *) NULL);

/* ****************************************************************** */
/* commands used to perform Biddy functions exported from biddyMain.c */
/* ****************************************************************** */

  Tcl_CreateCommand(interp, "biddy_swap_with_higher", BiddySwapWithHigherCmd,
                     (ClientData) NULL, (Tcl_CmdDeleteProc *) NULL);

  Tcl_CreateCommand(interp, "biddy_swap_with_lower", BiddySwapWithLowerCmd,
                     (ClientData) NULL, (Tcl_CmdDeleteProc *) NULL);

  Tcl_CreateCommand(interp, "biddy_sifting", BiddySiftingCmd,
                     (ClientData) NULL, (Tcl_CmdDeleteProc *) NULL);

  Tcl_CreateCommand(interp, "biddy_sifting_on_function", BiddySiftingOnFunctionCmd,
                     (ClientData) NULL, (Tcl_CmdDeleteProc *) NULL);

/* ****************************************************************** */
/* commands used to perform Biddy functions exported from biddyStat.c */
/* ****************************************************************** */


  Tcl_CreateCommand(interp, "biddy_node_number", BiddyNodeNumberCmd,
                     (ClientData) NULL, (Tcl_CmdDeleteProc *) NULL);

  Tcl_CreateCommand(interp, "biddy_node_max_level", BiddyNodeMaxLevelCmd,
                     (ClientData) NULL, (Tcl_CmdDeleteProc *) NULL);

  Tcl_CreateCommand(interp, "biddy_node_avg_level", BiddyNodeAvgLevelCmd,
                     (ClientData) NULL, (Tcl_CmdDeleteProc *) NULL);

  Tcl_CreateCommand(interp, "biddy_variabletable_num", BiddyVariableTableNumCmd,
                     (ClientData) NULL, (Tcl_CmdDeleteProc *) NULL);

  Tcl_CreateCommand(interp, "biddy_nodetable_size", BiddyNodeTableSizeCmd,
                     (ClientData) NULL, (Tcl_CmdDeleteProc *) NULL);

  Tcl_CreateCommand(interp, "biddy_nodetable_block_number", BiddyNodeTableBlockNumberCmd,
                     (ClientData) NULL, (Tcl_CmdDeleteProc *) NULL);

  Tcl_CreateCommand(interp, "biddy_nodetable_generated", BiddyNodeTableGeneratedCmd,
                     (ClientData) NULL, (Tcl_CmdDeleteProc *) NULL);

  Tcl_CreateCommand(interp, "biddy_nodetable_max", BiddyNodeTableMaxCmd,
                     (ClientData) NULL, (Tcl_CmdDeleteProc *) NULL);

  Tcl_CreateCommand(interp, "biddy_nodetable_num", BiddyNodeTableNumCmd,
                     (ClientData) NULL, (Tcl_CmdDeleteProc *) NULL);

  Tcl_CreateCommand(interp, "biddy_nodetable_numvar", BiddyNodeTableNumVarCmd,
                     (ClientData) NULL, (Tcl_CmdDeleteProc *) NULL);

  Tcl_CreateCommand(interp, "biddy_nodetable_resize_number", BiddyNodeTableResizeNumberCmd,
                     (ClientData) NULL, (Tcl_CmdDeleteProc *) NULL);

  Tcl_CreateCommand(interp, "biddy_nodetable_foa_number", BiddyNodeTableFoaNumberCmd,
                     (ClientData) NULL, (Tcl_CmdDeleteProc *) NULL);

  Tcl_CreateCommand(interp, "biddy_nodetable_find_number", BiddyNodeTableFindNumberCmd,
                     (ClientData) NULL, (Tcl_CmdDeleteProc *) NULL);

  Tcl_CreateCommand(interp, "biddy_nodetable_compare_number", BiddyNodeTableCompareNumberCmd,
                     (ClientData) NULL, (Tcl_CmdDeleteProc *) NULL);

  Tcl_CreateCommand(interp, "biddy_nodetable_add_number", BiddyNodeTableAddNumberCmd,
                     (ClientData) NULL, (Tcl_CmdDeleteProc *) NULL);

  Tcl_CreateCommand(interp, "biddy_nodetable_gc_number", BiddyNodeTableGCNumberCmd,
                     (ClientData) NULL, (Tcl_CmdDeleteProc *) NULL);

  Tcl_CreateCommand(interp, "biddy_nodetable_gc_time", BiddyNodeTableGCTimeCmd,
                     (ClientData) NULL, (Tcl_CmdDeleteProc *) NULL);

  Tcl_CreateCommand(interp, "biddy_nodetable_gc_obsolete_number", BiddyNodeTableGCObsoleteNumberCmd,
                     (ClientData) NULL, (Tcl_CmdDeleteProc *) NULL);

  Tcl_CreateCommand(interp, "biddy_nodetable_swap_number", BiddyNodeTableSwapNumberCmd,
                     (ClientData) NULL, (Tcl_CmdDeleteProc *) NULL);

  Tcl_CreateCommand(interp, "biddy_nodetable_sifting_number", BiddyNodeTableSiftingNumberCmd,
                     (ClientData) NULL, (Tcl_CmdDeleteProc *) NULL);

  Tcl_CreateCommand(interp, "biddy_nodetable_dr_time", BiddyNodeTableDRTimeCmd,
                     (ClientData) NULL, (Tcl_CmdDeleteProc *) NULL);

  Tcl_CreateCommand(interp, "biddy_nodetable_ite_number", BiddyNodeTableITENumberCmd,
                     (ClientData) NULL, (Tcl_CmdDeleteProc *) NULL);

  Tcl_CreateCommand(interp, "biddy_nodetable_ite_recursive_number", BiddyNodeTableITERecursiveNumberCmd,
                     (ClientData) NULL, (Tcl_CmdDeleteProc *) NULL);

  Tcl_CreateCommand(interp, "biddy_nodetable_andor_number", BiddyNodeTableANDORNumberCmd,
                     (ClientData) NULL, (Tcl_CmdDeleteProc *) NULL);

  Tcl_CreateCommand(interp, "biddy_nodetable_andor_recursive_number", BiddyNodeTableANDORRecursiveNumberCmd,
                     (ClientData) NULL, (Tcl_CmdDeleteProc *) NULL);

  Tcl_CreateCommand(interp, "biddy_nodetable_xor_number", BiddyNodeTableXORNumberCmd,
                     (ClientData) NULL, (Tcl_CmdDeleteProc *) NULL);

  Tcl_CreateCommand(interp, "biddy_nodetable_xor_recursive_number", BiddyNodeTableXORRecursiveNumberCmd,
                     (ClientData) NULL, (Tcl_CmdDeleteProc *) NULL);

  Tcl_CreateCommand(interp, "biddy_formulatable_num", BiddyFormulaTableNumCmd,
                     (ClientData) NULL, (Tcl_CmdDeleteProc *) NULL);

  Tcl_CreateCommand(interp, "biddy_list_used", BiddyListUsedCmd,
                     (ClientData) NULL, (Tcl_CmdDeleteProc *) NULL);

  Tcl_CreateCommand(interp, "biddy_list_max_length", BiddyListMaxLengthCmd,
                     (ClientData) NULL, (Tcl_CmdDeleteProc *) NULL);

  Tcl_CreateCommand(interp, "biddy_list_avg_length", BiddyListAvgLengthCmd,
                     (ClientData) NULL, (Tcl_CmdDeleteProc *) NULL);

  Tcl_CreateCommand(interp, "biddy_opcache_search", BiddyOPCacheSearchCmd,
                     (ClientData) NULL, (Tcl_CmdDeleteProc *) NULL);

  Tcl_CreateCommand(interp, "biddy_opcache_find", BiddyOPCacheFindCmd,
                     (ClientData) NULL, (Tcl_CmdDeleteProc *) NULL);

  Tcl_CreateCommand(interp, "biddy_opcache_insert", BiddyOPCacheInsertCmd,
                     (ClientData) NULL, (Tcl_CmdDeleteProc *) NULL);

  Tcl_CreateCommand(interp, "biddy_opcache_overwrite", BiddyOPCacheOverwriteCmd,
                     (ClientData) NULL, (Tcl_CmdDeleteProc *) NULL);

  Tcl_CreateCommand(interp, "biddy_node_number_plain", BiddyNodeNumberPlainCmd,
                     (ClientData) NULL, (Tcl_CmdDeleteProc *) NULL);

  Tcl_CreateCommand(interp, "biddy_dependent_variable_number", BiddyDependentVariableNumberCmd,
                     (ClientData) NULL, (Tcl_CmdDeleteProc *) NULL);

  Tcl_CreateCommand(interp, "biddy_node_var_number", BiddyNodeVarNumberCmd,
                     (ClientData) NULL, (Tcl_CmdDeleteProc *) NULL);

  Tcl_CreateCommand(interp, "biddy_count_complemented", BiddyCountComplementedCmd,
                     (ClientData) NULL, (Tcl_CmdDeleteProc *) NULL);

  Tcl_CreateCommand(interp, "biddy_count_paths", BiddyCountPathsCmd,
                     (ClientData) NULL, (Tcl_CmdDeleteProc *) NULL);

  Tcl_CreateCommand(interp, "biddy_count_minterm", BiddyCountMintermCmd,
                     (ClientData) NULL, (Tcl_CmdDeleteProc *) NULL);

  Tcl_CreateCommand(interp, "biddy_density_function", BiddyDensityFunctionCmd,
                     (ClientData) NULL, (Tcl_CmdDeleteProc *) NULL);

  Tcl_CreateCommand(interp, "biddy_density_bdd", BiddyDensityBDDCmd,
                     (ClientData) NULL, (Tcl_CmdDeleteProc *) NULL);

  Tcl_CreateCommand(interp, "biddy_read_memory_in_use", BiddyReadMemoryInUseCmd,
                     (ClientData) NULL, (Tcl_CmdDeleteProc *) NULL);

  Tcl_CreateCommand(interp, "biddy_print_info", BiddyPrintInfoCmd,
                     (ClientData) NULL, (Tcl_CmdDeleteProc *) NULL);

/* ******************************************************************* */
/* commands used to perform Biddy functions exported from biddyInOut.c */
/* ******************************************************************* */
    
  Tcl_CreateCommand(interp, "biddy_eval0", BiddyEval0Cmd,
                     (ClientData) NULL, (Tcl_CmdDeleteProc *) NULL);

  Tcl_CreateCommand(interp, "biddy_eval1", BiddyEval1Cmd,
                     (ClientData) NULL, (Tcl_CmdDeleteProc *) NULL);

  Tcl_CreateCommand(interp, "biddy_eval2", BiddyEval2Cmd,
                     (ClientData) NULL, (Tcl_CmdDeleteProc *) NULL);

  Tcl_CreateCommand(interp, "biddy_read_verilog_file", BiddyReadVerilogFileCmd,
                     (ClientData) NULL, (Tcl_CmdDeleteProc *) NULL);

  Tcl_CreateCommand(interp, "biddy_printf_bdd", BiddyPrintfBDDCmd,
                     (ClientData) NULL, (Tcl_CmdDeleteProc *) NULL);

  Tcl_CreateCommand(interp, "biddy_write_bdd", BiddyWriteBDDCmd,
                     (ClientData) NULL, (Tcl_CmdDeleteProc *) NULL);

  Tcl_CreateCommand(interp, "biddy_printf_table", BiddyPrintfTableCmd,
                     (ClientData) NULL, (Tcl_CmdDeleteProc *) NULL);

  Tcl_CreateCommand(interp, "biddy_write_table", BiddyWriteTableCmd,
                     (ClientData) NULL, (Tcl_CmdDeleteProc *) NULL);

  Tcl_CreateCommand(interp, "biddy_printf_sop", BiddyPrintfSOPCmd,
                     (ClientData) NULL, (Tcl_CmdDeleteProc *) NULL);

  Tcl_CreateCommand(interp, "biddy_write_sop", BiddyWriteSOPCmd,
                     (ClientData) NULL, (Tcl_CmdDeleteProc *) NULL);

  Tcl_CreateCommand(interp, "biddy_write_dot", BiddyWriteDotCmd,
                     (ClientData) NULL, (Tcl_CmdDeleteProc *) NULL);

  Tcl_CreateCommand(interp, "biddy_write_bddview", BiddyWriteBddviewCmd,
                     (ClientData) NULL, (Tcl_CmdDeleteProc *) NULL);

  return Tcl_PkgProvideEx(interp, "bddscout-lib", "1.0", &bddscoutStubs);
}

#ifdef __cplusplus
}
#endif

/*-----------------------------------------------------------------------*/
/* Static functions defining TCL commands                                */
/*-----------------------------------------------------------------------*/

/**Function****************************************************************
  Synopsis    []
  Description [general command]
  SideEffects []
  SeeAlso     []
  ************************************************************************/

static int
BddscoutInitPkgCmd(ClientData clientData, Tcl_Interp *interp, int argc,
                   const char **argv)
{
  if (argc != 1) {
    Tcl_SetResult(interp, (char *) "wrong # args", TCL_STATIC);
    return TCL_ERROR;
  }

  BddscoutInit();

  Tcl_SetResult(interp, (char *) "", TCL_STATIC);
  return TCL_OK;
}

/**Function****************************************************************
  Synopsis    []
  Description [general command]
  SideEffects []
  SeeAlso     []
  ************************************************************************/

static int
BddscoutExitPkgCmd(ClientData clientData, Tcl_Interp *interp, int argc,
                   const char **argv)
{
  if (argc != 1) {
    Tcl_SetResult(interp, (char *) "wrong # args", TCL_STATIC);
    return TCL_ERROR;
  }

  BddscoutExit();

  Tcl_SetResult(interp, (char *) "", TCL_STATIC);
  return TCL_OK;
}

/**Function****************************************************************
  Synopsis    []
  Description [general command]
  SideEffects []
  SeeAlso     []
  ************************************************************************/

static int
BddscoutClearPkgCmd(ClientData clientData, Tcl_Interp *interp, int argc,
                   const char **argv)
{
  if (argc != 1) {
    Tcl_SetResult(interp, (char *) "wrong # args", TCL_STATIC);
    return TCL_ERROR;
  }

  Bddscout_ClearActiveManager();

  Tcl_SetResult(interp, (char *) "", TCL_STATIC);
  return TCL_OK;
}

/**Function****************************************************************
  Synopsis    []
  Description [general command]
  SideEffects []
  SeeAlso     []
  ************************************************************************/

static int
BddscoutAboutPkgCmd(ClientData clientData, Tcl_Interp *interp, int argc,
                    const char **argv)
{
  char *bv;

  if (argc != 1) {
    Tcl_SetResult(interp, (char *) "wrong # args", TCL_STATIC);
    return TCL_ERROR;
  }

  printf("This is BDD Scout library");
  bv = Biddy_About();
  printf("Bdd package (Biddy v%s)",bv);

  free(bv);

  printf("\n");
  Tcl_SetResult(interp, (char *) "", TCL_STATIC);
  return TCL_OK;
}

/**Function****************************************************************
  Synopsis    []
  Description [general command]
  SideEffects []
  SeeAlso     []
  ************************************************************************/

static int
BddscoutChangeTypeCmd(ClientData clientData, Tcl_Interp *interp,
                    int argc, const char **argv)
{
  Biddy_String type;

  if (argc != 2) {
    Tcl_SetResult(interp, (char *) "wrong # args", TCL_STATIC);
    return TCL_ERROR;
  }

  type = strdup(argv[1]);

  Bddscout_ChangeActiveManager(type);

  Tcl_SetResult(interp, (char *) "", TCL_STATIC);
  free(type);

  return TCL_OK;
}

/**Function****************************************************************
  Synopsis    []
  Description [command used to perform Biddy functions exported from biddyMain.c]
  SideEffects []
  SeeAlso     []
  ************************************************************************/

static int
BddscoutCheckFormulaCmd(ClientData clientData, Tcl_Interp *interp, int argc,
                  const char **argv)
{
  Biddy_String type,fname,r;
  Biddy_Boolean OK;

  if (argc != 3) {
    Tcl_SetResult(interp, (char *) "wrong # args", TCL_STATIC);
    return TCL_ERROR;
  }

  type = strdup(argv[1]);
  fname = strdup(argv[2]);

  OK = BddscoutCheckFormula(type,fname);

  r = NULL;
  if (OK) {
    r = strdup("1");
  } else {
    r = strdup("0");
  }

  Tcl_SetResult(interp, r, TCL_VOLATILE);

  free(type);
  free(fname);
  free(r);

  return TCL_OK;
}

/**Function****************************************************************
  Synopsis    []
  Description []
  SideEffects []
  SeeAlso     []
  ************************************************************************/

static int
BddscoutCopyFormulaCmd(ClientData clientData, Tcl_Interp *interp, int argc,
                  const char **argv)
{
  Biddy_String fname,type1,type2;

  if (argc != 4) {
    Tcl_SetResult(interp, (char *) "wrong # args", TCL_STATIC);
    return TCL_ERROR;
  }

  fname = strdup(argv[1]);
  type1 = strdup(argv[2]);
  type2 = strdup(argv[3]);

  BddscoutCopyFormula(fname,type1,type2);

  free(fname);
  free(type1);
  free(type2);

  Tcl_SetResult(interp, (char *) "", TCL_STATIC);
  return TCL_OK;
}

/**Function****************************************************************
  Synopsis    []
  Description [general command]
  SideEffects []
  SeeAlso     []
  ************************************************************************/

static int
BddscoutConstructCmd(ClientData clientData, Tcl_Interp *interp, int argc,
                  const char **argv)
{
  Biddy_String s1,s2,s3,s4;
  int numV,numN;

  if (argc != 5) {
    Tcl_SetResult(interp, (char *) "wrong # args", TCL_STATIC);
    return TCL_ERROR;
  }

  s1 = strdup(argv[1]); /* number of variables */
  s2 = strdup(argv[2]); /* list of variables */
  s3 = strdup(argv[3]); /* number of nodes */
  s4 = strdup(argv[4]); /* BDD */

  numV = atoi(s1);
  numN = atoi(s3);

  BddscoutConstructBDD(numV,s2,numN,s4);

  free(s1);
  free(s2);
  free(s3);
  free(s4);

  Tcl_SetResult(interp, (char *) "", TCL_STATIC);
  return TCL_OK;
}

/**Function****************************************************************
  Synopsis    []
  Description [general command]
  SideEffects []
  SeeAlso     []
  ************************************************************************/

static int
BddscoutWriteBddviewCmd(ClientData clientData, Tcl_Interp *interp, int argc,
                  const char **argv)
{
  Biddy_String s1,s2,s2x,s3;
  int OK;

  /* DEBUGGING */
  /*
  printf("\nBddscoutWriteBddviewCmd started (GetActiveManager = %p)\n", Bddscout_GetActiveManager());
  */

#if defined(MINGW) || defined(_MSC_VER)
  char* docdir;
  char* appdir;
  docdir = getenv("USERPROFILE");
  appdir = strdup("\\AppData\\Local\\");
#endif

  if (argc != 4) {
    Tcl_SetResult(interp, (char *) "wrong # args", TCL_STATIC);
    return TCL_ERROR;
  }

  s1 = strdup(argv[1]);
  s2 = strdup(argv[2]);
  s3 = strdup(argv[3]);

#if defined(MINGW) || defined(_MSC_VER)
  s2x = (Biddy_String) malloc(strlen(docdir)+strlen(appdir)+strlen(s2)+1);
  if (!s2x) return TCL_ERROR;
  s2x[0] = 0;
  strcat(s2x,docdir);
  strcat(s2x,appdir);
  strcat(s2x,s2);
  free(appdir);
#elif UNIX
  s2x = (Biddy_String) malloc(strlen(s2)+6);
  s2x[0] = 0;
  strcat(s2x,"/tmp/");
  strcat(s2x,s2);
#else
  s2x = strdup(s2);
#endif

  OK = BddscoutWriteBddview(s2x,s3,s1);

  free(s1);
  free(s2);
  free(s3);

  if (!OK) {
    printf("BddscoutWriteBddview error\n");
  }

  Tcl_SetResult(interp, s2x, TCL_VOLATILE);
  free(s2x);

  return TCL_OK;
}

/**Function****************************************************************
  Synopsis    []
  Description [command to prepare lists used in menus and dialogs]
  SideEffects []
  SeeAlso     []
  ************************************************************************/

static int
BddscoutListVariablesByPositionCmd(ClientData clientData, Tcl_Interp *interp,
                    int argc, const char **argv)
{
  Biddy_String list;

  list = strdup("");

  BddscoutListVariablesByPosition(&list);

  Tcl_SetResult(interp, list, TCL_VOLATILE);
  free(list);

  return TCL_OK;
}

/**Function****************************************************************
  Synopsis    []
  Description [command to prepare lists used in menus and dialogs]
  SideEffects []
  SeeAlso     []
  ************************************************************************/

static int
BddscoutListVariablesByNameCmd(ClientData clientData, Tcl_Interp *interp,
                    int argc, const char **argv)
{
  Biddy_String list;

  list = strdup("");

  BddscoutListVariablesByName(&list);

  Tcl_SetResult(interp, list, TCL_VOLATILE);
  free(list);

  return TCL_OK;
}

/**Function****************************************************************
  Synopsis    []
  Description [command to prepare lists used in menus and dialogs]
  SideEffects []
  SeeAlso     []
  ************************************************************************/

static int
BddscoutListVariablesByOrderCmd(ClientData clientData, Tcl_Interp *interp,
                    int argc, const char **argv)
{
  Biddy_String list;

  list = strdup("");

  BddscoutListVariablesByOrder(&list);

  Tcl_SetResult(interp, list, TCL_VOLATILE);
  free(list);

  return TCL_OK;
}

/**Function****************************************************************
  Synopsis    []
  Description [command to prepare lists used in menus and dialogs]
  SideEffects []
  SeeAlso     []
  ************************************************************************/

static int
BddscoutListVariablesByNumberCmd(ClientData clientData, Tcl_Interp *interp,
                    int argc, const char **argv)
{
  Biddy_String list;

  list = strdup("");

  BddscoutListVariablesByNumber(&list);

  Tcl_SetResult(interp, list, TCL_VOLATILE);
  free(list);

  return TCL_OK;
}

/**Function****************************************************************
  Synopsis    []
  Description [command to prepare lists used in menus and dialogs]
  SideEffects []
  SeeAlso     []
  ************************************************************************/

static int
BddscoutListFormulaeByNameCmd(ClientData clientData, Tcl_Interp *interp, int argc,
                    const char **argv)
{
  Biddy_String list;

  list = strdup("NULL"); /* not required, but nice */

  BddscoutListFormulaByName(&list);

  Tcl_SetResult(interp, list, TCL_VOLATILE);
  free(list);

  return TCL_OK;
}

/**Function****************************************************************
  Synopsis    []
  Description [command to prepare lists used in menus and dialogs]
  SideEffects []
  SeeAlso     []
  ************************************************************************/

static int
BddscoutListFormulaeByNodeNumberCmd(ClientData clientData,
             Tcl_Interp *interp, int argc, const char **argv)
{
  Biddy_String list;

  list = strdup("NULL"); /* required! */

  BddscoutListFormulaByUIntParameter(&list,MyNodeNumber);

  Tcl_SetResult(interp, list, TCL_VOLATILE);
  free(list);

  return TCL_OK;
}

/**Function****************************************************************
  Synopsis    []
  Description [command to prepare lists used in menus and dialogs]
  SideEffects []
  SeeAlso     []
  ************************************************************************/

static int
BddscoutListFormulaeByNodeMaxLevelCmd(ClientData clientData,
             Tcl_Interp *interp, int argc, const char **argv)
{
  Biddy_String list;

  list = strdup("NULL"); /* required! */

  BddscoutListFormulaByUIntParameter(&list,MyNodeMaxLevel);

  Tcl_SetResult(interp, list, TCL_VOLATILE);
  free(list);

  return TCL_OK;
}

/**Function****************************************************************
  Synopsis    []
  Description [command to prepare lists used in menus and dialogs]
  SideEffects []
  SeeAlso     []
  ************************************************************************/

static int
BddscoutListFormulaeByNodeAvgLevelCmd(ClientData clientData,
             Tcl_Interp *interp, int argc, const char **argv)
{
  Biddy_String list;

  list = strdup("NULL"); /* required! */

  BddscoutListFormulaByFloatParameter(&list,MyNodeAvgLevel);

  Tcl_SetResult(interp, list, TCL_VOLATILE);
  free(list);

  return TCL_OK;
}

/**Function****************************************************************
  Synopsis    []
  Description [command to prepare lists used in menus and dialogs]
  SideEffects []
  SeeAlso     []
  ************************************************************************/

static int
BddscoutListFormulaeByPathNumberCmd(ClientData clientData,
             Tcl_Interp *interp, int argc, const char **argv)
{
  Biddy_String list;

  list = strdup("NULL"); /* required! */

  BddscoutListFormulaByUIntParameter(&list,MyPathNumber);

  Tcl_SetResult(interp, list, TCL_VOLATILE);
  free(list);

  return TCL_OK;
}

/**Function****************************************************************
  Synopsis    []
  Description [command to prepare lists used in menus and dialogs]
  SideEffects []
  SeeAlso     []
  ************************************************************************/

static int
BddscoutListFormulaeByMintermNumberCmd(ClientData clientData,
             Tcl_Interp *interp, int argc, const char **argv)
{
  Biddy_String list;

  list = strdup("NULL"); /* required! */

  BddscoutListFormulaByFloatParameter(&list,MyMintermNumber);

  Tcl_SetResult(interp, list, TCL_VOLATILE);
  free(list);

  return TCL_OK;
}

/**Function****************************************************************
  Synopsis    []
  Description [command used to import files of various formats]
  SideEffects []
  SeeAlso     []
  ************************************************************************/

static int
BddscoutReadBDDCmd(ClientData clientData, Tcl_Interp *interp, int argc,
                  const char **argv)
{
  Biddy_String s1;
  FILE *funfile;
  Biddy_String name;

  if (argc != 2) {
    Tcl_SetResult(interp, (char *) "wrong # args", TCL_STATIC);
    return TCL_ERROR;
  }

  s1 = strdup(argv[1]);

  funfile = fopen(s1,"r");
  if (!funfile) {
    printf("BddscoutReadBDDCmd: File error (%s)!\n",s1);
    free(s1);
    return TCL_ERROR;
  }

  name = BddscoutReadBDD(funfile);

  fclose(funfile);

  free(s1);

  Tcl_SetResult(interp, name, TCL_VOLATILE);
  free(name);

  return TCL_OK;
}

/**Function****************************************************************
  Synopsis    []
  Description [command used to import files of various formats]
  SideEffects []
  SeeAlso     []
  ************************************************************************/

static int
BddscoutReadBFCmd(ClientData clientData, Tcl_Interp *interp, int argc,
                  const char **argv)
{
  Biddy_String s1;
  FILE *funfile;
  Biddy_String name;

  if (argc != 2) {
    Tcl_SetResult(interp, (char *) "wrong # args", TCL_STATIC);
    return TCL_ERROR;
  }

  s1 = strdup(argv[1]);

  funfile = fopen(s1,"r");
  if (!funfile) {
    printf("BddscoutReadBFCmd: File error (%s)!\n",s1);
    free(s1);
    return TCL_ERROR;
  }

  name = BddscoutReadBF(funfile);

  fclose(funfile);

  free(s1);

  Tcl_SetResult(interp, name, TCL_VOLATILE);
  free(name);

  return TCL_OK;
}

/**Function****************************************************************
  Synopsis    []
  Description [command used to parse strings from input line]
  SideEffects []
  SeeAlso     []
  ************************************************************************/

static int BddscoutParseInputInfixCmd(ClientData clientData, Tcl_Interp *interp,
                    int argc, const char **argv)
{
  Biddy_Manager MNG;
  Biddy_String expr,name,form;
  unsigned int n,n1,n2,n3,n4;
  Biddy_Edge tmp,bdd;
  unsigned int idx;

  if (argc != 2) {
    Tcl_SetResult(interp, (char *) "wrong # args", TCL_STATIC);
    return TCL_ERROR;
  }

  expr = strdup(argv[1]);

  /* DEBUGGING */
  /*
  printf("PARSE: <%s>\n",expr);
  */

  MNG = Bddscout_GetActiveManager();

  name = NULL;
  form = NULL;
  if (strcmp(expr,"")) {

    n1 = (unsigned int) strspn(expr," ");
    n2 = (unsigned int) strcspn(&expr[n1]," =");
    n3 = (unsigned int) strspn(&expr[n1+n2]," ");

    if (expr[n1] != '=') {
      if ( (n1+n2+n3) < strlen(expr) ) {
        if (expr[n1+n2+n3] == '=') {
          expr[n1+n2] = 0;
          name = strdup(&expr[n1]);
          n4 = (unsigned int) strspn(&expr[n1+n2+n3+1]," ");
          form = strdup(&expr[n1+n2+n3+1+n4]);
        } else {
          /* name = strdup("NONAME"); */
          form = strdup(&expr[n1]);
        }
      } else {
        /* name = strdup("NONAME"); */
        form = strdup(&expr[n1]);
      }
    } else {
      /* name = strdup("NONAME"); */
      n4 = (unsigned int) strspn(&expr[n1+1]," ");
      form = strdup(&expr[n1+1+n4]);
    }

    tmp = NULL;
    bdd = NULL;

    if (!name) {
      n = Biddy_Managed_FormulaTableNum(MNG);
      name = strdup("F12345678"); /* max is 8-digit number */
      sprintf(name,"F%u",n);
    }
    else if (Biddy_Managed_FindFormula(MNG,name,&idx,&tmp)) {
      if (!idx) {
        /* THERE EXISTS A VARIABLE WITH THE SAME NAME! */
        free(name);
        name = NULL;
      }
    }

    /* DEBUGGING */
    /*
    printf("NAME: <%s>\n",name);
    printf("FORM: <%s>\n",form);
    */
    
    if (name) {
      bdd = Biddy_Managed_Eval2(MNG,form);
      if (Biddy_IsNull(bdd)) {
        free(name);
        name = NULL;
      } else if (bdd != tmp) {
        Biddy_Managed_AddPersistentFormula(MNG,name,bdd);
        /* ADAPT FORMULA WITH THE SAME NAME IN OTHER MANAGERS */
        BddscoutSyncFormula(form,name);
      }
    }

    if (name) {
      Tcl_SetResult(interp, name, TCL_VOLATILE);
    } else {
      Tcl_SetResult(interp, "", TCL_VOLATILE);
    }

    if (name) free(name);
    free(form);

  } else {

    Tcl_SetResult(interp, "", TCL_VOLATILE);

  }

  free(expr);

  return TCL_OK;
}

/**Function****************************************************************
  Synopsis    []
  Description [command used to perform Biddy functions exported from biddyMain.c]
  SideEffects []
  SeeAlso     []
  ************************************************************************/

static int
BiddySwapWithHigherCmd(ClientData clientData, Tcl_Interp *interp, int argc,
                  const char **argv)
{
  Biddy_Manager MNG;
  Biddy_String s1;
  Biddy_Variable v;
  Biddy_Boolean find;

  if (argc != 2) {
    Tcl_SetResult(interp, (char *) "wrong # args", TCL_STATIC);
    return TCL_ERROR;
  }

  s1 = strdup(argv[1]);

  MNG = Bddscout_GetActiveManager();

  v = 0;
  find = FALSE;
  while (!find && v<Biddy_Managed_VariableTableNum(MNG)) {
    if (!strcmp(s1,Biddy_Managed_GetVariableName(MNG,v))) {
      find = TRUE;
    } else {
      v++;
    }
  }
  free(s1);

  /* IF THE ELEMENT WAS NOT FOUND... */
  if (find) {
    Biddy_Managed_SwapWithHigher(MNG,v);
  }

  Tcl_SetResult(interp, (char *) "", TCL_STATIC);
  return TCL_OK;
}

/**Function****************************************************************
  Synopsis    []
  Description [command used to perform Biddy functions exported from biddyMain.c]
  SideEffects []
  SeeAlso     []
  ************************************************************************/

static int
BiddySwapWithLowerCmd(ClientData clientData, Tcl_Interp *interp, int argc,
                  const char **argv)
{
  Biddy_Manager MNG;
  Biddy_String s1;
  Biddy_Variable v;
  Biddy_Boolean find;

  if (argc != 2) {
    Tcl_SetResult(interp, (char *) "wrong # args", TCL_STATIC);
    return TCL_ERROR;
  }

  s1 = strdup(argv[1]);

  MNG = Bddscout_GetActiveManager();

  v = 0;
  find = FALSE;
  while (!find && v<Biddy_Managed_VariableTableNum(MNG)) {
    if (!strcmp(s1,Biddy_Managed_GetVariableName(MNG,v))) {
      find = TRUE;
    } else {
      v++;
    }
  }
  free(s1);

  /* IF THE ELEMENT WAS NOT FOUND... */
  if (find) {
    Biddy_Managed_SwapWithLower(MNG,v);
  }

  Tcl_SetResult(interp, (char *) "", TCL_STATIC);
  return TCL_OK;
}

/**Function****************************************************************
  Synopsis    []
  Description [command used to perform Biddy functions exported from biddyMain.c]
  SideEffects []
  SeeAlso     []
  ************************************************************************/

static int
BiddySiftingCmd(ClientData clientData, Tcl_Interp *interp, int argc,
                  const char **argv)
{
  Biddy_Manager MNG;

  if (argc != 1) {
    Tcl_SetResult(interp, (char *) "wrong # args", TCL_STATIC);
    return TCL_ERROR;
  }

  MNG = Bddscout_GetActiveManager();

  Biddy_Managed_Sifting(MNG,NULL,FALSE);

  Tcl_SetResult(interp, (char *) "", TCL_STATIC);
  return TCL_OK;

}

/**Function****************************************************************
  Synopsis    []
  Description [command used to perform Biddy functions exported from biddyMain.c]
  SideEffects []
  SeeAlso     []
  ************************************************************************/

static int
BiddySiftingOnFunctionCmd(ClientData clientData, Tcl_Interp *interp, int argc,
                  const char **argv)
{
  Biddy_Manager MNG;
  Biddy_String s1;
  Biddy_Edge f;
  unsigned int idx;

  if (argc != 2) {
    Tcl_SetResult(interp, (char *) "wrong # args", TCL_STATIC);
    return TCL_ERROR;
  }

  s1 = strdup(argv[1]);

  MNG = Bddscout_GetActiveManager();

  if (Biddy_Managed_FindFormula(MNG,s1,&idx,&f)) {
    Biddy_Managed_Sifting(MNG,f,FALSE);
  }

  free(s1);

  Tcl_SetResult(interp, (char *) "", TCL_STATIC);
  return TCL_OK;

}

/**Function****************************************************************
  Synopsis    []
  Description [command used to perform Biddy functions exported from biddyStat.c]
  SideEffects []
  SeeAlso     []
  ************************************************************************/

static int
BiddyNodeNumberCmd(ClientData clientData, Tcl_Interp *interp, int argc,
                  const char **argv)
{
  Biddy_Manager MNG;
  Biddy_String s1;
  unsigned int idx;
  Biddy_Edge f;
  Biddy_String sup;

  if (argc != 2) {
    Tcl_SetResult(interp, (char *) "wrong # args", TCL_STATIC);
    return TCL_ERROR;
  }

  s1 = strdup(argv[1]);

  MNG = Bddscout_GetActiveManager();

  sup = (Biddy_String) malloc(127);
  if (!sup) return TCL_ERROR;

  if (!Biddy_Managed_FindFormula(MNG,s1,&idx,&f)) {
    printf("BiddyNodeNumberCmd: Function %s does not exists!\n",s1);
    sprintf(sup,"0");
  } else {
    sprintf(sup,"%u",Biddy_Managed_NodeNumber(MNG,f));
  }

  free(s1);

  Tcl_SetResult(interp, sup, TCL_VOLATILE);
  free(sup);

  return TCL_OK;
}

/**Function****************************************************************
  Synopsis    []
  Description [command used to perform Biddy functions exported from biddyStat.c]
  SideEffects []
  SeeAlso     []
  ************************************************************************/

static int
BiddyNodeMaxLevelCmd(ClientData clientData, Tcl_Interp *interp, int argc,
                  const char **argv)
{
  Biddy_Manager MNG;
  Biddy_String s1;
  unsigned int idx;
  Biddy_Edge f;
  Biddy_String sup;

  if (argc != 2) {
    Tcl_SetResult(interp, (char *) "wrong # args", TCL_STATIC);
    return TCL_ERROR;
  }

  s1 = strdup(argv[1]);

  MNG = Bddscout_GetActiveManager();

  sup = (Biddy_String) malloc(127);
  if (!sup) return TCL_ERROR;

  if (!Biddy_Managed_FindFormula(MNG,s1,&idx,&f)) {
    printf("BiddyNodeMaxLevelCmd: Function %s does not exists!\n",s1);
    sprintf(sup,"0");
  } else {
    sprintf(sup,"%u",Biddy_Managed_NodeMaxLevel(MNG,f));
  }

  free(s1);

  Tcl_SetResult(interp, sup, TCL_VOLATILE);
  free(sup);

  return TCL_OK;
}

/**Function****************************************************************
  Synopsis    []
  Description [command used to perform Biddy functions exported from biddyStat.c]
  SideEffects []
  SeeAlso     []
  ************************************************************************/

static int
BiddyNodeAvgLevelCmd(ClientData clientData, Tcl_Interp *interp, int argc,
                  const char **argv)
{
  Biddy_Manager MNG;
  Biddy_String s1;
  unsigned int idx;
  Biddy_Edge f;
  Biddy_String sup;

  if (argc != 2) {
    Tcl_SetResult(interp, (char *) "wrong # args", TCL_STATIC);
    return TCL_ERROR;
  }

  s1 = strdup(argv[1]);

  MNG = Bddscout_GetActiveManager();

  sup = (Biddy_String) malloc(127);
  if (!sup) return TCL_ERROR;

  if (!Biddy_Managed_FindFormula(MNG,s1,&idx,&f)) {
    printf("BiddyNodeAvgLevelCmd: Function %s does not exists!\n",s1);
    sprintf(sup,"0");
  } else {
    sprintf(sup,"%f",Biddy_Managed_NodeAvgLevel(MNG,f));
  }

  free(s1);

  Tcl_SetResult(interp, sup, TCL_VOLATILE);
  free(sup);

  return TCL_OK;
}

/**Function****************************************************************
  Synopsis    []
  Description [command used to perform Biddy functions exported from biddyStat.c]
  SideEffects []
  SeeAlso     []
  ************************************************************************/

static int
BiddyVariableTableNumCmd(ClientData clientData, Tcl_Interp *interp, int argc,
                  const char **argv)
{
  Biddy_Manager MNG;
  Biddy_String sup;

  if (argc != 1) {
    Tcl_SetResult(interp, (char *) "wrong # args", TCL_STATIC);
    return TCL_ERROR;
  }

  MNG = Bddscout_GetActiveManager();

  sup = (Biddy_String) malloc(127);
  if (!sup) return TCL_ERROR;
  sprintf(sup,"%u",Biddy_Managed_VariableTableNum(MNG));
  Tcl_SetResult(interp, sup, TCL_VOLATILE);
  free(sup);

  return TCL_OK;
}

/**Function****************************************************************
  Synopsis    []
  Description [command used to perform Biddy functions exported from biddyStat.c]
  SideEffects []
  SeeAlso     []
  ************************************************************************/

static int
BiddyNodeTableSizeCmd(ClientData clientData, Tcl_Interp *interp, int argc,
                  const char **argv)
{
  Biddy_Manager MNG;
  Biddy_String sup;

  if (argc != 1) {
    Tcl_SetResult(interp, (char *) "wrong # args", TCL_STATIC);
    return TCL_ERROR;
  }

  MNG = Bddscout_GetActiveManager();

  sup = (Biddy_String) malloc(127);
  if (!sup) return TCL_ERROR;
  sprintf(sup,"%u",Biddy_Managed_NodeTableSize(MNG));
  Tcl_SetResult(interp, sup, TCL_VOLATILE);
  free(sup);

  return TCL_OK;
}

/**Function****************************************************************
  Synopsis    []
  Description [command used to perform Biddy functions exported from biddyStat.c]
  SideEffects []
  SeeAlso     []
  ************************************************************************/

static int
BiddyNodeTableBlockNumberCmd(ClientData clientData, Tcl_Interp *interp, int argc,
                  const char **argv)
{
  Biddy_Manager MNG;
  Biddy_String sup;

  if (argc != 1) {
    Tcl_SetResult(interp, (char *) "wrong # args", TCL_STATIC);
    return TCL_ERROR;
  }

  MNG = Bddscout_GetActiveManager();

  sup = (Biddy_String) malloc(127);
  if (!sup) return TCL_ERROR;
  sprintf(sup,"%u",Biddy_Managed_NodeTableBlockNumber(MNG));
  Tcl_SetResult(interp, sup, TCL_VOLATILE);
  free(sup);

  return TCL_OK;
}

/**Function****************************************************************
  Synopsis    []
  Description [command used to perform Biddy functions exported from biddyStat.c]
  SideEffects []
  SeeAlso     []
  ************************************************************************/

static int
BiddyNodeTableGeneratedCmd(ClientData clientData, Tcl_Interp *interp, int argc,
                  const char **argv)
{
  Biddy_Manager MNG;
  Biddy_String sup;

  if (argc != 1) {
    Tcl_SetResult(interp, (char *) "wrong # args", TCL_STATIC);
    return TCL_ERROR;
  }

  MNG = Bddscout_GetActiveManager();

  sup = (Biddy_String) malloc(127);
  if (!sup) return TCL_ERROR;
  sprintf(sup,"%u",Biddy_Managed_NodeTableGenerated(MNG));
  Tcl_SetResult(interp, sup, TCL_VOLATILE);
  free(sup);

  return TCL_OK;
}

/**Function****************************************************************
  Synopsis    []
  Description [command used to perform Biddy functions exported from biddyStat.c]
  SideEffects []
  SeeAlso     []
  ************************************************************************/

static int
BiddyNodeTableMaxCmd(ClientData clientData, Tcl_Interp *interp, int argc,
                  const char **argv)
{
  Biddy_Manager MNG;
  Biddy_String sup;

  if (argc != 1) {
    Tcl_SetResult(interp, (char *) "wrong # args", TCL_STATIC);
    return TCL_ERROR;
  }

  MNG = Bddscout_GetActiveManager();

  sup = (Biddy_String) malloc(127);
  if (!sup) return TCL_ERROR;
  sprintf(sup,"%u",Biddy_Managed_NodeTableMax(MNG));
  Tcl_SetResult(interp, sup, TCL_VOLATILE);
  free(sup);

  return TCL_OK;
}

/**Function****************************************************************
  Synopsis    []
  Description [command used to perform Biddy functions exported from biddyStat.c]
  SideEffects []
  SeeAlso     []
  ************************************************************************/

static int
BiddyNodeTableNumCmd(ClientData clientData, Tcl_Interp *interp, int argc,
                  const char **argv)
{
  Biddy_Manager MNG;
  Biddy_String sup;

  if (argc != 1) {
    Tcl_SetResult(interp, (char *) "wrong # args", TCL_STATIC);
    return TCL_ERROR;
  }

  MNG = Bddscout_GetActiveManager();

  sup = (Biddy_String) malloc(127);
  if (!sup) return TCL_ERROR;
  sprintf(sup,"%u",Biddy_Managed_NodeTableNum(MNG));
  Tcl_SetResult(interp, sup, TCL_VOLATILE);
  free(sup);

  return TCL_OK;
}

/**Function****************************************************************
  Synopsis    []
  Description [command used to perform Biddy functions exported from biddyStat.c]
  SideEffects []
  SeeAlso     []
  ************************************************************************/

static int
BiddyNodeTableNumVarCmd(ClientData clientData, Tcl_Interp *interp, int argc,
                  const char **argv)
{
  Biddy_Manager MNG;
  Biddy_String s1;
  Biddy_Variable v;
  Biddy_Boolean find;
  Biddy_String sup;

  if (argc != 2) {
    Tcl_SetResult(interp, (char *) "wrong # args", TCL_STATIC);
    return TCL_ERROR;
  }

  s1 = strdup(argv[1]);

  MNG = Bddscout_GetActiveManager();

  v = 0;
  find = FALSE;
  while (!find && v<Biddy_Managed_VariableTableNum(MNG)) {
    if (!strcmp(s1,Biddy_Managed_GetVariableName(MNG,v))) {
      find = TRUE;
    } else {
      v++;
    }
  }
  free(s1);

  /* IF THE ELEMENT WAS NOT FOUND... */
  if (!find) {
    sup = strdup("0");
  } else {
    sup = (Biddy_String) malloc(127);
    if (!sup) return TCL_ERROR;
    sprintf(sup,"%u",Biddy_Managed_NodeTableNumVar(MNG,v));
  }

  Tcl_SetResult(interp, sup, TCL_VOLATILE);
  free(sup);

  return TCL_OK;
}

/**Function****************************************************************
  Synopsis    []
  Description [command used to perform Biddy functions exported from biddyStat.c]
  SideEffects []
  SeeAlso     []
  ************************************************************************/

static int
BiddyNodeTableResizeNumberCmd(ClientData clientData, Tcl_Interp *interp, int argc,
                  const char **argv)
{
  Biddy_Manager MNG;
  Biddy_String sup;

  if (argc != 1) {
    Tcl_SetResult(interp, (char *) "wrong # args", TCL_STATIC);
    return TCL_ERROR;
  }

  MNG = Bddscout_GetActiveManager();

  sup = (Biddy_String) malloc(127);
  if (!sup) return TCL_ERROR;
  sprintf(sup,"%u",Biddy_Managed_NodeTableResizeNumber(MNG));
  Tcl_SetResult(interp, sup, TCL_VOLATILE);
  free(sup);

  return TCL_OK;
}

/**Function****************************************************************
  Synopsis    []
  Description [command used to perform Biddy functions exported from biddyStat.c]
  SideEffects []
  SeeAlso     []
  ************************************************************************/

static int
BiddyNodeTableFoaNumberCmd(ClientData clientData, Tcl_Interp *interp, int argc,
                  const char **argv)
{
  Biddy_Manager MNG;
  Biddy_String sup;

  if (argc != 1) {
    Tcl_SetResult(interp, (char *) "wrong # args", TCL_STATIC);
    return TCL_ERROR;
  }

  MNG = Bddscout_GetActiveManager();

  sup = (Biddy_String) malloc(127);
  if (!sup) return TCL_ERROR;

#if defined(MINGW) || defined(_MSC_VER)
  sprintf(sup,"%I64u",Biddy_Managed_NodeTableFoaNumber(MNG)); /* MINGW AND VS?? REQUIRE "%I64u" */
#else
  sprintf(sup,"%llu",Biddy_Managed_NodeTableFoaNumber(MNG));
#endif

  Tcl_SetResult(interp, sup, TCL_VOLATILE);
  free(sup);

  return TCL_OK;
}

/**Function****************************************************************
  Synopsis    []
  Description [command used to perform Biddy functions exported from biddyStat.c]
  SideEffects []
  SeeAlso     []
  ************************************************************************/

static int
BiddyNodeTableFindNumberCmd(ClientData clientData, Tcl_Interp *interp, int argc,
                  const char **argv)
{
  Biddy_Manager MNG;
  Biddy_String sup;

  if (argc != 1) {
    Tcl_SetResult(interp, (char *) "wrong # args", TCL_STATIC);
    return TCL_ERROR;
  }

  MNG = Bddscout_GetActiveManager();

  sup = (Biddy_String) malloc(127);
  if (!sup) return TCL_ERROR;

#if defined(MINGW) || defined(_MSC_VER)
  sprintf(sup,"%I64u",Biddy_Managed_NodeTableFindNumber(MNG)); /* MINGW AND VS?? REQUIRE "%I64u" */
#else
  sprintf(sup,"%llu",Biddy_Managed_NodeTableFindNumber(MNG));
#endif

  Tcl_SetResult(interp, sup, TCL_VOLATILE);
  free(sup);

  return TCL_OK;
}

/**Function****************************************************************
  Synopsis    []
  Description [command used to perform Biddy functions exported from biddyStat.c]
  SideEffects []
  SeeAlso     []
  ************************************************************************/

static int
BiddyNodeTableCompareNumberCmd(ClientData clientData, Tcl_Interp *interp, int argc,
                  const char **argv)
{
  Biddy_Manager MNG;
  Biddy_String sup;

  if (argc != 1) {
    Tcl_SetResult(interp, (char *) "wrong # args", TCL_STATIC);
    return TCL_ERROR;
  }

  MNG = Bddscout_GetActiveManager();

  sup = (Biddy_String) malloc(127);
  if (!sup) return TCL_ERROR;

#if defined(MINGW) || defined(_MSC_VER)
  sprintf(sup,"%I64u",Biddy_Managed_NodeTableCompareNumber(MNG)); /* MINGW AND VS?? REQUIRE "%I64u" */
#else
  sprintf(sup,"%llu",Biddy_Managed_NodeTableCompareNumber(MNG));
#endif

  Tcl_SetResult(interp, sup, TCL_VOLATILE);
  free(sup);

  return TCL_OK;
}

/**Function****************************************************************
  Synopsis    []
  Description [command used to perform Biddy functions exported from biddyStat.c]
  SideEffects []
  SeeAlso     []
  ************************************************************************/

static int
BiddyNodeTableAddNumberCmd(ClientData clientData, Tcl_Interp *interp, int argc,
                  const char **argv)
{
  Biddy_Manager MNG;
  Biddy_String sup;

  if (argc != 1) {
    Tcl_SetResult(interp, (char *) "wrong # args", TCL_STATIC);
    return TCL_ERROR;
  }

  MNG = Bddscout_GetActiveManager();

  sup = (Biddy_String) malloc(127);
  if (!sup) return TCL_ERROR;

#if defined(MINGW) || defined(_MSC_VER)
  sprintf(sup,"%I64u",Biddy_Managed_NodeTableAddNumber(MNG)); /* MINGW AND VS?? REQUIRE "%I64u" */
#else
  sprintf(sup,"%llu",Biddy_Managed_NodeTableAddNumber(MNG));
#endif

  Tcl_SetResult(interp, sup, TCL_VOLATILE);
  free(sup);

  return TCL_OK;
}

/**Function****************************************************************
  Synopsis    []
  Description [command used to perform Biddy functions exported from biddyStat.c]
  SideEffects []
  SeeAlso     []
  ************************************************************************/

static int
BiddyNodeTableGCNumberCmd(ClientData clientData, Tcl_Interp *interp, int argc,
                  const char **argv)
{
  Biddy_Manager MNG;
  Biddy_String sup;

  if (argc != 1) {
    Tcl_SetResult(interp, (char *) "wrong # args", TCL_STATIC);
    return TCL_ERROR;
  }

  MNG = Bddscout_GetActiveManager();

  sup = (Biddy_String) malloc(127);
  if (!sup) return TCL_ERROR;
  sprintf(sup,"%u",Biddy_Managed_NodeTableGCNumber(MNG));
  Tcl_SetResult(interp, sup, TCL_VOLATILE);
  free(sup);

  return TCL_OK;
}

/**Function****************************************************************
  Synopsis    []
  Description [command used to perform Biddy functions exported from biddyStat.c]
  SideEffects []
  SeeAlso     []
  ************************************************************************/

static int
BiddyNodeTableGCTimeCmd(ClientData clientData, Tcl_Interp *interp, int argc,
                  const char **argv)
{
  Biddy_Manager MNG;
  Biddy_String sup;

  if (argc != 1) {
    Tcl_SetResult(interp, (char *) "wrong # args", TCL_STATIC);
    return TCL_ERROR;
  }

  MNG = Bddscout_GetActiveManager();

  sup = (Biddy_String) malloc(127);
  if (!sup) return TCL_ERROR;
  sprintf(sup,"%.3fs",Biddy_Managed_NodeTableGCTime(MNG) / (1.0 * CLOCKS_PER_SEC));
  Tcl_SetResult(interp, sup, TCL_VOLATILE);
  free(sup);

  return TCL_OK;
}

/**Function****************************************************************
  Synopsis    []
  Description [command used to perform Biddy functions exported from biddyStat.c]
  SideEffects []
  SeeAlso     []
  ************************************************************************/

static int
BiddyNodeTableGCObsoleteNumberCmd(ClientData clientData, Tcl_Interp *interp, int argc,
                  const char **argv)
{
  Biddy_Manager MNG;
  Biddy_String sup;

  if (argc != 1) {
    Tcl_SetResult(interp, (char *) "wrong # args", TCL_STATIC);
    return TCL_ERROR;
  }

  MNG = Bddscout_GetActiveManager();

  sup = (Biddy_String) malloc(127);
  if (!sup) return TCL_ERROR;

#if defined(MINGW) || defined(_MSC_VER)
  sprintf(sup,"%I64u",Biddy_Managed_NodeTableGCObsoleteNumber(MNG)); /* MINGW AND VS?? REQUIRE "%I64u" */
#else
  sprintf(sup,"%llu",Biddy_Managed_NodeTableGCObsoleteNumber(MNG));
#endif

  Tcl_SetResult(interp, sup, TCL_VOLATILE);
  free(sup);

  return TCL_OK;
}

/**Function****************************************************************
  Synopsis    []
  Description [command used to perform Biddy functions exported from biddyStat.c]
  SideEffects []
  SeeAlso     []
  ************************************************************************/

static int
BiddyNodeTableSwapNumberCmd(ClientData clientData, Tcl_Interp *interp, int argc,
                  const char **argv)
{
  Biddy_Manager MNG;
  Biddy_String sup;

  if (argc != 1) {
    Tcl_SetResult(interp, (char *) "wrong # args", TCL_STATIC);
    return TCL_ERROR;
  }

  MNG = Bddscout_GetActiveManager();

  sup = (Biddy_String) malloc(127);
  if (!sup) return TCL_ERROR;
  sprintf(sup,"%u",Biddy_Managed_NodeTableSwapNumber(MNG));
  Tcl_SetResult(interp, sup, TCL_VOLATILE);
  free(sup);

  return TCL_OK;
}

/**Function****************************************************************
  Synopsis    []
  Description [command used to perform Biddy functions exported from biddyStat.c]
  SideEffects []
  SeeAlso     []
  ************************************************************************/

static int
BiddyNodeTableSiftingNumberCmd(ClientData clientData, Tcl_Interp *interp, int argc,
                  const char **argv)
{
  Biddy_Manager MNG;
  Biddy_String sup;

  if (argc != 1) {
    Tcl_SetResult(interp, (char *) "wrong # args", TCL_STATIC);
    return TCL_ERROR;
  }

  MNG = Bddscout_GetActiveManager();

  sup = (Biddy_String) malloc(127);
  if (!sup) return TCL_ERROR;
  sprintf(sup,"%u",Biddy_Managed_NodeTableSiftingNumber(MNG));
  Tcl_SetResult(interp, sup, TCL_VOLATILE);
  free(sup);

  return TCL_OK;
}

/**Function****************************************************************
  Synopsis    []
  Description [command used to perform Biddy functions exported from biddyStat.c]
  SideEffects []
  SeeAlso     []
  ************************************************************************/

static int
BiddyNodeTableDRTimeCmd(ClientData clientData, Tcl_Interp *interp, int argc,
                  const char **argv)
{
  Biddy_Manager MNG;
  Biddy_String sup;

  if (argc != 1) {
    Tcl_SetResult(interp, (char *) "wrong # args", TCL_STATIC);
    return TCL_ERROR;
  }

  MNG = Bddscout_GetActiveManager();

  sup = (Biddy_String) malloc(127);
  if (!sup) return TCL_ERROR;
  sprintf(sup,"%.3fs",Biddy_Managed_NodeTableDRTime(MNG) / (1.0 * CLOCKS_PER_SEC));
  Tcl_SetResult(interp, sup, TCL_VOLATILE);
  free(sup);

  return TCL_OK;
}

/**Function****************************************************************
  Synopsis    []
  Description [command used to perform Biddy functions exported from biddyStat.c]
  SideEffects []
  SeeAlso     []
  ************************************************************************/

static int
BiddyNodeTableITENumberCmd(ClientData clientData, Tcl_Interp *interp, int argc,
                  const char **argv)
{
  Biddy_Manager MNG;
  Biddy_String sup;

  if (argc != 1) {
    Tcl_SetResult(interp, (char *) "wrong # args", TCL_STATIC);
    return TCL_ERROR;
  }

  MNG = Bddscout_GetActiveManager();

  sup = (Biddy_String) malloc(127);
  if (!sup) return TCL_ERROR;
  sprintf(sup,"%u",Biddy_Managed_NodeTableITENumber(MNG));
  Tcl_SetResult(interp, sup, TCL_VOLATILE);
  free(sup);

  return TCL_OK;
}

/**Function****************************************************************
  Synopsis    []
  Description [command used to perform Biddy functions exported from biddyStat.c]
  SideEffects []
  SeeAlso     []
  ************************************************************************/

static int
BiddyNodeTableITERecursiveNumberCmd(ClientData clientData, Tcl_Interp *interp, int argc,
                  const char **argv)
{
  Biddy_Manager MNG;
  Biddy_String sup;

  if (argc != 1) {
    Tcl_SetResult(interp, (char *) "wrong # args", TCL_STATIC);
    return TCL_ERROR;
  }

  MNG = Bddscout_GetActiveManager();

  sup = (Biddy_String) malloc(127);
  if (!sup) return TCL_ERROR;

#if defined(MINGW) || defined(_MSC_VER)
  sprintf(sup,"%I64u",Biddy_Managed_NodeTableITERecursiveNumber(MNG)); /* MINGW AND VS?? REQUIRE "%I64u" */
#else
  sprintf(sup,"%llu",Biddy_Managed_NodeTableITERecursiveNumber(MNG));
#endif

  Tcl_SetResult(interp, sup, TCL_VOLATILE);
  free(sup);

  return TCL_OK;
}

/**Function****************************************************************
  Synopsis    []
  Description [command used to perform Biddy functions exported from biddyStat.c]
  SideEffects []
  SeeAlso     []
  ************************************************************************/

static int
BiddyNodeTableANDORNumberCmd(ClientData clientData, Tcl_Interp *interp, int argc,
                  const char **argv)
{
  Biddy_Manager MNG;
  Biddy_String sup;

  if (argc != 1) {
    Tcl_SetResult(interp, (char *) "wrong # args", TCL_STATIC);
    return TCL_ERROR;
  }

  MNG = Bddscout_GetActiveManager();

  sup = (Biddy_String) malloc(127);
  if (!sup) return TCL_ERROR;
  sprintf(sup,"%u",Biddy_Managed_NodeTableANDORNumber(MNG));
  Tcl_SetResult(interp, sup, TCL_VOLATILE);
  free(sup);

  return TCL_OK;
}

/**Function****************************************************************
  Synopsis    []
  Description [command used to perform Biddy functions exported from biddyStat.c]
  SideEffects []
  SeeAlso     []
  ************************************************************************/

static int
BiddyNodeTableANDORRecursiveNumberCmd(ClientData clientData, Tcl_Interp *interp, int argc,
                  const char **argv)
{
  Biddy_Manager MNG;
  Biddy_String sup;

  if (argc != 1) {
    Tcl_SetResult(interp, (char *) "wrong # args", TCL_STATIC);
    return TCL_ERROR;
  }

  MNG = Bddscout_GetActiveManager();

  sup = (Biddy_String) malloc(127);
  if (!sup) return TCL_ERROR;

#if defined(MINGW) || defined(_MSC_VER)
  sprintf(sup,"%I64u",Biddy_Managed_NodeTableANDORRecursiveNumber(MNG)); /* MINGW AND VS?? REQUIRE "%I64u" */
#else
  sprintf(sup,"%llu",Biddy_Managed_NodeTableANDORRecursiveNumber(MNG));
#endif

  Tcl_SetResult(interp, sup, TCL_VOLATILE);
  free(sup);

  return TCL_OK;
}

/**Function****************************************************************
  Synopsis    []
  Description [command used to perform Biddy functions exported from biddyStat.c]
  SideEffects []
  SeeAlso     []
  ************************************************************************/

static int
BiddyNodeTableXORNumberCmd(ClientData clientData, Tcl_Interp *interp, int argc,
                  const char **argv)
{
  Biddy_Manager MNG;
  Biddy_String sup;

  if (argc != 1) {
    Tcl_SetResult(interp, (char *) "wrong # args", TCL_STATIC);
    return TCL_ERROR;
  }

  MNG = Bddscout_GetActiveManager();

  sup = (Biddy_String) malloc(127);
  if (!sup) return TCL_ERROR;
  sprintf(sup,"%u",Biddy_Managed_NodeTableXORNumber(MNG));
  Tcl_SetResult(interp, sup, TCL_VOLATILE);
  free(sup);

  return TCL_OK;
}

/**Function****************************************************************
  Synopsis    []
  Description [command used to perform Biddy functions exported from biddyStat.c]
  SideEffects []
  SeeAlso     []
  ************************************************************************/

static int
BiddyNodeTableXORRecursiveNumberCmd(ClientData clientData, Tcl_Interp *interp, int argc,
                  const char **argv)
{
  Biddy_Manager MNG;
  Biddy_String sup;

  if (argc != 1) {
    Tcl_SetResult(interp, (char *) "wrong # args", TCL_STATIC);
    return TCL_ERROR;
  }

  MNG = Bddscout_GetActiveManager();

  sup = (Biddy_String) malloc(127);
  if (!sup) return TCL_ERROR;

#if defined(MINGW) || defined(_MSC_VER)
  sprintf(sup,"%I64u",Biddy_Managed_NodeTableXORRecursiveNumber(MNG)); /* MINGW AND VS?? REQUIRE "%I64u" */
#else
  sprintf(sup,"%llu",Biddy_Managed_NodeTableXORRecursiveNumber(MNG));
#endif

  Tcl_SetResult(interp, sup, TCL_VOLATILE);
  free(sup);

  return TCL_OK;
}

/**Function****************************************************************
  Synopsis    []
  Description [command used to perform Biddy functions exported from biddyStat.c]
  SideEffects []
  SeeAlso     []
  ************************************************************************/

static int
BiddyFormulaTableNumCmd(ClientData clientData, Tcl_Interp *interp, int argc,
                  const char **argv)
{
  Biddy_Manager MNG;
  Biddy_String sup;

  if (argc != 1) {
    Tcl_SetResult(interp, (char *) "wrong # args", TCL_STATIC);
    return TCL_ERROR;
  }

  MNG = Bddscout_GetActiveManager();

  sup = (Biddy_String) malloc(127);
  if (!sup) return TCL_ERROR;
  sprintf(sup,"%u",Biddy_Managed_FormulaTableNum(MNG));
  Tcl_SetResult(interp, sup, TCL_VOLATILE);
  free(sup);

  return TCL_OK;
}

/**Function****************************************************************
  Synopsis    []
  Description [command used to perform Biddy functions exported from biddyStat.c]
  SideEffects []
  SeeAlso     []
  ************************************************************************/

static int
BiddyListUsedCmd(ClientData clientData, Tcl_Interp *interp, int argc,
                  const char **argv)
{
  Biddy_Manager MNG;
  Biddy_String sup;

  if (argc != 1) {
    Tcl_SetResult(interp, (char *) "wrong # args", TCL_STATIC);
    return TCL_ERROR;
  }

  MNG = Bddscout_GetActiveManager();

  sup = (Biddy_String) malloc(127);
  if (!sup) return TCL_ERROR;
  sprintf(sup,"%u",Biddy_Managed_ListUsed(MNG));
  Tcl_SetResult(interp, sup, TCL_VOLATILE);
  free(sup);

  return TCL_OK;
}

/**Function****************************************************************
  Synopsis    []
  Description [command used to perform Biddy functions exported from biddyStat.c]
  SideEffects []
  SeeAlso     []
  ************************************************************************/

static int
BiddyListMaxLengthCmd(ClientData clientData, Tcl_Interp *interp, int argc,
                  const char **argv)
{
  Biddy_Manager MNG;
  Biddy_String sup;

  if (argc != 1) {
    Tcl_SetResult(interp, (char *) "wrong # args", TCL_STATIC);
    return TCL_ERROR;
  }

  MNG = Bddscout_GetActiveManager();

  sup = (Biddy_String) malloc(127);
  if (!sup) return TCL_ERROR;
  sprintf(sup,"%u",Biddy_Managed_ListMaxLength(MNG));
  Tcl_SetResult(interp, sup, TCL_VOLATILE);
  free(sup);

  return TCL_OK;
}

/**Function****************************************************************
  Synopsis    []
  Description [command used to perform Biddy functions exported from biddyStat.c]
  SideEffects []
  SeeAlso     []
  ************************************************************************/

static int
BiddyListAvgLengthCmd(ClientData clientData, Tcl_Interp *interp, int argc,
                  const char **argv)
{
  Biddy_Manager MNG;
  Biddy_String sup;

  if (argc != 1) {
    Tcl_SetResult(interp, (char *) "wrong # args", TCL_STATIC);
    return TCL_ERROR;
  }

  MNG = Bddscout_GetActiveManager();

  sup = (Biddy_String) malloc(127);
  if (!sup) return TCL_ERROR;
  sprintf(sup,"%.2f",Biddy_Managed_ListAvgLength(MNG));
  Tcl_SetResult(interp, sup, TCL_VOLATILE);
  free(sup);

  return TCL_OK;
}

/**Function****************************************************************
  Synopsis    []
  Description [command used to perform Biddy functions exported from biddyStat.c]
  SideEffects []
  SeeAlso     []
  ************************************************************************/

static int
BiddyOPCacheSearchCmd(ClientData clientData, Tcl_Interp *interp, int argc,
                  const char **argv)
{
  Biddy_Manager MNG;
  Biddy_String sup;

  if (argc != 1) {
    Tcl_SetResult(interp, (char *) "wrong # args", TCL_STATIC);
    return TCL_ERROR;
  }

  MNG = Bddscout_GetActiveManager();

  sup = (Biddy_String) malloc(127);
  if (!sup) return TCL_ERROR;

#if defined(MINGW) || defined(_MSC_VER)
  sprintf(sup,"%I64u",Biddy_Managed_OPCacheSearch(MNG)); /* MINGW AND VS?? REQUIRE "%I64u" */
#else
  sprintf(sup,"%llu",Biddy_Managed_OPCacheSearch(MNG));
#endif

  Tcl_SetResult(interp, sup, TCL_VOLATILE);
  free(sup);

  return TCL_OK;
}

/**Function****************************************************************
  Synopsis    []
  Description [command used to perform Biddy functions exported from biddyStat.c]
  SideEffects []
  SeeAlso     []
  ************************************************************************/

static int
BiddyOPCacheFindCmd(ClientData clientData, Tcl_Interp *interp, int argc,
                  const char **argv)
{
  Biddy_Manager MNG;
  Biddy_String sup;

  if (argc != 1) {
    Tcl_SetResult(interp, (char *) "wrong # args", TCL_STATIC);
    return TCL_ERROR;
  }

  MNG = Bddscout_GetActiveManager();

  sup = (Biddy_String) malloc(127);
  if (!sup) return TCL_ERROR;

#if defined(MINGW) || defined(_MSC_VER)
  sprintf(sup,"%I64u",Biddy_Managed_OPCacheFind(MNG)); /* MINGW AND VS?? REQUIRE "%I64u" */
#else
  sprintf(sup,"%llu",Biddy_Managed_OPCacheFind(MNG));
#endif

  Tcl_SetResult(interp, sup, TCL_VOLATILE);
  free(sup);

  return TCL_OK;
}

/**Function****************************************************************
  Synopsis    []
  Description [command used to perform Biddy functions exported from biddyStat.c]
  SideEffects []
  SeeAlso     []
  ************************************************************************/

static int
BiddyOPCacheInsertCmd(ClientData clientData, Tcl_Interp *interp, int argc,
                  const char **argv)
{
  Biddy_Manager MNG;
  Biddy_String sup;

  if (argc != 1) {
    Tcl_SetResult(interp, (char *) "wrong # args", TCL_STATIC);
    return TCL_ERROR;
  }

  MNG = Bddscout_GetActiveManager();

  sup = (Biddy_String) malloc(127);
  if (!sup) return TCL_ERROR;

#if defined(MINGW) || defined(_MSC_VER)
  sprintf(sup,"%I64u",Biddy_Managed_OPCacheInsert(MNG)); /* MINGW AND VS?? REQUIRE "%I64u" */
#else
  sprintf(sup,"%llu",Biddy_Managed_OPCacheInsert(MNG));
#endif

  Tcl_SetResult(interp, sup, TCL_VOLATILE);
  free(sup);

  return TCL_OK;
}

/**Function****************************************************************
  Synopsis    []
  Description [command used to perform Biddy functions exported from biddyStat.c]
  SideEffects []
  SeeAlso     []
  ************************************************************************/

static int
BiddyOPCacheOverwriteCmd(ClientData clientData, Tcl_Interp *interp, int argc,
                  const char **argv)
{
  Biddy_Manager MNG;
  Biddy_String sup;

  if (argc != 1) {
    Tcl_SetResult(interp, (char *) "wrong # args", TCL_STATIC);
    return TCL_ERROR;
  }

  MNG = Bddscout_GetActiveManager();

  sup = (Biddy_String) malloc(127);
  if (!sup) return TCL_ERROR;

#if defined(MINGW) || defined(_MSC_VER)
  sprintf(sup,"%I64u",Biddy_Managed_OPCacheOverwrite(MNG)); /* MINGW AND VS?? REQUIRE "%I64u" */
#else
  sprintf(sup,"%llu",Biddy_Managed_OPCacheOverwrite(MNG));
#endif

  Tcl_SetResult(interp, sup, TCL_VOLATILE);
  free(sup);

  return TCL_OK;
}

/**Function****************************************************************
  Synopsis    []
  Description [command used to perform Biddy functions exported from biddyStat.c]
  SideEffects []
  SeeAlso     []
  ************************************************************************/

static int
BiddyNodeNumberPlainCmd(ClientData clientData, Tcl_Interp *interp, int argc,
                  const char **argv)
{
  Biddy_Manager MNG;
  Biddy_String s1;
  unsigned int idx;
  Biddy_Edge f;
  Biddy_String sup;

  if (argc != 2) {
    Tcl_SetResult(interp, (char *) "wrong # args", TCL_STATIC);
    return TCL_ERROR;
  }

  s1 = strdup(argv[1]);

  MNG = Bddscout_GetActiveManager();

  sup = (Biddy_String) malloc(127);
  if (!sup) return TCL_ERROR;

  if (!Biddy_Managed_FindFormula(MNG,s1,&idx,&f)) {
    printf("BiddyNodeNumberPlainCmd: Function %s does not exists!\n",s1);
    sprintf(sup,"0");
  } else {
    sprintf(sup,"%u",Biddy_Managed_NodeNumberPlain(MNG,f));
  }

  free(s1);

  Tcl_SetResult(interp, sup, TCL_VOLATILE);
  free(sup);

  return TCL_OK;
}

/**Function****************************************************************
  Synopsis    []
  Description [command used to perform Biddy functions exported from biddyStat.c]
  SideEffects []
  SeeAlso     []
  ************************************************************************/

static int
BiddyDependentVariableNumberCmd(ClientData clientData, Tcl_Interp *interp, int argc,
                  const char **argv)
{
  Biddy_Manager MNG;
  Biddy_String s1;
  unsigned int idx;
  Biddy_Edge f;
  Biddy_String sup;

  if (argc != 2) {
    Tcl_SetResult(interp, (char *) "wrong # args", TCL_STATIC);
    return TCL_ERROR;
  }

  s1 = strdup(argv[1]);

  MNG = Bddscout_GetActiveManager();

  sup = (Biddy_String) malloc(127);
  if (!sup) return TCL_ERROR;

  if (!Biddy_Managed_FindFormula(MNG,s1,&idx,&f)) {
    printf("BiddyDependentVariableNumberCmd: Function %s does not exists!\n",s1);
    sprintf(sup,"0");
  } else {
    sprintf(sup,"%u",Biddy_Managed_DependentVariableNumber(MNG,f));
  }

  free(s1);

  Tcl_SetResult(interp, sup, TCL_VOLATILE);
  free(sup);

  return TCL_OK;
}

/**Function****************************************************************
  Synopsis    []
  Description [command used to perform Biddy functions exported from biddyStat.c]
  SideEffects []
  SeeAlso     []
  ************************************************************************/

static int
BiddyNodeVarNumberCmd(ClientData clientData, Tcl_Interp *interp, int argc,
                  const char **argv)
{
  /* Biddy_Manager MNG; */
  Biddy_String sup;

  if (argc != 1) {
    Tcl_SetResult(interp, (char *) "wrong # args", TCL_STATIC);
    return TCL_ERROR;
  }

  /* MNG = Bddscout_GetActiveManager(); */

  sup = (Biddy_String) malloc(127);
  if (!sup) return TCL_ERROR;
  Tcl_SetResult(interp, (char *) "not implemented, yet", TCL_STATIC);
  free(sup);

  return TCL_OK;
}

/**Function****************************************************************
  Synopsis    []
  Description [command used to perform Biddy functions exported from biddyStat.c]
  SideEffects []
  SeeAlso     []
  ************************************************************************/

static int
BiddyCountComplementedCmd(ClientData clientData, Tcl_Interp *interp, int argc,
                          const char **argv)
{
  Biddy_Manager MNG;
  Biddy_String s1;
  unsigned int idx;
  Biddy_Edge f;
  Biddy_String sup;

  if (argc != 2) {
    Tcl_SetResult(interp, (char *) "wrong # args", TCL_STATIC);
    return TCL_ERROR;
  }

  s1 = strdup(argv[1]);

  MNG = Bddscout_GetActiveManager();

  sup = (Biddy_String) malloc(127);
  if (!sup) return TCL_ERROR;

  if (!Biddy_Managed_FindFormula(MNG,s1,&idx,&f)) {
    printf("BiddyCountComplementedCmd: Function %s does not exists!\n",s1);
    sprintf(sup,"0");
  } else {
    sprintf(sup,"%u",Biddy_Managed_CountComplemented(MNG,f));
  }

  free(s1);

  Tcl_SetResult(interp, sup, TCL_VOLATILE);
  free(sup);

  return TCL_OK;
}

/**Function****************************************************************
  Synopsis    []
  Description [command used to perform Biddy functions exported from biddyStat.c]
  SideEffects []
  SeeAlso     []
  ************************************************************************/

static int
BiddyCountPathsCmd(ClientData clientData, Tcl_Interp *interp, int argc,
                  const char **argv)
{
  Biddy_Manager MNG;
  Biddy_String s1;
  unsigned int idx;
  Biddy_Edge f;
  Biddy_String sup;

  if (argc != 2) {
    Tcl_SetResult(interp, (char *) "wrong # args", TCL_STATIC);
    return TCL_ERROR;
  }

  s1 = strdup(argv[1]);

  MNG = Bddscout_GetActiveManager();

  sup = (Biddy_String) malloc(127);
  if (!sup) return TCL_ERROR;

  if (!Biddy_Managed_FindFormula(MNG,s1,&idx,&f)) {
    printf("BiddyCountPathsCmd: Function %s does not exists!\n",s1);
    sprintf(sup,"0");
  } else {
#if defined(MINGW) || defined(_MSC_VER)
    sprintf(sup,"%I64u",Biddy_Managed_CountPaths(MNG,f)); /* MINGW AND VS?? REQUIRE "%I64u" */
#else
    sprintf(sup,"%llu",Biddy_Managed_CountPaths(MNG,f));
#endif
  }

  free(s1);

  Tcl_SetResult(interp, sup, TCL_VOLATILE);
  free(sup);

  return TCL_OK;
}

/**Function****************************************************************
  Synopsis    []
  Description [command used to perform Biddy functions exported from biddyStat.c]
  SideEffects []
  SeeAlso     []
  ************************************************************************/

static int
BiddyCountMintermCmd(ClientData clientData, Tcl_Interp *interp, int argc,
                  const char **argv)
{
  Biddy_Manager MNG;
  Biddy_String s1,s2;
  unsigned int idx,nvars;
  Biddy_Edge f;
  Biddy_String sup;

  if (argc != 3) {
    Tcl_SetResult(interp, (char *) "wrong # args", TCL_STATIC);
    return TCL_ERROR;
  }

  s1 = strdup(argv[1]);
  s2 = strdup(argv[2]);

  MNG = Bddscout_GetActiveManager();

  nvars = atoi(s2);

  sup = (Biddy_String) malloc(127);
  if (!sup) return TCL_ERROR;

  if (!Biddy_Managed_FindFormula(MNG,s1,&idx,&f)) {
    printf("BiddyCountMintermCmd: Function %s does not exists!\n",s1);
    sprintf(sup,"0");
  } else {
    sprintf(sup,"%.0f",Biddy_Managed_CountMinterm(MNG,f,nvars));
  }

  free(s1);
  free(s2);

  Tcl_SetResult(interp, sup, TCL_VOLATILE);
  free(sup);

  return TCL_OK;
}

/**Function****************************************************************
  Synopsis    []
  Description [command used to perform Biddy functions exported from biddyStat.c]
  SideEffects []
  SeeAlso     []
  ************************************************************************/

static int
BiddyDensityFunctionCmd(ClientData clientData, Tcl_Interp *interp, int argc,
                  const char **argv)
{
  Biddy_Manager MNG;
  Biddy_String s1,s2;
  unsigned int idx,nvars;
  Biddy_Edge f;
  Biddy_String sup;

  if (argc != 3) {
    Tcl_SetResult(interp, (char *) "wrong # args", TCL_STATIC);
    return TCL_ERROR;
  }

  s1 = strdup(argv[1]);
  s2 = strdup(argv[2]);

  MNG = Bddscout_GetActiveManager();

  nvars = atoi(s2);

  sup = (Biddy_String) malloc(127);
  if (!sup) return TCL_ERROR;

  if (!Biddy_Managed_FindFormula(MNG,s1,&idx,&f)) {
    printf("BiddyDensityFunctionCmd: Function %s does not exists!\n",s1);
    sprintf(sup,"0");
  } else {
    sprintf(sup,"%.2f",Biddy_Managed_DensityFunction(MNG,f,nvars));
  }

  free(s1);
  free(s2);

  Tcl_SetResult(interp, sup, TCL_VOLATILE);
  free(sup);

  return TCL_OK;
}

/**Function****************************************************************
  Synopsis    []
  Description [command used to perform Biddy functions exported from biddyStat.c]
  SideEffects []
  SeeAlso     []
  ************************************************************************/

static int
BiddyDensityBDDCmd(ClientData clientData, Tcl_Interp *interp, int argc,
                  const char **argv)
{
  Biddy_Manager MNG;
  Biddy_String s1,s2;
  unsigned int idx,nvars;
  Biddy_Edge f;
  Biddy_String sup;

  if (argc != 3) {
    Tcl_SetResult(interp, (char *) "wrong # args", TCL_STATIC);
    return TCL_ERROR;
  }

  s1 = strdup(argv[1]);
  s2 = strdup(argv[2]);

  MNG = Bddscout_GetActiveManager();

  nvars = atoi(s2);

  sup = (Biddy_String) malloc(127);
  if (!sup) return TCL_ERROR;

  if (!Biddy_Managed_FindFormula(MNG,s1,&idx,&f)) {
    printf("BiddyDensityBDDCmd: Function %s does not exists!\n",s1);
    sprintf(sup,"0");
  } else {
    sprintf(sup,"%.2f",Biddy_Managed_DensityBDD(MNG,f,nvars));
  }

  free(s1);
  free(s2);

  Tcl_SetResult(interp, sup, TCL_VOLATILE);
  free(sup);

  return TCL_OK;
}

/**Function****************************************************************
  Synopsis    []
  Description [command used to perform Biddy functions exported from biddyStat.c]
  SideEffects []
  SeeAlso     []
  ************************************************************************/

static int
BiddyReadMemoryInUseCmd(ClientData clientData, Tcl_Interp *interp, int argc,
                  const char **argv)
{
  Biddy_Manager MNG;
  Biddy_String sup;

  if (argc != 1) {
    Tcl_SetResult(interp, (char *) "wrong # args", TCL_STATIC);
    return TCL_ERROR;
  }

  MNG = Bddscout_GetActiveManager();

  sup = (Biddy_String) malloc(127);
  if (!sup) return TCL_ERROR;

#if defined(MINGW) || defined(_MSC_VER)
  sprintf(sup,"%I64u",Biddy_Managed_ReadMemoryInUse(MNG)); /* MINGW AND VS?? REQUIRE "%I64u" */
#else
  sprintf(sup,"%llu",Biddy_Managed_ReadMemoryInUse(MNG));
#endif

  Tcl_SetResult(interp, sup, TCL_VOLATILE);
  free(sup);

  return TCL_OK;
}

/**Function****************************************************************
  Synopsis    []
  Description [command used to perform Biddy functions exported from biddyStat.c]
  SideEffects []
  SeeAlso     []
  ************************************************************************/

static int
BiddyPrintInfoCmd(ClientData clientData, Tcl_Interp *interp, int argc,
                  const char **argv)
{
  /* Biddy_Manager MNG; */
  Biddy_String sup;

  if (argc != 1) {
    Tcl_SetResult(interp, (char *) "wrong # args", TCL_STATIC);
    return TCL_ERROR;
  }

  /* MNG = Bddscout_GetActiveManager(); */

  sup = (Biddy_String) malloc(127);
  if (!sup) return TCL_ERROR;
  Tcl_SetResult(interp, (char *) "not implemented, yet", TCL_STATIC);
  free(sup);

  return TCL_OK;
}

/**Function****************************************************************
  Synopsis    []
  Description [command used to perform Biddy functions exported from biddyInOut.c]
  SideEffects []
  SeeAlso     []
  ************************************************************************/

static int
BiddyEval0Cmd(ClientData clientData, Tcl_Interp *interp, int argc,
                  const char **argv)
{
  /* Biddy_Manager MNG; */
  Biddy_String sup;

  if (argc != 2) {
    Tcl_SetResult(interp, (char *) "wrong # args", TCL_STATIC);
    return TCL_ERROR;
  }

  /* MNG = Bddscout_GetActiveManager(); */

  sup = (Biddy_String) malloc(127);
  if (!sup) return TCL_ERROR;
  Tcl_SetResult(interp, (char *) "not implemented, yet", TCL_STATIC);
  free(sup);

  return TCL_OK;
}

/**Function****************************************************************
  Synopsis    []
  Description [command used to perform Biddy functions exported from biddyInOut.c]
  SideEffects []
  SeeAlso     []
  ************************************************************************/

static int
BiddyEval1Cmd(ClientData clientData, Tcl_Interp *interp, int argc,
                  const char **argv)
{
  /* Biddy_Manager MNG; */
  Biddy_String sup;

  if (argc != 2) {
    Tcl_SetResult(interp, (char *) "wrong # args", TCL_STATIC);
    return TCL_ERROR;
  }

  /* MNG = Bddscout_GetActiveManager(); */

  sup = (Biddy_String) malloc(127);
  if (!sup) return TCL_ERROR;
  Tcl_SetResult(interp, (char *) "not implemented, yet", TCL_STATIC);
  free(sup);

  return TCL_OK;
}

/**Function****************************************************************
  Synopsis    []
  Description [command used to perform Biddy functions exported from biddyInOut.c]
  SideEffects []
  SeeAlso     []
  ************************************************************************/

static int
BiddyEval2Cmd(ClientData clientData, Tcl_Interp *interp, int argc,
                  const char **argv)
{
  /* Biddy_Manager MNG; */
  Biddy_String sup;

  if (argc != 2) {
    Tcl_SetResult(interp, (char *) "wrong # args", TCL_STATIC);
    return TCL_ERROR;
  }

  /* MNG = Bddscout_GetActiveManager(); */

  sup = (Biddy_String) malloc(127);
  if (!sup) return TCL_ERROR;
  Tcl_SetResult(interp, (char *) "not implemented, yet", TCL_STATIC);
  free(sup);

  return TCL_OK;
}

/**Function****************************************************************
  Synopsis    []
  Description [command used to perform Biddy functions exported from biddyInOut.c]
  SideEffects []
  SeeAlso     []
  ************************************************************************/

static int
BiddyReadVerilogFileCmd(ClientData clientData, Tcl_Interp *interp, int argc,
                  const char **argv)
{
  /* Biddy_Manager MNG; */
  Biddy_String sup;

  if (argc != 2) {
    Tcl_SetResult(interp, (char *) "wrong # args", TCL_STATIC);
    return TCL_ERROR;
  }

  /* MNG = Bddscout_GetActiveManager(); */

  sup = (Biddy_String) malloc(127);
  if (!sup) return TCL_ERROR;
  Tcl_SetResult(interp, (char *) "not implemented, yet", TCL_STATIC);
  free(sup);

  return TCL_OK;
}

/**Function****************************************************************
  Synopsis    []
  Description [command used to perform Biddy functions exported from biddyInOut.c]
  SideEffects []
  SeeAlso     []
  ************************************************************************/

static int
BiddyPrintfBDDCmd(ClientData clientData, Tcl_Interp *interp, int argc,
                  const char **argv)
{
  /* Biddy_Manager MNG; */
  Biddy_String sup;

  if (argc != 2) {
    Tcl_SetResult(interp, (char *) "wrong # args", TCL_STATIC);
    return TCL_ERROR;
  }

  /* MNG = Bddscout_GetActiveManager(); */

  sup = (Biddy_String) malloc(127);
  if (!sup) return TCL_ERROR;
  Tcl_SetResult(interp, (char *) "not implemented, yet", TCL_STATIC);
  free(sup);

  return TCL_OK;
}

/**Function****************************************************************
  Synopsis    []
  Description [command used to perform Biddy functions exported from biddyInOut.c]
  SideEffects []
  SeeAlso     []
  ************************************************************************/

static int
BiddyWriteBDDCmd(ClientData clientData, Tcl_Interp *interp, int argc,
                  const char **argv)
{
  Biddy_Manager MNG;
  Biddy_String s1,s2;
  unsigned int idx;
  Biddy_Edge f;

  if (argc != 3) {
    Tcl_SetResult(interp, (char *) "wrong # args", TCL_STATIC);
    return TCL_ERROR;
  }

  s1 = strdup(argv[1]);
  s2 = strdup(argv[2]);

  MNG = Bddscout_GetActiveManager();

  if (!Biddy_Managed_FindFormula(MNG,s2,&idx,&f)) {
    printf("BddscoutWriteBDD: Function %s does not exists!\n",s1);
  } else {
    Biddy_Managed_WriteBDD(MNG,s1,f,s2);
  }

  free(s1);
  free(s2);

  Tcl_SetResult(interp, (char *) "", TCL_STATIC);
  return TCL_OK;
}

/**Function****************************************************************
  Synopsis    []
  Description [command used to perform Biddy functions exported from biddyInOut.c]
  SideEffects []
  SeeAlso     []
  ************************************************************************/

static int
BiddyPrintfTableCmd(ClientData clientData, Tcl_Interp *interp, int argc,
                  const char **argv)
{
  /* Biddy_Manager MNG; */
  Biddy_String sup;

  if (argc != 2) {
    Tcl_SetResult(interp, (char *) "wrong # args", TCL_STATIC);
    return TCL_ERROR;
  }

  /* MNG = Bddscout_GetActiveManager(); */

  sup = (Biddy_String) malloc(127);
  if (!sup) return TCL_ERROR;
  Tcl_SetResult(interp, (char *) "not implemented, yet", TCL_STATIC);
  free(sup);

  return TCL_OK;
}

/**Function****************************************************************
  Synopsis    []
  Description [command used to perform Biddy functions exported from biddyInOut.c]
  SideEffects []
  SeeAlso     []
  ************************************************************************/

static int
BiddyWriteTableCmd(ClientData clientData, Tcl_Interp *interp, int argc,
                  const char **argv)
{
  /* Biddy_Manager MNG; */
  Biddy_String sup;

  if (argc != 3) {
    Tcl_SetResult(interp, (char *) "wrong # args", TCL_STATIC);
    return TCL_ERROR;
  }

  /* MNG = Bddscout_GetActiveManager(); */

  sup = (Biddy_String) malloc(127);
  if (!sup) return TCL_ERROR;
  Tcl_SetResult(interp, (char *) "not implemented, yet", TCL_STATIC);
  free(sup);

  return TCL_OK;
}

/**Function****************************************************************
  Synopsis    []
  Description [command used to perform Biddy functions exported from biddyInOut.c]
  SideEffects []
  SeeAlso     []
  ************************************************************************/

static int
BiddyPrintfSOPCmd(ClientData clientData, Tcl_Interp *interp, int argc,
                  const char **argv)
{
  Biddy_Manager MNG;
  Biddy_String s1;
  unsigned int idx;
  Biddy_Edge f;

  if (argc != 2) {
    Tcl_SetResult(interp, (char *) "wrong # args", TCL_STATIC);
    return TCL_ERROR;
  }

  s1 = strdup(argv[1]);

  MNG = Bddscout_GetActiveManager();

  if (!Biddy_Managed_FindFormula(MNG,s1,&idx,&f)) {

    printf("BiddyPrintfSOPCmd: Function %s does not exists!\n",s1);
    Tcl_SetResult(interp, (char *) "", TCL_STATIC);

  } else {

    FILE *tmpfile;
    char line[256];
    Biddy_String oneline,result;
    char* tmpname;
#if defined(MINGW) || defined(_MSC_VER)
    char* docdir;
    char* appdir;
    docdir = getenv("USERPROFILE");
    appdir = strdup("\\AppData\\Local\\");
#endif

#if defined(MINGW) || defined(_MSC_VER)
    tmpname = (Biddy_String) malloc(strlen(docdir)+strlen(appdir)+7+1);
    tmpname[0] = 0;
    strcat(tmpname,docdir);
    strcat(tmpname,appdir);
    strcat(tmpname,"biddy.tmp");
    free(appdir);
#elif UNIX
    tmpname = strdup("/tmp/XXXXXX");
    close(mkstemp(tmpname));
#else
    tmpname = strdup("biddy.tmp");
#endif

    /* DEBUGGING */
    /*
    Biddy_Managed_PrintfSOP(MNG,f);
    */

    Biddy_Managed_WriteSOP(MNG,tmpname,f);

    result = NULL;
    tmpfile = fopen(tmpname,"r");
    if (!tmpfile) {
      fprintf(stderr,"BiddyPrintfSOPCmd: File error (%s)!\n",tmpname);
    } else {
      oneline = fgets(line,255,tmpfile);
      if (oneline) result = strdup(oneline);
      while (oneline) {
        oneline = fgets(oneline,255,tmpfile);
        if (oneline) concat(&result,oneline);
      }
      
      fclose(tmpfile);
    }

    free(tmpname);

    Tcl_SetResult(interp, result, TCL_VOLATILE);
    free(result);
  }

  free(s1);

  return TCL_OK;
}

/**Function****************************************************************
  Synopsis    []
  Description [command used to perform Biddy functions exported from biddyInOut.c]
  SideEffects []
  SeeAlso     []
  ************************************************************************/

static int
BiddyWriteSOPCmd(ClientData clientData, Tcl_Interp *interp, int argc,
                  const char **argv)
{
  /* Biddy_Manager MNG; */
  Biddy_String sup;

  if (argc != 3) {
    Tcl_SetResult(interp, (char *) "wrong # args", TCL_STATIC);
    return TCL_ERROR;
  }

  /* MNG = Bddscout_GetActiveManager(); */

  sup = (Biddy_String) malloc(127);
  if (!sup) return TCL_ERROR;
  Tcl_SetResult(interp, (char *) "not implemented, yet", TCL_STATIC);
  free(sup);

  return TCL_OK;
}

/**Function****************************************************************
  Synopsis    []
  Description [command used to perform Biddy functions exported from biddyInOut.c]
  SideEffects []
  SeeAlso     []
  ************************************************************************/

static int
BiddyWriteDotCmd(ClientData clientData, Tcl_Interp *interp, int argc,
                  const char **argv)
{
  /* Biddy_Manager MNG; */
  Biddy_String sup;

  if (argc != 3) {
    Tcl_SetResult(interp, (char *) "wrong # args", TCL_STATIC);
    return TCL_ERROR;
  }

  /* MNG = Bddscout_GetActiveManager(); */

  sup = (Biddy_String) malloc(127);
  if (!sup) return TCL_ERROR;
  Tcl_SetResult(interp, (char *) "not implemented, yet", TCL_STATIC);
  free(sup);

  return TCL_OK;
}

/**Function****************************************************************
  Synopsis    []
  Description [command used to perform Biddy functions exported from biddyInOut.c]
  SideEffects []
  SeeAlso     []
  ************************************************************************/

static int
BiddyWriteBddviewCmd(ClientData clientData, Tcl_Interp *interp, int argc,
                  const char **argv)
{
  /* Biddy_Manager MNG; */
  Biddy_String sup;

  if (argc != 3) {
    Tcl_SetResult(interp, (char *) "wrong # args", TCL_STATIC);
    return TCL_ERROR;
  }

  /* MNG = Bddscout_GetActiveManager(); */

  sup = (Biddy_String) malloc(127);
  if (!sup) return TCL_ERROR;
  Tcl_SetResult(interp, (char *) "not implemented, yet", TCL_STATIC);
  free(sup);

  return TCL_OK;
}

/*-----------------------------------------------------------------------*/
/* Definition of static functions                                        */
/*-----------------------------------------------------------------------*/

static unsigned int 
MyNodeNumber(Biddy_Edge f)
{
  Biddy_Manager MNG;

  MNG = Bddscout_GetActiveManager();
  return Biddy_Managed_NodeNumber(MNG,f);
}

static unsigned int
MyNodeMaxLevel(Biddy_Edge f)
{
  return Biddy_NodeMaxLevel(f);
}

static float
MyNodeAvgLevel(Biddy_Edge f)
{
  return Biddy_NodeAvgLevel(f);
}

static unsigned int
MyPathNumber(Biddy_Edge f)
{
  Biddy_Manager MNG;
  unsigned int n;

  MNG = Bddscout_GetActiveManager();
  n = (unsigned int) Biddy_Managed_CountPaths(MNG,f); /* returns unsigned long long int */
  return n;
}

static float
MyMintermNumber(Biddy_Edge f)
{
  Biddy_Manager MNG;
  float n;

  MNG = Bddscout_GetActiveManager();
  n = (float) Biddy_Managed_CountMinterm(MNG,f,0); /* returns double */
  return n;
}

static void
concat(char **s1, const char *s2)
{
   if (s2) {
     *s1 = (char*) realloc(*s1,strlen(*s1)+strlen(s2)+1+1);
     strcat(*s1," ");
     strcat(*s1,s2);
   }
}

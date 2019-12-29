/**CFile*******************************************************************
  PackageName [Bdd Scout]
  Synopsis    [Bdd Scout]

  FileName    [bddscoutTcl.c]
  Revision    [$Revision: 555 $]
  Date        [$Date: 2019-10-04 17:21:27 +0200 (pet, 04 okt 2019) $]
  Authors     [Robert Meolic (robert@meolic.com)]
  Description [File bddscoutTcl.c contains definitions of Tcl commands,
               which can be used for manipulating with BDDs from
               Tcl interpreter (e.g. tclsh or wish).]
  SeeAlso     [bddscout.h, bddscout.c, bddscout.tcl]

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

static int BddscoutAboutPkgCmd(ClientData clientData, Tcl_Interp *interp, int argc, const char **argv);

/* ***************** */
/* internal commands */
/* ***************** */

static int BddscoutChangeTypeCmd(ClientData clientData, Tcl_Interp *interp, int argc, const char **argv);

static int BddscoutCheckFormulaCmd(ClientData clientData, Tcl_Interp *interp, int argc, const char **argv);

static int BddscoutCopyFormulaCmd(ClientData clientData, Tcl_Interp *interp, int argc, const char **argv);

static int BddscoutConstructCmd(ClientData clientData, Tcl_Interp *interp, int argc, const char **argv);

static int BddscoutWriteBddviewCmd(ClientData clientData, Tcl_Interp *interp, int argc, const char **argv);

/* ************************************************ */
/* commands used to import files of various formats */
/* ************************************************ */

static int BddscoutReadBDDCmd(ClientData clientData, Tcl_Interp *interp, int argc, const char **argv);

static int BddscoutReadBFCmd(ClientData clientData, Tcl_Interp *interp, int argc, const char **argv);

/* ********************************************** */
/* commands used to parse strings from input line */
/* ********************************************** */

static int BddscoutParseInputInfixCmd(ClientData clientData, Tcl_Interp *interp, int argc, const char **argv);

/* ************************************* */
/* commands used to manipulate variables */
/* ************************************* */

static int BddscoutListVariablesByPositionCmd(ClientData clientData, Tcl_Interp *interp, int argc, const char **argv);

static int BddscoutListVariablesByNameCmd(ClientData clientData, Tcl_Interp *interp, int argc, const char **argv);

static int BddscoutListVariablesByOrderCmd(ClientData clientData, Tcl_Interp *interp, int argc, const char **argv);

static int BddscoutListVariablesByNumberCmd(ClientData clientData, Tcl_Interp *interp, int argc, const char **argv);

/* ************************************ */
/* commands used to manipulate formulae */
/* ************************************ */

static int BddscoutListFormulaeByNameCmd(ClientData clientData, Tcl_Interp *interp, int argc, const char **argv);

static int BddscoutListFormulaeByNodeNumberCmd(ClientData clientData, Tcl_Interp *interp, int argc, const char **argv);

static int BddscoutListFormulaeByNodeMaxLevelCmd(ClientData clientData, Tcl_Interp *interp, int argc, const char **argv);

static int BddscoutListFormulaeByNodeAvgLevelCmd(ClientData clientData, Tcl_Interp *interp, int argc, const char **argv);

static int BddscoutListFormulaeByPathNumberCmd(ClientData clientData, Tcl_Interp *interp, int argc, const char **argv);

static int BddscoutListFormulaeByMintermNumberCmd(ClientData clientData, Tcl_Interp *interp, int argc, const char **argv);

/* ****************************************************************** */
/* commands used to perform Biddy functions exported from biddyMain.c */
/* ****************************************************************** */

/* 7 Biddy_Managed_GetThen */
static int BiddyGetThenCmd(ClientData clientData, Tcl_Interp *interp, int argc, const char **argv);

/* 8 Biddy_Managed_GetElse */
static int BiddyGetElseCmd(ClientData clientData, Tcl_Interp *interp, int argc, const char **argv);

/* 31 Biddy_ResetVariablesValue */
static int BiddyResetVariablesValueCmd(ClientData clientData, Tcl_Interp *interp, int argc, const char **argv);

/* 32 Biddy_SetVariableValue */
static int BiddySetVariableValueCmd(ClientData clientData, Tcl_Interp *interp, int argc, const char **argv);

/* 33 Biddy_GetVariableValue */
static int BiddyGetVariableValueCmd(ClientData clientData, Tcl_Interp *interp, int argc, const char **argv);

/* 34 Biddy_ClearVariablesData */
static int BiddyClearVariablesDataCmd(ClientData clientData, Tcl_Interp *interp, int argc, const char **argv);

/* 35 Biddy_SetVariableData */
static int BiddySetVariableDataIntCmd(ClientData clientData, Tcl_Interp *interp, int argc, const char **argv);
static int BiddySetVariableDataFloatCmd(ClientData clientData, Tcl_Interp *interp, int argc, const char **argv);
static int BiddySetVariableDataStringCmd(ClientData clientData, Tcl_Interp *interp, int argc, const char **argv);

/* 36 Biddy_GetVariableData */
static int BiddyGetVariableDataIntCmd(ClientData clientData, Tcl_Interp *interp, int argc, const char **argv);
static int BiddyGetVariableDataFloatCmd(ClientData clientData, Tcl_Interp *interp, int argc, const char **argv);
static int BiddyGetVariableDataStringCmd(ClientData clientData, Tcl_Interp *interp, int argc, const char **argv);

/* 37 Biddy_Managed_Eval */
static int BiddyEvalCmd(ClientData clientData, Tcl_Interp *interp, int argc, const char **argv);

/* 38 Biddy_Managed_EvalProbability */
static int BiddyEvalProbabilityCmd(ClientData clientData, Tcl_Interp *interp, int argc, const char **argv);

/* 66 Biddy_Managed_SetAlphabeticOrdering */
static int BiddySetAlphabeticOrderingCmd(ClientData clientData, Tcl_Interp *interp, int argc, const char **argv);

/* 67 Biddy_Managed_SwapWithHigher */
static int BiddySwapWithHigherCmd(ClientData clientData, Tcl_Interp *interp, int argc, const char **argv);

/* 68 Biddy_Managed_SwapWithLower */
static int BiddySwapWithLowerCmd(ClientData clientData, Tcl_Interp *interp, int argc, const char **argv);

/* 69 Biddy_Managed_Sifting */
static int BiddySiftingCmd(ClientData clientData, Tcl_Interp *interp, int argc, const char **argv);
static int BiddySiftingOnFunctionCmd(ClientData clientData, Tcl_Interp *interp, int argc, const char **argv);

/* 70 Biddy_Managed_MinimizeBDD */
static int BiddyMinimizeBDDCmd(ClientData clientData, Tcl_Interp *interp, int argc, const char **argv);

/* 71 Biddy_Managed_MaximizeBDD */
static int BiddyMaximizeBDDCmd(ClientData clientData, Tcl_Interp *interp, int argc, const char **argv);

/* ****************************************************************** */
/* commands used to perform Biddy functions exported from biddyOp.c   */
/* ****************************************************************** */

/* 75 Biddy_Managed_Not */
static int BiddyNotCmd(ClientData clientData, Tcl_Interp *interp, int argc, const char **argv);

/* 76 Biddy_Managed_ITE */
static int BiddyITECmd(ClientData clientData, Tcl_Interp *interp, int argc, const char **argv);

/* 77 Biddy_Managed_And */
static int BiddyAndCmd(ClientData clientData, Tcl_Interp *interp, int argc, const char **argv);

/* 78 Biddy_Managed_Or */
static int BiddyOrCmd(ClientData clientData, Tcl_Interp *interp, int argc, const char **argv);

/* 79 Biddy_Managed_Nand */
static int BiddyNandCmd(ClientData clientData, Tcl_Interp *interp, int argc, const char **argv);

/* 80 Biddy_Managed_Nor */
static int BiddyNorCmd(ClientData clientData, Tcl_Interp *interp, int argc, const char **argv);

/* 81 Biddy_Managed_Xor */
static int BiddyXorCmd(ClientData clientData, Tcl_Interp *interp, int argc, const char **argv);

/* 82 Biddy_Managed_Xnor */
static int BiddyXnorCmd(ClientData clientData, Tcl_Interp *interp, int argc, const char **argv);

/* 83 Biddy_Managed_Leq */
static int BiddyLeqCmd(ClientData clientData, Tcl_Interp *interp, int argc, const char **argv);

/* 84 Biddy_Managed_Gt */
static int BiddyGtCmd(ClientData clientData, Tcl_Interp *interp, int argc, const char **argv);

/* 85 Biddy_Managed_IsLeq */
static int BiddyIsLeqCmd(ClientData clientData, Tcl_Interp *interp, int argc, const char **argv);

/* 86 Biddy_Managed_Restrict */
static int BiddyRestrictCmd(ClientData clientData, Tcl_Interp *interp, int argc, const char **argv);

/* 87 Biddy_Managed_Compose */
static int BiddyComposeCmd(ClientData clientData, Tcl_Interp *interp, int argc, const char **argv);

/* 88 Biddy_Managed_E */
static int BiddyECmd(ClientData clientData, Tcl_Interp *interp, int argc, const char **argv);

/* 89 Biddy_Managed_A */
static int BiddyACmd(ClientData clientData, Tcl_Interp *interp, int argc, const char **argv);

/* 90 Biddy_Managed_IsVariableDependent */
static int BiddyIsVariableDependentCmd(ClientData clientData, Tcl_Interp *interp, int argc, const char **argv);

/* 91 Biddy_Managed_ExistAbstract */
static int BiddyExistAbstractCmd(ClientData clientData, Tcl_Interp *interp, int argc, const char **argv);

/* 92 Biddy_Managed_UnivAbstract */
static int BiddyUnivAbstractCmd(ClientData clientData, Tcl_Interp *interp, int argc, const char **argv);

/* 93 Biddy_Managed_AndAbstract */
static int BiddyAndAbstractCmd(ClientData clientData, Tcl_Interp *interp, int argc, const char **argv);

/* 94 Biddy_Managed_Constrain */
static int BiddyConstrainCmd(ClientData clientData, Tcl_Interp *interp, int argc, const char **argv);

/* 95 Biddy_Managed_Simplify */
static int BiddySimplifyCmd(ClientData clientData, Tcl_Interp *interp, int argc, const char **argv);

/* 96 Biddy_Managed_Support */
static int BiddySupportCmd(ClientData clientData, Tcl_Interp *interp, int argc, const char **argv);

/* 97 Biddy_Managed_Replace */
static int BiddyReplaceCmd(ClientData clientData, Tcl_Interp *interp, int argc, const char **argv);

/* 98 Biddy_Managed_Change */
static int BiddyChangeCmd(ClientData clientData, Tcl_Interp *interp, int argc, const char **argv);

/* 99 Biddy_Managed_VarSubset */
static int BiddyVarSubsetCmd(ClientData clientData, Tcl_Interp *interp, int argc, const char **argv);

/* 100 Biddy_Managed_ElementAbstract */
/* static int BiddyElementAbstractCmd(ClientData clientData, Tcl_Interp *interp, int argc, const char **argv); */

/* 101 Biddy_Managed_Product */
/* static int BiddyProductCmd(ClientData clientData, Tcl_Interp *interp, int argc, const char **argv); */

/* 102 Biddy_Managed_SelectiveProduct */
/* static int BiddySelectiveProductCmd(ClientData clientData, Tcl_Interp *interp, int argc, const char **argv); */

/* 103 Biddy_Managed_Supset */
/* static int BiddySupsetCmd(ClientData clientData, Tcl_Interp *interp, int argc, const char **argv); */

/* 104 Biddy_Managed_Subset */
/* static int BiddySubsetCmd(ClientData clientData, Tcl_Interp *interp, int argc, const char **argv); */

/* 105 Biddy_Managed_Permitsym */
static int BiddyPermitsymCmd(ClientData clientData, Tcl_Interp *interp, int argc, const char **argv);

/* 106 Biddy_Managed_Stretch */
static int BiddyStretchCmd(ClientData clientData, Tcl_Interp *interp, int argc, const char **argv);

/* 107 Biddy_Managed_CreateMinterm */
static int BiddyCreateMintermCmd(ClientData clientData, Tcl_Interp *interp, int argc, const char **argv);

/* 108 Biddy_Managed_CreateFunction */
static int BiddyCreateFunctionCmd(ClientData clientData, Tcl_Interp *interp, int argc, const char **argv);

/* 109 Biddy_Managed_RandomFunction */
static int BiddyRandomFunctionCmd(ClientData clientData, Tcl_Interp *interp, int argc, const char **argv);

/* 110 Biddy_Managed_RandomSet */
static int BiddyRandomSetCmd(ClientData clientData, Tcl_Interp *interp, int argc, const char **argv);

/* 111 Biddy_Managed_ExtractMinterm */
/* static int BiddyExtractMintermCmd(ClientData clientData, Tcl_Interp *interp, int argc, const char **argv); */

/* ****************************************************************** */
/* commands used to perform Biddy functions exported from biddyStat.c */
/* ****************************************************************** */

/* 112 Biddy_Managed_CountNodes */
static int BiddyCountNodesCmd(ClientData clientData, Tcl_Interp *interp, int argc, const char **argv);

/* 113 Biddy_MaxLevel */
static int BiddyMaxLevelCmd(ClientData clientData, Tcl_Interp *interp, int argc, const char **argv);

/* 114 Biddy_AvgLevel */
static int BiddyAvgLevelCmd(ClientData clientData, Tcl_Interp *interp, int argc, const char **argv);

/* 115 Biddy_Managed_VariableTableNum */
static int BiddyVariableTableNumCmd(ClientData clientData, Tcl_Interp *interp, int argc, const char **argv);

/* 116 Biddy_Managed_NodeTableSize */
static int BiddyNodeTableSizeCmd(ClientData clientData, Tcl_Interp *interp, int argc, const char **argv);

/* 117 Biddy_Managed_NodeTableBlockNumber */
static int BiddyNodeTableBlockNumberCmd(ClientData clientData, Tcl_Interp *interp, int argc, const char **argv);

/* 118 Biddy_Managed_NodeTableGenerated */
static int BiddyNodeTableGeneratedCmd(ClientData clientData, Tcl_Interp *interp, int argc, const char **argv);

/* 119 Biddy_Managed_NodeTableMax */
static int BiddyNodeTableMaxCmd(ClientData clientData, Tcl_Interp *interp, int argc, const char **argv);

/* 120 Biddy_Managed_NodeTableNum */
static int BiddyNodeTableNumCmd(ClientData clientData, Tcl_Interp *interp, int argc, const char **argv);

/* 121 Biddy_Managed_NodeTableNumVar */
static int BiddyNodeTableNumVarCmd(ClientData clientData, Tcl_Interp *interp, int argc, const char **argv);

/* 122 Biddy_Managed_NodeTableResizeNumber */
static int BiddyNodeTableResizeNumberCmd(ClientData clientData, Tcl_Interp *interp, int argc, const char **argv);

/* 123 Biddy_Managed_NodeTableFoaNumber */
static int BiddyNodeTableFoaNumberCmd(ClientData clientData, Tcl_Interp *interp, int argc, const char **argv);

/* 124 Biddy_Managed_NodeTableFindNumber */
static int BiddyNodeTableFindNumberCmd(ClientData clientData, Tcl_Interp *interp, int argc, const char **argv);

/* 125 Biddy_Managed_NodeTableCompareNumber */
static int BiddyNodeTableCompareNumberCmd(ClientData clientData, Tcl_Interp *interp, int argc, const char **argv);

/* 126 Biddy_Managed_NodeTableAddNumber */
static int BiddyNodeTableAddNumberCmd(ClientData clientData, Tcl_Interp *interp, int argc, const char **argv);

/* 127 Biddy_Managed_NodeTableGCNumber */
static int BiddyNodeTableGCNumberCmd(ClientData clientData, Tcl_Interp *interp, int argc, const char **argv);

/* 128 Biddy_Managed_NodeTableGCTime */
static int BiddyNodeTableGCTimeCmd(ClientData clientData, Tcl_Interp *interp, int argc, const char **argv);

/* 129 Biddy_Managed_NodeTableGCObsoleteNumber */
static int BiddyNodeTableGCObsoleteNumberCmd(ClientData clientData, Tcl_Interp *interp, int argc, const char **argv);

/* 130 Biddy_Managed_NodeTableSwapNumber */
static int BiddyNodeTableSwapNumberCmd(ClientData clientData, Tcl_Interp *interp, int argc, const char **argv);

/* 131 Biddy_Managed_NodeTableSiftingNumber */
static int BiddyNodeTableSiftingNumberCmd(ClientData clientData, Tcl_Interp *interp, int argc, const char **argv);

/* 132 Biddy_Managed_NodeTableDRTime */
static int BiddyNodeTableDRTimeCmd(ClientData clientData, Tcl_Interp *interp, int argc, const char **argv);

/* 133 Biddy_Managed_NodeTableITENumber */
static int BiddyNodeTableITENumberCmd(ClientData clientData, Tcl_Interp *interp, int argc, const char **argv);

/* 134 Biddy_Managed_NodeTableITERecursiveNumber */
static int BiddyNodeTableITERecursiveNumberCmd(ClientData clientData, Tcl_Interp *interp, int argc, const char **argv);

/* 135 Biddy_Managed_NodeTableANDORNumber */
static int BiddyNodeTableANDORNumberCmd(ClientData clientData, Tcl_Interp *interp, int argc, const char **argv);

/* 136 Biddy_Managed_NodeTableANDORRecursiveNumber */
static int BiddyNodeTableANDORRecursiveNumberCmd(ClientData clientData, Tcl_Interp *interp, int argc, const char **argv);

/* 137 Biddy_Managed_NodeTableXORNumber */
static int BiddyNodeTableXORNumberCmd(ClientData clientData, Tcl_Interp *interp, int argc, const char **argv);

/* 138 Biddy_Managed_NodeTableXORRecursiveNumber */
static int BiddyNodeTableXORRecursiveNumberCmd(ClientData clientData, Tcl_Interp *interp, int argc, const char **argv);

/* 139 Biddy_Managed_FormulaTableNum */
static int BiddyFormulaTableNumCmd(ClientData clientData, Tcl_Interp *interp, int argc, const char **argv);

/* 140 Biddy_Managed_ListUsed */
static int BiddyListUsedCmd(ClientData clientData, Tcl_Interp *interp, int argc, const char **argv);

/* 141 Biddy_Managed_ListMaxLength */
static int BiddyListMaxLengthCmd(ClientData clientData, Tcl_Interp *interp, int argc, const char **argv);

/* 142 Biddy_Managed_ListAvgLength */
static int BiddyListAvgLengthCmd(ClientData clientData, Tcl_Interp *interp, int argc, const char **argv);

/* 143 Biddy_Managed_OPCacheSearch */
static int BiddyOPCacheSearchCmd(ClientData clientData, Tcl_Interp *interp, int argc, const char **argv);

/* 144 Biddy_Managed_OPCacheFind */
static int BiddyOPCacheFindCmd(ClientData clientData, Tcl_Interp *interp, int argc, const char **argv);

/* 145 Biddy_Managed_OPCacheInsert */
static int BiddyOPCacheInsertCmd(ClientData clientData, Tcl_Interp *interp, int argc, const char **argv);

/* 146 Biddy_Managed_OPCacheOverwrite */
static int BiddyOPCacheOverwriteCmd(ClientData clientData, Tcl_Interp *interp, int argc, const char **argv);

/* 147 Biddy_Managed_CountNodesPlain */
static int BiddyCountNodesPlainCmd(ClientData clientData, Tcl_Interp *interp, int argc, const char **argv);

/* 148 Biddy_Managed_DependentVariableNumber */
static int BiddyDependentVariableNumberCmd(ClientData clientData, Tcl_Interp *interp, int argc, const char **argv);

/* 149 Biddy_Managed_CountComplementedEdges */
static int BiddyCountComplementedEdgesCmd(ClientData clientData, Tcl_Interp *interp, int argc, const char **argv);

/* 150 Biddy_Managed_CountPaths */
static int BiddyCountPathsCmd(ClientData clientData, Tcl_Interp *interp, int argc, const char **argv);

/* 151 Biddy_Managed_CountMinterms */
static int BiddyCountMintermsCmd(ClientData clientData, Tcl_Interp *interp, int argc, const char **argv);
static int BiddyCountMintermsDomainCmd(ClientData clientData, Tcl_Interp *interp, int argc, const char **argv);

/* 152 Biddy_Managed_DensityOfFunction */
static int BiddyDensityOfFunctionCmd(ClientData clientData, Tcl_Interp *interp, int argc, const char **argv);

/* 153 Biddy_Managed_DensityOfBDD */
static int BiddyDensityOfBDDCmd(ClientData clientData, Tcl_Interp *interp, int argc, const char **argv);

/* 154 Biddy_MinNodes */
static int BiddyMinNodesCmd(ClientData clientData, Tcl_Interp *interp, int argc, const char **argv);

/* 155 Biddy_MaxNodes */
static int BiddyMaxNodesCmd(ClientData clientData, Tcl_Interp *interp, int argc, const char **argv);

/* 156 Biddy_Managed_ReadMemoryInUse */
static int BiddyReadMemoryInUseCmd(ClientData clientData, Tcl_Interp *interp, int argc, const char **argv);

/* 157 Biddy_Managed_PrintInfo */
static int BiddyPrintInfoCmd(ClientData clientData, Tcl_Interp *interp, int argc, const char **argv);

/* ******************************************************************* */
/* commands used to perform Biddy functions exported from biddyInOut.c */
/* ******************************************************************* */

/* 158 Biddy_Managed_Eval0 */
static int BiddyEval0Cmd(ClientData clientData, Tcl_Interp *interp, int argc, const char **argv);

/* 159 Biddy_Managed_Eval1 */
static int BiddyEval1Cmd(ClientData clientData, Tcl_Interp *interp, int argc, const char **argv);

/* 160 Biddy_Managed_Eval2 */
static int BiddyEval2Cmd(ClientData clientData, Tcl_Interp *interp, int argc, const char **argv);

/* 161 Biddy_Managed_ReadBddview */
static int BiddyReadBddviewCmd(ClientData clientData, Tcl_Interp *interp, int argc, const char **argv);

/* 162 Biddy_Managed_ReadVerilogFile */
static int BiddyReadVerilogFileCmd(ClientData clientData, Tcl_Interp *interp, int argc, const char **argv);

/* 163 Biddy_Managed_PrintBDD */
static int BiddyPrintBDDCmd(ClientData clientData, Tcl_Interp *interp, int argc, const char **argv);

/* 164 Biddy_Managed_PrintTable */
static int BiddyPrintTableCmd(ClientData clientData, Tcl_Interp *interp, int argc, const char **argv);

/* 165 Biddy_Managed_PrintSOP */
static int BiddyPrintSOPCmd(ClientData clientData, Tcl_Interp *interp, int argc, const char **argv);

/* 166 Biddy_Managed_PrintMinterms */
static int BiddyPrintMintermsCmd(ClientData clientData, Tcl_Interp *interp, int argc, const char **argv);

/* 167 Biddy_Managed_WriteDot */
static int BiddyWriteDotCmd(ClientData clientData, Tcl_Interp *interp, int argc, const char **argv);

/* 168 Biddy_Managed_WriteBddview */
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

  Tcl_CreateCommand(interp, "bddscout_aboutPkg", BddscoutAboutPkgCmd,
                     (ClientData) NULL, (Tcl_CmdDeleteProc *) NULL);

/* ***************** */
/* internal commands */
/* ***************** */

  Tcl_CreateCommand(interp, "bddscoutChangeType", BddscoutChangeTypeCmd,
                     (ClientData) NULL, (Tcl_CmdDeleteProc *) NULL);

  Tcl_CreateCommand(interp, "bddscoutCheckFormula", BddscoutCheckFormulaCmd,
                     (ClientData) NULL, (Tcl_CmdDeleteProc *) NULL);

  Tcl_CreateCommand(interp, "bddscoutCopyFormula", BddscoutCopyFormulaCmd,
                     (ClientData) NULL, (Tcl_CmdDeleteProc *) NULL);

  Tcl_CreateCommand(interp, "bddscoutConstruct", BddscoutConstructCmd,
                     (ClientData) NULL, (Tcl_CmdDeleteProc *) NULL);

  Tcl_CreateCommand(interp, "bddscoutWriteBddview", BddscoutWriteBddviewCmd,
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

/* ************************************* */
/* commands used to manipulate variables */
/* ************************************* */

  Tcl_CreateCommand(interp, "bddscout_list_variables_by_positon", BddscoutListVariablesByPositionCmd,
                     (ClientData) NULL, (Tcl_CmdDeleteProc *) NULL);

  Tcl_CreateCommand(interp, "bddscout_list_variables_by_name", BddscoutListVariablesByNameCmd,
                     (ClientData) NULL, (Tcl_CmdDeleteProc *) NULL);

  Tcl_CreateCommand(interp, "bddscout_list_variables_by_order", BddscoutListVariablesByOrderCmd,
                     (ClientData) NULL, (Tcl_CmdDeleteProc *) NULL);

  Tcl_CreateCommand(interp, "bddscout_list_variables_by_number", BddscoutListVariablesByNumberCmd,
                     (ClientData) NULL, (Tcl_CmdDeleteProc *) NULL);

/* ************************************ */
/* commands used to manipulate formulae */
/* ************************************ */

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

/* ****************************************************************** */
/* commands used to perform Biddy functions exported from biddyMain.c */
/* ****************************************************************** */

  Tcl_CreateCommand(interp, "biddy_get_then", BiddyGetThenCmd,
                     (ClientData) NULL, (Tcl_CmdDeleteProc *) NULL);

  Tcl_CreateCommand(interp, "biddy_get_else", BiddyGetElseCmd,
                     (ClientData) NULL, (Tcl_CmdDeleteProc *) NULL);

  Tcl_CreateCommand(interp, "biddy_reset_variables_value", BiddyResetVariablesValueCmd,
                     (ClientData) NULL, (Tcl_CmdDeleteProc *) NULL);

  Tcl_CreateCommand(interp, "biddy_set_variable_value", BiddySetVariableValueCmd,
                     (ClientData) NULL, (Tcl_CmdDeleteProc *) NULL);

  Tcl_CreateCommand(interp, "biddy_get_variable_value", BiddyGetVariableValueCmd,
                     (ClientData) NULL, (Tcl_CmdDeleteProc *) NULL);

  Tcl_CreateCommand(interp, "biddy_clear_variables_data", BiddyClearVariablesDataCmd,
                     (ClientData) NULL, (Tcl_CmdDeleteProc *) NULL);

  Tcl_CreateCommand(interp, "biddy_set_variable_data_int", BiddySetVariableDataIntCmd,
                     (ClientData) NULL, (Tcl_CmdDeleteProc *) NULL);

  Tcl_CreateCommand(interp, "biddy_set_variable_data_float", BiddySetVariableDataFloatCmd,
                     (ClientData) NULL, (Tcl_CmdDeleteProc *) NULL);

  Tcl_CreateCommand(interp, "biddy_set_variable_data_string", BiddySetVariableDataStringCmd,
                     (ClientData) NULL, (Tcl_CmdDeleteProc *) NULL);

  Tcl_CreateCommand(interp, "biddy_get_variable_data_int", BiddyGetVariableDataIntCmd,
                     (ClientData) NULL, (Tcl_CmdDeleteProc *) NULL);

  Tcl_CreateCommand(interp, "biddy_get_variable_data_float", BiddyGetVariableDataFloatCmd,
                     (ClientData) NULL, (Tcl_CmdDeleteProc *) NULL);

  Tcl_CreateCommand(interp, "biddy_get_variable_data_string", BiddyGetVariableDataStringCmd,
                     (ClientData) NULL, (Tcl_CmdDeleteProc *) NULL);

  Tcl_CreateCommand(interp, "biddy_eval", BiddyEvalCmd,
                     (ClientData) NULL, (Tcl_CmdDeleteProc *) NULL);

  Tcl_CreateCommand(interp, "biddy_eval_probability", BiddyEvalProbabilityCmd,
                     (ClientData) NULL, (Tcl_CmdDeleteProc *) NULL);

  Tcl_CreateCommand(interp, "biddy_set_alphabetic_ordering", BiddySetAlphabeticOrderingCmd,
                     (ClientData) NULL, (Tcl_CmdDeleteProc *) NULL);

  Tcl_CreateCommand(interp, "biddy_swap_with_higher", BiddySwapWithHigherCmd,
                     (ClientData) NULL, (Tcl_CmdDeleteProc *) NULL);

  Tcl_CreateCommand(interp, "biddy_swap_with_lower", BiddySwapWithLowerCmd,
                     (ClientData) NULL, (Tcl_CmdDeleteProc *) NULL);

  Tcl_CreateCommand(interp, "biddy_sifting", BiddySiftingCmd,
                     (ClientData) NULL, (Tcl_CmdDeleteProc *) NULL);

  Tcl_CreateCommand(interp, "biddy_sifting_on_function", BiddySiftingOnFunctionCmd,
                     (ClientData) NULL, (Tcl_CmdDeleteProc *) NULL);

  Tcl_CreateCommand(interp, "biddy_minimize", BiddyMinimizeBDDCmd,
                     (ClientData) NULL, (Tcl_CmdDeleteProc *) NULL);

  Tcl_CreateCommand(interp, "biddy_maximize", BiddyMaximizeBDDCmd,
                     (ClientData) NULL, (Tcl_CmdDeleteProc *) NULL);

/* ****************************************************************** */
/* commands used to perform Biddy functions exported from biddyOp.c */
/* ****************************************************************** */

  Tcl_CreateCommand(interp, "biddy_not", BiddyNotCmd,
                     (ClientData) NULL, (Tcl_CmdDeleteProc *) NULL);

  Tcl_CreateCommand(interp, "biddy_ite", BiddyITECmd,
                     (ClientData) NULL, (Tcl_CmdDeleteProc *) NULL);

  Tcl_CreateCommand(interp, "biddy_and", BiddyAndCmd,
                     (ClientData) NULL, (Tcl_CmdDeleteProc *) NULL);

  Tcl_CreateCommand(interp, "biddy_or", BiddyOrCmd,
                     (ClientData) NULL, (Tcl_CmdDeleteProc *) NULL);

  Tcl_CreateCommand(interp, "biddy_nand", BiddyNandCmd,
                     (ClientData) NULL, (Tcl_CmdDeleteProc *) NULL);

  Tcl_CreateCommand(interp, "biddy_nor", BiddyNorCmd,
                     (ClientData) NULL, (Tcl_CmdDeleteProc *) NULL);

  Tcl_CreateCommand(interp, "biddy_xor", BiddyXorCmd,
                     (ClientData) NULL, (Tcl_CmdDeleteProc *) NULL);

  Tcl_CreateCommand(interp, "biddy_xnor", BiddyXnorCmd,
                     (ClientData) NULL, (Tcl_CmdDeleteProc *) NULL);

  Tcl_CreateCommand(interp, "biddy_leq", BiddyLeqCmd,
                     (ClientData) NULL, (Tcl_CmdDeleteProc *) NULL);

  Tcl_CreateCommand(interp, "biddy_gt", BiddyGtCmd,
                     (ClientData) NULL, (Tcl_CmdDeleteProc *) NULL);

  Tcl_CreateCommand(interp, "biddy_is_leq", BiddyIsLeqCmd,
                     (ClientData) NULL, (Tcl_CmdDeleteProc *) NULL);

  Tcl_CreateCommand(interp, "biddy_restrict", BiddyRestrictCmd,
                     (ClientData) NULL, (Tcl_CmdDeleteProc *) NULL);

  Tcl_CreateCommand(interp, "biddy_compose", BiddyComposeCmd,
                     (ClientData) NULL, (Tcl_CmdDeleteProc *) NULL);

  Tcl_CreateCommand(interp, "biddy_e", BiddyECmd,
                     (ClientData) NULL, (Tcl_CmdDeleteProc *) NULL);

  Tcl_CreateCommand(interp, "biddy_a", BiddyACmd,
                     (ClientData) NULL, (Tcl_CmdDeleteProc *) NULL);

  Tcl_CreateCommand(interp, "biddy_is_variable_dependent", BiddyIsVariableDependentCmd,
                     (ClientData) NULL, (Tcl_CmdDeleteProc *) NULL);

  Tcl_CreateCommand(interp, "biddy_existabstract", BiddyExistAbstractCmd,
                     (ClientData) NULL, (Tcl_CmdDeleteProc *) NULL);

  Tcl_CreateCommand(interp, "biddy_univabstract", BiddyUnivAbstractCmd,
                     (ClientData) NULL, (Tcl_CmdDeleteProc *) NULL);

  Tcl_CreateCommand(interp, "biddy_andabstract", BiddyAndAbstractCmd,
                     (ClientData) NULL, (Tcl_CmdDeleteProc *) NULL);

  Tcl_CreateCommand(interp, "biddy_constrain", BiddyConstrainCmd,
                     (ClientData) NULL, (Tcl_CmdDeleteProc *) NULL);

  Tcl_CreateCommand(interp, "biddy_simplify", BiddySimplifyCmd,
                     (ClientData) NULL, (Tcl_CmdDeleteProc *) NULL);

  Tcl_CreateCommand(interp, "biddy_support", BiddySupportCmd,
                     (ClientData) NULL, (Tcl_CmdDeleteProc *) NULL);

  Tcl_CreateCommand(interp, "biddy_replace", BiddyReplaceCmd,
                     (ClientData) NULL, (Tcl_CmdDeleteProc *) NULL);

  Tcl_CreateCommand(interp, "biddy_change", BiddyChangeCmd,
                     (ClientData) NULL, (Tcl_CmdDeleteProc *) NULL);

  Tcl_CreateCommand(interp, "biddy_varsubset", BiddyVarSubsetCmd,
                     (ClientData) NULL, (Tcl_CmdDeleteProc *) NULL);

  Tcl_CreateCommand(interp, "biddy_permitsym", BiddyPermitsymCmd,
                     (ClientData) NULL, (Tcl_CmdDeleteProc *) NULL);

  Tcl_CreateCommand(interp, "biddy_create_minterm", BiddyCreateMintermCmd,
                     (ClientData) NULL, (Tcl_CmdDeleteProc *) NULL);

  Tcl_CreateCommand(interp, "biddy_create_function", BiddyCreateFunctionCmd,
                     (ClientData) NULL, (Tcl_CmdDeleteProc *) NULL);

  Tcl_CreateCommand(interp, "biddy_random_function", BiddyRandomFunctionCmd,
                     (ClientData) NULL, (Tcl_CmdDeleteProc *) NULL);

  Tcl_CreateCommand(interp, "biddy_random_set", BiddyRandomSetCmd,
                     (ClientData) NULL, (Tcl_CmdDeleteProc *) NULL);

/* ****************************************************************** */
/* commands used to perform Biddy functions exported from biddyStat.c */
/* ****************************************************************** */

  Tcl_CreateCommand(interp, "biddy_count_nodes", BiddyCountNodesCmd,
                     (ClientData) NULL, (Tcl_CmdDeleteProc *) NULL);

  Tcl_CreateCommand(interp, "biddy_max_level", BiddyMaxLevelCmd,
                     (ClientData) NULL, (Tcl_CmdDeleteProc *) NULL);

  Tcl_CreateCommand(interp, "biddy_avg_level", BiddyAvgLevelCmd,
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

  Tcl_CreateCommand(interp, "biddy_count_nodes_plain", BiddyCountNodesPlainCmd,
                     (ClientData) NULL, (Tcl_CmdDeleteProc *) NULL);

  Tcl_CreateCommand(interp, "biddy_dependent_variable_number", BiddyDependentVariableNumberCmd,
                     (ClientData) NULL, (Tcl_CmdDeleteProc *) NULL);

  Tcl_CreateCommand(interp, "biddy_count_complemented_edges", BiddyCountComplementedEdgesCmd,
                     (ClientData) NULL, (Tcl_CmdDeleteProc *) NULL);

  Tcl_CreateCommand(interp, "biddy_count_paths", BiddyCountPathsCmd,
                     (ClientData) NULL, (Tcl_CmdDeleteProc *) NULL);

  Tcl_CreateCommand(interp, "biddy_count_minterms", BiddyCountMintermsCmd,
                     (ClientData) NULL, (Tcl_CmdDeleteProc *) NULL);

  Tcl_CreateCommand(interp, "biddy_count_minterms_domain", BiddyCountMintermsDomainCmd,
                     (ClientData) NULL, (Tcl_CmdDeleteProc *) NULL);

  Tcl_CreateCommand(interp, "biddy_density_of_function", BiddyDensityOfFunctionCmd,
                     (ClientData) NULL, (Tcl_CmdDeleteProc *) NULL);

  Tcl_CreateCommand(interp, "biddy_density_of_bdd", BiddyDensityOfBDDCmd,
                     (ClientData) NULL, (Tcl_CmdDeleteProc *) NULL);

  Tcl_CreateCommand(interp, "biddy_min_nodes", BiddyMinNodesCmd,
                     (ClientData) NULL, (Tcl_CmdDeleteProc *) NULL);

  Tcl_CreateCommand(interp, "biddy_max_nodes", BiddyMaxNodesCmd,
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

  Tcl_CreateCommand(interp, "biddy_read_bddview", BiddyReadBddviewCmd,
                     (ClientData) NULL, (Tcl_CmdDeleteProc *) NULL);

  Tcl_CreateCommand(interp, "biddy_read_verilog_file", BiddyReadVerilogFileCmd,
                     (ClientData) NULL, (Tcl_CmdDeleteProc *) NULL);

  Tcl_CreateCommand(interp, "biddy_print_bdd", BiddyPrintBDDCmd,
                     (ClientData) NULL, (Tcl_CmdDeleteProc *) NULL);

  Tcl_CreateCommand(interp, "biddy_print_table", BiddyPrintTableCmd,
                     (ClientData) NULL, (Tcl_CmdDeleteProc *) NULL);

  Tcl_CreateCommand(interp, "biddy_print_sop", BiddyPrintSOPCmd,
                     (ClientData) NULL, (Tcl_CmdDeleteProc *) NULL);

  Tcl_CreateCommand(interp, "biddy_print_minterms", BiddyPrintMintermsCmd,
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
        BddscoutSyncFormula(name);
      }
    } else {
      /* this is used to setup variable ordering */
      bdd = Biddy_Managed_Eval2(MNG,form);
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
  Description [command used to manipulate variables]
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
  Description [command used to manipulate variables]
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
  Description [command used to manipulate variables]
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
  Description [command used to manipulate variables]
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
  Description [command used to manipulate formulae]
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
  Description [command used to manipulate formulae]
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
  Description [command used to manipulate formulae]
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
  Description [command used to manipulate formulae]
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
  Description [command used to manipulate formulae]
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
  Description [command used to manipulate formulae]
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
  Description []
  SideEffects [This is a command used to perform Biddy function exported
               from biddyMain.c]
  SeeAlso     []
  ************************************************************************/

static int
BiddyGetThenCmd(ClientData clientData, Tcl_Interp *interp, int argc,
                const char **argv)
{
  Biddy_Manager MNG;
  Biddy_String s1,s2;
  Biddy_Edge f;
  unsigned int idx;
  Biddy_String sup;

  if (argc != 3) {
    Tcl_SetResult(interp, (char *) "wrong # args", TCL_STATIC);
    return TCL_ERROR;
  }

  s1 = strdup(argv[1]); /* resulting function */
  s2 = strdup(argv[2]);

  MNG = Bddscout_GetActiveManager();

  /* TO DO: IT IS NOT CHECKED THAT s1 IS VALID AND NOT THE EXISTING VARIABLE NAME */
  sup = NULL;
  if (Biddy_Managed_FindFormula(MNG,s2,&idx,&f)) {
    if (!Biddy_IsTerminal(f)) {
      f = Biddy_Managed_GetThen(MNG,f);
      Biddy_Managed_AddPersistentFormula(MNG,s1,f);
      sup = strdup(s1); /* report the name of created formula */
    } else {
      sup = strdup("");
    }
  }

  free(s1);
  free(s2);

  Tcl_SetResult(interp, sup, TCL_VOLATILE);
  if (sup) free(sup);

  return TCL_OK;
}

/**Function****************************************************************
  Synopsis    []
  Description []
  SideEffects [This is a command used to perform Biddy function exported
               from biddyMain.c]
  SeeAlso     []
  ************************************************************************/

static int
BiddyGetElseCmd(ClientData clientData, Tcl_Interp *interp, int argc,
                const char **argv)
{
  Biddy_Manager MNG;
  Biddy_String s1,s2;
  Biddy_Edge f;
  unsigned int idx;
  Biddy_String sup;

  if (argc != 3) {
    Tcl_SetResult(interp, (char *) "wrong # args", TCL_STATIC);
    return TCL_ERROR;
  }

  s1 = strdup(argv[1]); /* resulting function */
  s2 = strdup(argv[2]);

  MNG = Bddscout_GetActiveManager();

  /* TO DO: IT IS NOT CHECKED THAT s1 IS VALID AND NOT THE EXISTING VARIABLE NAME */
  sup = NULL;
  if (Biddy_Managed_FindFormula(MNG,s2,&idx,&f)) {
    if (!Biddy_IsTerminal(f)) {
      f = Biddy_Managed_GetElse(MNG,f);
      Biddy_Managed_AddPersistentFormula(MNG,s1,f);
      sup = strdup(s1); /* report the name of created formula */
    } else {
      sup = strdup("");
    }
  }

  free(s1);
  free(s2);

  Tcl_SetResult(interp, sup, TCL_VOLATILE);
  if (sup) free(sup);

  return TCL_OK;
}

/**Function****************************************************************
  Synopsis    []
  Description []
  SideEffects [This is a command used to perform Biddy function exported
               from biddyMain.c]
  SeeAlso     []
  ************************************************************************/

static int
BiddyResetVariablesValueCmd(ClientData clientData, Tcl_Interp *interp,
             int argc, const char **argv)
{
  Biddy_Manager MNG;

  if (argc != 1) {
    Tcl_SetResult(interp, (char *) "wrong # args", TCL_STATIC);
    return TCL_ERROR;
  }

  MNG = Bddscout_GetActiveManager();

  Biddy_Managed_ResetVariablesValue(MNG);

  Tcl_SetResult(interp, (char *) "", TCL_STATIC);
  return TCL_OK;
}

/**Function****************************************************************
  Synopsis    []
  Description []
  SideEffects [This is a command used to perform Biddy function exported
               from biddyMain.c]
  SeeAlso     []
  ************************************************************************/

static int
BiddySetVariableValueCmd(ClientData clientData, Tcl_Interp *interp,
             int argc, const char **argv)
{
  Biddy_Manager MNG;
  Biddy_String s1,s2;
  Biddy_Variable v;
  Biddy_Boolean find;

  if (argc != 3) {
    Tcl_SetResult(interp, (char *) "wrong # args", TCL_STATIC);
    return TCL_ERROR;
  }

  s1 = strdup(argv[1]);
  s2 = strdup(argv[2]);

  MNG = Bddscout_GetActiveManager();

  v = 1;
  find = FALSE;
  while (!find && v<Biddy_Managed_VariableTableNum(MNG)) {
    if (!strcmp(s1,Biddy_Managed_GetVariableName(MNG,v))) {
      find = TRUE;
    } else {
      v++;
    }
  }

  /* IF THE ELEMENT WAS FOUND... */
  if (find) {

    /* DEBUGGING */
    /*
    printf("BiddySetVariableValueCmd: %s = %s\n",s1,s2);
    */

    if (!strcmp(s2,"1") ||
        !strcmp(s2,"true") || !strcmp(s2,"True") || !strcmp(s2,"TRUE"))
    {
      Biddy_Managed_SetVariableValue(MNG,v,Biddy_Managed_GetConstantOne(MNG));
    } else {
      Biddy_Managed_SetVariableValue(MNG,v,Biddy_Managed_GetConstantZero(MNG));
    }
    BddscoutSyncVariable(s1);
  }

  free(s1);
  free(s2);

  Tcl_SetResult(interp, (char *) "", TCL_STATIC);
  return TCL_OK;
}

/**Function****************************************************************
  Synopsis    []
  Description []
  SideEffects [This is a command used to perform Biddy function exported
               from biddyMain.c]
  SeeAlso     []
  ************************************************************************/

static int
BiddyGetVariableValueCmd(ClientData clientData, Tcl_Interp *interp,
             int argc, const char **argv)
{
  Biddy_Manager MNG;
  Biddy_String s1;
  Biddy_Variable v;
  Biddy_Boolean find;
  Biddy_String r;

  if (argc != 2) {
    Tcl_SetResult(interp, (char *) "wrong # args", TCL_STATIC);
    return TCL_ERROR;
  }

  s1 = strdup(argv[1]);

  MNG = Bddscout_GetActiveManager();

  v = 1;
  find = FALSE;
  while (!find && v<Biddy_Managed_VariableTableNum(MNG)) {
    if (!strcmp(s1,Biddy_Managed_GetVariableName(MNG,v))) {
      find = TRUE;
    } else {
      v++;
    }
  }

  /* IF THE ELEMENT WAS FOUND... */
  r = NULL;
  if (find) {
    if (Biddy_Managed_GetVariableValue(MNG,v) == Biddy_Managed_GetConstantZero(MNG)) {
      r = strdup("0");
    } else {
      r = strdup("1");
    }
  }

  free(s1);

  Tcl_SetResult(interp, r, TCL_VOLATILE);

  if (r) free(r);

  return TCL_OK;
}

/**Function****************************************************************
  Synopsis    []
  Description []
  SideEffects [This is a command used to perform Biddy function exported
               from biddyMain.c]
  SeeAlso     []
  ************************************************************************/

static int
BiddyClearVariablesDataCmd(ClientData clientData, Tcl_Interp *interp,
             int argc, const char **argv)
{
  Biddy_Manager MNG;

  if (argc != 1) {
    Tcl_SetResult(interp, (char *) "wrong # args", TCL_STATIC);
    return TCL_ERROR;
  }

  MNG = Bddscout_GetActiveManager();

  Biddy_Managed_ClearVariablesData(MNG);

  Tcl_SetResult(interp, (char *) "", TCL_STATIC);
  return TCL_OK;
}

/**Function****************************************************************
  Synopsis    []
  Description []
  SideEffects [This is a command used to perform Biddy function exported
               from biddyMain.c]
  SeeAlso     []
  ************************************************************************/

static int
BiddySetVariableDataIntCmd(ClientData clientData, Tcl_Interp *interp,
             int argc, const char **argv)
{
  Biddy_Manager MNG;
  Biddy_String s1,s2;
  Biddy_Variable v;
  Biddy_Boolean find;
  int value;
  int *data;

  if (argc != 3) {
    Tcl_SetResult(interp, (char *) "wrong # args", TCL_STATIC);
    return TCL_ERROR;
  }

  s1 = strdup(argv[1]);
  s2 = strdup(argv[2]);

  sscanf(s2,"%d",&value);

  MNG = Bddscout_GetActiveManager();

  v = 1;
  find = FALSE;
  while (!find && v<Biddy_Managed_VariableTableNum(MNG)) {
    if (!strcmp(s1,Biddy_Managed_GetVariableName(MNG,v))) {
      find = TRUE;
    } else {
      v++;
    }
  }

  /* IF THE ELEMENT WAS FOUND... */
  if (find) {
    if (value <= 0) {
      Biddy_Managed_SetVariableData(MNG,v,NULL);
    } else {
      data = (int *) malloc(sizeof(int));
      *data = value;
      Biddy_Managed_SetVariableData(MNG,v,(void *) data);
    }
    BddscoutSyncVariable(s1);
  }

  free(s1);
  free(s2);

  Tcl_SetResult(interp, (char *) "", TCL_STATIC);
  return TCL_OK;
}

/**Function****************************************************************
  Synopsis    []
  Description []
  SideEffects [This is a command used to perform Biddy function exported
               from biddyMain.c]
  SeeAlso     []
  ************************************************************************/

static int
BiddySetVariableDataFloatCmd(ClientData clientData, Tcl_Interp *interp,
             int argc, const char **argv)
{
  Biddy_Manager MNG;
  Biddy_String s1,s2;
  Biddy_Variable v;
  Biddy_Boolean find;
  double value;
  double *data;

  if (argc != 3) {
    Tcl_SetResult(interp, (char *) "wrong # args", TCL_STATIC);
    return TCL_ERROR;
  }

  s1 = strdup(argv[1]);
  s2 = strdup(argv[2]);

  sscanf(s2,"%lf",&value);

  MNG = Bddscout_GetActiveManager();

  v = 1;
  find = FALSE;
  while (!find && v<Biddy_Managed_VariableTableNum(MNG)) {
    if (!strcmp(s1,Biddy_Managed_GetVariableName(MNG,v))) {
      find = TRUE;
    } else {
      v++;
    }
  }

  /* IF THE ELEMENT WAS FOUND... */
  if (find) {
    if (value <= 0) {
      Biddy_Managed_SetVariableData(MNG,v,NULL);
    } else {
      data = (double *) malloc(sizeof(double));
      *data = value;
      Biddy_Managed_SetVariableData(MNG,v,(void *) data);
    }
    BddscoutSyncVariable(s1);
  }

  free(s1);
  free(s2);

  Tcl_SetResult(interp, (char *) "", TCL_STATIC);
  return TCL_OK;
}

/**Function****************************************************************
  Synopsis    []
  Description []
  SideEffects [This is a command used to perform Biddy function exported
               from biddyMain.c]
  SeeAlso     []
  ************************************************************************/

static int
BiddySetVariableDataStringCmd(ClientData clientData, Tcl_Interp *interp,
             int argc, const char **argv)
{
  Biddy_Manager MNG;
  Biddy_String s1,s2;
  Biddy_Variable v;
  Biddy_Boolean find;
  Biddy_String data;

  if (argc != 3) {
    Tcl_SetResult(interp, (char *) "wrong # args", TCL_STATIC);
    return TCL_ERROR;
  }

  s1 = strdup(argv[1]);
  s2 = strdup(argv[2]);
  if (!strcmp(s2,"")) s2 = NULL;

  MNG = Bddscout_GetActiveManager();

  v = 1;
  find = FALSE;
  while (!find && v<Biddy_Managed_VariableTableNum(MNG)) {
    if (!strcmp(s1,Biddy_Managed_GetVariableName(MNG,v))) {
      find = TRUE;
    } else {
      v++;
    }
  }

  /* IF THE ELEMENT WAS FOUND... */
  if (find) {
    data = strdup(s2);
    Biddy_Managed_SetVariableData(MNG,v,(void *) data);
    BddscoutSyncVariable(s1);
  }

  free(s1);
  free(s2);

  Tcl_SetResult(interp, (char *) "", TCL_STATIC);
  return TCL_OK;
}

/**Function****************************************************************
  Synopsis    []
  Description []
  SideEffects [This is a command used to perform Biddy function exported
               from biddyMain.c]
  SeeAlso     []
  ************************************************************************/

static int
BiddyGetVariableDataIntCmd(ClientData clientData, Tcl_Interp *interp,
                           int argc, const char **argv)
{
  Biddy_Manager MNG;
  Biddy_String s1;
  Biddy_Variable v;
  Biddy_Boolean find;
  int *data;
  Biddy_String value,r;

  if (argc != 2) {
    Tcl_SetResult(interp, (char *) "wrong # args", TCL_STATIC);
    return TCL_ERROR;
  }

  s1 = strdup(argv[1]);

  MNG = Bddscout_GetActiveManager();

  value = (Biddy_String) malloc(255);

  v = 1;
  find = FALSE;
  while (!find && v<Biddy_Managed_VariableTableNum(MNG)) {
    if (!strcmp(s1,Biddy_Managed_GetVariableName(MNG,v))) {
      find = TRUE;
    } else {
      v++;
    }
  }

  /* IF THE ELEMENT WAS FOUND... */
  if (find) {
    data = (int *) Biddy_Managed_GetVariableData(MNG,v);
    if (data) {
      sprintf(value,"%d",*data);
    } else {
      value[0] = 0;
    }
  } else {
    value[0] = 0;
  }
  r = strdup(value);

  free(s1);
  free(value);

  Tcl_SetResult(interp, r, TCL_VOLATILE);

  free(r);

  return TCL_OK;
}

/**Function****************************************************************
  Synopsis    []
  Description []
  SideEffects [This is a command used to perform Biddy function exported
               from biddyMain.c]
  SeeAlso     []
  ************************************************************************/

static int
BiddyGetVariableDataFloatCmd(ClientData clientData, Tcl_Interp *interp,
                             int argc, const char **argv)
{
  Biddy_Manager MNG;
  Biddy_String s1;
  Biddy_Variable v;
  Biddy_Boolean find;
  double *data;
  Biddy_String value,r;

  if (argc != 2) {
    Tcl_SetResult(interp, (char *) "wrong # args", TCL_STATIC);
    return TCL_ERROR;
  }

  s1 = strdup(argv[1]);

  MNG = Bddscout_GetActiveManager();

  value = (Biddy_String) malloc(255);

  v = 1;
  find = FALSE;
  while (!find && v<Biddy_Managed_VariableTableNum(MNG)) {
    if (!strcmp(s1,Biddy_Managed_GetVariableName(MNG,v))) {
      find = TRUE;
    } else {
      v++;
    }
  }

  /* IF THE ELEMENT WAS FOUND... */
  if (find) {
    data = (double *) Biddy_Managed_GetVariableData(MNG,v);
    if (data) {
      sprintf(value,"%.15lf",*data); /* double has 15 decimal digits of precision */
    } else {
      value[0] = 0;
    }
  } else {
    value[0] = 0;
  }
  r = strdup(value);

  free(s1);
  free(value);

  Tcl_SetResult(interp, r, TCL_VOLATILE);

  free(r);

  return TCL_OK;
}

/**Function****************************************************************
  Synopsis    []
  Description []
  SideEffects [This is a command used to perform Biddy function exported
               from biddyMain.c]
  SeeAlso     []
  ************************************************************************/

static int
BiddyGetVariableDataStringCmd(ClientData clientData, Tcl_Interp *interp,
                              int argc, const char **argv)
{
  Biddy_Manager MNG;
  Biddy_String s1;
  Biddy_Variable v;
  Biddy_Boolean find;
  Biddy_String value,r;

  if (argc != 2) {
    Tcl_SetResult(interp, (char *) "wrong # args", TCL_STATIC);
    return TCL_ERROR;
  }

  s1 = strdup(argv[1]);

  MNG = Bddscout_GetActiveManager();

  v = 1;
  find = FALSE;
  while (!find && v<Biddy_Managed_VariableTableNum(MNG)) {
    if (!strcmp(s1,Biddy_Managed_GetVariableName(MNG,v))) {
      find = TRUE;
    } else {
      v++;
    }
  }

  /* IF THE ELEMENT WAS FOUND... */
  if (find) {
    value = (Biddy_String) Biddy_Managed_GetVariableData(MNG,v);
    if (!value) {
      value = strdup("");
    }
  } else {
    value = strdup("");
  }
  r = strdup(value);

  free(s1);
  free(value);

  Tcl_SetResult(interp, r, TCL_VOLATILE);

  free(r);

  return TCL_OK;
}

/**Function****************************************************************
  Synopsis    []
  Description []
  SideEffects [This is a command used to perform Biddy function exported
               from biddyMain.c]
  SeeAlso     []
  ************************************************************************/

static int
BiddyEvalCmd(ClientData clientData, Tcl_Interp *interp,
             int argc, const char **argv)
{
  Biddy_Manager MNG;
  Biddy_String s1;
  Biddy_Edge f;
  unsigned int idx;
  Biddy_String sup;

  if (argc != 2) {
    Tcl_SetResult(interp, (char *) "wrong # args", TCL_STATIC);
    return TCL_ERROR;
  }

  s1 = strdup(argv[1]);
  sup = NULL;

  MNG = Bddscout_GetActiveManager();

  if (Biddy_Managed_FindFormula(MNG,s1,&idx,&f)) {
    if (Biddy_Managed_Eval(MNG,f)) {
      sup = strdup("1");
    } else {
      sup = strdup("0");
    }
  } else {
    sup = strdup("0");
  }

  free(s1);

  Tcl_SetResult(interp, sup, TCL_VOLATILE);
  if (sup) free(sup);

  return TCL_OK;
}

/**Function****************************************************************
  Synopsis    []
  Description []
  SideEffects [This is a command used to perform Biddy function exported
               from biddyMain.c]
  SeeAlso     []
  ************************************************************************/

static int
BiddyEvalProbabilityCmd(ClientData clientData, Tcl_Interp *interp,
             int argc, const char **argv)
{
  Biddy_Manager MNG;
  Biddy_String s1;
  Biddy_Edge f;
  unsigned int idx;
  Biddy_String sup;

  if (argc != 2) {
    Tcl_SetResult(interp, (char *) "wrong # args", TCL_STATIC);
    return TCL_ERROR;
  }

  s1 = strdup(argv[1]);
  sup = (Biddy_String) malloc(255);

  MNG = Bddscout_GetActiveManager();

  if (Biddy_Managed_FindFormula(MNG,s1,&idx,&f)) {
    sprintf(sup,"%.15lf",Biddy_Managed_EvalProbability(MNG,f)); /* double has 15 decimal digits of precision */
  } else {
    sprintf(sup,"%lf",-1.0);
  }

  Tcl_SetResult(interp, sup, TCL_VOLATILE);
  if (sup) free(sup);

  return TCL_OK;
}

/**Function****************************************************************
  Synopsis    []
  Description []
  SideEffects [This is a command used to perform Biddy function exported
               from biddyMain.c]
  SeeAlso     []
  ************************************************************************/

static int
BiddySetAlphabeticOrderingCmd(ClientData clientData, Tcl_Interp *interp,
                              int argc, const char **argv)
{
  Biddy_Manager MNG;

  if (argc != 1) {
    Tcl_SetResult(interp, (char *) "wrong # args", TCL_STATIC);
    return TCL_ERROR;
  }

  MNG = Bddscout_GetActiveManager();

  Biddy_Managed_SetAlphabeticOrdering(MNG);

  Tcl_SetResult(interp, (char *) "", TCL_STATIC);
  return TCL_OK;
}

/**Function****************************************************************
  Synopsis    []
  Description []
  SideEffects [This is a command used to perform Biddy function exported
               from biddyMain.c]
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

  /* IF THE ELEMENT WAS FOUND... */
  if (find) {
    Biddy_Managed_SwapWithHigher(MNG,v);
  }

  Tcl_SetResult(interp, (char *) "", TCL_STATIC);
  return TCL_OK;
}

/**Function****************************************************************
  Synopsis    []
  Description []
  SideEffects [This is a command used to perform Biddy function exported
               from biddyMain.c]
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

  /* IF THE ELEMENT WAS FOUND... */
  if (find) {
    Biddy_Managed_SwapWithLower(MNG,v);
  }

  Tcl_SetResult(interp, (char *) "", TCL_STATIC);
  return TCL_OK;
}

/**Function****************************************************************
  Synopsis    []
  Description []
  SideEffects [This is a command used to perform Biddy function exported
               from biddyMain.c]
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
  Description []
  SideEffects [This is a command used to perform Biddy function exported
               from biddyMain.c]
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
  Description []
  SideEffects [This is a command used to perform Biddy function exported
               from biddyMain.c]
  SeeAlso     []
  ************************************************************************/

static int
BiddyMinimizeBDDCmd(ClientData clientData, Tcl_Interp *interp, int argc,
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
    Biddy_Managed_MinimizeBDD(MNG,s1);
  }

  free(s1);

  Tcl_SetResult(interp, (char *) "", TCL_STATIC);
  return TCL_OK;
}

/**Function****************************************************************
  Synopsis    []
  Description []
  SideEffects [This is a command used to perform Biddy function exported
               from biddyMain.c]
  SeeAlso     []
  ************************************************************************/

static int
BiddyMaximizeBDDCmd(ClientData clientData, Tcl_Interp *interp, int argc,
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
    Biddy_Managed_MaximizeBDD(MNG,s1);
  }

  free(s1);

  Tcl_SetResult(interp, (char *) "", TCL_STATIC);
  return TCL_OK;
}

/**Function****************************************************************
  Synopsis    []
  Description [biddy_not(r,f) calculates r = NOT(f)]
  SideEffects [This is a command used to perform Biddy function exported
               from biddyOp.c]
  SeeAlso     []
  ************************************************************************/

static int
BiddyNotCmd(ClientData clientData, Tcl_Interp *interp, int argc,
            const char **argv)
{
  Biddy_Manager MNG;
  Biddy_String s1,s2;
  Biddy_Edge r,f;
  unsigned int idx;

  if (argc != 3) {
    Tcl_SetResult(interp, (char *) "wrong # args", TCL_STATIC);
    return TCL_ERROR;
  }

  s1 = strdup(argv[1]);
  s2 = strdup(argv[2]);

  MNG = Bddscout_GetActiveManager();

  if (Biddy_Managed_FindFormula(MNG,s2,&idx,&f))
  {
    r = Biddy_Managed_Not(MNG,f);
    Biddy_Managed_AddPersistentFormula(MNG,s1,r);
  }

  free(s1);
  free(s2);

  Tcl_SetResult(interp, (char *) "", TCL_STATIC);
  return TCL_OK;
}

/**Function****************************************************************
  Synopsis    []
  Description [biddy_ite(r,f,g,h) calculates r = ITE(f,g,h)]
  SideEffects [This is a command used to perform Biddy function exported
               from biddyOp.c]
  SeeAlso     []
  ************************************************************************/

static int
BiddyITECmd(ClientData clientData, Tcl_Interp *interp, int argc,
            const char **argv)
{
  Biddy_Manager MNG;
  Biddy_String s1,s2,s3,s4;
  Biddy_Edge r,f,g,h;
  unsigned int idx;

  if (argc != 5) {
    Tcl_SetResult(interp, (char *) "wrong # args", TCL_STATIC);
    return TCL_ERROR;
  }

  s1 = strdup(argv[1]);
  s2 = strdup(argv[2]);
  s3 = strdup(argv[3]);
  s4 = strdup(argv[4]);

  MNG = Bddscout_GetActiveManager();

  if (Biddy_Managed_FindFormula(MNG,s2,&idx,&f) &&
      Biddy_Managed_FindFormula(MNG,s3,&idx,&g) &&
      Biddy_Managed_FindFormula(MNG,s4,&idx,&h))
  {
    r = Biddy_Managed_ITE(MNG,f,g,h);
    Biddy_Managed_AddPersistentFormula(MNG,s1,r);
  }

  free(s1);
  free(s2);
  free(s3);
  free(s4);

  Tcl_SetResult(interp, (char *) "", TCL_STATIC);
  return TCL_OK;
}

/**Function****************************************************************
  Synopsis    []
  Description [biddy_and(r,f,g) calculates r = AND(f,g)]
  SideEffects [This is a command used to perform Biddy function exported
               from biddyOp.c]
  SeeAlso     []
  ************************************************************************/

static int
BiddyAndCmd(ClientData clientData, Tcl_Interp *interp, int argc,
            const char **argv)
{
  Biddy_Manager MNG;
  Biddy_String s1,s2,s3;
  Biddy_Edge r,f,g;
  unsigned int idx;

  if (argc != 4) {
    Tcl_SetResult(interp, (char *) "wrong # args", TCL_STATIC);
    return TCL_ERROR;
  }

  s1 = strdup(argv[1]);
  s2 = strdup(argv[2]);
  s3 = strdup(argv[3]);

  MNG = Bddscout_GetActiveManager();

  if (Biddy_Managed_FindFormula(MNG,s2,&idx,&f) &&
      Biddy_Managed_FindFormula(MNG,s3,&idx,&g))
  {
    r = Biddy_Managed_And(MNG,f,g);
    Biddy_Managed_AddPersistentFormula(MNG,s1,r);
  }

  free(s1);
  free(s2);
  free(s3);

  Tcl_SetResult(interp, (char *) "", TCL_STATIC);
  return TCL_OK;
}

/**Function****************************************************************
  Synopsis    []
  Description [biddy_or(r,f,g) calculates r = OR(f,g)]
  SideEffects [This is a command used to perform Biddy function exported
               from biddyOp.c]
  SeeAlso     []
  ************************************************************************/

static int
BiddyOrCmd(ClientData clientData, Tcl_Interp *interp, int argc,
           const char **argv)
{
  Biddy_Manager MNG;
  Biddy_String s1,s2,s3;
  Biddy_Edge r,f,g;
  unsigned int idx;

  if (argc != 4) {
    Tcl_SetResult(interp, (char *) "wrong # args", TCL_STATIC);
    return TCL_ERROR;
  }

  s1 = strdup(argv[1]);
  s2 = strdup(argv[2]);
  s3 = strdup(argv[3]);

  MNG = Bddscout_GetActiveManager();

  if (Biddy_Managed_FindFormula(MNG,s2,&idx,&f) &&
      Biddy_Managed_FindFormula(MNG,s3,&idx,&g))
  {
    r = Biddy_Managed_Or(MNG,f,g);
    Biddy_Managed_AddPersistentFormula(MNG,s1,r);
  }

  free(s1);
  free(s2);
  free(s3);

  Tcl_SetResult(interp, (char *) "", TCL_STATIC);
  return TCL_OK;
}

/**Function****************************************************************
  Synopsis    []
  Description [biddy_nand(r,f,g) calculates r = NAND(f,g)]
  SideEffects [This is a command used to perform Biddy function exported
               from biddyOp.c]
  SeeAlso     []
  ************************************************************************/

static int
BiddyNandCmd(ClientData clientData, Tcl_Interp *interp, int argc,
           const char **argv)
{
  Biddy_Manager MNG;
  Biddy_String s1,s2,s3;
  Biddy_Edge r,f,g;
  unsigned int idx;

  if (argc != 4) {
    Tcl_SetResult(interp, (char *) "wrong # args", TCL_STATIC);
    return TCL_ERROR;
  }

  s1 = strdup(argv[1]);
  s2 = strdup(argv[2]);
  s3 = strdup(argv[3]);

  MNG = Bddscout_GetActiveManager();

  if (Biddy_Managed_FindFormula(MNG,s2,&idx,&f) &&
      Biddy_Managed_FindFormula(MNG,s3,&idx,&g))
  {
    r = Biddy_Managed_Nand(MNG,f,g);
    Biddy_Managed_AddPersistentFormula(MNG,s1,r);
  }

  free(s1);
  free(s2);
  free(s3);

  Tcl_SetResult(interp, (char *) "", TCL_STATIC);
  return TCL_OK;
}

/**Function****************************************************************
  Synopsis    []
  Description [biddy_nor(r,f,g) calculates r = NOR(f,g)]
  SideEffects [This is a command used to perform Biddy function exported
               from biddyOp.c]
  SeeAlso     []
  ************************************************************************/

static int
BiddyNorCmd(ClientData clientData, Tcl_Interp *interp, int argc,
           const char **argv)
{
  Biddy_Manager MNG;
  Biddy_String s1,s2,s3;
  Biddy_Edge r,f,g;
  unsigned int idx;

  if (argc != 4) {
    Tcl_SetResult(interp, (char *) "wrong # args", TCL_STATIC);
    return TCL_ERROR;
  }

  s1 = strdup(argv[1]);
  s2 = strdup(argv[2]);
  s3 = strdup(argv[3]);

  MNG = Bddscout_GetActiveManager();

  if (Biddy_Managed_FindFormula(MNG,s2,&idx,&f) &&
      Biddy_Managed_FindFormula(MNG,s3,&idx,&g))
  {
    r = Biddy_Managed_Nor(MNG,f,g);
    Biddy_Managed_AddPersistentFormula(MNG,s1,r);
  }

  free(s1);
  free(s2);
  free(s3);

  Tcl_SetResult(interp, (char *) "", TCL_STATIC);
  return TCL_OK;
}

/**Function****************************************************************
  Synopsis    []
  Description [biddy_xor(r,f,g) calculates r = XOR(f,g)]
  SideEffects [This is a command used to perform Biddy function exported
               from biddyOp.c]
  SeeAlso     []
  ************************************************************************/

static int
BiddyXorCmd(ClientData clientData, Tcl_Interp *interp, int argc,
           const char **argv)
{
  Biddy_Manager MNG;
  Biddy_String s1,s2,s3;
  Biddy_Edge r,f,g;
  unsigned int idx;

  if (argc != 4) {
    Tcl_SetResult(interp, (char *) "wrong # args", TCL_STATIC);
    return TCL_ERROR;
  }

  s1 = strdup(argv[1]);
  s2 = strdup(argv[2]);
  s3 = strdup(argv[3]);

  MNG = Bddscout_GetActiveManager();

  if (Biddy_Managed_FindFormula(MNG,s2,&idx,&f) &&
      Biddy_Managed_FindFormula(MNG,s3,&idx,&g))
  {
    r = Biddy_Managed_Xor(MNG,f,g);
    Biddy_Managed_AddPersistentFormula(MNG,s1,r);
  }

  free(s1);
  free(s2);
  free(s3);

  Tcl_SetResult(interp, (char *) "", TCL_STATIC);
  return TCL_OK;
}

/**Function****************************************************************
  Synopsis    []
  Description [biddy_xnor(r,f,g) calculates r = XNOR(f,g)]
  SideEffects [This is a command used to perform Biddy function exported
               from biddyOp.c]
  SeeAlso     []
  ************************************************************************/

static int
BiddyXnorCmd(ClientData clientData, Tcl_Interp *interp, int argc,
           const char **argv)
{
  Biddy_Manager MNG;
  Biddy_String s1,s2,s3;
  Biddy_Edge r,f,g;
  unsigned int idx;

  if (argc != 4) {
    Tcl_SetResult(interp, (char *) "wrong # args", TCL_STATIC);
    return TCL_ERROR;
  }

  s1 = strdup(argv[1]);
  s2 = strdup(argv[2]);
  s3 = strdup(argv[3]);

  MNG = Bddscout_GetActiveManager();

  if (Biddy_Managed_FindFormula(MNG,s2,&idx,&f) &&
      Biddy_Managed_FindFormula(MNG,s3,&idx,&g))
  {
    r = Biddy_Managed_Xnor(MNG,f,g);
    Biddy_Managed_AddPersistentFormula(MNG,s1,r);
  }

  free(s1);
  free(s2);
  free(s3);

  Tcl_SetResult(interp, (char *) "", TCL_STATIC);
  return TCL_OK;
}

/**Function****************************************************************
  Synopsis    []
  Description [biddy_leq(r,f,g) calculates r = LEQ(f,g)]
  SideEffects [This is a command used to perform Biddy function exported
               from biddyOp.c]
  SeeAlso     []
  ************************************************************************/

static int
BiddyLeqCmd(ClientData clientData, Tcl_Interp *interp, int argc,
           const char **argv)
{
  Biddy_Manager MNG;
  Biddy_String s1,s2,s3;
  Biddy_Edge r,f,g;
  unsigned int idx;

  if (argc != 4) {
    Tcl_SetResult(interp, (char *) "wrong # args", TCL_STATIC);
    return TCL_ERROR;
  }

  s1 = strdup(argv[1]);
  s2 = strdup(argv[2]);
  s3 = strdup(argv[3]);

  MNG = Bddscout_GetActiveManager();

  if (Biddy_Managed_FindFormula(MNG,s2,&idx,&f) &&
      Biddy_Managed_FindFormula(MNG,s3,&idx,&g))
  {
    r = Biddy_Managed_Leq(MNG,f,g);
    Biddy_Managed_AddPersistentFormula(MNG,s1,r);
  }

  free(s1);
  free(s2);
  free(s3);

  Tcl_SetResult(interp, (char *) "", TCL_STATIC);
  return TCL_OK;
}

/**Function****************************************************************
  Synopsis    []
  Description [biddy_gt(r,f,g) calculates r = GT(f,g)]
  SideEffects [This is a command used to perform Biddy function exported
               from biddyOp.c]
  SeeAlso     []
  ************************************************************************/

static int
BiddyGtCmd(ClientData clientData, Tcl_Interp *interp, int argc,
           const char **argv)
{
  Biddy_Manager MNG;
  Biddy_String s1,s2,s3;
  Biddy_Edge r,f,g;
  unsigned int idx;

  if (argc != 4) {
    Tcl_SetResult(interp, (char *) "wrong # args", TCL_STATIC);
    return TCL_ERROR;
  }

  s1 = strdup(argv[1]);
  s2 = strdup(argv[2]);
  s3 = strdup(argv[3]);

  MNG = Bddscout_GetActiveManager();

  if (Biddy_Managed_FindFormula(MNG,s2,&idx,&f) &&
      Biddy_Managed_FindFormula(MNG,s3,&idx,&g))
  {
    r = Biddy_Managed_Gt(MNG,f,g);
    Biddy_Managed_AddPersistentFormula(MNG,s1,r);
  }

  free(s1);
  free(s2);
  free(s3);

  Tcl_SetResult(interp, (char *) "", TCL_STATIC);
  return TCL_OK;
}

/**Function****************************************************************
  Synopsis    []
  Description [biddy_is_leq(f,g) calculates isLeq(f,g)]
  SideEffects [This is a command used to perform Biddy function exported
               from biddyOp.c]
  SeeAlso     []
  ************************************************************************/

static int
BiddyIsLeqCmd(ClientData clientData, Tcl_Interp *interp, int argc,
           const char **argv)
{
  Biddy_Manager MNG;
  Biddy_String s1,s2;
  Biddy_Edge f,g;
  unsigned int idx;
  Biddy_String sup;

  if (argc != 3) {
    Tcl_SetResult(interp, (char *) "wrong # args", TCL_STATIC);
    return TCL_ERROR;
  }

  s1 = strdup(argv[1]);
  s2 = strdup(argv[2]);

  MNG = Bddscout_GetActiveManager();

  sup = NULL;
  if (Biddy_Managed_FindFormula(MNG,s1,&idx,&f) &&
      Biddy_Managed_FindFormula(MNG,s2,&idx,&g))
  {
    if (Biddy_Managed_IsLeq(MNG,f,g)) {
      sup = (Biddy_String) strdup("TRUE");
    } else {
      sup = (Biddy_String) strdup("FALSE");
    }
  }

  free(s1);
  free(s2);

  Tcl_SetResult(interp, sup, TCL_VOLATILE);
  if (sup) free(sup);

  return TCL_OK;
}

/**Function****************************************************************
  Synopsis    []
  Description [biddy_restrict(r,f,v,value) calculates
               r = Restrict(f,v,value)]
  SideEffects [This is a command used to perform Biddy function exported
               from biddyOp.c]
  SeeAlso     []
  ************************************************************************/

static int
BiddyRestrictCmd(ClientData clientData, Tcl_Interp *interp, int argc,
           const char **argv)
{
  Biddy_Manager MNG;
  Biddy_String s1,s2,s3,s4;
  Biddy_Edge r,f;
  unsigned int idx;
  Biddy_Variable v;
  unsigned int b;

  if (argc != 5) {
    Tcl_SetResult(interp, (char *) "wrong # args", TCL_STATIC);
    return TCL_ERROR;
  }

  s1 = strdup(argv[1]);
  s2 = strdup(argv[2]);
  s3 = strdup(argv[3]);
  s4 = strdup(argv[4]);

  MNG = Bddscout_GetActiveManager();

  v = Biddy_Managed_GetVariable(MNG,s3);
  sscanf(s4,"%u",&b);

  if (Biddy_Managed_FindFormula(MNG,s2,&idx,&f) &&
      (v != 0))
  {
    if (b == 0) {
      r = Biddy_Managed_Restrict(MNG,f,v,FALSE);
    } else {
      r = Biddy_Managed_Restrict(MNG,f,v,TRUE);
    }
    Biddy_Managed_AddPersistentFormula(MNG,s1,r);
  }

  free(s1);
  free(s2);
  free(s3);
  free(s4);

  Tcl_SetResult(interp, (char *) "", TCL_STATIC);
  return TCL_OK;
}

/**Function****************************************************************
  Synopsis    []
  Description [biddy_compose(r,f,g,v) calculates r = Compose(f,g,v)]
  SideEffects [This is a command used to perform Biddy function exported
               from biddyOp.c]
  SeeAlso     []
  ************************************************************************/

static int
BiddyComposeCmd(ClientData clientData, Tcl_Interp *interp, int argc,
           const char **argv)
{
  Biddy_Manager MNG;
  Biddy_String s1,s2,s3,s4;
  Biddy_Edge r,f,g;
  unsigned int idx;
  Biddy_Variable v;

  if (argc != 5) {
    Tcl_SetResult(interp, (char *) "wrong # args", TCL_STATIC);
    return TCL_ERROR;
  }

  s1 = strdup(argv[1]);
  s2 = strdup(argv[2]);
  s3 = strdup(argv[3]);
  s4 = strdup(argv[4]);

  MNG = Bddscout_GetActiveManager();

  v = Biddy_Managed_GetVariable(MNG,s4);

  if (Biddy_Managed_FindFormula(MNG,s2,&idx,&f) &&
      Biddy_Managed_FindFormula(MNG,s3,&idx,&g) &&
      (v != 0))
  {
    r = Biddy_Managed_Compose(MNG,f,g,v);
    Biddy_Managed_AddPersistentFormula(MNG,s1,r);
  }

  free(s1);
  free(s2);
  free(s3);
  free(s4);

  Tcl_SetResult(interp, (char *) "", TCL_STATIC);
  return TCL_OK;
}

/**Function****************************************************************
  Synopsis    []
  Description [biddy_e(r,f,v) calculates r = E(f,v)]
  SideEffects [This is a command used to perform Biddy function exported
               from biddyOp.c]
  SeeAlso     []
  ************************************************************************/

static int
BiddyECmd(ClientData clientData, Tcl_Interp *interp, int argc,
           const char **argv)
{
  Biddy_Manager MNG;
  Biddy_String s1,s2,s3;
  Biddy_Edge r,f;
  unsigned int idx;
  Biddy_Variable v;

  if (argc != 4) {
    Tcl_SetResult(interp, (char *) "wrong # args", TCL_STATIC);
    return TCL_ERROR;
  }

  s1 = strdup(argv[1]);
  s2 = strdup(argv[2]);
  s3 = strdup(argv[3]);

  MNG = Bddscout_GetActiveManager();

  v = Biddy_Managed_GetVariable(MNG,s3);

  if (Biddy_Managed_FindFormula(MNG,s2,&idx,&f) &&
      (v != 0))
  {
    r = Biddy_Managed_E(MNG,f,v);
    Biddy_Managed_AddPersistentFormula(MNG,s1,r);
  }

  free(s1);
  free(s2);
  free(s3);

  Tcl_SetResult(interp, (char *) "", TCL_STATIC);
  return TCL_OK;
}

/**Function****************************************************************
  Synopsis    []
  Description [biddy_a(r,f,v) calculates r = A(f,v)]
  SideEffects [This is a command used to perform Biddy function exported
               from biddyOp.c]
  SeeAlso     []
  ************************************************************************/

static int
BiddyACmd(ClientData clientData, Tcl_Interp *interp, int argc,
           const char **argv)
{
  Biddy_Manager MNG;
  Biddy_String s1,s2,s3;
  Biddy_Edge r,f;
  unsigned int idx;
  Biddy_Variable v;

  if (argc != 4) {
    Tcl_SetResult(interp, (char *) "wrong # args", TCL_STATIC);
    return TCL_ERROR;
  }

  s1 = strdup(argv[1]);
  s2 = strdup(argv[2]);
  s3 = strdup(argv[3]);

  MNG = Bddscout_GetActiveManager();

  v = Biddy_Managed_GetVariable(MNG,s3);

  if (Biddy_Managed_FindFormula(MNG,s2,&idx,&f) &&
      (v != 0))
  {
    r = Biddy_Managed_A(MNG,f,v);
    Biddy_Managed_AddPersistentFormula(MNG,s1,r);
  }

  free(s1);
  free(s2);
  free(s3);

  Tcl_SetResult(interp, (char *) "", TCL_STATIC);
  return TCL_OK;
}

/**Function****************************************************************
  Synopsis    []
  Description [biddy_is_variable_dependent(f,g) calculates
               IsVariableDependent(f,v)]
  SideEffects [This is a command used to perform Biddy function exported
               from biddyOp.c]
  SeeAlso     []
  ************************************************************************/

static int
BiddyIsVariableDependentCmd(ClientData clientData, Tcl_Interp *interp, int argc,
           const char **argv)
{
  Biddy_Manager MNG;
  Biddy_String s1,s2;
  Biddy_Edge f;
  unsigned int idx;
  Biddy_Variable v;
  Biddy_String sup;

  if (argc != 3) {
    Tcl_SetResult(interp, (char *) "wrong # args", TCL_STATIC);
    return TCL_ERROR;
  }

  s1 = strdup(argv[1]);
  s2 = strdup(argv[2]);

  MNG = Bddscout_GetActiveManager();

  v = Biddy_Managed_GetVariable(MNG,s2);

  sup = NULL;
  if (Biddy_Managed_FindFormula(MNG,s1,&idx,&f) && (v != 0)) {
    if (Biddy_Managed_IsVariableDependent(MNG,f,v)) {
      sup = (Biddy_String) strdup("TRUE");
    } else {
      sup = (Biddy_String) strdup("FALSE");
    }
  }

  free(s1);
  free(s2);

  Tcl_SetResult(interp, sup, TCL_VOLATILE);
  if (sup) free(sup);

  return TCL_OK;
}

/**Function****************************************************************
  Synopsis    []
  Description [biddy_existabstract(r,f,cube) calculates
               r = ExistAbstract(f,cube)]
  SideEffects [This is a command used to perform Biddy function exported
               from biddyOp.c]
  SeeAlso     []
  ************************************************************************/

static int
BiddyExistAbstractCmd(ClientData clientData, Tcl_Interp *interp, int argc,
           const char **argv)
{
  Biddy_Manager MNG;
  Biddy_String s1,s2,s3;
  Biddy_Edge r,f,cube;
  unsigned int idx;

  if (argc != 4) {
    Tcl_SetResult(interp, (char *) "wrong # args", TCL_STATIC);
    return TCL_ERROR;
  }

  s1 = strdup(argv[1]);
  s2 = strdup(argv[2]);
  s3 = strdup(argv[3]);

  MNG = Bddscout_GetActiveManager();

  if (Biddy_Managed_FindFormula(MNG,s2,&idx,&f) &&
      Biddy_Managed_FindFormula(MNG,s3,&idx,&cube))
  {
    r = Biddy_Managed_ExistAbstract(MNG,f,cube);
    Biddy_Managed_AddPersistentFormula(MNG,s1,r);
  }

  free(s1);
  free(s2);
  free(s3);

  Tcl_SetResult(interp, (char *) "", TCL_STATIC);
  return TCL_OK;
}

/**Function****************************************************************
  Synopsis    []
  Description [biddy_univabstract(r,f,cube) calculates
               r = UnivAbstract(f,cube)]
  SideEffects [This is a command used to perform Biddy function exported
               from biddyOp.c]
  SeeAlso     []
  ************************************************************************/

static int
BiddyUnivAbstractCmd(ClientData clientData, Tcl_Interp *interp, int argc,
           const char **argv)
{
  Biddy_Manager MNG;
  Biddy_String s1,s2,s3;
  Biddy_Edge r,f,cube;
  unsigned int idx;

  if (argc != 4) {
    Tcl_SetResult(interp, (char *) "wrong # args", TCL_STATIC);
    return TCL_ERROR;
  }

  s1 = strdup(argv[1]);
  s2 = strdup(argv[2]);
  s3 = strdup(argv[3]);

  MNG = Bddscout_GetActiveManager();

  if (Biddy_Managed_FindFormula(MNG,s2,&idx,&f) &&
      Biddy_Managed_FindFormula(MNG,s3,&idx,&cube))
  {
    r = Biddy_Managed_UnivAbstract(MNG,f,cube);
    Biddy_Managed_AddPersistentFormula(MNG,s1,r);
  }

  free(s1);
  free(s2);
  free(s3);

  Tcl_SetResult(interp, (char *) "", TCL_STATIC);
  return TCL_OK;
}

/**Function****************************************************************
  Synopsis    []
  Description [biddy_andabstract(r,f,g,cube) calculates
               r = AndAbstract(f,g,cube)]
  SideEffects [This is a command used to perform Biddy function exported
               from biddyOp.c]
  SeeAlso     []
  ************************************************************************/

static int
BiddyAndAbstractCmd(ClientData clientData, Tcl_Interp *interp, int argc,
           const char **argv)
{
  Biddy_Manager MNG;
  Biddy_String s1,s2,s3,s4;
  Biddy_Edge r,f,g,cube;
  unsigned int idx;

  if (argc != 5) {
    Tcl_SetResult(interp, (char *) "wrong # args", TCL_STATIC);
    return TCL_ERROR;
  }

  s1 = strdup(argv[1]);
  s2 = strdup(argv[2]);
  s3 = strdup(argv[3]);
  s4 = strdup(argv[4]);

  MNG = Bddscout_GetActiveManager();

  if (Biddy_Managed_FindFormula(MNG,s2,&idx,&f) &&
      Biddy_Managed_FindFormula(MNG,s3,&idx,&g) &&
      Biddy_Managed_FindFormula(MNG,s4,&idx,&cube))
  {
    r = Biddy_Managed_AndAbstract(MNG,f,g,cube);
    Biddy_Managed_AddPersistentFormula(MNG,s1,r);
  }

  free(s1);
  free(s2);
  free(s3);
  free(s4);

  Tcl_SetResult(interp, (char *) "", TCL_STATIC);
  return TCL_OK;
}

/**Function****************************************************************
  Synopsis    []
  Description [biddy_constrain(r,f,c) calculates r = Constrain(f,c)]
  SideEffects [This is a command used to perform Biddy function exported
               from biddyOp.c]
  SeeAlso     []
  ************************************************************************/

static int
BiddyConstrainCmd(ClientData clientData, Tcl_Interp *interp, int argc,
           const char **argv)
{
  Biddy_Manager MNG;
  Biddy_String s1,s2,s3;
  Biddy_Edge r,f,c;
  unsigned int idx;

  if (argc != 4) {
    Tcl_SetResult(interp, (char *) "wrong # args", TCL_STATIC);
    return TCL_ERROR;
  }

  s1 = strdup(argv[1]);
  s2 = strdup(argv[2]);
  s3 = strdup(argv[3]);

  MNG = Bddscout_GetActiveManager();

  if (Biddy_Managed_FindFormula(MNG,s2,&idx,&f) &&
      Biddy_Managed_FindFormula(MNG,s3,&idx,&c))
  {
    r = Biddy_Managed_Constrain(MNG,f,c);
    Biddy_Managed_AddPersistentFormula(MNG,s1,r);
  }

  free(s1);
  free(s2);
  free(s3);

  Tcl_SetResult(interp, (char *) "", TCL_STATIC);
  return TCL_OK;
}

/**Function****************************************************************
  Synopsis    []
  Description [biddy_simplify(r,f,c) calculates r = Simplify(f,c)]
  SideEffects [This is a command used to perform Biddy function exported
               from biddyOp.c]
  SeeAlso     []
  ************************************************************************/

static int
BiddySimplifyCmd(ClientData clientData, Tcl_Interp *interp, int argc,
           const char **argv)
{
  Biddy_Manager MNG;
  Biddy_String s1,s2,s3;
  Biddy_Edge r,f,c;
  unsigned int idx;

  if (argc != 4) {
    Tcl_SetResult(interp, (char *) "wrong # args", TCL_STATIC);
    return TCL_ERROR;
  }

  s1 = strdup(argv[1]);
  s2 = strdup(argv[2]);
  s3 = strdup(argv[3]);

  MNG = Bddscout_GetActiveManager();

  if (Biddy_Managed_FindFormula(MNG,s2,&idx,&f) &&
      Biddy_Managed_FindFormula(MNG,s3,&idx,&c))
  {
    r = Biddy_Managed_Simplify(MNG,f,c);
    Biddy_Managed_AddPersistentFormula(MNG,s1,r);
  }

  free(s1);
  free(s2);
  free(s3);

  Tcl_SetResult(interp, (char *) "", TCL_STATIC);
  return TCL_OK;
}

/**Function****************************************************************
  Synopsis    []
  Description [biddy_support(r,f) calculates r = Support(f)]
  SideEffects [This is a command used to perform Biddy function exported
               from biddyOp.c]
  SeeAlso     []
  ************************************************************************/

static int
BiddySupportCmd(ClientData clientData, Tcl_Interp *interp, int argc,
            const char **argv)
{
  Biddy_Manager MNG;
  Biddy_String s1,s2;
  Biddy_Edge r,f;
  unsigned int idx;

  if (argc != 3) {
    Tcl_SetResult(interp, (char *) "wrong # args", TCL_STATIC);
    return TCL_ERROR;
  }

  s1 = strdup(argv[1]);
  s2 = strdup(argv[2]);

  MNG = Bddscout_GetActiveManager();

  if (Biddy_Managed_FindFormula(MNG,s2,&idx,&f))
  {
    r = Biddy_Managed_Support(MNG,f);
    Biddy_Managed_AddPersistentFormula(MNG,s1,r);
  }

  free(s1);
  free(s2);

  Tcl_SetResult(interp, (char *) "", TCL_STATIC);
  return TCL_OK;
}

/**Function****************************************************************
  Synopsis    []
  Description [biddy_replace(r,f) calculates r = Replace(f)]
  SideEffects [This is a command used to perform Biddy function exported
               from biddyOp.c]
  SeeAlso     []
  ************************************************************************/

static int
BiddyReplaceCmd(ClientData clientData, Tcl_Interp *interp, int argc,
            const char **argv)
{
  Biddy_Manager MNG;
  Biddy_String s1,s2;
  Biddy_Edge /*r,*/f;
  unsigned int idx;

  if (argc != 3) {
    Tcl_SetResult(interp, (char *) "wrong # args", TCL_STATIC);
    return TCL_ERROR;
  }

  s1 = strdup(argv[1]);
  s2 = strdup(argv[2]);

  MNG = Bddscout_GetActiveManager();

  if (Biddy_Managed_FindFormula(MNG,s2,&idx,&f))
  {
    /* NOT IMPLEMENTED, YET */
  }

  free(s1);
  free(s2);

  Tcl_SetResult(interp, (char *) "", TCL_STATIC);
  return TCL_OK;
}

/**Function****************************************************************
  Synopsis    []
  Description [biddy_change(r,f,v) calculates r = Change(f,v)]
  SideEffects [This is a command used to perform Biddy function exported
               from biddyOp.c]
  SeeAlso     []
  ************************************************************************/

static int
BiddyChangeCmd(ClientData clientData, Tcl_Interp *interp, int argc,
           const char **argv)
{
  Biddy_Manager MNG;
  Biddy_String s1,s2,s3;
  Biddy_Edge r,f;
  unsigned int idx;
  Biddy_Variable v;

  if (argc != 4) {
    Tcl_SetResult(interp, (char *) "wrong # args", TCL_STATIC);
    return TCL_ERROR;
  }

  s1 = strdup(argv[1]);
  s2 = strdup(argv[2]);
  s3 = strdup(argv[3]);

  MNG = Bddscout_GetActiveManager();

  v = Biddy_Managed_GetVariable(MNG,s3);

  if (Biddy_Managed_FindFormula(MNG,s2,&idx,&f) &&
      (v != 0))
  {
    r = Biddy_Managed_Change(MNG,f,v);
    Biddy_Managed_AddPersistentFormula(MNG,s1,r);
  }

  free(s1);
  free(s2);
  free(s3);

  Tcl_SetResult(interp, (char *) "", TCL_STATIC);
  return TCL_OK;
}

/**Function****************************************************************
  Synopsis    []
  Description [biddy_varsubset(r,f,v,value) calculates
               r = VarSubset(f,v,value)]
  SideEffects [This is a command used to perform Biddy function exported
               from biddyOp.c]
  SeeAlso     []
  ************************************************************************/

static int
BiddyVarSubsetCmd(ClientData clientData, Tcl_Interp *interp, int argc,
           const char **argv)
{
  Biddy_Manager MNG;
  Biddy_String s1,s2,s3,s4;
  Biddy_Edge r,f;
  unsigned int idx;
  Biddy_Variable v;
  unsigned int b;

  if (argc != 5) {
    Tcl_SetResult(interp, (char *) "wrong # args", TCL_STATIC);
    return TCL_ERROR;
  }

  s1 = strdup(argv[1]);
  s2 = strdup(argv[2]);
  s3 = strdup(argv[3]);
  s4 = strdup(argv[4]);

  MNG = Bddscout_GetActiveManager();

  v = Biddy_Managed_GetVariable(MNG,s3);
  sscanf(s4,"%u",&b);

  if (Biddy_Managed_FindFormula(MNG,s2,&idx,&f) &&
      (v != 0))
  {
    if (b == 0) {
      r = Biddy_Managed_VarSubset(MNG,f,v,FALSE);
    } else {
      r = Biddy_Managed_VarSubset(MNG,f,v,TRUE);
    }
    Biddy_Managed_AddPersistentFormula(MNG,s1,r);
  }

  free(s1);
  free(s2);
  free(s3);
  free(s4);

  Tcl_SetResult(interp, (char *) "", TCL_STATIC);
  return TCL_OK;
}

/**Function****************************************************************
  Synopsis    []
  Description [biddy_permitsym(r,f,n) calculates r = Permitsym(f,n)]
  SideEffects [This is a command used to perform Biddy function exported
               from biddyOp.c]
  SeeAlso     []
  ************************************************************************/

static int
BiddyPermitsymCmd(ClientData clientData, Tcl_Interp *interp, int argc,
                  const char **argv)
{
  Biddy_Manager MNG;
  Biddy_String s1,s2,s3;
  Biddy_Edge r,f;
  unsigned int idx;
  unsigned int n;

  if (argc != 4) {
    Tcl_SetResult(interp, (char *) "wrong # args", TCL_STATIC);
    return TCL_ERROR;
  }

  s1 = strdup(argv[1]);
  s2 = strdup(argv[2]);
  s3 = strdup(argv[3]);

  MNG = Bddscout_GetActiveManager();

  sscanf(s3,"%u",&n);

  if (Biddy_Managed_FindFormula(MNG,s2,&idx,&f))
  {
    r = Biddy_Managed_Permitsym(MNG,f,n);
    Biddy_Managed_AddPersistentFormula(MNG,s1,r);
  }

  free(s1);
  free(s2);
  free(s3);

  Tcl_SetResult(interp, (char *) "", TCL_STATIC);
  return TCL_OK;
}

/**Function****************************************************************
  Synopsis    []
  Description [biddy_create_minterm(r,support,x) creates
              r = Minterm(support,x)]
  SideEffects [This is a command used to perform Biddy function exported
               from biddyOp.c]
  SeeAlso     []
  ************************************************************************/

static int
BiddyCreateMintermCmd(ClientData clientData, Tcl_Interp *interp, int argc,
                      const char **argv)
{
  Biddy_Manager MNG;
  Biddy_String s1,s2,s3;
  Biddy_Edge r,support;
  unsigned int idx;
  long long unsigned int x;

  if (argc != 4) {
    Tcl_SetResult(interp, (char *) "wrong # args", TCL_STATIC);
    return TCL_ERROR;
  }

  s1 = strdup(argv[1]);
  s2 = strdup(argv[2]);
  s3 = strdup(argv[3]);

  MNG = Bddscout_GetActiveManager();

#ifdef MINGW
  sscanf(s3,"%I64u",&x);
#else
  sscanf(s3,"%llu",&x);
#endif

  if (Biddy_Managed_FindFormula(MNG,s2,&idx,&support))
  {
    r = Biddy_Managed_CreateMinterm(MNG,support,x);
    Biddy_Managed_AddPersistentFormula(MNG,s1,r);
  }

  free(s1);
  free(s2);
  free(s3);

  Tcl_SetResult(interp, (char *) "", TCL_STATIC);
  return TCL_OK;
}

/**Function****************************************************************
  Synopsis    []
  Description [biddy_create_function(r,support,x) creates
              r = Function(support,x)]
  SideEffects [This is a command used to perform Biddy function exported
               from biddyOp.c]
  SeeAlso     []
  ************************************************************************/

static int
BiddyCreateFunctionCmd(ClientData clientData, Tcl_Interp *interp, int argc,
                       const char **argv)
{
  Biddy_Manager MNG;
  Biddy_String s1,s2,s3;
  Biddy_Edge r,support;
  unsigned int idx;
  long long unsigned int x;

  if (argc != 4) {
    Tcl_SetResult(interp, (char *) "wrong # args", TCL_STATIC);
    return TCL_ERROR;
  }

  s1 = strdup(argv[1]);
  s2 = strdup(argv[2]);
  s3 = strdup(argv[3]);

  MNG = Bddscout_GetActiveManager();

#ifdef MINGW
  sscanf(s3,"%I64u",&x);
#else
  sscanf(s3,"%llu",&x);
#endif

  if (Biddy_Managed_FindFormula(MNG,s2,&idx,&support))
  {
    r = Biddy_Managed_CreateFunction(MNG,support,x);
    Biddy_Managed_AddPersistentFormula(MNG,s1,r);
  }

  free(s1);
  free(s2);
  free(s3);

  Tcl_SetResult(interp, (char *) "", TCL_STATIC);
  return TCL_OK;
}

/**Function****************************************************************
  Synopsis    []
  Description [biddy_random_function (r,support,x) calculates
              r = RandomFunction(support,x)]
  SideEffects [This is a command used to perform Biddy function exported
               from biddyOp.c]
  SeeAlso     []
  ************************************************************************/

static int
BiddyRandomFunctionCmd(ClientData clientData, Tcl_Interp *interp, int argc,
                       const char **argv)
{
  Biddy_Manager MNG;
  Biddy_String s1,s2,s3;
  Biddy_Edge r,support;
  unsigned int idx;
  double x;

  if (argc != 4) {
    Tcl_SetResult(interp, (char *) "wrong # args", TCL_STATIC);
    return TCL_ERROR;
  }

  s1 = strdup(argv[1]);
  s2 = strdup(argv[2]);
  s3 = strdup(argv[3]);

  MNG = Bddscout_GetActiveManager();

  sscanf(s3,"%lf",&x);

  if (Biddy_Managed_FindFormula(MNG,s2,&idx,&support))
  {
    r = Biddy_Managed_RandomFunction(MNG,support,x);
    Biddy_Managed_AddPersistentFormula(MNG,s1,r);
  }

  free(s1);
  free(s2);
  free(s3);

  Tcl_SetResult(interp, (char *) "", TCL_STATIC);
  return TCL_OK;
}

/**Function****************************************************************
  Synopsis    []
  Description [biddy_random_set(r,unit,x) calculates r = RandomSet(unit,x)]
  SideEffects [This is a command used to perform Biddy function exported
               from biddyOp.c]
  SeeAlso     []
  ************************************************************************/

static int
BiddyRandomSetCmd(ClientData clientData, Tcl_Interp *interp, int argc,
                const char **argv)
{
  Biddy_Manager MNG;
  Biddy_String s1,s2,s3;
  Biddy_Edge r,unit;
  unsigned int idx;
  double x;

  if (argc != 4) {
    Tcl_SetResult(interp, (char *) "wrong # args", TCL_STATIC);
    return TCL_ERROR;
  }

  s1 = strdup(argv[1]);
  s2 = strdup(argv[2]);
  s3 = strdup(argv[3]);

  MNG = Bddscout_GetActiveManager();

  sscanf(s3,"%lf",&x);

  if (Biddy_Managed_FindFormula(MNG,s2,&idx,&unit))
  {
    r = Biddy_Managed_RandomSet(MNG,unit,x);
    Biddy_Managed_AddPersistentFormula(MNG,s1,r);
  }

  free(s1);
  free(s2);
  free(s3);

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
BiddyCountNodesCmd(ClientData clientData, Tcl_Interp *interp, int argc,
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
    printf("BiddyCountNodesCmd: Function %s does not exists!\n",s1);
    sprintf(sup,"0");
  } else {
    sprintf(sup,"%u",Biddy_Managed_CountNodes(MNG,f));
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
BiddyMaxLevelCmd(ClientData clientData, Tcl_Interp *interp, int argc,
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
    printf("BiddyMaxLevelCmd: Function %s does not exists!\n",s1);
    sprintf(sup,"0");
  } else {
    sprintf(sup,"%u",Biddy_Managed_MaxLevel(MNG,f));
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
BiddyAvgLevelCmd(ClientData clientData, Tcl_Interp *interp, int argc,
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
    printf("BiddyAvgLevelCmd: Function %s does not exists!\n",s1);
    sprintf(sup,"0");
  } else {
    sprintf(sup,"%f",Biddy_Managed_AvgLevel(MNG,f));
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
BiddyCountNodesPlainCmd(ClientData clientData, Tcl_Interp *interp, int argc,
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
    printf("BiddyCountNodesPlainCmd: Function %s does not exists!\n",s1);
    sprintf(sup,"0");
  } else {
    sprintf(sup,"%u",Biddy_Managed_CountNodesPlain(MNG,f));
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
    sprintf(sup,"%u",Biddy_Managed_DependentVariableNumber(MNG,f,FALSE));
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
BiddyCountComplementedEdgesCmd(ClientData clientData, Tcl_Interp *interp, int argc,
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
    printf("BiddyCountComplementedEdgesCmd: Function %s does not exists!\n",s1);
    sprintf(sup,"0");
  } else {
    sprintf(sup,"%u",Biddy_Managed_CountComplementedEdges(MNG,f));
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
BiddyCountMintermsCmd(ClientData clientData, Tcl_Interp *interp, int argc,
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
    printf("BiddyCountMintermsCmd: Function %s does not exists!\n",s1);
    sprintf(sup,"0");
  } else {
    sprintf(sup,"%.0lf",Biddy_Managed_CountMinterms(MNG,f,0));
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
BiddyCountMintermsDomainCmd(ClientData clientData, Tcl_Interp *interp, int argc,
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
    printf("BiddyCountMintermsDomainCmd: Function %s does not exists!\n",s1);
    sprintf(sup,"0");
  } else {
    sprintf(sup,"%.0f",Biddy_Managed_CountMinterms(MNG,f,nvars));
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
BiddyDensityOfFunctionCmd(ClientData clientData, Tcl_Interp *interp, int argc,
                  const char **argv)
{
  Biddy_Manager MNG;
  Biddy_String s1,s2;
  unsigned int idx,nvars;
  Biddy_Edge f;
  Biddy_String sup;

  if ((argc != 2) && (argc != 3)) {
    Tcl_SetResult(interp, (char *) "wrong # args", TCL_STATIC);
    return TCL_ERROR;
  }

  s1 = strdup(argv[1]);

  MNG = Bddscout_GetActiveManager();

  if (argc == 2) {
    nvars = 0;
  } else {
    s2 = strdup(argv[2]);
    nvars = atoi(s2);
    free(s2);
  }

  sup = (Biddy_String) malloc(127);
  if (!sup) return TCL_ERROR;

  if (!Biddy_Managed_FindFormula(MNG,s1,&idx,&f)) {
    printf("BiddyDensityOfFunctionCmd: Function %s does not exists!\n",s1);
    sprintf(sup,"0");
  } else {
    sprintf(sup,"%.2f",Biddy_Managed_DensityOfFunction(MNG,f,nvars));
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
BiddyDensityOfBDDCmd(ClientData clientData, Tcl_Interp *interp, int argc,
                  const char **argv)
{
  Biddy_Manager MNG;
  Biddy_String s1,s2;
  unsigned int idx,nvars;
  Biddy_Edge f;
  Biddy_String sup;

  if ((argc != 2) && (argc != 3)) {
    Tcl_SetResult(interp, (char *) "wrong # args", TCL_STATIC);
    return TCL_ERROR;
  }

  s1 = strdup(argv[1]);

  MNG = Bddscout_GetActiveManager();

  if (argc == 2) {
    nvars = 0;
  } else {
    s2 = strdup(argv[2]);
    nvars = atoi(s2);
    free(s2);
  }

  sup = (Biddy_String) malloc(127);
  if (!sup) return TCL_ERROR;

  if (!Biddy_Managed_FindFormula(MNG,s1,&idx,&f)) {
    printf("BiddyDensityOfBDDCmd: Function %s does not exists!\n",s1);
    sprintf(sup,"0");
  } else {
    sprintf(sup,"%.2f",Biddy_Managed_DensityOfBDD(MNG,f,nvars));
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
BiddyMinNodesCmd(ClientData clientData, Tcl_Interp *interp, int argc,
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
    printf("BiddyMinNodesCmd: Function %s does not exists!\n",s1);
    sprintf(sup,"0");
  } else {
    sprintf(sup,"%u",Biddy_Managed_MinNodes(MNG,f));
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
BiddyMaxNodesCmd(ClientData clientData, Tcl_Interp *interp, int argc,
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
    printf("BiddyMaxNodesCmd: Function %s does not exists!\n",s1);
    sprintf(sup,"0");
  } else {
    sprintf(sup,"%u",Biddy_Managed_MaxNodes(MNG,f));
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
BiddyReadBddviewCmd(ClientData clientData, Tcl_Interp *interp, int argc,
                  const char **argv)
{
  Biddy_Manager MNG;
  Biddy_String s1;
  Biddy_String name;

  if (argc != 2) {
    Tcl_SetResult(interp, (char *) "wrong # args", TCL_STATIC);
    return TCL_ERROR;
  }

  s1 = strdup(argv[1]);

  MNG = Bddscout_GetActiveManager();

  name = (Biddy_String) malloc(127);
  if (!name) return TCL_ERROR;

  name = Biddy_Managed_ReadBddview(MNG,s1,NULL);
  /* ADAPT FORMULA WITH THE SAME NAME IN OTHER MANAGERS */
  BddscoutSyncFormula(name);

  free(s1);

  Tcl_SetResult(interp, name, TCL_VOLATILE);
  free(name);

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
BiddyPrintBDDCmd(ClientData clientData, Tcl_Interp *interp, int argc,
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
    printf("BddscoutPrintBDD: Function %s does not exists!\n",s1);
    Tcl_SetResult(interp, (char *) "", TCL_STATIC);
  } else {
    Biddy_String result = strdup("");
    Biddy_Managed_SprintfBDD(MNG,&result,f);
    free(s1);
    Tcl_SetResult(interp, result, TCL_VOLATILE);
    free(result);
  }

  return TCL_OK;
}

/**Function****************************************************************
  Synopsis    []
  Description [command used to perform Biddy functions exported from biddyInOut.c]
  SideEffects []
  SeeAlso     []
  ************************************************************************/

static int
BiddyPrintTableCmd(ClientData clientData, Tcl_Interp *interp, int argc,
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
    printf("BddscoutPrintTable: Function %s does not exists!\n",s1);
    Tcl_SetResult(interp, (char *) "", TCL_STATIC);
  } else {
    Biddy_String result = strdup("");
    Biddy_Managed_SprintfTable(MNG,&result,f);
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
BiddyPrintSOPCmd(ClientData clientData, Tcl_Interp *interp, int argc,
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
    printf("BddscoutPrintSOP: Function %s does not exists!\n",s1);
    Tcl_SetResult(interp, (char *) "", TCL_STATIC);
  } else {
    Biddy_String result = strdup("");
    Biddy_Managed_SprintfSOP(MNG,&result,f);
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
BiddyPrintMintermsCmd(ClientData clientData, Tcl_Interp *interp, int argc,
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
    printf("BddscoutPrintMinterms: Function %s does not exists!\n",s1);
    Tcl_SetResult(interp, (char *) "", TCL_STATIC);
  } else {
    Biddy_String result = strdup("");
    Biddy_Managed_SprintfMinterms(MNG,&result,f,TRUE);
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
  return Biddy_Managed_CountNodes(MNG,f);
}

static unsigned int
MyNodeMaxLevel(Biddy_Edge f)
{
  return Biddy_MaxLevel(f);
}

static float
MyNodeAvgLevel(Biddy_Edge f)
{
  return Biddy_AvgLevel(f);
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
  n = (float) Biddy_Managed_CountMinterms(MNG,f,0); /* returns double */
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

/**CHeaderFile*************************************************************
  PackageName [Bdd Scout]
  Synopsis    [Bdd Scout]

  FileName    [bddscoutDecls.h]
  Revision    [$Revision: 545 $]
  Date        [$Date: 2019-02-11 14:07:50 +0100 (pon, 11 feb 2019) $]
  Authors     [Robert Meolic (robert@meolic.com)]
  Description [Generated by genStubs.tcl, see http://wiki.tcl.tk/3358]
  SeeAlso     [bddscout.h, bddscout.decls, bddscoutStubInit.c, bddscoutStubLib.c]

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

typedef struct BddscoutStubs {
    int magic;
    const struct BddscoutStubHooks *hooks;

    void (*biddy_InitMNG) (Biddy_Manager *mng, int gddtype); /* 1 */
    void (*biddy_ExitMNG) (Biddy_Manager *mng); /* 2 */
    Biddy_String (*biddy_About) (); /* 3 */
    int (*biddy_Managed_GetManagerType) (Biddy_Manager MNG); /* 4 */
    Biddy_String (*biddy_Managed_GetManagerName) (Biddy_Manager MNG); /* 5 */
    void (*biddy_Managed_SetManagerParameters) (Biddy_Manager MNG, float gcr, float gcrF, float gcrX, float rr, float rrF, float rrX, float st, float cst); /* 6 */
    Biddy_Edge (*biddy_GetThen) (Biddy_Edge f); /* 7 */
    Biddy_Edge (*biddy_GetElse) (Biddy_Edge f); /* 8 */
    Biddy_Variable (*biddy_GetTopVariable) (Biddy_Edge f); /* 9 */
    Biddy_Boolean (*biddy_Managed_IsEqv) (Biddy_Manager MNG1, Biddy_Edge f1, Biddy_Manager MNG2, Biddy_Edge f2); /* 10 */
    void (*biddy_Managed_SelectNode) (Biddy_Manager MNG, Biddy_Edge f); /* 11 */
    void (*biddy_Managed_DeselectNode) (Biddy_Manager MNG, Biddy_Edge f); /* 12 */
    Biddy_Boolean (*biddy_Managed_IsSelected) (Biddy_Manager MNG, Biddy_Edge f); /* 13 */
    void (*biddy_Managed_SelectFunction) (Biddy_Manager MNG, Biddy_Edge f); /* 14 */
    void (*biddy_Managed_DeselectAll) (Biddy_Manager MNG); /* 15 */
    Biddy_Edge (*biddy_Managed_GetTerminal) (Biddy_Manager MNG); /* 16 */
    Biddy_Edge (*biddy_Managed_GetConstantZero) (Biddy_Manager MNG); /* 17 */
    Biddy_Edge (*biddy_Managed_GetConstantOne) (Biddy_Manager MNG); /* 18 */
    Biddy_Edge (*biddy_Managed_GetBaseSet) (Biddy_Manager MNG); /* 19 */
    Biddy_Variable (*biddy_Managed_GetVariable) (Biddy_Manager MNG, Biddy_String x); /* 20 */
    Biddy_Variable (*biddy_Managed_GetLowestVariable) (Biddy_Manager MNG); /* 21 */
    Biddy_Variable (*biddy_Managed_GetIthVariable) (Biddy_Manager MNG, Biddy_Variable i); /* 22 */
    Biddy_Variable (*biddy_Managed_GetPrevVariable) (Biddy_Manager MNG, Biddy_Variable v); /* 23 */
    Biddy_Variable (*biddy_Managed_GetNextVariable) (Biddy_Manager MNG, Biddy_Variable v); /* 24 */
    Biddy_Edge (*biddy_Managed_GetVariableEdge) (Biddy_Manager MNG, Biddy_Variable v); /* 25 */
    Biddy_Edge (*biddy_Managed_GetElementEdge) (Biddy_Manager MNG, Biddy_Variable v); /* 26 */
    Biddy_String (*biddy_Managed_GetVariableName) (Biddy_Manager MNG, Biddy_Variable v); /* 27 */
    Biddy_Edge (*biddy_Managed_GetTopVariableEdge) (Biddy_Manager MNG, Biddy_Edge f); /* 28 */
    Biddy_String (*biddy_Managed_GetTopVariableName) (Biddy_Manager MNG, Biddy_Edge f); /* 29 */
    char (*biddy_Managed_GetTopVariableChar) (Biddy_Manager MNG, Biddy_Edge f); /* 30 */
    void (*biddy_Managed_ResetVariablesValue) (Biddy_Manager MNG); /* 31 */
    void (*biddy_Managed_SetVariableValue) (Biddy_Manager MNG, Biddy_Variable v, Biddy_Edge f); /* 32 */
    Biddy_Edge (*biddy_Managed_GetVariableValue) (Biddy_Manager MNG, Biddy_Variable v); /* 33 */
    void (*biddy_Managed_ClearVariablesData) (Biddy_Manager MNG); /* 34 */
    void (*biddy_Managed_SetVariableData) (Biddy_Manager MNG, Biddy_Variable v, void *x); /* 35 */
    void* (*biddy_Managed_GetVariableData) (Biddy_Manager MNG, Biddy_Variable v); /* 36 */
    Biddy_Boolean (*biddy_Managed_Eval) (Biddy_Manager MNG, Biddy_Edge f); /* 37 */
    double (*biddy_Managed_EvalProbability) (Biddy_Manager MNG, Biddy_Edge f); /* 38 */
    Biddy_Boolean (*biddy_Managed_IsSmaller) (Biddy_Manager MNG, Biddy_Variable fv, Biddy_Variable gv); /* 39 */
    Biddy_Boolean (*biddy_Managed_IsLowest) (Biddy_Manager MNG, Biddy_Variable f); /* 40 */
    Biddy_Boolean (*biddy_Managed_IsHighest) (Biddy_Manager MNG, Biddy_Variable f); /* 41 */
    Biddy_Variable (*biddy_Managed_FoaVariable) (Biddy_Manager MNG, Biddy_String x, Biddy_Boolean varelem); /* 42 */
    void (*biddy_Managed_ChangeVariableName) (Biddy_Manager MNG, Biddy_Variable v, Biddy_String x); /* 43 */
    Biddy_Variable (*biddy_Managed_AddVariableByName) (Biddy_Manager MNG, Biddy_String x); /* 44 */
    Biddy_Variable (*biddy_Managed_AddElementByName) (Biddy_Manager MNG, Biddy_String x); /* 45 */
    Biddy_Edge (*biddy_Managed_AddVariableBelow) (Biddy_Manager MNG, Biddy_Variable v); /* 46 */
    Biddy_Edge (*biddy_Managed_AddVariableAbove) (Biddy_Manager MNG, Biddy_Variable v); /* 47 */
    Biddy_Edge (*biddy_Managed_TransferMark) (Biddy_Manager MNG, Biddy_Edge f, Biddy_Boolean mark, Biddy_Boolean leftright); /* 48 */
    Biddy_Edge (*biddy_Managed_IncTag) (Biddy_Manager MNG, Biddy_Edge f); /* 49 */
    Biddy_Edge (*biddy_Managed_TaggedFoaNode) (Biddy_Manager MNG, Biddy_Variable v, Biddy_Edge pf, Biddy_Edge pt, Biddy_Variable ptag, Biddy_Boolean garbageAllowed); /* 50 */
    Biddy_Boolean (*biddy_Managed_IsOK) (Biddy_Manager MNG, Biddy_Edge f); /* 51 */
    void (*biddy_Managed_GC) (Biddy_Manager MNG, Biddy_Variable targetLT, Biddy_Variable targetGEQ, Biddy_Boolean purge, Biddy_Boolean total); /* 52 */
    void (*biddy_Managed_Clean) (Biddy_Manager MNG); /* 53 */
    void (*biddy_Managed_Purge) (Biddy_Manager MNG); /* 54 */
    void (*biddy_Managed_PurgeAndReorder) (Biddy_Manager MNG, Biddy_Edge f, Biddy_Boolean converge); /* 55 */
    void (*biddy_Managed_Refresh) (Biddy_Manager MNG, Biddy_Edge f); /* 56 */
    void (*biddy_Managed_AddCache) (Biddy_Manager MNG, Biddy_GCFunction gc); /* 57 */
    unsigned int (*biddy_Managed_AddFormula) (Biddy_Manager MNG, Biddy_String x, Biddy_Edge f, int count); /* 58 */
    Biddy_Boolean (*biddy_Managed_FindFormula) (Biddy_Manager MNG, Biddy_String x, unsigned int *idx, Biddy_Edge *f); /* 59 */
    Biddy_Boolean (*biddy_Managed_DeleteFormula) (Biddy_Manager MNG, Biddy_String x); /* 60 */
    Biddy_Boolean (*biddy_Managed_DeleteIthFormula) (Biddy_Manager MNG, unsigned int i); /* 61 */
    Biddy_Edge (*biddy_Managed_GetIthFormula) (Biddy_Manager MNG, unsigned int i); /* 62 */
    Biddy_String (*biddy_Managed_GetIthFormulaName) (Biddy_Manager MNG, unsigned int i); /* 63 */
    Biddy_String (*biddy_Managed_GetOrdering) (Biddy_Manager MNG); /* 64 */
    void (*biddy_Managed_SetOrdering) (Biddy_Manager MNG, Biddy_String ordering); /* 65 */
    void (*biddy_Managed_SetAlphabeticOrdering) (Biddy_Manager MNG); /* 66 */
    Biddy_Variable (*biddy_Managed_SwapWithHigher) (Biddy_Manager MNG, Biddy_Variable v); /* 67 */
    Biddy_Variable (*biddy_Managed_SwapWithLower) (Biddy_Manager MNG, Biddy_Variable v); /* 68 */
    Biddy_Boolean (*biddy_Managed_Sifting) (Biddy_Manager MNG, Biddy_Edge f, Biddy_Boolean converge); /* 69 */
    void (*biddy_Managed_MinimizeBDD) (Biddy_Manager MNG, Biddy_String name); /* 70 */
    void (*biddy_Managed_MaximizeBDD) (Biddy_Manager MNG, Biddy_String name); /* 71 */
    Biddy_Edge (*biddy_Managed_Copy) (Biddy_Manager MNG1, Biddy_Manager MNG2, Biddy_Edge f); /* 72 */
    void (*biddy_Managed_CopyFormula) (Biddy_Manager MNG1, Biddy_Manager MNG2, Biddy_String x); /* 73 */
    Biddy_Edge (*biddy_Managed_ConstructBDD) (Biddy_Manager MNG, int numV, Biddy_String varlist, int numN, Biddy_String nodelist); /* 74 */

    Biddy_Edge (*biddy_Managed_Not) (Biddy_Manager MNG, Biddy_Edge f); /* 75 */
    Biddy_Edge (*biddy_Managed_ITE) (Biddy_Manager MNG, Biddy_Edge f, Biddy_Edge g, Biddy_Edge h); /* 76 */
    Biddy_Edge (*biddy_Managed_And) (Biddy_Manager MNG, Biddy_Edge f, Biddy_Edge g); /* 77 */
    Biddy_Edge (*biddy_Managed_Or) (Biddy_Manager MNG, Biddy_Edge f, Biddy_Edge g); /* 78 */
    Biddy_Edge (*biddy_Managed_Nand) (Biddy_Manager MNG, Biddy_Edge f, Biddy_Edge g); /* 79 */
    Biddy_Edge (*biddy_Managed_Nor) (Biddy_Manager MNG, Biddy_Edge f, Biddy_Edge g); /* 80 */
    Biddy_Edge (*biddy_Managed_Xor) (Biddy_Manager MNG, Biddy_Edge f, Biddy_Edge g); /* 81 */
    Biddy_Edge (*biddy_Managed_Xnor) (Biddy_Manager MNG, Biddy_Edge f, Biddy_Edge g); /* 82 */
    Biddy_Edge (*biddy_Managed_Leq) (Biddy_Manager MNG, Biddy_Edge f, Biddy_Edge g); /* 83 */
    Biddy_Edge (*biddy_Managed_Gt) (Biddy_Manager MNG, Biddy_Edge f, Biddy_Edge g); /* 84 */
    Biddy_Boolean (*biddy_Managed_IsLeq) (Biddy_Manager MNG, Biddy_Edge f, Biddy_Edge g); /* 85 */
    Biddy_Edge (*biddy_Managed_Restrict) (Biddy_Manager MNG, Biddy_Edge f, Biddy_Variable v, Biddy_Boolean value); /* 86 */
    Biddy_Edge (*biddy_Managed_Compose) (Biddy_Manager MNG, Biddy_Edge f, Biddy_Edge g, Biddy_Variable v); /* 87 */
    Biddy_Edge (*biddy_Managed_E) (Biddy_Manager MNG, Biddy_Edge f, Biddy_Variable v); /* 88 */
    Biddy_Edge (*biddy_Managed_A) (Biddy_Manager MNG, Biddy_Edge f, Biddy_Variable v); /* 89 */
    Biddy_Boolean (*biddy_Managed_IsVariableDependent) (Biddy_Manager MNG, Biddy_Edge f, Biddy_Variable v); /* 90 */
    Biddy_Edge (*biddy_Managed_ExistAbstract) (Biddy_Manager MNG, Biddy_Edge f, Biddy_Edge cube); /* 91 */
    Biddy_Edge (*biddy_Managed_UnivAbstract) (Biddy_Manager MNG, Biddy_Edge f, Biddy_Edge cube); /* 92 */
    Biddy_Edge (*biddy_Managed_AndAbstract) (Biddy_Manager MNG, Biddy_Edge f, Biddy_Edge g, Biddy_Edge cube); /* 93 */
    Biddy_Edge (*biddy_Managed_Constrain) (Biddy_Manager MNG, Biddy_Edge f, Biddy_Edge c); /* 94 */
    Biddy_Edge (*biddy_Managed_Simplify) (Biddy_Manager MNG, Biddy_Edge f, Biddy_Edge c); /* 95 */
    Biddy_Edge (*biddy_Managed_Support) (Biddy_Manager MNG, Biddy_Edge f); /* 96 */
    Biddy_Edge (*biddy_Managed_ReplaceByKeyword) (Biddy_Manager MNG, Biddy_Edge f, Biddy_String keyword); /* 97 */
    Biddy_Edge (*biddy_Managed_Change) (Biddy_Manager MNG, Biddy_Edge f, Biddy_Variable v); /* 98 */
    Biddy_Edge (*biddy_Managed_VarSubset) (Biddy_Manager MNG, Biddy_Edge f, Biddy_Variable v, Biddy_Boolean value); /* 99 */
    Biddy_Edge (*biddy_Managed_ElementAbstract) (Biddy_Manager MNG, Biddy_Edge f, Biddy_Variable v); /* 100 ... */
    Biddy_Edge (*biddy_Managed_Product) (Biddy_Manager MNG, Biddy_Edge f, Biddy_Edge g); /* 100 */
    Biddy_Edge (*biddy_Managed_SelectiveProduct) (Biddy_Manager MNG, Biddy_Edge f, Biddy_Edge g, Biddy_Edge cube); /* 101 */
    Biddy_Edge (*biddy_Managed_Supset) (Biddy_Manager MNG, Biddy_Edge f, Biddy_Edge g); /* 102 */
    Biddy_Edge (*biddy_Managed_Subset) (Biddy_Manager MNG, Biddy_Edge f, Biddy_Edge g); /* 103 */
    Biddy_Edge (*biddy_Managed_Permitsym) (Biddy_Manager MNG, Biddy_Edge f, unsigned int n); /* 104 */
    Biddy_Edge (*biddy_Managed_Stretch) (Biddy_Manager MNG, Biddy_Edge f); /* 105 */
    Biddy_Edge (*biddy_Managed_CreateMinterm) (Biddy_Manager MNG, Biddy_Edge support, long long unsigned int x); /* 106 */
    Biddy_Edge (*biddy_Managed_CreateFunction) (Biddy_Manager MNG, Biddy_Edge support, long long unsigned int x); /* 107 */
    Biddy_Edge (*biddy_Managed_RandomFunction) (Biddy_Manager MNG, Biddy_Edge support, double r); /* 108 */
    Biddy_Edge (*biddy_Managed_RandomSet) (Biddy_Manager MNG, Biddy_Edge empty, double r); /* 109 */
    Biddy_Edge (*biddy_Managed_ExtractMinterm) (Biddy_Manager MNG, Biddy_Edge support, Biddy_Edge f); /* 110 */

    unsigned int (*biddy_Managed_CountNodes) (Biddy_Manager MNG, Biddy_Edge f); /* 111 */
    unsigned int (*biddy_MaxLevel) (Biddy_Edge f); /* 112 */
    float (*biddy_AvgLevel) (Biddy_Edge f); /* 113 */
    Biddy_Variable (*biddy_Managed_VariableTableNum) (Biddy_Manager MNG); /* 114 */
    unsigned int (*biddy_Managed_NodeTableSize) (Biddy_Manager MNG); /* 115 */
    unsigned int (*biddy_Managed_NodeTableBlockNumber) (Biddy_Manager MNG); /* 116 */
    unsigned int (*biddy_Managed_NodeTableGenerated) (Biddy_Manager MNG); /* 117 */
    unsigned int (*biddy_Managed_NodeTableMax) (Biddy_Manager MNG); /* 118 */
    unsigned int (*biddy_Managed_NodeTableNum) (Biddy_Manager MNG); /* 119 */
    unsigned int (*biddy_Managed_NodeTableNumVar) (Biddy_Manager MNG, Biddy_Variable v); /* 120 */
    unsigned int (*biddy_Managed_NodeTableResizeNumber) (Biddy_Manager MNG); /* 121 */
    unsigned long long int (*biddy_Managed_NodeTableFoaNumber) (Biddy_Manager MNG); /* 122 */
    unsigned long long int (*biddy_Managed_NodeTableFindNumber) (Biddy_Manager MNG); /* 123 */
    unsigned long long int (*biddy_Managed_NodeTableCompareNumber) (Biddy_Manager MNG); /* 124 */
    unsigned long long int (*biddy_Managed_NodeTableAddNumber) (Biddy_Manager MNG); /* 125 */
    unsigned int (*biddy_Managed_NodeTableGCNumber) (Biddy_Manager MNG); /* 126 */
    unsigned int (*biddy_Managed_NodeTableGCTime) (Biddy_Manager MNG); /* 127 */
    unsigned long long int (*biddy_Managed_NodeTableGCObsoleteNumber) (Biddy_Manager MNG); /* 128 */
    unsigned int (*biddy_Managed_NodeTableSwapNumber) (Biddy_Manager MNG); /* 129 */
    unsigned int (*biddy_Managed_NodeTableSiftingNumber) (Biddy_Manager MNG); /* 130 */
    unsigned int (*biddy_Managed_NodeTableDRTime) (Biddy_Manager MNG); /* 131 */
    unsigned int (*biddy_Managed_NodeTableITENumber) (Biddy_Manager MNG); /* 132 */
    unsigned long long int (*biddy_Managed_NodeTableITERecursiveNumber) (Biddy_Manager MNG); /* 133 */
    unsigned int (*biddy_Managed_NodeTableANDORNumber) (Biddy_Manager MNG); /* 134 */
    unsigned long long int (*biddy_Managed_NodeTableANDORRecursiveNumber) (Biddy_Manager MNG); /* 135 */
    unsigned int (*biddy_Managed_NodeTableXORNumber) (Biddy_Manager MNG); /* 136 */
    unsigned long long int (*biddy_Managed_NodeTableXORRecursiveNumber) (Biddy_Manager MNG); /* 137 */
    unsigned int (*biddy_Managed_FormulaTableNum) (Biddy_Manager MNG); /* 138 */
    unsigned int (*biddy_Managed_ListUsed) (Biddy_Manager MNG); /* 139 */
    unsigned int (*biddy_Managed_ListMaxLength) (Biddy_Manager MNG); /* 140 */
    float (*biddy_Managed_ListAvgLength) (Biddy_Manager MNG); /* 141 */
    unsigned long long int (*biddy_Managed_OPCacheSearch) (Biddy_Manager MNG); /* 142 */
    unsigned long long int (*biddy_Managed_OPCacheFind) (Biddy_Manager MNG); /* 143 */
    unsigned long long int (*biddy_Managed_OPCacheInsert) (Biddy_Manager MNG); /* 144 */
    unsigned long long int (*biddy_Managed_OPCacheOverwrite) (Biddy_Manager MNG); /* 145 */
    unsigned int (*biddy_Managed_CountNodesPlain) (Biddy_Manager MNG, Biddy_Edge f); /* 146 */
    unsigned int (*biddy_Managed_DependentVariableNumber) (Biddy_Manager MNG, Biddy_Edge f, Biddy_Boolean select); /* 147 */
    unsigned int (*biddy_Managed_CountComplementedEdges) (Biddy_Manager MNG, Biddy_Edge f); /* 148 */
    unsigned long long int (*biddy_Managed_CountPaths) (Biddy_Manager MNG, Biddy_Edge f); /* 149 */
    double (*biddy_Managed_CountMinterms) (Biddy_Manager MNG, Biddy_Edge f, unsigned int nvars); /* 150 */
    double (*biddy_Managed_DensityOfFunction) (Biddy_Manager MNG, Biddy_Edge f, unsigned int nvars); /* 151 */
    double (*biddy_Managed_DensityOfBDD) (Biddy_Manager MNG, Biddy_Edge f, unsigned int nvars); /* 152 */
    unsigned int (*biddy_Managed_MinNodes) (Biddy_Manager MNG, Biddy_Edge f); /* 153 */
    unsigned int (*biddy_Managed_MaxNodes) (Biddy_Manager MNG, Biddy_Edge f); /* 154 */
    unsigned long long int (*biddy_Managed_ReadMemoryInUse) (Biddy_Manager MNG); /* 155 */
    void (*biddy_Managed_PrintInfo) (Biddy_Manager MNG, FILE *f); /* 156 */
    Biddy_String (*biddy_Managed_Eval0) (Biddy_Manager MNG, Biddy_String s); /* 157 */
    Biddy_Edge (*biddy_Managed_Eval1x) (Biddy_Manager MNG, Biddy_String s, Biddy_LookupFunction lf); /* 158 */
    Biddy_Edge (*biddy_Managed_Eval2) (Biddy_Manager MNG, Biddy_String boolFunc); /* 159 */
    Biddy_String (*biddy_Managed_ReadBddview) (Biddy_Manager MNG, const char filename[], Biddy_String name); /* 160 */
    void (*biddy_Managed_ReadVerilogFile) (Biddy_Manager MNG, const char filename[], Biddy_String prefix); /* 161 */
    void (*biddy_Managed_PrintfBDD) (Biddy_Manager MNG, Biddy_Edge f); /* 162 */
    void (*biddy_Managed_WriteBDD) (Biddy_Manager MNG, const char filename[], Biddy_Edge f, Biddy_String label); /* 163 */
    void (*biddy_Managed_PrintfTable) (Biddy_Manager MNG, Biddy_Edge f); /* 164 */
    void (*biddy_Managed_WriteTable) (Biddy_Manager MNG, const char filename[], Biddy_Edge f); /* 165 */
    void (*biddy_Managed_PrintfSOP) (Biddy_Manager MNG, Biddy_Edge f); /* 166 */
    void (*biddy_Managed_WriteSOP) (Biddy_Manager MNG, const char filename[], Biddy_Edge f); /* 167 */
    void (*biddy_Managed_PrintfMinterms) (Biddy_Manager MNG, Biddy_Edge f, Biddy_Boolean negative); /* 168 */
    unsigned int (*biddy_Managed_WriteDot) (Biddy_Manager MNG, const char filename[], Biddy_Edge f, const char label[], int id, Biddy_Boolean cudd); /* 169 */
    unsigned int (*biddy_Managed_WriteBddview) (Biddy_Manager MNG, const char filename[], Biddy_Edge f, const char label[], void *table); /* 170 */

    Biddy_Manager (*bddscout_GetActiveManager) (); /* 1 */
    void (*bddscout_ClearActiveManager) (); /* 2 */
    void (*bddscout_ChangeActiveManager) (); /* 3 */

} BddscoutStubs;

#ifndef BUILD_BDDSCOUTSTUBS

#ifdef __cplusplus
extern "C" {
#endif
extern const BddscoutStubs *bddscoutStubsPtr;
#ifdef __cplusplus
}
#endif

#define Biddy_InitMNG \
    (bddscoutStubsPtr->biddy_InitMNG) /* 1 */
#define Biddy_ExitMNG \
    (bddscoutStubsPtr->biddy_ExitMNG) /* 2 */
#define Biddy_About \
    (bddscoutStubsPtr->biddy_About) /* 3 */
#define Biddy_Managed_GetManagerType \
    (bddscoutStubsPtr->biddy_Managed_GetManagerType) /* 4 */
#define Biddy_Managed_GetManagerName \
    (bddscoutStubsPtr->biddy_Managed_GetManagerName) /* 5 */
#define Biddy_Managed_SetManagerParameters \
    (bddscoutStubsPtr->biddy_Managed_SetManagerParameters) /* 6 */
#define Biddy_GetThen \
    (bddscoutStubsPtr->biddy_GetThen) /* 7 */
#define Biddy_GetElse \
    (bddscoutStubsPtr->biddy_GetElse) /* 8 */
#define Biddy_GetTopVariable \
    (bddscoutStubsPtr->biddy_GetTopVariable) /* 9 */
#define Biddy_Managed_IsEqv \
    (bddscoutStubsPtr->biddy_Managed_IsEqv) /* 10 */
#define Biddy_Managed_SelectNode \
    (bddscoutStubsPtr->biddy_Managed_SelectNode) /* 11 */
#define Biddy_Managed_DeselectNode \
    (bddscoutStubsPtr->biddy_Managed_DeselectNode) /* 12 */
#define Biddy_Managed_IsSelected \
    (bddscoutStubsPtr->biddy_Managed_IsSelected) /* 13 */
#define Biddy_Managed_SelectFunction \
    (bddscoutStubsPtr->biddy_Managed_SelectFunction) /* 14 */
#define Biddy_Managed_DeselectAll \
    (bddscoutStubsPtr->biddy_Managed_DeselectAll) /* 15 */
#define Biddy_Managed_GetTerminal \
    (bddscoutStubsPtr->biddy_Managed_GetTerminal) /* 16 */
#define Biddy_Managed_GetConstantZero \
    (bddscoutStubsPtr->biddy_Managed_GetConstantZero) /* 17 */
#define Biddy_Managed_GetConstantOne \
    (bddscoutStubsPtr->biddy_Managed_GetConstantOne) /* 18 */
#define Biddy_Managed_GetBaseSet \
    (bddscoutStubsPtr->biddy_Managed_GetBaseSet) /* 19 */
#define Biddy_Managed_GetVariable \
    (bddscoutStubsPtr->biddy_Managed_GetVariable) /* 20 */
#define Biddy_Managed_GetLowestVariable \
    (bddscoutStubsPtr->biddy_Managed_GetLowestVariable) /* 21 */
#define Biddy_Managed_GetIthVariable \
    (bddscoutStubsPtr->biddy_Managed_GetIthVariable) /* 22 */
#define Biddy_Managed_GetPrevVariable \
    (bddscoutStubsPtr->biddy_Managed_GetPrevVariable) /* 23 */
#define Biddy_Managed_GetNextVariable \
    (bddscoutStubsPtr->biddy_Managed_GetNextVariable) /* 24 */
#define Biddy_Managed_GetVariableEdge \
    (bddscoutStubsPtr->biddy_Managed_GetVariableEdge) /* 25 */
#define Biddy_Managed_GetElementEdge \
    (bddscoutStubsPtr->biddy_Managed_GetElementEdge) /* 26 */
#define Biddy_Managed_GetVariableName \
    (bddscoutStubsPtr->biddy_Managed_GetVariableName) /* 27 */
#define Biddy_Managed_GetTopVariableEdge \
    (bddscoutStubsPtr->biddy_Managed_GetTopVariableEdge) /* 28 */
#define Biddy_Managed_GetTopVariableName \
    (bddscoutStubsPtr->biddy_Managed_GetTopVariableName) /* 29 */
#define Biddy_Managed_GetTopVariableChar \
    (bddscoutStubsPtr->biddy_Managed_GetTopVariableChar) /* 30 */
#define Biddy_Managed_ResetVariablesValue \
    (bddscoutStubsPtr->biddy_Managed_ResetVariablesValue) /* 31 */
#define Biddy_Managed_SetVariableValue \
    (bddscoutStubsPtr->biddy_Managed_SetVariableValue) /* 32 */
#define Biddy_Managed_GetVariableValue \
    (bddscoutStubsPtr->biddy_Managed_GetVariableValue) /* 33 */
#define Biddy_Managed_ClearVariablesData \
    (bddscoutStubsPtr->biddy_Managed_ClearVariablesData) /* 34 */
#define Biddy_Managed_SetVariableData \
    (bddscoutStubsPtr->biddy_Managed_SetVariableData) /* 35 */
#define Biddy_Managed_GetVariableData \
    (bddscoutStubsPtr->biddy_Managed_GetVariableData) /* 36 */
#define Biddy_Managed_Eval \
    (bddscoutStubsPtr->biddy_Managed_Eval) /* 37 */
#define Biddy_Managed_EvalProbability \
    (bddscoutStubsPtr->biddy_Managed_EvalProbability) /* 38 */
#define Biddy_Managed_IsSmaller \
    (bddscoutStubsPtr->biddy_Managed_IsSmaller) /* 39 */
#define Biddy_Managed_IsLowest \
    (bddscoutStubsPtr->biddy_Managed_IsLowest) /* 40 */
#define Biddy_Managed_IsHighest \
    (bddscoutStubsPtr->biddy_Managed_IsHighest) /* 41 */
#define Biddy_Managed_FoaVariable \
    (bddscoutStubsPtr->biddy_Managed_FoaVariable) /* 42 */
#define Biddy_Managed_ChangeVariableName \
    (bddscoutStubsPtr->biddy_Managed_ChangeVariableName) /* 43 */
#define Biddy_Managed_AddVariableByName \
    (bddscoutStubsPtr->biddy_Managed_AddVariableByName) /* 44 */
#define Biddy_Managed_AddElementByName \
    (bddscoutStubsPtr->biddy_Managed_AddElementByName) /* 45 */
#define Biddy_Managed_AddVariableBelow \
    (bddscoutStubsPtr->biddy_Managed_AddVariableBelow) /* 46 */
#define Biddy_Managed_AddVariableAbove \
    (bddscoutStubsPtr->biddy_Managed_AddVariableAbove) /* 47 */
#define Biddy_Managed_TransferMark \
    (bddscoutStubsPtr->biddy_Managed_TransferMark) /* 48 */
#define Biddy_Managed_IncTag \
    (bddscoutStubsPtr->biddy_Managed_IncTag) /* 49 */
#define Biddy_Managed_TaggedFoaNode \
    (bddscoutStubsPtr->biddy_Managed_TaggedFoaNode) /* 50 */
#define Biddy_Managed_IsOK \
    (bddscoutStubsPtr->biddy_Managed_IsOK) /* 51 */
#define Biddy_Managed_GC \
    (bddscoutStubsPtr->biddy_Managed_GC) /* 52 */
#define Biddy_Managed_Clean \
    (bddscoutStubsPtr->biddy_Managed_Clean) /* 53 */
#define Biddy_Managed_Purge \
    (bddscoutStubsPtr->biddy_Managed_Purge) /* 54 */
#define Biddy_Managed_PurgeAndReorder \
    (bddscoutStubsPtr->biddy_Managed_PurgeAndReorder) /* 55 */
#define Biddy_Managed_Refresh \
    (bddscoutStubsPtr->biddy_Managed_Refresh) /* 56 */
#define Biddy_Managed_AddCache \
    (bddscoutStubsPtr->biddy_Managed_AddCache) /* 57 */
#define Biddy_Managed_AddFormula \
    (bddscoutStubsPtr->biddy_Managed_AddFormula) /* 58 */
#define Biddy_Managed_FindFormula \
    (bddscoutStubsPtr->biddy_Managed_FindFormula) /* 59 */
#define Biddy_Managed_DeleteFormula \
    (bddscoutStubsPtr->biddy_Managed_DeleteFormula) /* 60 */
#define Biddy_Managed_DeleteIthFormula \
    (bddscoutStubsPtr->biddy_Managed_DeleteIthFormula) /* 61 */
#define Biddy_Managed_GetIthFormula \
    (bddscoutStubsPtr->biddy_Managed_GetIthFormula) /* 62 */
#define Biddy_Managed_GetIthFormulaName \
    (bddscoutStubsPtr->biddy_Managed_GetIthFormulaName) /* 63 */
#define Biddy_Managed_GetOrdering \
    (bddscoutStubsPtr->biddy_Managed_GetOrdering) /* 64 */
#define Biddy_Managed_SetOrdering \
    (bddscoutStubsPtr->biddy_Managed_SetOrdering) /* 65 */
#define Biddy_Managed_SetAlphabeticOrdering \
    (bddscoutStubsPtr->biddy_Managed_SetAlphabeticOrdering) /* 66 */
#define Biddy_Managed_SwapWithHigher \
    (bddscoutStubsPtr->biddy_Managed_SwapWithHigher) /* 67 */
#define Biddy_Managed_SwapWithLower \
    (bddscoutStubsPtr->biddy_Managed_SwapWithLower) /* 68 */
#define Biddy_Managed_Sifting \
    (bddscoutStubsPtr->biddy_Managed_Sifting) /* 69 */
#define Biddy_Managed_MinimizeBDD \
    (bddscoutStubsPtr->biddy_Managed_MinimizeBDD) /* 70 */
#define Biddy_Managed_MaximizeBDD \
    (bddscoutStubsPtr->biddy_Managed_MaximizeBDD) /* 71 */
#define Biddy_Managed_Copy \
    (bddscoutStubsPtr->biddy_Managed_Copy) /* 72 */
#define Biddy_Managed_CopyFormula \
    (bddscoutStubsPtr->biddy_Managed_CopyFormula) /* 73 */
#define Biddy_Managed_ConstructBDD \
    (bddscoutStubsPtr->biddy_Managed_ConstructBDD) /* 74 */

#define Biddy_Managed_Not \
    (bddscoutStubsPtr->biddy_Managed_Not) /* 75 */
#define Biddy_Managed_ITE \
    (bddscoutStubsPtr->biddy_Managed_ITE) /* 76 */
#define Biddy_Managed_And \
    (bddscoutStubsPtr->biddy_Managed_And) /* 77 */
#define Biddy_Managed_Or \
    (bddscoutStubsPtr->biddy_Managed_Or) /* 78 */
#define Biddy_Managed_Nand \
    (bddscoutStubsPtr->biddy_Managed_Nand) /* 79 */
#define Biddy_Managed_Nor \
    (bddscoutStubsPtr->biddy_Managed_Nor) /* 80 */
#define Biddy_Managed_Xor \
    (bddscoutStubsPtr->biddy_Managed_Xor) /* 81 */
#define Biddy_Managed_Xnor \
    (bddscoutStubsPtr->biddy_Managed_Xnor) /* 82 */
#define Biddy_Managed_Leq \
    (bddscoutStubsPtr->biddy_Managed_Leq) /* 83 */
#define Biddy_Managed_Gt \
    (bddscoutStubsPtr->biddy_Managed_Gt) /* 84 */
#define Biddy_Managed_IsLeq \
    (bddscoutStubsPtr->biddy_Managed_IsLeq) /* 85 */
#define Biddy_Managed_Restrict \
    (bddscoutStubsPtr->biddy_Managed_Restrict) /* 86 */
#define Biddy_Managed_Compose \
    (bddscoutStubsPtr->biddy_Managed_Compose) /* 87 */
#define Biddy_Managed_E \
    (bddscoutStubsPtr->biddy_Managed_E) /* 88 */
#define Biddy_Managed_A \
    (bddscoutStubsPtr->biddy_Managed_A) /* 89 */
#define Biddy_Managed_IsVariableDependent \
    (bddscoutStubsPtr->biddy_Managed_IsVariableDependent) /* 90 */
#define Biddy_Managed_ExistAbstract \
    (bddscoutStubsPtr->biddy_Managed_ExistAbstract) /* 91 */
#define Biddy_Managed_UnivAbstract \
    (bddscoutStubsPtr->biddy_Managed_UnivAbstract) /* 92 */
#define Biddy_Managed_AndAbstract \
    (bddscoutStubsPtr->biddy_Managed_AndAbstract) /* 93 */
#define Biddy_Managed_Constrain \
    (bddscoutStubsPtr->biddy_Managed_Constrain) /* 94 */
#define Biddy_Managed_Simplify \
    (bddscoutStubsPtr->biddy_Managed_Simplify) /* 95 */
#define Biddy_Managed_Support \
    (bddscoutStubsPtr->biddy_Managed_Support) /* 96 */
#define Biddy_Managed_ReplaceByKeyword \
    (bddscoutStubsPtr->biddy_Managed_ReplaceByKeyword) /* 97 */
#define Biddy_Managed_Change \
    (bddscoutStubsPtr->biddy_Managed_Change) /* 98 */
#define Biddy_Managed_VarSubset \
    (bddscoutStubsPtr->biddy_Managed_VarSubset) /* 99 */
#define Biddy_Managed_ElementAbstract \
    (bddscoutStubsPtr->biddy_Managed_ElementAbstract) /* 100... */
#define Biddy_Managed_Product \
    (bddscoutStubsPtr->biddy_Managed_Product) /* 100 */
#define Biddy_Managed_SelectiveProduct \
    (bddscoutStubsPtr->biddy_Managed_SelectiveProduct) /* 101 */
#define Biddy_Managed_Supset \
    (bddscoutStubsPtr->biddy_Managed_Supset) /* 102 */
#define Biddy_Managed_Subset \
    (bddscoutStubsPtr->biddy_Managed_Subset) /* 103 */
#define Biddy_Managed_Permitsym \
    (bddscoutStubsPtr->biddy_Managed_Permitsym) /* 104 */
#define Biddy_Managed_Stretch \
    (bddscoutStubsPtr->biddy_Managed_Stretch) /* 105 */
#define Biddy_Managed_CreateMinterm \
    (bddscoutStubsPtr->biddy_Managed_CreateMinterm) /* 106 */
#define Biddy_Managed_CreateFunction \
    (bddscoutStubsPtr->biddy_Managed_CreateFunction) /* 107 */
#define Biddy_Managed_RandomFunction \
    (bddscoutStubsPtr->biddy_Managed_RandomFunction) /* 108 */
#define Biddy_Managed_RandomSet \
    (bddscoutStubsPtr->biddy_Managed_RandomSet) /* 109 */
#define Biddy_Managed_ExtractMinterm \
    (bddscoutStubsPtr->biddy_Managed_ExtractMinterm) /* 110 */

#define Biddy_Managed_CountNodes \
    (bddscoutStubsPtr->biddy_Managed_CountNodes) /* 111 */
#define Biddy_MaxLevel \
    (bddscoutStubsPtr->biddy_MaxLevel) /* 112 */
#define Biddy_AvgLevel \
    (bddscoutStubsPtr->biddy_AvgLevel) /* 113 */
#define Biddy_Managed_VariableTableNum \
    (bddscoutStubsPtr->biddy_Managed_VariableTableNum) /* 114 */
#define Biddy_Managed_NodeTableSize \
    (bddscoutStubsPtr->biddy_Managed_NodeTableSize) /* 115 */
#define Biddy_Managed_NodeTableBlockNumber \
    (bddscoutStubsPtr->biddy_Managed_NodeTableBlockNumber) /* 116 */
#define Biddy_Managed_NodeTableGenerated \
    (bddscoutStubsPtr->biddy_Managed_NodeTableGenerated) /* 117 */
#define Biddy_Managed_NodeTableMax \
    (bddscoutStubsPtr->biddy_Managed_NodeTableMax) /* 118 */
#define Biddy_Managed_NodeTableNum \
    (bddscoutStubsPtr->biddy_Managed_NodeTableNum) /* 119 */
#define Biddy_Managed_NodeTableNumVar \
    (bddscoutStubsPtr->biddy_Managed_NodeTableNumVar) /* 120 */
#define Biddy_Managed_NodeTableResizeNumber \
    (bddscoutStubsPtr->biddy_Managed_NodeTableResizeNumber) /* 121 */
#define Biddy_Managed_NodeTableFoaNumber \
    (bddscoutStubsPtr->biddy_Managed_NodeTableFoaNumber) /* 122 */
#define Biddy_Managed_NodeTableFindNumber \
    (bddscoutStubsPtr->biddy_Managed_NodeTableFindNumber) /* 123 */
#define Biddy_Managed_NodeTableCompareNumber \
    (bddscoutStubsPtr->biddy_Managed_NodeTableCompareNumber) /* 124 */
#define Biddy_Managed_NodeTableAddNumber \
    (bddscoutStubsPtr->biddy_Managed_NodeTableAddNumber) /* 125 */
#define Biddy_Managed_NodeTableGCNumber \
    (bddscoutStubsPtr->biddy_Managed_NodeTableGCNumber) /* 126 */
#define Biddy_Managed_NodeTableGCTime \
    (bddscoutStubsPtr->biddy_Managed_NodeTableGCTime) /* 127 */
#define Biddy_Managed_NodeTableGCObsoleteNumber \
    (bddscoutStubsPtr->biddy_Managed_NodeTableGCObsoleteNumber) /* 128 */
#define Biddy_Managed_NodeTableSwapNumber \
    (bddscoutStubsPtr->biddy_Managed_NodeTableSwapNumber) /* 129 */
#define Biddy_Managed_NodeTableSiftingNumber \
    (bddscoutStubsPtr->biddy_Managed_NodeTableSiftingNumber) /* 130 */
#define Biddy_Managed_NodeTableDRTime \
    (bddscoutStubsPtr->biddy_Managed_NodeTableDRTime) /* 131 */
#define Biddy_Managed_NodeTableITENumber \
    (bddscoutStubsPtr->biddy_Managed_NodeTableITENumber) /* 132 */
#define Biddy_Managed_NodeTableITERecursiveNumber \
    (bddscoutStubsPtr->biddy_Managed_NodeTableITERecursiveNumber) /* 133 */
#define Biddy_Managed_NodeTableANDORNumber \
    (bddscoutStubsPtr->biddy_Managed_NodeTableANDORNumber) /* 134 */
#define Biddy_Managed_NodeTableANDORRecursiveNumber \
    (bddscoutStubsPtr->biddy_Managed_NodeTableANDORRecursiveNumber) /* 135 */
#define Biddy_Managed_NodeTableXORNumber \
    (bddscoutStubsPtr->biddy_Managed_NodeTableXORNumber) /* 136 */
#define Biddy_Managed_NodeTableXORRecursiveNumber \
    (bddscoutStubsPtr->biddy_Managed_NodeTableXORRecursiveNumber) /* 137 */
#define Biddy_Managed_FormulaTableNum \
    (bddscoutStubsPtr->biddy_Managed_FormulaTableNum) /* 138 */
#define Biddy_Managed_ListUsed \
    (bddscoutStubsPtr->biddy_Managed_ListUsed) /* 139 */
#define Biddy_Managed_ListMaxLength \
    (bddscoutStubsPtr->biddy_Managed_ListMaxLength) /* 140 */
#define Biddy_Managed_ListAvgLength \
    (bddscoutStubsPtr->biddy_Managed_ListAvgLength) /* 141 */
#define Biddy_Managed_OPCacheSearch \
    (bddscoutStubsPtr->biddy_Managed_OPCacheSearch) /* 142 */
#define Biddy_Managed_OPCacheFind \
    (bddscoutStubsPtr->biddy_Managed_OPCacheFind) /* 143 */
#define Biddy_Managed_OPCacheInsert \
    (bddscoutStubsPtr->biddy_Managed_OPCacheInsert) /* 144 */
#define Biddy_Managed_OPCacheOverwrite \
    (bddscoutStubsPtr->biddy_Managed_OPCacheOverwrite) /* 145 */
#define Biddy_Managed_CountNodesPlain \
    (bddscoutStubsPtr->biddy_Managed_CountNodesPlain) /* 146 */
#define Biddy_Managed_DependentVariableNumber \
    (bddscoutStubsPtr->biddy_Managed_DependentVariableNumber) /* 147 */
#define Biddy_Managed_CountComplementedEdges \
    (bddscoutStubsPtr->biddy_Managed_CountComplementedEdges) /* 148 */
#define Biddy_Managed_CountPaths \
    (bddscoutStubsPtr->biddy_Managed_CountPaths) /* 149 */
#define Biddy_Managed_CountMinterms \
    (bddscoutStubsPtr->biddy_Managed_CountMinterms) /* 150 */
#define Biddy_Managed_DensityOfFunction \
    (bddscoutStubsPtr->biddy_Managed_DensityOfFunction) /* 151 */
#define Biddy_Managed_DensityOfBDD \
    (bddscoutStubsPtr->biddy_Managed_DensityOfBDD) /* 152 */
#define Biddy_Managed_MinNodes \
    (bddscoutStubsPtr->biddy_Managed_MinNodes) /* 153 */
#define Biddy_Managed_MaxNodes \
    (bddscoutStubsPtr->biddy_Managed_MaxNodes) /* 154 */
#define Biddy_Managed_ReadMemoryInUse \
    (bddscoutStubsPtr->biddy_Managed_ReadMemoryInUse) /* 155 */
#define Biddy_Managed_PrintInfo \
    (bddscoutStubsPtr->biddy_Managed_PrintInfo) /* 156 */

#define Biddy_Managed_Eval0 \
    (bddscoutStubsPtr->biddy_Managed_Eval0) /* 157 */
#define Biddy_Managed_Eval1x \
    (bddscoutStubsPtr->biddy_Managed_Eval1x) /* 158 */
#define Biddy_Managed_Eval2 \
    (bddscoutStubsPtr->biddy_Managed_Eval2) /* 159 */
#define Biddy_Managed_ReadBddview\
    (bddscoutStubsPtr->biddy_Managed_ReadBddview) /* 160 */
#define Biddy_Managed_ReadVerilogFile\
    (bddscoutStubsPtr->biddy_Managed_ReadVerilogFile) /* 161 */
#define Biddy_Managed_PrintfBDD\
    (bddscoutStubsPtr->biddy_Managed_PrintfBDD) /* 162 */
#define Biddy_Managed_WriteBDD \
    (bddscoutStubsPtr->biddy_Managed_WriteBDD) /* 163 */
#define Biddy_Managed_PrintfTable \
    (bddscoutStubsPtr->biddy_Managed_PrintfTable) /* 164 */
#define Biddy_Managed_WriteTable \
    (bddscoutStubsPtr->biddy_Managed_WriteTable) /* 165 */
#define Biddy_Managed_PrintfSOP \
    (bddscoutStubsPtr->biddy_Managed_PrintfSOP) /* 166 */
#define Biddy_Managed_WriteSOP \
    (bddscoutStubsPtr->biddy_Managed_WriteSOP) /* 167 */
#define Biddy_Managed_PrintfMinterms \
    (bddscoutStubsPtr->biddy_Managed_PrintfMinterms) /* 168 */
#define Biddy_Managed_WriteDot \
    (bddscoutStubsPtr->biddy_Managed_WriteDot) /* 169 */
#define Biddy_Managed_WriteBddview \
    (bddscoutStubsPtr->biddy_Managed_WriteBddview) /* 170 */

#define Bddscout_GetActiveManager \
    (bddscoutStubsPtr->bddscout_GetActiveManager) /* 1 */
#define Bddscout_ClearActiveManager \
    (bddscoutStubsPtr->bddscout_ClearActiveManager) /* 2 */
#define Bddscout_ChangeActiveManager \
    (bddscoutStubsPtr->bddscout_ChangeActiveManager) /* 3 */

#endif

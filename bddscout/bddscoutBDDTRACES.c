/**CFile*******************************************************************
  PackageName [Bdd Scout]
  Synopsis    [Bdd Scout]

  FileName    [bddscoutBDDTRACES.c]
  Revision    [$Revision: 100 $]
  Date        [$Date: 2015-10-05 16:59:26 +0200 (pon, 05 okt 2015) $]
  Authors     [Robert Meolic (robert.meolic@um.si)]
  Description []
  SeeAlso     [bddscout.h]

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

/* THIS WORK IS DERIVED FROM bdd-trace-driver-0.9 (Bwolen Yang) */
/* ORIGINAL WORK IS OBTAINED FROM http://www.cs.cmu.edu/~bwolen/software/ */
/* HERE IS THE ORIGINAL COPYRIGHT */

/*
 * Copyright (c) 1998 Carnegie Mellon University.
 * All Rights Reserved.
 *
 * Permission to use, copy, modify, and distribute this software and
 * its documentation is hereby granted (including for commercial or
 * for-profit use), provided that both the copyright notice and this
 * permission notice appear in all copies of the software, derivative
 * works, or modified versions, and any portions thereof, and that
 * both notices appear in supporting documentation, and that credit
 * is given to Carnegie Mellon University in all publications reporting
 * on direct or indirect use of this code or its derivatives.
 * 
 * THIS IMPLEMENTATION MAY HAVE BUGS, SOME OF WHICH MAY HAVE SERIOUS
 * CONSEQUENCES.  CARNEGIE MELLON PROVIDES THIS SOFTWARE IN ITS "AS IS"
 * CONDITION, AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING,
 * BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY
 * AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL
 * CARNEGIE MELLON UNIVERSITY BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED
 * TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
 * LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 * 
 * Carnegie Mellon encourages (but does not require) users of this
 * software to return any improvements or extensions that they make,
 * and to grant Carnegie Mellon the rights to redistribute these
 * changes without encumbrance.   
 */

/* OTHER USEFUL LINKS */
/* http://javabdd.sourceforge.net/ */
/* http://javabdd.sourceforge.net/xref-test/trace/TraceDriver.html */

#include "bddscout.h"

/* on tcl 8.3 use #define USECONST */
/* on tcl 8.4 use #define USECONST const*/
/* this is defined in Makefile */

#define NODEBUG

/*-----------------------------------------------------------------------*/
/* Constant declarations                                                 */
/*-----------------------------------------------------------------------*/

#define T_TRACE_VERBOSE_PRINT_OP "trace_verbose_print"

#define T_NEW_INT_LEAF_OP "new_int_leaf"
#define T_EQUAL_OP "are_equal"

#define T_ITE_OP "ite"
#define T_NOT_OP "not"
#define T_AND_OP "and"
#define T_OR_OP "or"
#define T_XOR_OP "xor"
#define T_NAND_OP "nand"
#define T_NOR_OP "nor"
#define T_XNOR_OP "xnor"

#define T_VARS_CURR_TO_NEXT_OP "vars_curr_to_next"
#define T_VARS_NEXT_TO_CURR_OP "vars_next_to_curr"

#define T_EXISTS_OP "exists"
#define T_FORALL_OP "forall"
#define T_NEXT_OP "image"
#define T_PREV_OP "preimage"
#define T_REL_PROD_OP "rel_prod"

#define T_REDUCE_OP "restrict"
#define T_SATISFY_OP "satisfy"
#define T_N_CUBES_OP "set_size"

#define T_SUPPORT_VARS_OP "support_vars"
#define T_VARSET_TO_BDD_OP "varset_to_bdd"
#define T_VARSET_DIFF_OP "varset_diff"
#define T_COMPOSE_OP "compose"
#define T_COMPOSE_UNION_OP "compose_union"
#define T_COMPOSE_UNION_REVERSE_OP "compose_union_reverse"

#define T_PARTIAL_INTERSECT_OP "partial_intersect"
#define T_PARTIAL_FILTER_OP "partial_filter"

#define T_NEW_BDD_VAR_LAST_OP "new_bdd_var"
#define T_GET_ITH_VAR_OP "get_ith_var"
#define T_IDENTITY_TRANS_OP "identity_trans"

#define T_INCR_REF_OP "incr_ref"
#define T_DECR_REF_OP "decr_ref"

#define T_CHECK_POINT_FOR_REORDER_OP "check_point_for_force_reordering"

/*-----------------------------------------------------------------------*/
/* Structure declarations                                                */
/*-----------------------------------------------------------------------*/

/*-----------------------------------------------------------------------*/
/* Variable declarations                                                 */
/*-----------------------------------------------------------------------*/

int SCAN_RESULT; /* used in readln macro */

/*-----------------------------------------------------------------------*/
/* Functions                                                             */
/*-----------------------------------------------------------------------*/

/**Function****************************************************************
  Synopsis    [Function Bddscout_RunBddTrace]
  Description []
  SideEffects []
  SeeAlso     []
  ************************************************************************/

#define readln(f,s) SCAN_RESULT = fscanf(f," %[^\n] ",s)

Biddy_String
Bddscout_RunBddTrace(FILE *f)
{
  Biddy_String report;
  Biddy_String line,line1,line2,line3,numline;
  Biddy_Boolean quote;
  Biddy_String s1;
  Biddy_String var,op1,op2,op3;
  Biddy_Edge* varlist;
  int len,varnum;
  int i,j;
  Biddy_Edge bdd1,bdd2,bdd3,bdd;
  int result,correct;

  report = (Biddy_String) malloc(255);
  sprintf(report,"%s\n",Tcl_GetHostName());

  line = (Biddy_String) malloc(4096);
  numline = (Biddy_String) malloc(32);

  /* PREFIX - INCLUDES COMMENTS */

  do {
    readln(f,line);
  } while (strstr(line, "MODULE") != line);

  /* MODULE */

#ifdef DEBUG
  printf("MODULE\n");
#endif

  do {
    readln(f,line);
  } while (strstr(line,"INPUT") != line);

  /* INPUT  */

#ifdef DEBUG
  printf("INPUT\n");
#endif

  readln(f,line);
  s1 = strdup(line);
  readln(f,line);

  while (strstr(line,"OUTPUT") != line)
  {
    len = strlen(s1)+strlen(line)+1;
    s1 = (char *) realloc(s1, sizeof(char) * len);
    strcat(s1,line);
    readln(f,line);
  }

  varlist = (Biddy_Edge *) malloc(sizeof(Biddy_Edge));
  varlist[0] = BDDNULL;
  varnum = 0;

  var = strtok(s1," \t,;");
  while (var) {

    if (var[0]!=',' && var[0]!=';') {

#ifdef DEBUG
      printf("VARIABLE: <%s>\n",var);
#endif

      bdd = Biddy_FoaVariable(var);
      Biddy_AddPersistentFormula(var,bdd);

      varnum++;
      varlist = (Biddy_Edge *) realloc(varlist,(varnum+1)*sizeof(Biddy_Edge));
      varlist[varnum] = bdd;

    }
    var = strtok(NULL," \t,;");
  }

  free(s1);

  /* OUTPUT  */

#ifdef DEBUG
  printf("OUTPUT\n");
#endif

  readln(f,line);
  s1 = strdup(line);
  readln(f,line);

  while (strstr(line,"STRUCTURE") != line)
  {
    len = strlen(s1)+strlen(line)+1;
    s1 = (char *) realloc(s1, sizeof(char) * len);
    strcat(s1,line);
    readln(f,line);
  }

  var = strtok(s1," \t,;");
  while (var) {

    if (var[0]!=',' && var[0]!=';') {

#ifdef DEBUG
      printf("VARIABLE: <%s>\n",var);
#endif

    }
    var = strtok(NULL," \t,;");
  }

  free(s1);

  /* STRUCTURE */

  readln(f,line);

  while (strstr(line,"ENDMODULE") != line)
  {

#ifdef DEBUG
    printf("CMD: <%s>",line);
#endif

    /* REMOVE ALL UNQUOTED SPACES */
    len = strlen(line);
    j=0;
    quote = FALSE;
    for (i=0;i<len;i++) {
      if (quote || ((line[i]!=' ') && (line[i]!='\t'))) {
        line[j]=line[i];
        if (line[i]=='\"') quote = !quote;
        j++;
      }
    }
    line[j]= 0;

    if (!strncmp(line,T_TRACE_VERBOSE_PRINT_OP,strlen(T_TRACE_VERBOSE_PRINT_OP))) {
      correct = -1;

#ifdef DEBUG
      printf(" OK\n");
#endif

      result = -1;
    } else if (!strncmp(line,T_CHECK_POINT_FOR_REORDER_OP,strlen(T_CHECK_POINT_FOR_REORDER_OP))) {
      correct = -1;

#ifdef DEBUG
      printf(" OK\n");
#endif

      result = -1;
    } else {
      line1 = strrchr(line,'%');
      if (!line1) {
        correct = -1;
      } else {
        sscanf(&(line1[1]),"%d",&correct);
        line1[0] = 0;
      }

      if (!strncmp(line,T_EQUAL_OP,strlen(T_EQUAL_OP))) {

        line1 = strchr(&(line[10]),',');
        line1[0]=0;
        op1 = strdup(&(line[10]));
        line1[0]=',';
        line2 = strchr(&(line1[1]),')');
        line2[0]=0;
        op2 = strdup(&(line1[1]));
        line2[0]=')';

#ifdef DEBUG
        printf(" CHECK <%s> == <%s>",op1,op2);
#endif

        if (Biddy_FindFormula(op1,&bdd1) &&
            Biddy_FindFormula(op2,&bdd2))
        {
          if (Biddy_IsEqv(bdd1,bdd2)) {result = 1;} else {result = 0;}
        } else {
          result = -1;
        }

#ifdef DEBUG
        printf(" OK\n");
#endif

        free(op1);
        free(op2);

      } else {

        line1 = strchr(line,'=');
        if (!line1) {
 
#ifdef DEBUG
          printf(" UNKNOWN\n");
#endif

          result = -1;

        } else {

          line1[0]=0;
          var = strdup(line);
          line1[0]='=';
          s1 = strdup(&(line1[1]));

#ifdef DEBUG
          printf(" ASSIGN <%s> = <%s>",var,s1);
#endif

          if (!strncmp(s1,T_NEW_INT_LEAF_OP,strlen(T_NEW_INT_LEAF_OP))) {

            if (s1[strlen(T_NEW_INT_LEAF_OP)+1] == '0') {
              bdd = Biddy_GetConstantZero();
              Biddy_AddPersistentFormula(var,bdd);
 
#ifdef DEBUG
              printf(" OK\n");
#endif

            } else if (s1[strlen(T_NEW_INT_LEAF_OP)+1] == '1') {
              bdd = Biddy_GetConstantOne();
              Biddy_AddPersistentFormula(var,bdd);

#ifdef DEBUG
              printf(" OK\n");
#endif

            } else {

#ifdef DEBUG
              printf(" UNSUPPORTED\n");
#endif

              bdd = BDDNULL;

            }

            result = Biddy_NodeNumberPlain(bdd);

          } else if (!strncmp(s1,T_ITE_OP,strlen(T_ITE_OP))) {

            line1 = strchr(&(s1[strlen(T_ITE_OP)+1]),',');
            line1[0]=0;
            op1 = strdup(&(s1[strlen(T_ITE_OP)+1]));
            line1[0]=',';
            line2 = strchr(&(line1[1]),',');
            line2[0]=0;
            op2 = strdup(&(line1[1]));
            line2[0]=',';
            line3 = strchr(&(line2[1]),')');
            line3[0]=0;
            op3 = strdup(&(line2[1]));
            line3[0]=')';

            if (Biddy_FindFormula(op1,&bdd1) &&
                Biddy_FindFormula(op2,&bdd2) &&
                Biddy_FindFormula(op3,&bdd3))
            {
              bdd = Biddy_ITE(bdd1,bdd2,bdd3);
              Biddy_AddPersistentFormula(var,bdd);
            } else {
              bdd = BDDNULL;
            }

#ifdef DEBUG
            printf(" OK\n");
#endif

            free(op1);
            free(op2);
            free(op3);
            result = Biddy_NodeNumberPlain(bdd);

          } else if (!strncmp(s1,T_VARS_CURR_TO_NEXT_OP,strlen(T_VARS_CURR_TO_NEXT_OP))) {

            line1 = strchr(&(s1[strlen(T_VARS_CURR_TO_NEXT_OP)+1]),')');
            line1[0]=0;
            op1 = strdup(&(s1[strlen(T_VARS_CURR_TO_NEXT_OP)+1]));
            line1[0]=')';

            if (Biddy_FindFormula(op1,&bdd1))
            {
              Biddy_ResetVariablesValue();
              for (i=1;i<varnum;i=i+2) {
                Biddy_SetVariableValue(Biddy_GetTopVariable(varlist[i]),varlist[i+1]);
              }
              bdd = Biddy_Replace(bdd1);
              Biddy_AddPersistentFormula(var,bdd);
            } else {
              bdd = BDDNULL;
            }

#ifdef DEBUG
            printf(" OK\n");
#endif

            free(op1);
            result = Biddy_NodeNumberPlain(bdd);

          } else if (!strncmp(s1,T_VARS_NEXT_TO_CURR_OP,strlen(T_VARS_NEXT_TO_CURR_OP))) {

            line1 = strchr(&(s1[strlen(T_VARS_NEXT_TO_CURR_OP)+1]),')');
            line1[0]=0;
            op1 = strdup(&(s1[strlen(T_VARS_NEXT_TO_CURR_OP)+1]));
            line1[0]=')';

            if (Biddy_FindFormula(op1,&bdd1))
            {
              Biddy_ResetVariablesValue();
              for (i=1;i<varnum;i=i+2) {
                Biddy_SetVariableValue(Biddy_GetTopVariable(varlist[i+1]),varlist[i]);
              }
              bdd = Biddy_Replace(bdd1);
              Biddy_AddPersistentFormula(var,bdd);
            } else {
              bdd = BDDNULL;
            }

#ifdef DEBUG
            printf(" OK\n");
#endif

            free(op1);
            result = Biddy_NodeNumberPlain(bdd);

          } else if (!strncmp(s1,T_SUPPORT_VARS_OP,strlen(T_SUPPORT_VARS_OP))) {

            line1 = strchr(&(s1[strlen(T_SUPPORT_VARS_OP)+1]),')');
            line1[0]=0;
            op1 = strdup(&(s1[strlen(T_SUPPORT_VARS_OP)+1]));
            line1[0]=')';

            if (Biddy_FindFormula(op1,&bdd1))
            {
              bdd = Biddy_Support(bdd1);
              Biddy_AddPersistentFormula(var,bdd);
            } else {
              bdd = BDDNULL;
            }

#ifdef DEBUG
            printf(" OK\n");
#endif

            free(op1);
            result = Biddy_NodeNumberPlain(bdd);

          } else if (!strncmp(s1,T_EXISTS_OP,strlen(T_EXISTS_OP))) {

            line1 = strchr(&(s1[strlen(T_EXISTS_OP)+1]),',');
            line1[0]=0;
            op1 = strdup(&(s1[strlen(T_EXISTS_OP)+1]));
            line1[0]=',';
            line2 = strchr(&(line1[1]),')');
            line2[0]=0;
            op2 = strdup(&(line1[1]));
            line2[0]=')';

            if (Biddy_FindFormula(op1,&bdd1) &&
                Biddy_FindFormula(op2,&bdd2))
            {
              bdd = Biddy_ExistAbstract(bdd1,bdd2);
              Biddy_AddPersistentFormula(var,bdd);
            } else {
              bdd = BDDNULL;
            }

#ifdef DEBUG
            printf(" OK\n");
#endif

            free(op1);
            free(op2);
            result = Biddy_NodeNumberPlain(bdd);

          } else if (!strncmp(s1,T_NOT_OP,strlen(T_NOT_OP))) {

            line1 = strchr(&(s1[strlen(T_NOT_OP)+1]),')');
            line1[0]=0;
            op1 = strdup(&(s1[strlen(T_NOT_OP)+1]));
            line1[0]=')';

            if (Biddy_FindFormula(op1,&bdd1))
            {
              bdd = Biddy_Not(bdd1);
              Biddy_AddPersistentFormula(var,bdd);
            } else {
              bdd = BDDNULL;
            }

#ifdef DEBUG
            printf(" OK\n");
#endif

            free(op1);
            result = Biddy_NodeNumberPlain(bdd);

          } else if (!strncmp(s1,T_AND_OP,strlen(T_AND_OP))) {

            line1 = strchr(&(s1[strlen(T_AND_OP)+1]),',');
            line1[0]=0;
            op1 = strdup(&(s1[strlen(T_AND_OP)+1]));
            line1[0]=',';
            line2 = strchr(&(line1[1]),')');
            line2[0]=0;
            op2 = strdup(&(line1[1]));
            line2[0]=')';

            if (Biddy_FindFormula(op1,&bdd1) &&
                Biddy_FindFormula(op2,&bdd2))
            {
              bdd = Biddy_And(bdd1,bdd2);
              Biddy_AddPersistentFormula(var,bdd);
            } else {
              bdd = BDDNULL;
            }

#ifdef DEBUG
            printf(" OK\n");
#endif

            free(op1);
            free(op2);
            result = Biddy_NodeNumberPlain(bdd);

          } else if (!strncmp(s1,T_OR_OP,strlen(T_OR_OP))) {

            line1 = strchr(&(s1[strlen(T_OR_OP)+1]),',');
            line1[0]=0;
            op1 = strdup(&(s1[strlen(T_OR_OP)+1]));
            line1[0]=',';
            line2 = strchr(&(line1[1]),')');
            line2[0]=0;
            op2 = strdup(&(line1[1]));
            line2[0]=')';

            if (Biddy_FindFormula(op1,&bdd1) &&
                Biddy_FindFormula(op2,&bdd2))
            {
              bdd = Biddy_Or(bdd1,bdd2);
              Biddy_AddPersistentFormula(var,bdd);
            } else {
              bdd = BDDNULL;
            }

#ifdef DEBUG
            printf(" OK\n");
#endif

            free(op1);
            free(op2);
            result = Biddy_NodeNumberPlain(bdd);

          } else if (!strncmp(s1,T_REDUCE_OP,strlen(T_REDUCE_OP))) {

            line1 = strchr(&(s1[strlen(T_REDUCE_OP)+1]),',');
            line1[0]=0;
            op1 = strdup(&(s1[strlen(T_REDUCE_OP)+1]));
            line1[0]=',';
            line2 = strchr(&(line1[1]),')');
            line2[0]=0;
            op2 = strdup(&(line1[1]));
            line2[0]=')';

            if (Biddy_FindFormula(op1,&bdd1) &&
                Biddy_FindFormula(op2,&bdd2))
            {
              bdd = Biddy_Simplify(bdd1,bdd2);
              Biddy_AddPersistentFormula(var,bdd);
            } else {
              bdd = BDDNULL;
            }

#ifdef DEBUG
            printf(" OK\n");
#endif

            free(op1);
            free(op2);
            result = Biddy_NodeNumberPlain(bdd);

          } else if (!strncmp(s1,T_REL_PROD_OP,strlen(T_REL_PROD_OP))) {

            line1 = strchr(&(s1[strlen(T_REL_PROD_OP)+1]),',');
            line1[0]=0;
            op1 = strdup(&(s1[strlen(T_REL_PROD_OP)+1]));
            line1[0]=',';
            line2 = strchr(&(line1[1]),',');
            line2[0]=0;
            op2 = strdup(&(line1[1]));
            line2[0]=',';
            line3 = strchr(&(line2[1]),')');
            line3[0]=0;
            op3 = strdup(&(line2[1]));
            line3[0]=')';

            if (Biddy_FindFormula(op1,&bdd1) &&
                Biddy_FindFormula(op2,&bdd2) &&
                Biddy_FindFormula(op3,&bdd3))
            {
              bdd = Biddy_AndAbstract(bdd2,bdd3,bdd1);
              Biddy_AddPersistentFormula(var,bdd);
            } else {
              bdd = BDDNULL;
            }

#ifdef DEBUG
            printf(" OK\n");
#endif

            free(op1);
            free(op2);
            result = Biddy_NodeNumberPlain(bdd);

          } else {

#ifdef DEBUG
            printf(" NOT IMPLEMENTED\n");
#endif

            result = -1;

          }

          free(var);
          free(s1);
        }

      }
    }

    /* PREPARE REPORT */

    len = strlen(report)+strlen(line)+2;
    report = (Biddy_String) realloc(report,len);
    strcat(report,line);
    strcat(report,"\n");

    sprintf(numline,"%d",correct);
    len = strlen(report)+strlen(numline)+2;
    report = (Biddy_String) realloc(report,len);
    strcat(report,numline);
    strcat(report,"\n");

    sprintf(numline,"%d",result);
    len = strlen(report)+strlen(numline)+2;
    report = (Biddy_String) realloc(report,len);
    strcat(report,numline);
    strcat(report,"\n");

    readln(f,line);
  }

  /* FINAL COMPUTATION */

  free(line);
  free(numline);
  free(varlist);

  return report;
}

/*-----------------------------------------------------------------------*/
/* TCL related functions                                                 */
/*-----------------------------------------------------------------------*/

/**Function****************************************************************
  Synopsis    [Function Bddscoutbddtraces_Init.]
  Description [This function is used by Tcl/Tk only.]
  SideEffects []
  SeeAlso     []
  ************************************************************************/

static int BddscoutRunBddTraceCmd(ClientData clientData, Tcl_Interp *interp,
                          int argc, USECONST char **argv);

#ifdef __cplusplus
extern "C" {
#endif

int
Bddscoutbddtraces_Init(Tcl_Interp *interp)
{

#ifdef USE_TCL_STUBS
  if (Tcl_InitStubs(interp, "8.1", 0) == NULL) {
    return TCL_ERROR;
  }
#endif

#ifdef USE_BDDSCOUT_STUBS
  if (Bddscout_InitStubs(interp, (char *) "1.0", 0) == NULL) {
    return TCL_ERROR;
  }
#endif

  Tcl_CreateCommand(interp, "bddscout_runBddTrace", BddscoutRunBddTraceCmd,
                     (ClientData) NULL, (Tcl_CmdDeleteProc *) NULL);

  return Tcl_PkgProvide(interp, "bddscoutBDDTRACES", "1.0");
}

#ifdef __cplusplus
}
#endif

static int
BddscoutRunBddTraceCmd(ClientData clientData, Tcl_Interp *interp, int argc,
                    USECONST char **argv)
{
  Biddy_String s1;
  FILE *funfile;
  Biddy_String report;

  if (argc != 2) {
    Tcl_SetResult(interp, (char *) "wrong # args", TCL_STATIC);
    return TCL_ERROR;
  }

  s1 = strdup(argv[1]);

  funfile = fopen(s1,"r");
  if (!funfile) {
    printf("BddscoutRunBddTraceCmd: File error (%s)!\n",s1);
    return TCL_ERROR;
  }

  report = Bddscout_RunBddTrace(funfile);

  fclose(funfile);

  free(s1);

  Tcl_SetResult(interp, report, TCL_VOLATILE);
  free(report);

  return TCL_OK;
}

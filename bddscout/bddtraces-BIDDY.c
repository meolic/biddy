/**CFile*******************************************************************
  PackageName [Bdd Scout]
  Synopsis    [Bdd Scout]

  FileName    [bddtraces-BIDDY.c]
  Revision    [$Revision: 692 $]
  Date        [$Date: 2024-06-30 18:06:54 +0200 (ned, 30 jun 2024) $]
  Authors     [Robert Meolic (robert@meolic.com)]
  Description []
  SeeAlso     [bddscout.h]

  Copyright   [This file is part of Bdd Scout package.
               Copyright (C) 2008, 2019 UM FERI, Koroska cesta 46, SI-2000 Maribor, Slovenia
               Copyright (C) 2019, 2024 Robert Meolic, SI-2000 Maribor, Slovenia

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

#include "time.h"

static unsigned int getNumber(Biddy_String name);

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

/*-----------------------------------------------------------------------*/
/* Functions                                                             */
/*-----------------------------------------------------------------------*/

/**Function****************************************************************
  Synopsis    [Function BddscoutRunBddTrace]
  Description []
  SideEffects []
  SeeAlso     []
  ************************************************************************/

#define readln(f,s) SCAN_RESULT = fscanf(f," %[^\n] ",s)
int SCAN_RESULT; /* used in readln macro */

Biddy_String
BddscoutRunBddTrace(FILE *f)
{
  Biddy_Manager MNG;
  Biddy_String report;
  Biddy_String line,line1,line2,line3,numline;
  Biddy_Boolean quote;
  Biddy_String s1;
  Biddy_String var,op1,op2,op3;
  Biddy_Variable v;
  Biddy_Edge* varlist;
  Biddy_Edge* lvarlist;
  unsigned int len;
  unsigned int varnum,lvarnum;
  unsigned int i,j;
  unsigned int n;
  Biddy_Edge bdd1,bdd2,bdd3,bdd;
  int result,resultPlain,correct;
  clock_t startTime,endTime;
  double elapsedTime,statTime;

  MNG = Bddscout_GetActiveManager();

  report = strdup("");

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
    len = (unsigned int)strlen(s1)+(unsigned int)strlen(line)+1;
    s1 = (char *) realloc(s1, sizeof(char) * len);
    strcat(s1,line);
    readln(f,line);
  }

  varlist = (Biddy_Edge *) malloc(sizeof(Biddy_Edge));
  varlist[0] = BDDNULL;
  varnum = 1;

  lvarlist = (Biddy_Edge *) malloc(sizeof(Biddy_Edge));
  lvarlist[0] = BDDNULL;
  lvarnum = 1;

  startTime = clock();

  var = strtok(s1," \t,;");
  while (var) {

    if (var[0]!=',' && var[0]!=';') {

#ifdef DEBUG
      printf("VARIABLE: <%s>\n",var);
#endif

      /* THIS PART IS PROBABLY NOT CORRECT FOR ZBDDs AND ZFDDs! */
      v = Biddy_Managed_AddVariableByName(MNG,var);
      n = getNumber(var);
      if (varnum <= n) {
        varnum = n+1;
        varlist = (Biddy_Edge *) realloc(varlist,varnum*sizeof(Biddy_Edge));
      }
      varlist[n] = Biddy_Managed_GetVariableEdge(MNG,v);

    }
    var = strtok(NULL," \t,;");
  }

  endTime = clock();
  elapsedTime = (endTime - startTime) / (1.0 * CLOCKS_PER_SEC);

#ifdef DEBUG
  printf("INPUT VARIABLES ELAPSED TIME: %.2fs\n",elapsedTime);
#endif

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
    len = (unsigned int)strlen(s1)+(unsigned int)strlen(line)+1;
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
    len = (unsigned int)strlen(line);
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
      resultPlain = -1;

      bdd = BDDNULL;
      elapsedTime = 0.0;
      statTime = 0.0;

    } else if (!strncmp(line,T_CHECK_POINT_FOR_REORDER_OP,strlen(T_CHECK_POINT_FOR_REORDER_OP))) {

      correct = -1;

#ifdef DEBUG
      printf(" OK\n");
#endif

      result = -1;
      resultPlain = -1;

      elapsedTime = 0.0;
      statTime = 0.0;

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

        if (!strncmp("v",op1,1)) {
          bdd1 = varlist[getNumber(op1)];
        } else if (!strncmp("lv_",op1,3)) {
          bdd1 = lvarlist[getNumber(op1)];
        } else if (!strcmp("false",op1)) {
          bdd1 = Biddy_Managed_GetConstantZero(MNG);
        } else if (!strcmp("true",op1)) {
          bdd1 = Biddy_Managed_GetConstantOne(MNG);
        } else {
          bdd1 = BDDNULL;
          printf(" ERROR! WRONG VARIABLE NAME <%s>",op1);
        }

        if (!strncmp("v",op2,1)) {
          bdd2 = varlist[getNumber(op2)];
        } else if (!strncmp("lv_",op2,3)) {
          bdd2 = lvarlist[getNumber(op2)];
        } else if (!strcmp("false",op2)) {
          bdd2 = Biddy_Managed_GetConstantZero(MNG);
        } else if (!strcmp("true",op2)) {
          bdd2 = Biddy_Managed_GetConstantOne(MNG);
        } else {
          bdd2 = BDDNULL;
          printf(" ERROR! WRONG VARIABLE NAME <%s>",op2);
        }

        startTime = clock();

        if ((bdd1 != BDDNULL) && (bdd2 != BDDNULL))
        {
          if (bdd1 == bdd2) {
            result = 1;
            resultPlain = 1;
          } else {
            result = 0;
            resultPlain = 0;
          }
        } else {
          result = -1;
          resultPlain = -1;
        }

        endTime = clock();
        elapsedTime = (endTime - startTime) / (1.0 * CLOCKS_PER_SEC);
        statTime = 0.0;

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
          resultPlain = -1;

          elapsedTime = 0.0;
          statTime = 0.0;

        } else {

          line1[0]=0;
          var = strdup(line);
          line1[0]='=';
          s1 = strdup(&(line1[1]));

          if (!strncmp("lv_",var,3)) {
            n = getNumber(var);
            if (lvarnum <= n) {
              lvarnum = n+1;
              lvarlist = (Biddy_Edge *) realloc(lvarlist,lvarnum*sizeof(Biddy_Edge));
            }
          } else if (!strcmp("false",var)) {
            n = -1;
          } else if (!strcmp("true",var)) {
            n = -1;
          } else {
            n = -1;
            printf(" ERROR! WRONG VARIABLE NAME <%s>",var);
          }

#ifdef DEBUG
          printf(" ASSIGN <%s> = <%s>",var,s1);
#endif

          elapsedTime = 0.0;
          statTime = 0.0;

          if (!strncmp(s1,T_NEW_INT_LEAF_OP,strlen(T_NEW_INT_LEAF_OP))) {

            startTime = clock();

            if (s1[strlen(T_NEW_INT_LEAF_OP)+1] == '0') {
              bdd = Biddy_Managed_GetConstantZero(MNG);
              if (n != -1) {
                lvarlist[n] = bdd;
              }

#ifdef DEBUG
              printf(" OK\n");
#endif

            } else if (s1[strlen(T_NEW_INT_LEAF_OP)+1] == '1') {
              bdd = Biddy_Managed_GetConstantOne(MNG);
              if (n != -1) {
                lvarlist[n] = bdd;
              }

#ifdef DEBUG
              printf(" OK\n");
#endif

            } else {

#ifdef DEBUG
              printf(" UNSUPPORTED\n");
#endif

              bdd = BDDNULL;

            }

            endTime = clock();
            elapsedTime = (endTime - startTime) / (1.0 * CLOCKS_PER_SEC);

            startTime = clock();

            result = Biddy_Managed_CountNodes(MNG,bdd);
            resultPlain = Biddy_Managed_CountNodesPlain(MNG,bdd);

            endTime = clock();
            statTime = (endTime - startTime) / (1.0 * CLOCKS_PER_SEC);

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

            if (!strncmp("v",op1,1)) {
              bdd1 = varlist[getNumber(op1)];
            } else if (!strncmp("lv_",op1,3)) {
              bdd1 = lvarlist[getNumber(op1)];
            } else if (!strcmp("false",op1)) {
              bdd1 = Biddy_Managed_GetConstantZero(MNG);
            } else if (!strcmp("true",op1)) {
              bdd1 = Biddy_Managed_GetConstantOne(MNG);
            } else {
              bdd1 = BDDNULL;
              printf(" ERROR! WRONG VARIABLE NAME <%s>",op1);
            }

            if (!strncmp("v",op2,1)) {
              bdd2 = varlist[getNumber(op2)];
            } else if (!strncmp("lv_",op2,3)) {
              bdd2 = lvarlist[getNumber(op2)];
            } else if (!strcmp("false",op2)) {
              bdd2 = Biddy_Managed_GetConstantZero(MNG);
            } else if (!strcmp("true",op2)) {
              bdd2 = Biddy_Managed_GetConstantOne(MNG);
            } else {
              bdd2 = BDDNULL;
              printf(" ERROR! WRONG VARIABLE NAME <%s>",op2);
            }

            if (!strncmp("v",op3,1)) {
              bdd3 = varlist[getNumber(op3)];
            } else if (!strncmp("lv_",op3,3)) {
              bdd3 = lvarlist[getNumber(op3)];
            } else if (!strcmp("false",op3)) {
              bdd3 = Biddy_Managed_GetConstantZero(MNG);
            } else if (!strcmp("true",op3)) {
              bdd3 = Biddy_Managed_GetConstantOne(MNG);
            } else {
              bdd3 = BDDNULL;
              printf(" ERROR! WRONG VARIABLE NAME <%s>",op3);
            }

            startTime = clock();

            if ((bdd1 != BDDNULL) && (bdd2 != BDDNULL) && (bdd3 != BDDNULL))
            {
              bdd = Biddy_Managed_ITE(MNG,bdd1,bdd2,bdd3);
              if (n != -1) {
                lvarlist[n] = bdd;
              }
            } else {
              bdd = BDDNULL;
            }

            endTime = clock();
            elapsedTime = (endTime - startTime) / (1.0 * CLOCKS_PER_SEC);

#ifdef DEBUG
            printf(" OK\n");
#endif

            free(op1);
            free(op2);
            free(op3);

            startTime = clock();

            result = Biddy_Managed_CountNodes(MNG,bdd);
            resultPlain = Biddy_Managed_CountNodesPlain(MNG,bdd);

            endTime = clock();
            statTime = (endTime - startTime) / (1.0 * CLOCKS_PER_SEC);

          } else if (!strncmp(s1,T_VARS_CURR_TO_NEXT_OP,strlen(T_VARS_CURR_TO_NEXT_OP))) {

            line1 = strchr(&(s1[strlen(T_VARS_CURR_TO_NEXT_OP)+1]),')');
            line1[0]=0;
            op1 = strdup(&(s1[strlen(T_VARS_CURR_TO_NEXT_OP)+1]));
            line1[0]=')';

            if (!strncmp("v",op1,1)) {
              bdd1 = varlist[getNumber(op1)];
            } else if (!strncmp("lv_",op1,3)) {
              bdd1 = lvarlist[getNumber(op1)];
            } else if (!strcmp("false",op1)) {
              bdd1 = Biddy_Managed_GetConstantZero(MNG);
            } else if (!strcmp("true",op1)) {
              bdd1 = Biddy_Managed_GetConstantOne(MNG);
            } else {
              bdd1 = BDDNULL;
              printf(" ERROR! WRONG VARIABLE NAME <%s>",op1);
            }

            startTime = clock();

            if ((varnum > 1) && (bdd1 != BDDNULL))
            {
              bdd = bdd1;
              for (i=0;i<varnum;i=i+2) {
                bdd = Biddy_Managed_Compose(MNG,bdd,varlist[i+1],Biddy_Managed_GetTopVariable(MNG,varlist[i]));
              }
              if (n != -1) {
                lvarlist[n] = bdd;
              }
            } else {
              bdd = BDDNULL;
            }

            endTime = clock();
            elapsedTime = (endTime - startTime) / (1.0 * CLOCKS_PER_SEC);

#ifdef DEBUG
            printf(" OK\n");
#endif

            free(op1);

            startTime = clock();

            result = Biddy_Managed_CountNodes(MNG,bdd);
            resultPlain = Biddy_Managed_CountNodesPlain(MNG,bdd);

            endTime = clock();
            statTime = (endTime - startTime) / (1.0 * CLOCKS_PER_SEC);

          } else if (!strncmp(s1,T_VARS_NEXT_TO_CURR_OP,strlen(T_VARS_NEXT_TO_CURR_OP))) {

            line1 = strchr(&(s1[strlen(T_VARS_NEXT_TO_CURR_OP)+1]),')');
            line1[0]=0;
            op1 = strdup(&(s1[strlen(T_VARS_NEXT_TO_CURR_OP)+1]));
            line1[0]=')';

            if (!strncmp("v",op1,1)) {
              bdd1 = varlist[getNumber(op1)];
            } else if (!strncmp("lv_",op1,3)) {
              bdd1 = lvarlist[getNumber(op1)];
            } else if (!strcmp("false",op1)) {
              bdd1 = Biddy_Managed_GetConstantZero(MNG);
            } else if (!strcmp("true",op1)) {
              bdd1 = Biddy_Managed_GetConstantOne(MNG);
            } else {
              bdd1 = BDDNULL;
              printf(" ERROR! WRONG VARIABLE NAME <%s>",op1);
            }

            startTime = clock();

            if ((varnum > 1) && (bdd1 != BDDNULL))
            {
              bdd = bdd1;
              for (i=0;i<varnum;i=i+2) {
                bdd = Biddy_Managed_Compose(MNG,bdd,varlist[i],Biddy_Managed_GetTopVariable(MNG,varlist[i+1]));
              }
              if (n != -1) {
                lvarlist[n] = bdd;
              }
            } else {
              bdd = BDDNULL;
            }

            endTime = clock();
            elapsedTime = (endTime - startTime) / (1.0 * CLOCKS_PER_SEC);

#ifdef DEBUG
            printf(" OK\n");
#endif

            free(op1);

            startTime = clock();

            result = Biddy_Managed_CountNodes(MNG,bdd);
            resultPlain = Biddy_Managed_CountNodesPlain(MNG,bdd);

            endTime = clock();
            statTime = (endTime - startTime) / (1.0 * CLOCKS_PER_SEC);

          } else if (!strncmp(s1,T_SUPPORT_VARS_OP,strlen(T_SUPPORT_VARS_OP))) {

            line1 = strchr(&(s1[strlen(T_SUPPORT_VARS_OP)+1]),')');
            line1[0]=0;
            op1 = strdup(&(s1[strlen(T_SUPPORT_VARS_OP)+1]));
            line1[0]=')';

            if (!strncmp("v",op1,1)) {
              bdd1 = varlist[getNumber(op1)];
            } else if (!strncmp("lv_",op1,3)) {
              bdd1 = lvarlist[getNumber(op1)];
            } else if (!strcmp("false",op1)) {
              bdd1 = Biddy_Managed_GetConstantZero(MNG);
            } else if (!strcmp("true",op1)) {
              bdd1 = Biddy_Managed_GetConstantOne(MNG);
            } else {
              bdd1 = BDDNULL;
              printf(" ERROR! WRONG VARIABLE NAME <%s>",op1);
            }

            startTime = clock();

            if (bdd1 != BDDNULL)
            {
              bdd = Biddy_Managed_Support(MNG,bdd1);
              if (n != -1) {
                lvarlist[n] = bdd;
              }
            } else {
              bdd = BDDNULL;
            }

            endTime = clock();
            elapsedTime = (endTime - startTime) / (1.0 * CLOCKS_PER_SEC);

#ifdef DEBUG
            printf(" OK\n");
#endif

            free(op1);

            startTime = clock();

            result = Biddy_Managed_CountNodes(MNG,bdd);
            resultPlain = Biddy_Managed_CountNodesPlain(MNG,bdd);

            endTime = clock();
            statTime = (endTime - startTime) / (1.0 * CLOCKS_PER_SEC);

          } else if (!strncmp(s1,T_EXISTS_OP,strlen(T_EXISTS_OP))) {

            line1 = strchr(&(s1[strlen(T_EXISTS_OP)+1]),',');
            line1[0]=0;
            op1 = strdup(&(s1[strlen(T_EXISTS_OP)+1]));
            line1[0]=',';
            line2 = strchr(&(line1[1]),')');
            line2[0]=0;
            op2 = strdup(&(line1[1]));
            line2[0]=')';

            if (!strncmp("v",op1,1)) {
              bdd1 = varlist[getNumber(op1)];
            } else if (!strncmp("lv_",op1,3)) {
              bdd1 = lvarlist[getNumber(op1)];
            } else if (!strcmp("false",op1)) {
              bdd1 = Biddy_Managed_GetConstantZero(MNG);
            } else if (!strcmp("true",op1)) {
              bdd1 = Biddy_Managed_GetConstantOne(MNG);
            } else {
              bdd1 = BDDNULL;
              printf(" ERROR! WRONG VARIABLE NAME <%s>",op1);
            }

            if (!strncmp("v",op2,1)) {
              bdd2 = varlist[getNumber(op2)];
            } else if (!strncmp("lv_",op2,3)) {
              bdd2 = lvarlist[getNumber(op2)];
            } else if (!strcmp("false",op2)) {
              bdd2 = Biddy_Managed_GetConstantZero(MNG);
            } else if (!strcmp("true",op2)) {
              bdd2 = Biddy_Managed_GetConstantOne(MNG);
            } else {
              bdd2 = BDDNULL;
              printf(" ERROR! WRONG VARIABLE NAME <%s>",op2);
            }

            startTime = clock();

            if ((bdd1 != BDDNULL) && (bdd2 != BDDNULL))
            {
              bdd = Biddy_Managed_ExistAbstract(MNG,bdd1,bdd2);
              if (n != -1) {
                lvarlist[n] = bdd;
              }
            } else {
              bdd = BDDNULL;
            }

            endTime = clock();
            elapsedTime = (endTime - startTime) / (1.0 * CLOCKS_PER_SEC);

#ifdef DEBUG
            printf(" OK\n");
#endif

            free(op1);
            free(op2);

            startTime = clock();

            result = Biddy_Managed_CountNodes(MNG,bdd);
            resultPlain = Biddy_Managed_CountNodesPlain(MNG,bdd);

            endTime = clock();
            statTime = (endTime - startTime) / (1.0 * CLOCKS_PER_SEC);

          } else if (!strncmp(s1,T_NOT_OP,strlen(T_NOT_OP))) {

            line1 = strchr(&(s1[strlen(T_NOT_OP)+1]),')');
            line1[0]=0;
            op1 = strdup(&(s1[strlen(T_NOT_OP)+1]));
            line1[0]=')';

            if (!strncmp("v",op1,1)) {
              bdd1 = varlist[getNumber(op1)];
            } else if (!strncmp("lv_",op1,3)) {
              bdd1 = lvarlist[getNumber(op1)];
            } else if (!strcmp("false",op1)) {
              bdd1 = Biddy_Managed_GetConstantZero(MNG);
            } else if (!strcmp("true",op1)) {
              bdd1 = Biddy_Managed_GetConstantOne(MNG);
            } else {
              bdd1 = BDDNULL;
              printf(" ERROR! WRONG VARIABLE NAME <%s>",op1);
            }

            startTime = clock();

            if (bdd1 != BDDNULL)
            {
              bdd = Biddy_Managed_Not(MNG,bdd1);
              if (n != -1) {
                lvarlist[n] = bdd;
              }
            } else {
              bdd = BDDNULL;
            }

            endTime = clock();
            elapsedTime = (endTime - startTime) / (1.0 * CLOCKS_PER_SEC);

#ifdef DEBUG
            printf(" OK\n");
#endif

            free(op1);

            startTime = clock();

            result = Biddy_Managed_CountNodes(MNG,bdd);
            resultPlain = Biddy_Managed_CountNodesPlain(MNG,bdd);

            endTime = clock();
            statTime = (endTime - startTime) / (1.0 * CLOCKS_PER_SEC);

          } else if (!strncmp(s1,T_AND_OP,strlen(T_AND_OP))) {

            line1 = strchr(&(s1[strlen(T_AND_OP)+1]),',');
            line1[0]=0;
            op1 = strdup(&(s1[strlen(T_AND_OP)+1]));
            line1[0]=',';
            line2 = strchr(&(line1[1]),')');
            line2[0]=0;
            op2 = strdup(&(line1[1]));
            line2[0]=')';

            if (!strncmp("v",op1,1)) {
              bdd1 = varlist[getNumber(op1)];
            } else if (!strncmp("lv_",op1,3)) {
              bdd1 = lvarlist[getNumber(op1)];
            } else if (!strcmp("false",op1)) {
              bdd1 = Biddy_Managed_GetConstantZero(MNG);
            } else if (!strcmp("true",op1)) {
              bdd1 = Biddy_Managed_GetConstantOne(MNG);
            } else {
              bdd1 = BDDNULL;
              printf(" ERROR! WRONG VARIABLE NAME <%s>",op1);
            }

            if (!strncmp("v",op2,1)) {
              bdd2 = varlist[getNumber(op2)];
            } else if (!strncmp("lv_",op2,3)) {
              bdd2 = lvarlist[getNumber(op2)];
            } else if (!strcmp("false",op2)) {
              bdd2 = Biddy_Managed_GetConstantZero(MNG);
            } else if (!strcmp("true",op2)) {
              bdd2 = Biddy_Managed_GetConstantOne(MNG);
            } else {
              bdd2 = BDDNULL;
              printf(" ERROR! WRONG VARIABLE NAME <%s>",op2);
            }

            startTime = clock();

            if ((bdd1 != BDDNULL) && (bdd2 != BDDNULL))
            {
              bdd = Biddy_Managed_And(MNG,bdd1,bdd2);
              if (n != -1) {
                lvarlist[n] = bdd;
              }
            } else {
              bdd = BDDNULL;
            }

            endTime = clock();
            elapsedTime = (endTime - startTime) / (1.0 * CLOCKS_PER_SEC);

#ifdef DEBUG
            printf(" OK\n");
#endif

            free(op1);
            free(op2);

            startTime = clock();

            result = Biddy_Managed_CountNodes(MNG,bdd);
            resultPlain = Biddy_Managed_CountNodesPlain(MNG,bdd);

            endTime = clock();
            statTime = (endTime - startTime) / (1.0 * CLOCKS_PER_SEC);

          } else if (!strncmp(s1,T_OR_OP,strlen(T_OR_OP))) {

            line1 = strchr(&(s1[strlen(T_OR_OP)+1]),',');
            line1[0]=0;
            op1 = strdup(&(s1[strlen(T_OR_OP)+1]));
            line1[0]=',';
            line2 = strchr(&(line1[1]),')');
            line2[0]=0;
            op2 = strdup(&(line1[1]));
            line2[0]=')';

            if (!strncmp("v",op1,1)) {
              bdd1 = varlist[getNumber(op1)];
            } else if (!strncmp("lv_",op1,3)) {
              bdd1 = lvarlist[getNumber(op1)];
            } else if (!strcmp("false",op1)) {
              bdd1 = Biddy_Managed_GetConstantZero(MNG);
            } else if (!strcmp("true",op1)) {
              bdd1 = Biddy_Managed_GetConstantOne(MNG);
            } else {
              bdd1 = BDDNULL;
              printf(" ERROR! WRONG VARIABLE NAME <%s>",op1);
            }

            if (!strncmp("v",op2,1)) {
              bdd2 = varlist[getNumber(op2)];
            } else if (!strncmp("lv_",op2,3)) {
              bdd2 = lvarlist[getNumber(op2)];
            } else if (!strcmp("false",op2)) {
              bdd2 = Biddy_Managed_GetConstantZero(MNG);
            } else if (!strcmp("true",op2)) {
              bdd2 = Biddy_Managed_GetConstantOne(MNG);
            } else {
              bdd2 = BDDNULL;
              printf(" ERROR! WRONG VARIABLE NAME <%s>",op2);
            }

            startTime = clock();

            if ((bdd1 != BDDNULL) && (bdd2 != BDDNULL))
            {
              bdd = Biddy_Managed_Or(MNG,bdd1,bdd2);
              if (n != -1) {
                lvarlist[n] = bdd;
              }
            } else {
              bdd = BDDNULL;
            }

            endTime = clock();
            elapsedTime = (endTime - startTime) / (1.0 * CLOCKS_PER_SEC);

#ifdef DEBUG
            printf(" OK\n");
#endif

            free(op1);
            free(op2);

            startTime = clock();

            result = Biddy_Managed_CountNodes(MNG,bdd);
            resultPlain = Biddy_Managed_CountNodesPlain(MNG,bdd);

            endTime = clock();
            statTime = (endTime - startTime) / (1.0 * CLOCKS_PER_SEC);

          } else if (!strncmp(s1,T_XOR_OP,strlen(T_XOR_OP))) {

            line1 = strchr(&(s1[strlen(T_XOR_OP)+1]),',');
            line1[0]=0;
            op1 = strdup(&(s1[strlen(T_XOR_OP)+1]));
            line1[0]=',';
            line2 = strchr(&(line1[1]),')');
            line2[0]=0;
            op2 = strdup(&(line1[1]));
            line2[0]=')';

            if (!strncmp("v",op1,1)) {
              bdd1 = varlist[getNumber(op1)];
            } else if (!strncmp("lv_",op1,3)) {
              bdd1 = lvarlist[getNumber(op1)];
            } else if (!strcmp("false",op1)) {
              bdd1 = Biddy_Managed_GetConstantZero(MNG);
            } else if (!strcmp("true",op1)) {
              bdd1 = Biddy_Managed_GetConstantOne(MNG);
            } else {
              bdd1 = BDDNULL;
              printf(" ERROR! WRONG VARIABLE NAME <%s>",op1);
            }

            if (!strncmp("v",op2,1)) {
              bdd2 = varlist[getNumber(op2)];
            } else if (!strncmp("lv_",op2,3)) {
              bdd2 = lvarlist[getNumber(op2)];
            } else if (!strcmp("false",op2)) {
              bdd2 = Biddy_Managed_GetConstantZero(MNG);
            } else if (!strcmp("true",op2)) {
              bdd2 = Biddy_Managed_GetConstantOne(MNG);
            } else {
              bdd2 = BDDNULL;
              printf(" ERROR! WRONG VARIABLE NAME <%s>",op2);
            }

            startTime = clock();

            if ((bdd1 != BDDNULL) && (bdd2 != BDDNULL))
            {
              bdd = Biddy_Managed_Xor(MNG,bdd1,bdd2);
              if (n != -1) {
                lvarlist[n] = bdd;
              }
            } else {
              bdd = BDDNULL;
            }

            endTime = clock();
            elapsedTime = (endTime - startTime) / (1.0 * CLOCKS_PER_SEC);

#ifdef DEBUG
            printf(" OK\n");
#endif

            free(op1);
            free(op2);

            startTime = clock();

            result = Biddy_Managed_CountNodes(MNG,bdd);
            resultPlain = Biddy_Managed_CountNodesPlain(MNG,bdd);

            endTime = clock();
            statTime = (endTime - startTime) / (1.0 * CLOCKS_PER_SEC);

          } else if (!strncmp(s1,T_REDUCE_OP,strlen(T_REDUCE_OP))) {

            line1 = strchr(&(s1[strlen(T_REDUCE_OP)+1]),',');
            line1[0]=0;
            op1 = strdup(&(s1[strlen(T_REDUCE_OP)+1]));
            line1[0]=',';
            line2 = strchr(&(line1[1]),')');
            line2[0]=0;
            op2 = strdup(&(line1[1]));
            line2[0]=')';

            if (!strncmp("v",op1,1)) {
              bdd1 = varlist[getNumber(op1)];
            } else if (!strncmp("lv_",op1,3)) {
              bdd1 = lvarlist[getNumber(op1)];
            } else if (!strcmp("false",op1)) {
              bdd1 = Biddy_Managed_GetConstantZero(MNG);
            } else if (!strcmp("true",op1)) {
              bdd1 = Biddy_Managed_GetConstantOne(MNG);
            } else {
              bdd1 = BDDNULL;
              printf(" ERROR! WRONG VARIABLE NAME <%s>",op1);
            }

            if (!strncmp("v",op2,1)) {
              bdd2 = varlist[getNumber(op2)];
            } else if (!strncmp("lv_",op2,3)) {
              bdd2 = lvarlist[getNumber(op2)];
            } else if (!strcmp("false",op2)) {
              bdd2 = Biddy_Managed_GetConstantZero(MNG);
            } else if (!strcmp("true",op2)) {
              bdd2 = Biddy_Managed_GetConstantOne(MNG);
            } else {
              bdd2 = BDDNULL;
              printf(" ERROR! WRONG VARIABLE NAME <%s>",op2);
            }

            startTime = clock();

            if ((bdd1 != BDDNULL) && (bdd2 != BDDNULL))
            {
              bdd = Biddy_Managed_Simplify(MNG,bdd1,bdd2);
              if (n != -1) {
                lvarlist[n] = bdd;
              }
            } else {
              bdd = BDDNULL;
            }

            endTime = clock();
            elapsedTime = (endTime - startTime) / (1.0 * CLOCKS_PER_SEC);

#ifdef DEBUG
            printf(" OK\n");
#endif

            free(op1);
            free(op2);

            startTime = clock();

            result = Biddy_Managed_CountNodes(MNG,bdd);
            resultPlain = Biddy_Managed_CountNodesPlain(MNG,bdd);

            endTime = clock();
            statTime = (endTime - startTime) / (1.0 * CLOCKS_PER_SEC);

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

            if (!strncmp("v",op1,1)) {
              bdd1 = varlist[getNumber(op1)];
            } else if (!strncmp("lv_",op1,3)) {
              bdd1 = lvarlist[getNumber(op1)];
            } else if (!strcmp("false",op1)) {
              bdd1 = Biddy_Managed_GetConstantZero(MNG);
            } else if (!strcmp("true",op1)) {
              bdd1 = Biddy_Managed_GetConstantOne(MNG);
            } else {
              bdd1 = BDDNULL;
              printf(" ERROR! WRONG VARIABLE NAME <%s>",op1);
            }

            if (!strncmp("v",op2,1)) {
              bdd2 = varlist[getNumber(op2)];
            } else if (!strncmp("lv_",op2,3)) {
              bdd2 = lvarlist[getNumber(op2)];
            } else if (!strcmp("false",op2)) {
              bdd2 = Biddy_Managed_GetConstantZero(MNG);
            } else if (!strcmp("true",op2)) {
              bdd2 = Biddy_Managed_GetConstantOne(MNG);
            } else {
              bdd2 = BDDNULL;
              printf(" ERROR! WRONG VARIABLE NAME <%s>",op2);
            }

            if (!strncmp("v",op3,1)) {
              bdd3 = varlist[getNumber(op3)];
            } else if (!strncmp("lv_",op3,3)) {
              bdd3 = lvarlist[getNumber(op3)];
            } else if (!strcmp("false",op3)) {
              bdd3 = Biddy_Managed_GetConstantZero(MNG);
            } else if (!strcmp("true",op3)) {
              bdd3 = Biddy_Managed_GetConstantOne(MNG);
            } else {
              bdd3 = BDDNULL;
              printf(" ERROR! WRONG VARIABLE NAME <%s>",op3);
            }

            startTime = clock();

            if ((bdd1 != BDDNULL) && (bdd2 != BDDNULL) && (bdd3 != BDDNULL))
            {

              /* DEBUGGING */
              /*
              printf("Biddy_Managed_ExistAndAbstract: Function f has %u nodes.\n",Biddy_Managed_CountNodes(MNG,bdd2));
              printf("Biddy_Managed_ExistAndAbstract: Function g has %u nodes.\n",Biddy_Managed_CountNodes(MNG,bdd3));
              printf("Biddy_Managed_ExistAndAbstract: Function cube has %u nodes.\n",Biddy_Managed_CountNodes(MNG,bdd1));
              */

              bdd = Biddy_Managed_ExistAndAbstract(MNG,bdd2,bdd3,bdd1);
              if (n != -1) {
                lvarlist[n] = bdd;
              }
            } else {
              bdd = BDDNULL;
            }

            endTime = clock();
            elapsedTime = (endTime - startTime) / (1.0 * CLOCKS_PER_SEC);

#ifdef DEBUG
            printf(" OK\n");
#endif

            free(op1);
            free(op2);
            free(op3);

            startTime = clock();

            result = Biddy_Managed_CountNodes(MNG,bdd);
            resultPlain = Biddy_Managed_CountNodesPlain(MNG,bdd);

            endTime = clock();
            statTime = (endTime - startTime) / (1.0 * CLOCKS_PER_SEC);

          } else {

#ifdef DEBUG
            printf(" NOT IMPLEMENTED\n");
#endif

            result = -1;
            resultPlain = -1;

            bdd = BDDNULL;
            elapsedTime = 0.0;
            statTime = 0.0;

          }

          /* SAVE IN FORMULA TABLE */
          if (bdd != BDDNULL) Biddy_Managed_AddPersistentFormula(MNG,var,bdd);

          /* DEBUGGING: REPORT ONE RESULT IN DOT FORMAT */
          /*
          if ((bdd != BDDNULL) &&
              ((n == 0) || (n == 0) || (n == 0)))
          {
            Biddy_Managed_WriteDot(MNG,var,bdd,var);
          }
          */

          free(var);
          free(s1);
        }

      }
    }

    /* PREPARE REPORT */

    len = (unsigned int)strlen(report)+(unsigned int)strlen(line)+2;
    report = (Biddy_String) realloc(report,len);
    strcat(report,line);
    strcat(report,"\t");

    sprintf(numline,"%d",correct);
    len = (unsigned int)strlen(report)+(unsigned int)strlen(numline)+2;
    report = (Biddy_String) realloc(report,len);
    strcat(report,numline);
    strcat(report,"\t");

    sprintf(numline,"%.2fs",elapsedTime);
    len = (unsigned int)strlen(report)+(unsigned int)strlen(numline)+2;
    report = (Biddy_String) realloc(report,len);
    strcat(report,numline);
    strcat(report,"\t");

    sprintf(numline,"%.2fs",statTime);
    len = (unsigned int)strlen(report)+(unsigned int)strlen(numline)+2;
    report = (Biddy_String) realloc(report,len);
    strcat(report,numline);
    strcat(report,"\t");

    sprintf(numline,"%d",result);
    len = (unsigned int)strlen(report)+(unsigned int)strlen(numline)+2;
    report = (Biddy_String) realloc(report,len);
    strcat(report,numline);
    strcat(report,"\t");

    /* ENABLE EASY COMPARISON OF BIDDY AND CUDD RESULTS */
    /*
    if (resultPlain>1) resultPlain = -1;
    */

    sprintf(numline,"%d",resultPlain);
    len = (unsigned int)strlen(report)+(unsigned int)strlen(numline)+2;
    report = (Biddy_String) realloc(report,len);
    strcat(report,numline);
    strcat(report,"\n");

    readln(f,line);
  }

  /* FINAL COMPUTATION */

  free(line);
  free(numline);
  free(varlist);
  free(lvarlist);

  return report;
}

static unsigned int getNumber(Biddy_String name)
{
  unsigned int x,r;

  x = (unsigned int)strcspn(name,"0123456789");
  sscanf(&name[x],"%u",&r);

  return r;
}

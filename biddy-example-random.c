/* $Revision: 619 $ */
/* $Date: 2020-03-28 15:19:50 +0100 (sob, 28 mar 2020) $ */
/* This file (biddy-example-random.c) is a C file */
/* Author: Robert Meolic (robert@meolic.com) */
/* This file has been released into the public domain by the author. */

/* This example is compatible with Biddy v2.0 */

/* COMPILE WITH (ADD -lgmp IF USING STATIC BIDDY LIBRARY): */
/* gcc -DUNIX -O2 -o biddy-example-random biddy-example-random.c -I. -L./bin -lbiddy */

/* this example creates and converts random BDDs */

#include "biddy.h"

#define BOOLEANFUNCTION 1
#define COMBINATIONSET 2

/* (POPULATION == -2): create one random function, only */
/* (POPULATION == -1): create one random function and perform tests */
/* (POPULATION == 0): not allowed */
/* (POPULATION > 0): create many random function and get statistics */
#define POPULATION -1

#define SIZE 8
#define RATIO 0.5

/* create Boolean function (BOOLEANFUNCTION) or combination set (COMBINATIONSET) */
#define RANDOMTYPE BOOLEANFUNCTION

/* choose primary BDD type */
#define BDDTYPE BIDDYTYPEOBDD

#define PRINTCOMBINATIONS(MNG,X) \
{\
printf("%s has %u nodes (including terminals) and %.0f cubes\n",Biddy_Managed_GetManagerName(MNG),Biddy_Managed_CountNodes(MNG,X),Biddy_Managed_CountCombinations(MNG,X));\
Biddy_Managed_PrintfMinterms(MNG,X,FALSE);\
}

int main() {
  Biddy_Manager MNGOBDD,MNGOBDDC,MNGZBDD,MNGZBDDC,MNGTZBDD;
  Biddy_Edge tmp,support,result,coresult,result0,result1,result2;
  Biddy_Edge cube,cuberobdd,cuberobddc,cubezbdd,cubezbddc,cubetzbdd;
  Biddy_Edge robdd,robddc,rzbdd,rzbddc,rtzbdd;
  Biddy_Variable v,first,second,last,extra;
  unsigned int i;
  unsigned int s1,s2,s3;

  unsigned int obddsize,zbddsize,tzbddsize;
  unsigned int obddminsize,zbddminsize,tzbddminsize;
  unsigned int obddmaxsize,zbddmaxsize,tzbddmaxsize;
  unsigned int obddsmallest,zbddsmallest,tzbddsmallest;
  unsigned int obddlargest,zbddlargest,tzbddlargest;
  unsigned int allsamesize;

  if (POPULATION == 0) {
    exit(1);
  }

  if ((POPULATION == -1) || (POPULATION == -2)) {

    /* ************************************************************* */
    /* START OF CREATION */
    /* ************************************************************* */

    Biddy_InitAnonymous(BDDTYPE);

    Biddy_InitMNG(&MNGOBDD,BIDDYTYPEOBDD);
    Biddy_InitMNG(&MNGOBDDC,BIDDYTYPEOBDDC);
    Biddy_InitMNG(&MNGZBDD,BIDDYTYPEZBDD);
    Biddy_InitMNG(&MNGZBDDC,BIDDYTYPEZBDDC);
    Biddy_InitMNG(&MNGTZBDD,BIDDYTYPETZBDD);

    printf("Using %s...\n",Biddy_GetManagerName());

    second = 0;

    /* for OBDDs, support is the most easiest calculated using AddVariable and AND */
    if ((Biddy_GetManagerType() == BIDDYTYPEOBDD) || (Biddy_GetManagerType() == BIDDYTYPEOBDDC)) {
      tmp = Biddy_AddVariableEdge();
      support = tmp;
      first = Biddy_GetTopVariable(tmp);
      for (v=1; v<SIZE; v++) {
        tmp = Biddy_AddVariableEdge();
        support = Biddy_And(support,tmp);
        if (v == 1) second = Biddy_GetTopVariable(tmp);
      }
      last = Biddy_GetTopVariable(tmp);
    }

    /* for ZBDDs, support is the most easiest calculated using AddElement and CHANGE */
    if ((Biddy_GetManagerType() == BIDDYTYPEZBDD) || (Biddy_GetManagerType() == BIDDYTYPEZBDDC)) {
      tmp = Biddy_GetElementEdge(Biddy_AddElement());
      support = tmp;
      first = Biddy_GetTopVariable(tmp);
      for (v=1; v<SIZE; v++) {
        tmp = Biddy_GetElementEdge(Biddy_AddElement());
        support = Biddy_Change(support,Biddy_GetTopVariable(tmp));
        if (v == 1) second = Biddy_GetTopVariable(tmp);
      }
      last = Biddy_GetTopVariable(tmp);
    }

    /* for TZBDDs, the approaches from OBDDs and ZBDDs are both working, using the first one */
    if ((Biddy_GetManagerType() == BIDDYTYPETZBDD) || (Biddy_GetManagerType() == BIDDYTYPETZBDDC)) {
      tmp = Biddy_AddVariableEdge();
      support = tmp;
      first = Biddy_GetTopVariable(tmp);
      for (v=1; v<SIZE; v++) {
        tmp = Biddy_AddVariableEdge();
        support = Biddy_And(support,tmp);
        if (v == 1) second = Biddy_GetTopVariable(tmp);
      }
      last = Biddy_GetTopVariable(tmp);
    }

    /* set name for some variables - to suppport debuging */

    Biddy_ChangeVariableName(first,"first");
    Biddy_ChangeVariableName(second,"second");
    Biddy_ChangeVariableName(last,"last");

    /* initialize all BDD managers */
    Biddy_Copy(MNGOBDD,Biddy_GetConstantZero());
    Biddy_Copy(MNGOBDDC,Biddy_GetConstantZero());
    Biddy_Copy(MNGZBDD,Biddy_GetConstantZero());
    Biddy_Copy(MNGZBDDC,Biddy_GetConstantZero());
    Biddy_Copy(MNGTZBDD,Biddy_GetConstantZero());

    /* DEBUGGING */
    /**/
    printf("Variable first = %s (%u)\n",Biddy_GetVariableName(first),first);
    printf("Variable second = %s (%u) \n",Biddy_GetVariableName(second),second);
    printf("Variable last = %s (%u)\n",Biddy_GetVariableName(last),last);
    printf("Variable first (OBDD) = %s (%u)\n",Biddy_Managed_GetVariableName(MNGOBDD,first),first);
    printf("Variable second (OBDD) = %s (%u) \n",Biddy_Managed_GetVariableName(MNGOBDD,second),second);
    printf("Variable last (OBDD) = %s (%u)\n",Biddy_Managed_GetVariableName(MNGOBDD,last),last);
    printf("Variable first (ZBDD) = %s (%u)\n",Biddy_Managed_GetVariableName(MNGZBDD,first),first);
    printf("Variable second (ZBDD) = %s (%u) \n",Biddy_Managed_GetVariableName(MNGZBDD,second),second);
    printf("Variable last (ZBDD) = %s (%u)\n",Biddy_Managed_GetVariableName(MNGZBDD,last),last);
    printf("Variable first (TZBDD) = %s (%u)\n",Biddy_Managed_GetVariableName(MNGTZBDD,first),first);
    printf("Variable second (TZBDD) = %s (%u) \n",Biddy_Managed_GetVariableName(MNGTZBDD,second),second);
    printf("Variable last (TZBDD) = %s (%u)\n",Biddy_Managed_GetVariableName(MNGTZBDD,last),last);
    /**/

    /* GRAPHVIZ/DOT OUTPUT OF support - DEBUGGING, ONLY */
    /*
    if (SIZE < 10) {
      Biddy_WriteDot("support.dot",support,"support",-1,FALSE);
      printf("USE 'dot -y -Tpng -O support.dot' to visualize function support.\n");
    }
    */

    /* GENERATE RANDOM FUNCTION OR COMBINATION SET */

    result = NULL;

#if (RANDOMTYPE == BOOLEANFUNCTION)
    printf("Generating random Boolean function...\n");
    result = Biddy_RandomFunction(support,RATIO);
#endif

#if (RANDOMTYPE == COMBINATIONSET)
    printf("Generating random combination set...\n");
    result = Biddy_RandomSet(support,RATIO);
#endif

    if (result == NULL) {
      printf("ERROR: Function result is NULL!\n");
      exit(1);
    }

    /* GRAPHVIZ/DOT OUTPUT OF result - DEBUGGING, ONLY */
    /*
    if (SIZE < 10) {
      Biddy_WriteDot("result.dot", result, "result", -1, FALSE);
      printf("USE 'dot -y -Tpng -O result.dot' to visualize function result.\n");
    }
    */

    /* TRUTH TABLE FOR result  - DEBUGGING, ONLY */
    /*
    if (SIZE < 10) {
      printf("HERE IS A TRUTH TABLE FOR result:\n");
      Biddy_PrintfTable(result);
    }
    */

    printf("Function result has %.0f minterms/combinations.\n",Biddy_CountMinterms(result,SIZE));
    printf("Function result has density = %.2e.\n",Biddy_DensityOfFunction(result,SIZE));

    /*
    printf("Function represented by %s depends on %u variables.\n",Biddy_GetManagerName(),Biddy_DependentVariableNumber(result,FALSE));
    printf("%s for function result has %u nodes (including terminals).\n",Biddy_GetManagerName(),Biddy_CountNodes(result));
    printf("%s for function result has %u plain nodes (including terminals).\n",Biddy_GetManagerName(),Biddy_CountNodesPlain(result));
    printf("%s for function result has %llu one-paths.\n",Biddy_GetManagerName(),Biddy_CountPaths(result));
    */

    if (POPULATION == -2) exit(1);

    /* ************************************************************* */
    /* START OF TESTING OPERATIONS */
    /* ************************************************************* */

    printf("********* BASIC TESTS *********\n");

    /* OPERATION NOT */

    result0 = Biddy_Not(Biddy_Not(result));

    if (result0 != result) {
      printf("ERROR: Operation NOT is wrong!\n");
    }

    /* GRAPHVIZ/DOT OUTPUT OF RESULT0 AND RESULT1  - DEBUGGING, ONLY */
    /*
    if (SIZE < 10) {
      Biddy_WriteDot("result0.dot",result0,"result0",-1,FALSE);
      printf("USE 'dot -y -Tpng -O result0.dot' to visualize function result0.\n");
      exit(1);
    }
    */

    /* OPERATIONS AND/OR */

    result0 = Biddy_And(result,support);
    result1 = Biddy_Not(Biddy_Or(Biddy_Not(result),Biddy_Not(support)));

    if (result0 != result1) {
      printf("ERROR: Operations AND/OR are wrong!\n");
    }

    /* GRAPHVIZ/DOT OUTPUT OF RESULT0 AND RESULT1  - DEBUGGING, ONLY */
    /*
    if (SIZE < 10) {
      Biddy_WriteDot("result0.dot",result0,"result0",-1,FALSE);
      printf("USE 'dot -y -Tpng -O result0.dot' to visualize function result0.\n");
      Biddy_WriteDot("result1.dot",result1,"result1",-1,FALSE);
      printf("USE 'dot -y -Tpng -O result1.dot' to visualize function result1.\n");
      exit(1);
    }
    */

    /* OPERATIONS ITE/XOR */

    result0 = Biddy_ITE(result,Biddy_Not(support),support);
    result1 = Biddy_Xor(result,support);

    if (result0 != result1) {
      printf("ERROR: Operations ITE and/or XOR are wrong!\n");
    }

    /* GRAPHVIZ/DOT OUTPUT OF RESULT0 AND RESULT1  - DEBUGGING, ONLY */
    /*
    if (SIZE < 10) {
      Biddy_WriteDot("result.dot",result,"result",-1,FALSE);
      printf("USE 'dot -y -Tpng -O result.dot' to visualize function result.\n");
      Biddy_WriteDot("support.dot",support,"support",-1,FALSE);
      printf("USE 'dot -y -Tpng -O support.dot' to visualize function support.\n");
      Biddy_WriteDot("result0.dot",result0,"result0",-1,FALSE);
      printf("USE 'dot -y -Tpng -O result0.dot' to visualize function result0.\n");
      Biddy_WriteDot("result1.dot",result1,"result1",-1,FALSE);
      printf("USE 'dot -y -Tpng -O result1.dot' to visualize function result1.\n");
      exit(1);
    }
    */

    /* OPERATIONS LEQ AND GT */

    result0 = Biddy_Leq(result,support);
    result1 = Biddy_Or(Biddy_Not(result),support);

    if (result0 != result1) {
      printf("ERROR: Operation Leq (1) is wrong!\n");
    }

    /* GRAPHVIZ/DOT OUTPUT OF RESULT0 AND RESULT1  - DEBUGGING, ONLY */
    /*
    if (SIZE < 10) {
      Biddy_WriteDot("result0.dot",result0,"result0",-1,FALSE);
      printf("USE 'dot -y -Tpng -O result0.dot' to visualize function result0.\n");
      Biddy_WriteDot("result1.dot",result1,"result1",-1,FALSE);
      printf("USE 'dot -y -Tpng -O result1.dot' to visualize function result1.\n");
      exit(1);
    }
    */

    result0 = Biddy_Leq(support,result);
    result1 = Biddy_Or(Biddy_Not(support),result);

    if (result0 != result1) {
      printf("ERROR: Operation Leq (2) is wrong!\n");
    }

    /* GRAPHVIZ/DOT OUTPUT OF RESULT0 AND RESULT1  - DEBUGGING, ONLY */
    /*
    if (SIZE < 10) {
      Biddy_WriteDot("result0.dot",result0,"result0",-1,FALSE);
      printf("USE 'dot -y -Tpng -O result0.dot' to visualize function result0.\n");
      Biddy_WriteDot("result1.dot",result1,"result1",-1,FALSE);
      printf("USE 'dot -y -Tpng -O result1.dot' to visualize function result1.\n");
      exit(1);
    }
    */

    result0 = Biddy_Gt(result,support);
    result1 = Biddy_And(result,Biddy_Not(support));

    if (result0 != result1) {
      printf("ERROR: Operation Gt is wrong!\n");
    }

    /* GRAPHVIZ/DOT OUTPUT OF RESULT0 AND RESULT1  - DEBUGGING, ONLY */
    /*
    if (SIZE < 10) {
      Biddy_WriteDot("result0.dot",result0,"result0",-1,FALSE);
      printf("USE 'dot -y -Tpng -O result0.dot' to visualize function result0.\n");
      Biddy_WriteDot("result1.dot",result1,"result1",-1,FALSE);
      printf("USE 'dot -y -Tpng -O result1.dot' to visualize function result1.\n");
      exit(1);
    }
    */

    result0 = Biddy_Gt(support,result);
    result1 = Biddy_And(support,Biddy_Not(result));

    if (result0 != result1) {
      printf("ERROR: Operation Gt is wrong!\n");
    }

    /* GRAPHVIZ/DOT OUTPUT OF RESULT0 AND RESULT1  - DEBUGGING, ONLY */
    /*
    if (SIZE < 10) {
      Biddy_WriteDot("result0.dot",result0,"result0",-1,FALSE);
      printf("USE 'dot -y -Tpng -O result0.dot' to visualize function result0.\n");
      Biddy_WriteDot("result1.dot",result1,"result1",-1,FALSE);
      printf("USE 'dot -y -Tpng -O result1.dot' to visualize function result1.\n");
      exit(1);
    }
    */

    result0 = Biddy_Leq(result,support);
    result1 = Biddy_Not(Biddy_Gt(result,support));

    if (result0 != result1) {
      printf("ERROR: Operations Leq and/or Gt are wrong!\n");
    }

    /* GRAPHVIZ/DOT OUTPUT OF RESULT0 AND RESULT1  - DEBUGGING, ONLY */
    /*
    if (SIZE < 10) {
      Biddy_WriteDot("result0.dot",result0,"result0",-1,FALSE);
      printf("USE 'dot -y -Tpng -O result0.dot' to visualize function result0.\n");
      Biddy_WriteDot("result1.dot",result1,"result1",-1,FALSE);
      printf("USE 'dot -y -Tpng -O result1.dot' to visualize function result1.\n");
      exit(1);
    }
    */

    result0 = Biddy_Leq(support,result);
    result1 = Biddy_Not(Biddy_Gt(support,result));

    if (result0 != result1) {
      printf("ERROR: Operations Leq and/or Gt are wrong!\n");
    }

    /* GRAPHVIZ/DOT OUTPUT OF RESULT0 AND RESULT1  - DEBUGGING, ONLY */
    /*
    if (SIZE < 10) {
      Biddy_WriteDot("result0.dot",result0,"result0",-1,FALSE);
      printf("USE 'dot -y -Tpng -O result0.dot' to visualize function result0.\n");
      Biddy_WriteDot("result1.dot",result1,"result1",-1,FALSE);
      printf("USE 'dot -y -Tpng -O result1.dot' to visualize function result1.\n");
      exit(1);
    }
    */

    /* BOOLEAN OPERATIONS AND OPERATION ITE */

    result1 = Biddy_ITE(Biddy_E(result,first),Biddy_E(result,last),Biddy_A(result,last));
    result2 = Biddy_ITE(Biddy_A(result,first),Biddy_E(result,last),Biddy_A(result,last));

    printf("After the first ITE, function result has %.0f minterms/combinations\n",Biddy_CountMinterms(result1,SIZE));
    printf("After the second ITE, function result has %.0f minterms/combinations\n",Biddy_CountMinterms(result2,SIZE));

    if (result1 != Biddy_Xor(
      Biddy_And(Biddy_E(result, first), Biddy_E(result, last)),
      Biddy_And(Biddy_Not(Biddy_E(result, first)), Biddy_A(result, last))))
    {
      printf("ERROR: result1 for Boolean operations ITE-AND-XOR-NOT is wrong!\n");
    }
    if (result1 != Biddy_Or(
      Biddy_And(Biddy_E(result, first), Biddy_E(result, last)),
      Biddy_Gt(Biddy_A(result, last), Biddy_E(result, first))))
    {
      printf("ERROR: result1 for Boolean operations ITE-AND-OR-GT is wrong!\n");
    }

    if (result2 != Biddy_Xor(
      Biddy_And(Biddy_A(result, first), Biddy_E(result, last)),
      Biddy_And(Biddy_Not(Biddy_A(result, first)), Biddy_A(result, last))))
    {
      printf("ERROR: result2 for Boolean operations ITE-AND-XOR-NOT is wrong!\n");
    }
    if (result2 != Biddy_Or(
      Biddy_And(Biddy_A(result, first), Biddy_E(result, last)),
      Biddy_Gt(Biddy_A(result, last), Biddy_A(result, first))))
    {
      printf("ERROR: result2 for Boolean operations ITE-AND-OR-GT is wrong!\n");
    }

    /* GRAPHVIZ/DOT OUTPUT OF RESULT1 AND RESULT2 - DEBUGGING, ONLY */
    /*
    if (SIZE < 10) {
      Biddy_WriteDot("result1.dot",result1,"result1",-1,FALSE);
      printf("USE 'dot -y -Tpng -O result1.dot' to visualize function result1.\n");
      Biddy_WriteDot("result2.dot",result2,"result2",-1,FALSE);
      printf("USE 'dot -y -Tpng -O result2.dot' to visualize function result2.\n");
      exit(1);
    }
    */

    /* OPERATIONS SUBSET0 AND SUBSET1 */

    result0 = Biddy_Subset0(result,first);
    result1 = Biddy_Subset1(result,first);

    printf("Function result has %.0f minterms/combinations where first variable/element is NEGATIVE/ABSENT.\n", Biddy_CountMinterms(result0, SIZE));
    printf("Function result has %.0f minterms/combinations where first variable/element is POSITIVE/PRESENT.\n", Biddy_CountMinterms(result1, SIZE));

    s1 = (unsigned int)Biddy_CountMinterms(result, SIZE);
    s2 = (unsigned int)Biddy_CountMinterms(result0, SIZE);
    s3 = (unsigned int)Biddy_CountMinterms(result1, SIZE);

    if (s1 != (s2 + s3)) {
      printf("ERROR: Operation Subset is wrong!\n");
    }

    /* GRAPHVIZ/DOT OUTPUT OF RESULT0 AND RESULT1  - DEBUGGING, ONLY */
    /*
    if (SIZE < 10) {
      Biddy_WriteDot("result0.dot",result0,"result0",-1,FALSE);
      printf("USE 'dot -y -Tpng -O result0.dot' to visualize function result0.\n");
      Biddy_WriteDot("result1.dot",result1,"result1",-1,FALSE);
      printf("USE 'dot -y -Tpng -O result1.dot' to visualize function result1.\n");
      exit(1);
    }
    */

    result0 = Biddy_Subset0(result,last);
    result1 = Biddy_Subset1(result,last);

    printf("Function result has %.0f minterms/combinations where last variable/element is NEGATIVE/ABSENT.\n", Biddy_CountMinterms(result0, SIZE));
    printf("Function result has %.0f minterms/combinations where last variable/element is POSITIVE/PRESENT.\n", Biddy_CountMinterms(result1, SIZE));

    s1 = (unsigned int)Biddy_CountMinterms(result, SIZE);
    s2 = (unsigned int)Biddy_CountMinterms(result0, SIZE);
    s3 = (unsigned int)Biddy_CountMinterms(result1, SIZE);

    if (s1 != (s2 + s3)) {
      printf("ERROR: Operation Subset is wrong!\n");
    }

    /* GRAPHVIZ/DOT OUTPUT OF RESULT0 AND RESULT1  - DEBUGGING, ONLY */
    /*
    if (SIZE < 10) {
      Biddy_WriteDot("result0.dot",result0,"result0",-1,FALSE);
      printf("USE 'dot -y -Tpng -O result0.dot' to visualize function result0.\n");
      Biddy_WriteDot("result1.dot",result1,"result1",-1,FALSE);
      printf("USE 'dot -y -Tpng -O result1.dot' to visualize function result1.\n");
      exit(1);
    }
    */

    /* OPERATION RESTRICT */

    result0 = Biddy_Restrict(result,first,FALSE);
    result1 = Biddy_Restrict(result,first,TRUE);

    printf("If first variable is restricted to FALSE, function result has %.0f minterms/combinations\n",Biddy_CountMinterms(result0,SIZE));
    printf("If first variable is restricted to TRUE, function result has %.0f minterms/combinations\n",Biddy_CountMinterms(result1,SIZE));

    s1 = (unsigned int)Biddy_CountMinterms(result, SIZE);
    s2 = (unsigned int)Biddy_CountMinterms(result0, SIZE);
    s3 = (unsigned int)Biddy_CountMinterms(result1, SIZE);

    if ((s1 + s1) != (s2 + s3)) {
      printf("ERROR: Operation Restrict is wrong!\n");
    }

    /* GRAPHVIZ/DOT OUTPUT OF RESULT0 AND RESULT1  - DEBUGGING, ONLY */
    /*
    if (SIZE < 10) {
      Biddy_WriteDot("result0.dot",result0,"result0",-1,FALSE);
      printf("USE 'dot -y -Tpng -O result0.dot' to visualize function result0.\n");
      Biddy_WriteDot("result1.dot",result1,"result1",-1,FALSE);
      printf("USE 'dot -y -Tpng -O result1.dot' to visualize function result1.\n");
      exit(1);
    }
    */

    result0 = Biddy_Restrict(result,last,FALSE);
    result1 = Biddy_Restrict(result,last,TRUE);

    printf("If last variable is restricted to FALSE, function result has %.0f minterms/combinations\n", Biddy_CountMinterms(result0, SIZE));
    printf("If last variable is restricted to TRUE, function result has %.0f minterms/combinations\n", Biddy_CountMinterms(result1, SIZE));

    s1 = (unsigned int)Biddy_CountMinterms(result, SIZE);
    s2 = (unsigned int)Biddy_CountMinterms(result0, SIZE);
    s3 = (unsigned int)Biddy_CountMinterms(result1, SIZE);

    if ((s1 + s1) != (s2 + s3)) {
      printf("ERROR: Operation Restrict is wrong!\n");
    }

    /* GRAPHVIZ/DOT OUTPUT OF RESULT0 AND RESULT1  - DEBUGGING, ONLY */
    /*
    if (SIZE < 10) {
      Biddy_WriteDot("result0.dot",result0,"result0",-1,FALSE);
      printf("USE 'dot -y -Tpng -O result0.dot' to visualize function result0.\n");
      Biddy_WriteDot("result1.dot",result1,"result1",-1,FALSE);
      printf("USE 'dot -y -Tpng -O result1.dot' to visualize function result1.\n");
      exit(1);
    }
    */

    /* OPERATION QUANTIFICATION */

    result0 = Biddy_E(result,first);
    result1 = Biddy_A(result,first);

    printf("After the existential quantification of first variable, function result has %.0f minterms/combinations\n",Biddy_CountMinterms(result0,SIZE));
    printf("After the universal quantification of first variable, function result has %.0f minterms/combinations\n",Biddy_CountMinterms(result1,SIZE));

    s1 = (unsigned int)Biddy_CountMinterms(result, SIZE);
    s2 = (unsigned int)Biddy_CountMinterms(result0, SIZE);
    s3 = (unsigned int)Biddy_CountMinterms(result1, SIZE);

    if ((s1 + s1) != (s2 + s3)) {
      printf("ERROR: Operation Quantification is wrong!\n");
    }

    /* GRAPHVIZ/DOT OUTPUT OF RESULT0 AND RESULT1  - DEBUGGING, ONLY */
    /*
    if (SIZE < 10) {
      Biddy_WriteDot("result0.dot",result0,"result0",-1,FALSE);
      printf("USE 'dot -y -Tpng -O result0.dot' to visualize function result0.\n");
      Biddy_WriteDot("result1.dot",result1,"result1",-1,FALSE);
      printf("USE 'dot -y -Tpng -O result1.dot' to visualize function result1.\n");
      exit(1);
    }
    */

    result0 = Biddy_E(result,last);
    result1 = Biddy_A(result,last);

    printf("After the existential quantification of last variable, function result has %.0f minterms/combinations\n", Biddy_CountMinterms(result0, SIZE));
    printf("After the universal quantification of last variable, function result has %.0f minterms/combinations\n", Biddy_CountMinterms(result1, SIZE));

    s1 = (unsigned int)Biddy_CountMinterms(result, SIZE);
    s2 = (unsigned int)Biddy_CountMinterms(result0, SIZE);
    s3 = (unsigned int)Biddy_CountMinterms(result1, SIZE);

    if ((s1 + s1) != (s2 + s3)) {
      printf("ERROR: Operation Quantification is wrong!\n");
    }

    /* GRAPHVIZ/DOT OUTPUT OF RESULT0 AND RESULT1  - DEBUGGING, ONLY */
    /*
    if (SIZE < 10) {
      Biddy_WriteDot("result0.dot",result0,"result0",-1,FALSE);
      printf("USE 'dot -y -Tpng -O result0.dot' to visualize function result0.\n");
      Biddy_WriteDot("result1.dot",result1,"result1",-1,FALSE);
      printf("USE 'dot -y -Tpng -O result1.dot' to visualize function result1.\n");
      exit(1);
    }
    */

    /* EXTRACT MINTERM */
    
    result0 = Biddy_ExtractMinterm(result);

    if ((Biddy_CountMinterms(result,SIZE) > 0) && (Biddy_CountMinterms(result0,SIZE) != 1)) {
      printf("ERROR: Operation ExtractMinterm is wrong!\n");
    } else {
      printf("Operation ExtractMinterm is OK!\n");
    }

    s1 = (unsigned int)Biddy_CountMinterms(result,SIZE);

    s2 = 0;
    result1 = result;
    while (result1 != Biddy_GetConstantZero()) {
      result2 = Biddy_ExtractMinterm(result1);
      result1 = Biddy_ITE(result2,Biddy_GetConstantZero(),result1);
      s2++;
    }

    if (s1 != s2) {
      printf("ERROR: Operation ExtractMinterm is wrong!\n");
    }

    /* ************************************************************* */
    /* OUTPUT FUNCTIONS  */
    /* ************************************************************* */

    printf("********* OUTPUT FUNCTIONS *********\n");

    /* IT IS OK FOR OBDDs TO HAVE LESS REPORTED MINTERMS, BECAUSE DON'T CARE VARIABLES ARE OMITTED */
    /* NUMBER OF COMBINATIONS MUST BE THE SAME FOR ALL BDD TYPES */

    i = 0; /* use 0 to disable output */

    robdd = Biddy_Copy(MNGOBDD,result);
    robddc = Biddy_Copy(MNGOBDDC,result);
    rzbdd = Biddy_Copy(MNGZBDD,result);
    rzbddc = Biddy_Copy(MNGZBDDC,result);
    rtzbdd = Biddy_Copy(MNGTZBDD,result);

    /*
    PRINTCOMBINATIONS(MNGOBDD,robdd);
    PRINTCOMBINATIONS(MNGOBDDC,robddc);
    PRINTCOMBINATIONS(MNGZBDD,rzbdd);
    PRINTCOMBINATIONS(MNGZBDDC,rzbddc);
    PRINTCOMBINATIONS(MNGTZBDD,rtzbdd);
    */

    printf("MINTERMS for OBDD (%.0f):\n",Biddy_Managed_CountMinterms(MNGOBDD,robdd,-1));
    if (Biddy_Managed_CountMinterms(MNGOBDD,robdd,0) <= i) { /* 0 is used to consider all the dependent variables */
      Biddy_Managed_PrintfMinterms(MNGOBDD,robdd,TRUE); /* TRUE is used for Boolean functions */
    }

    printf("MINTERMS for OBDDC (%.0f):\n",Biddy_Managed_CountMinterms(MNGOBDDC,robddc,-1));
    if (Biddy_Managed_CountMinterms(MNGOBDDC,robddc,0) <= i) { /* 0 is used to consider all the dependent variables */
      Biddy_Managed_PrintfMinterms(MNGOBDDC,robddc,TRUE); /* TRUE is used for Boolean functions */
    }

    printf("MINTERMS for ZBDD (%.0f):\n",Biddy_Managed_CountMinterms(MNGZBDD,rzbdd,-1));
    if (Biddy_Managed_CountMinterms(MNGZBDD,rzbdd,0) <= i) { /* 0 is used to consider all the dependent variables */
      Biddy_Managed_PrintfMinterms(MNGZBDD,rzbdd,TRUE); /* TRUE is used for Boolean functions */
    }

    printf("MINTERMS for ZBDDC (%.0f):\n",Biddy_Managed_CountMinterms(MNGZBDDC,rzbddc,-1));
    if (Biddy_Managed_CountMinterms(MNGZBDDC,rzbddc,0) <= i) { /* 0 is used to consider all the dependent variables */
      Biddy_Managed_PrintfMinterms(MNGZBDDC,rzbddc,TRUE); /* TRUE is used for Boolean functions */
    }

    printf("MINTERMS for TZBDD (%.0f):\n",Biddy_Managed_CountMinterms(MNGTZBDD,rtzbdd,-1));
    if (Biddy_Managed_CountMinterms(MNGTZBDD,rtzbdd,0) <= i) { /* 0 is used to consider all the dependent variables */
      Biddy_Managed_PrintfMinterms(MNGTZBDD,rtzbdd,TRUE); /* TRUE is used for Boolean functions */
    }

    printf("COMBINATIONS for OBDD (%.0f):\n",Biddy_Managed_CountCombinations(MNGOBDD,robdd));
    if (Biddy_Managed_CountCombinations(MNGOBDD,robdd) <= i) {
      Biddy_Managed_PrintfMinterms(MNGOBDD,robdd,FALSE); /* FALSE is used for combinations */
    }

    printf("COMBINATIONS for OBDDC (%.0f):\n",Biddy_Managed_CountCombinations(MNGOBDDC,robddc));
    if (Biddy_Managed_CountCombinations(MNGOBDDC,robddc) <= i) {
      Biddy_Managed_PrintfMinterms(MNGOBDDC,robddc,FALSE); /* FALSE is used for combinations */
    }

    printf("COMBINATIONS for ZBDD (%.0f):\n",Biddy_Managed_CountCombinations(MNGZBDD,rzbdd));
    if (Biddy_Managed_CountCombinations(MNGZBDD,rzbdd) <= i) {
      Biddy_Managed_PrintfMinterms(MNGZBDD,rzbdd,FALSE); /* FALSE is used for combinations */
    }

    printf("COMBINATIONS for ZBDDC (%.0f):\n",Biddy_Managed_CountCombinations(MNGZBDDC,rzbddc));
    if (Biddy_Managed_CountCombinations(MNGZBDDC,rzbddc) <= i) {
      Biddy_Managed_PrintfMinterms(MNGZBDDC,rzbddc,FALSE); /* FALSE is used for combinations */
    }

    printf("COMBINATIONS for TZBDD (%.0f):\n",Biddy_Managed_CountCombinations(MNGTZBDD,rtzbdd));
    if (Biddy_Managed_CountCombinations(MNGTZBDD,rtzbdd) <= i) {
      Biddy_Managed_PrintfMinterms(MNGTZBDD,rtzbdd,FALSE); /* FALSE is used for combinations */
    }

    /* ************************************************************* */
    /* CONSISTENCY OVER DIFFERENT BDD TYPES  */
    /* ************************************************************* */

    printf("********* CONSISTENCY OVER DIFFERENT BDD TYPES *********\n");

    /* GENERATE ADDITIONAL RANDOM FUNCTION OR COMBINATION SET */

    coresult = NULL;

#if (RANDOMTYPE == BOOLEANFUNCTION)
    printf("Generating random Boolean function...\n");
    coresult = Biddy_RandomFunction(support,RATIO);
#endif

#if (RANDOMTYPE == COMBINATIONSET)
    printf("Generating random combination set...\n");
    coresult = Biddy_RandomSet(support,RATIO);
#endif

    if (coresult == NULL) {
      printf("ERROR: Function coresult is NULL!\n");
      exit(1);
    }

    /* GRAPHVIZ/DOT OUTPUT OF coresult - DEBUGGING, ONLY */
    /*
    if (SIZE < 10) {
      Biddy_WriteDot("result.dot", coresult, "coresult", -1, FALSE);
      printf("USE 'dot -y -Tpng -O coresult.dot' to visualize function coresult.\n");
    }
    */

    /* TRUTH TABLE FOR result AND coresult  - DEBUGGING, ONLY */
    /*
    if (SIZE < 10) {
      printf("HERE IS A TRUTH TABLE FOR result:\n");
      Biddy_PrintfTable(result);
      printf("HERE IS A TRUTH TABLE FOR coresult:\n");
      Biddy_PrintfTable(coresult);
    }
    */

    printf("Function coresult has %.0f minterms/combinations.\n",Biddy_CountMinterms(coresult,SIZE));
    printf("Function coresult has density = %.2e.\n",Biddy_DensityOfFunction(coresult,SIZE));

    /*
    printf("Function represented by %s depends on %u variables.\n",Biddy_GetManagerName(),Biddy_DependentVariableNumber(coresult,FALSE));
    printf("%s for function result has %u nodes (including terminals).\n",Biddy_GetManagerName(),Biddy_CountNodes(coresult));
    printf("%s for function result has %u plain nodes (including terminals).\n",Biddy_GetManagerName(),Biddy_CountNodesPlain(coresult));
    printf("%s for function result has %llu one-paths.\n",Biddy_GetManagerName(),Biddy_CountPaths(coresult));
    */

    /* OPERATION SUBSET */

    result0 = Biddy_Managed_Subset(MNGOBDD,Biddy_Copy(MNGOBDD,result),Biddy_Copy(MNGOBDD,coresult));
    printf("(SUBSET) Function result0 (MNGOBDD) has %.0f minterms/combinations.\n",Biddy_Managed_CountMinterms(MNGOBDD,result0,SIZE));

    /* TRUTH TABLE FOR OBDD - DEBUGGING, ONLY */
    /*
    if (SIZE < 10) {
      printf("HERE IS A TRUTH TABLE FOR OBDD:\n");
      Biddy_Managed_PrintfTable(MNGOBDD,result0);
    }
    */

    result1 = Biddy_Managed_Copy(MNGOBDD,MNGOBDDC,result0);
    result2 = Biddy_Managed_Subset(MNGOBDDC,Biddy_Copy(MNGOBDDC,result),Biddy_Copy(MNGOBDDC,coresult));
    printf("(SUBSET) Function result2 (MNGOBDDC) has %.0f minterms/combinations.\n",Biddy_Managed_CountMinterms(MNGOBDDC,result2,SIZE));

    if (result2 != result1) {
      printf("ERROR: Operation Subset is wrong for MNGOBDD / MNGOBDDC\n");
    }

    /* TRUTH TABLE FOR OBDDC - DEBUGGING, ONLY */
    /*
    if (SIZE < 10) {
      printf("HERE IS A TRUTH TABLE FOR result1 FOR OBDDC:\n");
      Biddy_Managed_PrintfTable(MNGOBDDC,result1);
      printf("HERE IS A TRUTH TABLE FOR result2 FOR OBDDC:\n");
      Biddy_Managed_PrintfTable(MNGOBDDC,result2);
    }
    */

    result1 = Biddy_Managed_Copy(MNGOBDD,MNGZBDD,result0);
    result2 = Biddy_Managed_Subset(MNGZBDD,Biddy_Copy(MNGZBDD,result),Biddy_Copy(MNGZBDD,coresult));
    printf("(SUBSET) Function result2 (MNGZBDD) has %.0f minterms/combinations.\n",Biddy_Managed_CountMinterms(MNGZBDD,result2,SIZE));

    if (result2 != result1) {
      printf("ERROR: Operation Subset is wrong for MNGOBDD / MNGZBDD\n");
    }

    /* TRUTH TABLE FOR ZBDD - DEBUGGING, ONLY */
    /*
    if (SIZE < 10) {
      printf("HERE IS A TRUTH TABLE FOR result1 FOR ZBDD:\n");
      Biddy_Managed_PrintfTable(MNGZBDD,result1);
      printf("HERE IS A TRUTH TABLE FOR result2 FOR ZBDD:\n");
      Biddy_Managed_PrintfTable(MNGZBDD,result2);
    }
    */

    result1 = Biddy_Managed_Copy(MNGOBDD,MNGZBDDC,result0);
    result2 = Biddy_Managed_Subset(MNGZBDDC,Biddy_Copy(MNGZBDDC,result),Biddy_Copy(MNGZBDDC,coresult));
    printf("(SUBSET) Function result2 (MNGZBDDC) has %.0f minterms/combinations.\n",Biddy_Managed_CountMinterms(MNGZBDDC,result2,SIZE));

    if (result2 != result1) {
      printf("ERROR: Operation Subset is wrong for MNGOBDD / MNGZBDDC\n");
    }

    /* TRUTH TABLE FOR ZBDDC - DEBUGGING, ONLY */
    /*
    if (SIZE < 10) {
      printf("HERE IS A TRUTH TABLE FOR result1 FOR ZBDDC:\n");
      Biddy_Managed_PrintfTable(MNGZBDDC,result1);
      printf("HERE IS A TRUTH TABLE FOR result2 FOR ZBDDC:\n");
      Biddy_Managed_PrintfTable(MNGZBDDC,result2);
    }
    */

    result1 = Biddy_Managed_Copy(MNGOBDD,MNGTZBDD,result0);
    result2 = Biddy_Managed_Subset(MNGTZBDD,Biddy_Copy(MNGTZBDD,result),Biddy_Copy(MNGTZBDD,coresult));
    printf("(SUBSET) Function result2 (MNGTZBDD) has %.0f minterms/combinations.\n",Biddy_Managed_CountMinterms(MNGTZBDD,result2,SIZE));

    if (result2 != result1) {
      printf("ERROR: Operation Subset is wrong for MNGOBDD / MNGTZBDD\n");
    }

    /* TRUTH TABLE FOR TZBDD - DEBUGGING, ONLY */
    /*
    if (SIZE < 10) {
      printf("HERE IS A TRUTH TABLE FOR result1 FOR TZBDD:\n");
      Biddy_Managed_PrintfTable(MNGTZBDD,result1);
      printf("HERE IS A TRUTH TABLE FOR result2 FOR TZBDD:\n");
      Biddy_Managed_PrintfTable(MNGTZBDD,result2);
    }
    */

    /* OPERATIONS SUPSET */

    result0 = Biddy_Managed_Supset(MNGOBDD,Biddy_Copy(MNGOBDD,result),Biddy_Copy(MNGOBDD,coresult));
    printf("(SUPSET) Function result0 (MNGOBDD) has %.0f minterms/combinations.\n",Biddy_Managed_CountMinterms(MNGOBDD,result0,SIZE));

    /* TRUTH TABLE FOR OBDD - DEBUGGING, ONLY */
    /*
    if (SIZE < 10) {
      printf("HERE IS A TRUTH TABLE FOR OBDD:\n");
      Biddy_Managed_PrintfTable(MNGOBDD,result0);
    }
    */

    result1 = Biddy_Managed_Copy(MNGOBDD,MNGOBDDC,result0);
    result2 = Biddy_Managed_Supset(MNGOBDDC,Biddy_Copy(MNGOBDDC,result),Biddy_Copy(MNGOBDDC,coresult));
    printf("(SUPSET) Function result2 (MNGOBDDC) has %.0f minterms/combinations.\n",Biddy_Managed_CountMinterms(MNGOBDDC,result2,SIZE));

    if (result2 != result1) {
      printf("ERROR: Operation Supset is wrong for MNGOBDD / MNGOBDDC\n");
    }

    /* TRUTH TABLE FOR OBDDC - DEBUGGING, ONLY */
    /*
    if (SIZE < 10) {
      printf("HERE IS A TRUTH TABLE FOR result1 FOR OBDDC:\n");
      Biddy_Managed_PrintfTable(MNGOBDDC,result1);
      printf("HERE IS A TRUTH TABLE FOR result2 FOR OBDDC:\n");
      Biddy_Managed_PrintfTable(MNGOBDDC,result2);
    }
    */

    result1 = Biddy_Managed_Copy(MNGOBDD,MNGZBDD,result0);
    result2 = Biddy_Managed_Supset(MNGZBDD,Biddy_Copy(MNGZBDD,result),Biddy_Copy(MNGZBDD,coresult));
    printf("(SUPSET) Function result2 (MNGZBDD) has %.0f minterms/combinations.\n",Biddy_Managed_CountMinterms(MNGZBDD,result2,SIZE));

    if (result2 != result1) {
      printf("ERROR: Operation Supset is wrong for MNGOBDD / MNGZBDD\n");
    }

    /* TRUTH TABLE FOR ZBDD - DEBUGGING, ONLY */
    /*
    if (SIZE < 10) {
      printf("HERE IS A TRUTH TABLE FOR result1 FOR ZBDD:\n");
      Biddy_Managed_PrintfTable(MNGZBDD,result1);
      printf("HERE IS A TRUTH TABLE FOR result2 FOR ZBDD:\n");
      Biddy_Managed_PrintfTable(MNGZBDD,result2);
    }
    */

    result1 = Biddy_Managed_Copy(MNGOBDD,MNGZBDDC,result0);
    result2 = Biddy_Managed_Supset(MNGZBDDC,Biddy_Copy(MNGZBDDC,result),Biddy_Copy(MNGZBDDC,coresult));
    printf("(SUPSET) Function result2 (MNGZBDDC) has %.0f minterms/combinations.\n",Biddy_Managed_CountMinterms(MNGZBDDC,result2,SIZE));

    if (result2 != result1) {
      printf("ERROR: Operation Supset is wrong for MNGOBDD / MNGZBDDC\n");
    }

    /* TRUTH TABLE FOR ZBDDC - DEBUGGING, ONLY */
    /*
    if (SIZE < 10) {
      printf("HERE IS A TRUTH TABLE FOR result1 FOR ZBDDC:\n");
      Biddy_Managed_PrintfTable(MNGZBDDC,result1);
      printf("HERE IS A TRUTH TABLE FOR result2 FOR ZBDDC:\n");
      Biddy_Managed_PrintfTable(MNGZBDDC,result2);
    }
    */

    result1 = Biddy_Managed_Copy(MNGOBDD,MNGTZBDD,result0);
    result2 = Biddy_Managed_Supset(MNGTZBDD,Biddy_Copy(MNGTZBDD,result),Biddy_Copy(MNGTZBDD,coresult));
    printf("(SUPSET) Function result2 (MNGTZBDD) has %.0f minterms/combinations.\n",Biddy_Managed_CountMinterms(MNGTZBDD,result2,SIZE));

    if (result2 != result1) {
      printf("ERROR: Operation Supset is wrong for MNGOBDD / MNGTZBDD\n");
    }

    /* TRUTH TABLE FOR TZBDD - DEBUGGING, ONLY */
    /*
    if (SIZE < 10) {
      printf("HERE IS A TRUTH TABLE FOR result1 FOR TZBDD:\n");
      Biddy_Managed_PrintfTable(MNGTZBDD,result1);
      printf("HERE IS A TRUTH TABLE FOR result2 FOR TZBDD:\n");
      Biddy_Managed_PrintfTable(MNGTZBDD,result2);
    }
    */

    /* OPERATION E (EXISTENTIAL ABSTRACTION) */

    result0 = Biddy_Managed_E(MNGOBDD,Biddy_Copy(MNGOBDD,result),second);
    printf("(E) Function result0 (MNGOBDD) has %.0f minterms/combinations.\n",Biddy_Managed_CountMinterms(MNGOBDD,result0,SIZE));

    /* TRUTH TABLE FOR OBDD - DEBUGGING, ONLY */
    /*
    if (SIZE < 10) {
      printf("HERE IS A TRUTH TABLE FOR OBDD:\n");
      Biddy_Managed_PrintfTable(MNGOBDD,result0);
    }
    */

    result1 = Biddy_Managed_Copy(MNGOBDD,MNGOBDDC,result0);
    result2 = Biddy_Managed_E(MNGOBDDC,Biddy_Copy(MNGOBDDC,result),second);
    printf("(E) Function result2 (MNGOBDDC) has %.0f minterms/combinations.\n",Biddy_Managed_CountMinterms(MNGOBDDC,result2,SIZE));

    if (result2 != result1) {
      printf("ERROR: Operation E is wrong for MNGOBDD / MNGOBDDC\n");
    }

    /* TRUTH TABLE FOR OBDDC - DEBUGGING, ONLY */
    /*
    if (SIZE < 10) {
      printf("HERE IS A TRUTH TABLE FOR result1 FOR OBDDC:\n");
      Biddy_Managed_PrintfTable(MNGOBDDC,result1);
      printf("HERE IS A TRUTH TABLE FOR result2 FOR OBDDC:\n");
      Biddy_Managed_PrintfTable(MNGOBDDC,result2);
    }
    */

    result1 = Biddy_Managed_Copy(MNGOBDD,MNGZBDD,result0);
    result2 = Biddy_Managed_E(MNGZBDD,Biddy_Copy(MNGZBDD,result),second);
    printf("(E) Function result2 (MNGZBDD) has %.0f minterms/combinations.\n",Biddy_Managed_CountMinterms(MNGZBDD,result2,SIZE));

    if (result2 != result1) {
      printf("ERROR: Operation E is wrong for MNGOBDD / MNGZBDD\n");
    }

    /* TRUTH TABLE FOR ZBDD - DEBUGGING, ONLY */
    /*
    if (SIZE < 10) {
      Biddy_Managed_WriteBddview(MNGZBDD,"result.bddview",Biddy_Copy(MNGZBDD,result),"result",NULL);
      printf("HERE IS A TRUTH TABLE FOR result1 FOR ZBDD:\n");
      Biddy_Managed_PrintfTable(MNGZBDD,result1);
      Biddy_Managed_WriteBddview(MNGZBDD,"result1.bddview",result1,"result1",NULL);
      printf("HERE IS A TRUTH TABLE FOR result2 FOR ZBDD:\n");
      Biddy_Managed_PrintfTable(MNGZBDD,result2);
      Biddy_Managed_WriteBddview(MNGZBDD,"result2.bddview",result2,"result2",NULL);
    }
    */

    result1 = Biddy_Managed_Copy(MNGOBDD,MNGZBDDC,result0);
    result2 = Biddy_Managed_E(MNGZBDDC,Biddy_Copy(MNGZBDDC,result),second);
    printf("(E) Function result2 (MNGZBDDC) has %.0f minterms/combinations.\n",Biddy_Managed_CountMinterms(MNGZBDDC,result2,SIZE));

    if (result2 != result1) {
      printf("ERROR: Operation E is wrong for MNGOBDD / MNGZBDDC\n");
    }

    /* TRUTH TABLE FOR ZBDDC - DEBUGGING, ONLY */
    /*
    if (SIZE < 10) {
      printf("HERE IS A TRUTH TABLE FOR result1 FOR ZBDDC:\n");
      Biddy_Managed_PrintfTable(MNGZBDDC,result1);
      printf("HERE IS A TRUTH TABLE FOR result2 FOR ZBDDC:\n");
      Biddy_Managed_PrintfTable(MNGZBDDC,result2);
    }
    */

    result1 = Biddy_Managed_Copy(MNGOBDD,MNGTZBDD,result0);
    result2 = Biddy_Managed_E(MNGTZBDD,Biddy_Copy(MNGTZBDD,result),second);
    printf("(E) Function result2 (MNGTZBDD) has %.0f minterms/combinations.\n",Biddy_Managed_CountMinterms(MNGTZBDD,result2,SIZE));

    if (result2 != result1) {
      printf("ERROR: Operation E is wrong for MNGOBDD / MNGTZBDD\n");
    }

    /* TRUTH TABLE FOR TZBDD - DEBUGGING, ONLY */
    /*
    if (SIZE < 10) {
      printf("HERE IS A TRUTH TABLE FOR result1 FOR TZBDD:\n");
      Biddy_Managed_PrintfTable(MNGTZBDD,result1);
      printf("HERE IS A TRUTH TABLE FOR result2 FOR TZBDD:\n");
      Biddy_Managed_PrintfTable(MNGTZBDD,result2);
    }
    */

    /* OPERATION ELEMENT ABSTRACT */

    result0 = Biddy_Managed_ElementAbstract(MNGOBDD,Biddy_Copy(MNGOBDD,result),second);
    printf("(ELEMENT ABSTRACT) Function result0 (MNGOBDD) has %.0f minterms/combinations.\n",Biddy_Managed_CountMinterms(MNGOBDD,result0,SIZE));

    /* TRUTH TABLE FOR OBDD - DEBUGGING, ONLY */
    /*
    if (SIZE < 10) {
      printf("HERE IS A TRUTH TABLE FOR result FOR OBDD:\n");
      Biddy_Managed_PrintfTable(MNGOBDD,Biddy_Copy(MNGOBDD,result));
      Biddy_Managed_WriteBddview(MNGOBDD,"result.bddview",Biddy_Copy(MNGOBDD,result),"result",NULL);
      printf("HERE IS A TRUTH TABLE FOR result0 FOR OBDD:\n");
      Biddy_Managed_PrintfTable(MNGOBDD,result0);
      Biddy_Managed_WriteBddview(MNGOBDD,"result0.bddview",result0,"result0",NULL);
    }
    */

    result1 = Biddy_Managed_Copy(MNGOBDD,MNGOBDDC,result0);
    result2 = Biddy_Managed_ElementAbstract(MNGOBDDC,Biddy_Copy(MNGOBDDC,result),second);
    printf("(ELEMENT ABSTRACT) Function result2 (MNGOBDDC) has %.0f minterms/combinations.\n",Biddy_Managed_CountMinterms(MNGOBDDC,result2,SIZE));

    if (result2 != result1) {
      printf("ERROR: Operation ElementAbstract is wrong for MNGOBDD / MNGOBDDC\n");
    }

    /* TRUTH TABLE FOR OBDDC - DEBUGGING, ONLY */
    /*
    if (SIZE < 10) {
      printf("HERE IS A TRUTH TABLE FOR result1 FOR OBDDC:\n");
      Biddy_Managed_PrintfTable(MNGOBDDC,result1);
      printf("HERE IS A TRUTH TABLE FOR result2 FOR OBDDC:\n");
      Biddy_Managed_PrintfTable(MNGOBDDC,result2);
    }
    */

    result1 = Biddy_Managed_Copy(MNGOBDD,MNGZBDD,result0);
    result2 = Biddy_Managed_ElementAbstract(MNGZBDD,Biddy_Copy(MNGZBDD,result),second);
    printf("(ELEMENT ABSTRACT) Function result2 (MNGZBDD) has %.0f minterms/combinations.\n",Biddy_Managed_CountMinterms(MNGZBDD,result2,SIZE));

    if (result2 != result1) {
      printf("ERROR: Operation ElementAbstract is wrong for MNGOBDD / MNGZBDD\n");
    }

    /* TRUTH TABLE FOR ZBDD - DEBUGGING, ONLY */
    /*
    if (SIZE < 10) {
      printf("HERE IS A TRUTH TABLE FOR result1 FOR ZBDD:\n");
      Biddy_Managed_PrintfTable(MNGZBDD,result1);
      Biddy_Managed_WriteBddview(MNGZBDD,"result1.bddview",result1,"result1",NULL);
      printf("HERE IS A TRUTH TABLE FOR result2 FOR ZBDD:\n");
      Biddy_Managed_PrintfTable(MNGZBDD,result2);
      Biddy_Managed_WriteBddview(MNGZBDD,"result2.bddview",result2,"result2",NULL);
    }
    */

    result1 = Biddy_Managed_Copy(MNGOBDD,MNGZBDDC,result0);
    result2 = Biddy_Managed_ElementAbstract(MNGZBDDC,Biddy_Copy(MNGZBDDC,result),second);
    printf("(ELEMENT ABSTRACT) Function result2 (MNGZBDDC) has %.0f minterms/combinations.\n",Biddy_Managed_CountMinterms(MNGZBDDC,result2,SIZE));

    if (result2 != result1) {
      printf("ERROR: Operation ElementAbstract is wrong for MNGOBDD / MNGZBDDC\n");
    }

    /* TRUTH TABLE FOR ZBDDC - DEBUGGING, ONLY */
    /*
    if (SIZE < 10) {
      printf("HERE IS A TRUTH TABLE FOR result1 FOR ZBDDC:\n");
      Biddy_Managed_PrintfTable(MNGZBDDC,result1);
      printf("HERE IS A TRUTH TABLE FOR result2 FOR ZBDDC:\n");
      Biddy_Managed_PrintfTable(MNGZBDDC,result2);
    }
    */

    result1 = Biddy_Managed_Copy(MNGOBDD,MNGTZBDD,result0);
    result2 = Biddy_Managed_ElementAbstract(MNGTZBDD,Biddy_Copy(MNGTZBDD,result),second);
    printf("(ELEMENT ABSTRACT) Function result2 (MNGTZBDD) has %.0f minterms/combinations.\n",Biddy_Managed_CountMinterms(MNGTZBDD,result2,SIZE));

    if (result2 != result1) {
      printf("ERROR: Operation ElementAbstract is wrong for MNGOBDD / MNGTZBDD\n");
    }

    /* TRUTH TABLE FOR TZBDD - DEBUGGING, ONLY */
    /*
    if (SIZE < 10) {
      Biddy_Managed_WriteBddview(MNGTZBDD,"result.bddview",Biddy_Copy(MNGTZBDD,result),"result",NULL);
      printf("HERE IS A TRUTH TABLE FOR result1 FOR TZBDD:\n");
      Biddy_Managed_PrintfTable(MNGTZBDD,result1);
      Biddy_Managed_WriteBddview(MNGTZBDD,"result1.bddview",result1,"result1",NULL);
      printf("HERE IS A TRUTH TABLE FOR result2 FOR TZBDD:\n");
      Biddy_Managed_PrintfTable(MNGTZBDD,result2);
      Biddy_Managed_WriteBddview(MNGTZBDD,"result2.bddview",result2,"result2",NULL);
    }
    */

    /* OPERATION STRETCH */

    result0 = Biddy_Managed_Stretch(MNGOBDD,Biddy_Copy(MNGOBDD,result));
    printf("(STRETCH) Function result0 (MNGOBDD) has %.0f minterms/combinations.\n",Biddy_Managed_CountMinterms(MNGOBDD,result0,SIZE));

    if (Biddy_Managed_Diff(MNGOBDD,result0,Biddy_Copy(MNGOBDD,result)) != Biddy_Managed_GetEmptySet(MNGOBDD)) {
      printf("ERROR: Operation Stretch has wrong result for MNGOBDD\n");
    }

    /* TRUTH TABLE FOR OBDD - DEBUGGING, ONLY */
    /*
    if (SIZE < 10) {
      printf("HERE IS A TRUTH TABLE FOR result FOR OBDD:\n");
      Biddy_Managed_PrintfTable(MNGOBDD,Biddy_Copy(MNGOBDD,result));
      Biddy_Managed_WriteBddview(MNGOBDD,"result.bddview",Biddy_Copy(MNGOBDD,result),"result",NULL);
      printf("HERE IS A TRUTH TABLE FOR result0 FOR OBDD:\n");
      Biddy_Managed_PrintfTable(MNGOBDD,result0);
      Biddy_Managed_WriteBddview(MNGOBDD,"result0.bddview",result0,"result0",NULL);
    }
    */

    result1 = Biddy_Managed_Copy(MNGOBDD,MNGOBDDC,result0);
    result2 = Biddy_Managed_Stretch(MNGOBDDC,Biddy_Copy(MNGOBDDC,result));
    printf("(STRETCH) Function result2 (MNGOBDDC) has %.0f minterms/combinations.\n",Biddy_Managed_CountMinterms(MNGOBDDC,result2,SIZE));

    if (Biddy_Managed_Diff(MNGOBDDC,result2,Biddy_Copy(MNGOBDDC,result)) != Biddy_Managed_GetEmptySet(MNGOBDDC)) {
      printf("ERROR: Operation Stretch has wrong result for MNGOBDDC\n");
    }

    if (result2 != result1) {
      printf("ERROR: Operation Stretch is wrong for MNGOBDD / MNGOBDDC\n");
    }

    /* TRUTH TABLE FOR OBDDC - DEBUGGING, ONLY */
    /*
    if (SIZE < 10) {
      printf("HERE IS A TRUTH TABLE FOR result1 FOR OBDDC:\n");
      Biddy_Managed_PrintfTable(MNGOBDDC,result1);
      printf("HERE IS A TRUTH TABLE FOR result2 FOR OBDDC:\n");
      Biddy_Managed_PrintfTable(MNGOBDDC,result2);
    }
    */

    result1 = Biddy_Managed_Copy(MNGOBDD,MNGZBDD,result0);
    result2 = Biddy_Managed_Stretch(MNGZBDD,Biddy_Copy(MNGZBDD,result));
    printf("(STRETCH) Function result2 (MNGZBDD) has %.0f minterms/combinations.\n",Biddy_Managed_CountMinterms(MNGZBDD,result2,SIZE));

    if (Biddy_Managed_Diff(MNGZBDD,result2,Biddy_Copy(MNGZBDD,result)) != Biddy_Managed_GetEmptySet(MNGZBDD)) {
      printf("ERROR: Operation Stretch has wrong result for MNGZBDD\n");
    }

    if (result2 != result1) {
      printf("ERROR: Operation Stretch is wrong for MNGOBDD / MNGZBDD\n");
    }

    /* TRUTH TABLE FOR ZBDD - DEBUGGING, ONLY */
    /*
    if (SIZE < 10) {
      printf("HERE IS A TRUTH TABLE FOR result1 FOR ZBDD:\n");
      Biddy_Managed_PrintfTable(MNGZBDD,result1);
      Biddy_Managed_WriteBddview(MNGZBDD,"result1.bddview",result1,"result1",NULL);
      printf("HERE IS A TRUTH TABLE FOR result2 FOR ZBDD:\n");
      Biddy_Managed_PrintfTable(MNGZBDD,result2);
      Biddy_Managed_WriteBddview(MNGZBDD,"result2.bddview",result2,"result2",NULL);
    }
    */

    result1 = Biddy_Managed_Copy(MNGOBDD,MNGZBDDC,result0);
    result2 = Biddy_Managed_Stretch(MNGZBDDC,Biddy_Copy(MNGZBDDC,result));
    printf("(STRETCH) Function result2 (MNGZBDDC) has %.0f minterms/combinations.\n",Biddy_Managed_CountMinterms(MNGZBDDC,result2,SIZE));

    if (Biddy_Managed_Diff(MNGZBDDC,result2,Biddy_Copy(MNGZBDDC,result)) != Biddy_Managed_GetEmptySet(MNGZBDDC)) {
      printf("ERROR: Operation Stretch has wrong result for MNGZBDDC\n");
    }

    if (result2 != result1) {
      printf("ERROR: Operation Stretch is wrong for MNGOBDD / MNGZBDDC\n");
    }

    /* TRUTH TABLE FOR ZBDDC - DEBUGGING, ONLY */
    /*
    if (SIZE < 10) {
      printf("HERE IS A TRUTH TABLE FOR result1 FOR ZBDDC:\n");
      Biddy_Managed_PrintfTable(MNGZBDDC,result1);
      printf("HERE IS A TRUTH TABLE FOR result2 FOR ZBDDC:\n");
      Biddy_Managed_PrintfTable(MNGZBDDC,result2);
    }
    */

    result1 = Biddy_Managed_Copy(MNGOBDD,MNGTZBDD,result0);
    result2 = Biddy_Managed_Stretch(MNGTZBDD,Biddy_Copy(MNGTZBDD,result));
    printf("(STRETCH) Function result2 (MNGTZBDD) has %.0f minterms/combinations.\n",Biddy_Managed_CountMinterms(MNGTZBDD,result2,SIZE));

    if (Biddy_Managed_Diff(MNGTZBDD,result2,Biddy_Copy(MNGTZBDD,result)) != Biddy_Managed_GetEmptySet(MNGTZBDD)) {
      printf("ERROR: Operation Stretch has wrong result for MNGTZBDD\n");
    }

    if (result2 != result1) {
      printf("ERROR: Operation Stretch is wrong for MNGOBDD / MNGTZBDD\n");
    }

    /* TRUTH TABLE FOR TZBDD - DEBUGGING, ONLY */
    /*
    if (SIZE < 10) {
      printf("HERE IS A TRUTH TABLE FOR result1 FOR TZBDD:\n");
      Biddy_Managed_PrintfTable(MNGTZBDD,result1);
      Biddy_Managed_WriteBddview(MNGTZBDD,"result1.bddview",result1,"result1",NULL);
      printf("HERE IS A TRUTH TABLE FOR result2 FOR TZBDD:\n");
      Biddy_Managed_PrintfTable(MNGTZBDD,result2);
      Biddy_Managed_WriteBddview(MNGTZBDD,"result2.bddview",result2,"result2",NULL);
    }
    */

    /* OPERATION PERMITSYM */

    for (i = 0; i<=SIZE; i++) {

      result0 = Biddy_Managed_Permitsym(MNGOBDD,Biddy_Copy(MNGOBDD,result),i);
      printf("(PERMITSYM) Function result0 has %.0f minterms/combinations with up to %u elements.\n",Biddy_Managed_CountMinterms(MNGOBDD,result0,SIZE),i);
      /* printf("(PERMITSYM) Function result0 (MNGOBDD) has %.0f minterms/combinations with up to %u elements.\n",Biddy_Managed_CountMinterms(MNGOBDD,result0,SIZE),i); */

      /* TRUTH TABLE FOR OBDD - DEBUGGING, ONLY */
      /*
      if (SIZE < 10) {
        printf("HERE IS A TRUTH TABLE FOR result FOR OBDD:\n");
        Biddy_Managed_PrintfTable(MNGOBDD,Biddy_Copy(MNGOBDD,result));
        Biddy_Managed_WriteBddview(MNGOBDD,"result.bddview",Biddy_Copy(MNGOBDD,result),"result",NULL);
        printf("HERE IS A TRUTH TABLE FOR result0 FOR OBDD:\n");
        Biddy_Managed_PrintfTable(MNGOBDD,result0);
        Biddy_Managed_WriteBddview(MNGOBDD,"result0.bddview",result0,"result0",NULL);
      }
      */

      result1 = Biddy_Managed_Copy(MNGOBDD,MNGOBDDC,result0);
      result2 = Biddy_Managed_Permitsym(MNGOBDDC,Biddy_Copy(MNGOBDDC,result),i);
      /* printf("(PERMITSYM) Function result2 (MNGOBDDC) has %.0f minterms/combinations with up to %u elements.\n",Biddy_Managed_CountMinterms(MNGOBDDC,result2,SIZE),i); */

      if (result2 != result1) {
        printf("ERROR: Operation Permitsym is wrong for MNGOBDD / MNGOBDDC and n = %u\n",i);
      }

      /* TRUTH TABLE FOR OBDDC - DEBUGGING, ONLY */
      /*
      if (SIZE < 10) {
        printf("HERE IS A TRUTH TABLE FOR result1 FOR OBDDC:\n");
        Biddy_Managed_PrintfTable(MNGOBDDC,result1);
        printf("HERE IS A TRUTH TABLE FOR result2 FOR OBDDC:\n");
        Biddy_Managed_PrintfTable(MNGOBDDC,result2);
      }
      */

      result1 = Biddy_Managed_Copy(MNGOBDD,MNGZBDD,result0);
      result2 = Biddy_Managed_Permitsym(MNGZBDD,Biddy_Copy(MNGZBDD,result),i);
      /* printf("(PERMITSYM) Function result2 (MNGZBDD) has %.0f minterms/combinations with up to %u elements.\n",Biddy_Managed_CountMinterms(MNGZBDD,result2,SIZE),i); */

      if (result2 != result1) {
        printf("ERROR: Operation Permitsym is wrong for MNGOBDD / MNGZBDD and n = %u\n",i);
      }

      /* TRUTH TABLE FOR ZBDD - DEBUGGING, ONLY */
      /*
      if (SIZE < 10) {
        printf("HERE IS A TRUTH TABLE FOR result1 FOR ZBDD:\n");
        Biddy_Managed_PrintfTable(MNGZBDD,result1);
        printf("HERE IS A TRUTH TABLE FOR result2 FOR ZBDD:\n");
        Biddy_Managed_PrintfTable(MNGZBDD,result2);
      }
      */

      result1 = Biddy_Managed_Copy(MNGOBDD,MNGZBDDC,result0);
      result2 = Biddy_Managed_Permitsym(MNGZBDDC,Biddy_Copy(MNGZBDDC,result),i);
      /* printf("(PERMITSYM) Function result2 (MNGZBDDC) has %.0f minterms/combinations with up to %u elements.\n",Biddy_Managed_CountMinterms(MNGZBDDC,result2,SIZE),i); */

      if (result2 != result1) {
        printf("ERROR: Operation Permitsym is wrong for MNGOBDD / MNGZBDDC and n = %u\n",i);
      }

      /* TRUTH TABLE FOR ZBDDC - DEBUGGING, ONLY */
      /*
      if (SIZE < 10) {
        printf("HERE IS A TRUTH TABLE FOR result1 FOR ZBDDC:\n");
        Biddy_Managed_PrintfTable(MNGZBDDC,result1);
        printf("HERE IS A TRUTH TABLE FOR result2 FOR ZBDDC:\n");
        Biddy_Managed_PrintfTable(MNGZBDDC,result2);
      }
      */

      result1 = Biddy_Managed_Copy(MNGOBDD,MNGTZBDD,result0);
      result2 = Biddy_Managed_Permitsym(MNGTZBDD,Biddy_Copy(MNGTZBDD,result),i);
      /* printf("(PERMITSYM) Function result2 (MNGTZBDD) has %.0f minterms/combinations with up to %u elements.\n",Biddy_Managed_CountMinterms(MNGTZBDD,result2,SIZE),i); */

      if (result2 != result1) {
        printf("ERROR: Operation Permitsym is wrong for MNGOBDD / MNGTZBDD and n = %u\n",i);
      }

      /* TRUTH TABLE FOR TZBDD - DEBUGGING, ONLY */
      /*
      if (SIZE < 10) {
        printf("HERE IS A TRUTH TABLE FOR result1 FOR TZBDD:\n");
        Biddy_Managed_PrintfTable(MNGTZBDD,result1);
        Biddy_Managed_WriteBddview(MNGTZBDD,"result1.bddview",result1,"result1",NULL);
        printf("HERE IS A TRUTH TABLE FOR result2 FOR TZBDD:\n");
        Biddy_Managed_PrintfTable(MNGTZBDD,result2);
        Biddy_Managed_WriteBddview(MNGTZBDD,"result2.bddview",result2,"result2",NULL);
        if (result2 != result1) exit(1);
      }
      */

    }

    /* OPERATION PRODUCT */

    result0 = Biddy_Managed_Product(MNGOBDD,Biddy_Copy(MNGOBDD,result),Biddy_Copy(MNGOBDD,coresult));
    printf("(PRODUCT) Function result0 (MNGOBDD) has %.0f minterms/combinations.\n",Biddy_Managed_CountMinterms(MNGOBDD,result0,SIZE));

    /* TRUTH TABLE FOR OBDD - DEBUGGING, ONLY */
    /*
    if (SIZE < 10) {
      printf("HERE IS A TRUTH TABLE FOR OBDD:\n");
      Biddy_Managed_PrintfTable(MNGOBDD,result0);
    }
    */

    result1 = Biddy_Managed_Copy(MNGOBDD,MNGOBDDC,result0);
    result2 = Biddy_Managed_Product(MNGOBDDC,Biddy_Copy(MNGOBDDC,result),Biddy_Copy(MNGOBDDC,coresult));
    printf("(PRODUCT) Function result2 (MNGOBDDC) has %.0f minterms/combinations.\n",Biddy_Managed_CountMinterms(MNGOBDDC,result2,SIZE));

    if (result2 != result1) {
      printf("ERROR: Operation Product is wrong for MNGOBDD / MNGOBDDC\n");
    }

    /* TRUTH TABLE FOR OBDDC - DEBUGGING, ONLY */
    /*
    if (SIZE < 10) {
      printf("HERE IS A TRUTH TABLE FOR result1 FOR OBDDC:\n");
      Biddy_Managed_PrintfTable(MNGOBDDC,result1);
      printf("HERE IS A TRUTH TABLE FOR result2 FOR OBDDC:\n");
      Biddy_Managed_PrintfTable(MNGOBDDC,result2);
    }
    */

    result1 = Biddy_Managed_Copy(MNGOBDD,MNGZBDD,result0);
    result2 = Biddy_Managed_Product(MNGZBDD,Biddy_Copy(MNGZBDD,result),Biddy_Copy(MNGZBDD,coresult));
    printf("(PRODUCT) Function result2 (MNGZBDD) has %.0f minterms/combinations.\n",Biddy_Managed_CountMinterms(MNGZBDD,result2,SIZE));

    if (result2 != result1) {
      printf("ERROR: Operation Product is wrong for MNGOBDD / MNGZBDD\n");
    }

    /* TRUTH TABLE FOR ZBDD - DEBUGGING, ONLY */
    /*
    if (SIZE < 10) {
      printf("HERE IS A TRUTH TABLE FOR result1 FOR ZBDD:\n");
      Biddy_Managed_PrintfTable(MNGZBDD,result1);
      printf("HERE IS A TRUTH TABLE FOR result2 FOR ZBDD:\n");
      Biddy_Managed_PrintfTable(MNGZBDD,result2);
    }
    */

    result1 = Biddy_Managed_Copy(MNGOBDD,MNGZBDDC,result0);
    result2 = Biddy_Managed_Product(MNGZBDDC,Biddy_Copy(MNGZBDDC,result),Biddy_Copy(MNGZBDDC,coresult));
    printf("(PRODUCT) Function result2 (MNGZBDDC) has %.0f minterms/combinations.\n",Biddy_Managed_CountMinterms(MNGZBDDC,result2,SIZE));

    if (result2 != result1) {
      printf("ERROR: Operation Product is wrong for MNGOBDD / MNGZBDDC\n");
    }

    /* TRUTH TABLE FOR ZBDDC - DEBUGGING, ONLY */
    /*
    if (SIZE < 10) {
      printf("HERE IS A TRUTH TABLE FOR result1 FOR ZBDDC:\n");
      Biddy_Managed_PrintfTable(MNGZBDDC,result1);
      printf("HERE IS A TRUTH TABLE FOR result2 FOR ZBDDC:\n");
      Biddy_Managed_PrintfTable(MNGZBDDC,result2);
    }
    */

    result1 = Biddy_Managed_Copy(MNGOBDD,MNGTZBDD,result0);
    result2 = Biddy_Managed_Product(MNGTZBDD,Biddy_Copy(MNGTZBDD,result),Biddy_Copy(MNGTZBDD,coresult));
    printf("(PRODUCT) Function result2 (MNGTZBDD) has %.0f minterms/combinations.\n",Biddy_Managed_CountMinterms(MNGTZBDD,result2,SIZE));

    if (result2 != result1) {
      printf("ERROR: Operation Product is wrong for MNGOBDD / MNGTZBDD\n");
    }

    /* TRUTH TABLE FOR TZBDD - DEBUGGING, ONLY */
    /*
    if (SIZE < 10) {
      printf("HERE IS A TRUTH TABLE FOR result1 FOR TZBDD:\n");
      Biddy_Managed_PrintfTable(MNGTZBDD,result1);
      printf("HERE IS A TRUTH TABLE FOR result2 FOR TZBDD:\n");
      Biddy_Managed_PrintfTable(MNGTZBDD,result2);
    }
    */

    /* OPERATION SELECTIVE PRODUCT */

    /* DEBUGGING */
    /*
    for (i = 1; i <= SIZE; i++) {
      printf("(SELECTIVE PRODUCT - ROBDD): variable %s (%u)\n",Biddy_Managed_GetVariableName(MNGOBDD,i),i);
      printf("(SELECTIVE PRODUCT - ROBDDC): variable %s (%u)\n",Biddy_Managed_GetVariableName(MNGOBDDC,i),i);
      printf("(SELECTIVE PRODUCT - ZBDD): variable %s (%u)\n",Biddy_Managed_GetVariableName(MNGZBDD,i),i);
      printf("(SELECTIVE PRODUCT - ZBDDC): variable %s (%u)\n",Biddy_Managed_GetVariableName(MNGZBDDC,i),i);
    }
    */

    /* EMPTY POSITIVE SET AND NEGATIVE SET */
    cuberobdd = Biddy_Managed_GetConstantOne(MNGOBDD);
    cuberobddc = Biddy_Managed_GetConstantOne(MNGOBDDC);
    cubezbdd = Biddy_Managed_GetBaseSet(MNGZBDD);
    cubezbddc = Biddy_Managed_GetBaseSet(MNGZBDDC);

    /* ALL IN POSITIVE SET */
    /*
    for (i = 1; i <= SIZE; i++) {
      cuberobdd = Biddy_Managed_And(MNGOBDD,cuberobdd,Biddy_Managed_GetVariableEdge(MNGOBDD,i));
      cuberobddc = Biddy_Managed_And(MNGOBDDC,cuberobddc,Biddy_Managed_GetVariableEdge(MNGOBDDC,i));
      cubezbdd = Biddy_Managed_Product(MNGZBDD,cubezbdd,Biddy_Managed_GetElementEdge(MNGZBDD,i));
      cubezbddc = Biddy_Managed_Product(MNGZBDDC,cubezbddc,Biddy_Managed_GetElementEdge(MNGZBDDC,i));
    }
    */

    /* ALL IN NEGATIVE SET */
    /*
    for (i = 1; i <= SIZE; i++) {
      cuberobdd = Biddy_Managed_Gt(MNGOBDD,cuberobdd,Biddy_Managed_GetVariableEdge(MNGOBDD,i));
      cuberobddc = Biddy_Managed_Gt(MNGOBDDC,cuberobddc,Biddy_Managed_GetVariableEdge(MNGOBDDC,i));
      cubezbdd = Biddy_Managed_E(MNGZBDD,cubezbdd,i);
      cubezbddc = Biddy_Managed_E(MNGZBDDC,cubezbddc,i);
    }
    */

    /* RANDOM POSITIVE SET AND NEGATIVE SET */
    for (i = 1; i <= SIZE; i++) {
      switch (rand()%3) {
        case 0: /* printf("(SELECTIVE PRODUCT): variable %s (%u) NEUTRAL\n",Biddy_Managed_GetVariableName(MNGOBDD,i),i); */
        break;
        case 1: /* printf("(SELECTIVE PRODUCT): variable %s (%u) POSITIVE\n",Biddy_Managed_GetVariableName(MNGOBDD,i),i); */
                cuberobdd = Biddy_Managed_And(MNGOBDD,cuberobdd,Biddy_Managed_GetVariableEdge(MNGOBDD,i));
                cuberobddc = Biddy_Managed_And(MNGOBDDC,cuberobddc,Biddy_Managed_GetVariableEdge(MNGOBDDC,i));
                cubezbdd = Biddy_Managed_Product(MNGZBDD,cubezbdd,Biddy_Managed_GetElementEdge(MNGZBDD,i));
                cubezbddc = Biddy_Managed_Product(MNGZBDDC,cubezbddc,Biddy_Managed_GetElementEdge(MNGZBDDC,i));
        break;
        case 2: /* printf("(SELECTIVE PRODUCT): variable %s (%u) NEGATIVE\n",Biddy_Managed_GetVariableName(MNGOBDD,i),i); */
                cuberobdd = Biddy_Managed_Gt(MNGOBDD,cuberobdd,Biddy_Managed_GetVariableEdge(MNGOBDD,i));
                cuberobddc = Biddy_Managed_Gt(MNGOBDDC,cuberobddc,Biddy_Managed_GetVariableEdge(MNGOBDDC,i));
                cubezbdd = Biddy_Managed_E(MNGZBDD,cubezbdd,i);
                cubezbddc = Biddy_Managed_E(MNGZBDDC,cubezbddc,i);
        break;
      }
    }

    /* DEBUGGING */
    /*
    Biddy_Managed_WriteBddview(MNGOBDD,"cuberobdd.bddview",cuberobdd,"cuberobdd",NULL);
    Biddy_Managed_WriteBddview(MNGOBDDC,"cuberobddc.bddview",cuberobddc,"cuberobddc",NULL);
    Biddy_Managed_WriteBddview(MNGZBDD,"cubezbdd.bddview",cubezbdd,"cubezbdd",NULL);
    Biddy_Managed_WriteBddview(MNGZBDDC,"cubezbddc.bddview",cubezbddc,"cubezbddc",NULL);
    */

    result0 = Biddy_Managed_SelectiveProduct(MNGOBDD,Biddy_Copy(MNGOBDD,result),Biddy_Copy(MNGOBDD,coresult),cuberobdd);
    printf("(SELECTIVE PRODUCT) Function result0 (MNGOBDD) has %.0f minterms/combinations.\n",Biddy_Managed_CountMinterms(MNGOBDD,result0,SIZE));

    /* TRUTH TABLE FOR OBDD - DEBUGGING, ONLY */
    /*
    if (SIZE < 10) {
      printf("HERE IS A TRUTH TABLE FOR OBDD:\n");
      Biddy_Managed_PrintfTable(MNGOBDD,result0);
    }
    */

    result1 = Biddy_Managed_Copy(MNGOBDD,MNGOBDDC,result0);
    result2 = Biddy_Managed_SelectiveProduct(MNGOBDDC,Biddy_Copy(MNGOBDDC,result),Biddy_Copy(MNGOBDDC,coresult),cuberobddc);
    printf("(SELECTIVE PRODUCT) Function result2 (MNGOBDDC) has %.0f minterms/combinations.\n",Biddy_Managed_CountMinterms(MNGOBDDC,result2,SIZE));

    if (result2 != result1) {
      printf("ERROR: Operation SelectiveProduct is wrong for MNGOBDD / MNGOBDDC\n");
    }

    /* TRUTH TABLE FOR OBDDC - DEBUGGING, ONLY */
    /*
    if (SIZE < 10) {
      printf("HERE IS A TRUTH TABLE FOR result1 FOR OBDDC:\n");
      Biddy_Managed_PrintfTable(MNGOBDDC,result1);
      printf("HERE IS A TRUTH TABLE FOR result2 FOR OBDDC:\n");
      Biddy_Managed_PrintfTable(MNGOBDDC,result2);
    }
    */

    result1 = Biddy_Managed_Copy(MNGOBDD,MNGZBDD,result0);
    result2 = Biddy_Managed_SelectiveProduct(MNGZBDD,Biddy_Copy(MNGZBDD,result),Biddy_Copy(MNGZBDD,coresult),cubezbdd);
    printf("(SELECTIVE PRODUCT) Function result2 (MNGZBDD) has %.0f minterms/combinations.\n",Biddy_Managed_CountMinterms(MNGZBDD,result2,SIZE));

    if (result2 != result1) {
      printf("ERROR: Operation SelectiveProduct is wrong for MNGOBDD / MNGZBDD\n");
    }

    /* TRUTH TABLE FOR ZBDD - DEBUGGING, ONLY */
    /*
    if (SIZE < 10) {
      printf("HERE IS A TRUTH TABLE FOR result1 FOR ZBDD:\n");
      Biddy_Managed_PrintfTable(MNGZBDD,result1);
      printf("HERE IS A TRUTH TABLE FOR result2 FOR ZBDD:\n");
      Biddy_Managed_PrintfTable(MNGZBDD,result2);
    }
    */

    result1 = Biddy_Managed_Copy(MNGOBDD,MNGZBDDC,result0);
    result2 = Biddy_Managed_SelectiveProduct(MNGZBDDC,Biddy_Copy(MNGZBDDC,result),Biddy_Copy(MNGZBDDC,coresult),cubezbddc);
    printf("(SELECTIVE PRODUCT) Function result2 (MNGZBDDC) has %.0f minterms/combinations.\n",Biddy_Managed_CountMinterms(MNGZBDDC,result2,SIZE));

    if (result2 != result1) {
      printf("ERROR: Operation SelectiveProduct is wrong for MNGOBDD / MNGZBDDC\n");
    }

    /* TRUTH TABLE FOR ZBDDC - DEBUGGING, ONLY */
    /*
    if (SIZE < 10) {
      printf("HERE IS A TRUTH TABLE FOR result1 FOR ZBDDC:\n");
      Biddy_Managed_PrintfTable(MNGZBDDC,result1);
      printf("HERE IS A TRUTH TABLE FOR result2 FOR ZBDDC:\n");
      Biddy_Managed_PrintfTable(MNGZBDDC,result2);
    }
    */

    /* ************************************************************* */
    /* CONVERSIONS */
    /* ************************************************************* */

    printf("********* CONVERSION TESTS *********\n");

    /* CONVERT result INTO OBDD */
    robdd = Biddy_Copy(MNGOBDD, result);
    printf("OBDD for function result has %u nodes (including both terminals).\n", Biddy_Managed_CountNodes(MNGOBDD, robdd));
    printf("OBDD for function result has %u plain nodes (including both terminals).\n", Biddy_Managed_CountNodesPlain(MNGOBDD, robdd));
    printf("OBDD for function result has %llu one-paths.\n", Biddy_Managed_CountPaths(MNGOBDD, robdd));
    printf("Function represented by OBDD depends on %u variables.\n", Biddy_Managed_DependentVariableNumber(MNGOBDD, robdd, FALSE));
    printf("Considering the given set of variables, OBDD for function result has %.0f minterms/combinations.\n", Biddy_Managed_CountMinterms(MNGOBDD, robdd, SIZE));
    /* GRAPHVIZ/DOT OUTPUT OF THE RESULT - DEBUGGING, ONLY */
    /*
    if (SIZE < 10) {
      Biddy_Managed_WriteDot(MNGOBDD, "obdd.dot", robdd, "obdd", -1, FALSE);
      printf("USE 'dot -y -Tpng -O obdd.dot' to visualize %s for function result.\n", Biddy_Managed_GetManagerName(MNGOBDD));
    }
    */

    /* CONVERT result INTO OBDDC */
    robddc = Biddy_Copy(MNGOBDDC,result);
    printf("OBDDC for function result has %u nodes (including terminal).\n",Biddy_Managed_CountNodes(MNGOBDDC,robddc));
    printf("OBDDC for function result has %u plain nodes (including both terminals).\n",Biddy_Managed_CountNodesPlain(MNGOBDDC,robddc));
    printf("OBDDC for function result has %llu one-paths.\n",Biddy_Managed_CountPaths(MNGOBDDC,robddc));
    printf("Function represented by OBDDC depends on %u variables.\n",Biddy_Managed_DependentVariableNumber(MNGOBDDC,robddc,FALSE));
    printf("Considering the given set of variables, OBDDC for function result has %.0f minterms/combinations.\n",Biddy_Managed_CountMinterms(MNGOBDDC,robddc,SIZE));
    /* GRAPHVIZ/DOT OUTPUT OF THE RESULT - DEBUGGING, ONLY */
    /*
    if (SIZE < 10) {
      Biddy_Managed_WriteDot(MNGOBDDC,"obddce.dot",robddc,"obddce",-1,FALSE);
      printf("USE 'dot -y -Tpng -O obddce.dot' to visualize %s for function result.\n",Biddy_Managed_GetManagerName(MNGOBDDC));
    }
    */

    /* CONVERT result INTO ZBDD */
    rzbdd = Biddy_Copy(MNGZBDD,result);
    printf("ZBDD for function result has %u nodes (including both terminals).\n",Biddy_Managed_CountNodes(MNGZBDD,rzbdd));
    printf("ZBDD for function result has %u plain nodes (including both terminals).\n",Biddy_Managed_CountNodesPlain(MNGZBDD,rzbdd));
    printf("ZBDD for function result has %llu one-paths.\n",Biddy_Managed_CountPaths(MNGZBDD,rzbdd));
    printf("Function represented by ZBDD depends on %u variables.\n",Biddy_Managed_DependentVariableNumber(MNGZBDD,rzbdd,FALSE));
    printf("Considering the given set of variables, ZBDD for function result has %.0f minterms/combinations.\n",Biddy_Managed_CountMinterms(MNGZBDD,rzbdd,SIZE));
    /* GRAPHVIZ/DOT OUTPUT OF THE RESULT - DEBUGGING, ONLY */
    /*
    if (SIZE < 10) {
      Biddy_Managed_WriteDot(MNGZBDD,"zbdd.dot",rzbdd,"zbdd",-1,FALSE);
      printf("USE 'dot -y -Tpng -O zbdd.dot' to visualize %s for function result.\n",Biddy_Managed_GetManagerName(MNGZBDD));
    }
    */

    /* CONVERT result INTO ZBDDC */
    rzbddc = Biddy_Copy(MNGZBDDC, result);
    printf("ZBDDC for function result has %u nodes (including terminal).\n", Biddy_Managed_CountNodes(MNGZBDDC, rzbddc));
    printf("ZBDDC for function result has %u plain nodes (including both terminals).\n", Biddy_Managed_CountNodesPlain(MNGZBDDC, rzbddc));
    printf("ZBDDC for function result has %llu one-paths.\n", Biddy_Managed_CountPaths(MNGZBDDC, rzbddc));
    printf("Function represented by ZBDDC depends on %u variables.\n", Biddy_Managed_DependentVariableNumber(MNGZBDDC, rzbddc, FALSE));
    printf("Considering the given set of variables, ZBDDC for function result has %.0f minterms/combinations.\n", Biddy_Managed_CountMinterms(MNGZBDDC, rzbddc, SIZE));
    /* GRAPHVIZ/DOT OUTPUT OF THE RESULT - DEBUGGING, ONLY */
    /*
    if (SIZE < 10) {
      Biddy_Managed_WriteDot(MNGZBDDC, "zbddce.dot", rzbddc, "zbddce", -1, FALSE);
      printf("USE 'dot -y -Tpng -O zbddce.dot' to visualize %s for function result.\n", Biddy_Managed_GetManagerName(MNGZBDDC));
    }
    */

    /* CONVERT result INTO TZBDDs */
    rtzbdd = Biddy_Copy(MNGTZBDD,result);
    printf("TZBDD for function result has %u nodes (including both terminals).\n",Biddy_Managed_CountNodes(MNGTZBDD,rtzbdd));
    printf("TZBDD for function result has %llu one-paths.\n",Biddy_Managed_CountPaths(MNGTZBDD,rtzbdd));
    printf("Function represented by TZBDD depends on %u variables.\n",Biddy_Managed_DependentVariableNumber(MNGTZBDD,rtzbdd,FALSE));
    printf("Considering the given set of variables, TZBDD for function result has %.0f minterms/combinations.\n",Biddy_Managed_CountMinterms(MNGTZBDD,rtzbdd,SIZE));
    /* GRAPHVIZ/DOT OUTPUT OF THE RESULT - DEBUGGING, ONLY */
    /*
    if (SIZE < 10) {
      Biddy_Managed_WriteDot(MNGTZBDD,"tzbdd.dot",rtzbdd,"tzbdd",-1,FALSE);
      printf("USE 'dot -y -Tpng -O tzbdd.dot' to visualize %s for function result.\n",Biddy_Managed_GetManagerName(MNGTZBDD));
    }
    */

    /* ************************************************************* */
    /* REPLACE AND ABSTRACTIONS */
    /* WE NEED extra VARIABLE THUS THESE TESTS ARE AT THE END +/
    /* ************************************************************* */

    printf("********* REPLACE AND ABSTRACTIONS *********\n");

    if ((Biddy_GetManagerType() == BIDDYTYPEOBDD) || (Biddy_GetManagerType() == BIDDYTYPEOBDDC)) {
      tmp = Biddy_AddVariableEdge();
    }
    if ((Biddy_GetManagerType() == BIDDYTYPEZBDD) || (Biddy_GetManagerType() == BIDDYTYPEZBDDC)) {
      tmp = Biddy_GetElementEdge(Biddy_AddElement());
    }
    if ((Biddy_GetManagerType() == BIDDYTYPETZBDD) || (Biddy_GetManagerType() == BIDDYTYPETZBDDC)) {
      tmp = Biddy_AddVariableEdge();
    }
    extra = Biddy_GetTopVariable(tmp);

    /* make sure, that result does not depend on variable extra */
    /* this is neccessary for ZBDDs, because result is not refreshed */
    /* after adding variable extra */
    result = Biddy_E(result,extra);

    /* OPERATION REPLACE */
    /* Replacing is controlled by variable's values (which are edges!). */
    /* For OBDDs and TZBDDs control values must be variables, but for ZBDDs they */
    /* must be elements! Use Biddy_ResetVariablesValue and Biddy_SetVariableValue */
    /* to prepare control values. */

    Biddy_ResetVariablesValue();
    if ((Biddy_GetManagerType() == BIDDYTYPEOBDD) || (Biddy_GetManagerType() == BIDDYTYPEOBDDC)) {
      Biddy_SetVariableValue(first, Biddy_GetVariableEdge(extra));
    }
    if ((Biddy_GetManagerType() == BIDDYTYPEZBDD) || (Biddy_GetManagerType() == BIDDYTYPEZBDDC)) {
      Biddy_SetVariableValue(first, Biddy_GetElementEdge(extra));
    }
    if ((Biddy_GetManagerType() == BIDDYTYPETZBDD) || (Biddy_GetManagerType() == BIDDYTYPETZBDDC)) {
      Biddy_SetVariableValue(first, Biddy_GetVariableEdge(extra));
    }

    result0 = Biddy_Replace(result);

    Biddy_ResetVariablesValue();
    if ((Biddy_GetManagerType() == BIDDYTYPEOBDD) || (Biddy_GetManagerType() == BIDDYTYPEOBDDC)) {
      Biddy_SetVariableValue(extra, Biddy_GetVariableEdge(first));
    }
    if ((Biddy_GetManagerType() == BIDDYTYPEZBDD) || (Biddy_GetManagerType() == BIDDYTYPEZBDDC)) {
      Biddy_SetVariableValue(extra, Biddy_GetElementEdge(first));
    }
    if ((Biddy_GetManagerType() == BIDDYTYPETZBDD) || (Biddy_GetManagerType() == BIDDYTYPETZBDDC)) {
      Biddy_SetVariableValue(extra, Biddy_GetVariableEdge(first));
    }

    result1 = Biddy_Replace(result0);

    if (result1 != result) {
      printf("ERROR: Operation Replace is wrong!\n");
    }

    /* GRAPHVIZ/DOT OUTPUT OF RESULT0 AND RESULT1 - DEBUGGING, ONLY */
    /*
    if (SIZE < 10) {
      Biddy_WriteDot("result.dot", result, "result", -1, FALSE);
      printf("USE 'dot -y -Tpng -O result.dot' to visualize function result.\n");
      Biddy_WriteDot("result0.dot",result0,"result0",-1,FALSE);
      printf("USE 'dot -y -Tpng -O result0.dot' to visualize function result0.\n");
      Biddy_WriteDot("result1.dot",result1,"result1",-1,FALSE);
      printf("USE 'dot -y -Tpng -O result1.dot' to visualize function result1.\n");
      exit(1);
    }
    */

    /* OPERATION ABSTRACTION */

    /* calculate cube */

    if ((Biddy_GetManagerType() == BIDDYTYPEOBDD) || (Biddy_GetManagerType() == BIDDYTYPEOBDDC)) {
      cube = Biddy_GetConstantOne();
      cube = Biddy_And(cube, Biddy_GetVariableEdge(second));
      cube = Biddy_And(cube, Biddy_GetVariableEdge(last));
      cube = Biddy_And(cube, Biddy_GetVariableEdge(extra));
    }

    if ((Biddy_GetManagerType() == BIDDYTYPEZBDD) || (Biddy_GetManagerType() == BIDDYTYPEZBDDC)) {
      cube = Biddy_GetBaseSet();
      cube = Biddy_Change(cube,second);
      if (last != first) cube = Biddy_Change(cube, last);
      cube = Biddy_Change(cube, extra);
    }

    if ((Biddy_GetManagerType() == BIDDYTYPETZBDD) || (Biddy_GetManagerType() == BIDDYTYPETZBDDC)) {
      cube = Biddy_GetConstantOne();
      cube = Biddy_And(cube, Biddy_GetVariableEdge(second));
      cube = Biddy_And(cube, Biddy_GetVariableEdge(last));
      cube = Biddy_And(cube, Biddy_GetVariableEdge(extra));
    }

    result1 = Biddy_E(result,second);
    result1 = Biddy_E(result1,last);
    result2 = Biddy_ExistAbstract(result,cube);

    printf("After the existential abstraction of second and last variable, function result has %.0f minterms/combinations\n", Biddy_CountMinterms(result2, SIZE));

    if (result1 != result2) {
      printf("ERROR: Operation Abstraction (1) is wrong!\n");
    }

    /* GRAPHVIZ/DOT OUTPUT OF RESULT1 - DEBUGGING, ONLY */
    /*
    if (SIZE < 10) {
      Biddy_WriteDot("result.dot", result, "result", -1, FALSE);
      printf("USE 'dot -y -Tpng -O result.dot' to visualize function result.\n");
      Biddy_WriteDot("result1.dot",result1,"result1",-1,FALSE);
      printf("USE 'dot -y -Tpng -O result1.dot' to visualize function result1.\n");
      Biddy_WriteDot("result2.dot", result2, "result2", -1, FALSE);
      printf("USE 'dot -y -Tpng -O result2.dot' to visualize function result2.\n");
      Biddy_WriteDot("baseset.dot", Biddy_GetBaseSet(), "baseset", -1, FALSE);
      printf("USE 'dot -y -Tpng -O baseset.dot' to visualize function baseset.\n");
      Biddy_WriteDot("cube.dot", cube, "cube", -1, FALSE);
      printf("USE 'dot -y -Tpng -O cube.dot' to visualize function cube.\n");
      exit(1);
    }
    */

    result1 = Biddy_A(result,second);
    result1 = Biddy_A(result1,last);
    result2 = Biddy_UnivAbstract(result,cube);

    printf("After the universal abstraction of second and last variable, function result has %.0f minterms/combinations\n", Biddy_CountMinterms(result2, SIZE));

    if (result1 != result2) {
      printf("ERROR: Operation Abstraction (2) is wrong!\n");
    }

    /* GRAPHVIZ/DOT OUTPUT OF RESULT1  - DEBUGGING, ONLY */
    /*
    if (SIZE < 10) {
      Biddy_WriteDot("result1.dot",result1,"result1",-1,FALSE);
      printf("USE 'dot -y -Tpng -O result1.dot' to visualize function result1.\n");
      exit(1);
    }
    */

    result0 = Biddy_E(result,second);
    result1 = Biddy_E(Biddy_Not(result),last);
    result2 = Biddy_And(result0,result1);
    result2 = Biddy_ExistAbstract(result2,cube);
    result1 = Biddy_AndAbstract(result0,result1,cube);

    printf("After the first user-defined abstraction, function result has %.0f minterms/combinations\n", Biddy_CountMinterms(result1, SIZE));

    if (result1 != result2) {
      printf("ERROR: Operation Abstraction (3) is wrong!\n");
    }

    result0 = Biddy_A(result,second);
    result1 = Biddy_A(Biddy_Not(result),last);
    result1 = Biddy_AndAbstract(result0,result1,cube);

    printf("After the second user-defined abstraction, function result has %.0f minterms/combinations\n", Biddy_CountMinterms(result1, SIZE));

    if (result1 != Biddy_GetConstantZero()) {
      printf("ERROR: Operation Abstraction (4) is wrong!\n");
    }

    Biddy_Exit();
    Biddy_ExitMNG(&MNGOBDD);
    Biddy_ExitMNG(&MNGOBDDC);
    Biddy_ExitMNG(&MNGZBDD);
    Biddy_ExitMNG(&MNGZBDDC);
    Biddy_ExitMNG(&MNGTZBDD);

  } else {

    /* ************************************************************* */
    /* STATISTICS */
    /* ************************************************************* */

    obddsize = zbddsize = tzbddsize = 0;
    obddminsize = zbddminsize = tzbddminsize = 0;
    obddmaxsize = zbddmaxsize = tzbddmaxsize = 0;
    obddsmallest = zbddsmallest = tzbddsmallest = 0;
    obddlargest = zbddlargest = tzbddlargest = 0;
    allsamesize = 0;

    for (i=0; i<POPULATION; i++) {

      Biddy_InitMNG(&MNGOBDD,BIDDYTYPEOBDD);
      Biddy_InitMNG(&MNGZBDD,BIDDYTYPEZBDD);
      Biddy_InitMNG(&MNGTZBDD,BIDDYTYPETZBDD);

      support = Biddy_Managed_GetConstantOne(MNGOBDD);
      for (v=0; v<SIZE; v++) {
        support = Biddy_Managed_And(MNGOBDD,
          support,Biddy_Managed_GetVariableEdge(MNGOBDD,Biddy_Managed_AddVariable(MNGOBDD)));
      }

      robdd = Biddy_Managed_RandomFunction(MNGOBDD,support,RATIO);
      rzbdd = Biddy_Managed_Copy(MNGOBDD,MNGZBDD,robdd);
      rtzbdd = Biddy_Managed_Copy(MNGOBDD,MNGTZBDD,robdd);

      s1 = Biddy_Managed_CountNodes(MNGOBDD,robdd);
      s2 = Biddy_Managed_CountNodes(MNGZBDD,rzbdd);
      s3 = Biddy_Managed_CountNodes(MNGTZBDD,rtzbdd);

      obddsize += s1;
      zbddsize += s2;
      tzbddsize += s3;

      if (obddminsize == 0) obddminsize = s1;
      if (zbddminsize == 0) zbddminsize = s2;
      if (tzbddminsize == 0) tzbddminsize = s3;

      if (s1 < obddminsize) obddminsize = s1;
      if (s2 < zbddminsize) zbddminsize = s2;
      if (s3 < tzbddminsize) tzbddminsize = s3;

      if (s1 > obddmaxsize) obddmaxsize = s1;
      if (s2 > zbddmaxsize) zbddmaxsize = s2;
      if (s3 > tzbddmaxsize) tzbddmaxsize = s3;

      if ((s1 < s2) && (s1 < s3)) obddsmallest++;
      if ((s2 < s1) && (s2 < s3)) zbddsmallest++;
      if ((s3 < s1) && (s3 < s2)) tzbddsmallest++;
      if ((s1 == s2) && (s1 < s3)) tzbddlargest++;
      if ((s1 == s3) && (s1 < s2)) zbddlargest++;
      if ((s2 == s3) && (s2 < s1)) obddlargest++;
      if ((s1 == s2) && (s2 == s3)) allsamesize++;

      Biddy_ExitMNG(&MNGOBDD);
      Biddy_ExitMNG(&MNGZBDD);
      Biddy_ExitMNG(&MNGTZBDD);

    }

    printf("RESULTS WITHOUT COMPLEMENTED EDGES FOR POPULATION=%u, SIZE=%u, RATIO=%.2e\n",POPULATION,SIZE,RATIO);
    printf("the average size of OBDDs = %.2f (min = %u, max = %u)\n",1.0*obddsize/POPULATION,obddminsize,obddmaxsize);
    printf("the average size of ZBDDs = %.2f (min = %u, max = %u)\n",1.0*zbddsize/POPULATION,zbddminsize,zbddmaxsize);
    printf("the average size of TZBDDs = %.2f (min = %u, max = %u)\n",1.0*tzbddsize/POPULATION,tzbddminsize,tzbddmaxsize);
    printf("OBDD is the smallest in %u examples (%.2f%%)\n",obddsmallest,100.0*obddsmallest/POPULATION);
    printf("ZBDD is the smallest in %u examples (%.2f%%)\n",zbddsmallest,100.0*zbddsmallest/POPULATION);
    printf("TZBDD is the smallest in %u examples (%.2f%%)\n",tzbddsmallest,100.0*tzbddsmallest/POPULATION);
    printf("OBDD and ZBDD are the same size and are smaller in %u examples (%.2f%%)\n",tzbddlargest,100.0*tzbddlargest/POPULATION);
    printf("OBDD and TZBDD are the same size and are smaller in %u examples (%.2f%%)\n",zbddlargest,100.0*zbddlargest/POPULATION);
    printf("ZBDD and TZBDD are the same size and are smaller in %u examples (%.2f%%)\n",obddlargest,100.0*obddlargest/POPULATION);
    printf("all three graphs have the same size in %u examples (%.2f%%)\n",allsamesize,100.0*allsamesize/POPULATION);

  }

}

/* RESULTS */
/*

RESULTS WITHOUT COMPLEMENTED EDGES FOR POPULATION=10000, SIZE=8, RATIO=1.00e-002
the average size of OBDDs = 18.81 (min = 10, max = 22)
the average size of ZBDDs = 11.33 (min = 3, max = 18)
the average size of TZBDDs = 11.28 (min = 3, max = 18)
OBDD is the smallest in 0 examples (0.00%)
ZBDD is the smallest in 0 examples (0.00%)
TZBDD is the smallest in 445 examples (4.45%)
OBDD and ZBDD are the same size and are smaller in 0 examples (0.00%)
OBDD and TZBDD are the same size and are smaller in 0 examples (0.00%)
ZBDD and TZBDD are the same size and are smaller in 9550 examples (95.50%)
all three graphs have the same size in 5 examples (0.05%)

RESULTS WITHOUT COMPLEMENTED EDGES FOR POPULATION=10000, SIZE=8, RATIO=1.00e-001
the average size of OBDDs = 48.17 (min = 38, max = 55)
the average size of ZBDDs = 38.31 (min = 31, max = 46)
the average size of TZBDDs = 36.61 (min = 29, max = 44)
OBDD is the smallest in 0 examples (0.00%)
ZBDD is the smallest in 0 examples (0.00%)
TZBDD is the smallest in 8560 examples (85.60%)
OBDD and ZBDD are the same size and are smaller in 0 examples (0.00%)
OBDD and TZBDD are the same size and are smaller in 0 examples (0.00%)
ZBDD and TZBDD are the same size and are smaller in 1440 examples (14.40%)
all three graphs have the same size in 0 examples (0.00%)

RESULTS WITHOUT COMPLEMENTED EDGES FOR POPULATION=10000, SIZE=8, RATIO=5.00e-001
the average size of OBDDs = 75.00 (min = 67, max = 79)
the average size of ZBDDs = 75.08 (min = 66, max = 79)
the average size of TZBDDs = 70.60 (min = 60, max = 76)
OBDD is the smallest in 0 examples (0.00%)
ZBDD is the smallest in 0 examples (0.00%)
TZBDD is the smallest in 9995 examples (99.95%)
OBDD and ZBDD are the same size and are smaller in 0 examples (0.00%)
OBDD and TZBDD are the same size and are smaller in 0 examples (0.00%)
ZBDD and TZBDD are the same size and are smaller in 5 examples (0.05%)
all three graphs have the same size in 0 examples (0.00%)

RESULTS WITHOUT COMPLEMENTED EDGES FOR POPULATION=10000, SIZE=8, RATIO=9.00e-001
the average size of OBDDs = 48.09 (min = 40, max = 55)
the average size of ZBDDs = 52.63 (min = 45, max = 59)
the average size of TZBDDs = 46.99 (min = 39, max = 54)
OBDD is the smallest in 0 examples (0.00%)
ZBDD is the smallest in 0 examples (0.00%)
TZBDD is the smallest in 10000 examples (100.00%)
OBDD and ZBDD are the same size and are smaller in 0 examples (0.00%)
OBDD and TZBDD are the same size and are smaller in 0 examples (0.00%)
ZBDD and TZBDD are the same size and are smaller in 0 examples (0.00%)
all three graphs have the same size in 0 examples (0.00%)

RESULTS WITHOUT COMPLEMENTED EDGES FOR POPULATION=10000, SIZE=8, RATIO=9.90e-001
the average size of OBDDs = 18.93 (min = 11, max = 22)
the average size of ZBDDs = 24.23 (min = 17, max = 27)
the average size of TZBDDs = 17.93 (min = 9, max = 21)
OBDD is the smallest in 0 examples (0.00%)
ZBDD is the smallest in 0 examples (0.00%)
TZBDD is the smallest in 8793 examples (87.93%)
OBDD and ZBDD are the same size and are smaller in 0 examples (0.00%)
OBDD and TZBDD are the same size and are smaller in 1207 examples (12.07%)
ZBDD and TZBDD are the same size and are smaller in 0 examples (0.00%)
all three graphs have the same size in 0 examples (0.00%)

---

RESULTS WITHOUT COMPLEMENTED EDGES FOR POPULATION=10000, SIZE=30, RATIO=1.00e-008
the average size of OBDDs = 267.68 (min = 240, max = 283)
the average size of ZBDDs = 139.47 (min = 101, max = 179)
the average size of TZBDDs = 139.47 (min = 101, max = 179)
OBDD is the smallest in 0 examples (0.00%)
ZBDD is the smallest in 0 examples (0.00%)
TZBDD is the smallest in 0 examples (0.00%)
OBDD and ZBDD are the same size and are smaller in 0 examples (0.00%)
OBDD and TZBDD are the same size and are smaller in 0 examples (0.00%)
ZBDD and TZBDD are the same size and are smaller in 10000 examples (100.00%)
all three graphs have the same size in 0 examples (0.00%)

*/

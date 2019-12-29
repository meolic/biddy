/* $Revision: 558 $ */
/* $Date: 2019-10-14 09:42:56 +0200 (pon, 14 okt 2019) $ */
/* This file (biddy-example-random.c) is a C file */
/* Author: Robert Meolic (robert@meolic.com) */
/* This file has been released into the public domain by the author. */

/* This example is compatible with Biddy v1.8 */

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

#define SIZE 4
#define RATIO 0.5

/* create Boolean function (BOOLEANFUNCTION) or combination set (COMBINATIONSET) */
#define RANDOMTYPE BOOLEANFUNCTION

/* choose primary BDD type */
#define BDDTYPE BIDDYTYPEOBDD

int main() {
  Biddy_Manager MNGOBDD,MNGOBDDC,MNGZBDD,MNGZBDDC,MNGTZBDD;
  Biddy_Edge tmp,support,cube,result,result0,result1,result2;
  Biddy_Edge robdd,robddc,rzbdd,rzbddc,rtzbdd;
  Biddy_Variable v,first,last,extra;
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

    /* for OBDDs, support is the most easiest calculated using AddVariable and AND */
    if ((Biddy_GetManagerType() == BIDDYTYPEOBDD) || (Biddy_GetManagerType() == BIDDYTYPEOBDDC)) {
      tmp = Biddy_AddVariableEdge();
      support = tmp;
      first = Biddy_GetTopVariable(tmp);
      for (v=1; v<SIZE; v++) {
        tmp = Biddy_AddVariableEdge();
        support = Biddy_And(support,tmp);
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
      }
      last = Biddy_GetTopVariable(tmp);
    }

    /* GRAPHVIZ/DOT OUTPUT OF support - DEBUGGING, ONLY */
    /*
    if (SIZE < 10) {
      Biddy_WriteDot("support.dot",support,"support",-1,FALSE);
      printf("USE 'dot -y -Tpng -O support.dot' to visualize function support.\n");
    }
    */

    /* GENERATE RANDOM FUNCTION / COMBINATION SET */

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
      printf("USE 'dot -y -Tpng -O result.dot' to visualize function support.\n");
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

    /* OPERATION SUBSET */

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

    /* ************************************************************* */
    /* CONVERSIONS */
    /* ************************************************************* */

    printf("********* CONVERSION TESTS *********\n");

    /* CONVERT THE RESULT INTO OBDD */
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

    /* CONVERT THE RESULT INTO OBDDC */
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

    /* CONVERT THE RESULT INTO ZBDDC */
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

    /* CONVERT THE RESULT INTO ZBDDC */
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

    /* CONVERT THE RESULT INTO TZBDDs */
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
      cube = Biddy_And(cube, Biddy_GetVariableEdge(first));
      cube = Biddy_And(cube, Biddy_GetVariableEdge(last));
      cube = Biddy_And(cube, Biddy_GetVariableEdge(extra));
    }

    if ((Biddy_GetManagerType() == BIDDYTYPEZBDD) || (Biddy_GetManagerType() == BIDDYTYPEZBDDC)) {
      cube = Biddy_GetBaseSet();
      cube = Biddy_Change(cube, first);
      if (last != first) cube = Biddy_Change(cube, last);
      cube = Biddy_Change(cube, extra);
    }

    if ((Biddy_GetManagerType() == BIDDYTYPETZBDD) || (Biddy_GetManagerType() == BIDDYTYPETZBDDC)) {
      cube = Biddy_GetConstantOne();
      cube = Biddy_And(cube, Biddy_GetVariableEdge(first));
      cube = Biddy_And(cube, Biddy_GetVariableEdge(last));
      cube = Biddy_And(cube, Biddy_GetVariableEdge(extra));
    }

    result1 = Biddy_E(result, first);
    result1 = Biddy_E(result1, last);
    result2 = Biddy_ExistAbstract(result, cube);

    printf("After the existential abstraction of first and last variable, function result has %.0f minterms/combinations\n", Biddy_CountMinterms(result2, SIZE));

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

    result1 = Biddy_A(result, first);
    result1 = Biddy_A(result1, last);
    result2 = Biddy_UnivAbstract(result, cube);

    printf("After the universal abstraction of first and last variable, function result has %.0f minterms/combinations\n", Biddy_CountMinterms(result2, SIZE));

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

    result0 = Biddy_E(result, first);
    result1 = Biddy_E(Biddy_Not(result), last);
    result2 = Biddy_And(result0, result1);
    result2 = Biddy_ExistAbstract(result2, cube);
    result1 = Biddy_AndAbstract(result0, result1, cube);

    printf("After the first user-defined abstraction, function result has %.0f minterms/combinations\n", Biddy_CountMinterms(result1, SIZE));

    if (result1 != result2) {
      printf("ERROR: Operation Abstraction (3) is wrong!\n");
    }

    result0 = Biddy_A(result, first);
    result1 = Biddy_A(Biddy_Not(result), last);
    result1 = Biddy_AndAbstract(result0, result1, cube);

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

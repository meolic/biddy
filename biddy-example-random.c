/* $Revision: 353 $ */
/* $Date: 2017-12-07 13:25:28 +0100 (čet, 07 dec 2017) $ */
/* This file (biddy-example-random.c) is a C file */
/* Author: Robert Meolic (robert.meolic@um.si) */
/* This file has been released into the public domain by the author. */

/* This example is compatible with Biddy v1.7 */

/* COMPILE WITH: */
/* gcc -DUNIX -O2 -o biddy-example-random biddy-example-random.c -I. -L./bin -static -lbiddy -lgmp */

/* this example creates and converts random BDDs */

#include "biddy.h"

#define BOOLEANFUNCTION 1
#define COMBINATIONSET 2

/* (POPULATION == -2): create one random function, only */
/* (POPULATION == -1): create one random function and perform tests */
/* (POPULATION == 0): not allowed */
/* (POPULATION > 0): create many random function and get statistics */
#define POPULATION -2

#define SIZE 4
#define RATIO 0.5

/* create Boolean function (BOOLEANFUNCTION) or combination set (COMBINATIONSET) */
#define RANDOMTYPE BOOLEANFUNCTION

/* choose primary BDD type */
#define BDDTYPE BIDDYTYPEOBDDC

int main() {
  Biddy_Manager MNGOBDD,MNGZBDD,MNGTZBDD;
  Biddy_Edge tmp,support,cube,result,result0,result1,result2;
  Biddy_Edge robdd,rzbdd,rtzbdd;
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

    Biddy_InitMNG(&MNGOBDD,BIDDYTYPEOBDDC);
    Biddy_InitMNG(&MNGZBDD,BIDDYTYPEZBDDC);
    Biddy_InitMNG(&MNGTZBDD,BIDDYTYPETZBDD);

    printf("Using %s...\n",Biddy_GetManagerName());
   
    /* for OBDDs, support is the most easiest calculated using AddVariable and AND */
    if ((Biddy_GetManagerType() == BIDDYTYPEOBDD) || (Biddy_GetManagerType() == BIDDYTYPEOBDDC)) {
      tmp = Biddy_AddVariable();
      support = tmp;
      first = Biddy_GetTopVariable(tmp);
      for (v=1; v<SIZE; v++) {
        tmp = Biddy_AddVariable();
        support = Biddy_And(support,tmp);
      }
      last = Biddy_GetTopVariable(tmp);
      tmp = Biddy_AddVariable();
      extra = Biddy_GetTopVariable(tmp);
    }

/* for ZBDDs, support is the most easiest calculated using AddElement and CHANGE */
    if ((Biddy_GetManagerType() == BIDDYTYPEZBDD) || (Biddy_GetManagerType() == BIDDYTYPEZBDDC)) {
      tmp = Biddy_AddElement();
      support = tmp;
      first = Biddy_GetTopVariable(tmp);
      for (v=1; v<SIZE; v++) {
        tmp = Biddy_AddElement();
        support = Biddy_Change(support,Biddy_GetTopVariable(tmp));
      }
      last = Biddy_GetTopVariable(tmp);
      tmp = Biddy_AddElement();
      extra = Biddy_GetTopVariable(tmp);
    }

/* for TZBDDs, the approaches from OBDDs and ZBDDs are both working, using the first one */
    if ((Biddy_GetManagerType() == BIDDYTYPETZBDD) || (Biddy_GetManagerType() == BIDDYTYPETZBDDC)) {
      tmp = Biddy_AddVariable();
      support = tmp;
      first = Biddy_GetTopVariable(tmp);
      for (v=1; v<SIZE; v++) {
        tmp = Biddy_AddVariable();
        support = Biddy_And(support,tmp);
      }
      last = Biddy_GetTopVariable(tmp);
      tmp = Biddy_AddVariable();
      extra = Biddy_GetTopVariable(tmp);
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

    /* TESTING */
    /*
    Biddy_Eval0("result B(0)(i(d(0)(y(0)(1)))(d(y(0)(1))(1)))");
    Biddy_FindFormula("result",&i,&result);
    Biddy_WriteDot("result.dot",result,"result",-1,FALSE);
    printf("USE 'dot -y -Tpng -O result.dot' to visualize function result.\n");
    Biddy_WriteBddview("result.bddview",result,"R",NULL);
    exit(1);
    */

    /* DEBUGGING */
    /*
    Biddy_Eval0("result 2(3(4(1)(*1))(4(*1)(1)))(3(4(*1)(1))(4(1)(*1)))");
    Biddy_FindFormula("result",&i,&result);
    */

    /* TRUTH TABLE AND GRAPH FOR THE RESULT  - DEBUGGING, ONLY */
    /*
    if (SIZE < 10) {
      printf("HERE IS A TRUTH TABLE FOR result:\n");
      Biddy_PrintfTable(result);
    }
    Biddy_PrintfBDD(result);
    */

    printf("Function result has %.0f minterms/combinations.\n",Biddy_CountMinterms(result,SIZE));
    printf("Function result has density = %.2e.\n",Biddy_DensityOfFunction(result,SIZE));

    if (POPULATION == -2) exit(1);

    /* ************************************************************* */
    /* START OF TESTING OPERATIONS */
    /* ************************************************************* */

    /* OPERATION NOT */

    result0 = Biddy_Not(Biddy_Not(result));

    if (result0 != result) {
      printf("ERROR: Operation NOT is wrong!\n");
    }

    /* OPERATIONS AND/OR */

    result0 = Biddy_And(result,support);
    result1 = Biddy_Not(Biddy_Or(Biddy_Not(result),Biddy_Not(support)));

    if (result0 != result1) {
      printf("ERROR: Operations AND/OR are wrong!\n");
    }

    /* OPERATIONS ITE/XOR */

    result0 = Biddy_ITE(result,Biddy_Not(support),support);
    result1 = Biddy_Xor(result,support);

    if (result0 != result1) {
      printf("ERROR: Operations ITE and/or XOR are wrong!\n");
    }

    /* OPERATIONS LEQ AND GT */

    result0 = Biddy_Leq(result,support);
    result1 = Biddy_Or(Biddy_Not(result),support);

    if (result0 != result1) {
      printf("ERROR: Operation Leq is wrong!\n");
    }

    result0 = Biddy_Leq(support,result);
    result1 = Biddy_Or(Biddy_Not(support),result);

    if (result0 != result1) {
      printf("ERROR: Operation Leq is wrong!\n");
    }

    result0 = Biddy_Gt(result,support);
    result1 = Biddy_And(result,Biddy_Not(support));

    if (result0 != result1) {
      printf("ERROR: Operation Gt is wrong!\n");
    }

    result0 = Biddy_Gt(support,result);
    result1 = Biddy_And(support,Biddy_Not(result));

    if (result0 != result1) {
      printf("ERROR: Operation Gt is wrong!\n");
    }

    result0 = Biddy_Leq(result,support);
    result1 = Biddy_Not(Biddy_Gt(result,support));

    if (result0 != result1) {
      printf("ERROR: Operations Leq and/or Gt are wrong!\n");
    }

    result0 = Biddy_Leq(support,result);
    result1 = Biddy_Not(Biddy_Gt(support,result));

    if (result0 != result1) {
      printf("ERROR: Operations Leq and/or Gt are wrong!\n");
    }

    /* OPERATION SUBSET */

    result0 = Biddy_Subset0(result,first);
    result1 = Biddy_Subset1(result,first);

    /* GRAPHVIZ/DOT OUTPUT OF RESULT0 AND RESULT1  - DEBUGGING, ONLY */
    /*
    if (SIZE < 10) {
      Biddy_WriteDot("result0.dot",result0,"result0",-1,FALSE);
      printf("USE 'dot -y -Tpng -O result0.dot' to visualize function result0.\n");
      Biddy_WriteDot("result1.dot",result1,"result1",-1,FALSE);
      printf("USE 'dot -y -Tpng -O result1.dot' to visualize function result1.\n");
    }
    */

    printf("Function result has %.0f minterms/combinations where first variable/element is NEGATIVE/ABSENT.\n",Biddy_CountMinterms(result0,SIZE));
    printf("Function result has %.0f minterms/combinations where first variable/element is POSITIVE/PRESENT.\n",Biddy_CountMinterms(result1,SIZE));

    s1 = (unsigned int) Biddy_CountMinterms(result,SIZE);
    s2 = (unsigned int) Biddy_CountMinterms(result0,SIZE);
    s3 = (unsigned int) Biddy_CountMinterms(result1,SIZE);

    if (s1 != (s2 + s3)) {
      printf("ERROR: Operation Subset is wrong!\n");
    }

    result0 = Biddy_Subset0(result,last);
    result1 = Biddy_Subset1(result,last);

    /* GRAPHVIZ/DOT OUTPUT OF RESULT0 AND RESULT1  - DEBUGGING, ONLY */
    /*
    if (SIZE < 10) {
      Biddy_WriteDot("result0.dot",result0,"result0",-1,FALSE);
      printf("USE 'dot -y -Tpng -O result0.dot' to visualize function result0.\n");
      Biddy_WriteDot("result1.dot",result1,"result1",-1,FALSE);
      printf("USE 'dot -y -Tpng -O result1.dot' to visualize function result1.\n");
    }
    */

    printf("Function result has %.0f minterms/combinations where last variable/element is NEGATIVE/ABSENT.\n",Biddy_CountMinterms(result0,SIZE));
    printf("Function result has %.0f minterms/combinations where last variable/element is POSITIVE/PRESENT.\n",Biddy_CountMinterms(result1,SIZE));

    s1 = (unsigned int) Biddy_CountMinterms(result,SIZE);
    s2 = (unsigned int) Biddy_CountMinterms(result0,SIZE);
    s3 = (unsigned int) Biddy_CountMinterms(result1,SIZE);

    if (s1 != (s2 + s3)) {
      printf("ERROR: Operation Subset is wrong!\n");
    }

    /* OPERATION RESTRICT */

    result0 = Biddy_Restrict(result,first,FALSE);
    result1 = Biddy_Restrict(result,first,TRUE);

    printf("If first variable is restricted to FALSE, function result has %.0f minterms/combinations\n",Biddy_CountMinterms(result0,SIZE));
    printf("If first variable is restricted to TRUE, function result has %.0f minterms/combinations\n",Biddy_CountMinterms(result1,SIZE));

    /* GRAPHVIZ/DOT OUTPUT OF RESULT0 AND RESULT1  - DEBUGGING, ONLY */
    /*
    if (SIZE < 10) {
      Biddy_WriteDot("result0.dot",result0,"result0",-1,FALSE);
      printf("USE 'dot -y -Tpng -O result0.dot' to visualize function result0.\n");
      Biddy_WriteDot("result1.dot",result1,"result1",-1,FALSE);
      printf("USE 'dot -y -Tpng -O result1.dot' to visualize function result1.\n");
    }
    */

    s1 = (unsigned int) Biddy_CountMinterms(result,SIZE);
    s2 = (unsigned int) Biddy_CountMinterms(result0,SIZE);
    s3 = (unsigned int) Biddy_CountMinterms(result1,SIZE);

    if ((s1 + s1) != (s2 + s3)) {
      printf("ERROR: Operation Restrict is wrong!\n");
    }

    result0 = Biddy_Restrict(result,last,FALSE);
    result1 = Biddy_Restrict(result,last,TRUE);

    /* GRAPHVIZ/DOT OUTPUT OF RESULT0 AND RESULT1  - DEBUGGING, ONLY */
    /*
    if (SIZE < 10) {
      Biddy_WriteDot("result0.dot",result0,"result0",-1,FALSE);
      printf("USE 'dot -y -Tpng -O result0.dot' to visualize function result0.\n");
      Biddy_WriteDot("result1.dot",result1,"result1",-1,FALSE);
      printf("USE 'dot -y -Tpng -O result1.dot' to visualize function result1.\n");
    }
    */

    printf("If last variable is restricted to FALSE, function result has %.0f minterms/combinations\n",Biddy_CountMinterms(result0,SIZE));
    printf("If last variable is restricted to TRUE, function result has %.0f minterms/combinations\n",Biddy_CountMinterms(result1,SIZE));

    s1 = (unsigned int) Biddy_CountMinterms(result,SIZE);
    s2 = (unsigned int) Biddy_CountMinterms(result0,SIZE);
    s3 = (unsigned int) Biddy_CountMinterms(result1,SIZE);

    if ((s1 + s1) != (s2 + s3)) {
      printf("ERROR: Operation Restrict is wrong!\n");
    }

    /* OPERATION REPLACE */

    Biddy_ResetVariablesValue();
    Biddy_SetVariableValue(first,Biddy_GetVariableEdge(extra));

    result0 = Biddy_Replace(result);

    Biddy_ResetVariablesValue();
    Biddy_SetVariableValue(extra,Biddy_GetVariableEdge(first));

    result1 = Biddy_Replace(result0);

    /* GRAPHVIZ/DOT OUTPUT OF RESULT0 AND RESULT1  - DEBUGGING, ONLY */
    /*
    if (SIZE < 10) {
      Biddy_WriteDot("result0.dot",result0,"result0",-1,FALSE);
      printf("USE 'dot -y -Tpng -O result0.dot' to visualize function result0.\n");
      Biddy_WriteDot("result1.dot",result1,"result1",-1,FALSE);
      printf("USE 'dot -y -Tpng -O result1.dot' to visualize function result1.\n");
    }
    */

    if (result1 != result) {
      printf("ERROR: Operation Replace is wrong!\n");
    }

    /* OPERATION QUANTIFICATION */

    result0 = Biddy_E(result,first);
    result1 = Biddy_A(result,first);

    printf("After the existential quantification of first variable, function result has %.0f minterms/combinations\n",Biddy_CountMinterms(result0,SIZE));
    printf("After the universal quantification of first variable, function result has %.0f minterms/combinations\n",Biddy_CountMinterms(result1,SIZE));

    /* GRAPHVIZ/DOT OUTPUT OF RESULT0 AND RESULT1  - DEBUGGING, ONLY */
    /*
    if (SIZE < 10) {
      Biddy_WriteDot("result0.dot",result0,"result0",-1,FALSE);
      printf("USE 'dot -y -Tpng -O result0.dot' to visualize function result0.\n");
      Biddy_WriteDot("result1.dot",result1,"result1",-1,FALSE);
      printf("USE 'dot -y -Tpng -O result1.dot' to visualize function result1.\n");
    }
    */

    s1 = (unsigned int) Biddy_CountMinterms(result,SIZE);
    s2 = (unsigned int) Biddy_CountMinterms(result0,SIZE);
    s3 = (unsigned int) Biddy_CountMinterms(result1,SIZE);

    if ((s1 + s1) != (s2 + s3)) {
      printf("ERROR: Operation Quantification is wrong!\n");
    }

    result0 = Biddy_E(result,last);
    result1 = Biddy_A(result,last);

    /* GRAPHVIZ/DOT OUTPUT OF RESULT0 AND RESULT1  - DEBUGGING, ONLY */
    /*
    if (SIZE < 10) {
      Biddy_WriteDot("result0.dot",result0,"result0",-1,FALSE);
      printf("USE 'dot -y -Tpng -O result0.dot' to visualize function result0.\n");
      Biddy_WriteDot("result1.dot",result1,"result1",-1,FALSE);
      printf("USE 'dot -y -Tpng -O result1.dot' to visualize function result1.\n");
    }
    */

    printf("After the existential quantification of last variable, function result has %.0f minterms/combinations\n",Biddy_CountMinterms(result0,SIZE));
    printf("After the universal quantification of last variable, function result has %.0f minterms/combinations\n",Biddy_CountMinterms(result1,SIZE));

    s1 = (unsigned int) Biddy_CountMinterms(result,SIZE);
    s2 = (unsigned int) Biddy_CountMinterms(result0,SIZE);
    s3 = (unsigned int) Biddy_CountMinterms(result1,SIZE);

    if ((s1 + s1) != (s2 + s3)) {
      printf("ERROR: Operation Quantification is wrong!\n");
    }

    /* OPERATION ABSTRACTION */

    /* calculate cube */

    if ((Biddy_GetManagerType() == BIDDYTYPEOBDD) || (Biddy_GetManagerType() == BIDDYTYPEOBDDC)) {
      cube = Biddy_GetConstantOne();
      cube = Biddy_And(cube,Biddy_GetVariableEdge(first));
      cube = Biddy_And(cube,Biddy_GetVariableEdge(last));
      cube = Biddy_And(cube,Biddy_GetVariableEdge(extra));
    }

    if ((Biddy_GetManagerType() == BIDDYTYPEZBDD) || (Biddy_GetManagerType() == BIDDYTYPEZBDDC)) {
      cube = Biddy_GetBaseSet();
      cube = Biddy_Change(cube,first);
      cube = Biddy_Change(cube,last);
      cube = Biddy_Change(cube,extra);
     }

    if ((Biddy_GetManagerType() == BIDDYTYPETZBDD) || (Biddy_GetManagerType() == BIDDYTYPETZBDDC)) {
      cube = Biddy_GetConstantOne();
      cube = Biddy_And(cube,Biddy_GetVariableEdge(first));
      cube = Biddy_And(cube,Biddy_GetVariableEdge(last));
      cube = Biddy_And(cube,Biddy_GetVariableEdge(extra));
    }

    result1 = Biddy_E(result,first);
    result1 = Biddy_E(result1,last);
    result2 = Biddy_ExistAbstract(result,cube);

    printf("After the existential abstraction of first and last variable, function result has %.0f minterms/combinations\n",Biddy_CountMinterms(result2,SIZE));

    /* GRAPHVIZ/DOT OUTPUT OF RESULT1 - DEBUGGING, ONLY */
    /*
    if (SIZE < 10) {
      Biddy_WriteDot("result1.dot",result1,"result1",-1,FALSE);
      printf("USE 'dot -y -Tpng -O result1.dot' to visualize function result1.\n");
    }
    */

    if (result1 != result2) {
      printf("ERROR: Operation Abstraction is wrong!\n");
    }

    result1 = Biddy_A(result,first);
    result1 = Biddy_A(result1,last);
    result2 = Biddy_UnivAbstract(result,cube);

    printf("After the universal abstraction of first and last variable, function result has %.0f minterms/combinations\n",Biddy_CountMinterms(result2,SIZE));

    /* GRAPHVIZ/DOT OUTPUT OF RESULT1  - DEBUGGING, ONLY */
    /*
    if (SIZE < 10) {
      Biddy_WriteDot("result1.dot",result1,"result1",-1,FALSE);
      printf("USE 'dot -y -Tpng -O result1.dot' to visualize function result1.\n");
    }
    */

    if (result1 != result2) {
      printf("ERROR: Operation Abstraction is wrong!\n");
    }

    result0 = Biddy_E(result,first);
    result1 = Biddy_E(Biddy_Not(result),last);
    result2 = Biddy_And(result0,result1);
    result2 = Biddy_ExistAbstract(result2,cube);
    result1 = Biddy_AndAbstract(result0,result1,cube);

    printf("After the first user-defined abstraction, function result has %.0f minterms/combinations\n",Biddy_CountMinterms(result1,SIZE));

    if (result1 != result2) {
      printf("ERROR: Operation Abstraction is wrong!\n");
    }

    result0 = Biddy_A(result,first);
    result1 = Biddy_A(Biddy_Not(result),last);
    result1 = Biddy_AndAbstract(result0,result1,cube);

    printf("After the second user-defined abstraction, function result has %.0f minterms/combinations\n",Biddy_CountMinterms(result1,SIZE));

    if (result1 != Biddy_GetConstantZero()) {
      printf("ERROR: Operation Abstraction is wrong!\n");
    }

    /* BOOLEAN OPERATIONS AND OPERATION ITE */

    result1 = Biddy_ITE(Biddy_E(result,first),Biddy_E(result,last),Biddy_A(result,last));
    result2 = Biddy_ITE(Biddy_A(result,first),Biddy_E(result,last),Biddy_A(result,last));

    printf("After the first ITE, function result has %.0f minterms/combinations\n",Biddy_CountMinterms(result1,SIZE));
    printf("After the second ITE, function result has %.0f minterms/combinations\n",Biddy_CountMinterms(result2,SIZE));

    /* GRAPHVIZ/DOT OUTPUT OF RESULT1 AND RESULT2 - DEBUGGING, ONLY */
    /*
    if (SIZE < 10) {
      Biddy_WriteDot("result1.dot",result1,"result1",-1,FALSE);
      printf("USE 'dot -y -Tpng -O result1.dot' to visualize function result1.\n");
      Biddy_WriteDot("result2.dot",result2,"result2",-1,FALSE);
      printf("USE 'dot -y -Tpng -O result2.dot' to visualize function result2.\n");
    }
    */

    if (result1 != Biddy_Xor(
              Biddy_And(Biddy_E(result,first),Biddy_E(result,last)),
              Biddy_And(Biddy_Not(Biddy_E(result,first)),Biddy_A(result,last))))
    {
      printf("ERROR: result1 for Boolean operations ITE-AND-XOR-NOT is wrong!\n");
    }
    if (result1 != Biddy_Or(
              Biddy_And(Biddy_E(result,first),Biddy_E(result,last)),
              Biddy_Gt(Biddy_A(result,last),Biddy_E(result,first))))
    {
      printf("ERROR: result1 for Boolean operations ITE-AND-OR-GT is wrong!\n");
    }

    if (result2 != Biddy_Xor(
              Biddy_And(Biddy_A(result,first),Biddy_E(result,last)),
              Biddy_And(Biddy_Not(Biddy_A(result,first)),Biddy_A(result,last))))
    {
      printf("ERROR: result2 for Boolean operations ITE-AND-XOR-NOT is wrong!\n");
    }
    if (result2 != Biddy_Or(
              Biddy_And(Biddy_A(result,first),Biddy_E(result,last)),
              Biddy_Gt(Biddy_A(result,last),Biddy_A(result,first))))
    {
      printf("ERROR: result2 for Boolean operations ITE-AND-OR-GT is wrong!\n");
    }

    /* ************************************************************* */
    /* START OF CONVERSIONS AND STATISTICS */
    /* ************************************************************* */

    printf("%s for function result has %u nodes (including terminals).\n",Biddy_GetManagerName(),Biddy_CountNodes(result));
    printf("%s for function result has %u plain nodes (including terminals).\n",Biddy_GetManagerName(),Biddy_CountNodesPlain(result));
    printf("%s for function result has %llu one-paths.\n",Biddy_GetManagerName(),Biddy_CountPaths(result));
    printf("Function represented by %s depends on %u variables.\n",Biddy_GetManagerName(),Biddy_DependentVariableNumber(result));
    printf("Considering the given set of variables, %s for function result has %.0f minterms/combinations.\n",Biddy_GetManagerName(),Biddy_CountMinterms(result,SIZE));
    /* GRAPHVIZ/DOT OUTPUT OF THE RESULT */
    if (SIZE < 10) {
      Biddy_WriteDot("result.dot",result,"result",-1,FALSE);
      printf("USE 'dot -y -Tpng -O result.dot' to visualize %s for function result.\n",Biddy_GetManagerName());
    }

    /* CONVERT THE RESULT INTO OBDDC */
    robdd = Biddy_Copy(MNGOBDD,result);
    printf("OBDDC for function result has %u nodes (including terminal).\n",Biddy_Managed_CountNodes(MNGOBDD,robdd));
    printf("OBDDC for function result has %u plain nodes (including both terminals).\n",Biddy_Managed_CountNodesPlain(MNGOBDD,robdd));
    printf("OBDDC for function result has %llu one-paths.\n",Biddy_Managed_CountPaths(MNGOBDD,robdd));
    printf("Function represented by OBDDC depends on %u variables.\n",Biddy_Managed_DependentVariableNumber(MNGOBDD,robdd));
    printf("Considering the given set of variables, OBDDC for function result has %.0f minterms/combinations.\n",Biddy_Managed_CountMinterms(MNGOBDD,robdd,SIZE));
    /* GRAPHVIZ/DOT OUTPUT OF THE RESULT */
    if (SIZE < 10) {
      Biddy_Managed_WriteDot(MNGOBDD,"obdd.dot",robdd,"obdd",-1,FALSE);
      printf("USE 'dot -y -Tpng -O obdd.dot' to visualize %s for function result.\n",Biddy_Managed_GetManagerName(MNGOBDD));
    }

    /* CONVERT THE RESULT INTO ZBDDC */
    if (Biddy_GetManagerType() != BIDDYTYPETZBDDC) {
      rzbdd = Biddy_Copy(MNGZBDD,result);
      printf("ZBDDC for function result has %u nodes (including terminal).\n",Biddy_Managed_CountNodes(MNGZBDD,rzbdd));
      printf("ZBDDC for function result has %u plain nodes (including both terminals).\n",Biddy_Managed_CountNodesPlain(MNGZBDD,rzbdd));
      printf("ZBDDC for function result has %llu one-paths.\n",Biddy_Managed_CountPaths(MNGZBDD,rzbdd));
      printf("Function represented by ZBDDC depends on %u variables.\n",Biddy_Managed_DependentVariableNumber(MNGZBDD,rzbdd));
      printf("Considering the given set of variables, ZBDDC for function result has %.0f minterms/combinations.\n",Biddy_Managed_CountMinterms(MNGZBDD,rzbdd,SIZE));
      /* GRAPHVIZ/DOT OUTPUT OF THE RESULT */
      if (SIZE < 10) {
        Biddy_Managed_WriteDot(MNGZBDD,"zbdd.dot",rzbdd,"zbdd",-1,FALSE);
        printf("USE 'dot -y -Tpng -O zbdd.dot' to visualize %s for function result.\n",Biddy_Managed_GetManagerName(MNGZBDD));
      }
    }

    if (Biddy_GetManagerType() != BIDDYTYPEZBDDC) {
      /* CONVERT THE RESULT INTO TZBDDs */
      rtzbdd = Biddy_Copy(MNGTZBDD,result);
      printf("TZBDD for function result has %u nodes (including the terminal).\n",Biddy_Managed_CountNodes(MNGTZBDD,rtzbdd));
      printf("TZBDD for function result has %llu one-paths.\n",Biddy_Managed_CountPaths(MNGTZBDD,rtzbdd));
      printf("Function represented by TZBDD depends on %u variables.\n",Biddy_Managed_DependentVariableNumber(MNGTZBDD,rtzbdd));
      printf("Considering the given set of variables, TZBDD for function result has %.0f minterms/combinations.\n",Biddy_Managed_CountMinterms(MNGTZBDD,rtzbdd,SIZE));
      /* GRAPHVIZ/DOT OUTPUT OF THE RESULT */
      if (SIZE < 10) {
        Biddy_Managed_WriteDot(MNGTZBDD,"tzbdd.dot",rtzbdd,"tzbdd",-1,FALSE);
        printf("USE 'dot -y -Tpng -O tzbdd.dot' to visualize %s for function result.\n",Biddy_Managed_GetManagerName(MNGTZBDD));
      }
    }

    Biddy_Exit();
    Biddy_ExitMNG(&MNGOBDD);
    Biddy_ExitMNG(&MNGZBDD);
    Biddy_ExitMNG(&MNGTZBDD);

  } else {

    obddsize = zbddsize = tzbddsize = 0;
    obddminsize = zbddminsize = tzbddminsize = 0;
    obddmaxsize = zbddmaxsize = tzbddmaxsize = 0;
    obddsmallest = zbddsmallest = tzbddsmallest = 0;
    obddlargest = zbddlargest = tzbddlargest = 0;
    allsamesize = 0;
  
    for (i=0; i<POPULATION; i++) {

      Biddy_InitMNG(&MNGOBDD,BIDDYTYPEOBDDC);
      Biddy_InitMNG(&MNGZBDD,BIDDYTYPEZBDDC);
      Biddy_InitMNG(&MNGTZBDD,BIDDYTYPETZBDD);

      support = Biddy_Managed_GetConstantOne(MNGOBDD);
      for (v=0; v<SIZE; v++) {
        support = Biddy_Managed_And(MNGOBDD,support,Biddy_Managed_AddVariable(MNGOBDD));
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

      /* TESTING: if ZBDD is the smallest */
      /*
      if ((s2 < s1) && (s2 < s3)) {
        printf("OBDD has %u nodes without complemented edges (%u with complemented edges).\n",
               Biddy_Managed_CountNodesPlain(MNGOBDD,robdd), Biddy_Managed_CountNodes(MNGOBDD,robdd));
        printf("ZBDD has %u nodes without complemented edges (%u with complemented edges).\n",
               Biddy_Managed_CountNodesPlain(MNGZBDD,rzbdd), Biddy_Managed_CountNodes(MNGZBDD,rzbdd));
        printf("TZBDD has %u nodes without complemented edges.\n",
               Biddy_Managed_CountNodes(MNGTZBDD,rtzbdd));
        Biddy_Managed_PrintfTable(MNGOBDD,robdd);
        Biddy_Managed_WriteDot(MNGOBDD,"obdd.dot",robdd,"obdd",-1,FALSE);
        printf("USE 'dot -y -Tpng -O obdd.dot' to visualize OBDDC for function result.\n");
        Biddy_Managed_WriteDot(MNGZBDD,"zbdd.dot",rzbdd,"zbdd",-1,FALSE);
        printf("USE 'dot -y -Tpng -O zbdd.dot' to visualize ZBDDC for function result.\n");
        Biddy_Managed_WriteDot(MNGTZBDD,"tzbdd.dot",rtzbdd,"tzbdd",-1,FALSE);
        printf("USE 'dot -y -Tpng -O tzbdd.dot' to visualize TZBDD for function result.\n");
      }
      */

      Biddy_ExitMNG(&MNGOBDD);
      Biddy_ExitMNG(&MNGZBDD);
      Biddy_ExitMNG(&MNGTZBDD);

    }

    printf("RESULTS FOR POPULATION=%u, SIZE=%u, RATIO=%.2e\n",POPULATION,SIZE,RATIO);
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

RESULTS FOR POPULATION=10000, SIZE=8, RATIO=0.01
the average size of OBDDs = 16.83
the average size of ZBDDs = 10.33
the average size of TZBDDs = 10.31
OBDD is the smallest in 0 examples (0.00%)
ZBDD is the smallest in 188 examples (1.88%)
TZBDD is the smallest in 425 examples (4.25%)
OBDD and ZBDD are the same size and are smaller in 0 examples (0.00%)
OBDD and TZBDD are the same size and are smaller in 0 examples (0.00%)
ZBDD and TZBDD are the same size and are smaller in 9387 examples (93.87%)
all three graphs have the same size in 0 examples (0.00%)

RESULTS FOR POPULATION=10000, SIZE=8, RATIO=0.10
the average size of OBDDs = 44.03
the average size of ZBDDs = 33.22
the average size of TZBDDs = 33.89
OBDD is the smallest in 0 examples (0.00%)
ZBDD is the smallest in 5425 examples (54.25%)
TZBDD is the smallest in 2154 examples (21.54%)
OBDD and ZBDD are the same size and are smaller in 0 examples (0.00%)
OBDD and TZBDD are the same size and are smaller in 0 examples (0.00%)
ZBDD and TZBDD are the same size and are smaller in 2421 examples (24.21%)
all three graphs have the same size in 0 examples (0.00%)

RESULTS FOR POPULATION=10000, SIZE=8, RATIO=0.50
the average size of OBDDs = 63.79
the average size of ZBDDs = 60.68
the average size of TZBDDs = 66.23
OBDD is the smallest in 235 examples (2.35%)
ZBDD is the smallest in 8878 examples (88.78%)
TZBDD is the smallest in 26 examples (0.26%)
OBDD and ZBDD are the same size and are smaller in 835 examples (8.35%)
OBDD and TZBDD are the same size and are smaller in 0 examples (0.00%)
ZBDD and TZBDD are the same size and are smaller in 0 examples (0.00%)
all three graphs have the same size in 26 examples (0.26%)

RESULTS FOR POPULATION=10000, SIZE=8, RATIO=0.90
the average size of OBDDs = 43.79
the average size of ZBDDs = 44.48
the average size of TZBDDs = 44.12
OBDD is the smallest in 1699 examples (16.99%)
ZBDD is the smallest in 2868 examples (28.68%)
TZBDD is the smallest in 0 examples (0.00%)
OBDD and ZBDD are the same size and are smaller in 693 examples (6.93%)
OBDD and TZBDD are the same size and are smaller in 3699 examples (36.99%)
ZBDD and TZBDD are the same size and are smaller in 0 examples (0.00%)
all three graphs have the same size in 1041 examples (10.41%)

RESULTS FOR POPULATION=10000, SIZE=8, RATIO=0.99
the average size of OBDDs = 16.86
the average size of ZBDDs = 21.07
the average size of TZBDDs = 16.59
OBDD is the smallest in 0 examples (0.00%)
ZBDD is the smallest in 0 examples (0.00%)
TZBDD is the smallest in 2759 examples (27.59%)
OBDD and ZBDD are the same size and are smaller in 0 examples (0.00%)
OBDD and TZBDD are the same size and are smaller in 6947 examples (69.47%)
ZBDD and TZBDD are the same size and are smaller in 0 examples (0.00%)
all three graphs have the same size in 294 examples (2.94%)

---

RESULTS FOR POPULATION=10000, SIZE=30, RATIO=1.00e-08
the average size of OBDDs = 262.92 (min = 233, max = 277)
the average size of ZBDDs = 136.48 (min = 113, max = 166)
the average size of TZBDDs = 136.48 (min = 113, max = 166)
OBDD is the smallest in 0 examples (0.00%)
ZBDD is the smallest in 0 examples (0.00%)
TZBDD is the smallest in 0 examples (0.00%)
OBDD and ZBDD are the same size and are smaller in 0 examples (0.00%)
OBDD and TZBDD are the same size and are smaller in 0 examples (0.00%)
ZBDD and TZBDD are the same size and are smaller in 10000 examples (100.00%)
all three graphs have the same size in 0 examples (0.00%)

RESULTS FOR POPULATION=10000, SIZE=30, RATIO=1.00e-04
the average size of OBDDs = 181687.04 (min = 180434, max = 182742)
the average size of ZBDDs = 144380.62 (min = 143671, max = 145055)
the average size of TZBDDs = 144396.46 (min = 143694, max = 145070)
OBDD is the smallest in 0 examples (0.00%)
ZBDD is the smallest in 9681 examples (96.81%)
TZBDD is the smallest in 228 examples (2.28%)
OBDD and ZBDD are the same size and are smaller in 0 examples (0.00%)
OBDD and TZBDD are the same size and are smaller in 0 examples (0.00%)
ZBDD and TZBDD are the same size and are smaller in 91 examples (0.91%)
all three graphs have the same size in 0 examples (0.00%)

RESULTS FOR POPULATION=10000, SIZE=30, RATIO=1.00e+00
the average size of OBDDs = 262.45 (min = 242, max = 277)
the average size of ZBDDs = 285.69 (min = 263, max = 302)
the average size of TZBDDs = 262.44 (min = 242, max = 277)
OBDD is the smallest in 0 examples (0.00%)
ZBDD is the smallest in 0 examples (0.00%)
TZBDD is the smallest in 50 examples (0.50%)
OBDD and ZBDD are the same size and are smaller in 0 examples (0.00%)
OBDD and TZBDD are the same size and are smaller in 9950 examples (99.50%)
ZBDD and TZBDD are the same size and are smaller in 0 examples (0.00%)
all three graphs have the same size in 0 examples (0.00%)

---

RESULTS FOR POPULATION=1000, SIZE=63, RATIO=1.00e-16
the average size of OBDDs = 37237.49 (min = 36928, max = 37498)
the average size of ZBDDs = 19092.70 (min = 18774, max = 19394)
the average size of TZBDDs = 19092.70 (min = 18774, max = 19394)
OBDD is the smallest in 0 examples (0.00%)
ZBDD is the smallest in 0 examples (0.00%)
TZBDD is the smallest in 0 examples (0.00%)
OBDD and ZBDD are the same size and are smaller in 0 examples (0.00%)
OBDD and TZBDD are the same size and are smaller in 0 examples (0.00%)
ZBDD and TZBDD are the same size and are smaller in 1000 examples (100.00%)
all three graphs have the same size in 0 examples (0.00%)

RESULTS FOR POPULATION=1000, SIZE=63, RATIO=1.00e+00
the average size of OBDDs = 41039.00 (min = 40743, max = 41381)
the average size of ZBDDs = 41088.37 (min = 40793, max = 41426)
the average size of TZBDDs = 41039.00 (min = 40743, max = 41381)
OBDD is the smallest in 0 examples (0.00%)
ZBDD is the smallest in 0 examples (0.00%)
TZBDD is the smallest in 0 examples (0.00%)
OBDD and ZBDD are the same size and are smaller in 0 examples (0.00%)
OBDD and TZBDD are the same size and are smaller in 1000 examples (100.00%)
ZBDD and TZBDD are the same size and are smaller in 0 examples (0.00%)
all three graphs have the same size in 0 examples (0.00%)

*/

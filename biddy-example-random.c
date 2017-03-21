/* $Revision: 244 $ */
/* $Date: 2017-02-14 23:23:32 +0100 (tor, 14 feb 2017) $ */
/* This file (biddy-example-random.c) is a C file */
/* Author: Robert Meolic (robert.meolic@um.si) */
/* This file has been released into the public domain by the author. */

/* This example is compatible with Biddy v1.7 */

/* COMPILE WITH: */
/* gcc -DUNIX -O2 -o biddy-example-random biddy-example-random.c -I. -L./bin -lbiddy -lgmp */

/* this example creates and converts random BDDs */

#include "biddy.h"

#define BOOLEANFUNCTION 1
#define COMBINATIONSET 2

#define POPULATION -1
#define SIZE 4
#define RATIO 0.5

/* create Boolean function (BOOLEANFUNCTION) or combination set (COMBINATIONSET) */
#define RANDOMTYPE BOOLEANFUNCTION
#define BDDTYPE BIDDYTYPEOBDD

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

  if (POPULATION == -1) {

    Biddy_InitAnonymous(BDDTYPE);

    Biddy_InitMNG(&MNGOBDD,BIDDYTYPEOBDD);
    Biddy_InitMNG(&MNGZBDD,BIDDYTYPEZBDD);
    Biddy_InitMNG(&MNGTZBDD,BIDDYTYPETZBDD);

/* for OBDDs, support is the most easiest calculated using AddVariable and AND */
#if (BDDTYPE == BIDDYTYPEOBDD)
    printf("Using OBDDs...\n");
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
#endif

/* for ZBDDs, support is the most easiest calculated using AddElement and CHANGE */
#if (BDDTYPE == BIDDYTYPEZBDD)
    printf("Using ZBDDs...\n");
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
#endif

/* for TZBDDs, the approaches from OBDDs and ZBDDs are both working, using the first one */
#if (BDDTYPE == BIDDYTYPETZBDD)
    printf("Using TZBDDs...\n");
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
#endif

    /* GRAPHVIZ/DOT OUTPUT OF support - DEBUGGING, ONLY */
    /*
    if (SIZE < 10) {
      Biddy_WriteDot("support.dot",support,"support",-1,FALSE);
      printf("USE 'dot -y -Tpng -O support.dot' to visualize function support.\n");
    }
    */

    result = NULL;

#if (RANDOMTYPE == BOOLEANFUNCTION)
    printf("Generating random Boolean function...\n");
    result = Biddy_Random(support,RATIO);
#endif

#if (RANDOMTYPE == COMBINATIONSET)
    printf("Generating random combination set...\n");
    result = Biddy_RandomSet(support,RATIO);
#endif

    if (result == NULL) {
      printf("ERROR: Function result is NULL!\n");
      exit(1);
    }

    /* GRAPHVIZ/DOT OUTPUT OF THE RESULT  - DEBUGGING, ONLY */
    /*
    if (SIZE < 10) {
      printf("HERE IS A TRUTH TABLE FOR result:\n");
      Biddy_PrintfTable(result);
      Biddy_WriteDot("result.dot",result,"result",-1,FALSE);
      printf("USE 'dot -y -Tpng -O result.dot' to visualize function result.\n");
    }
    */

    printf("Function result has %.0f minterms/combinations.\n",Biddy_CountMinterm(result,SIZE));
    printf("Function result has density = %.2e.\n",Biddy_DensityFunction(result,SIZE));

    /* ***  BEGINNING OF TESTING OPERATIONS *** */

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

    printf("Function result has %.0f minterms/combinations where first variable/element is NEGATIVE/ABSENT.\n",Biddy_CountMinterm(result0,SIZE));
    printf("Function result has %.0f minterms/combinations where first variable/element is POSITIVE/PRESENT.\n",Biddy_CountMinterm(result1,SIZE));

    s1 = (unsigned int) Biddy_CountMinterm(result,SIZE);
    s2 = (unsigned int) Biddy_CountMinterm(result0,SIZE);
    s3 = (unsigned int) Biddy_CountMinterm(result1,SIZE);

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

    printf("Function result has %.0f minterms/combinations where last variable/element is NEGATIVE/ABSENT.\n",Biddy_CountMinterm(result0,SIZE));
    printf("Function result has %.0f minterms/combinations where last variable/element is POSITIVE/PRESENT.\n",Biddy_CountMinterm(result1,SIZE));

    s1 = (unsigned int) Biddy_CountMinterm(result,SIZE);
    s2 = (unsigned int) Biddy_CountMinterm(result0,SIZE);
    s3 = (unsigned int) Biddy_CountMinterm(result1,SIZE);

    if (s1 != (s2 + s3)) {
      printf("ERROR: Operation Subset is wrong!\n");
    }

    /* OPERATION RESTRICT */

    result0 = Biddy_Restrict(result,first,FALSE);
    result1 = Biddy_Restrict(result,first,TRUE);

    printf("If first variable is restricted to FALSE, function result has %.0f minterms/combinations\n",Biddy_CountMinterm(result0,SIZE));
    printf("If first variable is restricted to TRUE, function result has %.0f minterms/combinations\n",Biddy_CountMinterm(result1,SIZE));

    /* GRAPHVIZ/DOT OUTPUT OF RESULT0 AND RESULT1  - DEBUGGING, ONLY */
    /*
    if (SIZE < 10) {
      Biddy_WriteDot("result0.dot",result0,"result0",-1,FALSE);
      printf("USE 'dot -y -Tpng -O result0.dot' to visualize function result0.\n");
      Biddy_WriteDot("result1.dot",result1,"result1",-1,FALSE);
      printf("USE 'dot -y -Tpng -O result1.dot' to visualize function result1.\n");
    }
    */

    s1 = (unsigned int) Biddy_CountMinterm(result,SIZE);
    s2 = (unsigned int) Biddy_CountMinterm(result0,SIZE);
    s3 = (unsigned int) Biddy_CountMinterm(result1,SIZE);

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

    printf("If last variable is restricted to FALSE, function result has %.0f minterms/combinations\n",Biddy_CountMinterm(result0,SIZE));
    printf("If last variable is restricted to TRUE, function result has %.0f minterms/combinations\n",Biddy_CountMinterm(result1,SIZE));

    s1 = (unsigned int) Biddy_CountMinterm(result,SIZE);
    s2 = (unsigned int) Biddy_CountMinterm(result0,SIZE);
    s3 = (unsigned int) Biddy_CountMinterm(result1,SIZE);

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

    printf("After the existential quantification of first variable, function result has %.0f minterms/combinations\n",Biddy_CountMinterm(result0,SIZE));
    printf("After the universal quantification of first variable, function result has %.0f minterms/combinations\n",Biddy_CountMinterm(result1,SIZE));

    /* GRAPHVIZ/DOT OUTPUT OF RESULT0 AND RESULT1  - DEBUGGING, ONLY */
    /*
    if (SIZE < 10) {
      Biddy_WriteDot("result0.dot",result0,"result0",-1,FALSE);
      printf("USE 'dot -y -Tpng -O result0.dot' to visualize function result0.\n");
      Biddy_WriteDot("result1.dot",result1,"result1",-1,FALSE);
      printf("USE 'dot -y -Tpng -O result1.dot' to visualize function result1.\n");
    }
    */

    s1 = (unsigned int) Biddy_CountMinterm(result,SIZE);
    s2 = (unsigned int) Biddy_CountMinterm(result0,SIZE);
    s3 = (unsigned int) Biddy_CountMinterm(result1,SIZE);

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

    printf("After the existential quantification of last variable, function result has %.0f minterms/combinations\n",Biddy_CountMinterm(result0,SIZE));
    printf("After the universal quantification of last variable, function result has %.0f minterms/combinations\n",Biddy_CountMinterm(result1,SIZE));

    s1 = (unsigned int) Biddy_CountMinterm(result,SIZE);
    s2 = (unsigned int) Biddy_CountMinterm(result0,SIZE);
    s3 = (unsigned int) Biddy_CountMinterm(result1,SIZE);

    if ((s1 + s1) != (s2 + s3)) {
      printf("ERROR: Operation Quantification is wrong!\n");
    }

    /* OPERATION ABSTRACTION */

    /* calculate cube */

#if (BDDTYPE == BIDDYTYPEOBDD)
    cube = Biddy_GetConstantOne();
    cube = Biddy_And(cube,Biddy_GetVariableEdge(first));
    cube = Biddy_And(cube,Biddy_GetVariableEdge(last));
    cube = Biddy_And(cube,Biddy_GetVariableEdge(extra));
#endif

#if (BDDTYPE == BIDDYTYPEZBDD)
    cube = Biddy_GetBaseSet();
    cube = Biddy_Change(cube,first);
    cube = Biddy_Change(cube,last);
    cube = Biddy_Change(cube,extra);
#endif

#if (BDDTYPE == BIDDYTYPETZBDD)
    cube = Biddy_GetConstantOne();
    cube = Biddy_And(cube,Biddy_GetVariableEdge(first));
    cube = Biddy_And(cube,Biddy_GetVariableEdge(last));
    cube = Biddy_And(cube,Biddy_GetVariableEdge(extra));
#endif

    result1 = Biddy_E(result,first);
    result1 = Biddy_E(result1,last);
    result2 = Biddy_ExistAbstract(result,cube);

    printf("After the existential abstraction of first and last variable, function result has %.0f minterms/combinations\n",Biddy_CountMinterm(result2,SIZE));

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

    printf("After the universal abstraction of first and last variable, function result has %.0f minterms/combinations\n",Biddy_CountMinterm(result2,SIZE));

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

    printf("After the first user-defined abstraction, function result has %.0f minterms/combinations\n",Biddy_CountMinterm(result1,SIZE));

    if (result1 != result2) {
      printf("ERROR: Operation Abstraction is wrong!\n");
    }

    result0 = Biddy_A(result,first);
    result1 = Biddy_A(Biddy_Not(result),last);
    result1 = Biddy_AndAbstract(result0,result1,cube);

    printf("After the second user-defined abstraction, function result has %.0f minterms/combinations\n",Biddy_CountMinterm(result1,SIZE));

    if (result1 != Biddy_GetConstantZero()) {
      printf("ERROR: Operation Abstraction is wrong!\n");
    }

    /* BOOLEAN OPERATIONS AND OPERATION ITE */

    result1 = Biddy_ITE(Biddy_E(result,first),Biddy_E(result,last),Biddy_A(result,last));
    result2 = Biddy_ITE(Biddy_A(result,first),Biddy_E(result,last),Biddy_A(result,last));

    printf("After the first ITE, function result has %.0f minterms/combinations\n",Biddy_CountMinterm(result1,SIZE));
    printf("After the second ITE, function result has %.0f minterms/combinations\n",Biddy_CountMinterm(result2,SIZE));

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

    /* ***  END OF TESTING OPERATIONS *** */

    /* CONVERT THE RESULT INTO OBDDs */
    robdd = Biddy_Copy(MNGOBDD,result);
    printf("OBDD for function result has %u nodes.\n",Biddy_Managed_NodeNumber(MNGOBDD,robdd));
    printf("OBDD without complemented edges for function result has %u nodes (including both constants).\n",Biddy_Managed_NodeNumberPlain(MNGOBDD,robdd));
    printf("OBDD for function result has %llu one-paths.\n",Biddy_Managed_CountPaths(MNGOBDD,robdd));
    printf("OBDD for function result has %.0f minterms/combinations.\n",Biddy_Managed_CountMinterm(MNGOBDD,robdd,SIZE));
    /* GRAPHVIZ/DOT OUTPUT OF THE RESULT */
    if (SIZE < 10) {
      Biddy_Managed_WriteDot(MNGOBDD,"obdd.dot",robdd,"obdd",-1,FALSE);
      printf("USE 'dot -y -Tpng -O obdd.dot' to visualize OBDD for function result.\n");
    }

    /* CONVERT THE RESULT INTO ZBDDs */
    if (Biddy_GetManagerType() != BIDDYTYPETZBDD) {
      rzbdd = Biddy_Copy(MNGZBDD,result);
      printf("ZBDD for function result has %u nodes.\n",Biddy_Managed_NodeNumber(MNGZBDD,rzbdd));
      printf("ZBDD without complemented edges for function result has %u nodes (including both constants).\n",Biddy_Managed_NodeNumberPlain(MNGZBDD,rzbdd));
      printf("ZBDD for function result has %llu one-paths.\n",Biddy_Managed_CountPaths(MNGZBDD,rzbdd));
      printf("ZBDD for function result has %.0f minterms/combinations.\n",Biddy_Managed_CountMinterm(MNGZBDD,rzbdd,SIZE));
      /* GRAPHVIZ/DOT OUTPUT OF THE RESULT */
      if (SIZE < 10) {
        Biddy_Managed_WriteDot(MNGZBDD,"zbdd.dot",rzbdd,"zbdd",-1,FALSE);
        printf("USE 'dot -y -Tpng -O zbdd.dot' to visualize ZBDD for function result.\n");
      }
    }

    if (Biddy_GetManagerType() != BIDDYTYPEZBDD) {
      /* CONVERT THE RESULT INTO TZBDDs */
      rtzbdd = Biddy_Copy(MNGTZBDD,result);
      printf("TZBDD for function result has %u nodes.\n",Biddy_Managed_NodeNumber(MNGTZBDD,rtzbdd));
      printf("TZBDD for function result has %llu one-paths.\n",Biddy_Managed_CountPaths(MNGTZBDD,rtzbdd));
      printf("TZBDD for function result has %.0f minterms/combinations.\n",Biddy_Managed_CountMinterm(MNGTZBDD,rtzbdd,SIZE));
      /* GRAPHVIZ/DOT OUTPUT OF THE RESULT */
      if (SIZE < 10) {
        Biddy_Managed_WriteDot(MNGTZBDD,"tzbdd.dot",rtzbdd,"tzbdd",-1,FALSE);
        printf("USE 'dot -y -Tpng -O tzbdd.dot' to visualize TZBDD for function result.\n");
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

      Biddy_InitMNG(&MNGOBDD,BIDDYTYPEOBDD);
      Biddy_InitMNG(&MNGZBDD,BIDDYTYPEZBDD);
      Biddy_InitMNG(&MNGTZBDD,BIDDYTYPETZBDD);

      support = Biddy_Managed_GetConstantOne(MNGOBDD);
      for (v=0; v<SIZE; v++) {
        support = Biddy_Managed_And(MNGOBDD,support,Biddy_Managed_AddVariable(MNGOBDD));
      }

      robdd = Biddy_Managed_Random(MNGOBDD,support,RATIO);
      rzbdd = Biddy_Managed_Copy(MNGOBDD,MNGZBDD,robdd);
      rtzbdd = Biddy_Managed_Copy(MNGOBDD,MNGTZBDD,robdd);

      s1 = Biddy_Managed_NodeNumber(MNGOBDD,robdd);
      s2 = Biddy_Managed_NodeNumber(MNGZBDD,rzbdd);
      s3 = Biddy_Managed_NodeNumber(MNGTZBDD,rtzbdd);

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

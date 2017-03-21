/* $Revision: 252 $ */
/* $Date: 2017-03-17 23:30:03 +0100 (pet, 17 mar 2017) $ */
/* This file (biddy-example-hanoi.c) is a C file */
/* Author: Robert Meolic (robert.meolic@um.si) */
/* This file has been released into the public domain by the author. */

/* THIS EXAMPLE IS BASED ON WORK PUBLISHED ON CMU, 2005 */
/* Course name: Introduction to Model Checking */
/* Prof. Edmund Clarke */
/* http://www.cs.cmu.edu/~emc/15817-s05/ */

/* The code using CUDD package is original */
/* The code using Biddy package was added by Robert Meolic */

/* This example is compatible with Biddy v1.7 and CUDD v3.0.0 */

/* COMPILE WITH: */
/* gcc -DREPORT -DUNIX -DUSE_BIDDY -O2 -o biddy-example-hanoi biddy-example-hanoi.c -I. -L./bin -lbiddy -lgmp */
/* gcc -DREPORT -DUSE_CUDD -O2 -o cudd-example-hanoi biddy-example-hanoi.c -I ../cudd/include/ -L ../cudd/lib/ -lcudd -lm */

/* Here are original comments */
/*
This example essentially illustrates reachability computation, which
is one of the core operations inside a model checker.  In model
checking, the goal state represents the error state, and the model
checker either:
1. Discovers that the error state is unreachable if the fixed point is
   achieved before hitting the error.
2. Discovers that the error is reachable. In that case it returns a
   counterexample, which is a sequence of steps from the initial state
   to the error state.
The above implementation does not produce a counterexample.
Exercise: Add counterexample generation to the above.
How:
You will need to store pointers to all the image BDDs.  Starting from
the error state, compute a "reverse image", intersect with the
correspoding forward image, pick a state from the intersection and
repeat till you hit the initial state.
*/

/* Towers of Hanoi problem using BDDs */
/* number of towers = 3 */
/* number of disks = SIZE */
#define SIZE 10

#ifdef USE_BIDDY
#  include "biddy.h"
#  define BDDNULL NULL
/* #define Biddy_Inv(f) Biddy_NOT(f) */ /* for compatibility with Biddy v1.6 */
#endif

#ifdef USE_CUDD
#  include <stdio.h>
#  include <stdlib.h>
#  include <stdint.h>
#  include <string.h>
#  include <ctype.h>
#  include <stdarg.h>
#  include <math.h>
#  include "cudd.h"
#  define BDDNULL NULL
#  ifndef TRUE
#    define TRUE (0 == 0)
#  endif
#  ifndef FALSE
#    define FALSE !TRUE
#  endif
typedef char *Biddy_String;
typedef char Biddy_Boolean;
typedef DdNode *Biddy_Edge;
DdManager * manager;   /* BDD Manager */
#endif

#include <time.h>

#ifdef USE_CUDD
#define BIDDYTYPEOBDD 1
#define BIDDYTYPEZBDD 2
#define BIDDYTYPETZBDD 3
#define Biddy_GetManagerType() BIDDYTYPEOBDD
#define Biddy_GetConstantZero() Cudd_ReadLogicZero(manager)
#define Biddy_GetConstantOne() Cudd_ReadOne(manager)
#define Biddy_AddVariable() Cudd_bddNewVar(manager)
#define Biddy_Inv(f) Cudd_Not(f)
#define Biddy_Not(f) Cudd_Not(f)
#define Biddy_And(f,g) Cudd_bddAnd(manager,f,g)
#define Biddy_Or(f,g) Cudd_bddOr(manager,f,g)
#define Biddy_Xor(f,g) Cudd_bddXor(manager,f,g)
#define Biddy_Xnor(f,g) Cudd_bddXnor(manager,f,g)
#define Biddy_Compose(f,g,v) Cudd_bddCompose(manager,f,g,v-1)
#define Biddy_AndAbstract(f,g,c) Cudd_bddAndAbstract(manager,f,g,c)
#define Biddy_DependentVariableNumber(f) Cudd_SupportSize(manager,f)
#define Biddy_CountMinterm(f,n) Cudd_CountMinterm(manager,f,n)
#define Biddy_CountPaths(f) (unsigned long long int)Cudd_CountPath(f)
#define Biddy_NodeNumber(f) Cudd_DagSize(f)
#define Biddy_PrintInfo(s) Cudd_PrintInfo(manager,s)
#define Biddy_NodeTableNum() Cudd_ReadKeys(manager)
#define Biddy_NodeTableSize() Cudd_ReadSlots(manager)
#endif

Biddy_Edge v1[SIZE][3];  /* the current state */
Biddy_Edge v2[SIZE][3];  /* the next state */

/* v[i][j] = 1 means that disk i is in tower j */

Biddy_Edge make_a_move(int i, int j, int k);
Biddy_Edge compute_image(Biddy_Edge R, Biddy_Edge T, Biddy_Edge cube);

int main(int argv, char ** argc) {

  Biddy_Edge I;  /* the initial state BDD */
  Biddy_Edge E,NOT_E;  /* the final state BDD */
  Biddy_Edge T;  /* the transition relation */
  Biddy_Edge R;
  Biddy_Edge image;
  int i,j,found,num_steps;
  Biddy_Edge tmp1,tmp2;
  Biddy_Edge cube;
  unsigned int step;

  clock_t elapsedtime;
  setbuf(stdout,NULL);
  elapsedtime = clock();

  /* initialize the BDD manager with default options */

#ifdef USE_BIDDY
  printf("Using Biddy");
  /* DEFAULT INIT CALL: Biddy_InitAnonymous(BIDDYTYPEOBDD) */
  Biddy_InitAnonymous(BIDDYTYPEOBDD);
#endif

#ifdef USE_CUDD
  printf("Using CUDD");
  /* DEFAULT INIT CALL: Cudd_Init(0,0,CUDD_UNIQUE_SLOTS,CUDD_CACHE_SLOTS,0) */
  manager = Cudd_Init(0,0,CUDD_UNIQUE_SLOTS,CUDD_CACHE_SLOTS,0);
  /* Cudd_SetMaxCacheHard(manager,262144); */
  /* Cudd_AutodynEnable(manager,CUDD_REORDER_SAME); */
#endif

  if (Biddy_GetManagerType() == BIDDYTYPEOBDD) {
    printf(" (OBDDs)...\n");
  }
  else if (Biddy_GetManagerType() == BIDDYTYPEZBDD) {
    printf(" (ZBDDs)...\n");
  }
  else if (Biddy_GetManagerType() == BIDDYTYPETZBDD) {
    printf(" (TZBDDs)...\n");
  }
  else {
    printf(" (unknown)... EXIT!\n");
    exit(1);
  }

  /* the current and next state variables are interleaved */
  /* this is usually a good ordering to start with */
  for(i=0;i<SIZE;i++) {
    for(j=0;j<3;j++) {
      v1[i][j] = Biddy_AddVariable();
      v2[i][j] = Biddy_AddVariable();
    }
  }

  /* for ZBDDs, by adding new variables all the existing ones are changed */
  /* after adding the last variable we have to refresh edges in v1 and v2 */
  /* NOTE: user variables in Biddy start with index 1 */
#ifdef USE_BIDDY
  if (Biddy_GetManagerType() == BIDDYTYPEZBDD) {
    for(i=0;i<SIZE;i++) {
      for(j=0;j<3;j++) {
        v1[i][j] = Biddy_GetVariableEdge(2*(i*3+j)+1);
        v2[i][j] = Biddy_GetVariableEdge(2*(i*3+j)+2);
      }
    }
  }
#endif

  /* Biddy allows a solution with named variables - useful for debugging */
  /*
  {
  Biddy_String name;
  name = malloc(9);
  for (i=0; i<SIZE; i++) {
    for (j=0; j<3; j++) {
      sprintf(name,"v1d%ut%u",i,j);
      v1[i][j] = Biddy_AddVariableByName(name);
      sprintf(name,"v2d%ut%u",i,j);
      v2[i][j] = Biddy_AddVariableByName(name);
    }
  }
  if (Biddy_GetManagerType() == BIDDYTYPEZBDD) {
    for (i=0; i<SIZE; i++) {
      for (j=0; j<3; j++) {
        sprintf(name,"v1d%ut%u",i,j);
        v1[i][j] = Biddy_GetVariableEdge(Biddy_GetVariable(name));
        sprintf(name,"v2d%ut%u",i,j);
        v2[i][j] = Biddy_GetVariableEdge(Biddy_GetVariable(name));
      }
    }
  }
  free(name);
  }
  */

  /* in the initial state, all the disks are in tower 0 */
  I = Biddy_GetConstantOne();
#ifdef USE_CUDD
  Cudd_Ref(I);
#endif
  for(i=0;i<SIZE;i++) {
    tmp1 = Biddy_And(I,v1[i][0]);
#ifdef USE_CUDD
    Cudd_Ref(tmp1);
    Cudd_RecursiveDeref(manager,I);
#endif
    I = tmp1;
  }
  for(i=0;i<SIZE;i++) {
    tmp1 = Biddy_And(I,Biddy_Not(v1[i][1]));
#ifdef USE_CUDD
    Cudd_Ref(tmp1);
    Cudd_RecursiveDeref(manager,I);
#endif
    I = tmp1;
  }
  for(i=0;i<SIZE;i++) {
    tmp1 = Biddy_And(I,Biddy_Not(v1[i][2]));
#ifdef USE_CUDD
    Cudd_Ref(tmp1);
    Cudd_RecursiveDeref(manager,I);
#endif
    I = tmp1;
  }

#ifdef USE_BIDDY
  Biddy_AddTmpFormula(I,1);
  Biddy_Clean();
#endif

  /* in the final state, we want all the disks in tower 2 */
  E = Biddy_GetConstantOne();
#ifdef USE_CUDD
  Cudd_Ref(E);
#endif
  for(i=0;i<SIZE;i++) {
    tmp1 = Biddy_And(E,v1[i][2]);
#ifdef USE_CUDD
    Cudd_Ref(tmp1);
    Cudd_RecursiveDeref(manager,E);
#endif
    E = tmp1;
  }
  for(i=0;i<SIZE;i++) {
    tmp1 = Biddy_And(E,Biddy_Not(v1[i][0]));
#ifdef USE_CUDD
    Cudd_Ref(tmp1);
    Cudd_RecursiveDeref(manager,E);
#endif
    E = tmp1;
  }
  for(i=0;i<SIZE;i++) {
    tmp1 = Biddy_And(E,Biddy_Not(v1[i][1]));
#ifdef USE_CUDD
    Cudd_Ref(tmp1);
    Cudd_RecursiveDeref(manager,E);
#endif
    E = tmp1;
  }

  NOT_E = Biddy_Not(E);

#ifdef USE_BIDDY
  Biddy_AddTmpFormula(I,1);
  Biddy_AddTmpFormula(NOT_E,1);
  Biddy_Clean();
#endif

  T = Biddy_GetConstantZero();
#ifdef USE_CUDD
  Cudd_Ref(T);
#endif

  /* the transition relation is the disjunction of all possible moves */
  for(i=0;i<SIZE;i++) {
    for(j=0;j<3;j++) {
      tmp1 = make_a_move(i,j,(j+1)%3);
      tmp2 = Biddy_Or(T,tmp1);
#ifdef USE_CUDD
      Cudd_Ref(tmp2);
      Cudd_RecursiveDeref(manager,tmp1);
      Cudd_RecursiveDeref(manager,T);
#endif
      T = tmp2;
      tmp1 = make_a_move(i,j,(j+2)%3);
      tmp2 = Biddy_Or(T,tmp1);
#ifdef USE_CUDD
      Cudd_Ref(tmp2);
      Cudd_RecursiveDeref(manager,tmp1);
      Cudd_RecursiveDeref(manager,T);
#endif
      T = tmp2;
#ifdef USE_BIDDY
      Biddy_AddTmpFormula(I,1);
      Biddy_AddTmpFormula(NOT_E,1);
      Biddy_AddTmpFormula(T,1);
      Biddy_Clean();
#endif
    }
  }

  /* cube of the current state variables, needed by bddAndAbstract */
  cube = Biddy_GetConstantOne();
  #ifdef USE_CUDD
    Cudd_Ref(cube);
  #endif
  if ((Biddy_GetManagerType() == BIDDYTYPEOBDD) ||
      (Biddy_GetManagerType() == BIDDYTYPETZBDD))
  {
    for(i=0;i<SIZE;i++) {
      for(j=0;j<3;j++) {
        tmp1 = Biddy_And(cube,v1[i][j]);
  #ifdef USE_CUDD
        Cudd_Ref(tmp1);
        Cudd_RecursiveDeref(manager,cube);
  #endif
        cube = tmp1;
      }
    }
  }
#ifdef USE_BIDDY
  else if (Biddy_GetManagerType() == BIDDYTYPEZBDD) {
    cube = Biddy_GetBaseSet();
    for(i=0;i<SIZE;i++) {
      for(j=0;j<3;j++) {
        cube = Biddy_Change(cube,2*(i*3+j)+1);
      }
    }
  }
#endif

#ifdef USE_BIDDY
  Biddy_AddTmpFormula(I,1);
  Biddy_AddTmpFormula(NOT_E,1);
  Biddy_AddTmpFormula(T,1);
  Biddy_AddTmpFormula(cube,1);
  Biddy_Clean();
#endif

  found = 0;
  num_steps = 0;

  /* R contains the states reached so far, initialized to I */
  R = I;
#ifdef USE_CUDD
  Cudd_Ref(R);
#endif

  /* fixed point computation */
  step = 0;
  while(1) {

#ifdef REPORT
    if (step % (SIZE*SIZE) == 0) {
      printf(
        "R in while loop (step=%u): %u variables, %.0f minterms, %u nodes.\n",
        step,3*SIZE,Biddy_CountMinterm(R,3*SIZE),Biddy_NodeNumber(R)
      );
    }
#endif

#ifdef USE_BIDDY
    Biddy_AddTmpFormula(R,1);
    Biddy_AddTmpFormula(NOT_E,1);
    Biddy_AddTmpFormula(T,1);
    Biddy_AddTmpFormula(cube,1);
    Biddy_Clean();
#endif

    /* check if we reached the goal state */
    tmp1 = Biddy_Or(NOT_E,R);
#ifdef USE_CUDD
    Cudd_Ref(tmp1);
#endif
    if (tmp1 == Biddy_GetConstantOne()) {
      found = 1;   /* goal reached */
      break;
    }
#ifdef USE_CUDD
    Cudd_RecursiveDeref(manager,tmp1);
#endif

    /* compute the successors of the current state */
    image = compute_image(R,T,cube);
    num_steps ++;

    /* check if we reached a new state */
    /* NOTE: fixed point check, easy with BDDs as they are canonical */
    tmp1 = Biddy_Or(Biddy_Not(image),R);
#ifdef USE_CUDD
    Cudd_Ref(tmp1);
#endif
    if (tmp1 == Biddy_GetConstantOne())
      break;  /* no new state reached */
#ifdef USE_CUDD
    Cudd_RecursiveDeref(manager,tmp1);
#endif

    /* add the new states to the reached set */
    tmp1 = Biddy_Or(image,R);
#ifdef USE_CUDD
    Cudd_Ref(tmp1);
    Cudd_RecursiveDeref(manager,R);
    Cudd_RecursiveDeref(manager,image);
#endif

    R = tmp1;
    step++;
  }

#ifdef REPORT
    printf(
      "R in while loop (step=%u): %u variables, %.0f minterms, %u nodes.\n",
      step,3*SIZE,Biddy_CountMinterm(R,3*SIZE),Biddy_NodeNumber(R)
    );
#endif

  /* DUMP RESULT USING GRAPHVIZ/DOT */
  /*
#ifdef USE_BIDDY
  Biddy_WriteDot("hanoi-biddy.dot",image,"r",-1,FALSE);
#elif USE_CUDD
  {
  FILE * fp;
  fp = fopen("hanoi-cudd.dot","w");
  Cudd_DumpDot(manager,1,&image,NULL,NULL,fp);
  fclose(fp);
  }
#endif
  */

#ifdef USE_CUDD
  Cudd_RecursiveDeref(manager,tmp1);
  Cudd_RecursiveDeref(manager,image);
#endif

  elapsedtime = clock()-elapsedtime;

  if (found) {
    printf("Goal Reached in %d steps\n",num_steps);
  }
  else
    printf("Goal not reached\n");

  /* we have problems with passing stdout in the case you compile this file */
  /* with MINGW and use biddy.dll generated with Visual Studio. */
  /* In such cases, please, use Biddy_PrintInfo(NULL) */
#ifdef REPORT
  Biddy_PrintInfo(stdout);
#endif

  fprintf(stderr,"clock() TIME = %.2f\n",elapsedtime/(1.0*CLOCKS_PER_SEC));

  /* free BDDs and the manager */

  /* EXIT */
#ifdef USE_BIDDY
  Biddy_Exit();
#endif
#ifdef USE_CUDD
  Cudd_RecursiveDeref(manager,cube);
  Cudd_RecursiveDeref(manager,I);
  Cudd_RecursiveDeref(manager,T);
  Cudd_RecursiveDeref(manager,E);
#ifdef REPORT
  printf("CUDD: nodes with non-zero reference counts: %d\n",Cudd_CheckZeroRef(manager));
#endif
  Cudd_Quit(manager);
#endif

}

/* returns a BDD representing move of disk i from tower j to tower k */
Biddy_Edge make_a_move(int i, int j, int k)
{
  Biddy_Edge result;
  int l,m;
  Biddy_Edge tmp1;
  Biddy_Edge tmp2;

  result = v1[i][j];  /* disk i is in tower j */
#ifdef USE_CUDD
  Cudd_Ref(result);
#endif

  /* this loop enforces that there is no smaller disk in tower j or tower k */
  /* no smaller disk in tower j implies that the disk at hand is at the top */
  /* no smaller disk in tower k implies that we can move the disk to tower k */
  for(l=i-1;l>=0;l--) {
    tmp1 = Biddy_And(result,Biddy_Not(v1[l][j]));
#ifdef USE_CUDD
    Cudd_Ref(tmp1);
    Cudd_RecursiveDeref(manager,result);
#endif
    result = tmp1;

    tmp1 = Biddy_And(result,Biddy_Not(v1[l][k]));
#ifdef USE_CUDD
    Cudd_Ref(tmp1);
    Cudd_RecursiveDeref(manager,result);
#endif
    result = tmp1;
  }

  /* move the current disk to tower k */
  tmp1 = Biddy_And(result,v2[i][k]);
#ifdef USE_CUDD
  Cudd_Ref(tmp1);
  Cudd_RecursiveDeref(manager,result);
#endif
  result = tmp1;

  tmp1 = Biddy_And(result,Biddy_Not(v2[i][(k+1)%3]));
#ifdef USE_CUDD
  Cudd_Ref(tmp1);
  Cudd_RecursiveDeref(manager,result);
#endif
  result = tmp1;

  tmp1 = Biddy_And(result,Biddy_Not(v2[i][(k+2)%3]));
#ifdef USE_CUDD
  Cudd_Ref(tmp1);
  Cudd_RecursiveDeref(manager,result);
#endif
  result = tmp1;

  /* the other disks stay where they are */
  for(l=0;l<SIZE;l++) {
    if (l!=i) {
      for(m=0;m<3;m++) {

        tmp1 = Biddy_Xnor(v1[l][m],v2[l][m]);
#ifdef USE_CUDD
        Cudd_Ref(tmp1);
#endif

        tmp2 = Biddy_And(result,tmp1);
#ifdef USE_CUDD
        Cudd_Ref(tmp2);
        Cudd_RecursiveDeref(manager,result);
        Cudd_RecursiveDeref(manager,tmp1);
#endif

        result = tmp2;
      }
    }
  }

  return result;
}

/* given a set of states R and a transition relation T, returns the BDD for the
   states that can be reached in one step from R following T */
Biddy_Edge compute_image(Biddy_Edge R, Biddy_Edge T, Biddy_Edge cube)
{
  Biddy_Edge result;
  int i;
  int j;
  Biddy_Edge tmp1;

  /* the following Cudd function computes the conjunction of R and T; */
  /* and quantifies out the variables in cube ( the current state variables ) */
  result = Biddy_AndAbstract(R,T,cube);
#ifdef USE_CUDD
  Cudd_Ref(result);
#endif

  /* DEBUGGING, ONLY */
  /*
  printf(
    "compute_image after ANDABSTRACT: i=%d, j=%d, %u variables, %.0f minterms, %u nodes.\n",
    i,j,3*SIZE,Biddy_CountMinterm(result,3*SIZE),Biddy_NodeNumber(result)
  );
  */

  /* rename the next state variables to the current state variables */
  for(i=0;i<SIZE;i++)
    for(j=0;j<3;j++) {
      /* compose replaces v2[i][j] with v1[i][j] */
      /* original: Cudd_bddCompose(manager,result,v1[i][j],2*(i*3+j)+1) */
      /* NOTE: user variables in CUDD start with index 0, user variables in Biddy start with index 1 */
      tmp1 = Biddy_Compose(result,v1[i][j],2*(i*3+j)+2);
#ifdef USE_CUDD
      Cudd_Ref(tmp1);
      Cudd_RecursiveDeref(manager,result);
#endif
      result = tmp1;

      /* DEBUGGING, ONLY */
      /*
      printf(
        "compute_image: i=%d, j=%d, %u variables, %.0f minterms, %u nodes.\n",
        i,j,3*SIZE,Biddy_CountMinterm(result,3*SIZE),Biddy_NodeNumber(result)
      );
      */

    }

  return result;
}

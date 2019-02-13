/* $Revision: 545 $ */
/* $Date: 2019-02-11 14:07:50 +0100 (pon, 11 feb 2019) $ */
/* This file (biddy-example-hanoi.c) is a C file */
/* Author: Robert Meolic (robert@meolic.com) */
/* This file has been released into the public domain by the author. */

/* THIS EXAMPLE IS BASED ON WORK PUBLISHED ON CMU, 2005 */
/* Course name: Introduction to Model Checking */
/* Prof. Edmund Clarke */
/* http://www.cs.cmu.edu/~emc/15817-s05/ */

/* The code using CUDD package is copyed from the original */
/* The code using Biddy package was added by Robert Meolic */

/* This example is compatible with Biddy v1.8 and CUDD v3.0.0 */
/* This example uses biddy-cudd.h and biddy-cudd.c */

/* COMPILE WITH: */
/* gcc -DREPORT -DUNIX -DBIDDY -DOBDD -O2 -o biddy-example-hanoi-obdd biddy-example-hanoi.c -I. -L./bin -static -lbiddy -lgmp */
/* gcc -DREPORT -DUNIX -DBIDDY -DOBDDC -O2 -o biddy-example-hanoi-obddc biddy-example-hanoi.c -I. -L./bin -static -lbiddy -lgmp */
/* gcc -DREPORT -DUNIX -DBIDDY -DZBDD -O2 -o biddy-example-hanoi-zbdd biddy-example-hanoi.c -I. -L./bin -static -lbiddy -lgmp */
/* gcc -DREPORT -DUNIX -DBIDDY -DZBDDC -O2 -o biddy-example-hanoi-zbddc biddy-example-hanoi.c -I. -L./bin -static -lbiddy -lgmp */
/* gcc -DREPORT -DUNIX -DBIDDY -DTZBDD -O2 -o biddy-example-hanoi-tzbdd biddy-example-hanoi.c -I. -L./bin -static -lbiddy -lgmp */
/* gcc -DREPORT -DCUDD -DOBDDC -O2 -o cudd-example-hanoi-obddc biddy-example-hanoi.c -I ../cudd/include/ -L ../cudd/lib/ -lcudd -lm */

/* Here are the original comments */
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
#define MAXSIZE 16
#define SIZE 10

#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <time.h>

#include "biddy-cudd.h"

Biddy_Edge v1[MAXSIZE][3];  /* the current state */
Biddy_Edge v2[MAXSIZE][3];  /* the next state */

/* v[i][j] = 1 means that disk i is in tower j */

Biddy_Edge make_a_move(unsigned int size, unsigned int i, unsigned int j, unsigned int k);
Biddy_Edge compute_image(unsigned int size, Biddy_Edge R, Biddy_Edge T, Biddy_Edge cube);

int main(int argc, char** argv) {

  unsigned int size;
  Biddy_Edge I;  /* the initial state BDD */
  Biddy_Edge E,NOT_E;  /* the final state BDD */
  Biddy_Edge T;  /* the transition relation */
  Biddy_Edge R;
  Biddy_Edge image;
  unsigned int i, j;
  int found, num_steps;
  Biddy_Edge tmp1,tmp2;
  Biddy_Edge cube;

  clock_t elapsedtime;

  if (argc > 1) {
    sscanf(argv[1],"%u",&size);
    if (size > MAXSIZE) {
      printf("ERROR: MAXSIZE = %u\n",MAXSIZE);
      exit(1);
    }
  } else {
    size = SIZE;
  }

  setbuf(stdout,NULL);
  elapsedtime = clock();

#include "biddy-cudd.c" /* this will initialize BDD manager */

#ifdef CUDD
  /* Cudd_SetMaxCacheHard(manager,262144); */
  /* Cudd_AutodynEnable(manager,CUDD_REORDER_SAME); */
#endif

  printf("%s\n",Biddy_GetManagerName());
  printf("SIZE: %u\n",size);

  /* create BDD variables */
  /* the current and next state variables are interleaved */
  /* this is usually a good ordering to start with */
  /* Biddy allows state names, in CUDD we simply ignore them */
  /* NOTE for Biddy: ZBDD and TZBDD variables are added in the reverse order by default */

#ifdef BIDDY
  if ((Biddy_GetManagerType() == BIDDYTYPEOBDD) || (Biddy_GetManagerType() == BIDDYTYPEOBDDC))
#endif
#ifdef CUDD
  if (TRUE)
#endif
  {
    char name[5];
    name[0] = name[1] = name[2] = name[3] = name[4] = 0;
    for(i=0;i<size;i++) {
      name[1] = '0' + i/10;
      name[2] = '0' + i%10;
      for(j=0;j<3;j++) {
        name[3] = '0' + j;
        v1[i][j] = Biddy_AddVariableEdge();
        name[0] = 'C';
        Biddy_ChangeVariableName(Biddy_VariableTableNum()-1,name); /* rename the last added variable */
        v2[i][j] = Biddy_AddVariableEdge();
        name[0] = 'N';
        Biddy_ChangeVariableName(Biddy_VariableTableNum()-1,name); /* rename the last added variable */
        REF(v1[i][j]);
        REF(v2[i][j]);
      }
    }
  }

#ifdef BIDDY
  if ((Biddy_GetManagerType() == BIDDYTYPEZBDD) || (Biddy_GetManagerType() == BIDDYTYPEZBDDC) ||
      (Biddy_GetManagerType() == BIDDYTYPETZBDD) || (Biddy_GetManagerType() == BIDDYTYPETZBDDC))
#endif
#ifdef CUDD
  if (FALSE)
#endif
  {
    char name[5];
    name[0] = name[1] = name[2] = name[3] = name[4] = 0;
    for(i=0;i<size;i++) {
      name[1] = '0' + (size-i-1)/10;
      name[2] = '0' + (size-i-1)%10;
      for(j=0;j<3;j++) {
        name[3] = '0' + (3-j-1);
        v2[size-i-1][3-j-1] = Biddy_AddVariableEdge();
        name[0] = 'N';
        Biddy_ChangeVariableName(Biddy_VariableTableNum()-1,name); /* rename the last added variable */
        v1[size-i-1][3-j-1] = Biddy_AddVariableEdge();
        name[0] = 'C';
        Biddy_ChangeVariableName(Biddy_VariableTableNum()-1,name); /* rename the last added variable */
        REF(v2[size-i-1][3-j-1]);
        REF(v1[size-i-1][3-j-1]);
      }
    }
  }

  /* For ZBDDs, by adding new variables all the existing ones are changed. */
  /* Thus, after adding the last variable we have to refresh the array of variables. */
  /* NOTE: Biddy_GetVariableEdge returns variables by the creation time, not by the order used in the BDD */
  /* NOTE for Biddy: user variables start with index 1 */
  /* NOTE for CUDD: user variables start with index 0 */

  if ((Biddy_GetManagerType() == BIDDYTYPEZBDD) || (Biddy_GetManagerType() == BIDDYTYPEZBDDC)) {
    for(i=0;i<size;i++) {
      for(j=0;j<3;j++) {
#ifdef BIDDY
        /* variable edges in Biddy never become obsolete */
        v1[i][j] = Biddy_GetVariableEdge(2*(size*3-(i*3+j)));
        v2[i][j] = Biddy_GetVariableEdge(2*(size*3-(i*3+j))-1);
#endif
#ifdef CUDD
        /* variable edges in CUDD are referenced - this is required for ZBDDs */
        DEREF(v1[i][j]);
        DEREF(v2[i][j]);
        v1[i][j] = Cudd_zddIthVar(manager,2*(i*3+j));
        v2[i][j] = Cudd_zddIthVar(manager,2*(i*3+j)+1);
        REF(v1[i][j]);
        REF(v2[i][j]);
#endif
      }
    }
  }

  /* DEBUGGING */
  /**/
#ifdef BIDDY
  {
  Biddy_Variable v;
  v = 0;
  do {
    v = Biddy_GetPrevVariable(v);
  } while (!(Biddy_IsLowest(v)));
  do {
    printf("[%s]",Biddy_GetVariableName(v));
    v = Biddy_GetNextVariable(v);
  } while (v != 0);
  printf("\n");
  }
#endif
  /**/

  /* in the initial state, all the disks are in tower 0 */

  I = Biddy_GetConstantOne();
  REF(I);
  for(i=0;i<size;i++) {
    tmp1 = Biddy_And(I,v1[i][0]);
    REF(tmp1);
    DEREF(I);
    I = tmp1;
  }
  for(i=0;i<size;i++) {
    tmp1 = Biddy_And(I,Biddy_Not(v1[i][1]));
    REF(tmp1);
    DEREF(I);
    I = tmp1;
  }
  for(i=0;i<size;i++) {
    tmp1 = Biddy_And(I,Biddy_Not(v1[i][2]));
    REF(tmp1);
    DEREF(I);
    I = tmp1;
  }

  MARK(I);
  SWEEP();

  /* in the final state, we want all the disks in tower 2 */

  E = Biddy_GetConstantOne();
  REF(E);
  for(i=0;i<size;i++) {
    tmp1 = Biddy_And(E,v1[i][2]);
    REF(tmp1);
    DEREF(E);
    E = tmp1;
  }

  for(i=0;i<size;i++) {
    tmp1 = Biddy_And(E,Biddy_Not(v1[i][0]));
    REF(tmp1);
    DEREF(E);
    E = tmp1;
  }

  for(i=0;i<size;i++) {
    tmp1 = Biddy_And(E,Biddy_Not(v1[i][1]));
    REF(tmp1);
    DEREF(E);
    E = tmp1;
  }

  NOT_E = Biddy_Not(E);
  REF(NOT_E);
  DEREF(E);

  /* E is not needed anymore, only NOT_E is used further */

  MARK(I);
  MARK(NOT_E);
  SWEEP();

  /* the transition relation is the disjunction of all possible moves */

  T = Biddy_GetConstantZero();
  REF(T);

  for(i=0;i<size;i++) {
    for(j=0;j<3;j++) {

      tmp1 = make_a_move(size,i,j,(j+1)%3);

      tmp2 = Biddy_Or(T,tmp1);
      REF(tmp2);
      DEREF(tmp1);
      DEREF(T);
      T = tmp2;

      tmp1 = make_a_move(size,i,j,(j+2)%3);

      tmp2 = Biddy_Or(T,tmp1);
      REF(tmp2);
      DEREF(tmp1);
      DEREF(T);
      T = tmp2;

      MARK(I);
      MARK(NOT_E);
      MARK(T);
      SWEEP();

    }
  }

  /* cube of the current state variables, needed by bddAndAbstract */
  if ((Biddy_GetManagerType() == BIDDYTYPEOBDD) || (Biddy_GetManagerType() == BIDDYTYPEOBDDC) ||
      (Biddy_GetManagerType() == BIDDYTYPETZBDD) || (Biddy_GetManagerType() == BIDDYTYPETZBDDC))
  {
    cube = Biddy_GetConstantOne();
    REF(cube);
    for(i=0;i<size;i++) {
      for(j=0;j<3;j++) {
        tmp1 = Biddy_And(cube,v1[i][j]);
        REF(tmp1);
        DEREF(cube);
        cube = tmp1;
      }
    }
  }
  else if ((Biddy_GetManagerType() == BIDDYTYPEZBDD) || (Biddy_GetManagerType() == BIDDYTYPEZBDDC)) {
#ifdef BIDDY
    cube = Biddy_GetBaseSet();
    for(i=0;i<size;i++) {
      for(j=0;j<3;j++) {
        /* for Biddy, user variables start with index 1 */
        /* for Biddy, ZBDD variables have been generated in the following order: */
        /* (1)=v2[size-1][2], (2)=v1[size-1][2], (3)=v2[size-1][1], (4)=v1[size-1][1],..., (2*size*3)=v1[0][0] */
        cube = Biddy_Change(cube,2*(i*3+j)+2);
      }
    }
#endif
#ifdef CUDD
    /* NOT IMPLEMENTED, YET */
#endif
  }

  MARK(I);
  MARK(NOT_E);
  MARK(T);
  MARK(cube);
  SWEEP();

  found = 0;
  num_steps = 0;

  /* R contains the states reached so far, initialized to I */
  R = I;
  REF(R);

  /* fixed point computation */
  while(1) {

#ifdef REPORT
    if (num_steps % (size*size) == 0) {
      printf(
        "R in while loop (num_steps=%u): %u variables, %.0f minterms, %u nodes.\n",
        num_steps,Biddy_DependentVariableNumber(R,FALSE),Biddy_CountMinterms(R,3*size),Biddy_CountNodes(R)
      );
    }
#endif

    MARK(R);
    MARK(NOT_E);
    MARK(T);
    MARK(cube);
    SWEEP();

    /* check if we reached the goal state */
    tmp1 = Biddy_Or(NOT_E,R);
    if (tmp1 == Biddy_GetConstantOne()) {
      found = 1;   /* goal reached */
      break;
    }

    /* compute the successors of the current state */
    image = compute_image(size,R,T,cube);
    num_steps++;

    /* DEBUGGING */
    /*
    printf(
      "image (num_steps=%u): %u variables, %.0f minterms, %u nodes.\n",
      num_steps,Biddy_DependentVariableNumber(image,FALSE),Biddy_CountMinterms(image,3*size),Biddy_CountNodes(image)
    );
    */

    /* check if we reached a new state */
    /* NOTE: fixed point check, easy with BDDs as they are canonical */
    tmp1 = Biddy_Not(image);
    REF(tmp1);
    tmp2 = Biddy_Or(tmp1,R);
    REF(tmp2);
    DEREF(tmp1);
    if (tmp2 == Biddy_GetConstantOne()) {
      DEREF(tmp2);
      DEREF(image);
      break;  /* no new state reached */
    }
    DEREF(tmp2);

    /* add the new states to the reached set */
    tmp1 = Biddy_Or(image,R);
    REF(tmp1);
    DEREF(image);
    DEREF(R);
    R = tmp1;

  }

#ifdef REPORT
  printf(
    "R after while loop (num_steps=%u): %u variables, %.0f minterms, %u nodes.\n",
    num_steps,Biddy_DependentVariableNumber(R,FALSE),Biddy_CountMinterms(R,3*size),Biddy_CountNodes(R)
  );
#endif

  /* DUMP RESULT USING GRAPHVIZ/DOT */
  /*
  Biddy_WriteDot("hanoi.dot",R,"R",-1,FALSE);
  */

  elapsedtime = clock()-elapsedtime;

  if (found) {
    printf("Goal Reached in %d steps\n",num_steps);
  }
  else
    printf("Goal not reached\n");

  /* we have problems with passing stdout in the case you compile this file */
  /* with MINGW and use biddy.dll generated with Visual Studio. */
  /* In such cases, please, use Biddy_PrintInfo(NULL) */
#ifdef REPORTX
  Biddy_PrintInfo(stdout);
#endif

  fprintf(stderr,"clock() TIME = %.2f\n",elapsedtime/(1.0*CLOCKS_PER_SEC));

  /* EXIT */

  DEREF(I);
  DEREF(NOT_E);
  DEREF(T);
  DEREF(cube);
  DEREF(R);
  for(i=0;i<size;i++) {
    for(j=0;j<3;j++) {
      DEREF(v1[i][j]);
      DEREF(v2[i][j]);
    }
  }

#ifdef REPORT
#ifdef CUDD
  printf("CUDD: nodes with non-zero reference counts: %d\n",Cudd_CheckZeroRef(manager));
#endif
#endif

  Biddy_Exit();
}

/* returns a BDD representing move of disk i from tower j to tower k */
Biddy_Edge make_a_move(unsigned int size, unsigned int i, unsigned int j,
                        unsigned int k)
{
  Biddy_Edge result;
  unsigned int l,m;
  Biddy_Edge tmp1;
  Biddy_Edge tmp2;

  result = v1[i][j];  /* disk i is in tower j */
  REF(result);

  /* this loop enforces that there is no smaller disk in tower j or tower k */
  /* no smaller disk in tower j implies that the disk at hand is at the top */
  /* no smaller disk in tower k implies that we can move the disk to tower k */
  for(l=0;l<i;l++) {
    tmp1 = Biddy_And(result,Biddy_Not(v1[l][j]));
    REF(tmp1);
    DEREF(result);
    result = tmp1;

    tmp1 = Biddy_And(result,Biddy_Not(v1[l][k]));
    REF(tmp1);
    DEREF(result);
    result = tmp1;
  }

  /* move the current disk to tower k */
  tmp1 = Biddy_And(result,v2[i][k]);
  REF(tmp1);
  DEREF(result);
  result = tmp1;

  tmp1 = Biddy_And(result,Biddy_Not(v2[i][(k+1)%3]));
  REF(tmp1);
  DEREF(result);
  result = tmp1;

  tmp1 = Biddy_And(result,Biddy_Not(v2[i][(k+2)%3]));
  REF(tmp1);
  DEREF(result);
  result = tmp1;

  /* the other disks stay where they are */
  for(l=0;l<size;l++) {
    if (l!=i) {
      for(m=0;m<3;m++) {

        tmp1 = Biddy_Xnor(v1[l][m],v2[l][m]);
        REF(tmp1);

        tmp2 = Biddy_And(result,tmp1);
        REF(tmp2);
        DEREF(result);
        DEREF(tmp1);

        result = tmp2;
      }
    }
  }

  return result;
}

/* given a set of states R and a transition relation T, returns the BDD */
/* for the states that can be reached in one step from R following T */
Biddy_Edge compute_image(unsigned int size, Biddy_Edge R, Biddy_Edge T,
                          Biddy_Edge cube)
{
  Biddy_Edge result;
  unsigned int i;
  unsigned int j;
  Biddy_Edge tmp1;

  /* the following Cudd function computes the conjunction of R and T; */
  /* and quantifies out the variables in cube ( the current state variables ) */
  result = Biddy_AndAbstract(R,T,cube);
  REF(result);

  /* DEBUGGING, ONLY */
  /*
  printf(
    "compute_image after ANDABSTRACT: i=%d, j=%d, %u variables, %.0f minterms, %u nodes.\n",
    i,j,3*size,Biddy_CountMinterms(result,3*size),Biddy_CountNodes(result)
  );
  */

  /* rename the next state variables to the current state variables */
  for(i=0;i<size;i++) {
    for(j=0;j<3;j++) {
      /* compose replaces v2[i][j] with v1[i][j] */
      /* original: Cudd_bddCompose(manager,result,v1[i][j],2*(i*3+j)+1) */
      /* for Biddy, user variables start with index 1 */
      /* for Biddy, OBDD variables have been generated in the following order: */
      /* (1)=v1[0][0], (2)=v2[0][0], (3)=v1[0][1], (4)=v2[0][1],..., (2*3*size)=v2[size-1][2] */
      /* for Biddy, ZBDDs and TZBDDs variables have been generated in the following order: */
      /* (1)=v2[size-1][2], (2)=v1[size-1][2], (3)=v2[size-1][1], (4)=v1[size-1][1],..., (2*3*size)=v1[0][0] */
      /* for CUDD, user variables start with index 0 */
      /* for CUDD, OBDDs and ZBDDs variables have been generated in the following order: */
      /* (0)=v1[0][0], (1)=v2[0][0], (2)=v1[0][1], (3)=v2[0][1],..., (2*3*size-1)=v2[size-1][2] */
#ifdef BIDDY
      if ((Biddy_GetManagerType() == BIDDYTYPEOBDD) || (Biddy_GetManagerType() == BIDDYTYPEOBDDC))
      {
        tmp1 = Biddy_Compose(result,v1[i][j],2*(i*3+j)+2);
      }
      else if ((Biddy_GetManagerType() == BIDDYTYPEZBDD) || (Biddy_GetManagerType() == BIDDYTYPEZBDDC) ||
                (Biddy_GetManagerType() == BIDDYTYPETZBDD) || (Biddy_GetManagerType() == BIDDYTYPETZBDDC))
      {
        tmp1 = Biddy_Compose(result,v1[i][j],2*(size*3-(i*3+j))-1);
      }
#endif
#ifdef CUDD
      tmp1 = Biddy_Compose(result,v1[i][j],2*(i*3+j)+1);
#endif
      REF(tmp1);
      DEREF(result);
      result = tmp1;

      /* DEBUGGING, ONLY */
      /*
      printf(
        "compute_image: i=%d, j=%d, %u variables, %.0f minterms, %u nodes.\n",
        i,j,3*size,Biddy_CountMinterms(result,3*size),Biddy_CountNodes(result)
      );
      */

    }
  }

  return result;
}

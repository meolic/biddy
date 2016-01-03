/* $Revision: 131 $ */
/* $Date: 2016-01-02 09:42:28 +0100 (sob, 02 jan 2016) $ */
/* This file (biddy-example-8queens.c) is a C file */
/* Author: Robert Meolic (robert.meolic@um.si) */
/* This file has been released into the public domain by the author. */

/* This example is compatible with Biddy v1.5 and CUDD v2.5.1 */
/* You must define UNIX, MACOSX, or MINGW, but Visual Studio is auto-recognized */
/* You must define USE_BIDDY iff you are using Biddy via dynamic library */

/* COMPILE WITH: */
/* gcc -DUNIX -DUSE_BIDDY -o biddy-example-8queens biddy-example-8queens.c -I. -L./bin -lbiddy -lgmp */
/* gcc -DUSE_CUDD -o biddy-example-8queens biddy-example-8queens.c -I ../cudd-2.5.1/include/ -L ../cudd-2.5.1/cudd/ -L ../cudd-2.5.1/util/ -L ../cudd-2.5.1/mtr/ -L ../cudd-2.5.1/st/ -L ../cudd-2.5.1/epd/ -lcudd -lutil -lepd -lmtr -lst -lm */

/* default size, this can be overriden via argument */
#define SIZE 8

#ifdef USE_BIDDY
#  include "biddy.h"
#  define BDDNULL NULL
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
DdManager *manager;
#endif

#ifdef USE_CUDD
#define Biddy_GetConstantZero() Cudd_ReadLogicZero(manager)
#define Biddy_GetConstantOne() Cudd_ReadOne(manager)
#define Biddy_AddVariable() Cudd_bddNewVar(manager)
#define Biddy_Not(f) Cudd_Not(f)
#define Biddy_And(f,g) Cudd_bddAnd(manager,f,g)
#define Biddy_Or(f,g) Cudd_bddOr(manager,f,g)
#define Biddy_VariableNumber(f) Cudd_SupportSize(manager,f)
#define Biddy_CountMinterm(f,n) Cudd_CountMinterm(manager,f,n)
#define Biddy_NodeNumber(f) Cudd_DagSize(f)
#define Biddy_PrintInfo(s) Cudd_PrintInfo(manager,s)
#define Biddy_Exit()
#endif

int main(int argc, char** argv) {
  unsigned int i,j,k,n,size;
  Biddy_Edge **board;
  Biddy_Edge zero,one,tmp;
  Biddy_Edge t,r;
 
  if (argc == 2) {
    sscanf(argv[1],"%u",&size);
  } else {
    size = SIZE;
  }
  
  if ((size < 2) || (size > 10)) {
    printf("WRONG SIZE!\n");
    exit(1);
  }

  board = (Biddy_Edge **) malloc((size+1)*sizeof(Biddy_Edge *));
  for (i=0; i<=size; i++) {
    board[i] = (Biddy_Edge *) malloc((size+1)*sizeof(Biddy_Edge));
  }

#ifdef USE_BIDDY
  Biddy_Init();
#elif USE_CUDD
  manager = Cudd_Init(0,0,CUDD_UNIQUE_SLOTS,CUDD_CACHE_SLOTS,0);
#endif

  zero = Biddy_GetConstantZero();
#ifdef USE_BIDDY
#elif USE_CUDD
  Cudd_Ref(zero);
#endif

  one = Biddy_GetConstantOne();
#ifdef USE_BIDDY
#elif USE_CUDD
  Cudd_Ref(one);
#endif

  for (i=1; i<=size; i++) {
    board[0][i] = board[i][0] = NULL;
  }

  for (i=1; i<=size; i++) {
    for (j=1; j<=size; j++) {
      board[i][j] = Biddy_AddVariable();
    }
  }
    
  r = one;
#ifdef USE_BIDDY
#elif USE_CUDD
  Cudd_Ref(r);
#endif

  /* ROWS */
  for (i=1; i<=size; i++) {
    for (j=1; j<=size; j++) {
      t = one;
#ifdef USE_BIDDY
#elif USE_CUDD
      Cudd_Ref(t);
#endif
      for (k=1; k<=size; k++) {
        if (k != j) {
          tmp = Biddy_And(t,Biddy_Not(board[i][k]));
#ifdef USE_BIDDY
#elif USE_CUDD
          Cudd_Ref(tmp);
          Cudd_RecursiveDeref(manager,t);
#endif
          t = tmp;
        }
      }
      tmp = Biddy_Or(t,Biddy_Not(board[i][j]));
#ifdef USE_BIDDY
#elif USE_CUDD
      Cudd_Ref(tmp);
      Cudd_RecursiveDeref(manager,t);
#endif
      t = tmp;
      tmp = Biddy_And(r,t);
#ifdef USE_BIDDY
      Biddy_AddTmpFormula(tmp,1);
      Biddy_Clean();
#elif USE_CUDD
      Cudd_Ref(tmp);
      Cudd_RecursiveDeref(manager,t);
      Cudd_RecursiveDeref(manager,r);
#endif
      r = tmp;
    }
  }

  /* COLUMNS */
  for (i=1; i<=size; i++) {
    for (j=1; j<=size; j++) {
      t = one;
#ifdef USE_BIDDY
#elif USE_CUDD
      Cudd_Ref(t);
#endif
      for (k=1; k<=size; k++) {
        if (k != i) {
          tmp = Biddy_And(t,Biddy_Not(board[k][j]));
#ifdef USE_BIDDY
#elif USE_CUDD
          Cudd_Ref(tmp);
          Cudd_RecursiveDeref(manager,t);
#endif
          t = tmp;
        }
      }
      tmp = Biddy_Or(t,Biddy_Not(board[i][j]));
#ifdef USE_BIDDY
#elif USE_CUDD
      Cudd_Ref(tmp);
      Cudd_RecursiveDeref(manager,t);
#endif
      t = tmp;
      tmp = Biddy_And(r,t);
#ifdef USE_BIDDY
      Biddy_AddTmpFormula(tmp,1);
      Biddy_Clean();
#elif USE_CUDD
      Cudd_Ref(tmp);
      Cudd_RecursiveDeref(manager,t);
      Cudd_RecursiveDeref(manager,r);
#endif
      r = tmp;
    }
  }

  /* RISING DIAGONALS */
  for (i=1; i<=size; i++) {
    for (j=1; j<=size; j++) {
      t = one;
#ifdef USE_BIDDY
#elif USE_CUDD
      Cudd_Ref(t);
#endif
      for (k=1; k<=size; k++) {
        if ((j+k-i >= 1) && (j+k-i <= size) && (k != i)) {
          tmp = Biddy_And(t,Biddy_Not(board[k][j+k-i]));
#ifdef USE_BIDDY
#elif USE_CUDD
          Cudd_Ref(tmp);
          Cudd_RecursiveDeref(manager,t);
#endif
          t = tmp;
        }
      }
      tmp = Biddy_Or(t,Biddy_Not(board[i][j]));
#ifdef USE_BIDDY
#elif USE_CUDD
      Cudd_Ref(tmp);
      Cudd_RecursiveDeref(manager,t);
#endif
      t = tmp;
      tmp = Biddy_And(r,t);
#ifdef USE_BIDDY
      Biddy_AddTmpFormula(tmp,1);
      Biddy_Clean();
#elif USE_CUDD
      Cudd_Ref(tmp);
      Cudd_RecursiveDeref(manager,t);
      Cudd_RecursiveDeref(manager,r);
#endif
      r = tmp;
    }
  }

  /* FALLING DIAGONALS */
  for (i=1; i<=size; i++) {
    for (j=1; j<=size; j++) {
      t = one;
#ifdef USE_BIDDY
#elif USE_CUDD
      Cudd_Ref(t);
#endif
      for (k=1; k<=size; k++) {
        if ((j+i-k >= 1) && (j+i-k <= size) && (k != i)) {
          tmp = Biddy_And(t,Biddy_Not(board[k][j+i-k]));
#ifdef USE_BIDDY
#elif USE_CUDD
          Cudd_Ref(tmp);
          Cudd_RecursiveDeref(manager,t);
#endif
          t = tmp;
        }
      }
      tmp = Biddy_Or(t,Biddy_Not(board[i][j]));
#ifdef USE_BIDDY
#elif USE_CUDD
      Cudd_Ref(tmp);
      Cudd_RecursiveDeref(manager,t);
#endif
      t = tmp;
      tmp = Biddy_And(r,t);
#ifdef USE_BIDDY
      Biddy_AddTmpFormula(tmp,1);
      Biddy_Clean();
#elif USE_CUDD
      Cudd_Ref(tmp);
      Cudd_RecursiveDeref(manager,t);
      Cudd_RecursiveDeref(manager,r);
#endif
      r = tmp;
    }
  }

  /* THERE MUST BE A QUEEN IN EACH ROW */
  for (i=1; i<=size; i++) {
    t = zero;
#ifdef USE_BIDDY
#elif USE_CUDD
    Cudd_Ref(t);
#endif
    for (j=1; j<=size; j++) {
      tmp = Biddy_Or(t,board[i][j]);
#ifdef USE_BIDDY
#elif USE_CUDD
      Cudd_Ref(tmp);
      Cudd_RecursiveDeref(manager,t);
#endif
      t = tmp;
    }
    tmp = Biddy_And(r,t);
#ifdef USE_BIDDY
    Biddy_AddTmpFormula(tmp,1);
    Biddy_Clean();
#elif USE_CUDD
    Cudd_Ref(tmp);
    Cudd_RecursiveDeref(manager,t);
    Cudd_RecursiveDeref(manager,r);
#endif
    r = tmp;
  }

  /* RESULT */
  n = Biddy_VariableNumber(r);
  printf("Resulting function r depends on %u variables.\n",n);
  printf("Resulting function r has %.0f minterms.\n",Biddy_CountMinterm(r,n));
  printf("BDD for resulting function r has %u nodes.\n",Biddy_NodeNumber(r));

  /*
  Biddy_WriteDot("8queens.dot",r,"r");
  */

  /* We have problems with passing stdout, if you compile this file */
  /* with MINGW and using biddy.dll generated with Visual Studio. */
  /* In such cases, please, use Biddy_PrintInfo(NULL) */
  /**/
  Biddy_PrintInfo(stdout);
  /**/

  Biddy_Exit();
}

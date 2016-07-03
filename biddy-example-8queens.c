/* $Revision: 170 $ */
/* $Date: 2016-06-29 23:10:22 +0200 (sre, 29 jun 2016) $ */
/* This file (biddy-example-8queens.c) is a C file */
/* Author: Robert Meolic (robert.meolic@um.si) */
/* This file has been released into the public domain by the author. */

/* This example is compatible with Biddy v1.6 and CUDD v3.0.0 */

/* COMPILE WITH: */
/* gcc -DREPORT -DUNIX -DUSE_BIDDY -O2 -o biddy-example-8queens biddy-example-8queens.c -I. -L./bin -lbiddy -lgmp */
/* gcc -DREPORT -DUSE_CUDD -O2 -o cudd-example-8queens biddy-example-8queens.c -I ../cudd/include/ -L ../cudd/lib/ -lcudd -lm */

/* to enable the 1993 edition of the POSIX.1b standard (IEEE Standard 1003.1b-1993) */
/* #define _POSIX_C_SOURCE (199309L) */

/* default size, this can be overriden via argument */
#define SIZE 9
#define USE_GARBAGE_COLLECTION

/* for BIDDY there are 3 variants possible */
/* NOT_USE_SIFTING, USE_SIFTING, USE_SIFTING_R */
#define NOT_USE_SIFTING

/* there are 5 variants possible */
/* NOT_POSTPONE, POSTPONE_FIFO, POSTPONE_LIFO, POSTPONE_REVERSE_FIFO, POSTPONE_REVERSE_LIFO */
#define NOT_POSTPONE

/* event log (NONE, TIME, SIZE, or RESULT) */
#define EVENTLOG_NONE
#ifdef EVENTLOG_TIME
#define ZF_LOGI(x) {static unsigned int zfstat = 0;printf(x);printf("#%u,%.2f\n",++zfstat,clock()/(1.0*CLOCKS_PER_SEC));}
#endif
#ifdef EVENTLOG_SIZE
#ifdef USE_BIDDY
#define ZF_LOGI(x) {static unsigned int zfstat = 0;printf(x);printf("#%u,%u,%u\n",++zfstat,Biddy_NodeTableNum(),Biddy_NodeTableSize());}
#endif
#ifdef USE_CUDD
#define ZF_LOGI(x) {static unsigned int zfstat = 0;printf(x);printf("#%u,%u,%u\n",++zfstat,Cudd_ReadKeys(manager),Cudd_ReadSlots(manager));}
#endif
#endif
#ifdef EVENTLOG_RESULT
#ifdef USE_BIDDY
#define ZF_LOGI(x) {static unsigned int zfstat = 0;printf(x);printf("#%u,%u\n",++zfstat,Biddy_NodeNumber(r));}
#endif
#ifdef USE_CUDD
#define ZF_LOGI(x) {static unsigned int zfstat = 0;printf(x);printf("#%u,%u\n",++zfstat,Cudd_DagSize(r));}
#endif
#endif
#ifndef ZF_LOGI
#define ZF_LOGI(x)
#endif

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

#include <time.h>

#ifdef USE_CUDD
#define Biddy_GetConstantZero() Cudd_ReadLogicZero(manager)
#define Biddy_GetConstantOne() Cudd_ReadOne(manager)
#define Biddy_AddVariable() Cudd_bddNewVar(manager)
#define Biddy_Not(f) Cudd_Not(f)
/* #define Biddy_And(f,g) Cudd_bddAnd(manager,f,g) */
/* #define Biddy_Or(f,g) Cudd_bddOr(manager,f,g) */
#define Biddy_And(f,g) Cudd_bddIte(manager,f,g,zero)
#define Biddy_Or(f,g) Cudd_bddIte(manager,f,one,g)
#define Biddy_VariableNumber(f) Cudd_SupportSize(manager,f)
#define Biddy_CountMinterm(f,n) Cudd_CountMinterm(manager,f,n)
#define Biddy_NodeNumber(f) Cudd_DagSize(f)
#define Biddy_PrintInfo(s) Cudd_PrintInfo(manager,s)
#define Biddy_NodeTableNum() Cudd_ReadKeys(manager)
#define Biddy_NodeTableSize() Cudd_ReadSlots(manager)
#define Biddy_Exit()
#endif

int main(int argc, char** argv) {
  unsigned int i,j,k,n,size,seq,seq1;
  Biddy_Edge **board;
  Biddy_Edge zero,one,tmp;
  Biddy_Edge t,r;
  Biddy_Edge *fifo1,*LP1;
  Biddy_Edge *fifo2,*LP2;

  clock_t starttime;
  /* struct timespec starttimex, stoptimex; */

  starttime = clock();
  /* if (clock_gettime(CLOCK_PROCESS_CPUTIME_ID,&starttimex) == -1) printf("ERROR WITH clock_gettime!\n"); */

  if (argc == 2) {
    sscanf(argv[1],"%u",&size);
  } else {
    size = SIZE;
  }

  if ((size < 2) || (size > 12)) {
    printf("WRONG SIZE!\n");
    exit(1);
  }

  setbuf(stdout, NULL);

  LP1 = fifo1 = (Biddy_Edge *) malloc((4 * size * size + size) * sizeof(Biddy_Edge));
  LP2 = fifo2 = (Biddy_Edge *) malloc((4 * size * size + size) * sizeof(Biddy_Edge));
  LP1--;

#ifdef USE_BIDDY
  seq = 0;
#endif

  board = (Biddy_Edge **) malloc((size+1)*sizeof(Biddy_Edge *));
  for (i=0; i<=size; i++) {
    board[i] = (Biddy_Edge *) malloc((size+1)*sizeof(Biddy_Edge));
  }

#ifdef USE_BIDDY
  /* Biddy v1.6 does not support runtime changes of internal parameters */
  /* There is only one unique table in Biddy */
  /* There are three caches in Biddy */
  /* New nodes are added in blocks */
  /* The max number of variables is hardcoded in biddyInt. h */
  /* biddyVariableTable.size = BIDDYVARMAX = 2048 */
  /* The following constants are hardcoded in biddyMain.c */
  /* biddyNodeTable.size = BIG_TABLE = 1048575 */
  /* biddyIteCache.size = MEDIUM_TABLE = 262143 */
  /* biddyEACache.size = SMALL_TABLE = 65535 */
  /* biddyRCCache.size = SMALL_TABLE = 65535 */
  /* biddyBlockSize = 524288 */
  /* DEFAULT INIT CALL: Biddy_Init() */
  Biddy_Init();
#endif

#ifdef USE_CUDD
  /* In CUDD each variable has its own subtable in the unique table */
  /* There is only one cache in CUDD */
  /* Subtables grow over the time, you can set limit for fast unique table growth */
  /* Cudd_SetLooseUpTo(manager,1048576) */
  /* Cache can grow over the time, you can set the max size */
  /* Cudd_SetMaxCacheHard(manager,262144) */
  /* These two constants are hardcoded in v3.0.0 */
  /* CUDD_UNIQUE_SLOTS = 256 (default initial size of each subtable) */
  /* CUDD_CACHE_SLOTS = 262144 (default initial size of cache table) */
  /* DEFAULT INIT CALL: Cudd_Init(0,0,CUDD_UNIQUE_SLOTS,CUDD_CACHE_SLOTS,0) */
  manager = Cudd_Init(0,0,CUDD_UNIQUE_SLOTS,CUDD_CACHE_SLOTS,0);
  Cudd_SetMaxCacheHard(manager,262144);
#ifndef USE_GARBAGE_COLLECTION
  Cudd_DisableGarbageCollection(manager);
#endif
#endif

  zero = Biddy_GetConstantZero();
#ifdef USE_CUDD
  Cudd_Ref(zero);
#endif

  one = Biddy_GetConstantOne();
#ifdef USE_CUDD
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
#ifdef USE_CUDD
  Cudd_Ref(r);
#endif

  ZF_LOGI("INIT");

  /* ROWS */
#ifdef REPORT
  printf("\nSTARTING ROWS\n");
#endif
  for (i=1; i<=size; i++) {
#ifdef REPORT
    printf("(%d/%d)",i,size);
#endif
    for (j=1; j<=size; j++) {
      t = one;
#ifdef USE_CUDD
      Cudd_Ref(t);
#endif
      for (k=1; k<=size; k++) {
        if (k != j) {
          ZF_LOGI("R_AND");
          tmp = Biddy_And(t,Biddy_Not(board[i][k]));
#ifdef USE_CUDD
          Cudd_Ref(tmp);
          Cudd_RecursiveDeref(manager,t);
#endif
          t = tmp;
        }
      }
      ZF_LOGI("R_OR");
      tmp = Biddy_Or(t,Biddy_Not(board[i][j]));
#ifdef USE_CUDD
      Cudd_Ref(tmp);
      Cudd_RecursiveDeref(manager,t);
#endif
      t = tmp;

#ifdef NOT_POSTPONE
      ZF_LOGI("R_AND");
      tmp = Biddy_And(r,t);
#ifdef USE_BIDDY
#ifdef USE_GARBAGE_COLLECTION
       Biddy_AddTmpFormula(tmp,1);
       Biddy_Clean();
#endif
#endif
#ifdef USE_CUDD
      Cudd_Ref(tmp);
      Cudd_RecursiveDeref(manager,t);
      Cudd_RecursiveDeref(manager,r);
#endif
      r = tmp;
#else
      *(++LP1) = t;
#endif

      ZF_LOGI("R_MEM");

#ifdef USE_BIDDY
#ifdef USE_GARBAGE_COLLECTION
#ifdef POSTPONE_FIFO
      seq++;
      Biddy_AddTmpFormula(t,(seq-1)/2);
#endif
#ifdef POSTPONE_REVERSE_FIFO
      seq++;
      Biddy_AddTmpFormula(t,(4 * size * size + size - seq)/2);
#endif
#ifdef POSTPONE_LIFO
      seq++;
      if (seq == 1) {
        Biddy_AddTmpFormula(t,seq-1);
      } else {
        Biddy_AddTmpFormula(t,seq-2);
      }
#endif
#ifdef POSTPONE_REVERSE_LIFO
      seq++;
      Biddy_AddTmpFormula(t,(4 * size * size + size) - seq - 1);
#endif
#endif
#endif

    }

/*
#ifdef NOT_POSTPONE
#ifdef USE_BIDDY
#ifdef USE_GARBAGE_COLLECTION
      Biddy_AddTmpFormula(r,1);
      Biddy_Clean();
#endif
#endif
#endif
*/

  }

  ZF_LOGI("R_SIFT");

  /* SIFTING AFTER EACH MAJOR PART - IT IS NOT VERY EFFECTIVE FOR THIS PROBLEM */
#ifdef USE_BIDDY
#ifdef NOT_POSTPONE
#ifdef USE_SIFTING
  Biddy_AddTmpFormula(r,1);
  Biddy_PurgeAndReorder(NULL,FALSE);
  Biddy_Clean();
#endif
#ifdef USE_SIFTING_R
  Biddy_AddTmpFormula(r,1);
  Biddy_PurgeAndReorder(r,FALSE);
  Biddy_Clean();
#endif
#endif
#endif

  /* COLUMNS */
#ifdef REPORT
  printf("\nSTARTING COLUMNS\n");
#endif
  for (i=1; i<=size; i++) {
#ifdef REPORT
    printf("(%d/%d)",i,size);
#endif
    for (j=1; j<=size; j++) {
      t = one;
#ifdef USE_CUDD
      Cudd_Ref(t);
#endif
      for (k=1; k<=size; k++) {
        if (k != i) {
          ZF_LOGI("C_AND");
          tmp = Biddy_And(t,Biddy_Not(board[k][j]));
#ifdef USE_CUDD
          Cudd_Ref(tmp);
          Cudd_RecursiveDeref(manager,t);
#endif
          t = tmp;
        }
      }
      ZF_LOGI("C_OR");
      tmp = Biddy_Or(t,Biddy_Not(board[i][j]));
#ifdef USE_CUDD
      Cudd_Ref(tmp);
      Cudd_RecursiveDeref(manager,t);
#endif
      t = tmp;

#ifdef NOT_POSTPONE
      ZF_LOGI("C_AND");
      tmp = Biddy_And(r,t);
#ifdef USE_BIDDY
#ifdef USE_GARBAGE_COLLECTION
       Biddy_AddTmpFormula(tmp,1);
       Biddy_Clean();
#endif
#endif
#ifdef USE_CUDD
      Cudd_Ref(tmp);
      Cudd_RecursiveDeref(manager,t);
      Cudd_RecursiveDeref(manager,r);
#endif
      r = tmp;
#else
      *(++LP1) = t;
#endif

      ZF_LOGI("C_MEM");

#ifdef USE_BIDDY
#ifdef USE_GARBAGE_COLLECTION
#ifdef POSTPONE_FIFO
      seq++;
      Biddy_AddTmpFormula(t,(seq-1)/2);
#endif
#ifdef POSTPONE_REVERSE_FIFO
      seq++;
      Biddy_AddTmpFormula(t,(4 * size * size + size - seq)/2);
#endif
#ifdef POSTPONE_LIFO
      seq++;
      Biddy_AddTmpFormula(t,seq-2);
#endif
#ifdef POSTPONE_REVERSE_LIFO
      seq++;
      Biddy_AddTmpFormula(t,(4 * size * size + size) - seq - 1);
#endif
#endif
#endif

    }

/*
#ifdef NOT_POSTPONE
#ifdef USE_BIDDY
#ifdef USE_GARBAGE_COLLECTION
      Biddy_AddTmpFormula(r,1);
      Biddy_Clean();
#endif
#endif
#endif
*/

  }

  ZF_LOGI("C_SIFT");

  /* SIFTING AFTER EACH MAJOR PART - IT IS NOT VERY EFFECTIVE FOR THIS PROBLEM */
#ifdef USE_BIDDY
#ifdef NOT_POSTPONE
#ifdef USE_SIFTING
  Biddy_AddTmpFormula(r,1);
  Biddy_PurgeAndReorder(NULL,FALSE);
  Biddy_Clean();
#endif
#ifdef USE_SIFTING_R
  Biddy_AddTmpFormula(r,1);
  Biddy_PurgeAndReorder(r,FALSE);
  Biddy_Clean();
#endif
#endif
#endif

  /* RISING DIAGONALS */
#ifdef REPORT
  printf("\nSTARTING RISING DIAGONALS\n");
#endif
  for (i=1; i<=size; i++) {
#ifdef REPORT
    printf("(%d/%d)",i,size);
#endif
    for (j=1; j<=size; j++) {
      t = one;
#ifdef USE_CUDD
      Cudd_Ref(t);
#endif
      for (k=1; k<=size; k++) {
        if ((j+k-i >= 1) && (j+k-i <= size) && (k != i)) {
          ZF_LOGI("RD_AND");
          tmp = Biddy_And(t,Biddy_Not(board[k][j+k-i]));
#ifdef USE_CUDD
          Cudd_Ref(tmp);
          Cudd_RecursiveDeref(manager,t);
#endif
          t = tmp;
        }
      }
      ZF_LOGI("RD_OR");
      tmp = Biddy_Or(t,Biddy_Not(board[i][j]));
#ifdef USE_CUDD
      Cudd_Ref(tmp);
      Cudd_RecursiveDeref(manager,t);
#endif
      t = tmp;

#ifdef NOT_POSTPONE
      ZF_LOGI("RD_AND");
      tmp = Biddy_And(r,t);
#ifdef USE_BIDDY
#ifdef USE_GARBAGE_COLLECTION
       Biddy_AddTmpFormula(tmp,1);
       Biddy_Clean();
#endif
#endif
#ifdef USE_CUDD
      Cudd_Ref(tmp);
      Cudd_RecursiveDeref(manager,t);
      Cudd_RecursiveDeref(manager,r);
#endif
      r = tmp;
#else
      *(++LP1) = t;
#endif

      ZF_LOGI("RD_MEM");

#ifdef USE_BIDDY
#ifdef USE_GARBAGE_COLLECTION
#ifdef POSTPONE_FIFO
      seq++;
      Biddy_AddTmpFormula(t,(seq-1)/2);
#endif
#ifdef POSTPONE_REVERSE_FIFO
      seq++;
      Biddy_AddTmpFormula(t,(4 * size * size + size - seq)/2);
#endif
#ifdef POSTPONE_LIFO
      seq++;
      Biddy_AddTmpFormula(t,seq-2);
#endif
#ifdef POSTPONE_REVERSE_LIFO
      seq++;
      Biddy_AddTmpFormula(t,(4 * size * size + size) - seq - 1);
#endif
#endif
#endif

    }

/*
#ifdef NOT_POSTPONE
#ifdef USE_BIDDY
#ifdef USE_GARBAGE_COLLECTION
      Biddy_AddTmpFormula(r,1);
      Biddy_Clean();
#endif
#endif
#endif
*/

  }

  ZF_LOGI("RD_SIFT");

  /* SIFTING AFTER EACH MAJOR PART - IT IS NOT VERY EFFECTIVE FOR THIS PROBLEM */
#ifdef USE_BIDDY
#ifdef NOT_POSTPONE
#ifdef USE_SIFTING
  Biddy_AddTmpFormula(r,1);
  Biddy_PurgeAndReorder(NULL,FALSE);
  Biddy_Clean();
#endif
#ifdef USE_SIFTING_R
  Biddy_AddTmpFormula(r,1);
  Biddy_PurgeAndReorder(r,FALSE);
  Biddy_Clean();
#endif
#endif
#endif

  /* FALLING DIAGONALS */
#ifdef REPORT
  printf("\nSTARTING FALLING DIAGONALS\n");
#endif
  for (i=1; i<=size; i++) {
#ifdef REPORT
    printf("(%d/%d)",i,size);
#endif
    for (j=1; j<=size; j++) {
      t = one;
#ifdef USE_CUDD
      Cudd_Ref(t);
#endif
      for (k=1; k<=size; k++) {
        if ((j+i-k >= 1) && (j+i-k <= size) && (k != i)) {
          ZF_LOGI("FD_AND");
          tmp = Biddy_And(t,Biddy_Not(board[k][j+i-k]));
#ifdef USE_CUDD
          Cudd_Ref(tmp);
          Cudd_RecursiveDeref(manager,t);
#endif
          t = tmp;
        }
      }
      ZF_LOGI("FD_OR");
      tmp = Biddy_Or(t,Biddy_Not(board[i][j]));
#ifdef USE_CUDD
      Cudd_Ref(tmp);
      Cudd_RecursiveDeref(manager,t);
#endif
      t = tmp;

#ifdef NOT_POSTPONE
      ZF_LOGI("FD_AND");
      tmp = Biddy_And(r,t);
#ifdef USE_BIDDY
#ifdef USE_GARBAGE_COLLECTION
       Biddy_AddTmpFormula(tmp,1);
       Biddy_Clean();
#endif
#endif
#ifdef USE_CUDD
      Cudd_Ref(tmp);
      Cudd_RecursiveDeref(manager,t);
      Cudd_RecursiveDeref(manager,r);
#endif
      r = tmp;
#else
      *(++LP1) = t;
#endif

      ZF_LOGI("FD_MEM");

#ifdef USE_BIDDY
#ifdef USE_GARBAGE_COLLECTION
#ifdef POSTPONE_FIFO
      seq++;
      Biddy_AddTmpFormula(t,(seq-1)/2);
#endif
#ifdef POSTPONE_REVERSE_FIFO
      seq++;
      Biddy_AddTmpFormula(t,(4 * size * size + size - seq)/2);
#endif
#ifdef POSTPONE_LIFO
      seq++;
      Biddy_AddTmpFormula(t,seq-2);
#endif
#ifdef POSTPONE_REVERSE_LIFO
      seq++;
      Biddy_AddTmpFormula(t,(4 * size * size + size) - seq - 1);
#endif
#endif
#endif

    }

/*
#ifdef NOT_POSTPONE
#ifdef USE_BIDDY
#ifdef USE_GARBAGE_COLLECTION
      Biddy_AddTmpFormula(r,1);
      Biddy_Clean();
#endif
#endif
#endif
*/

  }

  ZF_LOGI("FD_SIFT");

  /* SIFTING AFTER EACH MAJOR PART - IT IS NOT VERY EFFECTIVE FOR THIS PROBLEM */
#ifdef USE_BIDDY
#ifdef NOT_POSTPONE
#ifdef USE_SIFTING
  Biddy_AddTmpFormula(r,1);
  Biddy_PurgeAndReorder(NULL,FALSE);
  Biddy_Clean();
#endif
#ifdef USE_SIFTING_R
  Biddy_AddTmpFormula(r,1);
  Biddy_PurgeAndReorder(r,FALSE);
  Biddy_Clean();
#endif
#endif
#endif

  /* THERE MUST BE A QUEEN IN EACH ROW */
#ifdef REPORT
  printf("\nSTARTING FINAL CALCULATONS\n");
#endif
  for (i=1; i<=size; i++) {
#ifdef REPORT
    printf("(%d/%d)",i,size);
#endif
    t = zero;
#ifdef USE_CUDD
    Cudd_Ref(t);
#endif
    for (j=1; j<=size; j++) {
      ZF_LOGI("F_OR");
      tmp = Biddy_Or(t,board[i][j]);
#ifdef USE_CUDD
      Cudd_Ref(tmp);
      Cudd_RecursiveDeref(manager,t);
#endif
      t = tmp;
    }

#ifdef NOT_POSTPONE
    ZF_LOGI("F_AND");
    tmp = Biddy_And(r,t);
#ifdef USE_BIDDY
#ifdef USE_GARBAGE_COLLECTION
    Biddy_AddTmpFormula(tmp,1);
    Biddy_Clean();
#endif
#endif
#ifdef USE_CUDD
    Cudd_Ref(tmp);
    Cudd_RecursiveDeref(manager,t);
    Cudd_RecursiveDeref(manager,r);
#endif
    r = tmp;
#else
    *(++LP1) = t;
#endif

    ZF_LOGI("F_MEM");

#ifdef USE_BIDDY
#ifdef USE_GARBAGE_COLLECTION
#ifdef POSTPONE_FIFO
    seq++;
    Biddy_AddTmpFormula(t,(seq-1)/2);
    /* printf("(add %d)",(seq-1)/2); */
#endif
#ifdef POSTPONE_REVERSE_FIFO
    seq++;
    Biddy_AddTmpFormula(t,(4 * size * size + size - seq)/2);
    /* printf("(add %d)",(4 * size * size + size - seq)/2); */
#endif
#ifdef POSTPONE_LIFO
    seq++;
    Biddy_AddTmpFormula(t,seq-2);
    /* printf("(add %d)",seq-2); */
#endif
#ifdef POSTPONE_REVERSE_LIFO
    seq++;
    if (seq == (4 * size *size + size)) {
      Biddy_AddTmpFormula(t,4 * size * size + size - seq);
      /* printf("(add %d)",4 * size * size + size - seq); */
    } else {
      Biddy_AddTmpFormula(t,4 * size * size + size - seq - 1);
      /* printf("(add %d)",4 * size * size + size - seq - 1); */
    }
#endif
#endif
#endif

  }

  ZF_LOGI("F_SIFT");

  /* SIFTING AFTER EACH MAJOR PART - IT IS NOT VERY EFFECTIVE FOR THIS PROBLEM */
#ifdef USE_BIDDY
#ifdef NOT_POSTPONE
#ifdef USE_SIFTING
  Biddy_AddTmpFormula(r,1);
  Biddy_PurgeAndReorder(NULL,FALSE);
  Biddy_Clean();
#endif
#ifdef USE_SIFTING_R
  Biddy_AddTmpFormula(r,1);
  Biddy_PurgeAndReorder(r,FALSE);
  Biddy_Clean();
#endif
#endif
#endif

  ZF_LOGI("P_MEM");

/* POSTPONED ELEMENTS WILL BE CALCULATED IN REVERSED ORDER */
#if defined(POSTPONE_FIFO) || defined(POSTPONE_LIFO)
  LP1++;
  LP2 = fifo2;
  LP2--;
  while(LP1 != fifo1) {
    *(++LP2) = *(--LP1);
  }
  fifo1 = fifo2;
  fifo2 = LP1;
  LP1 = LP2;
#endif

#ifdef NOT_POSTPONE
#ifdef REPORT
  printf("\nAnd OPERATIONS ARE NOT POSTPONED\n");
#endif
#endif

/* POSTPONE And OPERATIONS USING FIFO */
#if defined(POSTPONE_FIFO) || defined(POSTPONE_REVERSE_FIFO)
#ifdef REPORT
  printf("\nPOSTPONED And OPERATIONS USING FIFO\n");
#endif
#ifdef USE_CUDD
  Cudd_RecursiveDeref(manager,r);
#endif
  i = 0;
  do {
/*
    printf("(%d)",++i);
*/
    LP1++;
    LP2 = fifo2;
    LP2--;
    seq1 = (seq+1)/2;
    seq = 0;
    while(LP1 != fifo1) {
      ZF_LOGI("P_MEM");
      seq++;
      r = *(--LP1);
      if (LP1 == fifo1) {
        *(++LP2) = r;
#ifdef USE_BIDDY
#ifdef USE_GARBAGE_COLLECTION
        Biddy_AddTmpFormula(r,seq1+(seq-1)/2);
        Biddy_Clean();
        seq1--;
#endif
#endif
      } else {
        t = *(--LP1);
        ZF_LOGI("P_AND");
        tmp = Biddy_And(r,t);
        *(++LP2) = tmp;
#ifdef USE_BIDDY
#ifdef USE_GARBAGE_COLLECTION
        Biddy_AddTmpFormula(tmp,seq1+(seq-1)/2);
        Biddy_Clean();
        seq1--;
#endif
#endif
#ifdef USE_CUDD
        Cudd_Ref(tmp);
        Cudd_RecursiveDeref(manager,r);
        Cudd_RecursiveDeref(manager,t);
#endif
      }
    }
    fifo1 = fifo2;
    fifo2 = LP1;
    LP1 = LP2;

    /* TO BE CONSISTENT YOU HAVE TO REVERSE ORDERING OF ELEMENTS */
    LP1++;
    LP2 = fifo2;
    LP2--;
    while(LP1 != fifo1) {
      *(++LP2) = *(--LP1);
    }
    fifo1 = fifo2;
    fifo2 = LP1;
    LP1 = LP2;

  } while (LP1 != fifo1);
  r = *(LP1);
#endif

/* POSTPONE And OPERATIONS USING LIFO */
#if defined(POSTPONE_LIFO) || defined(POSTPONE_REVERSE_LIFO)
#ifdef REPORT
  printf("\nPOSTPONED And OPERATIONS USING LIFO\n");
#endif
#ifdef USE_CUDD
  Cudd_RecursiveDeref(manager,r);
#endif
  i = 0;
  while(LP1 != fifo1) {
/*
    printf("(%d)",++i);
*/
    r = *(LP1--);
    t = *(LP1--);
    ZF_LOGI("P_AND");
    tmp = Biddy_And(r,t);
    *(++LP1) = tmp;
#ifdef USE_BIDDY
#ifdef USE_GARBAGE_COLLECTION
    Biddy_AddTmpFormula(tmp,1);
    Biddy_Clean();
#endif
#endif
#ifdef USE_CUDD
    Cudd_Ref(tmp);
    Cudd_RecursiveDeref(manager,r);
    Cudd_RecursiveDeref(manager,t);
#endif
  }
  r = *(LP1);
#endif

  free(fifo1);
  free(fifo2);

  /* RESULT */
#ifdef REPORT
  printf("\n");
  n = Biddy_VariableNumber(r);
  printf("Resulting function r depends on %u variables.\n",n);
  printf("Resulting function r has %.0f minterms.\n",Biddy_CountMinterm(r,n));
  printf("BDD for resulting function r has %u nodes.\n",Biddy_NodeNumber(r));
#endif

  /* DUMP RESULT USING GRAPHVIZ/DOT */
/*
#ifdef USE_BIDDY
  Biddy_WriteDot("8queens-biddy.dot",r,"r");
#elif USE_CUDD
  {
  FILE * fp;
  fp = fopen("8queens-cudd.dot","w");
  Cudd_DumpDot(manager,1,&r,NULL,NULL,fp);
  fclose(fp);
  }
#endif
*/

  /* We have problems with passing stdout in the case you compile this file */
  /* with MINGW and use biddy.dll generated with Visual Studio. */
  /* In such cases, please, use Biddy_PrintInfo(NULL) */
#ifdef REPORT
  Biddy_PrintInfo(stdout);
#endif

  /* SIFTING ON THE RESULT - THIS IS NOT VERY EFFECTIVE FOR THIS PROBLEM */
  /*
#ifdef USE_BIDDY
  Biddy_AddTmpFormula(r,1);
  Biddy_PurgeAndReorder(NULL,FALSE);
  Biddy_Clean();
#endif
#ifdef USE_CUDD
  Cudd_ReduceHeap(manager,CUDD_REORDER_SIFT,0);
#endif
#ifdef REPORT
  n = Biddy_VariableNumber(r);
  printf("(AFTER SIFTING) Resulting function r depends on %u variables.\n",n);
  printf("(AFTER SIFTING) Resulting function r has %.0f minterms.\n",Biddy_CountMinterm(r,n));
  printf("(AFTER SIFTING) BDD for resulting function r has %u nodes.\n",Biddy_NodeNumber(r));
#endif
  */

  ZF_LOGI("EXIT");

  fprintf(stderr,"clock() TIME = %.2f\n",(clock()-starttime)/(1.0*CLOCKS_PER_SEC));

  /*
  if (clock_gettime(CLOCK_PROCESS_CPUTIME_ID,&stoptimex) == -1) printf("ERROR WITH clock_gettime!\n");
  fprintf(stderr,"clock_gettime() TIME = %.2f\n",(stoptimex.tv_sec-starttimex.tv_sec)+(stoptimex.tv_nsec-starttimex.tv_nsec)/1000000000.0);
  */
            
  /* WAIT FOR USER */
#ifdef REPORT
  printf("\nCALCULATION FINISHED\n");
#endif

  /* EXIT */
#ifdef USE_BIDDY
  Biddy_Exit();
#endif
#ifdef USE_CUDD
  Cudd_RecursiveDeref(manager,zero);
  Cudd_RecursiveDeref(manager,one);
  Cudd_RecursiveDeref(manager,r);
#ifdef REPORT
  printf("CUDD: nodes with non-zero reference counts: %d\n",Cudd_CheckZeroRef(manager));
#endif
  Cudd_Quit(manager);
#endif

}

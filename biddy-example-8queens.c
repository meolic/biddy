/* $Revision: 253 $ */
/* $Date: 2017-03-20 09:03:47 +0100 (pon, 20 mar 2017) $ */
/* This file (biddy-example-8queens.c) is a C file */
/* Author: Robert Meolic (robert.meolic@um.si) */
/* This file has been released into the public domain by the author. */

/* This example is compatible with Biddy v1.7 and CUDD v3.0.0 */

/* COMPILE WITH: */
/* gcc -DREPORT -DEVENTLOG_NONE -DUSE_BIDDY -DUNIX -O2 -o biddy-example-8queens biddy-example-8queens.c -I. -L./bin -lbiddy -lgmp */
/* gcc -DREPORT -DEVENTLOG_NONE -DUSE_CUDD -O2 -o cudd-example-8queens biddy-example-8queens.c -I ../cudd/include/ -L ../cudd/lib/ -lcudd -lm */

/* to enable the 1993 edition of the POSIX.1b standard (IEEE Standard 1003.1b-1993) */
/* #define _POSIX_C_SOURCE (199309L) */

/* default size, this can be overriden via argument */
#define SIZE 8
#define USE_GARBAGE_COLLECTION

/* for BIDDY there are 3 variants possible: */
/* NOT_USE_SIFTING, USE_SIFTING, USE_SIFTING_R */
#define NOT_USE_SIFTING

/* there are 5 variants possible */
/* NOT_POSTPONE, POSTPONE_FIFO, POSTPONE_LIFO, POSTPONE_REVERSE_FIFO, POSTPONE_REVERSE_LIFO */
#define NOT_POSTPONE

/* event log (EVENTLOG_TIME, EVENTLOG_SIZE, or EVENTLOG_RESULT) */
#ifdef EVENTLOG_TIME
#  define ZF_LOGI(x) {static unsigned int zfstat = 0;printf(x);printf("#%u,%.2f\n",++zfstat,clock()/(1.0*CLOCKS_PER_SEC));}
#endif
#ifdef EVENTLOG_SIZE
#  ifdef USE_BIDDY
#    define ZF_LOGI(x) {static unsigned int zfstat = 0;printf(x);printf("#%u,%u,%u\n",++zfstat,Biddy_NodeTableNum(),Biddy_NodeTableSize());}
#  endif
#  ifdef USE_CUDD
#    define ZF_LOGI(x) {static unsigned int zfstat = 0;printf(x);printf("#%u,%u,%u\n",++zfstat,Cudd_ReadKeys(manager),Cudd_ReadSlots(manager));}
#  endif
#endif
#ifdef EVENTLOG_RESULT
#  ifdef USE_BIDDY
#    define ZF_LOGI(x) {static unsigned int zfstat = 0;printf(x);printf("#%u,%u\n",++zfstat,Biddy_NodeNumber(r));}
#  endif
#  ifdef USE_CUDD
#    define ZF_LOGI(x) {static unsigned int zfstat = 0;printf(x);printf("#%u,%u\n",++zfstat,Cudd_DagSize(r));}
#  endif
#endif
#ifndef ZF_LOGI
#  define ZF_LOGI(x)
#endif

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
DdManager *manager;
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
#define Biddy_DependentVariableNumber(f) Cudd_SupportSize(manager,f)
#define Biddy_CountMinterm(f,n) Cudd_CountMinterm(manager,f,n)
#define Biddy_DensityBDD(f,n) Cudd_Density(manager,f,n)
#define Biddy_NodeNumber(f) Cudd_DagSize(f)
#define Biddy_PrintInfo(s) Cudd_PrintInfo(manager,s)
#define Biddy_NodeTableNum() Cudd_ReadKeys(manager)
#define Biddy_NodeTableSize() Cudd_ReadSlots(manager)
#define Biddy_ReadMemoryInUse() Cudd_ReadMemoryInUse(manager)
#endif

void checkOutOfLimits(unsigned int size, clock_t elapsedtime) {
#ifdef USE_BIDDY
  if ((size == 9) && (Biddy_ReadMemoryInUse() > 120000000)) {
    fprintf(stderr,"9, -1, -1, -1\n");
    exit(1);
  }
  if ((size == 10) && (Biddy_NodeTableSize() > 4194304)) {
    fprintf(stderr,"10, -1, -1, -1\n");
    exit(1);
  }
  if ((size == 10) && (Biddy_ReadMemoryInUse() > 300000000)) {
    fprintf(stderr,"10, -1, -1, -1\n");
    exit(1);
  }
/*
  if ((size == 10) && ((clock()-elapsedtime)/(1.0*CLOCKS_PER_SEC) > 120.0)) {
    fprintf(stderr,"10, -1, -1, -1\n");
    exit(1);
  }
*/
#endif
}

int main(int argc, char** argv) {
  unsigned int i,j,k,n,size,seq,seq1;
  Biddy_Edge **board;
  Biddy_Edge tmp;
  Biddy_Edge t,r;
  Biddy_Edge *fifo1,*LP1;
  Biddy_Edge *fifo2,*LP2;

  clock_t elapsedtime;
  /* struct timespec elapsedtimex, stoptimex; */

  elapsedtime = clock();
  /* if (clock_gettime(CLOCK_PROCESS_CPUTIME_ID,&elapsedtimex) == -1) printf("ERROR WITH clock_gettime!\n"); */

  if (argc > 1) {
    sscanf(argv[1],"%u",&size);
  } else {
    size = SIZE;
  }

  if ((size < 2) || (size > 15)) {
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
  /* DEFAULT INIT CALL: Biddy_InitAnonymous(BIDDYTYPEOBDD) */
  Biddy_InitAnonymous(BIDDYTYPEOBDD);
  if (argc == 2) {
    Biddy_SetManagerParameters(-1,-1,-1,-1,-1,-1,-1,-1,-1,-1);
  }
  else if (argc == 8) {
    unsigned int gcr,gcrF,gcrX;
    unsigned int rr,rrF,rrX;
    sscanf(argv[2],"%u",&gcr);
    sscanf(argv[3],"%u",&gcrF);
    sscanf(argv[4],"%u",&gcrX);
    sscanf(argv[5],"%u",&rr);
    sscanf(argv[6],"%u",&rrF);
    sscanf(argv[7],"%u",&rrX);
    Biddy_SetManagerParameters(gcr/100.0,gcrF/100.0,gcrX/100.0,rr/100.0,rrF/100.0,rrX/100.0,-1,-1,-1,-1);
  }
  else if (argc == 12) {
    unsigned int gcr,gcrF,gcrX;
    unsigned int rr,rrF,rrX;
    unsigned int st;
    unsigned int fst;
    unsigned int cst;
    unsigned int fcst;
    sscanf(argv[2],"%u",&gcr);
    sscanf(argv[3],"%u",&gcrF);
    sscanf(argv[4],"%u",&gcrX);
    sscanf(argv[5],"%u",&rr);
    sscanf(argv[6],"%u",&rrF);
    sscanf(argv[7],"%u",&rrX);
    sscanf(argv[8],"%u",&st);
    sscanf(argv[9],"%u",&fst);
    sscanf(argv[10],"%u",&cst);
    sscanf(argv[11],"%u",&fcst);
    Biddy_SetManagerParameters(gcr/100.0,gcrF/100.0,gcrX/100.0,rr/100.0,rrF/100.0,rrX/100.0,st/100.0,fst/100.0,cst/100.0,fcst/100.0);
  }
  else {
    printf("Wrong number of parameters!\n");
    printf("USAGE (short): biddy-example-8queens SIZE GCR GCRF GCRX RR RRF RRX\n");
    printf("USAGE (long): biddy-example-8queens SIZE GCR GCRF GCRX RR RRF RRX ST FST CST FCST\n");
    printf("All parameters should be integer numbers >= 0");
    exit(1);
  }
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

  for (i=1; i<=size; i++) {
    board[0][i] = board[i][0] = NULL;
  }

  for (i=1; i<=size; i++) {
    for (j=1; j<=size; j++) {
      board[i][j] = Biddy_AddVariable();
    }
  }

  /* for ZBDDs, by adding new variables all the existing ones are changed */
  /* after adding the last variable we have to refresh edges in v1 and v2 */
  /* NOTE: user variables in Biddy start with index 1 */
#ifdef USE_BIDDY
  if (Biddy_GetManagerType() == BIDDYTYPEZBDD) {
    for (i=1; i<=size; i++) {
      for (j=1; j<=size; j++) {
        board[i][j] = Biddy_GetVariableEdge((i-1)*size+(j-1)+1);
      }
    }
  }
#endif
  
  /* Alternatively, Biddy allows a solution with named variables */
  /*
  {
  Biddy_String name;
  name = malloc(7);
  for (i=1; i<=size; i++) {
    for (j=1; j<=size; j++) {
      sprintf(name,"r%uc%u",i,j);
      board[i][j] = Biddy_AddVariableByName(name);
    }
  }
  if (Biddy_GetManagerType() == BIDDYTYPEZBDD) {
    for (i=1; i<=size; i++) {
      for (j=1; j<=size; j++) {
        sprintf(name,"r%uc%u",i,j);
        board[i][j] = Biddy_GetVariableEdge(Biddy_GetVariable(name));
      }
    }
  }
  free(name);
  }
  */

  r = Biddy_GetConstantOne();
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
      t = Biddy_GetConstantOne();
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
#ifdef NOT_POSTPONE
#ifdef USE_BIDDY
#ifdef USE_GARBAGE_COLLECTION
      Biddy_AddTmpFormula(r,1);
      Biddy_AddTmpFormula(tmp,1);
      Biddy_Clean();
#endif
#endif
#endif
#ifdef USE_CUDD
      Cudd_Ref(tmp);
      Cudd_RecursiveDeref(manager,t);
#endif
      t = tmp;

  /* FREQUENT GC CALLS SEEMS TO BE BENEFICIAL FOR THIS EXAMPLE */
#ifdef USE_BIDDY_X
  Biddy_AutoGC();
#endif

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
  }

  /* EXPLICIT CALL TO GC AFTER EACH MAJOR PART - IMPLEMENTED FOR BIDDY, ONLY */
#ifdef USE_BIDDY_X
  Biddy_AutoGC();
#endif

  /* PROFILING */
  /*
  checkOutOfLimits(size,elapsedtime);
  */

  /* SIFTING AFTER EACH MAJOR PART - IMPLEMENTED FOR BIDDY, ONLY */
  /* IT IS NOT VERY EFFECTIVE FOR THIS PROBLEM */
#ifdef USE_BIDDY
#ifdef NOT_POSTPONE
#ifdef USE_SIFTING
  ZF_LOGI("R_SIFT");
  Biddy_Sifting(NULL,FALSE);
#endif
#ifdef USE_SIFTING_R
  ZF_LOGI("R_SIFT_R");
  Biddy_Sifting(r,FALSE);
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
      t = Biddy_GetConstantOne();
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
#ifdef NOT_POSTPONE
#ifdef USE_BIDDY
#ifdef USE_GARBAGE_COLLECTION
      Biddy_AddTmpFormula(r,1);
      Biddy_AddTmpFormula(tmp,1);
      Biddy_Clean();
#endif
#endif
#endif
#ifdef USE_CUDD
      Cudd_Ref(tmp);
      Cudd_RecursiveDeref(manager,t);
#endif
      t = tmp;

  /* FREQUENT GC CALLS SEEMS TO BE BENEFICIAL FOR THIS EXAMPLE */
#ifdef USE_BIDDY_X
  Biddy_AutoGC();
#endif

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
  }

  /* EXPLICIT CALL TO GC AFTER EACH MAJOR PART - IMPLEMENTED FOR BIDDY, ONLY */
#ifdef USE_BIDDY_X
  Biddy_AutoGC();
#endif

  /* PROFILING */
  /*
  checkOutOfLimits(size,elapsedtime);
  */

  /* SIFTING AFTER EACH MAJOR PART - IMPLEMENTED FOR BIDDY, ONLY */
  /* IT IS NOT VERY EFFECTIVE FOR THIS PROBLEM */
#ifdef USE_BIDDY
#ifdef NOT_POSTPONE
#ifdef USE_SIFTING
  ZF_LOGI("C_SIFT");
  Biddy_Sifting(NULL,FALSE);
#endif
#ifdef USE_SIFTING_R
  ZF_LOGI("C_SIFT_R");
  Biddy_Sifting(r,FALSE);
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
      t = Biddy_GetConstantOne();
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
#ifdef NOT_POSTPONE
#ifdef USE_BIDDY
#ifdef USE_GARBAGE_COLLECTION
      Biddy_AddTmpFormula(r,1);
      Biddy_AddTmpFormula(tmp,1);
      Biddy_Clean();
#endif
#endif
#endif
#ifdef USE_CUDD
      Cudd_Ref(tmp);
      Cudd_RecursiveDeref(manager,t);
#endif
      t = tmp;

  /* FREQUENT GC CALLS SEEMS TO BE BENEFICIAL FOR THIS EXAMPLE */
#ifdef USE_BIDDY_X
  Biddy_AutoGC();
#endif

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
  }

  /* EXPLICIT CALL TO GC AFTER EACH MAJOR PART - IMPLEMENTED FOR BIDDY, ONLY */
#ifdef USE_BIDDY_X
  Biddy_AutoGC();
#endif

  /* PROFILING */
  /*
  checkOutOfLimits(size,elapsedtime);
  */

  /* SIFTING AFTER EACH MAJOR PART - IMPLEMENTED FOR BIDDY, ONLY */
  /* IT IS NOT VERY EFFECTIVE FOR THIS PROBLEM */
#ifdef USE_BIDDY
#ifdef NOT_POSTPONE
#ifdef USE_SIFTING
  ZF_LOGI("RD_SIFT");
  Biddy_Sifting(NULL,FALSE);
#endif
#ifdef USE_SIFTING_R
  ZF_LOGI("RD_SIFT_R");
  Biddy_Sifting(r,FALSE);
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
      t = Biddy_GetConstantOne();
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
#ifdef NOT_POSTPONE
#ifdef USE_BIDDY
#ifdef USE_GARBAGE_COLLECTION
      Biddy_AddTmpFormula(r,1);
      Biddy_AddTmpFormula(tmp,1);
      Biddy_Clean();
#endif
#endif
#endif
#ifdef USE_CUDD
      Cudd_Ref(tmp);
      Cudd_RecursiveDeref(manager,t);
#endif
      t = tmp;

  /* FREQUENT GC CALLS SEEMS TO BE BENEFICIAL FOR THIS EXAMPLE */
#ifdef USE_BIDDY_X
  Biddy_AutoGC();
#endif

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
  }

  /* EXPLICIT CALL TO GC AFTER EACH MAJOR PART - IMPLEMENTED FOR BIDDY, ONLY */
#ifdef USE_BIDDY_X
  Biddy_AutoGC();
#endif

  /* PROFILING */
  /*
  checkOutOfLimits(size,elapsedtime);
  */

  /* SIFTING AFTER EACH MAJOR PART - IMPLEMENTED FOR BIDDY, ONLY */
  /* IT IS NOT VERY EFFECTIVE FOR THIS PROBLEM */
#ifdef USE_BIDDY
#ifdef NOT_POSTPONE
#ifdef USE_SIFTING
  ZF_LOGI("FD_SIFT");
  Biddy_Sifting(NULL,FALSE);
#endif
#ifdef USE_SIFTING_R
  ZF_LOGI("FD_SIFT_R");
  Biddy_Sifting(r,FALSE);
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
    t = Biddy_GetConstantZero();
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

  /* FREQUENT GC CALLS SEEMS TO BE BENEFICIAL FOR THIS EXAMPLE */
#ifdef USE_BIDDY_X
  Biddy_AutoGC();
#endif

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

    /* PROFILING */
    /*
    checkOutOfLimits(size,elapsedtime);
    */

  }

  /* EXPLICIT CALL TO GC AFTER EACH MAJOR PART - IMPLEMENTED FOR BIDDY, ONLY */
#ifdef USE_BIDDY_X
  Biddy_AutoGC();
#endif

  /* PROFILING */
  /*
  checkOutOfLimits(size,elapsedtime);
  */

  /* SIFTING AFTER EACH MAJOR PART - IMPLEMENTED FOR BIDDY, ONLY */
  /* IT IS NOT VERY EFFECTIVE FOR THIS PROBLEM */
#ifdef USE_BIDDY
#ifdef NOT_POSTPONE
#ifdef USE_SIFTING
  ZF_LOGI("FD_SIFT");
  Biddy_Sifting(NULL,FALSE);
#endif
#ifdef USE_SIFTING_R
  ZF_LOGI("FD_SIFT_R");
  Biddy_Sifting(r,FALSE);
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

  /* EXPLICIT CALL TO GC TO MINIMIZE THE RESULT - IMPLEMENTED FOR BIDDY, ONLY */
#ifdef USE_BIDDY_X
  Biddy_AutoGC();
#endif

  /* SIFTING TO MINIMIZE THE RESULT - IMPLEMENTED FOR BIDDY, ONLY */
#ifdef USE_BIDDY
#ifdef NOT_POSTPONE
#ifdef USE_SIFTING
  ZF_LOGI("F_SIFT");
  Biddy_Sifting(NULL,FALSE);
#endif
#ifdef USE_SIFTING_R
  ZF_LOGI("F_SIFT_R");
  Biddy_Sifting(r,FALSE);
#endif
#endif
#endif

  ZF_LOGI("EXIT");

  /* PROFILING */
  /*
  checkOutOfLimits(size,elapsedtime);
  */

  elapsedtime = clock()-elapsedtime;

  /*
  if (clock_gettime(CLOCK_PROCESS_CPUTIME_ID,&stoptimex) == -1) printf("ERROR WITH clock_gettime!\n");
  fprintf(stderr,"clock_gettime() TIME = %.2f\n",(stoptimex.tv_sec-elapsedtimex.tv_sec)+(stoptimex.tv_nsec-elapsedtimex.tv_nsec)/1000000000.0);
  */

  /* RESULT */
#ifdef REPORT
  printf("\n");
  n = Biddy_DependentVariableNumber(r);
  printf("Resulting function r has %.0f minterms.\n",Biddy_CountMinterm(r,n));
  printf("Resulting function r depends on %u variables.\n",n);
  printf("BDD for resulting function r has %u nodes.\n",Biddy_NodeNumber(r));
  printf("BDD for the resulting function has density = %.2f.\n",Biddy_DensityBDD(r,0));
#ifdef USE_BIDDY
  printf("Resulting function has density = %e.\n",Biddy_DensityFunction(r,0));
  printf("BDD without complemented edges for resulting function r has %u nodes (including both constants).\n",Biddy_NodeNumberPlain(r));
#endif
#endif

  /* CONVERT TO OBDD AND REPORT THE SIZE OF THE EQUIVALENT OBDD - Biddy ONLY! */
  /*
  {
  Biddy_Manager MNGOBDD;
  Biddy_Edge obdd;
  Biddy_InitMNG(&MNGOBDD,BIDDYTYPEOBDD);
  obdd = Biddy_Copy(MNGOBDD,r);
  n = Biddy_Managed_DependentVariableNumber(MNGOBDD,obdd);
  printf("Function represented by the equivalent OBDD depends on %u variables.\n",n);
  printf("Function represented by the equivalent OBDD has %.0f minterms.\n",Biddy_Managed_CountMinterm(MNGOBDD,obdd,n));
  printf("Function represented by the equivalent OBDD has density = %e.\n",Biddy_Managed_DensityFunction(MNGOBDD,obdd,0));
  printf("Equivalent OBDD has %u nodes.\n",Biddy_Managed_NodeNumber(MNGOBDD,obdd));
  printf("Equivalent OBDD without complemented edges for resulting function r has %u nodes (including both constants).\n",Biddy_Managed_NodeNumberPlain(MNGOBDD,obdd));
  printf("Equivalent OBDD has density = %.2f.\n",Biddy_Managed_DensityBDD(MNGOBDD,obdd,0));
  }
  */

  /* CONVERT TO ZBDD AND REPORT THE SIZE OF THE EQUIVALENT TZBDD - Biddy ONLY! */
  /*
  {
  Biddy_Manager MNGZBDD;
  Biddy_Edge zbdd;
  Biddy_InitMNG(&MNGZBDD,BIDDYTYPEZBDD);
  zbdd = Biddy_Copy(MNGZBDD,r);
  printf("ZBDD for function r has %u nodes.\n",Biddy_Managed_NodeNumber(MNGZBDD,zbdd));
  }
  */

  /* CONVERT TO TZBDD AND REPORT THE SIZE OF THE EQUIVALENT TZBDD - Biddy ONLY! */
  /*
  {
  Biddy_Manager MNGTZBDD;
  Biddy_Edge tzbdd;
  Biddy_InitMNG(&MNGTZBDD,BIDDYTYPETZBDD);
  tzbdd = Biddy_Copy(MNGTZBDD,r);
  printf("TZBDD for function r has %u nodes.\n",Biddy_Managed_NodeNumber(MNGTZBDD,tzbdd));
  }  
  */

  /* DUMP RESULT USING GRAPHVIZ/DOT */
  /*
#ifdef USE_BIDDY
  Biddy_WriteDot("8queens-biddy.dot",r,"r",-1,FALSE);
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
  printf("\n");
  Biddy_PrintInfo(stdout);
#endif

  /* SIFTING ON THE RESULT - THIS IS NOT VERY EFFECTIVE FOR THIS PROBLEM */
  /*
#ifdef USE_BIDDY
  Biddy_Sifting(NULL,FALSE);
#endif
#ifdef USE_CUDD
  Cudd_ReduceHeap(manager,CUDD_REORDER_SIFT,0);
#endif
#ifdef REPORT
  n = Biddy_DependentVariableNumber(r);
  printf("(AFTER SIFTING) Resulting function r depends on %u variables.\n",n);
  printf("(AFTER SIFTING) Resulting function r has %.0f minterms.\n",Biddy_CountMinterm(r,n));
  printf("(AFTER SIFTING) BDD for resulting function r has %u nodes.\n",Biddy_NodeNumber(r));
#endif
  */

#ifndef REPORT
  /*
  if (Biddy_GetManagerType() == BIDDYTYPEOBDD) {
    fprintf(stderr,"(OBDD) ");
  }
  else if (Biddy_GetManagerType() == BIDDYTYPEZBDD) {
    fprintf(stderr,"(ZBDD) ");
  }
  else if (Biddy_GetManagerType() == BIDDYTYPETZBDD) {
    fprintf(stderr,"(TZBDD) ");
  }
  else {
    fprintf(stderr,"(unknown BDD type)");
    exit(1);
  }
  n = Biddy_DependentVariableNumber(r);
  fprintf(stderr,"Resulting function r has %.0f minterms.\n",Biddy_CountMinterm(r,n));
  */
#endif

  /* REPORT ELAPSED TIME */
  /*
  fprintf(stderr,"clock() TIME = %.2f\n",elapsedtime/(1.0*CLOCKS_PER_SEC));
  */
            
#ifdef REPORT
  printf("\nCALCULATION FINISHED\n");
#endif

  /* PROFILING */
  /*
#ifdef USE_BIDDY
  fprintf(stderr,"%2u, ",size);
#ifdef MINGW
  fprintf(stderr,"%I64u, ",Biddy_ReadMemoryInUse());
#else
  fprintf(stderr,"%10llu, ",Biddy_ReadMemoryInUse());
#endif
  if (Biddy_NodeTableANDORRecursiveNumber()) {
    fprintf(stderr,"%llu, ",Biddy_NodeTableANDORRecursiveNumber());
  }
  fprintf(stderr,"%2u, ",Biddy_NodeTableGCNumber());
  if (Biddy_NodeTableGCObsoleteNumber()) {
    fprintf(stderr,"%llu, ",Biddy_NodeTableGCObsoleteNumber());
    fprintf(stderr,"%.2f, ",Biddy_NodeTableGCTime()/(1.0*CLOCKS_PER_SEC));
  }
  fprintf(stderr,"%.2f\n",elapsedtime/(1.0*CLOCKS_PER_SEC));
#endif
#ifdef USE_CUDD
  fprintf(stderr,"%u, ",size);
  fprintf(stderr,"%zu, ",Cudd_ReadMemoryInUse(manager));
  fprintf(stderr,"0, ");
  fprintf(stderr,"%.2f\n",elapsedtime/(1.0*CLOCKS_PER_SEC));
#endif
  */

  /* PROFILING */
  /*
  printf("\n");
  Biddy_PrintInfo(stdout);
  */

  /* EXIT */
#ifdef USE_BIDDY
  Biddy_Exit();
#endif
#ifdef USE_CUDD
  Cudd_RecursiveDeref(manager,r);
#ifdef REPORT
  printf("CUDD: nodes with non-zero reference counts: %d\n",Cudd_CheckZeroRef(manager));
#endif
  Cudd_Quit(manager);
#endif

}

/* $Revision: 456 $ */
/* $Date: 2018-07-14 21:11:41 +0200 (sob, 14 jul 2018) $ */
/* This file (biddy-example-8queens.c) is a C file */
/* Author: Robert Meolic (robert.meolic@um.si) */
/* This file has been released into the public domain by the author. */

/* This example is compatible with Biddy v1.8 and CUDD v3.0.0 */

/* COMPILE WITH (remove -static if you have Biddy as a dynamic library): */
/* gcc -DNOREPORT -DNOPROFILE -DEVENTLOG_NONE -DBIDDY -DOBDD -DUNIX -O2 -o biddy-example-8queens-obdd biddy-example-8queens.c -I. -L./bin -static -lbiddy -lgmp */
/* gcc -DNOREPORT -DNOPROFILE -DEVENTLOG_NONE -DBIDDY -DOBDDC -DUNIX -O2 -o biddy-example-8queens-obddc biddy-example-8queens.c -I. -L./bin -static -lbiddy -lgmp */
/* gcc -DNOREPORT -DNOPROFILE -DEVENTLOG_NONE -DBIDDY -DZBDD -DUNIX -O2 -o biddy-example-8queens-zbdd biddy-example-8queens.c -I. -L./bin -static -lbiddy -lgmp */
/* gcc -DNOREPORT -DNOPROFILE -DEVENTLOG_NONE -DBIDDY -DZBDDC -DUNIX -O2 -o biddy-example-8queens-zbddc biddy-example-8queens.c -I. -L./bin -static -lbiddy -lgmp */
/* gcc -DNOREPORT -DNOPROFILE -DEVENTLOG_NONE -DBIDDY -DTZBDD -DUNIX -O2 -o biddy-example-8queens-tzbdd biddy-example-8queens.c -I. -L./bin -static -lbiddy -lgmp */
/* gcc -DNOREPORT -DEVENTLOG_NONE -DCUDD -DOBDDC -O2 -o cudd-example-8queens-obddc biddy-example-8queens.c -I ../cudd/include/ -L ../cudd/lib/ -lcudd -lm */
/* gcc -DNOREPORT -DEVENTLOG_NONE -DCUDD -DZBDDC -O2 -o cudd-example-8queens-zbddc biddy-example-8queens.c -I ../cudd/include/ -L ../cudd/lib/ -lcudd -lm */

/* for stats with more details, use BIDDYEXTENDEDSTATS_YES in biddyInt.h and use -lm for linking */

/* default size, this can be overriden via argument */
#define SIZE 9
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
#  ifdef BIDDY
#    define ZF_LOGI(x) {static unsigned int zfstat = 0;printf(x);printf("#%u,%u,%u\n",++zfstat,Biddy_NodeTableNum(),Biddy_NodeTableSize());}
#  endif
#  ifdef CUDD
#    define ZF_LOGI(x) {static unsigned int zfstat = 0;printf(x);printf("#%u,%u,%u\n",++zfstat,Cudd_ReadKeys(manager),Cudd_ReadSlots(manager));}
#  endif
#endif
#ifdef EVENTLOG_RESULT
#  ifdef BIDDY
#    define ZF_LOGI(x) {static unsigned int zfstat = 0;printf(x);printf("#%u,%u\n",++zfstat,Biddy_CountNodes(r));}
#  endif
#  ifdef CUDD
#    define ZF_LOGI(x) {static unsigned int zfstat = 0;printf(x);printf("#%u,%u\n",++zfstat,Cudd_DagSize(r));}
#  endif
#endif
#ifndef ZF_LOGI
#  define ZF_LOGI(x)
#endif

#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <time.h>

#include "biddy-cudd.h"

void checkOutOfLimits(unsigned int size, clock_t elapsedtime) {
#ifdef BIDDY
  if ((size == 10) && (Biddy_NodeTableSize() > 4194304)) {
    fprintf(stderr,"10, -1, -1, -1\n");
    exit(0);
  }
  if ((size == 9) && (Biddy_ReadMemoryInUse() > 120000000)) {
    fprintf(stderr,"9, -1, -1, -1\n");
    exit(0);
  }
  if ((size == 10) && (Biddy_ReadMemoryInUse() > 350000000)) {
    fprintf(stderr,"10, -1, -1, -1\n");
    exit(0);
  }
#endif
}

int main(int argc, char** argv) {
  unsigned int i,j,k,n,size,seq;
  Biddy_Edge **board,**board_not;
  Biddy_Edge tmp;
  Biddy_Edge t,r;
  Biddy_Edge *fifo1,*LP1;
  Biddy_Edge *fifo2,*LP2;
#if defined(POSTPONE_FIFO) || defined(POSTPONE_REVERSE_FIFO)
  unsigned int seq1;
#endif

  clock_t elapsedtime;

  if (argc > 1) {
#pragma warning(suppress: 6031)
    sscanf(argv[1],"%u",&size);
  } else {
    size = SIZE;
  }

  if ((size < 2) || (size > 15)) {
    printf("WRONG SIZE!\n");
    exit(0);
  }

  setbuf(stdout, NULL);

#ifdef PROFILE
  printf("!!!");
#endif

  elapsedtime = clock();

#include "biddy-cudd.c" /* this will initialize BDD manager */

  LP1 = fifo1 = (Biddy_Edge *) malloc((4 * size * size + size) * sizeof(Biddy_Edge));
  LP2 = fifo2 = (Biddy_Edge *) malloc((4 * size * size + size) * sizeof(Biddy_Edge));
  LP1--;

#ifdef BIDDY
  seq = 0;
#endif

  board = (Biddy_Edge **) malloc((size+1)*sizeof(Biddy_Edge *));
  board_not = (Biddy_Edge **) malloc((size+1)*sizeof(Biddy_Edge *));
  for (i=0; i<=size; i++) {
    board[i] = (Biddy_Edge *) malloc((size+1)*sizeof(Biddy_Edge));
    board_not[i] = (Biddy_Edge *) malloc((size+1)*sizeof(Biddy_Edge));
  }

#ifdef BIDDY
  if ((argc == 1) || (argc == 2)) {
    Biddy_SetManagerParameters(-1,-1,-1,-1,-1,-1,-1,-1);
  }
  else if (argc == 8) {
    int gcr,gcrF,gcrX;
    int rr,rrF,rrX;
    sscanf(argv[2],"%d",&gcr);
    sscanf(argv[3],"%d",&gcrF);
    sscanf(argv[4],"%d",&gcrX);
    sscanf(argv[5],"%d",&rr);
    sscanf(argv[6],"%d",&rrF);
    sscanf(argv[7],"%d",&rrX);
    Biddy_SetManagerParameters(gcr/100.0,gcrF/100.0,gcrX/100.0,rr/100.0,rrF/100.0,rrX/100.0,-1,-1);
    printf("%d, %d, %d, %d, %d, %d, ",gcr,gcrF,gcrX,rr,rrF,rrX);
  }
  else if (argc == 11) {
    int gcr,gcrF,gcrX;
    int rr,rrF,rrX;
    int st,cst;
    sscanf(argv[2],"%d",&gcr);
    sscanf(argv[3],"%d",&gcrF);
    sscanf(argv[4],"%d",&gcrX);
    sscanf(argv[5],"%d",&rr);
    sscanf(argv[6],"%d",&rrF);
    sscanf(argv[7],"%d",&rrX);
    sscanf(argv[8],"%d",&st);
    sscanf(argv[9],"%d",&cst);
    Biddy_SetManagerParameters(gcr/100.0,gcrF/100.0,gcrX/100.0,rr/100.0,rrF/100.0,rrX/100.0,st/100.0,cst/100.0);
    printf("%d, %d, %d, %d, %d, %d, ",gcr,gcrF,gcrX,rr,rrF,rrX);
  }
  else {
    printf("Wrong number of parameters!\n");
    printf("USAGE (short): biddy-example-8queens SIZE GCR GCRF GCRX RR RRF RRX\n");
    printf("USAGE (long): biddy-example-8queens SIZE GCR GCRF GCRX RR RRF RRX ST CST\n");
    printf("All parameters should be integer numbers >= 0");
    exit(0);
  }
#endif

#ifdef CUDD
  Cudd_SetMaxCacheHard(manager,262144);
#endif

  for (i=1; i<=size; i++) {
#pragma warning(suppress: 6011)
    board[0][i] = board[i][0] = NULL;
    board_not[0][i] = board_not[i][0] = NULL;
  }

  /* create BDD variables */
  if ((Biddy_GetManagerType() == BIDDYTYPEOBDD) || (Biddy_GetManagerType() == BIDDYTYPEOBDDC) ||
      (Biddy_GetManagerType() == BIDDYTYPETZBDD) || (Biddy_GetManagerType() == BIDDYTYPETZBDDC))
  {
    for (i=1; i<=size; i++) {
      for (j=1; j<=size; j++) {
        board[i][j] = Biddy_AddVariableEdge();
        REF(board[i][j]);
        board_not[i][j] = Biddy_Not(board[i][j]);
        REF(board_not[i][j]);
      }
    }
  }

  /* for Biddy, ZBDD variables are added in the reverse order to get consistent results */
#ifdef BIDDY
  if ((Biddy_GetManagerType() == BIDDYTYPEZBDD) || (Biddy_GetManagerType() == BIDDYTYPEZBDDC))
  {
    for (i=1; i<=size; i++) {
      for (j=1; j<=size; j++) {
        board[size-i+1][size-j+1] = Biddy_AddVariableEdge();
        board_not[size-i+1][size-j+1] = Biddy_Not(board[size-i+1][size-j+1]);
      }
    }
  }
#endif
#ifdef CUDD
  if ((Biddy_GetManagerType() == BIDDYTYPEZBDD) || (Biddy_GetManagerType() == BIDDYTYPEZBDDC))
  {
    for (i=1; i<=size; i++) {
      for (j=1; j<=size; j++) {
        board[i][j] = Biddy_AddVariableEdge();
        REF(board[i][j]);
        board_not[i][j] = Biddy_Not(board[i][j]);
        REF(board_not[i][j]);
      }
    }
  }
#endif

  /* for ZBDDs, by adding new variables all the existing variables are changed */
  /* after adding the last variable we have to refresh edges in board[i][j] and board_not[i][j] */
  /* NOTE: for Biddy, ZBDDs variables are added in the reverse order */
  /* NOTE: for Biddy, user variables start with index 1 */
  /* NOTE: for CUDD, user variables start with index 0 */
  if ((Biddy_GetManagerType() == BIDDYTYPEZBDD) || (Biddy_GetManagerType() == BIDDYTYPEZBDDC))
  {
#ifdef BIDDY
    /* user variables in Biddy start with index 1 */
    for (i=1; i<=size; i++) {
      for (j=1; j<=size; j++) {
        board[i][j] = Biddy_GetVariableEdge(size*size-((i-1)*size+(j-1)));
        board_not[i][j] = Biddy_Not(board[i][j]);
      }
    }
#endif
#ifdef CUDD
    /* user variables in CUDD start with index 0 */
    for (i=1; i<=size; i++) {
      for (j=1; j<=size; j++) {
        DEREF(board[i][j]);
        board[i][j] = Cudd_zddIthVar(manager,(i-1)*size+(j-1));
        REF(board[i][j]);
        DEREF(board_not[i][j]);
        board_not[i][j] = Biddy_Not(board[i][j]);
        REF(board_not[i][j]);
      }
    }
#endif
  }

  /* for TZBDDs, by adding new variables the negations of all the existing variables are changed */
  /* after adding the last variable we have to refresh edges in board_not[i][j] */
  /* NOTE: TZBDDs are supported only in Biddy */
  if ((Biddy_GetManagerType() == BIDDYTYPETZBDD) || (Biddy_GetManagerType() == BIDDYTYPETZBDDC))
  {
    for (i=1; i<=size; i++) {
      for (j=1; j<=size; j++) {
        board_not[i][j] = Biddy_Not(board[i][j]);
      }
    }
  }

/* for OBDD, ZBDD, ZBDDC, and TZBDD, preserve board_not[i][j] */
/* this is not needed for OBDDC because they are simple complemented edges */
/* TO DO: replace Biddy_AddPersistentFormula with the correct MARK_N */
#ifdef BIDDY
#ifdef NOT_POSTPONE
  if ((Biddy_GetManagerType() == BIDDYTYPEOBDD) ||
      (Biddy_GetManagerType() == BIDDYTYPEZBDD) || (Biddy_GetManagerType() == BIDDYTYPEZBDDC) ||
      (Biddy_GetManagerType() == BIDDYTYPETZBDD) || (Biddy_GetManagerType() == BIDDYTYPETZBDDC))
  {
    for (i=1; i<=size; i++) {
      for (j=1; j<=size; j++) {
        Biddy_AddPersistentFormula(NULL,board_not[i][j]);
      }
    }
  }
#endif
#endif

  r = Biddy_GetConstantOne();
  REF(r);

  ZF_LOGI("INIT");

  /* ROWS */
#ifdef REPORT
  printf("\nSTARTING ROWS\n");
#endif
  for (i=1; i<=size; i++) {
#ifdef REPORT
    printf("(%d/%d-%u)",i,size,Biddy_CountNodes(r));
#endif
    for (j=1; j<=size; j++) {
      t = Biddy_GetConstantOne();
      REF(t);
      for (k=1; k<=size; k++) {
        if (k != j) {
          ZF_LOGI("R_AND");
          tmp = Biddy_And(t,board_not[i][k]);
          REF(tmp);
          DEREF(t);
          t = tmp;
        }
      }
      ZF_LOGI("R_OR");
      tmp = Biddy_Or(t,board_not[i][j]);
#ifdef NOT_POSTPONE
      MARK(r);
      MARK(tmp);
      SWEEP();
#endif
      REF(tmp);
      DEREF(t);
      t = tmp;

#ifdef NOT_POSTPONE
      ZF_LOGI("R_AND");
      tmp = Biddy_And(r,t);
      MARK(tmp);
      SWEEP();
      REF(tmp);
      DEREF(t);
      DEREF(r);
      r = tmp;
#else
      *(++LP1) = t;
#endif

      ZF_LOGI("R_MEM");

#ifdef POSTPONE_FIFO
      seq++;
      MARK_N(t,(seq-1)/2);
#endif
#ifdef POSTPONE_REVERSE_FIFO
      seq++;
      MARK_N(t,(4 * size * size + size - seq)/2);
#endif
#ifdef POSTPONE_LIFO
      seq++;
      if (seq == 1) {
        MARK_N(t,seq-1);
      } else {
        MARK_N(t,seq-2);
      }
#endif
#ifdef POSTPONE_REVERSE_LIFO
      seq++;
      MARK_N(t,(4 * size * size + size) - seq - 1);
#endif

    }
  }

#ifdef PROFILE
  checkOutOfLimits(size,elapsedtime);
#endif

  /* SIFTING AFTER EACH MAJOR PART - IMPLEMENTED FOR BIDDY, ONLY */
  /* IT IS NOT VERY EFFECTIVE FOR THIS PROBLEM */
#ifdef BIDDY
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
    printf("(%d/%d-%u)",i,size,Biddy_CountNodes(r));
#endif
    for (j=1; j<=size; j++) {
      t = Biddy_GetConstantOne();
      REF(t);
      for (k=1; k<=size; k++) {
        if (k != i) {
          ZF_LOGI("C_AND");
          tmp = Biddy_And(t,board_not[k][j]);
          REF(tmp);
          DEREF(t);
          t = tmp;
        }
      }
      ZF_LOGI("C_OR");
      tmp = Biddy_Or(t,board_not[i][j]);
#ifdef NOT_POSTPONE
      MARK(r);
      MARK(tmp);
      SWEEP();
#endif
      REF(tmp);
      DEREF(t);
      t = tmp;

#ifdef NOT_POSTPONE
      ZF_LOGI("C_AND");
      tmp = Biddy_And(r,t);
      MARK(tmp);
      SWEEP();
      REF(tmp);
      DEREF(t);
      DEREF(r);
      r = tmp;
#else
      *(++LP1) = t;
#endif

      ZF_LOGI("C_MEM");

#ifdef POSTPONE_FIFO
      seq++;
      MARK_N(t,(seq-1)/2);
#endif
#ifdef POSTPONE_REVERSE_FIFO
      seq++;
      MARK_N(t,(4 * size * size + size - seq)/2);
#endif
#ifdef POSTPONE_LIFO
      seq++;
      MARK_N(t,seq-2);
#endif
#ifdef POSTPONE_REVERSE_LIFO
      seq++;
      MARK_N(t,(4 * size * size + size) - seq - 1);
#endif

    }
  }

#ifdef PROFILE
  checkOutOfLimits(size,elapsedtime);
#endif

  /* SIFTING AFTER EACH MAJOR PART - IMPLEMENTED FOR BIDDY, ONLY */
  /* IT IS NOT VERY EFFECTIVE FOR THIS PROBLEM */
#ifdef BIDDY
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
    printf("(%d/%d-%u)",i,size,Biddy_CountNodes(r));
#endif
    for (j=1; j<=size; j++) {
      t = Biddy_GetConstantOne();
      REF(t);
      for (k=1; k<=size; k++) {
        if ((j+k-i >= 1) && (j+k-i <= size) && (k != i)) {
          ZF_LOGI("RD_AND");
          tmp = Biddy_And(t,board_not[k][j+k-i]);
          REF(tmp);
          DEREF(t);
          t = tmp;
        }
      }
      ZF_LOGI("RD_OR");
      tmp = Biddy_Or(t,board_not[i][j]);
#ifdef NOT_POSTPONE
      MARK(r);
      MARK(tmp);
      SWEEP();
#endif
      REF(tmp);
      DEREF(t);
      t = tmp;

#ifdef NOT_POSTPONE
      ZF_LOGI("RD_AND");
      tmp = Biddy_And(r,t);
      MARK(tmp);
      SWEEP();
      REF(tmp);
      DEREF(t);
      DEREF(r);
      r = tmp;
#else
      *(++LP1) = t;
#endif

      ZF_LOGI("RD_MEM");

#ifdef POSTPONE_FIFO
      seq++;
      MARK_N(t,(seq-1)/2);
#endif
#ifdef POSTPONE_REVERSE_FIFO
      seq++;
      MARK_N(t,(4 * size * size + size - seq)/2);
#endif
#ifdef POSTPONE_LIFO
      seq++;
      MARK_N(t,seq-2);
#endif
#ifdef POSTPONE_REVERSE_LIFO
      seq++;
      MARK_N(t,(4 * size * size + size) - seq - 1);
#endif

    }
  }

#ifdef PROFILE
  checkOutOfLimits(size,elapsedtime);
#endif

  /* SIFTING AFTER EACH MAJOR PART - IMPLEMENTED FOR BIDDY, ONLY */
  /* IT IS NOT VERY EFFECTIVE FOR THIS PROBLEM */
#ifdef BIDDY
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
    printf("(%d/%d-%u)",i,size,Biddy_CountNodes(r));
#endif
    for (j=1; j<=size; j++) {
      t = Biddy_GetConstantOne();
      REF(t);
      for (k=1; k<=size; k++) {
        if ((j+i-k >= 1) && (j+i-k <= size) && (k != i)) {
          ZF_LOGI("FD_AND");
          tmp = Biddy_And(t,board_not[k][j+i-k]);
          REF(tmp);
          DEREF(t);
          t = tmp;
        }
      }
      ZF_LOGI("FD_OR");
#pragma warning(suppress: 6011)
      tmp = Biddy_Or(t,board_not[i][j]);
#ifdef NOT_POSTPONE
      MARK(r);
      MARK(tmp);
      SWEEP();
#endif
      REF(tmp);
      DEREF(t);
      t = tmp;

#ifdef NOT_POSTPONE
      ZF_LOGI("FD_AND");
      tmp = Biddy_And(r,t);
      MARK(tmp);
      SWEEP();
      REF(tmp);
      DEREF(t);
      DEREF(r);
      r = tmp;
#else
      *(++LP1) = t;
#endif

      ZF_LOGI("FD_MEM");

#ifdef POSTPONE_FIFO
      seq++;
      MARK_N(t,(seq-1)/2);
#endif
#ifdef POSTPONE_REVERSE_FIFO
      seq++;
      MARK_N(t,(4 * size * size + size - seq)/2);
#endif
#ifdef POSTPONE_LIFO
      seq++;
      MARK_N(t,seq-2);
#endif
#ifdef POSTPONE_REVERSE_LIFO
      seq++;
      MARK_N(t,(4 * size * size + size) - seq - 1);
#endif

    }
  }

#ifdef PROFILE
  checkOutOfLimits(size,elapsedtime);
#endif

  /* SIFTING AFTER EACH MAJOR PART - IMPLEMENTED FOR BIDDY, ONLY */
  /* IT IS NOT VERY EFFECTIVE FOR THIS PROBLEM */
#ifdef BIDDY
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
    printf("(%d/%d-%u)",i,size,Biddy_CountNodes(r));
#endif
    t = Biddy_GetConstantZero();
    REF(t);
    for (j=1; j<=size; j++) {
      ZF_LOGI("F_OR");
      tmp = Biddy_Or(t,board[i][j]);
      REF(tmp);
      DEREF(t);
      t = tmp;
    }

#ifdef NOT_POSTPONE
    ZF_LOGI("F_AND");
    tmp = Biddy_And(r,t);
    MARK(tmp);
    SWEEP();
    REF(tmp);
    DEREF(t);
    DEREF(r);
    r = tmp;
#else
    *(++LP1) = t;
#endif

    ZF_LOGI("F_MEM");

#ifdef POSTPONE_FIFO
    seq++;
    MARK_N(t,(seq-1)/2);
#endif
#ifdef POSTPONE_REVERSE_FIFO
    seq++;
    MARK_N(t,(4 * size * size + size - seq)/2);
#endif
#ifdef POSTPONE_LIFO
    seq++;
    MARK_N(t,seq-2);
#endif
#ifdef POSTPONE_REVERSE_LIFO
    seq++;
    if (seq == (4 * size *size + size)) {
      MARK_N(t,4 * size * size + size - seq);
    } else {
      MARK_N(t,4 * size * size + size - seq - 1);
    }
#endif

#ifdef PROFILE
  checkOutOfLimits(size,elapsedtime);
#endif

  }

#ifdef PROFILE
  checkOutOfLimits(size,elapsedtime);
#endif

  /* SIFTING AFTER EACH MAJOR PART - IMPLEMENTED FOR BIDDY, ONLY */
  /* IT IS NOT VERY EFFECTIVE FOR THIS PROBLEM */
#ifdef BIDDY
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
  DEREF(r);
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
        MARK_N(r,seq1+(seq-1)/2);
        SWEEP();
        seq1--;
      } else {
        t = *(--LP1);
        ZF_LOGI("P_AND");
        tmp = Biddy_And(r,t);
        *(++LP2) = tmp;
        MARK_N(tmp,seq1+(seq-1)/2);
        SWEEP();
        seq1--;
        REF(tmp);
        DEREF(manager,r);
        DEREF(manager,t);
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
  DEREF(r);
  i = 0;
  while(LP1 != fifo1) {
    r = *(LP1--);
    t = *(LP1--);
    ZF_LOGI("P_AND");
    tmp = Biddy_And(r,t);
    *(++LP1) = tmp;
    MARK(tmp);
    SWEEP();
    REF(tmp);
    DEREF(r);
    DEREF(t);
  }
  r = *(LP1);
#endif

  free(fifo1);
  free(fifo2);

  /* SIFTING TO MINIMIZE THE RESULT - IMPLEMENTED FOR BIDDY, ONLY */
#ifdef BIDDY
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

#ifdef PROFILE
  checkOutOfLimits(size,elapsedtime);
#endif

  elapsedtime = clock()-elapsedtime;

  /* RESULT */
#ifdef REPORT
  printf("\n");
  n = Biddy_DependentVariableNumber(r,FALSE);
  printf("Resulting function has %.0f minterms.\n",Biddy_CountMinterms(r,n));
  printf("Resulting function depends on %u variables.\n",n);
  printf("BDD for resulting function has %u nodes.\n",Biddy_CountNodes(r));
  printf("BDD for the resulting function has density = %.2f.\n",Biddy_DensityOfBDD(r,0));
#ifdef BIDDY
  printf("Resulting function has density = %e.\n",Biddy_DensityOfFunction(r,0));
  if ((Biddy_GetManagerType() == BIDDYTYPEOBDDC) || (Biddy_GetManagerType() == BIDDYTYPEZBDDC))
  {
    printf("BDD for resulting function has %u complemented edges.\n",Biddy_CountComplementedEdges(r));
    printf("BDD without complemented edges for resulting function has %u nodes.\n",Biddy_CountNodesPlain(r));
  }
#endif
#else
  n = Biddy_DependentVariableNumber(r,FALSE);
  printf("%.0f minterms, %u nodes\n",Biddy_CountMinterms(r,n),Biddy_CountNodes(r));
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
  printf("Function represented by the equivalent OBDD has %u nodes = %e.\n",Biddy_Managed_CountNodes(MNGOBDD,obdd));
  printf("Function represented by the equivalent OBDD has %.0f minterms.\n",Biddy_Managed_CountMinterms(MNGOBDD,obdd,n));
  printf("Function represented by the equivalent OBDD has density = %e.\n",Biddy_Managed_DensityOfFunction(MNGOBDD,obdd,0));
  }
  */

  /* CONVERT TO ZBDD AND REPORT THE SIZE OF THE EQUIVALENT ZBDD - Biddy ONLY! */
  /*
  {
  Biddy_Manager MNGZBDD;
  Biddy_Edge zbdd;
  Biddy_InitMNG(&MNGZBDD,BIDDYTYPEZBDD);
  zbdd = Biddy_Copy(MNGZBDD,r);
  n = Biddy_Managed_DependentVariableNumber(MNGZBDD,obdd);
  printf("Function represented by the equivalent ZBDD depends on %u variables.\n",n);
  printf("Function represented by the equivalent ZBDD has %u nodes = %e.\n",Biddy_Managed_CountNodes(MNGZBDD,zbdd));
  printf("Function represented by the equivalent ZBDD has %.0f minterms.\n",Biddy_Managed_CountMinterms(MNGZBDD,zbdd,n));
  printf("Function represented by the equivalent ZBDD has density = %e.\n",Biddy_Managed_DensityOfFunction(MNGZBDD,zbdd,0));
  }
  */

  /* CONVERT TO TZBDD AND REPORT THE SIZE OF THE EQUIVALENT TZBDD - Biddy ONLY! */
  /*
  {
  Biddy_Manager MNGTZBDD;
  Biddy_Edge tzbdd;
  Biddy_InitMNG(&MNGTZBDD,BIDDYTYPETZBDD);
  tzbdd = Biddy_Copy(MNGTZBDD,r);
  n = Biddy_Managed_DependentVariableNumber(MNGTZBDD,obdd);
  printf("Function represented by the equivalent TZBDD depends on %u variables.\n",n);
  printf("Function represented by the equivalent TZBDD has %u nodes = %e.\n",Biddy_Managed_CountNodes(MNGTZBDD,tzbdd));
  printf("Function represented by the equivalent TZBDD has %.0f minterms.\n",Biddy_Managed_CountMinterms(MNGTZBDD,tzbdd,n));
  printf("Function represented by the equivalent TZBDD has density = %e.\n",Biddy_Managed_DensityOfFunction(MNGTZBDD,tzbdd,0));
  }
  */

  /* DUMP RESULT USING GRAPHVIZ/DOT */
  /*
#ifdef BIDDY
  Biddy_WriteDot("8queens-biddy.dot",r,"r",-1,FALSE);
#elif CUDD
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
#ifdef BIDDY
  Biddy_Sifting(NULL,FALSE);
#endif
#ifdef CUDD
  Cudd_ReduceHeap(manager,CUDD_REORDER_SIFT,0);
#endif
#ifdef REPORT
  n = Biddy_DependentVariableNumber(r,FALSE);
  printf("(AFTER SIFTING) Resulting function r depends on %u variables.\n",n);
  printf("(AFTER SIFTING) Resulting function r has %.0f minterms.\n",Biddy_CountMinterms(r,n));
  printf("(AFTER SIFTING) BDD for resulting function r has %u nodes.\n",Biddy_CountNodes(r));
#endif
  */

#ifndef REPORT
  /*
  if (Biddy_GetManagerType() == BIDDYTYPEOBDD) {
    fprintf(stderr,"(OBDD) ");
  }
  else if (Biddy_GetManagerType() == BIDDYTYPEOBDDC) {
    fprintf(stderr,"(OBDDC) ");
  }
  else if (Biddy_GetManagerType() == BIDDYTYPEZBDD) {
    fprintf(stderr,"(ZBDD) ");
  }
  else if (Biddy_GetManagerType() == BIDDYTYPEZBDDC) {
    fprintf(stderr,"(ZBDDC) ");
  }
  else if (Biddy_GetManagerType() == BIDDYTYPETZBDD) {
    fprintf(stderr,"(TZBDD) ");
  }
  else {
    fprintf(stderr,"(unknown BDD type)");
    exit(0);
  }
  n = Biddy_DependentVariableNumber(r,FALSE);
  fprintf(stderr,"Resulting function r has %.0f minterms.\n",Biddy_CountMinterms(r,n));
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
  /**/
#ifdef BIDDY
  fprintf(stderr,"#%u, ",size);
#ifdef MINGW
  fprintf(stderr,"%I64u, ",Biddy_ReadMemoryInUse());
#else
  fprintf(stderr,"%llu, ",Biddy_ReadMemoryInUse());
#endif
/*
  if (Biddy_NodeTableFoaNumber()) {
    fprintf(stderr,"%llu, ",Biddy_NodeTableFoaNumber());
  }
  if (Biddy_NodeTableFindNumber()) {
    fprintf(stderr,"%llu, ",Biddy_NodeTableFindNumber());
  }
  if (Biddy_NodeTableCompareNumber()) {
    fprintf(stderr,"%llu, ",Biddy_NodeTableCompareNumber());
  }
  if (Biddy_NodeTableAddNumber()) {
    fprintf(stderr,"%llu, ",Biddy_NodeTableAddNumber());
  }
  if (Biddy_NodeTableGCObsoleteNumber()) {
    fprintf(stderr,"%llu, ",Biddy_NodeTableGCObsoleteNumber());
  }
  if (Biddy_NodeTableANDORRecursiveNumber()) {
    fprintf(stderr,"%llu, ",Biddy_NodeTableANDORRecursiveNumber());
  }
*/
#endif
#ifdef CUDD
  fprintf(stderr,"%u, ",size);
  fprintf(stderr,"%zu, ",Cudd_ReadMemoryInUse(manager));
#endif
  fprintf(stderr,"%u, ",Biddy_NodeTableGCNumber());
  fprintf(stderr,"%u, ",(unsigned int)Biddy_NodeTableGCTime());
  fprintf(stderr,"%.2f",elapsedtime/(1.0*CLOCKS_PER_SEC));
  fprintf(stderr,",\n");
  /**/

  /* PROFILING */
  /*
  printf("\n");
  Biddy_PrintInfo(stdout);
  */

  /* EXIT */
  for (i=1; i<=size; i++) {
    for (j=1; j<=size; j++) {
      DEREF(board[i][j]);
      DEREF(board_not[i][j]);
    }
  }
  DEREF(r);
  free(board);
  free(board_not);

#ifdef REPORT
#ifdef CUDD
  printf("CUDD: nodes with non-zero reference counts: %d\n",Cudd_CheckZeroRef(manager));
#endif
#endif

  Biddy_Exit();
}

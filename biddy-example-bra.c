/* $Revision: 361 $ */
/* $Date: 2017-12-16 21:25:47 +0100 (sob, 16 dec 2017) $ */
/* This file (biddy-example-bra.c) is a C file */
/* Author: Robert Meolic (robert.meolic@um.si) */
/* This file has been released into the public domain by the author. */

/* COMPILE WITH: */
/* gcc -DUNIX -O2 -o biddy-example-bra biddy-example-bra.c -I. -L./bin -static -lbiddy -lgmp */

/* MAXNODES IS FOR OBDD AND ZBDD, THEY ARE ALWAYS THE LARGEST AMONG THE IMPLEMENTED BDD TYPES */
/* 0var = 2 functions, 1 ordering, MAXNODES = 1 */
/* 1var = 4 functions, 1 ordering, MAXNODES = 3 */
/* 2var = 16 functions, 2 orderings, MAXNODES = 5 */
/* 3var = 256 functions, 6 orderings, MAXNODES = 7 */
/* 4var = 65536 functions, 24 orderings, MAXNODES = 11 */
/* 5var = 4294967296 functions, 120 orderings, MAXNODES = 19 */
#define MAXNODES 19

/* for statall() you must define COUNTFUNCTIONS or/and COUNTNODES */
/* for 5 variables and more it is not possible to store nodes for all functions */
#define COUNTFUNCTIONS
#define COUNTNODES

#include <string.h>
#include <time.h>
#include "biddy.h"

void statAll(unsigned int N);
void statMinMax(unsigned int N);
void statByFile(Biddy_String bddtype, unsigned int N);
void sjt1(Biddy_Manager MNG, unsigned int fidx,
          unsigned long long int *MINCE, unsigned long long int *MAXCE, unsigned long long int *BASECE, unsigned long long int *SUMCE,
          unsigned long long int *MIN, unsigned long long int *MAX, unsigned long long int *BASE, unsigned long long int *SUM,
          Biddy_Boolean usece);
void sjt2(Biddy_Manager MNG, unsigned int fidx,
          unsigned int *MINCE, unsigned int *MAXCE, unsigned int *SUMCE,
          unsigned int *MIN, unsigned int *MAX, unsigned int *SUM,
          Biddy_Boolean usece);
void sjt_init(Biddy_Manager MNG);
void sjt_exit(Biddy_Manager MNG);
Biddy_Boolean sjt_step(Biddy_Manager MNG);
void sjt_print(Biddy_Manager MNG, Biddy_Edge f);

void results1(Biddy_Manager MNG, unsigned long long int *MIN, unsigned long long int *MAX, unsigned long long int *BASE, unsigned long long int *ALL, unsigned long long int nf, unsigned long long int nc);
void results2a(Biddy_Manager MNG, unsigned int *MIN, unsigned int *MAX, unsigned int *SUM, unsigned long long int nf, unsigned long long int nc);
void results2b(Biddy_Manager MNG1, unsigned int *MIN1, Biddy_Manager MNG2, unsigned int *MIN2, unsigned long long int nf);

unsigned long long int rnd8(unsigned long long int x);
unsigned long long int rnd16(unsigned long long int x);
unsigned long long int rnd32(unsigned long long int x);
unsigned long long int rnd8lfsr(unsigned long long int x);
unsigned long long int rnd16lfsr(unsigned long long int x);

void testrnd();
void testswap();
void trim(Biddy_String source);

typedef struct {
  unsigned int value;
  Biddy_Boolean direction;
} userdata;

int main() {

  /* TEST 1 */
  /*
  testrnd();
  exit(1);
  */

  /* TEST 2 */
  /*
  testswap();
  exit(1);
  */

  /*
  statAll(3);
  */

  /*
  statMinMax(3);
  */
  
  /**/
  statByFile("OBDDCE",5);
  /**/
}

/* ########################################################################## */
/* function statAll */
/* ########################################################################## */

void statAll(unsigned int N) {
  Biddy_String SUPPORT;
  Biddy_Manager MNGOBDD,MNGOBDDCE,MNGZBDD,MNGZBDDCE,MNGTZBDD,MNGTZBDDCE;
  Biddy_Edge s,f;

  /* MINxBDD[n] IS A NUMBER OF FUNCTIONS, */
  /* WHICH HAVE n NODES USING THE BEST ORDERING */
  unsigned long long int *MINOBDD,*MINZBDD,*MINTZBDD;
  unsigned long long int *MINOBDDCE,*MINZBDDCE,*MINTZBDDCE;

  /* MAXxBDD[n] IS A NUMBER OF FUNCTIONS, */
  /* WHICH HAVE n NODES USING THE WORST ORDERING */
  unsigned long long int *MAXOBDD,*MAXZBDD,*MAXTZBDD;
  unsigned long long int *MAXOBDDCE,*MAXZBDDCE,*MAXTZBDDCE;

  /* BASExBDD[n] IS A NUMBER OF FUNCTIONS, */
  /* WHICH HAVE n NODES USING THE BASIC ORDERING a,b,c,... */
  unsigned long long int *BASEOBDD,*BASEZBDD,*BASETZBDD;
  unsigned long long int *BASEOBDDCE,*BASEZBDDCE,*BASETZBDDCE;

  /* ALLxBDD[n] IS A NUMBER OF FUNCTIONS, */
  /* WHICH HAVE n NODES IN ANY OF THE POSSIBLE ORDERINGS */
  unsigned long long int *ALLOBDD,*ALLZBDD,*ALLTZBDD;
  unsigned long long int *ALLOBDDCE,*ALLZBDDCE,*ALLTZBDDCE;

  /* MINNODESxBDD[n] IS A NUMBER OF NODES */
  /* FOR FUNCTION n USING THE BEST ORDERING */
  unsigned int *MINNODESOBDD,*MINNODESZBDD,*MINNODESTZBDD;
  unsigned int *MINNODESOBDDCE,*MINNODESZBDDCE,*MINNODESTZBDDCE;

  /* MAXNODESxBDD[n] IS A NUMBER OF NODES */
  /* FOR FUNCTION n USING THE WORST ORDERING */
  unsigned int *MAXNODESOBDD,*MAXNODESZBDD,*MAXNODESTZBDD;
  unsigned int *MAXNODESOBDDCE,*MAXNODESZBDDCE,*MAXNODESTZBDDCE;

  /* SUMxBDD[n] IS A SUM OF ALL NODES */
  /* FOR FUNCTION n FOR ALL THE POSSIBLE ORDERINGS */
  unsigned int *SUMNODESOBDD,*SUMNODESZBDD,*SUMNODESTZBDD;
  unsigned int *SUMNODESOBDDCE,*SUMNODESZBDDCE,*SUMNODESTZBDDCE;

  unsigned long long int n,nf,nc;
  unsigned int fidx;

  Biddy_InitMNG(&MNGOBDD,BIDDYTYPEOBDD);
  Biddy_InitMNG(&MNGOBDDCE,BIDDYTYPEOBDDC);
  Biddy_InitMNG(&MNGZBDD,BIDDYTYPEZBDD);
  Biddy_InitMNG(&MNGZBDDCE,BIDDYTYPEZBDDC);
  Biddy_InitMNG(&MNGTZBDD,BIDDYTYPETZBDD);
  /* Biddy_InitMNG(&MNGTZBDDCE,BIDDYTYPETZBDDC); */

  n = N;
  nf = 1;
  nc = 1;
  while (n>0) {
    nc = n * nc;
    nf = 2 * nf;
    n--;
  }
  n = nf;
  nf = 1;
  while (n>0) {
    nf = 2 * nf;
    n--;
  }

  if (N == 2) {
    SUPPORT = (Biddy_String) strdup("a*b");
  }
  else if (N == 3) {
    SUPPORT = (Biddy_String) strdup("a*b*c");
  }
  else if (N == 4) {
    SUPPORT = (Biddy_String) strdup("a*b*c*d");
  }
  else if (N == 5) {
    SUPPORT = (Biddy_String) strdup("a*b*c*d*e");
  } else {
    printf("N == %u IS NOT SUPPORTED!\n",N);
    exit(1);
  }

#ifdef COUNTFUNCTIONS
  MINOBDD = (unsigned long long int *) malloc((MAXNODES+1) * sizeof(unsigned long long int));
  MINZBDD = (unsigned long long int *) malloc((MAXNODES+1) * sizeof(unsigned long long int));
  MINTZBDD = (unsigned long long int *) malloc((MAXNODES+1) * sizeof(unsigned long long int));
  MINOBDDCE = (unsigned long long int *) malloc((MAXNODES+1) * sizeof(unsigned long long int));
  MINZBDDCE = (unsigned long long int *) malloc((MAXNODES+1) * sizeof(unsigned long long int));
  MINTZBDDCE = (unsigned long long int *) malloc((MAXNODES+1) * sizeof(unsigned long long int));

  MAXOBDD = (unsigned long long int *) malloc((MAXNODES+1) * sizeof(unsigned long long int));
  MAXZBDD = (unsigned long long int *) malloc((MAXNODES+1) * sizeof(unsigned long long int));
  MAXTZBDD = (unsigned long long int *) malloc((MAXNODES+1) * sizeof(unsigned long long int));
  MAXOBDDCE = (unsigned long long int *) malloc((MAXNODES+1) * sizeof(unsigned long long int));
  MAXZBDDCE = (unsigned long long int *) malloc((MAXNODES+1) * sizeof(unsigned long long int));
  MAXTZBDDCE = (unsigned long long int *) malloc((MAXNODES+1) * sizeof(unsigned long long int));

  BASEOBDD = (unsigned long long int *) malloc((MAXNODES+1) * sizeof(unsigned long long int));
  BASEZBDD = (unsigned long long int *) malloc((MAXNODES+1) * sizeof(unsigned long long int));
  BASETZBDD = (unsigned long long int *) malloc((MAXNODES+1) * sizeof(unsigned long long int));
  BASEOBDDCE = (unsigned long long int *) malloc((MAXNODES+1) * sizeof(unsigned long long int));
  BASEZBDDCE = (unsigned long long int *) malloc((MAXNODES+1) * sizeof(unsigned long long int));
  BASETZBDDCE = (unsigned long long int *) malloc((MAXNODES+1) * sizeof(unsigned long long int));

  ALLOBDD = (unsigned long long int *) malloc((MAXNODES+1) * sizeof(unsigned long long int));
  ALLZBDD = (unsigned long long int *) malloc((MAXNODES+1) * sizeof(unsigned long long int));
  ALLTZBDD = (unsigned long long int *) malloc((MAXNODES+1) * sizeof(unsigned long long int));
  ALLOBDDCE = (unsigned long long int *) malloc((MAXNODES+1) * sizeof(unsigned long long int));
  ALLZBDDCE = (unsigned long long int *) malloc((MAXNODES+1) * sizeof(unsigned long long int));
  ALLTZBDDCE = (unsigned long long int *) malloc((MAXNODES+1) * sizeof(unsigned long long int));

  for (n = 0; n<=MAXNODES; n++) {
    MINOBDD[n] = MAXOBDD[n] = BASEOBDD[n] = ALLOBDD[n] = 0;
    MINZBDD[n] = MAXZBDD[n] = BASEZBDD[n] = ALLZBDD[n] = 0;
    MINTZBDD[n] = MAXTZBDD[n] = BASETZBDD[n] = ALLTZBDD[n] = 0;
    MINOBDDCE[n] = MAXOBDDCE[n] = BASEOBDDCE[n] = ALLOBDDCE[n] = 0;
    MINZBDDCE[n] = MAXZBDDCE[n] = BASEZBDDCE[n] = ALLZBDDCE[n] = 0;
    MINTZBDDCE[n] = MAXTZBDDCE[n] = BASETZBDDCE[n] = ALLTZBDDCE[n] = 0;
  }
#endif

#ifdef COUNTNODES
  if (N < 5) {
    MINNODESOBDD = (unsigned int *) malloc(nf * sizeof(unsigned int));
    MINNODESZBDD = (unsigned int *) malloc(nf * sizeof(unsigned int));
    MINNODESTZBDD = (unsigned int *) malloc(nf * sizeof(unsigned int));
    MINNODESOBDDCE = (unsigned int *) malloc(nf * sizeof(unsigned int));
    MINNODESZBDDCE = (unsigned int *) malloc(nf * sizeof(unsigned int));
    MINNODESTZBDDCE = (unsigned int *) malloc(nf * sizeof(unsigned int));

    MAXNODESOBDD = (unsigned int *) malloc(nf * sizeof(unsigned int));
    MAXNODESZBDD = (unsigned int *) malloc(nf * sizeof(unsigned int));
    MAXNODESTZBDD = (unsigned int *) malloc(nf * sizeof(unsigned int));
    MAXNODESOBDDCE = (unsigned int *) malloc(nf * sizeof(unsigned int));
    MAXNODESZBDDCE = (unsigned int *) malloc(nf * sizeof(unsigned int));
    MAXNODESTZBDDCE = (unsigned int *) malloc(nf * sizeof(unsigned int));

    SUMNODESOBDD = (unsigned int *) malloc(nf * sizeof(unsigned int));
    SUMNODESZBDD = (unsigned int *) malloc(nf * sizeof(unsigned int));
    SUMNODESTZBDD = (unsigned int *) malloc(nf * sizeof(unsigned int));
    SUMNODESOBDDCE = (unsigned int *) malloc(nf * sizeof(unsigned int));
    SUMNODESZBDDCE = (unsigned int *) malloc(nf * sizeof(unsigned int));
    SUMNODESTZBDDCE = (unsigned int *) malloc(nf * sizeof(unsigned int));

    for (n = 0; n<nf; n++) {
      MINNODESOBDD[n] = MAXNODESOBDD[n] = SUMNODESOBDD[n] = 0;
      MINNODESZBDD[n] = MAXNODESZBDD[n] = SUMNODESZBDD[n] = 0;
      MINNODESTZBDD[n] = MAXNODESTZBDD[n] = SUMNODESTZBDD[n] = 0;
      MINNODESOBDDCE[n] = MAXNODESOBDDCE[n] = SUMNODESOBDDCE[n] = 0;
      MINNODESZBDDCE[n] = MAXNODESZBDDCE[n] = SUMNODESZBDDCE[n] = 0;
      MINNODESTZBDDCE[n] = MAXNODESTZBDDCE[n] = SUMNODESTZBDDCE[n] = 0;
    }
  }
#endif
  
  n = 0;
  do {

    /* calculate results for OBDDCE and OBDD */
    s = Biddy_Managed_Eval2(MNGOBDDCE,(Biddy_String)SUPPORT);
    f = Biddy_Managed_CreateFunction(MNGOBDDCE,s,n);
    fidx = Biddy_Managed_AddTmpFormula(MNGOBDDCE,"F",f);
#ifdef COUNTFUNCTIONS
    sjt1(MNGOBDDCE,fidx,
         MINOBDDCE,MAXOBDDCE,BASEOBDDCE,ALLOBDDCE,
         MINOBDD,MAXOBDD,BASEOBDD,ALLOBDD,TRUE);
#endif
#ifdef COUNTNODES
    if (N < 5) {
      sjt2(MNGOBDDCE,fidx,
           &MINNODESOBDDCE[n],&MAXNODESOBDDCE[n],&SUMNODESOBDDCE[n],
           &MINNODESOBDD[n],&MAXNODESOBDD[n],&SUMNODESOBDD[n],TRUE);
    }
#endif

    /* calculate results for ZBDDCE and ZBDD */
    s = Biddy_Managed_Eval2(MNGZBDDCE,(Biddy_String)SUPPORT);
    f = Biddy_Managed_CreateFunction(MNGZBDDCE,s,n);
    fidx = Biddy_Managed_AddTmpFormula(MNGZBDDCE,"F",f);
#ifdef COUNTFUNCTIONS
    sjt1(MNGZBDDCE,fidx,
         MINZBDDCE,MAXZBDDCE,BASEZBDDCE,ALLZBDDCE,
         MINZBDD,MAXZBDD,BASEZBDD,ALLZBDD,TRUE);
#endif
#ifdef COUNTNODES
    if (N < 5) {
      sjt2(MNGZBDDCE,fidx,
           &MINNODESZBDDCE[n],&MAXNODESZBDDCE[n],&SUMNODESZBDDCE[n],
           &MINNODESZBDD[n],&MAXNODESZBDD[n],&SUMNODESZBDD[n],TRUE);
    }
#endif

    /* calculate results for TZBDDCE and TZBDD */
    /* because TZBDDCE is not supported yet, this is disabled */
    /*
    s = Biddy_Managed_Eval2(MNGTZBDDCE,(Biddy_String)SUPPORT);
    f = Biddy_Managed_CreateFunction(MNGTZBDDCE,s,n);
    fidx = Biddy_Managed_AddTmpFormula(MNGTZBDDCE,"F",f);
#ifdef COUNTFUNCTIONS
    sjt1(MNGTZBDDCE,fidx,
         MINTZBDDCE,MAXTZBDDCE,BASETZBDDCE,ALLTZBDDCE,
         MINTZBDD,MAXTZBDD,BASETZBDD,ALLTZBDD,TRUE);
#endif
#ifdef COUNTNODES
    if (N < 5) {
      sjt2(MNGTZBDDCE,fidx,
           &MINNODESTZBDDCE[n],&MAXNODESTZBDDCE[n],&SUMNODESTZBDDCE[n],
           &MINNODESTZBDD[n],&MAXNODESTZBDD[n],&SUMNODESTZBDD[n],TRUE);
    }
#endif
    */

    /* calculate results for OBDD only */
    /*
    s = Biddy_Managed_Eval2(MNGOBDD,(Biddy_String)SUPPORT);
    f = Biddy_Managed_CreateFunction(MNGOBDD,s,n);
    fidx = Biddy_Managed_AddTmpFormula(MNGOBDD,"F",f);
#ifdef COUNTFUNCTIONS
    sjt1(MNGOBDD,fidx,
         NULL,NULL,NULL,NULL,
         MINOBDD,MAXOBDD,BASEOBDD,ALLOBDD,FALSE);
#endif
#ifdef COUNTNODES
    if (N < 5) {
      sjt2(MNGOBDD,fidx,
           NULL,NULL,NULL,
           &MINNODESOBDD[n],&MAXNODESOBDD[n],&SUMNODESOBDD[n],FALSE);
    }
#endif
    */

    /* calculate results for ZBDD only */
    /*
    s = Biddy_Managed_Eval2(MNGZBDD,(Biddy_String)SUPPORT);
    f = Biddy_Managed_CreateFunction(MNGZBDD,s,n);
    fidx = Biddy_Managed_AddTmpFormula(MNGZBDD,"F",f);
#ifdef COUNTFUNCTIONS
    sjt1(MNGTZBDD,fidx,
         NULL,NULL,NULL,NULL,
         MINZBDD,MAXZBDD,BASEZBDD,ALLZBDD,FALSE);
#endif
#ifdef COUNTNODES
    if (N < 5) {
      sjt2(MNGTZBDD,fidx,
           NULL,NULL,NULL,
           &MINNODESZBDD[n],&MAXNODESZBDD[n],&SUMNODESZBDD[n],FALSE);
    }
#endif
    */

    /* calculate results for TZBDD only */
    s = Biddy_Managed_Eval2(MNGTZBDD,(Biddy_String)SUPPORT);
    f = Biddy_Managed_CreateFunction(MNGTZBDD,s,n);
    fidx = Biddy_Managed_AddTmpFormula(MNGTZBDD,"F",f);
#ifdef COUNTFUNCTIONS
    sjt1(MNGTZBDD,fidx,
         NULL,NULL,NULL,NULL,
         MINTZBDD,MAXTZBDD,BASETZBDD,ALLTZBDD,FALSE);
#endif
#ifdef COUNTNODES
    if (N < 5) {
      sjt2(MNGTZBDD,fidx,
           NULL,NULL,NULL,
           &MINNODESTZBDD[n],&MAXNODESTZBDD[n],&SUMNODESTZBDD[n],FALSE);
    }
#endif

    if (N == 2) {
      if (++n == 16) n = 0;
    }
    else if (N == 3) {
      n = rnd8(n);
    }
    else if (N == 4) {
      n = rnd16(n);
    }
    else if (N == 5) {
      n = rnd32(n);
    } else {
      printf("N == %u IS NOT SUPPORTED!\n",N);
      exit(1);
    }

  } while (n != 0);

#ifdef COUNTFUNCTIONS
  /* SUMMARY ABOUT THE SIZE */
  /* THESE RESULTS ARE COLLECTED BY sjt1 */
  printf("RESULTS WITHOUT COMPLEMENTED EDGES\n");
  results1(MNGOBDD,MINOBDD,MAXOBDD,BASEOBDD,ALLOBDD,nf,nc);
  results1(MNGZBDD,MINZBDD,MAXZBDD,BASEZBDD,ALLZBDD,nf,nc);
  results1(MNGTZBDD,MINTZBDD,MAXTZBDD,BASETZBDD,ALLTZBDD,nf,nc);
  printf("\n");

  /* SUMMARY ABOUT THE SIZE */
  /* THESE RESULTS ARE COLLECTED BY sjt1 */
  printf("RESULTS WITH COMPLEMENTED EDGES\n");
  results1(MNGOBDDCE,MINOBDDCE,MAXOBDDCE,BASEOBDDCE,ALLOBDDCE,nf,nc);
  results1(MNGZBDDCE,MINZBDDCE,MAXZBDDCE,BASEZBDDCE,ALLZBDDCE,nf,nc);
  printf("\n");
#endif

#ifdef COUNTNODES
  if (N < 5) {
    /* SOME DETAILS ABOUT THE SIZE */
    /* THESE RESULTS ARE COLLECTED BY sjt2 */
    printf("RESULTS WITHOUT COMPLEMENTED EDGES\n");
    results2a(MNGOBDD,MINNODESOBDD,MAXNODESOBDD,SUMNODESOBDD,nf,nc);
    results2a(MNGZBDD,MINNODESZBDD,MAXNODESZBDD,SUMNODESZBDD,nf,nc);
    results2a(MNGTZBDD,MINNODESTZBDD,MAXNODESTZBDD,SUMNODESTZBDD,nf,nc);
    printf("\n");

    /* STATS ABOUT THE SIZE */
    /* THESE RESULTS ARE COLLECTED BY sjt2 */
    printf("RESULTS WITH COMPLEMENTED EDGES\n");
    results2a(MNGOBDDCE,MINNODESOBDDCE,MAXNODESOBDDCE,SUMNODESOBDDCE,nf,nc);
    results2a(MNGZBDDCE,MINNODESZBDDCE,MAXNODESZBDDCE,SUMNODESZBDDCE,nf,nc);
    printf("\n");
  }
#endif

#ifdef COUNTNODES
  if (N < 5) {
    /* COMPARE OBDD AND ZBDD */
    /* THESE RESULTS ARE COLLECTED BY sjt2 */
    results2b(MNGOBDD,MINNODESOBDD,MNGZBDD,MINNODESZBDD,nf);
    printf("\n");

    /* COMPARE OBDD AND TZBDD */
    /* THESE RESULTS ARE COLLECTED BY sjt2 */
    results2b(MNGOBDD,MINNODESOBDD,MNGTZBDD,MINNODESTZBDD,nf);
    printf("\n");

    /* COMPARE ZBDD AND TZBDD */
    /* THESE RESULTS ARE COLLECTED BY sjt2 */
    results2b(MNGZBDD,MINNODESZBDD,MNGTZBDD,MINNODESTZBDD,nf);
    printf("\n");
  }
#endif

  free(SUPPORT);

#ifdef COUNTFUNCTIONS
  free(MINOBDD);
  free(MINZBDD);
  free(MINTZBDD);
  free(MINOBDDCE);
  free(MINZBDDCE);
  free(MINTZBDDCE);

  free(MAXOBDD);
  free(MAXZBDD);
  free(MAXTZBDD);
  free(MAXOBDDCE);
  free(MAXZBDDCE);
  free(MAXTZBDDCE);

  free(BASEOBDD);
  free(BASEZBDD);
  free(BASETZBDD);
  free(BASEOBDDCE);
  free(BASEZBDDCE);
  free(BASETZBDDCE);

  free(ALLOBDD);
  free(ALLZBDD);
  free(ALLTZBDD);
  free(ALLOBDDCE);
  free(ALLZBDDCE);
  free(ALLTZBDDCE);
#endif

#ifdef COUNTNODES
  if (N < 5) {
    free(MINNODESOBDD);
    free(MINNODESZBDD);
    free(MINNODESTZBDD);
    free(MINNODESOBDDCE);
    free(MINNODESZBDDCE);
    free(MINNODESTZBDDCE);

    free(MAXNODESOBDD);
    free(MAXNODESZBDD);
    free(MAXNODESTZBDD);
    free(MAXNODESOBDDCE);
    free(MAXNODESZBDDCE);
    free(MAXNODESTZBDDCE);

    free(SUMNODESOBDD);
    free(SUMNODESZBDD);
    free(SUMNODESTZBDD);
    free(SUMNODESOBDDCE);
    free(SUMNODESZBDDCE);
    free(SUMNODESTZBDDCE);
  }
#endif
  
  Biddy_ExitMNG(&MNGOBDD);
  Biddy_ExitMNG(&MNGOBDDCE);
  Biddy_ExitMNG(&MNGZBDD);
  Biddy_ExitMNG(&MNGZBDDCE);
  Biddy_ExitMNG(&MNGTZBDD);
  /* Biddy_ExitMNG(&MNGTZBDDCE); */ /* not supported, yet */
}

/* ########################################################################## */
/* function statMinMax */
/* ########################################################################## */

void statMinMax(unsigned int N) {
  Biddy_String SUPPORT;
  Biddy_Manager MNGOBDD,MNGOBDDCE,MNGZBDD,MNGZBDDCE,MNGTZBDD,MNGTZBDDCE;
  Biddy_Manager MNG;
  Biddy_Edge s,f;
  Biddy_Edge sobdd,sobddce,szbddce,stzbdd;
  unsigned long long int n,nf,nc;
  unsigned int fidx,num;

  /* MINxBDD[n] IS A NUMBER OF FUNCTIONS, */
  /* WHICH HAVE n NODES USING THE BEST ORDERING */
  unsigned long long int *MINOBDD,*MINZBDD,*MINTZBDD;
  unsigned long long int *MINOBDDCE,*MINZBDDCE,*MINTZBDDCE;

  /* MAXxBDD[n] IS A NUMBER OF FUNCTIONS, */
  /* WHICH HAVE n NODES USING THE WORST ORDERING */
  unsigned long long int *MAXOBDD,*MAXZBDD,*MAXTZBDD;
  unsigned long long int *MAXOBDDCE,*MAXZBDDCE,*MAXTZBDDCE;

  Biddy_InitMNG(&MNGOBDD,BIDDYTYPEOBDD);
  Biddy_InitMNG(&MNGOBDDCE,BIDDYTYPEOBDDC);
  Biddy_InitMNG(&MNGZBDD,BIDDYTYPEZBDD);
  Biddy_InitMNG(&MNGZBDDCE,BIDDYTYPEZBDDC);
  Biddy_InitMNG(&MNGTZBDD,BIDDYTYPETZBDD);
  /* Biddy_InitMNG(&MNGTZBDDCE,BIDDYTYPETZBDDC); */

  /* TESTING */
  /*
  szbddce = Biddy_Managed_Eval2(MNGZBDDCE,(Biddy_String)"a+!b");
  fidx = Biddy_Managed_AddTmpFormula(MNGZBDDCE,"F",szbddce);
  f = Biddy_Managed_GetIthFormula(MNGZBDDCE,fidx);
  num = Biddy_Managed_CountNodesPlain(MNGZBDDCE,f);
  printf("THERE ARE %u PLAIN NODES.\n",num);
  Biddy_Managed_PrintfBDD(MNGZBDDCE,f);
  Biddy_Managed_MinimizeBDD(MNGZBDDCE,"F");
  f = Biddy_Managed_GetIthFormula(MNGZBDDCE,fidx);
  num = Biddy_Managed_CountNodesPlain(MNGZBDDCE,f);
  printf("THERE ARE %u MIN PLAIN NODES.\n",num);
  Biddy_Managed_PrintfBDD(MNGZBDDCE,f);
  Biddy_Managed_MaximizeBDD(MNGZBDDCE,"F");
  f = Biddy_Managed_GetIthFormula(MNGZBDDCE,fidx);
  num = Biddy_Managed_CountNodesPlain(MNGZBDDCE,f);
  printf("THERE ARE %u MAX PLAIN NODES.\n",num);
  Biddy_Managed_PrintfBDD(MNGZBDDCE,f);
  exit(1);
  */

  n = N;
  nf = 1;
  while (n>0) {
    nf = 2 * nf;
    n--;
  }
  n = nf;
  nf = 1;
  while (n>0) {
    nf = 2 * nf;
    n--;
  }

  if (N == 2) {
    SUPPORT = (Biddy_String) strdup("a*b");
  }
  else if (N == 3) {
    SUPPORT = (Biddy_String) strdup("a*b*c");
  }
  else if (N == 4) {
    SUPPORT = (Biddy_String) strdup("a*b*c*d");
  }
  else if (N == 5) {
    SUPPORT = (Biddy_String) strdup("a*b*c*d*e");
  } else {
    printf("N == %u IS NOT SUPPORTED!\n",N);
    exit(1);
  }

  MINOBDD = (unsigned long long int *) malloc((MAXNODES+1) * sizeof(unsigned long long int));
  MINZBDD = (unsigned long long int *) malloc((MAXNODES+1) * sizeof(unsigned long long int));
  MINTZBDD = (unsigned long long int *) malloc((MAXNODES+1) * sizeof(unsigned long long int));
  MINOBDDCE = (unsigned long long int *) malloc((MAXNODES+1) * sizeof(unsigned long long int));
  MINZBDDCE = (unsigned long long int *) malloc((MAXNODES+1) * sizeof(unsigned long long int));
  MINTZBDDCE = (unsigned long long int *) malloc((MAXNODES+1) * sizeof(unsigned long long int));

  MAXOBDD = (unsigned long long int *) malloc((MAXNODES+1) * sizeof(unsigned long long int));
  MAXZBDD = (unsigned long long int *) malloc((MAXNODES+1) * sizeof(unsigned long long int));
  MAXTZBDD = (unsigned long long int *) malloc((MAXNODES+1) * sizeof(unsigned long long int));
  MAXOBDDCE = (unsigned long long int *) malloc((MAXNODES+1) * sizeof(unsigned long long int));
  MAXZBDDCE = (unsigned long long int *) malloc((MAXNODES+1) * sizeof(unsigned long long int));
  MAXTZBDDCE = (unsigned long long int *) malloc((MAXNODES+1) * sizeof(unsigned long long int));

  for (n = 0; n<=MAXNODES; n++) {
    MINOBDD[n] = MAXOBDD[n] = 0;
    MINZBDD[n] = MAXZBDD[n] = 0;
    MINTZBDD[n] = MAXTZBDD[n] = 0;
    MINOBDDCE[n] = MAXOBDDCE[n] = 0;
    MINZBDDCE[n] = MAXZBDDCE[n] = 0;
    MINTZBDDCE[n] = MAXTZBDDCE[n] = 0;
  }

  sobdd = Biddy_Managed_Eval2(MNGOBDD,(Biddy_String)SUPPORT);
  sobddce = Biddy_Managed_Eval2(MNGOBDDCE,(Biddy_String)SUPPORT);
  szbddce = Biddy_Managed_Eval2(MNGZBDDCE,(Biddy_String)SUPPORT);
  stzbdd = Biddy_Managed_Eval2(MNGTZBDD,(Biddy_String)SUPPORT);

  nc = 0;  
  n = 0;
  do {

    /*
    printf("(%llu#%llu/%llu)",n,++nc,nf);
    */

    /* calculate results for OBDD */
    f = Biddy_Managed_CreateFunction(MNGOBDD,sobdd,n);
    Biddy_InitMNG(&MNG,BIDDYTYPEOBDD);
    f = Biddy_Managed_Copy(MNGOBDD,MNG,f);
    fidx = Biddy_Managed_AddTmpFormula(MNG,"F",f);
    Biddy_Managed_MinimizeBDD(MNG,"F");
    f = Biddy_Managed_GetIthFormula(MNG,fidx);
    num = Biddy_Managed_CountNodes(MNG,f);
    MINOBDD[num]++;
    Biddy_Managed_MaximizeBDD(MNG,"F");
    f = Biddy_Managed_GetIthFormula(MNG,fidx);
    num = Biddy_Managed_CountNodes(MNG,f);
    MAXOBDD[num]++;
    Biddy_ExitMNG(&MNG);

    /* calculate results for OBDDCE */
    f = Biddy_Managed_CreateFunction(MNGOBDDCE,sobddce,n);
    Biddy_InitMNG(&MNG,BIDDYTYPEOBDDC);
    f = Biddy_Managed_Copy(MNGOBDDCE,MNG,f);
    fidx = Biddy_Managed_AddTmpFormula(MNG,"F",f);
    Biddy_Managed_MinimizeBDD(MNG,"F");
    f = Biddy_Managed_GetIthFormula(MNG,fidx);
    num = Biddy_Managed_CountNodes(MNG,f);
    MINOBDDCE[num]++;
    Biddy_Managed_MaximizeBDD(MNG,"F");
    f = Biddy_Managed_GetIthFormula(MNG,fidx);
    num = Biddy_Managed_CountNodes(MNG,f);
    MAXOBDDCE[num]++;
    Biddy_ExitMNG(&MNG);

    /* calculate results for ZBDDCE */
    f = Biddy_Managed_CreateFunction(MNGZBDDCE,szbddce,n);
    Biddy_InitMNG(&MNG,BIDDYTYPEZBDDC);
    f = Biddy_Managed_Copy(MNGZBDDCE,MNG,f);
    fidx = Biddy_Managed_AddTmpFormula(MNG,"F",f);
    Biddy_Managed_MinimizeBDD(MNG,"F");
    f = Biddy_Managed_GetIthFormula(MNG,fidx);
    num = Biddy_Managed_CountNodes(MNG,f);
    MINZBDDCE[num]++;
    Biddy_Managed_MaximizeBDD(MNG,"F");
    f = Biddy_Managed_GetIthFormula(MNG,fidx);
    num = Biddy_Managed_CountNodes(MNG,f);
    MAXZBDDCE[num]++;
    Biddy_ExitMNG(&MNG);

    /* calculate results for TZBDD */
    f = Biddy_Managed_CreateFunction(MNGTZBDD,stzbdd,n);
    Biddy_InitMNG(&MNG,BIDDYTYPETZBDD);
    f = Biddy_Managed_Copy(MNGTZBDD,MNG,f);
    fidx = Biddy_Managed_AddTmpFormula(MNG,"F",f);
    Biddy_Managed_MinimizeBDD(MNG,"F");
    f = Biddy_Managed_GetIthFormula(MNG,fidx);
    num = Biddy_Managed_CountNodes(MNG,f);
    MINTZBDD[num]++;
    Biddy_Managed_MaximizeBDD(MNG,"F");
    f = Biddy_Managed_GetIthFormula(MNG,fidx);
    num = Biddy_Managed_CountNodes(MNG,f);
    MAXTZBDD[num]++;
    Biddy_ExitMNG(&MNG);

    if (N == 2) {
      if (++n == 16) n = 0;
    }
    else if (N == 3) {
      n = rnd8(n);
    }
    else if (N == 4) {
      n = rnd16(n);
    }
    else if (N == 5) {
      n = rnd32(n);
    } else {
      printf("N == %u IS NOT SUPPORTED!\n",N);
      exit(1);
    }

  } while (n != 0);

  /* printf("\n"); */
  printf("%s\n",Biddy_Managed_GetManagerName(MNGOBDD));
  for (num = 0; num<=MAXNODES; num++) {
    if ((MINOBDD[num]!=0) || (MAXOBDD[num]!=0))
      printf("NODES=%.2u, MIN = %llu, MAX = %llu\n",
             num,MINOBDD[num],MAXOBDD[num]);
  }
  printf("%s\n",Biddy_Managed_GetManagerName(MNGTZBDD));
  for (num = 0; num<=MAXNODES; num++) {
    if ((MINTZBDD[num]!=0) || (MAXTZBDD[num]!=0))
      printf("NODES=%.2u, MIN = %llu, MAX = %llu\n",
             num,MINTZBDD[num],MAXTZBDD[num]);
  }
  printf("%s\n",Biddy_Managed_GetManagerName(MNGOBDDCE));
  for (num = 0; num<=MAXNODES; num++) {
    if ((MINOBDDCE[num]!=0) || (MAXOBDDCE[num]!=0))
      printf("NODES=%.2u, MIN = %llu, MAX = %llu\n",
             num,MINOBDDCE[num],MAXOBDDCE[num]);
  }
  printf("%s\n",Biddy_Managed_GetManagerName(MNGZBDDCE));
  for (num = 0; num<=MAXNODES; num++) {
    if ((MINZBDDCE[num]!=0) || (MAXZBDDCE[num]!=0))
      printf("NODES=%.2u, MIN = %llu, MAX = %llu\n",
             num,MINZBDDCE[num],MAXZBDDCE[num]);
  }

  free(SUPPORT);

  free(MINOBDD);
  free(MINZBDD);
  free(MINTZBDD);
  free(MINOBDDCE);
  free(MINZBDDCE);
  free(MINTZBDDCE);

  free(MAXOBDD);
  free(MAXZBDD);
  free(MAXTZBDD);
  free(MAXOBDDCE);
  free(MAXZBDDCE);
  free(MAXTZBDDCE);

  printf("Manager %s finished with %u nodes\n",Biddy_Managed_GetManagerName(MNGOBDD),Biddy_Managed_NodeTableNum(MNGOBDD));
  printf("Manager %s finished with %u nodes\n",Biddy_Managed_GetManagerName(MNGOBDDCE),Biddy_Managed_NodeTableNum(MNGOBDDCE));
  printf("Manager %s finished with %u nodes\n",Biddy_Managed_GetManagerName(MNGZBDDCE),Biddy_Managed_NodeTableNum(MNGZBDDCE));
  printf("Manager %s finished with %u nodes\n",Biddy_Managed_GetManagerName(MNGTZBDD),Biddy_Managed_NodeTableNum(MNGTZBDD));

  Biddy_ExitMNG(&MNGOBDD);
  Biddy_ExitMNG(&MNGOBDDCE);
  Biddy_ExitMNG(&MNGZBDD);
  Biddy_ExitMNG(&MNGZBDDCE);
  Biddy_ExitMNG(&MNGTZBDD);
  /* Biddy_ExitMNG(&MNGTZBDDCE); */ /* not supported, yet */
}

/* ########################################################################## */
/* function statByFile */
/* ########################################################################## */

void statByFile(Biddy_String bddtype, unsigned int N) {
  FILE *fd;
  char filename[32];
  Biddy_Manager MNG;
  Biddy_String SUPPORT;
  Biddy_String buffer,word;
  unsigned int i,varnum,fidx;
  unsigned long long int n,nf,nc,quant,rndseed,c;
  unsigned int tts;
  Biddy_Edge s,f;
  Biddy_Boolean usece;
  clock_t elapsedtime;

  /* MIN[n] IS A NUMBER OF FUNCTIONS, */
  /* WHICH HAVE n NODES USING THE BEST ORDERING */
  unsigned long long int *MIN,*MINCE;

  /* MAX[n] IS A NUMBER OF FUNCTIONS, */
  /* WHICH HAVE n NODES USING THE WORST ORDERING */
  unsigned long long int *MAX,*MAXCE;

  /* BASE[n] IS A NUMBER OF FUNCTIONS, */
  /* WHICH HAVE n NODES USING THE BASIC ORDERING a,b,c,... */
  unsigned long long int *BASE,*BASECE;

  /* ALL[n] IS A NUMBER OF FUNCTIONS, */
  /* WHICH HAVE n NODES IN ANY OF THE POSSIBLE ORDERINGS */
  unsigned long long int *ALL,*ALLCE;

  MIN = (unsigned long long int *) malloc((MAXNODES+1) * sizeof(unsigned long long int));
  MAX = (unsigned long long int *) malloc((MAXNODES+1) * sizeof(unsigned long long int));
  BASE = (unsigned long long int *) malloc((MAXNODES+1) * sizeof(unsigned long long int));
  ALL = (unsigned long long int *) malloc((MAXNODES+1) * sizeof(unsigned long long int));

  MINCE = (unsigned long long int *) malloc((MAXNODES+1) * sizeof(unsigned long long int));
  MAXCE = (unsigned long long int *) malloc((MAXNODES+1) * sizeof(unsigned long long int));
  BASECE = (unsigned long long int *) malloc((MAXNODES+1) * sizeof(unsigned long long int));
  ALLCE = (unsigned long long int *) malloc((MAXNODES+1) * sizeof(unsigned long long int));

  n = N;
  nf = 1;
  while (n>0) {
    nf = 2 * nf;
    n--;
  }
  n = nf;
  nf = 1;
  while (n>0) {
    nf = 2 * nf;
    n--;
  }
  varnum = N;
  n = 0;
  quant = 0;
  rndseed = 0;
  tts = 0;

  /* READ FILE */
  sprintf((char *)filename,"STAT-%s-%.2u.TXT",bddtype,varnum);
  buffer = (Biddy_String) malloc(255);
  fd = fopen(filename,"r");
  if (fd) {

    if (!fgets(buffer,255,fd)) exit(255);
    trim(buffer); /* to assure Win/Unix EOL independecy */
    if (strcmp(bddtype,buffer)) {
      printf("Wrong input format (BDDTYPE)!\n");
      printf("%s\n",buffer);
      exit(1);
    }
    printf("BDD TYPE: <%s>\n",bddtype);

    if (!fgets(buffer,255,fd)) exit(255);
    trim(buffer); /* to assure Win/Unix EOL independency */
    word = strtok(buffer," ");
    if (strcmp(word,"SIZE")) {
      printf("Wrong input format (SIZE)!\n");
      printf("%s\n",buffer);
      exit(1);
    }
    word = strtok(NULL," ");
    sscanf(word,"%u",&varnum);
    printf("SIZE: <%u>\n",varnum);

    if (!fgets(buffer,255,fd)) exit(255);
    trim(buffer); /* to assure Win/Unix EOL independency */
    word = strtok(buffer," ");
    if (strcmp(word,"NF")) {
      printf("Wrong input format (NF)!\n");
      printf("%s\n",buffer);
      exit(1);
    }
    word = strtok(NULL," ");
    sscanf(word,"%llu",&nf);
    printf("NF: <%llu>\n",nf);

    if (!fgets(buffer,255,fd)) exit(255);
    trim(buffer); /* to assure Win/Unix EOL independency */
    word = strtok(buffer," ");
    if (strcmp(word,"N")) {
      printf("Wrong input format (N)!\n");
      printf("%s\n",buffer);
      exit(1);
    }
    word = strtok(NULL," ");
    sscanf(word,"%llu",&n);
    printf("N: <%llu>\n",n);

    if (!fgets(buffer,255,fd)) exit(255);
    trim(buffer); /* to assure Win/Unix EOL independency */
    word = strtok(buffer," ");
    if (strcmp(word,"QUANT")) {
      printf("Wrong input format (QUANT)!\n");
      printf("%s\n",buffer);
      exit(1);
    }
    word = strtok(NULL," ");
    sscanf(word,"%llu",&quant);
    printf("QUANT: <%llu>\n",quant);

    if (!fgets(buffer,255,fd)) exit(255);
    trim(buffer); /* to assure Win/Unix EOL independency */
    word = strtok(buffer," ");
    if (strcmp(word,"SEED")) {
      printf("Wrong input format (SEED)!\n");
      printf("%s\n",buffer);
      exit(1);
    }
    word = strtok(NULL," ");
    sscanf(word,"%llu",&rndseed);
    printf("SEED: <%llu>\n",rndseed);

    if (!fgets(buffer,255,fd)) exit(255);
    trim(buffer); /* to assure Win/Unix EOL independency */
    word = strtok(buffer," ");
    if (strcmp(word,"TIME")) {
      printf("Wrong input format (TIME)!\n");
      printf("%s\n",buffer);
      exit(1);
    }
    word = strtok(NULL," ");
    sscanf(word,"%u",&tts);
    printf("TIME [s]: <%u>\n",tts);

    if (!fgets(buffer,255,fd)) exit(255);
    trim(buffer); /* to assure Win/Unix EOL independecy */
    if (strcmp(buffer,"STATSCE")) {
      printf("Wrong input format (STATCE)!\n");
      printf("%s\n",buffer);
      exit(1);
    }

    for (i = 1; i <= MAXNODES; i++) {
      if (!fgets(buffer,255,fd)) exit(255);
      trim(buffer); /* to assure Win/Unix EOL independecy */
      word = strtok(buffer,"= ");
      if (strcmp(word,"NODES")) {
        printf("Wrong input format (NODES)!\n");
        printf("%s\n",buffer);
        exit(1);
      }
      word = strtok(NULL,", ");
      sscanf(word,"%llu",&c);
      if (c != i) {
        printf("Wrong input format (i=%u, c=%llu)!\n",i,c);
        printf("%s\n",buffer);
        exit(1);
      }
      word = strtok(NULL,"= ");
      if (strcmp(word,"MINCE")) {
        printf("Wrong input format (MINCE)!\n");
        printf("%s\n",buffer);
        exit(1);
      }
      word = strtok(NULL,", ");
      sscanf(word,"%llu",&MINCE[i]);
      word = strtok(NULL,"= ");
      if (strcmp(word,"MAXCE")) {
        printf("Wrong input format (MAXCE)!\n");
        printf("%s\n",buffer);
        exit(1);
      }
      word = strtok(NULL,", ");
      sscanf(word,"%llu",&MAXCE[i]);
      word = strtok(NULL,"= ");
      if (strcmp(word,"BASECE")) {
        printf("Wrong input format (BASECE)!\n");
        printf("%s\n",buffer);
        exit(1);
      }
      word = strtok(NULL,", ");
      sscanf(word,"%llu",&BASECE[i]);
      word = strtok(NULL,"= ");
      if (strcmp(word,"ALLCE")) {
        printf("Wrong input format (ALLCE)!\n");
        printf("%s\n",buffer);
        exit(1);
      }
      word = strtok(NULL,", ");
      sscanf(word,"%llu",&ALLCE[i]);
    }

    if (!fgets(buffer,255,fd)) exit(255);
    trim(buffer); /* to assure Win/Unix EOL independecy */
    if (strcmp(buffer,"STATS")) {
      printf("Wrong input format!\n");
      printf("%s\n",buffer);
      exit(1);
    }

    for (i = 1; i <= MAXNODES; i++) {
      if (!fgets(buffer,255,fd)) exit(255);
      trim(buffer); /* to assure Win/Unix EOL independecy */
      word = strtok(buffer,"= ");
      if (strcmp(word,"NODES")) {
        printf("Wrong input format (NODES)!\n");
        printf("%s\n",buffer);
        exit(1);
      }
      word = strtok(NULL,", ");
      sscanf(word,"%llu",&c);
      if (c != i) {
        printf("Wrong input format (i=%u, c=%llu)!\n",i,c);
        printf("%s\n",buffer);
        exit(1);
      }
      word = strtok(NULL,"= ");
      if (strcmp(word,"MIN")) {
        printf("Wrong input format (MIN)!\n");
        printf("%s\n",buffer);
        exit(1);
      }
      word = strtok(NULL,", ");
      sscanf(word,"%llu",&MIN[i]);
      word = strtok(NULL,"= ");
      if (strcmp(word,"MAX")) {
        printf("Wrong input format (MAX)!\n");
        printf("%s\n",buffer);
        exit(1);
      }
      word = strtok(NULL,", ");
      sscanf(word,"%llu",&MAX[i]);
      word = strtok(NULL,"= ");
      if (strcmp(word,"BASE")) {
        printf("Wrong input format (BASE)!\n");
        printf("%s\n",buffer);
        exit(1);
      }
      word = strtok(NULL,", ");
      sscanf(word,"%llu",&BASE[i]);
      word = strtok(NULL,"= ");
      if (strcmp(word,"ALL")) {
        printf("Wrong input format (ALL)!\n");
        printf("%s\n",buffer);
        exit(1);
      }
      word = strtok(NULL,", ");
      sscanf(word,"%llu",&ALL[i]);
    }

    fclose(fd);
    free(buffer);

    /* CREATE BACKUP FILE */
    fd = fopen("STAT-BACKUP.TXT","w");
    if (!fd) return;
    fprintf(fd,"%s\n",bddtype);
    fprintf(fd,"SIZE %u\n",varnum);
    fprintf(fd,"NF %llu\n",nf);
    fprintf(fd,"N %llu\n",n);
    fprintf(fd,"QUANT %llu\n",quant);
    fprintf(fd,"SEED %llu\n",rndseed);
    fprintf(fd,"TIME %u\n",tts);
    fprintf(fd,"STATSCE\n");
    for (i = 1; i <= MAXNODES; i++) {
      fprintf(fd,"NODES=%.2u, MINCE=%llu, MAXCE=%llu, BASECE=%llu, ALLCE=%llu\n",i,MINCE[i],MAXCE[i],BASECE[i],ALLCE[i]);
    }
    fprintf(fd,"STATS\n");
    for (i = 1; i <= MAXNODES; i++) {
      fprintf(fd,"NODES=%.2u, MIN=%llu, MAX=%llu, BASE=%llu, ALL=%llu\n",i,MIN[i],MAX[i],BASE[i],ALL[i]);
    }
    fclose(fd);
    
    /* COLLECT STATS */
    usece = FALSE;
    if (!strcmp(bddtype,"OBDD")) {
      Biddy_InitMNG(&MNG,BIDDYTYPEOBDD);
    }
    else if (!strcmp(bddtype,"OBDDCE")) {
      Biddy_InitMNG(&MNG,BIDDYTYPEOBDDC);
      usece = TRUE;
    }
    else if (!strcmp(bddtype,"ZBDDCE")) {
      Biddy_InitMNG(&MNG,BIDDYTYPEZBDDC);
      usece = TRUE;
    }
    else if (!strcmp(bddtype,"TZBDD")) {
      Biddy_InitMNG(&MNG,BIDDYTYPETZBDD);
    }
    else {
      printf("Unsupported BDD type!\n");
      exit(1);
    }
    if (varnum == 2) {
      SUPPORT = (Biddy_String) strdup("a*b");
    }
    else if (varnum == 3) {
      SUPPORT = (Biddy_String) strdup("a*b*c");
    }
    else if (varnum == 4) {
      SUPPORT = (Biddy_String) strdup("a*b*c*d");
    }
    else if (varnum == 5) {
      SUPPORT = (Biddy_String) strdup("a*b*c*d*e");
    } else {
      printf("SIZE == %llu IS NOT SUPPORTED!\n",n);
      exit(1);
    }
    elapsedtime = 0;
    if (quant) {
      elapsedtime = clock();
      c = quant;
      do {
        s = Biddy_Managed_Eval2(MNG,(Biddy_String)SUPPORT);
        f = Biddy_Managed_CreateFunction(MNG,s,rndseed);
        Biddy_Managed_Clean(MNG);
        fidx = Biddy_Managed_AddTmpFormula(MNG,"F",f);
        sjt1(MNG,fidx,MINCE,MAXCE,BASECE,ALLCE,MIN,MAX,BASE,ALL,usece);
        Biddy_Managed_Clean(MNG);
        Biddy_Managed_Purge(MNG);
        n++;
        c--;
        if (varnum == 2) {
          if (++rndseed == 16) rndseed = 0;
        }
        else if (varnum == 3) {
          rndseed = rnd8(rndseed);
        }
        else if (varnum == 4) {
          rndseed = rnd16(rndseed);
        }
        else if (varnum == 5) {
          rndseed = rnd32(rndseed);
        }
      } while ((rndseed != 0) && (c != 0));
      elapsedtime = clock() - elapsedtime;
    }
    tts += (unsigned long long int)(0.5+(((double)elapsedtime)/((double)CLOCKS_PER_SEC)));
    if (rndseed == 0) {
      printf("CALCULATION COMPLETE!\n");
      quant = 0;
    } else {
      printf("CALCULATION FINISHED (n=%llu/%llu/%.2f%%)\n",n,nf,(100.0*((double)n)/(double)nf));
    }

    /*
    Biddy_Managed_PrintInfo(MNG,stdout);
    */

    Biddy_ExitMNG(&MNG);
    free(SUPPORT);

  }  /* if CONFIG FILE EXIST */

  /* WRITE FILE */
  fd = fopen(filename,"w");
  if (!fd) return;
  fprintf(fd,"%s\n",bddtype);
  fprintf(fd,"SIZE %u\n",varnum);
  fprintf(fd,"NF %llu\n",nf);
  fprintf(fd,"N %llu\n",n);
  fprintf(fd,"QUANT %llu\n",quant);
  fprintf(fd,"SEED %llu\n",rndseed);
  fprintf(fd,"TIME %u\n",tts);
  fprintf(fd,"STATSCE\n");
  for (i = 1; i <= MAXNODES; i++) {
    fprintf(fd,"NODES=%.2u, MINCE=%llu, MAXCE=%llu, BASECE=%llu, ALLCE=%llu\n",i,MINCE[i],MAXCE[i],BASECE[i],ALLCE[i]);
  }
  fprintf(fd,"STATS\n");
  for (i = 1; i <= MAXNODES; i++) {
    fprintf(fd,"NODES=%.2u, MIN=%llu, MAX=%llu, BASE=%llu, ALL=%llu\n",i,MIN[i],MAX[i],BASE[i],ALL[i]);
  }
  fclose(fd);
  
  free(MIN);
  free(MAX);
  free(BASE);
  free(ALL);

  free(MINCE);
  free(MAXCE);
  free(BASECE);
  free(ALLCE);
}

void
trim(Biddy_String source)
{
  unsigned int sr_length;

  sr_length = (unsigned int) strlen(source);
  while (sr_length && ((source[sr_length-1] == ' ') ||
                       (source[sr_length-1] == '\t') ||
                       (source[sr_length-1] == 10) ||
                       (source[sr_length-1] == 13)))
  {
    source[sr_length-1] = 0;
    sr_length = (unsigned int) strlen(source);
  }
}

/* ########################################################################## */
/* function sjt1 */
/* generates all variable permutations of function f given by fidx */
/* increase counters MIN and MINCE - the number of nodes required for minimal ordering */
/* increase counters MAX and MAXCE - the number of nodes required for maximal ordering */
/* increase counters BASE and BASECE - the number of nodes required for initial ordering */
/* increase counters ALL and ALLLCE - the number of nodes required for any ordering */
/* ########################################################################## */

void
sjt1(Biddy_Manager MNG, unsigned int fidx,
     unsigned long long int *MINCE, unsigned long long int *MAXCE,
     unsigned long long int *BASECE, unsigned long long int *ALLCE,
     unsigned long long int *MIN, unsigned long long int *MAX,
     unsigned long long int *BASE, unsigned long long int *ALL,
     Biddy_Boolean usece)
{
  Biddy_Edge f;
  unsigned int i,n;
  Biddy_Variable v;
  unsigned int MINCETMP,MAXCETMP;
  unsigned int MINTMP,MAXTMP;
  
  MINTMP = MAXTMP = 0;
  MINCETMP = MAXCETMP = 0;

  sjt_init(MNG);

  i = 0;
  do {

    i++;
    f = Biddy_Managed_GetIthFormula(MNG,fidx);

    /*
    printf("ITERATION %u\n",i);
    sjt_print(MNG,f);
    */

    if (usece) {

      n = Biddy_Managed_CountNodes(MNG,f);

      if (n > MAXNODES) {
        printf("ERROR: MAXNODES MUST BE AT LEAST %u\n",n);
        exit(1);
      }

      if (!MINCETMP || n < MINCETMP) MINCETMP = n;
      if (!MAXCETMP || n > MAXCETMP) MAXCETMP = n;
      if (i == 1) BASECE[n]++;
      ALLCE[n]++;

      n = Biddy_Managed_CountNodesPlain(MNG,f);

      if (n > MAXNODES) {
        printf("ERROR: MAXNODES MUST BE AT LEAST %u\n",n);
        exit(1);
      }

      if (!MINTMP || n < MINTMP) MINTMP = n;
      if (!MAXTMP || n > MAXTMP) MAXTMP = n;
      if (i == 1) BASE[n]++;
      ALL[n]++;

    } else {

      n = Biddy_Managed_CountNodes(MNG,f);

      if (n > MAXNODES) {
        printf("ERROR: MAXNODES MUST BE AT LEAST %u\n",n);
        exit(1);
      }

      if (!MINTMP || n < MINTMP) MINTMP = n;
      if (!MAXTMP || n > MAXTMP) MAXTMP = n;
      if (i == 1) BASE[n]++;
      ALL[n]++;

    }

  } while(sjt_step(MNG));

  sjt_exit(MNG);

  /* restore initial ordering - after sjt only one extra swap is needed */
  v = Biddy_Managed_GetLowestVariable(MNG);
  Biddy_Managed_SwapWithHigher(MNG,v);

  if (usece) {
    MINCE[MINCETMP]++;
    MAXCE[MAXCETMP]++;
    MIN[MINTMP]++;
    MAX[MAXTMP]++;
  } else {
    MIN[MINTMP]++;
    MAX[MAXTMP]++;
  }
}

/* ########################################################################## */
/* function sjt2 */
/* generates all variable permutations of function f given by fidx */
/* counts MIN and MINCE - the number of nodes required for minimal reordering */
/* counts MAX and MAXCE - the number of nodes required for maximal reordering */
/* counts SUM and SUMCE - the sum of nodes required for all reorderings */
/* ########################################################################## */

void
sjt2(Biddy_Manager MNG, unsigned int fidx,
     unsigned int *MINCE, unsigned int *MAXCE, unsigned int *SUMCE,
     unsigned int *MIN, unsigned int *MAX, unsigned int *SUM,
     Biddy_Boolean usece)
{
  Biddy_Edge f;
  unsigned int n;
  Biddy_Variable v;
  unsigned int MINCETMP,MAXCETMP,SUMCETMP;
  unsigned int MINTMP,MAXTMP,SUMTMP;
  
  MINCETMP = MAXCETMP = SUMCETMP = 0;
  MINTMP = MAXTMP = SUMTMP = 0;

  sjt_init(MNG);

  do {

    f = Biddy_Managed_GetIthFormula(MNG,fidx);

    /*
    printf("ITERATION\n");
    sjt_print(MNG,f);
    */

    if (usece) {

      n = Biddy_Managed_CountNodes(MNG,f);
      if (!MINCETMP || n < MINCETMP) MINCETMP = n;
      if (!MAXCETMP || n > MAXCETMP) MAXCETMP = n;
      SUMCETMP += n;

      n = Biddy_Managed_CountNodesPlain(MNG,f);
      if (!MINTMP || n < MINTMP) MINTMP = n;
      if (!MAXTMP || n > MAXTMP) MAXTMP = n;
      SUMTMP += n;

    } else {

      n = Biddy_Managed_CountNodes(MNG,f);
      if (!MINTMP || n < MINTMP) MINTMP = n;
      if (!MAXTMP || n > MAXTMP) MAXTMP = n;
      SUMTMP += n;

    }

  } while(sjt_step(MNG));

  sjt_exit(MNG);

  /* restore initial ordering - after sjt only one extra swap is needed */
  v = Biddy_Managed_GetLowestVariable(MNG);
  Biddy_Managed_SwapWithHigher(MNG,v);

  if (usece) {
    *MINCE = MINCETMP;
    *MAXCE = MAXCETMP;
    *SUMCE = SUMCETMP;
    *MIN = MINTMP;
    *MAX = MAXTMP;
    *SUM = SUMTMP;
  } else {
    if (*MIN) {
      if (*MIN != MINTMP) {
        printf("ERROR: plain number not consistent!\n");
      }
    } else {
      *MIN = MINTMP;
    }
    if (*MAX) {
      if (*MAX != MAXTMP) {
        printf("ERROR: plain number not consistent!\n");
      }
    } else {
      *MAX = MAXTMP;
    }
    if (*SUM) {
      if (*SUM != SUMTMP) {
        printf("ERROR: plain number not consistent!\n");
      }
    } else {
      *SUM = SUMTMP;
    }
  }
}

/* ########################################################################## */
/* SJT */
/* ########################################################################## */

void
sjt_init(Biddy_Manager MNG)
{
  unsigned int i;
  Biddy_Variable v;

  struct userdata {
    unsigned int value;
    Biddy_Boolean direction;
  } *ud;

  /* DEBUGGING */
  /*
  printf("sjt_init %s\n",Biddy_Managed_GetManagerName(MNG));
  */

  v = Biddy_Managed_GetLowestVariable(MNG);
  i = 0;
  do {
    ud = (struct userdata *) malloc(sizeof(struct userdata));
    ud->value = i;
    ud->direction = FALSE; /* FALSE = left = toTop = toLowest, TRUE = right = toBottom = toHighest */
    Biddy_Managed_SetVariableData(MNG,v,(void *) ud);
    v = Biddy_Managed_GetNextVariable(MNG,v);
    i++;
  } while (v != 0);
}

void
sjt_exit(Biddy_Manager MNG)
{
  /* DEBUGGING */
  /*
  printf("sjt_exit %s\n",Biddy_Managed_GetManagerName(MNG));
  */

  Biddy_Managed_ClearVariablesData(MNG);
}

Biddy_Boolean
sjt_step(Biddy_Manager MNG)
{
  Biddy_Variable v,mm;
  userdata *ud1, *ud2;
  unsigned int mmvalue;
  Biddy_Boolean STOP;

  /* DEBUGGING */
  /*
  printf("sjt_step %s\n",Biddy_Managed_GetManagerName(MNG));
  */

  /* find the largest mobile element */
  mm = 0;
  mmvalue = 0;
  STOP = TRUE;
  v = 0;
  do {
    v = Biddy_Managed_GetPrevVariable(MNG,v);
    ud1 = (userdata *) Biddy_Managed_GetVariableData(MNG,v);
    if (ud1->direction && !(Biddy_Managed_IsHighest(MNG,v)))
    {
      ud2 = (userdata *) Biddy_Managed_GetVariableData(MNG,Biddy_Managed_GetNextVariable(MNG,v));
      if (ud1->value > ud2->value)
      {
        STOP = FALSE; /* there exists a mobile element */
        if (ud1->value > mmvalue) {
          mm = v;
          mmvalue = ud1->value;
        }
      }
    }
    if (!(ud1->direction) && !(Biddy_Managed_IsLowest(MNG,v)))
    {
      ud2 = (userdata *) Biddy_Managed_GetVariableData(MNG,Biddy_Managed_GetPrevVariable(MNG,v));
      if (ud1->value > ud2->value)
      {
        STOP = FALSE; /* there exists a mobile element */
        if (ud1->value > mmvalue) {
          mm = v;
          mmvalue = ud1->value;
        }
      }
    }
  } while (!(Biddy_Managed_IsLowest(MNG,v)));

  if (!(STOP)) {
    /* swap mm and the adjacent element it is looking at */
    ud1 = (userdata *) Biddy_Managed_GetVariableData(MNG,mm);
    if (ud1->direction) {
      Biddy_Managed_SwapWithHigher(MNG,mm);
    } else {
      Biddy_Managed_SwapWithLower(MNG,mm);
    }

    /* reverse the direction of all elements larger than mm */
    ud2 = (userdata *) Biddy_Managed_GetVariableData(MNG,mm);
    for (v = 1; v < Biddy_Managed_VariableTableNum(MNG); v++) {
      ud1 = (userdata *) Biddy_Managed_GetVariableData(MNG,v);
      if (ud1->value > ud2->value)
      {
        if (ud1->direction) {
          ud1->direction = FALSE;
        } else {
          ud1->direction = TRUE;
        }
      }
    }
  }

  return !STOP;
}

void sjt_print(Biddy_Manager MNG, Biddy_Edge f)
{
  Biddy_Variable v;

  v = 0;
  do {
    v = Biddy_Managed_GetPrevVariable(MNG,v);
  } while (!(Biddy_Managed_IsLowest(MNG,v)));
  do {
    printf("[%s]",Biddy_Managed_GetVariableName(MNG,v));
    v = Biddy_Managed_GetNextVariable(MNG,v);
  } while (v != 0);
  printf(": ");
  printf("%u depvars, ",Biddy_Managed_DependentVariableNumber(MNG,f));
  printf("%.0f minterms, ",Biddy_Managed_CountMinterms(MNG,f,0));
  printf("%u complemented, ",Biddy_Managed_CountComplementedEdges(MNG,f));
  printf("%u nodes.\n",Biddy_Managed_CountNodes(MNG,f));
  /**/
  printf("#");
  Biddy_Managed_PrintfBDD(MNG,f);
  /**/
}

/* ########################################################################## */
/* RESULTS */
/* ########################################################################## */

void
results1(Biddy_Manager MNG, unsigned long long int *MIN, unsigned long long int *MAX,
         unsigned long long int *BASE, unsigned long long int *ALL,
         unsigned long long int nf, unsigned long long int nc)
{
  unsigned long long int n;

  printf("%s\n",Biddy_Managed_GetManagerName(MNG));
  for (n = 0; n<=MAXNODES; n++) {
    if (ALL[n]) {
      printf("NODES=%.2llu, MIN = %llu, MAX = %llu, BASE = %llu, ALL = %llu (%.2f%%)\n",
             n,MIN[n],MAX[n],BASE[n],ALL[n],(100*((float)ALL[n]/((float)nf*nc))));
    }
  }
}

void
results2a(Biddy_Manager MNG, unsigned int *MIN, unsigned int *MAX, unsigned int *SUM,
          unsigned long long int nf, unsigned long long int nc)
{
  unsigned int MAXDIFF;
  unsigned int MINSUM,MAXSUM;
  double AVGSUM;
  unsigned long long int NMAXDIFF, NMINSUM, NMAXSUM, COMPLETESUM;
  unsigned long long int n;

  MAXDIFF = 0;
  MINSUM = 0;
  MAXSUM = 0;
  AVGSUM = 0.0;
  NMAXDIFF = 0;
  NMINSUM = 0;
  NMAXSUM = 0;
  COMPLETESUM = 0;
  for (n = 0; n<nf; n++) {
    if ((MAX[n]-MIN[n])>=MAXDIFF) {
      MAXDIFF = MAX[n]-MIN[n];
      NMAXDIFF = n;
    }
    if ((!MINSUM) || (SUM[n] <= MINSUM)) {
      MINSUM = SUM[n];
      NMINSUM = n;
    }
    if ((!MAXSUM) || (SUM[n] >= MAXSUM)) {
      MAXSUM = SUM[n];
      NMAXSUM = n;
    }
    COMPLETESUM += SUM[n];
  }
  printf("%s\n",Biddy_Managed_GetManagerName(MNG));
  printf("function %llu: MAXDIFF = %u, MIN = %u, MAX = %u\n",NMAXDIFF,MAXDIFF,MIN[NMAXDIFF],MAX[NMAXDIFF]);
  printf("function %llu, MINSUM = %u\n",NMINSUM,MINSUM);
  printf("function %llu, MAXSUM = %u\n",NMAXSUM,MAXSUM);
  printf("COMPLETE SUM = %llu, AVGSUM = %.2f\n",COMPLETESUM,((double)COMPLETESUM/((double)nf*(double)nc)));
  if (nf == 16) {
    for (n = 0; n<nf; n++) {
      printf("MIN[%llu]=%u, MAX[%llu]=%u, SUM[%llu]=%u\n",n,MIN[n],n,MAX[n],n,SUM[n]);
    }
  }
}

void
results2b(Biddy_Manager MNG1, unsigned int *MIN1,
          Biddy_Manager MNG2,unsigned int *MIN2,
          unsigned long long int nf)
{
  int MIN,NMIN,MAX,NMAX;
  unsigned long long int n;

  /* COMPARE OBDD AND ZBDD */
  MIN = NMIN = MAX = NMAX = 0;
  for (n = 0; n<nf; n++) {
    if ((int)(MIN1[n]-MIN2[n])<=MIN) {
      MIN = MIN1[n]-MIN2[n];
      NMIN = n;
    }
    if ((int)(MIN1[n]-MIN2[n])>=MAX) {
      MAX = MIN1[n]-MIN2[n];
      NMAX = n;
    }
  }
  printf("%s versus %s\n",Biddy_Managed_GetManagerName(MNG1),Biddy_Managed_GetManagerName(MNG2));
  printf("MIN = %d, function %u: MIN1 = %u, MIN2 = %u\n",MIN,NMIN,MIN1[NMIN],MIN2[NMIN]);
  printf("MAX = %d, function %u: MIN1 = %u, MIN2 = %u\n",MAX,NMAX,MIN1[NMAX],MIN2[NMAX]);
  if (nf == 16) {
    for (n = 0; n<nf; n++) {
     printf("n = %llu, MIN1[n]-MIN2[n] = %d\n",n,MIN1[n]-MIN2[n]);
    }
  }
}

/* ########################################################################## */
/* RANDOM NUMBER GENERATORS */
/* ########################################################################## */

/* Xorshift */
/* Algorithm "xor" from p. 4 of Marsaglia, "Xorshift RNGs" */
/* https://en.wikipedia.org/wiki/Xorshift */
/* http://www.arklyffe.com/main/2010/08/29/xorshift-pseudorandom-number-generator/ */
/* 0 is always followed by 1 */ 
unsigned long long int rnd8(unsigned long long int x)
{
  if (x == 0) {
    x = 1;
  } else {
    x ^= (x << 7) & 0xffULL;
    x ^= (x >> 5) & 0xffULL;
    x ^= (x << 3) & 0xffULL;
    if (x == 1) x = 0;
  }
	return x;
}

/* Xorshift */
/* Algorithm "xor" from p. 4 of Marsaglia, "Xorshift RNGs" */
/* https://en.wikipedia.org/wiki/Xorshift */
/* http://www.arklyffe.com/main/2010/08/29/xorshift-pseudorandom-number-generator/ */
/* 0 is always followed by 1 */ 
unsigned long long int rnd16(unsigned long long int x)
{
  if (x == 0) {
    x = 1;
  } else {
    x ^= x << 13 & 0xffffULL;
    x ^= x >> 9 & 0xffffULL;
    x ^= x << 7 & 0xffffULL;
    if (x == 1) x = 0;
  }
	return x;
}

/* Xorshift */
/* Algorithm "xor" from p. 4 of Marsaglia, "Xorshift RNGs" */
/* https://en.wikipedia.org/wiki/Xorshift */
/* 0 is always followed by 1 */ 
unsigned long long int rnd32(unsigned long long int x)
{
  if (x == 0) {
    x = 1;
  } else {
  	x ^= x << 13 & 0xffffffffULL;
  	x ^= x >> 17 & 0xffffffffULL;
  	x ^= x << 5 & 0xffffffffULL;
    if (x == 1) x = 0;
  }
	return x;
}

/* Fibonacci LFSR */
/* x^8 + x^6 + x^5 + x^4 + 1 */
/* https://en.wikipedia.org/wiki/Linear-feedback_shift_register */
/* 0 is always followed by 1 */ 
unsigned long long int rnd8lfsr(unsigned long long int x)
{
  unsigned long long int bit;

  if (x == 0) {
    x = 1;
  } else {
    bit = ((x >> 0) ^ (x >> 2) ^ (x >> 3) ^ (x >> 4) ) & 1;
    x = (x >> 1) | (bit << 7);
    if (x == 1) x = 0;
  }
  return x;
}

/* Fibonacci LFSR */
/* x^16 + x^14 + x^13 + x^11 + 1 */
/* https://en.wikipedia.org/wiki/Linear-feedback_shift_register */
/* 0 is always followed by 1 */ 
unsigned long long int rnd16lfsr(unsigned long long int x)
{
  unsigned long long int bit;

  if (x == 0) {
    x = 1;
  } else {
    bit = ((x >> 0) ^ (x >> 2) ^ (x >> 3) ^ (x >> 5) ) & 1;
    x = (x >> 1) | (bit << 15);
    if (x == 1) x = 0;
  }
  return x;
}

/* ########################################################################## */
/* TESTING */
/* ########################################################################## */

void
testrnd()
{
  unsigned long long int x;
  unsigned long long int n;

  /* RND8 */
  n = 0;
  x = 0;
  do {
    n++;
    x = rnd8(x);
  } while (x != 0);
  printf("N = %llu\n",n);

  /* RND16 */
  n = 0;
  x = 0;
  do {
    n++;
    x = rnd16(x);
  } while (x != 0);
  printf("N = %llu\n",n);

  /* RND32 */
  n = 0;
  x = 0;
  do {
    n++;
    x = rnd32(x);
  } while (x != 0);
  printf("N = %llu\n",n);

  /* RND8LFSR */
  n = 0;
  x = 0;
  do {
    n++;
    x = rnd8lfsr(x);
  } while (x != 0);
  printf("N = %llu\n",n);

  /* RND16LFSR */
  n = 0;
  x = 0;
  do {
    n++;
    x = rnd16lfsr(x);
  } while (x != 0);
  printf("N = %llu\n",n);
}

void
testswap()
{
  Biddy_Manager MNG;
  Biddy_Edge s,f,g;
  unsigned int fidx,gidx;
  Biddy_Variable v;

  Biddy_InitMNG(&MNG,BIDDYTYPETZBDD);

  s = Biddy_Managed_Eval2(MNG,"a*b*c");

  f = Biddy_Managed_Eval2(MNG,"~b");
  fidx = Biddy_Managed_AddTmpFormula(MNG,"F",f);
  printf("Initial order\n");
  sjt_print(MNG,f);
  Biddy_Managed_SwapWithHigher(MNG,2);
  f = Biddy_Managed_GetIthFormula(MNG,fidx);
  printf("After swapping\n");
  sjt_print(MNG,f);
  Biddy_Managed_SwapWithLower(MNG,2);
  f = Biddy_Managed_GetIthFormula(MNG,fidx);
  printf("After swapping\n");
  sjt_print(MNG,f);

  g = Biddy_Managed_Eval2(MNG,"~a*~b*~c");
  gidx = Biddy_Managed_AddTmpFormula(MNG,"G",g);
  printf("Initial order\n");
  sjt_print(MNG,g);
  Biddy_Managed_SwapWithLower(MNG,2);
  g = Biddy_Managed_GetIthFormula(MNG,gidx);
  printf("After swapping\n");
  sjt_print(MNG,g);
  Biddy_Managed_SwapWithHigher(MNG,2);
  g = Biddy_Managed_GetIthFormula(MNG,gidx);
  printf("After swapping\n");
  sjt_print(MNG,g);

  Biddy_ExitMNG(&MNG);
}

/* THIS RESULTS ARE FOR 2 VARIABLES */
/*
RESULTS WITHOUT COMPLEMENTED EDGES
ROBDD
NODES=01, MIN = 2, MAX = 2, BASE = 2, ALL = 4 (12.50%)
NODES=03, MIN = 4, MAX = 4, BASE = 4, ALL = 8 (25.00%)
NODES=04, MIN = 8, MAX = 8, BASE = 8, ALL = 16 (50.00%)
NODES=05, MIN = 2, MAX = 2, BASE = 2, ALL = 4 (12.50%)
ZBDD
NODES=01, MIN = 2, MAX = 2, BASE = 2, ALL = 4 (12.50%)
NODES=02, MIN = 2, MAX = 2, BASE = 2, ALL = 4 (12.50%)
NODES=03, MIN = 6, MAX = 4, BASE = 5, ALL = 10 (31.25%)
NODES=04, MIN = 5, MAX = 5, BASE = 5, ALL = 10 (31.25%)
NODES=05, MIN = 1, MAX = 3, BASE = 2, ALL = 4 (12.50%)
TAGGED ZBDD
NODES=01, MIN = 5, MAX = 3, BASE = 4, ALL = 8 (25.00%)
NODES=02, MIN = 3, MAX = 3, BASE = 3, ALL = 6 (18.75%)
NODES=03, MIN = 4, MAX = 4, BASE = 4, ALL = 8 (25.00%)
NODES=04, MIN = 4, MAX = 6, BASE = 5, ALL = 10 (31.25%)

RESULTS WITH COMPLEMENTED EDGES
ROBDD WITH COMPLEMENTED EDGES
NODES=01, MIN = 2, MAX = 2, BASE = 2, ALL = 4 (12.50%)
NODES=02, MIN = 4, MAX = 4, BASE = 4, ALL = 8 (25.00%)
NODES=03, MIN = 10, MAX = 10, BASE = 10, ALL = 20 (62.50%)
ZBDD WITH COMPLEMENTED EDGES
NODES=01, MIN = 2, MAX = 2, BASE = 2, ALL = 4 (12.50%)
NODES=02, MIN = 4, MAX = 4, BASE = 4, ALL = 8 (25.00%)
NODES=03, MIN = 10, MAX = 10, BASE = 10, ALL = 20 (62.50%)
*/

/* THIS RESULTS ARE FOR 3 VARIABLES */
/*
RESULTS WITHOUT COMPLEMENTED EDGES
ROBDD
NODES=01, MIN = 2, MAX = 2, BASE = 2, ALL = 12 (0.78%)
NODES=03, MIN = 6, MAX = 6, BASE = 6, ALL = 36 (2.34%)
NODES=04, MIN = 24, MAX = 24, BASE = 24, ALL = 144 (9.38%)
NODES=05, MIN = 94, MAX = 22, BASE = 62, ALL = 372 (24.22%)
NODES=06, MIN = 104, MAX = 56, BASE = 88, ALL = 528 (34.38%)
NODES=07, MIN = 26, MAX = 146, BASE = 74, ALL = 444 (28.91%)
ZBDD
NODES=01, MIN = 2, MAX = 2, BASE = 2, ALL = 12 (0.78%)
NODES=02, MIN = 3, MAX = 3, BASE = 3, ALL = 18 (1.17%)
NODES=03, MIN = 15, MAX = 9, BASE = 12, ALL = 72 (4.69%)
NODES=04, MIN = 62, MAX = 17, BASE = 32, ALL = 192 (12.50%)
NODES=05, MIN = 79, MAX = 31, BASE = 57, ALL = 342 (22.27%)
NODES=06, MIN = 69, MAX = 63, BASE = 76, ALL = 456 (29.69%)
NODES=07, MIN = 26, MAX = 131, BASE = 74, ALL = 444 (28.91%)
TAGGED ZBDD
NODES=01, MIN = 9, MAX = 3, BASE = 5, ALL = 30 (1.95%)
NODES=02, MIN = 24, MAX = 6, BASE = 14, ALL = 84 (5.47%)
NODES=03, MIN = 44, MAX = 14, BASE = 28, ALL = 168 (10.94%)
NODES=04, MIN = 72, MAX = 30, BASE = 50, ALL = 300 (19.53%)
NODES=05, MIN = 91, MAX = 46, BASE = 89, ALL = 534 (34.77%)
NODES=06, MIN = 16, MAX = 157, BASE = 70, ALL = 420 (27.34%)

RESULTS WITH COMPLEMENTED EDGES
ROBDD WITH COMPLEMENTED EDGES
NODES=01, MIN = 2, MAX = 2, BASE = 2, ALL = 12 (0.78%)
NODES=02, MIN = 6, MAX = 6, BASE = 6, ALL = 36 (2.34%)
NODES=03, MIN = 30, MAX = 30, BASE = 30, ALL = 180 (11.72%)
NODES=04, MIN = 186, MAX = 18, BASE = 98, ALL = 588 (38.28%)
NODES=05, MIN = 32, MAX = 200, BASE = 120, ALL = 720 (46.88%)
ZBDD WITH COMPLEMENTED EDGES
NODES=01, MIN = 2, MAX = 2, BASE = 2, ALL = 12 (0.78%)
NODES=02, MIN = 6, MAX = 6, BASE = 6, ALL = 36 (2.34%)
NODES=03, MIN = 30, MAX = 30, BASE = 30, ALL = 180 (11.72%)
NODES=04, MIN = 174, MAX = 24, BASE = 98, ALL = 588 (38.28%)
NODES=05, MIN = 44, MAX = 194, BASE = 120, ALL = 720 (46.88%)
*/


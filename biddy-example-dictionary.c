/* $Revision: 305 $ */
/* $Date: 2017-09-13 10:47:03 +0200 (sre, 13 sep 2017) $ */
/* This file (biddy-example-dictionary.c) is a C file */
/* Author: Robert Meolic (robert.meolic@um.si) */
/* This file has been released into the public domain by the author. */

/* This example is compatible with Biddy v1.7 and CUDD v3.0.0 */

/* COMPILE WITH: */
/* gcc -DREPORT -DUNIX -DUSE_BIDDY -O2 -o biddy-example-dictionary biddy-example-dictionary.c -I. -L./bin -lbiddy -lgmp */
/* gcc -DREPORT -DUSE_CUDD -O2 -o cudd-example-dictionary biddy-example-dictionary.c -I ../cudd/include/ -L ../cudd/lib/ -lcudd -lm */

/* Some notes */
/*
1 2 3 4 5 6 7 8 9 10 11 12 13 14 15 16 17 18 19 20 21 22 23 24 25 26 27 28 29
A B C Č D E F G H I  J  K  L  M  N  O  P  Q  R  S  Š  T  U  V  W  X  Y  Z  Ž
*/

#define WORDSIZE 5
#define ALPHABETSIZE 26
#define ALPHABET {'A','B','C','D','E','F','G','H','I','J','K','L','M','N','O','P','Q','R','S','T','U','V','W','X','Y','Z'}
/*
#define ALPHABET {'A','B','C','Č','D','E','F','G','H','I','J','K','L','M','N','O','P','R','S','Š','T','U','V','Z','Ž'}
#define ALPHABET {'A','B','C','Č','D','E','F','G','H','I','J','K','L','M','N','O','P','Q','R','S','Š','T','U','V','W','X','Y','Z','Ž'}
*/

#include <stdio.h>
#include <string.h>
#include <ctype.h>

#ifdef USE_BIDDY
#  include "biddy.h"
#  define BDDNULL NULL
#endif

#ifdef USE_CUDD
#  include <stdlib.h>
#  include <stdint.h>
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
#define BIDDYTYPEOBDDC 2
#define BIDDYTYPEZBDD 3
#define BIDDYTYPEZBDDC 4
#define BIDDYTYPETZBDD 5
#define BIDDYTYPETZBDDC 6
#define Biddy_GetManagerType() BIDDYTYPEOBDDC
#define Biddy_GetConstantZero() Cudd_ReadLogicZero(manager)
#define Biddy_GetConstantOne() Cudd_ReadOne(manager)
#define Biddy_AddVariable() Cudd_bddNewVar(manager)
#define Biddy_Inv(f) Cudd_Not(f)
#define Biddy_Not(f) Cudd_Not(f)
#define Biddy_And(f,g) Cudd_bddAnd(manager,f,g)
#define Biddy_Or(f,g) Cudd_bddOr(manager,f,g)
#define Biddy_Xor(f,g) Cudd_bddXor(manager,f,g)
#define Biddy_Xnor(f,g) Cudd_bddXnor(manager,f,g)
#define Biddy_Support(f) Cudd_Support(manager,f)
#define Biddy_DependentVariableNumber(f) Cudd_SupportSize(manager,f)
#define Biddy_CountMinterm(f,n) Cudd_CountMinterm(manager,f,n)
#define Biddy_ChangeVariableName(v,name) 0
#define Biddy_NodeNumber(f) Cudd_DagSize(f)
#define Biddy_NodeTableNum() Cudd_ReadKeys(manager)
#define Biddy_NodeTableSize() Cudd_ReadSlots(manager)
#define Biddy_PrintInfo(s) Cudd_PrintInfo(manager,s)
#endif

int main(int argv, char ** argc) {
  clock_t elapsedtime;
  FILE *f;
  char alphabet[ALPHABETSIZE] = ALPHABET;
  Biddy_Edge code[WORDSIZE][ALPHABETSIZE];  /* BDD variables */
  Biddy_Edge dictionary;
  Biddy_Edge tmp1;
  unsigned int i,j;
  unsigned int letter;
  Biddy_String name;
  char word5[6]; /* 5-letter word + \0 */
  Biddy_Boolean eof;

  setbuf(stdout,NULL);

  /* initialize the BDD manager with default options */

#ifdef USE_BIDDY
  printf("Using Biddy");
  /* DEFAULT INIT CALL: Biddy_InitAnonymous(BIDDYTYPEOBDDC) */
  Biddy_InitAnonymous(BIDDYTYPEOBDDC);
#endif

#ifdef USE_CUDD
  printf("Using CUDD");
  /* DEFAULT INIT CALL: Cudd_Init(0,0,CUDD_UNIQUE_SLOTS,CUDD_CACHE_SLOTS,0) */
  manager = Cudd_Init(0,0,CUDD_UNIQUE_SLOTS,CUDD_CACHE_SLOTS,0);
  /* Cudd_SetMaxCacheHard(manager,262144); */
  /* Cudd_AutodynEnable(manager,CUDD_REORDER_SAME); */
#endif

  printf(" (%s)...\n",Biddy_GetManagerName());

  /* check alphabet */
  for (i=0; i<ALPHABETSIZE; i++) {
    printf("(%c)",alphabet[i]);
  }
  printf("\n");

  elapsedtime = clock();

  /* create BDD variables */
#ifdef USE_BIDDY
  if ((Biddy_GetManagerType() == BIDDYTYPEOBDD) || (Biddy_GetManagerType() == BIDDYTYPEOBDDC) ||
      (Biddy_GetManagerType() == BIDDYTYPETZBDD) || (Biddy_GetManagerType() == BIDDYTYPETZBDD))
  {
    char name[3];
    name[0] = name[1] = name[2] = 0;
    for (i=0; i<WORDSIZE; i++) {
      name[1] = '1' + i;
      for (j=0; j<ALPHABETSIZE; j++) {
        name[0] = alphabet[j];
        code[i][j] = Biddy_AddVariable();
        Biddy_ChangeVariableName(Biddy_VariableTableNum()-1,name); /* rename the last added variable */
      }
    }
  }
#endif

  /* for ZBDDs, by adding new variables all the existing ones are changed */
  /* after adding the last variable we have to refresh the array of variables */
  /* NOTE: for Biddy, variables are added in the reverse order to make results comparable with Knuth */
  /* NOTE: for Biddy, code[0][0] = "A1" and it is last added, it can be retrieved by Biddy_GetVariableEdge(WORDSIZE*ALPHABETSIZE) */
  /* NOTE: for Biddy, user variables start with index 1 */
#ifdef USE_BIDDY
  if ((Biddy_GetManagerType() == BIDDYTYPEZBDD) || (Biddy_GetManagerType() == BIDDYTYPEZBDDC))
  {
    char name[3];
    name[0] = name[1] = name[2] = 0;
    for (i=0; i<WORDSIZE; i++) {
      name[1] = '1' + (WORDSIZE-i-1);
      for (j=0; j<ALPHABETSIZE; j++) {
        name[0] = alphabet[ALPHABETSIZE-j-1];
        code[WORDSIZE-i-1][ALPHABETSIZE-j-1] = Biddy_AddVariable();
        Biddy_ChangeVariableName(Biddy_VariableTableNum()-1,name); /* rename the last added variable */
      }
    }
    for (i=0; i<WORDSIZE; i++) {
      for (j=0; j<ALPHABETSIZE; j++) {
        code[i][j] = Biddy_GetVariableEdge(WORDSIZE*ALPHABETSIZE-(i*ALPHABETSIZE+j));
      }
    }
  }
#endif

  dictionary = Biddy_GetConstantZero();

  f = fopen("./test/sgb-words.txt","r");
  eof = FALSE;
  while (!eof) {
    eof = ( fscanf(f,"%s",word5) == EOF );
    if (!eof) {

      tmp1 = Biddy_GetConstantOne();
      for (i=0; i<WORDSIZE; i++) {
        letter = toupper(word5[i]);
        for (j=0; j<ALPHABETSIZE; j++) {
          if (letter == alphabet[j]) {
            tmp1 = Biddy_And(tmp1,code[i][j]);
          } else {
            tmp1 = Biddy_And(tmp1,Biddy_Not(code[i][j]));
          }
        }
      }
      dictionary = Biddy_Or(dictionary,tmp1);

#ifdef USE_BIDDY
      Biddy_AddTmpFormula(dictionary,1);
      Biddy_Clean();
#endif

      /* DEBUGGING */
      /*
      printf("<%s,%u>",word5,Biddy_NodeNumber(dictionary));
      */

    }
    eof = (fgetc(f) == EOF);
  }
  fclose(f);

  elapsedtime = clock()-elapsedtime;

  printf("\n");
  printf("Resulting dictionary depends on %u variables.\n",Biddy_DependentVariableNumber(dictionary));
  printf("Resulting dictionary has %.0f minterms.\n",Biddy_CountMinterm(dictionary,WORDSIZE*ALPHABETSIZE));
  printf("System has %u nodes.\n",Biddy_NodeTableNum());

#ifdef USE_BIDDY
#ifdef MINGW
  printf("Memory in use: %I64u B\n",Biddy_ReadMemoryInUse());
#else
  printf("Memory in use: %llu B\n",Biddy_ReadMemoryInUse());
#endif
#endif

  printf("TIME: %.2f\n",elapsedtime/(1.0*CLOCKS_PER_SEC));

#ifdef USE_BIDDY
  if (Biddy_GetManagerType() == BIDDYTYPEOBDD) {
    printf("OBDD for resulting dictionary has %u nodes.\n",
           Biddy_NodeNumber(dictionary));
  } else if (Biddy_GetManagerType() == BIDDYTYPEOBDDC) {
    printf("OBDD for resulting dictionary has %u nodes (%u nodes if using complement edges).\n",
           Biddy_NodeNumberPlain(dictionary),Biddy_NodeNumber(dictionary));
  } else if (Biddy_GetManagerType() == BIDDYTYPEZBDD) {
    printf("ZBDD for resulting dictionary has %u nodes.\n",
           Biddy_NodeNumber(dictionary));
  } else if (Biddy_GetManagerType() == BIDDYTYPEZBDDC) {
    printf("ZBDD for resulting dictionary has %u nodes (%u nodes if using complement edges).\n",
           Biddy_NodeNumberPlain(dictionary),Biddy_NodeNumber(dictionary));
  } else if (Biddy_GetManagerType() == BIDDYTYPETZBDD) {
    printf("TZBDD for resulting dictionary has %u nodes.\n",
           Biddy_NodeNumber(dictionary));
  } else if (Biddy_GetManagerType() == BIDDYTYPETZBDDC) {
    printf("TZBDD for resulting dictionary has %u nodes (%u nodes if using complement edges).\n",
           Biddy_NodeNumberPlain(dictionary),Biddy_NodeNumber(dictionary));
  }
#endif

  printf("\n");
  Biddy_PrintInfo(stdout);

  /* DEBUGGING */
  /*
  printf("\n");
  Biddy_WriteDot("dictionary.dot",dictionary,"DICT",-1,FALSE);
  printf("USE 'dot -y -Tpng -O dictionary.dot' to visualize BDD for dictionary.\n");
  */

  /* DEBUGGING */
  /*
  printf("\n");
  Biddy_WriteBddview("dictionary.bddview",dictionary,"DICT",NULL);
  */
}

/* $Revision: 353 $ */
/* $Date: 2017-12-07 13:25:28 +0100 (čet, 07 dec 2017) $ */
/* This file (biddy-example-dictionary.c) is a C file */
/* Author: Robert Meolic (robert.meolic@um.si) */
/* This file has been released into the public domain by the author. */

/* This example is compatible with Biddy v1.7 and CUDD v3.0.0 */
/* This example uses biddy-cudd.h and biddy-cudd.c */

/* COMPILE WITH: */
/* gcc -DNOREPORT -DSIFTING -DUNIX -DBIDDY -DOBDDC -O2 -o biddy-example-dictionary biddy-example-dictionary.c -I. -L./bin -static -lbiddy -lgmp */
/* gcc -DNOREPORT -DSIFTING -DCUDD -DOBDDC -O2 -o cudd-example-dictionary biddy-example-dictionary.c -I ../cudd/include/ -L ../cudd/lib/ -lcudd -lm */

/* THIS IS COMPATIBLE WITH D. E. Knuth's BENCHMARKS */
/**/
#define WORDSIZE 5
#define ALPHABETSIZE 26
#define ALPHABET {'A','B','C','D','E','F','G','H','I','J','K','L','M','N','O','P','Q','R','S','T','U','V','W','X','Y','Z'}
#define TOUPPER
#define FILENAME "./test/sgb-words.txt"
/**/

/* THIS IS (ALMOST!?) COMPATIBLE WITH R. Bryant's BENCHMARKS */
/* NEWLINE IS THE LAST SYMBOL */
/*
#define WORDSIZE 24
#define ALPHABETSIZE 54
#define ALPHABET {'-', \
                  'A','B','C','D','E','F','G','H','I','J','K','L','M','N','O','P','Q','R','S','T','U','V','W','X','Y','Z', \
                  'a','b','c','d','e','f','g','h','i','j','k','l','m','n','o','p','q','r','s','t','u','v','w','x','y','z', \
                  '.' \
                 }
#define WITHNEWLINE
#define FILENAME "./test/words.txt"
*/

/* THIS IS (ALMOST!?) COMPATIBLE WITH R. Bryant's BENCHMARKS */
/* NEWLINE IS THE LAST SYMBOL */
/* BIDDY MUST BE ADAPTED TO SUPPORT THE HUGE NUMBER OF VARIABLES */
/*
#define WORDSIZE 24
#define ALPHABETSIZE 129
#define ALPHABET {}
#define ASCII
#define WITHNEWLINE
#define FILENAME "./test/words.txt"
*/

/* THIS IS FOR TESTING, ONLY */
/*
1 2 3 4 5 6 7 8 9 10 11 12 13 14 15 16 17 18 19 20 21 22 23 24 25 26 27 28 29
A B C Č D E F G H I  J  K  L  M  N  O  P  Q  R  S  Š  T  U  V  W  X  Y  Z  Ž
*/
/*
#define ALPHABET {'A','B','C','Č','D','E','F','G','H','I','J','K','L','M','N','O','P','R','S','Š','T','U','V','Z','Ž'}
#define ALPHABET {'A','B','C','Č','D','E','F','G','H','I','J','K','L','M','N','O','P','Q','R','S','Š','T','U','V','W','X','Y','Z','Ž'}
*/

#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <time.h>

#include "biddy-cudd.h"

/* BDD CREATION IS OPTIMIZED BY DEFAULT BUT YOU CAN OVERRIDE THIS */
#if !defined(BOTTOMUP) && !defined(TOPDOWN)
#  if defined(OBDD) || defined(OBDDC)
#    define BOTTOMUP
#  endif
#  if defined(ZBDD) || defined(ZBDDC)
#    define TOPDOWN
#  endif
#  if defined(TZBDD) || defined(TZBDDC)
#    define BOTTOMUP
#  endif
#endif

int main(int argv, char ** argc) {
  clock_t elapsedtime;
  FILE *f;
  char alphabet[ALPHABETSIZE] = ALPHABET;
  Biddy_Edge code[WORDSIZE][ALPHABETSIZE];  /* BDD variables */
  Biddy_Edge codenot[WORDSIZE][ALPHABETSIZE];  /* BDD variables */
  Biddy_Edge dictionary;
  Biddy_Edge tmp1,tmp2;
  unsigned int i,j,n;
  unsigned int letter;
  Biddy_String name;
  char oneword[WORDSIZE+1]; /* word + \0 */
  Biddy_Boolean eof,stop;
  Biddy_String userinput;
  unsigned int dictionary_idx;
  
  setbuf(stdout,NULL);
  userinput = strdup("...");

#include "biddy-cudd.c" /* this will initialize BDD manager */

  printf(" (%s)\n",Biddy_GetManagerName());

#ifdef WITHNEWLINE
  printf("USING NEW LINE\n");
#endif

#ifdef REVERSEORDER
  printf("USING REVERSE ORDER\n");
#endif

#ifdef BOTTOMUP
  printf("USING BOTTOM-UP APPROACH\n");
#endif

#ifdef TOPDOWN
  printf("USING TOP-DOWN APPROACH\n");
#endif

#ifdef ASCII
  for (i=0; i<ALPHABETSIZE; i++) {
    alphabet[i] = i;
  }
#endif

  /* check alphabet */
  for (i=0; i<ALPHABETSIZE; i++) {
    if ((alphabet[i] >= 32) && (alphabet[i] <= 126)) {
      printf("(%c)",alphabet[i]);
    } else {
      printf("()");
    }
  }
  printf("\n");

  elapsedtime = clock();

  /* create BDD variables */
  /* NOTE for Biddy: ZBDD and TZBDD variables are added in the reverse order by default */
  /* NOTE for Biddy and CUDD: in any case, code[0][0] = "A01" (i.e. the first letter in the alphabet) */

#ifdef BIDDY
#ifdef REVERSEORDER
  if ((Biddy_GetManagerType() == BIDDYTYPEZBDD) || (Biddy_GetManagerType() == BIDDYTYPEZBDDC) ||
      (Biddy_GetManagerType() == BIDDYTYPETZBDD) || (Biddy_GetManagerType() == BIDDYTYPETZBDDC))
#else
  if ((Biddy_GetManagerType() == BIDDYTYPEOBDD) || (Biddy_GetManagerType() == BIDDYTYPEOBDDC))
#endif
#endif

#ifdef CUDD
#ifdef REVERSEORDER
  if (FALSE)
#else
  if (TRUE)
#endif
#endif

  {
    char name[4];
    name[0] = name[1] = name[2] = 0;
    name[3] = 0; /* end of string */
    for (i=0; i<WORDSIZE; i++) {
      name[1] = '0' + (i+1)/10;
      name[2] = '0' + (i+1)%10;
      for (j=0; j<ALPHABETSIZE; j++) {
        name[0] = alphabet[j];
        code[i][j] = Biddy_AddVariable();
        Biddy_ChangeVariableName(Biddy_VariableTableNum()-1,name); /* rename the last added variable */
        REF(code[i][j]);
        /* printf("(code[%u][%u]=%s)",i,j,name); */
      }
    }
  }

#ifdef BIDDY
#ifdef REVERSEORDER
  if ((Biddy_GetManagerType() == BIDDYTYPEOBDD) || (Biddy_GetManagerType() == BIDDYTYPEOBDDC))
#else
  if ((Biddy_GetManagerType() == BIDDYTYPEZBDD) || (Biddy_GetManagerType() == BIDDYTYPEZBDDC) ||
      (Biddy_GetManagerType() == BIDDYTYPETZBDD) || (Biddy_GetManagerType() == BIDDYTYPETZBDDC))
#endif
#endif

#ifdef CUDD
#ifdef REVERSEORDER
  if (TRUE)
#else
  if (FALSE)
#endif
#endif

  {
    char name[4];
    name[0] = name[1] = name[2] = 0;
    name[3] = 0; /* end of string */
    for (i=0; i<WORDSIZE; i++) {
      name[1] = '0' + (WORDSIZE-i)/10;
      name[2] = '0' + (WORDSIZE-i)%10;
      for (j=0; j<ALPHABETSIZE; j++) {
        name[0] = alphabet[ALPHABETSIZE-j-1];
        code[WORDSIZE-i-1][ALPHABETSIZE-j-1] = Biddy_AddVariable();
        Biddy_ChangeVariableName(Biddy_VariableTableNum()-1,name); /* rename the last added variable */
        REF(code[WORDSIZE-i-1][ALPHABETSIZE-j-1]);
        /* printf("(code[%u][%u]=%s)",WORDSIZE-i-1,ALPHABETSIZE-j-1,name); */
      }
    }
  }

  /* For ZBDDs, by adding new variables all the existing ones are changed. */
  /* Thus, after adding the last variable we have to refresh the array of variables. */
  /* NOTE: Biddy_GetVariableEdge returns variables by the creation time, not by the order used in the BDD */
  /* NOTE for Biddy: user variables start with index 1 */
  /* NOTE for CUDD: user variables start with index 0 */

  if ((Biddy_GetManagerType() == BIDDYTYPEZBDD) || (Biddy_GetManagerType() == BIDDYTYPEZBDDC))
  {
    for (i=0; i<WORDSIZE; i++) {
      for (j=0; j<ALPHABETSIZE; j++) {
#ifdef BIDDY
        /* variable edges in Biddy never become obsolete */
#ifdef REVERSEORDER
        code[i][j] = Biddy_GetVariableEdge(i*ALPHABETSIZE+j+1);
#else
        code[i][j] = Biddy_GetVariableEdge(WORDSIZE*ALPHABETSIZE-(i*ALPHABETSIZE+j));
#endif
#endif
#ifdef CUDD
        /* variable edges in CUDD are referenced - this is required for ZBDDs */
        DEREF(code[i][j]);
#ifdef REVERSEORDER
        code[i][j] = Biddy_GetVariableEdge(WORDSIZE*ALPHABETSIZE-(i*ALPHABETSIZE+j)-1);
#else
        code[i][j] = Biddy_GetVariableEdge(i*ALPHABETSIZE+j);
#endif
        REF(code[i][j]);
#endif
      }
    }
  }

  /* CREATE NEGATIONS */
  for (i=0; i<WORDSIZE; i++) {
    for (j=0; j<ALPHABETSIZE; j++) {
      codenot[i][j] = Biddy_Not(code[i][j]);
      REF(codenot[i][j]);
      MARK_0(codenot[i][j]);
    }
  }

  /* START CALCULATION */

#ifdef TOPDOWN
#ifdef REVERSEORDER
#  undef REVERSEORDER
#else
#  define REVERSEORDER
#endif
#endif

  dictionary = Biddy_GetConstantZero();
  REF(dictionary); /* not needed for Biddy but not wrong */

  /* DEBUGGING */
  /**/
  printf("\n");
  printf("Initial system has %u nodes / %lu live nodes.\n",Biddy_NodeTableNum(),Biddy_NodeTableNumLive());
#ifdef BIDDY
  printf("Initial system has %u user variables.\n",Biddy_VariableTableNum()-1);
#endif
#ifdef CUDD
  printf("Initial system has %u user variables.\n",Biddy_VariableTableNum());
#endif
  printf("Empty dictionary depends on %u variables.\n",Biddy_DependentVariableNumber(dictionary));
  printf("Empty dictionary has %.0f minterms.\n",Biddy_CountMinterms(dictionary,WORDSIZE*ALPHABETSIZE));
  printf("Empty dictionary has %u node(s).\n",Biddy_CountNodes(dictionary));
  /**/

  n = 0;
  f = fopen(FILENAME,"r");
  eof = FALSE;
  while (!eof) {
    eof = ( fscanf(f,"%s",oneword) == EOF );
    if (!eof) {
      if (strlen(oneword) > WORDSIZE) {
        printf("WORDSIZE TO SMALL, YOU NEED AT LEAST %u\n",(unsigned int)strlen(oneword));
        exit(1);
      }
      n++;
      tmp1 = Biddy_GetConstantOne();
      REF(tmp1); /* not needed for Biddy but not wrong */

/* IF NOT REVERSEORDER THEN ADD UNUSED VARIABLES FIRST BECAUSE THEY ARE AT THE BOTTOM PART OF THE BDD */
#ifndef REVERSEORDER
      i = WORDSIZE-1;
      while (i > strlen(oneword)) {
        for (j=ALPHABETSIZE-1; j<ALPHABETSIZE; j--) { /* j is unsigned */
          tmp2 = Biddy_And(tmp1,codenot[i][j]);
          /* printf("AND[%u][%u]",i,j); */
          REF(tmp2);
          DEREF(tmp1);
          tmp1 = tmp2;
        }
        i--;
      }
#endif

/* IF NOT REVERSEORDER THEN ADD NEWLINE VARIABLE BEFORE THE WORD BECAUSE THIS VARIABLE IS BOTTOM-MORE */
/* NEWLINE VARIABLE IS THE LAST VARIABLE IN THE ALPHABET */
#ifndef REVERSEORDER
#ifdef WITHNEWLINE
      if (strlen(oneword) < WORDSIZE) {
        j=ALPHABETSIZE-1;
        tmp2 = Biddy_And(tmp1,code[i][j]);
        /* printf("AND[%u][%u]",i,j); */
        REF(tmp2);
        DEREF(tmp1);
        tmp1 = tmp2;
        j--;
        for (j; j<ALPHABETSIZE; j--) { /* j is unsigned */
          tmp2 = Biddy_And(tmp1,codenot[i][j]);
          /* printf("AND[%u][%u]",i,j); */
          REF(tmp2);
          DEREF(tmp1);
          tmp1 = tmp2;
        }
        i--;
      }
#endif
#endif

/* ADD ALL LETTERS FROM THE WORD */

#ifdef REVERSEORDER
      for (i=0; i<strlen(oneword); i++) {
#else
      for (i; i<strlen(oneword); i--) { /* i is unsigned */
#endif

#ifdef TOUPPER
        letter = toupper(oneword[i]);
#else
        letter = oneword[i];
#endif
#ifdef REVERSEORDER
        for (j=0; j<ALPHABETSIZE; j++) {
#else
        for (j=ALPHABETSIZE-1; j<ALPHABETSIZE; j--) { /* j is unsigned */
#endif
          if (letter == alphabet[j]) {
            tmp2 = Biddy_And(tmp1,code[i][j]);
          } else {
            tmp2 = Biddy_And(tmp1,codenot[i][j]);
          }
          /* printf("AND[%u][%u]",i,j); */
          REF(tmp2);
          DEREF(tmp1);
          tmp1 = tmp2;
        }
      }

/* IF REVERSEORDER THEN ADD NEWLINE VARIABLE AFTER THE WORD BECAUSE THIS VARIABLE IS TOP-MORE */
/* NEWLINE VARIABLE IS THE LAST VARIABLE IN THE ALPHABET */
#ifdef REVERSEORDER
#ifdef WITHNEWLINE
      if (i < WORDSIZE) {
        for (j=0; j<ALPHABETSIZE-1; j++) {
          tmp2 = Biddy_And(tmp1,codenot[i][j]);
          /* printf("AND[%u][%u]",i,j); */
          REF(tmp2);
          DEREF(tmp1);
          tmp1 = tmp2;
        }
        tmp2 = Biddy_And(tmp1,code[i][j]);
        REF(tmp2);
        DEREF(tmp1);
        tmp1 = tmp2;
        /* printf("AND[%u][%u]",i,j); */
        i++;
      }
#endif
#endif

/* IF  REVERSEORDER THEN ADD UNUSED VARIABLES LAST BECAUSE THEY ARE AT THE TOP PART OF THE BDD */
#ifdef REVERSEORDER
      while (i < WORDSIZE) {
        for (j=0; j<ALPHABETSIZE; j++) {
          tmp2 = Biddy_And(tmp1,codenot[i][j]);
          /* printf("AND[%u][%u]",i,j); */
          REF(tmp2);
          DEREF(tmp1);
          tmp1 = tmp2;
        }
        i++;
      }
#endif
      /* printf("OR\n"); */

      tmp2 = Biddy_Or(dictionary,tmp1);
      REF(tmp2);
      DEREF(dictionary);
      DEREF(tmp1);
      dictionary = tmp2;

      MARK(dictionary);
      SWEEP();

      /* DEBUGGING */
      /* Biddy uses Biddy_CountNodesPlain() to report ZBDD nodes instead of ZBDDC nodes */
      /*
#ifdef BIDDY
      if ((Biddy_GetManagerType() == BIDDYTYPEZBDD) || (Biddy_GetManagerType() == BIDDYTYPEZBDDC))
        printf("<%s, nodes=%u / all=%u / live=%lu>\n",oneword,Biddy_CountNodesPlain(dictionary),Biddy_CountNodesNum(),Biddy_NodeTableNumLive());
      else
        printf("<%s, nodes=%u / all=%u / live=%lu>\n",oneword,Biddy_CountNodes(dictionary),Biddy_CountNodesNum(),Biddy_NodeTableNumLive());
#endif
#ifdef CUDD
      printf("<%s, nodes=%u / all=%u / live=%lu>\n",oneword,Biddy_CountNodes(dictionary),Biddy_NodeTableNum(),Biddy_NodeTableNumLive());
#endif
      */
      
    }
    eof = (fgetc(f) == EOF);
  }
  fclose(f);
  printf("%u WORDS READ FROM FILE %s\n",n,FILENAME);

  elapsedtime = clock()-elapsedtime;

#ifdef SIFTING
#ifdef BIDDY
    dictionary_idx = Biddy_AddPersistentFormula("DICT",dictionary);
    Biddy_Purge(); /* dictionary_idx may change during purge */
    Biddy_FindFormula("DICT",&dictionary_idx,&dictionary);
#endif
#endif

  do {

    /* FOR TZBDD, TOP EDGE OF dictionary MAY CHANGE DURING THE SIFTING */
#ifdef BIDDY
    dictionary = Biddy_GetIthFormula(dictionary_idx);
#endif

    printf("\n");
    printf("System has %u nodes / %lu live nodes.\n",Biddy_NodeTableNum(),Biddy_NodeTableNumLive());
#ifdef BIDDY
    printf("System has %u user variables.\n",Biddy_VariableTableNum()-1);
#endif
#ifdef CUDD
    printf("System has %u user variables.\n",Biddy_VariableTableNum());
#endif
    printf("Resulting dictionary depends on %u variables.\n",Biddy_DependentVariableNumber(dictionary));
    printf("Resulting dictionary has %.0f minterms.\n",Biddy_CountMinterms(dictionary,WORDSIZE*ALPHABETSIZE));
    printf("Resulting dictionary has %u nodes.\n",Biddy_CountNodes(dictionary));
#ifdef BIDDY
#ifdef MINGW
    printf("Memory in use: %I64u B\n",Biddy_ReadMemoryInUse());
#else
    printf("Memory in use: %llu B\n",Biddy_ReadMemoryInUse());
#endif
#endif
#ifdef CUDD
    printf("Memory in use: %lu B\n",Biddy_ReadMemoryInUse());
#endif

    printf("TIME: %.2f\n",elapsedtime/(1.0*CLOCKS_PER_SEC));

#ifdef BIDDY
    if (Biddy_GetManagerType() == BIDDYTYPEOBDD) {
      printf("OBDD for resulting dictionary has %u nodes.\n",
             Biddy_CountNodes(dictionary));
    } else if (Biddy_GetManagerType() == BIDDYTYPEOBDDC) {
      printf("OBDD for resulting dictionary has %u nodes (%u nodes if using complement edges).\n",
             Biddy_CountNodesPlain(dictionary),Biddy_CountNodes(dictionary));
    } else if (Biddy_GetManagerType() == BIDDYTYPEZBDD) {
      printf("ZBDD for resulting dictionary has %u nodes.\n",
             Biddy_CountNodes(dictionary));
    } else if (Biddy_GetManagerType() == BIDDYTYPEZBDDC) {
      printf("ZBDD for resulting dictionary has %u nodes (%u nodes if using complement edges).\n",
             Biddy_CountNodesPlain(dictionary),Biddy_CountNodes(dictionary));
    } else if (Biddy_GetManagerType() == BIDDYTYPETZBDD) {
      printf("TZBDD for resulting dictionary has %u nodes.\n",
             Biddy_CountNodes(dictionary));
    } else if (Biddy_GetManagerType() == BIDDYTYPETZBDDC) {
      printf("TZBDD for resulting dictionary has %u nodes (%u nodes if using complement edges).\n",
             Biddy_CountNodesPlain(dictionary),Biddy_CountNodes(dictionary));
    }
#endif

#ifdef REPORT
    Biddy_PrintInfo(stdout);
#endif

#ifdef SIFTING
    printf("SIFTING / CONVERGE SIFTING / EXIT [S/C/X]): ");
    if (!scanf("%s",userinput)) printf("ERROR\n");

    elapsedtime = clock();

    if (toupper(userinput[0]) == 'S') {
#ifdef BIDDY
      Biddy_Sifting(NULL,FALSE); /* global sifiting because no function is given, sifting is not converge */
#endif
#ifdef CUDD
      Cudd_ReduceHeap(manager,CUDD_REORDER_SIFT,0);
#endif
      stop = FALSE;
    } else if (toupper(userinput[0]) == 'C') {
#ifdef BIDDY
      Biddy_Sifting(NULL,TRUE); /* global sifiting because no function is given, converge sifting */
#endif
#ifdef CUDD
      Cudd_ReduceHeap(manager,CUDD_REORDER_SIFT_CONVERGE,0);
#endif
      stop = FALSE;
    } else if (toupper(userinput[0]) == 'X') {
      stop = TRUE;
    } else {
      stop = FALSE;
    }
#else
    stop = TRUE;
#endif

    elapsedtime = clock()-elapsedtime;

  } while (!stop);

  free(userinput);

  /* DEBUGGING */
  /*
  printf("\n");
  Biddy_WriteDot("dictionary.dot",dictionary,"DICT",-1,FALSE);
  printf("USE 'dot -y -Tpng -O dictionary.dot' to visualize BDD for dictionary.\n");
  */

  /* DEBUGGING */
  /* THIS WILL GENERATE bddview FILE WITHOUT COORDINATES */
  /* THIS FILE IS NOT SUPPORTED BY BDD Scout, YET */
  /*
  printf("\n");
  Biddy_WriteBddview("dictionary.bddview",dictionary,"DICT",NULL);
  */

  /* EXIT */

  DEREF(dictionary);
  for (i=0; i<WORDSIZE; i++) {
    for (j=0; j<ALPHABETSIZE; j++) {
      DEREF(code[i][j]);
      DEREF(codenot[i][j]);
    }
  }

#ifdef REPORT
#ifdef CUDD
  printf("CUDD: nodes with non-zero reference counts: %d\n",Cudd_CheckZeroRef(manager));
#endif
#endif

  Biddy_Exit();

}

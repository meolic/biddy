/* $Revision: 652 $ */
/* $Date: 2021-08-28 09:52:46 +0200 (sob, 28 avg 2021) $ */
/* This file (biddy-example-dictionary.c) is a C file */
/* Author: Robert Meolic (robert@meolic.com) */
/* This file has been released into the public domain by the author. */

/* This example is compatible with Biddy v2.0 (and laters) and CUDD v3.0.0 */
/* This example uses biddy-cudd.h and biddy-cudd.c */

/* COMPILE WITH (ADD -lgmp IF USING STATIC BIDDY LIBRARY): */
/* gcc -DNOREPORT -DNOSIFTING -DINTERACTIVE -DUNIX -DBIDDY -DTZBDD -O2 -o biddy-example-dictionary biddy-example-dictionary.c biddy-example-dict-common.c -I. -L./bin -static -lbiddy */
/* gcc -DNOREPORT -DNOSIFTING -DINTERACTIVE -DCUDD -DZBDD -O2 -o cudd-example-dictionary biddy-example-dictionary.c biddy-example-dict-common.c -I ../cudd/include/ -L ../cudd/lib/ -lcudd -lm */

/* TO FIND THE LONGES LINES (THANKS TO Chris Koknat) */
/* https://stackoverflow.com/questions/1655372/longest-line-in-a-file */
/* perl -ne 'print length()."  line $.  $_"' myfile | sort -nr | head -n 1 */

/* NOTES ON BENCHMARKS */
/* MAXWORDSIZE - maximal word length */
/* ALPHABETSIZE - number of letters (without null symbol) */
/* ALPHABET - a given array of letters */
/* ASCII - use letters from ASCII table instead from the given array of letters */
/* WITHNULLSYMBOL/NONULLSYMBOL - use null symbol, this allows words of different length */
/* TOUPPER - change all letters to uppercase */
/* FILENAME - dictionary, default benchmarks are in folder ./bddscout/DICTIONARY */
/* OUTPUTNAME - where to save BDD for dictionary, default folder is ./bddscout/DICTIONARY */

/* BENCHMARK USED IN THE PAPER */
/* THIS IS A SMALL EXAMPLE USING JUST 4 WORDS AND ALPHABET WITH 4 LETTERS */
/* MAXIMAL WORD LENGTH: 3 */
/* NUMBER OF LETTERS (INCLUDING NULL SYMBOL): 5 */
/* NUMBER OF BDD VARIABLES (WITHOUT CONSTANT ONE): 15 */
/* NUMBER OF WORDS: 4 */
/*
#define MAXWORDSIZE 3
#define ALPHABETSIZE 5
#define ALPHABET {'b','i','d','y','.'}
#define NOASCII
#define WITHNULLSYMBOL
#define TOUPPER
#define FILENAME "./bddscout/DICTIONARY/biddy-words.txt"
#ifdef OBDD
#  define OUTPUTNAME "./bddscout/DICTIONARY/biddy-words-OBDD.bddview"
#endif
#ifdef OBDDC
#  define OUTPUTNAME "./bddscout/DICTIONARY/biddy-words-OBDDC.bddview"
#endif
#ifdef ZBDD
#  define OUTPUTNAME "./bddscout/DICTIONARY/biddy-words-ZBDD.bddview"
#endif
#ifdef ZBDDC
#  define OUTPUTNAME "./bddscout/DICTIONARY/biddy-words-ZBDDC.bddview"
#endif
#ifdef TZBDD
#  define OUTPUTNAME "./bddscout/DICTIONARY/biddy-words-TZBDD.bddview"
#endif
*/

/* BENCHMARK 1 */
/* THIS IS A SMALL EXAMPLE USING JUST 12 WORDS AND ALPHABET WITH 4 LETTERS */
/* MAXIMAL WORD LENGTH: 3 (ALL WORDS ARE THE SAME LENGTH) */
/* NUMBER OF LETTERS (NULL SYMBOL IS NOT USED): 4 */
/* NUMBER OF BDD VARIABLES (WITHOUT CONSTANT ONE): 12 */
/* NUMBER OF WORDS: 12 */
/*
#define MAXWORDSIZE 3
#define ALPHABETSIZE 4
#define ALPHABET {'A','B','C','D'}
#define NOASCII
#define NONULLSYMBOL
#define TOUPPER
#define FILENAME "./bddscout/DICTIONARY/abcd-words.txt"
#ifdef OBDD
#  define OUTPUTNAME "./bddscout/DICTIONARY/abcd-words-fixedsize-OBDD.bddview"
#endif
#ifdef OBDDC
#  define OUTPUTNAME "./bddscout/DICTIONARY/abcd-words-fixedsize-OBDDC.bddview"
#endif
#ifdef ZBDD
#  define OUTPUTNAME "./bddscout/DICTIONARY/abcd-words-fixedsize-ZBDD.bddview"
#endif
#ifdef ZBDDC
#  define OUTPUTNAME "./bddscout/DICTIONARY/abcd-words-fixedsize-ZBDDC.bddview"
#endif
#ifdef TZBDD
#  define OUTPUTNAME "./bddscout/DICTIONARY/abcd-words-fixedsize-TZBDD.bddview"
#endif
*/

/* BENCHMARK 2 */
/* THIS IS A SMALL EXAMPLE USING JUST 12 WORDS AND ALPHABET WITH 4 LETTERS */
/* MAXIMAL WORD LENGTH: 3 */
/* NUMBER OF LETTERS (INCLUDING NULL SYMBOL): 5 */
/* NUMBER OF BDD VARIABLES (WITHOUT CONSTANT ONE): 15 */
/* NUMBER OF WORDS: 12 */
/*
#define MAXWORDSIZE 3
#define ALPHABETSIZE 5
#define ALPHABET {'A','B','C','D','.'}
#define NOASCII
#define WITHNULLSYMBOL
#define TOUPPER
#define FILENAME "./bddscout/DICTIONARY/abcd-words.txt"
#ifdef OBDD
#  define OUTPUTNAME "./bddscout/DICTIONARY/abcd-words-OBDD.bddview"
#endif
#ifdef OBDDC
#  define OUTPUTNAME "./bddscout/DICTIONARY/abcd-words-OBDDC.bddview"
#endif
#ifdef ZBDD
#  define OUTPUTNAME "./bddscout/DICTIONARY/abcd-words-ZBDD.bddview"
#endif
#ifdef ZBDDC
#  define OUTPUTNAME "./bddscout/DICTIONARY/abcd-words-ZBDDC.bddview"
#endif
#ifdef TZBDD
#  define OUTPUTNAME "./bddscout/DICTIONARY/abcd-words-TZBDD.bddview"
#endif
*/

/* BENCHMARK 3 */
/* THIS IS A SMALL EXAMPLE USING JUST 12 WORDS AND ALPHABET WITH 5 LETTERS */
/* MAXIMAL WORD LENGTH: 4 */
/* NUMBER OF LETTERS (INCLUDING NULL SYMBOL): 6 */
/* NUMBER OF BDD VARIABLES (WITHOUT CONSTANT ONE): 24 */
/* NUMBER OF WORDS: 12 */
/*
#define MAXWORDSIZE 4
#define ALPHABETSIZE 6
#define ALPHABET {'A','B','C','D','E','.'}
#define NOASCII
#define WITHNULLSYMBOL
#define TOUPPER
#define FILENAME "./bddscout/DICTIONARY/abcd-words.txt"
#ifdef OBDD
#  define OUTPUTNAME "./bddscout/DICTIONARY/abcd-words-OBDD.bddview"
#endif
#ifdef OBDDC
#  define OUTPUTNAME "./bddscout/DICTIONARY/abcd-words-OBDDC.bddview"
#endif
#ifdef ZBDD
#  define OUTPUTNAME "./bddscout/DICTIONARY/abcd-words-ZBDD.bddview"
#endif
#ifdef ZBDDC
#  define OUTPUTNAME "./bddscout/DICTIONARY/abcd-words-ZBDDC.bddview"
#endif
#ifdef TZBDD
#  define OUTPUTNAME "./bddscout/DICTIONARY/abcd-words-TZBDD.bddview"
#endif
*/

/* BENCHMARK 4 */
/* THIS IS A SMALL EXAMPLE USING JUST 12 WORDS */
/* MAXIMAL WORD LENGTH: 5 */
/* NUMBER OF LETTERS (INCLUDING NULL SYMBOL): 54 */
/* NUMBER OF BDD VARIABLES (WITHOUT CONSTANT ONE): 270 */
/* NUMBER OF WORDS: 12 */
/* NULL SYMBOL IS THE LAST SYMBOL - IT IS DENOTED WITH . */
/*
#define MAXWORDSIZE 5
#define ALPHABETSIZE 54
#define ALPHABET {'-', \
                  'A','B','C','D','E','F','G','H','I','J','K','L','M','N','O','P','Q','R','S','T','U','V','W','X','Y','Z', \
                  'a','b','c','d','e','f','g','h','i','j','k','l','m','n','o','p','q','r','s','t','u','v','w','x','y','z', \
                  '.' \
                 }
#define NOASCII
#define WITHNULLSYMBOL
#define NOTOUPPER
#define FILENAME "./bddscout/DICTIONARY/words12.txt"
#ifdef OBDD
#  define OUTPUTNAME "./bddscout/DICTIONARY/words12-OBDD.bddview"
#endif
#ifdef OBDDC
#  define OUTPUTNAME "./bddscout/DICTIONARY/words12-OBDDC.bddview"
#endif
#ifdef ZBDD
#  define OUTPUTNAME "./bddscout/DICTIONARY/words12-ZBDD.bddview"
#endif
#ifdef ZBDDC
#  define OUTPUTNAME "./bddscout/DICTIONARY/words12-ZBDDC.bddview"
#endif
#ifdef TZBDD
#  define OUTPUTNAME "./bddscout/DICTIONARY/words12-TZBDD.bddview"
#endif
*/

/* BENCHMARK 5 */
/* THIS IS A SMALL EXAMPLE USING JUST 12 WORDS */
/* MAXIMAL WORD LENGTH: 5 */
/* NUMBER OF LETTERS (INCLUDING NULL SYMBOL): 129 */
/* NUMBER OF BDD VARIABLES (WITHOUT CONSTANT ONE): 645 */
/* NUMBER OF WORDS: 12 */
/* VARIABLE NAMES ARE NUMBERS, NULL SYMBOL IS THE LAST SYMBOL */
/*
#define MAXWORDSIZE 5
#define ALPHABETSIZE 129
#define ALPHABET {}
#define ASCII
#define WITHNULLSYMBOL
#define NOTOUPPER
#define FILENAME "./bddscout/DICTIONARY/words12.txt"
#ifdef OBDD
#  define OUTPUTNAME "./bddscout/DICTIONARY/words12-ascii-OBDD.bddview"
#endif
#ifdef OBDDC
#  define OUTPUTNAME "./bddscout/DICTIONARY/words12-ascii-OBDDC.bddview"
#endif
#ifdef ZBDD
#  define OUTPUTNAME "./bddscout/DICTIONARY/words12-ascii-ZBDD.bddview"
#endif
#ifdef ZBDDC
#  define OUTPUTNAME "./bddscout/DICTIONARY/words12-ascii-ZBDDC.bddview"
#endif
#ifdef TZBDD
#  define OUTPUTNAME "./bddscout/DICTIONARY/words12-ascii-TZBDD.bddview"
#endif
*/

/* BENCHMARK 6 */
/* THIS IS COMPATIBLE WITH D. E. Knuth's BENCHMARKS */
/* MAXIMAL WORD LENGTH: 5 (ALL WORDS ARE THE SAME LENGTH) */
/* NUMBER OF LETTERS (NULL SYMBOL IS NOT USED): 26 */
/* NUMBER OF BDD VARIABLES (WITHOUT CONSTANT ONE): 130 */
/* NUMBER OF WORDS: 5757 */
/*
#define MAXWORDSIZE 5
#define ALPHABETSIZE 26
#define ALPHABET {'A','B','C','D','E','F','G','H','I','J','K','L','M','N','O','P','Q','R','S','T','U','V','W','X','Y','Z'}
#define NOASCII
#define NONULLSYMBOL
#define TOUPPER
#define FILENAME "./bddscout/DICTIONARY/sgb-words.txt"
#ifdef OBDD
#  define OUTPUTNAME "./bddscout/DICTIONARY/sgb-words-fixedsize-OBDD.bddview"
#endif
#ifdef OBDDC
#  define OUTPUTNAME "./bddscout/DICTIONARY/sgb-words-fixedsize-OBDDC.bddview"
#endif
#ifdef ZBDD
#  define OUTPUTNAME "./bddscout/DICTIONARY/sgb-words-fixedsize-ZBDD.bddview"
#endif
#ifdef ZBDDC
#  define OUTPUTNAME "./bddscout/DICTIONARY/sgb-words-fixedsize-ZBDDC.bddview"
#endif
#ifdef TZBDD
#  define OUTPUTNAME "./bddscout/DICTIONARY/sgb-words-fixedsize-TZBDD.bddview"
#endif
*/

/* BENCHMARK 7 */
/* THIS IS (ALMOST) COMPATIBLE WITH R. Bryant's BENCHMARKS - COMPACT WORD LIST, ONE-HOT VARIABLES */
/* MAXIMAL WORD LENGTH: 24 */
/* NUMBER OF LETTERS (INCLUDING NULL SYMBOL): 54 */
/* NUMBER OF BDD VARIABLES (WITHOUT CONSTANT ONE): 1296 */
/* NUMBER OF WORDS: 235886 */
/* NULL SYMBOL IS THE LAST SYMBOL - IT IS DENOTED WITH . */
/**/
#define MAXWORDSIZE 24
#define ALPHABETSIZE 54
#define ALPHABET {'-', \
                  'A','B','C','D','E','F','G','H','I','J','K','L','M','N','O','P','Q','R','S','T','U','V','W','X','Y','Z', \
                  'a','b','c','d','e','f','g','h','i','j','k','l','m','n','o','p','q','r','s','t','u','v','w','x','y','z', \
                  '.' \
                 }
#define NOASCII
#define WITHNULLSYMBOL
#define NOTOUPPER
#define FILENAME "./bddscout/DICTIONARY/words.txt"
#ifdef OBDD
#  define OUTPUTNAME "./bddscout/DICTIONARY/words-OBDD.bddview"
#endif
#ifdef OBDDC
#  define OUTPUTNAME "./bddscout/DICTIONARY/words-OBDDC.bddview"
#endif
#ifdef ZBDD
#  define OUTPUTNAME "./bddscout/DICTIONARY/words-ZBDD.bddview"
#endif
#ifdef ZBDDC
#  define OUTPUTNAME "./bddscout/DICTIONARY/words-ZBDDC.bddview"
#endif
#ifdef TZBDD
#  define OUTPUTNAME "./bddscout/DICTIONARY/words-TZBDD.bddview"
#endif
/**/

/* BENCHMARK 8 */
/* THIS IS (ALMOST) COMPATIBLE WITH R. Bryant's BENCHMARKS - ASCII WORD LIST, ONE-HOT VARIABLES */
/* MAXIMAL WORD LENGTH: 24 */
/* NUMBER OF LETTERS (INCLUDING NULL SYMBOL): 129 */
/* NUMBER OF BDD VARIABLES (WITHOUT CONSTANT ONE): 3096 */
/* NUMBER OF WORDS: 235886 */
/* VARIABLE NAMES ARE NUMBERS, NULL SYMBOL IS THE LAST SYMBOL */
/*
#define MAXWORDSIZE 24
#define ALPHABETSIZE 129
#define ALPHABET {}
#define ASCII
#define WITHNULLSYMBOL
#define NOTOUPPER
#define FILENAME "./bddscout/DICTIONARY/words.txt"
#ifdef OBDD
#  define OUTPUTNAME "./bddscout/DICTIONARY/words-ascii-OBDD.bddview"
#endif
#ifdef OBDDC
#  define OUTPUTNAME "./bddscout/DICTIONARY/words-ascii-OBDDC.bddview"
#endif
#ifdef ZBDD
#  define OUTPUTNAME "./bddscout/DICTIONARY/words-ascii-ZBDD.bddview"
#endif
#ifdef ZBDDC
#  define OUTPUTNAME "./bddscout/DICTIONARY/words-ascii-ZBDDC.bddview"
#endif
#ifdef TZBDD
#  define OUTPUTNAME "./bddscout/DICTIONARY/words-ascii-TZBDD.bddview"
#endif
*/

/* BENCHMARK 9 */
/* THIS IS USED FOR JOURNAL PAPER, AUTHORS: ROBERT MEOLIC, UROS BERGLEZ, AND GREGOR DONAJ */
/* MAXIMAL WORD LENGTH: 25 */
/* NUMBER OF LETTERS (INCLUDING NULL SYMBOL): 31 */
/* NUMBER OF BDD VARIABLES (WITHOUT CONSTANT ONE): 775 */
/* NUMBER OF WORDS: 301334 */
/* NULL SYMBOL IS THE LAST SYMBOL - IT IS DENOTED WITH . */
/*
#define MAXWORDSIZE 25
#define ALPHABETSIZE 31
#define ALPHABET {'a','b','c','C','d','D','e','f','g','h','i','j','k','l','m','n','o','p','q', \
                  'r','s','S','t','u','v','w','x','y','z','Z', \
                  '.' \
                 }
#define NOASCII
#define WITHNULLSYMBOL
#define NOTOUPPER
#define FILENAME "../donaj/donaj.txt"
#ifdef OBDD
#  define OUTPUTNAME "../donaj/donaj-OBDD.bddview"
#endif
#ifdef OBDDC
#  define OUTPUTNAME "../donaj/donaj-OBDDC.bddview"
#endif
#ifdef ZBDD
#  define OUTPUTNAME "../donaj/donaj-ZBDD.bddview"
#endif
#ifdef ZBDDC
#  define OUTPUTNAME "../donaj/donaj-ZBDDC.bddview"
#endif
#ifdef TZBDD
#  define OUTPUTNAME "../donaj/donaj-TZBDD.bddview"
#endif
*/

#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <time.h>

#include "biddy-example-dict-common.h"

void createLevenshteinTemplate(unsigned int wordlength, unsigned char alphabet[], Biddy_Boolean usenullsymbol, unsigned char nullsymbol, Biddy_Edge code[], Biddy_Edge codenot[], unsigned int alphabetsize, unsigned int maxwordlength);
void createLevenshteinTemplatePerPartes(unsigned int wordlength, unsigned char alphabet[], Biddy_Boolean usenullsymbol, unsigned char nullsymbol, Biddy_Edge code[], Biddy_Edge codenot[], unsigned int alphabetsize, unsigned int maxwordlength);

int main(int argv, char ** argc) {
  clock_t elapsedtime;
  FILE *f;
  unsigned char alphabet[ALPHABETSIZE] = ALPHABET;
  Biddy_Edge code[MAXWORDSIZE*ALPHABETSIZE]; /* BDD variables */
  Biddy_Edge codenot[MAXWORDSIZE*ALPHABETSIZE]; /* BDD variables */
  Biddy_Edge dictionary,nullword;
  Biddy_Boolean usenullsymbol;
  unsigned char nullsymbol;
  Biddy_Edge tmp1,tmp2;
  unsigned int i,j,n;
  unsigned char oneword[MAXWORDSIZE+1]; /* word + \0 */
  unsigned int wordlength;
  Biddy_Boolean eof,stop;
  Biddy_String userinput;
  unsigned int dictionary_idx,nullword_idx;

  /* TESTING */
  /*
  Biddy_Edge *dictionaryByLength;
  */

  setbuf(stdout,NULL);
  userinput = strdup("...");

#ifdef WITHNULLSYMBOL
  if (ALPHABETSIZE < (MAXWORDSIZE+2)) {
    printf("ERROR: ALPHABETSIZE (=%u) MUST BE AT LEAST MAXWORDSIZE+2 (=%u)\n",ALPHABETSIZE,(MAXWORDSIZE+2));
  }
#else
  if (ALPHABETSIZE < (MAXWORDSIZE+1)) {
    printf("ERROR: ALPHABETSIZE (=%u) MUST BE AT LEAST MAXWORDSIZE+1 (=%u)\n",ALPHABETSIZE,(MAXWORDSIZE+1));
  }
#endif

#include "biddy-cudd.c" /* this will initialize BDD manager */

  printf("%s ",Biddy_GetManagerName());

#ifdef WITHNULLSYMBOL
  printf("USING NULL SYMBOL + ");
  usenullsymbol = TRUE;
  nullsymbol = alphabet[ALPHABETSIZE-1];
#else
  usenullsymbol = FALSE;
  nullsymbol = 0;
#endif

#ifdef REVERSEORDER
  printf("USING REVERSE ORDER + ");
#endif

#ifdef ASCII
  printf("USING FULL ASCII TABLE + ");
#endif

#ifdef SIFTING
  printf("USING SIFTING + ");
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

  elapsedtime = clock();

  /* create BDD variables */
  /* NOTE: code[i][j] = code[i*ALPHABETSIZE+j] */
  /* NOTE for Biddy: ZBDDs and TZBDDs variables are added in the reverse order by default */
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

#ifdef ASCII
  {
    char name[6];
    for (i=0; i<MAXWORDSIZE; i++) {
      for (j=0; j<ALPHABETSIZE; j++) {
        code[i*ALPHABETSIZE+j] = Biddy_AddVariableEdge(); /* add unnamed/numbered variable */
        createVarNameAscii(j,i+1,name);
        Biddy_ChangeVariableName(Biddy_VariableTableNum()-1,name); /* rename the last added variable */
        REF(code[i*ALPHABETSIZE+j]);
      }
    }
  }
#else
  {
    char name[4];
    for (i=0; i<MAXWORDSIZE; i++) {
      for (j=0; j<ALPHABETSIZE; j++) {
        code[i*ALPHABETSIZE+j] = Biddy_AddVariableEdge(); /* add unnamed/numbered variable */
        createVarName(alphabet[j],i+1,name);
        Biddy_ChangeVariableName(Biddy_VariableTableNum()-1,name); /* rename the last added variable */
        REF(code[i*ALPHABETSIZE+j]);
        /* printf("(code[%u][%u]=%s(%u))",i,j,name,Biddy_VariableTableNum()-1); */
      }
    }
  }
#endif

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

#ifdef ASCII
  {
    char name[6];
    for (i=0; i<MAXWORDSIZE; i++) {
      for (j=0; j<ALPHABETSIZE; j++) {
        code[(MAXWORDSIZE-i-1)*ALPHABETSIZE+(ALPHABETSIZE-j-1)] = Biddy_AddVariableEdge(); /* add unnamed/numbered variable */
        createVarNameAscii(ALPHABETSIZE-j-1,MAXWORDSIZE-i,name);
        Biddy_ChangeVariableName(Biddy_VariableTableNum()-1,(Biddy_String)name); /* rename the last added variable */
        REF(code[(MAXWORDSIZE-i-1)*ALPHABETSIZE+(ALPHABETSIZE-j-1)]);
      }
    }
  }
#else
  {
    char name[4];
    for (i=0; i<MAXWORDSIZE; i++) {
      for (j=0; j<ALPHABETSIZE; j++) {
        code[(MAXWORDSIZE-i-1)*ALPHABETSIZE+(ALPHABETSIZE-j-1)] = Biddy_AddVariableEdge(); /* add unnamed/numbered variable */
        createVarName(alphabet[ALPHABETSIZE-j-1],MAXWORDSIZE-i,name);
        Biddy_ChangeVariableName(Biddy_VariableTableNum()-1,name); /* rename the last added variable */
        REF(code[(MAXWORDSIZE-i-1)*ALPHABETSIZE+(ALPHABETSIZE-j-1)]);
        /* printf("(code[%u][%u]=%s(%u))",MAXWORDSIZE-i-1,ALPHABETSIZE-j-1,name,Biddy_VariableTableNum()-1); */
      }
    }
  }
#endif

  /* For ZBDDs, by adding new variables all the existing ones are changed. */
  /* Thus, after adding the last variable we have to refresh the array of variables. */
  /* NOTE: Biddy_GetVariableEdge returns variables by the creation time, not by the order used in the BDD */
  /* NOTE for Biddy: user variables start with index 1 */
  /* NOTE for CUDD: user variables start with index 0 */

  if ((Biddy_GetManagerType() == BIDDYTYPEZBDD) || (Biddy_GetManagerType() == BIDDYTYPEZBDDC))
  {
    for (i=0; i<MAXWORDSIZE; i++) {
      for (j=0; j<ALPHABETSIZE; j++) {
#ifdef BIDDY
        /* variable edges in Biddy never become obsolete */
#ifdef REVERSEORDER
        code[i*ALPHABETSIZE+j] = Biddy_GetVariableEdge(i*ALPHABETSIZE+j+1);
#else
        code[i*ALPHABETSIZE+j] = Biddy_GetVariableEdge(MAXWORDSIZE*ALPHABETSIZE-(i*ALPHABETSIZE+j));
#endif
#endif
#ifdef CUDD
        /* variable edges in CUDD are referenced - this is required for ZBDDs */
        DEREF(code[i][j]);
#ifdef REVERSEORDER
        code[i*ALPHABETSIZE+j] = Biddy_GetVariableEdge(MAXWORDSIZE*ALPHABETSIZE-(i*ALPHABETSIZE+j)-1);
#else
        code[i*ALPHABETSIZE+j] = Biddy_GetVariableEdge(i*ALPHABETSIZE+j);
#endif
        REF(code[i*ALPHABETSIZE+j]);
#endif
      }
    }
  }

  /* CREATE NEGATIONS */
  for (i=0; i<MAXWORDSIZE; i++) {
    for (j=0; j<ALPHABETSIZE; j++) {
      char notname[32];
      sprintf(notname,"NOT%s",Biddy_GetVariableName(i*ALPHABETSIZE+j));
      codenot[i*ALPHABETSIZE+j] = Biddy_Not(code[i*ALPHABETSIZE+j]);
      REF(codenot[i*ALPHABETSIZE+j]);
      MARK_X(notname,codenot[i*ALPHABETSIZE+j]);
    }
  }

  nullword = encodeNullWord(code,codenot,ALPHABETSIZE,MAXWORDSIZE);

#ifdef BIDDY
  nullword_idx = MARK_X("NULLWORD",nullword);
#endif

  dictionary = Biddy_GetConstantZero();
  REF(dictionary); /* not needed for Biddy but not wrong */

  /* TESTING */
  /*
  dictionaryByLength = (Biddy_Edge *) malloc(MAXWORDSIZE * sizeof(Biddy_Edge));
  for (i=0; i<MAXWORDSIZE; i++) {
    dictionaryByLength[i] = Biddy_GetConstantZero();
    REF(dictionaryByLength[i]);
  }
  */

  /* DEBUGGING */
  /*
  printf("\nREPORT AFTER THE INITIALIZATION OF ALPHABET\n");
  reportSystem();
  reportDictionary(dictionary,ALPHABETSIZE,MAXWORDSIZE);
  */

  /* START CALCULATION */
  n = 0;
  f = fopen(FILENAME,"r");
  if (!f) {
    printf("\nERROR: FILE %s DOES NOT EXIST\n",FILENAME);
    exit(1);
  }
  printf("CREATING DICTIONARY FOR LIST OF WORDS FROM FILE %s\n",FILENAME);
  eof = FALSE;
  while (!eof) {
    eof = ( fscanf(f,"%s",oneword) == EOF );
    if (!eof) {
      wordlength = (unsigned int) strlen((char *)oneword);
      if (wordlength > MAXWORDSIZE) {
        printf("MAXWORDSIZE TO SMALL, YOU NEED AT LEAST %u\n",(unsigned int)strlen((char *)oneword));
        exit(1);
      }
      n++;

      /* DEBUGGING */
      /*
      printf("WORD %u: <%s>\n",n,oneword);
      reportSystem();
      reportDictionary(dictionary,ALPHABETSIZE,MAXWORDSIZE);
      reportOrdering();
      */

      tmp1 = encodeOneWord(oneword,alphabet,usenullsymbol,nullsymbol,code,codenot,ALPHABETSIZE,MAXWORDSIZE);

      /* DEBUGGING */
      /*
      printf("WORD %s FINISHED\n",oneword);
      reportDictionary(tmp1,ALPHABETSIZE,MAXWORDSIZE);
      */

      /* DEBUGGING */
      /*
      printf("OR<%s/%u>",oneword,wordlength);
      */

      /* DISABLED FOR TESTING */
      /**/
      tmp2 = Biddy_Or(dictionary,tmp1);
      REF(tmp2);
      DEREF(dictionary);
      dictionary = tmp2;
      MARK(dictionary);
      /**/

      /* TESTING */
      /*
      tmp2 = Biddy_Or(dictionaryByLength[wordlength-1],tmp1);
      REF(tmp2);
      DEREF(dictionaryByLength[wordlength-1]);
      dictionaryByLength[wordlength-1] = tmp2;
      for (i=0; i<MAXWORDSIZE; i++) {
        MARK(dictionaryByLength[i]);
      }
      */

      DEREF(tmp1);
      SWEEP();

      /* DEBUGGING */
      /*
      printf("DICTIONARY UPDATED\n");
      reportSystem();
      reportDictionary(dictionary,ALPHABETSIZE,MAXWORDSIZE);
      reportOrdering();
      */

#ifdef SIFTING
      /* PERFORM SIFTING DURING THE CREATION OF DICTIONARY */
      /* THIS SHOULD BE ADAPTED FOR TZBDDs - SIFTING HAS SIDE EFFECTS */
      if ((n % 1000) == 0) {
        Biddy_GlobalSifting(); /* global sifting, sifting is not converge */
      }
      /* DEBUGGING */
      /*
      if ((n % 1000) == 0) {
        printf("[SIFTING-%d]\n",n);
        reportSystem();
        reportDictionary(dictionary,ALPHABETSIZE,MAXWORDSIZE);
        reportOrdering();
      }
      */
#endif

    } /*if (!eof) */
    eof = (fgetc(f) == EOF);

  } /* while (!eof) */

  fclose(f);

  elapsedtime = clock()-elapsedtime;

  printf("\n%u WORDS READ FROM FILE %s\n",n,FILENAME);

  /* REMOVE TMP RESULTS WHICH ARE NOT NEEDED ANYMORE */
  /* ALSO, SET UP dictionary_idx WHICH IS NEEDED FOR TZBDDs */
#ifdef BIDDY
  dictionary_idx = MARK_X("DICT",dictionary);
  Biddy_Purge(); /* dictionary_idx and nullword_idx may change during purge */
  Biddy_FindFormula((Biddy_String)"DICT",&dictionary_idx,&dictionary);
  Biddy_FindFormula((Biddy_String)"NULLWORD",&nullword_idx,&nullword);
#endif

  /* TESTING */
  /*
  {
  unsigned int sum = 0;
  printf("DICTIONARY BY LENGTH: /");
  for (i=0; i<MAXWORDSIZE; i++) {
    sum = sum + Biddy_CountNodes(dictionaryByLength[i]);
    printf("%u/",Biddy_CountNodes(dictionaryByLength[i]));
  }
  printf("\n");
  printf("TOTAL FOR DICTIONARY BY LENGTH: %u nodes\n",sum);
  }
  */

  do {

    /* FOR TZBDDs, TOP EDGE OF dictionary AND nullword MAY CHANGE DURING THE SIFTING */
#ifdef BIDDY
    dictionary = Biddy_GetIthFormula(dictionary_idx);
    nullword = Biddy_GetIthFormula(nullword_idx);
#endif

    if ((toupper(userinput[0]) != 'O') && (toupper(userinput[0]) != 'F')) {
      printf("TIME: %.2f s\n",elapsedtime/(1.0*CLOCKS_PER_SEC));
      reportSystem();
      reportDictionary(dictionary,ALPHABETSIZE,MAXWORDSIZE);
    }

#ifdef INTERACTIVE
    printf("\n[S]IFTING / [C]ONVERGE SIFTING / [O]RDERING / SAVE TO [F]ILE / [L]EVENSHTEIN TEMPLATE / E[X]IT: ");
    if (!scanf("%s",userinput)) printf("ERROR\n");

    elapsedtime = clock();

    if (toupper(userinput[0]) == 'X') {
      stop = TRUE;
    } else if (toupper(userinput[0]) == 'S') {
      Biddy_GlobalSifting(); /* global sifting, sifting is not converge */
      stop = FALSE;
    } else if (toupper(userinput[0]) == 'C') {
      Biddy_GlobalConvergedSifting(); /* global sifting, converge sifting */
      stop = FALSE;
    } else if (toupper(userinput[0]) == 'O') {
      reportOrdering();
      stop = FALSE;
    } else if (toupper(userinput[0]) == 'F') {
      Biddy_Edge dict;

      /* IF NULL SYMBOL IS USED THEN ADD NULL WORD (IT HAS LENGTH 1) TO THE DICTIONARY */
#ifdef WITHNULLSYMBOL
      dict = Biddy_Or(dictionary,nullword);
      REF(dict);
      Biddy_WriteBddview(OUTPUTNAME,dict,"DICT",NULL);
      DEREF(dict);
#else
      Biddy_WriteBddview(OUTPUTNAME,dictionary,"DICT",NULL);
#endif

      /* DEBUGGING */
      /*
      Biddy_WriteDot("dictionary.dot",dictionary,"DICT",-1,FALSE);
      printf("\nCreated graph in dot format.\n");
      printf("Use 'dot -y -Tpng -O dictionary.dot' to visualize BDD for dictionary.\n");
      */
#ifdef WITHNULLSYMBOL
      printf("\nDictionary extended with null word and saved to file %s.\n",OUTPUTNAME);
#else
      printf("\nDictionary saved to file %s.\n",OUTPUTNAME);
#endif
      printf("Use biddy-example-dictman to manipulate this file.\n");
      printf("Use Bdd Scout to view graphical representation of this file.\n");
      stop = FALSE;
    } else if (toupper(userinput[0]) == 'L') {
      unsigned int len;
      printf("Levenshtein templates for word with length: ");
      while (!(scanf("%u",&len)) || len > MAXWORDSIZE) {
        printf("Enter valid length: ");
      }
      if (len) createLevenshteinTemplate(len,alphabet,usenullsymbol,nullsymbol,code,codenot,ALPHABETSIZE,MAXWORDSIZE);
      printf("\n");
      stop = FALSE;
    } else {
      stop = FALSE;
    }
#else
    stop = TRUE;
#endif

    elapsedtime = clock()-elapsedtime;

  } while (!stop);

  free(userinput);

  /* EXIT */

  /* TESTING */
  /*
  for (i=0; i<MAXWORDSIZE; i++) {
    DEREF(dictionaryByLength[i]);
  }
  free(dictionaryByLength);
  */

  DEREF(dictionary);
  for (i=0; i<MAXWORDSIZE; i++) {
    for (j=0; j<ALPHABETSIZE; j++) {
      DEREF(code[i*ALPHABETSIZE+j]);
      DEREF(codenot[i*ALPHABETSIZE+j]);
    }
  }

#ifdef REPORT
#ifdef CUDD
  printf("CUDD: nodes with non-zero reference counts: %d\n",Cudd_CheckZeroRef(manager));
#endif
#endif

  Biddy_Exit();

}

/*******************************************************************************
\brief Function createLevenshteinTemplate creates graphs for a given
        dictionary which can be used to quickly compute subgraphLevenshtein.

### Description
### Side effects
### More info
*******************************************************************************/

void
createLevenshteinTemplate(unsigned int wordlength, unsigned char alphabet[],
  Biddy_Boolean usenullsymbol, unsigned char nullsymbol,
  Biddy_Edge code[], Biddy_Edge codenot[], unsigned int alphabetsize,
  unsigned int maxwordlength)
{
  unsigned char *word;
  unsigned char nonnullsymbol;
  VarTable *vartable;
  Biddy_Edge lgraph,hgraph,igraph,dgraph,fgraph;
  unsigned int i;
  /* char name[255]; */

  if (!usenullsymbol) {
    printf("Without null symbol Levenshtein is not implemented\n");
    return;
  }

#ifdef ASCII
    word = createPatternAscii(alphabet,wordlength);
    nonnullsymbol = 'A' + alphabet[wordlength];
#else
    word = createPattern(alphabet,wordlength);
    nonnullsymbol = alphabet[wordlength];
#endif

  /* DEBUGGING */
  /**/
  printf("\nLEVENSHTEIN TEMPLATES FOR WORD WITH SIZE %u (\"%s\", nonnullsymbol = \"%c\")\n",wordlength,word,nonnullsymbol);
  /**/

  /* vartable contains the table of all variables */
  /* vartable[0] is not used, vartable[i] corresponds to variable Biddy_GetVariableName(i) */
  vartable = createVarTable(alphabetsize,maxwordlength);

  lgraph = encodeOneWord(word,alphabet,usenullsymbol,nullsymbol,code,codenot,alphabetsize,maxwordlength);
  printf("Size of lgraph/0 = %u\n",Biddy_CountNodes(lgraph));
  /* reportSystem(); */ /* PROFILING */

  for (i=1; i<=maxwordlength; i++) {
    if (i > wordlength) {
      igraph = oneInsertion(lgraph,alphabet,vartable,nonnullsymbol,nullsymbol,alphabetsize,maxwordlength);
      lgraph = igraph;
    } else {
      hgraph = oneSubstitution(lgraph,vartable,nonnullsymbol,usenullsymbol,nullsymbol,alphabetsize,maxwordlength);
      printf("hgraph/%u = %u, ",i,Biddy_CountNodes(hgraph));
      /* reportSystem(); */ /* PROFILING */
      igraph = oneInsertion(lgraph,alphabet,vartable,nonnullsymbol,nullsymbol,alphabetsize,maxwordlength);
      printf("igraph/%u = %u, ",i,Biddy_CountNodes(igraph));
      /* reportSystem(); */ /* PROFILING */
      dgraph = oneDeletion(lgraph,alphabet,vartable,nullsymbol,alphabetsize,maxwordlength);
      printf("dgraph/%u = %u, ",i,Biddy_CountNodes(dgraph));
      /* reportSystem(); */ /* PROFILING */
      lgraph = Biddy_Union(lgraph,hgraph);
      lgraph = Biddy_Union(lgraph,igraph);
      lgraph = Biddy_Union(lgraph,dgraph);
    }

    printf("lgraph/%u = %u",i,Biddy_CountNodes(lgraph));
    /* reportSystem(); */ /* PROFILING */

    /* TESTING */
    /*
    if (lgraph == igraph) printf(", IGRAPH FINAL");
    */

    /* DEBUGGING */
    /*
    Biddy_WriteBddview("hgraph.bddview",hgraph,"HGRAPH",NULL);
    Biddy_WriteBddview("igraph.bddview",igraph,"IGRAPH",NULL);
    Biddy_WriteBddview("dgraph.bddview",dgraph,"DGRAPH",NULL);
    */

    /* TESTING */
    /**/
    MARK(lgraph);
    SWEEP();
    fgraph = extendNonNullSymbol(lgraph,alphabet,vartable,nonnullsymbol,usenullsymbol,nullsymbol,alphabetsize,maxwordlength);
    printf(", fgraph/%u = %u",i,Biddy_CountNodes(fgraph));
    /**/

    /* TESTING */
    /*
    if (Biddy_GetManagerType() == BIDDYTYPEOBDD) {
      sprintf(name,"levenshtein-full-OBDD-%u-%u-%u-%u.bddview",alphabetsize,maxwordlength,wordlength,i);
    } else if (Biddy_GetManagerType() == BIDDYTYPEOBDDC) {
      sprintf(name,"levenshtein-full-OBDDC-%u-%u-%u-%u.bddview",alphabetsize,maxwordlength,wordlength,i);
    } else if (Biddy_GetManagerType() == BIDDYTYPEZBDD) {
      sprintf(name,"levenshtein-full-ZBDD-%u-%u-%u-%u.bddview",alphabetsize,maxwordlength,wordlength,i);
    } else if (Biddy_GetManagerType() == BIDDYTYPEZBDDC) {
      sprintf(name,"levenshtein-full-ZBDDC-%u-%u-%u-%u.bddview",alphabetsize,maxwordlength,wordlength,i);
    } else if (Biddy_GetManagerType() == BIDDYTYPETZBDD) {
      sprintf(name,"levenshtein-full-TZBDD-%u-%u-%u-%u.bddview",alphabetsize,maxwordlength,wordlength,i);
    } else if (Biddy_GetManagerType() == BIDDYTYPETZBDDC) {
      sprintf(name,"levenshtein-full-TZBDDC-%u-%u-%u-%u.bddview",alphabetsize,maxwordlength,wordlength,i);
    }
    Biddy_WriteBddview(name,fgraph,name,NULL);
    */

    /* DISABLED FOR TESTING */
    /*
    if (Biddy_GetManagerType() == BIDDYTYPEOBDD) {
      sprintf(name,"levenshtein-template-OBDD-%u-%u-%u-%u.bddview",alphabetsize,maxwordlength,wordlength,i);
    } else if (Biddy_GetManagerType() == BIDDYTYPEOBDDC) {
      sprintf(name,"levenshtein-template-OBDDC-%u-%u-%u-%u.bddview",alphabetsize,maxwordlength,wordlength,i);
    } else if (Biddy_GetManagerType() == BIDDYTYPEZBDD) {
      sprintf(name,"levenshtein-template-ZBDD-%u-%u-%u-%u.bddview",alphabetsize,maxwordlength,wordlength,i);
    } else if (Biddy_GetManagerType() == BIDDYTYPEZBDDC) {
      sprintf(name,"levenshtein-template-ZBDDC-%u-%u-%u-%u.bddview",alphabetsize,maxwordlength,wordlength,i);
    } else if (Biddy_GetManagerType() == BIDDYTYPETZBDD) {
      sprintf(name,"levenshtein-template-TZBDD-%u-%u-%u-%u.bddview",alphabetsize,maxwordlength,wordlength,i);
    } else if (Biddy_GetManagerType() == BIDDYTYPETZBDDC) {
      sprintf(name,"levenshtein-template-TZBDDC-%u-%u-%u-%u.bddview",alphabetsize,maxwordlength,wordlength,i);
    }
    Biddy_WriteBddview(name,lgraph,name,NULL);
    printf("\ncreated %s",name);
    */

    printf("\n");

    MARK(lgraph);
    SWEEP();
  }

  free(word);
  free(vartable);
}

void
createLevenshteinTemplatePerPartes(unsigned int wordlength, unsigned char alphabet[],
  Biddy_Boolean usenullsymbol, unsigned char nullsymbol,
  Biddy_Edge code[], Biddy_Edge codenot[], unsigned int alphabetsize,
  unsigned int maxwordlength)
{
  unsigned char *word;
  unsigned char nonnullsymbol;
  VarTable *vartable;
  Biddy_Edge *lgraph;
  Biddy_Edge *fgraph;
  Biddy_Edge hgraph,igraph,dgraph;
  /* Biddy_Edge llgraph,ffgraph; */
  unsigned int i,j;
  char name[255];

  if (!usenullsymbol) {
    printf("Without null symbol Levenshtein is not implemented\n");
    return;
  }

  lgraph = ( Biddy_Edge *) calloc(maxwordlength+1,sizeof( Biddy_Edge));
  fgraph = ( Biddy_Edge *) calloc(maxwordlength+1,sizeof( Biddy_Edge));

  for (i=0; i<=maxwordlength; i++) {
    lgraph[i] = fgraph[i] = Biddy_GetEmptySet();
  }

#ifdef ASCII
    word = createPatternAscii(alphabet,wordlength);
    nonnullsymbol = 'A' + alphabet[wordlength];
#else
    word = createPattern(alphabet,wordlength);
    nonnullsymbol = alphabet[wordlength];
#endif

  /* DEBUGGING */
  /**/
  printf("\nLEVENSHTEIN TEMPLATES FOR WORD WITH SIZE %u (\"%s\", nonnullsymbol = \"%c\")\n",wordlength,word,nonnullsymbol);
  /**/

  /* vartable contains the table of all variables */
  /* vartable[0] is not used, vartable[i] corresponds to variable Biddy_GetVariableName(i) */
  vartable = createVarTable(alphabetsize,maxwordlength);

  lgraph[0] = encodeOneWord(word,alphabet,usenullsymbol,nullsymbol,code,codenot,alphabetsize,maxwordlength);
  printf("Size of lgraph/0 = %u\n",Biddy_CountNodes(lgraph[0]));
  /* reportSystem(); */ /* PROFILING */

  for (i=1; (i<=maxwordlength) && (i); i++) {
    hgraph = oneSubstitution(lgraph[i-1],vartable,nonnullsymbol,usenullsymbol,nullsymbol,alphabetsize,maxwordlength);
    printf("hgraph/%u = %u, ",i,Biddy_CountNodes(hgraph));
    /* reportSystem(); */ /* PROFILING */
    igraph = oneInsertion(lgraph[i-1],alphabet,vartable,nonnullsymbol,nullsymbol,alphabetsize,maxwordlength);
    printf("igraph/%u = %u, ",i,Biddy_CountNodes(igraph));
    /* reportSystem(); */ /* PROFILING */
    dgraph = oneDeletion(lgraph[i-1],alphabet,vartable,nullsymbol,alphabetsize,maxwordlength);
    printf("dgraph/%u = %u, ",i,Biddy_CountNodes(dgraph));
    /* reportSystem(); */ /* PROFILING */
    lgraph[i] = Biddy_Union(lgraph[i],hgraph);
    lgraph[i] = Biddy_Union(lgraph[i],igraph);
    lgraph[i] = Biddy_Union(lgraph[i],dgraph);
    for (j=0; j<i; j++) {
      lgraph[i] = Biddy_Diff(lgraph[i],lgraph[j]);
    }
    printf("lgraph/%u = %u",i,Biddy_CountNodes(lgraph[i]));
    /* reportSystem(); */ /* PROFILING */

    /* DEBUGGING */
    /*
    Biddy_WriteBddview("hgraph.bddview",hgraph,"HGRAPH",NULL);
    Biddy_WriteBddview("igraph.bddview",igraph,"IGRAPH",NULL);
    Biddy_WriteBddview("dgraph.bddview",dgraph,"DGRAPH",NULL);
    */

    /* TESTING */
    /*
    llgraph = Biddy_GetEmptySet();
    for (j=0; j<=i; j++) {
      llgraph = Biddy_Union(llgraph,lgraph[j]);
    }
    printf("/%u",Biddy_CountNodes(llgraph));
    for (j=0; j<=maxwordlength; j++) {
      MARK(lgraph[j]);
      MARK(fgraph[j]);
    }
    SWEEP();
    fgraph[i] = extendNonNullSymbol(lgraph[i],alphabet,vartable,nonnullsymbol,usenullsymbol,nullsymbol,alphabetsize,maxwordlength);
    printf(", fgraph/%u = %u",i,Biddy_CountNodes(fgraph[i]));
    for (j=0; j<=maxwordlength; j++) {
      MARK(fgraph[j]);
    }
    ffgraph = Biddy_GetEmptySet();
    for (j=0; j<=i; j++) {
      ffgraph = Biddy_Union(ffgraph,fgraph[j]);
    }
    printf("/%u",Biddy_CountNodes(ffgraph));
    */

    /* TESTING */
    /*
    if (Biddy_GetManagerType() == BIDDYTYPEOBDD) {
      sprintf(name,"levenshtein-full-OBDD-%u-%u-%u-%u.bddview",alphabetsize,maxwordlength,wordlength,i);
    } else if (Biddy_GetManagerType() == BIDDYTYPEOBDDC) {
      sprintf(name,"levenshtein-full-OBDDC-%u-%u-%u-%u.bddview",alphabetsize,maxwordlength,wordlength,i);
    } else if (Biddy_GetManagerType() == BIDDYTYPEZBDD) {
      sprintf(name,"levenshtein-full-ZBDD-%u-%u-%u-%u.bddview",alphabetsize,maxwordlength,wordlength,i);
    } else if (Biddy_GetManagerType() == BIDDYTYPEZBDDC) {
      sprintf(name,"levenshtein-full-ZBDDC-%u-%u-%u-%u.bddview",alphabetsize,maxwordlength,wordlength,i);
    } else if (Biddy_GetManagerType() == BIDDYTYPETZBDD) {
      sprintf(name,"levenshtein-full-TZBDD-%u-%u-%u-%u.bddview",alphabetsize,maxwordlength,wordlength,i);
    } else if (Biddy_GetManagerType() == BIDDYTYPETZBDDC) {
      sprintf(name,"levenshtein-full-TZBDDC-%u-%u-%u-%u.bddview",alphabetsize,maxwordlength,wordlength,i);
    }
    Biddy_WriteBddview(name,fgraph[i],name,NULL);
    */

    /* DISABLED FOR TESTING */
    /**/
    if (Biddy_GetManagerType() == BIDDYTYPEOBDD) {
      sprintf(name,"levenshtein-template-OBDD-%u-%u-%u-%u.bddview",alphabetsize,maxwordlength,wordlength,i);
    } else if (Biddy_GetManagerType() == BIDDYTYPEOBDDC) {
      sprintf(name,"levenshtein-template-OBDDC-%u-%u-%u-%u.bddview",alphabetsize,maxwordlength,wordlength,i);
    } else if (Biddy_GetManagerType() == BIDDYTYPEZBDD) {
      sprintf(name,"levenshtein-template-ZBDD-%u-%u-%u-%u.bddview",alphabetsize,maxwordlength,wordlength,i);
    } else if (Biddy_GetManagerType() == BIDDYTYPEZBDDC) {
      sprintf(name,"levenshtein-template-ZBDDC-%u-%u-%u-%u.bddview",alphabetsize,maxwordlength,wordlength,i);
    } else if (Biddy_GetManagerType() == BIDDYTYPETZBDD) {
      sprintf(name,"levenshtein-template-TZBDD-%u-%u-%u-%u.bddview",alphabetsize,maxwordlength,wordlength,i);
    } else if (Biddy_GetManagerType() == BIDDYTYPETZBDDC) {
      sprintf(name,"levenshtein-template-TZBDDC-%u-%u-%u-%u.bddview",alphabetsize,maxwordlength,wordlength,i);
    }
    Biddy_WriteBddview(name,lgraph[i],name,NULL);
    printf("\ncreated %s",name);
    /**/

    printf("\n");

    for (j=0; j<=maxwordlength; j++) {
      MARK(lgraph[j]);
    }
    SWEEP();
  }

  free(lgraph);
  free(fgraph);

  free(word);
  free(vartable);
}

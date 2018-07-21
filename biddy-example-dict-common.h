/* $Revision: 441 $ */
/* $Date: 2018-05-22 12:58:10 +0200 (tor, 22 maj 2018) $ */
/* This file (biddy-example-dict-common.h) is a C header file */
/* Author: Robert Meolic (robert.meolic@um.si) */
/* This file has been released into the public domain by the author. */

#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <time.h>

#include "biddy-cudd.h"

/* BDD CREATION IS OPTIMIZED BY DEFAULT */
/* YOU CAN OVERRIDE DEFAULTS BY USING COMPILER DIRECTIVES */
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
#if !defined(BOTTOMUP) && !defined(TOPDOWN)
#  define BOTTOMUP
#endif

typedef struct {
  unsigned char x;
  unsigned int y;
} VarTable;

void createVarName(unsigned char x, unsigned int y, char varname[]);
void createVarNameAscii(unsigned char x, unsigned int y, char varname[]);
unsigned int getLetterIndex(unsigned char alphabet[], unsigned int alphabetsize, unsigned char c);
Biddy_Boolean isNullSymbol(unsigned int v, VarTable *vartable, unsigned char nullsymbol);
Biddy_String createPattern(unsigned char alphabet[], unsigned int wordlength);
Biddy_String createPatternAscii(unsigned char alphabet[], unsigned int wordlength);
VarTable * createVarTable(unsigned int alphabetsize, unsigned int maxwordlength);
void reportOrdering();
void reportSystem();
void reportDictionary(Biddy_Edge d, unsigned int alphabetsize, unsigned int maxwordlength);
void reportAlphabet(unsigned char alphabet[], unsigned int alphabetsize);
void reportCodes(unsigned char alphabet[], Biddy_Edge c[], unsigned int alphabetsize, unsigned int maxwordlength);
Biddy_Edge encodeNullWord(Biddy_Edge code[], Biddy_Edge codenot[], unsigned int alphabetsize, unsigned int maxwordlength);
Biddy_Edge encodeOneWord(unsigned char oneword[], unsigned char alphabet[], Biddy_Boolean usenullsymbol, unsigned char nullsymbol, Biddy_Edge code[], Biddy_Edge codenot[], unsigned int alphabetsize, unsigned int maxwordlength);
Biddy_Edge encodeOneWordBryant(unsigned char oneword[], unsigned char alphabet[], Biddy_Boolean usenullsymbol, unsigned char nullsymbol, Biddy_Edge code[], Biddy_Edge codenot[], unsigned int alphabetsize, unsigned int maxwordlength);
Biddy_Edge oneSubstitution(Biddy_Edge ngraph, VarTable *vartable, unsigned char nonnullsymbol, Biddy_Boolean usenullsymbol, unsigned char nullsymbol, unsigned int alphabetsize, unsigned int maxwordlength);
Biddy_Edge oneSubstitutionFull(Biddy_Edge ngraph, VarTable *vartable, Biddy_Boolean usenullsymbol, unsigned char nullsymbol, unsigned int alphabetsize, unsigned int maxwordlength);
Biddy_Edge oneInsertion(Biddy_Edge ngraph, unsigned char alphabet[], VarTable *vartable, unsigned char nonnullsymbol, unsigned char nullsymbol, unsigned int alphabetsize, unsigned int maxwordlength);
Biddy_Edge oneInsertionFull(Biddy_Edge ngraph, unsigned char alphabet[], VarTable *vartable, unsigned char nullsymbol, unsigned int alphabetsize, unsigned int maxwordlength);
Biddy_Edge oneDeletion(Biddy_Edge ngraph, unsigned char alphabet[], VarTable *vartable, unsigned char nullsymbol, unsigned int alphabetsize, unsigned int maxwordlength);
Biddy_Edge extendNonNullSymbol(Biddy_Edge graph, unsigned char alphabet[], VarTable *vartable, unsigned char nonnullsymbol, Biddy_Boolean usenullsymbol, unsigned char nullsymbol, unsigned int alphabetsize, unsigned int maxwordlength);

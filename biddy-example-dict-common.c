/* $Revision: 455 $ */
/* $Date: 2018-07-14 12:11:54 +0200 (sob, 14 jul 2018) $ */
/* This file (biddy-example-dict-common.c) is a C file */
/* Author: Robert Meolic (robert.meolic@um.si) */
/* This file has been released into the public domain by the author. */

#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <time.h>

#include <biddy-example-dict-common.h>

/* DEFAULT IS BOTTOMUP */
/* TOPDOWN APPROACH MUST BE COMPENSATED BY INVERTING REVERSEORDERCALC */
#ifdef REVERSEORDER
#  define REVERSEORDERCALC
#endif
#ifdef TOPDOWN
#ifdef REVERSEORDERCALC
#  undef REVERSEORDERCALC
#else
#  define REVERSEORDERCALC
#endif
#endif

/*******************************************************************************
\brief Function createVarName.

### Description
### Side effects
### More info
*******************************************************************************/

void
createVarName(unsigned char x, unsigned int y, char varname[])
{
  varname[0] = x;
  varname[1] = '0' + y/10;
  varname[2] = '0' + y%10;
  varname[3] = 0; /* end of string, max word length = 99 */
}

/*******************************************************************************
\brief Function createVarNameAscii.

### Description
### Side effects
### More info
*******************************************************************************/

void
createVarNameAscii(unsigned char x, unsigned int y, char varname[])
{
  varname[0] = '0' + x/100;
  varname[1] = '0' + (x/10)%10;
  varname[2] = '0' + x%10;
  varname[3] = '0' + y/10;
  varname[4] = '0' + y%10;
  varname[5] = 0; /* end of string, max word length = 99 */
}

/*******************************************************************************
\brief Function getLetterIndex.

### Description
### Side effects
### More info
*******************************************************************************/

unsigned int
getLetterIndex(unsigned char alphabet[], unsigned int alphabetsize, unsigned char c)
{
  unsigned int i;
  Biddy_Boolean OK;

  OK = FALSE;
  for (i=0; i<alphabetsize; i++) {
    if (alphabet[i] == c) {
      OK = TRUE;
      break;
    }
  }
  if (!OK) {
    printf("getLetterIndex: unknown letter\n");
    exit(1);
  }
  return i;
}

/*******************************************************************************
\brief Function isNullSymbol.

### Description
### Side effects
### More info
*******************************************************************************/

Biddy_Boolean
isNullSymbol(unsigned int v, VarTable *vartable, unsigned char nullsymbol)
{
  return (vartable[v].x == nullsymbol);
}


/*******************************************************************************
\brief Function createPattern.

### Description
### Side effects
### More info
*******************************************************************************/

Biddy_String
createPattern(unsigned char alphabet[], unsigned int wordlength)
{
  Biddy_String word;
  unsigned int i;

  word = (unsigned char *) calloc(wordlength+1,sizeof(unsigned char));
  for (i=0; i<wordlength; i++) {
    word[i] = alphabet[i];
  }
  return word;
}

/*******************************************************************************
\brief Function createPatternAscii.

### Description
### Side effects
### More info
*******************************************************************************/

Biddy_String
createPatternAscii(unsigned char alphabet[], unsigned int wordlength)
{
  Biddy_String word;
  unsigned int i;

  word = (unsigned char *) calloc(wordlength+1,sizeof(unsigned char));
  for (i=0; i<wordlength; i++) {
    word[i] = 'A' + alphabet[i];
  }
  return word;
}

/*******************************************************************************
\brief Function createVarTable.

### Description
### Side effects
### More info
*******************************************************************************/

VarTable *
createVarTable(unsigned int alphabetsize, unsigned int maxwordlength)
{
  VarTable *vartable;
  unsigned int i;
  Biddy_String name;

  vartable = (VarTable *) malloc((alphabetsize*maxwordlength+1)*sizeof(VarTable));
  for (i=1; i<(alphabetsize*maxwordlength+1); i++) {
    name = Biddy_GetVariableName(i);
    if (strlen(name) == 3) {
      vartable[i].x = name[0];
      sscanf(&name[1],"%u",&(vartable[i].y));
    }
    else if (strlen(name) == 5) {
      vartable[i].x = 100*(name[0]-'0')+10*(name[1]-'0')+(name[2]-'0');
      sscanf(&name[3],"%u",&(vartable[i].y));
    }
    else {
      printf("File error - wrong variable name (%s)!\n",name);
      exit(1);
    }
  }

  return vartable;
}

/*******************************************************************************
\brief Function reportOrdering.

### Description
### Side effects
### More info
*******************************************************************************/

void
reportOrdering()
{
  Biddy_Variable var;

#ifdef BIDDY
  printf("Variable ordering:\n");
  var = Biddy_GetLowestVariable();
  while (var != 0) {
    printf("(%s)",Biddy_GetVariableName(var));
    var = Biddy_GetNextVariable(var);
  }
  printf("\n");
#endif
}

/*******************************************************************************
\brief Function reportSystem.

### Description
### Side effects
### More info
*******************************************************************************/

void
reportSystem()
{
  printf("%s\n",Biddy_GetManagerName());
#ifdef BIDDY
#ifdef MINGW
  printf("Memory in use: %I64u B\n",Biddy_ReadMemoryInUse());
#else
  printf("Memory in use: %llu B\n",Biddy_ReadMemoryInUse());
#endif
#endif
  printf("Number of OPCacheSearch: %llu\n",Biddy_OPCacheSearch());
#ifdef CUDD
  printf("Memory in use: %lu B\n",Biddy_ReadMemoryInUse());
#endif
  printf("Node table has %u nodes / %lu live nodes.\n",Biddy_NodeTableNum(),Biddy_NodeTableNumLive());
#ifdef BIDDY
  printf("Variable table has %u user variables.\n",Biddy_VariableTableNum()-1);
#endif
#ifdef CUDD
  printf("Variable table has %u user variables.\n",Biddy_VariableTableNum());
#endif
#ifdef REPORT
  Biddy_PrintInfo(stdout);
#endif
}

/*******************************************************************************
\brief Function reportDictionary.

### Description
### Side effects
### More info
*******************************************************************************/

void
reportDictionary(Biddy_Edge d, unsigned int alphabetsize, unsigned int maxwordlength)
{
  printf("Dictionary depends on %u variables.\n",Biddy_DependentVariableNumber(d,FALSE));
  printf("Dictionary has %.0f minterms.\n",Biddy_CountMinterms(d,maxwordlength*alphabetsize));
  printf("Dictionary has %u node(s) / %u plain node(s).\n",Biddy_CountNodes(d),Biddy_CountNodesPlain(d));
}

/*******************************************************************************
\brief Function reportAlphabet.

### Description
### Side effects
### More info
*******************************************************************************/

void
reportAlphabet(unsigned char alphabet[], unsigned int alphabetsize)
{
  unsigned int i;

  printf("ALPHABET:\n");
  for (i=0; i<alphabetsize; i++) {
    if ((alphabet[i] >= 32) && (alphabet[i] <= 126)) {
      printf("(%c)",alphabet[i]);
    } else {
      printf("(#%u)",alphabet[i]);
    }
  }
  printf("\n");
}

/*******************************************************************************
\brief Function reportCodes.

### Description
### Side effects
### More info
*******************************************************************************/

void
reportCodes(unsigned char alphabet[], Biddy_Edge c[], unsigned int alphabetsize, unsigned int maxwordlength)
{
  unsigned int i,j;

  printf("CODES:\n");
  for (i=0; i<maxwordlength; i++) {
    for (j=0; j<alphabetsize; j++) {
      printf("[%u][%u]%c\n",i,j,alphabet[j]);
      Biddy_PrintfSOP(c[i*alphabetsize+j]);
    }
  }
  printf("\n");
}

/*******************************************************************************
\brief Function encodeNullWord creates graph for word with exactly 1 letter,
        this single letter is a null symbol.

### Description
### Side effects
### More info
*******************************************************************************/

Biddy_Edge
encodeNullWord(Biddy_Edge code[], Biddy_Edge codenot[],
               unsigned int alphabetsize, unsigned int maxwordlength)
{
  Biddy_Edge nullword,tmp;
  unsigned int i,j;

  nullword = code[alphabetsize-1]; /* the first null symbol */
  REF(nullword); /* not needed for Biddy but not wrong */
  for (i=0; i<maxwordlength; i++) {
    for (j=0; j<alphabetsize; j++) {
      if (i*alphabetsize+j != alphabetsize-1) {
        tmp = Biddy_And(nullword,codenot[i*alphabetsize+j]);
        /* printf("AND[%u][%u]",i,j); */
        REF(tmp); /* not needed for Biddy but not wrong */
        DEREF(nullword);
        nullword = tmp;
      }
    }
  }
  return nullword;
}

/*******************************************************************************
\brief Function encodeOneWord creates graph for the given word.

### Description
### Side effects
### More info
*******************************************************************************/

Biddy_Edge
encodeOneWord(unsigned char oneword[], unsigned char alphabet[],
  Biddy_Boolean usenullsymbol, unsigned char nullsymbol, Biddy_Edge code[],
  Biddy_Edge codenot[], unsigned int alphabetsize, unsigned int maxwordlength)
{
  Biddy_Edge tmp1,tmp2;
  unsigned int i,j;
  unsigned int letter;
  unsigned int wordlength;

  wordlength = 0;
  while (oneword[wordlength] != 0) wordlength++;

  /* DEBUGGING */
  /*
  printf("createOneWord: wordlength = %u\n",wordlength);
  */

  tmp1 = Biddy_GetConstantOne();
  REF(tmp1); /* not needed for Biddy but not wrong */

/* IF NOT REVERSEORDERCALC THEN ADD UNUSED LETTERS FIRST BECAUSE THEY ARE AT THE BOTTOM PART OF THE BDD */
/* IF NULL SYMBOL IS USED THEN UNUSED LETTERS ARE MARKED WITH IT */
/* THIS IS USED BY DEFAULT FOR OBDD, OBDDC, TZBDD, AND TZBDDC */
#ifndef REVERSEORDERCALC
  if (!usenullsymbol) {
    i = maxwordlength-1;
    while (i >= wordlength) { /* word's length cannot be 0 */
      for (j=alphabetsize-1; j<alphabetsize; j--) { /* j is unsigned! */
        tmp2 = Biddy_And(tmp1,codenot[i*alphabetsize+j]);
        /* printf("AND[%u][%u]",i,j); */
        REF(tmp2);
        DEREF(tmp1);
        tmp1 = tmp2;
      }
      i--;
    }
  } else {
    i = maxwordlength-1;
    while ((i<maxwordlength) && (i >= wordlength)) { /* word's length can be 0, i is unsigned! */
      j=alphabetsize-1; /* null symbol is the last symbol */
      tmp2 = Biddy_And(tmp1,code[i*alphabetsize+j]);
      /* printf("AND[%u][%u]",i,j); */
      REF(tmp2);
      DEREF(tmp1);
      tmp1 = tmp2;
      j--;
      for (; j<alphabetsize; j--) { /* all other symbols, j is unsigned! */
        tmp2 = Biddy_And(tmp1,codenot[i*alphabetsize+j]);
        /* printf("AND[%u][%u]",i,j); */
        REF(tmp2);
        DEREF(tmp1);
        tmp1 = tmp2;
      }
      i--;
    }
  }
#endif

/* ADD ALL LETTERS FROM THE WORD */
#ifdef REVERSEORDERCALC
  for (i=0; i<wordlength; i++)
#else
  for (; i<wordlength; i--) /* i is unsigned */
#endif
  {

#ifdef TOUPPER
    letter = toupper(oneword[i]);
#else
    letter = oneword[i];
#endif
#ifdef REVERSEORDERCALC
    for (j=0; j<alphabetsize; j++)
#else
    for (j=alphabetsize-1; j<alphabetsize; j--) /* j is unsigned */
#endif
    {
      if (letter == alphabet[j]) {
        tmp2 = Biddy_And(tmp1,code[i*alphabetsize+j]);
      } else {
        tmp2 = Biddy_And(tmp1,codenot[i*alphabetsize+j]);
      }
      /* printf("AND[%u][%u]",i,j); */
      REF(tmp2);
      DEREF(tmp1);
      tmp1 = tmp2;
    }
  }

/* IF REVERSEORDERCALC THEN ADD UNUSED LETERS LAST BECAUSE THEY ARE AT THE TOP PART OF THE BDD */
/* IF NULL SYMBOL IS USED THEN UNUSED LETTERS ARE MARKED WITH IT */
/* THIS IS USED BY DEFAULT FOR ZBDD AND ZBDDC */
#ifdef REVERSEORDERCALC
  if (!usenullsymbol) {
    while (i < maxwordlength) {
      for (j=0; j<alphabetsize; j++) {
        tmp2 = Biddy_And(tmp1,codenot[i*alphabetsize+j]);
        /* printf("AND[%u][%u]",i,j); */
        REF(tmp2);
        DEREF(tmp1);
        tmp1 = tmp2;
      }
      i++;
    }
  } else {
    while (i < maxwordlength) {
      for (j=0; j<alphabetsize-1; j++) {
        tmp2 = Biddy_And(tmp1,codenot[i*alphabetsize+j]); /* all other symbols */
        /* printf("AND[%u][%u]",i,j); */
        REF(tmp2);
        DEREF(tmp1);
        tmp1 = tmp2;
      }
      tmp2 = Biddy_And(tmp1,code[i*alphabetsize+j]); /* null symbol is the last symbol */
      /* printf("AND[%u][%u]",i,j); */
      REF(tmp2);
      DEREF(tmp1);
      tmp1 = tmp2;
      i++;
    }
  }
#endif

  return tmp1;
}

/*******************************************************************************
\brief Function encodeOneWordBryant is an old implementation of createOneWord.

### Description
### Side effects
### More info
*******************************************************************************/

Biddy_Edge
encodeOneWordBryant(unsigned char oneword[], unsigned char alphabet[],
  Biddy_Boolean usenullsymbol, unsigned char nullsymbol, Biddy_Edge code[],
  Biddy_Edge codenot[], unsigned int alphabetsize, unsigned int maxwordlength)
{
  Biddy_Edge tmp1,tmp2;
  unsigned int i,j;
  unsigned int letter;
  unsigned int wordlength;

  wordlength = 0;
  while (oneword[wordlength] != 0) wordlength++;

  /* DEBUGGING */
  /*
  printf("encodeOneWordBryant: wordlength = %u\n",wordlength);
  */

  tmp1 = Biddy_GetConstantOne();
  REF(tmp1); /* not needed for Biddy but not wrong */

/* IF NOT REVERSEORDERCALC THEN ADD UNUSED VARIABLES FIRST BECAUSE THEY ARE AT THE BOTTOM PART OF THE BDD */
/* THIS IS USED BY DEFAULT FOR OBDD, OBDDC, TZBDD, AND TZBDDC */
#ifndef REVERSEORDERCALC
  i = maxwordlength-1;
  while (i > wordlength) {
    for (j=alphabetsize-1; j<alphabetsize; j--) { /* j is unsigned */
      tmp2 = Biddy_And(tmp1,codenot[i*alphabetsize+j]);
      /* printf("AND[%u][%u]",i,j); */
      REF(tmp2);
      DEREF(tmp1);
      tmp1 = tmp2;
    }
    i--;
  }
#endif

/* IF NOT REVERSEORDERCALC THEN ADD NULL SYMBOL BEFORE THE WORD BECAUSE THIS VARIABLE IS BOTTOM-MORE */
/* THIS IS USED BY DEFAULT FOR OBDD, OBDDC, TZBDD, AND TZBDDC */
/* TO DO: NULL SYMBOL MAY NOT BE THE LAST VARIABLE IN THE ALPHABET */
#ifndef REVERSEORDERCALC
  if (usenullsymbol) {
    if (wordlength < maxwordlength) {
      j=alphabetsize-1;
      tmp2 = Biddy_And(tmp1,code[i*alphabetsize+j]);
      /* printf("AND[%u][%u]",i,j); */
      REF(tmp2);
      DEREF(tmp1);
      tmp1 = tmp2;
      j--;
      for (; j<alphabetsize; j--) { /* j is unsigned */
        tmp2 = Biddy_And(tmp1,codenot[i*alphabetsize+j]);
        /* printf("AND[%u][%u]",i,j); */
        REF(tmp2);
        DEREF(tmp1);
        tmp1 = tmp2;
      }
      i--;
    }
  }
#endif

/* ADD ALL LETTERS FROM THE WORD */
#ifdef REVERSEORDERCALC
  for (i=0; i<wordlength; i++)
#else
  for (; i<wordlength; i--) /* i is unsigned */
#endif
  {

#ifdef TOUPPER
    letter = toupper(oneword[i]);
#else
    letter = oneword[i];
#endif
#ifdef REVERSEORDERCALC
    for (j=0; j<alphabetsize; j++)
#else
    for (j=alphabetsize-1; j<alphabetsize; j--) /* j is unsigned */
#endif
    {
      if (letter == alphabet[j]) {
        tmp2 = Biddy_And(tmp1,code[i*alphabetsize+j]);
      } else {
        tmp2 = Biddy_And(tmp1,codenot[i*alphabetsize+j]);
      }
      /* printf("AND[%u][%u]",i,j); */
      REF(tmp2);
      DEREF(tmp1);
      tmp1 = tmp2;
    }

    /* DEBUGGING */
    /*
    printf("encodeOneWordBryant before after letter <%c>, here is result:\n",letter);
    Biddy_PrintfSOP(tmp1);
    */
  }

/* IF REVERSEORDERCALC THEN ADD NULL SYMBOL AFTER THE WORD BECAUSE THIS VARIABLE IS TOP-MORE */
/* THIS IS USED BY DEFAULT FOR ZBDD AND ZBDDC */
/* TO DO: NULL SYMBOL MAY NOT BE THE LAST VARIABLE IN THE ALPHABET */
#ifdef REVERSEORDERCALC
  if (usenullsymbol) {
    if (i < maxwordlength) {
      for (j=0; j<alphabetsize-1; j++) {
        tmp2 = Biddy_And(tmp1,codenot[i*alphabetsize+j]);
        /* printf("AND[%u][%u]",i,j); */
        REF(tmp2);
        DEREF(tmp1);
        tmp1 = tmp2;
      }
      tmp2 = Biddy_And(tmp1,code[i*alphabetsize+j]);
      REF(tmp2);
      DEREF(tmp1);
      tmp1 = tmp2;
      /* printf("AND[%u][%u]",i,j); */
      i++;
    }
  }
#endif

/* IF REVERSEORDERCALC THEN ADD UNUSED VARIABLES LAST BECAUSE THEY ARE AT THE TOP PART OF THE BDD */
/* THIS IS USED BY DEFAULT FOR ZBDD AND ZBDDC */
#ifdef REVERSEORDERCALC
  while (i < maxwordlength) {
    for (j=0; j<alphabetsize; j++) {
      tmp2 = Biddy_And(tmp1,codenot[i*alphabetsize+j]);
      /* printf("AND[%u][%u]",i,j); */
      REF(tmp2);
      DEREF(tmp1);
      tmp1 = tmp2;
    }
    i++;
  }
#endif

  return tmp1;
}

/*******************************************************************************
\brief Function oneSubstitution creates a graph representing one substituion.

### Description
    ngraph is a dictionary representing one or more words.
### Side effects
### More info
*******************************************************************************/

Biddy_Edge
oneSubstitution(Biddy_Edge ngraph, VarTable *vartable,
  unsigned char nonnullsymbol,
  Biddy_Boolean usenullsymbol, unsigned char nullsymbol,
  unsigned int alphabetsize, unsigned int maxwordlength)
{
  Biddy_Edge cgraph,cube,base,nonnull,result;
  unsigned int i,j;

  result = ngraph;

  for (i=1; i<=maxwordlength; i++) {

    /* existential quantification of i-th letter */
    nonnull = Biddy_GetEmptySet();
    base = Biddy_GetUniversalSet();
    for (j=1; j<(alphabetsize*maxwordlength+1); j++) {
      if (vartable[j].y == i) base = Biddy_Diff(base,Biddy_GetVariableEdge(j));
    }
    if ((Biddy_GetManagerType() == BIDDYTYPEZBDD) || (Biddy_GetManagerType() == BIDDYTYPEZBDDC))
      cube = Biddy_GetBaseSet(); else cube = Biddy_GetUniversalSet();
    for (j=1; j<(alphabetsize*maxwordlength+1); j++) {
      if (vartable[j].y == i) {
        if (!usenullsymbol || !isNullSymbol(j,vartable,nullsymbol)) {
          if ((Biddy_GetManagerType() == BIDDYTYPEZBDD) || (Biddy_GetManagerType() == BIDDYTYPEZBDDC))
            cube = Biddy_Change(cube,j); else cube = Biddy_Intersect(cube,Biddy_GetVariableEdge(j));
        }
        /* nonnull represents a non-null i-th symbol - this is used later */
        if (vartable[j].x == nonnullsymbol) {
          nonnull = Biddy_Change(base,j);
        }
      }
    }
    cgraph = Biddy_ExistAbstract(ngraph,cube);

    /* DEBUGGING */
    /*
    {
    Biddy_String name;
    name = strdup("afterX.bddview");
    name[5] = '0' + i;
    Biddy_WriteBddview(name,Biddy_Intersect(cgraph,nonnull),name,NULL);
    free(name);
    }
    */

    result = Biddy_Union(result,Biddy_Intersect(cgraph,nonnull));
  }

  return result;
}

Biddy_Edge
oneSubstitutionFull(Biddy_Edge ngraph, VarTable *vartable,
  Biddy_Boolean usenullsymbol, unsigned char nullsymbol,
  unsigned int alphabetsize, unsigned int maxwordlength)
{
  Biddy_Edge cgraph,cube,base,nonnull,result;
  unsigned int i,j;

  result = ngraph;

  for (i=1; i<=maxwordlength; i++) {

    /* existential quantification of i-th letter */
    nonnull = Biddy_GetEmptySet();
    base = Biddy_GetUniversalSet();
    for (j=1; j<(alphabetsize*maxwordlength+1); j++) {
      if (vartable[j].y == i) base = Biddy_Diff(base,Biddy_GetVariableEdge(j));
    }
    if ((Biddy_GetManagerType() == BIDDYTYPEZBDD) || (Biddy_GetManagerType() == BIDDYTYPEZBDDC))
      cube = Biddy_GetBaseSet(); else cube = Biddy_GetUniversalSet();
    for (j=1; j<(alphabetsize*maxwordlength+1); j++) {
      if (vartable[j].y == i) {
        if (!usenullsymbol || !isNullSymbol(j,vartable,nullsymbol)) {
          if ((Biddy_GetManagerType() == BIDDYTYPEZBDD) || (Biddy_GetManagerType() == BIDDYTYPEZBDDC))
            cube = Biddy_Change(cube,j); else cube = Biddy_Intersect(cube,Biddy_GetVariableEdge(j));
          /* nonnull represents a non-null i-th symbol - this is used later */
          nonnull = Biddy_Union(nonnull,Biddy_Change(base,j));
        }
      }
    }
    cgraph = Biddy_ExistAbstract(ngraph,cube);

    /* DEBUGGING */
    /*
    {
    Biddy_String name;
    name = strdup("afterX.bddview");
    name[5] = '0' + i;
    Biddy_WriteBddview(name,Biddy_Intersect(cgraph,code),name,NULL);
    free(name);
    }
    */

    result = Biddy_Union(result,Biddy_Intersect(cgraph,nonnull));
  }

  return result;
}

/*******************************************************************************
\brief Function oneInsertion creates a graph representing one insertion.

### Description
    ngraph is a dictionary representing one or more words.
### Side effects
### More info
*******************************************************************************/

Biddy_Edge
oneInsertion(Biddy_Edge ngraph, unsigned char alphabet[], VarTable *vartable,
  unsigned char nonnullsymbol,
  unsigned char nullsymbol, unsigned int alphabetsize,
  unsigned int maxwordlength)
{
  Biddy_Edge cube,cube2,code,ngraph2;
  Biddy_Edge null_i1,null_i2,base_i1,base_i2,base_max,nonnull_i1,nonnull_i2,nonnull_max;
  Biddy_Edge result;
  Biddy_Edge *nextcode;
  unsigned int i,j;

  result = ngraph;

  /* WORD OF MAX LENGTH CANNOT BE PROLONGED */
  /* WORD OF (MAX LENGTH - 1) CAN BE PROLONGED */
  cube = cube2 = Biddy_GetEmptySet();
  base_max = Biddy_GetUniversalSet();
  for (j=1; j<(alphabetsize*maxwordlength+1); j++) {
    if ((vartable[j].y == maxwordlength) && !isNullSymbol(j,vartable,nullsymbol)) {
      cube = Biddy_Union(cube,Biddy_GetVariableEdge(j));
    }
    if ((vartable[j].y == (maxwordlength-1)) && !isNullSymbol(j,vartable,nullsymbol)) {
      cube2 = Biddy_Union(cube2,Biddy_GetVariableEdge(j));
    }
    if (vartable[j].y == maxwordlength) base_max = Biddy_Diff(base_max,Biddy_GetVariableEdge(j));
  }
  ngraph = Biddy_Diff(ngraph,cube); /* remove all words with max length */

  if (ngraph == Biddy_GetEmptySet()) return result;

  nextcode = (Biddy_Edge *) malloc(alphabetsize * sizeof(Biddy_Edge));

  /* existential quantification of all symbols at the maxwordlength-th place */
  nonnull_max = Biddy_GetEmptySet();
  if ((Biddy_GetManagerType() == BIDDYTYPEZBDD) || (Biddy_GetManagerType() == BIDDYTYPEZBDDC))
    cube = Biddy_GetBaseSet(); else cube = Biddy_GetUniversalSet();
  for (j=1; j<(alphabetsize*maxwordlength+1); j++) {
    if (vartable[j].y == maxwordlength) {
      if ((Biddy_GetManagerType() == BIDDYTYPEZBDD) || (Biddy_GetManagerType() == BIDDYTYPEZBDDC))
        cube = Biddy_Change(cube,j); else cube = Biddy_And(cube,Biddy_GetVariableEdge(j));
      /* nonnull_max represents a non-null maxwordlength-th symbol - this is used later */
      if (vartable[j].x == nonnullsymbol) {
        nonnull_max = Biddy_Change(base_max,j);
      }
    }
  }
  ngraph = Biddy_ExistAbstract(ngraph,cube);
  ngraph2 = Biddy_Intersect(ngraph,cube2); /* all words with (max length -1), without last letter, used later */

  for (i=maxwordlength; i>1; i--) {

    /* substitution, all letters at (i-1)-th place are moved to i-th place */
    base_i1 = Biddy_GetUniversalSet();
    base_i2 = Biddy_GetUniversalSet();
    null_i1 = nonnull_i1 = Biddy_GetEmptySet();
    if (i == 2) {
      null_i2 = Biddy_GetEmptySet();
      nonnull_i2 = Biddy_GetUniversalSet();
    } else {
      null_i2 = nonnull_i2 = Biddy_GetEmptySet();
    }
    for (j=1; j<(alphabetsize*maxwordlength+1); j++) {
      if (vartable[j].y == (i-1)) base_i1 = Biddy_Diff(base_i1,Biddy_GetVariableEdge(j));
      if (vartable[j].y == (i-2)) base_i2 = Biddy_Diff(base_i2,Biddy_GetVariableEdge(j));
    }
    Biddy_ResetVariablesValue();
    for (j=1; j<(alphabetsize*maxwordlength+1); j++) {
      if (vartable[j].y == i) {
        /* Biddy_Replace is controlled by variable's values (which are edges!). */
        /* for OBDDs and TZBDDs control values must be variables, but for ZBDDs they must be elements! */
        nextcode[getLetterIndex(alphabet,alphabetsize,vartable[j].x)] =
          ((Biddy_GetManagerType() == BIDDYTYPEZBDD) || (Biddy_GetManagerType() == BIDDYTYPEZBDDC)) ?
            Biddy_GetElementEdge(j) : Biddy_GetVariableEdge(j);
      }
    }
    for (j=1; j<(alphabetsize*maxwordlength+1); j++) {
      if (vartable[j].y == (i-1)) {
        /* (i-1)-th variable will be replaced with corresponding i-th variable determined before */
        Biddy_SetVariableValue(j,nextcode[getLetterIndex(alphabet,alphabetsize,vartable[j].x)]);
        /* null_i1 represents null (i-1)-th symbol - this is used later */
        /* nonnull_i1 represents a non-null (i-1)-th symbol - this is used later */
        if (isNullSymbol(j,vartable,nullsymbol)) {
          null_i1 = Biddy_Change(base_i1,j);
        }
        if (vartable[j].x == nonnullsymbol) {
          nonnull_i1 = Biddy_Change(base_i1,j);
        }
      }
      if (vartable[j].y == (i-2)) {
        /* null_i2 represents null (i-2)-th symbol - this is used later */
        /* nonnull_i2 represents all-but-null (i-2)-th symbols - this is used later */
        if (isNullSymbol(j,vartable,nullsymbol)) {
          null_i2 = Biddy_Change(base_i2,j);
          nonnull_i2 = Biddy_Not(Biddy_GetVariableEdge(j));
        }
      }
    }
    ngraph = Biddy_Replace(ngraph);

    code = Biddy_Intersect(null_i2,null_i1);
    code = Biddy_Union(code,Biddy_Intersect(nonnull_i2,nonnull_i1));

    /* DEBUGGING */
    /*
    {
    Biddy_String name;
    name = strdup("afterX.bddview");
    name[5] = '0' + i;
    Biddy_WriteBddview(name,Biddy_Intersect(ngraph,code),name,NULL);
    free(name);
    }
    */

    result = Biddy_Union(result,Biddy_Intersect(ngraph,code));
  }

  /* add words of (maxwordlength-1) prolonged with 1 non-null letter */
  result = Biddy_Union(result,Biddy_Intersect(ngraph2,nonnull_max));

  free(nextcode);
  return result;
}

Biddy_Edge
oneInsertionFull(Biddy_Edge ngraph, unsigned char alphabet[], VarTable *vartable,
  unsigned char nullsymbol, unsigned int alphabetsize,
  unsigned int maxwordlength)
{
  Biddy_Edge cube,cube2,code,ngraph2;
  Biddy_Edge null_i1,null_i2,base_i1,base_i2,base_max,nonnull_i1,nonnull_i2,nonnull_max;
  Biddy_Edge result;
  Biddy_Edge *nextcode;
  unsigned int i,j;

  result = ngraph;

  /* WORD OF MAX LENGTH CANNOT BE PROLONGED */
  /* WORD OF (MAX LENGTH - 1) CAN BE PROLONGED */
  cube = cube2 = Biddy_GetEmptySet();
  base_max = Biddy_GetUniversalSet();
  for (j=1; j<(alphabetsize*maxwordlength+1); j++) {
    if ((vartable[j].y == maxwordlength) && !isNullSymbol(j,vartable,nullsymbol)) {
      cube = Biddy_Union(cube,Biddy_GetVariableEdge(j));
    }
    if ((vartable[j].y == (maxwordlength-1)) && !isNullSymbol(j,vartable,nullsymbol)) {
      cube2 = Biddy_Union(cube2,Biddy_GetVariableEdge(j));
    }
    if (vartable[j].y == maxwordlength) base_max = Biddy_Diff(base_max,Biddy_GetVariableEdge(j));
  }
  ngraph = Biddy_Diff(ngraph,cube); /* remove all words with max length */

  if (ngraph == Biddy_GetEmptySet()) return result;

  nextcode = (Biddy_Edge *) malloc(alphabetsize * sizeof(Biddy_Edge));

  /* existential quantification of all symbols at the maxwordlength-th place */
  nonnull_max = Biddy_GetEmptySet();
  if ((Biddy_GetManagerType() == BIDDYTYPEZBDD) || (Biddy_GetManagerType() == BIDDYTYPEZBDDC))
    cube = Biddy_GetBaseSet(); else cube = Biddy_GetUniversalSet();
  for (j=1; j<(alphabetsize*maxwordlength+1); j++) {
    if (vartable[j].y == maxwordlength) {
      if ((Biddy_GetManagerType() == BIDDYTYPEZBDD) || (Biddy_GetManagerType() == BIDDYTYPEZBDDC))
        cube = Biddy_Change(cube,j); else cube = Biddy_And(cube,Biddy_GetVariableEdge(j));
      /* nonnull_max represents a non-null maxwordlength-th symbol - this is used later */
      if (!isNullSymbol(j,vartable,nullsymbol)) {
        nonnull_max = Biddy_Union(nonnull_max,Biddy_Change(base_max,j));
      }
    }
  }
  ngraph = Biddy_ExistAbstract(ngraph,cube);
  ngraph2 = Biddy_Intersect(ngraph,cube2); /* all words with (max length -1), without last letter, used later */

  for (i=maxwordlength; i>1; i--) {

    /* substitution, all letters at (i-1)-th place are moved to i-th place */
    base_i1 = Biddy_GetUniversalSet();
    base_i2 = Biddy_GetUniversalSet();
    null_i1 = nonnull_i1 = Biddy_GetEmptySet();
    if (i == 2) {
      null_i2 = Biddy_GetEmptySet();
      nonnull_i2 = Biddy_GetUniversalSet();
    } else {
      null_i2 = nonnull_i2 = Biddy_GetEmptySet();
    }
    for (j=1; j<(alphabetsize*maxwordlength+1); j++) {
      if (vartable[j].y == (i-1)) base_i1 = Biddy_Diff(base_i1,Biddy_GetVariableEdge(j));
      if (vartable[j].y == (i-2)) base_i2 = Biddy_Diff(base_i2,Biddy_GetVariableEdge(j));
    }
    Biddy_ResetVariablesValue();
    for (j=1; j<(alphabetsize*maxwordlength+1); j++) {
      if (vartable[j].y == i) {
        /* Biddy_Replace is controlled by variable's values (which are edges!). */
        /* for OBDDs and TZBDDs control values must be variables, but for ZBDDs they must be elements! */
        nextcode[getLetterIndex(alphabet,alphabetsize,vartable[j].x)] =
          ((Biddy_GetManagerType() == BIDDYTYPEZBDD) || (Biddy_GetManagerType() == BIDDYTYPEZBDDC)) ?
            Biddy_GetElementEdge(j) : Biddy_GetVariableEdge(j);
      }
    }
    for (j=1; j<(alphabetsize*maxwordlength+1); j++) {
      if (vartable[j].y == (i-1)) {
        /* (i-1)-th variable will be replaced with corresponding i-th variable determined before */
        Biddy_SetVariableValue(j,nextcode[getLetterIndex(alphabet,alphabetsize,vartable[j].x)]);
        /* null_i1 represents null (i-1)-th symbol - this is used later */
        /* nonnull_i1 represents a non-null (i-1)-th symbol - this is used later */
        if (isNullSymbol(j,vartable,nullsymbol)) {
          null_i1 = Biddy_Change(base_i1,j);
        } else {
          nonnull_i1 = Biddy_Union(nonnull_i1,Biddy_Change(base_i1,j));
        }
      }
      if (vartable[j].y == (i-2)) {
        /* null_i2 represents null (i-2)-th symbol - this is used later */
        /* nonnull_i2 represents a non-null (i-2)-th symbol - this is used later */
        if (isNullSymbol(j,vartable,nullsymbol)) {
          null_i2 = Biddy_Change(base_i2,j);
        } else {
          nonnull_i2 = Biddy_Union(nonnull_i2,Biddy_Change(base_i2,j));
        }
      }
    }
    ngraph = Biddy_Replace(ngraph);

    code = Biddy_Intersect(null_i1,null_i2);
    code = Biddy_Union(code,Biddy_Intersect(nonnull_i1,nonnull_i2));

    /* DEBUGGING */
    /*
    {
    Biddy_String name;
    name = strdup("afterX.bddview");
    name[5] = '0' + i;
    Biddy_WriteBddview(name,Biddy_Intersect(ngraph,code),name,NULL);
    free(name);
    }
    */

    result = Biddy_Union(result,Biddy_Intersect(ngraph,code));
  }

  /* add words of (maxwordlength-1) prolonged with 1 non-null letter */
  result = Biddy_Union(result,Biddy_Intersect(ngraph2,nonnull_max));

  free(nextcode);
  return result;
}

/*******************************************************************************
\brief Function oneDeletion creates a graph representing one deletion.

### Description
    ngraph is a dictionary representing one or more words.
### Side effects
### More info
*******************************************************************************/

Biddy_Edge
oneDeletion(Biddy_Edge ngraph, unsigned char alphabet[], VarTable *vartable,
  unsigned char nullsymbol, unsigned int alphabetsize,
  unsigned int maxwordlength)
{

  Biddy_Edge cgraph,cube,code,empty,result;
  Biddy_Edge *nextcode;
  unsigned int i,j,k;

  nextcode = (Biddy_Edge *) malloc(alphabetsize * sizeof(Biddy_Edge));

  result = ngraph;

  empty = Biddy_GetEmptySet();
  for (i=1; i<=maxwordlength; i++) {

    /* existential quantification of all letters at the i-th place */
    code = Biddy_GetUniversalSet();
    if ((Biddy_GetManagerType() == BIDDYTYPEZBDD) || (Biddy_GetManagerType() == BIDDYTYPEZBDDC))
      cube = Biddy_GetBaseSet(); else cube = Biddy_GetUniversalSet();
    for (j=1; j<(alphabetsize*maxwordlength+1); j++) {
      if (vartable[j].y == i) {
        if ((Biddy_GetManagerType() == BIDDYTYPEZBDD) || (Biddy_GetManagerType() == BIDDYTYPEZBDDC))
          cube = Biddy_Change(cube,j); else cube = Biddy_And(cube,Biddy_GetVariableEdge(j));
      }
      /* code for the last letter - this is used later */
      if (vartable[j].y == maxwordlength) {
        if (isNullSymbol(j,vartable,nullsymbol)) {
          code = Biddy_Intersect(code,Biddy_GetVariableEdge(j));
        } else {
          code = Biddy_Diff(code,Biddy_GetVariableEdge(j));
        }
      }
      /* code for the first null symbol - this is used later */
      if ((i == 1) && (vartable[j].y == 1) && isNullSymbol(j,vartable,nullsymbol)) {
        empty = Biddy_GetVariableEdge(j);
      }
    }
    cgraph = Biddy_ExistAbstract(ngraph,cube);

    /* substitution, all letters at (i+1)-th, (i+2)-th, ... place are moved one place left */
    for (j=1; j<(alphabetsize*maxwordlength+1); j++) {
      if (vartable[j].y == i) {
        /* Biddy_Replace is controlled by variable's values (which are edges!). */
        /* for OBDDs and TZBDDs control values must be variables, but for ZBDDs they must be elements! */
        nextcode[getLetterIndex(alphabet,alphabetsize,vartable[j].x)] =
          ((Biddy_GetManagerType() == BIDDYTYPEZBDD) || (Biddy_GetManagerType() == BIDDYTYPEZBDDC)) ?
            Biddy_GetElementEdge(j) : Biddy_GetVariableEdge(j);
      }
    }
    for (j=i+1; j<=maxwordlength; j++) {
      Biddy_ResetVariablesValue();
      for (k=1; k<(alphabetsize*maxwordlength+1); k++) {
        if (vartable[k].y == j) {
          Biddy_SetVariableValue(k,nextcode[getLetterIndex(alphabet,alphabetsize,vartable[k].x)]);
          /* Biddy_Replace is controlled by variable's values (which are edges!). */
          /* for OBDDs and TZBDDs control values must be variables, but for ZBDDs they must be elements! */
          nextcode[getLetterIndex(alphabet,alphabetsize,vartable[k].x)] =
            ((Biddy_GetManagerType() == BIDDYTYPEZBDD) || (Biddy_GetManagerType() == BIDDYTYPEZBDDC)) ?
              Biddy_GetElementEdge(k) : Biddy_GetVariableEdge(k);
        }
      }
      cgraph = Biddy_Replace(cgraph);
    }

    /* empty word is not allowed */
    if (i == 1) {
      cgraph = Biddy_Diff(cgraph,empty);
    }

    /* DEBUGGING */
    /*
    {
    Biddy_String name;
    name = strdup("afterX.bddview");
    name[5] = '0' + i;
    Biddy_WriteBddview(name,Biddy_Intersect(cgraph,code),name,NULL);
    free(name);
    }
    */

    result = Biddy_Union(result,Biddy_Intersect(cgraph,code));
  }

  free(nextcode);

  return result;
}

/*******************************************************************************
\brief Function extendNonNullSymbol.

### Description
### Side effects
### More info
*******************************************************************************/

/* use EXTENDUP or EXTENDDOWN */
/* EXTENDDOWN seems to be MUCH better for TZBDD */
#define EXTENDDOWN

Biddy_Edge
extendNonNullSymbol(Biddy_Edge graph, unsigned char alphabet[],
  VarTable *vartable, unsigned char nonnullsymbol,
  Biddy_Boolean usenullsymbol, unsigned char nullsymbol,
  unsigned int alphabetsize, unsigned int maxwordlength)
{
  Biddy_Edge *ngraph;
  Biddy_Edge *base;
  Biddy_Edge cube,graph1,graph0;
  unsigned int i,j;

  ngraph = (Biddy_Edge *) malloc((maxwordlength+1) * sizeof(Biddy_Edge));
  base = (Biddy_Edge *) malloc((maxwordlength+1) * sizeof(Biddy_Edge));

  for (i=1; i<=maxwordlength; i++) {
    base[i] = Biddy_GetUniversalSet();
    ngraph[i] = Biddy_GetEmptySet();
  }
  for (j=1; j<(alphabetsize*maxwordlength+1); j++) {
    base[vartable[j].y] = Biddy_Diff(base[vartable[j].y],Biddy_GetVariableEdge(j));
  }

  /* ngraph[i] represents a nonnull i-th symbol */
  for (j=1; j<(alphabetsize*maxwordlength+1); j++) {
    if (!usenullsymbol || !isNullSymbol(j,vartable,nullsymbol)) {
      ngraph[vartable[j].y] = Biddy_Union(ngraph[vartable[j].y],Biddy_Change(base[vartable[j].y],j));
    }
  }

#ifdef EXTENDUP
  for (i=1; i<=maxwordlength; i++) {
#endif
#ifdef EXTENDDOWN
  for (i=maxwordlength; i>0; i--) {
#endif
    graph1 = graph0 = NULL;
    for (j=1; j<(alphabetsize*maxwordlength+1); j++) {
      if ((vartable[j].y == i) && (vartable[j].x == nonnullsymbol)) {
        graph1 = Biddy_Subset1(graph,j); /* words with i-th nonnull symbol */
        graph0 = Biddy_Subset0(graph,j); /* words without i-th nonnull symbol */
        break;
      }
    }
    if (graph1 == Biddy_GetEmptySet()) continue;

    /* existential quantification of i-th letter in graph1 */
    if ((Biddy_GetManagerType() == BIDDYTYPEZBDD) || (Biddy_GetManagerType() == BIDDYTYPEZBDDC))
      cube = Biddy_GetBaseSet(); else cube = Biddy_GetUniversalSet();
    for (j=1; j<(alphabetsize*maxwordlength+1); j++) {
      if (vartable[j].y == i) {
        if (!usenullsymbol || !isNullSymbol(j,vartable,nullsymbol)) {
          if ((Biddy_GetManagerType() == BIDDYTYPEZBDD) || (Biddy_GetManagerType() == BIDDYTYPEZBDDC))
            cube = Biddy_Change(cube,j); else cube = Biddy_Intersect(cube,Biddy_GetVariableEdge(j));
        }
      }
    }
    graph1 = Biddy_ExistAbstract(graph1,cube);

    graph1 = Biddy_Intersect(graph1,ngraph[i]);
    graph = Biddy_Union(graph0,graph1);
  }

  free(ngraph);
  free(base);

  return graph;
}

/* $Revision$ */
/* $Date$ */
/* This file (biddy-example-benchmarks.c) is a C file */
/* Author: Robert Meolic (robert@meolic.com) */
/* This file has been released into the public domain by the author. */

/* This example is compatible with Biddy v2.0 and laters */

/* COMPILE WITH (ADD -lgmp IF USING STATIC BIDDY LIBRARY): */
/* gcc -O2 -o biddy-example-benchmarks biddy-example-benchmarks.c -I. -L./bin -lbiddy */

/* this example demonstrates the ability of Biddy to read various benchmark files */

#include "biddy.h"
#include "string.h"
#include <time.h>

/* choose primary BDD type */
#define BDDTYPE BIDDYTYPEOBDD

static void
concat(Biddy_String *s1, const Biddy_String s2)
{
   *s1 = (Biddy_String) realloc(*s1,strlen(*s1)+strlen(s2)+1);
   strcat(*s1,s2);
}

static void
concatNumber(Biddy_String *s1, const int n)
{
   char s2[32];
   sprintf(s2,"%d",n);
   *s1 = (Biddy_String) realloc(*s1,strlen(*s1)+strlen(s2)+1);
   strcat(*s1,s2);
}

static void
concatStatic(Biddy_String s1, const Biddy_String s2)
{
   strcat(s1,s2);
}

/* ************************************************************************** */
/* REPORT (TO BE INCLUDED IN THE NEXT VERSION OF BIDDY) */
/* ************************************************************************** */

void BiddyReport()
{
  unsigned int i,num;
  Biddy_String name;
  Biddy_Edge bdd;
  unsigned int nodes;
  double minterms;

  num = Biddy_SystemStat(BIDDYSTATFORMULATABLENUM);

  for (i=0; i<num; i++) {
    name = Biddy_GetIthFormulaName(i);
    bdd = Biddy_GetIthFormula(i);
    nodes = Biddy_CountNodes(bdd);
    minterms = Biddy_CountMinterms(bdd,-1);
    printf("%u. %s, ",i,name);
    printf("%u node",nodes);
    if (nodes != 1) printf("s, "); else printf(", ");
    printf("%.0f minterm",minterms);
    if (minterms != 1) printf("s");
    printf("\n");

    /* DEBUGGING */
    /* Biddy_PrintfSOP(bdd); */
    /* Biddy_WriteBddview(name,bdd,name,NULL); */
  }
}

/* ************************************************************************** */
/* EQV_BDD (TO BE INCLUDED IN THE NEXT VERSION OF BIDDY) */
/* ************************************************************************** */

Biddy_Boolean BiddyEqvBdd(Biddy_String name1, Biddy_String name2)
{
  Biddy_Edge bdd1,bdd2;
  unsigned int idx;
  Biddy_Boolean OK;

  OK = TRUE;

  if (OK) OK = Biddy_FindFormula(name1,&idx,&bdd1);
  if (OK) OK = Biddy_FindFormula(name2,&idx,&bdd2);
  if (OK) OK = (bdd1 == bdd2);

  return OK;
}

/* ************************************************************************** */
/* READ_PREFIXFORMAT (TO BE INCLUDED IN THE NEXT VERSION OF BIDDY) */
/* ************************************************************************** */

void BiddyReadPrefixFormat(const char filename[], Biddy_String prefix)
{
  FILE *infile;
  char* buffer;
  size_t len,index;
  ssize_t bytes_read;
  int i,j,par;
  Biddy_String sname,prefixname,s;
  Biddy_Edge bdd;

  infile = fopen(filename,"r");
  if (!infile) {
    printf("ERROR: File %s does not exists!\n",filename);
    return;
  }

  /* this is from */
  /* https://stackoverflow.com/questions/174531/how-to-read-the-content-of-a-file-to-a-string-in-c */
  buffer = NULL;
  len = 0;
  bytes_read = getdelim(&buffer,&len,'\0',infile);
  if (bytes_read == -1) {
    printf("ERROR: File %s cannot be read!\n",filename);
    free(buffer);
    return;
  }

  /* DEBUGGING */
  /*
  printf("%s",buffer);
  */

  /* buffer MAY INCLUDE MANY BOOLEAN FUNCTIONS */
  /* FIND INDIVIDUAL BOOLEAN FUNCTION */
  index = 0;
  len = strlen(buffer); /* getdelim() returns len, but for me, it is wrong */
  while (index != len) {
    while ((index < len) && (buffer[index] == ' ' || buffer[index] == '\t' || buffer[index] == '\n')) index++;
    if (index == len) break;

    /* NAME */

    i = index;
    while ((index < len) &&
           (buffer[index] != ' ') && (buffer[index] != '\t') && (buffer[index] != '\n') &&
           (buffer[index] != '='))
    {
      /* printf("%c",buffer[index]); */ /* debugging */
      index++;
    }

    if (index == len) break;

    /* printf("%c",buffer[index]); */ /* debugging */
    j = index-1;

    /* printf("NAME: i = %d '%c', j = %d '%c', len = %lu\n",i,buffer[i],j,buffer[j],len); */ /* debugging */

    sname = malloc(j-i+2);
    memcpy(sname,&buffer[i],j-i+1);
    sname[j-i+1] = 0;
    /* printf("%s\n",sname); */ /* debugging */

    while ((index < len) && (buffer[index] != '='))
    {
      /* printf("%c",buffer[index]); */ /* debugging */
      index++;
    }

    if (index == len) {
      printf("ERROR: Error in file %s!\n",filename);
      return;
    }

    index++;

    /* BOOLEAN FUNCTION */

    while ((index < len) && (buffer[index] != '(')) {
      /* printf("%c",buffer[index]); */ /* debugging */
      index++;
    }
    /* printf("%c",buffer[index]); */ /* debugging */

    if (index == len) {
      printf("ERROR: Error in file %s!\n",filename);
      return;
    }

    i = index++; /* skip '(' */
    par = 1;
    while ((index < len) && (par > 0)) {
      /* printf("%c",buffer[index]); */ /* debugging */
      if (buffer[index] == '(') par++;
      if (buffer[index] == ')') par--;
      index++;
    }
    /* printf("%c",buffer[index]); */ /* debugging */
    j = index-1;

    /* printf("FUNCTION: i = %d '%c', j = %d '%c', len = %lu\n",i,buffer[i],j,buffer[j],len); */ /* debugging */

    s = malloc(j-i+2);
    memcpy(s,&buffer[i],j-i+1);
    s[j-i+1] = 0;
    /* printf("%s\n",s); */ /* debugging */

    bdd = Biddy_Eval1(s);

    if (prefix) {
      prefixname = strdup(prefix);
      concat(&prefixname,sname);
      Biddy_AddPersistentFormula(prefixname,bdd);
      free(prefixname);
    } else {
      Biddy_AddPersistentFormula(sname,bdd);
    }

    free(sname);
    free(s);
  }

  free(buffer);
  fclose(infile);
}

/* ************************************************************************** */
/* READ_BENCHFORMAT (TO BE INCLUDED IN THE NEXT VERSION OF BIDDY) */
/* ************************************************************************** */

void BiddyReadBenchFormat(const char filename[], Biddy_String prefix)
{
  FILE *infile;
  char* buffer;
  size_t len,index;
  ssize_t bytes_read;
  int i,j,par;
  Biddy_String sname,prefixname,s;
  Biddy_Edge bdd;

  infile = fopen(filename,"r");
  if (!infile) {
    printf("ERROR: File %s does not exists!\n",filename);
    return;
  }

  /* this is from */
  /* https://stackoverflow.com/questions/174531/how-to-read-the-content-of-a-file-to-a-string-in-c */
  buffer = NULL;
  len = 0;
  bytes_read = getdelim(&buffer,&len,'\0',infile);
  if (bytes_read == -1) {
    printf("ERROR: File %s has wrong format!\n",filename);
    return;
  }

  /* DEBUGGING */
  /**/
  printf("%s",buffer);
  /**/

  free(buffer);
  fclose(infile);
}

/* ************************************************************************** */
/* READ_INFIXFORMAT (TO BE INCLUDED IN THE NEXT VERSION OF BIDDY) */
/* ************************************************************************** */

void BiddyReadInfixFormat(const char filename[], Biddy_String prefix)
{
  FILE *infile;
  char* buffer;
  size_t len,index;
  ssize_t bytes_read;
  int i,j,par;
  Biddy_String sname,prefixname,s;
  Biddy_Edge bdd;

  infile = fopen(filename,"r");
  if (!infile) {
    printf("ERROR: File %s does not exists!\n",filename);
    return;
  }

  /* this is from */
  /* https://stackoverflow.com/questions/174531/how-to-read-the-content-of-a-file-to-a-string-in-c */
  buffer = NULL;
  len = 1;
  bytes_read = getdelim(&buffer,&len,'\0',infile);
  if (bytes_read == -1) {
    printf("ERROR: File %s has wrong format!\n",filename);
    return;
  }

  /* DEBUGGING */
  /*
  printf("%s",buffer);
  */

  /* buffer MAY INCLUDE MANY BOOLEAN FUNCTIONS */
  /* FIND INDIVIDUAL BOOLEAN FUNCTION */
  index = 0;
  len = strlen(buffer); /* getdelim() returns len, but for me, it is wrong */
  while (index != len) {
    while ((index < len) && (buffer[index] == ' ' || buffer[index] == '\t' || buffer[index] == '\r' || buffer[index] == '\n')) index++;
    if (index == len) break;

    /* NAME */

    i = index;
    while ((index < len) &&
           (buffer[index] != ' ') && (buffer[index] != '\t') && (buffer[index] != '\n') &&
           (buffer[index] != '='))
    {
      /* printf("%c",buffer[index]); */ /* debugging */
      index++;
    }

    if (index == len) break;

    /* printf("%c",buffer[index]); */ /* debugging */
    j = index-1;

    /* printf("NAME: i = %d '%c', j = %d '%c', len = %lu\n",i,buffer[i],j,buffer[j],len); */ /* debugging */

    sname = malloc(j-i+2);
    memcpy(sname,&buffer[i],j-i+1);
    sname[j-i+1] = 0;
    /* printf("%s\n",sname); */ /* debugging */

    while ((index < len) && (buffer[index] != '='))
    {
      /* printf("%c",buffer[index]); */ /* debugging */
      index++;
    }

    if (index == len) {
      printf("ERROR: Error in file %s!\n",filename);
      return;
    }

    index++;

    while ((index < len) &&
           ((buffer[index] == ' ') || (buffer[index] == '\t') || (buffer[index] == '\n')))
    {
      /* printf("%c",buffer[index]); */ /* debugging */
      index++;
    }

    if (index == len) {
      printf("ERROR: Error in file %s!\n",filename);
      return;
    }

    /* BOOLEAN FUNCTION */

    i = index;
    while ((index < len) && (buffer[index] != ';')) {
      /* printf("%c",buffer[index]); */ /* debugging */
      index++;
    }
    /* printf("%c",buffer[index]); */ /* debugging */
    j = index-1;
    if (index < len) index++; /* skip ';' */

    /* printf("FUNCTION: i = %d '%c', j = %d '%c', len = %lu\n",i,buffer[i],j,buffer[j],len); */ /* debugging */

    s = malloc(j-i+2);
    memcpy(s,&buffer[i],j-i+1);
    s[j-i+1] = 0;
    /* printf("%s\n",s); */ /* debugging */

    bdd = Biddy_Eval2(s);

    if (prefix) {
      prefixname = strdup(prefix);
      concat(&prefixname,sname);
      Biddy_AddPersistentFormula(prefixname,bdd);
      free(prefixname);
    } else {
      Biddy_AddPersistentFormula(sname,bdd);
    }

    free(sname);
    free(s);
  }

  free(buffer);
  fclose(infile);
}

/* ************************************************************************** */
/* MAIN PROGRAM */
/* ************************************************************************** */

int main(int argc, char **argv) {
  clock_t elapsedtime;
  FILE *benchfile;
  Biddy_String s;
  Biddy_Edge f;

  Biddy_String prefixFile,benchFile,infixFile,vhdlFile;
  
  prefixFile = strdup("./benchmarks/c17.be");
  benchFile = strdup("./benchmarks/c17.bench");
  infixFile = strdup("./benchmarks/c17.txt");
  vhdlFile = strdup("./benchmarks/c17.v");
 
  /* ================================================================ */
  /* READ PREFIX FORMAT */
  /* AS USED IN: IMEC-IFIP International Workshop on Applied Formal */
  /* Methods For Correct VLSI Design (L. Claesen, ed.), Houthalen, */
  /* Belgium, pp. 568-569, November 1989. */
  /* ================================================================ */

  Biddy_InitAnonymous(BDDTYPE);
  printf("BiddyReadPrefixFormat %s using %s...\n",prefixFile,Biddy_GetManagerName());

  BiddyReadPrefixFormat(prefixFile,NULL);
  Biddy_SetAlphabeticOrdering();

  /* TESTING */
  /**/
  if (!BiddyEqvBdd("G16","G16x")) printf("ERROR: BiddyReadPrefixFormat returns wrong result!\n");
  if (!BiddyEqvBdd("G17","G17x")) printf("ERROR: BiddyReadPrefixFormat returns wrong result!\n");
  /**/

  BiddyReport();
  /* Biddy_PrintInfo(NULL); */
  Biddy_Exit();

  /* ================================================================ */
  /* READ BENCH FORMAT */
  /* AS USED IN ISCAS89 netlist format */
  /* Note: Variables started with number are prefixed with letter 'G' */
  /* See: https://people.engr.ncsu.edu/brglez/CBL/benchmarks/index.html */
  /* See: http://www.pld.ttu.ee/~maksim/benchmarks */
  /* See: https://filebox.ece.vt.edu/~mhsiao/iscas89.html */
  /* See: https://ddd.fit.cvut.cz/prj/Benchmarks/ */
  /* Atalanta-M 2.0: https://ddd.fit.cvut.cz/prj/Atalanta-M/index.php?page=manual */
  /* ================================================================ */

  Biddy_InitAnonymous(BDDTYPE);
  printf("BiddyReadBenchFormat %s using %s...\n",benchFile,Biddy_GetManagerName());

  BiddyReadBenchFormat(benchFile,NULL);
  Biddy_SetAlphabeticOrdering();

  /* TESTING */
  /**/
  if (!BiddyEqvBdd("G16","G26")) printf("ERROR: BiddyReadBenchFormat returns wrong result!\n");
  if (!BiddyEqvBdd("G17","G37")) printf("ERROR: BiddyReadBenchFormat returns wrong result!\n");
  /**/

  BiddyReport();
  /* Biddy_PrintInfo(NULL); */
  Biddy_Exit();

  /* ================================================================ */
  /* READ INFIX FORMAT */
  /* ================================================================ */

  Biddy_InitAnonymous(BDDTYPE);
  printf("BiddyReadInfixFormat %s using %s...\n",infixFile,Biddy_GetManagerName());

  BiddyReadInfixFormat(infixFile,NULL);
  Biddy_SetAlphabeticOrdering();

  /* TESTING */
  /**/
  if (!BiddyEqvBdd("G16","G16x")) printf("ERROR: BiddyReadInfixFormat returns wrong result!\n");
  if (!BiddyEqvBdd("G17","G17x")) printf("ERROR: BiddyReadInfixFormat returns wrong result!\n");
  /**/

  BiddyReport();
  /* Biddy_PrintInfo(NULL); */
  Biddy_Exit();

  /* ================================================================ */
  /* READ VHDL FORMAT */
  /* ================================================================ */

  Biddy_InitAnonymous(BDDTYPE);
  printf("BiddyReadVerilog %s using %s...\n",vhdlFile,Biddy_GetManagerName());

  Biddy_ReadVerilogFile(vhdlFile,NULL);
  Biddy_Purge(); /* delete all temporal functions created by Biddy_ReadVerilogFile */
  Biddy_SetAlphabeticOrdering();

  BiddyReport();
  /* Biddy_PrintInfo(NULL); */
  Biddy_Exit();
  
  free(prefixFile);
  free(benchFile);
  free(infixFile);
  free(vhdlFile);
}

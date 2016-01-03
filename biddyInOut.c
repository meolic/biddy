/***************************************************************************//*!
\file biddyInOut.c
\brief File biddyInOut.c contains various parsers and generators.

### Description

    PackageName [Biddy]
    Synopsis    [Biddy provides data structures and algorithms for the
                 representation and manipulation of Boolean functions with
                 ROBDDs. A hash table is used for quick search of nodes.
                 Complement edges decreases the number of nodes. An automatic
                 garbage collection with a formulae counter is implemented.
                 Variable swapping and sifting are implemented.]

    FileName    [biddyInOut.c]
    Revision    [$Revision: 124 $]
    Date        [$Date: 2015-12-30 17:27:05 +0100 (sre, 30 dec 2015) $]
    Authors     [Robert Meolic (robert.meolic@um.si),
                 Ales Casar (ales@homemade.net),
                 Volodymyr Mihav (mihaw.wolodymyr@gmail.com),
                 Jan Kraner (jankristian.kraner@student.um.si),
                 Ziga Kobale (ziga.kobale@student.um.si)]

### Copyright

Copyright (C) 2006, 2015 UM-FERI, Smetanova ulica 17, SI-2000 Maribor, Slovenia

Biddy is free software; you can redistribute it and/or modify it under the terms
of the GNU General Public License as published by the Free Software Foundation;
either version 2 of the License, or (at your option) any later version.

Biddy is distributed in the hope that it will be useful, but WITHOUT ANY
WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A
PARTICULAR PURPOSE. See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License along with
this program; if not, write to the Free Software Foundation, Inc., 51 Franklin
Street, Fifth Floor, Boston, MA 02110-1301 USA.

### More info

See also: biddy.h, biddyInt.h

*******************************************************************************/

#include "biddyInt.h"

/*
typedef struct {
  int n;
  void *p;
  int t;
} EnumeratedNode;
*/

typedef struct BoolTreeNodeType {
  long parent;
  long left;
  long right;
  int index;
  unsigned char op;
} BTreeNode;

typedef struct BoolTreeContainerType {
  BTreeNode *tnode;
  long availableNode;
} BTreeContainer;  

/*----------------------------------------------------------------------------*/
/* Static function prototypes                                                 */
/*----------------------------------------------------------------------------*/

/* Functions charOK() and nextCh() are used in Biddy_Eval0(), */
/* Biddy_Eval1(), and Biddy_Eval1x(). */

static Biddy_Boolean charOK(char c);

static void nextCh(Biddy_String s, int *i, Biddy_String *ch);

/*! Function ReadBDD()is used in Biddy_Eval0(). */

static Biddy_Edge ReadBDD(Biddy_Manager MNG, Biddy_String s, int *i, Biddy_String *ch);

/* Functions evaluate1(), evaluateN(), and Op() are used in */
/* Biddy_Eval1() and Biddy_Eval1x(). */

static Biddy_Edge evaluate1(Biddy_Manager MNG, Biddy_String s, int *i, Biddy_String *ch, Biddy_LookupFunction lf);

static Biddy_Edge evaluateN(Biddy_Manager MNG, Biddy_String s, int *i, Biddy_String *ch, Biddy_Edge g, int op, Biddy_LookupFunction lf);

static int Op(Biddy_String s, int *i, Biddy_String *ch);

/* Function createBddFromBTree() is used in Biddy_Eval2(). */

static Biddy_Edge createBddFromBTree(Biddy_Manager MNG, BTreeContainer *tree, long i);

/* Function WriteBDD() is used in Biddy_WriteBDD(). */

static void WriteBDD(Biddy_Manager MNG, Biddy_Edge f);

/* Function WriteBDDx() is used in Biddy_WriteBDDx(). */

static void WriteBDDx(Biddy_Manager MNG, FILE *funfile, Biddy_Edge f, unsigned int *line);

/* The following functions are used in Biddy_WriteDot(). */

static unsigned int enumerateNodes(Biddy_Edge f, unsigned int n);

static void WriteDotNodes(Biddy_Manager MNG, FILE *dotfile, Biddy_Edge f, int id);

static void WriteDotEdges(FILE *dotfile, Biddy_Edge f);

/* Other functions that may be used everywhere */

static Biddy_String getname(Biddy_Manager MNG, void *p);

static Biddy_String getshortname(Biddy_Manager MNG, void *p, int n);

/*----------------------------------------------------------------------------*/
/* Definition of exported functions                                           */
/*----------------------------------------------------------------------------*/

/***************************************************************************//*!
\brief Function Biddy_Managed_Eval0 evaluates raw format.

### Description
### Side effects
    Not reentrant.
### More info
    Macro Biddy_Eval0(s) is defined for use with anonymous manager.
*******************************************************************************/

#ifdef __cplusplus
extern "C" {
#endif

Biddy_String
Biddy_Managed_Eval0(Biddy_Manager MNG, Biddy_String s)
{
  int i;
  Biddy_String ch;
  Biddy_String name;
  Biddy_Edge f;

  if (!MNG) MNG = biddyAnonymousManager;

  ch = (Biddy_String) malloc(255); /* max variable name length */

  i = 0;
  nextCh(s,&i,&ch);
  name = strdup(ch);

  f = ReadBDD(MNG,s,&i,&ch);
  if (Biddy_IsNull(f)) {
    /* printf("(Biddy_Managed_Eval0) ERROR in file: char %d\n",i); */
    free(ch);
    return((Biddy_String)"");
  } else {
    Biddy_Managed_AddPersistentFormula(MNG,name,f);
  }

  nextCh(s,&i,&ch);
  if (strcmp(ch,"")) {
    /* printf("(Biddy_Managed_Eval0) ERROR in file: extra characters\n"); */
    free(ch);
    return((Biddy_String)"");
  }

  free(ch);
  return(name);
}

#ifdef __cplusplus
}
#endif

/***************************************************************************//*!
\brief Function Biddy_Managed_Eval1 evaluates prefix AND-OR-EXOR-NOT format.

### Description
### Side effects
    Not reentrant.
### More info
    Macro Biddy_Eval1(s) is defined for use with anonymous manager.
*******************************************************************************/
  
#ifdef __cplusplus
extern "C" {
#endif

Biddy_Edge
Biddy_Managed_Eval1(Biddy_Manager MNG, Biddy_String s)
{
  Biddy_Edge sup;
  Biddy_String ch;
  int i;

  if (!MNG) MNG = biddyAnonymousManager;

  ch = (Biddy_String) malloc(255);

  i = 0;
  nextCh(s,&i,&ch);
  sup = evaluate1(MNG,s,&i,&ch,NULL); /* user's formula table is not given */

  free(ch);
  return sup;
}

Biddy_Edge
Biddy_Managed_Eval1x(Biddy_Manager MNG, Biddy_String s, Biddy_LookupFunction lf)
{
  Biddy_Edge sup;
  Biddy_String ch;
  int i;

  if (!MNG) MNG = biddyAnonymousManager;

  ch = (Biddy_String) malloc(255);

  i = 0;
  nextCh(s,&i,&ch);
  sup = evaluate1(MNG,s,&i,&ch,lf);

  free(ch);
  return sup;
}

#ifdef __cplusplus
}
#endif

/***************************************************************************//*!
\brief Function Biddy_Managed_Eval2 evaluates infix &|^~>< format.

### Description
    Boolean constants are '0' and '1'.
    Parenthesis are implemented.
    Operators' priority is implemented.
    Formula Tree is suported (global table, only).
    Operators '*' and '+' are also allowed for conjunction/disjunction.
### Side effects
    Variable names must be one letter a-zA-Z optionally followed by int number.
### More info
    Author: Volodymyr Mihav (mihaw.wolodymyr@gmail.com)
    Original implementation of this function is on
    https://github.com/sungmaster/liBDD.
    Macro Biddy_Eval2(boolFunc) is defined for use with anonymous manager.
*******************************************************************************/

/* USE THIS FOR DEBUGGING */
/*
static void printBTreeContainer(BTreeContainer *tree);
static void printPrefixFromBTree(BTreeContainer *tree, long i);
*/

#ifdef __cplusplus
extern "C" {
#endif

Biddy_Edge
Biddy_Managed_Eval2(Biddy_Manager MNG, Biddy_String boolFunc)
{
  /* NOT (~), AND (&), OR (|), XOR (^) , BUTNOT (>), AND NOTBUT (<) ARE IMPLEMENTED */
  char boolOperators[] = { '~', '&', '>', '#', '<', '#', '^', '|', '#', '#', '#', '#', '#', '#', '#', '#' };
  long tempPos,currStringPos,currTreePos,oldTreePos; /* tmp variables */
  long offset; /* this is used for implementation of parenthesis and operator's priority */
  char currS;
  long i,j;
  BTreeContainer *tree;
  Biddy_Edge fbdd; /* result */

  if (!MNG) MNG = biddyAnonymousManager;
  
  i = 0; j = 0; 
  tree = (BTreeContainer *) calloc(1, sizeof(BTreeContainer));
  if (!tree) return biddyNull;
  tree->tnode = (BTreeNode*) malloc(1024 * sizeof(BTreeNode));
  if (!tree->tnode) return biddyNull;
  tree->availableNode = -1;
  for (i = 1023; i > 0; i--) {
    tree->tnode[i].parent = tree->availableNode;
    tree->availableNode = i;
  }
  tree->tnode[0].index = 0;

  tempPos = 0; currStringPos = 0; currTreePos = 1; oldTreePos = 0;
  offset = 0;
  currS = boolFunc[currStringPos];
  while (currS>'\n') {
    if (currS == ' ') {
      /* WHITE SPACE */
      currStringPos++; currS = boolFunc[currStringPos];
      continue;
    }
    currTreePos = tree->availableNode;
    if (currTreePos != -1) {
      tree->availableNode = tree->tnode[tree->availableNode].parent;
    } else {
      /* printf("ERROR: SOMETHING WRONG WITH tree->availableNode\n"); */
      break;
    }
    if (j == 0) {
      tree->tnode[oldTreePos].right = currTreePos;
      tree->tnode[currTreePos].parent = oldTreePos;
      tree->tnode[currTreePos].left = -1;
      if (currS == boolOperators[0]) {
        /* UNARY OPERATOR NOT */
        tree->tnode[currTreePos].op = 0;
        tree->tnode[currTreePos].index = offset + 3;
      }
      else if ((currS == '(') || (currS == '[') || (currS == '{')) {
        /* OPEN PARENTHESIS */
        offset += 4; currStringPos++;
        currS = boolFunc[currStringPos];
        continue;
      }
      else if (((currS >= '0') && (currS <= '1')) || ((currS >= 'A') && (currS <= 'Z')) || ((currS >= 'a') && (currS <= 'z'))) {
        /* CONSTANT OR VARIABLE NAME */
        tree->tnode[currTreePos].right = -1;
        tree->tnode[currTreePos].op = currS;
        currStringPos++;
        if ((boolFunc[currStringPos] >= '0') && (boolFunc[currStringPos] <= '9')) {
          tree->tnode[currTreePos].index = 0;
        } else {
          tree->tnode[currTreePos].index = -1; /* number is optional */
        }
        while ((boolFunc[currStringPos] >= '0') && (boolFunc[currStringPos] <= '9')) {
          tree->tnode[currTreePos].index = tree->tnode[currTreePos].index * 10 + (boolFunc[currStringPos] - '0');
          currStringPos++;
        }
        currStringPos--;
        j = 1;
      }
      else {
        /* printf("ERROR: MISSING OR INVALID VARIABLE NAME\n"); */
        break;
      }
    }
    else if (j == 1) {
      /* CLOSE PARENTHESIS */
      if ((currS == ')') || (currS == ']') || (currS == '}')) {
        offset -= 4; currStringPos++;
        currS = boolFunc[currStringPos];
        if (offset < 0) {
          /* printf("ERROR: INVALID CLOSE PARENTHESIS\n"); */
          j = 0;
          break;
        }
        continue;
      }
      else {
        /* BOOLEAN BINARY OPERATOR */
        j = 0;
        if (currS == '*') currS = '&'; /* operator '*' is also allowed for conjunction */
        if (currS == '+') currS = '|'; /* operator '+' is also allowed for disjunction */
        for (i = 0; i < 16; i++) { if (boolOperators[i] == currS) { break; } }
        if (i == 0) {
          /* printf("ERROR:INVALID USE OF UNARY OPERATOR ~\n"); */
          break; 
        }
        else if ((i < 16) && (boolOperators[i] != '#')) {
          tree->tnode[currTreePos].op = (char) i;
          if (i == 1) { tree->tnode[currTreePos].index = offset + 3; }
          else if ((i == 2) || (i == 4)) { tree->tnode[currTreePos].index = offset + 3; }
          else if ((i == 6) || (i == 7)) { tree->tnode[currTreePos].index = offset + 2; }
          else { tree->tnode[currTreePos].index = offset + 1; }
          oldTreePos = tree->tnode[oldTreePos].parent;
          while (tree->tnode[oldTreePos].index >= tree->tnode[currTreePos].index) {
            oldTreePos = tree->tnode[oldTreePos].parent;
          }
          tempPos = tree->tnode[oldTreePos].right;
          tree->tnode[oldTreePos].right = currTreePos;
          tree->tnode[tempPos].parent = currTreePos;
          tree->tnode[currTreePos].parent = oldTreePos;
          tree->tnode[currTreePos].left = tempPos;
        }
        else { 
          /* printf("ERROR: MISSING OPERATOR OR INVALID BOOLEAN OPERATOR %c\n",currS); */
          break; 
        }
      }
    }
    currStringPos++; currS = boolFunc[currStringPos];
    oldTreePos = currTreePos;
  }

  if (j == 0) {
    /* printf("ERROR: INCORRECT REQUEST\n"); */
  }

  /*
  printBTreeContainer(tree);
  */

  /*
  printf("%s\n",boolFunc);
  printPrefixFromBTree(tree,tree->tnode[0].right);
  printf("\n");
  */

  fbdd = createBddFromBTree(MNG,tree,tree->tnode[0].right);
  free(tree->tnode);
  free(tree);
  
  return fbdd;
}

#ifdef __cplusplus
}
#endif

/***************************************************************************//*!
\brief Function Biddy_Managed_WriteBDD writes raw format.

### Description
### Side effects
### More info
    Macro Biddy_WriteBDD(f) is defined for use with anonymous manager.
*******************************************************************************/

#ifdef __cplusplus
extern "C" {
#endif

void
Biddy_Managed_WriteBDD(Biddy_Manager MNG, Biddy_Edge f)
{
  if (!MNG) MNG = biddyAnonymousManager;

  if (Biddy_IsNull(f)) {
    printf("NULL\n");
  } else {
    WriteBDD(MNG,f);
    printf("\n");
  }
}

void
Biddy_Managed_WriteBDDx(Biddy_Manager MNG, const char filename[], Biddy_Edge f,
                        Biddy_String label)
{
  unsigned int line;
  FILE *s;

  if (!MNG) MNG = biddyAnonymousManager;

  s = fopen(filename,"w");
  if (!s) return;

  fprintf(s,"%s ",label);
        
  if (Biddy_IsNull(f)) {
    fprintf(s,"NULL\n");
  } else {
    line = 0;
    WriteBDDx(MNG,s,f,&line);
    if (line != 0) fprintf(s,"\n");
  }

  fclose(s);
}

#ifdef __cplusplus
}
#endif

/***************************************************************************//*!
\brief Function Biddy_Managed_WriteDot writes dot/graphviz format.

### Description
### Side effects
### More info
    Macro Biddy_WriteDot(filename,f,label) is defined for use with anonymous
    manager.
*******************************************************************************/

#ifdef __cplusplus
extern "C" {
#endif

unsigned int
Biddy_Managed_WriteDot(Biddy_Manager MNG, const char filename[], Biddy_Edge f,
                       const char label[])
{
  int id;
  unsigned int n;
  char *label1;
  char *hash;
  FILE *s;

  if (!MNG) MNG = biddyAnonymousManager;

  id = -1; /* if (id != -1) use it instead of <...> */

  n = 0;
  if (Biddy_IsNull(f)) {
    /* printf("ERROR Biddy_Managed_WriteDot: Function is null!\n"); */
    return 0 ;
  } else {

    s = fopen(filename,"w");
    if (!s) return 0;

    label1 = strdup(label);
    while ((hash=strchr(label1,'#'))) hash[0]='_'; /* avoid hashes in the name */

    fprintf(s,"//USE 'dot -y -Tpng -O %s' TO VISUALIZE BDD\n",filename);
    fprintf(s,"digraph BDD {\n");
    fprintf(s,"  ordering = out;\n");
    fprintf(s,"  splines = true;\n");
    fprintf(s,"  edge [dir=\"both\"];\n");
    fprintf(s,"  edge [arrowhead=\"none\"]\n");
    fprintf(s,"  node [shape = none, label = \"%s\"] 0;\n",label1);

    free(label1);

    BiddyCreateLocalInfo(MNG,f);
    n = enumerateNodes(f,0); /* this will select all nodes except constant node */
    Biddy_NodeRepair(f);
    WriteDotNodes(MNG,s,f,id);
    if (Biddy_GetMark(f)) {
      fprintf(s,"  0 -> 1 [arrowtail=\"none\" arrowhead=\"dot\"];\n");
    } else {
      fprintf(s,"  0 -> 1 [arrowtail=\"none\"];\n");
    }
    WriteDotEdges(s,f); /* this will select all nodes except constant node */
    BiddyDeleteLocalInfo(MNG, f);

    fprintf(s,"}\n");
    fclose(s);
  }

  return n;
}

unsigned int
Biddy_Managed_WriteDotx(Biddy_Manager MNG, const char filename[], Biddy_Edge f,
                        const char label[], int id)
{
  unsigned int n;
  char *label1;
  char *hash;
  FILE *s;

  if (!MNG) MNG = biddyAnonymousManager;

  n = 0;
  if (Biddy_IsNull(f)) {
    /* printf("ERROR Biddy_Managed_WriteDotx: Function is null!\n"); */
    return 0;
  } else {

    s = fopen(filename,"w");
    if (!s) return 0;

    label1 = strdup(label);
    while ((hash=strchr(label1,'#'))) hash[0]='_'; /* avoid hashes in the name */

    fprintf(s,"//USE 'dot -y -Tpng -O %s' TO VISUALIZE BDD\n",filename);
    fprintf(s,"digraph BDD {\n");
    fprintf(s,"  ordering = out;\n");
    fprintf(s,"  splines = true;\n");
    fprintf(s,"  edge [dir=\"both\"];\n");
    fprintf(s,"  edge [arrowhead=\"none\"]\n");
    fprintf(s,"  node [shape = none, label = \"%s\"] 0;\n",label1);

    free(label1);

    BiddyCreateLocalInfo(MNG,f);
    n = enumerateNodes(f,0); /* this will select all nodes except constant node */
    Biddy_NodeRepair(f);
    WriteDotNodes(MNG,s,f,id);
    if (Biddy_GetMark(f)) {
      fprintf(s,"  0 -> 1 [arrowtail=\"none\" arrowhead=\"dot\"];\n");
    } else {
      fprintf(s,"  0 -> 1 [arrowtail=\"none\"];\n");
    }
    WriteDotEdges(s,f); /* this will select all nodes except constant node */
    BiddyDeleteLocalInfo(MNG, f);

    fprintf(s,"}\n");
    fclose(s);
  }

  return n;
}

#ifdef __cplusplus
}
#endif

/***************************************************************************//*!
\brief Function Biddy_Managed_WriteTable writes truth table.

### Description
### Side effects
### More info
    Author: Jan Kraner (jankristian.kraner@student.um.si)
    Author: Ziga Kobale (ziga.kobale@student.um.si)
    Macro Biddy_WriteTable(f) is defined for use with anonymous manager.
*******************************************************************************/

#ifdef __cplusplus
extern "C" {
#endif

void
Biddy_Managed_WriteTable(Biddy_Manager MNG, Biddy_Edge f)
{
  Biddy_Edge support;
  unsigned int variableNumber;
  Biddy_Variable* variableTable;
  unsigned int i;
  int j;
  Biddy_Edge temp;
  unsigned int numcomb;

  if (!MNG) MNG = biddyAnonymousManager;

  /* support is a product of all variables in BDD */
  support = Biddy_Managed_Support(MNG,f);

  /* variableNumber is the number of variables in BDD */
  variableNumber = Biddy_Managed_VariableNumber(MNG,support);

  if (variableNumber > 8) {
    printf("Table for %d variables is to large for output.\n",variableNumber);
    return;
  }

  /* variableTable is a table of all variables in BDD */
  if (!(variableTable = (Biddy_Variable *)malloc((variableNumber) * sizeof(Biddy_Variable)))) return;

  for (i = 0; i < variableNumber; i++) {
#pragma warning(suppress: 6386)
    variableTable[i] = Biddy_GetTopVariable(support);
#pragma warning(suppress: 6385)
    printf("|%s",Biddy_Managed_GetVariableName(MNG,variableTable[i]));
    support = BiddyT(support);
  }
  printf("|:value\n");

  numcomb = 1;
  for (i = 0; i < variableNumber; i++) numcomb = 2 * numcomb;
  for (i = 0; i < numcomb; i++)
  {
    temp = f;
    for (j = variableNumber - 1; j >= 0; j--)
    {
      temp = Biddy_Managed_Restrict(MNG,temp,
                            variableTable[variableNumber-j-1],
                            (i&(1 << j)) ? biddyOne : biddyZero);
      if (i&(1 << j)) {
        printf ("%*c",1+(int)strlen(Biddy_Managed_GetVariableName(MNG,variableTable[variableNumber-j-1])),'1');
      } else {
        printf ("%*c",1+(int)strlen(Biddy_Managed_GetVariableName(MNG,variableTable[variableNumber-j-1])),'0');
      }
    }
    printf(" :");
    if (temp == biddyOne) printf("1\n"); else printf("0\n");
  }

  free(variableTable);
}

#ifdef __cplusplus
}
#endif
  
/*----------------------------------------------------------------------------*/
/* Definition of internal functions                                           */
/*----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------*/
/* Definition of static functions                                             */
/*----------------------------------------------------------------------------*/

/* Some of these were implemented long time ago (1995). */
/* Not comprehensible. */

#define oAND 1
#define oOR 2
#define oEXOR 3

static Biddy_Boolean
charOK(char c)
{
  return (c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z') || c == '_' ||
         (c >= '0' && c <= '9') || c == '[' || c == ']' ||
         c == '+' || c == '-' || c == '*' || c == '\'' ||
         c == '$' || c == '&' || c == '%' || c == '#' ||
         c == '?' || c == '!' || c == ':' || c == '.' ;
}

static void
nextCh(Biddy_String s, int *i, Biddy_String *ch)
{
  char c;
  int j;

  while (s[*i] == ' ' || s[*i] == '\t' || s[*i] == '\n') (*i)++;
  j = *i;
  c = s[*i];

  if (c == '(') {
    (*i)++;
    strcpy(*ch,"(");
    return;
  }

  if (c == ')') {
    (*i)++;
    strcpy(*ch,")");
    return;
  }

  if (c == '*') {
    (*i)++;
    strcpy(*ch,"*");
    return;
  }

  if (charOK(c)) {
    (*i)++;
    while (charOK(s[*i])) (*i)++;
    c = s[*i];

    if ( (c == ' ') || (c == '\t') || (c == '\n') || (c == '(') || (c == ')') ) {
      strncpy(*ch, &(s[j]), *i-j);
      (*ch)[*i-j] = 0;
      return;
    }
  }

  strcpy(*ch,"");
  return;
}

static Biddy_Edge
ReadBDD(Biddy_Manager MNG, Biddy_String s, int *i, Biddy_String *ch)
{
  Biddy_Boolean inv;
  Biddy_String varname;
  Biddy_Edge f,n,e,t;

  nextCh(s,i,ch);

  if (strcmp(*ch,"*")) {
    inv = FALSE;
  } else {
    inv = TRUE;
    nextCh(s,i,ch);
  }

  /* printf("<%s>",*ch); */

  if (strcmp(*ch,"1")) {
    varname = strdup(*ch);
    n = Biddy_Managed_FoaVariable(MNG,varname);
    nextCh(s,i,ch);
    if (strcmp(*ch,"(")) {
      /* printf("ERROR: <%s>\n",*ch); */
      return biddyNull;
    }
    e = ReadBDD(MNG,s,i,ch);
    nextCh(s,i,ch);
    if (strcmp(*ch,")")) {
      /* printf("ERROR: <%s>\n",*ch); */
      return biddyNull;
    }
    nextCh(s,i,ch);
    if (strcmp(*ch,"(")) {
      /* printf("ERROR: <%s>\n",*ch); */
      return biddyNull;
    }
    t = ReadBDD(MNG,s,i,ch);
    nextCh(s,i,ch);
    if (strcmp(*ch,")")) {
      /* printf("ERROR: <%s>\n",*ch); */
      return biddyNull;
    }
    f = Biddy_NotCond(Biddy_Managed_ITE(MNG,n,t,e),inv);
    free(varname);
  } else {
    f = Biddy_NotCond(biddyOne,inv);
  }

  return f;
}

static Biddy_Edge
evaluate1(Biddy_Manager MNG, Biddy_String s, int *i, Biddy_String *ch,
          Biddy_LookupFunction lf)
{
  Biddy_Edge f;
  int op;

  if (!strcmp(*ch,"(")) {
    nextCh(s,i,ch);

    if (!strcasecmp(*ch, "NOT")) {
      nextCh(s,i,ch);
      f = Biddy_Not(evaluate1(MNG,s,i,ch,lf));
    } else {
      if ((op = Op(s,i,ch))) {
        f = evaluateN(MNG,s,i,ch,evaluate1(MNG,s,i,ch,lf),op,lf);
      } else {
        f = evaluate1(MNG,s,i,ch,lf);
      }
    }

    if (!strcmp(*ch, ")")) nextCh(s,i,ch);

    return f;
  }

  /* evaluate1 will always check Biddy's formula table */
  /* if (lf != NULL) then user's formula table will be checked, too */
  /* user's formula table will be checked before Biddy's formula table */
  /* it is wrong if entries in user's formula are bad */

  if (charOK(*ch[0])) {
    if (!lf) {
      if (!Biddy_Managed_FindFormula(MNG,*ch,&f)) {
        f = Biddy_Managed_FoaVariable(MNG,*ch);
      }
    } else {
      if ((!(lf(*ch,&f))) &&
          (!Biddy_Managed_FindFormula(MNG,*ch,&f))) {
        f = Biddy_Managed_FoaVariable(MNG,*ch);
      }
    }
    nextCh(s,i,ch);
    return f;
  }

  return biddyNull;
}

static Biddy_Edge
evaluateN(Biddy_Manager MNG, Biddy_String s, int *i, Biddy_String *ch,
          Biddy_Edge g, int op, Biddy_LookupFunction lf)
{
  Biddy_Edge f, h;

  h = evaluate1(MNG,s,i,ch,lf);
  if (!Biddy_IsNull(h)) {
    f = biddyNull;
    switch (op) {
            case oAND:  f = Biddy_Managed_ITE(MNG,g,h,biddyZero);
                        break;
            case oOR:  f = Biddy_Managed_ITE(MNG,g,biddyOne,h);
                        break;
            case oEXOR:  f = Biddy_Managed_ITE(MNG,g,Biddy_Not(h),h);
    }
    return evaluateN(MNG,s,i,ch,f,op,lf);
  }
  return g;
}

static int
Op(Biddy_String s, int *i, Biddy_String *ch)
{
  if (!strcasecmp(*ch, "AND")) {
    nextCh(s,i,ch);
    return oAND;
  }

  if (!strcasecmp(*ch, "OR")) {
    nextCh(s,i,ch);
    return oOR;
  }

  if (!strcasecmp(*ch, "EXOR")) {
    nextCh(s,i,ch);
    return oEXOR;
  }

  return 0;
}

/* DEBUGGING, ONLY */
/*
static void
printBTreeContainer(BTreeContainer *tree)
{
  long i;
  printf("availableNode=%ld\n",tree->availableNode);
    for (i = tree->availableNode; i >= 0; i--) {
    printf("[%ld]parent=%ld,left=%ld,right=%ld,index=%d,op=%u\n",i,
           tree->tnode[i].parent,
           tree->tnode[i].left,
           tree->tnode[i].right,
           tree->tnode[i].index,
           tree->tnode[i].op);
  }
}
*/

/* DEBUGGING, ONLY */
/*
static void
printPrefixFromBTree(BTreeContainer *tree, long i)
{
  if (i == -1) return;
  if (tree->tnode[i].op == 48) {
    printf("0");
    return;
  }
  if (tree->tnode[i].op == 49) {
    printf("1");
    return;
  }
  if (tree->tnode[i].op >= 64) {
    if (tree->tnode[i].index == -1) {
      printf("%c",tree->tnode[i].op);
    } else {
      printf("%c%d",tree->tnode[i].op,tree->tnode[i].index);
    }
    return;
  }
  if (tree->tnode[i].op == 0) printf("(NOT");
  else if (tree->tnode[i].op == 1) printf("(AND ");
  else if (tree->tnode[i].op == 2) printf("(BUTNOT ");
  else if (tree->tnode[i].op == 4) printf("(NOTBUT ");
  else if (tree->tnode[i].op == 6) printf("(XOR ");
  else if (tree->tnode[i].op == 7) printf("(OR ");
  else printf("(? ");
  printPrefixFromBTree(tree,tree->tnode[i].left);
  printf(" ");
  printPrefixFromBTree(tree,tree->tnode[i].right);
  printf(")");
}
*/

static Biddy_Edge
createBddFromBTree(Biddy_Manager MNG, BTreeContainer *tree, long i)
{
  Biddy_Edge lbdd,rbdd,fbdd;
  if (i == -1) return biddyNull;
  if (tree->tnode[i].op == 48) return biddyZero;
  if (tree->tnode[i].op == 49) return biddyOne;
  if (tree->tnode[i].op >= 64) {
    Biddy_String varname;
    varname = strdup("x000000000"); /* max is 9-digit number */
    if (tree->tnode[i].index == -1) {
      sprintf(varname,"%c",tree->tnode[i].op);
    } else {
      sprintf(varname,"%c%d",tree->tnode[i].op,tree->tnode[i].index);
    }
    if (!(Biddy_Managed_FindFormula(MNG,varname,&fbdd))) {
      fbdd = Biddy_Managed_FoaVariable(MNG,varname);
    }
    free(varname);
    return fbdd;
  }
  lbdd = createBddFromBTree(MNG,tree,tree->tnode[i].left);
  rbdd = createBddFromBTree(MNG,tree,tree->tnode[i].right);
  if (tree->tnode[i].op == 0) fbdd = Biddy_Not(rbdd); /* NOT */
  else if (tree->tnode[i].op == 1) fbdd = Biddy_Managed_ITE(MNG,lbdd,rbdd,biddyZero); /* AND */
  else if (tree->tnode[i].op == 2) fbdd = Biddy_Managed_ITE(MNG,rbdd,biddyZero,lbdd); /* BUTNOT */
  else if (tree->tnode[i].op == 4) fbdd = Biddy_Managed_ITE(MNG,lbdd,biddyZero,rbdd); /* NOTBUT */
  else if (tree->tnode[i].op == 6) fbdd = Biddy_Managed_ITE(MNG,lbdd,Biddy_Not(rbdd),rbdd); /* EXOR */
  else if (tree->tnode[i].op == 7) fbdd = Biddy_Managed_ITE(MNG,lbdd,biddyOne,rbdd); /* OR */
  else fbdd = biddyNull;
  return fbdd;
}

static void
WriteBDD(Biddy_Manager MNG, Biddy_Edge f)
{
  Biddy_String var;

  if (Biddy_GetMark(f)) {
    printf("*");
  }

  var = Biddy_Managed_GetTopVariableName(MNG,f);
  printf("%s",var);

  if (!Biddy_IsConstant(f)) {
    printf("(");
    WriteBDD(MNG,BiddyE(f));
    printf(")(");
    WriteBDD(MNG,BiddyT(f));
    printf(")");
  }
}

static void
WriteBDDx(Biddy_Manager MNG, FILE *funfile, Biddy_Edge f, unsigned int *line)
{
  Biddy_String var;

  if (Biddy_GetMark(f)) {
    fprintf(funfile,"* ");
  }

  *line = *line + 2;

  var = Biddy_Managed_GetTopVariableName(MNG,f);
  fprintf(funfile,"%s",var);

  *line = *line + (unsigned int)strlen(var);

  if (!Biddy_IsConstant(f)) {
    if (*line >= 80) {
      fprintf(funfile,"\n");
      *line = 0;
    }
    fprintf(funfile," (");
    *line = *line + 2;
    WriteBDDx(MNG,funfile,BiddyE(f),line);
    if (*line >= 80) {
      fprintf(funfile,"\n");
      *line = 0;
    }
    fprintf(funfile,") (");
    *line = *line + 3;
    WriteBDDx(MNG,funfile,BiddyT(f),line);
    if (*line >= 80) {
      fprintf(funfile,"\n");
      *line = 0;
    }
    fprintf(funfile,")");
    *line = *line + 1;
  }
}

static unsigned int
enumerateNodes(Biddy_Edge f, unsigned int n)
{
  if (!Biddy_IsSelected(f)) {
    if (Biddy_IsConstant(f)) {
    } else {
      Biddy_SelectNode(f);
      n++;
      BiddySetEnumerator(f,n);
      if (Biddy_IsConstant(BiddyE(f)) || Biddy_IsConstant(BiddyT(f))) {
        n++; /* every instance of constant node is enumerated */
      }
      if (!Biddy_IsConstant(BiddyE(f))) {
        n = enumerateNodes(BiddyE(f),n);
      }
      if (!Biddy_IsConstant(BiddyT(f))) {
        n = enumerateNodes(BiddyT(f),n);
      }
    }
  }
  return n;
}

static void
WriteDotNodes(Biddy_Manager MNG, FILE *dotfile, Biddy_Edge f, int id)
{
  BiddyLocalInfo *li;
  Biddy_String name,hash;

  if (Biddy_IsConstant(f)) {
    fprintf(dotfile, "  node [shape = none, label = \"1\"] %u;\n",1);
    return;
  }
  li = (BiddyLocalInfo *)(((BiddyNode *)Biddy_Regular(f))->list);
  while (li->back) {
    if (id == -1) {
      name = getname(MNG,li->back);
    } else {
      name = getshortname(MNG,li->back,id);
    }
    while ((hash = strchr(name, '#'))) hash[0] = '_';
    fprintf(dotfile,"  node [shape = circle, label = \"%s\"] %u;\n",name,li->data.enumerator);
    free(name);
    if (Biddy_IsConstant(BiddyE(li->back)) || Biddy_IsConstant(BiddyT(li->back))) {
      fprintf(dotfile,"  node [shape = none, label = \"1\"] %u;\n",li->data.enumerator+1);
    }
    li = &li[1]; /* next field in the array */
  }
}

static void
WriteDotEdges(FILE *dotfile, Biddy_Edge f)
{
  unsigned int n1,n2;

  if (!Biddy_IsConstant(f) && !Biddy_IsSelected(f)) {
    Biddy_SelectNode(f);
    n1 = BiddyGetEnumerator(f);

    if (Biddy_IsConstant(BiddyE(f))) {
      n2 = n1 + 1;
    } else {
      n2 = BiddyGetEnumerator(BiddyE(f));
    }
    if (Biddy_GetMark(BiddyE(f))) {
      fprintf(dotfile,"  %u -> %u [arrowtail=\"odot\" arrowhead=\"dot\"];\n",n1,n2);
    } else {
      fprintf(dotfile,"  %u -> %u [arrowtail=\"odot\"];\n",n1,n2);
    }

    if (Biddy_IsConstant(BiddyT(f))) {
      n2 = n1 + 1;
    } else {
      n2 = BiddyGetEnumerator(BiddyT(f));
    }
    if (Biddy_GetMark(BiddyT(f))) {
      fprintf(dotfile,"  %d -> %d [arrowtail=\"invempty\" arrowhead=\"dot\"];\n",n1,n2);
    } else {
      fprintf(dotfile,"  %d -> %d [arrowtail=\"invempty\"];\n",n1,n2);
    }

    WriteDotEdges(dotfile,BiddyE(f));
    WriteDotEdges(dotfile,BiddyT(f));
  }
}

static Biddy_String
getname(Biddy_Manager MNG, void *p) {
  unsigned int i;
  Biddy_String newname;

  newname = strdup(Biddy_Managed_GetTopVariableName(MNG,(Biddy_Edge) p));
  for (i=0; i<strlen(newname); ++i) {
    if (newname[i] == '<') newname[i] = '_';
    if (newname[i] == '>') newname[i] = 0;
  }

  return (newname);
}

static Biddy_String
getshortname(Biddy_Manager MNG, void *p, int n) {
  unsigned int i;
  Biddy_String name;
  Biddy_String shortname;

  name = strdup(Biddy_Managed_GetTopVariableName(MNG,(Biddy_Edge) p));
  i = (unsigned int)strcspn(name,"<");
  name[i]=0;
  shortname = strdup(name);
  free(name);

  return (shortname);
}

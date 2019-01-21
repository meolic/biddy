/***************************************************************************//*!
\file biddyInOut.c
\brief File biddyInOut.c contains various parsers and generators.

### Description

    PackageName [Biddy]
    Synopsis    [Biddy provides data structures and algorithms for the
                 representation and manipulation of Boolean functions with
                 ROBDDs, 0-sup-BDDs, and TZBDDs. A hash table is used for quick
                 search of nodes. Complement edges decreases the number of
                 nodes. An automatic garbage collection with a system age is
                 implemented. Variable swapping and sifting are implemented.]

    FileName    [biddyInOut.c]
    Revision    [$Revision: 541 $]
    Date        [$Date: 2019-01-21 21:25:47 +0100 (pon, 21 jan 2019) $]
    Authors     [Robert Meolic (robert.meolic@um.si),
                 Ales Casar (ales@homemade.net),
                 Volodymyr Mihav (mihaw.wolodymyr@gmail.com),
                 David Kebo Houngninou (dhoungninou@smu.edu)]

### Copyright

Copyright (C) 2006, 2018 UM FERI, Koroska cesta 46, SI-2000 Maribor, Slovenia

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

/* the following constants and types are used in Biddy_ReadVerilogFile() */
#define LINESIZE 999 /* maximum length of each input line read */
#define BUFSIZE 99999 /* maximum length of a buffer */
#define INOUTNUM 999 /* maximum number of inputs and outputs */
#define REGNUM 9999 /* maximum number of registers */
#define TOKENNUM 9999 /* maximum number of tokens */
#define GATENUM 9999 /* maximum number of gates */
#define LINENUM 99999 /* maximum number of lines */
#define WIRENUM 99999 /* maximum number of wires */

/* recognized Verilog Boolean operators */
/* some of them are not very standard but they are used in various benchmarks */
/* we are trying to accept everything which is not in direct conflict with the standard */
/* https://www.xilinx.com/support/documentation/sw_manuals/xilinx11/ite_r_verilog_reserved_words.htm */
/* http://sutherland-hdl.com/pdfs/verilog_2001_ref_guide.pdf, page 3 */
/* http://www.externsoft.ch/download/verilog.html */
#define GATESNUM 9
const char* VerilogFileGateName[] = {
  "buf",
  "and",
  "nand",
  "or",
  "nor",
  "xor",
  "xnor",
  "not", "inv"
};

/*----------------------------------------------------------------------------*/
/* Static function prototypes                                                 */
/*----------------------------------------------------------------------------*/

/* The following functions are used in Biddy_Eval0(), Biddy_Eval1(), and Biddy_Eval1x(). */

static Biddy_Boolean charOK(char c);
static void nextCh(Biddy_String s, int *i, Biddy_String *ch);

/* Function ReadBDD()is used in Biddy_Eval0(). */

static Biddy_Edge ReadBDD(Biddy_Manager MNG, Biddy_String s, int *i, Biddy_String *ch);

/* The following functions are used in Biddy_Eval1() and Biddy_Eval1x(). */

static Biddy_Edge evaluate1(Biddy_Manager MNG, Biddy_String s, int *i, Biddy_String *ch, Biddy_LookupFunction lf);
static Biddy_Edge evaluateN(Biddy_Manager MNG, Biddy_String s, int *i, Biddy_String *ch, Biddy_Edge g, int op, Biddy_LookupFunction lf);
static int Op(Biddy_String s, int *i, Biddy_String *ch);

/* Functions createVariablesFromBTree and createBddFromBTree() are used in Biddy_Eval2(). */

static void createVariablesFromBTree(Biddy_Manager MNG, BiddyBTreeContainer *tree);
static Biddy_Edge createBddFromBTree(Biddy_Manager MNG, BiddyBTreeContainer *tree, int i);

/* The following functions are used in Biddy_ReadVerilogFile(). */
static void parseVerilogFile(FILE *verilogfile, unsigned int *l, BiddyVerilogLine ***lt, unsigned int *n, BiddyVerilogModule ***mt);
static void createVerilogCircuit(unsigned int linecount, BiddyVerilogLine **lt, unsigned int modulecount, BiddyVerilogModule **mt, BiddyVerilogCircuit *c);
static void createBddFromVerilogCircuit(Biddy_Manager MNG, BiddyVerilogCircuit *c, Biddy_String prefix);
static void parseSignalVector(Biddy_String signal_arr[], Biddy_String token[], unsigned int *index, unsigned int *count);
static void printModuleSummary(BiddyVerilogModule *m);
static void printCircuitSummary(BiddyVerilogCircuit *c);
static Biddy_Boolean isGate(Biddy_String word);
static Biddy_Boolean isSignalVector(Biddy_String word);
static Biddy_Boolean isEndOfLine(Biddy_String source);
static Biddy_Boolean isDefined(BiddyVerilogCircuit *c, Biddy_String name);
static void buildNode(BiddyVerilogNode *n, Biddy_String type, Biddy_String name);
static void buildWire(BiddyVerilogCircuit *c, BiddyVerilogWire *w, Biddy_String type, Biddy_String name);
static int getNodeIdByName(BiddyVerilogCircuit *c, Biddy_String name);
static BiddyVerilogWire *getWireById(BiddyVerilogCircuit *c, int id);
static BiddyVerilogWire *getWireByName(BiddyVerilogCircuit *c, Biddy_String name);
static Biddy_String trim(Biddy_String source);
static void concat(char **s1, const char *s2);

/* Function WriteBDD() is used in Biddy_WriteBDD(). */

static void WriteBDD(Biddy_Manager MNG, Biddy_Edge f);

/* Function WriteBDDx() is used in Biddy_WriteBDDx(). */

static void WriteBDDx(Biddy_Manager MNG, FILE *funfile, Biddy_Edge f, unsigned int *line);

/* The following functions are used in Biddy_PrintfSOP() in Biddy_WriteSOP(). */

static Biddy_Boolean WriteProduct(Biddy_Manager MNG, BiddyVarList *l, Biddy_Boolean combinationset, Biddy_Boolean first);
static Biddy_Boolean WriteProductx(Biddy_Manager MNG, FILE *s, BiddyVarList *l, Biddy_Boolean combinationset, Biddy_Boolean first);
static void WriteProductAlphabetic(Biddy_Manager MNG, BiddyVarList *l, Biddy_Boolean combinationset, Biddy_Boolean first);
static void WriteSOP(Biddy_Manager MNG, Biddy_Edge f, Biddy_Variable top, Biddy_Boolean mark, BiddyVarList *l, unsigned int *maxsize, Biddy_Boolean combinationset);
static void WriteSOPx(Biddy_Manager MNG, FILE *s, Biddy_Edge f, Biddy_Variable top, Biddy_Boolean mark, BiddyVarList *l, unsigned int *maxsize, Biddy_Boolean combinationset);

/* The following functions are used in Biddy_WriteDot(). */

static unsigned int enumerateNodes(Biddy_Manager MNG, Biddy_Edge f, unsigned int n);
static void WriteDotNodes(Biddy_Manager MNG, FILE *dotfile, Biddy_Edge f, int id, Biddy_Boolean cudd);
static void WriteDotEdges(Biddy_Manager MNG, FILE *dotfile, Biddy_Edge f, Biddy_Boolean cudd);

/* The following functions are used in Biddy_WriteBddview(). */

static void WriteBddviewConnections(Biddy_Manager MNG, FILE *funfile, Biddy_Edge f);

/* Other functions that may be used everywhere */

static Biddy_String getname(Biddy_Manager MNG, void *p);
static Biddy_String getshortname(Biddy_Manager MNG, void *p, int n);

/*----------------------------------------------------------------------------*/
/* Definition of exported functions                                           */
/*----------------------------------------------------------------------------*/

/***************************************************************************//*!
\brief Function Biddy_Managed_Eval0 evaluates raw format.

### Description
    First word is a name. It is followed by raw format.
    Function return name of the formula.
### Side effects
    All variables should already exists in the correct ordering!
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

  if ((biddyManagerType == BIDDYTYPEOBDD) || (biddyManagerType == BIDDYTYPEOBDDC)) {
    /* IMPLEMENTED */
  } else if ((biddyManagerType == BIDDYTYPEZBDD) || (biddyManagerType == BIDDYTYPEZBDDC) ||
              (biddyManagerType == BIDDYTYPETZBDD) || (biddyManagerType == BIDDYTYPETZBDDC))
  {
    fprintf(stderr,"Biddy_Eval0: this BDD type is not supported, yet!\n");
    return NULL;
  } else if ((biddyManagerType == BIDDYTYPEOFDD) || (biddyManagerType == BIDDYTYPEOFDDC) ||
              (biddyManagerType == BIDDYTYPEZFDD) || (biddyManagerType == BIDDYTYPEZFDDC) ||
              (biddyManagerType == BIDDYTYPETZFDD) || (biddyManagerType == BIDDYTYPETZFDDC))
  {
    fprintf(stderr,"Biddy_Eval0: this BDD type is not supported, yet!\n");
    return NULL;
  } else {
    fprintf(stderr,"Biddy_Eval0: Unsupported BDD type!\n");
    return NULL;
  }

  ch = (Biddy_String) malloc(255); /* max variable name length */

  i = 0;
  nextCh(s,&i,&ch);
  name = strdup(ch);

  f = ReadBDD(MNG,s,&i,&ch);

  if (Biddy_IsNull(f)) {
    printf("(Biddy_Managed_Eval0) ERROR: char %d\n",i);
    free(ch);
    return((Biddy_String)"");
  } else {
    Biddy_Managed_AddPersistentFormula(MNG,name,f);
  }

  nextCh(s,&i,&ch);
  if (strcmp(ch,"")) {
    printf("(Biddy_Managed_Eval0) ERROR: extra characters\n");
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
\brief Function Biddy_Managed_Eval1x evaluates prefix AND-OR-EXOR-NOT format.

### Description
    Parameter lf is a lookup function in the user-defined cache table.
### Side effects
    Not reentrant.
### More info
    Macro Biddy_Eval1x(s,lf) is defined for use with anonymous manager.
    Macros Biddy_Managed_Eval1(s) and Biddy_Eval1(s) are defined for use
    without searching in the user-defined cache.
*******************************************************************************/

#ifdef __cplusplus
extern "C" {
#endif

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

  if (Biddy_IsNull(sup)) {
    printf("(Biddy_Managed_Eval1x) ERROR: return NULL\n");
  }

  return sup;
}

#ifdef __cplusplus
}
#endif

/***************************************************************************//*!
\brief Function Biddy_Managed_Eval2 evaluates infix format.

### Description
    Parenthesis are implemented.
    Operators' priority is implemented.
    Formula Tree is suported (global table, only).
    Boolean constants are '0' and '1'.
    Boolean operators are NOT (~!), AND (&*), OR (|+), XOR (^%), XNOR (-),
    IMPLIES (><), NAND (@), NOR (#), BUTNOT (\), NOTBUT (/).
### Side effects
### More info
    Original author: Volodymyr Mihav (mihaw.wolodymyr@gmail.com)
    Original implementation of this function is on
    https://github.com/sungmaster/liBDD.
    Macro Biddy_Eval2(boolFunc) is defined for use with anonymous manager.
*******************************************************************************/

/* USE THIS FOR DEBUGGING */
/*
static void
printBiddyBTreeContainer(BiddyBTreeContainer *tree)
{
  int i;
  printf("availableNode=%d\n",tree->availableNode);
    for (i = tree->availableNode; i >= 0; i--) {
    printf("[%d]parent=%d,left=%d,right=%d,index=%d,op=%u",i,
           tree->tnode[i].parent,
           tree->tnode[i].left,
           tree->tnode[i].right,
           tree->tnode[i].index,
           tree->tnode[i].op
    );
    if (tree->tnode[i].name) {
      printf(",name=%s\n",tree->tnode[i].name);
    } else {
      printf("\n");
    }
  }
}

static void
printPrefixFromBTree(BiddyBTreeContainer *tree, int i)
{
  if (i == -1) return;
  if (tree->tnode[i].op == 255) {
    if (tree->tnode[i].name) {
      printf("%s",tree->tnode[i].name);
    } else {
      printf("NULL");
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

#ifdef __cplusplus
extern "C" {
#endif

Biddy_Edge
Biddy_Managed_Eval2(Biddy_Manager MNG, Biddy_String boolFunc)
{
  /* NOT (~!), AND (&*), OR (|+), XOR (^%) , XNOR(-), IMPLIES (><), NAND (@), NOR (#), BUTNOT (\), AND NOTBUT (/) ARE IMPLEMENTED */
  char boolOperators[] = { '~', '&', '\\', ' ', '/', ' ', '^', '|', '#', '-', ' ', '<', ' ', '>', '@', ' ' };
  long tempPos,currStringPos,currTreePos,oldTreePos; /* tmp variables */
  long offset; /* this is used for implementation of parenthesis and operator's priority */
  char currS;
  long i,j;
  BiddyBTreeContainer *tree;
  Biddy_Edge fbdd; /* result */
  Biddy_String expression;

  if (!MNG) MNG = biddyAnonymousManager;

  i = 0; j = 0;
  tree = (BiddyBTreeContainer *) calloc(1, sizeof(BiddyBTreeContainer));
  if (!tree) return biddyNull;
  tree->tnode = (BiddyBTreeNode*) malloc(1024 * sizeof(BiddyBTreeNode));
  if (!tree->tnode) return biddyNull;
  tree->availableNode = -1;
  for (i = 1023; i > 0; i--) {
    tree->tnode[i].name = NULL;
    tree->tnode[i].parent = tree->availableNode;
    tree->availableNode = i;
  }
  tree->tnode[0].index = 0;
  tree->tnode[0].name = NULL;

  tempPos = 0;
  currStringPos = 0;
  currTreePos = 1;
  oldTreePos = 0;
  offset = 0;
  expression = strdup(boolFunc);
  currS = expression[currStringPos];
  while (currS > '\n') {
    if (currS == ' ') {
      /* WHITE SPACE */
      currStringPos++;
      currS = expression[currStringPos];
      continue;
    }
    currTreePos = tree->availableNode;
    if (currTreePos != -1) {
      tree->availableNode = tree->tnode[tree->availableNode].parent;
    } else {
      fprintf(stderr,"ERROR: SOMETHING WRONG WITH tree->availableNode\n");
      return biddyNull;
    }
    if (j == 0) {
      tree->tnode[oldTreePos].right = currTreePos;
      tree->tnode[currTreePos].parent = oldTreePos;
      tree->tnode[currTreePos].left = -1;
      if (currS == '!') currS = '~'; /* operator '!' is also allowed for negation */
      if (currS == boolOperators[0]) {
        /* UNARY OPERATOR NOT */
        tree->tnode[currTreePos].op = 0;
        tree->tnode[currTreePos].index = offset + 3;
      }
      else if ((currS == '(') || (currS == '[') || (currS == '{')) {
        /* OPEN PARENTHESIS */
        offset += 4; currStringPos++;
        currS = expression[currStringPos];
        continue;
      }
      else if ((currS == '_') || ((currS >= '0') && (currS <= '9')) || ((currS >= 'A') && (currS <= 'Z')) || ((currS >= 'a') && (currS <= 'z'))) {
        /* CONSTANT OR VARIABLE NAME */
        tree->tnode[currTreePos].right = -1;
        tree->tnode[currTreePos].op = 255;
        tree->tnode[currTreePos].index = currStringPos;
        while ((currS == '_') || ((currS >= '0') && (currS <= '9')) || ((currS >= 'A') && (currS <= 'Z')) || ((currS >= 'a') && (currS <= 'z'))) {
          currStringPos++;
          currS = expression[currStringPos];
        }
        expression[currStringPos] = 0;
        tree->tnode[currTreePos].name = strdup(&expression[tree->tnode[currTreePos].index]);
        tree->tnode[currTreePos].index = (int) strlen(tree->tnode[currTreePos].name); /* length of the variable name */
        expression[currStringPos] = currS;
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
        currS = expression[currStringPos];
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
        if (currS == '%') currS = '^'; /* operator '%' is also allowed for xor */
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
    currStringPos++;
    currS = expression[currStringPos];
    oldTreePos = currTreePos;
  }

  if (j == 0) {
    /* printf("ERROR: INCORRECT REQUEST\n"); */
    return biddyNull;
  }

  /*
  printBiddyBTreeContainer(tree);
  */

  /*
  printf("%s\n",expression);
  printPrefixFromBTree(tree,tree->tnode[0].right);
  printf("\n");
  */

  free(expression);

  /* FOR SOME BDD TYPES, IT IS NECESSARY TO CREATE ALL VARIABLES IN ADVANCE */
  /* createBddFromBTree DOES NOT STORE TMP RESULTS AS FORMULAE AND THUS */
  /* TMP RESULTS BECOME WRONG IF NEW VARIABLES ARE ADDED ON-THE-FLY */

  createVariablesFromBTree(MNG,tree);
  fbdd = createBddFromBTree(MNG,tree,tree->tnode[0].right);

  for (i = 1023; i >= 0; i--) {
    if (tree->tnode[i].name) free(tree->tnode[i].name);
  }
  free(tree->tnode);
  free(tree);

  return fbdd;
}

#ifdef __cplusplus
}
#endif

/***************************************************************************//*!
\brief Function Biddy_Managed_ReadBddview reads bddview file and creates
        a Boolean function.

### Description
    If (name != NULL) then name will be used for the resulting BDD.
    If (name == NULL) then the resulting BDD will have name given in the file.
    Resulting BDD will not be preserved.
### Side effects
    Biddy_Managed_ConstructBDD is used and thus, if variable ordering in the
    file is not compatible with the active ordering then the result will be
    wrong!
    To improve efficiency, only the first "type", "var", and "label" are used.
### More info
    Macro Biddy_ReadBddview(filename) is defined for use with anonymous
    manager.
*******************************************************************************/

#ifdef __cplusplus
extern "C" {
#endif

Biddy_String
Biddy_Managed_ReadBddview(Biddy_Manager MNG, const char filename[],
                          Biddy_String name)
{
  FILE *bddfile;
  char buffer[65536]; /* line with variables can be very long! */
  char buffer2[256];
  char code[4];
  Biddy_String word,word2,line;
  BiddyVariableOrder *tableV;
  BiddyNodeList *tableN;
  int numV,numN;
  int i,n,m; /* this must be signed because of the used for loop */
  Biddy_Boolean typeOK,varOK,labelOK;
  Biddy_Edge r;

  if (!MNG) MNG = biddyAnonymousManager;

  bddfile = fopen(filename,"r");
  if (!bddfile) {
    printf("Biddy_ReadBddview: File error (%s)!\n",filename);
    return NULL;
  }

  numV = 0;
  numN = 0;
  tableV = NULL;
  tableN = NULL;

  typeOK = FALSE;
  varOK = FALSE;
  labelOK = FALSE;

  /* PARSE FILE */

  line = fgets(buffer,65535,bddfile);
  while (line) {

    line = trim(line);

    /* DEBUGGING */
    /*
    printf("<%s>\n",line);
    */

    if (line[0] == '#') { /* skip the comment line */
      line = fgets(buffer,65535,bddfile);
      continue;
    }
    else if ((line[0] == 'c') && !strncmp(line,"connect",7)) { /* parsing line with a connection */
      line = &line[8];
      word = word2 = NULL;
      if ((biddyManagerType == BIDDYTYPETZBDDC) || (biddyManagerType == BIDDYTYPETZBDD)) {
        buffer[0] = buffer2[0] = 0;
        sscanf(line,"%u %u %s %s",&n,&m,code,buffer);
        if ((code[0] == 'e') || (code[0] == 'd'))
        {
          sscanf(line,"%u %u %s %s %s",&n,&m,code,buffer,buffer2);
        }
        if (buffer[0] == '"') {
          buffer[strlen(buffer)-1] = 0;
          word = &buffer[1];
        } else {
          if (buffer[0]) word = &buffer[0];
        }
        if (buffer2[0] == '"') {
          buffer2[strlen(buffer2)-1] = 0;
          word2= &buffer2[1];
        } else {
          if (buffer2[0]) word2 = &buffer2[0];
        }
      } else {
        sscanf(line,"%u %u %s",&n,&m,code);
      }
      if (word2) {
        tableN[n].ltag = Biddy_Managed_GetVariable(MNG,word);
        tableN[n].rtag = Biddy_Managed_GetVariable(MNG,word2);
      } else if (word) {
        /* if (!strcmp(code,"l") || !strcmp(code,"s") || !strcmp(code,"si")) */
        if ((code[0] == 'l') || (code[0] == 's'))
        {
          tableN[n].ltag = Biddy_Managed_GetVariable(MNG,word);
        }
        /* else if (!strcmp(code,"r") || !strcmp(code,"ri")) */
        else if (code[0] == 'r')
        {
          tableN[n].rtag = Biddy_Managed_GetVariable(MNG,word);
        }
        /* else if (!strcmp(code,"e") || !strcmp(code,"di")) */
        else if ((code[0] == 'e') || (code[0] == 'd'))
        {
          tableN[n].ltag = tableN[n].rtag = Biddy_Managed_GetVariable(MNG,word);
        }
      }
      /* if (!strcmp(code,"s")) */
      if ((code[0] == 's') && (code[1] == 0))
      { /* regular label */
        /* this is default type for all bdd types */
        tableN[n].l = m;
      }
      /* else if (!strcmp(code,"si")) */
      else if ((code[0] == 's') && (code[1] != 0))
      { /* complemented label */
        /* for OBDD, this is not allowed */
        /* for OBDDC, this is type 3 */
        /* for ZBDD, this is not allowed */
        /* for ZBDDC, this is type 3 */
        /* for TZBDD, this is not allowed */
        /* for TZBDDC, this is type 3 */
        tableN[n].l = m;
        if (biddyManagerType == BIDDYTYPEOBDDC) {
          tableN[n].type = 3;
        }
        else if (biddyManagerType == BIDDYTYPEZBDDC) {
          tableN[n].type = 3;
        }
        else if (biddyManagerType == BIDDYTYPETZBDDC) {
          tableN[n].type = 3;
        } else {
          printf("Biddy_ReadBddview: Problem with connection - wrong code\n");
          return NULL;
        }
      }
      /* else if (!strcmp(code,"l")) */
      else if ((code[0] == 'l') && (code[1] == 0))
      { /* regular left successor */
        /* this is default type for all bdd types, this can be changed by right successor */
        tableN[n].l = m;
      }
      /* else if (!strcmp(code,"li")) */
      else if ((code[0] == 'l') && (code[1] != 0))
      { /* complemented left successor */
        /* for OBDD, this is not allowed */
        /* for OBDDC, this is type 5 */
        /* for ZBDD, this is not allowed */
        /* for ZBDDC, this is not allowed */
        /* for TZBDD, this is not allowed */
        /* for TZBDDC, this is not allowed */
        tableN[n].l = m;
        if (biddyManagerType == BIDDYTYPEOBDDC) {
          tableN[n].type = 5;
        } else {
          printf("Biddy_ReadBddview: Problem with connection - wrong code\n");
          return NULL;
        }
      }
      /* else if (!strcmp(code,"r")) */
      else if ((code[0] == 'r') && (code[1] == 0))
      { /* regular right successor */
        /* this is default type for all bdd types, this can be changed by left successor */
        tableN[n].r = m;
      }
      /* else if (!strcmp(code,"ri")) */
      else if ((code[0] == 'r') && (code[1] != 0))
      { /* complemented right successor */
        tableN[n].r = m;
        /* for OBDD, this is not allowed */
        /* for OBDDC, this is not allowed */
        /* for ZBDD, this is not allowed */
        /* for ZBDDC, this is type 5 */
        /* for TZBDD, this is not allowed */
        /* for TZBDDC, this is type 5 */
        if (biddyManagerType == BIDDYTYPEZBDDC) {
          tableN[n].type = 5;
        }
        else if (biddyManagerType == BIDDYTYPETZBDDC) {
          tableN[n].type = 5;
        } else {
          printf("Biddy_ReadBddview: Problem with connection - wrong code\n");
          return NULL;
        }
      }
      /* else if (!strcmp(code,"d")) */
      else if ((code[0] == 'd') && (code[1] == 0))
      { /* double line, type 'd' */
        tableN[n].l = tableN[n].r = m;
        /* for OBDD, this is not allowed */
        /* for OBDDC, this is type 5 */
        /* for ZBDD, this is not allowed */
        /* for ZBDDC, this is not allowed */
        /* for TZBDD, this is not allowed */
        /* for TZBDDC, this is not allowed */
        if (biddyManagerType == BIDDYTYPEOBDDC) {
          tableN[n].type = 5;
        } else {
          printf("Biddy_ReadBddview: Problem with connection - wrong code\n");
          return NULL;
        }
      }
      /* else if (!strcmp(code,"di")) */
      else if ((code[0] == 'd') && (code[1] != 0))
      { /* double line, type 'di' */
        /* for OBDD, this is not allowed */
        /* for OBDDC, this is not allowed */
        /* for ZBDD, this is not allowed */
        /* for ZBDDC, this is type 5 */
        /* for TZBDD, this is not allowed */
        /* for TZBDDC, this is type 5 */
        tableN[n].l = tableN[n].r = m;
        if (biddyManagerType == BIDDYTYPEZBDDC) {
          tableN[n].type = 5;
        }
        else if (biddyManagerType == BIDDYTYPETZBDDC) {
          tableN[n].type = 5;
        } else {
          printf("Biddy_ReadBddview: Problem with connection - wrong code\n");
          return NULL;
        }
      }
      /* else if (!strcmp(code,"e")) */
      else if ((code[0] == 'e') && (code[1] == 0))
      { /* double line, type 'e' */
        /* this is default type for all allowed bdd types */
        tableN[n].l = tableN[n].r = m;
      }
      /* else if (!strcmp(code,"ei")) */
      else if ((code[0] == 'e') && (code[1] != 0))
      { /* double line, type 'ei' */
        /* for OBDD, this is not allowed */
        /* for OBDDC, this is not allowed */
        /* for ZBDD, this is not allowed */
        /* for ZBDDC, this is not allowed */
        /* for TZBDD, this is not allowed */
        /* for TZBDDC, this is not allowed */
        tableN[n].l = tableN[n].r = m;
        printf("Biddy_ReadBddview: Problem with connection - wrong code\n");
        return NULL;
      } else {
        printf("Biddy_ReadBddview: Problem with connection - unknown code\n");
        return NULL;
      }
      /* printf("CONNECT: <%u><%u><%s><%s><%s>\n",n,m,code,word,word2); */
    }
    else if ((line[0] == 'n') && !strncmp(line,"node",4)) { /* parsing line with node */
      line = &line[5];
      word = NULL;
      sscanf(line,"%u %s",&n,buffer);
      if (buffer[0]=='"') {
        buffer[strlen(buffer)-1] = 0;
        word = &buffer[1];
      } else {
        word = &buffer[0];
      }
      if (!numN) {
        tableN = (BiddyNodeList *) malloc(sizeof(BiddyNodeList));
        if (!tableN) return NULL;
      } else if (numN <= n) {
        tableN = (BiddyNodeList *) realloc(tableN,(n+1)*sizeof(BiddyNodeList));
        if (!tableN) return NULL;
      }
      tableN[n].name = strdup(word);
      tableN[n].id = n;
      tableN[n].type = 4; /* 4 is for regular node, this may change to type 5 later */
      tableN[n].l = tableN[n].r = -1;
      tableN[n].ltag = tableN[n].rtag = 0;
      tableN[n].f = NULL;
      tableN[n].created = FALSE;
      numN++;
      /* printf("NODE: <%u><%s>\n",n,word); */
    }
    else if ((line[0] == 't') && !strncmp(line,"terminal",8)) { /* parsing line with terminal */
      line = &line[9];
      word = NULL;
      sscanf(line,"%u %s",&n,buffer);
      if (buffer[0]=='"') {
        buffer[strlen(buffer)-1] = 0;
        word = &buffer[1];
      } else {
        word = &buffer[0];
      }
      if (!numN) {
        tableN = (BiddyNodeList *) malloc(sizeof(BiddyNodeList));
        if (!tableN) return NULL;
      } else if (numN <= n) {
        tableN = (BiddyNodeList *) realloc(tableN,(n+1)*sizeof(BiddyNodeList));
        if (!tableN) return NULL;
      }
      tableN[n].name = strdup(word);
      tableN[n].id = n;
      if (!strcmp(word,"0")) {
        tableN[n].type = 0;
      } else if (!strcmp(word,"1")) {
        tableN[n].type = 1;
      } else {
        printf("Biddy_ReadBddview: Problem with terminal\n");
        return NULL;
      }
      tableN[n].l = tableN[n].r = -1;
      tableN[n].ltag = tableN[n].rtag = 0;
      tableN[n].f = NULL;
      tableN[n].created = FALSE;
      numN++;
      /* printf("TERMINAL: <%u><%s>\n",n,word); */
    }
    else if (!typeOK && !strncmp(line,"type",4)) { /* parsing line with type */
      typeOK = TRUE;
      line = &line[5];
      /* printf("TYPE: %s\n",line); */
      if (((biddyManagerType == BIDDYTYPEOBDD) && strcmp(line,"robdd") && strcmp(line,"ROBDD")) ||
          ((biddyManagerType == BIDDYTYPEOBDDC) && strcmp(line,"robddce") && strcmp(line,"ROBDDCE")) ||
          ((biddyManagerType == BIDDYTYPEZBDD) && strcmp(line,"zbdd") && strcmp(line,"ZBDD")) ||
          ((biddyManagerType == BIDDYTYPEZBDDC) && strcmp(line,"zbddce") && strcmp(line,"ZBDDCE")) ||
          ((biddyManagerType == BIDDYTYPETZBDD) && strcmp(line,"tzbdd") && strcmp(line,"TZBDD")) ||
          ((biddyManagerType == BIDDYTYPETZBDDC) && strcmp(line,"tzbddce") && strcmp(line,"TZBDDCE")))
      {
        fprintf(stderr,"Biddy_ReadBddview: Wrong BDD type!\n");
        return NULL;
      }
    }
    else if (!varOK && !strncmp(line,"var",3)) { /* parsing line with variables */
      varOK = TRUE;
      line = &line[4];
      word = strtok(line," ");
      while (word) {
        if (word[0]=='"') {
          word[strlen(word)-1] = 0;
          word = &word[1];
        }
        if (!numV) {
          tableV = (BiddyVariableOrder *) malloc(sizeof(BiddyVariableOrder));
          if (!tableV) return NULL;
        } else {
          tableV = (BiddyVariableOrder *) realloc(tableV,(numV+1)*sizeof(BiddyVariableOrder));
          if (!tableV) return NULL;
        }
        tableV[numV].name = strdup(word);
        tableV[numV].order = numV;
        numV++;
        /* printf("<%s>\n",word); */
        word = strtok(NULL," ");
      }
      if ((biddyManagerType == BIDDYTYPEOBDDC) || (biddyManagerType == BIDDYTYPEOBDD))
      {
        for (i=0; i<numV; i++) {
          Biddy_Managed_AddVariableByName(MNG,tableV[i].name);
        }
      }
      else if ((biddyManagerType == BIDDYTYPEZBDDC) || (biddyManagerType == BIDDYTYPEZBDD))
      {
        for (i=numV-1; i>=0; i--) {
          Biddy_Managed_AddVariableByName(MNG,tableV[i].name);
        }
      }
      else if ((biddyManagerType == BIDDYTYPETZBDDC) || (biddyManagerType == BIDDYTYPETZBDD))
      {
        for (i=numV-1; i>=0; i--) {
          Biddy_Managed_AddVariableByName(MNG,tableV[i].name);
        }
      }
      else {
        fprintf(stderr,"Biddy_ReadBddview: Unknown BDD type!\n");
        return NULL;
      }
    }
    else if (!labelOK && !strncmp(line,"label",5)) { /* parsing line with label */
      labelOK = TRUE;
      line = &line[6];
      word = NULL;
      sscanf(line,"%u %s",&n,buffer);
      if (buffer[0]=='"') {
        buffer[strlen(buffer)-1] = 0;
        word = &buffer[1];
      } else {
        word = &buffer[0];
      }
      if (!numN) {
        tableN = (BiddyNodeList *) malloc(sizeof(BiddyNodeList));
        if (!tableN) return NULL;
      } else if (numN <= n) {
        tableN = (BiddyNodeList *) realloc(tableN,(n+1)*sizeof(BiddyNodeList));
        if (!tableN) return NULL;
      }
      tableN[n].name = strdup(word);
      tableN[n].id = n;
      tableN[n].type = 2; /* 2 is for regular label, this may change to type 3 later */
      tableN[n].l = tableN[n].r = -1;
      tableN[n].ltag = tableN[n].rtag = 0;
      tableN[n].f = NULL;
      tableN[n].created = FALSE;
      numN++;
      if (!name) name = strdup(word); /* only one label is supported */
      /* printf("LABEL: <%u><%s>\n",n,word); */
    }

    line = fgets(buffer,65535,bddfile);
  }

  fclose(bddfile);

  /* CONSTRUCT BDD */

  /* DEBUGGING */
  /*
  for (i=0; i<numN; i++) {
    printf("tableN[%d]: %s, type:%d, l:%d, ltag:%u<%s>, r:%d, rtag:%u<%s>\n",
           tableN[i].id,tableN[i].name,tableN[i].type,
           tableN[i].l,tableN[i].ltag,Biddy_Managed_GetVariableName(MNG,tableN[i].ltag),
           tableN[i].r,tableN[i].rtag,Biddy_Managed_GetVariableName(MNG,tableN[i].rtag));
  }
  */

  r = BiddyConstructBDD(MNG,numN,tableN);
  Biddy_Managed_AddFormula(MNG,name,r,-1);

  /* DEBUGGING */
  /*
  printf("BDD %s CONSTRUCTED\n",name);
  */

  return name;
}

#ifdef __cplusplus
}
#endif

/***************************************************************************//*!
\brief Function Biddy_Managed_ReadVerilogFile reads Verilog file and creates
       variables for all primary inputs and Boolean functions for all
       primary outputs.

### Description
    If (prefix != NULL) then the created BDD variables and formulae will get it.
### Side effects
### More info
    Original author: David Kebo Houngninou, Southern Methodist University
    Original implementation of this function is on
    https://github.com/davidkebo/verilog-parser
    Macro Biddy_ReadVerilogFile(filename,prefix) is defined for use with
    anonymous manager.
*******************************************************************************/

#ifdef __cplusplus
extern "C" {
#endif

void
Biddy_Managed_ReadVerilogFile(Biddy_Manager MNG, const char filename[],
                              Biddy_String prefix)
{
  FILE *s;
  BiddyVerilogCircuit *c;
  unsigned int linecount, modulecount;
  BiddyVerilogLine **lt;
  BiddyVerilogModule **mt;
  BiddyVerilogModule *md;
  BiddyVerilogLine *ln;
  unsigned int i,j;

  if (!MNG) MNG = biddyAnonymousManager;

  s = fopen(filename,"r");
  if (!s) return;

  printf("PARSING FILE %s\n",filename);
  c = (BiddyVerilogCircuit *) calloc(1,sizeof(BiddyVerilogCircuit)); /* declare an instance of a circuit */
  c->name = strdup("CIRCUIT"); /* set circuit name */
  parseVerilogFile(s,&linecount,&lt,&modulecount,&mt); /* read all modules from the file */

  fclose(s);

  printf("TARGET MODULE: %s\n",(mt[0])->name);

  /* DEBUGGING */
  /*
  for (i = 0; i < modulecount; i++) {
    printf("MODULE: %s\n",(mt[i])->name);
  }
  printf("ACCEPTABLE LINES\n");
  for (i = 0; i < linecount; i++) {
    printf("<%s> %s\n",(lt[i])->keyword,(lt[i])->line);
  }
  */

  createVerilogCircuit(linecount,lt,modulecount,mt,c); /* create circuit object for the given table of acceptable lines and table of modules */
  createBddFromVerilogCircuit(MNG,c,prefix); /* create BDDs for all primary outputs */

  for (i = 0; i < linecount; i++) {
    ln = lt[i];
    free(ln->keyword);
    free(ln->line);
    free(ln);
  }
  free(lt);

  for (i = 0; i < modulecount; i++) {
    md = mt[i];
    for (j = 0; j < md->outputcount; j++) free (md->outputs[j]);
    for (j = 0; j < md->inputcount; j++) free (md->inputs[j]);
    for (j = 0; j < md->wirecount; j++) free (md->wires[j]);
    for (j = 0; j < md->gatecount; j++) free (md->gates[j]);
    free(md->name);
    free(md);
  }
  free(mt);

  for (i=0; i < c->outputcount; i++) free (c->outputs[i]);

  for (i=0; i < c->inputcount; i++) free (c->inputs[i]);

  for (i=0; i < c->nodecount; i++) {
    free(c->nodes[i]->type);
    free(c->nodes[i]->name);
    free(c->nodes[i]);
  }
  free(c->nodes);

  for (i=0; i < c->wirecount; i++) {
    free (c->wires[i]->type);
    free (c->wires[i]);
  }
  free(c->wires);

  free(c->name);
  free(c);
}

#ifdef __cplusplus
}
#endif

/***************************************************************************//*!
\brief Function Biddy_Managed_PrintfBDD writes raw format using printf.

### Description
    This function is intended for writing to an output channel via
    macro which overrides the meaning of standard printf calls.
    For writing raw format into the file, use Biddy_Managed_WriteBDD.
### Side effects
### More info
    Macro Biddy_PrintfBDD(f) is defined for use with anonymous manager.
*******************************************************************************/

#ifdef __cplusplus
extern "C" {
#endif

void
Biddy_Managed_PrintfBDD(Biddy_Manager MNG, Biddy_Edge f)
{
  if (!MNG) MNG = biddyAnonymousManager;

  if (Biddy_IsNull(f)) {
    printf("NULL\n");
  } else {
    WriteBDD(MNG,f);
    printf("\n");
  }
}

#ifdef __cplusplus
}
#endif

/***************************************************************************//*!
\brief Function Biddy_Managed_WriteBDD writes raw format using fprintf.

### Description
### Side effects
### More info
    Macro Biddy_WriteBDD(f) is defined for use with anonymous manager.
*******************************************************************************/

#ifdef __cplusplus
extern "C" {
#endif

void
Biddy_Managed_WriteBDD(Biddy_Manager MNG, const char filename[], Biddy_Edge f,
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
\brief Function Biddy_Managed_PrintfTable writes truth table using printf.

### Description
    This function is intended for writing to an output channel via
    macro which overrides the meaning of standard printf calls.
    For writing truth table into the file, use Biddy_Managed_WriteTable.
### Side effects
### More info
    Thanks to Jan Kraner and Ziga Kobale for prototype implementation.
    Macro Biddy_PrintfTable(f) is defined for use with anonymous manager.
*******************************************************************************/

#ifdef __cplusplus
extern "C" {
#endif

void
Biddy_Managed_PrintfTable(Biddy_Manager MNG, Biddy_Edge f)
{
  unsigned int variableNumber;
  Biddy_Variable* variableTable;
  unsigned int i;
  int j;
  Biddy_Variable k,v;
  unsigned int numcomb;

  if (!MNG) MNG = biddyAnonymousManager;

  if (Biddy_IsNull(f)) {
    printf("NULL\n");
    return;
  }

  /* variableNumber is the number of dependent variables in BDD */
  variableNumber = Biddy_Managed_DependentVariableNumber(MNG,f,TRUE);

  /* DEBUGGING */
  /*
  reportOrdering();
  printf("DEPENDENT: %u\n",variableNumber);
  printf("TOP VARIABLE: %s\n",Biddy_Managed_GetVariableName(MNG,BiddyV(f)));
  for (k = 1; k < biddyVariableTable.num; k++) {
    if (biddyVariableTable.table[k].selected == TRUE) {
      printf("SELECTED: %s\n",Biddy_Managed_GetVariableName(MNG,k));
    }
  }
  */

  if (variableNumber > 6) {
    printf("Table for %d variables is to large for output.\n",variableNumber);
    for (k = 0; k < biddyVariableTable.num; k++) {
      biddyVariableTable.table[k].selected = FALSE; /* deselect variable */
    }
    return;
  }

  /* variableTable is a table of all dependent variables in BDD */
  if (!(variableTable = (Biddy_Variable *) malloc((variableNumber) * sizeof(Biddy_Variable)))) return;
  i = 0;
  v = Biddy_Managed_GetLowestVariable(MNG); /* lowest = topmost */
  for (k = 1; k < biddyVariableTable.num; k++) {
    if (biddyVariableTable.table[v].selected == TRUE) {
      variableTable[i] = v;
      printf("|%s",Biddy_Managed_GetVariableName(MNG,v));
      biddyVariableTable.table[v].selected = FALSE; /* deselect variable */
      i++;
    }
    v = biddyVariableTable.table[v].next;
  }
  printf("|:value\n");

  numcomb = 1;
  for (i = 0; i < variableNumber; i++) numcomb = 2 * numcomb;
  for (i = 0; i < numcomb; i++)
  {
    for (j = variableNumber - 1; j >= 0; j--) {
      biddyVariableTable.table[variableTable[variableNumber-j-1]].value = (i&(1 << j))?biddyOne:biddyZero;
      if (i&(1 << j)) {
        printf("%*c",1+(int)strlen(
          Biddy_Managed_GetVariableName(MNG,variableTable[variableNumber-j-1])
        ),'1');
      } else {
        printf("%*c",1+(int)strlen(
          Biddy_Managed_GetVariableName(MNG,variableTable[variableNumber-j-1])
        ),'0');
      }
    }
    printf(" :");
    if (Biddy_Managed_Eval(MNG,f)) printf("1\n"); else printf("0\n");
  }

  free(variableTable);
}

#ifdef __cplusplus
}
#endif


/***************************************************************************//*!
\brief Function Biddy_Managed_WriteTable writes truth table using fprintf.

### Description
### Side effects
### More info
    Thanks to Jan Kraner and Ziga Kobale for prototype implementation.
    Macro Biddy_WriteTable(f) is defined for use with anonymous manager.
*******************************************************************************/

#ifdef __cplusplus
extern "C" {
#endif

void
Biddy_Managed_WriteTable(Biddy_Manager MNG, const char filename[], Biddy_Edge f)
{
  unsigned int variableNumber;
  Biddy_Variable* variableTable;
  unsigned int i;
  int j;
  Biddy_Variable k,v;
  unsigned int numcomb;
  FILE *s;

  if (!MNG) MNG = biddyAnonymousManager;

  if (filename) {
    s = fopen(filename,"w");
  } else {
    s = stdout;
  }
  if (!s) return;

  if (Biddy_IsNull(f)) {
    fprintf(s,"NULL\n");
    fclose(s);
    return;
  }

  /* variableNumber is the number of dependent variables in BDD */
  variableNumber = Biddy_Managed_DependentVariableNumber(MNG,f,TRUE);

  if (variableNumber > 6) {
    fprintf(s,"Table for %d variables is to large for output.\n",variableNumber);
    for (k = 0; k < biddyVariableTable.num; k++) {
      biddyVariableTable.table[k].selected = FALSE; /* deselect variable */
    }
    fclose(s);
    return;
  }

  /* variableTable is a table of all dependent variables in BDD */
  if (!(variableTable = (Biddy_Variable *) malloc((variableNumber) * sizeof(Biddy_Variable)))) return;
  i = 0;
  v = Biddy_Managed_GetLowestVariable(MNG); /* lowest = topmost */
  for (k = 1; k < biddyVariableTable.num; k++) {
    if (biddyVariableTable.table[v].selected == TRUE) {
      variableTable[i] = v;
      fprintf(s,"|%s",Biddy_Managed_GetVariableName(MNG,v));
      biddyVariableTable.table[v].selected = FALSE; /* deselect variable */
      i++;
    }
    v = biddyVariableTable.table[v].next;
  }
  fprintf(s,"|:value\n");

  numcomb = 1;
  for (i = 0; i < variableNumber; i++) numcomb = 2 * numcomb;
  for (i = 0; i < numcomb; i++)
  {
    for (j = variableNumber - 1; j >= 0; j--)
    {
      biddyVariableTable.table[variableTable[variableNumber-j-1]].value = (i&(1 << j))?biddyOne:biddyZero;
      if (i&(1 << j)) {
        fprintf(s,"%*c",1+(int)strlen(
          Biddy_Managed_GetVariableName(MNG,variableTable[variableNumber-j-1])
        ),'1');
      } else {
        fprintf(s,"%*c",1+(int)strlen(
          Biddy_Managed_GetVariableName(MNG,variableTable[variableNumber-j-1])
        ),'0');
      }
    }
    fprintf(s," :");
    if (Biddy_Managed_Eval(MNG,f)) fprintf(s,"1\n"); else fprintf(s,"0\n");
  }

  free(variableTable);

  if (filename) {
    fclose(s);
  }
}

#ifdef __cplusplus
}
#endif

/***************************************************************************//*!
\brief Function Biddy_Managed_PrintfSOP writes SOP using printf.

### Description
### Side effects
### More info
*******************************************************************************/

#ifdef __cplusplus
extern "C" {
#endif

void
Biddy_Managed_PrintfSOP(Biddy_Manager MNG, Biddy_Edge f)
{
  unsigned int maxsize = 100;
  Biddy_Variable top;

  if (!MNG) MNG = biddyAnonymousManager;

  if (Biddy_IsNull(f)) {
    printf("  NULL\n");
    return;
  }

  if (Biddy_IsTerminal(f)) {
    if (f == biddyZero) {
      printf("  + 0\n");
      return;
    }
    else if (f == biddyOne) {
      printf("  + 1\n");
      return;
    }
  }

  top = BiddyV(f);
  if (biddyManagerType == BIDDYTYPEOBDD) {
    /* IMPLEMENTED */
  }
  else if (biddyManagerType == BIDDYTYPEOBDDC) {
    /* IMPLEMENTED */
  }
  else if (biddyManagerType == BIDDYTYPEZBDD) {
    /* IMPLEMENTED */
    while (biddyVariableTable.table[top].prev != biddyVariableTable.num) {
      top = biddyVariableTable.table[top].prev;
    }
  }
  else if (biddyManagerType == BIDDYTYPEZBDDC) {
    /* IMPLEMENTED */
    while (biddyVariableTable.table[top].prev != biddyVariableTable.num) {
      top = biddyVariableTable.table[top].prev;
    }
  }
  else if (biddyManagerType == BIDDYTYPETZBDD) {
    /* IMPLEMENTED */
    top = Biddy_GetTag(f);
  }
  else if (biddyManagerType == BIDDYTYPETZBDDC)
  {
    fprintf(stderr,"Biddy_PrintfSOP: this BDD type is not supported, yet!\n");
    return;
  }
  else if ((biddyManagerType == BIDDYTYPEOFDD) || (biddyManagerType == BIDDYTYPEOFDDC) ||
            (biddyManagerType == BIDDYTYPEZFDD) || (biddyManagerType == BIDDYTYPEZFDDC) ||
            (biddyManagerType == BIDDYTYPETZFDD) || (biddyManagerType == BIDDYTYPETZFDDC))
  {
    fprintf(stderr,"Biddy_PrintfSOP: this BDD type is not supported, yet!\n");
    return;
  }
  else {
    fprintf(stderr,"Biddy_PrintfSOP: Unsupported BDD type!\n");
    return;
  }

  WriteSOP(MNG,f,top,Biddy_GetMark(f),NULL,&maxsize,FALSE);
  printf("\n");
}

#ifdef __cplusplus
}
#endif

/***************************************************************************//*!
\brief Function Biddy_Managed_WriteSOP writes SOP using fprintf.

### Description
### Side effects
### More info
*******************************************************************************/

#ifdef __cplusplus
extern "C" {
#endif

void
Biddy_Managed_WriteSOP(Biddy_Manager MNG, const char filename[], Biddy_Edge f)
{
  unsigned int maxsize = 100;
  Biddy_Variable top;
  FILE *s;

  if (!MNG) MNG = biddyAnonymousManager;

  if (filename) {
    s = fopen(filename,"w");
  } else {
    s = stdout;
  }
  if (!s) return;

  if (Biddy_IsNull(f)) {
    fprintf(s,"  NULL\n");
    if (filename) {
      fclose(s);
    }
    return;
  }

  if (Biddy_IsTerminal(f)) {
    if (f == biddyZero) {
      fprintf(s,"  + 0\n");
      if (filename) {
        fclose(s);
      }
    }
    else if (f == biddyOne) {
      fprintf(s,"  + 1\n");
      if (filename) {
        fclose(s);
      }
    }
  }

  top = BiddyV(f);
  if (biddyManagerType == BIDDYTYPEOBDD) {
    /* IMPLEMENTED */
  }
  else if (biddyManagerType == BIDDYTYPEOBDDC) {
    /* IMPLEMENTED */
  }
  else if (biddyManagerType == BIDDYTYPEZBDD) {
    /* IMPLEMENTED */
    while (biddyVariableTable.table[top].prev != biddyVariableTable.num) {
      top = biddyVariableTable.table[top].prev;
    }
  }
  else if (biddyManagerType == BIDDYTYPEZBDDC) {
    /* IMPLEMENTED */
    while (biddyVariableTable.table[top].prev != biddyVariableTable.num) {
      top = biddyVariableTable.table[top].prev;
    }
  }
  else if (biddyManagerType == BIDDYTYPETZBDD) {
    /* IMPLEMENTED */
    top = Biddy_GetTag(f);
  }
  else if (biddyManagerType == BIDDYTYPETZBDDC)
  {
    fprintf(stderr,"Biddy_PrintfSOP: this BDD type is not supported, yet!\n");
    if (filename) {
      fclose(s);
    }
    return;
  }
  else if ((biddyManagerType == BIDDYTYPEOFDD) || (biddyManagerType == BIDDYTYPEOFDDC) ||
            (biddyManagerType == BIDDYTYPEZFDD) || (biddyManagerType == BIDDYTYPEZFDDC) ||
            (biddyManagerType == BIDDYTYPETZFDD) || (biddyManagerType == BIDDYTYPETZFDDC))
  {
    fprintf(stderr,"Biddy_PrintfSOP: this BDD type is not supported, yet!\n");
    if (filename) {
      fclose(s);
    }
    return;
  }
  else {
    fprintf(stderr,"Biddy_PrintfSOP: Unsupported BDD type!\n");
    if (filename) {
      fclose(s);
    }
    return;
  }

  WriteSOPx(MNG,s,f,top,Biddy_GetMark(f),NULL,&maxsize,FALSE);

  if (filename) {
    fclose(s);
  }

}

#ifdef __cplusplus
}
#endif


/***************************************************************************//*!
\brief Function Biddy_Managed_PrintfMinterms writes minterms using printf.

### Description
    If negative == true then writes complete minterms (useful for Boolean
    functions), otherwise writes only positive literals (useful for
    combination sets).
### Side effects
    For OBDDs, the used algorithm is imitating writing a truth table,
    this is not an efficient approach (suitable only for a few variables)!
    For ZBDDs and TZBDDs, the used algorithm is imitating writing SOP,
    this is very efficient.
    This function is intended for writing to an output channel via
    macro which overrides the meaning of standard printf calls.
### More info
    Macro Biddy_PrintfMinterms(f) is defined for use with anonymous manager.
*******************************************************************************/

static void PrintfMintermsOBDD(Biddy_Manager MNG, Biddy_Edge f, Biddy_Boolean negative);

#ifdef __cplusplus
extern "C" {
#endif

void
Biddy_Managed_PrintfMinterms(Biddy_Manager MNG, Biddy_Edge f,
                             Biddy_Boolean negative)
{
  unsigned int maxsize = 0; /* needed for WriteSOP but not used */

  if (!MNG) MNG = biddyAnonymousManager;

  if (Biddy_IsNull(f)) {
    printf("NULL\n");
    return;
  }

  if (f == biddyZero) {
    printf("EMPTY\n");
    return;
  }

  if ((biddyManagerType == BIDDYTYPEOBDDC) || (biddyManagerType == BIDDYTYPEOBDD)) {
    PrintfMintermsOBDD(MNG,f,negative);
  }
  else if ((biddyManagerType == BIDDYTYPEZBDDC) || (biddyManagerType == BIDDYTYPEZBDD)) {
    WriteSOP(MNG,f,Biddy_Managed_GetTopVariable(MNG,f),Biddy_GetMark(f),NULL,&maxsize,TRUE);
    printf("\n");
  }
  else if (biddyManagerType == BIDDYTYPETZBDD) {
    WriteSOP(MNG,f,Biddy_Managed_GetLowestVariable(MNG),Biddy_GetMark(f),NULL,&maxsize,TRUE);
    printf("\n");
  }
  else if (biddyManagerType == BIDDYTYPETZBDDC)
  {
    printf("Biddy_PrintfMinterms: this BDD type is not supported, yet!\n");
    return;
  } else if ((biddyManagerType == BIDDYTYPEOFDDC) || (biddyManagerType == BIDDYTYPEOFDD) ||
              (biddyManagerType == BIDDYTYPEZFDDC) || (biddyManagerType == BIDDYTYPEZFDD) ||
              (biddyManagerType == BIDDYTYPETZFDDC) || (biddyManagerType == BIDDYTYPETZFDD))
  {
    printf("Biddy_PrintfMinterms: this BDD type is not supported, yet!\n");
    return;
  } else {
    printf("Biddy_PrintfMinterms: Unsupported BDD type!\n");
    return;
  }
}

#ifdef __cplusplus
}
#endif

static void
PrintfMintermsOBDD(Biddy_Manager MNG, Biddy_Edge f, Biddy_Boolean negative)
{
  unsigned int variableNumber;
  Biddy_Variable* variableTable;
  unsigned int i;
  int j;
  Biddy_Variable k,v;
  unsigned int numcomb;
  Biddy_Boolean first;

  /* all variables are included into minterms! */
  variableNumber = biddyVariableTable.num - 1;

  /* DEBUGGING */
  /*
  printf("variableNumber = %u\n",variableNumber);
  */

  if (variableNumber > 16) {
    printf("Biddy_PrintfMinterms: to many variables, variableNumber = %d, max = 16\n",variableNumber);
    return;
  }

  /* variableTable is a table of all variables - the active ordering is considered */
  if (!(variableTable = (Biddy_Variable *) malloc((variableNumber) * sizeof(Biddy_Variable)))) return;
  i = 0;
  v = Biddy_Managed_GetLowestVariable(MNG); /* lowest = topmost */
  for (k = 1; k < biddyVariableTable.num; k++) {
    variableTable[i] = v;
    i++;
    v = biddyVariableTable.table[v].next;
  }

  numcomb = 1;
  for (i = 0; i < variableNumber; i++) numcomb = 2 * numcomb;
  /* for (i = numcomb - 1; i >= 0; i--) */
  for (i = 0; i < numcomb; i++)
  {
    for (j = variableNumber - 1; j >= 0; j--) {
      biddyVariableTable.table[variableTable[variableNumber-j-1]].value = (i&(1 << j))?biddyOne:biddyZero;
    }
    if (Biddy_Managed_Eval(MNG,f)) {
      printf("{");
      first = TRUE;
      for (j = variableNumber - 1; j >= 0; j--) {
        if (i&(1 << j)) {
          if (first) first = FALSE; else printf(",");
          printf("%s",Biddy_Managed_GetVariableName(MNG,variableTable[variableNumber-j-1]));
        } else {
          if (negative) {
            if (first) first = FALSE; else printf(",");
            printf("*%s",Biddy_Managed_GetVariableName(MNG,variableTable[variableNumber-j-1]));
          }
        }
      }
      printf("}");
    }
  }
  printf("\n");

  free(variableTable);
}

/***************************************************************************//*!
\brief Function Biddy_Managed_WriteDot writes dot/graphviz format using
       fprintf.

### Description
    Output dot format. Two approaches are implemented.
    The CUDD-like implementation is copyied from CUDD 3.0.
### Side effects
    If (id != -1) then id is used instead of <...> for variable names.
    If (filename == NULL) then output is to stdout.
    Function resets all variables value.
    Variable names containing # are adpated.
### More info
    Macro Biddy_WriteDot(filename,f,label,id,cudd) is defined for use with
    anonymous manager.
*******************************************************************************/

#ifdef __cplusplus
extern "C" {
#endif

unsigned int
Biddy_Managed_WriteDot(Biddy_Manager MNG, const char filename[], Biddy_Edge f,
                       const char label[], int id, Biddy_Boolean cudd)
{
  unsigned int n;
  char *label1;
  char *hash;
  FILE *s;
  Biddy_Variable tag;
  BiddyLocalInfo *li;
  Biddy_Variable v;
  Biddy_String name;

  if (!MNG) MNG = biddyAnonymousManager;

  /* if (id != -1) use it instead of <...> */

  if (Biddy_IsNull(f)) {
    printf("WARNING (Biddy_Managed_WriteDot): Function is null!\n");
    return 0;
  }

  if (filename) {
    s = fopen(filename,"w");
  } else {
    s = stdout;
  }
  if (!s) return 0;

  if (Biddy_IsTerminal(f)) {
    n = 1; /* there is only one terminal node */
  } else {
    BiddyCreateLocalInfo(MNG,f);
    n = enumerateNodes(MNG,f,0); /* this will select all nodes except terminal node */
    Biddy_Managed_DeselectAll(MNG);
  }

  label1 = strdup(label);
  while ((hash=strchr(label1,'#'))) hash[0]='_'; /* avoid hashes in the name */

  if (cudd) {
    fprintf(s,"//GENERATED WITH BIDDY - biddy.meolic.com\n");
    if (filename) {
      fprintf(s,"//USE 'dot -y -Tpng -O %s' TO VISUALIZE %s\n",filename,biddyManagerName);
    }
    fprintf(s,"digraph \"DD\" {\n");
    /* fprintf(s,"size = \"7.5,10\"\n"); */
    fprintf(s,"center=true;\nedge [dir=none];\n");
    fprintf(s,"{ node [shape=plaintext];\n");
    fprintf(s,"  edge [style=invis];\n");
    fprintf(s,"  \"CONST NODES\" [style = invis];\n");

    if (!Biddy_IsTerminal(f)) {
      li = (BiddyLocalInfo *)(BiddyN(f)->list);
      Biddy_Managed_ResetVariablesValue(MNG);
      while (li->back) {
        v = BiddyV(li->back);
        if (biddyVariableTable.table[v].value == biddyZero) {
          biddyVariableTable.table[v].value = biddyOne;
          /* variable names containing # are adpated */
          name = strdup(Biddy_Managed_GetVariableName(MNG,v));
          while ((hash = strchr(name,'#'))) hash[0] = '_';
          fprintf(s,"\" %s \" -> ",name);
          free(name);
        }
        li = &li[1]; /* next field in the array */
      }
      Biddy_Managed_ResetVariablesValue(MNG);
    }

    fprintf(s,"\"CONST NODES\"; \n}\n");
    fprintf(s,"{ rank=same; node [shape=box]; edge [style=invis];\n");
    fprintf(s,"\"%s\"",label1);
    fprintf(s,"; }\n");
  } else {
    fprintf(s,"//GENERATED WITH BIDDY - biddy.meolic.com\n");
    if (filename) {
      fprintf(s,"//USE 'dot -y -Tpng -O %s' TO VISUALIZE %s\n",filename,biddyManagerName);
    }
    fprintf(s,"digraph ");
    if (biddyManagerType == BIDDYTYPEOBDD) fprintf(s,"ROBDD");
    else if (biddyManagerType == BIDDYTYPEZBDD) fprintf(s,"ZBDD");
    else if (biddyManagerType == BIDDYTYPETZBDD) fprintf(s,"TZBDD");
    else if (biddyManagerType == BIDDYTYPEOBDDC) fprintf(s,"ROBDDCE");
    else if (biddyManagerType == BIDDYTYPEZBDDC) fprintf(s,"ZBDDCE");
    else if (biddyManagerType == BIDDYTYPETZBDDC) fprintf(s,"TZBDDCE");
    else fprintf(s,"BDD");
    fprintf(s," {\n");
    fprintf(s,"  ordering=out;\n");
#ifdef LEGACY_DOT
#else
    fprintf(s,"  edge [dir=\"both\"];\n");
#endif
    fprintf(s,"  edge [arrowhead=\"none\"]\n");
    fprintf(s,"  node [shape=none, label=\"%s\"] 0;\n",label1);
  }

  WriteDotNodes(MNG,s,f,id,cudd);

  if (((biddyManagerType == BIDDYTYPEOBDD) ||
       (biddyManagerType == BIDDYTYPEZBDD) ||
       (biddyManagerType == BIDDYTYPETZBDD) ||
       !(Biddy_GetMark(f))))
  {
    if ((f != biddyZero) && (tag = Biddy_GetTag(f))) {
      if (cudd) {
        fprintf(s,"\"%s\"",label1);
        if (Biddy_IsTerminal(f)) {
          fprintf(s," -> \"1\" [style=solid label=\"%s\"];\n",Biddy_Managed_GetVariableName(MNG,tag));
        } else {
          fprintf(s," -> \"%p\" [style=solid label=\"%s\"];\n",BiddyP(f),Biddy_Managed_GetVariableName(MNG,tag));
        }
      } else {
#ifdef LEGACY_DOT
        fprintf(s,"  0 -> 1 [style=solid label=\"%s\"];\n",Biddy_Managed_GetVariableName(MNG,tag));
#else
        fprintf(s,"  0 -> 1 [arrowtail=\"none\" label=\"%s\"];\n",Biddy_Managed_GetVariableName(MNG,tag));
#endif
      }
    } else {
      if (cudd) {
        fprintf(s,"\"%s\"",label1);
        if (Biddy_IsTerminal(f)) {
          fprintf(s," -> \"1\" [style=solid];\n");
        } else {
          fprintf(s," -> \"%p\" [style=solid];\n",BiddyP(f));
        }
      } else {
#ifdef LEGACY_DOT
        fprintf(s,"  0 -> 1 [style=solid];\n");
#else
        fprintf(s,"  0 -> 1 [arrowtail=\"none\"];\n");
#endif
      }
    }
  } else {
    if ((f != biddyZero) && (tag = Biddy_GetTag(f))) {
      if (cudd) {
        fprintf(s,"\"%s\"",label1);
        if (Biddy_IsTerminal(f)) {
          fprintf(s," -> \"1\" [style=dotted label=\"%s\"];\n",Biddy_Managed_GetVariableName(MNG,tag));
        } else {
          fprintf(s," -> \"%p\" [style=dotted label=\"%s\"];\n",BiddyP(f),Biddy_Managed_GetVariableName(MNG,tag));
        }
      } else {
#ifdef LEGACY_DOT
        fprintf(s,"  0 -> 1 [style=dotted label=\"%s\"];\n",Biddy_Managed_GetVariableName(MNG,tag));
#else
        fprintf(s,"  0 -> 1 [arrowtail=\"none\" arrowhead=\"dot\" label=\"%s\"];\n",Biddy_Managed_GetVariableName(MNG,tag));
#endif
      }
    } else {
      if (cudd) {
        fprintf(s,"\"%s\"",label1);
        if (Biddy_IsTerminal(f)) {
          if ((biddyManagerType == BIDDYTYPEOBDDC) || (biddyManagerType == BIDDYTYPEOBDD)) {
            fprintf(s," -> \"1\" [style=dotted];\n");
          } else {
            fprintf(s," -> \"1\" [style=bold];\n");
          }
        } else {
          if ((biddyManagerType == BIDDYTYPEOBDDC) || (biddyManagerType == BIDDYTYPEOBDD)) {
            fprintf(s," -> \"%p\" [style=dotted];\n",BiddyP(f));
          } else {
            fprintf(s," -> \"%p\" [style=bold];\n",BiddyP(f));
          }
        }
      } else {
#ifdef LEGACY_DOT
        if ((biddyManagerType == BIDDYTYPEOBDDC) || (biddyManagerType == BIDDYTYPEOBDD)) {
          fprintf(s,"  0 -> 1 [style=dotted];\n");
        } else {
          fprintf(s,"  0 -> 1 [style=bold];\n");
        }
#else
        fprintf(s,"  0 -> 1 [arrowtail=\"none\" arrowhead=\"dot\"];\n");
#endif
      }
    }
  }

  free(label1);

  WriteDotEdges(MNG,s,f,cudd); /* this will select all nodes except terminal node */

  if (!Biddy_IsTerminal(f)) {
    BiddyDeleteLocalInfo(MNG,f);
  }

  fprintf(s,"}\n");

  if (filename) {
    fclose(s);
  }

  return n;
}

#ifdef __cplusplus
}
#endif

/***************************************************************************//*!
\brief Function Biddy_Managed_WriteBDDView writes bddview format using fprintf.

### Description
    Output bddview format.
    Parameter table is optional, if not NULL then it must contain
    node names and coordinates. If (filename == NULL) then output is to stdout.
### Side effects
    A conservative approach to list all the existing variables in the manager
    (and not only the dependent ones) is used. Variable names containing # are
    adpated. To support EST, also variable names containing <> are adapted.
    The optional table is type-casted to BiddyXYTable.
### More info
    Macro Biddy_WriteBddview(filename,f,label) is defined for use with anonymous
    manager.
*******************************************************************************/

#ifdef __cplusplus
extern "C" {
#endif

unsigned int
Biddy_Managed_WriteBddview(Biddy_Manager MNG, const char filename[],
                           Biddy_Edge f, const char label[], void *xytable)
{
  FILE *s;
  unsigned int i,n;
  Biddy_Boolean useCoordinates;
  BiddyLocalInfo *li;
  Biddy_Variable v,k,tag;
  char *hash;
  Biddy_String name;
  BiddyXY *table;

  if (!MNG) MNG = biddyAnonymousManager;

  if (Biddy_IsNull(f)) {
    /* printf("ERROR Biddy_Managed_WriteBddview: Function is null!\n"); */
    return 0 ;
  }

  if (filename) {
    s = fopen(filename,"w");
  } else {
    s = stdout;
  }
  if (!s) {
    printf("ERROR: s = 0\n");
    return(0);
  }

  table = (BiddyXY *)xytable;

  fprintf(s,"#GENERATED WITH BIDDY - biddy.meolic.com\n");

  /* WRITE BDD TYPE */

  if (biddyManagerType == BIDDYTYPEOBDD) {
    fprintf(s,"type ROBDD\n");
  } else if (biddyManagerType == BIDDYTYPEOBDDC) {
    fprintf(s,"type ROBDDCE\n");
  } else if (biddyManagerType == BIDDYTYPEZBDD) {
    fprintf(s,"type ZBDD\n");
  } else if (biddyManagerType == BIDDYTYPEZBDDC) {
    fprintf(s,"type ZBDDCE\n");
  } else if (biddyManagerType == BIDDYTYPETZBDD) {
    fprintf(s,"type TZBDD\n");
  } else if (biddyManagerType == BIDDYTYPETZBDDC) {
    fprintf(s,"type TZBDDCE\n");
  } else if (biddyManagerType == BIDDYTYPEOFDD) {
    fprintf(s,"type ROFDD\n");
  } else if (biddyManagerType == BIDDYTYPEOFDDC) {
    fprintf(s,"type ROFDDCE\n");
  } else if (biddyManagerType == BIDDYTYPEZFDD) {
    fprintf(s,"type ZFDD\n");
  } else if (biddyManagerType == BIDDYTYPEZFDDC) {
    fprintf(s,"type ZFDDCE\n");
  } else if (biddyManagerType == BIDDYTYPETZFDD) {
    fprintf(s,"type TZFDD\n");
  } else if (biddyManagerType == BIDDYTYPETZFDDC) {
    fprintf(s,"type TZFDDCE\n");
  }

  /* WRITE VARIABLES */
  /* A conservative approach to list all the existing variables */
  /* in the manager (and not only the dependent ones) is used. */

  fprintf(s,"var");
  v = Biddy_Managed_GetLowestVariable(MNG); /* lowest = topmost */
  for (k = 1; k < biddyVariableTable.num; k++) {
    name = strdup(Biddy_Managed_GetVariableName(MNG,v));

    /* variable names containing # are adpated */
    while ((hash = strchr(name,'#'))) hash[0] = '_';
    /* to support EST, also variable names containing <> are adapted */
    for (i=0; i<strlen(name); i++) {
      if (name[i] == '<') name[i] = '_';
      if (name[i] == '>') name[i] = 0;
    }

    fprintf(s," \"%s\"",name);
    free(name);
    v = biddyVariableTable.table[v].next;
  }
  fprintf(s,"\n");

  /* PREPARE CALCULATION */

  useCoordinates = FALSE;
  if (Biddy_IsTerminal(f)) {

    n = 1; /* for all BDD types, in memory there is only one terminal node */
    if (table) {
      /* use table of nodes given by the user */
      useCoordinates = TRUE;
    } else {
      n = 1; /* there is one terminal node */
      table = (BiddyXY *) malloc((n+1) * sizeof(BiddyXY)); /* n = nodes, add one label */
      if (!table) return 0;
      table[0].id = 0;
      table[0].label = strdup(label);
      table[0].x = 0;
      table[0].y = 0;
      table[0].isConstant = FALSE;
      table[1].id = 1;
      table[1].label = getname(MNG,f);
      table[1].x = 0;
      table[1].y = 0;
      table[1].isConstant = TRUE;
    }

  } else {

    BiddyCreateLocalInfo(MNG,f);
    n = enumerateNodes(MNG,f,0); /* this will select all nodes except terminal node */
    Biddy_Managed_DeselectAll(MNG);

    if (table) {
      /* use table of nodes given by the user */
      useCoordinates = TRUE;
    } else {
      /* generate table of nodes */
      /* TO DO: coordinates are not calculated, how to do this without graphviz/dot? */
      table = (BiddyXY *) malloc((n+1) * sizeof(BiddyXY)); /* n = nodes, add one label */
      if (!table) return 0;
      table[0].id = 0;
      table[0].label = strdup(label);
      table[0].x = 0;
      table[0].y = 0;
      table[0].isConstant = FALSE;
      li = (BiddyLocalInfo *)(BiddyN(f)->list);
      while (li->back) {

        /* add one node */
        table[li->data.enumerator].id = li->data.enumerator;
        table[li->data.enumerator].label = getname(MNG,li->back);
        table[li->data.enumerator].x = 0;
        table[li->data.enumerator].y = 0;
        table[li->data.enumerator].isConstant = FALSE;

        /* if one or both successors are terminal node then extra nodes must be added */
        if (Biddy_IsTerminal(BiddyE(li->back)) || Biddy_IsTerminal(BiddyT(li->back))) {
          if (((biddyManagerType == BIDDYTYPEOBDD) ||
               (biddyManagerType == BIDDYTYPEZBDD) ||
               (biddyManagerType == BIDDYTYPETZBDD)
              ) && (Biddy_GetMark(BiddyE(li->back)) != Biddy_GetMark(BiddyT(li->back))))
          {
            /* two terminal nodes only if complemented edges are not used and they are different */
            unsigned int num;
            num = 0;
            if (Biddy_IsTerminal(BiddyE(li->back))) {
              num = 1;
              table[li->data.enumerator+1].id = li->data.enumerator+1;
              if (BiddyE(li->back) == biddyZero) {
                table[li->data.enumerator+1].label = strdup("0");
              } else {
                table[li->data.enumerator+1].label = strdup("1");
              }
              table[li->data.enumerator+1].x = 0;
              table[li->data.enumerator+1].y = 0;
              table[li->data.enumerator+1].isConstant = TRUE;
            }
            if (Biddy_IsTerminal(BiddyT(li->back))) {
              table[li->data.enumerator+num+1].id = li->data.enumerator+num+1;
              if (BiddyT(li->back) == biddyZero) {
                table[li->data.enumerator+num+1].label = strdup("0");
              } else {
                table[li->data.enumerator+num+1].label = strdup("1");
              }
              table[li->data.enumerator+num+1].x = 0;
              table[li->data.enumerator+num+1].y = 0;
              table[li->data.enumerator+num+1].isConstant = TRUE;
            }
          } else {
            /* only one terminal node */
            table[li->data.enumerator+1].id = li->data.enumerator+1;
            table[li->data.enumerator+1].label = strdup("1");
            table[li->data.enumerator+1].x = 0;
            table[li->data.enumerator+1].y = 0;
            table[li->data.enumerator+1].isConstant = TRUE;
          }
        }

        li = &li[1]; /* next field in the array */
      }
    }

  }

  /* WRITE bddview NODES */

  if (useCoordinates) {
    fprintf(s,"label %d %s %d %d\n",table[0].id,table[0].label,table[0].x,table[0].y);
  } else {
    fprintf(s,"label %d %s\n",table[0].id,table[0].label);
  }
  for (i=1;i<(n+1);i++) {
    if (table[i].isConstant) {
      fprintf(s,"terminal ");
    } else {
      fprintf(s,"node ");
    }
    if (useCoordinates) {
      fprintf(s,"%d %s %d %d\n",table[i].id,table[i].label,table[i].x,table[i].y);
    } else {
      fprintf(s,"%d %s\n",table[i].id,table[i].label);
    }
  }

  if (!useCoordinates) {
    for (i=1;i<(n+1);i++) {
      free(table[i].label);
    }
    free(table);
  }

  /* WRITE bddview CONNECTIONS */

  fprintf(s,"connect 0 1 ");
  if ((biddyManagerType == BIDDYTYPEOBDD) ||
      (biddyManagerType == BIDDYTYPEZBDD) ||
      (biddyManagerType == BIDDYTYPETZBDD))
  {
    fprintf(s,"s");
  }
  else {
    if (Biddy_GetMark(f)) {
      fprintf(s,"si");
    } else {
      fprintf(s,"s");
    }
  }

  if ((biddyManagerType == BIDDYTYPETZBDDC) || (biddyManagerType == BIDDYTYPETZBDD))
  {
    tag = Biddy_GetTag(f);
    fprintf(s," %s\n",Biddy_Managed_GetVariableName(MNG,tag));
  } else {
    fprintf(s,"\n");
  }

  if (!Biddy_IsTerminal(f)) {
    WriteBddviewConnections(MNG,s,f); /* this will select all nodes except terminal node */
    BiddyDeleteLocalInfo(MNG,f);
  }

  if (filename) {
    fclose(s);
  }

  return n;
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

  /* DEBUGGING */
  /*
  printf("%c",c);
  */

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

    /* DEBUGGING */
    /*
    printf("%c",c);
    */

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

  assert( (biddyManagerType == BIDDYTYPEOBDD) || (biddyManagerType == BIDDYTYPEOBDDC) );

  nextCh(s,i,ch);

  if (strcmp(*ch,"*")) {
    inv = FALSE;
  } else {
    inv = TRUE;
    /* printf("<*>\n"); */
    nextCh(s,i,ch);
  }

  /* printf("<%s>\n",*ch); */

  if (!strcmp(*ch,"0")) {
    if (inv) {
      f = biddyOne;
    } else {
      f = biddyZero;
    }
  } else if (!strcmp(*ch,"1")) {
    if (inv) {
      f = biddyZero;
    } else {
      f = biddyOne;
    }
  } else {
    varname = strdup(*ch);
    n = Biddy_Managed_GetVariableEdge(MNG,Biddy_Managed_GetVariable(MNG,varname)); /* variables should already exist */
    if (!n) return biddyNull;
    nextCh(s,i,ch);
    if (strcmp(*ch,"(")) {
      /**/ printf("ERROR: <%s>\n",*ch); /**/
      return biddyNull;
    }
    e = ReadBDD(MNG,s,i,ch);
    nextCh(s,i,ch);
    if (strcmp(*ch,")")) {
      /**/ printf("ERROR: <%s>\n",*ch); /**/
      return biddyNull;
    }
    nextCh(s,i,ch);
    if (strcmp(*ch,"(")) {
      /**/ printf("ERROR: <%s>\n",*ch); /**/
      return biddyNull;
    }
    t = ReadBDD(MNG,s,i,ch);
    nextCh(s,i,ch);
    if (strcmp(*ch,")")) {
      /**/ printf("ERROR: <%s>\n",*ch); /**/
      return biddyNull;
    }
    if (inv) {
      f = Biddy_Managed_Not(MNG,Biddy_Managed_ITE(MNG,n,t,e));
    } else {
      f = Biddy_Managed_ITE(MNG,n,t,e);
    }
    free(varname);
  }

  return f;
}

static Biddy_Edge
evaluate1(Biddy_Manager MNG, Biddy_String s, int *i, Biddy_String *ch,
          Biddy_LookupFunction lf)
{
  unsigned int idx;
  Biddy_Edge f;
  int op;

  /* DEBUGGING */
  /*
  printf("evaluate1: %s\n",s);
  printf("evaluate1 i = %d\n",*i);
  printf("evaluate1 ch = %s\n",*ch);
  */

  if (!strcmp(*ch,"(")) {
    nextCh(s,i,ch);

    if (!strcasecmp(*ch, "NOT")) {
      nextCh(s,i,ch);
      f = Biddy_Managed_Not(MNG,evaluate1(MNG,s,i,ch,lf));
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
  /* entries in user's formula should not be obsolete */

  if (charOK(*ch[0])) {
    if (!lf) {
      if (!Biddy_Managed_FindFormula(MNG,*ch,&idx,&f)) {
        f = Biddy_Managed_GetVariableEdge(MNG,Biddy_Managed_AddVariableByName(MNG,*ch));
      }
    } else {
      if ((!(lf(*ch,&f))) &&
          (!Biddy_Managed_FindFormula(MNG,*ch,&idx,&f))) {
        f = Biddy_Managed_GetVariableEdge(MNG,Biddy_Managed_AddVariableByName(MNG,*ch));
      }
    }
    nextCh(s,i,ch);

    assert( f != biddyNull );

    return f;
  }

  return biddyNull;
}

static Biddy_Edge
evaluateN(Biddy_Manager MNG, Biddy_String s, int *i, Biddy_String *ch,
          Biddy_Edge g, int op, Biddy_LookupFunction lf)
{
  Biddy_Edge f, h;

  /* DEBUGGING */
  /*
  printf("evaluateN: %s\n",s);
  printf("evaluateN i = %d\n",*i);
  printf("evaluateN ch = %s\n",*ch);
  */

  h = evaluate1(MNG,s,i,ch,lf);
  if (!Biddy_IsNull(h)) {
    f = biddyNull;
    switch (op) {
            case oAND:  f = Biddy_Managed_And(MNG,g,h);
                        break;
            case oOR:  f = Biddy_Managed_Or(MNG,g,h);
                        break;
            case oEXOR:  f = Biddy_Managed_Xor(MNG,g,h);
    }
    return evaluateN(MNG,s,i,ch,f,op,lf);
  }
  return g;
}

static int
Op(Biddy_String s, int *i, Biddy_String *ch)
{

  /* DEBUGGING */
  /*
  printf("Op ch = %s\n",*ch);
  */

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

static void
createVariablesFromBTree(Biddy_Manager MNG, BiddyBTreeContainer *tree)
{
  int i;
  unsigned int idx;
  Biddy_Edge tmp;

  if ((biddyManagerType == BIDDYTYPEOBDDC) || (biddyManagerType == BIDDYTYPEOBDD))
  {
    for (i = 0; i <= tree->availableNode; i++) {
      if (tree->tnode[i].op == 255) {
        if (tree->tnode[i].name) {
          if (!(Biddy_Managed_FindFormula(MNG,tree->tnode[i].name,&idx,&tmp))) {
            Biddy_Managed_GetVariableEdge(MNG,Biddy_Managed_AddVariableByName(MNG,tree->tnode[i].name));
          }
        }
      }
    }
  }

  if ((biddyManagerType == BIDDYTYPEZBDDC) || (biddyManagerType == BIDDYTYPEZBDD) ||
      (biddyManagerType == BIDDYTYPETZBDDC) || (biddyManagerType == BIDDYTYPETZBDD))
  {
    for (i = tree->availableNode; i >= 0; i--) {
      if (tree->tnode[i].op == 255) {
        if (tree->tnode[i].name) {
          if (!(Biddy_Managed_FindFormula(MNG,tree->tnode[i].name,&idx,&tmp))) {
            Biddy_Managed_GetVariableEdge(MNG,Biddy_Managed_AddVariableByName(MNG,tree->tnode[i].name));
          }
        }
      }
    }
  }
}

static Biddy_Edge
createBddFromBTree(Biddy_Manager MNG, BiddyBTreeContainer *tree, int i)
{
  unsigned int idx;
  Biddy_Edge lbdd,rbdd,fbdd;

  if (i == -1) return biddyNull;
  if (tree->tnode[i].op == 255) {
    if (tree->tnode[i].name) {
      if (!strcmp(tree->tnode[i].name,"0")) fbdd = biddyZero;
      else if (!strcmp(tree->tnode[i].name,"1")) fbdd = biddyOne;
      else {
        if (!(Biddy_Managed_FindFormula(MNG,tree->tnode[i].name,&idx,&fbdd))) {
          fbdd = Biddy_Managed_GetVariableEdge(MNG,Biddy_Managed_AddVariableByName(MNG,tree->tnode[i].name));
        }
      }
    } else {
      fbdd = biddyNull;
    }

    /* DEBUGGING */
    /*
    printf("createBddFromBTree: op == 255\n");
    printPrefixFromBTree(tree,i);
    printf("\n");
    printf("HERE IS fbdd:\n");
    Biddy_PrintfBDD(fbdd);
    Biddy_PrintfSOP(fbdd);
    */

    return fbdd;
  }
  lbdd = createBddFromBTree(MNG,tree,tree->tnode[i].left);
  rbdd = createBddFromBTree(MNG,tree,tree->tnode[i].right);

  /* NOT (~!), AND (&*), OR (|+), XOR (^%) , XNOR(-), IMPLIES (><), NAND (@), NOR (#), BUTNOT (\), AND NOTBUT (/) ARE IMPLEMENTED */
  /* char boolOperators[] = { '~', '&', '\\', ' ', '/', ' ', '^', '|', '#', '-', ' ', '<', ' ', '>', '@', ' ' }; */
  if (tree->tnode[i].op == 0) fbdd = Biddy_Managed_Not(MNG,rbdd); /* NOT */
  else if (tree->tnode[i].op == 1) fbdd = Biddy_Managed_And(MNG,lbdd,rbdd); /* AND */
  else if (tree->tnode[i].op == 2) fbdd = Biddy_Managed_Gt(MNG,lbdd,rbdd); /* BUTNOT */
  else if (tree->tnode[i].op == 4) fbdd = Biddy_Managed_Gt(MNG,rbdd,lbdd); /* NOTBUT */
  else if (tree->tnode[i].op == 6) fbdd = Biddy_Managed_Xor(MNG,lbdd,rbdd); /* XOR */
  else if (tree->tnode[i].op == 7) fbdd = Biddy_Managed_Or(MNG,lbdd,rbdd); /* OR */
  else if (tree->tnode[i].op == 8) fbdd = Biddy_Managed_Nor(MNG,lbdd,rbdd); /* NOR */
  else if (tree->tnode[i].op == 9) fbdd = Biddy_Managed_Xnor(MNG,lbdd,rbdd); /* XNOR */
  else if (tree->tnode[i].op == 11) fbdd = Biddy_Managed_Leq(MNG,rbdd,lbdd); /* IMPLIES <= */
  else if (tree->tnode[i].op == 13) fbdd = Biddy_Managed_Leq(MNG,lbdd,rbdd); /* IMPLIES => */
  else if (tree->tnode[i].op == 14) fbdd = Biddy_Managed_Nand(MNG,lbdd,rbdd); /* NAND */
  else fbdd = biddyNull;

  /* DEBUGGING */
  /*
  printf("createBddFromBTree: op == %u\n",tree->tnode[i].op);
  printPrefixFromBTree(tree,i);
  printf("\n");
  printf("HERE IS lbdd:\n");
  Biddy_PrintfBDD(lbdd);
  Biddy_PrintfSOP(lbdd);
  printf("HERE IS rbdd:\n");
  Biddy_PrintfBDD(rbdd);
  Biddy_PrintfSOP(rbdd);
  printf("HERE IS fbdd:\n");
  Biddy_PrintfBDD(fbdd);
  Biddy_PrintfSOP(fbdd);
  */

  return fbdd;
}

static void
parseVerilogFile(FILE *verilogfile, unsigned int *l, BiddyVerilogLine ***lt, unsigned int *n, BiddyVerilogModule ***mt)
{
  unsigned int activemodule=0;
  BiddyVerilogLine *ln;
  BiddyVerilogModule *md;
  unsigned int i=0, j=0, k=0; /* indexes */
  char ch, linebuf[LINESIZE]; /* buffer for single line from the verilog file */
  Biddy_String buffer; /* buffer for complete verilog statements */
  Biddy_String token[TOKENNUM]; /* array to hold tokens for the line */
  Biddy_String keyword; /* keyword from verilog line */
  Biddy_Boolean comment, noname, ok, acceptable;
  Biddy_String t,tc;
  unsigned int tn;

  (*l) = 0;
  (*lt) = NULL;
  (*n) = 0;
  (*mt) = NULL;

  keyword = NULL;
  buffer = NULL;
  md = NULL;
  while (fgets(linebuf,LINESIZE,verilogfile) != NULL) {

    if (keyword) free(keyword);
    tn = (unsigned int) strspn(linebuf," ");

    if (tn == strlen(linebuf)) {
      continue; /* skip empty lines and white space */
    }

    /* get 1st word from the line */
    tn += (unsigned int) strcspn(&linebuf[tn]," ");
    ch = 0;
    if (tn < strlen(linebuf)) {
      ch = linebuf[tn];
      linebuf[tn] = 0;
    }
    tn = (unsigned int) strspn(linebuf," ");
    keyword = strdup(&linebuf[tn]);
    if (ch) {
      tn += (unsigned int) strcspn(&linebuf[tn]," ");
      linebuf[tn] = ch;
    }
    keyword = trim(keyword);

    if (!strlen(keyword)) {
      continue; /* skip white space not recognised before */
    }

    if (!strcmp(keyword,"//")) {
      continue; /* skip comment lines */
    }

    if (buffer) free(buffer);
    buffer = strdup("");
    if (!strcmp(keyword,"endmodule")) { /* endmodule keyword does not use ';' */
      concat(&buffer,keyword);
    } else {
      concat(&buffer,linebuf);
      while (!isEndOfLine(linebuf)) { /* check if the line ends with a ';' character (multiple lines statement) */
        if (fgets(linebuf,LINESIZE,verilogfile) != NULL) {/* otherwise, append all the following lines */
          concat(&buffer,linebuf);
          if ((strlen(buffer) + LINESIZE) > BUFSIZE) printf("ERROR (parseVerilogFile): BUFFER TO SMALL\n");
        }
      }
    }

    /* tokenize the line to extract data */
    noname = FALSE;
    if (strcmp(keyword,"module") && strcmp(keyword,"endmodule") &&
        strcmp(keyword,"input") && strcmp(keyword,"output") &&
        strcmp(keyword,"wire") && strcmp(keyword,"reg") &&
        strcmp(keyword,"assign"))
    {
      tn = (unsigned int) strspn(buffer," ");
      tn += (unsigned int) strcspn(&buffer[tn]," ");
      tn += (unsigned int) strspn(&buffer[tn]," ");
      if (buffer[tn] == '(') noname = TRUE;
    }
    i=0;
    comment = FALSE;
    t = strtok(buffer," ");
    token[i++] = strdup(t); /* full keyword */
    if (noname) {
      sprintf(linebuf,"NONAME%d",(*l));
      token[i++] = strdup(linebuf); /* add NONAME */
    }
    while ((t = strtok(NULL," (),;\r\n")) && (strcmp(t,"//"))) {
      if (!strcmp(t,"/*")) comment = TRUE;
      if (!comment) {
        if (strlen(t)>1) {
          tn = (unsigned int) strcspn(t,"{}");
          while (tn != strlen(t)) {
            tc = NULL;
            if (t[tn] == '{') tc = strdup("{");
            if (t[tn] == '}') tc = strdup("}");
            if (tn) {
              t[tn] = 0;
              token[i++] = strdup(t);
              if (i >= TOKENNUM) printf("ERROR (parseVerilogFile): TOKENNUM TO SMALL\n");
            }
            token[i++] = tc;
            if (i >= TOKENNUM) printf("ERROR (parseVerilogFile): TOKENNUM TO SMALL\n");
            t = &t[tn+1];
            tn = (unsigned int) strcspn(t,"{}");
          }
        }
        if (strlen(t)) {
          token[i++] = strdup(t);
          if (i >= TOKENNUM) printf("ERROR (parseVerilogFile): TOKENNUM TO SMALL\n");
        }
      }
      if (comment && !strcmp(t,"*/")) comment = FALSE;
    }
    token[i] = NULL;

    if (!strcmp(keyword,"module")) { /* START OF MODULE */
      (*n)++;
      (*mt) = (BiddyVerilogModule **) realloc((*mt), (*n) * sizeof(BiddyVerilogModule *)); /* create or enlarge table of modules */
      md = (BiddyVerilogModule *) calloc(1, sizeof(BiddyVerilogModule)); /* declare an instance of a module */
      (*mt)[(*n)-1] = md;
      md->name = (Biddy_String) calloc(strlen(token[1]) + 1, sizeof(char)); /* allocating memory for module name string */
      strcpy(md->name,token[1]); /* set module name */
      activemodule = (*n);
      /* DEBUGGING */
      /*
      printf("MODULE: %s (#%d)\n",token[1],activemodule);
      */
      md->outputFirst = TRUE; /* TRUE iff "nand NAND2_1 (N1, N2, N3);" defines N1 = NAND(N2,N3), FALSE iff N3 = NAND(N1,N2) */
    }

    if (!strcmp(keyword,"endmodule")) { /* END OF MODULE */
      /* DEBUGGING */
      /*
      printf("ENDMODULE: #%d\n",activemodule);
      */
      activemodule = 0;
      md = NULL;
    } else {
      if (!activemodule) {
        printf("ERROR (parseVerilogFile): statement <%s> is outside of module\n",keyword);
        continue;
      }
    }

    acceptable = FALSE;
    if ((activemodule == 1) || !strcmp(keyword,"module") || !strcmp(keyword,"endmodule")) {
      /* TO DO: MULTIPLE MODULES ARE NOT SUPPORTED, YET */

      if (!strcmp(keyword,"module") || !strcmp(keyword,"endmodule")) {
        acceptable = TRUE;
      }

      else if (!strcmp(keyword,"input")) { /* PRIMARY INPUTS */
        acceptable = TRUE;
        for (j = 1; j < i; j++) { /* parse all the words in the line */
          if (isSignalVector(token[j])) /* handle a vector of signals, this is not implemented correctly! */
            parseSignalVector(md->inputs, token, &j, &md->inputcount);
          else { /* not a vector of signal */
            md->inputs[md->inputcount] = (Biddy_String) calloc(strlen(token[j]) + 1, sizeof(char)); /* allocating memory for input string */
            strcpy(md->inputs[md->inputcount],token[j]); /* add the input name to the array of inputs */
            (md->inputcount)++; /* update the number of inputs in the circuit */
            if (md->inputcount >= INOUTNUM) printf("ERROR (parseVerilogFile): INOUTNUM TO SMALL\n");
          }
        }
      }

      else if (!strcmp(keyword,"output")) { /* PRIMARY OUTPUTS */
        acceptable = TRUE;
        for (j = 1; j < i; j++) { /* parse all the words in the line */
          if (isSignalVector(token[j])) /* handle a vector of signals, this is not implemented correctly! */
            parseSignalVector(md->outputs, token, &j, &md->outputcount);
          else { /* not a vector of signal */
            md->outputs[md->outputcount] = (Biddy_String) calloc(strlen(token[j]) + 1, sizeof(char)); /* allocating memory for output string */
            strcpy(md->outputs[md->outputcount],token[j]); /* add the output name to the array of outputs */
            (md->outputcount)++; /* update the number of outputs in the circuit */
            if (md->outputcount >= INOUTNUM) printf("ERROR (parseVerilogFile): INOUTNUM TO SMALL\n");
          }
        }
      }

      else if (!strcmp(keyword,"wire")) { /* WIRES */
        for (j = 1; j < i; j++) { /* parse all the words in the line */
          if (isSignalVector(token[j])) /* handle a vector of signals, this is not implemented correctly! */
            parseSignalVector(md->wires, token, &j, &md->wirecount);
          else { /* not a vector of signal */
            /* check if this wire has not already been stated as input or output */
            ok = TRUE;
            for (k=0; k < md->inputcount; k++) {
              if (!strcmp(md->inputs[k],token[j])) ok = FALSE;
            }
            for (k=0; k < md->outputcount; k++) {
              if (!strcmp(md->outputs[k],token[j])) ok = FALSE;
            }
            if (ok) {
              acceptable = TRUE;
              md->wires[md->wirecount] = (Biddy_String) calloc(strlen(token[j]) + 1, sizeof(char)); /* allocating memory for wire string */
              strcpy(md->wires[md->wirecount],token[j]); /* add the wire name to the array of wires */
              (md->wirecount)++; /* update the number of wires in the circuit */
              if (md->wirecount >= WIRENUM) printf("ERROR (parseVerilogFile): WIRENUM TO SMALL\n");
            } else {
              /* DEBUGGING */
              /*
              printf("WARNING (parseVerilogFile): wire %s already defined as input/output\n",token[j]);
              */
            }
          }
        }
      }

      else if (!strcmp(keyword,"reg")) { /* REGS */
        acceptable = TRUE;
        for (j = 1; j < i; j++) { /* parse all the words in the line */
          if (isSignalVector(token[j])) /* handle a vector of signals, this is not implemented correctly! */
            parseSignalVector (md->regs, token, &j, &md->regcount);
          else { /* not a vector of signal */
            md->regs[md->regcount] = (Biddy_String) calloc(strlen(token[j]) + 1, sizeof(char)); /* allocating memory for reg string */
            strcpy(md->regs [md->regcount],token[j]); /* add the reg name to the array of regs */
            (md->regcount)++; /* update the number of regs in the circuit */
            if (md->regcount >= REGNUM) printf("ERROR (parseVerilogFile): REGNUM TO SMALL\n");
          }
        }
      }

      else if (isGate(keyword)) { /* BASIC GATES */
        acceptable = TRUE;
        md->gates[md->gatecount] = (Biddy_String) calloc(strlen(token[1]) + 1, sizeof(char)); /* allocating memory for gate name string */
        strcpy(md->gates[md->gatecount],token[1]); /* add the gate name to the array of gates */
        if (!md->gatecount) { /* use the first gate to determine md->outputFirst */
          for (j=0; md->inputs[j] != NULL; j++) if (!strcmp(md->inputs[j],token[2])) md->outputFirst = FALSE;
        }
        (md->gatecount)++; /* update the number of gates in the circuit */
        if (md->gatecount >= GATENUM) printf("ERROR (parseVerilogFile): GATENUM TO SMALL\n");
      }

      else {
        /* AN UNIMPLEMENTED KEYWORD */
        /* GATES DEFINED BY OTHER MODULES ARE NOT RESOLVED */
      }

    }

    /* add this line to the table of acceptable lines */
    if (acceptable) {
      (*l)++;
      (*lt) = (BiddyVerilogLine **) realloc((*lt), (*l) * sizeof(BiddyVerilogLine *)); /* create or enlarge table of lines */
      ln = (BiddyVerilogLine *) calloc(1, sizeof(BiddyVerilogLine)); /* declare an instance of a line */
      ln->keyword = strdup(keyword);
      ln->line = strdup(token[0]);
      for (j=1; j<i; j++) concat(&(ln->line),token[j]);
      (*lt)[(*l)-1] = ln;
    }

  }

  if (keyword) free(keyword);
  if (buffer) free(buffer);

  /* DEBUGGING - print summary of the module */
  /*
  printModuleSummary(m);
  */
}

/**
 * Build a circuit from a parsed verilog description (target module is the first one!)
 * @param a table of acceptable lines, a table of modules, a circuit object
 */
static void
createVerilogCircuit(unsigned int linecount, BiddyVerilogLine **lt, unsigned int modulecount, BiddyVerilogModule **mt, BiddyVerilogCircuit *c)
{
  BiddyVerilogModule *m;
  unsigned int i=0, j=0, k=0, l=0;
  int in=0, id=0, index=0;
  unsigned int activemodule=0;
  Biddy_String keyword, buffer;
  Biddy_String token[TOKENNUM]; /* array to hold tokens for one line */
  Biddy_Boolean ok;
  Biddy_String tmptoken;

  memset(token, 0, sizeof(char*) * TOKENNUM);

  m = mt[0]; /* target module is the first one in the table */

  /* initialization of the circuit */
  c->inputcount = m->inputcount; /* set the number of inputs for the ciruit */
  c->outputcount = m->outputcount; /* set the number of outputs for the circuit */
  c->wirecount = m->inputcount + m->outputcount + m->wirecount + m->gatecount; /* set the number of wires for the circuit */
  c->nodecount = m->inputcount + m->outputcount + m->wirecount + m->gatecount; /* set the number of nodes for the circuit */
  c->wires = (BiddyVerilogWire **) calloc(c->wirecount,sizeof(BiddyVerilogWire *)); /* allocate a contiguous array to index every wire */
  c->nodes = (BiddyVerilogNode **) calloc(c->nodecount,sizeof(BiddyVerilogNode *)); /* allocate a contiguous array to index every node */

  id = 0;
  for (i=0; i < m->inputcount; i++) { /* store the names of primary inputs */
    c->inputs[i] = (Biddy_String) calloc(strlen(m->inputs[i]) + 1, sizeof (char));
    strcpy(c->inputs[i],m->inputs[i]);
    c->nodes[id] = (BiddyVerilogNode *) calloc(1,sizeof(BiddyVerilogNode));
    buildNode(c->nodes[id],(Biddy_String)"input", m->inputs[i]);
    id++;
  }
  for (i=0; i < m->outputcount; i++) { /* store the names of primary outputs */
    c->outputs[i] = (Biddy_String) calloc(strlen(m->outputs[i]) + 1, sizeof(char));
    strcpy(c->outputs[i],m->outputs[i]);
    c->nodes[id] = (BiddyVerilogNode *) calloc(1,sizeof(BiddyVerilogNode));
    buildNode(c->nodes[id],(Biddy_String)"output",m->outputs[i]);
    id++;
  }
  for (i=0; i < m->wirecount; i++) { /* store the names of wires */
    c->nodes[id] = (BiddyVerilogNode *) calloc(1,sizeof(BiddyVerilogNode));
    buildNode(c->nodes[id],(Biddy_String)"wire",m->wires[i]);
    id++;
  }
  for (i=0; i < m->gatecount; i++) { /* store the names of gates */
    c->nodes[id] = (BiddyVerilogNode *) calloc(1,sizeof(BiddyVerilogNode));
    buildNode(c->nodes[id],(Biddy_String)"gate",m->gates[i]);
    id++;
  }

  index = 0;
  for (l=0; l<linecount;l++) {
    keyword = lt[l]->keyword;
    buffer = lt[l]->line;

    if (!strcmp(keyword,"module")) {
      activemodule++;
      continue;
    }

    if (!strcmp(keyword,"endmodule")) {
      continue;
    }

    if (activemodule > 1) {
      /* TO DO: MULTIPLE MODULES ARE NOT FULLY SUPPORTED, YET */
      continue;
    }

    if (!strcmp(keyword,"input")) {
      continue;
    }
    if (!strcmp(keyword,"output")) {
      continue;
    }
    if (!strcmp(keyword,"wire")) {
      continue;
    }
    if (!strcmp(keyword,"reg")) {
      continue;
    }
    if (!strcmp(keyword,"assign")) {
      continue;
    }

    /* tokenize the line to extract data */
    i=0;
    token[0] = strtok(buffer," ");
    while(token[i]!= NULL) {
      /* DEBUGGING */
      /*
      printf("(#%d) TOKENIZE: <%s>\n",i,token[i]);
      */
      i++;
      token[i] = strtok(NULL," ");
    }

    if (!isGate(keyword)) {
      printf("WARNING (createVerilogCircuit): SKIP <%s>\n",buffer);
      continue;
    }

    if (!m->outputFirst) {
      /* m->outputFirst == FALSE iff "nand NAND2_1 (N1, N2, N3);" defines N3 = NAND(N1,N2) */
      /* m->outputFirst == TRUE iff "nand NAND2_1 (N1, N2, N3);" defines N1 = NAND(N2,N3) */
      /* exchange token[2] and token[i-1] */
      tmptoken = token[2];
      token[2] = token[i-1];
      token[i-1] = tmptoken;
    }

    /* A. Check or create wires for all the gate inputs */
    /* if wire is not known it will be treated as a primary input */
    for (j = 3; j < i; j++) {
      if (!isDefined(c,token[j])) {
        ok = FALSE;
        for (k=0; k < m->wirecount; k++) { /* check the names of wires */
          if (!strcmp(m->wires[k],token[j])) {
            printf("ERROR (createVerilogCircuit): wire %s used before defined!\n",token[j]);
            exit(1);
          }
        }
        for (k=0; !ok && k < m->inputcount; k++) { /* check the names of primary inputs */
          if (!strcmp(m->inputs[k],token[j])) ok = TRUE;
        }
        if (!ok) {
          printf("WARNING (createVerilogCircuit): wire %s treated as primary input!\n",token[j]);
        }
        c->wires[index] = (BiddyVerilogWire *) calloc(1,sizeof(BiddyVerilogWire));
        buildWire(c,c->wires[index],(Biddy_String)"I",token[j]);
        /* assign inputs to the wires representing gate inputs */
        /* NOT NEEDED */
        /* assign outputs to the wires representing gate inputs */
        /* NOT USED */
        index++;
      }
    }

    /* B. Create a wire for the gate */
    c->wires[index] = (BiddyVerilogWire *) calloc(1,sizeof(BiddyVerilogWire));
    buildWire(c,c->wires[index],keyword,token[1]);
    /* assign inputs to the wire representing gate */
    in = 0;
    for (j = 3; j < i; j++) {
      c->wires[index]->inputs[in] = getNodeIdByName(c,token[j]);
      c->wires[index]->inputcount++;
      in++;
    }
    /* assign outputs to the wire representing gate */
    /* NOT USED */
    index++;

    /* C. Check or create a wire for the gate output */
    if (!isDefined(c,token[2])) { /* if wire is not already defined */
      c->wires[index] = (BiddyVerilogWire *) calloc(1,sizeof(BiddyVerilogWire));
      buildWire(c,c->wires[index],(Biddy_String)"W",token[2]);
      /* assign inputs to the wire representing gate ouput */
      c->wires[index]->inputs[0] = getNodeIdByName(c,token[1]);
      c->wires[index]->inputcount = 1;
      /* assign outputs to the wire representing gate ouput */
      /* NOT USED */
      index++;
    } else { /* if wire is already defined */
      getWireByName(c,token[2])->inputs[0] = getNodeIdByName(c,token[1]); /* find the wire and attach an input to it */
      getWireByName(c,token[2])->inputcount = 1;
    }

  }

  c->wirecount = index;

  /* c->wires[i]->fanout is the count of gates (and wires), where */
  /* this wire is used as an input */
  i=0;
  while (i<c->wirecount && c->wires[i] != NULL) {
    for (j=0; j<c->wires[i]->inputcount; j++) {
      (getWireById(c,c->wires[i]->inputs[j])->fanout)++;
      /* DEBUGGING */
      /*
      printf("Increment TTL for wire %d\n",c->wires[i]->inputs[j]);
      */
    }
    i++;
  }

  /* DEBUGGING - print summary of the circuit */
  /*
  printCircuitSummary(c);
  */
}

/**
 * Build a bdd from a verilog circuit object
 * @param a verilog circuit object, optional prefix for output names
 * NOTE: Because intermediate results are stored in C variables, new
 * BDD variables must not be added on-the-fly for suppressed types,
 * e.g. ZBDD and TZBDD. Thus, it is important to create all variables before
 * the calculation.
 */
static void
createBddFromVerilogCircuit(Biddy_Manager MNG, BiddyVerilogCircuit *c, Biddy_String prefix)
{
  unsigned int i,j;
  BiddyVerilogWire *w;
  Biddy_Edge f;

  /* DEBUGGING */
  /*
  printf("Creating %d primary inputs\n",c->inputcount);
  */

  /* DEBUGGING */
  /*
  for (i = 0; i < c->inputcount; i++) {
    printf("%s ",c->inputs[i]);
  }
  if (c->inputcount) printf("\n");
  */

  /* DEBUGGING */
  /*
  printf("Creating %d primary outputs\n",c->outputcount);
  */

  /* DEBUGGING */
  /*
  for (i = 0; i < c->outputcount; i++) {
    printf("%s ",c->outputs[i]);
  }
  if (c->outputcount) printf("\n");
  */

  /* DEBUGGING */
  /*
  printf("Creating %d wires\n",c->wirecount);
  */

  /* create all BDD variables - important for ZBDD and TZBDD */
  for (i = 0; i < c->nodecount; i++) {
    if (!strcmp(c->nodes[i]->type,"input")) {
      Biddy_Managed_FoaVariable(MNG,c->nodes[i]->name,TRUE);  /* TO DO: add prefix */
    }
    if (!strcmp(c->nodes[i]->type,"extern")) {
      Biddy_Managed_FoaVariable(MNG,c->nodes[i]->name,TRUE);  /* TO DO: add prefix */
    }
  }

  /* prepare c->wires[i]->ttl which will be used to guide GC */
  for (i = 0; i < c->wirecount; i++) {
    c->wires[i]->ttl = 0;
  }
  for (i = 0; i < c->wirecount; i++) {
    for (j = 0; j < c->wires[i]->inputcount; j++) {
      w = getWireById(c,c->wires[i]->inputs[j]);
      (w->ttl)++;
      if (w->ttl == w->fanout) {
        w->ttl = i;
      }
    }
  }
  for (i = 0; i < c->outputcount; i++) {
    w = getWireByName(c,c->outputs[i]); /* if NULL then this output is undefined! */
    if (w) {
      w->ttl = c->wirecount - 1;
    }
  }
  for (i = 0; i < c->wirecount; i++) {
    if (c->wires[i]->ttl) c->wires[i]->ttl = c->wires[i]->ttl - i;
  }

  for (i = 0; i < c->wirecount; i++) {

    /* REPORT PROGRESS */
    /**/
    printf("#%d",i);
    if (i == c->wirecount-1) printf("\n");
    /**/

    /* DEBUGGING */
    /*
    printf("Creating c->wire[%d], ",i);
    printf("ID: %d, ",c->wires[i]->id);
    printf("name: %s, ",c->nodes[c->wires[i]->id]->name);
    printf("type: %s, ",c->wires[i]->type);
    printf("fanout: %d, ",c->wires[i]->fanout);
    printf("ttl: %d, ",c->wires[i]->ttl);
    printf("Inputs (%d): ",c->wires[i]->inputcount);
    if (strcmp(c->wires[i]->type,"I")) {
      if (c->wires[i]->inputcount) {
        for (j=0;j<c->wires[i]->inputcount;j++) {
          printf("%d ",c->wires[i]->inputs[j]);
          if (getWireById(c,c->wires[i]->inputs[j])->bdd == biddyNull) {
            printf("ERROR (gate ordering is wrong)");
          }
        }
      } else {
        printf("ERROR (wire must be a primary input or it must have some inputs)");
      }
    } else {
      if (c->wires[i]->inputcount) {
        printf("ERROR (primary input cannot have additional inputs)");
      } else {
        printf("/");
      }
    }
    printf("\n");
    */

    f = biddyNull;

    if (!strcmp(c->wires[i]->type,"I")) {
      if (c->wires[i]->inputcount) {
        printf("ERROR (primary input cannot have additional inputs)\n");
      } else {
        f = Biddy_Managed_GetVariableEdge(MNG,Biddy_Managed_GetVariable(MNG,c->nodes[c->wires[i]->id]->name)); /* TO DO: add prefix */
      }
    }
    else if (!strcmp(c->wires[i]->type,"M")) {
      /* TO DO: this will be used for gates defined by other modules */
    }
    else {
      if (c->wires[i]->inputcount) {
        for (j=0; j<c->wires[i]->inputcount; j++) {
          if (getWireById(c,c->wires[i]->inputs[j])->bdd == biddyNull) {
            printf("ERROR (gate ordering is wrong)\n");
          }
        }

        if (!strcmp(c->wires[i]->type,"W")) {
          if (c->wires[i]->inputcount == 1) {
            f = getWireById(c,c->wires[i]->inputs[0])->bdd;
          } else {
            printf("ERROR (internal wire must have exactly one input)\n");
          }
        }

        else if (!strcmp(c->wires[i]->type,"buf"))
        {
          if (c->wires[i]->inputcount != 1) {
            printf("ERROR (buf gate must have exactly 1 input)\n");
          } else {
            f = getWireById(c,c->wires[i]->inputs[0])->bdd;
          }
        }

        else if (!strcmp(c->wires[i]->type,"and"))
        {
          f = biddyOne;
          for (j=0; j<c->wires[i]->inputcount; j++) {
            f = Biddy_Managed_And(MNG,f,getWireById(c,c->wires[i]->inputs[j])->bdd);
          }
        }

        else if (!strcmp(c->wires[i]->type,"nand"))
        {
          f = biddyOne;
          for (j=0; j<c->wires[i]->inputcount; j++) {
            f = Biddy_Managed_And(MNG,f,getWireById(c,c->wires[i]->inputs[j])->bdd);
          }
          f = Biddy_Managed_Not(MNG,f);
        }

        else if (!strcmp(c->wires[i]->type,"or"))
        {
          f = biddyZero;
          for (j=0; j<c->wires[i]->inputcount; j++) {
            f = Biddy_Managed_Or(MNG,f,getWireById(c,c->wires[i]->inputs[j])->bdd);
          }
        }

        else if (!strcmp(c->wires[i]->type,"nor"))
        {
          f = biddyZero;
          for (j=0; j<c->wires[i]->inputcount; j++) {
            f = Biddy_Managed_Or(MNG,f,getWireById(c,c->wires[i]->inputs[j])->bdd);
          }
          f = Biddy_Managed_Not(MNG,f);
        }

        else if (!strcmp(c->wires[i]->type,"xor"))
        {
          if (c->wires[i]->inputcount != 2) {
            printf("ERROR (xor gate not having exactly 2 inputs is not supported)\n");
          } else {
            f = Biddy_Managed_Xor(MNG,getWireById(c,c->wires[i]->inputs[0])->bdd,
                                      getWireById(c,c->wires[i]->inputs[1])->bdd);
          }
        }

        else if (!strcmp(c->wires[i]->type,"xnor"))
        {
          if (c->wires[i]->inputcount != 2) {
            printf("ERROR (xnor gate not having exactly 2 inputs is not supported)\n");
          } else {
            f = Biddy_Managed_Xor(MNG,getWireById(c,c->wires[i]->inputs[0])->bdd,
                                      getWireById(c,c->wires[i]->inputs[1])->bdd);
            f = Biddy_Managed_Not(MNG,f);
          }
        }

        else if (!strcmp(c->wires[i]->type,"not") ||
                 !strcmp(c->wires[i]->type,"inv"))
        {
          if (c->wires[i]->inputcount != 1) {
            printf("ERROR (not gate must have exactly 1 input)\n");
          } else {
            f = Biddy_Managed_Not(MNG,getWireById(c,c->wires[i]->inputs[0])->bdd);
          }
        }

        else {
          printf("ERROR (unimplemented type: %s)\n",c->wires[i]->type);
        }

      } else {
        printf("ERROR (wire must be a primary input or it must have some inputs)\n");
      }
    }

    c->wires[i]->bdd = f;

    Biddy_Managed_KeepFormulaProlonged(MNG,f,c->wires[i]->ttl);
    Biddy_Managed_Clean(MNG);
  }

  for (i = 0; i < c->outputcount; i++) {
    w = getWireByName(c,c->outputs[i]); /* if NULL then this output is undefined! */
    if (w) {
      f = w->bdd;
    } else {
      f = biddyZero; /* undefined outputs are represented with biddyZero */
    }
    Biddy_Managed_AddFormula(MNG,c->outputs[i],f,1);  /* TO DO: add prefix */
  }
  Biddy_Managed_Clean(MNG);

  /*
  printf("Report on primary outputs\n");
  for (i = 0; i < c->outputcount; i++) {
    f = getWireByName(c,c->outputs[i])->bdd;
    printf("%s - %d nodes / %d nodes without complement edge\n",
      c->outputs[i],
      Biddy_Managed_CountNodes(MNG,f),
      Biddy_Managed_CountNodesPlain(MNG,f)
    );
  }
  */

  /**/
  j = 0;
  for (i = 0; i < c->outputcount; i++) {
    f = getWireByName(c,c->outputs[i])->bdd;
    j = j + Biddy_Managed_CountNodes(MNG,f);
  }
  printf("The sum of BDD sizes for all outputs: %u\n",j);
  /**/

}

/**
 * Parse a signal vector
 * @param a collection of signals, a collection of tokens, a pointer to the index, a pointer to the number of counts
 */
static void
parseSignalVector(Biddy_String signal_arr[], Biddy_String token[], unsigned int *index, unsigned int *count)
{
  int v,v1,v2;
  char *sig_vector; /* array to hold tokens for the line */

  sig_vector = strtok(token[*index],":"); /* tokenize the vector to extract vector width */
  v1 = atoi(sig_vector); /* starting index for the vector */
  sig_vector = strtok(NULL, ":");
  v2 = atoi(sig_vector); /* ending index for the vector */
  (*index)++; /* get vector name from the next token */
  for (v = v2; v <= v1; v++) { /* add the vector signals to the array of signals */
    int wordsize = (int) strlen(token[*index]); /* size of the string read from the line */
    signal_arr [*count] = (Biddy_String) calloc(wordsize + 1, sizeof(char)); /* allocating memory for signal string */
    strcpy(signal_arr [*count], token[*index]); /* add the signal name to the array of signals */
    (*count)++; /* update the number of signals in the circuit */
  }
}

static void
printModuleSummary(BiddyVerilogModule *m)
{
  unsigned int i;

  printf("************** Module %s statistical results *************\n", m->name);

  printf("Number of primary inputs: %d\n", m->inputcount);
  for (i = 0; i < m->inputcount; i++)
    printf("%s ", m->inputs[i]);
  if (m->inputcount) printf("\n");

  printf("Number of primary outputs: %d\n", m->outputcount);
  for (i = 0; i < m->outputcount; i++)
    printf("%s ", m->outputs[i]);
  if ( m->outputcount) printf("\n");

  printf("Number of gates: %d\n", m->gatecount);
  for (i = 0; i < m->gatecount; i++)
    printf("%s ", m->gates[i]);
  if (m->gatecount) printf("\n");

  printf("Number of wires: %d\n", m->wirecount);
  for (i = 0; i < m->wirecount; i++)
    printf("%s ", m->wires[i]);
  if (m->wirecount) printf("\n");

  printf("Number of regs: %d\n", m->regcount);
  for (i = 0; i < m->regcount; i++)
    printf("%s ", m->regs[i]);
  if (m->regcount) printf("\n");

  printf("*************************** END **************************\n");
}

/**
 * Prints the summary of a circuit - Statistical information
 * @param the circuit object
 */
static void
printCircuitSummary(BiddyVerilogCircuit *c)
{
  unsigned int i,j;

  printf("************** Circuit %s statistical results *************\n", c->name);

  printf("Circuit size: %d\n", c->nodecount);

  printf("Number of primary inputs: %d\n", c->inputcount);
  for (i = 0; i < c->inputcount; i++)
    printf("%s ", c->inputs[i]);
  if (c->inputcount) printf("\n");

  printf("Number of primary outputs: %d\n", c->outputcount);
  for (i = 0; i < c->outputcount; i++)
    printf("%s ", c->outputs[i]);
  if (c->outputcount) printf("\n");

  i=0;
  while (i<c->wirecount && c->wires[i] != NULL) {
    printf ("c->wire[%d]->type: %s, ",i, c->wires[i]->type);
    printf ("ID: %d,  ", c->wires[i]->id);
    printf ("name: %s, ", c->nodes[c->wires[i]->id]->name);

    printf ("\nInputs (%d): ", c->wires[i]->inputcount); /* wire inputs */
    for (j=0; j<c->wires[i]->inputcount; j++)
      printf ("%d ",c->wires[i]->inputs[j]);

    printf ("\nOutputs (%d): ", c->wires[i]->outputcount); /* wire outputs */
    for (j=0; j<c->wires[i]->outputcount; j++)
      printf ("%d ",c->wires[i]->outputs[j]);

    i++;
    printf ("\n");
  }

  printf("*************************** END **************************\n");
}

/**
 * Determines if a string is a basic gate
 * @param the string to check
 * @return whether the string is a gate or not
 */
static Biddy_Boolean
isGate(Biddy_String word)
{
  int i;
  for (i = 0; i < GATESNUM; i++)
    if (strcmp(word,VerilogFileGateName[i]) == 0)
      return TRUE;
  return FALSE;
}

/**
 * Determines if a string is a vector of signals
 * @param the string to check
 * @return whether the string is a vector of signals or not
 */
static Biddy_Boolean
isSignalVector(Biddy_String word)
{
  unsigned int i;
  for (i = 0; i < strlen(word); i++)
    if (word[i] == ':')
      return TRUE;
  return FALSE;
}

/**
 * Parses a string and search for the character ';'
 * @param the source string
 * @return whether the character ';' is found or not
 */
static Biddy_Boolean
isEndOfLine(Biddy_String source)
{
  Biddy_String pchar = strchr(source, ';');
  return (pchar == NULL) ? FALSE : TRUE;
}

/**
 * Determines if a wire is already created
 * @param the circuit object, the wire name
 * @return whether the wire is already created or not
 */
static Biddy_Boolean
isDefined(BiddyVerilogCircuit *c, Biddy_String name)
{
  unsigned int i;
  for (i=0; (i < c->wirecount) && c->wires[i]; i++) {
    if (strcmp(c->nodes[c->wires[i]->id]->name, name) == 0) return TRUE;
  }
  return FALSE;
}

/**
 * Set values of a node
 * @param the node object, the type of node, the name of the node
 */
static void
buildNode(BiddyVerilogNode *n, Biddy_String type, Biddy_String name)
{
  n->type = strdup(type);
  n->name = strdup(name);
}

/**
 * Create a wire
 * @param the circuit object, the wire object, the wire type, the wire name
 */
static void
buildWire(BiddyVerilogCircuit *c, BiddyVerilogWire *w, Biddy_String type, Biddy_String name)
{
  int id;

  /* TO DO: check if wire with the same name does not already exists */

  if ((id = getNodeIdByName(c,name)) == -1) {
    printf("ERROR (buildWire): node %s does not exists!\n",name);
    exit(1);
  }
  w->id = id; /* wire ID */
  w->type = strdup(type); /* wire type */
  w->inputcount = 0; /* initial number of inputs */
  w->outputcount = 0; /* initial number of outputs */
  w->fanout = 0;
  w->ttl = 0;
  w->bdd = biddyNull;

  /* DEBUGGING */
  /*
  printf("Creating wire: id: %d, type: %s\n", w->id, w->type);
  */
}

/**
 * Get a node id by name
 * @param the circuit, the name
 * @return the node id
 */
static int
getNodeIdByName(BiddyVerilogCircuit *c, Biddy_String name)
{
  unsigned int i;
  if (strcmp(name,c->nodes[c->nodecount-1]->name) == 0) return (int) c->nodecount-1;
  for (i=0; i < c->nodecount; i++) {
    if (strcmp(name,c->nodes[i]->name) == 0) return (int) i;
  }
  return -1;
}

/**
 * Get a wire by id
 * @param the circuit, the id
 * @return the wire
 */
static BiddyVerilogWire *
getWireById(BiddyVerilogCircuit *c, int id)
{
  unsigned int i;
  for (i=0; i < c->wirecount; i++) {
    if (c->wires[i]->id == id) return c->wires[i];
  }
  return NULL;
}

/**
 * Get a wire by name
 * @param the circuit, the name
 * @return the wire
 */
static BiddyVerilogWire *
getWireByName(BiddyVerilogCircuit *c, Biddy_String name)
{
  unsigned int i;
  for (i=0; i < c->wirecount; i++) {
    if (strcmp(name, c->nodes[c->wires[i]->id]->name) == 0) return c->wires[i];
  }
  return NULL;
}

/**
 * Parses a string and removes unneccessary characters
 * @param the source string
 */
static Biddy_String
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

  return source;
}

static void
concat(char **s1, const char *s2)
{
   if (s2) {
     *s1 = (char*) realloc(*s1,strlen(*s1)+strlen(s2)+1+1);
     strcat(*s1," ");
     strcat(*s1,s2);
   }
}

static void
WriteBDD(Biddy_Manager MNG, Biddy_Edge f)
{
  Biddy_Variable v;
  Biddy_String var;

  if (Biddy_GetMark(f)) {
    printf("*");
  }
  if ((v = Biddy_GetTag(f))) {
    printf("<%s>",Biddy_Managed_GetVariableName(MNG,v));
  }

  var = Biddy_Managed_GetTopVariableName(MNG,f);
  printf("%s",var);

  if (!Biddy_IsTerminal(f)) {
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

  if (!Biddy_IsTerminal(f)) {
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

static Biddy_Boolean
WriteProduct(Biddy_Manager MNG, BiddyVarList *l, Biddy_Boolean combinationset, Biddy_Boolean first)
{
  /* DEBUGGING */
  /*
  typedef struct LIMIT {
    unsigned int part;
    unsigned int w;
  } LIMIT;
  void *data;
  */

  /* ITERATIVE VARIANT */
  /* LITERALS ARE ADDED IN REVERSE ORDERING */
  /*
  while (l) {
    if (combinationset) {
      if (!l->negative) {
        if (first) first = FALSE; else printf(",");
        printf("%s",Biddy_Managed_GetVariableName(MNG,l->v));
    } else {
      printf(" ");
      if (l->negative) {
        printf("*");
      }
      printf("%s",Biddy_Managed_GetVariableName(MNG,l->v));
    }
    l = l->next;
  }
  */

  /* RECURSIVE VARIANT */
  /* LITERALS ARE ADDED IN REVERSE ORDERING BUT THIS WILL REVERSE THE OUTPUT */
  /**/
  if (l) {
    if (combinationset) {
      first = WriteProduct(MNG,l->next,combinationset,first);
      if (!l->negative) {
        if (first) first = FALSE; else printf(" ");
        printf("%s",Biddy_Managed_GetVariableName(MNG,l->v));

        /* DEBUGGING */
        /*
        if ((data = Biddy_Managed_GetVariableData(MNG,l->v))) {
          printf("<%u>",((LIMIT *) data)->w);
        }
        */

      }
    } else {
      WriteProduct(MNG,l->next,combinationset,first);
      printf(" ");
      if (l->negative) {
        printf("*");
      }
      printf("%s",Biddy_Managed_GetVariableName(MNG,l->v));
    }
  }
  /**/

  return first;
}

static Biddy_Boolean
WriteProductx(Biddy_Manager MNG, FILE *s, BiddyVarList *l, Biddy_Boolean combinationset, Biddy_Boolean first)
{
  if (l) {
    if (combinationset) {
      first = WriteProductx(MNG,s,l->next,combinationset,first);
      if (!l->negative) {
        if (!first) fprintf(s,","); else first = FALSE;
        fprintf(s,"%s",Biddy_Managed_GetVariableName(MNG,l->v));
      }
    } else {
      WriteProductx(MNG,s,l->next,combinationset,first);
      fprintf(s," ");
      if (l->negative) {
        fprintf(s,"*");
      }
      fprintf(s,"%s",Biddy_Managed_GetVariableName(MNG,l->v));
    }
  }
  return first;
}

static void
WriteProductAlphabetic(Biddy_Manager MNG, BiddyVarList *l, Biddy_Boolean combinationset, Biddy_Boolean first)
{
  BiddyVarList *tmp;
  Biddy_String tmpname,minname,prevname;

  if (l) {
    if (combinationset) {
      prevname = NULL;
      do {
        tmp = l;
        minname = NULL;
        while (tmp) {
          if (!tmp->negative) {
            tmpname = Biddy_Managed_GetVariableName(MNG,tmp->v);
            if ((!minname || (strcmp(tmpname,minname)<0)) && (!prevname || (strcmp(tmpname,prevname)>0))) {
              minname = tmpname;
            }
          }
          tmp = tmp->next;
        }
        if (minname) {
          if (!first) printf(","); else first = FALSE;
          printf("%s",minname);
          prevname = minname;
        }
      } while (minname);
    } else {
      /* NOT IMPLEMENTED, YET */
    }
  }
}

static void
WriteSOP(Biddy_Manager MNG, Biddy_Edge f, Biddy_Variable top,
         Biddy_Boolean mark, BiddyVarList *l, unsigned int *maxsize,
         Biddy_Boolean combinationset)
{
  BiddyVarList tmp;
  Biddy_Variable v;

  /* parameter combinationset is meaningful for ZBDDs and TZBDDs, only */

  if ((!combinationset) && (*maxsize == 0)) return; /* SOP to large */

  v = BiddyV(f);

  /* DEBUGGING */
  /*
  printf("WriteSOP: top = %u (%s), v = %u (%s)\n",top,Biddy_Managed_GetVariableName(MNG,top),v,Biddy_Managed_GetVariableName(MNG,v));
  */

  assert( !(BiddyIsSmaller(BiddyV(f),top)) );

  if ((top != v) && (f != biddyZero)) {

    if (biddyManagerType == BIDDYTYPEOBDD) {
      /* NO ACTION IS NEEDED */
    } else if (biddyManagerType == BIDDYTYPEOBDDC) {
      /* NO ACTION IS NEEDED */
    } else if (biddyManagerType == BIDDYTYPEZBDD) {
      tmp.next = l;
      tmp.v = top;
      tmp.negative = TRUE; /* negative literal */
      WriteSOP(MNG,f,biddyVariableTable.table[top].next,mark,&tmp,maxsize,combinationset);
    } else if (biddyManagerType == BIDDYTYPEZBDDC) {
      tmp.next = l;
      tmp.v = top;
      tmp.negative = TRUE; /* negative literal */
      WriteSOP(MNG,f,biddyVariableTable.table[top].next,mark,&tmp,maxsize,combinationset);
    } else if (biddyManagerType == BIDDYTYPETZBDD) {
      tmp.next = l;
      tmp.v = top;
      tmp.negative = TRUE; /* negative literal */
      WriteSOP(MNG,f,biddyVariableTable.table[top].next,mark,&tmp,maxsize,combinationset);
      if (combinationset && Biddy_Managed_IsSmaller(MNG,top,Biddy_GetTag(f))) {
        tmp.negative = FALSE; /* positive literal */
        WriteSOP(MNG,f,biddyVariableTable.table[top].next,mark,&tmp,maxsize,combinationset);
      }
    } else if (biddyManagerType == BIDDYTYPETZBDDC)
    {
      fprintf(stderr,"WriteSOP: this BDD type is not supported, yet!\n");
      return;
    } else if ((biddyManagerType == BIDDYTYPEOFDD) || (biddyManagerType == BIDDYTYPEOFDDC) ||
                (biddyManagerType == BIDDYTYPEZFDD) || (biddyManagerType == BIDDYTYPEZFDDC) ||
                (biddyManagerType == BIDDYTYPETZFDD) || (biddyManagerType == BIDDYTYPETZFDDC))
    {
      fprintf(stderr,"WriteSOP: this BDD type is not supported, yet!\n");
      return;
    } else {
      fprintf(stderr,"WriteSOP: Unsupported BDD type!\n");
      return;
    }

  } else if (v == 0) {

    if (combinationset) {
      if (!mark) {
        if (l) {
          /* printf("{"); */
          /* WriteProductAlphabetic(MNG,l,combinationset,TRUE); */
          WriteProduct(MNG,l,combinationset,TRUE);
          /* printf("}"); */
          printf(" ; ");
          (*maxsize)--;
        } else {
          /* printf("{}"); */
          printf(",");
        }
      } else {
        if (!l) {
          printf("EMPTY");
        }
      }
    } else {
      if (!mark) {
        printf("  + ");
        if (l) {
          WriteProduct(MNG,l,combinationset,TRUE);
          printf("\n");
          (*maxsize)--;
        } else {
          printf(" 1");
        }
      } else {
        if (!l) {
          printf("  +  0");
        }
      }
    }

  } else {

    tmp.next = l;
    tmp.v = v;

    if ((biddyManagerType == BIDDYTYPEOBDDC) || (biddyManagerType == BIDDYTYPEOBDD)) {
      tmp.negative = TRUE; /* left successor */
      WriteSOP(MNG,BiddyE(f),BiddyV(BiddyE(f)),(mark ^ Biddy_GetMark(BiddyE(f))),&tmp,maxsize,combinationset);
    }
    else if ((biddyManagerType == BIDDYTYPEZBDDC) || (biddyManagerType == BIDDYTYPEZBDD)) {
      if (combinationset) {
        tmp.negative = FALSE; /* right successor for combination sets */
        WriteSOP(MNG,BiddyT(f),biddyVariableTable.table[v].next,Biddy_GetMark(BiddyT(f)),&tmp,maxsize,combinationset);
      } else {
        tmp.negative = TRUE; /* left successor for Boolean function */
        WriteSOP(MNG,BiddyE(f),biddyVariableTable.table[v].next,(mark ^ Biddy_GetMark(BiddyE(f))),&tmp,maxsize,combinationset);
      }
    }
    else if ((biddyManagerType == BIDDYTYPETZBDDC) || (biddyManagerType == BIDDYTYPETZBDD)) {
      if (combinationset) {
        tmp.negative = FALSE; /* right successor for combination sets */
        WriteSOP(MNG,BiddyT(f),biddyVariableTable.table[v].next,Biddy_GetMark(BiddyT(f)),&tmp,maxsize,combinationset);
      } else {
        tmp.negative = TRUE; /* left successor for Boolean function */
        WriteSOP(MNG,BiddyE(f),Biddy_GetTag(BiddyE(f)),(mark ^ Biddy_GetMark(BiddyE(f))),&tmp,maxsize,combinationset);
      }
    }

    if ((biddyManagerType == BIDDYTYPEOBDDC) || (biddyManagerType == BIDDYTYPEOBDD)) {
      tmp.negative = FALSE; /* right successor */
      WriteSOP(MNG,BiddyT(f),BiddyV(BiddyT(f)),(mark ^ Biddy_GetMark(BiddyT(f))),&tmp,maxsize,combinationset);
    }
    else if ((biddyManagerType == BIDDYTYPEZBDDC) || (biddyManagerType == BIDDYTYPEZBDD)) {
      if (combinationset) {
        tmp.negative = TRUE; /* left successor for combination sets */
        WriteSOP(MNG,BiddyE(f),biddyVariableTable.table[v].next,(mark ^ Biddy_GetMark(BiddyE(f))),&tmp,maxsize,combinationset);
      } else {
        tmp.negative = FALSE; /* right successor for Boolean function */
        WriteSOP(MNG,BiddyT(f),biddyVariableTable.table[v].next,Biddy_GetMark(BiddyT(f)),&tmp,maxsize,combinationset);
      }
    }
    else if ((biddyManagerType == BIDDYTYPETZBDDC) || (biddyManagerType == BIDDYTYPETZBDD)) {
      if (combinationset) {
        tmp.negative = TRUE; /* left successor for combination sets */
        WriteSOP(MNG,BiddyE(f),biddyVariableTable.table[v].next,(mark ^ Biddy_GetMark(BiddyE(f))),&tmp,maxsize,combinationset);
      } else {
        tmp.negative = FALSE; /* right successor for Boolean function */
        WriteSOP(MNG,BiddyT(f),Biddy_GetTag(BiddyT(f)),Biddy_GetMark(BiddyT(f)),&tmp,maxsize,combinationset);
      }
    }

  }
}

static void
WriteSOPx(Biddy_Manager MNG, FILE *s, Biddy_Edge f, Biddy_Variable top,
         Biddy_Boolean mark, BiddyVarList *l, unsigned int *maxsize,
         Biddy_Boolean combinationset)
{
  BiddyVarList tmp;
  Biddy_Variable v;

  if ((!combinationset) && (*maxsize == 0)) return; /* SOP to large */

  v = BiddyV(f);

  /* DEBUGGING */
  /*
  printf("WriteSOPx: top = %u (%s), v = %u (%s)\n",top,Biddy_Managed_GetVariableName(MNG,top),v,Biddy_Managed_GetVariableName(MNG,v));
  */

  assert( !(BiddyIsSmaller(BiddyV(f),top)) );

  if ((top != v) && (f != biddyZero)) {

    if (biddyManagerType == BIDDYTYPEOBDD) {
      /* IMPLEMENTED */
    } else if (biddyManagerType == BIDDYTYPEOBDDC) {
      /* IMPLEMENTED */
    } else if (biddyManagerType == BIDDYTYPEZBDD) {
      /* IMPLEMENTED */
      tmp.next = l;
      tmp.v = top;
      tmp.negative = TRUE; /* left successor */
      WriteSOPx(MNG,s,f,biddyVariableTable.table[top].next,mark,&tmp,maxsize,combinationset);
    } else if (biddyManagerType == BIDDYTYPEZBDDC) {
      /* IMPLEMENTED */
      tmp.next = l;
      tmp.v = top;
      tmp.negative = TRUE; /* left successor */
      WriteSOPx(MNG,s,f,biddyVariableTable.table[top].next,mark,&tmp,maxsize,combinationset);
    } else if (biddyManagerType == BIDDYTYPETZBDD) {
      /* IMPLEMENTED */
      tmp.next = l;
      tmp.v = top;
      tmp.negative = TRUE; /* left successor */
      WriteSOPx(MNG,s,f,biddyVariableTable.table[top].next,mark,&tmp,maxsize,combinationset);
    } else if (biddyManagerType == BIDDYTYPETZBDDC)
    {
      fprintf(stderr,"WriteSOPx: this BDD type is not supported, yet!\n");
      return;
    } else if ((biddyManagerType == BIDDYTYPEOFDD) || (biddyManagerType == BIDDYTYPEOFDDC) ||
                (biddyManagerType == BIDDYTYPEZFDD) || (biddyManagerType == BIDDYTYPEZFDDC) ||
                (biddyManagerType == BIDDYTYPETZFDD) || (biddyManagerType == BIDDYTYPETZFDDC))
    {
      fprintf(stderr,"WriteSOPx: this BDD type is not supported, yet!\n");
      return;
    } else {
      fprintf(stderr,"WriteSOPx: Unsupported BDD type!\n");
      return;
    }

  } else if (v == 0) {

    if (combinationset) {
    } else {
      if (!mark) {
        fprintf(s,"  + ");
        if (l) {
          WriteProductx(MNG,s,l,combinationset,TRUE);
          fprintf(s,"\n");
          (*maxsize)--;
        } else {
          fprintf(s," 1");
        }
      } else {
        if (!l) {
          fprintf(s,"  +  0");
        }
      }
    }

  } else {

    tmp.next = l;
    tmp.v = v;
    tmp.negative = TRUE; /* left successor */

    if ((biddyManagerType == BIDDYTYPEOBDDC) || (biddyManagerType == BIDDYTYPEOBDD)) {
      WriteSOPx(MNG,s,BiddyE(f),BiddyV(BiddyE(f)),(mark ^ Biddy_GetMark(BiddyE(f))),&tmp,maxsize,combinationset);
    }
    else if ((biddyManagerType == BIDDYTYPEZBDDC) || (biddyManagerType == BIDDYTYPEZBDD)) {
      WriteSOPx(MNG,s,BiddyE(f),biddyVariableTable.table[v].next,(mark ^ Biddy_GetMark(BiddyE(f))),&tmp,maxsize,combinationset);
    }
    else if ((biddyManagerType == BIDDYTYPETZBDDC) || (biddyManagerType == BIDDYTYPETZBDD)) {
      WriteSOPx(MNG,s,BiddyE(f),Biddy_GetTag(BiddyE(f)),(mark ^ Biddy_GetMark(BiddyE(f))),&tmp,maxsize,combinationset);
    }

    tmp.negative = FALSE; /* right successor */

    if ((biddyManagerType == BIDDYTYPEOBDDC) || (biddyManagerType == BIDDYTYPEOBDD)) {
      WriteSOPx(MNG,s,BiddyT(f),BiddyV(BiddyT(f)),(mark ^ Biddy_GetMark(BiddyT(f))),&tmp,maxsize,combinationset);
    }
    else if ((biddyManagerType == BIDDYTYPEZBDDC) || (biddyManagerType == BIDDYTYPEZBDD)) {
      WriteSOPx(MNG,s,BiddyT(f),biddyVariableTable.table[v].next,Biddy_GetMark(BiddyT(f)),&tmp,maxsize,combinationset);
    }
    else if ((biddyManagerType == BIDDYTYPETZBDDC) || (biddyManagerType == BIDDYTYPETZBDD)) {
      WriteSOPx(MNG,s,BiddyT(f),Biddy_GetTag(BiddyT(f)),Biddy_GetMark(BiddyT(f)),&tmp,maxsize,combinationset);
    }

  }
}

static unsigned int
enumerateNodes(Biddy_Manager MNG, Biddy_Edge f, unsigned int n)
{
  if (!Biddy_Managed_IsSelected(MNG,f)) {
    if (Biddy_IsTerminal(f)) {
    } else {
      Biddy_Managed_SelectNode(MNG,f);
      n++;
      BiddySetEnumerator(f,n);

      /* one or both are terminal node */
      /* every instance of terminal node is enumerated */
      if (Biddy_IsTerminal(BiddyE(f)) || Biddy_IsTerminal(BiddyT(f))) {

        if (((biddyManagerType == BIDDYTYPEOBDD) ||
             (biddyManagerType == BIDDYTYPEZBDD) ||
             (biddyManagerType == BIDDYTYPETZBDD)
            ) && (Biddy_GetMark(BiddyE(f)) != Biddy_GetMark(BiddyT(f))))
        {
          /* two terminal nodes only if complemented edges are not used and they are different */
          if (Biddy_IsTerminal(BiddyE(f))) {
            n++;
          }
          if (Biddy_IsTerminal(BiddyT(f))) {
            n++;
          }
        }
        else {
          /* only one terminal node */
          n++;
        }

      }

      if (!Biddy_IsTerminal(BiddyE(f))) {
        n = enumerateNodes(MNG,BiddyE(f),n);
      }
      if (!Biddy_IsTerminal(BiddyT(f))) {
        n = enumerateNodes(MNG,BiddyT(f),n);
      }
    }
  }
  return n;
}

static void
WriteDotNodes(Biddy_Manager MNG, FILE *dotfile, Biddy_Edge f, int id, Biddy_Boolean cudd)
{
  BiddyLocalInfo *li;
  Biddy_String name,hash;
  Biddy_Variable v;
  BiddyLocalInfo *li2;

  if (Biddy_IsTerminal(f)) {
    if (cudd) {

      /* VARIANT 1 - THIS SHOULD BE COMBINED WITH VARIANT 1 IN WriteDotEdges */
      /*
      fprintf(dotfile,"{ rank = same; \"CONST NODES\";\n");
      fprintf(dotfile,"{ node [shape = box label = \"1\"]; ");
      fprintf(dotfile,"\"%p\";\n",biddyOne);
      */

      /* VARIANT 2 - THIS SHOULD BE COMBINED WITH VARIANT 2 IN WriteDotEdges */
      /**/
      fprintf(dotfile,"{ \"CONST NODES\";\n");
      fprintf(dotfile,"{ node [shape = none label = \"1\"];\n");
      fprintf(dotfile,"\"1\";\n");
      /**/

      fprintf(dotfile,"}\n}\n");
    } else {
      if ((biddyManagerType == BIDDYTYPEOBDD) ||
          (biddyManagerType == BIDDYTYPEZBDD) ||
          (biddyManagerType == BIDDYTYPETZBDD))
      {
        if (f == biddyZero) {
          fprintf(dotfile, "  node [shape = none, label = \"0\"] %u;\n",1);
        } else {
          fprintf(dotfile, "  node [shape = none, label = \"1\"] %u;\n",1);
        }
      }
      else {
        fprintf(dotfile, "  node [shape = none, label = \"1\"] %u;\n",1);
      }
    }
    return;
  }
  if (cudd) {
    Biddy_Managed_ResetVariablesValue(MNG);
    li = (BiddyLocalInfo *)(BiddyN(f)->list);
    while (li->back) {
      v = BiddyV(li->back);
      if (biddyVariableTable.table[v].value == biddyZero) {
        biddyVariableTable.table[v].value = biddyOne;
        name = strdup(Biddy_Managed_GetVariableName(MNG,v));
        while ((hash = strchr(name,'#'))) hash[0] = '_';
        fprintf(dotfile,"{ rank = same; ");
        fprintf(dotfile,"\" %s \";\n",name);
        free(name);
        li2 = li;
        while (li2->back) {
          if (BiddyV(li2->back) == v) {
            fprintf(dotfile,"\"%p\";\n",li2->back);
          }
          li2 = &li2[1]; /* next field in the array */
        }
        fprintf(dotfile,"}\n");
      }
      li = &li[1]; /* next field in the array */
    }
    Biddy_Managed_ResetVariablesValue(MNG);

    /* VARIANT 1 - THIS SHOULD BE COMBINED WITH VARIANT 1 IN WriteDotEdges */
    /*
    fprintf(dotfile,"{ rank = same; \"CONST NODES\";\n");
    fprintf(dotfile,"{ node [shape = box label = \"1\"]; ");
    fprintf(dotfile,"\"%p\";\n",biddyOne);
    */

    /* VARIANT 2 - THIS SHOULD BE COMBINED WITH VARIANT 2 IN WriteDotEdges */
    /**/
    fprintf(dotfile,"{ \"CONST NODES\";\n");
    fprintf(dotfile,"{ node [shape = none label = \"1\"];\n");
    li = (BiddyLocalInfo *)(BiddyN(f)->list);
    while (li->back) {
      if (Biddy_IsTerminal(BiddyE(li->back)) || Biddy_IsTerminal(BiddyT(li->back))) {
        fprintf(dotfile,"\"%u\";\n",li->data.enumerator+1);
      }
      li = &li[1];
    }
    /**/

    fprintf(dotfile,"}\n}\n");
  } else {
    li = (BiddyLocalInfo *)(BiddyN(f)->list);
    while (li->back) {
      if (id == -1) {
        name = getname(MNG,li->back);
      } else {
        name = getshortname(MNG,li->back,id);
      }
      while ((hash = strchr(name,'#'))) hash[0] = '_';
      fprintf(dotfile,"  node [shape = circle, label = \"%s\"] %u;\n",name,li->data.enumerator);
      free(name);
      if ((biddyManagerType == BIDDYTYPEOBDD) ||
          (biddyManagerType == BIDDYTYPEZBDD) ||
          (biddyManagerType == BIDDYTYPETZBDD))
      {
        if (BiddyE(li->back) == biddyZero) {
          fprintf(dotfile,"  node [shape = none, label = \"0\"] %u;\n",li->data.enumerator+1);
        }
        else if (Biddy_IsTerminal(BiddyE(li->back))) {
          fprintf(dotfile,"  node [shape = none, label = \"1\"] %u;\n",li->data.enumerator+1);
        }
        if (BiddyT(li->back) == biddyZero) {
          if (Biddy_IsTerminal(BiddyE(li->back)) && (Biddy_GetMark(BiddyE(li->back)) != Biddy_GetMark(BiddyT(li->back)))) {
            fprintf(dotfile,"  node [shape = none, label = \"0\"] %u;\n",li->data.enumerator+2);
          } else {
            fprintf(dotfile,"  node [shape = none, label = \"0\"] %u;\n",li->data.enumerator+1);
          }
        }
        else if (Biddy_IsTerminal(BiddyT(li->back))) {
          if (Biddy_IsTerminal(BiddyE(li->back)) && (Biddy_GetMark(BiddyE(li->back)) != Biddy_GetMark(BiddyT(li->back)))) {
            fprintf(dotfile,"  node [shape = none, label = \"1\"] %u;\n",li->data.enumerator+2);
          } else {
            fprintf(dotfile,"  node [shape = none, label = \"1\"] %u;\n",li->data.enumerator+1);
          }
        }
      }
      else {
        if (Biddy_IsTerminal(BiddyE(li->back)) || Biddy_IsTerminal(BiddyT(li->back))) {
          fprintf(dotfile,"  node [shape = none, label = \"1\"] %u;\n",li->data.enumerator+1);
        }
      }
      li = &li[1]; /* next field in the array */
    }
  }
}

static void
WriteDotEdges(Biddy_Manager MNG, FILE *dotfile, Biddy_Edge f, Biddy_Boolean cudd)
{
  unsigned int n1,n2;
  Biddy_Variable tag;

  if (!Biddy_IsTerminal(f) && !Biddy_Managed_IsSelected(MNG,f)) {
    Biddy_Managed_SelectNode(MNG,f);
    n1 = BiddyGetEnumerator(f);

    if (Biddy_IsTerminal(BiddyE(f))) {
      n2 = n1 + 1;
    } else {
      n2 = BiddyGetEnumerator(BiddyE(f));
    }
    if (Biddy_GetMark(BiddyE(f))) {
      if ((BiddyE(f) != biddyZero) && (tag = Biddy_GetTag(BiddyE(f)))) {
        if (cudd) {

          /* VARIANT 1 - THIS SHOULD BE COMBINED WITH VARIANT 1 IN WriteDotNodes */
          /*
          fprintf(dotfile,"\"%p\" -> \"%p\" [style = dotted label=\"%s\"];\n",BiddyP(f),BiddyP(BiddyE(f)),Biddy_Managed_GetVariableName(MNG,tag));
          */

          /* VARIANT 2 - THIS SHOULD BE COMBINED WITH VARIANT 2 IN WriteDotNodes */
          /**/
          if (Biddy_IsTerminal(BiddyE(f))) {
            fprintf(dotfile,"\"%p\" -> \"%u\" [style=dotted label=\"%s\"];\n",BiddyP(f),n2,Biddy_Managed_GetVariableName(MNG,tag));
          } else {
            fprintf(dotfile,"\"%p\" -> \"%p\" [style=dotted label=\"%s\"];\n",BiddyP(f),BiddyP(BiddyE(f)),Biddy_Managed_GetVariableName(MNG,tag));
          }
          /**/

        } else {
#ifdef LEGACY_DOT
          fprintf(dotfile,"  %u -> %u [style=dotted label=\"%s\"];\n",n1,n2,Biddy_Managed_GetVariableName(MNG,tag));
#else
          fprintf(dotfile,"  %u -> %u [arrowtail=\"odot\" arrowhead=\"dot\" label=\"%s\"];\n",n1,n2,Biddy_Managed_GetVariableName(MNG,tag));
#endif
        }
      } else {
        if (cudd) {

          /* VARIANT 1 - THIS SHOULD BE COMBINED WITH VARIANT 1 IN WriteDotNodes */
          /*
          fprintf(dotfile,"\"%p\" -> \"%p\" [style = dotted];\n",BiddyP(f),BiddyP(BiddyE(f)));
          */

          /* VARIANT 2 - THIS SHOULD BE COMBINED WITH VARIANT 2 IN WriteDotNodes */
          /**/
          if (Biddy_IsTerminal(BiddyE(f))) {
            fprintf(dotfile,"\"%p\" -> \"%u\" [style=dotted];\n",BiddyP(f),n2);
          } else {
            fprintf(dotfile,"\"%p\" -> \"%p\" [style=dotted];\n",BiddyP(f),BiddyP(BiddyE(f)));
          }
          /**/

        } else {
          if ((biddyManagerType == BIDDYTYPEOBDD) ||
              (biddyManagerType == BIDDYTYPEZBDD) ||
              (biddyManagerType == BIDDYTYPETZBDD))
          {
#ifdef LEGACY_DOT
            fprintf(dotfile,"  %u -> %u [style=dashed];\n",n1,n2);
#else
            fprintf(dotfile,"  %u -> %u [arrowtail=\"odot\"];\n",n1,n2);
#endif
          }
          else {
#ifdef LEGACY_DOT
            fprintf(dotfile,"  %u -> %u [style=dotted];\n",n1,n2);
#else
            fprintf(dotfile,"  %u -> %u [arrowtail=\"odot\" arrowhead=\"dot\"];\n",n1,n2);
#endif
          }
        }
      }
    } else {
      if ((BiddyE(f) != biddyZero) && (tag = Biddy_GetTag(BiddyE(f)))) {
        if (cudd) {

          /* VARIANT 1 - THIS SHOULD BE COMBINED WITH VARIANT 1 IN WriteDotNodes */
          /*
          fprintf(dotfile,"\"%p\" -> \"%p\" [style = dashed label=\"%s\"];\n",BiddyP(f),BiddyP(BiddyE(f)),Biddy_Managed_GetVariableName(MNG,tag));
          */

          /* VARIANT 2 - THIS SHOULD BE COMBINED WITH VARIANT 2 IN WriteDotNodes */
          /**/
          if (Biddy_IsTerminal(BiddyE(f))) {
            fprintf(dotfile,"\"%p\" -> \"%u\" [style=dashed label=\"%s\"];\n",BiddyP(f),n2,Biddy_Managed_GetVariableName(MNG,tag));
          } else {
            fprintf(dotfile,"\"%p\" -> \"%p\" [style=dashed label=\"%s\"];\n",BiddyP(f),BiddyP(BiddyE(f)),Biddy_Managed_GetVariableName(MNG,tag));
          }
          /**/

        } else {
#ifdef LEGACY_DOT
          fprintf(dotfile,"  %u -> %u [style=dashed label=\"%s\"];\n",n1,n2,Biddy_Managed_GetVariableName(MNG,tag));
#else
          fprintf(dotfile,"  %u -> %u [arrowtail=\"odot\" label=\"%s\"];\n",n1,n2,Biddy_Managed_GetVariableName(MNG,tag));
#endif
        }
      } else {
        if (cudd) {

          /* VARIANT 1 - THIS SHOULD BE COMBINED WITH VARIANT 1 IN WriteDotNodes */
          /*
          fprintf(dotfile,"\"%p\" -> \"%p\" [style = dashed];\n",BiddyP(f),BiddyP(BiddyE(f)));
          */

          /* VARIANT 2 - THIS SHOULD BE COMBINED WITH VARIANT 2 IN WriteDotNodes */
          /**/
          if (Biddy_IsTerminal(BiddyE(f))) {
            fprintf(dotfile,"\"%p\" -> \"%u\" [style=dashed];\n",BiddyP(f),n2);
          } else {
            fprintf(dotfile,"\"%p\" -> \"%p\" [style=dashed];\n",BiddyP(f),BiddyP(BiddyE(f)));
          }
          /**/

        } else {
#ifdef LEGACY_DOT
        fprintf(dotfile,"  %u -> %u [style=dashed];\n",n1,n2);
#else
        fprintf(dotfile,"  %u -> %u [arrowtail=\"odot\"];\n",n1,n2);
#endif
        }
      }
    }

    if (Biddy_IsTerminal(BiddyT(f))) {
      if (((biddyManagerType == BIDDYTYPEOBDD) ||
           (biddyManagerType == BIDDYTYPEZBDD) ||
           (biddyManagerType == BIDDYTYPETZBDD))
          && (Biddy_IsTerminal(BiddyE(f)))
          && (Biddy_GetMark(BiddyE(f)) != Biddy_GetMark(BiddyT(f))))
      {
        n2 = n1 + 2;
      }
      else {
        n2 = n1 + 1;
      }
    } else {
      n2 = BiddyGetEnumerator(BiddyT(f));
    }
    if (Biddy_GetMark(BiddyT(f))) {
      if ((BiddyT(f) != biddyZero) && (tag = Biddy_GetTag(BiddyT(f)))) {
        if (cudd) {

          /* VARIANT 1 - THIS SHOULD BE COMBINED WITH VARIANT 1 IN WriteDotNodes */
          /*
          fprintf(dotfile,"\"%p\" -> \"%p\" [style = bold label=\"%s\"];\n",BiddyP(f),BiddyP(BiddyT(f)),Biddy_Managed_GetVariableName(MNG,tag));
          */

          /* VARIANT 2 - THIS SHOULD BE COMBINED WITH VARIANT 2 IN WriteDotNodes */
          /**/
          if (Biddy_IsTerminal(BiddyT(f))) {
            fprintf(dotfile,"\"%p\" -> \"%u\" [style=bold label=\"%s\"];\n",BiddyP(f),n2,Biddy_Managed_GetVariableName(MNG,tag));
          } else {
            fprintf(dotfile,"\"%p\" -> \"%p\" [style=bold label=\"%s\"];\n",BiddyP(f),BiddyP(BiddyT(f)),Biddy_Managed_GetVariableName(MNG,tag));
          }
          /**/

        } else {
#ifdef LEGACY_DOT
          fprintf(dotfile,"  %d -> %d [style=bold label=\"%s\"];\n",n1,n2,Biddy_Managed_GetVariableName(MNG,tag));
#else
          fprintf(dotfile,"  %d -> %d [arrowtail=\"invempty\" arrowhead=\"dot\" label=\"%s\"];\n",n1,n2,Biddy_Managed_GetVariableName(MNG,tag));
#endif
        }
      } else {
        if (cudd) {

          /* VARIANT 1 - THIS SHOULD BE COMBINED WITH VARIANT 1 IN WriteDotNodes */
          /*
          fprintf(dotfile,"\"%p\" -> \"%p\" [style = bold];\n",BiddyP(f),BiddyP(BiddyT(f)));
          */

          /* VARIANT 2 - THIS SHOULD BE COMBINED WITH VARIANT 2 IN WriteDotNodes */
          /**/
          if (Biddy_IsTerminal(BiddyT(f))) {
            fprintf(dotfile,"\"%p\" -> \"%u\" [style=bold];\n",BiddyP(f),n2);
          } else {
            fprintf(dotfile,"\"%p\" -> \"%p\" [style=bold];\n",BiddyP(f),BiddyP(BiddyT(f)));
          }
          /**/

        } else {
          if ((biddyManagerType == BIDDYTYPEOBDD) ||
              (biddyManagerType == BIDDYTYPEZBDD) ||
              (biddyManagerType == BIDDYTYPETZBDD))
          {
#ifdef LEGACY_DOT
            fprintf(dotfile,"  %d -> %d;\n",n1,n2);
#else
            fprintf(dotfile,"  %d -> %d [arrowtail=\"invempty\"];\n",n1,n2);
#endif
          }
          else {
#ifdef LEGACY_DOT
            fprintf(dotfile,"  %d -> %d [style=bold];\n",n1,n2);
#else
            fprintf(dotfile,"  %d -> %d [arrowtail=\"invempty\" arrowhead=\"dot\"];\n",n1,n2);
#endif
          }
        }
      }
    } else {
      if ((BiddyT(f) != biddyZero) && (tag = Biddy_GetTag(BiddyT(f)))) {
        if (cudd) {

          /* VARIANT 1 - THIS SHOULD BE COMBINED WITH VARIANT 1 IN WriteDotNodes */
          /*
          fprintf(dotfile,"\"%p\" -> \"%p\" [label=\"%s\"];\n",BiddyP(f),BiddyP(BiddyT(f)),Biddy_Managed_GetVariableName(MNG,tag));
          */

          /* VARIANT 2 - THIS SHOULD BE COMBINED WITH VARIANT 2 IN WriteDotNodes */
          /**/
          if (Biddy_IsTerminal(BiddyT(f))) {
            fprintf(dotfile,"\"%p\" -> \"%u\" [label=\"%s\"];\n",BiddyP(f),n2,Biddy_Managed_GetVariableName(MNG,tag));
          } else {
            fprintf(dotfile,"\"%p\" -> \"%p\" [label=\"%s\"];\n",BiddyP(f),BiddyP(BiddyT(f)),Biddy_Managed_GetVariableName(MNG,tag));
          }
          /**/

        } else {
#ifdef LEGACY_DOT
          fprintf(dotfile,"  %d -> %d [label=\"%s\"];\n",n1,n2,Biddy_Managed_GetVariableName(MNG,tag));
#else
          fprintf(dotfile,"  %d -> %d [arrowtail=\"invempty\" label=\"%s\"];\n",n1,n2,Biddy_Managed_GetVariableName(MNG,tag));
#endif
        }
      } else {
        if (cudd) {

        /* VARIANT 1 - THIS SHOULD BE COMBINED WITH VARIANT 1 IN WriteDotNodes */
        /*
        fprintf(dotfile,"\"%p\" -> \"%p\";\n",BiddyP(f),BiddyP(BiddyT(f)));
        */
        /* VARIANT 2 - THIS SHOULD BE COMBINED WITH VARIANT 2 IN WriteDotNodes */

        /**/
        if (Biddy_IsTerminal(BiddyT(f))) {
          fprintf(dotfile,"\"%p\" -> \"%u\";\n",BiddyP(f),n2);
        } else {
          fprintf(dotfile,"\"%p\" -> \"%p\";\n",BiddyP(f),BiddyP(BiddyT(f)));
        }
        /**/

        } else {
#ifdef LEGACY_DOT
          fprintf(dotfile,"  %d -> %d [style=solid];\n",n1,n2);
#else
          fprintf(dotfile,"  %d -> %d [arrowtail=\"invempty\"];\n",n1,n2);
#endif
        }
      }
    }

    WriteDotEdges(MNG,dotfile,BiddyE(f),cudd);
    WriteDotEdges(MNG,dotfile,BiddyT(f),cudd);
  }
}

static void
WriteBddviewConnections(Biddy_Manager MNG, FILE *funfile, Biddy_Edge f)
{
  int n1,n2;
  Biddy_Variable tag1,tag2;

  if (!Biddy_IsTerminal(f) && !Biddy_Managed_IsSelected(MNG,f)) {
    Biddy_Managed_SelectNode(MNG,f);
    n1 = BiddyGetEnumerator(f);

    /* if successors are equal then double line is possible */
    if ((BiddyP(BiddyE(f)) != BiddyP(BiddyT(f))) ||
         (((biddyManagerType == BIDDYTYPEOBDD) ||
          (biddyManagerType == BIDDYTYPEZBDD) ||
          (biddyManagerType == BIDDYTYPETZBDD))
          &&
          (Biddy_GetMark((BiddyE(f))) != Biddy_GetMark((BiddyT(f)))))
       )
    {

      unsigned int num;

      /* SINGLE LINE */

      num = 0;
      if (Biddy_IsTerminal(BiddyE(f))) {
        n2 = n1 + 1;
        num = 1;
      } else {
        n2 = BiddyGetEnumerator(BiddyE(f));
      }
      fprintf(funfile,"connect %d %d ",n1,n2);
      if ((biddyManagerType == BIDDYTYPEOBDD) ||
          (biddyManagerType == BIDDYTYPEZBDD) ||
          (biddyManagerType == BIDDYTYPETZBDD))
      {
        fprintf(funfile,"l");
      }
      else {
        if (Biddy_GetMark((BiddyE(f)))) {
          fprintf(funfile,"li");
        } else {
          fprintf(funfile,"l");
        }
      }
      if ((biddyManagerType == BIDDYTYPETZBDDC) || (biddyManagerType == BIDDYTYPETZBDD))
      {
        if (BiddyE(f) == biddyZero) {
          fprintf(funfile," 0\n");
        } else {
          tag1 = Biddy_GetTag(BiddyE(f));
          fprintf(funfile," %s\n",Biddy_Managed_GetVariableName(MNG,tag1));
        }
      } else {
        fprintf(funfile,"\n");
      }

      if (Biddy_IsTerminal(BiddyT(f))) {
        n2 = n1 + num + 1;
      } else {
        n2 = BiddyGetEnumerator(BiddyT(f));
      }
      fprintf(funfile,"connect %d %d ",n1,n2);
      if ((biddyManagerType == BIDDYTYPEOBDD) ||
          (biddyManagerType == BIDDYTYPEZBDD) ||
          (biddyManagerType == BIDDYTYPETZBDD))
      {
        fprintf(funfile,"r");
      }
      else {
        if (Biddy_GetMark((BiddyT(f)))) {
          fprintf(funfile,"ri");
        } else {
          fprintf(funfile,"r");
        }
      }
      if ((biddyManagerType == BIDDYTYPETZBDDC) || (biddyManagerType == BIDDYTYPETZBDD))
      {
        if (BiddyT(f) == biddyZero) {
          fprintf(funfile," 0\n");
        } else {
          tag2 = Biddy_GetTag(BiddyT(f));
          fprintf(funfile," %s\n",Biddy_Managed_GetVariableName(MNG,tag2));
        }
      } else {
        fprintf(funfile,"\n");
      }

    } else {

      /* DOUBLE LINE */

      if (Biddy_IsTerminal(BiddyE(f))) {
        n2 = n1 + 1;
      } else {
        n2 = BiddyGetEnumerator(BiddyE(f));
      }

      fprintf(funfile,"connect %d %d ",n1,n2);
      if (Biddy_GetMark((BiddyE(f)))) {
        if (Biddy_GetMark((BiddyT(f)))) {
          fprintf(funfile,"ei");
        } else {
          fprintf(funfile,"d");
        }
      } else {
        if (Biddy_GetMark((BiddyT(f)))) {
          fprintf(funfile,"di");
        } else {
          fprintf(funfile,"e");
        }
      }
      if ((biddyManagerType == BIDDYTYPETZBDDC) || (biddyManagerType == BIDDYTYPETZBDD))
      {
        tag1 = Biddy_GetTag(BiddyE(f));
        tag2 = Biddy_GetTag(BiddyT(f));
        fprintf(funfile," %s %s\n",Biddy_Managed_GetVariableName(MNG,tag1),Biddy_Managed_GetVariableName(MNG,tag2));
      } else {
        fprintf(funfile,"\n");
      }

    }

    WriteBddviewConnections(MNG,funfile,BiddyE(f));
    WriteBddviewConnections(MNG,funfile,BiddyT(f));
  }
}

static Biddy_String
getname(Biddy_Manager MNG, void *p)
{
  unsigned int i;
  Biddy_String newname;

  if ((p == biddyZero) &&
      ((biddyManagerType == BIDDYTYPEOBDD) ||
      (biddyManagerType == BIDDYTYPEZBDD) ||
      (biddyManagerType == BIDDYTYPETZBDD)))
  {
    newname = strdup("0");
    return (newname);
  }

  newname = strdup(Biddy_Managed_GetTopVariableName(MNG,(Biddy_Edge) p));
  for (i=0; i<strlen(newname); ++i) {
    if (newname[i] == '<') newname[i] = '_';
    if (newname[i] == '>') newname[i] = 0;
  }

  return (newname);
}

static Biddy_String
getshortname(Biddy_Manager MNG, void *p, int n)
{
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

static void
reportOrdering()
{
  Biddy_Variable var;

  printf("Variable ordering:\n");
  var = Biddy_GetLowestVariable();
  while (var != 0) {
    printf("(%s)",Biddy_GetVariableName(var));
    var = Biddy_GetNextVariable(var);
  }
  printf("\n");
}

/***************************************************************************//*!
\file biddyInOut.c
\brief File biddyInOut.c contains various parsers and generators.

### Description

    PackageName [Biddy]
    Synopsis    [Biddy provides data structures and algorithms for the
                 representation and manipulation of Boolean functions with
                 ROBDDs. A hash table is used for quick search of nodes.
                 Complement edges decreases the number of nodes. An automatic
                 garbage collection with a system age is implemented.
                 Variable swapping and sifting are implemented.]

    FileName    [biddyInOut.c]
    Revision    [$Revision: 251 $]
    Date        [$Date: 2017-03-01 21:34:41 +0100 (sre, 01 mar 2017) $]
    Authors     [Robert Meolic (robert.meolic@um.si),
                 Ales Casar (ales@homemade.net),
                 Jan Kraner (jankristian.kraner@student.um.si),
                 Ziga Kobale (ziga.kobale@student.um.si),
                 Volodymyr Mihav (mihaw.wolodymyr@gmail.com),
                 David Kebo Houngninou (dhoungninou@smu.edu)]

### Copyright

Copyright (C) 2006, 2017 UM-FERI, Smetanova ulica 17, SI-2000 Maribor, Slovenia

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

/* the following types are used in Biddy_Eval2() */

typedef struct {
  int parent;
  int left;
  int right;
  int index;
  unsigned char op;
  Biddy_String name;
} BTreeNode;

typedef struct {
  BTreeNode *tnode;
  int availableNode;
} BTreeContainer;

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

typedef struct { /* table of lines is used to store acceptable lines */
  Biddy_String keyword;
  Biddy_String line;
} VerilogLine;

typedef struct { /* table of nodes is used to store names of all symbols */
  Biddy_String type; /* input, output, wire, gate, extern */
  Biddy_String name; /* node name */
} VerilogNode;

typedef struct { /* wires are primary inputs and gates */
  int id; /* input/gate ID number */
  Biddy_String type; /* type of a signal driving this wire, "I" for primary input, "W" for internal wire */
  unsigned int inputcount; /* number of input/gate inputs */
  int inputs[INOUTNUM]; /* array of inputs */
  unsigned int outputcount; /* number of input/gate outputs - currently not used */
  int outputs[INOUTNUM]; /* array of outputs - currently not used */
  int fanout; /* how many times this wire is used as an input to some gate or another wire */
  int ttl; /* tmp nuber used to guide GC during the creation of BDD */
  Biddy_Edge bdd; /* BDD associated with the wire/gate */
} VerilogWire;

typedef struct {
  char *name; /* name of the module */
  Biddy_Boolean outputFirst; /* TRUE iff "nand NAND2_1 (N1, N2, N3);" defines N1 = NAND(N2,N3) */
  unsigned int inputcount, outputcount; /* count of primary inputs and primary outputs. */
  unsigned int wirecount, regcount, gatecount; /* count of wires ,regs, gates */
  Biddy_String inputs[INOUTNUM], outputs[INOUTNUM]; /* name list of primary inputs and primary outputs in the netlist */
  Biddy_String wires[WIRENUM]; /* name list of wires in the netlist */
  Biddy_String regs[REGNUM]; /* name list of regs in the netlist */
  Biddy_String gates[GATENUM]; /* name list of gates in the netlist */
} VerilogModule;

typedef struct {
  Biddy_String name; /* name of the circuit */
  unsigned int inputcount, outputcount; /* number of primary inputs and primary outputs */
  unsigned int nodecount, wirecount; /* number of nodes and wires */
  Biddy_String inputs[INOUTNUM], outputs[INOUTNUM]; /* name list of primary inputs and primary outputs in the netlist */
  VerilogWire **wires; /* array of all wires */
  VerilogNode **nodes; /* array of nodes */
} VerilogCircuit;

/* the following type is used in Biddy_PrintfSOP() and BiddyWriteSOP() */

typedef struct VarList {
  struct VarList *next;
  Biddy_Variable v;
  Biddy_Boolean mark;
} VarList;


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

/* Function createBddFromBTree() is used in Biddy_Eval2(). */

static Biddy_Edge createBddFromBTree(Biddy_Manager MNG, BTreeContainer *tree, int i);

/* The following functions are used in Biddy_ReadVerilogFile(). */
static void parseVerilogFile(FILE *verilogfile, unsigned int *l, VerilogLine ***lt, unsigned int *n, VerilogModule ***mt);
static void createVerilogCircuit(unsigned int linecount, VerilogLine **lt, unsigned int modulecount, VerilogModule **mt, VerilogCircuit *c);
static void createBddFromVerilogCircuit(Biddy_Manager MNG, VerilogCircuit *c, Biddy_String prefix);
static void parseSignalVector(Biddy_String signal_arr[], Biddy_String token[], unsigned int *index, unsigned int *count);
static void printModuleSummary(VerilogModule *m);
static void printCircuitSummary(VerilogCircuit *c);
static Biddy_Boolean isGate(Biddy_String word);
static Biddy_Boolean isSignalVector(Biddy_String word);
static Biddy_Boolean isEndOfLine(Biddy_String source);
static Biddy_Boolean isDefined(VerilogCircuit *c, Biddy_String name);
static void buildNode(VerilogNode *n, Biddy_String type, Biddy_String name);
static void buildWire(VerilogCircuit *c, VerilogWire *w, Biddy_String type, Biddy_String name);
static int getNodeIdByName(VerilogCircuit *c, Biddy_String name);
static VerilogWire *getWireById(VerilogCircuit *c, int id);
static VerilogWire *getWireByName(VerilogCircuit *c, Biddy_String name);
static Biddy_String trim(Biddy_String source);
static void concat(char **s1, const char *s2);

/* Function WriteBDD() is used in Biddy_WriteBDD(). */

static void WriteBDD(Biddy_Manager MNG, Biddy_Edge f);

/* Function WriteBDDx() is used in Biddy_WriteBDDx(). */

static void WriteBDDx(Biddy_Manager MNG, FILE *funfile, Biddy_Edge f, unsigned int *line);

/* The following functions are used in Biddy_PrintfSOP() in Biddy_WriteSOP(). */

static void WriteProduct(Biddy_Manager MNG, VarList *l);
static void WriteSOP(Biddy_Manager MNG, Biddy_Edge f, Biddy_Boolean mark, VarList *l, unsigned int *maxsize);

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

  if (biddyManagerType == BIDDYTYPEOBDD) {
  } else {
    /* NOT IMPLEMENTED, YET */
    printf("(Biddy_Managed_Eval0) ERROR: Unsupported GDD type!\n");
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
    Original author: Volodymyr Mihav (mihaw.wolodymyr@gmail.com)
    Original implementation of this function is on
    https://github.com/sungmaster/liBDD.
    Macro Biddy_Eval2(boolFunc) is defined for use with anonymous manager.
*******************************************************************************/

/* USE THIS FOR DEBUGGING */
/*
static void printBTreeContainer(BTreeContainer *tree);
static void printPrefixFromBTree(BTreeContainer *tree, int i);
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
  Biddy_String expression;

  if (!MNG) MNG = biddyAnonymousManager;

  i = 0; j = 0;
  tree = (BTreeContainer *) calloc(1, sizeof(BTreeContainer));
  if (!tree) return biddyNull;
  tree->tnode = (BTreeNode*) malloc(1024 * sizeof(BTreeNode));
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
  }

  /*
  printBTreeContainer(tree);
  */

  /*
  printf("%s\n",expression);
  printPrefixFromBTree(tree,tree->tnode[0].right);
  printf("\n");
  */

  free(expression);

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
  VerilogCircuit *c;
  unsigned int linecount, modulecount;
  VerilogLine **lt;
  VerilogModule **mt;
  VerilogModule *md;
  VerilogLine *ln;
  unsigned int i,j;

  if (!MNG) MNG = biddyAnonymousManager;

  s = fopen(filename,"r");
  if (!s) return;

  printf("PARSING FILE %s\n",filename);
  c = (VerilogCircuit *) calloc(1,sizeof(VerilogCircuit)); /* declare an instance of a circuit */
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

  /* DEBUGGING */
  /*
  if (biddyManagerType == BIDDYTYPETZBDD) {
    Biddy_Variable v,k;
    Biddy_String vname;
    printf("DOMAIN\n");
    v = 0;
    for (k = 1; k < biddyVariableTable.num; k++) {
      v = biddyVariableTable.table[v].prev;
    }
    for (k = 1; k < biddyVariableTable.num; k++) {
      vname = Biddy_Managed_GetVariableName(MNG,v);
      printf("%s,",vname);
      v = biddyVariableTable.table[v].next;
    }
    printf("\n");
  }
  */

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
    Thanks to Jan Kraner (jankristian.kraner@student.um.si) and
    Ziga Kobale (ziga.kobale@student.um.si).
    Macro Biddy_PrintfTable(f) is defined for use with anonymous manager.
*******************************************************************************/

#ifdef __cplusplus
extern "C" {
#endif

void
Biddy_Managed_PrintfTable(Biddy_Manager MNG, Biddy_Edge f)
{
  Biddy_Edge support;
  unsigned int variableNumber;
  Biddy_Variable* variableTable;
  unsigned int i;
  int j;
  Biddy_Edge temp;
  unsigned int numcomb;

  if (!MNG) MNG = biddyAnonymousManager;

  if (Biddy_IsNull(f)) {
    printf("NULL\n");
    return;
  }

  /* support is a product of all dependent variables in BDD */
  support = Biddy_Managed_Support(MNG,f);

  /* variableNumber is the number of dependent variables in BDD */
  variableNumber = 0;
  temp = support;
  while (!Biddy_IsConstant(temp)) {
    variableNumber++;
    temp = BiddyT(temp);
  }

  if (variableNumber > 8) {
    printf("Table for %d variables is to large for output.\n",variableNumber);
    return;
  }

  /* variableTable is a table of all dependent variables in BDD */
  if (!(variableTable = (Biddy_Variable *)malloc((variableNumber) * sizeof(Biddy_Variable)))) return;
  i = 0;
  while (!Biddy_IsConstant(support)) {
    variableTable[i] = Biddy_GetTopVariable(support);
    printf("|%s",Biddy_Managed_GetVariableName(MNG,variableTable[i]));
    support = BiddyT(support);
    i++;
  }
  printf("|:value\n");

  numcomb = 1;
  for (i = 0; i < variableNumber; i++) numcomb = 2 * numcomb;
  for (i = 0; i < numcomb; i++)
  {
    temp = f;
    for (j = variableNumber - 1; j >= 0; j--)
    {
      temp = BiddyManagedRestrict(MNG,
               temp,variableTable[variableNumber-j-1],(i&(1 << j))?TRUE:FALSE);
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
    if (temp == biddyZero) printf("0\n"); else printf("1\n");
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
    Thanks to Jan Kraner (jankristian.kraner@student.um.si) and
    Ziga Kobale (ziga.kobale@student.um.si).
    Macro Biddy_WriteTable(f) is defined for use with anonymous manager.
*******************************************************************************/

#ifdef __cplusplus
extern "C" {
#endif

void
Biddy_Managed_WriteTable(Biddy_Manager MNG, const char filename[], Biddy_Edge f)
{
  return;
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

  if (Biddy_IsNull(f)) {
    printf("NULL\n");
  } else {
    WriteSOP(MNG,f,Biddy_GetMark(f),NULL,&maxsize);
    printf("\n");
  }
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
  return;
}

#ifdef __cplusplus
}
#endif

/***************************************************************************//*!
\brief Function Biddy_Managed_WriteDot writes dot/graphviz format using
       fprintf.

### Description
    Output dot format. Two approaches are implemented.
    The CUDD-like implementation is copyied from CUDD 3.0.
### Side effects
    If (id != -1) then id is used instead of <...> for variable names.
    Function resets all variables value.
### More info
    Macro Biddy_WriteDot(filename,f,label) is defined for use with anonymous
    manager.
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

  assert( filename != NULL );

  /* if (id != -1) use it instead of <...> */

  if (Biddy_IsNull(f)) {
    /* printf("ERROR Biddy_Managed_WriteDot: Function is null!\n"); */
    return 0 ;
  }

  s = fopen(filename,"w");
  if (!s) return 0;

  if (Biddy_IsConstant(f)) {
    n = 1; /* there is one constant node */
  } else {
    BiddyCreateLocalInfo(MNG,f);
    n = enumerateNodes(MNG,f,0); /* this will select all nodes except constant node */
    Biddy_Managed_DeselectAll(MNG);
  }

  label1 = strdup(label);
  while ((hash=strchr(label1,'#'))) hash[0]='_'; /* avoid hashes in the name */

  if (cudd) {
    fprintf(s,"//GENERATED WITH BIDDY - biddy.meolic.com\n");
    fprintf(s,"//USE 'dot -y -Tpng -O %s' TO VISUALIZE %s\n",filename,biddyManagerName);
    fprintf(s,"digraph \"DD\" {\n");
    /* fprintf(s,"size = \"7.5,10\"\n"); */
    fprintf(s,"center = true;\nedge [dir = none];\n");
    fprintf(s,"{ node [shape = plaintext];\n");
    fprintf(s,"  edge [style = invis];\n");
    fprintf(s,"  \"CONST NODES\" [style = invis];\n");

    if (!Biddy_IsConstant(f)) {
      li = (BiddyLocalInfo *)(BiddyN(f)->list);
      Biddy_Managed_ResetVariablesValue(MNG);
      while (li->back) {
        v = BiddyV(li->back);
        if (biddyVariableTable.table[v].value == biddyZero) {
          biddyVariableTable.table[v].value = biddyOne;
          name = strdup(Biddy_Managed_GetVariableName(MNG,v));
          while ((hash = strchr(name, '#'))) hash[0] = '_';
          fprintf(s,"\" %s \" -> ",name);
          free(name);
        }
        li = &li[1]; /* next field in the array */
      }
      Biddy_Managed_ResetVariablesValue(MNG);
    }

    fprintf(s,"\"CONST NODES\"; \n}\n");
    fprintf(s,"{ rank = same; node [shape = box]; edge [style = invis];\n");
    fprintf(s,"\"%s\"",label1);
    fprintf(s,"; }\n");
  } else {
    fprintf(s,"//GENERATED WITH BIDDY - biddy.meolic.com\n");
    fprintf(s,"//USE 'dot -y -Tpng -O %s' TO VISUALIZE %s\n",filename,biddyManagerName);
    fprintf(s,"digraph BDD {\n");
    fprintf(s,"  ordering = out;\n");
    fprintf(s,"  splines = true;\n");
#ifdef LEGACY_DOT
#else
    fprintf(s,"  edge [dir=\"both\"];\n");
#endif
    fprintf(s,"  edge [arrowhead=\"none\"]\n");
    fprintf(s,"  node [shape = none, label = \"%s\"] 0;\n",label1);
  }

  WriteDotNodes(MNG,s,f,id,cudd);
  if (Biddy_GetMark(f)) {
    if ((tag = Biddy_GetTag(f))) {
      if (cudd) {
        /* li = (BiddyLocalInfo *)(BiddyN(f)->list); */
        fprintf(s,"\"%s\"",label1);
        if (Biddy_IsConstant(f)) {
          fprintf(s," -> \"1\" [style = dotted label=\"%s\"];\n",Biddy_Managed_GetVariableName(MNG,tag));
        } else {
          fprintf(s," -> \"%p\" [style = dotted label=\"%s\"];\n",BiddyP(f),Biddy_Managed_GetVariableName(MNG,tag));
        }
      } else {
#ifdef LEGACY_DOT
        fprintf(s,"  0 -> 1 [style = dotted label=\"%s\"];\n",Biddy_Managed_GetVariableName(MNG,tag));
#else
        fprintf(s,"  0 -> 1 [arrowtail=\"none\" arrowhead=\"dot\" label=\"%s\"];\n",Biddy_Managed_GetVariableName(MNG,tag));
#endif
      }
    } else {
      if (cudd) {
        /* li = (BiddyLocalInfo *)(BiddyN(f)->list); */
        fprintf(s,"\"%s\"",label1);
        if (Biddy_IsConstant(f)) {
          if (biddyManagerType == BIDDYTYPEZBDD) {
            fprintf(s," -> \"1\" [style = bold];\n");
          } else {
            fprintf(s," -> \"1\" [style = dotted];\n");
          }
        } else {
          if (biddyManagerType == BIDDYTYPEZBDD) {
            fprintf(s," -> \"%p\" [style = bold];\n",BiddyP(f));
          } else {
            fprintf(s," -> \"%p\" [style = dotted];\n",BiddyP(f));
          }
        }
      } else {
#ifdef LEGACY_DOT
        if (biddyManagerType == BIDDYTYPEZBDD) {
          fprintf(s,"  0 -> 1 [style = bold];\n");
        } else {
          fprintf(s,"  0 -> 1 [style = dotted];\n");
        }
#else
        fprintf(s,"  0 -> 1 [arrowtail=\"none\" arrowhead=\"dot\"];\n");
#endif
      }
    }
  } else {
    if ((tag = Biddy_GetTag(f))) {
      if (cudd) {
        /* li = (BiddyLocalInfo *)(BiddyN(f)->list); */
        fprintf(s,"\"%s\"",label1);
        if (Biddy_IsConstant(f)) {
          fprintf(s," -> \"1\" [style = solid label=\"%s\"];\n",Biddy_Managed_GetVariableName(MNG,tag));
        } else {
          fprintf(s," -> \"%p\" [style = solid label=\"%s\"];\n",BiddyP(f),Biddy_Managed_GetVariableName(MNG,tag));
        }
      } else {
#ifdef LEGACY_DOT
        fprintf(s,"  0 -> 1 [style = solid label=\"%s\"];\n",Biddy_Managed_GetVariableName(MNG,tag));
#else
        fprintf(s,"  0 -> 1 [arrowtail=\"none\" label=\"%s\"];\n",Biddy_Managed_GetVariableName(MNG,tag));
#endif
      }
    } else {
      if (cudd) {
        /* li = (BiddyLocalInfo *)(BiddyN(f)->list); */
        fprintf(s,"\"%s\"",label1);
        if (Biddy_IsConstant(f)) {
          fprintf(s," -> \"1\" [style = solid];\n");
        } else {
          fprintf(s," -> \"%p\" [style = solid];\n",BiddyP(f));
        }
      } else {
#ifdef LEGACY_DOT
        fprintf(s,"  0 -> 1 [style = solid];\n");
#else
        fprintf(s,"  0 -> 1 [arrowtail=\"none\"];\n");
#endif
      }
    }
  }

  free(label1);

  WriteDotEdges(MNG,s,f,cudd); /* this will select all nodes except constant node */

  if (!Biddy_IsConstant(f)) {
    BiddyDeleteLocalInfo(MNG,f);
  }

  fprintf(s,"}\n");
  fclose(s);

  return n;
}

#ifdef __cplusplus
}
#endif

/***************************************************************************//*!
\brief Function Biddy_Managed_WriteBDDView writes bddview format using fprintf.

### Description
    Output bddview format.
### Side effects
    Parameter table is optional, if not NULL then it must contain node names
    and coordinates.
### More info
    Macro Biddy_WriteBddview(filename,f,label) is defined for use with anonymous
    manager.
*******************************************************************************/

#ifdef __cplusplus
extern "C" {
#endif

unsigned int
Biddy_Managed_WriteBddview(Biddy_Manager MNG, const char filename[],
                           Biddy_Edge f, const char label[], Biddy_XY *table)
{
  FILE *s;
  int i,n;
  Biddy_Boolean useCoordinates;
  BiddyLocalInfo *li;
  Biddy_Variable tag;

  if (!MNG) MNG = biddyAnonymousManager;

  assert( filename != NULL );

  if (Biddy_IsNull(f)) {
    /* printf("ERROR Biddy_Managed_WriteBddview: Function is null!\n"); */
    return 0 ;
  }

  s = fopen(filename,"w");
  if (!s) return(0);

  useCoordinates = FALSE;
  if (Biddy_IsConstant(f)) {

    n = 1; /* there is one constant node */
    if (table) {
      /* use table of nodes given by the user */
      useCoordinates = TRUE;
    } else {
      n = 1; /* there is one constant node */
      table = (Biddy_XY *) malloc((n+1) * sizeof(Biddy_XY)); /* n = nodes, add one label */
      if (!table) return 0;
      table[0].id = 0;
      table[0].label = strdup(label);
      table[0].x = 0;
      table[0].y = 0;
      table[0].isConstant = FALSE;
      table[1].id = 1;
      table[1].label = getname(MNG,biddyOne);
      table[1].x = 0;
      table[1].y = 0;
      table[1].isConstant = TRUE;
    }

  } else {

    BiddyCreateLocalInfo(MNG,f);
    n = enumerateNodes(MNG,f,0); /* this will select all nodes except constant node */
    Biddy_Managed_DeselectAll(MNG);

    if (table) {
      /* use table of nodes given by the user */
      useCoordinates = TRUE;
    } else {
      /* generate table of nodes */
      /* TO DO: coordinates are not calculated, how to do this without graphviz/dot? */
      table = (Biddy_XY *) malloc((n+1) * sizeof(Biddy_XY)); /* n = nodes, add one label */
      if (!table) return 0;
      table[0].id = 0;
      table[0].label = strdup(label);
      table[0].x = 0;
      table[0].y = 0;
      table[0].isConstant = FALSE;
      li = (BiddyLocalInfo *)(BiddyN(f)->list);
      while (li->back) {
        table[li->data.enumerator].id = li->data.enumerator;
        table[li->data.enumerator].label = getname(MNG,li->back);
        table[li->data.enumerator].x = 0;
        table[li->data.enumerator].y = 0;
        table[li->data.enumerator].isConstant = FALSE;
        if (Biddy_IsConstant(BiddyE(li->back)) || Biddy_IsConstant(BiddyT(li->back))) {
          table[li->data.enumerator+1].id = li->data.enumerator+1;
          table[li->data.enumerator+1].label = getname(MNG,biddyOne);
          table[li->data.enumerator+1].x = 0;
          table[li->data.enumerator+1].y = 0;
          table[li->data.enumerator+1].isConstant = TRUE;
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
  for (i=1;i<(n+1);++i) {
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
    for (i=1;i<(n+1);++i) {
      free(table[i].label);
    }
    free(table);
  }

  /* WRITE bddview CONNECTIONS */

  fprintf(s,"connect 0 1 ");
  if (Biddy_GetMark(f)) {
    fprintf(s,"si");
  } else {
    fprintf(s,"s");
  }
  if ((biddyManagerType == BIDDYTYPETZBDD) || (biddyManagerType == BIDDYTYPETZFDD)) {
    tag = Biddy_GetTag(f);
    fprintf(s," %s\n",Biddy_Managed_GetVariableName(MNG,tag));
  } else {
    fprintf(s,"\n");
  }

  if (!Biddy_IsConstant(f)) {
    WriteBddviewConnections(MNG,s,f); /* this will select all nodes except constant node */
    BiddyDeleteLocalInfo(MNG,f);
  }

  fclose(s);

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

  assert( biddyManagerType == BIDDYTYPEOBDD );

  nextCh(s,i,ch);

  if (strcmp(*ch,"*")) {
    inv = FALSE;
  } else {
    inv = TRUE;
    /* printf("<*>"); */
    nextCh(s,i,ch);
  }

  /* printf("<%s>",*ch); */

  if (strcmp(*ch,"1")) {
    varname = strdup(*ch);
    n = Biddy_Managed_AddVariableByName(MNG,varname); /* variables should already exist */
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
    f = Biddy_InvCond(Biddy_Managed_ITE(MNG,n,t,e),inv); /* OBDDs */
    free(varname);
  } else {
    f = Biddy_InvCond(biddyOne,inv); /* OBDDs */
  }

  return f;
}

static Biddy_Edge
evaluate1(Biddy_Manager MNG, Biddy_String s, int *i, Biddy_String *ch,
          Biddy_LookupFunction lf)
{
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
      if (!Biddy_Managed_FindFormula(MNG,*ch,&f)) {
        f = Biddy_Managed_AddVariableByName(MNG,*ch);
      }
    } else {
      if ((!(lf(*ch,&f))) &&
          (!Biddy_Managed_FindFormula(MNG,*ch,&f))) {
        f = Biddy_Managed_AddVariableByName(MNG,*ch);
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

/* DEBUGGING, ONLY */
/*
static void
printBTreeContainer(BTreeContainer *tree)
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
*/

/* DEBUGGING, ONLY */
/*
static void
printPrefixFromBTree(BTreeContainer *tree, int i)
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

static Biddy_Edge
createBddFromBTree(Biddy_Manager MNG, BTreeContainer *tree, int i)
{
  Biddy_Edge lbdd,rbdd,fbdd;
  if (i == -1) return biddyNull;
  if (tree->tnode[i].op == 255) {
    if (tree->tnode[i].name) {
      if (!strcmp(tree->tnode[i].name,"0")) return biddyZero;
      else if (!strcmp(tree->tnode[i].name,"1")) return biddyOne;
      else {
        if (!(Biddy_Managed_FindFormula(MNG,tree->tnode[i].name,&fbdd))) {
          fbdd = Biddy_Managed_AddVariableByName(MNG,tree->tnode[i].name);
        }
        return fbdd;
      }
    }
    return biddyNull;
  }
  lbdd = createBddFromBTree(MNG,tree,tree->tnode[i].left);
  rbdd = createBddFromBTree(MNG,tree,tree->tnode[i].right);
  if (tree->tnode[i].op == 0) fbdd = Biddy_Managed_Not(MNG,rbdd); /* NOT */
  else if (tree->tnode[i].op == 1) fbdd = Biddy_Managed_And(MNG,lbdd,rbdd); /* AND */
  else if (tree->tnode[i].op == 2) fbdd = Biddy_Managed_And(MNG,lbdd,Biddy_Managed_Not(MNG,rbdd)); /* BUTNOT */
  else if (tree->tnode[i].op == 4) fbdd = Biddy_Managed_And(MNG,Biddy_Managed_Not(MNG,lbdd),rbdd); /* NOTBUT */
  else if (tree->tnode[i].op == 6) fbdd = Biddy_Managed_Xor(MNG,lbdd,rbdd); /* EXOR */
  else if (tree->tnode[i].op == 7) fbdd = Biddy_Managed_Or(MNG,lbdd,rbdd); /* OR */
  else fbdd = biddyNull;
  return fbdd;
}

static void
parseVerilogFile(FILE *verilogfile, unsigned int *l, VerilogLine ***lt, unsigned int *n, VerilogModule ***mt)
{
  unsigned int activemodule=0;
  VerilogLine *ln;
  VerilogModule *md;
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
      (*mt) = (VerilogModule **) realloc((*mt), (*n) * sizeof(VerilogModule *)); /* create or enlarge table of modules */
      md = (VerilogModule *) calloc(1, sizeof(VerilogModule)); /* declare an instance of a module */
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
              /**/
              printf("WARNING (parseVerilogFile): wire %s already defined as input/output\n",token[j]);
              /**/
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
      (*lt) = (VerilogLine **) realloc((*lt), (*l) * sizeof(VerilogLine *)); /* create or enlarge table of lines */
      ln = (VerilogLine *) calloc(1, sizeof(VerilogLine)); /* declare an instance of a line */
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
createVerilogCircuit(unsigned int linecount, VerilogLine **lt, unsigned int modulecount, VerilogModule **mt, VerilogCircuit *c)
{
  VerilogModule *m;
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
  c->wires = (VerilogWire **) calloc(c->wirecount,sizeof(VerilogWire *)); /* allocate a contiguous array to index every wire */
  c->nodes = (VerilogNode **) calloc(c->nodecount,sizeof(VerilogNode *)); /* allocate a contiguous array to index every node */

  id = 0;
  for (i=0; i < m->inputcount; i++) { /* store the names of primary inputs */
    c->inputs[i] = (Biddy_String) calloc(strlen(m->inputs[i]) + 1, sizeof (char));
    strcpy(c->inputs[i],m->inputs[i]);
    c->nodes[id] = (VerilogNode *) calloc(1,sizeof(VerilogNode));
    buildNode(c->nodes[id],(Biddy_String)"input", m->inputs[i]);
    id++;
  }
  for (i=0; i < m->outputcount; i++) { /* store the names of primary outputs */
    c->outputs[i] = (Biddy_String) calloc(strlen(m->outputs[i]) + 1, sizeof(char));
    strcpy(c->outputs[i],m->outputs[i]);
    c->nodes[id] = (VerilogNode *) calloc(1,sizeof(VerilogNode));
    buildNode(c->nodes[id],(Biddy_String)"output",m->outputs[i]);
    id++;
  }
  for (i=0; i < m->wirecount; i++) { /* store the names of wires */
    c->nodes[id] = (VerilogNode *) calloc(1,sizeof(VerilogNode));
    buildNode(c->nodes[id],(Biddy_String)"wire",m->wires[i]);
    id++;
  }
  for (i=0; i < m->gatecount; i++) { /* store the names of gates */
    c->nodes[id] = (VerilogNode *) calloc(1,sizeof(VerilogNode));
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
        c->wires[index] = (VerilogWire *) calloc(1,sizeof(VerilogWire));
        buildWire(c,c->wires[index],(Biddy_String)"I",token[j]);
        /* assign inputs to the wires representing gate inputs */
        /* NOT NEEDED */
        /* assign outputs to the wires representing gate inputs */
        /* NOT USED */
        index++;
      }
    }

    /* B. Create a wire for the gate */
    c->wires[index] = (VerilogWire *) calloc(1,sizeof(VerilogWire));
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
      c->wires[index] = (VerilogWire *) calloc(1,sizeof(VerilogWire));
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
createBddFromVerilogCircuit(Biddy_Manager MNG, VerilogCircuit *c, Biddy_String prefix)
{
  unsigned int i,j;
  VerilogWire *w;
  Biddy_Edge f;

  /* DEBUGGING */
  /**/
  printf("Creating %d primary inputs\n",c->inputcount);
  /**/
  /*
  for (i = 0; i < c->inputcount; i++) {
    printf("%s ",c->inputs[i]);
  }
  if (c->inputcount) printf("\n");
  */
  /**/
  printf("Creating %d primary outputs\n",c->outputcount);
  /**/
  /*
  for (i = 0; i < c->outputcount; i++) {
    printf("%s ",c->outputs[i]);
  }
  if (c->outputcount) printf("\n");
  */
  /**/
  printf("Creating %d wires\n",c->wirecount);
  /**/

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

    Biddy_Managed_AddTmpFormula(MNG,f,c->wires[i]->ttl);
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
      Biddy_Managed_NodeNumber(MNG,f),
      Biddy_Managed_NodeNumberPlain(MNG,f)
    );
  }
  */

  /**/
  j = 0;
  for (i = 0; i < c->outputcount; i++) {
    f = getWireByName(c,c->outputs[i])->bdd;
    j = j + Biddy_Managed_NodeNumber(MNG,f);
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
printModuleSummary(VerilogModule *m)
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
printCircuitSummary(VerilogCircuit *c)
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
isDefined(VerilogCircuit *c, Biddy_String name)
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
buildNode(VerilogNode *n, Biddy_String type, Biddy_String name)
{
  n->type = strdup(type);
  n->name = strdup(name);
}

/**
 * Create a wire
 * @param the circuit object, the wire object, the wire type, the wire name
 */
static void
buildWire(VerilogCircuit *c, VerilogWire *w, Biddy_String type, Biddy_String name)
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
getNodeIdByName(VerilogCircuit *c, Biddy_String name)
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
static VerilogWire *
getWireById(VerilogCircuit *c, int id)
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
static VerilogWire *
getWireByName(VerilogCircuit *c, Biddy_String name)
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

static void
WriteProduct(Biddy_Manager MNG, VarList *l)
{
  if (l) {
    WriteProduct(MNG,l->next);
    printf(" ");
    if (l->mark) {
      printf("*");
    }
    printf("%s",Biddy_Managed_GetVariableName(MNG,l->v));
  }
}

static void
WriteSOP(Biddy_Manager MNG, Biddy_Edge f, Biddy_Boolean mark, VarList *l, unsigned int *maxsize)
{
  VarList tmp;

  if (*maxsize == 0) return; /* SOP to large */

  if (Biddy_IsConstant(f)) {
    if (!mark) {
      printf("  + ");
      if (l) {
        WriteProduct(MNG,l);
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
  } else {
    tmp.next = l;
    tmp.v = BiddyV(f);
    tmp.mark = TRUE;
    WriteSOP(MNG,BiddyE(f),(mark ^ Biddy_GetMark(BiddyE(f))),&tmp,maxsize);
    tmp.mark = FALSE;
    WriteSOP(MNG,BiddyT(f),(mark ^ Biddy_GetMark(BiddyT(f))),&tmp,maxsize);
  }
}

static unsigned int
enumerateNodes(Biddy_Manager MNG, Biddy_Edge f, unsigned int n)
{
  if (!Biddy_Managed_IsSelected(MNG,f)) {
    if (Biddy_IsConstant(f)) {
    } else {
      Biddy_Managed_SelectNode(MNG,f);
      n++;
      BiddySetEnumerator(f,n);
      if (Biddy_IsConstant(BiddyE(f)) || Biddy_IsConstant(BiddyT(f))) {
        n++; /* every instance of constant node is enumerated */
      }
      if (!Biddy_IsConstant(BiddyE(f))) {
        n = enumerateNodes(MNG,BiddyE(f),n);
      }
      if (!Biddy_IsConstant(BiddyT(f))) {
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

  if (Biddy_IsConstant(f)) {
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
      fprintf(dotfile, "  node [shape = none, label = \"1\"] %u;\n",1);
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
        while ((hash = strchr(name, '#'))) hash[0] = '_';
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
      if (Biddy_IsConstant(BiddyE(li->back)) || Biddy_IsConstant(BiddyT(li->back))) {
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
      while ((hash = strchr(name, '#'))) hash[0] = '_';
      fprintf(dotfile,"  node [shape = circle, label = \"%s\"] %u;\n",name,li->data.enumerator);
      free(name);
      if (Biddy_IsConstant(BiddyE(li->back)) || Biddy_IsConstant(BiddyT(li->back))) {
        fprintf(dotfile,"  node [shape = none, label = \"1\"] %u;\n",li->data.enumerator+1);
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

  if (!Biddy_IsConstant(f) && !Biddy_Managed_IsSelected(MNG,f)) {
    Biddy_Managed_SelectNode(MNG,f);
    n1 = BiddyGetEnumerator(f);

    if (Biddy_IsConstant(BiddyE(f))) {
      n2 = n1 + 1;
    } else {
      n2 = BiddyGetEnumerator(BiddyE(f));
    }
    if (Biddy_GetMark(BiddyE(f))) {
      if ((tag = Biddy_GetTag(BiddyE(f)))) {
        if (cudd) {
          /* VARIANT 1 - THIS SHOULD BE COMBINED WITH VARIANT 1 IN WriteDotNodes */
          /*
          fprintf(dotfile,"\"%p\" -> \"%p\" [style = dotted label=\"%s\"];\n",BiddyP(f),BiddyP(BiddyE(f)),Biddy_Managed_GetVariableName(MNG,tag));
          */
          /* VARIANT 2 - THIS SHOULD BE COMBINED WITH VARIANT 2 IN WriteDotNodes */
          /**/
          if (Biddy_IsConstant(BiddyE(f))) {
            fprintf(dotfile,"\"%p\" -> \"%u\" [style = dotted label=\"%s\"];\n",BiddyP(f),n2,Biddy_Managed_GetVariableName(MNG,tag));
          } else {
            fprintf(dotfile,"\"%p\" -> \"%p\" [style = dotted label=\"%s\"];\n",BiddyP(f),BiddyP(BiddyE(f)),Biddy_Managed_GetVariableName(MNG,tag));
          }
          /**/
        } else {
#ifdef LEGACY_DOT
          fprintf(dotfile,"  %u -> %u [style = dotted label=\"%s\"];\n",n1,n2,Biddy_Managed_GetVariableName(MNG,tag));
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
          if (Biddy_IsConstant(BiddyE(f))) {
            fprintf(dotfile,"\"%p\" -> \"%u\" [style = dotted];\n",BiddyP(f),n2);
          } else {
            fprintf(dotfile,"\"%p\" -> \"%p\" [style = dotted];\n",BiddyP(f),BiddyP(BiddyE(f)));
          }
          /**/
        } else {
#ifdef LEGACY_DOT
          fprintf(dotfile,"  %u -> %u [style = dotted];\n",n1,n2);
#else
          fprintf(dotfile,"  %u -> %u [arrowtail=\"odot\" arrowhead=\"dot\"];\n",n1,n2);
#endif
        }
      }
    } else {
      if ((tag = Biddy_GetTag(BiddyE(f)))) {
        if (cudd) {
          /* VARIANT 1 - THIS SHOULD BE COMBINED WITH VARIANT 1 IN WriteDotNodes */
          /*
          fprintf(dotfile,"\"%p\" -> \"%p\" [style = dashed label=\"%s\"];\n",BiddyP(f),BiddyP(BiddyE(f)),Biddy_Managed_GetVariableName(MNG,tag));
          */
          /* VARIANT 2 - THIS SHOULD BE COMBINED WITH VARIANT 2 IN WriteDotNodes */
          /**/
          if (Biddy_IsConstant(BiddyE(f))) {
            fprintf(dotfile,"\"%p\" -> \"%u\" [style = dashed label=\"%s\"];\n",BiddyP(f),n2,Biddy_Managed_GetVariableName(MNG,tag));
          } else {
            fprintf(dotfile,"\"%p\" -> \"%p\" [style = dashed label=\"%s\"];\n",BiddyP(f),BiddyP(BiddyE(f)),Biddy_Managed_GetVariableName(MNG,tag));
          }
          /**/
        } else {
#ifdef LEGACY_DOT
          fprintf(dotfile,"  %u -> %u [style = dashed label=\"%s\"];\n",n1,n2,Biddy_Managed_GetVariableName(MNG,tag));
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
          if (Biddy_IsConstant(BiddyE(f))) {
            fprintf(dotfile,"\"%p\" -> \"%u\" [style = dashed];\n",BiddyP(f),n2);
          } else {
            fprintf(dotfile,"\"%p\" -> \"%p\" [style = dashed];\n",BiddyP(f),BiddyP(BiddyE(f)));
          }
          /**/
        } else {
#ifdef LEGACY_DOT
        fprintf(dotfile,"  %u -> %u [style = dashed];\n",n1,n2);
#else
        fprintf(dotfile,"  %u -> %u [arrowtail=\"odot\"];\n",n1,n2);
#endif
        }
      }
    }

    if (Biddy_IsConstant(BiddyT(f))) {
      n2 = n1 + 1;
    } else {
      n2 = BiddyGetEnumerator(BiddyT(f));
    }
    if (Biddy_GetMark(BiddyT(f))) {
      if ((tag = Biddy_GetTag(BiddyT(f)))) {
        if (cudd) {
          /* VARIANT 1 - THIS SHOULD BE COMBINED WITH VARIANT 1 IN WriteDotNodes */
          /*
          fprintf(dotfile,"\"%p\" -> \"%p\" [style = bold label=\"%s\"];\n",BiddyP(f),BiddyP(BiddyT(f)),Biddy_Managed_GetVariableName(MNG,tag));
          */
          /* VARIANT 2 - THIS SHOULD BE COMBINED WITH VARIANT 2 IN WriteDotNodes */
          /**/
          if (Biddy_IsConstant(BiddyT(f))) {
            fprintf(dotfile,"\"%p\" -> \"%u\" [style = bold label=\"%s\"];\n",BiddyP(f),n2,Biddy_Managed_GetVariableName(MNG,tag));
          } else {
            fprintf(dotfile,"\"%p\" -> \"%p\" [style = bold label=\"%s\"];\n",BiddyP(f),BiddyP(BiddyT(f)),Biddy_Managed_GetVariableName(MNG,tag));
          }
          /**/
        } else {
#ifdef LEGACY_DOT
          fprintf(dotfile,"  %d -> %d [style = bold label=\"%s\"];\n",n1,n2,Biddy_Managed_GetVariableName(MNG,tag));
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
          if (Biddy_IsConstant(BiddyT(f))) {
            fprintf(dotfile,"\"%p\" -> \"%u\" [style = bold];\n",BiddyP(f),n2);
          } else {
            fprintf(dotfile,"\"%p\" -> \"%p\" [style = bold];\n",BiddyP(f),BiddyP(BiddyT(f)));
          }
          /**/
        } else {
#ifdef LEGACY_DOT
          fprintf(dotfile,"  %d -> %d [style = bold];\n",n1,n2);
#else
          fprintf(dotfile,"  %d -> %d [arrowtail=\"invempty\" arrowhead=\"dot\"];\n",n1,n2);
#endif
        }
      }
    } else {
      if ((tag = Biddy_GetTag(BiddyT(f)))) {
        if (cudd) {
          /* VARIANT 1 - THIS SHOULD BE COMBINED WITH VARIANT 1 IN WriteDotNodes */
          /*
          fprintf(dotfile,"\"%p\" -> \"%p\" [label=\"%s\"];\n",BiddyP(f),BiddyP(BiddyT(f)),Biddy_Managed_GetVariableName(MNG,tag));
          */
          /* VARIANT 2 - THIS SHOULD BE COMBINED WITH VARIANT 2 IN WriteDotNodes */
          /**/
          if (Biddy_IsConstant(BiddyT(f))) {
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
        if (Biddy_IsConstant(BiddyT(f))) {
          fprintf(dotfile,"\"%p\" -> \"%u\";\n",BiddyP(f),n2);
        } else {
          fprintf(dotfile,"\"%p\" -> \"%p\";\n",BiddyP(f),BiddyP(BiddyT(f)));
        }
        /**/
        } else {
#ifdef LEGACY_DOT
          fprintf(dotfile,"  %d -> %d;\n",n1,n2);
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

  if (!Biddy_IsConstant(f) && !Biddy_Managed_IsSelected(MNG,f)) {
    Biddy_Managed_SelectNode(MNG,f);
    n1 = BiddyGetEnumerator(f);

    if (BiddyP(BiddyE(f)) == BiddyP(BiddyT(f))) {

      /* DOUBLE LINE */

      if (Biddy_IsConstant(BiddyE(f))) {
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
      if ((biddyManagerType == BIDDYTYPETZBDD) || (biddyManagerType == BIDDYTYPETZFDD)) {
        tag1 = Biddy_GetTag(BiddyE(f));
        tag2 = Biddy_GetTag(BiddyT(f));
        fprintf(funfile," %s %s\n",Biddy_Managed_GetVariableName(MNG,tag1),Biddy_Managed_GetVariableName(MNG,tag2));
      } else {
        fprintf(funfile,"\n");
      }

    } else {

      /* SINGLE LINE */

      if (Biddy_IsConstant(BiddyE(f))) {
        n2 = n1 + 1;
      } else {
        n2 = BiddyGetEnumerator(BiddyE(f));
      }
      fprintf(funfile,"connect %d %d ",n1,n2);
      if (Biddy_GetMark((BiddyE(f)))) {
        fprintf(funfile,"li");
      } else {
        fprintf(funfile,"l");
      }
      if ((biddyManagerType == BIDDYTYPETZBDD) || (biddyManagerType == BIDDYTYPETZFDD)) {
        tag1 = Biddy_GetTag(BiddyE(f));
        fprintf(funfile," %s\n",Biddy_Managed_GetVariableName(MNG,tag1));
      } else {
        fprintf(funfile,"\n");
      }

      if (Biddy_IsConstant(BiddyT(f))) {
        n2 = n1 + 1;
      } else {
        n2 = BiddyGetEnumerator(BiddyT(f));
      }
      fprintf(funfile,"connect %d %d ",n1,n2);
      if (Biddy_GetMark((BiddyT(f)))) {
        fprintf(funfile,"ri");
      } else {
        fprintf(funfile,"r");
      }
      if ((biddyManagerType == BIDDYTYPETZBDD) || (biddyManagerType == BIDDYTYPETZFDD)) {
        tag2 = Biddy_GetTag(BiddyT(f));
        fprintf(funfile," %s\n",Biddy_Managed_GetVariableName(MNG,tag2));
      } else {
        fprintf(funfile,"\n");
      }

    }

    WriteBddviewConnections(MNG,funfile,BiddyE(f));
    WriteBddviewConnections(MNG,funfile,BiddyT(f));
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

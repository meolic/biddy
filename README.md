USER MANUAL
==========

### TL;DR
--------------

**Biddy** is a multi-platform Binary Decision Diagrams package.
It supports plain ROBDDs, ROBDDs with complemented edges,
0-sup-BDDs, 0-sup-BDDs with complemented edges, and
tagged 0-sup-BDDs.

Biddy is a mathematical library capable of all the typical
operations regarding Boolean functions and combination sets.

Biddy is a library to be included in your C and C++ projects:

~~~c
#include "/path/to/biddy.h"
~~~

To **compile** Biddy library use "make static" or "make dynamic" or "make debug".
There is no configuration script, you should edit **Makefiles** to adapt the system
configuration. Alternatively, use the prepared **Visual Studio project** (VS/Biddy.sln).

~~~
biddy> make static
biddy> make clean
~~~

**Dependencies**

- you need libgmp (https://gmplib.org/), this library is already available on GNU/Linux and MSYS2 environments
- on MS Windows, you can use MPIR library (http://mpir.org/) instead of libgmp from MSYS2

Programs builded staticaly with Biddy should run everywhere without any dependency problems.

On GNU/Linux, to run programs builded dynamicaly with Biddy, you may need to set some environment variables, e.g. for bash:

~~~bash
export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:/absolute/path/to/biddy/library
~~~

On MS Windows, programs builded dynamicaly with Biddy will run outside of the MSYS2 environment only, if /mingw64/bin/libgmp-10.dll from MSYS2 is also distributed
(or the corresponding mpir library that was used for building).

**Tests**:

- Create a static Biddy library.
- Compile the Hanoi example (notes for building are given in the source code).
- Alternatively, compile any of the other example.

~~~
biddy> make static
biddy> gcc -DUNIX -DBIDDY -o hanoi biddy-example-hanoi.c -I. -L./bin -static -lbiddy -lgmp
biddy> ./hanoi 10
~~~

There are two **aditional packages** included into Biddy distribution:

- **bddview** is a pure Tcl/Tk script for visualization of BDDs,
- **BDD Scout** is a demo application demonstrating the capability of Biddy and bddview.

To create BDD Scout:

~~~
biddy> make static
biddy> cd bddscout
bddscout> make
~~~

To run BDD Scout:

- You will need Tcl/Tk + BWidgets to run it.
- You have to install Graphviz to be able to visualize BDDs created by Biddy.
- You have to install Ghostscript to be able to export BDDs as PNG images and PDF documents.

Biddy is **free software** maintained by Robert Meolic (robert@meolic.com).

- You can fork this project on https://github.com/meolic/biddy.
- Fell free to report requests, bugs, and other issues on GitHub.
- Contributions (patches and ideas) can be send also via author's email.

**Homepage**: http://biddy.meolic.com/

### 1. AN OVERVIEW
--------------

Biddy supports ROBDDs as described in "K. S. Brace,
R. L. Rudell, R. E. Bryant. Efficient  Implementation  of a  BDD
Package. 27th ACM/IEEE DAC, pages 40-45, 1990".

Biddy supports 0-sup-BDDs (also denoted ZBDDs or ZDDs) as described in
"S. Minato. Zero-Suppressed BDDs for Set Manipulation in Combinatorial
Problems, 30th ACM/IEEE DAC, pages 272-277, 1993".

Biddy supports tagged 0-sup-BDDs (also denoted TZDDs or TZBDDs) as
introduced in "R. Meolic. Implementation aspects of a BDD package supporting
general decision diagrams. https://dk.um.si/IzpisGradiva.php?id=68831"
and described in "T. van Dijk, R. Wille, R. Meolic.
Tagged BDDs: Combining Reduction Rules from Different Decision Diagram Types,
17th FMCAD, pages 108-115, 2017".

Biddy includes:

- automatic garbage collection with a system age
(a variant of a mark-and-sweep approach),
- node management through formulae protecting,
- variable swapping and sifting algorithm for all supported BDD types.

Biddy is optimized for efficiency, but it is  mainly oriented towards
readable and comprehensible source  code in C.

Biddy  is  currently  used  in  the following  projects:

- BDD Scout, demo project which allows visualization of BDDs and
also includes some benchmarks
- BDD Encyclopedia, a set of documents produced with Biddy and BDD Scout
(http://svn.savannah.nongnu.org/viewvc/*checkout*/biddy/bddscout/ENCYCLOPEDIA/bddencyclopedia.html)
- Efficient Symbolic Tools (EST), model checking and other algorithms
for formal verification of systems (http://est.meolic.com/)

### 2. SOURCE CODE
--------------

Biddy is free software released under GPL.

The short name of Biddy package is 'biddy'. This name is placed in front
of  all filenames and external identifiers.  It may appear in
all  lowercase,  or with  its  first  letter  capitalized, written  as
'biddy' and 'Biddy', respectively.

There are three categories of C functions.

- Exported functions are visible outside the package.
- Internal functions are visible to all files within the package.
- Static functions are visible to the file only.

There are two types of C functions.

- General functions which operates on a particular BDD and considering only
  graph properties (i.e. changing edge's mark, selecting nodes, counting nodes
  etc.). These functions are the same for different type of decision diagrams
  (BDD, ZDD, etc.). Functions, which add or delete nodes or those which needs
  info about variables (e.g. a name or a value) are not general functions.
  Exported general functions have prefix Biddy_.
- Managed functions, which operates on a global properties of a BDD system
  (e.g. node table, variable table, formula table, various caches, etc.)
  or consider a BDD as a Boolean function (e.g. Boolean operations, counting
  minterms, etc.). These functions need info stored in a manager.
  Exported managed functions have prefix Biddy_Managed_.

Biddy consists of the following core files:

- README.md (this file)
- paper.md (JOSS paper)
- CHANGES (history of changes)
- COPYING (license file)
- VERSION (project's version)
- Makefile (used to produce binary code)
- Makefile.Linux (Makefile definitions for GNU/Linux)
- Makefile.MINGW (Makefile definitions for MS Windows)
- Makefile.Darwin (Makefile definitions for MacOS)
- biddy.h (header)
- biddyInt.h (header)
- biddyMain.c (main functions)
- biddyOp.c (functions for operations on BDDs)
- biddyStat.c (functions for statistic)
- biddyInOut.c (parsers and generators for Boolean functions)
- biddy-example-*.* (examples)
- package-source (script used to build distribution)
- package-bin (script used to build distribution)
- package-bin.bat (script used to build distribution)
- package-tgz (script used to build distribution)
- package-deb (script used to build distribution)
- package-rpm (script used to build distribution)
- VS/* (project files for MS Visual Studio)
- debian/* (files used when creating deb package)
- rpm/* (files used when creating rpm package)

There are  two C headers,  external and internal. The  external header
file,  named  biddy.h, defines  features  visible  from outside  the
package. The internal header file, named biddyInt.h defines features
used in multiple files inside the package, but not outside.

Details about building are given in Section 4.

### 3. USING BIDDY LIBRARY
----------------------

Precompiled packages include dynamically linked library
(i.e. *.so on GNU/Linux, *.dll on MS Windows, *.dylib on Mac OS X),
and the appropriate C header biddy.h. Currently, there are no interfaces
for other programming languages.

For linking with Biddy library you have to use (remove -lgmp if you have Biddy as a dynamic library):

~~~
-lbiddy -lgmp
~~~

The following code is an example of usage.
Please note, that functions for node management are not shown. Moreover,
Biddy has a manager but its usage is optional and it is also not shown in
the given example.

IMPORTANT:
You should define UNIX, MACOSX, or MINGW.
You should define USE_BIDDY if you are using Biddy via dynamic library.

~~~c
/* Linux + gcc: gcc -DUNIX -o biddyexample1 biddyexample1.c -I. -L./bin -lbiddy -lgmp */
/* WINDOWS + MINGW: gcc -DMINGW -o biddyexample1.exe biddyexample1.c -I. -L./bin -lbiddy -lgmp */
/* in this example, x1, x2, and x3 are Boolean variables */
#include "biddy.h"
int main() {
  Biddy_Edge f,g,h,r;
  Biddy_Init(); /* use default, i.e. PLAIN ROBDDS WITHOUT COMPLEMENTED EDGES */
  printf("Biddy is using %s.\n",Biddy_GetManagerName());
  f = Biddy_Eval1((Biddy_String)"(OR (AND (NOT x1) x3) (AND x1 (NOT x2) (NOT x3)))"); /* PREFIX */
  g = Biddy_Eval2((Biddy_String)"~x1*(~x3*0+x3*1)+x1*(~x2*(~x3*1+x3*0)+x2*0)"); /* INFIX */
  if (f == g) printf("Boolean functions f and g are equal.\n");
  if (f != Biddy_Not(g)) printf("Boolean functions f and ~g are not equal.\n");
  printf("Boolean function f depends on %u variables.\n",Biddy_DependentVariableNumber(f,FALSE));
  printf("Boolean function f has %.0f minterms.\n",Biddy_CountMinterms(f,0));
  printf("BDD for function f has %u nodes (including both terminals).\n",Biddy_CountNodes(f));
  printf("Here is a truth table for Boolean function f\n");
  Biddy_PrintfTable(f);
  Biddy_WriteDot("biddyexample1.dot",f,"f",-1,FALSE);
  printf("USE 'dot -y -Tpng -O biddyexample1.dot' to visualize BDD for Boolean function f.\n");
  Biddy_Exit();
}
~~~

If you do not want to use the default BDD type then simply change the
initialization call. Supported BDD types are BIDDYTYPEOBDD,
BIDDYTYPEOBDDC, BIDDYTYPEZBDD, BIDDYTYPEZBDDC, and BIDDYTYPETZBDD.

~~~c
Biddy_InitAnonymous(BIDDYTYPEZBDD);
~~~

Here is another example. Please note the explicite usage of a manager which enables working with different types of BDDs simultaneously.

~~~c
/* Linux + gcc: gcc -DUNIX -o biddyexample2 biddyexample2.c -I. -L./bin -lbiddy -lgmp */
/* WINDOWS + MINGW: gcc -DMINGW -o biddyexample2.exe biddyexample2.c -I. -L./bin -lbiddy -lgmp */
/* in this example, letters D, E, H, L, O, R, and W are Boolean variables */
#include "biddy.h"
#define Str2Var(mng,x) (Biddy_Managed_GetVariable(mng,(Biddy_String)x))
int main() {
  Biddy_Manager mngobdd, mngobddc;
  Biddy_Edge f,g,r1,r2;

  Biddy_InitMNG(&mngobdd,BIDDYTYPEOBDD); /* PLAIN ROBDDS WITHOUT COMPLEMENTED EDGES */
  Biddy_InitMNG(&mngobddc,BIDDYTYPEOBDDC); /* ROBDDS WITH COMPLEMENTED EDGES */
  f = Biddy_Managed_Eval1(mngobdd,(Biddy_String)"(EXOR H E L L O (AND W O R L D))"); /* PREFIX */
  g = Biddy_Managed_Eval2(mngobdd,(Biddy_String)"~(H * E + R * O)"); /* INFIX */
  /* BASIC OPERATION */
  r1 = Biddy_Managed_Nor(mngobdd,f,g);
  /* SOP OUTPUT */
  printf("Here is Boolean function:\n");
  Biddy_Managed_PrintfSOP(mngobdd,r1);
  /* REPLACE SOME VARIABLES: H-->L, R-->L */
  Biddy_Managed_ResetVariablesValue(mngobdd);
  Biddy_Managed_SetVariableValue(mngobdd,Str2Var(mngobdd,"H"),Biddy_Managed_GetVariableEdge(mngobdd,Str2Var(mngobdd,"L")));
  Biddy_Managed_SetVariableValue(mngobdd,Str2Var(mngobdd,"R"),Biddy_Managed_GetVariableEdge(mngobdd,Str2Var(mngobdd,"L")));
  r1 = Biddy_Managed_Replace(mngobdd,r1);
  /* SOP OUTPUT AFTER REPLACING VARIABLES */
  printf("Here is this Boolean function after replacing H-->L and R-->L:\n");
  Biddy_Managed_PrintfSOP(mngobdd,r1);
  /* SIMPLE RESTRICTION */
  r1 = Biddy_Managed_Restrict(mngobdd,r1,Str2Var(mngobdd,"E"),FALSE);
  /* SOP OUTPUT AFTER REPLACING VARIABLES */
  printf("Here is the Boolean function after additional operation 'restrict E':\n");
  Biddy_Managed_PrintfSOP(mngobdd,r1);
  /* CONVERT OBDD TO OBDDC */
  r2 = Biddy_Managed_Copy(mngobdd,mngobddc,r1);
  /* GRAPHVIZ/DOT OUTPUT */
  Biddy_Managed_WriteDot(mngobdd,"biddyexample2obdd.dot",r1,"r1",-1,FALSE);
  printf("USE 'dot -y -Tpng -O biddyexample2obdd.dot' to visualize OBDD for the final Boolean function.\n");
  Biddy_Managed_WriteDot(mngobddc,"biddyexample2obddc.dot",r2,"r2",-1,FALSE);
  printf("USE 'dot -y -Tpng -O biddyexample2obddc.dot' to visualize OBDDC for the final Boolean function.\n");
  Biddy_ExitMNG(&mngobdd);
  Biddy_ExitMNG(&mngobddc);
}
~~~

### 3.1 NODE MANAGEMENT THROUGH FORMULAE PROTECTING

Biddy includes powerful node management based on formulae tagging.
There are six user functions to maintain nodes.

### Biddy_AddFormula(name,bdd,c)

Given BDD becomes a formula. Its nodes will be preserved for the given
number of cleanings. If (name != NULL) then formula is accessible by its
name. If formula with a given name already exists it is overwritten.
If (c == -1) then formula is refreshed but not preserved.
If (c == 0) then formula is persistently preserved. There are five
macros defined to simplify adding formulae: Biddy_AddTmpFormula(name,bdd),
Biddy_AddPersistentFormula(name,bdd), Biddy_KeepFormula(bdd),
Biddy_KeepFormulaUntilPurge(bdd), and Biddy_KeepFormulaProlonged(bdd,c).

### Biddy_DeleteFormula(name)

Nodes of the given formula are marked as deleted.
Formula is not accessible by its name anymore.
Nodes of deleted formula which are preserved or persistently preserved
will not be removed by regular GC whilst Biddy_Purge will
immediately remove all of them.

### Biddy_Clean()

Discard all nodes which were not preserved or which are not preserved
anymore. Obsolete nodes are not immediately removed, they will be removed
during the first garbage collection. Use Biddy_Purge or Biddy_PurgeAndReorder
to immediately remove all obsolete nodes.

### Biddy_Purge()

Immediately removes all nodes which were not preserved or which are not
preserved anymore. Moreover, all obsolete formulae and all formulae without
a name are deleted. Nodes from all deleted formulae are removed if they are
not needed by other formulae. Call to Biddy_Purge does not count as a cleaning
and thus all preserved formulae remains preserved for the same number of
cleanings.

### Biddy_PurgeAndReorder(bdd)

The same as Biddy_Purge but also triggers reordering on function
(if BDD is given) or global reordering (if NULL is given).

### Biddy_Refresh(bdd)

All obsolete nodes in the given bdd become fresh nodes. Formula is not
created and no information about the existing formulae is changed.
This function is needed to implement user's operation caches, only. 

### 3.2. EXAMPLES OF NODE MANAGEMENT WITH BIDDY

The first example is a straightforward calculation.

~~~c
f1 = op(...);
f2 = op(...);
g1 = op(f1,f2,...);
Biddy_KeepFormula(g1); /* g1 is preserved for next cleaning */
f1 = op(...);
f2 = op(...);
g2 = op(f1,f2,...);
Biddy_KeepFormula(g2); /* g2 is preserved for next cleaning */
Biddy_Clean(); /* g1 and g2 are still usable, f1 and f2 are obsolete */
result = op(g1,g2,...);
Biddy_KeepFormulaUntilPurge(result); /* result is preserved until Biddy_Purge() */
Biddy_Clean(); /* only nodes from result remain preserved */
~~~

If garbage collection is needed after the calculation of g1,
you must add some code after the calculation of g1:

~~~c
f1 = op(...);
f2 = op(...);
g1 = op(f1,f2,...);
Biddy_KeepFormulaProlonged(g1,2); /* g1 is preserved for next two cleanings */
Biddy_Clean(); /* g1 remains preserved for next cleaning */
f1 = op(...);
f2 = op(...);
g2 = op(f1,f2,...);
Biddy_KeepFormula(g2); /* g2 is preserved for next cleaning */
Biddy_Clean(); /* g1 and g2 are still usable, f1 and f2 are obsolete */
result = op(g1,g2,...);
Biddy_KeepFormulaUntilPurge(result); /* result is preserved until Biddy_Purge() */
Biddy_Clean(); /* only nodes from result remain preserved */
~~~

In this approach, f1 and f2 become obsolete after Biddy_Clean, but their
nodes are not immediately removed (automatic garbage collection is only
started when there are no free nodes in the system).

Alternatively, you can use the following code which is simpler but somehow
less efficient because Biddy_Purge() immediately starts garbage collection
(even if there are enough of free nodes and GC is not needed):

~~~c
f1 = op(...);
f2 = op(...);
g1 = op(f1,f2,...);
Biddy_AddTmpFormula("g1",g1); /* g1 is named */
Biddy_Purge(); /* keep only nodes from non-obsolete named formulae */
f1 = op(...);
f2 = op(...);
g2 = op(f1,f2,...);
Biddy_AddTmpFormula("g2",g2); /* g2 is named */
Biddy_Purge(); /* keep only nodes from non-obsolete named formulae */
result = op(g1,g2,...);
Biddy_AddPersistentFormula("result",result); /* result is permanently preserved */
Biddy_Clean(); /* all tmp formulae become obsolete */
Biddy_Purge(); /* keep only nodes from non-obsolete named formulae */
~~~

The second example is an iterative calculation:

~~~c
result = op(...);
while (!finish) {
  Biddy_KeepFormula(result); /* result is preserved for next cleaning */
  Biddy_Clean(); /* result remains preserved */
  f = op(...);
  g = op(f,...);
  result = op(result,g,...);
}
Biddy_KeepFormulaUntilPurge(result); /* result is preserved until Biddy_Purge() */
Biddy_Clean(); /* tmp results are not needed, anymore */
~~~

If garbage collection is needed after the calculation of g, you must
use the following code:

~~~c
result = op(...);
while (!finish) {
  Biddy_KeepFormulaProlonged(result,2); /* result is preserved for next two cleanings */
  Biddy_Clean(); /* result remains preserved */
  f = op(...);
  g = op(f,...);
  Biddy_KeepFormula(g); /* g is preserved for next cleaning */
  Biddy_Clean(); /* result and g are still usable, f is obsolete */
  result = op(result,g,...);
}
Biddy_KeepFormulaUntilPurge(result); /* result is preserved until Biddy_Purge() */
Biddy_Clean(); /* tmp results are not needed, anymore */
~~~

The third example is an outline of an implementation of bisimulation:

~~~c
init = AND(init_p,init_q)
Biddy_KeepFormulaUntilPurge(init) /* init is preserved until Biddy_Purge() */
eq = InitialEq(init_p,tr_p,init_q,tr_q,...);
do {
  Biddy_KeepFormula(eq); /* eq is preserved for next cleaning */
  Biddy_Clean(); /* eq remains preserved */
  last = eq;
  eq1 = NextEqPart1(eq,tr_p,tr_q,...);
  eq2 = NextEqPart2(eq,tr_p,tr_q,...);
  eq = AND(eq1,eq2);
} while (AND(init,eq)!=0 && eq!=last)
if (AND(init,eq)!=0) result=false; else result=true;
Biddy_Purge(); /* immediately remove all nodes created during the calculation */
~~~

The fourth example is an outline of an implementation of model checking:

~~~c
sup = Prepare(...);
Biddy_KeepFormulaUntilPurge(sup) /* preserve for the iteration */
Z = Biddy_GetConstantZero();
last = Biddy_GetConstantOne();
while (Z != last) {
  last = Z;
  Z = Image(Z,sup,...);
  Biddy_KeepFormula(last); /* preserve for the next cleaning */
  Biddy_KeepFormula(Z); /* preserve for the next cleaning */
  Biddy_Clean(); /* remove unnecessary intermediate results */
}
Biddy_AddPersistentFormula("result",Z);
Biddy_Purge(); /* remove all nodes not belonging to result */
~~~

The fifth example is again a model checking algorithm, but
here we are trying to benefit from regularly reordering:

~~~c
sup = Prepare(...);
Biddy_AddPersistentFormula("sup",sup) /* sup is permanently preserved */
Z = 0;
last = 1;
while (Z!=last) {
  Biddy_AddPersistentFormula("Z",Z); /* old Z is marked as deleted */
  Biddy_PurgeAndReorder(Z); /* perform reordering to optimize Z */
  last = Z;
  Z = NextSet(Z,sup,...);
}
result = Z;
Biddy_AddPersistentFormula("result",result); /* final result is permanently preserved */
Biddy_DeleteFormula("sup"); /* sup is marked as deleted */
Biddy_DeleteFormula("Z"); /* Z is marked as deleted */
Biddy_Purge(); /* remove nodes not belonging to result */
~~~

The sixth example is an outline of a complex implementation of operation parallel
composition in process algebra where we are trying to benefit from intensive GC:

~~~c
sacc = snew = AND(init_1,init_2,...,init_N);
for (i=1;i<=N;i++) di[i] = 0;
for (i=1;i<=N;i++) for (j=1;i<=N;j++) dij[i,j] = 0;
do {
  Biddy_KeepFormulaProlonged(snew,N*(N+1)); /* snew is preserved just long enough */
  Biddy_KeepFormulaProlonged(sacc,N*(N+1)); /* sacc is preserved just long enough */
  new1 = 0;
  for (i=1;i<=N;i++) {
    sup = OneStep(snew,tr_i,...);
    di[i] = OR(d[i],sup);
    new1 = OR(new1,NextState(sup,...));
    Biddy_KeepFormulaProlonged(d[i],N*(N+1)); /* di[i] is preserved just long enough */
    Biddy_KeepFormulaProlonged(new1,1); /* new1 is preserved for next cleaning, only */
    Biddy_Clean(); /* new1 remains usable */
  }
  Biddy_KeepFormulaProlonged(new1,N*N); /* new1 is preserved just long enough */
  new2 = 0;
  for (i=1;i<=N;i++) for (j=1;j<=N;j++) {
    sup = OneStep(snew,tr_i,tr_j,...);
    dij[i,j] = OR(d[i,j],sup);
    new2 = OR(new2,NextState(sup,...);
    Biddy_KeepFormulaProlonged(dij[i,j],N*(N+1)); /* dij[i,j] is preserved just long enough */
    Biddy_KeepFormulaProlonged(new2,1); /* new2 is preserved for next cleaning, only */
    Biddy_Clean(); /* new2 remains usable */
  }
  snew = AND(OR(new1,new2),NOT(sacc));
  sacc = OR(sacc,snew);
} while (snew!=0)
tr1 = 0;
for (i=1;i<=N;i++) {
  sup = AddStab(di[i],...);
  tr1 = OR(tr1,sup); 
  Biddy_KeepFormulaProlonged(tr1,1); /* tr1 is preserved for next cleaning, only */
  Biddy_Clean(); /* tr1 remains usable */
}
Biddy_KeepFormulaProlonged(tr1,N*N); /* tr1 is preserved just long enough */
tr2 = 0;
for (i=1;i<=N;i++) for (j=1;j<=N;j++) {
  sup = AddStab(dij[i,j],...);
  tr2 = OR(tr2,sup);
  Biddy_KeepFormulaProlonged(tr2,1); /* tr2 is preserved for next cleaning, only */
  Biddy_Clean(); /* tr2 remains usable */
}
result = OR(tr1,tr2);
Biddy_KeepFormulaUntilPurge(result); /* final result is permanently preserved */
Biddy_Clean(); /* tmp results are not needed, anymore */
~~~

### 3.3 MORE DETAILS ON MEMORY MANAGEMENT (SYSTEM AGE AND NODE CHAINING)

Garbage collection is automatically triggered if nodes from all reserved
blocks of nodes are used. Garbage collection will remove as many
obsolete nodes as possible.

Biddy does not use reference counter but a different approach.
We call the implemented algorithm "GC with a system age".
It is a variant of a mark-and-sweep approach.

Using system age instead of reference counter has
some advantages. It allows GC to be started in any time
without breaking the ongoing calculation. Thus, there is no need
to taking care of repeating broken calculations. Moreover,
the usage of system age instead of reference counter
removes all the hassle of referencing and dereferencing nodes
and thus it is favorable in an academic package oriented towards
simple and readable source code.

There are four classes of nodes. Every node belongs to one of
these classes:

- __fortified__ node (expiry value == 0);
- __fresh__ node (expiry value == biddySystemAge);
- __prolonged__ node (expiry value > biddySystemAge);
- __obsolete__ node (0 < expiry value < biddySystemAge).

Before each GC, nodes must be refreshed in such a way that no successor of
a non-obsolete node is obsolete. This is achieved with a relatively simple
loop which check every nodes at most once.

Biddy relies on a single hash table for all variables. However, it supports
chaining of nodes to form different lists (using an extra pointer in
each node). This facility is used to improve the efficiency of garbage
collection and sifting.

Please node, that node chaining is not determined or limited by
using formulae protecting schema or a system age approach, it is 
an independent mechanism.

### 4. BUILDING PACKAGES
--------------------

### Compiling Biddy library

On GNU/Linux, we are using gcc.

~~~
biddy> make dynamic
biddy> make clean
~~~

Alternatively, you can use:

~~~
biddy> make static
biddy> make debug
biddy> make profile
~~~

You can use specify the target folder:

~~~
biddy> make dynamic "BINDIR = ./bin"
biddy> make clean "BINDIR = ./bin"
~~~

On MS Windows, we are using MSYS2.
We start "MSYS2 MinGW x64" and use pacman to prepare the development environment:

~~~
MSYS shell> pacman -Syuu
MSYS shell> pacman -S tar
MSYS shell> pacman -S subversion
MSYS shell> pacman -S mingw-w64-x86_64-gcc
MSYS shell> pacman -S make
MSYS shell> pacman -S autoconf
MSYS shell> pacman -S automake
MSYS shell> pacman -S libtool
MSYS shell> pacman -S yasm
MSYS shell> pacman -S texinfo
MSYS shell> pacman -S bison
MSYS shell> pacman -S gdb
~~~

Alternatively, you can use Visual Studio for building.
There is a prepared solution consisting of many projects
(VS/Biddy.sln). You may need to adapt include and lib folders.

### Dependencies

Biddy uses libgmp (https://gmplib.org/).

On MS Windows, MPIR library (http://mpir.org/) can be used instead of libgmp from MSYS2.

Programs builded staticaly with Biddy should run everywhere without any dependency problems.

On GNU/Linux, to run programs builded dynamicaly with Biddy, you may need to set some environment variables, e.g. for bash:

~~~bash
export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:/absolute/path/to/biddy/library
~~~

On MS Windows, programs builded dynamicaly with Biddy will run outside of the MSYS2 environment only, if /mingw64/bin/libgmp-10.dll from MSYS2 is also distributed
(or the corresponding mpir library that was used for building).

### Creating dynamic Biddy library as a zip or a self-extracting exe package

~~~
biddy> ./package-bin (GNU/Linux)
biddy> ./package-bin.bat (Win + MSYS2)
~~~

On GNU/Linux, the packaging script prepares a plain zip file. You only need a zip program.

On MS Windows, the packaging script prepares a self-extracting exe package.
You  need 7-Zip (http://7-zip.org/) - and it has a strange use of -x!
You also   need   file  a special sfx file  that  was developed
as part of "7z SFX Tools". That project is no longer active, the latest
snapshot is available at:
http://web.archive.org/web/20160819141806/http://www.7zsfx.info/en/

The required tool itself is now available at:
https://github.com/chrislake/7zsfxmm/releases/

Download 7zsd_extra_171_3901.7z and extract 7zsd_All_x64.sfx in the directory containing 7z.exe.

You install the resulting package by extracting libraries to the
appropriate directory (may be local, e.g. user's home directory).

### Creating zip file with source code of a complete Biddy project

~~~
biddy> ./package-source
~~~

If available, source code of bddview and BDD Scout will be included, too.

### Creating packages for GNU/Linux

~~~
biddy> ./package-tgz
biddy> ./package-deb
biddy> ./package-rpm
~~~

These scripts are intedend to be used on Ubuntu.
These scripts needs release  number as an argument. Script package-tgz
must be  invoked before running package-deb.  Debian  packages must be
created before RPM packages. 

./package-tgz should  create orig.tar.gz file  and prepare directories
for creating debian  and RPM packages. You can  run ./package-tgz only
if version changes.

./package-deb   should    create   debian   packages    (libbidy   and
libbiddy-dev).  They are tested on Ubuntu system.

./package-rpm should create RPM packages (libbidy and libbiddy-devel).
They are tested on openSUSE system.

### Creating demo aplication bddscout

You need complete sources for biddy, bddview, and bddscout,
Scripts are located in biddy/bddscout.

~~~
bddscout> ./package-bin (GNU/Linux)
bddscout> ./package-tgz (GNU/Linux)
bddscout> ./package-deb (GNU/Linux)
bddscout> ./package-rpm (GNU/Linux)
bddscout> ./package-bin.bat (Win + MSYS2)
~~~

package-bin should  create BDD Scout  (staticaly linked with Biddy
library). The script  will produce a zip  file. You install BDD  Scout
by simply unzip to the target directory.

package-bin.bat should  create BDD Scout on MS Windows (staticaly linked
with Biddy library). The script  will produce a self-extracting exe file.

./package-tgz should  create orig.tar.gz file and prepare directories
for creating debian  and RPM packages. You can run ./package-tgz only
if version changes.

./package-deb should create debian packages (bddscout, bddscout-bra,
bddscout-ifip, bddscout-bddtrace, bddscout-ifip-data, and
bddscout-bddtrace-data). They are tested on Ubuntu system.

./package-rpm should create RPM packages (bddscout, bddscout-bra,
bddscout-ifip, bddscout-bddtrace, bddscout-ifip-data, and
bddscout-bddtrace-data). They are tested on openSUSE system.

### 5. HISTORY
----------

Biddy is based on a BDD package written in Pascal in 1992 as a student
project.  At that time, it was a great work and the paper about it won
a  second place  at  IEEE Region  Student  Paper Contest  (Paris-Evry,
1993).  The  paper was  published by IEEE  as "A.  Casar,  R.  Meolic.
Representation of  Boolean functions with ROBDDs.   IEEE Student paper
contest :  regional contest  winners 1990-1997 :  prize-winning papers
demonstrating student excellence  worldwide, Piscataway, 2000" and can
be obtained from
http://www.meolic.com/research/papers/robdd-casar-meolic.pdf .

In 1995, this BDD package was  rewritten in C. Later, this BDD package
become an  integral part of EST  package, a prototype  tool for formal
verification of  concurrent systems.  The homepage for  EST project is
http://est.meolic.com/ .

In 2003, BDD package from EST  was included in the report presented at
16th Symposium  on Integrated Circuits and  Systems Design (SBCCI'03).
The report is available as a paper "G.  Janssen.  A Consumer Report on
BDD Packages.  IBM T.J.  Watson  Research Center.  2003".  Get it from
http://doi.ieeecomputersociety.org/10.1109/SBCCI.2003.1232832 .

In 2006, BDD package in EST got the name Biddy.

In 2007, a main part of Biddy package was extracted from EST forming a
separate package called Biddy. The code has been reorganized in such a
way,  that EST  is not  using  internal structures  (e.g. node  table)
directly but using the provided API only.

In 2007,  we created local  svn repository for maintaining  the source
code (not live, anymore).

On  May 15,  2008, Biddy  v1.0  was  released containing  also bddview
v0.95 (Tcl/Tk BDD viewer) and Bdd Scout v0.90 (demo application).

In 2009, 2010, and 2012 updated versions of Biddy v1.0 were  released
which added support for debian packaging, support for  RPM  packaging,
fix errors,  and improve documentation, packaging, and Tcl/Tk GUI.

In 2012, a research paper about Biddy library appears in
Journal of Software (doi:10.4304/jsw.7.6.1358-1366)
http://www.jsoftware.us/vol7/jsw0706-23.pdf .

In 2013, Biddy  v1.1  was released. Biddy_Edge becames a pointer instead
of a structure and all other structures were optimized.

In 2014, Biddy  v1.2  was released. Variable swapping and sifting algorithm
were the most significant additions.

In 2014, svn repositories for biddy, bddview and bddscout are moved
to Savannah. http://svn.savannah.nongnu.org/viewvc/?root=biddy .

In 2015, Biddy  v1.3, v.1.4 and v1.5 was released. Various input/output methods
have been added. Support for 64-bit arhitectures and support for Visual Studio
projects were improved. Biddy got a manager. Many CUDD-like functions have been
added. Comment's style changed to support doxygen. HTML and PDF documentation
were produced.

Also in 2015, Personal Package Archive ppa:meolic/biddy has been created
https://launchpad.net/~meolic/+archive/ubuntu/biddy .

Also in 2015, sources became available on GitHub
https://github.com/meolic/biddy .

In 2016, Biddy v1.6 was released. Formulae are not recursively refreshed
all the time, anymore. The size of Node table became resizable.

In 2017, Biddy v1.7 was released (there were 4 minor releases).
Terminology has changed a lot,
e.g. "formulae counter" became "system age". Added support for ROBDDs
without complemented edges, 0-sup-BDDs and Tagged 0-sup-BDDs. Implemented
creation and manipulaton of non-anonymous managers. Added manipulation of
combination sets. Improved many functionalities, e.g sifting.
Many new CUDD-like functions have been added.
Moreover, bddview and BDD Scout have been significantly improved.

In 2018, Biddy v1.8 was released (there were 2 minor releases).
Added support for 0-sup-BDDs without complemented edges.
Added several new operations on 0-sup-BDDs that enable us to write
a very interesting paper "Flexible job shop scheduling using zero-suppressed
binary decision diagrams".
Again, bddview and BDD Scout have been significantly improved.
Started BDD Encyclopedia, check it out at
http://svn.savannah.nongnu.org/viewvc/*checkout*/biddy/bddscout/ENCYCLOPEDIA/bddencyclopedia.html .

In 2019, a short paper in Journal of Open Source Software was published.
Biddy v1.9 was released which brought an improved source code formatting.
Copyright changed because Robert Meolic is no longer with University of Maribor.

In 2020, Biddy v2.0 was released that was focused on features for unate cube
set algebra. Many internal improvements for Biddy and BDD Scout.

Biddy v2.1, released in 2021 completed the implementation of all unate cube set
algebra operations for all BDD types, including TZBDDs. Due to internal changes,
much more variables are supported and they are more efficiently manipulated.

Development of core have been slow on next years, no new functionalities in v2.2, and v2.3 was
not released at all. On the other hand, biddy-bff project started to produce a Sailfish OS
native mobile app named BFF. It will use Biddy to implement Quine-McCluskey minimization.

In 2024, Biddy v2.4 brought basic parser for Knuth's BDDL. BFF app evolved to the first
public release.

### 6. PUBLICATIONS
---------------

If you find our work useful, please, cite us.

- Robert Meolic. __Implementation aspects of a BDD package supporting general decision diagrams.__
  Technical report, University of Maribor, 2016.
  https://dk.um.si/IzpisGradiva.php?id=68831

- Tom van Dijk, Robert Wille, Robert Meolic.
  __Tagged BDDs: combining reduction rules from different decision diagram types.__
  In Proc. FMCAD 2017, pp. 108-115, 2017.
  https://ieeexplore.ieee.org/abstract/document/8102248
  (also https://dl.acm.org/doi/abs/10.5555/3168451.3168478)

- Robert Meolic, Zmago Brezočnik. __Flexible job shop scheduling using zero-suppressed binary decision diagrams.__
  Advances in production engineering & management, 13(4), pp. 373-388, 2018.
  http://apem-journal.org/Archives/2018/APEM13-4_373-388.pdf

- Robert Meolic. __The Biddy BDD package.__
  Journal of Open Source Software, 4(34), 1189, 2019.
  https://doi.org/10.21105/joss.01189

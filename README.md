### TL;DR
--------------

Biddy is a multi-platform academic Binary Decision Diagrams package.
It supports ROBDDs with complemented edges, 0-sup-BDDs with
complemented edges, and TZBDDs.

Biddy is capable of all the typical operations regarding
Boolean functions and BDDs.

Biddy is a library to be included in your C and C++ projects:

~~~
#define UNIX
#define USE_BIDDY
#include "/path/to/biddy.h"
~~~

To compile Biddy library:

~~~
biddy> make dynamic
biddy> make clean
~~~

Dependencies:

- on GNU/Linux, you need libgmp (https://gmplib.org/).
- on MS Windows, you need MPIR library (http://mpir.org/).

When using Biddy on GNU/Linux, you may have to tell bash about the library:

~~~
export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:/absolute/path/to/biddy/library
~~~

There  are two  aditional packages included into Biddy distribution:

- bddview is a pure Tcl/Tk script for visualization of BDDs,
- BDD Scout is a demo  application demonstrating the capability of Biddy and bddview.

Biddy is free software maintained by Robert Meolic
(robert.meolic@um.si) at University of Maribor, Slovenia.

Homepage: http://biddy.meolic.com/

### 1. AN OVERVIEW
--------------

Biddy supports ROBDDs with complemented edges as described in "K.S. Brace,
R.L. Rudell, R.E. Bryant. Efficient  Implementation  of a  BDD
Package. 27th ACM/IEEE DAC, pages 40-45, 1990".

Biddy supports 0-sup-BDDs with complemented edges as described in
"S. Minato. Zero-Suppressed BDDs for Set Manipulation in Combinatorial
Problems, 30th ACM/IEEE DAC, pages 272-277, 1993".

Biddy supports TZBDDs (tagged zero-supressed binary decison diagrams)
as introduced by Robert Meolic in 2016 (to be published).

Biddy includes:

- automatic garbage collection with a system age
  (a variant of a mark-and-sweep approach),
- node management through formulae protecting,
- variable swapping and sifting algorithm (ROBDDs, only).

Biddy is optimized for efficiency, but it is  mainly oriented towards
readable and  comprehensible source  code in C.

Biddy  is  currently  used  in  the following  projects:

- BDD Scout, demo project which allows visualization of BDDs and
  also includes some benchmarks
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
  info about variables (e.g. name) are not general functions.
  Exported general functions have prefix Biddy_.
- Managed functions, which operates on a global properties of a BDD system
  (e.g. node table, variable table, formula table, various caches, etc.)
  or consider a BDD as a Boolean function (e.g. Boolean operations, counting
  minterms, etc.). These functions need info stored in a manager.
  Exported managed functions have prefix Biddy_Managed_.

Biddy consists of the following files:

- README.md (this file)
- CHANGES (history of changes)
- COPYING (license file)
- VERSION (project's version)
- Makefile (used to produce binary code)
- Makefile.Linux (Makefile definitions for GNU/Linux)
- Makefile.MINGW (Makefile definitions for MS Windows)
- Makefile.Darwin (Makefile definitions for MacOS)
- biddy.h (header)
- biddyInt.h (header)
- biddyMain.c (main functions, ROBDDs only, deprecated)
- biddyMainGDD.c (main functions)
- biddyStat.c (functions for statistic)
- biddyInOut.c (parsers and generators for Boolean functions)
- package-source (script used to build distribution)
- package-bin (script used to build distribution)
- package-bin.bat (script used to build distribution)
- package-tgz (script used to build distribution)
- package-deb (script used to build distribution)
- package-rpm (script used to build distribution)
- debian/* (files used when creating deb package)
- rpm/* (files used when creating rpm package)
- biddy-example-8queens.c (8 Queens example)
- biddy-example-independence.c (8 Queens example)
- biddy-example-independence-usa.c (Independence example)
- biddy-example-independence-europe.c (Independence example)
- biddy-example-random.c (Random formulae example)
- biddy-example-hanoi.c (Tower of Hanoi example)

There are  two C headers,  external and internal. The  external header
file,  named  biddy.h, defines  features  visible  from outside  the
package. The internal header file, named biddyInt.h defines features
used in multiple files inside the package, but not outside.

There  are two  aditional packages  included into  Biddy distribution:

- bddview is a pure Tcl/Tk script for visualization of BDDs.
- BDD Scout is a demo  application demonstrating the capability of Biddy
  and bddview.

Details about building are given in Section 4.

### 3. USING BIDDY LIBRARY
----------------------

Precompiled packages include dynamically linked library
(i.e. *.so on GNU/Linux, *.dll on MS Windows, *.dylib on Mac OS X),
and the appropriate C header biddy.h. Currently, there are no interfaces
for other programming languages.

For linking with Biddy library you have to use:

~~~
-lbiddy -lgmp
~~~

Biddy is capable of all the typical operations regarding
Boolean functions and BDDs.

The following code is an example of usage.
Please note, that functions for node management are not shown. Moreover,
Biddy has a manager but its usage is optional and it is also not shown in
the given example.

IMPORTANT:
You should define UNIX, MACOSX, or MINGW.
You should define USE_BIDDY iff you are using Biddy via dynamic library.

~~~
#define UNIX
#define USE_BIDDY
#include "/path/to/biddy.h"

#define Str2Var(x) (Biddy_GetVariable(Biddy_String)x)

int main() {
  Biddy_Edge f,g,h,r;

  Biddy_Init();

  f = Biddy_Eval1((Biddy_String)"(OR H E L L O)"); /* PREFIX INPUT */
  g = Biddy_Eval1((Biddy_String)"(AND W O R L D)"); /* PREFIX INPUT */
  h = Biddy_Eval2((Biddy_String)"~(H * E * L * L)"); /* INFIX INPUT */

  /* BASIC OPERATION */
  r = Biddy_Xor(f,g);

  /* REPLACE SOME VARIABLES */
  Biddy_ResetVariablesValue();
  Biddy_SetVariableValue(Str2Var("H"),Biddy_FoaVariable((Biddy_String)"L"));
  Biddy_SetVariableValue(Str2Var("K"),Biddy_FoaVariable((Biddy_String)"L"));
  Biddy_SetVariableValue(Str2Var("W"),Biddy_FoaVariable((Biddy_String)"L"));
  r = Biddy_Replace(r);

  /* SIMPLE RESTRICTION */
  r = Biddy_Restrict(r,Str2Var("E"),Biddy_GetConstantZero());

  /* COUDERT AND MADRE'S RESTRICT FUNCTION */
  r = Biddy_Simplify(r,h);

  /* SOME STATS */
  printf("Function r depends on %u variables.\n",Biddy_VariableNumber(r));
  printf("Function r has %.0f minterms.\n",Biddy_CountMinterm(r,0));
  printf("BDD for function r has %u nodes.\n",Biddy_NodeNumber(r));

  /* TRUTH TABLE */
  printf("Here is a truth table for function r\n");
  Biddy_WriteTable(r);

  /* GRAPHVIZ/DOT OUTPUT */
  Biddy_WriteDot("biddy.dot",r,"Function r");
  printf("USE 'dot -y -Tpng -O biddy.dot' to visualize BDD for function r.\n");

  Biddy_Exit();
}
~~~

### 3.1 NODE MANAGEMENT THROUGH FORMULAE PROTECTING

Biddy includes powerful node management based on formulae tagging.
There are six user functions to maintain nodes.

### Biddy_AddFormula(name,bdd,c)

Nodes of the given BDD will be preserved for the given number of cleanings.
If (name != NULL) then formula is accessible by its name. If formula with
a given name already exists it is overwritten. If (c == 0) then
formula is persistently preserved and you have to use Biddy_DeleteFormula
to remove its nodes. There are two macros defined to simplify
formulae management. Macro Biddy_AddTmpFormula(bdd,c) is defined as
Biddy_AddFormula(NULL,bdd,c) and macro Biddy_AddPersistentFormula(name,bdd)
is defined as Biddy_AddFormula(name,bdd,0).

### Biddy_DeleteFormula(name)

Nodes of the given formula are tagged as not needed.
Formula is not accessible by its name anymore.
Regular cleaning with Biddy_Clean is not considered this tag.

### Biddy_Clean()

Discard all nodes which were not preserved or which are not preserved
anymore. Obsolete nodes are not immediately removed, they will be removed
during the first garbage collection. Use Biddy_Purge or Biddy_PurgeAndReorder
to immediately remove all non-preserved nodes.

### Biddy_Purge()

Immediately remove all nodes which were not preserved or which are not
preserved anymore.
Nodes from deleted prolonged formulae and nodes from deleted fortified
formulae are removed if they are not needed by other formulae.
Call to Biddy_Purge does not count as cleaning and thus all
preserved formulae remains preserved for the same number of cleanings. 

### Biddy_PurgeAndReorder(bdd)

The same as Biddy_Purge but also trigger reordering on function
(if BDD is given) or global reordering (if NULL is given).

### Biddy_Refresh(bdd)

All obsolete nodes become fresh nodes. This is an external variant of
internal function BiddyRefresh. It is needed to implement user caches.

### 3.2. EXAMPLES OF NODE MANAGEMENT WITH BIDDY

The first example is a straightforward calculation.

~~~
f1 = op(...);
f2 = op(...);
g1 = op(f1,f2,...);
Biddy_AddTmpFormula(g1,1); /* g1 is preserved for next cleaning */
f1 = op(...);
f2 = op(...);
g2 = op(f1,f2,...);
Biddy_AddTmpFormula(g2,1); /* g2 is preserved for next cleaning */
Biddy_Clean(); /* g1 and g2 are still usable, f1 and f2 are obsolete */
result = op(g1,g2,...);
Biddy_AddPersistentFormula("result",result); /* final result is permanently preserved */
Biddy_Clean(); /* g1 and g2 are not needed, anymore */
~~~

If additional garbage collection is needed also after the calculation of g1,
you can use the following code after the calculation of g1:

~~~
Biddy_AddTmpFormula(g1,2); /* g1 is preserved for next two cleanings */
Biddy_Clean(); /* g1 remains preserved for next cleaning */
~~~

In this approach, f1 and f2 become obsolete after Biddy_Clean, but their
nodes are not immediately removed (automatic garbage collection is only
started when there are no free nodes in the system).
If garbage collection should be started immediately, you must
use the following code after the calculation of g1:

~~~
Biddy_AddTmpFormula(g1,2); /* g1 is preserved for next two cleanings */
Biddy_Clean(); /* g1 remains preserved for next cleaning */
Biddy_Purge(); /* keep only preserved (g1) formulae */
~~~

Alternatively, you can use the following approach:

~~~
Biddy_AddTmpFormula(g1,1); /* g1 is preserved for next cleaning */
Biddy_Purge(); /* this will not make g1 obsolete */
~~~

To trigger reordering in this example, you should use Biddy_PurgeAndReorder
to get the following code:

~~~
f1 = op(...);
f2 = op(...);
g1 = op(f1,f2,...);
Biddy_AddTmpFormula(g1,2); /* g1 is preserved for next two cleanings */
Biddy_PurgeAndReorder(NULL); /* keep only preserved formulae (g1), perform reordering */
Biddy_Clean(); /* g1 remains preserved for one additional cleaning */
f1 = op(...);
f2 = op(...);
g2 = op(f1,f2,...);
Biddy_AddTmpFormula(g2,1); /* g2 is preserved for next cleaning */
Biddy_PurgeAndReorder(NULL); /* keep only preserved formulae (g1, g2), perform reordering */
Biddy_Clean(); /* g1 and g2 are still usable but not preserved */
result = op(g1,g2,...);
Biddy_AddPersistentFormula("result",result); /* result is permanently preserved */
Biddy_PurgeAndReorder(NULL); /* keep only preserved formulae (result), perform reordering */
~~~

The second example is an iterative calculation:

~~~
f = op(...);
result = op(f,...);
while (!finish) {
  Biddy_AddTmpFormula(result,1); /* result is preserved for next cleaning */
  Biddy_Clean(); /* result is still usable but not preserved */
  f = op(...);
  g = op(f,...);
  result = op(result,g,...);
}
Biddy_AddPersistentFormula("result",result); /* final result is permanently preserved */
Biddy_Clean(); /* temp results are not needed, anymore */
~~~

If garbage collection is needed also after the calculation of g, you must
use the following code:

~~~
f = op(...);
result = op(f,...);
while (!finish) {
  Biddy_AddTmpFormula(result,2); /* result is preserved for next two cleanings */
  Biddy_Clean(); /* result remains preserved for one additional cleaning */
  f = op(...);
  g = op(f,...);
  Biddy_AddTmpFormula(g,1); /* g is preserved for next cleaning */
  Biddy_Clean(); /* result and g are still usable but not preserved */
  result = op(result,g,...);
}
Biddy_AddPersistentFormula("result",result); /* final result is permanently preserved */
Biddy_Clean(); /* temp results are not needed, anymore */
~~~

To trigger reordering in the second example, you should change code in the following way:

~~~
f = op(...);
result = op(f,...);
while (!finish) {
  Biddy_AddTmpFormula(result,2); /* result is preserved for next two cleanings */
  Biddy_PurgeAndReorder(NULL); /* keep only preserved formulae (result), perform reordering */
  Biddy_Clean(); /* result remains preserved for one additional cleaning */
  f = op(...);
  g = op(f,...);
  Biddy_AddTmpFormula(g,1); /* g is preserved for next cleaning */
  Biddy_Clean(); /* result and g are still usable but not preserved */
  result = op(result,g,...);
}
Biddy_AddPersistentFormula("result",result); /* final result is permanently preserved */
Biddy_PurgeAndReorder(NULL); /* keep only preserved formulae (result), perform reordering */
~~~

The third example is an outline of an implementation of model checking where
we are trying to benefit from regularly reordering:

~~~
sup = Prepare(...);
Biddy_AddPersistentFormula("sup",sup) /* sup is permanently preserved */
Z = 0;
last = 1;
while (Z!=last) {
  Biddy_AddTmpFormula(Z,1); /* Z is preserved for next cleaning */
  Biddy_PurgeAndReorder(NULL); /* keep only preserved formulae (sup,Z), perform reordering */
  Biddy_Clean(); /* Z remains usable but not preserved */
  last = Z;
  Z = NextSet(Z,sup,...);
}
result = Z;
Biddy_AddPersistentFormula("result",result); /* final result is permanently preserved */
Biddy_DeleteFormula("sup"); /* optional, if you really need to remove sup */
Biddy_Purge(); /* optional, immediately remove non-preserved nodes */
~~~

The fourth example is an outline of an implementation of bisimulation
where we are trying to benefit from regularly reordering:

~~~
init = AND(init_p,init_q)
Biddy_AddPersistentFormula("init",init) /* init is permanently preserved */
eq = InitialEq(init_p,tr_p,init_q,tr_q,...);
do {
  Biddy_AddTmpFormula(eq,1); /* eq is preserved for next cleaning */
  Biddy_PurgeAndReorder(NULL); /* keep only preserved formulae (init, eq), perform reordering */
  Biddy_Clean(); /* eq remains usable but not preserved */
  last = eq;
  eq1 = NextEqPart1(eq,tr_p,tr_q,...);
  eq2 = NextEqPart2(eq,tr_p,tr_q,...);
  eq = AND(eq1,eq2);
} while (AND(init,eq)!=0 && eq!=last)
if (AND(init,eq)!=0) return false; else return true;
Biddy_DeleteFormula("init"); /* optional, if you really need to remove init */
Biddy_Purge(); /* optional, immediately remove non-preserved nodes */
~~~

The fifth example is an outline of an implementation of parallel composition
where we are trying to benefit from intensive GC:

~~~
sacc = snew = AND(init_1,init_2,...,init_N);
for (i=1;i<=N;i++) di[i] = 0;
for (i=1;i<=N;i++) for (j=1;i<=N;j++) dij[i,j] = 0;
do {
  Biddy_AddTmpFormula(snew,N*(N+1)); /* snew is preserved just long enough */
  Biddy_AddTmpFormula(sacc,N*(N+1)); /* sacc is preserved just long enough */
  new1 = 0;
  for (i=1;i<=N;i++) {
    sup = OneStep(snew,tr_i,...);
    di[i] = OR(d[i],sup);
    new1 = OR(new1,NextState(sup,...));
    Biddy_AddTmpFormula(d[i],N*(N+1)); /* di[i] is preserved just long enough */
    Biddy_AddTmpFormula(new1,1); /* new1 is preserved for next cleaning */
    Biddy_Clean(); /* new1 remains usable but not preserved */
  }
  Biddy_AddTmpFormula(new1,N*N); /* new1 is preserved just long enough */
  new2 = 0;
  for (i=1;i<=N;i++) for (j=1;j<=N;j++) {
    sup = OneStep(snew,tr_i,tr_j,...);
    dij[i,j] = OR(d[i,j],sup);
    new2 = OR(new2,NextState(sup,...);
    Biddy_AddTmpFormula(dij[i,j],N*(N+1)); /* dij[i,j] is preserved just long enough */
    Biddy_AddTmpFormula(new2,1); /* new2 is preserved for next cleaning */
    Biddy_Clean(); /* new2 remains usable but not preserved */
  }
  snew = AND(OR(new1,new2),NOT(sacc));
  sacc = OR(sacc,snew);
} while (snew!=0)
tr1 = 0;
for (i=1;i<=N;i++) {
  sup = AddStab(di[i],...);
  tr1 = OR(tr1,sup); 
  Biddy_AddTmpFormula(tr1,1); /* tr1 is preserved for next cleaning */
  Biddy_Clean(); /* tr1 remains usable but not preserved */
}
Biddy_AddTmpFormula(tr1,N*N); /* tr1 is preserved just long enough */
tr2 = 0;
for (i=1;i<=N;i++) for (j=1;j<=N;j++) {
  sup = AddStab(dij[i,j],...);
  tr2 = OR(tr2,sup);
  Biddy_AddTmpFormula(tr2,1); /* tr2 is preserved for next cleaning */
  Biddy_Clean(); /* tr2 remains usable but not preserved */
}
result = OR(tr1,tr2);
Biddy_AddPersistentFormula("result",result); /* final result is permanently preserved */
Biddy_Clean(); /* temp results are not needed, anymore */
~~~

### 3.3 GARBAGE COLLECTION WITH A SYSTEM AGE

Garbage collection is automatically triggered if nodes from all reserved
blocks of nodes are used. Garbage collection will remove as many
obsolete nodes as possible.

Biddy does not use reference counter.
We call the implemented algorithm "GC with a system age".
It is a variant of a mark-and-sweep approach.
Please note, that it relies on additional structure (Formula table).

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

- __fortified__ node (expiry value = 0);
- __fresh__ node (expiry value = biddySystemAge);
- __prolonged__ node (expiry value > biddySystemAge);
- __obsolete__ node (0 < expiry value < biddySystemAge).

Before GC, nodes must be refreshed in such a way that no successor of
a non-obsolete node is obsolete. This can be achieved with relative simple
loop which will check each node at least once.

There are three internal functions to maintain nodes.

- __BiddyProlongOne__ (one node is prolonged)
- __BiddyProlongRecursively__ (one node and its successors are prolonged recursively until a non-obsolete node is reached)
- __BiddyIncSystemAge__ (all fresh nodes become obsolete nodes)

There are four functions which can be used by an expert user.

- __Biddy_Garbage__ (explicite garbage collection call, removes all obsolete nodes)
- __Biddy_SwapWithLower__ (explicite swap of two variables, removes all obsolete and fresh nodes)
- __Biddy_SwapWithHigher__ (explicite swap of two variables, removes all obsolete and fresh nodes)
- __Biddy_Sifting__ (explicite sifting, removes all obsolete and fresh nodes)

These four functions will keep all nodes preserved by Biddy_AddFormula and
they will not change the class of any node. Please note, that Biddy_Garbage
can be started in any time whilst Biddy_SwapWithLower, Biddy_SwapWithHigher,
and Biddy_Sifting will break an ongoing calculation (because they remove
fresh nodes).

### 3.4 MORE DETAILS OF MEMORY MANAGEMENT (NODE CHAINING)

Biddy relies on a single hash table for all variables. However, it supports
chaining of nodes to form different lists (using an extra pointer in
each node). This facility is used to improve efficiency of garbage
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
We use pacman to prepare the environment:

~~~
MSYS shell> pacman -Syuu
MSYS shell> pacman -S mingw-w64-i686-gcc
MSYS shell> pacman -S mingw-w64-x86_64-gcc
MSYS shell> pacman -S make
MSYS shell> pacman -S bison
MSYS shell> pacman -S gdb
MSYS shell> pacman -S nano
MSYS shell> pacman -S tar
MSYS shell> pacman -S subversion
~~~

Alternatively, you can use Visual Studio for building.
There is a prepared solution consisting of many projects.
You need to adapt include and lib folders.

~~~
./VS/Biddy.sln
~~~

To produce nice setup files, we use Advanced Installer
(http://www.advancedinstaller.com/).
We have been granted a free licence. MANY THANKS!

### Dependencies

On GNU/Linux, we are using libgmp (https://gmplib.org/).

On MS Windows, we are using MPIR library (http://mpir.org/).

### Creating Biddy library as a zip package

~~~
biddy> ./package-bin
~~~

You need a zip program.

On MS Windows, you  need 7-Zip (http://7-zip.org/) - and it has a strange
use of -x! You also   need   file   7zsd_All_x64.sfx  that   you   should 
download  as part of "7z SFX Tools" from http://7zsfx.info/en/ and put
in the directory containing 7z.exe.

You install the resulting package by extracting libraries to the
appropriate directory (may be local, e.g. user's home directory).

When using this package on GNU/Linux, you have to tell bash about the library:

~~~
export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:/absolute/path/to/biddy/library
~~~

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
bddscout> ./package-bin
bddscout> ./package-tgz
bddscout> ./package-deb
bddscout> ./package-rpm
~~~

package-bin should  create BDD Scout  (staticaly linked with Biddy
library). The script  will produce a zip  file. You install BDD  Scout
by simply unzip to the target directory.

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
be obtained from http://research.meolic.com/papers/robdd.pdf

In 1995, this BDD package was  rewritten in C. Later, this BDD package
become an  integral part of EST  package, a prototype  tool for formal
verification of  concurrent systems.  The homepage for  EST project is
http://est.meolic.com/

In 2003, BDD package from EST  was included in the report presented at
16th Symposium  on Integrated Circuits and  Systems Design (SBCCI'03).
The report is available as a paper "G.  Janssen.  A Consumer Report on
BDD Packages.  IBM T.J.  Watson  Research Center.  2003".  Get it from
http://doi.ieeecomputersociety.org/10.1109/SBCCI.2003.1232832

In 2006, BDD package in EST got the name Biddy.

In 2007, a main part of Biddy package was extracted from EST forming a
separate package called Biddy. The code has been reorganized in such a
way,  that EST  is not  using  internal structures  (e.g. node  table)
directly but using the provided API only.

In 2007,  we created local  svn repository for maintaining  the source
code (not live, anymore).

On  May 15,  2008, Biddy  v1.0  was  released containing  also bddview
v0.95 (Tcl/Tk BDD viewer) and Bdd Scout v0.90 (demo application).

In 2009, 2010, and 2012 an updated version of Biddy v1.0 was  released
which added support for debian packaging, support for  RPM  packaging,
fix error in Biddy_E and Biddy_A,  and  fix  and  improve  details  of
documentation, packaging, and Tcl/Tk GUI.

In 2012, a research paper about Biddy library appears in
Journal of Software (doi:10.4304/jsw.7.6.1358-1366)
http://ojs.academypublisher.com/index.php/jsw/article/view/jsw070613581366/

In 2013, Biddy  v1.1  was released. Biddy_Edge became a pointer instead
of structure and all other structures were optimized.

In 2014, Biddy  v1.2  was released. Variable swapping and sifting algorithm
were the most significant additions.

In 2014, svn repositories for biddy, bddview and bddscout are moved
to Savannah. http://svn.savannah.nongnu.org/viewvc/?root=biddy

In 2015, Biddy  v1.3, v.1.4 and v1.5 was released. Various input/output methods
have been added. Support for 64-bit arhitectures and support for Visual Studio
projects were improved. Biddy got a manager. Many CUDD-like functions have been
added. Comment's style changed to support doxygen. HTML and PDF documentation
were produced.

Also in 2015, Personal Package Archive ppa:meolic/biddy has been created
https://launchpad.net/~meolic/+archive/ubuntu/biddy

Also in 2015, sources became available on GitHub
https://github.com/meolic/biddy

In 2016, Biddy v1.6 was released. Formulae are not recursively refreshed
all the time. The size of Node table became resizable.

In 2017, Biddy v1.7.1 was released. Terminology is changed a lot,
e.g. "formulae counter" is now "system age". Added support for
0-sup-BDDs and TZBDDs. Implemented creation and manipulaton of
non-anonymous managers. Added manipulation of combination sets.
Sifting does not require removing fresh nodes.
Many new CUDD-like functions have been added.
bddview and BDD Scout have been significantly improved.

### 6. PUBLICATIONS
---------------

If you find our work useful, please, cite us.

- Robert Meolic. __Biddy - a multi-platform academic BDD package.__
  Journal of Software, 7(6), pp. 1358-1366, 2012.
  http://ojs.academypublisher.com/index.php/jsw/article/view/jsw070613581366

- Robert Meolic. __Biddy: ???__
  We are preparing a paper for SCP.

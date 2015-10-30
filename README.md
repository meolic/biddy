Biddy is a multi-platform academic Binary Decision Diagrams package.

Biddy is maintained by Robert Meolic (robert.meolic@um.si) at University
of Maribor, Slovenia.

Homepage: http://biddy.meolic.com/

1. AN OVERVIEW
--------------

Biddy is free software released under GPL.

Biddy uses ROBDDs with complement  edges as described in "K. S. Brace,
R.L.   Rudell,  R.  E.   Bryant.  Efficient  Implementation  of a  BDD
Package. 27. ACM/IEEE DAC, pages 40-45, 1990."

Biddy includes automatic garbage collection with a formulae counter,
node management with formulae tagging, and variable swapping.
Sifting algorithm is implemented.

Biddy is optimized for efficiency, but it is  mainly oriented towards
readable and  comprehensible source  code in C.

Biddy  is  currently  used  in  the following  projects:
- BDD Scout, demo project
- Efficient Symbolic Tools (EST), http://est.meolic.com/

2. SOURCE CODE
--------------

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
- biddyMain.c (main functions)
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

There are  two C headers,  external and internal. The  external header
file,  named  biddy.h, defines  features  visible  from outside  the
package. The internal header file, named biddyInt.h defines features
used in multiple files inside the package, but not outside.

There  are two  aditional packages  included into  Biddy distribution:
bddview and Bdd Scout.

bddview is  a pure Tcl/Tk  script for visualization of  BDDs.

BDD Scout is a demo  application demonstrating the capability of Biddy
and bddview.

To compile Biddy library, you can use:

~~~
biddy> make dynamic "BINDIR = ./bin"
biddy> make clean "BINDIR = ./bin"
~~~

More details about building are given in Section 4.

3. USING BIDDY LIBRARY
----------------------

Precompiled packages include dynamically linked library
(i.e. *.so on GNU/Linux, *.dll on MS Windows, *.dylib on Mac OS X),
and the appropriate C header biddy.h. Currently, there are no interfaces
for other programming languages.

Biddy is capable of all the typical operations regarding
Boolean functions and BDDs.

The following code is an example of usage.
Please note, that functions for node management are not shown. Moreover,
Biddy has a manager but its usage is optional and it is also not shown in
the given example.

IMPORTANT:
You should define UNIX, MACOSX, or WINDOWS.
You should define USE_BIDDY iff you are using Biddy via dynamic library.

```
\code
#define UNIX
#define USE_BIDDY
#include "/path/to/biddy.h"

#define Str2Var(x) (Biddy_GetTopVariable(Biddy_FoaVariable((Biddy_String)x)))

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
\endcode
```

### 3.1 NODE MANAGEMENT WITH FORMULAE TAGGING

Biddy includes powerful node management based on formulae tagging.
There are seven user functions to maintain nodes.

### Biddy_AddFormula(name,bdd,count)

Nodes of the given BDD will be preserved for the given number of cleanings.
If (name != NULL) then formula is accessible by its name. If formula with
a given name already exists it is overwritten. If (count == 0) then
formula is persistently preserved and you have to use Biddy_DeleteFormula
and Biddy_ClearAll to remove its nodes. There are two macros defined to simplify
formulae management. Macro Biddy_AddTmpFormula(bdd,count) is defined as
Biddy_AddFormula(NULL,bdd,count) and macro Biddy_AddPersistentFormula(name,bdd)
is defined as Biddy_AddFormula(name,bdd,0).

### Biddy_DeleteFormula(name)

Nodes of the given formula are tagged as not needed.
Formula is not accessible by its name anymore.
Regular cleaning with Biddy_Clean is not considered this tag.
Obsolete nodes from preserved formulae can be immediately removed
in explicite call to Biddy_ClearAll (this is the only way to remove
obsolete nodes from permanently preserved formulae).

### Biddy_Clean()

Performs cleaning.
Discard all nodes which were not preserved or which are not preserved
anymore. Obsolete nodes are not immediately removed, they will be removed
during the first garbage collection. Use Biddy_Purge or Biddy_PurgeAndReorder
to immediately remove all nodes which are not preserved.

### Biddy_Purge()

Immediately remove all nodes which were not preserved or which are not
preserved anymore. Call to Biddy_Result does not count as cleaning and thus all
preserved formulae remains preserved for the same number of cleanings. 

### Biddy_PurgeAndReorder(bdd)

The same as Biddy_Purge but also trigger reordering on function
(if BDD is given) or global reordering (if NULL is given).

### Biddy_Refresh(bdd)

All obsolete nodes become fresh nodes. This is an external variant of
internal function BiddyRefresh. It is needed to implement user caches.

### Biddy_ClearAll()

Immediately remove all nodes except the permanently preserved ones
which were not made obsolete by Biddy_DeleteFormula.
Please note, that regularly use of Biddy_Clean will trigger automatic garbage
collection, thus Biddy_ClearAll is needed only if you need more extensive
cleaning, e.g. to remove all obsolete nodes after a call to Biddy_DeleteFormula.

### 3.2. EXAMPLES OF NODE MANAGEMENT WITH BIDDY

The first example is a straightforward calculation.

```
\code
f1 = op(...);
f2 = op(...);
g1 = op(f1,f2,...);
Biddy_AddTmpFormula(g1,1); /* g1 is preserved for next cleaning */
f1 = op(...);
f2 = op(...);
g2 = op(f1,f2,...);
Biddy_AddTmpFormula(g2,1); /* g2 is preserved for next cleaning */
Biddy_Clean(); /* g1 and g2 are still usable */
result = op(g1,g2,...);
Biddy_AddPersistentFormula("result",result); /* final result is permanently preserved */
Biddy_Clean(); /* tmp result are not needed, anymore */
\endcode
```

If garbage collection is needed also after the calculation of g1, you must
add the following code after the calculation of g1:

```
\code
Biddy_AddTmpFormula(g1,2); /* g1 is preserved for next two cleanings */
Biddy_Clean(); /* g1 remains preserved for one additional cleaning */
\endcode
```

To trigger reordering in the first example, you should add calls to
Biddy_PurgeAndReorder to get the following code:

```
\code
f1 = op(...);
f2 = op(...);
g1 = op(f1,f2,...);
Biddy_AddTmpFormula(g1,2); /* g1 is preserved for next two cleanings */
Biddy_PurgeAndReorder(g1); /* keep only preserved formulae (g1), perform reordering on g1 */
Biddy_Clean(); /* g1 remains preserved for one additional cleaning */
f1 = op(...);
f2 = op(...);
g2 = op(f1,f2,...);
Biddy_AddTmpFormula(g2,1); /* g2 is preserved for next cleaning */
Biddy_PurgeAndReorder(g2); /* keep only preserved formulae (g1, g2), perform reordering on g2 */
Biddy_Clean(); /* g1 and g2 are still usable but not preserved */
result = op(g1,g2,...);
Biddy_AddPersistentFormula("result",result); /* result is permanently preserved */
Biddy_PurgeAndReorder(result); /* keep only preserved formulae (result), perform reordering on result */
\endcode
```

The second example is an iterative calculation:

```
\code
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
Biddy_Clean(); /* tmp results are not needed, anymore */
\endcode
```

If garbage collection is needed also after the calculation of g, you must
use the following code:

```
\code
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
Biddy_Clean(); /* tmp results are not needed, anymore */
\endcode
```

To trigger reordering in the second example, you should change code in the following way:

```
\code
f = op(...);
result = op(f,...);
while (!finish) {
  Biddy_AddTmpFormula(result,2); /* result is preserved for next two cleanings */
  Biddy_PurgeAndReorder(result); /* keep only preserved formulae (result), perform reordering on result */
  Biddy_Clean(); /* result remains preserved for one additional cleaning */
  f = op(...);
  g = op(f,...);
  Biddy_AddTmpFormula(g,1); /* g is preserved for next cleaning */
  Biddy_Clean(); /* result and g are still usable but not preserved */
  result = op(result,g,...);
}
Biddy_AddPersistentFormula("result",result); /* final result is permanently preserved */
Biddy_PurgeAndReorder(result); /* keep only preserved formulae (result), perform reordering on result */
\endcode
```

The third example is an outline of an implementation of model checking where
we are trying to benefit from regularly reordering:

```
\code
sup = Prepare(...);
Biddy_AddPersistentFormula("sup",sup) /* sup is permanently preserved */
Z = 0;
last = 1;
while (Z!=last) {
  Biddy_AddTmpFormula(Z,1); /* Z is preserved for next cleaning */
  Biddy_PurgeAndReorder(Z); /* keep only preserved formulae (sup, Z), perform reordering on Z */
  Biddy_Clean(); /* Z remains usable but not preserved */
  last = Z;
  Z = NextSet(Z,sup,...);
}
result = Z;
Biddy_AddPersistentFormula("result",result); /* final result is permanently preserved */
Biddy_DeleteFormula("sup");
Biddy_ClearAll(); /* optional, if you really need to immediately remove sup */
\endcode
```

The fourth example is an outline of an implementation of bisimulation
where we are trying to benefit from regularly reordering:

```
\code
init = AND(init_p,init_q)
Biddy_AddPersistentFormula("init",init) /* init is permanently preserved */
eq = InitialEq(init_p,tr_p,init_q,tr_q,...);
do {
  Biddy_AddTmpFormula(eq,1); /* eq is preserved for next cleaning */
  Biddy_PurgeAndReorder(eq); /* keep only preserved formulae (init, eq), perform reordering on eq */
  Biddy_Clean(); /* eq remains usable but not preserved */
  last = eq;
  eq1 = NextEqPart1(eq,tr_p,tr_q,...);
  eq2 = NextEqPart2(eq,tr_p,tr_q,...);
  eq = AND(eq1,eq2);
} while (AND(init,eq)!=0 && eq!=last)
if (AND(init,eq)!=0) return false; else return true;
Biddy_DeleteFormula("init");
Biddy_ClearAll(); /* optional, if you really need to immediately remove init */
\endcode
```

The fifth example is an outline of an implementation of parallel composition
where we are trying to benefit from intensive GC:

```
\code
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
Biddy_ClearAll(); /* optional, tmp result are not needed, anymore */
\endcode
```

### 3.3 GARBAGE COLLECTION WITH A FORMULAE COUNTER

Garbage collection is automatically triggered if nodes from all reserved
blocks of nodes are used. Garbage collection will remove as many
obsolete nodes as possible.

Biddy does not use reference counter.
We call the implemented algorithm "GC with a formulae counter".
Please note, that formulae tagging as presented above can
be implemented via reference counter or via formulae counter,
in both case it relies on additional structures, e.g. formula table.

While using formulae counter instead of reference counter is less
efficient (i.e. it requires more work to maintain nodes) it has
some advantages. It allows GC to be started in any time
without breaking the ongoing calculation. Thus, there is no need
to taking care of repeating broken calculations. Moreover,
the usage of formulae counter instead of reference counter
removes all the hassle of referencing and dereferencing nodes
and thus it is favorable in an academic package oriented towards
simple and readable source code.

There are four classes of nodes. Every node belongs to one of
these classes:

- __fortified__ node (count = 0);
- __fresh__ node (count = biddyCount);
- __prolonged__ node (count > biddyCount);
- __obsolete__ node (0 < count < biddyCount).

All successors of a fortified node are fortified nodes.
All successors of a prolonged node are prolonged or fortified nodes.
All successors of a fresh node are fresh, prolonged, or fortified nodes.

There are four internal functions to maintain nodes.

- __BiddyFortiy__ (all nodes become fortified nodes)
- __BiddyRefresh__ (all obsolete nodes become fresh nodes)
- __BiddyProlong__ (all obsolete and fresh nodes become prolonged nodes)
- __BiddyIncCounter__ (all prolonged nodes become fresh nodes, all fresh nodes become obsolete nodes)

There are four functions which can be used by an expert user.

- __Biddy_Garbage__ (explicite garbage collection, remove all obsolete nodes)
- __Biddy_SwapWithLower__ (explicite swap of two variables, it removes all obsolete and fresh nodes)
- __Biddy_SwapWithHigher__ (explicite swap of two variables, it removes all obsolete and fresh nodes)
- __Biddy_Sifting__ (explicite sifting, it removes all obsolete and fresh nodes)

These four functions will keep all nodes preserved by Biddy_AddFormula
they will not change the class of any node. Please note, that Biddy_Garbage
can be started in any time whilst Biddy_SwapWithLower, Biddy_SwapWithHigher,
and Biddy_Sifting will break an ongoing calculation (because they remove
fresh nodes).

### 3.4 MORE DETAILS OF MEMORY MANAGEMENT (NODE CHAINING)

Biddy relies on a single hash table for all variables. However, it supports
chaining of nodes to form different lists (ather is an extra pointer in
each node). This facility is used to improve efficiency of garbage
collection and sifting.

Please node, that node chaining is not determined or limited by
using formulae tagging or GC with a formulae counter, it is 
an independent mechanism.

4. BUILDING PACKAGES
--------------------

### Compiling Biddy library

~~~
biddy> make "BINDIR = ./bin"
biddy> make clean "BINDIR = ./bin"
~~~

Alternatively, you can use:

~~~
biddy> make static "BINDIR = ./bin"
biddy> make dynamic "BINDIR = ./bin"
biddy> make debug "BINDIR = ./bin"
~~~

### Creating Biddy library as a zip package

~~~
biddy> ./package-bin
~~~

The same, but using specific gcc options (Linux, only)

~~~
biddy> export CFLAGS=-m64;./package-bin 
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

### Create create zip file with source code of complete Biddy project

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

5. HISTORY
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

In 2015, Biddy  v1.3  was released. Various input/output methods have been
added. Support for 64-bit arhitectures and support for Visual Studio projects
were improved.

Also in 2015, Personal Package Archive ppa:meolic/biddy has been created
https://launchpad.net/~meolic/+archive/ubuntu/biddy

Also in 2015, Biddy  v1.4  was released. Biddy got a manager. Many CUDD-like
functions have been added. Comment's style changed to support doxygen which
allows producing HTML and PDF documentation.

6. PUBLICATIONS
---------------

If you find our work useful, please, cite us.

- Robert Meolic. __Biddy - a multi-platform academic BDD package.__
  Journal of Software, 7(6), pp. 1358-1366, 2012.
  http://ojs.academypublisher.com/index.php/jsw/article/view/jsw070613581366

- Robert Meolic. __Biddy: An academic BDD package.__
  We are preparing a paper for SCP.

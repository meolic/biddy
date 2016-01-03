/**CFile*******************************************************************
  PackageName [Bdd Scout]
  Synopsis    [Bdd Scout]

  FileName    [bddtraces-cmd.c]
  Revision    [$Revision: 119 $]
  Date        [$Date: 2015-12-24 10:59:37 +0100 (čet, 24 dec 2015) $]
  Authors     [Robert Meolic (robert.meolic@um.si)]
  Description []
  SeeAlso     [bddscout.h]

  Copyright   [This file is part of Bdd Scout package.
               Copyright (C) 2008, 2015 UM-FERI
               UM-FERI, Smetanova ulica 17, SI-2000 Maribor, Slovenia

               Bdd Scout is free software; you can redistribute it and/or modify
               it under the terms of the GNU General Public License as
               published by the Free Software Foundation; either version 2
               of the License, or (at your option) any later version.

               Bdd Scout is distributed in the hope that it will be useful,
               but WITHOUT ANY WARRANTY; without even the implied warranty of
               MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
               GNU General Public License for more details.

               You should have received a copy of the GNU General Public
               License along with this program; if not, write to the Free
               Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
               Boston, MA 02110-1301 USA.]
  ************************************************************************/

#define USEBIDDY
#define NOCUDD

/* COMPILE WITH: */
/* gcc -DUNIX -o bddtraces-cmd bddtraces-cmd.c -I../ -L../bin -lbiddy -lgmp */
/* gcc -o bddtraces-cmd bddtraces-cmd.c -I ../../cudd-2.5.1/include/ -L ../../cudd-2.5.1/cudd/ -L ../../cudd-2.5.1/util/ -L ../../cudd-2.5.1/mtr/ -L ../../cudd-2.5.1/st/ -L ../../cudd-2.5.1/epd/ -lcudd -lutil -lepd -lmtr -lst -lm */

#define NODEBUG

#ifdef USEBIDDY
#  include "biddy.h"
#  define BDDNULL NULL
#  include "./bddtraces-BIDDY.c"
#endif

#ifdef USECUDD
#  include <stdio.h>
#  include <stdlib.h>
#  include <stdint.h>
#  include <string.h>
#  include <ctype.h>
#  include <stdarg.h>
#  include <math.h>
#  include "cudd.h"
#  define BDDNULL NULL
#  ifndef TRUE
#    define TRUE (0 == 0)
#  endif
#  ifndef FALSE
#    define FALSE !TRUE
#  endif
typedef char *Biddy_String;
typedef char Biddy_Boolean;
typedef DdNode *Biddy_Edge;
DdManager *manager;
#  include "./bddtraces-CUDD.c"
#endif

int main(int argc, char *argv[])
{
  char *s1;
  FILE *funfile;
  FILE *statfile;
  char *report;

  if (argc != 2) {
    printf("wrong # args\n");
    return 1;
  }
  
  s1 = strdup(argv[1]);

  funfile = fopen(s1,"r");
  if (!funfile) {
    printf("bddtraces-cmd: File error (%s)!\n",s1);
  }

#ifdef USEBIDDY
  Biddy_Init();
  report = BddscoutRunBddTrace(funfile);
#endif

#ifdef USECUDD
  manager = Cudd_Init(0,0,CUDD_UNIQUE_SLOTS,CUDD_CACHE_SLOTS,0);
  report = BddscoutRunBddTrace(funfile);
#endif
  
  printf("\nREPORT\n");
  printf("%s\n",report);

  fclose(funfile);
  free(s1);

#ifdef USEBIDDY
  statfile = fopen("statBIDDY.txt","w");
  if (!statfile) {
    printf("bddtraces-cmd: Stat file error!\n");
  }
  Biddy_PrintInfo(statfile);
  fclose(statfile);
#endif

#ifdef USECUDD
  statfile = fopen("statCUDD.txt","w");
  if (!statfile) {
    printf("bddtraces-cmd: Stat file error!\n");
  }
  Cudd_PrintInfo(manager,statfile);
  fclose(statfile);
#endif

}

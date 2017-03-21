/**CFile*******************************************************************
  PackageName [Bdd Scout]
  Synopsis    [Bdd Scout]

  FileName    [bddscoutBenchmark.c]
  Revision    [$Revision: 263 $]
  Date        [$Date: 2017-03-21 09:49:54 +0100 (tor, 21 mar 2017) $]
  Authors     [Robert Meolic (robert.meolic@um.si)]
  Description [File bddscoutBenchmark.c is used for profiling.]
  SeeAlso     []

  Copyright   [This file is part of Bdd Scout package.
               Copyright (C) 2008, 2017 UM-FERI
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

/* COMPILE WITH: */
/* gcc -DUNIX -DUSE_BIDDY -O0 -g3 -o bddscoutBenchmark bddscoutBenchmark.c -I.. -L../bin -lbiddy -lgmp */

/* PROFILE WITH: */
/*
perf record ./bddscoutBenchmark
perf report
*/

#define BDDTYPE BIDDYTYPEOBDD

#define LIST \
    "cath/add1.be " \
    "cath/add2.be " \
    "cath/add3.be " \
    "cath/add4.be " \
    "cath/addsub.be " \
    "cath/alu.be " \
    "" \
    "ex/ex2.be " \
    "ex/mul03.be " \
    "ex/mul04.be " \
    "ex/mul05.be " \
    "ex/mul06.be " \
    "ex/mul07.be " \
    "ex/mul08.be " \
    "ex/rip02.be " \
    "ex/rip04.be " \
    "ex/rip06.be " \
    "ex/rip08.be " \
    "ex/transp.be " \
    "ex/ztwaalf1.be " \
    "ex/ztwaalf2.be " \
    "" \
    "hachtel/alupla20.be " \
    "hachtel/alupla21.be " \
    "hachtel/alupla22.be " \
    "hachtel/alupla23.be " \
    "hachtel/alupla24.be " \
    "hachtel/dc2.be " \
    "hachtel/dk17.be " \
    "hachtel/dk27.be " \
    "hachtel/f51m.be " \
    "hachtel/misg.be " \
    "hachtel/mlp4.be " \
    "hachtel/rd73.be " \
    "hachtel/risc.be " \
    "hachtel/root.be " \
    "hachtel/sqn.be " \
    "hachtel/vg2.be " \
    "hachtel/x1dn.be " \
    "hachtel/x6dn.be " \
    "hachtel/z4.be " \
    "hachtel/z5xpl.be " \
    "hachtel/z9sym.be " \
    "" \
    "plasco/counter.be " \
    "plasco/d3.be " \
    "plasco/hostint1.be " \
    "plasco/in1.be " \
    "plasco/mp2d.be " \
    "plasco/mul.be " \
    "plasco/pitch.be " \
    "plasco/rom2.be " \
    "plasco/table.be " \
    "plasco/werner.be " \

#define BDDSCOUTPROFILE
#include "bddscoutIFIP.c"

int main(int argc, char *argv[]) {
  Biddy_String list,s1,s2;
  FILE *funfile;
  Biddy_String report;
  char *saveptr;

  Biddy_InitAnonymous(BDDTYPE);

  list = strdup(LIST);
  s2 = malloc(255);

  s1 = strtok_r(list," ",&saveptr);
  while (s1) {
    sprintf(s2,"/home/meolic/est/biddy/bddscout/IFIP/%s",s1);
    printf("%s\n",s2);
    funfile = fopen(s2,"r");
    if (!funfile) {
      printf("File error (%s)!\n",s2);
    } else {
      report = BddscoutBenchmarkIFIP(funfile,s1);
      printf("%s\n",report);
      free(report);
      fclose(funfile);
    }
    s1 = strtok_r(NULL," ",&saveptr);
  }
  
  free(list);
  free(s2);

  Biddy_Exit();
}

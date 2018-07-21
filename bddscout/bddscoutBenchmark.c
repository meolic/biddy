/**CFile*******************************************************************
  PackageName [Bdd Scout]
  Synopsis    [Bdd Scout]

  FileName    [bddscoutBenchmark.c]
  Revision    [$Revision: 451 $]
  Date        [$Date: 2018-06-26 12:07:56 +0200 (tor, 26 jun 2018) $]
  Authors     [Robert Meolic (robert.meolic@um.si)]
  Description [File bddscoutBenchmark.c is used for profiling.]
  SeeAlso     []

  Copyright   [This file is part of Bdd Scout package.
               Copyright (C) 2008, 2018 UM FERI
               UM FERI, Koroska cesta 46, SI-2000 Maribor, Slovenia

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

/* IF BIDDY IS STATICALLY LINKED COMPILE WITH: */
/* gcc -DUNIX -O0 -g3 -o bddscoutBenchmark bddscoutBenchmark.c bddscout.c -I.. -L../bin -lbiddy -lgmp */

/* IF BIDDY IS DYNAMICALLY LINKED COMPILE WITH: */
/* gcc -DUNIX -DUSE_BIDDY -O0 -g3 -o bddscoutBenchmark bddscoutBenchmark.c bddscout.c -I.. -L../bin -lbiddy -lgmp */

/* PROFILE WITH: */
/*
perf record ./bddscoutBenchmark
perf report
*/

#define BDDSCOUTPROFILE
#include "bddscout.h"
#include "bddscoutIFIP.c"

Biddy_Manager MNGROBDD,MNGROBDDCE,MNGZBDD,MNGZBDDCE,MNGTZBDD;

#define BDDTYPE BIDDYTYPEOBDDC

#define IFIPDIR "./IFIP"

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

void doDEBUG1 ()
{
  Biddy_Edge f;
  unsigned int idx;

  f = Biddy_Managed_Eval2(MNGTZBDD,"a + b");
  Biddy_Managed_AddPersistentFormula(MNGTZBDD,"F",f);

  Biddy_Managed_FindFormula(MNGTZBDD,"F",&idx,&f);
  Biddy_Managed_PrintfBDD(MNGTZBDD,f);
  printf("\n");

  Biddy_Managed_PrintfSOP(MNGTZBDD,f);

  Biddy_Managed_CopyFormula(MNGTZBDD,MNGZBDD,"F");

  Biddy_Managed_FindFormula(MNGZBDD,"F",&idx,&f);
  Biddy_Managed_PrintfBDD(MNGZBDD,f);
  printf("\n");

  Biddy_Managed_PrintfSOP(MNGZBDD,f);
}

void doDEBUG2 ()
{
  Biddy_Edge f,g;
  unsigned int idx;

  f = Biddy_Managed_Eval2(MNGTZBDD,"a + b");
  Biddy_Managed_AddPersistentFormula(MNGTZBDD,"F",f);
  g = Biddy_Managed_Eval2(MNGTZBDD,"~a");
  Biddy_Managed_AddPersistentFormula(MNGTZBDD,"G",g);

  Biddy_Managed_FindFormula(MNGTZBDD,"F",&idx,&f);
  Biddy_Managed_PrintfBDD(MNGTZBDD,f);
  Biddy_Managed_PrintfSOP(MNGTZBDD,f);
  Biddy_Managed_WriteDot(MNGTZBDD,"f.dot",f,"F",-1,FALSE);
  Biddy_Managed_WriteBddview(MNGTZBDD,"f.bddview",f,"F",NULL);

  Biddy_Managed_FindFormula(MNGTZBDD,"G",&idx,&g);
  Biddy_Managed_PrintfBDD(MNGTZBDD,g);
  Biddy_Managed_PrintfSOP(MNGTZBDD,g);
  Biddy_Managed_WriteDot(MNGTZBDD,"g.dot",g,"G",-1,FALSE);
  Biddy_Managed_WriteBddview(MNGTZBDD,"g.bddview",g,"G",NULL);
}

void doIFIP ()
{
  Biddy_String list,s1,s2;
  FILE *funfile;
  Biddy_String report;
  char *saveptr;

  list = strdup(LIST);
  s2 = malloc(255);

  s1 = strtok_r(list," ",&saveptr);
  while (s1) {
    sprintf(s2,"%s/%s",IFIPDIR,s1);
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
}

int main(int argc, char *argv[])
{
  Biddy_InitMNG(&MNGROBDD,BIDDYTYPEOBDD);
  Biddy_InitMNG(&MNGROBDDCE,BIDDYTYPEOBDDC);
  Biddy_InitMNG(&MNGZBDD,BIDDYTYPEZBDD);
  Biddy_InitMNG(&MNGZBDDCE,BIDDYTYPEZBDDC);
  Biddy_InitMNG(&MNGTZBDD,BIDDYTYPETZBDD);
  Biddy_InitAnonymous(BDDTYPE);

  doDEBUG2();
  /* doIFIP(); */

  Biddy_ExitMNG(&MNGROBDD);
  Biddy_ExitMNG(&MNGROBDDCE);
  Biddy_ExitMNG(&MNGZBDD);
  Biddy_ExitMNG(&MNGZBDDCE);
  Biddy_ExitMNG(&MNGTZBDD);
  Biddy_Exit();
}

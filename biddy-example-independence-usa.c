/* $Revision: 545 $ */
/* $Date: 2019-02-11 14:07:50 +0100 (pon, 11 feb 2019) $ */
/* This file (biddy-example-independence-usa.c) is a C file */
/* Author: Robert Meolic (robert@meolic.com) */
/* This file has been released into the public domain by the author. */

/* data are obtained from Donald E. Knuth, http://www-cs-faculty.stanford.edu/~uno/sgb.html */
/* direct link: http://www-cs-faculty.stanford.edu/~uno/contiguous-*usa.dat */

/* CODES IN ALPHABETICAL ORDER */
#define CODES "AL,AR,AZ,CA,CO,CT,DC,DE,FL,GA,IA,ID,IL,IN,KS,KY,LA,MA,MD,ME,MI,MN,MO,MS,MT,NC,ND,NE,NH,NJ,NM,NV,NY,OH,OK,OR,PA,RI,SC,SD,TN,TX,UT,VA,VT,WA,WI,WV,WY"

/* THIS LIST MUST BE IN THE SAME (ALPHABETICAL) ORDER */
#define AL 0
#define AR 1
#define AZ 2
#define CA 3
#define CO 4
#define CT 5
#define DC 6
#define DE 7
#define FL 8
#define GA 9
#define IA 10
#define ID 11
#define IL 12
#define IN 13
#define KS 14
#define KY 15
#define LA 16
#define MA 17
#define MD 18
#define ME 19
#define MI 20
#define MN 21
#define MO 22
#define MS 23
#define MT 24
#define NC 25
#define ND 26
#define NE 27
#define NH 28
#define NJ 29
#define NM 30
#define NV 31
#define NY 32
#define OH 33
#define OK 34
#define OR 35
#define PA 36
#define RI 37
#define SC 38
#define SD 39
#define TN 40
#define TX 41
#define UT 42
#define VA 43
#define VT 44
#define WA 45
#define WI 46
#define WV 47
#define WY 48

#  include <stdio.h>
#  include <stdlib.h>
#  include <stdint.h>
#  include <string.h>
#  include <ctype.h>
#  include <stdarg.h>

void setDataUSA(unsigned int *size, unsigned int ***usa, unsigned int **order, char **codes) {
  
  unsigned int i,j;

  /* initialorder1: ALPHABETICAL ORDER - THERE ARE 306214 NODES (306213 IF USING COMPLEMENT EDGES) IN BDD FOR INDEPENDENCE SETS */
  /* initialorder2: FROM D.E.KNUTH - THERE ARE 339 NODES (338 IF USING COMPLEMENT EDGES) IN BDD FOR INDEPENDENCE SETS */
  /* initialorder3: THE BEST OBTAINED FROM initialorder1 BY BIDDY USING REPEATED SIFTING ON SYSTEM - THERE ARE 860 NODES (859 IF USING COMPLEMENT EDGES) IN BDD FOR INDEPENDENCE SETS */
  /* initialorder4: THE BEST OBTAINED FROM initialorder1 BY BIDDY USING REPEATED SIFTING ON FUNCTION - THERE ARE 859 NODES (858 IF USING COMPLEMENT EDGES) IN BDD FOR INDEPENDENCE SETS */
  /* initialorder5: THE BEST OBTAINED FROM initialorder1 BY CUDD USING REPEATED SIFTING ON SYSTEM - THERE ARE 521 NODES (USING COMPLEMENT EDGES) IN BDD FOR INDEPENDENCE SETS */

  unsigned int initialorder1[] = {AL,AR,AZ,CA,CO,CT,DC,DE,FL,GA,IA,ID,IL,IN,KS,KY,LA,MA,MD,ME,MI,MN,MO,MS,MT,NC,ND,NE,NH,NJ,NM,NV,NY,OH,OK,OR,PA,RI,SC,SD,TN,TX,UT,VA,VT,WA,WI,WV,WY};
  unsigned int initialorder2[] = {OR,ID,NV,WA,AZ,CA,UT,NM,WY,CO,MT,SD,MN,ND,IA,NE,OK,KS,TX,MO,LA,AR,MS,WI,KY,MI,IN,IL,AL,TN,FL,NC,SC,GA,WV,OH,MD,DC,VA,PA,NJ,DE,NY,CT,RI,NH,ME,VT,MA};
  unsigned int initialorder3[] = {AL,GA,FL,TN,NC,SC,VA,MS,AR,TX,LA,OK,KY,NM,MO,NE,KS,CO,IA,IL,WI,MI,IN,SD,MN,MT,ND,UT,WY,OH,PA,WV,DC,NJ,MD,DE,NY,VT,NH,ME,RI,CT,MA,NV,ID,WA,CA,AZ,OR};
  unsigned int initialorder4[] = {AL,GA,FL,TN,NC,SC,VA,MS,AR,TX,LA,OK,KY,NM,MO,NE,KS,CO,IA,IL,WI,MI,IN,SD,MN,MT,ND,UT,WY,OH,PA,WV,DC,NJ,MD,DE,NY,VT,NH,ME,RI,CT,MA,ID,CA,AZ,OR,WA,NV};
  unsigned int initialorder5[] = {GA,AL,FL,TN,NC,SC,MS,VA,AR,TX,LA,OK,NM,NE,KS,CO,AZ,UT,ID,CA,OR,WA,NV,WY,SD,IA,ND,MT,MN,IL,MO,KY,MI,WI,IN,WV,DC,PA,OH,MD,NJ,DE,NY,RI,CT,NH,ME,VT,MA};

  *size = 49;

  /* THIS LIST MUST BE IN ALPHABETICAL ORDER */
  *codes = strdup(CODES);

  *order = (unsigned int *) malloc((*size)*sizeof(unsigned int));
  for (i=0; i<(*size); i++) {
    (*order)[i] = initialorder1[i];
  }

  *usa = (unsigned int **) malloc((*size)*sizeof(unsigned int *));
  for (i=0; i<(*size); i++) {
    (*usa)[i] = (unsigned int *) malloc((*size)*sizeof(unsigned int));
  }

  for (i=0; i<(*size); i++) {
    for (j=0; j<(*size); j++) {
      (*usa)[i][j] = 0;
    }
  }

  (*usa)[AL][FL]=1;
  (*usa)[AL][GA]=1;
  (*usa)[AL][MS]=1;
  (*usa)[AL][TN]=1;
  (*usa)[AR][LA]=1;
  (*usa)[AR][MO]=1;
  (*usa)[AR][MS]=1;
  (*usa)[AR][OK]=1;
  (*usa)[AR][TN]=1;
  (*usa)[AR][TX]=1;
  (*usa)[AZ][CA]=1;
  (*usa)[AZ][NM]=1;
  (*usa)[AZ][NV]=1;
  (*usa)[AZ][UT]=1;
  (*usa)[CA][NV]=1;
  (*usa)[CA][OR]=1;
  (*usa)[CO][KS]=1;
  (*usa)[CO][NE]=1;
  (*usa)[CO][NM]=1;
  (*usa)[CO][OK]=1;
  (*usa)[CO][UT]=1;
  (*usa)[CO][WY]=1;
  (*usa)[CT][MA]=1;
  (*usa)[CT][NY]=1;
  (*usa)[CT][RI]=1;
  (*usa)[DC][MD]=1;
  (*usa)[DC][VA]=1;
  (*usa)[DE][MD]=1;
  (*usa)[DE][NJ]=1;
  (*usa)[DE][PA]=1;
  (*usa)[FL][GA]=1;
  (*usa)[GA][NC]=1;
  (*usa)[GA][SC]=1;
  (*usa)[GA][TN]=1;
  (*usa)[IA][IL]=1;
  (*usa)[IA][MN]=1;
  (*usa)[IA][MO]=1;
  (*usa)[IA][NE]=1;
  (*usa)[IA][SD]=1;
  (*usa)[IA][WI]=1;
  (*usa)[ID][MT]=1;
  (*usa)[ID][NV]=1;
  (*usa)[ID][OR]=1;
  (*usa)[ID][UT]=1;
  (*usa)[ID][WA]=1;
  (*usa)[ID][WY]=1;
  (*usa)[IL][IN]=1;
  (*usa)[IL][KY]=1;
  (*usa)[IL][MO]=1;
  (*usa)[IL][WI]=1;
  (*usa)[IN][KY]=1;
  (*usa)[IN][MI]=1;
  (*usa)[IN][OH]=1;
  (*usa)[KS][MO]=1;
  (*usa)[KS][NE]=1;
  (*usa)[KS][OK]=1;
  (*usa)[KY][MO]=1;
  (*usa)[KY][OH]=1;
  (*usa)[KY][TN]=1;
  (*usa)[KY][VA]=1;
  (*usa)[KY][WV]=1;
  (*usa)[LA][MS]=1;
  (*usa)[LA][TX]=1;
  (*usa)[MA][NH]=1;
  (*usa)[MA][NY]=1;
  (*usa)[MA][RI]=1;
  (*usa)[MA][VT]=1;
  (*usa)[MD][PA]=1;
  (*usa)[MD][VA]=1;
  (*usa)[MD][WV]=1;
  (*usa)[ME][NH]=1;
  (*usa)[MI][OH]=1;
  (*usa)[MI][WI]=1;
  (*usa)[MN][ND]=1;
  (*usa)[MN][SD]=1;
  (*usa)[MN][WI]=1;
  (*usa)[MO][NE]=1;
  (*usa)[MO][OK]=1;
  (*usa)[MO][TN]=1;
  (*usa)[MS][TN]=1;
  (*usa)[MT][ND]=1;
  (*usa)[MT][SD]=1;
  (*usa)[MT][WY]=1;
  (*usa)[NC][SC]=1;
  (*usa)[NC][TN]=1;
  (*usa)[NC][VA]=1;
  (*usa)[ND][SD]=1;
  (*usa)[NE][SD]=1;
  (*usa)[NE][WY]=1;
  (*usa)[NH][VT]=1;
  (*usa)[NJ][NY]=1;
  (*usa)[NJ][PA]=1;
  (*usa)[NM][OK]=1;
  (*usa)[NM][TX]=1;
  (*usa)[NV][OR]=1;
  (*usa)[NV][UT]=1;
  (*usa)[NY][PA]=1;
  (*usa)[NY][VT]=1;
  (*usa)[OH][PA]=1;
  (*usa)[OH][WV]=1;
  (*usa)[OK][TX]=1;
  (*usa)[OR][WA]=1;
  (*usa)[PA][WV]=1;
  (*usa)[SD][WY]=1;
  (*usa)[TN][VA]=1;
  (*usa)[UT][WY]=1;
  (*usa)[VA][WV]=1;

  for (i=0; i<(*size); i++) {
    for (j=0; j<(*size); j++) {
      if ((*usa)[j][i]) (*usa)[i][j]=1;
    }
  }

}

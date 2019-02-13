/* $Revision: 545 $ */
/* $Date: 2019-02-11 14:07:50 +0100 (pon, 11 feb 2019) $ */
/* This file (biddy-example-independence-europe.c) is a C file */
/* Author: Robert Meolic (robert@meolic.com) */
/* This file has been released into the public domain by the author. */

/* data were composed by Robert Meolic, 2016 */

/* CODES IN ALPHABETICAL ORDER */
#define CODES "AD,AL,AT,AZ,BA,BE,BG,BY,CH,CZ,DE,DK,EE,ES,FI,FR,GB,GE,GR,HR,HU,IE,IT,KZ,LI,LT,LU,LV,MC,MD,ME,MK,NL,NO,PL,PT,RO,RS,RU,SE,SI,SK,SM,TR,UA,VA"

/* THIS LIST MUST BE IN THE SAME (ALPHABETICAL) ORDER */
#define AD 0
#define AL 1
#define AT 2
#define AZ 3
#define BA 4
#define BE 5
#define BG 6
#define BY 7
#define CH 8
#define CZ 9
#define DE 10
#define DK 11
#define EE 12
#define ES 13
#define FI 14
#define FR 15
#define GB 16
#define GE 17
#define GR 18
#define HR 19
#define HU 20
#define IE 21
#define IT 22
#define KZ 23
#define LI 24
#define LT 25
#define LU 26
#define LV 27
#define MC 28
#define MD 29
#define ME 30
#define MK 31
#define NL 32
#define NO 33
#define PL 34
#define PT 35
#define RO 36
#define RS 37
#define RU 38
#define SE 39
#define SI 40
#define SK 41
#define SM 42
#define TR 43
#define UA 44
#define VA 45

#  include <stdio.h>
#  include <stdlib.h>
#  include <stdint.h>
#  include <string.h>
#  include <ctype.h>
#  include <stdarg.h>

void setDataEurope(unsigned int *size, unsigned int ***europe, unsigned int **order, char **codes) {
  
  unsigned int i,j;

  /* initialorder1: ALPHABETICAL ORDER - THERE ARE xxxxxx NODES (xxxxxx IF USING COMPLEMENT EDGES) IN BDD FOR INDEPENDENCE SETS */

  unsigned int initialorder1[] = {AD,AL,AT,AZ,BA,BE,BG,BY,CH,CZ,DE,DK,EE,ES,FI,FR,GB,GE,GR,HR,HU,IE,IT,KZ,LI,LT,LU,LV,MC,MD,ME,MK,NL,NO,PL,PT,RO,RS,RU,SE,SI,SK,SM,TR,UA,VA};

  *size = 46;

  /* THIS LIST MUST BE IN ALPHABETICAL ORDER */
  *codes = strdup(CODES);

  *order = (unsigned int *) malloc((*size)*sizeof(unsigned int));
  for (i=0; i<(*size); i++) {
    (*order)[i] = initialorder1[i];
  }

  *europe = (unsigned int **) malloc((*size)*sizeof(unsigned int *));
  for (i=0; i<(*size); i++) {
    (*europe)[i] = (unsigned int *) malloc((*size)*sizeof(unsigned int));
  }

  for (i=0; i<(*size); i++) {
    for (j=0; j<(*size); j++) {
      (*europe)[i][j] = 0;
    }
  }

  
  (*europe)[AD][ES]=1;
  (*europe)[AD][FR]=1;
  (*europe)[AL][GR]=1;
  (*europe)[AL][ME]=1;
  (*europe)[AL][MK]=1;
  (*europe)[AL][RS]=1;
  (*europe)[AT][CH]=1;
  (*europe)[AT][CZ]=1;
  (*europe)[AT][DE]=1;
  (*europe)[AT][HU]=1;
  (*europe)[AT][IT]=1;
  (*europe)[AT][LI]=1;
  (*europe)[AT][SI]=1;
  (*europe)[AT][SK]=1;
  (*europe)[AZ][GE]=1;
  (*europe)[AZ][RU]=1;
  (*europe)[BA][HR]=1;
  (*europe)[BA][ME]=1;
  (*europe)[BA][RS]=1;
  (*europe)[BE][DE]=1;
  (*europe)[BE][FR]=1;
  (*europe)[BE][LU]=1;
  (*europe)[BE][NL]=1;
  (*europe)[BG][GR]=1;
  (*europe)[BG][MK]=1;
  (*europe)[BG][RO]=1;
  (*europe)[BG][RS]=1;
  (*europe)[BG][TR]=1;
  (*europe)[BY][LT]=1;
  (*europe)[BY][LV]=1;
  (*europe)[BY][PL]=1;
  (*europe)[BY][RU]=1;
  (*europe)[BY][UA]=1;
  (*europe)[CH][DE]=1;
  (*europe)[CH][FR]=1;
  (*europe)[CH][IT]=1;
  (*europe)[CH][LI]=1;
  (*europe)[CZ][DE]=1;
  (*europe)[CZ][PL]=1;
  (*europe)[CZ][SK]=1;
  (*europe)[DE][DK]=1;
  (*europe)[DE][FR]=1;
  (*europe)[DE][LU]=1;
  (*europe)[DE][NL]=1;
  (*europe)[DE][PL]=1;
  (*europe)[EE][LV]=1;
  (*europe)[EE][RU]=1;
  (*europe)[ES][FR]=1;
  (*europe)[ES][PT]=1;
  (*europe)[FI][NO]=1;
  (*europe)[FI][RU]=1;
  (*europe)[FI][SE]=1;
  (*europe)[FR][IT]=1;
  (*europe)[FR][LU]=1;
  (*europe)[FR][MC]=1;
  (*europe)[GB][IE]=1;
  (*europe)[GE][RU]=1;
  (*europe)[GE][TR]=1;
  (*europe)[GR][MK]=1;
  (*europe)[GR][TR]=1;
  (*europe)[HR][HU]=1;
  (*europe)[HR][ME]=1;
  (*europe)[HR][RS]=1;
  (*europe)[HR][SI]=1;
  (*europe)[HU][RO]=1;
  (*europe)[HU][RS]=1;
  (*europe)[HU][SI]=1;
  (*europe)[HU][SK]=1;
  (*europe)[HU][UA]=1;
  (*europe)[IT][SI]=1;
  (*europe)[IT][SM]=1;
  (*europe)[IT][VA]=1;
  (*europe)[KZ][RU]=1;
  (*europe)[LT][LV]=1;
  (*europe)[LT][PL]=1;
  (*europe)[LT][RU]=1;
  (*europe)[LV][RU]=1;
  (*europe)[MD][RO]=1;
  (*europe)[MD][UA]=1;
  (*europe)[ME][RS]=1;
  (*europe)[MK][RS]=1;
  (*europe)[NO][RU]=1;
  (*europe)[NO][SE]=1;
  (*europe)[PL][RU]=1;
  (*europe)[PL][SK]=1;
  (*europe)[PL][UA]=1;
  (*europe)[RO][RS]=1;
  (*europe)[RO][UA]=1;
  (*europe)[RU][UA]=1;
  (*europe)[SE][FI]=1;
  (*europe)[SE][NO]=1;
  (*europe)[SK][UA]=1;

  for (i=0; i<(*size); i++) {
    for (j=0; j<(*size); j++) {
      if ((*europe)[j][i]) (*europe)[i][j]=1;
    }
  }

}

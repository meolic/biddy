/* $Revision: 148 $ */
/* $Date: 2016-03-08 22:03:25 +0100 (tor, 08 mar 2016) $ */
/* This file (biddy-example-independence-test.c) is a C file */
/* Author: Robert Meolic (robert.meolic@um.si) */
/* This file has been released into the public domain by the author. */

/* CODES IN ALPHABETICAL ORDER */
#define CODES "N1,N2,N3,N4,N5,N6"

/* THIS LIST MUST BE IN THE SAME (ALPHABETICAL) ORDER */
#define N1 0
#define N2 1
#define N3 2
#define N4 3
#define N5 4
#define N6 5

#  include <stdio.h>
#  include <stdlib.h>
#  include <stdint.h>
#  include <string.h>
#  include <ctype.h>
#  include <stdarg.h>

void setDataTEST(unsigned int *size, unsigned int ***data, unsigned int **order, char **codes) {
  
  unsigned int i,j;
  unsigned int initialorder1[] = {N1,N2,N3,N4,N5,N6};
  unsigned int initialorder2[] = {N2,N4,N6,N1,N3,N5};

  *size = 6;

  /* THIS LIST MUST BE IN ALPHABETICAL ORDER */
  *codes = strdup(CODES);

  *order = (unsigned int *) malloc((*size)*sizeof(unsigned int));
  for (i=0; i<(*size); i++) {
    (*order)[i] = initialorder1[i];
  }

  *data = (unsigned int **) malloc((*size)*sizeof(unsigned int *));
  for (i=0; i<(*size); i++) {
    (*data)[i] = (unsigned int *) malloc((*size)*sizeof(unsigned int));
  }

  for (i=0; i<(*size); i++) {
    for (j=0; j<(*size); j++) {
      (*data)[i][j] = 0;
    }
  }

  (*data)[N1][N2]=1;
  (*data)[N1][N4]=1;
  (*data)[N1][N6]=1;
  (*data)[N2][N3]=1;
  (*data)[N2][N6]=1;
  (*data)[N3][N4]=1;
  (*data)[N3][N5]=1;
  (*data)[N4][N5]=1;
  (*data)[N5][N6]=1;

  for (i=0; i<(*size); i++) {
    for (j=0; j<(*size); j++) {
      if ((*data)[j][i]) (*data)[i][j]=1;
    }
  }

}

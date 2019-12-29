/* $Revision: 558 $ */
/* $Date: 2019-10-14 09:42:56 +0200 (pon, 14 okt 2019) $ */
/* This file (biddy-example-bra-manager.c) is a C file */
/* Author: Robert Meolic (robert@meolic.com) */
/* This file has been released into the public domain by the author. */

/* COMPILE WITH (ADD -lgmp IF USING STATIC BIDDY LIBRARY): */
/* gcc -DUNIX -O2 -o biddy-example-bra-manager biddy-example-bra-manager.c -I. -L./bin */

/* MAXNODES IS FOR OBDD AND ZBDD, THEY ARE ALWAYS THE LARGEST AMONG THE IMPLEMENTED BDD TYPES */
/* 0var = 2 functions, 1 ordering, MAXNODES = 1 */
/* 1var = 4 functions, 1 ordering, MAXNODES = 3 */
/* 2var = 16 functions, 2 orderings, MAXNODES = 5 */
/* 3var = 256 functions, 6 orderings, MAXNODES = 7 */
/* 4var = 65536 functions, 24 orderings, MAXNODES = 11 */
/* 5var = 4294967296 functions, 120 orderings, MAXNODES = 19 */
#define MAXNODES 19

#include <string.h>
#include <time.h>
#include "biddy.h"

void statByFileCreate(char bddtype[], unsigned int N, unsigned int QUANT);
void statByFileSum(char bddtype[], unsigned int N);
void trim(Biddy_String source);
unsigned long long int rnd8(unsigned long long int x);
unsigned long long int rnd16(unsigned long long int x);
unsigned long long int rnd32(unsigned long long int x);
unsigned long long int rnd8lfsr(unsigned long long int x);
unsigned long long int rnd16lfsr(unsigned long long int x);
void testrnd();

int main(int argc, char *argv[]) {
  Biddy_String bddtype;
  unsigned int varnum,quant;

  bddtype = strdup("OBDDCE");
  varnum = 3;

  /* how many Boolean functions will be included into one file */
  /* if you put to much into one file, it can take many days for it to be processed */
  /* if you put to less into one file, you can get a huge number of files */
  quant = 1000000;

  if (argc == 1) {
    printf("USAGE: biddy-example-bra-manager <create>|<sum>\n");
    free(bddtype);
    return 0;
  }
  if (!strcmp(argv[1],"create")) {
    statByFileCreate(bddtype,varnum,quant);
    printf("1. Use \"biddy-example-bra\" to calculate statistics for all generate files.\n");
    printf("2. Use \"biddy-example-bra-manager sum\" to collect all calculated statistics.\n");
  }
  if (!strcmp(argv[1],"sum")) {
    statByFileSum(bddtype,varnum);
  }

  free(bddtype);
}

/* ########################################################################## */
/* function statByFileCreate */
/* ########################################################################## */

void statByFileCreate(char bddtype[], unsigned int N, unsigned int QUANT) {
  FILE *fd;
  char filename[32];
  unsigned int i,seq;
  unsigned long long int nf,n,quant,rndseed;
  unsigned long long int tts;

  n = N;
  nf = 1;
  while (n>0) {
    nf = 2 * nf;
    n--;
  }
  n = nf;
  nf = 1;
  while (n>0) {
    nf = 2 * nf;
    n--;
  }
  n = 0;
  quant = QUANT;
  rndseed = 0;
  tts = 0;

  printf("statByFileCreate...\n");

  seq = 0;
  do {

    sprintf((char *)filename,"FILE-%s-%.2u-%u.TXT",bddtype,N,seq);
    printf("(%s)",filename);
    fd = fopen(filename,"w");
    if (!fd) return;
    fprintf(fd,"%s\n",bddtype);
    fprintf(fd,"SIZE %u\n",N);
    fprintf(fd,"NF %llu\n",nf);
    fprintf(fd,"N %llu\n",n);
    fprintf(fd,"QUANT %llu\n",quant);
    fprintf(fd,"SEED %llu\n",rndseed);
    fprintf(fd,"TIME %llu\n",tts);
    fprintf(fd,"STATSCE\n");
    for (i = 1; i <= MAXNODES; i++) {
      fprintf(fd,"NODES=%.2u, MINCE=%llu, MAXCE=%llu, BASECE=%llu, ALLCE=%llu\n",i,0ULL,0ULL,0ULL,0ULL);
    }
    fprintf(fd,"STATS\n");
    for (i = 1; i <= MAXNODES; i++) {
      fprintf(fd,"NODES=%.2u, MIN=%llu, MAX=%llu, BASE=%llu, ALL=%llu\n",i,0ULL,0ULL,0ULL,0ULL);
    }
    fclose(fd);

    i=0;
    do {
      i++;
      n++;
      if (N == 2) {
        if (++rndseed == 16) rndseed = 0;
      }
      else if (N == 3) {
        rndseed = rnd8(rndseed);
      }
      else if (N == 4) {
        rndseed = rnd16(rndseed);
      }
      else if (N == 5) {
        rndseed = rnd32(rndseed);
      } else {
        printf("N == %u IS NOT SUPPORTED!\n",N);
        exit(1);
      }
    } while ( (rndseed!=0) && (i<QUANT) );

    seq++;

  } while (rndseed!=0);

  printf("\n");
}

/* ########################################################################## */
/* function statByFileSum */
/* this will generate a file which can be read into R
/* ########################################################################## */

void statByFileSum(char bddtype[], unsigned int N) {
  FILE *fd;
  char filename[32];
  char bddtypeplain[32];
  Biddy_String buffer,word;
  unsigned int i,x,seq;
  unsigned long long int nf,n;
  unsigned long long int tts;
  Biddy_Boolean STOP;

  /* MIN[n] IS A NUMBER OF FUNCTIONS, */
  /* WHICH HAVE n NODES USING THE BEST ORDERING */
  unsigned long long int *MIN,*MINCE;

  /* MAX[n] IS A NUMBER OF FUNCTIONS, */
  /* WHICH HAVE n NODES USING THE WORST ORDERING */
  unsigned long long int *MAX,*MAXCE;

  /* BASE[n] IS A NUMBER OF FUNCTIONS, */
  /* WHICH HAVE n NODES USING THE BASIC ORDERING a,b,c,... */
  unsigned long long int *BASE,*BASECE;

  /* ALL[n] IS A NUMBER OF FUNCTIONS, */
  /* WHICH HAVE n NODES IN ANY OF THE POSSIBLE ORDERINGS */
  unsigned long long int *ALL,*ALLCE;

  MIN = (unsigned long long int *) malloc((MAXNODES+1) * sizeof(unsigned long long int));
  MAX = (unsigned long long int *) malloc((MAXNODES+1) * sizeof(unsigned long long int));
  BASE = (unsigned long long int *) malloc((MAXNODES+1) * sizeof(unsigned long long int));
  ALL = (unsigned long long int *) malloc((MAXNODES+1) * sizeof(unsigned long long int));

  MINCE = (unsigned long long int *) malloc((MAXNODES+1) * sizeof(unsigned long long int));
  MAXCE = (unsigned long long int *) malloc((MAXNODES+1) * sizeof(unsigned long long int));
  BASECE = (unsigned long long int *) malloc((MAXNODES+1) * sizeof(unsigned long long int));
  ALLCE = (unsigned long long int *) malloc((MAXNODES+1) * sizeof(unsigned long long int));

  if (!strcmp(bddtype,"OBDDCE")) {
    strcpy(bddtypeplain,"OBDD");
  }
  else if (!strcmp(bddtype,"ZBDDCE")) {
    strcpy(bddtypeplain,"ZBDD");
  }
  else if (!strcmp(bddtype,"TZBDDCE")) {
    strcpy(bddtypeplain,"TZBDD");
  } else {
    strcpy(bddtypeplain,bddtype);
  }

  n = N;
  nf = 1;
  while (n>0) {
    nf = 2 * nf;
    n--;
  }
  n = nf;
  nf = 1;
  while (n>0) {
    nf = 2 * nf;
    n--;
  }
  tts = 0;

  /* INIT */
  for (i = 1; i <= MAXNODES; i++) {
    MIN[i] = MAX[i] = BASE[i] = ALL[i] = 0LLU;
    MINCE[i] = MAXCE[i] = BASECE[i] = ALLCE[i] = 0LLU;
  }

  printf("statByFileSum...\n");

  seq = 0;
  STOP = FALSE;
  while (!STOP) {

    /* READ ONE FILE */
    buffer = (Biddy_String) malloc(255);
    sprintf((char *)filename,"FILE-%s-%.2u-%u.TXT",bddtype,N,seq);
    fd = fopen(filename,"r");
    if (fd) {
      printf("(%s)",filename);

      if (!fgets(buffer,255,fd)) exit(255);
      trim(buffer); /* to assure Win/Unix EOL independecy */
      if (strcmp(bddtype,buffer)) {
        printf("Wrong input format (BDDTYPE)!\n");
        printf("%s\n",buffer);
        exit(1);
      }

      if (!fgets(buffer,255,fd)) exit(255);
      trim(buffer); /* to assure Win/Unix EOL independency */
      word = strtok(buffer," ");
      if (strcmp(word,"SIZE")) {
        printf("Wrong input format (SIZE)!\n");
        printf("%s\n",buffer);
        exit(1);
      }
      word = strtok(NULL," ");
      sscanf(word,"%u",&x);
      if (x != N) {
        printf("Wrong SIZE!\n");
        printf("%s\n",buffer);
        exit(1);
      }

      if (!fgets(buffer,255,fd)) exit(255);
      trim(buffer); /* to assure Win/Unix EOL independency */
      word = strtok(buffer," ");
      if (strcmp(word,"NF")) {
        printf("Wrong input format (NF)!\n");
        printf("%s\n",buffer);
        exit(1);
      }
      word = strtok(NULL," ");
      sscanf(word,"%llu",&n);
      if (n != nf) {
        printf("Wrong NF!\n");
        printf("%s\n",buffer);
        exit(1);
      }

      if (!fgets(buffer,255,fd)) exit(255);
      trim(buffer); /* to assure Win/Unix EOL independency */
      word = strtok(buffer," ");
      if (strcmp(word,"N")) {
        printf("Wrong input format (N)!\n");
        printf("%s\n",buffer);
        exit(1);
      }
      word = strtok(NULL," ");
      sscanf(word,"%llu",&n);
      /* N is not used */

      if (!fgets(buffer,255,fd)) exit(255);
      trim(buffer); /* to assure Win/Unix EOL independency */
      word = strtok(buffer," ");
      if (strcmp(word,"QUANT")) {
        printf("Wrong input format (QUANT)!\n");
        printf("%s\n",buffer);
        exit(1);
      }
      word = strtok(NULL," ");
      sscanf(word,"%llu",&n);
      /* QUANT is not used */

      if (!fgets(buffer,255,fd)) exit(255);
      trim(buffer); /* to assure Win/Unix EOL independency */
      word = strtok(buffer," ");
      if (strcmp(word,"SEED")) {
        printf("Wrong input format (SEED)!\n");
        printf("%s\n",buffer);
        exit(1);
      }
      word = strtok(NULL," ");
      sscanf(word,"%llu",&n);
      /* SEED is not used */

      if (!fgets(buffer,255,fd)) exit(255);
      trim(buffer); /* to assure Win/Unix EOL independency */
      word = strtok(buffer," ");
      if (strcmp(word,"TIME")) {
        printf("Wrong input format (TIME)!\n");
        printf("%s\n",buffer);
        exit(1);
      }
      word = strtok(NULL," ");
      sscanf(word,"%llu",&n);
      tts += n; /* update total elapsed time */

      if (!fgets(buffer,255,fd)) exit(255);
      trim(buffer); /* to assure Win/Unix EOL independecy */
      if (strcmp(buffer,"STATSCE")) {
        printf("Wrong input format (STATCE)!\n");
        printf("%s\n",buffer);
        exit(1);
      }

      for (i = 1; i <= MAXNODES; i++) {
        if (!fgets(buffer,255,fd)) exit(255);
        trim(buffer); /* to assure Win/Unix EOL independecy */
        word = strtok(buffer,"= ");
        if (strcmp(word,"NODES")) {
          printf("Wrong input format (NODES)!\n");
          printf("%s\n",buffer);
          exit(1);
        }
        word = strtok(NULL,", ");
        sscanf(word,"%u",&x);
        if (x != i) {
          printf("Wrong input format (i=%u, x=%u)!\n",i,x);
          printf("%s\n",buffer);
          exit(1);
        }
        word = strtok(NULL,"= ");
        if (strcmp(word,"MINCE")) {
          printf("Wrong input format (MINCE)!\n");
          printf("%s\n",buffer);
          exit(1);
        }
        word = strtok(NULL,", ");
        sscanf(word,"%llu",&n);
        MINCE[i] += n; /* update stats for MINCE */
        word = strtok(NULL,"= ");
        if (strcmp(word,"MAXCE")) {
          printf("Wrong input format (MAXCE)!\n");
          printf("%s\n",buffer);
          exit(1);
        }
        word = strtok(NULL,", ");
        sscanf(word,"%llu",&n);
        MAXCE[i] += n; /* update stats for MAXCE */
        word = strtok(NULL,"= ");
        if (strcmp(word,"BASECE")) {
          printf("Wrong input format (BASECE)!\n");
          printf("%s\n",buffer);
          exit(1);
        }
        word = strtok(NULL,", ");
        sscanf(word,"%llu",&n);
        BASECE[i] += n; /* update stats for BASECE */
        word = strtok(NULL,"= ");
        if (strcmp(word,"ALLCE")) {
          printf("Wrong input format (ALLCE)!\n");
          printf("%s\n",buffer);
          exit(1);
        }
        word = strtok(NULL,", ");
        sscanf(word,"%llu",&n);
        ALLCE[i] += n; /* update stats for ALLCE */
      }

      if (!fgets(buffer,255,fd)) exit(255);
      trim(buffer); /* to assure Win/Unix EOL independecy */
      if (strcmp(buffer,"STATS")) {
        printf("Wrong input format!\n");
        printf("%s\n",buffer);
        exit(1);
      }

      for (i = 1; i <= MAXNODES; i++) {
        if (!fgets(buffer,255,fd)) exit(255);
        trim(buffer); /* to assure Win/Unix EOL independecy */
        word = strtok(buffer,"= ");
        if (strcmp(word,"NODES")) {
          printf("Wrong input format (NODES)!\n");
          printf("%s\n",buffer);
          exit(1);
        }
        word = strtok(NULL,", ");
        sscanf(word,"%u",&x);
        if (x != i) {
          printf("Wrong input format (i=%u, x=%u)!\n",i,x);
          printf("%s\n",buffer);
          exit(1);
        }
        word = strtok(NULL,"= ");
        if (strcmp(word,"MIN")) {
          printf("Wrong input format (MIN)!\n");
          printf("%s\n",buffer);
          exit(1);
        }
        word = strtok(NULL,", ");
        sscanf(word,"%llu",&n);
        MIN[i] += n; /* update stats for MIN */
        word = strtok(NULL,"= ");
        if (strcmp(word,"MAX")) {
          printf("Wrong input format (MAX)!\n");
          printf("%s\n",buffer);
          exit(1);
        }
        word = strtok(NULL,", ");
        sscanf(word,"%llu",&n);
        MAX[i] += n; /* update stats for MAX */
        word = strtok(NULL,"= ");
        if (strcmp(word,"BASE")) {
          printf("Wrong input format (BASE)!\n");
          printf("%s\n",buffer);
          exit(1);
        }
        word = strtok(NULL,", ");
        sscanf(word,"%llu",&n);
        BASE[i] += n; /* update stats for BASE */
        word = strtok(NULL,"= ");
        if (strcmp(word,"ALL")) {
          printf("Wrong input format (ALL)!\n");
          printf("%s\n",buffer);
          exit(1);
        }
        word = strtok(NULL,", ");
        sscanf(word,"%llu",&n);
        ALL[i] += n; /* update stats for ALL */
      }

      fclose(fd);
      free(buffer);

      seq++;

    } else {
      STOP = TRUE;
    }
  }

  printf("\n");
  printf("TOTAL TIME[s]: %llu\n",tts);


  /* WRITE SUM FILE FOR STAT */
  sprintf((char *)filename,"SUM-%s-%.2u.TXT",bddtypeplain,N);
  printf("Write file: %s\n",filename);
  fd = fopen(filename,"w");
  if (!fd) exit(1);
  fprintf(fd,"#Generated with Biddy (biddy.meolic.com)\n");
  fprintf(fd,"#%s-PLAIN\n",bddtype);
  fprintf(fd,"#SIZE: %u\n",N);
  fprintf(fd,"#NF: %llu\n",nf);
  fprintf(fd,"\n");
  fprintf(fd,"%15s ","NODES");
  fprintf(fd,"%15s ","MIN");
  fprintf(fd,"%15s ","MAX");
  fprintf(fd,"%15s ","BASE");
  fprintf(fd,"%15s","ALL");
  fprintf(fd,"\n");
  for (i = 1; i <= MAXNODES; i++) {
    fprintf(fd,"%15u %15llu %15llu %15llu %15llu\n",i,MIN[i],MAX[i],BASE[i],ALL[i]);
  }
  fclose(fd);

  /* WRITE SUM FILE FOR STATCE */
  sprintf((char *)filename,"SUM-%s-%.2u.TXT",bddtype,N);
  printf("Write file: %s\n",filename);
  fd = fopen(filename,"w");
  if (!fd) exit(1);
  fprintf(fd,"#Generated with Biddy (biddy.meolic.com)\n");
  fprintf(fd,"#%s\n",bddtype);
  fprintf(fd,"#SIZE: %u\n",N);
  fprintf(fd,"#NF: %llu\n",nf);
  fprintf(fd,"\n");
  fprintf(fd,"%15s ","NODES");
  fprintf(fd,"%15s ","MIN");
  fprintf(fd,"%15s ","MAX");
  fprintf(fd,"%15s ","BASE");
  fprintf(fd,"%15s","ALL");
  fprintf(fd,"\n");
  for (i = 1; i <= MAXNODES; i++) {
    fprintf(fd,"%15u %15llu %15llu %15llu %15llu\n",i,MINCE[i],MAXCE[i],BASECE[i],ALLCE[i]);
  }
  fclose(fd);
}

void
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
}

/* ########################################################################## */
/* RANDOM NUMBER GENERATORS */
/* ########################################################################## */

/* Xorshift */
/* Algorithm "xor" from p. 4 of Marsaglia, "Xorshift RNGs" */
/* https://en.wikipedia.org/wiki/Xorshift */
/* http://www.arklyffe.com/main/2010/08/29/xorshift-pseudorandom-number-generator/ */
/* 0 is always followed by 1 */
unsigned long long int rnd8(unsigned long long int x)
{
  if (x == 0) {
    x = 1;
  } else {
    x ^= (x << 7) & 0xffULL;
    x ^= (x >> 5) & 0xffULL;
    x ^= (x << 3) & 0xffULL;
    if (x == 1) x = 0;
  }
  return x;
}

/* Xorshift */
/* Algorithm "xor" from p. 4 of Marsaglia, "Xorshift RNGs" */
/* https://en.wikipedia.org/wiki/Xorshift */
/* http://www.arklyffe.com/main/2010/08/29/xorshift-pseudorandom-number-generator/ */
/* 0 is always followed by 1 */
unsigned long long int rnd16(unsigned long long int x)
{
  if (x == 0) {
    x = 1;
  } else {
    x ^= x << 13 & 0xffffULL;
    x ^= x >> 9 & 0xffffULL;
    x ^= x << 7 & 0xffffULL;
    if (x == 1) x = 0;
  }
  return x;
}

/* Xorshift */
/* Algorithm "xor" from p. 4 of Marsaglia, "Xorshift RNGs" */
/* https://en.wikipedia.org/wiki/Xorshift */
/* 0 is always followed by 1 */
unsigned long long int rnd32(unsigned long long int x)
{
  if (x == 0) {
    x = 1;
  } else {
    x ^= x << 13 & 0xffffffffULL;
    x ^= x >> 17 & 0xffffffffULL;
    x ^= x << 5 & 0xffffffffULL;
    if (x == 1) x = 0;
  }
  return x;
}

/* Fibonacci LFSR */
/* x^8 + x^6 + x^5 + x^4 + 1 */
/* https://en.wikipedia.org/wiki/Linear-feedback_shift_register */
/* 0 is always followed by 1 */
unsigned long long int rnd8lfsr(unsigned long long int x)
{
  unsigned long long int bit;

  if (x == 0) {
    x = 1;
  } else {
    bit = ((x >> 0) ^ (x >> 2) ^ (x >> 3) ^ (x >> 4) ) & 1;
    x = (x >> 1) | (bit << 7);
    if (x == 1) x = 0;
  }
  return x;
}

/* Fibonacci LFSR */
/* x^16 + x^14 + x^13 + x^11 + 1 */
/* https://en.wikipedia.org/wiki/Linear-feedback_shift_register */
/* 0 is always followed by 1 */
unsigned long long int rnd16lfsr(unsigned long long int x)
{
  unsigned long long int bit;

  if (x == 0) {
    x = 1;
  } else {
    bit = ((x >> 0) ^ (x >> 2) ^ (x >> 3) ^ (x >> 5) ) & 1;
    x = (x >> 1) | (bit << 15);
    if (x == 1) x = 0;
  }
  return x;
}

/* ########################################################################## */
/* TESTING */
/* ########################################################################## */

void
testrnd()
{
  unsigned long long int x;
  unsigned long long int n;

  /* RND8 */
  n = 0;
  x = 0;
  do {
    n++;
    x = rnd8(x);
  } while (x != 0);
  printf("N = %llu\n",n);

  /* RND16 */
  n = 0;
  x = 0;
  do {
    n++;
    x = rnd16(x);
  } while (x != 0);
  printf("N = %llu\n",n);

  /* RND32 */
  n = 0;
  x = 0;
  do {
    n++;
    x = rnd32(x);
  } while (x != 0);
  printf("N = %llu\n",n);

  /* RND8LFSR */
  n = 0;
  x = 0;
  do {
    n++;
    x = rnd8lfsr(x);
  } while (x != 0);
  printf("N = %llu\n",n);

  /* RND16LFSR */
  n = 0;
  x = 0;
  do {
    n++;
    x = rnd16lfsr(x);
  } while (x != 0);
  printf("N = %llu\n",n);
}

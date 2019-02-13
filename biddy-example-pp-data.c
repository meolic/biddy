/* $Revision: 545 $ */
/* $Date: 2019-02-11 14:07:50 +0100 (pon, 11 feb 2019) $ */
/* This file (biddy-example-pp-data.c) is a C file to be included into biddy-example-pp.c */
/* Author: Robert Meolic (robert@meolic.com) */
/* This file has been released into the public domain by the author. */

/* this example is about the production planning problem */
/* this file contains multiple benchmark systems */

/*
1. generateSystemTakahashi()

these data are from K. Takahashi, M. Onosato, F. Tanaka.
A comprehensive approach for managing feasible solutions in production
planning by an interacting network of Zero-Suppressed Binary Decision Diagrams.
Journal of Computational Design and Engineering, 2(2), 2015

a subset of this problem is published in K. Takahashi, M. Onosato, F. Tanaka.
Comprehensive representation of feasible combinations of alternatives
for dynamic production planning using Zero-Suppressed Binary Decision Diagram.
Journal of Advanced Mechanical Design, Systems, and Manufacturing, 8(4), 2014
*/

/*
2. generateSystemKacem4x5()

these data are from I. A. Chaudry, A. M. Khan, A. A. Khan.
A Genetic Algorithm for Flexible Job Shop Scheduling.
WCE 2013

original source of data
I. Kacem, S. Hammadi, P. Borne.
Pareto-optimality approach for flexible job-shop scheduling problems:
hybridization of evolutionary algorithms and fuzzy logic.
Mathematics and Computers in Simulation, 60 (2002) 245-276
*/

/*
3. generateSystemKacem8x8()

these data are from I. A. Chaudry, A. M. Khan, A. A. Khan.
A Genetic Algorithm for Flexible Job Shop Scheduling.
WCE 2013

original source of data
I. Kacem, S. Hammadi, P. Borne.
Approach by Localization and Multiobjective Evolutionary Optimization
for Flexible Job-Shop Scheduling Problems.
IEEE Transactions on Systems, Man, and Cybernetics, part C, 1(32), 2002
*/

/*
4. generateSystemKacem10x7()

these data are from I. Kacem, S. Hammadi, P. Borne.
Pareto-optimality approach for flexible job-shop scheduling problems:
hybridization of evolutionary algorithms and fuzzy logic.
Mathematics and Computers in Simulation, 60 (2002) 245-276
*/

/*
5. generateSystemKacem10x10()

these data are from I. Kacem, S. Hammadi, P. Borne.
Pareto-optimality approach for flexible job-shop scheduling problems:
hybridization of evolutionary algorithms and fuzzy logic.
Mathematics and Computers in Simulation, 60 (2002) 245-276
*/

/*
6. generateSystemKacem15x10()

these data are from I. Kacem, S. Hammadi, P. Borne.
Pareto-optimality approach for flexible job-shop scheduling problems:
hybridization of evolutionary algorithms and fuzzy logic.
Mathematics and Computers in Simulation, 60 (2002) 245-276
*/

/* ************************************************************************************ */
/* ************************************************************************************ */
/* ************************************************************************************ */

void
generateSystemTakahashi()
{
  unsigned int part,pnum,xnum,onum;
  unsigned int i,j;

  if (NUMPARTS == 0) NUMPARTS = 10;
  if (NUMMACHINES == 0) NUMMACHINES = 8;
  if (FACTORYCAPACITY == 0) FACTORYCAPACITY = 12;

  MAXSEQUENCELENGTH = 4;

  /* user defined parts, sequences, and operations start at index 1 */

  /* *************************************** */
  /* OPERATION TABLE */
  /* *************************************** */
  
  operationTableSize = 15; /* number of operations */
  operationTable = (OPERATION *) malloc((1 + operationTableSize) * sizeof(OPERATION));
  (operationTable)[0].name = NULL;
  for (i = 1; i <= operationTableSize; i++)  (operationTable)[i].idx = i;

  (operationTable)[1].name = strdup("O01");
  (operationTable)[2].name = strdup("O02");
  (operationTable)[3].name = strdup("O03");
  (operationTable)[4].name = strdup("O04");
  (operationTable)[5].name = strdup("O05");
  (operationTable)[6].name = strdup("O06");
  (operationTable)[7].name = strdup("O07");
  (operationTable)[8].name = strdup("O08");
  (operationTable)[9].name = strdup("O09");
  (operationTable)[10].name = strdup("O10");
  (operationTable)[11].name = strdup("O11");
  (operationTable)[12].name = strdup("O12");
  (operationTable)[13].name = strdup("O13");
  (operationTable)[14].name = strdup("O14");
  (operationTable)[15].name = strdup("O15");

  /* *************************************** */
  /* MACHINE TABLE */
  /* *************************************** */
  
  machineTableSize = 8; /* number of machines */
  machineTable = (MACHINE *) malloc((1 + machineTableSize) * sizeof(MACHINE));
  machineTable[0].name = NULL;
  for (i = 1; i <= machineTableSize; i++) machineTable[i].idx = i;

  machineTable[1].name = strdup("M1"); machineTable[1].n = 3;
  machineTable[2].name = strdup("M2"); machineTable[2].n = 1;
  machineTable[3].name = strdup("M3"); machineTable[3].n = 1;
  machineTable[4].name = strdup("M4"); machineTable[4].n = 2;
  machineTable[5].name = strdup("M5"); machineTable[5].n = 1;
  machineTable[6].name = strdup("M6"); machineTable[6].n = 1;
  machineTable[7].name = strdup("M7"); machineTable[7].n = 2;
  machineTable[8].name = strdup("M8"); machineTable[8].n = 1;

  if ((NUMMACHINES == 0) || (NUMMACHINES > machineTableSize)) {
    NUMMACHINES = machineTableSize;
  }

  /* *************************************** */
  /* OPERATION MATRIX */
  /* *************************************** */

  operationMatrix = (unsigned int *)
    malloc((1 + machineTableSize) * (1 + operationTableSize) * sizeof(unsigned int));

  for (i = 0; i <= operationTableSize; i++) {
    for (j = 0; j <= machineTableSize; j++) {
      (operationMatrix)[i + j * (1 + operationTableSize)] = 0; /* Oi @ Mj */
    }
  }

  operationMatrix[1 + 1 * (1 + operationTableSize)] = 6; /* O1 @ M1 */
  operationMatrix[1 + 2 * (1 + operationTableSize)] = 5; /* O1 @ M2 */
  operationMatrix[1 + 7 * (1 + operationTableSize)] = 3; /* O1 @ M7 */
  operationMatrix[2 + 2 * (1 + operationTableSize)] = 8; /* O2 @ M2 */
  operationMatrix[2 + 6 * (1 + operationTableSize)] = 4; /* O2 @ M6 */
  operationMatrix[3 + 3 * (1 + operationTableSize)] = 10; /* O3 @ M3 */
  operationMatrix[3 + 4 * (1 + operationTableSize)] = 8; /* O3 @ M4 */
  operationMatrix[4 + 1 * (1 + operationTableSize)] = 6; /* O4 @ M1 */
  operationMatrix[4 + 4 * (1 + operationTableSize)] = 4; /* O4 @ M4 */
  operationMatrix[5 + 1 * (1 + operationTableSize)] = 9; /* O5 @ M1 */
  operationMatrix[5 + 3 * (1 + operationTableSize)] = 8; /* O5 @ M3 */
  operationMatrix[5 + 5 * (1 + operationTableSize)] = 7; /* O5 @ M5 */
  operationMatrix[6 + 2 * (1 + operationTableSize)] = 5; /* O6 @ M2 */
  operationMatrix[6 + 4 * (1 + operationTableSize)] = 6; /* O6 @ M4 */
  operationMatrix[6 + 5 * (1 + operationTableSize)] = 5; /* O6 @ M5 */
  operationMatrix[7 + 1 * (1 + operationTableSize)] = 2; /* O7 @ M1 */
  operationMatrix[7 + 2 * (1 + operationTableSize)] = 4; /* O7 @ M2 */
  operationMatrix[7 + 6 * (1 + operationTableSize)] = 1; /* O7 @ M6 */
  operationMatrix[8 + 3 * (1 + operationTableSize)] = 6; /* O8 @ M3 */
  operationMatrix[9 + 4 * (1 + operationTableSize)] = 3; /* O9 @ M4 */
  operationMatrix[9 + 8 * (1 + operationTableSize)] = 5; /* O9 @ M8 */
  operationMatrix[10 + 6 * (1 + operationTableSize)] = 5; /* O10 @ M6 */
  operationMatrix[10 + 7 * (1 + operationTableSize)] = 2; /* O10 @ M7 */
  operationMatrix[11 + 3 * (1 + operationTableSize)] = 8; /* O11 @ M3 */
  operationMatrix[11 + 5 * (1 + operationTableSize)] = 5; /* O11 @ M5 */
  operationMatrix[11 + 8 * (1 + operationTableSize)] = 3; /* O11 @ M8 */
  operationMatrix[12 + 2 * (1 + operationTableSize)] = 4; /* O12 @ M2 */
  operationMatrix[12 + 6 * (1 + operationTableSize)] = 2; /* O12 @ M6 */
  operationMatrix[12 + 7 * (1 + operationTableSize)] = 1; /* O12 @ M7 */
  operationMatrix[13 + 1 * (1 + operationTableSize)] = 3; /* O13 @ M1 */
  operationMatrix[13 + 6 * (1 + operationTableSize)] = 2; /* O13 @ M6 */
  operationMatrix[13 + 8 * (1 + operationTableSize)] = 4; /* O13 @ M8 */
  operationMatrix[14 + 4 * (1 + operationTableSize)] = 10; /* O14 @ M4 */
  operationMatrix[14 + 8 * (1 + operationTableSize)] = 14; /* O14 @ M8 */
  operationMatrix[15 + 5 * (1 + operationTableSize)] = 8; /* O15 @ M5 */
  operationMatrix[15 + 7 * (1 + operationTableSize)] = 12; /* O15 @ M7 */

  /* *************************************** */
  /* PRODUCTION TABLE */
  /* *************************************** */

  productionTableSize = 10;
  productionTable = (PPART *) malloc((1 + productionTableSize) * sizeof(PPART));
  productionTable[0] = NULL;

  if ((NUMPARTS == 0) || (NUMPARTS > productionTableSize)) {
    NUMPARTS = productionTableSize;
  }

  /* *************************************** */
  /* PART TABLE */
  /* *************************************** */

  pnum = 0;
  part = 0;

  partTableSize = 21; /* number of parts (including subparts) */
  partTable = (PART *) malloc((1 + partTableSize) * sizeof(PART));
  partTable[0].name = NULL; partTable[0].n = 0; partTable[0].X = NULL;
  for (i = 1; i <= partTableSize; i++)  {
    partTable[i].idx = i;
    partTable[i].color = 0;
  }

  /* *************************************** */
  /* PART P1 */
  /* *************************************** */
  
  part++;

  pnum++; xnum = 0; onum = 0; /* next part, id = 1 */

  partTable[pnum].name = strdup("P1a");
  partTable[pnum].part = part;
  partTable[pnum].n = 2; /* number of sequences */
  partTable[pnum].code = NULL; partTable[pnum].feasible = NULL;
  partTable[pnum].X = (SEQUENCE *) malloc((1 + partTable[pnum].n) * sizeof(SEQUENCE));
  partTable[pnum].X[0].n = 0; partTable[pnum].X[0].O = NULL;

  xnum++; onum = 0; /* next sequence */

  partTable[pnum].X[xnum].n = 1; /* number of items for the 1st sequence in P1a */
  partTable[pnum].X[xnum].O = (SEQUENCEITEM *) malloc((1 + partTable[pnum].X[xnum].n) * sizeof(SEQUENCEITEM));
  partTable[pnum].X[xnum].O[0].type = NONE; partTable[pnum].X[xnum].O[0].ITEM = NULL;
  partTable[pnum].X[xnum].O[++onum].type = DEFAULT; partTable[pnum].X[xnum].O[onum].ITEM = (void *) &(operationTable[5]); /* O5 */

  xnum++; onum = 0; /* next sequence */

  partTable[pnum].X[xnum].n = 1; /* number of items for the 2nd sequence in P1a */
  partTable[pnum].X[xnum].O = (SEQUENCEITEM *) malloc((1 + partTable[pnum].X[xnum].n) * sizeof(SEQUENCEITEM));
  partTable[pnum].X[xnum].O[0].type = NONE; partTable[pnum].X[xnum].O[0].ITEM = NULL;
  partTable[pnum].X[xnum].O[++onum].type = DEFAULT; partTable[pnum].X[xnum].O[onum].ITEM = (void *) &(operationTable[1]); /* O1 */

  pnum++; xnum = 0; onum = 0; /* next part, id = 2 */

  partTable[pnum].name = strdup("P01");
  partTable[pnum].part = part;
  partTable[pnum].n = 2; /* number of sequences */
  partTable[pnum].code = NULL; partTable[pnum].feasible = NULL;
  partTable[pnum].X = (SEQUENCE *) malloc((1 + partTable[pnum].n) * sizeof(SEQUENCE));
  partTable[pnum].X[0].n = 0; partTable[pnum].X[0].O = NULL;

  xnum++; onum = 0; /* next sequence */

  partTable[pnum].X[xnum].n = 3; /* number of items for the 1st sequence in P1 */
  partTable[pnum].X[xnum].O = (SEQUENCEITEM *) malloc((1 + partTable[pnum].X[xnum].n) * sizeof(SEQUENCEITEM));
  partTable[pnum].X[xnum].O[0].type = NONE; partTable[pnum].X[xnum].O[0].ITEM = NULL;
  partTable[pnum].X[xnum].O[++onum].type = DEFAULT; partTable[pnum].X[xnum].O[onum].ITEM = (void *) &(operationTable[1]); /* O1 */
  partTable[pnum].X[xnum].O[++onum].type = DEFAULT; partTable[pnum].X[xnum].O[onum].ITEM = (void *) &(operationTable[2]); /* O2 */
  partTable[pnum].X[xnum].O[++onum].type = DEFAULT; partTable[pnum].X[xnum].O[onum].ITEM = (void *) &(operationTable[3]); /* O3 */

  xnum++; onum = 0; /* next sequence */

  partTable[pnum].X[xnum].n = 3; /* number of items for the 2nd sequence in P1 */
  partTable[pnum].X[xnum].O = (SEQUENCEITEM *) malloc((1 + partTable[pnum].X[xnum].n) * sizeof(SEQUENCEITEM));
  partTable[pnum].X[xnum].O[0].type = NONE; partTable[pnum].X[xnum].O[0].ITEM = NULL;
  partTable[pnum].X[xnum].O[++onum].type = DEFAULT; partTable[pnum].X[xnum].O[onum].ITEM = (void *) &(operationTable[3]); /* O3 */
  partTable[pnum].X[xnum].O[++onum].type = ALTERNATIVE; partTable[pnum].X[xnum].O[onum].ITEM = (void *) &partTable[1]; /* |P1a| */
  partTable[pnum].X[xnum].O[++onum].type = DEFAULT; partTable[pnum].X[xnum].O[onum].ITEM = (void *) &(operationTable[4]); /* O4 */

  productionTable[part] = &(partTable[pnum]);
  partTable[pnum].color = 0;

  /* ADDVARIABLE(w,r) */
  ADDVARIABLE(1,1);
  ADDVARIABLE(2,2);
  ADDVARIABLE(3,3);
  ADDVARIABLE(3,1);
  ADDVARIABLE(5,2);
  ADDVARIABLE(1,2);
  ADDVARIABLE(4,3);

  /* *************************************** */
  /* PART P2 */
  /* *************************************** */

  part++;

  pnum++; xnum = 0; onum = 0; /* next part, id = 3 */

  partTable[pnum].name = strdup("P2a");
  partTable[pnum].part = part;
  partTable[pnum].n = 2; /* number of sequences */
  partTable[pnum].code = NULL; partTable[pnum].feasible = NULL;
  partTable[pnum].X = (SEQUENCE *) malloc((1 + partTable[pnum].n) * sizeof(SEQUENCE));
  partTable[pnum].X[0].n = 0; partTable[pnum].X[0].O = NULL;

  xnum++; onum = 0; /* next sequence */

  partTable[pnum].X[xnum].n = 1; /* number of items for the 1st sequence in P2a */
  partTable[pnum].X[xnum].O = (SEQUENCEITEM *) malloc((1 + partTable[pnum].X[xnum].n) * sizeof(SEQUENCEITEM));
  partTable[pnum].X[xnum].O[0].type = NONE; partTable[pnum].X[xnum].O[0].ITEM = NULL;
  partTable[pnum].X[xnum].O[++onum].type = DEFAULT; partTable[pnum].X[xnum].O[onum].ITEM = (void *) &(operationTable[7]); /* O7 */

  xnum++; onum = 0; /* next sequence */

  partTable[pnum].X[xnum].n = 1; /* number of items for the 2nd sequence in P2a */
  partTable[pnum].X[xnum].O = (SEQUENCEITEM *) malloc((1 + partTable[pnum].X[xnum].n) * sizeof(SEQUENCEITEM));
  partTable[pnum].X[xnum].O[0].type = NONE; partTable[pnum].X[xnum].O[0].ITEM = NULL;
  partTable[pnum].X[xnum].O[++onum].type = DEFAULT; partTable[pnum].X[xnum].O[onum].ITEM = (void *) &(operationTable[4]); /* O4 */

  pnum++; xnum = 0; onum = 0; /* next part, id = 4 */

  partTable[pnum].name = strdup("P2b");
  partTable[pnum].part = part;
  partTable[pnum].n = 2; /* number of sequences */
  partTable[pnum].code = NULL; partTable[pnum].feasible = NULL;
  partTable[pnum].X = (SEQUENCE *) malloc((1 + partTable[pnum].n) * sizeof(SEQUENCE));
  partTable[pnum].X[0].n = 0; partTable[pnum].X[0].O = NULL;

  xnum++; onum = 0; /* next sequence */

  partTable[pnum].X[xnum].n = 1; /* number of items for the 1st sequence in P2b */
  partTable[pnum].X[xnum].O = (SEQUENCEITEM *) malloc((1 + partTable[pnum].X[xnum].n) * sizeof(SEQUENCEITEM));
  partTable[pnum].X[xnum].O[0].type = NONE; partTable[pnum].X[xnum].O[0].ITEM = NULL;
  partTable[pnum].X[xnum].O[++onum].type = DEFAULT; partTable[pnum].X[xnum].O[onum].ITEM = (void *) &(operationTable[8]); /* O8 */

  xnum++; onum = 0; /* next sequence */

  partTable[pnum].X[xnum].n = 1; /* number of items for the 2nd sequence in P2b */
  partTable[pnum].X[xnum].O = (SEQUENCEITEM *) malloc((1 + partTable[pnum].X[xnum].n) * sizeof(SEQUENCEITEM));
  partTable[pnum].X[xnum].O[0].type = NONE; partTable[pnum].X[xnum].O[0].ITEM = NULL;
  partTable[pnum].X[xnum].O[++onum].type = DEFAULT; partTable[pnum].X[xnum].O[onum].ITEM = (void *) &(operationTable[2]); /* O2 */

  pnum++; xnum = 0; onum = 0; /* next part, id = 5 */

  partTable[pnum].name = strdup("P02");
  partTable[pnum].part = part;
  partTable[pnum].n = 3; /* number of sequences */
  partTable[pnum].code = NULL; partTable[pnum].feasible = NULL;
  partTable[pnum].X = (SEQUENCE *) malloc((1 + partTable[pnum].n) * sizeof(SEQUENCE));
  partTable[pnum].X[0].n = 0; partTable[pnum].X[0].O = NULL;

  xnum++; onum = 0; /* next sequence */

  partTable[pnum].X[xnum].n = 3; /* number of items for the 1st sequence in P2 */
  partTable[pnum].X[xnum].O = (SEQUENCEITEM *) malloc((1 + partTable[pnum].X[xnum].n) * sizeof(SEQUENCEITEM));
  partTable[pnum].X[xnum].O[0].type = NONE; partTable[pnum].X[xnum].O[0].ITEM = NULL;
  partTable[pnum].X[xnum].O[++onum].type = DEFAULT; partTable[pnum].X[xnum].O[onum].ITEM = (void *) &(operationTable[6]); /* O6 */
  partTable[pnum].X[xnum].O[++onum].type = DEFAULT; partTable[pnum].X[xnum].O[onum].ITEM = (void *) &(operationTable[3]); /* O3 */
  partTable[pnum].X[xnum].O[++onum].type = DEFAULT; partTable[pnum].X[xnum].O[onum].ITEM = (void *) &(operationTable[2]); /* O2 */

  xnum++; onum = 0; /* next sequence */

  partTable[pnum].X[xnum].n = 3; /* number of items for the 2nd sequence in P2 */
  partTable[pnum].X[xnum].O = (SEQUENCEITEM *) malloc((1 + partTable[pnum].X[xnum].n) * sizeof(SEQUENCEITEM));
  partTable[pnum].X[xnum].O[0].type = NONE; partTable[pnum].X[xnum].O[0].ITEM = NULL;
  partTable[pnum].X[xnum].O[++onum].type = DEFAULT; partTable[pnum].X[xnum].O[onum].ITEM = (void *) &(operationTable[8]); /* O8 */
  partTable[pnum].X[xnum].O[++onum].type = PERMUTATION; partTable[pnum].X[xnum].O[onum].ITEM = (void *) &(operationTable[1]); /* [O1] */
  partTable[pnum].X[xnum].O[++onum].type = PERMUTATION; partTable[pnum].X[xnum].O[onum].ITEM = (void *) &(operationTable[5]); /* [O5] */

  xnum++; onum = 0; /* next sequence */

  partTable[pnum].X[xnum].n = 3; /* number of items for the 3rd sequence in P2 */
  partTable[pnum].X[xnum].O = (SEQUENCEITEM *) malloc((1 + partTable[pnum].X[xnum].n) * sizeof(SEQUENCEITEM));
  partTable[pnum].X[xnum].O[0].type = NONE; partTable[pnum].X[xnum].O[0].ITEM = NULL;
  partTable[pnum].X[xnum].O[++onum].type = ALTERNATIVE; partTable[pnum].X[xnum].O[onum].ITEM = (void *) &partTable[3]; /* |P2a| */
  partTable[pnum].X[xnum].O[++onum].type = DEFAULT; partTable[pnum].X[xnum].O[onum].ITEM = (void *) &(operationTable[5]); /* O5 */
  partTable[pnum].X[xnum].O[++onum].type = ALTERNATIVE; partTable[pnum].X[xnum].O[onum].ITEM = (void *) &partTable[4]; /* |P2b| */

  productionTable[part] = &(partTable[pnum]);
  partTable[pnum].color = 60;

  /* ADDVARIABLE(w,r) */
  ADDVARIABLE(6,1);
  ADDVARIABLE(3,2);
  ADDVARIABLE(2,3);
  ADDVARIABLE(8,1);
  ADDVARIABLE(1,2);
  ADDVARIABLE(1,3);
  ADDVARIABLE(5,2);
  ADDVARIABLE(5,3);
  ADDVARIABLE(7,1);
  ADDVARIABLE(4,1);
  ADDVARIABLE(5,2);
  ADDVARIABLE(8,3);
  ADDVARIABLE(2,3);

  /* *************************************** */
  /* PART P3 */
  /* *************************************** */

  part++;

  pnum++; xnum = 0; onum = 0; /* next part, id = 6 */

  partTable[pnum].name = strdup("P3a");
  partTable[pnum].part = part;
  partTable[pnum].n = 2; /* number of sequences */
  partTable[pnum].code = NULL; partTable[pnum].feasible = NULL;
  partTable[pnum].X = (SEQUENCE *) malloc((1 + partTable[pnum].n) * sizeof(SEQUENCE));
  partTable[pnum].X[0].n = 0; partTable[pnum].X[0].O = NULL;

  xnum++; onum = 0; /* next sequence */

  partTable[pnum].X[xnum].n = 1; /* number of items for the 1st sequence in P3a */
  partTable[pnum].X[xnum].O = (SEQUENCEITEM *) malloc((1 + partTable[pnum].X[xnum].n) * sizeof(SEQUENCEITEM));
  partTable[pnum].X[xnum].O[0].type = NONE; partTable[pnum].X[xnum].O[0].ITEM = NULL;
  partTable[pnum].X[xnum].O[++onum].type = DEFAULT; partTable[pnum].X[xnum].O[onum].ITEM = (void *) &(operationTable[8]); /* O8 */

  xnum++; onum = 0; /* next sequence */

  partTable[pnum].X[xnum].n = 1; /* number of items for the 2nd sequence in P3a */
  partTable[pnum].X[xnum].O = (SEQUENCEITEM *) malloc((1 + partTable[pnum].X[xnum].n) * sizeof(SEQUENCEITEM));
  partTable[pnum].X[xnum].O[0].type = NONE; partTable[pnum].X[xnum].O[0].ITEM = NULL;
  partTable[pnum].X[xnum].O[++onum].type = DEFAULT; partTable[pnum].X[xnum].O[onum].ITEM = (void *) &(operationTable[9]); /* O9 */

  pnum++; xnum = 0; onum = 0; /* next part, id = 7 */

  partTable[pnum].name = strdup("P03");
  partTable[pnum].part = part;
  partTable[pnum].n = 1; /* number of sequences */
  partTable[pnum].code = NULL; partTable[pnum].feasible = NULL;
  partTable[pnum].X = (SEQUENCE *) malloc((1 + partTable[pnum].n) * sizeof(SEQUENCE));
  partTable[pnum].X[0].n = 0; partTable[pnum].X[0].O = NULL;

  xnum++; onum = 0; /* next sequence */

  partTable[pnum].X[xnum].n = 3; /* number of items for the 1st sequence in P3 */
  partTable[pnum].X[xnum].O = (SEQUENCEITEM *) malloc((1 + partTable[pnum].X[xnum].n) * sizeof(SEQUENCEITEM));
  partTable[pnum].X[xnum].O[0].type = NONE; partTable[pnum].X[xnum].O[0].ITEM = NULL;
  partTable[pnum].X[xnum].O[++onum].type = PERMUTATION; partTable[pnum].X[xnum].O[onum].ITEM = (void *) &(operationTable[4]); /* [O4] */
  partTable[pnum].X[xnum].O[++onum].type = PERMUTATION; partTable[pnum].X[xnum].O[onum].ITEM = (void *) &(operationTable[7]); /* [O7] */
  partTable[pnum].X[xnum].O[++onum].type = ALTERNATIVE; partTable[pnum].X[xnum].O[onum].ITEM = (void *) &partTable[6]; /* |P3a| */

  productionTable[part] = &(partTable[pnum]);
  partTable[pnum].color = 140;

  /* ADDVARIABLE(w,r) */
  ADDVARIABLE(4,1);
  ADDVARIABLE(4,2);
  ADDVARIABLE(7,1);
  ADDVARIABLE(7,2);
  ADDVARIABLE(8,3);
  ADDVARIABLE(9,3);

  /* *************************************** */
  /* PART P4 */
  /* *************************************** */

  part++;

  pnum++; xnum = 0; onum = 0; /* next part, id = 8 */

  partTable[pnum].name = strdup("P4a");
  partTable[pnum].part = part;
  partTable[pnum].n = 2; /* number of sequences */
  partTable[pnum].code = NULL; partTable[pnum].feasible = NULL;
  partTable[pnum].X = (SEQUENCE *) malloc((1 + partTable[pnum].n) * sizeof(SEQUENCE));
  partTable[pnum].X[0].n = 0; partTable[pnum].X[0].O = NULL;

  xnum++; onum = 0; /* next sequence */

  partTable[pnum].X[xnum].n = 1; /* number of items for the 1st sequence in P4a */
  partTable[pnum].X[xnum].O = (SEQUENCEITEM *) malloc((1 + partTable[pnum].X[xnum].n) * sizeof(SEQUENCEITEM));
  partTable[pnum].X[xnum].O[0].type = NONE; partTable[pnum].X[xnum].O[0].ITEM = NULL;
  partTable[pnum].X[xnum].O[++onum].type = DEFAULT; partTable[pnum].X[xnum].O[onum].ITEM = (void *) &(operationTable[1]); /* O1 */

  xnum++; onum = 0; /* next sequence */

  partTable[pnum].X[xnum].n = 1; /* number of items for the 2nd sequence in P4a */
  partTable[pnum].X[xnum].O = (SEQUENCEITEM *) malloc((1 + partTable[pnum].X[xnum].n) * sizeof(SEQUENCEITEM));
  partTable[pnum].X[xnum].O[0].type = NONE; partTable[pnum].X[xnum].O[0].ITEM = NULL;
  partTable[pnum].X[xnum].O[++onum].type = DEFAULT; partTable[pnum].X[xnum].O[onum].ITEM = (void *) &(operationTable[3]); /* O3 */

  pnum++; xnum = 0; onum = 0; /* next part, id = 9 */

  partTable[pnum].name = strdup("P04");
  partTable[pnum].part = part;
  partTable[pnum].n = 1; /* number of sequences */
  partTable[pnum].code = NULL; partTable[pnum].feasible = NULL;
  partTable[pnum].X = (SEQUENCE *) malloc((1 + partTable[pnum].n) * sizeof(SEQUENCE));
  partTable[pnum].X[0].n = 0; partTable[pnum].X[0].O = NULL;

  xnum++; onum = 0; /* next sequence */

  partTable[pnum].X[xnum].n = 3; /* number of items for the 1st sequence in P4 */
  partTable[pnum].X[xnum].O = (SEQUENCEITEM *) malloc((1 + partTable[pnum].X[xnum].n) * sizeof(SEQUENCEITEM));
  partTable[pnum].X[xnum].O[0].type = NONE; partTable[pnum].X[xnum].O[0].ITEM = NULL;
  partTable[pnum].X[xnum].O[++onum].type = PERMUTATION; partTable[pnum].X[xnum].O[onum].ITEM = (void *) &(operationTable[5]); /* [O5] */
  partTable[pnum].X[xnum].O[++onum].type = PERMUTATION; partTable[pnum].X[xnum].O[onum].ITEM = (void *) &(operationTable[6]); /* [O6] */
  partTable[pnum].X[xnum].O[++onum].type = ALTERNATIVE; partTable[pnum].X[xnum].O[onum].ITEM = (void *) &partTable[8]; /* |P4a| */

  productionTable[part] = &(partTable[pnum]);
  partTable[pnum].color = 180;

  /* ADDVARIABLE(w,r) */
  ADDVARIABLE(5,1);
  ADDVARIABLE(5,2);
  ADDVARIABLE(6,1);
  ADDVARIABLE(6,2);
  ADDVARIABLE(1,3);
  ADDVARIABLE(3,3);

  /* *************************************** */
  /* PART P5 */
  /* *************************************** */

  part++;

  pnum++; xnum = 0; onum = 0; /* next part, id = 10 */

  partTable[pnum].name = strdup("P5a");
  partTable[pnum].part = part;
  partTable[pnum].n = 2; /* number of sequences */
  partTable[pnum].code = NULL; partTable[pnum].feasible = NULL;
  partTable[pnum].X = (SEQUENCE *) malloc((1 + partTable[pnum].n) * sizeof(SEQUENCE));
  partTable[pnum].X[0].n = 0; partTable[pnum].X[0].O = NULL;

  xnum++; onum = 0; /* next sequence */

  partTable[pnum].X[xnum].n = 1; /* number of items for the 1st sequence in P5a */
  partTable[pnum].X[xnum].O = (SEQUENCEITEM *) malloc((1 + partTable[pnum].X[xnum].n) * sizeof(SEQUENCEITEM));
  partTable[pnum].X[xnum].O[0].type = NONE; partTable[pnum].X[xnum].O[0].ITEM = NULL;
  partTable[pnum].X[xnum].O[++onum].type = DEFAULT; partTable[pnum].X[xnum].O[onum].ITEM = (void *) &(operationTable[2]); /* O2 */

  xnum++; onum = 0; /* next sequence */

  partTable[pnum].X[xnum].n = 1; /* number of items for the 2nd sequence in P5a */
  partTable[pnum].X[xnum].O = (SEQUENCEITEM *) malloc((1 + partTable[pnum].X[xnum].n) * sizeof(SEQUENCEITEM));
  partTable[pnum].X[xnum].O[0].type = NONE; partTable[pnum].X[xnum].O[0].ITEM = NULL;
  partTable[pnum].X[xnum].O[++onum].type = DEFAULT; partTable[pnum].X[xnum].O[onum].ITEM = (void *) &(operationTable[12]); /* O12 */

  pnum++; xnum = 0; onum = 0; /* next part, id = 11 */

  partTable[pnum].name = strdup("P5b");
  partTable[pnum].part = part;
  partTable[pnum].n = 2; /* number of sequences */
  partTable[pnum].code = NULL; partTable[pnum].feasible = NULL;
  partTable[pnum].X = (SEQUENCE *) malloc((1 + partTable[pnum].n) * sizeof(SEQUENCE));
  partTable[pnum].X[0].n = 0; partTable[pnum].X[0].O = NULL;

  xnum++; onum = 0; /* next sequence */

  partTable[pnum].X[xnum].n = 1; /* number of items for the 1st sequence in P5b */
  partTable[pnum].X[xnum].O = (SEQUENCEITEM *) malloc((1 + partTable[pnum].X[xnum].n) * sizeof(SEQUENCEITEM));
  partTable[pnum].X[xnum].O[0].type = NONE; partTable[pnum].X[xnum].O[0].ITEM = NULL;
  partTable[pnum].X[xnum].O[++onum].type = DEFAULT; partTable[pnum].X[xnum].O[onum].ITEM = (void *) &(operationTable[10]); /* O10 */

  xnum++; onum = 0; /* next sequence */

  partTable[pnum].X[xnum].n = 1; /* number of items for the 2nd sequence in P5b */
  partTable[pnum].X[xnum].O = (SEQUENCEITEM *) malloc((1 + partTable[pnum].X[xnum].n) * sizeof(SEQUENCEITEM));
  partTable[pnum].X[xnum].O[0].type = NONE; partTable[pnum].X[xnum].O[0].ITEM = NULL;
  partTable[pnum].X[xnum].O[++onum].type = DEFAULT; partTable[pnum].X[xnum].O[onum].ITEM = (void *) &(operationTable[8]); /* O8 */

  pnum++; xnum = 0; onum = 0; /* next part, id = 12 */

  partTable[pnum].name = strdup("P5c");
  partTable[pnum].part = part;
  partTable[pnum].n = 2; /* number of sequences */
  partTable[pnum].code = NULL; partTable[pnum].feasible = NULL;
  partTable[pnum].X = (SEQUENCE *) malloc((1 + partTable[pnum].n) * sizeof(SEQUENCE));
  partTable[pnum].X[0].n = 0; partTable[pnum].X[0].O = NULL;

  xnum++; onum = 0; /* next sequence */

  partTable[pnum].X[xnum].n = 1; /* number of items for the 1st sequence in P5c */
  partTable[pnum].X[xnum].O = (SEQUENCEITEM *) malloc((1 + partTable[pnum].X[xnum].n) * sizeof(SEQUENCEITEM));
  partTable[pnum].X[xnum].O[0].type = NONE; partTable[pnum].X[xnum].O[0].ITEM = NULL;
  partTable[pnum].X[xnum].O[++onum].type = DEFAULT; partTable[pnum].X[xnum].O[onum].ITEM = (void *) &(operationTable[13]); /* O13 */

  xnum++; onum = 0; /* next sequence */

  partTable[pnum].X[xnum].n = 1; /* number of items for the 2nd sequence in P5c */
  partTable[pnum].X[xnum].O = (SEQUENCEITEM *) malloc((1 + partTable[pnum].X[xnum].n) * sizeof(SEQUENCEITEM));
  partTable[pnum].X[xnum].O[0].type = NONE; partTable[pnum].X[xnum].O[0].ITEM = NULL;
  partTable[pnum].X[xnum].O[++onum].type = DEFAULT; partTable[pnum].X[xnum].O[onum].ITEM = (void *) &(operationTable[9]); /* O9 */

  pnum++; xnum = 0; onum = 0; /* next part, id = 13 */

  partTable[pnum].name = strdup("P05");
  partTable[pnum].part = part;
  partTable[pnum].n = 1; /* number of sequences */
  partTable[pnum].code = NULL; partTable[pnum].feasible = NULL;
  partTable[pnum].X = (SEQUENCE *) malloc((1 + partTable[pnum].n) * sizeof(SEQUENCE));
  partTable[pnum].X[0].n = 0; partTable[pnum].X[0].O = NULL;

  xnum++; onum = 0; /* next sequence */

  partTable[pnum].X[xnum].n = 3; /* number of items for the 1st sequence in P5 */
  partTable[pnum].X[xnum].O = (SEQUENCEITEM *) malloc((1 + partTable[pnum].X[xnum].n) * sizeof(SEQUENCEITEM));
  partTable[pnum].X[xnum].O[0].type = NONE; partTable[pnum].X[xnum].O[0].ITEM = NULL;
  partTable[pnum].X[xnum].O[++onum].type = ALTERNATIVE; partTable[pnum].X[xnum].O[onum].ITEM = (void *) &partTable[10]; /* |P5a| */
  partTable[pnum].X[xnum].O[++onum].type = ALTERNATIVE; partTable[pnum].X[xnum].O[onum].ITEM = (void *) &partTable[11]; /* |P5b| */
  partTable[pnum].X[xnum].O[++onum].type = ALTERNATIVE; partTable[pnum].X[xnum].O[onum].ITEM = (void *) &partTable[12]; /* |P5c| */

  productionTable[part] = &(partTable[pnum]);
  partTable[pnum].color = 230;

  /* ADDVARIABLE(w,r) */
  ADDVARIABLE(2,1);
  ADDVARIABLE(12,1);
  ADDVARIABLE(10,2);
  ADDVARIABLE(8,2);
  ADDVARIABLE(13,3);
  ADDVARIABLE(9,3);

  /* *************************************** */
  /* PART P6 */
  /* *************************************** */

  part++;

  pnum++; xnum = 0; onum = 0; /* next part, id = 14 */

  partTable[pnum].name = strdup("P06");
  partTable[pnum].part = part;
  partTable[pnum].n = 2; /* number of sequences */
  partTable[pnum].code = NULL; partTable[pnum].feasible = NULL;
  partTable[pnum].X = (SEQUENCE *) malloc((1 + partTable[pnum].n) * sizeof(SEQUENCE));
  partTable[pnum].X[0].n = 0; partTable[pnum].X[0].O = NULL;

  xnum++; onum = 0; /* next sequence */

  partTable[pnum].X[xnum].n = 2; /* number of items for the 1st sequence in P6 */
  partTable[pnum].X[xnum].O = (SEQUENCEITEM *) malloc((1 + partTable[pnum].X[xnum].n) * sizeof(SEQUENCEITEM));
  partTable[pnum].X[xnum].O[0].type = NONE; partTable[pnum].X[xnum].O[0].ITEM = NULL;
  partTable[pnum].X[xnum].O[++onum].type = DEFAULT; partTable[pnum].X[xnum].O[onum].ITEM = (void *) &(operationTable[3]); /* O3 */
  partTable[pnum].X[xnum].O[++onum].type = DEFAULT; partTable[pnum].X[xnum].O[onum].ITEM = (void *) &(operationTable[11]); /* O11 */

  xnum++; onum = 0; /* next sequence */

  partTable[pnum].X[xnum].n = 2; /* number of items for the 2nd sequence in P6 */
  partTable[pnum].X[xnum].O = (SEQUENCEITEM *) malloc((1 + partTable[pnum].X[xnum].n) * sizeof(SEQUENCEITEM));
  partTable[pnum].X[xnum].O[0].type = NONE; partTable[pnum].X[xnum].O[0].ITEM = NULL;
  partTable[pnum].X[xnum].O[++onum].type = DEFAULT; partTable[pnum].X[xnum].O[onum].ITEM = (void *) &(operationTable[14]); /* O14 */
  partTable[pnum].X[xnum].O[++onum].type = DEFAULT; partTable[pnum].X[xnum].O[onum].ITEM = (void *) &(operationTable[5]); /* O5 */

  productionTable[part] = &(partTable[pnum]);
  partTable[pnum].color = 300;

  /* ADDVARIABLE(w,r) */
  ADDVARIABLE(3,1);
  ADDVARIABLE(11,2);
  ADDVARIABLE(14,1);
  ADDVARIABLE(5,2);

  /* *************************************** */
  /* PART P7 */
  /* *************************************** */

  part++;

  pnum++; xnum = 0; onum = 0; /* next part, id = 15 */

  partTable[pnum].name = strdup("P07");
  partTable[pnum].part = part;
  partTable[pnum].n = 2; /* number of sequences */
  partTable[pnum].code = NULL; partTable[pnum].feasible = NULL;
  partTable[pnum].X = (SEQUENCE *) malloc((1 + partTable[pnum].n) * sizeof(SEQUENCE));
  partTable[pnum].X[0].n = 0; partTable[pnum].X[0].O = NULL;

  xnum++; onum = 0; /* next sequence */

  partTable[pnum].X[xnum].n = 3; /* number of items for the 1st sequence in P7 */
  partTable[pnum].X[xnum].O = (SEQUENCEITEM *) malloc((1 + partTable[pnum].X[xnum].n) * sizeof(SEQUENCEITEM));
  partTable[pnum].X[xnum].O[0].type = NONE; partTable[pnum].X[xnum].O[0].ITEM = NULL;
  partTable[pnum].X[xnum].O[++onum].type = DEFAULT; partTable[pnum].X[xnum].O[onum].ITEM = (void *) &(operationTable[14]); /* O14 */
  partTable[pnum].X[xnum].O[++onum].type = DEFAULT; partTable[pnum].X[xnum].O[onum].ITEM = (void *) &(operationTable[4]); /* O4 */
  partTable[pnum].X[xnum].O[++onum].type = DEFAULT; partTable[pnum].X[xnum].O[onum].ITEM = (void *) &(operationTable[13]); /* O13 */

  xnum++; onum = 0; /* next sequence */

  partTable[pnum].X[xnum].n = 2; /* number of items for the 2nd sequence in P7 */
  partTable[pnum].X[xnum].O = (SEQUENCEITEM *) malloc((1 + partTable[pnum].X[xnum].n) * sizeof(SEQUENCEITEM));
  partTable[pnum].X[xnum].O[0].type = NONE; partTable[pnum].X[xnum].O[0].ITEM = NULL;
  partTable[pnum].X[xnum].O[++onum].type = DEFAULT; partTable[pnum].X[xnum].O[onum].ITEM = (void *) &(operationTable[15]); /* O15 */
  partTable[pnum].X[xnum].O[++onum].type = DEFAULT; partTable[pnum].X[xnum].O[onum].ITEM = (void *) &(operationTable[10]); /* O10 */

  productionTable[part] = &(partTable[pnum]);
  partTable[pnum].color = 30;

  /* ADDVARIABLE(w,r) */
  ADDVARIABLE(14,1);
  ADDVARIABLE(4,2);
  ADDVARIABLE(13,3);
  ADDVARIABLE(15,1);
  ADDVARIABLE(10,2);

  /* *************************************** */
  /* PART P8 */
  /* *************************************** */

  part++;

  pnum++; xnum = 0; onum = 0; /* next part, id = 16 */

  partTable[pnum].name = strdup("P8a");
  partTable[pnum].part = part;
  partTable[pnum].n = 2; /* number of sequences */
  partTable[pnum].code = NULL; partTable[pnum].feasible = NULL;
  partTable[pnum].X = (SEQUENCE *) malloc((1 + partTable[pnum].n) * sizeof(SEQUENCE));
  partTable[pnum].X[0].n = 0; partTable[pnum].X[0].O = NULL;

  xnum++; onum = 0; /* next sequence */

  partTable[pnum].X[xnum].n = 1; /* number of items for the 1st sequence in P8a */
  partTable[pnum].X[xnum].O = (SEQUENCEITEM *) malloc((1 + partTable[pnum].X[xnum].n) * sizeof(SEQUENCEITEM));
  partTable[pnum].X[xnum].O[0].type = NONE; partTable[pnum].X[xnum].O[0].ITEM = NULL;
  partTable[pnum].X[xnum].O[++onum].type = DEFAULT; partTable[pnum].X[xnum].O[onum].ITEM = (void *) &(operationTable[14]); /* O14 */

  xnum++; onum = 0; /* next sequence */

  partTable[pnum].X[xnum].n = 1; /* number of items for the 2nd sequence in P8a */
  partTable[pnum].X[xnum].O = (SEQUENCEITEM *) malloc((1 + partTable[pnum].X[xnum].n) * sizeof(SEQUENCEITEM));
  partTable[pnum].X[xnum].O[0].type = NONE; partTable[pnum].X[xnum].O[0].ITEM = NULL;
  partTable[pnum].X[xnum].O[++onum].type = DEFAULT; partTable[pnum].X[xnum].O[onum].ITEM = (void *) &(operationTable[1]); /* O1 */

  pnum++; xnum = 0; onum = 0; /* next part, id = 17 */

  partTable[pnum].name = strdup("P8b");
  partTable[pnum].part = part;
  partTable[pnum].n = 2; /* number of sequences */
  partTable[pnum].code = NULL; partTable[pnum].feasible = NULL;
  partTable[pnum].X = (SEQUENCE *) malloc((1 + partTable[pnum].n) * sizeof(SEQUENCE));
  partTable[pnum].X[0].n = 0; partTable[pnum].X[0].O = NULL;

  xnum++; onum = 0; /* next sequence */

  partTable[pnum].X[xnum].n = 1; /* number of items for the 1st sequence in P8b */
  partTable[pnum].X[xnum].O = (SEQUENCEITEM *) malloc((1 + partTable[pnum].X[xnum].n) * sizeof(SEQUENCEITEM));
  partTable[pnum].X[xnum].O[0].type = NONE; partTable[pnum].X[xnum].O[0].ITEM = NULL;
  partTable[pnum].X[xnum].O[++onum].type = DEFAULT; partTable[pnum].X[xnum].O[onum].ITEM = (void *) &(operationTable[10]); /* O10 */

  xnum++; onum = 0; /* next sequence */

  partTable[pnum].X[xnum].n = 1; /* number of items for the 2nd sequence in P8b */
  partTable[pnum].X[xnum].O = (SEQUENCEITEM *) malloc((1 + partTable[pnum].X[xnum].n) * sizeof(SEQUENCEITEM));
  partTable[pnum].X[xnum].O[0].type = NONE; partTable[pnum].X[xnum].O[0].ITEM = NULL;
  partTable[pnum].X[xnum].O[++onum].type = DEFAULT; partTable[pnum].X[xnum].O[onum].ITEM = (void *) &(operationTable[12]); /* O12 */

  pnum++; xnum = 0; onum = 0; /* next part, id = 18 */

  partTable[pnum].name = strdup("P08");
  partTable[pnum].part = part;
  partTable[pnum].n = 2; /* number of sequences */
  partTable[pnum].code = NULL; partTable[pnum].feasible = NULL;
  partTable[pnum].X = (SEQUENCE *) malloc((1 + partTable[pnum].n) * sizeof(SEQUENCE));
  partTable[pnum].X[0].n = 0; partTable[pnum].X[0].O = NULL;

  xnum++; onum = 0; /* next sequence */

  partTable[pnum].X[xnum].n = 4; /* number of items for the 1st sequence in P8 */
  partTable[pnum].X[xnum].O = (SEQUENCEITEM *) malloc((1 + partTable[pnum].X[xnum].n) * sizeof(SEQUENCEITEM));
  partTable[pnum].X[xnum].O[0].type = NONE; partTable[pnum].X[xnum].O[0].ITEM = NULL;
  partTable[pnum].X[xnum].O[++onum].type = DEFAULT; partTable[pnum].X[xnum].O[onum].ITEM = (void *) &(operationTable[11]); /* O11 */
  partTable[pnum].X[xnum].O[++onum].type = DEFAULT; partTable[pnum].X[xnum].O[onum].ITEM = (void *) &(operationTable[9]); /* O9 */
  partTable[pnum].X[xnum].O[++onum].type = DEFAULT; partTable[pnum].X[xnum].O[onum].ITEM = (void *) &(operationTable[13]); /* O13 */
  partTable[pnum].X[xnum].O[++onum].type = DEFAULT; partTable[pnum].X[xnum].O[onum].ITEM = (void *) &(operationTable[2]); /* O2 */

  xnum++; onum = 0; /* next sequence */

  partTable[pnum].X[xnum].n = 4; /* number of items for the 2nd sequence in P8 */
  partTable[pnum].X[xnum].O = (SEQUENCEITEM *) malloc((1 + partTable[pnum].X[xnum].n) * sizeof(SEQUENCEITEM));
  partTable[pnum].X[xnum].O[0].type = NONE; partTable[pnum].X[xnum].O[0].ITEM = NULL;
  partTable[pnum].X[xnum].O[++onum].type = DEFAULT; partTable[pnum].X[xnum].O[onum].ITEM = (void *) &(operationTable[13]); /* O13 */
  partTable[pnum].X[xnum].O[++onum].type = ALTERNATIVE; partTable[pnum].X[xnum].O[onum].ITEM = (void *) &partTable[16]; /* |P8a| */
  partTable[pnum].X[xnum].O[++onum].type = DEFAULT; partTable[pnum].X[xnum].O[onum].ITEM = (void *) &(operationTable[6]); /* O6 */
  partTable[pnum].X[xnum].O[++onum].type = ALTERNATIVE; partTable[pnum].X[xnum].O[onum].ITEM = (void *) &partTable[17]; /* |P8b| */

  productionTable[part] = &(partTable[pnum]);
  partTable[pnum].color = 270;

  /* ADDVARIABLE(w,r) */
  ADDVARIABLE(11,1);
  ADDVARIABLE(9,2);
  ADDVARIABLE(13,3);
  ADDVARIABLE(2,4);
  ADDVARIABLE(13,1);
  ADDVARIABLE(14,2);
  ADDVARIABLE(1,2);
  ADDVARIABLE(6,3);
  ADDVARIABLE(10,4);
  ADDVARIABLE(12,4);

  /* *************************************** */
  /* PART P9 */
  /* *************************************** */

  part++;

  pnum++; xnum = 0; onum = 0; /* next part, id = 19 */

  partTable[pnum].name = strdup("P09");
  partTable[pnum].part = part;
  partTable[pnum].n = 2; /* number of sequences */
  partTable[pnum].code = NULL; partTable[pnum].feasible = NULL;
  partTable[pnum].X = (SEQUENCE *) malloc((1 + partTable[pnum].n) * sizeof(SEQUENCE));
  partTable[pnum].X[0].n = 0; partTable[pnum].X[0].O = NULL;

  xnum++; onum = 0; /* next sequence */

  partTable[pnum].X[xnum].n = 3; /* number of items for the 1st sequence in P8 */
  partTable[pnum].X[xnum].O = (SEQUENCEITEM *) malloc((1 + partTable[pnum].X[xnum].n) * sizeof(SEQUENCEITEM));
  partTable[pnum].X[xnum].O[0].type = NONE; partTable[pnum].X[xnum].O[0].ITEM = NULL;
  partTable[pnum].X[xnum].O[++onum].type = DEFAULT; partTable[pnum].X[xnum].O[onum].ITEM = (void *) &(operationTable[10]); /* O10 */
  partTable[pnum].X[xnum].O[++onum].type = DEFAULT; partTable[pnum].X[xnum].O[onum].ITEM = (void *) &(operationTable[2]); /* O2 */
  partTable[pnum].X[xnum].O[++onum].type = DEFAULT; partTable[pnum].X[xnum].O[onum].ITEM = (void *) &(operationTable[6]); /* O6 */

  xnum++; onum = 0; /* next sequence */

  partTable[pnum].X[xnum].n = 3; /* number of items for the 2nd sequence in P8 */
  partTable[pnum].X[xnum].O = (SEQUENCEITEM *) malloc((1 + partTable[pnum].X[xnum].n) * sizeof(SEQUENCEITEM));
  partTable[pnum].X[xnum].O[0].type = NONE; partTable[pnum].X[xnum].O[0].ITEM = NULL;
  partTable[pnum].X[xnum].O[++onum].type = DEFAULT; partTable[pnum].X[xnum].O[onum].ITEM = (void *) &(operationTable[7]); /* O7 */
  partTable[pnum].X[xnum].O[++onum].type = DEFAULT; partTable[pnum].X[xnum].O[onum].ITEM = (void *) &(operationTable[11]); /* O11 */
  partTable[pnum].X[xnum].O[++onum].type = DEFAULT; partTable[pnum].X[xnum].O[onum].ITEM = (void *) &(operationTable[1]); /* O1 */

  productionTable[part] = &(partTable[pnum]);
  partTable[pnum].color = 90;

  /* ADDVARIABLE(w,r) */
  ADDVARIABLE(10,1);
  ADDVARIABLE(2,2);
  ADDVARIABLE(6,3);
  ADDVARIABLE(7,1);
  ADDVARIABLE(11,2);
  ADDVARIABLE(1,3);

  /* *************************************** */
  /* PART P10 */
  /* *************************************** */
  
  part++;

  pnum++; xnum = 0; onum = 0; /* next part, id = 20 */

  partTable[pnum].name = strdup("P10a");
  partTable[pnum].part = part;
  partTable[pnum].n = 2; /* number of sequences */
  partTable[pnum].code = NULL; partTable[pnum].feasible = NULL; partTable[pnum].feasible = NULL;
  partTable[pnum].X = (SEQUENCE *) malloc((1 + partTable[pnum].n) * sizeof(SEQUENCE));
  partTable[pnum].X[0].n = 0; partTable[pnum].X[0].O = NULL;

  xnum++; onum = 0; /* next sequence */

  partTable[pnum].X[xnum].n = 1; /* number of items for the 1st sequence in P10a */
  partTable[pnum].X[xnum].O = (SEQUENCEITEM *) malloc((1 + partTable[pnum].X[xnum].n) * sizeof(SEQUENCEITEM));
  partTable[pnum].X[xnum].O[0].type = NONE; partTable[pnum].X[xnum].O[0].ITEM = NULL;
  partTable[pnum].X[xnum].O[++onum].type = DEFAULT; partTable[pnum].X[xnum].O[onum].ITEM = (void *) &(operationTable[12]); /* O12 */

  xnum++; onum = 0; /* next sequence */

  partTable[pnum].X[xnum].n = 1; /* number of items for the 2nd sequence in P10a */
  partTable[pnum].X[xnum].O = (SEQUENCEITEM *) malloc((1 + partTable[pnum].X[xnum].n) * sizeof(SEQUENCEITEM));
  partTable[pnum].X[xnum].O[0].type = NONE; partTable[pnum].X[xnum].O[0].ITEM = NULL;
  partTable[pnum].X[xnum].O[++onum].type = DEFAULT; partTable[pnum].X[xnum].O[onum].ITEM = (void *) &(operationTable[13]); /* O13 */

  pnum++; xnum = 0; onum = 0; /* next part, id = 21 */

  partTable[pnum].name = strdup("P10");
  partTable[pnum].part = part;
  partTable[pnum].n = 2; /* number of sequences */
  partTable[pnum].code = NULL; partTable[pnum].feasible = NULL;
  partTable[pnum].X = (SEQUENCE *) malloc((1 + partTable[pnum].n) * sizeof(SEQUENCE));
  partTable[pnum].X[0].n = 0; partTable[pnum].X[0].O = NULL;

  xnum++; onum = 0; /* next sequence */

  partTable[pnum].X[xnum].n = 3; /* number of items for the 1st sequence in P10 */
  partTable[pnum].X[xnum].O = (SEQUENCEITEM *) malloc((1 + partTable[pnum].X[xnum].n) * sizeof(SEQUENCEITEM));
  partTable[pnum].X[xnum].O[0].type = NONE; partTable[pnum].X[xnum].O[0].ITEM = NULL;
  partTable[pnum].X[xnum].O[++onum].type = DEFAULT; partTable[pnum].X[xnum].O[onum].ITEM = (void *) &(operationTable[10]); /* O10 */
  partTable[pnum].X[xnum].O[++onum].type = DEFAULT; partTable[pnum].X[xnum].O[onum].ITEM = (void *) &(operationTable[15]); /* O15 */
  partTable[pnum].X[xnum].O[++onum].type = DEFAULT; partTable[pnum].X[xnum].O[onum].ITEM = (void *) &(operationTable[8]); /* O8 */

  xnum++; onum = 0; /* next sequence */

  partTable[pnum].X[xnum].n = 3; /* number of items for the 2nd sequence in P10 */
  partTable[pnum].X[xnum].O = (SEQUENCEITEM *) malloc((1 + partTable[pnum].X[xnum].n) * sizeof(SEQUENCEITEM));
  partTable[pnum].X[xnum].O[0].type = NONE; partTable[pnum].X[xnum].O[0].ITEM = NULL;
  partTable[pnum].X[xnum].O[++onum].type = DEFAULT; partTable[pnum].X[xnum].O[onum].ITEM = (void *) &(operationTable[9]); /* O9 */
  partTable[pnum].X[xnum].O[++onum].type = ALTERNATIVE; partTable[pnum].X[xnum].O[onum].ITEM = (void *) &partTable[20]; /* |P10a| */
  partTable[pnum].X[xnum].O[++onum].type = DEFAULT; partTable[pnum].X[xnum].O[onum].ITEM = (void *) &(operationTable[14]); /* O14 */

  productionTable[part] = &(partTable[pnum]);
  partTable[pnum].color = 200;

  /* ADDVARIABLE(w,r) */
  ADDVARIABLE(10,1);
  ADDVARIABLE(15,2);
  ADDVARIABLE(8,3);
  ADDVARIABLE(9,1);
  ADDVARIABLE(12,2);
  ADDVARIABLE(13,2);
  ADDVARIABLE(14,3);
}

/* ************************************************************************************ */
/* ************************************************************************************ */
/* ************************************************************************************ */

void
generateSystemKacem4x5()
{
  unsigned int part,pnum,xnum,onum;
  unsigned int i,j;

  if (NUMPARTS == 0) NUMPARTS = 4;
  if (NUMMACHINES == 0) NUMMACHINES = 5;

  MAXSEQUENCELENGTH = 4;

  /* user defined parts, sequences, and operations start at index 1 */

  /* *************************************** */
  /* OPERATION TABLE */
  /* *************************************** */
  
  operationTableSize = 12; /* number of operations */
  operationTable = (OPERATION *) malloc((1 + operationTableSize) * sizeof(OPERATION));
  (operationTable)[0].name = NULL;
  for (i = 1; i <= operationTableSize; i++)  (operationTable)[i].idx = i;

  (operationTable)[1].name = strdup("O1J1");
  (operationTable)[2].name = strdup("O2J1");
  (operationTable)[3].name = strdup("O3J1");
  (operationTable)[4].name = strdup("O1J2");
  (operationTable)[5].name = strdup("O2J2");
  (operationTable)[6].name = strdup("O3J2");
  (operationTable)[7].name = strdup("O1J3");
  (operationTable)[8].name = strdup("O2J3");
  (operationTable)[9].name = strdup("O3J3");
  (operationTable)[10].name = strdup("O4J3");
  (operationTable)[11].name = strdup("O1J4");
  (operationTable)[12].name = strdup("O2J4");

  /* *************************************** */
  /* MACHINE TABLE */
  /* *************************************** */
  
  machineTableSize = 5; /* number of machines */
  machineTable = (MACHINE *) malloc((1 + machineTableSize) * sizeof(MACHINE));
  machineTable[0].name = NULL;
  for (i = 1; i <= machineTableSize; i++) machineTable[i].idx = i;

  machineTable[1].name = strdup("M1"); machineTable[1].n = 1;
  machineTable[2].name = strdup("M2"); machineTable[2].n = 1;
  machineTable[3].name = strdup("M3"); machineTable[3].n = 1;
  machineTable[4].name = strdup("M4"); machineTable[4].n = 1;
  machineTable[5].name = strdup("M5"); machineTable[5].n = 1;

  if ((NUMMACHINES == 0) || (NUMMACHINES > machineTableSize)) {
    NUMMACHINES = machineTableSize;
  }

  /* *************************************** */
  /* OPERATION MATRIX */
  /* *************************************** */

  operationMatrix = (unsigned int *)
    malloc((1 + machineTableSize) * (1 + operationTableSize) * sizeof(unsigned int));

  for (i = 0; i <= operationTableSize; i++) {
    for (j = 0; j <= machineTableSize; j++) {
      (operationMatrix)[i + j * (1 + operationTableSize)] = 0; /* Oi @ mj */
    }
  }

  operationMatrix[1 + 1 * (1 + operationTableSize)] = 2; /* O1J1 @ M1 */
  operationMatrix[1 + 2 * (1 + operationTableSize)] = 5; /* O1J1 @ M2 */
  operationMatrix[1 + 3 * (1 + operationTableSize)] = 4; /* O1J1 @ M3 */
  operationMatrix[1 + 4 * (1 + operationTableSize)] = 1; /* O1J1 @ M4 */
  operationMatrix[1 + 5 * (1 + operationTableSize)] = 2; /* O1J1 @ M5 */

  operationMatrix[2 + 1 * (1 + operationTableSize)] = 5; /* O2J1 @ M1 */
  operationMatrix[2 + 2 * (1 + operationTableSize)] = 4; /* O2J1 @ M2 */
  operationMatrix[2 + 3 * (1 + operationTableSize)] = 5; /* O2J1 @ M3 */
  operationMatrix[2 + 4 * (1 + operationTableSize)] = 7; /* O2J1 @ M4 */
  operationMatrix[2 + 5 * (1 + operationTableSize)] = 5; /* O2J1 @ M5 */

  operationMatrix[3 + 1 * (1 + operationTableSize)] = 4; /* O3J1 @ M1 */
  operationMatrix[3 + 2 * (1 + operationTableSize)] = 5; /* O3J1 @ M2 */
  operationMatrix[3 + 3 * (1 + operationTableSize)] = 5; /* O3J1 @ M3 */
  operationMatrix[3 + 4 * (1 + operationTableSize)] = 4; /* O3J1 @ M4 */
  operationMatrix[3 + 5 * (1 + operationTableSize)] = 5; /* O3J1 @ M5 */

  operationMatrix[4 + 1 * (1 + operationTableSize)] = 2; /* O1J2 @ M1 */
  operationMatrix[4 + 2 * (1 + operationTableSize)] = 5; /* O1J2 @ M2 */
  operationMatrix[4 + 3 * (1 + operationTableSize)] = 4; /* O1J2 @ M3 */
  operationMatrix[4 + 4 * (1 + operationTableSize)] = 7; /* O1J2 @ M4 */
  operationMatrix[4 + 5 * (1 + operationTableSize)] = 8; /* O1J2 @ M5 */

  operationMatrix[5 + 1 * (1 + operationTableSize)] = 5; /* O2J2 @ M1 */
  operationMatrix[5 + 2 * (1 + operationTableSize)] = 6; /* O2J2 @ M2 */
  operationMatrix[5 + 3 * (1 + operationTableSize)] = 9; /* O2J2 @ M3 */
  operationMatrix[5 + 4 * (1 + operationTableSize)] = 8; /* O2J2 @ M4 */
  operationMatrix[5 + 5 * (1 + operationTableSize)] = 5; /* O2J2 @ M5 */

  operationMatrix[6 + 1 * (1 + operationTableSize)] = 4; /* O3J2 @ M1 */
  operationMatrix[6 + 2 * (1 + operationTableSize)] = 5; /* O3J2 @ M2 */
  operationMatrix[6 + 3 * (1 + operationTableSize)] = 4; /* O3J2 @ M3 */
  operationMatrix[6 + 4 * (1 + operationTableSize)] = 54; /* O3J2 @ M4 */
  operationMatrix[6 + 5 * (1 + operationTableSize)] = 5; /* O3J2 @ M5 */

  operationMatrix[7 + 1 * (1 + operationTableSize)] = 9; /* O1J3 @ M1 */
  operationMatrix[7 + 2 * (1 + operationTableSize)] = 8; /* O1J3 @ M2 */
  operationMatrix[7 + 3 * (1 + operationTableSize)] = 6; /* O1J3 @ M3 */
  operationMatrix[7 + 4 * (1 + operationTableSize)] = 7; /* O1J3 @ M4 */
  operationMatrix[7 + 5 * (1 + operationTableSize)] = 9; /* O1J3 @ M5 */

  operationMatrix[8 + 1 * (1 + operationTableSize)] = 6; /* O2J3 @ M1 */
  operationMatrix[8 + 2 * (1 + operationTableSize)] = 1; /* O2J3 @ M2 */
  operationMatrix[8 + 3 * (1 + operationTableSize)] = 2; /* O2J3 @ M3 */
  operationMatrix[8 + 4 * (1 + operationTableSize)] = 5; /* O2J3 @ M4 */
  operationMatrix[8 + 5 * (1 + operationTableSize)] = 4; /* O2J3 @ M5 */

  operationMatrix[9 + 1 * (1 + operationTableSize)] = 2; /* O3J3 @ M1 */
  operationMatrix[9 + 2 * (1 + operationTableSize)] = 5; /* O3J3 @ M2 */
  operationMatrix[9 + 3 * (1 + operationTableSize)] = 4; /* O3J3 @ M3 */
  operationMatrix[9 + 4 * (1 + operationTableSize)] = 2; /* O3J3 @ M4 */
  operationMatrix[9 + 5 * (1 + operationTableSize)] = 4; /* O3J3 @ M5 */

  operationMatrix[10 + 1 * (1 + operationTableSize)] = 4; /* O4J3 @ M1 */
  operationMatrix[10 + 2 * (1 + operationTableSize)] = 5; /* O4J3 @ M2 */
  operationMatrix[10 + 3 * (1 + operationTableSize)] = 2; /* O4J3 @ M3 */
  operationMatrix[10 + 4 * (1 + operationTableSize)] = 1; /* O4J3 @ M4 */
  operationMatrix[10 + 5 * (1 + operationTableSize)] = 5; /* O4J3 @ M5 */

  operationMatrix[11 + 1 * (1 + operationTableSize)] = 1; /* O1J4 @ M1 */
  operationMatrix[11 + 2 * (1 + operationTableSize)] = 5; /* O1J4 @ M2 */
  operationMatrix[11 + 3 * (1 + operationTableSize)] = 2; /* O1J4 @ M3 */
  operationMatrix[11 + 4 * (1 + operationTableSize)] = 4; /* O1J4 @ M4 */
  operationMatrix[11 + 5 * (1 + operationTableSize)] = 12; /* O1J4 @ M5 */

  operationMatrix[12 + 1 * (1 + operationTableSize)] = 5; /* O2J4 @ M1 */
  operationMatrix[12 + 2 * (1 + operationTableSize)] = 1; /* O2J4 @ M2 */
  operationMatrix[12 + 3 * (1 + operationTableSize)] = 2; /* O2J4 @ M3 */
  operationMatrix[12 + 4 * (1 + operationTableSize)] = 1; /* O2J4 @ M4 */
  operationMatrix[12 + 5 * (1 + operationTableSize)] = 2; /* O2J4 @ M5 */

  /* *************************************** */
  /* PRODUCTION TABLE */
  /* *************************************** */

  productionTableSize = 4;
  productionTable = (PPART *) malloc((1 + productionTableSize) * sizeof(PPART));
  productionTable[0] = NULL;

  if ((NUMPARTS == 0) || (NUMPARTS > productionTableSize)) {
    NUMPARTS = productionTableSize;
  }

  /* *************************************** */
  /* PART TABLE */
  /* *************************************** */

  pnum = 0;
  part = 0;

  partTableSize = 4; /* number of parts (including subparts) */
  partTable = (PART *) malloc((1 + partTableSize) * sizeof(PART));
  partTable[0].name = NULL; partTable[0].n = 0; partTable[0].X = NULL;
  for (i = 1; i <= partTableSize; i++)  {
    partTable[i].idx = i;
    partTable[i].color = 0;
  }

  /* *************************************** */
  /* JOB J1 */
  /* *************************************** */
  
  part++;

  pnum++; xnum = 0; onum = 0; /* next part, id = 1 */

  partTable[pnum].name = strdup("P1");
  partTable[pnum].part = part;
  partTable[pnum].n = 1; /* number of sequences */
  partTable[pnum].code = NULL; partTable[pnum].feasible = NULL;
  partTable[pnum].X = (SEQUENCE *) malloc((1 + partTable[pnum].n) * sizeof(SEQUENCE));
  partTable[pnum].X[0].n = 0; partTable[pnum].X[0].O = NULL;

  xnum++; onum = 0; /* next sequence */

  partTable[pnum].X[xnum].n = 3; /* number of items for the 1st sequence in J1 */
  partTable[pnum].X[xnum].O = (SEQUENCEITEM *) malloc((1 + partTable[pnum].X[xnum].n) * sizeof(SEQUENCEITEM));
  partTable[pnum].X[xnum].O[0].type = NONE; partTable[pnum].X[xnum].O[0].ITEM = NULL;
  partTable[pnum].X[xnum].O[++onum].type = DEFAULT; partTable[pnum].X[xnum].O[onum].ITEM = (void *) &(operationTable[1]); /* O1J1 */
  partTable[pnum].X[xnum].O[++onum].type = DEFAULT; partTable[pnum].X[xnum].O[onum].ITEM = (void *) &(operationTable[2]); /* O2J1 */
  partTable[pnum].X[xnum].O[++onum].type = DEFAULT; partTable[pnum].X[xnum].O[onum].ITEM = (void *) &(operationTable[3]); /* O3J1 */

  productionTable[part] = &(partTable[pnum]);
  partTable[pnum].color = 0;

  /* ADDVARIABLE(w,r) */
  ADDVARIABLE(1,1);
  ADDVARIABLE(2,2);
  ADDVARIABLE(3,3);

  /* *************************************** */
  /* JOB J2 */
  /* *************************************** */

  part++;

  pnum++; xnum = 0; onum = 0; /* next part, id = 2 */

  partTable[pnum].name = strdup("P2");
  partTable[pnum].part = part;
  partTable[pnum].n = 1; /* number of sequences */
  partTable[pnum].code = NULL; partTable[pnum].feasible = NULL;
  partTable[pnum].X = (SEQUENCE *) malloc((1 + partTable[pnum].n) * sizeof(SEQUENCE));
  partTable[pnum].X[0].n = 0; partTable[pnum].X[0].O = NULL;

  xnum++; onum = 0; /* next sequence */

  partTable[pnum].X[xnum].n = 3; /* number of items for the 1st sequence in J2 */
  partTable[pnum].X[xnum].O = (SEQUENCEITEM *) malloc((1 + partTable[pnum].X[xnum].n) * sizeof(SEQUENCEITEM));
  partTable[pnum].X[xnum].O[0].type = NONE; partTable[pnum].X[xnum].O[0].ITEM = NULL;
  partTable[pnum].X[xnum].O[++onum].type = DEFAULT; partTable[pnum].X[xnum].O[onum].ITEM = (void *) &(operationTable[4]); /* O1J2 */
  partTable[pnum].X[xnum].O[++onum].type = DEFAULT; partTable[pnum].X[xnum].O[onum].ITEM = (void *) &(operationTable[5]); /* O2J2 */
  partTable[pnum].X[xnum].O[++onum].type = DEFAULT; partTable[pnum].X[xnum].O[onum].ITEM = (void *) &(operationTable[6]); /* O3J2 */

  productionTable[part] = &(partTable[pnum]);
  partTable[pnum].color = 60;

  /* ADDVARIABLE(w,r) */
  ADDVARIABLE(4,1);
  ADDVARIABLE(5,2);
  ADDVARIABLE(6,3);

  /* *************************************** */
  /* JOB J3 */
  /* *************************************** */

  part++;

  pnum++; xnum = 0; onum = 0; /* next part, id = 3 */

  partTable[pnum].name = strdup("P3");
  partTable[pnum].part = part;
  partTable[pnum].n = 1; /* number of sequences */
  partTable[pnum].code = NULL; partTable[pnum].feasible = NULL;
  partTable[pnum].X = (SEQUENCE *) malloc((1 + partTable[pnum].n) * sizeof(SEQUENCE));
  partTable[pnum].X[0].n = 0; partTable[pnum].X[0].O = NULL;

  xnum++; onum = 0; /* next sequence */

  partTable[pnum].X[xnum].n = 4; /* number of items for the 1st sequence in J3 */
  partTable[pnum].X[xnum].O = (SEQUENCEITEM *) malloc((1 + partTable[pnum].X[xnum].n) * sizeof(SEQUENCEITEM));
  partTable[pnum].X[xnum].O[0].type = NONE; partTable[pnum].X[xnum].O[0].ITEM = NULL;
  partTable[pnum].X[xnum].O[++onum].type = DEFAULT; partTable[pnum].X[xnum].O[onum].ITEM = (void *) &(operationTable[7]); /* O1J3 */
  partTable[pnum].X[xnum].O[++onum].type = DEFAULT; partTable[pnum].X[xnum].O[onum].ITEM = (void *) &(operationTable[8]); /* O2J3 */
  partTable[pnum].X[xnum].O[++onum].type = DEFAULT; partTable[pnum].X[xnum].O[onum].ITEM = (void *) &(operationTable[9]); /* O3J3 */
  partTable[pnum].X[xnum].O[++onum].type = DEFAULT; partTable[pnum].X[xnum].O[onum].ITEM = (void *) &(operationTable[10]); /* O4J3 */

  productionTable[part] = &(partTable[pnum]);
  partTable[pnum].color = 140;

  /* ADDVARIABLE(w,r) */
  ADDVARIABLE(7,1);
  ADDVARIABLE(8,2);
  ADDVARIABLE(9,3);
  ADDVARIABLE(10,4);

  /* *************************************** */
  /* JOB J4 */
  /* *************************************** */

  part++;

  pnum++; xnum = 0; onum = 0; /* next part, id = 4 */

  partTable[pnum].name = strdup("P4");
  partTable[pnum].part = part;
  partTable[pnum].n = 1; /* number of sequences */
  partTable[pnum].code = NULL; partTable[pnum].feasible = NULL;
  partTable[pnum].X = (SEQUENCE *) malloc((1 + partTable[pnum].n) * sizeof(SEQUENCE));
  partTable[pnum].X[0].n = 0; partTable[pnum].X[0].O = NULL;

  xnum++; onum = 0; /* next sequence */

  partTable[pnum].X[xnum].n = 2; /* number of items for the 1st sequence in J4 */
  partTable[pnum].X[xnum].O = (SEQUENCEITEM *) malloc((1 + partTable[pnum].X[xnum].n) * sizeof(SEQUENCEITEM));
  partTable[pnum].X[xnum].O[0].type = NONE; partTable[pnum].X[xnum].O[0].ITEM = NULL;
  partTable[pnum].X[xnum].O[++onum].type = DEFAULT; partTable[pnum].X[xnum].O[onum].ITEM = (void *) &(operationTable[11]); /* O1J4 */
  partTable[pnum].X[xnum].O[++onum].type = DEFAULT; partTable[pnum].X[xnum].O[onum].ITEM = (void *) &(operationTable[12]); /* O2J4 */

  productionTable[part] = &(partTable[pnum]);
  partTable[pnum].color = 180;

  /* ADDVARIABLE(w,r) */
  ADDVARIABLE(11,1);
  ADDVARIABLE(12,2);
}

/* ************************************************************************************ */
/* ************************************************************************************ */
/* ************************************************************************************ */

void
generateSystemKacem8x8()
{
  unsigned int part,pnum,xnum,onum;
  unsigned int i,j;

  if (NUMPARTS == 0) NUMPARTS = 8;
  if (NUMMACHINES == 0) NUMMACHINES = 8;

  MAXSEQUENCELENGTH = 4;

  /* user defined parts, sequences, and operations start at index 1 */

  /* *************************************** */
  /* OPERATION TABLE */
  /* *************************************** */
  
  operationTableSize = 27; /* number of operations */
  operationTable = (OPERATION *) malloc((1 + operationTableSize) * sizeof(OPERATION));
  (operationTable)[0].name = NULL;
  for (i = 1; i <= operationTableSize; i++)  (operationTable)[i].idx = i;

  (operationTable)[1].name = strdup("O1J1");
  (operationTable)[2].name = strdup("O2J1");
  (operationTable)[3].name = strdup("O3J1");
  (operationTable)[4].name = strdup("O1J2");
  (operationTable)[5].name = strdup("O2J2");
  (operationTable)[6].name = strdup("O3J2");
  (operationTable)[7].name = strdup("O4J2");
  (operationTable)[8].name = strdup("O1J3");
  (operationTable)[9].name = strdup("O2J3");
  (operationTable)[10].name = strdup("O3J3");
  (operationTable)[11].name = strdup("O1J4");
  (operationTable)[12].name = strdup("O2J4");
  (operationTable)[13].name = strdup("O3J4");
  (operationTable)[14].name = strdup("O1J5");
  (operationTable)[15].name = strdup("O2J5");
  (operationTable)[16].name = strdup("O3J5");
  (operationTable)[17].name = strdup("O4J5");
  (operationTable)[18].name = strdup("O1J6");
  (operationTable)[19].name = strdup("O2J6");
  (operationTable)[20].name = strdup("O3J6");
  (operationTable)[21].name = strdup("O1J7");
  (operationTable)[22].name = strdup("O2J7");
  (operationTable)[23].name = strdup("O3J7");
  (operationTable)[24].name = strdup("O1J8");
  (operationTable)[25].name = strdup("O2J8");
  (operationTable)[26].name = strdup("O3J8");
  (operationTable)[27].name = strdup("O4J8");

  /* *************************************** */
  /* MACHINE TABLE */
  /* *************************************** */
  
  machineTableSize = 8; /* number of machines */
  machineTable = (MACHINE *) malloc((1 + machineTableSize) * sizeof(MACHINE));
  machineTable[0].name = NULL;
  for (i = 1; i <= machineTableSize; i++) machineTable[i].idx = i;

  machineTable[1].name = strdup("M1"); machineTable[1].n = 1;
  machineTable[2].name = strdup("M2"); machineTable[2].n = 1;
  machineTable[3].name = strdup("M3"); machineTable[3].n = 1;
  machineTable[4].name = strdup("M4"); machineTable[4].n = 1;
  machineTable[5].name = strdup("M5"); machineTable[5].n = 1;
  machineTable[6].name = strdup("M6"); machineTable[6].n = 1;
  machineTable[7].name = strdup("M7"); machineTable[7].n = 1;
  machineTable[8].name = strdup("M8"); machineTable[8].n = 1;

  if ((NUMMACHINES == 0) || (NUMMACHINES > machineTableSize)) {
    NUMMACHINES = machineTableSize;
  }

  /* *************************************** */
  /* OPERATION MATRIX */
  /* *************************************** */

  operationMatrix = (unsigned int *)
    malloc((1 + machineTableSize) * (1 + operationTableSize) * sizeof(unsigned int));

  for (i = 0; i <= operationTableSize; i++) {
    for (j = 0; j <= machineTableSize; j++) {
      (operationMatrix)[i + j * (1 + operationTableSize)] = 0; /* Oi @ mj */
    }
  }

  operationMatrix[1 + 1 * (1 + operationTableSize)] = 5; /* O1J1 @ M1 */
  operationMatrix[1 + 2 * (1 + operationTableSize)] = 3; /* O1J1 @ M2 */
  operationMatrix[1 + 3 * (1 + operationTableSize)] = 5; /* O1J1 @ M3 */
  operationMatrix[1 + 4 * (1 + operationTableSize)] = 3; /* O1J1 @ M4 */
  operationMatrix[1 + 5 * (1 + operationTableSize)] = 3; /* O1J1 @ M5 */
  operationMatrix[1 + 6 * (1 + operationTableSize)] = 0; /* O1J1 @ M6 */
  operationMatrix[1 + 7 * (1 + operationTableSize)] = 10; /* O1J1 @ M7 */
  operationMatrix[1 + 8 * (1 + operationTableSize)] = 9; /* O1J1 @ M8 */

  operationMatrix[2 + 1 * (1 + operationTableSize)] = 10; /* O2J1 @ M1 */
  operationMatrix[2 + 2 * (1 + operationTableSize)] = 0; /* O2J1 @ M2 */
  operationMatrix[2 + 3 * (1 + operationTableSize)] = 5; /* O2J1 @ M3 */
  operationMatrix[2 + 4 * (1 + operationTableSize)] = 8; /* O2J1 @ M4 */
  operationMatrix[2 + 5 * (1 + operationTableSize)] = 3; /* O2J1 @ M5 */
  operationMatrix[2 + 6 * (1 + operationTableSize)] = 9; /* O2J1 @ M6 */
  operationMatrix[2 + 7 * (1 + operationTableSize)] = 9; /* O2J1 @ M7 */
  operationMatrix[2 + 8 * (1 + operationTableSize)] = 6; /* O2J1 @ M8 */

  operationMatrix[3 + 1 * (1 + operationTableSize)] = 0; /* O3J1 @ M1 */
  operationMatrix[3 + 2 * (1 + operationTableSize)] = 10; /* O3J1 @ M2 */
  operationMatrix[3 + 3 * (1 + operationTableSize)] = 0; /* O3J1 @ M3 */
  operationMatrix[3 + 4 * (1 + operationTableSize)] = 5; /* O3J1 @ M4 */
  operationMatrix[3 + 5 * (1 + operationTableSize)] = 6; /* O3J1 @ M5 */
  operationMatrix[3 + 6 * (1 + operationTableSize)] = 2; /* O3J1 @ M6 */
  operationMatrix[3 + 7 * (1 + operationTableSize)] = 4; /* O3J1 @ M7 */
  operationMatrix[3 + 8 * (1 + operationTableSize)] = 5; /* O3J1 @ M8 */

  operationMatrix[4 + 1 * (1 + operationTableSize)] = 5; /* O1J2 @ M1 */
  operationMatrix[4 + 2 * (1 + operationTableSize)] = 7; /* O1J2 @ M2 */
  operationMatrix[4 + 3 * (1 + operationTableSize)] = 3; /* O1J2 @ M3 */
  operationMatrix[4 + 4 * (1 + operationTableSize)] = 9; /* O1J2 @ M4 */
  operationMatrix[4 + 5 * (1 + operationTableSize)] = 8; /* O1J2 @ M5 */
  operationMatrix[4 + 6 * (1 + operationTableSize)] = 0; /* O1J2 @ M6 */
  operationMatrix[4 + 7 * (1 + operationTableSize)] = 9; /* O1J2 @ M7 */
  operationMatrix[4 + 8 * (1 + operationTableSize)] = 0; /* O1J2 @ M8 */

  operationMatrix[5 + 1 * (1 + operationTableSize)] = 0; /* O2J2 @ M1 */
  operationMatrix[5 + 2 * (1 + operationTableSize)] = 8; /* O2J2 @ M2 */
  operationMatrix[5 + 3 * (1 + operationTableSize)] = 5; /* O2J2 @ M3 */
  operationMatrix[5 + 4 * (1 + operationTableSize)] = 2; /* O2J2 @ M4 */
  operationMatrix[5 + 5 * (1 + operationTableSize)] = 6; /* O2J2 @ M5 */
  operationMatrix[5 + 6 * (1 + operationTableSize)] = 7; /* O2J2 @ M6 */
  operationMatrix[5 + 7 * (1 + operationTableSize)] = 10; /* O2J2 @ M7 */
  operationMatrix[5 + 8 * (1 + operationTableSize)] = 9; /* O2J2 @ M8 */

  operationMatrix[6 + 1 * (1 + operationTableSize)] = 0; /* O3J2 @ M1 */
  operationMatrix[6 + 2 * (1 + operationTableSize)] = 10; /* O3J2 @ M2 */
  operationMatrix[6 + 3 * (1 + operationTableSize)] = 0; /* O3J2 @ M3 */
  operationMatrix[6 + 4 * (1 + operationTableSize)] = 5; /* O3J2 @ M4 */
  operationMatrix[6 + 5 * (1 + operationTableSize)] = 6; /* O3J2 @ M5 */
  operationMatrix[6 + 6 * (1 + operationTableSize)] = 4; /* O3J2 @ M6 */
  operationMatrix[6 + 7 * (1 + operationTableSize)] = 1; /* O3J2 @ M7 */
  operationMatrix[6 + 8 * (1 + operationTableSize)] = 7; /* O3J2 @ M8 */

  operationMatrix[7 + 1 * (1 + operationTableSize)] = 10; /* O4J2 @ M1 */
  operationMatrix[7 + 2 * (1 + operationTableSize)] = 8; /* O4J2 @ M2 */
  operationMatrix[7 + 3 * (1 + operationTableSize)] = 9; /* O4J2 @ M3 */
  operationMatrix[7 + 4 * (1 + operationTableSize)] = 6; /* O4J2 @ M4 */
  operationMatrix[7 + 5 * (1 + operationTableSize)] = 4; /* O4J2 @ M5 */
  operationMatrix[7 + 6 * (1 + operationTableSize)] = 7; /* O4J2 @ M6 */
  operationMatrix[7 + 7 * (1 + operationTableSize)] = 0; /* O4J2 @ M7 */
  operationMatrix[7 + 8 * (1 + operationTableSize)] = 0; /* O4J2 @ M8 */

  operationMatrix[8 + 1 * (1 + operationTableSize)] = 10; /* O1J3 @ M1 */
  operationMatrix[8 + 2 * (1 + operationTableSize)] = 0; /* O1J3 @ M2 */
  operationMatrix[8 + 3 * (1 + operationTableSize)] = 0; /* O1J3 @ M3 */
  operationMatrix[8 + 4 * (1 + operationTableSize)] = 7; /* O1J3 @ M4 */
  operationMatrix[8 + 5 * (1 + operationTableSize)] = 6; /* O1J3 @ M5 */
  operationMatrix[8 + 6 * (1 + operationTableSize)] = 5; /* O1J3 @ M6 */
  operationMatrix[8 + 7 * (1 + operationTableSize)] = 2; /* O1J3 @ M7 */
  operationMatrix[8 + 8 * (1 + operationTableSize)] = 4; /* O1J3 @ M8 */

  operationMatrix[9 + 1 * (1 + operationTableSize)] = 0; /* O2J3 @ M1 */
  operationMatrix[9 + 2 * (1 + operationTableSize)] = 10; /* O2J3 @ M2 */
  operationMatrix[9 + 3 * (1 + operationTableSize)] = 6; /* O2J3 @ M3 */
  operationMatrix[9 + 4 * (1 + operationTableSize)] = 4; /* O2J3 @ M4 */
  operationMatrix[9 + 5 * (1 + operationTableSize)] = 8; /* O2J3 @ M5 */
  operationMatrix[9 + 6 * (1 + operationTableSize)] = 9; /* O2J3 @ M6 */
  operationMatrix[9 + 7 * (1 + operationTableSize)] = 10; /* O2J3 @ M7 */
  operationMatrix[9 + 8 * (1 + operationTableSize)] = 0; /* O2J3 @ M8 */

  operationMatrix[10 + 1 * (1 + operationTableSize)] = 1; /* O3J3 @ M1 */
  operationMatrix[10 + 2 * (1 + operationTableSize)] = 4; /* O3J3 @ M2 */
  operationMatrix[10 + 3 * (1 + operationTableSize)] = 5; /* O3J3 @ M3 */
  operationMatrix[10 + 4 * (1 + operationTableSize)] = 6; /* O3J3 @ M4 */
  operationMatrix[10 + 5 * (1 + operationTableSize)] = 0; /* O3J3 @ M5 */
  operationMatrix[10 + 6 * (1 + operationTableSize)] = 10; /* O3J3 @ M6 */
  operationMatrix[10 + 7 * (1 + operationTableSize)] = 0; /* O3J3 @ M7 */
  operationMatrix[10 + 8 * (1 + operationTableSize)] = 7; /* O3J3 @ M8 */

  operationMatrix[11 + 1 * (1 + operationTableSize)] = 3; /* O1J4 @ M1 */
  operationMatrix[11 + 2 * (1 + operationTableSize)] = 1; /* O1J4 @ M2 */
  operationMatrix[11 + 3 * (1 + operationTableSize)] = 6; /* O1J4 @ M3 */
  operationMatrix[11 + 4 * (1 + operationTableSize)] = 5; /* O1J4 @ M4 */
  operationMatrix[11 + 5 * (1 + operationTableSize)] = 9; /* O1J4 @ M5 */
  operationMatrix[11 + 6 * (1 + operationTableSize)] = 7; /* O1J4 @ M6 */
  operationMatrix[11 + 7 * (1 + operationTableSize)] = 8; /* O1J4 @ M7 */
  operationMatrix[11 + 8 * (1 + operationTableSize)] = 4; /* O1J4 @ M8 */

  operationMatrix[12 + 1 * (1 + operationTableSize)] = 12; /* O2J4 @ M1 */
  operationMatrix[12 + 2 * (1 + operationTableSize)] = 11; /* O2J4 @ M2 */
  operationMatrix[12 + 3 * (1 + operationTableSize)] = 7; /* O2J4 @ M3 */
  operationMatrix[12 + 4 * (1 + operationTableSize)] = 8; /* O2J4 @ M4 */
  operationMatrix[12 + 5 * (1 + operationTableSize)] = 10; /* O2J4 @ M5 */
  operationMatrix[12 + 6 * (1 + operationTableSize)] = 5; /* O2J4 @ M6 */
  operationMatrix[12 + 7 * (1 + operationTableSize)] = 6; /* O2J4 @ M7 */
  operationMatrix[12 + 8 * (1 + operationTableSize)] = 9; /* O2J4 @ M8 */

  operationMatrix[13 + 1 * (1 + operationTableSize)] = 4; /* O3J4 @ M1 */
  operationMatrix[13 + 2 * (1 + operationTableSize)] = 6; /* O3J4 @ M2 */
  operationMatrix[13 + 3 * (1 + operationTableSize)] = 2; /* O3J4 @ M3 */
  operationMatrix[13 + 4 * (1 + operationTableSize)] = 10; /* O3J4 @ M4 */
  operationMatrix[13 + 5 * (1 + operationTableSize)] = 3; /* O3J4 @ M5 */
  operationMatrix[13 + 6 * (1 + operationTableSize)] = 9; /* O3J4 @ M6 */
  operationMatrix[13 + 7 * (1 + operationTableSize)] = 5; /* O3J4 @ M7 */
  operationMatrix[13 + 8 * (1 + operationTableSize)] = 7; /* O3J4 @ M8 */

  operationMatrix[14 + 1 * (1 + operationTableSize)] = 3; /* O1J5 @ M1 */
  operationMatrix[14 + 2 * (1 + operationTableSize)] = 6; /* O1J5 @ M2 */
  operationMatrix[14 + 3 * (1 + operationTableSize)] = 7; /* O1J5 @ M3 */
  operationMatrix[14 + 4 * (1 + operationTableSize)] = 8; /* O1J5 @ M4 */
  operationMatrix[14 + 5 * (1 + operationTableSize)] = 9; /* O1J5 @ M5 */
  operationMatrix[14 + 6 * (1 + operationTableSize)] = 0; /* O1J5 @ M6 */
  operationMatrix[14 + 7 * (1 + operationTableSize)] = 10; /* O1J5 @ M7 */
  operationMatrix[14 + 8 * (1 + operationTableSize)] = 0; /* O1J5 @ M8 */

  operationMatrix[15 + 1 * (1 + operationTableSize)] = 10; /* O2J5 @ M1 */
  operationMatrix[15 + 2 * (1 + operationTableSize)] = 0; /* O2J5 @ M2 */
  operationMatrix[15 + 3 * (1 + operationTableSize)] = 7; /* O2J5 @ M3 */
  operationMatrix[15 + 4 * (1 + operationTableSize)] = 4; /* O2J5 @ M4 */
  operationMatrix[15 + 5 * (1 + operationTableSize)] = 9; /* O2J5 @ M5 */
  operationMatrix[15 + 6 * (1 + operationTableSize)] = 8; /* O2J5 @ M6 */
  operationMatrix[15 + 7 * (1 + operationTableSize)] = 6; /* O2J5 @ M7 */
  operationMatrix[15 + 8 * (1 + operationTableSize)] = 0; /* O2J5 @ M8 */

  operationMatrix[16 + 1 * (1 + operationTableSize)] = 0; /* O3J5 @ M1 */
  operationMatrix[16 + 2 * (1 + operationTableSize)] = 9; /* O3J5 @ M2 */
  operationMatrix[16 + 3 * (1 + operationTableSize)] = 8; /* O3J5 @ M3 */
  operationMatrix[16 + 4 * (1 + operationTableSize)] = 7; /* O3J5 @ M4 */
  operationMatrix[16 + 5 * (1 + operationTableSize)] = 4; /* O3J5 @ M5 */
  operationMatrix[16 + 6 * (1 + operationTableSize)] = 2; /* O3J5 @ M6 */
  operationMatrix[16 + 7 * (1 + operationTableSize)] = 7; /* O3J5 @ M7 */
  operationMatrix[16 + 8 * (1 + operationTableSize)] = 0; /* O3J5 @ M8 */

  operationMatrix[17 + 1 * (1 + operationTableSize)] = 11; /* O4J5 @ M1 */
  operationMatrix[17 + 2 * (1 + operationTableSize)] = 9; /* O4J5 @ M2 */
  operationMatrix[17 + 3 * (1 + operationTableSize)] = 0; /* O4J5 @ M3 */
  operationMatrix[17 + 4 * (1 + operationTableSize)] = 6; /* O4J5 @ M4 */
  operationMatrix[17 + 5 * (1 + operationTableSize)] = 7; /* O4J5 @ M5 */
  operationMatrix[17 + 6 * (1 + operationTableSize)] = 5; /* O4J5 @ M6 */
  operationMatrix[17 + 7 * (1 + operationTableSize)] = 3; /* O4J5 @ M7 */
  operationMatrix[17 + 8 * (1 + operationTableSize)] = 6; /* O4J5 @ M8 */

  operationMatrix[18 + 1 * (1 + operationTableSize)] = 6; /* O1J6 @ M1 */
  operationMatrix[18 + 2 * (1 + operationTableSize)] = 7; /* O1J6 @ M2 */
  operationMatrix[18 + 3 * (1 + operationTableSize)] = 1; /* O1J6 @ M3 */
  operationMatrix[18 + 4 * (1 + operationTableSize)] = 4; /* O1J6 @ M4 */
  operationMatrix[18 + 5 * (1 + operationTableSize)] = 6; /* O1J6 @ M5 */
  operationMatrix[18 + 6 * (1 + operationTableSize)] = 9; /* O1J6 @ M6 */
  operationMatrix[18 + 7 * (1 + operationTableSize)] = 0; /* O1J6 @ M7 */
  operationMatrix[18 + 8 * (1 + operationTableSize)] = 10; /* O1J6 @ M8 */

  operationMatrix[19 + 1 * (1 + operationTableSize)] = 11; /* O2J6 @ M1 */
  operationMatrix[19 + 2 * (1 + operationTableSize)] = 0; /* O2J6 @ M2 */
  operationMatrix[19 + 3 * (1 + operationTableSize)] = 9; /* O2J6 @ M3 */
  operationMatrix[19 + 4 * (1 + operationTableSize)] = 9; /* O2J6 @ M4 */
  operationMatrix[19 + 5 * (1 + operationTableSize)] = 9; /* O2J6 @ M5 */
  operationMatrix[19 + 6 * (1 + operationTableSize)] = 7; /* O2J6 @ M6 */
  operationMatrix[19 + 7 * (1 + operationTableSize)] = 6; /* O2J6 @ M7 */
  operationMatrix[19 + 8 * (1 + operationTableSize)] = 4; /* O2J6 @ M8 */

  operationMatrix[20 + 1 * (1 + operationTableSize)] = 10; /* O3J6 @ M1 */
  operationMatrix[20 + 2 * (1 + operationTableSize)] = 5; /* O3J6 @ M2 */
  operationMatrix[20 + 3 * (1 + operationTableSize)] = 9; /* O3J6 @ M3 */
  operationMatrix[20 + 4 * (1 + operationTableSize)] = 10; /* O3J6 @ M4 */
  operationMatrix[20 + 5 * (1 + operationTableSize)] = 11; /* O3J6 @ M5 */
  operationMatrix[20 + 6 * (1 + operationTableSize)] = 0; /* O3J6 @ M6 */
  operationMatrix[20 + 7 * (1 + operationTableSize)] = 10; /* O3J6 @ M7 */
  operationMatrix[20 + 8 * (1 + operationTableSize)] = 0; /* O3J6 @ M8 */

  operationMatrix[21 + 1 * (1 + operationTableSize)] = 5; /* O1J7 @ M1 */
  operationMatrix[21 + 2 * (1 + operationTableSize)] = 4; /* O1J7 @ M2 */
  operationMatrix[21 + 3 * (1 + operationTableSize)] = 2; /* O1J7 @ M3 */
  operationMatrix[21 + 4 * (1 + operationTableSize)] = 6; /* O1J7 @ M4 */
  operationMatrix[21 + 5 * (1 + operationTableSize)] = 7; /* O1J7 @ M5 */
  operationMatrix[21 + 6 * (1 + operationTableSize)] = 0; /* O1J7 @ M6 */
  operationMatrix[21 + 7 * (1 + operationTableSize)] = 10; /* O1J7 @ M7 */
  operationMatrix[21 + 8 * (1 + operationTableSize)] = 0; /* O1J7 @ M8 */

  operationMatrix[22 + 1 * (1 + operationTableSize)] = 0; /* O2J7 @ M1 */
  operationMatrix[22 + 2 * (1 + operationTableSize)] = 9; /* O2J7 @ M2 */
  operationMatrix[22 + 3 * (1 + operationTableSize)] = 0; /* O2J7 @ M3 */
  operationMatrix[22 + 4 * (1 + operationTableSize)] = 9; /* O2J7 @ M4 */
  operationMatrix[22 + 5 * (1 + operationTableSize)] = 11; /* O2J7 @ M5 */
  operationMatrix[22 + 6 * (1 + operationTableSize)] = 9; /* O2J7 @ M6 */
  operationMatrix[22 + 7 * (1 + operationTableSize)] = 10; /* O2J7 @ M7 */
  operationMatrix[22 + 8 * (1 + operationTableSize)] = 5; /* O2J7 @ M8 */

  operationMatrix[23 + 1 * (1 + operationTableSize)] = 0; /* O3J7 @ M1 */
  operationMatrix[23 + 2 * (1 + operationTableSize)] = 8; /* O3J7 @ M2 */
  operationMatrix[23 + 3 * (1 + operationTableSize)] = 9; /* O3J7 @ M3 */
  operationMatrix[23 + 4 * (1 + operationTableSize)] = 3; /* O3J7 @ M4 */
  operationMatrix[23 + 5 * (1 + operationTableSize)] = 8; /* O3J7 @ M5 */
  operationMatrix[23 + 6 * (1 + operationTableSize)] = 6; /* O3J7 @ M6 */
  operationMatrix[23 + 7 * (1 + operationTableSize)] = 0; /* O3J7 @ M7 */
  operationMatrix[23 + 8 * (1 + operationTableSize)] = 10; /* O3J7 @ M8 */

  operationMatrix[24 + 1 * (1 + operationTableSize)] = 2; /* O1J8 @ M1 */
  operationMatrix[24 + 2 * (1 + operationTableSize)] = 8; /* O1J8 @ M2 */
  operationMatrix[24 + 3 * (1 + operationTableSize)] = 5; /* O1J8 @ M3 */
  operationMatrix[24 + 4 * (1 + operationTableSize)] = 9; /* O1J8 @ M4 */
  operationMatrix[24 + 5 * (1 + operationTableSize)] = 0; /* O1J8 @ M5 */
  operationMatrix[24 + 6 * (1 + operationTableSize)] = 4; /* O1J8 @ M6 */
  operationMatrix[24 + 7 * (1 + operationTableSize)] = 0; /* O1J8 @ M7 */
  operationMatrix[24 + 8 * (1 + operationTableSize)] = 10; /* O1J8 @ M8 */

  operationMatrix[25 + 1 * (1 + operationTableSize)] = 7; /* O2J8 @ M1 */
  operationMatrix[25 + 2 * (1 + operationTableSize)] = 4; /* O2J8 @ M2 */
  operationMatrix[25 + 3 * (1 + operationTableSize)] = 7; /* O2J8 @ M3 */
  operationMatrix[25 + 4 * (1 + operationTableSize)] = 8; /* O2J8 @ M4 */
  operationMatrix[25 + 5 * (1 + operationTableSize)] = 9; /* O2J8 @ M5 */
  operationMatrix[25 + 6 * (1 + operationTableSize)] = 0; /* O2J8 @ M6 */
  operationMatrix[25 + 7 * (1 + operationTableSize)] = 10; /* O2J8 @ M7 */
  operationMatrix[25 + 8 * (1 + operationTableSize)] = 0; /* O2J8 @ M8 */

  operationMatrix[26 + 1 * (1 + operationTableSize)] = 9; /* O3J8 @ M1 */
  operationMatrix[26 + 2 * (1 + operationTableSize)] = 9; /* O3J8 @ M2 */
  operationMatrix[26 + 3 * (1 + operationTableSize)] = 0; /* O3J8 @ M3 */
  operationMatrix[26 + 4 * (1 + operationTableSize)] = 8; /* O3J8 @ M4 */
  operationMatrix[26 + 5 * (1 + operationTableSize)] = 5; /* O3J8 @ M5 */
  operationMatrix[26 + 6 * (1 + operationTableSize)] = 6; /* O3J8 @ M6 */
  operationMatrix[26 + 7 * (1 + operationTableSize)] = 7; /* O3J8 @ M7 */
  operationMatrix[26 + 8 * (1 + operationTableSize)] = 1; /* O3J8 @ M8 */

  operationMatrix[27 + 1 * (1 + operationTableSize)] = 9; /* O4J8 @ M1 */
  operationMatrix[27 + 2 * (1 + operationTableSize)] = 0; /* O4J8 @ M2 */
  operationMatrix[27 + 3 * (1 + operationTableSize)] = 3; /* O4J8 @ M3 */
  operationMatrix[27 + 4 * (1 + operationTableSize)] = 7; /* O4J8 @ M4 */
  operationMatrix[27 + 5 * (1 + operationTableSize)] = 1; /* O4J8 @ M5 */
  operationMatrix[27 + 6 * (1 + operationTableSize)] = 5; /* O4J8 @ M6 */
  operationMatrix[27 + 7 * (1 + operationTableSize)] = 8; /* O4J8 @ M7 */
  operationMatrix[27 + 8 * (1 + operationTableSize)] = 0; /* O4J8 @ M8 */

  /* *************************************** */
  /* PRODUCTION TABLE */
  /* *************************************** */

  productionTableSize = 8;
  productionTable = (PPART *) malloc((1 + productionTableSize) * sizeof(PPART));
  productionTable[0] = NULL;

  if ((NUMPARTS == 0) || (NUMPARTS > productionTableSize)) {
    NUMPARTS = productionTableSize;
  }

  /* *************************************** */
  /* PART TABLE */
  /* *************************************** */

  pnum = 0;
  part = 0;

  partTableSize = 8; /* number of parts (including subparts) */
  partTable = (PART *) malloc((1 + partTableSize) * sizeof(PART));
  partTable[0].name = NULL; partTable[0].n = 0; partTable[0].X = NULL;
  for (i = 1; i <= partTableSize; i++)  {
    partTable[i].idx = i;
    partTable[i].color = 0;
  }

  /* *************************************** */
  /* JOB J1 */
  /* *************************************** */
  
  part++;

  pnum++; xnum = 0; onum = 0; /* next part, id = 1 */

  partTable[pnum].name = strdup("P1");
  partTable[pnum].part = part;
  partTable[pnum].n = 1; /* number of sequences */
  partTable[pnum].code = NULL; partTable[pnum].feasible = NULL;
  partTable[pnum].X = (SEQUENCE *) malloc((1 + partTable[pnum].n) * sizeof(SEQUENCE));
  partTable[pnum].X[0].n = 0; partTable[pnum].X[0].O = NULL;

  xnum++; onum = 0; /* next sequence */

  partTable[pnum].X[xnum].n = 3; /* number of items for the 1st sequence in J1 */
  partTable[pnum].X[xnum].O = (SEQUENCEITEM *) malloc((1 + partTable[pnum].X[xnum].n) * sizeof(SEQUENCEITEM));
  partTable[pnum].X[xnum].O[0].type = NONE; partTable[pnum].X[xnum].O[0].ITEM = NULL;
  partTable[pnum].X[xnum].O[++onum].type = DEFAULT; partTable[pnum].X[xnum].O[onum].ITEM = (void *) &(operationTable[1]); /* O1J1 */
  partTable[pnum].X[xnum].O[++onum].type = DEFAULT; partTable[pnum].X[xnum].O[onum].ITEM = (void *) &(operationTable[2]); /* O2J1 */
  partTable[pnum].X[xnum].O[++onum].type = DEFAULT; partTable[pnum].X[xnum].O[onum].ITEM = (void *) &(operationTable[3]); /* O3J1 */

  productionTable[part] = &(partTable[pnum]);
  partTable[pnum].color = 0;

  /* ADDVARIABLE(w,r) */
  ADDVARIABLE(1,1);
  ADDVARIABLE(2,2);
  ADDVARIABLE(3,3);

  /* *************************************** */
  /* JOB J2 */
  /* *************************************** */

  part++;

  pnum++; xnum = 0; onum = 0; /* next part, id = 2 */

  partTable[pnum].name = strdup("P2");
  partTable[pnum].part = part;
  partTable[pnum].n = 1; /* number of sequences */
  partTable[pnum].code = NULL; partTable[pnum].feasible = NULL;
  partTable[pnum].X = (SEQUENCE *) malloc((1 + partTable[pnum].n) * sizeof(SEQUENCE));
  partTable[pnum].X[0].n = 0; partTable[pnum].X[0].O = NULL;

  xnum++; onum = 0; /* next sequence */

  partTable[pnum].X[xnum].n = 4; /* number of items for the 1st sequence in J2 */
  partTable[pnum].X[xnum].O = (SEQUENCEITEM *) malloc((1 + partTable[pnum].X[xnum].n) * sizeof(SEQUENCEITEM));
  partTable[pnum].X[xnum].O[0].type = NONE; partTable[pnum].X[xnum].O[0].ITEM = NULL;
  partTable[pnum].X[xnum].O[++onum].type = DEFAULT; partTable[pnum].X[xnum].O[onum].ITEM = (void *) &(operationTable[4]); /* O1J2 */
  partTable[pnum].X[xnum].O[++onum].type = DEFAULT; partTable[pnum].X[xnum].O[onum].ITEM = (void *) &(operationTable[5]); /* O2J2 */
  partTable[pnum].X[xnum].O[++onum].type = DEFAULT; partTable[pnum].X[xnum].O[onum].ITEM = (void *) &(operationTable[6]); /* O3J2 */
  partTable[pnum].X[xnum].O[++onum].type = DEFAULT; partTable[pnum].X[xnum].O[onum].ITEM = (void *) &(operationTable[7]); /* O4J2 */

  productionTable[part] = &(partTable[pnum]);
  partTable[pnum].color = 60;

  /* ADDVARIABLE(w,r) */
  ADDVARIABLE(4,1);
  ADDVARIABLE(5,2);
  ADDVARIABLE(6,3);
  ADDVARIABLE(7,4);

  /* *************************************** */
  /* JOB J3 */
  /* *************************************** */

  part++;

  pnum++; xnum = 0; onum = 0; /* next part, id = 3 */

  partTable[pnum].name = strdup("P3");
  partTable[pnum].part = part;
  partTable[pnum].n = 1; /* number of sequences */
  partTable[pnum].code = NULL; partTable[pnum].feasible = NULL;
  partTable[pnum].X = (SEQUENCE *) malloc((1 + partTable[pnum].n) * sizeof(SEQUENCE));
  partTable[pnum].X[0].n = 0; partTable[pnum].X[0].O = NULL;

  xnum++; onum = 0; /* next sequence */

  partTable[pnum].X[xnum].n = 3; /* number of items for the 1st sequence in J3 */
  partTable[pnum].X[xnum].O = (SEQUENCEITEM *) malloc((1 + partTable[pnum].X[xnum].n) * sizeof(SEQUENCEITEM));
  partTable[pnum].X[xnum].O[0].type = NONE; partTable[pnum].X[xnum].O[0].ITEM = NULL;
  partTable[pnum].X[xnum].O[++onum].type = DEFAULT; partTable[pnum].X[xnum].O[onum].ITEM = (void *) &(operationTable[8]); /* O1J3 */
  partTable[pnum].X[xnum].O[++onum].type = DEFAULT; partTable[pnum].X[xnum].O[onum].ITEM = (void *) &(operationTable[9]); /* O2J3 */
  partTable[pnum].X[xnum].O[++onum].type = DEFAULT; partTable[pnum].X[xnum].O[onum].ITEM = (void *) &(operationTable[10]); /* O3J3 */

  productionTable[part] = &(partTable[pnum]);
  partTable[pnum].color = 140;

  /* ADDVARIABLE(w,r) */
  ADDVARIABLE(8,1);
  ADDVARIABLE(9,2);
  ADDVARIABLE(10,3);

  /* *************************************** */
  /* JOB J4 */
  /* *************************************** */

  part++;

  pnum++; xnum = 0; onum = 0; /* next part, id = 4 */

  partTable[pnum].name = strdup("P4");
  partTable[pnum].part = part;
  partTable[pnum].n = 1; /* number of sequences */
  partTable[pnum].code = NULL; partTable[pnum].feasible = NULL;
  partTable[pnum].X = (SEQUENCE *) malloc((1 + partTable[pnum].n) * sizeof(SEQUENCE));
  partTable[pnum].X[0].n = 0; partTable[pnum].X[0].O = NULL;

  xnum++; onum = 0; /* next sequence */

  partTable[pnum].X[xnum].n = 3; /* number of items for the 1st sequence in J4 */
  partTable[pnum].X[xnum].O = (SEQUENCEITEM *) malloc((1 + partTable[pnum].X[xnum].n) * sizeof(SEQUENCEITEM));
  partTable[pnum].X[xnum].O[0].type = NONE; partTable[pnum].X[xnum].O[0].ITEM = NULL;
  partTable[pnum].X[xnum].O[++onum].type = DEFAULT; partTable[pnum].X[xnum].O[onum].ITEM = (void *) &(operationTable[11]); /* O1J4 */
  partTable[pnum].X[xnum].O[++onum].type = DEFAULT; partTable[pnum].X[xnum].O[onum].ITEM = (void *) &(operationTable[12]); /* O2J4 */
  partTable[pnum].X[xnum].O[++onum].type = DEFAULT; partTable[pnum].X[xnum].O[onum].ITEM = (void *) &(operationTable[13]); /* O3J4 */

  productionTable[part] = &(partTable[pnum]);
  partTable[pnum].color = 180;

  /* ADDVARIABLE(w,r) */
  ADDVARIABLE(11,1);
  ADDVARIABLE(12,2);
  ADDVARIABLE(13,3);

  /* *************************************** */
  /* JOB J5 */
  /* *************************************** */

  part++;

  pnum++; xnum = 0; onum = 0; /* next part, id = 5 */

  partTable[pnum].name = strdup("P5");
  partTable[pnum].part = part;
  partTable[pnum].n = 1; /* number of sequences */
  partTable[pnum].code = NULL; partTable[pnum].feasible = NULL;
  partTable[pnum].X = (SEQUENCE *) malloc((1 + partTable[pnum].n) * sizeof(SEQUENCE));
  partTable[pnum].X[0].n = 0; partTable[pnum].X[0].O = NULL;

  xnum++; onum = 0; /* next sequence */

  partTable[pnum].X[xnum].n = 4; /* number of items for the 1st sequence in J5 */
  partTable[pnum].X[xnum].O = (SEQUENCEITEM *) malloc((1 + partTable[pnum].X[xnum].n) * sizeof(SEQUENCEITEM));
  partTable[pnum].X[xnum].O[0].type = NONE; partTable[pnum].X[xnum].O[0].ITEM = NULL;
  partTable[pnum].X[xnum].O[++onum].type = DEFAULT; partTable[pnum].X[xnum].O[onum].ITEM = (void *) &(operationTable[14]); /* O1J5 */
  partTable[pnum].X[xnum].O[++onum].type = DEFAULT; partTable[pnum].X[xnum].O[onum].ITEM = (void *) &(operationTable[15]); /* O2J5 */
  partTable[pnum].X[xnum].O[++onum].type = DEFAULT; partTable[pnum].X[xnum].O[onum].ITEM = (void *) &(operationTable[16]); /* O3J5 */
  partTable[pnum].X[xnum].O[++onum].type = DEFAULT; partTable[pnum].X[xnum].O[onum].ITEM = (void *) &(operationTable[17]); /* O4J5 */

  productionTable[part] = &(partTable[pnum]);
  partTable[pnum].color = 230;

  /* ADDVARIABLE(w,r) */
  ADDVARIABLE(14,1);
  ADDVARIABLE(15,2);
  ADDVARIABLE(16,3);
  ADDVARIABLE(17,4);

  /* *************************************** */
  /* JOB J6 */
  /* *************************************** */

  part++;

  pnum++; xnum = 0; onum = 0; /* next part, id = 6 */

  partTable[pnum].name = strdup("P6");
  partTable[pnum].part = part;
  partTable[pnum].n = 1; /* number of sequences */
  partTable[pnum].code = NULL; partTable[pnum].feasible = NULL;
  partTable[pnum].X = (SEQUENCE *) malloc((1 + partTable[pnum].n) * sizeof(SEQUENCE));
  partTable[pnum].X[0].n = 0; partTable[pnum].X[0].O = NULL;

  xnum++; onum = 0; /* next sequence */

  partTable[pnum].X[xnum].n = 3; /* number of items for the 1st sequence in J6 */
  partTable[pnum].X[xnum].O = (SEQUENCEITEM *) malloc((1 + partTable[pnum].X[xnum].n) * sizeof(SEQUENCEITEM));
  partTable[pnum].X[xnum].O[0].type = NONE; partTable[pnum].X[xnum].O[0].ITEM = NULL;
  partTable[pnum].X[xnum].O[++onum].type = DEFAULT; partTable[pnum].X[xnum].O[onum].ITEM = (void *) &(operationTable[18]); /* O1J6 */
  partTable[pnum].X[xnum].O[++onum].type = DEFAULT; partTable[pnum].X[xnum].O[onum].ITEM = (void *) &(operationTable[19]); /* O2J6 */
  partTable[pnum].X[xnum].O[++onum].type = DEFAULT; partTable[pnum].X[xnum].O[onum].ITEM = (void *) &(operationTable[20]); /* O3J6 */

  productionTable[part] = &(partTable[pnum]);
  partTable[pnum].color = 300;

  /* ADDVARIABLE(w,r) */
  ADDVARIABLE(18,1);
  ADDVARIABLE(19,2);
  ADDVARIABLE(20,3);

  /* *************************************** */
  /* JOB J7 */
  /* *************************************** */

  part++;

  pnum++; xnum = 0; onum = 0; /* next part, id = 7 */

  partTable[pnum].name = strdup("P7");
  partTable[pnum].part = part;
  partTable[pnum].n = 1; /* number of sequences */
  partTable[pnum].code = NULL; partTable[pnum].feasible = NULL;
  partTable[pnum].X = (SEQUENCE *) malloc((1 + partTable[pnum].n) * sizeof(SEQUENCE));
  partTable[pnum].X[0].n = 0; partTable[pnum].X[0].O = NULL;

  xnum++; onum = 0; /* next sequence */

  partTable[pnum].X[xnum].n = 3; /* number of items for the 1st sequence in J7 */
  partTable[pnum].X[xnum].O = (SEQUENCEITEM *) malloc((1 + partTable[pnum].X[xnum].n) * sizeof(SEQUENCEITEM));
  partTable[pnum].X[xnum].O[0].type = NONE; partTable[pnum].X[xnum].O[0].ITEM = NULL;
  partTable[pnum].X[xnum].O[++onum].type = DEFAULT; partTable[pnum].X[xnum].O[onum].ITEM = (void *) &(operationTable[21]); /* O1J7 */
  partTable[pnum].X[xnum].O[++onum].type = DEFAULT; partTable[pnum].X[xnum].O[onum].ITEM = (void *) &(operationTable[22]); /* O2J7 */
  partTable[pnum].X[xnum].O[++onum].type = DEFAULT; partTable[pnum].X[xnum].O[onum].ITEM = (void *) &(operationTable[23]); /* O3J7 */

  productionTable[part] = &(partTable[pnum]);
  partTable[pnum].color = 30;

  /* ADDVARIABLE(w,r) */
  ADDVARIABLE(21,1);
  ADDVARIABLE(22,2);
  ADDVARIABLE(23,3);

  /* *************************************** */
  /* JOB J8 */
  /* *************************************** */

  part++;

  pnum++; xnum = 0; onum = 0; /* next part, id = 8 */

  partTable[pnum].name = strdup("P8");
  partTable[pnum].part = part;
  partTable[pnum].n = 1; /* number of sequences */
  partTable[pnum].code = NULL; partTable[pnum].feasible = NULL;
  partTable[pnum].X = (SEQUENCE *) malloc((1 + partTable[pnum].n) * sizeof(SEQUENCE));
  partTable[pnum].X[0].n = 0; partTable[pnum].X[0].O = NULL;

  xnum++; onum = 0; /* next sequence */

  partTable[pnum].X[xnum].n = 4; /* number of items for the 1st sequence in J8 */
  partTable[pnum].X[xnum].O = (SEQUENCEITEM *) malloc((1 + partTable[pnum].X[xnum].n) * sizeof(SEQUENCEITEM));
  partTable[pnum].X[xnum].O[0].type = NONE; partTable[pnum].X[xnum].O[0].ITEM = NULL;
  partTable[pnum].X[xnum].O[++onum].type = DEFAULT; partTable[pnum].X[xnum].O[onum].ITEM = (void *) &(operationTable[24]); /* O1J8 */
  partTable[pnum].X[xnum].O[++onum].type = DEFAULT; partTable[pnum].X[xnum].O[onum].ITEM = (void *) &(operationTable[25]); /* O2J8 */
  partTable[pnum].X[xnum].O[++onum].type = DEFAULT; partTable[pnum].X[xnum].O[onum].ITEM = (void *) &(operationTable[26]); /* O3J8 */
  partTable[pnum].X[xnum].O[++onum].type = DEFAULT; partTable[pnum].X[xnum].O[onum].ITEM = (void *) &(operationTable[27]); /* O4J8 */

  productionTable[part] = &(partTable[pnum]);
  partTable[pnum].color = 270;

  /* ADDVARIABLE(w,r) */
  ADDVARIABLE(24,1);
  ADDVARIABLE(25,2);
  ADDVARIABLE(26,3);
  ADDVARIABLE(27,4);
}

/* ************************************************************************************ */
/* ************************************************************************************ */
/* ************************************************************************************ */

void
generateSystemKacem10x7()
{
  unsigned int part,pnum,xnum,onum;
  unsigned int i,j;

  if (NUMPARTS == 0) NUMPARTS = 10;
  if (NUMMACHINES == 0) NUMMACHINES = 7;

  MAXSEQUENCELENGTH = 3;

  /* user defined parts, sequences, and operations start at index 1 */

  /* *************************************** */
  /* OPERATION TABLE */
  /* *************************************** */
  
  operationTableSize = 29; /* number of operations */
  operationTable = (OPERATION *) malloc((1 + operationTableSize) * sizeof(OPERATION));
  (operationTable)[0].name = NULL;
  for (i = 1; i <= operationTableSize; i++)  (operationTable)[i].idx = i;

  (operationTable)[1].name = strdup("O1J1");
  (operationTable)[2].name = strdup("O2J1");
  (operationTable)[3].name = strdup("O3J1");
  (operationTable)[4].name = strdup("O1J2");
  (operationTable)[5].name = strdup("O2J2");
  (operationTable)[6].name = strdup("O1J3");
  (operationTable)[7].name = strdup("O2J3");
  (operationTable)[8].name = strdup("O3J3");
  (operationTable)[9].name = strdup("O1J4");
  (operationTable)[10].name = strdup("O2J4");
  (operationTable)[11].name = strdup("O3J4");
  (operationTable)[12].name = strdup("O1J5");
  (operationTable)[13].name = strdup("O2J5");
  (operationTable)[14].name = strdup("O3J5");
  (operationTable)[15].name = strdup("O1J6");
  (operationTable)[16].name = strdup("O2J6");
  (operationTable)[17].name = strdup("O3J6");
  (operationTable)[18].name = strdup("O1J7");
  (operationTable)[19].name = strdup("O2J7");
  (operationTable)[20].name = strdup("O3J7");
  (operationTable)[21].name = strdup("O1J8");
  (operationTable)[22].name = strdup("O2J8");
  (operationTable)[23].name = strdup("O3J8");
  (operationTable)[24].name = strdup("O1J9");
  (operationTable)[25].name = strdup("O2J9");
  (operationTable)[26].name = strdup("O3J9");
  (operationTable)[27].name = strdup("O1J10");
  (operationTable)[28].name = strdup("O2J10");
  (operationTable)[29].name = strdup("O3J10");

  /* *************************************** */
  /* MACHINE TABLE */
  /* *************************************** */
  
  machineTableSize = 7; /* number of machines */
  machineTable = (MACHINE *) malloc((1 + machineTableSize) * sizeof(MACHINE));
  machineTable[0].name = NULL;
  for (i = 1; i <= machineTableSize; i++) machineTable[i].idx = i;

  machineTable[1].name = strdup("M1"); machineTable[1].n = 1;
  machineTable[2].name = strdup("M2"); machineTable[2].n = 1;
  machineTable[3].name = strdup("M3"); machineTable[3].n = 1;
  machineTable[4].name = strdup("M4"); machineTable[4].n = 1;
  machineTable[5].name = strdup("M5"); machineTable[5].n = 1;
  machineTable[6].name = strdup("M6"); machineTable[6].n = 1;
  machineTable[7].name = strdup("M7"); machineTable[7].n = 1;

  if ((NUMMACHINES == 0) || (NUMMACHINES > machineTableSize)) {
    NUMMACHINES = machineTableSize;
  }

  /* *************************************** */
  /* OPERATION MATRIX */
  /* *************************************** */

  operationMatrix = (unsigned int *)
    malloc((1 + machineTableSize) * (1 + operationTableSize) * sizeof(unsigned int));

  for (i = 0; i <= operationTableSize; i++) {
    for (j = 0; j <= machineTableSize; j++) {
      (operationMatrix)[i + j * (1 + operationTableSize)] = 0; /* Oi @ mj */
    }
  }

  operationMatrix[1 + 1 * (1 + operationTableSize)] = 1; /* O1J1 @ M1 */
  operationMatrix[1 + 2 * (1 + operationTableSize)] = 4; /* O1J1 @ M2 */
  operationMatrix[1 + 3 * (1 + operationTableSize)] = 6; /* O1J1 @ M3 */
  operationMatrix[1 + 4 * (1 + operationTableSize)] = 9; /* O1J1 @ M4 */
  operationMatrix[1 + 5 * (1 + operationTableSize)] = 3; /* O1J1 @ M5 */
  operationMatrix[1 + 6 * (1 + operationTableSize)] = 5; /* O1J1 @ M6 */
  operationMatrix[1 + 7 * (1 + operationTableSize)] = 2; /* O1J1 @ M7 */

  operationMatrix[2 + 1 * (1 + operationTableSize)] = 8; /* O2J1 @ M1 */
  operationMatrix[2 + 2 * (1 + operationTableSize)] = 9; /* O2J1 @ M2 */
  operationMatrix[2 + 3 * (1 + operationTableSize)] = 5; /* O2J1 @ M3 */
  operationMatrix[2 + 4 * (1 + operationTableSize)] = 4; /* O2J1 @ M4 */
  operationMatrix[2 + 5 * (1 + operationTableSize)] = 1; /* O2J1 @ M5 */
  operationMatrix[2 + 6 * (1 + operationTableSize)] = 1; /* O2J1 @ M6 */
  operationMatrix[2 + 7 * (1 + operationTableSize)] = 3; /* O2J1 @ M7 */

  operationMatrix[3 + 1 * (1 + operationTableSize)] = 4; /* O3J1 @ M1 */
  operationMatrix[3 + 2 * (1 + operationTableSize)] = 8; /* O3J1 @ M2 */
  operationMatrix[3 + 3 * (1 + operationTableSize)] = 10; /* O3J1 @ M3 */
  operationMatrix[3 + 4 * (1 + operationTableSize)] = 4; /* O3J1 @ M4 */
  operationMatrix[3 + 5 * (1 + operationTableSize)] = 11; /* O3J1 @ M5 */
  operationMatrix[3 + 6 * (1 + operationTableSize)] = 4; /* O3J1 @ M6 */
  operationMatrix[3 + 7 * (1 + operationTableSize)] = 3; /* O3J1 @ M7 */

  operationMatrix[4 + 1 * (1 + operationTableSize)] = 6; /* O1J2 @ M1 */
  operationMatrix[4 + 2 * (1 + operationTableSize)] = 9; /* O1J2 @ M2 */
  operationMatrix[4 + 3 * (1 + operationTableSize)] = 8; /* O1J2 @ M3 */
  operationMatrix[4 + 4 * (1 + operationTableSize)] = 6; /* O1J2 @ M4 */
  operationMatrix[4 + 5 * (1 + operationTableSize)] = 5; /* O1J2 @ M5 */
  operationMatrix[4 + 6 * (1 + operationTableSize)] = 10; /* O1J2 @ M6 */
  operationMatrix[4 + 7 * (1 + operationTableSize)] = 3; /* O1J2 @ M7 */

  operationMatrix[5 + 1 * (1 + operationTableSize)] = 2; /* O2J2 @ M1 */
  operationMatrix[5 + 2 * (1 + operationTableSize)] = 10; /* O2J2 @ M2 */
  operationMatrix[5 + 3 * (1 + operationTableSize)] = 4; /* O2J2 @ M3 */
  operationMatrix[5 + 4 * (1 + operationTableSize)] = 5; /* O2J2 @ M4 */
  operationMatrix[5 + 5 * (1 + operationTableSize)] = 9; /* O2J2 @ M5 */
  operationMatrix[5 + 6 * (1 + operationTableSize)] = 8; /* O2J2 @ M6 */
  operationMatrix[5 + 7 * (1 + operationTableSize)] = 4; /* O2J2 @ M7 */

  operationMatrix[6 + 1 * (1 + operationTableSize)] = 15; /* O1J3 @ M1 */
  operationMatrix[6 + 2 * (1 + operationTableSize)] = 4; /* O1J3 @ M2 */
  operationMatrix[6 + 3 * (1 + operationTableSize)] = 8; /* O1J3 @ M3 */
  operationMatrix[6 + 4 * (1 + operationTableSize)] = 4; /* O1J3 @ M4 */
  operationMatrix[6 + 5 * (1 + operationTableSize)] = 8; /* O1J3 @ M5 */
  operationMatrix[6 + 6 * (1 + operationTableSize)] = 7; /* O1J3 @ M6 */
  operationMatrix[6 + 7 * (1 + operationTableSize)] = 1; /* O1J3 @ M7 */

  operationMatrix[7 + 1 * (1 + operationTableSize)] = 9; /* O2J3 @ M1 */
  operationMatrix[7 + 2 * (1 + operationTableSize)] = 6; /* O2J3 @ M2 */
  operationMatrix[7 + 3 * (1 + operationTableSize)] = 1; /* O2J3 @ M3 */
  operationMatrix[7 + 4 * (1 + operationTableSize)] = 10; /* O2J3 @ M4 */
  operationMatrix[7 + 5 * (1 + operationTableSize)] = 7; /* O2J3 @ M5 */
  operationMatrix[7 + 6 * (1 + operationTableSize)] = 1; /* O2J3 @ M6 */
  operationMatrix[7 + 7 * (1 + operationTableSize)] = 6; /* O2J3 @ M7 */

  operationMatrix[8 + 1 * (1 + operationTableSize)] = 11; /* O3J3 @ M1 */
  operationMatrix[8 + 2 * (1 + operationTableSize)] = 2; /* O3J3 @ M2 */
  operationMatrix[8 + 3 * (1 + operationTableSize)] = 7; /* O3J3 @ M3 */
  operationMatrix[8 + 4 * (1 + operationTableSize)] = 5; /* O3J3 @ M4 */
  operationMatrix[8 + 5 * (1 + operationTableSize)] = 2; /* O3J3 @ M5 */
  operationMatrix[8 + 6 * (1 + operationTableSize)] = 3; /* O3J3 @ M6 */
  operationMatrix[8 + 7 * (1 + operationTableSize)] = 14; /* O3J3 @ M7 */

  operationMatrix[9 + 1 * (1 + operationTableSize)] = 2; /* O1J4 @ M1 */
  operationMatrix[9 + 2 * (1 + operationTableSize)] = 8; /* O1J4 @ M2 */
  operationMatrix[9 + 3 * (1 + operationTableSize)] = 5; /* O1J4 @ M3 */
  operationMatrix[9 + 4 * (1 + operationTableSize)] = 8; /* O1J4 @ M4 */
  operationMatrix[9 + 5 * (1 + operationTableSize)] = 9; /* O1J4 @ M5 */
  operationMatrix[9 + 6 * (1 + operationTableSize)] = 4; /* O1J4 @ M6 */
  operationMatrix[9 + 7 * (1 + operationTableSize)] = 3; /* O1J4 @ M7 */

  operationMatrix[10 + 1 * (1 + operationTableSize)] = 5; /* O2J4 @ M1 */
  operationMatrix[10 + 2 * (1 + operationTableSize)] = 3; /* O2J4 @ M2 */
  operationMatrix[10 + 3 * (1 + operationTableSize)] = 8; /* O2J4 @ M3 */
  operationMatrix[10 + 4 * (1 + operationTableSize)] = 1; /* O2J4 @ M4 */
  operationMatrix[10 + 5 * (1 + operationTableSize)] = 9; /* O2J4 @ M5 */
  operationMatrix[10 + 6 * (1 + operationTableSize)] = 3; /* O2J4 @ M6 */
  operationMatrix[10 + 7 * (1 + operationTableSize)] = 6; /* O2J4 @ M7 */

  operationMatrix[11 + 1 * (1 + operationTableSize)] = 1; /* O3J4 @ M1 */
  operationMatrix[11 + 2 * (1 + operationTableSize)] = 2; /* O3J4 @ M2 */
  operationMatrix[11 + 3 * (1 + operationTableSize)] = 6; /* O3J4 @ M3 */
  operationMatrix[11 + 4 * (1 + operationTableSize)] = 4; /* O3J4 @ M4 */
  operationMatrix[11 + 5 * (1 + operationTableSize)] = 1; /* O3J4 @ M5 */
  operationMatrix[11 + 6 * (1 + operationTableSize)] = 7; /* O3J4 @ M6 */
  operationMatrix[11 + 7 * (1 + operationTableSize)] = 2; /* O3J4 @ M7 */

  operationMatrix[12 + 1 * (1 + operationTableSize)] = 7; /* O1J5 @ M1 */
  operationMatrix[12 + 2 * (1 + operationTableSize)] = 1; /* O1J5 @ M2 */
  operationMatrix[12 + 3 * (1 + operationTableSize)] = 8; /* O1J5 @ M3 */
  operationMatrix[12 + 4 * (1 + operationTableSize)] = 5; /* O1J5 @ M4 */
  operationMatrix[12 + 5 * (1 + operationTableSize)] = 4; /* O1J5 @ M5 */
  operationMatrix[12 + 6 * (1 + operationTableSize)] = 3; /* O1J5 @ M6 */
  operationMatrix[12 + 7 * (1 + operationTableSize)] = 9; /* O1J5 @ M7 */

  operationMatrix[13 + 1 * (1 + operationTableSize)] = 2; /* O2J5 @ M1 */
  operationMatrix[13 + 2 * (1 + operationTableSize)] = 4; /* O2J5 @ M2 */
  operationMatrix[13 + 3 * (1 + operationTableSize)] = 5; /* O2J5 @ M3 */
  operationMatrix[13 + 4 * (1 + operationTableSize)] = 10; /* O2J5 @ M4 */
  operationMatrix[13 + 5 * (1 + operationTableSize)] = 6; /* O2J5 @ M5 */
  operationMatrix[13 + 6 * (1 + operationTableSize)] = 4; /* O2J5 @ M6 */
  operationMatrix[13 + 7 * (1 + operationTableSize)] = 9; /* O2J5 @ M7 */

  operationMatrix[14 + 1 * (1 + operationTableSize)] = 5; /* O3J5 @ M1 */
  operationMatrix[14 + 2 * (1 + operationTableSize)] = 1; /* O3J5 @ M2 */
  operationMatrix[14 + 3 * (1 + operationTableSize)] = 7; /* O3J5 @ M3 */
  operationMatrix[14 + 4 * (1 + operationTableSize)] = 1; /* O3J5 @ M4 */
  operationMatrix[14 + 5 * (1 + operationTableSize)] = 6; /* O3J5 @ M5 */
  operationMatrix[14 + 6 * (1 + operationTableSize)] = 6; /* O3J5 @ M6 */
  operationMatrix[14 + 7 * (1 + operationTableSize)] = 2; /* O3J5 @ M7 */

  operationMatrix[15 + 1 * (1 + operationTableSize)] = 8; /* O1J6 @ M1 */
  operationMatrix[15 + 2 * (1 + operationTableSize)] = 7; /* O1J6 @ M2 */
  operationMatrix[15 + 3 * (1 + operationTableSize)] = 4; /* O1J6 @ M3 */
  operationMatrix[15 + 4 * (1 + operationTableSize)] = 56; /* O1J6 @ M4 */
  operationMatrix[15 + 5 * (1 + operationTableSize)] = 9; /* O1J6 @ M5 */
  operationMatrix[15 + 6 * (1 + operationTableSize)] = 8; /* O1J6 @ M6 */
  operationMatrix[15 + 7 * (1 + operationTableSize)] = 4; /* O1J6 @ M7 */

  operationMatrix[16 + 1 * (1 + operationTableSize)] = 5; /* O2J6 @ M1 */
  operationMatrix[16 + 2 * (1 + operationTableSize)] = 14; /* O2J6 @ M2 */
  operationMatrix[16 + 3 * (1 + operationTableSize)] = 1; /* O2J6 @ M3 */
  operationMatrix[16 + 4 * (1 + operationTableSize)] = 9; /* O2J6 @ M4 */
  operationMatrix[16 + 5 * (1 + operationTableSize)] = 6; /* O2J6 @ M5 */
  operationMatrix[16 + 6 * (1 + operationTableSize)] = 5; /* O2J6 @ M6 */
  operationMatrix[16 + 7 * (1 + operationTableSize)] = 8; /* O2J6 @ M7 */

  operationMatrix[17 + 1 * (1 + operationTableSize)] = 3; /* O3J6 @ M1 */
  operationMatrix[17 + 2 * (1 + operationTableSize)] = 5; /* O3J6 @ M2 */
  operationMatrix[17 + 3 * (1 + operationTableSize)] = 2; /* O3J6 @ M3 */
  operationMatrix[17 + 4 * (1 + operationTableSize)] = 5; /* O3J6 @ M4 */
  operationMatrix[17 + 5 * (1 + operationTableSize)] = 4; /* O3J6 @ M5 */
  operationMatrix[17 + 6 * (1 + operationTableSize)] = 5; /* O3J6 @ M6 */
  operationMatrix[17 + 7 * (1 + operationTableSize)] = 7; /* O3J6 @ M7 */

  operationMatrix[18 + 1 * (1 + operationTableSize)] = 5; /* O1J7 @ M1 */
  operationMatrix[18 + 2 * (1 + operationTableSize)] = 6; /* O1J7 @ M2 */
  operationMatrix[18 + 3 * (1 + operationTableSize)] = 3; /* O1J7 @ M3 */
  operationMatrix[18 + 4 * (1 + operationTableSize)] = 6; /* O1J7 @ M4 */
  operationMatrix[18 + 5 * (1 + operationTableSize)] = 5; /* O1J7 @ M5 */
  operationMatrix[18 + 6 * (1 + operationTableSize)] = 15; /* O1J7 @ M6 */
  operationMatrix[18 + 7 * (1 + operationTableSize)] = 2; /* O1J7 @ M7 */

  operationMatrix[19 + 1 * (1 + operationTableSize)] = 6; /* O2J7 @ M1 */
  operationMatrix[19 + 2 * (1 + operationTableSize)] = 5; /* O2J7 @ M2 */
  operationMatrix[19 + 3 * (1 + operationTableSize)] = 4; /* O2J7 @ M3 */
  operationMatrix[19 + 4 * (1 + operationTableSize)] = 9; /* O2J7 @ M4 */
  operationMatrix[19 + 5 * (1 + operationTableSize)] = 5; /* O2J7 @ M5 */
  operationMatrix[19 + 6 * (1 + operationTableSize)] = 4; /* O2J7 @ M6 */
  operationMatrix[19 + 7 * (1 + operationTableSize)] = 3; /* O2J7 @ M7 */

  operationMatrix[20 + 1 * (1 + operationTableSize)] = 9; /* O3J7 @ M1 */
  operationMatrix[20 + 2 * (1 + operationTableSize)] = 8; /* O3J7 @ M2 */
  operationMatrix[20 + 3 * (1 + operationTableSize)] = 2; /* O3J7 @ M3 */
  operationMatrix[20 + 4 * (1 + operationTableSize)] = 8; /* O3J7 @ M4 */
  operationMatrix[20 + 5 * (1 + operationTableSize)] = 6; /* O3J7 @ M5 */
  operationMatrix[20 + 6 * (1 + operationTableSize)] = 1; /* O3J7 @ M6 */
  operationMatrix[20 + 7 * (1 + operationTableSize)] = 7; /* O3J7 @ M7 */

  operationMatrix[21 + 1 * (1 + operationTableSize)] = 6; /* O1J8 @ M1 */
  operationMatrix[21 + 2 * (1 + operationTableSize)] = 1; /* O1J8 @ M2 */
  operationMatrix[21 + 3 * (1 + operationTableSize)] = 4; /* O1J8 @ M3 */
  operationMatrix[21 + 4 * (1 + operationTableSize)] = 1; /* O1J8 @ M4 */
  operationMatrix[21 + 5 * (1 + operationTableSize)] = 10; /* O1J8 @ M5 */
  operationMatrix[21 + 6 * (1 + operationTableSize)] = 4; /* O1J8 @ M6 */
  operationMatrix[21 + 7 * (1 + operationTableSize)] = 3; /* O1J8 @ M7 */

  operationMatrix[22 + 1 * (1 + operationTableSize)] = 11; /* O2J8 @ M1 */
  operationMatrix[22 + 2 * (1 + operationTableSize)] = 13; /* O2J8 @ M2 */
  operationMatrix[22 + 3 * (1 + operationTableSize)] = 9; /* O2J8 @ M3 */
  operationMatrix[22 + 4 * (1 + operationTableSize)] = 8; /* O2J8 @ M4 */
  operationMatrix[22 + 5 * (1 + operationTableSize)] = 9; /* O2J8 @ M5 */
  operationMatrix[22 + 6 * (1 + operationTableSize)] = 10; /* O2J8 @ M6 */
  operationMatrix[22 + 7 * (1 + operationTableSize)] = 8; /* O2J8 @ M7 */

  operationMatrix[23 + 1 * (1 + operationTableSize)] = 4; /* O3J8 @ M1 */
  operationMatrix[23 + 2 * (1 + operationTableSize)] = 2; /* O3J8 @ M2 */
  operationMatrix[23 + 3 * (1 + operationTableSize)] = 7; /* O3J8 @ M3 */
  operationMatrix[23 + 4 * (1 + operationTableSize)] = 8; /* O3J8 @ M4 */
  operationMatrix[23 + 5 * (1 + operationTableSize)] = 3; /* O3J8 @ M5 */
  operationMatrix[23 + 6 * (1 + operationTableSize)] = 10; /* O3J8 @ M6 */
  operationMatrix[23 + 7 * (1 + operationTableSize)] = 7; /* O3J8 @ M7 */

  operationMatrix[24 + 1 * (1 + operationTableSize)] = 12; /* O1J9 @ M1 */
  operationMatrix[24 + 2 * (1 + operationTableSize)] = 5; /* O1J9 @ M2 */
  operationMatrix[24 + 3 * (1 + operationTableSize)] = 4; /* O1J9 @ M3 */
  operationMatrix[24 + 4 * (1 + operationTableSize)] = 5; /* O1J9 @ M4 */
  operationMatrix[24 + 5 * (1 + operationTableSize)] = 4; /* O1J9 @ M5 */
  operationMatrix[24 + 6 * (1 + operationTableSize)] = 5; /* O1J9 @ M6 */
  operationMatrix[24 + 7 * (1 + operationTableSize)] = 5; /* O1J9 @ M7 */

  operationMatrix[25 + 1 * (1 + operationTableSize)] = 4; /* O2J9 @ M1 */
  operationMatrix[25 + 2 * (1 + operationTableSize)] = 2; /* O2J9 @ M2 */
  operationMatrix[25 + 3 * (1 + operationTableSize)] = 15; /* O2J9 @ M3 */
  operationMatrix[25 + 4 * (1 + operationTableSize)] = 99; /* O2J9 @ M4 */
  operationMatrix[25 + 5 * (1 + operationTableSize)] = 4; /* O2J9 @ M5 */
  operationMatrix[25 + 6 * (1 + operationTableSize)] = 7; /* O2J9 @ M6 */
  operationMatrix[25 + 7 * (1 + operationTableSize)] = 3; /* O2J9 @ M7 */

  operationMatrix[26 + 1 * (1 + operationTableSize)] = 9; /* O3J9 @ M1 */
  operationMatrix[26 + 2 * (1 + operationTableSize)] = 5; /* O3J9 @ M2 */
  operationMatrix[26 + 3 * (1 + operationTableSize)] = 11; /* O3J9 @ M3 */
  operationMatrix[26 + 4 * (1 + operationTableSize)] = 2; /* O3J9 @ M4 */
  operationMatrix[26 + 5 * (1 + operationTableSize)] = 5; /* O3J9 @ M5 */
  operationMatrix[26 + 6 * (1 + operationTableSize)] = 4; /* O3J9 @ M6 */
  operationMatrix[26 + 7 * (1 + operationTableSize)] = 2; /* O3J9 @ M7 */

  operationMatrix[27 + 1 * (1 + operationTableSize)] = 9; /* O1J10 @ M1 */
  operationMatrix[27 + 2 * (1 + operationTableSize)] = 4; /* O1J10 @ M2 */
  operationMatrix[27 + 3 * (1 + operationTableSize)] = 13; /* O1J10 @ M3 */
  operationMatrix[27 + 4 * (1 + operationTableSize)] = 10; /* O1J10 @ M4 */
  operationMatrix[27 + 5 * (1 + operationTableSize)] = 7; /* O1J10 @ M5 */
  operationMatrix[27 + 6 * (1 + operationTableSize)] = 6; /* O1J10 @ M6 */
  operationMatrix[27 + 7 * (1 + operationTableSize)] = 8; /* O1J10 @ M7 */

  operationMatrix[28 + 1 * (1 + operationTableSize)] = 4; /* O2J10 @ M1 */
  operationMatrix[28 + 2 * (1 + operationTableSize)] = 3; /* O2J10 @ M2 */
  operationMatrix[28 + 3 * (1 + operationTableSize)] = 25; /* O2J10 @ M3 */
  operationMatrix[28 + 4 * (1 + operationTableSize)] = 3; /* O2J10 @ M4 */
  operationMatrix[28 + 5 * (1 + operationTableSize)] = 8; /* O2J10 @ M5 */
  operationMatrix[28 + 6 * (1 + operationTableSize)] = 1; /* O2J10 @ M6 */
  operationMatrix[28 + 7 * (1 + operationTableSize)] = 2; /* O2J10 @ M7 */

  operationMatrix[29 + 1 * (1 + operationTableSize)] = 1; /* O3J10 @ M1 */
  operationMatrix[29 + 2 * (1 + operationTableSize)] = 2; /* O3J10 @ M2 */
  operationMatrix[29 + 3 * (1 + operationTableSize)] = 6; /* O3J10 @ M3 */
  operationMatrix[29 + 4 * (1 + operationTableSize)] = 11; /* O3J10 @ M4 */
  operationMatrix[29 + 5 * (1 + operationTableSize)] = 13; /* O3J10 @ M5 */
  operationMatrix[29 + 6 * (1 + operationTableSize)] = 3; /* O3J10 @ M6 */
  operationMatrix[29 + 7 * (1 + operationTableSize)] = 5; /* O3J10 @ M7 */

  /* *************************************** */
  /* PRODUCTION TABLE */
  /* *************************************** */

  productionTableSize = 10;
  productionTable = (PPART *) malloc((1 + productionTableSize) * sizeof(PPART));
  productionTable[0] = NULL;

  if ((NUMPARTS == 0) || (NUMPARTS > productionTableSize)) {
    NUMPARTS = productionTableSize;
  }

  /* *************************************** */
  /* PART TABLE */
  /* *************************************** */

  pnum = 0;
  part = 0;

  partTableSize = 10; /* number of parts (including subparts) */
  partTable = (PART *) malloc((1 + partTableSize) * sizeof(PART));
  partTable[0].name = NULL; partTable[0].n = 0; partTable[0].X = NULL;
  for (i = 1; i <= partTableSize; i++)  {
    partTable[i].idx = i;
    partTable[i].color = 0;
  }

  /* *************************************** */
  /* JOB J1 */
  /* *************************************** */
  
  part++;

  pnum++; xnum = 0; onum = 0; /* next part, id = 1 */

  partTable[pnum].name = strdup("P1");
  partTable[pnum].part = part;
  partTable[pnum].n = 1; /* number of sequences */
  partTable[pnum].code = NULL; partTable[pnum].feasible = NULL;
  partTable[pnum].X = (SEQUENCE *) malloc((1 + partTable[pnum].n) * sizeof(SEQUENCE));
  partTable[pnum].X[0].n = 0; partTable[pnum].X[0].O = NULL;

  xnum++; onum = 0; /* next sequence */

  partTable[pnum].X[xnum].n = 3; /* number of items for the 1st sequence in J1 */
  partTable[pnum].X[xnum].O = (SEQUENCEITEM *) malloc((1 + partTable[pnum].X[xnum].n) * sizeof(SEQUENCEITEM));
  partTable[pnum].X[xnum].O[0].type = NONE; partTable[pnum].X[xnum].O[0].ITEM = NULL;
  partTable[pnum].X[xnum].O[++onum].type = DEFAULT; partTable[pnum].X[xnum].O[onum].ITEM = (void *) &(operationTable[1]); /* O1J1 */
  partTable[pnum].X[xnum].O[++onum].type = DEFAULT; partTable[pnum].X[xnum].O[onum].ITEM = (void *) &(operationTable[2]); /* O2J1 */
  partTable[pnum].X[xnum].O[++onum].type = DEFAULT; partTable[pnum].X[xnum].O[onum].ITEM = (void *) &(operationTable[3]); /* O3J1 */

  productionTable[part] = &(partTable[pnum]);
  partTable[pnum].color = 0;

  /* ADDVARIABLE(w,r) */
  ADDVARIABLE(1,1);
  ADDVARIABLE(2,2);
  ADDVARIABLE(3,3);

  /* *************************************** */
  /* JOB J2 */
  /* *************************************** */

  part++;

  pnum++; xnum = 0; onum = 0; /* next part, id = 2 */

  partTable[pnum].name = strdup("P2");
  partTable[pnum].part = part;
  partTable[pnum].n = 1; /* number of sequences */
  partTable[pnum].code = NULL; partTable[pnum].feasible = NULL;
  partTable[pnum].X = (SEQUENCE *) malloc((1 + partTable[pnum].n) * sizeof(SEQUENCE));
  partTable[pnum].X[0].n = 0; partTable[pnum].X[0].O = NULL;

  xnum++; onum = 0; /* next sequence */

  partTable[pnum].X[xnum].n = 2; /* number of items for the 1st sequence in J2 */
  partTable[pnum].X[xnum].O = (SEQUENCEITEM *) malloc((1 + partTable[pnum].X[xnum].n) * sizeof(SEQUENCEITEM));
  partTable[pnum].X[xnum].O[0].type = NONE; partTable[pnum].X[xnum].O[0].ITEM = NULL;
  partTable[pnum].X[xnum].O[++onum].type = DEFAULT; partTable[pnum].X[xnum].O[onum].ITEM = (void *) &(operationTable[4]); /* O1J2 */
  partTable[pnum].X[xnum].O[++onum].type = DEFAULT; partTable[pnum].X[xnum].O[onum].ITEM = (void *) &(operationTable[5]); /* O2J2 */

  productionTable[part] = &(partTable[pnum]);
  partTable[pnum].color = 60;

  /* ADDVARIABLE(w,r) */
  ADDVARIABLE(4,1);
  ADDVARIABLE(5,2);

  /* *************************************** */
  /* JOB J3 */
  /* *************************************** */

  part++;

  pnum++; xnum = 0; onum = 0; /* next part, id = 3 */

  partTable[pnum].name = strdup("P3");
  partTable[pnum].part = part;
  partTable[pnum].n = 1; /* number of sequences */
  partTable[pnum].code = NULL; partTable[pnum].feasible = NULL;
  partTable[pnum].X = (SEQUENCE *) malloc((1 + partTable[pnum].n) * sizeof(SEQUENCE));
  partTable[pnum].X[0].n = 0; partTable[pnum].X[0].O = NULL;

  xnum++; onum = 0; /* next sequence */

  partTable[pnum].X[xnum].n = 3; /* number of items for the 1st sequence in J3 */
  partTable[pnum].X[xnum].O = (SEQUENCEITEM *) malloc((1 + partTable[pnum].X[xnum].n) * sizeof(SEQUENCEITEM));
  partTable[pnum].X[xnum].O[0].type = NONE; partTable[pnum].X[xnum].O[0].ITEM = NULL;
  partTable[pnum].X[xnum].O[++onum].type = DEFAULT; partTable[pnum].X[xnum].O[onum].ITEM = (void *) &(operationTable[6]); /* O1J3 */
  partTable[pnum].X[xnum].O[++onum].type = DEFAULT; partTable[pnum].X[xnum].O[onum].ITEM = (void *) &(operationTable[7]); /* O2J3 */
  partTable[pnum].X[xnum].O[++onum].type = DEFAULT; partTable[pnum].X[xnum].O[onum].ITEM = (void *) &(operationTable[8]); /* O3J3 */

  productionTable[part] = &(partTable[pnum]);
  partTable[pnum].color = 140;

  /* ADDVARIABLE(w,r) */
  ADDVARIABLE(6,1);
  ADDVARIABLE(7,2);
  ADDVARIABLE(8,3);

  /* *************************************** */
  /* JOB J4 */
  /* *************************************** */

  part++;

  pnum++; xnum = 0; onum = 0; /* next part, id = 4 */

  partTable[pnum].name = strdup("P4");
  partTable[pnum].part = part;
  partTable[pnum].n = 1; /* number of sequences */
  partTable[pnum].code = NULL; partTable[pnum].feasible = NULL;
  partTable[pnum].X = (SEQUENCE *) malloc((1 + partTable[pnum].n) * sizeof(SEQUENCE));
  partTable[pnum].X[0].n = 0; partTable[pnum].X[0].O = NULL;

  xnum++; onum = 0; /* next sequence */

  partTable[pnum].X[xnum].n = 3; /* number of items for the 1st sequence in J4 */
  partTable[pnum].X[xnum].O = (SEQUENCEITEM *) malloc((1 + partTable[pnum].X[xnum].n) * sizeof(SEQUENCEITEM));
  partTable[pnum].X[xnum].O[0].type = NONE; partTable[pnum].X[xnum].O[0].ITEM = NULL;
  partTable[pnum].X[xnum].O[++onum].type = DEFAULT; partTable[pnum].X[xnum].O[onum].ITEM = (void *) &(operationTable[9]); /* O1J4 */
  partTable[pnum].X[xnum].O[++onum].type = DEFAULT; partTable[pnum].X[xnum].O[onum].ITEM = (void *) &(operationTable[10]); /* O2J4 */
  partTable[pnum].X[xnum].O[++onum].type = DEFAULT; partTable[pnum].X[xnum].O[onum].ITEM = (void *) &(operationTable[11]); /* O3J4 */

  productionTable[part] = &(partTable[pnum]);
  partTable[pnum].color = 180;

  /* ADDVARIABLE(w,r) */
  ADDVARIABLE(9,1);
  ADDVARIABLE(10,2);
  ADDVARIABLE(11,3);

  /* *************************************** */
  /* JOB J5 */
  /* *************************************** */

  part++;

  pnum++; xnum = 0; onum = 0; /* next part, id = 5 */

  partTable[pnum].name = strdup("P5");
  partTable[pnum].part = part;
  partTable[pnum].n = 1; /* number of sequences */
  partTable[pnum].code = NULL; partTable[pnum].feasible = NULL;
  partTable[pnum].X = (SEQUENCE *) malloc((1 + partTable[pnum].n) * sizeof(SEQUENCE));
  partTable[pnum].X[0].n = 0; partTable[pnum].X[0].O = NULL;

  xnum++; onum = 0; /* next sequence */

  partTable[pnum].X[xnum].n = 3; /* number of items for the 1st sequence in J5 */
  partTable[pnum].X[xnum].O = (SEQUENCEITEM *) malloc((1 + partTable[pnum].X[xnum].n) * sizeof(SEQUENCEITEM));
  partTable[pnum].X[xnum].O[0].type = NONE; partTable[pnum].X[xnum].O[0].ITEM = NULL;
  partTable[pnum].X[xnum].O[++onum].type = DEFAULT; partTable[pnum].X[xnum].O[onum].ITEM = (void *) &(operationTable[12]); /* O1J5 */
  partTable[pnum].X[xnum].O[++onum].type = DEFAULT; partTable[pnum].X[xnum].O[onum].ITEM = (void *) &(operationTable[13]); /* O2J5 */
  partTable[pnum].X[xnum].O[++onum].type = DEFAULT; partTable[pnum].X[xnum].O[onum].ITEM = (void *) &(operationTable[14]); /* O3J5 */

  productionTable[part] = &(partTable[pnum]);
  partTable[pnum].color = 230;

  /* ADDVARIABLE(w,r) */
  ADDVARIABLE(12,1);
  ADDVARIABLE(13,2);
  ADDVARIABLE(14,3);

  /* *************************************** */
  /* JOB J6 */
  /* *************************************** */

  part++;

  pnum++; xnum = 0; onum = 0; /* next part, id = 6 */

  partTable[pnum].name = strdup("P6");
  partTable[pnum].part = part;
  partTable[pnum].n = 1; /* number of sequences */
  partTable[pnum].code = NULL; partTable[pnum].feasible = NULL;
  partTable[pnum].X = (SEQUENCE *) malloc((1 + partTable[pnum].n) * sizeof(SEQUENCE));
  partTable[pnum].X[0].n = 0; partTable[pnum].X[0].O = NULL;

  xnum++; onum = 0; /* next sequence */

  partTable[pnum].X[xnum].n = 3; /* number of items for the 1st sequence in J6 */
  partTable[pnum].X[xnum].O = (SEQUENCEITEM *) malloc((1 + partTable[pnum].X[xnum].n) * sizeof(SEQUENCEITEM));
  partTable[pnum].X[xnum].O[0].type = NONE; partTable[pnum].X[xnum].O[0].ITEM = NULL;
  partTable[pnum].X[xnum].O[++onum].type = DEFAULT; partTable[pnum].X[xnum].O[onum].ITEM = (void *) &(operationTable[15]); /* O1J6 */
  partTable[pnum].X[xnum].O[++onum].type = DEFAULT; partTable[pnum].X[xnum].O[onum].ITEM = (void *) &(operationTable[16]); /* O2J6 */
  partTable[pnum].X[xnum].O[++onum].type = DEFAULT; partTable[pnum].X[xnum].O[onum].ITEM = (void *) &(operationTable[17]); /* O3J6 */

  productionTable[part] = &(partTable[pnum]);
  partTable[pnum].color = 300;

  /* ADDVARIABLE(w,r) */
  ADDVARIABLE(15,1);
  ADDVARIABLE(16,2);
  ADDVARIABLE(17,3);

  /* *************************************** */
  /* JOB J7 */
  /* *************************************** */

  part++;

  pnum++; xnum = 0; onum = 0; /* next part, id = 7 */

  partTable[pnum].name = strdup("P7");
  partTable[pnum].part = part;
  partTable[pnum].n = 1; /* number of sequences */
  partTable[pnum].code = NULL; partTable[pnum].feasible = NULL;
  partTable[pnum].X = (SEQUENCE *) malloc((1 + partTable[pnum].n) * sizeof(SEQUENCE));
  partTable[pnum].X[0].n = 0; partTable[pnum].X[0].O = NULL;

  xnum++; onum = 0; /* next sequence */

  partTable[pnum].X[xnum].n = 3; /* number of items for the 1st sequence in J7 */
  partTable[pnum].X[xnum].O = (SEQUENCEITEM *) malloc((1 + partTable[pnum].X[xnum].n) * sizeof(SEQUENCEITEM));
  partTable[pnum].X[xnum].O[0].type = NONE; partTable[pnum].X[xnum].O[0].ITEM = NULL;
  partTable[pnum].X[xnum].O[++onum].type = DEFAULT; partTable[pnum].X[xnum].O[onum].ITEM = (void *) &(operationTable[18]); /* O1J7 */
  partTable[pnum].X[xnum].O[++onum].type = DEFAULT; partTable[pnum].X[xnum].O[onum].ITEM = (void *) &(operationTable[19]); /* O2J7 */
  partTable[pnum].X[xnum].O[++onum].type = DEFAULT; partTable[pnum].X[xnum].O[onum].ITEM = (void *) &(operationTable[20]); /* O3J7 */

  productionTable[part] = &(partTable[pnum]);
  partTable[pnum].color = 30;

  /* ADDVARIABLE(w,r) */
  ADDVARIABLE(18,1);
  ADDVARIABLE(19,2);
  ADDVARIABLE(20,3);

  /* *************************************** */
  /* JOB J8 */
  /* *************************************** */

  part++;

  pnum++; xnum = 0; onum = 0; /* next part, id = 8 */

  partTable[pnum].name = strdup("P8");
  partTable[pnum].part = part;
  partTable[pnum].n = 1; /* number of sequences */
  partTable[pnum].code = NULL; partTable[pnum].feasible = NULL;
  partTable[pnum].X = (SEQUENCE *) malloc((1 + partTable[pnum].n) * sizeof(SEQUENCE));
  partTable[pnum].X[0].n = 0; partTable[pnum].X[0].O = NULL;

  xnum++; onum = 0; /* next sequence */

  partTable[pnum].X[xnum].n = 3; /* number of items for the 1st sequence in J8 */
  partTable[pnum].X[xnum].O = (SEQUENCEITEM *) malloc((1 + partTable[pnum].X[xnum].n) * sizeof(SEQUENCEITEM));
  partTable[pnum].X[xnum].O[0].type = NONE; partTable[pnum].X[xnum].O[0].ITEM = NULL;
  partTable[pnum].X[xnum].O[++onum].type = DEFAULT; partTable[pnum].X[xnum].O[onum].ITEM = (void *) &(operationTable[21]); /* O1J8 */
  partTable[pnum].X[xnum].O[++onum].type = DEFAULT; partTable[pnum].X[xnum].O[onum].ITEM = (void *) &(operationTable[22]); /* O2J8 */
  partTable[pnum].X[xnum].O[++onum].type = DEFAULT; partTable[pnum].X[xnum].O[onum].ITEM = (void *) &(operationTable[23]); /* O3J8 */

  productionTable[part] = &(partTable[pnum]);
  partTable[pnum].color = 270;

  /* ADDVARIABLE(w,r) */
  ADDVARIABLE(21,1);
  ADDVARIABLE(22,2);
  ADDVARIABLE(23,3);

  /* *************************************** */
  /* JOB J9 */
  /* *************************************** */

  part++;

  pnum++; xnum = 0; onum = 0; /* next part, id = 9 */

  partTable[pnum].name = strdup("P9");
  partTable[pnum].part = part;
  partTable[pnum].n = 1; /* number of sequences */
  partTable[pnum].code = NULL; partTable[pnum].feasible = NULL;
  partTable[pnum].X = (SEQUENCE *) malloc((1 + partTable[pnum].n) * sizeof(SEQUENCE));
  partTable[pnum].X[0].n = 0; partTable[pnum].X[0].O = NULL;

  xnum++; onum = 0; /* next sequence */

  partTable[pnum].X[xnum].n = 3; /* number of items for the 1st sequence in J9 */
  partTable[pnum].X[xnum].O = (SEQUENCEITEM *) malloc((1 + partTable[pnum].X[xnum].n) * sizeof(SEQUENCEITEM));
  partTable[pnum].X[xnum].O[0].type = NONE; partTable[pnum].X[xnum].O[0].ITEM = NULL;
  partTable[pnum].X[xnum].O[++onum].type = DEFAULT; partTable[pnum].X[xnum].O[onum].ITEM = (void *) &(operationTable[24]); /* O1J9 */
  partTable[pnum].X[xnum].O[++onum].type = DEFAULT; partTable[pnum].X[xnum].O[onum].ITEM = (void *) &(operationTable[25]); /* O2J9 */
  partTable[pnum].X[xnum].O[++onum].type = DEFAULT; partTable[pnum].X[xnum].O[onum].ITEM = (void *) &(operationTable[26]); /* O3J9 */

  productionTable[part] = &(partTable[pnum]);
  partTable[pnum].color = 90;

  /* ADDVARIABLE(w,r) */
  ADDVARIABLE(24,1);
  ADDVARIABLE(25,2);
  ADDVARIABLE(26,3);

  /* *************************************** */
  /* JOB J10 */
  /* *************************************** */

  part++;

  pnum++; xnum = 0; onum = 0; /* next part, id = 10 */

  partTable[pnum].name = strdup("P10");
  partTable[pnum].part = part;
  partTable[pnum].n = 1; /* number of sequences */
  partTable[pnum].code = NULL; partTable[pnum].feasible = NULL;
  partTable[pnum].X = (SEQUENCE *) malloc((1 + partTable[pnum].n) * sizeof(SEQUENCE));
  partTable[pnum].X[0].n = 0; partTable[pnum].X[0].O = NULL;

  xnum++; onum = 0; /* next sequence */

  partTable[pnum].X[xnum].n = 3; /* number of items for the 1st sequence in J10 */
  partTable[pnum].X[xnum].O = (SEQUENCEITEM *) malloc((1 + partTable[pnum].X[xnum].n) * sizeof(SEQUENCEITEM));
  partTable[pnum].X[xnum].O[0].type = NONE; partTable[pnum].X[xnum].O[0].ITEM = NULL;
  partTable[pnum].X[xnum].O[++onum].type = DEFAULT; partTable[pnum].X[xnum].O[onum].ITEM = (void *) &(operationTable[27]); /* O1J10 */
  partTable[pnum].X[xnum].O[++onum].type = DEFAULT; partTable[pnum].X[xnum].O[onum].ITEM = (void *) &(operationTable[28]); /* O2J10 */
  partTable[pnum].X[xnum].O[++onum].type = DEFAULT; partTable[pnum].X[xnum].O[onum].ITEM = (void *) &(operationTable[29]); /* O3J10 */

  productionTable[part] = &(partTable[pnum]);
  partTable[pnum].color = 200;

  /* ADDVARIABLE(w,r) */
  ADDVARIABLE(27,1);
  ADDVARIABLE(28,2);
  ADDVARIABLE(29,3);
}

/* ************************************************************************************ */
/* ************************************************************************************ */
/* ************************************************************************************ */

void
generateSystemKacem10x10()
{
  unsigned int part,pnum,xnum,onum;
  unsigned int i,j;

  if (NUMPARTS == 0) NUMPARTS = 10;
  if (NUMMACHINES == 0) NUMMACHINES = 10;

  MAXSEQUENCELENGTH = 3;

  /* user defined parts, sequences, and operations start at index 1 */

  /* *************************************** */
  /* OPERATION TABLE */
  /* *************************************** */
  
  operationTableSize = 30; /* number of operations */
  operationTable = (OPERATION *) malloc((1 + operationTableSize) * sizeof(OPERATION));
  (operationTable)[0].name = NULL;
  for (i = 1; i <= operationTableSize; i++)  (operationTable)[i].idx = i;

  (operationTable)[1].name = strdup("O1J1");
  (operationTable)[2].name = strdup("O2J1");
  (operationTable)[3].name = strdup("O3J1");
  (operationTable)[4].name = strdup("O1J2");
  (operationTable)[5].name = strdup("O2J2");
  (operationTable)[6].name = strdup("O3J2");
  (operationTable)[7].name = strdup("O1J3");
  (operationTable)[8].name = strdup("O2J3");
  (operationTable)[9].name = strdup("O3J3");
  (operationTable)[10].name = strdup("O1J4");
  (operationTable)[11].name = strdup("O2J4");
  (operationTable)[12].name = strdup("O3J4");
  (operationTable)[13].name = strdup("O1J5");
  (operationTable)[14].name = strdup("O2J5");
  (operationTable)[15].name = strdup("O3J5");
  (operationTable)[16].name = strdup("O1J6");
  (operationTable)[17].name = strdup("O2J6");
  (operationTable)[18].name = strdup("O3J6");
  (operationTable)[19].name = strdup("O1J7");
  (operationTable)[20].name = strdup("O2J7");
  (operationTable)[21].name = strdup("O3J7");
  (operationTable)[22].name = strdup("O1J8");
  (operationTable)[23].name = strdup("O2J8");
  (operationTable)[24].name = strdup("O3J8");
  (operationTable)[25].name = strdup("O1J9");
  (operationTable)[26].name = strdup("O2J9");
  (operationTable)[27].name = strdup("O3J9");
  (operationTable)[28].name = strdup("O1J10");
  (operationTable)[29].name = strdup("O2J10");
  (operationTable)[30].name = strdup("O3J10");

  /* *************************************** */
  /* MACHINE TABLE */
  /* *************************************** */
  
  machineTableSize = 10; /* number of machines */
  machineTable = (MACHINE *) malloc((1 + machineTableSize) * sizeof(MACHINE));
  machineTable[0].name = NULL;
  for (i = 1; i <= machineTableSize; i++) machineTable[i].idx = i;

  machineTable[1].name = strdup("M1"); machineTable[1].n = 1;
  machineTable[2].name = strdup("M2"); machineTable[2].n = 1;
  machineTable[3].name = strdup("M3"); machineTable[3].n = 1;
  machineTable[4].name = strdup("M4"); machineTable[4].n = 1;
  machineTable[5].name = strdup("M5"); machineTable[5].n = 1;
  machineTable[6].name = strdup("M6"); machineTable[6].n = 1;
  machineTable[7].name = strdup("M7"); machineTable[7].n = 1;
  machineTable[8].name = strdup("M8"); machineTable[8].n = 1;
  machineTable[9].name = strdup("M9"); machineTable[9].n = 1;
  machineTable[10].name = strdup("M10"); machineTable[10].n = 1;

  if ((NUMMACHINES == 0) || (NUMMACHINES > machineTableSize)) {
    NUMMACHINES = machineTableSize;
  }

  /* *************************************** */
  /* OPERATION MATRIX */
  /* *************************************** */

  operationMatrix = (unsigned int *)
    malloc((1 + machineTableSize) * (1 + operationTableSize) * sizeof(unsigned int));

  for (i = 0; i <= operationTableSize; i++) {
    for (j = 0; j <= machineTableSize; j++) {
      (operationMatrix)[i + j * (1 + operationTableSize)] = 0; /* Oi @ mj */
    }
  }

  operationMatrix[1 + 1 * (1 + operationTableSize)] = 1; /* O1J1 @ M1 */
  operationMatrix[1 + 2 * (1 + operationTableSize)] = 4; /* O1J1 @ M2 */
  operationMatrix[1 + 3 * (1 + operationTableSize)] = 6; /* O1J1 @ M3 */
  operationMatrix[1 + 4 * (1 + operationTableSize)] = 9; /* O1J1 @ M4 */
  operationMatrix[1 + 5 * (1 + operationTableSize)] = 3; /* O1J1 @ M5 */
  operationMatrix[1 + 6 * (1 + operationTableSize)] = 5; /* O1J1 @ M6 */
  operationMatrix[1 + 7 * (1 + operationTableSize)] = 2; /* O1J1 @ M7 */
  operationMatrix[1 + 8 * (1 + operationTableSize)] = 8; /* O1J1 @ M8 */
  operationMatrix[1 + 9 * (1 + operationTableSize)] = 9; /* O1J1 @ M9 */
  operationMatrix[1 + 10 * (1 + operationTableSize)] = 5; /* O1J1 @ M10 */

  operationMatrix[2 + 1 * (1 + operationTableSize)] = 4; /* O2J1 @ M1 */
  operationMatrix[2 + 2 * (1 + operationTableSize)] = 1; /* O2J1 @ M2 */
  operationMatrix[2 + 3 * (1 + operationTableSize)] = 1; /* O2J1 @ M3 */
  operationMatrix[2 + 4 * (1 + operationTableSize)] = 3; /* O2J1 @ M4 */
  operationMatrix[2 + 5 * (1 + operationTableSize)] = 4; /* O2J1 @ M5 */
  operationMatrix[2 + 6 * (1 + operationTableSize)] = 8; /* O2J1 @ M6 */
  operationMatrix[2 + 7 * (1 + operationTableSize)] = 10; /* O2J1 @ M7 */
  operationMatrix[2 + 8 * (1 + operationTableSize)] = 4; /* O2J1 @ M8 */
  operationMatrix[2 + 9 * (1 + operationTableSize)] = 11; /* O2J1 @ M9 */
  operationMatrix[2 + 10 * (1 + operationTableSize)] = 4; /* O2J1 @ M10 */

  operationMatrix[3 + 1 * (1 + operationTableSize)] = 3; /* O3J1 @ M1 */
  operationMatrix[3 + 2 * (1 + operationTableSize)] = 2; /* O3J1 @ M2 */
  operationMatrix[3 + 3 * (1 + operationTableSize)] = 5; /* O3J1 @ M3 */
  operationMatrix[3 + 4 * (1 + operationTableSize)] = 1; /* O3J1 @ M4 */
  operationMatrix[3 + 5 * (1 + operationTableSize)] = 5; /* O3J1 @ M5 */
  operationMatrix[3 + 6 * (1 + operationTableSize)] = 6; /* O3J1 @ M6 */
  operationMatrix[3 + 7 * (1 + operationTableSize)] = 9; /* O3J1 @ M7 */
  operationMatrix[3 + 8 * (1 + operationTableSize)] = 5; /* O3J1 @ M8 */
  operationMatrix[3 + 9 * (1 + operationTableSize)] = 10; /* O3J1 @ M9 */
  operationMatrix[3 + 10 * (1 + operationTableSize)] = 3; /* O3J1 @ M10 */

  operationMatrix[4 + 1 * (1 + operationTableSize)] = 2; /* O1J2 @ M1 */
  operationMatrix[4 + 2 * (1 + operationTableSize)] = 10; /* O1J2 @ M2 */
  operationMatrix[4 + 3 * (1 + operationTableSize)] = 4; /* O1J2 @ M3 */
  operationMatrix[4 + 4 * (1 + operationTableSize)] = 5; /* O1J2 @ M4 */
  operationMatrix[4 + 5 * (1 + operationTableSize)] = 9; /* O1J2 @ M5 */
  operationMatrix[4 + 6 * (1 + operationTableSize)] = 8; /* O1J2 @ M6 */
  operationMatrix[4 + 7 * (1 + operationTableSize)] = 4; /* O1J2 @ M7 */
  operationMatrix[4 + 8 * (1 + operationTableSize)] = 15; /* O1J2 @ M8 */
  operationMatrix[4 + 9 * (1 + operationTableSize)] = 8; /* O1J2 @ M9 */
  operationMatrix[4 + 10 * (1 + operationTableSize)] = 4; /* O1J2 @ M10 */

  operationMatrix[5 + 1 * (1 + operationTableSize)] = 4; /* O2J2 @ M1 */
  operationMatrix[5 + 2 * (1 + operationTableSize)] = 8; /* O2J2 @ M2 */
  operationMatrix[5 + 3 * (1 + operationTableSize)] = 7; /* O2J2 @ M3 */
  operationMatrix[5 + 4 * (1 + operationTableSize)] = 1; /* O2J2 @ M4 */
  operationMatrix[5 + 5 * (1 + operationTableSize)] = 9; /* O2J2 @ M5 */
  operationMatrix[5 + 6 * (1 + operationTableSize)] = 6; /* O2J2 @ M6 */
  operationMatrix[5 + 7 * (1 + operationTableSize)] = 1; /* O2J2 @ M7 */
  operationMatrix[5 + 8 * (1 + operationTableSize)] = 10; /* O2J2 @ M8 */
  operationMatrix[5 + 9 * (1 + operationTableSize)] = 7; /* O2J2 @ M9 */
  operationMatrix[5 + 10 * (1 + operationTableSize)] = 1; /* O2J2 @ M10 */

  operationMatrix[6 + 1 * (1 + operationTableSize)] = 6; /* O3J2 @ M1 */
  operationMatrix[6 + 2 * (1 + operationTableSize)] = 11; /* O3J2 @ M2 */
  operationMatrix[6 + 3 * (1 + operationTableSize)] = 2; /* O3J2 @ M3 */
  operationMatrix[6 + 4 * (1 + operationTableSize)] = 7; /* O3J2 @ M4 */
  operationMatrix[6 + 5 * (1 + operationTableSize)] = 5; /* O3J2 @ M5 */
  operationMatrix[6 + 6 * (1 + operationTableSize)] = 3; /* O3J2 @ M6 */
  operationMatrix[6 + 7 * (1 + operationTableSize)] = 5; /* O3J2 @ M7 */
  operationMatrix[6 + 8 * (1 + operationTableSize)] = 14; /* O3J2 @ M8 */
  operationMatrix[6 + 9 * (1 + operationTableSize)] = 9; /* O3J2 @ M9 */
  operationMatrix[6 + 10 * (1 + operationTableSize)] = 2; /* O3J2 @ M10 */

  operationMatrix[7 + 1 * (1 + operationTableSize)] = 8; /* O1J3 @ M1 */
  operationMatrix[7 + 2 * (1 + operationTableSize)] = 5; /* O1J3 @ M2 */
  operationMatrix[7 + 3 * (1 + operationTableSize)] = 8; /* O1J3 @ M3 */
  operationMatrix[7 + 4 * (1 + operationTableSize)] = 9; /* O1J3 @ M4 */
  operationMatrix[7 + 5 * (1 + operationTableSize)] = 4; /* O1J3 @ M5 */
  operationMatrix[7 + 6 * (1 + operationTableSize)] = 3; /* O1J3 @ M6 */
  operationMatrix[7 + 7 * (1 + operationTableSize)] = 5; /* O1J3 @ M7 */
  operationMatrix[7 + 8 * (1 + operationTableSize)] = 3; /* O1J3 @ M8 */
  operationMatrix[7 + 9 * (1 + operationTableSize)] = 8; /* O1J3 @ M9 */
  operationMatrix[7 + 10 * (1 + operationTableSize)] = 1; /* O1J3 @ M10 */

  operationMatrix[8 + 1 * (1 + operationTableSize)] = 9; /* O2J3 @ M1 */
  operationMatrix[8 + 2 * (1 + operationTableSize)] = 3; /* O2J3 @ M2 */
  operationMatrix[8 + 3 * (1 + operationTableSize)] = 6; /* O2J3 @ M3 */
  operationMatrix[8 + 4 * (1 + operationTableSize)] = 1; /* O2J3 @ M4 */
  operationMatrix[8 + 5 * (1 + operationTableSize)] = 2; /* O2J3 @ M5 */
  operationMatrix[8 + 6 * (1 + operationTableSize)] = 6; /* O2J3 @ M6 */
  operationMatrix[8 + 7 * (1 + operationTableSize)] = 4; /* O2J3 @ M7 */
  operationMatrix[8 + 8 * (1 + operationTableSize)] = 1; /* O2J3 @ M8 */
  operationMatrix[8 + 9 * (1 + operationTableSize)] = 7; /* O2J3 @ M9 */
  operationMatrix[8 + 10 * (1 + operationTableSize)] = 2; /* O2J3 @ M10 */

  operationMatrix[9 + 1 * (1 + operationTableSize)] = 7; /* O3J3 @ M1 */
  operationMatrix[9 + 2 * (1 + operationTableSize)] = 1; /* O3J3 @ M2 */
  operationMatrix[9 + 3 * (1 + operationTableSize)] = 8; /* O3J3 @ M3 */
  operationMatrix[9 + 4 * (1 + operationTableSize)] = 5; /* O3J3 @ M4 */
  operationMatrix[9 + 5 * (1 + operationTableSize)] = 4; /* O3J3 @ M5 */
  operationMatrix[9 + 6 * (1 + operationTableSize)] = 9; /* O3J3 @ M6 */
  operationMatrix[9 + 7 * (1 + operationTableSize)] = 1; /* O3J3 @ M7 */
  operationMatrix[9 + 8 * (1 + operationTableSize)] = 2; /* O3J3 @ M8 */
  operationMatrix[9 + 9 * (1 + operationTableSize)] = 3; /* O3J3 @ M9 */
  operationMatrix[9 + 10 * (1 + operationTableSize)] = 4; /* O3J3 @ M10 */

  operationMatrix[10 + 1 * (1 + operationTableSize)] = 5; /* O1J4 @ M1 */
  operationMatrix[10 + 2 * (1 + operationTableSize)] = 10; /* O1J4 @ M2 */
  operationMatrix[10 + 3 * (1 + operationTableSize)] = 6; /* O1J4 @ M3 */
  operationMatrix[10 + 4 * (1 + operationTableSize)] = 4; /* O1J4 @ M4 */
  operationMatrix[10 + 5 * (1 + operationTableSize)] = 9; /* O1J4 @ M5 */
  operationMatrix[10 + 6 * (1 + operationTableSize)] = 5; /* O1J4 @ M6 */
  operationMatrix[10 + 7 * (1 + operationTableSize)] = 1; /* O1J4 @ M7 */
  operationMatrix[10 + 8 * (1 + operationTableSize)] = 7; /* O1J4 @ M8 */
  operationMatrix[10 + 9 * (1 + operationTableSize)] = 1; /* O1J4 @ M9 */
  operationMatrix[10 + 10 * (1 + operationTableSize)] = 6; /* O1J4 @ M10 */

  operationMatrix[11 + 1 * (1 + operationTableSize)] = 4; /* O2J4 @ M1 */
  operationMatrix[11 + 2 * (1 + operationTableSize)] = 2; /* O2J4 @ M2 */
  operationMatrix[11 + 3 * (1 + operationTableSize)] = 3; /* O2J4 @ M3 */
  operationMatrix[11 + 4 * (1 + operationTableSize)] = 8; /* O2J4 @ M4 */
  operationMatrix[11 + 5 * (1 + operationTableSize)] = 7; /* O2J4 @ M5 */
  operationMatrix[11 + 6 * (1 + operationTableSize)] = 4; /* O2J4 @ M6 */
  operationMatrix[11 + 7 * (1 + operationTableSize)] = 6; /* O2J4 @ M7 */
  operationMatrix[11 + 8 * (1 + operationTableSize)] = 9; /* O2J4 @ M8 */
  operationMatrix[11 + 9 * (1 + operationTableSize)] = 8; /* O2J4 @ M9 */
  operationMatrix[11 + 10 * (1 + operationTableSize)] = 4; /* O2J4 @ M10 */

  operationMatrix[12 + 1 * (1 + operationTableSize)] = 7; /* O3J4 @ M1 */
  operationMatrix[12 + 2 * (1 + operationTableSize)] = 3; /* O3J4 @ M2 */
  operationMatrix[12 + 3 * (1 + operationTableSize)] = 12; /* O3J4 @ M3 */
  operationMatrix[12 + 4 * (1 + operationTableSize)] = 1; /* O3J4 @ M4 */
  operationMatrix[12 + 5 * (1 + operationTableSize)] = 6; /* O3J4 @ M5 */
  operationMatrix[12 + 6 * (1 + operationTableSize)] = 5; /* O3J4 @ M6 */
  operationMatrix[12 + 7 * (1 + operationTableSize)] = 8; /* O3J4 @ M7 */
  operationMatrix[12 + 8 * (1 + operationTableSize)] = 3; /* O3J4 @ M8 */
  operationMatrix[12 + 9 * (1 + operationTableSize)] = 5; /* O3J4 @ M9 */
  operationMatrix[12 + 10 * (1 + operationTableSize)] = 2; /* O3J4 @ M10 */

  operationMatrix[13 + 1 * (1 + operationTableSize)] = 7; /* O1J5 @ M1 */
  operationMatrix[13 + 2 * (1 + operationTableSize)] = 10; /* O1J5 @ M2 */
  operationMatrix[13 + 3 * (1 + operationTableSize)] = 4; /* O1J5 @ M3 */
  operationMatrix[13 + 4 * (1 + operationTableSize)] = 5; /* O1J5 @ M4 */
  operationMatrix[13 + 5 * (1 + operationTableSize)] = 6; /* O1J5 @ M5 */
  operationMatrix[13 + 6 * (1 + operationTableSize)] = 3; /* O1J5 @ M6 */
  operationMatrix[13 + 7 * (1 + operationTableSize)] = 5; /* O1J5 @ M7 */
  operationMatrix[13 + 8 * (1 + operationTableSize)] = 15; /* O1J5 @ M8 */
  operationMatrix[13 + 9 * (1 + operationTableSize)] = 2; /* O1J5 @ M9 */
  operationMatrix[13 + 10 * (1 + operationTableSize)] = 6; /* O1J5 @ M10 */

  operationMatrix[14 + 1 * (1 + operationTableSize)] = 5; /* O2J5 @ M1 */
  operationMatrix[14 + 2 * (1 + operationTableSize)] = 6; /* O2J5 @ M2 */
  operationMatrix[14 + 3 * (1 + operationTableSize)] = 3; /* O2J5 @ M3 */
  operationMatrix[14 + 4 * (1 + operationTableSize)] = 9; /* O2J5 @ M4 */
  operationMatrix[14 + 5 * (1 + operationTableSize)] = 8; /* O2J5 @ M5 */
  operationMatrix[14 + 6 * (1 + operationTableSize)] = 2; /* O2J5 @ M6 */
  operationMatrix[14 + 7 * (1 + operationTableSize)] = 8; /* O2J5 @ M7 */
  operationMatrix[14 + 8 * (1 + operationTableSize)] = 6; /* O2J5 @ M8 */
  operationMatrix[14 + 9 * (1 + operationTableSize)] = 1; /* O2J5 @ M9 */
  operationMatrix[14 + 10 * (1 + operationTableSize)] = 7; /* O2J5 @ M10 */

  operationMatrix[15 + 1 * (1 + operationTableSize)] = 6; /* O3J5 @ M1 */
  operationMatrix[15 + 2 * (1 + operationTableSize)] = 1; /* O3J5 @ M2 */
  operationMatrix[15 + 3 * (1 + operationTableSize)] = 4; /* O3J5 @ M3 */
  operationMatrix[15 + 4 * (1 + operationTableSize)] = 1; /* O3J5 @ M4 */
  operationMatrix[15 + 5 * (1 + operationTableSize)] = 10; /* O3J5 @ M5 */
  operationMatrix[15 + 6 * (1 + operationTableSize)] = 4; /* O3J5 @ M6 */
  operationMatrix[15 + 7 * (1 + operationTableSize)] = 3; /* O3J5 @ M7 */
  operationMatrix[15 + 8 * (1 + operationTableSize)] = 11; /* O3J5 @ M8 */
  operationMatrix[15 + 9 * (1 + operationTableSize)] = 13; /* O3J5 @ M9 */
  operationMatrix[15 + 10 * (1 + operationTableSize)] = 9; /* O3J5 @ M10 */

  operationMatrix[16 + 1 * (1 + operationTableSize)] = 8; /* O1J6 @ M1 */
  operationMatrix[16 + 2 * (1 + operationTableSize)] = 9; /* O1J6 @ M2 */
  operationMatrix[16 + 3 * (1 + operationTableSize)] = 10; /* O1J6 @ M3 */
  operationMatrix[16 + 4 * (1 + operationTableSize)] = 8; /* O1J6 @ M4 */
  operationMatrix[16 + 5 * (1 + operationTableSize)] = 4; /* O1J6 @ M5 */
  operationMatrix[16 + 6 * (1 + operationTableSize)] = 2; /* O1J6 @ M6 */
  operationMatrix[16 + 7 * (1 + operationTableSize)] = 7; /* O1J6 @ M7 */
  operationMatrix[16 + 8 * (1 + operationTableSize)] = 8; /* O1J6 @ M8 */
  operationMatrix[16 + 9 * (1 + operationTableSize)] = 3; /* O1J6 @ M9 */
  operationMatrix[16 + 10 * (1 + operationTableSize)] = 10; /* O1J6 @ M10 */

  operationMatrix[17 + 1 * (1 + operationTableSize)] = 7; /* O2J6 @ M1 */
  operationMatrix[17 + 2 * (1 + operationTableSize)] = 3; /* O2J6 @ M2 */
  operationMatrix[17 + 3 * (1 + operationTableSize)] = 12; /* O2J6 @ M3 */
  operationMatrix[17 + 4 * (1 + operationTableSize)] = 5; /* O2J6 @ M4 */
  operationMatrix[17 + 5 * (1 + operationTableSize)] = 4; /* O2J6 @ M5 */
  operationMatrix[17 + 6 * (1 + operationTableSize)] = 3; /* O2J6 @ M6 */
  operationMatrix[17 + 7 * (1 + operationTableSize)] = 6; /* O2J6 @ M7 */
  operationMatrix[17 + 8 * (1 + operationTableSize)] = 9; /* O2J6 @ M8 */
  operationMatrix[17 + 9 * (1 + operationTableSize)] = 2; /* O2J6 @ M9 */
  operationMatrix[17 + 10 * (1 + operationTableSize)] = 15; /* O2J6 @ M10 */

  operationMatrix[18 + 1 * (1 + operationTableSize)] = 4; /* O3J6 @ M1 */
  operationMatrix[18 + 2 * (1 + operationTableSize)] = 7; /* O3J6 @ M2 */
  operationMatrix[18 + 3 * (1 + operationTableSize)] = 3; /* O3J6 @ M3 */
  operationMatrix[18 + 4 * (1 + operationTableSize)] = 6; /* O3J6 @ M4 */
  operationMatrix[18 + 5 * (1 + operationTableSize)] = 3; /* O3J6 @ M5 */
  operationMatrix[18 + 6 * (1 + operationTableSize)] = 4; /* O3J6 @ M6 */
  operationMatrix[18 + 7 * (1 + operationTableSize)] = 1; /* O3J6 @ M7 */
  operationMatrix[18 + 8 * (1 + operationTableSize)] = 5; /* O3J6 @ M8 */
  operationMatrix[18 + 9 * (1 + operationTableSize)] = 1; /* O3J6 @ M9 */
  operationMatrix[18 + 10 * (1 + operationTableSize)] = 11; /* O3J6 @ M10 */

  operationMatrix[19 + 1 * (1 + operationTableSize)] = 1; /* O1J7 @ M1 */
  operationMatrix[19 + 2 * (1 + operationTableSize)] = 7; /* O1J7 @ M2 */
  operationMatrix[19 + 3 * (1 + operationTableSize)] = 8; /* O1J7 @ M3 */
  operationMatrix[19 + 4 * (1 + operationTableSize)] = 3; /* O1J7 @ M4 */
  operationMatrix[19 + 5 * (1 + operationTableSize)] = 4; /* O1J7 @ M5 */
  operationMatrix[19 + 6 * (1 + operationTableSize)] = 9; /* O1J7 @ M6 */
  operationMatrix[19 + 7 * (1 + operationTableSize)] = 4; /* O1J7 @ M7 */
  operationMatrix[19 + 8 * (1 + operationTableSize)] = 13; /* O1J7 @ M8 */
  operationMatrix[19 + 9 * (1 + operationTableSize)] = 10; /* O1J7 @ M9 */
  operationMatrix[19 + 10 * (1 + operationTableSize)] = 7; /* O1J7 @ M10 */

  operationMatrix[20 + 1 * (1 + operationTableSize)] = 3; /* O2J7 @ M1 */
  operationMatrix[20 + 2 * (1 + operationTableSize)] = 8; /* O2J7 @ M2 */
  operationMatrix[20 + 3 * (1 + operationTableSize)] = 1; /* O2J7 @ M3 */
  operationMatrix[20 + 4 * (1 + operationTableSize)] = 2; /* O2J7 @ M4 */
  operationMatrix[20 + 5 * (1 + operationTableSize)] = 3; /* O2J7 @ M5 */
  operationMatrix[20 + 6 * (1 + operationTableSize)] = 6; /* O2J7 @ M6 */
  operationMatrix[20 + 7 * (1 + operationTableSize)] = 11; /* O2J7 @ M7 */
  operationMatrix[20 + 8 * (1 + operationTableSize)] = 2; /* O2J7 @ M8 */
  operationMatrix[20 + 9 * (1 + operationTableSize)] = 13; /* O2J7 @ M9 */
  operationMatrix[20 + 10 * (1 + operationTableSize)] = 3; /* O2J7 @ M10 */

  operationMatrix[21 + 1 * (1 + operationTableSize)] = 5; /* O3J7 @ M1 */
  operationMatrix[21 + 2 * (1 + operationTableSize)] = 4; /* O3J7 @ M2 */
  operationMatrix[21 + 3 * (1 + operationTableSize)] = 2; /* O3J7 @ M3 */
  operationMatrix[21 + 4 * (1 + operationTableSize)] = 1; /* O3J7 @ M4 */
  operationMatrix[21 + 5 * (1 + operationTableSize)] = 2; /* O3J7 @ M5 */
  operationMatrix[21 + 6 * (1 + operationTableSize)] = 1; /* O3J7 @ M6 */
  operationMatrix[21 + 7 * (1 + operationTableSize)] = 8; /* O3J7 @ M7 */
  operationMatrix[21 + 8 * (1 + operationTableSize)] = 14; /* O3J7 @ M8 */
  operationMatrix[21 + 9 * (1 + operationTableSize)] = 5; /* O3J7 @ M9 */
  operationMatrix[21 + 10 * (1 + operationTableSize)] = 7; /* O3J7 @ M10 */

  operationMatrix[22 + 1 * (1 + operationTableSize)] = 5; /* O1J8 @ M1 */
  operationMatrix[22 + 2 * (1 + operationTableSize)] = 7; /* O1J8 @ M2 */
  operationMatrix[22 + 3 * (1 + operationTableSize)] = 11; /* O1J8 @ M3 */
  operationMatrix[22 + 4 * (1 + operationTableSize)] = 3; /* O1J8 @ M4 */
  operationMatrix[22 + 5 * (1 + operationTableSize)] = 2; /* O1J8 @ M5 */
  operationMatrix[22 + 6 * (1 + operationTableSize)] = 9; /* O1J8 @ M6 */
  operationMatrix[22 + 7 * (1 + operationTableSize)] = 8; /* O1J8 @ M7 */
  operationMatrix[22 + 8 * (1 + operationTableSize)] = 5; /* O1J8 @ M8 */
  operationMatrix[22 + 9 * (1 + operationTableSize)] = 12; /* O1J8 @ M9 */
  operationMatrix[22 + 10 * (1 + operationTableSize)] = 8; /* O1J8 @ M10 */

  operationMatrix[23 + 1 * (1 + operationTableSize)] = 8; /* O2J8 @ M1 */
  operationMatrix[23 + 2 * (1 + operationTableSize)] = 3; /* O2J8 @ M2 */
  operationMatrix[23 + 3 * (1 + operationTableSize)] = 10; /* O2J8 @ M3 */
  operationMatrix[23 + 4 * (1 + operationTableSize)] = 7; /* O2J8 @ M4 */
  operationMatrix[23 + 5 * (1 + operationTableSize)] = 5; /* O2J8 @ M5 */
  operationMatrix[23 + 6 * (1 + operationTableSize)] = 13; /* O2J8 @ M6 */
  operationMatrix[23 + 7 * (1 + operationTableSize)] = 4; /* O2J8 @ M7 */
  operationMatrix[23 + 8 * (1 + operationTableSize)] = 6; /* O2J8 @ M8 */
  operationMatrix[23 + 9 * (1 + operationTableSize)] = 8; /* O2J8 @ M9 */
  operationMatrix[23 + 10 * (1 + operationTableSize)] = 4; /* O2J8 @ M10 */

  operationMatrix[24 + 1 * (1 + operationTableSize)] = 6; /* O3J8 @ M1 */
  operationMatrix[24 + 2 * (1 + operationTableSize)] = 2; /* O3J8 @ M2 */
  operationMatrix[24 + 3 * (1 + operationTableSize)] = 13; /* O3J8 @ M3 */
  operationMatrix[24 + 4 * (1 + operationTableSize)] = 5; /* O3J8 @ M4 */
  operationMatrix[24 + 5 * (1 + operationTableSize)] = 4; /* O3J8 @ M5 */
  operationMatrix[24 + 6 * (1 + operationTableSize)] = 3; /* O3J8 @ M6 */
  operationMatrix[24 + 7 * (1 + operationTableSize)] = 5; /* O3J8 @ M7 */
  operationMatrix[24 + 8 * (1 + operationTableSize)] = 7; /* O3J8 @ M8 */
  operationMatrix[24 + 9 * (1 + operationTableSize)] = 9; /* O3J8 @ M9 */
  operationMatrix[24 + 10 * (1 + operationTableSize)] = 5; /* O3J8 @ M10 */

  operationMatrix[25 + 1 * (1 + operationTableSize)] = 3; /* O1J9 @ M1 */
  operationMatrix[25 + 2 * (1 + operationTableSize)] = 9; /* O1J9 @ M2 */
  operationMatrix[25 + 3 * (1 + operationTableSize)] = 1; /* O1J9 @ M3 */
  operationMatrix[25 + 4 * (1 + operationTableSize)] = 3; /* O1J9 @ M4 */
  operationMatrix[25 + 5 * (1 + operationTableSize)] = 8; /* O1J9 @ M5 */
  operationMatrix[25 + 6 * (1 + operationTableSize)] = 1; /* O1J9 @ M6 */
  operationMatrix[25 + 7 * (1 + operationTableSize)] = 6; /* O1J9 @ M7 */
  operationMatrix[25 + 8 * (1 + operationTableSize)] = 7; /* O1J9 @ M8 */
  operationMatrix[25 + 9 * (1 + operationTableSize)] = 5; /* O1J9 @ M9 */
  operationMatrix[25 + 10 * (1 + operationTableSize)] = 4; /* O1J9 @ M10 */

  operationMatrix[26 + 1 * (1 + operationTableSize)] = 4; /* O2J9 @ M1 */
  operationMatrix[26 + 2 * (1 + operationTableSize)] = 6; /* O2J9 @ M2 */
  operationMatrix[26 + 3 * (1 + operationTableSize)] = 2; /* O2J9 @ M3 */
  operationMatrix[26 + 4 * (1 + operationTableSize)] = 5; /* O2J9 @ M4 */
  operationMatrix[26 + 5 * (1 + operationTableSize)] = 7; /* O2J9 @ M5 */
  operationMatrix[26 + 6 * (1 + operationTableSize)] = 3; /* O2J9 @ M6 */
  operationMatrix[26 + 7 * (1 + operationTableSize)] = 1; /* O2J9 @ M7 */
  operationMatrix[26 + 8 * (1 + operationTableSize)] = 9; /* O2J9 @ M8 */
  operationMatrix[26 + 9 * (1 + operationTableSize)] = 6; /* O2J9 @ M9 */
  operationMatrix[26 + 10 * (1 + operationTableSize)] = 7; /* O2J9 @ M10 */

  operationMatrix[27 + 1 * (1 + operationTableSize)] = 8; /* O3J9 @ M1 */
  operationMatrix[27 + 2 * (1 + operationTableSize)] = 5; /* O3J9 @ M2 */
  operationMatrix[27 + 3 * (1 + operationTableSize)] = 4; /* O3J9 @ M3 */
  operationMatrix[27 + 4 * (1 + operationTableSize)] = 8; /* O3J9 @ M4 */
  operationMatrix[27 + 5 * (1 + operationTableSize)] = 6; /* O3J9 @ M5 */
  operationMatrix[27 + 6 * (1 + operationTableSize)] = 1; /* O3J9 @ M6 */
  operationMatrix[27 + 7 * (1 + operationTableSize)] = 2; /* O3J9 @ M7 */
  operationMatrix[27 + 8 * (1 + operationTableSize)] = 3; /* O3J9 @ M8 */
  operationMatrix[27 + 9 * (1 + operationTableSize)] = 10; /* O3J9 @ M9 */
  operationMatrix[27 + 10 * (1 + operationTableSize)] = 12; /* O3J9 @ M10 */

  operationMatrix[28 + 1 * (1 + operationTableSize)] = 4; /* O1J10 @ M1 */
  operationMatrix[28 + 2 * (1 + operationTableSize)] = 3; /* O1J10 @ M2 */
  operationMatrix[28 + 3 * (1 + operationTableSize)] = 1; /* O1J10 @ M3 */
  operationMatrix[28 + 4 * (1 + operationTableSize)] = 6; /* O1J10 @ M4 */
  operationMatrix[28 + 5 * (1 + operationTableSize)] = 7; /* O1J10 @ M5 */
  operationMatrix[28 + 6 * (1 + operationTableSize)] = 1; /* O1J10 @ M6 */
  operationMatrix[28 + 7 * (1 + operationTableSize)] = 2; /* O1J10 @ M7 */
  operationMatrix[28 + 8 * (1 + operationTableSize)] = 6; /* O1J10 @ M8 */
  operationMatrix[28 + 9 * (1 + operationTableSize)] = 20; /* O1J10 @ M9 */
  operationMatrix[28 + 10 * (1 + operationTableSize)] = 6; /* O1J10 @ M10 */

  operationMatrix[29 + 1 * (1 + operationTableSize)] = 3; /* O2J10 @ M1 */
  operationMatrix[29 + 2 * (1 + operationTableSize)] = 1; /* O2J10 @ M2 */
  operationMatrix[29 + 3 * (1 + operationTableSize)] = 8; /* O2J10 @ M3 */
  operationMatrix[29 + 4 * (1 + operationTableSize)] = 1; /* O2J10 @ M4 */
  operationMatrix[29 + 5 * (1 + operationTableSize)] = 9; /* O2J10 @ M5 */
  operationMatrix[29 + 6 * (1 + operationTableSize)] = 4; /* O2J10 @ M6 */
  operationMatrix[29 + 7 * (1 + operationTableSize)] = 1; /* O2J10 @ M7 */
  operationMatrix[29 + 8 * (1 + operationTableSize)] = 4; /* O2J10 @ M8 */
  operationMatrix[29 + 9 * (1 + operationTableSize)] = 17; /* O2J10 @ M9 */
  operationMatrix[29 + 10 * (1 + operationTableSize)] = 15; /* O2J10 @ M10 */

  operationMatrix[30 + 1 * (1 + operationTableSize)] = 9; /* O3J10 @ M1 */
  operationMatrix[30 + 2 * (1 + operationTableSize)] = 2; /* O3J10 @ M2 */
  operationMatrix[30 + 3 * (1 + operationTableSize)] = 4; /* O3J10 @ M3 */
  operationMatrix[30 + 4 * (1 + operationTableSize)] = 2; /* O3J10 @ M4 */
  operationMatrix[30 + 5 * (1 + operationTableSize)] = 3; /* O3J10 @ M5 */
  operationMatrix[30 + 6 * (1 + operationTableSize)] = 5; /* O3J10 @ M6 */
  operationMatrix[30 + 7 * (1 + operationTableSize)] = 2; /* O3J10 @ M7 */
  operationMatrix[30 + 8 * (1 + operationTableSize)] = 4; /* O3J10 @ M8 */
  operationMatrix[30 + 9 * (1 + operationTableSize)] = 10; /* O3J10 @ M9 */
  operationMatrix[30 + 10 * (1 + operationTableSize)] = 23; /* O3J10 @ M10 */

  /* *************************************** */
  /* PRODUCTION TABLE */
  /* *************************************** */

  productionTableSize = 10;
  productionTable = (PPART *) malloc((1 + productionTableSize) * sizeof(PPART));
  productionTable[0] = NULL;

  if ((NUMPARTS == 0) || (NUMPARTS > productionTableSize)) {
    NUMPARTS = productionTableSize;
  }

  /* *************************************** */
  /* PART TABLE */
  /* *************************************** */

  pnum = 0;
  part = 0;

  partTableSize = 10; /* number of parts (including subparts) */
  partTable = (PART *) malloc((1 + partTableSize) * sizeof(PART));
  partTable[0].name = NULL; partTable[0].n = 0; partTable[0].X = NULL;
  for (i = 1; i <= partTableSize; i++)  {
    partTable[i].idx = i;
    partTable[i].color = 0;
  }

  /* *************************************** */
  /* JOB J1 */
  /* *************************************** */
  
  part++;

  pnum++; xnum = 0; onum = 0; /* next part, id = 1 */

  partTable[pnum].name = strdup("P1");
  partTable[pnum].part = part;
  partTable[pnum].n = 1; /* number of sequences */
  partTable[pnum].code = NULL; partTable[pnum].feasible = NULL;
  partTable[pnum].X = (SEQUENCE *) malloc((1 + partTable[pnum].n) * sizeof(SEQUENCE));
  partTable[pnum].X[0].n = 0; partTable[pnum].X[0].O = NULL;

  xnum++; onum = 0; /* next sequence */

  partTable[pnum].X[xnum].n = 3; /* number of items for the 1st sequence in J1 */
  partTable[pnum].X[xnum].O = (SEQUENCEITEM *) malloc((1 + partTable[pnum].X[xnum].n) * sizeof(SEQUENCEITEM));
  partTable[pnum].X[xnum].O[0].type = NONE; partTable[pnum].X[xnum].O[0].ITEM = NULL;
  partTable[pnum].X[xnum].O[++onum].type = DEFAULT; partTable[pnum].X[xnum].O[onum].ITEM = (void *) &(operationTable[1]); /* O1J1 */
  partTable[pnum].X[xnum].O[++onum].type = DEFAULT; partTable[pnum].X[xnum].O[onum].ITEM = (void *) &(operationTable[2]); /* O2J1 */
  partTable[pnum].X[xnum].O[++onum].type = DEFAULT; partTable[pnum].X[xnum].O[onum].ITEM = (void *) &(operationTable[3]); /* O3J1 */

  productionTable[part] = &(partTable[pnum]);
  partTable[pnum].color = 0;

  /* ADDVARIABLE(w,r) */
  ADDVARIABLE(1,1);
  ADDVARIABLE(2,2);
  ADDVARIABLE(3,3);

  /* *************************************** */
  /* JOB J2 */
  /* *************************************** */

  part++;

  pnum++; xnum = 0; onum = 0; /* next part, id = 2 */

  partTable[pnum].name = strdup("P2");
  partTable[pnum].part = part;
  partTable[pnum].n = 1; /* number of sequences */
  partTable[pnum].code = NULL; partTable[pnum].feasible = NULL;
  partTable[pnum].X = (SEQUENCE *) malloc((1 + partTable[pnum].n) * sizeof(SEQUENCE));
  partTable[pnum].X[0].n = 0; partTable[pnum].X[0].O = NULL;

  xnum++; onum = 0; /* next sequence */

  partTable[pnum].X[xnum].n = 3; /* number of items for the 1st sequence in J2 */
  partTable[pnum].X[xnum].O = (SEQUENCEITEM *) malloc((1 + partTable[pnum].X[xnum].n) * sizeof(SEQUENCEITEM));
  partTable[pnum].X[xnum].O[0].type = NONE; partTable[pnum].X[xnum].O[0].ITEM = NULL;
  partTable[pnum].X[xnum].O[++onum].type = DEFAULT; partTable[pnum].X[xnum].O[onum].ITEM = (void *) &(operationTable[4]); /* O1J2 */
  partTable[pnum].X[xnum].O[++onum].type = DEFAULT; partTable[pnum].X[xnum].O[onum].ITEM = (void *) &(operationTable[5]); /* O2J2 */
  partTable[pnum].X[xnum].O[++onum].type = DEFAULT; partTable[pnum].X[xnum].O[onum].ITEM = (void *) &(operationTable[6]); /* O3J2 */

  productionTable[part] = &(partTable[pnum]);
  partTable[pnum].color = 60;

  /* ADDVARIABLE(w,r) */
  ADDVARIABLE(4,1);
  ADDVARIABLE(5,2);
  ADDVARIABLE(6,3);

  /* *************************************** */
  /* JOB J3 */
  /* *************************************** */

  part++;

  pnum++; xnum = 0; onum = 0; /* next part, id = 3 */

  partTable[pnum].name = strdup("P3");
  partTable[pnum].part = part;
  partTable[pnum].n = 1; /* number of sequences */
  partTable[pnum].code = NULL; partTable[pnum].feasible = NULL;
  partTable[pnum].X = (SEQUENCE *) malloc((1 + partTable[pnum].n) * sizeof(SEQUENCE));
  partTable[pnum].X[0].n = 0; partTable[pnum].X[0].O = NULL;

  xnum++; onum = 0; /* next sequence */

  partTable[pnum].X[xnum].n = 3; /* number of items for the 1st sequence in J3 */
  partTable[pnum].X[xnum].O = (SEQUENCEITEM *) malloc((1 + partTable[pnum].X[xnum].n) * sizeof(SEQUENCEITEM));
  partTable[pnum].X[xnum].O[0].type = NONE; partTable[pnum].X[xnum].O[0].ITEM = NULL;
  partTable[pnum].X[xnum].O[++onum].type = DEFAULT; partTable[pnum].X[xnum].O[onum].ITEM = (void *) &(operationTable[7]); /* O1J3 */
  partTable[pnum].X[xnum].O[++onum].type = DEFAULT; partTable[pnum].X[xnum].O[onum].ITEM = (void *) &(operationTable[8]); /* O2J3 */
  partTable[pnum].X[xnum].O[++onum].type = DEFAULT; partTable[pnum].X[xnum].O[onum].ITEM = (void *) &(operationTable[9]); /* O3J3 */

  productionTable[part] = &(partTable[pnum]);
  partTable[pnum].color = 140;

  /* ADDVARIABLE(w,r) */
  ADDVARIABLE(7,1);
  ADDVARIABLE(8,2);
  ADDVARIABLE(9,3);

  /* *************************************** */
  /* JOB J4 */
  /* *************************************** */

  part++;

  pnum++; xnum = 0; onum = 0; /* next part, id = 4 */

  partTable[pnum].name = strdup("P4");
  partTable[pnum].part = part;
  partTable[pnum].n = 1; /* number of sequences */
  partTable[pnum].code = NULL; partTable[pnum].feasible = NULL;
  partTable[pnum].X = (SEQUENCE *) malloc((1 + partTable[pnum].n) * sizeof(SEQUENCE));
  partTable[pnum].X[0].n = 0; partTable[pnum].X[0].O = NULL;

  xnum++; onum = 0; /* next sequence */

  partTable[pnum].X[xnum].n = 3; /* number of items for the 1st sequence in J4 */
  partTable[pnum].X[xnum].O = (SEQUENCEITEM *) malloc((1 + partTable[pnum].X[xnum].n) * sizeof(SEQUENCEITEM));
  partTable[pnum].X[xnum].O[0].type = NONE; partTable[pnum].X[xnum].O[0].ITEM = NULL;
  partTable[pnum].X[xnum].O[++onum].type = DEFAULT; partTable[pnum].X[xnum].O[onum].ITEM = (void *) &(operationTable[10]); /* O1J4 */
  partTable[pnum].X[xnum].O[++onum].type = DEFAULT; partTable[pnum].X[xnum].O[onum].ITEM = (void *) &(operationTable[11]); /* O2J4 */
  partTable[pnum].X[xnum].O[++onum].type = DEFAULT; partTable[pnum].X[xnum].O[onum].ITEM = (void *) &(operationTable[12]); /* O3J4 */

  productionTable[part] = &(partTable[pnum]);
  partTable[pnum].color = 180;

  /* ADDVARIABLE(w,r) */
  ADDVARIABLE(10,1);
  ADDVARIABLE(11,2);
  ADDVARIABLE(12,3);

  /* *************************************** */
  /* JOB J5 */
  /* *************************************** */

  part++;

  pnum++; xnum = 0; onum = 0; /* next part, id = 5 */

  partTable[pnum].name = strdup("P5");
  partTable[pnum].part = part;
  partTable[pnum].n = 1; /* number of sequences */
  partTable[pnum].code = NULL; partTable[pnum].feasible = NULL;
  partTable[pnum].X = (SEQUENCE *) malloc((1 + partTable[pnum].n) * sizeof(SEQUENCE));
  partTable[pnum].X[0].n = 0; partTable[pnum].X[0].O = NULL;

  xnum++; onum = 0; /* next sequence */

  partTable[pnum].X[xnum].n = 3; /* number of items for the 1st sequence in J5 */
  partTable[pnum].X[xnum].O = (SEQUENCEITEM *) malloc((1 + partTable[pnum].X[xnum].n) * sizeof(SEQUENCEITEM));
  partTable[pnum].X[xnum].O[0].type = NONE; partTable[pnum].X[xnum].O[0].ITEM = NULL;
  partTable[pnum].X[xnum].O[++onum].type = DEFAULT; partTable[pnum].X[xnum].O[onum].ITEM = (void *) &(operationTable[13]); /* O1J5 */
  partTable[pnum].X[xnum].O[++onum].type = DEFAULT; partTable[pnum].X[xnum].O[onum].ITEM = (void *) &(operationTable[14]); /* O2J5 */
  partTable[pnum].X[xnum].O[++onum].type = DEFAULT; partTable[pnum].X[xnum].O[onum].ITEM = (void *) &(operationTable[15]); /* O3J5 */

  productionTable[part] = &(partTable[pnum]);
  partTable[pnum].color = 230;

  /* ADDVARIABLE(w,r) */
  ADDVARIABLE(13,1);
  ADDVARIABLE(14,2);
  ADDVARIABLE(15,3);

  /* *************************************** */
  /* JOB J6 */
  /* *************************************** */

  part++;

  pnum++; xnum = 0; onum = 0; /* next part, id = 6 */

  partTable[pnum].name = strdup("P6");
  partTable[pnum].part = part;
  partTable[pnum].n = 1; /* number of sequences */
  partTable[pnum].code = NULL; partTable[pnum].feasible = NULL;
  partTable[pnum].X = (SEQUENCE *) malloc((1 + partTable[pnum].n) * sizeof(SEQUENCE));
  partTable[pnum].X[0].n = 0; partTable[pnum].X[0].O = NULL;

  xnum++; onum = 0; /* next sequence */

  partTable[pnum].X[xnum].n = 3; /* number of items for the 1st sequence in J6 */
  partTable[pnum].X[xnum].O = (SEQUENCEITEM *) malloc((1 + partTable[pnum].X[xnum].n) * sizeof(SEQUENCEITEM));
  partTable[pnum].X[xnum].O[0].type = NONE; partTable[pnum].X[xnum].O[0].ITEM = NULL;
  partTable[pnum].X[xnum].O[++onum].type = DEFAULT; partTable[pnum].X[xnum].O[onum].ITEM = (void *) &(operationTable[16]); /* O1J6 */
  partTable[pnum].X[xnum].O[++onum].type = DEFAULT; partTable[pnum].X[xnum].O[onum].ITEM = (void *) &(operationTable[17]); /* O2J6 */
  partTable[pnum].X[xnum].O[++onum].type = DEFAULT; partTable[pnum].X[xnum].O[onum].ITEM = (void *) &(operationTable[18]); /* O3J6 */

  productionTable[part] = &(partTable[pnum]);
  partTable[pnum].color = 300;

  /* ADDVARIABLE(w,r) */
  ADDVARIABLE(16,1);
  ADDVARIABLE(17,2);
  ADDVARIABLE(18,3);

  /* *************************************** */
  /* JOB J7 */
  /* *************************************** */

  part++;

  pnum++; xnum = 0; onum = 0; /* next part, id = 7 */

  partTable[pnum].name = strdup("P7");
  partTable[pnum].part = part;
  partTable[pnum].n = 1; /* number of sequences */
  partTable[pnum].code = NULL; partTable[pnum].feasible = NULL;
  partTable[pnum].X = (SEQUENCE *) malloc((1 + partTable[pnum].n) * sizeof(SEQUENCE));
  partTable[pnum].X[0].n = 0; partTable[pnum].X[0].O = NULL;

  xnum++; onum = 0; /* next sequence */

  partTable[pnum].X[xnum].n = 3; /* number of items for the 1st sequence in J7 */
  partTable[pnum].X[xnum].O = (SEQUENCEITEM *) malloc((1 + partTable[pnum].X[xnum].n) * sizeof(SEQUENCEITEM));
  partTable[pnum].X[xnum].O[0].type = NONE; partTable[pnum].X[xnum].O[0].ITEM = NULL;
  partTable[pnum].X[xnum].O[++onum].type = DEFAULT; partTable[pnum].X[xnum].O[onum].ITEM = (void *) &(operationTable[19]); /* O1J7 */
  partTable[pnum].X[xnum].O[++onum].type = DEFAULT; partTable[pnum].X[xnum].O[onum].ITEM = (void *) &(operationTable[20]); /* O2J7 */
  partTable[pnum].X[xnum].O[++onum].type = DEFAULT; partTable[pnum].X[xnum].O[onum].ITEM = (void *) &(operationTable[21]); /* O3J7 */

  productionTable[part] = &(partTable[pnum]);
  partTable[pnum].color = 30;

  /* ADDVARIABLE(w,r) */
  ADDVARIABLE(19,1);
  ADDVARIABLE(20,2);
  ADDVARIABLE(21,3);

  /* *************************************** */
  /* JOB J8 */
  /* *************************************** */

  part++;

  pnum++; xnum = 0; onum = 0; /* next part, id = 8 */

  partTable[pnum].name = strdup("P8");
  partTable[pnum].part = part;
  partTable[pnum].n = 1; /* number of sequences */
  partTable[pnum].code = NULL; partTable[pnum].feasible = NULL;
  partTable[pnum].X = (SEQUENCE *) malloc((1 + partTable[pnum].n) * sizeof(SEQUENCE));
  partTable[pnum].X[0].n = 0; partTable[pnum].X[0].O = NULL;

  xnum++; onum = 0; /* next sequence */

  partTable[pnum].X[xnum].n = 3; /* number of items for the 1st sequence in J8 */
  partTable[pnum].X[xnum].O = (SEQUENCEITEM *) malloc((1 + partTable[pnum].X[xnum].n) * sizeof(SEQUENCEITEM));
  partTable[pnum].X[xnum].O[0].type = NONE; partTable[pnum].X[xnum].O[0].ITEM = NULL;
  partTable[pnum].X[xnum].O[++onum].type = DEFAULT; partTable[pnum].X[xnum].O[onum].ITEM = (void *) &(operationTable[22]); /* O1J8 */
  partTable[pnum].X[xnum].O[++onum].type = DEFAULT; partTable[pnum].X[xnum].O[onum].ITEM = (void *) &(operationTable[23]); /* O2J8 */
  partTable[pnum].X[xnum].O[++onum].type = DEFAULT; partTable[pnum].X[xnum].O[onum].ITEM = (void *) &(operationTable[24]); /* O3J8 */

  productionTable[part] = &(partTable[pnum]);
  partTable[pnum].color = 270;

  /* ADDVARIABLE(w,r) */
  ADDVARIABLE(22,1);
  ADDVARIABLE(23,2);
  ADDVARIABLE(24,3);

  /* *************************************** */
  /* JOB J9 */
  /* *************************************** */

  part++;

  pnum++; xnum = 0; onum = 0; /* next part, id = 9 */

  partTable[pnum].name = strdup("P9");
  partTable[pnum].part = part;
  partTable[pnum].n = 1; /* number of sequences */
  partTable[pnum].code = NULL; partTable[pnum].feasible = NULL;
  partTable[pnum].X = (SEQUENCE *) malloc((1 + partTable[pnum].n) * sizeof(SEQUENCE));
  partTable[pnum].X[0].n = 0; partTable[pnum].X[0].O = NULL;

  xnum++; onum = 0; /* next sequence */

  partTable[pnum].X[xnum].n = 3; /* number of items for the 1st sequence in J9 */
  partTable[pnum].X[xnum].O = (SEQUENCEITEM *) malloc((1 + partTable[pnum].X[xnum].n) * sizeof(SEQUENCEITEM));
  partTable[pnum].X[xnum].O[0].type = NONE; partTable[pnum].X[xnum].O[0].ITEM = NULL;
  partTable[pnum].X[xnum].O[++onum].type = DEFAULT; partTable[pnum].X[xnum].O[onum].ITEM = (void *) &(operationTable[25]); /* O1J9 */
  partTable[pnum].X[xnum].O[++onum].type = DEFAULT; partTable[pnum].X[xnum].O[onum].ITEM = (void *) &(operationTable[26]); /* O2J9 */
  partTable[pnum].X[xnum].O[++onum].type = DEFAULT; partTable[pnum].X[xnum].O[onum].ITEM = (void *) &(operationTable[27]); /* O3J9 */

  productionTable[part] = &(partTable[pnum]);
  partTable[pnum].color = 90;

  /* ADDVARIABLE(w,r) */
  ADDVARIABLE(25,1);
  ADDVARIABLE(26,2);
  ADDVARIABLE(27,3);

  /* *************************************** */
  /* JOB J10 */
  /* *************************************** */

  part++;

  pnum++; xnum = 0; onum = 0; /* next part, id = 10 */

  partTable[pnum].name = strdup("P10");
  partTable[pnum].part = part;
  partTable[pnum].n = 1; /* number of sequences */
  partTable[pnum].code = NULL; partTable[pnum].feasible = NULL;
  partTable[pnum].X = (SEQUENCE *) malloc((1 + partTable[pnum].n) * sizeof(SEQUENCE));
  partTable[pnum].X[0].n = 0; partTable[pnum].X[0].O = NULL;

  xnum++; onum = 0; /* next sequence */

  partTable[pnum].X[xnum].n = 3; /* number of items for the 1st sequence in J10 */
  partTable[pnum].X[xnum].O = (SEQUENCEITEM *) malloc((1 + partTable[pnum].X[xnum].n) * sizeof(SEQUENCEITEM));
  partTable[pnum].X[xnum].O[0].type = NONE; partTable[pnum].X[xnum].O[0].ITEM = NULL;
  partTable[pnum].X[xnum].O[++onum].type = DEFAULT; partTable[pnum].X[xnum].O[onum].ITEM = (void *) &(operationTable[28]); /* O1J10 */
  partTable[pnum].X[xnum].O[++onum].type = DEFAULT; partTable[pnum].X[xnum].O[onum].ITEM = (void *) &(operationTable[29]); /* O2J10 */
  partTable[pnum].X[xnum].O[++onum].type = DEFAULT; partTable[pnum].X[xnum].O[onum].ITEM = (void *) &(operationTable[30]); /* O3J10 */

  productionTable[part] = &(partTable[pnum]);
  partTable[pnum].color = 200;

  /* ADDVARIABLE(w,r) */
  ADDVARIABLE(28,1);
  ADDVARIABLE(29,2);
  ADDVARIABLE(30,3);
}

/* ************************************************************************************ */
/* ************************************************************************************ */
/* ************************************************************************************ */

void
generateSystemKacem15x10()
{
  unsigned int part,pnum,xnum,onum;
  unsigned int i,j;

  if (NUMPARTS == 0) NUMPARTS = 15;
  if (NUMMACHINES == 0) NUMMACHINES = 10;

  MAXSEQUENCELENGTH = 4;

  /* user defined parts, sequences, and operations start at index 1 */

  /* *************************************** */
  /* OPERATION TABLE */
  /* *************************************** */
  
  operationTableSize = 56; /* number of operations */
  operationTable = (OPERATION *) malloc((1 + operationTableSize) * sizeof(OPERATION));
  (operationTable)[0].name = NULL;
  for (i = 1; i <= operationTableSize; i++)  (operationTable)[i].idx = i;

  (operationTable)[1].name = strdup("O1J1");
  (operationTable)[2].name = strdup("O2J1");
  (operationTable)[3].name = strdup("O3J1");
  (operationTable)[4].name = strdup("O4J1");
  (operationTable)[5].name = strdup("O1J2");
  (operationTable)[6].name = strdup("O2J2");
  (operationTable)[7].name = strdup("O3J2");
  (operationTable)[8].name = strdup("O4J2");
  (operationTable)[9].name = strdup("O1J3");
  (operationTable)[10].name = strdup("O2J3");
  (operationTable)[11].name = strdup("O3J3");
  (operationTable)[12].name = strdup("O4J3");
  (operationTable)[13].name = strdup("O1J4");
  (operationTable)[14].name = strdup("O2J4");
  (operationTable)[15].name = strdup("O3J4");
  (operationTable)[16].name = strdup("O4J4");
  (operationTable)[17].name = strdup("O1J5");
  (operationTable)[18].name = strdup("O2J5");
  (operationTable)[19].name = strdup("O3J5");
  (operationTable)[20].name = strdup("O4J5");
  (operationTable)[21].name = strdup("O1J6");
  (operationTable)[22].name = strdup("O2J6");
  (operationTable)[23].name = strdup("O1J7");
  (operationTable)[24].name = strdup("O2J7");
  (operationTable)[25].name = strdup("O1J8");
  (operationTable)[26].name = strdup("O2J8");
  (operationTable)[27].name = strdup("O3J8");
  (operationTable)[28].name = strdup("O4J8");
  (operationTable)[29].name = strdup("O1J9");
  (operationTable)[30].name = strdup("O2J9");
  (operationTable)[31].name = strdup("O3J9");
  (operationTable)[32].name = strdup("O4J9");
  (operationTable)[33].name = strdup("O1J10");
  (operationTable)[34].name = strdup("O2J10");
  (operationTable)[35].name = strdup("O3J10");
  (operationTable)[36].name = strdup("O4J10");
  (operationTable)[37].name = strdup("O1J11");
  (operationTable)[38].name = strdup("O2J11");
  (operationTable)[39].name = strdup("O3J11");
  (operationTable)[40].name = strdup("O4J11");
  (operationTable)[41].name = strdup("O1J12");
  (operationTable)[42].name = strdup("O2J12");
  (operationTable)[43].name = strdup("O3J12");
  (operationTable)[44].name = strdup("O4J12");
  (operationTable)[45].name = strdup("O1J13");
  (operationTable)[46].name = strdup("O2J13");
  (operationTable)[47].name = strdup("O3J13");
  (operationTable)[48].name = strdup("O4J13");
  (operationTable)[49].name = strdup("O1J14");
  (operationTable)[50].name = strdup("O2J14");
  (operationTable)[51].name = strdup("O3J14");
  (operationTable)[52].name = strdup("O4J14");
  (operationTable)[53].name = strdup("O1J15");
  (operationTable)[54].name = strdup("O2J15");
  (operationTable)[55].name = strdup("O3J15");
  (operationTable)[56].name = strdup("O4J15");

  /* *************************************** */
  /* MACHINE TABLE */
  /* *************************************** */
  
  machineTableSize = 10; /* number of machines */
  machineTable = (MACHINE *) malloc((1 + machineTableSize) * sizeof(MACHINE));
  machineTable[0].name = NULL;
  for (i = 1; i <= machineTableSize; i++) machineTable[i].idx = i;

  machineTable[1].name = strdup("M1"); machineTable[1].n = 1;
  machineTable[2].name = strdup("M2"); machineTable[2].n = 1;
  machineTable[3].name = strdup("M3"); machineTable[3].n = 1;
  machineTable[4].name = strdup("M4"); machineTable[4].n = 1;
  machineTable[5].name = strdup("M5"); machineTable[5].n = 1;
  machineTable[6].name = strdup("M6"); machineTable[6].n = 1;
  machineTable[7].name = strdup("M7"); machineTable[7].n = 1;
  machineTable[8].name = strdup("M8"); machineTable[8].n = 1;
  machineTable[9].name = strdup("M9"); machineTable[9].n = 1;
  machineTable[10].name = strdup("M10"); machineTable[10].n = 1;

  if ((NUMMACHINES == 0) || (NUMMACHINES > machineTableSize)) {
    NUMMACHINES = machineTableSize;
  }

  /* *************************************** */
  /* OPERATION MATRIX */
  /* *************************************** */

  operationMatrix = (unsigned int *)
    malloc((1 + machineTableSize) * (1 + operationTableSize) * sizeof(unsigned int));

  for (i = 0; i <= operationTableSize; i++) {
    for (j = 0; j <= machineTableSize; j++) {
      (operationMatrix)[i + j * (1 + operationTableSize)] = 0; /* Oi @ mj */
    }
  }

  operationMatrix[1 + 1 * (1 + operationTableSize)] = 1; /* O1J1 @ M1 */
  operationMatrix[1 + 2 * (1 + operationTableSize)] = 4; /* O1J1 @ M2 */
  operationMatrix[1 + 3 * (1 + operationTableSize)] = 6; /* O1J1 @ M3 */
  operationMatrix[1 + 4 * (1 + operationTableSize)] = 9; /* O1J1 @ M4 */
  operationMatrix[1 + 5 * (1 + operationTableSize)] = 3; /* O1J1 @ M5 */
  operationMatrix[1 + 6 * (1 + operationTableSize)] = 5; /* O1J1 @ M6 */
  operationMatrix[1 + 7 * (1 + operationTableSize)] = 2; /* O1J1 @ M7 */
  operationMatrix[1 + 8 * (1 + operationTableSize)] = 8; /* O1J1 @ M8 */
  operationMatrix[1 + 9 * (1 + operationTableSize)] = 9; /* O1J1 @ M9 */
  operationMatrix[1 + 10 * (1 + operationTableSize)] = 4; /* O1J1 @ M10 */

  operationMatrix[2 + 1 * (1 + operationTableSize)] = 1; /* O2J1 @ M1 */
  operationMatrix[2 + 2 * (1 + operationTableSize)] = 1; /* O2J1 @ M2 */
  operationMatrix[2 + 3 * (1 + operationTableSize)] = 3; /* O2J1 @ M3 */
  operationMatrix[2 + 4 * (1 + operationTableSize)] = 4; /* O2J1 @ M4 */
  operationMatrix[2 + 5 * (1 + operationTableSize)] = 8; /* O2J1 @ M5 */
  operationMatrix[2 + 6 * (1 + operationTableSize)] = 10; /* O2J1 @ M6 */
  operationMatrix[2 + 7 * (1 + operationTableSize)] = 4; /* O2J1 @ M7 */
  operationMatrix[2 + 8 * (1 + operationTableSize)] = 11; /* O2J1 @ M8 */
  operationMatrix[2 + 9 * (1 + operationTableSize)] = 4; /* O2J1 @ M9 */
  operationMatrix[2 + 10 * (1 + operationTableSize)] = 3; /* O2J1 @ M10 */

  operationMatrix[3 + 1 * (1 + operationTableSize)] = 2; /* O3J1 @ M1 */
  operationMatrix[3 + 2 * (1 + operationTableSize)] = 5; /* O3J1 @ M2 */
  operationMatrix[3 + 3 * (1 + operationTableSize)] = 1; /* O3J1 @ M3 */
  operationMatrix[3 + 4 * (1 + operationTableSize)] = 5; /* O3J1 @ M4 */
  operationMatrix[3 + 5 * (1 + operationTableSize)] = 6; /* O3J1 @ M5 */
  operationMatrix[3 + 6 * (1 + operationTableSize)] = 9; /* O3J1 @ M6 */
  operationMatrix[3 + 7 * (1 + operationTableSize)] = 5; /* O3J1 @ M7 */
  operationMatrix[3 + 8 * (1 + operationTableSize)] = 10; /* O3J1 @ M8 */
  operationMatrix[3 + 9 * (1 + operationTableSize)] = 3; /* O3J1 @ M9 */
  operationMatrix[3 + 10 * (1 + operationTableSize)] = 2; /* O3J1 @ M10 */

  operationMatrix[4 + 1 * (1 + operationTableSize)] = 10; /* O4J1 @ M1 */
  operationMatrix[4 + 2 * (1 + operationTableSize)] = 4; /* O4J1 @ M2 */
  operationMatrix[4 + 3 * (1 + operationTableSize)] = 5; /* O4J1 @ M3 */
  operationMatrix[4 + 4 * (1 + operationTableSize)] = 9; /* O4J1 @ M4 */
  operationMatrix[4 + 5 * (1 + operationTableSize)] = 8; /* O4J1 @ M5 */
  operationMatrix[4 + 6 * (1 + operationTableSize)] = 4; /* O4J1 @ M6 */
  operationMatrix[4 + 7 * (1 + operationTableSize)] = 15; /* O4J1 @ M7 */
  operationMatrix[4 + 8 * (1 + operationTableSize)] = 8; /* O4J1 @ M8 */
  operationMatrix[4 + 9 * (1 + operationTableSize)] = 4; /* O4J1 @ M9 */
  operationMatrix[4 + 10 * (1 + operationTableSize)] = 4; /* O4J1 @ M10 */

  operationMatrix[5 + 1 * (1 + operationTableSize)] = 4; /* O1J2 @ M1 */
  operationMatrix[5 + 2 * (1 + operationTableSize)] = 8; /* O1J2 @ M2 */
  operationMatrix[5 + 3 * (1 + operationTableSize)] = 7; /* O1J2 @ M3 */
  operationMatrix[5 + 4 * (1 + operationTableSize)] = 1; /* O1J2 @ M4 */
  operationMatrix[5 + 5 * (1 + operationTableSize)] = 9; /* O1J2 @ M5 */
  operationMatrix[5 + 6 * (1 + operationTableSize)] = 6; /* O1J2 @ M6 */
  operationMatrix[5 + 7 * (1 + operationTableSize)] = 1; /* O1J2 @ M7 */
  operationMatrix[5 + 8 * (1 + operationTableSize)] = 10; /* O1J2 @ M8 */
  operationMatrix[5 + 9 * (1 + operationTableSize)] = 7; /* O1J2 @ M9 */
  operationMatrix[5 + 10 * (1 + operationTableSize)] = 1; /* O1J2 @ M10 */

  operationMatrix[6 + 1 * (1 + operationTableSize)] = 6; /* O2J2 @ M1 */
  operationMatrix[6 + 2 * (1 + operationTableSize)] = 11; /* O2J2 @ M2 */
  operationMatrix[6 + 3 * (1 + operationTableSize)] = 2; /* O2J2 @ M3 */
  operationMatrix[6 + 4 * (1 + operationTableSize)] = 7; /* O2J2 @ M4 */
  operationMatrix[6 + 5 * (1 + operationTableSize)] = 5; /* O2J2 @ M5 */
  operationMatrix[6 + 6 * (1 + operationTableSize)] = 3; /* O2J2 @ M6 */
  operationMatrix[6 + 7 * (1 + operationTableSize)] = 5; /* O2J2 @ M7 */
  operationMatrix[6 + 8 * (1 + operationTableSize)] = 14; /* O2J2 @ M8 */
  operationMatrix[6 + 9 * (1 + operationTableSize)] = 9; /* O2J2 @ M9 */
  operationMatrix[6 + 10 * (1 + operationTableSize)] = 2; /* O2J2 @ M10 */

  operationMatrix[7 + 1 * (1 + operationTableSize)] = 8; /* O3J2 @ M1 */
  operationMatrix[7 + 2 * (1 + operationTableSize)] = 5; /* O3J2 @ M2 */
  operationMatrix[7 + 3 * (1 + operationTableSize)] = 8; /* O3J2 @ M3 */
  operationMatrix[7 + 4 * (1 + operationTableSize)] = 9; /* O3J2 @ M4 */
  operationMatrix[7 + 5 * (1 + operationTableSize)] = 4; /* O3J2 @ M5 */
  operationMatrix[7 + 6 * (1 + operationTableSize)] = 3; /* O3J2 @ M6 */
  operationMatrix[7 + 7 * (1 + operationTableSize)] = 5; /* O3J2 @ M7 */
  operationMatrix[7 + 8 * (1 + operationTableSize)] = 3; /* O3J2 @ M8 */
  operationMatrix[7 + 9 * (1 + operationTableSize)] = 8; /* O3J2 @ M9 */
  operationMatrix[7 + 10 * (1 + operationTableSize)] = 1; /* O3J2 @ M10 */

  operationMatrix[8 + 1 * (1 + operationTableSize)] = 9; /* O4J2 @ M1 */
  operationMatrix[8 + 2 * (1 + operationTableSize)] = 3; /* O4J2 @ M2 */
  operationMatrix[8 + 3 * (1 + operationTableSize)] = 6; /* O4J2 @ M3 */
  operationMatrix[8 + 4 * (1 + operationTableSize)] = 1; /* O4J2 @ M4 */
  operationMatrix[8 + 5 * (1 + operationTableSize)] = 2; /* O4J2 @ M5 */
  operationMatrix[8 + 6 * (1 + operationTableSize)] = 6; /* O4J2 @ M6 */
  operationMatrix[8 + 7 * (1 + operationTableSize)] = 4; /* O4J2 @ M7 */
  operationMatrix[8 + 8 * (1 + operationTableSize)] = 1; /* O4J2 @ M8 */
  operationMatrix[8 + 9 * (1 + operationTableSize)] = 7; /* O4J2 @ M9 */
  operationMatrix[8 + 10 * (1 + operationTableSize)] = 2; /* O4J2 @ M10 */

  operationMatrix[9 + 1 * (1 + operationTableSize)] = 7; /* O1J3 @ M1 */
  operationMatrix[9 + 2 * (1 + operationTableSize)] = 1; /* O1J3 @ M2 */
  operationMatrix[9 + 3 * (1 + operationTableSize)] = 8; /* O1J3 @ M3 */
  operationMatrix[9 + 4 * (1 + operationTableSize)] = 5; /* O1J3 @ M4 */
  operationMatrix[9 + 5 * (1 + operationTableSize)] = 4; /* O1J3 @ M5 */
  operationMatrix[9 + 6 * (1 + operationTableSize)] = 9; /* O1J3 @ M6 */
  operationMatrix[9 + 7 * (1 + operationTableSize)] = 1; /* O1J3 @ M7 */
  operationMatrix[9 + 8 * (1 + operationTableSize)] = 2; /* O1J3 @ M8 */
  operationMatrix[9 + 9 * (1 + operationTableSize)] = 3; /* O1J3 @ M9 */
  operationMatrix[9 + 10 * (1 + operationTableSize)] = 4; /* O1J3 @ M10 */

  operationMatrix[10 + 1 * (1 + operationTableSize)] = 5; /* O2J3 @ M1 */
  operationMatrix[10 + 2 * (1 + operationTableSize)] = 10; /* O2J3 @ M2 */
  operationMatrix[10 + 3 * (1 + operationTableSize)] = 6; /* O2J3 @ M3 */
  operationMatrix[10 + 4 * (1 + operationTableSize)] = 4; /* O2J3 @ M4 */
  operationMatrix[10 + 5 * (1 + operationTableSize)] = 9; /* O2J3 @ M5 */
  operationMatrix[10 + 6 * (1 + operationTableSize)] = 5; /* O2J3 @ M6 */
  operationMatrix[10 + 7 * (1 + operationTableSize)] = 1; /* O2J3 @ M7 */
  operationMatrix[10 + 8 * (1 + operationTableSize)] = 7; /* O2J3 @ M8 */
  operationMatrix[10 + 9 * (1 + operationTableSize)] = 1; /* O2J3 @ M9 */
  operationMatrix[10 + 10 * (1 + operationTableSize)] = 6; /* O2J3 @ M10 */

  operationMatrix[11 + 1 * (1 + operationTableSize)] = 4; /* O3J3 @ M1 */
  operationMatrix[11 + 2 * (1 + operationTableSize)] = 2; /* O3J3 @ M2 */
  operationMatrix[11 + 3 * (1 + operationTableSize)] = 3; /* O3J3 @ M3 */
  operationMatrix[11 + 4 * (1 + operationTableSize)] = 8; /* O3J3 @ M4 */
  operationMatrix[11 + 5 * (1 + operationTableSize)] = 7; /* O3J3 @ M5 */
  operationMatrix[11 + 6 * (1 + operationTableSize)] = 4; /* O3J3 @ M6 */
  operationMatrix[11 + 7 * (1 + operationTableSize)] = 6; /* O3J3 @ M7 */
  operationMatrix[11 + 8 * (1 + operationTableSize)] = 9; /* O3J3 @ M8 */
  operationMatrix[11 + 9 * (1 + operationTableSize)] = 8; /* O3J3 @ M9 */
  operationMatrix[11 + 10 * (1 + operationTableSize)] = 4; /* O3J3 @ M10 */

  operationMatrix[12 + 1 * (1 + operationTableSize)] = 7; /* O4J3 @ M1 */
  operationMatrix[12 + 2 * (1 + operationTableSize)] = 3; /* O4J3 @ M2 */
  operationMatrix[12 + 3 * (1 + operationTableSize)] = 12; /* O4J3 @ M3 */
  operationMatrix[12 + 4 * (1 + operationTableSize)] = 1; /* O4J3 @ M4 */
  operationMatrix[12 + 5 * (1 + operationTableSize)] = 6; /* O4J3 @ M5 */
  operationMatrix[12 + 6 * (1 + operationTableSize)] = 5; /* O4J3 @ M6 */
  operationMatrix[12 + 7 * (1 + operationTableSize)] = 8; /* O4J3 @ M7 */
  operationMatrix[12 + 8 * (1 + operationTableSize)] = 3; /* O4J3 @ M8 */
  operationMatrix[12 + 9 * (1 + operationTableSize)] = 5; /* O4J3 @ M9 */
  operationMatrix[12 + 10 * (1 + operationTableSize)] = 2; /* O4J3 @ M10 */

  operationMatrix[13 + 1 * (1 + operationTableSize)] = 6; /* O1J4 @ M1 */
  operationMatrix[13 + 2 * (1 + operationTableSize)] = 2; /* O1J4 @ M2 */
  operationMatrix[13 + 3 * (1 + operationTableSize)] = 5; /* O1J4 @ M3 */
  operationMatrix[13 + 4 * (1 + operationTableSize)] = 4; /* O1J4 @ M4 */
  operationMatrix[13 + 5 * (1 + operationTableSize)] = 1; /* O1J4 @ M5 */
  operationMatrix[13 + 6 * (1 + operationTableSize)] = 2; /* O1J4 @ M6 */
  operationMatrix[13 + 7 * (1 + operationTableSize)] = 3; /* O1J4 @ M7 */
  operationMatrix[13 + 8 * (1 + operationTableSize)] = 6; /* O1J4 @ M8 */
  operationMatrix[13 + 9 * (1 + operationTableSize)] = 5; /* O1J4 @ M9 */
  operationMatrix[13 + 10 * (1 + operationTableSize)] = 4; /* O1J4 @ M10 */

  operationMatrix[14 + 1 * (1 + operationTableSize)] = 8; /* O2J4 @ M1 */
  operationMatrix[14 + 2 * (1 + operationTableSize)] = 5; /* O2J4 @ M2 */
  operationMatrix[14 + 3 * (1 + operationTableSize)] = 7; /* O2J4 @ M3 */
  operationMatrix[14 + 4 * (1 + operationTableSize)] = 4; /* O2J4 @ M4 */
  operationMatrix[14 + 5 * (1 + operationTableSize)] = 1; /* O2J4 @ M5 */
  operationMatrix[14 + 6 * (1 + operationTableSize)] = 2; /* O2J4 @ M6 */
  operationMatrix[14 + 7 * (1 + operationTableSize)] = 36; /* O2J4 @ M7 */
  operationMatrix[14 + 8 * (1 + operationTableSize)] = 5; /* O2J4 @ M8 */
  operationMatrix[14 + 9 * (1 + operationTableSize)] = 8; /* O2J4 @ M9 */
  operationMatrix[14 + 10 * (1 + operationTableSize)] = 5; /* O2J4 @ M10 */

  operationMatrix[15 + 1 * (1 + operationTableSize)] = 9; /* O3J4 @ M1 */
  operationMatrix[15 + 2 * (1 + operationTableSize)] = 6; /* O3J4 @ M2 */
  operationMatrix[15 + 3 * (1 + operationTableSize)] = 2; /* O3J4 @ M3 */
  operationMatrix[15 + 4 * (1 + operationTableSize)] = 4; /* O3J4 @ M4 */
  operationMatrix[15 + 5 * (1 + operationTableSize)] = 5; /* O3J4 @ M5 */
  operationMatrix[15 + 6 * (1 + operationTableSize)] = 1; /* O3J4 @ M6 */
  operationMatrix[15 + 7 * (1 + operationTableSize)] = 3; /* O3J4 @ M7 */
  operationMatrix[15 + 8 * (1 + operationTableSize)] = 6; /* O3J4 @ M8 */
  operationMatrix[15 + 9 * (1 + operationTableSize)] = 5; /* O3J4 @ M9 */
  operationMatrix[15 + 10 * (1 + operationTableSize)] = 2; /* O3J4 @ M10 */

  operationMatrix[16 + 1 * (1 + operationTableSize)] = 11; /* O4J4 @ M1 */
  operationMatrix[16 + 2 * (1 + operationTableSize)] = 4; /* O4J4 @ M2 */
  operationMatrix[16 + 3 * (1 + operationTableSize)] = 5; /* O4J4 @ M3 */
  operationMatrix[16 + 4 * (1 + operationTableSize)] = 6; /* O4J4 @ M4 */
  operationMatrix[16 + 5 * (1 + operationTableSize)] = 2; /* O4J4 @ M5 */
  operationMatrix[16 + 6 * (1 + operationTableSize)] = 7; /* O4J4 @ M6 */
  operationMatrix[16 + 7 * (1 + operationTableSize)] = 5; /* O4J4 @ M7 */
  operationMatrix[16 + 8 * (1 + operationTableSize)] = 4; /* O4J4 @ M8 */
  operationMatrix[16 + 9 * (1 + operationTableSize)] = 2; /* O4J4 @ M9 */
  operationMatrix[16 + 10 * (1 + operationTableSize)] = 1; /* O4J4 @ M10 */

  operationMatrix[17 + 1 * (1 + operationTableSize)] = 6; /* O1J5 @ M1 */
  operationMatrix[17 + 2 * (1 + operationTableSize)] = 9; /* O1J5 @ M2 */
  operationMatrix[17 + 3 * (1 + operationTableSize)] = 2; /* O1J5 @ M3 */
  operationMatrix[17 + 4 * (1 + operationTableSize)] = 3; /* O1J5 @ M4 */
  operationMatrix[17 + 5 * (1 + operationTableSize)] = 5; /* O1J5 @ M5 */
  operationMatrix[17 + 6 * (1 + operationTableSize)] = 8; /* O1J5 @ M6 */
  operationMatrix[17 + 7 * (1 + operationTableSize)] = 7; /* O1J5 @ M7 */
  operationMatrix[17 + 8 * (1 + operationTableSize)] = 4; /* O1J5 @ M8 */
  operationMatrix[17 + 9 * (1 + operationTableSize)] = 1; /* O1J5 @ M9 */
  operationMatrix[17 + 10 * (1 + operationTableSize)] = 2; /* O1J5 @ M10 */

  operationMatrix[18 + 1 * (1 + operationTableSize)] = 5; /* O2J5 @ M1 */
  operationMatrix[18 + 2 * (1 + operationTableSize)] = 4; /* O2J5 @ M2 */
  operationMatrix[18 + 3 * (1 + operationTableSize)] = 6; /* O2J5 @ M3 */
  operationMatrix[18 + 4 * (1 + operationTableSize)] = 3; /* O2J5 @ M4 */
  operationMatrix[18 + 5 * (1 + operationTableSize)] = 5; /* O2J5 @ M5 */
  operationMatrix[18 + 6 * (1 + operationTableSize)] = 2; /* O2J5 @ M6 */
  operationMatrix[18 + 7 * (1 + operationTableSize)] = 28; /* O2J5 @ M7 */
  operationMatrix[18 + 8 * (1 + operationTableSize)] = 7; /* O2J5 @ M8 */
  operationMatrix[18 + 9 * (1 + operationTableSize)] = 4; /* O2J5 @ M9 */
  operationMatrix[18 + 10 * (1 + operationTableSize)] = 5; /* O2J5 @ M10 */

  operationMatrix[19 + 1 * (1 + operationTableSize)] = 6; /* O3J5 @ M1 */
  operationMatrix[19 + 2 * (1 + operationTableSize)] = 2; /* O3J5 @ M2 */
  operationMatrix[19 + 3 * (1 + operationTableSize)] = 4; /* O3J5 @ M3 */
  operationMatrix[19 + 4 * (1 + operationTableSize)] = 3; /* O3J5 @ M4 */
  operationMatrix[19 + 5 * (1 + operationTableSize)] = 6; /* O3J5 @ M5 */
  operationMatrix[19 + 6 * (1 + operationTableSize)] = 5; /* O3J5 @ M6 */
  operationMatrix[19 + 7 * (1 + operationTableSize)] = 2; /* O3J5 @ M7 */
  operationMatrix[19 + 8 * (1 + operationTableSize)] = 4; /* O3J5 @ M8 */
  operationMatrix[19 + 9 * (1 + operationTableSize)] = 7; /* O3J5 @ M9 */
  operationMatrix[19 + 10 * (1 + operationTableSize)] = 9; /* O3J5 @ M10 */

  operationMatrix[20 + 1 * (1 + operationTableSize)] = 6; /* O4J5 @ M1 */
  operationMatrix[20 + 2 * (1 + operationTableSize)] = 5; /* O4J5 @ M2 */
  operationMatrix[20 + 3 * (1 + operationTableSize)] = 4; /* O4J5 @ M3 */
  operationMatrix[20 + 4 * (1 + operationTableSize)] = 2; /* O4J5 @ M4 */
  operationMatrix[20 + 5 * (1 + operationTableSize)] = 3; /* O4J5 @ M5 */
  operationMatrix[20 + 6 * (1 + operationTableSize)] = 2; /* O4J5 @ M6 */
  operationMatrix[20 + 7 * (1 + operationTableSize)] = 5; /* O4J5 @ M7 */
  operationMatrix[20 + 8 * (1 + operationTableSize)] = 4; /* O4J5 @ M8 */
  operationMatrix[20 + 9 * (1 + operationTableSize)] = 7; /* O4J5 @ M9 */
  operationMatrix[20 + 10 * (1 + operationTableSize)] = 5; /* O4J5 @ M10 */

  operationMatrix[21 + 1 * (1 + operationTableSize)] = 4; /* O1J6 @ M1 */
  operationMatrix[21 + 2 * (1 + operationTableSize)] = 1; /* O1J6 @ M2 */
  operationMatrix[21 + 3 * (1 + operationTableSize)] = 3; /* O1J6 @ M3 */
  operationMatrix[21 + 4 * (1 + operationTableSize)] = 2; /* O1J6 @ M4 */
  operationMatrix[21 + 5 * (1 + operationTableSize)] = 6; /* O1J6 @ M5 */
  operationMatrix[21 + 6 * (1 + operationTableSize)] = 9; /* O1J6 @ M6 */
  operationMatrix[21 + 7 * (1 + operationTableSize)] = 8; /* O1J6 @ M7 */
  operationMatrix[21 + 8 * (1 + operationTableSize)] = 5; /* O1J6 @ M8 */
  operationMatrix[21 + 9 * (1 + operationTableSize)] = 4; /* O1J6 @ M9 */
  operationMatrix[21 + 10 * (1 + operationTableSize)] = 2; /* O1J6 @ M10 */

  operationMatrix[22 + 1 * (1 + operationTableSize)] = 1; /* O2J6 @ M1 */
  operationMatrix[22 + 2 * (1 + operationTableSize)] = 3; /* O2J6 @ M2 */
  operationMatrix[22 + 3 * (1 + operationTableSize)] = 6; /* O2J6 @ M3 */
  operationMatrix[22 + 4 * (1 + operationTableSize)] = 5; /* O2J6 @ M4 */
  operationMatrix[22 + 5 * (1 + operationTableSize)] = 4; /* O2J6 @ M5 */
  operationMatrix[22 + 6 * (1 + operationTableSize)] = 7; /* O2J6 @ M6 */
  operationMatrix[22 + 7 * (1 + operationTableSize)] = 5; /* O2J6 @ M7 */
  operationMatrix[22 + 8 * (1 + operationTableSize)] = 4; /* O2J6 @ M8 */
  operationMatrix[22 + 9 * (1 + operationTableSize)] = 6; /* O2J6 @ M9 */
  operationMatrix[22 + 10 * (1 + operationTableSize)] = 5; /* O2J6 @ M10 */

  operationMatrix[23 + 1 * (1 + operationTableSize)] = 1; /* O1J7 @ M1 */
  operationMatrix[23 + 2 * (1 + operationTableSize)] = 4; /* O1J7 @ M2 */
  operationMatrix[23 + 3 * (1 + operationTableSize)] = 2; /* O1J7 @ M3 */
  operationMatrix[23 + 4 * (1 + operationTableSize)] = 5; /* O1J7 @ M4 */
  operationMatrix[23 + 5 * (1 + operationTableSize)] = 3; /* O1J7 @ M5 */
  operationMatrix[23 + 6 * (1 + operationTableSize)] = 6; /* O1J7 @ M6 */
  operationMatrix[23 + 7 * (1 + operationTableSize)] = 9; /* O1J7 @ M7 */
  operationMatrix[23 + 8 * (1 + operationTableSize)] = 8; /* O1J7 @ M8 */
  operationMatrix[23 + 9 * (1 + operationTableSize)] = 5; /* O1J7 @ M9 */
  operationMatrix[23 + 10 * (1 + operationTableSize)] = 4; /* O1J7 @ M10 */

  operationMatrix[24 + 1 * (1 + operationTableSize)] = 2; /* O2J7 @ M1 */
  operationMatrix[24 + 2 * (1 + operationTableSize)] = 1; /* O2J7 @ M2 */
  operationMatrix[24 + 3 * (1 + operationTableSize)] = 4; /* O2J7 @ M3 */
  operationMatrix[24 + 4 * (1 + operationTableSize)] = 5; /* O2J7 @ M4 */
  operationMatrix[24 + 5 * (1 + operationTableSize)] = 2; /* O2J7 @ M5 */
  operationMatrix[24 + 6 * (1 + operationTableSize)] = 3; /* O2J7 @ M6 */
  operationMatrix[24 + 7 * (1 + operationTableSize)] = 5; /* O2J7 @ M7 */
  operationMatrix[24 + 8 * (1 + operationTableSize)] = 4; /* O2J7 @ M8 */
  operationMatrix[24 + 9 * (1 + operationTableSize)] = 2; /* O2J7 @ M9 */
  operationMatrix[24 + 10 * (1 + operationTableSize)] = 5; /* O2J7 @ M10 */

  operationMatrix[25 + 1 * (1 + operationTableSize)] = 2; /* O1J8 @ M1 */
  operationMatrix[25 + 2 * (1 + operationTableSize)] = 3; /* O1J8 @ M2 */
  operationMatrix[25 + 3 * (1 + operationTableSize)] = 6; /* O1J8 @ M3 */
  operationMatrix[25 + 4 * (1 + operationTableSize)] = 2; /* O1J8 @ M4 */
  operationMatrix[25 + 5 * (1 + operationTableSize)] = 5; /* O1J8 @ M5 */
  operationMatrix[25 + 6 * (1 + operationTableSize)] = 4; /* O1J8 @ M6 */
  operationMatrix[25 + 7 * (1 + operationTableSize)] = 1; /* O1J8 @ M7 */
  operationMatrix[25 + 8 * (1 + operationTableSize)] = 5; /* O1J8 @ M8 */
  operationMatrix[25 + 9 * (1 + operationTableSize)] = 8; /* O1J8 @ M9 */
  operationMatrix[25 + 10 * (1 + operationTableSize)] = 7; /* O1J8 @ M10 */

  operationMatrix[26 + 1 * (1 + operationTableSize)] = 4; /* O2J8 @ M1 */
  operationMatrix[26 + 2 * (1 + operationTableSize)] = 5; /* O2J8 @ M2 */
  operationMatrix[26 + 3 * (1 + operationTableSize)] = 6; /* O2J8 @ M3 */
  operationMatrix[26 + 4 * (1 + operationTableSize)] = 2; /* O2J8 @ M4 */
  operationMatrix[26 + 5 * (1 + operationTableSize)] = 3; /* O2J8 @ M5 */
  operationMatrix[26 + 6 * (1 + operationTableSize)] = 5; /* O2J8 @ M6 */
  operationMatrix[26 + 7 * (1 + operationTableSize)] = 4; /* O2J8 @ M7 */
  operationMatrix[26 + 8 * (1 + operationTableSize)] = 1; /* O2J8 @ M8 */
  operationMatrix[26 + 9 * (1 + operationTableSize)] = 2; /* O2J8 @ M9 */
  operationMatrix[26 + 10 * (1 + operationTableSize)] = 5; /* O2J8 @ M10 */

  operationMatrix[27 + 1 * (1 + operationTableSize)] = 3; /* O3J8 @ M1 */
  operationMatrix[27 + 2 * (1 + operationTableSize)] = 5; /* O3J8 @ M2 */
  operationMatrix[27 + 3 * (1 + operationTableSize)] = 4; /* O3J8 @ M3 */
  operationMatrix[27 + 4 * (1 + operationTableSize)] = 2; /* O3J8 @ M4 */
  operationMatrix[27 + 5 * (1 + operationTableSize)] = 5; /* O3J8 @ M5 */
  operationMatrix[27 + 6 * (1 + operationTableSize)] = 49; /* O3J8 @ M6 */
  operationMatrix[27 + 7 * (1 + operationTableSize)] = 8; /* O3J8 @ M7 */
  operationMatrix[27 + 8 * (1 + operationTableSize)] = 5; /* O3J8 @ M8 */
  operationMatrix[27 + 9 * (1 + operationTableSize)] = 4; /* O3J8 @ M9 */
  operationMatrix[27 + 10 * (1 + operationTableSize)] = 5; /* O3J8 @ M10 */

  operationMatrix[28 + 1 * (1 + operationTableSize)] = 1; /* O4J8 @ M1 */
  operationMatrix[28 + 2 * (1 + operationTableSize)] = 2; /* O4J8 @ M2 */
  operationMatrix[28 + 3 * (1 + operationTableSize)] = 36; /* O4J8 @ M3 */
  operationMatrix[28 + 4 * (1 + operationTableSize)] = 5; /* O4J8 @ M4 */
  operationMatrix[28 + 5 * (1 + operationTableSize)] = 2; /* O4J8 @ M5 */
  operationMatrix[28 + 6 * (1 + operationTableSize)] = 3; /* O4J8 @ M6 */
  operationMatrix[28 + 7 * (1 + operationTableSize)] = 6; /* O4J8 @ M7 */
  operationMatrix[28 + 8 * (1 + operationTableSize)] = 4; /* O4J8 @ M8 */
  operationMatrix[28 + 9 * (1 + operationTableSize)] = 11; /* O4J8 @ M9 */
  operationMatrix[28 + 10 * (1 + operationTableSize)] = 2; /* O4J8 @ M10 */

  operationMatrix[29 + 1 * (1 + operationTableSize)] = 6; /* O1J9 @ M1 */
  operationMatrix[29 + 2 * (1 + operationTableSize)] = 3; /* O1J9 @ M2 */
  operationMatrix[29 + 3 * (1 + operationTableSize)] = 2; /* O1J9 @ M3 */
  operationMatrix[29 + 4 * (1 + operationTableSize)] = 22; /* O1J9 @ M4 */
  operationMatrix[29 + 5 * (1 + operationTableSize)] = 44; /* O1J9 @ M5 */
  operationMatrix[29 + 6 * (1 + operationTableSize)] = 11; /* O1J9 @ M6 */
  operationMatrix[29 + 7 * (1 + operationTableSize)] = 10; /* O1J9 @ M7 */
  operationMatrix[29 + 8 * (1 + operationTableSize)] = 23; /* O1J9 @ M8 */
  operationMatrix[29 + 9 * (1 + operationTableSize)] = 5; /* O1J9 @ M9 */
  operationMatrix[29 + 10 * (1 + operationTableSize)] = 1; /* O1J9 @ M10 */

  operationMatrix[30 + 1 * (1 + operationTableSize)] = 2; /* O2J9 @ M1 */
  operationMatrix[30 + 2 * (1 + operationTableSize)] = 3; /* O2J9 @ M2 */
  operationMatrix[30 + 3 * (1 + operationTableSize)] = 2; /* O2J9 @ M3 */
  operationMatrix[30 + 4 * (1 + operationTableSize)] = 12; /* O2J9 @ M4 */
  operationMatrix[30 + 5 * (1 + operationTableSize)] = 15; /* O2J9 @ M5 */
  operationMatrix[30 + 6 * (1 + operationTableSize)] = 10; /* O2J9 @ M6 */
  operationMatrix[30 + 7 * (1 + operationTableSize)] = 12; /* O2J9 @ M7 */
  operationMatrix[30 + 8 * (1 + operationTableSize)] = 14; /* O2J9 @ M8 */
  operationMatrix[30 + 9 * (1 + operationTableSize)] = 18; /* O2J9 @ M9 */
  operationMatrix[30 + 10 * (1 + operationTableSize)] = 16; /* O2J9 @ M10 */

  operationMatrix[31 + 1 * (1 + operationTableSize)] = 20; /* O3J9 @ M1 */
  operationMatrix[31 + 2 * (1 + operationTableSize)] = 17; /* O3J9 @ M2 */
  operationMatrix[31 + 3 * (1 + operationTableSize)] = 12; /* O3J9 @ M3 */
  operationMatrix[31 + 4 * (1 + operationTableSize)] = 5; /* O3J9 @ M4 */
  operationMatrix[31 + 5 * (1 + operationTableSize)] = 9; /* O3J9 @ M5 */
  operationMatrix[31 + 6 * (1 + operationTableSize)] = 6; /* O3J9 @ M6 */
  operationMatrix[31 + 7 * (1 + operationTableSize)] = 4; /* O3J9 @ M7 */
  operationMatrix[31 + 8 * (1 + operationTableSize)] = 7; /* O3J9 @ M8 */
  operationMatrix[31 + 9 * (1 + operationTableSize)] = 5; /* O3J9 @ M9 */
  operationMatrix[31 + 10 * (1 + operationTableSize)] = 6; /* O3J9 @ M10 */

  operationMatrix[32 + 1 * (1 + operationTableSize)] = 9; /* O4J9 @ M1 */
  operationMatrix[32 + 2 * (1 + operationTableSize)] = 8; /* O4J9 @ M2 */
  operationMatrix[32 + 3 * (1 + operationTableSize)] = 7; /* O4J9 @ M3 */
  operationMatrix[32 + 4 * (1 + operationTableSize)] = 4; /* O4J9 @ M4 */
  operationMatrix[32 + 5 * (1 + operationTableSize)] = 5; /* O4J9 @ M5 */
  operationMatrix[32 + 6 * (1 + operationTableSize)] = 8; /* O4J9 @ M6 */
  operationMatrix[32 + 7 * (1 + operationTableSize)] = 7; /* O4J9 @ M7 */
  operationMatrix[32 + 8 * (1 + operationTableSize)] = 4; /* O4J9 @ M8 */
  operationMatrix[32 + 9 * (1 + operationTableSize)] = 56; /* O4J9 @ M9 */
  operationMatrix[32 + 10 * (1 + operationTableSize)] = 2; /* O4J9 @ M10 */

  operationMatrix[33 + 1 * (1 + operationTableSize)] = 5; /* O1J10 @ M1 */
  operationMatrix[33 + 2 * (1 + operationTableSize)] = 8; /* O1J10 @ M2 */
  operationMatrix[33 + 3 * (1 + operationTableSize)] = 7; /* O1J10 @ M3 */
  operationMatrix[33 + 4 * (1 + operationTableSize)] = 4; /* O1J10 @ M4 */
  operationMatrix[33 + 5 * (1 + operationTableSize)] = 56; /* O1J10 @ M5 */
  operationMatrix[33 + 6 * (1 + operationTableSize)] = 3; /* O1J10 @ M6 */
  operationMatrix[33 + 7 * (1 + operationTableSize)] = 2; /* O1J10 @ M7 */
  operationMatrix[33 + 8 * (1 + operationTableSize)] = 5; /* O1J10 @ M8 */
  operationMatrix[33 + 9 * (1 + operationTableSize)] = 4; /* O1J10 @ M9 */
  operationMatrix[33 + 10 * (1 + operationTableSize)] = 1; /* O1J10 @ M10 */

  operationMatrix[34 + 1 * (1 + operationTableSize)] = 2; /* O2J10 @ M1 */
  operationMatrix[34 + 2 * (1 + operationTableSize)] = 5; /* O2J10 @ M2 */
  operationMatrix[34 + 3 * (1 + operationTableSize)] = 6; /* O2J10 @ M3 */
  operationMatrix[34 + 4 * (1 + operationTableSize)] = 9; /* O2J10 @ M4 */
  operationMatrix[34 + 5 * (1 + operationTableSize)] = 8; /* O2J10 @ M5 */
  operationMatrix[34 + 6 * (1 + operationTableSize)] = 5; /* O2J10 @ M6 */
  operationMatrix[34 + 7 * (1 + operationTableSize)] = 4; /* O2J10 @ M7 */
  operationMatrix[34 + 8 * (1 + operationTableSize)] = 2; /* O2J10 @ M8 */
  operationMatrix[34 + 9 * (1 + operationTableSize)] = 5; /* O2J10 @ M9 */
  operationMatrix[34 + 10 * (1 + operationTableSize)] = 4; /* O2J10 @ M10 */

  operationMatrix[35 + 1 * (1 + operationTableSize)] = 6; /* O3J10 @ M1 */
  operationMatrix[35 + 2 * (1 + operationTableSize)] = 3; /* O3J10 @ M2 */
  operationMatrix[35 + 3 * (1 + operationTableSize)] = 2; /* O3J10 @ M3 */
  operationMatrix[35 + 4 * (1 + operationTableSize)] = 5; /* O3J10 @ M4 */
  operationMatrix[35 + 5 * (1 + operationTableSize)] = 4; /* O3J10 @ M5 */
  operationMatrix[35 + 6 * (1 + operationTableSize)] = 7; /* O3J10 @ M6 */
  operationMatrix[35 + 7 * (1 + operationTableSize)] = 4; /* O3J10 @ M7 */
  operationMatrix[35 + 8 * (1 + operationTableSize)] = 5; /* O3J10 @ M8 */
  operationMatrix[35 + 9 * (1 + operationTableSize)] = 2; /* O3J10 @ M9 */
  operationMatrix[35 + 10 * (1 + operationTableSize)] = 1; /* O3J10 @ M10 */

  operationMatrix[36 + 1 * (1 + operationTableSize)] = 3; /* O4J10 @ M1 */
  operationMatrix[36 + 2 * (1 + operationTableSize)] = 2; /* O4J10 @ M2 */
  operationMatrix[36 + 3 * (1 + operationTableSize)] = 5; /* O4J10 @ M3 */
  operationMatrix[36 + 4 * (1 + operationTableSize)] = 6; /* O4J10 @ M4 */
  operationMatrix[36 + 5 * (1 + operationTableSize)] = 5; /* O4J10 @ M5 */
  operationMatrix[36 + 6 * (1 + operationTableSize)] = 8; /* O4J10 @ M6 */
  operationMatrix[36 + 7 * (1 + operationTableSize)] = 7; /* O4J10 @ M7 */
  operationMatrix[36 + 8 * (1 + operationTableSize)] = 4; /* O4J10 @ M8 */
  operationMatrix[36 + 9 * (1 + operationTableSize)] = 5; /* O4J10 @ M9 */
  operationMatrix[36 + 10 * (1 + operationTableSize)] = 2; /* O4J10 @ M10 */

  operationMatrix[37 + 1 * (1 + operationTableSize)] = 1; /* O1J11 @ M1 */
  operationMatrix[37 + 2 * (1 + operationTableSize)] = 2; /* O1J11 @ M2 */
  operationMatrix[37 + 3 * (1 + operationTableSize)] = 3; /* O1J11 @ M3 */
  operationMatrix[37 + 4 * (1 + operationTableSize)] = 6; /* O1J11 @ M4 */
  operationMatrix[37 + 5 * (1 + operationTableSize)] = 5; /* O1J11 @ M5 */
  operationMatrix[37 + 6 * (1 + operationTableSize)] = 2; /* O1J11 @ M6 */
  operationMatrix[37 + 7 * (1 + operationTableSize)] = 1; /* O1J11 @ M7 */
  operationMatrix[37 + 8 * (1 + operationTableSize)] = 4; /* O1J11 @ M8 */
  operationMatrix[37 + 9 * (1 + operationTableSize)] = 2; /* O1J11 @ M9 */
  operationMatrix[37 + 10 * (1 + operationTableSize)] = 1; /* O1J11 @ M10 */

  operationMatrix[38 + 1 * (1 + operationTableSize)] = 2; /* O2J11 @ M1 */
  operationMatrix[38 + 2 * (1 + operationTableSize)] = 3; /* O2J11 @ M2 */
  operationMatrix[38 + 3 * (1 + operationTableSize)] = 6; /* O2J11 @ M3 */
  operationMatrix[38 + 4 * (1 + operationTableSize)] = 3; /* O2J11 @ M4 */
  operationMatrix[38 + 5 * (1 + operationTableSize)] = 2; /* O2J11 @ M5 */
  operationMatrix[38 + 6 * (1 + operationTableSize)] = 1; /* O2J11 @ M6 */
  operationMatrix[38 + 7 * (1 + operationTableSize)] = 4; /* O2J11 @ M7 */
  operationMatrix[38 + 8 * (1 + operationTableSize)] = 10; /* O2J11 @ M8 */
  operationMatrix[38 + 9 * (1 + operationTableSize)] = 12; /* O2J11 @ M9 */
  operationMatrix[38 + 10 * (1 + operationTableSize)] = 1; /* O2J11 @ M10 */

  operationMatrix[39 + 1 * (1 + operationTableSize)] = 3; /* O3J11 @ M1 */
  operationMatrix[39 + 2 * (1 + operationTableSize)] = 6; /* O3J11 @ M2 */
  operationMatrix[39 + 3 * (1 + operationTableSize)] = 2; /* O3J11 @ M3 */
  operationMatrix[39 + 4 * (1 + operationTableSize)] = 5; /* O3J11 @ M4 */
  operationMatrix[39 + 5 * (1 + operationTableSize)] = 8; /* O3J11 @ M5 */
  operationMatrix[39 + 6 * (1 + operationTableSize)] = 4; /* O3J11 @ M6 */
  operationMatrix[39 + 7 * (1 + operationTableSize)] = 6; /* O3J11 @ M7 */
  operationMatrix[39 + 8 * (1 + operationTableSize)] = 3; /* O3J11 @ M8 */
  operationMatrix[39 + 9 * (1 + operationTableSize)] = 2; /* O3J11 @ M9 */
  operationMatrix[39 + 10 * (1 + operationTableSize)] = 5; /* O3J11 @ M10 */

  operationMatrix[40 + 1 * (1 + operationTableSize)] = 4; /* O4J11 @ M1 */
  operationMatrix[40 + 2 * (1 + operationTableSize)] = 1; /* O4J11 @ M2 */
  operationMatrix[40 + 3 * (1 + operationTableSize)] = 45; /* O4J11 @ M3 */
  operationMatrix[40 + 4 * (1 + operationTableSize)] = 6; /* O4J11 @ M4 */
  operationMatrix[40 + 5 * (1 + operationTableSize)] = 2; /* O4J11 @ M5 */
  operationMatrix[40 + 6 * (1 + operationTableSize)] = 4; /* O4J11 @ M6 */
  operationMatrix[40 + 7 * (1 + operationTableSize)] = 1; /* O4J11 @ M7 */
  operationMatrix[40 + 8 * (1 + operationTableSize)] = 25; /* O4J11 @ M8 */
  operationMatrix[40 + 9 * (1 + operationTableSize)] = 2; /* O4J11 @ M9 */
  operationMatrix[40 + 10 * (1 + operationTableSize)] = 4; /* O4J11 @ M10 */

  operationMatrix[41 + 1 * (1 + operationTableSize)] = 9; /* O1J12 @ M1 */
  operationMatrix[41 + 2 * (1 + operationTableSize)] = 8; /* O1J12 @ M2 */
  operationMatrix[41 + 3 * (1 + operationTableSize)] = 5; /* O1J12 @ M3 */
  operationMatrix[41 + 4 * (1 + operationTableSize)] = 6; /* O1J12 @ M4 */
  operationMatrix[41 + 5 * (1 + operationTableSize)] = 3; /* O1J12 @ M5 */
  operationMatrix[41 + 6 * (1 + operationTableSize)] = 6; /* O1J12 @ M6 */
  operationMatrix[41 + 7 * (1 + operationTableSize)] = 5; /* O1J12 @ M7 */
  operationMatrix[41 + 8 * (1 + operationTableSize)] = 2; /* O1J12 @ M8 */
  operationMatrix[41 + 9 * (1 + operationTableSize)] = 4; /* O1J12 @ M9 */
  operationMatrix[41 + 10 * (1 + operationTableSize)] = 2; /* O1J12 @ M10 */

  operationMatrix[42 + 1 * (1 + operationTableSize)] = 5; /* O2J12 @ M1 */
  operationMatrix[42 + 2 * (1 + operationTableSize)] = 8; /* O2J12 @ M2 */
  operationMatrix[42 + 3 * (1 + operationTableSize)] = 9; /* O2J12 @ M3 */
  operationMatrix[42 + 4 * (1 + operationTableSize)] = 5; /* O2J12 @ M4 */
  operationMatrix[42 + 5 * (1 + operationTableSize)] = 4; /* O2J12 @ M5 */
  operationMatrix[42 + 6 * (1 + operationTableSize)] = 75; /* O2J12 @ M6 */
  operationMatrix[42 + 7 * (1 + operationTableSize)] = 63; /* O2J12 @ M7 */
  operationMatrix[42 + 8 * (1 + operationTableSize)] = 6; /* O2J12 @ M8 */
  operationMatrix[42 + 9 * (1 + operationTableSize)] = 5; /* O2J12 @ M9 */
  operationMatrix[42 + 10 * (1 + operationTableSize)] = 21; /* O2J12 @ M10 */

  operationMatrix[43 + 1 * (1 + operationTableSize)] = 12; /* O3J12 @ M1 */
  operationMatrix[43 + 2 * (1 + operationTableSize)] = 5; /* O3J12 @ M2 */
  operationMatrix[43 + 3 * (1 + operationTableSize)] = 4; /* O3J12 @ M3 */
  operationMatrix[43 + 4 * (1 + operationTableSize)] = 6; /* O3J12 @ M4 */
  operationMatrix[43 + 5 * (1 + operationTableSize)] = 3; /* O3J12 @ M5 */
  operationMatrix[43 + 6 * (1 + operationTableSize)] = 2; /* O3J12 @ M6 */
  operationMatrix[43 + 7 * (1 + operationTableSize)] = 5; /* O3J12 @ M7 */
  operationMatrix[43 + 8 * (1 + operationTableSize)] = 4; /* O3J12 @ M8 */
  operationMatrix[43 + 9 * (1 + operationTableSize)] = 2; /* O3J12 @ M9 */
  operationMatrix[43 + 10 * (1 + operationTableSize)] = 5; /* O3J12 @ M10 */

  operationMatrix[44 + 1 * (1 + operationTableSize)] = 8; /* O4J12 @ M1 */
  operationMatrix[44 + 2 * (1 + operationTableSize)] = 7; /* O4J12 @ M2 */
  operationMatrix[44 + 3 * (1 + operationTableSize)] = 9; /* O4J12 @ M3 */
  operationMatrix[44 + 4 * (1 + operationTableSize)] = 5; /* O4J12 @ M4 */
  operationMatrix[44 + 5 * (1 + operationTableSize)] = 6; /* O4J12 @ M5 */
  operationMatrix[44 + 6 * (1 + operationTableSize)] = 3; /* O4J12 @ M6 */
  operationMatrix[44 + 7 * (1 + operationTableSize)] = 2; /* O4J12 @ M7 */
  operationMatrix[44 + 8 * (1 + operationTableSize)] = 5; /* O4J12 @ M8 */
  operationMatrix[44 + 9 * (1 + operationTableSize)] = 8; /* O4J12 @ M9 */
  operationMatrix[44 + 10 * (1 + operationTableSize)] = 4; /* O4J12 @ M10 */

  operationMatrix[45 + 1 * (1 + operationTableSize)] = 4; /* O1J13 @ M1 */
  operationMatrix[45 + 2 * (1 + operationTableSize)] = 2; /* O1J13 @ M2 */
  operationMatrix[45 + 3 * (1 + operationTableSize)] = 5; /* O1J13 @ M3 */
  operationMatrix[45 + 4 * (1 + operationTableSize)] = 6; /* O1J13 @ M4 */
  operationMatrix[45 + 5 * (1 + operationTableSize)] = 8; /* O1J13 @ M5 */
  operationMatrix[45 + 6 * (1 + operationTableSize)] = 5; /* O1J13 @ M6 */
  operationMatrix[45 + 7 * (1 + operationTableSize)] = 6; /* O1J13 @ M7 */
  operationMatrix[45 + 8 * (1 + operationTableSize)] = 4; /* O1J13 @ M8 */
  operationMatrix[45 + 9 * (1 + operationTableSize)] = 6; /* O1J13 @ M9 */
  operationMatrix[45 + 10 * (1 + operationTableSize)] = 2; /* O1J13 @ M10 */

  operationMatrix[46 + 1 * (1 + operationTableSize)] = 3; /* O2J13 @ M1 */
  operationMatrix[46 + 2 * (1 + operationTableSize)] = 5; /* O2J13 @ M2 */
  operationMatrix[46 + 3 * (1 + operationTableSize)] = 4; /* O2J13 @ M3 */
  operationMatrix[46 + 4 * (1 + operationTableSize)] = 7; /* O2J13 @ M4 */
  operationMatrix[46 + 5 * (1 + operationTableSize)] = 5; /* O2J13 @ M5 */
  operationMatrix[46 + 6 * (1 + operationTableSize)] = 8; /* O2J13 @ M6 */
  operationMatrix[46 + 7 * (1 + operationTableSize)] = 6; /* O2J13 @ M7 */
  operationMatrix[46 + 8 * (1 + operationTableSize)] = 6; /* O2J13 @ M8 */
  operationMatrix[46 + 9 * (1 + operationTableSize)] = 3; /* O2J13 @ M9 */
  operationMatrix[46 + 10 * (1 + operationTableSize)] = 2; /* O2J13 @ M10 */

  operationMatrix[47 + 1 * (1 + operationTableSize)] = 5; /* O3J13 @ M1 */
  operationMatrix[47 + 2 * (1 + operationTableSize)] = 4; /* O3J13 @ M2 */
  operationMatrix[47 + 3 * (1 + operationTableSize)] = 5; /* O3J13 @ M3 */
  operationMatrix[47 + 4 * (1 + operationTableSize)] = 8; /* O3J13 @ M4 */
  operationMatrix[47 + 5 * (1 + operationTableSize)] = 5; /* O3J13 @ M5 */
  operationMatrix[47 + 6 * (1 + operationTableSize)] = 4; /* O3J13 @ M6 */
  operationMatrix[47 + 7 * (1 + operationTableSize)] = 6; /* O3J13 @ M7 */
  operationMatrix[47 + 8 * (1 + operationTableSize)] = 5; /* O3J13 @ M8 */
  operationMatrix[47 + 9 * (1 + operationTableSize)] = 4; /* O3J13 @ M9 */
  operationMatrix[47 + 10 * (1 + operationTableSize)] = 2; /* O3J13 @ M10 */

  operationMatrix[48 + 1 * (1 + operationTableSize)] = 3; /* O4J13 @ M1 */
  operationMatrix[48 + 2 * (1 + operationTableSize)] = 2; /* O4J13 @ M2 */
  operationMatrix[48 + 3 * (1 + operationTableSize)] = 5; /* O4J13 @ M3 */
  operationMatrix[48 + 4 * (1 + operationTableSize)] = 6; /* O4J13 @ M4 */
  operationMatrix[48 + 5 * (1 + operationTableSize)] = 5; /* O4J13 @ M5 */
  operationMatrix[48 + 6 * (1 + operationTableSize)] = 4; /* O4J13 @ M6 */
  operationMatrix[48 + 7 * (1 + operationTableSize)] = 8; /* O4J13 @ M7 */
  operationMatrix[48 + 8 * (1 + operationTableSize)] = 5; /* O4J13 @ M8 */
  operationMatrix[48 + 9 * (1 + operationTableSize)] = 6; /* O4J13 @ M9 */
  operationMatrix[48 + 10 * (1 + operationTableSize)] = 4; /* O4J13 @ M10 */

  operationMatrix[49 + 1 * (1 + operationTableSize)] = 2; /* O1J14 @ M1 */
  operationMatrix[49 + 2 * (1 + operationTableSize)] = 3; /* O1J14 @ M2 */
  operationMatrix[49 + 3 * (1 + operationTableSize)] = 5; /* O1J14 @ M3 */
  operationMatrix[49 + 4 * (1 + operationTableSize)] = 4; /* O1J14 @ M4 */
  operationMatrix[49 + 5 * (1 + operationTableSize)] = 6; /* O1J14 @ M5 */
  operationMatrix[49 + 6 * (1 + operationTableSize)] = 5; /* O1J14 @ M6 */
  operationMatrix[49 + 7 * (1 + operationTableSize)] = 4; /* O1J14 @ M7 */
  operationMatrix[49 + 8 * (1 + operationTableSize)] = 85; /* O1J14 @ M8 */
  operationMatrix[49 + 9 * (1 + operationTableSize)] = 4; /* O1J14 @ M9 */
  operationMatrix[49 + 10 * (1 + operationTableSize)] = 5; /* O1J14 @ M10 */

  operationMatrix[50 + 1 * (1 + operationTableSize)] = 6; /* O2J14 @ M1 */
  operationMatrix[50 + 2 * (1 + operationTableSize)] = 2; /* O2J14 @ M2 */
  operationMatrix[50 + 3 * (1 + operationTableSize)] = 4; /* O2J14 @ M3 */
  operationMatrix[50 + 4 * (1 + operationTableSize)] = 5; /* O2J14 @ M4 */
  operationMatrix[50 + 5 * (1 + operationTableSize)] = 8; /* O2J14 @ M5 */
  operationMatrix[50 + 6 * (1 + operationTableSize)] = 6; /* O2J14 @ M6 */
  operationMatrix[50 + 7 * (1 + operationTableSize)] = 5; /* O2J14 @ M7 */
  operationMatrix[50 + 8 * (1 + operationTableSize)] = 4; /* O2J14 @ M8 */
  operationMatrix[50 + 9 * (1 + operationTableSize)] = 2; /* O2J14 @ M9 */
  operationMatrix[50 + 10 * (1 + operationTableSize)] = 6; /* O2J14 @ M10 */

  operationMatrix[51 + 1 * (1 + operationTableSize)] = 3; /* O3J14 @ M1 */
  operationMatrix[51 + 2 * (1 + operationTableSize)] = 25; /* O3J14 @ M2 */
  operationMatrix[51 + 3 * (1 + operationTableSize)] = 4; /* O3J14 @ M3 */
  operationMatrix[51 + 4 * (1 + operationTableSize)] = 8; /* O3J14 @ M4 */
  operationMatrix[51 + 5 * (1 + operationTableSize)] = 5; /* O3J14 @ M5 */
  operationMatrix[51 + 6 * (1 + operationTableSize)] = 6; /* O3J14 @ M6 */
  operationMatrix[51 + 7 * (1 + operationTableSize)] = 3; /* O3J14 @ M7 */
  operationMatrix[51 + 8 * (1 + operationTableSize)] = 2; /* O3J14 @ M8 */
  operationMatrix[51 + 9 * (1 + operationTableSize)] = 5; /* O3J14 @ M9 */
  operationMatrix[51 + 10 * (1 + operationTableSize)] = 4; /* O3J14 @ M10 */

  operationMatrix[52 + 1 * (1 + operationTableSize)] = 8; /* O4J14 @ M1 */
  operationMatrix[52 + 2 * (1 + operationTableSize)] = 5; /* O4J14 @ M2 */
  operationMatrix[52 + 3 * (1 + operationTableSize)] = 6; /* O4J14 @ M3 */
  operationMatrix[52 + 4 * (1 + operationTableSize)] = 4; /* O4J14 @ M4 */
  operationMatrix[52 + 5 * (1 + operationTableSize)] = 2; /* O4J14 @ M5 */
  operationMatrix[52 + 6 * (1 + operationTableSize)] = 3; /* O4J14 @ M6 */
  operationMatrix[52 + 7 * (1 + operationTableSize)] = 6; /* O4J14 @ M7 */
  operationMatrix[52 + 8 * (1 + operationTableSize)] = 8; /* O4J14 @ M8 */
  operationMatrix[52 + 9 * (1 + operationTableSize)] = 5; /* O4J14 @ M9 */
  operationMatrix[52 + 10 * (1 + operationTableSize)] = 4; /* O4J14 @ M10 */

  operationMatrix[53 + 1 * (1 + operationTableSize)] = 2; /* O1J15 @ M1 */
  operationMatrix[53 + 2 * (1 + operationTableSize)] = 5; /* O1J15 @ M2 */
  operationMatrix[53 + 3 * (1 + operationTableSize)] = 6; /* O1J15 @ M3 */
  operationMatrix[53 + 4 * (1 + operationTableSize)] = 8; /* O1J15 @ M4 */
  operationMatrix[53 + 5 * (1 + operationTableSize)] = 5; /* O1J15 @ M5 */
  operationMatrix[53 + 6 * (1 + operationTableSize)] = 6; /* O1J15 @ M6 */
  operationMatrix[53 + 7 * (1 + operationTableSize)] = 3; /* O1J15 @ M7 */
  operationMatrix[53 + 8 * (1 + operationTableSize)] = 2; /* O1J15 @ M8 */
  operationMatrix[53 + 9 * (1 + operationTableSize)] = 5; /* O1J15 @ M9 */
  operationMatrix[53 + 10 * (1 + operationTableSize)] = 4; /* O1J15 @ M10 */

  operationMatrix[54 + 1 * (1 + operationTableSize)] = 5; /* O2J15 @ M1 */
  operationMatrix[54 + 2 * (1 + operationTableSize)] = 6; /* O2J15 @ M2 */
  operationMatrix[54 + 3 * (1 + operationTableSize)] = 2; /* O2J15 @ M3 */
  operationMatrix[54 + 4 * (1 + operationTableSize)] = 5; /* O2J15 @ M4 */
  operationMatrix[54 + 5 * (1 + operationTableSize)] = 4; /* O2J15 @ M5 */
  operationMatrix[54 + 6 * (1 + operationTableSize)] = 2; /* O2J15 @ M6 */
  operationMatrix[54 + 7 * (1 + operationTableSize)] = 5; /* O2J15 @ M7 */
  operationMatrix[54 + 8 * (1 + operationTableSize)] = 3; /* O2J15 @ M8 */
  operationMatrix[54 + 9 * (1 + operationTableSize)] = 2; /* O2J15 @ M9 */
  operationMatrix[54 + 10 * (1 + operationTableSize)] = 5; /* O2J15 @ M10 */

  operationMatrix[55 + 1 * (1 + operationTableSize)] = 4; /* O3J15 @ M1 */
  operationMatrix[55 + 2 * (1 + operationTableSize)] = 5; /* O3J15 @ M2 */
  operationMatrix[55 + 3 * (1 + operationTableSize)] = 2; /* O3J15 @ M3 */
  operationMatrix[55 + 4 * (1 + operationTableSize)] = 3; /* O3J15 @ M4 */
  operationMatrix[55 + 5 * (1 + operationTableSize)] = 5; /* O3J15 @ M5 */
  operationMatrix[55 + 6 * (1 + operationTableSize)] = 2; /* O3J15 @ M6 */
  operationMatrix[55 + 7 * (1 + operationTableSize)] = 8; /* O3J15 @ M7 */
  operationMatrix[55 + 8 * (1 + operationTableSize)] = 4; /* O3J15 @ M8 */
  operationMatrix[55 + 9 * (1 + operationTableSize)] = 7; /* O3J15 @ M9 */
  operationMatrix[55 + 10 * (1 + operationTableSize)] = 5; /* O3J15 @ M10 */

  operationMatrix[56 + 1 * (1 + operationTableSize)] = 6; /* O4J15 @ M1 */
  operationMatrix[56 + 2 * (1 + operationTableSize)] = 2; /* O4J15 @ M2 */
  operationMatrix[56 + 3 * (1 + operationTableSize)] = 11; /* O4J15 @ M3 */
  operationMatrix[56 + 4 * (1 + operationTableSize)] = 14; /* O4J15 @ M4 */
  operationMatrix[56 + 5 * (1 + operationTableSize)] = 2; /* O4J15 @ M5 */
  operationMatrix[56 + 6 * (1 + operationTableSize)] = 3; /* O4J15 @ M6 */
  operationMatrix[56 + 7 * (1 + operationTableSize)] = 6; /* O4J15 @ M7 */
  operationMatrix[56 + 8 * (1 + operationTableSize)] = 5; /* O4J15 @ M8 */
  operationMatrix[56 + 9 * (1 + operationTableSize)] = 4; /* O4J15 @ M9 */
  operationMatrix[56 + 10 * (1 + operationTableSize)] = 8; /* O4J15 @ M10 */

  /* *************************************** */
  /* PRODUCTION TABLE */
  /* *************************************** */

  productionTableSize = 15;
  productionTable = (PPART *) malloc((1 + productionTableSize) * sizeof(PPART));
  productionTable[0] = NULL;

  if ((NUMPARTS == 0) || (NUMPARTS > productionTableSize)) {
    NUMPARTS = productionTableSize;
  }

  /* *************************************** */
  /* PART TABLE */
  /* *************************************** */

  pnum = 0;
  part = 0;

  partTableSize = 15; /* number of parts (including subparts) */
  partTable = (PART *) malloc((1 + partTableSize) * sizeof(PART));
  partTable[0].name = NULL; partTable[0].n = 0; partTable[0].X = NULL;
  for (i = 1; i <= partTableSize; i++)  {
    partTable[i].idx = i;
    partTable[i].color = 0;
  }

  /* *************************************** */
  /* JOB J1 */
  /* *************************************** */
  
  part++;

  pnum++; xnum = 0; onum = 0; /* next part, id = 1 */

  partTable[pnum].name = strdup("P1");
  partTable[pnum].part = part;
  partTable[pnum].n = 1; /* number of sequences */
  partTable[pnum].code = NULL; partTable[pnum].feasible = NULL;
  partTable[pnum].X = (SEQUENCE *) malloc((1 + partTable[pnum].n) * sizeof(SEQUENCE));
  partTable[pnum].X[0].n = 0; partTable[pnum].X[0].O = NULL;

  xnum++; onum = 0; /* next sequence */

  partTable[pnum].X[xnum].n = 4; /* number of items for the 1st sequence in J1 */
  partTable[pnum].X[xnum].O = (SEQUENCEITEM *) malloc((1 + partTable[pnum].X[xnum].n) * sizeof(SEQUENCEITEM));
  partTable[pnum].X[xnum].O[0].type = NONE; partTable[pnum].X[xnum].O[0].ITEM = NULL;
  partTable[pnum].X[xnum].O[++onum].type = DEFAULT; partTable[pnum].X[xnum].O[onum].ITEM = (void *) &(operationTable[1]); /* O1J1 */
  partTable[pnum].X[xnum].O[++onum].type = DEFAULT; partTable[pnum].X[xnum].O[onum].ITEM = (void *) &(operationTable[2]); /* O2J1 */
  partTable[pnum].X[xnum].O[++onum].type = DEFAULT; partTable[pnum].X[xnum].O[onum].ITEM = (void *) &(operationTable[3]); /* O3J1 */
  partTable[pnum].X[xnum].O[++onum].type = DEFAULT; partTable[pnum].X[xnum].O[onum].ITEM = (void *) &(operationTable[4]); /* O4J1 */

  productionTable[part] = &(partTable[pnum]);
  partTable[pnum].color = 0;

  /* ADDVARIABLE(w,r) */
  ADDVARIABLE(1,1);
  ADDVARIABLE(2,2);
  ADDVARIABLE(3,3);
  ADDVARIABLE(4,4);

  /* *************************************** */
  /* JOB J2 */
  /* *************************************** */

  part++;

  pnum++; xnum = 0; onum = 0; /* next part, id = 2 */

  partTable[pnum].name = strdup("P2");
  partTable[pnum].part = part;
  partTable[pnum].n = 1; /* number of sequences */
  partTable[pnum].code = NULL; partTable[pnum].feasible = NULL;
  partTable[pnum].X = (SEQUENCE *) malloc((1 + partTable[pnum].n) * sizeof(SEQUENCE));
  partTable[pnum].X[0].n = 0; partTable[pnum].X[0].O = NULL;

  xnum++; onum = 0; /* next sequence */

  partTable[pnum].X[xnum].n = 4; /* number of items for the 1st sequence in J2 */
  partTable[pnum].X[xnum].O = (SEQUENCEITEM *) malloc((1 + partTable[pnum].X[xnum].n) * sizeof(SEQUENCEITEM));
  partTable[pnum].X[xnum].O[0].type = NONE; partTable[pnum].X[xnum].O[0].ITEM = NULL;
  partTable[pnum].X[xnum].O[++onum].type = DEFAULT; partTable[pnum].X[xnum].O[onum].ITEM = (void *) &(operationTable[5]); /* O1J2 */
  partTable[pnum].X[xnum].O[++onum].type = DEFAULT; partTable[pnum].X[xnum].O[onum].ITEM = (void *) &(operationTable[6]); /* O2J2 */
  partTable[pnum].X[xnum].O[++onum].type = DEFAULT; partTable[pnum].X[xnum].O[onum].ITEM = (void *) &(operationTable[7]); /* O3J2 */
  partTable[pnum].X[xnum].O[++onum].type = DEFAULT; partTable[pnum].X[xnum].O[onum].ITEM = (void *) &(operationTable[8]); /* O4J2 */

  productionTable[part] = &(partTable[pnum]);
  partTable[pnum].color = 60;

  /* ADDVARIABLE(w,r) */
  ADDVARIABLE(5,1);
  ADDVARIABLE(6,2);
  ADDVARIABLE(7,3);
  ADDVARIABLE(8,4);

  /* *************************************** */
  /* JOB J3 */
  /* *************************************** */

  part++;

  pnum++; xnum = 0; onum = 0; /* next part, id = 3 */

  partTable[pnum].name = strdup("P3");
  partTable[pnum].part = part;
  partTable[pnum].n = 1; /* number of sequences */
  partTable[pnum].code = NULL; partTable[pnum].feasible = NULL;
  partTable[pnum].X = (SEQUENCE *) malloc((1 + partTable[pnum].n) * sizeof(SEQUENCE));
  partTable[pnum].X[0].n = 0; partTable[pnum].X[0].O = NULL;

  xnum++; onum = 0; /* next sequence */

  partTable[pnum].X[xnum].n = 4; /* number of items for the 1st sequence in J3 */
  partTable[pnum].X[xnum].O = (SEQUENCEITEM *) malloc((1 + partTable[pnum].X[xnum].n) * sizeof(SEQUENCEITEM));
  partTable[pnum].X[xnum].O[0].type = NONE; partTable[pnum].X[xnum].O[0].ITEM = NULL;
  partTable[pnum].X[xnum].O[++onum].type = DEFAULT; partTable[pnum].X[xnum].O[onum].ITEM = (void *) &(operationTable[9]); /* O1J3 */
  partTable[pnum].X[xnum].O[++onum].type = DEFAULT; partTable[pnum].X[xnum].O[onum].ITEM = (void *) &(operationTable[10]); /* O2J3 */
  partTable[pnum].X[xnum].O[++onum].type = DEFAULT; partTable[pnum].X[xnum].O[onum].ITEM = (void *) &(operationTable[11]); /* O3J3 */
  partTable[pnum].X[xnum].O[++onum].type = DEFAULT; partTable[pnum].X[xnum].O[onum].ITEM = (void *) &(operationTable[12]); /* O4J3 */

  productionTable[part] = &(partTable[pnum]);
  partTable[pnum].color = 140;

  /* ADDVARIABLE(w,r) */
  ADDVARIABLE(9,1);
  ADDVARIABLE(10,2);
  ADDVARIABLE(11,3);
  ADDVARIABLE(12,4);

  /* *************************************** */
  /* JOB J4 */
  /* *************************************** */

  part++;

  pnum++; xnum = 0; onum = 0; /* next part, id = 4 */

  partTable[pnum].name = strdup("P4");
  partTable[pnum].part = part;
  partTable[pnum].n = 1; /* number of sequences */
  partTable[pnum].code = NULL; partTable[pnum].feasible = NULL;
  partTable[pnum].X = (SEQUENCE *) malloc((1 + partTable[pnum].n) * sizeof(SEQUENCE));
  partTable[pnum].X[0].n = 0; partTable[pnum].X[0].O = NULL;

  xnum++; onum = 0; /* next sequence */

  partTable[pnum].X[xnum].n = 4; /* number of items for the 1st sequence in J4 */
  partTable[pnum].X[xnum].O = (SEQUENCEITEM *) malloc((1 + partTable[pnum].X[xnum].n) * sizeof(SEQUENCEITEM));
  partTable[pnum].X[xnum].O[0].type = NONE; partTable[pnum].X[xnum].O[0].ITEM = NULL;
  partTable[pnum].X[xnum].O[++onum].type = DEFAULT; partTable[pnum].X[xnum].O[onum].ITEM = (void *) &(operationTable[13]); /* O1J4 */
  partTable[pnum].X[xnum].O[++onum].type = DEFAULT; partTable[pnum].X[xnum].O[onum].ITEM = (void *) &(operationTable[14]); /* O2J4 */
  partTable[pnum].X[xnum].O[++onum].type = DEFAULT; partTable[pnum].X[xnum].O[onum].ITEM = (void *) &(operationTable[15]); /* O3J4 */
  partTable[pnum].X[xnum].O[++onum].type = DEFAULT; partTable[pnum].X[xnum].O[onum].ITEM = (void *) &(operationTable[16]); /* O4J4 */

  productionTable[part] = &(partTable[pnum]);
  partTable[pnum].color = 180;

  /* ADDVARIABLE(w,r) */
  ADDVARIABLE(13,1);
  ADDVARIABLE(14,2);
  ADDVARIABLE(15,3);
  ADDVARIABLE(16,4);

  /* *************************************** */
  /* JOB J5 */
  /* *************************************** */

  part++;

  pnum++; xnum = 0; onum = 0; /* next part, id = 5 */

  partTable[pnum].name = strdup("P5");
  partTable[pnum].part = part;
  partTable[pnum].n = 1; /* number of sequences */
  partTable[pnum].code = NULL; partTable[pnum].feasible = NULL;
  partTable[pnum].X = (SEQUENCE *) malloc((1 + partTable[pnum].n) * sizeof(SEQUENCE));
  partTable[pnum].X[0].n = 0; partTable[pnum].X[0].O = NULL;

  xnum++; onum = 0; /* next sequence */

  partTable[pnum].X[xnum].n = 4; /* number of items for the 1st sequence in J5 */
  partTable[pnum].X[xnum].O = (SEQUENCEITEM *) malloc((1 + partTable[pnum].X[xnum].n) * sizeof(SEQUENCEITEM));
  partTable[pnum].X[xnum].O[0].type = NONE; partTable[pnum].X[xnum].O[0].ITEM = NULL;
  partTable[pnum].X[xnum].O[++onum].type = DEFAULT; partTable[pnum].X[xnum].O[onum].ITEM = (void *) &(operationTable[17]); /* O1J5 */
  partTable[pnum].X[xnum].O[++onum].type = DEFAULT; partTable[pnum].X[xnum].O[onum].ITEM = (void *) &(operationTable[18]); /* O2J5 */
  partTable[pnum].X[xnum].O[++onum].type = DEFAULT; partTable[pnum].X[xnum].O[onum].ITEM = (void *) &(operationTable[19]); /* O3J5 */
  partTable[pnum].X[xnum].O[++onum].type = DEFAULT; partTable[pnum].X[xnum].O[onum].ITEM = (void *) &(operationTable[20]); /* O4J5 */

  productionTable[part] = &(partTable[pnum]);
  partTable[pnum].color = 240;

  /* ADDVARIABLE(w,r) */
  ADDVARIABLE(17,1);
  ADDVARIABLE(18,2);
  ADDVARIABLE(19,3);
  ADDVARIABLE(20,4);

  /* *************************************** */
  /* JOB J6 */
  /* *************************************** */

  part++;

  pnum++; xnum = 0; onum = 0; /* next part, id = 6 */

  partTable[pnum].name = strdup("P6");
  partTable[pnum].part = part;
  partTable[pnum].n = 1; /* number of sequences */
  partTable[pnum].code = NULL; partTable[pnum].feasible = NULL;
  partTable[pnum].X = (SEQUENCE *) malloc((1 + partTable[pnum].n) * sizeof(SEQUENCE));
  partTable[pnum].X[0].n = 0; partTable[pnum].X[0].O = NULL;

  xnum++; onum = 0; /* next sequence */

  partTable[pnum].X[xnum].n = 2; /* number of items for the 1st sequence in J6 */
  partTable[pnum].X[xnum].O = (SEQUENCEITEM *) malloc((1 + partTable[pnum].X[xnum].n) * sizeof(SEQUENCEITEM));
  partTable[pnum].X[xnum].O[0].type = NONE; partTable[pnum].X[xnum].O[0].ITEM = NULL;
  partTable[pnum].X[xnum].O[++onum].type = DEFAULT; partTable[pnum].X[xnum].O[onum].ITEM = (void *) &(operationTable[21]); /* O1J6 */
  partTable[pnum].X[xnum].O[++onum].type = DEFAULT; partTable[pnum].X[xnum].O[onum].ITEM = (void *) &(operationTable[22]); /* O2J6 */

  productionTable[part] = &(partTable[pnum]);
  partTable[pnum].color = 300;

  /* ADDVARIABLE(w,r) */
  ADDVARIABLE(21,1);
  ADDVARIABLE(22,2);

  /* *************************************** */
  /* JOB J7 */
  /* *************************************** */

  part++;

  pnum++; xnum = 0; onum = 0; /* next part, id = 7 */

  partTable[pnum].name = strdup("P7");
  partTable[pnum].part = part;
  partTable[pnum].n = 1; /* number of sequences */
  partTable[pnum].code = NULL; partTable[pnum].feasible = NULL;
  partTable[pnum].X = (SEQUENCE *) malloc((1 + partTable[pnum].n) * sizeof(SEQUENCE));
  partTable[pnum].X[0].n = 0; partTable[pnum].X[0].O = NULL;

  xnum++; onum = 0; /* next sequence */

  partTable[pnum].X[xnum].n = 2; /* number of items for the 1st sequence in J7 */
  partTable[pnum].X[xnum].O = (SEQUENCEITEM *) malloc((1 + partTable[pnum].X[xnum].n) * sizeof(SEQUENCEITEM));
  partTable[pnum].X[xnum].O[0].type = NONE; partTable[pnum].X[xnum].O[0].ITEM = NULL;
  partTable[pnum].X[xnum].O[++onum].type = DEFAULT; partTable[pnum].X[xnum].O[onum].ITEM = (void *) &(operationTable[23]); /* O1J7 */
  partTable[pnum].X[xnum].O[++onum].type = DEFAULT; partTable[pnum].X[xnum].O[onum].ITEM = (void *) &(operationTable[24]); /* O2J7 */

  productionTable[part] = &(partTable[pnum]);
  partTable[pnum].color = 25;

  /* ADDVARIABLE(w,r) */
  ADDVARIABLE(23,1);
  ADDVARIABLE(24,2);

  /* *************************************** */
  /* JOB J8 */
  /* *************************************** */

  part++;

  pnum++; xnum = 0; onum = 0; /* next part, id = 8 */

  partTable[pnum].name = strdup("P8");
  partTable[pnum].part = part;
  partTable[pnum].n = 1; /* number of sequences */
  partTable[pnum].code = NULL; partTable[pnum].feasible = NULL;
  partTable[pnum].X = (SEQUENCE *) malloc((1 + partTable[pnum].n) * sizeof(SEQUENCE));
  partTable[pnum].X[0].n = 0; partTable[pnum].X[0].O = NULL;

  xnum++; onum = 0; /* next sequence */

  partTable[pnum].X[xnum].n = 4; /* number of items for the 1st sequence in J8 */
  partTable[pnum].X[xnum].O = (SEQUENCEITEM *) malloc((1 + partTable[pnum].X[xnum].n) * sizeof(SEQUENCEITEM));
  partTable[pnum].X[xnum].O[0].type = NONE; partTable[pnum].X[xnum].O[0].ITEM = NULL;
  partTable[pnum].X[xnum].O[++onum].type = DEFAULT; partTable[pnum].X[xnum].O[onum].ITEM = (void *) &(operationTable[25]); /* O1J8 */
  partTable[pnum].X[xnum].O[++onum].type = DEFAULT; partTable[pnum].X[xnum].O[onum].ITEM = (void *) &(operationTable[26]); /* O2J8 */
  partTable[pnum].X[xnum].O[++onum].type = DEFAULT; partTable[pnum].X[xnum].O[onum].ITEM = (void *) &(operationTable[27]); /* O3J8 */
  partTable[pnum].X[xnum].O[++onum].type = DEFAULT; partTable[pnum].X[xnum].O[onum].ITEM = (void *) &(operationTable[28]); /* O4J8 */

  productionTable[part] = &(partTable[pnum]);
  partTable[pnum].color = 85;

  /* ADDVARIABLE(w,r) */
  ADDVARIABLE(25,1);
  ADDVARIABLE(26,2);
  ADDVARIABLE(27,3);
  ADDVARIABLE(28,4);

  /* *************************************** */
  /* JOB J9 */
  /* *************************************** */

  part++;

  pnum++; xnum = 0; onum = 0; /* next part, id = 9 */

  partTable[pnum].name = strdup("P9");
  partTable[pnum].part = part;
  partTable[pnum].n = 1; /* number of sequences */
  partTable[pnum].code = NULL; partTable[pnum].feasible = NULL;
  partTable[pnum].X = (SEQUENCE *) malloc((1 + partTable[pnum].n) * sizeof(SEQUENCE));
  partTable[pnum].X[0].n = 0; partTable[pnum].X[0].O = NULL;

  xnum++; onum = 0; /* next sequence */

  partTable[pnum].X[xnum].n = 4; /* number of items for the 1st sequence in J9 */
  partTable[pnum].X[xnum].O = (SEQUENCEITEM *) malloc((1 + partTable[pnum].X[xnum].n) * sizeof(SEQUENCEITEM));
  partTable[pnum].X[xnum].O[0].type = NONE; partTable[pnum].X[xnum].O[0].ITEM = NULL;
  partTable[pnum].X[xnum].O[++onum].type = DEFAULT; partTable[pnum].X[xnum].O[onum].ITEM = (void *) &(operationTable[29]); /* O1J9 */
  partTable[pnum].X[xnum].O[++onum].type = DEFAULT; partTable[pnum].X[xnum].O[onum].ITEM = (void *) &(operationTable[30]); /* O2J9 */
  partTable[pnum].X[xnum].O[++onum].type = DEFAULT; partTable[pnum].X[xnum].O[onum].ITEM = (void *) &(operationTable[31]); /* O3J9 */
  partTable[pnum].X[xnum].O[++onum].type = DEFAULT; partTable[pnum].X[xnum].O[onum].ITEM = (void *) &(operationTable[32]); /* O4J9 */

  productionTable[part] = &(partTable[pnum]);
  partTable[pnum].color = 145;

  /* ADDVARIABLE(w,r) */
  ADDVARIABLE(29,1);
  ADDVARIABLE(30,2);
  ADDVARIABLE(31,3);
  ADDVARIABLE(32,4);

  /* *************************************** */
  /* JOB J10 */
  /* *************************************** */

  part++;

  pnum++; xnum = 0; onum = 0; /* next part, id = 10 */

  partTable[pnum].name = strdup("P10");
  partTable[pnum].part = part;
  partTable[pnum].n = 1; /* number of sequences */
  partTable[pnum].code = NULL; partTable[pnum].feasible = NULL;
  partTable[pnum].X = (SEQUENCE *) malloc((1 + partTable[pnum].n) * sizeof(SEQUENCE));
  partTable[pnum].X[0].n = 0; partTable[pnum].X[0].O = NULL;

  xnum++; onum = 0; /* next sequence */

  partTable[pnum].X[xnum].n = 4; /* number of items for the 1st sequence in J10 */
  partTable[pnum].X[xnum].O = (SEQUENCEITEM *) malloc((1 + partTable[pnum].X[xnum].n) * sizeof(SEQUENCEITEM));
  partTable[pnum].X[xnum].O[0].type = NONE; partTable[pnum].X[xnum].O[0].ITEM = NULL;
  partTable[pnum].X[xnum].O[++onum].type = DEFAULT; partTable[pnum].X[xnum].O[onum].ITEM = (void *) &(operationTable[33]); /* O1J10 */
  partTable[pnum].X[xnum].O[++onum].type = DEFAULT; partTable[pnum].X[xnum].O[onum].ITEM = (void *) &(operationTable[34]); /* O2J10 */
  partTable[pnum].X[xnum].O[++onum].type = DEFAULT; partTable[pnum].X[xnum].O[onum].ITEM = (void *) &(operationTable[35]); /* O3J10 */
  partTable[pnum].X[xnum].O[++onum].type = DEFAULT; partTable[pnum].X[xnum].O[onum].ITEM = (void *) &(operationTable[36]); /* O4J10 */

  productionTable[part] = &(partTable[pnum]);
  partTable[pnum].color = 205;

  /* ADDVARIABLE(w,r) */
  ADDVARIABLE(33,1);
  ADDVARIABLE(34,2);
  ADDVARIABLE(35,3);
  ADDVARIABLE(36,4);

  /* *************************************** */
  /* JOB J11 */
  /* *************************************** */

  part++;

  pnum++; xnum = 0; onum = 0; /* next part, id = 11 */

  partTable[pnum].name = strdup("P11");
  partTable[pnum].part = part;
  partTable[pnum].n = 1; /* number of sequences */
  partTable[pnum].code = NULL; partTable[pnum].feasible = NULL;
  partTable[pnum].X = (SEQUENCE *) malloc((1 + partTable[pnum].n) * sizeof(SEQUENCE));
  partTable[pnum].X[0].n = 0; partTable[pnum].X[0].O = NULL;

  xnum++; onum = 0; /* next sequence */

  partTable[pnum].X[xnum].n = 4; /* number of items for the 1st sequence in J11 */
  partTable[pnum].X[xnum].O = (SEQUENCEITEM *) malloc((1 + partTable[pnum].X[xnum].n) * sizeof(SEQUENCEITEM));
  partTable[pnum].X[xnum].O[0].type = NONE; partTable[pnum].X[xnum].O[0].ITEM = NULL;
  partTable[pnum].X[xnum].O[++onum].type = DEFAULT; partTable[pnum].X[xnum].O[onum].ITEM = (void *) &(operationTable[37]); /* O1J11 */
  partTable[pnum].X[xnum].O[++onum].type = DEFAULT; partTable[pnum].X[xnum].O[onum].ITEM = (void *) &(operationTable[38]); /* O2J11 */
  partTable[pnum].X[xnum].O[++onum].type = DEFAULT; partTable[pnum].X[xnum].O[onum].ITEM = (void *) &(operationTable[39]); /* O3J11 */
  partTable[pnum].X[xnum].O[++onum].type = DEFAULT; partTable[pnum].X[xnum].O[onum].ITEM = (void *) &(operationTable[40]); /* O4J11 */

  productionTable[part] = &(partTable[pnum]);
  partTable[pnum].color = 265;

  /* ADDVARIABLE(w,r) */
  ADDVARIABLE(37,1);
  ADDVARIABLE(38,2);
  ADDVARIABLE(39,3);
  ADDVARIABLE(40,4);

  /* *************************************** */
  /* JOB J12 */
  /* *************************************** */

  part++;

  pnum++; xnum = 0; onum = 0; /* next part, id = 12 */

  partTable[pnum].name = strdup("P12");
  partTable[pnum].part = part;
  partTable[pnum].n = 1; /* number of sequences */
  partTable[pnum].code = NULL; partTable[pnum].feasible = NULL;
  partTable[pnum].X = (SEQUENCE *) malloc((1 + partTable[pnum].n) * sizeof(SEQUENCE));
  partTable[pnum].X[0].n = 0; partTable[pnum].X[0].O = NULL;

  xnum++; onum = 0; /* next sequence */

  partTable[pnum].X[xnum].n = 4; /* number of items for the 1st sequence in J12 */
  partTable[pnum].X[xnum].O = (SEQUENCEITEM *) malloc((1 + partTable[pnum].X[xnum].n) * sizeof(SEQUENCEITEM));
  partTable[pnum].X[xnum].O[0].type = NONE; partTable[pnum].X[xnum].O[0].ITEM = NULL;
  partTable[pnum].X[xnum].O[++onum].type = DEFAULT; partTable[pnum].X[xnum].O[onum].ITEM = (void *) &(operationTable[41]); /* O1J12 */
  partTable[pnum].X[xnum].O[++onum].type = DEFAULT; partTable[pnum].X[xnum].O[onum].ITEM = (void *) &(operationTable[42]); /* O2J12 */
  partTable[pnum].X[xnum].O[++onum].type = DEFAULT; partTable[pnum].X[xnum].O[onum].ITEM = (void *) &(operationTable[43]); /* O3J12 */
  partTable[pnum].X[xnum].O[++onum].type = DEFAULT; partTable[pnum].X[xnum].O[onum].ITEM = (void *) &(operationTable[44]); /* O4J12 */

  productionTable[part] = &(partTable[pnum]);
  partTable[pnum].color = 325;

  /* ADDVARIABLE(w,r) */
  ADDVARIABLE(41,1);
  ADDVARIABLE(42,2);
  ADDVARIABLE(43,3);
  ADDVARIABLE(44,4);

  /* *************************************** */
  /* JOB J13 */
  /* *************************************** */

  part++;

  pnum++; xnum = 0; onum = 0; /* next part, id = 13 */

  partTable[pnum].name = strdup("P13");
  partTable[pnum].part = part;
  partTable[pnum].n = 1; /* number of sequences */
  partTable[pnum].code = NULL; partTable[pnum].feasible = NULL;
  partTable[pnum].X = (SEQUENCE *) malloc((1 + partTable[pnum].n) * sizeof(SEQUENCE));
  partTable[pnum].X[0].n = 0; partTable[pnum].X[0].O = NULL;

  xnum++; onum = 0; /* next sequence */

  partTable[pnum].X[xnum].n = 4; /* number of items for the 1st sequence in J13 */
  partTable[pnum].X[xnum].O = (SEQUENCEITEM *) malloc((1 + partTable[pnum].X[xnum].n) * sizeof(SEQUENCEITEM));
  partTable[pnum].X[xnum].O[0].type = NONE; partTable[pnum].X[xnum].O[0].ITEM = NULL;
  partTable[pnum].X[xnum].O[++onum].type = DEFAULT; partTable[pnum].X[xnum].O[onum].ITEM = (void *) &(operationTable[45]); /* O1J13 */
  partTable[pnum].X[xnum].O[++onum].type = DEFAULT; partTable[pnum].X[xnum].O[onum].ITEM = (void *) &(operationTable[46]); /* O2J13 */
  partTable[pnum].X[xnum].O[++onum].type = DEFAULT; partTable[pnum].X[xnum].O[onum].ITEM = (void *) &(operationTable[47]); /* O3J13 */
  partTable[pnum].X[xnum].O[++onum].type = DEFAULT; partTable[pnum].X[xnum].O[onum].ITEM = (void *) &(operationTable[48]); /* O4J13 */

  productionTable[part] = &(partTable[pnum]);
  partTable[pnum].color = 40;

  /* ADDVARIABLE(w,r) */
  ADDVARIABLE(45,1);
  ADDVARIABLE(46,2);
  ADDVARIABLE(47,3);
  ADDVARIABLE(48,4);

  /* *************************************** */
  /* JOB J14 */
  /* *************************************** */

  part++;

  pnum++; xnum = 0; onum = 0; /* next part, id = 14 */

  partTable[pnum].name = strdup("P14");
  partTable[pnum].part = part;
  partTable[pnum].n = 1; /* number of sequences */
  partTable[pnum].code = NULL; partTable[pnum].feasible = NULL;
  partTable[pnum].X = (SEQUENCE *) malloc((1 + partTable[pnum].n) * sizeof(SEQUENCE));
  partTable[pnum].X[0].n = 0; partTable[pnum].X[0].O = NULL;

  xnum++; onum = 0; /* next sequence */

  partTable[pnum].X[xnum].n = 4; /* number of items for the 1st sequence in J14 */
  partTable[pnum].X[xnum].O = (SEQUENCEITEM *) malloc((1 + partTable[pnum].X[xnum].n) * sizeof(SEQUENCEITEM));
  partTable[pnum].X[xnum].O[0].type = NONE; partTable[pnum].X[xnum].O[0].ITEM = NULL;
  partTable[pnum].X[xnum].O[++onum].type = DEFAULT; partTable[pnum].X[xnum].O[onum].ITEM = (void *) &(operationTable[49]); /* O1J14 */
  partTable[pnum].X[xnum].O[++onum].type = DEFAULT; partTable[pnum].X[xnum].O[onum].ITEM = (void *) &(operationTable[50]); /* O2J14 */
  partTable[pnum].X[xnum].O[++onum].type = DEFAULT; partTable[pnum].X[xnum].O[onum].ITEM = (void *) &(operationTable[51]); /* O3J14 */
  partTable[pnum].X[xnum].O[++onum].type = DEFAULT; partTable[pnum].X[xnum].O[onum].ITEM = (void *) &(operationTable[52]); /* O4J14 */

  productionTable[part] = &(partTable[pnum]);
  partTable[pnum].color = 160;

  /* ADDVARIABLE(w,r) */
  ADDVARIABLE(49,1);
  ADDVARIABLE(50,2);
  ADDVARIABLE(51,3);
  ADDVARIABLE(52,4);

  /* *************************************** */
  /* JOB J15 */
  /* *************************************** */

  part++;

  pnum++; xnum = 0; onum = 0; /* next part, id = 15 */

  partTable[pnum].name = strdup("P15");
  partTable[pnum].part = part;
  partTable[pnum].n = 1; /* number of sequences */
  partTable[pnum].code = NULL; partTable[pnum].feasible = NULL;
  partTable[pnum].X = (SEQUENCE *) malloc((1 + partTable[pnum].n) * sizeof(SEQUENCE));
  partTable[pnum].X[0].n = 0; partTable[pnum].X[0].O = NULL;

  xnum++; onum = 0; /* next sequence */

  partTable[pnum].X[xnum].n = 4; /* number of items for the 1st sequence in J15 */
  partTable[pnum].X[xnum].O = (SEQUENCEITEM *) malloc((1 + partTable[pnum].X[xnum].n) * sizeof(SEQUENCEITEM));
  partTable[pnum].X[xnum].O[0].type = NONE; partTable[pnum].X[xnum].O[0].ITEM = NULL;
  partTable[pnum].X[xnum].O[++onum].type = DEFAULT; partTable[pnum].X[xnum].O[onum].ITEM = (void *) &(operationTable[53]); /* O1J15 */
  partTable[pnum].X[xnum].O[++onum].type = DEFAULT; partTable[pnum].X[xnum].O[onum].ITEM = (void *) &(operationTable[54]); /* O2J15 */
  partTable[pnum].X[xnum].O[++onum].type = DEFAULT; partTable[pnum].X[xnum].O[onum].ITEM = (void *) &(operationTable[55]); /* O3J15 */
  partTable[pnum].X[xnum].O[++onum].type = DEFAULT; partTable[pnum].X[xnum].O[onum].ITEM = (void *) &(operationTable[56]); /* O4J15 */

  productionTable[part] = &(partTable[pnum]);
  partTable[pnum].color = 280;

  /* ADDVARIABLE(w,r) */
  ADDVARIABLE(53,1);
  ADDVARIABLE(54,2);
  ADDVARIABLE(55,3);
  ADDVARIABLE(56,4);
}

/* ************************************************************************************ */
/* ************************************************************************************ */
/* ************************************************************************************ */

void
generateSystem()
{
  printf("\nNOT IMPLEMENTED, YET\n");
  exit(1);
}

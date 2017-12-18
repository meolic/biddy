/* $Revision: 356 $ */
/* $Date: 2017-12-13 22:30:46 +0100 (sre, 13 dec 2017) $ */
/* This file (biddy-cudd.c) is a C file */
/* Author: Robert Meolic (robert.meolic@um.si) */
/* This file has been released into the public domain by the author. */

/* This example is compatible with Biddy v1.7.4 and CUDD v3.0.0 */
/* See Biddy examples (e.g. biddy-example-dictionary.c) to see how this file is used */

/* There is only one unique table in Biddy */
/* There are four caches in Biddy */
/* New nodes are added in blocks */
/* The max number of variables is hardcoded in biddyInt. h */
/* biddyVariableTable.size = BIDDYVARMAX = 2048 */
/* The following constants are hardcoded in biddyMain.c */
/* biddyNodeTable.size = BIG_TABLE = 1048575 */
/* biddyIteCache.size = MEDIUM_TABLE = 262143 */
/* biddyEACache.size = SMALL_TABLE = 65535 */
/* biddyRCCache.size = SMALL_TABLE = 65535 */
/* biddyReplaceCache.size = SMALL_TABLE = 65535 */
/* biddyBlockSize = 524288 */
/* DEFAULT INIT CALL: Biddy_InitAnonymous(BDDTYPE) */

#ifdef BIDDY
  Biddy_InitAnonymous(BDDTYPE);
  INITGC;
#endif

/* In CUDD each variable has its own subtable in the unique table */
/* There is only one cache in CUDD */
/* Subtables grow over the time, you can set limit for fast unique table growth */
/* Cudd_SetLooseUpTo(manager,1048576) */
/* Cache can grow over the time, you can set the max size */
/* Cudd_SetMaxCacheHard(manager,262144) */
/* These two constants are hardcoded in v3.0.0 */
/* CUDD_UNIQUE_SLOTS = 256 (default initial size of each subtable) */
/* CUDD_CACHE_SLOTS = 262144 (default initial size of cache table) */
/* DEFAULT INIT CALL: Cudd_Init(0,0,CUDD_UNIQUE_SLOTS,CUDD_CACHE_SLOTS,0) */

#ifdef CUDD
  manager = Cudd_Init(0,0,CUDD_UNIQUE_SLOTS,CUDD_CACHE_SLOTS,0);
  Cudd_DisableReorderingReporting(manager);
  INITGC;
#endif

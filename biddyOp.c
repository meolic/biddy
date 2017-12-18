/***************************************************************************//*!
\file biddyOp.c
\brief File biddyOp.c contains functions for operations on various types of
Binary Decision Diagrams.

### Description

    PackageName [Biddy]
    Synopsis    [Biddy provides data structures and algorithms for the
                 representation and manipulation of Boolean functions with
                 ROBDDs, 0-sup-BDDs, and TZBDDs. A hash table is used for quick
             		 search of nodes. Complement edges decreases the number of
		             nodes. An automatic garbage collection with a system age is
                 implemented. Variable swapping and sifting are implemented.]

    FileName    [biddyOp.c]
    Revision    [$Revision: 365 $]
    Date        [$Date: 2017-12-18 13:01:40 +0100 (pon, 18 dec 2017) $]
    Authors     [Robert Meolic (robert.meolic@um.si)]

### Copyright

Copyright (C) 2006, 2017 UM FERI, Koroska cesta 46, SI-2000 Maribor, Slovenia

Biddy is free software; you can redistribute it and/or modify it under the terms
of the GNU General Public License as published by the Free Software Foundation;
either version 2 of the License, or (at your option) any later version.

Biddy is distributed in the hope that it will be useful, but WITHOUT ANY
WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A
PARTICULAR PURPOSE. See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License along with
this program; if not, write to the Free Software Foundation, Inc., 51 Franklin
Street, Fifth Floor, Boston, MA 02110-1301 USA.

### More info

See also: biddy.h, biddyInt.h

*******************************************************************************/

#include "biddyInt.h"

/*----------------------------------------------------------------------------*/
/* Constant declarations                                                      */
/*----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------*/
/* Variable declarations                                                      */
/*----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------*/
/* Static function prototypes                                                 */
/*----------------------------------------------------------------------------*/

static void exchangeEdges(Biddy_Edge *f, Biddy_Edge *g);

static void complExchangeEdges(Biddy_Edge *f, Biddy_Edge *g);

static Biddy_Edge createMinterm(Biddy_Manager MNG, Biddy_Edge support, long long unsigned int n, long long unsigned int x);

static Biddy_Edge createFunction(Biddy_Manager MNG, Biddy_Edge support, long long unsigned int n, long long unsigned int m, long long unsigned int x);

static inline unsigned int op3Hash(Biddy_Edge a, Biddy_Edge b, Biddy_Edge c, unsigned int size);

static inline unsigned int keywordHash(Biddy_Edge a, unsigned int k, unsigned int size);

static inline void addOp3Cache(Biddy_Manager MNG, BiddyOp3CacheTable cache, Biddy_Edge a, Biddy_Edge b, Biddy_Edge c, Biddy_Edge r, unsigned int index);

static inline Biddy_Boolean findOp3Cache(Biddy_Manager MNG, BiddyOp3CacheTable cache, Biddy_Edge a, Biddy_Edge b, Biddy_Edge c, Biddy_Edge *r, unsigned int *index);

static inline void addKeywordCache(Biddy_Manager MNG, BiddyKeywordCacheTable cache, Biddy_Edge a, unsigned int k, Biddy_Edge r, unsigned int index);

static inline Biddy_Boolean findKeywordCache(Biddy_Manager MNG, BiddyKeywordCacheTable cache, Biddy_Edge a, unsigned int k, Biddy_Edge *r, unsigned int *index);

/*----------------------------------------------------------------------------*/
/* Definition of exported functions                                           */
/*----------------------------------------------------------------------------*/

/***************************************************************************//*!
\brief Function Biddy_Managed_Not calculates Boolean function NOT.

### Description
### Side effects
    Implemented for OBDD, OBDDC, ZBDDC, and TZBDD.
    For OBDDC and OFDDC, it is better to use macro Biddy_Inv.
    For OBDDC, cache table is not needed.
    For ZBDDC, recursive calls are via Xor and thus its cache table is used.
    For OBDD and TZBDD, results are cached as (f,biddyZero,biddyOne).
### More info
    Macro Biddy_Not() is defined for use with anonymous manager.
*******************************************************************************/

#ifdef __cplusplus
extern "C" {
#endif

Biddy_Edge
Biddy_Managed_Not(Biddy_Manager MNG, Biddy_Edge f)
{
  Biddy_Edge r;

  assert( f != NULL );

  if (!MNG) MNG = biddyAnonymousManager;
  ZF_LOGI("Biddy_Not");

  assert( BiddyIsOK(f) == TRUE );

  r = biddyNull;

  if (biddyManagerType == BIDDYTYPEOBDD) {
    /* IMPLEMENTED */
    r = BiddyManagedNot(MNG,f);
    BiddyRefresh(r); /* not always refreshed by BiddyManagedNot */
  } else if (biddyManagerType == BIDDYTYPEOBDDC) {
    /* IMPLEMENTED */
    r = Biddy_Inv(f);
    /* BiddyRefresh() */ /* not needed because the same node is returned */
  } else if (biddyManagerType == BIDDYTYPEZBDDC) {
    /* IMPLEMENTED */
    r = BiddyManagedNot(MNG,f);
    BiddyRefresh(r); /* not always refreshed by BiddyManagedNot */
  } else if (biddyManagerType == BIDDYTYPETZBDD) {
    /* IMPLEMENTED */
    r = BiddyManagedNot(MNG,f);
    BiddyRefresh(r); /* not always refreshed by BiddyManagedNot */
  } else if ((biddyManagerType == BIDDYTYPEZBDD) ||
              (biddyManagerType == BIDDYTYPETZBDDC))
  {
    fprintf(stderr,"Biddy_Not: this GDD type is not supported, yet!\n");
    return biddyNull;
  } else if ((biddyManagerType == BIDDYTYPEOFDDC) || (biddyManagerType == BIDDYTYPEOFDD) ||
              (biddyManagerType == BIDDYTYPEZFDDC) || (biddyManagerType == BIDDYTYPEZFDD) ||
              (biddyManagerType == BIDDYTYPETZFDDC) || (biddyManagerType == BIDDYTYPETZFDD))
  {
    fprintf(stderr,"Biddy_Not: this GDD type is not supported, yet!\n");
    return biddyNull;
  } else {
    fprintf(stderr,"Biddy_Not: Unsupported GDD type!\n");
    return biddyNull;
  }

  return r;
}

#ifdef __cplusplus
}
#endif

Biddy_Edge
BiddyManagedNot(const Biddy_Manager MNG, const Biddy_Edge f)
{
  Biddy_Edge r, T, E, Fv, Gv, Fneg_v, Gneg_v;
  Biddy_Edge FF, GG, HH;
  Biddy_Variable v, nt;
  unsigned int cindex;

  assert( MNG != NULL );
  assert( f != NULL );

  /* IMPLEMENTED FOR OBDD, OBDDC, ZBDDC, AND TZBDD */
  assert(
    (biddyManagerType == BIDDYTYPEOBDD) ||
    (biddyManagerType == BIDDYTYPEOBDDC) ||
    (biddyManagerType == BIDDYTYPEZBDDC) ||
    (biddyManagerType == BIDDYTYPETZBDD)
  );

  r = biddyNull;

  /* LOOKING FOR SIMPLE CASE */

  if (f == biddyZero) {
    return biddyOne;
  }
  if (f == biddyOne) {
    return biddyZero;
  }

  /* THIS IS NOT A SIMPLE CASE */

  FF = GG = HH = biddyNull;
  Fneg_v = Fv = biddyNull;
  Gneg_v = Gv = biddyNull;
  v = 0;

  if (biddyManagerType == BIDDYTYPEOBDDC) {

    /* FOR OBDDC, CACHE TABLE IS NOT NEEDED */

    r = Biddy_Inv(f);

  }

  else if (biddyManagerType == BIDDYTYPEOBDD) {

    /* FOR OBDD, 'NOT' USES CACHE TABLE DIRECTLY */
    /* THIS IS NOT ITE CACHE! */

    FF = f;
    GG = biddyZero;
    HH = biddyOne;

    /* IF RESULT IS NOT IN THE CACHE TABLE... */
    cindex = 0;
    if (!findOp3Cache(MNG,biddyOPCache,FF,GG,HH,&r,&cindex))
    {

      /* DETERMINING PARAMETERS FOR RECURSIVE CALLS */
      v = BiddyV(f);
      Fneg_v = BiddyE(f);
      Fv = BiddyT(f);

      /* RECURSIVE CALLS */
      if (Fneg_v == biddyZero) {
        E = biddyOne;
      } else if (Fneg_v == biddyOne) {
        E = biddyZero;
      } else  {
        E = BiddyManagedNot(MNG,Fneg_v);
      }
      if (Fv == biddyZero) {
        T = biddyOne;
      } else if (Fv == biddyOne) {
        T = biddyZero;
      } else  {
        T = BiddyManagedNot(MNG,Fv);
      }

      r = BiddyManagedTaggedFoaNode(MNG,v,E,T,v,TRUE);
      BiddyRefresh(r); /* FoaNode returns an obsolete node! */

      addOp3Cache(MNG,biddyOPCache,FF,GG,HH,r,cindex);

    } else {

      /* IF THE RESULT IS FROM CACHE TABLE, REFRESH IT! */
      BiddyRefresh(r);

    }

  }

  else if (biddyManagerType == BIDDYTYPEZBDDC) {

    /* FOR ZBDD AND ZBDDC, RECURSIVE CALLS ARE VIA 'XOR' AND THUS ITS CACHE TABLE IS USED */

    v = BiddyV(biddyOne); /* biddyOne includes all variables */

    /* DETERMINING PARAMETERS FOR RECURSIVE CALLS */
    /* COMPLEMENTED EDGES MUST BE TRANSFERED */
    if (BiddyV(f) == v) {
      Fneg_v = Biddy_InvCond(BiddyE(f),Biddy_GetMark(f));
      Fv = BiddyT(f);
    } else {
      Fneg_v = f;
      Fv = biddyZero;
    }
    Gneg_v = BiddyE(biddyOne); /* biddyOne does not have complement */
    Gv = BiddyT(biddyOne);

    /* RECURSIVE CALLS */
    if (Fneg_v == biddyZero) {
      E = Gneg_v;
    } else if (Gneg_v == biddyZero) {
      E = Fneg_v;
    } else  {
      E = BiddyManagedXor(MNG,Fneg_v,Gneg_v);
    }
    if (Fv == biddyZero) {
      T = Gv;
    } else if (Gv == biddyZero) {
      T = Fv;
    } else  {
      T = BiddyManagedXor(MNG,Fv,Gv);
    }

    r = BiddyManagedTaggedFoaNode(MNG,v,E,T,0,TRUE);
    BiddyRefresh(r); /* FoaNode returns an obsolete node! */

  }

  else if (biddyManagerType == BIDDYTYPETZBDD) {

    /* FOR TZBDD, 'NOT' USES CACHE TABLE DIRECTLY */
    /* THIS IS NOT ITE CACHE! */

    FF = f;
    GG = biddyZero;
    HH = biddyOne;

    /* IF RESULT IS NOT IN THE CACHE TABLE... */
    cindex = 0;
    if (!findOp3Cache(MNG,biddyOPCache,FF,GG,HH,&r,&cindex))
    {

      v = BiddyV(f);
      nt = Biddy_GetTag(f);
      if (!v) {
        r = biddyZero;
      } else {
        Fneg_v = BiddyE(f);
        Fv = BiddyT(f);

        /* RECURSIVE CALLS */
        if (Fneg_v == biddyZero) {
          E = biddyOne;
        } else if (Fneg_v == biddyOne) {
          E = biddyZero;
        } else  {
          E = BiddyManagedNot(MNG,Fneg_v);
        }
        if (Fv == biddyZero) {
          T = biddyOne;
        } else if (Fv == biddyOne) {
          T = biddyZero;
        } else  {
          T = BiddyManagedNot(MNG,Fv);
        }

        r = BiddyManagedTaggedFoaNode(MNG,v,E,T,v,TRUE);
        BiddyRefresh(r); /* FoaNode returns an obsolete node! */
      }

      while (v != nt) {
        v = biddyVariableTable.table[v].prev;
        r = BiddyManagedTaggedFoaNode(MNG,v,r,biddyOne,v,TRUE);
        BiddyRefresh(r); /* FoaNode returns an obsolete node! */
      }

      addOp3Cache(MNG,biddyOPCache,FF,GG,HH,r,cindex);

    } else {

      /* IF THE RESULT IS FROM CACHE TABLE, REFRESH IT! */
      BiddyRefresh(r);

    }

  }

  return r;
}

/***************************************************************************//*!
\brief Function Biddy_Managed_ITE calculates ITE operation of three Boolean
       functions.

### Description
### Side Effects
    Implemented for OBDD, OBDDC, ZBDDC, and TZBDD.
    For OBDDC, results are cached as parameters to ITE(F,G,H)= F*G XOR F'*H.
    For all other BDD types, results are cached as (f,g,h) where
    f,g,h != 0, f != g, f != h, and g != h.
### More info
    Macro Biddy_ITE(f,g,h) is defined for use with anonymous manager.
*******************************************************************************/

#ifdef __cplusplus
extern "C" {
#endif

Biddy_Edge
Biddy_Managed_ITE(Biddy_Manager MNG, Biddy_Edge f, Biddy_Edge g, Biddy_Edge h)
{
  Biddy_Edge r;

  assert( f != NULL );
  assert( g != NULL );
  assert( h != NULL );

  if (!MNG) MNG = biddyAnonymousManager;
  ZF_LOGI("Biddy_ITE");

  assert( BiddyIsOK(f) == TRUE );
  assert( BiddyIsOK(g) == TRUE );
  assert( BiddyIsOK(h) == TRUE );

  biddyNodeTable.funite++;

  r = biddyNull;

  if (biddyManagerType == BIDDYTYPEOBDD) {
    /* IMPLEMENTED */
    r = BiddyManagedITE(MNG,f,g,h);
    BiddyRefresh(r); /* not always refreshed by BiddyManagedITE */
  } else if (biddyManagerType == BIDDYTYPEOBDDC) {
    /* IMPLEMENTED */
    r = BiddyManagedITE(MNG,f,g,h);
    BiddyRefresh(r); /* not always refreshed by BiddyManagedITE */
  } else if (biddyManagerType == BIDDYTYPEZBDDC) {
    /* IMPLEMENTED */
    r = BiddyManagedITE(MNG,f,g,h);
    BiddyRefresh(r); /* not always refreshed by BiddyManagedITE */
  } else if (biddyManagerType == BIDDYTYPETZBDD) {
    /* IMPLEMENTED */
    r = BiddyManagedITE(MNG,f,g,h);
    BiddyRefresh(r); /* not always refreshed by BiddyManagedITE */
  } else if ((biddyManagerType == BIDDYTYPEZBDD) ||
              (biddyManagerType == BIDDYTYPETZBDDC))
  {
    fprintf(stderr,"Biddy_ITE: this GDD type is not supported, yet!\n");
    return biddyNull;
  } else if ((biddyManagerType == BIDDYTYPEOFDDC) || (biddyManagerType == BIDDYTYPEOFDD) ||
              (biddyManagerType == BIDDYTYPEZFDDC) || (biddyManagerType == BIDDYTYPEZFDD) ||
              (biddyManagerType == BIDDYTYPETZFDDC) || (biddyManagerType == BIDDYTYPETZFDD))
  {
    fprintf(stderr,"Biddy_ITE: this GDD type is not supported, yet!\n");
    return biddyNull;
  } else {
    fprintf(stderr,"Biddy_ITE: Unsupported GDD type!\n");
    return biddyNull;
  }

  return r;
}

#ifdef __cplusplus
}
#endif

Biddy_Edge
BiddyManagedITE(const Biddy_Manager MNG, Biddy_Edge f, Biddy_Edge g, Biddy_Edge h)
{
  Biddy_Edge r, T, E, Fv, Gv, Hv, Fneg_v, Gneg_v, Hneg_v;
  Biddy_Edge FF, GG, HH;
  Biddy_Boolean NN;
  Biddy_Variable v,rtag;
  unsigned cindex;

  static Biddy_Variable topF;    /* CAN BE STATIC, WHAT IS BETTER? */
  static Biddy_Variable topG;    /* CAN BE STATIC, WHAT IS BETTER? */
  static Biddy_Variable topH;    /* CAN BE STATIC, WHAT IS BETTER? */
  static Biddy_Variable tagF;    /* CAN BE STATIC, WHAT IS BETTER? */
  static Biddy_Variable tagG;    /* CAN BE STATIC, WHAT IS BETTER? */
  static Biddy_Variable tagH;    /* CAN BE STATIC, WHAT IS BETTER? */

  /* DEBUGGING */
  /*
  static int mm = 0;
  int nn;
  nn = ++mm;
  fprintf(stdout,"Biddy_Managed_ITE (%d) F\n",nn);
  writeBDD(MNG,f);
  fprintf(stdout,"\n");
  fprintf(stdout,"Biddy_Managed_ITE (%d) G\n",nn);
  writeBDD(MNG,g);
  fprintf(stdout,"\n");
  fprintf(stdout,"Biddy_Managed_ITE (%d) H\n",nn);
  writeBDD(MNG,h);
  fprintf(stdout,"\n");
  mm = nn;
  */

  assert( MNG != NULL );
  assert( f != NULL );
  assert( g != NULL );
  assert( h != NULL );

  /* IMPLEMENTED FOR OBDD, OBDDC, ZBDDC, AND TZBDD */
  assert(
    (biddyManagerType == BIDDYTYPEOBDD) ||
    (biddyManagerType == BIDDYTYPEOBDDC) ||
    (biddyManagerType == BIDDYTYPEZBDDC) ||
    (biddyManagerType == BIDDYTYPETZBDD)
  );

#ifdef BIDDYEXTENDEDSTATS_YES
  biddyNodeTable.iterecursive++;
#endif

  r = biddyNull;

  /* LOOKING FOR SIMPLE CASE */
  /* using Not/Leq and returning biddyOne in a recursive call is wrong for ZBDD and ZBDDC, */
  /* because top variable of the result may be lower (topmore) than top variables of the arguments */
  /* (i.e. because the result depends on domain) */

  if (f == biddyOne) {
    return g;
  } else if (f == biddyZero) {
    return h;
  } else if (g == biddyOne) {
    if (h == biddyOne) {
      /* FOR ZBDD AND ZBDDC THIS LINE IS NOT REACHABLE IN RECURSIVE CALLS */
      return biddyOne;
    } else if (h == biddyZero) {
      return f;
    } else {
      return BiddyManagedOr(MNG,f,h);
    }
  } else if (g == biddyZero) {
    if (h == biddyOne) {
      /* FOR ZBDD AND ZBDDC THIS LINE IS NOT REACHABLE IN RECURSIVE CALLS */
      return BiddyManagedNot(MNG,f);
    } else if (h == biddyZero) {
      return biddyZero;
    } else {
      return BiddyManagedGt(MNG,h,f);
    }
  } else if (h == biddyOne) {
    if ((biddyManagerType != BIDDYTYPEZBDDC) && (biddyManagerType == BIDDYTYPEZBDD)) {
      /* FOR ZBDD AND ZBDDC THIS LINE IS NOT REACHABLE IN RECURSIVE CALLS */
      /* FOR ZBDD AND ZBDDC LEQ CANNOT BE RESOLVED BY LEQ BECAUSE LEQ IS RESOLVED BY ITE */
      return BiddyManagedLeq(MNG,f,g);
    }
  } else if (h == biddyZero) {
    return BiddyManagedAnd(MNG,f,g);
  } else if (g == h) {
    return g;
  } else if (f == g) {
    return BiddyManagedOr(MNG,f,h);
  } else if (f == h) {
    return BiddyManagedAnd(MNG,f,g);
  }

  if (biddyManagerType == BIDDYTYPEOBDDC) {
    if (Biddy_IsEqvPointer(g,h)) {
      return BiddyManagedXor(MNG,f,h);
    } else if (Biddy_IsEqvPointer(f,g)) {
      return BiddyManagedAnd(MNG,g,h);
    } else if (Biddy_IsEqvPointer(f,h)) {
      return BiddyManagedOr(MNG,g,h);
    }
  }

  /* THIS IS NOT A SIMPLE CASE */

  FF = GG = HH = biddyNull;
  NN = FALSE;
  Fneg_v = Fv = biddyNull;
  Gneg_v = Gv = biddyNull;
  Hneg_v = Hv = biddyNull;
  rtag = 0;
  v = 0;

  /* NORMALIZATION OF COMPLEMENTED EDGES */
  /* APLICABLE ONLY TO OBDDC */
  /* FF, GG, HH, and NN ARE USED FOR CACHE LOOKUP, ONLY */

  if (biddyManagerType == BIDDYTYPEOBDDC) {
    if (Biddy_GetMark(f)) {
      if (Biddy_GetMark(h)) {
        NN = TRUE;
        FF = Biddy_Inv(f);
        GG = Biddy_Inv(h);
        HH = Biddy_Inv(g);
      } else {
        FF = Biddy_Inv(f);
        GG = h;
        HH = g;
      }
    } else if (Biddy_GetMark(g)) {
      NN = TRUE;
      FF = f;
      GG = Biddy_Inv(g);
      HH = Biddy_Inv(h);
    } else {
      FF = f;
      GG = g;
      HH = h;
    }
  }

  else {

    /* IF NOT OBDDC THEN ITE CACHE IS NOT SHARED WITH OTHER OPERATIONS! */

    FF = f;
    GG = g;
    HH = h;
  }

  /* IF RESULT IS NOT IN THE CACHE TABLE... */
  cindex = 0;
  if (!findOp3Cache(MNG,biddyOPCache,FF,GG,HH,&r,&cindex))
  {

    if ((biddyManagerType == BIDDYTYPEOBDDC) || (biddyManagerType == BIDDYTYPEOBDD)) {

      /* LOOKING FOR THE SMALLEST TOP VARIABLE */
      topF = BiddyV(f);
      topG = BiddyV(g);
      topH = BiddyV(h);

      /* CONSTANT VARIABLE MUST HAVE MAX ORDER */
      if (BiddyIsSmaller(topF,topG)) {
        v = BiddyIsSmaller(topF,topH) ? topF : topH;
      } else {
        v = BiddyIsSmaller(topH,topG) ? topH : topG;
      }

      /* DETERMINING PARAMETERS FOR RECURSIVE CALLS */
      if (topF == v) {
        Fneg_v = Biddy_InvCond(BiddyE(f),Biddy_GetMark(f));
        Fv = Biddy_InvCond(BiddyT(f),Biddy_GetMark(f));
      } else {
        Fneg_v = Fv = f;
      }
      if (topG == v) {
        Gneg_v = Biddy_InvCond(BiddyE(g),Biddy_GetMark(g));
        Gv = Biddy_InvCond(BiddyT(g),Biddy_GetMark(g));
      } else {
        Gneg_v = Gv = g;
      }
      if (topH == v) {
        Hneg_v = Biddy_InvCond(BiddyE(h),Biddy_GetMark(h));
        Hv = Biddy_InvCond(BiddyT(h),Biddy_GetMark(h));
      } else {
        Hneg_v = Hv = h;
      }

      rtag = v;

    }

    else if (biddyManagerType == BIDDYTYPEZBDDC) {

      /* LOOKING FOR THE SMALLEST TOP VARIABLE */
      topF = BiddyV(f);
      topG = BiddyV(g);
      topH = BiddyV(h);

      /* CONSTANT VARIABLE MUST HAVE MAX ORDER */
      if (BiddyIsSmaller(topF,topG)) {
        v = BiddyIsSmaller(topF,topH) ? topF : topH;
      } else {
        v = BiddyIsSmaller(topH,topG) ? topH : topG;
      }

      /* DETERMINING PARAMETERS FOR RECURSIVE CALLS */
      if (topF == v) {
        Fneg_v = Biddy_InvCond(BiddyE(f),Biddy_GetMark(f));
        Fv = BiddyT(f);
      } else {
        Fv = biddyZero;
        Fneg_v = f;
      }
      if (topG == v) {
        Gneg_v = Biddy_InvCond(BiddyE(g),Biddy_GetMark(g));
        Gv = BiddyT(g);
      } else {
        Gv = biddyZero;
        Gneg_v = g;
      }
      if (topH == v) {
        Hneg_v = Biddy_InvCond(BiddyE(h),Biddy_GetMark(h));
        Hv = BiddyT(h);
      } else {
        Hv = biddyZero;
        Hneg_v = h;
      }

      rtag = 0;

    }

    else if (biddyManagerType == BIDDYTYPETZBDD) {

      /* LOOKING FOR THE SMALLEST TAG AND THE SMALLEST TOP VARIABLE */
      /* CONSTANT VARIABLE MUST HAVE MAX ORDER */
      tagF = Biddy_GetTag(f);
      tagG = Biddy_GetTag(g);
      tagH = Biddy_GetTag(h);
      topF = BiddyV(f);
      topG = BiddyV(g);
      topH = BiddyV(h);

      /* CONSTANT VARIABLE MUST HAVE MAX ORDER */
      if (BiddyIsSmaller(tagF,tagG)) {
        rtag = BiddyIsSmaller(tagF,tagH) ? tagF : tagH;
      } else {
        rtag = BiddyIsSmaller(tagH,tagG) ? tagH : tagG;
      }
      if (BiddyIsSmaller(topF,topG)) {
        v = BiddyIsSmaller(topF,topH) ? topF : topH;
      } else {
        v = BiddyIsSmaller(topH,topG) ? topH : topG;
      }

      if ((tagF != tagG) || (tagF != tagH)) v = rtag;

      if (tagF == rtag) {
        if (topF == v) {
          Fneg_v = BiddyE(f);
          Fv = BiddyT(f);
        } else {
          Fneg_v = f;
          Biddy_SetTag(Fneg_v,v);
          Fneg_v = Biddy_Managed_IncTag(MNG,Fneg_v);
          Fv = biddyZero;
        }
      } else {
        Fneg_v = Fv = f;
      }
      if (tagG == rtag) {
        if (topG == v) {
          Gneg_v = BiddyE(g);
          Gv = BiddyT(g);
        } else {
          Gneg_v = g;
          Biddy_SetTag(Gneg_v,v);
          Gneg_v = Biddy_Managed_IncTag(MNG,Gneg_v);
          Gv = biddyZero;
        }
      } else {
        Gneg_v = Gv = g;
      }
      if (tagH == rtag) {
        if (topH == v) {
          Hneg_v = BiddyE(h);
          Hv = BiddyT(h);
        } else {
          Hneg_v = h;
          Biddy_SetTag(Hneg_v,v);
          Hneg_v = Biddy_Managed_IncTag(MNG,Hneg_v);
          Hv = biddyZero;
        }
      } else {
        Hneg_v = Hv = h;
      }

    }

    /* RECURSIVE CALLS */
    if (Fneg_v == biddyOne) {
      E = Gneg_v;
    } else if (Fneg_v == biddyZero) {
      E = Hneg_v;
    } else if (Gneg_v == Hneg_v) {
      E = Gneg_v;
    } else {
      E = BiddyManagedITE(MNG,Fneg_v,Gneg_v,Hneg_v);
    }
    if (Fv == biddyOne) {
      T = Gv;
    } else if (Fv == biddyZero) {
      T = Hv;
    } else if (Gv == Hv) {
      T = Gv;
    } else {
      T = BiddyManagedITE(MNG,Fv,Gv,Hv);
    }

    r = BiddyManagedTaggedFoaNode(MNG,v,E,T,rtag,TRUE);
    BiddyRefresh(r); /* FoaNode returns an obsolete node! */

    /* CACHE EVERYTHING */
    if (NN) {
      addOp3Cache(MNG,biddyOPCache,FF,GG,HH,Biddy_Inv(r),cindex);
    } else {
      addOp3Cache(MNG,biddyOPCache,FF,GG,HH,r,cindex);
    }

    /* DEBUGGING */
    /*
    printf("ITE has not used cache\n");
    */

  } else {

    if (NN) {
      Biddy_InvertMark(r);
    }

    /* IF THE RESULT IS FROM CACHE TABLE, REFRESH IT! */
    BiddyRefresh(r);

  }

  /* DEBUGGING */
  /*
  fprintf(stdout,"Biddy_ITE (%d) R\n",nn);
  writeBDD(MNG,r);
  */

  return r;
}

/***************************************************************************//*!
\brief Function Biddy_Managed_And calculates Boolean function AND
       (conjunction).

### Description
    For combination sets, this function coincides with Intersection.
### Side Effects
    Used by ITE (for OBDD).
    Implemented for OBDD, OBDDC, ZBDDC, and TZBDD.
    For OBDDC, results are cached as parameters to ITE(F,G,H)= F*G XOR F'*H.
    For all other BDD types, results are cached as (f,g,biddyZero).
### More Info
    Macro Biddy_And(f,g) is defined for use with anonymous manager.
    Macros Biddy_Managed_Intersect(MNG,f,g) and Biddy_Intersect(f,g) are
    defined for manipulation of combination sets.
*******************************************************************************/

#ifdef __cplusplus
extern "C" {
#endif

Biddy_Edge
Biddy_Managed_And(Biddy_Manager MNG, Biddy_Edge f, Biddy_Edge g)
{
  Biddy_Edge r;

  assert( f != NULL );
  assert( g != NULL );

  if (!MNG) MNG = biddyAnonymousManager;
  ZF_LOGI("Biddy_And");

  assert( BiddyIsOK(f) == TRUE );
  assert( BiddyIsOK(g) == TRUE );

  biddyNodeTable.funandor++;

  r = biddyNull;

  if (biddyManagerType == BIDDYTYPEOBDD) {
    /* IMPLEMENTED */
    r = BiddyManagedAnd(MNG,f,g);
    BiddyRefresh(r); /* not always refreshed by BiddyManagedAnd */
  } else if (biddyManagerType == BIDDYTYPEOBDDC) {
    /* IMPLEMENTED */
    r = BiddyManagedAnd(MNG,f,g);
    BiddyRefresh(r); /* not always refreshed by BiddyManagedAnd */
  } else if (biddyManagerType == BIDDYTYPEZBDDC) {
    /* IMPLEMENTED */
    r = BiddyManagedAnd(MNG,f,g);
    BiddyRefresh(r); /* not always refreshed by BiddyManagedAnd */
  } else if (biddyManagerType == BIDDYTYPETZBDD) {
    /* IMPLEMENTED */
    r = BiddyManagedAnd(MNG,f,g);
    BiddyRefresh(r); /* not always refreshed by BiddyManagedAnd */
  } else if ((biddyManagerType == BIDDYTYPEZBDD) ||
              (biddyManagerType == BIDDYTYPETZBDDC))
  {
    fprintf(stderr,"Biddy_And: this GDD type is not supported, yet!\n");
    return biddyNull;
  } else if ((biddyManagerType == BIDDYTYPEOFDDC) || (biddyManagerType == BIDDYTYPEOFDD) ||
              (biddyManagerType == BIDDYTYPEZFDDC) || (biddyManagerType == BIDDYTYPEZFDD) ||
              (biddyManagerType == BIDDYTYPETZFDDC) || (biddyManagerType == BIDDYTYPETZFDD))
  {
    fprintf(stderr,"Biddy_And: this GDD type is not supported, yet!\n");
    return biddyNull;
  } else {
    fprintf(stderr,"Biddy_And: Unsupported GDD type!\n");
    return biddyNull;
  }

  return r;
}

#ifdef __cplusplus
}
#endif

Biddy_Edge
BiddyManagedAnd(const Biddy_Manager MNG, const Biddy_Edge f, const Biddy_Edge g)
{
  Biddy_Edge r, T, E, Fv, Gv, Fneg_v, Gneg_v;
  Biddy_Edge FF, GG, HH;
  Biddy_Boolean NN;
  Biddy_Variable v,rtag;
  unsigned int cindex;

  static Biddy_Variable topF;    /* CAN BE STATIC, WHAT IS BETTER? */
  static Biddy_Variable topG;    /* CAN BE STATIC, WHAT IS BETTER? */
  static Biddy_Variable tagF;    /* CAN BE STATIC, WHAT IS BETTER? */
  static Biddy_Variable tagG;    /* CAN BE STATIC, WHAT IS BETTER? */

  assert( MNG != NULL );
  assert( f != NULL );
  assert( g != NULL );

  /* IMPLEMENTED FOR OBDD, OBDDC, ZBDDC, AND TZBDD */
  assert(
    (biddyManagerType == BIDDYTYPEOBDD) ||
    (biddyManagerType == BIDDYTYPEOBDDC) ||
    (biddyManagerType == BIDDYTYPEZBDDC) ||
    (biddyManagerType == BIDDYTYPETZBDD)
  );

#ifdef BIDDYEXTENDEDSTATS_YES
  biddyNodeTable.andorrecursive++;
#endif

  /* LOOKING FOR SIMPLE CASE */
  if (f == biddyZero) {
    return biddyZero;
  } else if (g == biddyZero) {
    return biddyZero;
  } else if (f == biddyOne) {
    return g;
  } else if (g == biddyOne) {
    return f;
  } else if (f == g) {
    return f;
  }

  if (biddyManagerType == BIDDYTYPEOBDDC) {
    if (Biddy_IsEqvPointer(f,g)) {
      return biddyZero;
    }
  }

  else if (biddyManagerType == BIDDYTYPETZBDD) {
    if (BiddyR(f) == BiddyR(g)) {
      if (BiddyIsSmaller(Biddy_GetTag(f),Biddy_GetTag(g))) {
        return f;
      } else {
        return g;
      }
    }
  }

  /* THIS IS NOT A SIMPLE CASE */

  FF = GG = HH = biddyNull;
  NN = FALSE;
  Fneg_v = Fv = biddyNull;
  Gneg_v = Gv = biddyNull;
  rtag = 0;
  v = 0;

  /* NORMALIZATION OF COMPLEMENTED EDGES */
  /* FF, GG, and HH ARE USED FOR CACHE LOOKUP, ONLY */
  /* FF AND GG SHOULD NOT BE COMPLEMENTED */
  /* ((uintptr_t) f) SHOULD BE GREATER THAN ((uintptr_t) g) */

  if (biddyManagerType == BIDDYTYPEOBDDC) {

    if (((uintptr_t) f) > ((uintptr_t) g)) {
      if (Biddy_GetMark(f)) {
        NN = TRUE;
        FF = Biddy_Inv(f);
        GG = biddyOne; /* Biddy_Inv(h) */
        HH = Biddy_Inv(g);
      } else {
        if (Biddy_GetMark(g)) {
          NN = TRUE;
          FF = f;
          GG = Biddy_Inv(g);
          HH = biddyOne; /* Biddy_Inv(h) */
        } else {
          FF = f;
          GG = g;
          HH = biddyZero; /* h */
        }
      }
    } else {
      if (Biddy_GetMark(g)) {
        NN = TRUE;
        FF = Biddy_Inv(g);
        GG = biddyOne; /* Biddy_Inv(h) */
        HH = Biddy_Inv(f);
      } else {
        if (Biddy_GetMark(f)) {
          NN = TRUE;
          FF = g;
          GG = Biddy_Inv(f);
          HH = biddyOne; /* Biddy_Inv(h) */
        } else {
          FF = g;
          GG = f;
          HH = biddyZero; /* h */
        }
      }
    }

  }

  else {

    /* THIS IS NOT ITE CACHE! */

    if (((uintptr_t) f) > ((uintptr_t) g)) {
      FF = f;
      GG = g;
      HH = biddyZero;
    } else {
      FF = g;
      GG = f;
      HH = biddyZero;
    }

  }

  /* IF RESULT IS NOT IN THE CACHE TABLE... */
  /* FOR ZBDDs, NOT EVERYTHING IS CACHED */
  cindex = 0;
  if (((biddyManagerType == BIDDYTYPEZBDDC) && ((topF = BiddyV(f)) != (topG = BiddyV(g))))
      || !findOp3Cache(MNG,biddyOPCache,FF,GG,HH,&r,&cindex))
  {

    if ((biddyManagerType == BIDDYTYPEOBDDC) || (biddyManagerType == BIDDYTYPEOBDD)) {

      /* LOOKING FOR THE SMALLEST TOP VARIABLE */
      /* CONSTANT VARIABLE MUST HAVE MAX ORDER */
      topF = BiddyV(f);
      topG = BiddyV(g);
      v = BiddyIsSmaller(topF,topG) ? topF : topG;

      /* DETERMINING PARAMETERS FOR RECURSIVE CALLS */
      /* COMPLEMENTED EDGES MUST BE TRANSFERED */

      if (topF == v) {
        Fneg_v = Biddy_InvCond(BiddyE(f),Biddy_GetMark(f));
        Fv = Biddy_InvCond(BiddyT(f),Biddy_GetMark(f));
      } else {
        Fneg_v = Fv = f;
      }

      if (topG == v) {
        Gneg_v = Biddy_InvCond(BiddyE(g),Biddy_GetMark(g));
        Gv = Biddy_InvCond(BiddyT(g),Biddy_GetMark(g));
      } else {
        Gneg_v = Gv = g;
      }

      rtag = v;

    }

    else if (biddyManagerType == BIDDYTYPEZBDDC) {

      /* LOOKING FOR THE SMALLEST TOP VARIABLE */
      /* CONSTANT VARIABLE MUST HAVE MAX ORDER */
      /* FOR ZBDD, topF AND topG HAVE BEEN ALREADY CALCULATED */
      /* topF = BiddyV(f); */ /* ALREADY CALCULATED */
      /* topG = BiddyV(g); */ /* ALREADY CALCULATED */
      v = BiddyIsSmaller(topF,topG) ? topF : topG;

      /* THIS CALL IS NOT INTRODUCING NEW NODES */
      /* COMPLEMENTED EDGES MUST BE TRANSFERED */
      /* IT SEEMS THAT CACHING THIS RESULTS IS NOT A GOOD IDEA */
      if (topF != topG) {
        if (topF == v) {
          r = BiddyManagedAnd(MNG,Biddy_InvCond(BiddyE(f),Biddy_GetMark(f)),g);
          return r;
        } else {
          r = BiddyManagedAnd(MNG,f,Biddy_InvCond(BiddyE(g),Biddy_GetMark(g)));
          return r;
        }
      }

      /* DETERMINING PARAMETERS FOR RECURSIVE CALLS, topF == topG */
      /* COMPLEMENTED EDGES MUST BE TRANSFERED */
      Fneg_v = Biddy_InvCond(BiddyE(f),Biddy_GetMark(f));
      Fv = BiddyT(f);
      Gneg_v = Biddy_InvCond(BiddyE(g),Biddy_GetMark(g));
      Gv = BiddyT(g);

      rtag = 0;

    }

    else if (biddyManagerType == BIDDYTYPETZBDD) {

      /* LOOKING FOR THE SMALLEST TAG AND SMALLEST TOP VARIABLE */
      /* CONSTANT VARIABLE MUST HAVE MAX ORDER */
      tagF = Biddy_GetTag(f);
      tagG = Biddy_GetTag(g);
      topF = BiddyV(f);
      topG = BiddyV(g);
      rtag = BiddyIsSmaller(tagF,tagG) ? tagF : tagG;
      v = BiddyIsSmaller(topF,topG) ? topF : topG;

      /* VARIANT 1 */
      /*
      if (BiddyIsSmaller(topF,tagG)) {
        Fneg_v = BiddyE(f);
        Fv = BiddyT(f);
        Gneg_v = Gv = g;
      } else if (BiddyIsSmaller(topG,tagF)) {
        Fneg_v = Fv = f;
        Gneg_v = BiddyE(g);
        Gv = BiddyT(g);
      } else {
        if (topF == v) {
          Fneg_v = BiddyE(f);
          Fv = BiddyT(f);
        } else {
          Fneg_v = f;
          Biddy_SetTag(Fneg_v,v);
          Fneg_v = Biddy_Managed_IncTag(MNG,Fneg_v);
          Fv = biddyZero;
        }
        if (topG == v) {
          Gneg_v = BiddyE(g);
          Gv = BiddyT(g);
        } else {
          Gneg_v = g;
          Biddy_SetTag(Gneg_v,v);
          Gneg_v = Biddy_Managed_IncTag(MNG,Gneg_v);
          Gv = biddyZero;
        }
      }
      */

      /* VARIANT 2 */
      /*
      if (topF == v) {
        Fneg_v = BiddyE(f);
        Fv = BiddyT(f);
      } else {
        if (BiddyIsSmaller(v,tagF)) {
          Fneg_v = Fv = f;
        } else {
          Fneg_v = f;
          Biddy_SetTag(Fneg_v,v);
          Fneg_v = Biddy_Managed_IncTag(MNG,Fneg_v);
          Fv = biddyZero;
        }
      }
      if (topG == v) {
        Gneg_v = BiddyE(g);
        Gv = BiddyT(g);
      } else {
        if (BiddyIsSmaller(v,tagG)) {
          Gneg_v = Gv = g;
        } else {
          Gneg_v = g;
          Biddy_SetTag(Gneg_v,v);
          Gneg_v = Biddy_Managed_IncTag(MNG,Gneg_v);
          Gv = biddyZero;
        }
      }
      */

      /* VARIANT 3 */
      /**/
      if (BiddyIsSmaller(v,tagF)) {
        Fneg_v = Fv = f;
      } else if (v == topF) {
        Fneg_v = BiddyE(f);
        Fv = BiddyT(f);
      } else {
        Fneg_v = f;
        Biddy_SetTag(Fneg_v,v);
        Fneg_v = Biddy_Managed_IncTag(MNG,Fneg_v);
        Fv = biddyZero;
      }
      if (BiddyIsSmaller(v,tagG)) {
        Gneg_v = Gv = g;
      } else if (v == topG) {
        Gneg_v = BiddyE(g);
        Gv = BiddyT(g);
      } else {
        Gneg_v = g;
        Biddy_SetTag(Gneg_v,v);
        Gneg_v = Biddy_Managed_IncTag(MNG,Gneg_v);
        Gv = biddyZero;
      }
      /**/

    }

    /* RECURSIVE CALLS */
    if (Fneg_v == biddyZero) {
      E = biddyZero;
    } else if (Gneg_v == biddyZero) {
      E = biddyZero;
    } else if (Fneg_v == biddyOne) {
      E = Gneg_v;
    } else if (Gneg_v == biddyOne) {
      E = Fneg_v;
    } else  {
      E = BiddyManagedAnd(MNG,Fneg_v,Gneg_v);
    }
    if (Fv == biddyZero) {
      T = biddyZero;
    } else if (Gv == biddyZero) {
      T = biddyZero;
    } else if (Fv == biddyOne) {
      T = Gv;
    } else if (Gv == biddyOne) {
      T = Fv;
    } else  {
      T = BiddyManagedAnd(MNG,Fv,Gv);
    }

    r = BiddyManagedTaggedFoaNode(MNG,v,E,T,rtag,TRUE);
    BiddyRefresh(r); /* FoaNode returns an obsolete node! */

    if (NN) {
      addOp3Cache(MNG,biddyOPCache,FF,GG,HH,Biddy_Inv(r),cindex);
    } else {
      addOp3Cache(MNG,biddyOPCache,FF,GG,HH,r,cindex);
    }

  } else {

    if (NN) {
      Biddy_InvertMark(r);
    }

    /* IF THE RESULT IS FROM CACHE TABLE, REFRESH IT! */
    BiddyRefresh(r);

  }

  return r;

}

/***************************************************************************//*!
\brief Function Biddy_Managed_Or calculates Boolean function OR
       (disjunction).

### Description
    For combination sets, this function coincides with Union.
### Side Effects
    Implemented for OBDD, OBDDC, ZBDDC, and TZBDD.
    For OBDDC, results are cached as parameters to ITE(F,G,H)= F*G XOR F'*H.
    For all other BDD types, results are cached as (biddyZero,f,g).
### More Info
    Macro Biddy_Or(f,g) is defined for use with anonymous manager.
    Macros Biddy_Managed_Union(MNG,f,g) and Biddy_Union(f,g) are defined
    for manipulation of combination sets.
*******************************************************************************/

#ifdef __cplusplus
extern "C" {
#endif

Biddy_Edge
Biddy_Managed_Or(Biddy_Manager MNG, Biddy_Edge f, Biddy_Edge g)
{
  Biddy_Edge r;

  assert( f != NULL );
  assert( g != NULL );

  if (!MNG) MNG = biddyAnonymousManager;
  ZF_LOGI("Biddy_Or");

  assert( BiddyIsOK(f) == TRUE );
  assert( BiddyIsOK(g) == TRUE );

  biddyNodeTable.funandor++;

  r = biddyNull;

  if (biddyManagerType == BIDDYTYPEOBDD) {
    /* PROCEED BY CALCULATING NOT-AND */
    /*
    r = BiddyManagedNot(MNG,BiddyManagedAnd(MNG,BiddyManagedNot(MNG,f),BiddyManagedNot(MNG,g)));
    */
    /* PROCEED BY CALCULATING XOR-AND */
    /*
    r = BiddyManagedXor(MNG,BiddyManagedXor(MNG,f,g),BiddyManagedAnd(MNG,f,g));
    */
    /* IMPLEMENTED */
    r = BiddyManagedOr(MNG,f,g);
    BiddyRefresh(r); /* not always refreshed by BiddyManagedOr */
  } else if (biddyManagerType == BIDDYTYPEOBDDC) {
    /* PROCEED BY CALCULATING NOT-AND */
    /*
    r = Biddy_Inv(BiddyManagedAnd(MNG,Biddy_Inv(f),Biddy_Inv(g)));
    */
    /* PROCEED BY CALCULATING XOR-AND */
    /*
    r = BiddyManagedXor(MNG,BiddyManagedXor(MNG,f,g),BiddyManagedAnd(MNG,f,g));
    */
    /* IMPLEMENTED */
    r = BiddyManagedOr(MNG,f,g);
    BiddyRefresh(r); /* not always refreshed by BiddyManagedOr */
  } else if (biddyManagerType == BIDDYTYPEZBDDC) {
    /* PROCEED BY CALCULATING NOT-AND */
    /*
    r = BiddyManagedNot(MNG,BiddyManagedAnd(MNG,BiddyManagedNot(MNG,f),BiddyManagedNot(MNG,g)));
    */
    /* PROCEED BY CALCULATING XOR-AND */
    /*
    r = BiddyManagedXor(MNG,BiddyManagedXor(MNG,f,g),BiddyManagedAnd(MNG,f,g));
    */
    /* IMPLEMENTED */
    r = BiddyManagedOr(MNG,f,g);
    BiddyRefresh(r); /* not always refreshed by BiddyManagedOr */
  } else if (biddyManagerType == BIDDYTYPETZBDD) {
    /* PROCEED BY CALCULATING NOT-AND */
    /*
    r = BiddyManagedNot(MNG,BiddyManagedAnd(MNG,BiddyManagedNot(MNG,f),BiddyManagedNot(MNG,g)));
    */
    /* PROCEED BY CALCULATING XOR-AND */
    /*
    r = BiddyManagedXor(MNG,BiddyManagedXor(MNG,f,g),BiddyManagedAnd(MNG,f,g));
    */
    /* IMPLEMENTED */
    r = BiddyManagedOr(MNG,f,g);
    BiddyRefresh(r); /* not always refreshed by BiddyManagedOr */
  } else if ((biddyManagerType == BIDDYTYPEZBDD) ||
              (biddyManagerType == BIDDYTYPETZBDDC))
  {
    fprintf(stderr,"Biddy_Or: this GDD type is not supported, yet!\n");
    return biddyNull;
  } else if ((biddyManagerType == BIDDYTYPEOFDDC) || (biddyManagerType == BIDDYTYPEOFDD) ||
              (biddyManagerType == BIDDYTYPEZFDDC) || (biddyManagerType == BIDDYTYPEZFDD) ||
              (biddyManagerType == BIDDYTYPETZFDDC) || (biddyManagerType == BIDDYTYPETZFDD))
  {
    fprintf(stderr,"Biddy_Or: this GDD type is not supported, yet!\n");
    return biddyNull;
  } else {
    fprintf(stderr,"Biddy_Or: Unsupported GDD type!\n");
    return biddyNull;
  }

  return r;
}

#ifdef __cplusplus
}
#endif

Biddy_Edge
BiddyManagedOr(const Biddy_Manager MNG, const Biddy_Edge f, const Biddy_Edge g)
{
  Biddy_Edge r, T, E, Fv, Gv, Fneg_v, Gneg_v;
  Biddy_Edge FF, GG, HH;
  Biddy_Boolean NN;
  Biddy_Variable v,rtag;
  unsigned int cindex;

  static Biddy_Variable topF;    /* CAN BE STATIC, WHAT IS BETTER? */
  static Biddy_Variable topG;    /* CAN BE STATIC, WHAT IS BETTER? */
  static Biddy_Variable tagF;    /* CAN BE STATIC, WHAT IS BETTER? */
  static Biddy_Variable tagG;    /* CAN BE STATIC, WHAT IS BETTER? */

  assert( MNG != NULL );
  assert( f != NULL );
  assert( g != NULL );

  /* IMPLEMENTED FOR OBDD, OBDDC, ZBDDC, AND TZBDD */
  assert(
    (biddyManagerType == BIDDYTYPEOBDD) ||
    (biddyManagerType == BIDDYTYPEOBDDC) ||
    (biddyManagerType == BIDDYTYPEZBDDC) ||
    (biddyManagerType == BIDDYTYPETZBDD)
  );

#ifdef BIDDYEXTENDEDSTATS_YES
  biddyNodeTable.andorrecursive++;
#endif

  r = biddyNull;

  /* LOOKING FOR SIMPLE CASE */
  /* returning biddyOne in a recursive call is wrong for ZBDD and ZBDDC, */
  /* because top variable of the result may be lower (topmore) than top variables of the arguments */
  /* (i.e. because the result depends on domain) */

  if (f == biddyZero) {
    return g;
  } else if (g == biddyZero) {
    return f;
  } else if (f == biddyOne) {
    /* FOR ZBDD AND ZBDDC THIS LINE IS NOT REACHABLE IN RECURSIVE CALLS */
    return biddyOne;
  } else if (g == biddyOne) {
    /* FOR ZBDD AND ZBDDC THIS LINE IS NOT REACHABLE IN RECURSIVE CALLS */
    return biddyOne;
  } else if (f == g) {
    return f;
  }

  if (biddyManagerType == BIDDYTYPEOBDDC) {
    if (Biddy_IsEqvPointer(f,g)) {
      return biddyOne;
    }
  }

  if (biddyManagerType == BIDDYTYPETZBDD) {
    if (BiddyR(f) == BiddyR(g)) {
      if (BiddyIsSmaller(Biddy_GetTag(f),Biddy_GetTag(g))) {
        return g;
      } else {
        return f;
      }
    }
  }

  /* THIS IS NOT A SIMPLE CASE */

  FF = GG = HH = biddyNull;
  NN = FALSE;
  Fneg_v = Fv = biddyNull;
  Gneg_v = Gv = biddyNull;
  rtag = 0;
  v = 0;

  /* NORMALIZATION OF COMPLEMENTED EDGES */
  /* FF, GG, and HH ARE USED FOR CACHE LOOKUP, ONLY */
  /* FF AND GG SHOULD NOT BE COMPLEMENTED */
  /* ((uintptr_t) f) SHOULD BE GREATER THAN ((uintptr_t) g) */

  if (biddyManagerType == BIDDYTYPEOBDDC) {

    if (((uintptr_t) f) > ((uintptr_t) g)) {
      if (Biddy_GetMark(f)) {
        if (Biddy_GetMark(g)) {
          NN = TRUE;
          FF = Biddy_Inv(f);
          GG = Biddy_Inv(g);
          HH = biddyZero;  /* Biddy_Inv(g) */
        } else {
          FF = Biddy_Inv(f);
          GG = g;
          HH = biddyOne; /* g */
        }
      } else {
        FF = f;
        GG = biddyOne;
        HH = g;
      }
    } else {
      if (Biddy_GetMark(g)) {
        if (Biddy_GetMark(f)) {
          NN = TRUE;
          FF = Biddy_Inv(g);
          GG = Biddy_Inv(f);
          HH = biddyZero;  /* Biddy_Inv(g) */
        } else {
          FF = Biddy_Inv(g);
          GG = f;
          HH = biddyOne; /* g */
        }
      } else {
        FF = g;
        GG = biddyOne;
        HH = f;
      }
    }
  }

  else {

    /* THIS IS NOT ITE CACHE! */

    if (((uintptr_t) f) > ((uintptr_t) g)) {
      FF = biddyZero;
      GG = f;
      HH = g;
    } else {
      FF = biddyZero;
      GG = g;
      HH = f;
    }

  }

  /* IF RESULT IS NOT IN THE CACHE TABLE... */
  cindex = 0;
  if (!findOp3Cache(MNG,biddyOPCache,FF,GG,HH,&r,&cindex))
  {

    if ((biddyManagerType == BIDDYTYPEOBDDC) || (biddyManagerType == BIDDYTYPEOBDD)) {

      /* LOOKING FOR THE SMALLEST TOP VARIABLE */
      /* CONSTANT VARIABLE MUST HAVE MAX ORDER */
      topF = BiddyV(f);
      topG = BiddyV(g);
      v = BiddyIsSmaller(topF,topG) ? topF : topG;

      /* DETERMINING PARAMETERS FOR RECURSIVE CALLS */
      /* COMPLEMENTED EDGES MUST BE TRANSFERED */
      if (topF == v) {
        Fneg_v = Biddy_InvCond(BiddyE(f),Biddy_GetMark(f));
        Fv = Biddy_InvCond(BiddyT(f),Biddy_GetMark(f));
      } else {
        Fneg_v = Fv = f;
      }

      if (topG == v) {
        Gneg_v = Biddy_InvCond(BiddyE(g),Biddy_GetMark(g));
        Gv = Biddy_InvCond(BiddyT(g),Biddy_GetMark(g));
      } else {
        Gneg_v = Gv = g;
      }

      rtag = v;

    }

    else if (biddyManagerType == BIDDYTYPEZBDDC) {

      /* LOOKING FOR THE SMALLEST TOP VARIABLE */
      /* CONSTANT VARIABLE MUST HAVE MAX ORDER */
      topF = BiddyV(f);
      topG = BiddyV(g);
      v = BiddyIsSmaller(topF,topG) ? topF : topG;

      /* DETERMINING PARAMETERS FOR RECURSIVE CALLS */
      /* COMPLEMENTED EDGES MUST BE TRANSFERED */
      if (topF == v) {
        Fneg_v = Biddy_InvCond(BiddyE(f),Biddy_GetMark(f));
        Fv = BiddyT(f);
      } else {
        Fneg_v = f;
        Fv = biddyZero;
      }
      if (topG == v) {
        Gneg_v = Biddy_InvCond(BiddyE(g),Biddy_GetMark(g));
        Gv = BiddyT(g);
      } else {
        Gneg_v = g;
        Gv = biddyZero;
      }

      rtag = 0;

    }

    else if (biddyManagerType == BIDDYTYPETZBDD) {

      /* VARIANT 1 */
      /*
      tagF = Biddy_GetTag(f);
      tagG = Biddy_GetTag(g);
      if (tagF == tagG) {
        / * THE SAME TAG * /
        topF = BiddyV(f);
        topG = BiddyV(g);
        v = BiddyIsSmaller(topF,topG) ? topF : topG;
        rtag = tagF;
        w = v;
        if (topF == v) {
          Fneg_v = BiddyE(f);
          Fv = BiddyT(f);
        } else {
          Fneg_v = f;
          Biddy_SetTag(Fneg_v,v);
          Fneg_v = Biddy_Managed_IncTag(MNG,Fneg_v);
          Fv = biddyZero;
        }
        if (topG == v) {
          Gneg_v = BiddyE(g);
          Gv = BiddyT(g);
        } else {
          Gneg_v = g;
          Biddy_SetTag(Gneg_v,v);
          Gneg_v = Biddy_Managed_IncTag(MNG,Gneg_v);
          Gv = biddyZero;
        }
      } else {
        / * DIFFERENT TAG * /
        rtag = BiddyIsSmaller(tagF,tagG) ? tagF : tagG;
        w = rtag;
        if (tagF == rtag) {
          if (BiddyV(f) == rtag) {
            / * TAG AND TOP VARIABLE ARE EQUAL * /
            Fneg_v = BiddyE(f);
            Fv = BiddyT(f);
          } else {
            / * TAG AND TOP VARIABLE ARE NOT EQUAL * /
            Fneg_v = f;
            Biddy_SetTag(Fneg_v,rtag);
            Fneg_v = Biddy_Managed_IncTag(MNG,Fneg_v);
            Fv = biddyZero;
          }
          Gneg_v = Gv = g;
        } else {
          if (BiddyV(g) == rtag) {
            / * TAG AND TOP VARIABLE ARE EQUAL * /
            Gneg_v = BiddyE(g);
            Gv = BiddyT(g);
          } else {
            / * TAG AND TOP VARIABLE ARE NOT EQUAL * /
            Gneg_v = g;
            Biddy_SetTag(Gneg_v,rtag);
            Gneg_v = Biddy_Managed_IncTag(MNG,Gneg_v);
            Gv = biddyZero;
          }
          Fneg_v = Fv = f;
        }
      }
      */

      /* VARIANT 2 */
      /**/
      tagF = Biddy_GetTag(f);
      tagG = Biddy_GetTag(g);
      topF = BiddyV(f);
      topG = BiddyV(g);
      rtag = BiddyIsSmaller(tagF,tagG) ? tagF : tagG;
      v = BiddyIsSmaller(topF,topG) ? topF : topG;
      if (tagF != tagG) v = rtag;
      if (tagF == rtag) {
        if (topF == v) {
          Fneg_v = BiddyE(f);
          Fv = BiddyT(f);
        } else {
          Fneg_v = f;
          Biddy_SetTag(Fneg_v,v);
          Fneg_v = Biddy_Managed_IncTag(MNG,Fneg_v);
          Fv = biddyZero;
        }
      } else {
        Fneg_v = Fv = f;
      }
      if (tagG == rtag) {
        if (topG == v) {
          Gneg_v = BiddyE(g);
          Gv = BiddyT(g);
        } else {
          Gneg_v = g;
          Biddy_SetTag(Gneg_v,v);
          Gneg_v = Biddy_Managed_IncTag(MNG,Gneg_v);
          Gv = biddyZero;
        }
      } else {
        Gneg_v = Gv = g;
      }
      /**/

    }

    /* RECURSIVE CALLS */
    /* returning biddyOne in a recursive call is wrong for ZBDD and ZBDDC, */
    /* because top variable of the result may be lower (topmore) than top variables of the arguments */
    /* (i.e. because the result depends on domain) */
    if (Fneg_v == biddyZero) {
      E = Gneg_v;
    } else if (Gneg_v == biddyZero) {
      E = Fneg_v;
    } else if (Fneg_v == biddyOne) {
      /* FOR ZBDD AND ZBDDC THIS LINE IS NOT REACHABLE IN RECURSIVE CALLS */
      E = biddyOne;
    } else if (Gneg_v == biddyOne) {
      /* FOR ZBDD AND ZBDDC THIS LINE IS NOT REACHABLE IN RECURSIVE CALLS */
      E = biddyOne;
    } else  {
      E = BiddyManagedOr(MNG,Fneg_v,Gneg_v);
    }
    if (Fv == biddyZero) {
      T = Gv;
    } else if (Gv == biddyZero) {
      T = Fv;
    } else if (Fv == biddyOne) {
      /* FOR ZBDD AND ZBDDC THIS LINE IS NOT REACHABLE IN RECURSIVE CALLS */
      T = biddyOne;
    } else if (Gv == biddyOne) {
      /* FOR ZBDD AND ZBDDC THIS LINE IS NOT REACHABLE IN RECURSIVE CALLS */
      T = biddyOne;
    } else  {
      T = BiddyManagedOr(MNG,Fv,Gv);
    }

    r = BiddyManagedTaggedFoaNode(MNG,v,E,T,rtag,TRUE);
    BiddyRefresh(r); /* FoaNode returns an obsolete node! */

    if (NN) {
      addOp3Cache(MNG,biddyOPCache,FF,GG,HH,Biddy_Inv(r),cindex);
    } else {
      addOp3Cache(MNG,biddyOPCache,FF,GG,HH,r,cindex);
    }

  } else {

    if (NN) {
      Biddy_InvertMark(r);
    }

    /* IF THE RESULT IS FROM CACHE TABLE, REFRESH IT! */
    BiddyRefresh(r);

  }

  return r;
}

/***************************************************************************//*!
\brief Function Biddy_Managed_Nand calculates Boolean function NAND
       (Sheffer).

### Description
### Side Effects
    Implemented for OBDDC. Prototyped for OBDD.
    Prototyped for ZBDDC and TZBDD (via and-not).
    For OBDDC, results are cached as parameters to ITE(F,G,H)= F*G XOR F'*H.
    For OBDD, ZBDDC and TZBDD, results could be cached as (f,g,biddyNull).
### More Info
    Macro Biddy_Nand(f,g) is defined for use with anonymous manager.
*******************************************************************************/

#ifdef __cplusplus
extern "C" {
#endif

Biddy_Edge
Biddy_Managed_Nand(Biddy_Manager MNG, Biddy_Edge f, Biddy_Edge g)
{
  Biddy_Edge r;

  assert( f != NULL );
  assert( g != NULL );

  if (!MNG) MNG = biddyAnonymousManager;
  ZF_LOGI("Biddy_Nand");

  assert( BiddyIsOK(f) == TRUE );
  assert( BiddyIsOK(g) == TRUE );

  r = biddyNull;

  if (biddyManagerType == BIDDYTYPEOBDD) {
    /* PROTOTYPED */
    r = BiddyManagedNot(MNG,BiddyManagedAnd(MNG,f,g));
    BiddyRefresh(r); /* not always refreshed by BiddyManagedNot */
  } else if (biddyManagerType == BIDDYTYPEOBDDC) {
    /* IMPLEMENTED */
    r = BiddyManagedNand(MNG,f,g);
    BiddyRefresh(r); /* not always refreshed by BiddyManagedNand */
  } else if (biddyManagerType == BIDDYTYPEZBDDC) {
    /* PROTOTYPED */
    r = BiddyManagedNot(MNG,BiddyManagedAnd(MNG,f,g));
    BiddyRefresh(r); /* not always refreshed by BiddyManagedNot */
  } else if (biddyManagerType == BIDDYTYPETZBDD) {
    /* PROTOTYPED */
    r = BiddyManagedNot(MNG,BiddyManagedAnd(MNG,f,g));
    BiddyRefresh(r); /* not always refreshed by BiddyManagedNot */
  } else if ((biddyManagerType == BIDDYTYPEZBDD) ||
              (biddyManagerType == BIDDYTYPETZBDDC))
  {
    fprintf(stderr,"Biddy_Nand: this GDD type is not supported, yet!\n");
    return biddyNull;
  } else if ((biddyManagerType == BIDDYTYPEOFDDC) || (biddyManagerType == BIDDYTYPEOFDD) ||
              (biddyManagerType == BIDDYTYPEZFDDC) || (biddyManagerType == BIDDYTYPEZFDD) ||
              (biddyManagerType == BIDDYTYPETZFDDC) || (biddyManagerType == BIDDYTYPETZFDD))
  {
    fprintf(stderr,"Biddy_Nand: this GDD type is not supported, yet!\n");
    return biddyNull;
  } else {
    fprintf(stderr,"Biddy_Nand: Unsupported GDD type!\n");
    return biddyNull;
  }

  return r;
}

#ifdef __cplusplus
}
#endif

Biddy_Edge
BiddyManagedNand(const Biddy_Manager MNG, const Biddy_Edge f,
                 const Biddy_Edge g)
{
  Biddy_Edge r;

  assert( MNG != NULL );
  assert( f != NULL );
  assert( g != NULL );

  /* IMPLEMENTED FOR OBDDC, ONLY */
  assert( biddyManagerType == BIDDYTYPEOBDDC );

  r = biddyNull;

  if (biddyManagerType == BIDDYTYPEOBDDC) {
    r = Biddy_Inv(BiddyManagedAnd(MNG,f,g));
  }

  return r;
}

/***************************************************************************//*!
\brief Function Biddy_Managed_Nor calculates Boolean function NOR
       (Peirce).

### Description
### Side Effects
    Implemented for OBDDC. Prototyped for OBDD.
    Prototyped for ZBDDC and TZBDD (via or-not).
    For OBDDC, results are cached as parameters to ITE(F,G,H)= F*G XOR F'*H.
    For OBDD, ZBDDC and TZBDD, results could be cached as (biddyNull,f,g).
### More Info
    Macro Biddy_Nor(f,g) is defined for use with anonymous manager.
*******************************************************************************/

#ifdef __cplusplus
extern "C" {
#endif

Biddy_Edge
Biddy_Managed_Nor(Biddy_Manager MNG, Biddy_Edge f, Biddy_Edge g)
{
  Biddy_Edge r;

  assert( f != NULL );
  assert( g != NULL );

  if (!MNG) MNG = biddyAnonymousManager;
  ZF_LOGI("Biddy_Nor");

  assert( BiddyIsOK(f) == TRUE );
  assert( BiddyIsOK(g) == TRUE );

  r = biddyNull;

  if (biddyManagerType == BIDDYTYPEOBDD) {
    /* PROTOTYPED */
    r = BiddyManagedNot(MNG,BiddyManagedOr(MNG,f,g));
    BiddyRefresh(r); /* not always refreshed by BiddyManagedNot */
  } else if (biddyManagerType == BIDDYTYPEOBDDC) {
    /* IMPLEMENTED */
    r = BiddyManagedNor(MNG,f,g);
    BiddyRefresh(r); /* not always refreshed by BiddyManagedNor */
  } else if (biddyManagerType == BIDDYTYPEZBDDC) {
    /* PROTOTYPED */
    r = BiddyManagedNot(MNG,BiddyManagedOr(MNG,f,g));
    BiddyRefresh(r); /* not always refreshed by BiddyManagedNot */
  } else if (biddyManagerType == BIDDYTYPETZBDD) {
    /* PROTOTYPED */
    r = BiddyManagedNot(MNG,BiddyManagedOr(MNG,f,g));
    BiddyRefresh(r); /* not always refreshed by BiddyManagedNot */
  } else if ((biddyManagerType == BIDDYTYPEZBDD) ||
              (biddyManagerType == BIDDYTYPETZBDDC))
  {
    fprintf(stderr,"Biddy_Nor: this GDD type is not supported, yet!\n");
    return biddyNull;
  } else if ((biddyManagerType == BIDDYTYPEOFDDC) || (biddyManagerType == BIDDYTYPEOFDD) ||
              (biddyManagerType == BIDDYTYPEZFDDC) || (biddyManagerType == BIDDYTYPEZFDD) ||
              (biddyManagerType == BIDDYTYPETZFDDC) || (biddyManagerType == BIDDYTYPETZFDD))
  {
    fprintf(stderr,"Biddy_Nor: this GDD type is not supported, yet!\n");
    return biddyNull;
  } else {
    fprintf(stderr,"Biddy_Nor: Unsupported GDD type!\n");
    return biddyNull;
  }

  return r;
}

#ifdef __cplusplus
}
#endif

Biddy_Edge
BiddyManagedNor(const Biddy_Manager MNG, const Biddy_Edge f,
                const Biddy_Edge g)
{
  Biddy_Edge r;

  assert( MNG != NULL );
  assert( f != NULL );
  assert( g != NULL );

  /* IMPLEMENTED FOR OBDDC, ONLY */
  assert( biddyManagerType == BIDDYTYPEOBDDC );

  r = biddyNull;

  if (biddyManagerType == BIDDYTYPEOBDDC) {
    r = BiddyManagedAnd(MNG,Biddy_Inv(f),Biddy_Inv(g));
  }

  return r;
}

/***************************************************************************//*!
\brief Function Biddy_Managed_Xor calculates Boolean function XOR.

### Description
### Side Effects
    Used by ITE (for OBDDC).
    Implemented for OBDD, OBDDC, ZBDDC, and TZBDD.
    For OBDDC, results are cached as parameters to ITE(F,G,H)= F*G XOR F'*H.
    For all other BDD types, results are cached as (f,biddyZero,g).
### More Info
    Macro Biddy_Xor(f,g) is defined for use with anonymous manager.
*******************************************************************************/

#ifdef __cplusplus
extern "C" {
#endif

Biddy_Edge
Biddy_Managed_Xor(Biddy_Manager MNG, Biddy_Edge f, Biddy_Edge g)
{
  Biddy_Edge r;

  assert( f != NULL );
  assert( g != NULL );

  if (!MNG) MNG = biddyAnonymousManager;
  ZF_LOGI("Biddy_Xor");

  assert( BiddyIsOK(f) == TRUE );
  assert( BiddyIsOK(g) == TRUE );

  biddyNodeTable.funxor++;

  r = biddyNull;

  if (biddyManagerType == BIDDYTYPEOBDD) {
    /* IMPLEMENTED */
    r = BiddyManagedXor(MNG,f,g);
    BiddyRefresh(r); /* not always refreshed by BiddyManagedXor */
  } else if (biddyManagerType == BIDDYTYPEOBDDC) {
    /* IMPLEMENTED */
    r = BiddyManagedXor(MNG,f,g);
    BiddyRefresh(r); /* not always refreshed by BiddyManagedXor */
  } else if (biddyManagerType == BIDDYTYPEZBDDC) {
    /* IMPLEMENTED */
    r = BiddyManagedXor(MNG,f,g);
    BiddyRefresh(r); /* not always refreshed by BiddyManagedXor */
  } else if (biddyManagerType == BIDDYTYPETZBDD) {
    /* IMPLEMENTED */
    r = BiddyManagedXor(MNG,f,g);
    BiddyRefresh(r); /* not always refreshed by BiddyManagedXor */
  } else if ((biddyManagerType == BIDDYTYPEZBDD) ||
              (biddyManagerType == BIDDYTYPETZBDDC))
  {
    fprintf(stderr,"Biddy_Xor: this GDD type is not supported, yet!\n");
    return biddyNull;
  } else if ((biddyManagerType == BIDDYTYPEOFDDC) || (biddyManagerType == BIDDYTYPEOFDD) ||
              (biddyManagerType == BIDDYTYPEZFDDC) || (biddyManagerType == BIDDYTYPEZFDD) ||
              (biddyManagerType == BIDDYTYPETZFDDC) || (biddyManagerType == BIDDYTYPETZFDD))
  {
    fprintf(stderr,"Biddy_Xor: this GDD type is not supported, yet!\n");
    return biddyNull;
  } else {
    fprintf(stderr,"Biddy_Xor: Unsupported GDD type!\n");
    return biddyNull;
  }

  return r;
}

#ifdef __cplusplus
}
#endif

Biddy_Edge
BiddyManagedXor(const Biddy_Manager MNG, const Biddy_Edge f,
                const Biddy_Edge g)
{
  Biddy_Edge r, T, E, Fv, Gv, Fneg_v, Gneg_v;
  Biddy_Edge FF, GG, HH;
  Biddy_Boolean NN;
  Biddy_Variable v,rtag;
  unsigned int cindex;

  static Biddy_Variable topF;    /* CAN BE STATIC, WHAT IS BETTER? */
  static Biddy_Variable topG;    /* CAN BE STATIC, WHAT IS BETTER? */
  static Biddy_Variable tagF;    /* CAN BE STATIC, WHAT IS BETTER? */
  static Biddy_Variable tagG;    /* CAN BE STATIC, WHAT IS BETTER? */

  assert( MNG != NULL );
  assert( f != NULL );
  assert( g != NULL );

  /* IMPLEMENTED FOR OBDD, OBDDC, ZBDDC, AND TZBDD */
  assert(
    (biddyManagerType == BIDDYTYPEOBDD) ||
    (biddyManagerType == BIDDYTYPEOBDDC) ||
    (biddyManagerType == BIDDYTYPEZBDDC) ||
    (biddyManagerType == BIDDYTYPETZBDD)
  );

#ifdef BIDDYEXTENDEDSTATS_YES
  biddyNodeTable.xorrecursive++;
#endif

  /* LOOKING FOR SIMPLE CASE */
  /* returning biddyOne in a recursive call is wrong for ZBDD and ZBDDC, */
  /* because top variable of the result may be lower (topmore) than top variables of the arguments */
  /* (i.e. because the result depends on domain) */

  if (f == biddyZero) {
    return g;
  } else if (g == biddyZero) {
    return f;
  } else if (f == biddyOne) {
    /* FOR ZBDD AND ZBDDC THIS LINE IS NOT REACHABLE IN RECURSIVE CALLS */
    return BiddyManagedNot(MNG,g);
  } else if (g == biddyOne) {
    /* FOR ZBDD AND ZBDDC THIS LINE IS NOT REACHABLE IN RECURSIVE CALLS */
    return BiddyManagedNot(MNG,f);
  } else if (f == g) {
    return biddyZero;
  }

  if (biddyManagerType == BIDDYTYPEOBDDC) {
    if (Biddy_IsEqvPointer(f,g)) {
      return biddyOne;
    }
  }

  /* THIS IS NOT A SIMPLE CASE */

  FF = GG = HH = biddyNull;
  NN = FALSE;
  Fneg_v = Fv = biddyNull;
  Gneg_v = Gv = biddyNull;
  rtag = 0;
  v = 0;

  /* NORMALIZATION OF COMPLEMENTED EDGES */
  /* FF, GG, and HH ARE USED FOR CACHE LOOKUP, ONLY */
  /* FF AND GG SHOULD NOT BE COMPLEMENTED */
  /* ((uintptr_t) f) SHOULD BE GREATER THAN ((uintptr_t) g) */

  if (biddyManagerType == BIDDYTYPEOBDDC) {

    if (((uintptr_t) f) > ((uintptr_t) g)) {
      if (Biddy_GetMark(f)) {
        if (Biddy_GetMark(g)) {
          FF = Biddy_Inv(f);
          GG = Biddy_Inv(g); /* h */
          HH = g;
        } else {
          NN = TRUE;
          FF = Biddy_Inv(f);
          GG = g;
          HH = Biddy_Inv(g); /* h */
        }
      } else {
        if (Biddy_GetMark(g)) {
          NN = TRUE;
          FF = f;
          GG = Biddy_Inv(g); /* h */
          HH = g;
        } else {
          FF = f;
          GG = g;
          HH = Biddy_Inv(g); /* h */
        }
      }
    } else {
      if (Biddy_GetMark(g)) {
        if (Biddy_GetMark(f)) {
          FF = Biddy_Inv(g);
          GG = Biddy_Inv(f); /* h */
          HH = f;
        } else {
          NN = TRUE;
          FF = Biddy_Inv(g);
          GG = f;
          HH = Biddy_Inv(f); /* h */
        }
      } else {
        if (Biddy_GetMark(f)) {
          NN = TRUE;
          FF = g;
          GG = Biddy_Inv(f); /* h */
          HH = f;
        } else {
          FF = g;
          GG = f;
          HH = Biddy_Inv(f); /* h */
        }
      }
    }

  }

  else {

    /* THIS IS NOT ITE CACHE! */

    if (((uintptr_t) f) > ((uintptr_t) g)) {
      FF = f;
      GG = biddyZero;
      HH = g;
    } else {
      FF = g;
      GG = biddyZero;
      HH = f;
    }

  }

  /* IF RESULT IS NOT IN THE CACHE TABLE... */
  cindex = 0;
  if (!findOp3Cache(MNG,biddyOPCache,FF,GG,HH,&r,&cindex))
  {

    if ((biddyManagerType == BIDDYTYPEOBDDC) || (biddyManagerType == BIDDYTYPEOBDD)) {

      /* LOOKING FOR THE SMALLEST TOP VARIABLE */
      /* CONSTANT VARIABLE MUST HAVE MAX ORDER */
      topF = BiddyV(f);
      topG = BiddyV(g);
      v = BiddyIsSmaller(topF,topG) ? topF : topG;

      /* DETERMINING PARAMETERS FOR RECURSIVE CALLS */
      /* COMPLEMENTED EDGES MUST BE TRANSFERED */
      if (topF == v) {
        Fneg_v = Biddy_InvCond(BiddyE(f),Biddy_GetMark(f));
        Fv = Biddy_InvCond(BiddyT(f),Biddy_GetMark(f));
      } else {
        Fneg_v = Fv = f;
      }
      if (topG == v) {
        Gneg_v = Biddy_InvCond(BiddyE(g),Biddy_GetMark(g));
        Gv = Biddy_InvCond(BiddyT(g),Biddy_GetMark(g));
      } else {
        Gneg_v = Gv = g;
      }

      rtag = v;

    }

    else if (biddyManagerType == BIDDYTYPEZBDDC) {

      /* LOOKING FOR THE SMALLEST TOP VARIABLE */
      /* CONSTANT VARIABLE MUST HAVE MAX ORDER */
      topF = BiddyV(f);
      topG = BiddyV(g);
      v = BiddyIsSmaller(topF,topG) ? topF : topG;

      /* DETERMINING PARAMETERS FOR RECURSIVE CALLS */
      /* COMPLEMENTED EDGES MUST BE TRANSFERED */
      if (topF == v) {
        Fneg_v = Biddy_InvCond(BiddyE(f),Biddy_GetMark(f));
        Fv = BiddyT(f);
      } else {
        Fneg_v = f;
        Fv = biddyZero;
      }
      if (topG == v) {
        Gneg_v = Biddy_InvCond(BiddyE(g),Biddy_GetMark(g));
        Gv = BiddyT(g);
      } else {
        Gneg_v = g;
        Gv = biddyZero;
      }

      rtag = 0;

    }

    else if (biddyManagerType == BIDDYTYPETZBDD) {

      /* VARIANT 1 */
      /*
      tagF = Biddy_GetTag(f);
      tagG = Biddy_GetTag(g);
      if (tagF == tagG) {
        / * THE SAME TAG * /
        topF = BiddyV(f);
        topG = BiddyV(g);
        v = BiddyIsSmaller(topF,topG) ? topF : topG;
        rtag = tagF;
        w = v;
        if (topF == v) {
          Fneg_v = BiddyE(f);
          Fv = BiddyT(f);
        } else {
          Fneg_v = f;
          Biddy_SetTag(Fneg_v,v);
          Fneg_v = Biddy_Managed_IncTag(MNG,Fneg_v);
          Fv = biddyZero;
        }
        if (topG == v) {
          Gneg_v = BiddyE(g);
          Gv = BiddyT(g);
        } else {
          Gneg_v = g;
          Biddy_SetTag(Gneg_v,v);
          Gneg_v = Biddy_Managed_IncTag(MNG,Gneg_v);
          Gv = biddyZero;
        }
      } else {
        / * DIFFERENT TAG * /
        rtag = BiddyIsSmaller(tagF,tagG) ? tagF : tagG;
        w = rtag;
        if (tagF == rtag) {
          if (BiddyV(f) == rtag) {
            / * TAG AND TOP VARIABLE ARE EQUAL * /
            Fneg_v = BiddyE(f);
            Fv = BiddyT(f);
          } else {
            / * TAG AND TOP VARIABLE ARE NOT EQUAL * /
            Fneg_v = f;
            Biddy_SetTag(Fneg_v,rtag);
            Fneg_v = Biddy_Managed_IncTag(MNG,Fneg_v);
            Fv = biddyZero;
          }
          Gneg_v = Gv = g;
        } else {
          if (BiddyV(g) == rtag) {
            / * TAG AND TOP VARIABLE ARE EQUAL * /
            Gneg_v = BiddyE(g);
            Gv = BiddyT(g);
          } else {
            / * TAG AND TOP VARIABLE ARE NOT EQUAL * /
            Gneg_v = g;
            Biddy_SetTag(Gneg_v,rtag);
            Gneg_v = Biddy_Managed_IncTag(MNG,Gneg_v);
            Gv = biddyZero;
          }
          Fneg_v = Fv = f;
        }
      }
      */

      /* VARIANT 2 */
      /**/
      tagF = Biddy_GetTag(f);
      tagG = Biddy_GetTag(g);
      topF = BiddyV(f);
      topG = BiddyV(g);
      rtag = BiddyIsSmaller(tagF,tagG) ? tagF : tagG;
      v = BiddyIsSmaller(topF,topG) ? topF : topG;
      if (tagF != tagG) v = rtag;
      if (tagF == rtag) {
        if (topF == v) {
          Fneg_v = BiddyE(f);
          Fv = BiddyT(f);
        } else {
          Fneg_v = f;
          Biddy_SetTag(Fneg_v,v);
          Fneg_v = Biddy_Managed_IncTag(MNG,Fneg_v);
          Fv = biddyZero;
        }
      } else {
        Fneg_v = Fv = f;
      }
      if (tagG == rtag) {
        if (topG == v) {
          Gneg_v = BiddyE(g);
          Gv = BiddyT(g);
        } else {
          Gneg_v = g;
          Biddy_SetTag(Gneg_v,v);
          Gneg_v = Biddy_Managed_IncTag(MNG,Gneg_v);
          Gv = biddyZero;
        }
      } else {
        Gneg_v = Gv = g;
      }
      /**/

    }

    /* RECURSIVE CALLS */
    /* returning biddyOne in a recursive call is wrong for ZBDD and ZBDDC, */
    /* because top variable of the result may be lower (topmore) than top variables of the arguments */
    /* (i.e. because the result depends on domain) */
    if (Fneg_v == biddyZero) {
      E = Gneg_v;
    } else if (Gneg_v == biddyZero) {
      E = Fneg_v;
    } else if (Fneg_v == biddyOne) {
      /* FOR ZBDD AND ZBDDC THIS LINE IS NOT REACHABLE IN RECURSIVE CALLS */
      E = BiddyManagedNot(MNG,Gneg_v);
    } else if (Gneg_v == biddyOne) {
      /* FOR ZBDD AND ZBDDC THIS LINE IS NOT REACHABLE IN RECURSIVE CALLS */
      E = BiddyManagedNot(MNG,Fneg_v);
    } else if (Fneg_v == Gneg_v) {
      E = biddyZero;
    } else  {
      E = BiddyManagedXor(MNG,Fneg_v,Gneg_v);
    }
    if (Fv == biddyZero) {
      T = Gv;
    } else if (Gv == biddyZero) {
      T = Fv;
    } else if (Fv == biddyOne) {
      /* FOR ZBDD AND ZBDDC THIS LINE IS NOT REACHABLE IN RECURSIVE CALLS */
      T = BiddyManagedNot(MNG,Gv);
    } else if (Gv == biddyOne) {
      /* FOR ZBDD AND ZBDDC THIS LINE IS NOT REACHABLE IN RECURSIVE CALLS */
      T = BiddyManagedNot(MNG,Fv);
    } else if (Fv == Gv) {
      T = biddyZero;
    } else  {
      T = BiddyManagedXor(MNG,Fv,Gv);
    }

    r = BiddyManagedTaggedFoaNode(MNG,v,E,T,rtag,TRUE);
    BiddyRefresh(r); /* FoaNode returns an obsolete node! */

    if (NN) {
      addOp3Cache(MNG,biddyOPCache,FF,GG,HH,Biddy_Inv(r),cindex);
    } else {
      addOp3Cache(MNG,biddyOPCache,FF,GG,HH,r,cindex);
    }

  } else {

    if (NN) {
      Biddy_InvertMark(r);
    }

    /* IF THE RESULT IS FROM CACHE TABLE, REFRESH IT! */
    BiddyRefresh(r);

  }

  return r;
}

/***************************************************************************//*!
\brief Function Biddy_Managed_Xnor calculates Boolean function XNOR.

### Description
### Side Effects
    Implemented for OBDDC. Prototyped for OBDD.
    Prototyped for ZBDDC and TZBDD (via xor-not).
    For OBDDC, results are cached as parameters to ITE(F,G,H)= F*G XOR F'*H.
    For OBDD, ZBDDC and TZBDD, results could be cached as (f,biddyNull,g).
### More Info
    Macro Biddy_Xnor(f,g) is defined for use with anonymous manager.
*******************************************************************************/

#ifdef __cplusplus
extern "C" {
#endif

Biddy_Edge
Biddy_Managed_Xnor(Biddy_Manager MNG, Biddy_Edge f, Biddy_Edge g)
{
  Biddy_Edge r;

  assert( f != NULL );
  assert( g != NULL );

  if (!MNG) MNG = biddyAnonymousManager;
  ZF_LOGI("Biddy_Xnor");

  assert( BiddyIsOK(f) == TRUE );
  assert( BiddyIsOK(g) == TRUE );

  r = biddyNull;

  if (biddyManagerType == BIDDYTYPEOBDD) {
    /* PROTOTYPED */
    r = BiddyManagedNot(MNG,BiddyManagedXor(MNG,f,g));
    BiddyRefresh(r); /* not always refreshed by BiddyManagedNot */
  } else if (biddyManagerType == BIDDYTYPEOBDDC) {
    /* IMPLEMENTED */
    r = BiddyManagedXnor(MNG,f,g);
    BiddyRefresh(r); /* not always refreshed by BiddyManagedXnor */
  } else if (biddyManagerType == BIDDYTYPEZBDDC) {
    /* PROTOTYPED */
    r = BiddyManagedNot(MNG,BiddyManagedXor(MNG,f,g));
    BiddyRefresh(r); /* not always refreshed by BiddyManagedNot */
  } else if (biddyManagerType == BIDDYTYPETZBDD) {
    /* PROTOTYPED */
    r = BiddyManagedNot(MNG,BiddyManagedXor(MNG,f,g));
    BiddyRefresh(r); /* not always refreshed by BiddyManagedNot */
  } else if ((biddyManagerType == BIDDYTYPEZBDD) ||
              (biddyManagerType == BIDDYTYPETZBDDC))
  {
    fprintf(stderr,"Biddy_Xnor: this GDD type is not supported, yet!\n");
    return biddyNull;
  } else if ((biddyManagerType == BIDDYTYPEOFDDC) || (biddyManagerType == BIDDYTYPEOFDD) ||
              (biddyManagerType == BIDDYTYPEZFDDC) || (biddyManagerType == BIDDYTYPEZFDD) ||
              (biddyManagerType == BIDDYTYPETZFDDC) || (biddyManagerType == BIDDYTYPETZFDD))
  {
    fprintf(stderr,"Biddy_Xnor: this GDD type is not supported, yet!\n");
    return biddyNull;
  } else {
    fprintf(stderr,"Biddy_Xnor: Unsupported GDD type!\n");
    return biddyNull;
  }

  return r;
}

#ifdef __cplusplus
}
#endif

Biddy_Edge
BiddyManagedXnor(const Biddy_Manager MNG, const Biddy_Edge f,
                 const Biddy_Edge g)
{
  Biddy_Edge r;

  assert( MNG != NULL );
  assert( f != NULL );
  assert( g != NULL );

  r = biddyNull;

  /* IMPLEMENTED FOR OBDDC, ONLY */
  assert( biddyManagerType == BIDDYTYPEOBDDC );

  if (biddyManagerType == BIDDYTYPEOBDDC) {
    r = Biddy_Inv(BiddyManagedXor(MNG,f,g));
  }

  return r;
}

/***************************************************************************//*!
\brief Function Biddy_Managed_Leq calculates Boolean implication.

### Description
    Boolean function leq(f,g) = or(not(f),g) = not(gt(f,g)).
    This function coincides with implication f->g.
### Side Effects
    Implemented for OBDD, OBDDC, ZBDDC, and TZBDD.
    For OBDDC, results are cached as parameters to ITE(F,G,H)= F*G XOR F'*H.
    For all other BDD types, results are cached as (f,f,g).
### More Info
    Macro Biddy_Leq(f,g) is defined for use with anonymous manager.
*******************************************************************************/

#ifdef __cplusplus
extern "C" {
#endif

Biddy_Edge
Biddy_Managed_Leq(Biddy_Manager MNG, Biddy_Edge f, Biddy_Edge g)
{
  Biddy_Edge r;

  assert( f != NULL );
  assert( g != NULL );

  if (!MNG) MNG = biddyAnonymousManager;
  ZF_LOGI("Biddy_Leq");

  assert( BiddyIsOK(f) == TRUE );
  assert( BiddyIsOK(g) == TRUE );

  r = biddyNull;

  if (biddyManagerType == BIDDYTYPEOBDD) {
    /* IMPLEMENTED */
    r = BiddyManagedLeq(MNG,f,g);
    BiddyRefresh(r); /* not always refreshed by BiddyManagedLeq */
  } else if (biddyManagerType == BIDDYTYPEOBDDC) {
    /* IMPLEMENTED */
    r = BiddyManagedLeq(MNG,f,g);
    BiddyRefresh(r); /* not always refreshed by BiddyManagedLeq */
  } else if (biddyManagerType == BIDDYTYPEZBDDC) {
    /* IMPLEMENTED */
    r = BiddyManagedLeq(MNG,f,g);
    BiddyRefresh(r); /* not always refreshed by BiddyManagedLeq */
  } else if (biddyManagerType == BIDDYTYPETZBDD) {
    /* IMPLEMENTED */
    r = BiddyManagedLeq(MNG,f,g);
    BiddyRefresh(r); /* not always refreshed by BiddyManagedLeq */
  } else if ((biddyManagerType == BIDDYTYPEZBDD) ||
              (biddyManagerType == BIDDYTYPETZBDDC))
  {
    fprintf(stderr,"Biddy_Leq: this GDD type is not supported, yet!\n");
    return biddyNull;
  } else if ((biddyManagerType == BIDDYTYPEOFDDC) || (biddyManagerType == BIDDYTYPEOFDD) ||
              (biddyManagerType == BIDDYTYPEZFDDC) || (biddyManagerType == BIDDYTYPEZFDD) ||
              (biddyManagerType == BIDDYTYPETZFDDC) || (biddyManagerType == BIDDYTYPETZFDD))
  {
    fprintf(stderr,"Biddy_Leq: this GDD type is not supported, yet!\n");
    return biddyNull;
  } else {
    fprintf(stderr,"Biddy_Leq: Unsupported GDD type!\n");
    return biddyNull;
  }

  return r;
}

#ifdef __cplusplus
}
#endif

Biddy_Edge
BiddyManagedLeq(const Biddy_Manager MNG, const Biddy_Edge f, const Biddy_Edge g)
{
  Biddy_Edge r, T, E, Fv, Gv, Fneg_v, Gneg_v;
  Biddy_Edge FF, GG, HH;
  Biddy_Variable v,rtag;
  unsigned int cindex;

  static Biddy_Variable topF; /* CAN BE STATIC, WHAT IS BETTER? */
  static Biddy_Variable topG; /* CAN BE STATIC, WHAT IS BETTER? */
  static Biddy_Variable tagF; /* CAN BE STATIC, WHAT IS BETTER? */
  static Biddy_Variable tagG; /* CAN BE STATIC, WHAT IS BETTER? */

  assert( MNG != NULL );
  assert( f != NULL );
  assert( g != NULL );

  r = biddyNull;

  /* IMPLEMENTED FOR OBDD, OBDDC, ZBDDC, AND TZBDDC */
  assert(
    (biddyManagerType == BIDDYTYPEOBDD) ||
    (biddyManagerType == BIDDYTYPEOBDDC) ||
    (biddyManagerType == BIDDYTYPEZBDDC) ||
    (biddyManagerType == BIDDYTYPETZBDD)
  );

  if (biddyManagerType == BIDDYTYPEOBDDC) {

    /* FOR OBDDC, PROCEED BY CALCULATING NOT-AND */

    r = Biddy_Inv(BiddyManagedAnd(MNG,f,Biddy_Inv(g)));
    return r;

  }

  if ((biddyManagerType == BIDDYTYPEZBDD) || (biddyManagerType == BIDDYTYPEZBDDC)) {

    /* FOR ZBDD AND ZBDDC, PROCEED BY CALCULATING ITE */

    r = BiddyManagedITE(MNG,f,g,biddyOne);
    return r;

  }

  /* LOOKING FOR SIMPLE CASE */
  /* returning biddyOne in a recursive call is wrong for ZBDD and ZBDDC, */
  /* because top variable of the result may be lower (topmore) than top variables of the arguments */
  /* (i.e. because the result depends on domain) */

  if (f == biddyZero) {
    /* ZBDD AND ZBDDC ARE ALREADY CALCULATED */
    return biddyOne;
  } else if (g == biddyZero) {
    /* OBDDC, ZBDD, AND ZBDDC ARE ALREADY CALCULATED */
    return BiddyManagedNot(MNG,f);
  } else if (f == biddyOne) {
    return g;
  } else if (g == biddyOne) {
    /* ZBDD AND ZBDDC ARE ALREADY CALCULATED */
    return biddyOne;
  } else if (f == g) {
    /* ZBDD AND ZBDDC ARE ALREADY CALCULATED */
    return biddyOne;
  }

  if (biddyManagerType == BIDDYTYPETZBDD) {
    if (BiddyR(f) == BiddyR(g)) {
      if (BiddyIsSmaller(Biddy_GetTag(f),Biddy_GetTag(g))) {
        return biddyOne;
      }
    }
  }

  /* THIS IS NOT A SIMPLE CASE */

  FF = GG = HH = biddyNull;
  Fneg_v = Fv = biddyNull;
  Gneg_v = Gv = biddyNull;
  rtag = 0;
  v = 0;

  if (biddyManagerType == BIDDYTYPEOBDDC) {
    /* ALREADY CALCULATED */
  }

  else {

#ifdef BIDDYEXTENDEDSTATS_YES
    biddyNodeTable.andorrecursive++;
#endif

    /* FF, GG, and HH ARE USED FOR CACHE LOOKUP, ONLY */
    /* THIS IS NOT ITE CACHE! */

    FF = f;
    GG = f;
    HH = g;

  }

  /* IF RESULT IS NOT IN THE CACHE TABLE... */
  cindex = 0;
  if (!findOp3Cache(MNG,biddyOPCache,FF,GG,HH,&r,&cindex))
  {

    if (biddyManagerType == BIDDYTYPEOBDDC) {
      /* ALREADY CALCULATED */
    }

    else if (biddyManagerType == BIDDYTYPEOBDD) {

      /* LOOKING FOR THE SMALLEST TOP VARIABLE */
      /* CONSTANT VARIABLE MUST HAVE MAX ORDER */
      topF = BiddyV(f);
      topG = BiddyV(g);
      v = BiddyIsSmaller(topF,topG) ? topF : topG;

      /* DETERMINING PARAMETERS FOR RECURSIVE CALLS */
      /* COMPLEMENTED EDGES MUST BE TRANSFERED */
      if (topF == v) {
        Fneg_v = BiddyE(f);
        Fv = BiddyT(f);
      } else {
        Fneg_v = Fv = f;
      }

      if (topG == v) {
        Gneg_v = BiddyE(g);
        Gv = BiddyT(g);
      } else {
        Gneg_v = Gv = g;
      }

      rtag = v;

    }

    else if (biddyManagerType == BIDDYTYPEZBDDC) {
      /* ALREADY CALCULATED */
    }

    else if (biddyManagerType == BIDDYTYPETZBDD) {

      /* LOOKING FOR THE SMALLEST TAG AND SMALLEST TOP VARIABLE */
      /* CONSTANT VARIABLE MUST HAVE MAX ORDER */
      tagF = Biddy_GetTag(f);
      tagG = Biddy_GetTag(g);
      topF = BiddyV(f);
      topG = BiddyV(g);
      rtag = BiddyIsSmaller(tagF,tagG) ? tagF : tagG;
      v = BiddyIsSmaller(topF,topG) ? topF : topG;

      if (tagF != tagG) v = rtag;
      if (tagF == rtag) {
        if (topF == v) {
          Fneg_v = BiddyE(f);
          Fv = BiddyT(f);
        } else {
          Fneg_v = f;
          Biddy_SetTag(Fneg_v,v);
          Fneg_v = Biddy_Managed_IncTag(MNG,Fneg_v);
          Fv = biddyZero;
        }
      } else {
        Fneg_v = Fv = f;
      }
      if (tagG == rtag) {
        if (topG == v) {
          Gneg_v = BiddyE(g);
          Gv = BiddyT(g);
        } else {
          Gneg_v = g;
          Biddy_SetTag(Gneg_v,v);
          Gneg_v = Biddy_Managed_IncTag(MNG,Gneg_v);
          Gv = biddyZero;
        }
      } else {
        Gneg_v = Gv = g;
      }

    }

    /* RECURSIVE CALLS */
    /* returning biddyOne in a recursive call is wrong for ZBDD and ZBDDC, */
    /* because top variable of the result may be lower (topmore) than top variables of the arguments */
    /* (i.e. because the result depends on domain) */
    if (Fneg_v == biddyZero) {
      /* ZBDD AND ZBDDC ARE ALREADY CALCULATED */
      E = biddyOne;
    } else if (Gneg_v == biddyZero) {
      /* OBDDC, ZBDD, AND ZBDDC ARE ALREADY CALCULATED */
      E = BiddyManagedNot(MNG,Fneg_v);
    } else if (Fneg_v == biddyOne) {
      E = Gneg_v;
    } else if (Gneg_v == biddyOne) {
      /* ZBDD AND ZBDDC ARE ALREADY CALCULATED */
      E = biddyOne;
    } else if (Fneg_v == Gneg_v) {
      /* ZBDD AND ZBDDC ARE ALREADY CALCULATED */
      E = biddyOne;
    } else  {
      E = BiddyManagedLeq(MNG,Fneg_v,Gneg_v);
    }
    if (Fv == biddyZero) {
      /* ZBDD AND ZBDDC ARE ALREADY CALCULATED */
      T = biddyOne;
    } else if (Gv == biddyZero) {
      /* OBDDC, ZBDD, AND ZBDDC ARE ALREADY CALCULATED */
      T = BiddyManagedNot(MNG,Fv);
    } else if (Fv == biddyOne) {
      T = Gv;
    } else if (Gv == biddyOne) {
      /* ZBDD AND ZBDDC ARE ALREADY CALCULATED */
      T = biddyOne;
    } else if (Fv == Gv) {
      /* ZBDD AND ZBDDC ARE ALREADY CALCULATED */
      T = biddyOne;
    } else  {
      T = BiddyManagedLeq(MNG,Fv,Gv);
    }

    r = BiddyManagedTaggedFoaNode(MNG,v,E,T,rtag,TRUE);
    BiddyRefresh(r); /* FoaNode returns an obsolete node! */

    addOp3Cache(MNG,biddyOPCache,FF,GG,HH,r,cindex);

  } else {

    /* IF THE RESULT IS FROM CACHE TABLE, REFRESH IT! */
    BiddyRefresh(r);

  }

  return r;
}

/***************************************************************************//*!
\brief Function Biddy_Managed_Gt calculates the negation of Boolean implication.

### Description
    Boolean function gt(f,g) = and(f,not(g)).
    For combination sets, this function coincides with Diff.
### Side Effects
    Implemented for OBDD, OBDDC, ZBDDC, and TZBDD.
    For OBDDC, results are cached as parameters to ITE(F,G,H)= F*G XOR F'*H.
    For all other BDD types, results are cached as (f,g,g).
### More Info
    Macro Biddy_Gt(f,g) is defined for use with anonymous manager.
    Macros Biddy_Managed_Diff(MNG,f,g) and Biddy_Diff(f,g) are defined for
    manipulation of combination sets.
*******************************************************************************/

#ifdef __cplusplus
extern "C" {
#endif

Biddy_Edge
Biddy_Managed_Gt(Biddy_Manager MNG, Biddy_Edge f, Biddy_Edge g)
{
  Biddy_Edge r;

  assert( f != NULL );
  assert( g != NULL );

  if (!MNG) MNG = biddyAnonymousManager;
  ZF_LOGI("Biddy_Gt");

  assert( BiddyIsOK(f) == TRUE );
  assert( BiddyIsOK(g) == TRUE );

  r = biddyNull;

  if (biddyManagerType == BIDDYTYPEOBDD) {
    /* IMPLEMENTED */
    r = BiddyManagedGt(MNG,f,g);
    BiddyRefresh(r); /* not always refreshed by BiddyManagedGt */
  } else if (biddyManagerType == BIDDYTYPEOBDDC) {
    /* IMPLEMENTED */
    r = BiddyManagedGt(MNG,f,g);
    BiddyRefresh(r); /* not always refreshed by BiddyManagedGt */
  } else if (biddyManagerType == BIDDYTYPEZBDDC) {
    /* IMPLEMENTED */
    r = BiddyManagedGt(MNG,f,g);
    BiddyRefresh(r); /* not always refreshed by BiddyManagedGt */
  } else if (biddyManagerType == BIDDYTYPETZBDD) {
    /* IMPLEMENTED */
    r = BiddyManagedGt(MNG,f,g);
    BiddyRefresh(r); /* not always refreshed by BiddyManagedGt */
  } else if ((biddyManagerType == BIDDYTYPEZBDD) ||
              (biddyManagerType == BIDDYTYPETZBDDC))
  {
    fprintf(stderr,"Biddy_Gt: this GDD type is not supported, yet!\n");
    return biddyNull;
  } else if ((biddyManagerType == BIDDYTYPEOFDDC) || (biddyManagerType == BIDDYTYPEOFDD) ||
              (biddyManagerType == BIDDYTYPEZFDDC) || (biddyManagerType == BIDDYTYPEZFDD) ||
              (biddyManagerType == BIDDYTYPETZFDDC) || (biddyManagerType == BIDDYTYPETZFDD))
  {
    fprintf(stderr,"Biddy_Gt: this GDD type is not supported, yet!\n");
    return biddyNull;
  } else {
    fprintf(stderr,"Biddy_Gt: Unsupported GDD type!\n");
    return biddyNull;
  }

  return r;
}

#ifdef __cplusplus
}
#endif

Biddy_Edge
BiddyManagedGt(const Biddy_Manager MNG, const Biddy_Edge f, const Biddy_Edge g)
{
  Biddy_Edge r, T, E, Fv, Gv, Fneg_v, Gneg_v;
  Biddy_Edge FF, GG, HH;
  Biddy_Variable v,rtag;
  unsigned int cindex;

  static Biddy_Variable topF; /* CAN BE STATIC, WHAT IS BETTER? */
  static Biddy_Variable topG; /* CAN BE STATIC, WHAT IS BETTER? */
  static Biddy_Variable tagF; /* CAN BE STATIC, WHAT IS BETTER? */
  static Biddy_Variable tagG; /* CAN BE STATIC, WHAT IS BETTER? */

  assert( MNG != NULL );
  assert( f != NULL );
  assert( g != NULL );

  r = biddyNull;

  /* IMPLEMENTED FOR OBDD, OBDDC, ZBDDC, AND TZBDDC */
  assert(
    (biddyManagerType == BIDDYTYPEOBDD) ||
    (biddyManagerType == BIDDYTYPEOBDDC) ||
    (biddyManagerType == BIDDYTYPEZBDDC) ||
    (biddyManagerType == BIDDYTYPETZBDD)
  );

  if (biddyManagerType == BIDDYTYPEOBDDC) {

    /* FOR OBDDC, PROCEED BY CALCULATING NOT-AND */

    r = BiddyManagedAnd(MNG,f,Biddy_Inv(g));
    return r;

  }

  /* LOOKING FOR SIMPLE CASE */
  /* using Not in a recursive call is wrong for ZBDD and ZBDDC, */
  /* because top variable of the result may be lower (topmore) than top variables of the arguments */
  /* (i.e. because the result depends on domain) */

  if (f == biddyZero) {
    return biddyZero;
  } else if (g == biddyZero) {
    return f;
  } else if (f == biddyOne) {
    /* FOR ZBDD AND ZBDDC THIS LINE IS NOT REACHABLE IN RECURSIVE CALLS */
    return BiddyManagedNot(MNG,g);
  } else if (g == biddyOne) {
    return biddyZero;
  } else if (f == g) {
    return biddyZero;
  }

  if (biddyManagerType == BIDDYTYPETZBDD) {
    if (BiddyR(f) == BiddyR(g)) {
      if (BiddyIsSmaller(Biddy_GetTag(f),Biddy_GetTag(g))) {
        return biddyZero;
      }
    }
  }

  /* THIS IS NOT A SIMPLE CASE */

  FF = GG = HH = biddyNull;
  Fneg_v = Fv = biddyNull;
  Gneg_v = Gv = biddyNull;
  rtag = 0;
  v = 0;

  if (biddyManagerType == BIDDYTYPEOBDDC) {
    /* ALREADY CALCULATED */
  }

  else {

#ifdef BIDDYEXTENDEDSTATS_YES
    biddyNodeTable.andorrecursive++;
#endif

    /* FF, GG, and HH ARE USED FOR CACHE LOOKUP, ONLY */
    /* THIS IS NOT ITE CACHE! */

    FF = f;
    GG = g;
    HH = g;

  }

  /* IF RESULT IS NOT IN THE CACHE TABLE... */
  cindex = 0;
  if (((biddyManagerType == BIDDYTYPEZBDDC) && BiddyIsSmaller(BiddyV(g),BiddyV(f)))
      || !findOp3Cache(MNG,biddyOPCache,FF,GG,HH,&r,&cindex))
  {

    if (biddyManagerType == BIDDYTYPEOBDDC) {
      /* ALREADY CALCULATED */
    }

    else if (biddyManagerType == BIDDYTYPEOBDD) {

      /* LOOKING FOR THE SMALLEST TOP VARIABLE */
      /* CONSTANT VARIABLE MUST HAVE MAX ORDER */
      topF = BiddyV(f);
      topG = BiddyV(g);
      v = BiddyIsSmaller(topF,topG) ? topF : topG;

      /* DETERMINING PARAMETERS FOR RECURSIVE CALLS */
      /* COMPLEMENTED EDGES MUST BE TRANSFERED */
      if (topF == v) {
        Fneg_v = BiddyE(f);
        Fv = BiddyT(f);
      } else {
        Fneg_v = Fv = f;
      }

      if (topG == v) {
        Gneg_v = BiddyE(g);
        Gv = BiddyT(g);
      } else {
        Gneg_v = Gv = g;
      }

      rtag = v;

    }

    else if (biddyManagerType == BIDDYTYPEZBDDC) {

      /* LOOKING FOR THE SMALLEST TOP VARIABLE */
      /* CONSTANT VARIABLE MUST HAVE MAX ORDER */
      topF = BiddyV(f);
      topG = BiddyV(g);
      v = BiddyIsSmaller(topF,topG) ? topF : topG;

      /* THIS CALL IS NOT INTRODUCING NEW NODES */
      /* COMPLEMENTED EDGES MUST BE TRANSFERED */
      /* IT SEEMS THAT CACHING THIS RESULTS IS NOT A GOOD IDEA */
      if (topF != v) {
        r = BiddyManagedGt(MNG,f,Biddy_InvCond(BiddyE(g),Biddy_GetMark(g)));
        return r;
      }

      /* DETERMINING PARAMETERS FOR RECURSIVE CALLS */
      /* IT IS ALREADY KNOWN THAT topF == v */
      /* COMPLEMENTED EDGES MUST BE TRANSFERED */
      Fneg_v = Biddy_InvCond(BiddyE(f),Biddy_GetMark(f));
      Fv = BiddyT(f);
      if (topG == v) {
        Gneg_v = Biddy_InvCond(BiddyE(g),Biddy_GetMark(g));
        Gv = BiddyT(g);
      } else {
         Gneg_v = g;
         Gv = biddyZero;
      }

      rtag = 0;

    }

    else if (biddyManagerType == BIDDYTYPETZBDD) {

      /* LOOKING FOR THE SMALLEST TAG AND SMALLEST TOP VARIABLE */
      /* CONSTANT VARIABLE MUST HAVE MAX ORDER */
      tagF = Biddy_GetTag(f);
      tagG = Biddy_GetTag(g);
      topF = BiddyV(f);
      topG = BiddyV(g);
      rtag = BiddyIsSmaller(tagF,tagG) ? tagF : tagG;
      v = BiddyIsSmaller(topF,topG) ? topF : topG;

      if (BiddyIsSmaller(tagG,tagF)) v = rtag;

      if (BiddyIsSmaller(v,tagF)) {
        Fneg_v = Fv = f;
      } else if (BiddyIsSmaller(v,topF)) {
        Fneg_v = f;
        Biddy_SetTag(Fneg_v,v);
        Fneg_v = Biddy_Managed_IncTag(MNG,Fneg_v);
        Fv = biddyZero;
      } else {
        /* HERE topF == v */
        Fneg_v = BiddyE(f);
        Fv = BiddyT(f);
      }
      if (BiddyIsSmaller(v,tagG)) {
        Gneg_v = Gv = g;
      } else if (BiddyIsSmaller(v,topG)) {
        Gneg_v = g;
        Biddy_SetTag(Gneg_v,v);
        Gneg_v = Biddy_Managed_IncTag(MNG,Gneg_v);
        Gv = biddyZero;
      } else {
        /* HERE topG == v */
        Gneg_v = BiddyE(g);
        Gv = BiddyT(g);
      }
      
    }

    /* RECURSIVE CALLS */
    /* using Not in a recursive call is wrong for ZBDD and ZBDDC, */
    /* because top variable of the result may be lower (topmore) than top variables of the arguments */
    /* (i.e. because the result depends on domain) */
    if (Fneg_v == biddyZero) {
      E = biddyZero;
    } else if (Gneg_v == biddyZero) {
      E = Fneg_v;
    } else if (Fneg_v == biddyOne) {
      /* FOR ZBDD AND ZBDDC THIS LINE IS NOT REACHABLE IN RECURSIVE CALLS */
      E = BiddyManagedNot(MNG,Gneg_v);
    } else if (Gneg_v == biddyOne) {
      E = biddyZero;
    } else if (Fneg_v == Gneg_v) {
      E = biddyZero;
    } else  {
      E = BiddyManagedGt(MNG,Fneg_v,Gneg_v);
    }
    if (Fv == biddyZero) {
      T = biddyZero;
    } else if (Gv == biddyZero) {
      T = Fv;
    } else if (Fv == biddyOne) {
      /* FOR ZBDD AND ZBDDC THIS LINE IS NOT REACHABLE IN RECURSIVE CALLS */
      T = BiddyManagedNot(MNG,Gv);
    } else if (Gv == biddyOne) {
      T = biddyZero;
    } else if (Fv == Gv) {
      T = biddyZero;
    } else  {
      T = BiddyManagedGt(MNG,Fv,Gv);
    }

    r = BiddyManagedTaggedFoaNode(MNG,v,E,T,rtag,TRUE);
    BiddyRefresh(r); /* FoaNode returns an obsolete node! */

    addOp3Cache(MNG,biddyOPCache,FF,GG,HH,r,cindex);

  } else {

    /* IF THE RESULT IS FROM CACHE TABLE, REFRESH IT! */
    BiddyRefresh(r);

  }

  return r;
}

/***************************************************************************//*!
\brief Function Biddy_Managed_IsLeq returns TRUE iff function f is included in
       function g.

### Description
### Side Effects
    Prototyped for OBDDs, ZBDDs, and TZBDDs (via calculating
    full implication, this is less efficient as implementation in CUDD).
### More Info
    Macro Biddy_IsLeq(f,g) is defined for use with anonymous manager.
*******************************************************************************/

#ifdef __cplusplus
extern "C" {
#endif

Biddy_Boolean
Biddy_Managed_IsLeq(Biddy_Manager MNG, Biddy_Edge f, Biddy_Edge g)
{
  Biddy_Edge imp;
  Biddy_Boolean r;

  if (!MNG) MNG = biddyAnonymousManager;
  ZF_LOGI("Biddy_IsLeq");

  r = FALSE;

  if (biddyManagerType == BIDDYTYPEOBDD) {
    /* PROTOTYPED */
    imp = BiddyManagedLeq(MNG,f,g);
    r = (imp == biddyOne);
  } else if (biddyManagerType == BIDDYTYPEOBDDC) {
    /* PROTOTYPED */
    imp = BiddyManagedLeq(MNG,f,g);
    r = (imp == biddyOne);
  } else if (biddyManagerType == BIDDYTYPEZBDDC) {
    /* PROTOTYPED */
    imp = BiddyManagedLeq(MNG,f,g);
    r = (imp == biddyOne);
  } else if (biddyManagerType == BIDDYTYPETZBDD) {
    /* PROTOTYPED */
    imp = BiddyManagedLeq(MNG,f,g);
    r = (imp == biddyOne);
  } else if ((biddyManagerType == BIDDYTYPEZBDD) ||
              (biddyManagerType == BIDDYTYPETZBDDC))
  {
    fprintf(stderr,"Biddy_IsLeq: this GDD type is not supported, yet!\n");
    return FALSE;
  } else if ((biddyManagerType == BIDDYTYPEOFDDC) || (biddyManagerType == BIDDYTYPEOFDD) ||
              (biddyManagerType == BIDDYTYPEZFDDC) || (biddyManagerType == BIDDYTYPEZFDD) ||
              (biddyManagerType == BIDDYTYPETZFDDC) || (biddyManagerType == BIDDYTYPETZFDD))
  {
    fprintf(stderr,"Biddy_IsLeq: this GDD type is not supported, yet!\n");
    return FALSE;
  } else {
    fprintf(stderr,"Biddy_IsLeq: Unsupported GDD type!\n");
    return FALSE;
  }

  return r;
}

#ifdef __cplusplus
}
#endif

/***************************************************************************//*!
\brief Function Biddy_Managed_Restrict calculates a restriction of Boolean
       function.

### Description
    This is not Coudert and Madre's restrict function (use Biddy_Simplify if
    you need that one).
### Side effects
    Original BDD is not changed.
    Implemented for OBDD, OBDDC, ZBDDC, and TZBDD.
    For OBDDs, recursive calls use optimization: F(a=x) == NOT((NOT F)(a=x)).
### More info
    Macro Biddy_Restrict(f,v,value) is defined for use with anonymous manager.
*******************************************************************************/

#ifdef __cplusplus
extern "C" {
#endif

Biddy_Edge
Biddy_Managed_Restrict(Biddy_Manager MNG, Biddy_Edge f, Biddy_Variable v,
                       Biddy_Boolean value)
{
  Biddy_Edge r;

  assert( f != NULL );

  if (!MNG) MNG = biddyAnonymousManager;
  ZF_LOGI("Biddy_Restrict");

  assert( BiddyIsOK(f) == TRUE );

  r = biddyNull;

  if (biddyManagerType == BIDDYTYPEOBDD) {
    /* IMPLEMENTED */
    r = BiddyManagedRestrict(MNG,f,v,value);
    BiddyRefresh(r); /* not always refreshed by BiddyManagedRestrict */
  } else if (biddyManagerType == BIDDYTYPEOBDDC) {
    /* IMPLEMENTED */
    r = BiddyManagedRestrict(MNG,f,v,value);
    BiddyRefresh(r); /* not always refreshed by BiddyManagedRestrict */
  } else if (biddyManagerType == BIDDYTYPEZBDDC) {
    /* IMPLEMENTED */
    r = BiddyManagedRestrict(MNG,f,v,value);
    BiddyRefresh(r); /* not always refreshed by BiddyManagedRestrict */
  } else if (biddyManagerType == BIDDYTYPETZBDD) {
    /* IMPLEMENTED */
    r = BiddyManagedRestrict(MNG,f,v,value);
    BiddyRefresh(r); /* not always refreshed by BiddyManagedRestrict */
  } else if ((biddyManagerType == BIDDYTYPEZBDD) ||
              (biddyManagerType == BIDDYTYPETZBDDC))
  {
    fprintf(stderr,"Biddy_Restrict: this GDD type is not supported, yet!\n");
    return biddyNull;
  } else if ((biddyManagerType == BIDDYTYPEOFDDC) || (biddyManagerType == BIDDYTYPEOFDD) ||
              (biddyManagerType == BIDDYTYPEZFDDC) || (biddyManagerType == BIDDYTYPEZFDD) ||
              (biddyManagerType == BIDDYTYPETZFDDC) || (biddyManagerType == BIDDYTYPETZFDD))
  {
    fprintf(stderr,"Biddy_Restrict: this GDD type is not supported, yet!\n");
    return biddyNull;
  } else {
    fprintf(stderr,"Biddy_Restrict: Unsupported GDD type!\n");
    return biddyNull;
  }

  return r;
}

#ifdef __cplusplus
}
#endif

Biddy_Edge
BiddyManagedRestrict(Biddy_Manager MNG, Biddy_Edge f, Biddy_Variable v,
                     Biddy_Boolean value)
{
  Biddy_Edge e, t, r;
  Biddy_Variable fv,tag;
  Biddy_Edge FF, GG, HH;
  unsigned int cindex;

  assert( MNG != NULL );
  assert( f != NULL );

  /* IMPLEMENTED FOR OBDD, OBDDC, ZBDDC, AND TZBDD */
  assert(
    (biddyManagerType == BIDDYTYPEOBDD) ||
    (biddyManagerType == BIDDYTYPEOBDDC) ||
    (biddyManagerType == BIDDYTYPEZBDDC) ||
    (biddyManagerType == BIDDYTYPETZBDD)
  );

  r = biddyNull;

  if (f == biddyZero) return biddyZero;

  FF = f;
  GG = value?biddyZero:biddyOne;
  HH = biddyVariableTable.table[v].variable;

  /* IF RESULT IS NOT IN THE CACHE TABLE... */
  /* Result is stored in cache only if recursive calls are needed */
  /* I am not sure if this is a good strategy! */
  cindex = 0;
  if (!findOp3Cache(MNG,biddyRCCache,FF,GG,HH,&r,&cindex))
  {

    if ((biddyManagerType == BIDDYTYPEOBDDC) || (biddyManagerType == BIDDYTYPEOBDD)) {
      if ((fv=BiddyV(f)) == v) {
        if (value) {
          r = Biddy_InvCond(BiddyT(f),Biddy_GetMark(f));
        } else {
          r = Biddy_InvCond(BiddyE(f),Biddy_GetMark(f));
        }
      }
      else if (BiddyIsSmaller(v,fv)) {
        r = f;
      }
      else {
        e = BiddyManagedRestrict(MNG,BiddyE(f),v,value);
        t = BiddyManagedRestrict(MNG,BiddyT(f),v,value);
        r = Biddy_InvCond(BiddyManagedTaggedFoaNode(MNG,fv,e,t,fv,TRUE),Biddy_GetMark(f));
        BiddyRefresh(r); /* FoaNode returns an obsolete node! */
        addOp3Cache(MNG,biddyRCCache,FF,GG,HH,r,cindex);
      }
    }

    else if (biddyManagerType == BIDDYTYPEZBDDC) {
      if ((fv=BiddyV(f)) == v) {
        if (value) {
          t = BiddyT(f);
          r = BiddyManagedTaggedFoaNode(MNG,v,t,t,0,TRUE);
          BiddyRefresh(r); /* FoaNode returns an obsolete node! */
        } else {
          e = Biddy_InvCond(BiddyE(f),Biddy_GetMark(f));
          r = BiddyManagedTaggedFoaNode(MNG,v,e,e,0,TRUE);
          BiddyRefresh(r); /* FoaNode returns an obsolete node! */
        }
      }
      else if (BiddyIsSmaller(v,fv)) {
        if (value) {
          r = biddyZero;
        } else {
          r = BiddyManagedTaggedFoaNode(MNG,v,f,f,0,TRUE);
          BiddyRefresh(r); /* FoaNode returns an obsolete node! */
        }
      }
      else {
        e = BiddyManagedRestrict(MNG,Biddy_InvCond(BiddyE(f),Biddy_GetMark(f)),v,value);
        t = BiddyManagedRestrict(MNG,BiddyT(f),v,value);
        r = BiddyManagedTaggedFoaNode(MNG,fv,e,t,0,TRUE);
        BiddyRefresh(r); /* FoaNode returns an obsolete node! */
        addOp3Cache(MNG,biddyRCCache,FF,GG,HH,r,cindex);
      }
    }

    else if (biddyManagerType == BIDDYTYPETZBDD) {
      tag = Biddy_GetTag(f);
      if (BiddyIsSmaller(v,tag)) {
        r = f;
      } else {
        fv = BiddyV(f);
        if (v == fv) {
          if (v == tag) {
            if (value) {
              r = BiddyT(f);
            } else {
              r = BiddyE(f);
            }
          } else {
            if (value) {
              t = BiddyT(f);
              r = BiddyManagedTaggedFoaNode(MNG,v,t,t,tag,TRUE);
              BiddyRefresh(r); /* FoaNode returns an obsolete node! */
            } else {
              e = BiddyE(f);
              r = BiddyManagedTaggedFoaNode(MNG,v,e,e,tag,TRUE);
              BiddyRefresh(r); /* FoaNode returns an obsolete node! */
            }
          }
        } else if (BiddyIsSmaller(v,fv)) {
          if (value) {
            r = biddyZero;
          } else {
            if (v == tag) {
              r = Biddy_Managed_IncTag(MNG,f);
            } else {
              r = f;
              Biddy_SetTag(r,v);
              r = Biddy_Managed_IncTag(MNG,r);
              r = BiddyManagedTaggedFoaNode(MNG,v,r,r,tag,TRUE);
              BiddyRefresh(r); /* FoaNode returns an obsolete node! */
            }
          }
        } else {
          e = BiddyManagedRestrict(MNG,BiddyE(f),v,value);
          t = BiddyManagedRestrict(MNG,BiddyT(f),v,value);
          r = BiddyManagedTaggedFoaNode(MNG,fv,e,t,tag,TRUE);
          BiddyRefresh(r); /* FoaNode returns an obsolete node! */
          addOp3Cache(MNG,biddyRCCache,FF,GG,HH,r,cindex);
        }
      }
    }

  } else {

    /* IF THE RESULT IS FROM CACHE TABLE, REFRESH IT! */
    BiddyRefresh(r);

  }

  return r;
}

/***************************************************************************//*!
\brief Function Biddy_Managed_Compose calculates a composition of two Boolean
       functions.

### Description
### Side effects
    Original BDD is not changed.
    Implemented for OBDD, OBDDC, ZBDDC, and TZBDD.
    For OBDDs, recursive calls use optimization: F(a=G) == NOT((NOT F)(a=G)).
### More info
    Macro Biddy_Compose(f,g,v) is defined for use with anonymous manager.
*******************************************************************************/

#ifdef __cplusplus
extern "C" {
#endif

Biddy_Edge
Biddy_Managed_Compose(Biddy_Manager MNG, Biddy_Edge f, Biddy_Edge g,
                      Biddy_Variable v)
{
  Biddy_Edge r;

  assert( f != NULL );
  assert( g != NULL );

  if (!MNG) MNG = biddyAnonymousManager;
  ZF_LOGI("Biddy_Compose");

  assert( BiddyIsOK(f) == TRUE );
  assert( BiddyIsOK(g) == TRUE );

  r = biddyNull;

  if (biddyManagerType == BIDDYTYPEOBDD) {
    /* IMPLEMENTED */
    r = BiddyManagedCompose(MNG,f,g,v);
    BiddyRefresh(r); /* not always refreshed by BiddyManagedCompose */
  } else if (biddyManagerType == BIDDYTYPEOBDDC) {
    /* IMPLEMENTED */
    r = BiddyManagedCompose(MNG,f,g,v);
    BiddyRefresh(r); /* not always refreshed by BiddyManagedCompose */
  } else if (biddyManagerType == BIDDYTYPEZBDDC) {
    /* IMPLEMENTED */
    r = BiddyManagedCompose(MNG,f,g,v);
    BiddyRefresh(r); /* not always refreshed by BiddyManagedCompose */
  } else if (biddyManagerType == BIDDYTYPETZBDD) {
    /* IMPLEMENTED */
    r = BiddyManagedCompose(MNG,f,g,v);
    BiddyRefresh(r); /* not always refreshed by BiddyManagedCompose */
  } else if ((biddyManagerType == BIDDYTYPEZBDD) ||
              (biddyManagerType == BIDDYTYPETZBDDC))
  {
    fprintf(stderr,"Biddy_Compose: this GDD type is not supported, yet!\n");
    return biddyNull;
  } else if ((biddyManagerType == BIDDYTYPEOFDDC) || (biddyManagerType == BIDDYTYPEOFDD) ||
              (biddyManagerType == BIDDYTYPEZFDDC) || (biddyManagerType == BIDDYTYPEZFDD) ||
              (biddyManagerType == BIDDYTYPETZFDDC) || (biddyManagerType == BIDDYTYPETZFDD))
  {
    fprintf(stderr,"Biddy_Compose: this GDD type is not supported, yet!\n");
    return biddyNull;
  } else {
    fprintf(stderr,"Biddy_Compose: Unsupported GDD type!\n");
    return biddyNull;
  }

  return r;
}

#ifdef __cplusplus
}
#endif

Biddy_Edge
BiddyManagedCompose(Biddy_Manager MNG, Biddy_Edge f, Biddy_Edge g,
                    Biddy_Variable v)
{
  Biddy_Edge e, t, r;
  Biddy_Variable fv,tag;
  Biddy_Edge FF, GG, HH;
  unsigned int cindex;

  assert( MNG != NULL );
  assert( f != NULL );
  assert( g != NULL );

  /* IMPLEMENTED FOR OBDD, OBDDC, ZBDDC, AND TZBDD */
  assert(
    (biddyManagerType == BIDDYTYPEOBDD) ||
    (biddyManagerType == BIDDYTYPEOBDDC) ||
    (biddyManagerType == BIDDYTYPEZBDDC) ||
    (biddyManagerType == BIDDYTYPETZBDD)
  );

  r = biddyNull;

  if (f == biddyZero) return f;

  FF = f;
  GG = g;
  HH = biddyVariableTable.table[v].variable;

  /* IF RESULT IS NOT IN THE CACHE TABLE... */
  /* TO DO: CHECK ONLY IF IT IS POSSIBLE TO EXIST IN THE CACHE */
  cindex = 0;
  if (!findOp3Cache(MNG,biddyRCCache,FF,GG,HH,&r,&cindex))
  {

    if ((biddyManagerType == BIDDYTYPEOBDDC) || (biddyManagerType == BIDDYTYPEOBDD)) {
      if ((fv=BiddyV(f)) == v) {
        r = Biddy_InvCond(BiddyManagedITE(MNG,g,BiddyT(f),BiddyE(f)),Biddy_GetMark(f));
      }
      else if (BiddyIsSmaller(v,fv)) {
        r = f;
      }
      else {
        e = BiddyManagedCompose(MNG,BiddyE(f),g,v);
        t = BiddyManagedCompose(MNG,BiddyT(f),g,v);
        /* VARIANT A: USE TRUE FOR THE FIRST CONDITION */
        /* VARIANT B: USE FALSE FOR THE FIRST CONDITION */
        if (TRUE && BiddyIsSmaller(fv,Biddy_GetTopVariable(e)) && BiddyIsSmaller(fv,Biddy_GetTopVariable(t))) {
          r = Biddy_InvCond(BiddyManagedTaggedFoaNode(MNG,fv,e,t,fv,TRUE),Biddy_GetMark(f));
          BiddyRefresh(r); /* FoaNode returns an obsolete node! */
        } else {
          r = Biddy_InvCond(BiddyManagedITE(MNG,biddyVariableTable.table[fv].variable,t,e),Biddy_GetMark(f));
        }
        addOp3Cache(MNG,biddyRCCache,FF,GG,HH,r,cindex);
      }
    }

    else if (biddyManagerType == BIDDYTYPEZBDDC) {
      if ((fv=BiddyV(f)) == v) {
        e = Biddy_InvCond(BiddyE(f),Biddy_GetMark(f));
        e = BiddyManagedTaggedFoaNode(MNG,v,e,e,0,TRUE);
        BiddyRefresh(e); /* FoaNode returns an obsolete node! */
        t = BiddyT(f);
        t = BiddyManagedTaggedFoaNode(MNG,v,t,t,0,TRUE);
        BiddyRefresh(t); /* FoaNode returns an obsolete node! */
        r = BiddyManagedITE(MNG,g,t,e);
        addOp3Cache(MNG,biddyRCCache,FF,GG,HH,r,cindex);
      }
      else if (BiddyIsSmaller(v,fv)) {
        r = BiddyManagedTaggedFoaNode(MNG,v,f,f,0,TRUE);
        BiddyRefresh(r); /* FoaNode returns an obsolete node! */
        r = BiddyManagedITE(MNG,g,biddyZero,r);
        addOp3Cache(MNG,biddyRCCache,FF,GG,HH,r,cindex);
      }
      else {
        e = BiddyManagedCompose(MNG,Biddy_InvCond(BiddyE(f),Biddy_GetMark(f)),g,v);
        t = BiddyManagedCompose(MNG,BiddyT(f),BiddyManagedChange(MNG,g,fv),v);
        t = BiddyManagedChange(MNG,t,fv);
        r = BiddyManagedXor(MNG,e,t);
        addOp3Cache(MNG,biddyRCCache,FF,GG,HH,r,cindex);
      }
    }

    else if (biddyManagerType == BIDDYTYPETZBDD) {
      tag = Biddy_GetTag(f);
      if (BiddyIsSmaller(v,tag)) {
        r = f;
      } else {
        if ((fv=BiddyV(f)) == v) {
          /* VARIANT 1 */
          /**/
          if (v == tag) {
            e = BiddyE(f);
            t = BiddyT(f);
          } else {
            e = BiddyManagedTaggedFoaNode(MNG,v,BiddyE(f),BiddyE(f),tag,TRUE);
            BiddyRefresh(e); /* FoaNode returns an obsolete node! */
            t = BiddyManagedTaggedFoaNode(MNG,v,BiddyT(f),BiddyT(f),tag,TRUE);
            BiddyRefresh(t); /* FoaNode returns an obsolete node! */
          }
          r = BiddyManagedITE(MNG,g,t,e);
          /**/
          /* VARIANT 2 - THIS COULD BE BETTER FOR SOME EXAMPLES */
          /*
          r = BiddyManagedTaggedFoaNode(MNG,v,biddyOne,biddyOne,tag,TRUE);
          BiddyRefresh(r);
          e = BiddyE(f);
          t = BiddyT(f);
          r = BiddyManagedAnd(MNG,r,BiddyManagedITE(MNG,g,t,e));
          */
          addOp3Cache(MNG,biddyRCCache,FF,GG,HH,r,cindex);
        }
        else if (BiddyIsSmaller(v,fv)) {
          r = BiddyManagedRestrict(MNG,f,v,FALSE);
          r = BiddyManagedITE(MNG,g,biddyZero,r);
          addOp3Cache(MNG,biddyRCCache,FF,GG,HH,r,cindex);
        } else {
          /* VARIANT 1 */
          /**/
          r = BiddyManagedTaggedFoaNode(MNG,fv,biddyOne,biddyOne,tag,TRUE);
          BiddyRefresh(r); /* FoaNode returns an obsolete node! */
          e = BiddyManagedCompose(MNG,BiddyE(f),g,v);
          t = BiddyManagedCompose(MNG,BiddyT(f),g,v);
          r = BiddyManagedAnd(MNG,r,BiddyManagedITE(MNG,biddyVariableTable.table[fv].variable,t,e));
          /**/
          /* VARIANT 2 - THIS SEEMS TO BE SLOWER */
          /*
          e = BiddyManagedTaggedFoaNode(MNG,fv,biddyOne,biddyZero,tag,TRUE);
          BiddyRefresh(e);
          t = BiddyManagedTaggedFoaNode(MNG,fv,biddyZero,biddyOne,tag,TRUE);
          BiddyRefresh(t);
          e = BiddyManagedAnd(MNG,e,BiddyManagedCompose(MNG,BiddyE(f),g,v));
          t = BiddyManagedAnd(MNG,t,BiddyManagedCompose(MNG,BiddyT(f),g,v));
          r = BiddyManagedXor(MNG,e,t);
          */
          addOp3Cache(MNG,biddyRCCache,FF,GG,HH,r,cindex);
        }
      }
    }

  } else {

    /* IF THE RESULT IS FROM CACHE TABLE, FRESH IT! */
    BiddyRefresh(r);

  }

  return r;
}

/***************************************************************************//*!
\brief Function Biddy_Managed_E calculates an existential quantification of
       Boolean function.

### Description
### Side effects
    Original BDD is not changed.
    Implemented for OBDD, OBDDC, ZBDDC, and TZBDD.
    Be careful: ExA F != NOT(ExA (NOT F)).
    Counterexample: Exb (AND (NOT a) b c).
### More info
    Macro Biddy_E(f,v) is defined for use with anonymous manager.
*******************************************************************************/

#ifdef __cplusplus
extern "C" {
#endif

Biddy_Edge
Biddy_Managed_E(Biddy_Manager MNG, Biddy_Edge f, Biddy_Variable v)
{
  Biddy_Edge r;

  assert( f != NULL );

  if (!MNG) MNG = biddyAnonymousManager;
  ZF_LOGI("Biddy_E");

  assert( BiddyIsOK(f) == TRUE );

  r = biddyNull;

  if (biddyManagerType == BIDDYTYPEOBDD) {
    /* IMPLEMENTED */
    r = BiddyManagedE(MNG,f,v);
    BiddyRefresh(r); /* not always refreshed by BiddyManagedE */
  } else if (biddyManagerType == BIDDYTYPEOBDDC) {
    /* IMPLEMENTED */
    r = BiddyManagedE(MNG,f,v);
    BiddyRefresh(r); /* not always refreshed by BiddyManagedE */
  } else if (biddyManagerType == BIDDYTYPEZBDDC) {
    /* IMPLEMENTED */
    r = BiddyManagedE(MNG,f,v);
    BiddyRefresh(r); /* not always refreshed by BiddyManagedE */
  } else if (biddyManagerType == BIDDYTYPETZBDD) {
    /* IMPLEMENTED */
    r = BiddyManagedE(MNG,f,v);
    BiddyRefresh(r); /* not always refreshed by BiddyManagedE */
  } else if ((biddyManagerType == BIDDYTYPEZBDD) ||
              (biddyManagerType == BIDDYTYPETZBDDC))
  {
    fprintf(stderr,"Biddy_E: this GDD type is not supported, yet!\n");
    return biddyNull;
  } else if ((biddyManagerType == BIDDYTYPEOFDDC) || (biddyManagerType == BIDDYTYPEOFDD) ||
              (biddyManagerType == BIDDYTYPEZFDDC) || (biddyManagerType == BIDDYTYPEZFDD) ||
              (biddyManagerType == BIDDYTYPETZFDDC) || (biddyManagerType == BIDDYTYPETZFDD))
  {
    fprintf(stderr,"Biddy_E: this GDD type is not supported, yet!\n");
    return biddyNull;
  } else {
    fprintf(stderr,"Biddy_E: Unsupported GDD type!\n");
    return biddyNull;
  }

  return r;
}

#ifdef __cplusplus
}
#endif

Biddy_Edge
BiddyManagedE(Biddy_Manager MNG, Biddy_Edge f, Biddy_Variable v)
{
  Biddy_Edge h;
  Biddy_Edge e, t, r;
  Biddy_Variable fv,tag;
  unsigned int cindex;

  assert( MNG != NULL );
  assert( f != NULL );

  /* IMPLEMENTED FOR OBDD, OBDDC, ZBDDC, AND TZBDD */
  assert(
    (biddyManagerType == BIDDYTYPEOBDD) ||
    (biddyManagerType == BIDDYTYPEOBDDC) ||
    (biddyManagerType == BIDDYTYPEZBDDC) ||
    (biddyManagerType == BIDDYTYPETZBDD)
  );

  r = biddyNull;

  /* LOOKING FOR SIMPLE CASE */
  if (f == biddyZero) return f;

  h = biddyNull;

  /* h MUST BE COMPATIBLE WITH CUBE */
  if ((biddyManagerType == BIDDYTYPEOBDDC) || (biddyManagerType == BIDDYTYPEOBDD)) {
    h = biddyVariableTable.table[v].variable;
  }
  else if (biddyManagerType == BIDDYTYPEZBDDC) {
    h = biddyVariableTable.table[v].element;
  }
  else if (biddyManagerType == BIDDYTYPETZBDD) {
    h = biddyVariableTable.table[v].variable;
  }

  /* IF RESULT IS NOT IN THE CACHE TABLE... */
  cindex = 0;
  if ((((uintptr_t) f) > ((uintptr_t) biddyOne)) ?
      !findOp3Cache(MNG,biddyEACache,biddyOne,f,h,&r,&cindex) :
      !findOp3Cache(MNG,biddyEACache,f,biddyOne,h,&r,&cindex))
  {

    if ((biddyManagerType == BIDDYTYPEOBDDC) || (biddyManagerType == BIDDYTYPEOBDD)) {
      if ((fv=BiddyV(f)) == v) {
        r = BiddyManagedOr(MNG,
                           Biddy_InvCond(BiddyE(f),Biddy_GetMark(f)),
                           Biddy_InvCond(BiddyT(f),Biddy_GetMark(f)));
      }
      else if (BiddyIsSmaller(v,fv)) {
        r = f;
      }
      else {
        e = BiddyManagedE(MNG,Biddy_InvCond(BiddyE(f),Biddy_GetMark(f)),v);
        t = BiddyManagedE(MNG,Biddy_InvCond(BiddyT(f),Biddy_GetMark(f)),v);
        r = BiddyManagedTaggedFoaNode(MNG,fv,e,t,fv,TRUE);
        BiddyRefresh(r); /* FoaNode returns an obsolete node! */
      }
    }

    else if (biddyManagerType == BIDDYTYPEZBDDC) {
      if ((fv=BiddyV(f)) == v) {
        r = BiddyManagedOr(MNG,
                           Biddy_InvCond(BiddyE(f),Biddy_GetMark(f)),
                           BiddyT(f));
        r = BiddyManagedTaggedFoaNode(MNG,v,r,r,0,TRUE);
        BiddyRefresh(r); /* FoaNode returns an obsolete node! */
      }
      else if (BiddyIsSmaller(v,fv)) {
        r = BiddyManagedTaggedFoaNode(MNG,v,f,f,0,TRUE);
        BiddyRefresh(r); /* FoaNode returns an obsolete node! */
      }
      else {
        e = BiddyManagedE(MNG,Biddy_InvCond(BiddyE(f),Biddy_GetMark(f)),v);
        t = BiddyManagedE(MNG,BiddyT(f),v);
        r = BiddyManagedTaggedFoaNode(MNG,fv,e,t,0,TRUE);
        BiddyRefresh(r); /* FoaNode returns an obsolete node! */
      }
    }

    else if (biddyManagerType == BIDDYTYPETZBDD) {
      tag = Biddy_GetTag(f);
      if (BiddyIsSmaller(v,tag)) {
        r = f;
      } else {
        fv = BiddyV(f);
        if (v == fv) {
          r = BiddyManagedOr(MNG,BiddyE(f),BiddyT(f));
          if (v != tag) {
            r = BiddyManagedTaggedFoaNode(MNG,v,r,r,tag,TRUE);
            BiddyRefresh(r); /* FoaNode returns an obsolete node! */
          }
        } else if (BiddyIsSmaller(v,fv)) {
          if (v == tag) {
            r = Biddy_Managed_IncTag(MNG,f);
          } else {
            r = f;
            Biddy_SetTag(r,v);
            r = Biddy_Managed_IncTag(MNG,r);
            r = BiddyManagedTaggedFoaNode(MNG,v,r,r,tag,TRUE);
            BiddyRefresh(r); /* FoaNode returns an obsolete node! */
          }
        } else {
          e = BiddyManagedE(MNG,BiddyE(f),v);
          t = BiddyManagedE(MNG,BiddyT(f),v);
          r = BiddyManagedTaggedFoaNode(MNG,fv,e,t,tag,TRUE);
          BiddyRefresh(r); /* FoaNode returns an obsolete node! */
        }
      }
    }

    /* CACHE FOR Ex(v)(f*1), f*1 <=> 1*f */
    if (((uintptr_t) f) > ((uintptr_t) biddyOne)) {
      addOp3Cache(MNG,biddyEACache,biddyOne,f,h,r,cindex);
    } else {
      addOp3Cache(MNG,biddyEACache,f,biddyOne,h,r,cindex);
    }

  } else {

    /* IF THE RESULT IS FROM CACHE TABLE, REFRESH IT! */
    BiddyRefresh(r);

  }

  return r;
}

/***************************************************************************//*!
\brief Function Biddy_Managed_A calculates an universal quantification of
       Boolean function.

### Description
### Side effects
    Original BDD is not changed.
    Implemented for OBDDC. Prototyped for OBDD.
    Prototyped for ZBDDC and TZBDD.
    Be careful: AxA F != NOT(AxA (NOT F)).
    Counterexample: Axb (AND (NOT a) b c).
### More info
    Macro Biddy_A(f,v) is defined for use with anonymous manager.
*******************************************************************************/

#ifdef __cplusplus
extern "C" {
#endif

Biddy_Edge
Biddy_Managed_A(Biddy_Manager MNG, Biddy_Edge f, Biddy_Variable v)
{
  Biddy_Edge r;

  assert( f != NULL );

  if (!MNG) MNG = biddyAnonymousManager;
  ZF_LOGI("Biddy_A");

  assert( BiddyIsOK(f) == TRUE );

  r = biddyNull;

  if (biddyManagerType == BIDDYTYPEOBDD) {
    /* PROTOTYPED */
    r = BiddyManagedNot(MNG,BiddyManagedE(MNG,BiddyManagedNot(MNG,f),v));
    BiddyRefresh(r); /* not always refreshed by BiddyManagedNot */
  } else if (biddyManagerType == BIDDYTYPEOBDDC) {
    /* IMPLEMENTED */
    r = BiddyManagedA(MNG,f,v);
    BiddyRefresh(r); /* not always refreshed by BiddyManagedA */
  } else if (biddyManagerType == BIDDYTYPEZBDDC) {
    /* PROTOTYPED */
    r = BiddyManagedNot(MNG,BiddyManagedE(MNG,BiddyManagedNot(MNG,f),v));
    BiddyRefresh(r); /* not always refreshed by BiddyManagedNot */
  } else if (biddyManagerType == BIDDYTYPETZBDD) {
    /* PROTOTYPED */
    r = BiddyManagedNot(MNG,BiddyManagedE(MNG,BiddyManagedNot(MNG,f),v));
    BiddyRefresh(r); /* not always refreshed by BiddyManagedNot */
  } else if ((biddyManagerType == BIDDYTYPEZBDD) ||
              (biddyManagerType == BIDDYTYPETZBDDC))
  {
    fprintf(stderr,"Biddy_A: this GDD type is not supported, yet!\n");
    return biddyNull;
  } else if ((biddyManagerType == BIDDYTYPEOFDDC) || (biddyManagerType == BIDDYTYPEOFDD) ||
              (biddyManagerType == BIDDYTYPEZFDDC) || (biddyManagerType == BIDDYTYPEZFDD) ||
              (biddyManagerType == BIDDYTYPETZFDDC) || (biddyManagerType == BIDDYTYPETZFDD))
  {
    fprintf(stderr,"Biddy_A: this GDD type is not supported, yet!\n");
    return biddyNull;
  } else {
    fprintf(stderr,"Biddy_A: Unsupported GDD type!\n");
    return biddyNull;
  }

  return r;
}

#ifdef __cplusplus
}
#endif

Biddy_Edge
BiddyManagedA(Biddy_Manager MNG, Biddy_Edge f, Biddy_Variable v)
{
  Biddy_Edge r;

  assert( MNG != NULL );
  assert( f != NULL );

  /* IMPLEMENTED FOR OBDDC, ONLY */
  assert( biddyManagerType == BIDDYTYPEOBDDC );

  /* LOOKING FOR SIMPLE CASE */
  if (Biddy_IsTerminal(f)) return f;

  /* PROCEED BY CALCULATING NOT (EX v NOT f) */
  r = Biddy_Inv(BiddyManagedE(MNG,Biddy_Inv(f),v));

  return r;
}

/***************************************************************************//*!
\brief Function Biddy_Managed_IsVariableDependent returns TRUE iff variable is
       dependent on others in a function.

### Description
    A variable is dependent on others in a function iff universal
    quantification of this variable returns constant FALSE.
### Side effects
    Prototyped for OBDDs (via xA, calculating full universal quantification
    is less efficient as direct implementation in CUDD).
### More info
    Macro Biddy_IsVariableDependent(f,v) is defined for use with anonymous
    manager.
*******************************************************************************/

#ifdef __cplusplus
extern "C" {
#endif

Biddy_Boolean
Biddy_Managed_IsVariableDependent(Biddy_Manager MNG, Biddy_Edge f, Biddy_Variable v)
{
  Biddy_Edge xa;
  Biddy_Boolean r;

  if (!MNG) MNG = biddyAnonymousManager;
  ZF_LOGI("Biddy_IsVariableDependent");

  r = FALSE;

  if (biddyManagerType == BIDDYTYPEOBDD) {
    /* PROTOTYPED */
    xa = BiddyManagedA(MNG,f,v);
    r = (xa == biddyZero);
  } else if (biddyManagerType == BIDDYTYPEOBDDC) {
    /* PROTOTYPED */
    xa = BiddyManagedA(MNG,f,v);
    r = (xa == biddyZero);
  } else if ((biddyManagerType == BIDDYTYPEZBDDC) ||
              (biddyManagerType == BIDDYTYPETZBDD))
  {
    fprintf(stderr,"Biddy_IsVariableDependent: this GDD type is not supported, yet!\n");
    return FALSE;
  } else if ((biddyManagerType == BIDDYTYPEZBDD) ||
              (biddyManagerType == BIDDYTYPETZBDDC))
  {
    fprintf(stderr,"Biddy_IsVariableDependent: this GDD type is not supported, yet!\n");
    return FALSE;
  } else if ((biddyManagerType == BIDDYTYPEOFDDC) || (biddyManagerType == BIDDYTYPEOFDD) ||
              (biddyManagerType == BIDDYTYPEZFDDC) || (biddyManagerType == BIDDYTYPEZFDD) ||
              (biddyManagerType == BIDDYTYPETZFDDC) || (biddyManagerType == BIDDYTYPETZFDD))
  {
    fprintf(stderr,"Biddy_IsVariableDependent: this GDD type is not supported, yet!\n");
    return FALSE;
  } else {
    fprintf(stderr,"Biddy_IsVariableDependent: Unsupported GDD type!\n");
    return FALSE;
  }

  return r;
}

#ifdef __cplusplus
}
#endif

/***************************************************************************//*!
\brief Function Biddy_Managed_ExistAbstract existentially abstracts all the
       variables in cube from f.

### Description
### Side effects
    Original BDD is not changed.
    Implemented for OBDD, OBDDC, ZBDDC, and TZBDD.
### More info
    Macro Biddy_ExistAbstract(f,cube) is defined for use with anonymous manager.
*******************************************************************************/

#ifdef __cplusplus
extern "C" {
#endif

Biddy_Edge
Biddy_Managed_ExistAbstract(Biddy_Manager MNG, Biddy_Edge f, Biddy_Edge cube)
{
  Biddy_Edge r;

  assert( f != NULL );
  assert( cube != NULL );

  if (!MNG) MNG = biddyAnonymousManager;
  ZF_LOGI("Biddy_ExistAbstract");

  assert( BiddyIsOK(f) == TRUE );
  assert( BiddyIsOK(cube) == TRUE );

  r = biddyNull;

  if (biddyManagerType == BIDDYTYPEOBDD) {
    /* IMPLEMENTED */
    r = BiddyManagedExistAbstract(MNG,f,cube);
    BiddyRefresh(r); /* not always refreshed by BiddyManagedExistAbstract */
  } else if (biddyManagerType == BIDDYTYPEOBDDC) {
    /* IMPLEMENTED */
    r = BiddyManagedExistAbstract(MNG,f,cube);
    BiddyRefresh(r); /* not always refreshed by BiddyManagedExistAbstract */
  } else if (biddyManagerType == BIDDYTYPEZBDDC) {
    /* IMPLEMENTED */
    r = BiddyManagedExistAbstract(MNG,f,cube);
    BiddyRefresh(r); /* not always refreshed by BiddyManagedExistAbstract */
  } else if (biddyManagerType == BIDDYTYPETZBDD) {
    /* IMPLEMENTED */
    r = BiddyManagedExistAbstract(MNG,f,cube);
    BiddyRefresh(r); /* not always refreshed by BiddyManagedExistAbstract */
  } else if ((biddyManagerType == BIDDYTYPEZBDD) ||
              (biddyManagerType == BIDDYTYPETZBDDC))
  {
    fprintf(stderr,"Biddy_ExistAbstract: this GDD type is not supported, yet!\n");
    return biddyNull;
  } else if ((biddyManagerType == BIDDYTYPEOFDDC) || (biddyManagerType == BIDDYTYPEOFDD) ||
              (biddyManagerType == BIDDYTYPEZFDDC) || (biddyManagerType == BIDDYTYPEZFDD) ||
              (biddyManagerType == BIDDYTYPETZFDDC) || (biddyManagerType == BIDDYTYPETZFDD))
  {
    fprintf(stderr,"Biddy_ExistAbstract: this GDD type is not supported, yet!\n");
    return biddyNull;
  } else {
    fprintf(stderr,"Biddy_ExistAbstract: Unsupported GDD type!\n");
    return biddyNull;
  }

  return r;
}

#ifdef __cplusplus
}
#endif

Biddy_Edge
BiddyManagedExistAbstract(Biddy_Manager MNG, Biddy_Edge f, Biddy_Edge cube)
{
  Biddy_Edge e, t, r;
  Biddy_Variable fv,cv,tag;
  unsigned int cindex;

  assert( MNG != NULL );
  assert( f != NULL );
  assert( cube != NULL );

  /* IMPLEMENTED FOR OBDD, OBDDC, ZBDDC, AND TZBDD */
  assert(
    (biddyManagerType == BIDDYTYPEOBDD) ||
    (biddyManagerType == BIDDYTYPEOBDDC) ||
    (biddyManagerType == BIDDYTYPEZBDDC) ||
    (biddyManagerType == BIDDYTYPETZBDD)
  );

  r = biddyNull;

  /* LOOKING FOR SIMPLE CASE */
  if (f == biddyZero) return f;

  /* IF RESULT IS NOT IN THE CACHE TABLE... */
  cindex = 0;
  if ((((uintptr_t) f) > ((uintptr_t) biddyOne)) ?
      !findOp3Cache(MNG,biddyEACache,biddyOne,f,cube,&r,&cindex) :
      !findOp3Cache(MNG,biddyEACache,f,biddyOne,cube,&r,&cindex))
  {

    if ((biddyManagerType == BIDDYTYPEOBDDC) || (biddyManagerType == BIDDYTYPEOBDD)) {
      fv = BiddyV(f);
      cv = BiddyV(cube);
      while (!Biddy_IsTerminal(cube) && BiddyIsSmaller(cv,fv)) {
        cube = BiddyT(cube);
        cv = BiddyV(cube);
      }
      if (Biddy_IsTerminal(cube)) {
        return f;
      }
      if (cv == fv) {
        e = BiddyManagedExistAbstract(MNG,
              Biddy_InvCond(BiddyE(f),Biddy_GetMark(f)),BiddyT(cube));
        t = BiddyManagedExistAbstract(MNG,
              Biddy_InvCond(BiddyT(f),Biddy_GetMark(f)),BiddyT(cube));
        r = BiddyManagedOr(MNG,e,t);
      } else {
        e = BiddyManagedExistAbstract(MNG,
              Biddy_InvCond(BiddyE(f),Biddy_GetMark(f)),cube);
        t = BiddyManagedExistAbstract(MNG,
              Biddy_InvCond(BiddyT(f),Biddy_GetMark(f)),cube);
        r = BiddyManagedTaggedFoaNode(MNG,fv,e,t,fv,TRUE);
        BiddyRefresh(r); /* FoaNode returns an obsolete node! */
      }
    }

    else if (biddyManagerType == BIDDYTYPEZBDDC) {
      if (Biddy_IsTerminal(cube)) {
        return f;
      }
      cv = BiddyV(cube);
      fv = BiddyV(f);
      if (BiddyIsSmaller(cv,fv)) {
        r = BiddyManagedExistAbstract(MNG,f,BiddyT(cube));
        r = BiddyManagedTaggedFoaNode(MNG,cv,r,r,0,TRUE);
        BiddyRefresh(r); /* FoaNode returns an obsolete node! */
      }
      else if (cv == fv) {
        e = BiddyManagedExistAbstract(MNG,
              Biddy_InvCond(BiddyE(f),Biddy_GetMark(f)),BiddyT(cube));
        t = BiddyManagedExistAbstract(MNG,BiddyT(f),BiddyT(cube));
        r = BiddyManagedOr(MNG,e,t);
        r = BiddyManagedTaggedFoaNode(MNG,cv,r,r,0,TRUE);
        BiddyRefresh(r); /* FoaNode returns an obsolete node! */
      } else {
        e = BiddyManagedExistAbstract(MNG,
              Biddy_InvCond(BiddyE(f),Biddy_GetMark(f)),cube);
        t = BiddyManagedExistAbstract(MNG,BiddyT(f),cube);
        r = BiddyManagedTaggedFoaNode(MNG,fv,e,t,0,TRUE);
        BiddyRefresh(r); /* FoaNode returns an obsolete node! */
      }
    }

    else if (biddyManagerType == BIDDYTYPETZBDD) {
      tag = Biddy_GetTag(f);
      cv = BiddyV(cube);
      while (!Biddy_IsTerminal(cube) && BiddyIsSmaller(cv,tag)) {
        cube = BiddyT(cube);
        cv = BiddyV(cube);
      }
      if (Biddy_IsTerminal(cube)) {
        return f;
      }
      fv = BiddyV(f);

      /* VARIANT 1 */
      /*
      if (cv == tag) {
        if (tag == fv) {
          e = BiddyManagedExistAbstract(MNG,BiddyE(f),BiddyT(cube));
          t = BiddyManagedExistAbstract(MNG,BiddyT(f),BiddyT(cube));
          r = BiddyManagedOr(MNG,e,t);
        } else {
          r = Biddy_Managed_IncTag(MNG,f);
          r = BiddyManagedExistAbstract(MNG,r,BiddyT(cube));
        }
      } else if (BiddyIsSmaller(cv,fv)) {
        r = f;
        Biddy_SetTag(r,cv);
        r = Biddy_Managed_IncTag(MNG,r);
        r = BiddyManagedExistAbstract(MNG,r,BiddyT(cube));
        r = BiddyManagedTaggedFoaNode(MNG,cv,r,r,tag,TRUE);
        BiddyRefresh(r); / * FoaNode returns an obsolete node! * /
      } else if (cv == fv) {
        e = BiddyManagedExistAbstract(MNG,BiddyE(f),BiddyT(cube));
        t = BiddyManagedExistAbstract(MNG,BiddyT(f),BiddyT(cube));
        r = BiddyManagedOr(MNG,e,t);
        r = BiddyManagedTaggedFoaNode(MNG,cv,r,r,tag,TRUE);
        BiddyRefresh(r); / * FoaNode returns an obsolete node! * /
      } else {
        e = BiddyManagedExistAbstract(MNG,BiddyE(f),cube);
        t = BiddyManagedExistAbstract(MNG,BiddyT(f),cube);
        r = BiddyManagedTaggedFoaNode(MNG,fv,e,t,tag,TRUE);
        BiddyRefresh(r); / * FoaNode returns an obsolete node! * /
      }
      */

      /* VARIANT 2 */
      /**/
      if (cv == fv) {
        e = BiddyManagedExistAbstract(MNG,BiddyE(f),BiddyT(cube));
        t = BiddyManagedExistAbstract(MNG,BiddyT(f),BiddyT(cube));
        r = BiddyManagedOr(MNG,e,t);
        if (cv != tag) {
          r = BiddyManagedTaggedFoaNode(MNG,cv,r,r,tag,TRUE);
          BiddyRefresh(r); /* FoaNode returns an obsolete node! */
        }
      } else if (BiddyIsSmaller(cv,fv)) {
        if (cv == tag) {
          r = Biddy_Managed_IncTag(MNG,f);
          r = BiddyManagedExistAbstract(MNG,r,BiddyT(cube));
        } else {
          r = f;
          Biddy_SetTag(r,cv);
          r = Biddy_Managed_IncTag(MNG,r);
          r = BiddyManagedExistAbstract(MNG,r,BiddyT(cube));
          r = BiddyManagedTaggedFoaNode(MNG,cv,r,r,tag,TRUE);
          BiddyRefresh(r); /* FoaNode returns an obsolete node! */
        }
      } else {
        e = BiddyManagedExistAbstract(MNG,BiddyE(f),cube);
        t = BiddyManagedExistAbstract(MNG,BiddyT(f),cube);
        r = BiddyManagedTaggedFoaNode(MNG,fv,e,t,tag,TRUE);
        BiddyRefresh(r); /* FoaNode returns an obsolete node! */
      }
      /**/

    }

    /* CACHE EVERYTHING */
    if (((uintptr_t) f) > ((uintptr_t) biddyOne)) {
      addOp3Cache(MNG,biddyEACache,biddyOne,f,cube,r,cindex);
    } else {
      addOp3Cache(MNG,biddyEACache,f,biddyOne,cube,r,cindex);
    }

  } else {

    /* IF THE RESULT IS FROM CACHE TABLE, REFRESH IT! */
    BiddyRefresh(r);

  }

  return r;
}

/***************************************************************************//*!
\brief Function Biddy_Managed_UnivAbstract universally abstracts all the
       variables in cube from f.

### Description
### Side effects
    Original BDD is not changed.
    Implemented for OBDDC. Prototyped for OBDD.
    Prototyped for ZBDDC and TZBDD.
### More info
    Macro Biddy_UnivAbstract(f,cube) is defined for use with anonymous manager.
*******************************************************************************/

#ifdef __cplusplus
extern "C" {
#endif

Biddy_Edge
Biddy_Managed_UnivAbstract(Biddy_Manager MNG, Biddy_Edge f, Biddy_Edge cube)
{
  Biddy_Edge r;

  assert( f != NULL );
  assert( cube != NULL );

  if (!MNG) MNG = biddyAnonymousManager;
  ZF_LOGI("Biddy_UnivAbstract");

  assert( BiddyIsOK(f) == TRUE );
  assert( BiddyIsOK(cube) == TRUE );

  r = biddyNull;

  if (biddyManagerType == BIDDYTYPEOBDD) {
    /* PROTOTYPED */
    r = BiddyManagedNot(MNG,BiddyManagedExistAbstract(MNG,BiddyManagedNot(MNG,f),cube));
    BiddyRefresh(r); /* not always refreshed by BiddyManagedNot */
  } else if (biddyManagerType == BIDDYTYPEOBDDC) {
    /* IMPLEMENTED */
    r = BiddyManagedUnivAbstract(MNG,f,cube);
    BiddyRefresh(r); /* not always refreshed by BiddyManagedUnivAbstract */
  } else if (biddyManagerType == BIDDYTYPEZBDDC) {
    /* PROTOTYPED */
    r = BiddyManagedNot(MNG,BiddyManagedExistAbstract(MNG,BiddyManagedNot(MNG,f),cube));
    BiddyRefresh(r); /* not always refreshed by BiddyManagedNot */
  } else if (biddyManagerType == BIDDYTYPETZBDD) {
    /* PROTOTYPED */
    r = BiddyManagedNot(MNG,BiddyManagedExistAbstract(MNG,BiddyManagedNot(MNG,f),cube));
    BiddyRefresh(r); /* not always refreshed by BiddyManagedNot */
  } else if ((biddyManagerType == BIDDYTYPEZBDD) ||
              (biddyManagerType == BIDDYTYPETZBDDC))
  {
    fprintf(stderr,"Biddy_UnivAbstract: this GDD type is not supported, yet!\n");
    return biddyNull;
  } else if ((biddyManagerType == BIDDYTYPEOFDDC) || (biddyManagerType == BIDDYTYPEOFDD) ||
              (biddyManagerType == BIDDYTYPEZFDDC) || (biddyManagerType == BIDDYTYPEZFDD) ||
              (biddyManagerType == BIDDYTYPETZFDDC) || (biddyManagerType == BIDDYTYPETZFDD))
  {
    fprintf(stderr,"Biddy_UnivAbstract: this GDD type is not supported, yet!\n");
    return biddyNull;
  } else {
    fprintf(stderr,"Biddy_UnivAbstract: Unsupported GDD type!\n");
    return biddyNull;
  }

  return r;
}

#ifdef __cplusplus
}
#endif

Biddy_Edge
BiddyManagedUnivAbstract(Biddy_Manager MNG, Biddy_Edge f, Biddy_Edge cube)
{
  Biddy_Edge r;

  assert( MNG != NULL );
  assert( f != NULL );
  assert( cube != NULL );

  /* IMPLEMENTED FOR OBDDC, ONLY */
  assert( biddyManagerType == BIDDYTYPEOBDDC );

  /* LOOKING FOR SIMPLE CASE */
  if (Biddy_IsTerminal(f)) return f;

  /* PROCEED BY CALCULATING NOT (EX cube NOT f) */
  r = Biddy_Inv(BiddyManagedExistAbstract(MNG,Biddy_Inv(f),cube));

  return r;
}

/***************************************************************************//*!
\brief Function Biddy_Managed_AndAbstract calculates the AND of two BDDs and
       simultaneously (existentially) abstracts the variables in cube.

### Description
### Side effects
    Original BDD is not changed.
    Implemented for OBDD, OBDDC, ZBDDC, and TZBDD.
### More info
    Macro Biddy_AndAbstract(f,g,cube) is defined for use with anonymous manager.
*******************************************************************************/

#ifdef __cplusplus
extern "C" {
#endif

Biddy_Edge
Biddy_Managed_AndAbstract(Biddy_Manager MNG, Biddy_Edge f, Biddy_Edge g,
                          Biddy_Edge cube)
{
  Biddy_Edge r;

  assert( f != NULL );
  assert( g != NULL );
  assert( cube != NULL );

  if (!MNG) MNG = biddyAnonymousManager;
  ZF_LOGI("Biddy_AndAbstract");

  assert( BiddyIsOK(f) == TRUE );
  assert( BiddyIsOK(g) == TRUE );
  assert( BiddyIsOK(cube) == TRUE );

  r = biddyNull;

  if (biddyManagerType == BIDDYTYPEOBDD) {
    /* IMPLEMENTED */
    r = BiddyManagedAndAbstract(MNG,f,g,cube);
    BiddyRefresh(r); /* not always refreshed by BiddyManagedAndAbstract */
  } else if (biddyManagerType == BIDDYTYPEOBDDC) {
    /* IMPLEMENTED */
    r = BiddyManagedAndAbstract(MNG,f,g,cube);
    BiddyRefresh(r); /* not always refreshed by BiddyManagedAndAbstract */
  } else if (biddyManagerType == BIDDYTYPEZBDDC) {
    /* IMPLEMENTED */
    r = BiddyManagedAndAbstract(MNG,f,g,cube);
    BiddyRefresh(r); /* not always refreshed by BiddyManagedAndAbstract */
  } else if (biddyManagerType == BIDDYTYPETZBDD) {
    /* IMPLEMENTED */
    r = BiddyManagedAndAbstract(MNG,f,g,cube);
    BiddyRefresh(r); /* not always refreshed by BiddyManagedAndAbstract */
  } else if ((biddyManagerType == BIDDYTYPEZBDD) ||
              (biddyManagerType == BIDDYTYPETZBDDC))
  {
    fprintf(stderr,"Biddy_AndAbstract: this GDD type is not supported, yet!\n");
    return biddyNull;
  } else if ((biddyManagerType == BIDDYTYPEOFDDC) || (biddyManagerType == BIDDYTYPEOFDD) ||
              (biddyManagerType == BIDDYTYPEZFDDC) || (biddyManagerType == BIDDYTYPEZFDD) ||
              (biddyManagerType == BIDDYTYPETZFDDC) || (biddyManagerType == BIDDYTYPETZFDD))
  {
    fprintf(stderr,"Biddy_AndAbstract: this GDD type is not supported, yet!\n");
    return biddyNull;
  } else {
    fprintf(stderr,"Biddy_AndAbstract: Unsupported GDD type!\n");
    return biddyNull;
  }

  return r;
}

#ifdef __cplusplus
}
#endif

Biddy_Edge
BiddyManagedAndAbstract(Biddy_Manager MNG, Biddy_Edge f, Biddy_Edge g,
                        Biddy_Edge cube)
{
  Biddy_Edge f0,f1,g0,g1;
  Biddy_Edge e, t, r;
  Biddy_Variable fv,gv,minv,cv;
  Biddy_Variable ftag,gtag,mintag;
  Biddy_Boolean cgt;
  unsigned int cindex;

  assert( MNG != NULL );
  assert( f != NULL );
  assert( g != NULL );
  assert( cube != NULL );

  /* IMPLEMENTED FOR OBDD, OBDDC, ZBDDC, AND TZBDDC */
  assert(
    (biddyManagerType == BIDDYTYPEOBDD) ||
    (biddyManagerType == BIDDYTYPEOBDDC) ||
    (biddyManagerType == BIDDYTYPEZBDDC) ||
    (biddyManagerType == BIDDYTYPETZBDD)
  );

  r = biddyNull;

  /* LOOKING FOR SIMPLE CASE */
  if ((f == biddyZero) || (g == biddyZero)) {
    return biddyZero;
  } else if (f == biddyOne) {
    return BiddyManagedExistAbstract(MNG,g,cube);
  } else if (g == biddyOne) {
    return BiddyManagedExistAbstract(MNG,f,cube);
  }

  /* NORMALIZATION */
  if (((uintptr_t) f) > ((uintptr_t) g)) {
    exchangeEdges(&f,&g);
  }

  /* IF RESULT IS NOT IN THE CACHE TABLE... */
  cindex = 0;
  if (!findOp3Cache(MNG,biddyEACache,f,g,cube,&r,&cindex))
  {

    if ((biddyManagerType == BIDDYTYPEOBDDC) || (biddyManagerType == BIDDYTYPEOBDD)) {
      fv = BiddyV(f);
      gv = BiddyV(g);
      if (BiddyIsSmaller(fv,gv)) {
        f0 = Biddy_InvCond(BiddyE(f),Biddy_GetMark(f));
        f1 = Biddy_InvCond(BiddyT(f),Biddy_GetMark(f));
        g0 = g;
        g1 = g;
        minv = fv;
      } else if (BiddyIsSmaller(gv,fv)) {
        f0 = f;
        f1 = f;
        g0 = Biddy_InvCond(BiddyE(g),Biddy_GetMark(g));
        g1 = Biddy_InvCond(BiddyT(g),Biddy_GetMark(g));
        minv = gv;
      } else {
        f0 = Biddy_InvCond(BiddyE(f),Biddy_GetMark(f));
        f1 = Biddy_InvCond(BiddyT(f),Biddy_GetMark(f));
        g0 = Biddy_InvCond(BiddyE(g),Biddy_GetMark(g));
        g1 = Biddy_InvCond(BiddyT(g),Biddy_GetMark(g));
        minv = fv;
      }
      cv = BiddyV(cube);
      while (!Biddy_IsTerminal(cube) && BiddyIsSmaller(cv,minv)) {
        cube = BiddyT(cube);
        cv = BiddyV(cube);
      }
      if (Biddy_IsTerminal(cube)) {
        return BiddyManagedAnd(MNG,f,g);
        /* r = BiddyManagedAnd(MNG,f,g); */  /* I AM NOT SURE IF THIS SHOULD BE CACHED */
      } else {
        if (minv == cv) {
          /* Tricky optimizations are from: */
          /* B. Yang et al. A Performance Study of BDD-Based Model Checking. 1998. */
          /* http://fmv.jku.at/papers/YangEtAl-FMCAD98.pdf */
          e = BiddyManagedAndAbstract(MNG,f0,g0,BiddyT(cube));
          if (e == biddyOne) return biddyOne;
          if ((e == f1) || (e == g1)) {
            return e;
            /* r = e */ /* I AM NOT SURE IF THIS SHOULD BE CACHED */
          } else {
            if (e == Biddy_Inv(f1)) f1 = biddyOne; /* this is useful only for OBDDC, but not wrong for OBDD */
            if (e == Biddy_Inv(g1)) g1 = biddyOne; /* this is useful only for OBDDC, but not wrong for OBDD */
            t = BiddyManagedAndAbstract(MNG,f1,g1,BiddyT(cube));
            r = BiddyManagedOr(MNG,e,t);
          }
        } else {
          e = BiddyManagedAndAbstract(MNG,f0,g0,cube);
          t = BiddyManagedAndAbstract(MNG,f1,g1,cube);
          r = BiddyManagedTaggedFoaNode(MNG,minv,e,t,minv,TRUE);
          BiddyRefresh(r); /* FoaNode returns an obsolete node! */
        }
      }
    }

    else if (biddyManagerType == BIDDYTYPEZBDDC) {
      if (Biddy_IsTerminal(cube)) {
        return BiddyManagedAnd(MNG,f,g);
        /* r = BiddyManagedAnd(MNG,f,g); */  /* I AM NOT SURE IF THIS SHOULD BE CACHED */
      } else {
        fv = BiddyV(f);
        gv = BiddyV(g);
        minv = BiddyIsSmaller(fv,gv) ? fv : gv;
        if (fv != gv) {
          if (fv == minv) {
            if (Biddy_GetMark(f)) {
              /* return BiddyManagedAndAbstract(MNG,Biddy_Inv(BiddyE(f)),g,cube); */
              r = BiddyManagedAndAbstract(MNG,Biddy_Inv(BiddyE(f)),g,cube);
            } else {
              /* return BiddyManagedAndAbstract(MNG,BiddyE(f),g,cube); */
              r = BiddyManagedAndAbstract(MNG,BiddyE(f),g,cube);
            }
          } else {
            if (Biddy_GetMark(g)) {
              /* return BiddyManagedAndAbstract(MNG,f,Biddy_Inv(BiddyE(g)),cube); */
              r = BiddyManagedAndAbstract(MNG,f,Biddy_Inv(BiddyE(g)),cube);
            } else {
              /* return BiddyManagedAndAbstract(MNG,f,BiddyE(g),cube); */
              r = BiddyManagedAndAbstract(MNG,f,BiddyE(g),cube);
            }
          }
        } else {
          cv = BiddyV(cube);
          if (BiddyIsSmaller(cv,minv)) {
            r = BiddyManagedAndAbstract(MNG,f,g,BiddyT(cube));
            r = BiddyManagedTaggedFoaNode(MNG,cv,r,r,0,TRUE);
            BiddyRefresh(r); /* FoaNode returns an obsolete node! */
          }
          else if (cv == minv) {
            f0 = Biddy_InvCond(BiddyE(f),Biddy_GetMark(f));
            f1 = BiddyT(f);
            g0 = Biddy_InvCond(BiddyE(g),Biddy_GetMark(g));
            g1 = BiddyT(g);
            e = BiddyManagedAndAbstract(MNG,f0,g0,BiddyT(cube));
            t = BiddyManagedAndAbstract(MNG,f1,g1,BiddyT(cube));
            r = BiddyManagedOr(MNG,e,t);
            r = BiddyManagedTaggedFoaNode(MNG,cv,r,r,0,TRUE);
            BiddyRefresh(r); /* FoaNode returns an obsolete node! */
          } else {
            f0 = Biddy_InvCond(BiddyE(f),Biddy_GetMark(f));
            f1 = BiddyT(f);
            g0 = Biddy_InvCond(BiddyE(g),Biddy_GetMark(g));
            g1 = BiddyT(g);
            e = BiddyManagedAndAbstract(MNG,f0,g0,cube);
            t = BiddyManagedAndAbstract(MNG,f1,g1,cube);
            r = BiddyManagedTaggedFoaNode(MNG,minv,e,t,0,TRUE);
            BiddyRefresh(r); /* FoaNode returns an obsolete node! */
          }
        }
      }
    }

    else if (biddyManagerType == BIDDYTYPETZBDD) {
      ftag = Biddy_GetTag(f);
      gtag = Biddy_GetTag(g);
      mintag = BiddyIsSmaller(ftag,gtag) ? ftag : gtag;
      cv = BiddyV(cube);
      while (!Biddy_IsTerminal(cube) && BiddyIsSmaller(cv,mintag)) {
        cube = BiddyT(cube);
        cv = BiddyV(cube);
      }
      if (Biddy_IsTerminal(cube)) {
        return BiddyManagedAnd(MNG,f,g);
        /* r = BiddyManagedAnd(MNG,f,g); */  /* I AM NOT SURE IF THIS SHOULD BE CACHED */
      } else {

        /* VARIANT 1 */
        /*
        fv = BiddyV(f);
        gv = BiddyV(g);
        minv = BiddyIsSmaller(fv,gv) ? fv : gv;
        if (BiddyIsSmaller(fv,gtag)) {
          f0 = BiddyE(f);
          f1 = BiddyT(f);
          g0 = g1 = g;
        } else if (BiddyIsSmaller(gv,ftag)) {
          f0 = f1 = f;
          g0 = BiddyE(g);
          g1 = BiddyT(g);
        } else {
          if (minv == fv) {
            f0 = BiddyE(f);
            f1 = BiddyT(f);
          } else {
            f0 = f;
            Biddy_SetTag(f0,minv);
            f0 = Biddy_Managed_IncTag(MNG,f0);
            f1 = biddyZero;
          }
          if (minv == gv) {
            g0 = BiddyE(g);
            g1 = BiddyT(g);
          } else {
            g0 = g;
            Biddy_SetTag(g0,minv);
            g0 = Biddy_Managed_IncTag(MNG,g0);
            g1 = biddyZero;
          }
        }
        if (BiddyIsSmaller(minv,cv)) {
          e = BiddyManagedAndAbstract(MNG,f0,g0,cube);
          t = BiddyManagedAndAbstract(MNG,f1,g1,cube);
        } else {
          e = BiddyManagedAndAbstract(MNG,f0,g0,BiddyT(cube));
          t = BiddyManagedAndAbstract(MNG,f1,g1,BiddyT(cube));
        }
        if (cv == mintag) {
          if (minv == mintag) {
            r = BiddyManagedOr(MNG,e,t);
          } else {
            r = BiddyManagedTaggedFoaNode(MNG,minv,e,t,mintag,TRUE);
            BiddyRefresh(r); / * FoaNode returns an obsolete node! * /
            if (r != biddyZero) r = Biddy_Managed_IncTag(MNG,r);
          }
        } else if (BiddyIsSmaller(cv,minv)) {
          r = BiddyManagedTaggedFoaNode(MNG,minv,e,t,cv,TRUE);
          r = Biddy_Managed_IncTag(MNG,r);
          r = BiddyManagedTaggedFoaNode(MNG,cv,r,r,mintag,TRUE);
          BiddyRefresh(r); / * FoaNode returns an obsolete node! * /
        } else if (cv == minv) {
          r = BiddyManagedOr(MNG,e,t);
          r = BiddyManagedTaggedFoaNode(MNG,cv,r,r,mintag,TRUE);
          BiddyRefresh(r); / * FoaNode returns an obsolete node! * /
        } else {
          r = BiddyManagedTaggedFoaNode(MNG,minv,e,t,mintag,TRUE);
          BiddyRefresh(r); / * FoaNode returns an obsolete node! * /
        }
        */

        /* VARIANT 2 */
        /**/
        fv = BiddyV(f);
        gv = BiddyV(g);
        minv = BiddyIsSmaller(fv,gv) ? fv : gv;
        if (BiddyIsSmaller(minv,ftag)) {
          f0 = f1 = f;
        } else if (minv == fv) {
          f0 = BiddyE(f);
          f1 = BiddyT(f);
        } else {
          f0 = f;
          Biddy_SetTag(f0,minv);
          f0 = Biddy_Managed_IncTag(MNG,f0);
          f1 = biddyZero;
        }
        if (BiddyIsSmaller(minv,gtag)) {
          g0 = g1 = g;
        } else if (minv == gv) {
          g0 = BiddyE(g);
          g1 = BiddyT(g);
        } else {
          g0 = g;
          Biddy_SetTag(g0,minv);
          g0 = Biddy_Managed_IncTag(MNG,g0);
          g1 = biddyZero;
        }
        cgt = BiddyIsSmaller(minv,cv);
        if (f0 == biddyZero) {
          e = biddyZero;
        } else if (g0 == biddyZero) {
          e = biddyZero;
        } else {
          e = BiddyManagedAndAbstract(MNG,f0,g0,cgt?cube:BiddyT(cube));
        }
        if (f1 == biddyZero) {
          t = biddyZero;
        } else if (g1 == biddyZero) {
          t = biddyZero;
        } else {
          t = BiddyManagedAndAbstract(MNG,f1,g1,cgt?cube:BiddyT(cube));
        }

        /* VARIANT 2A */
        /*
        if (cv == mintag) {
          if (minv == mintag) {
            r = BiddyManagedOr(MNG,e,t);
          } else {
            r = BiddyManagedTaggedFoaNode(MNG,minv,e,t,mintag,TRUE);
            BiddyRefresh(r); / * FoaNode returns an obsolete node! * /
            if (r != biddyZero) r = Biddy_Managed_IncTag(MNG,r);
          }
        } else if (BiddyIsSmaller(cv,minv)) {
          r = BiddyManagedTaggedFoaNode(MNG,minv,e,t,cv,TRUE);
          BiddyRefresh(r); / * FoaNode returns an obsolete node! * /
          if (r != biddyZero) r = Biddy_Managed_IncTag(MNG,r);
          r = BiddyManagedTaggedFoaNode(MNG,cv,r,r,mintag,TRUE);
          BiddyRefresh(r); / * FoaNode returns an obsolete node! * /
        } else if (cv == minv) {
          r = BiddyManagedOr(MNG,e,t);
          r = BiddyManagedTaggedFoaNode(MNG,cv,r,r,mintag,TRUE);
          BiddyRefresh(r); / * FoaNode returns an obsolete node! * /
        } else {
          r = BiddyManagedTaggedFoaNode(MNG,minv,e,t,mintag,TRUE);
          BiddyRefresh(r); / * FoaNode returns an obsolete node! * /
        }
        */

        /* VARIANT 2B */
        if (cv == minv) {
          r = BiddyManagedOr(MNG,e,t);
          if (cv != mintag) {
            r = BiddyManagedTaggedFoaNode(MNG,cv,r,r,mintag,TRUE);
            BiddyRefresh(r); /* FoaNode returns an obsolete node! */
          }
        } else if (BiddyIsSmaller(cv,minv)) {
          r = BiddyManagedTaggedFoaNode(MNG,minv,e,t,cv,TRUE);
          BiddyRefresh(r); /* FoaNode returns an obsolete node! */
          if (r != biddyZero) r = Biddy_Managed_IncTag(MNG,r);
          if (cv != mintag) {
            r = BiddyManagedTaggedFoaNode(MNG,cv,r,r,mintag,TRUE);
            BiddyRefresh(r); /* FoaNode returns an obsolete node! */
          }
        } else {
          r = BiddyManagedTaggedFoaNode(MNG,minv,e,t,mintag,TRUE);
          BiddyRefresh(r); /* FoaNode returns an obsolete node! */
        }
        /**/

      }
    }

    /* CACHE EVERYTHING */
    addOp3Cache(MNG,biddyEACache,f,g,cube,r,cindex);

  } else {

    /* IF THE RESULT IS FROM CACHE TABLE, REFRESH IT! */
    BiddyRefresh(r);

  }

  return r;
}

/***************************************************************************//*!
\brief Function Biddy_Managed_Constrain calculates Coudert and Madre's constrain
       function.

### Description
    Coudert and Madre's constrain function is also called a generalized cofactor
    of function f with respect to function c.
### Side effects
    Original BDD is not changed.
    Implemented for OBDD and OBDDC.
    Cache table is not used.
### More info
    Macro Biddy_Constrain(f,c) is defined for use with anonymous manager.
*******************************************************************************/

/* SOME REFERENCES FOR Biddy_Constrain AND Biddy_Simplify */
/* O. Coudert et al. Verification of synchronous sequential machines based on symbolic execution, 1989 */
/* O. Coudert et al. A unified framework for the formal verification of sequential circuits, 1990 */
/* B. Lin et al.  Don't care minimization of multi-level sequential logic networks. 1990. */
/* Thomas R. Shiple et al. Heuristic minimization of BDDs using don't cares, 1994 */
/* Mark D. Aagaard. et al. Formal verification using parametric representations of Boolean constraints, 1999 */

#ifdef __cplusplus
extern "C" {
#endif

Biddy_Edge
Biddy_Managed_Constrain(Biddy_Manager MNG, Biddy_Edge f, Biddy_Edge c)
{
  Biddy_Edge r;

  assert( f != NULL );
  assert( c != NULL );

  if (!MNG) MNG = biddyAnonymousManager;
  ZF_LOGI("Biddy_Constrain");

  assert( BiddyIsOK(f) == TRUE );
  assert( BiddyIsOK(c) == TRUE );

  r = biddyNull;

  if (biddyManagerType == BIDDYTYPEOBDD) {
    /* IMPLEMENTED */
    r = BiddyManagedConstrain(MNG,f,c);
    BiddyRefresh(r); /* not always refreshed by BiddyManagedConstrain */
  } else if (biddyManagerType == BIDDYTYPEOBDDC) {
    /* IMPLEMENTED */
    r = BiddyManagedConstrain(MNG,f,c);
    BiddyRefresh(r); /* not always refreshed by BiddyManagedConstrain */
  } else if ((biddyManagerType == BIDDYTYPEZBDDC) ||
              (biddyManagerType == BIDDYTYPETZBDD))
  {
    fprintf(stderr,"Biddy_Constrain: this GDD type is not supported, yet!\n");
    return f;
  } else if ((biddyManagerType == BIDDYTYPEZBDD) ||
              (biddyManagerType == BIDDYTYPETZBDDC))
  {
    fprintf(stderr,"Biddy_Constrain: this GDD type is not supported, yet!\n");
    return biddyNull;
  } else if ((biddyManagerType == BIDDYTYPEOFDDC) || (biddyManagerType == BIDDYTYPEOFDD) ||
              (biddyManagerType == BIDDYTYPEZFDDC) || (biddyManagerType == BIDDYTYPEZFDD) ||
              (biddyManagerType == BIDDYTYPETZFDDC) || (biddyManagerType == BIDDYTYPETZFDD))
  {
    fprintf(stderr,"Biddy_Constrain: this GDD type is not supported, yet!\n");
    return biddyNull;
  } else {
    fprintf(stderr,"Biddy_Constrain: Unsupported GDD type!\n");
    return biddyNull;
  }

  return r;
}

#ifdef __cplusplus
}
#endif

Biddy_Edge
BiddyManagedConstrain(Biddy_Manager MNG, Biddy_Edge f, Biddy_Edge c)
{
  Biddy_Edge f0,f1,c0,c1;
  Biddy_Edge e, t, r;
  Biddy_Variable fv,cv,minv;

  assert( MNG != NULL );
  assert( f != NULL );
  assert( c != NULL );

  /* IMPLEMENTED FOR OBDD AND OBDDC */
  assert(
    (biddyManagerType == BIDDYTYPEOBDD) ||
    (biddyManagerType == BIDDYTYPEOBDDC)
  );

  r = biddyNull;

  /* LOOKING FOR SIMPLE CASE */
  if (c == biddyZero) {
    return biddyZero;
  } else if (c == biddyOne) {
    return f;
  } else if (Biddy_IsTerminal(f)) {
    return f;
  }

  fv = BiddyV(f);
  cv = BiddyV(c);

  if (BiddyIsSmaller(fv,cv)) {
    f0 = Biddy_InvCond(BiddyE(f),Biddy_GetMark(f));
    f1 = Biddy_InvCond(BiddyT(f),Biddy_GetMark(f));
    c0 = c;
    c1 = c;
    minv = fv;
  } else if (BiddyIsSmaller(cv,fv)) {
    f0 = f;
    f1 = f;
    c0 = Biddy_InvCond(BiddyE(c),Biddy_GetMark(c));
    c1 = Biddy_InvCond(BiddyT(c),Biddy_GetMark(c));
    minv = cv;
  } else {
    f0 = Biddy_InvCond(BiddyE(f),Biddy_GetMark(f));
    f1 = Biddy_InvCond(BiddyT(f),Biddy_GetMark(f));
    c0 = Biddy_InvCond(BiddyE(c),Biddy_GetMark(c));
    c1 = Biddy_InvCond(BiddyT(c),Biddy_GetMark(c));
    minv = cv;
  }

  if (c0 == biddyZero) return BiddyManagedConstrain(MNG,f1,c1);
  if (c1 == biddyZero) return BiddyManagedConstrain(MNG,f0,c0);

  e = BiddyManagedConstrain(MNG,f0,c0);
  t = BiddyManagedConstrain(MNG,f1,c1);
  r = BiddyManagedITE(MNG,biddyVariableTable.table[minv].variable,t,e);

  return r;
}

/***************************************************************************//*!
\brief Function Biddy_Managed_Simplify calculates Coudert and Madre's restrict
       function.

### Description
    Coudert and Madre's restrict function tries to simplify function f by
    restricting it to the domain covered by function c. No checks are done
    to see if the result is actually smaller than the input.
### Side effects
    Original BDD is not changed.
    Implemented for OBDD and OBDDC.
    Cache table is not used.
### More info
    Macro Biddy_Simplify(f,c) is defined for use with anonymous manager.
*******************************************************************************/

#ifdef __cplusplus
extern "C" {
#endif

Biddy_Edge
Biddy_Managed_Simplify(Biddy_Manager MNG, Biddy_Edge f, Biddy_Edge c)
{
  Biddy_Edge r;

  assert( f != NULL );
  assert( c != NULL );

  if (!MNG) MNG = biddyAnonymousManager;
  ZF_LOGI("Biddy_Simplify");

  assert( BiddyIsOK(f) == TRUE );
  assert( BiddyIsOK(c) == TRUE );

  r = biddyNull;

  if (biddyManagerType == BIDDYTYPEOBDD) {
    /* IMPLEMENTED */
    r = BiddyManagedSimplify(MNG,f,c);
    BiddyRefresh(r); /* not always refreshed by BiddyManagedSimplify */
  } else if (biddyManagerType == BIDDYTYPEOBDDC) {
    /* IMPLEMENTED */
    r = BiddyManagedSimplify(MNG,f,c);
    BiddyRefresh(r); /* not always refreshed by BiddyManagedSimplify */
  } else if ((biddyManagerType == BIDDYTYPEZBDDC) ||
              (biddyManagerType == BIDDYTYPETZBDD))
  {
    fprintf(stderr,"Biddy_Simplify: this GDD type is not supported, yet!\n");
    return f;
  } else if ((biddyManagerType == BIDDYTYPEZBDD) ||
              (biddyManagerType == BIDDYTYPETZBDDC))
  {
    fprintf(stderr,"Biddy_Simplify: this GDD type is not supported, yet!\n");
    return biddyNull;
  } else if ((biddyManagerType == BIDDYTYPEOFDDC) || (biddyManagerType == BIDDYTYPEOFDD) ||
              (biddyManagerType == BIDDYTYPEZFDDC) || (biddyManagerType == BIDDYTYPEZFDD) ||
              (biddyManagerType == BIDDYTYPETZFDDC) || (biddyManagerType == BIDDYTYPETZFDD))
  {
    fprintf(stderr,"Biddy_Simplify: this GDD type is not supported, yet!\n");
    return biddyNull;
  } else {
    fprintf(stderr,"Biddy_Simplify: Unsupported GDD type!\n");
    return biddyNull;
  }

  return r;
}

#ifdef __cplusplus
}
#endif

Biddy_Edge
BiddyManagedSimplify(Biddy_Manager MNG, Biddy_Edge f, Biddy_Edge c)
{
  Biddy_Edge f0,f1,c0,c1;
  Biddy_Edge e, t, r;
  Biddy_Variable fv,cv,minv;

  assert( MNG != NULL );
  assert( f != NULL );
  assert( c != NULL );

  /* IMPLEMENTED FOR OBDD AND OBDDC */
  assert(
    (biddyManagerType == BIDDYTYPEOBDD) ||
    (biddyManagerType == BIDDYTYPEOBDDC)
  );

  r = biddyNull;

  /* LOOKING FOR SIMPLE CASE */
  if (c == biddyZero) return biddyNull;
  if (c == biddyOne) {
    return f;
  } else if (Biddy_IsTerminal(f)) {
    return f;
  }

  /* THIS IS FROM 20-CS-626-001 */
  /* http://gauss.ececs.uc.edu/Courses/c626/lectures/BDD/bdd-desc.pdf */
  if (f == c) return biddyOne;
  if (biddyManagerType == BIDDYTYPEOBDDC) {
    if (Biddy_IsEqvPointer(f,c)) {
      return biddyZero;
    }
  }

  fv = BiddyV(f);
  cv = BiddyV(c);

  /* SPECIAL CASE: f/~a == f/a */
  if (BiddyIsSmaller(cv,fv)) {
    return BiddyManagedSimplify(MNG,f,BiddyManagedE(MNG,c,cv));
  }

  if (BiddyIsSmaller(fv,cv)) {
    f0 = Biddy_InvCond(BiddyE(f),Biddy_GetMark(f));
    f1 = Biddy_InvCond(BiddyT(f),Biddy_GetMark(f));
    c0 = c;
    c1 = c;
    minv = fv;
  } else {
    f0 = Biddy_InvCond(BiddyE(f),Biddy_GetMark(f));
    f1 = Biddy_InvCond(BiddyT(f),Biddy_GetMark(f));
    c0 = Biddy_InvCond(BiddyE(c),Biddy_GetMark(c));
    c1 = Biddy_InvCond(BiddyT(c),Biddy_GetMark(c));
    minv = cv;
  }

  if (c0 == biddyZero) return BiddyManagedSimplify(MNG,f1,c1);
  if (c1 == biddyZero) return BiddyManagedSimplify(MNG,f0,c0);

  e = BiddyManagedSimplify(MNG,f0,c0);
  t = BiddyManagedSimplify(MNG,f1,c1);
  r = BiddyManagedITE(MNG,biddyVariableTable.table[minv].variable,t,e);

  return r;
}

/***************************************************************************//*!
\brief Function Biddy_Managed_Support calculates a product of all dependent
       variables (OBDD and TZBDD) or the combination set containing a subset
       which includes all dependent variables (ZBDD).

### Description
    Implemented for OBDD, OBDDC, ZBDDC, and TZBDD.
    For OBDDs, dependent variables are all variables existing in the graph.
    For ZBDDs and TZBDDs, this is not true.
### Side effects
### More info
    Macro Biddy_Support(f) is defined for use with anonymous manager.
*******************************************************************************/

#ifdef __cplusplus
extern "C" {
#endif

Biddy_Edge
Biddy_Managed_Support(Biddy_Manager MNG, Biddy_Edge f)
{
  Biddy_Edge r;

  if (!MNG) MNG = biddyAnonymousManager;
  ZF_LOGI("Biddy_Support");

  assert( (f == NULL) || (BiddyIsOK(f) == TRUE) );

  r = biddyNull;

  if (biddyManagerType == BIDDYTYPEOBDD) {
    /* IMPLEMENTED */
    r = BiddyManagedSupport(MNG,f);
    BiddyRefresh(r); /* not always refreshed by BiddyManagedSupport */
  } else if (biddyManagerType == BIDDYTYPEOBDDC) {
    /* IMPLEMENTED */
    r = BiddyManagedSupport(MNG,f);
    BiddyRefresh(r); /* not always refreshed by BiddyManagedSupport */
  } else if (biddyManagerType == BIDDYTYPEZBDDC) {
    /* PROTOTYPED USING Restrict */
    /*
    Biddy_Variable v;
    r = biddyTerminal;
    for (v=1;v<biddyVariableTable.num;v++) {
      if (BiddyManagedRestrict(MNG,f,v,FALSE) != BiddyManagedRestrict(MNG,f,v,TRUE)) {
        r = BiddyManagedChange(MNG,r,v);
      }
    }
    */
    /* IMPLEMENTED */
    r = BiddyManagedSupport(MNG,f);
    BiddyRefresh(r); /* not always refreshed by BiddyManagedSupport */
  } else if (biddyManagerType == BIDDYTYPETZBDD) {
    /* PROTOTYPED USING Restrict */
    /*
    Biddy_Variable v;
    r = biddyOne;
    for (v=1;v<biddyVariableTable.num;v++) {
      if (BiddyManagedRestrict(MNG,f,v,FALSE) != BiddyManagedRestrict(MNG,f,v,TRUE)) {
        r = BiddyManagedAnd(MNG,r,biddyVariableTable.table[v].variable);
      }
    }
    */
    /* IMPLEMENTED */
    r = BiddyManagedSupport(MNG,f);
    BiddyRefresh(r); /* not always refreshed by BiddyManagedSupport */
  } else if ((biddyManagerType == BIDDYTYPEZBDD) ||
              (biddyManagerType == BIDDYTYPETZBDDC))
  {
    fprintf(stderr,"Biddy_Support: this GDD type is not supported, yet!\n");
    return biddyNull;
  } else if ((biddyManagerType == BIDDYTYPEOFDDC) || (biddyManagerType == BIDDYTYPEOFDD) ||
              (biddyManagerType == BIDDYTYPEZFDDC) || (biddyManagerType == BIDDYTYPEZFDD) ||
              (biddyManagerType == BIDDYTYPETZFDDC) || (biddyManagerType == BIDDYTYPETZFDD))
  {
    fprintf(stderr,"Biddy_Support: this GDD type is not supported, yet!\n");
    return biddyNull;
  } else {
    fprintf(stderr,"Biddy_Support: Unsupported GDD type!\n");
    return biddyNull;
  }

  return r;
}

#ifdef __cplusplus
}
#endif

Biddy_Edge
BiddyManagedSupport(Biddy_Manager MNG, Biddy_Edge f)
{
  Biddy_Variable v;
  unsigned int n;
  Biddy_Edge s;

  assert( MNG != NULL );

  /* IMPLEMENTED FOR OBDD, OBDDC, ZBDDC, and TZBDD */
  assert(
    (biddyManagerType == BIDDYTYPEOBDD) ||
    (biddyManagerType == BIDDYTYPEOBDDC) ||
    (biddyManagerType == BIDDYTYPEZBDDC) ||
    (biddyManagerType == BIDDYTYPETZBDD)
  );

  if (Biddy_IsNull(f)) return biddyNull;
  if (Biddy_IsTerminal(f)) return biddyZero;

  /* MAKE VARIABLES UNSELECTED */
  /*
  for (v=1;v<biddyVariableTable.num;v++) {
    biddyVariableTable.table[v].selected = FALSE;
  }
  */

  if ((biddyManagerType == BIDDYTYPEOBDDC) || (biddyManagerType == BIDDYTYPEOBDD)) {
    n = 1; /* NUMBER OF NODES NOT NEEDED, BUT NODES ARE COUNTED BY BiddyNodeVarNumber */
    Biddy_Managed_SelectNode(MNG,biddyTerminal); /* for OBDDs, this is needed for BiddyNodeVarNumber */
    BiddyNodeVarNumber(MNG,f,&n); /* DEPENDENT VARIABLES ARE MARKED, all nodes are selected */
    Biddy_Managed_DeselectAll(MNG);
  }
  else if (biddyManagerType == BIDDYTYPEZBDDC) {
    BiddyCreateLocalInfo(MNG,f); /* FOR ZBDDs, localinfo IS USED BY BiddyNodeVarNumber */
    n = 1; /* NUMBER OF NODES NOT NEEDED, BUT NODES ARE COUNTED BY BiddyNodeVarNumber */
    BiddyNodeVarNumber(MNG,f,&n); /* DEPENDENT VARIABLES ARE MARKED, selects all except terminal node */
    BiddyDeleteLocalInfo(MNG,f); /* FOR ZBDDs, localinfo IS USED BY BiddyNodeVarNumber */
  }
  else if (biddyManagerType == BIDDYTYPETZBDD) {
    n = 1; /* NUMBER OF NODES NOT NEEDED, BUT NODES ARE COUNTED BY BiddyNodeVarNumber */
    Biddy_Managed_SelectNode(MNG,biddyTerminal); /* for TZBDDs, this is needed for BiddyNodeVarNumber */
    BiddyNodeVarNumber(MNG,f,&n); /* DEPENDENT VARIABLES ARE MARKED, all nodes are selected */
    Biddy_Managed_DeselectAll(MNG);
  }

  s = biddyNull;
  if ((biddyManagerType == BIDDYTYPEOBDDC) || (biddyManagerType == BIDDYTYPEOBDD)) {
    s = biddyOne;
    for (v=1;v<biddyVariableTable.num;v++) {
      if (biddyVariableTable.table[v].selected == TRUE) {
        s = BiddyManagedAnd(MNG,s,biddyVariableTable.table[v].variable);
        biddyVariableTable.table[v].selected = FALSE;
      }
    }
  }
  else if (biddyManagerType == BIDDYTYPEZBDDC) {
    /* NOTE: for ZBDDs, variables above the top variable (which are always all dependent) */
    /* are not selected by BiddyNodeVarNumber! */
    s = biddyTerminal; /* this is base set */
    for (v=1;v<biddyVariableTable.num;v++) {
      if ((biddyVariableTable.table[v].selected == TRUE) || BiddyIsSmaller(v,BiddyV(f)))
      {
        s = BiddyManagedChange(MNG,s,v);
        biddyVariableTable.table[v].selected = FALSE;
      }
    }
  }
  else if (biddyManagerType == BIDDYTYPETZBDD) {
    s = biddyOne;
    for (v=1;v<biddyVariableTable.num;v++) {
      if (biddyVariableTable.table[v].selected == TRUE) {
        s = BiddyManagedAnd(MNG,s,biddyVariableTable.table[v].variable);
        biddyVariableTable.table[v].selected = FALSE;
      }
    }
  }

  return s;
}

/***************************************************************************//*!
\brief Function Biddy_Managed_ReplaceByKeyword calculates Boolean function
        with one or more variables replaced.

### Description
    Original BDD is not changed.
    Implemented for OBDD and OBDDC.
    Prototyped for ZBDDC and TZBDD (via And-Xor-Not-Restrict).
    The sets of current and new variables should be disjoint.
    Replacing is controlled by variable's values (which are edges!).
    Use Biddy_Managed_ResetVariablesValue and Biddy_Managed_SetVariableValue
    to prepare replacing.
    Parameter keyword is used to maintain cache table. If (keyword == NULL)
    then entries in the cache table from previous calculations are deleted.
### Side effects
### More info
    Macro Biddy_ReplaceByKeyword(f,keyword) is defined for use with anonymous
    manager. Macros Biddy_Managed_Replace(MNG,f) and Biddy_Replace(f) are
    variants with less effective cache table.
*******************************************************************************/

#ifdef __cplusplus
extern "C" {
#endif

Biddy_Edge
Biddy_Managed_ReplaceByKeyword(Biddy_Manager MNG, Biddy_Edge f,
                               Biddy_String keyword)
{
  Biddy_Edge e,t,tmp,r;
  Biddy_Variable v;
  unsigned int key;
  Biddy_Boolean OK;
  int cc;
  const Biddy_Boolean disableCache = FALSE;

  assert( f != NULL );

  if (!MNG) MNG = biddyAnonymousManager;
  ZF_LOGI("Biddy_Replace");

  assert( BiddyIsOK(f) == TRUE );

  r = biddyNull;

  /* find keyword in keywordList */

  /* VARIANT A: LINEAR SEARCH */
  if (!disableCache && keyword) {
    OK = FALSE;
    if (!biddyReplaceCache.keywordList) {
      key = 0;
    } else {
      for (key = 0; !OK && (key < biddyReplaceCache.keywordNum); key++) {
        cc = strcmp(keyword,biddyReplaceCache.keywordList[key]);
        if (cc == 0) {
          OK = TRUE;
          break;
        }
        if (cc < 0) break;
      }
    }
  }

  /* TO DO: BINARY SEARCH */
  /* NOT IMPORTANT BECAUSE NUMBER OF KEYWORDS IS VERY SMALL IN THE PRACTICE */

  /* keyword not found - it will be added */
  if (disableCache || !keyword) {
    key = 0;
  } else {
    if (!OK) {
      Biddy_String *tmp1;
      unsigned int *tmp2;

      biddyReplaceCache.keywordNum++;
      if (!(tmp1 = (Biddy_String *)
         realloc(biddyReplaceCache.keywordList,biddyReplaceCache.keywordNum*sizeof(Biddy_String))))
      {
        fprintf(stderr,"Biddy_Managed_ReplaceByKeyword: Out of memoy!\n");
        exit(1);
      }
      biddyReplaceCache.keywordList = tmp1;
      if (!(tmp2 = (unsigned int *)
         realloc(biddyReplaceCache.keyList,biddyReplaceCache.keywordNum*sizeof(unsigned int))))
      {
        fprintf(stderr,"Biddy_Managed_ReplaceByKeyword: Out of memoy!\n");
        exit(1);
      }
      biddyReplaceCache.keyList = tmp2;

      memmove(&biddyReplaceCache.keywordList[key+1],&biddyReplaceCache.keywordList[key],(biddyReplaceCache.keywordNum-key-1)*sizeof(Biddy_String));
      memmove(&biddyReplaceCache.keyList[key+1],&biddyReplaceCache.keyList[key],(biddyReplaceCache.keywordNum-key-1)*sizeof(unsigned int));
      /*
      for (i = biddyReplaceCache.keywordNum-1; i > key; i--) {
        biddyReplaceCache.keywordList[i] = biddyReplaceCache.keywordList[i-1];
        biddyReplaceCache.keyList[i] = biddyReplaceCache.keyList[i-1];
      }
      */

      biddyReplaceCache.keywordList[key] = strdup(keyword);
      biddyReplaceCache.keyList[key] = biddyReplaceCache.keywordNum;
    }
    key = biddyReplaceCache.keyList[key];
  }

  /* DEBUGGING */
  /*
  fprintf(stderr,"ReplaceByKeyword key = %u (%s)\n",key,keyword);
  */

  if (biddyManagerType == BIDDYTYPEOBDD) {
    /* IMPLEMENTED */
    if (!disableCache && !key) BiddyReplaceGarbageDeleteAll(MNG);
    r = BiddyManagedReplaceByKeyword(MNG,f,key);
    BiddyRefresh(r); /* not always refreshed by BiddyManagedReplaceByKeyword */
    Biddy_Managed_DeselectAll(MNG);
  } else if (biddyManagerType == BIDDYTYPEOBDDC) {
    /* IMPLEMENTED */
    if (!disableCache && !key) BiddyReplaceGarbageDeleteAll(MNG);
    r = BiddyManagedReplaceByKeyword(MNG,f,key);
    BiddyRefresh(r); /* not always refreshed by BiddyManagedReplaceByKeyword */
    Biddy_Managed_DeselectAll(MNG);
  } else if (biddyManagerType == BIDDYTYPEZBDDC) {
    /* PROTOTYPED */
    r = f;
    for (v=1;v<biddyVariableTable.num;v++) {
      if ((tmp=biddyVariableTable.table[v].value) != biddyZero) {
        e = BiddyManagedRestrict(MNG,r,v,FALSE);
        t = BiddyManagedRestrict(MNG,r,v,TRUE);
        r = BiddyManagedXor(MNG,
              BiddyManagedAnd(MNG,tmp,t),
              BiddyManagedAnd(MNG,BiddyManagedNot(MNG,tmp),e));
      }
    }
    BiddyRefresh(r); /* not always refreshed by BiddyManagedXor */
  } else if (biddyManagerType == BIDDYTYPETZBDD) {
    /* PROTOTYPED */
    r = f;
    for (v=1;v<biddyVariableTable.num;v++) {
      if ((tmp=biddyVariableTable.table[v].value) != biddyZero) {
        e = BiddyManagedRestrict(MNG,r,v,FALSE);
        t = BiddyManagedRestrict(MNG,r,v,TRUE);
        r = BiddyManagedXor(MNG,
              BiddyManagedAnd(MNG,tmp,t),
              BiddyManagedAnd(MNG,BiddyManagedNot(MNG,tmp),e));
      }
    }
    BiddyRefresh(r); /* not always refreshed by BiddyManagedXor */
  } else if ((biddyManagerType == BIDDYTYPEOBDD) ||
              (biddyManagerType == BIDDYTYPEZBDD) ||
              (biddyManagerType == BIDDYTYPETZBDDC))
  {
    fprintf(stderr,"Biddy_ReplaceByKeyword: this GDD type is not supported, yet!\n");
    return biddyNull;
  } else if ((biddyManagerType == BIDDYTYPEOFDDC) || (biddyManagerType == BIDDYTYPEOFDD) ||
              (biddyManagerType == BIDDYTYPEZFDDC) || (biddyManagerType == BIDDYTYPEZFDD) ||
              (biddyManagerType == BIDDYTYPETZFDDC) || (biddyManagerType == BIDDYTYPETZFDD))
  {
    fprintf(stderr,"Biddy_ReplaceByKeyword: this GDD type is not supported, yet!\n");
    return biddyNull;
  } else {
    fprintf(stderr,"Biddy_ReplaceByKeyword: Unsupported GDD type!\n");
    return biddyNull;
  }

  return r;
}

#ifdef __cplusplus
}
#endif

Biddy_Edge
BiddyManagedReplaceByKeyword(Biddy_Manager MNG, Biddy_Edge f, const unsigned int key)
{
  Biddy_Edge e,t,r;
  Biddy_Variable fv;
  Biddy_Edge FF;
  Biddy_Boolean NN;
  unsigned int cindex;

  assert( MNG != NULL );
  assert( f != NULL );

  /* IMPLEMENTED FOR OBDD AND OBDDC */
  assert(
    (biddyManagerType == BIDDYTYPEOBDD) ||
    (biddyManagerType == BIDDYTYPEOBDDC)
  );

  if (Biddy_IsTerminal(f)) return f;

  if (key && Biddy_GetMark(f)) {
    NN = TRUE;
    FF = Biddy_Inv(f);
  } else {
    NN = FALSE;
    FF = f;
  }

  /* IF RESULT IS NOT IN THE CACHE TABLE... */
  cindex = 0;
  if (!key || !findKeywordCache(MNG,biddyReplaceCache,FF,key,&r,&cindex))
  {
    fv = BiddyV(f);
    if ((r=biddyVariableTable.table[fv].value) != biddyZero) {

      assert( r != NULL );

      fv = BiddyV(r);

      assert( fv != 0 );

      /* DEBUGGING */
      /*
      fprintf(stdout,"REPLACE: %s(%u) -> %s(%u)\n",
              Biddy_Managed_GetTopVariableName(MNG,f),
              BiddyV(f),
              Biddy_Managed_GetVariableName(MNG,fv),
              fv
      );
      */
    }

    e = BiddyManagedReplaceByKeyword(MNG,Biddy_InvCond(BiddyE(f),Biddy_GetMark(f)),key);
    t = BiddyManagedReplaceByKeyword(MNG,Biddy_InvCond(BiddyT(f),Biddy_GetMark(f)),key);

    /* VARIANT A */
    /* THIS IS NOT SLOW BECAUSE CACHE TABLE FOR ITE RESOLVES A LOT OF CALLS */
    /**/
    r = BiddyManagedITE(MNG,biddyVariableTable.table[fv].variable,t,e);
    /**/

    /* VARIANT B */
    /* THIS IS NOT AS FAST AS WE WANT IT */
    /*
    if (BiddyIsSmaller(fv,BiddyV(e)) && BiddyIsSmaller(fv,BiddyV(t))) {
      r = BiddyManagedTaggedFoaNode(MNG,fv,e,t,fv,TRUE);
      BiddyRefresh(r);
    } else {
      r = BiddyManagedITE(MNG,biddyVariableTable.table[fv].variable,t,e);
    }
    */

    if (key) {
      if (NN) {
        addKeywordCache(MNG,biddyReplaceCache,FF,key,Biddy_Inv(r),cindex);
      } else {
        addKeywordCache(MNG,biddyReplaceCache,FF,key,r,cindex);
      }
    }

  } else {

    if (NN) {
      Biddy_InvertMark(r);
    }

    /* IF THE RESULT IS FROM CACHE TABLE, REFRESH IT! */
    BiddyRefresh(r);

  }

  return r;
}

/***************************************************************************//*!
\brief Function Biddy_Managed_Change change the form of the given variable
       (positive literal becomes negative and vice versa).

### Description
### Side effects
    Original BDD is not changed.
    Implemented for OBDD, OBDDC, ZBDDC, and TZBDD.
    RC Cache is used with parameters (f,biddyNull,v).
### More info
    Macro Biddy_Change() is defined for use with anonymous manager.
*******************************************************************************/

#ifdef __cplusplus
extern "C" {
#endif

Biddy_Edge
Biddy_Managed_Change(Biddy_Manager MNG, Biddy_Edge f, Biddy_Variable v)
{
  Biddy_Edge r;

  assert( f != NULL );

  if (!MNG) MNG = biddyAnonymousManager;
  ZF_LOGI("Biddy_Change");

  assert( BiddyIsOK(f) == TRUE );

  r = biddyNull;

  if (biddyManagerType == BIDDYTYPEOBDD) {
    /* IMPLEMENTED */
    r = BiddyManagedChange(MNG,f,v);
    BiddyRefresh(r); /* not always refreshed by BiddyManagedChange */
  } else if (biddyManagerType == BIDDYTYPEOBDDC) {
    /* IMPLEMENTED */
    r = BiddyManagedChange(MNG,f,v);
    BiddyRefresh(r); /* not always refreshed by BiddyManagedChange */
  } else if (biddyManagerType == BIDDYTYPEZBDDC) {
    /* IMPLEMENTED */
    r = BiddyManagedChange(MNG,f,v);
    BiddyRefresh(r); /* not always refreshed by BiddyManagedChange */
  } else if (biddyManagerType == BIDDYTYPETZBDD) {
    /* IMPLEMENTED */
    r = BiddyManagedChange(MNG,f,v);
    BiddyRefresh(r); /* not always refreshed by BiddyManagedChange */
  } else if ((biddyManagerType == BIDDYTYPEZBDD) ||
              (biddyManagerType == BIDDYTYPETZBDDC))
  {
    fprintf(stderr,"Biddy_Change: this GDD type is not supported, yet!\n");
    return biddyNull;
  } else if ((biddyManagerType == BIDDYTYPEOFDDC) || (biddyManagerType == BIDDYTYPEOFDD) ||
              (biddyManagerType == BIDDYTYPEZFDDC) || (biddyManagerType == BIDDYTYPEZFDD) ||
              (biddyManagerType == BIDDYTYPETZFDDC) || (biddyManagerType == BIDDYTYPETZFDD))
  {
    fprintf(stderr,"Biddy_Change: this GDD type is not supported, yet!\n");
    return biddyNull;
  } else {
    fprintf(stderr,"Biddy_Change: Unsupported GDD type!\n");
    return biddyNull;
  }

  return r;
}

#ifdef __cplusplus
}
#endif

Biddy_Edge
BiddyManagedChange(Biddy_Manager MNG, Biddy_Edge f, Biddy_Variable v)
{
  Biddy_Edge e,t,r;
  Biddy_Variable fv,tag;
  Biddy_Edge FF, GG, HH;
  unsigned int cindex;

  assert( MNG != NULL );
  assert( f != NULL );

  /* IMPLEMENTED FOR OBDD, OBDDC, ZBDDC, AND TZBDD */
  assert(
    (biddyManagerType == BIDDYTYPEOBDD) ||
    (biddyManagerType == BIDDYTYPEOBDDC) ||
    (biddyManagerType == BIDDYTYPEZBDDC) ||
    (biddyManagerType == BIDDYTYPETZBDD)
  );

  if (f == biddyZero) return biddyZero;

  r = biddyNull;

  FF = f;
  GG = biddyNull;
  HH = biddyVariableTable.table[v].variable;

  /* IF RESULT IS NOT IN THE CACHE TABLE... */
  /* TO DO: CHECK ONLY IF IT IS POSSIBLE TO EXIST IN THE CACHE */
  cindex = 0;
  if (!findOp3Cache(MNG,biddyRCCache,FF,GG,HH,&r,&cindex))
  {

    if ((biddyManagerType == BIDDYTYPEOBDDC) || (biddyManagerType == BIDDYTYPEOBDD)) {
      fv = BiddyV(f);
      if (v == fv) {
        e = Biddy_InvCond(BiddyT(f),Biddy_GetMark(f));
        t = Biddy_InvCond(BiddyE(f),Biddy_GetMark(f));
        r = BiddyManagedTaggedFoaNode(MNG,v,e,t,v,TRUE);
        BiddyRefresh(r); /* FoaNode returns an obsolete node! */
      } else if (BiddyIsSmaller(v,fv)) {
        r = f;
      } else {
        e = BiddyManagedChange(MNG,Biddy_InvCond(BiddyE(f),Biddy_GetMark(f)),v);
        t = BiddyManagedChange(MNG,Biddy_InvCond(BiddyT(f),Biddy_GetMark(f)),v);
        r = BiddyManagedTaggedFoaNode(MNG,fv,e,t,fv,TRUE);
        BiddyRefresh(r); /* FoaNode returns an obsolete node! */
        addOp3Cache(MNG,biddyRCCache,FF,GG,HH,r,cindex);
      }
    }

    else if (biddyManagerType == BIDDYTYPEZBDDC) {
      fv = BiddyV(f);
      if (v == fv) {
        e = BiddyT(f);
        t = Biddy_InvCond(BiddyE(f),Biddy_GetMark(f));
        r = BiddyManagedTaggedFoaNode(MNG,v,e,t,0,TRUE);
        BiddyRefresh(r); /* FoaNode returns an obsolete node! */
      } else if (BiddyIsSmaller(v,fv)) {
        r = BiddyManagedTaggedFoaNode(MNG,v,biddyZero,f,0,TRUE);
        BiddyRefresh(r); /* FoaNode returns an obsolete node! */
      } else {
        e = BiddyManagedChange(MNG,Biddy_InvCond(BiddyE(f),Biddy_GetMark(f)),v);
        t = BiddyManagedChange(MNG,BiddyT(f),v);
        r = BiddyManagedTaggedFoaNode(MNG,fv,e,t,0,TRUE);
        BiddyRefresh(r); /* FoaNode returns an obsolete node! */
        addOp3Cache(MNG,biddyRCCache,FF,GG,HH,r,cindex);
      }
    }

    else if (biddyManagerType == BIDDYTYPETZBDD) {
      tag = Biddy_GetTag(f);
      if (BiddyIsSmaller(v,tag)) {
        r = f;
      } else {
        fv = BiddyV(f);
        if (v == fv) {
          e = BiddyT(f);
          t = BiddyE(f);
          r = BiddyManagedTaggedFoaNode(MNG,v,e,t,tag,TRUE);
          BiddyRefresh(r); /* FoaNode returns an obsolete node! */
        } else if (BiddyIsSmaller(v,fv)) {
          r = f;
          Biddy_SetTag(r,v);
          r = Biddy_Managed_IncTag(MNG,r);
          r = BiddyManagedTaggedFoaNode(MNG,v,biddyZero,r,tag,TRUE);
          BiddyRefresh(r); /* FoaNode returns an obsolete node! */
        } else {
          e = BiddyManagedChange(MNG,BiddyE(f),v);
          t = BiddyManagedChange(MNG,BiddyT(f),v);
          r = BiddyManagedTaggedFoaNode(MNG,fv,e,t,tag,TRUE);
          BiddyRefresh(r); /* FoaNode returns an obsolete node! */
          addOp3Cache(MNG,biddyRCCache,FF,GG,HH,r,cindex);
        }
      }
    }

  } else {

    /* IF THE RESULT IS FROM CACHE TABLE, REFRESH IT! */
    BiddyRefresh(r);

  }

  return r;
}

/***************************************************************************//*!
\brief Function Biddy_Managed_Subset calculates a division of Boolean
       function with a literal.

### Description
    For combination sets, this function coincides with Subset0 and Subset1.
### Side effects
    Original BDD is not changed.
    Implemented for OBDD, OBDDC, ZBDDC, and TZBDD.
    Cache table for AND is used.
### More info
    Macro Biddy_Subset(f,v,value) is defined for use with anonymous manager.
    Macros Biddy_Managed_Subset0(MNG,f,v), Biddy_Subset0(f,v),
    Biddy_Managed_Subset1(MNG,f,v), and Biddy_Subset1(f,v) are defined for
    manipulation of combination sets.
*******************************************************************************/

#ifdef __cplusplus
extern "C" {
#endif

Biddy_Edge
Biddy_Managed_Subset(Biddy_Manager MNG, Biddy_Edge f, Biddy_Variable v,
                     Biddy_Boolean value)
{
  Biddy_Edge r;

  assert( f != NULL );

  if (!MNG) MNG = biddyAnonymousManager;
  ZF_LOGI("Biddy_Subset");

  assert( BiddyIsOK(f) == TRUE );

  r = biddyNull;

  if (biddyManagerType == BIDDYTYPEOBDD) {
    /* IMPLEMENTED */
    r = BiddyManagedSubset(MNG,f,v,value);
    BiddyRefresh(r); /* not always refreshed by BiddyManagedSubset */
  } else if (biddyManagerType == BIDDYTYPEOBDDC) {
    /* IMPLEMENTED */
    r = BiddyManagedSubset(MNG,f,v,value);
    BiddyRefresh(r); /* not always refreshed by BiddyManagedSubset */
  } else if (biddyManagerType == BIDDYTYPEZBDDC) {
    /* IMPLEMENTED */
    r = BiddyManagedSubset(MNG,f,v,value);
    BiddyRefresh(r); /* not always refreshed by BiddyManagedSubset */
  } else if (biddyManagerType == BIDDYTYPETZBDD) {
    /* IMPLEMENTED */
    r = BiddyManagedSubset(MNG,f,v,value);
    BiddyRefresh(r); /* not always refreshed by BiddyManagedSubset */
  } else if ((biddyManagerType == BIDDYTYPEZBDD) ||
              (biddyManagerType == BIDDYTYPETZBDDC))
  {
    fprintf(stderr,"Biddy_Subset: this GDD type is not supported, yet!\n");
    return biddyNull;
  } else if ((biddyManagerType == BIDDYTYPEOFDDC) || (biddyManagerType == BIDDYTYPEOFDD) ||
              (biddyManagerType == BIDDYTYPEZFDDC) || (biddyManagerType == BIDDYTYPEZFDD) ||
              (biddyManagerType == BIDDYTYPETZFDDC) || (biddyManagerType == BIDDYTYPETZFDD))
  {
    fprintf(stderr,"Biddy_Subset: this GDD type is not supported, yet!\n");
    return biddyNull;
  } else {
    fprintf(stderr,"Biddy_Subset: Unsupported GDD type!\n");
    return biddyNull;
  }

  return r;
}

#ifdef __cplusplus
}
#endif

Biddy_Edge
BiddyManagedSubset(Biddy_Manager MNG, Biddy_Edge f, Biddy_Variable v,
                   Biddy_Boolean value)
{
  Biddy_Edge h;
  Biddy_Edge e, t, r;
  Biddy_Variable fv,tag,ntag;
  Biddy_Edge FF, GG, HH;
  unsigned int cindex;

  assert( MNG != NULL );
  assert( f != NULL );

  /* IMPLEMENTED FOR OBDD, OBDDC, ZBDDC, AND TZBDD */
  assert(
    (biddyManagerType == BIDDYTYPEOBDD) ||
    (biddyManagerType == BIDDYTYPEOBDDC) ||
    (biddyManagerType == BIDDYTYPEZBDDC) ||
    (biddyManagerType == BIDDYTYPETZBDD)
  );

  if (f == biddyZero) return biddyZero;

  FF = biddyNull;
  GG = biddyNull;
  HH = biddyNull;

  h = biddyVariableTable.table[v].variable;
  if (!value) {
    h = BiddyManagedNot(MNG,h);
  }

  /* TO DO: CACHE TABLE FOR OBDDC COULD BE OPTIMIZED BY USING COMPLEMENTED EDGES */
  if (biddyManagerType == BIDDYTYPEOBDDC) {
    if (((uintptr_t) f) > ((uintptr_t) h)) {
      FF = f;
      GG = h;
      HH = biddyZero;
    } else {
      FF = h;
      GG = f;
      HH = biddyZero;
    }
  }

  else {
    if (((uintptr_t) f) > ((uintptr_t) h)) {
      FF = f;
      GG = h;
      HH = biddyZero;
    } else {
      FF = h;
      GG = f;
      HH = biddyZero;
    }
  }

  /* IF RESULT IS NOT IN THE CACHE TABLE... */
  /* TO DO: CHECK ONLY IF IT IS POSSIBLE TO EXIST IN THE CACHE */
  cindex = 0;
  if (!findOp3Cache(MNG,biddyOPCache,FF,GG,HH,&r,&cindex))
  {
    if ((biddyManagerType == BIDDYTYPEOBDDC) || (biddyManagerType == BIDDYTYPEOBDD)) {
      if ((fv=BiddyV(f)) == v) {
        if (value) {
          r = BiddyManagedTaggedFoaNode(MNG,v,biddyZero,Biddy_InvCond(BiddyT(f),Biddy_GetMark(f)),v,TRUE);
          BiddyRefresh(r); /* FoaNode returns an obsolete node! */
        } else {
          r = BiddyManagedTaggedFoaNode(MNG,v,Biddy_InvCond(BiddyE(f),Biddy_GetMark(f)),biddyZero,v,TRUE);
          BiddyRefresh(r); /* FoaNode returns an obsolete node! */
        }
      }
      else if (BiddyIsSmaller(v,fv)) {
        if (value) {
          r = BiddyManagedTaggedFoaNode(MNG,v,biddyZero,f,v,TRUE);
          BiddyRefresh(r); /* FoaNode returns an obsolete node! */
        } else {
          r = BiddyManagedTaggedFoaNode(MNG,v,f,biddyZero,v,TRUE);
          BiddyRefresh(r); /* FoaNode returns an obsolete node! */
        }
      } else {
        e = BiddyManagedSubset(MNG,Biddy_InvCond(BiddyE(f),Biddy_GetMark(f)),v,value);
        t = BiddyManagedSubset(MNG,Biddy_InvCond(BiddyT(f),Biddy_GetMark(f)),v,value);
        r = BiddyManagedTaggedFoaNode(MNG,fv,e,t,fv,TRUE);
        BiddyRefresh(r); /* FoaNode returns an obsolete node! */
        addOp3Cache(MNG,biddyOPCache,FF,GG,HH,r,cindex);
      }
    }

    else if (biddyManagerType == BIDDYTYPEZBDDC) {
      if ((fv=BiddyV(f)) == v) {
        if (value) {
          r = BiddyManagedTaggedFoaNode(MNG,v,biddyZero,BiddyT(f),0,TRUE);
          BiddyRefresh(r); /* FoaNode returns an obsolete node! */
        } else {
          r = Biddy_InvCond(BiddyE(f),Biddy_GetMark(f));
        }
      }
      else if (BiddyIsSmaller(v,fv)) {
        if (value) {
          r = biddyZero;
        } else {
          r = f;
        }
      } else {
        e = BiddyManagedSubset(MNG,Biddy_InvCond(BiddyE(f),Biddy_GetMark(f)),v,value);
        t = BiddyManagedSubset(MNG,BiddyT(f),v,value);
        r = BiddyManagedTaggedFoaNode(MNG,fv,e,t,0,TRUE);
        BiddyRefresh(r); /* FoaNode returns an obsolete node! */
        addOp3Cache(MNG,biddyOPCache,FF,GG,HH,r,cindex);
      }
    }

    else if (biddyManagerType == BIDDYTYPETZBDD) {
      tag = Biddy_GetTag(f);
      if (BiddyIsSmaller(v,tag)) {
        if (value) {
          r = BiddyManagedTaggedFoaNode(MNG,v,biddyZero,f,v,TRUE);
          BiddyRefresh(r); /* FoaNode returns an obsolete node! */
        } else {
          ntag = biddyVariableTable.table[v].next;
          if (tag == ntag) {
            r = f;
            Biddy_SetTag(r,v);
          } else {
            r = BiddyManagedTaggedFoaNode(MNG,ntag,f,f,v,TRUE);
            BiddyRefresh(r); /* FoaNode returns an obsolete node! */
          }
        }
      } else {
        fv = BiddyV(f);
        if (v == fv) {
          if (value) {
            r = BiddyManagedTaggedFoaNode(MNG,v,biddyZero,BiddyT(f),tag,TRUE);
            BiddyRefresh(r); /* FoaNode returns an obsolete node! */
          } else {
            ntag = biddyVariableTable.table[v].next;
            e = BiddyE(f);
            if (ntag == Biddy_GetTag(e)) {
              r = e;
              if (r != biddyZero) {
                Biddy_SetTag(r,tag);
              }
            } else {
              r = BiddyManagedTaggedFoaNode(MNG,ntag,e,e,tag,TRUE);
              BiddyRefresh(r); /* FoaNode returns an obsolete node! */
            }
          }
        } else if (BiddyIsSmaller(v,fv)) {
          if (value) {
            r = biddyZero;
          } else {
            r = f;
          }
        } else {
          e = BiddyManagedSubset(MNG,BiddyE(f),v,value);
          t = BiddyManagedSubset(MNG,BiddyT(f),v,value);
          r = BiddyManagedTaggedFoaNode(MNG,fv,e,t,tag,TRUE);
          BiddyRefresh(r); /* FoaNode returns an obsolete node! */
        }
      }
    }

  } else {

    /* IF THE RESULT IS FROM CACHE TABLE, REFRESH IT! */
    BiddyRefresh(r);

  }

  return r;
}

/***************************************************************************//*!
\brief Function Biddy_Managed_CreateMinterm generates one minterm.

### Description
    The represented Boolean function depends on the variables given with
    parameter support whilst the parameter n determines the generated minterm.
### Side effects
    Implemented for OBDD, OBDDC, ZBDDC, and TZBDD.
### More info
    Macro Biddy_CreateMinterm(support,x) is defined for use with anonymous
    manager.
*******************************************************************************/

#ifdef __cplusplus
extern "C" {
#endif

Biddy_Edge
Biddy_Managed_CreateMinterm(Biddy_Manager MNG, Biddy_Edge support,
                            long long unsigned int x)
{
  long long unsigned int n;
  Biddy_Edge tmp,result;

  if (!MNG) MNG = biddyAnonymousManager;
  ZF_LOGI("Biddy_CreateMinterm");

  if (biddyManagerType == BIDDYTYPEOBDD) {
    /* IMPLEMENTED */
    /* assert( printf("Biddy_CreateMinterm: OBDD\n") ); */
  } else if (biddyManagerType == BIDDYTYPEOBDDC) {
    /* IMPLEMENTED */
    /* assert( printf("Biddy_CreateMinterm: OBDDC\n") ); */
  } else if (biddyManagerType == BIDDYTYPEZBDDC) {
    /* IMPLEMENTED */
    /* assert( printf("Biddy_CreateMinterm: ZBDDC\n") ); */
  } else if (biddyManagerType == BIDDYTYPETZBDD) {
    /* IMPLEMENTED */
    /* assert( printf("Biddy_CreateMinterm: TZBDD\n") ); */
  } else if ((biddyManagerType == BIDDYTYPEZBDD) ||
              (biddyManagerType == BIDDYTYPETZBDDC))
  {
    fprintf(stderr,"Biddy_CreateMinterm: this GDD type is not supported, yet!\n");
    return biddyNull;
  } else if ((biddyManagerType == BIDDYTYPEOFDDC) || (biddyManagerType == BIDDYTYPEOFDD) ||
              (biddyManagerType == BIDDYTYPEZFDDC) || (biddyManagerType == BIDDYTYPEZFDD) ||
              (biddyManagerType == BIDDYTYPETZFDDC) || (biddyManagerType == BIDDYTYPETZFDD))
  {
    fprintf(stderr,"Biddy_CreateMinterm: this GDD type is not supported, yet!\n");
    return biddyNull;
  } else {
    fprintf(stderr,"Biddy_CreateMinterm: Unsupported GDD type!\n");
    return biddyNull;
  }

  n = 1;
  tmp = support;
  while (!Biddy_IsTerminal(tmp)) {

    if (!(n<(2*n))) {
      /* support has to many variables */
      return biddyNull;
    }

    n = 2 * n;
    tmp = BiddyT(tmp);
  }

  result = createMinterm(MNG,support,n,x);
  BiddyRefresh(result);  /* not refreshed by minterm */

  return result;
}

#ifdef __cplusplus
}
#endif

/***************************************************************************//*!
\brief Function Biddy_Managed_CreateFunction generates one Boolean function.

### Description
    The represented Boolean function depends on the variables given with
    parameter support whilst the parameter n determines the generated function.
### Side effects
    Implemented for OBDD, OBDDC, ZBDDC, and TZBDD.
### More info
    Macro Biddy_CreateFunction(support,x) is defined for use with anonymous
    manager.
*******************************************************************************/

#ifdef __cplusplus
extern "C" {
#endif

Biddy_Edge
Biddy_Managed_CreateFunction(Biddy_Manager MNG, Biddy_Edge support, long long unsigned int x)
{
  long long unsigned int n,m,q;
  Biddy_Edge tmp,result;

  if (!MNG) MNG = biddyAnonymousManager;
  ZF_LOGI("Biddy_CreateFunction");

  if (biddyManagerType == BIDDYTYPEOBDD) {
    /* IMPLEMENTED */
    /* assert( printf("Biddy_CreateFunction: OBDD\n") ); */
  } else if (biddyManagerType == BIDDYTYPEOBDDC) {
    /* IMPLEMENTED */
    /* assert( printf("Biddy_CreateFunction: OBDDC\n") ); */
  } else if (biddyManagerType == BIDDYTYPEZBDDC) {
    /* IMPLEMENTED */
    /* assert( printf("Biddy_CreateFunction: ZBDDC\n") ); */
  } else if (biddyManagerType == BIDDYTYPETZBDD) {
    /* IMPLEMENTED */
    /* assert( printf("Biddy_CreateFunction: TZBDD\n") ); */
  } else if ((biddyManagerType == BIDDYTYPEZBDD) ||
              (biddyManagerType == BIDDYTYPETZBDDC))
  {
    fprintf(stderr,"Biddy_CreateFunction: this GDD type is not supported, yet!\n");
    return biddyNull;
  } else if ((biddyManagerType == BIDDYTYPEOFDDC) || (biddyManagerType == BIDDYTYPEOFDD) ||
              (biddyManagerType == BIDDYTYPEZFDDC) || (biddyManagerType == BIDDYTYPEZFDD) ||
              (biddyManagerType == BIDDYTYPETZFDDC) || (biddyManagerType == BIDDYTYPETZFDD))
  {
    fprintf(stderr,"Biddy_CreateFunction: this GDD type is not supported, yet!\n");
    return biddyNull;
  } else {
    fprintf(stderr,"Biddy_CreateFunction: Unsupported GDD type!\n");
    return biddyNull;
  }

  n = 1;
  tmp = support;
  while (!Biddy_IsTerminal(tmp)) {

    if (!(n<(2*n))) {
      /* support has to many variables */
      return biddyNull;
    }

    n = 2 * n;
    tmp = BiddyT(tmp);
  }

  m = 1;
  q = n;
  while (q>0) {

    if (!(m<(2*m))) {
      /* support has to many variables */
      return biddyNull;
    }

    m = 2 * m;
    q--;
  }

  result = createFunction(MNG,support,n,m,x);
  BiddyRefresh(result);  /* not refreshed by function */

  return result;
}

#ifdef __cplusplus
}
#endif

/***************************************************************************//*!
\brief Function Biddy_Managed_RandomFunction generates a random BDD.

### Description
    The represented Boolean function depends on the variables given with
    parameter support whilst the parameter r determines the ratio between
    the number of function's minterms and the number of all possible minterms.
    Parameter support is a product of positive varaiables.
### Side effects
    Implemented for OBDD, OBDDC, ZBDDC, and TZBDD.
    Parameter r must be a number from [0,1]. Otherwise, function returns
    biddyNull.
### More info
    Macro Biddy_RandomFunction(support,r) is defined for use with anonymous
    manager.
*******************************************************************************/

#ifdef __cplusplus
extern "C" {
#endif

Biddy_Edge
Biddy_Managed_RandomFunction(Biddy_Manager MNG, Biddy_Edge support, double r)
{
  long long unsigned int n,m,x;
  Biddy_Edge tmp,result;
  Biddy_Boolean opposite;

  if (!MNG) MNG = biddyAnonymousManager;
  ZF_LOGI("Biddy_RandomFunction");

  if (biddyManagerType == BIDDYTYPEOBDD) {
    /* IMPLEMENTED */
    /* assert( printf("Biddy_Random: OBDD\n") ); */
  } else if (biddyManagerType == BIDDYTYPEOBDDC) {
    /* IMPLEMENTED */
    /* assert( printf("Biddy_Random: OBDDC\n") ); */
  } else if (biddyManagerType == BIDDYTYPEZBDDC) {
    /* IMPLEMENTED */
    /* assert( printf("Biddy_Random: ZBDDC\n") ); */
  } else if (biddyManagerType == BIDDYTYPETZBDD) {
    /* IMPLEMENTED */
    /* assert( printf("Biddy_Random: TZBDD\n") ); */
  } else if ((biddyManagerType == BIDDYTYPEZBDD) ||
              (biddyManagerType == BIDDYTYPETZBDDC))
  {
    fprintf(stderr,"Biddy_Random: this GDD type is not supported, yet!\n");
    return biddyNull;
  } else if ((biddyManagerType == BIDDYTYPEOFDDC) || (biddyManagerType == BIDDYTYPEOFDD) ||
              (biddyManagerType == BIDDYTYPEZFDDC) || (biddyManagerType == BIDDYTYPEZFDD) ||
              (biddyManagerType == BIDDYTYPETZFDDC) || (biddyManagerType == BIDDYTYPETZFDD))
  {
    fprintf(stderr,"Biddy_Random: this GDD type is not supported, yet!\n");
    return biddyNull;
  } else {
    fprintf(stderr,"Biddy_Random: Unsupported GDD type!\n");
    return biddyNull;
  }

  if ((r < 0.0) || (r > 1.0)) {
    return biddyNull;
  }

  n = 1;
  tmp = support;
  while (!Biddy_IsTerminal(tmp)) {

    if (!(n<(2*n))) {
      /* support has to many variables */
      return biddyNull;
    }

    n = 2 * n;
    tmp = BiddyT(tmp);
  }

  opposite = FALSE;
  if (r > 0.5) {
    r = 1.0 - r;
    opposite = TRUE;
  }

  m = (long long unsigned int) (0.5 + n * r);

  if (m == 0) {
    if (opposite) {
      return biddyOne;
    } else {
      return biddyZero;
    }
  }

  result = biddyZero;
  while (m != 0) {

    /* DEBUGGING */
    /*
    printf("RANDOM: m=%llu\n",m);
    */

    /* GENERATE RANDOM NUMBER */
    x = rand();
    x = (n*x)/RAND_MAX;

    /* DEBUGGING */
    /*
    printf("RANDOM NUMBER = %llu\n",x);
    */

    tmp = createMinterm(MNG,support,n,x);
    tmp = BiddyManagedOr(MNG,result,tmp);
    if (tmp != result) {
      result = tmp;
      m--;
    }

  }

  if (opposite) {
    result = BiddyManagedNot(MNG,result);
  }

  BiddyRefresh(result); /* not always refreshed by Or/Not */

  return result;
}

#ifdef __cplusplus
}
#endif

/***************************************************************************//*!
\brief Function Biddy_Managed_RandomSet generates a random BDD.

### Description
    The represented set is a random combination set determined by the
    parameter unit whilst the parameter r determines the ratio between
    the number of set's subsets and the number of all possible subsets.
    Parameter set is a set containing only one subset which consist
    of all elements, i.e. it is a set {{x1,x2,...,xn}}.
### Side effects
    Implemented for OBDD, OBDDC, ZBDDC, and TZBDD.
    Parameter r must be a number from [0,1]. Otherwise, function returns
    biddyNull.
### More info
    Macro Biddy_RandomSet(unit,r) is defined for use with anonymous manager.
*******************************************************************************/

#ifdef __cplusplus
extern "C" {
#endif

Biddy_Edge
Biddy_Managed_RandomSet(Biddy_Manager MNG, Biddy_Edge unit, double r)
{
  long long unsigned int n,m,x,z;
  Biddy_Edge p,q,tmp,result;
  Biddy_Boolean opposite;
  Biddy_Variable one;

  if (!MNG) MNG = biddyAnonymousManager;
  ZF_LOGI("Biddy_RandomSet");

  if (biddyManagerType == BIDDYTYPEOBDD) {
    /* IMPLEMENTED */
    /* assert( printf("Biddy_RandomSet: OBDD\n") ); */
  } else if (biddyManagerType == BIDDYTYPEOBDDC) {
    /* IMPLEMENTED */
    /* assert( printf("Biddy_RandomSet: OBDD\n") ); */
  } else if (biddyManagerType == BIDDYTYPEZBDDC) {
    /* IMPLEMENTED */
    /* assert( printf("Biddy_RandomSet: ZBDD\n") ); */
  } else if (biddyManagerType == BIDDYTYPETZBDD) {
    /* IMPLEMENTED */
    /* assert( printf("Biddy_RandomSet: TZBDD\n") ); */
  } else if ((biddyManagerType == BIDDYTYPEZBDD) ||
              (biddyManagerType == BIDDYTYPETZBDDC))
  {
    fprintf(stderr,"Biddy_RandomSet: this GDD type is not supported, yet!\n");
    return biddyNull;
  } else if ((biddyManagerType == BIDDYTYPEOFDDC) || (biddyManagerType == BIDDYTYPEOFDD) ||
              (biddyManagerType == BIDDYTYPEZFDDC) || (biddyManagerType == BIDDYTYPEZFDD) ||
              (biddyManagerType == BIDDYTYPETZFDDC) || (biddyManagerType == BIDDYTYPETZFDD))
  {
    fprintf(stderr,"Biddy_RandomSet: this GDD type is not supported, yet!\n");
    return biddyNull;
  } else {
    fprintf(stderr,"Biddy_RandomSet: Unsupported GDD type!\n");
    return biddyNull;
  }

  if ((r < 0.0) || (r > 1.0)) {
    return biddyNull;
  }

  n = 1;
  p = unit;
  while (!Biddy_IsTerminal(p)) {

    if (!(n<(2*n))) {
      /* unit has to many variables */
      return biddyNull;
    }

    n = 2 * n;
    p = BiddyT(p);
  }

  opposite = FALSE;
  if (r > 0.5) {
    r = 1.0 - r;
    opposite = TRUE;
  }

  m = (long long unsigned int) (0.5 + n * r);

  if (m == 0) {
    if (opposite) {
      return biddyOne;
    } else {
      return biddyZero;
    }
  }

  result = biddyZero;
  while (m != 0) {

    /* DEBUGGING */
    /*
    printf("RANDOM SET: m=%llu\n",m);
    */

    /* GENERATE RANDOM NUMBER */
    x = rand();
    x = (n*x)/RAND_MAX;

    /* DEBUGGING */
    /*
    printf("RANDOM NUMBER = %llu\n",x);
    */

    p = unit;
    z = n;
    q = Biddy_Managed_GetBaseSet(MNG);
    while (z != 1) {
      one = BiddyV(p);
      p = BiddyT(p);
      if (x <= ((z - 0.5) / 2)) {
        z = (long long unsigned int) (z + 0.5) / 2;
      } else {
        z = (long long unsigned int) (z + 0.5) / 2;
        x = x - z;
        q = BiddyManagedChange(MNG,q,one);
      }
    }

    tmp = BiddyManagedUnion(MNG,result,q);
    if (tmp != result) {
      result = tmp;
      m--;
    }

  }

  if (opposite) {
    result = BiddyManagedNot(MNG,result);
  }

  BiddyRefresh(result); /* not always refreshed by Union/Not */

  return result;
}

#ifdef __cplusplus
}
#endif

/*----------------------------------------------------------------------------*/
/* Definition of internal functions                                           */
/*----------------------------------------------------------------------------*/

/*******************************************************************************
\brief Function BiddyOPGarbage performs Garbage Collection for the OP cache.

### Description
### Side effects
### More info
*******************************************************************************/

void
BiddyOPGarbage(Biddy_Manager MNG)
{
  unsigned int j;
  BiddyOp3Cache *c;

  if (biddyOPCache.disabled) return;

  for (j=0; j<=biddyOPCache.size; j++) {
    if (!Biddy_IsNull(biddyOPCache.table[j].result)) {
      c = &biddyOPCache.table[j];
      /* VARIANT A */
      /*
      if (((BiddyN(c->f)->expiry) && ((BiddyN(c->f)->expiry) < biddySystemAge)) ||
          ((BiddyN(c->g)->expiry) && ((BiddyN(c->g)->expiry) < biddySystemAge)) ||
          ((BiddyN(c->h)->expiry) && ((BiddyN(c->h)->expiry) < biddySystemAge)) ||
          ((BiddyN(c->result)->expiry) && ((BiddyN(c->result)->expiry) < biddySystemAge)))
      */
      /* VARIANT B */
      /*
      if (((BiddyN(c->result)->expiry) && ((BiddyN(c->result)->expiry) < biddySystemAge)) ||
          ((BiddyN(c->f)->expiry) && ((BiddyN(c->f)->expiry) < biddySystemAge)) ||
          ((BiddyN(c->g)->expiry) && ((BiddyN(c->g)->expiry) < biddySystemAge)) ||
          ((BiddyN(c->h)->expiry) && ((BiddyN(c->h)->expiry) < biddySystemAge)))
      */
      /* VARIANT C */
      /**/
      if (((BiddyN(c->f)->expiry) && ((BiddyN(c->f)->expiry) < biddySystemAge)) ||
          ((BiddyN(c->result)->expiry) && ((BiddyN(c->result)->expiry) < biddySystemAge)) ||
          ((BiddyN(c->g)->expiry) && ((BiddyN(c->g)->expiry) < biddySystemAge)) ||
          ((BiddyN(c->h)->expiry) && ((BiddyN(c->h)->expiry) < biddySystemAge)))
      /**/
      {
        c->result = biddyNull;
      }
    }
  }
}

/*******************************************************************************
\brief Function BiddyOPGarbageNewVariable is used to clean the OP cache after
        the adding new variable.

### Description
    This function is needed for BDD types where adding new variable below
    the existing one changes the meaning of the existing formulae.
### Side effects
### More info
*******************************************************************************/

void
BiddyOPGarbageNewVariable(Biddy_Manager MNG, Biddy_Variable v)
{
  unsigned int j;
  BiddyOp3Cache *c;

  if (biddyOPCache.disabled) return;

  for (j=0; j<=biddyOPCache.size; j++) {
    if (!Biddy_IsNull(biddyOPCache.table[j].result)) {
      c = &biddyOPCache.table[j];
      if ((BiddyIsSmaller(BiddyN(c->f)->v,v)) ||
          (BiddyIsSmaller(BiddyN(c->g)->v,v)) ||
          (BiddyIsSmaller(BiddyN(c->h)->v,v)) ||
          (BiddyIsSmaller(BiddyN(c->result)->v,v)))
      {
        c->result = biddyNull;
      }
    }
  }
}

/*******************************************************************************
\brief Function BiddyOPGarbageDeleteAll deletes all entries in the OP cache.

### Description
### Side effects
### More info
*******************************************************************************/

void
BiddyOPGarbageDeleteAll(Biddy_Manager MNG)
{
  unsigned int j;
  BiddyOp3Cache *c;

  if (biddyOPCache.disabled) return;

  for (j=0; j<=biddyOPCache.size; j++) {
    c = &biddyOPCache.table[j];
    c->result = biddyNull;
  }
}

/*******************************************************************************
\brief Function BiddyEAGarbage performs Garbage Collection for the EA cache.

### Description
### Side effects
### More info
*******************************************************************************/

void
BiddyEAGarbage(Biddy_Manager MNG)
{
  unsigned int j;
  BiddyOp3Cache *c;

  if (biddyEACache.disabled) return;

  for (j=0; j<=biddyEACache.size; j++) {
    if (!Biddy_IsNull(biddyEACache.table[j].result)) {
      c = &biddyEACache.table[j];
      if (((BiddyN(c->f)->expiry) && ((BiddyN(c->f)->expiry) < biddySystemAge)) ||
          ((BiddyN(c->g)->expiry) && ((BiddyN(c->g)->expiry) < biddySystemAge)) ||
          ((BiddyN(c->h)->expiry) && ((BiddyN(c->h)->expiry) < biddySystemAge)) ||
          ((BiddyN(c->result)->expiry) && ((BiddyN(c->result)->expiry) < biddySystemAge)))
      {
        c->result = biddyNull;
      }
    }
  }
}

/*******************************************************************************
\brief Function BiddyEAGarbageNewVariable is used to clean the EA cache after
        the adding new variable.

### Description
    This function is needed for BDD types where adding new variable below
    the existing one changes the meaning of the existing formulae.
### Side effects
### More info
*******************************************************************************/

void
BiddyEAGarbageNewVariable(Biddy_Manager MNG, Biddy_Variable v)
{
  unsigned int j;
  BiddyOp3Cache *c;

  if (biddyEACache.disabled) return;

  for (j=0; j<=biddyEACache.size; j++) {
    if (!Biddy_IsNull(biddyEACache.table[j].result)) {
      c = &biddyEACache.table[j];
      if ((BiddyIsSmaller(BiddyN(c->f)->v,v)) ||
          (BiddyIsSmaller(BiddyN(c->g)->v,v)) ||
          (BiddyIsSmaller(BiddyN(c->h)->v,v)) ||
          (BiddyIsSmaller(BiddyN(c->result)->v,v)))
      {
        c->result = biddyNull;
      }
    }
  }
}

/*******************************************************************************
\brief Function BiddyEAGarbageDeleteAll deletes all entries in the EA cache.

### Description
### Side effects
### More info
*******************************************************************************/

void
BiddyEAGarbageDeleteAll(Biddy_Manager MNG)
{
  unsigned int j;
  BiddyOp3Cache *c;

  if (biddyEACache.disabled) return;

  for (j=0; j<=biddyEACache.size; j++) {
    c = &biddyEACache.table[j];
    c->result = biddyNull;
  }
}

/*******************************************************************************
\brief Function BiddyRCGarbage performs Garbage Collection for the RC cache.

### Description
### Side effects
### More info
*******************************************************************************/

void
BiddyRCGarbage(Biddy_Manager MNG)
{
  unsigned int j;
  BiddyOp3Cache *c;

  if (biddyRCCache.disabled) return;

  for (j=0; j<=biddyRCCache.size; j++) {
    if (!Biddy_IsNull(biddyRCCache.table[j].result)) {
      c = &biddyRCCache.table[j];
      if (((BiddyN(c->f)->expiry) && ((BiddyN(c->f)->expiry) < biddySystemAge)) ||
          (c->g && (BiddyN(c->g)->expiry) && ((BiddyN(c->g)->expiry) < biddySystemAge)) ||
          ((BiddyN(c->h)->expiry) && ((BiddyN(c->h)->expiry) < biddySystemAge)) ||
          ((BiddyN(c->result)->expiry) && ((BiddyN(c->result)->expiry) < biddySystemAge)))
      {
        c->result = biddyNull;
      }
    }
  }
}

/*******************************************************************************
\brief Function BiddyRCGarbageNewVariable is used to clean the RC cache after
        the adding new variable.

### Description
    This function is needed for BDD types where adding new variable below
    the existing one changes the meaning of the existing formulae.
### Side effects
### More info
*******************************************************************************/

void
BiddyRCGarbageNewVariable(Biddy_Manager MNG, Biddy_Variable v)
{
  unsigned int j;
  BiddyOp3Cache *c;

  if (biddyRCCache.disabled) return;

  for (j=0; j<=biddyRCCache.size; j++) {
    if (!Biddy_IsNull(biddyRCCache.table[j].result)) {
      c = &biddyRCCache.table[j];
      if ((BiddyIsSmaller(BiddyN(c->f)->v,v)) ||
          (c->g && BiddyIsSmaller(BiddyN(c->g)->v,v)) ||
          (BiddyIsSmaller(BiddyN(c->h)->v,v)) ||
          (BiddyIsSmaller(BiddyN(c->result)->v,v)))
      {
        c->result = biddyNull;
      }
    }
  }
}

/*******************************************************************************
\brief Function BiddyRCGarbageDeleteAll deletes all entries in the RC cache.

### Description
### Side effects
### More info
*******************************************************************************/

void
BiddyRCGarbageDeleteAll(Biddy_Manager MNG)
{
  unsigned int j;
  BiddyOp3Cache *c;

  if (biddyRCCache.disabled) return;

  for (j=0; j<=biddyRCCache.size; j++) {
    c = &biddyRCCache.table[j];
    c->result = biddyNull;
  }
}

/*******************************************************************************
\brief Function BiddyReplaceGarbage performs Garbage Collection for the Replace
        cache.

### Description
### Side effects
### More info
*******************************************************************************/

void
BiddyReplaceGarbage(Biddy_Manager MNG)
{
  unsigned int j;
  BiddyKeywordCache *c;

  if (biddyReplaceCache.disabled) return;

  for (j=0; j<=biddyReplaceCache.size; j++) {
    if (!Biddy_IsNull(biddyReplaceCache.table[j].result)) {
      c = &biddyReplaceCache.table[j];
      if (((BiddyN(c->f)->expiry) && ((BiddyN(c->f)->expiry) < biddySystemAge)) ||
          ((BiddyN(c->result)->expiry) && ((BiddyN(c->result)->expiry) < biddySystemAge)))
      {
        c->result = biddyNull;
      }
    }
  }
}

/*******************************************************************************
\brief Function BiddyReplaceGarbageNewVariable is used to clean the Replace
        cache after the adding new variable.

### Description
    This function is needed for BDD types where adding new variable below
    the existing one changes the meaning of the existing formulae.
### Side effects
### More info
*******************************************************************************/

void
BiddyReplaceGarbageNewVariable(Biddy_Manager MNG, Biddy_Variable v)
{
  unsigned int j;
  BiddyKeywordCache *c;

  if (biddyReplaceCache.disabled) return;

  for (j=0; j<=biddyReplaceCache.size; j++) {
    if (!Biddy_IsNull(biddyReplaceCache.table[j].result)) {
      c = &biddyReplaceCache.table[j];
      if ((BiddyIsSmaller(BiddyN(c->f)->v,v)) ||
          (BiddyIsSmaller(BiddyN(c->result)->v,v)))
      {
        c->result = biddyNull;
      }
    }
  }
}

/*******************************************************************************
\brief Function BiddyReplaceGarbageDeleteAll deletes all entries in the Replace
        cache.

### Description
### Side effects
### More info
*******************************************************************************/

void
BiddyReplaceGarbageDeleteAll(Biddy_Manager MNG)
{
  unsigned int j;
  BiddyKeywordCache *c;

  if (biddyReplaceCache.disabled) return;

  for (j=0; j<=biddyReplaceCache.size; j++) {
    c = &biddyReplaceCache.table[j];
    c->result = biddyNull;
  }
}

/*----------------------------------------------------------------------------*/
/* Definition of static functions                                             */
/*----------------------------------------------------------------------------*/

/*******************************************************************************
\brief Function exchangeEdges exchanges two functions.

### Description
### Side effects
### More info
*******************************************************************************/

static void
exchangeEdges(Biddy_Edge *f, Biddy_Edge *g)
{
  Biddy_Edge sup;

  sup = *f;
  *f = *g;
  *g = sup;
}

/*******************************************************************************
\brief Function complExchangeEdges complements and exchanges two functions.

### Description
### Side effects
### More info
*******************************************************************************/

static void
complExchangeEdges(Biddy_Edge *f, Biddy_Edge *g)
{
  Biddy_Edge sup;

  sup = *f;
  *f = *g;
  *g = sup;
  Biddy_InvertMark((*f));
  Biddy_InvertMark((*g));
}

/*******************************************************************************
\brief Function createMinterm generates one minterm.

### Description
### Side effects
### More info
*******************************************************************************/

static Biddy_Edge
createMinterm(Biddy_Manager MNG, Biddy_Edge support, long long unsigned int n,
              long long unsigned int x)
{
  Biddy_Edge one,q;

  q = biddyOne;
  while (n != 1) {

    one = biddyVariableTable.table[BiddyV(support)].variable;

    assert( !Biddy_IsTerminal(one) );

    support = BiddyT(support);
    n = n / 2;
    if (x < n) {
      q = BiddyManagedITE(MNG,one,biddyZero,q);
    } else {
      x = x - n;
      q = BiddyManagedITE(MNG,one,q,biddyZero);
    }
  }

  return q;
}

/*******************************************************************************
\brief Function createFunction generates one Boolean function.

### Description
### Side effects
### More info
*******************************************************************************/

static Biddy_Edge
createFunction(Biddy_Manager MNG, Biddy_Edge support, long long unsigned int n,
               long long unsigned int m, long long unsigned int x)
{
  Biddy_Edge mnt,q;
  long long unsigned int y;

  q = biddyZero;
  y = 0;
  while (m > 1) {
    m = m / 2;
    if (x >= m) {
      x = x - m;
      mnt = createMinterm(MNG,support,n,y);
      q = BiddyManagedOr(MNG,q,mnt);
    }
    y++;
  }

  return q;
}

/*******************************************************************************
\brief Function op3Hash.

### Description
### Side effects
### More info
*******************************************************************************/

static inline unsigned int
op3Hash(Biddy_Edge a, Biddy_Edge b, Biddy_Edge c, unsigned int size)
{

  unsigned int hash;

  /* this is very simple but quite efficient */
  /*
  {
  uintptr_t k;
  k = ((uintptr_t) a >> 4) + ((uintptr_t) b >> 2) + ((uintptr_t) c >> 4);
  hash = k & size;
  return hash;
  }
  */

#if UINTPTR_MAX == 0xffffffffffffffff

  /* this is experimentally determined */
  /* rndc1 = 12582917, rndc2 = 4256249, >> 1,1,2,8, calls: 33415082 */
  /* rndc1 = 1982250849, rndc2 = 423214982, >> 0,0,0,16, calls: 33382146 */
  /* rndc1 = 1327958971, rndc2=516963069, >> 1,2,0,19, calls: 33374477 */
  /**/
  {
  uintptr_t k;
  k = ((((uintptr_t) b >> 1) + ((uintptr_t) c >> 2)) * 1327958971 + (uintptr_t) a) * 516963069;
  hash = (k >> 19) & size;
  return hash;
  }
  /**/

  /* this is using ideas from MURMUR3, see https://code.google.com/p/smhasher/ */
  /*
  {
  uintptr_t k,k1,k2;
  k = (uintptr_t) a;
  k1 = (uintptr_t) b;
  k1 = (k1 << 31) | (k1 >> 33);
  k ^= k1;
  k2 = (uintptr_t) c;
  k2 = (k2 << 35) | (k2 >> 29);
  k ^= k2;
  k *= 0xff51afd7ed558ccdL;
  k ^= k >> 32;
  k *= 0xc4ceb9fe1a85ec53L;
  k ^= k >> 32;
  hash = k & size;
  return hash;
  }
  */

#else

  /* MURMUR3 (A KIND OF) 32-bit HASH FUNCTION */
  /* https://code.google.com/p/smhasher/ */
  {
  uintptr_t k,k1,k2;
  k = (uintptr_t) a;
  k1 = (uintptr_t) b;
  k1 = (k1 << 15) | (k1 >> 17);
  k ^= k1;
  k2 = (uintptr_t) c;
  k2 = (k2 << 19) | (k2 >> 13);
  k ^= k2;
  k *= 0x85ebca6b;
  k ^= k >> 16;
  k *= 0xc2b2ae35;
  k ^= k >> 16;
  hash = k & size;
  return hash;
  }

#endif

  return 0;
}

/*******************************************************************************
\brief Function keywordHash.

### Description
### Side effects
### More info
*******************************************************************************/

static inline unsigned int
keywordHash(Biddy_Edge a, unsigned int k, unsigned int size)
{

  unsigned int hash;

  /* this is very simple */
  /**/
  {
  uintptr_t key;
  key = (uintptr_t) a >> 3;
  hash = key & size;
  return hash;
  }
  /**/

#if UINTPTR_MAX == 0xffffffffffffffff

  /* this is experimentally determined */
  {
  uintptr_t key;
  key = ((uintptr_t) a >> 3) * 1327958971;
  hash = (key >> 19) & size;
  return hash;
  }

#else

  /* this is experimentally determined */
  {
  uintptr_t key;
  key = ((uintptr_t) a >> 3) * 7958971;
  hash = (key >> 13) & size;
  return hash;
  }

#endif

  return 0;
}

/*******************************************************************************
\brief Function addOp3Cache adds a result to the cache table for
       three-arguments operations.

### Description
### Side effects
### More info
*******************************************************************************/

static inline void
addOp3Cache(Biddy_Manager MNG, BiddyOp3CacheTable cache, Biddy_Edge a,
            Biddy_Edge b, Biddy_Edge c, Biddy_Edge r, unsigned int index)
{
  BiddyOp3Cache *p;

  if (cache.disabled) return;

  p = &cache.table[index];

#ifdef BIDDYEXTENDEDSTATS_YES
  if (!Biddy_IsNull(p->result)) {
    /* THE CELL IS NOT EMPTY, THUS THIS IS OVERWRITING */
    (*cache.overwrite)++;
  }
#endif

#ifdef BIDDYEXTENDEDSTATS_YES
  (*cache.insert)++;
#endif

  p->f = a;
  p->g = b;
  p->h = c;
  p->result = r;
}

/*******************************************************************************
\brief Function findOp3Cache looks for the result in the cache table for
       three-arguments operations.

### Description
### Side effects
### More info
*******************************************************************************/

static inline Biddy_Boolean
findOp3Cache(Biddy_Manager MNG, BiddyOp3CacheTable cache, Biddy_Edge a,
             Biddy_Edge b, Biddy_Edge c, Biddy_Edge *r, unsigned int *index)
{
  Biddy_Boolean q;
  BiddyOp3Cache *p;

  if (cache.disabled) return FALSE;

  q = FALSE;

  *index = op3Hash(a,b,c,cache.size);
  p = &cache.table[*index];

  (*cache.search)++;

  if ((p->f == a) &&
      (p->g == b) &&
      (p->h == c) &&
      !Biddy_IsNull(p->result)
     )
  {
    (*cache.find)++;
    *r = p->result;
    q = TRUE;
  }

  /* DEBUGGING */
  /*
  if (q) printf("OP3CACHE RETURNED: %p\n",(void *) *r);
  */

  return q;
}

/*******************************************************************************
\brief Function addKeywordCache adds a result to the cache table for
       one-arguments operation and labels the entry with a given keyword.

### Description
### Side effects
### More info
*******************************************************************************/

static inline void
addKeywordCache(Biddy_Manager MNG, BiddyKeywordCacheTable cache,
                Biddy_Edge a, unsigned int k, Biddy_Edge r,
                unsigned int index)
{
  BiddyKeywordCache *p;

  if (cache.disabled) return;

  p = &cache.table[index];

#ifdef BIDDYEXTENDEDSTATS_YES
  if (!Biddy_IsNull(p->result)) {
    /* THE CELL IS NOT EMPTY, THUS THIS IS OVERWRITING */
    (*cache.overwrite)++;
  }
#endif

#ifdef BIDDYEXTENDEDSTATS_YES
  (*cache.insert)++;
#endif

  p->f = a;
  p->keyword = k;
  p->result = r;
}

/*******************************************************************************
\brief Function findKeywordCache looks for the result in the cache table for
       one-argument operation which is labelled with a given keyword.

### Description
### Side effects
### More info
*******************************************************************************/

static inline Biddy_Boolean
findKeywordCache(Biddy_Manager MNG, BiddyKeywordCacheTable cache,
                 Biddy_Edge a, unsigned int k, Biddy_Edge *r,
                 unsigned int *index)
{
  Biddy_Boolean q;
  BiddyKeywordCache *p;

  if (cache.disabled) return FALSE;

  q = FALSE;

  *index = keywordHash(a,k,cache.size);
  p = &cache.table[*index];

  (*cache.search)++;

  if ((p->f == a) &&
      (p->keyword == k) &&
      !Biddy_IsNull(p->result)
     )
  {
    (*cache.find)++;
    *r = p->result;
    q = TRUE;
  }

  return q;
}

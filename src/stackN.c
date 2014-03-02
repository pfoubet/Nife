/* Copyright (C) 2011-2013  Patrick H. E. Foubet - S.E.R.I.A.N.E.

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or any
later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>
*******************************************************************/
/* stackN.c */
#include "conf.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <sys/types.h>
#include <math.h>

#include "nife.h"
#include "mth.h"
#include "err.h"
#include "lib.h"
#include "stackN.h"
#include "stackL.h"
#include "stackF.h"
#include "stackV.h"
#include "debug.h"
#include "net.h"
#include "scs.h"

#define _VERIF_STACK_ if (StackN == VIDE) { messErr(2); return; }


void IF_vars(void)
{
char * L;
    printf("DEBUG : ");
    if (Debug) printf("ON"); else printf("OFF");
    printf("\nDefault type=");
    if (DOUBLE) printf("REAL"); else printf("INTEGER");
    printf("\nDefault echo=");
    if (ECHOOFF) printf("OFF"); else printf("ON");
    printf("\nNetServer : \"%s\"",NetServer);
    printf("\nSCS Key   : 0x%lx",(long)getScs());
    printf("\nNetKey    : 0x%lx",(long)NetKey);
    printf("\nVARS : ");
    switch(VARS) {
       case 1 :
         printf("DOWN");
         break;
       case 2 :
         printf("UP");
         break;
       default :
         printf("OFF");
         break;
    }
    printf("\nVariable Function : ");
    if ((L=libByAddr(FCT_INST)) != NULL) printf("%s (std lib)",L);
    else {
      if ((L=fctByAddr(FCT_INST)) != NULL) printf("%s (user function)",L);
        else {
          if ((L=varByAddr(FCT_INST)) != NULL) printf("%s (variable)",L);
          else printf("none");
        }
    }
    printf("\nNBTAB=%d\nNBLIG=%d\n",NBTAB,NBLIG);
}

void IF_REAL(void) { _MODIF_DOUBLE_(1); }
void IF_INTEGER(void) { _MODIF_DOUBLE_(0); }

void IF_ECHOFF(void) { _MODIF_ECHOOFF_(1); }
void IF_ECHOON(void) { _MODIF_ECHOOFF_(0); }

/* IMPORTANT **************************
   la taille t est codee sur 30 bits + a droite
   B31 = 1 si Var
   B32 = 1 si REAL 
**********************/
#define MSK_T    (uint32_t)(0x3FFFFFFF)
#define MSK_V    (uint32_t)(0x40000000)
#define MSK_R    (uint32_t)(0x80000000)

struct Num {
    uint32_t t; /* taille : cf precisions ci-dessus */
    uint32_t key; /* net key */
    void *n;
    union {
      long long l;
      double d;
    };
};

int lAdrNum(void)
{
struct Num N;
   return((char*)&(N.l) - (char*)&(N.n));
}

void putLong(long long l)
{
void * M;
struct Num * N;
    if ((M = malloc(sizeof(struct Num))) == NULL) stopErr("putLong","malloc");
    N = (struct Num*)M;
    N->t = 1;
    N->n = StackN;
    N->l = l;
    _MODIF_STACKN_(M);
}

void putDouble(double d)
{
void * M;
struct Num * N;
    if ((M = malloc(sizeof(struct Num))) == NULL) stopErr("putDouble","malloc");
    N = (struct Num*)M;
    N->t = 1 | MSK_R;
    N->n = StackN;
    N->d = d;
    _MODIF_STACKN_(M);
}

int putVal(char *V)
{
void * M;
char * R;
struct Num * N;
long long l;
double d;
#ifdef DEBUG
    printf("putVal (%s) \n",V);
#endif
    l = strtoll(V,&R,0);
    if (strlen(R)==0) {
       if ((M = malloc(sizeof(struct Num))) == NULL) stopErr("putVal","malloc");
       N = (struct Num*)M;
       N->t = 1;
       N->n = StackN;
       N->l = l;
       if (fctEnCours) makeFct(T_NUM,M);
       else _MODIF_STACKN_(M);
       return 1;
    } else {
       d = strtod(V,&R);
       if (strlen(R)==0) {
         if ((M=malloc(sizeof(struct Num))) == NULL) stopErr("putVal","malloc");
         N = (struct Num*)M;
         N->t = 1 | MSK_R;
         N->n = StackN;
         N->d = d;
         if (fctEnCours) makeFct(T_NUM,M);
         else _MODIF_STACKN_(M);
         return 1;
       }
    }
    return 0;
}

static int isScalar(void)
{
struct Num *Elt;
int t;
   if(StackN == VIDE) return 0;
   Elt = (struct Num *)StackN;
   if ((t = Elt->t&MSK_T) == 1) return 1;
   return 0;
}

static int dropElt(void)
{
struct Num *Elt;
   if(StackN == VIDE) return 0;
   Elt = (struct Num *)StackN;
   _MODIF_STACKN_(Elt->n);
   if (!(Elt->t&MSK_V)) free((void*)Elt);
   return 1;
}

void IF_drop(void)
{
   _VERIF_STACK_
   dropElt();
}


static long long getVal(void)
{
struct Num *Elt;
   Elt = (struct Num *)StackN;
   if (Elt->t & MSK_R) return((long long)Elt->d);
   else  return(Elt->l);
}

void IF_vers(void)
{
    putDouble(atof(VERSION));
}

/* fonction pour les autres */
int getParLong(long *V)
{
   if (StackN == VIDE) { 
        messErr(2); return 0 ;
   }
   if (!isScalar()) {
        messErr(36); return 0 ;
   }
   *V = (long)getVal();
   dropElt();
   return 1;
}


void putVar(void * V)
{
struct Num *Elt;
   if (V==VIDE) return;
   Elt = (struct Num *)V;
   Elt->n = StackN;
   _MODIF_STACKN_(V);
}

void * getVar(void)
{
void * N;
struct Num *Elt;
   N = StackN;
   if (N != VIDE) {
      Elt = (struct Num *)N;
      _MODIF_STACKN_(Elt->n); /* drop no free !! */
      Elt->n = VIDE;
      Elt->t = Elt->t|MSK_V; /* VARIABLE ! */
   }
   return N;
}

void IF_NBTAB(void)
{
long V;
   if (getParLong(&V)) _MODIF_NBTAB_(V);
}

void IF_NBLIG(void)
{
long V;
   if (getParLong(&V)) _MODIF_NBLIG_(V);
}

void IF_VAROFF(void) { _MODIF_VARS_(0); }
void IF_VARDOWN(void) { _MODIF_VARS_(1); }
void IF_VARUP(void) { _MODIF_VARS_(2); }

void insertVal(void*A)
{
void * M;
struct Num *Elt;
    if ((M = malloc(sizeof(struct Num))) == NULL) stopErr("insertVal","malloc");
    bcopy(A,M,sizeof(struct Num));
    Elt=(struct Num*)M;
    Elt->n = StackN;
    _MODIF_STACKN_(M);
}

static void Ramp(int D)
{
long n, i, dep=1;
void * M;
struct Num * N;
long long *T;
   _VERIF_STACK_
   if (!isScalar()) {
       messErr(36); return;
   }
   n = (long)getVal();
   dropElt();
   if (n > 1) {
    if (D) { /* double ramp */
        dep = -n;
        n = (2*n) +1;
    }
    if ((M = malloc(sizeof(struct Num)+((n-1)*(sizeof(double))))) == NULL)
             stopErr("Ramp","malloc");
    N = (struct Num*)M;
    N->t = n;
    N->n = StackN;
    _MODIF_STACKN_(M);
    T = &(N->l);
    for(i=0;i<n;i++) T[i]=i+dep;
   } else messErr(30);
}

void IF_ramp(void)
{
    Ramp(0);
}

void IF_dramp(void)
{
    Ramp(1);
}

void IF_stack_clear(void)
{ 
    while (StackN != VIDE) dropElt();
}

#define ELT_POINT -9
static void printElt(struct Num * N, long I)
{
long n, i, m, nt, IB;
long long *l;
double *d;
   IB = I;
   n = N->t&MSK_T;
   if (IB < 0) nt = 3;
   else nt = NBTAB;
   if (n > nt) m=nt-1;
   else m=n-1;
   if (I==ELT_POINT) {
      IB=0;
      n=2;
      m=1;
   }
   if (IB) printf(" ");
   if(N->t & MSK_R) {
         if (n==1) printf("%g (REAL)",N->d);
         else  {
            d = &N->d;
            for(i=0;i<m;i++) printf("%g ",*d++);
            if (I==ELT_POINT) return;
            if (n > nt) printf("... ");
            printf("%g (REAL)[%ld]",*(&N->d+(n-1)),n);
         }
   } else {
         if (n==1) printf("%lld (INTEGER)",N->l);
         else  {
            l = &N->l;
            for(i=0;i<m;i++) printf("%lld ",*l++);
            if (I==ELT_POINT) return;
            if (n > nt) printf("... ");
            printf("%lld (INTEGER)[%ld]",*(&N->l+(n-1)),n);
         }
   }
   if ((IB>0) && (N->t&MSK_V)) printf(" Var. %s",varByAddrA((void*)N));
   if (IB==1) printf(" <- top");
   if (IB) printf("\n");
}

void printNumber(void * E)
{
   printElt((struct Num*)E, 0);
}

void numVarOff(void * A)
{
struct Num * N;
   N = (struct Num*) A;
   N->t = N->t & ~MSK_V;
}

void IF_show_stack(void)
{
void * Next;
struct Num * N;
long i=0,Nbl;
char s;
    Nbl=NBLIG;
    Next = StackN;
    while (Next != VIDE) {
       N = (struct Num*) Next;
       i++;
       if (i<=Nbl) printElt(N,i);
       Next = N->n;
    }
    if (i<=Nbl) printf("<end of stack>\n");
    else {
       if (i==Nbl+1) s = ' ';
       else s = 's';
       printf(" ... and %ld other%c element%c !\n",i-Nbl,s,s);
    }
}

void IF_point(void)
{
struct Num *Elt;
   _VERIF_STACK_
   Elt = (struct Num *)StackN;
   printElt(Elt,ELT_POINT);
   /* printf("\n"); */
   dropElt();
}

void * duplicateNum(void * S, int vSoff)
{
struct Num *Elt, *NElt;
void * M;
uint32_t n;
int s;
    Elt = (struct Num*)S;
    n = Elt->t&MSK_T;
    s = sizeof(struct Num)+((n-1)*(sizeof(double)));
    if ((M = malloc(s)) == NULL) stopErr("dupElt","malloc");
    bcopy((void*)Elt,M,s);
    NElt = (struct Num *)M;
    NElt->n = VIDE;
    NElt->t = Elt->t;
    if (vSoff) Elt->t = Elt->t & ~MSK_V; /* Source no more a Var */
    return(M);
}

static void dupElt(struct Num * Elt)
{
struct Num *NElt;
void * M;
uint32_t n;
int s;
      n = Elt->t&MSK_T;
      s = sizeof(struct Num)+((n-1)*(sizeof(double)));
      if ((M = malloc(s)) == NULL) stopErr("dupElt","malloc");
      bcopy((void*)Elt,M,s);
      NElt = (struct Num *)M;
      NElt->n = StackN;
      NElt->t = Elt->t & ~MSK_V; /* au cas ou Var */
      _MODIF_STACKN_(M);
      ;
}

void IF_dup(void)
{
   _VERIF_STACK_
   dupElt((struct Num *)StackN);
}

void IF_swap(void)
{
struct Num *Elt, *Elt2;
   _VERIF_STACK_
   Elt = (struct Num *)StackN;
   if (Elt->n != VIDE) {
      _MODIF_STACKN_(Elt->n);
      Elt2 = (struct Num *)StackN;
      Elt->n = Elt2->n;
      Elt2->n = (void*)Elt;
   }
   else messErr(4);
}

void IF_over (void)
{
struct Num *Elt;
   _VERIF_STACK_
   Elt = (struct Num *)StackN;
   if (Elt->n != VIDE)
         dupElt((struct Num *)Elt->n);
   else messErr(4);
}

void IF_pick(void)
{
void * Next;
struct Num * N;
long n, i;
  _VERIF_STACK_
  if (!isScalar()) {
       messErr(36); return;
  }
  n = (long)getVal();
  dropElt();
  if (n>0) {
   Next = StackN;
   i=1;
   while (Next != VIDE) {
       if (i==n) break;
       N = (struct Num*) Next;
       Next = N->n;
       i++;
   }
   if (Next != VIDE) dupElt((struct Num *)Next);
   else messErr(4);
  } else messErr(29);
}

static int rotateBid(long n, int d) /* d=0 : rot d=1 : unrot */
{
void **ANext;
struct Num * N, *N1;
long i;
   ANext = _ADDR_STACKN_;
   i=1;
   while (*ANext != VIDE) {
      if (i==n) break;
      N = (struct Num*) *ANext;
      ANext = &N->n;
      i++;
   }
   if (*ANext != VIDE) {
      N = (struct Num*) *ANext;
      if (d) { /* unrot */
         N1 = (struct Num*) StackN;
         _MODIF_STACKN_(N1->n);
         N1->n = N->n;
         N->n = (void*)N1;
      } else { /* rot */
         *ANext = N->n;
         N->n = StackN;
         _MODIF_STACKN_((void*)N);
      }
      return 1;
   } else return 0;
}

void IF_rot(void)
{
   if (!rotateBid(3L,0)) messErr(4);
}

void IF_unrot(void)
{
   if (!rotateBid(3L,1)) messErr(4);
}

void IF_roll(void)
{
long n;
  _VERIF_STACK_
  if (!isScalar()) {
       messErr(36); return;
  }
  n = (long)getVal();
  dropElt();
  if (n>1) {
   if (!rotateBid(n,0)) messErr(4);
  } else messErr(30);
}

void IF_unroll(void)
{
long n;
  _VERIF_STACK_
  if (!isScalar()) {
       messErr(36); return;
  }
  n = (long)getVal();
  dropElt();
  if (n>1) {
   if (!rotateBid(n,1)) messErr(4);
  } else messErr(30);
}

void IF_depth(void)
{
void * Next;
struct Num * N;
long long i=0;
    Next = StackN;
    while (Next != VIDE) {
       N = (struct Num*) Next;
       i++;
       Next = N->n;
    }
    putLong(i);
}

int nbOnStack(void* A)
{
void * Next;
struct Num * N;
int i=0;
    Next = StackN;
    while (Next != VIDE) {
       N = (struct Num*) Next;
       if (Next == A) i++;
       Next = N->n;
    }
    return i;
}

static uint32_t nbSizeTypeOnStack(uint32_t n,uint32_t *T)
{
void * Next;
struct Num * N;
uint32_t i=0, D=0, S=0;
    Next = StackN;
    while (Next != VIDE) {
       N = (struct Num*) Next;
       S += N->t&MSK_T;
       if (N->t&MSK_R) D=1;
       Next = N->n;
       i++;
       if (i==n) break;
    }
    if (D) S = S|MSK_R;
    *T = S;
    return i;
}

void IF_Ndrop(void)
{
long n, i;
   _VERIF_STACK_
   if (!isScalar()) {
       messErr(36); return;
   }
   n = (long)getVal();
   dropElt();
   for(i=0;i<n;i++) if (!dropElt()) break;
   if (i<n) messErr(4); /* erreur pour la rigueur ! */
}

void IF_Ndup(void)
{
void *Next, *M, *NewS, **P;
struct Num * Elt, * NElt;
uint32_t n, i=0, t;
int s;
   _VERIF_STACK_
   if (!isScalar()) {
       messErr(36); return;
   }
   n = (long)getVal();
   dropElt();
   if (n>0) {
    NewS=VIDE;
    P=&NewS;
    Next = StackN;
    while (Next != VIDE) {
       Elt = (struct Num*) Next;
       t = Elt->t&MSK_T;
       s = sizeof(struct Num)+((t-1)*(sizeof(double)));
       if ((M = malloc(s)) == NULL) stopErr("IF_Ndup","malloc");
       bcopy(Next,M,s);
       *P = M;
       NElt = (struct Num *)M;
       NElt->t = NElt->t & ~MSK_V; /* au cas ou Var */
       P=&NElt->n;
       i++;
       if (i==n) break;
       Next = Elt->n;
    }
    NElt->n = StackN;
    _MODIF_STACKN_(NewS);
    if (i<n) messErr(4); /* erreur pour la rigueur ! */
   } else messErr(29);
}

static void toDouble(struct Num * N)
{
uint32_t n;
int i;
double *d;
long long  *l;
    if (N->t & MSK_R) return;
    n = N->t&MSK_T;
    d = &N->d;
    l = &N->l;
    for(i=0; i<n; i++) *d++ = (double)*l++;
    N->t = N->t|MSK_R;
}

/* NET Functions for STSP */

void IF_NetKey (void)
{
  _VERIF_STACK_
  if (!isScalar()) {
       messErr(36); return;
  }
  _MODIF_NetKey_((uint32_t)getVal());
  dropElt();
}

void IF_NetErrVal (void)
{
   putLong(-(long long)NetKey);
}

void StackToNet(long n)
{
struct Num *Elt;
int i;
uint32_t t, l;
   for (i=0; i<n ; i++) {
      if(StackN == VIDE) break;
      Elt = (struct Num *)StackN;
      t = Elt->t&MSK_T;
      l=(sizeof(struct Num)+((t-1)*(sizeof(double))));
      Elt->key = NetKey;
      sendDataC(StackN, l);
      dropElt();
   }
}

int NetDepth(uint32_t k)
{
void * Next;
struct Num * N;
int v=0;
    Next = StackN;
    while (Next != VIDE) {
       N = (struct Num*) Next;
       if (N->key == k) v++;
       Next = N->n;
    }
    return v;
}

void NetToStack(int s, uint32_t k)
{
void * Next;
struct Num * N;
uint32_t t, l;
    Next = StackN;
    while (Next != VIDE) {
       N = (struct Num*) Next;
       if (N->key == k) {
          t = N->t&MSK_T;
          l=(sizeof(struct Num)+((t-1)*(sizeof(double))));
          sendData(s, Next, l);
       }
       Next = N->n;
    }
}

void IF_show_netStack(uint32_t k)
{
void * Next;
struct Num * N;
    Next = StackN;
    printf("<bottom of net stack>\n");
    while (Next != VIDE) {
       N = (struct Num*) Next;
       if (k == UNI_KEY) {
          printf("<0x%.8lx> ",(long)N->key);
          printElt(N,(long)-1);
       } else {
          if (N->key == k) printElt(N,(long)-1);
       }
       Next = N->n;
    }
    printf("<top of net stack> key=0x%lx\n",(long)k);
}

void IF_netDrop(uint32_t k)
{
void * Next, **ANext;
struct Num * N;
    Next = StackN;
    ANext = _ADDR_STACKN_;
    while (Next != VIDE) {
       N = (struct Num*) Next;
       if (k == N->key) {
           *ANext = N->n;
           free(Next);
           Next = *ANext;
           continue;
       }
       Next = N->n;
       ANext = &(N->n);
    }
}

/* end of Net functions */

static void IF_fct_2(char O)
{
struct Num *Elt, *Elt2;
long long ConstL, *l1, *l2;
double ConstD, *d1, *d2;
int D1=0, D2=0, T1, T2, i;
int M_C, M_D, M_S; /* Mode Const : 1 ou 0 | Double : 1 ou 0 | Swap : 1 ou 0 */
bool B=TRUE;

    _VERIF_STACK_
    Elt = (struct Num *)StackN;
    T1 = Elt->t&MSK_T;
    D1 = Elt->t&MSK_R;
    if (Elt->n == VIDE) {
       messErr(4);
       return;
    }
    Elt2 = (struct Num *)Elt->n;
    T2 = Elt2->t&MSK_T;
    D2 = Elt2->t&MSK_R;
    /* si 2 tab de dim diff pas possible !! */
    if ((T1>1) && (T2>1) && (T1!=T2)) {
       messErr(3);
       return;
    }
    M_S = M_C = M_D = 0;
    if ((T1>1) && (T2==1)) {  /* on swap */
       IF_swap();
       D1=D2=0;
       Elt = (struct Num *)StackN;
       T1 = Elt->t&MSK_T;
       D1 = Elt->t&MSK_R;
       Elt2 = (struct Num *)Elt->n;
       T2 = Elt2->t&MSK_T;
       D2 = Elt2->t&MSK_R;
       M_S=1;
    }
    if (D1!=D2) { /* on transforme long en double */
        if (D2) toDouble(Elt); else toDouble(Elt2);
        M_D = 1;
    } else if(D1) M_D = 1;
    l1 = &Elt->l;
    l2 = &Elt2->l;
    d1 = &Elt->d;
    d2 = &Elt2->d;
    if (T1==1) {
        M_C=1;
        if (M_D) ConstD = *d1;
        else ConstL = *l1;
    }
/* pour debug
    printf("T1=%d T2=%d M_C=%d M_D=%d M_S=%d ",T1,T2,M_C,M_D,M_S);
    if (M_C)
       if (M_D) printf("ConstD=%g",ConstD); else printf("ConstL=%lld",ConstL);
    printf("\n");
*****/
    switch(O) {
       case '+' :
           if (M_C) {
              if (M_D) for (i=0;i<T2;i++) *d2++ += ConstD;
              else  for (i=0;i<T2;i++) *l2++ += ConstL;
           } else {
              if (M_D) for (i=0;i<T2;i++) *d2++ += *d1++;
              else  for (i=0;i<T2;i++) *l2++ += *l1++;
           }
           break;
       case '-' :
           if (M_C) {
              if (M_S) {
                 if (M_D) for (i=0;i<T2;i++) *d2++ = ConstD - *d2;
                 else  for (i=0;i<T2;i++) *l2++ = ConstL - *l2;
              } else {
                 if (M_D) for (i=0;i<T2;i++) *d2++ -= ConstD;
                 else  for (i=0;i<T2;i++) *l2++ -= ConstL;
              }
           } else {
              if (M_D) for (i=0;i<T2;i++) *d2++ -= *d1++;
              else  for (i=0;i<T2;i++) *l2++ -= *l1++;
           }
           break;
       case '*' :
           if (M_C) {
              if (M_D) for (i=0;i<T2;i++) *d2++ *= ConstD;
              else  for (i=0;i<T2;i++) *l2++ *= ConstL;
           } else {
              if (M_D) for (i=0;i<T2;i++) *d2++ *= *d1++;
              else  for (i=0;i<T2;i++) *l2++ *= *l1++;
           }
           break;
       case '/' :
           if (M_C) {
              if (M_S) {
                 if (M_D) for (i=0;i<T2;i++) *d2++ = ConstD / *d2;
                 else  for (i=0;i<T2;i++) *l2++ = ConstL / *l2;
              } else {
                 if (M_D) for (i=0;i<T2;i++) *d2++ /= ConstD;
                 else  for (i=0;i<T2;i++) *l2++ /= ConstL;
              }
           } else {
              if (M_D) for (i=0;i<T2;i++) *d2++ /= *d1++;
              else  for (i=0;i<T2;i++) *l2++ /= *l1++;
           }
           break;
       case 'm' : /* min */
         if (M_C) {
           if (M_D) for(i=0;i<T2;i++) { if (*d2>ConstD) *d2=ConstD; d2++; }
           else for(i=0;i<T2;i++) { if (*l2>ConstL) *l2=ConstL; l2++; }
         } else {
           if (M_D) for (i=0;i<T2;i++) { if (*d2>*d1) *d2=*d1; d2++; d1++; }
           else  for (i=0;i<T2;i++) { if (*l2>*l1) *l2=*l1; l2++; l1++; }
         }
         break;
       case 'M' : /* max */
         if (M_C) {
           if (M_D) for(i=0;i<T2;i++) { if (*d2<ConstD) *d2=ConstD; d2++; }
           else for(i=0;i<T2;i++) { if (*l2<ConstL) *l2=ConstL; l2++; }
         } else {
           if (M_D) for (i=0;i<T2;i++) { if (*d2<*d1) *d2=*d1; d2++; d1++; }
           else  for (i=0;i<T2;i++) { if (*l2<*l1) *l2=*l1; l2++; l1++; }
         }
         break;
       case '%' : /* modulo */
         if (M_C) {
          if (M_S) {
            if (M_D) for (i=0;i<T2;i++) *d2++ = fmod(ConstD,*d2);
            else  for (i=0;i<T2;i++) *l2++ = ConstL % *l2;
          } else {
            if (M_D) for (i=0;i<T2;i++) *d2++ = fmod(*d2,ConstD);
            else  for (i=0;i<T2;i++) *l2++ %= ConstL;
          }
         } else {
          if (M_D) for (i=0;i<T2;i++) { *d2++ = fmod(*d2,*d1); d1++; }
          else  for (i=0;i<T2;i++) *l2++ %= *l1++;
         }
         break;
       case '^' : /* puissance */
         if (! M_D) { /* passage en double force car + pratique */
            toDouble(Elt); toDouble(Elt2);
            if (M_C) ConstD=*d1;
         }
         if (M_C) {
          if (M_S) {
            for (i=0;i<T2;i++) *d2++ = pow(ConstD,*d2);
          } else {
            for (i=0;i<T2;i++) *d2++ = pow(*d2,ConstD);
          }
         } else {
           for (i=0;i<T2;i++) { *d2++ = pow(*d2,*d1); d1++; }
         }
         break;
       case '=' :  /* test egal */
         if (M_C) {
            if (M_D) for(i=0;i<T2;i++) {
                 if (*d2 != ConstD) {B=FALSE; break;} d2++; }
            else  for (i=0;i<T2;i++)  {
                 if (*l2 != ConstL) {B=FALSE; break;} l2++; }
         } else {
            if (M_D) for (i=0;i<T2;i++) {
                 if (*d2 != *d1) {B=FALSE; break;} d1++; d2++; }
            else  for (i=0;i<T2;i++) {
                 if (*l2 != *l1) {B=FALSE; break;} l1++; l2++; }
         }
         putBool(B);
         dropElt(); /* suppression des 2 !! */
         break;
       case '#' :  /* test different */
         if (M_C) {
            if (M_D) for(i=0;i<T2;i++) {
                 if (*d2 == ConstD) {B=FALSE; break;} d2++; }
            else  for (i=0;i<T2;i++)  {
                 if (*l2 == ConstL) {B=FALSE; break;} l2++; }
         } else {
            if (M_D) for (i=0;i<T2;i++) {
                 if (*d2 == *d1) {B=FALSE; break;} d1++; d2++; }
            else  for (i=0;i<T2;i++) {
                 if (*l2 == *l1) {B=FALSE; break;} l1++; l2++; }
         }
         putBool(B);
         dropElt(); /* suppression des 2 !! */
         break;
       case '<' :  /* test inf */
         if (M_C) {
           if (M_S) {
            if (M_D) for(i=0;i<T2;i++) {
                 if (*d2 < ConstD) {B=FALSE; break;} d2++; }
            else  for (i=0;i<T2;i++)  {
                 if (*l2 < ConstL) {B=FALSE; break;} l2++; }
           } else {
            if (M_D) for(i=0;i<T2;i++) {
                 if (*d2 >= ConstD) {B=FALSE; break;} d2++; }
            else  for (i=0;i<T2;i++)  {
                 if (*l2 >= ConstL) {B=FALSE; break;} l2++; }
           }
         } else {
            if (M_D) for (i=0;i<T2;i++) {
                 if (*d2 >= *d1) {B=FALSE; break;} d1++; d2++; }
            else  for (i=0;i<T2;i++) {
                 if (*l2 >= *l1) {B=FALSE; break;} l1++; l2++; }
         }
         putBool(B);
         dropElt(); /* suppression des 2 !! */
         break;
       case '>' :  /* test sup */
         if (M_C) {
           if (M_S) {
            if (M_D) for(i=0;i<T2;i++) {
                 if (*d2 > ConstD) {B=FALSE; break;} d2++; }
            else  for (i=0;i<T2;i++)  {
                 if (*l2 > ConstL) {B=FALSE; break;} l2++; }
           } else {
            if (M_D) for(i=0;i<T2;i++) {
                 if (*d2 <= ConstD) {B=FALSE; break;} d2++; }
            else  for (i=0;i<T2;i++)  {
                 if (*l2 <= ConstL) {B=FALSE; break;} l2++; }
           }
         } else {
            if (M_D) for (i=0;i<T2;i++) {
                 if (*d2 <= *d1) {B=FALSE; break;} d1++; d2++; }
            else  for (i=0;i<T2;i++) {
                 if (*l2 <= *l1) {B=FALSE; break;} l1++; l2++; }
         }
         putBool(B);
         dropElt(); /* suppression des 2 !! */
         break;
       case 'i' :  /* test inf ou egal */
         if (M_C) {
           if (M_S) {
            if (M_D) for(i=0;i<T2;i++) {
                 if (*d2 <= ConstD) {B=FALSE; break;} d2++; }
            else  for (i=0;i<T2;i++)  {
                 if (*l2 <= ConstL) {B=FALSE; break;} l2++; }
           } else {
            if (M_D) for(i=0;i<T2;i++) {
                 if (*d2 > ConstD) {B=FALSE; break;} d2++; }
            else  for (i=0;i<T2;i++)  {
                 if (*l2 > ConstL) {B=FALSE; break;} l2++; }
           }
         } else {
            if (M_D) for (i=0;i<T2;i++) {
                 if (*d2 > *d1) {B=FALSE; break;} d1++; d2++; }
            else  for (i=0;i<T2;i++) {
                 if (*l2 > *l1) {B=FALSE; break;} l1++; l2++; }
         }
         putBool(B);
         dropElt(); /* suppression des 2 !! */
         break;
       case 's' :  /* test sup ou egal */
         if (M_C) {
           if (M_S) {
            if (M_D) for(i=0;i<T2;i++) {
                 if (*d2 >= ConstD) {B=FALSE; break;} d2++; }
            else  for (i=0;i<T2;i++)  {
                 if (*l2 >= ConstL) {B=FALSE; break;} l2++; }
           } else {
            if (M_D) for(i=0;i<T2;i++) {
                 if (*d2 < ConstD) {B=FALSE; break;} d2++; }
            else  for (i=0;i<T2;i++)  {
                 if (*l2 < ConstL) {B=FALSE; break;} l2++; }
           }
         } else {
            if (M_D) for (i=0;i<T2;i++) {
                 if (*d2 < *d1) {B=FALSE; break;} d1++; d2++; }
            else  for (i=0;i<T2;i++) {
                 if (*l2 < *l1) {B=FALSE; break;} l1++; l2++; }
         }
         putBool(B);
         dropElt(); /* suppression des 2 !! */
         break;
       default :
         printf("%c : not yet implemented !\n",O);
       break;
    }
    dropElt(); /* suppression du 1er */
}

void IF_plus(void) { IF_fct_2('+'); }
void IF_moins(void) { IF_fct_2('-'); }
void IF_mult(void) { IF_fct_2('*'); }
void IF_div(void) { IF_fct_2('/'); }
void IF_min(void) { IF_fct_2('m'); }
void IF_max(void) { IF_fct_2('M'); }
void IF_modulo(void) { IF_fct_2('%'); }
void IF_puiss(void) { IF_fct_2('^'); }
void IF_neg(void)
{
    putLong((long long)-1);
    IF_mult();
}

void IF_Legal(void) { IF_fct_2('='); }
void IF_Ldiff(void) { IF_fct_2('#'); }
void IF_Lsup(void) { IF_fct_2('>'); }
void IF_Linf(void) { IF_fct_2('<'); }
void IF_Lsupeg(void) { IF_fct_2('s'); }
void IF_Linfeg(void) { IF_fct_2('i'); }

void IF_fctD_1(double(*f)(double))
{
struct Num *Elt;
uint32_t n;
long long *L;
double *D;
int i;
   _VERIF_STACK_
   Elt = (struct Num *)StackN;
   n = Elt->t&MSK_T;
   if (Elt->t&MSK_R) { /* double */
      D = &(Elt->d);
      for(i=0;i<n;i++) *D++ = f(*D);
   } else {
      D = &(Elt->d);
      L = &(Elt->l);
      for(i=0;i<n;i++) *D++ = f((double)*L++);
      Elt->t = Elt->t | MSK_R;
   }
}

void IF_fctB_1(long long (*f1)(long long), double(*f2)(double))
{
struct Num *Elt;
uint32_t n;
long long *L;
double *D;
int i;
   _VERIF_STACK_
   Elt = (struct Num *)StackN;
   n = Elt->t&MSK_T;
   if (Elt->t&MSK_R) { /* double */
      D = &(Elt->d);
      for(i=0;i<n;i++) *D++ = f2(*D);
   } else {
      L = &(Elt->l);
      for(i=0;i<n;i++) *L++ = f1(*L);
   }
}

void IF_fctD_1L(long long(*f)(double))
{
struct Num *Elt;
uint32_t n;
long long *L;
double *D;
int i;
   _VERIF_STACK_
   Elt = (struct Num *)StackN;
   n = Elt->t&MSK_T;
   if (Elt->t&MSK_R) { /* double */
      D = &(Elt->d);
      L = &(Elt->l);
      for(i=0;i<n;i++) *L++ = f(*D++);
      Elt->t = Elt->t & ~MSK_R; /* change type */
   }
   /* rien si long */
}

void IF_fctD_1LB(long long(*f)(double))
{
struct Num *Elt;
uint32_t n;
long long *L;
double *D;
int i;
   _VERIF_STACK_
   Elt = (struct Num *)StackN;
   n = Elt->t&MSK_T;
   D = &(Elt->d);
   L = &(Elt->l);
   if (Elt->t&MSK_R) { /* double */
      for(i=0;i<n;i++) *L++ = f(*D++);
      Elt->t = Elt->t & ~MSK_R; /* change type */
   } else {
      for(i=0;i<n;i++) *L++ = f(*L);
   }
}

void IF_inFile_1d(FILE * fd, char delim, int virg)
{
struct Num *Elt;
uint32_t n;
long long *L;
double *D;
int i;
char buf[40], *pt;
   _VERIF_STACK_
   Elt = (struct Num *)StackN;
   n = Elt->t&MSK_T;
   if (Elt->t&MSK_R) { /* double */
      D = &(Elt->d);
      for(i=0;i<n;i++) {
         sprintf(buf,"%.15f%c",*D++,delim);
         pt = buf + strlen(buf) - 2;
         while (*pt == '0') pt--;
         pt++;
         *pt++ = delim;
         *pt='\0';
         if (virg)
            if ((pt = strchr(buf, (int)'.')) != NULL) *pt = ',';
         fprintf(fd,buf);
      }
   } else {
      L = &(Elt->l);
      for(i=0;i<n;i++) fprintf(fd,"%lld%c",*L++,delim);
   }
   fprintf(fd,"\n");
   dropElt(); /* suppression */
}

void IF_inFile_1(FILE * fd)
{
struct Num *Elt;
uint32_t n;
long long *L;
double *D;
int i;
   _VERIF_STACK_
   Elt = (struct Num *)StackN;
   n = Elt->t&MSK_T;
   if (Elt->t&MSK_R) { /* double */
      D = &(Elt->d);
      for(i=0;i<n;i++) fprintf(fd,"%d %g\n",i,*D++);
   } else {
      L = &(Elt->l);
      for(i=0;i<n;i++) fprintf(fd,"%d %lld\n",i,*L++);
   }
}

void IF_inFile_2(FILE * fd)
{
struct Num *Elt, *Elt2;
uint32_t n, n2;
long long *L, *L2;
double *D, *D2;
int i;
   _VERIF_STACK_
   Elt = (struct Num *)StackN;
   n = Elt->t&MSK_T;
   Elt2 = Elt->n;
   n2 = Elt2->t&MSK_T;
   if (n>n2) n=n2;
   if (Elt->t&MSK_R) { /* double */
      D = &(Elt->d);
      if (Elt2->t&MSK_R) { /* double */
         D2 = &(Elt2->d);
         for(i=0;i<n;i++) fprintf(fd,"%g %g\n",*D2++,*D++);
      } else {
         L2 = &(Elt2->l);
         for(i=0;i<n;i++) fprintf(fd,"%lld %g\n",*L2++,*D++);
      }
   } else {
      L = &(Elt->l);
      if (Elt2->t&MSK_R) { /* double */
         D2 = &(Elt2->d);
         for(i=0;i<n;i++) fprintf(fd,"%g %lld\n",*D2++,*L++);
      } else {
         L2 = &(Elt2->l);
         for(i=0;i<n;i++) fprintf(fd,"%lld %lld\n",*L2++,*L++);
      }
   }
}

static int dimTab(void * A)
{
struct Num *N;
uint32_t l;
    if (A==VIDE) return 0L;
    N = (struct Num*)A;
    l = N->t&MSK_T;
    return (int)l;
}

static long transLongTo(long long * C)
{
struct Num *Elt;
int i;
uint32_t l;
long long *L;
double *D;
   Elt = (struct Num *)StackN;
   l = Elt->t&MSK_T;
   L = &(Elt->l)+l-1;
   D = &(Elt->d)+l-1;
   if (Elt->t & MSK_R)
      for (i=0;i<l;i++) *C--=(long long)*D--;
   else
      for (i=0;i<l;i++) *C--=*L--;
   dropElt();
   return(l);
}

static long transDoubTo(double * C)
{
struct Num *Elt;
int i;
uint32_t l;
long long *L;
double *D;
   Elt = (struct Num *)StackN;
   l = Elt->t&MSK_T;
   L = &(Elt->l)+l-1;
   D = &(Elt->d)+l-1;
   if (Elt->t & MSK_R)
      for (i=0;i<l;i++) *C--=*D--;
   else
      for (i=0;i<l;i++) *C--=(double)*L--;
   dropElt();
   return(l);
}

void IF_toArray( void )
{
uint32_t n, i, l, T, t;
long long  *L;
double *D;
void * M;
struct Num *N;
  _VERIF_STACK_
  if (!isScalar()) {
       messErr(36); return;
  }
  n = (uint32_t)getVal();
  dropElt();
  if (n>1) {
   i = nbSizeTypeOnStack(n,&t);
   if (i<n) messErr(4);
   else {
      T = t & MSK_T;      
      if ((M = malloc(sizeof(struct Num)+((T-1)*(sizeof(double))))) == NULL)
             stopErr("IF_toArray","malloc");
      N = (struct Num*)M;
      N->t = t;
      if (t & MSK_R) {
         D = &(N->d)+(T-1);
         for(i=0;i<n;i++) { l=transDoubTo(D); D -= l; }
      } else {
         L = &(N->l)+(T-1);
         for(i=0;i<n;i++) { l=transLongTo(L); L -= l; }
      }
      /* on the stack */
      N->n = StackN;
      _MODIF_STACKN_(M);
   }
  } else messErr(30);
}

static void toScalar( int s )
{
struct Num *Elt;
uint32_t l;
int i;
double *D;
long long * L;
   _VERIF_STACK_
   Elt = (struct Num *)StackN;
   l = Elt->t&MSK_T;
   if (l==1) return;
   _MODIF_STACKN_(Elt->n); /* depile */
   if (s) {
      L = &(Elt->l);
      D = &(Elt->d);
      if (Elt->t & MSK_R)
         for(i=0;i<l;i++) putDouble(*D++);
      else 
         for(i=0;i<l;i++) putLong(*L++);
   } else {
      L = &(Elt->l)+l-1;
      D = &(Elt->d)+l-1;
      if (Elt->t & MSK_R)
         for(i=0;i<l;i++) putDouble(*D--);
      else 
         for(i=0;i<l;i++) putLong(*L--);
   }
   if (!(Elt->t&MSK_V)) free((void*)Elt);
}

void IF_toScalar( void )
{
    toScalar(1);
}

void IF_toScalarR( void )
{
    toScalar(0);
}

static void tabShift(void **A,long s)
{
struct Num *Elt, *NElt;
void * M;
long j, k;
uint32_t l;
long long *L, *NL;
   _VERIF_STACK_
   Elt = (struct Num *)*A;
   l = Elt->t&MSK_T;
   if (l==1) return;
   if (s>0)
      while (s>=l) s-=l;
   else  while (-s>=l) s+=l;
   if (s==0) return;
   if (s>0) j=s;
   else j=l+s;
   k = sizeof(struct Num)+((l-1)*(sizeof(double)));
   if ((M = malloc(k)) == NULL) stopErr("tabShift","malloc");
   NElt = (struct Num *)M;
   *A = M;
   NElt->t = Elt->t;
   NElt->n = Elt->n;
   L = &(Elt->l);
   NL = &(NElt->l);
   k=l-j;
   bcopy((void*)&L[0],(void*)&NL[j],k*sizeof(long long));
   bcopy((void*)&L[k],(void*)&NL[0],j*sizeof(long long));
   if (!(Elt->t&MSK_V)) free((void*)Elt);
}

void IF_TShiftR( void )
{
   tabShift(_ADDR_STACKN_,1);
}

void IF_TShiftL( void )
{
   tabShift(_ADDR_STACKN_,-1);
}

static void nTabShift( int v )
{
void ** ANext;
struct Num * N;
long i=0, n;
   _VERIF_STACK_
   if (!isScalar()) {
       messErr(36); return;
   }
   n = (long)getVal();
   dropElt();
   if (n>0) {
    ANext = _ADDR_STACKN_;
    while (*ANext != VIDE) {
       tabShift(ANext,v);
       N = (struct Num*) *ANext;
       ANext = &(N->n);
       i++;
       if (i==n) break;
    }
   } else messErr(29);
}

void IF_NTShiftR( void )
{
   nTabShift(1);
}

void IF_NTShiftL( void )
{
   nTabShift(-1);
}

void IF_TNShiftR( void )
{
long n;
   _VERIF_STACK_
   if (!isScalar()) {
       messErr(36); return;
   }
   n = (long)getVal();
   dropElt();
   tabShift(_ADDR_STACKN_,n);
}

void IF_TNShiftL( void )
{
long n;
   _VERIF_STACK_
   if (!isScalar()) {
       messErr(36); return;
   }
   n = (long)getVal();
   dropElt();
   tabShift(_ADDR_STACKN_,-n);
}

static void nTNShift( int s )
{
long n;
   _VERIF_STACK_
   if (!isScalar()) {
       messErr(36); return;
   }
   n = (long)getVal();
   dropElt();
   nTabShift(n*s);
}

void IF_NTNShiftR( void )
{
    nTNShift(1);
}

void IF_NTNShiftL( void )
{
    nTNShift(-1);
}

static void subTab(void **pA , long n, char r) /* r=1 : right else left */
{
struct Num *Elt, *Elt2;
uint32_t l;
long k;
long long *L, *Lf;
void *A, * M;
double *D, *Df;

   _VERIF_STACK_
   A = *pA;
   Elt = (struct Num *)A;
   l = Elt->t&MSK_T;
   if (l==n) return;
   k = sizeof(struct Num)+((n-1)*(sizeof(long long)));
   if ((M = malloc(k)) == NULL) stopErr("subTab","malloc");
   Elt2 = (struct Num *)M;
   Elt2->n = Elt->n;
   Elt2->t = (Elt->t&MSK_R) | n;
   *pA = M;
   if (n<l) {
      L = &(Elt->l);
      if (r) L += (l-n);
      bcopy((void*)L,(void*)&(Elt2->l),n*sizeof(long long));
   } else { /* fill with zero */
      L = &(Elt2->l);
      if (!r) L += (n-l);
      bcopy((void*)&(Elt->l),(void*)L,l*sizeof(long long));
      if (Elt->t&MSK_R) {
         D = &(Elt2->d);
         if (r) D += l;
         Df = D + (n-l);
         while (D < Df) *D++ = (double)0;
      } else {
         L = &(Elt2->l);
         if (r) L += l;
         Lf = L + (n-l);
         while (L < Lf) *L++ = 0L;
      }
   }
   if (!(Elt->t&MSK_V)) free(A);
}

void IF_subTab(void)
{
long n;
   _VERIF_STACK_
   if (!isScalar()) {
       messErr(36); return;
   }
   n = (long)getVal();
   dropElt();
   if (n>0) subTab(_ADDR_STACKN_, n, 0);
   else messErr(29);
}

void IF_subTabR(void)
{
long n;
   _VERIF_STACK_
   if (!isScalar()) {
       messErr(36); return;
   }
   n = (long)getVal();
   dropElt();
   if (n>0) subTab(_ADDR_STACKN_, n, (char)1);
   else messErr(29);
}

static void NSubTab( char r )
{
void **pNext;
struct Num * N;
long i=0, n, l;
   _VERIF_STACK_
   if (!isScalar()) {
       messErr(36); return;
   }
   l = (long)getVal();
   dropElt();
   if (l>0) {
    if (!isScalar()) {
       messErr(36); return;
    }
    n = (long)getVal();
    dropElt();
    if (n>0) {
     pNext = _ADDR_STACKN_;
     while (*pNext != VIDE) {
       subTab(pNext,l,r);
       N = (struct Num*) *pNext;
       pNext = &(N->n);
       i++;
       if (i==n) break;
     }
    } else messErr(29);
   } else messErr(29);
}

void IF_NsubTab(void)
{
    NSubTab((char)0);
}

void IF_NsubTabR(void)
{
    NSubTab((char)1);
}

static void tabRev( void* A )
{
struct Num *Elt;
uint32_t l;
double *D, *FD, vD;
long long * L, *FL, vL;
   _VERIF_STACK_
   Elt = (struct Num *)A;
   l = Elt->t&MSK_T;
   if (l==1) return;
   if (Elt->t & MSK_R) {
      D = &(Elt->d);
      FD = D+l-1;
      while (D<FD) {
        vD=*D; *D=*FD; *FD=vD;
        D++; FD--;
      }
   } else  {
      L = &(Elt->l);
      FL=L+l-1;
      while (L<FL) {
        vL=*L; *L=*FL; *FL=vL;
        L++; FL--;
      }
   }
}

void IF_TabRev( void )
{
    tabRev(StackN);
}

void IF_NTabRev( void )
{
void * Next;
struct Num * N;
long i=0, n;
   _VERIF_STACK_
   if (!isScalar()) {
       messErr(36); return;
   }
   n = (long)getVal();
   dropElt();
   if (n>0) {
    Next = StackN;
    while (Next != VIDE) {
       N = (struct Num*) Next;
       tabRev(Next);
       Next = N->n;
       i++;
       if (i==n) break;
    }
   } else messErr(29);
}

static void tabTransp (int sens)
{
void * Next, *Next2, **Suiv, *SNext, **Last;
struct Num * N, *N2;
long i=0, j, n;
uint32_t l;
short Doub=0;
double *D, *D2;
long long *L, *L2;
  _VERIF_STACK_
  if (!isScalar()) {
       messErr(36); return;
  }
  n = (long)getVal();
  dropElt();
  if (n>0) {
   if (n==1) {
      if (sens) toScalar(1);
      else toScalar(0);
      return;
   }
   /* the n elts on stack must have the same dim */
   Next = StackN;
   while (Next != VIDE) {
      N = (struct Num*) Next;
      if (i) {
         if (l != (N->t&MSK_T)) break;
      } else l = N->t&MSK_T;
      if (N->t&MSK_R) Doub=1;
      i++;
      if (i==n) break;
      Next = N->n;
   }
   if (i!=n) {
     if (Next == VIDE) messErr(4);
     else messErr(3);
   } else {
     /* make l elts of dim n */
     Suiv = &Next2;
     for (i=0;i<l;i++) {
        if ((*Suiv=malloc(sizeof(struct Num)+((n-1)*(sizeof(double)))))==NULL)
           stopErr("tabTransp","malloc");
        N2 = (struct Num*) *Suiv;
        N2->t = n;
        if (Doub) N2->t |= MSK_R;
        /* remplissage */
        if (sens) {
           j=0;
           if (sens==1) {
              N2->n = SNext;
              SNext = Next2;
              if (i==0) Last = &(N2->n);
           } else
              Suiv = &N2->n;
        } else {
           j=n-1;
           Suiv = &N2->n;
        }
        if (Doub) {
           D2 = &(N2->d);
        } else {
           L2 = &(N2->l);
        }
        Next = StackN;
        while (Next != VIDE) {
           N = (struct Num*) Next;
           if (Doub) {
              if (N->t&MSK_R) {
                 D = &(N->d); 
                 if (sens) D+=(l-i-1);
                 else D+=i;
                 *(D2+j) = *D;
              } else {
                 L = &(N->l);
                 if (sens) L+=(l-i-1);
                 else L+=i;
                 *(D2+j) = (double)*L;
              }
           } else {
              L = &(N->l);
              if (sens) L+=(l-i-1);
              else L+=i;
              *(L2+j) = *L;
              /* printf("%ld ",*L); */
           }
           if (sens) {
              j++;
              if (j>=n) break;
           } else {
              j--;
              if (j<0) break;
           }
           Next = N->n;
        }
        /* printf("\n"); */
     }
     if (sens!=1) Last = &(N2->n);
     /* drop n elts */
     for (i=0;i<n;i++) dropElt();
     /* put new elts on the stack */
     *Last = StackN;
     _MODIF_STACKN_(Next2);
   }
  } else messErr(29);
}

void IF_TabTransp (void) /* crot */
{
   tabTransp(0);
}

void IF_TabTranspN (void) /* transpose */
{
   tabTransp(1);
}

void IF_TabTranspT (void) /* trot */
{
   tabTransp(-1);
}

int is1Tab(void)
{
    if (dimTab(StackN) < 2L) return 0;
    return 1;
}

int is2Tab(void)
{
struct Num *N;
    if (dimTab(StackN) < 2L) return 0;
    N = (struct Num*)StackN;
    if (dimTab(N->n) < 2L) return 0;
    return 1;
}

int isNTabSameDim(int n)
{
void * Next;
struct Num * N;
long i=0;
uint32_t l;
   /* the n elts on stack must have the same dim */
   Next = StackN;
   while (Next != VIDE) {
      N = (struct Num*) Next;
      if (i) {
         if (l != (long)(N->t&MSK_T)) break;
      } else l = N->t&MSK_T;
      i++;
      if (i==n) return 1; /* OK */
      Next = N->n;
   }
   return 0;
}

/* []functions ****/

static void TAB_Fct(char C)
{
struct Num *Elt;
uint32_t n;
long long *L, L1, L2;
double *D, D1, D2;
int i;
  _VERIF_STACK_
  Elt = (struct Num *)StackN;
  n = Elt->t&MSK_T;
  if (n>1) {
    if (Elt->t&MSK_R) { /* double */
      switch(C) {
        case 'm' :
          D1 = Elt->d;
          D = &(Elt->d)+1;
          for(i=1;i<n;i++) { if (*D<D1) D1=*D; D++; }
          putDouble(D1);
          break;
        case 'M' :
          D1 = Elt->d;
          D = &(Elt->d)+1;
          for(i=1;i<n;i++) { if (*D>D1) D1=*D; D++; }
          putDouble(D1);
          break;
        case '+' :
          D1 = Elt->d;
          D = &(Elt->d)+1;
          for(i=1;i<n;i++)  D1 += *D++;
          putDouble(D1);
          break;
        case '*' :
          D1 = Elt->d;
          D = &(Elt->d)+1;
          for(i=1;i<n;i++)  D1 *= *D++;
          putDouble(D1);
          break;
        default :  /* E = extrems */
          D1 = D2 = Elt->d;
          D = &(Elt->d)+1;
          for(i=1;i<n;i++) {
              if (*D>D2) D2=*D;
              if (*D<D1) D1=*D; D++;
          }
          putDouble(D1);
          IF_swap();
          putDouble(D2);
          break;
      }
    } else {
      switch(C) {
        case 'm' :
          L1 = Elt->l;
          L = &(Elt->l)+1;
          for(i=1;i<n;i++) { if (*L<L1) L1=*L; L++; }
          putLong(L1);
          break;
        case 'M' :
          L1 = Elt->l;
          L = &(Elt->l)+1;
          for(i=1;i<n;i++) { if (*L>L1) L1=*L; L++; }
          putLong(L1);
          break;
        case '+' :
          L1 = Elt->l;
          L = &(Elt->l)+1;
          for(i=1;i<n;i++)  L1 += *L++;
          putLong(L1);
          break;
        case '*' :
          L1 = Elt->l;
          L = &(Elt->l)+1;
          for(i=1;i<n;i++)  L1 *= *L++;
          putLong(L1);
          break;
        default :  /* E = extrems */
          L1 = L2 = Elt->l;
          L = &(Elt->l)+1;
          for(i=1;i<n;i++) {
              if (*L>L2) L2=*L;
              if (*L<L1) L1=*L; L++;
          }
          putLong(L1);
          IF_swap();
          putLong(L2);
          break;
      }
    }
    IF_swap();
    dropElt();
  } else messErr(12);
}

void IF_TABMin(void) { TAB_Fct('m'); }
void IF_TABMax(void) { TAB_Fct('M'); }
void IF_TABProd(void) { TAB_Fct('*'); }
void IF_TABSum(void) { TAB_Fct('+'); }
void IF_TABMinMax(void) { TAB_Fct('E'); }


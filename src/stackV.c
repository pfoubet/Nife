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
#include "conf.h"
/* stackV.c */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>

#include "nife.h"
#include "mth.h"
#include "err.h"
#include "lib.h"
#include "stackV.h"
#include "stackF.h"
#include "stackN.h"
#include "stackC.h"
#include "stackL.h"

/* types of var */
#define VT_I	0 /* Initial = INTERGER NULL */
#define VT_B	1 /* BOOLEAN */
#define VT_C	2 /* CHARACTER STRING */
#define VT_N	3 /* NUMBER */
#define VT_L	4 /* LIB FUNCTION */
#define VT_F	5 /* USER FUNCTION */

#define VT_V	9 /* VARIABLE (for "install_v ... in" only) */

#define VT_NO   -1 /* cf ci-dessous */

static void * stackV = VIDE;
/* modeExeVar guide le traitement des variables
 * si affectation : un des VT_x sinon -1
 * *************************************/

static int modeExeVar=VT_NO;

struct Var {
    char *l; /* libelle */
    void *n; /* next */
    union {
       bool b;
       void *a; /* adresse num/char/code */
    };
    short t; /* type */
};

static void cpVar(void * S, void *D)
{
struct Var *vS, *vD;
    if (S==D) return;
    vS = (struct Var *)S;
    vD = (struct Var *)D;
    vD->t = vS->t;
    vD->a = vS->a;
    /* duplication if number !! */
    if (vD->t == VT_N)
       vD->a = duplicateNum(vS->a, 0);
}

void initVar(char *Lib)
{
void * M, *L;
struct Var * N;
   if ((M = malloc(sizeof(struct Var))) == NULL) stopErr("initVar","malloc");
   if ((L = malloc(strlen(Lib)+1)) == NULL) stopErr("initVar","malloc");
   strcpy((char*)L,Lib);
   N = (struct Var*)M;
   N->l = (char*)L;
   N->n = stackV;
   N->t = VT_I;
   stackV = M;
}

static void setCodeVar(struct Var * Elt, short t, void* A)
{
   switch (Elt->t) { /* TODO VT_F */
      case  VT_C :
        free(Elt->a);
        break;
      case  VT_N :
        if (nbOnStack(Elt->a) == 0) free(Elt->a);
        else numVarOff(Elt->a);
        break;
      default : /* VT_I, VT_B, VT_L, VT_F */
        break;
   }
   Elt->t = t;
   Elt->a = A;
}

static void rmVar(void **Ref, struct Var * Elt)
{
   if (Elt->t==VT_N) setCodeVar(Elt, VT_I, VIDE);
   *Ref = Elt->n;
   free((void*)Elt->l);
   free((void*)Elt);
}

void rmLastVar(void)
{
   if (stackV != VIDE) rmVar(&stackV, (struct Var *)stackV);
   else messErr(23);
}

void IF_show_stackV(void)
{
void * Next;
struct Var * N;
    Next = stackV;
    while (Next != VIDE) {
       N = (struct Var*) Next;
       printf(" %-25s : Type ",N->l);
       switch(N->t) {
          case VT_I :
            printf("initial (NULL)");
            break;
          case VT_B :
            printf("Boolean ");
            if (N->b) printf("TRUE"); else printf("FALSE");
            break;
          case VT_C :
            printf("String \"%s\"", (char*)N->a);
            break;
          case VT_N :
            printf("Number ");
            printNumber(N->a);
            break;
          case VT_L :
            printf("Lib. Fct. %s", libByAddr(N->a));
            break;
          case VT_F :
            printf("User Fct. %s", fctByAddr(N->a));
            break;
       }
       printf("\n");
       Next = N->n;
    }
    printf("<end of variable list>\n");
}

static void newVar(char * S)
{
char Lib[LDFLT+1];
    strncpy(Lib,S,LDFLT);
    Lib[LDFLT]='\0';
    initVar(Lib);
    dropTrSuite();
}

void IF_debVar(void)
{
   putTrSuite(newVar);
}
void IF_debVarCS(void)
{
char *v;
    v = getString();
    if (v != NULL) {
       initVar(v);
       free((void*)v);
    }
}

void * varByName(char * L)
{
void * Next;
struct Var * N;
    Next = stackV;
    while (Next != VIDE) {
       N = (struct Var*) Next;
       if (strcmp(N->l,L)==0) return(Next);
       Next = N->n;
    }
    return VIDE;
}

char * varByAddr(void * A) /* non optimise mais C'EST FAIT EXPRES !! */
{
void * Next;
struct Var * N;
    Next = stackV;
    while (Next != VIDE) {
       N = (struct Var*) Next;
       if (Next==A) return(N->l);
       Next = N->n;
    }
    return NULL;
}

char * varByAddrA(void * A)
{
void * Next;
struct Var * N;
    Next = stackV;
    while (Next != VIDE) {
       N = (struct Var*) Next;
       if (N->a==A) return(N->l);
       Next = N->n;
    }
    return NULL;
}

int isVarChar(void * A)
{
struct Var * N;
    N = (struct Var*) A;
    if (N->t == VT_C) return 1;
    return 0;
}

static void exec_Var(void * A)
{
void * C;
struct Var * N;
void (*f) (void);
    N = (struct Var*) A;
    /* printf("executeVar %s %d !!\n",N->l, N->t);*/
    switch(N->t) {
       case VT_B :
          putBool(N->b);
          break;
       case VT_C :
          putString((char*)N->a);
          break;
       case VT_N :
          if (nbOnStack(N->a) > 0) {
             C = duplicateNum(N->a,1);
             N->a = C;
          }
          putVar(N->a);
          break;
       case VT_L :
          f = (PFV)(N->a);
          f();
          break;
       case VT_F :
          execFctV(N->a);
          break;
       default : /* VT_I */
          break;
    }
}

static void delVar(char * L)
{
void ** PNext;
struct Var * N;
    dropTrSuite();
    PNext = &stackV;
    while (*PNext != VIDE) {
       N = (struct Var*) *PNext;
       if (strcmp(N->l,L)==0) {
           rmVar(PNext, N);
           return;
       }
       PNext = &N->n;
    }
    messErr(24);
}

void IF_delVar(void)
{
   putTrSuite(delVar);
}

void putInVar(void * A, short t)
{
struct Var * N;
    N = (struct Var*) A;
    switch(t) {
       case VT_B :
          setCodeVar(N, t, VIDE);
          N->b = getBool();
          break;
       case VT_C :
          setCodeVar(N, t, getString());
          break;
       case VT_N :
          setCodeVar(N, t, getVar());
          break;
       case VT_L :
       case VT_F :
          setCodeVar(N, t, FCT_INST);
          break;
       default :
          setCodeVar(N, VT_I, VIDE);
          break;
    }
}

/* PLUS UTILISEE ***********************
static void updateVar(char * L, short t)
{
void * Next;
struct Var * N;
    Next = stackV;
    while (Next != VIDE) {
       N = (struct Var*) Next;
       if (strcmp(N->l,L)==0) {
           putInVar(Next, t);
           return;
       }
       Next = N->n;
    }
    messErr(24);
}
*****************/

void IF_setVarN(void)
{
   modeExeVar=VT_N;
}

void IF_setVarC(void)
{
   modeExeVar=VT_C;
}

void IF_setVarB(void)
{
   modeExeVar=VT_B;
}

void IF_setVarI(void)
{
   modeExeVar=VT_I;
}

void IF_setVarLF(void)
{
   switch (FCT_TYP) {
     case 0 :
        modeExeVar=VT_I;
        break;
     case 1 :
        modeExeVar=VT_L;
        break;
     case 2 :
        modeExeVar=VT_F;
        break;
     case 3 :
        modeExeVar=VT_V;
        break;
   }
}

void executeVar(void * A)
{
     if (fctEnCours) makeFct(T_VAR, A);
     else {
        if (modeExeVar == VT_NO) exec_Var(A);
        else {
          if (modeExeVar == VT_V) cpVar(FCT_INST,A);
          else putInVar(A, modeExeVar);
          modeExeVar=VT_NO;
        }
     }
}

int IF_execVar(char * L)
{
void * Next;
struct Var * N;
    Next = stackV;
    while (Next != VIDE) {
       N = (struct Var*) Next;
       if (strcmp(N->l,L)==0) {
           executeVar(Next);
           return 1;
       }
       Next = N->n;
    }
    return 0;
}


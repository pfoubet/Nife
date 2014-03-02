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
/* stackF.c */
#include "conf.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>

#include "nife.h"
#include "mth.h"
#include "err.h"
#include "lib.h"
#include "stackF.h"
#include "stackN.h"
#include "stackC.h"
#include "stackL.h"
#include "stackV.h"
#include "tasks.h"

#define MAXCODE  2048

int FctInTask=0;

static void * stackF = VIDE;

struct Fct {
    char *l; /* libelle */
    void *n; /* next */
    void *c; /* code */
    short typ; /* type : 0 std, 1 sys in compilation, 2 sys terminated */
};

void initFct(char *Lib, int typ)
{
void * M, *L;
struct Fct * N;
   if ((M = malloc(sizeof(struct Fct))) == NULL) stopErr("initFct","malloc");
   if ((L = malloc(strlen(Lib)+1)) == NULL) stopErr("initFct","malloc");
   strcpy((char*)L,Lib);
   N = (struct Fct*)M;
   N->l = (char*)L;
   N->n = stackF;
   N->c = VIDE;
   N->typ = typ;
   stackF = M;
}

static void eraseFct(struct Fct *F)
{
int i,n;
char *C, *E;
void *A, *W;
struct Fct *FD;
   /* printf("eraseFct(%s) at 0x%lx\n", F->l, (long)F); */
   free((void*)F->l);
   /* free associates memories */
   if (F->c != VIDE) {
      A = F->c;
      n = sizeof(A);
      i = *(int*)A;
      C = (char*)A+(3*sizeof(int));
      E = C+i;
      while (C < E) {
         switch((Code)*C) {
         case T_CHA :
         case T_CHAS :
         case T_NUM :
         case T_BKC :
         case T_BKC1 :
            bcopy((void*)(C+1),(void*)&W,n);
            free(W);
            break;
         case T_FCTD :
         case T_FCTDS :
         case T_FCTDW :
         case T_FCTDWS :
            bcopy((void*)(C+1),(void*)&W,n);
            FD = (struct Fct*)W;
            eraseFct(FD);
            break;
         default:
            break;
         }
         C+= n+1;
      }
      free(A);
   }
   free((void*)F);
}

void updDynFct(void *AF, int M) /* M:0=init, 1=start, 2=stop */
{
int i,n;
char *C, *E;
void *A, *W;
struct Fct *F, *FD;
   F = (struct Fct *)AF;
   /* printf("updDynFct(%s) at 0x%lx\n", F->l, (long)F); */
   if (F->c == VIDE) return;
   A = F->c;
   n = sizeof(A);
   i = *(int*)A;
   C = (char*)A+(3*sizeof(int));
   E = C+i;
   switch(M) {
   case 0 : /* init */
      while (C < E) {
         switch((Code)*C) {
         case T_CHAS :
            *C=T_CHA; 
            break;
         case T_VARS :
            *C=T_VAR; 
            break;
         case T_EXEKS :
            *C=T_EXEK; 
            break;
         case T_FCTD :
         case T_FCTDS :
         case T_FCTDW :
         case T_FCTDWS :
            *C=T_EXEK; 
            bcopy((void*)(C+1),(void*)&W,n);
            FD = (struct Fct*)W;
            eraseFct(FD);
            break;
         default:
            break;
         }
         C+= n+1;
      }
      break;
   case 1 : /* start */
      while (C < E) {
         switch((Code)*C) {
         case T_FCTDS :
            *C=T_FCTD; 
            break;
         case T_FCTDWS :
            *C=T_FCTDW; 
            break;
         default:
            break;
         }
         C+= n+1;
      }
      break;
   case 2 : /* stop */
      while (C < E) {
         switch((Code)*C) {
         case T_EXEK :
            *C=T_EXEKS; 
            break;
         case T_FCTD :
            *C=T_FCTDS; 
            break;
         case T_FCTDW :
            *C=T_FCTDWS; 
            break;
         default:
            break;
         }
         C+= n+1;
      }
      break;
   default:
      break;
   }
}

void rmLastFct(void)
{
struct Fct *Elt;
   if (stackF != VIDE) {
      Elt = (struct Fct *)stackF;
      if (Elt->typ==2) return;
      stackF = Elt->n;
      eraseFct(Elt);
   } else messErr(7);
}

static void unlinkLastFct(void)
{
struct Fct *Elt;
   if (stackF != VIDE) {
      Elt = (struct Fct *)stackF;
      stackF = Elt->n;
   }
}

struct Fct *putCodeFct(void* C)
{
struct Fct *Elt;
   if (stackF != VIDE) {
      Elt = (struct Fct *)stackF;
      if (Elt->c == VIDE) Elt->c = C;
      else messErr(8);
   }
   else messErr(7);
   return Elt;
}

void IF_show_stackF(void)
{
void * Next;
struct Fct * N;
char Ctyp;
    Next = stackF;
    while (Next != VIDE) {
       N = (struct Fct*) Next;
       if (N->typ) Ctyp='S'; else Ctyp=' ';
       printf(" %-25s%c %d octets\n",N->l,Ctyp,*((int*)N->c));
       Next = N->n;
    }
    printf("<end of function list>\n");
}

static char cod[MAXCODE];
static int i_cod;
/* pile pour IF ELSE THEN */
static int i_adr;
static void * adr[MAXCODE/4];
static char tad[MAXCODE/4];
/* pile pour BEGIN ... WHILE ... REPEAT / BEGIN ... AGAIN / BEGIN ... UNTIL */
static int i_adB;
static int adB[MAXCODE/4];
static char tcB[MAXCODE/4];
/* pile pour DO ... LOOP / +LOOP */
static int i_adD;
static int adD[MAXCODE/4];
static char tcD[MAXCODE/4];
/* pour l'execution */
static int I_DO=-1;
static char S_DO[MAXCODE/4];
static long D_DO[MAXCODE/4], L_DO[MAXCODE/4];

void IF_nDO (void)
{
   putLong((long long)(I_DO+1));
}

static void IF_getIndDo(int v)
{
int i = I_DO - v;
    if (i<0) putLong((long long)0);
    else putLong((long long)D_DO[i]);
}

int tadExist(Code c)
{
int i=0;
   while (i<i_adr) {
     if (tad[i] == c) return 1;
     i++;
   }
   return 0;
}

void IF_finFct(void)
{
void * M;
struct Fct * F;
int i,l, *ea, *Ea;
   if ((M = malloc((3*sizeof(int))+i_cod)) == NULL)
                                       stopErr("IF_finFct","malloc");
   ea = (int*)M;
   *ea++ = i_cod;
   *ea=0;
   Ea=ea+1;
   *Ea=0;
   /* on remplace tous les MYSELF */
   l = sizeof(M);
   for (i=0; i<i_cod; i+=(l+1)) {
      if (cod[i]==T_MYSF) {
            cod[i] = T_FCT;
            bcopy((void*)&M,(void*)&cod[i+1],l);
      } else  {
         if (cod[i]==T_ONER) {
            if (*ea==0) *ea = i;
            else {
               messErr(46);
               return;
            }
         } else {
            if (cod[i]==T_END) {
               if (*Ea==0) *Ea = i;
               else {
                  messErr(47);
                  return;
               }
            }
         }
      }
   }
   bcopy((void*)cod,(void*)((char*)M+(3*sizeof(int))),i_cod);
   F=putCodeFct(M);
   if (F->typ) {
      F->typ=2;
      addFonU(F->l,M);
   }
   /* printf("Total Fct : %d + %d !\n",i_cod,(3*sizeof(int))); */
   _MODIF_fctEnCours_(0);
}

void makeFct(Code c,void *A)
{
int d,i;
long L, L2, LE;
      d = sizeof(A);
      /*  printf("makeFct Entree : code %d + %d\n",(int)c,i_cod); */
      switch(c) {
      case T_RET :
      case T_NUM :
      case T_CHA :
      case T_LIB :
      case T_FCT :
      case T_MYSF :
      case T_DO_I  :
      case T_DO_J  :
      case T_DO_K  :
      case T_VAR  :
      case T_BKC :
      case T_BKC1 :
      case T_ONER :
      case T_END :
      case T_JEND :
      case T_EXEK :
          cod[i_cod++] = c;
          bcopy((void*)&A,(void*)&cod[i_cod],d);
          i_cod+=d;
          break;
      case T_IF :
          cod[i_cod++] = c;
          adr[i_adr]=(void*)&cod[i_cod];
          i_cod+=d;
          tad[i_adr++]=c;
          break;
      case T_ELSE :
          if (tad[i_adr-1] == T_IF) {
             cod[i_cod++] = T_JMP;
             adr[i_adr]=(void*)&cod[i_cod];
             L = (void*)&(cod[i_cod]) - adr[i_adr-1];
             i_cod+=d;
             bcopy((void*)&L,adr[i_adr-1],d);
             tad[i_adr++]=c;
          } else messErr(14);
          break;
      case T_THEN :
          if ((tad[i_adr-1] == T_IF) || (tad[i_adr-1] == T_ELSE)) {
             L = (void*)&cod[i_cod+1] - adr[i_adr-1] - (sizeof(void*)+1);/*AV5*/
             bcopy((void*)&L,adr[i_adr-1],d);
             tad[i_adr]='\0';
             while (tad[i_adr] != T_IF) i_adr--; /* depile adr */
          } else messErr(14);
          break;
      case T_BEGI :
          adB[i_adB]=i_cod;
          tcB[i_adB++]=c;
          break;
      case T_DO :
          cod[i_cod++] = c;
          i_cod+=d;
          adD[i_adD]=i_cod;
          tcD[i_adD++]=c;
          cod[i_cod++] = T_IFD;
          L = d+1;
          bcopy((void*)&L,(void*)&cod[i_cod],d);
          i_cod+=d;
          cod[i_cod++] = T_GOTO;
          L = -1;
          bcopy((void*)&L,(void*)&cod[i_cod],d);
          i_cod+=d;
          break;
      case T_PLOO :
      case T_LOOP :
          if (tcD[i_adD-1] == T_DO) {
             i_adD--; /* on depile */
             cod[i_cod++] = c;
             i_cod+=d;
             cod[i_cod++] = T_GOTO;
             L = adD[i_adD];
             bcopy((void*)&L,(void*)&cod[i_cod],d);
             i_cod+=d;
             /* maj des breaks GOTO -1 */
             LE = i_cod;
             for(i=L;i<i_cod-(d+1);i+=(d+1)) {
                 if (cod[i] == T_GOTO) {
                    bcopy((void*)&cod[i+1],(void*)&L2,d);
                    if (L2==-1) bcopy((void*)&LE,(void*)&cod[i+1],d);
                 }
             }
          } else messErr(39);
          break;
      case T_AGAI :
      case T_REPE :
          if (tcB[i_adB-1] == T_BEGI) {
             i_adB--; /* on depile */
             cod[i_cod++] = T_GOTO;
             L = adB[i_adB];
             bcopy((void*)&L,(void*)&cod[i_cod],d);
             i_cod+=d;
             /* maj des breaks GOTO -1 */
             LE = i_cod;
             for(i=L;i<i_cod-(d+1);i+=(d+1)) {
                 if (cod[i] == T_GOTO) {
                    bcopy((void*)&cod[i+1],(void*)&L2,d);
                    if (L2==-1) bcopy((void*)&LE,(void*)&cod[i+1],d);
                 }
             }
          } else messErr(22);
          break;
      case T_UNTI :
      case T_WHIL :
          if (tcB[i_adB-1] == T_BEGI) {
             cod[i_cod++] = T_IFN;
             /* if (c==T_UNTI) cod[i_cod++] = T_IFN;
             else           cod[i_cod++] = T_IF;
*/
             L = d+1;
             bcopy((void*)&L,(void*)&cod[i_cod],d);
             i_cod+=d;
             cod[i_cod++] = T_GOTO;
             if (c==T_UNTI) L = adB[i_adB-1];
             else           L = -1;
             bcopy((void*)&L,(void*)&cod[i_cod],d);
             i_cod+=d;
             if (c==T_UNTI) {
                i_adB--; /* depile adB */
                /* maj des breaks GOTO -1 */
                LE = i_cod;
                for(i=L;i<i_cod-(d+1);i+=(d+1)) {
                  if (cod[i] == T_GOTO) {
                    bcopy((void*)&cod[i+1],(void*)&L2,d);
                    if (L2==-1) bcopy((void*)&LE,(void*)&cod[i+1],d);
                  }
                }
             }
          } else messErr(22);
          break;
      case T_BREA :
             cod[i_cod++] = T_GOTO;
             L = -1; /* special value for BREAK */
             bcopy((void*)&L,(void*)&cod[i_cod],d);
             i_cod+=d;
          break;
      default :
          messErr(11);
      }
}

static void newFct2(char * S, int U)
{
char Lib[LDFLT+1];
    strncpy(Lib,S,LDFLT);
    Lib[LDFLT]='\0';
    initFct(Lib, U);
    _MODIF_fctEnCours_(1);
    dropTrSuite();
    i_cod = 0;
    i_adr = 0;
    i_adB = 0;
    i_adD = 0;
}
static void newFct0(char * S)
{
    newFct2(S,0);
}
static void newFct1(char * S)
{
    newFct2(S,1);
}

void IF_debFct(void)
{
   putTrSuite(newFct0);
}

void IF_debFctS(void)
{
   putTrSuite(newFct1);
}

int D_Cod=0; /* deep of execution functions */
static int D_LooP=0; /* deep of execution loops */
static int Do_Evts=0; /* Global events indicator for do...loop */

void IF_DO_MLeave (void)
{
long P, i;
    getParLong(&P);
    i=I_DO+1;
    if (i) {
       if (P > i) P=i;
       Do_Evts=P;
    }
}

void IF_DO_Leave (void)
{
    if (I_DO>=0) Do_Evts=1;
}

void IF_DO_Next (void)
{
    if (I_DO>=0) Do_Evts=-1;
}

void IF_DO_Show (void)
{
    printf("do vars : I_DO=%d Evts=%d\n",I_DO, Do_Evts);
}

void execCod(void *A)
{
int i,n, ea, Ea, *ai, InDo=0, OnErr=0, mFCTP;
long L, P;
char * C, *D, *F, *W, *S, *ADo_Next, *ADo_Leave;
void * T, *T2;
void (*f)(void);
struct Fct * FR;

    /* printf("pid = %d ITASK=%d FctInTask=%d\n",getpid(),ITASK,FctInTask);*/
    if (FctInTask) {
        if (ITASK==0) {
           if (FctInTask==-1) {
              FctInTask=0; return;
           }
           if (MakeTask(A)) return;
        }
        if (ITASK!=FctInTask) return;
    }
    D_Cod++;
    ai = (int*)A;
    i = *ai++;
    ea = *ai++;
    Ea = *ai;
    if (ea) tellOnErr(A);
    C = (char*)A+(3*sizeof(int));
    D = C;
    F = C+i;
    n = sizeof(T);
    while (C <= F) {
       /* printf("execCod : %s %d - %x : %ld\n",
              codByAddr(A),(int)(C-D),*C,(long)*(C+1));
       */
       if (noErr() && ((C==F) || ((Code)*C != T_ONER)) ) { /* to find onerr: */
          if (ea && (OnErr==0)) {
             C = D+ea;
          }  else {
             printf("Called in %s err=%d i=%d/%d cod=<%x>\n",
                    codByAddr(A),noErr(),(int)(C-D),i,*C);
             break; /* end of while */
          }
       }
       if (C==F) break; /* end of code */
       switch((Code)*C) {
       case T_ONER :
           if (noErr()==0) { /* jmp end: */
               if (Ea) C = D+Ea;
               else C = F; /* to break */
           } else {
             if (OnErr==0) {
                OnErr=1;
                majLastErr(A);
                razErr();
             } else C = F;
           }
           break;
       case T_RET :
           C = F; /* to break */
           break;
       case T_END :
           break; /* nothing */
       case T_JEND :
           if (Ea) C = D+Ea;
           else C = F; /* to break */
           break;
       case T_NUM :
           bcopy((void*)(C+1),(void*)&T,n);
           insertVal(T);
           break;
       case T_CHA :
           bcopy((void*)(C+1),(void*)&W,n);
           putString(W);
           break;
       case T_LIB :
           if (InstallOn) {
              if (InstallOn < 3) {
                 bcopy((void*)(C+1),(void*)&T,n);
                 _MODIF_FCT_INST_(T);
                 _MODIF_FCT_TYP_(1);
              } else {
                 _MODIF_FCT_INST_(VIDE);
                 _MODIF_FCT_TYP_(0);
              }
              InstallOn=0;
           } else {
              bcopy((void*)(C+1),(void*)&f,n);
              f();
              /* free context loops */
              if (Do_Evts) { /* quit or cut */
                 /*printf("execCod T_LIB : Evts %d\n",Do_Evts);*/
                 if (InDo) {
                    if (Do_Evts>0) {
                       C=ADo_Leave;
                       I_DO--;
                       InDo = 0;
                       Do_Evts--;
                    } else {
                       C=ADo_Next;
                       Do_Evts=0;
                    }
                 } else { /* quit */
                    C = F;
                 }
              }
           }
           break;
       case T_FCT :
           if (InstallOn) {
              if (InstallOn < 3) {
                 bcopy((void*)(C+1),(void*)&T,n);
                 T2=fctByCode(T);
                 _MODIF_FCT_INST_(T2);
                 _MODIF_FCT_TYP_(2);
              } else {
                 _MODIF_FCT_INST_(VIDE);
                 _MODIF_FCT_TYP_(0);
              }
              InstallOn=0;
           } else {
              bcopy((void*)(C+1),(void*)&T,n);
              execCod(T);
              /* free context loops */
              if (Do_Evts) { /* quit or cut */
                 /*printf("execCod T_FCT : Evts %d\n",Do_Evts);*/
                 if (InDo) {
                    if (Do_Evts>0) {
                       C=ADo_Leave;
                       I_DO--;
                       InDo = 0;
                       Do_Evts--;
                    } else {
                       C=ADo_Next;
                       Do_Evts=0;
                    }
                 } else { /* quit */
                    C = F;
                 }
              }
           }
           break;
       case T_FCTDS :
       case T_EXEKS :
           if ((S = getString()) != NULL)
              free((void*)S); /* remove the string */
           break;
       case T_FCTD :
           if ((S = getString()) != NULL)
              free((void*)S); /* remove the string */
           if (noErr()) break;
       case T_FCTDW :
       case T_FCTP :
              bcopy((void*)(C+1),(void*)&T,n);
              FR = (struct Fct *)T;
              execCod(FR->c);
              /* free context loops */
              if (Do_Evts) { /* quit or cut */
                 /*printf("execCod T_FCTD : Evts %d\n",Do_Evts);*/
                 if (InDo) {
                    if (Do_Evts>0) {
                       C=ADo_Leave;
                       I_DO--;
                       InDo = 0;
                       Do_Evts--;
                    } else {
                       C=ADo_Next;
                       Do_Evts=0;
                    }
                 } else { /* quit */
                    C = F;
                 }
              }
           if (*C == T_FCTP) {
              if (mFCTP) *C = T_FCTDW;
              else *C = T_FCTD;
           }
           break;
       case T_EXEK :
           if ((S = getString()) != NULL) {
              if (strlen(S)>0) { /* to do with T_FCTD */
                 mFCTP=0;
                 T = makeFunction(S);
                 if (T != VIDE) {
                    bcopy((void*)&T, (void*)(C+1),n);
                    *C = T_FCTP;
                    C -= (n+1);
                    unlinkLastFct();
                    /* upgrading precedent code ? not always ! */
                    if (C >= D) {
                       if (*C == T_CHA) { /* case of a string */
                          *C = T_CHAS;
                          mFCTP=1;
                       }
                       if (*C == T_VAR) { /* case of a variable string */
                          bcopy((void*)(C+1),(void*)&W,n);
                          if (isVarChar(W)) {
                             *C = T_VARS;
                             mFCTP=1;
                          }
                       }
                    }
                 } else /* error in compilation */
                    *C = T_EXEKS;
              }
              free((void*)S); 
           }
           break;
       case T_IF :
           if (!getBool()) {
               bcopy((void*)(C+1),(void*)&L,n);
               C += L;
           }
           break;
       case T_IFN :
           if (getBool()) {
               bcopy((void*)(C+1),(void*)&L,n);
               C += L;
           }
           break;
       case T_DO :
           I_DO++;
           InDo=1;
           /* maj do_adresses */
           W = C + (2*(n+1));
           bcopy((void*)(W+1),(void*)&L,n);
           ADo_Leave=D+L-n-1;
           ADo_Next=ADo_Leave-(2*(n+1));
           /* printf("execCod T_DO : AL= %d AN=%d\n",
                (int)(ADo_Leave-D), (int)(ADo_Next-D));*/
           getParLong(&P);
           D_DO[I_DO] = P;
           getParLong(&P);
           L_DO[I_DO] = P;
           if (P > D_DO[I_DO]) S_DO[I_DO]=0;
           else S_DO[I_DO]=1;
           break;
       case T_DO_I :
           IF_getIndDo(0);
           break;
       case T_DO_J :
           IF_getIndDo(1);
           break;
       case T_DO_K :
           IF_getIndDo(2);
           break;
       case T_IFD :
           if (S_DO[I_DO]) {
             if (D_DO[I_DO] > L_DO[I_DO]) {
               bcopy((void*)(C+1),(void*)&L,n);
               C += L;
             } else {
               I_DO--;
               InDo=0;
             }
           } else {
             if (D_DO[I_DO] < L_DO[I_DO]) {
               bcopy((void*)(C+1),(void*)&L,n);
               C += L;
             } else {
               I_DO--;
               InDo=0;
             }
           }
           break;
       case T_LOOP :
           if (S_DO[I_DO]) D_DO[I_DO]--;
           else D_DO[I_DO]++;
           break;
       case T_PLOO :
           getParLong(&P);
           D_DO[I_DO]+=P;
           break;
       case T_JMP :
           bcopy((void*)(C+1),(void*)&L,n);
           C += L;
           break;
       case T_GOTO :
           bcopy((void*)(C+1),(void*)&L,n);
           C = D + L - n-1;
           break;
       case T_VAR  :
           if (InstallOn) {
              if (InstallOn  == 3) {
                 bcopy((void*)(C+1),(void*)&T,n);
                 _MODIF_FCT_INST_(T);
                 _MODIF_FCT_TYP_(3);
              } else {
                 _MODIF_FCT_INST_(VIDE);
                 _MODIF_FCT_TYP_(0);
              }
              InstallOn=0;
           } else {
              bcopy((void*)(C+1),(void*)&W,n);
              executeVar(W);
              /* free context loops */
              if (Do_Evts) { /* quit or cut */
                 /*printf("execCod T_VAR : Evts %d\n",Do_Evts);*/
                 if (InDo) {
                    if (Do_Evts>0) {
                       C=ADo_Leave;
                       I_DO--;
                       InDo = 0;
                       Do_Evts--;
                    } else {
                       C=ADo_Next;
                       Do_Evts=0;
                    }
                 } else { /* quit */
                    C = F;
                 }
              }
           }
           break;
       case T_BKC :
           bcopy((void*)(C+1),(void*)&W,n);
           execLib(W);
           break;
       case T_BKC1 : /* like makeFct */
           bcopy((void*)(C+1),(void*)&W,n);
           /* try to modify the code */
           if (VARS==2) { /* VARS UP */
              if ((T = varByName(W)) != VIDE) {
                 *C = T_VAR;
              } else {
                 if ((T = fctByName(W)) != VIDE) {
                    *C = T_FCT;
                    FR = (struct Fct *)T;
                    T = FR->c;
                 }
              }
           } else {
              if ((T = fctByName(W)) != VIDE) {
                 *C = T_FCT;
                 FR = (struct Fct *)T;
                 T = FR->c;
              } else {
                 if ((VARS==1) && ((T = varByName(W)) != VIDE)) {
                    *C = T_VAR;
                 }
              }
           }
           if ((Code)*C != T_BKC1) { /* code is updated */
              bcopy((void*)&T, (void*)(C+1),n);
              C-=(n+1);       /* it must be executed */
           }
           break;
       case T_NOP :
       case T_CHAS :
       case T_VARS :
       case T_FCTDWS :
           break;
       default :
           messErr(11);
       }
       C+= n+1;
    }
    D_Cod--;
    if (ea) tellOnErr(VIDE);
}

void execFctV(void * A)
{
struct Fct * N;
   N = (struct Fct*) A;
   execCod(N->c);
}

int IF_execFct(char * L)
{
void * Next;
struct Fct * N;
    Next = stackF;
    while (Next != VIDE) {
       N = (struct Fct*) Next;
       if (strcmp(N->l,L)==0) {
           if (fctEnCours) makeFct(T_FCT,N->c);
           else execCod(N->c);
           return 1;
       }
       Next = N->n;
    }
    return 0;
}

void * fctByName(char * L)
{
void * Next;
struct Fct * N;
    Next = stackF;
    while (Next != VIDE) {
       N = (struct Fct*) Next;
       if (strcmp(N->l,L)==0) return Next;
       Next = N->n;
    }
    return VIDE;
}

void * fctByCode(void * C)
{
void * Next;
struct Fct * N;
    Next = stackF;
    while (Next != VIDE) {
       N = (struct Fct*) Next;
       if (N->c==C) return Next;
       Next = N->n;
    }
    return VIDE;
}

static void rmFct(char * L)
{
void ** PNext;
struct Fct * N;
    dropTrSuite();
    PNext = &stackF;
    while (*PNext != VIDE) {
       N = (struct Fct*) *PNext;
       if (N->typ==0) 
       if (strcmp(N->l,L)==0) {
           *PNext = N->n;
           eraseFct(N);
           return;
       }
       PNext = &N->n;
    }
    messErr(21);
}
static void rmAFct(char * L)
{
void ** PNext;
struct Fct * N;
    dropTrSuite();
    PNext = &stackF;
    while (*PNext != VIDE) {
       N = (struct Fct*) *PNext;
       if ((N->typ==0) && (strncmp(N->l,L,strlen(L))==0)) {
          *PNext = N->n;
          eraseFct(N);
       }
       else PNext = &N->n;
    }
}
static void rmOFct(char * L)
{
void ** PNext, ** FP;
struct Fct * N, * F;
    dropTrSuite();
    F = VIDE;
    PNext = &stackF;
    while (*PNext != VIDE) {
       N = (struct Fct*) *PNext;
       if (N->typ==0) 
       if (strcmp(N->l,L)==0) {
           FP = PNext;
           F = N;
       }
       PNext = &N->n;
    }
    if (F != VIDE) {
       *FP = F->n;
       eraseFct(F);
    }
    else messErr(21);
}

char * fctByAddr(void * A)
{
void * Next;
struct Fct * N;
    Next = stackF;
    while (Next != VIDE) {
       N = (struct Fct*) Next;
       if (Next==A) return N->l;
       Next = N->n;
    }
    return NULL;
}

char * codByAddr(void * A)
{
void * Next;
struct Fct * N;
    Next = stackF;
    while (Next != VIDE) {
       N = (struct Fct*) Next;
       if (N->c==A) return N->l;
       Next = N->n;
    }
    return NULL;
}

void prMarge(int n)
{
int N, i;
  N = n*3;
  for(i=0;i<N;i++) printf(" ");
}

static void scanFoncI(void * AdF, int marge)
{
void *A, *W;
struct Fct * N;
int i,n, ea, Ea, *ai;
long L;
char * C, *F, *D, lm[6];
    N = (struct Fct *)AdF;
    *lm = '\0';
    A = N->c;
    ai = (int*)A;
    i = *ai++;
    ea = *ai++;
    Ea = *ai;
    C = (char*)A+(3*sizeof(int));
    D = C;
    F = C+i;
    n = sizeof(A);
    if (marge) prMarge(marge);
    if (N->typ) printf ("System ");
    printf("Fonction : %s (%d) : 0x%lx\n", N->l, i, (unsigned long)A );
    if (ea+Ea) {
       if (ea) printf("Catching error at %d",ea);
       if (Ea) {
          if (ea) printf(" - ");
          printf("End label at %d",Ea);
       }
       printf("\n");
    }
    while (C < F) {
       if (marge) prMarge(marge);
       printf(" %.4d : ",(int)(C-D));
       switch((Code)*C) {
       case T_NOP :
       case T_CHAS :
       case T_VARS :
       case T_FCTDWS :
         printf("NOP\n");
         break;
       case T_FCTDS :
       case T_EXEKS :
         printf("\"drop\n");
         break;
       case T_RET :
         printf("RETURN\n");
         break;
       case T_ONER :
         printf("onerr: label\n");
         break;
       case T_END :
         printf("end: label\n");
         break;
       case T_JEND :
         printf("goto end:\n");
         break;
       case T_NUM :
         bcopy((void*)(C+1),(void*)&W,n);
         printf("Number value : ");
         printNumber(W);
         printf("\n");
         break;
       case T_CHA :
         bcopy((void*)(C+1),(void*)&W,n);
         printf("Character String \"%s\"\n",(char*)W);
         break;
       case T_LIB :
         bcopy((void*)(C+1),(void*)&W,n);
         printf("Call to library : %s\n", libByAddr(W));
         break;
       case T_FCT :
         bcopy((void*)(C+1),(void*)&W,n);
         printf("Function : %s\n", codByAddr(W));
         break;
       case T_FCTD :
         printf("\"drop + ");
       case T_FCTDW :
         bcopy((void*)(C+1),(void*)&W,n);
         N = (struct Fct *)W;
         printf("Dynamic Function at 0x%lx\n", (long)W);
         scanFoncI(W,marge+1);
         break;
       case T_IF :
         bcopy((void*)(C+1),(void*)&L,n);
         printf("IF false goto %ld\n",(C-D)+L+n+1);
         break;
       case T_DO :
         printf("DO [ LIMIT I -- ]\n");
         break;
       case T_DO_I :
         printf("GET I [ -- I ]\n");
         break;
       case T_DO_J :
         printf("GET J [ -- J ]\n");
         break;
       case T_DO_K :
         printf("GET K [ -- K ]\n");
         break;
       case T_LOOP :
         printf("I=+/-1\n");
         break;
       case T_PLOO :
         printf("I += V [ V -- ]\n");
         break;
       case T_IFN :
         bcopy((void*)(C+1),(void*)&L,n);
         printf("IF true goto %ld\n",(C-D)+L+n+1);
         break;
       case T_IFD :
         bcopy((void*)(C+1),(void*)&L,n);
         printf("IF (LIMIT NOT REACHED) goto %ld\n",(C-D)+L+n+1);
         break;
       case T_JMP :
         bcopy((void*)(C+1),(void*)&L,n);
         printf("JMP $+%ld\n",L);
         break;
       case T_GOTO :
         bcopy((void*)(C+1),(void*)&L,n);
         printf("GOTO %ld\n",L);
         break;
       case T_EXEK  :
         printf("Dynamic Compile (\"execk) !\n");
         break;
       case T_VAR  :
         bcopy((void*)(C+1),(void*)&W,n);
         printf("Call variable : %s\n", varByAddr(W));
         break;
       case T_BKC1 :
         strcpy(lm,"1st ");
       case T_BKC :
         bcopy((void*)(C+1),(void*)&W,n);
         printf("Back Compile %s: \"%s\"\n",lm, (char*)W);
         break;
       default :
           printf("0x%x : code inconnu !!\n",(int)*C);
       }
       C+= n+1;
    }
}

static void scanFonc(char * Lib)
{
void ** PNext;
struct Fct * N;
    dropTrSuite();
    PNext = &stackF;
    while (*PNext != VIDE) {
       N = (struct Fct*) *PNext;
       if (strcmp(N->l,Lib)==0) break;
       PNext = &N->n;
    }
    if (strcmp(N->l,Lib)!=0) {
        messErr(21);
        return;
    }
    scanFoncI((void*)N ,0);
}

void IF_execCS(void)
{
char * f;
    f = getString();
    if (f != NULL) {
       if (!IF_execFct(f)) {
          printf("%s - ",f); messErr(21);
       }
       free((void*)f);
    }
}

void IF_execCSl(void)
{
char * f;
    f = getString();
    if (f != NULL) {
       if (IF_execFct(f)) putBool(TRUE);
       else putBool(FALSE);
       free((void*)f);
    }
    else putBool(FALSE);
}

void IF_execCSv(void)
{
char * f;
    f = getString();
    if (f != NULL) {
       if (!IF_execVar(f)) {
          printf("%s - ",f); messErr(24);
       }
       free((void*)f);
    }
}

void IF_execCSvl(void)
{
char * f;
    f = getString();
    if (f != NULL) {
       if (IF_execVar(f)) putBool(TRUE);
       else putBool(FALSE);
       free((void*)f);
    }
    else putBool(FALSE);
}

void IF_delFct(void)
{
   putTrSuite(rmFct);
}
void IF_delAFct(void)
{
   putTrSuite(rmAFct);
}
void IF_delOFct(void)
{
   putTrSuite(rmOFct);
}

void IF_scanFct(void)
{
   putTrSuite(scanFonc);
}

static void IF_instruct(Code C)
{
    if (fctEnCours) makeFct(C,NULL);
    else messErr(13);
}

void IF_RET(void) { IF_instruct(T_RET); }
void IF_IF(void) { IF_instruct(T_IF); }
void IF_THEN(void) { IF_instruct(T_THEN); }
void IF_ELSE(void) { IF_instruct(T_ELSE); }
void IF_BEGIN(void) { IF_instruct(T_BEGI); }
void IF_AGAIN(void) { IF_instruct(T_AGAI); }
void IF_UNTIL(void) { IF_instruct(T_UNTI); }
void IF_WHILE(void) { IF_instruct(T_WHIL); }
void IF_REPEAT(void) { IF_instruct(T_REPE); }
void IF_BREAK(void) { IF_instruct(T_BREA); }
void IF_MYSELF(void) { IF_instruct(T_MYSF); }
void IF_DO(void) { IF_instruct(T_DO); }
void IF_LOOP(void) { IF_instruct(T_LOOP); }
void IF_PLOOP(void) { IF_instruct(T_PLOO); }
void IF_I_DO(void) { IF_instruct(T_DO_I); }
void IF_J_DO(void) { IF_instruct(T_DO_J); }
void IF_K_DO(void) { IF_instruct(T_DO_K); }
void IF_ONERR(void) { IF_instruct(T_ONER); }
void IF_END(void) { IF_instruct(T_END); }
void IF_JEND(void) { IF_instruct(T_JEND); }
void IF_EXEK(void) { IF_instruct(T_EXEK); }

/* code for back compilation of calling functions and variables */

void suiteBackC(char *S)
{
void * M;
    dropTrSuite();
    if (strlen(S) > LDFLT) {
        messErr(9);
        return;
    }
    if (S[strlen(S)-1] != '\'') {
        messErr(44);
        return;
    }
    S[strlen(S)-1] = '\0';
    if ((M = malloc(strlen(S)+1)) == NULL) stopErr("suiteBackC","malloc");
#ifdef DEBUG_M
    printf("New String address : %lu \n",(unsigned long)M);
#endif
    strcpy((char*)M,S);
    if (fctEnCours) makeFct(T_BKC,M);
    else messErr(13);
}

void IF_debBackC(void)
{
    putTrSuite(suiteBackC);
}

void suiteBackC1(char *S)
{
void * M;
    dropTrSuite();
    if (strlen(S) > LDFLT) {
        messErr(9);
        return;
    }
    if (S[strlen(S)-1] != '`') {
        messErr(44);
        return;
    }
    S[strlen(S)-1] = '\0';
    if ((M = malloc(strlen(S)+1)) == NULL) stopErr("suiteBackC1","malloc");
#ifdef DEBUG_M
    printf("New String address : %lu \n",(unsigned long)M);
#endif
    strcpy((char*)M,S);
    if (fctEnCours) makeFct(T_BKC1,M);
    else messErr(13);
}

void IF_debBackC1(void)
{
    putTrSuite(suiteBackC1);
}



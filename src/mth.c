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
/* mth.c  : for multi-threading implementation */
#include "conf.h"

#include <stdio.h>
#include <stdlib.h>

#include "nife.h"
#include "mth.h"
#include "err.h"

#ifdef _MULTI_THREADING_

struct TH_Base {
   void *   stk_N;   
   void *   Fct_Inst;
   uint32_t netKEY;
   int      i_stkL;
   int      i_stkC;
   int      Fd_In;
   int      i_Ts;
   char *   stk_C[LSTACKC];
   short    NbLig;
   short    NbTab;
   short    Vars;
   short    FcType;
   bool     stk_L[LSTACKL];
   bool     Double;
   bool     EchoOff;
   bool     Run;
   bool     WaitPid;
   bool     fctEnC;
   bool     strEnC;
   bool     inSonP;
   char     BufC[MAXSTRING];
   char     BufP[LBUF];
   char     BufP2[LBUF];
   PFC      trSuite[NBTRSUITE];
   jmp_buf  Env_Int;
};

pthread_key_t
    k_Base,       /* the base structure */
    k_StkN,       /* Numerical stack */
    k_StkL,       /* Logical stack */
    k_iStL,       /* index of Logical stack */
    k_StkC,       /* Character stack */
    k_iStC,       /* index of Character stack */
    k_FdIn,       /* File Descriptor input  */
    k_iTs,        /* index of TraiteSuite */
    k_bufC,       /* buffer for Character stack */
    k_bufP,       /* buffer for Principal Input */
    k_bufP2,      /* buffer for Secondary Input */
    k_trSu,       /* table PFC traiteSuite */
    k_FIns,       /* Fct_Inst Fct to be install Lib or User */
    k_EnvI,       /* Env_Int */
    k_NetK,       /* NetKey  */
    k_NLig,       /* NbLig  */
    k_NTab,       /* NbTab  */
    k_Vars,       /* VARS  */
    k_FTyp,       /* FCT_TYP  */
    k_Doub,       /* Double On/Off */
    k_Run,        /* RUN On/Off */
    k_WPid,       /* WAITPID On/Off */
    k_fEnC,       /* fctEnCours On/Off */
    k_sEnC,       /* stringEnCours On/Off */
    k_inSP,       /* inSonProc On/Off */
    k_Echo;       /* EchoOff On/Off */

static pthread_once_t k_Init = PTHREAD_ONCE_INIT;
static void make_keys()
{
   pthread_key_create(&k_Base, free);
   pthread_key_create(&k_StkN, NULL);
   pthread_key_create(&k_StkL, NULL);
   pthread_key_create(&k_iStL, NULL);
   pthread_key_create(&k_StkC, NULL);
   pthread_key_create(&k_iStC, NULL);
   pthread_key_create(&k_FdIn, NULL);
   pthread_key_create(&k_iTs, NULL);
   pthread_key_create(&k_bufC, NULL);
   pthread_key_create(&k_bufP, NULL);
   pthread_key_create(&k_bufP2, NULL);
   pthread_key_create(&k_trSu, NULL);
   pthread_key_create(&k_FIns, NULL);
   pthread_key_create(&k_EnvI, NULL);
   pthread_key_create(&k_NetK, NULL);
   pthread_key_create(&k_NLig, NULL);
   pthread_key_create(&k_NTab, NULL);
   pthread_key_create(&k_Vars, NULL);
   pthread_key_create(&k_FTyp, NULL);
   pthread_key_create(&k_Doub, NULL);
   pthread_key_create(&k_Run, NULL);
   pthread_key_create(&k_WPid, NULL);
   pthread_key_create(&k_fEnC, NULL);
   pthread_key_create(&k_sEnC, NULL);
   pthread_key_create(&k_inSP, NULL);
   pthread_key_create(&k_Echo, NULL);
}


void TH_create(void) /* create current thread variables */
{
void * M;
struct TH_Base * A;
    pthread_once(&k_Init, make_keys);
    if ((M = pthread_getspecific(k_Base)) == NULL) {
        if ((M=malloc(sizeof(struct TH_Base)))==NULL)
            stopErr("TH_create","malloc");
        pthread_setspecific(k_Base, M);
        /* initialisation */
        A = (struct TH_Base *)M;
        A->stk_N = VIDE;
        A->Fct_Inst = VIDE;
        A->netKEY = 0;
        A->NbLig = 10;
        A->NbTab = 6;
        A->Vars = 1;
        A->FcType = 0;
        A->EchoOff = 0;
        A->Double = 0;
        A->Run = 1;
        A->WaitPid = 0;
        A->fctEnC = 0;
        A->strEnC = 0;
        A->inSonP = 0;
        A->i_stkL = 0;
        A->i_stkC = 0;
        A->Fd_In = 0;
        A->i_Ts = 0;
        pthread_setspecific(k_StkN, (void*)&(A->stk_N));
        pthread_setspecific(k_FIns, (void*)&(A->Fct_Inst));
        pthread_setspecific(k_NetK, (void*)&(A->netKEY));
        pthread_setspecific(k_NLig, (void*)&(A->NbLig));
        pthread_setspecific(k_NTab, (void*)&(A->NbTab));
        pthread_setspecific(k_Vars, (void*)&(A->Vars));
        pthread_setspecific(k_FTyp, (void*)&(A->FcType));
        pthread_setspecific(k_Echo, (void*)&(A->EchoOff));
        pthread_setspecific(k_Doub, (void*)&(A->Double));
        pthread_setspecific(k_StkL, (void*)(A->stk_L));
        pthread_setspecific(k_iStL, (void*)&(A->i_stkL));
        pthread_setspecific(k_StkC, (void*)(A->stk_C));
        pthread_setspecific(k_iStC, (void*)&(A->i_stkC));
        pthread_setspecific(k_FdIn, (void*)&(A->Fd_In));
        pthread_setspecific(k_iTs, (void*)&(A->i_Ts));
        pthread_setspecific(k_bufC, (void*)(A->BufC));
        pthread_setspecific(k_bufP, (void*)(A->BufP));
        pthread_setspecific(k_bufP2, (void*)(A->BufP2));
        pthread_setspecific(k_trSu, (void*)(A->trSuite));
        pthread_setspecific(k_Run, (void*)&(A->Run));
        pthread_setspecific(k_WPid, (void*)&(A->WaitPid));
        pthread_setspecific(k_fEnC, (void*)&(A->fctEnC));
        pthread_setspecific(k_sEnC, (void*)&(A->strEnC));
        pthread_setspecific(k_inSP, (void*)&(A->inSonP));
        pthread_setspecific(k_EnvI, (void*)&(A->Env_Int));
    }
}

void TH_init(void)
{
   TH_create();
}


#else /* NOT _MULTI_THREADING_ */

void * G_StackN = VIDE;
int G_Double=0;  /* 0 si LONG, 1 si DOUBLE */
int G_EchoOff=0; /* 0 si echo on, 1 si echo off */
int G_NBTAB=6;   /* nb d'elements de tableau affiches */
int G_NBLIG=10;  /* nb de lignes du stack affichees */
short G_VARS=1,  /* 0 VAR_OFF , 1 VAR_DOWN (default), 2 VAR_UP */
          G_FCT_TYP=0;  /* 0 None (default) , 1 Lib Fct , 2 User Fct */
void * G_F_INS=VIDE; /* fct lib ou usr a installer */
uint32_t G_NetKey=0;
bool G_stackL[LSTACKL];
int G_i_stackL=0;
char * G_stackC[LSTACKC];
int G_i_stackC=0;
char G_bufC[MAXSTRING];
short G_Run=1;
short G_WAITPID=0;
short G_strEnCours=0;
short G_fctEnCours=0;
short G_inSonProc=0;
int G_FD_IN = 0; /* lecture sur stdin par defaut */
int G_iTS=0;
char G_bufP[LBUF];
char G_bufP2[LBUF];
PFC G_traiteSuite [NBTRSUITE];
jmp_buf G_ENV_INT;

void TH_init(void) /* do nothing */
{
   return;
}
#endif /* _MULTI_THREADING_ */


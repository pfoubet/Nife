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
/* mth.h  : for multi-threading implementation */

extern void TH_init(void);

#ifndef __NIFE_MTH_H__
#define __NIFE_MTH_H__

#include <stdio.h>
#include <stdint.h>
#include <setjmp.h>
#include <pthread.h>

#define LSTACKL 512
#define LSTACKC 512
#define MAXSTRING 512
#define NBTRSUITE 10
#define LBUF 512
#define VIDE ((void*)NULL)


#ifdef _MULTI_THREADING_
extern pthread_key_t k_Base, k_StkN, k_FIns, k_NetK, k_NLig, k_NTab, k_Vars,
       k_FTyp, k_Doub, k_Echo, k_StkL, k_iStL, k_StkC, k_iStC, k_bufC, k_Run,
       k_WPid, k_fEnC, k_sEnC, k_inSP, k_iTs, k_FdIn, k_bufP, k_bufP2, k_trSu,
       k_EnvI;

#define StackN *((void**)pthread_getspecific(k_StkN))
#define _MODIF_STACKN_(x)  *((void**)pthread_getspecific(k_StkN))=x
#define _ADDR_STACKN_  (void**)pthread_getspecific(k_StkN)

#define DOUBLE  *((bool*)pthread_getspecific(k_Doub))
#define _MODIF_DOUBLE_(x) *((bool*)pthread_getspecific(k_Doub))=x

#define ECHOOFF  *((bool*)pthread_getspecific(k_Echo))
#define _MODIF_ECHOOFF_(x) *((bool*)pthread_getspecific(k_Echo))=x

#define NBLIG  *((short*)pthread_getspecific(k_NLig))
#define _MODIF_NBLIG_(x) *((short*)pthread_getspecific(k_NLig))=x

#define NBTAB  *((short*)pthread_getspecific(k_NTab))
#define _MODIF_NBTAB_(x) *((short*)pthread_getspecific(k_NTab))=x

#define VARS  *((short*)pthread_getspecific(k_Vars))
#define _MODIF_VARS_(x) *((short*)pthread_getspecific(k_Vars))=x

#define FCT_TYP  *((short*)pthread_getspecific(k_FTyp))
#define _MODIF_FCT_TYP_(x) *((short*)pthread_getspecific(k_FTyp))=x

#define FCT_INST *((void**)pthread_getspecific(k_FIns))
#define _MODIF_FCT_INST_(x)  *((void**)pthread_getspecific(k_FIns))=x

#define NetKey  *((uint32_t*)pthread_getspecific(k_NetK))
#define _MODIF_NetKey_(x) *((uint32_t*)pthread_getspecific(k_NetK))=x
#define _ADDV_NetKey_  pthread_getspecific(k_NetK)

#define stackL  ((bool*)pthread_getspecific(k_StkL))

#define i_StackL *((int*)pthread_getspecific(k_iStL))
#define _MODIF_i_StackL_(x)  *((int*)pthread_getspecific(k_iStL))=x

#define stackC  ((char**)pthread_getspecific(k_StkC))

#define i_StackC *((int*)pthread_getspecific(k_iStC))
#define _MODIF_i_StackC_(x)  *((int*)pthread_getspecific(k_iStC))=x

#define bufC  ((char*)pthread_getspecific(k_bufC))
#define bufP  ((char*)pthread_getspecific(k_bufP))
#define bufP2 ((char*)pthread_getspecific(k_bufP2))
#define traiteSuite  ((PFC*)pthread_getspecific(k_trSu))

#define RUN  *((bool*)pthread_getspecific(k_Run))
#define _MODIF_RUN_(x) *((bool*)pthread_getspecific(k_Run))=x

#define WAITPID  *((bool*)pthread_getspecific(k_WPid))
#define _MODIF_WAITPID_(x) *((bool*)pthread_getspecific(k_WPid))=x

#define stringEnCours  *((bool*)pthread_getspecific(k_sEnC))
#define _MODIF_stringEnCours_(x) *((bool*)pthread_getspecific(k_sEnC))=x

#define fctEnCours  *((bool*)pthread_getspecific(k_fEnC))
#define _MODIF_fctEnCours_(x) *((bool*)pthread_getspecific(k_fEnC))=x

#define inSonProc  *((bool*)pthread_getspecific(k_inSP))
#define _MODIF_inSonProc_(x) *((bool*)pthread_getspecific(k_inSP))=x

#define FD_IN *((int*)pthread_getspecific(k_FdIn))
#define _MODIF_FD_IN_(x)  *((int*)pthread_getspecific(k_FdIn))=x

#define iTS *((int*)pthread_getspecific(k_iTs))
#define _MODIF_iTS_(x)  *((int*)pthread_getspecific(k_iTs))=x

#define ENV_INT (jmp_buf*)pthread_getspecific(k_EnvI)

#else /* *************** NOT _MULTI_THREADING_ ******************** */

extern void * G_StackN;
extern int G_Double;
extern int G_EchoOff;
extern int G_NBTAB;
extern int G_NBLIG;
extern short G_VARS;
extern short G_FCT_TYP;
extern void * G_F_INS;
extern uint32_t G_NetKey;
extern bool G_stackL[];
extern int G_i_stackL;
extern char * G_stackC[];
extern int G_i_stackC;
extern char G_bufC[];
extern short G_Run;
extern short G_WAITPID;
extern short G_strEnCours;
extern short G_fctEnCours;
extern short G_inSonProc;
extern int G_FD_IN;
extern int G_iTS;
extern char G_bufP[];
extern char G_bufP2[];
extern PFC G_traiteSuite[];
/* a regler */
extern jmp_buf G_ENV_INT;


#define RUN G_Run
#define _MODIF_RUN_(x) G_Run=(x)

#define WAITPID G_WAITPID
#define _MODIF_WAITPID_(x) G_WAITPID=(x)

#define stringEnCours G_strEnCours
#define _MODIF_stringEnCours_(x) G_strEnCours=(x)

#define fctEnCours G_fctEnCours
#define _MODIF_fctEnCours_(x) G_fctEnCours=(x)

#define inSonProc G_inSonProc
#define _MODIF_inSonProc_(x) G_inSonProc=(x)

#define FD_IN G_FD_IN
#define _MODIF_FD_IN_(x) G_FD_IN=(x)

#define iTS G_iTS
#define _MODIF_iTS_(x) G_iTS=(x)

#define stackL G_stackL

#define i_StackL G_i_stackL
#define _MODIF_i_StackL_(x) G_i_stackL=(x)

#define stackC G_stackC

#define i_StackC G_i_stackC
#define _MODIF_i_StackC_(x) G_i_stackC=(x)

#define bufC G_bufC
#define bufP G_bufP
#define bufP2 G_bufP2
#define traiteSuite G_traiteSuite

#define StackN G_StackN
#define _MODIF_STACKN_(x) G_StackN=(x)
#define _ADDR_STACKN_ &G_StackN

#define DOUBLE G_Double
#define _MODIF_DOUBLE_(x) G_Double=(x)

#define ECHOOFF G_EchoOff
#define _MODIF_ECHOOFF_(x) G_EchoOff=(x)

#define NBLIG G_NBLIG
#define _MODIF_NBLIG_(x) G_NBLIG=(x)

#define NBTAB G_NBTAB
#define _MODIF_NBTAB_(x) G_NBTAB=(x)

#define VARS G_VARS
#define _MODIF_VARS_(x) G_VARS=(x)

#define FCT_TYP G_FCT_TYP
#define _MODIF_FCT_TYP_(x) G_FCT_TYP=(x)

#define FCT_INST G_F_INS
#define _MODIF_FCT_INST_(x) G_F_INS=(x)

#define NetKey G_NetKey
#define _MODIF_NetKey_(x) G_NetKey=(x)
#define _ADDV_NetKey_  (void*)&G_NetKey

#define ENV_INT G_ENV_INT

#endif /* _MULTI_THREADING_ */


#endif


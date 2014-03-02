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
/* stackF.h */

#ifndef __NIFE_STACKF_H__
#define __NIFE_STACKF_H__

extern int FctInTask;

typedef unsigned char Code;
#define T_NOP    ((Code)0) /* No OPeration */
#define T_RET    ((Code)1) /* RETURN in function */
#define T_NUM    ((Code)2) /* Numeric element */
#define T_CHA    ((Code)3) /* Character element */
#define T_LIB    ((Code)4) /* Standard Library function */
#define T_FCT    ((Code)5) /* User function */
#define T_IF     ((Code)6) /* IF */
#define T_ELSE   ((Code)7) /* THEN */
#define T_THEN   ((Code)8) /* ELSE */
#define T_JMP    ((Code)9) /* JUMP depl */
#define T_BEGI   ((Code)10)/* BEGIN */
#define T_AGAI   ((Code)11)/* AGAIN */
#define T_UNTI   ((Code)12)/* UNTIL */
#define T_WHIL   ((Code)13)/* WHILE */
#define T_REPE   ((Code)14)/* REPEAT */
#define T_DO     ((Code)15)/* DO */
#define T_LOOP   ((Code)16)/* LOOP */
#define T_PLOO   ((Code)17)/* +LOOP */
#define T_BREA   ((Code)18)/* BREAK in all loops */
#define T_GOTO   ((Code)19)/* GOTO */
#define T_MYSF   ((Code)20)/* MYSELF for current function */
#define T_IFN    ((Code)21)/* IF NOT */
#define T_IFD    ((Code)22)/* IF in DO */
#define T_DO_I   ((Code)23)/* INDEX of DO */
#define T_DO_J   ((Code)24)/* INDEX of DO PREVIOUS LEVEL */
#define T_DO_K   ((Code)25)/* INDEX of DO 2 LEVELS UP */
#define T_VAR    ((Code)26)/* Variable */
#define T_BKC    ((Code)27)/* Back Compilation */
#define T_BKC1   ((Code)28)/* Back Compilation on 1st time */
#define T_ONER   ((Code)29)/* onerr: for current function */
#define T_END    ((Code)30)/* end: for current function */
#define T_JEND   ((Code)31)/* goto end: for current function */
#define T_EXEK   ((Code)32)/* "execk code */
#define T_FCTD   ((Code)33)/* Dynamic User function */
#define T_FCTP   ((Code)34)/* Primary Dynamic User function */
#define T_FCTDW  ((Code)35)/* Dynamic User function without remove string */
#define T_CHAS   ((Code)36)/* Character element Stopped */
#define T_VARS   ((Code)37)/* Variable Stopped */
#define T_FCTDS  ((Code)38)/* Dynamic User funct. Stopped */
#define T_FCTDWS ((Code)39)/* Dynamic User funct. Stopped without remove str. */
#define T_EXEKS  ((Code)40)/* "execk code Stopped */

extern int D_Cod;
extern void IF_show_stackF(void);
extern void IF_debFct(void);
extern void IF_debFctS(void);
extern void IF_finFct(void);
extern void updDynFct(void *A, int Mode);
extern void rmLastFct(void);
extern void makeFct(Code c, void * E);
extern int IF_execFct(char * L);
extern void IF_delFct(void);
extern void IF_delAFct(void);
extern void IF_delOFct(void);
extern void IF_scanFct(void);
extern void IF_DO_Leave(void);
extern void IF_DO_MLeave(void);
extern void IF_DO_Next(void);
extern void IF_DO_Show(void);
extern void execCode(void *C);
extern void * fctByName(char *L);
extern void * fctByCode(void *C);
extern void execFctV(void * A);
extern char * fctByAddr(void *A);
extern char * codByAddr(void *A);
extern void IF_nDO(void);

extern void IF_execCS(void);
extern void IF_execCSv(void);
extern void IF_execCSl(void);
extern void IF_execCSvl(void);
extern void IF_RET(void);
extern void IF_IF(void);
extern void IF_THEN(void);
extern void IF_ELSE(void);
extern void IF_BEGIN(void);
extern void IF_AGAIN(void);
extern void IF_UNTIL(void);
extern void IF_WHILE(void);
extern void IF_REPEAT(void);
extern void IF_BREAK(void);
extern void IF_MYSELF(void);
extern void IF_ONERR(void);
extern void IF_END(void);
extern void IF_JEND(void);
extern void IF_DO(void);
extern void IF_LOOP(void);
extern void IF_PLOOP(void);
extern void IF_I_DO(void);
extern void IF_J_DO(void);
extern void IF_K_DO(void);
extern void IF_EXEK(void);

extern void IF_debBackC(void);
extern void IF_debBackC1(void);

#endif

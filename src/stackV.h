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
/* stackV.h */

#ifndef __NIFE_STACKV_H__
#define __NIFE_STACKV_H__

extern void IF_debVar(void);
extern void IF_debVarCS(void);
extern void IF_show_stackV(void);
extern void IF_delVar(void);
extern void rmLastVar(void);
extern int IF_execVar(char *L);
extern int isVarChar(void *A);
extern void * varByName(char *L);
extern char * varByAddr(void *A);
extern char * varByAddrA(void *A);
extern void putInVar(void *A, short t);
extern void IF_setVarI(void);
extern void IF_setVarB(void);
extern void IF_setVarC(void);
extern void IF_setVarN(void);
extern void IF_setVarLF(void);
extern void executeVar(void *);

#endif

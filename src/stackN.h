/* Copyright (C) 2011-2014  Patrick H. E. Foubet - S.E.R.I.A.N.E.

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
/* stackN.h */

#ifndef __NIFE_STACKN_H__
#define __NIFE_STACKN_H__

#include <stdint.h>

#define _VERIF_STACK_ if (StackN == VIDE) { messErr(2); return; }

extern int lAdrNum(void);
extern int putVal(char *V);
extern void putLong(long long V);
extern void putDouble(double d);
extern int getParLong(long *V);
extern void insertVal(void *A);
extern void putVar(void *A);
extern void * getVar(void);
extern void IF_vers(void);
extern void IF_drop(void);
extern void IF_stack_clear(void);
extern void printNumber(void * N);
extern void numVarOff(void * N);
extern void * duplicateNum(void * S, int vSoff);
extern void IF_show_stack(void);
extern void IF_ramp(void);
extern void IF_dramp(void);
extern void IF_DEC(void);
extern void IF_HEX(void);
extern void IF_OCT(void);
extern void IF_BIN(void);
extern void IF_REAL(void);
extern void IF_INTEGER(void);
extern void IF_ECHOFF(void);
extern void IF_ECHOON(void);
extern void IF_NBTAB(void);
extern void IF_NBLIG(void);
extern void IF_VAROFF(void);
extern void IF_VARUP(void);
extern void IF_VARDOWN(void);
extern void IF_vars(void);
extern void IF_point(void);
extern void IF_swap(void);
extern void IF_dup(void);
extern void IF_over(void);
extern void IF_pick(void);
extern void IF_rot(void);
extern void IF_unrot(void);
extern void IF_roll(void);
extern void IF_unroll(void);
extern void IF_Ndrop(void);
extern void IF_Ndup(void);
extern void IF_depth(void);
extern int  nbOnStack(void *A);
extern void IF_toArray(void);
extern void IF_toScalar(void);
extern void IF_toScalarR(void);
extern void IF_fctD_1(double(*f)(double));
extern void IF_fctD_1L(long long(*f)(double));
extern void IF_fctD_1LB(long long(*f)(double));
extern void IF_fctB_1(long long (*f1)(long long), double(*f2)(double));
extern void IF_plus(void);
extern void IF_moins(void);
extern void IF_mult(void);
extern void IF_div(void);
extern void IF_neg(void);
extern void IF_min(void);
extern void IF_max(void);
extern void IF_modulo(void);
extern void IF_puiss(void);
extern void IF_Legal(void);
extern void IF_Ldiff(void);
extern void IF_Lsup(void);
extern void IF_Linf(void);
extern void IF_Lsupeg(void);
extern void IF_Linfeg(void);
extern void IF_inFile_1(FILE * fd);
extern void IF_inFile_1d(FILE * fd, char delim, int virg);
extern void IF_inFile_2(FILE * fd);
extern void IF_TShiftR(void);
extern void IF_TShiftL(void);
extern void IF_NTShiftR(void);
extern void IF_NTShiftL(void);
extern void IF_TNShiftR(void);
extern void IF_TNShiftL(void);
extern void IF_NTNShiftR(void);
extern void IF_NTNShiftL(void);
extern void IF_subTab(void);
extern void IF_subTabR(void);
extern void IF_NsubTab(void);
extern void IF_NsubTabR(void);
extern void IF_TabRev(void);
extern void IF_NTabRev(void);
extern void IF_TabTransp(void);
extern void IF_TabTranspN(void);
extern void IF_TabTranspT(void);
extern int is1Tab(void);
extern int is2Tab(void);
extern int isNTabSameDim(int n);
extern void IF_TABMin(void);
extern void IF_TABMax(void);
extern void IF_TABProd(void);
extern void IF_TABSum(void);
extern void IF_TABMinMax(void);

extern void dump_eltN(int fd, void *A, uint32_t M);
extern void * restore_eltN(int fd);
extern void dump_stackN(int fd);
extern void restore_stackN(int fd);
extern void restore_links_stackN(void);

/* net functions */
extern void IF_NetKey(void);
extern void IF_NetErrVal(void);
extern void StackToNet(long n);
extern void NetToStack(int s, uint32_t k);
extern int NetDepth(uint32_t k);
extern void IF_show_netStack(uint32_t k);
extern void IF_netDrop(uint32_t k);

#endif

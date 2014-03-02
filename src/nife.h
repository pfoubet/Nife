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
/* nife.h */
#ifndef __NIFE_NIFE_H__
#define __NIFE_NIFE_H__

#include <setjmp.h>
#include <errno.h>

#define LDFLT 24 /* default length for names */

extern jmp_buf ENV_INT;
extern void interInfos(char *P, char*M);

typedef void (*PFV) (void);
typedef void (*PFC) (char *);
typedef unsigned char bool;

extern int isSepa(char c, int m);
extern void IF_about(void);
extern void IF_Load(void);
extern void IF_LoadCS(void);
extern void * makeFunction(char *S);
extern void IF_ExecCS(void);
extern void IF_ExecCSf(void);
extern void putTrSuite(PFC);
extern void dropTrSuite(void);
extern void compileFile(char *);
extern PFC getTrSuite(void);

#endif

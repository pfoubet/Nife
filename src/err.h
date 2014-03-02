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
/* err.h */

#ifndef __NIFE_ERR_H__
#define __NIFE_ERR_H__

extern char * InExec;
extern void tellOnErr (void*A);
extern void razErr(void);
extern void majLastErr(void*A);
extern int  noErr(void);
extern void stopErr(char *M, char *NomFctSystem);
extern void messErr(int N);
extern void messErr2(int N, char * Lib);
extern void IF_showError(void);
extern void IF_IsError(void);
extern void IF_NoError(void);
extern void IF_LibError(void);

#endif

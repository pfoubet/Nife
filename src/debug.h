/* Copyright (C) 2011-2019  Patrick H. E. Foubet - S.E.R.I.A.N.E.

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
/* debug.h */

#ifndef __NIFE_DEBUG_H__
#define __NIFE_DEBUG_H__
extern int Debug;
extern int InDebugFct;

#define _IFD_BEGIN_ int fd; fd=dup(1); dup2(2,1); InDebugFct=1;
#define _IFD_END_   dup2(fd,1); close(fd); InDebugFct=0;

extern void D_Reset(void);
extern void IFD_Update(void);
extern void IFD_SaveLog(void);
extern void IFD_DebugTOn(void);
extern void IFD_DebugTOff(void);
extern void D_Close(void);

extern void D_Trace(char * M);
extern void D_Tracenl(char * M);
extern void D_TraceH(char * M, int l);

extern void * Malloc(int s);
extern void Free(void * A);

#endif

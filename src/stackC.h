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
/* stackC.h */

#ifndef __NIFE_STACKC_H__
#define __NIFE_STACKC_H__

extern void IF_stackC_clear(void);
extern void putString(char * S);
extern char * getString(void);
extern int isNString(int n);
extern void IF_dropC(void);
extern void IF_dupC(void);
extern void IF_overC(void);
extern void IF_swapC(void);
extern void IF_catC(void);
extern void IF_catsC(void);
extern void IF_crC(void);
extern void IF_typeC(void);
extern void IF_timeC(void);
extern void IF_dateC(void);
extern void IF_show_stackC(void);
extern void IF_debString(void);

#endif

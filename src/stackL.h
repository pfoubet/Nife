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
/* stackL.h */

#ifndef __NIFE_STACKL_H__
#define __NIFE_STACKL_H__

#define TRUE  ((bool)1)
#define FALSE ((bool)0)

extern void IF_stackL_clear(void);
extern void putBool(bool);
extern bool getBool(void);
extern void negBool(void);
extern void IF_dropL(void);
extern void IF_dupL(void);
extern void IF_swapL(void);
extern void IF_overL(void);
extern void IF_typeL(void);
extern void IF_andL(void);
extern void IF_orL(void);
extern void IF_xorL(void);
extern void IF_true(void);
extern void IF_false(void);
extern void IF_show_stackL(void);

#endif

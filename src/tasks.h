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
/* tasks.h */

#ifndef __NIFE_TASKS_H__
#define __NIFE_TASKS_H__

extern int ITASK;

extern void IF_NewTask(void);
extern void IF_show_Tasks(void);
extern void IF_statusTask(void);
extern void IF_stopTask(void);
extern void IF_delTask(void);
extern void IF_showCons(void);

extern int MakeTask(void * A);

#endif

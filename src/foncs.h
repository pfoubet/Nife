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
/* liste des fonctions systeme */

#ifndef __NIFE_FONCS_H__
#define __NIFE_FONCS_H__
extern long long Nife_time(struct timeval);

extern void IF_exit(void);
extern void IF_time(void);
extern void IF_sleep(void);
extern void IF_sh(void);
extern void IF_toCsv(void);
extern void IF_yXgraph(void);
extern void IF_ytXgraph(void);
extern void IF_xyXgraph(void);
extern void IF_xytXgraph(void);
extern void IF_resUsage(void);

extern void runCommand( char * C);

#endif

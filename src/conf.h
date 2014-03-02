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
/* conf.h */
#ifndef __NIFE_CONF_H__
#define __NIFE_CONF_H__

#ifdef HAVE_CONFIG_H
#include "../config.h"
#else
#define VERSION "0.44"
#endif

#ifdef HAVE_COMEDILIB_H
#define _WITH_COMEDILIB
#endif

/*
#ifdef HAVE_PTHREAD_H
#define _MULTI_THREADING_
#define _REENTRANT
#endif
*/

#endif

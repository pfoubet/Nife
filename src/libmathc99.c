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
/* libmathc99.c */
#include "conf.h"

#define _ISOC99_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <inttypes.h>
#include <math.h>
#include "stackN.h"

void IF_abs(void) { IF_fctB_1(llabs,fabs); }


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
/* libmath.c */
#include "conf.h"

#include <stdio.h>
#include <stdlib.h>
#include <inttypes.h>
#include <math.h>
#include "stackN.h"

static double inv(double a)
{
double v;
   if (a==0.0) v=HUGE;
   else v = (double)1.0/a;
   return v;
}

static long long L_round(double v) { return (long long)rint(v); }
static long long L_floor(double v) { return (long long)floor(v); }
static long long L_ceil(double v) { return (long long)ceil(v); }
static long long L_sgn(double v) 
{
   if (v > 0.0) return (long long)1;
   if (v < 0.0) return (long long)-1;
   return (long long)0;
}

void IF_pi(void)
{
    putDouble(M_PI);
}


void IF_inv(void) { IF_fctD_1(inv); }
void IF_sqrt(void) { IF_fctD_1(sqrt); }
void IF_cbrt(void) { IF_fctD_1(cbrt); }
void IF_round(void) { IF_fctD_1L(L_round); }
void IF_floor(void) { IF_fctD_1L(L_floor); }
void IF_ceil(void) { IF_fctD_1L(L_ceil); }
void IF_sgn(void) { IF_fctD_1LB(L_sgn); }

/* fct trigonometriques */
void IF_sin(void) { IF_fctD_1(sin); }
void IF_asin(void) { IF_fctD_1(asin); }
void IF_cos(void) { IF_fctD_1(cos); }
void IF_acos(void) { IF_fctD_1(acos); }
void IF_tan(void) { IF_fctD_1(tan); }
void IF_atan(void) { IF_fctD_1(atan); }
void IF_sinh(void) { IF_fctD_1(sinh); }
void IF_asinh(void) { IF_fctD_1(asinh); }
void IF_cosh(void) { IF_fctD_1(cosh); }
void IF_acosh(void) { IF_fctD_1(acosh); }
void IF_tanh(void) { IF_fctD_1(tanh); }
void IF_atanh(void) { IF_fctD_1(atanh); }

/* fct logarithmiques et exponentielles */
void IF_ln(void) { IF_fctD_1(log); }
void IF_log(void) { IF_fctD_1(log10); }
void IF_exp(void) { IF_fctD_1(exp); }


/* fonctions de Bessel */
void IF_j0(void) { IF_fctD_1(j0); }
void IF_j1(void) { IF_fctD_1(j1); }
void IF_y0(void) { IF_fctD_1(y0); }
void IF_y1(void) { IF_fctD_1(y1); }


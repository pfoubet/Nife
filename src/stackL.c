/* Copyright (C) 2011-2022  Patrick H. E. Foubet - S.E.R.I.A.N.E.

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
/* stackL.c */
#include "conf.h"

#include <stdio.h>
#include <stdlib.h>

#include "nife.h"
#include "mth.h"
#include "err.h"
#include "debug.h"
#include "stackL.h"


void IF_stackL_clear(void)
{
    _MODIF_i_StackL_(0);
}

void putBool(bool B)
{
int i;
    i=i_StackL;
    stackL[i++] = B;
    _MODIF_i_StackL_(i);
    if (i == LSTACKL) stopErr("putBool",NULL);
}

void IF_dupL(void)
{
int i;
    i=i_StackL;
    if (i) {
       putBool(stackL[i-1]);
    } else messErr(5);
}

void IF_swapL(void)
{
bool B;
int i;
    i=i_StackL;
    if (i > 1) {
       B = stackL[i-1];
       stackL[i-1] = stackL[i-2];
       stackL[i-2] = B;
    } else messErr(20);
}

void IF_andL(void)
{
bool B;
int i;
    i=i_StackL;
    if (i > 1) {
       B = stackL[i-1] & stackL[i-2];
       stackL[i-2] = B;
       i--;
       _MODIF_i_StackL_(i);
    } else messErr(20);
}

void IF_orL(void)
{
bool B;
int i;
    i=i_StackL;
    if (i > 1) {
       B = stackL[i-1] | stackL[i-2];
       stackL[i-2] = B;
       i--;
       _MODIF_i_StackL_(i);
    } else messErr(20);
}

void IF_xorL(void)
{
bool B;
int i;
    i=i_StackL;
    if (i> 1) {
       if (stackL[i-1] == stackL[i-2]) B=FALSE;
       else B = TRUE;
       stackL[i-2] = B;
       i--;
       _MODIF_i_StackL_(i);
    } else messErr(20);
}

void IF_overL(void)
{
int i;
    i=i_StackL;
    if (i > 1) {
       putBool(stackL[i-2]);
    } else messErr(20);
}

bool getBool(void)
{
int i;
    i=i_StackL;
    if (i) {
       i--;
       _MODIF_i_StackL_(i);
       return(stackL[i]);
    } else messErr(5);
    return -1;
}

void IF_typeL(void)
{
int i;
    i=i_StackL;
    if (i) {
       i--;
       _MODIF_i_StackL_(i);
       if(stackL[i]) printf("true\n");
       else printf("false\n");
    } else messErr(5);
}

void negBool(void)
{
int i;
    i=i_StackL;
    if (i) {
       i--;
       if(stackL[i]) stackL[i]= FALSE;
       else          stackL[i]= TRUE;
    } else messErr(5);
}

void IF_true(void)
{
	putBool(TRUE);
}

void IF_false(void)
{
	putBool(FALSE);
}

void IF_dropL(void)
{
	getBool();
}

void IF_show_stackL(void)
{
int i,j=0,I;
char s;
    I=i_StackL;
    for(i=I-1;i>=0;i--) {
       if (stackL[i]) printf(" TRUE");
       else printf(" FALSE");
       if (j==0) printf(" <- top");
       printf("\n");
       j++;
       if (j==NBLIG) break;
    }
    if (i>0) {
       if (i==1) s=' ';
       else s='s';
       printf(" ... and %d other%c boolean%c !\n",I-NBLIG,s,s);
    } else printf("<end of logical stack>\n");
}

void IFD_show_stackL(void)
{
    _IFD_BEGIN_
    IF_show_stackL();
    _IFD_END_
}

void dump_stackL(int fd)
{
uint32_t n;
    n = i_StackL;
    write(fd, (void*)&n, sizeof(n));
    if (n) write(fd, (void*)stackL, n);
    dump_rest_pr(0,n,"logical");
}

void restore_stackL(int fd)
{
uint32_t n=0;
    if (read(fd, (void*)&n, sizeof(n)) != sizeof(n)) return;
    IF_stackL_clear();
    if (n) {
        read(fd, (void*)stackL, n);
        _MODIF_i_StackL_(n);
    }
    dump_rest_pr(1,n,"logical");
}

/* gestion des meta-stacks */

void IF_new_stackL(void)
{
  if (G_i_TStackL == LSTACKS) {
       messErr(60); return;
  }
  G_TiStackL[G_i_TStackL] = i_StackL;
  G_TStackL[G_i_TStackL++] = stackL;
  stackL = G_TStackL[G_i_TStackL];
  i_StackL = G_TiStackL[G_i_TStackL];
  if (stackL == (bool *)0) {
     if ((stackL = (bool*)malloc(sizeof(bool*)*LSTACKL)) == NULL)
          stopErr("IF_new_stackL","malloc");
     i_StackL=0;
  }
}

void IF_old_stackL(void)
{
  if (G_i_TStackL == 0) {
       messErr(61); return;
  }
  G_TiStackL[G_i_TStackL] = i_StackL;
  G_TStackL[G_i_TStackL--] = stackL;
  stackL = G_TStackL[G_i_TStackL];
  i_StackL = G_TiStackL[G_i_TStackL];
}


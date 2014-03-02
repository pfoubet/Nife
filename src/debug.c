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
/* debug.c */
#include "conf.h"

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include "nife.h"
#include "mth.h"
#include "debug.h"

int Debug=1;	/* OK by default */

void D_Reset(void)
{
int fd;
char NF[24];
   chdir(".nife");
   if (Debug) {
     sprintf(NF,".nife_%d.log",getpid());
     if ((fd=open(NF,O_CREAT|O_RDWR|O_TRUNC,0644)) < 0) perror(NF);
     else {
      dup2(fd,2);
      close(fd);
     }
   } else dup2(1,2);
   chdir("..");
}

void D_Update(void)
{
   if (Debug) Debug=0;
   else Debug=1;
   D_Reset();
}

void D_Trace(char * M)
{
   if (Debug) {
      fprintf(stderr," %s",M);
      fflush(stderr);
   }
}

static char D_Mes[20];
void * Malloc(int s)
{
void * M;
    D_Trace("\n");
    M=malloc(s);
    sprintf(D_Mes,"Malloc %d : %lx\n",s,(long)M);
    D_Trace(D_Mes);
    return M;
}

void Free(void *A)
{
    D_Trace("\n");
    sprintf(D_Mes,"Free      : %lx\n",(long)A);
    D_Trace(D_Mes);
    free(A);
}


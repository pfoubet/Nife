/* Copyright (C) 2011-2016  Patrick H. E. Foubet - S.E.R.I.A.N.E.

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
#include "foncs.h"
#include "debug.h"
#include "stackC.h"

int Debug=1;	/* OK by default */
int InDebugFct=0;

static void D_Logname(char*B)
{
  switch(Debug) {
   case 1:
     sprintf(B,".nife_%d.log",getpid());
     break;
   case 2:
     sprintf(B,".nifeNS_%d.log",getpid());
     break;
   case 3:
     sprintf(B,".nifeRPC_%d.log",getpid());
     break;
   default:
     *B='\0';
  }
}

void D_Reset(void)
{
int fd, nc;
char NF[24];
   if (Debug) {
     nc=chdir(".nife"); 
     D_Logname(NF);
     if ((fd=open(NF,O_CREAT|O_RDWR|O_TRUNC,0644)) < 0) perror(NF);
     else {
      dup2(fd,2);
      close(fd);
     }
     nc=chdir("..");
   } else dup2(1,2);
}

void IFD_SaveLog(void)
{
char NF[24],comm[50],*newf;
   newf = getString();
   if (Debug) {
      D_Logname(NF);
      sprintf(comm,"cp .nife/%s %s",NF, newf);
      runCommand(comm);
   }
}

static void DebugTerms(char c)
{
char comm[24];
   if (Debug) {
      sprintf(comm,"ex/NifeDebugTerms -%c", c);
      runCommand(comm);
   }
}

void IFD_DebugTOn(void)
{
   DebugTerms('r');
}

void IFD_DebugTOff(void)
{
   DebugTerms('s');
}

void IFD_Update(void)
{
   if (Debug) Debug=0;
   else Debug=1;
   D_Reset();
}

void D_Trace(char * M)
{
   if (Debug) {
      fprintf(stderr,"%s ",M);
      fflush(stderr);
   }
}

void D_Close(void)
{
int nc;
char NF[24];
   fclose(stderr);
   if (Debug) {
      nc=chdir(".nife"); 
      D_Logname(NF);
      unlink(NF);
      nc=chdir(".."); 
   }
}

void D_Tracenl(char * M)
{
   if (Debug) {
      fprintf(stderr,"%s\n",M);
      fflush(stderr);
   }
}

void D_TraceH(char * M, int l)
{
int i;
   if (Debug) {
      fprintf(stderr,"HEX:");
      for (i=0;i<l;i++) fprintf(stderr," %x",(int)((unsigned char)M[i]));
      fprintf(stderr,"\n");
      fflush(stderr);
   }
}

static char D_Mes[20];
void * Malloc(int s)
{
void * M;
    D_Tracenl("EOL");
    M=malloc(s);
    sprintf(D_Mes,"Malloc %d : %lx",s,(long)M);
    D_Tracenl(D_Mes);
    return M;
}

void Free(void *A)
{
    D_Tracenl("EOL");
    sprintf(D_Mes,"Free      : %lx",(long)A);
    D_Tracenl(D_Mes);
    free(A);
}


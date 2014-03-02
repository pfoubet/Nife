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
#include "conf.h"
/* err.c */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "nife.h"
#include "mth.h"
#include "err.h"
#include "histo.h"
#include "stackF.h"
#include "stackN.h"
#include "stackL.h"
#include "stackC.h"

char * InExec;
static int ERROR=0, LASTERR=0;
static void *ADDRERR=VIDE, *ADDRONE=VIDE;

void tellOnErr (void*A)
{
   ADDRONE=A;
   if (A != VIDE) LASTERR=0;
}

void majLastErr(void*A)
{
/*    if (LASTERR != ERROR) {*/
      LASTERR=ERROR;
      ADDRERR=A;
/*    }*/
}

void razErr(void)
{
   ERROR=0;
}

int noErr(void)
{
   return ERROR;
}

static char *TabErr[] = {
	"No error",
	"no such error",
	"empty stack",
	"sizes are differents",
	"not enought elements on the stack",
	"empty logical stack",
	"empty character stack",
	"empty function stack",
	"function code is present",
	"string too long",
	"command not found",				/* 10 */
	"code type invalid",
	"need one array on top",
	"only allowed in a function code",
	"IF instruction missing",
	"already in function",
	"loading error",
        "no comedilib",
        "invalid argument",
	"not enought elements on the character stack",
	"not enought elements on the logical stack",	/* 20 */
	"function not found",
	"BEGIN instruction missing",
	"empty variable stack",
	"variable not found",
	"not possible in function code",
	"no more task possible",
	"task is running",
	"task not exists",
	"value must be greater than zero",
	"value must be greater than 1",			/* 30 */
	"create socket error",
	"net is off",
	"gethostbyname error, server not found",
	"start server error",
	"net message error",
	"need a scalar on the top",
	"not possible with Universal NetKey",
	"not a Sister Chip System",
	"DO instruction missing",
	"net protocol error",                           /* 40 */
	"network system unknown",
	"gnuplot not found",
	"file open error",
	"back compile limit missing",
	"too many loads",
	"label onerr: already found",
	"label end: already found",
	"inside compilation aborted"
};
#define ERR_MAX 48

void stopErr(char *M, char *F)
{
    fprintf(stderr,"ERREUR : %s !!\n",M);
    if (F != NULL) perror(F);
    if (inSonProc) exit(1);
    termReset();
    exit(1);
}

static void traiteErr(int n, char * L)
{
    ERROR=n;
    if (D_Cod==0) {
       if (ECHOOFF) printf("\n");
       printf("%s : %s !!\a\n",L,TabErr[n]);
    }
    if (inSonProc) exit(1);
    if (fctEnCours) {
        if (D_Cod==0) printf("Compilation aborted !\n");
        else
           if (ADDRONE == VIDE) printf("Inside compilation aborted !\n");
        _MODIF_fctEnCours_(0);
        rmLastFct();
    }
    if (ADDRONE != VIDE) return;
    if (FD_IN) {
        printf("In loading file %s : line %d !\n", getFDname(),getFDlig());
        closeFD();
    }
    if (iTERM) {
      printf("In loading stdin : line %d !\n", getFDlig());
      close(FD_IN);  /* pipe ou autre */
      dup(iTERM); /* stdin on term */
      iTERM = 0;
    }
}

void messErr(int n)
{
char M[50];
    if ((n < 0) || (n > ERR_MAX)) n = 1;
    sprintf(M,"Error in '%s'",InExec);
    traiteErr(n,M);
}

void messErr2(int n, char * M)
{
    if ((n < 0) || (n > ERR_MAX)) n = 1;
    traiteErr(n,M);
}

void IF_NoError(void)
{
   putLong((long)LASTERR);
}

void IF_LibError(void)
{
long P;
   getParLong(&P);
   putString(TabErr[(int)P]);
}

void IF_IsError(void)
{
   if (LASTERR) putBool(TRUE);
   else putBool(FALSE);
}

void IF_showError(void)
{
   if (ERROR) 
      printf("%s : %s !!\n",InExec,TabErr[ERROR]);
   else
      printf("%s : %s.\n",codByAddr(ADDRERR),TabErr[LASTERR]);
}


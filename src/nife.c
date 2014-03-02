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
/* nife.c */
#include "conf.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

#include "nife.h"
#include "mth.h"
#include "err.h"
#include "lib.h"
#include "stackC.h"
#include "stackF.h"
#include "histo.h"
#include "tasks.h"
#include "debug.h"
#include "net.h"
#include "gplot.h"

static char sepa[] = " \t\n";

void putTrSuite(void (*f)(char*))
{
int i;
    i=iTS;
    traiteSuite[i++]=f;
    _MODIF_iTS_(i);
    if (iTS==NBTRSUITE) fprintf(stderr,"traiteSuite limit raise !\n");
}
void dropTrSuite(void)
{
int i;
   i=iTS-1;
   _MODIF_iTS_(i);
   if (iTS < 0) fprintf(stderr,"traiteSuite index negative !\n");
}
PFC getTrSuite(void)
{
    if (iTS<1) return (PFC)NULL;
    else return(traiteSuite[iTS-1]);
}

void interInfos(char *F, char*P)
{
   fprintf(stderr, " Error in %s ( %s ) !!\n",F,P);
   if (errno) perror(F);
   if (inSonProc) {
      sleep(2); exit(1);
   }
}

void Interrupt(int S)
{
int status;
   switch(S) {
   case SIGCHLD :
       if(WAITPID) return;
       while (waitpid(-1, &status, WNOHANG) > 0);
       return;
       break;
   case SIGSEGV :
       printf("Segmentation Error !!\n");
       exit(1);
       break;
   case SIGPIPE :
       printf("Pipe is broken");
       break;
   case SIGFPE :
       printf("Floating Point");
       break;
   case SIGALRM :
       printf("Compilation");
       break;
   default :
       printf("Signal %d",S);
       break;
   }
   siglongjmp(ENV_INT,1);
}

void IF_about(void)
{
char Lib[8];
    *Lib='\0';
#ifdef _MULTI_THREADING_
    strcpy(Lib,"mt-");
#endif
    printf("nife (Networking Industrial Forth-like Environment) - version %s%s-%ld/%ld\n\t (c) S.E.R.I.A.N.E. 2009-13\n",Lib,VERSION,sizeof(long)*8,sizeof(double)*8);
}

int isSepa(char c, int m)
{
unsigned int i;
    if (m == 1) /* '\0 fait partie du lot */
       if (c == (char)'\0') return 1;
    for (i=0; i<strlen(sepa);i++)
        if (c == sepa[i]) return 1;
    return 0;
}

int traiteMot(char *M)
{
int Err=0;
PFC tS;
    if (sigsetjmp(ENV_INT,1)) {
       interInfos("traiteMot",M);
       return 1;
    }
    /*  printf("traiteMot <%s> iTS=%d\n",M,iTS); */
    tS = getTrSuite();
    if (tS != (PFC)NULL) tS(M);
    else
       if (! execLib(M))  { Err=1; messErr2(10,M); }
    if (ITASK) exit(0); /* non interpretation in task ! */
    return Err;
}

static void traiteLigne(char *b)
{
char *mot, *d, *f, *w;
   d=b; f=b+strlen(d);
#ifdef DEBUG
     printf("traiteLigne : <%s>\n",d);
#endif
   while (d<f) {
       if (noErr()) break;
       /* recherche du 1er mot */
       if (stringEnCours) {
         mot = d;
         while (1) {
            if((d = strchr(d,'"')) == NULL) {
               d=mot+strlen(mot);
               break;
            }
            if (*(d-1) == '\\') {
               w = d-1;
               while (*w != '\0') {
                   *w = *(w+1);
                   w++;
               }
               continue;
            }
            d++;
            if (!isSepa(*d,1)) continue;
            break;
         }
       }  else {
         /* on ignore les commentaires */
         if ((mot = strchr(d, (int)'#')) != NULL) {
                *mot = '\0';
                f = mot;
         }
         while (isSepa(*d,0)) d++; /* on avance tant que separateurs */
         mot = d;
         while (!isSepa(*d,1)) d++; /* on avance si nonSepa ET non \0 */
       }
       *d++ = '\0';  /* fin de la commande */
       if (strlen(mot)>0)
          if (traiteMot(mot)) break; /* abort if error */
   }
}

void compileFile(char * f)
{
FILE *F;
int i=0;
    if ((F = fopen(f,"r")) != NULL) {
       while (fgets(bufP, LBUF,F)) {
          if (noErr()) {
             printf("In file %s line %d !\n",f,i);
             break;
          }
          traiteLigne(bufP);
          i++;
       }
       fclose(F);
    }
}

static void lectFic(char *L)
{
int fd;
    dropTrSuite();
    if ((fd = open(L,O_RDONLY)) == -1) {
        perror(L);
        messErr(16);
    } else addFD(fd,L);
}

void IF_LoadCS(void)
{
char * f;
    f = getString();
    if (f != NULL) {
       compileFile(f);
       free((void*)f);
    }
}

void IF_ExecCS(void)
{
char * f;
    f = getString();
    if (f != NULL) {
       if (strlen(f)>0) traiteLigne(f);
       free((void*)f);
    }
}

void * makeFunction(char * f)
{
void *M;
   if ((M = malloc(strlen(f)+8)) == NULL) stopErr("makeFunction","malloc");
   sprintf((char*)M,": _f %s ;",f);
   traiteLigne((char*)M);
   free(M);
   if (noErr() == 0) {
       M = fctByName("_f");
       return M;
   }
   messErr(48);
   return VIDE;
}

void IF_ExecCSf(void)
{
char * f;
void *C;
    f = getString();
    if (f != NULL) {
       C = VIDE;
       if (strlen(f)>0) C = makeFunction(f);
       free((void*)f);
       if (C != VIDE) {
          IF_execFct("_f");
          rmLastFct();
       }
    }
}

void IF_Load(void)
{
      putTrSuite(lectFic);
}

int main(int N, char *P[])
{
int n;
char *dirW = ".nife";
    if (N > 2) {
       fprintf(stderr,"nife [nif-file]\n");
       return(1);
    }
    if ((sizeof(void*) != sizeof(long)) ||
       (sizeof(double) != sizeof(long long))) {
       fprintf(stderr,"Nife open-source don't runs on these machine !\n");
       return(2);
    }
    signal(SIGUSR1,SIG_IGN);
    signal(SIGINT,Interrupt);
    signal(SIGTERM,Interrupt);
    signal(SIGPIPE,Interrupt);
    signal(SIGCHLD,Interrupt);
    signal(SIGQUIT,Interrupt);
    signal(SIGSEGV,Interrupt);
    signal(SIGFPE,Interrupt);
    signal(SIGALRM,Interrupt);
    /* work in ./.nife for facilities of debugging !! */
    if (chdir(dirW) != 0) {
       if (mkdir(dirW, 0755) == -1) {
          perror("mkdir"); return 1;
       }
       if (chdir(dirW) != 0) {
          perror("chdir"); return 1;
       }
    }
    termInit(); /* may stop if no term found */
    TH_init();
    initLib();
    D_Reset();
    if (N==2) {
       IF_Load();
       lectFic(P[1]);
    } else
      printf("Welcome to Nife : Just stack it !\n");
    while (RUN) {
       if ((FD_IN+iTERM) == 0) {
          printf("> ");
          fflush(stdout);
       }
       razErr();
       if ((n=lireLigne(FD_IN,bufP,bufP2,LBUF)) == -1)
                 printf("Line too long!\n");
       else
          if (n>0) traiteLigne(bufP);
    }
    IF_delAllGP();
    IF_netStopS();
    IF_netOff();
    termReset();
    printf("Bye !\n");
    return 0;
}


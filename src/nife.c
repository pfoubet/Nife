/* Copyright (C) 2011-2019  Patrick H. E. Foubet - S.E.R.I.A.N.E.

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
#include "stackV.h"
#include "stackF.h"
#include "histo.h"
#include "tasks.h"
#include "debug.h"
#include "help.h"
#include "net.h"
#include "gplot.h"

static char sepa[] = " \t\n";
static int SigOn=0; /* only for interractive task */

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
       termReset();
       _exit(1);
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
   case SIGINT :
       if (!SigOn) return;
   default :
       printf("Signal %d !\n",S);
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
    printf("nife (Networking Industrial Forth-like Environment) - version %s%s-%ld/%ld\n\t (c) S.E.R.I.A.N.E. 2009-2015\n",Lib,VERSION,sizeof(long)*8,sizeof(double)*8);
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
    if (!ITASK) SigOn=1;
    /*  printf("traiteMot <%s> iTS=%d\n",M,iTS); */
    tS = getTrSuite();
    if (tS != (PFC)NULL) tS(M);
    else
       if (! execLib(M))  { Err=1; messErr2(10,M); }
    if (ITASK) exit(0); /* non interpretation in task ! */
    if (!ITASK) SigOn=0;
    return Err;
}

static void traiteLigne(char *b, int Ctx)
{
char *mot, *d, *f, *w;
   /* case of sh command : ! */
   if (*b=='!') {
     runCommandT(b+1);
     return;
   }
   d=b; f=b+strlen(d);
#ifdef DEBUG
     printf("traiteLigne : <%s>\n",d);
#endif
   switch(Ctx) {
     case 1 : /* compileFile */
       D_Trace(" #");
       break;
     case 2 : /* IF_ExecCS */
       D_Trace("# ExecCS:");
       break;
     case 3 : /* makeFunction */
       D_Trace("# makeFunction:");
       break;
     default : /* 0 */
       if (getiFD()) D_Trace(" #");
   }
   D_Tracenl(b);
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
          traiteLigne(bufP,1);
          i++;
       }
       fclose(F);
    }
}

struct DumpEnt {
    double V;
    char L[8];
    uint32_t Scs;
};

/* Dump and Restore Nblf : Nife Binary Linkable Format */
#define LENT 20
#define LMARK 3

char * DumpRest_ext(char * L)
{
void * M;
char *F;
   if ((M = malloc(strlen(L)+5)) == NULL) stopErr("DumpRest_ext","malloc");
   F = (char*)M;
   sprintf(F,"%s.nblf",L);
   return F;
}

void dump_marque(int fd, char C)
{
char b[LMARK+1];
   sprintf(b,"<%c>",C);
   if ((write (fd, (void*)b, LMARK)) != LMARK)
       stopErr("dump","marque");
}

void restore_marque(int fd, char C)
{
char b[LMARK+1];
   /* printf("Restore %c ! \n", C); */
   if ((read(fd, (void*)b, LMARK)) == LMARK)
       if (b[1] == C) return;
   stopErr("restore","marque");
}

void rest_links_pr(int i, char *O, char *C)
{
   if (i) {
      printf("Linking %d %s", i, O);
      if (i > 1) printf("s");
      printf(" to %s stack.\n",C);
   }
}

void dump_rest_pr(int T, int N, char * L) /* T=0 dump, T=1 restore */
{
    if (T==0) printf("Dump ");
    printf("%d elt",N);
    if (N>1) printf("s");
    printf(" for %s stack",L);
    if (T) printf(" restored");
    printf(".\n");
}

static void restoreFic(char *L)
{
int fd;
struct DumpEnt E;
char * F;

    dropTrSuite();
    F = DumpRest_ext(L);
    if ((fd = open(F,O_RDONLY)) == -1) {
        perror(F);
        messErr(43);
    } else {
      if (read(fd,(void*)&E, LENT) != LENT) {
        printf("File too small !\n");
        messErr(59);
      } else {
         if (strncmp(E.L,"Nblf010", 7) == 0) {
          if (E.Scs == (long)getScs()) {
            if (E.V == atof(VERSION)) {
               restore_marque(fd, 'N');
               restore_stackN(fd);          
               restore_marque(fd, 'C');
               restore_stackC(fd);          
               restore_marque(fd, 'L');
               restore_stackL(fd);          
               restore_marque(fd, 'V');
               restore_stackV(fd);          
               restore_links_stackN();          
               restore_marque(fd, 'F');
               restore_stackF(fd);          
               restore_marque(fd, 'X');
               restore_links_stackV();          
            } else printf("This file is just available for Nife v %g !\n",E.V);
          } else printf("This file have another SCS !\n");
         } else printf("Not a NBLF File !\n");
         close(fd);
      }
    }
    free((void*)F);
}

static void dumpFic(char *L)
{
int fd;
struct DumpEnt E;
char * F;

    dropTrSuite();
    F = DumpRest_ext(L);
    if ((fd = open(F,O_CREAT|O_WRONLY,0600)) == -1) {
        perror(F);
        messErr(58);
    } else {
      strncpy(E.L,"Nblf010", 7);
      E.V=atof(VERSION);
      E.Scs=(long)getScs();
      if ((write(fd,(void*)&E, LENT)) == LENT) {
         dump_marque(fd, 'N');
         dump_stackN(fd);
         dump_marque(fd, 'C');
         dump_stackC(fd);
         dump_marque(fd, 'L');
         dump_stackL(fd);
         dump_marque(fd, 'V');
         dump_stackV(fd);
         dump_marque(fd, 'F');
         dump_stackF(fd);
         dump_marque(fd, 'X');
         close(fd);
      } else messErr(58);
    }
    free((void*)F);
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
       if (strlen(f)>0) traiteLigne(f,2);
       free((void*)f);
    }
}

void * makeFunction(char * f)
{
void *M;
   if ((M = malloc(strlen(f)+8)) == NULL) stopErr("makeFunction","malloc");
   sprintf((char*)M,": _f %s ;",f);
   traiteLigne((char*)M,3);
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

void IF_Dump(void)
{
      putTrSuite(dumpFic);
}

void IF_Restore(void)
{
      putTrSuite(restoreFic);
}

void IF_Load(void)
{
      putTrSuite(lectFic);
}

int main(int N, char *P[])
{
int n,Ctx;
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
    signal(SIGQUIT,SIG_IGN);
    signal(SIGABRT,SIG_IGN);
    signal(SIGUSR1,SIG_IGN);
    signal(SIGCONT,SIG_IGN);
    signal(SIGSTOP,SIG_IGN);
    signal(SIGTSTP,SIG_IGN);
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
    } else {
      printf("Welcome to Nife : Just stack it !\n");
      IF_helpS();
    }
    while (RUN) {
       if ((FD_IN+iTERM) == 0) {
          printf("> ");
          fflush(stdout);
          Ctx=0;
       } else Ctx=1;
       razErr();
       if ((n=lireLigne(FD_IN,bufP,bufP2,LBUF)) == -1)
                 printf("Line too long!\n");
       else
          if (n>0) traiteLigne(bufP,0);
    }
    IF_delAllGP();
    IF_netStopS();
    IF_netOff();
    D_Close();
    termReset();
    printf("Bye !\n");
    return 0;
}


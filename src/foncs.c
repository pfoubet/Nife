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
/* foncs.c liste des fonctions systeme */
#include "conf.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>
#include <time.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <sys/types.h>
#include <sys/wait.h>

#include "nife.h"
#include "mth.h"
#include "err.h"
#include "foncs.h"
#include "histo.h"
#include "stackN.h"
#include "stackC.h"


void IF_exit(void) { _MODIF_RUN_(0); }

long long Nife_time(struct timeval tv)
{
long long v;
    /* il y a 10958 jours entre le 1/1/1970 et le 1/1/2000 */
    v=(long long)((long)tv.tv_usec) +
      ((long long)(tv.tv_sec - 946771200L)*(long long)1000000);
    return v;
}

void IF_time(void)
{
struct timeval tv;
long long v;
    gettimeofday(&tv,NULL);
    v=Nife_time(tv);
    putLong(v);
}

void IF_sleep(void)
{
long t;
    if (getParLong(&t)) sleep((unsigned int)t);
}

void IF_sh (void)
{
int pid;
char * Sh;
   _MODIF_WAITPID_(1);
   if ((pid = fork()) == -1) stopErr("IF_sh","fork");
   if (pid == 0) { /* fils */
      _MODIF_inSonProc_(1);
      termReset();
      dup2(1,2);
      if ((Sh=getenv("SHELL")) == NULL) execlp("sh", "nife-sh",NULL);
      else execlp(Sh,"nife-sh",NULL);
      perror("sh");
      exit(1);
   } 
   waitpid(pid,NULL,0);
   _MODIF_WAITPID_(0);
   termInit();
   printf("Come back to nife !\n");
}

void runCommand (char * com)
{
int pid;
char * Sh;
   _MODIF_WAITPID_(1);
   if ((pid = fork()) == -1) stopErr("runComm","fork");
   if (pid == 0) { /* fils */
      _MODIF_inSonProc_(1);
      termReset();
      if ((Sh=getenv("SHELL")) == NULL) execlp("sh","sh","-c",com,NULL);
      else execlp(Sh,"sh","-c",com,NULL);
      perror("sh");
      exit(1);
   }
   waitpid(pid,NULL,0);
   _MODIF_WAITPID_(0);
   termInit();
   printf("\n");
}

void IF_toCsv(void)
{
int i, lib=0;
long t;
char *f, *s, *e;
void *M;
FILE * fd;
    if (getParLong(&t)) {
        if (!isNTabSameDim((int)t)) {
           messErr(3);
           return;
        }
        if (!isNString(1)) {
           messErr(6);
           return;
        }
        f = getString();
        s = f;
        e = f + strlen(f);
        while (s < e) {
           if (*s == ';') { *s= '\0'; s++; break; }
           s++;
        }
        if (strlen(s) > 0) lib=1;
        if ((M = malloc(strlen(f)+5)) == NULL) stopErr("IF_toCsv","malloc");
        sprintf((char*)M,"%s.csv",f);
        fd = fopen((char*)M,"w+");
        free(M);
        for (i=0; i<(int)t; i++) {
           if (lib) {
            if (strlen(s) > 0) {
              f = s; 
              while (s < e) {
                 if (*s == ';') { *s= '\0'; s++; break; }
                 s++;
              }
              fprintf(fd,"%s;",f);
            } else 
              fprintf(fd,"lig%d;",i+1);
           }
           IF_inFile_1d(fd, ';', 1);
        }
        fclose(fd);
        free((void*)f);
    }
}

static void lanceXgraph(int mode, int tit)
{
FILE * fd;
char nf[30];
    sprintf(nf,".nife/Nife_%d.gx",getpid());
    fd = fopen(nf,"w+");
    fprintf(fd,"Device: Postscript\nDisposition: To File\nTitleText: ");
    if (tit) fprintf(fd,"%s\n",getString());
    else  fprintf(fd,"Data from Nife %s\n",VERSION);
    if (mode) IF_inFile_2(fd); else IF_inFile_1(fd);
    fclose(fd);
    execlp("xgraph","xgraph",nf,NULL);
}

static void gen_Xgraph (int m, int t)
{
int pid;
    if (m) {
        if (!isNTabSameDim(2)) {
           messErr(3);
           return;
        }
    } else {
        if (!is1Tab()) {
           messErr(12);
           return;
        }
    }
    if (t) {
        if (!isNString(1)) {
           messErr(6);
           return;
        }
    }
    if ((pid = fork()) == -1) stopErr("IF_yXgraph","fork");
    if (pid == 0) { /* fils */
       _MODIF_inSonProc_(1);
       setsid();
       lanceXgraph(m,t);
       perror("xgraph");
       exit(1);
    }
    IF_drop();
    if (m) IF_drop();
    if (t) IF_dropC();
    /* test if xgraph is executed */
    if (kill(pid,SIGWINCH) == -1) 
       if (errno == ESRCH) messErr(10);
}

void IF_yXgraph (void)
{
    gen_Xgraph(0,0);
}
void IF_ytXgraph (void)
{
    gen_Xgraph(0,1);
}
void IF_xyXgraph (void)
{
    gen_Xgraph(1,0);
}
void IF_xytXgraph (void)
{
    gen_Xgraph(1,1);
}

static void printLimits(char * M,char *U, struct rlimit * L)
{
    printf("Limites %-10s : ",M);
    if (L->rlim_cur == RLIM_INFINITY) printf("infini");
    else printf("%ld",L->rlim_cur);
    if (L->rlim_cur == RLIM_INFINITY) printf("/infini");
    else printf("/%ld",L->rlim_cur);
    printf(" %s\n",U);
}


void IF_resUsage(void)
{
struct rusage R;
struct rlimit L;
    if (getrusage(RUSAGE_SELF,&R) == 0) {
        printf("Temps processus                  : %ld.%.6ld\n",R.ru_utime.tv_sec,R.ru_utime.tv_usec);
        printf("Temps système                    : %ld.%.6ld\n",R.ru_utime.tv_sec,R.ru_utime.tv_usec);
/* non significatif **************
        printf("Taille résidente maximale        : %ld\n",R.ru_maxrss);
        printf("Taille des données non partagées : %ld\n",R.ru_idrss);
        printf("Taille de Pile                   : %ld\n",R.ru_isrss);
**********************************/
        printf("Demandes de pages                : %ld\n",R.ru_minflt);
        printf("Nombre de fautes de pages        : %ld\n",R.ru_majflt);
        printf("Changts de contexte volontaires  : %ld\n",R.ru_nvcsw);
        printf("Changts de contexte involontaires: %ld\n",R.ru_nivcsw);
    } else perror("getrusage");
/*
    if (getrlimit(RLIMIT_AS,&L) == 0) { printLimits("AS","octets",&L);
    } else perror("getrlimit AS");
***************/
    if (getrlimit(RLIMIT_CORE,&L) == 0) { printLimits("CORE","octets",&L);
    } else perror("getrlimit CORE");
    if (getrlimit(RLIMIT_CPU,&L) == 0) { printLimits("CPU","sec.",&L);
    } else perror("getrlimit CPU");
/* not in Solaris ***************
    if (getrlimit(RLIMIT_RSS,&L) == 0) { printLimits("RSS","pages",&L);
    } else perror("getrlimit RSS"); */
    if (getrlimit(RLIMIT_DATA,&L) == 0) { printLimits("DATA","pages",&L);
    } else perror("getrlimit DATA");
    if (getrlimit(RLIMIT_STACK,&L) == 0) { printLimits("STACK","octets",&L);
    } else perror("getrlimit STACK");
/* not in Solaris ***************
    if (getrlimit(RLIMIT_NPROC,&L) == 0) { printLimits("NPROC","processus",&L);
    } else perror("getrlimit NPROC"); */
    if (getrlimit(RLIMIT_NOFILE,&L) == 0) { printLimits("NOFILE","file desc.",&L);
    } else perror("getrlimit NOFILE");
    
}


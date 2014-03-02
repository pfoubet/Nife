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
/* task.c  gestion des taches */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <signal.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <fcntl.h>

#include "nife.h"
#include "mth.h"
#include "err.h"
#include "tasks.h"
#include "foncs.h"
#include "histo.h"
#include "stackN.h"
#include "stackF.h"
#include "stackL.h"

int ITASK=0;  /* no de tache */
#define MAX_TASK   10
static pid_t  TASKp[MAX_TASK]; /* pid */
static void * TASKf[MAX_TASK]; /* func */

void IF_NewTask (void)
{
int i;
   for (i=0;i<MAX_TASK;i++) if (TASKp[i]==0) break;
   if (i<MAX_TASK) {
      FctInTask = i+1;
   } else {
     FctInTask = -1;
     messErr(26);
   }
}

static char FLib[24];
static char * FicCons(int t)
{
    sprintf(FLib,".nife/Cons%d.log",t);
    return FLib;
}

int MakeTask (void * A)
{
int i, pid;
char * NF;
      i = FctInTask-1;
      if ((pid = fork()) == -1) stopErr("IF_NewTask","fork");
      if (pid == 0) { /* fils */
         ITASK=FctInTask;  /* TASK 0 is the interractive */
         NF = FicCons(ITASK);
         if ((i=open(NF,O_CREAT|O_RDWR|O_TRUNC,0600)) < 0) perror(NF);
         else {
            dup2(i,1);
            dup2(i,2);
            close(i);
            close(0);
         }
      } else {
         TASKp[i] = pid;
         putLong(FctInTask);
         TASKf[i]=A;
         FctInTask=0;
      }
      return(pid);
}

void IF_show_Tasks(void)
{
int i;
   for (i=0;i<MAX_TASK;i++) {
      if (TASKp[i] != 0) {
         printf(" %.2d  :  %s  (",i+1,codByAddr(TASKf[i]));
         if (kill(TASKp[i],SIGUSR1) ==0) printf("running");
         else printf("stopped");
         printf(")\n");
      }
   }
   printf("<end of tasks list>\n");
}

void IF_statusTask(void)
{
long V;
int i;
   if (getParLong(&V)) {
      i = V -1;
      if (TASKp[i] != 0) {
         if (kill(TASKp[i],SIGUSR1)==0) {
            putBool(TRUE);
            return;
         }
      }
   }
   putBool(FALSE);
}

void IF_stopTask(void)
{
long V;
int i;
   if (getParLong(&V)) {
      i = V -1;
      if (TASKp[i] != 0) {
         _MODIF_WAITPID_(1);
         if (kill(TASKp[i],SIGKILL) == 0)
             waitpid(TASKp[i],NULL,0);
         _MODIF_WAITPID_(0);
      }
   }
}

void IF_delTask(void)
{
long V;
int i;
   if (getParLong(&V)) {
      i = V -1;
      if (TASKp[i] != 0) {
         if (kill(TASKp[i],SIGUSR1)==0) {
            messErr(27);
            return;
         }
      }
   }
   TASKp[i] = 0;
}

void IF_showCons( void)
{
long V;
int i;
char * NF, comm[30];
   if (getParLong(&V)) {
      i = V -1;
      if (TASKp[i] != 0) {
         NF = FicCons((int)V);
         sprintf(comm,"more %s",NF);
         runCommand(comm);
      }
      else  messErr(28);
   }
}



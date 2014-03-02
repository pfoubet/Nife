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
/* liste des fonctions liees a gnuplot */
/* gplot.c */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include "gplot.h"
#include "nife.h"
#include "mth.h"
#include "stackN.h"
#include "stackC.h"
#include "err.h"

#define GPO_STD	0	/* standard */
#define GPO_MRG	0x100	/* merge */

struct GPlot {
    short id;
    short op;/* options + nb plots */
    FILE *p; /* pipe */
    char *t; /* title */
    char *f; /* file */
    char *a; /* command append */
    int pid; /* pid */
    void *n; /* next */
};

static short GPid=0;

static void *stackGP = VIDE;

/* file manipulations */
static void GPF_init(char *f)
{
int fd;
   chdir(".nife");
   if ((fd = creat(f,0600)) != -1) {
      write(fd,"# Nife auto-generated GNUplot file !\n",37);
      close(fd);
   }
   chdir("..");
}

static void GPF_del(char *f)
{
    unlink(f);
}

static void GPF_supL(char * f, int n)
{
void * M;
char *f2;
char l[512];
FILE *fIn, *fOut;
int i=1;
   if ((M = malloc(strlen(f)+5)) == NULL) stopErr("GPF_supL","malloc");
   f2 = (char*)M;
   strcpy(f2,f);
   f2 += strlen(f);
   strcpy(f2,".wni");
   f2 = (char*)M;
   if ((fIn = fopen(f, "r")) != NULL) {
      if ((fOut = fopen(f2, "w")) != NULL) {
         while (fgets(l, sizeof(l), fIn)) {
            if (i != n) fputs(l, fOut);
            i++;
         }
         fclose(fOut);
      }
      fclose(fIn);
      rename(f2, f);
   }
   free(M);
}

/* struct GPlot functions */

static void eraseGP(struct GPlot *F)
{
   GPF_del(F->f);
   free((void*)F->t);
   free((void*)F->f);
   if (F->a != NULL) free((void*)F->a);
   free((void*)F);
}

static void GP_initial(struct GPlot * N)
{
   fprintf(N->p, "set term x11 title \"Nife GPlot %d\" size 400,300\n", N->id);
   fprintf(N->p, "unset mouse\n");
}

static void GP_create(short op, char *T, char *F)
{
int pid, p[2];
void * M;
struct GPlot *N;
   if ((M = malloc(sizeof(struct GPlot)))==NULL) stopErr("GP_create","malloc");
   N = (struct GPlot*)M;
   N->id = ++GPid;
   N->op = op;
   N->t = T;
   N->f = F;
   N->a = NULL;
   N->n = stackGP;
   GPF_init(N->f);
   /* Old method **********
   N->p = popen("gnuplot -p -raise","w");
   *********/
   if (pipe(p) != 0)  stopErr("GP_create","pipe");
   N->p = fdopen(p[1], "w"); /* w side */
   if ((pid = fork()) == -1) {
      fprintf(stderr,"GP_create : error fork !\n");
      eraseGP(N);
   } else {
     if (pid == 0) { /* fils */
       dup2(p[0],0);
       close(p[0]);
       close(p[1]);
       _MODIF_inSonProc_(1);
       setsid();
       execlp("gnuplot", "Nife_gplot", "-p", "-raise", NULL);
       perror("gnuplot");
       exit(1);
     } else {
       N->pid = pid;
       close(p[0]);
       stackGP = M;
       GP_initial(N);
     }
   }
}

static void GP_del(short id)
{
void ** PNext;
struct GPlot * N;
    PNext = &stackGP;
    while (*PNext != VIDE) {
       N = (struct GPlot*) *PNext;
       if (N->id == id) {
           *PNext = N->n;
           /* stop gnuplot */
           fprintf(N->p,"exit\n");
           fflush(N->p);
           kill(N->pid,SIGKILL);
           eraseGP(N);
           return;
       }
       PNext = &N->n;
    }
    messErr(42);
}

void IF_delAllGP(void)
{
struct GPlot * N;
    while (stackGP != VIDE) {
       N = (struct GPlot*) stackGP;
       stackGP = N->n;
       /* stop gnuplot */
       fprintf(N->p,"exit\n");
       fflush(N->p);
       kill(N->pid,SIGKILL);
       eraseGP(N);
    }
    GPid=0;
}

static struct GPlot * GP_getPlot(short id)
{
void ** PNext;
struct GPlot * N;
    PNext = &stackGP;
    while (*PNext != VIDE) {
       N = (struct GPlot*) *PNext;
       if (N->id == id) {
           return N;
       }
       PNext = &N->n;
    }
    messErr(42);
    return NULL;
}


static void GP_plot(struct GPlot * N)
{
int i;
   fprintf(N->p, "set term x11 title \"Nife GPlot %d\"\n", N->id);
   fprintf(N->p, "plot ");
   for (i=0; i<(N->op&0xFF); i++) {
      if (i) fprintf(N->p,", ");
      fprintf(N->p,"\"%s\" using 1:%d title \"",N->f, i+2);
      if (i) fprintf(N->p,"col %d",i+1);
      else   fprintf(N->p,"%s",N->t);
      fprintf(N->p,"\" with lines ");
      if (N->a != NULL) fprintf(N->p,"%s",N->a);
   }
   fprintf(N->p,"\n");
   fflush(N->p);
}

static void GP_updApp(short id, char * a)
{
struct GPlot * N;
    if ((N = GP_getPlot(id)) != NULL) {
       if (N->a != NULL) free((void*)N->a);
       N->a = a;
       /* redraw gnuplot */
       GP_plot(N);
       return;
    }
    free((void*)a);
}

static void GP_addData(short id)
{
struct GPlot * N;
FILE *fd;
    if ((N = GP_getPlot(id)) != NULL) {
       /* Add data to file */
       fd = fopen(N->f,"a");
       IF_inFile_1d(fd, ' ', 0);
       fclose(fd);
       /* redraw gnuplot */
       GP_plot(N);
    }
}

static void GP_replData(short id)
{
struct GPlot * N;
FILE *fd;
    if ((N = GP_getPlot(id)) != NULL) {
       /* delete the second line */
       GPF_supL(N->f,2);
       /* Add data to file */
       fd = fopen(N->f,"a");
       IF_inFile_1d(fd, ' ', 0);
       fclose(fd);
       /* redraw gnuplot */
       GP_plot(N);
    }
}

void IF_gplot_new(void)
{
char *t, *f;
   if (!isNString(2)) {
        messErr(6);
        return;
   }
   t = getString();
   f = getString();
   GP_create(GPO_STD|1,t,f);
}

void IF_gplot_newM(void)
{
char *t, *f;
long n;
  if (getParLong(&n)) {
     if (!isNString(2)) {
        messErr(6);
        return;
     }
     t = getString();
     f = getString();
     GP_create(GPO_STD|(short)n,t,f);
  }
}

void IF_gplot_del(void)
{
long t;
   if (getParLong(&t)) {
       GP_del((short)t);
   }
}

void IF_gplot_clear(void)
{
long t;
struct GPlot * N;
   if (getParLong(&t)) {
     if ((N = GP_getPlot((short)t)) != NULL) {
       GPF_init(N->f);
       /* redraw gnuplot */
       GP_plot(N);
     }
   }
}

void IF_gplot_commapp(void)
{
long t;
char *a;
   if (getParLong(&t)) {
      if (!isNString(1)) {
        messErr(6);
        return;
      }
      a = getString();
      GP_updApp((short)t,a);
   }
}

void IF_gplot_append(void)
{
long t;
   if (getParLong(&t)) {
      if (!is1Tab()) {
           messErr(12);
           return;
      }
      GP_addData((short)t);
   }
}

void IF_gplot_replace(void)
{
long t;
   if (getParLong(&t)) {
      if (!is1Tab()) {
           messErr(12);
           return;
      }
      GP_replData((short)t);
   }
}

void IF_show_stackGP(void)
{
void * Next;
struct GPlot * N;
    if (stackGP != VIDE)
       printf("  id |NbP|  filename  |       Title             | Options\n");
    Next = stackGP;
    while (Next != VIDE) {
       N = (struct GPlot*) Next;
       printf("%5d|%3d|%-12s|%-25s|%s\n",N->id,N->op&0xFF,N->f, N->t, N->a);
       Next = N->n;
    }
    printf("<end of GNUPlot list>\n");
}





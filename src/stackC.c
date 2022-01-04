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
#include "conf.h"
/* stackC.c */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "nife.h"
#include "mth.h"
#include "err.h"
#include "debug.h"
#include "stackC.h"
#include "stackF.h"

/* #define DEBUG_M */


void putString(char * S)
{
void * M;
int i;
    i = i_StackC;
    if (i == LSTACKC) stopErr("putString",NULL);
    if ((M = malloc(strlen(S)+1)) == NULL) stopErr("putString","malloc");
#ifdef DEBUG_M
    printf("New String address : %lu \n",(unsigned long)M);
#endif
    strcpy((char*)M,S);
    if (fctEnCours) makeFct(T_CHA,M);
    else {
      stackC[i++] = (char*)M;
      _MODIF_i_StackC_(i);
    }
}

char * getString(void)  /* NOT free() !!! */
{
int i;
    i = i_StackC;
    if (i) {
       i--;
       _MODIF_i_StackC_(i);
       return(stackC[i]);
    } 
    messErr(6);
    return NULL;
}

int isNString(int n)
{
    if (i_StackC >= n) return 1;
    return 0;
}


void IF_dropC(void)
{
char * S;
	S=getString();
#ifdef DEBUG_M
    printf("Del String address : %lu \n",(unsigned long)S);
#endif
        if (S != NULL) free((void*)S);
}

static void IF_dupC_i(int i)
{
char * S;
int I;
    I = i_StackC;
    if (I>=i) {
       S = stackC[I-i];
       putString(S);
    } 
    else messErr(19);
}

void IF_dupC(void)
{
    IF_dupC_i(1);
}

void IF_overC(void)
{
    IF_dupC_i(2);
}

void IF_swapC(void)
{
char * S;
int I;
    I = i_StackC;
    if (I>1) {
       S = stackC[I-1];
       stackC[I-1] = stackC[I-2];
       stackC[I-2] = S;
    } 
    else messErr(19);
}

void IF_stackC_clear(void)
{
    while (i_StackC) IF_dropC();
}

static void IF_catC_i(int i)
{
char * S1, * S2, *S;
int l, I;
    I = i_StackC;
    if (I>1) {
       S1 = stackC[I-2];
       S2 = stackC[I-1];
       l = strlen(S1) + strlen(S2) + i + 1;
       if ((S = (char*)malloc(l+1)) == NULL) stopErr("IF_catC_i","malloc");
       strcpy(S,S1);
       if (i) strcat(S, " ");
       strcat(S,S2);
       IF_dropC();
       IF_dropC();
       I = i_StackC;
       stackC[I++]=S;
       _MODIF_i_StackC_(I);
    } 
    else messErr(19);
}

void IF_catC(void)
{
    IF_catC_i(0);
}

void IF_catsC(void)
{
    IF_catC_i(1);
}

void IF_crC(void)
{
    printf("\n");
}

static void Get_Date_Time(int x)
{
struct tm * T;
time_t t0;
char b[12];
    t0 = time(NULL);
    T = localtime(&t0);
    if (x) sprintf(b,"%.2d/%.2d/%.4d",T->tm_mday,T->tm_mon+1,T->tm_year+1900);
    else   sprintf(b,"%.2d:%.2d:%.2d",T->tm_hour,T->tm_min,T->tm_sec);
    putString(b);
}

void IF_dateC(void)
{
    Get_Date_Time(1);
}

void IF_timeC(void)
{
    Get_Date_Time(0);
}

void IF_typeC(void)
{
int i;
    i = i_StackC;
    if (i) {
       printf("%s",stackC[i-1]);
       IF_dropC();
    } 
    else messErr(6);
}

void IF_show_stackC(void)
{
int i,j=0,I;
char s;
    I=i_StackC;
    for(i=I-1;i>=0;i--) {
#ifdef DEBUG_M
       printf(" %.5d : \"%s\" Add=%lu",strlen(stackC[i]),stackC[i],
            (unsigned long)(stackC[i]));
#else
       printf(" %.5d : \"%s\"",(int)strlen(stackC[i]),stackC[i]);
#endif
       if (j==0) printf(" <- top");
       printf("\n");
       j++;
       if (j==NBLIG) break;
    }
    if (i>0) {
       if (i==1) s=' ';
       else s='s';
       printf(" ... and %d other%c string%c !\n",I-NBLIG,s,s);
    } else printf("<end of character stack>\n");
}

void IFD_show_stackC(void)
{
    _IFD_BEGIN_
    IF_show_stackC();
    _IFD_END_
}

void suiteString(char *S)
{
int end=0;
    if ((strlen(bufC)+strlen(S)+1) > MAXSTRING) {
        dropTrSuite();
        _MODIF_stringEnCours_(0);
        messErr(9);
        return;
    }
    if (S[strlen(S)-1] == '"') {
        S[strlen(S)-1] = '\0';
        end=1;
    }
    strcat(bufC,S);
    if (end) {
        dropTrSuite();
        _MODIF_stringEnCours_(0);
        putString(bufC);
    }
}

void IF_debString(void)
{
    bufC[0]='\0';
    putTrSuite(suiteString);
    _MODIF_stringEnCours_(1);
}

void dump_eltC(int fd, char *A)
{
uint32_t l;
      l=strlen(A);
      write(fd, (void*)&l, sizeof(l));
      write(fd, A, l+1);
}

void dump_stackC(int fd)
{
uint32_t n, i, l;
    n = i_StackC;
    write(fd, (void*)&n, sizeof(n));
    for (i=0; i<n; i++) dump_eltC(fd, stackC[i]);
    dump_rest_pr(0,n,"character");
}

char * restore_eltC(int fd)
{
uint32_t l;
void * M;
    read(fd, (void*)&l, sizeof(l));
    if ((M = malloc(l+1)) == NULL) stopErr("restore_eltC","malloc");
    read(fd, M, l+1);
    return (char*)M;
}

void restore_stackC(int fd)
{
uint32_t n=0, i, j;
    if (read(fd, (void*)&n, sizeof(n)) != sizeof(n)) return;
    IF_stackC_clear();
    for (i=0; i<n; i++) {
        j = i_StackC;
        stackC[j++] = restore_eltC(fd);
        _MODIF_i_StackC_(j);
    }
    dump_rest_pr(1,n,"character");
}

/* gestion des meta-stacks */

void IF_new_stackC(void)
{
  if (G_i_TStackC == LSTACKS) {
       messErr(60); return;
  }
  G_TiStackC[G_i_TStackC] = i_StackC;
  G_TStackC[G_i_TStackC++] = stackC;
  stackC = G_TStackC[G_i_TStackC];
  i_StackC = G_TiStackC[G_i_TStackC];
  if (stackC == (char **)0) {
     if ((stackC = (char**)malloc(sizeof(char**)*LSTACKC)) == NULL)
          stopErr("IF_new_stackC","malloc");
     i_StackC=0;
  }
}

void IF_old_stackC(void)
{
  if (G_i_TStackC == 0) {
       messErr(61); return;
  }
  G_TiStackC[G_i_TStackC] = i_StackC;
  G_TStackC[G_i_TStackC--] = stackC;
  stackC = G_TStackC[G_i_TStackC];
  i_StackC = G_TiStackC[G_i_TStackC];
}


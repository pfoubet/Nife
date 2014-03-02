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
/* histo.c */
#include "conf.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <termios.h>
#include <unistd.h>

#include "histo.h"
#include "nife.h"
#include "mth.h"
#include "err.h"
#include "lib.h"
#include "debug.h"


#define HISTOMAX 100
static char * HIST[HISTOMAX];
static int iH=0, iHt, iHl, iHFull=0;

#define FDMAX 128
static int iFD=0;
static int FD_T[FDMAX], FD_L[FDMAX]; /* fd table and line numbers */
static void *FD_N[FDMAX];

void IF_showFD(void)
{
    printf("iFD=%d FD_T=%d FD_L=%d\n",iFD, FD_T[iFD], FD_L[iFD]);
}

void addFD(int fd, char*S)
{
void *M;
    iFD++;
    /* printf("addFD iFD=%d\n",iFD); */
    if (iFD == FDMAX) {
       iFD--;
       messErr(45);
       return;
    }
    _MODIF_FD_IN_(fd);
    FD_T[iFD]=fd;
    FD_L[iFD]=0;
    if ((M = malloc(strlen(S)+1)) == NULL) stopErr("addFD","malloc");
#ifdef DEBUG_M
    printf("New String address : %lu \n",(unsigned long)M);
#endif
    strcpy((char*)M,S);
    FD_N[iFD]=M;
}

void closeFD(void)
{
   /* printf("closeFD iFD=%d\n",iFD); */
   close(FD_IN);
   free(FD_N[iFD]);
   iFD--;
   if (iFD) _MODIF_FD_IN_(FD_T[iFD]);
   else _MODIF_FD_IN_(0);
}

int getFDlig(void)
{
    return FD_L[iFD];
}

char * getFDname(void)
{
    return (char*)FD_N[iFD];
}

void incFDlig(void)
{
    FD_L[iFD]++;
}


void razHisto(void)
{
int i;
   for(i=0;i<iH;i++) free((void*)HIST[i]);
   iH=0;
}

void putHisto(char *C)
{
int L;
    if ((L=strlen(C)) == 0) return;
    HIST[iH] = (char*)malloc(L+1);
    if (HIST[iH] != NULL) {
       strcpy(HIST[iH],C);
       iHt=iH; /* depart recherche */
       iH++;
       iHl=iH;
       if (iH == HISTOMAX) {
         iHFull=1;
         iH=0;
       }
    }
}

char * getHisto(char S) /* Sens A remonte, B descend */
{
char * C;
   if (S == 'B') {
       if (iHt == iH-1) {
           iHl=iH;
           printf("\a");
           fflush(stdout);
           C = NULL;
       } else {
           iHt++;
           if (iHt == HISTOMAX) iHt=0;
           if (iHl==iHt) iHt++;
           C = HIST[iHt];
           iHl=iHt;
       }
   } else { /* A */
       if (iHt == -1) {
           iHl=iHt;
           printf("\a");
           fflush(stdout);
           C = NULL;
       } else {
          if (iHl==iHt) iHt--;
          iHl=iHt;
          C = HIST[iHt--];
          if ((iHt < 0) && (iHFull)) iHt = HISTOMAX-1;
       }
   }
   return(C);
}

static struct termios t0, t1;
int iTERM=0;

void termInit(void)
{
    for (iTERM=0; iTERM<3; iTERM++) {
       if (tcgetattr(iTERM, &t0) != -1)  break;
       if (iTERM<2) continue;
       perror("No terminal found ! tcgetattr");  /* sauvegarde */
       exit(2);
    }
    if (tcgetattr(iTERM, &t1) == -1) perror("tcgetattr"); 
    /* cfmakeraw(&t1); */
    t1.c_iflag &= ~(IGNBRK | BRKINT | PARMRK | ISTRIP
                           | INLCR | IGNCR | ICRNL | IXON);
    /* t1.c_oflag &= ~OPOST; */
    t1.c_lflag &= ~(ECHO | ECHONL | ICANON | ISIG | IEXTEN);
    t1.c_cflag &= ~(CSIZE | PARENB);
    t1.c_cflag |= CS8;
    if (tcsetattr(iTERM, TCSAFLUSH, &t1) == -1) perror("tcsetattr"); /* raw */
}

void termReset(void)
{
    if (tcsetattr(iTERM, TCSAFLUSH, &t0) == -1) perror("tcsetattr");/*restaure*/
}

void enleve1(char * s)
{
char * w;
    w=s;
    while (*w != '\0') *w++ = *(w+1);
}

void insertC(char c, char * s)
{
char *w;
    w=s;
    while (*w != '\0') w++;
    while (w >= s) { *(w+1) = *w; w--; }
    *s=c;
}

int lireLigne(int fd, char *b, char *s, int nc)
/* fd = file descriptor
   b  = adresse du buffer
   nc = nb de caracteres possible (longueur du buffer */
{
char *d, *f, c, c2, c3, *h, *w, *Wl, *rac;
int n, i, l, ls=0, ins=0, ignTild=0, nbT=0;
unsigned int j;
   /* printf("lireLigne ... \n"); */
   d = b;
   f = b+nc;
   while(d<f-ls) {
     if (noErr()) {
         n = -1;
         break;
     }
     /* fprintf(stderr,"d-b=%d ins=%d s=<%s> b=<%s>\n",d-b,ins,s,b); */
     if ((n=read(fd,d,1)) != 1) break;
     c=*d;
     if (ignTild && (c == '~')) {
        ignTild=0; continue;
     }
     if ((c > 31) && (c < 127)) { /* de SPACE a TILDE */
         if (!((FD_IN || iTERM) && ECHOOFF)) {
               printf("%c",c);
               if (ins) {
                  if (ins==2)  /* rewrite */
                     enleve1(s);
                  if(*s =='\0') ins=0;
                  else {
                     printf("%s",s);
                     for (j=0; j<strlen(s); j++) printf("\b");  
                  }
               }
         }
         fflush(stdout);
         d++;
     } else {
         switch (c) {
            case '\t': /* tab */
              if (d>b) {
                 *d='\0';
                 w=d-1;
                 while ((w>b) && (!(isSepa(*w,1)))) w--;
                 if (isSepa(*w,0)) w++;
                 /* fprintf(stderr,"d-b=%d  w=<%s>\n",d-b,w); */
                 if (strlen(w) > 0) {
                    j=nbLibBegin(w, &rac);
                    /* fprintf(stderr,"j=%d  w=<%s>\n",j,w); */
                    if (j==0) printf("\a");
                    else {
                       if (j==1) {
                          Wl=getLibBegin(w);
                          i=strlen(Wl)-strlen(w);
                          /* fprintf(stderr,"i=%d  Wl=<%s>\n",i,Wl); */
                          if (i>0) {
                             strcpy(w,Wl);
                             printf("%s ",d);
                             d+=i;
                             *d++ = ' ';
                             if (ins) {
                              printf("%s ",s);
                              for (j=0; j<(strlen(s)+1); j++) printf("\b");  
                             }
                          } else { /* XXXX */
                             if (i==0) {
                               printf (" ");
                               *d++ = ' ';
                             }
                          }
                       } else {
                          if (rac != NULL) {
                             i=strlen(rac)-strlen(w);
                             strcpy(w,rac);
                             printf("%s",d);
                             d+=i;
                             if (ins) {
                              printf("%s ",s);
                              for (j=0; j<(strlen(s)+1); j++) printf("\b");  
                             }
                          } else {
                            nbT++;
                            if (nbT>1) {
                               nbT=0;
                               printf("\n");
                               listLibBegin(w);
                               *d='\0';
                               printf("> %s",b);
                               if (ins) {
                                printf("%s ",s);
                                for (j=0; j<(strlen(s)+1); j++) printf("\b");  
                               }
                               fflush(stdout);
                            }
                          }
                       }
                    }
                    fflush(stdout);
                 }
              }
              break;
            case '\177':
              if (d>b) {
                 printf("\b \b");
                 if (ins) {
                  printf("%s ",s);
                  for (j=0; j<(strlen(s)+1); j++) printf("\b");  
                 }
                 fflush(stdout);
                 d--;
              }
              break;
            case '\n':
            case '\r':
              if ((FD_IN || iTERM) && ECHOOFF) printf(".");
              else {
                 printf("\n");
                 if (ins) {
                     if (d+strlen(s) < f) {
                        sprintf(d,"%s",s);
                        d+=strlen(s);
                     } else return(-1);
                 }
              }
              goto finBoucle;
         /* gestion des caracteres speciaux */
            case '\033':   /* ESCAPE */
              ignTild=1;
              read(fd,&c2,1);
              read(fd,&c3,1);
              if (c2 == '[') {
                  switch(c3) {
                      case '2' : /* Insert */
                        if (ins) {
                           ins++;
                           if (ins==3) ins=1;
                        }
                        break; 
                      case '3' : /* Suppr */
                        if (ins) {
                           enleve1(s);
                           if(*s =='\0') ins=0;
                           printf("%s \b",s);
                           for (j=0; j<strlen(s); j++) printf("\b");  
                           fflush(stdout);
                        }
                        break; 
                      case 'A' :
                      case 'B' :
                        ins = 0;
                        /* efface la ligne en cours */
                        l=d-b;
                        for(i=0;i<l;i++) printf("\b \b");
                        fflush(stdout);
                        d=b;
                        *d = '\0';
                        if ((h=getHisto(c3)) != NULL) {
                           strcpy(b,h);
                           d=b+strlen(h);
/*
                           printf("\n%s (iH=%d iHt=%d)\n",h,iH,iHt);
                           *d='\0';
*/
                           printf("%s",b);
                           fflush(stdout);
                        }
                        break; 
                      case 'C' : /* -> */
                        if (ins) {
                           *d = *s;
                           printf("%c",*d++);
                           enleve1(s);
                           if(*s =='\0') ins=0;
                           else {
                              printf("%s",s);
                              for (j=0; j<strlen(s); j++) printf("\b");  
                           }
                           fflush(stdout);
                        }
                        break; 
                      case 'D' : /* <- */
                        if (d>b) {
                           if (ins==0) {
                              ins=1;
                              *d='\0';
                              strcpy(s,d-1);
                           } else insertC(*(d-1),s);
                           d--;
                           printf("\b");
                           fflush(stdout);
                        }
                        break; 
                      default:
                        printf("\nESC [ %d (%c) !\n",(int)c3, c3);
                        *d='\0';
                        printf("> %s",b);
                        fflush(stdout);
                  }
              } else {
                 if (c2 == 'O') {
                    switch(c3) {
                      case 'P' : /* F1 */
                      break; 
                      case 'Q' : /* F2 */
                      break; 
                      case 'R' : /* F3 */
                      break; 
                      case 'S' : /* F4 */
                      break; 
                    }
                 } else {
                  printf("\nESC %d %d (%c) !\n",(int)c2,(int)c3, c3);
                  *d='\0';
                  printf("> %s",b);
                  fflush(stdout);
                 }
              }
              break; 
            default :
/*
              printf("\nCar = %d !\n",(int)c);
              *d='\0';
              printf("> %s",b);
              fflush(stdout);
*/
              break; 
         }
     }
     if (ins) ls = strlen(s);
   }
finBoucle:
   /* printf("fin lireLigne!\n"); */
   if ((n<1) && (FD_IN !=0)) {
      closeFD();  /* fichier loader */
      if (ECHOOFF) printf("\n");
      return 0;
   }
   if ((n<1) && iTERM) {
      close(FD_IN);  /* pipe ou autre */
      dup(iTERM); /* stdin on term */
      iTERM = 0;
      if (ECHOOFF) printf("\n");
      return 0;
   }
   if (d == f) { /* cas du buffer trop petit */
/*   d=b;
     while (*d != '\n') read(fd,d,1);
**** not in raw mode */
#ifdef DEBUG
     printf("lireLigne : erreur !\n");
#endif
     return(-1);
   } 
   *d = '\0';
#ifdef DEBUG
     printf("lireLigne : retour <%s> !\n",b);
#endif
   if (!FD_IN) putHisto(b);
   incFDlig();
   return(strlen(b));
}



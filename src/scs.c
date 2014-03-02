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
/* scs.c */
/* Scs means Sister Chip Signature */
#include <stdio.h>
#include <sys/types.h>

#include "scs.h"

static struct Ref { /* Representatives data */
    double D;
    char C1;
    char C2;
    short S;
    char C3;
    int I;
    char C4;
    long long LL;
} E;
static uint32_t MyScs=0;

static void Scs_init(void)
{
     E.C1 = 'N';
     E.C2 = 'i';
     E.C3 = 'f';
     E.C4 = 'e';
     E.D = 2.7182818284590452354;  /* e */
     E.S = 763;
     E.I = 33497;
     E.LL = (long long) 762572642;
}

uint32_t getScs(void)
{
int i,l;
unsigned char *t, k;
uint32_t r;
    if (MyScs == 0) {
       Scs_init();
       l = sizeof(E);
       t =(unsigned char*)&E;
       r=0;
       k=0;
       for (i=0; i<l; i++) {
         /* printf("%d : k=%d %u r=0x%.8lx\n",i,(int)k,(unsigned int)t[i],r);*/
         switch(k) {
         case 0:
           r+=(unsigned int)t[i];
           break;
         case 1:
           r+=(unsigned int)(((int)t[i])<<5);
           break;
         case 2:
           r+=(unsigned int)(((int)t[i])<<10);
           break;
         default:
           r+=(unsigned int)(((int)t[i])<<15);
           break;
         }
         if (++k==4) k=0;
       }
       /* printf("Key : l=%d r=%lu (0x%.8lx)\n",l,r,r); */
       MyScs = r + (uint32_t)(l<<24);
    }
    return MyScs;
}
/* ******************** for tests ************************************
int main(int N, char*P[])
{
int i;
unsigned char *d, *f;
unsigned short *ds, *fs;
unsigned int *di, *fi;
unsigned long long *dl, *fl;
   printf("short : %.2d\n", sizeof(short));
   printf("int : %.2d\n", sizeof(int));
   printf("size_t : %.2d\n", sizeof(size_t));
   printf("long : %.2d\n", sizeof(long));
   printf("long long : %.2d\n", sizeof(long long));
   printf("float : %.2d\n", sizeof(float));
   printf("double : %.2d\n", sizeof(double));
   printf("long double : %.2d\n", sizeof(long double));
   printf("E : %.2d\n", sizeof(E));
   Scs_init();
   d=(char*)&E;
   f=d+sizeof(E);
   while (d < f) printf("%.2x",(int)*d++);
   printf("\n");
   ds=(short*)&E;
   fs=ds+(sizeof(E)/sizeof(short));
   while (ds < fs) printf("%4x",(int)*ds++);
   printf("\n");
   di=(int*)&E;
   fi=di+(sizeof(E)/sizeof(int));
   while (di < fi) printf("%8x",*di++);
   printf("\n");
   dl=(long long*)&E;
   fl=dl+(sizeof(E)/sizeof(long long));
   while (dl < fl) printf("%.16llx",*dl++);
   printf("\n");
   printf("D : %.2d s=%.2d\n",(void*)(&E.D)-(void*)(&E),sizeof(E.D));
   printf("S : %.2d s=%.2d\n",(void*)(&E.S)-(void*)(&E),sizeof(E.S));
   printf("I : %.2d s=%.2d\n",(void*)(&E.I)-(void*)(&E),sizeof(E.I));
   printf("LL: %.2d s=%.2d\n",(void*)(&E.LL)-(void*)(&E),sizeof(E.LL));
   printf("MyScs = 0x%.8lx\n",getScs());
   printf("MyScs = 0x%.8lx\n",getScs());
}
****************/


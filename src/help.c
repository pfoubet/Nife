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
/* help.c */
#include <stdio.h>
#include <string.h>

#include "help.h"
#include "nife.h"
#include "lib.h"
#include "mth.h"

#define LHLP 200 /* longueur MAX des lignes du fichier hlp */
static char buf[LHLP];

static void helpShow(char * L)
{
FILE * fd;
int debut=0,l;
    if ((fd = fopen("nife.hlp","r")) == NULL) {
	perror("help file");
        return;
    } else {
      if (L == NULL) {
        debut=1;
        while (fgets(buf,LHLP,fd) != NULL) {
           if (buf[0] != ' ') break;
           printf("%s",buf);
        }
      } else {
        while (fgets(buf,LHLP,fd) != NULL) {
           if (debut) {
              if (buf[0] != ' ') break;
           } else {
              if (buf[0] == ' ') continue;
              l=strlen(L);
              if (strncmp(L,buf,l)!=0) continue;
              if (buf[l]!=' ') continue;
              debut=1;
           }
           printf("%s",buf);
        }
      }
    }
    if (debut==0) printf("No help find !\n");
    fclose(fd);
}

void helpStd(char * L)
{
    dropTrSuite();
    if (fctExists(L)==0) {
       printf("%s is not a User System function !\n",L);
       return;
    }
    if (*L == '_') {
      printf("%s :\n",L);
      printf("The same as '%s', but with all displays in the log file.\n", L+1);
      L++;
    }
    helpShow(L);
}


void IF_help(void)
{
    putTrSuite(helpStd);
}

void IF_helpS(void)
{
    helpShow(NULL);
}

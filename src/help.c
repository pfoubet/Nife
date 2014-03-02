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
/* help.c */
#include <stdio.h>
#include <string.h>

#include "help.h"
#include "nife.h"

#define LHLP 200 /* longueur MAX des lignes du fichier hlp */
static char buf[LHLP];
void helpStd(char * L)
{
FILE * fd;
int debut=0,l;
    dropTrSuite();
    if ((fd = fopen("nife.hlp","r")) == NULL) {
	perror("help file");
        return;
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
    if (debut==0) printf("No help find !\n");
    fclose(fd);
}


void IF_help(void)
{
    putTrSuite(helpStd);
}

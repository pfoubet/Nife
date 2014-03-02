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
/* net.h */

#ifndef __NIFE_NET_H__
#define __NIFE_NET_H__

#include <stdint.h>

#define UNI_KEY 0xffffffff

extern char NetServer[];

extern void IF_NetServer(void);
extern void IF_Me(void);
extern void IF_netOn(void);
extern void IF_netOff(void);
extern void IF_netDt(void);
extern void IF_netDepth(void);
extern void IF_netList(void);
extern void IF_netStopS(void);
extern void IF_netDropS(void);
extern void IF_netRusage (void);
extern void IF_netStackList (void);
extern void IF_netU2S (void);
extern void IF_netS2U (void);
extern void IF_netExec (void);
extern void IF_netCompile (void);
extern void sendData(int s, void * b, uint32_t n);
extern void sendDataC(void * b, uint32_t n);

#endif

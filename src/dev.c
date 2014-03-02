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
/* dev.c */

#ifdef HAVE_CONFIG_H
#include "../config.h"
#endif

#ifdef HAVE_COMEDILIB_H
#define _WITH_COMEDILIB
#endif

#include <stdio.h>
#include <string.h>
static int DFL_did=0, DFL_sid=0, DFL_cid=0;

#ifdef _WITH_COMEDILIB

#include <comedilib.h>

static char *subdev_types[]={
        "unused",
        "analog input",
        "analog output",
        "digital input",
        "digital output",
        "digital I/O",
        "counter",
        "timer",
        "memory",
        "calibration",
        "processor",
        "serial digital I/O"
};

static comedi_t *it;
static long n_it = -1;

static char *cmd_src(int src,char *buf)
{
    buf[0]=0;

    if(src&TRIG_NONE)strcat(buf,"none|");
    if(src&TRIG_NOW)strcat(buf,"now|");
    if(src&TRIG_FOLLOW)strcat(buf, "follow|");
    if(src&TRIG_TIME)strcat(buf, "time|");
    if(src&TRIG_TIMER)strcat(buf, "timer|");
    if(src&TRIG_COUNT)strcat(buf, "count|");
    if(src&TRIG_EXT)strcat(buf, "ext|");
    if(src&TRIG_INT)strcat(buf, "int|");
#ifdef TRIG_OTHER
        if(src&TRIG_OTHER)strcat(buf, "other|");
#endif
    if(strlen(buf)==0){
                sprintf(buf,"unknown(0x%08x)",src);
    }else{
                buf[strlen(buf)-1]=0;
    }
    return buf;
}

static void probe_max_1chan(comedi_t *it,int s)
{
        comedi_cmd cmd;
        char buf[100];

        printf("  command fast 1chan:\n");
        if(comedi_get_cmd_generic_timed(it, s, &cmd, 1, 1)<0){
                printf("    not supported\n");
        }else{
                printf("    start: %s %d\n",
                        cmd_src(cmd.start_src,buf),cmd.start_arg);
                printf("    scan_begin: %s %d\n",
                        cmd_src(cmd.scan_begin_src,buf),cmd.scan_begin_arg);
                printf("    convert: %s %d\n",
                        cmd_src(cmd.convert_src,buf),cmd.convert_arg);
                printf("    scan_end: %s %d\n",
                        cmd_src(cmd.scan_end_src,buf),cmd.scan_end_arg);
                printf("    stop: %s %d\n",
                        cmd_src(cmd.stop_src,buf),cmd.stop_arg);
        }
}

static void get_command_stuff(comedi_t *it,int s)
{
comedi_cmd cmd;
char buf[100];

    if(comedi_get_cmd_src_mask(it,s,&cmd)<0){
         printf("not supported\n");
    }else{
         printf("\n");
         printf("    start: %s\n",cmd_src(cmd.start_src,buf));
         printf("    scan_begin: %s\n",cmd_src(cmd.scan_begin_src,buf));
         printf("    convert: %s\n",cmd_src(cmd.convert_src,buf));
         printf("    scan_end: %s\n",cmd_src(cmd.scan_end_src,buf));
         printf("    stop: %s\n",cmd_src(cmd.stop_src,buf));

         probe_max_1chan(it,s);
    }
}

static int devOpen(long n)
{
char filename[24];
    if (n == n_it) return 1; /* deja fait */
    n_it=-1;
    sprintf(filename,"/dev/comedi%ld",n);
    if (! (it = comedi_open(filename))) {
        printf("%s : Error access !\n",filename);
        return 0;
    }
    n_it = n;
    return 1;
}

#endif  /* for _WITH_COMEDILIB */
/* *********************************************************************/

#include "stackN.h"
#include "dev.h"
#include "err.h"
#define LBUFD 200
static char buf[LBUFD];

void IF_listDev (void)
{
FILE * fd;
int id,ns;
char driver[24], nomdev[24];
    if ((fd = fopen("/proc/comedi","r")) == NULL)
        printf("No devices !\n");
    else {
      while (fgets(buf,LBUFD,fd) != NULL) {
         ns = -1;
         sscanf(buf,"%d: %s %s %d\n",&id,driver,nomdev,&ns);
         if (ns != -1)
           printf(" %2d : %-20s driver %-20s %4d subdevices\n",id,nomdev,driver,ns);
      }
    }
}

#ifdef _WITH_COMEDILIB
static void show_subdev(int i, int V)
{
int j, type, chan,n_chans, n_ranges, subdev_flags;
comedi_range *rng;
   printf("subdevice %d :",i);
   type = comedi_get_subdevice_type(it, i);
   printf("  type %d (%s) ",type,subdev_types[type]);
   if (type==COMEDI_SUBD_UNUSED) return;
   subdev_flags = comedi_get_subdevice_flags(it, i);
   printf("- flags 0x%08x\n",subdev_flags);
   n_chans=comedi_get_n_channels(it,i);
   printf("  nb of channels : %d",n_chans);
   if(!comedi_maxdata_is_chan_specific(it,i)){
      printf(" -  max data value = %lu\n",(unsigned long)comedi_get_maxdata(it,i,0));
   } else {
      printf("\n  max data value: (channel specific)\n");
      for(chan=0;chan<n_chans;chan++){
         printf("    chan%d: %lu\n",chan,
                        (unsigned long)comedi_get_maxdata(it,i,chan));
      }
   }
   if (V >= n_chans) {
      printf("Error : CId must be < %d !!\n", n_chans);
      return;
   }
   printf("  ranges (Volts) :");
   if(!comedi_range_is_chan_specific(it,i)){
      n_ranges=comedi_get_n_ranges(it,i,0);
      printf(" all chans:");
      for(j=0;j<n_ranges;j++){
         rng=comedi_get_range(it,i,0,j);
         printf(" [%g,%g]",rng->min,rng->max);
      }
      printf("\n");
   } else {
      printf("\n");
      for (chan=0;chan<n_chans;chan++){
          n_ranges=comedi_get_n_ranges(it,i,chan);
          printf("    chan%d:",chan);
          for (j=0;j<n_ranges;j++){
              rng=comedi_get_range(it,i,chan,j);
              printf(" [%g,%g]",rng->min,rng->max);
          }
          printf("\n");
      }
   }
   printf("  command        : ");
   get_command_stuff(it,i);
}
#endif

void IF_showDev (void)
{
long n;
    if (!getParLong(&n)) return;

#ifdef _WITH_COMEDILIB
int i, n_subdev;

    if (! devOpen(n)) return;
    printf("overall info:\n");
    printf("  version code   : 0x%06x\n", comedi_get_version_code(it));
    printf("  driver name    : %s\n", comedi_get_driver_name(it));
    printf("  board name     : %s\n", comedi_get_board_name(it));
    printf("  nb subdevices  : %d\n", n_subdev = comedi_get_n_subdevices(it));
    for (i = 0; i < n_subdev; i++) {
        show_subdev(i,0);
    }
#else
	messErr(17);
#endif
}

void IF_devShowDflt (void)
{
#ifdef _WITH_COMEDILIB
int n_subdev;
    printf("Default : DId=%d, SId=%d CId=%d\n", DFL_did, DFL_sid, DFL_cid);
    if (! devOpen(DFL_did)) return;
    printf("overall info:\n");
    printf("  version code   : 0x%06x\n", comedi_get_version_code(it));
    printf("  driver name    : %s\n", comedi_get_driver_name(it));
    printf("  board name     : %s\n", comedi_get_board_name(it));
    printf("  nb subdevices  : %d\n", n_subdev = comedi_get_n_subdevices(it));
    if (DFL_sid < n_subdev)
        show_subdev(DFL_sid,DFL_cid);
    else printf("Error : SId must be < %d !!\n", n_subdev);
#else
	messErr(17);
#endif
}

static void dev_read(int did, int sid, int cid)
{
#ifdef _WITH_COMEDILIB
lsampl_t data;
    if (! devOpen(did)) return;
    if (comedi_data_read(it,sid,cid,0,0,&data) < 0)
	messErr(18);
    else putLong((long)data);
#else
	messErr(17);
#endif
}

void IF_devDflR (void)
{
    dev_read(DFL_did, DFL_sid, DFL_cid);
}

void IF_devRead (void)
{
long n, sd, ch;
    if (!getParLong(&ch)) return;
    if (!getParLong(&sd)) return;
    if (!getParLong(&n)) return;
    dev_read(n, sd, ch);
}

void IF_devDflt (void)
{
long n, sd, ch;
    if (!getParLong(&ch)) return;
    if (!getParLong(&sd)) return;
    if (!getParLong(&n)) return;
    DFL_did=n;
    DFL_sid=sd;
    DFL_cid=ch;
}

static void dev_write(int did, int sid, int cid, long d)
{
#ifdef _WITH_COMEDILIB
lsampl_t data;
    if (! devOpen(did)) return;
    data = (lsampl_t)d;
    if (comedi_data_write(it,sid,cid,0,0,data) < 0)
	messErr(18);
#else
	messErr(17);
#endif
}

void IF_devWrite (void)
{
long d, n, sd, ch;
    if (!getParLong(&ch)) return;
    if (!getParLong(&sd)) return;
    if (!getParLong(&n)) return;
    if (!getParLong(&d)) return;
    dev_write(n, sd, ch, d);
    return;
}

void IF_devDflW (void)
{
long d;
    if (!getParLong(&d)) return;
    dev_write(DFL_did, DFL_sid, DFL_cid, d);
    return;
}


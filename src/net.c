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
/* net.c */
#include "conf.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/socket.h>
#include <signal.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
/*
*/
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>

#include "nife.h"
#include "mth.h"
#include "net.h"
#include "lib.h"
#include "debug.h"
#include "stackC.h"
#include "stackN.h"
#include "stackL.h"
#include "foncs.h"
#include "scs.h"
#include "err.h"

#define LOCALS "localhost"
#define MAXCLI 200
#define LLIB 24
static char *NetName=NULL;
/* for messages */
struct Mess1 {
    uint32_t pid;
    union {
      char lib[LLIB];
      in_port_t port;
    };
};

/* RPC structures */
struct Nrpc {
    uint32_t nkey;  /* netkey */
    char fct[LLIB]; /* function */
    struct Nrpc *next;
};
/* Table for Server */
struct Client {
    uint32_t pid;
    long long dt; /* delta time local - remote */
    struct in_addr ip; /* net order */
    in_port_t  port; /* Nrpc port in net order */
    struct Nrpc *ff; /* first function */
    char lib[LLIB];
} TCli[MAXCLI];

static int NCli, STS=0, SameSaS=0;

static void initCli(void)
{
int i;
   for(i=0;i<MAXCLI;i++) TCli[i].pid=0;
   NCli=0;
}

static int addCli(uint32_t pid, struct in_addr ip, char *lib, long long dt)
{
int i;
   if (NCli==MAXCLI) return -1;
   for(i=0;i<MAXCLI;i++) 
      if (TCli[i].pid == 0) break;
   TCli[i].pid = pid;
   TCli[i].dt = dt;
   TCli[i].port = 0;
   TCli[i].ff = (struct Nrpc*)NULL;
   TCli[i].ip.s_addr = ip.s_addr;
   strncpy(TCli[i].lib,lib,LLIB-1);
   TCli[i].lib[LLIB-1] = '\0';
   NCli++;
   return 0;
}

static void delCli(int pid, struct in_addr ip)
{
int i;
   for(i=0;i<MAXCLI;i++) 
      if ((TCli[i].pid == pid) && (TCli[i].ip.s_addr == ip.s_addr)) {
         TCli[i].pid = 0;
         TCli[i].lib[0] = '\0';
         NCli--;
         break;
      }
}

static void majNRP(int pid, in_port_t p)
{
int i;
   for(i=0;i<MAXCLI;i++) 
      if (TCli[i].pid == pid) {
         TCli[i].port = p;
         break;
      }
}

static long long getDt(int pid, struct in_addr ip)
{
int i;
   for(i=0;i<MAXCLI;i++) 
      if ((TCli[i].pid == pid) && (TCli[i].ip.s_addr == ip.s_addr)) 
         return TCli[i].dt;
   return 0;
}

static int getCliId(char *N)
{
int i;
   for(i=0;i<MAXCLI;i++) 
      if (strcmp(TCli[i].lib,N) == 0)
         return i;
   return -1;
}

static void listCli(void)
{
int i,j=0;
char S;
   for(i=0;i<MAXCLI;i++) 
      if (TCli[i].pid != 0) {
         j++;
         if (TCli[i].dt == 0) S='=';
         else {
             if (TCli[i].dt > 0) S='+'; else S='-';
         }
         printf("%3d : %6d %15s:%d %c %s\n",j,TCli[i].pid,inet_ntoa(TCli[i].ip),
                  (int)ntohs(TCli[i].port), S, TCli[i].lib);
         /* with dt */
/*
         printf("%3d : %6d %15s %15ld %s\n",j,TCli[i].pid,inet_ntoa(TCli[i].ip),
                  (long)TCli[i].dt, TCli[i].lib);
*/
      }
}

#define SERV_PORT 32011 /* Stack to Stack Protocol port */

/* commands list  (default 1+1) */
#define NET_SLI  '?'	/* ask the stack list */
#define NET_DRS  '-'	/* 1+0 drop net stack */
#define NET_ONR  'a'	/* remote login */
#define NET_ON   'A'	/* login */
#define NET_CAL  'C'	/* calcul of dt */
#define NET_DPH  'd'	/* depth */
#define NET_DAT  'D'	/* data */
#define NET_OFF  'E'	/* logoff */
#define NET_U2S  'F'	/* From user to server */
#define NET_FIL  'f'	/* ok to send file */
#define NET_GDT  'G'	/* get dt */
#define NET_LI   'L'	/* ask the list */
#define NET_NRP  'N'	/* NRpc Port */
#define NET_PAR  'P'	/* call distant parser */
#define NET_RUS  'R'	/* ask to Ressources Usage */
#define NET_STS  'S'	/* ask to Stop The Server */
#define NET_S2U  'T'	/* from server To user */
#define NET_EXE  'X'	/* send a RPC command */
#define NET_ERR  'Z'	/* Error */
#define NET_EOM (char)3 /* End Of Message */

#define LNBUF  200
#define LNSRV  24

char NetServer[24]=LOCALS;
static int Sock, NET=0;
static struct sockaddr_in Sin, SinSav;

static int connSock2(int n) /* connect to Nife RPC Client */
{
   if ((Sock = socket(AF_INET, SOCK_STREAM,  IPPROTO_TCP)) == -1) return 1;
   bcopy(&SinSav,&Sin,sizeof(Sin));
   bcopy(&TCli[n].ip,&Sin.sin_addr,sizeof(Sin.sin_addr));
   bcopy(&TCli[n].port,&Sin.sin_port,sizeof(Sin.sin_port));
   if(connect(Sock,(struct sockaddr *)&Sin,sizeof(Sin)) < 0) return 1;
   return 0; /* OK */
}

void IF_Me(void)
{
char * Me;
void * NM;
int i=0;
static char *ve[] = { "NIFENAME", "LOGNAME", "USERNAME", "USER", "" };
static char Nobody[] = "Nobody";
  if (NetName == NULL) {
    Me=NULL;
    while (Me == NULL) {
       if (strlen(ve[i])==0) break;
       Me = getenv(ve[i++]);
    }
    if (Me == NULL) Me=Nobody;
    if ((NM = malloc(strlen(Me)+1)) != NULL)
       strcpy((char*)NM,Me);
    NetName=(char*)NM;
  }
  putString(NetName);
}

void IF_NetServer(void)
{
char *s;
int i;
   if (!isNString(1)) {
           messErr(6); return;
   }
   s = getString();
   i=LNSRV-1;
   strncpy(NetServer,s,i);
   NetServer[i]='\0';
   free((void*)s);
}

static void stopServ(int TS, char * m, int n)
{
char buf[20];
    perror(m);
    if (TS) sprintf(buf,"main_Serv : Erreur %d %s\n",n,m);
    else    sprintf(buf,"main_Nrpc : Erreur %d %s\n",n,m);
    D_Trace(buf);
    exit(n);
}

static char NET_BUF[LNBUF];

static int readMess(int s)
{
char * b, *f;
   b = NET_BUF;
   f = b + LNBUF - 1;
   while (1) {
     if (read(s,(void*)b,1) <= 0) break;
     if (*b == NET_EOM) break;
     b++;
     if (b>f) { b--; break; }
   }
   if (*b != NET_EOM) return -1;
   b++;
   return(b-NET_BUF);
}

static void readAff(int s, char C)
{
char  b[2];
int ok=0;
   if (read(s,(void*)b,1) <= 0) return;
   if (*b == C) ok=1;
   while (*b != NET_EOM) {
         if (read(s,(void*)b,1) <= 0) break;
         else {
            if (ok) write(1,(void*)b,1);
         }
   }
}

static void sendC(char c)
{
    write(Sock,(void*)&c,1);
}

static void sendBuf(int s, void * b, size_t no)
{
    write(s,b,no);
}

static void rcvBuf(int s, void * b, uint32_t no)
{
int r,ar;
void *t;
    ar = no;
    r = 0;
    t = b;
    while ((r=read(s,t,ar)) != ar) {
        if (r<0) break;
        ar -= r;
        t += r;
    }
}

void sendData(int s, void * b, uint32_t n)
{
char c;
    c=NET_DAT;
    write(s,(void*)&c,1);
    n -= lAdrNum(); /* don't send the addresse */
    sendBuf(s,&n,sizeof(n));
    /* sendBuf(s,b,(size_t)n); */
    sendBuf(s,b,sizeof(uint32_t)*2);
    b+=(sizeof(uint32_t)*2)+lAdrNum();
    n-=(sizeof(uint32_t)*2);
    sendBuf(s,b,(size_t)n);
}

void sendDataC(void *b, uint32_t n)
{
    sendData(Sock,b,n);
}

void sendFile(int s, int fid)
{
int n;
char Buf[LNBUF];
   while ((n = read(fid,(void*)Buf,LNBUF)) > 0)
     sendBuf(s,(void*)Buf,(size_t)n);
   close(fid);
}

static void sendInsc(void)
{
char *s;
struct Mess1 M1;
uint32_t scs;
struct timeval tv;
long long v;

    scs = getScs();
    s = getString();
    M1.pid = getpid();
    strncpy(M1.lib,s,LLIB-1);
    M1.lib[LLIB-1]='\0';
    if (SameSaS) sendC(NET_ON);
    else sendC(NET_ONR);
    write(Sock,(void*)&scs,sizeof(scs)); 
    write(Sock,(void*)&M1,sizeof(M1)); 
    sendC(NET_EOM);
    /* update NetName */
    if (NetName != NULL) free((void*)NetName);
    NetName=s;
    if (SameSaS==0) {
      if (readMess(Sock) == 2) {
         if (*NET_BUF == (NET_CAL))  {
            gettimeofday(&tv,NULL);
            v=Nife_time(tv);
            sendC(NET_CAL+1);
            write(Sock,(void*)&v,sizeof(v)); 
            sendC(NET_EOM);
         }
      }
    } else SameSaS=0;
}

static void sendGetStd(char Code)
{
struct Mess1 M1;
    switch(Code) {
      case NET_OFF :
      case NET_GDT :
         M1.pid = getpid();
         break;
      default :
         M1.pid = NetKey;
    }
    strcpy(M1.lib,"-");
    sendC(Code);
    write(Sock,(void*)&M1,sizeof(M1)); 
    sendC(NET_EOM);
}

static void sendNrpcPort(in_port_t P)
{
struct Mess1 M1;
    M1.pid = getppid(); /* parent pid ! */
    M1.port = P;
    sendC(NET_NRP);
    write(Sock,(void*)&M1,sizeof(M1)); 
    sendC(NET_EOM);
}

static void send1car(int s, char c)
{
char b[2];
    b[0]=c;
    b[1]=NET_EOM;
    write(s,b,2); 
}

void NTraceM1(char * A)
{
struct Mess1 M1;
char buf[50];
   bcopy((void*)(A+1),(void*)&M1,sizeof(M1));
   sprintf(buf,"TraceM1 %c %d %s !",*A, M1.pid, M1.lib);
   D_Trace(buf);
}

void NTraceData(char A, long n)
{
char buf[50];
   sprintf(buf,"TraceData %c %ld !",A, n);
   D_Trace(buf);
}

static void RemotExecBegin(long long k)
{
   putLong(k);
   IF_NetKey();
   IF_netS2U();
   IF_netDropS();
}

static void RemotExecEnd(void)
{
   if (noErr()) {
      razErr();
      IF_stack_clear(); /* clear all stacks */
      IF_stackL_clear();
      IF_stackC_clear();
      IF_NetErrVal();
   }
   IF_depth();
   IF_netU2S();
   IF_stack_clear(); /* don't eat any bread ! */
   IF_stackL_clear();
   IF_stackC_clear();
}

static void net_servrpc(int s)
{
int i, j, fid;
uint32_t k;
char *sF, nF[30];
   i=readMess(s);
   NET_BUF[i]='\0';
   /* D_Trace(NET_BUF); */
   switch(*NET_BUF) {
     case NET_OFF :
       send1car(s, NET_OFF+1);
       exit(0); /* stop the Nrpc client */
       break;
     case NET_EXE :
       bcopy((void*)(NET_BUF+1),(void*)&k,sizeof(k));
       sF = (char *)(NET_BUF+sizeof(k)+1);
       /* execute the function call */
       /* fprintf(stderr,"Nife_RPC : %s NetKey=%x !\n",sF,k);*/
       RemotExecBegin((long long)k);
       if (!execLibNrpc(sF)) IF_NetErrVal();
       RemotExecEnd();
       break;
     case NET_PAR :
       bcopy((void*)(NET_BUF+1),(void*)&k,sizeof(k));
       sprintf(nF,"w_%d.nif",getpid());
       if ((fid = open(nF,O_CREAT|O_TRUNC|O_RDWR,0600)) != -1) {
          j=1;
          while ((j==1) && ((i=read(s,NET_BUF,LNBUF)) > 0)) {
               if (NET_BUF[i-1] == NET_EOM){
                 j=0; i--;
               }
               write(fid, NET_BUF, i);
          }
          close(fid);
          RemotExecBegin((long long)k);
          compileFile(nF);
          RemotExecEnd();
          unlink(nF);
       }
       break;
     default :
       break;
   }
}


static void net_service(int s)
{
int i, CalDT=0;
uint32_t n, k,scs;
void *M;
struct Mess1 M1;
struct timeval tv;
long long v1, v2, vm, vr, dt;
char c, *sN, *sF;

   i=readMess(s);
   NET_BUF[i]='\0';
   /* D_Trace(NET_BUF); */
   switch(*NET_BUF) {
     case NET_ONR :
       CalDT=1;
     case NET_ON :
       /* NTraceM1(NET_BUF); */
       bcopy((void*)(NET_BUF+1),(void*)&k,sizeof(k));
       scs = getScs();
       if (k==scs) {
          bcopy((void*)(NET_BUF+sizeof(k)+1),(void*)&M1,sizeof(M1));
          if (CalDT) {
             gettimeofday(&tv,NULL);
             send1car(s, NET_CAL);
             v1=Nife_time(tv);
             i=readMess(s);
             gettimeofday(&tv,NULL);
             v2=Nife_time(tv);
             if (*NET_BUF == NET_CAL+1) {
                bcopy((void*)(NET_BUF+1),(void*)&vr,sizeof(vr));
                vm = v1 + ((v2-v1)/2);
                addCli(M1.pid, Sin.sin_addr, M1.lib, (int)(vm-vr));
             }
             else
                addCli(M1.pid, Sin.sin_addr, M1.lib, -9);
             send1car(s, NET_ON+1);
          } else {
             addCli(M1.pid, Sin.sin_addr, M1.lib, 0);
             send1car(s, NET_ON+1);
          }
       }
       else 
          send1car(s, NET_OFF);
       break;
     case NET_OFF :
       bcopy((void*)(NET_BUF+1),(void*)&M1,sizeof(M1));
       delCli(M1.pid, Sin.sin_addr);
       send1car(s, NET_OFF+1);
       if (STS) {
          if (NCli == 0) exit(0); /* stop the server */
          STS=0;
       }
       break;
     case NET_NRP :
       bcopy((void*)(NET_BUF+1),(void*)&M1,sizeof(M1));
       majNRP(M1.pid, M1.port);
       send1car(s, NET_NRP+1);
       break;
     case NET_DPH :
       bcopy((void*)(NET_BUF+1),(void*)&M1,sizeof(M1));
       dt=NetDepth(M1.pid);
       c=*NET_BUF+1;
       write(s,(void*)&c,1);
       write(s,(void*)&dt,sizeof(dt));
       c=NET_EOM;
       write(s,(void*)&c,1);
       break;
     case NET_GDT :
       bcopy((void*)(NET_BUF+1),(void*)&M1,sizeof(M1));
       dt=getDt(M1.pid, Sin.sin_addr);
       c=*NET_BUF+1;
       write(s,(void*)&c,1);
       write(s,(void*)&dt,sizeof(dt));
       c=NET_EOM;
       write(s,(void*)&c,1);
       break;
     case NET_EXE :
       bcopy((void*)(NET_BUF+1),(void*)&k,sizeof(k));
       sN = (char *)(NET_BUF+sizeof(k)+1);
       sF = sN+strlen(sN)+1;
       if (strlen(sN)>=LLIB) sN[LLIB-1]='\0';
       i=getCliId(sN);
       if (i>=0) {
          send1car(s, NET_EXE+1);
          /* send the call */
          if (!connSock2(i)) {
             sendC(NET_EXE);
             sendBuf(Sock,(void*)&k, sizeof(k));
             sendBuf(Sock,(void*)sF, strlen(sF)+1);
             sendC(NET_EOM);
          }
       }
       else send1car(s, NET_ERR);
       break;
     case NET_PAR :
       bcopy((void*)(NET_BUF+1),(void*)&k,sizeof(k));
       sN = (char *)(NET_BUF+sizeof(k)+1);
       if (strlen(sN)>=LLIB) sN[LLIB-1]='\0';
       i=getCliId(sN);
       if (i>=0) {
          /* connect to client */
          if (!connSock2(i)) {
             send1car(s, NET_FIL); /* tell ok to sender */
             sendC(NET_PAR);
             sendBuf(Sock,(void*)&k, sizeof(k));
             sendC(NET_EOM);
             while ((i=read(s,NET_BUF,LNBUF)) > 0) {
               sendBuf(Sock, NET_BUF, i);
               if (NET_BUF[i-1] == NET_EOM) break;
             }
             send1car(s, NET_PAR+1);
          }
          else send1car(s, NET_ERR);
       }
       else send1car(s, NET_ERR);
       break;
     case NET_STS :
       STS=1;
       send1car(s, NET_STS+1);
       break;
     case NET_DRS :
       bcopy((void*)(NET_BUF+1),(void*)&k,sizeof(k));
       IF_netDrop(k);
       break;
     case NET_LI :
       c=*NET_BUF+1;
       write(s,(void*)&c,1);
       dup2(s,1);
       listCli();
       close(1);
       c=NET_EOM;
       write(s,(void*)&c,1);
       break;
     case NET_SLI :
       bcopy((void*)(NET_BUF+1),(void*)&k,sizeof(k));
       c=*NET_BUF+1;
       write(s,(void*)&c,1);
       dup2(s,1);
       IF_show_netStack(k);
       close(1);
       c=NET_EOM;
       write(s,(void*)&c,1);
       break;
     case NET_RUS :
       c=*NET_BUF+1;
       write(s,(void*)&c,1);
       dup2(s,1);
       IF_resUsage();
       close(1);
       c=NET_EOM;
       write(s,(void*)&c,1);
       break;
     case NET_U2S :
       while (1) {
           read(s,(void*)&c,1);
           if (c == NET_EOM) break;
           if (c != NET_DAT) {
              D_Trace("Erreur NET_DAT !"); break;
           }
           read(s,(void*)&n,sizeof(n));
           /* NTraceData(c, n);*/
           /* add the addresse */
           if ((M = malloc((size_t)n+lAdrNum())) != NULL) {
              rcvBuf(s,M,(sizeof(uint32_t)*2));
              rcvBuf(s,M+((sizeof(uint32_t)*2)+lAdrNum()),
                                           n-(sizeof(uint32_t)*2));
              putVar(M);
           }
       }
       break;
     case NET_S2U :
       bcopy((void*)(NET_BUF+1),(void*)&k,sizeof(k));
       /* NTraceData(NET_S2U, k);*/
       NetToStack(s,k);
       c=NET_EOM;
       write(s,(void*)&c,1);
       break;
     default :
       break;
   }
}

static int connSock(void)
{
   if ((Sock = socket(AF_INET, SOCK_STREAM,  IPPROTO_TCP)) == -1) {
      messErr(31);
      return 1;
   }
   bcopy(&SinSav,&Sin,sizeof(Sin));
   if(connect(Sock,(struct sockaddr *)&Sin,sizeof(Sin)) < 0) {
      messErr(34);
      return 1;
   }
   return 0; /* OK */
}

void main_Nrpc(void) /* main function for Nrpc client */ 
{
int sock, nsock;
socklen_t ln;
struct sockaddr_in Nsin;

    signal(SIGTERM, SIG_DFL);
    if ((sock = socket(AF_INET, SOCK_STREAM,  IPPROTO_TCP)) < 0)
       stopServ(0, "socket",1); 
    bzero(&Nsin,sizeof(Nsin));
    Nsin.sin_family = AF_INET;
    if (bind(sock, (struct sockaddr *)&Nsin, sizeof(Nsin)) < 0) 
       stopServ(0, "bind",2); 
    /* send Nrpc port */
    ln = sizeof(Nsin);
    if (getsockname(sock, (struct sockaddr *)&Nsin, &ln) < 0)
       stopServ(0, "gsname",3); 
    if (connSock()) return;
    sendNrpcPort(Nsin.sin_port);
    if (readMess(Sock) != 2) { messErr(35); return; }
    close(Sock);
    if (*NET_BUF != (NET_NRP+1))  { messErr(40); return; }
    /* listen and wait demands */
    if (listen(sock,5) < 0) stopServ(0, "listen",4); 
    ln = sizeof(Nsin);
    for (;;) {
        if((nsock=accept(sock,(struct sockaddr *)&Nsin,(socklen_t*)&ln)) < 0)
            stopServ(0, "accept",5); 
        net_servrpc(nsock);
        close(nsock);
    }
}

int Nrpc_pid=-1;
static void startNrpc(void)
{

    if ((Nrpc_pid = fork()) == -1) stopErr("startNrpc","fork");
    if (Nrpc_pid == 0) { /* fils */
       close(0);
       close(1);
       /* close(2); */
       D_Reset();
       IF_stack_clear(); /* clear all stacks */
       IF_stackL_clear();
       IF_stackC_clear();
       main_Nrpc();
       exit(1);
    }
}

void main_Serv(void) /* main function of Server */
{
int ln, sock, nsock;

    initCli();
    if ((sock = socket(AF_INET, SOCK_STREAM,  IPPROTO_TCP)) < 0)
       stopServ(1, "socket",1); 
    bzero(&Sin.sin_addr,sizeof(Sin.sin_addr));
    if (bind(sock, (struct sockaddr *)&Sin, sizeof(Sin)) < 0) 
       stopServ(1, "bind",2); 
    if (listen(sock,5) < 0) stopServ(1, "listen",3); 
    ln = sizeof(Sin);
    for (;;) {
        if((nsock=accept(sock,(struct sockaddr *)&Sin,(socklen_t*)&ln)) < 0)
            stopServ(1, "accept",4); 
        net_service(nsock);
        close(nsock);
    }
}

static void startServer(void)
{
int pid;
    if ((pid = fork()) == -1) stopErr("startServer","fork");
    if (pid == 0) { /* fils */
       setsid();
       close(0);
       close(1);
       /* close(2); */
       D_Reset();
       close(Sock);
       IF_stack_clear(); /* clear all stacks */
       IF_stackL_clear();
       IF_stackC_clear();
       main_Serv();
    }
    sleep(2); /* wait for server initialization */
}

static void IF_netGetVal (char Code)
{
long long v;
   if (NET) {
      if (connSock()) return;
      sendGetStd(Code);
      if (readMess(Sock) != 2+sizeof(v)) { messErr(35); return; }
      close(Sock);
      if (*NET_BUF != (Code+1))  { messErr(40); return; }
      bcopy((void*)(NET_BUF+1),(void*)&v,sizeof(v));
      putLong(v);
   } else messErr(32);
}

void IF_netDt (void)
{
    IF_netGetVal(NET_GDT);
}


void IF_netDepth (void)
{
    IF_netGetVal(NET_DPH);
}

void IF_netOff (void)
{
   if (NET) {
      if (connSock()) return;
      sendGetStd(NET_OFF);
      if (readMess(Sock) != 2) { messErr(35); return; }
      close(Sock);
      if (*NET_BUF != (NET_OFF+1))  { messErr(40); return; }
      if (Nrpc_pid > -1) { /* stop Nrpc local server */
          kill(Nrpc_pid, SIGTERM);
          Nrpc_pid=-1;
      }
      NET=0;
   }
}

void IF_netOn (void)
{
struct hostent *h;
   if (!isNString(1)) {
        messErr(6); return;
   }
   if (NET) {
        IF_dropC(); return;
   }
   if ((Sock = socket(AF_INET, SOCK_STREAM,  IPPROTO_TCP)) != -1) {
     if(!(h=gethostbyname(NetServer))) {
        messErr(33);
        return;
     }
     bzero(&Sin,sizeof(Sin));
     Sin.sin_family = AF_INET;
     bcopy(h->h_addr,&Sin.sin_addr,h->h_length);
     Sin.sin_port = htons(SERV_PORT);
     bcopy(&Sin,&SinSav,sizeof(Sin));
     if (strcmp(NetServer,LOCALS) == 0) /* localhost */
       SameSaS=1;                        /* same system as server */
     if(connect(Sock,(struct sockaddr *)&Sin,sizeof(Sin)) < 0) {
        if (strcmp(NetServer,LOCALS) != 0) { /* not localhost */
           messErr(34);
           return;
        }
        /* Server is absent : fork it ! */
        startServer();
        if(connect(Sock,(struct sockaddr *)&Sin,sizeof(Sin)) < 0) {
           messErr(34);
           return;
        }
     }
     sendInsc();
     if (readMess(Sock) != 2) { messErr(35); return; }
     close(Sock);
     if (*NET_BUF == (NET_OFF))  { messErr(38); return; }
     if (*NET_BUF != (NET_ON+1))  { messErr(40); return; }
     NET=1; /* net on !! */
     /* start RPC local server */
     startNrpc();
   } else messErr(31);
}

void IF_netExec (void)
{
char *sN, *sF;
   if (!isNString(2)) {
        messErr(6); return;
   }
   sN=getString();
   sF=getString();
   if (NET) {
     if (!connSock()) {
        sendC(NET_EXE);
        sendBuf(Sock,_ADDV_NetKey_, sizeof(NetKey));
        sendBuf(Sock,(void*)sN, strlen(sN)+1);
        sendBuf(Sock,(void*)sF, strlen(sF)+1);
        sendC(NET_EOM);
        if (readMess(Sock) != 2) messErr(35);
        close(Sock);
        if (*NET_BUF != (NET_EXE+1)) {
            if (*NET_BUF == (NET_ERR)) messErr(41);
            else messErr(40);
        }
     }
   } 
   free((void*)sN);
   free((void*)sF);
}

void IF_netCompile (void)
{
char *sN, *sF;
int fid;
   if (!isNString(2)) {
        messErr(6); return;
   }
   sN=getString();
   sF=getString();
   if ((fid=open(sF,O_RDONLY)) == -1) messErr(43);
   else
   if (NET) {
     if (!connSock()) {
        sendC(NET_PAR);
        sendBuf(Sock,_ADDV_NetKey_, sizeof(NetKey));
        sendBuf(Sock,(void*)sN, strlen(sN)+1);
        sendC(NET_EOM);
        /* wait for invitation */
        if (readMess(Sock) != 2) messErr(35);
        else {
         if (*NET_BUF == (NET_FIL)) {
           sendFile(Sock, fid);
           sendC(NET_EOM);
           if (readMess(Sock) != 2) messErr(35);
         }
        }
        close(Sock);
        if (*NET_BUF != (NET_PAR+1)) {
            if (*NET_BUF == (NET_ERR)) messErr(41);
            else messErr(40);
        }
     }
   } 
   free((void*)sN);
   free((void*)sF);
}

void IF_netList (void)
{
   if (NET) {
     if (connSock()) return;
     send1car(Sock,NET_LI);
     readAff(Sock, NET_LI+1);
     close(Sock);
   } 
   printf("<end of net list>\n");
}

void IF_netStackList (void)
{
   if (NET) {
     if (connSock()) return;
     sendC(NET_SLI);
     sendBuf(Sock,_ADDV_NetKey_, sizeof(NetKey));
     sendC(NET_EOM);
     readAff(Sock, NET_SLI+1);
     close(Sock);
   } 
   /* printf("<end of net stack>\n"); */
}

void IF_netDropS (void)
{
   if (NET) {
     if (NetKey == UNI_KEY) {
        messErr(37);
        return;
     }
     if (connSock()) return;
     sendC(NET_DRS);
     sendBuf(Sock,_ADDV_NetKey_, sizeof(NetKey));
     sendC(NET_EOM);
     close(Sock);
   } 
}

void IF_netRusage (void)
{
   if (NET) {
     if (connSock()) return;
     send1car(Sock,NET_RUS);
     readAff(Sock, NET_RUS+1);
     close(Sock);
   } 
   printf("<end of server ressources usage>\n");
}

void IF_netStopS (void)
{
   if (NET) {
     if (connSock()) return;
     send1car(Sock,NET_STS);
     if (readMess(Sock) != 2) { messErr(35); return; }
     close(Sock);
     if (*NET_BUF != (NET_STS+1))  { messErr(40); return; }
   } 
}

void IF_netU2S(void)
{
long n;
   if (NET) {
     if (!getParLong(&n)) return;
     if (n==0) return;
     if (NetKey == UNI_KEY) {
        messErr(37);
        return;
     }
     if (connSock()) return;
     send1car(Sock,NET_U2S);
     StackToNet(n);
     sendC(NET_EOM);
     close(Sock);
   } else messErr(32);
}

void IF_netS2U(void)
{
uint32_t n;
void *M;
char c;
   if (NET) {
     if (connSock()) return;
     sendC(NET_S2U);
     sendBuf(Sock,_ADDV_NetKey_, sizeof(NetKey));
     sendC(NET_EOM);
     while (1) {
         read(Sock,(void*)&c,1);
         if (c == NET_EOM) break;
         if (c != NET_DAT) {
              messErr(40); break;
         }
         read(Sock,(void*)&n,sizeof(n));
         /* add the addresse */
         if ((M = malloc((size_t)n+lAdrNum())) != NULL) {
              rcvBuf(Sock,M,(sizeof(uint32_t)*2));
              rcvBuf(Sock,M+((sizeof(uint32_t)*2)+lAdrNum()),
                                           n-(sizeof(uint32_t)*2));
              putVar(M);
         } else stopErr("IF_netS2U","malloc");
     }
     close(Sock);
   } else messErr(32);
}




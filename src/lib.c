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
/* lib.c  librairie de base */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "nife.h"
#include "mth.h"
#include "err.h"
#include "histo.h"
#include "foncs.h"
#include "tasks.h"
#include "stackN.h"
#include "stackL.h"
#include "stackC.h"
#include "stackF.h"
#include "stackV.h"
#include "libmath.h"
#include "help.h"
#include "dev.h"
#include "net.h"
#include "gplot.h"
#include "debug.h"

/* familles des fonctions */
#define F_CORE  1 /* Core */
#define F_MATH  2 /* Mathematiques */
#define F_PROG  4 /* Programmation, variables, tasks, ... */
#define F_TOOL  8 /* External Tools Graphical or others */
#define F_DEV  16 /* devices, comedi interfaces, ... */
#define F_NET  32 /* networking functions */
#define F_USR  64 /* user system functions */

struct fonction {
    char * nam;
    void (* fct)(void);
    short typ; /* 0 fct, 1 instruction, 2 externe, 3 usr fct */
    short fam; /* cf F_xxx ci-dessus */
};

#define NBFONC 400
#define NBCMOY 12	/* nb de car. en moyenne */
static char Libelles[NBFONC*NBCMOY], *pLibs=Libelles;
static struct fonction Fonctions[NBFONC];
static int NBFonc=0; 
int InstallOn=0;

static void IF_INSTALL(void) { InstallOn=1; }
static void IF_INSTALLF(void) { InstallOn=2; }
static void IF_INSTALLV(void) { InstallOn=3; }
/* for dynamic functions */
static void IF_DF_INIT(void) { InstallOn=8; }
static void IF_DF_START(void) { InstallOn=9; }
static void IF_DF_STOP(void) { InstallOn=10; }

static void addFoncT(char *l, void (*f)(void), short T, short F)
{
char *LMax;
    if (NBFonc >= NBFONC) stopErr("addFonc : NBFONC !",NULL);
    LMax = Libelles + (NBFONC*NBCMOY);
    if (pLibs + strlen(l) + 1 >= LMax) stopErr("addFonc : Libelles !",NULL);
    Fonctions[NBFonc].nam = pLibs;
    Fonctions[NBFonc].typ = T;
    Fonctions[NBFonc].fam = F;
    Fonctions[NBFonc++].fct = f;
    strcpy(pLibs,l);
    pLibs += strlen(l);
    *pLibs++ = '\0';
}
static void addFonc(char *l, void (*f)(void))
{
    addFoncT(l,f,0,F_CORE);
}
static void addFonM(char *l, void (*f)(void))
{
    addFoncT(l,f,0,F_MATH);
}
static void addFonP(char *l, void (*f)(void))
{
    addFoncT(l,f,0,F_PROG);
}
static void addFonG(char *l, void (*f)(void))
{
    addFoncT(l,f,0,F_TOOL);
}
static void addFonD(char *l, void (*f)(void))
{
    addFoncT(l,f,0,F_DEV);
}
static void addFonN(char *l, void (*f)(void))
{
    addFoncT(l,f,0,F_NET);
}
void addFonU(char *l, void *A)
{
PFV f;
    f = (PFV)A;
    addFoncT(l,f,0,F_USR);
}
static void addFonE(char *l, void (*f)(void), short F) /* External functions */
{                                                      /* not beetween : and ;*/
    addFoncT(l,f,2,F);
}
static void addInst(char *l, void (*f)(void))
{
    addFoncT(l,f,1,F_PROG);
}

static char **ListFonc = (char**)NULL;

static void Show_library(int NbF)
{
int i,j=0;
    for (i=0;i<NbF;i++) {
        /* printf("%-13s",Fonctions[i].nam);*/
        printf("%-13s",ListFonc[i]);
        j++;
        if (j == 6) {
           j=0;
           printf("\n");
        }
    }
    if (j) printf("\n");
}

static int triList(short F)
{
void * M;
char * T;
int i,j, NbF;
   if (ListFonc != (char**)NULL) free((void*)ListFonc);
   if ((M = malloc(sizeof(char*)* NBFonc)) == NULL) stopErr("triList","malloc");
   ListFonc = (char**)M;
   j=0;
   for(i=0;i<NBFonc;i++) {
      if (Fonctions[i].fam & F) ListFonc[j++]=Fonctions[i].nam;
   }
   NbF = j;
   for(i=0; i<NbF-1; i++) /* avant NbF-2 */
      for(j=i+1; j<NbF; j++)
        if(strcmp(ListFonc[j],ListFonc[i]) < 0) {
          T=ListFonc[i]; ListFonc[i]=ListFonc[j]; ListFonc[j]=T;
        }
   return NbF;
}

void listLibBegin(char * b)
{
int i,j=0,l,n;
   l=strlen(b);
   n=triList(0xFF);
   for(i=0;i<n;i++) {
      if (strncmp(ListFonc[i],b,l)==0){
        printf("%-13s",ListFonc[i]);
        j++;
        if (j == 6) {
           j=0;
           printf("\n");
        }
      } else
         if (strncmp(ListFonc[i],b,l)>0) break;
   }
   if (j) printf("\n");
}

char * getLibBegin(char * b)
{
int i,l;
   l=strlen(b);
   for(i=0;i<NBFonc;i++) {
      if (strncmp(Fonctions[i].nam,b,l)==0) break;
   }
   if (i==NBFonc) return NULL;
   return Fonctions[i].nam;
}

static char Rac[20]; /* longueur maxi d'une commande */
int nbLibBegin(char * b, char ** r)
{
int i,n, first=1;
unsigned int j, l,t=0;
   n=triList(0xFF);
   l=strlen(b);
   *Rac='\0';
   for (i=0;i<n;i++) {
        if (strncmp(ListFonc[i],b,l)>0) break;
        if (strncmp(ListFonc[i],b,l)==0) {
           t++;
           if (first) {
              strcpy(Rac,ListFonc[i]);
              first=0;
           } else {
              if (strlen(Rac) > l) {
                 for(j=l;j<strlen(Rac);j++)
                     if (strncmp(Rac,ListFonc[i],j) != 0)  break;
                 if (j!=strlen(Rac)) Rac[j-1]='\0';
                 else 
                    if (Rac[j-1] != ListFonc[i][j-1]) Rac[j-1]='\0';
              }
           }
        }
   }
   if (strlen(Rac) > l) *r = Rac;
   else *r = NULL;
   return(t);
}

void IF_show_liball(void)
{
   Show_library(triList(0xFF));
}
void IF_show_libstd(void)
{
   Show_library(triList(F_CORE));
}
void IF_show_libmath(void)
{
   Show_library(triList(F_MATH));
}
void IF_show_libprog(void)
{
   Show_library(triList(F_PROG));
}
void IF_show_libtools(void)
{
   Show_library(triList(F_TOOL));
}
void IF_show_libdev(void)
{
   Show_library(triList(F_DEV));
}
void IF_show_libnet(void)
{
   Show_library(triList(F_NET));
}
void IF_show_libusr(void)
{
   Show_library(triList(F_USR));
}

void initLib(void)
{
    addFonc(".",IF_point);
    addFonc("+",IF_plus);
    addFonc("-",IF_moins);
    addFonc("=",IF_Legal);
    addFonc("<>",IF_Ldiff);
    addFonc(">",IF_Lsup);
    addFonc("<",IF_Linf);
    addFonc(">=",IF_Lsupeg);
    addFonc("<=",IF_Linfeg);
    addFonc("*",IF_mult);
    addFonc("/",IF_div);
    addFonc("neg",IF_neg);
    addFonc("min",IF_min);
    addFonc("max",IF_max);
    addFonc("modulo",IF_modulo);
    addFonc("**",IF_puiss);
    addFonc("[]-sub",IF_subTab);
    addFonc("[]sub",IF_subTabR);
    addFonc("*[]-sub",IF_NsubTab);
    addFonc("*[]sub",IF_NsubTabR);
    addFonc("[]rev",IF_TabRev);
    addFonc("*[]rev",IF_NTabRev);
    addFonc("[]crot",IF_TabTransp);
    addFonc("[]transp",IF_TabTranspN);
    addFonc("[]trot",IF_TabTranspT);
    addFonc("[]>>",IF_TNShiftR);
    addFonc("[]<<",IF_TNShiftL);
    addFonc("*[]>>",IF_NTNShiftR);
    addFonc("*[]<<",IF_NTNShiftL);
    addFonc("[]>",IF_TShiftR);
    addFonc("[]<",IF_TShiftL);
    addFonc("*[]>",IF_NTShiftR);
    addFonc("*[]<",IF_NTShiftL);
    addFonc("[]min",IF_TABMin);
    addFonc("[]max",IF_TABMax);
    addFonc("[]sum",IF_TABSum);
    addFonc("[]prod",IF_TABProd);
    addFonc("[]min/max",IF_TABMinMax);
    addFonc(">array",IF_toArray);
    addFonc(">scalar",IF_toScalar);
    addFonc(">-scalar",IF_toScalarR);
    addFonc("\"",IF_debString);
    addFonc("\"drop",IF_dropC);
    addFonc("\"dup",IF_dupC);
    addFonc("\"over",IF_overC);
    addFonc("\"swap",IF_swapC);
    addFonc("\"type",IF_typeC);
    addFonc("\"cat",IF_catC);
    addFonc("\"cats",IF_catsC);
    addFonc("cr",IF_crC);
    addFonc("\"time",IF_timeC);
    addFonc("\"date",IF_dateC);
    addFonc("sleep",IF_sleep);
    addFonc("sh",IF_sh);
    addFonc("?drop",IF_dropL);
    addFonc("?dup",IF_dupL);
    addFonc("?swap",IF_swapL);
    addFonc("?over",IF_overL);
    addFonc("?t/f",IF_typeL);
    addFonc("and",IF_andL);
    addFonc("or",IF_orL);
    addFonc("xor",IF_xorL);
    addFonP("fdrop",rmLastFct);
    addFonE("del_func",IF_delFct, F_PROG);
    addFonE("del_afunc",IF_delAFct, F_PROG);
    addFonE("del_ofunc",IF_delOFct, F_PROG);
    addFonE("fscan",IF_scanFct, F_PROG);
    addFonc("?cs",IF_show_stackC);
    addFonP("?f",IF_show_stackF);
    /* addFonP("?l",IF_showFD); for debugging */
    addFonP("?v",IF_show_stackV);
    addFonE("del_var",IF_delVar,F_PROG);
    addFonP("vdrop",rmLastVar);
    addFonP("reset_var",IF_setVarI);
    addFonP(">v",IF_setVarN);
    addFonP("?>v",IF_setVarB);
    addFonP("\">v",IF_setVarC);
    addFonP("in",IF_setVarLF);
    addFonP("install",IF_INSTALL);
    addFonP("install_f",IF_INSTALLF);
    addFonP("install_v",IF_INSTALLV);
    addFonP("df_init",IF_DF_INIT);
    addFonP("df_start",IF_DF_START);
    addFonP("df_stop",IF_DF_STOP);
    addFonc("?ls",IF_show_stackL);
    addFonc("?s",IF_show_stack);
    addFonc("?libs",IF_show_liball);
    addFonc("?lib",IF_show_libstd);
    addFonc("?libM",IF_show_libmath);
    addFonc("?libT",IF_show_libtools);
    addFonc("?libP",IF_show_libprog);
    addFonc("?libD",IF_show_libdev);
    addFonc("?libN",IF_show_libnet);
    addFonc("?libU",IF_show_libusr);
    addFonc("?lasterr",IF_showError);
    addFonc("?err",IF_IsError);
    addFonc("noerr",IF_NoError);
    addFonc("messerr",IF_LibError);
    addFonc("ls_clear",IF_stackL_clear);
    addFonc("cs_clear",IF_stackC_clear);
    addFonc("REAL",IF_REAL);
    addFonc("INTEGER",IF_INTEGER);
    addFonc("echo_on",IF_ECHOON);
    addFonc("echo_off",IF_ECHOFF);
    addFonc("DEBUG_I/O",D_Update);
    addFonc("NBTAB",IF_NBTAB);
    addFonc("NBLIG",IF_NBLIG);
    addFonE("Var",IF_debVar, F_PROG);
    addFonc("\"Var",IF_debVarCS);
    addFonP("var_off",IF_VAROFF);
    addFonP("var_up",IF_VARUP);
    addFonP("var_down",IF_VARDOWN);
    addFonc("?vars",IF_vars);
    addFonc("drop",IF_drop);
    addFonc("dup",IF_dup);
    addFonc("swap",IF_swap);
    addFonc("over",IF_over);
    addFonc("pick",IF_pick);
    addFonc("rot",IF_rot);
    addFonc("unrot",IF_unrot);
    addFonc("roll",IF_roll);
    addFonc("unroll",IF_unroll);
    addFonc("*drop",IF_Ndrop);
    addFonc("*dup",IF_Ndup);
    addFonc("depth",IF_depth);
    addFonc("exit",IF_exit);
    addFonc("false",IF_false);
    addFonc("not",negBool);
    addFonc("ramp",IF_ramp);
    addFonc("dramp",IF_dramp);
    addFonc("rusage",IF_resUsage);
    addFonc("s_clear",IF_stack_clear);
    addFonM("inv",IF_inv);
    addFonM("sqrt",IF_sqrt);
    addFonM("cbrt",IF_cbrt);
    addFonM("round",IF_round);
    addFonM("floor",IF_floor);
    addFonM("ceil",IF_ceil);
    addFonM("sgn",IF_sgn);
    addFonM("abs",IF_abs);
    addFonM("pi",IF_pi);
    addFonM("sin",IF_sin);
    addFonM("cos",IF_cos);
    addFonM("tan",IF_tan);
    addFonM("asin",IF_asin);
    addFonM("acos",IF_acos);
    addFonM("atan",IF_atan);
    addFonM("sinh",IF_sinh);
    addFonM("cosh",IF_cosh);
    addFonM("tanh",IF_tanh);
    addFonM("asinh",IF_asinh);
    addFonM("acosh",IF_acosh);
    addFonM("atanh",IF_atanh);
    addFonM("ln",IF_ln);
    addFonM("log",IF_log);
    addFonM("exp",IF_exp);
    addFonM("j0",IF_j0);
    addFonM("j1",IF_j1);
    addFonM("y0",IF_y0);
    addFonM("y1",IF_y1);
    addFonc("time",IF_time);
    addFonc("true",IF_true);
    addFonc("about",IF_about);
    addFonc("vers",IF_vers);
    addFonE("load",IF_Load, F_PROG);
    addFonP("\"load",IF_LoadCS);
    addFonP("\"exec",IF_ExecCS);
    addFonP("\"execf",IF_ExecCSf);
    addInst("\"execk",IF_EXEK);
    addFonG(">csv",IF_toCsv);
    addFonG("y_xgraph",IF_yXgraph);
    addFonG("yt_xgraph",IF_ytXgraph);
    addFonG("xy_xgraph",IF_xyXgraph);
    addFonG("xyt_xgraph",IF_xytXgraph);
    addFonG("?gp",IF_show_stackGP);
    addFonG("gplot",IF_gplot_new);
    addFonG("gplotM",IF_gplot_newM);
    addFonG("gplotRaz",IF_delAllGP);
    addFonG("gplotCmd",IF_gplot_commapp);
    addFonG("gplotAdd",IF_gplot_append);
    addFonG("gplotRepl",IF_gplot_replace);
    addFonG("del_gplot",IF_gplot_del);
    addFonG("gplotClear",IF_gplot_clear);
    addFonP(":",IF_debFct);
    addFonP(":!",IF_debFctS);
    addFonP("Task",IF_NewTask);
    addFonP("?t",IF_show_Tasks);
    addFonP("?task_run",IF_statusTask);
    addFonP("del_task",IF_delTask);
    addFonP("\"f",IF_execCS);
    addFonP("\"v",IF_execCSv);
    addFonP("\"f?",IF_execCSl);
    addFonP("\"v?",IF_execCSvl);
    addFonP("stop_task",IF_stopTask);
    addFonP("?console",IF_showCons);
    addInst(";",IF_finFct);
    addInst("'",IF_debBackC);
    addInst("`",IF_debBackC1);
    addInst("return",IF_RET);
    addInst("if",IF_IF);
    addInst("else",IF_ELSE);
    addInst("then",IF_THEN);
    addInst("begin",IF_BEGIN);
    addInst("again",IF_AGAIN);
    addInst("until",IF_UNTIL);
    addInst("while",IF_WHILE);
    addInst("repeat",IF_REPEAT);
    addInst("do",IF_DO);
    addFonc("do_leave",IF_DO_Leave);
    addFonc("*do_leave",IF_DO_MLeave);
    addFonc("do_next",IF_DO_Next);
    /* addFonc("?do",IF_DO_Show); for debugging */
    addFonc("ndo",IF_nDO);
    addInst("loop",IF_LOOP);
    addInst("+loop",IF_PLOOP);
    addInst("I",IF_I_DO);
    addInst("J",IF_J_DO);
    addInst("K",IF_K_DO);
    addInst("break",IF_BREAK);
    addInst("myself",IF_MYSELF);
    addInst("onerr:",IF_ONERR);
    addInst("end:",IF_END);
    addInst("goto_end",IF_JEND);
    addFonE("help",IF_help, F_CORE);
    addFonD("?dev",IF_listDev);
    addFonD("dev_info",IF_showDev);
    addFonD("dev_read",IF_devRead);
    addFonD("dev_write",IF_devWrite);
    addFonD("dev_dflt",IF_devDflt);
    addFonD("?dev_dflt",IF_devShowDflt);
    addFonD("dev_dflW",IF_devDflW);
    addFonD("dev_dflR",IF_devDflR);
    addFonN("?n",IF_netList);
    addFonN("netOn",IF_netOn);
    addFonN("netOff",IF_netOff);
    addFonN("netDt>",IF_netDt);
    addFonN("netExec",IF_netExec);
    addFonN("netCompile",IF_netCompile);
    addFonN("ndepth",IF_netDepth);
    addFonN("stopServer",IF_netStopS);
    addFonN("NetServer",IF_NetServer);
    addFonN("srusage",IF_netRusage);
    addFonN("NetKey",IF_NetKey);
    addFonN("NetErr",IF_NetErrVal);
    addFonN("Me",IF_Me);
    addFonN("?ns",IF_netStackList);
    addFonN(">net",IF_netU2S);
    addFonN("net>",IF_netS2U);
    addFonN("ndrop",IF_netDropS);
    /* triList(); */
}

void * libByName(char * L)
{
int i;
    for (i=0;i<NBFonc;i++) {
        if (strcmp(L,Fonctions[i].nam) == 0) {
           if (Fonctions[i].typ) break;
           else return((void*)Fonctions[i].fct);
        }
    }
    return VIDE;
}

int execLibNrpc(char *C)
{
int i;
    if (sigsetjmp(ENV_INT,1)) {
       return 0;
    }
    if (IF_execFct(C)) return 1;
    for (i=0;i<NBFonc;i++) {
        if (strcmp(C,Fonctions[i].nam) == 0) {
           switch (Fonctions[i].typ) {
            case 1:
            case 3: /* usr fct */
              return 0;
              break;
            default: /* typ = 0 et 2 */
              Fonctions[i].fct();
              break;
           }
           return 1;
        }
    }
    if (IF_execVar(C)) return 1; /* VARS DOWN */
    return 0;
}

int execLib(char *C)
{
int i;
void * A;
short T=0;
    InExec = C;
    D_Trace(C);
    if (sigsetjmp(ENV_INT,1)) {
       interInfos("execLib",C);
       return 1;
    }
    if (InstallOn) {
      switch (InstallOn) {
        case 1 : /* lib first */
          A=libByName(C);
          if (A==VIDE) {
             A=fctByName(C);
             if (A!=VIDE) T=2;
          } else T=1;
          break;
        case 2 : /* user functions first */
          A=fctByName(C);
          if (A==VIDE) {
             A=libByName(C);
             if (A!=VIDE) T=1;
          } else T=2;
          break;
        case 3 : /* variables only */
          A=varByName(C);
          if (A!=VIDE) T=3;
          break;
        case 8 : /* df_init */
          A=fctByName(C);
          updDynFct(A,0);
          break;
        case 9 : /* df_start */
          A=fctByName(C);
          updDynFct(A,1);
          break;
        case 10 : /* df_stop */
          A=fctByName(C);
          updDynFct(A,2);
          break;
        default :
          break;
      }
      _MODIF_FCT_INST_(A);
      _MODIF_FCT_TYP_(T);
      InstallOn=0;
      return 1;
    }
    if ((VARS==2) && (IF_execVar(C))) return 1; /* VARS UP */
    if (IF_execFct(C)) return 1;
    for (i=0;i<NBFonc;i++) {
        /* printf("execLib : teste %s !\n", Fonctions[i].nam); */
        if (strcmp(C,Fonctions[i].nam) == 0) {
           switch (Fonctions[i].typ) {
            case 1:
              if (fctEnCours) Fonctions[i].fct();
              else messErr(13);
              break;
            case 2:
              if (fctEnCours) messErr(25);
              else Fonctions[i].fct();
              break;
            case 3: /* usr fct */
              break;
            default: /* typ = 0 */
              if (fctEnCours) {
                 if (strcmp(C,":") == 0) messErr(15);
                 else {
                    if (strcmp(C,"\"") == 0)  Fonctions[i].fct();
                    else makeFct(T_LIB,(void*)Fonctions[i].fct);
                 }
              } else Fonctions[i].fct();
              break;
           }
           return 1;
        }
    }
    if ((VARS==1) && (IF_execVar(C))) return 1; /* VARS DOWN */
    /* printf("execLib : appel putVal(%s)\n",C); */
    return(putVal(C));
}

char * libByAddr(void *A)
{
PFC f;
int i;
    f = (PFC)A;
    for (i=0;i<NBFonc;i++) {
        if (f == (PFC)(Fonctions[i].fct)) return Fonctions[i].nam;
    }
    return NULL;
}


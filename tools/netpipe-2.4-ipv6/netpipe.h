/*****************************************************************************/
/* "NetPIPE" -- Network Protocol Independent Performance Evaluator.          */
/* Copyright 1997, 1998 Iowa State University Research Foundation, Inc.      */
/*                                                                           */
/* This program is free software; you can redistribute it and/or modify      */
/* it under the terms of the GNU General Public License as published by      */
/* the Free Software Foundation.  You should have received a copy of the     */
/* GNU General Public License along with this program; if not, write to the  */
/* Free Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.   */
/*                                                                           */
/*     * netpipe.h          ---- General include file                        */
/*****************************************************************************/


#include <ctype.h>
#include <errno.h>
#include <signal.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>         /* malloc(3) */
#include <string.h>
#include <sys/types.h>
#include <sys/time.h>       /* struct timeval */
#ifdef HAVE_GETRUSAGE
#include <sys/resource.h>
#endif


#define  DEFPORT            5002
#define  TRIALS             7
#define  NSAMP              8000
#define  PERT               3
#define  LATENCYREPS        100
#define  LONGTIME           1e99
#define  CHARSIZE           8
#define  RUNTM              0.25
#define  STOPTM             1.0
#define  MAXINT             2147483647

#define     ABS(x)     (((x) < 0)?(-(x)):(x))
#define     MIN(x,y)   (((x) < (y))?(x):(y))
#define     MAX(x,y)   (((x) > (y))?(x):(y))

/* Need to include the protocol structure header file.                       */
/* Change this to reflect the protocol                                       */
#if defined(TCP)
#include "TCP.h"
#elif defined(MPI)
#include "MPI.h"
#elif defined(PVM)
#include "PVM.h"
#else
#error "One of TCP, MPI, or PVM must be defined during compilation"
#endif


typedef struct argstruct ArgStruct;
struct argstruct 
{
    /* This is the common information that is needed for all tests           */
    char     *host;         /* Name of receiving host                        */
    char     *server_host;  /* Name of sending host                          */
    int      servicefd,     /* File descriptor of the network socket         */
             commfd;        /* Communication file descriptor                 */
    short    port;          /* Port used for connection                      */
    char     *buff;         /* Transmitted buffer                            */
    char     *buff1;        /* Transmitted buffer                            */
    int      bufflen,       /* Length of transmitted buffer                  */
             tr,            /* Transmit flag                                 */
             sr,            /* Server flag                                   */
             nbuff;         /* Number of buffers to transmit                 */

    /* Now we work with a union of information for protocol dependent stuff  */
    ProtocolStruct prot;    /* Structure holding necessary info for TCP      */
};

typedef struct data Data;
struct data
{
    double t;
    double bps;
    double variance;
    int    bits;
    int    repeat;
};

double When();

int Setup(ArgStruct *p);

void Sync(ArgStruct *p);

void PrepareToReceive(ArgStruct *p);

void SendData(ArgStruct *p);

void RecvData(ArgStruct *p);

void SendTime(ArgStruct *p, double *t);

void RecvTime(ArgStruct *p, double *t);

void SendRepeat(ArgStruct *p, int rpt);

void RecvRepeat(ArgStruct *p, int *rpt);

int Establish(ArgStruct *p);

int  CleanUp(ArgStruct *p);

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
/*     * MPI.c              ---- MPI calls source                            */
/*****************************************************************************/
#include    "netpipe.h"
#include    <mpi.h>

#ifdef BSEND
char *messbuff;
#define MAXBUFSIZE (10*1024*1024)
#endif


int Setup(ArgStruct *p)
{
    int nproc;

    MPI_Comm_rank(MPI_COMM_WORLD, &p->prot.iproc);
    MPI_Comm_size(MPI_COMM_WORLD, &nproc);
    {
        char s[255];
        gethostname(s,253);
        printf("%d: %s\n",p->prot.iproc,s);
        fflush(stdout);
    }
    p->prot.nbor = !p->prot.iproc;

    if (nproc != 2)
    {
	printf("Need two processes\n");
	exit(-2);
    }

    if (p->prot.iproc == 0)
	p->tr = 1;
    else
	p->tr = 0;

#ifdef BSEND
    messbuff = (char *)malloc(MAXBUFSIZE * sizeof(char));
    if (messbuff == NULL)
    {
        printf("Can't allocate for message buffer\n");
        exit(-1);
    }
    MPI_Buffer_attach(messbuff, MAXBUFSIZE);
#endif

}

void Sync(ArgStruct *p)
{
	MPI_Barrier(MPI_COMM_WORLD);
}

static int recvPosted = 0;
static MPI_Request recvRequest;

void PrepareToReceive(ArgStruct *p)
{
	/*
	  Providing a buffer for reception of data in advance of
	  the sender sending the data provides a major performance
	  boost on some implementations of MPI, particularly shared
	  memory implementations on the Cray T3E and Intel Paragon.
	*/
	if (recvPosted)
	{
	    printf("Can't prepare to receive: outstanding receive!\n");
	    exit(-1);
	}
	MPI_Irecv(p->buff, p->bufflen, MPI_BYTE,
		  p->prot.nbor, 1, MPI_COMM_WORLD, &recvRequest);
	recvPosted = -1;
}

void SendData(ArgStruct *p)
{
#ifdef BSEND
	MPI_Bsend(p->buff, p->bufflen, MPI_BYTE, p->prot.nbor, 1, MPI_COMM_WORLD);
#else
	MPI_Send(p->buff, p->bufflen, MPI_BYTE, p->prot.nbor, 1, MPI_COMM_WORLD);
#endif
}

void RecvData(ArgStruct *p)
{
	MPI_Status status;
	if (recvPosted)
	{
		MPI_Wait(&recvRequest, &status);
		recvPosted = 0;
	}
	else
	{
		MPI_Recv(p->buff, p->bufflen, MPI_BYTE,
				p->prot.nbor, 1, MPI_COMM_WORLD, &status);
	}
}


void SendTime(ArgStruct *p, double *t)
{
#ifdef BSEND
	MPI_Bsend(t, 1, MPI_DOUBLE, p->prot.nbor, 2, MPI_COMM_WORLD);
#else
	MPI_Send(t, 1, MPI_DOUBLE, p->prot.nbor, 2, MPI_COMM_WORLD);
#endif
}

void RecvTime(ArgStruct *p, double *t)
{
	MPI_Status status;

	MPI_Recv(t, 1, MPI_DOUBLE, p->prot.nbor, 2, MPI_COMM_WORLD, &status);
}


void SendRepeat(ArgStruct *p, int rpt)
{
#ifdef BSEND
	MPI_Bsend(&rpt, 1, MPI_INT, p->prot.nbor, 2, MPI_COMM_WORLD);
#else
	MPI_Send(&rpt, 1, MPI_INT, p->prot.nbor, 2, MPI_COMM_WORLD);
#endif
}

void RecvRepeat(ArgStruct *p, int *rpt)
{
	MPI_Status status;

	MPI_Recv(rpt, 1, MPI_INT, p->prot.nbor, 2, MPI_COMM_WORLD, &status);
}


int Establish(ArgStruct *p)
{
}

int  CleanUp(ArgStruct *p)
{
	MPI_Finalize();
}

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
/*     * PVM.c              ---- PVM calls source                            */
/*****************************************************************************/
#include    "netpipe.h"
#include    <pvm3.h>

/**********************************************************************/
/* Set up the communcations system.                                   */
/*    In pvm, this means to join the parallel machine                 */
/**********************************************************************/
int
Setup(ArgStruct *p)
{
    p->prot.mytid = pvm_mytid();
#ifdef DEBUG
    printf("My task id is %d \n",p->prot.mytid);
#endif
}   

/**********************************************************************/
/* Establish a link with the other processor                          */
/*    In pvm, this means to send a simple message, but to keep the    */
/*    communication line open.  Since the assumption is that we are   */
/*    starting it by hand on the other machine, we don't know what    */
/*    the other task id is.                                           */
/**********************************************************************/
int
Establish(ArgStruct *p)
{
    /* Task information for the entire parallel machine (if trans) */
    int                     tasks_status;
    struct pvmtaskinfo      *taskp;
    int                     ntasks;

        /* Received buffer (if receiver)  */
    int buffer_id;

    /*
    If we are the transmitting side, go find the other one and send
    it a message containing our tid. If we are the receiving side,
    just wait for a message.
    */
    if ( p->tr ) {
#ifdef DEBUG
	printf("this is the transmitter\n");
#endif
	tasks_status = pvm_tasks( 0, &ntasks, &taskp );
	if ( ntasks != 2 ) {
	    printf("Error, too many processes in parallel machine \n");
	    printf("Start a clean machine.  n=%d\n", ntasks);
	    exit(-1);
	}

	/* Since there are two tasks, one is ours the other is the receiver */
	p->prot.othertid = -1;
	if ( taskp[0].ti_tid == p->prot.mytid ) {
	    p->prot.othertid = taskp[1].ti_tid;
	}
	if ( taskp[1].ti_tid == p->prot.mytid ) {
	    p->prot.othertid = taskp[0].ti_tid;
	}
	if ( p->prot.othertid == -1 ) {
	    printf("Error, cannot find other (receiving) task \n");
	    printf("Id's:  %d %d  \n",taskp[0].ti_tid,taskp[1].ti_tid);
	}

	/* Send the receiver a message.  Tell pvm to keep the channel open */

#ifdef DEBUG
	printf("The receiver tid is %d \n",p->prot.othertid);
#endif
	pvm_setopt( PvmRoute, PvmRouteDirect );
	pvm_initsend( PVMDATA );
	pvm_pkint( &p->prot.mytid, 1, 1 );
	pvm_send( p->prot.othertid, 1 );
    } else {
#ifdef DEBUG
	printf("This is the receiver \n");
#endif
                
	/* Receive any message from any task */
	buffer_id = pvm_recv(-1, -1);

	if ( buffer_id < 0 ) {
	    printf("Error on receive in receiver\n");
	    exit(-1);
	}
	pvm_upkint( &p->prot.othertid, 1, 1 );
    }
}

/**********************************************************************/
/* Prepare to receive                                                 */
/*    In pvm, you cannot set up a reception buffer ahead of time      */
/**********************************************************************/
void
PrepareToReceive(ArgStruct *p)
{
}

/**********************************************************************/
/* Synchronize                                                        */
/*     In pvm, this is not necessary                                  */
/**********************************************************************/
void
Sync(ArgStruct *p)
{
}

/**********************************************************************/
/* Send a buffer full of information                                  */
/*    In pvm, we use pvm_pkbyte and then send it.                     */
/**********************************************************************/
void
SendData(ArgStruct *p)
{
#ifdef DEBUG
    printf(" In send \n");
#endif
    pvm_initsend( PVMDATA );
    pvm_pkbyte( p->buff, p->bufflen, 1 );
    pvm_send( p->prot.othertid, 1 );
#ifdef DEBUG
    printf(" message sent.  Size=%d\n",p->bufflen);
#endif
}

/**********************************************************************/
/* Receive a buffer full of information                               */
/**********************************************************************/
void
RecvData(ArgStruct *p)
{
#ifdef DEBUG
    printf(" In receive \n");
#endif
    pvm_recv( -1, -1);
    pvm_upkbyte( p->buff, p->bufflen, 1);
#ifdef DEBUG
    printf(" message received .  Size=%d \n", p->bufflen);
#endif
}

/**********************************************************************/
/* Send elapsed time to the other process                             */
/**********************************************************************/
void
SendTime(ArgStruct *p, double *t)
{
    pvm_initsend( PVMDATA );
    pvm_pkdouble( t, 1, 1 );
    pvm_send( p->prot.othertid, 1);
}

/**********************************************************************/
/* Receive elapsed time from the other process                        */
/**********************************************************************/
void
RecvTime(ArgStruct *p, double *t)
{
    pvm_recv(-1, -1);
    pvm_upkdouble( t, 1, 1 );
}

/**********************************************************************/
/* Send repeat count to the other process                             */
/**********************************************************************/
void
SendRepeat(ArgStruct *p, int rpt)
{
    pvm_initsend( PVMDATA );
    pvm_pkint( &rpt, 1, 1 );
    pvm_send( p->prot.othertid, 1);
}

/**********************************************************************/
/* Receiver repeat count from other process                           */
/**********************************************************************/
void
RecvRepeat(ArgStruct *p, int *rpt)
{
    pvm_recv(-1, -1);
    pvm_upkint( rpt, 1, 1 );
}

/**********************************************************************/
/* Close down the connection.
/**********************************************************************/
int
CleanUp(ArgStruct *p)
{
}

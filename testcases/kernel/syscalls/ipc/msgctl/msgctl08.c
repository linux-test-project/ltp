/*
 *
 *   Copyright (c) International Business Machines  Corp., 2002
 *
 *   This program is free software;  you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 2 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY;  without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See
 *   the GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program;  if not, write to the Free Software
 *   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 */

/* 06/30/2001	Port to Linux	nsharoff@us.ibm.com */
/* 11/06/2002   Port to LTP     dbarrera@us.ibm.com */

/*
 * NAME
 *	msgctl08
 *
 * CALLS
 *	msgget(2) msgctl(2) 
 *
 * ALGORITHM
 *	Get and manipulate a message queue.
 *
 * RESTRICTIONS
 *
 */

#define _XOPEN_SOURCE 500
#include <signal.h>
#include <errno.h>
#include <string.h>
#include <fcntl.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include "test.h"
#include "usctest.h"

void setup();
void cleanup();
/*
 *  *  *  * These globals must be defined in the test.
 *   *   *   */


char *TCID="msgctl08";           /* Test program identifier.    */
int TST_TOTAL=1;                /* Total number of test cases. */
extern int Tst_count;           /* Test Case counter for tst_* routines */

int exp_enos[]={0};     /* List must end with 0 */


#define MAXNREPS	100000
#define MAXNPROCS	20
#define FAIL		1
#define PASS		0

key_t	keyarray[MAXNPROCS];

struct {
	long	type;
	struct {
		char	len;
		char	pbytes[99];
		} data;
	} buffer;

int	pidarray[MAXNPROCS];
int tid;
int nprocs, nreps;
int procstat;
int dotest(key_t key, int child_process);
int doreader(int id, long key, int child);
int dowriter(int id,long key, int child);
int fill_buffer(register char *buf, char val, register int size);
int verify(register char *buf,char val, register int size,int child);
void sig_handler();             /* signal catching function */
int mykid;

/*-----------------------------------------------------------------*/
int main(argc, argv)
int	argc;
char	*argv[];
{
	register int i, j, ok, pid;
	int count, status;

	setup();

	if (argc == 1 )
	{
		/* Set default parameters */
		nreps = MAXNREPS;
		nprocs = MAXNPROCS;
	}
	else if (argc == 3 )
	{
		if ( atoi(argv[1]) > MAXNREPS )
		{
		        tst_resm(TCONF,"\tRequested number of iterations too large, setting to Max. of %d \n", MAXNREPS);
			nreps = MAXNREPS;
		}
		else
		{
			nreps = atoi(argv[1]);
		}
		if (atoi(argv[2]) > MAXNPROCS )
		{
		        tst_resm(TCONF,"\tRequested number of processes too large, setting to Max. of %d \n", MAXNPROCS);
			nprocs = MAXNPROCS;
		}
		else
		{
			nprocs = atoi(argv[2]);
		}
	}
	else
	{
	        tst_resm(TCONF," Usage: %s [ number of iterations  number of processes ]\n", argv[0]);
		tst_exit();
	}

	srand(getpid());
	tid = -1;

	/* Setup signal handleing routine */
	if (signal(SIGTERM, sig_handler) == SIG_ERR) 
	{
                tst_resm(TFAIL, "\tSigset SIGTERM failed\n");
                tst_exit();
	}
	/* Set up array of unique keys for use in allocating message
	 * queues
	 */
	for (i = 0; i < nprocs; i++) 
	{
		ok = 1;
		do 
		{
			/* Get random key */
			keyarray[i] = (key_t)rand();
			/* Make sure key is unique and not private */
			if (keyarray[i] == IPC_PRIVATE) 
			{
				ok = 0;
				continue;
			}
			for (j = 0; j < i; j++) 
			{
				if (keyarray[j] == keyarray[i]) 
				{
					ok = 0;
					break;
				}
				ok = 1;
			}
		} while (ok == 0);
	}
	 
	/* Fork a number of processes, each of which will
	 * create a message queue with one reader/writer
	 * pair which will read and write a number (iterations)
	 * of random length messages with specific values.
	 */

	for (i = 0; i <  nprocs; i++) 
	{
		fflush(stdout);
		if ((pid = fork()) < 0) 
		{
	                tst_resm(TFAIL, "\tFork failed (may be OK if under stress)");
        	        tst_exit();
		}
		/* Child does this */
		if (pid == 0) 
		{
			procstat = 1;
			exit( dotest(keyarray[i], i) );
		}
		pidarray[i] = pid;
	}

	count = 0;
	while(1)
	{
		if (( wait(&status)) > 0)
		{
			if (status>>8 != 0 )
			{
	                	tst_resm(TFAIL, "\tChild exit status = %d \n", status>>8);
	        	        tst_exit();
			}
			count++;
		}	
		else
		{
			if (errno != EINTR)
			{	
				break;
			}
#ifdef DEBUG
		        tst_resm(TINFO,"\tSignal detected during wait \n");
#endif
		}
	}
	/* Make sure proper number of children exited */
	if (count != nprocs)
	{
                tst_resm(TFAIL, "\tWrong number of children exited, Saw %d, Expected %d \n", count, nprocs);
       	        tst_exit();
	}

        tst_resm(TPASS,"msgctl08 ran successfully!");
	 
	cleanup();
        return (0);
								
}
/*--------------------------------------------------------------------*/

int dotest(key, child_process)
key_t 	key;
int	child_process;
{
	int id, pid;

	sighold(SIGTERM);
	TEST(msgget(key, IPC_CREAT));
	if (TEST_RETURN < 0)
	{
                tst_resm(TFAIL, "\tMsgget error in child %d, errno = %d\n", child_process, TEST_ERRNO);
       	        tst_exit();
	}
	tid = id = TEST_RETURN;
	sigrelse(SIGTERM);

	fflush(stdout);
	if ((pid = fork()) < 0) 
	{
                tst_resm(TWARN, "\tFork failed (may be OK if under stress)");
		TEST(msgctl(tid, IPC_RMID, 0));
		if (TEST_RETURN < 0)
		{
                	tst_resm(TFAIL, "\tMsgctl error in cleanup, errno = %d\n", errno);
		}
       	        tst_exit();
	}
	/* Child does this */
	if (pid == 0) 
	{
		exit( doreader(id, key % 255, child_process) );
	}
	/* Parent does this */
	mykid = pid;
	procstat = 2;
	dowriter(id, key % 255, child_process);
	wait(0);
	TEST(msgctl(id, IPC_RMID, 0));
	if (TEST_RETURN < 0)
	{
                tst_resm(TFAIL, "msgctl errno %d\n", TEST_ERRNO);
       	        tst_exit();
	}
	exit(PASS);
}

int doreader(id, key, child)
int id, child;
long key;
{
	int i, size;

	for (i = 0; i < nreps; i++) 
	{
		if ((size = msgrcv(id, &buffer, 100, 0, 0)) < 0) 
		{
                	tst_brkm(TBROK, cleanup, "\tMsgrcv error in child %d, read # = %d, errno = %d\n", (i + 1), child, errno);
	       	        tst_exit();
		}
		if (buffer.data.len + 1 != size)  
		{
        	       	tst_resm(TFAIL, "\tSize mismatch in child %d, read # = %d\n", child, (i + 1));
        	       	tst_resm(TFAIL, "\t\tfor message size got  %d expected  %d %s \n",size ,buffer.data.len);
	       	        tst_exit();
		}
		if ( verify(buffer.data.pbytes, key, size - 1, child) ) 
		{
                	tst_resm(TFAIL, "\tin read # = %d,key =  %x\n", (i + 1), child, key);
	       	        tst_exit();
		}
		key++;
	}
	return (0);
}

int dowriter(id, key, child)
int id,child;
long key;
{
	int i, size;

	for (i = 0; i < nreps; i++) 
	{
		do 
		{
			size = (rand() % 99);
		} while (size == 0);
		fill_buffer(buffer.data.pbytes, key, size);
		buffer.data.len = size;
		buffer.type = 1;
		TEST(msgsnd(id, &buffer, size + 1, 0));
		if (TEST_RETURN < 0)
		{
                	tst_brkm(TBROK, cleanup, "\tMsgsnd error in child %d, key =   %x errno  = %d\n", child, key, TEST_ERRNO);
		}
		key++;
	}
	return (0);
}	

int fill_buffer(buf, val, size)
register char *buf;
char	val;
register int size;
{
	register int i;

	for(i = 0; i < size; i++)
	{
		buf[i] = val;
	}

	return (0);
}


/*
 * verify()
 *	Check a buffer for correct values.
 */

int verify(buf, val, size, child)
	register char *buf;
	char	val;
	register int size;
	int	child;
{
	while(size-- > 0)
	{
		if (*buf++ != val)
		{
                	tst_resm(TWARN, "\tVerify error in child %d, *buf = %x, val = %x, size = %d\n", child, *buf, val, size);
			return(FAIL);
		}
	}
	return(PASS);
}

/*
 *  * void
 *  * sig_handler() - signal catching function for 'SIGUSR1' signal.
 *  *
 *  *   This is a null function and used only to catch the above signal
 *  *   generated in parent process.
 *  */
void
sig_handler()
{
}

/***************************************************************
 * setup() - performs all ONE TIME setup for this test.
 *****************************************************************/
void
setup()
{
        /* You will want to enable some signal handling so you can capture
	 * unexpected signals like SIGSEGV.
	 */
        tst_sig(FORK, DEF_HANDLER, cleanup);
 

        /* Pause if that option was specified */
        /* One cavet that hasn't been fixed yet.  TEST_PAUSE contains the code to
	 * fork the test with the -c option.  You want to make sure you do this
	 * before you create your temporary directory.
	 */
        TEST_PAUSE;
}


/***************************************************************
 * cleanup() - performs all ONE TIME cleanup for this test at
 *             completion or premature exit.
 ****************************************************************/
void
cleanup()
{
	int status;
        /*
	 *  Remove the message queue from the system
	 */
#ifdef DEBUG
        tst_resm(TINFO,"Removing the message queue\n");
#endif
        fflush (stdout);
        (void) msgctl(tid, IPC_RMID, (struct msqid_ds *)NULL);
        if ((status = msgctl(tid, IPC_STAT, (struct msqid_ds *)NULL)) != -1)
        {
                (void) msgctl(tid, IPC_RMID, (struct msqid_ds *)NULL);
                tst_resm(TFAIL, "msgctl(tid, IPC_RMID) failed\n");
                tst_exit();
        }

        fflush (stdout);
        /*
	 * print timing stats if that option was specified.
	 * print errno log if that option was specified.
	 */
        TEST_CLEANUP;

        /* exit with return code appropriate for results */
        tst_exit();
}


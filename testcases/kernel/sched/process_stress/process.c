/*
 *
 *   Copyright (c) International Business Machines  Corp., 2001
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
 *   Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 */

#include <sys/mman.h>
#include <ctype.h>
#include <errno.h>
#include <signal.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#ifndef _LINUX
			/* LINUX INCLUDES */
#include <sys/mode.h>
#include <sys/timers.h>
#else
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/ipc.h>
#endif
#include <sys/msg.h>
#include <sys/resource.h>
#include <sys/select.h>
#include <sys/sem.h>
#include <sys/shm.h>
#include <sys/types.h>
#include <sys/wait.h>
#include "lapi/semun.h"

/* indexes into environment variable array */
#define ADBG 0
#define BNDX 1
#define DNDX 2
#define TNDX 3
#define MAXBVAL 70
#define MAXDVAL 11
#define SLOTDIR "./slot/"

#ifdef _LINUX
			/* LINUX #defnes */
#ifndef TRUE
#define TRUE 1
#endif
#ifndef FALSE
#define FALSE 0
#endif
#endif

#if defined _LINUX && defined DEBUG
#define prtln()	printf("At line number: %d\n", __LINE__); \
		fflush(NULL)
#define dprt(fmt, args...) printf(fmt, ## args)
#else
#define prtln()
#define dprt(fmt, args...)
#endif

/* aliases for environment variable entries */
#define    AUSDEBUG  (*edat[ADBG].eval.vint)	/* debug value */
#define    BVAL  (*edat[BNDX].eval.vint)	/* # of childern per parent */
#define    DVAL  (*edat[DNDX].eval.vint)	/* depth of process tree */
#define    TVAL  (*edat[TNDX].eval.vint)	/* timer value */

#ifdef _LINUX
typedef long mtyp_t;
#endif

/* structure of information stored about each process in shared memory */
typedef struct proc_info {
#ifdef __64LDT__
	pid_t pid;		/* process id */
	pid_t ppid;		/* parent process id */
#else
	int pid;		/* process id */
	int ppid;		/* parent process id */
#endif
	int msg;		/* parent process id */
	int err;		/* error indicator */
	int *list;		/* pointer to list of parent and sibling slot locations */
} Pinfo;

typedef struct messagebuf {
	mtyp_t mtyp;		/* message type */
	char mtext[80];		/* message text */
} Msgbuf;

union semun semarg;

/* structure of all environment variable used by program */
struct envstruct {
	char *env_name;
	union {
		char *chptr;
		int *vint;
	} eval;
} envdata[] = {
	{
		"AUSDBG", {
	"0"}}, {
		"BVAL", {
	"3"}}, {
		"DVAL", {
	"2"}}, {
		"FORCE", {
	"0"}}, {
		"TVAL", {
	"1"}}, {
		"", {
	""}}
};

char *errfile;			/* pointer to errfile name */

int msgid;			/* message queue for leaf nodes */
int msgerr;			/* message queue for errors */
int nodesum;			/* total number of process to be created */
int sem_count;			/* counter semaphore */
int sem_lock;			/* locks access to counter semaphore */
int shmid;			/* global shared memory id varible */
int procgrp;			/* process group id */

timer_t timer;			/* timer structure */

Pinfo *shmaddr;			/* Start address  of shared memory */

#ifndef _LINUX
FILE *errfp = stderr;		/* error file pointer, probably not necessary */
FILE *debugfp = stderr;		/* debug file pointer, used if AUSDEBUG set */
#else
#define errfp stderr
#define debugfp stderr
#endif

struct envstruct *edat = envdata;	/* pointer to environment data */

/* external function declarations */
extern int killpg(int procgrp, int sig);
extern timer_t gettimerid(int Timer_type, int Notify_type);
extern int reltimerid(timer_t timer);

/* internal function declarations */
void cleanup(int sig, int code, struct sigcontext *scp);
void nextofkin(int sig, int code, struct sigcontext *scp);
void doit(void);
void debugout(char *fmt, ...);
int getenv_val(void);
void messenger(void);
void nextofkin(int sig, int code, struct sigcontext *scp);
int notify(int slot);
void parse_args(int argc, char *argv[]);
void print_shm(void);
Pinfo *put_proc_info(int tval);
void rm_msgqueue(void);
void rm_semseg(void);
void rm_shmseg(void);
int semoper(int slot, int smid, int opval);
int send_message(int id, mtyp_t type, char *text);
void set_timer(void);
void set_signals(void *sighandler());
void setup_msgqueue(void);
void setup_semaphores(void);
void setup_shm(void);
void severe(char *fmt, ...);
Pinfo *shmgetseg(void);
int spawn(int val);
unsigned long sumit(int B, int D);

/*
 *  Prints out the data structures in shared memory.
 */
void print_shm(void)
{
	extern int nodesum;	/* total number of nodes created */
	extern Pinfo *shmaddr;	/* shared memory pointer */
	extern int shmid;	/* shared memory id */

	Pinfo *pinfo;		/* pointer to process info in shared memory */
	int *listp;		/* pointer to sibling info in shared memory */
	int i, j;		/* counters */
	struct shmid_ds buf;

	if (shmctl(shmid, IPC_STAT, &buf))
		return;

	for (pinfo = shmaddr, i = 0; i < nodesum; i++, pinfo++) {
		fprintf(errfp,
			"slot: %-4d pid: %-6d ppid: %-6d msg: %-2d err: %-2d lst:",
			i, pinfo->pid, pinfo->ppid, pinfo->msg, pinfo->err);
		for (j = 0, listp = pinfo->list; j < BVAL; j++, listp++)
			fprintf(errfp, " %d", *listp);
		fprintf(errfp, "\n");
	}
}

/*
 *  Generalized send routine.  Sends a message on message queue.
 */
int send_message(int id, mtyp_t type, char *text)
{
	int rc;

	Msgbuf sndbuf;

	strcpy(sndbuf.mtext, text);
	sndbuf.mtyp = type;
	while (TRUE) {
		rc = msgsnd(id, &sndbuf, sizeof(struct messagebuf), IPC_NOWAIT);
		if (rc == -1 && errno == EAGAIN) {
			debugout("msgqueue %d of mtyp %d not ready to send\n",
				 msgid, type);
			errno = 0;
		} else
			return (rc);
	}
}

/*
 *  Sends error message to initial parent (messenger).i
 */
void severe(char *fmt, ...)
{
	va_list args;
	int rc;
	char mtext[80];
	extern int msgerr;

	va_start(args, fmt);
	vsprintf(mtext, fmt, args);
	va_end(args);

	rc = send_message(msgerr, 2, mtext);
	if (rc == -1) {
		perror("cannot send message to msgerr");
		exit(1);
	}
}

/*
 *  if AUSDEBUG set will print information to file associated with slot number.
 */
void debugout(char *fmt, ...)
{
	va_list args;

	if (AUSDEBUG) {
		va_start(args, fmt);
		vfprintf(debugfp, fmt, args);
		va_end(args);
	}
}

/*
 *  Remove message queues.
 */
void rm_msgqueue(void)
{
	extern int msgid;

	/* remove message queue id. */
	if (msgctl(msgid, IPC_RMID, NULL) && errno != EINVAL) {
		fprintf(errfp, "msgctl failed msgid: errno %d\n", errno);
		perror("msgctl failed");
	}

	/* remove message queue id. */
	if (msgctl(msgerr, IPC_RMID, NULL) && errno != EINVAL) {
		fprintf(errfp, "msgctl failed msgerr: errno %d\n", errno);
		perror("msgctl failed");
	}
}

/*
 *  Remove shared memory segment.
 */
void rm_shmseg(void)
{
	extern int shmid;	/* Global shared memory id */
	extern Pinfo *shmaddr;	/* Global shared memory address */

	/* remove shared memory id (and shared memory segment). */
	if (shmctl(shmid, IPC_RMID, NULL) && errno != EINVAL) {
		fprintf(errfp, "shmctl failed: errno %d\n", errno);
		perror("shmctl failed");
	}
}

/*
 *  Remove semaphores.
 */
void rm_semseg(void)
{
	extern int sem_lock;
	extern int sem_count;

	/* remove sem_lock semaphore id */
	semarg.val = 0;		/* to fix problem with 4th arg of semctl in 64 bits MARIOG */
	if (semctl(sem_lock, 0, IPC_RMID, semarg.val) && errno != EINVAL) {
		fprintf(errfp, "semctl failed: errno %d\n", errno);
		perror("semctl failed");
	}
	/* remove sem_count semaphore id. */
	semarg.val = 0;		/* to fix problem with 4th arg of semctl in 64 bits MARIOG */
	if (semctl(sem_count, 0, IPC_RMID, semarg.val) && errno != EINVAL) {
		fprintf(errfp, "semctl failed: errno %d\n", errno);
		perror("semctl failed");
	}
}

/*
 * Routine to clean up shared memory and return exit status (CHILD handler).
 */
void cleanup(int sig, int code, struct sigcontext *scp)
{
	int rc;
	char mtext[80];

	killpg(procgrp, SIGTERM);
	sprintf(mtext, "%d", sig);
	rc = send_message(msgerr, 3, mtext);
	if (rc == -1) {
		severe("msgsnd failed: %d msgid %d mtyp %d mtext %d\n",
		       errno, msgerr, 3, mtext);
	}
}

/*
 * Routine to clean up shared memory and return exit status (PARENT handler).
 */
void nextofkin(int sig, int code, struct sigcontext *scp)
{
	int rc;
	char mtext[80];

	sprintf(mtext, "%d", sig);
	rc = send_message(msgerr, 3, mtext);
	if (rc == -1) {
		severe("msgsnd failed: %d msgid %d mtyp %d mtext %d\n",
		       errno, msgerr, 3, mtext);
	}
#ifndef _LINUX
	reltimerid(timer);
#endif
	exit(1);
}

/* given breadth and depth of a tree, sum up total number of nodes created */
unsigned long sumit(int B, int D)
{
	int i;
	int exp = 1;		/* exponent of breadth */
	unsigned long sum = 1;	/* running sum of nodes */

	for (sum = 1, i = 1; i <= D; i++) {
		exp = B * exp;
		sum += (int)exp;
	}
	return (sum);
}

/* Finds correct slot for current process in shared memory and stores
 * information about process in it.
 */
Pinfo *put_proc_info(int tval)
{
	extern int nodesum;
	extern Pinfo *shmaddr;

	int sibslot = 0;	/* sibling slot number */
	int *listp;		/* ptr to sibling info for current proc */
	Pinfo *smp;		/* ptr to current process data slot */

	smp = shmaddr + tval;
	smp->pid = getpid();
	smp->ppid = getppid();
	smp->err = 0;
	smp->msg = 0;

	/* if very first process (slot 0), dont fill in info about siblings
	 *  and parent.  Sibling and parent info is irrevelant in this case.
	 */
	if (!tval)
		return (smp);

	/* find parent of current process and store slot location */
	smp->list = (int *)(Pinfo *) (shmaddr + nodesum) + (BVAL * tval);
	*smp->list = (tval - 1) / BVAL;
	listp = smp->list + 1;

	/* calculate and store sibling slot numbers of current process */
	for (sibslot = *smp->list * BVAL + 1; listp < smp->list + BVAL;
	     sibslot++) {
		if (tval != sibslot)
			*(listp++) = sibslot;
	}
	return (smp);
}

/* This routine sends a message from the current process to all of her
 * siblings and then waits to receive responses from them.  A timer is
 * set so that if a message is lost or not received for some reason
 * we can exit gracefully.
 */
int notify(int slot)
{
	extern int msgid;
	extern Pinfo *shmaddr;

	int i;
	int rc;
	int tslot;
	int *listp = (shmaddr + slot)->list;
	int cldcnt = 1;
	int ndx = 0;
#ifdef __64LDT__
	pid_t pid = 0;
#else
	int pid = 0;
#endif
	char mtext[80];

	Msgbuf rcvbuf;

	for (i = 1, listp++; i < BVAL; i++, listp++) {
		sprintf(mtext, "%d %d %d", i, slot, (shmaddr + slot)->pid);
		rc = send_message(msgid, (mtyp_t) * listp, mtext);
		if (rc == -1) {
			severe
			    ("notify: send_message Failed: %d msgid %d mtyp %d mtext %d\n",
			     errno, msgid, *listp, mtext);
			exit(1);
		}
	}

	while (cldcnt < BVAL) {
		rc = msgrcv(msgid, &rcvbuf, sizeof(struct messagebuf), slot, 0);
		if (rc == -1) {
			switch (errno) {
			case EAGAIN:
				printf("msgqueue %d not ready to receive\n",
				       msgid);
				fflush(stdout);
				errno = 0;
				break;
			case ENOMSG:
				printf("msgqueue %d no message\n", msgid);
				fflush(stdout);
				errno = 0;
				break;
			default:
				perror("msgrcv failed");
				severe("msgrcv failed, errno: %d\n", errno);
				exit(1);
			}
		} else {
			sscanf(rcvbuf.mtext, "%d %d %d", &ndx, &tslot, &pid);
			if (*((shmaddr + tslot)->list + ndx) == slot &&
			    (shmaddr + tslot)->pid == pid) {
				debugout
				    ("MSGRCV:slot: %d ndx: %d tslot: %d pid: %d\n",
				     slot, ndx, tslot, pid);
				(shmaddr + slot)->msg++;
				cldcnt++;
			} else {
				(shmaddr + slot)->err--;
				debugout
				    ("MSGRCV: slot: %d ndx: %d tslot: %d pid: %d\n",
				     slot, ndx, tslot, pid);
			}
		}
	}
	return 0;
}

/*
 * Calculates semaphore number and sets semaphore (lock).
 */
int semoper(int slot, int smid, int opval)
{
	int pslot;		/* parent slot */
	struct sembuf smop;	/* semaphore operator */

	pslot = (slot - 1) / BVAL;	/* calculate parent node */
	smop.sem_num = pslot;
	smop.sem_op = opval;
	smop.sem_flg = 0;
	semop(smid, &smop, 1);
	return (pslot);
}

/*
 * This is the meat and potatoes of the program.  Spawn creates a tree
 * of processes with Dval depth and Bval breadth.  Each parent will spawn
 * Bval children.  Each child will store information about themselves
 * in shared memory.  The leaf nodes will communicate the existence
 * of one another through message queues, once each leaf node has
 * received communication from all of her siblings she will reduce
 * the semaphore count and exit.  Meanwhile all parents are waiting
 * to hear from their children through the use of semaphores.  When
 * the semaphore count reaches zero then the parent knows all the
 * children have talked to one another.  Locking of the connter semaphore
 * is provided by the use of another (binary) semaphore.
 */
int spawn(int val)
{
	extern int sem_count;	/* used to keep track of childern */
	extern int sem_lock;	/* used to lock access to sem_count semaphore */

	int i;			/* Breadth counter */
	static int level = 0;	/* level counter */
	int lvlflg = 0;		/* level toggle, limits parental spawning
				   to one generation */
	int pslot = 0;
#ifdef __64LDT__
	pid_t pid;		/* pid of child process */
#else
	int pid;		/* pid of child process */
#endif
	Pinfo *pinfo;		/* pointer to process information in shared mem */
	int semval;		/* value of semaphore ( equals BVAL initially */
	static int tval = 1;	/* tree node value of child. */

	char foo[1024];

	level++;

	for (i = 1; i <= BVAL; i++) {
		tval = (val * BVAL) + i;
		if (!lvlflg) {
			pid = fork();
			if (!pid) {	/* CHILD */
				if (AUSDEBUG) {
					sprintf(foo, "%sslot%d", SLOTDIR, tval);
					debugfp = fopen(foo, "a+");
				}
				pinfo = put_proc_info(tval);

				debugout
				    ("pid: %-6d ppid: %-6d lev: %-2d i: %-2d val: %-3d\n",
				     pinfo->pid, pinfo->ppid, level, i, tval);

				set_timer();	/* set up signal handlers and initialize pgrp */
				if (level < DVAL) {
					if (spawn(tval) == -1) {
						pslot =
						    semoper(tval, sem_lock, -1);
						semarg.val = 0;	/* to fix problem with 4th arg of semctl in 64 bits MARIOG */
						semval =
						    semctl(sem_count, pslot,
							   GETVAL, semarg);
						semarg.val = --semval;	/* to fix problem with 4th arg of semctl in 64 bits MARIOG */
						semctl(sem_count, pslot, SETVAL,
						       semarg);
						semarg.val = 1;	/* to fix problem with 4th arg of semctl in 64 bits MARIOG */
						semctl(sem_lock, pslot, SETVAL,
						       semarg);
					}
					lvlflg++;
				} else {	/* leaf node */
					notify(tval);
					return (-1);
				}
			}
#ifdef __64LDT__
			else if (pid > 0 && i >= BVAL) {	/* PARENT */
#else
			else if (pid > (pid_t) 0 && i >= BVAL) {	/* PARENT */
#endif
				pslot = semoper(tval, sem_count, 0);
				pslot = semoper(pslot, sem_lock, -1);
				semarg.val = 0;	/* to fix problem with 4th arg of semctl in 64 bits MARIOG */
				semval =
				    semctl(sem_count, pslot, GETVAL, semarg);
				semarg.val = --semval;	/* to fix problem with 4th arg of semctl in 64 bits MARIOG */
				semctl(sem_count, pslot, SETVAL, semarg);
				semarg.val = 1;	/* to fix problem with 4th arg of semctl in 64 bits MARIOG */
				semctl(sem_lock, pslot, SETVAL, semarg);
				(shmaddr + val)->msg++;
			}
#ifdef __64LDT__
			else if (pid < (pid_t) 0) {
#else
			else if (pid < 0) {
#endif
				perror("spawn: fork failed");
				severe
				    ("spawn: fork failed, exiting with errno %d\n",
				     errno);
				exit(1);
			} else
				(shmaddr + val)->msg++;
		}
	}
	return (pslot);
}

/*
 * Allocate message queues.
 */
void setup_msgqueue(void)
{
	extern int msgid;
	extern int msgerr;

	msgid = msgget(IPC_PRIVATE,
		       IPC_CREAT | IPC_EXCL | S_IRUSR | S_IWUSR | S_IRGRP |
		       S_IWGRP);
	if (msgid == -1) {
		perror("msgget msgid failed");
		fprintf(stderr, " SEVERE : msgget msgid failed: errno %d\n",
			errno);
		exit(1);
	}

	msgerr = msgget(IPC_PRIVATE,
			IPC_CREAT | IPC_EXCL | S_IRUSR | S_IWUSR | S_IRGRP |
			S_IWGRP);
	if (msgerr == -1) {
		perror("msgget msgerr failed");
		fprintf(stderr, " SEVERE : msgget msgerr failed: errno %d\n",
			errno);
		exit(1);
	}
}

/*
 * Set up and initialize all semaphores
 */
void setup_semaphores(void)
{
	extern int sem_count;
	extern int sem_lock;

	int i;
	int rc;

	prtln();
	sem_lock = semget(IPC_PRIVATE, nodesum - 1,
			  IPC_CREAT | IPC_EXCL | S_IRUSR | S_IWUSR | S_IRGRP |
			  S_IWGRP);
	dprt("nodesum = %d, sem_lock = %d\n", nodesum, sem_lock);

	prtln();
	if (sem_lock == -1) {
		perror("semget failed for sem_lock");
		fprintf(stderr,
			" SEVERE : semget failed for sem_lock, errno: %d\n",
			errno);
		rm_shmseg();
		exit(1);
	}

	prtln();
	sem_count = semget(IPC_PRIVATE, nodesum - 1,
			   IPC_CREAT | IPC_EXCL | S_IRUSR | S_IWUSR | S_IRGRP |
			   S_IWGRP);

	if (sem_count == -1) {
		perror("semget failed for sem_count");
		fprintf(stderr,
			" SEVERE : semget failed for sem_count, errno: %d\n",
			errno);
		rm_shmseg();
		exit(1);
	}
	prtln();

	for (i = 0; i < (nodesum - 1); i++) {
		semarg.val = 1;	/* to fix problem with 4th arg of semctl in 64 bits MARIOG */
		rc = semctl(sem_lock, i, SETVAL, semarg);
		prtln();
		if (rc == -1) {
			perror("semctl failed for sem_lock failed");
			fprintf(stderr,
				" SEVERE : semctl failed for sem_lock, errno: %d\n",
				errno);
			rm_shmseg();
			exit(1);
		}

		semarg.val = BVAL;	/* to fix problem with 4th arg of semctl in 64 bits MARIOG */
		rc = semctl(sem_count, i, SETVAL, semarg);
		prtln();
		if (rc == -1) {
			perror("semctl failed for sem_lock failed");
			fprintf(stderr,
				" SEVERE : semctl failed for sem_lock, errno: %d\n",
				errno);
			rm_shmseg();
			exit(1);
		}
	}
}

/*
 * Set up and allocate shared memory.
 */
void setup_shm(void)
{
	extern int nodesum;	/* global shared memory id */
	extern int shmid;	/* global shared memory id */
	extern Pinfo *shmaddr;

	int i, j;		/* counters */
	Pinfo *shmad = NULL;	/* ptr to start of shared memory. */
	Pinfo *pinfo = NULL;	/* ptr to struct in shared memory. */

	debugout("size = %d, size (in hex) =  %#x  nodes: %d\n",
		 sizeof(Pinfo) * nodesum + (nodesum * BVAL * sizeof(int)),
		 sizeof(Pinfo) * nodesum + (nodesum * BVAL * sizeof(int)),
		 nodesum);

	/* Get shared memory id */
	shmid = shmget(IPC_PRIVATE,
		       sizeof(Pinfo) * nodesum + (nodesum * BVAL * sizeof(int)),
		       IPC_CREAT | IPC_EXCL | S_IRUSR | S_IWUSR | S_IRGRP |
		       S_IWGRP);
	if (shmid < 0) {
		perror("shmget failed");
		fprintf(stderr, " SEVERE : shmget failed: errno %d\n", errno);
		exit(1);
	}

	/* allocate shared memory */

	if ((shmad = shmat(shmid, (char *)shmad, 0)) == MAP_FAILED) {
		printf("SEVERE : shmat failed\n");
		exit(1);
	} else {
		shmctl(shmid, IPC_RMID, NULL);
	}

	/* set all fields in shared memory to -1 */
	for (pinfo = shmad, i = 0; i < nodesum; i++, pinfo++) {
#ifdef __64LDT__
		pinfo->pid = (pid_t) - 1;
		pinfo->ppid = (pid_t) - 1;
#else
		pinfo->pid = -1;
		pinfo->ppid = -1;
#endif
		pinfo->msg = -1;
		pinfo->err = -1;

		/* Changed 10/9/97 */
		/* pinfo->list = (int *)((ulong)shmad + nodesum * sizeof(Pinfo)
		   + (sizeof(int) * BVAL * i)); */
		pinfo->list =
		    (int *)((long)shmad + nodesum * sizeof(Pinfo) +
			    (sizeof(int) * BVAL * i));
		for (j = 0; j < BVAL; j++)
			*(pinfo->list + j) = -1;
	}
	shmaddr = shmad;
}

/*
 * Set up Signal handler and which signals to catch
 */
void set_signals(void *sighandler())
{
	int i;
	int rc;

	struct sigaction action;

	/* list of signals we want to catch */
	static struct signalinfo {
		int signum;
		char *signame;
	} siginfo[] = {
		{
		SIGHUP, "SIGHUP"}, {
		SIGINT, "SIGINT"}, {
		SIGQUIT, "SIGQUIT"}, {
		SIGABRT, "SIGABRT"}, {
		SIGBUS, "SIGBUS"}, {
		SIGSEGV, "SIGSEGV"}, {
		SIGALRM, "SIGALRM"}, {
		SIGUSR1, "SIGUSR1"}, {
		SIGUSR2, "SIGUSR2"}, {
		-1, "ENDSIG"}
	};

	char tmpstr[1024];

	action.sa_handler = (void *)sighandler;

#ifdef _LINUX
	sigfillset(&action.sa_mask);
#else
	SIGINITSET(action.sa_mask);
#endif
	action.sa_flags = 0;

	/* Set the signal handler up */
#ifdef _LINUX
	sigaddset(&action.sa_mask, SIGTERM);
#else
	SIGADDSET(action.sa_mask, SIGTERM);
#endif
	for (i = 0; siginfo[i].signum != -1; i++) {
#ifdef _LINUX
		sigaddset(&action.sa_mask, siginfo[i].signum);
#else
		SIGADDSET(action.sa_mask, siginfo[i].signum);
#endif
		rc = sigaction(siginfo[i].signum, &action, NULL);
		if (rc == -1) {
			sprintf(tmpstr, "sigaction: %s\n", siginfo[i].signame);
			perror(tmpstr);
			fprintf(stderr,
				" SEVERE : Could not set %s signal action, errno=%d.",
				siginfo[i].signame, errno);
			exit(1);
		}
	}
}

/*
* Get and set a timer for current process.
*/
#ifndef _LINUX
void set_timer(void)
{
	struct itimerstruc_t itimer, old_itimer;

	if ((timer = gettimerid(TIMERID_REAL, DELIVERY_SIGNALS)) == -1) {
		perror("gettimerid");
		fprintf(stderr, " SEVERE : Could not get timer id, errno=%d.",
			errno);
		exit(1);
	}

	/*
	 * Start the timer.
	 */
	itimer.it_interval.tv_nsec = 0;
	itimer.it_interval.tv_sec = 0;
	itimer.it_value.tv_nsec = 0;
	itimer.it_value.tv_sec = (time_t) (TVAL * 60.0);
	if (incinterval(timer, &itimer, &old_itimer) == -1) {
		perror("incinterval");
		fprintf(stderr,
			" SEVERE : Could not set timer interval, errno=%d.",
			errno);
		(void)reltimerid(timer);
		exit(1);
	}
}
#else

void set_timer(void)
{
	struct itimerval itimer;

	memset(&itimer, 0, sizeof(struct itimerval));
	/*
	 * Start the timer.
	 */
	itimer.it_interval.tv_usec = 0;
	itimer.it_interval.tv_sec = 0;
	itimer.it_value.tv_usec = 0;
	itimer.it_value.tv_sec = (time_t) (TVAL * 60.0);

	if (setitimer(ITIMER_REAL, &itimer, NULL)) {
		perror("setitimer");
		exit(1);
	}
}
#endif

/*
 * parse_args
 *
 * Parse command line arguments.  Any errors cause the program to exit
 * at this point.
 */
void parse_args(int argc, char *argv[])
{
	int i;
	int opt, errflag = 0;
	int dflag = 0, bflag = 0, fflag = 0, tflag = 0;
	extern int optind;
	extern char *optarg;

	/* DVAL:        0  1     2      3   4  5  6  7  8  9  10 11 */
	int limits[] = { -1, -1, MAXBVAL, 17, 8, 5, 4, 3, 2, 2, 2, 2 };

	while ((opt = getopt(argc, argv, "b:d:ft:D?")) != EOF) {
		switch (opt) {
		case 'b':
			if (bflag)
				errflag++;
			else {
				bflag++;
				errno = 0;
				BVAL = atoi(optarg);
				if (errno) {
					perror("atoi");
					fprintf(stderr,
						" ERROR : atoi - errno %d.",
						errno);
					errflag++;
				}
			}
			break;
		case 'd':
			if (dflag)
				errflag++;
			else {
				dflag++;
				errno = 0;
				DVAL = atoi(optarg);
				if (errno) {
					perror("atoi");
					fprintf(stderr,
						" ERROR : atoi - errno %d.",
						errno);
					errflag++;
				}
			}
			break;
		case 'f':
			fflag = 1;
			break;
		case 'D':
			AUSDEBUG = 1;
			break;
		case 't':
			if (tflag)
				errflag++;
			else {
				tflag++;
				errno = 0;
				TVAL = atoi(optarg);
				if (!TVAL || errno) {
					perror("atoi");
					fprintf(stderr,
						" ERROR : atoi - errno %d.",
						errno);
					errflag++;
				}
			}
			break;
		case '?':
			errflag++;
			break;
		}
	}

	if (BVAL < 2) {
		errflag++;
		fprintf(stderr, "The value of b must be greater than 1\n");
	} else if (DVAL < 2) {
		errflag++;
		fprintf(stderr, "The depth value must be greater than 1\n");
	} else if (!fflag && (DVAL > MAXDVAL)) {
/* || BVAL > limits[DVAL])) { */
		fprintf(stderr, "\tExceeded process creation limits.   \
\n\tParameters will generate %lu processes.  \n\tThe preset limits are as \
follows:\n\t\tdepth\tbreadth\ttotal\n", sumit(BVAL, DVAL));
		for (i = 2; i <= MAXDVAL; i++)
			fprintf(stderr, "\t\t %-3d\t  %-5d\t%-5lu\n", i,
				limits[i], sumit(limits[i], i));
		exit(1);
	}

	if (errflag) {
		fprintf(stderr,
			"usage: %s [-b number] [-d number] [-t number] \n",
			argv[0]);
		fprintf(stderr, "where:\n");
		fprintf(stderr,
			"\t-b number\tnumber of children each parent will spawn ( > 1)\n");
		fprintf(stderr, "\t-d number\tdepth of process tree ( > 1)\n");
		fprintf(stderr, "\t-t\t\tset timeout value\n");
		fprintf(stderr, " SEVERE : Command line parameter error.\n");
		exit(1);
	}
}

/*
 * Initializes environment variables, using defaults if not set in env.
 */
int getenv_val(void)
{
	char *c;		/* character pointer */
	struct envstruct *envd = envdata;	/* pointer to environment data */

	union {
		int *vint;
		char *chptr;
	} val;

	/*
	 * Loop through envdata, set default first then set environment
	 * variable value if present.
	 */
	for (; *envd->env_name != '\0'; envd++) {
		if ((val.chptr = getenv(envd->env_name)) == NULL)
			val.chptr = envd->eval.chptr;

		c = val.chptr;
		while (isdigit(*c))
			c++;

		if (*c == '\0') {
			(envd->eval.vint) = malloc(sizeof(int));
			*(envd->eval.vint) = atoi(val.chptr);
		} else {
			envd->eval.chptr = malloc(strlen(val.chptr) + 1);
			strcpy(envd->eval.chptr, val.chptr);
		}
	}
	return 0;
}

/*
 * Prints all errors coming from the children and terminates execution if
 * an error execption is received.  In addition messenger() is sent the
 * process group id of the children so it can terminate all children.
 * This routine uses message queues to receive all communications.
 */
void messenger(void)
{				/* AKA Assassin */
	Msgbuf rcvbuf;

	int discrim = 0;
	int rc;			/* generic return code var */
	int sig = -1;		/* type of signal received */
	extern int msgerr;	/* message queue used to send error messages */
	extern int procgrp;	/* process group of children (used to kill them) */

	/*
	 *  Infinite loop used to receive error messages from children and
	 *  to terminate process tree.
	 */
	while (TRUE) {
		rc = msgrcv(msgerr, &rcvbuf, sizeof(struct messagebuf), 0, 0);
		if (rc == -1) {
			switch (errno) {
			case EAGAIN:
				printf("msgqueue %d not ready to receive\n",
				       msgid);
				fflush(stdout);
				errno = 0;
				break;
			case ENOMSG:
				printf("msgqueue %d no message\n", msgid);
				fflush(stdout);
				errno = 0;
				break;
			default:
				perror("msgrcv failed");
				fprintf(stderr,
					" SEVERE : messenger - msgrcv failed, errno: %d\n",
					errno);
				errno = 0;
				break;
			}
		} else {
			switch ((int)rcvbuf.mtyp) {
			case 1:	/* type 1: we received the process group id */
				sscanf(rcvbuf.mtext, "%d", &procgrp);
				break;

			case 2:	/*  type 2: we received an error */
				fprintf(stderr, " SEVERE : %s ", rcvbuf.mtext);
				/* rcvbuf.mtext type %s ou %d ??? */
				break;

			case 3:	/* type 3: somebody got a signal, now we terminate */
				sscanf(rcvbuf.mtext, "%d", &sig);

				switch (sig) {
				case SIGALRM:
					/* a process is hung, we will terminate */
					killpg(procgrp, sig);
					fprintf(errfp,
						"ALERT! ALERT! WE HAVE TIMED OUT\n");
					fprintf(stderr,
						" SEVERE : SIGALRM: A process timed out, we failed\n");
					shmaddr->err++;
					break;

				case SIGUSR1:
					/* Special: means everything went ok */
					discrim = 1;
					break;

				default:
					/* somebody sent a signal, we will terminate */
					killpg(procgrp, sig);
					fprintf(errfp,
						"We received signal %d\n", sig);
					fprintf(stderr,
						" SEVERE : signal %d received, A proc was killed\n",
						sig);
					break;
				}
				/* clean up and exit with status */
				rm_msgqueue();
				rm_semseg();
				if (AUSDEBUG)
					print_shm();
				prtln();
				rm_shmseg();
				prtln();
				if (discrim) {
					prtln();
					printf("Test exiting with SUCCESS\n");
					exit(0);
				}
				exit(1);

				break;
			}
		}
	}
}

/*
 *  This routine spawns off the first child (node 0) of the process tree.
 *  This child set up the signal handler for all of the children and also
 *  sets up a process group so that all children can be terminated easily.
 *  The child then calls spawn which creates the process tree.  After spawn
 *  has returned the child contacts the parent and the parent exits.
 *  The parent sets her own signal handler and then calls messenger.
 */
void doit(void)
{
	pid_t pid;		/* process id */
	int rc;
	char mtext[80];		/* message text */
	extern int msgerr;
	extern int procgrp;

	pid = fork();
#ifdef __64LDT__
	if (pid == (pid_t) 0) {
#else
	if (pid == 0) {
#endif
		/* set the process group so we can terminate all children */
		set_signals((void *)nextofkin);	/* set up signal handlers and initialize pgrp */
#ifndef _LINUX
		procgrp = setpgrp(0, 0);
#else
		procgrp = setpgrp();
#endif
		if (AUSDEBUG) {
			fprintf(stderr, "process group: %d\n", procgrp);
			fflush(stderr);
		}
		if (procgrp == -1) {
			perror("setpgid failed");
			fprintf(stderr, " SEVERE : setpgid failed, errno: %d\n",
				errno);
			exit(1);
		}
		sprintf(mtext, "%d", procgrp);
		rc = send_message(msgerr, 1, mtext);
		if (rc == -1) {
			perror("send_message failed");
			fprintf(stderr,
				" SEVERE : send_message failed, errno: %d\n",
				errno);
			exit(1);
		}

		put_proc_info(0);	/* store process info for this (root) process */
		spawn(0);
		if (shmaddr->pid == getpid()) {
			sprintf(mtext, "%d", SIGUSR1);
			rc = send_message(msgerr, 3, mtext);
			if (rc == -1) {
				severe
				    ("msgsnd failed: %d msgid %d mtyp %d mtext %d\n",
				     errno, msgerr, 3, mtext);
				exit(1);

			}
		}
		exit(0);
	}
#ifdef __64LDT__
	else if (pid > (pid_t) 0) {
#else
	else if (pid > 0) {
#endif
		set_signals((void *)cleanup);	/* set up signal handlers and initialize pgrp */
		messenger();	/* receives and acts upon messages */
		exit(1);
	} else {
		perror("fork failed");
		fprintf(stderr,
			" SEVERE : fork failed, exiting with errno %d\n",
			errno);
		exit(1);
	}
}

/* main */
int main(int argc, char *argv[])
{
	extern Pinfo *shmaddr;	/* start address of shared memory */

	prtln();
	getenv_val();		/* Get and initialize all environment variables */
	prtln();

	if (argc < 2) {
		fprintf(stderr,
			"usage: %s [-b number] [-d number] [-t number] \n",
			argv[0]);
		fprintf(stderr, "where:\n");
		fprintf(stderr,
			"\t-b number\tnumber of children each parent will spawn ( > 1)\n");
		fprintf(stderr, "\t-d number\tdepth of process tree ( > 1)\n");
		fprintf(stderr, "\t-t\t\tset timeout value\n");
		fprintf(stderr, " SEVERE : Command line parameter error.\n");
		exit(1);
	}

	parse_args(argc, argv);	/* Get all command line arguments */
	dprt("value of BVAL = %d, value of DVAL = %d\n", BVAL, DVAL);
	nodesum = sumit(BVAL, DVAL);
#ifdef _LINUX
	if (nodesum > 250) {
		printf("total number of process to be created "
		       "nodesum (%d) is greater\n than the allowed "
		       "SEMMSL value (250)\n", nodesum);
		printf("reseting the value of nodesum to SEMMSL\n");
		nodesum = 250;
	}
#endif

	dprt("value of nodesum is initiallized to: %d\n", nodesum);

	prtln();
	setup_shm();		/* Set up, allocate and initialize shared memory */
	prtln();
	setup_semaphores();	/* Set up, allocate and initialize semaphores */
	prtln();
	setup_msgqueue();	/* Set up, allocate and initialize message queues */
	prtln();

	doit();			/* spawn off processes */
	prtln();
	return 0;

}

/*
* Copyright (c) International Business Machines Corp., 2007
* This program is free software; you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation; either version 2 of the License, or
* (at your option) any later version.
* This program is distributed in the hope that it will be useful
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See
* the GNU General Public License for more details.
* You should have received a copy of the GNU General Public License
* along with this program; if not, write to the Free Software
* Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
*
***************************************************************************
*
* Assertion:
*   a) Create a  container.
*   b) Create many levels of child containers inside this container.
*   c) Now do kill -9 init , outside of the container.
*   d) This should kill all the child containers.
*      (containers created at the level below)
*
* Description:
* 1. Parent process clone a process with flag CLONE_NEWPID
* 2. The container will recursively loop and creates 4 more containers.
* 3. All the container init's  goes into sleep(), waiting to be terminated.
* 4. The parent process will kill child[3] by passing SIGKILL
* 5. Now parent process, verifies the child containers 4 & 5 are destroyed.
* 6. If they are killed then
*	Test PASSed
*  else Test FAILed.
*
* Test Name: pidns05
*
* History:
*
* FLAG DATE     	NAME           		        DESCRIPTION
* 31/10/08  Veerendra C <vechandr@in.ibm.com> 	Verifies killing of NestedCont's
*
*******************************************************************************/
#define _GNU_SOURCE 1
#include <sys/wait.h>
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <usctest.h>
#include <test.h>
#include <libclone.h>

#define INIT_PID	1
#define CINIT_PID	1
#define PARENT_PID	0
#define MAX_DEPTH	5

char *TCID = "pidns05";
int TST_TOTAL = 1;
int fd[2];

/*
 * cleanup() - performs all ONE TIME cleanup for this test at
 *             completion or premature exit.
 */
void cleanup()
{
	/* Clean the test testcase as LTP wants*/
	TEST_CLEANUP;

	/* exit with return code appropriate for results */
	tst_exit();
}
int max_pid()
{
	FILE *fp;
	int ret;

	fp = fopen("/proc/sys/kernel/pid_max", "r") ;
	if (fp != NULL) {
		fscanf(fp, "%d", &ret);
		fclose(fp);
	} else {
		tst_resm(TBROK, "Cannot open /proc/sys/kernel/pid_max \n");
		ret = -1;
	}
	return ret;
}

/* find_cinit_pids() iteratively finds the pid's having same PGID as its parent.
 * Input parameter - Accepts pointer to pid_t : To copy the pid's matching.
 * Returns - the number of pids matched.
*/
int find_cinit_pids(pid_t *pids)
{
	int next = 0, pid_max, i ;
	pid_t parentpid, pgid, pgid2;

	pid_max = max_pid();
	parentpid = getpid();
	pgid = getpgid(parentpid);

	/* The loop breaks, when the loop counter reaches the parentpid value */
	for (i = parentpid + 1; i != parentpid; i++) {
		if (i > pid_max)
			i = 2;

		pgid2 = getpgid(i);
		if (pgid2 == pgid) {
			pids[next] = i;
			next++;
		}
	}
	return next;
}

/*
* create_nested_container() Recursively create MAX_DEPTH nested containers
*/
int create_nested_container(void *vtest)
{
	int ret, count, *level ;
	pid_t cpid, ppid;
	cpid = getpid();
	ppid = getppid();
	char mesg[] = "Nested Containers are created";

	level = (int *)vtest;
	count = *level;

	/* Child process closes up read side of pipe */
	close(fd[0]);

	/* Comparing the values to make sure pidns is created correctly */
	if ((cpid != CINIT_PID) || (ppid != PARENT_PID)) {
		tst_resm(TFAIL, "FAIL: Got unexpected result of"
		" cpid=%d ppid=%d\n", cpid, ppid);
		cleanup();
	}
	if (count > 1) {
		count--;
		ret = do_clone_unshare_test(T_CLONE, CLONE_NEWPID,
				create_nested_container, (void *) &count);
		if (ret == -1) {
			tst_resm(TFAIL, "clone() Failed, errno = %d : %s\n" ,
				 ret, strerror(ret));
			cleanup();
		}
	} else {
		/* Sending mesg, 'Nested containers created' through the pipe */
		write(fd[1], mesg, (strlen(mesg)+1));
	}

	close(fd[1]);
	pause();

	/* NOT REACHED */
	return 0;
}


void kill_nested_containers()
{
	int orig_count, new_count, status = 0, i;
	pid_t pids[MAX_DEPTH];
	pid_t pids_new[MAX_DEPTH];

	orig_count = find_cinit_pids(pids);
	kill(pids[MAX_DEPTH - 3], SIGKILL) ;
	sleep(1);

	/* After killing child container, getting the New PID list */
	new_count = find_cinit_pids(pids_new);

	/*Verifyng if the child containers are destroyed when parent is killed*/
	if (orig_count - 2 != new_count)
		status = -1;

	for (i = 0; i < new_count; i++) {
		if (pids[i] != pids_new[i])
			status = -1;
	}

	if (status == 0)
		tst_resm(TPASS, "The number of containers killed are %d\n" ,
				orig_count - new_count);
	else
		tst_resm(TFAIL, "Failed to kill the sub-containers of "
				"the container %d\n", pids[MAX_DEPTH - 3]);

	/* Loops through the containers created,  to exit from sleep() */
	for (i = 0; i < MAX_DEPTH; i++) {
		kill(pids[i], SIGKILL);
		waitpid(pids[i], &status, 0);
	}
}


/***********************************************************************
*   M A I N
***********************************************************************/

int main(int argc, char *argv[])
{
	int ret, nbytes, status;
	char readbuffer[80];
	pid_t pid, pgid;
	int count = MAX_DEPTH;

	pid = fork();
	if (pid < 0) {
		perror("fork()");
		exit(1);
	} else if (pid) {
		wait(&status);
		exit(0);
	}

	/* To make all the containers share the same PGID as its parent */
	setpgid(0, 0);

	pid = getpid();
	pgid = getpgid(pid);
	ret = pipe(fd);
	if (ret == -1)
		tst_brkm(TBROK, cleanup, "pipe() failed, errno %d", errno);

	ret = do_clone_unshare_test(T_CLONE, CLONE_NEWPID,
				create_nested_container, (void *) &count);
	if (ret == -1) {
		tst_resm(TFAIL, "clone() Failed, errno = %d : %s\n" ,
			 ret, strerror(ret));
		cleanup();
	}

	close(fd[1]);
	/* Waiting for the MAX_DEPTH number of containers to be created */
	nbytes = read(fd[0], readbuffer, sizeof(readbuffer));
	close(fd[0]);
	if (nbytes > 0)
		tst_resm(TINFO, " %d %s", MAX_DEPTH, readbuffer);
	else {
		tst_resm(TFAIL, "Unable to create %d containers\n", MAX_DEPTH);
		cleanup();
	}

	/* Kill the container created  */
	kill_nested_containers();
	/* cleanup and exit */
	cleanup();

	/*NOTREACHED*/
	return 0;
}


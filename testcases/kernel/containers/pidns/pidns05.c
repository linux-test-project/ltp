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
* Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
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
*	Test passed
*  else Test failed.
*
* Test Name: pidns05
*
* History:
*
* FLAG DATE		NAME				DESCRIPTION
* 31/10/08  Veerendra C <vechandr@in.ibm.com>	Verifies killing of NestedCont's
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
#include "pidns_helper.h"
#include "test.h"
#include "safe_macros.h"

#define INIT_PID	1
#define CINIT_PID	1
#define PARENT_PID	0
#define MAX_DEPTH	5

char *TCID = "pidns05";
int TST_TOTAL = 1;
int fd[2];

int max_pid(void)
{
	FILE *fp;
	int ret;

	fp = fopen("/proc/sys/kernel/pid_max", "r");
	if (fp != NULL) {
		fscanf(fp, "%d", &ret);
		fclose(fp);
	} else {
		tst_resm(TBROK, "Cannot open /proc/sys/kernel/pid_max");
		ret = -1;
	}
	return ret;
}

/* find_cinit_pids() iteratively finds the pid's having same PGID as its parent.
 * Input parameter - Accepts pointer to pid_t : To copy the pid's matching.
 * Returns - the number of pids matched.
*/
int find_cinit_pids(pid_t * pids)
{
	int next = 0, pid_max, i;
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
	int exit_val;
	int ret, count, *level;
	pid_t cpid, ppid;
	cpid = getpid();
	ppid = getppid();
	char mesg[] = "Nested Containers are created";

	level = (int *)vtest;
	count = *level;

	/* Child process closes up read side of pipe */
	close(fd[0]);

	/* Comparing the values to make sure pidns is created correctly */
	if (cpid != CINIT_PID || ppid != PARENT_PID) {
		printf("Got unexpected cpid and/or ppid (cpid=%d ppid=%d)\n",
		       cpid, ppid);
		exit_val = 1;
	}
	if (count > 1) {
		count--;
		ret = do_clone_unshare_test(T_CLONE, CLONE_NEWPID,
					    create_nested_container,
					    (void *)&count);
		if (ret == -1) {
			printf("clone failed; errno = %d : %s\n",
			       ret, strerror(ret));
			exit_val = 1;
		} else
			exit_val = 0;
	} else {
		/* Sending mesg, 'Nested containers created' through the pipe */
		write(fd[1], mesg, (strlen(mesg) + 1));
		exit_val = 0;
	}

	close(fd[1]);
	pause();

	return exit_val;
}

void kill_nested_containers()
{
	int orig_count, new_count, status = 0, i;
	pid_t pids[MAX_DEPTH];
	pid_t pids_new[MAX_DEPTH];

	orig_count = find_cinit_pids(pids);
	kill(pids[MAX_DEPTH - 3], SIGKILL);
	sleep(1);

	/* After killing child container, getting the New PID list */
	new_count = find_cinit_pids(pids_new);

	/* Verifying that the child containers were destroyed when parent is killed */
	if (orig_count - 2 != new_count)
		status = -1;

	for (i = 0; i < new_count; i++) {
		if (pids[i] != pids_new[i])
			status = -1;
	}

	if (status == 0)
		tst_resm(TPASS, "The number of containers killed are %d",
			 orig_count - new_count);
	else
		tst_resm(TFAIL, "Failed to kill the sub-containers of "
			 "the container %d", pids[MAX_DEPTH - 3]);

	/* Loops through the containers created to exit from sleep() */
	for (i = 0; i < MAX_DEPTH; i++) {
		kill(pids[i], SIGKILL);
		waitpid(pids[i], &status, 0);
	}
}

static void setup(void)
{
	tst_require_root();
	check_newpid();
}

int main(int argc, char *argv[])
{
	int ret, nbytes, status;
	char readbuffer[80];
	pid_t pid, pgid;
	int count = MAX_DEPTH;

	setup();

	/*
	 * XXX (garrcoop): why in the hell is this fork-wait written this way?
	 * This doesn't add up with the pattern used for the rest of the tests,
	 * so I'm pretty damn sure this test is written incorrectly.
	 */
	pid = fork();
	if (pid == -1) {
		tst_brkm(TBROK | TERRNO, NULL, "fork failed");
	} else if (pid != 0) {
		/*
		 * NOTE: use waitpid so that we know we're waiting for the
		 * _top-level_ child instead of a spawned subcontainer.
		 *
		 * XXX (garrcoop): Might want to mask SIGCHLD in the top-level
		 * child too, or not *shrugs*.
		 */
		if (waitpid(pid, &status, 0) == -1) {
			perror("wait failed");
		}
		if (WIFEXITED(status))
			exit(WEXITSTATUS(status));
		else
			exit(status);
	}

	/* To make all the containers share the same PGID as its parent */
	setpgid(0, 0);

	pid = getpid();
	pgid = getpgid(pid);
	SAFE_PIPE(NULL, fd);

	TEST(do_clone_unshare_test(T_CLONE, CLONE_NEWPID,
				   create_nested_container, (void *)&count));
	if (TEST_RETURN == -1) {
		tst_brkm(TFAIL | TTERRNO, NULL, "clone failed");
	}

	close(fd[1]);
	/* Waiting for the MAX_DEPTH number of containers to be created */
	nbytes = read(fd[0], readbuffer, sizeof(readbuffer));
	close(fd[0]);
	if (nbytes > 0)
		tst_resm(TINFO, " %d %s", MAX_DEPTH, readbuffer);
	else
		tst_brkm(TFAIL, NULL, "unable to create %d containers",
			 MAX_DEPTH);

	/* Kill the container created */
	kill_nested_containers();

	tst_exit();
}

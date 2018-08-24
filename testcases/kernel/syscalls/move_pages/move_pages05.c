/*
 *   Copyright (c) 2008 Vijay Kumar B. <vijaykumar@bravegnu.org>
 *
 *   Based on testcases/kernel/syscalls/waitpid/waitpid01.c
 *   Original copyright message:
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

/*
 * NAME
 *	move_pages05.c
 *
 * DESCRIPTION
 *      Test movement of pages mapped by a process.
 *
 * ALGORITHM
 *      1. Start the test case program as root.
 *      2. Allocate a shared memory in NUMA node A.
 *      3. Fork another process.
 *      4. Use move_pages() to move the pages to NUMA node B, without
 *         the MPOL_MF_MOVE_ALL.
 *      5. Check if the corresponding status is set to -EACCES.
 *
 * USAGE:  <for command-line>
 *      move_pages05 [-c n] [-i n] [-I x] [-P x] [-t]
 *      where,  -c n : Run n copies concurrently.
 *              -i n : Execute test n times.
 *              -I x : Execute test for x seconds.
 *              -P x : Pause for x seconds between iterations.
 *              -t   : Turn on syscall timing.
 *
 * History
 *	05/2008 Vijay Kumar
 *		Initial Version.
 *
 * Restrictions
 *	None
 */

#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <errno.h>
#include "test.h"
#include "move_pages_support.h"

#define SHARED_PAGE 0
#define N_SHARED_PAGES 1
#define UNSHARED_PAGE 1
#define N_UNSHARED_PAGES 1
#define N_TEST_PAGES (N_SHARED_PAGES + N_UNSHARED_PAGES)
#define N_TEST_NODES 2

enum {
	SEM_CHILD_SETUP,
	SEM_PARENT_TEST,

	MAX_SEMS
};

void setup(void);
void cleanup(void);

char *TCID = "move_pages05";
int TST_TOTAL = 1;

/*
 * child() - touches pages, and waits for signal from parent.
 * @pages: shared pages allocated in parent
 */
void child(void **pages, sem_t * sem)
{
	int i;

	for (i = 0; i < N_TEST_PAGES; i++) {
		char *page;

		page = pages[i];
		page[0] = 0xAA;
	}

	/* Setup complete. Ask parent to continue. */
	if (sem_post(&sem[SEM_CHILD_SETUP]) == -1)
		tst_resm(TWARN | TERRNO, "error post semaphore");

	/* Wait for testcase in parent to complete. */
	if (sem_wait(&sem[SEM_PARENT_TEST]) == -1)
		tst_resm(TWARN | TERRNO, "error waiting for semaphore");

	exit(0);
}

int main(int argc, char **argv)
{

	tst_parse_opts(argc, argv, NULL, NULL);

	setup();

#ifdef HAVE_NUMA_V2
	unsigned int i;
	int lc;
	unsigned int from_node;
	unsigned int to_node;
	int ret;

	ret = get_allowed_nodes(NH_MEMS, 2, &from_node, &to_node);
	if (ret < 0)
		tst_brkm(TBROK | TERRNO, cleanup, "get_allowed_nodes: %d", ret);

	/* check for looping state if -i option is given */
	for (lc = 0; TEST_LOOPING(lc); lc++) {
		void *pages[N_TEST_PAGES] = { 0 };
		int nodes[N_TEST_PAGES];
		int status[N_TEST_PAGES];
		pid_t cpid;
		sem_t *sem;

		/* reset tst_count in case we are looping */
		tst_count = 0;

		ret = alloc_shared_pages_on_node(pages + SHARED_PAGE,
						 N_SHARED_PAGES, from_node);
		if (ret == -1)
			continue;

		ret = alloc_pages_on_node(pages + UNSHARED_PAGE,
					  N_UNSHARED_PAGES, from_node);
		if (ret == -1)
			goto err_free_shared;

		for (i = 0; i < N_TEST_PAGES; i++) {
			nodes[i] = to_node;
		}

		sem = alloc_sem(MAX_SEMS);
		if (sem == NULL) {
			goto err_free_unshared;
		}

		/*
		 * Fork a child process so that the shared pages are
		 * now really shared between two processes.
		 */
		cpid = fork();
		if (cpid == -1) {
			tst_resm(TBROK, "forking child failed");
			goto err_free_sem;
		} else if (cpid == 0) {
			child(pages, sem);
		}

		/* Wait for child to setup and signal. */
		if (sem_wait(&sem[SEM_CHILD_SETUP]) == -1)
			tst_resm(TWARN | TERRNO, "error wait semaphore");

		ret = numa_move_pages(0, N_TEST_PAGES, pages, nodes,
				      status, MPOL_MF_MOVE);
		if (ret == -1) {
			tst_resm(TFAIL | TERRNO,
				 "move_pages unexpectedly failed");
			goto err_kill_child;
		} else if (ret > 0) {
			tst_resm(TINFO, "move_pages() returned %d\n", ret);
		}

		if (status[SHARED_PAGE] == -EACCES)
			tst_resm(TPASS, "status[%d] set to expected -EACCES",
				 SHARED_PAGE);
		else
			tst_resm(TFAIL, "status[%d] is %d",
				 SHARED_PAGE, status[SHARED_PAGE]);

err_kill_child:
		/* Test done. Ask child to terminate. */
		if (sem_post(&sem[SEM_PARENT_TEST]) == -1)
			tst_resm(TWARN | TERRNO, "error post semaphore");
		/* Read the status, no zombies! */
		wait(NULL);
err_free_sem:
		free_sem(sem, MAX_SEMS);
err_free_unshared:
		free_pages(pages + UNSHARED_PAGE, N_UNSHARED_PAGES);
err_free_shared:
		free_shared_pages(pages + SHARED_PAGE, N_SHARED_PAGES);
	}
#else
	tst_resm(TCONF, NUMA_ERROR_MSG);
#endif

	cleanup();

	tst_exit();
}

/*
 * setup() - performs all ONE TIME setup for this test
 */
void setup(void)
{

	tst_sig(FORK, DEF_HANDLER, cleanup);

	check_config(N_TEST_NODES);

	/* Pause if that option was specified
	 * TEST_PAUSE contains the code to fork the test with the -c option.
	 */
	TEST_PAUSE;
}

/*
 * cleanup() - performs all ONE TIME cleanup for this test at completion
 */
void cleanup(void)
{

}

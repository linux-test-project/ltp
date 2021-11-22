/*
 * Copyright (c) 2003, Intel Corporation. All rights reserved.
 * Created by:  julie.n.fleischer REMOVE-THIS AT intel DOT com
 * This file is licensed under the GPL license.  For the full content
 * of this license, see the COPYING file at the top level of this
 * source tree.
 */

/*
 * Test that tests to check if O_CREAT and O_EXCL are set that no other
 * message queue exists are atomic.
 *
 * Test case will just attempt to call mq_open() with O_CREAT and O_EXCL
 * using the same name in two different processes.  If one process fails,
 * the test is considered a pass.
 *
 * This is a best attempt to test that these are atomic.  It does make the
 * assumption (which could generally be untrue) that both mq_open() calls
 * will attempt to be made at the same time.  For the sake of this test case,
 * this is fine (will have some false positives, but no false negatives).
 */

#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <errno.h>
#include <fcntl.h>
#include <mqueue.h>
#include <signal.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include "posixtest.h"
#include "tempfile.h"

#define TNAME "mq_open/16-1.c"

int main(void)
{
	char qname[NAME_MAX];
	char fname[PATH_MAX];
	int pid, succeeded = 0;
	int fd;
	void *pa = NULL;
	mqd_t childqueue, queue;

	/*
	 * initialize both queues
	 */
	childqueue = (mqd_t) - 1;
	queue = (mqd_t) - 1;

	sprintf(qname, "/mq_open_16-1_%d", getpid());

	PTS_GET_TMP_FILENAME(fname, "pts_mq_open_16_1");
	unlink(fname);
	fd = open(fname, O_CREAT | O_RDWR | O_EXCL, S_IRUSR | S_IWUSR);
	if (fd == -1) {
		printf(TNAME " Error at open(): %s\n", strerror(errno));
		return PTS_UNRESOLVED;
	}
	unlink(fname);

	if (ftruncate(fd, sizeof(int))) {
		perror("ftruncate");
		close(fd);
		return PTS_UNRESOLVED;
	}

	pa = mmap(NULL, sizeof(int), PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
	if (pa == MAP_FAILED) {
		printf(TNAME " Error at mmap: %s\n", strerror(errno));
		close(fd);
		return PTS_FAIL;
	}
	*(int *)pa = 0;

	pid = fork();
	if (pid == 0) {
		sigset_t mask;
		int sig;

		/* child here */

		/* try to sync with parent for mq_open */
		sigemptyset(&mask);
		sigaddset(&mask, SIGUSR1);
		sigprocmask(SIG_BLOCK, &mask, NULL);
		sigwait(&mask, &sig);

		childqueue = mq_open(qname, O_CREAT | O_EXCL | O_RDWR,
				     S_IRUSR | S_IWUSR, NULL);
		if (childqueue != (mqd_t) - 1) {
			++*(int *)pa;
#ifdef DEBUG
			printf("mq_open() in child succeeded\n");
		} else {
			printf("mq_open() in child failed\n");
#endif
		}
	} else {
		/* parent here */
		int i;

		sleep(1);
		kill(pid, SIGUSR1);

		queue = mq_open(qname, O_CREAT | O_EXCL | O_RDWR,
				S_IRUSR | S_IWUSR, NULL);
		if (queue != (mqd_t) - 1) {
			++*(int *)pa;
#ifdef DEBUG
			printf("mq_open() in parent succeeded\n");
		} else {
			printf("mq_open() in parent failed\n");
#endif
		}

		if (wait(&i) == -1) {
			perror("Error waiting for child to exit");
			printf("Test UNRESOLVED\n");
			mq_close(queue);
			mq_close(childqueue);
			mq_unlink(qname);
			close(fd);
			munmap(pa, sizeof(int));
			return PTS_UNRESOLVED;
		}

		mq_close(queue);
		mq_close(childqueue);
		mq_unlink(qname);

		succeeded = *(int *)pa;
		close(fd);
		munmap(pa, sizeof(int));

		if (succeeded == 0) {
			printf("Test FAILED - mq_open() never succeeded\n");
			return PTS_FAIL;
		}

		if (succeeded > 1) {
			printf("Test FAILED - mq_open() succeeded twice\n");
			return PTS_FAIL;
		}

		printf("Test PASSED\n");
		return PTS_PASS;
	}

	return PTS_UNRESOLVED;
}

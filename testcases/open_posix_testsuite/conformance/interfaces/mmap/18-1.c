/*
 * Copyright (c) 2002, Intel Corporation. All rights reserved.
 * Copyright (c) 2012, Cyril Hrubis <chrubis@suse.cz>
 *
 * This file is licensed under the GPL license.  For the full content
 * of this license, see the COPYING file at the top level of this
 * source tree.
 *
 * The mmap() function shall fail if:
 * ML [EAGAIN] The mapping could not be locked in memory,
 * if required by mlockall(), due to a lack of resources.
 *
 * Test Steps:
 * 1. Call mlockall(), setting MCL_FUTURE;
 * 2. Call setrlimit(), set rlim_cur of resource RLIMIT_MEMLOCK to a
 *    certain value.
 * 3. Change user to non-root user seteuid()
 * 4. Map a shared memory object, with size larger than the
 *    rlim_cur value set in step 2
 * 5. Should get EAGAIN.
 * 6. Change user to root seteuid()
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/resource.h>
#include <fcntl.h>
#include <pwd.h>
#include <string.h>
#include <errno.h>
#include "posixtest.h"

/* Set the euid of this process to a non-root uid */
static int set_nonroot(void)
{
	struct passwd *pw;
	setpwent();
	/* search for the first user which is non root */
	while ((pw = getpwent()) != NULL)
		if (strcmp(pw->pw_name, "root"))
			break;
	endpwent();
	if (pw == NULL) {
		printf("There is no other user than current and root.\n");
		return 1;
	}

	if (seteuid(pw->pw_uid) != 0) {
		if (errno == EPERM) {
			printf
			    ("You don't have permission to change your UID.\n");
			return 1;
		}
		perror("An error occurs when calling seteuid()");
		return 1;
	}

	printf("Testing with user '%s' (uid: %d)\n",
	       pw->pw_name, (int)geteuid());
	return 0;
}

int main(void)
{
	char tmpfname[256];

	/* size of shared memory object */
	size_t shm_size;

	void *pa;
	size_t len;
	int fd;

	size_t memlock_size;
	struct rlimit rlim = {.rlim_max = RLIM_INFINITY };

	/* Lock all memory page to be mapped */
	if (mlockall(MCL_FUTURE) == -1) {
		printf("Error at mlockall(): %s\n", strerror(errno));
		return PTS_UNRESOLVED;
	}

	/* Set rlim.rlim_cur < len */
	len = 1024 * 1024;
	memlock_size = len / 2;
	rlim.rlim_cur = memlock_size;

	/* We don't care of the size of the actual shared memory object */
	shm_size = 1024;

	if (setrlimit(RLIMIT_MEMLOCK, &rlim) == -1) {
		printf("Error at setrlimit(): %s\n", strerror(errno));
		return PTS_UNRESOLVED;
	}

	snprintf(tmpfname, sizeof(tmpfname), "pts_mmap_18_1_%d", getpid());

	/* Create shared object */
	shm_unlink(tmpfname);
	fd = shm_open(tmpfname, O_RDWR | O_CREAT | O_EXCL, S_IRUSR | S_IWUSR);
	if (fd == -1) {
		printf("Error at shm_open(): %s\n", strerror(errno));
		return PTS_UNRESOLVED;
	}
	shm_unlink(tmpfname);
	if (ftruncate(fd, shm_size) == -1) {
		printf("Error at ftruncate(): %s\n", strerror(errno));
		return PTS_UNRESOLVED;
	}

	/* This test should be run under standard user permissions */
	if (getuid() == 0) {
		if (set_nonroot() != 0) {
			printf("Cannot run this test as non-root user\n");
			return PTS_UNTESTED;
		}
	}

	/*
	 * EAGAIN:
	 * Lock all the memory by mlockall().
	 * Set resource limit setrlimit()
	 * Change the user to non-root then only setrlimit is applicable.
	 */
	pa = mmap(NULL, len, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
	if (pa == MAP_FAILED && errno == EAGAIN) {
		printf("Got EAGAIN: %s\n", strerror(errno));
		printf("Test PASSED\n");
		/* Change user to root */
		if (seteuid(0)) {
			close(fd);
			perror("seteuid");
			return PTS_UNRESOLVED;
		}
		close(fd);
		munmap(pa, len);
		return PTS_PASS;
	}

	if (pa == MAP_FAILED)
		perror("Error at mmap()");
	close(fd);
	munmap(pa, len);
	printf("Test FAILED: Did not get EAGAIN as expected\n");
	return PTS_FAIL;
}

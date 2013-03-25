/*
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 2.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 * Test that the permission bits of the shared memory object are set to the
 * value of the mode argument except those set in the file mode creation mask
 * of the process when the shared memory object does not exists and the O_CREAT
 * flags is set.
 *
 * Steps:
 *  1. Set umask to UMASK_FLAGS.
 *  2. Ensure that the shared memory object does not exist.
 *  3. Call shm_open with mode MOD_FLAGS.
 *  4. Get the stat of the opened file.
 *  5. Check that the permissions flags are right.
 */

#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <errno.h>
#include <unistd.h>
#include "posixtest.h"

#define SHM_NAME "posixtest_18-1"

/* execution bits is undefined */
#define MOD_FLAGS     (S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH)	/* -w?rw?r-? */
#define UMASK_FLAGS   (S_IRGRP | S_IWOTH)	/* --?r-?-w? */
#define ALL_MOD_FLAGS (S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | \
		       S_IROTH | S_IWOTH)	/* rw?rw?rw? */

int main(void)
{
	int fd, result;
	struct stat stat_buf;

	umask(UMASK_FLAGS);

	result = shm_unlink(SHM_NAME);
	if (result != 0 && errno != ENOENT) {
		/* The shared memory object exist and shm_unlink can not
		   remove it. */
		perror("An error occurs when calling shm_unlink()");
		return PTS_UNRESOLVED;
	}

	fd = shm_open(SHM_NAME, O_RDONLY | O_CREAT, MOD_FLAGS);
	if (fd == -1) {
		perror("An error occurs when calling shm_open()");
		return PTS_UNRESOLVED;
	}

	result = fstat(fd, &stat_buf);
	if (result != 0) {
		perror("An error occurs when calling fstat()");
		shm_unlink(SHM_NAME);
		return PTS_UNRESOLVED;
	}

	shm_unlink(SHM_NAME);

	/* Check the permissions flags.
	 * Permissions flags are right when the permissions bits  of
	 * stat_buf.st_mode are set if and only if the same bits of MOD_FLAGS
	 * are set and thoses of UMASK_FLAGS are not, i.e.:
	 * ALL_MOD_FLAGS & (stat_buf.st_mode ^ (MOD_FLAGS & ~UMASK_FLAGS)) == 0
	 */
	if (!(ALL_MOD_FLAGS & (stat_buf.st_mode ^ (MOD_FLAGS & ~UMASK_FLAGS)))) {
		printf("Test PASSED\n");
		return PTS_PASS;
	}

	printf("shm_open() does not set the rights permissions.\n");
	return PTS_FAIL;

}

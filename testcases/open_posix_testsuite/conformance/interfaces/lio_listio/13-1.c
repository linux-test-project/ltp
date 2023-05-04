/*
 * Copyright (c) 2004, Bull SA. All rights reserved.
 * Created by:  Laurent.Vivier@bull.net
 * This file is licensed under the GPL license.  For the full content
 * of this license, see the COPYING file at the top level of this
 * source tree.
 */

/*
 * assertion:
 *
 *	if mode is LIO_WAIT, lio_listio() shall return the value -1 and set
 *	errno to indicate error if the operation is not successfully queued.
 *
 * method:
 *
 *	- open a file for writing
 *	- submit a list with invalid opcodes to lio_listio in LIO_WAIT mode
 *	- check that lio_listio returns -1
 *
 */

#include <sys/stat.h>
#include <aio.h>
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "posixtest.h"
#include "tempfile.h"

#define TNAME "lio_listio/13-1.c"

#define NUM_AIOCBS	10
#define BUF_SIZE	1024

int main(void)
{
	char tmpfname[PATH_MAX];
	int fd;

	struct aiocb *aiocbs[NUM_AIOCBS];
	char *bufs;
	int errors = 0;
	int ret;
	int err;
	int i;

	if (sysconf(_SC_ASYNCHRONOUS_IO) < 200112L)
		exit(PTS_UNSUPPORTED);

	PTS_GET_TMP_FILENAME(tmpfname, "pts_lio_listio_13_1");
	unlink(tmpfname);

	fd = open(tmpfname, O_CREAT | O_RDWR | O_EXCL, S_IRUSR | S_IWUSR);

	if (fd == -1) {
		printf(TNAME " Error at open(): %s\n", strerror(errno));
		exit(PTS_UNRESOLVED);
	}

	unlink(tmpfname);

	bufs = malloc(NUM_AIOCBS * BUF_SIZE);

	if (bufs == NULL) {
		printf(TNAME " Error at malloc(): %s\n", strerror(errno));
		close(fd);
		exit(PTS_UNRESOLVED);
	}

	/* Queue up a bunch of aio writes */
	for (i = 0; i < NUM_AIOCBS; i++) {

		aiocbs[i] = malloc(sizeof(struct aiocb));
		memset(aiocbs[i], 0, sizeof(struct aiocb));

		aiocbs[i]->aio_fildes = fd;
		aiocbs[i]->aio_offset = 0;
		aiocbs[i]->aio_buf = &bufs[i * BUF_SIZE];
		aiocbs[i]->aio_nbytes = BUF_SIZE;

		if (i == 2)
			aiocbs[i]->aio_lio_opcode = -1;
		else
			aiocbs[i]->aio_lio_opcode = LIO_WRITE;
	}

	/* Submit request list */
	ret = lio_listio(LIO_WAIT, aiocbs, NUM_AIOCBS, NULL);

	if (ret == 0) {
		printf(TNAME " Error lio_listio() should have returned -1\n");

		for (i = 0; i < NUM_AIOCBS; i++)
			free(aiocbs[i]);
		free(bufs);
		close(fd);
		exit(PTS_FAIL);
	}

	if (errno != EIO) {
		printf(TNAME " lio_listio() sould set errno to EIO %d\n",
		       errno);

		for (i = 0; i < NUM_AIOCBS; i++)
			free(aiocbs[i]);

		close(fd);
		exit(PTS_FAIL);
	}

	/* Check return code and free things */
	for (i = 0; i < NUM_AIOCBS; i++) {
		if (i == 2)
			continue;

		err = aio_error(aiocbs[i]);
		ret = aio_return(aiocbs[i]);

		if ((err != 0) && (ret != BUF_SIZE)) {
			printf(TNAME " req %d: error = %d - return = %d\n", i,
			       err, ret);
			errors++;
		}

		free(aiocbs[i]);
	}

	free(bufs);

	close(fd);

	if (errors != 0)
		exit(PTS_FAIL);

	printf(TNAME " PASSED\n");

	return PTS_PASS;
}

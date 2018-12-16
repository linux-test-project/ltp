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
 *	supported operation for aio_lio_opcode shall be one of LIO_READ,
 *	LIO_WRITE or LIO_NOP.
 *
 * method:
 *
 *	- Open a file for writing
 *	- submit a list using valid opcodes to lio_listio
 *	- Check that no error occurs
 *	- Submit a list with invalid opcodes to lio_listio
 *	- Check that lio_listio returns an error
 *
 */

#include <sys/stat.h>
#include <sys/types.h>
#include <aio.h>
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "posixtest.h"

#define NUM_AIOCBS	3
#define BUF_SIZE	1024

#define TNAME "lio_listio/5-1.c"

int main(void)
{
	char tmpfname[256];
	int fd;

	struct aiocb *aiocbs[NUM_AIOCBS];
	char buf[BUF_SIZE];
	int errors = 0;
	int ret;
	int err;
	int i;

	if (sysconf(_SC_ASYNCHRONOUS_IO) < 200112L)
		exit(PTS_UNSUPPORTED);

	snprintf(tmpfname, sizeof(tmpfname), "/tmp/pts_lio_listio_4_1_%d",
		 getpid());
	unlink(tmpfname);

	fd = open(tmpfname, O_CREAT | O_RDWR | O_EXCL, S_IRUSR | S_IWUSR);

	if (fd == -1) {
		printf(TNAME " Error at open(): %s\n", strerror(errno));
		exit(PTS_UNRESOLVED);
	}

	unlink(tmpfname);

	/* Queue valid lio_opcode requests */
	for (i = 0; i < NUM_AIOCBS; i++) {

		aiocbs[i] = malloc(sizeof(struct aiocb));
		memset(aiocbs[i], 0, sizeof(struct aiocb));

		aiocbs[i]->aio_fildes = fd;
		aiocbs[i]->aio_offset = 0;
		aiocbs[i]->aio_buf = buf;
		aiocbs[i]->aio_nbytes = BUF_SIZE;

		if (i == 0)
			aiocbs[i]->aio_lio_opcode = LIO_WRITE;
		else if (i == 1)
			aiocbs[i]->aio_lio_opcode = LIO_READ;
		else
			aiocbs[i]->aio_lio_opcode = LIO_NOP;
	}

	/* Submit request list */
	ret = lio_listio(LIO_WAIT, aiocbs, NUM_AIOCBS, NULL);

	if (ret) {
		printf(TNAME " lio_listio() does not accept valid opcodes\n");
		for (i = 0; i < NUM_AIOCBS; i++)
			free(aiocbs[i]);
		close(fd);
		exit(PTS_FAIL);
	}

	/* Check return code and free things */
	for (i = 0; i < NUM_AIOCBS - 1; i++) {
		err = aio_error(aiocbs[i]);
		ret = aio_return(aiocbs[i]);

		if ((err != 0) && (ret != BUF_SIZE)) {
			printf(TNAME " req %d: error = %d - return = %d\n", i,
			       err, ret);
			errors++;
		}
	}

	if (errors) {
		for (i = 0; i < NUM_AIOCBS; i++)
			free(aiocbs[i]);

		close(fd);
		exit(PTS_FAIL);
	}

	/* Queue invalid lio_opcode requests */
	aiocbs[0]->aio_lio_opcode = -1;

	if (lio_listio(LIO_WAIT, aiocbs, 1, NULL) != -1) {
		printf(TNAME " lio_listio() accepts invalid opcode\n");

		for (i = 0; i < NUM_AIOCBS; i++)
			free(aiocbs[i]);

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

	for (i = 0; i < NUM_AIOCBS; i++)
		free(aiocbs[i]);

	close(fd);

	printf("Test PASSED\n");
	return PTS_PASS;
}

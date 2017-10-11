/*************************************************************************************
*
*  Copyright (c) International Business Machines  Corp., 2003
*
*  This program is free software;  you can redistribute it and/or modify
*  it under the terms of the GNU General Public License as published by
*  the Free Software Foundation; either version 2 of the License, or
*  (at your option) any later version.
*
*  This program is distributed in the hope that it will be useful,
*  but WITHOUT ANY WARRANTY;  without even the implied warranty of
*  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See
*  the GNU General Public License for more details.
*
*  You should have received a copy of the GNU General Public License
*  along with this program;  if not, write to the Free Software
*  Foundation,
*
*  FILE        : aio_tio
*  USAGE       : ./aio_tio
*
*  DESCRIPTION : This program will test Asynchronous I/O for 2.5 Kernel infrastructure
*  REQUIREMENTS:
*                1) libaio-0.3.92 or up for 2.5 kernal
*                2) glibc 2.1.91 or up
*  HISTORY     :
*      11/03/2003 Kai Zhao (ltcd3@cn.ibm.com)
*
*  CODE COVERAGE:
*                 68.3% - fs/aio.c
*
************************************************************************************/

#include "config.h"
#include "common.h"
#include "test.h"
#include <string.h>
#include <errno.h>

#ifdef HAVE_LIBAIO

#define AIO_MAXIO 32
#define AIO_BLKSIZE (64*1024)

static int alignment = 512;
static int wait_count = 0;

/*
 * write work done
 */
static void work_done(io_context_t ctx, struct iocb *iocb, long res, long res2)
{

	if (res2 != 0) {
		io_error("aio write", res2);
	}

	if (res != iocb->u.c.nbytes) {
		fprintf(stderr, "write missed bytes expect %lu got %ld\n",
			iocb->u.c.nbytes, res2);
		exit(1);
	}
	wait_count--;
}

/*
 * io_wait_run() - wait for an io_event and then call the callback.
 */
int io_wait_run(io_context_t ctx, struct timespec *to)
{
	struct io_event events[AIO_MAXIO];
	struct io_event *ep;
	int ret, n;

	/*
	 * get up to aio_maxio events at a time.
	 */
	ret = n = io_getevents(ctx, 1, AIO_MAXIO, events, to);

	/*
	 * Call the callback functions for each event.
	 */
	for (ep = events; n-- > 0; ep++) {
		io_callback_t cb = (io_callback_t) ep->data;
		struct iocb *iocb = ep->obj;
		cb(ctx, iocb, ep->res, ep->res2);
	}
	return ret;
}

int io_tio(char *pathname, int flag, int n, int operation)
{
	int res, fd = 0, i = 0;
	void *bufptr = NULL;
	off_t offset = 0;
	struct timespec timeout;

	io_context_t myctx;
	struct iocb iocb_array[AIO_MAXIO];
	struct iocb *iocbps[AIO_MAXIO];

	fd = open(pathname, flag, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
	if (fd <= 0) {
		printf("open for %s failed: %s\n", pathname, strerror(errno));
		return -1;
	}

	res = io_queue_init(n, &myctx);
	//printf (" res = %d \n", res);

	for (i = 0; i < AIO_MAXIO; i++) {

		switch (operation) {
		case IO_CMD_PWRITE:
			if (posix_memalign(&bufptr, alignment, AIO_BLKSIZE)) {
				perror(" posix_memalign failed ");
				return -1;
			}
			memset(bufptr, 0, AIO_BLKSIZE);

			io_prep_pwrite(&iocb_array[i], fd, bufptr,
				       AIO_BLKSIZE, offset);
			io_set_callback(&iocb_array[i], work_done);
			iocbps[i] = &iocb_array[i];
			offset += AIO_BLKSIZE;

			break;
		case IO_CMD_PREAD:
			if (posix_memalign(&bufptr, alignment, AIO_BLKSIZE)) {
				perror(" posix_memalign failed ");
				return -1;
			}
			memset(bufptr, 0, AIO_BLKSIZE);

			io_prep_pread(&iocb_array[i], fd, bufptr,
				      AIO_BLKSIZE, offset);
			io_set_callback(&iocb_array[i], work_done);
			iocbps[i] = &iocb_array[i];
			offset += AIO_BLKSIZE;
			break;
		case IO_CMD_POLL:
		case IO_CMD_NOOP:
			break;
		default:
			tst_resm(TFAIL,
				 "Command failed; opcode returned: %d\n",
				 operation);
			return -1;
			break;
		}
	}

	do {
		res = io_submit(myctx, AIO_MAXIO, iocbps);
	} while (res == -EAGAIN);
	if (res < 0) {
		io_error("io_submit tio", res);
	}

	/*
	 * We have submitted all the i/o requests. Wait for at least one to complete
	 * and call the callbacks.
	 */
	wait_count = AIO_MAXIO;

	timeout.tv_sec = 30;
	timeout.tv_nsec = 0;

	switch (operation) {
	case IO_CMD_PREAD:
	case IO_CMD_PWRITE:
		{
			while (wait_count) {
				res = io_wait_run(myctx, &timeout);
				if (res < 0)
					io_error("io_wait_run", res);
			}
		}
		break;
	}

	close(fd);

	for (i = 0; i < AIO_MAXIO; i++) {
		if (iocb_array[i].u.c.buf != NULL) {
			free(iocb_array[i].u.c.buf);
		}
	}

	io_queue_release(myctx);

	return 0;
}

int test_main(void)
{
	int status = 0;

	tst_resm(TINFO, "Running test 1\n");
	status = io_tio("file1",
			O_TRUNC | O_DIRECT | O_WRONLY | O_CREAT | O_LARGEFILE,
			AIO_MAXIO, IO_CMD_PWRITE);
	if (status) {
		return status;
	}

	tst_resm(TINFO, "Running test 2\n");
	status = io_tio("file1", O_RDONLY | O_DIRECT | O_LARGEFILE,
			AIO_MAXIO, IO_CMD_PREAD);
	if (status) {
		return status;
	}

	tst_resm(TINFO, "Running test 3\n");
	status = io_tio("file1", O_TRUNC | O_RDWR, AIO_MAXIO, IO_CMD_PWRITE);
	if (status) {
		return status;
	}

	tst_resm(TINFO, "Running test 4\n");
	status = io_tio("file1", O_RDWR, AIO_MAXIO, IO_CMD_PREAD);
	if (status) {
		return status;
	}

	tst_resm(TINFO, "Running test 5\n");
	status = io_tio("file1", O_TRUNC | O_WRONLY, AIO_MAXIO, IO_CMD_PWRITE);
	if (status) {
		return status;
	}

	tst_resm(TINFO, "Running test 6 \n");
	status = io_tio("file1", O_RDONLY, AIO_MAXIO, IO_CMD_PREAD);
	if (status) {
		return status;
	}

	return status;
}
#endif

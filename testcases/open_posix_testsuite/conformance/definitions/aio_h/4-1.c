/*
 * Copyright (c) 2004, Bull SA. All rights reserved.
 * Created by:  Laurent.Vivier@bull.net
 * This file is licensed under the GPL license.  For the full content
 * of this license, see the COPYING file at the top level of this
 * source tree.
 */

#include <aio.h>

static int (*dummy0) (int, struct aiocb*) = aio_cancel;
static int (*dummy1) (const struct aiocb*) = aio_error;
static int (*dummy2) (int, struct aiocb*) = aio_fsync;
static int (*dummy3) (struct aiocb*) = aio_read;
static ssize_t (*dummy4) (struct aiocb*) = aio_return;
static int (*dummy5) (const struct aiocb* const[], int,
	       const struct timespec *) = aio_suspend;
static int (*dummy6) (struct aiocb *) = aio_write;
static int (*dummy7) (int, struct aiocb *const [restrict],
	       int, struct sigevent *restrict) = lio_listio;

int main(void)
{
	return 0;
}

/*
 * Copyright (c) 2004, Bull SA. All rights reserved.
 * Created by:  Laurent.Vivier@bull.net
 * This file is licensed under the GPL license.  For the full content
 * of this license, see the COPYING file at the top level of this
 * source tree.
 */

 /* test if aio.h exists and can be included */

#include <aio.h>
#include <stdlib.h>		/* For NULL on non-linux platforms. */
#include <string.h>

int main(void)
{
	struct aiocb aiocb;
	struct sigevent sigevent;

	memset(&sigevent, 0, sizeof(sigevent));

	aiocb.aio_fildes = -1;
	aiocb.aio_offset = -1;
	aiocb.aio_buf = NULL;
	aiocb.aio_nbytes = 0;
	aiocb.aio_sigevent = sigevent;
	aiocb.aio_reqprio = -1;

	return 0;
}

/*
 * Copyright (c) 2004, Bull SA. All rights reserved.
 * Created by:  Laurent.Vivier@bull.net
 * This file is licensed under the GPL license.  For the full content
 * of this license, see the COPYING file at the top level of this
 * source tree.
 */

 /* test if aio.h exists and can be included */

#include <aio.h>

int dummy0 = AIO_ALLDONE;
int dummy1 = AIO_CANCELED;
int dummy2 = AIO_NOTCANCELED;
int dummy3 = LIO_NOP;
int dummy4 = LIO_NOWAIT;
int dummy5 = LIO_READ;
int dummy6 = LIO_WAIT;
int dummy7 = LIO_WRITE;

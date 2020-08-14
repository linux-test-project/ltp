/*
 * Copyright (c) 2004, Bull SA. All rights reserved.
 * Created by:  Laurent.Vivier@bull.net
 * This file is licensed under the GPL license.  For the full content
 * of this license, see the COPYING file at the top level of this
 * source tree.
 */

 /* test if aio.h exists and can be included */

#include <aio.h>

static int dummy0 = AIO_ALLDONE;
static int dummy1 = AIO_CANCELED;
static int dummy2 = AIO_NOTCANCELED;
static int dummy3 = LIO_NOP;
static int dummy4 = LIO_NOWAIT;
static int dummy5 = LIO_READ;
static int dummy6 = LIO_WAIT;
static int dummy7 = LIO_WRITE;

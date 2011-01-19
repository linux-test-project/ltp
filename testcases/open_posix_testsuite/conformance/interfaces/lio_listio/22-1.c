/*
 * Copyright (c) 2004, Bull SA. All rights reserved.
 * Created by:  Laurent.Vivier@bull.net
 * This file is licensed under the GPL license.  For the full content
 * of this license, see the COPYING file at the top level of this
 * source tree.
 */

#define _XOPEN_SOURCE 600
#include <stdlib.h>
#include <unistd.h>
#include "posixtest.h"

int main()
{
	if (sysconf(_SC_ASYNCHRONOUS_IO) < 200112L)
		exit(PTS_UNSUPPORTED);
	exit(PTS_UNTESTED);
}
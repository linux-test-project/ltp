/*
 * Copyright (c) 2003, Intel Corporation. All rights reserved.
 * Created by:  julie.n.fleischer REMOVE-THIS AT intel DOT com
 * This file is licensed under the GPL license.  For the full content
 * of this license, see the COPYING file at the top level of this
 * source tree.
 */

/*
 * Will not test returning EACCESS when privileges are denied as POSIX does not
 * define when this error occurs.
 */

#include <stdio.h>
#include "posixtest.h"

int main()
{
	printf("Will not test returning EACCESS when privileges are denied\n");
	printf("as POSIX does not define when this error occurs.\n");
	return PTS_UNTESTED;
}


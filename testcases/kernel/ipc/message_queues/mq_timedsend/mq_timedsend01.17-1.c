/*
 * Copyright (c) 2003, Intel Corporation. All rights reserved.
 * Created by:  geoffrey.r.gustafson REMOVE-THIS AT intel DOT com
 * This file is licensed under the GPL license.  For the full content
 * of this license, see the COPYING file at the top level of this 
 * source tree.
 */

/*
 * Since timeouts can happen at any time on the system, resolution of
 * timeouts cannot be determined.  Will not test timeout resolution.
 */

#include <stdio.h>
#include "posixtest.h"

int main()
{
	printf("Will not test timeout resolution.\n");
	return PTS_UNTESTED;
}

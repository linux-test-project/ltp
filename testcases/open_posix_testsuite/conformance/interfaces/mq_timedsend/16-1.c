/*
 * Copyright (c) 2003, Intel Corporation. All rights reserved.
 * Created by:  geoffrey.r.gustafson REMOVE-THIS AT intel DOT com
 * This file is licensed under the GPL license.  For the full content
 * of this license, see the COPYING file at the top level of this 
 * source tree.
 */

/*
 * As per coverage.txt, these tests assume Timers is not supported.  If
 * it is, tests should be changed to use clock_gettime() instead of
 * time().
 */

#include <stdio.h>
#include "posixtest.h"

int main()
{
	printf("Tests assume Timers is not supported.\n");
	return PTS_UNTESTED;
}

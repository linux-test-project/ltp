/*
 * Copyright (c) 2002, Intel Corporation. All rights reserved.
 * Created by:  julie.n.fleischer REMOVE-THIS AT intel DOT com
 * This file is licensed under the GPL license.  For the full content
 * of this license, see the COPYING file at the top level of this
 * source tree.
 *
 * Test that timer_getoverrun() can only return a value up to but 
 * not including {DELAYTIMER_MAX}.  
 */

#include <time.h>
#include <stdio.h>
#include <limits.h>
#include "posixtest.h"

int main()
{
	printf("Cannot be tested as DELAYTIMER_MAX is too large.\n");
	printf("DELAYTIMER_MAX is %d\n", DELAYTIMER_MAX);
	return PTS_UNRESOLVED;
}

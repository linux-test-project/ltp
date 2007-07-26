/*
 * Copyright (c) 2002, Intel Corporation. All rights reserved.
 * Created by:  julie.n.fleischer REMOVE-THIS AT intel DOT com
 * This file is licensed under the GPL license.  For the full content
 * of this license, see the COPYING file at the top level of this
 * source tree.
 *
 * Test that timer_getoverrun() can only return a value up to but 
 * not including {DELAYTIMER_MAX}. 
 * - adam.li 2004-04-29 Make the output as HEX. 
 * - s.decugis 2005-11-25 Changed return value to UNTESTED.
 */

#include <time.h>
#include <stdio.h>
#include <limits.h>
#include <unistd.h>
#include "posixtest.h"

int main()
{
	long delaytimer_max = sysconf(_SC_DELAYTIMER_MAX);
	printf("Cannot be tested as DELAYTIMER_MAX is too large.\n");
	printf("DELAYTIMER_MAX is %lx\n", delaytimer_max);
	return PTS_UNTESTED;
}

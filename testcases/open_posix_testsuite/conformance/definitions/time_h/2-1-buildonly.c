/*
 * Copyright (c) 2002, Intel Corporation. All rights reserved.
 * Created by:  julie.n.fleischer REMOVE-THIS AT intel DOT com
 * This file is licensed under the GPL license.  For the full content
 * of this license, see the COPYING file at the top level of this
 * source tree.

 * Test that tm structure is declared.
 */
#include <time.h>

struct tm this_type_should_exist, t;

int dummyfcn(void)
{
	int week, year, dst;

	t.tm_sec = 0;
	t.tm_min = 10;
	t.tm_hour = 17;
	t.tm_mday = 1;
	t.tm_mon = 11;
	t.tm_year = 102;
	week = t.tm_wday;
	year = t.tm_yday;
	dst = t.tm_isdst;

	return 0;
}

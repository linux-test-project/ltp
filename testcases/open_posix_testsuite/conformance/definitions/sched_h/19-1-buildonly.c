/*
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 2.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *
 *  Test that inclusion of sched.h makes visible all symbols in time.h.
 *
 * NOTE - THIS TEST CASE IS NOT COMPLETE.  NEEDS TO GROW MUCH LARGER
 * see posixtestsuite / conformance / definitions / signal_h / 50-1.c
 */

#include <sched.h>

struct tm *mytime;

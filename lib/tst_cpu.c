/*
 *   Copyright (c) 2012 Fujitsu Ltd.
 *   Author: Wanlong Gao <gaowanlong@cn.fujitsu.com>
 *
 *   This program is free software;  you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 2 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY;  without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See
 *   the GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program;  if not, write to the Free Software
 *   Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 */

#include <stdlib.h>
#include <unistd.h>
#include "test.h"
#include "safe_macros.h"

long tst_ncpus(void)
{
	long ncpus = -1;
#ifdef _SC_NPROCESSORS_ONLN
	ncpus = SAFE_SYSCONF(NULL, _SC_NPROCESSORS_ONLN);
#else
	tst_brkm(TBROK, NULL, "could not determine number of CPUs online");
#endif
	return ncpus;
}

long tst_ncpus_max(void)
{
	long ncpus_max = -1;
#ifdef _SC_NPROCESSORS_CONF
	ncpus_max = SAFE_SYSCONF(NULL, _SC_NPROCESSORS_CONF);
#else
	tst_brkm(TBROK, NULL, "could not determine number of CPUs configured");
#endif
	return ncpus_max;
}

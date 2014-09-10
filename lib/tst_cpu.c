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

long tst_ncpus_conf(void)
{
	long ncpus_conf = -1;
#ifdef _SC_NPROCESSORS_CONF
	ncpus_conf = SAFE_SYSCONF(NULL, _SC_NPROCESSORS_CONF);
#else
	tst_brkm(TBROK, NULL, "could not determine number of CPUs configured");
#endif
	return ncpus_conf;
}

#define KERNEL_MAX "/sys/devices/system/cpu/kernel_max"

long tst_ncpus_max(void)
{
	long ncpus_max = -1;
	struct stat buf;

	/* sched_getaffinity() and sched_setaffinity() cares about number of
	 * possible CPUs the OS or hardware can support, which can be larger
	 * than what sysconf(_SC_NPROCESSORS_CONF) currently provides
	 * (by enumarating /sys/devices/system/cpu/cpu* entries).
	 *
	 *  Use /sys/devices/system/cpu/kernel_max, if available. This
	 *  represents NR_CPUS-1, a compile time option which specifies
	 *  "maximum number of CPUs which this kernel will support".
	 *  This should provide cpu mask size large enough for any purposes. */
	if (stat(KERNEL_MAX, &buf) == 0) {
		SAFE_FILE_SCANF(NULL, KERNEL_MAX, "%ld", &ncpus_max);
		/* this is maximum CPU index allowed by the kernel
		 * configuration, so # of cpus allowed by config is +1 */
		ncpus_max++;
	} else {
		/* fall back to _SC_NPROCESSORS_CONF */
		ncpus_max = tst_ncpus_conf();
	}
	return ncpus_max;
}

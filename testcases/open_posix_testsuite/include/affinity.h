/*
 *
 *   Copyright (c) Novell Inc. 2011
 *
 *   This program is free software;  you can redistribute it and/or modify
 *   it under the terms in version 2 of the GNU General Public License as
 *   published by the Free Software Foundation.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY;  without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See
 *   the GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program;  if not, write to the Free Software
 *   Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 *
 *   Author:  Peter W. Morreale <pmorreale AT novell DOT com>
 *   Date:    11/08/2011
 */

/*
 * Must be ported to OS's other than Linux
 */

#ifdef __linux__
# define _GNU_SOURCE
# include <sched.h>
# include <stdio.h>

static int set_affinity(int cpu)
{
	cpu_set_t mask;

	CPU_ZERO(&mask);
	CPU_SET(cpu, &mask);

	return (sched_setaffinity(0, sizeof(cpu_set_t), &mask));
}

static int get_online_cpu_from_sysfs(void)
{
	FILE *f;
	int cpu = -1;

	f = fopen("/sys/devices/system/cpu/online", "r");
	if (!f)
		return -1;
	if (!fscanf(f, "%d", &cpu))
		cpu = -1;
	fclose(f);

	return cpu;
}

static int get_online_cpu_from_cpuinfo(void)
{
	FILE *f;
	int cpu = -1;
	char line[4096];

	f = fopen("/proc/cpuinfo", "r");
	if (!f)
		return -1;

	while (!feof(f)) {
		if (!fgets(line, sizeof(line), f))
			return -1;
		/*
		 * cpuinfo output is not consistent across all archictures,
		 * it can be "processor        : N", but for example on s390
		 * it's: "processor N: ...", so ignore any non-number
		 * after "processor"
		 */
		if (sscanf(line, "processor%*[^0123456789]%d", &cpu) == 1)
			break;
	}
	fclose(f);

	return cpu;
}

static int set_affinity_single(void)
{
	int cpu;

	cpu = get_online_cpu_from_sysfs();
	if (cpu >= 0)
		goto set_affinity;

	cpu = get_online_cpu_from_cpuinfo();
	if (cpu >= 0)
		goto set_affinity;

	fprintf(stderr, "WARNING: Failed to detect online cpu, using cpu=0\n");
	cpu = 0;
set_affinity:
	return set_affinity(cpu);
}

#else
#include <errno.h>

static int set_affinity_single(void)
{
	errno = ENOSYS;
	return -1;
}
#endif

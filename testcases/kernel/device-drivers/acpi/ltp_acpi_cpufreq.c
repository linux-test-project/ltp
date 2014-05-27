/*
 * Copyright (c) 2013 Oracle and/or its affiliates. All Rights Reserved.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it would be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write the Free Software Foundation,
 * Inc.,  51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 *
 * Author: Alexey Kodanev <alexey.kodanev@oracle.com>
 *
 * Check support for disabling dynamic overclocking in acpi_cpufreq driver.
 * Required Linux 3.7+.
 *
 * The test loads CPU0 in a thread. When the loading approaches to the end
 * (2/3 elapsed), the test will check processor frequency. if boost is enabled
 * we can get a slightly higher speed under load.
 */

#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <sched.h>
#include <sys/time.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include "test.h"
#include "usctest.h"
#include "safe_macros.h"
#include "safe_stdio.h"

char *TCID = "ltp_acpi_cpufreq";

#define SYSFS_CPU_DIR "/sys/devices/system/cpu/"

const char boost[]	= SYSFS_CPU_DIR "cpufreq/boost";
static int boost_value;

const char governor[]	= SYSFS_CPU_DIR "cpu0/cpufreq/scaling_governor";
static char governor_name[16];

const char setspeed[]	= SYSFS_CPU_DIR "cpu0/cpufreq/scaling_setspeed";
const char curspeed[]	= SYSFS_CPU_DIR "cpu0/cpufreq/cpuinfo_cur_freq";
const char maxspeed[]	= SYSFS_CPU_DIR "cpu0/cpufreq/scaling_max_freq";

const char up_limit[]	= SYSFS_CPU_DIR "cpufreq/ondemand/up_threshold";
static int threshold;

static void cleanup(void)
{
	SAFE_FILE_PRINTF(NULL, boost, "%d", boost_value);

	if (governor[0] != '\0')
		SAFE_FILE_PRINTF(NULL, governor, "%s", governor_name);

	if (threshold)
		SAFE_FILE_PRINTF(NULL, up_limit, "%d", threshold);

	TEST_CLEANUP;
}

static void setup(void)
{
	int fd;
	tst_require_root(NULL);

	fd = open(boost, O_RDWR);
	if (fd == -1) {
		tst_brkm(TCONF, NULL,
			"acpi-cpufreq not loaded or overclock not supported");
	}
	close(fd);

	tst_sig(FORK, DEF_HANDLER, cleanup);

	SAFE_FILE_SCANF(NULL, boost, "%d", &boost_value);

	SAFE_FILE_SCANF(NULL, up_limit, "%d", &threshold);

	/* change cpu0 scaling governor */
	SAFE_FILE_SCANF(NULL, governor, "%s", governor_name);
	SAFE_FILE_PRINTF(cleanup, governor, "%s", "userspace");

	/* use only cpu0 */
	cpu_set_t set;
	CPU_ZERO(&set);
	CPU_SET(0, &set);
	if (sched_setaffinity(0, sizeof(cpu_set_t), &set) < 0)
		tst_brkm(TBROK | TERRNO, cleanup, "failed to set CPU0");

	SAFE_FILE_PRINTF(cleanup, up_limit, "11");
}

static void set_speed(int freq)
{
	int set_freq;
	SAFE_FILE_SCANF(cleanup, setspeed, "%d", &set_freq);

	if (set_freq != freq) {
		tst_resm(TINFO, "change speed from %d to %d...",
			set_freq, freq);
		SAFE_FILE_PRINTF(cleanup, setspeed, "%d", freq);
	} else {
		tst_resm(TINFO, "set speed is %d", set_freq);
	}
}

void *thread_fn(void *val)
{
	struct timeval tv_start;
	struct timeval tv_end;
	int i, res = 0;
	intptr_t timeout = (intptr_t) val;

	gettimeofday(&tv_start, NULL);
	tst_resm(TINFO, "load CPU0 for %ld sec...", timeout);

	do {
		for (i = 1; i < 10000; ++i)
			res += i * i;
		gettimeofday(&tv_end, NULL);
		sched_yield();
	} while ((tv_end.tv_sec - tv_start.tv_sec) < timeout);

	tst_resm(TINFO, "CPU0 load done: insignificant value '%d'", res);

	return NULL;
}

static int load_cpu(intptr_t timeout)
{
	pthread_t thread_id;

	if (pthread_create(&thread_id, 0, thread_fn,
		(void *) timeout) != 0) {
		tst_brkm(TBROK | TERRNO, cleanup,
			"pthread_create failed at %s:%d",
			__FILE__, __LINE__);
	}

	sleep(2 * timeout / 3);

	int cur_freq;
	SAFE_FILE_SCANF(cleanup, curspeed, "%d", &cur_freq);
	tst_resm(TINFO, "got cpu freq under load: %d", cur_freq);

	pthread_join(thread_id, NULL);

	return cur_freq;
}

static void test_run(void)
{
	int cur_freq, max_freq;
	SAFE_FILE_SCANF(cleanup, maxspeed, "%d", &max_freq);
	set_speed(max_freq);

	/* Enable boost */
	if (boost_value == 0)
		SAFE_FILE_PRINTF(cleanup, boost, "1");
	cur_freq = load_cpu(30);
	tst_resm((cur_freq >= max_freq) ? TPASS : TFAIL,
		"compare current speed %d and max speed %d",
		cur_freq, max_freq);

	/* Disable boost */
	SAFE_FILE_PRINTF(cleanup, boost, "0");
	cur_freq = load_cpu(30);
	tst_resm((cur_freq < max_freq) ? TPASS : TFAIL,
		"compare current speed %d and max speed %d",
		cur_freq, max_freq);
}

int main(int argc, char *argv[])
{
	const char *msg;
	msg = parse_opts(argc, argv, NULL, NULL);
	if (msg != NULL)
		tst_brkm(TBROK, NULL, "OPTION PARSING ERROR - %s", msg);

	setup();

	test_run();

	cleanup();

	tst_exit();
}

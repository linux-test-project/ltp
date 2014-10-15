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
 * The test compares time spent on sum calculation with/without
 * boost-disable bit. If boost is enabled we can get a slightly shorter
 * time period. Measure elapsed time instead of sysfs cpuinfo_cur_freq value,
 * because after the upstream commit 8673b83bf2f013379453b4779047bf3c6ae387e4,
 * current cpu frequency became target cpu frequency.
 */

#define _GNU_SOURCE
#include <sched.h>
#include <time.h>

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
const char maxspeed[]	= SYSFS_CPU_DIR "cpu0/cpufreq/scaling_max_freq";

static void cleanup(void)
{
	SAFE_FILE_PRINTF(NULL, boost, "%d", boost_value);

	if (governor[0] != '\0')
		SAFE_FILE_PRINTF(NULL, governor, "%s", governor_name);

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

	/* change cpu0 scaling governor */
	SAFE_FILE_SCANF(NULL, governor, "%s", governor_name);
	SAFE_FILE_PRINTF(cleanup, governor, "%s", "userspace");

	/* use only cpu0 */
	cpu_set_t set;
	CPU_ZERO(&set);
	CPU_SET(0, &set);
	if (sched_setaffinity(0, sizeof(cpu_set_t), &set) < 0)
		tst_brkm(TBROK | TERRNO, cleanup, "failed to set CPU0");

	struct sched_param params;
	params.sched_priority = sched_get_priority_max(SCHED_FIFO);
	if (sched_setscheduler(getpid(), SCHED_FIFO, &params)) {
		tst_resm(TWARN | TERRNO,
			"failed to set FIFO sched with max priority");
	}
}

static void set_speed(int freq)
{
	int set_freq;
	SAFE_FILE_SCANF(cleanup, setspeed, "%d", &set_freq);

	if (set_freq != freq) {
		tst_resm(TINFO, "change target speed from %d KHz to %d KHz",
			set_freq, freq);
		SAFE_FILE_PRINTF(cleanup, setspeed, "%d", freq);
	} else {
		tst_resm(TINFO, "target speed is %d KHz", set_freq);
	}
}

static int load_cpu(int max_freq_khz)
{
	int sum = 0, i = 0, total_time_ms;
	struct timespec tv_start, tv_end;

	const int max_sum = max_freq_khz / 1000;
	const int units = 1000000; /* Mhz */

	clock_gettime(CLOCK_MONOTONIC_RAW, &tv_start);

	do {
		for (i = 0; i < units; ++i)
			asm ("" : : : "memory");
	} while (++sum < max_sum);

	clock_gettime(CLOCK_MONOTONIC_RAW, &tv_end);

	total_time_ms = (tv_end.tv_sec - tv_start.tv_sec) * 1000 +
		(tv_end.tv_nsec - tv_start.tv_nsec) / 1000000;

	if (!total_time_ms)
		tst_brkm(TBROK, cleanup, "time period is 0");

	tst_resm(TINFO, "elapsed time is %d ms", total_time_ms);

	return total_time_ms;
}

static void test_run(void)
{
	int boost_time, boost_off_time, max_freq_khz;
	SAFE_FILE_SCANF(cleanup, maxspeed, "%d", &max_freq_khz);
	set_speed(max_freq_khz);

	/* Enable boost */
	if (boost_value == 0)
		SAFE_FILE_PRINTF(cleanup, boost, "1");
	tst_resm(TINFO, "load CPU0 with boost enabled");
	boost_time = load_cpu(max_freq_khz);

	/* Disable boost */
	SAFE_FILE_PRINTF(cleanup, boost, "0");
	tst_resm(TINFO, "load CPU0 with boost disabled");
	boost_off_time = load_cpu(max_freq_khz);

	boost_off_time *= .98;

	tst_resm((boost_time < boost_off_time) ? TPASS : TFAIL,
		"compare time spent with and without boost (-2%%)");
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

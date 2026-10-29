// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (C) 2026 Intel - http://www.intel.com/
 */

/*\
 * Tests the CPU package thermal sensor interface for Intel platforms.
 *
 * Works by checking the initial count of thermal interrupts. Then it
 * decreases the threshold for sending a thermal interrupt to just above
 * the current temperature and runs a workload on the CPU. Finally, it restores
 * the original thermal threshold and checks whether the number of thermal
 * interrupts increased.
 */

#include <ctype.h>
#include <inttypes.h>
#include "tst_safe_stdio.h"
#include "tst_test.h"
#include "tst_timer_test.h"

#define	TEST_RUNTIME	((RUNTIME + SLEEPTIME) * (SLEEPTIME / 2) + COOLDOWN)
#define	RUNTIME		10
#define	SLEEPTIME	10
#define	COOLDOWN	300
#define	TEMP_INCREMENT	5000

static bool *x86_pkg_temp_tz;
static char temp_path[PATH_MAX], trip_path[PATH_MAX];
static int x86_pkg_temp_tz_found, nproc, temp_high, temp, *trip_orig, tz_counter;
static uint64_t *interrupt_init, *interrupt_later;

static void read_interrupts(uint64_t *interrupts)
{
	bool interrupts_found = false;
	char line[8192];

	memset(interrupts, 0, nproc * sizeof(*interrupts));
	FILE *fp = SAFE_FOPEN("/proc/interrupts", "r");

	while (fgets(line, sizeof(line), fp)) {
		if (!strstr(line, "Thermal event interrupts"))
			continue;

		interrupts_found = true;
		char *ptr = strchr(line, ':');

		for (int i = 0; i < nproc; i++) {
			char *endptr;

			while (*ptr && !isdigit(*ptr))
				ptr++;

			errno = 0;

			interrupts[i] = strtoull(ptr, &endptr, 10);

			if (ptr == endptr)
				tst_brk(TBROK, "CPU %d: interrupt not found", i);

			if (errno == ERANGE)
				tst_brk(TBROK, "CPU %d: interrupt out of range", i);

			ptr = endptr;
			tst_res(TDEBUG, "interrupts[%d]: %" PRIu64, i, interrupts[i]);
		}
		break;
	}
	SAFE_FCLOSE(fp);
	if (!interrupts_found)
		tst_brk(TCONF, "No Thermal event interrupts line in /proc/interrupts");
}

static void setup(void)
{
	char line[8192];

	nproc = tst_ncpus();

	tst_res(TDEBUG, "Number of logical cores: %d", nproc);
	interrupt_init = SAFE_CALLOC(nproc, sizeof(uint64_t));
	interrupt_later = SAFE_CALLOC(nproc, sizeof(uint64_t));

	DIR *dir = SAFE_OPENDIR("/sys/class/thermal/");
	struct dirent *entry;

	while ((entry = SAFE_READDIR(dir))) {
		if ((!strncmp(entry->d_name, "thermal_zone", sizeof("thermal_zone") - 1)))
			tz_counter++;
	}
	SAFE_CLOSEDIR(dir);
	tst_res(TDEBUG, "Found %d thermal zone(s)", tz_counter);

	x86_pkg_temp_tz = SAFE_CALLOC(tz_counter, sizeof(bool));
	trip_orig = SAFE_CALLOC(tz_counter, sizeof(int));

	for (int i = 0; i < tz_counter; i++) {
		char path[PATH_MAX];

		snprintf(path, PATH_MAX, "/sys/class/thermal/thermal_zone%d/type", i);
		tst_res(TDEBUG, "Checking whether %s is x86_pkg_temp", path);

		SAFE_FILE_SCANF(path, "%8191s", line);
		if (strstr(line, "x86_pkg_temp")) {
			tst_res(TDEBUG, "Thermal zone %d uses x86_pkg_temp", i);
			x86_pkg_temp_tz[i] = true;
			x86_pkg_temp_tz_found++;
			snprintf(trip_path, PATH_MAX, "/sys/class/thermal/thermal_zone%d/trip_point_1_temp", i);
			SAFE_ACCESS(trip_path, R_OK | W_OK);
			SAFE_FILE_SCANF(trip_path, "%d", &trip_orig[i]);
		}
	}

	if (!x86_pkg_temp_tz_found)
		tst_brk(TCONF, "No thermal zone uses x86_pkg_temp");

	tst_set_runtime(x86_pkg_temp_tz_found * TEST_RUNTIME);
}

static void cpu_workload(double run_time)
{
	tst_timer_start(CLOCK_MONOTONIC);
	int num = 2;

	while (!tst_timer_expired_ms(run_time * 1000)) {
		for (int i = 2; i * i <= num; i++) {
			if (num % i == 0)
				break;
		}
		num++;
		SAFE_FILE_SCANF(temp_path, "%d", &temp);

		if (temp > temp_high)
			break;
	}
}

static void set_trippoint(void)
{
	SAFE_FILE_SCANF(temp_path, "%d", &temp);
	if (temp < 0)
		tst_brk(TBROK, "Unexpected zone temperature value %d", temp);

	tst_res(TDEBUG, "Current temperature for %s: %d", temp_path, temp);

	temp_high = temp + TEMP_INCREMENT;

	tst_res(TDEBUG, "Setting new trip_point_1_temp value: %d", temp_high);
	SAFE_FILE_PRINTF(trip_path, "%d", temp_high);
}

static void test_zone(int i)
{
	int sleep_time = SLEEPTIME;
	double run_time = RUNTIME;

	snprintf(temp_path, PATH_MAX, "/sys/class/thermal/thermal_zone%d/temp", i);
	snprintf(trip_path, PATH_MAX, "/sys/class/thermal/thermal_zone%d/trip_point_1_temp", i);

	tst_res(TINFO, "Testing %s", temp_path);

	set_trippoint();

	while (sleep_time > 0) {
		tst_res(TDEBUG, "Running for %.1f seconds, then sleeping for %d seconds", run_time, sleep_time);

		for (int j = 0; j < nproc; j++) {
			if (!SAFE_FORK()) {
				cpu_workload(run_time);
				exit(0);
			}
		}

		tst_reap_children();

		SAFE_FILE_SCANF(temp_path, "%d", &temp);
		tst_res(TDEBUG, "Temperature for %s after a test: %d", temp_path, temp);

		if (temp > temp_high)
			break;
		sleep(sleep_time--);
		run_time--;

		set_trippoint();
	}
}

static void cleanup(void)
{
	if (x86_pkg_temp_tz_found) {
		for (int i = 0; i < tz_counter; i++) {
			if (x86_pkg_temp_tz[i]) {
				snprintf(trip_path, PATH_MAX, "/sys/class/thermal/thermal_zone%d/trip_point_1_temp", i);
				SAFE_FILE_PRINTF(trip_path, "%d", trip_orig[i]);
			}
		}
	}

	free(x86_pkg_temp_tz);
	free(interrupt_init);
	free(interrupt_later);
	free(trip_orig);
}

static void run(void)
{
	bool first_zone = true;

	for (int i = 0; i < tz_counter; i++) {
		if (x86_pkg_temp_tz[i]) {
			if (!first_zone) {
				tst_res(TINFO, "Cooling down for %d seconds", COOLDOWN);
				sleep(COOLDOWN);
			}
			first_zone = false;

			read_interrupts(interrupt_init);
			test_zone(i);
			read_interrupts(interrupt_later);

			bool interrupt_increased = false;

			for (int j = 0; j < nproc; j++) {
				if (interrupt_later[j] > interrupt_init[j]) {
					interrupt_increased = true;
					tst_res(TINFO, "CPU %d interrupt counter: %" PRIu64 " (previous: %" PRIu64 ")",
						j, interrupt_later[j], interrupt_init[j]);
				}
			}

			if (!interrupt_increased)
				tst_res(TFAIL, "No thermal interrupt increase detected");
			else if (temp <= temp_high)
				tst_res(TFAIL, "Thermal interrupts increased but temperature did not rise as expected");
			else
				tst_res(TPASS, "x86 package thermal interrupt triggered");
		}
	}
}

static struct tst_test test = {
	.cleanup = cleanup,
	.forks_child = 1,
	.needs_kconfigs = (const char *const []) {
		"CONFIG_X86_PKG_TEMP_THERMAL",
		NULL
	},
	.min_runtime = 5,
	.needs_root = 1,
	.setup = setup,
	.supported_archs = (const char *const []) {
		"x86",
		"x86_64",
		NULL
	},
	.tags = (const struct tst_tag[]) {
		{"linux-git", "9635c586a559ba0e45b2bfbff79c937ddbaf1a62"},
		{}
	},
	.test_all = run
};

// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (C) 2026 Tomasz Ossowski tomasz.ossowski@intel.com
 */

/*\
 * Validate that the platform supports a Dual Core Module for the big core.
 * The test will verify that the scheduler evenly distributes the load between the cores.
 */

#include "tst_safe_stdio.h"
#include "tst_test.h"
#include "tst_timer_test.h"
#include <ctype.h>

struct core_count {
	int number_p_core;
	int number_e_core;
	int number_lpe_core;
};

struct ranges {
	int min;
	int max;
};

static int nproc;
static struct core_count number_cores;
static int *list_core_type; // P core 0, E core 1, LPE 2
static uint64_t *array_usage;
static bool *list_initial_cpu_state;

static bool is_in_range(const int lower_limit, const int upper_limit, const int no)
{
	return (lower_limit <= no && no <= upper_limit);
}

static void count_cores(void)
{
	char path[PATH_MAX];
	struct ranges *list_ranges;
	int number_p_core = 0;
	int number_e_core = 0;
	int number_lpe_core = 0;

	list_ranges = SAFE_MALLOC(2 * sizeof(struct ranges));
	SAFE_FILE_SCANF("/sys/devices/cpu_core/cpus", "%d-%d", &list_ranges[0].min, &list_ranges[0].max);
	SAFE_FILE_SCANF("/sys/devices/cpu_atom/cpus", "%d-%d", &list_ranges[1].min, &list_ranges[1].max);

	for (int i = 0; i < nproc; i++) {
		if (is_in_range(list_ranges[0].min, list_ranges[0].max, i)) {
			list_core_type[i] = 0;
			number_p_core++;
		}

		else if (is_in_range(list_ranges[1].min, list_ranges[1].max, i)) {
			bool found_cache_l3 = false;

			for (int idx = 0; idx < 32; idx++) {
				snprintf(path, sizeof(path), "/sys/devices/system/cpu/cpu%d/cache/index%d/level", i, idx);

				if (access(path, F_OK) == 0) {
					int value;

					SAFE_FILE_SCANF(path, "%d", &value);
					if (value == 3) {
						list_core_type[i] = 1;
						number_e_core++;
						found_cache_l3 = true;
						break;
					}
				}
			}
			if(!found_cache_l3) {
				list_core_type[i] = 2;
				number_lpe_core++;
			}
		}
	}
	number_cores.number_p_core = number_p_core;
	number_cores.number_e_core = number_e_core;
	number_cores.number_lpe_core = number_lpe_core;

	tst_res(TINFO,
			"P core: %d, E core: %d, LPE core: %d",
			number_p_core,
			number_e_core,
			number_lpe_core);
	if(list_ranges)
		free(list_ranges);
}

static void *cpu_workload(void *arg)
{
	double run_time = *(double *) arg;

	tst_timer_start(CLOCK_MONOTONIC);
	int num = 2;

	while (!tst_timer_expired_ms(run_time * 1000)) {
		for (int i = 2; i * i <= num; i++) {
			if (num % i == 0)
				break;
		}
		num++;
	}
	return NULL;
}

static void cleanup(void)
{
	if (array_usage)
		free(array_usage);
	if(list_core_type)
		free(list_core_type);

	for (int i = 1; i < nproc; i++) {
		char path[PATH_MAX];

		snprintf(path, sizeof(path), "/sys/devices/system/cpu/cpu%d/online", i);
		SAFE_FILE_PRINTF(path, "%d", list_initial_cpu_state[i]);
	}

	if(list_initial_cpu_state)
		free(list_initial_cpu_state);
}

static void setup(void)
{
	nproc = tst_ncpus_conf();
	list_initial_cpu_state = SAFE_MALLOC(nproc * sizeof(bool));
	list_initial_cpu_state[0] = true;

	for (int i = 1; i < nproc; i++) {
		char path[PATH_MAX];
		int online = 1;

		snprintf(path, sizeof(path), "/sys/devices/system/cpu/cpu%d/online", i);
		SAFE_FILE_SCANF(path, "%d", &online);
		list_initial_cpu_state[i] = (bool)online;
		if(!online) {
			tst_res(TDEBUG, "Set Core %d online", i);
			SAFE_FILE_PRINTF(path, "1");
		}
	}
	list_core_type = SAFE_MALLOC(nproc * sizeof(int));
	for(int i=0; i< nproc; i++)
		list_core_type[i] = -1;
	count_cores();
}

static void count_utilized_cores(uint64_t *usage_per_core_array, const int expected_p, const int expected_e, const int expected_lpe)
{
	const int usage_threshold = 80;

	int p_usage = 0;
	int e_usage = 0;
	int lpe_usage = 0;

	for (int i = 0; i < nproc; i++) {
		if (usage_per_core_array[i] > usage_threshold) {
			if (list_core_type[i] == 0)
				p_usage++;
			else if (list_core_type[i] == 1)
				e_usage++;
			else
				lpe_usage++;
		}
	}
	tst_res(TDEBUG, "%d p core utilized by workload", p_usage);
	tst_res(TDEBUG, "%d e core utilized by workload", e_usage);
	tst_res(TDEBUG, "%d lpe core utilized by workload", lpe_usage);

	if (p_usage == expected_p && e_usage == expected_e)
		tst_res(TPASS,
				"tasks distributed as expected: P=%d, E=%d, LPE=%d",
				p_usage,
				e_usage,
				lpe_usage);
	else
		tst_res(TFAIL,
				"unexpected distribution: P %d/%d, E %d/%d, LPE %d/%d (actual/expected)",
				p_usage,
				expected_p,
				e_usage,
				expected_e,
				lpe_usage,
				expected_lpe);
}

static void read_cpu(uint64_t idle_list[], uint64_t total_list[])
{
	char line[PATH_MAX];

	FILE *fp = SAFE_FOPEN("/proc/stat", "r");
	int cpuid;
	uint64_t user, nice, system, idle, iowait, irq, softirq, steal;

	while (fgets(line, sizeof(line), fp)) {
		if (strncmp(line, "cpu", 3))
			continue;

		if (!isdigit(line[3]))
			continue;

		SAFE_SSCANF(line,
					"cpu%d %lu %lu %lu %lu %lu %lu %lu %lu",
					&cpuid,
					&user,
					&nice,
					&system,
					&idle,
					&iowait,
					&irq,
					&softirq,
					&steal);
		if (cpuid < nproc) {
			idle_list[cpuid] = idle;
			total_list[cpuid] = user + nice + system + idle + iowait + irq + softirq + steal;
		}
	}
	SAFE_FCLOSE(fp);
}

static uint64_t *count_usage_per_cpu(void)
{
	if (array_usage)
		free(array_usage);

	uint64_t *array_idle_before, *array_total_before, *array_idle_after, *array_total_after;

	array_idle_before = SAFE_MALLOC(nproc * sizeof(uint64_t));
	array_total_before = SAFE_MALLOC(nproc * sizeof(uint64_t));

	array_idle_after = SAFE_MALLOC(nproc * sizeof(uint64_t));
	array_total_after = SAFE_MALLOC(nproc * sizeof(uint64_t));

	read_cpu(array_idle_before, array_total_before);
	sleep(2);
	read_cpu(array_idle_after, array_total_after);

	array_usage = SAFE_MALLOC(nproc * sizeof(uint64_t));
	for (int i = 0; i < nproc; i++) {
		uint64_t total_diff = array_total_after[i] - array_total_before[i];
		uint64_t idle_diff = array_idle_after[i] - array_idle_before[i];

		if (total_diff == 0) {
			array_usage[i] = 0;
			continue;
		}

		double cpu_usage = ((double) (total_diff - idle_diff) / (double) total_diff) * 100.0;

		if (cpu_usage < 0.0)
			cpu_usage = 0.0;
		if (cpu_usage > 100.0)
			cpu_usage = 100.0;

		tst_res(TINFO, "CPU%d Usage: %.2f%%\n", i, cpu_usage);
		array_usage[i] = (uint64_t) cpu_usage;
	}

	free(array_idle_before);
	free(array_total_before);
	free(array_idle_after);
	free(array_total_after);

	return array_usage;
}

static void run(void)
{
	double run_workload_time = 30;
	pid_t *pid_write;

	pid_write = SAFE_MALLOC(nproc * sizeof(pid_t));

	int half_p_cores = div(number_cores.number_p_core, 2).quot;

	for (int j = 0; j < half_p_cores; j++) {
		pid_write[j] = SAFE_FORK();
		if (!pid_write[j]) {
			cpu_workload(&run_workload_time);
			exit(0);
		}
	}

	count_utilized_cores(count_usage_per_cpu(), half_p_cores, 0, 0);
	tst_reap_children();

	int half_e_cores = div(number_cores.number_e_core, 2).quot;
	int all_p_cores_half_ecores = number_cores.number_p_core + half_e_cores;

	for (int j = 0; j < all_p_cores_half_ecores; j++) {
		pid_write[j] = SAFE_FORK();
		if (!pid_write[j]) {
			cpu_workload(&run_workload_time);
			exit(0);
		}
	}

	count_utilized_cores(count_usage_per_cpu(), number_cores.number_p_core, half_e_cores, 0);
	tst_reap_children();
	free(pid_write);
}

static struct tst_test test = {
	.min_runtime = 80,
	.forks_child = 1,
	.setup = setup,
	.supported_archs = (const char *const[]) {
		"x86",
		"x86_64",
		 NULL
	},

	.min_kver = "6.19",
	.test_all = run,
	.cleanup = cleanup
};

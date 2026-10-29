// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2017 Cyril Hrubis <chrubis@suse.cz>
 */

#include <sys/prctl.h>
#include <stdlib.h>
#include <stdio.h>
#include <limits.h>
#include <string.h>

#define TST_NO_DEFAULT_MAIN
#include "tst_test.h"
#include "tst_clocks.h"
#include "tst_timer_test.h"

#define MAX_SAMPLES 500

static const char *scall;
static void (*setup)(void);
static void (*cleanup)(void);
static int (*sample)(int clk_id, long long usec);
static struct tst_test *test;

static long long *samples;
static unsigned int cur_sample;
static unsigned int monotonic_resolution;
static unsigned int timerslack;
static int virt_env;

static char *print_frequency_plot;
static char *file_name;
static char *str_sleep_time;
static char *str_sample_cnt;
static int sleep_time = -1;
static int sample_cnt;

static void print_line(char c, int len)
{
	while (len-- > 0)
		fputc(c, stderr);
}

static unsigned int ceilu(float f)
{
	if (f - (int)f > 0)
		return (unsigned int)f + 1;

	return (unsigned int)f;
}

static unsigned int flooru(float f)
{
	return (unsigned int)f;
}

static float bucket_len(unsigned int bucket, unsigned int max_bucket,
		        unsigned int cols)
{
	return 1.00 * bucket * cols / max_bucket;
}

static const char *table_heading = " Time: us ";

/*
 * Line Header: '10023 | '
 */
static unsigned int header_len(long long max_sample)
{
	size_t l = 1;

	while (max_sample/=10)
		l++;

	return MAX(strlen(table_heading) + 2, l + 3);
}

static void frequency_plot(void)
{
	unsigned int cols = 80;
	unsigned int rows = 20;
	unsigned int i, buckets[rows];
	long long max_sample = samples[0];
	long long min_sample = samples[cur_sample-1];
	unsigned int line_header_len = header_len(max_sample);
	unsigned int plot_line_len = cols - line_header_len;
	unsigned int bucket_size;

	memset(buckets, 0, sizeof(buckets));

	/*
	 * We work with discrete data buckets smaller than 1 does not make
	 * sense as well as it's a good idea to keep buckets integer sized
	 * to avoid scaling artifacts.
	 */
	bucket_size = MAX(1u, ceilu(1.00 * (max_sample - min_sample)/(rows-1)));

	for (i = 0; i < cur_sample; i++) {
		unsigned int bucket;
		bucket = flooru(1.00 * (samples[i] - min_sample)/bucket_size);
		buckets[bucket]++;
	}

	unsigned int max_bucket = buckets[0];
	for (i = 1; i < rows; i++)
		max_bucket = MAX(max_bucket, buckets[i]);

	fprintf(stderr, "\n%*s| Frequency\n", line_header_len - 2, table_heading);

	print_line('-', cols);
	fputc('\n', stderr);

	unsigned int l, r;

	for (l = 0; l < rows; l++) {
		if (buckets[l])
			break;
	}

	for (r = rows-1; r > l; r--) {
		if (buckets[r])
			break;
	}

	for (i = l; i <= r; i++) {
		float len = bucket_len(buckets[i], max_bucket, plot_line_len);

		fprintf(stderr, "%*lli | ",
			line_header_len - 3, min_sample + bucket_size*i);
		print_line('*', len);

		if ((len - (int)len) >= 0.5)
			fputc('+', stderr);
		else if ((len - (int)len) >= 0.25)
			fputc('-', stderr);
		else if (len < 0.25 && buckets[i])
			fputc('.', stderr);

		fputc('\n', stderr);
	}

	print_line('-', cols);
	fputc('\n', stderr);

	float scale = 1.00 * plot_line_len / max_bucket;

	fprintf(stderr,
		"%*uus | 1 sample = %.5f '*', %.5f '+', %.5f '-', non-zero '.'\n",
		line_header_len - 5, bucket_size, scale, scale * 2, scale * 4);

	fputc('\n', stderr);
}

void tst_timer_sample(void)
{
	samples[cur_sample++] = tst_timer_elapsed_us();
}

static int cmp(const void *a, const void *b)
{
	const long long *aa = a, *bb = b;

	return (*bb - *aa);
}

/*
 * The threshold per one syscall is computed as a sum of:
 *
 *  400 us                 - accomodates for context switches, process
 *                           migrations between CPUs on SMP, etc.
 *  2*monotonic_resolution - accomodates for granurality of the CLOCK_MONOTONIC
 *  slack_per_scall        - max of 0.1% of the sleep capped on 100ms or
 *                           current->timer_slack_ns, which is slack allowed
 *                           in kernel
 *
 *  The formula	for slack_per_scall applies to select() and *poll*() syscalls,
 *  the futex and *nanosleep() use only the timer_slack_ns, so we are a bit
 *  less strict here that we could be for these two for longer sleep times...
 *
 * We also allow for outliners, i.e. add some number to the threshold in case
 * that the number of iteration is small. For large enoung number of iterations
 * outliners are discarded and averaged out.
 */
static long long compute_threshold(long long requested_us,
				   unsigned int nsamples)
{
	unsigned int slack_per_scall = MIN(100000LL, requested_us / 1000);

	slack_per_scall = MAX(slack_per_scall, timerslack);

	return (400 + 2 * monotonic_resolution + slack_per_scall) * nsamples
		+ 3000/nsamples;
}

/*
 * Returns number of samples to discard.
 *
 * We set it to either at least 1 if number of samples > 1 or 5%.
 */
static unsigned int compute_discard(unsigned int nsamples)
{
	if (nsamples == 1)
		return 0;

	return MAX(1u, nsamples / 20);
}

static void write_to_file(void)
{
	unsigned int i;
	FILE *f;

	if (!file_name)
		return;

	f = fopen(file_name, "w");

	if (!f) {
		tst_res(TWARN | TERRNO,
			"Failed to open '%s'", file_name);
		return;
	}

	for (i = 0; i < cur_sample; i++)
		fprintf(f, "%lli\n", samples[i]);

	if (fclose(f)) {
		tst_res(TWARN | TERRNO,
			"Failed to close file '%s'", file_name);
	}
}


/*
 * Timer testing function.
 *
 * What we do here is:
 *
 * * Take nsamples measurements of the timer function, the function
 *   to be sampled is defined in the actual test.
 *
 * * We sort the array of samples, then:
 *
 *   - look for outliners which are samples where the sleep time has exceeded
 *     requested sleep time by an order of magnitude and, at the same time, are
 *     greater than clock resolution multiplied by three.
 *
 *   - check for samples where the call has woken up too early which is a plain
 *     old bug
 *
 *   - then we compute truncated mean and compare that with the requested sleep
 *     time increased by a threshold
 */
void do_timer_test(long long usec, unsigned int nsamples)
{
	long long trunc_mean, median;
	unsigned int discard = compute_discard(nsamples);
	unsigned int keep_samples = nsamples - discard;
	long long threshold = compute_threshold(usec, keep_samples);
	int i;
	int failed = 0;

	tst_res(TINFO,
		"%s sleeping for %llius %u iterations, threshold %.2fus",
		scall, usec, nsamples, 1.00 * threshold / (keep_samples));

	cur_sample = 0;
	for (i = 0; i < (int)nsamples; i++) {
		if (sample(CLOCK_MONOTONIC, usec)) {
			tst_res(TINFO, "sampling function failed, exiting");
			return;
		}
	}

	qsort(samples, nsamples, sizeof(samples[0]), cmp);

	write_to_file();

	for (i = 0; samples[i] > 10 * usec && i < (int)nsamples; i++) {
		if (samples[i] <= 3 * monotonic_resolution)
			break;
	}

	if (i > 0) {
		tst_res(TINFO, "Found %i outliners in [%lli,%lli] range",
			i, samples[0], samples[i-1]);
	}

	for (i = nsamples - 1; samples[i] < usec && i > -1; i--);

	if (i < (int)nsamples - 1) {
		tst_res(TFAIL, "%s woken up early %u times range: [%lli,%lli]",
			scall, nsamples - 1 - i,
			samples[i+1], samples[nsamples-1]);
		failed = 1;
	}

	median = samples[nsamples/2];

	trunc_mean = 0;

	for (i = discard; i < (int)nsamples; i++)
		trunc_mean += samples[i];

	tst_res(TINFO,
		"min %llius, max %llius, median %llius, trunc mean %.2fus (discarded %u)",
		samples[nsamples-1], samples[0], median,
		1.00 * trunc_mean / keep_samples, discard);

	if (virt_env) {
		tst_res(TINFO,
			"Virtualisation detected, skipping oversleep checks");
	} else if (trunc_mean > (nsamples - discard) * usec + threshold) {
		tst_res(TFAIL, "%s slept for too long", scall);

		if (!print_frequency_plot)
			frequency_plot();

		failed = 1;
	}

	if (print_frequency_plot)
		frequency_plot();

	if (!failed)
		tst_res(TPASS, "Measured times are within thresholds");
}

static void parse_timer_opts(void);

static int set_latency(void)
{
        int fd, latency = 0;

        fd = open("/dev/cpu_dma_latency", O_WRONLY);
        if (fd < 0)
                return fd;

        return write(fd, &latency, sizeof(latency));
}

static void timer_setup(void)
{
	struct timespec t;
	int ret;

	if (setup)
		setup();

	/*
	 * Running tests in VM may cause timing issues, disable upper bound
	 * checks if any hypervisor is detected.
	 */
	virt_env = tst_is_virt(VIRT_ANY);
	tst_clock_getres(CLOCK_MONOTONIC, &t);

	tst_res(TINFO, "CLOCK_MONOTONIC resolution %lins", (long)t.tv_nsec);

	monotonic_resolution = t.tv_nsec / 1000;
	timerslack = 50;

#ifdef PR_GET_TIMERSLACK
	ret = prctl(PR_GET_TIMERSLACK);
	if (ret < 0) {
		tst_res(TINFO, "prctl(PR_GET_TIMERSLACK) = -1, using %uus",
			timerslack);
	} else {
		timerslack = ret / 1000;
		tst_res(TINFO, "prctl(PR_GET_TIMERSLACK) = %ius", timerslack);
	}
#else
	tst_res(TINFO, "PR_GET_TIMERSLACK not defined, using %uus",
		timerslack);
#endif /* PR_GET_TIMERSLACK */
	parse_timer_opts();

	samples = SAFE_MALLOC(sizeof(long long) * MAX(MAX_SAMPLES, sample_cnt));
	if (set_latency() < 0)
		tst_res(TINFO, "Failed to set zero latency constraint: %m");
}

static void timer_cleanup(void)
{
	free(samples);

	if (cleanup)
		cleanup();
}

static struct tst_timer_tcase {
	long long usec;
	unsigned int samples;
} tcases[] = {
	{1000,  500},
	{2000,  500},
	{5000,  300},
	{10000, 100},
	{25000,  50},
	{100000, 10},
	{1000000, 2},
};

static void timer_test_fn(unsigned int n)
{
	do_timer_test(tcases[n].usec, tcases[n].samples);
}

static void single_timer_test(void)
{
	do_timer_test(sleep_time, sample_cnt);
}

static struct tst_option options[] = {
	{"p",  &print_frequency_plot, "-p       Print frequency plot"},
	{"s:", &str_sleep_time, "-s us    Sleep time"},
	{"n:", &str_sample_cnt, "-n uint  Number of samples to take"},
	{"f:", &file_name, "-f fname Write measured samples into a file"},
	{NULL, NULL, NULL}
};

static void parse_timer_opts(void)
{
	size_t i;
	long long runtime_us = 0;

	if (str_sleep_time) {
		if (tst_parse_int(str_sleep_time, &sleep_time, 0, INT_MAX)) {
			tst_brk(TBROK,
				"Invalid sleep time '%s'", str_sleep_time);
		}
	}

	if (str_sample_cnt) {
		if (tst_parse_int(str_sample_cnt, &sample_cnt, 1, INT_MAX)) {
			tst_brk(TBROK,
				"Invalid sample count '%s'", str_sample_cnt);
		}
	}

	if (str_sleep_time || str_sample_cnt) {
		if (sleep_time < 0)
			sleep_time = 10000;

		if (!sample_cnt)
			sample_cnt = 500;

		runtime_us = sleep_time * sample_cnt;

		test->test_all = single_timer_test;
		test->test = NULL;
		test->tcnt = 0;
	} else {
		for (i = 0; i < ARRAY_SIZE(tcases); i++)
			runtime_us += tcases[i].usec * tcases[i].samples;
	}

	tst_set_runtime((runtime_us + runtime_us/10)/1000000);
}

struct tst_test *tst_timer_test_setup(struct tst_test *timer_test)
{
	setup = timer_test->setup;
	cleanup = timer_test->cleanup;
	scall = timer_test->scall;
	sample = timer_test->sample;

	timer_test->scall = NULL;
	timer_test->setup = timer_setup;
	timer_test->cleanup = timer_cleanup;
	timer_test->test = timer_test_fn;
	timer_test->tcnt = ARRAY_SIZE(tcases);
	timer_test->sample = NULL;
	timer_test->options = options;

	test = timer_test;

	return timer_test;
}

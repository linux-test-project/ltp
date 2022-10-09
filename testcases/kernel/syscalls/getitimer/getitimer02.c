// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) International Business Machines  Corp., 2001
 *               03/2001 - Written by Wayne Boyer
 * Copyright (c) 2022 SUSE LLC Avinesh Kumar <avinesh.kumar@suse.com>
 */

/*\
 * [Description]
 *
 * Check that getitimer() call fails:
 *
 * 1. EFAULT with invalid itimerval pointer
 * 2. EINVAL when called with an invalid first argument
 */

#include <stdlib.h>
#include <errno.h>
#include <sys/time.h>
#include "tst_test.h"
#include "lapi/syscalls.h"

static struct itimerval *value;
static struct itimerval *invalid;

static struct tcase {
       int which;
       struct itimerval **val;
       int exp_errno;
} tcases[] = {
       {ITIMER_REAL,    &invalid, EFAULT},
       {ITIMER_VIRTUAL, &invalid, EFAULT},
       {-ITIMER_PROF,   &value,   EINVAL},
};

static int sys_getitimer(int which, void *curr_value)
{
        return tst_syscall(__NR_getitimer, which, curr_value);
}

static void setup(void)
{
        value = SAFE_MALLOC(sizeof(struct itimerval));
        invalid = (struct itimerval *)-1;
}

static void verify_getitimer(unsigned int i)
{
        struct tcase *tc = &tcases[i];

        TST_EXP_FAIL(sys_getitimer(tc->which, *(tc->val)), tc->exp_errno);
}

static void cleanup(void)
{
        free(value);
        value = NULL;
}

static struct tst_test test = {
        .tcnt = ARRAY_SIZE(tcases),
        .test = verify_getitimer,
        .setup = setup,
        .cleanup = cleanup,
};

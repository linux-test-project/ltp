// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (C) Bull S.A. 2001
 * Copyright (c) International Business Machines  Corp., 2001
 * 05/2002 Ported by Jacky Malcles
 */

/*\
 * [Description]
 *
 * Testcase to check EISDIR error when fd refers to a directory.
 */

#include <sys/uio.h>
#include <fcntl.h>
#include "tst_test.h"

#define K_1     1024
#define MODES   S_IRWXU

static char buf1[K_1];

static struct iovec rd_iovec[1] = {
        {buf1, K_1}
};

const char *TEST_DIR = "alpha";
static int fd;

static void verify_readv(void)
{
        TST_EXP_FAIL2(readv(fd, rd_iovec, 1), EISDIR,
                     "readv() got EISDIR");
}

void setup(void)
{
        SAFE_MKDIR(TEST_DIR, MODES);
        fd = SAFE_OPEN(TEST_DIR, O_RDONLY);
}

static void cleanup(void)
{
        if (fd > 0)
                SAFE_CLOSE(fd);
}

static struct tst_test test = {
        .needs_tmpdir = 1,
        .setup = setup,
        .cleanup = cleanup,
        .test_all = verify_readv,
};

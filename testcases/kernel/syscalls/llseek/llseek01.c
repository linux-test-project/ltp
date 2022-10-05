// SPDX-License-Identifier: GPL-2.0-or-later
/*
 *   Copyright (c) International Business Machines  Corp., 2001
 *   07/2001 Ported by Wayne Boyer
 */
/*
 * Verify that lseek64() call succeeds to set the file pointer position to an
 * offset larger than file size limit (RLIMIT_FSIZE). Also, verify that any
 * attempt to write to this location fails.
 */

#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif

#include <sys/types.h>
#include <unistd.h>
#include <stdio.h>

#include "tst_test.h"

#define TEMP_FILE	"tmp_file"
#define FILE_MODE	0644

static char write_buff[BUFSIZ];
static int fildes;

static void verify_llseek(void)
{
	TEST(lseek64(fildes, (loff_t) (80 * BUFSIZ), SEEK_SET));
	if (TST_RET == (80 * BUFSIZ))
		tst_res(TPASS, "lseek64() can set file pointer position larger than file size limit");
	else
		tst_res(TFAIL, "lseek64() returned wrong value %ld when write past file size", TST_RET);

	if (write(fildes, write_buff, BUFSIZ) == -1)
		tst_res(TPASS,"write failed after file size limit");
	else
		tst_brk(TFAIL, "write successful after file size limit");

	TEST(lseek64(fildes, (loff_t) BUFSIZ, SEEK_SET));
	if (TST_RET == BUFSIZ)
		tst_res(TPASS,"lseek64() can set file pointer position under filer size limit");
	else
		tst_brk(TFAIL,"lseek64() returns wrong value %ld when write under file size", TST_RET);

	if (write(fildes, write_buff, BUFSIZ) != -1)
		tst_res(TPASS, "write succcessfully under file size limit");
	else
		tst_brk(TFAIL, "write failed under file size limit");

	if (write(fildes, write_buff, BUFSIZ) == -1)
		tst_res(TPASS, "write failed after file size limit");
	else
		tst_brk(TFAIL, "write successfully after file size limit");
}

static void setup(void)
{
	struct sigaction act;
	struct rlimit rlp;

	act.sa_handler = SIG_IGN;
	act.sa_flags = 0;
	sigemptyset(&act.sa_mask);
	SAFE_SIGACTION(SIGXFSZ, &act, NULL);

	rlp.rlim_cur = rlp.rlim_max = 2 * BUFSIZ;
	SAFE_SETRLIMIT(RLIMIT_FSIZE, &rlp);

	fildes = SAFE_OPEN(TEMP_FILE, O_RDWR | O_CREAT, FILE_MODE);

	SAFE_WRITE(SAFE_WRITE_ALL, fildes, write_buff, BUFSIZ);
}

static struct tst_test test = {
	.setup = setup,
	.needs_tmpdir = 1,
	.test_all = verify_llseek,
};

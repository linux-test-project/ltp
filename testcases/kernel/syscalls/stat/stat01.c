/*
 * Copyright (c) International Business Machines  Corp., 2001
 *  07/2001 John George
 *
 * This program is free software;  you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY;  without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See
 * the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program;  if not, write to the Free Software Foundation,
 * Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 */
/*
 *  Verify that, stat(2) succeeds to get the status of a file and fills the
 *  stat structure elements.
 */
#include <stdio.h>
#include <sys/types.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <errno.h>
#include <string.h>
#include <signal.h>
#include <pwd.h>

#include "test.h"
#include "safe_macros.h"

#define FILE_MODE	0644
#define TESTFILE	"testfile"
#define FILE_SIZE       1024
#define BUF_SIZE	256
#define MASK		0777

char *TCID = "stat01";
int TST_TOTAL = 1;

static uid_t user_id;
static gid_t group_id;

static void setup(void);
static void cleanup(void);

static void verify(void)
{
	struct stat stat_buf;
	int fail = 0;

	TEST(stat(TESTFILE, &stat_buf));

	if (TEST_RETURN == -1) {
		tst_resm(TFAIL | TTERRNO, "fstat(%s) failed", TESTFILE);
		return;
	}

	if (stat_buf.st_uid != user_id) {
		tst_resm(TINFO, "stat_buf.st_uid = %i expected %i",
		         stat_buf.st_uid, user_id);
		fail++;
	}

	if (stat_buf.st_gid != group_id) {
		tst_resm(TINFO, "stat_buf.st_gid = %i expected %i",
		         stat_buf.st_gid, group_id);
		fail++;
	}

	if (stat_buf.st_size != FILE_SIZE) {
		tst_resm(TINFO, "stat_buf.st_size = %zu expected %i",
		         stat_buf.st_size, FILE_SIZE);
		fail++;
	}

        if ((stat_buf.st_mode & MASK) != FILE_MODE) {
		tst_resm(TINFO, "stat_buf.st_mode = %o expected %o",
		         (stat_buf.st_mode & MASK), FILE_MODE);
		fail++;
	}

	if (fail) {
		tst_resm(TFAIL, "functionality of fstat incorrect");
		return;
	}

	tst_resm(TPASS, "functionality of fstat correct");
}

int main(int ac, char **av)
{
	int lc;

	tst_parse_opts(ac, av, NULL, NULL);

	setup();

	for (lc = 0; TEST_LOOPING(lc); lc++)
		verify();

	cleanup();
	tst_exit();
}

static void setup(void)
{
	struct passwd *ltpuser;
	char tst_buff[BUF_SIZE];
	int wbytes;
	int write_len = 0;
	int fd;

	tst_require_root();

	tst_sig(NOFORK, DEF_HANDLER, cleanup);

	ltpuser = SAFE_GETPWNAM(NULL, "nobody");
	SAFE_SETUID(NULL, ltpuser->pw_uid);

	TEST_PAUSE;

	tst_tmpdir();

	umask(022);

	fd = SAFE_OPEN(tst_rmdir, TESTFILE, O_WRONLY | O_CREAT, FILE_MODE);

	/* Fill the test buffer with the known data */
	memset(tst_buff, 'a', BUF_SIZE - 1);

	/* Write to the file 1k data from the buffer */
	while (write_len < FILE_SIZE) {
		if ((wbytes = write(fd, tst_buff, sizeof(tst_buff))) <= 0)
			tst_brkm(TBROK | TERRNO, cleanup, "write failed");
		else
			write_len += wbytes;
	}

	SAFE_CLOSE(tst_rmdir, fd);

	user_id = getuid();
	group_id = getgid();
}

static void cleanup(void)
{
	tst_rmdir();
}

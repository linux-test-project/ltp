/*
 *
 *   Copyright (c) International Business Machines  Corp., 2001
 *
 *   This program is free software;  you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 2 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY;  without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See
 *   the GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program;  if not, write to the Free Software
 *   Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 */

/*
 * NAME
 *	execve03.c
 *
 * DESCRIPTION
 *	Testcase to check execve sets the following errnos correctly:
 *	1.	ENAMETOOLONG
 *	2.	ENOENT
 *	3.	ENOTDIR
 *	4.	EFAULT
 *	5.	EACCES
 *	6.	ENOEXEC
 *
 * ALGORITHM
 *	1.	Attempt to execve(2) a file whose name is more than
 *		VFS_MAXNAMLEN fails with ENAMETOOLONG.
 *
 *	2.	Attempt to execve(2) a file which doesn't exist fails with
 *		ENOENT.
 *
 *	3.	Attempt to execve(2) a pathname (executabl) comprising of a
 *		directory, which doesn't exist fails with ENOTDIR.
 *
 *	4.	Attempt to execve(2) a filename not within the address space
 *		of the process fails with EFAULT.
 *
 *	5.	Attempt to execve(2) a filename that does not have executable
 *		permission - fails with EACCES.
 *
 *	6.	Attempt to execve(2) a zero length file with executable
 *		permissions - fails with ENOEXEC.
 *
 * USAGE:  <for command-line>
 *  execve03 [-c n] [-e] [-i n] [-I x] [-P x] [-t]
 *     where,  -c n : Run n copies concurrently.
 *             -e   : Turn on errno logging.
 *             -i n : Execute test n times.
 *             -I x : Execute test for x seconds.
 *             -P x : Pause for x seconds between iterations.
 *             -t   : Turn on syscall timing.
 *
 * HISTORY
 *	07/2001 Ported by Wayne Boyer
 *
 * RESTRICTIONS
 *	test #5 will fail with ETXTBSY not EACCES if the test is run as root
 */

#include <sys/types.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <errno.h>
#include <fcntl.h>
#include <pwd.h>
#include <stdio.h>

#include "test.h"
#include "safe_macros.h"

char *TCID = "execve03";
int fileHandle = 0;

char nobody_uid[] = "nobody";
struct passwd *ltpuser;

#ifndef UCLINUX
void *bad_addr = NULL;
#endif

void setup(void);
void cleanup(void);

char long_file[] =
    "abcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstmnopqrstuvwxyzabcdefghijklmnopqrstmnopqrstuvwxyzabcdefghijklmnopqrstmnopqrstuvwxyzabcdefghijklmnopqrstmnopqrstuvwxyzabcdefghijklmnopqrstmnopqrstuvwxyzabcdefghijklmnopqrstmnopqrstuvwxyz";
char no_dir[] = "testdir";
char test_name3[1000];
char test_name5[1000];
char test_name6[1000];

struct test_case_t {
	char *tname;		/* the command name to pass to execve() */
	int error;
} TC[] = {
	/* the file name is greater than VFS_MAXNAMELEN - ENAMTOOLONG */
	{
	long_file, ENAMETOOLONG},
	    /* the filename does not exist - ENOENT */
	{
	no_dir, ENOENT},
	    /* the path contains a directory name which doesn't exist - ENOTDIR */
	{
	test_name3, ENOTDIR},
#if !defined(UCLINUX)
	    /* the filename isn't part of the process address space - EFAULT */
	{
	(char *)-1, EFAULT},
#endif
	    /* the filename does not have execute permission - EACCES */
	{
	test_name5, EACCES},
	    /* the file is zero length with execute permissions - ENOEXEC */
	{
	test_name6, ENOEXEC}
};

int TST_TOTAL = ARRAY_SIZE(TC);

int main(int ac, char **av)
{
	int lc;
	int i;

	tst_parse_opts(ac, av, NULL, NULL);

	setup();

	for (lc = 0; TEST_LOOPING(lc); lc++) {

		tst_count = 0;

		for (i = 0; i < TST_TOTAL; i++) {

			TEST(execve(TC[i].tname, av, NULL));

			if (TEST_RETURN != -1) {
				tst_resm(TFAIL, "call succeeded unexpectedly");
				continue;
			}

			if (TEST_ERRNO == TC[i].error)
				tst_resm(TPASS | TTERRNO,
					 "execve failed as expected");
			else
				tst_resm(TFAIL | TTERRNO,
					 "execve failed unexpectedly; expected "
					 "%d - %s",
					 TC[i].error, strerror(TC[i].error));
		}
	}
	cleanup();

	tst_exit();
}

void setup(void)
{
	char *cwdname = NULL;
	int fd;

	tst_require_root();

	umask(0);

	tst_sig(NOFORK, DEF_HANDLER, cleanup);

	TEST_PAUSE;

	ltpuser = SAFE_GETPWNAM(NULL, nobody_uid);

	SAFE_SETGID(NULL, ltpuser->pw_gid);

	tst_tmpdir();

	cwdname = SAFE_GETCWD(cleanup, cwdname, 0);

	sprintf(test_name5, "%s/fake.%d", cwdname, getpid());

	fileHandle = SAFE_CREAT(cleanup, test_name5, 0444);

	sprintf(test_name3, "%s/fake.%d", test_name5, getpid());

	/* creat() and close a zero length file with executeable permission */
	sprintf(test_name6, "%s/execve03.%d", cwdname, getpid());

	fd = SAFE_CREAT(cleanup, test_name6, 0755);
	fd = SAFE_CLOSE(cleanup, fd);
#ifndef UCLINUX
	bad_addr = SAFE_MMAP(cleanup, NULL, 1, PROT_NONE,
			     MAP_PRIVATE_EXCEPT_UCLINUX | MAP_ANONYMOUS, 0, 0);
	TC[3].tname = bad_addr;
#endif
}

void cleanup(void)
{
#ifndef UCLINUX
	SAFE_MUNMAP(NULL, bad_addr, 1);
#endif
	SAFE_CLOSE(NULL, fileHandle);

	tst_rmdir();

}

/*
 *
 *   Copyright (c) International Business Machines  Corp., 2002
 *   Copyright (c) Cyril Hrubis chrubis@suse.cz 2009
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
 *	ftest06.c -- test inode things (ported from SPIE section2/filesuite/ftest7.c, by Airong Zhang)
 *
 * 	this is the same as ftest2, except that it uses lseek64
 *
 * CALLS
 *	open, close,  read, write, lseek64,
 *	unlink, chdir
 *
 *
 * ALGORITHM
 *
 *	This was tino.c by rbk.  Moved to test suites by dale.
 *
 *	ftest06 [-f tmpdirname] nchild iterations [partition]
 *
 *	This forks some child processes, they do some random operations
 *	which use lots of directory operations.
 *
 * RESTRICTIONS
 *	Runs a long time with default args - can take others on input
 *	line.  Use with "term mode".
 *	If run on vax the ftruncate will not be random - will always go to
 *	start of file.  NOTE: produces a very high load average!!
 *
 */

#define _LARGEFILE64_SOURCE 1
#include <stdio.h>
#include <sys/types.h>
#include <sys/param.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/mount.h>
#include <sys/wait.h>
#include <errno.h>
#include <signal.h>
#include <unistd.h>
#include "test.h"
#include "libftest.h"

char *TCID = "ftest06";
int TST_TOTAL = 1;

#define PASSED 1
#define FAILED 0

static void crfile(int, int);
static void unlfile(int, int);
static void fussdir(int, int);
static void dotest(int, int);
static void dowarn(int, char *, char *);
static void term(int sig);
static void cleanup(void);

#define MAXCHILD	25
#define K_1		1024
#define K_2		2048
#define K_4		4096

static int local_flag;

#define M       (1024*1024)

static int iterations;
static int nchild;
static int parent_pid;
static int pidlist[MAXCHILD];

static char homedir[MAXPATHLEN];
static char dirname[MAXPATHLEN];
static int dirlen;
static int mnt = 0;
static char startdir[MAXPATHLEN], mntpoint[MAXPATHLEN];
static char *partition;
static char *cwd;
static char *fstyp;

int main(int ac, char *av[])
{
	int pid, child, status, count, k, j;
	char name[MAXPATHLEN];

	int lc;

	/*
	 * parse standard options
	 */
	tst_parse_opts(ac, av, NULL, NULL);

	/*
	 * Default values for run conditions.
	 */
	iterations = 50;
	nchild = 5;

	if (signal(SIGTERM, term) == SIG_ERR) {
		tst_resm(TBROK, "first signal failed");

	}

	/* use the default values for run conditions */
	for (lc = 0; TEST_LOOPING(lc); lc++) {

		local_flag = PASSED;
		/*
		 * Make a directory to do this in; ignore error if already exists.
		 */
		parent_pid = getpid();
		tst_tmpdir();

		if (!startdir[0]) {
			if (getcwd(startdir, MAXPATHLEN) == NULL) {
				tst_brkm(TFAIL | TERRNO, NULL, "getcwd failed");
			}
		}
		cwd = startdir;

		snprintf(dirname, ARRAY_SIZE(dirname),
			 "%s/ftest06.%d", cwd, getpid());
		snprintf(homedir, ARRAY_SIZE(homedir),
			 "%s/ftest06h.%d", cwd, getpid());

		mkdir(dirname, 0755);
		mkdir(homedir, 0755);

		if (chdir(dirname) < 0)
			tst_brkm(TFAIL | TERRNO, cleanup, "\tCan't chdir(%s)",
				 dirname);

		dirlen = strlen(dirname);

		if (chdir(homedir) < 0)
			tst_brkm(TFAIL | TERRNO, cleanup, "\tCan't chdir(%s)",
				 homedir);

		/* enter block */
		for (k = 0; k < nchild; k++) {
			if ((child = fork()) == 0) {
				dotest(k, iterations);
				tst_exit();
			}
			if (child < 0) {
				tst_brkm(TBROK | TERRNO, cleanup,
					 "fork failed");
			}
			pidlist[k] = child;
		}

		/*
		 * Wait for children to finish.
		 */
		count = 0;
		while ((child = wait(&status)) > 0) {
			//tst_resm(TINFO,"Test{%d} exited status = 0x%x", child, status);
			//fprintf(stdout, "status is %d",status);
			if (status) {
				tst_resm(TFAIL,
					 "Test{%d} failed, expected 0 exit.",
					 child);
				local_flag = FAILED;
			}
			++count;
		}

		/*
		 * Should have collected all children.
		 */
		if (count != nchild) {
			tst_resm(TFAIL,
				 "Wrong # children waited on, count = %d",
				 count);
			local_flag = FAILED;
		}

		if (local_flag == PASSED)
			tst_resm(TPASS, "Test passed.");
		else
			tst_resm(TFAIL, "Test failed.");

		if (iterations > 26)
			iterations = 26;

		for (k = 0; k < nchild; k++)
			for (j = 0; j < iterations + 1; j++) {
				ft_mkname(name, dirname, k, j);
				rmdir(name);
				unlink(name);
			}

		if (chdir(startdir) < 0)
			tst_brkm(TFAIL | TERRNO, cleanup, "Can't chdir(%s)",
				 startdir);

		pid = fork();
		if (pid < 0) {
			tst_brkm(TBROK | TERRNO, NULL, "fork failed");
		}

		if (pid == 0) {
			execl("/bin/rm", "rm", "-rf", homedir, NULL);

		} else
			wait(&status);

		if (status)
			tst_resm(TINFO,
				 "CAUTION - ftest06, '%s' may not have been removed.",
				 homedir);

		pid = fork();
		if (pid < 0) {
			tst_brkm(TBROK | TERRNO, NULL, "fork failed");
		}
		if (pid == 0) {
			execl("/bin/rm", "rm", "-rf", dirname, NULL);
			exit(1);
		} else
			wait(&status);
		if (status) {
			tst_resm(TWARN,
				 "CAUTION - ftest06, '%s' may not have been removed.",
				 dirname);
		}

		sync();

	}

	if (local_flag == FAILED)
		tst_resm(TFAIL, "Test failed.");
	else
		tst_resm(TPASS, "Test passed.");

	cleanup();
	tst_exit();
}

#define	warn(val,m1,m2)	if ((val) < 0) dowarn(me,m1,m2)

/*
 * crfile()
 *	Create a file and write something into it.
 */
static char crmsg[] = "Gee, let's write something in the file!\n";

static void crfile(int me, int count)
{
	int fd;
	off64_t seekval;
	int val;
	char fname[MAXPATHLEN];
	char buf[MAXPATHLEN];

	ft_mkname(fname, dirname, me, count);

	fd = open(fname, O_RDWR | O_CREAT | O_TRUNC, 0666);
	if (fd < 0 && errno == EISDIR) {
		val = rmdir(fname);
		warn(val, "rmdir", fname);
		fd = open(fname, O_RDWR | O_CREAT | O_TRUNC, 0666);
	}
	warn(fd, "creating", fname);

	seekval = lseek64(fd, (off64_t) (rand() % M), 0);
	warn(seekval, "lseek64", 0);

	val = write(fd, crmsg, sizeof(crmsg) - 1);
	warn(val, "write", 0);

	seekval = lseek(fd, -((off64_t) sizeof(crmsg) - 1), 1);
	warn(seekval, "lseek64", 0);

	val = read(fd, buf, sizeof(crmsg) - 1);
	warn(val, "read", 0);

	if (strncmp(crmsg, buf, sizeof(crmsg) - 1))
		dowarn(me, "compare", 0);

	val = close(fd);
	warn(val, "close", 0);
}

/*
 * unlfile()
 *	Unlink some of the files.
 */
static void unlfile(int me, int count)
{
	int val, i;
	char fname[MAXPATHLEN];

	i = count - 10;
	if (i < 0)
		i = 0;
	for (; i < count; i++) {
		ft_mkname(fname, dirname, me, i);
		val = rmdir(fname);
		if (val < 0)
			val = unlink(fname);
		if (val == 0 || errno == ENOENT)
			continue;
		dowarn(me, "unlink", fname);
	}
}

/*
 * fussdir()
 *	Make a directory, put stuff in it, remove it, and remove directory.
 *
 * Randomly leave the directory there.
 */
static void fussdir(int me, int count)
{
	int val;
	char dir[MAXPATHLEN], fname[MAXPATHLEN], savedir[MAXPATHLEN];

	ft_mkname(dir, dirname, me, count);
	rmdir(dir);
	unlink(dir);

	val = mkdir(dir, 0755);
	warn(val, "mkdir", dir);

	/*
	 * Arrange to create files in the directory.
	 */
	strcpy(savedir, dirname);
	strcpy(dirname, "");

	val = chdir(dir);
	warn(val, "chdir", dir);

	crfile(me, count);
	crfile(me, count + 1);

	val = chdir("..");
	warn(val, "chdir", "..");

	val = rmdir(dir);

	if (val >= 0) {
		tst_brkm(TFAIL, NULL,
			 "Test[%d]: rmdir of non-empty %s succeeds!", me,
			 dir);
	}

	val = chdir(dir);
	warn(val, "chdir", dir);

	ft_mkname(fname, dirname, me, count);
	val = unlink(fname);
	warn(val, "unlink", fname);

	ft_mkname(fname, dirname, me, count + 1);
	val = unlink(fname);
	warn(val, "unlink", fname);

	val = chdir(homedir);
	warn(val, "chdir", homedir);

	if (rand() & 0x01) {
		val = rmdir(dir);
		warn(val, "rmdir", dir);
	}

	strcpy(dirname, savedir);
}

/*
 * dotest()
 *	Children execute this.
 *
 * Randomly do an inode thing; loop for # iterations.
 */
#define	THING(p)	{p, "p"}

struct ino_thing {
	void (*it_proc) ();
	char *it_name;
} ino_thing[] = {
THING(crfile), THING(unlfile), THING(fussdir), THING(sync),};

#define	NTHING	ARRAY_SIZE(ino_thing)

int thing_cnt[NTHING];
int thing_last[NTHING];

static void dotest(int me, int count)
{
	int thing, i;

	//tst_resm(TINFO,"Test %d pid %d starting.", me, getpid());

	srand(getpid());

	for (i = 0; i < count; i++) {
		thing = (rand() >> 3) % NTHING;
		(*ino_thing[thing].it_proc) (me, i, ino_thing[thing].it_name);
		++thing_cnt[thing];
	}

	//tst_resm(TINFO,"Test %d pid %d exiting.", me, getpid());
}

static void dowarn(int me, char *m1, char *m2)
{
	int err = errno;

	tst_brkm(TFAIL, NULL, "Test[%d]: error %d on %s %s",
		 me, err, m1, (m2 ? m2 : ""));
}

static void term(int sig LTP_ATTRIBUTE_UNUSED)
{
	int i;

	tst_resm(TINFO, "\tterm -[%d]- got sig term.", getpid());

	if (parent_pid == getpid()) {
		for (i = 0; i < nchild; i++)
			if (pidlist[i])
				kill(pidlist[i], SIGTERM);
		return;
	}

	tst_brkm(TBROK, NULL, "Term: Child process exiting.");
}

static void cleanup(void)
{
	char mount_buffer[1024];

	if (mnt == 1) {
		if (chdir(startdir) < 0) {
			tst_resm(TINFO, "Could not change to %s ", startdir);
		}
		if (!strcmp(fstyp, "cfs")) {
			sprintf(mount_buffer, "/bin/umount %s", partition);
			if (system(mount_buffer) != 0) {
				tst_resm(TINFO, "Unable to unmount %s from %s ",
					 partition, mntpoint);
				if (umount(partition)) {
					tst_resm(TINFO,
						 "Unable to unmount %s from %s ",
						 partition, mntpoint);
				} else {
					tst_resm(TINFO,
						 "Forced umount for %s, /etc/mtab now dirty",
						 partition);
				}
			}
		} else {
			if (umount(partition)) {
				tst_resm(TINFO, "Unable to unmount %s from %s ",
					 partition, mntpoint);
			}
		}
		if (rmdir(mntpoint) != 0) {
			tst_resm(TINFO, "Unable to rmdir %s ", mntpoint);
		}
	}
	tst_rmdir();

}

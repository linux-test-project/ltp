/*
 *  Copyright (c) International Business Machines  Corp., 2002
 *
 *  This program is free software;  you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY;  without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See
 *  the GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program;  if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 */

/*
 * NAME
 *	ftrunc04.c - test truncate and mandatory record locking
 *		    (ported from SPIE, section2/filesuite/ftrunc2.c and
 *		     ftrunc3.c, by Airong Zhang)
 *		   - Modified to use fcntl() instead of lockf,
 *		     Robbie Williamson <robbiew@us.ibm.com>
 *		   - Fix concurrency issue
 *		     Roy Lee <roylee@andestech.com>
 *
 * CALLS
 *	truncate(2), ftruncate(2), fcntl(2)
 *
 * Algorithm
 *	Iterate for the requested number of times:
 *
 *	Parent creats a a file  with mandatory locking modes.
 *	parent calculates a position to place a record lock.
 *	parent forks child, and waits for child's signal
 *	child opens file and asserts lock.
 *	child signals parent that lock is asserted.
 *	child pauses forever (until killed by parent)
 *	parent tries to truncate after the end of the lock
 *		(should succeed),
 *	parent tries to truncate in the locked region (should fail)
 *	parent tries to truncate ahead of the locked region (fails)
 *	parent kills child and waits for it to exit
 *		this releases the lock held by the child.
 *	parent re-tries the failed cases above.  They should now
 *		succeed.
 *
 * USAGE
 *	ftrunc04 -i 5 -l 8192
 *		-i	number of iterations.
 *		-l	length of file to create
 *
 * Restrictions:
 *	The filesystem containing /tmp MUST have "mand" specified as
 *	a mount option.  This option allows the use of mandatory locks. 
 */

#include <signal.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <errno.h>
#include <wait.h>
#include <inttypes.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mount.h>
#include <sys/statvfs.h>
#include "test.h"
#include "usctest.h"
#include "libtestsuite.h"

char progname[] = "ftruncate04()";

TCID_DEFINE(ftruncate04);
int TST_TOTAL = 1;
extern int Tst_count;

int sync_pipes[2];
int len = 8 * 1024;
int iterations = 5;
char filename[80];
int recstart;
#define RECLEN	100
#define PASSED 1
#define FAILED 0

int reclen;
int cpid;
int ppid;
int usrcnt;
sigset_t set;

#define BUFSIZE (8*1024)
char buffer[BUFSIZE];

extern char *optarg;
extern int optind, opterr;
int local_flag;

void usr1hndlr()
{
	usrcnt++;
}

void cleanup()
{
	kill(cpid, SIGKILL);
	unlink(filename);
	tst_rmdir();
	tst_exit();
}

void doparent()
{
	int fd;
	struct stat sb;

	sigemptyset(&set);
	sigsuspend(&set);
	if ((fd = open(filename, O_RDWR | O_NONBLOCK)) < 0) {
		tst_resm(TBROK, "parent open1 failed");
		cleanup();
	}			/* end if */
	lseek(fd, 0, SEEK_SET);

	/* first delete BEFORE lock, expect failure */
	if (ftruncate(fd, RECLEN) >= 0) {
		tst_resm(TFAIL, "unexpected ftruncate success case 1");
		local_flag = FAILED;
		cleanup();
	}
	if (errno != EAGAIN) {
		tst_resm(TFAIL, "bad ftruncate errno case 1");
		local_flag = FAILED;
		cleanup();
	}

	/* delete IN the lock, expect failure */
	if (ftruncate(fd, recstart + (RECLEN / 2)) >= 0) {
		tst_resm(TFAIL, "unexpected ftruncate success case 2");
		local_flag = FAILED;
		cleanup();
	}
	if (errno != EAGAIN) {
		tst_resm(TFAIL, "bad ftruncate errno case 2");
		local_flag = FAILED;
		cleanup();
	}

	/* delete AFTER lock, expect success */
	if (ftruncate(fd, recstart + RECLEN) != 0) {
		tst_resm(TFAIL, "unexpected ftruncate success case 3");
		local_flag = FAILED;
		cleanup();
	}
	if (errno != EAGAIN) {
		tst_resm(TFAIL, "bad ftruncate errno case 3");
		local_flag = FAILED;
		cleanup();
	}

	/* kill off child, freeing record lock */
	if (kill(cpid, SIGKILL) < 0) {
		tst_resm(TFAIL, "kill child");
		local_flag = FAILED;
		cleanup();
	}
	wait(0);

	/* truncate IN record lock */
	if (truncate(filename, recstart + (RECLEN / 2)) < 0) {
		tst_resm(TFAIL, "truncate failure case 4");
		local_flag = FAILED;
		cleanup();
	}
	if (ftruncate(fd, recstart + (RECLEN / 2)) < 0) {
		tst_resm(TFAIL, "truncate failure case 4");
		local_flag = FAILED;
		cleanup();
	}

	if (fstat(fd, &sb) < 0) {
		tst_resm(TFAIL, "fstat failure, case 4");
		local_flag = FAILED;
		cleanup();
	}
	if (sb.st_size != recstart + (RECLEN / 2)) {
		tst_resm(TFAIL, "unexpected ftruncate failure case 4");
		tst_resm(TFAIL, "expected size of %d, got size of %"PRId64,
			 recstart + (RECLEN / 2), (int64_t)sb.st_size);
		local_flag = FAILED;
		cleanup();
	}

	/* truncate BEFORE record lock */
	if (truncate(filename, RECLEN) < 0) {
		tst_resm(TFAIL, "truncate failure case 5");
		local_flag = FAILED;
		cleanup();
	}
	if (ftruncate(fd, RECLEN) < 0) {
		tst_resm(TFAIL, "truncate failure case 5");
		local_flag = FAILED;
		cleanup();
	}
	if (fstat(fd, &sb) < 0) {
		tst_resm(TFAIL, "fstat failure, case 5");
		local_flag = FAILED;
		cleanup();
	}
	if (sb.st_size != RECLEN) {
		tst_resm(TFAIL, "unexpected ftruncate failure case 5");
		tst_resm(TFAIL, "expected size of %d, got size of %"PRId64,
			 RECLEN, (int64_t)sb.st_size);
		local_flag = FAILED;
		cleanup();
	}

	/* truncate AFTER record lock */
	if (ftruncate(fd, (2 * len)) < 0) {
		tst_resm(TFAIL, "truncate failure case 6");
		local_flag = FAILED;
		cleanup();
	}
	if (fstat(fd, &sb) < 0) {
		tst_resm(TFAIL, "fstat failure, case 6");
		local_flag = FAILED;
		cleanup();
	}
	if (sb.st_size != (2 * len)) {
		tst_resm(TFAIL, "unexpected ftruncate failure case 6");
		tst_resm(TFAIL, "expected size of %d, got size of %"PRId64,
			 (2 * len), (int64_t)sb.st_size);
		local_flag = FAILED;
		cleanup();
	}

	close(fd);
}

void dochild()
{
	int fd;
	struct flock flocks;

#ifdef UCLINUX
#define PIPE_NAME	"ftruncate04"
	if (sync_pipe_create(sync_pipes, PIPE_NAME) == -1)
		tst_brkm(TBROK, cleanup, "sync_pipe_create failed");
#endif

	if ((fd = open(filename, O_RDWR)) < 0) {
		tst_resm(TFAIL, "child open");
		tst_exit();
	}
	lseek(fd, 0, SEEK_SET);
	flocks.l_type = F_WRLCK;
	flocks.l_whence = SEEK_CUR;
	flocks.l_start = recstart;
	flocks.l_len = reclen;
	if (fcntl(fd, F_SETLKW, &flocks) < 0) {
		tst_resm(TFAIL, "child fcntl failed");
		tst_exit();
	}

	if (kill(ppid, SIGUSR1) < 0) {
		tst_resm(TFAIL, "child kill");
		tst_exit();
	}

	if (sync_pipe_notify(sync_pipes) == -1)
		tst_brkm(TBROK, cleanup, "sync_pipe_notify failed");

	if (sync_pipe_close(sync_pipes, PIPE_NAME) == -1)
		tst_brkm(TBROK, cleanup, "sync_pipe_close failed");
	pause();
	tst_exit();
}

int main(int ac, char **av)
{
	int fd, i;
	int tlen = 0;
	struct sigaction act;
	int lc;			/* loop counter */
	char *msg;		/* message returned from parse_opts */
	struct statvfs fs;

	/*
	 * parse standard options
	 */
	if ((msg = parse_opts(ac, av, (option_t *) NULL, NULL)) != (char *)NULL) {
		tst_resm(TBROK, "OPTION PARSING ERROR - %s", msg);
		tst_exit();
		/* NOTREACHED */
	}
#ifdef UCLINUX
	maybe_run_child(&dochild, "dddd", filename, &recstart, &reclen, &ppid);
#endif

	local_flag = PASSED;
	tst_tmpdir();
	if (statvfs(".", &fs) == -1) {
		tst_resm(TFAIL|TERRNO, "statvfs failed");
		tst_rmdir();
		tst_exit();
	}
	if ((fs.f_flag & MS_MANDLOCK) == 0) {
		tst_resm(TCONF,
			 "The filesystem where /tmp is mounted does"
			 " not support mandatory locks. Cannot run this test.");
		tst_rmdir();
		tst_exit();
		/* NOTREACHED */
	}
	for (lc = 0; TEST_LOOPING(lc); lc++) {
		setvbuf(stdin, 0, _IOLBF, BUFSIZ);
		setvbuf(stdout, 0, _IOLBF, BUFSIZ);
		setvbuf(stderr, 0, _IOLBF, BUFSIZ);
		ppid = getpid();
		srand(ppid);
		sigemptyset(&set);
		act.sa_handler = (void (*)())usr1hndlr;
		act.sa_mask = set;
		act.sa_flags = 0;
		if (sigaction(SIGUSR1, &act, 0)) {
			tst_resm(TBROK, "Sigaction for SIGUSR1 failed");
			tst_rmdir();
			tst_exit();
		}		/* end if */
		if (sigaddset(&set, SIGUSR1)) {
			tst_resm(TBROK, "sigaddset for SIGUSR1 failed");
			tst_rmdir();
			tst_exit();
		}
		if (sigprocmask(SIG_SETMASK, &set, 0)) {
			tst_resm(TBROK, "sigprocmask for SIGUSR1 failed");
			tst_rmdir();
			tst_exit();
		}
		for (i = 0; i < iterations; i++) {
			sprintf(filename, "%s.%d.%d\n", progname, ppid, i);
			if ((fd = open(filename, O_CREAT | O_RDWR, 02666)) < 0) {
				tst_resm(TBROK,
					 "parent error opening/creating %s",
					 filename);
				cleanup();
			}	/* end if */
			if (chown(filename, geteuid(), getegid()) == -1) {
				tst_resm(TBROK, "parent error chowning %s",
					 filename);
				cleanup();
			}	/* end if */
			if (chmod(filename, 02666) == -1) {
				tst_resm(TBROK, "parent error chmoding %s",
					 filename);
				cleanup();
			}	/* end if */
			do {
				if (write(fd, buffer, BUFSIZE) < 0) {
					tst_resm(TBROK,
						 "parent write failed to %s",
						 filename);
					cleanup();
				}
				tlen += BUFSIZE;
			} while (tlen < len);
			close(fd);
			reclen = RECLEN;
			/*
			 * want at least RECLEN bytes BEFORE AND AFTER the
			 * record lock.
			 */
			recstart = RECLEN + rand() % (len - 3 * RECLEN);

			if (sync_pipe_create(sync_pipes, PIPE_NAME) == -1)
				tst_brkm(TBROK, cleanup,
					 "sync_pipe_create failed");

			if ((cpid = FORK_OR_VFORK()) < 0) {
				unlink(filename);
				tst_resm(TINFO,
					 "System resource may be too low, fork() malloc()"
					 " etc are likely to fail.");
				tst_resm(TBROK,
					 "Test broken due to inability of fork.");
				tst_rmdir();
				tst_exit();
			}

			if (cpid == 0) {
#ifdef UCLINUX
				if (self_exec
				    (av[0], "dddd", filename, recstart, reclen,
				     ppid) < -1) {
					unlink(filename);
					tst_resm(TBROK, "self_exec failed.");
					tst_rmdir();
					tst_exit();
				}
#else
				dochild();
#endif
				/* never returns */
			}

			if (sync_pipe_wait(sync_pipes) == -1)
				tst_brkm(TBROK, cleanup,
					 "sync_pipe_wait failed");

			if (sync_pipe_close(sync_pipes, PIPE_NAME) == -1)
				tst_brkm(TBROK, cleanup,
					 "sync_pipe_close failed");

			doparent();
			/* child should already be dead */
			unlink(filename);
		}
		if (local_flag == PASSED)
			tst_resm(TPASS, "Test passed.");
		else
			tst_resm(TFAIL, "Test failed.");

		tst_rmdir();
		tst_exit();
	}			/* end for */
	return 0;
}

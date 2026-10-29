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
 *	ftest08.c -- test single file io (tsfio.c by rbk) (ported from SPIE,
 *		     section2/filesuite/ftest10.c, by Airong Zhang)
 *
 * 	this is the same as ftest4, except that it uses lseek64
 *
 * CALLS
 *	fsync, sync, lseek64, read, write
 *
 *
 * ALGORITHM
 *	Several child processes doing random seeks, read/write
 *	operations on the same file.
 *
 *
 * RESTRICTIONS
 *	Runs a long time with default args - can take others on input
 *	line.  Use with "term mode".
 *
 */

#define _XOPEN_SOURCE 500
#define _LARGEFILE64_SOURCE 1
#include <stdio.h>
#include <sys/types.h>
#include <sys/param.h>
#include <sys/wait.h>
#include <sys/file.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/uio.h>
#include <errno.h>
#include <signal.h>
#include <unistd.h>
#include <inttypes.h>
#include "test.h"
#include "tso_safe_macros.h"
#include "libftest.h"

char *TCID = "ftest08";
int TST_TOTAL = 1;

#define PASSED 1
#define FAILED 0

#define MAXCHILD	25
#define K_1		1024
#define K_2		2048
#define K_4		4096
#define	MAXIOVCNT	16

static void init(void);
static void runtest(void);
static void dotest(int, int, int);
static void domisc(int, int);
static void term(int sig);
static void cleanup(void);

static int csize;		/* chunk size */
static int iterations;		/* # total iterations */
static off64_t max_size;	/* max file size */
static int misc_intvl;		/* for doing misc things; 0 ==> no */
static int nchild;		/* number of child processes */
static int parent_pid;
static int pidlist[MAXCHILD];

static char filename[MAXPATHLEN];
static int local_flag;

int main(int ac, char *av[])
{
	int lc;

	tst_parse_opts(ac, av, NULL, NULL);

	for (lc = 0; TEST_LOOPING(lc); lc++) {

		local_flag = PASSED;
		init();
		runtest();

		if (local_flag == PASSED)
			tst_resm(TPASS, "Test passed.");
		else
			tst_resm(TFAIL, "Test failed.");
	}

	cleanup();
	tst_exit();
}

static void init(void)
{
	int fd;
	char wdbuf[MAXPATHLEN];

	parent_pid = getpid();
	tst_tmpdir();

	/*
	 * Make a filename for the test.
	 */
	if (!filename[0])
		sprintf(filename, "%s/ftest08.%d", getcwd(wdbuf, MAXPATHLEN),
			getpid());

	fd = SAFE_OPEN(NULL, filename, O_RDWR | O_CREAT | O_TRUNC, 0666);

	close(fd);

	/*
	 * Default values for run conditions.
	 */
	iterations = 10;
	nchild = 5;
	csize = K_2;		/* should run with 1, 2, and 4 K sizes */
	max_size = K_1 * K_1;
	misc_intvl = 10;

	if (sigset(SIGTERM, term) == SIG_ERR) {
		tst_brkm(TBROK | TERRNO, NULL, "first sigset failed");
	}

}

static void runtest(void)
{
	int child, count, fd, i, nwait, status;

	nwait = 0;

	for (i = 0; i < nchild; i++) {

		if ((child = fork()) == 0) {
			fd = open(filename, O_RDWR);
			if (fd < 0) {
				tst_brkm(TFAIL,
					 NULL,
					 "\tTest[%d]: error %d openning %s.",
					 i,
					 errno, filename);
			}
			dotest(nchild, i, fd);
			close(fd);
			tst_exit();
		}

		if (child < 0) {
			tst_brkm(TBROK | TERRNO, NULL, "fork failed");
		} else {
			pidlist[i] = child;
			nwait++;
		}
	}

	/*
	 * Wait for children to finish.
	 */
	count = 0;
	while ((child = wait(&status)) != -1 || errno == EINTR) {
		if (child > 0) {
			//tst_resm(TINFO, "\tTest{%d} exited status = 0x%x", child, status);
			if (status) {
				tst_resm(TFAIL,
					 "\tExpected 0 exit status - failed.");
				local_flag = FAILED;
			}
			++count;
		}
	}

	/*
	 * Should have collected all children.
	 */
	if (count != nwait) {
		tst_resm(TFAIL, "\tWrong # children waited on, count = %d",
			 count);
		local_flag = FAILED;
	}

	unlink(filename);
	sync();
}

/*
 * dotest()
 *	Children execute this.
 *
 * Randomly read/mod/write chunks with known pattern and check.
 * When fill sectors, iterate.
 */
#define	NMISC	2
enum m_type { m_fsync, m_sync };
char *m_str[] = { "fsync", "sync" };

int misc_cnt[NMISC];		/* counts # of each kind of misc */
int misc_flag;
int nchunks;

#define	CHUNK(i)	((((off64_t)i) * testers + me) * csize)
#define	NEXTMISC	((rand() % misc_intvl) + 5)

static void dotest(int testers, int me, int fd)
{
	char *bits;
	char val, val0;
	int count, collide, chunk, whenmisc, xfr, i;

	/* Stuff for the readv call */
	struct iovec r_iovec[MAXIOVCNT];
	int r_ioveclen;

	/* Stuff for the writev call */
	struct iovec val0_iovec[MAXIOVCNT];
	struct iovec val_iovec[MAXIOVCNT];
	int w_ioveclen;
	struct stat stat;

	nchunks = max_size / (testers * csize);
	whenmisc = 0;

	if ((bits = malloc((nchunks + 7) / 8)) == NULL) {
		tst_brkm(TBROK, NULL, "\tmalloc failed(bits)");
	}

	/* Allocate memory for the iovec buffers and init the iovec arrays */
	r_ioveclen = w_ioveclen = csize / MAXIOVCNT;

	/* Please note that the above statement implies that csize
	 * be evenly divisible by MAXIOVCNT.
	 */
	for (i = 0; i < MAXIOVCNT; i++) {
		if ((r_iovec[i].iov_base = malloc(r_ioveclen)) == NULL) {
			tst_brkm(TBROK, NULL, "\tmalloc failed(iov_base)");
		}
		r_iovec[i].iov_len = r_ioveclen;

		/* Allocate unused memory areas between all the buffers to
		 * make things more diffult for the OS.
		 */
		if (malloc((i + 1) * 8) == NULL) {
			tst_brkm(TBROK, NULL, "\tmalloc failed((i+1)*8)");
		}

		if ((val0_iovec[i].iov_base = malloc(w_ioveclen)) == NULL) {
			tst_brkm(TBROK, NULL, "\tmalloc failed(val0_iovec)");
		}

		val0_iovec[i].iov_len = w_ioveclen;

		if (malloc((i + 1) * 8) == NULL) {
			tst_brkm(TBROK, NULL, "\tmalloc failed((i+1)*8)");
		}

		if ((val_iovec[i].iov_base = malloc(w_ioveclen)) == NULL) {
			tst_brkm(TBROK, NULL, "\tmalloc failed(iov_base)");
		}
		val_iovec[i].iov_len = w_ioveclen;

		if (malloc((i + 1) * 8) == NULL) {
			tst_brkm(TBROK, NULL, "\tmalloc failed(((i+1)*8)");
		}
	}

	/*
	 * No init sectors; file-sys makes 0 to start.
	 */
	val = (64 / testers) * me + 1;
	val0 = 0;

	/*
	 * For each iteration:
	 *      zap bits array
	 *      loop:
	 *              pick random chunk, read it.
	 *              if corresponding bit off {
	 *                      verify == 0. (sparse file)
	 *                      ++count;
	 *              } else
	 *                      verify == val.
	 *              write "val" on it.
	 *              repeat until count = nchunks.
	 *      ++val.
	 */
	srand(getpid());

	if (misc_intvl)
		whenmisc = NEXTMISC;

	while (iterations-- > 0) {
		for (i = 0; i < NMISC; i++)
			misc_cnt[i] = 0;
		memset(bits, 0, (nchunks + 7) / 8);
		/* Have to fill the val0 and val iov buffers in a different manner
		 */
		for (i = 0; i < MAXIOVCNT; i++) {
			memset(val0_iovec[i].iov_base, val0,
			       val0_iovec[i].iov_len);
			memset(val_iovec[i].iov_base, val,
			       val_iovec[i].iov_len);

		}

		count = 0;
		collide = 0;

		while (count < nchunks) {
			chunk = rand() % nchunks;
			/*
			 * Read it.
			 */
			if (lseek64(fd, CHUNK(chunk), 0) < 0) {
				tst_brkm(TFAIL,
					 NULL, "\tTest[%d]: lseek64(0) fail at %"
					 PRIx64 "x, errno = %d.", me,
					 CHUNK(chunk), errno);
			}
			if ((xfr = readv(fd, &r_iovec[0], MAXIOVCNT)) < 0) {
				tst_brkm(TFAIL,
					 NULL, "\tTest[%d]: readv fail at %" PRIx64
					 "x, errno = %d.", me, CHUNK(chunk),
					 errno);
			}
			/*
			 * If chunk beyond EOF just write on it.
			 * Else if bit off, haven't seen it yet.
			 * Else, have.  Verify values.
			 */
			if (xfr == 0) {
				bits[chunk / 8] |= (1 << (chunk % 8));
			} else if ((bits[chunk / 8] & (1 << (chunk % 8))) == 0) {
				if (xfr != csize) {
					tst_brkm(TFAIL,
						 NULL,
						 "\tTest[%d]: xfr=%d != %d, zero read.",
						 me, xfr, csize);
				}
				for (i = 0; i < MAXIOVCNT; i++) {
					if (memcmp
					    (r_iovec[i].iov_base,
					     val0_iovec[i].iov_base,
					     r_iovec[i].iov_len)) {
						tst_resm(TFAIL,
							 "\tTest[%d] bad verify @ 0x%"
							 PRIx64
							 " for val %d count %d xfr %d.",
							 me, CHUNK(chunk), val0,
							 count, xfr);
						fstat(fd, &stat);
						tst_resm(TINFO,
							 "\tStat: size=%llx, ino=%x",
							 stat.st_size, (unsigned)stat.st_ino);
						ft_dumpiov(&r_iovec[i]);
						ft_dumpbits(bits,
							    (nchunks + 7) / 8);
						tst_exit();
					}
				}
				bits[chunk / 8] |= (1 << (chunk % 8));
				++count;
			} else {
				if (xfr != csize) {
					tst_brkm(TFAIL,
						 NULL,
						 "\tTest[%d]: xfr=%d != %d, val read.",
						 me, xfr, csize);
				}
				++collide;
				for (i = 0; i < MAXIOVCNT; i++) {
					if (memcmp
					    (r_iovec[i].iov_base,
					     val_iovec[i].iov_base,
					     r_iovec[i].iov_len)) {
						tst_resm(TFAIL,
							 "\tTest[%d] bad verify @ 0x%"
							 PRIx64
							 " for val %d count %d xfr %d.",
							 me, CHUNK(chunk), val,
							 count, xfr);
						fstat(fd, &stat);
						tst_resm(TINFO,
							 "\tStat: size=%llx, ino=%x",
							 stat.st_size, (unsigned)stat.st_ino);
						ft_dumpiov(&r_iovec[i]);
						ft_dumpbits(bits,
							    (nchunks + 7) / 8);
						tst_exit();
					}
				}
			}
			/*
			 * Write it.
			 */
			if (lseek64(fd, -xfr, 1) < 0) {
				tst_brkm(TFAIL,
					 NULL, "\tTest[%d]: lseek64(1) fail at %"
					 PRIx64 ", errno = %d.", me,
					 CHUNK(chunk), errno);
			}
			if ((xfr =
			     writev(fd, &val_iovec[0], MAXIOVCNT)) < csize) {
				if (errno == ENOSPC) {
					tst_resm(TFAIL,
						 "\tTest[%d]: no space, exiting.",
						 me);
					fsync(fd);
					tst_exit();
				}
				tst_brkm(TFAIL,
					 NULL, "\tTest[%d]: writev fail at %" PRIx64
					 "x xfr %d, errno = %d.", me,
					 CHUNK(chunk), xfr, errno);
			}
			/*
			 * If hit "misc" interval, do it.
			 */
			if (misc_intvl && --whenmisc <= 0) {
				domisc(me, fd);
				whenmisc = NEXTMISC;
			}
			if (count + collide > 2 * nchunks)
				break;
		}

		/*
		 * End of iteration, maybe before doing all chunks.
		 */

		if (count < nchunks) {
			//tst_resm(TINFO, "\tTest{%d} val %d stopping @ %d, collide = {%d}.",
			//              me, val, count, collide);
			for (i = 0; i < nchunks; i++) {
				if ((bits[i / 8] & (1 << (i % 8))) == 0) {
					if (lseek64(fd, CHUNK(i), 0) <
					    (off64_t) 0) {
						tst_brkm(TFAIL,
							 NULL, "\tTest[%d]: lseek64 fail at %"
							 PRIx64
							 "x, errno = %d.", me,
							 CHUNK(i), errno);
					}
					if (writev(fd, &val_iovec[0], MAXIOVCNT)
					    != csize) {
						tst_brkm(TFAIL,
							 NULL, "\tTest[%d]: writev fail at %"
							 PRIx64
							 "x, errno = %d.", me,
							 CHUNK(i), errno);
					}
				}
			}
		}

		fsync(fd);
		++misc_cnt[m_fsync];
		//tst_resm(TINFO, "\tTest[%d] val %d done, count = %d, collide = %d.",
		//              me, val, count, collide);
		//for (i = 0; i < NMISC; i++)
		//      tst_resm(TINFO, "\t\tTest[%d]: %d %s's.", me, misc_cnt[i], m_str[i]);
		val0 = val++;
	}
}

/*
 * domisc()
 *	Inject misc syscalls into the thing.
 */
static void domisc(int me, int fd)
{
	enum m_type type;

	if (misc_flag) {
		type = m_fsync;
		misc_flag = 0;
	} else {
		type = m_sync;;
		misc_flag = 1;
	}

	switch (type) {
	case m_fsync:
		if (fsync(fd) < 0) {
			tst_brkm(TFAIL, NULL, "\tTest[%d]: fsync error %d.",
				 me,
				 errno);
		}
		break;
	case m_sync:
		sync();
		break;
	}

	++misc_cnt[type];
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

	tst_exit();
}

void cleanup(void)
{

	tst_rmdir();
}

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
 *	ftest01.c -- test file I/O (ported from SPIE section2, filesuite, by Airong Zhang)
 *
 * CALLS
 *	lseek, read, write
 *	truncate, ftruncate, fsync, sync, fstat
 *
 * ALGORITHM
 *	A bitmap is used to map pieces of a file.
 *      Loop: pick a random piece of the file
 *            if we haven't seen it before make sure it is zero,
 *            write pattern
 *            if we have seen it before make sure correct pattern.
 *
 *      This was originally written by rbk - was program tfio.c
 *	Modified by dale to integrate with test suites.
 *
 * RESTRICTIONS
 *	Runs a long time with default args - can take others on input
 *	line.  Use with "term mode".
 *	If run on vax the ftruncate will not be random - will always go to
 *	start of file.  NOTE: produces a very high load average!!
 *
 * CAUTION!!
 *	If a file is supplied to this program with the "-f" option
 *	it will be removed with a system("rm -rf filename") call.
 *
 */
#define _GNU_SOURCE 1
#include <stdio.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <sys/param.h>
#include <errno.h>
#include <fcntl.h>
#include <signal.h>
#include <unistd.h>
#include <inttypes.h>
#include "test.h"
#include "tso_safe_macros.h"
#include "libftest.h"

char *TCID = "ftest01";
int TST_TOTAL = 1;

static void setup(void);
static void runtest(void);
static void dotest(int, int, int);
static void domisc(int, int, char *);
static void cleanup(void);
static void term(int sig);

#define PASSED 1
#define FAILED 0

#define MAXCHILD	25
#define K_1		1024
#define K_2		2048
#define K_4		4096

static int csize;		/* chunk size */
static int iterations;		/* # total iterations */
static int max_size;		/* max file size */
static const int misc_intvl = 10;		/* for doing misc things; 0 ==> no */
static int nchild;		/* how many children */
static int fd;			/* file descriptor used by child */
static int parent_pid;
static int pidlist[MAXCHILD];
static char test_name[2];

static char fuss[MAXPATHLEN];	/* directory to do this in */
static char homedir[MAXPATHLEN];	/* where we started */

static int local_flag;

int main(int ac, char *av[])
{
	int lc;

	tst_parse_opts(ac, av, NULL, NULL);

	setup();

	for (lc = 0; TEST_LOOPING(lc); lc++) {

		runtest();

		if (local_flag == PASSED)
			tst_resm(TPASS, "Test passed.");
		else
			tst_resm(TFAIL, "Test failed.");
	}

	cleanup();
	tst_exit();

}

static void setup(void)
{

	tst_tmpdir();
	getcwd(homedir, sizeof(homedir));
	parent_pid = getpid();

	if (!fuss[0])
		sprintf(fuss, "./ftest1.%d", getpid());

	mkdir(fuss, 0755);

	SAFE_CHDIR(NULL, fuss);

	/*
	 * Default values for run conditions.
	 */
	iterations = 10;
	nchild = 5;
	csize = K_2;		/* should run with 1, 2, and 4 K sizes */
	max_size = K_1 * K_1;

	if (sigset(SIGTERM, term) == SIG_ERR) {
		tst_brkm(TBROK | TERRNO, NULL, "sigset failed");
	}

	local_flag = PASSED;
}

static void runtest(void)
{
	pid_t pid;
	int i, child, count, nwait, status;

	nwait = 0;

	for (i = 0; i < nchild; i++) {

		test_name[0] = 'a' + i;
		test_name[1] = '\0';
		fd = SAFE_OPEN(NULL, test_name, O_RDWR | O_CREAT | O_TRUNC,
			       0666);

		if ((child = fork()) == 0) {
			dotest(nchild, i, fd);
			exit(0);
		}

		close(fd);

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
	while (1) {
		if ((child = wait(&status)) >= 0) {
			if (status) {
				tst_resm(TFAIL,
					 "Test{%d} failed, expected 0 exit",
					 child);
				local_flag = FAILED;
			}
			++count;
		} else {
			if (errno != EINTR)
				break;
		}
	}

	/*
	 * Should have collected all children.
	 */
	if (count != nwait) {
		tst_resm(TFAIL, "Wrong # children waited on, count = %d",
			 count);
		local_flag = FAILED;
	}

	if (local_flag == PASSED)
		tst_resm(TPASS, "Test passed in fork and wait.");
	else
		tst_resm(TFAIL, "Test failed in fork and wait.");

	chdir(homedir);
	pid = fork();

	if (pid < 0) {
		tst_brkm(TBROK | TERRNO, sync, "fork failed");
		tst_exit();
	}

	if (pid == 0) {
		execl("/bin/rm", "rm", "-rf", fuss, NULL);
		exit(1);
	}

	wait(&status);

	if (status)
		tst_resm(TINFO, "CAUTION - ftest1, '%s' may not be removed",
			 fuss);

	sync();
}

/*
 * dotest()
 *	Children execute this.
 *
 * Randomly read/mod/write chunks with known pattern and check.
 * When fill sectors, iterate.
 */

#define	NMISC	4
enum m_type { m_fsync, m_trunc, m_sync, m_fstat };
char *m_str[] = { "fsync", "trunc", "sync", "fstat" };

int misc_cnt[NMISC];		/* counts # of each kind of misc */
int file_max;			/* file-max size */
int nchunks;
int last_trunc = -1;
int tr_flag;
enum m_type type = m_fsync;

#define	CHUNK(i)	((i) * csize)
#define	NEXTMISC	((rand() % misc_intvl) + 5)

/* XXX (garrcoop): should not be using libltp as it runs forked. */
static void dotest(int testers, int me, int fd)
{
	char *bits, *hold_bits, *buf, *val_buf, *zero_buf;
	char val;
	int count, collide, chunk, whenmisc, xfr, i;
	struct stat stat;

	nchunks = max_size / csize;

	if ((bits = calloc((nchunks + 7) / 8, 1)) == 0) {
		tst_brkm(TBROK,
			 NULL,
			 "Test broken due to inability of malloc(bits).");
	}

	if ((hold_bits = calloc((nchunks + 7) / 8, 1)) == 0) {
		tst_brkm(TBROK,
			 NULL,
			 "Test broken due to inability of malloc(hold_bits).");
	}

	if ((buf = (calloc(csize, 1))) == 0) {
		tst_brkm(TBROK, NULL,
			 "Test broken due to inability of malloc(buf).");
	}

	if ((val_buf = (calloc(csize, 1))) == 0) {
		tst_brkm(TBROK,
			 NULL,
			 "Test broken due to inability of malloc(val_buf).");
	}

	if ((zero_buf = (calloc(csize, 1))) == 0) {
		tst_brkm(TBROK,
			 NULL,
			 "Test broken due to inability of malloc(zero_buf).");
	}

	/*
	 * No init sectors; allow file to be sparse.
	 */
	val = (64 / testers) * me + 1;

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
		ftruncate(fd, 0);
		file_max = 0;
		memset(bits, 0, (nchunks + 7) / 8);
		memset(hold_bits, 0, (nchunks + 7) / 8);
		memset(val_buf, val, csize);
		memset(zero_buf, 0, csize);
		count = 0;
		collide = 0;
		while (count < nchunks) {
			chunk = rand() % nchunks;
			/*
			 * Read it.
			 */
			if (lseek(fd, CHUNK(chunk), 0) < 0) {
				tst_brkm(TFAIL,
					 NULL,
					 "Test[%d]: lseek(0) fail at %x, errno = %d.",
					 me, CHUNK(chunk), errno);
			}
			if ((xfr = read(fd, buf, csize)) < 0) {
				tst_brkm(TFAIL,
					 NULL,
					 "Test[%d]: read fail at %x, errno = %d.",
					 me, CHUNK(chunk), errno);
			}
			/*
			 * If chunk beyond EOF just write on it.
			 * Else if bit off, haven't seen it yet.
			 * Else, have.  Verify values.
			 */
			if (CHUNK(chunk) >= file_max) {
				bits[chunk / 8] |= (1 << (chunk % 8));
				++count;
			} else if ((bits[chunk / 8] & (1 << (chunk % 8))) == 0) {
				if (xfr != csize) {
					tst_brkm(TFAIL,
						 NULL,
						 "Test[%d]: xfr=%d != %d, zero read.",
						 me, xfr, csize);
				}
				if (memcmp(buf, zero_buf, csize)) {
					tst_resm(TFAIL,
						 "Test[%d] bad verify @ 0x%x for val %d "
						 "count %d xfr %d file_max 0x%x, should be %d.",
						 me, CHUNK(chunk), val, count,
						 xfr, file_max, zero_buf[0]);
					tst_resm(TINFO,
						 "Test[%d]: last_trunc = 0x%x",
						 me, last_trunc);
					fstat(fd, &stat);
					tst_resm(TINFO,
						 "\tStat: size=%llx, ino=%x",
						 stat.st_size, (unsigned)stat.st_ino);
					sync();
					ft_dumpbuf(buf, csize);
					ft_dumpbits(bits, (nchunks + 7) / 8);
					ft_orbits(hold_bits, bits,
						  (nchunks + 7) / 8);
					tst_resm(TINFO, "Hold ");
					ft_dumpbits(hold_bits,
						    (nchunks + 7) / 8);
					tst_exit();
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
				if (memcmp(buf, val_buf, csize)) {
					tst_resm(TFAIL,
						 "Test[%d] bad verify @ 0x%x for val %d "
						 "count %d xfr %d file_max 0x%x.",
						 me, CHUNK(chunk), val, count,
						 xfr, file_max);
					tst_resm(TINFO,
						 "Test[%d]: last_trunc = 0x%x",
						 me, last_trunc);
					fstat(fd, &stat);
					tst_resm(TINFO,
						 "\tStat: size=%llx, ino=%x",
						 stat.st_size, (unsigned)stat.st_ino);
					sync();
					ft_dumpbuf(buf, csize);
					ft_dumpbits(bits, (nchunks + 7) / 8);
					ft_orbits(hold_bits, bits,
						  (nchunks + 7) / 8);
					tst_resm(TINFO, "Hold ");
					ft_dumpbits(hold_bits,
						    (nchunks + 7) / 8);
					tst_exit();
				}
			}
			/*
			 * Write it.
			 */
			if (lseek(fd, -xfr, 1) < 0) {
				tst_brkm(TFAIL,
					 NULL,
					 "Test[%d]: lseek(1) fail at %x, errno = %d.",
					 me, CHUNK(chunk), errno);
			}
			if ((xfr = write(fd, val_buf, csize)) < csize) {
				if (errno == ENOSPC) {
					tst_resm(TFAIL,
						 "Test[%d]: no space, exiting.",
						 me);
					fsync(fd);
					tst_exit();
				}
				tst_brkm(TFAIL,
					 NULL,
					 "Test[%d]: write fail at %x xfr %d, errno = %d.",
					 me, CHUNK(chunk), xfr, errno);
			}
			if (CHUNK(chunk) + csize > file_max)
				file_max = CHUNK(chunk) + csize;
			/*
			 * If hit "misc" interval, do it.
			 */
			if (misc_intvl && --whenmisc <= 0) {
				ft_orbits(hold_bits, bits, (nchunks + 7) / 8);
				domisc(me, fd, bits);
				whenmisc = NEXTMISC;
			}
			if (count + collide > 2 * nchunks)
				break;
		}

		/*
		 * End of iteration, maybe before doing all chunks.
		 */
		fsync(fd);
		++misc_cnt[m_fsync];
		//tst_resm(TINFO, "Test{%d} val %d done, count = %d, collide = {%d}",
		//              me, val, count, collide);
		//for (i = 0; i < NMISC; i++)
		//      tst_resm(TINFO, "Test{%d}: {%d} %s's.", me, misc_cnt[i], m_str[i]);
		++val;
	}
}

/*
 * domisc()
 *	Inject misc syscalls into the thing.
 */
static void domisc(int me, int fd, char *bits)
{
	int chunk;
	struct stat sb;

	if (type > m_fstat)
		type = m_fsync;
	switch (type) {
	case m_fsync:
		if (fsync(fd) < 0) {
			tst_brkm(TFAIL | TERRNO, NULL,
				 "Test[%d]: fsync failed.", me);
		}
		break;
	case m_trunc:
		chunk = rand() % (file_max / csize);
		file_max = CHUNK(chunk);
		last_trunc = file_max;
		if (tr_flag) {
			if (ftruncate(fd, file_max) < 0) {
				tst_brkm(TFAIL | TERRNO, NULL,
					 "Test[%d]: ftruncate failed @ 0x%x.",
					 me, file_max);
			}
			tr_flag = 0;
		} else {
			if (truncate(test_name, file_max) < 0) {
				tst_brkm(TFAIL | TERRNO, NULL,
					 "Test[%d]: truncate failed @ 0x%x.",
					 me, file_max);
			}
			tr_flag = 1;
		}
		for (; chunk % 8 != 0; chunk++)
			bits[chunk / 8] &= ~(1 << (chunk % 8));
		for (; chunk < nchunks; chunk += 8)
			bits[chunk / 8] = 0;
		break;
	case m_sync:
		sync();
		break;
	case m_fstat:
		if (fstat(fd, &sb) < 0)
			tst_brkm(TFAIL | TERRNO, NULL,
				 "\tTest[%d]: fstat failed", me);
		if (sb.st_size != file_max)
			tst_brkm(TFAIL, NULL,
				 "\tTest[%d]: fstat() mismatch; st_size=%lu, "
				 "file_max=%x.", me, sb.st_size, file_max);
		break;
	}

	++misc_cnt[type];
	++type;
}

/*
 * SIGTERM signal handler.
 */
static void term(int sig LTP_ATTRIBUTE_UNUSED)
{
	int i;

	tst_resm(TINFO, "\tterm -[%d]- got sig term.", getpid());

	/*
	 * If run by hand we like to have the parent send the signal to
	 * the child processes.
	 */
	if (parent_pid == getpid()) {
		for (i = 0; i < nchild; i++)
			if (pidlist[i])
				kill(pidlist[i], SIGTERM);
		tst_exit();
	}

	tst_resm(TINFO, "\tunlinking '%s'", test_name);

	close(fd);

	if (unlink(test_name) == -1)
		tst_resm(TBROK | TERRNO, "unlink failed");

	tst_exit();
}

static void cleanup(void)
{

	tst_rmdir();
}

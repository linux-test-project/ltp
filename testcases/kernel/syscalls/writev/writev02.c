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
 *	writev02.c
 *
 * DESCRIPTION
 *	In these testcases, writev() is called with partially valid data
 *	to be written in a sparse file.
 *
 * ALGORITHM
 *	The file is created and write() is called with valid buffer to write
 *	at some 8k th offset. After that writev() will be called with invalid
 *	vector. This should return EFAULT. And at the same time, try looking at
 *	the 8kth offset whether the file is intact or not.
 *
 * USAGE: <for command-line>
 *	writev02 [-c n] [-e] [-i n] [-I x] [-P x] [-t]
 *      where,  -c n : Run n copies concurrently.
 *              -e   : Turn on errno logging.
 *              -i n : Execute test n times.
 *              -I x : Execute test for x seconds.
 *              -P x : Pause for x seconds between iterations.
 *              -t   : Turn on syscall timing.
 *
 * History
 *	07/2001 John George
 *		-Ported
 *      04/2002 wjhuie sigset cleanups
 *
 * Restrictions
 *	None
 */

#include <sys/types.h>
#include <signal.h>
#include <sys/uio.h>
#include <fcntl.h>
#include <memory.h>
#include <errno.h>
#include "test.h"
#include <sys/mman.h>

#define	K_1	8192

#define	NBUFS		2
#define	CHUNK		K_1	/* single chunk */
#define	MAX_IOVEC	2
#define	DATA_FILE	"writev_data_file"

char buf1[K_1];
char buf2[K_1];

char *bad_addr = 0;

struct iovec wr_iovec[MAX_IOVEC] = {
	{(caddr_t) - 1, CHUNK},
	{NULL, 0},
};

char name[K_1], f_name[K_1];

int fd[2], in_sighandler;
char *buf_list[NBUFS];

char *TCID = "writev02";
int TST_TOTAL = 1;

void sighandler(int);
void l_seek(int, off_t, int);
void setup(void);
void cleanup(void);

int main(int argc, char **argv)
{
	int lc;

	int nbytes;

	tst_parse_opts(argc, argv, NULL, NULL);

	setup();

	for (lc = 0; TEST_LOOPING(lc); lc++) {

		tst_count = 0;

		buf_list[0] = buf1;
		buf_list[1] = buf2;

		memset(buf_list[0], 0, K_1);
		memset(buf_list[1], 0, K_1);

		fd[1] = -1;	/* Invalid file descriptor */

		if (signal(SIGTERM, sighandler) == SIG_ERR)
			tst_brkm(TFAIL | TERRNO, cleanup,
				 "signal(SIGTERM, ..)");

		if (signal(SIGPIPE, sighandler) == SIG_ERR)
			tst_brkm(TFAIL | TERRNO, cleanup,
				 "signal(SIGPIPE, ..)");

		if ((fd[0] = open(f_name, O_WRONLY | O_CREAT, 0666)) < 0)
			tst_brkm(TFAIL | TERRNO, cleanup,
				 "open(.., O_WRONLY|O_CREAT, ..) failed");
		else {
			l_seek(fd[0], K_1, 0);
			if ((nbytes = write(fd[0], buf_list[1], K_1)) != K_1)
				tst_brkm(TFAIL | TERRNO, cleanup,
					 "write failed");
		}

		if (close(fd[0]) == -1)
			tst_brkm(TFAIL | TERRNO, cleanup, "close failed");

		if ((fd[0] = open(f_name, O_RDWR, 0666)) < 0)
			tst_brkm(TFAIL | TERRNO, cleanup,
				 "open(.., O_RDWR, ..) failed");
		/*
		 * In this block we are trying to call writev() with invalid
		 * vector to be written in a sparse file. This will return
		 * EFAULT. At the same time, check should be made whether
		 * the scheduled write() with valid data at 8k th offset is
		 * done correctly or not.
		 */
		l_seek(fd[0], 0, 0);
		TEST(writev(fd[0], wr_iovec, 2));
		if (TEST_RETURN < 0) {
			if (TEST_ERRNO == EFAULT) {
				tst_resm(TPASS, "Received EFAULT as expected");
			} else if (TEST_ERRNO != EFAULT) {
				tst_resm(TFAIL, "Expected EFAULT, got %d",
					 TEST_ERRNO);
			}
			l_seek(fd[0], K_1, 0);
			if ((nbytes = read(fd[0], buf_list[0], CHUNK)) != CHUNK) {
				tst_resm(TFAIL, "Expected nbytes = 64, got "
					 "%d", nbytes);
			} else {
				if (memcmp(buf_list[0], buf_list[1], CHUNK)
				    != 0)
					tst_resm(TFAIL, "Error: writev() "
						 "over wrote %s", f_name);
			}
		} else
			tst_resm(TFAIL, "Error writev returned a positive "
				 "value");
	}
	cleanup();
	tst_exit();
}

void setup(void)
{
	tst_sig(FORK, sighandler, cleanup);

	TEST_PAUSE;

	tst_tmpdir();

	strcpy(name, DATA_FILE);
	sprintf(f_name, "%s.%d", name, getpid());

	bad_addr = mmap(0, 1, PROT_NONE, MAP_PRIVATE | MAP_ANONYMOUS, 0, 0);
	if (bad_addr == MAP_FAILED)
		tst_brkm(TBROK | TERRNO, cleanup, "mmap failed");
	wr_iovec[0].iov_base = bad_addr;

}

void cleanup(void)
{
	close(fd[0]);
	close(fd[1]);

	if (munmap(bad_addr, 1) == -1)
		tst_resm(TWARN | TERRNO, "unmap failed");
	if (unlink(f_name) == -1)
		tst_resm(TWARN | TERRNO, "unlink failed");

	tst_rmdir();

}

void sighandler(int sig)
{
	switch (sig) {
	case SIGTERM:
		break;
	case SIGPIPE:
		++in_sighandler;
		return;
	default:
		tst_resm(TBROK, "sighandler received invalid signal : %d", sig);
		break;
	}

	if (unlink(f_name) == -1 && errno != ENOENT)
		tst_resm(TFAIL | TERRNO, "unlink failed");
}

/*
 * l_seek()
 *	Wrap around for regular lseek function for giving error message
 */
void l_seek(int fdesc, off_t offset, int whence)
{
	if (lseek(fdesc, offset, whence) == -1)
		tst_resm(TBROK | TERRNO, "lseek failed");
}

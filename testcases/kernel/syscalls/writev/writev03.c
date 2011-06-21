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
 *   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 */

/*
 * NAME
 *	writev03.c
 *
 * DESCRIPTION
 *	The testcases are written calling writev() with partially valid data
 *	to overwrite the contents, to write in the beginning and to write in
 *	the end of the file.
 *
 * USAGE:  <for command-line>
 *      writev03 [-c n] [-e] [-i n] [-I x] [-P x] [-t]
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
 *	NONE
 */

#include <sys/types.h>
#include <sys/uio.h>
#include <sys/mman.h>
#include <unistd.h>
#include <signal.h>
#include <fcntl.h>
#include <memory.h>
#include <errno.h>
#include "test.h"
#include "usctest.h"

#define	K_1	1024

#define	NBUFS		4
#define	CHUNK		64	/* single chunk */
#define	MAX_IOVEC	4
#define	DATA_FILE	"writev_data_file"

char buf1[K_1], buf2[K_1], buf3[K_1];
char *bad_addr = 0;

struct iovec wr_iovec[MAX_IOVEC] = {
	/* testcase #1 */
	{buf1 + (CHUNK * 6), CHUNK},
	{(caddr_t) - 1, CHUNK},
	{buf1 + (CHUNK * 8), CHUNK},
	{(caddr_t) NULL, 0}
};

/* 0 terminated list of expected errnos */
int exp_enos[] = { 0 };

char name[K_1], f_name[K_1];
int fd[2], in_sighandler;
char *buf_list[NBUFS];

char *TCID = "writev03";
int TST_TOTAL = 1;

void sighandler(int);
void l_seek(int, off_t, int);
void setup(void);
void cleanup(void);

int main(int argc, char **argv)
{
	int lc;			/* loop counter */
	char *msg;		/* message returned from parse_opts */

	int nbytes;

	if ((msg = parse_opts(argc, argv, NULL, NULL)) != NULL)
		tst_brkm(TBROK, NULL, "OPTION PARSING ERROR - %s", msg);

	setup();

	for (lc = 0; TEST_LOOPING(lc); lc++) {

		Tst_count = 0;

		buf_list[0] = buf1;
		buf_list[1] = buf2;
		buf_list[2] = buf3;
		buf_list[3] = NULL;

		fd[1] = -1;	/* Invalid file descriptor */

		if (signal(SIGTERM, sighandler) == SIG_ERR)
			tst_brkm(TBROK|TERRNO, cleanup,
			    "signal(SIGTERM, ..) failed");

		if (signal(SIGPIPE, sighandler) == SIG_ERR)
			tst_brkm(TBROK|TERRNO, cleanup,
			    "signal(SIGPIPE, ..) failed");

		memset(buf_list[0], 0, K_1);
		memset(buf_list[1], 0, K_1);

		if ((fd[0] = open(f_name, O_WRONLY|O_CREAT, 0666)) == -1)
			tst_brkm(TBROK|TERRNO, cleanup,
			    "open(.., O_WRONLY|O_CREAT, ..) failed");
		else
			if ((nbytes = write(fd[0], buf_list[1], K_1)) != K_1)
				tst_brkm(TFAIL|TERRNO, cleanup, "write failed");

		if (close(fd[0]) < 0)
			tst_brkm(TBROK|TERRNO, cleanup, "close failed");

		if ((fd[0] = open(f_name, O_RDWR, 0666)) == -1)
			tst_brkm(TBROK|TERRNO, cleanup,
			    "open(.., O_RDWR, ..) failed");
//block1:
		tst_resm(TINFO, "Enter block 1");

		/*
		 * In this block we are trying to call writev() with
		 * partially valid data. This should return the valid number
		 * of bytes written in the vector. If it returns EFAULT, it
		 * is an error. And after returning the number of valid
		 * bytes written, the check should be made to verify the
		 * contents of the first valid write() scheduled.
		 */

		if (writev(fd[0], wr_iovec, 3) == -1) {
			if (errno == EFAULT)
				tst_resm(TFAIL, "Got EFAULT");
		} else {
			l_seek(fd[0], 0, 0);
			read(fd[0], buf_list[0], CHUNK);
			if (memcmp(buf_list[0], buf_list[1], CHUNK) != 0)
				tst_resm(TFAIL, "writev overwrote the file");
		}
		tst_resm(TINFO, "Exit block 1");

//block2:
		tst_resm(TINFO, "Enter block 2");

		/*
		 * In this block we are trying to over write the contents by
		 * calling writev() with partially valid data. It should
		 * return the valid number of bytes written but not EFAULT.
		 * Also the check should be made whether the initial write()
		 * scheduled is done correctly or not.
		 */
		l_seek(fd[0], 0, 0);
		if (writev(fd[0], wr_iovec, 3) == -1) {
			if (errno == EFAULT)
				tst_resm(TFAIL, "Got EFAULT");
		} else {
			l_seek(fd[0], 0, 0);
			read(fd[0], buf_list[0], CHUNK);
			if (memcmp(buf_list[0], buf_list[1], CHUNK) != 0)
				tst_resm(TFAIL, "writev overwrote the file");
		}
		tst_resm(TINFO, "Exit block 2");

//block3:
		tst_resm(TINFO, "Enter block 3");

		/*
		 * In this block, we are trying to call writev() by going to
		 * some end position of the file. Here writev() is called
		 * with partially valid data, and this will return the
		 * number of valid bytes written and not EFAULT. Also, the
		 * check should be made whether the inital write() that is
		 * scheduled with valid data is done correctly done or not.
		 */

		l_seek(fd[0], 8192, 0);
		if (writev(fd[0], wr_iovec, 3) == -1) {
			TEST_ERROR_LOG(errno);
			if (errno == EFAULT)
				tst_resm(TFAIL, "Got EFAULT");
		} else {
			l_seek(fd[0], 0, 0);
			read(fd[0], buf_list[0], CHUNK);
			if (memcmp(buf_list[0], buf_list[1], CHUNK) != 0) {
				tst_resm(TFAIL, "writev overwrote the file");
			}
		}

		tst_resm(TINFO, "Exit block 3");
	}
	cleanup();
	tst_exit();
}

/*
 * setup()
 *	performs all ONE TIME setup for this test
 */
void setup(void)
{

	tst_sig(FORK, DEF_HANDLER, cleanup);

	/* Set up the expected error numbers for -e option */
	TEST_EXP_ENOS(exp_enos);

	/* Pause if that option was specified.
	 * TEST_PAUSE contains the code to fork the test with the -i option.
	 * You want to make sure you do this before you create your temporary
	 * directory.
	 */
	TEST_PAUSE;

	strcpy(name, DATA_FILE);
	sprintf(f_name, "%s.%d", name, getpid());

	bad_addr = mmap(0, 1, PROT_NONE, MAP_PRIVATE | MAP_ANONYMOUS, 0, 0);
	if (bad_addr == MAP_FAILED) {
		tst_brkm(TBROK, cleanup, "mmap failed");
	}
	wr_iovec[1].iov_base = bad_addr;
	tst_tmpdir();
}

/*
 * cleanup()
 *	performs all ONE TIME cleanup for this test at
 *	completion or premature exit
 */
void cleanup(void)
{
	TEST_CLEANUP;

	close(fd[0]);
	close(fd[1]);

	if (unlink(f_name) == -1)
		tst_resm(TFAIL, "unlink failed");
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
}

/*
 * l_seek()
 *	Wrap around for regular lseek function for giving error message
 */
void l_seek(int fdesc, off_t offset, int whence)
{
	if (lseek(fdesc, offset, whence) == -1)
		tst_brkm(TBROK|TERRNO, cleanup, "lseek failed");
}

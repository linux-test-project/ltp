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
 * 	readv02.c
 *
 * DESCRIPTION
 *	Testcase to check the error conditions of the readv(2) system call.
 *
 * CALLS
 * 	readv()
 *
 * ALGORITHM
 *	Create a IO vector, and attempt to readv() various components of it.
 *
 * USAGE
 *	readv02
 *
 * HISTORY
 *	07/2001 Ported by Wayne Boyer
 *
 * RESTRICTIONS
 * 	None
 */
#include <sys/types.h>
#include <sys/uio.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <memory.h>
#include <errno.h>

#include "test.h"
#include "safe_macros.h"

#define	K_1	1024
#define	M_1	K_1 * K_1
#define	G_1	M_1 * K_1

#define	NBUFS		4
#define	CHUNK		64
#define	MAX_IOVEC	16
#define DATA_FILE	"readv_data_file"

char buf1[K_1], buf2[K_1], buf3[K_1];

struct iovec rd_iovec[MAX_IOVEC] = {
	/* iov_base *//* iov_len */

	/* Test case #1 */
	{buf2, -1},
	{(buf2 + CHUNK), CHUNK},
	{(buf2 + CHUNK * 2), CHUNK},

	/* Test case #2 */
	{(buf2 + CHUNK * 3), G_1},
	{(buf2 + CHUNK * 4), G_1},
	{(buf2 + CHUNK * 5), G_1},

	/* Test case #3 */
	{(caddr_t) - 1, CHUNK},
	{(buf2 + CHUNK * 6), CHUNK},
	{(buf2 + CHUNK * 8), CHUNK},

	/* Test case #4 */
	{(buf2 + CHUNK * 9), CHUNK}
};

char f_name[K_1];

int fd[4];
char *buf_list[NBUFS];

char *TCID = "readv02";
int TST_TOTAL = 1;

char *bad_addr = 0;

int init_buffs(char **);
int fill_mem(char *, int, int);
long l_seek(int, long, int);
char *getenv();
void setup();
void cleanup();

int main(int ac, char **av)
{
	int lc;

	tst_parse_opts(ac, av, NULL, NULL);

	setup();

	/* The following loop checks looping state if -i option given */
	for (lc = 0; TEST_LOOPING(lc); lc++) {

		/* reset tst_count in case we are looping */
		tst_count = 0;

//test1:
		if (readv(fd[0], rd_iovec, 1) < 0) {
			if (errno != EINVAL) {
				tst_resm(TFAIL, "readv() set an illegal errno:"
					 " expected: EINVAL, got %d", errno);
			} else {
				tst_resm(TPASS, "got EINVAL");
			}
		} else {
			tst_resm(TFAIL, "Error: readv returned a positive "
				 "value");
		}

//test2:
		l_seek(fd[0], CHUNK * 6, 0);
		if (readv(fd[0], (rd_iovec + 6), 3) < 0) {
			if (errno != EFAULT) {
				tst_resm(TFAIL, "expected errno = EFAULT, "
					 "got %d", errno);
			} else {
				tst_resm(TPASS, "got EFAULT");
			}
			if (memcmp((buf_list[0] + CHUNK * 6),
				   (buf_list[1] + CHUNK * 6), CHUNK * 3) != 0) {
				tst_resm(TFAIL, "Error: readv() partially "
					 "overlaid buf[2]");
			}
		} else {
			tst_resm(TFAIL, "Error: readv returned a positive "
				 "value");
		}

//test3:
		if (readv(fd[1], (rd_iovec + 9), 1) < 0) {
			if (errno != EBADF) {
				tst_resm(TFAIL, "expected errno = EBADF, "
					 "got %d", errno);
			} else {
				tst_resm(TPASS, "got EBADF");
			}
		} else {
			tst_resm(TFAIL, "Error: readv returned a positive "
				 "value");
		}

//test4:
		l_seek(fd[0], CHUNK * 10, 0);
		if (readv(fd[0], (rd_iovec + 10), -1) < 0) {
			if (errno != EINVAL) {
				tst_resm(TFAIL, "expected errno = EINVAL, "
					 "got %d", errno);
			} else {
				tst_resm(TPASS, "got EINVAL");
			}
		} else {
			tst_resm(TFAIL, "Error: readv returned a positive "
				 "value");
		}

	}
	close(fd[0]);
	close(fd[1]);
	cleanup();
	tst_exit();

}

/*
 * setup() - performs all ONE TIME setup for this test.
 */
void setup(void)
{
	int nbytes;

	tst_sig(NOFORK, DEF_HANDLER, cleanup);

	TEST_PAUSE;

	/* make a temporary directory and cd to it */
	tst_tmpdir();

	buf_list[0] = buf1;
	buf_list[1] = buf2;
	buf_list[2] = buf3;
	buf_list[3] = NULL;

	init_buffs(buf_list);

	sprintf(f_name, "%s.%d", DATA_FILE, getpid());

	if ((fd[0] = open(f_name, O_WRONLY | O_CREAT, 0666)) < 0) {
		tst_brkm(TBROK, cleanup, "open failed: fname = %s, "
			 "errno = %d", f_name, errno);
	} else {
		if ((nbytes = write(fd[0], buf_list[2], K_1)) != K_1) {
			tst_brkm(TBROK, cleanup, "write failed: nbytes "
				 "= %d " "errno = %d", nbytes, errno);
		}
	}

	SAFE_CLOSE(cleanup, fd[0]);

	if ((fd[0] = open(f_name, O_RDONLY, 0666)) < 0) {
		tst_brkm(TBROK, cleanup, "open failed: fname = %s, "
			 "errno = %d", f_name, errno);
	}

	fd[1] = -1;		/* Invalid file descriptor */

	bad_addr = mmap(0, 1, PROT_NONE,
			MAP_PRIVATE_EXCEPT_UCLINUX | MAP_ANONYMOUS, 0, 0);
	if (bad_addr == MAP_FAILED) {
		tst_brkm(TBROK, cleanup, "mmap failed");
	}
	rd_iovec[6].iov_base = bad_addr;
}

/*
 * cleanup() - performs all ONE TIME cleanup for this test at
 *	       completion or premature exit.
 */
void cleanup(void)
{
	SAFE_UNLINK(NULL, f_name);
	tst_rmdir();

}

int init_buffs(char *pbufs[])
{
	int i;

	for (i = 0; pbufs[i] != NULL; i++) {
		switch (i) {
		case 0:
		 /*FALLTHROUGH*/ case 1:
			fill_mem(pbufs[i], 0, 1);
			break;

		case 2:
			fill_mem(pbufs[i], 1, 0);
			break;

		default:
			tst_brkm(TBROK, cleanup, "Error in init_buffs()");
		}
	}
	return 0;
}

int fill_mem(char *c_ptr, int c1, int c2)
{
	int count;

	for (count = 1; count <= K_1 / CHUNK; count++) {
		if (count & 0x01) {	/* if odd */
			memset(c_ptr, c1, CHUNK);
		} else {	/* if even */
			memset(c_ptr, c2, CHUNK);
		}
	}
	return 0;
}

long l_seek(int fdesc, long offset, int whence)
{
	SAFE_LSEEK(cleanup, fdesc, offset, whence);
	return 0;
}

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
 * 	readv01.c
 *
 * DESCRIPTION
 *	Testcase to check the basic functionality of the readv(2) system call.
 *
 * CALLS
 * 	readv()
 *
 * ALGORITHM
 *	Create a IO vector, and attempt to readv() various components of it.
 *
 * USAGE
 *	readv01
 *
 * HISTORY
 *	07/2001 Ported by Wayne Boyer
 *
 * RESTRICTIONS
 * 	None
 */
#include <sys/types.h>
#include <sys/uio.h>
#include <sys/fcntl.h>
#include <memory.h>
#include <errno.h>

#include "test.h"
#include "usctest.h"

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
	{(buf2 + CHUNK * 10), CHUNK},

	/* Test case #2 */
	{(buf2 + CHUNK * 11), CHUNK}
};

char f_name[K_1];

int fd;
char *buf_list[NBUFS];

char *TCID = "readv01";
int TST_TOTAL = 1;

int init_buffs(char **);
int fill_mem(char *, int, int);
long l_seek(int, long, int);
char *getenv();
void setup();
void cleanup();

int main(int ac, char **av)
{
	int lc;			/* loop counter */
	char *msg;		/* message returned from parse_opts */

	/* parse standard options */
	if ((msg = parse_opts(ac, av, NULL, NULL)) != NULL) {
		tst_brkm(TBROK, NULL, "OPTION PARSING ERROR - %s", msg);
	 }

	setup();

	/* The following loop checks looping state if -i option given */
	for (lc = 0; TEST_LOOPING(lc); lc++) {

		/* reset Tst_count in case we are looping */
		Tst_count = 0;

//test1:
		l_seek(fd, CHUNK * 11, 0);
		if (readv(fd, (rd_iovec + 0), 0) == -1) {
			tst_resm(TFAIL, "readv() failed with unexpected errno "
				 "%d", errno);
		} else {
			tst_resm(TPASS, "readv read 0 io vectors");
		}

//test2:
		l_seek(fd, CHUNK * 12, 0);
		if (readv(fd, (rd_iovec + 1), 4) != CHUNK) {
			tst_resm(TFAIL, "readv failed reading %d bytes, "
				 "followed by two NULL vectors", CHUNK);
		} else {
			tst_resm(TPASS, "readv passed reading %d bytes, "
				 "followed by two NULL vectors", CHUNK);
		}
	}
	close(fd);
	cleanup();
	tst_exit();

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
	if (lseek(fdesc, offset, whence) < 0) {
		tst_brkm(TBROK, cleanup, "lseek Failed : errno = %d", errno);
	}
	return 0;
}

/*
 * setup() - performs all ONE TIME setup for this test.
 */
void setup()
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

	if ((fd = open(f_name, O_WRONLY | O_CREAT, 0666)) < 0) {
		tst_brkm(TBROK, cleanup, "open failed: fname = %s, "
			 "errno = %d", f_name, errno);
	} else {
		if ((nbytes = write(fd, buf_list[2], K_1)) != K_1) {
			tst_brkm(TBROK, cleanup, "write failed: nbytes "
				 "= %d errno = %d", nbytes, errno);
		}
	}

	if (close(fd) < 0) {
		tst_brkm(TBROK, cleanup, "close failed: errno = %d", errno);
	}

	if ((fd = open(f_name, O_RDONLY, 0666)) < 0) {
		tst_brkm(TBROK, cleanup, "open failed: fname = %s, "
			 "errno = %d", f_name, errno);
	}
}

/*
 * cleanup() - performs all ONE TIME cleanup for this test at
 *	       completion or premature exit.
 */
void cleanup()
{
	/*
	 * print timing stats if that option was specified.
	 * print errno log if that option was specified.
	 */
	TEST_CLEANUP;

	if (unlink(f_name) < 0) {
		tst_brkm(TBROK, NULL, "unlink FAILED: file %s, errno %d",
			 f_name, errno);
	}
	tst_rmdir();

}
/*
 *
 *   Copyright (c) International Business Machines  Corp., 2002
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
 *      pwrite04.c (ported from SPIE, section2/filesuite/pread_pwrite.c,
 *      	        by Airong Zhang)
 *
 * TEST SUMMARY
 *	Test the pwrite() system call with O_APPEND.
 *
 * USAGE
 *  	pwrite04
 *
 */

#define _XOPEN_SOURCE 500
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <memory.h>
#include <errno.h>
#include "test.h"

char *TCID = "pwrite04";
int TST_TOTAL = 1;
int local_flag;

#define PASSED 1
#define FAILED 0

int block_cnt = 0;

#define K1    		1024
#define K2    		(K1 * 2)
#define K3    		(K1 * 3)
#define K4    		(K1 * 4)
#define K5    		(K1 * 5)
#define	NBUFS 		4
#define DATA_FILE	"pwrite04_file"

char name[256], fname[256];

void init_buffers(char *[]);
void l_seek(int, off_t, int, off_t);
static void cleanup(void);

int main(int ac, char *av[])
{
	int fd;
	int nbytes;
	char *wbuf[NBUFS];
	struct stat statbuf;
	int lc;

	strcpy(name, DATA_FILE);
	sprintf(fname, "%s.%d", name, getpid());

	tst_parse_opts(ac, av, NULL, NULL);

	tst_tmpdir();
	for (lc = 0; TEST_LOOPING(lc); lc++) {

		init_buffers(wbuf);
		local_flag = PASSED;

		if ((fd = open(fname, O_RDWR | O_CREAT, 0666)) < 0) {
			tst_resm(TBROK, "open failed: fname = %s, errno = %d",
				 fname, errno);
			cleanup();
		}
		/*
		 * pwrite() K1 of data (0's) at offset 0.
		 */
		if ((nbytes = pwrite(fd, wbuf[0], K1, 0)) != K1) {
			tst_resm(TFAIL,
				 "pwrite at 0 failed: nbytes=%d, errno=%d",
				 nbytes, errno);
			cleanup();
		}

		/*
		 * We should still be at offset 0.
		 */
		l_seek(fd, 0, SEEK_CUR, 0);

		/*
		 * lseek() to a non K boundary, just to be different.
		 */
		l_seek(fd, K1 / 2, SEEK_SET, K1 / 2);

		/*
		 * pwrite() K1 of data (2's) at offset K2.
		 */
		if ((nbytes = pwrite(fd, wbuf[2], K1, K2)) != K1) {
			tst_resm(TFAIL,
				 "pwrite at K2 failed: nbytes=%d, errno=%d",
				 nbytes, errno);
			cleanup();
		}

		/*
		 * We should still be at our non K boundary.
		 */
		l_seek(fd, 0, SEEK_CUR, K1 / 2);

		/*
		 * lseek() to an offset of K3.
		 */
		l_seek(fd, K3, SEEK_SET, K3);

		/*
		 * This time use a normal write() of K1 of data (3's) which should
		 * take place at an offset of K3, moving the file pointer to K4.
		 */
		if ((nbytes = write(fd, wbuf[3], K1)) != K1) {
			tst_resm(TFAIL, "write failed: nbytes=%d, errno=%d",
				 nbytes, errno);
			cleanup();
		}

		/*
		 * We should be at offset K4.
		 */
		l_seek(fd, 0, SEEK_CUR, K4);

		/*
		 * pwrite() K1 of data (1's) at offset K1.
		 */
		if ((nbytes = pwrite(fd, wbuf[1], K1, K1)) != K1) {
			tst_resm(TFAIL, "pwrite failed: nbytes=%d, errno=%d",
				 nbytes, errno);
			cleanup();
		}

	/*--------------------------------------------------------------*/

		/*
		 * Now test that O_APPEND takes precedence over any
		 * offset specified by pwrite(), but that the file
		 * pointer remains unchanged.  First, close then reopen
		 * the file and ensure it is already K4 in length and
		 * set the file pointer to it's midpoint, K2.
		 */
		close(fd);
		if ((fd = open(fname, O_RDWR | O_APPEND, 0666)) < 0) {
			tst_resm(TBROK, "open failed: fname = %s, errno = %d",
				 fname, errno);
			cleanup();
		}
		if (fstat(fd, &statbuf) == -1) {
			tst_resm(TFAIL, "fstat failed: errno = %d", errno);
			cleanup();
		}
		if (statbuf.st_size != K4) {
			tst_resm(TFAIL, "file size is %ld != K4",
				 statbuf.st_size);
			cleanup();
		}
		l_seek(fd, K2, SEEK_SET, K2);

		/*
		 * Finally, pwrite() some K1 of data at offset 0.
		 * What we should end up with is:
		 *      -The file pointer should still be at K2.
		 *      -The data should have been written to the end
		 *       of the file (O_APPEND) and should be K5 in size.
		 */
		if ((nbytes = pwrite(fd, wbuf[0], K1, 0)) != K1) {
			tst_resm(TFAIL,
				 "pwrite at 0 failed: nbytes=%d, errno=%d",
				 nbytes, errno);

		}
		l_seek(fd, 0, SEEK_CUR, K2);
		if (fstat(fd, &statbuf) == -1) {
			tst_resm(TFAIL, "fstat failed: errno = %d", errno);

		}
		if (statbuf.st_size != K5) {
			tst_resm(TFAIL, "file size is %ld != K4",
				 statbuf.st_size);

		}
		tst_resm(TPASS, "O_APPEND test passed.");

	/*------------------------------------------------------------------------*/

		close(fd);
		unlink(fname);
	}			/* end for */
	cleanup();
	tst_exit();

}

/*------------------------------------------------------------------------*/

/*
 * init_buffers() allocates wbuf[] array
 * as follows:
 * wbuf[0] has 0's, wbuf[1] has 1's, wbuf[2] has 2's, and wbuf[3] has 3's.
 */
void init_buffers(char *wbuf[])
{
	int i;

	for (i = 0; i < NBUFS; i++) {
		wbuf[i] = malloc(K1);
		if (wbuf[i] == NULL) {
			tst_brkm(TBROK, NULL, "ib: malloc failed: errno=%d",
				 errno);
		}
		memset(wbuf[i], i, K1);
	}
}

/*
 * l_seek() is a local front end to lseek().
 * "checkoff" is the offset at which we believe we should be at.
 * Used to validate pwrite doesn't move the offset.
 */
void l_seek(int fdesc, off_t offset, int whence, off_t checkoff)
{
	off_t offloc;

	if ((offloc = lseek(fdesc, offset, whence)) != checkoff) {
		tst_brkm(TFAIL, NULL,
			 "(%ld = lseek(%d, %ld, %d)) != %ld) errno = %d",
			 offloc, fdesc, offset, whence, checkoff, errno);
	}
}

/*
 * cleanup() - Performs all ONE TIME cleanup for this test at
 *             completion or premature exit.
 *
 *	Print test timing stats and errno log if test executed with options.
 *	Close the testfile if still opened.
 *	Remove temporary directory and sub-directories/files under it
 *	created during setup().
 *	Exit the test program with normal exit code.
 */
void cleanup(void)
{

	tst_rmdir();

}

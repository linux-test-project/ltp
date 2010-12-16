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
 *   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 */

/*
 * NAME
 *      diotest1.c
 *
 * DESCRIPTION
 *	Copy the contents of the input file to output file using direct read
 *	and direct write. The input file size is numblks*bufsize.
 *	The read and write calls use bufsize to perform IO. Input and output
 *	files can be specified through commandline and is useful for running
 *	test with raw devices as files.
 *
 * USAGE
 *	diotest1 [-b bufsize] [-n numblks] [-i infile] [-o outfile]
 *
 * History
 *	04/22/2002	Narasimha Sharoff nsharoff@us.ibm.com
 *
 * RESTRICTIONS
 *	None
*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <string.h>
#include <sys/fcntl.h>
#include <errno.h>

#include "diotest_routines.h"

#include "test.h"
#include "usctest.h"

char *TCID="diotest01";		 		 /* Test program identifier.    */
int TST_TOTAL=1;		 		 /* Total number of test conditions */

#ifdef O_DIRECT

#define	BUFSIZE	8192
#define	NBLKS	20
#define	LEN	30
#define	TRUE 1

/*
 * prg_usage: display the program usage.
*/
void
prg_usage()
{
	fprintf(stderr, "Usage: diotest1 [-b bufsize] [-n numblks] [-i infile] [-o outfile]\n");
	tst_resm (TBROK, "usage");
	tst_exit();
}

/*
 * fail_clean: cleanup and exit.
*/
void
fail_clean(int fd1, int fd2, char *infile, char *outfile)
{
	close(fd1);
	close(fd2);
	unlink(infile);
	unlink(outfile);
	tst_resm (TFAIL, "Test failed");
	tst_exit();
}

int
main(int argc, char *argv[])
{
	int	bufsize = BUFSIZE;	/* Buffer size. Default 8k */
	int	numblks = NBLKS;	/* Number of blocks. Default 20 */
	char	infile[LEN];		/* Input file. Default "infile" */
	char	outfile[LEN];		/* Output file. Default "outfile" */
	int	fd, fd1, fd2;
	int	i, n, offset;
	char	*buf;

	/* Options */
	strcpy(infile, "infile");	/* Default input file */
	strcpy(outfile, "outfile");	/* Default outfile file */
	while ((i = getopt(argc, argv, "b:n:i:o:")) != -1) {
		switch(i) {
		case 'b':
			if ((bufsize = atoi(optarg)) <= 0) {
				fprintf(stderr, "bufsize must be > 0\n");
				prg_usage();
			}
			if (bufsize % 4096 != 0) {
				fprintf(stderr, "bufsize must be multiple of 4k\n");
				prg_usage();
			}
			break;
		case 'n':
			if ((numblks = atoi(optarg)) <= 0) {
				fprintf(stderr, "numblks must be > 0\n");
				prg_usage();
			}
			break;
		case 'i':
			strcpy(infile, optarg);
			break;
		case 'o':
			strcpy(outfile, optarg);
			break;
		default:
			prg_usage();
		}
	}

	/* Test for filesystem support of O_DIRECT */
	if ((fd = open(infile, O_DIRECT|O_RDWR|O_CREAT, 0666)) < 0) {
	         tst_resm(TCONF,"O_DIRECT is not supported by this filesystem.");
                 tst_exit();
	}else{
		close(fd);
	}

	/* Open files */
	if ((fd1 = open(infile, O_DIRECT|O_RDWR|O_CREAT, 0666)) < 0) {
		tst_resm(TFAIL, "open infile failed: %s", strerror(errno));
		tst_exit();
	}

	if ((fd2 = open(outfile, O_DIRECT|O_RDWR|O_CREAT, 0666)) < 0) {
		close(fd1);
		unlink(infile);
		tst_resm(TFAIL,"open outfile failed: %s", strerror(errno));
		tst_exit();
	}

	/* Allocate for buf, Create input file */
	if ((buf = valloc(bufsize)) == 0) {
		tst_resm(TFAIL, "valloc() failed: %s", strerror(errno));
		fail_clean(fd1, fd2, infile, outfile);
	}
	for (i = 0; i < numblks; i++) {
		fillbuf(buf, bufsize, (char)(i % 256));
		if (write(fd1, buf, bufsize) < 0) {
			tst_resm(TFAIL, "write infile failed: %s", strerror(errno));
			fail_clean(fd1, fd2, infile, outfile);
		}
	}

	/* Copy infile to outfile using direct read and direct write */
	offset = 0;
	if (lseek(fd1, offset, SEEK_SET) < 0) {
		tst_resm(TFAIL, "lseek(infd) failed: %s", strerror(errno));
		fail_clean(fd1, fd2, infile, outfile);
	}
	while ((n = read(fd1, buf, bufsize)) > 0) {
		if (lseek(fd2, offset, SEEK_SET) < 0) {
			tst_resm(TFAIL, "lseek(outfd) failed: %s", strerror(errno));
			fail_clean(fd1, fd2, infile, outfile);
		}
		if (write(fd2, buf, n) < n) {
			tst_resm(TFAIL, "write(outfd) failed: %s", strerror(errno));
			fail_clean(fd1, fd2, infile, outfile);
		}
		offset += n;
		if (lseek(fd1, offset, SEEK_SET) < 0) {
			tst_resm(TFAIL, "lseek(infd) failed: %s", strerror(errno));
			fail_clean(fd1, fd2, infile, outfile);
		}
	}

	/* Verify */
	if (filecmp(infile, outfile) != 0) {
		tst_resm(TFAIL,"file compare failed for %s and %s",
			infile, outfile);
		fail_clean(fd1, fd2, infile, outfile);
	}

	/* Cleanup */
	close(fd1);
	close(fd2);
	unlink(infile);
	unlink(outfile);
	tst_resm(TPASS, "Test passed");
	tst_exit();
	tst_exit();
}

#else /* O_DIRECT */

int
main() {
	tst_resm(TCONF,"O_DIRECT is not defined.");
	tst_exit();
}
#endif /* O_DIRECT */
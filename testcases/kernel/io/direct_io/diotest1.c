/*
 * Copyright (c) 2016 Oracle and/or its affiliates. All Rights Reserved.
 * Copyright (c) International Business Machines  Corp., 2002
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it would be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
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
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <string.h>
#include <fcntl.h>
#include <errno.h>

#include "diotest_routines.h"

#include "test.h"
#include "safe_macros.h"

char *TCID = "diotest01";	/* Test program identifier.    */
int TST_TOTAL = 1;		/* Total number of test conditions */

#ifdef O_DIRECT

#define	BUFSIZE	8192
#define	NBLKS	20
#define	LEN	30
#define	TRUE 1

static char infile[LEN];	/* Input file. Default "infile" */
static char outfile[LEN];	/* Output file. Default "outfile" */
static int fd1, fd2;

/*
 * prg_usage: display the program usage.
*/
void prg_usage(void)
{
	fprintf(stderr,
		"Usage: diotest1 [-b bufsize] [-n numblks] [-i infile] [-o outfile]\n");
	tst_brkm(TBROK, NULL, "usage");
}

void cleanup(void)
{
	if (fd1 > 0)
		close(fd1);
	if (fd2 > 0)
		close(fd2);

	tst_rmdir();
}

int main(int argc, char *argv[])
{
	int bufsize = BUFSIZE;	/* Buffer size. Default 8k */
	int numblks = NBLKS;	/* Number of blocks. Default 20 */
	int i, n, offset;
	char *buf;

	/* Options */
	strcpy(infile, "infile");	/* Default input file */
	strcpy(outfile, "outfile");	/* Default outfile file */
	while ((i = getopt(argc, argv, "b:n:i:o:")) != -1) {
		switch (i) {
		case 'b':
			if ((bufsize = atoi(optarg)) <= 0) {
				fprintf(stderr, "bufsize must be > 0\n");
				prg_usage();
			}
			if (bufsize % 4096 != 0) {
				fprintf(stderr,
					"bufsize must be multiple of 4k\n");
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

	tst_tmpdir();

	/* Test for filesystem support of O_DIRECT */
	int fd = open(infile, O_DIRECT | O_RDWR | O_CREAT, 0666);

	if (fd < 0)
		tst_brkm(TCONF, cleanup, "O_DIRECT not supported by FS");
	SAFE_CLOSE(cleanup, fd);

	/* Open files */
	fd1 = SAFE_OPEN(cleanup, infile, O_DIRECT | O_RDWR | O_CREAT, 0666);
	fd2 = SAFE_OPEN(cleanup, outfile, O_DIRECT | O_RDWR | O_CREAT, 0666);

	/* Allocate for buf, Create input file */
	buf = valloc(bufsize);

	if (!buf)
		tst_brkm(TFAIL | TERRNO, cleanup, "valloc() failed");

	for (i = 0; i < numblks; i++) {
		fillbuf(buf, bufsize, (char)(i % 256));
		SAFE_WRITE(cleanup, SAFE_WRITE_ALL, fd1, buf, bufsize);
	}

	/* Copy infile to outfile using direct read and direct write */
	offset = 0;
	SAFE_LSEEK(cleanup, fd1, offset, SEEK_SET);

	while ((n = read(fd1, buf, bufsize)) > 0) {
		SAFE_LSEEK(cleanup, fd2, offset, SEEK_SET);

		SAFE_WRITE(cleanup, SAFE_WRITE_ALL, fd2, buf, n);

		offset += n;
		SAFE_LSEEK(cleanup, fd1, offset, SEEK_SET);
	}

	/* Verify */
	if (filecmp(infile, outfile) != 0) {
		tst_brkm(TFAIL, cleanup, "file compare failed for %s and %s",
			 infile, outfile);
	}

	tst_resm(TPASS, "Test passed");

	cleanup();
	tst_exit();
}

#else /* O_DIRECT */

int main()
{
	tst_brkm(TCONF, NULL, "O_DIRECT is not defined.");
}
#endif /* O_DIRECT */

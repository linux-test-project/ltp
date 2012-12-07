/*
 *
 *   Copyright (c) CHANG Industry, Inc., 2004
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
 *  FILE        : writetest.c
 *  DESCRIPTION : The purpose of this test is to verify that writes to
 *                disk occur without corruption.  It writes one 1MB
 *                buffer at a time, where each byte in the buffer is
 *                generated from a random number.  Once done
 *                completed, the file is re-opened, the random number
 *                generator is re-seeded, and the file is verified.
 *
 *  HISTORY:
 *   05/12/2004 : Written by Danny Sung <dannys@changind.com> to
 *                verify integrity of disk writes.
 *
 */

#include <fcntl.h>
#include <getopt.h>
#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#include "test.h"

#define BLOCKSIZE (1024*1024)
#define FILE_OUT    "fileout"
#define FILE_MODE   0644
#define MAX_FILENAME_LEN 1024

int Verbosity = 0;
int DefaultSeed = 0;
char Filename[MAX_FILENAME_LEN] = FILE_OUT;
off_t NumBlocks = 1;
char *TCID = "writetest";
int TST_TOTAL = 2;

void buf_init(void)
{
	static int seed = 0;
	if (seed == 0)
		seed = DefaultSeed;
	srand(seed);
}

void buf_fill(uint8_t * buf)
{
	int i;
	for (i = 0; i < BLOCKSIZE; i++) {
		*buf = (rand() & 0xff);
		buf++;
	}
}

int write_file(off_t num_blocks, const char *filename)
{
	int fd;
	int ret = 0;
	off_t block;
	uint8_t buf[BLOCKSIZE];

	fd = open(filename, O_RDWR | O_CREAT | O_TRUNC | O_LARGEFILE,
		  FILE_MODE);
	if (fd < 0) {
		perror(TCID);
		return (-1);
	}
	for (block = 0; block < num_blocks; block++) {
		int rv;
		if (Verbosity > 2)
			tst_resm(TINFO, "Block: %lld/%lld  (%3lld%%)\r",
				 (long long int)block,
				 (long long int)num_blocks,
				 (long long int)(block * 100 / num_blocks));
		buf_fill(buf);
		rv = write(fd, buf, BLOCKSIZE);
		if (rv != BLOCKSIZE) {
			ret = -1;
			break;
		}
	}
	if (Verbosity > 2)
		tst_resm(TINFO, "Block: %lld/%lld  (%3lld%%)\r",
			 (long long int)block, (long long int)num_blocks,
			 (long long int)(block * 100 / num_blocks));
	close(fd);
	return (ret);
}

int verify_file(off_t num_blocks, const char *filename)
{
	int fd;
	int ret = 0;
	off_t block;
	uint8_t buf_actual[BLOCKSIZE];
	char buf_read[BLOCKSIZE];

	fd = open(filename, O_RDONLY);
	if (fd < 0) {
		perror(TCID);
		return (-1);
	}
	for (block = 0; block < num_blocks; block++) {
		int rv;
		int n;
		if (Verbosity > 2)
			tst_resm(TINFO, "Block: %lld/%lld  (%3lld%%)\r",
				 (long long int)block,
				 (long long int)num_blocks,
				 (long long int)(block * 100 / num_blocks));
		buf_fill(buf_actual);
		rv = read(fd, buf_read, BLOCKSIZE);
		if (rv != BLOCKSIZE) {
			ret = -1;
			break;
		}
		for (n = 0; n < BLOCKSIZE; n++) {
			int ba, br;
			ba = buf_actual[n] & 0xff;
			br = buf_read[n] & 0xff;
			if (ba != br) {
				if (Verbosity > 2)
					tst_resm(TINFO,
						 "Mismatch: block=%lld +%d bytes offset=%lld read: %02xh actual: %02xh\n",
						 (long long int)block, n,
						 (long long
						  int)((block * BLOCKSIZE) + n),
						 br, ba);
				ret++;
			}
		}
	}
	close(fd);
	if (Verbosity > 2)
		tst_resm(TINFO, "Block: %lld/%lld  (%3lld%%)\r",
			 (long long int)block, (long long int)num_blocks,
			 (long long int)(block * 100 / num_blocks));
	return (ret);
}

void usage(void)
{
	printf("%s [-v] [-b blocks] [-s seed] [-o filename]\n", TCID);
	printf("\n"
	       "   -v          - increase verbosity level\n"
	       "   blocks      - number of blocks to write\n"
	       "   seed        - seed to use (0 to use timestamp)\n"
	       "   filename    - name of output file\n");
}

void parse_args(int argc, char **argv)
{
	int c;
	TCID = argv[0];

	while (1) {
		int option_index = 0;
		static struct option long_options[] = {
			{"blocks", 1, 0, 'b'},
			{"out", 1, 0, 'o'},
			{"seed", 1, 0, 's'},
			{"verbose", 0, 0, 'v'},
			{"help", 0, 0, 'h'},
			{0, 0, 0, 0}
		};
		c = getopt_long(argc, argv, "hvb:o:s:", long_options,
				&option_index);
		if (c == -1)
			break;
		switch (c) {
		case 'b':
			NumBlocks = strtoul(optarg, NULL, 0);
			break;
		case 'o':
			strncpy(Filename, optarg, MAX_FILENAME_LEN);
			break;
		case 's':
			DefaultSeed = strtoul(optarg, NULL, 0);
			break;
		case 'v':
			Verbosity++;
			break;
		case 'h':
		default:
			usage();
			exit(-1);
		}
	}
}

void setup()
{
	tst_tmpdir();

}

void cleanup(void)
{
	tst_rmdir();
	tst_exit();
}

int main(int argc, char *argv[])
{
	int rv;

	setup();

	DefaultSeed = time(NULL);
	parse_args(argc, argv);
	tst_resm(TINFO, "Blocks:       %lld\n", (long long int)NumBlocks);
	tst_resm(TINFO, "Seed:         %d\n", DefaultSeed);
	tst_resm(TINFO, "Output file: '%s'\n", Filename);

	tst_resm(TINFO, "Writing %lld blocks of %d bytes to '%s'\n",
		 (long long int)NumBlocks, BLOCKSIZE, Filename);
	buf_init();
	rv = write_file(NumBlocks, Filename);
	if (rv == 0) {
		tst_resm(TPASS, "Write: Success");
	} else {
		tst_resm(TFAIL, "Write: Failure");
	}

	tst_resm(TINFO, "Verifying %lld blocks in '%s'\n",
		 (long long int)NumBlocks, Filename);
	buf_init();
	rv = verify_file(NumBlocks, Filename);
	if (rv == 0) {
		tst_resm(TPASS, "Verify: Success\n");
	} else {
		tst_resm(TFAIL, "Verify: Failure");
		tst_resm(TINFO, "Total mismatches: %d bytes\n", rv);
	}

	cleanup();
	return 0;
}

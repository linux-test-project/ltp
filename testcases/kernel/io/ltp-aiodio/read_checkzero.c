/*
 * Copyright (c) 2004 Daniel McNeil <daniel@osdl.org>
 *               2004 Open Source Development Lab
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
 *
 * Module: .c
 */

/*
 * Change History:
 *
 * 2/2004  Marty Ridgeway (mridge@us.ibm.com) Changes to adapt to LTP
 *
 */
#define _GNU_SOURCE

#include <stdlib.h>
#include <sys/types.h>
#include <signal.h>
#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>

char *check_zero(unsigned char *buf, int size)
{
	unsigned char *p;

	p = buf;

	while (size > 0) {
		if (*buf != 0) {
			fprintf(stderr, "non zero buffer at buf[%d] => 0x%02x,%02x,%02x,%02x\n",
				buf - p, (unsigned int)buf[0],
				size > 1 ? (unsigned int)buf[1] : 0,
				size > 2 ? (unsigned int)buf[2] : 0,
				size > 3 ? (unsigned int)buf[3] : 0);
			fprintf(stderr, "buf %p, p %p\n", buf, p);
			return buf;
		}
		buf++;
		size--;
	}
	return 0;	/* all zeros */
}
int read_eof(char *filename)
{
	int fd;
	int i;
	int r;
	char buf[4096];

       if ((fd = open(filename, O_RDWR)) < 0) {
               fprintf(stderr, "can't open file %s \n",filename);
               exit(1);
	}

	for (i = 0 ; i < 100000; i++) {
		off_t offset;
		char *bufoff;

               offset = lseek(fd, 4096, SEEK_END);
               r = write(fd,"A",1);

               offset = lseek(fd, offset - 4096 , SEEK_SET);

		r = read(fd, buf, 4096);
		if (r > 0) {
			if ((bufoff = check_zero(buf, r))) {
				fprintf(stderr, "non-zero read at offset %p\n",
					offset + bufoff);
				exit(1);
			}
		}
	}
	fprintf(stderr, "read_checkzero done\n");
  return 0;
}

int main(int argc, char **argv)
{
    if (argc < 2) {
        printf("You must pass a filename to the test \n");
        exit(1);
    }

	char *filename = argv[1];

	read_eof(filename);

  return 0;
}

/*
 * Written by Serge E. Hallyn <serue@us.ibm.com>
 * Copyright (c) International Business Machines  Corp., 2005
 *
 * This program is free software;  you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY;  without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See
 * the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program;  if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 */

#include <stdio.h>
#include <stdlib.h>

int usage(char *prog)
{
	printf("Usage: %s [-r] file offset count\n", prog);
	exit(1);
}

int main(int argc, char *argv[])
{
	int reverse = 0, count;
	FILE *fin;
	char c;

	if (argc > 1 && strcmp(argv[1], "-r")==0) {
		reverse = 1;
		argc--;
		argv++;
	}

	if (argc < 4)
		usage(argv[0]);

	fin = fopen(argv[1], "r+");

	if (!fin) {
		printf("Could not open %s for writing\n", argv[1]);
		exit(1);
	}

	count = atoi(argv[2]);

	if (count < 0) {
		printf("Bad count value: %d\n", count);
		fclose(fin);
		exit(1);
	}

	count += atoi(argv[3]);

	fseek(fin, count, SEEK_SET);
	fread(&c, 1, 1, fin);
	if (reverse)
		c += 1;
	else
		c -= 1;
	fseek(fin, -1, SEEK_CUR);
	fwrite(&c, 1, 1, fin);

	fclose(fin);

	return 0;
}

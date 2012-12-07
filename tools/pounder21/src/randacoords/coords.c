/* Generate window size/position coordinates for -geometry switches */

/*
 * Copyright (C) 2003-2006 IBM
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA
 * 02111-1307, USA.
 */

#include <unistd.h>
#include <limits.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>

int seed_random(void)
{
	int fp;
	long seed;

	fp = open("/dev/urandom", O_RDONLY);
	if (fp < 0) {
		perror("/dev/urandom");
		return 0;
	}

	if (read(fp, &seed, sizeof(seed)) != sizeof(seed)) {
		perror("read random seed");
		return 0;
	}

	close(fp);
	srand(seed);

	return 1;
}

int main(int argc, char *argv[])
{
	unsigned long xmax, ymax;
	unsigned long x, y;

	if (argc < 3) {
		fprintf(stderr, "Usage: %s xmax ymax\n", argv[0]);
		return 2;
	}

	xmax = atoi(argv[1]);
	ymax = atoi(argv[2]);

	if (!seed_random()) {
		return 1;
	}

	x = 1 + (unsigned long)((float)xmax * (rand() / (RAND_MAX + 1.0f)));
	y = 1 + (unsigned long)((float)ymax * (rand() / (RAND_MAX + 1.0f)));

	printf("+%lu+%lu\n", x, y);

	return 0;
}

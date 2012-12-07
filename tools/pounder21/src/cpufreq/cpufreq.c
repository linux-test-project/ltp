/**
 * Program to exercise CPU frequency switching via sysfs.
 * You probably want to turn on userspace switching and disable
 * powernowd/cpuspeed/powersaved programs.
 */

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

#include <stdio.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <stdlib.h>

static unsigned int cpunum = 0;

static int check_writable(const char *fname)
{
	int fd;

	fd = open(fname, O_WRONLY);
	if (fd >= 0)
		close(fd);

	return fd >= 0;
}

static int seed_random(void)
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

static unsigned int get_randnum(unsigned int max)
{
	return (unsigned int)((float)max * (rand() / (RAND_MAX + 1.0)));
}

static int set_cpuspeed(const char *ctrlfile, unsigned int speed)
{
	int fd, x;
	unsigned int y;
	char buf[256];

	/* First try to write a new speed. */
	fd = open(ctrlfile, O_WRONLY);
	if (fd < 0) {
		perror(ctrlfile);
		return 0;
	}

	printf("CPU %d speed set to %u kHz.\n", cpunum, speed);
	fflush(stdout);

	x = snprintf(buf, 256, "%u\n", speed);
	x = write(fd, buf, x);

	if (x == 0) {
		perror("Setting new speed");
		close(fd);
		return 0;
	}
	close(fd);

	/* Sleep for a while */
	usleep(500000);

	/* Now try to read the speed */
	fd = open(ctrlfile, O_RDONLY);
	if (fd < 0) {
		perror(ctrlfile);
		return 0;
	}

	x = read(fd, buf, 256);
	if (x == 0) {
		perror("Reading speed");
		close(fd);
		return 0;
	}
	close(fd);

	y = atoi(buf);
	if (y != speed) {
		printf("ERROR: Set CPU %d speed to %u but speed is now %u!\n",
		       cpunum, speed, y);
		fflush(stdout);
		return -1;
	}

	return 1;
}

int main(int argc, char *argv[])
{
	const char *ctrl;
	unsigned int rounds;
	unsigned int *frequencies;
	int y;
	unsigned int x, num_freqs;
	int ret = 0;

	/* Usage: cpufreq control_file rounds [frequencies...] */
	if (argc < 6) {
		printf
		    ("Usage: %s control_file rounds cpunum [frequencies...]\n",
		     argv[0]);
		ret = 1;
		goto out;
	}

	/* copy command line args */
	ctrl = argv[1];
	if (!check_writable(ctrl)) {
		perror(ctrl);
		ret = 2;
		goto out;
	}

	rounds = atoi(argv[2]);
	cpunum = atoi(argv[3]);

	num_freqs = argc - 4;
	frequencies = calloc(num_freqs, sizeof(unsigned int));
	if (frequencies == NULL) {
		perror("Error allocating memory");
		ret = 3;
		goto out;
	}

	for (x = 4; x < argc; x++) {
		frequencies[x - 4] = atoi(argv[x]);
	}

	/* Now run program. */
	printf("Running %u loops with these %d frequencies:\n", rounds,
	       num_freqs);
	for (x = 0; x < num_freqs; x++) {
		printf("%u KHz\n", frequencies[x]);
	}

	fflush(stdout);

	seed_random();

	for (x = rounds; x > 0; x--) {
		y = get_randnum(num_freqs);
		y = set_cpuspeed(ctrl, frequencies[y]);
		if (y != 1) {
			ret = 4;
			goto out;
		}
	}

out:
	printf("Exiting with return code %d.\n", ret);
	fflush(stdout);
	return ret;
}

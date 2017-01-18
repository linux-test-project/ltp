/******************************************************************************/
/*                                                                            */
/* Copyright (c) International Business Machines  Corp., 2007                 */
/* Copyright (c) Linux Test Project, 2016                                     */
/*                                                                            */
/* This program is free software: you can redistribute it and/or modify       */
/* it under the terms of the GNU General Public License as published by       */
/* the Free Software Foundation, either version 3 of the License, or          */
/* (at your option) any later version.                                        */
/*                                                                            */
/* This program is distributed in the hope that it will be useful,            */
/* but WITHOUT ANY WARRANTY; without even the implied warranty of             */
/* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the              */
/* GNU General Public License for more details.                               */
/*                                                                            */
/* You should have received a copy of the GNU General Public License          */
/* along with this program. If not, see <http://www.gnu.org/licenses/>.       */
/*                                                                            */
/******************************************************************************/

/******************************************************************************/
/*                                                                            */
/* File:        support_numa.c                                                */
/*                                                                            */
/* Description: Allocates 1MB of memory and touches it to verify numa         */
/*                                                                            */
/* Author:      Sivakumar Chinnaiah  Sivakumar.C@in.ibm.com                   */
/*                                                                            */
/******************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>
#include <signal.h>
#include <limits.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <fcntl.h>

/* Global Variables */
#define MB (1<<20)
#define PAGE_SIZE getpagesize()
#define barrier() __asm__ __volatile__("": : :"memory")
#define TEST_SFILE "ltp_numa_testfile"
#define STR "abcdefghijklmnopqrstuvwxyz12345\n"

static void help(void)
{
	printf("Input:	Describe input arguments to this program\n");
	printf("	argv[1] == 1 then allocate 1MB of memory\n");
	printf("	argv[1] == 2 then allocate 1MB of share memory\n");
	printf("	argv[1] == 3 then pause the program to catch sigint\n");
	printf("Exit:	On failure - Exits with non-zero value\n");
	printf("	On success - exits with 0 exit value\n");

	exit(1);
}

int main(int argc, char *argv[])
{
	int i, fd, rc;
	char *buf = NULL;
	struct stat sb;

	if (argc != 2) {
		fprintf(stderr, "Here expect only one number(i.e. 2) as the parameter\n");
		exit(1);
	}

	switch (atoi(argv[1])) {
	case 1:
		buf = malloc(MB);
		if (!buf) {
			fprintf(stderr, "Memory is not available\n");
			exit(1);
		}
		for (i = 0; i < MB; i += PAGE_SIZE) {
			buf[i] = 'a';
			barrier();
		}

		raise(SIGSTOP);

		free(buf);
		break;
	case 2:
		fd = open(TEST_SFILE, O_RDWR | O_CREAT, 0666);
		/* Writing 1MB of random data into this file [32 * 32768 = 1024 * 1024] */
		for (i = 0; i < 32768; i++){
			rc = write(fd, STR, strlen(STR));
			if (rc == -1 || ((size_t)rc != strlen(STR)))
				fprintf(stderr, "write failed\n");
		}

		if ((fstat(fd, &sb)) == -1)
			fprintf(stderr, "fstat failed\n");

		buf = mmap(NULL, sb.st_size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
		if (buf == MAP_FAILED){
			fprintf(stderr, "mmap failed\n");
			close(fd);
			exit(1);
		}

		memset(buf, 'a', sb.st_size);

		raise(SIGSTOP);

		munmap(buf, sb.st_size);
		close(fd);
		remove(TEST_SFILE);
		break;
	case 3:
		raise(SIGSTOP);
		break;
	default:
		help();
	}

	return 0;
}

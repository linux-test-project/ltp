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
/* Description: Allocates memory and touches it to verify numa                */
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
#include "lapi/mmap.h"

/* Global Variables */
#define MB (1<<20)
#define PAGE_SIZE getpagesize()
#define barrier() __asm__ __volatile__("": : :"memory")
#define TEST_SFILE "ltp_numa_testfile"
#define STR "abcdefghijklmnopqrstuvwxyz12345\n"

static void help(void)
{
	printf("Input:	Describe input arguments to this program\n");
	printf("	argv[1] == \"alloc_1MB\" then allocate 1MB of memory\n");
	printf("	argv[1] == \"alloc_2HPSZ_THP\" then allocate 2HUGE PAGE SIZE of THP memory\n");
	printf("        argv[1] == \"alloc_1huge_page\" then allocate 1HUGE PAGE SIZE of memory\n");
	printf("        argv[1] == \"pause\" then pause the program to catch sigint\n");
	printf("Exit:	On failure - Exits with non-zero value\n");
	printf("	On success - exits with 0 exit value\n");

	exit(1);
}

static int read_hugepagesize(void)
{
	FILE *fp;
	char line[BUFSIZ], buf[BUFSIZ];
	int val;

	fp = fopen("/proc/meminfo", "r");
	if (fp == NULL) {
		fprintf(stderr, "Failed to open /proc/meminfo");
		return 0;
	}

	while (fgets(line, BUFSIZ, fp) != NULL) {
		if (sscanf(line, "%64s %d", buf, &val) == 2)
			if (strcmp(buf, "Hugepagesize:") == 0) {
				fclose(fp);
				return 1024 * val;
			}
	}

	fclose(fp);
	fprintf(stderr, "can't find \"%s\" in %s", "Hugepagesize:", "/proc/meminfo");

	return 0;
}

int main(int argc, char *argv[])
{
	int i, hpsz;
	char *buf = NULL;

	if (argc != 2) {
		fprintf(stderr, "Here expect only one number(i.e. 2) as the parameter\n");
		exit(1);
	}

	if (!strcmp(argv[1], "alloc_1MB")) {
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
	} else if (!strcmp(argv[1], "alloc_2HPSZ_THP")) {
		ssize_t size = 2 * read_hugepagesize();
		if (size == 0)
			exit(1);

		buf = mmap(NULL, size, PROT_READ | PROT_WRITE,
				MAP_PRIVATE | MAP_ANONYMOUS,
				-1, 0);
		if (buf == MAP_FAILED) {
			perror("mmap failed");
			exit(1);
		}

		memset(buf, 'a', size);

		raise(SIGSTOP);

		munmap(buf, size);
	} else if (!strcmp(argv[1], "alloc_1huge_page")) {
		hpsz = read_hugepagesize();
		if (hpsz == 0)
			exit(1);

		buf = mmap(NULL, hpsz, PROT_READ | PROT_WRITE,
				MAP_PRIVATE | MAP_ANONYMOUS | MAP_HUGETLB,
				-1, 0);

		if (buf == MAP_FAILED) {
			perror("mmap failed");
			exit(1);
		}

		memset(buf, 'a', hpsz);

		raise(SIGSTOP);

		munmap(buf, hpsz);
	} else if (!strcmp(argv[1], "pause")) {
		raise(SIGSTOP);
	} else {
		help();
	}

	return 0;
}

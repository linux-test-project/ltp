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

/* Global Variables */
#define MB (1<<20)
#define PAGE_SIZE getpagesize()
#define barrier() __asm__ __volatile__("": : :"memory")

static void sigfunc(__attribute__ ((unused)) int sig)
{
}

static void help(void)
{
	printf("Input:	Describe input arguments to this program\n");
	printf("	argv[1] == 1 then print pagesize\n");
	printf("	argv[1] == 2 then allocate 1MB of memory\n");
	printf("	argv[1] == 3 then pause the program to catch sigint\n");
	printf("Exit:	On failure - Exits with non-zero value\n");
	printf("	On success - exits with 0 exit value\n");

	exit(1);
}

int main(int argc, char *argv[])
{
	int i;
	char *buf = NULL;
	struct sigaction sa;

	if (argc != 2) {
		fprintf(stderr, "Here expect only one number(i.e. 2) as the parameter.\n");
		exit(1);
	}

	switch (atoi(argv[1])) {
	case 1:
		printf("%d", PAGE_SIZE);
		return 0;
	case 2:
		buf = malloc(MB);
		if (!buf) {
			fprintf(stderr, "Memory is not available\n");
			exit(2);
		}
		for (i = 0; i < MB; i += PAGE_SIZE) {
			buf[i] = 'a';
			barrier();
		}
		free(buf);
		return 0;
	case 3:
		/* Trap SIGINT */
		sa.sa_handler = sigfunc;
		sa.sa_flags = SA_RESTART;
		sigemptyset(&sa.sa_mask);
		if (sigaction(SIGINT, &sa, 0) < 0) {
			fprintf(stderr, "Sigaction SIGINT failed\n");
			exit(3);
		}
		/* wait for signat Int */
		pause();
		return 0;
	default:
		help();
	}

	return 0;
}

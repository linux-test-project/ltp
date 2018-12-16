/******************************************************************************/
/*                                                                            */
/* Copyright (c) International Business Machines  Corp., 2008                 */
/*                                                                            */
/* This program is free software;  you can redistribute it and/or modify      */
/* it under the terms of the GNU General Public License as published by       */
/* the Free Software Foundation; either version 2 of the License, or          */
/* (at your option) any later version.                                        */
/*                                                                            */
/* This program is distributed in the hope that it will be useful,            */
/* but WITHOUT ANY WARRANTY;  without even the implied warranty of            */
/* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See                  */
/* the GNU General Public License for more details.                           */
/*                                                                            */
/* You should have received a copy of the GNU General Public License          */
/* along with this program;  if not, write to the Free Software               */
/* Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA    */
/*                                                                            */
/******************************************************************************/

/******************************************************************************/
/*                                                                            */
/* File:        memctl_test01.c                                               */
/*                                                                            */
/* Description: This is a c program that allocates memory in chunks of size   */
/*              as given by the calling script. The program touches all the   */
/*              allocated pages by writing a string on each page.             */
/*                                                                            */
/* Total Tests: 3                                                             */
/*                                                                            */
/* Test Name:   mem_controller_test01-03                                      */
/*                                                                            */
/*                                                                            */
/* Test Assertion                                                             */
/*              Please refer to the file memctl_testplan.txt                  */
/*                                                                            */
/* Author:      Sudhir Kumar skumar@linux.vnet.ibm.com                        */
/*                                                                            */
/* History:                                                                   */
/* Created      12/03/2008  Sudhir Kumar <skumar@linux.vnet.ibm.com>          */
/* Modified     11/05/2008  Sudhir Kumar <skumar@linux.vnet.ibm.com>          */
/*                                                                            */
/******************************************************************************/

#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include "libcontrollers.h"
#include "test.h"

char *TCID = "memory_controller_test01-03";
int TST_TOTAL = 3;

pid_t scriptpid;
typedef size_t record_t;
record_t **array_of_chunks;
record_t tmp;
int num_of_chunks, chunk_size, test_num, limit;

void cleanup();
void signal_handler_sigusr1(int signal);
void signal_handler_sigusr2(int signal);
int allocate_memory(void);

int main(void)
{
	int ret;
	char mygroup[FILENAME_MAX], mytaskfile[FILENAME_MAX];
	char *mygroup_p, *script_pid_p, *test_num_p, *chunk_size_p;
	char *num_chunks_p;
	struct sigaction newaction1, newaction2, oldaction1, oldaction2;

	/* Capture variables from the script environment */
	test_num_p = getenv("TEST_NUM");
	mygroup_p = getenv("MYGROUP");
	script_pid_p = getenv("SCRIPT_PID");
	chunk_size_p = getenv("CHUNK_SIZE");
	num_chunks_p = getenv("NUM_CHUNKS");

	if (test_num_p != NULL && mygroup_p != NULL && script_pid_p != NULL &&
	    chunk_size_p != NULL && num_chunks_p != NULL) {
		scriptpid = atoi(script_pid_p);
		test_num = atoi(test_num_p);
		chunk_size = atoi(chunk_size_p);
		num_of_chunks = atoi(num_chunks_p);
		sprintf(mygroup, "%s", mygroup_p);
	} else {
		tst_brkm(TBROK, cleanup,
			 "invalid parameters received from script\n");
	}

	/* XXX (garrcoop): this section really needs error handling. */

	/* Signal handling for SIGUSR1 received from script */
	sigemptyset(&newaction1.sa_mask);
	newaction1.sa_handler = signal_handler_sigusr1;
	newaction1.sa_flags = 0;
	sigaction(SIGUSR1, &newaction1, &oldaction1);

	/* Signal handling for SIGUSR2 received from script */
	sigemptyset(&newaction2.sa_mask);
	newaction2.sa_handler = signal_handler_sigusr2;
	newaction2.sa_flags = 0;
	sigaction(SIGUSR2, &newaction2, &oldaction2);

	sprintf(mytaskfile, "%s", mygroup);
	strcat(mytaskfile, "/tasks");
	/* Assign the task to it's group */
	write_to_file(mytaskfile, "a", getpid());	/* Assign the task to it's group */

	ret = allocate_memory();	/*should i check ret? */

	cleanup();

	tst_exit();
}

/*
 * Function: cleanup()
 * signals for system cleanup in case test breaks
 */
void cleanup(void)
{
	if (kill(scriptpid, SIGUSR1) == -1)
		tst_resm(TWARN | TERRNO, "kill failed");
}

/*
 * Function: signal_handler_sigusr1()
 * signal handler for the new action
 */

void signal_handler_sigusr1(int signal)
{
	int i;
	(void) signal;
	for (i = 0; i < num_of_chunks; ++i)
		free(array_of_chunks[i]);
	free(array_of_chunks);
	exit(0);
}

/*
 * Function: signal_handler_sigusr2()
 * signal handler for the new action
 */

void signal_handler_sigusr2(int signal)
{
	int i;
	(void) signal;
	for (i = 0; i < num_of_chunks; ++i)
		free(array_of_chunks[i]);
	free(array_of_chunks);
	if (test_num == 4) {
		/* Allocate different amount of memory for second step */
		chunk_size = 5242880;	/* 5 MB chunks */
		num_of_chunks = 15;
	}
	allocate_memory();
}

int allocate_memory(void)
{
	int i, j;
	/*
	 * Allocate array which contains base addresses of all chunks
	 */
	array_of_chunks = malloc(sizeof(record_t *) * num_of_chunks);
	if (array_of_chunks == NULL)
		tst_brkm(TBROK, cleanup,
			 "Memory allocation failed for array_of_chunks");
	/*
	 * Allocate chunks of memory
	 */

	for (i = 0; i < num_of_chunks; ++i) {
		array_of_chunks[i] = malloc(chunk_size);
		if (array_of_chunks[i] == NULL)
			tst_brkm(TBROK, cleanup,
				 "Memory allocation failed for chunks. Try smaller chunk size");
	}

	/*
	 * Touch all the pages of allocated memory by writing some string
	 */
	limit = chunk_size / sizeof(record_t);

	for (i = 0; i < num_of_chunks; ++i)
		for (j = 0; j < limit; ++j)
			array_of_chunks[i][j] = 0xaa;

	/*
	 * Just keep on accessing the allocated pages and do nothing relevant
	 */
	while (1) {
		for (i = 0; i < num_of_chunks; ++i)
			for (j = 0; j < limit; ++j)
				tmp = array_of_chunks[i][j];
	}
	return 0;
}

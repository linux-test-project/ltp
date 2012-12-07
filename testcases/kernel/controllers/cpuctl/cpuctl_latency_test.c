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
/* File:        cpuctl_latency_test.c                                         */
/*                                                                            */
/* Description: This is a c program that runs a task which tries to keep cpu  */
/*              busy in doing some calculations. The task will be used to     */
/*              check the latency under group scheduling.                     */
/*              The file is to be used by script                              */
/*                                                                            */
/* Total Tests: 2                                                             */
/*                                                                            */
/* Test Name:   cpu_controller_latency_tests                                  */
/*                                                                            */
/* Test Assertion                                                             */
/*              Please refer to the file cpuctl_testplan.txt                  */
/*                                                                            */
/* Author:      Sudhir Kumar skumar@linux.vnet.ibm.com                        */
/*                                                                            */
/* History:                                                                   */
/* Created-     26/11/2008 -Sudhir Kumar <skumar@linux.vnet.ibm.com>          */
/*                                                                            */
/******************************************************************************/

#include <errno.h>
#include <err.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "../libcontrollers/libcontrollers.h"

char *TCID = "cpu_controller_latency_tests";
int TST_TOTAL = 2;

void sighandler(int i)
{
	exit(0);
}

int main(int argc, char *argv[])
{
	char mytaskfile[FILENAME_MAX];
	int test_num;

	struct sigaction newaction, oldaction;

	/* TODO (garrcoop): add error handling. */
	sigemptyset(&newaction.sa_mask);
	sigaddset(&newaction.sa_mask, SIGUSR1);
	newaction.sa_handler = &sighandler;
	sigaction(SIGUSR1, &newaction, &oldaction);

	if (argc < 2 || argc > 3) {
		errx(EINVAL, "TBROK\t Invalid #args received from script"
		     " The test will run without any cpu load");
	}

	/* Migrate the task to its group if applicable */
	test_num = atoi(argv[1]);
	if (test_num < 0) {
		errx(EINVAL,
		     "Invalid test number received from script. "
		     "Skipping load creation");
	}

	if (test_num == 2) {
		strncpy(mytaskfile, argv[2], FILENAME_MAX);
		strncat(mytaskfile, "/tasks",
			FILENAME_MAX - strlen(mytaskfile) - 1);
		write_to_file(mytaskfile, "a", getpid());
	}

	while (1) ;

	return 0;
}

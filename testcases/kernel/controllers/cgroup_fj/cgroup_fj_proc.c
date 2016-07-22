/******************************************************************************/
/*                                                                            */
/* Copyright (c) 2009 FUJITSU LIMITED                                         */
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
/* Author: Shi Weihua <shiwh@cn.fujitsu.com>                                  */
/*                                                                            */
/******************************************************************************/

#include <sys/types.h>
#include <sys/wait.h>
#include <err.h>
#include <errno.h>
#include <signal.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>

#define UNUSED __attribute__ ((unused))

static int test_switch = 0;

void sighandler(UNUSED int signo)
{
	test_switch = !test_switch;
}

int main(void)
{
	sigset_t signalset;
	struct sigaction sa;
	pid_t pid;
	int status;
	int count = 0;

	sa.sa_handler = sighandler;
	if (sigemptyset(&sa.sa_mask) < 0)
		err(1, "sigemptyset()");

	sa.sa_flags = 0;
	if (sigaction(SIGUSR1, &sa, NULL) < 0)
		err(1, "sigaction()");

	if (sigemptyset(&signalset) < 0)
		err(1, "sigemptyset()");

	/* wait for the signal SIGUSR1 to start testing */
	sigsuspend(&signalset);
	if (errno != EINTR)
		err(1, "sigsuspend()");

	do {
		count++;
		pid = fork();
		if (pid == -1)
			err(1, "fork()");
		else if (pid == 0) {
			return 0;
		} else {
			wait(&status);
		}
	} while (test_switch);

	return 0;
}

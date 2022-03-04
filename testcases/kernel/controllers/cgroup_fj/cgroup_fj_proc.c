// SPDX-License-Identifier: GPL-2.0-or-later
// Copyright (c) 2009 FUJITSU LIMITED
// Author: Shi Weihua <shiwh@cn.fujitsu.com>

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

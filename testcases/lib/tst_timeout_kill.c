// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2021 Cyril Hrubis <chrubis@suse.cz>
 */

#include <stdio.h>
#include <signal.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>

static void print_help(const char *name)
{
	fprintf(stderr, "usage: %s timeout pid\n", name);
}

#define print_msg(fmt, ...) fprintf(stderr, fmt "\n", ##__VA_ARGS__)

int main(int argc, char *argv[])
{
	int timeout, pid, ret, i;

	if (argc != 3) {
		print_help(argv[0]);
		return 1;
	}

	timeout = atoi(argv[1]);
	pid = atoi(argv[2]);

	if (timeout < 0) {
		fprintf(stderr, "Invalid timeout '%s'\n", argv[1]);
		print_help(argv[0]);
		return 1;
	}

	if (pid <= 1) {
		fprintf(stderr, "Invalid pid '%s'\n", argv[2]);
		print_help(argv[0]);
		return 1;
	}

	ret = setpgid(0, 0);
	if (ret)
		print_msg("setpgid() failed: %s", strerror(errno));

	if (timeout)
		sleep(timeout);

	print_msg("Test timed out, sending SIGTERM!");
	print_msg("If you are running on slow machine, try exporting LTP_TIMEOUT_MUL > 1");

	ret = kill(-pid, SIGTERM);
	if (ret) {
		print_msg("kill(%i) failed: %s", -pid, strerror(errno));
		return 1;
	}

	usleep(100000);

	i = 10;

	while (!kill(-pid, 0) && i-- > 0) {
		print_msg("Test is still running... %i", i + 1);
		sleep(1);
	}

	if (!kill(-pid, 0)) {
		print_msg("Test is still running, sending SIGKILL");
		ret = kill(-pid, SIGKILL);
		if (ret) {
			print_msg("kill(%i) failed: %s", -pid, strerror(errno));
			return 1;
		}
	}

	return 0;
}

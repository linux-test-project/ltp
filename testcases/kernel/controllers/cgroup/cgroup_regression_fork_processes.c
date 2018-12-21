// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2009 FUJITSU LIMITED
 *
 * Author: Li Zefan <lizf@cn.fujitsu.com>
 */

#include <sys/types.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <unistd.h>

int main(void)
{
	int i;
	pid_t pid;

	while (1) {
		for (i = 0; i < 200; i++) {
			pid = fork();
			if (pid == 0) {
				exit(0);
			} else if (pid == -1) {
				continue;
			}
		}

		for (i = 0; i < 200; i++)
			if (wait(NULL) < 0)
				break;
	}

	return 0;
}

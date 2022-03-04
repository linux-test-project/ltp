// SPDX-License-Identifier: GPL-2.0-or-later
// Copyright (c) 2009 FUJITSU LIMITED
// Author: Li Zefan <lizf@cn.fujitsu.com>

#include <sys/mman.h>
#include <err.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <unistd.h>

#define MEM_SIZE	(1024 * 1024 * 100)

void sigusr_handler(int __attribute__ ((unused)) signo)
{
	char *p;

	p = mmap(NULL, MEM_SIZE, PROT_READ | PROT_WRITE,
		 MAP_PRIVATE | MAP_ANONYMOUS, 0, 0);
	if (p == MAP_FAILED) {
		err(1, "failed to allocate memory!");
	}

	memset(p, 'z', MEM_SIZE);
}

int main(void)
{
	struct sigaction sigusr_action;

	memset(&sigusr_action, 0, sizeof(sigusr_action));
	sigusr_action.sa_handler = &sigusr_handler;
	sigaction(SIGUSR1, &sigusr_action, NULL);

	while (1)
		sleep(1);

	return 0;
}

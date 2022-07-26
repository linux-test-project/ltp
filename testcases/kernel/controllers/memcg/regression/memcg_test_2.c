// SPDX-License-Identifier: GPL-2.0-or-later
// Copyright (c) 2009 FUJITSU LIMITED
// Author: Li Zefan <lizf@cn.fujitsu.com>

#include <sys/mman.h>
#include <signal.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

void sigusr_handler(int __attribute__ ((unused)) signo)
{
	char *p;
	int size = getpagesize() * 2;

	p = mmap(NULL, size, PROT_READ | PROT_WRITE,
		 MAP_PRIVATE | MAP_ANONYMOUS | MAP_LOCKED, 0, 0);
	if (p == MAP_FAILED)
		exit(1);
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

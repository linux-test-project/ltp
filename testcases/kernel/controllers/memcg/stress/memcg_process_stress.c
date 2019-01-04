// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2009 FUJITSU LIMITED
 * Author: Li Zefan <lizf@cn.fujitsu.com>
 */

#include <sys/mman.h>
#include <err.h>
#include <math.h>
#include <signal.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

int flag_exit;
int flag_ready;

int interval;
unsigned long memsize;

char **pages;
int nr_page;

void touch_memory(void)
{
	int i;

	for (i = 0; i < nr_page; i++)
		pages[i][0] = 0xef;
}

void sigusr_handler(int __attribute__ ((unused)) signo)
{
	int i;
	int pagesize;

	pagesize = getpagesize();

	nr_page = ceil((double)memsize / pagesize);

	pages = calloc(nr_page, sizeof(char *));
	if (pages == NULL)
		errx(1, "calloc failed");

	for (i = 0; i < nr_page; i++) {
		pages[i] = mmap(NULL, pagesize, PROT_WRITE | PROT_READ,
				MAP_PRIVATE | MAP_ANONYMOUS, 0, 0);
		if (pages[i] == MAP_FAILED)
			err(1, "map failed\n");
	}

	flag_ready = 1;
}

void sigint_handler(int __attribute__ ((unused)) signo)
{
	flag_exit = 1;
}

int main(int argc, char *argv[])
{
	char *end;
	struct sigaction sigint_action;
	struct sigaction sigusr_action;

	if (argc != 3)
		errx(1, "wrong argument num");

	memsize = strtoul(argv[1], &end, 10);
	if (*end != '\0')
		errx(1, "wrong memsize");
	memsize = memsize * 1024 * 1024;

	interval = atoi(argv[2]);
	if (interval <= 0)
		interval = 1;

	memset(&sigint_action, 0, sizeof(sigint_action));
	sigint_action.sa_handler = &sigint_handler;
	if (sigaction(SIGINT, &sigint_action, NULL))
		err(1, "sigaction(%s) failed", "SIGINT");

	memset(&sigusr_action, 0, sizeof(sigusr_action));
	sigusr_action.sa_handler = &sigusr_handler;
	if (sigaction(SIGUSR1, &sigusr_action, NULL))
		err(1, "sigaction(%s) failed", "SIGUSR1");

	while (!flag_exit) {
		sleep(interval);

		if (flag_ready)
			touch_memory();
	}

	return 0;
}

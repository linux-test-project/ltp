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
/* Author: Li Zefan <lizf@cn.fujitsu.com>                                     */
/*                                                                            */
/******************************************************************************/

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

void touch_memory()
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

	/* TODO (garrcoop): add error handling. */
	memset(&sigint_action, 0, sizeof(sigint_action));
	sigint_action.sa_handler = &sigint_handler;
	sigaction(SIGINT, &sigint_action, NULL);

	memset(&sigusr_action, 0, sizeof(sigusr_action));
	sigusr_action.sa_handler = &sigusr_handler;
	sigaction(SIGUSR1, &sigusr_action, NULL);

	while (!flag_exit) {
		sleep(interval);

		if (flag_ready)
			touch_memory();
	}

	return 0;
}

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
/* Author: Miao Xie <miaox@cn.fujitsu.com>                                    */
/*                                                                            */
/******************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <ctype.h>
#include <getopt.h>
#include <err.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include "../cpuset_lib/common.h"

#define BUFFER_SIZE 100

volatile int end;

void sighandler1(UNUSED int signo)
{
}

void sighandler2(UNUSED int signo)
{
	end = 1;
}

int page_cache_hog(void)
{
	int fd = -1;
	char buff[BUFFER_SIZE];
	char path[BUFFER_SIZE];
	int ret = 0;

	sprintf(path, "%s", "DATAFILE");
	fd = open(path, O_RDONLY);
	if (fd == -1) {
		warn("open %s failed", path);
		return -1;
	}

	while ((ret = read(fd, buff, sizeof(buff))) > 0) ;
	if (ret == -1)
		warn("read %s failed", path);

	close(fd);
	return ret;
}

int mem_hog(void)
{
	sigset_t signalset;
	int fd;
	int ret = 0;

	if (sigemptyset(&signalset) < 0)
		err(1, "sigemptyset()");
	sigsuspend(&signalset);

	while (!end) {
		ret = page_cache_hog();

		fd = open("./myfifo", O_WRONLY);
		if (fd == -1)
			err(1, "open fifo failed");

		if (ret) {
			if (write(fd, "0", 1) == -1)
				warn("write fifo failed.");
		} else {
			if (write(fd, "1", 1) == -1)
				warn("write fifo failed.");
		}

		close(fd);

		sigsuspend(&signalset);
	}

	return ret;
}

int main(void)
{
	struct sigaction sa1, sa2;

	sa1.sa_handler = sighandler1;
	if (sigemptyset(&sa1.sa_mask) < 0)
		err(1, "sigemptyset()");

	sa1.sa_flags = 0;
	if (sigaction(SIGUSR1, &sa1, NULL) < 0)
		err(1, "sigaction()");

	sa2.sa_handler = sighandler2;
	if (sigemptyset(&sa2.sa_mask) < 0)
		err(1, "sigemptyset()");

	sa2.sa_flags = 0;
	if (sigaction(SIGUSR2, &sa2, NULL) < 0)
		err(1, "sigaction()");

	return mem_hog();
}

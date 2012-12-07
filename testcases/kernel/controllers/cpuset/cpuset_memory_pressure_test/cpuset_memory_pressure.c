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

#include <sys/mman.h>
#include <err.h>
#include <errno.h>
#include <libgen.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

int unit = 1024 * 1024;

//argv[1] memory use(M) (>0)

int main(int argc, char *argv[])
{
	long int use;
	if (argc != 2) {
		fprintf(stderr, "usage: %s mmap-size-in-kB", basename(argv[0]));
		exit(1);
	}
	if ((use = strtol(argv[1], NULL, 10)) < 0) {
		errx(EINVAL, "Invalid mmap size specified (must be a long "
		     "int greater than 1)");
	}

	long int pagesize = getpagesize();
	long int mmap_block = use * unit / 10 / pagesize;
	long int total_block = 0;

	while (pagesize * mmap_block > 2 * unit) {
		unsigned long *addr = mmap(NULL, pagesize * mmap_block,
					   PROT_READ | PROT_WRITE,
					   MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
		if (addr == MAP_FAILED) {
			mmap_block = mmap_block / 2;
			continue;
		}
		memset(addr, 0xF7, pagesize * mmap_block);
		total_block += mmap_block;
		if (total_block * pagesize >= use * unit)
			break;
	}
	return 0;
}

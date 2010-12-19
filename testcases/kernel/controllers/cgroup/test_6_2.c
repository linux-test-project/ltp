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
/* Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA    */
/*                                                                            */
/* Author: Li Zefan <lizf@cn.fujitsu.com>                                     */
/*                                                                            */
/******************************************************************************/

#include <sched.h>
#include <stdlib.h>
#include <unistd.h>
#include "test.h"

#define DEFAULT_USEC	30000

int foo(void __attribute__((unused)) *arg)
{
	return 0;
}

int main(int argc, char **argv)
{
	int usec;

	if (argc == 2)
		usec = atoi(argv[1]);
	else
		usec = DEFAULT_USEC;

	while (1) {
		usleep(usec);
		ltp_clone_quick(CLONE_NEWNS, foo, NULL);
	}

	tst_exit();
}
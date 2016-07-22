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
/* Restructure for LTP: Shi Weihua <shiwh@cn.fujitsu.com>                     */
/*                                                                            */
/******************************************************************************/

#include <stdio.h>
#include <math.h>
#include <err.h>
#include <errno.h>
#include <signal.h>

#define __USE_GNU
#include <sched.h>

#define UNUSED __attribute__ ((unused))

unsigned long count;
volatile int start = 0;
volatile double f = 2744545.34456455;

void sighandler(UNUSED int signo)
{
	start = !start;
}

int main(void)
{
	sigset_t signalset;
	struct sigaction sa;

	sa.sa_handler = sighandler;
	if (sigemptyset(&sa.sa_mask) < 0)
		err(1, "sigemptyset()");

	sa.sa_flags = 0;
	if (sigaction(SIGUSR1, &sa, NULL) < 0)
		err(1, "sigaction()");

	if (sigemptyset(&signalset) < 0)
		err(1, "sigemptyset()");

	sigsuspend(&signalset);
	if (errno != EINTR)
		err(1, "sigsuspend()");

	while (start) {
		f = sqrt(f * f);
	}

	return 0;
}

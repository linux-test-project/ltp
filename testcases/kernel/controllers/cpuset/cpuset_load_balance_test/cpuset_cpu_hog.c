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

#include "config.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <math.h>
#include <err.h>
#include <errno.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <fcntl.h>

char *TCID = "cpuset_cpu_hog";
int TST_TOTAL = 1;

#if HAVE_LINUX_MEMPOLICY_H

#include "../cpuset_lib/common.h"
#include "../cpuset_lib/bitmask.h"
#include "../cpuset_lib/cpuset.h"

#define MAX_NPROCS	1000
#define USAGE	("Usage: %s [-p nprocs] [-h]\n"		\
		 "\t-p nprocs\n"					\
		 "\t\tThe num of the procs. [Default = 2 * nr_cpus]\n"	\
		 "\t-h\tHelp.\n")

static int nprocs;
static volatile int end;

/*
 * report executing result to the parent by fifo
 *     "0\n" - everything is OK
 *     "1\n" - everything is OK, but break the test
 *     "2\n" - something failed
 */
int report_result(char str[])
{
	int fd;

	fd = open("./myfifo", O_WRONLY);
	if (fd == -1) {
		warn("open fifo failed");
		return -1;
	}

	if (write(fd, str, strlen(str)) == -1) {
		warn("write fifo failed.");
		close(fd);
		return -1;
	}

	close(fd);
	return 0;
}

void sighandler1(UNUSED int signo)
{
}

void sighandler2(UNUSED int signo)
{
	end = 1;
}

void usage(char *prog_name, int status)
{
	FILE *output = NULL;

	if (prog_name == NULL)
		prog_name = "cpu-hog";

	if (status)
		output = stderr;
	else
		output = stdout;

	fprintf(output, USAGE, prog_name);

	if (status)
		report_result("2\n");
	else
		report_result("1\n");

	exit(status);
}

void checkopt(int argc, char **argv)
{
	char c = '\0';
	char *endptr = NULL;
	long nr_cpus = 0;
	long opt_value = 0;

	nr_cpus = sysconf(_SC_NPROCESSORS_ONLN);
	if (nr_cpus <= 0) {
		fprintf(stderr, "Error: sysconf failed\n");
		report_result("2\n");
		exit(1);
	}

	while ((c = getopt(argc, argv, "p:h")) != -1) {
		switch (c) {
		case 'p':
			if (optarg[0] == '-' && !isdigit(optarg[1]))
				OPT_MISSING(argv[0], c);
			else {
				opt_value = strtol(optarg, &endptr, DECIMAL);
				if (errno || (endptr != NULL && *endptr != '\0')
				    || opt_value <= 0 || opt_value > MAX_NPROCS)
					ARG_WRONG(argv[0], c, optarg);
				nprocs = atoi(optarg);
			}
			break;
		case 'h':	/* usage message */
			usage(argv[0], 0);
			break;
		default:
			usage(argv[0], 1);
			break;
		}
	}

	if (nprocs == 0)
		nprocs = 2 * nr_cpus;
}

/*
 * hog the cpu time and check the cpu which the task is running on is in the
 * cpus of the cpuset or not.
 *
 * return value: 0  - success.
 *               1  - the cpu which the task is running on isn't in the cpus
 *                    of the cpuset.
 *               -1 - failure for other reason.
 */
int cpu_hog(void)
{
	double f = 2744545.34456455;
	sigset_t signalset;
	struct cpuset *cp = NULL;
	struct bitmask *cpumask = NULL;
	int cpu;
	int nbits;
	int ret = 0;

	nbits = cpuset_cpus_nbits();

	cp = cpuset_alloc();
	if (cp == NULL)
		return -1;

	cpumask = bitmask_alloc(nbits);
	if (cpumask == NULL) {
		ret = -1;
		goto err1;
	}

	if (sigemptyset(&signalset) < 0) {
		ret = -1;
		goto err2;
	}

	sigsuspend(&signalset);

	if (cpuset_cpusetofpid(cp, 0) < 0) {
		ret = -1;
		goto err2;
	}
	if (cpuset_getcpus(cp, cpumask) != 0) {
		ret = -1;
		goto err2;
	}

	while (!end) {
		f = sqrt(f * f);
		cpu = cpuset_latestcpu(0);
		if (cpu < 0) {
			warn("get latest cpu failed.\n");
			ret = -1;
			goto err2;
		}
		if (!bitmask_isbitset(cpumask, cpu)) {
			char str[50];
			bitmask_displaylist(str, 50, cpumask);
			warn("the task(%d) is running on the cpu(%d) excluded"
			     " by cpuset(cpus: %s)\n", getpid(), cpu, str);
			ret = 1;
			goto err2;
		}
	}

err2:
	bitmask_free(cpumask);
err1:
	cpuset_free(cp);
	return ret;
}

int initialize(void)
{
	struct sigaction sa1, sa2;

	sa1.sa_handler = sighandler1;
	if (sigemptyset(&sa1.sa_mask) < 0)
		return -1;

	sa1.sa_flags = 0;
	if (sigaction(SIGUSR1, &sa1, NULL) < 0)
		return -1;

	sa2.sa_handler = sighandler2;
	if (sigemptyset(&sa2.sa_mask) < 0)
		return -1;

	sa2.sa_flags = 0;
	if (sigaction(SIGUSR2, &sa2, NULL) < 0)
		return -1;

	return 0;
}

int main(int argc, char **argv)
{
	int i = 0;
	pid_t pid;
	pid_t *childpids = NULL;
	sigset_t signalset;
	int status = 0;
	int ret = 0;

	checkopt(argc, argv);
	if (initialize()) {
		warn("initialize failed");
		report_result("2\n");
		exit(EXIT_FAILURE);
	}

	if (sigemptyset(&signalset) < 0) {
		warn("sigemptyset failed");
		report_result("2\n");
		exit(EXIT_FAILURE);
	}

	childpids = malloc((nprocs) * sizeof(pid_t));
	if (childpids == NULL) {
		warn("alloc for child pids failed");
		report_result("2\n");
		exit(EXIT_FAILURE);
	}
	memset(childpids, 0, (nprocs) * sizeof(pid_t));

	report_result("0\n");
	sigsuspend(&signalset);
	for (; i < nprocs; i++) {
		pid = fork();
		if (pid == -1) {
			while (--i >= 0)
				kill(childpids[i], SIGKILL);
			warn("fork test tasks failed");
			report_result("2\n");
			exit(EXIT_FAILURE);
		} else if (!pid) {
			ret = cpu_hog();
			exit(ret);
		}
		childpids[i] = pid;
	}

	report_result("0\n");

	while (!end) {
		if (sigemptyset(&signalset) < 0)
			ret = -1;
		else
			sigsuspend(&signalset);

		if (ret || end) {
			for (i = 0; i < nprocs; i++) {
				kill(childpids[i], SIGUSR2);
			}
			break;
		} else {
			for (i = 0; i < nprocs; i++) {
				kill(childpids[i], SIGUSR1);
			}
		}
	}
	for (i = 0; i < nprocs; i++) {
		wait(&status);
		if (status)
			ret = EXIT_FAILURE;
	}

	free(childpids);
	return ret;
}

#else /* ! HAVE_LINUX_MEMPOLICY_H */
int main(void)
{
	printf("System doesn't have required mempolicy support\n");
	return 1;
}
#endif

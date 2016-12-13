/*
 * Program that makes random system calls with random arguments.
 */

/*
 * Copyright (C) 2003-2006 IBM
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA
 * 02111-1307, USA.
 */

#include <signal.h>
#include <limits.h>
#include <strings.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include <stdio.h>
#include <sys/syscall.h>
#include <stdint.h>
#include <stdlib.h>

unsigned long callnum, args[6];

int seed_random(void)
{
	int fp;
	long seed;

	fp = open("/dev/urandom", O_RDONLY);
	if (fp < 0) {
		perror("/dev/urandom");
		return 0;
	}

	if (read(fp, &seed, sizeof(seed)) != sizeof(seed)) {
		perror("read random seed");
		return 0;
	}

	close(fp);
	srand(seed);

	return 1;
}

void get_big_randnum(void *buf, unsigned int size)
{
	uint32_t *x = buf;
	int i;

	for (i = 0; i < size; i += 4, x++) {
		*x = (unsigned long)((float)UINT_MAX *
				     (rand() / (RAND_MAX + 1.0)));
	}
}

unsigned long get_randnum(unsigned long min, unsigned long max)
{
	return min + (unsigned long)((float)max * (rand() / (RAND_MAX + 1.0)));
}

int find_syscall(void)
{
	int x;

badcall:
	x = get_randnum(0, 384);

	/* poorly implemented blacklist */
	switch (x) {
		/* don't screw with signal handling */
#ifdef SYS_signal
	case SYS_signal:
#endif
#ifdef SYS_sigaction
	case SYS_sigaction:
#endif
#ifdef SYS_sigsuspend
	case SYS_sigsuspend:
#endif
#ifdef SYS_sigpending
	case SYS_sigpending:
#endif
#ifdef SYS_sigreturn
	case SYS_sigreturn:
#endif
#ifdef SYS_sigprocmask
	case SYS_sigprocmask:
#endif
#ifdef SYS_rt_sigreturn
	case SYS_rt_sigreturn:
#endif
#ifdef SYS_rt_sigaction
	case SYS_rt_sigaction:
#endif
#ifdef SYS_rt_sigprocmask
	case SYS_rt_sigprocmask:
#endif
#ifdef SYS_rt_sigpending
	case SYS_rt_sigpending:
#endif
#ifdef SYS_rt_sigtimedwait
	case SYS_rt_sigtimedwait:
#endif
#ifdef SYS_rt_sigqueueinfo
	case SYS_rt_sigqueueinfo:
#endif
#ifdef SYS_rt_sigsuspend
	case SYS_rt_sigsuspend:
#endif
#ifdef SYS_sigaltstack
	case SYS_sigaltstack:
#endif
#ifdef SYS_settimeofday
	case SYS_settimeofday:
#endif

		/* don't exit the program :P */
#ifdef SYS_exit
	case SYS_exit:
#endif
#ifdef SYS_exit_group
	case SYS_exit_group:
#endif

		/* don't put it to sleep either */
#ifdef SYS_pause
	case SYS_pause:
#endif
#ifdef SYS_select
	case SYS_select:
#endif
#ifdef SYS_read
	case SYS_read:
#endif
#ifdef SYS_write
	case SYS_write:
#endif

		/* these can fill the process table */
#ifdef SYS_fork
	case SYS_fork:
#endif
#ifdef SYS_vfork
	case SYS_vfork:
#endif
#ifdef SYS_clone
	case SYS_clone:
#endif

		/* This causes OOM conditions */
#if 1
#ifdef SYS_brk
	case SYS_brk:
#endif
#endif

		/* these get our program killed */
#ifdef SYS_vm86
	case SYS_vm86:
#endif
#ifdef SYS_vm86old
	case SYS_vm86old:
#endif
		goto badcall;
	}

	return x;
}

void bogus_signal_handler(int signum)
{
	fprintf(stderr,
		"                                    Signal %d on syscall(%lu, 0x%lX, 0x%lX, 0x%lX, 0x%lX, 0x%lX, 0x%lX).\n",
		signum, callnum, args[0], args[1], args[2], args[3], args[4],
		args[5]);
}

void real_signal_handler(int signum)
{
	exit(0);
}

void install_signal_handlers(void)
{
	int x;
	struct sigaction zig;

	memset(&zig, 0x00, sizeof(zig));
	zig.sa_handler = bogus_signal_handler;
	for (x = 0; x < 64; x++) {
		sigaction(x, &zig, NULL);
	}

	zig.sa_handler = real_signal_handler;
	sigaction(SIGINT, &zig, NULL);
	sigaction(SIGTERM, &zig, NULL);
}

int main(int argc, char *argv[])
{
	int i;
	int debug = 0, zero_mode = 0;

	if (!seed_random()) {
		return 1;
	}

	for (i = 1; i < argc; i++) {
		if (!strcmp(argv[i], "-d"))
			debug = 1;
		else if (!strcmp(argv[i], "-z"))
			zero_mode = 1;
	}

	memset(args, 0, sizeof(unsigned long) * 6);

	install_signal_handlers();

	while (1) {
		callnum = find_syscall();
		if (!zero_mode)
			get_big_randnum(&args[0], sizeof(unsigned long) * 6);

		if (debug) {
			printf("syscall(%lu, 0x%lX, 0x%lX, 0x%lX, 0x%lX, "
			       "0x%lX, 0x%lX);       \n",
			       callnum, args[0], args[1], args[2], args[3],
			       args[4], args[5]);
			fflush(stdout);
		}

		syscall(callnum, args[0], args[1], args[2],
			args[3], args[4], args[5]);
	}

	return 0;
}

#
#  Copyright (c) International Business Machines  Corp., 2001
#
#  This program is free software;  you can redistribute it and/or modify
#  it under the terms of the GNU General Public License as published by
#  the Free Software Foundation; either version 2 of the License, or
#  (at your option) any later version.
#
#  This program is distributed in the hope that it will be useful,
#  but WITHOUT ANY WARRANTY;  without even the implied warranty of
#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See
#  the GNU General Public License for more details.
#
#  You should have received a copy of the GNU General Public License
#  along with this program;  if not, write to the Free Software
#  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
#

#########################################################################
# name of file	: Makefile						#
# description	: make(1) description for compiling the /ltp/testsuite	#
#		  tests. This Makefile would recursively call the 	#
#		  Makefiles inside each subdirectories, and, compiles	#
#		  and installs the tests.				#
#########################################################################
DEBUG_CFLAGS=-O
DEBUG_LDFLAGS=-s
CFLAGS+=-I../../include -g
LDFLAGS+=-L../../lib -lltp -L.. -ltestsuite

SUBDIR= \
	accept access alarm asyncio bind brk \
	chdir chmod chown chroot close connect \
	creat dup dup2 execl execle execlp \
	execv execve execvp exit fchdir fchmod \
	fchown fcntl fork fpathconf fstat fstatfs \
	fsync ftruncate getcwd getdents getegid geteuid \
	getgid getgroups gethostid gethostname getitimer getpeername \
	getpgid getpgrp getpid getppid getpriority getresgid \
	getresuid getsid getsockname getsockopt gettimeofday getuid \
	ioctl ipc kill lchown link \
	listen llseek lseek lstat \
	mkdir mknod mmap modify_ldt mprotect mremap \
	msync munmap nanosleep nice open \
	pathconf pause personality pipe poll \
	pread pwrite read readdir \
	readlink readv recv recvfrom recvmsg rename \
	rmdir sbrk sched_getscheduler sched_setscheduler sched_yield select \
	send sendfile sendmsg sendto setfsgid setfsuid \
	setgid setgroups setitimer setpgid setpgrp setpriority \
	setregid setresuid setreuid setrlimit setsid setsockopt \
	settimeofday setuid sigaction sigaltstack sighold signal \
	sigprocmask sigrelse sigsuspend socket socketpair sockioctl \
	stat statfs stime symlink sync sysctl \
	sysinfo time times truncate ulimit umask \
	uname unlink utime vfork vhangup wait \
	wait4 waitpid write writev


all:  $& $(SUBDIR)
	@for i in lib $(SUBDIR); do \
		(cd $$i; $(MAKE) all ) \
	done

install:
	@for i in lib $(SUBDIR); do \
		(cd $$i; $(MAKE) install) \
	done

clean:
	@for i in lib $(SUBDIR); do \
		(cd $$i; $(MAKE) clean) \
	done


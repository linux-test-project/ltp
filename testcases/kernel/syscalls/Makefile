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
CFLAGS+=-I../../include -g -Wall
LOADLIBES+=
EXCLUDE_DIR=epoll
#
# Commented this out since there are directories here we don't want built by default
#
#SUBDIR = `ls */Makefile | sed "s/Makefile//g"` 

SUBDIR = abort accept access acct adjtimex alarm asyncio bind brk capget capset chdir chmod chown \
chroot clone close confstr connect creat dup dup2 execl execle execlp execv  execve execvp exit \
fchdir fchmod fchown fcntl fdatasync flock fmtmsg fork fpathconf fstat fstatfs  fsync ftruncate \
getcwd getdents getdomainname getegid geteuid getgid getgroups gethostid gethostname getitimer \
getpeername getpgid getpgrp getpid getppid getpriority  getresgid getresuid getrlimit getrusage \
getsid  getsockname getsockopt  gettimeofday getuid ioctl ioperm  iopl ipc kill lchown lib \
link listen llseek lseek lstat mallopt  memcmp memcpy  memset mkdir mknod mlock mlockall mmap \
modify_ldt mount mprotect mremap msync munlock munlockall munmap nanosleep nftw nice open pathconf \
pause  personality pipe poll prctl pread profil ptrace pwrite read readdir readlink readv reboot \
recv recvfrom recvmsg rename rmdir sbrk sched_get_priority_max sched_get_priority_min sched_getparam \
sched_getscheduler sched_rr_get_interval sched_setparam sched_setscheduler sched_yield select send \
sendfile sendmsg sendto setdomainname setegid setfsgid setfsuid  setgid setgroups sethostname setitimer \
setpgid setpgrp setpriority setregid setresgid setresuid setreuid  setrlimit setsid setsockopt settimeofday \
setuid sigaction sigaltstack sighold signal sigpending sigprocmask sigrelse sigsuspend socket socketcall \
socketpair sockioctl stat statfs stime string swapoff swapon symlink sync syscall sysconf sysctl sysfs \
sysinfo syslog time times truncate ulimit umask umount uname unlink ustat utime vfork vhangup wait \
wait4 waitpid write writev


all:  
	@set -e; for i in lib $(SUBDIR); do \
		$(MAKE) -C $$i  all; \
	done

install:
	@set -e; for i in lib $(SUBDIR); do \
		$(MAKE) -C $$i  install; \
	done

clean:
	@set -e; for i in lib $(SUBDIR); do \
		$(MAKE) -C $$i  clean; \
	done


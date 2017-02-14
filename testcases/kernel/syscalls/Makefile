#
#    testcases/kernel/syscalls Makefile.
#
#    Copyright (C) 2009, Cisco Systems Inc.
#
#    This program is free software; you can redistribute it and/or modify
#    it under the terms of the GNU General Public License as published by
#    the Free Software Foundation; either version 2 of the License, or
#    (at your option) any later version.
#
#    This program is distributed in the hope that it will be useful,
#    but WITHOUT ANY WARRANTY; without even the implied warranty of
#    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#    GNU General Public License for more details.
#
#    You should have received a copy of the GNU General Public License along
#    with this program; if not, write to the Free Software Foundation, Inc.,
#    51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
#
# Ngie Cooper, July 2009
#

top_srcdir		?= ../../..

include	$(top_srcdir)/include/mk/env_pre.mk

ifeq ($(UCLINUX),1)
FILTER_OUT_DIRS	+= capget capset chmod chown clone fork getcontext llseek \
		   mincore mprotect nftw profil remap_file_pages sbrk
endif

ifeq ($(UCLIBC),1)
FILTER_OUT_DIRS	+= profil
endif

ifeq ($(ANDROID), 1)
FILTER_OUT_DIRS	+= \
	accept4 adjtimex cma confstr fcntl fmtmsg futex getcontext getcpu \
	getdomainname getdtablesize gethostid getgroups get_mempolicy ipc \
	linkat kill mallopt memmap mq_notify mq_open mq_timedreceive \
	mq_timedsend mq_unlink mmap mremap open openat profil ptrace quotactl \
	readahead remap_file_pages rt_sigsuspend rt_sigtimedwait \
	sched_getaffinity sched_setaffinity sendmsg setgroups setns sighold \
	sigrelse sigsuspend sigtimedwait sigwait sigwaitinfo stime \
	setdomainname sethostname symlinkat ulimit ustat vfork vhangup vmsplice
endif

include $(top_srcdir)/include/mk/generic_trunk_target.mk

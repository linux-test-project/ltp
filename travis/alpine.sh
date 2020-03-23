#!/bin/sh
# Copyright (c) 2019-2020 Petr Vorel <petr.vorel@gmail.com>
set -ex

apk update

apk add \
	acl-dev \
	autoconf \
	automake \
	clang \
	gcc \
	keyutils-dev \
	libaio-dev \
	libacl \
	libcap-dev \
	libselinux-dev \
	libsepol-dev \
	libtirpc-dev \
	linux-headers \
	make \
	musl-dev \
	numactl-dev \
	openssl-dev \
	pkgconfig

cat /etc/os-release

echo "WARNING: remove unsupported tests (until they're fixed)"
cd ..
rm -rfv \
	testcases/kernel/sched/process_stress/process.c \
	testcases/kernel/syscalls/confstr/confstr01.c \
	testcases/kernel/syscalls/fmtmsg/fmtmsg01.c \
	testcases/kernel/syscalls/getcontext/getcontext01.c \
	testcases/kernel/syscalls/getdents/getdents01.c \
	testcases/kernel/syscalls/getdents/getdents02.c \
	testcases/kernel/syscalls/rt_tgsigqueueinfo/rt_tgsigqueueinfo01.c \
	testcases/kernel/syscalls/timer_create/timer_create01.c \
	testcases/kernel/syscalls/timer_create/timer_create03.c \
	utils/benchmark/ebizzy-0.3
cd -

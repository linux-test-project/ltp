Build Notes for POSIX Test Suite
================================
Last update : 2010-07-10 updated for LTP-head (based off of posixtestsuite 1.40)

The POSIX Test Suite should be able to be run independently of any given
OS, so the OS-specific steps for configuring the POSIX Test Suite to run
are currently manual.

IN GENERAL
==========
In general, when calling *make:
- If you need to pass in any header locations, preprocessor definitions, etc,
  set the value via CFLAGS.
- If you need to pass in any libraries for linking, set the value via LDLIBS.
- If you need to pass any other values to the linker, set the value via LDFLAGS.
- Each subdirectory optionally contains one of the above files to denote the
  fact that it needs special linker flags, preprocessor defines, etc. This is
  done in this fashion to ensure that the requirements for each subdirectory is
  correctly tuned in a deterministic manner.

FREEBSD
=======

================
Asynchronous I/O
================
Realtime asynchronous I/O has been available in FreeBSD since 5.x. Please add:

options		VFS_AIO

to your kernel conf or build, install, and load the aio module.

================
Message Queues
================
POSIX message queues have been available in FreeBSD since 6.x. Please add:

options		P1003_1B_MQUEUE

to your kernel conf or build and install the mqueue module, and load it with:

kldload mqueuefs

================
Realtime Support
================
Realtime support has been available for a long time (I think 3.x). Please add:

options		_KPOSIX_PRIORITY_SCHEDULING

to your kernel conf. There isn't a corresponding module for this feature.

================
Semaphores
================
POSIX semaphores have been available in FreeBSD since 5.x. Please add:

options		P1003_1B_SEMAPHORES

to your kernel conf or build, install, and load the sem module.

LINUX
=====
Notes for running POSIX Test Suite against Linux implementations of
POSIX functionality.  (Note:  If these implementations are accepted into
the kernel, then the manual intervention will no longer apply, and we will
update this document.)

Please refer to your linux distribution's manual for the kernel and
library information.

Notes for each test section are here. It is targeted for general linux
kernel 2.6 and glibc-2.3.3.

=======
Threads
=======

LinuxThreads
-------------
If LinuxThreads is the default POSIX thread implementation,
to build against LinuxThreads, add '-lpthread' to LDLIBS.

NPTL
----
If NPTL is the default POSIX thread implementation, add '-lpthread' to LDLIBS.

Otherwise, to build against NPTL, export the following variable:

export GLIBCDIR=/path/to/NPTL/libc-build

Then in LDFLAGS, add the following lines:

$GLIBCDIR/nptl/libpthread.so.0 $GLIBCDIR/libc.so.6 -Wl,-rpath,$GLIBCDIR:$GLIBCDIR/nptl:$GLIBCDIR/elf,-dynamic-linker,$GLIBCDIR/elf/ld-linux.so.2

NGPT
----
To build against NGPT, export the following variables:

export LD_PRELOAD=libpthread.so.0
export LD_LIBRARY_PATH="/usr/lib:/lib:$LD_LIBRARY_PATH"

Then make sure to add '-lpthread' to LDLIBS.

==============
Message Queues
==============
POSIX Message Queue (Wronski/Benedyczak's implementation) has been
included into linux kernel since 2.6.4-rc1-mm1.
But you still need to install a user library at the time of writing this
document.

posix1b
-------
To build against the posix1b message queues, see the semaphores information for posix1b.
Also, add -I/usr/include/posix1b/ to CPPFLAGS.

Wronski/Benedyczak
------------------
To build against the Michal Wronski/Krzysztof Benedyczak message queues,
install the kernel patches from http://www.mat.uni.torun.pl/~wrona/posix_ipc/
and then build the user library from the same site.  Add '-lmqueue' to
LDLIBS to run tests.
To create the message queue file system, do:
# mkdir /dev/mqueue
# mount -t mqueue none /dev/mqueue

==========
Semaphores
==========
NPTL
----
If NPTL is installed as default POSIX thread library, set '-lpthread -lrt' in
LDLIBS.

Otherwise, to build against NPTL, export the following variable:

export GLIBCDIR=/path/to/NPTL/libc-build

Then set the following to LDFLAGS:

$GLIBCDIR/nptl/libpthread.so.0 $GLIBCDIR/libc.so.6 -Wl,-rpath,$GLIBCDIR:$GLIBCDIR/nptl:$GLIBCDIR/elf,-dynamic-linker,$GLIBCDIR/elf/ld-linux.so.2


posix1b
-------
To run the semaphore test suite against posix1b, you will need to download
and install the posix1b library.

Start downloading it from: http://www.garret.ru/~knizhnik/posix1b.tar.gz
Once you have the library compiled and installed in /usr/lib.
Add '-lposix1b' to LDLIBS:

Make sure /usr/lib/ is in your PATH.

=======
Signals
=======

For the signals-related interfaces that start with "pthread_", refer to
the Threads section above to learn how to build those particular tests.

Also, please note that if you using version of gcc older than version 3.2-7,
you may run into lots of build and link errors. Please use gcc version 3.2-7
or newer.

=====
XCOFF
=====

Since the XCOFF main is called .main, the line:
64          nm -g --defined-only $< | grep -q " T main" || exit 0; \

needs to replace main with .main to get these tests to compile.

Maintainers:	Julie Fleischer
		Rolla Selbak
		Salwan Searty
		Majid Awad
		Crystal Xiong
		Adam Li
Contributors:	Jerome Marchand
		Ulrich Drepper
		Ngie Cooper

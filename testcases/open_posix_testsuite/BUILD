Build Notes for POSIX Test Suite
================================
Last update : 2004-04-04 updated for posixtestsuite-1.4.0 

The POSIX Test Suite should be able to be run independently of any given
OS, so the OS-specific steps for configuring the POSIX Test Suite to run
are currently manual.

IN GENERAL
==========
In general, if you need to link in a library, place the -l or -L commands
in the LDFLAGS file in the same directory as the Makefile.  This will
ensure that these libraries are linked in.

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

=============
Clocks/Timers
=============
Both Linux kernel 2.6 and Glibc support Clocks/Timers now.
Add the '-lrt' option to the LDFLAGS.

=======
Threads
=======

LinuxThreads
-------------
If LinuxThreads is the default POSIX thread implementation,
to build against LinuxThreads, add the line -lpthread to LDFLAGS.

NPTL
----
If NPTL is the default POSIX thread implementation,
add the line -lpthread to LDFLAGS.

Otherwise, to build against NPTL, export the following variable:

export GLIBCDIR=/path/to/NPTL/libc-build

Then in LDFLAGS, add the following lines:

$GLIBCDIR/nptl/libpthread.so.0 $GLIBCDIR/libc.so.6 -Wl,-rpath,$GLIBCDIR:$GLIBCDIR/nptl:$GLIBCDIR/elf,-dynamic-linker,$GLIBCDIR/elf/ld-linux.so.2

NGPT
----
To build against NGPT, export the following variables:

export LD_PRELOAD=libpthread.so.0
export LD_LIBRARY_PATH="/usr/lib:/lib:$LD_LIBRARY_PATH"

Then make sure to add the line -lpthread to the file LDFLAGS.

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
Also, add -I/usr/include/posix1b/ to LDFLAGS.

Wronski/Benedyczak
------------------
To build against the Michal Wronski/Krzysztof Benedyczak message queues,
install the kernel patches from http://www.mat.uni.torun.pl/~wrona/posix_ipc/
and then build the user library from the same site.  Add -lmqueue to 
LDFLAGS to run tests.
To create the message queue file system, do:
# mkdir /dev/mqueue
# mount -t mqueue none /dev/mqueue

==========
Semaphores
==========
NPTL
----
If NPTL is installed as default POSIX thread library, use
'-lpthread' and '-lrt' flags.

Otherwise, to build against NPTL, export the following variable:
export GLIBCDIR=/path/to/NPTL/libc-build
Then in LDFLAGS, add the following lines:

$GLIBCDIR/nptl/libpthread.so.0 $GLIBCDIR/libc.so.6 -Wl,-rpath,$GLIBCDIR:$GLIBCDIR/nptl:$GLIBCDIR/elf,-dynamic-linker,$GLIBCDIR/elf/ld-linux.so.2


posix1b
-------
To run the semaphore test suite against posix1b, you will need to download
and install the posix1b library.

Start downloading it from: http://www.garret.ru/~knizhnik/posix1b.tar.gz
Once you have the library compiled and installed in /usr/lib.
Add the following library in the LDFLAGS:

		-lposix1b

Make sure /usr/lib/ is in your PATH.

=======
Signals
=======

For the signals-related interfaces that start with "pthread_", refer to 
the Threads section above to learn how to build those particular tests.

Also, please note that if you using version of gcc older that version 3.2-7,
you may run into lots of build and link errors since our Makefile uses the
compiler options "-std=c99" and "-std=gnu99". The solution to this is to
use gcc version 3.2-7 or newer.

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


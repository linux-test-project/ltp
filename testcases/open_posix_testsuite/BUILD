Build Notes for POSIX Test Suite
================================

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

Notes for each test section are here.

=============
Clocks/Timers
=============
High-Res-Timers POSIX interfaces should now be in the kernel; however,
the user-space library still needs to be installed.

If you are using the user-space library from http://high-res-timers.sf.net,
then do:

# cd /usr/src/linux/Documentation/high-res-timers/lib
# make
  (Note: You may need to add:  -I../../../include/ to the SYSASM= line)
# cp libposixtime.so /usr/lib
# vi posixtestsuite/LDFLAGS
  - Add line: "-lposixtime -D_GNU_SOURCE"
(If /usr/lib is not in your path, add -L/usr/lib.)

For the CLOCK_MONOTONIC tests, you may need to play around with include
files and headers to get the CLOCK_MONOTONIC defines.  (Kind of sloppy, but
you can also just do a "#define CLOCK_MONOTONIC <correct value>" instead.)

=======
Threads
=======

LinuxThreads
-------------
To build against Linux Threads, add the line -lpthread to LDFLAGS.

NPTL
----
To build against NPTL, export the following variable:

export GLIBCDIR=/path/to/NPTL/libc-build

Then in LDFLAGS, add the following lines:

$GLIBCDIR/nptl/libpthread.so.0 $GLIBCDIR/libc.so.6 -Wl,-rpath,$GLIBCDIR:$GLIBCDIR/nptl:$GLIBCDIR/elf,-dynamic-linker,$GLIBCDIR/elf/ld-linux.so.2

Also, in the Makefile, line #113, remove the reference to the variable $(LDFLAGS) completely.  This is a quick-fix to a linking problem that we are currently trying to find a more efficient solution for.

*Note: Make sure you modify the Makefile to have all the include files you need for NPTL.

NGPT
----
To build against NGPT, export the following variables:

export LD_PRELOAD=libpthread.so.0
export LD_LIBRARY_PATH="/usr/lib:/lib:$LD_LIBRARY_PATH"

Then make sure to add the line -lpthread to the file LDFLAGS.

==============
Message Queues
==============

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

Waechtler
---------
To build against the Waechtler message queues TBD.

==========
Semaphores
==========
NPTL
----
To build against NPTL, export the following variable:
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
Contributors:	Jerome Marchand


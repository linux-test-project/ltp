.. SPDX-License-Identifier: GPL-2.0-or-later

Debugging
=========

This section explains some tricks which can be used to debug test binaries.

Debug messages
--------------

The LTP framework currently supports ``TDEBUG`` flag test debug messages. These
messages can be enabled using the ``-D`` test's argument.

Tracing and debugging syscalls
------------------------------

The new test library runs the actual test (i.e. the ``test()`` function) in a
forked process. To get stack trace of a crashing test in ``gdb`` it's needed to
`set follow-fork-mode child <https://sourceware.org/gdb/current/onlinedocs/gdb.html/Forks.html>`_.

To trace the test, please use ``strace -f`` to enable tracing also for the
forked processes.

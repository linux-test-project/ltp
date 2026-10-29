.. SPDX-License-Identifier: GPL-2.0-or-later

Debugging
=========

This section explains some tricks which can be used to debug test binaries.

Debug messages
--------------

The LTP framework supports ``TDEBUG`` flag test debug messages. These
messages can be enabled using the ``-D`` parameter or setting ``LTP_DEBUG``
environment variable (see :doc:`../users/setup_tests`).

Both ``-D`` parameter and ``LTP_DEBUG`` support the following verbosity levels:

  ``-D1`` (or ``-D``): Enable debug logs for the test process only.
  ``-D2``: Enable verbose debug logs for both the test and library processes.

Suppress all debug logs if no '-D' flag passed (default behavior).

``LTP_DEBUG`` has higher preference than ``-D``.

Tracing and debugging syscalls
------------------------------

The new test library runs the actual test (i.e. the ``test()`` function) in a
forked process. To get stack trace of a crashing test in ``gdb`` it's needed to
`set follow-fork-mode child <https://sourceware.org/gdb/current/onlinedocs/gdb.html/Forks.html>`_.

To trace the test, please use ``strace -f`` to enable tracing also for the
forked processes.

.. SPDX-License-Identifier: GPL-2.0-or-later

List of ongoing tasks
=====================

This is a comprehensive list of tasks where LTP maintainers are currently
working on. Priorities might change over time, but these are the most important
points which are currently being achieved.

Fade out old tests runner
-------------------------

:master:`runltp` script is old and unmaintaned. We are slowly shifting to
`kirk <https://github.com/linux-test-project/kirk>`_, that will be our official
tests runner in the future.

Kirk provides support for remote testing via Qemu, SSH, LTX, parallel
execution and much more.

Test new syscalls
-----------------

Syscalls and new syscalls flags are added to Linux kernel each development
cycle and LTP still falls behind. Unfortunately there is no single place that
would store comprehensive list of syscalls, but there are a few places to look
at:

- `man-pages repository <http://git.kernel.org/cgit/docs/man-pages/man-pages.git>`_
  or the ``man2`` directory, where it's possible to find newly documented
  functionalities.
- `LWN <http://lwn.net>`_ weekly editions.
- `linux-api mailing list <https://lore.kernel.org/linux-api/>`_ where
  changes in kernel userspace API are discussed.
- `LTP Github issues <https://github.com/linux-test-project/ltp/issues>`_

Rewrite old API tests
---------------------

LTP has a long story and, at certain point of its development, new API were
introduced to make kernel testing easier and more efficient. This happened when
lots of tests were still using old, messy API.

Some of these tests have been converted to the new API, but this process is
still ongoing for many others. To have an overview of the tests using old API,
please run the following command inside the LTP root folder:

.. code-block:: bash

        git --no-pager grep -l 'include "test\.h"' testcases/

Fade out shell scripts
----------------------

LTP was initially thought as a generic framework for running tests with both
shell and plain-C languages. Even if writing tests in shell script might seem
easy, the reality is that debugging and maintaining certain test cases is
difficult and slow down the whole validation process. This is particularly
visible for cgroup tests, since shell doesn't add enough control over race
conditions.

LTP maintainers are working on converting shell scripts to plain-C tests, in
order to reduce the impact that shell scripts might have on the overall kernel
testing.

For a complete list of shell tests, please run the following command inside the
LTP root folder:

.. code-block:: bash

        git --no-pager grep -l -e '^\. .*_lib\.sh' -e '^\. .*test.sh'

LTP also provides a shell loader implementation for plain-C tests inside
:master:`testcases/lib/tst_run_shell.c` and it permits to run shell tests
into plain-C LTP API, featuring :ref:`struct tst_test` initializations and a
direct access to kernel syscalls.

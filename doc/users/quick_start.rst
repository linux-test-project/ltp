.. SPDX-License-Identifier: GPL-2.0-or-later

Installation and tests execution
================================

Basics requirements to build LTP are the following:

* git
* autoconf
* automake
* make
* gcc
* m4
* pkgconf / pkg-config
* libc headers
* linux headers

.. code-block:: console

   $ git clone https://github.com/linux-test-project/ltp.git
   $ cd ltp
   $ make autotools
   $ ./configure

.. note::

   For optional library dependencies, take a look at the scripts inside :master:`ci/`
   directory.

Running single tests
--------------------

LTP provides the possibility to build and run single tests:

.. code-block:: console

   $ cd testcases/kernel/syscalls/foo
   $ make
   $ PATH=$PATH:$PWD ./foo01

Shell testcases are a bit more complicated, since they need to setup ``PATH``
as well as to compiled binary helpers:

.. code-block:: console

   $ cd testcases/lib
   $ make
   $ cd ../commands/foo
   $ PATH=$PATH:$PWD:$PWD/../../lib/ ./foo01.sh

Open Posix Testsuite has it's own build system which needs Makefiles to be
generated first:

.. code-block:: console

   $ cd testcases/open_posix_testsuite/
   $ make generate-makefiles
   $ cd conformance/interfaces/foo
   $ make
   $ ./foo_1-1.run-test

Compiling and installing all testcases
--------------------------------------

To compile all tests is really simple:

.. code-block:: console

   $ make

   $ # install LTP inside /opt/ltp by default
   $ make install

.. note::

   Some tests will be disabled if ``configure`` script won't find the build
   dependencies.

Running tests
-------------

To run all the test suites

.. code-block:: console

   $ cd /opt/ltp

   $ # run syscalls testing suite
   $ ./kirk -f ltp -r syscalls

.. note::

   Many test cases have to be executed as root.

Test suites (e.g. syscalls) are defined in the ``runtest`` directory. Each file
contains a list of test cases in a simple format.

Each test case has its own executable or script that can directly executed:

.. code-block:: console

   $ testcases/bin/abort01

   $ # some tests have arguments
   $ testcases/bin/mesgq_nstest -m none

   $ # vast majority of tests have a help
   $ testcases/bin/ioctl01 -h

   $ # Many require certain environment variables to be set
   $ LTPROOT=/opt/ltp PATH="$PATH:$LTPROOT/testcases/bin" testcases/bin/wc01.sh

Most commonly, the ``PATH`` variable needs to be set and also ``LTPROOT``, but
there are a number of other variables which usually ``kirk`` sets for you.

.. note::

   All shell scripts need the ``PATH`` to be set. However, this is not limited
   to shell scripts and some C based tests need environment variables as well.
   They usually raise a configuration error when this is needed.

Network tests
-------------

Network tests usually require a certain setup that is described in
:master:`testcases/network/README.md`.

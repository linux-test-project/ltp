.. SPDX-License-Identifier: GPL-2.0-or-later

Writing tests
=============

This document describes LTP guidelines and it's intended for anybody who wants
to write or to modify a LTP testcase. It's not a definitive guide and it's not,
by any means, a substitute for common sense.

Guide to clean and understandable code
--------------------------------------

Testcases require that the source code is easy to follow. When a test starts to
fail, the failure has to be analyzed and clean test codebase makes this task
much easier and quicker.

Keep things simple
~~~~~~~~~~~~~~~~~~

It's worth to keep testcases simple or, better, as simple as possible.

The kernel and libc are tricky beasts and the complexity imposed by their
interfaces is quite high. Concentrate on the interface you want to test and
follow the UNIX philosophy.

It's a good idea to make the test as self-contained as possible too, ideally
tests should not depend on tools or libraries that are not widely available.

Do not reinvent the wheel!

* Use LTP standard interface
* Do not add custom PASS/FAIL reporting functions
* Do not write Makefiles from scratch, use LTP build system instead
* Etc.

Keep functions and variable names short
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Choosing a good name for an API functions or even variables is a difficult
task do not underestimate it.

There are a couple of customary names for different things that help people to
understand code, for example:

* For loop variables are usually named with a single letter ``i``, ``j``, ...
* File descriptors ``fd`` or ``fd_foo``.
* Number of bytes stored in file are usually named as ``size`` or ``len``
* Etc.

Do not over-comment
~~~~~~~~~~~~~~~~~~~

Comments can sometimes save your day, but they can easily do more harm than
good. There has been several cases where comments and actual implementation
drifted slowly apart which yielded into API misuses and hard to find bugs.
Remember there is only one thing worse than no documentation: wrong
documentation.

Ideally, everybody should write code that is obvious, which unfortunately isn't
always possible. If there is a code that requires to be commented, keep it
short and to the point. These comments should explain *why* and not *how*
things are done.

Never ever comment the obvious.

In case of LTP testcases, it's customary to add an `RST
<https://www.sphinx-doc.org/en/master/usage/restructuredtext/basics.html>`_
formatted comment paragraph with high-level test description at the beginning
of the file right under the GPL SPDX header. This helps other people to
understand the overall goal of the test before they dive into the technical
details. It's also exported into generated documentation hence it should mostly
explain what is tested.

DRY (Code duplication)
~~~~~~~~~~~~~~~~~~~~~~

Copy & paste is a good servant but very poor master. If you are about to copy a
large part of the code from one testcase to another, think what would happen if
you find bug in the code that has been copied all around the tree. What about
moving it to a library instead?

The same goes for short but complicated parts, whenever you are about to copy &
paste a syscall wrapper that packs arguments accordingly to machine
architecture or similarly complicated code, put it into a header instead.

C coding style
--------------

LTP adopted `Linux kernel coding style <https://www.kernel.org/doc/html/latest/process/coding-style.html>`_.
Run ``make check`` in the test's directory and/or use ``make check-$TCID``, it
uses (among other checks) our vendoring version of
`checkpatch.pl <https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/plain/scripts/checkpatch.pl>`_
script from kernel git tree.

.. note::
      If ``make check`` does not report any problems, the code still may be wrong
      as all tools used for checking only look for common mistakes.

The following linting code can be found when we run ``make check``:

.. list-table::
    :header-rows: 1

    * - Linting code
      - Message
      - Explanation

    * - LTP-001
      - Library source files have ``tst_`` prefix
      - API source code is inside headers in ``include/{empty}*.h``,
        ``include/lapi/{empty}*.h`` (backward compatibility for old kernel and
        libc) and C sources in ``lib/{empty}*.c``. Files must have ``tst_``
        prefix.

    * - LTP-002
      - ``TST_RET`` and ``TST_ERR`` are never modified by test library functions
      - The test author is guaranteed that the test API will not modify these
        variables. This prevents silent errors where the return value and
        errno are overwritten before the test has chance to check them.

        The macros which are clearly intended to update these variables. That
        is ``TEST`` and those in :master:`include/tst_test_macros.h`. Are of
        course allowed to update these variables.

    * - LTP-003
      - Externally visible library symbols have the ``tst_`` prefix
      - Functions, types and variables in the public test API should have the
        ``tst_`` prefix. With some exceptions for symbols already prefixed with
        ``safe_`` or ``ltp_``.

        Static (private) symbols should not have the prefix.

    * - LTP-004
      - Test executable symbols are marked ``static``
      - Test executables should not export symbols unnecessarily. This means
        that all top-level variables and functions should be marked with the
        ``static`` keyword. The only visible symbols should be those included
        from shared object files.

    * - LTP-005
      - Array must terminate with a sentinel value (i.e. ``NULL`` or ``{}``)
      - When defining arrays in the :ref:`struct tst_test` structure, we need to
        end the array items with a sentinel ``NULL`` value.

Shell coding style
------------------

When writing testcases in shell, write in *portable shell* only, it's a good
idea to try to run the test using alternative shell (alternative to bash, for
example dash) too.

*Portable shell* means Shell Command Language as defined by POSIX with an
exception of few widely used extensions, namely **local** keyword used inside of
functions and ``-o`` and ``-a`` test parameters (that are marked as obsolete in
POSIX).

You can either try to run the testcases in Debian which has ``/bin/sh`` pointing
to ``dash`` by default, or to install ``dash`` on your favorite distribution,
then use it to run tests. If your distribution lacks ``dash`` package, you can
always compile it from `sources <http://gondor.apana.org.au/~herbert/dash/files/>`_.

Run ``make check`` in the test's directory and/or use ``make check-$TCID.sh``.
It uses (among other checks) our vendoring version of
`checkbashism.pl <https://salsa.debian.org/debian/devscripts/raw/master/scripts/checkbashisms.pl>`_
from Debian that is used to check for non-portable shell code.

.. note::

      If ``make check`` does not report any problems the code still may be wrong,
      as ``checkbashisms.pl`` is used for checking only common mistakes.

Here there are some common sense style rules for shell

* Keep lines under 80 chars
* Use tabs for indentation
* Keep things simple, avoid unnecessary subshells
* Don't do confusing things (i.e. don't name your functions like common shell
  commands, etc.)
* Quote variables
* Be consistent

3 Backwards compatibility
~~~~~~~~~~~~~~~~~~~~~~~~~

LTP test should be as backward compatible as possible. Think of an enterprise
distributions with long term support (more than five years since the initial
release) or of an embedded platform that needs to use several years old
toolchain supplied by the manufacturer.

Therefore LTP test for more current features should be able to cope with older
systems. It should at least compile fine and if it's not appropriate for the
configuration it should return ``TCONF``.

There are several types of checks we use:

* The *configure script* is usually used to detect availability of a function
  declarations in system headers. It's used to disable tests at compile time or
  to enable fallback definitions.

* Checking the ``errno`` value is another type of runtime check. Most of the
  syscalls returns either ``EINVAL`` or ``ENOSYS`` when syscall was not
  implemented or was disabled upon kernel compilation.

* LTP has kernel version detection that can be used to disable tests at runtime.
  Unfortunately, the kernel version does not always corresponds to a well
  defined feature set, as distributions tend to backport hundreds of patches
  while the kernel version stays the same. Use with caution.

* Lately, we added a kernel ``.config`` parser. A test can define a boolean
  expression of kernel config variables that has to be satisfied in order to run
  a test. At the moment, this is mostly used for kernel namespaces.

* Sometimes it makes sense to define a few macros instead of creating a
  configure test. One example is Linux specific POSIX clock ids in
  :master:`include/lapi/posix_clocks.h`.

Dealing with messed up legacy code
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

LTP still contains a lot of old and messy code and we are cleaning it up as
fast as we can but, despite the decade of efforts, there is still a lot of it.
If you start modifying old or a messy testcase and your changes are more
complicated than simple typo fixes, you should convert the test into a new
library first.

It's also much easier to review the changes if you split them into a smaller
logical groups. The same goes for moving files: if you need to rename or to move
files, do it in a separate patch.

License
~~~~~~~

Code contributed to LTP should be licensed under GPLv2+ (GNU GPL version 2 or
any later version).

Use ``SPDX-License-Identifier: GPL-2.0-or-later``

LTP Structure
-------------

The structure of LTP is quite simple. Each test is a binary written either in
portable shell or C. The test gets a configuration via environment variables
and/or command line parameters, it prints additional information into the
stdout and reports overall success/failure via the exit value.

Tests are generally placed under the :master:`testcases` directory. Everything that
is a syscall or (slightly confusingly) libc syscall wrapper, goes under
:master:`testcases/kernel/syscalls/`.

There is also :master:`testcases/open_posix_testsuite/` which is a well maintained
fork of the Open POSIX testsuite project, that has been dead since 2005.

We also have a number of directories with tests for more specific features, such
as containers, etc.

Runtest Files
~~~~~~~~~~~~~

The list of tests to be executed is stored in runtest files under the
:master:`runtest` directory. The default set of runtest files to be executed is
stored in :master:`scenario_groups/default`. When you add a test, you should add
corresponding entries into some runtest file(s) as well.

Each line of runtest file contains one test. The first item is the test name.
All other items, separated by space will be executed as a command.

.. code-block:: bash

      shell_test01 echo "SUCCESS" | shell_pipe01.sh
      splice02 splice02 -s 20

Blank lines and lines starting with a ``#`` (comments) are ignored.

Syscalls tests, placed under :master:`testcases/kernel/syscalls/`, use
:master:`runtest/syscalls` file. For kernel related tests for memory management we
have :master:`runtest/mm`, etc.

.. note::

      runtest files should have one entry per a test. Creating a
      wrapper that runs all your tests and adding it as a single test
      into runtest file is strongly discouraged.

Datafiles
---------

If your test needs data files, these should be put into a subdirectory
named ``datafiles`` and installed into the ``testcases/data/$TCID`` directory.
This will require to add ``INSTALL_DIR := testcases/data/TCID`` into
correspondent ``datafiles/Makefile``.

You can obtain path to datafiles via ``$TST_DATAROOT`` provided by ``test.sh``
or via C function ``tst_dataroot()`` provided by libltp:

.. code-block:: c

      const char *dataroot = tst_dataroot();

Datafiles can also be accessed as ``$LTPROOT/testcases/data/$TCID/...``,
but ``$TST_DATAROOT`` and ``tst_dataroot()`` are preferred, as these can be used
when running testcases directly in git tree as well as from install location.

Sub-executables
~~~~~~~~~~~~~~~

If your test needs to execute a binary, place it in the same directory of the
testcase and name the binary with ``$TESTNAME_`` prefix, where ``$TESTNAME`` is
the name of the test binary. Once the test is executed by the framework, the
path to the directory with all LTP binaries is added to the ``$PATH`` and you
can execute it via its name.

.. note::

      If you need to execute a test from the LTP tree, you can add ``PATH`` to
      the current directory with ``PATH="$PATH:$PWD" ./foo01``.

Test Contribution Checklist
---------------------------

#. Test compiles and it runs fine (check with ``-i 10`` and ``-i 0`` too)
#. ``make check`` should not emit any warnings for the test you are working on
   (hint: run it in the test's directory and/or use ``make check-$TCID``)
#. The runtest entries are in place
#. New test binaries are added into the corresponding ``.gitignore`` files
#. Patches apply over the latest git

About .gitignore files
~~~~~~~~~~~~~~~~~~~~~~

There are numerous ``.gitignore`` files in the LTP tree. Usually, there is a
``.gitignore`` file for a group of tests. The reason of this setup is simple:
it's easier to maintain a ``.gitignore`` file per tests' directory, rather
than having a single file in the project root directory. In this way, we don't
have to update all the gitignore files when moving directories, and they get
deleted automatically when a directory with tests is removed.

Testing pre-release kernel features
-----------------------------------

Tests for features not yet in the mainline kernel release are accepted. However,
they must be added only to :master:`runtest/staging`. Once a feature is part
of the stable kernel ABI, the associated test must be moved out of staging.

Testing builds with GitHub Actions
----------------------------------

Master branch is tested in GitHub :repo:`actions`
to ensure LTP builds in various distributions, including old, current and
bleeding edge. ``gcc`` and ``clang`` toolchains are also tested for various
architectures using cross-compilation. For a full list of tested distros, please
check :master:`.github/workflows/ci-docker-build.yml`.

.. note::

      Passing the GitHub Actions CI means that LTP compiles in a variety of
      different distributions on their **newest releases**.
      The CI also checks for code linting, running ``make check`` in the whole
      LTP project.

LTP C And Shell Test API Comparison
-----------------------------------

.. list-table::
    :header-rows: 1

    * - C API :ref:`struct tst_test` members
      - Shell API ``$TST_*`` variables

    * - .all_filesystems
      - TST_ALL_FILESYSTEMS

    * - .bufs
      - \-

    * - .caps
      - \-

    * - .child_needs_reinit
      - not applicable

    * - .cleanup
      - TST_CLEANUP

    * - .dev_extra_opts
      - TST_DEV_EXTRA_OPTS

    * - .dev_fs_opts
      - TST_DEV_FS_OPTS

    * - .dev_fs_type
      - TST_FS_TYPE

    * - .dev_min_size
      - TST_DEVICE_SIZE

    * - .format_device
      - TST_FORMAT_DEVICE

    * - .max_runtime
      - TST_TIMEOUT (not exactly the same, a real timeout based on old .timeout
        concept. .max_runtime has also an extra 30 sec safety margin for
        teardown of the test.)

    * - .min_cpus
      - not applicable

    * - .min_kver
      - TST_MIN_KVER

    * - .min_mem_avail
      - not applicable

    * - .mnt_flags
      - TST_MNT_PARAMS

    * - .min_swap_avail
      - not applicable

    * - .mntpoint | .mnt_data
      - TST_MNTPOINT

    * - .mount_device
      - TST_MOUNT_DEVICE

    * - .needs_cgroup_ctrls
      - \-

    * - .needs_checkpoints
      - TST_NEEDS_CHECKPOINTS

    * - .needs_cmds
      - TST_NEEDS_CMDS

    * - .needs_devfs
      - \-

    * - .needs_device
      - TST_NEEDS_DEVICE

    * - .needs_drivers
      - TST_NEEDS_DRIVERS

    * - .needs_kconfigs
      - TST_NEEDS_KCONFIGS

    * - .needs_overlay
      - \-

    * - .needs_rofs
      - \-

    * - .needs_root
      - TST_NEEDS_ROOT

    * - .needs_tmpdir
      - TST_NEEDS_TMPDIR

    * - .options
      - TST_PARSE_ARGS | TST_OPTS

    * - .resource_files
      - \-

    * - .restore_wallclock
      - not applicable

    * - .sample
      - \-

    * - .save_restore
      - \-

    * - .scall
      - not applicable

    * - .setup
      - TST_SETUP

    * - .skip_filesystems
      - TST_SKIP_FILESYSTEMS

    * - .skip_in_compat
      - \-

    * - .skip_in_lockdown
      - TST_SKIP_IN_LOCKDOWN

    * - .skip_in_secureboot
      - TST_SKIP_IN_SECUREBOOT

    * - .supported_archs
      - not applicable

    * - .tags
      - \-

    * - .taint_check
      - \-

    * - .tcnt
      - TST_CNT

    * - .tconf_msg
      - not applicable

    * - .test | .test_all
      - TST_TESTFUNC

    * - .test_variants
      - \-

    * - .tst_hugepage
      - not applicable

    * - .ulimit
      - not applicable

    * - not applicable
      - TST_NEEDS_KCONFIGS_IFS

    * - not applicable
      - TST_NEEDS_MODULE

    * - not applicable
      - TST_POS_ARGS

    * - not applicable
      - TST_USAGE

.. list-table::
    :header-rows: 1

    * - C API other structs
      - Shell API ``$TST_*`` variables

    * - :ref:`struct tst_device`
      - TST_DEVICE

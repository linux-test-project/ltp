.. SPDX-License-Identifier: GPL-2.0-or-later

Test case tutorial
==================

This is a step-by-step tutorial on writing a simple C LTP test, where topics
of the LTP and Linux kernel testing will be introduced gradually using a
concrete example. Most sections will include exercises, some trivial and
others not so much. If you find an exercise is leading you off at too much of
a tangent, just leave it for later and move on.

LTP tests can be written in C or Shell script. This tutorial is **only for tests
written in C** using the new LTP test API. Note that while we go into some
detail on using Git, this is not intended as a canonical or complete guide
for Git.

Assumptions & Feedback
----------------------

We assume the reader is familiar with C, Git and common Unix/Linux/GNU tools
and has some general knowledge of Operating Systems. Experienced Linux
developers may find it too verbose while people new to system level Linux
development may find it overwhelming.

Comments and feedback are welcome, please direct them to the Mailing list.

Getting Started
---------------

First of all, make sure you have a copy of LTP in the current folder
and we recommended cloning the Linux kernel repository for reference, this
guide will refer to files and directories.

.. code-block:: bash

    git clone git://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git

There are a number of other repositories which are useful for reference as
well, including the GNU C library ``glibc`` and the alternative C library
``musl``. Some system calls are partially or even entirely implemented in user
land as part of the standard C library. So in these cases, the C library is an
important reference. ``glibc`` is the most common C library for Linux, however
``musl`` is generally easier to understand.

How system calls are implemented varies from one architecture to another and
across kernel and C library versions. To find out whether a system call is
actually accessing the kernel (whether it is actually a system call) on any
given machine you can use the ``strace`` utility. This intercepts system calls
made by an executable and prints them. We will use this later in the tutorial.

Choose a System Call to test
----------------------------

We will use the ``statx()`` system call, to provide a concrete example of a
test. At the time of writing there is no test for this call which was
introduced in Linux kernel version 4.11.

Linux system call specific tests are primarily contained in
``testcases/kernel/syscalls``, but you should also ``git grep`` the entire LTP
repository to check for any existing usages of a system call.

One way to find a system call which is not currently tested by the LTP is to
look at ``include/linux/syscalls.h`` in the kernel tree.

Something the LTP excels to ensure bug-fixes are back ported to
maintenance releases, so targeting a specific regression is another
option.

Find an untested System call
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Try to find an untested system call which has a manual page (i.e. ``man
syscall`` produces a result). It is a good idea to Git-clone the latest kernel
man-pages repository.

.. code-block:: bash

    git clone git://git.kernel.org/pub/scm/docs/man-pages/man-pages.git

At the time of writing, the difference between the latest man-pages release and
the ``HEAD`` of the repository (usually the latest commit) is well over 100
commits. This represents about 9 weeks of changes. If you are using a stable
Linux distribution, your man-pages package may well be years old. So as with
the kernel, it is best to have the Git repository as a reference.

You could also find a system call with untested parameters or use whatever it
is you are planning to use the LTP for.

Create the test skeleton
------------------------

I shall call my test ``statx01.c``, by the time you read this that file name
will probably be taken, so increment the number in the file name as
appropriate or replace ``statx`` with the system call in the chosen exercise.

.. code-block:: bash

    mkdir testcases/kernel/syscalls/statx
    cd testcases/kernel/syscalls/statx
    echo statx >> .gitignore

Next open ``statx01.c`` and add the following boilerplate. Make sure to change
the copyright notice to your name/company, correct the test name and minimum
kernel version if necessary. I will explain what the code does below.

.. code-block:: c

    // SPDX-License-Identifier: GPL-2.0-or-later
    /*
    * Copyright (c) 2017 Instruction Ignorer <"can't"@be.bothered.com>
    */

    /*\
    * [Description]
    *
    * All tests should start with a description of _what_ we are testing.
    * Non-trivial explanations of _how_ the code works should also go here.
    * Include relevant links, Git commit hashes and CVE numbers.
    * Inline comments should be avoided.
    */

    #include "tst_test.h"

    static void run(void)
    {
        tst_res(TPASS, "Doing hardly anything is easy");
    }

    static struct tst_test test = {
        .test_all = run,
        .min_kver = "4.11",
    };

Starting with the ``#include`` statement we copy in the main LTP test library
headers. This includes the most common test API functions and the test harness
initialization code. It is important to note that this is a completely
ordinary, independent C program, however ``main()`` is missing because it is
implemented in ``tst_test.h``.

We specify what code we want to run as part of the test using the ``tst_test
test`` structure. Various callbacks can be set by the test writer, including
``test.test_all``, which we have set to ``run()``. The test harness will execute
this callback in a separate process (using ``fork()``), forcibly terminating it
if it does not return after ``test.timeout`` seconds.

We have also set ``test.min_kver`` to the kernel version where ``statx`` was
introduced. The test library will determine the kernel version at runtime. If
the version is less than 4.11 then the test harness will return ``TCONF``,
indicating that this test is not suitable for the current system
configuration.

Occasionally features are back ported to older kernel versions, so ``statx`` may
exist on kernels with a lower version. However we don't need to worry about
that unless there is evidence of it happening.

As mentioned in the code itself, you should specify what you are testing and
the expected outcome, even if it is relatively simple. If your program flow is
necessarily complex and difficult to understand (which is often the case when
trying to manipulate the kernel into doing something bad), then a detailed
explanation of how the code works is welcome.

What you should not do, is use inline comments or include the same level of
explanation which is written here. As a general rule, if something is easy to
document, then the code should also be easy to read. So don't document the easy
stuff (except for the basic test specification).

Before continuing we should compile this and check that the basics work. In
order to compile the test we need a ``Makefile`` in the same subdirectory. If
one already exists, then nothing needs to be done, otherwise add one with the
following contents.

.. code-block:: make

    # SPDX-License-Identifier: GPL-2.0-or-later
    # Copyright (c) 2019 Linux Test Project

    top_srcdir		?= ../../../..

    include $(top_srcdir)/include/mk/testcases.mk

    include $(top_srcdir)/include/mk/generic_leaf_target.mk

This will automatically add ``statx01.c`` as a build target producing a
``statx01`` executable. Unless you have heavily deviated from the tutorial, and
probably need to change ``top_srcdir``, nothing else needs to be done.

Normally, if you were starting a Makefile from scratch, then you would need to
add ``statx01`` as a build target. Specifying that you would like to run some
program (e.g. ``gcc`` or ``clang``) to transform ``statx01.c`` into ``statx01``.
Here we don't need to do that, but sometimes it is still necessary. For example,
if we needed to link to the POSIX threading library, then we could add the
following line after ``testcases.mk``.

.. code-block:: make

    statx01: CFLAGS += -pthread

Assuming you are in the test's subdirectory ``testcases/kernel/syscalls/statx``,
please do:

.. code-block:: bash

    make
    ./statx01

This should build the test and then run it. However, even though the test is
in the ``syscalls`` directory it won't be automatically ran as part of the
_syscalls_ test group (remember ``./runltp -f syscalls`` from the
``README.md``?). For this we need to add it to the ``runtest`` file. So open
``runtest/syscalls`` and add the lines starting with a ``+``.

.. code-block::

    statvfs01 statvfs01
    statvfs02 statvfs02

    +statx01 statx01
    +
    stime01 stime01
    stime02 stime02

The ``runtest`` files are in a two column format. The first column is the test
name, which is mainly used by test runners for reporting and filtering. It is
just a single string of text with no spaces. The second column, which can
contain spaces, is passed to the shell in order to execute the test. Often it
is just the executable name, but some tests also take arguments (the LTP has a
library for argument parsing, by the way).

If you haven't done so already, we should add all these new files to Git. It
is vitally important that you do not make changes to the master branch. If you
do then pulling changes from upstream becomes a major issue. So first of all
create a new branch.

.. code-block:: bash

    git checkout -b statx01 master

Now we want to add the files we have created or modified, but before doing a
commit make sure you have configured Git correctly. You need to at least set
your Name and e-mail address in ``~/.gitconfig``, but there are some other
settings which come in handy too. My relatively simple configuration is similar
to the below:

.. code-block:: ini

    [user]
        name = Sarah Jane
        email = sjane@e-mail.address
    [core]
        editor = emacs
    [sendemail]
        smtpServer = smtp.server.address

Obviously you need to at least change your name and e-mail. The SMTP server is
useful for ``git send-email``, which we will discuss later. The editor value is
used for things like writing commits (without the ``-m`` option).

.. code-block:: bash

    git add -v :/testcases/kernel/syscalls/statx :/runtest/syscalls
    git commit -m "statx01: Add new test for statx syscall"

This should add all the new files in the ``statx`` directory and the ``runtest``
file. It is good practice to commit early and often. Later on we will do a
Git-rebase, which allows us to clean up the commit history. So don't worry
about how presentable your commit log is for now. Also don't hesitate to
create a new branch when doing the exercises or experimenting. This will allow
you to diverge from the tutorial and then easily come back again.

I can't emphasize enough that Git makes things easy through branching and that
things quickly get complicated if you don't do it. However if you do get into
a mess, Git-reflog and Git-reset, will usually get you out of it. If you also
mess that up then it may be possible to cherry pick 'dangling' commits out of
the database into a branch.

Report TCONF instead of TPASS
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Maybe the test should report ``TCONF: Not implemented`` instead or perhaps
``TBROK``. Try changing it do so.

Check Git ignores the executable
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Is your ``.gitignore`` correct?

Run make check
~~~~~~~~~~~~~~~~~~

Check coding style with ``make check``.

Install the LTP and run the test with runtest
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Run ``statx01`` on its own, also using ``-I0`` amd ``-I10``.

Call the system call
--------------------

At the time of writing ``statx`` has no ``glibc`` wrapper. It is also fairly common
for a distribution's C library version to be older than its kernel or it may use a
cut down C library in comparison to the GNU one. So we must call ``statx()``
using the general ``syscall()`` interface.

The LTP contains a library for dealing with the ``syscall`` interface, which is
located in ``include/lapi``. System call numbers are listed against the relevant
call in the ``*.in`` files (e.g. ``x86_64.in``) which are used to generate
``syscalls.h``, which is the header you should include. On rare occasions you
may find the system call number is missing from the ``*.in`` files and will need
to add it (see ``include/lapi/syscalls/strip_syscall.awk``).

System call numbers vary between architectures, hence there are multiple
``*.in`` files for each architecture. You can find the various values for the
``statx`` system call across a number of ``unistd.h`` files in the Linux kernel.

Note that we don't use the system-call-identifier value available in
``/usr/include/linux/uinstd.h`` because the kernel might be much newer than the
user land development packages.

For ``statx`` we had to add ``statx 332`` to ``include/lapi/syscalls/x86_64.in``,
``statx 383`` to ``include/lapi/syscalls/powerpc.in``, etc.  Now lets look at
the code, which I will explain in more detail further down.

.. code-block:: c

    /*
    * Test statx
    *
    * Check if statx exists and what error code it returns when we give it dodgy
    * data.
    */

    #include <stdint.h>
    #include "tst_test.h"
    #include "lapi/syscalls.h"

    struct statx_timestamp {
        int64_t	       tv_sec;
        uint32_t       tv_nsec;
        int32_t	       __reserved;
    };

    struct statx {
        uint32_t	stx_mask;
        uint32_t	stx_blksize;
        uint64_t	stx_attributes;
        uint32_t	stx_nlink;
        uint32_t	stx_uid;
        uint32_t	stx_gid;
        uint16_t	stx_mode;
        uint16_t	__spare0[1];
        uint64_t	stx_ino;
        uint64_t	stx_size;
        uint64_t	stx_blocks;
        uint64_t	stx_attributes_mask;
        struct statx_timestamp	stx_atime;
        struct statx_timestamp	stx_btime;
        struct statx_timestamp	stx_ctime;
        struct statx_timestamp	stx_mtime;
        uint32_t	stx_rdev_major;
        uint32_t	stx_rdev_minor;
        uint32_t	stx_dev_major;
        uint32_t	stx_dev_minor;
        uint64_t	__spare2[14];
    };

    static int sys_statx(int dirfd, const char *pathname, int flags,
                unsigned int mask, struct statx *statxbuf)
    {
        return tst_syscall(__NR_statx, dirfd, pathname, flags, mask, statxbuf);
    }

    ...

So the top part of the code is now boiler plate for calling ``statx``. It is
common for the kernel to be newer than the user land libraries and headers. So
for new system calls like ``statx``, we copy, with a few modifications, the
relevant definitions into the LTP. This is somewhat like 'vendoring', although
we are usually just copying headers required for interacting with the Kernel's
ABI (Application Binary Interface), rather than integrating actual
functionality.

So from the top we include the ``stdint.h`` library which gives us the standard
``(u)int*_t`` type definitions. We use these in place of the Kernel type
definitions such as ``__u64`` in ``linux/types.h``. We then have a couple of
structure definitions which form part of the ``statx`` API. These were copied
from ``include/uapi/linux/stat.h`` in the Kernel tree.

After that, there is a wrapper function, which saves us from writing
``tst_syscall(__NR_statx, ...``, every time we want to make a call to
``statx``. This also provides a stub for when ``statx`` is eventually integrated
into the LTP library and also implemented by the C library. At that point we
can switch to using the C library implementation if available or fallback to
our own.

The advantage of using the C library implementation is that it will often be
better supported across multiple architectures. It will also mean we are using
the system call in the same way most real programs would. Sometimes there are
advantages to bypassing the C library, but in general it should not be our
first choice.

The final test should do a check during configuration (i.e. when we run
``./configure`` before building) which checks if the ``statx`` system call and
associated structures exists. This requires writing an ``m4`` file for use with
``configure.ac`` which is processed during ``make autotools`` and produces the
configure script.

For the time being though we shall just ignore this. All you need to know for
now is that this is a problem which eventually needs to be dealt with and that
there is a system in place to handle it.

.. code-block:: c

    ...

    static void run(void)
    {
        struct statx statxbuf = { 0 };

        TEST(sys_statx(0, NULL, 0, 0, &statxbuf));

        if (TST_RET == 0)
            tst_res(TFAIL, "statx thinks it can stat NULL");
        else if (TST_ERR == EFAULT)
            tst_res(TPASS, "statx set errno to EFAULT as expected");
        else
            tst_res(TFAIL | TERRNO, "statx set errno to some unexpected value");
    }

    static struct tst_test test = {
        .test_all = run,
        .min_kver = "4.11",
    };

The ``TEST`` macro sets ``TST_RET`` to the return value of ``tst_statx()`` and
``TST_ERR`` to the value of ``errno`` immediately after the functions
return. This is mainly just for convenience, although it potentially could
have other uses.

We check whether the return value indicates success and if it doesn't also
check the value of ``errno``. The last call to ``tst_res`` includes ``TERRNO``,
which will print the current error number and associated description in
addition to the message we have provided. Note that it uses the current value
of ``errno`` not ``TST_ERR``.

What we should have done in the example above is use ``TTERRNO`` which takes the
value of ``TST_ERR``.

If we try to run the test on a kernel where ``statx`` does not exist, then
``tst_syscall`` will cause it to fail gracefully with ``TCONF``. Where ``TCONF``
indicates the test is not applicable to our configuration.

The function ``tst_syscall`` calls ``tst_brk(TCONF,...)`` on failure. ``tst_brk``
causes the test to exit immediately, which prevents any further test code from
being run.

What are the differences between ``tst_brk`` and ``tst_res``?
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

See ``include/tst_test.h``. Also what do they have in common?

What happens if you call ``tst_res(TINFO, ...)`` after ``sys_statx``?
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Does the test still function correctly?

Extend the test to handle other basic error conditions
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

For example, see if you can trigger ``ENOENT`` instead. You shouldn't
have to create any files, which is discussed in the next section.

Setup, Cleanup and files
------------------------

Some tests require resources to be allocated, or system settings to be
changed, before the test begins. This ``setup`` only has to be done once at the
beginning and at the end of the test needs to be removed or reverted. The
``cleanup`` also has to be done regardless of whether the test breaks.

Fortunately, like most test libraries, we have setup and cleanup (teardown)
callbacks. ``setup`` is called once before ``run`` and ``cleanup`` is called once
afterwards. Note that ``run`` itself can be called multiple times by the test
harness, but that ``setup`` and ``cleanup`` are only called once.

If either your code, a ``SAFE_*`` macro or a library function such as
``tst_syscall`` call ``tst_brk``, then ``run`` will exit immediately and the
``cleanup`` function is then called. Once ``cleanup`` is completed, the test
executable will then exit altogether abandoning any remaining iterations of
``run``.

For ``statx`` we would like to create some files or file like objects which we
have control over. Deciding where to create the files is easy, we just create
it in the current working directory and let the LTP test harness handle where
that should be by setting ``.needs_tmpdir = 1``.

.. code-block:: c

    /*
    * Test statx
    *
    * Check if statx exists and what error code it returns when we give it dodgy
    * data. Then stat a file and check it returns success.
    */

    #include <stdint.h>
    #include "tst_test.h"
    #include "lapi/syscalls.h"
    #include "lapi/fcntl.h"

    #define FNAME "file_to_stat"
    #define STATX_BASIC_STATS 0x000007ffU

    /*************** statx structure and wrapper goes here ! ***************/
    ...

We have added an extra include ``lapi/fcntl.h`` which wraps the system header by
the same name (``#include <fcntl.h>``). This header ensures we have definitions
for recently added macros such as ``AT_FDCWD`` by providing fall backs if the
system header does not have them. The ``lapi`` directory contains a number of
headers like this.

At some point we may wish to add ``lapi/stat.h`` to provide a fall back for
macros such as ``STATX_BASIC_STATS``. However for the time being we have just
defined it in the test.


.. code-block:: c

    ...

    static void setup(void)
    {
        SAFE_TOUCH(FNAME, 0777, NULL);
    }

    static void run(void)
    {
        struct statx statxbuf = { 0 };

        TEST(sys_statx(0, NULL, 0, 0, &statxbuf));
        if (TST_RET == 0)
            tst_res(TFAIL, "statx thinks it can stat NULL");
        else if (TST_ERR == EFAULT)
            tst_res(TPASS, "statx set errno to EFAULT as expected");
        else
            tst_res(TFAIL | TERRNO, "statx set errno to some unexpected value");

        TEST(sys_statx(AT_FDCWD, FNAME, 0, STATX_BASIC_STATS, &statxbuf));
        if (TST_RET == 0)
            tst_res(TPASS, "It returned zero so it must have worked!");
        else
            tst_res(TFAIL | TERRNO, "statx can not stat a basic file");
    }

    static struct tst_test test = {
        .setup = setup,
        .test_all = run,
        .min_kver = "4.11",
        .needs_tmpdir = 1
    };

The ``setup`` callback uses one of the LTP's ``SAFE`` functions to create an empty
file ``file_to_stat``. Because we have set ``.needs_tmpdir``, we can just create
this file in the present working directory. We don't need to create a
``cleanup`` callback yet because the LTP test harness will recursively delete
the temporary directory and its contents.

The ``run`` function can be called multiple times by the test harness, however
``setup`` and ``cleanup`` callbacks will only be ran once.

.. warning::

    By this point you may have begun to explore the LTP library headers or older
    tests. In which case you will have come across functions from the old API such
    as ``tst_brkm``. The old API is being phased out, so you should not use these
    functions.

So far we haven't had to do any clean up. So our example doesn't answer the
question "what happens if part of the clean up fails?". To answer this we are
going to modify the test to ask the (highly contrived) question "What happens
if I create and open a file, then create a hard-link to it, then call open
again on the hard-link, then ``stat`` the file".


.. code-block:: c

    #define LNAME "file_to_stat_link"

    ...

    static void setup(void)
    {
        fd = SAFE_OPEN(FNAME, O_CREAT, 0777);
        SAFE_LINK(FNAME, LNAME);
        lfd = SAFE_OPEN(LNAME, 0);
    }

    static void cleanup(void)
    {
        if (lfd != 0)
            SAFE_CLOSE(lfd);

        if (fd != 0)
            SAFE_CLOSE(fd);
    }

    static void run(void)
    {
            ...

        TEST(sys_statx(AT_FDCWD, LNAME, 0, STATX_BASIC_STATS, &statxbuf));
        if (TST_RET == 0)
            tst_res(TPASS, "It returned zero so it must have worked!");
        else
            tst_res(TFAIL | TERRNO, "statx can not stat a basic file");
    }

    static struct tst_test test = {
        .setup = setup,
        .cleanup = cleanup,
        .test_all = run,
        .tcnt = 2,
        .min_kver = "4.11",
        .needs_tmpdir = 1
    };

Because we are now opening a file, we need a ``cleanup`` function to close the
file descriptors. We have to manually close the files to ensure the temporary
directory is deleted by the test harness (see the
https://github.com/linux-test-project/ltp/wiki/Test-Writing-Guidelines[test
writing guidelines] for details).

As a matter of good practice, the file descriptors are closed in reverse
order. In some circumstances the order which ``cleanup`` is performed is
significant. In those cases, resources created towards the end of ``setup`` are
dependent to ones near the beginning. During ``cleanup`` we remove the
dependants before their dependencies.

If, for some reason, the file descriptor ``lfd`` became invalid during the test,
but ``fd`` was still open, we do not want ``SAFE_CLOSE(lfd)`` to cause the
``cleanup`` function to exit prematurely. If it did, then ``fd`` would remain
open which would cause problems on some file systems.

Nor do we want to call ``cleanup`` recursively. So during ``cleanup``
``tst_brk``, and consequently the ``SAFE`` functions, do not cause the test to
exit with ``TBROK``. Instead they just print an error message with ``TWARN``.

It is not entirely necessary to check if the file descriptors have a none zero
value before attempting to close them. However it avoids a bunch of spurious
warning messages if we fail to open ``file_to_stat``. Test case failures can be
difficult to interpret at the best of times, so avoid filling the log with
noise.

Check ``statx`` returns the correct number of hard links
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

The field ``statx.stx_nlink`` should be equal to 2, right?

Git-branch
~~~~~~~~~~

We are about to make some organizational changes to the test, so now would be
a good time to branch. Then we can switch between the old and new versions, to
check the behavior has not been changed by accident.

Split the test
--------------

In our current test, we have essentially rolled two different test cases into
one. Firstly we check if an error is returned when bad arguments are provided
and secondly we check what happens when we stat an actual file. Quite often it
makes sense to call ``tst_res`` multiple times in a single test case because we
are checking different properties of the same result, but here we are clearly
testing two different scenarios.

So we should split the test in two. One obvious way to do this is to create
``statx02.c``, but that seems like overkill in order to separate two simple test
cases. So, for now at least, we are going to do it a different way.

.. code-block:: c

    ...

    static void run_stat_null(void)
    {
        struct statx statxbuf = { 0 };

        TEST(sys_statx(0, NULL, 0, 0, &statxbuf));
        if (TST_RET == 0)
            tst_res(TFAIL, "statx thinks it can stat NULL");
        else if (TST_ERR == EFAULT)
            tst_res(TPASS, "statx set errno to EFAULT as expected");
        else
            tst_res(TFAIL | TERRNO, "statx set errno to some unexpected value");
    }

    static void run_stat_symlink(void)
    {
        struct statx statxbuf = { 0 };

        TEST(sys_statx(AT_FDCWD, LNAME, 0, STATX_BASIC_STATS, &statxbuf));
        if (TST_RET == 0)
            tst_res(TPASS, "It returned zero so it must have worked!");
        else
            tst_res(TFAIL | TERRNO, "statx can not stat a basic file");
    }

    static void run(unsigned int i)
    {
        switch(i) {
        case 0: run_stat_null();
        case 1: run_stat_symlink();
        }
    }

    static struct tst_test test = {
        .setup = setup,
        .cleanup = cleanup,
        .test = run,
        .tcnt = 2,
        .min_kver = "4.11",
        .needs_tmpdir = 1
    };

So we have used an alternative form of the ``test`` or ``run`` callback which
accepts an index. Some tests use this index with an array of parameters and
expected return values. Others do something similar to the above. The index
can be used how you want so long as each iteration calls ``tst_res`` in a
meaningful way.

If an iteration fails to return a result (i.e. call ``tst_res`` with a value
other than ``TINFO``) then the test harness will report ``TBROK`` and print the
iteration which failed. This prevents a scenario in your test from silently
failing due to some faulty logic.

What is wrong with the switch statement?
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Were you paying attention? Also see the output of ``make check``.

Test a feature unique to statx
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

So far we have not tested anything which is unique to ``statx``. So, for
example, you could check stx_btime is correct (possibly only to within a
margin of error) and that it differs from ``stx_mtime`` after writing to the
file.

Alternatively you could check that ``stx_dev_major`` and ``stx_dev_minor`` are set
correctly. Note that the LTP has helper functions for creating devices and
file systems.

This could be quite a challenging exercise. You may wish to tackle an
altogether different test scenario instead. If you get stuck just move onto
the next section and come back later.

Submitting the test for review
------------------------------

Ignoring the fact we should probably create ``lapi/stat.h`` along with a bunch
of fallback logic in the build system. We can now get our test ready for
submission.

The first thing you need to do before considering submitting your test is run
``make check-statx01`` or ``make check`` in the test's directory. Again, we use
the kernel style guidelines where possible. Next you should create a new
branch, this will allow you to reshape your commit history without fear.

After that we have the pleasure of doing an interactive ``rebase`` to clean up
our commit history. In its current form the test only really needs a single
commit, but if you have been using Git correctly then you should have
many. The main reason we want to compress it to a single commit, is to make
the LTP's Git-log readable. It also allows us to write a coherent description
of the work as a whole in retrospective. Although, when adding a new test, the
test description in the code will probably make the commit message redundant.

Anyway, as an example, we shall look at my personal commit history from this
tutorial and ``rebase`` it. You should try following along with your own
repository. First lets look at the commit history since we branched from
master.

.. code-block:: bash

    git log --oneline master..HEAD
    152d39fe7 (HEAD -> tutorial-rebase2, tutorial-rebase) tutorial: Start Submitting patch section
    70f7ce7ce statx01: Stop checkpatch from complaining
    bb0332bd7 tutorial: Fix review problems
    6a87a084a statx01: Fix review problems
    d784b1e85 test-writing-guidelines: Remove old API argument
    c26e1be7a fixup! tutorial
    1e24a5fb5 (me/tutorial-rebase) fixup! tutorial
    568a3f7be fixup! tutorial
    09dd2c829 statx: stage 6
    bfeef7902 statx: stage 5b
    76e03d714 statx: stage 5a
    98f5bc7ac statx: stage 4
    6f8c16438 statx: stage 3 (Add statx01)
    5d93b84d8 Add statx and other syscall numbers
    5ca627b78 tutorial: Add a step-by-step C test tutorial

So we have told git to show all the commits which don't exist in ``master``, but
are in ``HEAD``, where ``HEAD`` is the top of the current branch. The current
branch is ``tutorial-rebase2`` which I just created. I have already done one
``rebase`` and submitted a patch for review, so my original branch was just called
``tutorial``.

As usual my commit history is starting to look like a bit of mess! There is
even a commit in there which should not be in the this branch (Remove old API
argument), however it can be ignored for now and 'cherry picked' into a new branch
later.

For my patch I actually need at least two commits, one which contains the
tutorial text and one which contains the test and associated files. So first
of all I want to 'squash' (amalgamate) all the commits appended with
``tutorial:`` into the bottom commit.

.. code-block:: bash

    $ git rebase -i 5ca627b78\^
    ...

This begins an interactive ``rebase`` where commit ``5ca6427b78`` is the earliest
commit we want to edit. The ``^`` symbol after the commit hash, specifies the
commit before this one. The interactive ``rebase`` command takes the last commit
we want to keep unaltered as it's argument (in other words it takes a
non-inclusive range).

Upon entering a similar command you will be presented with a text file
similar to the following. The file should be displayed in your text editor of
choice, if it doesn't, then you may change the editor variable in
``.gitconfig``.

.. code-block:: bash

    pick 5ca627b78 tutorial: Add a step-by-step C test tutorial
    pick 5d93b84d8 Add statx and other syscall numbers
    pick 6f8c16438 statx: stage 3 (Add statx01)
    pick 98f5bc7ac statx: stage 4
    pick 76e03d714 statx: stage 5a
    pick bfeef7902 statx: stage 5b
    pick 09dd2c829 statx: stage 6
    pick 568a3f7be fixup! tutorial
    pick 1e24a5fb5 fixup! tutorial
    pick c26e1be7a fixup! tutorial
    pick d784b1e85 test-writing-guidelines: Remove old API argument
    pick 6a87a084a statx01: Fix review problems
    pick bb0332bd7 tutorial: Fix review problems
    pick 70f7ce7ce statx01: Stop checkpatch from complaining
    pick 152d39fe7 tutorial: Start Submitting patch section

The last commit from Git-log is shown at the top. The left hand column
contains the commands we want to run on each commit. ``pick`` just means we
re-apply the commit as-is. We can reorder the lines to apply the commits in a
different order, but we need to be careful when reordering commits to the same
file. If your ``rebase`` results in a merge conflict, then you have probably
reordered some commits which contained changes to the same piece of code.

Perhaps a better name for the interactive ``rebase`` command would be 'replay'. As
we pick a point in the commit history, undo all those commits before that
point, then reapply them one at a time. During the replay we can reorder the
commits, drop, merge, split and edit them, creating a new history.

The commands I am going to use are ``reword`` and ``fixup``. The ``reword`` command
allows you to edit a single commit's message. The 'fixup' command 'squashes' a
commit into the commit above/preceding it, merging the two commits into
one. The commit which has ``fixup`` applied has its commit message deleted. If
you think a commit might have something useful in its message then you can use
``squash`` instead.

.. code-block:: bash

    reword 5ca627b78 tutorial: Add a step-by-step C test tutorial
    fixup 568a3f7be fixup! tutorial
    fixup 1e24a5fb5 fixup! tutorial
    fixup c26e1be7a fixup! tutorial
    fixup bb0332bd7 tutorial: Fix review problems
    fixup 152d39fe7 tutorial: Start Submitting patch section
    fixup 276edecab tutorial: Save changes before rebase
    pick 5d93b84d8 Add statx and other syscall numbers
    pick 6f8c16438 statx: stage 3 (Add statx01)
    pick 98f5bc7ac statx: stage 4
    pick 76e03d714 statx: stage 5a
    pick bfeef7902 statx: stage 5b
    pick 09dd2c829 statx: stage 6
    pick d784b1e85 test-writing-guidelines: Remove old API argument
    pick 6a87a084a statx01: Fix review problems

So all the commits marked with ``fixup`` will be re-played by Git immediately
after 5ca62 at the top. A new commit will then be created with the amalgamated
changes of all the commits and 5ca62's log message. It turns out that I didn't
need to reword anything, but there is no harm in checking. It is easy to
forget the ``Signed-off-by:`` line.

I could now do the same for the commits to the ``statx`` test, making the commit
message prefixes consistent. However I am not actually going to submit the
test (yet).

I won't attempt to show you this, but if you need to do the opposite and split
apart a commit. It is also possible using Git-rebase by marking a line with
``edit``. This will pause Git just after replaying the marked commit. You can
then use a 'soft' Git-reset to bring the selected commit's changes back into
the 'index' where you are then able to un-stage some parts before
re-committing.

You can also use ``edit`` and ``git commit --amend`` together to change a commit
deep in your history, but without resetting the 'index'. The 'index' contains
changes which you have staged with ``git add``, but not yet committed.

So now that the commit history has been cleaned up, we need to submit a patch
to the mailing list or make a pull request on GitHub. The mailing list is the
preferred place to make submissions and is more difficult for most people, so
I will only cover that method.

Just before we create the patch, we need to check that our changes will still
apply to the master branch without problems. To do this we can use another
type of ``rebase`` and then try rebuilding and running the test.

.. code-block:: bash

    git checkout master
    git pull origin
    git checkout tutorial-rebase2
    git rebase master

Above, I update the master branch and then replay our changes onto it using
``git rebase master``. You may find that after the rebase there is a merge
conflict. This will result in something which looks like the following (taken
from a Makefile conflict which was caused by reordering commits in a ``rebase``).

.. code-block:: diff

    <<<<<<< HEAD
    cve-2016-7117:	LDFLAGS += -lpthread
    =======
    cve-2014-0196:	LDFLAGS += -lpthread -lutil -lrt
    cve-2016-7117:	LDFLAGS += -lpthread -lrt
    >>>>>>> 4dbfb8e79... Add -lrt

The first line tells us this is the beginning of a conflict. The third line
separates the two conflicting pieces of content and the last line is the end
of the conflict. Usually, all you need to do is remove the lines you don't
want, stage the changes and continue the ``rebase`` with ``git rebase
--continue``.

In order to create a patch e-mail we use
`git format-patch <https://git-scm.com/docs/git-format-patch>`_,
we can then send that e-mail using
`git send-email <https://git-scm.com/docs/git-send-email>`_.
It is also possible to import the patch (``mbox``) file into a number of e-mail
programs.

.. code-block:: bash

    $ git format-patch -1 -v 2 -o output --to ltp@lists.linux.it fd3cc8596
    output/v2-0001-tutorial-Add-a-step-by-step-C-test-tutorial.patch

The first argument ``-1`` specifies we want one commit from fd3cc8596
onwards. If we wanted this commit and the one after it we could specify ``-2``
instead.

This is my second patch submission so I have used ``-v 2``, which indicates this
is the second version of a patch set. The ``-o`` option specifies the output
directory (literally called ``output``). The ``--to`` option adds the ``To:`` e-mail
header, which I have set to the LTP mailing list.

We can then send this patch with the following command sans ``--dry-run``.

.. code-block:: bash

    git send-email --dry-run output/v2-0001-tutorial-Add-a-step-by-step-C-test-tutorial.patch

Git will ask some questions (which you can ignore) and then tell you what it
would do if this weren't a dry-run. In order for this to work you have to have
a valid SMTP server set in ``.gitconfig`` and also be signed up to the LTP
mailing list under the same e-mail address you have configured in Git. You can
sign up at https://lists.linux.it/listinfo/ltp.

Doing code review
-----------------

While waiting for your test to be reviewed, you are invited and encouraged to
review other contributors' code. This may seem bizarre when you are completely
new to the project, but there are two important ways in which you can
contribute here:

A.   Point out logical errors in the code.
B.   Improve your own understanding

It doesn't matter whether you know the canonical way of writing an LTP test in
C. An error of logic, when properly explained, is usually indisputable. These
are the most important errors to find as they always result in false test
results. Once someone points out such an error it is usually obvious to
everyone that it is a bug and needs to be fixed.

Obviously testing the patch is one way of finding errors. You can apply
patches using ``git am``. Then it is just a case of compiling and running the
tests.

Finally, reading and attempting to comment on other peoples patches, gives
you a better understanding of the reviewers perspective. This is better for
the project and for you.

Style and organizational issues are best left to after you have found logical
errors.

Final notes
-----------

Hopefully you can now grasp the structure of an LTP test and have some idea of
what is available in the LTP test library. There are a vast number of library
functions available (mainly located in include and lib), some of which are
documented in the test writing guidelines and many of which are not.

We have only scratched the surface of the immense technical complexity of
systems programming across multiple Kernel and C lib versions as well as
different hardware architectures. The important thing to take away from this
is that you have to be conscientious of what will happen on systems different
from yours. The LTP has a huge and varied user base, so situations you may
think are unlikely can and do happen to somebody.

Of course you don't want to spend time allowing for situations which may never
arise either, so you have to do your research and think about each situation
critically. The more systems you can test on before submitting your changes,
the better, although we understand not everyone has access to a lab.

One important topic which has not been covered by this tutorial, is
multi-process or multi-threaded testing. The LTP library functions work inside
child processes and threads, but their semantics change slightly. There are
also various helper functions for synchronizing and forking processes.

.. note::

    When it comes time to submit a test, the preferred way to do it is on the
    mailing list although you can also use GitHub. The LTP follows similar rules
    to the kernel for formatting and submitting patches. Generally speaking the
    review cycle is easier for small patches, so try to make small changes or
    additions where possible.

Linux Test Project
==================

Linux Test Project is a joint project started by SGI, OSDL and Bull developed
and maintained by IBM, Cisco, Fujitsu, SUSE, Red Hat, Oracle and others. The
project goal is to deliver tests to the open source community that validate the
reliability, robustness, and stability of Linux.

The LTP testsuite contains a collection of tools for testing the Linux kernel
and related features. Our goal is to improve the Linux kernel and system
libraries by bringing test automation to the testing effort. Interested open
source contributors are encouraged to join.

Project pages are located at: http://linux-test-project.github.io/

The latest image is always available at:
https://github.com/linux-test-project/ltp/releases

The discussion about the project happens at ltp mailing list:
http://lists.linux.it/listinfo/ltp

The git repository is located at GitHub at:
https://github.com/linux-test-project/ltp

Warning!
========

**Be careful with these tests!**

Don't run them on production systems. Growfiles, doio, and iogen in particular
stress the I/O capabilities of systems and while they should not cause problems
on properly functioning systems, they are intended to find (or cause) problems.

Quick guide to running the tests
================================

If you have git, autoconf, automake, m4, the linux headers and the common
developer packages installed, the chances are the following will work.

```
$ git clone https://github.com/linux-test-project/ltp.git
$ cd ltp
$ make autotools
$ ./configure
$ make
$ make install
```

This will install LTP to `/opt/ltp`.
* If you have a problem see `doc/mini-howto-building-ltp-from-git.txt`.
* If you still have a problem see `INSTALL` and `./configure --help`.
* Failing that, ask for help on the mailing list or Github.

Some tests will be disabled if the configure script can not find their build
dependencies.

* If a test returns `TCONF` due to a missing component, check the `./configure`
  output.
* If a tests fails due to a missing user or group, see the Quick Start section
  of `INSTALL`.

To run all the test suites

```
$ cd /opt/ltp
$ ./runltp
```

Note that many test cases have to be executed as root.

To run a particular test suite

```
$ ./runltp -f syscalls
```

To run all tests with `madvise` in the name

```
$ ./runltp -f syscalls -s madvise
```
Also see

```
$ ./runltp --help
```

Test suites (e.g. syscalls) are defined in the runtest directory. Each file
contains a list of test cases in a simple format, see doc/ltp-run-files.txt.

Each test case has its own executable or script, these can be executed
directly

```
$ testcases/bin/abort01
```

Some have arguments

```
$ testcases/bin/fork13 -i 37
```

The vast majority of test cases accept the -h (help) switch

```
$ testcases/bin/ioctl01 -h
```

Many require certain environment variables to be set

```
$ LTPROOT=/opt/ltp PATH="$PATH:$LTPROOT/testcases/bin" testcases/bin/wc01.sh
```

Most commonly, the path variable needs to be set and also `LTPROOT`, but there
are a number of other variables, `runltp` usually sets these for you.

Note that all shell scripts need the `PATH` to be set. However this is not
limited to shell scripts, many C based tests need environment variables as
well.

Developers corner
=================

Before you start you should read following documents:

* `doc/test-writing-guidelines.txt`
* `doc/build-system-guide.txt`

There is also a step-by-step tutorial:

* `doc/c-test-tutorial-simple.txt`

If something is not covered there don't hesitate to ask on the LTP mailing
list. Also note that these documents are available online at:

https://github.com/linux-test-project/ltp/wiki/Test-Writing-Guidelines
https://github.com/linux-test-project/ltp/wiki/BuildSystem
https://github.com/linux-test-project/ltp/wiki/C-Test-Case-Tutorial

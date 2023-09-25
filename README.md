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

The discussion about the project happens at LTP mailing list:
http://lists.linux.it/listinfo/ltp

LTP mailing list is archived at:
https://lore.kernel.org/ltp/

IRC #ltp at: [irc.libera.chat](https://libera.chat/)

The git repository is located at GitHub at:
https://github.com/linux-test-project/ltp

The patchwork instance is at:
https://patchwork.ozlabs.org/project/ltp/list/

Warning!
========

**Be careful with these tests!**

Don't run them on production systems. Growfiles, doio, and iogen in particular
stress the I/O capabilities of systems and while they should not cause problems
on properly functioning systems, they are intended to find (or cause) problems.

Quick guide to running the tests
================================

If you have git, autoconf, automake, m4, pkgconf / pkg-config, libc headers,
linux kernel headers and other common development packages installed (see
INSTALL and ci/*.sh), the chances are the following will work:

```
$ git clone https://github.com/linux-test-project/ltp.git
$ cd ltp
$ make autotools
$ ./configure
```

Now you can continue either with compiling and running a single test or with
compiling and installing the whole testsuite.

For optional library dependencies look into scripts for major distros in
`ci/` directory. You can also build whole LTP with `./build.sh` script.

Shortcut to running a single test
---------------------------------
If you need to execute a single test you actually do not need to compile
the whole LTP, if you want to run a syscall testcase following should work.

```
$ cd testcases/kernel/syscalls/foo
$ make
$ PATH=$PATH:$PWD ./foo01
```

Shell testcases are a bit more complicated since these need a path to a shell
library as well as to compiled binary helpers, but generally following should
work.

```
$ cd testcases/lib
$ make
$ cd ../commands/foo
$ PATH=$PATH:$PWD:$PWD/../../lib/ ./foo01.sh
```

Open Posix Testsuite has it's own build system which needs Makefiles to be
generated first, then compilation should work in subdirectories as well.

```
$ cd testcases/open_posix_testsuite/
$ make generate-makefiles
$ cd conformance/interfaces/foo
$ make
$ ./foo_1-1.run-test
```

Compiling and installing all testcases
--------------------------------------

```
$ make
$ make install
```

This will install LTP to `/opt/ltp`.
* If you have a problem see `INSTALL` and `./configure --help`.
* Failing that, ask for help on the mailing list or Github.

Some tests will be disabled if the configure script can not find their build
dependencies.

* If a test returns `TCONF` due to a missing component, check the `./configure`
  output.
* If a tests fails due to a missing user or group, see the Quick Start section
  of `INSTALL`.

Running tests
-------------

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
$ testcases/bin/mesgq_nstest -m none
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

For more info see `doc/User-Guidelines.asciidoc` or online at
https://github.com/linux-test-project/ltp/wiki/User-Guidelines.

Network tests
-------------
Network tests require certain setup, described in `testcases/network/README.md`
(online at https://github.com/linux-test-project/ltp/tree/master/testcases/network).

Containers
----------

*Presently running the LTP inside a container is not a shortcut. It
will make things much harder for you.*

There is a Containerfile which can be used with Docker or
Podman. Currently it can build Alpine and OpenSUSE images.

The container can be built with a command like:

```
$ podman build -t tumbleweed/ltp \
       --build-arg PREFIX=registry.opensuse.org/opensuse/ \
       --build-arg DISTRO_NAME=tumbleweed \
       --build-arg DISTRO_RELEASE=20230925 .
```

Or just `podman build .` which will create an Alpine container.

It contains Kirk in /opt/kirk. So the following will run some tests.

```
$ podman run -it --rm tumbleweed/ltp:latest
$ cd /opt/kirk && ./kirk -f ltp -r syscalls
```

SUSE also publishes a
[smaller LTP container](https://registry.opensuse.org/cgi-bin/cooverview?srch_term=project%3D%5Ebenchmark+container%3D.*)
that is not based on the Containerfile.

Developers corner
=================

Before you start you should read following documents:

* `doc/Test-Writing-Guidelines.asciidoc`
* `doc/Build-System.asciidoc`
* `doc/LTP-Library-API-Writing-Guidelines.asciidoc`

There is also a step-by-step tutorial:

* `doc/C-Test-Case-Tutorial.asciidoc`

If something is not covered there don't hesitate to ask on the LTP mailing
list. Also note that these documents are available online at:

* https://github.com/linux-test-project/ltp/wiki/Test-Writing-Guidelines
* https://github.com/linux-test-project/ltp/wiki/LTP-Library-API-Writing-Guidelines
* https://github.com/linux-test-project/ltp/wiki/Build-System
* https://github.com/linux-test-project/ltp/wiki/C-Test-Case-Tutorial

Although we accept GitHub pull requests, the preferred way is sending patches to our mailing list.

It's a good idea to test patches on GitHub Actions before posting to mailing
list. Our GitHub Actions setup covers various architectures and distributions in
order to make sure LTP compiles cleanly on most common configurations.
For testing you need to just push your changes to your own LTP fork on GitHub.

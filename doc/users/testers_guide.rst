.. SPDX-License-Identifier: GPL-2.0-or-later

Testers guide to the Linux test project
=======================================

While we try to make LTP work out of the box as much as possible there are
still many things that testers need to consider before the actual testing
starts. It's advisable to make a test plan in order to asses and formalize the
expected test coverage or even just sit down for a while and consider different
aspects of the problem at hand.


Is testing even required?
-------------------------

Some may argue that testing the Linux kernel locally is unnecessary because it
is already thoroughly tested upstream and considered stable. While it's true
that upstream releases generally go through extensive validation, including
test suites like LTP, stability is only guaranteed when you use the upstream
kernel sources and configuration exactly as released.

This assumption breaks down once you apply any changes: whether that's
modifying the source code, enabling/disabling different `.config` options, or
backporting patches. Such changes can introduce subtle bugs or unintended
behavior, even if the upstream kernel is stable.

For example, backporting patches without their full dependency chain can lead
to unexpected regressions. Therefore, it’s crucial to test your own kernel
builds in the environment where they will actually run, using tools like LTP to
catch issues that does not exists or are not triggered in the upstream
configuration.


Multi dimensionality
--------------------

First of all kernel testing is a multi dimensional problem, just compiling and
running LTP will give you some coverage but very likely not enough. There are
several big gaps that may be easily missed.

For example 64bit Linux kernel provides compatibility layer for 32bit
applications whose code quality is usually a bit worse than the 64bit ABI.
Hence recompiling LTP with `-m32` in compiler flags and running both 64bit and
32bit test binaries is a good start. If you try to make an argument that your
application does not need 32bit support it's better to disable the compat layer
completely since it's possible source of security bugs.

Another dimension is the number of architectures you need to test, for a
general distribution testing you may end up with a couple of them. Different
architectures have different platform code as well as differences in memory
orderings, etc. that all means that running tests on one architecture out of
several will give you incomplete coverage.

Since most of the POSIX API deals with files, the choice of filesystem for the
testing changes the focus and coverage too. LTP defaults to using `/tmp/` as a
temporary directory for the tests. If `/tmp/` is mounted as tmpfs subset of
tests will be skipped, if that is the case it's advisable to point environment
variable `TMPDIR` to a path with a different filesystem instead. Then there are
tests that format a device with a filesystem. LTP defaults to `ext2` and loop
devices for these testcases, that can be changed with environment variables as
well. Lastly but not least a few testcases repeat the test for all supported
filesystem, if you are interested in testing on a single filesystem only, you
can limit these tests to a single filesystem too. See the tests setup for a
comprehensive list of the `evironment variables
<setup_tests.html#library-environment-variables>`_.

Then you also have to decide if you are going to run tests in virtual machine
e.g. `qemu-kvm`, on bare metal or both. Testing in virtual machine will give you
about 90% of the coverage for bare metal and vice versa.

There are other options worth of consideration too, Linux kernel has many
debugging options that are usually disabled on runtime since they incur
significant performance penalty. Having a few more LTP test runs with different
debug options enabled e.g. `KASAN
<https://www.kernel.org/doc/html/latest/dev-tools/kasan.html>`_ or `KMEMLEAK
<https://www.kernel.org/doc/html/latest/dev-tools/kmemleak.html>`_ may help
catch bugs before they materialize in production.

In practice your test matrix may easily explode and you may end up with dozens
of differently configured testruns based on different considerations. The hard
task at hand is not to have too many since computing power is not an infinite
resource and does not scale that easily. If you managed to read up to this
point *"Don't Panic!"* things are almost never as bad as they may seem at first
glance.

It's a good idea to start small with an environment that models your
production.  Once that works well you can try different configurations. Select
a few interesting ones and run them for some time in order to get an idea of
their usefulness. If you are feeling adventurous you may try to measure and
compare actual test coverage with one of the tools such as `gcov
<https://www.kernel.org/doc/html/latest/dev-tools/gcov.html>`_ and `lcov
<https://github.com/linux-test-project/lcov>`_. If you do so do not fall into a
trap of attempting to have 100% line coverage. Having 100% of lines executed
during the test does not mean that your test coverage is 100%.  Good tests
validate much more than just how much code from the tested binary was executed.

You may need to sacrifice some coverage in order to match the tests runtime to
the available computing power. When doing so `Pareto principle
<https://en.wikipedia.org/wiki/Pareto_principle>`_ is your friend.


Test scope
----------

So far we were talking about a code coverage from a point of maximizing test
coverage while keeping our test matrix as small as possible. While that is a
noble goal it's not the universal holy grail of testing. Different use cases
have different considerations and scope. For a testing before a final release
such testing is very desirable, however for a continuous integration or smoke
testing the main requirement is that feedback loops are as short as possible.

When a developer changes the kernel and submits the changes to be merged it's
desirable to run some tests. Again the hard question is which tests. If we run
all possible tests in all possible combinations it may take a day or two and
the developer will move to a different tasks before the tests have a chance to
finish. If you multiply that by a number of developers in the team you may end
up in a situation where a developer will retire before tests for his patch may
have had a chance to finish.

In this case careful selection of tests is even more important. Having less is
more in this context. One of the first ideas for CI is to skip tests that run
for more than a second or so, happily this can be easily done with `kirk
<https://github.com/linux-test-project/kirk/>`_. In the future we may want to
explore some heuristics that would map the code changes in kernel into a subset
of tests, which would allow for a very quick feedback.


Debugging test failures
-----------------------

You may think that you will enjoy some rest once you have your test matrix
ready and your tests are running. Unfortunately that's where the actual work
starts. Debugging test failures is probably the hardest part of the testing
process. In some cases failures are easily reproducible and it's not that hard
to locate the bug, either in the test or in the kernel itself. There are
however, quite common, cases where the test failure reproduces only in 10% or
even 1% of the test runs. Sometimes tests are not failing in isolation, that is
because operating system has a huge internal state and a test failure manifests
only after running right sequence of tests. All of that does not mean that
there is no bug, that usually means that the bug depends on more prerequisites
that have to manifest at the right time in order to trigger the failure. Sadly
for modern systems that are asynchronous in nature such bugs are more and more
common.

The debugging process itself is not complicated by its nature. You have to
attempt to understand the failure by checking the logs, reading and
understanding the source code, debugging with strace, gdb, etc. Then form a
hypothesis and either prove or disprove it. Rinse and repeat until you end up
with a clear description of what went wrong. Hopefully you will manage to find
the root cause, but you should not be discouraged, if you do not. Debugging
kernel bugs takes a lot of experience and skill one can say as much as is
needed to write the kernel code.


Happy testing!

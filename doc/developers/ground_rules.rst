.. SPDX-License-Identifier: GPL-2.0-or-later

Ground Rules
============

Do not work around kernel bugs
------------------------------

We have decided that we will not work around bugs in upstream LTP sources. If a
test fails on your system for a good reason, e.g. patch wasn't backported and
the bug is present, work around for this will not be accepted upstream. The
main reason for this decision is that this masks the failure for everyone else.


Do not synchronize by sleep
---------------------------

Why is sleep in tests bad then?
```````````````````````````````

The first problem is that it will likely introduce very rare test failures,
that means somebody has to spend time looking into these, which is a wasted
effort. Nobody likes tests that will fail rarely for no good reason. Even more
so you cannot run such tests with a background load to ensure that everything
works correctly on a busy system, because that will increase the likehood of a
failure.

The second problem is that this wastes resources and slows down a test run. If
you think that adding a sleep to a test is not a big deal, lets have a look at
the bigger perspective. There are about 1600 syscall tests in Linux Test
Project, if 7.5% of them would sleep just for one second, we would end up with
two minutes of wasted time per testrun. In practice most of the tests, that
historically misused sleep for synchronization, waited for much longer just to
be sure that things will works even on slower hardware. With sleeps between 2
and 5 seconds that puts us somewhere between 4 and 10 minutes which is between
13% and 33% of the syscall runtime on an outdated thinkpad, where the run
finishes in a bit less than half an hour. It's even worse on newer hardware,
because this slowdown will not change no matter how fast your machine is, which
is maybe the reason why this was acceptable twenty years ago but it's not now.


What to do instead?
```````````````````

Use proper synchronization.

There are different problems and different solutions. Most often test needs to
synchronize between child and parent process.

The easiest case is when parent needs to wait for a child to finish, that can
be fixed just be adding a :manpage:`waitpid(2)` in the parent which ensures that child
has finished before parent runs.

Commonly child has to execute certain piece of code before parent can continue.
For that LTP library implements checkpoints with simple
:c:func:`TST_CHECKPOINT_WAIT()` and :c:func:`TST_CHECKPOINT_WAKE()` functions based
on futexes on a piece of shared memory set up by the test library.

Another common case is when a child must sleep in a syscall before parent can
continue, for which we have a :c:func:`TST_PROCESS_STATE_WAIT()` helper that
polls `/proc/$PID/stat`.

Less often test needs to wait for an action that is done asynchronously, or for
a kernel resource deallocation that is deferred to a later time. In such cases
the best we can do is to poll. In LTP we ended up with a macro that polls by
calling a piece of code in a loop with exponentially increasing sleeps between
retries until it succeeds. Which means that instead of sleeping for a maximal
time event can possibly take the sleep is capped by twice of the optimal
sleeping time while we avoid polling too aggressively.


Use runtime checks for kernel features
--------------------------------------

What is and what isn't supported by kernel is determined by the version and
configuration of the kernel the system is currently running on.  That
especially means that any checks done during the compilation cannot be used to
assume features supported by the kernel the tests end up running on. The
compile time checks, done by :master:`configure.ac` script, are only useful for
enabling fallback kernel API definitions when missing, as we do in
:master:`include/lapi/` directory.


Don't require root unless it's essential
----------------------------------------

If root/caps are needed, say why in the test doc comment. Drop privileges for
the part that doesn't need them and avoid running the whole test as root
“because it's easier”.


Always clean up, even on failure
--------------------------------

Every test should leave the system as it found it: unmount, restore sysctls,
delete temp files/dirs, kill spawned processes, remove cgroups/namespaces,
detach loop devices, restore ulimits, etc. Cleanup must run on early-exit
paths too.

The test library can simplify cleanup greatly as there are various helpers such as:

- :c:type:`.needs_tmpdir = 1 <tst_test>` that creates and deletes a temporary directory for the test
- :c:type:`.save_restore = 1 <tst_test>` that saves and restores /sys/ and /proc/ files
- :c:type:`.needs_device = 1 <tst_test>` sets up and tears down a block device for the test
- :c:type:`.restore_wallclock = 1 <tst_test>` that restores wall clock after the test
- :c:type:`.needs_cgroup_ctrls = 1 <tst_test>` sets up and cleans up cgroups for the test
- And many more.


Write portable code
-------------------

Avoid nonstandard libc APIs when a portable equivalent exists; don't assume
64-bit, page size, endianness, or particular tool versions.

If the test is specific to a certain architecture, make sure that it at least
compiles at the rest of architectures and set the
:c:type:`.supported_archs = const char *const []){"s390x", ..., NULL} <tst_test>`.

This also applies to shell code where it's easy to use bash features that are
not available on other shell implementations, e.g. dash or busybox. Make sure
to stick to portable POSIX shell whenever possible.

You can check for common mistakes, not only in portability, with our
``make check`` tooling.


Split changed into well defined chunks
--------------------------------------

When submitting patches make sure to split the work into small well-defined
chunks. Patches that touch many files or mix unrelated changes and features are
difficult to review and are likely to be delayed or even ignored.

Aim for a single logical change per patch. Split more complex works into a
patch series where each patch:

  - builds/compiles successfully
  - keeps tests and tooling functional
  - does not introduce intermediate breakage
  - has a clear commit message to explain the change
  - significant changes need to be detailed in the cover letter


Be careful when using AI tools
------------------------------

AI tools can be useful for executing, summarizing, or suggesting approaches,
but they can also be confidently wrong and give an illusion of correctness.
Treat AI output as untrusted: verify claims against the code, documentation,
and actual behavior on a reproducer.

Do not send AI-generated changes as raw patches. AI-generated diffs often
contain irrelevant churn, incorrect assumptions, inconsistent style, or subtle
bugs, which creates additional burden for maintainers to review and fix.

Best practice is to write your own patches and have them reviewed by AI before
submitting them, which helps add beneficial improvements to your work.


Kernel features and RCs
-----------------------

LTP tests or fixes for kernel changes that have not yet been released may be
posted to the LTP list for a review but they will not be be accepted until
respective kernel changes are released. Review of such changes is also
considered to be lower priority than rest of the changes. This is because
kernel changes especially in the early RC phase are volatile and could be
changed or reverted.

These patches should also add a [STAGING] keyword into the patch subject, e.g.
"Subject: [PATCH v1][STAGING] fanotify: add test for <feature> (requires v6.19-rc3)"

In a case that a test for unreleased kernel is really needed to be merged we do
not add it to the list of test executed by default and keep it in
:master:`runtest/staging` file until the kernel code is finalized.

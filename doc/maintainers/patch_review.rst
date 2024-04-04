.. SPDX-License-Identifier: GPL-2.0-or-later

Patch review
============

Anyone can and should review patches. It's the only way to get good at patch
review and for the project to scale. For this reason, we have a short guide on
what to do during the review process.

Goals of patch review
---------------------

#. Prevent false positive test results
#. Prevent false negative test results
#. Keep the code as simple as possible, but no simpler

How to find clear errors
------------------------

A clear error is one where there is unlikely to be any argument if you
provide evidence of it. Evidence being an error trace or logical proof
that an error will occur in a common situation.

The following are examples and may not be appropriate for all tests.

* Merge the patch locally. It should apply cleanly to master.
* Compile the patch with default and non-default configurations.

  * Use sanitizers e.g. undefined behavior, address.
  * Compile on non-x86
  * Compile on x86 with ``-m32``
  * Compile testing patches with GitHub Actions in LTP repo fork can cover
    various distros/architectures

* Use ``make check``
* Run effected tests in a VM

  * Use single vCPU
  * Use many vCPUs and enable NUMA
  * Restrict RAM to < 1GB.

* Run effected tests on an embedded device
* Run effected tests on non-x86 machine in general
* Run reproducers on a kernel where the bug is present
* Run tests with ``-i0`` and ``-i2``
* Compare usage of system calls with man page descriptions
* Compare usage of system calls with kernel code
* Double check commit message
* Search the LTP library for existing helper functions
* Check doc formatting, i.e. ``make doc && chromium docparse/metadata.html``

How to find subtle errors
-------------------------

A subtle error is one where you can expect some argument because you
do not have clear evidence of an error. It is best to state these as
questions and not make assertions if possible.

Although if it is a matter of style or "taste" then senior maintainers
can assert what is correct to avoid bike shedding.

* Ask what happens if there is an error, could it be debugged just
  with the test output?
* Are we testing undefined behavior?

  * Could future kernel behavior change without "breaking userland"?
  * Does the kernel behave differently depending on hardware?
  * Does it behave differently depending on kernel configuration?
  * Does it behave differently depending on the compiler?
  * Would it behave differently if the order of checks on syscall parameters
    changed in the kernel?

* Will it scale to tiny and huge systems?

  * What happens if there are 100+ CPUs?
  * What happens if each CPU core is very slow?
  * What happens if there are 2TB of RAM?

* Are we repeating a pattern that can be turned into a library function?
* Is a single test trying to do too much?
* Could multiple similar tests be merged?
* Race conditions

  * What happens if a process gets preempted?
  * Could checkpoints or fuzzsync by used instead?
  * Note, usually you can insert a sleep to prove a race condition
    exists however finding them is hard

* Is there a simpler way to achieve the same kernel coverage?

How to get patches merged
-------------------------

Once you think a patch is good enough you should add your ``Reviewed-by``
and/or ``Tested-by`` tags. This means you will get some credit for getting
the patch merged. Also some blame if there are problems.

If you ran the test you can add the ``Tested-by`` tag. If you read the
code or used static analysis tools on it, you can add the Reviewed-by
tag.

In addition you can expect others to review your patches and add their
tags. This will speed up the process of getting your patches merged.

Maintainers Checklist
---------------------

Patchset should be tested locally and ideally also in maintainer's fork in
GitHub Actions on GitHub.

.. note::

    GitHub Actions do only build testing, passing the CI means only that
    the test compiles fine on variety of different distributions and releases.

The test should be executed at least once locally and should PASS as well.

Commit messages should have

* Author's ``Signed-off-by`` tag
* Committer's ``Reviewed-by`` or ``Signed-off-by`` tag
* Check also mailing lists for other reviewers / testers tags, notes and failure
  reports
* ``Fixes: hash`` if it fixes particular LTP commit
* ``Fixes: #N`` if it fixes github issue number N, so it's automatically closed
* LTP documentation should be kept up to date.

After patch is accepted or rejected, set correct state and archive in the
`LTP patchwork instance <https://patchwork.ozlabs.org/project/ltp/list/>`_.

New tests
---------

New test should

* Have a record in runtest file
* Test should work fine with more than one iteration (e.g. run with ``-i 100``)
* Run with ``-i 0`` to check that setup and cleanup are coded properly
  (no test is being run)
* Have a brief description
* License: the default license for new tests is GPL v2 or later, use
  ``GPL-2.0-or-later``; the license for test (e.g. GPL-2.0) should not change
  unless test is completely rewritten
* Old copyrights should be kept unless test is completely rewritten

C tests
~~~~~~~

* Use the new C API, implementing ``struct tst_test``
* Test binaries are added into corresponding ``.gitignore`` files
* Check coding style with ``make check``
* Docparse documentation
* If a test is a regression test it should include ``.tags`` in the
  ``struct tst_test`` definition
* When rewriting old tests, `uClinux <https://en.wikipedia.org/wiki/%CE%9CClinux>`_
  support should be removed (project has been discontinued).
  E.g. remove ``#ifdef UCLINUX``, replace ``FORK_OR_VFORK()`` with simple
  ``fork()`` or ``SAFE_FORK()``.

Shell tests
~~~~~~~~~~~

* Use new shell API
* Check coding style with ``make check``
* If a test is a regression test it should include related kernel or glibc
  commits as a comment

LTP library
~~~~~~~~~~~

For patchset touching the LTP library, follow the LTP library guidelines.

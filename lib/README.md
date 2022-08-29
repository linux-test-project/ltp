# Test library design document

## High-level picture

    library process
    +----------------------------+
    | main                       |
    |  tst_run_tcases            |
    |   do_setup                 |
    |   for_each_variant         |
    |    for_each_filesystem     |   test process
    |     fork_testrun ------------->+--------------------------------------------+
    |      waitpid               |   | testrun                                    |
    |                            |   |  do_test_setup                             |
    |                            |   |   tst_test->setup                          |
    |                            |   |  run_tests                                 |
    |                            |   |   tst_test->test(i) or tst_test->test_all  |
    |                            |   |  do_test_cleanup                           |
    |                            |   |   tst_test->cleanup                        |
    |                            |   |  exit(0)                                   |
    |   do_exit                  |   +--------------------------------------------+
    |    do_cleanup              |
    |     exit(ret)              |
    +----------------------------+

## Test lifetime overview

When a test is executed the very first thing to happen is that we check for
various test prerequisites. These are described in the tst\_test structure and
range from simple '.require\_root' to a more complicated kernel .config boolean
expressions such as: "CONFIG\_X86\_INTEL\_UMIP=y | CONFIG\_X86\_UMIP=y".

If all checks are passed the process carries on with setting up the test
environment as requested in the tst\_test structure. There are many different
setup steps that have been put into the test library again ranging from rather
simple creation of a unique test temporary directory to a bit more complicated
ones such as preparing, formatting, and mounting a block device.

The test library also intializes shrared memory used for IPC at this step.

Once all the prerequisites are checked and test environment has been prepared
we can move on executing the testcase itself. The actual test is executed in a
forked process, however there are a few hops before we get there.

First of all there are test variants, which means that the test is re-executed
several times with a slightly different setting. This is usually used to test a
family of similar syscalls, where we test each of these syscalls exactly the
same, but without re-executing the test binary itself. Test variants are
implemented as a simple global variable counter that gets increased on each
iteration. In a case of syscall tests we switch between which syscall to call
based on the global counter.

Then there is all\_filesystems flag which is mostly the same as test variants
but executes the test for each filesystem supported by the system. Note that we
can get cartesian product between test variants and all filesystems as well.

In a pseudo code it could be expressed as:

```
for test_variants:
	for all_filesystems:
		fork_testrun()
```

Before we fork() the test process the test library sets up a timeout alarm and
also a heartbeat signal handlers and also sets up an alarm(2) accordingly to
the test timeout. When a test times out the test library gets SIGALRM and the
alarm handler mercilessly kills all forked children by sending SIGKILL to the
whole process group. The heartbeat handler is used by the test process to reset
this timer for example when the test functions run in a loop.

With that done we finally fork() the test process. The test process firstly
resets signal handlers and sets its pid to be a process group leader so that we
can slaughter all children if needed. The test library proceeds with suspending
itself in waitpid() syscall and waits for the child to finish at this point.

The test process goes ahead and calls the test setup() function if present in
the tst\_test structure. It's important that we execute all test callbacks
after we have forked the process, that way we cannot crash the test library
process. The setup can also cause the test to exit prematurely by either direct
or indirect (SAFE\_MACROS()) call to tst\_brk().  In this case the
fork\_testrun() function exits, but the loops for test variants or filesystems
carries on.

All that is left to be done is to actually execute the tests, what happnes now
depends on the -i and -I command line parameters that can request that the
run() or run\_all() callbacks are executed N times or for N seconds. Again the
test can exit at any time by direct or indirect call to tst\_brk().

Once the test is finished all that is left for the test process is the test
cleanup(). So if a there is a cleanup() callback in the tst\_test structure
it's executed. The cleanup() callback runs in a special context where the
tst\_brk(TBROK, ...) calls are converted into tst\_res(TWARN, ...) calls. This
is because we found out that carrying on with partially broken cleanup is
usually better option than exiting it in the middle.

The test cleanup() is also called by the tst\_brk() handler in order to cleanup
before exiting the test process, hence it must be able to cope even with
partial test setup. Usually it suffices to make sure to clean up only
resources that already have been set up and to do that in an inverse order that
we did in setup().

Once the test process exits or leaves the run() or run\_all() function the test
library wakes up from the waitpid() call, and checks if the test process
exited normally.

Once the testrun is finished the test library does a cleanup() as well to clean
up resources set up in the test library setup(), reports test results and
finally exits the process.

### Test library and fork()-ing

Things are a bit more complicated when fork()-ing is involved, however the test
results are stored in a page of a shared memory and incremented by atomic
operations, hence the results are stored right after the test reporting
function returns from the test library and the access is, by definition,
race-free as well.

On the other hand the test library, apart from sending a SIGKILL to the whole
process group on timeout, does not track grandchildren.

This especially means that:

- The test exits once the main test process exits.

- While the test results are, by the design, propagated to the test library
  we may still miss a child that gets killed by a signal or exits unexpectedly.

The test writer should, because of this, take care for reaping these proceses
properly, in most cases this could be simply done by calling
tst\_reap\_children() to collect and dissect deceased.

Also note that tst\_brk() does exit only the current process, so if a child
process calls tst\_brk() the counters are incremented and only the process
exits.

### Test library and exec()

The piece of mapped memory to store the results to is not preserved over
exec(2), hence to use the test library from a binary started by an exec() it
has to be remaped. In this case the process must to call tst\_reinit() before
calling any other library functions. In order to make this happen the program
environment carries LTP\_IPC\_PATH variable with a path to the backing file on
tmpfs. This also allows us to use the test library from shell testcases.

### Test library and process synchronization

The piece of mapped memory is also used as a base for a futex-based
synchronization primitives called checkpoints. And as said previously the
memory can be mapped to any process by calling the tst\_reinit() function. As a
matter of a fact there is even a tst\_checkpoint binary that allows us to use
the checkpoints from shell code as well.

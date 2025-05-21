.. SPDX-License-Identifier: GPL-2.0-or-later

Tests setup
===========

The internal LTP library provides a set of features that permits to customize
tests behavior by setting environment variables and using specific tests
arguments.

Library environment variables
-----------------------------

Following environment variables are expected to be set by LTP users. Therefore,
with some exceptions, they have ``LTP_`` prefix. Environment variables with
``TST_`` prefix are used inside LTP shell API and should **not** be set by
users.

.. list-table::
   :header-rows: 1

   * - Variable
     - Note

   * - KCONFIG_PATH
     - The path to the kernel config file, (if not set, it tries the usual paths
       ``/boot/config-RELEASE`` or ``/proc/config.gz``)

   * - KCONFIG_SKIP_CHECK
     - Skip kernel config check if variable set (not set by default)

   * - LTPROOT
     - Prefix for installed LTP.  **Should be always set**, since some tests
       need it to use data files (``LTP_DATAROOT``). LTP is by default installed
       into ``/opt/ltp``

   * - LTP_COLORIZE_OUTPUT
     - By default LTP colorizes it's output unless it's redirected to a pipe or
       file. Force colorized output behavior: ``y`` or ``1``: always colorize,
       ``n`` or ``0``: never colorize.

   * - LTP_DEV
     - Path to the block device to be used. C Language: ``.needs_device = 1``.
       Shell language: ``TST_NEEDS_DEVICE=1``.

   * - LTP_REPRODUCIBLE_OUTPUT
     - When set to ``1`` or ``y`` discards the actual content of the messages
       printed by the test (suitable for a reproducible output).

   * - LTP_SINGLE_FS_TYPE
     - Specifies single filesystem to run the test on instead all supported
       (for tests with ``.all_filesystems``).

   * - LTP_FORCE_SINGLE_FS_TYPE
     - Testing only. Behaves like LTP_SINGLE_FS_TYPE but ignores test skiplists.

   * - LTP_DEV_FS_TYPE
     - Filesystem used for testing (default: ``ext2``).

   * - LTP_TIMEOUT_MUL
     - Multiplies timeout, must be number >= 0.1 (> 1 is useful for slow
       machines to avoid unexpected timeout). It's mainly for shell API, which
       does not have LTP_RUNTIME_MUL. In C API it scales the default 30 sec
       safety margin, probably LTP_RUNTIME_MUL should be used instead.

   * - LTP_RUNTIME_MUL
     - Multiplies maximal test iteration runtime. Tests that run for more than a
       second or two are capped on runtime. You can scale the default runtime
       both up and down with this multiplier. This is not yet implemented in the
       shell API.

   * - LTP_IMA_LOAD_POLICY
     - Load IMA example policy, see :master:`testcases/kernel/security/integrity/ima/README.md`.

   * - LTP_VIRT_OVERRIDE
     - Overrides virtual machine detection in the test library. Setting it to
       empty string, tells the library that system is not a virtual machine.
       Other possible values are ``kvm``, ``xen``, ``zvm`` and ``microsoft``
       that describe different types supervisors.

   * - PATH
     - It's required to adjust path: ``PATH="$PATH:$LTPROOT/testcases/bin"``

   * - TMPDIR
     - Base directory for template directory (C language: ``.needs_tmpdir = 1``
       and shell: ``TST_NEEDS_TMPDIR=1``). Must be an absolute path (default:
       '/tmp').

   * - LTP_NO_CLEANUP
     - Disable running test cleanup (defined in ``TST_CLEANUP``).
       Shell API only.

   * - LTP_ENABLE_DEBUG
     - Enable debug info (value ``1`` or ``y``). Equivalent of ``-D`` parameter.

Environment variables for network tests
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
See :master:`testcases/network/README.md`.

Test execution time and timeout
-------------------------------

The limit on how long a test can run does compose of two parts: ``runtime``
and ``timeout``. The limit does apply to a single test variant. That means, for
example, that tests which run for all available filesystems will apply this
limit for a single filesystem only.

The ``runtime`` is a cap on how long the ``run()`` function can take and for
most testcases this part is set to zero. For tests that do run for more than a
second or two the ``runtime`` has to be defined and the ``run()`` function
has to check actively how much runtime is left.

Test runtime can be scaled up and down with ``LTP_RUNTIME_MUL`` environment
variable or set on a command-line by the ``-I`` parameter. However,
setting the runtime too low will cause long running tests to exit prematurely,
possibly before having a chance to actually test anything.

The timeout is a limit for test setup and cleanup and it's also a safety
margin for the runtime accounting. It's currently set to 30 seconds but it may
change later. If your target machine is too slow, it can be scaled up with the
``LTP_TIMEOUT_MUL`` environment variable.

.. SPDX-License-Identifier: GPL-2.0-or-later

LTP shell API
=============

Shell API overview
------------------

First lines of the shell test should be a shebang, a license, and copyrights.

.. code-block:: shell

   #!/bin/sh
   # SPDX-License-Identifier: GPL-2.0-or-later
   # Copyright 2099 Foo Bar <foo.bar@example.org>

A documentation comment block formatted in ReStructuredText should follow right
after these lines. This comment is parsed and exported into the LTP
documentation at https://linux-test-project.readthedocs.io/en/latest/users/test_catalog.html

.. code-block:: shell

   # ---
   # doc
   # Test for a foo bar.
   #
   # This test is testing foo by checking that bar is doing xyz.
   # ---

The shell loader test library uses the :doc:`../developers/api_c_tests`
internally by parsing a special JSON formatted comment and
initializing it accordingly. The JSON format is nearly 1:1 serialization of the
:ref:`struct tst_test` into a JSON. The environment must be always preset even
when it's empty.

.. code-block:: shell

   # ---
   # env
   # {
   #  "needs_root": true,
   #  "needs_tmpdir": true,
   #  "needs_kconfigs": ["CONFIG_NUMA=y"],
   #  "tags": {
   #   ["linux-git", "432fd03240fa"]
   #  }
   # }

After the documentation and environment has been laid out we finally import the
:shell_lib:`tst_loader.sh`. This will, among other things, start the
:shell_lib:`tst_run_shell.c` binary, that will parse the shell test environment
comment and initialize the C test library accordingly.

.. code-block:: shell

   . tst_loader.sh

At this point everything has been set up and we can finally write the test
function. The test results are reported by the usual functions :ref:`tst_res` and
:ref:`tst_brk`. As in the C API these functions store results into a piece of shared
memory as soon as they return so there is no need to propagate results event
from child processes.

.. code-block:: shell

   tst_test()
   {
        tst_res TPASS "Bar enabled Foo"
   }

In order for the test to be actually executed the very last line of the script
must source the :shell_lib:`tst_run.sh` script.

.. code-block:: shell

   . tst_run.sh

In order to run a test from a LTP tree a few directories has to be added to the
`$PATH`. Note that the number of `../` may depend on the depth of the current
directory relative to the LTP root.

.. code-block:: shell

   $ PATH=$PATH:$PWD:$PWD/../../lib/ ./foo01.sh

Test setup and cleanup
----------------------

The test setup and cleanup functions are optional and passed via variables.
Similarly to the C API the setup is executed exactly once at the start of the
test and the test cleanup is executed at the test end or when test was
interrupted by :ref:`tst_brk`.

   .. literalinclude:: ../../testcases/lib/tests/shell_loader_setup_cleanup.sh
      :language: shell

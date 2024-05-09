.. SPDX-License-Identifier: GPL-2.0-or-later

LTP Library guidelines
======================

General Rules
-------------

When we extend library API, we need to apply the same general rules that we use
when writing tests, plus:

#. LTP library tests must go inside :master:`lib/newlib_tests` directory
#. LTP documentation has to be updated according to API changes

Shell API
---------

API source code is in :master:`testcases/lib/tst_test.sh`,
:master:`testcases/lib/tst_security.sh` and :master:`testcases/lib/tst_net.sh`.

Changes in the shell API should not introduce uncommon dependencies
(use basic commands installed everywhere by default).

Shell libraries
~~~~~~~~~~~~~~~

Aside from shell API libraries in :master:`testcases/lib` directory, it's
worth putting common code for a group of tests into a shell library.
The filename should end with ``_lib.sh`` and the library should load
``tst_test.sh`` or ``tst_net.sh``.

Shell libraries should have conditional expansion for ``TST_SETUP`` or
``TST_CLEANUP``, to avoid surprises when test specific setup/cleanup function is
redefined by shell library.

.. code-block:: bash

    # ipsec_lib.sh
    # SPDX-License-Identifier: GPL-2.0-or-later
    TST_SETUP="${TST_SETUP:-ipsec_lib_setup}"
    ...
    . tst_test.sh

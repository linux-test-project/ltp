.. SPDX-License-Identifier: GPL-2.0-or-later

LTP Library guidelines
======================

General Rules
-------------

For extending the LTP library API it applies the same general rules as
for :doc:`writing tests <../developers/writing_tests>`
(with strong focus on readability and simplicity), plus:

#. LTP library tests must go inside :master:`lib/newlib_tests` directory
#. LTP documentation has to be updated according to API changes
#. Do not add new API functions to the old API. Add new functions to
   ``tst_.[ch]`` files.

Library naming and scope
------------------------

To keep the library API easy to navigate and to make layering explicit, LTP
library components follow these naming rules:

- **tst_**: Core LTP library API (located in :master:`lib/`).

  - Stable, widely used interfaces intended for general consumption by tests.
  - New public APIs should normally live here (in ``tst_*.h`` / ``tst_*.c``).

- **tse_**: Non-core / extended library code (located in :master:`libs/`).

  - Optional or specialized helpers that are not part of the core API.
  - May have narrower scope or fewer stability guarantees than ``tst_``.
  - Can be promoted to ``tst_`` later if it becomes broadly useful and stable.

- **tso_**: Legacy / old library code.

  - Kept for backward compatibility.
  - No new features should be added; only minimal fixes are acceptable
    (e.g. build fixes, correctness fixes, security fixes).
  - New code should not depend on ``tso_`` unless strictly necessary.

.. note::

   Prefer adding new code to ``tst_`` or ``tse_``; avoid introducing new ``tso_`` components.
   When adding a new public interface, document where it belongs (``tst_`` vs ``tse_``) and why.

Shell API
---------

API source code is in :shell_lib:`tst_test.sh`, :shell_lib:`tst_security.sh`
and :shell_lib:`tst_net.sh`.

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

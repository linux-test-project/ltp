.. SPDX-License-Identifier: GPL-2.0-or-later

Linux Test Project
==================

Linux Test Project is a joint project started by SGI, OSDL and Bull developed
and maintained by SUSE, Red Hat, Fujitsu, IBM, Cisco, Oracle and others. The
project goal is to deliver tests to the open source community that validate
reliability, robustness, and stability of the Linux Kernel.

The testing suites contain a collection of tools for testing the Linux kernel
and related features. Our goal is to improve the Linux kernel and system
libraries by bringing test automation.

.. warning::

   LTP tests shouldn't run in production systems. In particular,
   growfiles, doio, and iogen, stress the I/O capabilities of the systems and
   they are intended to find (or cause) problems.

Some references:

* `Documentation <http://linux-test-project.rtfd.io/>`_
* `Source code <https://github.com/linux-test-project/ltp>`_
* `Releases <https://github.com/linux-test-project/ltp/releases>`_
* `Mailing List <http://lists.linux.it/listinfo/ltp>`_
* `Working patches (patchwork) <https://patchwork.ozlabs.org/project/ltp/list/>`_
* `Working patches (lore.kernel.org) <https://lore.kernel.org/ltp>`_
* `#ltp @ libera chat <https://libera.chat/>`_

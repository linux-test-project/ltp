.. SPDX-License-Identifier: GPL-2.0-or-later

Build system
============

The following document briefly describes the steps and methodologies used for
the new and improved Makefile system.

The Problem
-----------

The problem with the old Makefile system is that it was very difficult to
maintain and it lacked any sense of formal structure, thus developing for LTP
and including new targets was more difficult than it should have been
(maintenance). Furthermore, proper option-based cross-compilation was
impossible due to the fact that the Makefiles didn't support a prefixing
system, and the appropriate implicit / static rules hadn't been configured to
compile into multiple object directories for out-of-tree build support (ease of
use / functionality). Finally, there wasn't a means to setup dependencies
between components, such that if a component required ``libltp.a`` in order to
compile, it would go off and compile ``libltp.a`` first (ease of use).

These items needed to be fixed to reduce maintenance nightmares for the
development community contributing to LTP, and the project maintainers.

Design
------

The system was designed such that including a single GNU Makefile compatible
set in each new directory component is all that's essentially required to
build the system.

Say you had a directory like the following (with ``.c`` files in them which
directly tie into applications, e.g. baz.c -> baz):

.. code-block::

    .../foo/
        |--> Makefile
        |
        --> bar/
            |
            --> Makefile
            |
            --> baz.c

.. code-block:: make
  :caption: .../foo/Makefile

    #
    # Copyright disclaimer goes here -- please use GPLv2.
    #

    top_srcdir		?= ..

    include $(top_srcdir)/include/mk/env_pre.mk
    include $(top_srcdir)/include/mk/generic_trunk_target.mk

.. code-block:: make
  :caption: .../foo/bar/Makefile

    #
    # Copyright disclaimer goes here -- please use GPLv2.
    #

    top_srcdir		?= ../..

    include $(top_srcdir)/include/mk/env_pre.mk
    include $(top_srcdir)/include/mk/generic_leaf_target.mk

Kernel Modules
--------------

Some of the tests need to build kernel modules, happily LTP has
infrastructure for this.

.. code-block:: make

    ifneq ($(KERNELRELEASE),)

    obj-m := module01.o

    else

    top_srcdir	?= ../../../..
    include $(top_srcdir)/include/mk/testcases.mk

    REQ_VERSION_MAJOR	:= 2
    REQ_VERSION_PATCH	:= 6
    MAKE_TARGETS		:= test01 test02 module01.ko

    include $(top_srcdir)/include/mk/module.mk
    include $(top_srcdir)/include/mk/generic_leaf_target.mk

    endif

This is a Makefile example that allows you to build kernel modules inside LTP.
The prerequisites for the build are detected by the ``configure`` script.

The ``REQ_VERSION_MAJOR`` and ``REQ_VERSION_PATCH`` describe minimal kernel
version for which the build system tries to build the module.

The build system is also forward compatible with changes in Linux kernel
internal API so that, if module fails to build, the failure is ignored both on
build and installation. If the userspace counterpart of the test fails to load
the module because the file does not exists, the test is skipped.

Note the ``ifneq($(KERNELRELEASE),)``. The reason it exists, it is that the
Makefile is executed twice: once by LTP build system and once by kernel kbuild,
see :kernel_doc:`kbuild/modules` in the Linux kernel documentation for details
on external module build.

Make Rules and Make Variables
-----------------------------

When using make rules, avoid writing ad hoc rules like:

.. code-block:: make

    [prog]: [dependencies]
        cc -I../../include $(CFLAGS) $(CPPFLAGS) $(LDFLAGS) $(LDLIBS) \
	    -o [prog] [dependencies]

This makes cross-compilation and determinism difficult, if not impossible.
Besides, implicit rules are your friends and as long as you use ``MAKEOPTS=;``
in the top-level caller (or do ``$(subst r,$(MAKEOPTS)``) to remove ``-r``),
the compile will complete successfully, assuming all other prerequisites have
been fulfilled (libraries, headers, etc).

.. list-table::
    :header-rows: 1

    * - Variable
      - Explanation

    * - $(AR)
      - The library archiver

    * - $(CC)
      - The system C compiler

    * - $(CCP)
      - The system C preprocessor

    * - $(CFLAGS)
      - C compiler flags

    * - $(CPPFLAGS)
      - Preprocessor flags, e.g. ``-I`` arguments

    * - $(DEBUG_CFLAGS)
      - Debug flags to pass to ``$(CC)``, ``-g``, etc

    * - $(KVM_LD)
      - Special linker for wrapping KVM payload binaries into linkable object
        files. Defaults to ``$(LD)``. Change this variable if the KVM Makefile
        fails to build files named ``*-payload.o``

    * - $(LD)
      - The system linker (typically ``$(CC)``, but not necessarily)

    * - $(LDFLAGS)
      - What to pass in to the linker, including ``-L`` arguments and other ld
        arguments, apart from ``-l`` library includes (see ``$(LDLIBS)``).
        This should be done in the ``$(CC)`` args passing style when
        ``LD := $(CC)``, e.g. ``-Wl,-foo``, as opposed to ``-foo``

    * - $(LDLIBS)
      - Libraries to pass to the linker (e.g. ``-lltp``, etc)

    * - $(LTPLDLIBS)
      - LTP internal libraries i.e. these in libs/ directory

    * - $(OPT_CFLAGS)
      - Optimization flags to pass into the C compiler, ``-O2``, etc. If you
        specify ``-O2`` or higher, you should also specify
        ``-fno-strict-aliasing``, because of gcc fstrict-aliasing optimization
        bugs in the tree optimizer. Search for **fstrict-aliasing optimization
        bug** with your favorite search engine.

        Examples of more recent bugs: tree-optimization/17510
        and tree-optimization/39100.

        Various bugs have occurred in the past due to buggy logic in the
        tree-optimization portion of the gcc compiler, from 3.3.x to 4.4.

    * - $(RANLIB)
      - What to run after archiving a library

    * - $(WCFLAGS)
      - Warning flags to pass to ``$(CC)``, e.g. ``-Werror``, ``-Wall``, etc.

Make System Variables
---------------------

A series of variables are used within the make system that direct what actions
need to be taken. Rather than listing the variables here, please refer to the
comments contained in :master:`include/mk/env_pre.mk`.

Guidelines and Recommendations
------------------------------

Of course, GNU Make manual is the key to understand the Make system, but
following manuals are probably the most important:

* `Implicit Rules <http://www.gnu.org/software/make/manual/make.html#Implicit-Rules>`_
* `Variables and Expansion <http://www.gnu.org/software/make/manual/make.html#Using-Variables>`_
* `Origin Use <http://www.gnu.org/software/make/manual/make.html#Origin-Function>`_
* `VPath Use <http://www.gnu.org/software/make/manual/make.html#Directory-Search>`_

.. warning::

    Rebuild from scratch before committing anything in the build system.

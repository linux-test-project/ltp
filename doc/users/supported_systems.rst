.. SPDX-License-Identifier: GPL-2.0-or-later

Supported systems
=================

LTP `master <https://github.com/linux-test-project/ltp/commits/master>`_
branch is build tested in
`GitHub Actions <https://github.com/linux-test-project/ltp/actions>`_.

.. note::

      There is no CI for the actual test runs.

Kernel version
--------------

Minimal supported kernel version is **4.4**.

Oldest build tested distributions
---------------------------------

.. list-table::
    :header-rows: 1

    * - Distro
      - Kernel
      - glibc
      - gcc
      - clang

    * - openSUSE Leap 42.2
      - 4.4
      - 2.22
      - 4.8.5
      - \-

    * - Ubuntu 18.04 LTS bionic
      - 4.15
      - 2.27
      - 7.3.0
      - \-

    * - Debian 11 (bullseye)
      - 5.10
      - 2.31
      - 10.2.1
      - 11.0.1

For a full list of build tested distros, please check :master:`.github/workflows/ci-docker-build.yml`.

Older distributions are not officially supported, which means that it
may or may not work. It all depends on your luck. It should be possible
to compile latest LTP even on slightly older distributions than we
support with a few manual tweaks, e.g. disabling manually tests for
newly added syscalls, etc. **Trivial fixes/workarounds may be accepted,
but users are encouraged to move to a newer distro.**

If latest LTP cannot be compiled even with some amount of workarounds,
you may result to older LTP releases, however these are **not** supported
in any way. Also if you are trying to run LTP on more than 10 years old
distribution you may as well reconsider you life choices.

Build tested architectures
--------------------------

.. list-table::
    :header-rows: 1

    * - Architecture
      - Build

    * - x86_64
      - native

    * - x86 emulation
      - native

    * - aarch64
      - cross compilation

    * - ppc64le
      - cross compilation

    * - s390x
      - cross compilation

Supported C libraries
---------------------

.. list-table::
    :header-rows: 1

    * - C library
      - Note

    * - `glibc <https://www.gnu.org/software/libc/>`_
      - Targeted libc, tested both compilation and actual test results.

    * - `uClibc-ng <https://uclibc-ng.org/>`_
      - Although not being tested, it should work as it attempt to maintain a glibc compatible interface.

    * - `uClibc <https://www.uclibc.org/>`_
      - Older uClibc might have problems.

    * - `musl <https://musl.libc.org/>`_
      - Not yet fully supported. Check :master:`ci/alpine.sh` script.

    * - Android
      - Please use `AOSP fork <https://android.googlesource.com/platform/external/ltp>`_.

C version
---------

LTP is compiled with ``-std=gnu99``.

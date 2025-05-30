.. SPDX-License-Identifier: GPL-2.0-or-later

Documentation
=============

This section explains how to use and develop the LTP documentation. The current
documentation format is written using
`reStructedText <https://www.sphinx-doc.org/en/master/usage/restructuredtext/index.html>`_
and it's built on top of `Sphinx <https://www.sphinx-doc.org/en/master/>`_.

Building documentation
~~~~~~~~~~~~~~~~~~~~~~

Before building, make sure you have python3 ``virtualenv`` module installed.

.. code-block:: bash

    # run configure to be able to compile doc dependencies in metadata/
    make autotools
    ./configure
    cd doc

    # prepare virtual environment
    python3 -m virtualenv .venv
    . .venv/bin/activate
    pip install -r requirements.txt

    # build documentation
    make

Once the procedure has been completed, documentation will be visible at
``doc/html/index.html``.

.. warning::

    Documentation requires ``Python >= 3.6``.
    The current :master:`.readthedocs.yml` workflow is using ``Python 3.12``,
    it is tested in GitHub Actions :master:`.github/workflows/ci-sphinx-doc.yml`.

Validating spelling
~~~~~~~~~~~~~~~~~~~

To check documentation words spelling, we provide support for
`aspell <http://aspell.net/>`_, so make sure that it's installed. The
documentation can be tested via ``make spelling`` command. Output will be
visible in the ``doc/build`` folder and, if any error will be found, a warning
message will be shown.

C API documentation
~~~~~~~~~~~~~~~~~~~

The C API documentation is generated from headers using
`kernel-doc <https://return42.github.io/linuxdoc/linuxdoc-howto/kernel-doc-syntax.html>`_
syntax which is supported by Sphinx via
`linuxdoc <https://pypi.org/project/linuxdoc/>`_ extension.

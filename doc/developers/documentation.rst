.. SPDX-License-Identifier: GPL-2.0-or-later

Documentation
=============

This section explains how to use and develop the LTP documentation. The current
documentation format is written using
`reStructedText <https://www.sphinx-doc.org/en/master/usage/restructuredtext/index.html>`_
and it's built on top of `Sphinx <https://www.sphinx-doc.org/en/master/>`_.

Building documentation
~~~~~~~~~~~~~~~~~~~~~~

First of all, to build the documentation we must be sure that all dependences
have been installed (please check ``doc/requirements.txt`` file). Sometimes the
Linux distros are providing them, but the best way is to use ``virtualenv``
command as following:

.. code-block:: bash

    cd doc

    # prepare virtual enviroment
    python -m virtualenv .venv
    source .venv/bin/activate

    pip install sphinx
    pip install -r requirements.txt

    # build documentation
    make

Once the procedure has been completed, documentation will be visible at
``doc/html/index.html``.

.. warning::

    The current ``.readthedocs.yml`` workflow is using ``Python 3.6`` because
    other Python versions were causing issues. No other version has been tested,
    but it should work anyway.

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

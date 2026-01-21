.. SPDX-License-Identifier: GPL-2.0-or-later

Setting up the Mailing list
===========================

Before using ``git send-email``, you need to set up your email client to send
emails from the command line. This typically involves configuring an SMTP server
and authentication details.

Open a terminal and configure Git with your email settings using the following
commands:

.. code-block:: bash

    git config --global sendemail.from "Your Name <your_email@example.com>"
    git config --global sendemail.smtpserver "smtp.example.com"
    git config --global sendemail.smtpuser "your_email@example.com"
    git config --global sendemail.smtpserverport 587
    git config --global sendemail.smtpencryption tls

Replace ``smtp.example.com`` with the SMTP server address provided by your email
provider. Replace ``your_email@example.com`` with your email address. Adjust the
SMTP port and encryption settings according to your email provider's
requirements.

To test the configuration you can use ``--dry-run`` parameter.

.. code-block:: bash

    git send-email --dry-run --to "ltp@lists.linux.it" --subject "Test Email" HEAD^

Depending on your SMTP server's configuration, you may need to authenticate
before sending emails. If required, configure authentication settings using:

.. code-block:: bash

    git config --global sendemail.smtpuser "your_email@example.com"
    git config --global sendemail.smtppass "your_password"

Replace ``your_email@example.com`` with your email address and ``your_password``
with your email account password.

For any corner case, please take a look at the
`email + git <https://git-send-email.io/>`_ documentation.

.. note::

    This method still works in most of the cases, but nowadays we often
    require to setup a two factor authentication. If this is the case, please
    consider setting up Git accordingly.

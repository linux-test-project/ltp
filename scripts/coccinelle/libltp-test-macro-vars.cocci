// SPDX-License-Identifier: GPL-2.0-or-later
// Copyright (c) 2021 SUSE LLC  <rpalethorpe@suse.com>

// Find violations of LTP-002
@ find_use exists @
expression E;
@@

(
* TST_ERR
|
* TST_RET
|
* TTERRNO | E
)

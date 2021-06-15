// SPDX-License-Identifier: GPL-2.0-or-later
// Copyright (c) 2021 SUSE LLC  <rpalethorpe@suse.com>

// The TEST macro should not be used in the library because it sets
// TST_RET and TST_ERR which are global variables. The test author
// only expects these to be changed if *they* call TEST directly.

// Find all positions where TEST's variables are used
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

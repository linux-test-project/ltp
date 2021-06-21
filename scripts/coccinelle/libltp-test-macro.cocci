// SPDX-License-Identifier: GPL-2.0-or-later
// Copyright (c) 2021 SUSE LLC  <rpalethorpe@suse.com>

// Find and fix violations of rule LTP-002

// Set with -D fix
virtual fix

// Find all positions where TEST is _used_.
@ depends on !fix exists @
@@

* TEST(...);

// Below are rules which will create a patch to replace TEST usage
// It assumes we can use the ret var without conflicts

// Fix all references to the variables TEST modifies when they occur in a
// function where TEST was used.
@ depends on fix exists @
@@

 TEST(...)

 <...

(
- TST_RET
+ ret
|
- TST_ERR
+ errno
|
- TTERRNO
+ TERRNO
)

 ...>

// Replace TEST in all functions where it occurs only at the start. It
// is slightly complicated by adding a newline if a statement appears
// on the line after TEST(). It is not clear to me what the rules are
// for matching whitespace as it has no semantic meaning, but this
// appears to work.
@ depends on fix @
identifier fn;
expression tested_expr;
statement st;
@@

  fn (...)
  {
- 	TEST(tested_expr);
+	const long ret = tested_expr;
(
+
	st
|

)
	... when != TEST(...)
  }

// Replace TEST in all functions where it occurs at the start
// Functions where it *only* occurs at the start were handled above
@ depends on fix @
identifier fn;
expression tested_expr;
statement st;
@@

  fn (...)
  {
- 	TEST(tested_expr);
+	long ret = tested_expr;
(
+
	st
|

)
	...
  }

// Add ret var at the start of a function where TEST occurs and there
// is not already a ret declaration
@ depends on fix exists @
identifier fn;
@@

  fn (...)
  {
+	long ret;
	... when != long ret;

	TEST(...)
	...
  }

// Replace any remaining occurrences of TEST
@ depends on fix @
expression tested_expr;
@@

- 	TEST(tested_expr);
+	ret = tested_expr;


// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2017 Richard Palethorpe <rpalethorpe@suse.com>
 * Copyright (c) 2012, Kees Cook <keescook@chromium.org>
 */
/*
 * Check that memory after the string terminator in all the utsname fields has
 * been zeroed. cve-2012-0957 leaked kernel memory through the release field
 * when the UNAME26 personality was set.
 *
 * Thanks to Kees Cook for the original proof of concept:
 * http://www.securityfocus.com/bid/55855/info
 */

#include <string.h>
#include <sys/utsname.h>
#include "tst_test.h"
#include "lapi/personality.h"

static struct utsname saved_buf;

static int check_field(char *bytes, char *saved_bytes, size_t length,
		       char *field)
{
	size_t i = strlen(bytes) + 1;

	for (; i < length; i++) {
		if (bytes[i] && (bytes[i] != saved_bytes[i])) {
			tst_res(TFAIL, "Bytes leaked in %s!", field);
			return 1;
		}
	}
	return 0;
}


static void try_leak_bytes(unsigned int test_nr)
{
	struct utsname buf;

	memset(&buf, 0, sizeof(buf));

	if (uname(&buf))
		tst_brk(TBROK | TERRNO, "Call to uname failed");

	if (!test_nr)
		memcpy(&saved_buf, &buf, sizeof(saved_buf));

#define CHECK_FIELD(field_name) \
	(check_field(buf.field_name, saved_buf.field_name, \
		     ARRAY_SIZE(buf.field_name), #field_name))

	if (!(CHECK_FIELD(release) |
	    CHECK_FIELD(sysname) |
	    CHECK_FIELD(nodename) |
	    CHECK_FIELD(version) |
	    CHECK_FIELD(machine) |
#ifdef HAVE_STRUCT_UTSNAME_DOMAINNAME
	    CHECK_FIELD(domainname) |
#endif
		    0)) {
		tst_res(TPASS, "No bytes leaked");
	}
#undef CHECK_FIELD
}

static void run(unsigned int test_nr)
{
	if (!test_nr) {
		tst_res(TINFO, "Calling uname with default personality");
	} else {
		SAFE_PERSONALITY(PER_LINUX | UNAME26);
		tst_res(TINFO, "Calling uname with UNAME26 personality");
	}

	try_leak_bytes(test_nr);
}

static struct tst_test test = {
	.test = run,
	.tcnt = 2,
};

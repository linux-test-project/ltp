// SPDX-License-Identifier: GPL-2.0-or-later

/*\
 * Test for cve group derived from the CVE tag.
 */

static struct tst_test test = {
	.tags = (const struct tst_tag[]) {
		{"CVE", "2017-16939"},
		{}
	}
};

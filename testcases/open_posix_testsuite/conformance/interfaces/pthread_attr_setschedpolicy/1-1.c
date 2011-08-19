/*
 * Copyright (c) 2002, Intel Corporation. All rights reserved.
 * Created by:  bing.wei.liu REMOVE-THIS AT intel DOT com
 * This file is licensed under the GPL license.  For the full content
 * of this license, see the COPYING file at the top level of this
 * source tree.

 * This test case initializes an attr and sets its schedpolicy,
 * then create a thread with the attr.

 *
 * Complete rewrite by Peter W. Morreale <pmorreale AT novell DOT com>
 * Date: 20/05/2011

 */

#include "common.h"

int main(void)
{
	int rc;
	struct params p;

	p.policy = SCHED_OTHER;
	p.priority = PRIORITY_OTHER;
	p.policy_label = "SCHED_OTHER";
	p.status = PTS_UNRESOLVED;
	rc = create_test_thread(&p);

	if (rc == PTS_PASS)
		printf("Test PASSED\n");

	return rc;
}

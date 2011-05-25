/*
 *
 *   Copyright (c) Novell Inc. 2011
 *
 *   This program is free software; you can redistribute it and/or
 *   modify it under the terms of the GNU General Public License
 *   as published by the Free Software Foundation; either version 2
 *   of the License, or (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY;  without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See
 *   the GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program;  if not, write to the Free Software
 *   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 *
 *   Author:  Peter W. Morreale <pmorreale AT novell DOT com>
 *
 *   Date:  20/05/2011
 */

#include "common.h"

int main(void)
{
	int rc;
	struct params p;

	p.policy = SCHED_RR;
	p.priority = PRIORITY_RR;
	p.policy_label = "SCHED_RR";
	p.status = PTS_UNRESOLVED;
	rc = create_test_thread(&p);

	if (rc == PTS_PASS)
		printf("Test PASS\n");

	return rc;
}

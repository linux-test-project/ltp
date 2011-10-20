/*
 *
 *   Copyright (c) Novell Inc. 2011
 *
 *   This program is free software;  you can redistribute it and/or modify
 *   it under the terms in version 2 of the GNU General Public License as
 *   published by the Free Software Foundation.
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
 *   Date:    11/08/2011
 */

/*
 * Must be ported to OS's other than Linux 
 */

#ifdef __linux__
#include <sched.h>

static int set_affinity(int cpu)
{
	cpu_set_t mask;

	CPU_ZERO(&mask);
	CPU_SET(cpu, &mask);

	return (sched_setaffinity(0, sizeof(cpu_set_t), &mask));
}
#else
static int set_affinity(int cpu)
{
	errno = ENOSYS;
	return -1;
}
#endif

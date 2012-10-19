/*
 *
 *   Copyright (c) International Business Machines  Corp., 2001
 *
 *   This program is free software;  you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 2 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY;  without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See
 *   the GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program;  if not, write to the Free Software
 *   Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 */

struct child_info {
	int	index;			/* our index into the array */
	int	status;			/* return status of this thread */
	int	child_count;		/* Count of children created */
	int	talk_count;		/* Count of siblings that */
					/* have talked to us */
	pthread_t	*threads;	/* dynamic array of thread */
					/* ids of children */
	pthread_mutex_t	talk_mutex;	/* mutex for the talk_count */
	pthread_mutex_t	child_mutex;	/* mutex for the child_count */
	pthread_cond_t	talk_condvar;	/* condition variable for talk_count */
	pthread_cond_t	child_condvar;	/* condition variable for child_count */
	struct child_info	**child_ptrs; 	/* dynamic array of ptrs */
						/* to children */
} ;

typedef struct child_info c_info;

/*
 * Copyright (c) 2015 Linux Test Project
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it would be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write the Free Software Foundation,
 * Inc.,  51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 */

#ifndef SEMUN_H__
#define SEMUN_H__

#if defined(__GNU_LIBRARY__) && !defined(_SEM_SEMUN_UNDEFINED)
/* union semun is defined by including <sys/sem.h> */
#else
/* according to X/OPEN we have to define it ourselves */
union semun {
	int val;                /* value for SETVAL */
	struct semid_ds *buf;   /* buffer for IPC_STAT, IPC_SET */
	unsigned short *array;  /* array for GETALL, SETALL */
	/* Linux specific part: */
	struct seminfo *__buf;  /* buffer for IPC_INFO */
};
#endif

#endif /* SEMUN_H__ */

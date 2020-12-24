// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2015 Linux Test Project
 */

#include <sys/sem.h>

#ifndef LAPI_SEM_H
#define LAPI_SEM_H

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

#ifndef SEM_STAT_ANY
# define SEM_STAT_ANY 20
#endif

#endif /* LAPI_SEM_H */

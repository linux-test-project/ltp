/* SPDX-License-Identifier: GPL-2.0-or-later */
/*
 * Copyright (c) 2019 Linus Walleij <linus.walleij@linaro.org>
 */

#ifndef LTP_IOPRIO_H
#define LTP_IOPRIO_H

enum {
	IOPRIO_CLASS_NONE = 0,
	IOPRIO_CLASS_RT,
	IOPRIO_CLASS_BE,
	IOPRIO_CLASS_IDLE,
};

enum {
	IOPRIO_WHO_PROCESS = 1,
	IOPRIO_WHO_PGRP,
	IOPRIO_WHO_USER,
};

/* The I/O scheduler classes have 8 priorities 0..7 except for the IDLE class */
#define IOPRIO_PRIO_NUM		8

#define IOPRIO_CLASS_SHIFT	(13)
#define IOPRIO_PRIO_MASK	((1UL << IOPRIO_CLASS_SHIFT) - 1)

#define IOPRIO_PRIO_CLASS(data)	((data) >> IOPRIO_CLASS_SHIFT)
#define IOPRIO_PRIO_LEVEL(data)	((data) & IOPRIO_PRIO_MASK)
#define IOPRIO_PRIO_VALUE(class, data)	(((class) << IOPRIO_CLASS_SHIFT) | data)

static const char * const to_class_str[] = {
	[IOPRIO_CLASS_NONE] = "NONE",
	[IOPRIO_CLASS_RT]   = "REALTIME",
	[IOPRIO_CLASS_BE]   = "BEST-EFFORT",
	[IOPRIO_CLASS_IDLE] = "IDLE"
};

static inline int sys_ioprio_get(int which, int who)
{
	return tst_syscall(__NR_ioprio_get, which, who);
}

static inline int sys_ioprio_set(int which, int who, int ioprio)
{
	return tst_syscall(__NR_ioprio_set, which, who, ioprio);
}

/* Priority range from 0 (highest) to 7 (lowest) */
static inline int prio_in_range(int prio)
{
	if ((prio < 0) || (prio > 7))
		return 0;
	return 1;
}

/* Priority range from 0 to 3 using the enum */
static inline int class_in_range(int class)
{
	if ((class < IOPRIO_CLASS_NONE) || (class > IOPRIO_CLASS_IDLE))
		return 0;
	return 1;
}

static inline void ioprio_check_setting(int class, int prio, int report)
{
	int res;
	int newclass, newprio;

	res = sys_ioprio_get(IOPRIO_WHO_PROCESS, 0);
	if (res == -1) {
		tst_res(TFAIL | TTERRNO,
			 "reading back prio failed");
		return;
	}

	newclass = IOPRIO_PRIO_CLASS(res);
	newprio = IOPRIO_PRIO_LEVEL(res);
	if (newclass != class)
		tst_res(TFAIL,
			"wrong class after setting, expected %s got %s",
			to_class_str[class],
			to_class_str[newclass]);
	else if (newprio != prio)
		tst_res(TFAIL,
			"wrong prio after setting, expected %d got %d",
			prio, newprio);
	else if (report)
		tst_res(TPASS, "ioprio_set new class %s, new prio %d",
			to_class_str[newclass],
			newprio);
}

#endif

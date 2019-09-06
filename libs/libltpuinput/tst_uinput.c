// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2015 Cedric Hnyda <chnyda@suse.com>
 * Copyright (c) 2019 Cyril Hrubis <chrubis@suse.cz>
 */

#include <linux/input.h>
#include <linux/uinput.h>
#include <stdio.h>
#include <string.h>

#define TST_NO_DEFAULT_MAIN
#include "tst_test.h"

#include "tst_uinput.h"

#define VIRTUAL_DEVICE "virtual-device-ltp"

static const char *uinput_paths[] = {
	"/dev/input/uinput",
	"/dev/uinput",
};

int open_uinput(void)
{
	unsigned int i;
	int fd;

	for (i = 0; i < ARRAY_SIZE(uinput_paths); i++) {
		fd = open(uinput_paths[i], O_WRONLY | O_NONBLOCK);

		if (fd > 0) {
			tst_res(TINFO, "Found uinput dev at %s", uinput_paths[i]);
			return fd;
		}

		if (fd < 0 && errno != ENOENT) {
			tst_brk(TBROK | TERRNO, "open(%s)", uinput_paths[i]);
		}
	}

	return -1;
}


#define SYSFS_PREFIX "Sysfs="
#define HANDLERS_PREFIX "Handlers="

static char *parse_field(char *line, char field)
{
	char *value;

	switch (field) {
	case 'H':
		value = strstr(line, HANDLERS_PREFIX) + sizeof(HANDLERS_PREFIX) - 1;
		break;
	case 'S':
		value = strstr(line, SYSFS_PREFIX) + sizeof(SYSFS_PREFIX) - 1;
		break;
	default:
		return NULL;
	}

	value[strlen(value) - 1] = 0;

	return strdup(value);
}

char *get_input_field_value(char field)
{
	FILE *file;
	char line[1024];
	int flag = 0;

	file = fopen("/proc/bus/input/devices", "r");
	if (!file)
		return NULL;

	while (fgets(line, sizeof(line), file)) {
		if (strstr(line, "N: Name=\""VIRTUAL_DEVICE"\""))
			flag = 1;

		if (flag) {
			if (line[0] == field)
				return parse_field(line, field);

			if (line[0] == '\n')
				flag = 0;
		}
	}

	fclose(file);
	return NULL;
}

static int check_device(void)
{
	FILE *file;
	char line[256];

	file = fopen("/proc/bus/input/devices", "r");
	if (!file)
		return 0;

	while (fgets(line, sizeof(line), file)) {
		if (strstr(line, "Name=\""VIRTUAL_DEVICE"\""))
			return 1;
	}

	fclose(file);

	return 0;
}

void setup_mouse_events(int fd)
{
	SAFE_IOCTL(fd, UI_SET_EVBIT, EV_KEY);
	SAFE_IOCTL(fd, UI_SET_KEYBIT, BTN_LEFT);
	SAFE_IOCTL(fd, UI_SET_EVBIT, EV_REL);
	SAFE_IOCTL(fd, UI_SET_RELBIT, REL_X);
	SAFE_IOCTL(fd, UI_SET_RELBIT, REL_Y);
}

void destroy_input_device(int fd)
{
	SAFE_IOCTL(fd, UI_DEV_DESTROY, NULL);
	SAFE_CLOSE(fd);
}

void create_input_device(int fd)
{
	int nb;
	struct uinput_user_dev uidev = {
		.name = VIRTUAL_DEVICE,
		.id = {
			.bustype = BUS_USB,
			.vendor = 0x1,
			.product = 0x1,
			.version = 1,
		}
	};

	SAFE_WRITE(1, fd, &uidev, sizeof(uidev));
	SAFE_IOCTL(fd, UI_DEV_CREATE, NULL);

	for (nb = 100; nb > 0; nb--) {
		if (check_device())
			return;
		usleep(10000);
	}

	destroy_input_device(fd);
	tst_brk(TBROK, "Failed to create device");
}

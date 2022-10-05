/*
 * Copyright (c) 2015 Cedric Hnyda <chnyda@suse.com>
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

#include <linux/input.h>
#include <linux/uinput.h>
#include <fnmatch.h>
#include <errno.h>
#include <poll.h>

#include "test.h"
#include "safe_macros.h"
#include "input_helper.h"

#define VIRTUAL_DEVICE "virtual-device-ltp"

#define VIRTUAL_DEVICE_REGEX "*virtual-device-ltp*"

static int uinput_loaded;
static int check_device(void);

static int try_open_device(void)
{
	char path[256];
	char name[256];
	int ret, fd = -1;
	unsigned int i;

	for (i = 0; i < 100; i++) {
		snprintf(path, sizeof(path), "/dev/input/event%i", i);

		fd = open(path, O_RDONLY);

		if (fd < 0 && errno == ENOENT)
			continue;

		if (fd < 0) {
			tst_resm(TINFO | TERRNO, "failed to open %s", path);
			break;
		}

		ret = ioctl(fd, EVIOCGNAME(sizeof(name)), name);
		if (ret < 0) {
			tst_resm(TINFO | TERRNO,
				"ioctl(%s, EVIOCGNAME(256), ...) failed",
				path);
			break;
		}

		if (strcmp(name, VIRTUAL_DEVICE) == 0)
			return fd;
		close(fd);
	}

	return -1;
}

int open_device(void)
{
	int fd;
	int retries = 10;

	while (retries--) {
		fd = try_open_device();
		if (fd > 0)
			return fd;
		tst_resm(TINFO, "Device not found, retrying...");
		usleep(10000);
	}

	tst_brkm(TBROK, NULL, "Unable to find the input device");
}

static int try_load_uinput(void)
{
	const char *argv[] = {"modprobe", "uinput", NULL};
	int ret;

	tst_resm(TINFO, "Trying to load uinput kernel module");

	ret = tst_cmd(NULL, argv, NULL, NULL, TST_CMD_PASS_RETVAL);
	if (ret) {
		tst_resm(TINFO, "Failed to load the uinput module");
		return 0;
	}

	return 1;
}

static void unload_uinput(void)
{
	const char *argv[] = {"modprobe", "-r", "uinput", NULL};
	int ret;

	tst_resm(TINFO, "Unloading uinput kernel module");

	ret = tst_cmd(NULL, argv, NULL, NULL, TST_CMD_PASS_RETVAL);
	if (ret)
		tst_resm(TWARN, "Failed to unload uinput module");
}

static const char *uinput_paths[] = {
	"/dev/input/uinput",
	"/dev/uinput",
};

static int try_open_uinput(void)
{
	unsigned int i;
	int fd;

	for (i = 0; i < ARRAY_SIZE(uinput_paths); i++) {
		fd = open(uinput_paths[i], O_WRONLY | O_NONBLOCK);

		if (fd > 0) {
			tst_resm(TINFO, "Found uinput dev at %s",
			         uinput_paths[i]);
			return fd;
		}

		if (fd < 0 && errno != ENOENT) {
			tst_brkm(TBROK | TERRNO, NULL,
			         "open(%s)", uinput_paths[i]);
		}
	}

	return -1;
}

int open_uinput(void)
{
	int fd;
	int retries = 10;

	fd = try_open_uinput();
	if (fd > 0)
		return fd;

	if (try_load_uinput()) {
		while (retries--) {
			fd = try_open_uinput();
			if (fd > 0) {
				uinput_loaded = 1;
				return fd;
			}
			tst_resm(TINFO, "Uinput dev not found, retrying...");
			usleep(10000);
		}

		unload_uinput();
	}

	tst_brkm(TCONF, NULL, "Unable to find and open uinput");
}

void send_event(int fd, int event, int code, int value)
{
	struct input_event ev = {
		.type = event,
		.code = code,
		.value = value,
	};

	SAFE_WRITE(NULL, SAFE_WRITE_ALL, fd, &ev, sizeof(ev));
}

void send_rel_move(int fd, int x, int y)
{
	send_event(fd, EV_REL, REL_X, x);
	send_event(fd, EV_REL, REL_Y, y);
	send_event(fd, EV_SYN, 0, 0);
}

void create_device(int fd)
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

	SAFE_WRITE(NULL, SAFE_WRITE_ALL, fd, &uidev, sizeof(uidev));
	SAFE_IOCTL(NULL, fd, UI_DEV_CREATE, NULL);

	for (nb = 100; nb > 0; nb--) {
		if (check_device())
			return;
		usleep(10000);
	}

	destroy_device(fd);
	tst_brkm(TBROK, NULL, "Failed to create device");
}

void setup_mouse_events(int fd)
{
	SAFE_IOCTL(NULL, fd, UI_SET_EVBIT, EV_KEY);
	SAFE_IOCTL(NULL, fd, UI_SET_KEYBIT, BTN_LEFT);
	SAFE_IOCTL(NULL, fd, UI_SET_EVBIT, EV_REL);
	SAFE_IOCTL(NULL, fd, UI_SET_RELBIT, REL_X);
	SAFE_IOCTL(NULL, fd, UI_SET_RELBIT, REL_Y);
}

void destroy_device(int fd)
{
	SAFE_IOCTL(NULL, fd, UI_DEV_DESTROY, NULL);
	SAFE_CLOSE(NULL, fd);

	if (uinput_loaded)
		unload_uinput();
}

int check_event_code(struct input_event *iev, int event, int code)
{
	return iev->type == event && iev->code == code;
}

int check_sync_event(struct input_event *iev)
{
	return check_event_code(iev, EV_SYN, SYN_REPORT);
}

/*
 * the value of stray_sync_event:
 * 0: EV_SYN/SYN_REPORT events should not be received in /dev/input/eventX
 * 1: EV_SYN/SYN_REPORT events may be received in /dev/input/eventX
 * On an old kernel(before v3.7.0), EV_SYN/SYN_REPORT events are always
 * received even though we send empty moves.
 */
int no_events_queued(int fd, int stray_sync_event)
{
	struct pollfd fds = {.fd = fd, .events = POLLIN};
	int ret, res, sync_event_ignored;
	struct input_event ev;

	if (tst_kvercmp(3, 7, 0) < 0 && stray_sync_event)
		sync_event_ignored = 1;

	ret = poll(&fds, 1, 30);

	if (ret > 0) {
		res = read(fd, &ev, sizeof(ev));

		if (res == sizeof(ev)) {
			if (sync_event_ignored && check_sync_event(&ev)) {
				ret = 0;
				tst_resm(TINFO,
					 "Ignoring stray sync event (known problem)");
			} else {
				tst_resm(TINFO,
					 "Unexpected ev type=%i code=%i value=%i",
					 ev.type, ev.code, ev.value);
			}
		}
	}

	return ret == 0;
}

static int check_device(void)
{
	FILE *file;
	char line[256];

	file = fopen("/proc/bus/input/devices", "r");
	if (!file)
		return 0;

	while (fgets(line, 256, file)) {
		if (fnmatch(VIRTUAL_DEVICE_REGEX, line, 0) == 0)
			return 1;
	}

	fclose(file);

	return 0;
}

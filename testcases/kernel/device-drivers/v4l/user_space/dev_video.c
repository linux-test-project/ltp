/*
 * v4l-test: Test environment for Video For Linux Two API
 *
 *  3 Apr 2009  0.2  Added camera enabling through
 *                   /sys/devices/platform/eeepc/camera
 * 18 Dec 2008  0.1  First release
 *
 * Written by Márton Németh <nm127@freemail.hu>
 * Released under GPL
 */

#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>

#include "dev_video.h"

#define DEV_VIDEO_STATE_NORMAL		0
#define DEV_VIDEO_STATE_ASUS		1
#define DEV_VIDEO_STATE_EEEPC		2

static int f;
static int dev_video_state;

static int try_ASUS_camera(char *path, char *value)
{
	int ASUS_camera;
	size_t s;
	int ret;

	ASUS_camera = open(path, O_WRONLY);
	if (ASUS_camera < 0) {
		fprintf(stderr, "Cannot open %s: %s\n", path, strerror(errno));
		return ASUS_camera;
	}
	s = write(ASUS_camera, value, 1);
	ret = close(ASUS_camera);
	if (s != 1) {
		return -1;
	}
	if (ret < 0) {
		perror("Cannot close ASUS camera");
		return ret;
	}

	sleep(1);

	f = open("/dev/video0", O_RDWR);
	if (f < 0) {
		perror("Cannot open /dev/video0");
		return f;
	}
	dev_video_state = DEV_VIDEO_STATE_ASUS;

	return f;
}

int open_video()
{
	int error = 0;

	fflush(stdout);

	f = open("/dev/video0", O_RDWR);
	if (f < 0) {
		perror("Cannot open /dev/video0");
		fprintf(stderr, "Retrying with ASUS camera...\n");
		f = try_ASUS_camera("/proc/acpi/asus/camera", "1");
		if (f < 0) {
			f = try_ASUS_camera
			    ("/sys/devices/platform/eeepc/camera", "1");
			if (f < 0) {
				error = 1;
			} else {
				dev_video_state = DEV_VIDEO_STATE_EEEPC;
			}
		} else {
			dev_video_state = DEV_VIDEO_STATE_ASUS;
		}
	} else {
		dev_video_state = DEV_VIDEO_STATE_NORMAL;
	}
	return error;
}

int close_video()
{
	int ret;

	fflush(stdout);

	ret = close(f);
	f = 0;
	if (ret < 0) {
		perror("Cannot close");
		return 1;
	}
	switch (dev_video_state) {
	case DEV_VIDEO_STATE_NORMAL:
		break;
	case DEV_VIDEO_STATE_ASUS:
		try_ASUS_camera("/proc/acpi/asus/camera", "0");
		break;
	case DEV_VIDEO_STATE_EEEPC:
		try_ASUS_camera("/sys/devices/platform/eeepc/camera", "0");
		break;
	}

	return 0;
}

int get_video_fd()
{
	return f;
}

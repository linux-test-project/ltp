/*
 * v4l-test: Test environment for Video For Linux Two API
 *
 * 23 Dec 2008  0.1  First release
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

#include <linux/videodev2.h>
#include <linux/errno.h>

#include <CUnit/CUnit.h>

#include "v4l2_test.h"
#include "dev_video.h"
#include "video_limits.h"

#include "test_VIDIOC_LOG_STATUS.h"

void test_VIDIOC_LOG_STATUS() {
	int ret;

	ret = ioctl(get_video_fd(), VIDIOC_LOG_STATUS);
	dprintf("ret=%i, errno=%i\n", ret, errno);

	/* this is an optional ioctl, so two possible return values */
	/* are possible */
	if (ret == 0) {
		CU_ASSERT_EQUAL(ret, 0);
		/* TODO: check if something is shown in dmesg */

	} else {
		CU_ASSERT_EQUAL(ret, -1);
		CU_ASSERT_EQUAL(errno, EINVAL);
	}
}

/*
 * v4l-test: Test environment for Video For Linux Two API
 *
 * 30 Jan 2009  0.1  First release
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
#include "v4l2_validator.h"

#include "test_VIDIOC_QUERYSTD.h"

void test_VIDIOC_QUERYSTD() {
	int ret;
	v4l2_std_id id;

	memset(&id, 0, sizeof(id));

	ret = ioctl(get_video_fd(), VIDIOC_QUERYSTD, &id);

	dprintf("VIDIOC_QUERYSTD, ret=%i\n", ret);
	dprintf("\tid=0x%llx\n", id);

	if (ret == 0) {
		CU_ASSERT(id != 0);
		CU_ASSERT(valid_v4l2_std_id(id));

	} else if (ret == -1) {
		/* if this ioctl is not supported, then errno shall be EINVAL */
		dprintf("\tret=%d (expected %d)\n", ret, -1);
		dprintf("\terrno=%d (expected %d)\n", errno, EINVAL);
		CU_ASSERT_EQUAL(errno, EINVAL);
	}

}

void test_VIDIOC_QUERYSTD_NULL() {
	int ret1;
	int errno1;
	int ret2;
	v4l2_std_id id;

	memset(&id, 0, sizeof(id));
	ret1 = ioctl(get_video_fd(), VIDIOC_QUERYSTD, NULL);
	errno1 = errno;
	dprintf("\tChecking VIDIOC_QUERYSTD whether is supported: "
		"%s (ret1=%d, errno1=%d)\n", (ret1 == -1 && errno1 == EINVAL) ? "no" : "yes", ret1, errno1);

	ret2 = ioctl(get_video_fd(), VIDIOC_QUERYSTD, NULL);

	/* check if VIDIOC_QUERYSTD is supported at all or not */
	if (ret1 == -1 && errno1 == EINVAL) {
		/* VIDIOC_QUERYSTD not supported at all, the parameter should not be evaluated */
		dprintf("\t%s:%u: ret2=%d (expected %d)\n", __FILE__, __LINE__, ret2, -1);
		dprintf("\t%s:%u: errno=%d (expected %d)\n", __FILE__, __LINE__, errno, EINVAL);
		CU_ASSERT_EQUAL(ret2, -1);
		CU_ASSERT_EQUAL(errno, EINVAL);

	} else {
		/* VIDIOC_QUERYSTD is supported, the parameter should be checked */
		dprintf("\t%s:%u: ret2=%d (expected %d)\n", __FILE__, __LINE__, ret2, -1);
		dprintf("\t%s:%u: errno=%d (expected %d)\n", __FILE__, __LINE__, errno, EFAULT);
		CU_ASSERT_EQUAL(ret2, -1);
		CU_ASSERT_EQUAL(errno, EFAULT);
	}
}

/* TODO: check for different input settings */

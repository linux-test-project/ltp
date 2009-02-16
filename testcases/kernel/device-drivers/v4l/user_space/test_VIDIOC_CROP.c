/*
 * v4l-test: Test environment for Video For Linux Two API
 *
 *  7 Feb 2009  0.1  First release
 *
 * Written by Márton Németh <nm127@freemail.hu>
 * Released under GPL
 */

#include <sys/ioctl.h>
#include <errno.h>
#include <string.h>

#include <linux/videodev2.h>
#include <linux/errno.h>

#include <CUnit/CUnit.h>

#include "v4l2_test.h"
#include "dev_video.h"
#include "video_limits.h"

#include "test_VIDIOC_CROP.h"


void test_VIDIOC_G_CROP() {
	int ret1, errno1;
	struct v4l2_crop crop;

	memset(&crop, 0xff, sizeof(crop));
	ret1 = ioctl(get_video_fd(), VIDIOC_G_CROP, &crop);
	errno1 = errno;

	dprintf("VIDIOC_G_CROP ret1=%i, errno1=%i\n", ret1, errno1);

	if (ret1 == 0) {
		CU_ASSERT_EQUAL(ret1, 0);

	} else {
		CU_ASSERT_EQUAL(ret1, -1);
		CU_ASSERT_EQUAL(errno1, EINVAL);

	}

}

void test_VIDIOC_G_CROP_NULL() {
	int ret1, errno1;
	int ret2, errno2;
	struct v4l2_crop crop;

	memset(&crop, 0xff, sizeof(crop));
	ret1 = ioctl(get_video_fd(), VIDIOC_G_CROP, &crop);
	errno1 = errno;

	dprintf("VIDIOC_G_CROP ret1=%i, errno1=%i\n", ret1, errno1);

	memset(&crop, 0xff, sizeof(crop));
	ret2 = ioctl(get_video_fd(), VIDIOC_G_CROP, NULL);
	errno2 = errno;

	dprintf("VIDIOC_G_CROP ret2=%i, errno2=%i\n", ret2, errno2);

	if (ret1 == 0) {
		CU_ASSERT_EQUAL(ret1, 0);
		CU_ASSERT_EQUAL(ret2, -1);
		CU_ASSERT_EQUAL(errno2, EFAULT);

	} else {
		CU_ASSERT_EQUAL(ret1, -1);
		CU_ASSERT_EQUAL(errno1, EINVAL);
		CU_ASSERT_EQUAL(ret2, -1);
		CU_ASSERT_EQUAL(errno2, EINVAL);

	}

}

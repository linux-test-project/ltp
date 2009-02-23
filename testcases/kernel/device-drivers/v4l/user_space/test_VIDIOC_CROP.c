/*
 * v4l-test: Test environment for Video For Linux Two API
 *
 * 13 Feb 2009  0.2  Test cases added for VIDIOC_G_CROP
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


void do_get_crop(enum v4l2_buf_type type) {
	int ret1, errno1;
	struct v4l2_crop crop;

	memset(&crop, 0xff, sizeof(crop));
	crop.type = type;
	ret1 = ioctl(get_video_fd(), VIDIOC_G_CROP, &crop);
	errno1 = errno;

	dprintf("\tVIDIOC_G_CROP: type=%i, ret1=%i, errno1=%i\n",
		type, ret1, errno1);

	if (ret1 == 0) {
		CU_ASSERT_EQUAL(ret1, 0);

	} else {
		CU_ASSERT_EQUAL(ret1, -1);
		CU_ASSERT_EQUAL(errno1, EINVAL);
	}

}

void test_VIDIOC_G_CROP() {
	do_get_crop(V4L2_BUF_TYPE_VIDEO_CAPTURE);
	do_get_crop(V4L2_BUF_TYPE_VIDEO_OUTPUT);
	do_get_crop(V4L2_BUF_TYPE_VIDEO_OVERLAY);
}


void do_get_crop_invalid(enum v4l2_buf_type type) {
	int ret1, errno1;
	struct v4l2_crop crop;

	memset(&crop, 0xff, sizeof(crop));
	crop.type = type;
	ret1 = ioctl(get_video_fd(), VIDIOC_G_CROP, &crop);
	errno1 = errno;

	dprintf("\tVIDIOC_G_CROP: type=%i, ret1=%i, errno1=%i\n",
		type, ret1, errno1);

	CU_ASSERT_EQUAL(ret1, -1);
	CU_ASSERT_EQUAL(errno1, EINVAL);
}

void test_VIDIOC_G_CROP_invalid() {
	do_get_crop_invalid(0);
	do_get_crop_invalid(V4L2_BUF_TYPE_VBI_CAPTURE);
	do_get_crop_invalid(V4L2_BUF_TYPE_VBI_OUTPUT);
	do_get_crop_invalid(V4L2_BUF_TYPE_SLICED_VBI_CAPTURE);
	do_get_crop_invalid(V4L2_BUF_TYPE_SLICED_VBI_OUTPUT);
	do_get_crop_invalid(V4L2_BUF_TYPE_VIDEO_OUTPUT_OVERLAY);
	do_get_crop_invalid(V4L2_BUF_TYPE_PRIVATE);
	do_get_crop_invalid(S32_MAX);
	do_get_crop_invalid( ((__u32)S32_MAX)+1 );
	do_get_crop_invalid(U32_MAX);
}

void test_VIDIOC_G_CROP_NULL() {
	int ret1, errno1;
	int ret2, errno2;
	int ret3, errno3;
	int ret4, errno4;
	struct v4l2_crop crop;

	memset(&crop, 0, sizeof(crop));
	crop.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;

	ret1 = ioctl(get_video_fd(), VIDIOC_G_CROP, &crop);
	errno1 = errno;

	dprintf("VIDIOC_G_CROP ret1=%i, errno1=%i\n", ret1, errno1);

	memset(&crop, 0, sizeof(crop));
	crop.type = V4L2_BUF_TYPE_VIDEO_OUTPUT;

	ret2 = ioctl(get_video_fd(), VIDIOC_G_CROP, &crop);
	errno2 = errno;

	dprintf("VIDIOC_G_CROP ret2=%i, errno2=%i\n", ret2, errno2);

	memset(&crop, 0, sizeof(crop));
	crop.type = V4L2_BUF_TYPE_VIDEO_OVERLAY;

	ret3 = ioctl(get_video_fd(), VIDIOC_G_CROP, &crop);
	errno3 = errno;

	dprintf("VIDIOC_G_CROP ret3=%i, errno3=%i\n", ret3, errno3);


	ret4 = ioctl(get_video_fd(), VIDIOC_G_CROP, NULL);
	errno4 = errno;

	dprintf("VIDIOC_G_CROP ret4=%i, errno4=%i\n", ret4, errno4);

	if (ret1 == 0 || ret2 == 0 || ret3 == 0) {
		CU_ASSERT_EQUAL(ret4, -1);
		CU_ASSERT_EQUAL(errno4, EFAULT);

	} else {
		CU_ASSERT_EQUAL(ret1, -1);
		CU_ASSERT_EQUAL(errno1, EINVAL);
		CU_ASSERT_EQUAL(ret2, -1);
		CU_ASSERT_EQUAL(errno2, EINVAL);
		CU_ASSERT_EQUAL(ret3, -1);
		CU_ASSERT_EQUAL(errno3, EINVAL);
		CU_ASSERT_EQUAL(ret4, -1);
		CU_ASSERT_EQUAL(errno4, EINVAL);

	}

}

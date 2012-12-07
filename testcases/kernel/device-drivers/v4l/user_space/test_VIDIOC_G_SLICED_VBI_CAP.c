/*
 * v4l-test: Test environment for Video For Linux Two API
 *
 * 22 Mar 2009  0.1  First release
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

#include "test_VIDIOC_G_SLICED_VBI_CAP.h"

void do_get_sliced_vbi_cap(enum v4l2_buf_type type)
{
	int ret_cap, errno_cap;
	struct v4l2_sliced_vbi_cap sliced_vbi_cap;

	memset(&sliced_vbi_cap, 0xff, sizeof(sliced_vbi_cap));
	sliced_vbi_cap.type = type;
	ret_cap =
	    ioctl(get_video_fd(), VIDIOC_G_SLICED_VBI_CAP, &sliced_vbi_cap);
	errno_cap = errno;

	dprintf
	    ("\tVIDIOC_G_SLICED_VBI_CAP: type=%i, ret_cap=%i, errno_cap=%i\n",
	     type, ret_cap, errno_cap);

	if (ret_cap == 0) {
		CU_ASSERT_EQUAL(ret_cap, 0);

	} else {
		CU_ASSERT_EQUAL(ret_cap, -1);
		CU_ASSERT_EQUAL(errno_cap, EINVAL);
	}

}

void test_VIDIOC_G_SLICED_VBI_CAP()
{
	do_get_sliced_vbi_cap(V4L2_BUF_TYPE_SLICED_VBI_CAPTURE);
	do_get_sliced_vbi_cap(V4L2_BUF_TYPE_SLICED_VBI_OUTPUT);
}

void do_get_sliced_vbi_cap_invalid(enum v4l2_buf_type type)
{
	int ret_cap, errno_cap;
	struct v4l2_sliced_vbi_cap sliced_vbi_cap;

	memset(&sliced_vbi_cap, 0xff, sizeof(sliced_vbi_cap));
	sliced_vbi_cap.type = type;
	ret_cap =
	    ioctl(get_video_fd(), VIDIOC_G_SLICED_VBI_CAP, &sliced_vbi_cap);
	errno_cap = errno;

	dprintf
	    ("\tVIDIOC_G_SLICED_VBI_CAP: type=%i, ret_cap=%i, errno_cap=%i\n",
	     type, ret_cap, errno_cap);

	CU_ASSERT_EQUAL(ret_cap, -1);
	CU_ASSERT_EQUAL(errno_cap, EINVAL);
}

void test_VIDIOC_G_SLICED_VBI_CAP_invalid()
{
	do_get_sliced_vbi_cap_invalid(0);
	do_get_sliced_vbi_cap_invalid(V4L2_BUF_TYPE_VIDEO_CAPTURE);
	do_get_sliced_vbi_cap_invalid(V4L2_BUF_TYPE_VIDEO_OUTPUT);
	do_get_sliced_vbi_cap_invalid(V4L2_BUF_TYPE_VIDEO_OVERLAY);
	do_get_sliced_vbi_cap_invalid(V4L2_BUF_TYPE_VBI_CAPTURE);
	do_get_sliced_vbi_cap_invalid(V4L2_BUF_TYPE_VBI_OUTPUT);
	do_get_sliced_vbi_cap_invalid(V4L2_BUF_TYPE_VIDEO_OUTPUT_OVERLAY);
	do_get_sliced_vbi_cap_invalid(V4L2_BUF_TYPE_PRIVATE);
	do_get_sliced_vbi_cap_invalid(S32_MAX);
	do_get_sliced_vbi_cap_invalid(((__u32) S32_MAX) + 1);
	do_get_sliced_vbi_cap_invalid(U32_MAX);
}

void test_VIDIOC_G_SLICED_VBI_CAP_NULL()
{
	int ret_get, errno_get;
	int ret_set_capture, errno_set_capture;
	int ret_set_output, errno_set_output;
	int ret_null, errno_null;
	struct v4l2_sliced_vbi_cap sliced_vbi_cap;

	memset(&sliced_vbi_cap, 0, sizeof(sliced_vbi_cap));
	sliced_vbi_cap.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;

	ret_get =
	    ioctl(get_video_fd(), VIDIOC_G_SLICED_VBI_CAP, &sliced_vbi_cap);
	errno_get = errno;

	dprintf("\tVIDIOC_G_SLICED_VBI_CAP ret_get=%i, errno_get=%i\n", ret_get,
		errno_get);

	memset(&sliced_vbi_cap, 0, sizeof(sliced_vbi_cap));
	sliced_vbi_cap.type = V4L2_BUF_TYPE_SLICED_VBI_CAPTURE;

	ret_set_capture =
	    ioctl(get_video_fd(), VIDIOC_G_SLICED_VBI_CAP, &sliced_vbi_cap);
	errno_set_capture = errno;

	dprintf
	    ("\tVIDIOC_G_SLICED_VBI_CAP ret_set_capture=%i, errno_set_capture=%i\n",
	     ret_set_capture, errno_set_capture);

	memset(&sliced_vbi_cap, 0, sizeof(sliced_vbi_cap));
	sliced_vbi_cap.type = V4L2_BUF_TYPE_SLICED_VBI_OUTPUT;

	ret_set_output =
	    ioctl(get_video_fd(), VIDIOC_G_SLICED_VBI_CAP, &sliced_vbi_cap);
	errno_set_output = errno;

	dprintf
	    ("\tVIDIOC_G_SLICED_VBI_CAP ret_set_output=%i, errno_set_output=%i\n",
	     ret_set_output, errno_set_output);

	ret_null = ioctl(get_video_fd(), VIDIOC_G_SLICED_VBI_CAP, NULL);
	errno_null = errno;

	dprintf("\tVIDIOC_G_SLICED_VBI_CAP ret_null=%i, errno_null=%i\n",
		ret_null, errno_null);

	if (ret_get == 0 || ret_set_capture == 0 || ret_set_output == 0) {
		CU_ASSERT_EQUAL(ret_null, -1);
		CU_ASSERT_EQUAL(errno_null, EFAULT);

	} else {
		CU_ASSERT_EQUAL(ret_get, -1);
		CU_ASSERT_EQUAL(errno_get, EINVAL);
		CU_ASSERT_EQUAL(ret_set_capture, -1);
		CU_ASSERT_EQUAL(errno_set_capture, EINVAL);
		CU_ASSERT_EQUAL(ret_set_capture, -1);
		CU_ASSERT_EQUAL(errno_set_capture, EINVAL);
		CU_ASSERT_EQUAL(ret_null, -1);
		CU_ASSERT_EQUAL(errno_null, EINVAL);

	}

}

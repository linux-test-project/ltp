/*
 * v4l-test: Test environment for Video For Linux Two API
 *
 * 22 Dec 2008  0.2  Test case with NULL parameter added
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

#include <linux/videodev2.h>
#include <linux/errno.h>

#include <CUnit/CUnit.h>
#include <CUnit/Basic.h>

#include "v4l2_test.h"
#include "dev_video.h"
#include "video_limits.h"

#include "test_VIDIOC_ENUM_FMT.h"

#ifndef V4L2_BUF_TYPE_VIDEO_OUTPUT_OVERLAY
#define V4L2_BUF_TYPE_VIDEO_OUTPUT_OVERLAY 8
#endif


/* VIDIOC_ENUM_FMT */

static void do_enumerate_formats(enum v4l2_buf_type type) {
	int ret;
	struct v4l2_fmtdesc format;
	struct v4l2_fmtdesc format2;
	__u32 i;

	i = 0;
	do {
		memset(&format, 0xff, sizeof(format));
		format.index = i;
		format.type = type;

		ret = ioctl(get_video_fd(), VIDIOC_ENUM_FMT, &format);
		dprintf("VIDIOC_ENUM_FMT, ret=%i\n", ret);
		if (ret == 0) {
			CU_ASSERT_EQUAL(ret, 0);
			CU_ASSERT_EQUAL(format.index, i);
			//CU_ASSERT_EQUAL(format.type, ?);
			//CU_ASSERT_EQUAL(format.flags, ?);

			//CU_ASSERT_EQUAL(format.description, ?);
			CU_ASSERT(0 < strlen( (char*)format.description ));

			//CU_ASSERT_EQUAL(format.pixelformat, ?);
			CU_ASSERT_EQUAL(format.reserved[0], 0);
			CU_ASSERT_EQUAL(format.reserved[1], 0);
			CU_ASSERT_EQUAL(format.reserved[2], 0);
			CU_ASSERT_EQUAL(format.reserved[3], 0);

			dprintf("\tformat = {.index=%u, .type=0x%X, .flags=0x%X, "
				".description=\"%s\", .pixelformat=0x%X, "
				".reserved[]={ 0x%X, 0x%X, 0x%X, 0x%X } }\n",
				format.index,
				format.type,
				format.flags,
				format.description,
				format.pixelformat,
				format.reserved[0],
				format.reserved[1],
				format.reserved[2],
				format.reserved[3]
			);

		} else {
			CU_ASSERT_EQUAL(ret, -1);
			CU_ASSERT_EQUAL(errno, EINVAL);

			memset(&format2, 0xff, sizeof(format2));
			format2.index = i;
			format2.type = type;
			CU_ASSERT_EQUAL(memcmp(&format, &format2, sizeof(format)), 0);

			dprintf("\terrno=%i\n", errno);

		}
		i++;
	} while (ret == 0);

}

void test_VIDIOC_ENUM_FMT_1() {
	do_enumerate_formats(V4L2_BUF_TYPE_VIDEO_CAPTURE);
	do_enumerate_formats(V4L2_BUF_TYPE_VIDEO_CAPTURE);
	do_enumerate_formats(V4L2_BUF_TYPE_VIDEO_OUTPUT);
	do_enumerate_formats(V4L2_BUF_TYPE_VIDEO_OVERLAY);
	do_enumerate_formats(V4L2_BUF_TYPE_VBI_CAPTURE);
	do_enumerate_formats(V4L2_BUF_TYPE_VBI_OUTPUT);
	do_enumerate_formats(V4L2_BUF_TYPE_SLICED_VBI_CAPTURE);
	do_enumerate_formats(V4L2_BUF_TYPE_SLICED_VBI_OUTPUT);
	do_enumerate_formats(V4L2_BUF_TYPE_VIDEO_OUTPUT_OVERLAY);
	do_enumerate_formats(V4L2_BUF_TYPE_PRIVATE);
}

void test_VIDIOC_ENUM_FMT_2() {
	int ret;
	struct v4l2_fmtdesc format;
	struct v4l2_fmtdesc format2;

	/* test invalid index */
	memset(&format, 0xff, sizeof(format));
	format.index = U32_MAX;
	format.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	ret = ioctl(get_video_fd(), VIDIOC_ENUM_FMT, &format);

	CU_ASSERT_EQUAL(ret, -1);
	CU_ASSERT_EQUAL(errno, EINVAL);

	memset(&format2, 0xff, sizeof(format2));
	format2.index = U32_MAX;
	format2.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	CU_ASSERT_EQUAL(memcmp(&format, &format2, sizeof(format)), 0);
}

void test_VIDIOC_ENUM_FMT_3() {
	int ret;
	struct v4l2_fmtdesc format;
	struct v4l2_fmtdesc format2;
	int i;

	/* test invalid .type=0 */
	memset(&format, 0xff, sizeof(format));
	format.index = 0;
	format.type = 0;
	ret = ioctl(get_video_fd(), VIDIOC_ENUM_FMT, &format);

	CU_ASSERT_EQUAL(ret, -1);
	CU_ASSERT_EQUAL(errno, EINVAL);

	memset(&format2, 0xff, sizeof(format2));
	format2.index = 0;
	format2.type = 0;
	CU_ASSERT_EQUAL(memcmp(&format, &format2, sizeof(format)), 0);

	/* test invalid .type=0x8000 0000 */
	memset(&format, 0xff, sizeof(format));
	format.index = 0;
	format.type = 0x80000000;
	ret = ioctl(get_video_fd(), VIDIOC_ENUM_FMT, &format);

	CU_ASSERT_EQUAL(ret, -1);
	CU_ASSERT_EQUAL(errno, EINVAL);

	memset(&format2, 0xff, sizeof(format2));
	format2.index = 0;
	format2.type = 0x80000000;
	CU_ASSERT_EQUAL(memcmp(&format, &format2, sizeof(format)), 0);

	/* test invalid .type=-1 */
	memset(&format, 0xff, sizeof(format));
	format.index = 0;
	format.type = -1;
	ret = ioctl(get_video_fd(), VIDIOC_ENUM_FMT, &format);

	CU_ASSERT_EQUAL(ret, -1);
	CU_ASSERT_EQUAL(errno, EINVAL);

	memset(&format2, 0xff, sizeof(format2));
	format2.index = 0;
	format2.type = -1;
	CU_ASSERT_EQUAL(memcmp(&format, &format2, sizeof(format)), 0);

	/* test invalid .type= 8..0x7F */
	for (i = V4L2_BUF_TYPE_VIDEO_OUTPUT_OVERLAY+1; i < V4L2_BUF_TYPE_PRIVATE; i++) {
		memset(&format, 0xff, sizeof(format));
		format.index = 0;
		format.type = i;
		ret = ioctl(get_video_fd(), VIDIOC_ENUM_FMT, &format);

		CU_ASSERT_EQUAL(ret, -1);
		CU_ASSERT_EQUAL(errno, EINVAL);

		memset(&format2, 0xff, sizeof(format2));
		format2.index = 0;
		format2.type = i;
		CU_ASSERT_EQUAL(memcmp(&format, &format2, sizeof(format)), 0);
	}

	/* .type = 0x80..0x7FFF FFFF is the private range */
}

void test_VIDIOC_ENUM_FMT_NULL() {
	int ret;

	ret = ioctl(get_video_fd(), VIDIOC_ENUM_FMT, NULL);
	CU_ASSERT_EQUAL(ret, -1);
	CU_ASSERT_EQUAL(errno, EFAULT);

}

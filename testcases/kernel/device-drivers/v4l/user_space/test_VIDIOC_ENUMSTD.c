/*
 * v4l-test: Test environment for Video For Linux Two API
 *
 * 18 Jan 2009  0.4  Test case for MAX_EM28XX_TVNORMS removed, test cases for
 *                   S32_MAX & U32_MAX are enough
 *  1 Jan 2009  0.3  Added index=S32_MAX and S32_MAX+1
 * 22 Dec 2008  0.2  Test case with NULL parameter added
 * 18 Dec 2008  0.1  First release
 *
 * Written by Márton Németh <nm127@freemail.hu>
 * Released under GPL
 */

/* TODO: from V4L2 Spec:
 * "Drivers may enumerate a different set of standards after switching the video input or output."
 * 
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

#include "test_VIDIOC_ENUMSTD.h"

void test_VIDIOC_ENUMSTD() {
	int ret;
	struct v4l2_standard std;
	struct v4l2_standard std2;
	__u32 i;

	i = 0;
	do {
		memset(&std, 0xff, sizeof(std));
		std.index = i;
		ret = ioctl(get_video_fd(), VIDIOC_ENUMSTD, &std);

		dprintf("VIDIOC_ENUMSTD, ret=%i\n", ret);

		if (ret == 0) {
			CU_ASSERT_EQUAL(ret, 0);
			CU_ASSERT_EQUAL(std.index, i);
			//CU_ASSERT_EQUAL(std.id, ?);

			//CU_ASSERT_EQUAL(std.name, ?);
			CU_ASSERT(0 < strlen( (char*)std.name ));

			//CU_ASSERT_EQUAL(std.frameperiod.numerator, ?);
			//CU_ASSERT_EQUAL(std.frameperiod.denominator, ?);
			//CU_ASSERT_EQUAL(std.framelines, ?);
			CU_ASSERT_EQUAL(std.reserved[0], 0);
			CU_ASSERT_EQUAL(std.reserved[1], 0);
			CU_ASSERT_EQUAL(std.reserved[2], 0);
			CU_ASSERT_EQUAL(std.reserved[3], 0);

			dprintf("\tstd = {.index=%u, .id=%llX, .name=\"%s\", "
				".frameperiod={ .numerator=%u, .denominator=%u }, "
				".framelines=%u, "
				".reserved[]={ 0x%X, 0x%X, 0x%X, 0x%X } }\n",
				std.index,
				std.id,
				std.name,
				std.frameperiod.numerator,
				std.frameperiod.denominator,
				std.framelines,
				std.reserved[0],
				std.reserved[1],
				std.reserved[2],
				std.reserved[3]
			);

		} else {
			CU_ASSERT_EQUAL(ret, -1);
			CU_ASSERT_EQUAL(errno, EINVAL);

			memset(&std2, 0xff, sizeof(std2));
			std2.index = i;
			CU_ASSERT_EQUAL(memcmp(&std, &std2, sizeof(std)), 0);

			dprintf("\terrno=%i\n", errno);

		}
		i++;
	} while (ret == 0);
}

void test_VIDIOC_ENUMSTD_S32_MAX() {
	int ret;
	struct v4l2_standard std;
	struct v4l2_standard std2;

	memset(&std, 0xff, sizeof(std));
	std.index = (__u32)S32_MAX;
	ret = ioctl(get_video_fd(), VIDIOC_ENUMSTD, &std);

	CU_ASSERT_EQUAL(ret, -1);
	CU_ASSERT_EQUAL(errno, EINVAL);

	memset(&std2, 0xff, sizeof(std2));
	std2.index = (__u32)S32_MAX;
	CU_ASSERT_EQUAL(memcmp(&std, &std2, sizeof(std)), 0);
}

void test_VIDIOC_ENUMSTD_S32_MAX_1() {
	int ret;
	struct v4l2_standard std;
	struct v4l2_standard std2;

	memset(&std, 0xff, sizeof(std));
	std.index = ((__u32)S32_MAX)+1;
	ret = ioctl(get_video_fd(), VIDIOC_ENUMSTD, &std);

	CU_ASSERT_EQUAL(ret, -1);
	CU_ASSERT_EQUAL(errno, EINVAL);

	memset(&std2, 0xff, sizeof(std2));
	std2.index = ((__u32)S32_MAX)+1;
	CU_ASSERT_EQUAL(memcmp(&std, &std2, sizeof(std)), 0);
}


void test_VIDIOC_ENUMSTD_U32_MAX() {
	int ret;
	struct v4l2_standard std;
	struct v4l2_standard std2;

	memset(&std, 0xff, sizeof(std));
	std.index = U32_MAX;
	ret = ioctl(get_video_fd(), VIDIOC_ENUMSTD, &std);

	CU_ASSERT_EQUAL(ret, -1);
	CU_ASSERT_EQUAL(errno, EINVAL);

	memset(&std2, 0xff, sizeof(std2));
	std2.index = U32_MAX;
	CU_ASSERT_EQUAL(memcmp(&std, &std2, sizeof(std)), 0);
}

void test_VIDIOC_ENUMSTD_NULL() {
	int ret;

	ret = ioctl(get_video_fd(), VIDIOC_ENUMSTD, NULL);
	CU_ASSERT_EQUAL(ret, -1);
	CU_ASSERT_EQUAL(errno, EFAULT);

}

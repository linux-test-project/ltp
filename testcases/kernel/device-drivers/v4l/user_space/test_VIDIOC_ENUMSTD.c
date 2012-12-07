/*
 * v4l-test: Test environment for Video For Linux Two API
 *
 * 20 Apr 2009  0.9  Added string content validation
 * 19 Apr 2009  0.8  Also check std field
 * 18 Apr 2009  0.7  More strict check for strings
 *  3 Apr 2009  0.6  Test case for NULL parameter reworked
 * 28 Mar 2009  0.5  Clean up ret and errno variable names and dprintf() output
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
#include "v4l2_validator.h"

#include "test_VIDIOC_ENUMSTD.h"

void test_VIDIOC_ENUMSTD()
{
	int ret_enum, errno_enum;
	struct v4l2_standard std;
	struct v4l2_standard std2;
	__u32 i;

	i = 0;
	do {
		memset(&std, 0xff, sizeof(std));
		std.index = i;
		ret_enum = ioctl(get_video_fd(), VIDIOC_ENUMSTD, &std);
		errno_enum = errno;

		dprintf("\t%s:%u: VIDIOC_ENUMSTD, ret_enum=%i, errno_enum=%i\n",
			__FILE__, __LINE__, ret_enum, errno_enum);

		if (ret_enum == 0) {
			CU_ASSERT_EQUAL(ret_enum, 0);
			CU_ASSERT_EQUAL(std.index, i);
			CU_ASSERT(valid_v4l2_std_id(std.id));

			CU_ASSERT(0 < strlen((char *)std.name));
			CU_ASSERT(valid_string
				  ((char *)std.name, sizeof(std.name)));

			//CU_ASSERT_EQUAL(std.frameperiod.numerator, ?);
			//CU_ASSERT_EQUAL(std.frameperiod.denominator, ?);
			//CU_ASSERT_EQUAL(std.framelines, ?);
			CU_ASSERT_EQUAL(std.reserved[0], 0);
			CU_ASSERT_EQUAL(std.reserved[1], 0);
			CU_ASSERT_EQUAL(std.reserved[2], 0);
			CU_ASSERT_EQUAL(std.reserved[3], 0);

			/* Check if the unused bytes of the name string is also filled
			 * with zeros. Also check if there is any padding byte between
			 * any two fields then this padding byte is also filled with zeros.
			 */
			memset(&std2, 0, sizeof(std2));
			std2.index = std.index;
			std2.id = std.id;
			strncpy((char *)std2.name, (char *)std.name,
				sizeof(std2.name));
			std2.frameperiod.numerator = std.frameperiod.numerator;
			std2.frameperiod.denominator =
			    std.frameperiod.denominator;
			std2.framelines = std.framelines;
			CU_ASSERT_EQUAL(memcmp(&std, &std2, sizeof(std)), 0);

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
				std.reserved[2], std.reserved[3]
			    );

		} else {
			CU_ASSERT_EQUAL(ret_enum, -1);
			CU_ASSERT_EQUAL(errno_enum, EINVAL);

			memset(&std2, 0xff, sizeof(std2));
			std2.index = i;
			CU_ASSERT_EQUAL(memcmp(&std, &std2, sizeof(std)), 0);

		}
		i++;
	} while (ret_enum == 0);
}

void test_VIDIOC_ENUMSTD_S32_MAX()
{
	int ret_enum, errno_enum;
	struct v4l2_standard std;
	struct v4l2_standard std2;

	memset(&std, 0xff, sizeof(std));
	std.index = (__u32) S32_MAX;
	ret_enum = ioctl(get_video_fd(), VIDIOC_ENUMSTD, &std);
	errno_enum = errno;

	CU_ASSERT_EQUAL(ret_enum, -1);
	CU_ASSERT_EQUAL(errno_enum, EINVAL);

	memset(&std2, 0xff, sizeof(std2));
	std2.index = (__u32) S32_MAX;
	CU_ASSERT_EQUAL(memcmp(&std, &std2, sizeof(std)), 0);
}

void test_VIDIOC_ENUMSTD_S32_MAX_1()
{
	int ret_enum, errno_enum;
	struct v4l2_standard std;
	struct v4l2_standard std2;

	memset(&std, 0xff, sizeof(std));
	std.index = ((__u32) S32_MAX) + 1;
	ret_enum = ioctl(get_video_fd(), VIDIOC_ENUMSTD, &std);
	errno_enum = errno;

	CU_ASSERT_EQUAL(ret_enum, -1);
	CU_ASSERT_EQUAL(errno_enum, EINVAL);

	memset(&std2, 0xff, sizeof(std2));
	std2.index = ((__u32) S32_MAX) + 1;
	CU_ASSERT_EQUAL(memcmp(&std, &std2, sizeof(std)), 0);
}

void test_VIDIOC_ENUMSTD_U32_MAX()
{
	int ret_enum, errno_enum;
	struct v4l2_standard std;
	struct v4l2_standard std2;

	memset(&std, 0xff, sizeof(std));
	std.index = U32_MAX;
	ret_enum = ioctl(get_video_fd(), VIDIOC_ENUMSTD, &std);
	errno_enum = errno;

	dprintf("\t%s:%u: VIDIOC_ENUMSTD, ret_enum=%i, errno_enum=%i\n",
		__FILE__, __LINE__, ret_enum, errno_enum);

	CU_ASSERT_EQUAL(ret_enum, -1);
	CU_ASSERT_EQUAL(errno_enum, EINVAL);

	memset(&std2, 0xff, sizeof(std2));
	std2.index = U32_MAX;
	CU_ASSERT_EQUAL(memcmp(&std, &std2, sizeof(std)), 0);
}

void test_VIDIOC_ENUMSTD_NULL()
{
	int ret_enum, errno_enum;
	int ret_null, errno_null;
	struct v4l2_standard std;

	memset(&std, 0xff, sizeof(std));
	std.index = 0;
	ret_enum = ioctl(get_video_fd(), VIDIOC_ENUMSTD, &std);
	errno_enum = errno;

	dprintf("\t%s:%u: VIDIOC_ENUMSTD, ret_enum=%i, errno_enum=%i\n",
		__FILE__, __LINE__, ret_enum, errno_enum);

	ret_null = ioctl(get_video_fd(), VIDIOC_ENUMSTD, NULL);
	errno_null = errno;

	dprintf("\t%s:%u: VIDIOC_ENUMSTD, ret_null=%i, errno_null=%i\n",
		__FILE__, __LINE__, ret_null, errno_null);

	if (ret_enum == 0) {
		CU_ASSERT_EQUAL(ret_enum, 0);
		CU_ASSERT_EQUAL(ret_null, -1);
		CU_ASSERT_EQUAL(errno_null, EFAULT);
	} else {
		CU_ASSERT_EQUAL(ret_enum, -1);
		CU_ASSERT_EQUAL(errno_enum, EINVAL);
		CU_ASSERT_EQUAL(ret_null, -1);
		CU_ASSERT_EQUAL(errno_null, EINVAL);
	}

}

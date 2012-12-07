/*
 * v4l-test: Test environment for Video For Linux Two API
 *
 *  5 Jul 2009  0.10 show_v4l2_input() introduced
 * 20 Apr 2009  0.9  Added string content validation
 * 19 Apr 2009  0.8  Also check std field
 * 18 Apr 2009  0.7  More strict check for strings
 *  3 Apr 2009  0.6  Test case for NULL parameter reworked
 * 28 Mar 2009  0.5  Clean up ret and errno variable names and dprintf() output
 * 18 Jan 2009  0.4  Test case for MAX_EM28XX_INPUT removed, test cases with
 *                   U32_MAX and S32_MAX are enough
 *  1 Jan 2009  0.3  Added index=S32_MAX and S32_MAX+1
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
#include "v4l2_validator.h"
#include "v4l2_show.h"

#include "test_VIDIOC_ENUMINPUT.h"

void test_VIDIOC_ENUMINPUT()
{
	int ret_enum, errno_enum;
	struct v4l2_input input;
	struct v4l2_input input2;
	__u32 i;

	i = 0;
	do {
		memset(&input, 0xff, sizeof(input));
		input.index = i;
		ret_enum = ioctl(get_video_fd(), VIDIOC_ENUMINPUT, &input);
		errno_enum = errno;

		dprintf
		    ("\t%s:%u: VIDIOC_ENUMINPUT, ret_enum=%i, errno_enum=%i\n",
		     __FILE__, __LINE__, ret_enum, errno_enum);

		if (ret_enum == 0) {
			CU_ASSERT_EQUAL(ret_enum, 0);
			CU_ASSERT_EQUAL(input.index, i);

			CU_ASSERT(0 < strlen((char *)input.name));
			CU_ASSERT(valid_string
				  ((char *)input.name, sizeof(input.name)));

			//CU_ASSERT_EQUAL(input.type, ?);
			//CU_ASSERT_EQUAL(input.audioset, ?);
			//CU_ASSERT_EQUAL(input.tuner, ?);
			CU_ASSERT(valid_v4l2_std_id(input.std));
			//CU_ASSERT_EQUAL(input.status, ?);
			CU_ASSERT_EQUAL(input.reserved[0], 0);
			CU_ASSERT_EQUAL(input.reserved[1], 0);
			CU_ASSERT_EQUAL(input.reserved[2], 0);
			CU_ASSERT_EQUAL(input.reserved[3], 0);

			/* Check if the unused bytes of the name string are
			 * also filled with zeros. Also check if there is any
			 * padding byte between any two fields then this
			 * padding byte is also filled with zeros.
			 */
			memset(&input2, 0, sizeof(input2));
			input2.index = input.index;
			strncpy((char *)input2.name, (char *)input.name,
				sizeof(input2.name));
			input2.type = input.type;
			input2.audioset = input.audioset;
			input2.tuner = input.tuner;
			input2.std = input.std;
			input2.status = input.status;
			CU_ASSERT_EQUAL(memcmp(&input, &input2, sizeof(input)),
					0);

			show_v4l2_input(&input);

		} else {
			CU_ASSERT_EQUAL(ret_enum, -1);
			CU_ASSERT_EQUAL(errno_enum, EINVAL);

			memset(&input2, 0xff, sizeof(input2));
			input2.index = i;
			CU_ASSERT_EQUAL(memcmp(&input, &input2, sizeof(input)),
					0);

		}
		i++;
	} while (ret_enum == 0);

}

void test_VIDIOC_ENUMINPUT_S32_MAX()
{
	int ret_enum, errno_enum;
	struct v4l2_input input;
	struct v4l2_input input2;

	memset(&input, 0xff, sizeof(input));
	input.index = (__u32) S32_MAX;
	ret_enum = ioctl(get_video_fd(), VIDIOC_ENUMINPUT, &input);
	errno_enum = errno;

	dprintf("\t%s:%u: VIDIOC_ENUMINPUT, ret_enum=%i, errno_enum=%i\n",
		__FILE__, __LINE__, ret_enum, errno_enum);

	CU_ASSERT_EQUAL(ret_enum, -1);
	CU_ASSERT_EQUAL(errno_enum, EINVAL);

	memset(&input2, 0xff, sizeof(input2));
	input2.index = (__u32) S32_MAX;
	CU_ASSERT_EQUAL(memcmp(&input, &input2, sizeof(input)), 0);
}

void test_VIDIOC_ENUMINPUT_S32_MAX_1()
{
	int ret_enum, errno_enum;
	struct v4l2_input input;
	struct v4l2_input input2;

	memset(&input, 0xff, sizeof(input));
	input.index = ((__u32) S32_MAX) + 1;
	ret_enum = ioctl(get_video_fd(), VIDIOC_ENUMINPUT, &input);
	errno_enum = errno;

	dprintf("\t%s:%u: VIDIOC_ENUMINPUT, ret_enum=%i, errno_enum=%i\n",
		__FILE__, __LINE__, ret_enum, errno_enum);

	CU_ASSERT_EQUAL(ret_enum, -1);
	CU_ASSERT_EQUAL(errno_enum, EINVAL);

	memset(&input2, 0xff, sizeof(input2));
	input2.index = ((__u32) S32_MAX) + 1;
	CU_ASSERT_EQUAL(memcmp(&input, &input2, sizeof(input)), 0);
}

void test_VIDIOC_ENUMINPUT_U32_MAX()
{
	int ret_enum, errno_enum;
	struct v4l2_input input;
	struct v4l2_input input2;

	memset(&input, 0xff, sizeof(input));
	input.index = U32_MAX;
	ret_enum = ioctl(get_video_fd(), VIDIOC_ENUMINPUT, &input);
	errno_enum = errno;

	dprintf("\t%s:%u: VIDIOC_ENUMINPUT, ret_enum=%i, errno_enum=%i\n",
		__FILE__, __LINE__, ret_enum, errno_enum);

	CU_ASSERT_EQUAL(ret_enum, -1);
	CU_ASSERT_EQUAL(errno_enum, EINVAL);

	memset(&input2, 0xff, sizeof(input2));
	input2.index = U32_MAX;
	CU_ASSERT_EQUAL(memcmp(&input, &input2, sizeof(input)), 0);
}

void test_VIDIOC_ENUMINPUT_NULL()
{
	int ret_enum, errno_enum;
	int ret_null, errno_null;
	struct v4l2_input input;

	memset(&input, 0xff, sizeof(input));
	input.index = 0;
	ret_enum = ioctl(get_video_fd(), VIDIOC_ENUMINPUT, &input);
	errno_enum = errno;

	dprintf("\t%s:%u: VIDIOC_ENUMINPUT, ret_enum=%i, errno_enum=%i\n",
		__FILE__, __LINE__, ret_enum, errno_enum);

	ret_null = ioctl(get_video_fd(), VIDIOC_ENUMINPUT, NULL);
	errno_null = errno;

	dprintf("\t%s:%u: VIDIOC_ENUMINPUT, ret_null=%i, errno_null=%i\n",
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

/*
 * v4l-test: Test environment for Video For Linux Two API
 *
 * 20 Apr 2009  0.6  Added string content validation
 * 19 Apr 2009  0.5  Also check std field
 * 18 Apr 2009  0.4  More strict check for strings
 *  3 Apr 2009  0.3  Test case for NULL parameter reworked
 * 28 Mar 2009  0.2  Clean up ret and errno variable names and dprintf() output
 *  1 Jan 2009  0.1  First release
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

#include "test_VIDIOC_ENUMOUTPUT.h"

void test_VIDIOC_ENUMOUTPUT()
{
	int ret_enum, errno_enum;
	struct v4l2_output output;
	struct v4l2_output output2;
	__u32 i;

	i = 0;
	do {
		memset(&output, 0xff, sizeof(output));
		output.index = i;
		ret_enum = ioctl(get_video_fd(), VIDIOC_ENUMOUTPUT, &output);
		errno_enum = errno;

		dprintf
		    ("\t%s:%u: VIDIOC_ENUMOUTPUT, ret_enum=%i, errno_enum=%i\n",
		     __FILE__, __LINE__, ret_enum, errno_enum);

		if (ret_enum == 0) {
			CU_ASSERT_EQUAL(ret_enum, 0);
			CU_ASSERT_EQUAL(output.index, i);

			CU_ASSERT(0 < strlen((char *)output.name));
			CU_ASSERT(valid_string
				  ((char *)output.name, sizeof(output.name)));

			//CU_ASSERT_EQUAL(output.type, ?);
			//CU_ASSERT_EQUAL(output.audioset, ?);
			//CU_ASSERT_EQUAL(output.modulator, ?);
			CU_ASSERT(valid_v4l2_std_id(output.std));
			CU_ASSERT_EQUAL(output.reserved[0], 0);
			CU_ASSERT_EQUAL(output.reserved[1], 0);
			CU_ASSERT_EQUAL(output.reserved[2], 0);
			CU_ASSERT_EQUAL(output.reserved[3], 0);

			/* Check if the unused bytes of the name string are
			 * also filled with zeros. Also check if there is any
			 * padding byte between any two fields then this
			 * padding byte is also filled with zeros.
			 */
			memset(&output2, 0, sizeof(output2));
			output2.index = output.index;
			strncpy((char *)output2.name, (char *)output.name,
				sizeof(output2.name));
			output2.type = output.type;
			output2.audioset = output.audioset;
			output2.modulator = output.modulator;
			output2.std = output.std;
			CU_ASSERT_EQUAL(memcmp
					(&output, &output2, sizeof(output)), 0);

			dprintf("\toutput = {.index=%u, .name=\"%s\", "
				".type=0x%X, .audioset=0x%X, .modulator=0x%X, "
				".std=%llX, "
				".reserved[]={ 0x%X, 0x%X, 0x%X, 0x%X } }\n",
				output.index,
				output.name,
				output.type,
				output.audioset,
				output.modulator,
				output.std,
				output.reserved[0],
				output.reserved[1],
				output.reserved[2], output.reserved[3]
			    );

		} else {
			CU_ASSERT_EQUAL(ret_enum, -1);
			CU_ASSERT_EQUAL(errno_enum, EINVAL);

			memset(&output2, 0xff, sizeof(output2));
			output2.index = i;
			CU_ASSERT_EQUAL(memcmp
					(&output, &output2, sizeof(output)), 0);

			dprintf("\terrno=%i\n", errno);

		}
		i++;
	} while (ret_enum == 0);

}

void test_VIDIOC_ENUMOUTPUT_S32_MAX()
{
	int ret_enum, errno_enum;
	struct v4l2_output output;
	struct v4l2_output output2;

	memset(&output, 0xff, sizeof(output));
	output.index = (__u32) S32_MAX;
	ret_enum = ioctl(get_video_fd(), VIDIOC_ENUMOUTPUT, &output);
	errno_enum = errno;

	CU_ASSERT_EQUAL(ret_enum, -1);
	CU_ASSERT_EQUAL(errno_enum, EINVAL);

	memset(&output2, 0xff, sizeof(output2));
	output2.index = (__u32) S32_MAX;
	CU_ASSERT_EQUAL(memcmp(&output, &output2, sizeof(output)), 0);
}

void test_VIDIOC_ENUMOUTPUT_S32_MAX_1()
{
	int ret_enum, errno_enum;
	struct v4l2_output output;
	struct v4l2_output output2;

	memset(&output, 0xff, sizeof(output));
	output.index = ((__u32) S32_MAX) + 1;
	ret_enum = ioctl(get_video_fd(), VIDIOC_ENUMOUTPUT, &output);
	errno_enum = errno;

	CU_ASSERT_EQUAL(ret_enum, -1);
	CU_ASSERT_EQUAL(errno_enum, EINVAL);

	memset(&output2, 0xff, sizeof(output2));
	output2.index = ((__u32) S32_MAX) + 1;
	CU_ASSERT_EQUAL(memcmp(&output, &output2, sizeof(output)), 0);
}

void test_VIDIOC_ENUMOUTPUT_U32_MAX()
{
	int ret_enum, errno_enum;
	struct v4l2_output output;
	struct v4l2_output output2;

	memset(&output, 0xff, sizeof(output));
	output.index = U32_MAX;
	ret_enum = ioctl(get_video_fd(), VIDIOC_ENUMOUTPUT, &output);
	errno_enum = errno;

	CU_ASSERT_EQUAL(ret_enum, -1);
	CU_ASSERT_EQUAL(errno_enum, EINVAL);

	memset(&output2, 0xff, sizeof(output2));
	output2.index = U32_MAX;
	CU_ASSERT_EQUAL(memcmp(&output, &output2, sizeof(output)), 0);
}

void test_VIDIOC_ENUMOUTPUT_NULL()
{
	int ret_enum, errno_enum;
	int ret_null, errno_null;
	struct v4l2_output output;

	memset(&output, 0xff, sizeof(output));
	output.index = 0;
	ret_enum = ioctl(get_video_fd(), VIDIOC_ENUMOUTPUT, &output);
	errno_enum = errno;

	dprintf("\t%s:%u: VIDIOC_ENUMOUTPUT, ret_enum=%i, errno_enum=%i\n",
		__FILE__, __LINE__, ret_enum, errno_enum);

	ret_null = ioctl(get_video_fd(), VIDIOC_ENUMOUTPUT, NULL);
	errno_null = errno;

	dprintf("\t%s:%u: VIDIOC_ENUMOUTPUT, ret_null=%i, errno_null=%i\n",
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

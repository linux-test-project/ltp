/*
 * v4l-test: Test environment for Video For Linux Two API
 *
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

#include "test_VIDIOC_ENUMOUTPUT.h"

void test_VIDIOC_ENUMOUTPUT() {
	int ret;
	struct v4l2_output output;
	struct v4l2_output output2;
	__u32 i;

	i = 0;
	do {
		memset(&output, 0xff, sizeof(output));
		output.index = i;
		ret = ioctl(get_video_fd(), VIDIOC_ENUMOUTPUT, &output);

		dprintf("VIDIOC_ENUMOUTPUT, ret=%i\n", ret);

		if (ret == 0) {
			CU_ASSERT_EQUAL(ret, 0);
			CU_ASSERT_EQUAL(output.index, i);

			//CU_ASSERT_EQUAL(output.name, ?);
			CU_ASSERT(0 < strlen( (char*)output.name ));

			//CU_ASSERT_EQUAL(output.type, ?);
			//CU_ASSERT_EQUAL(output.audioset, ?);
			//CU_ASSERT_EQUAL(output.modulator, ?);
			//CU_ASSERT_EQUAL(output.std, ?);
			CU_ASSERT_EQUAL(output.reserved[0], 0);
			CU_ASSERT_EQUAL(output.reserved[1], 0);
			CU_ASSERT_EQUAL(output.reserved[2], 0);
			CU_ASSERT_EQUAL(output.reserved[3], 0);

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
				output.reserved[2],
				output.reserved[3]
				);

		} else {
			CU_ASSERT_EQUAL(ret, -1);
			CU_ASSERT_EQUAL(errno, EINVAL);

			memset(&output2, 0xff, sizeof(output2));
			output2.index = i;
			CU_ASSERT_EQUAL(memcmp(&output, &output2, sizeof(output)), 0);

			dprintf("\terrno=%i\n", errno);

		}
		i++;
	} while (ret == 0);

}

void test_VIDIOC_ENUMOUTPUT_S32_MAX() {
	int ret;
	struct v4l2_output output;
	struct v4l2_output output2;

	memset(&output, 0xff, sizeof(output));
	output.index = (__u32)S32_MAX;
	ret = ioctl(get_video_fd(), VIDIOC_ENUMOUTPUT, &output);

	CU_ASSERT_EQUAL(ret, -1);
	CU_ASSERT_EQUAL(errno, EINVAL);

	memset(&output2, 0xff, sizeof(output2));
	output2.index = (__u32)S32_MAX;
	CU_ASSERT_EQUAL(memcmp(&output, &output2, sizeof(output)), 0);
}

void test_VIDIOC_ENUMOUTPUT_S32_MAX_1() {
	int ret;
	struct v4l2_output output;
	struct v4l2_output output2;

	memset(&output, 0xff, sizeof(output));
	output.index = ((__u32)S32_MAX)+1;
	ret = ioctl(get_video_fd(), VIDIOC_ENUMOUTPUT, &output);

	CU_ASSERT_EQUAL(ret, -1);
	CU_ASSERT_EQUAL(errno, EINVAL);

	memset(&output2, 0xff, sizeof(output2));
	output2.index = ((__u32)S32_MAX)+1;
	CU_ASSERT_EQUAL(memcmp(&output, &output2, sizeof(output)), 0);
}

void test_VIDIOC_ENUMOUTPUT_U32_MAX() {
	int ret;
	struct v4l2_output output;
	struct v4l2_output output2;

	memset(&output, 0xff, sizeof(output));
	output.index = U32_MAX;
	ret = ioctl(get_video_fd(), VIDIOC_ENUMOUTPUT, &output);

	CU_ASSERT_EQUAL(ret, -1);
	CU_ASSERT_EQUAL(errno, EINVAL);

	memset(&output2, 0xff, sizeof(output2));
	output2.index = U32_MAX;
	CU_ASSERT_EQUAL(memcmp(&output, &output2, sizeof(output)), 0);
}

void test_VIDIOC_ENUMOUTPUT_NULL() {
	int ret;

	ret = ioctl(get_video_fd(), VIDIOC_ENUMOUTPUT, NULL);
	CU_ASSERT_EQUAL(ret, -1);
	CU_ASSERT_EQUAL(errno, EFAULT);
}

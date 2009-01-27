/*
 * v4l-test: Test environment for Video For Linux Two API
 *
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

#include "test_VIDIOC_ENUMINPUT.h"

void test_VIDIOC_ENUMINPUT() {
	int ret;
	struct v4l2_input input;
	struct v4l2_input input2;
	__u32 i;

	i = 0;
	do {
		memset(&input, 0xff, sizeof(input));
		input.index = i;
		ret = ioctl(get_video_fd(), VIDIOC_ENUMINPUT, &input);

		dprintf("VIDIOC_ENUMINPUT, ret=%i\n", ret);

		if (ret == 0) {
			CU_ASSERT_EQUAL(ret, 0);
			CU_ASSERT_EQUAL(input.index, i);

			//CU_ASSERT_EQUAL(input.name, ?);
			CU_ASSERT(0 < strlen( (char*)input.name ));

			//CU_ASSERT_EQUAL(input.type, ?);
			//CU_ASSERT_EQUAL(input.audioset, ?);
			//CU_ASSERT_EQUAL(input.tuner, ?);
			//CU_ASSERT_EQUAL(input.std, ?);
			//CU_ASSERT_EQUAL(input.status, ?);
			CU_ASSERT_EQUAL(input.reserved[0], 0);
			CU_ASSERT_EQUAL(input.reserved[1], 0);
			CU_ASSERT_EQUAL(input.reserved[2], 0);
			CU_ASSERT_EQUAL(input.reserved[3], 0);

			dprintf("\tinput = {.index=%u, .name=\"%s\", "
				".type=0x%X, .audioset=0x%X, .tuner=0x%X, "
				".std=%llX, "
				".status=0x%X, "
				".reserved[]={ 0x%X, 0x%X, 0x%X, 0x%X } }\n",
				input.index,
				input.name,
				input.type,
				input.audioset,
				input.tuner,
				input.std,
				input.status,
				input.reserved[0],
				input.reserved[1],
				input.reserved[2],
				input.reserved[3]
				);

		} else {
			CU_ASSERT_EQUAL(ret, -1);
			CU_ASSERT_EQUAL(errno, EINVAL);

			memset(&input2, 0xff, sizeof(input2));
			input2.index = i;
			CU_ASSERT_EQUAL(memcmp(&input, &input2, sizeof(input)), 0);

			dprintf("\terrno=%i\n", errno);

		}
		i++;
	} while (ret == 0);

}

void test_VIDIOC_ENUMINPUT_S32_MAX() {
	int ret;
	struct v4l2_input input;
	struct v4l2_input input2;

	memset(&input, 0xff, sizeof(input));
	input.index = (__u32)S32_MAX;
	ret = ioctl(get_video_fd(), VIDIOC_ENUMINPUT, &input);

	CU_ASSERT_EQUAL(ret, -1);
	CU_ASSERT_EQUAL(errno, EINVAL);

	memset(&input2, 0xff, sizeof(input2));
	input2.index = (__u32)S32_MAX;
	CU_ASSERT_EQUAL(memcmp(&input, &input2, sizeof(input)), 0);
}

void test_VIDIOC_ENUMINPUT_S32_MAX_1() {
	int ret;
	struct v4l2_input input;
	struct v4l2_input input2;

	memset(&input, 0xff, sizeof(input));
	input.index = ((__u32)S32_MAX)+1;
	ret = ioctl(get_video_fd(), VIDIOC_ENUMINPUT, &input);

	CU_ASSERT_EQUAL(ret, -1);
	CU_ASSERT_EQUAL(errno, EINVAL);

	memset(&input2, 0xff, sizeof(input2));
	input2.index = ((__u32)S32_MAX)+1;
	CU_ASSERT_EQUAL(memcmp(&input, &input2, sizeof(input)), 0);
}

void test_VIDIOC_ENUMINPUT_U32_MAX() {
	int ret;
	struct v4l2_input input;
	struct v4l2_input input2;

	memset(&input, 0xff, sizeof(input));
	input.index = U32_MAX;
	ret = ioctl(get_video_fd(), VIDIOC_ENUMINPUT, &input);

	CU_ASSERT_EQUAL(ret, -1);
	CU_ASSERT_EQUAL(errno, EINVAL);

	memset(&input2, 0xff, sizeof(input2));
	input2.index = U32_MAX;
	CU_ASSERT_EQUAL(memcmp(&input, &input2, sizeof(input)), 0);
}

void test_VIDIOC_ENUMINPUT_NULL() {
	int ret;

	ret = ioctl(get_video_fd(), VIDIOC_ENUMINPUT, NULL);
	CU_ASSERT_EQUAL(ret, -1);
	CU_ASSERT_EQUAL(errno, EFAULT);

}

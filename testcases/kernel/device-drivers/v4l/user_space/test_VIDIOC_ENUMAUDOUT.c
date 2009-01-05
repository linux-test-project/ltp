/*
 * v4l-test: Test environment for Video For Linux Two API
 *
 *  1 Jan 2009  0.1  First release
 *
 * Written by Márton Németh <nm127@freemail.hu>
 * Released under GPL
 *
 * Note: in ioctl VIDIOC_ENUMAUDOUT reference (doc/spec/r8304.htm)
 * the description text mentions the the VIDIOC_G_AUDOUT should
 * be called. This is a typo in the V4L2 specification (revision 0.24).
 * To enumerate the audio outputs the VIDIOC_ENUMAUDOUT should be
 * called.
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

#include "test_VIDIOC_ENUMAUDOUT.h"

void test_VIDIOC_ENUMAUDOUT() {
	int ret;
	struct v4l2_audioout audioout;
	struct v4l2_audioout audioout2;
	__u32 i;

	i = 0;
	do {
		memset(&audioout, 0xff, sizeof(audioout));
		audioout.index = i;
		ret = ioctl(get_video_fd(), VIDIOC_ENUMAUDOUT, &audioout);

		dprintf("VIDIOC_ENUMAUDOUT, ret=%i\n", ret);

		if (ret == 0) {
			CU_ASSERT_EQUAL(ret, 0);
			CU_ASSERT_EQUAL(audioout.index, i);

			//CU_ASSERT_EQUAL(audioout.name, ?);
			CU_ASSERT(0 < strlen( (char*)audioout.name ));

			//CU_ASSERT_EQUAL(audioout.capability, ?);
			//CU_ASSERT_EQUAL(audioout.mode, ?);
			CU_ASSERT_EQUAL(audioout.reserved[0], 0);
			CU_ASSERT_EQUAL(audioout.reserved[1], 0);

			dprintf("\taudioout = {.index=%u, .name=\"%s\", "
				".capability=0x%X, .mode=0x%X, "
				".reserved[]={ 0x%X, 0x%X } }\n",
				audioout.index,
				audioout.name,
				audioout.capability,
				audioout.mode,
				audioout.reserved[0],
				audioout.reserved[1]
				);

		} else {
			CU_ASSERT_EQUAL(ret, -1);
			CU_ASSERT_EQUAL(errno, EINVAL);

			memset(&audioout2, 0xff, sizeof(audioout2));
			audioout2.index = i;
			CU_ASSERT_EQUAL(memcmp(&audioout, &audioout2, sizeof(audioout)), 0);

			dprintf("\terrno=%i\n", errno);

		}
		i++;
	} while (ret == 0);

}

void test_VIDIOC_ENUMAUDOUT_S32_MAX() {
	int ret;
	struct v4l2_audioout audioout;
	struct v4l2_audioout audioout2;

	memset(&audioout, 0xff, sizeof(audioout));
	audioout.index = (__u32)S32_MAX;
	ret = ioctl(get_video_fd(), VIDIOC_ENUMAUDOUT, &audioout);

	CU_ASSERT_EQUAL(ret, -1);
	CU_ASSERT_EQUAL(errno, EINVAL);

	/* Check whether the original audioout struct is untouched */
	memset(&audioout2, 0xff, sizeof(audioout2));
	audioout2.index = (__u32)S32_MAX;
	CU_ASSERT_EQUAL(memcmp(&audioout, &audioout2, sizeof(audioout)), 0);
}

void test_VIDIOC_ENUMAUDOUT_S32_MAX_1() {
	int ret;
	struct v4l2_audioout audioout;
	struct v4l2_audioout audioout2;

	memset(&audioout, 0xff, sizeof(audioout));
	audioout.index = ((__u32)S32_MAX)+1;
	ret = ioctl(get_video_fd(), VIDIOC_ENUMAUDOUT, &audioout);

	CU_ASSERT_EQUAL(ret, -1);
	CU_ASSERT_EQUAL(errno, EINVAL);

	/* Check whether the original audioout struct is untouched */
	memset(&audioout2, 0xff, sizeof(audioout2));
	audioout2.index = ((__u32)S32_MAX)+1;
	CU_ASSERT_EQUAL(memcmp(&audioout, &audioout2, sizeof(audioout)), 0);
}


void test_VIDIOC_ENUMAUDOUT_U32_MAX() {
	int ret;
	struct v4l2_audioout audioout;
	struct v4l2_audioout audioout2;

	memset(&audioout, 0xff, sizeof(audioout));
	audioout.index = U32_MAX;
	ret = ioctl(get_video_fd(), VIDIOC_ENUMAUDOUT, &audioout);

	CU_ASSERT_EQUAL(ret, -1);
	CU_ASSERT_EQUAL(errno, EINVAL);

	/* Check whether the original audioout struct is untouched */
	memset(&audioout2, 0xff, sizeof(audioout2));
	audioout2.index = U32_MAX;
	CU_ASSERT_EQUAL(memcmp(&audioout, &audioout2, sizeof(audioout)), 0);
}

void test_VIDIOC_ENUMAUDOUT_NULL() {
	int ret;

	ret = ioctl(get_video_fd(), VIDIOC_ENUMAUDOUT, NULL);
	CU_ASSERT_EQUAL(ret, -1);
	CU_ASSERT_EQUAL(errno, EFAULT);

}

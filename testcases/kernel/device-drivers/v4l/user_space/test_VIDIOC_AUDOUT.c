/*
 * v4l-test: Test environment for Video For Linux Two API
 *
 *  3 Feb 2009  0.1  First release
 *
 * Written by Márton Németh <nm127@freemail.hu>
 * Released under GPL
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

#include "v4l2_test.h"
#include "dev_video.h"
#include "video_limits.h"

#include "test_VIDIOC_AUDOUT.h"

int valid_audioout_mode(__u32 mode) {
	int valid = 0;

	if ( (mode & ~(V4L2_AUDMODE_AVL))
		== 0) {
		valid = 1;
	} else {
		valid = 0;
	}
	return valid;
}

void test_VIDIOC_G_AUDOUT() {
	int ret;
	struct v4l2_audioout audioout;
	struct v4l2_audioout audioout2;

	memset(&audioout, 0xff, sizeof(audioout));
	ret = ioctl(get_video_fd(), VIDIOC_G_AUDOUT, &audioout);

	dprintf("VIDIOC_AUDIOOUT, ret=%i\n", ret);

	if (ret == 0) {
		CU_ASSERT_EQUAL(ret, 0);

		//CU_ASSERT_EQUAL(audioout.index, ?);

		//CU_ASSERT_EQUAL(audioout.name, ?);
		CU_ASSERT(0 < strlen( (char*)audioout.name ));
		CU_ASSERT(strlen( (char*)audioout.name ) < sizeof(audioout.name));

		CU_ASSERT_EQUAL(audioout.capability, 0);
		CU_ASSERT_EQUAL(audioout.mode, 0);

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

		/* check if the audioout structure is untouched */
		memset(&audioout2, 0xff, sizeof(audioout2));
		CU_ASSERT_EQUAL(memcmp(&audioout, &audioout2, sizeof(audioout)), 0);

		dprintf("\terrno=%i\n", errno);

	}

}

void test_VIDIOC_G_AUDOUT_NULL() {
	int ret1;
	int errno1;
	int ret2;
	int errno2;
	struct v4l2_audioout audioout;

	memset(&audioout, 0xff, sizeof(audioout));
	ret1 = ioctl(get_video_fd(), VIDIOC_G_AUDOUT, &audioout);
	errno1 = errno;

	dprintf("VIDIOC_AUDIOOUT, ret1=%i\n", ret1);

	ret2 = ioctl(get_video_fd(), VIDIOC_G_AUDOUT, NULL);
	errno2 = errno;

	/* check if VIDIOC_G_AUDOUT is supported at all or not */
	if (ret1 == -1 && errno1 == EINVAL) {
		/* VIDIOC_G_AUDOUT not supported at all, the parameter should not be evaluated */
		dprintf("\t%s:%u: ret2=%d (expected %d)\n", __FILE__, __LINE__, ret2, -1);
		dprintf("\t%s:%u: errno2=%d (expected %d)\n", __FILE__, __LINE__, errno2, EINVAL);
		CU_ASSERT_EQUAL(ret2, -1);
		CU_ASSERT_EQUAL(errno, EINVAL);

	} else {
		/* VIDIOC_G_AUDOUT is supported, the parameter should be checked */
		dprintf("\t%s:%u: ret2=%d (expected %d)\n", __FILE__, __LINE__, ret2, -1);
		dprintf("\t%s:%u: errno2=%d (expected %d)\n", __FILE__, __LINE__, errno2, EFAULT);
		CU_ASSERT_EQUAL(ret2, -1);
		CU_ASSERT_EQUAL(errno2, EFAULT);
	}

}

/* TODO: - try with all possible outputs (VIDIOC_ENUMOUTPUT)
 *       - try with STREAM_ON
 */

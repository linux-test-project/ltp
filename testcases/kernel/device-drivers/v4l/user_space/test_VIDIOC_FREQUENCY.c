/*
 * v4l-test: Test environment for Video For Linux Two API
 *
 * 31 Jan 2009  0.1  First release
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

#include "v4l2_test.h"
#include "dev_video.h"
#include "video_limits.h"

#include "test_VIDIOC_FREQUENCY.h"

void test_VIDIOC_G_FREQUENCY() {
	int ret;
	__u32 tuner;
	struct v4l2_frequency freq;

	tuner = 0;

	memset(&freq, 0xff, sizeof(freq));
	freq.tuner = tuner;
	ret = ioctl(get_video_fd(), VIDIOC_G_FREQUENCY, &freq);

	dprintf("VIDIOC_G_FREQUENCY, ret=%i\n", ret);

	if (ret == 0) {
		CU_ASSERT_EQUAL(freq.tuner, tuner);

		//CU_ASSERT(freq.type, ???);
		//CU_ASSERT_EQUAL(freq.frequency, ???);

		CU_ASSERT_EQUAL(freq.reserved[0], 0);
		CU_ASSERT_EQUAL(freq.reserved[1], 0);
		CU_ASSERT_EQUAL(freq.reserved[2], 0);
		CU_ASSERT_EQUAL(freq.reserved[3], 0);
		CU_ASSERT_EQUAL(freq.reserved[4], 0);
		CU_ASSERT_EQUAL(freq.reserved[5], 0);
		CU_ASSERT_EQUAL(freq.reserved[6], 0);
		CU_ASSERT_EQUAL(freq.reserved[7], 0);

		dprintf("\tfreq = { "
			".tuner = %u, "
			".type = 0x%X, "
			".frequency = %u "
			".reserved[]={ 0x%X, 0x%X, 0x%X, 0x%X, 0x%X, 0x%X, 0x%X, 0x%X } }\n",
			freq.tuner,
			freq.type,
			freq.frequency,
			freq.reserved[0],
			freq.reserved[1],
			freq.reserved[2],
			freq.reserved[3],
			freq.reserved[4],
			freq.reserved[5],
			freq.reserved[6],
			freq.reserved[7]
		);
	} else {
		dprintf("\t%s:%u: ret=%d (expected %d)\n", __FILE__, __LINE__, ret, -1);
		dprintf("\t%s:%u: errno=%d (expected %d)\n", __FILE__, __LINE__, errno, EINVAL);
		CU_ASSERT_EQUAL(ret, -1);
		CU_ASSERT_EQUAL(errno, EINVAL);
	}

}

void test_VIDIOC_G_FREQUENCY_S32_MAX() {
	int ret;
	__u32 tuner;
	struct v4l2_frequency freq;

	tuner = (__u32)S32_MAX;

	memset(&tuner, 0xff, sizeof(tuner));
	freq.tuner = tuner;
	ret = ioctl(get_video_fd(), VIDIOC_G_FREQUENCY, &freq);

	dprintf("VIDIOC_G_FREQUENCY, ret=%i\n", ret);

	dprintf("\t%s:%u: ret=%d (expected %d)\n", __FILE__, __LINE__, ret, -1);
	dprintf("\t%s:%u: errno=%d (expected %d)\n", __FILE__, __LINE__, errno, EINVAL);
	CU_ASSERT_EQUAL(ret, -1);
	CU_ASSERT_EQUAL(errno, EINVAL);
}

void test_VIDIOC_G_FREQUENCY_S32_MAX_1() {
	int ret;
	__u32 tuner;
	struct v4l2_frequency freq;

	tuner = (__u32)S32_MAX+1;

	memset(&tuner, 0xff, sizeof(tuner));
	freq.tuner = tuner;
	ret = ioctl(get_video_fd(), VIDIOC_G_FREQUENCY, &freq);

	dprintf("VIDIOC_G_FREQUENCY, ret=%i\n", ret);

	dprintf("\t%s:%u: ret=%d (expected %d)\n", __FILE__, __LINE__, ret, -1);
	dprintf("\t%s:%u: errno=%d (expected %d)\n", __FILE__, __LINE__, errno, EINVAL);
	CU_ASSERT_EQUAL(ret, -1);
	CU_ASSERT_EQUAL(errno, EINVAL);
}

void test_VIDIOC_G_FREQUENCY_U32_MAX() {
	int ret;
	__u32 tuner;
	struct v4l2_frequency freq;

	tuner = U32_MAX;

	memset(&tuner, 0xff, sizeof(tuner));
	freq.tuner = tuner;
	ret = ioctl(get_video_fd(), VIDIOC_G_FREQUENCY, &freq);

	dprintf("VIDIOC_G_FREQUENCY, ret=%i\n", ret);

	dprintf("\t%s:%u: ret=%d (expected %d)\n", __FILE__, __LINE__, ret, -1);
	dprintf("\t%s:%u: errno=%d (expected %d)\n", __FILE__, __LINE__, errno, EINVAL);
	CU_ASSERT_EQUAL(ret, -1);
	CU_ASSERT_EQUAL(errno, EINVAL);
}

void test_VIDIOC_G_FREQUENCY_NULL() {
	int ret1;
	int errno1;
	int ret2;
	struct v4l2_frequency freq;

	memset(&freq, 0xff, sizeof(freq));
	freq.tuner = 0;
	ret1 = ioctl(get_video_fd(), VIDIOC_G_FREQUENCY, &freq);
	errno1 = errno;

	ret2 = ioctl(get_video_fd(), VIDIOC_G_FREQUENCY, NULL);

	/* check if VIDIOC_G_FREQUENCY is supported at all or not */
	if (ret1 == -1 && errno1 == EINVAL) {
		/* VIDIOC_G_FREQUENCY not supported at all, the parameter should not be evaluated */
		dprintf("\t%s:%u: ret2=%d (expected %d)\n", __FILE__, __LINE__, ret2, -1);
		dprintf("\t%s:%u: errno=%d (expected %d)\n", __FILE__, __LINE__, errno, EINVAL);
		CU_ASSERT_EQUAL(ret2, -1);
		CU_ASSERT_EQUAL(errno, EINVAL);

	} else {
		/* VIDIOC_G_FREQUENCY is supported, the parameter should be checked */
		dprintf("\t%s:%u: ret2=%d (expected %d)\n", __FILE__, __LINE__, ret2, -1);
		dprintf("\t%s:%u: errno=%d (expected %d)\n", __FILE__, __LINE__, errno, EFAULT);
		CU_ASSERT_EQUAL(ret2, -1);
		CU_ASSERT_EQUAL(errno, EFAULT);
	}
}

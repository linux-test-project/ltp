/*
 * v4l-test: Test environment for Video For Linux Two API
 *
 *  2 Feb 2009  0.1  First release
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
#include "v4l2_validator.h"

#include "test_VIDIOC_MODULATOR.h"

int valid_modulator_sub(__u32 tuner_sub) {
	int valid = 0;

	CU_ASSERT_EQUAL(V4L2_TUNER_SUB_SAP, V4L2_TUNER_SUB_LANG2);

	if ( (tuner_sub & ~(V4L2_TUNER_SUB_MONO |
			    V4L2_TUNER_SUB_STEREO |
			    V4L2_TUNER_SUB_LANG1 |
			    V4L2_TUNER_SUB_LANG2 |
			    V4L2_TUNER_SUB_SAP))
		== 0) {
		valid = 1;
	} else {
		valid = 0;
	}
	return valid;
}

static int do_get_modulator(int f, __u32 index) {
	int ret;
	struct v4l2_modulator modulator;

	memset(&modulator, 0xff, sizeof(modulator));
	modulator.index = index;
	ret = ioctl(f, VIDIOC_G_MODULATOR, &modulator);

	dprintf("VIDIOC_G_MODULATOR, ret=%i\n", ret);

	if (ret == 0) {
		CU_ASSERT_EQUAL(ret, 0);

		CU_ASSERT_EQUAL(modulator.index, index);

		CU_ASSERT(0 < strlen( (char*)modulator.name ));

		CU_ASSERT(valid_modulator_capability(modulator.capability));

		CU_ASSERT(modulator.rangelow <= modulator.rangehigh);

		CU_ASSERT(valid_modulator_sub(modulator.txsubchans));

		CU_ASSERT_EQUAL(modulator.reserved[0], 0);
		CU_ASSERT_EQUAL(modulator.reserved[1], 0);
		CU_ASSERT_EQUAL(modulator.reserved[2], 0);
		CU_ASSERT_EQUAL(modulator.reserved[3], 0);

		dprintf("\tmodulator = { "
			".index = %u, "
			".name = \"%s\", "
			".capability = 0x%X, "
			".rangelow = %u, "
			".rangehigh = %u, "
			".txsubchans = %u, "
			".reserved[]={ 0x%X, 0x%X, 0x%X, 0x%X } }\n",
			modulator.index,
			modulator.name,
			modulator.capability,
			modulator.rangelow,
			modulator.rangehigh,
			modulator.txsubchans,
			modulator.reserved[0],
			modulator.reserved[1],
			modulator.reserved[2],
			modulator.reserved[3]
		);

	} else {
		dprintf("\t%s:%u: ret=%d (expected %d)\n", __FILE__, __LINE__, ret, -1);
		dprintf("\t%s:%u: errno=%d (expected %d)\n", __FILE__, __LINE__, errno, EINVAL);
		CU_ASSERT_EQUAL(ret, -1);
		CU_ASSERT_EQUAL(errno, EINVAL);
	}

	return ret;
}

void test_VIDIOC_G_MODULATOR() {
	int ret;
	__u32 index;
	int f;

	f = get_video_fd();

	index = 0;
	do {
		ret = do_get_modulator(f, index);
		index++;
	} while (ret == 0);

}

void test_VIDIOC_G_MODULATOR_S32_MAX() {
	int ret;
	__u32 index;
	struct v4l2_modulator modulator;

	index = (__u32)S32_MAX;

	memset(&modulator, 0xff, sizeof(modulator));
	modulator.index = index;
	ret = ioctl(get_video_fd(), VIDIOC_G_MODULATOR, &modulator);

	dprintf("VIDIOC_G_MODULATOR, ret=%i\n", ret);

	dprintf("\t%s:%u: ret=%d (expected %d)\n", __FILE__, __LINE__, ret, -1);
	dprintf("\t%s:%u: errno=%d (expected %d)\n", __FILE__, __LINE__, errno, EINVAL);
	CU_ASSERT_EQUAL(ret, -1);
	CU_ASSERT_EQUAL(errno, EINVAL);
}

void test_VIDIOC_G_MODULATOR_S32_MAX_1() {
	int ret;
	__u32 index;
	struct v4l2_modulator modulator;

	index = (__u32)S32_MAX+1;

	memset(&modulator, 0xff, sizeof(modulator));
	modulator.index = index;
	ret = ioctl(get_video_fd(), VIDIOC_G_MODULATOR, &modulator);

	dprintf("VIDIOC_G_MODULATOR, ret=%i\n", ret);

	dprintf("\t%s:%u: ret=%d (expected %d)\n", __FILE__, __LINE__, ret, -1);
	dprintf("\t%s:%u: errno=%d (expected %d)\n", __FILE__, __LINE__, errno, EINVAL);
	CU_ASSERT_EQUAL(ret, -1);
	CU_ASSERT_EQUAL(errno, EINVAL);
}

void test_VIDIOC_G_MODULATOR_U32_MAX() {
	int ret;
	__u32 index;
	struct v4l2_modulator modulator;

	index = U32_MAX;

	memset(&modulator, 0xff, sizeof(modulator));
	modulator.index = index;
	ret = ioctl(get_video_fd(), VIDIOC_G_MODULATOR, &modulator);

	dprintf("VIDIOC_G_MODULATOR, ret=%i\n", ret);

	dprintf("\t%s:%u: ret=%d (expected %d)\n", __FILE__, __LINE__, ret, -1);
	dprintf("\t%s:%u: errno=%d (expected %d)\n", __FILE__, __LINE__, errno, EINVAL);
	CU_ASSERT_EQUAL(ret, -1);
	CU_ASSERT_EQUAL(errno, EINVAL);
}

void test_VIDIOC_G_MODULATOR_NULL() {
	int ret1;
	int errno1;
	int ret2;
	struct v4l2_modulator modulator;

	memset(&modulator, 0xff, sizeof(modulator));
	modulator.index = 0;
	ret1 = ioctl(get_video_fd(), VIDIOC_G_MODULATOR, &modulator);
	errno1 = errno;

	ret2 = ioctl(get_video_fd(), VIDIOC_G_MODULATOR, NULL);

	/* check if VIDIOC_G_MODULATOR is supported at all or not */
	if (ret1 == -1 && errno1 == EINVAL) {
		/* VIDIOC_G_MODULATOR not supported at all, the parameter should not be evaluated */
		dprintf("\t%s:%u: ret2=%d (expected %d)\n", __FILE__, __LINE__, ret2, -1);
		dprintf("\t%s:%u: errno=%d (expected %d)\n", __FILE__, __LINE__, errno, EINVAL);
		CU_ASSERT_EQUAL(ret2, -1);
		CU_ASSERT_EQUAL(errno, EINVAL);

	} else {
		/* VIDIOC_G_MODULATOR is supported, the parameter should be checked */
		dprintf("\t%s:%u: ret2=%d (expected %d)\n", __FILE__, __LINE__, ret2, -1);
		dprintf("\t%s:%u: errno=%d (expected %d)\n", __FILE__, __LINE__, errno, EFAULT);
		CU_ASSERT_EQUAL(ret2, -1);
		CU_ASSERT_EQUAL(errno, EFAULT);
	}

}

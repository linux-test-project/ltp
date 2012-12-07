/*
 * v4l-test: Test environment for Video For Linux Two API
 *
 * 20 Apr 2009  0.4  Added string content validation
 * 18 Apr 2009  0.3  More strict check for strings
 * 28 Mar 2009  0.2  Clean up ret and errno variable names and dprintf() output
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

int valid_modulator_sub(__u32 tuner_sub)
{
	int valid = 0;

	CU_ASSERT_EQUAL(V4L2_TUNER_SUB_SAP, V4L2_TUNER_SUB_LANG2);

	if ((tuner_sub & ~(V4L2_TUNER_SUB_MONO |
			   V4L2_TUNER_SUB_STEREO |
			   V4L2_TUNER_SUB_LANG1 |
			   V4L2_TUNER_SUB_LANG2 | V4L2_TUNER_SUB_SAP))
	    == 0) {
		valid = 1;
	} else {
		valid = 0;
	}
	return valid;
}

static int do_get_modulator(int f, __u32 index)
{
	int ret_get, errno_get;
	struct v4l2_modulator modulator;
	struct v4l2_modulator modulator2;

	memset(&modulator, 0xff, sizeof(modulator));
	modulator.index = index;
	ret_get = ioctl(f, VIDIOC_G_MODULATOR, &modulator);
	errno_get = errno;

	dprintf("\t%s:%u: VIDIOC_G_MODULATOR, ret_get=%i, errno_get=%i\n",
		__FILE__, __LINE__, ret_get, errno_get);

	if (ret_get == 0) {
		CU_ASSERT_EQUAL(ret_get, 0);

		CU_ASSERT_EQUAL(modulator.index, index);

		CU_ASSERT(0 < strlen((char *)modulator.name));
		CU_ASSERT(valid_string
			  ((char *)modulator.name, sizeof(modulator.name)));

		CU_ASSERT(valid_modulator_capability(modulator.capability));

		CU_ASSERT(modulator.rangelow <= modulator.rangehigh);

		CU_ASSERT(valid_modulator_sub(modulator.txsubchans));

		CU_ASSERT_EQUAL(modulator.reserved[0], 0);
		CU_ASSERT_EQUAL(modulator.reserved[1], 0);
		CU_ASSERT_EQUAL(modulator.reserved[2], 0);
		CU_ASSERT_EQUAL(modulator.reserved[3], 0);

		/* Check if the unused bytes of the name string are also filled
		 * with zeros. Also check if there is any padding byte between
		 * any two fields then this padding byte is also filled with
		 * zeros.
		 */
		memset(&modulator2, 0, sizeof(modulator2));
		modulator2.index = modulator.index;
		strncpy((char *)modulator2.name, (char *)modulator.name,
			sizeof(modulator2.name));
		modulator2.capability = modulator.capability;
		modulator2.rangelow = modulator.rangelow;
		modulator2.rangehigh = modulator.rangehigh;
		modulator2.txsubchans = modulator.txsubchans;
		CU_ASSERT_EQUAL(memcmp
				(&modulator, &modulator2, sizeof(modulator)),
				0);

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
			modulator.reserved[2], modulator.reserved[3]
		    );

	} else {
		dprintf("\t%s:%u: ret_get=%d (expected %d)\n", __FILE__,
			__LINE__, ret_get, -1);
		dprintf("\t%s:%u: errno_get=%d (expected %d)\n", __FILE__,
			__LINE__, errno_get, EINVAL);
		CU_ASSERT_EQUAL(ret_get, -1);
		CU_ASSERT_EQUAL(errno_get, EINVAL);
	}

	return ret_get;
}

void test_VIDIOC_G_MODULATOR()
{
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

void test_VIDIOC_G_MODULATOR_S32_MAX()
{
	int ret_get, errno_get;
	__u32 index;
	struct v4l2_modulator modulator;

	index = (__u32) S32_MAX;

	memset(&modulator, 0xff, sizeof(modulator));
	modulator.index = index;
	ret_get = ioctl(get_video_fd(), VIDIOC_G_MODULATOR, &modulator);
	errno_get = errno;

	dprintf("\t%s:%u: VIDIOC_G_MODULATOR, ret_get=%i, errno_get=%i\n",
		__FILE__, __LINE__, ret_get, errno_get);

	dprintf("\t%s:%u: ret_get=%d (expected %d)\n", __FILE__, __LINE__,
		ret_get, -1);
	dprintf("\t%s:%u: errno_get=%d (expected %d)\n", __FILE__, __LINE__,
		errno_get, EINVAL);
	CU_ASSERT_EQUAL(ret_get, -1);
	CU_ASSERT_EQUAL(errno_get, EINVAL);
}

void test_VIDIOC_G_MODULATOR_S32_MAX_1()
{
	int ret_get, errno_get;
	__u32 index;
	struct v4l2_modulator modulator;

	index = (__u32) S32_MAX + 1;

	memset(&modulator, 0xff, sizeof(modulator));
	modulator.index = index;
	ret_get = ioctl(get_video_fd(), VIDIOC_G_MODULATOR, &modulator);
	errno_get = errno;

	dprintf("VIDIOC_G_MODULATOR, ret_get=%i\n", ret_get);

	dprintf("\t%s:%u: ret_get=%d (expected %d)\n", __FILE__, __LINE__,
		ret_get, -1);
	dprintf("\t%s:%u: errno_get=%d (expected %d)\n", __FILE__, __LINE__,
		errno_get, EINVAL);
	CU_ASSERT_EQUAL(ret_get, -1);
	CU_ASSERT_EQUAL(errno_get, EINVAL);
}

void test_VIDIOC_G_MODULATOR_U32_MAX()
{
	int ret_get, errno_get;
	__u32 index;
	struct v4l2_modulator modulator;

	index = U32_MAX;

	memset(&modulator, 0xff, sizeof(modulator));
	modulator.index = index;
	ret_get = ioctl(get_video_fd(), VIDIOC_G_MODULATOR, &modulator);
	errno_get = errno;

	dprintf("\t%s:%u: VIDIOC_G_MODULATOR, ret_get=%i, errno_get=%i\n",
		__FILE__, __LINE__, ret_get, errno_get);

	dprintf("\t%s:%u: ret_get=%d (expected %d)\n", __FILE__, __LINE__,
		ret_get, -1);
	dprintf("\t%s:%u: errno_get=%d (expected %d)\n", __FILE__, __LINE__,
		errno_get, EINVAL);
	CU_ASSERT_EQUAL(ret_get, -1);
	CU_ASSERT_EQUAL(errno_get, EINVAL);
}

void test_VIDIOC_G_MODULATOR_NULL()
{
	int ret_get, errno_get;
	int ret_null, errno_null;
	struct v4l2_modulator modulator;

	memset(&modulator, 0xff, sizeof(modulator));
	modulator.index = 0;
	ret_get = ioctl(get_video_fd(), VIDIOC_G_MODULATOR, &modulator);
	errno_get = errno;

	dprintf("\t%s:%u: VIDIOC_G_MODULATOR, ret_get=%i, errno_get=%i\n",
		__FILE__, __LINE__, ret_get, errno_get);

	ret_null = ioctl(get_video_fd(), VIDIOC_G_MODULATOR, NULL);
	errno_null = errno;

	dprintf("\t%s:%u: VIDIOC_G_MODULATOR, ret_null=%i, errno_null=%i\n",
		__FILE__, __LINE__, ret_null, errno_null);

	/* check if VIDIOC_G_MODULATOR is supported at all or not */
	if (ret_get == 0) {
		/* VIDIOC_G_MODULATOR is supported, the parameter should be checked */
		CU_ASSERT_EQUAL(ret_get, 0);
		CU_ASSERT_EQUAL(ret_null, -1);
		CU_ASSERT_EQUAL(errno_null, EFAULT);
	} else {
		/* VIDIOC_G_MODULATOR not supported at all, the parameter should not be evaluated */
		CU_ASSERT_EQUAL(ret_get, -1);
		CU_ASSERT_EQUAL(errno_get, EINVAL);
		CU_ASSERT_EQUAL(ret_null, -1);
		CU_ASSERT_EQUAL(errno_null, EINVAL);
	}

}

/* TODO: test cases for VIDIOC_S_MODULATOR */

/*
 * v4l-test: Test environment for Video For Linux Two API
 *
 * 20 Apr 2009  0.5  Added string content validation
 * 18 Apr 2009  0.4  More strict check for strings
 * 28 Mar 2009  0.3  Clean up ret and errno variable names and dprintf() output
 *  9 Feb 2009  0.2  Added test cases for VIDIOC_S_TUNER;
 *                   Some typos corrected;
 *                   Add some debug messages
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
#include "v4l2_validator.h"

#include "test_VIDIOC_TUNER.h"

int valid_tuner_type(enum v4l2_tuner_type type)
{
	int valid = 0;

	switch (type) {
	case V4L2_TUNER_RADIO:
	case V4L2_TUNER_ANALOG_TV:
		valid = 1;
		break;
	default:
		valid = 0;
	}

	return valid;
}

int valid_tuner_sub(__u32 tuner_sub)
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

int valid_tuner_audmode(__u32 audmode)
{
	int valid = 0;

	CU_ASSERT_EQUAL(V4L2_TUNER_MODE_SAP, V4L2_TUNER_MODE_LANG2);

	switch (audmode) {
	case V4L2_TUNER_MODE_MONO:
	case V4L2_TUNER_MODE_STEREO:
	case V4L2_TUNER_MODE_LANG1:
	case V4L2_TUNER_MODE_LANG2:
	case V4L2_TUNER_MODE_LANG1_LANG2:
		valid = 1;
		break;
	default:
		valid = 0;
	}

	return valid;
}

static int do_get_tuner(int f, __u32 index)
{
	int ret_get, errno_get;
	struct v4l2_tuner tuner;
	struct v4l2_tuner tuner2;

	memset(&tuner, 0xff, sizeof(tuner));
	tuner.index = index;
	ret_get = ioctl(f, VIDIOC_G_TUNER, &tuner);
	errno_get = errno;

	dprintf("\t%s:%u: VIDIOC_G_TUNER, ret_get=%i, errno_get=%i\n",
		__FILE__, __LINE__, ret_get, errno_get);

	if (ret_get == 0) {
		CU_ASSERT_EQUAL(ret_get, 0);

		CU_ASSERT_EQUAL(tuner.index, index);

		CU_ASSERT(0 < strlen((char *)tuner.name));
		CU_ASSERT(valid_string((char *)tuner.name, sizeof(tuner.name)));

		CU_ASSERT(valid_tuner_type(tuner.type));
		CU_ASSERT(valid_tuner_capability(tuner.capability));

		CU_ASSERT(tuner.rangelow <= tuner.rangehigh);
		CU_ASSERT(valid_tuner_sub(tuner.rxsubchans));
		CU_ASSERT(valid_tuner_audmode(tuner.audmode));

		/* tuner.signal can have any value */
		//CU_ASSERT_EQUAL(tuner.signal, ???);

		/* tuner.afc can have any value */
		//CU_ASSERT_EQUAL(tuner.afc, ???);

		CU_ASSERT_EQUAL(tuner.reserved[0], 0);
		CU_ASSERT_EQUAL(tuner.reserved[1], 0);
		CU_ASSERT_EQUAL(tuner.reserved[2], 0);
		CU_ASSERT_EQUAL(tuner.reserved[3], 0);

		/* Check if the unused bytes of the name string are also filled
		 * with zeros. Also check if there is any padding byte between
		 * any two fields then this padding byte is also filled with
		 * zeros.
		 */
		memset(&tuner2, 0, sizeof(tuner2));
		tuner2.index = tuner.index;
		strncpy((char *)tuner2.name, (char *)tuner.name,
			sizeof(tuner2.name));
		tuner2.type = tuner.type;
		tuner2.capability = tuner.capability;
		tuner2.rangelow = tuner.rangelow;
		tuner2.rangehigh = tuner.rangehigh;
		tuner2.rxsubchans = tuner.rxsubchans;
		tuner2.audmode = tuner.audmode;
		tuner2.signal = tuner.signal;
		tuner2.afc = tuner.afc;
		CU_ASSERT_EQUAL(memcmp(&tuner, &tuner2, sizeof(tuner)), 0);

		dprintf("\ttuner = { "
			".index = %u, "
			".name = \"%s\", "
			".type = %i, "
			".capability = %u, "
			".rangelow = %u, "
			".rangehigh = %u, "
			".rxsubchans = %u, "
			".audmode = %u, "
			".signal = %u, "
			".afc = %i, "
			".reserved[]={ 0x%X, 0x%X, 0x%X, 0x%X } }\n",
			tuner.index,
			tuner.name,
			tuner.type,
			tuner.capability,
			tuner.rangelow,
			tuner.rangehigh,
			tuner.rxsubchans,
			tuner.audmode,
			tuner.signal,
			tuner.afc,
			tuner.reserved[0],
			tuner.reserved[1], tuner.reserved[2], tuner.reserved[3]
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

void test_VIDIOC_G_TUNER()
{
	int ret;
	__u32 index;
	int f;

	f = get_video_fd();

	index = 0;
	do {
		ret = do_get_tuner(f, index);
		index++;
	} while (ret == 0);

}

void test_VIDIOC_G_TUNER_S32_MAX()
{
	int ret_get, errno_get;
	__u32 index;
	struct v4l2_tuner tuner;

	index = (__u32) S32_MAX;

	memset(&tuner, 0xff, sizeof(tuner));
	tuner.index = index;
	ret_get = ioctl(get_video_fd(), VIDIOC_G_TUNER, &tuner);
	errno_get = errno;

	dprintf("\t%s:%u: VIDIOC_G_TUNER, ret_get=%i, errno_get=%i\n",
		__FILE__, __LINE__, ret_get, errno_get);

	dprintf("\t%s:%u: ret_get=%d (expected %d)\n", __FILE__, __LINE__,
		ret_get, -1);
	dprintf("\t%s:%u: errno_get=%d (expected %d)\n", __FILE__, __LINE__,
		errno_get, EINVAL);
	CU_ASSERT_EQUAL(ret_get, -1);
	CU_ASSERT_EQUAL(errno_get, EINVAL);
}

void test_VIDIOC_G_TUNER_S32_MAX_1()
{
	int ret_get, errno_get;
	__u32 index;
	struct v4l2_tuner tuner;

	index = (__u32) S32_MAX + 1;

	memset(&tuner, 0xff, sizeof(tuner));
	tuner.index = index;
	ret_get = ioctl(get_video_fd(), VIDIOC_G_TUNER, &tuner);
	errno_get = errno;

	dprintf("\t%s:%u: VIDIOC_G_TUNER, ret_get=%i\n",
		__FILE__, __LINE__, ret_get);

	dprintf("\t%s:%u: ret_get=%d (expected %d)\n", __FILE__, __LINE__,
		ret_get, -1);
	dprintf("\t%s:%u: errno_get=%d (expected %d)\n", __FILE__, __LINE__,
		errno_get, EINVAL);
	CU_ASSERT_EQUAL(ret_get, -1);
	CU_ASSERT_EQUAL(errno_get, EINVAL);
}

void test_VIDIOC_G_TUNER_U32_MAX()
{
	int ret_get, errno_get;
	__u32 index;
	struct v4l2_tuner tuner;

	index = U32_MAX;

	memset(&tuner, 0xff, sizeof(tuner));
	tuner.index = index;
	ret_get = ioctl(get_video_fd(), VIDIOC_G_TUNER, &tuner);
	errno_get = errno;

	dprintf("\t%s:%u: VIDIOC_G_TUNER, ret_get=%i, errno_get=%i\n",
		__FILE__, __LINE__, ret_get, errno_get);

	dprintf("\t%s:%u: ret_get=%d (expected %d)\n", __FILE__, __LINE__,
		ret_get, -1);
	dprintf("\t%s:%u: errno_get=%d (expected %d)\n", __FILE__, __LINE__,
		errno_get, EINVAL);
	CU_ASSERT_EQUAL(ret_get, -1);
	CU_ASSERT_EQUAL(errno_get, EINVAL);
}

void test_VIDIOC_G_TUNER_NULL()
{
	int ret_get, errno_get;
	int ret_null, errno_null;
	struct v4l2_tuner tuner;

	memset(&tuner, 0xff, sizeof(tuner));
	tuner.index = 0;
	ret_get = ioctl(get_video_fd(), VIDIOC_G_TUNER, &tuner);
	errno_get = errno;
	dprintf("\t%s:%u: VIDIOC_G_TUNER: ret_get=%i, errno_get=%i\n",
		__FILE__, __LINE__, ret_get, errno_get);

	ret_null = ioctl(get_video_fd(), VIDIOC_G_TUNER, NULL);
	errno_null = errno;

	dprintf("\t%s:%u: VIDIOC_G_TUNER: ret_null=%i, errno_null=%i\n",
		__FILE__, __LINE__, ret_null, errno_null);

	/* check if VIDIOC_G_TUNER is supported at all or not */
	if (ret_get == 0) {
		/* VIDIOC_G_TUNER is supported, the parameter should be checked */
		CU_ASSERT_EQUAL(ret_get, 0);
		CU_ASSERT_EQUAL(ret_null, -1);
		CU_ASSERT_EQUAL(errno_null, EFAULT);
	} else {
		/* VIDIOC_G_TUNER not supported at all, the parameter should not be evaluated */
		CU_ASSERT_EQUAL(ret_get, -1);
		CU_ASSERT_EQUAL(errno_get, EINVAL);
		CU_ASSERT_EQUAL(ret_null, -1);
		CU_ASSERT_EQUAL(errno_null, EINVAL);
	}

}

void do_set_tuner_audmode(__u32 index, __u32 audmode)
{
	int ret_set, errno_set;
	struct v4l2_tuner tuner;

	memset(&tuner, 0xff, sizeof(tuner));
	tuner.index = index;
	tuner.audmode = audmode;
	ret_set = ioctl(get_video_fd(), VIDIOC_S_TUNER, &tuner);
	errno_set = errno;

	dprintf("\t%s:%u: VIDIOC_S_TUNER: index=%u, audmode=%u, "
		"ret_set=%i (expected %i), errno_set=%i\n",
		__FILE__, __LINE__, index, audmode, ret_set, 0, errno_set);

	CU_ASSERT_EQUAL(ret_set, 0);

}

void do_set_tuner_audmode_invalid(__u32 index, __u32 audmode)
{
	int ret_set, errno_set;
	struct v4l2_tuner tuner;

	memset(&tuner, 0xff, sizeof(tuner));
	tuner.index = index;
	tuner.audmode = audmode;
	ret_set = ioctl(get_video_fd(), VIDIOC_S_TUNER, &tuner);
	errno_set = errno;

	dprintf("\t%s:%u: VIDIOC_S_TUNER: index=%u, audmode=%u, "
		"ret_set=%i (expected %i), errno_set=%i (expected %i)\n",
		__FILE__, __LINE__,
		index, audmode, ret_set, -1, errno_set, EINVAL);

	CU_ASSERT_EQUAL(ret_set, -1);
	CU_ASSERT_EQUAL(errno_set, EINVAL);
}

void test_VIDIOC_S_TUNER()
{
	int ret_get, errno_get;
	int ret_set, errno_set;
	struct v4l2_tuner tuner_orig;
	struct v4l2_tuner tuner_set;

	/* remember the tuner settings */
	memset(&tuner_orig, 0, sizeof(tuner_orig));
	ret_get = ioctl(get_video_fd(), VIDIOC_G_TUNER, &tuner_orig);
	errno_get = errno;

	dprintf("\t%s:%u: VIDIOC_G_TUNER, ret_get=%i, errno_get=%i\n",
		__FILE__, __LINE__, ret_get, errno_get);

	if (ret_get == 0) {
		CU_ASSERT_EQUAL(ret_get, 0);

		do_set_tuner_audmode(tuner_orig.index, V4L2_TUNER_MODE_MONO);
		do_set_tuner_audmode(tuner_orig.index, V4L2_TUNER_MODE_STEREO);
		do_set_tuner_audmode(tuner_orig.index, V4L2_TUNER_MODE_LANG1);
		do_set_tuner_audmode(tuner_orig.index, V4L2_TUNER_MODE_LANG2);
		do_set_tuner_audmode(tuner_orig.index, V4L2_TUNER_MODE_SAP);
		do_set_tuner_audmode(tuner_orig.index,
				     V4L2_TUNER_MODE_LANG1_LANG2);

	} else {
		CU_ASSERT_EQUAL(ret_get, -1);
		CU_ASSERT_EQUAL(errno_get, EINVAL);

		/* if VIDIOC_G_TUNER is not supported then VIDIOC_S_TUNER shall also
		 * not supported.
		 */
		do_set_tuner_audmode_invalid(tuner_orig.index,
					     V4L2_TUNER_MODE_MONO);
		do_set_tuner_audmode_invalid(tuner_orig.index,
					     V4L2_TUNER_MODE_STEREO);
		do_set_tuner_audmode_invalid(tuner_orig.index,
					     V4L2_TUNER_MODE_LANG1);
		do_set_tuner_audmode_invalid(tuner_orig.index,
					     V4L2_TUNER_MODE_LANG2);
		do_set_tuner_audmode_invalid(tuner_orig.index,
					     V4L2_TUNER_MODE_SAP);
		do_set_tuner_audmode_invalid(tuner_orig.index,
					     V4L2_TUNER_MODE_LANG1_LANG2);

	}

	if (ret_get == 0) {

		/* restore the tuner settings */
		memset(&tuner_set, 0xff, sizeof(tuner_set));
		tuner_set.index = tuner_orig.index;
		tuner_set.audmode = tuner_orig.audmode;
		ret_set = ioctl(get_video_fd(), VIDIOC_S_TUNER, &tuner_orig);
		errno_set = errno;

		dprintf("\t%s:%u: VIDIOC_S_TUNER, ret_set=%i, errno_set=%i\n",
			__FILE__, __LINE__, ret_set, errno_set);

		CU_ASSERT_EQUAL(ret_set, 0);
	}

}

void test_VIDIOC_S_TUNER_invalid()
{
	int ret1, errno1;
	int ret_set, errno_set;
	struct v4l2_tuner tuner_orig;
	struct v4l2_tuner tuner_set;

	/* remember the tuner settings */
	memset(&tuner_orig, 0, sizeof(tuner_orig));
	ret1 = ioctl(get_video_fd(), VIDIOC_G_TUNER, &tuner_orig);
	errno1 = errno;

	dprintf("\t%s:%u: VIDIOC_G_TUNER, ret1=%i, errno1=%i\n",
		__FILE__, __LINE__, ret1, errno1);

	if (ret1 == 0) {
		CU_ASSERT_EQUAL(ret1, 0);

		/* try with invalid index */
		do_set_tuner_audmode_invalid(tuner_orig.index + 1,
					     V4L2_TUNER_MODE_MONO);
		do_set_tuner_audmode_invalid(tuner_orig.index - 1,
					     V4L2_TUNER_MODE_MONO);
		do_set_tuner_audmode_invalid((__u32) S32_MAX,
					     V4L2_TUNER_MODE_MONO);
		do_set_tuner_audmode_invalid(((__u32) S32_MAX) + 1,
					     V4L2_TUNER_MODE_MONO);
		do_set_tuner_audmode_invalid(U32_MAX, V4L2_TUNER_MODE_MONO);

		do_set_tuner_audmode_invalid(tuner_orig.index + 1,
					     V4L2_TUNER_MODE_STEREO);
		do_set_tuner_audmode_invalid(tuner_orig.index - 1,
					     V4L2_TUNER_MODE_STEREO);
		do_set_tuner_audmode_invalid((__u32) S32_MAX,
					     V4L2_TUNER_MODE_STEREO);
		do_set_tuner_audmode_invalid(((__u32) S32_MAX) + 1,
					     V4L2_TUNER_MODE_STEREO);
		do_set_tuner_audmode_invalid(U32_MAX, V4L2_TUNER_MODE_STEREO);

		do_set_tuner_audmode_invalid(tuner_orig.index + 1,
					     V4L2_TUNER_MODE_LANG1);
		do_set_tuner_audmode_invalid(tuner_orig.index - 1,
					     V4L2_TUNER_MODE_LANG1);
		do_set_tuner_audmode_invalid((__u32) S32_MAX,
					     V4L2_TUNER_MODE_LANG1);
		do_set_tuner_audmode_invalid(((__u32) S32_MAX) + 1,
					     V4L2_TUNER_MODE_LANG1);
		do_set_tuner_audmode_invalid(U32_MAX, V4L2_TUNER_MODE_LANG1);

		do_set_tuner_audmode_invalid(tuner_orig.index + 1,
					     V4L2_TUNER_MODE_LANG2);
		do_set_tuner_audmode_invalid(tuner_orig.index - 1,
					     V4L2_TUNER_MODE_LANG2);
		do_set_tuner_audmode_invalid((__u32) S32_MAX,
					     V4L2_TUNER_MODE_LANG2);
		do_set_tuner_audmode_invalid(((__u32) S32_MAX) + 1,
					     V4L2_TUNER_MODE_LANG2);
		do_set_tuner_audmode_invalid(U32_MAX, V4L2_TUNER_MODE_LANG2);

		do_set_tuner_audmode_invalid(tuner_orig.index + 1,
					     V4L2_TUNER_MODE_SAP);
		do_set_tuner_audmode_invalid(tuner_orig.index - 1,
					     V4L2_TUNER_MODE_SAP);
		do_set_tuner_audmode_invalid((__u32) S32_MAX,
					     V4L2_TUNER_MODE_SAP);
		do_set_tuner_audmode_invalid(((__u32) S32_MAX) + 1,
					     V4L2_TUNER_MODE_SAP);
		do_set_tuner_audmode_invalid(U32_MAX, V4L2_TUNER_MODE_SAP);

		do_set_tuner_audmode_invalid(tuner_orig.index + 1,
					     V4L2_TUNER_MODE_LANG1_LANG2);
		do_set_tuner_audmode_invalid(tuner_orig.index - 1,
					     V4L2_TUNER_MODE_LANG1_LANG2);
		do_set_tuner_audmode_invalid((__u32) S32_MAX,
					     V4L2_TUNER_MODE_LANG1_LANG2);
		do_set_tuner_audmode_invalid(((__u32) S32_MAX) + 1,
					     V4L2_TUNER_MODE_LANG1_LANG2);
		do_set_tuner_audmode_invalid(U32_MAX,
					     V4L2_TUNER_MODE_LANG1_LANG2);

	} else {
		CU_ASSERT_EQUAL(ret1, -1);
		CU_ASSERT_EQUAL(errno1, EINVAL);

	}

	/* try with invalid audmode */
	do_set_tuner_audmode_invalid(tuner_orig.index, 5);
	do_set_tuner_audmode_invalid(tuner_orig.index, (__u32) S32_MAX);
	do_set_tuner_audmode_invalid(tuner_orig.index, ((__u32) S32_MAX) + 1);
	do_set_tuner_audmode_invalid(tuner_orig.index, U32_MAX);

	if (ret1 == 0) {

		/* restore the tuner settings */
		memset(&tuner_set, 0xff, sizeof(tuner_set));
		tuner_set.index = tuner_orig.index;
		tuner_set.audmode = tuner_orig.audmode;
		ret_set = ioctl(get_video_fd(), VIDIOC_S_TUNER, &tuner_orig);
		errno_set = errno;

		dprintf("\t%s:%u: VIDIOC_S_TUNER, ret_set=%i, errno_set=%i\n",
			__FILE__, __LINE__, ret_set, errno_set);

		CU_ASSERT_EQUAL(ret_set, 0);
	}

}

void test_VIDIOC_S_TUNER_NULL()
{
	int ret_orig, errno_orig;
	int ret_null, errno_null;
	int ret_set, errno_set;
	struct v4l2_tuner tuner_orig;
	struct v4l2_tuner tuner;

	/* remember the tuner settings */
	memset(&tuner_orig, 0, sizeof(tuner_orig));
	ret_orig = ioctl(get_video_fd(), VIDIOC_G_TUNER, &tuner_orig);
	errno_orig = errno;

	dprintf("\t%s:%u: VIDIOC_G_TUNER, ret_orig=%i, errno_orig=%i\n",
		__FILE__, __LINE__, ret_orig, errno_orig);

	memset(&tuner, 0, sizeof(tuner));
	tuner.index = tuner_orig.index;
	tuner.audmode = tuner_orig.audmode;
	ret_set = ioctl(get_video_fd(), VIDIOC_S_TUNER, &tuner);
	errno_set = errno;

	dprintf("\t%s:%u:VIDIOC_S_TUNER: ret_set=%i, errno_set=%i\n",
		__FILE__, __LINE__, ret_set, errno_set);

	ret_null = ioctl(get_video_fd(), VIDIOC_S_TUNER, NULL);
	errno_null = errno;

	dprintf("\t%s:%u:VIDIOC_S_TUNER: ret_null=%i, errno_null=%i\n",
		__FILE__, __LINE__, ret_set, errno_set);

	if (ret_set == 0) {
		CU_ASSERT_EQUAL(ret_set, 0);
		CU_ASSERT_EQUAL(ret_null, -1);
		CU_ASSERT_EQUAL(errno_null, EFAULT);
	} else {
		CU_ASSERT_EQUAL(ret_set, -1);
		CU_ASSERT_EQUAL(errno_set, EINVAL);
		CU_ASSERT_EQUAL(ret_null, -1);
		CU_ASSERT_EQUAL(errno_null, EINVAL);
	}

}

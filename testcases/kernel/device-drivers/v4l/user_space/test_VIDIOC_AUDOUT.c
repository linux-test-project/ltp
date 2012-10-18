/*
 * v4l-test: Test environment for Video For Linux Two API
 *
 * 20 Apr 2009  0.7  Added string content validation
 * 18 Apr 2009  0.6  More strict check for strings
 * 29 Mar 2009  0.5  Clean up test case for NULL parameter
 * 22 Mar 2009  0.4  Cleaned up dprintf() messages
 *  9 Feb 2009  0.3  Typo corrected; added some debug messages
 *  7 Feb 2009  0.2  Test case test_VIDIOC_G_AUDOUT_ignore_index added
 *  3 Feb 2009  0.1  First release
 *
 * Written by M�rton N�meth <nm127@freemail.hu>
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
#include "v4l2_validator.h"

#include "test_VIDIOC_AUDOUT.h"

int valid_audioout_mode(__u32 mode) {
	int valid = 0;

	if ((mode & ~(V4L2_AUDMODE_AVL))
		== 0) {
		valid = 1;
	} else {
		valid = 0;
	}
	return valid;
}

void test_VIDIOC_G_AUDOUT() {
	int ret_get, errno_get;
	struct v4l2_audioout audioout;
	struct v4l2_audioout audioout2;

	memset(&audioout, 0xff, sizeof(audioout));
	ret_get = ioctl(get_video_fd(), VIDIOC_G_AUDOUT, &audioout);
	errno_get = errno;

	dprintf("\tVIDIOC_AUDIOOUT, ret_get=%i, errno_get=%i\n", ret_get, errno_get);

	if (ret_get == 0) {
		CU_ASSERT_EQUAL(ret_get, 0);

		//CU_ASSERT_EQUAL(audioout.index, ?);

		CU_ASSERT(0 < strlen( (char*)audioout.name ));
		CU_ASSERT(valid_string((char*)audioout.name, sizeof(audioout.name)));

		CU_ASSERT_EQUAL(audioout.capability, 0);
		CU_ASSERT_EQUAL(audioout.mode, 0);

		CU_ASSERT_EQUAL(audioout.reserved[0], 0);
		CU_ASSERT_EQUAL(audioout.reserved[1], 0);

		/* Check if the unused bytes of the name string is also filled
		 * with zeros. Also check if there is any padding byte between
		 * any two fields then this padding byte is also filled with zeros.
		 */
		memset(&audioout2, 0, sizeof(audioout2));
		audioout2.index = audioout.index;
		strncpy((char*)audioout2.name, (char*)audioout.name, sizeof(audioout2.name));
		audioout2.capability = audioout.capability;
		audioout2.mode = audioout.mode;
		CU_ASSERT_EQUAL(memcmp(&audioout, &audioout2, sizeof(audioout)), 0);

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
		CU_ASSERT_EQUAL(ret_get, -1);
		CU_ASSERT_EQUAL(errno_get, EINVAL);

		/* check if the audioout structure is untouched */
		memset(&audioout2, 0xff, sizeof(audioout2));
		CU_ASSERT_EQUAL(memcmp(&audioout, &audioout2, sizeof(audioout)), 0);

	}

}

void test_VIDIOC_G_AUDOUT_ignore_index() {
	int reg_get1, errno1;
	int reg_get2, errno2;
	struct v4l2_audioout audioout;
	struct v4l2_audioout audioout2;

	/* check whether the "index" field is ignored by VIDIOC_G_AUDOUT */

	memset(&audioout, 0, sizeof(audioout));
	reg_get1 = ioctl(get_video_fd(), VIDIOC_G_AUDOUT, &audioout);
	errno1 = errno;

	dprintf("\tVIDIOC_G_AUDOUT, reg_get1=%i, errno1=%i\n", reg_get1, errno1);

	memset(&audioout2, 0, sizeof(audioout2));
	audioout2.index = U32_MAX;
	reg_get2 = ioctl(get_video_fd(), VIDIOC_G_AUDOUT, &audioout2);
	errno2 = errno;

	dprintf("\tVIDIOC_G_AUDOUT, reg_get2=%i, errno2=%i\n", reg_get2, errno2);

	if (reg_get1 == 0) {
		CU_ASSERT_EQUAL(reg_get2, 0);
		CU_ASSERT_EQUAL(memcmp(&audioout, &audioout2, sizeof(audioout)), 0);
	} else {
		CU_ASSERT_EQUAL(reg_get1, -1);
		CU_ASSERT_EQUAL(errno1, EINVAL);
		CU_ASSERT_EQUAL(reg_get2, -1);
		CU_ASSERT_EQUAL(errno2, EINVAL);
	}

}

void test_VIDIOC_G_AUDOUT_NULL() {
	int ret_get, errno_get;
	int ret_null, errno_null;
	struct v4l2_audioout audioout;

	memset(&audioout, 0xff, sizeof(audioout));
	ret_get = ioctl(get_video_fd(), VIDIOC_G_AUDOUT, &audioout);
	errno_get = errno;

	dprintf("\t%s:%u: VIDIOC_AUDIOOUT, ret_get=%i, errno_get=%i\n",
		__FILE__, __LINE__, ret_get, errno_get);

	ret_null = ioctl(get_video_fd(), VIDIOC_G_AUDOUT, NULL);
	errno_null = errno;

	dprintf("\t%s:%u: VIDIOC_AUDIOOUT, ret_null=%i, errno_null=%i\n",
		__FILE__, __LINE__, ret_null, errno_null);

	/* check if VIDIOC_G_AUDOUT is supported at all or not */
	if (ret_get == 0) {
		/* VIDIOC_G_AUDOUT is supported, the parameter should be checked */
		CU_ASSERT_EQUAL(ret_null, -1);
		CU_ASSERT_EQUAL(errno_null, EFAULT);
	} else {
		/* VIDIOC_G_AUDOUT not supported at all, the parameter should not be evaluated */
		CU_ASSERT_EQUAL(ret_null, -1);
		CU_ASSERT_EQUAL(errno_null, EINVAL);
	}

}

/* TODO: - try with all possible outputs (VIDIOC_ENUMOUTPUT)
 *       - try with STREAM_ON
 */

void test_VIDIOC_S_AUDOUT() {
	int ret_orig, errno_orig;
	int ret_enum, errno_enum;
	int ret_set, errno_set;
	__u32 index;
	__u32 i;
	struct v4l2_audioout audioout_orig;
	struct v4l2_audioout audioout_enum;
	struct v4l2_audioout audioout_set;

	/* This testcase tries to find out the relations between the following
	 * commands:
	 *  - VIDIOC_ENUMAUDOUT
	 *  - VIDIOC_G_AUDOUT
	 *  - VIDIOC_S_AUDOUT
	 */

	/* remember the original settings */
	memset(&audioout_orig, 0, sizeof(audioout_orig));
	ret_orig = ioctl(get_video_fd(), VIDIOC_G_AUDOUT, &audioout_orig);
	errno_orig = errno;

	dprintf("\tVIDIOC_G_AUDOUT, ret_orig=%i, errno_orig=%i\n", ret_orig, errno_orig);

	if (ret_orig == 0) {
		CU_ASSERT_EQUAL(ret_orig, 0);
	} else {
		CU_ASSERT_EQUAL(ret_orig, -1);
		CU_ASSERT_EQUAL(errno_orig, EINVAL);
	}

	/* try to continue even if VIDIOC_G_AUDOUT seems to be not supported */

	index = 0;
	do {
		memset(&audioout_enum, 0, sizeof(audioout_enum));
		audioout_enum.index = index;
		ret_enum = ioctl(get_video_fd(), VIDIOC_ENUMAUDOUT, &audioout_enum);
		errno_enum = errno;

		if (ret_enum == 0) {
			memset(&audioout_set, 0xff, sizeof(audioout_set));
			audioout_set.index = index;
			ret_set = ioctl(get_video_fd(), VIDIOC_S_AUDOUT, &audioout_set);
			errno_set = errno;

			/* It shall be always possible to set the audio output to the
			 * enumerated values.
			 */
			CU_ASSERT_EQUAL(ret_set, 0);

			index++;
		}

	} while (ret_enum == 0);
	CU_ASSERT_EQUAL(ret_enum, -1);
	CU_ASSERT_EQUAL(errno_enum, EINVAL);

	/* try to set audio output to beyond the enumerated values */
	for (i=0; i<=32; i++) {
		memset(&audioout_set, 0xff, sizeof(audioout_set));
		audioout_set.index = index;
		ret_set = ioctl(get_video_fd(), VIDIOC_S_AUDOUT, &audioout_set);
		errno_set = errno;

		CU_ASSERT_EQUAL(ret_set, -1);
		CU_ASSERT_EQUAL(errno_set, EINVAL);

		index++;
	}

	/* restore the original audio output settings */
	memset(&audioout_set, 0, sizeof(audioout_set));
	audioout_set.index = audioout_orig.index;
	ret_set = ioctl(get_video_fd(), VIDIOC_S_AUDOUT, &audioout_set);
	errno_set = errno;

	if (ret_orig == 0) {
		/* If it was possible at the beginning to get the audio output then
		 * it shall be possible to set it again.
		 */
		CU_ASSERT_EQUAL(ret_set, 0);
	} else {
		/* In case we could not fetch the audio output value at the start
		 * of this test case: the VIDIOC_S_AUDOUT shall also fail.
		 */
		CU_ASSERT_EQUAL(ret_set, -1);
		CU_ASSERT_EQUAL(errno_set, EINVAL);
	}

}

void test_VIDIOC_S_AUDOUT_S32_MAX() {
	int ret_orig, errno_orig;
	int ret_set, errno_set;
	struct v4l2_audioout audioout;
	struct v4l2_audioout audioout2;
	struct v4l2_audioout audioout_orig;
	struct v4l2_audioout audioout_set;

	/* remember the original settings */
	memset(&audioout_orig, 0, sizeof(audioout_orig));
	ret_orig = ioctl(get_video_fd(), VIDIOC_G_AUDOUT, &audioout_orig);
	errno_orig = errno;

	dprintf("\tVIDIOC_G_AUDOUT, ret_orig=%i, errno_orig=%i\n", ret_orig, errno_orig);

	/* test invalid index */
	memset(&audioout, 0xff, sizeof(audioout));
	audioout.index = (__u32)S32_MAX;
	ret_set = ioctl(get_video_fd(), VIDIOC_S_AUDOUT, &audioout);
	errno_set = errno;

	CU_ASSERT_EQUAL(ret_set, -1);
	CU_ASSERT_EQUAL(errno_set, EINVAL);

	/* Check whether the original audioout struct is untouched */
	memset(&audioout2, 0xff, sizeof(audioout2));
	audioout2.index = (__u32)S32_MAX;
	CU_ASSERT_EQUAL(memcmp(&audioout, &audioout2, sizeof(audioout)), 0);

	/* restore the original audio output settings */
	memset(&audioout_set, 0, sizeof(audioout_set));
	audioout_set.index = audioout_orig.index;
	ret_set = ioctl(get_video_fd(), VIDIOC_S_AUDOUT, &audioout_set);
	errno_set = errno;

	dprintf("\tVIDIOC_S_AUDOUT, ret_set=%i, errno_set=%i\n", ret_set, errno_set);

	if (ret_orig == 0) {
		/* If it was possible at the beginning to get the audio output then
		 * it shall be possible to set it again.
		 */
		CU_ASSERT_EQUAL(ret_set, 0);
	} else {
		/* In case we could not fetch the audio output value at the start
		 * of this test case: the VIDIOC_S_AUDOUT shall also fail.
		 */
		CU_ASSERT_EQUAL(ret_set, -1);
		CU_ASSERT_EQUAL(errno_set, EINVAL);
	}
}

void test_VIDIOC_S_AUDOUT_S32_MAX_1() {
	int ret_orig, errno_orig;
	int ret_set, errno_set;
	struct v4l2_audioout audioout;
	struct v4l2_audioout audioout2;
	struct v4l2_audioout audioout_orig;
	struct v4l2_audioout audioout_set;

	/* remember the original settings */
	memset(&audioout_orig, 0, sizeof(audioout_orig));
	ret_orig = ioctl(get_video_fd(), VIDIOC_G_AUDOUT, &audioout_orig);
	errno_orig = errno;

	dprintf("\tVIDIOC_G_AUDOUT, ret_orig=%i, errno_orig=%i\n", ret_orig, errno_orig);

	/* test invalid index */
	memset(&audioout, 0xff, sizeof(audioout));
	audioout.index = ((__u32)S32_MAX)+1;
	ret_set = ioctl(get_video_fd(), VIDIOC_S_AUDOUT, &audioout);
	errno_set = errno;

	CU_ASSERT_EQUAL(ret_set, -1);
	CU_ASSERT_EQUAL(errno_set, EINVAL);

	/* Check whether the original audioout struct is untouched */
	memset(&audioout2, 0xff, sizeof(audioout2));
	audioout2.index = ((__u32)S32_MAX)+1;
	CU_ASSERT_EQUAL(memcmp(&audioout, &audioout2, sizeof(audioout)), 0);

	/* restore the original audio output settings */
	memset(&audioout_set, 0, sizeof(audioout_set));
	audioout_set.index = audioout_orig.index;
	ret_set = ioctl(get_video_fd(), VIDIOC_S_AUDOUT, &audioout_set);
	errno_set = errno;

	dprintf("\tVIDIOC_S_AUDOUT, ret_set=%i, errno_set=%i\n", ret_set, errno_set);

	if (ret_orig == 0) {
		/* If it was possible at the beginning to get the audio output then
		 * it shall be possible to set it again.
		 */
		CU_ASSERT_EQUAL(ret_set, 0);
	} else {
		/* In case we could not fetch the audio output value at the start
		 * of this test case: the VIDIOC_S_AUDOUT shall also fail.
		 */
		CU_ASSERT_EQUAL(ret_set, -1);
		CU_ASSERT_EQUAL(errno_set, EINVAL);
	}
}

void test_VIDIOC_S_AUDOUT_U32_MAX() {
	int ret_orig, errno_orig;
	int ret_set, errno_set;
	struct v4l2_audioout audioout;
	struct v4l2_audioout audioout2;
	struct v4l2_audioout audioout_orig;
	struct v4l2_audioout audioout_set;

	/* remember the original settings */
	memset(&audioout_orig, 0, sizeof(audioout_orig));
	ret_orig = ioctl(get_video_fd(), VIDIOC_G_AUDOUT, &audioout_orig);
	errno_orig = errno;

	dprintf("\tVIDIOC_G_AUDOUT, ret_orig=%i, errno_orig=%i\n", ret_orig, errno_orig);
	/* test invalid index */
	memset(&audioout, 0xff, sizeof(audioout));
	audioout.index = U32_MAX;
	ret_set = ioctl(get_video_fd(), VIDIOC_S_AUDOUT, &audioout);
	errno_set = errno;

	CU_ASSERT_EQUAL(ret_set, -1);
	CU_ASSERT_EQUAL(errno_set, EINVAL);

	/* Check whether the original audioout struct is untouched */
	memset(&audioout2, 0xff, sizeof(audioout2));
	audioout2.index = U32_MAX;
	CU_ASSERT_EQUAL(memcmp(&audioout, &audioout2, sizeof(audioout)), 0);

	/* restore the original audio output settings */
	memset(&audioout_set, 0, sizeof(audioout_set));
	audioout_set.index = audioout_orig.index;
	ret_set = ioctl(get_video_fd(), VIDIOC_S_AUDOUT, &audioout_set);
	errno_set = errno;

	dprintf("\tVIDIOC_S_AUDOUT, ret_set=%i, errno_set=%i\n", ret_set, errno_set);

	if (ret_orig == 0) {
		/* If it was possible at the beginning to get the audio output then
		 * it shall be possible to set it again.
		 */
		CU_ASSERT_EQUAL(ret_set, 0);
	} else {
		/* In case we could not fetch the audio output value at the start
		 * of this test case: the VIDIOC_S_AUDOUT shall also fail.
		 */
		CU_ASSERT_EQUAL(ret_set, -1);
		CU_ASSERT_EQUAL(errno_set, EINVAL);
	}
}

void test_VIDIOC_S_AUDOUT_NULL() {
	int ret_orig, errno_orig;
	int ret_set, errno_set;
	int ret_get, errno_get;
	struct v4l2_audio audio_orig;
	struct v4l2_audio audio_set;

	/* remember the original settings */
	memset(&audio_orig, 0, sizeof(audio_orig));
	ret_orig = ioctl(get_video_fd(), VIDIOC_G_AUDOUT, &audio_orig);
	errno_orig = errno;

	dprintf("\tVIDIOC_G_AUDOUT, ret_orig=%i, errno_orig=%i\n", ret_orig, errno_orig);

	memset(&audio_set, 0, sizeof(audio_set));
	ret_get = ioctl(get_video_fd(), VIDIOC_S_AUDOUT, &audio_set);
	errno_get = errno;

	dprintf("\tVIDIOC_S_AUDOUT, ret_get=%i, errno_get=%i\n", ret_get, errno_get);

	ret_set = ioctl(get_video_fd(), VIDIOC_S_AUDOUT, NULL);
	errno_set = errno;

	dprintf("\tVIDIOC_S_AUDOUT, ret_set=%i, errno_set=%i\n", ret_set, errno_set);

	if (ret_get == 0) {
		CU_ASSERT_EQUAL(ret_get, 0);
		CU_ASSERT_EQUAL(ret_set, -1);
		CU_ASSERT_EQUAL(errno_set, EFAULT);
	} else {
		CU_ASSERT_EQUAL(ret_get, -1);
		CU_ASSERT_EQUAL(errno_get, EINVAL);
		CU_ASSERT_EQUAL(ret_set, -1);
		CU_ASSERT_EQUAL(errno_set, EINVAL);
	}

	/* restore the original audio input settings */
	memset(&audio_set, 0, sizeof(audio_set));
	audio_set.index = audio_orig.index;
	ret_set = ioctl(get_video_fd(), VIDIOC_S_AUDOUT, &audio_set);
	errno_set = errno;

	dprintf("\tVIDIOC_S_AUDOUT, ret_set=%i, errno_set=%i\n", ret_set, errno_set);

	if (ret_orig == 0) {
		/* If it was possible at the beginning to get the audio input then
		 * it shall be possible to set it again.
		 */
		CU_ASSERT_EQUAL(ret_orig, 0);
		CU_ASSERT_EQUAL(ret_set, 0);
	} else {
		/* In case we could not fetch the audio input value at the start
		 * of this test case: the VIDIOC_S_AUDOUT shall also fail.
		 */
		CU_ASSERT_EQUAL(ret_orig, -1);
		CU_ASSERT_EQUAL(errno_orig, EINVAL);
		CU_ASSERT_EQUAL(ret_set, -1);
		CU_ASSERT_EQUAL(errno_set, EINVAL);
	}
}

/*
 * v4l-test: Test environment for Video For Linux Two API
 *
 *  9 Feb 2009  0.3  Typo corrected; added some debug messages
 *  7 Feb 2009  0.2  Test case test_VIDIOC_G_AUDIO_ignore_index added
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

#include "test_VIDIOC_ENUMAUDIO.h"

int valid_audio_capability(__u32 capability) {
	int valid = 0;

	if ( (capability & ~(V4L2_AUDCAP_STEREO |
			     V4L2_AUDCAP_AVL))
		== 0) {
		valid = 1;
	} else {
		valid = 0;
	}
	return valid;
}

int valid_audio_mode(__u32 mode) {
	int valid = 0;

	if ( (mode & ~(V4L2_AUDMODE_AVL))
		== 0) {
		valid = 1;
	} else {
		valid = 0;
	}
	return valid;
}

void test_VIDIOC_G_AUDIO() {
	int ret;
	struct v4l2_audio audio;
	struct v4l2_audio audio2;

	memset(&audio, 0xff, sizeof(audio));
	ret = ioctl(get_video_fd(), VIDIOC_G_AUDIO, &audio);

	dprintf("VIDIOC_G_AUDIO, ret=%i\n", ret);

	if (ret == 0) {
		CU_ASSERT_EQUAL(ret, 0);

		//CU_ASSERT_EQUAL(audio.index, ?);

		//CU_ASSERT_EQUAL(audio.name, ?);
		CU_ASSERT(0 < strlen( (char*)audio.name ));
		CU_ASSERT(strlen( (char*)audio.name ) < sizeof(audio.name));

		CU_ASSERT(valid_audio_capability(audio.capability));
		CU_ASSERT(valid_audio_mode(audio.mode));

		CU_ASSERT_EQUAL(audio.reserved[0], 0);
		CU_ASSERT_EQUAL(audio.reserved[1], 0);

		dprintf("\taudio = {.index=%u, .name=\"%s\", "
			".capability=0x%X, .mode=0x%X, "
			".reserved[]={ 0x%X, 0x%X } }\n",
			audio.index,
			audio.name,
			audio.capability,
			audio.mode,
			audio.reserved[0],
			audio.reserved[1]
			);

	} else {
		CU_ASSERT_EQUAL(ret, -1);
		CU_ASSERT_EQUAL(errno, EINVAL);

		/* check if the audio structure is untouched */
		memset(&audio2, 0xff, sizeof(audio2));
		CU_ASSERT_EQUAL(memcmp(&audio, &audio2, sizeof(audio)), 0);

		dprintf("\terrno=%i\n", errno);

	}

}

void test_VIDIOC_G_AUDIO_ignore_index() {
	int ret1;
	int errno1;
	int ret2;
	int errno2;
	struct v4l2_audio audio;
	struct v4l2_audio audio2;

	/* check whether the "index" field is ignored by VIDIOC_G_AUDIO */

	memset(&audio, 0, sizeof(audio));
	dprintf("&audio=%p\n", &audio);
	dprintf("audio.index=%u\n", audio.index);
	ret1 = ioctl(get_video_fd(), VIDIOC_G_AUDIO, &audio);
	errno1 = errno;

	dprintf("VIDIOC_G_AUDIO, ret1=%i, errno1=%i\n", ret1, errno1);

	memset(&audio2, 0, sizeof(audio2));
	audio2.index = U32_MAX;
	ret2 = ioctl(get_video_fd(), VIDIOC_G_AUDIO, &audio2);
	errno2 = errno;

	dprintf("VIDIOC_G_AUDIO, ret2=%i, errno2=%i\n", ret2, errno2);

	if (ret1 == 0) {
		CU_ASSERT_EQUAL(ret2, 0);
		CU_ASSERT_EQUAL(memcmp(&audio, &audio2, sizeof(audio)), 0);
	} else {
		CU_ASSERT_EQUAL(ret1, -1);
		CU_ASSERT_EQUAL(errno1, EINVAL);
		CU_ASSERT_EQUAL(ret2, -1);
		CU_ASSERT_EQUAL(errno2, EINVAL);
	}

}

void test_VIDIOC_G_AUDIO_NULL() {
	int ret1;
	int errno1;
	int ret2;
	int errno2;
	struct v4l2_audio audio;

	memset(&audio, 0xff, sizeof(audio));
	ret1 = ioctl(get_video_fd(), VIDIOC_G_AUDIO, &audio);
	errno1 = errno;

	dprintf("VIDIOC_AUDIO, ret1=%i\n", ret1);

	ret2 = ioctl(get_video_fd(), VIDIOC_G_AUDIO, NULL);
	errno2 = errno;

	/* check if VIDIOC_G_AUDIO is supported at all or not */
	if (ret1 == -1 && errno1 == EINVAL) {
		/* VIDIOC_G_AUDIO not supported at all, the parameter should not be evaluated */
		dprintf("\t%s:%u: ret2=%d (expected %d)\n", __FILE__, __LINE__, ret2, -1);
		dprintf("\t%s:%u: errno2=%d (expected %d)\n", __FILE__, __LINE__, errno2, EINVAL);
		CU_ASSERT_EQUAL(ret2, -1);
		CU_ASSERT_EQUAL(errno, EINVAL);

	} else {
		/* VIDIOC_G_AUDIO is supported, the parameter should be checked */
		dprintf("\t%s:%u: ret2=%d (expected %d)\n", __FILE__, __LINE__, ret2, -1);
		dprintf("\t%s:%u: errno2=%d (expected %d)\n", __FILE__, __LINE__, errno2, EFAULT);
		CU_ASSERT_EQUAL(ret2, -1);
		CU_ASSERT_EQUAL(errno2, EFAULT);
	}

}

void test_VIDIOC_S_AUDIO() {
	int ret_orig, errno_orig;
	int ret;
	__u32 index;
	__u32 i;
	struct v4l2_audio audio_orig;
	struct v4l2_audio audio_enum;
	struct v4l2_audio audio_set;

	/* This testcase tries to find out the relations between the following
	 * commands:
	 *  - VIDIOC_ENUMAUDIO
	 *  - VIDIOC_G_AUDIO
	 *  - VIDIOC_S_AUDIO
	 */

	/* remember the original settings */
	memset(&audio_orig, 0, sizeof(audio_orig));
	ret_orig = ioctl(get_video_fd(), VIDIOC_G_AUDIO, &audio_orig);
	errno_orig = errno;

	dprintf("VIDIOC_G_AUDIO, ret_orig=%i, errno_orig=%i\n", ret_orig, errno_orig);

	if (ret_orig == 0) {
		CU_ASSERT_EQUAL(ret_orig, 0);
	} else {
		CU_ASSERT_EQUAL(ret_orig, -1);
		CU_ASSERT_EQUAL(errno_orig, EINVAL);
	}

	/* try to continue even if VIDIOC_G_AUDIO seems to be not supported */

	index = 0;
	do {
		memset(&audio_enum, 0, sizeof(audio_enum));
		audio_enum.index = index;
		ret = ioctl(get_video_fd(), VIDIOC_ENUMAUDIO, &audio_enum);

		if (ret == 0) {
			memset(&audio_set, 0xff, sizeof(audio_set));
			audio_set.index = index;
			ret = ioctl(get_video_fd(), VIDIOC_S_AUDIO, &audio_set);

			/* It shall be always possible to set the audio input to the
			 * enumerated values.
			 */
			CU_ASSERT_EQUAL(ret, 0);

			index++;
		}

	} while (ret == 0);
	CU_ASSERT_EQUAL(ret, -1);
	CU_ASSERT_EQUAL(errno, EINVAL);

	/* try to set audio input to beyond the enumerated values */
	for (i=0; i<=32; i++) {
		memset(&audio_set, 0xff, sizeof(audio_set));
		audio_set.index = index;
		ret = ioctl(get_video_fd(), VIDIOC_S_AUDIO, &audio_set);

		CU_ASSERT_EQUAL(ret, -1);
		CU_ASSERT_EQUAL(errno, EINVAL);

		index++;
	}

	/* restore the original audio input settings */
	memset(&audio_set, 0, sizeof(audio_set));
	audio_set.index = audio_orig.index;
	ret = ioctl(get_video_fd(), VIDIOC_S_AUDIO, &audio_set);

	if (ret_orig == 0) {
		/* If it was possible at the beginning to get the audio input then
		 * it shall be possible to set it again.
		 */
		CU_ASSERT_EQUAL(ret, 0);
	} else {
		/* In case we could not fetch the audio input value at the start
		 * of this test case: the VIDIOC_S_AUDIO shall also fail.
		 */
		CU_ASSERT_EQUAL(ret, -1);
		CU_ASSERT_EQUAL(errno, EINVAL);
	}

}

void test_VIDIOC_S_AUDIO_S32_MAX() {
	int ret;
	int ret_orig, errno_orig;
	int ret_set, errno_set;
	struct v4l2_audio audio;
	struct v4l2_audio audio2;
	struct v4l2_audio audio_orig;
	struct v4l2_audio audio_set;

	/* remember the original settings */
	memset(&audio_orig, 0, sizeof(audio_orig));
	ret_orig = ioctl(get_video_fd(), VIDIOC_G_AUDIO, &audio_orig);
	errno_orig = errno;

	dprintf("VIDIOC_G_AUDIO, ret_orig=%i, errno_orig=%i\n", ret_orig, errno_orig);

	/* test invalid index */
	memset(&audio, 0xff, sizeof(audio));
	audio.index = (__u32)S32_MAX;
	ret = ioctl(get_video_fd(), VIDIOC_S_AUDIO, &audio);

	CU_ASSERT_EQUAL(ret, -1);
	CU_ASSERT_EQUAL(errno, EINVAL);

	/* Check whether the original audio struct is untouched */
	memset(&audio2, 0xff, sizeof(audio2));
	audio2.index = (__u32)S32_MAX;
	CU_ASSERT_EQUAL(memcmp(&audio, &audio2, sizeof(audio)), 0);

	/* restore the original audio input settings */
	memset(&audio_set, 0, sizeof(audio_set));
	audio_set.index = audio_orig.index;
	ret_set = ioctl(get_video_fd(), VIDIOC_S_AUDIO, &audio_set);
	errno_set = errno;

	dprintf("VIDIOC_S_AUDIO, ret_set=%i, errno_set=%i\n", ret_set, errno_set);

	if (ret_orig == 0) {
		/* If it was possible at the beginning to get the audio input then
		 * it shall be possible to set it again.
		 */
		CU_ASSERT_EQUAL(ret_set, 0);
	} else {
		/* In case we could not fetch the audio input value at the start
		 * of this test case: the VIDIOC_S_AUDIO shall also fail.
		 */
		CU_ASSERT_EQUAL(ret_set, -1);
		CU_ASSERT_EQUAL(errno_set, EINVAL);
	}
}

void test_VIDIOC_S_AUDIO_S32_MAX_1() {
	int ret;
	int ret_orig, errno_orig;
	int ret_set, errno_set;
	struct v4l2_audio audio;
	struct v4l2_audio audio2;
	struct v4l2_audio audio_orig;
	struct v4l2_audio audio_set;

	/* remember the original settings */
	memset(&audio_orig, 0, sizeof(audio_orig));
	ret_orig = ioctl(get_video_fd(), VIDIOC_G_AUDIO, &audio_orig);
	errno_orig = errno;

	dprintf("VIDIOC_G_AUDIO, ret_orig=%i, errno_orig=%i\n", ret_orig, errno_orig);

	/* test invalid index */
	memset(&audio, 0xff, sizeof(audio));
	audio.index = ((__u32)S32_MAX)+1;
	ret = ioctl(get_video_fd(), VIDIOC_S_AUDIO, &audio);

	CU_ASSERT_EQUAL(ret, -1);
	CU_ASSERT_EQUAL(errno, EINVAL);

	/* Check whether the original audio struct is untouched */
	memset(&audio2, 0xff, sizeof(audio2));
	audio2.index = ((__u32)S32_MAX)+1;
	CU_ASSERT_EQUAL(memcmp(&audio, &audio2, sizeof(audio)), 0);

	/* restore the original audio input settings */
	memset(&audio_set, 0, sizeof(audio_set));
	audio_set.index = audio_orig.index;
	ret_set = ioctl(get_video_fd(), VIDIOC_S_AUDIO, &audio_set);
	errno_set = errno;

	dprintf("VIDIOC_S_AUDIO, ret_set=%i, errno_set=%i\n", ret_set, errno_set);

	if (ret_orig == 0) {
		/* If it was possible at the beginning to get the audio input then
		 * it shall be possible to set it again.
		 */
		CU_ASSERT_EQUAL(ret_set, 0);
	} else {
		/* In case we could not fetch the audio input value at the start
		 * of this test case: the VIDIOC_S_AUDIO shall also fail.
		 */
		CU_ASSERT_EQUAL(ret_set, -1);
		CU_ASSERT_EQUAL(errno_set, EINVAL);
	}
}


void test_VIDIOC_S_AUDIO_U32_MAX() {
	int ret;
	int ret_orig, errno_orig;
	int ret_set, errno_set;
	struct v4l2_audio audio;
	struct v4l2_audio audio2;
	struct v4l2_audio audio_orig;
	struct v4l2_audio audio_set;

	/* remember the original settings */
	memset(&audio_orig, 0, sizeof(audio_orig));
	ret_orig = ioctl(get_video_fd(), VIDIOC_G_AUDIO, &audio_orig);
	errno_orig = errno;

	dprintf("VIDIOC_G_AUDIO, ret_orig=%i, errno_orig=%i\n", ret_orig, errno_orig);
	/* test invalid index */
	memset(&audio, 0xff, sizeof(audio));
	audio.index = U32_MAX;
	ret = ioctl(get_video_fd(), VIDIOC_S_AUDIO, &audio);

	CU_ASSERT_EQUAL(ret, -1);
	CU_ASSERT_EQUAL(errno, EINVAL);

	/* Check whether the original audio struct is untouched */
	memset(&audio2, 0xff, sizeof(audio2));
	audio2.index = U32_MAX;
	CU_ASSERT_EQUAL(memcmp(&audio, &audio2, sizeof(audio)), 0);

	/* restore the original audio input settings */
	memset(&audio_set, 0, sizeof(audio_set));
	audio_set.index = audio_orig.index;
	ret_set = ioctl(get_video_fd(), VIDIOC_S_AUDIO, &audio_set);
	errno_set = errno;

	dprintf("VIDIOC_S_AUDIO, ret_set=%i, errno_set=%i\n", ret_set, errno_set);

	if (ret_orig == 0) {
		/* If it was possible at the beginning to get the audio input then
		 * it shall be possible to set it again.
		 */
		CU_ASSERT_EQUAL(ret_set, 0);
	} else {
		/* In case we could not fetch the audio input value at the start
		 * of this test case: the VIDIOC_S_AUDIO shall also fail.
		 */
		CU_ASSERT_EQUAL(ret_set, -1);
		CU_ASSERT_EQUAL(errno_set, EINVAL);
	}
}

void test_VIDIOC_S_AUDIO_NULL() {
	int ret_orig, errno_orig;
	int ret_set, errno_set;
	int ret1, errno1;
	int ret2, errno2;
	struct v4l2_audio audio_orig;
	struct v4l2_audio audio_set;

	/* remember the original settings */
	memset(&audio_orig, 0, sizeof(audio_orig));
	ret_orig = ioctl(get_video_fd(), VIDIOC_G_AUDIO, &audio_orig);
	errno_orig = errno;

	dprintf("\tVIDIOC_G_AUDIO, ret_orig=%i, errno_orig=%i\n", ret_orig, errno_orig);

	memset(&audio_set, 0, sizeof(audio_set));
	ret1 = ioctl(get_video_fd(), VIDIOC_S_AUDIO, &audio_set);
	errno1 = errno;

	dprintf("\tVIDIOC_S_AUDIO, ret1=%i, errno1=%i\n", ret1, errno1);

	ret2 = ioctl(get_video_fd(), VIDIOC_S_AUDIO, NULL);
	errno2 = errno;

	dprintf("\tVIDIOC_S_AUDIO, ret2=%i, errno2=%i\n", ret2, errno2);

	if (ret1 == 0) {
		CU_ASSERT_EQUAL(ret1, 0);
		CU_ASSERT_EQUAL(ret2, -1);
		CU_ASSERT_EQUAL(errno2, EFAULT);
	} else {
		CU_ASSERT_EQUAL(ret1, -1);
		CU_ASSERT_EQUAL(errno1, EINVAL);
		CU_ASSERT_EQUAL(ret2, -1);
		CU_ASSERT_EQUAL(errno2, EINVAL);
	}

	/* restore the original audio input settings */
	memset(&audio_set, 0, sizeof(audio_set));
	audio_set.index = audio_orig.index;
	ret_set = ioctl(get_video_fd(), VIDIOC_S_AUDIO, &audio_set);
	errno_set = errno;

	dprintf("VIDIOC_S_AUDIO, ret_set=%i, errno_set=%i\n", ret_set, errno_set);

	if (ret_orig == 0) {
		/* If it was possible at the beginning to get the audio input then
		 * it shall be possible to set it again.
		 */
		CU_ASSERT_EQUAL(ret_set, 0);
	} else {
		/* In case we could not fetch the audio input value at the start
		 * of this test case: the VIDIOC_S_AUDIO shall also fail.
		 */
		CU_ASSERT_EQUAL(ret_set, -1);
		CU_ASSERT_EQUAL(errno_set, EINVAL);
	}
}

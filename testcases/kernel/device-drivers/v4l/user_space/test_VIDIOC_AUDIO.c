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

	dprintf("VIDIOC_AUDIO, ret=%i\n", ret);

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

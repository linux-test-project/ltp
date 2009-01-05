/*
 * v4l-test: Test environment for Video For Linux Two API
 *
 *  1 Jan 2009  0.3  Added index=S32_MAX, S32_MAX+1 and U32_MAX
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

#include "test_VIDIOC_ENUMAUDIO.h"

void test_VIDIOC_ENUMAUDIO() {
	int ret;
	struct v4l2_audio audio;
	struct v4l2_audio audio2;
	__u32 i;

	i = 0;
	do {
		memset(&audio, 0xff, sizeof(audio));
		audio.index = i;
		ret = ioctl(get_video_fd(), VIDIOC_ENUMAUDIO, &audio);

		dprintf("VIDIOC_ENUMAUDIO, ret=%i\n", ret);

		if (ret == 0) {
			CU_ASSERT_EQUAL(ret, 0);
			CU_ASSERT_EQUAL(audio.index, i);

			//CU_ASSERT_EQUAL(audio.name, ?);
			CU_ASSERT(0 < strlen( (char*)audio.name ));

			//CU_ASSERT_EQUAL(audio.capability, ?);
			//CU_ASSERT_EQUAL(audio.mode, ?);
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

			memset(&audio2, 0xff, sizeof(audio2));
			audio2.index = i;
			CU_ASSERT_EQUAL(memcmp(&audio, &audio2, sizeof(audio)), 0);

			dprintf("\terrno=%i\n", errno);

		}
		i++;
	} while (ret == 0);

}

void test_VIDIOC_ENUMAUDIO_S32_MAX() {
	int ret;
	struct v4l2_audio audio;
	struct v4l2_audio audio2;

	memset(&audio, 0xff, sizeof(audio));
	audio.index = (__u32)S32_MAX;
	ret = ioctl(get_video_fd(), VIDIOC_ENUMAUDIO, &audio);

	CU_ASSERT_EQUAL(ret, -1);
	CU_ASSERT_EQUAL(errno, EINVAL);

	/* Check whether the original audio struct is untouched */
	memset(&audio2, 0xff, sizeof(audio2));
	audio2.index = (__u32)S32_MAX;
	CU_ASSERT_EQUAL(memcmp(&audio, &audio2, sizeof(audio)), 0);
}

void test_VIDIOC_ENUMAUDIO_S32_MAX_1() {
	int ret;
	struct v4l2_audio audio;
	struct v4l2_audio audio2;

	memset(&audio, 0xff, sizeof(audio));
	audio.index = ((__u32)S32_MAX)+1;
	ret = ioctl(get_video_fd(), VIDIOC_ENUMAUDIO, &audio);

	CU_ASSERT_EQUAL(ret, -1);
	CU_ASSERT_EQUAL(errno, EINVAL);

	/* Check whether the original audio struct is untouched */
	memset(&audio2, 0xff, sizeof(audio2));
	audio2.index = ((__u32)S32_MAX)+1;
	CU_ASSERT_EQUAL(memcmp(&audio, &audio2, sizeof(audio)), 0);
}


void test_VIDIOC_ENUMAUDIO_U32_MAX() {
	int ret;
	struct v4l2_audio audio;
	struct v4l2_audio audio2;

	memset(&audio, 0xff, sizeof(audio));
	audio.index = U32_MAX;
	ret = ioctl(get_video_fd(), VIDIOC_ENUMAUDIO, &audio);

	CU_ASSERT_EQUAL(ret, -1);
	CU_ASSERT_EQUAL(errno, EINVAL);

	/* Check whether the original audio struct is untouched */
	memset(&audio2, 0xff, sizeof(audio2));
	audio2.index = U32_MAX;
	CU_ASSERT_EQUAL(memcmp(&audio, &audio2, sizeof(audio)), 0);
}

void test_VIDIOC_ENUMAUDIO_NULL() {
	int ret;

	ret = ioctl(get_video_fd(), VIDIOC_ENUMAUDIO, NULL);
	CU_ASSERT_EQUAL(ret, -1);
	CU_ASSERT_EQUAL(errno, EFAULT);

}

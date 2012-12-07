/*
 * v4l-test: Test environment for Video For Linux Two API
 *
 * 20 Apr 2009  0.7  Added string content validation
 * 18 Apr 2009  0.6  More strict check for strings
 * 29 Mar 2009  0.5  Clean up test case for NULL parameter
 * 28 Mar 2009  0.4  Clean up ret and errno variable names and dprintf() output
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
#include "v4l2_validator.h"

#include "test_VIDIOC_ENUMAUDIO.h"

void test_VIDIOC_ENUMAUDIO()
{
	int ret_enum, errno_enum;
	struct v4l2_audio audio;
	struct v4l2_audio audio2;
	__u32 i;

	i = 0;
	do {
		memset(&audio, 0xff, sizeof(audio));
		audio.index = i;
		ret_enum = ioctl(get_video_fd(), VIDIOC_ENUMAUDIO, &audio);
		errno_enum = errno;

		dprintf
		    ("\t%s:%u: VIDIOC_ENUMAUDIO, ret_enum=%i, errno_enum=%i\n",
		     __FILE__, __LINE__, ret_enum, errno_enum);

		if (ret_enum == 0) {
			CU_ASSERT_EQUAL(ret_enum, 0);
			CU_ASSERT_EQUAL(audio.index, i);

			CU_ASSERT(0 < strlen((char *)audio.name));
			CU_ASSERT(valid_string
				  ((char *)audio.name, sizeof(audio.name)));

			//CU_ASSERT_EQUAL(audio.capability, ?);
			//CU_ASSERT_EQUAL(audio.mode, ?);
			CU_ASSERT_EQUAL(audio.reserved[0], 0);
			CU_ASSERT_EQUAL(audio.reserved[1], 0);

			/* Check if the unused bytes of the name string are
			 * also filled with zeros. Also check if there is any
			 * padding byte between any two fields then this
			 * padding byte is also filled with zeros.
			 */
			memset(&audio2, 0, sizeof(audio2));
			audio2.index = audio.index;
			strncpy((char *)audio2.name, (char *)audio.name,
				sizeof(audio2.name));
			audio2.capability = audio.capability;
			audio2.mode = audio.mode;
			CU_ASSERT_EQUAL(memcmp(&audio, &audio2, sizeof(audio)),
					0);

			dprintf("\taudio = {.index=%u, .name=\"%s\", "
				".capability=0x%X, .mode=0x%X, "
				".reserved[]={ 0x%X, 0x%X } }\n",
				audio.index,
				audio.name,
				audio.capability,
				audio.mode, audio.reserved[0], audio.reserved[1]
			    );

		} else {
			CU_ASSERT_EQUAL(ret_enum, -1);
			CU_ASSERT_EQUAL(errno_enum, EINVAL);

			memset(&audio2, 0xff, sizeof(audio2));
			audio2.index = i;
			CU_ASSERT_EQUAL(memcmp(&audio, &audio2, sizeof(audio)),
					0);

		}
		i++;
	} while (ret_enum == 0);

}

void test_VIDIOC_ENUMAUDIO_S32_MAX()
{
	int ret_enum, errno_enum;
	struct v4l2_audio audio;
	struct v4l2_audio audio2;

	memset(&audio, 0xff, sizeof(audio));
	audio.index = (__u32) S32_MAX;
	ret_enum = ioctl(get_video_fd(), VIDIOC_ENUMAUDIO, &audio);
	errno_enum = errno;

	CU_ASSERT_EQUAL(ret_enum, -1);
	CU_ASSERT_EQUAL(errno_enum, EINVAL);

	/* Check whether the original audio struct is untouched */
	memset(&audio2, 0xff, sizeof(audio2));
	audio2.index = (__u32) S32_MAX;
	CU_ASSERT_EQUAL(memcmp(&audio, &audio2, sizeof(audio)), 0);
}

void test_VIDIOC_ENUMAUDIO_S32_MAX_1()
{
	int ret_enum, errno_enum;
	struct v4l2_audio audio;
	struct v4l2_audio audio2;

	memset(&audio, 0xff, sizeof(audio));
	audio.index = ((__u32) S32_MAX) + 1;
	ret_enum = ioctl(get_video_fd(), VIDIOC_ENUMAUDIO, &audio);
	errno_enum = errno;

	CU_ASSERT_EQUAL(ret_enum, -1);
	CU_ASSERT_EQUAL(errno_enum, EINVAL);

	/* Check whether the original audio struct is untouched */
	memset(&audio2, 0xff, sizeof(audio2));
	audio2.index = ((__u32) S32_MAX) + 1;
	CU_ASSERT_EQUAL(memcmp(&audio, &audio2, sizeof(audio)), 0);
}

void test_VIDIOC_ENUMAUDIO_U32_MAX()
{
	int ret_enum, errno_enum;
	struct v4l2_audio audio;
	struct v4l2_audio audio2;

	memset(&audio, 0xff, sizeof(audio));
	audio.index = U32_MAX;
	ret_enum = ioctl(get_video_fd(), VIDIOC_ENUMAUDIO, &audio);
	errno_enum = errno;

	CU_ASSERT_EQUAL(ret_enum, -1);
	CU_ASSERT_EQUAL(errno_enum, EINVAL);

	/* Check whether the original audio struct is untouched */
	memset(&audio2, 0xff, sizeof(audio2));
	audio2.index = U32_MAX;
	CU_ASSERT_EQUAL(memcmp(&audio, &audio2, sizeof(audio)), 0);
}

void test_VIDIOC_ENUMAUDIO_NULL()
{
	int ret_enum, errno_enum;
	int ret_null, errno_null;
	struct v4l2_audio audio;

	memset(&audio, 0, sizeof(audio));
	audio.index = 0;
	ret_enum = ioctl(get_video_fd(), VIDIOC_ENUMAUDIO, &audio);
	errno_enum = errno;

	dprintf("\t%s:%u: VIDIOC_ENUMAUDIO, ret_enum=%i, errno_enum=%i\n",
		__FILE__, __LINE__, ret_enum, errno_enum);

	ret_null = ioctl(get_video_fd(), VIDIOC_ENUMAUDIO, NULL);
	errno_null = errno;

	dprintf("\t%s:%u: VIDIOC_ENUMAUDIO, ret_null=%i, errno_null=%i\n",
		__FILE__, __LINE__, ret_null, errno_null);

	if (ret_enum == 0) {
		CU_ASSERT_EQUAL(ret_enum, 0);
		CU_ASSERT_EQUAL(ret_null, -1);
		CU_ASSERT_EQUAL(errno_null, EFAULT);
	} else {
		CU_ASSERT_EQUAL(ret_enum, -1);
		CU_ASSERT_EQUAL(errno_enum, EINVAL);
		CU_ASSERT_EQUAL(ret_null, -1);
		CU_ASSERT_EQUAL(errno_null, EINVAL);
	}

}

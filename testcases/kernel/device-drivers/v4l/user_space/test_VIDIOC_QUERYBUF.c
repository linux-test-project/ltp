/*
 * v4l-test: Test environment for Video For Linux Two API
 *
 *  5 May 2009  0.1  First release
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
#include "v4l2_show.h"
#include "dev_video.h"
#include "video_limits.h"

#include "test_VIDIOC_REQBUFS.h"

static void do_VIDIOC_QUERYBUF(enum v4l2_memory memory,
			       enum v4l2_buf_type type, __u32 count,
			       int expected_ret_req)
{
	int ret_req, errno_req;
	int ret_query, errno_query;
	struct v4l2_requestbuffers reqbuf;
	struct v4l2_buffer buf;
	__u32 i;
	unsigned int j;
	const enum v4l2_buf_type buffer_types[] = {
		0,
		V4L2_BUF_TYPE_VIDEO_CAPTURE,
		V4L2_BUF_TYPE_VIDEO_OUTPUT,
		V4L2_BUF_TYPE_VIDEO_OVERLAY,
		V4L2_BUF_TYPE_VBI_CAPTURE,
		V4L2_BUF_TYPE_VBI_OUTPUT,
		V4L2_BUF_TYPE_SLICED_VBI_CAPTURE,
		V4L2_BUF_TYPE_SLICED_VBI_OUTPUT,
		V4L2_BUF_TYPE_VIDEO_OUTPUT_OVERLAY,
		V4L2_BUF_TYPE_PRIVATE - 1,
		V4L2_BUF_TYPE_PRIVATE,
		V4L2_BUF_TYPE_PRIVATE + 1,
		S32_MAX,
		(__s32) ((__u32) S32_MAX + 1),
		U32_MAX - 1,
		U32_MAX
	};

	memset(&reqbuf, 0, sizeof(reqbuf));
	reqbuf.count = count;
	reqbuf.type = type;
	reqbuf.memory = memory;

	ret_req = ioctl(get_video_fd(), VIDIOC_REQBUFS, &reqbuf);
	errno_req = errno;

	if (expected_ret_req == 0) {
		CU_ASSERT_EQUAL(ret_req, 0);
	} else {
		CU_ASSERT_EQUAL(ret_req, -1);
		CU_ASSERT_EQUAL(errno_req, EINVAL);
	}

	dprintf
	    ("\t%s:%u: VIDIOC_REQBUF, count=%u, type=%i, memory=%i, ret_req=%i, errno_req=%i\n",
	     __FILE__, __LINE__, count, type, memory, ret_req, errno_req);

	if (ret_req == 0) {
		show_v4l2_requestbuffers(&reqbuf);
	}

	if (ret_req != 0) {
		reqbuf.count = 0;
	}

	for (i = 0; i < reqbuf.count; i++) {
		memset(&buf, 0xff, sizeof(buf));
		buf.type = type;
		buf.index = i;

		ret_query = ioctl(get_video_fd(), VIDIOC_QUERYBUF, &buf);
		errno_query = errno;

		CU_ASSERT_EQUAL(buf.index, i);
		CU_ASSERT_EQUAL(buf.type, type);
		//CU_ASSERT_EQUAL(buf.bytesused, ???);
		CU_ASSERT_EQUAL(buf.
				flags & ~(V4L2_BUF_FLAG_MAPPED |
					  V4L2_BUF_FLAG_QUEUED |
					  V4L2_BUF_FLAG_DONE), 0);
		//CU_ASSERT_EQUAL(buf.field, ???);
		//CU_ASSERT_EQUAL(buf.timestamp.tv_sec, ???);
		//CU_ASSERT_EQUAL(buf.timestamp.tv_usec, ???);
		//CU_ASSERT_EQUAL(buf.timecode.type, ???);
		//CU_ASSERT_EQUAL(buf.timecode.flags, ???);
		//CU_ASSERT_EQUAL(buf.timecode.frames, ???);
		//CU_ASSERT_EQUAL(buf.timecode.seconds, ???);
		//CU_ASSERT_EQUAL(buf.timecode.minutes, ???);
		//CU_ASSERT_EQUAL(buf.timecode.hours, ???);
		//CU_ASSERT_EQUAL(buf.timecode.userbits[0], ???);
		//CU_ASSERT_EQUAL(buf.timecode.userbits[1], ???);
		//CU_ASSERT_EQUAL(buf.timecode.userbits[2], ???);
		//CU_ASSERT_EQUAL(buf.timecode.userbits[3], ???);
		//CU_ASSERT_EQUAL(buf.sequence, ???);
		CU_ASSERT_EQUAL(buf.memory, memory);
		//CU_ASSERT_EQUAL(buf.m.userptr, ???);
		//CU_ASSERT_EQUAL(buf.m.offset, ???);
		CU_ASSERT(0 < buf.length);
		//CU_ASSERT_EQUAL(buf.input, ???);
		CU_ASSERT_EQUAL(buf.reserved, 0);

		dprintf
		    ("\t%s:%u: VIDIOC_QUERYBUF, type=%i, index=%u, ret_query=%i, errno_query=%i\n",
		     __FILE__, __LINE__, buf.type, buf.index, ret_query,
		     errno_query);
		if (ret_query == 0)
			show_v4l2_buffer(&buf);

	}

	/* Check for not buffer types which do not match the VIDIOC_REQBUF
	 * buffer type
	 */
	for (i = 0; i < reqbuf.count; i++) {
		for (j = 0; j < sizeof(buffer_types) / sizeof(*buffer_types);
		     j++) {
			if (buffer_types[j] != type) {
				memset(&buf, 0xff, sizeof(buf));
				buf.type = buffer_types[j];
				buf.index = i;

				ret_query =
				    ioctl(get_video_fd(), VIDIOC_QUERYBUF,
					  &buf);
				errno_query = errno;

				CU_ASSERT_EQUAL(ret_query, -1);
				CU_ASSERT_EQUAL(errno_query, EINVAL);

				dprintf
				    ("\t%s:%u: VIDIOC_QUERYBUF, type=%i, index=%u, ret_query=%i, errno_query=%i\n",
				     __FILE__, __LINE__, buf.type, buf.index,
				     ret_query, errno_query);
				if (ret_query == 0)
					show_v4l2_buffer(&buf);
			}
		}
	}

	memset(&buf, 0xff, sizeof(buf));
	buf.type = type;
	buf.index = reqbuf.count;

	ret_query = ioctl(get_video_fd(), VIDIOC_QUERYBUF, &buf);
	errno_query = errno;

	dprintf
	    ("\t%s:%u: VIDIOC_QUERYBUF, type=%i, index=%u, ret_query=%i, errno_query=%i\n",
	     __FILE__, __LINE__, buf.type, buf.index, ret_query, errno_query);

	CU_ASSERT_EQUAL(ret_query, -1);
	CU_ASSERT_EQUAL(errno_query, EINVAL);

	if (reqbuf.count < (__u32) S16_MIN) {
		memset(&buf, 0xff, sizeof(buf));
		buf.type = type;
		buf.index = (__u32) S16_MIN;

		ret_query = ioctl(get_video_fd(), VIDIOC_QUERYBUF, &buf);
		errno_query = errno;

		dprintf
		    ("\t%s:%u: VIDIOC_QUERYBUF, type=%i, index=%u, ret_query=%i, errno_query=%i\n",
		     __FILE__, __LINE__, buf.type, buf.index, ret_query,
		     errno_query);

		CU_ASSERT_EQUAL(ret_query, -1);
		CU_ASSERT_EQUAL(errno_query, EINVAL);
	}

	if (reqbuf.count < (__u32) S16_MAX) {
		memset(&buf, 0xff, sizeof(buf));
		buf.type = type;
		buf.index = (__u32) S16_MAX;

		ret_query = ioctl(get_video_fd(), VIDIOC_QUERYBUF, &buf);
		errno_query = errno;

		dprintf
		    ("\t%s:%u: VIDIOC_QUERYBUF, type=%i, index=%u, ret_query=%i, errno_query=%i\n",
		     __FILE__, __LINE__, buf.type, buf.index, ret_query,
		     errno_query);

		CU_ASSERT_EQUAL(ret_query, -1);
		CU_ASSERT_EQUAL(errno_query, EINVAL);
	}

	if (reqbuf.count < U32_MAX) {
		memset(&buf, 0xff, sizeof(buf));
		buf.type = type;
		buf.index = U32_MAX;

		ret_query = ioctl(get_video_fd(), VIDIOC_QUERYBUF, &buf);
		errno_query = errno;

		dprintf
		    ("\t%s:%u: VIDIOC_QUERYBUF, type=%i, index=%u, ret_query=%i, errno_query=%i\n",
		     __FILE__, __LINE__, buf.type, buf.index, ret_query,
		     errno_query);

		CU_ASSERT_EQUAL(ret_query, -1);
		CU_ASSERT_EQUAL(errno_query, EINVAL);
	}

	memset(&reqbuf, 0, sizeof(reqbuf));
	reqbuf.count = 0;
	reqbuf.type = type;
	reqbuf.memory = V4L2_MEMORY_MMAP;

	ret_req = ioctl(get_video_fd(), VIDIOC_REQBUFS, &reqbuf);
	errno_req = errno;

	dprintf("\t%s:%u: VIDIOC_REQBUF, count=%u, ret_req=%i, errno_req=%i\n",
		__FILE__, __LINE__, count, ret_req, errno_req);

	if (expected_ret_req == 0) {
		CU_ASSERT_EQUAL(ret_req, 0);
	} else {
		CU_ASSERT_EQUAL(ret_req, -1);
		CU_ASSERT_EQUAL(errno_req, EINVAL);
	}
	if (ret_req == 0) {
		show_v4l2_requestbuffers(&reqbuf);
	}
}

void test_VIDIOC_QUERYBUF_capture_mmap()
{
	int ret_cap, errno_cap;
	struct v4l2_capability cap;
	int expected_ret_req;

	memset(&cap, 0, sizeof(cap));

	ret_cap = ioctl(get_video_fd(), VIDIOC_QUERYCAP, &cap);
	errno_cap = errno;

	if (ret_cap == 0 &&
	    (cap.capabilities & V4L2_CAP_STREAMING) &&
	    (cap.capabilities & V4L2_CAP_VIDEO_CAPTURE)) {
		expected_ret_req = 0;
	} else {
		expected_ret_req = -1;
	}

	do_VIDIOC_QUERYBUF(V4L2_MEMORY_MMAP, V4L2_BUF_TYPE_VIDEO_CAPTURE, 0,
			   expected_ret_req);
	do_VIDIOC_QUERYBUF(V4L2_MEMORY_MMAP, V4L2_BUF_TYPE_VIDEO_CAPTURE, 1,
			   expected_ret_req);
	do_VIDIOC_QUERYBUF(V4L2_MEMORY_MMAP, V4L2_BUF_TYPE_VIDEO_CAPTURE, 2,
			   expected_ret_req);
	do_VIDIOC_QUERYBUF(V4L2_MEMORY_MMAP, V4L2_BUF_TYPE_VIDEO_CAPTURE, 3,
			   expected_ret_req);
	do_VIDIOC_QUERYBUF(V4L2_MEMORY_MMAP, V4L2_BUF_TYPE_VIDEO_CAPTURE, 4,
			   expected_ret_req);
	do_VIDIOC_QUERYBUF(V4L2_MEMORY_MMAP, V4L2_BUF_TYPE_VIDEO_CAPTURE,
			   (__u32) S16_MIN, expected_ret_req);
	do_VIDIOC_QUERYBUF(V4L2_MEMORY_MMAP, V4L2_BUF_TYPE_VIDEO_CAPTURE,
			   (__u32) S16_MAX, expected_ret_req);
	do_VIDIOC_QUERYBUF(V4L2_MEMORY_MMAP, V4L2_BUF_TYPE_VIDEO_CAPTURE,
			   U32_MAX, expected_ret_req);
}

void test_VIDIOC_QUERYBUF_capture_userptr()
{
	int ret_cap, errno_cap;
	int ret_req, errno_req;
	struct v4l2_capability cap;
	struct v4l2_requestbuffers reqbuf;
	int expected_ret_req;

	memset(&cap, 0, sizeof(cap));

	ret_cap = ioctl(get_video_fd(), VIDIOC_QUERYCAP, &cap);
	errno_cap = errno;

	memset(&reqbuf, 0, sizeof(reqbuf));
	reqbuf.count = 2;
	reqbuf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	reqbuf.memory = V4L2_MEMORY_USERPTR;

	ret_req = ioctl(get_video_fd(), VIDIOC_REQBUFS, &reqbuf);
	errno_req = errno;

	dprintf("\t%s:%u: VIDIOC_REQBUF, count=%u, ret_req=%i, errno_req=%i\n",
		__FILE__, __LINE__, reqbuf.count, ret_req, errno_req);

	if (ret_cap == 0 &&
	    (cap.capabilities & V4L2_CAP_STREAMING) &&
	    (cap.capabilities & V4L2_CAP_VIDEO_CAPTURE) && ret_req == 0) {
		expected_ret_req = 0;
	} else {
		expected_ret_req = -1;
	}

	do_VIDIOC_QUERYBUF(V4L2_MEMORY_USERPTR, V4L2_BUF_TYPE_VIDEO_CAPTURE, 0,
			   expected_ret_req);
	do_VIDIOC_QUERYBUF(V4L2_MEMORY_USERPTR, V4L2_BUF_TYPE_VIDEO_CAPTURE, 1,
			   expected_ret_req);
	do_VIDIOC_QUERYBUF(V4L2_MEMORY_USERPTR, V4L2_BUF_TYPE_VIDEO_CAPTURE, 2,
			   expected_ret_req);
	do_VIDIOC_QUERYBUF(V4L2_MEMORY_USERPTR, V4L2_BUF_TYPE_VIDEO_CAPTURE, 3,
			   expected_ret_req);
	do_VIDIOC_QUERYBUF(V4L2_MEMORY_USERPTR, V4L2_BUF_TYPE_VIDEO_CAPTURE, 4,
			   expected_ret_req);
	do_VIDIOC_QUERYBUF(V4L2_MEMORY_USERPTR, V4L2_BUF_TYPE_VIDEO_CAPTURE,
			   (__u32) S16_MIN, expected_ret_req);
	do_VIDIOC_QUERYBUF(V4L2_MEMORY_USERPTR, V4L2_BUF_TYPE_VIDEO_CAPTURE,
			   (__u32) S16_MAX, expected_ret_req);
	do_VIDIOC_QUERYBUF(V4L2_MEMORY_USERPTR, V4L2_BUF_TYPE_VIDEO_CAPTURE,
			   U32_MAX, expected_ret_req);
	do_VIDIOC_QUERYBUF(V4L2_MEMORY_USERPTR, V4L2_BUF_TYPE_VIDEO_CAPTURE, 0,
			   expected_ret_req);
}

void test_VIDIOC_QUERYBUF_output_mmap()
{
	int ret_cap, errno_cap;
	struct v4l2_capability cap;
	int expected_ret_req;

	memset(&cap, 0, sizeof(cap));

	ret_cap = ioctl(get_video_fd(), VIDIOC_QUERYCAP, &cap);
	errno_cap = errno;

	if (ret_cap == 0 &&
	    (cap.capabilities & V4L2_CAP_STREAMING) &&
	    (cap.capabilities & V4L2_CAP_VIDEO_OUTPUT)) {
		expected_ret_req = 0;
	} else {
		expected_ret_req = -1;
	}

	do_VIDIOC_QUERYBUF(V4L2_MEMORY_MMAP, V4L2_BUF_TYPE_VIDEO_OUTPUT, 0,
			   expected_ret_req);
	do_VIDIOC_QUERYBUF(V4L2_MEMORY_MMAP, V4L2_BUF_TYPE_VIDEO_OUTPUT, 1,
			   expected_ret_req);
	do_VIDIOC_QUERYBUF(V4L2_MEMORY_MMAP, V4L2_BUF_TYPE_VIDEO_OUTPUT, 2,
			   expected_ret_req);
	do_VIDIOC_QUERYBUF(V4L2_MEMORY_MMAP, V4L2_BUF_TYPE_VIDEO_OUTPUT, 3,
			   expected_ret_req);
	do_VIDIOC_QUERYBUF(V4L2_MEMORY_MMAP, V4L2_BUF_TYPE_VIDEO_OUTPUT, 4,
			   expected_ret_req);
	do_VIDIOC_QUERYBUF(V4L2_MEMORY_MMAP, V4L2_BUF_TYPE_VIDEO_OUTPUT,
			   (__u32) S16_MIN, expected_ret_req);
	do_VIDIOC_QUERYBUF(V4L2_MEMORY_MMAP, V4L2_BUF_TYPE_VIDEO_OUTPUT,
			   (__u32) S16_MAX, expected_ret_req);
	do_VIDIOC_QUERYBUF(V4L2_MEMORY_MMAP, V4L2_BUF_TYPE_VIDEO_OUTPUT,
			   U32_MAX, expected_ret_req);
	do_VIDIOC_QUERYBUF(V4L2_MEMORY_MMAP, V4L2_BUF_TYPE_VIDEO_OUTPUT, 0,
			   expected_ret_req);
}

void test_VIDIOC_QUERYBUF_output_userptr()
{
	int ret_cap, errno_cap;
	int ret_req, errno_req;
	struct v4l2_capability cap;
	struct v4l2_requestbuffers reqbuf;
	int expected_ret_req;

	memset(&cap, 0, sizeof(cap));

	ret_cap = ioctl(get_video_fd(), VIDIOC_QUERYCAP, &cap);
	errno_cap = errno;

	memset(&reqbuf, 0, sizeof(reqbuf));
	reqbuf.count = 2;
	reqbuf.type = V4L2_BUF_TYPE_VIDEO_OUTPUT;
	reqbuf.memory = V4L2_MEMORY_USERPTR;

	ret_req = ioctl(get_video_fd(), VIDIOC_REQBUFS, &reqbuf);
	errno_req = errno;

	dprintf("\t%s:%u: VIDIOC_REQBUF, count=%u, ret_req=%i, errno_req=%i\n",
		__FILE__, __LINE__, reqbuf.count, ret_req, errno_req);

	if (ret_cap == 0 &&
	    (cap.capabilities & V4L2_CAP_STREAMING) &&
	    (cap.capabilities & V4L2_CAP_VIDEO_OUTPUT) && ret_req == 0) {
		expected_ret_req = 0;
	} else {
		expected_ret_req = -1;
	}

	do_VIDIOC_QUERYBUF(V4L2_MEMORY_USERPTR, V4L2_BUF_TYPE_VIDEO_OUTPUT, 0,
			   expected_ret_req);
	do_VIDIOC_QUERYBUF(V4L2_MEMORY_USERPTR, V4L2_BUF_TYPE_VIDEO_OUTPUT, 1,
			   expected_ret_req);
	do_VIDIOC_QUERYBUF(V4L2_MEMORY_USERPTR, V4L2_BUF_TYPE_VIDEO_OUTPUT, 2,
			   expected_ret_req);
	do_VIDIOC_QUERYBUF(V4L2_MEMORY_USERPTR, V4L2_BUF_TYPE_VIDEO_OUTPUT, 3,
			   expected_ret_req);
	do_VIDIOC_QUERYBUF(V4L2_MEMORY_USERPTR, V4L2_BUF_TYPE_VIDEO_OUTPUT, 4,
			   expected_ret_req);
	do_VIDIOC_QUERYBUF(V4L2_MEMORY_USERPTR, V4L2_BUF_TYPE_VIDEO_OUTPUT,
			   (__u32) S16_MIN, expected_ret_req);
	do_VIDIOC_QUERYBUF(V4L2_MEMORY_USERPTR, V4L2_BUF_TYPE_VIDEO_OUTPUT,
			   (__u32) S16_MAX, expected_ret_req);
	do_VIDIOC_QUERYBUF(V4L2_MEMORY_USERPTR, V4L2_BUF_TYPE_VIDEO_OUTPUT,
			   U32_MAX, expected_ret_req);
	do_VIDIOC_QUERYBUF(V4L2_MEMORY_USERPTR, V4L2_BUF_TYPE_VIDEO_OUTPUT, 0,
			   expected_ret_req);
}

void test_VIDIOC_QUERYBUF_overlay_capture()
{
	do_VIDIOC_QUERYBUF(V4L2_MEMORY_OVERLAY, V4L2_BUF_TYPE_VIDEO_CAPTURE, 2,
			   -1);
}

void test_VIDIOC_QUERYBUF_overlay_output()
{
	do_VIDIOC_QUERYBUF(V4L2_MEMORY_OVERLAY, V4L2_BUF_TYPE_VIDEO_OUTPUT, 2,
			   -1);
}

void test_VIDIOC_QUERYBUF_invalid_memory_capture()
{
	do_VIDIOC_QUERYBUF(SINT_MIN, V4L2_BUF_TYPE_VIDEO_CAPTURE, 2, -1);
	do_VIDIOC_QUERYBUF(0, V4L2_BUF_TYPE_VIDEO_CAPTURE, 2, -1);
	do_VIDIOC_QUERYBUF(V4L2_MEMORY_OVERLAY + 1, V4L2_BUF_TYPE_VIDEO_CAPTURE,
			   2, -1);
	do_VIDIOC_QUERYBUF(SINT_MAX, V4L2_BUF_TYPE_VIDEO_CAPTURE, 2, -1);
}

void test_VIDIOC_QUERYBUF_invalid_memory_output()
{
	do_VIDIOC_QUERYBUF(SINT_MIN, V4L2_BUF_TYPE_VIDEO_OUTPUT, 2, -1);
	do_VIDIOC_QUERYBUF(0, V4L2_BUF_TYPE_VIDEO_OUTPUT, 2, -1);
	do_VIDIOC_QUERYBUF(V4L2_MEMORY_OVERLAY + 1, V4L2_BUF_TYPE_VIDEO_OUTPUT,
			   2, -1);
	do_VIDIOC_QUERYBUF(SINT_MAX, V4L2_BUF_TYPE_VIDEO_OUTPUT, 2, -1);
}

void test_VIDIOC_QUERYBUF_invalid_type_mmap()
{
	do_VIDIOC_QUERYBUF(V4L2_MEMORY_MMAP, 0, 2, -1);
	do_VIDIOC_QUERYBUF(V4L2_MEMORY_MMAP, V4L2_BUF_TYPE_VIDEO_OVERLAY, 2,
			   -1);
	do_VIDIOC_QUERYBUF(V4L2_MEMORY_MMAP, V4L2_BUF_TYPE_VBI_CAPTURE, 2, -1);
	do_VIDIOC_QUERYBUF(V4L2_MEMORY_MMAP, V4L2_BUF_TYPE_VBI_OUTPUT, 2, -1);
	do_VIDIOC_QUERYBUF(V4L2_MEMORY_MMAP, V4L2_BUF_TYPE_SLICED_VBI_CAPTURE,
			   2, -1);
	do_VIDIOC_QUERYBUF(V4L2_MEMORY_MMAP, V4L2_BUF_TYPE_SLICED_VBI_OUTPUT, 2,
			   -1);
	do_VIDIOC_QUERYBUF(V4L2_MEMORY_MMAP, V4L2_BUF_TYPE_VIDEO_OUTPUT_OVERLAY,
			   2, -1);
	do_VIDIOC_QUERYBUF(V4L2_MEMORY_MMAP, V4L2_BUF_TYPE_PRIVATE - 1, 2, -1);
	do_VIDIOC_QUERYBUF(V4L2_MEMORY_MMAP, V4L2_BUF_TYPE_PRIVATE, 2, -1);
	do_VIDIOC_QUERYBUF(V4L2_MEMORY_MMAP, V4L2_BUF_TYPE_PRIVATE + 1, 2, -1);
	do_VIDIOC_QUERYBUF(V4L2_MEMORY_MMAP, S32_MAX, 2, -1);
	do_VIDIOC_QUERYBUF(V4L2_MEMORY_MMAP, (__s32) ((__u32) S32_MAX + 1), 2,
			   -1);
	do_VIDIOC_QUERYBUF(V4L2_MEMORY_MMAP, U32_MAX - 1, 2, -1);
	do_VIDIOC_QUERYBUF(V4L2_MEMORY_MMAP, U32_MAX, 2, -1);
}

void test_VIDIOC_QUERYBUF_invalid_type_userptr()
{
	do_VIDIOC_QUERYBUF(V4L2_MEMORY_USERPTR, 0, 2, -1);
	do_VIDIOC_QUERYBUF(V4L2_MEMORY_USERPTR, V4L2_BUF_TYPE_VIDEO_OVERLAY, 2,
			   -1);
	do_VIDIOC_QUERYBUF(V4L2_MEMORY_USERPTR, V4L2_BUF_TYPE_VBI_CAPTURE, 2,
			   -1);
	do_VIDIOC_QUERYBUF(V4L2_MEMORY_USERPTR, V4L2_BUF_TYPE_VBI_OUTPUT, 2,
			   -1);
	do_VIDIOC_QUERYBUF(V4L2_MEMORY_USERPTR,
			   V4L2_BUF_TYPE_SLICED_VBI_CAPTURE, 2, -1);
	do_VIDIOC_QUERYBUF(V4L2_MEMORY_USERPTR, V4L2_BUF_TYPE_SLICED_VBI_OUTPUT,
			   2, -1);
	do_VIDIOC_QUERYBUF(V4L2_MEMORY_USERPTR,
			   V4L2_BUF_TYPE_VIDEO_OUTPUT_OVERLAY, 2, -1);
	do_VIDIOC_QUERYBUF(V4L2_MEMORY_USERPTR, V4L2_BUF_TYPE_PRIVATE - 1, 2,
			   -1);
	do_VIDIOC_QUERYBUF(V4L2_MEMORY_USERPTR, V4L2_BUF_TYPE_PRIVATE, 2, -1);
	do_VIDIOC_QUERYBUF(V4L2_MEMORY_USERPTR, V4L2_BUF_TYPE_PRIVATE + 1, 2,
			   -1);
	do_VIDIOC_QUERYBUF(V4L2_MEMORY_USERPTR, S32_MAX, 2, -1);
	do_VIDIOC_QUERYBUF(V4L2_MEMORY_USERPTR, (__s32) ((__u32) S32_MAX + 1),
			   2, -1);
	do_VIDIOC_QUERYBUF(V4L2_MEMORY_USERPTR, U32_MAX - 1, 2, -1);
	do_VIDIOC_QUERYBUF(V4L2_MEMORY_USERPTR, U32_MAX, 2, -1);
}

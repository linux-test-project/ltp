/*
 * v4l-test: Test environment for Video For Linux Two API
 *
 *  7 May 2009  0.2  show_v4l2_*() function extracted to v4l2_show.c
 * 29 Apr 2009  0.1  First release
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

static void do_VIDIOC_REQBUFS_capture_mmap(__u32 count)
{
	int ret_cap, errno_cap;
	int ret_req, errno_req;
	struct v4l2_capability cap;
	struct v4l2_requestbuffers reqbuf;
	struct v4l2_requestbuffers reqbuf2;

	memset(&cap, 0, sizeof(cap));

	ret_cap = ioctl(get_video_fd(), VIDIOC_QUERYCAP, &cap);
	errno_cap = errno;

	memset(&reqbuf, 0xff, sizeof(reqbuf));
	reqbuf.count = count;
	reqbuf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	reqbuf.memory = V4L2_MEMORY_MMAP;

	ret_req = ioctl(get_video_fd(), VIDIOC_REQBUFS, &reqbuf);
	errno_req = errno;

	dprintf("\t%s:%u: VIDIOC_REQBUF, count=%u, ret_req=%i, errno_req=%i\n",
		__FILE__, __LINE__, count, ret_req, errno_req);

	if (ret_cap == 0 &&
	    (cap.capabilities & V4L2_CAP_STREAMING) &&
	    (cap.capabilities & V4L2_CAP_VIDEO_CAPTURE)) {

		CU_ASSERT_EQUAL(ret_cap, 0);
		CU_ASSERT(cap.capabilities & V4L2_CAP_STREAMING);

		CU_ASSERT_EQUAL(ret_req, 0);
		//CU_ASSERT_EQUAL(reqbuf.count, ???);
		CU_ASSERT_EQUAL(reqbuf.type, V4L2_BUF_TYPE_VIDEO_CAPTURE);
		CU_ASSERT_EQUAL(reqbuf.memory, V4L2_MEMORY_MMAP);
		CU_ASSERT_EQUAL(reqbuf.reserved[0], 0);
		CU_ASSERT_EQUAL(reqbuf.reserved[1], 0);

	} else {
		CU_ASSERT_EQUAL(ret_req, -1);
		CU_ASSERT_EQUAL(errno_req, EINVAL);

		memset(&reqbuf2, 0xff, sizeof(reqbuf2));
		reqbuf2.count = count;
		reqbuf2.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
		reqbuf2.memory = V4L2_MEMORY_MMAP;

		CU_ASSERT_EQUAL(memcmp(&reqbuf, &reqbuf2, sizeof(reqbuf)), 0);
	}

	if (ret_req == 0) {
		show_v4l2_requestbuffers(&reqbuf);
	}

}

void test_VIDIOC_REQBUFS_capture_mmap()
{
	do_VIDIOC_REQBUFS_capture_mmap(0);
	do_VIDIOC_REQBUFS_capture_mmap(1);
	do_VIDIOC_REQBUFS_capture_mmap(2);
	do_VIDIOC_REQBUFS_capture_mmap(3);
	do_VIDIOC_REQBUFS_capture_mmap(4);
	do_VIDIOC_REQBUFS_capture_mmap((__u32) S16_MIN);
	do_VIDIOC_REQBUFS_capture_mmap((__u32) S16_MAX);
	do_VIDIOC_REQBUFS_capture_mmap(U32_MAX);
	do_VIDIOC_REQBUFS_capture_mmap(0);
}

static void do_VIDIOC_REQBUFS_capture_userptr(__u32 count)
{
	int ret_cap, errno_cap;
	int ret_req, errno_req;
	struct v4l2_capability cap;
	struct v4l2_requestbuffers reqbuf;
	struct v4l2_requestbuffers reqbuf2;

	memset(&cap, 0, sizeof(cap));

	ret_cap = ioctl(get_video_fd(), VIDIOC_QUERYCAP, &cap);
	errno_cap = errno;

	memset(&reqbuf, 0xff, sizeof(reqbuf));
	reqbuf.count = count;
	reqbuf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	reqbuf.memory = V4L2_MEMORY_USERPTR;

	ret_req = ioctl(get_video_fd(), VIDIOC_REQBUFS, &reqbuf);
	errno_req = errno;

	dprintf("\t%s:%u: VIDIOC_REQBUF, count=%u, ret_req=%i, errno_req=%i\n",
		__FILE__, __LINE__, count, ret_req, errno_req);

	if (ret_cap == 0 &&
	    (cap.capabilities & V4L2_CAP_STREAMING) &&
	    (cap.capabilities & V4L2_CAP_VIDEO_CAPTURE) && ret_req == 0) {
		CU_ASSERT_EQUAL(ret_cap, 0);
		CU_ASSERT(cap.capabilities & V4L2_CAP_STREAMING);

		CU_ASSERT_EQUAL(ret_req, 0);
		//CU_ASSERT_EQUAL(reqbuf.count, ???);
		CU_ASSERT_EQUAL(reqbuf.type, V4L2_BUF_TYPE_VIDEO_CAPTURE);
		CU_ASSERT_EQUAL(reqbuf.memory, V4L2_MEMORY_USERPTR);
		CU_ASSERT_EQUAL(reqbuf.reserved[0], 0);
		CU_ASSERT_EQUAL(reqbuf.reserved[1], 0);

	} else {
		CU_ASSERT_EQUAL(ret_req, -1);
		CU_ASSERT_EQUAL(errno_req, EINVAL);

		memset(&reqbuf2, 0xff, sizeof(reqbuf2));
		reqbuf2.count = count;
		reqbuf2.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
		reqbuf2.memory = V4L2_MEMORY_USERPTR;

		CU_ASSERT_EQUAL(memcmp(&reqbuf, &reqbuf2, sizeof(reqbuf)), 0);
	}

	if (ret_req == 0) {
		show_v4l2_requestbuffers(&reqbuf);
	}

}

void test_VIDIOC_REQBUFS_capture_userptr()
{
	do_VIDIOC_REQBUFS_capture_userptr(0);
	do_VIDIOC_REQBUFS_capture_userptr(1);
	do_VIDIOC_REQBUFS_capture_userptr((__u32) S16_MIN);
	do_VIDIOC_REQBUFS_capture_userptr((__u32) S16_MAX);
	do_VIDIOC_REQBUFS_capture_userptr(U32_MAX);
	do_VIDIOC_REQBUFS_capture_userptr(0);
}

static void do_VIDIOC_REQBUFS_output_mmap(__u32 count)
{
	int ret_cap, errno_cap;
	int ret_req, errno_req;
	struct v4l2_capability cap;
	struct v4l2_requestbuffers reqbuf;
	struct v4l2_requestbuffers reqbuf2;

	memset(&cap, 0, sizeof(cap));

	ret_cap = ioctl(get_video_fd(), VIDIOC_QUERYCAP, &cap);
	errno_cap = errno;

	memset(&reqbuf, 0xff, sizeof(reqbuf));
	reqbuf.count = count;
	reqbuf.type = V4L2_BUF_TYPE_VIDEO_OUTPUT;
	reqbuf.memory = V4L2_MEMORY_MMAP;

	ret_req = ioctl(get_video_fd(), VIDIOC_REQBUFS, &reqbuf);
	errno_req = errno;

	dprintf("\t%s:%u: VIDIOC_REQBUF, count=%u, ret_req=%i, errno_req=%i\n",
		__FILE__, __LINE__, count, ret_req, errno_req);

	if (ret_cap == 0 &&
	    (cap.capabilities & V4L2_CAP_STREAMING) &&
	    (cap.capabilities & V4L2_CAP_VIDEO_OUTPUT)) {

		CU_ASSERT_EQUAL(ret_cap, 0);
		CU_ASSERT(cap.capabilities & V4L2_CAP_STREAMING);

		CU_ASSERT_EQUAL(ret_req, 0);
		//CU_ASSERT_EQUAL(reqbuf.count, ???);
		CU_ASSERT_EQUAL(reqbuf.type, V4L2_BUF_TYPE_VIDEO_OUTPUT);
		CU_ASSERT_EQUAL(reqbuf.memory, V4L2_MEMORY_MMAP);
		CU_ASSERT_EQUAL(reqbuf.reserved[0], 0);
		CU_ASSERT_EQUAL(reqbuf.reserved[1], 0);

	} else {
		CU_ASSERT_EQUAL(ret_req, -1);
		CU_ASSERT_EQUAL(errno_req, EINVAL);

		memset(&reqbuf2, 0xff, sizeof(reqbuf2));
		reqbuf2.count = count;
		reqbuf2.type = V4L2_BUF_TYPE_VIDEO_OUTPUT;
		reqbuf2.memory = V4L2_MEMORY_MMAP;

		CU_ASSERT_EQUAL(memcmp(&reqbuf, &reqbuf2, sizeof(reqbuf)), 0);
	}

	if (ret_req == 0) {
		show_v4l2_requestbuffers(&reqbuf);
	}

}

void test_VIDIOC_REQBUFS_output_mmap()
{
	do_VIDIOC_REQBUFS_output_mmap(0);
	do_VIDIOC_REQBUFS_output_mmap(1);
	do_VIDIOC_REQBUFS_output_mmap(2);
	do_VIDIOC_REQBUFS_output_mmap(3);
	do_VIDIOC_REQBUFS_output_mmap(4);
	do_VIDIOC_REQBUFS_output_mmap((__u32) S16_MIN);
	do_VIDIOC_REQBUFS_output_mmap((__u32) S16_MAX);
	do_VIDIOC_REQBUFS_output_mmap(U32_MAX);
	do_VIDIOC_REQBUFS_output_mmap(0);
}

static void do_VIDIOC_REQBUFS_output_userptr(__u32 count)
{
	int ret_cap, errno_cap;
	int ret_req, errno_req;
	struct v4l2_capability cap;
	struct v4l2_requestbuffers reqbuf;
	struct v4l2_requestbuffers reqbuf2;

	memset(&cap, 0, sizeof(cap));

	ret_cap = ioctl(get_video_fd(), VIDIOC_QUERYCAP, &cap);
	errno_cap = errno;

	memset(&reqbuf, 0xff, sizeof(reqbuf));
	reqbuf.count = count;
	reqbuf.type = V4L2_BUF_TYPE_VIDEO_OUTPUT;
	reqbuf.memory = V4L2_MEMORY_USERPTR;

	ret_req = ioctl(get_video_fd(), VIDIOC_REQBUFS, &reqbuf);
	errno_req = errno;

	dprintf("\t%s:%u: VIDIOC_REQBUF, count=%u, ret_req=%i, errno_req=%i\n",
		__FILE__, __LINE__, count, ret_req, errno_req);

	if (ret_cap == 0 &&
	    (cap.capabilities & V4L2_CAP_STREAMING) &&
	    (cap.capabilities & V4L2_CAP_VIDEO_OUTPUT) && ret_req == 0) {
		CU_ASSERT_EQUAL(ret_cap, 0);
		CU_ASSERT(cap.capabilities & V4L2_CAP_STREAMING);

		CU_ASSERT_EQUAL(ret_req, 0);
		//CU_ASSERT_EQUAL(reqbuf.count, ???);
		CU_ASSERT_EQUAL(reqbuf.type, V4L2_BUF_TYPE_VIDEO_OUTPUT);
		CU_ASSERT_EQUAL(reqbuf.memory, V4L2_MEMORY_USERPTR);
		CU_ASSERT_EQUAL(reqbuf.reserved[0], 0);
		CU_ASSERT_EQUAL(reqbuf.reserved[1], 0);

	} else {
		CU_ASSERT_EQUAL(ret_req, -1);
		CU_ASSERT_EQUAL(errno_req, EINVAL);

		memset(&reqbuf2, 0xff, sizeof(reqbuf2));
		reqbuf2.count = count;
		reqbuf2.type = V4L2_BUF_TYPE_VIDEO_OUTPUT;
		reqbuf2.memory = V4L2_MEMORY_USERPTR;

		CU_ASSERT_EQUAL(memcmp(&reqbuf, &reqbuf2, sizeof(reqbuf)), 0);
	}

	if (ret_req == 0) {
		show_v4l2_requestbuffers(&reqbuf);
	}

}

void test_VIDIOC_REQBUFS_output_userptr()
{
	do_VIDIOC_REQBUFS_output_userptr(0);
	do_VIDIOC_REQBUFS_output_userptr(1);
	do_VIDIOC_REQBUFS_output_userptr((__u32) S16_MIN);
	do_VIDIOC_REQBUFS_output_userptr((__u32) S16_MAX);
	do_VIDIOC_REQBUFS_output_userptr(U32_MAX);
	do_VIDIOC_REQBUFS_output_userptr(0);
}

static void do_VIDIOC_REQBUFS_invalid_memory(enum v4l2_buf_type type,
					     enum v4l2_memory memory)
{
	int ret_req, errno_req;
	struct v4l2_requestbuffers reqbuf;
	struct v4l2_requestbuffers reqbuf2;

	memset(&reqbuf, 0xff, sizeof(reqbuf));
	reqbuf.count = 0;
	reqbuf.type = type;
	reqbuf.memory = memory;

	ret_req = ioctl(get_video_fd(), VIDIOC_REQBUFS, &reqbuf);
	errno_req = errno;

	dprintf("\t%s:%u: VIDIOC_REQBUF, type=0%x, ret_req=%i, errno_req=%i\n",
		__FILE__, __LINE__, type, ret_req, errno_req);

	CU_ASSERT_EQUAL(ret_req, -1);
	CU_ASSERT_EQUAL(errno_req, EINVAL);

	memset(&reqbuf2, 0xff, sizeof(reqbuf2));
	reqbuf2.count = 0;
	reqbuf2.type = type;
	reqbuf2.memory = memory;

	CU_ASSERT_EQUAL(memcmp(&reqbuf, &reqbuf2, sizeof(reqbuf)), 0);

	if (ret_req == 0) {
		show_v4l2_requestbuffers(&reqbuf);
	}

}

void test_VIDIOC_REQBUFS_invalid_memory_capture()
{
	do_VIDIOC_REQBUFS_invalid_memory(V4L2_BUF_TYPE_VIDEO_CAPTURE, SINT_MIN);
	do_VIDIOC_REQBUFS_invalid_memory(V4L2_BUF_TYPE_VIDEO_CAPTURE, 0);
	do_VIDIOC_REQBUFS_invalid_memory(V4L2_BUF_TYPE_VIDEO_CAPTURE,
					 V4L2_MEMORY_OVERLAY);
	do_VIDIOC_REQBUFS_invalid_memory(V4L2_BUF_TYPE_VIDEO_CAPTURE,
					 V4L2_MEMORY_OVERLAY + 1);
	do_VIDIOC_REQBUFS_invalid_memory(V4L2_BUF_TYPE_VIDEO_CAPTURE, SINT_MAX);
}

void test_VIDIOC_REQBUFS_invalid_memory_output()
{
	do_VIDIOC_REQBUFS_invalid_memory(V4L2_BUF_TYPE_VIDEO_OUTPUT, SINT_MIN);
	do_VIDIOC_REQBUFS_invalid_memory(V4L2_BUF_TYPE_VIDEO_OUTPUT, 0);
	do_VIDIOC_REQBUFS_invalid_memory(V4L2_BUF_TYPE_VIDEO_OUTPUT,
					 V4L2_MEMORY_OVERLAY);
	do_VIDIOC_REQBUFS_invalid_memory(V4L2_BUF_TYPE_VIDEO_OUTPUT,
					 V4L2_MEMORY_OVERLAY + 1);
	do_VIDIOC_REQBUFS_invalid_memory(V4L2_BUF_TYPE_VIDEO_OUTPUT, SINT_MAX);
}

static void do_VIDIOC_REQBUFS_invalid_type(enum v4l2_memory memory,
					   enum v4l2_buf_type type)
{
	int ret_req, errno_req;
	struct v4l2_requestbuffers reqbuf;
	struct v4l2_requestbuffers reqbuf2;
	__u32 count;

	count = 1;

	memset(&reqbuf, 0xff, sizeof(reqbuf));
	reqbuf.count = count;
	reqbuf.type = type;
	reqbuf.memory = memory;

	ret_req = ioctl(get_video_fd(), VIDIOC_REQBUFS, &reqbuf);
	errno_req = errno;

	dprintf
	    ("\t%s:%u: VIDIOC_REQBUF, type=0x%x, memory=%i, ret_req=%i, errno_req=%i\n",
	     __FILE__, __LINE__, type, memory, ret_req, errno_req);

	CU_ASSERT_EQUAL(ret_req, -1);
	CU_ASSERT_EQUAL(errno_req, EINVAL);

	memset(&reqbuf2, 0xff, sizeof(reqbuf2));
	reqbuf2.count = count;
	reqbuf2.type = type;
	reqbuf2.memory = memory;

	CU_ASSERT_EQUAL(memcmp(&reqbuf, &reqbuf2, sizeof(reqbuf)), 0);

	if (ret_req == 0) {
		show_v4l2_requestbuffers(&reqbuf);
	}
}

void test_VIDIOC_REQUBUFS_invalid_type_mmap()
{
	do_VIDIOC_REQBUFS_invalid_type(V4L2_MEMORY_MMAP, 0);
	do_VIDIOC_REQBUFS_invalid_type(V4L2_MEMORY_MMAP,
				       V4L2_BUF_TYPE_VIDEO_OVERLAY);
	do_VIDIOC_REQBUFS_invalid_type(V4L2_MEMORY_MMAP,
				       V4L2_BUF_TYPE_VBI_CAPTURE);
	do_VIDIOC_REQBUFS_invalid_type(V4L2_MEMORY_MMAP,
				       V4L2_BUF_TYPE_VBI_OUTPUT);
	do_VIDIOC_REQBUFS_invalid_type(V4L2_MEMORY_MMAP,
				       V4L2_BUF_TYPE_SLICED_VBI_CAPTURE);
	do_VIDIOC_REQBUFS_invalid_type(V4L2_MEMORY_MMAP,
				       V4L2_BUF_TYPE_SLICED_VBI_OUTPUT);
	do_VIDIOC_REQBUFS_invalid_type(V4L2_MEMORY_MMAP,
				       V4L2_BUF_TYPE_VIDEO_OUTPUT_OVERLAY);
	do_VIDIOC_REQBUFS_invalid_type(V4L2_MEMORY_MMAP,
				       V4L2_BUF_TYPE_PRIVATE - 1);
	do_VIDIOC_REQBUFS_invalid_type(V4L2_MEMORY_MMAP, V4L2_BUF_TYPE_PRIVATE);
	do_VIDIOC_REQBUFS_invalid_type(V4L2_MEMORY_MMAP,
				       V4L2_BUF_TYPE_PRIVATE + 1);
	do_VIDIOC_REQBUFS_invalid_type(V4L2_MEMORY_MMAP, S32_MAX);
	do_VIDIOC_REQBUFS_invalid_type(V4L2_MEMORY_MMAP,
				       (__s32) ((__u32) S32_MAX + 1));
	do_VIDIOC_REQBUFS_invalid_type(V4L2_MEMORY_MMAP, U32_MAX - 1);
	do_VIDIOC_REQBUFS_invalid_type(V4L2_MEMORY_MMAP, U32_MAX);

}

void test_VIDIOC_REQUBUFS_invalid_type_userptr()
{
	do_VIDIOC_REQBUFS_invalid_type(V4L2_MEMORY_USERPTR, 0);
	do_VIDIOC_REQBUFS_invalid_type(V4L2_MEMORY_USERPTR,
				       V4L2_BUF_TYPE_VIDEO_OVERLAY);
	do_VIDIOC_REQBUFS_invalid_type(V4L2_MEMORY_USERPTR,
				       V4L2_BUF_TYPE_VBI_CAPTURE);
	do_VIDIOC_REQBUFS_invalid_type(V4L2_MEMORY_USERPTR,
				       V4L2_BUF_TYPE_VBI_OUTPUT);
	do_VIDIOC_REQBUFS_invalid_type(V4L2_MEMORY_USERPTR,
				       V4L2_BUF_TYPE_SLICED_VBI_CAPTURE);
	do_VIDIOC_REQBUFS_invalid_type(V4L2_MEMORY_USERPTR,
				       V4L2_BUF_TYPE_SLICED_VBI_OUTPUT);
	do_VIDIOC_REQBUFS_invalid_type(V4L2_MEMORY_USERPTR,
				       V4L2_BUF_TYPE_VIDEO_OUTPUT_OVERLAY);
	do_VIDIOC_REQBUFS_invalid_type(V4L2_MEMORY_USERPTR,
				       V4L2_BUF_TYPE_PRIVATE - 1);
	do_VIDIOC_REQBUFS_invalid_type(V4L2_MEMORY_USERPTR,
				       V4L2_BUF_TYPE_PRIVATE);
	do_VIDIOC_REQBUFS_invalid_type(V4L2_MEMORY_USERPTR,
				       V4L2_BUF_TYPE_PRIVATE + 1);
	do_VIDIOC_REQBUFS_invalid_type(V4L2_MEMORY_USERPTR, S32_MAX);
	do_VIDIOC_REQBUFS_invalid_type(V4L2_MEMORY_USERPTR,
				       (__s32) ((__u32) S32_MAX + 1));
	do_VIDIOC_REQBUFS_invalid_type(V4L2_MEMORY_USERPTR, U32_MAX - 1);
	do_VIDIOC_REQBUFS_invalid_type(V4L2_MEMORY_USERPTR, U32_MAX);

}

/*
 * v4l-test: Test environment for Video For Linux Two API
 *
 *  7 May 2009  0.1  First release
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

#include "test_VIDIOC_REQBUFS.h"

void show_v4l2_requestbuffers(struct v4l2_requestbuffers *reqbuf) {
	dprintf("\treqbuf = { "
		".count=%u, "
		".type=%i, "
		".memory=%i, "
		".reserved = { 0x%X, 0x%X } "
		"}\n",
		reqbuf->count,
		reqbuf->type,
		reqbuf->memory,
		reqbuf->reserved[0],
		reqbuf->reserved[1]
	);
}

void show_v4l2_buffer(struct v4l2_buffer *buf) {
	unsigned int i;

	dprintf("\tbuf = { "
		".index=%u, "
		".type=%i, "
		".bytesused=%u, "
		".flags=0x%x, "
		".field=%i, "
		".timestamp = { tv_sec=%lu, tv_usec=%lu }, "
		".timecode = { "
		".type=%u, "
		".flags=0x%x, "
		".frames=%u, "
		".seconds=%u, "
		".minutes=%u, "
		".hours=%u, "
		".userbits = { 0x%x, 0x%x, 0x%x, 0x%x } "
		" }, "
		".sequence=%u, "
		".memory=%i, ",
		buf->index,
		buf->type,
		buf->bytesused,
		buf->flags,
		buf->field,
		buf->timestamp.tv_sec,
		buf->timestamp.tv_usec,
		buf->timecode.type,
		buf->timecode.flags,
		buf->timecode.frames,
		buf->timecode.seconds,
		buf->timecode.minutes,
		buf->timecode.hours,
		buf->timecode.userbits[0],
		buf->timecode.userbits[1],
		buf->timecode.userbits[2],
		buf->timecode.userbits[3],
		buf->sequence,
		buf->memory
	);

	switch (buf->memory) {
		case V4L2_MEMORY_USERPTR:
			dprintf(".m.userptr=0x%lx, ",
				buf->m.userptr);
			for (i = sizeof(buf->m.userptr); i < sizeof(buf->m); i++) {
				dprintf("((__u8*)&.m)[%u]=0x%x, ",
					i, ((__u8*)&buf->m)[i]);
			}
			break;
		case V4L2_MEMORY_MMAP:
		case V4L2_MEMORY_OVERLAY:
		default:
			dprintf(".m.offset=%u, ",
				buf->m.offset);
			for (i = sizeof(buf->m.offset); i < sizeof(buf->m); i++) {
				dprintf("((__u8*)&.m)[%u]=0x%x, ",
					i, ((__u8*)&buf->m)[i]);
			}
	}

	dprintf(".length=%u, "
		".input=%u, "
		".reserved=0x%x "
		"}\n",
		buf->length,
		buf->input,
		buf->reserved
	);

}

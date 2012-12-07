/*
 * v4l-test: Test environment for Video For Linux Two API
 *
 * 17 Jul 2009  0.3  show_v4l2_frmsizeenum() added
 *  5 Jul 2009  0.2  show_v4l2_input() introduced
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

void show_v4l2_requestbuffers(struct v4l2_requestbuffers *reqbuf)
{
	dprintf("\treqbuf = { "
		".count=%u, "
		".type=%i, "
		".memory=%i, "
		".reserved = { 0x%X, 0x%X } "
		"}\n",
		reqbuf->count,
		reqbuf->type,
		reqbuf->memory, reqbuf->reserved[0], reqbuf->reserved[1]
	    );
}

void show_v4l2_buffer(struct v4l2_buffer *buf)
{
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
		buf->timecode.userbits[3], buf->sequence, buf->memory);

	switch (buf->memory) {
	case V4L2_MEMORY_USERPTR:
		dprintf(".m.userptr=0x%lx, ", buf->m.userptr);
		for (i = sizeof(buf->m.userptr); i < sizeof(buf->m); i++) {
			dprintf("((__u8*)&.m)[%u]=0x%x, ",
				i, ((__u8 *) & buf->m)[i]);
		}
		break;
	case V4L2_MEMORY_MMAP:
	case V4L2_MEMORY_OVERLAY:
	default:
		dprintf(".m.offset=%u, ", buf->m.offset);
		for (i = sizeof(buf->m.offset); i < sizeof(buf->m); i++) {
			dprintf("((__u8*)&.m)[%u]=0x%x, ",
				i, ((__u8 *) & buf->m)[i]);
		}
	}

	dprintf(".length=%u, "
		".input=%u, "
		".reserved=0x%x "
		"}\n", buf->length, buf->input, buf->reserved);

}

void show_v4l2_input(struct v4l2_input *input)
{
	dprintf("\tinput = {.index=%u, .name=\"%s\", "
		".type=0x%X, .audioset=0x%X, .tuner=0x%X, "
		".std=%llX, "
		".status=0x%X, "
		".reserved[]={ 0x%X, 0x%X, 0x%X, 0x%X } }\n",
		input->index,
		input->name,
		input->type,
		input->audioset,
		input->tuner,
		input->std,
		input->status,
		input->reserved[0],
		input->reserved[1], input->reserved[2], input->reserved[3]
	    );
}

void show_v4l2_frmsizeenum(struct v4l2_frmsizeenum *framesize)
{
	dprintf("\tframesize = { .index=%u, "
		".pixel_format=0x%x, "
		".type=%u, ",
		framesize->index, framesize->pixel_format, framesize->type);

	switch (framesize->type) {
	case V4L2_FRMSIZE_TYPE_DISCRETE:
		dprintf(".discrete = { .width=%u, heigth=%u }, ",
			framesize->discrete.width, framesize->discrete.height);
		break;
	case V4L2_FRMSIZE_TYPE_CONTINUOUS:
	case V4L2_FRMSIZE_TYPE_STEPWISE:
		dprintf(".stepwise = { .min_width=%u, "
			".max_width=%u, "
			".step_width=%u, "
			".min_height=%u, "
			".max_height=%u, "
			".step_height=%u }, ",
			framesize->stepwise.min_width,
			framesize->stepwise.max_width,
			framesize->stepwise.step_width,
			framesize->stepwise.min_height,
			framesize->stepwise.max_height,
			framesize->stepwise.step_height);
		break;
	default:
		;
	}

	dprintf(".reserved = { 0x%x, 0x%x } }\n",
		framesize->reserved[0], framesize->reserved[1]
	    );

}

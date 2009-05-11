/*
 * v4l-test: Test environment for Video For Linux Two API
 *
 *  5 May 2009  0.1  First release
 *
 * Written by Márton Németh <nm127@freemail.hu>
 * Released under GPL
 */

void test_VIDIOC_QUERYBUF_capture_mmap(void);
void test_VIDIOC_QUERYBUF_capture_userptr(void);
void test_VIDIOC_QUERYBUF_output_mmap(void);
void test_VIDIOC_QUERYBUF_output_userptr(void);
void test_VIDIOC_QUERYBUF_overlay_capture(void);
void test_VIDIOC_QUERYBUF_overlay_output(void);
void test_VIDIOC_QUERYBUF_invalid_memory_capture(void);
void test_VIDIOC_QUERYBUF_invalid_memory_output(void);
void test_VIDIOC_QUERYBUF_invalid_type_mmap(void);
void test_VIDIOC_QUERYBUF_invalid_type_userptr(void);

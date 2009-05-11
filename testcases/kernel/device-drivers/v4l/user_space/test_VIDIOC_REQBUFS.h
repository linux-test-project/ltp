/*
 * v4l-test: Test environment for Video For Linux Two API
 *
 * 29 Apr 2009  0.1  First release
 *
 * Written by Márton Németh <nm127@freemail.hu>
 * Released under GPL
 */

void test_VIDIOC_REQBUFS_capture_mmap(void);
void test_VIDIOC_REQBUFS_capture_userptr(void);

void test_VIDIOC_REQBUFS_output_mmap(void);
void test_VIDIOC_REQBUFS_output_userptr(void);

void test_VIDIOC_REQBUFS_invalid_memory_capture(void);
void test_VIDIOC_REQBUFS_invalid_memory_output(void);

void test_VIDIOC_REQUBUFS_invalid_type_mmap(void);
void test_VIDIOC_REQUBUFS_invalid_type_userptr(void);

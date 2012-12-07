
#include <linux/module.h>
#include <linux/version.h>
#include <linux/fs.h>
#include <linux/videodev2.h>
#include <media/v4l2-ioctl.h>
#include <media/v4l2-dev.h>

struct dummy_dev {
	struct video_device *vfd;
};

static int dummy_open(struct inode *inode, struct file *file)
{
	int minor = iminor(inode);

	printk(KERN_DEBUG "video_dummy: open called (minor=%d)\n", minor);

	return 0;
}

static int dummy_close(struct inode *inode, struct file *file)
{
	int minor = iminor(inode);

	printk(KERN_DEBUG "video_dummy: close called (minor=%d)\n", minor);

	return 0;
}

static int vidioc_querycap(struct file *file, void *priv,
			   struct v4l2_capability *cap)
{
	strcpy(cap->driver, "dummy");
	strcpy(cap->card, "dummy");
	cap->version = KERNEL_VERSION(0, 0, 1);
	cap->capabilities = 0;
	return 0;
}

static struct file_operations dummy_fops = {
	.owner = THIS_MODULE,
	.open = dummy_open,
	.release = dummy_close,
	.ioctl = video_ioctl2,	/* V4L2 ioctl handler */

#ifdef CONFIG_COMPAT
/*
 * V4L/DVB (10139): v4l: rename v4l_compat_ioctl32 to v4l2_compat_ioctl32
 * http://git.kernel.org/?p=linux/kernel/git/torvalds/linux-2.6.git;a=commit;h=9bb7cde793f0637cfbdd21c04050ffcef33a5624
 */
#if LINUX_VERSION_CODE < KERNEL_VERSION(2, 6, 19)
	.compat_ioctl = v4l_compat_ioctl32,
#else
	.compat_ioctl = v4l2_compat_ioctl32,
#endif
#endif
};

static const struct v4l2_ioctl_ops dummy_ioctl_ops = {
	.vidioc_querycap = vidioc_querycap,
};

static const struct video_device dummy_template = {
	.name = "dummy",
	.fops = &dummy_fops,
	.ioctl_ops = &dummy_ioctl_ops,
	.minor = -1,
	.release = video_device_release,

	.tvnorms = V4L2_STD_525_60,
	.current_norm = V4L2_STD_NTSC_M,
};

static struct video_device *dummy_vfd = NULL;

static int __init video_dummy_init(void)
{
	int ret;

	dummy_vfd = video_device_alloc();
	if (!dummy_vfd)
		return -ENOMEM;

	*dummy_vfd = dummy_template;

	ret = video_register_device(dummy_vfd, VFL_TYPE_GRABBER, -1);
	if (ret < 0) {
		video_device_release(dummy_vfd);
		dummy_vfd = NULL;
		return ret;
	}

	printk(KERN_INFO
	       "video_dummy: V4L2 device registered as /dev/video%d\n",
	       dummy_vfd->num);

	return 0;
}

static void __exit video_dummy_exit(void)
{

	printk(KERN_INFO "video_dummy: removing /dev/video%d\n",
	       dummy_vfd->num);
	video_unregister_device(dummy_vfd);
	dummy_vfd = NULL;

}

module_init(video_dummy_init);
module_exit(video_dummy_exit);

MODULE_DESCRIPTION("Dummy video module");
MODULE_AUTHOR("Márton Németh <nm127@freemail.hu>");
MODULE_LICENSE("GPL");

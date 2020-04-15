/*
 * Copyright (c) 2014-2016 Cyril Hrubis <chrubis@suse.cz>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef OLD_DEVICE_H__
#define OLD_DEVICE_H__

/*
 * Returns filesystem type to be used for the testing. Unless your test is
 * designed for specific filesystem you should use this function to the tested
 * filesystem.
 *
 * If TST_DEV_FS_TYPE is set the function returns it's content,
 * otherwise default fs type hardcoded in the library is returned.
 */
const char *tst_dev_fs_type(void);

/*
 * Acquires test device.
 *
 * Can be used only once, i.e. you cannot get two different devices.
 *
 * Looks for LTP_DEV env variable first (which may be passed by the test
 * driver or by a user) and returns just it's value if found.
 *
 * Otherwise creates a temp file and loop device.
 *
 * Note that you have to call tst_tmpdir() beforehand.
 *
 * Returns path to the device or NULL if it cannot be created.
 * Call tst_release_device() when you're done.
 */
const char *tst_acquire_device_(void (cleanup_fn)(void), unsigned int size);

const char *tst_acquire_device__(unsigned int size);

static inline const char *tst_acquire_device(void (cleanup_fn)(void))
{
	return tst_acquire_device_(cleanup_fn, 0);
}

/*
 * Acquire a loop device with specified temp filename. This function allows
 * you to acquire multiple devices at the same time. LTP_DEV is ignored.
 * If you call this function directly, use tst_detach_device() to release
 * the devices. tst_release_device() will not work correctly.
 *
 * The return value points to a static buffer and additional calls of
 * tst_acquire_loop_device() or tst_acquire_device() will overwrite it.
 */
const char *tst_acquire_loop_device(unsigned int size, const char *filename);

/*
 * @dev: device path returned by the tst_acquire_device()
 */
int tst_release_device(const char *dev);

/*
 * Cleanup function for tst_acquire_loop_device(). If you have acquired
 * a device using tst_acquire_device(), use tst_release_device() instead.
 * @dev: device path returned by the tst_acquire_loop_device()
 */
int tst_detach_device(const char *dev);

/*
 * Just like umount() but retries several times on failure.
 * @path: Path to umount
 */
int tst_umount(const char *path);

#endif	/* OLD_DEVICE_H__ */

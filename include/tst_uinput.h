// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2019 Cyril Hrubis <chrubis@suse.cz>
 */

#ifndef TST_UINPUT_H__
#define TST_UINPUT_H__

/**
 * Tries to open the uinput device.
 *
 * Returns file descriptor on success, -1 on failure.
 */
int open_uinput(void);

/**
 * Creates virtual input device.
 *
 * @fd File descriptor returned by open_uinput().
 */
void create_input_device(int fd);

/**
 * Parses /proc/bus/input/devices and returns the handlers strings for our
 * virtual device, which is list of input devices that receive events from the
 * device separated by whitestpaces.
 *
 * Returns newly allocated string, list of handlers separated by whitespaces,
 * or NULL in a case of failure.
 */
char *get_input_handlers(void);

/**
 * Sets up the virtual device to appear as a mouse, this must be called before
 * the call to create_input_device().
 *
 * @fd File descriptor as returned by open_uinput().
 */
void setup_mouse_events(int fd);

/**
 * Destroys virtual input device.
 *
 * @fd File descriptor returned by open_uinput().
 */
void destroy_input_device(int fd);

#endif	/* TST_UINPUT_H__ */

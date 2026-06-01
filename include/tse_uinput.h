// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2019 Cyril Hrubis <chrubis@suse.cz>
 */

#ifndef TSE_UINPUT_H__
#define TSE_UINPUT_H__

/**
 * open_uinput() - Try to open the uinput device.
 *
 * Return: File descriptor on success, -1 on failure.
 */
int open_uinput(void);

/**
 * create_input_device() - Create a virtual input device.
 * @fd: File descriptor returned by open_uinput().
 */
void create_input_device(int fd);

/**
 * get_input_field_value() - Get a field string for the virtual device.
 * @field: Field character ('H' for handlers, 'S' for sysfs).
 *
 * Parses /proc/bus/input/devices and returns the string for our
 * virtual device matching the requested field.
 *
 * Return: Newly allocated string, or NULL on failure.
 */
char *get_input_field_value(char field);

/**
 * setup_mouse_events() - Set up the virtual device as a mouse.
 * @fd: File descriptor as returned by open_uinput().
 *
 * Must be called before create_input_device().
 */
void setup_mouse_events(int fd);

/**
 * destroy_input_device() - Destroy a virtual input device.
 * @fd: File descriptor returned by open_uinput().
 */
void destroy_input_device(int fd);

#endif	/* TSE_UINPUT_H__ */

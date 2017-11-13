/*
 * Copyright (c) 2015 Cedric Hnyda <chnyda@suse.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it would be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write the Free Software Foundation,
 * Inc.,  51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 */

#ifndef INPUT_HELPER_H
#define INPUT_HELPER_H

#include <sys/types.h>
#include <dirent.h>

int open_device(void);
void send_rel_move(int fd, int x, int y);
void send_event(int fd, int event, int code, int value);
int open_uinput(void);
void create_device(int fd);
void setup_mouse_events(int fd);
void destroy_device(int fd);
int check_event_code(struct input_event *iev, int event, int code);
int check_sync_event(struct input_event *iev);
int no_events_queued(int fd, int stray_sync_event);

#endif /* INPUT_HELPER_H */

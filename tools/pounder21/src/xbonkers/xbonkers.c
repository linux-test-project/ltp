/*
 * Add more stress to X server by moving, resizing and activating windows
 * Author: Darrick Wong <djwong@us.ibm.com>
 */

/*
 * Copyright (C) 2003-2006 IBM
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA
 * 02111-1307, USA.
 */

#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <X11/Xlib.h>
#include <X11/Xatom.h>
#include <string.h>
#include <stdint.h>
#include <limits.h>

static int enable_fullscreen = 0;

#define MAX_PROPERTY_VALUE_LEN	4096
#define _NET_WM_STATE_TOGGLE	2
#define SLIDE_THRESHOLD 	32

/* We assume that the workspace number will either be -1 or some
 * huge number for "On All Workspaces" windows.  Presumably there
 * aren't 1,000,000 workspaces, so that should be a safe number.
 */
#define DESKTOP_MAX		1000000

#define ACTION_MOVE_WINDOW		0
#define ACTION_ACTIVATE_WINDOW		1
#define ACTION_MAXIMIZE_WINDOW		2
#define ACTION_FULLSCREEN_WINDOW	3
#define ACTION_HIDDEN_WINDOW		4
#define ACTION_SLIDE_WINDOW_0		5
#define ACTION_SLIDE_WINDOW_1		6
#define ACTION_SLIDE_WINDOW_2		7
#define ACTION_SLIDE_WINDOW_3		8
#define ACTION_SLIDE_WINDOW_4		9
#define ACTION_SLIDE_WINDOW_5		10
#define ACTION_MIN		ACTION_MOVE_WINDOW
#define ACTION_MAX		ACTION_SLIDE_WINDOW_5

/* The goal of this program:
 * 0. Seed random number generator
 * 1. Grab the list of windows and the desktop size.
 * 2. Filter out the panel/desktop/whatever.  We're going to make
 *    a cheesy assumption that a window on desktop -1 should be left
 *    alone.  (Actually, the -1 denotes "all desktops")
 * 3. For each window:
 *    a. Figure out what we're going to do--activate, move/resize,
 *       or maximize it.
 *    b. If we're going to move/resize, grab 4 random numbers.
 *    c. Actually perform the action.
 * 4. Every so often, jump back to (2) in case there are new windows.
 *    Maybe every 10,000 moves or so.
 *
 * Note that you do NOT want to run this on any X session you care about.
 * It shouldn't take down X, but YMMV and in any case mad window resizing
 * makes it hard to get work done.
 */
static int seed_random(void);
static int get_desktop_size(Display * disp, unsigned long *w, unsigned long *h);
static char *get_property(Display * disp, Window win, Atom xa_prop_type,
			  char *prop_name, unsigned long *size,
			  unsigned long *items);
static void go_bonkers(Display * disp, unsigned long iterations,
		       unsigned long sleep);
static Window *get_interesting_windows(Display * disp,
				       unsigned long *num_windows);
static Window *get_client_list(Display * disp, unsigned long *size,
			       unsigned long *items);
static long get_randnum(long min, long max);
static int send_client_msg(Display * disp, Window win, char *msg,
			   unsigned long data0, unsigned long data1,
			   unsigned long data2, unsigned long data3,
			   unsigned long data4);
static int activate_window(Display * disp, Window * win);
static int wm_supports(Display * disp, const char *prop);
static void move_window(Display * disp, Window * win, unsigned long desk_w,
			unsigned long desk_h);
static int toggle_property(Display * disp, Window * win, const char *property);
static inline unsigned long clamp_value(unsigned long value,
					unsigned long min, unsigned long max);
static int ignore_xlib_error(Display * disp, XErrorEvent * xee);

/* Actual functions begin here. */

static int seed_random(void)
{
	int fp;
	long seed;

	fp = open("/dev/urandom", O_RDONLY);
	if (fp < 0) {
		perror("/dev/urandom");
		return 0;
	}

	if (read(fp, &seed, sizeof(seed)) != sizeof(seed)) {
		perror("read random seed");
		return 0;
	}

	close(fp);
	srand(seed);

	return 1;
}

static int get_desktop_size(Display * disp, unsigned long *w, unsigned long *h)
{
	*w = DisplayWidth(disp, 0);
	*h = DisplayHeight(disp, 0);

	return 1;
}

static char *get_property(Display * disp, Window win, Atom xa_prop_type,
			  char *prop_name, unsigned long *size,
			  unsigned long *items)
{
	Atom xa_prop_name;
	Atom xa_ret_type;
	int ret_format;
	unsigned long ret_nitems;
	unsigned long ret_bytes_after;
	unsigned long tmp_size;
	unsigned char *ret_prop;
	char *ret;

	xa_prop_name = XInternAtom(disp, prop_name, False);

	if (XGetWindowProperty
	    (disp, win, xa_prop_name, 0, MAX_PROPERTY_VALUE_LEN / 4, False,
	     xa_prop_type, &xa_ret_type, &ret_format, &ret_nitems,
	     &ret_bytes_after, &ret_prop) != Success) {
		fprintf(stderr, "Cannot get %s property.\n", prop_name);
		return NULL;
	}

	if (xa_ret_type != xa_prop_type) {
		fprintf(stderr, "Invalid type of %s property.\n", prop_name);
		XFree(ret_prop);
		return NULL;
	}

	/* XXX: EVIL HACK to get around a bug when sizeof(Window) is 8 yet ret_format
	 * is listed as 32bits and we're trying to get the client list.  Just double
	 * ret_format and proceed. */
	if (ret_format == 32 && strcmp(prop_name, "_NET_CLIENT_LIST") == 0 &&
	    sizeof(Window) == 8) {
		ret_format *= 2;
	}

	/* null terminate the result to make string handling easier */
	tmp_size = (ret_format / 8) * ret_nitems;
	ret = calloc(tmp_size + 1, 1);
	if (!ret) {
		perror("get_property malloc failed");
		return NULL;
	}
	memcpy(ret, ret_prop, tmp_size);
	ret[tmp_size] = '\0';

	if (size) {
		*size = ret_format / 8;
	}
	if (items) {
		*items = ret_nitems;
	}

	XFree(ret_prop);
	return ret;
}

static long get_randnum(long min, long max)
{
	return min + (long)((float)max * (rand() / (RAND_MAX + 1.0)));
}

static int wm_supports(Display * disp, const char *prop)
{
	Atom xa_prop = XInternAtom(disp, prop, False);
	Atom *list;
	unsigned long size, items;
	int i;

	if (!(list = (Atom *) get_property(disp, DefaultRootWindow(disp),
					   XA_ATOM, "_NET_SUPPORTED", &size,
					   &items))) {
		fprintf(stderr, "Cannot get _NET_SUPPORTED property.\n");
		return 0;
	}

	size *= items;

	for (i = 0; i < size / sizeof(Atom); i++) {
		if (list[i] == xa_prop) {
			free(list);
			return 1;
		}
	}

	free(list);
	return 0;
}

static inline unsigned long clamp_value(unsigned long value,
					unsigned long min, unsigned long max)
{
	return (value < min ? min : (value > max ? max : value));
}

static int ignore_xlib_error(Display * disp, XErrorEvent * xee)
{
	char errbuf[256];

	XGetErrorText(disp, xee->error_code, errbuf, 256);
	fprintf(stderr,
		"IGNORING Xlib error %d (%s) on request (%d.%d), sernum = %lu.\n",
		xee->error_code, errbuf, xee->request_code, xee->minor_code,
		xee->serial);
	return 1;
}

static void slide_window(Display * disp, Window * win, unsigned long desk_w,
			 unsigned long desk_h)
{
	unsigned long x, y;
	unsigned long w, h;
	XWindowAttributes moo;
	Window junk;

	if (XGetWindowAttributes(disp, *win, &moo) != 1) {
		fprintf(stderr, "Cannot get attributes of window 0x%lx.\n",
			*win);
		return;
	}

	if (XTranslateCoordinates(disp, *win, moo.root,
				  -moo.border_width, -moo.border_width, &moo.x,
				  &moo.y, &junk) != 1) {
		fprintf(stderr,
			"Cannot translate coordinates of window 0x%lx.\n",
			*win);
		return;
	}

	x = moo.x + get_randnum(-SLIDE_THRESHOLD, SLIDE_THRESHOLD);
	y = moo.y + get_randnum(-SLIDE_THRESHOLD, SLIDE_THRESHOLD);
	w = moo.width + get_randnum(-SLIDE_THRESHOLD, SLIDE_THRESHOLD);
	h = moo.height + get_randnum(-SLIDE_THRESHOLD, SLIDE_THRESHOLD);

	x = clamp_value(x, 0, desk_w);
	y = clamp_value(y, 0, desk_h);
	w = clamp_value(w, 0, desk_w);
	h = clamp_value(h, 0, desk_h);

	if (wm_supports(disp, "_NET_MOVERESIZE_WINDOW")) {
		send_client_msg(disp, *win, "_NET_MOVERESIZE_WINDOW",
				0, x, y, w, h);
	} else {
		XMoveResizeWindow(disp, *win, x, y, w, h);
	}
}

static void move_window(Display * disp, Window * win, unsigned long desk_w,
			unsigned long desk_h)
{
	unsigned long x, y, w, h;

	x = get_randnum(0, desk_w);
	y = get_randnum(0, desk_h);
	w = get_randnum(150, desk_w);
	h = get_randnum(150, desk_h);

	if (wm_supports(disp, "_NET_MOVERESIZE_WINDOW")) {
		send_client_msg(disp, *win, "_NET_MOVERESIZE_WINDOW",
				0, x, y, w, h);
	} else {
		XMoveResizeWindow(disp, *win, x, y, w, h);
	}
}

static int toggle_property(Display * disp, Window * win, const char *property)
{
	Atom prop;

	prop = XInternAtom(disp, property, False);
	return send_client_msg(disp, *win, "_NET_WM_STATE",
			       _NET_WM_STATE_TOGGLE, prop, 0, 0, 0);
}

static void go_bonkers(Display * disp, unsigned long iterations,
		       unsigned long sleep)
{
	unsigned long desk_w, desk_h;
	Window *windows, *window;
	unsigned long windows_length = 0, i;

	if (!get_desktop_size(disp, &desk_w, &desk_h)) {
		fprintf(stderr, "WARNING: Assuming desktop to be 1024x768!\n");
		desk_w = 1024;
		desk_h = 768;
	}
	printf("Desktop is %lu by %lu.\n", desk_w, desk_h);

	windows = get_interesting_windows(disp, &windows_length);
	if (!windows) {
		usleep(1000000);
		return;
	}
	printf("There are %lu interesting windows.\n", windows_length);

	/* Bump up the iteration count so that all windows get
	 * some exercise. */
	iterations += iterations % windows_length;

	for (i = 0; i < iterations; i++) {
		window = &windows[i % windows_length];
		switch (get_randnum(ACTION_MIN, ACTION_MAX)) {
		case ACTION_MOVE_WINDOW:
			move_window(disp, window, desk_w, desk_h);
			break;
		case ACTION_ACTIVATE_WINDOW:
			activate_window(disp, window);
			break;
		case ACTION_MAXIMIZE_WINDOW:
			toggle_property(disp, window,
					"_NET_WM_STATE_MAXIMIZED_VERT");
			toggle_property(disp, window,
					"_NET_WM_STATE_MAXIMIZED_HORZ");
			break;
		case ACTION_FULLSCREEN_WINDOW:
			if (!enable_fullscreen)
				break;
			toggle_property(disp, window,
					"_NET_WM_STATE_FULLSCREEN");
			break;
		case ACTION_HIDDEN_WINDOW:
			toggle_property(disp, window, "_NET_WM_STATE_HIDDEN");
			break;
		case ACTION_SLIDE_WINDOW_0:
		case ACTION_SLIDE_WINDOW_1:
		case ACTION_SLIDE_WINDOW_2:
		case ACTION_SLIDE_WINDOW_3:
		case ACTION_SLIDE_WINDOW_4:
		case ACTION_SLIDE_WINDOW_5:
			slide_window(disp, window, desk_w, desk_h);
			break;
		}
		usleep(sleep);
	}

	free(windows);
}

static int send_client_msg(Display * disp, Window win, char *msg,
			   unsigned long data0, unsigned long data1,
			   unsigned long data2, unsigned long data3,
			   unsigned long data4)
{
	XEvent event;
	long mask = SubstructureRedirectMask | SubstructureNotifyMask;

	event.xclient.type = ClientMessage;
	event.xclient.serial = 0;
	event.xclient.send_event = True;
	event.xclient.message_type = XInternAtom(disp, msg, False);
	event.xclient.window = win;
	event.xclient.format = 32;
	event.xclient.data.l[0] = data0;
	event.xclient.data.l[1] = data1;
	event.xclient.data.l[2] = data2;
	event.xclient.data.l[3] = data3;
	event.xclient.data.l[4] = data4;

	if (XSendEvent(disp, DefaultRootWindow(disp), False, mask, &event)) {
		return 1;
	} else {
		fprintf(stderr, "Cannot send %s event.\n", msg);
		return 0;
	}
}

static int activate_window(Display * disp, Window * win)
{
	int ret;

	ret = send_client_msg(disp, *win, "_NET_ACTIVE_WINDOW", 0, 0, 0, 0, 0);
	XMapRaised(disp, *win);

	return ret;
}

static Window *get_client_list(Display * disp, unsigned long *size,
			       unsigned long *items)
{
	void *res;

	if ((res = (Window *) get_property(disp, DefaultRootWindow(disp),
					   XA_WINDOW, "_NET_CLIENT_LIST", size,
					   items)) == NULL) {
		if ((res =
		     (Window *) get_property(disp, DefaultRootWindow(disp),
					     XA_CARDINAL, "_WIN_CLIENT_LIST",
					     size, items)) == NULL) {
			fprintf(stderr,
				"Cannot get client list properties. \n"
				"(_NET_CLIENT_LIST or _WIN_CLIENT_LIST)" "\n");
			return NULL;
		}
	}

	return (Window *) res;
}

static Window *get_interesting_windows(Display * disp,
				       unsigned long *num_windows)
{
	Window *client_list, *ret, *tmp;
	unsigned long client_list_size, client_list_items, i;
	long *desktop;
	unsigned long num_needed = 0;

	if ((client_list = get_client_list(disp, &client_list_size,
					   &client_list_items)) == NULL) {
		return NULL;
	}

	/* Figure out how many Window structs we'll ultimately need. */
	for (i = 0; i < client_list_items; i++) {
		/* desktop ID */
		if ((desktop = (long *)get_property(disp, client_list[i],
						    XA_CARDINAL,
						    "_NET_WM_DESKTOP", NULL,
						    NULL)) == NULL) {
			desktop =
			    (long *)get_property(disp, client_list[i],
						 XA_CARDINAL, "_WIN_WORKSPACE",
						 NULL, NULL);
		}

		/* Ignore windows on unknown desktops */
		if (desktop && *desktop >= 0 && *desktop < DESKTOP_MAX) {
			num_needed++;
			free(desktop);
		}
	}

	ret = calloc(num_needed, sizeof(Window));
	if (!ret) {
		perror("get_interesting_window allocations");
		free(client_list);
		return NULL;
	}
	tmp = ret;

	/* Now copy all that crud. */
	for (i = 0; i < client_list_items; i++) {
		/* desktop ID */
		if ((desktop = (long *)get_property(disp, client_list[i],
						    XA_CARDINAL,
						    "_NET_WM_DESKTOP", NULL,
						    NULL)) == NULL) {
			desktop =
			    (long *)get_property(disp, client_list[i],
						 XA_CARDINAL, "_WIN_WORKSPACE",
						 NULL, NULL);
		}

		if (desktop && *desktop >= 0 && *desktop < DESKTOP_MAX) {
			memcpy(tmp, &client_list[i], sizeof(Window));
			tmp++;
			free(desktop);
		}
	}
	free(client_list);

	*num_windows = num_needed;
	return ret;
}

int main(int argc, char *argv[])
{
	char *disp_string = NULL;
	unsigned long iterations = 10000, rounds = -1, i;
	unsigned long sleep = 100000;
	int opt;
	Display *disp;

	while ((opt = getopt(argc, argv, "d:i:r:s:f")) != -1) {
		switch (opt) {
		case 'd':
			disp_string = optarg;
			break;
		case 'i':
			iterations = atoi(optarg);
			break;
		case 'r':
			rounds = atoi(optarg);
			break;
		case 's':
			sleep = atoi(optarg);
			break;
		case 'f':
			enable_fullscreen = 1;
			break;
		default:
			fprintf(stderr,
				"Usage: %s [-d DISPLAY] [-i ITERATIONS] [-r ROUNDS] [-s SLEEP] [-f]\n\
	DISPLAY is an X11 display string.\n\
	ITERATIONS is the approximate number of windows to play with before generating a new window list.\n\
	SLEEP is the amount of time (in usec) to sleep between window tweaks.\n\
	-f enables fullscreen toggling.\n\
	ROUNDS is the number of iterations to run, or -1 to run forever.\n",
				argv[0]);
			return 0;
		}
	}

	if (!(disp = XOpenDisplay(disp_string))) {
		fprintf(stderr, "Unable to connect to display '%s'.\n",
			(disp_string !=
			 NULL ? disp_string : getenv("DISPLAY")));
		return 1;
	}

	seed_random();

	XSetErrorHandler(&ignore_xlib_error);

	for (i = 0; i < rounds || rounds == -1; i++) {
		go_bonkers(disp, iterations, sleep);
	}

	printf("Enough of that; I'm done.\n");

	XCloseDisplay(disp);

	return 0;
}

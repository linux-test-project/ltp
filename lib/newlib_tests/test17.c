// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2017 Richard Palethorpe <rpalethorpe@suse.com>
 */
/* Basic functionality test for tst_fuzzy_sync.h similar to the atomic tests
 * (test15.c). One thread writes to the odd indexes of an array while the
 * other writes to the even. If the threads are not synchronised then they
 * will probably write to the wrong indexes as they share an index variable
 * which they should take it in turns to update.
 */

#include <stdlib.h>
#include "tst_test.h"

#define TEXT "The only purpose of this text si to try to overflow the buffers "\
	     "in test library, other than that it does not include any useful "\
	     "information. Whoever decides to read this wall of text is just " \
	     "simply wasting his time. You are not reading this are you? Hmm " \
	     "you still do. Are you feeling rebelious today? Well whatever. "  \
	     "It's _your_ time not mine. Feel free to waste it if you want "   \
	     "to. Still reading? I bet you are not. And don't try to prove me "\
	     "wrong just because you can. Still reading? Do you feel better "\
	     "now? Now even I am bored, how come that can you still continue "\
	     "reading? And now for something completely different! Let's try "\
	     "some ASCII art! This is a sitting mouse from a top: <3)~~ Did "\
	     "like that? No? Hmm, let me think, I think I can draw a pengiun "\
	     "as well what about this one: <(^) ? You liked the mouse better? "\
	     "Why I'm even trying? Anyway I'm pretty sure nobody got here, so "\
	     "I will write down a secret. It will be burried here forever and "\
	     "ever until brave adventurer decides to read this paragraph to "\
	     "the very end. Here it comes: the text was long enough even "\
	     "before I added this sentence, therefore this sentence is as "\
	     "useless as it can be, yet it exists here, sometimes strange "\
	     "things like this happens..."

static void run(void)
{
	tst_res(TPASS, TEXT);
	tst_res(TPASS | TERRNO, TEXT);
}

static struct tst_test test = {
	.test_all = run,
};

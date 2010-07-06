#
# Read COPYING for licensing details.
#
# Garrett Cooper, June 2010
#

srcdir?=	.

all: t0

clean:
	@rm -f t0

t0: $(srcdir)/t0.c
	@$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $< $(LDLIBS)

#build/t0.val: build/t0
#	echo `./t0 0; echo $$?` > t0.val


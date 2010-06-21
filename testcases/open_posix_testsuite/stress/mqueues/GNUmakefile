CFLAGS+=	-I../../include
LDLIBS+=	-lrt

TARGETS=	multi_send_rev_1.test multi_send_rev_2.test

all: $(TARGETS)

multi_send_rev_1.test: multi_send_rev_1.c
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $< $(LDLIBS)

multi_send_rev_2.test: multi_send_rev_2.c
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $< $(LDLIBS)

clean: 
	rm -f $(TARGETS)

test:
	@./run.sh

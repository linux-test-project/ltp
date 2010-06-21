CFLAGS+=	-I../../include
LDLIBS+=	-lpthread

TARGETS+=	multi_con_pro.test

all: $(TARGETS)

multi_con_pro.test: multi_con_pro.c
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $< $(LDLIBS)

clean: 
	rm -f $(TARGETS)

test:
	@./run.sh

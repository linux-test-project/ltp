
CFLAGS += -O2 -Wall -W -g
OBJS = tbio_ki.o user_tbio.o

test_bio: $(OBJS)
	$(CC) $(CFLAGS) $(LDFLAGS) $^ -o $@

clean:
	rm -f $(OBJS) test_bio

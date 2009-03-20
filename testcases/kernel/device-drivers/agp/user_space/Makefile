
CFLAGS += -Wall
OBJS = tagp_ki.o user_tagp.o

test_agp: $(OBJS)
	$(CC) $(CFLAGS) $(LDFLAGS) $^ -o $@

clean:
	rm -f $(OBJS) test_agp

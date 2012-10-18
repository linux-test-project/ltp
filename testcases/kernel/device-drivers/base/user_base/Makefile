
CFLAGS += -Wall
OBJS = tbase_ki.o user_tbase.o

test_base: $(OBJS)
	$(CC) $(CFLAGS) $(LDFLAGS) $^ -o $@

clean:
	rm -f $(OBJS) test_base

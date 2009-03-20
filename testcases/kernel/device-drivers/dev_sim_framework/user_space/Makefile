
CFLAGS += -Wall
OBJS = tmod_ki.o user_tmod.o

test_mod: $(OBJS)
	$(CC) $(CFLAGS) $(LDFLAGS) $^ -o $@

clean:
	rm -f $(OBJS) test_mod

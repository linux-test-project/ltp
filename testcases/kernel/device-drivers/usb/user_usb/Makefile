
CFLAGS += -Wall
OBJS = tusb_ki.o user_tusb.o

test_usb: $(OBJS)
	$(CC) $(CFLAGS) $(LDFLAGS) $^ -o $@

clean:
	rm -f $(OBJS) test_usb


CFLAGS += -Wall
OBJS = tpci_ki.o user_tpci.o

test_pci: $(OBJS)
	$(CC) $(CFLAGS) $(LDFLAGS) $^ -o $@

clean:
	rm -f $(OBJS) test_pci

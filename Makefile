
CFLAGS += $(shell pkg-config --cflags hidapi)
LDFLAGS += $(shell pkg-config --libs hidapi)

keytar: keytar.o
	$(CC) -o $@ $(LDFLAGS) $<

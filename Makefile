.DEFAULT_GOAL := nanoanno
CC := gcc
CFILES   := $(wildcard *.c)
OBJFILES := $(CFILES:.c=.o)
LDLIBS := $(shell pkg-config --cflags --libs gtk+-3.0)


nanoanno: $(OBJFILES)
	$(CC) -o $@ $^ -Wall $(LDLIBS) -export-dynamic

%.o: %.c
	$(CC) $< -c -Wall $(LDLIBS)

.PHONY: install
install: nanoanno
	cp nanoanno /usr/local/bin/nanoanno
	mkdir -p /usr/local/lib/nanoanno/
	cp window.ui /usr/local/lib/nanoanno/window.ui

.PHONY: clean
clean:
	rm -f $(OBJFILES) $(.DEFAULT_GOAL)

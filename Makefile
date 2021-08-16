.DEFAULT_GOAL := pinsel
CC := gcc
CFILES   := $(wildcard *.c)
OBJFILES := $(CFILES:.c=.o)
LDLIBS := $(shell pkg-config --cflags --libs gtk+-3.0)


pinsel: $(OBJFILES)
	$(CC) -o $@ $^ -Wall $(LDLIBS) -export-dynamic

%.o: %.c
	$(CC) $< -c -Wall $(LDLIBS)

.PHONY: install
install: pinsel
	cp pinsel /usr/local/bin/pinsel
	mkdir -p /usr/local/lib/pinsel/
	cp window.ui /usr/local/lib/pinsel/window.ui

.PHONY: clean
clean:
	rm -f $(OBJFILES) $(.DEFAULT_GOAL)

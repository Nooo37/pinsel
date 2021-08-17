.DEFAULT_GOAL := pinsel
CC := gcc
CFILES   := $(wildcard *.c) resources.c
OBJFILES := $(CFILES:.c=.o)
LDLIBS := $(shell pkg-config --cflags --libs gtk+-3.0)


pinsel: $(OBJFILES)
	$(CC) -o $@ $^ -Wall $(LDLIBS) -export-dynamic

resources.c: ui.gresource.xml
	glib-compile-resources --target=resources.c --generate-source ui.gresource.xml

%.o: %.c
	$(CC) $< -c -Wall $(LDLIBS)

.PHONY: install
install: pinsel
	cp pinsel /usr/local/bin/pinsel
	mkdir -p /usr/local/lib/pinsel/

.PHONY: clean
clean:
	rm -f $(OBJFILES) $(.DEFAULT_GOAL)

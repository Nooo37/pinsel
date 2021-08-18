.DEFAULT_GOAL := pinsel
CC := gcc
RESFILE  := resources.c
CFILES   := $(wildcard *.c) $(RESFILE)
OBJFILES := $(CFILES:.c=.o)
LDLIBS := $(shell pkg-config --cflags --libs gtk+-3.0)


pinsel: $(OBJFILES)
	$(CC) -o $@ $^ -Wall $(LDLIBS) -export-dynamic

$(RESFILE): ui.gresource.xml window.ui
	glib-compile-resources --target=$@ --generate-source $<

%.o: %.c
	$(CC) $< -c -Wall $(LDLIBS)

.PHONY: install
install: pinsel
	cp pinsel /usr/local/bin/pinsel

.PHONY: clean
clean:
	rm -f $(OBJFILES) $(.DEFAULT_GOAL) $(RESFILE)

.DEFAULT_GOAL := nanoanno
CC := gcc
LDLIBS := $(shell pkg-config --cflags --libs gtk+-3.0)


nanoanno: main.o draw.o
	$(CC) main.o draw.o -o nanoanno -Wall $(LDLIBS) -export-dynamic

main.o: main.c
	$(CC) main.c -c -Wall $(LDLIBS)

draw.o: draw.c
	$(CC) draw.c -c -Wall $(LDLIBS)

install: nanoanno
	cp nanoanno /usr/local/bin/nanoanno
	mkdir -p /usr/local/lib/nanoanno/
	cp window.ui /usr/local/lib/nanoanno/window.ui

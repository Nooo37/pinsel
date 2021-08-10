# still no real idea
CC = gcc
LDLIBS = $(shell pkg-config --cflags --libs gtk+-3.0)

nanoanno: main.c
	$(CC) main.c -o nanoanno -Wall $(LDLIBS) -export-dynamic


# still no real idea
CC = gcc
LDLIBS = $(shell pkg-config --cflags --libs gtk+-3.0)

nanoanno: main.o draw.o
	$(CC) main.o draw.o -o nanoanno -Wall $(LDLIBS) -export-dynamic

main.o: main.c
	$(CC) main.c -c -Wall $(LDLIBS)

draw.o: draw.c
	$(CC) draw.c -c -Wall $(LDLIBS)

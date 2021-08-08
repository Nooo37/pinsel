# idk makefiles (yet) 🤷
default: main.c
	gcc -o nanoanno main.c -Wall `pkg-config --cflags --libs gtk+-3.0` -export-dynamic

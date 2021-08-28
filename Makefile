.DEFAULT_GOAL := pinsel
CC            := gcc
INSTALLDIR    := /usr/local/bin
DATADIR       := ./data
SRCDIR        := ./src
OBJDIR        := ./obj
DATAFILES     := $(wildcard $(DATADIR)/*.ui) $(DATADIR)/help.txt $(DATADIR)/pinsel.lua
RESFILE       := $(SRCDIR)/resources.c
CFILES        := $(wildcard $(SRCDIR)/*.c) $(RESFILE)
OBJFILES      := $(CFILES:$(SRCDIR)/%.c=$(OBJDIR)/%.o)
LDLIBS        := $(shell pkg-config --cflags --libs gtk+-3.0 lua)


pinsel: $(OBJFILES)
	$(CC) -o $@ $^ -Wall $(LDLIBS) -export-dynamic

$(RESFILE): $(DATADIR)/data.gresource.xml $(DATAFILES)
	glib-compile-resources --target $@ --generate-source $<

$(OBJDIR)/%.o: $(SRCDIR)/%.c
	mkdir -p obj
	$(CC) $< -c -o $@ -Wall $(LDLIBS)

.PHONY: install
install: pinsel
	cp pinsel $(INSTALLDIR)/pinsel

.PHONY: uninstall
uninstall:
	rm $(INSTALLDIR)/pinsel

.PHONY: clean
clean:
	rm -f $(OBJFILES) $(.DEFAULT_GOAL) $(RESFILE)

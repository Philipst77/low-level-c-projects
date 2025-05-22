CC = gcc -std=gnu99
OPTS = -Wall -Werror
DEBUG = -g

SRCDIR = ./src
OBJDIR = ./obj
INCDIR = ./inc

INCLUDE=$(addprefix -I,$(INCDIR))
CFLAGS=$(OPTS) $(INCLUDE) $(DEBUG)
OBJECTS=$(addprefix $(OBJDIR)/,nodemngr.o exec.o)

all: nodemngr

nodemngr: $(OBJECTS)
	$(CC) $(CFLAGS) -o $@ $^
$(OBJDIR)/nodemngr.o: $(SRCDIR)/nodemngr.c $(INCDIR)/exec.h $(INCDIR)/listnode.h
	$(CC) $(CFLAGS) -c $< -o $@
clean:
	rm nodemngr
	rm $(OBJDIR)/nodemngr.o

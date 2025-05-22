#-----------------------------------------------------------------------------
# Makefile
#-----------------------------------------------------------------------------

#-----------------------------------------------------------------------------
# Choose a compiler and its options
#--------------------------------------------------------------------------
CC   = gcc -std=gnu99	# Use gcc for Zeus
OPTS = -Og -Wall -Werror -Wno-error=unused-variable -Wno-error=unused-function
DEBUG = -g					# -g for GDB debugging

#--------------------------------------------------------------------
# Build Environment
#--------------------------------------------------------------------
UID := $(shell id -u)
SRCDIR=./src
OBJDIR=./obj
INCDIR=./inc
STAGINGDIR=./staging
BINDIR=.

#--------------------------------------------------------------------
# Build Files
#--------------------------------------------------------------------

#--------------------------------------------------------------------
# Compiler Options
#--------------------------------------------------------------------
INCLUDE=$(addprefix -I,$(INCDIR))
HEADERS=$(wildcard $(INCDIR)/*.h)
SOURCES=$(OBJDIR)/muan.o $(STAGINGDIR)/lex.yy.c $(STAGINGDIR)/muan_grammar.tab.c $(SRCDIR)/smallfp.c $(SRCDIR)/common_functions.c $(SRCDIR)/symtab.c $(SRCDIR)/hashmap.c 
CFLAGS=$(OPTS) $(INCLUDE) $(DEBUG)

#--------------------------------------------------------------------
# Build Recipies for the Executables (binary)
#--------------------------------------------------------------------
TARGET=$(BINDIR)/muan $(BINDIR)/ref_all_values

all: $(TARGET)

tester: $(SRCDIR)/test_smallfp.c $(SRCDIR)/smallfp.c
	gcc -g -o tester -I./$(INCDIR) $(SRCDIR)/test_smallfp.c $(SRCDIR)/smallfp.c

$(BINDIR)/ref_all_values: $(OBJDIR)/ref_all_values.o 
	$(CC) $(CFLAGS) -o $@ $^ -lm

$(BINDIR)/muan: $(SOURCES) $(HEADERS)
	$(CC) -I./inc -g -o $@ $(SOURCES) -lfl -ly

$(OBJDIR)/muan.o: $(SRCDIR)/muan.c
	$(CC) -c $(CFLAGS) -o $@ $^

$(STAGINGDIR)/lex.yy.c: $(SRCDIR)/muan_tokens.l
	flex -o $@ $^

$(STAGINGDIR)/muan_grammar.tab.c $(STAGINGDIR)/muan_grammar.tab.h: $(SRCDIR)/muan_grammar.y
	bison -vd -o $@ $^

clean:
	rm -f $(TARGET) $(STAGINGDIR)/* $(OBJDIR)/muan.o tester $(BINDIR)/muan 

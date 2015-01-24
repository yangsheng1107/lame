# ------------------------------------------------
# Generic Makefile
#
# Author: yanick.rochon@gmail.com
# Date  : 2010-11-05
#
# Changelog :
#   0.01 - first version
# ------------------------------------------------

# change these to set the proper directories where each files should be
SRCDIR   = ./src
OBJDIR   = ./obj
BINDIR   = ./bin
INCDIR   = ./include
LIBDIR   = ./lib

# project name (generate executable with this name)
CC       = gcc

# compiling flags here
CFLAGS   = -Wall -I.
LINKER   = $(CC) -o -g

# linking flags here
LFLAGS   = -Wall -L$(LIBDIR)
LIBS     = -lmp3lame 

all: dir_create lame

lame: lame_obj
	$(CC) $(OBJDIR)/lame.o -o $(BINDIR)/lame $(LFLAGS) $(LIBS)
	
lame_obj: $(SRCDIR)/lame.c
	$(CC) -I$(INCDIR) $(CFLAGS) -c $(SRCDIR)/lame.c -o $(OBJDIR)/lame.o

dir_create:
ifeq "$(wildcard $(OBJDIR) )" ""
	mkdir -p $(OBJDIR)
endif
ifeq "$(wildcard $(BINDIR) )" ""
	mkdir -p $(BINDIR)
endif

clean:
	rm -rf $(BINDIR)/lame
	rm -rf $(OBJDIR)/*.*

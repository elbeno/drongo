#
#
#/*
#         (C) Copyright, 1988, 1989 Marcus J. Ranum
#                    All rights reserved
#
#
#          This software, its documentation,  and  supporting
#          files  are  copyrighted  material  and may only be
#          distributed in accordance with the terms listed in
#          the COPYRIGHT document.
#*/
#
#

INCLUDE= ../include
# compiler to use
CC= cc

# compiler flags
#CFLAGS= -p
CFLAGS= -O -I$(INCLUDE)

# where to build the library. must be set in testrack/Makefile
# if you change it from here.
LIBDIR=../obj
LIB=	$(LIBDIR)/libbtree.a

CFILES=	btclose.c \
	btdebug.c \
	btdelete.c \
	bterrors.c \
	btfind.c \
	btgoto.c \
	btinsert.c \
	btio.c \
	btlabel.c \
	btload.c \
	btopen.c \
	btoopen.c \
	btpage1.c \
	btpage2.c \
	btravrs.c \
	btseek.c \
	btzap.c

HFILES=	btconf.h \
	$(INCLUDE)/btree.h \
	btintern.h

OFILES=	btclose.o \
	btdebug.o \
	btdelete.o \
	bterrors.o \
	btfind.o \
	btgoto.o \
	btio.o \
	btinsert.o \
	btlabel.o \
	btload.o \
	btopen.o \
	btoopen.o \
	btpage1.o \
	btpage2.o \
	btravrs.o \
	btseek.o \
	btzap.o

FILES=	$(CFILES) \
	$(HFILES) \
	README \
	Makefile

all:	$(LIB)
	@echo done

$(LIB):	$(OFILES) 
	ar crv $(LIB) $(OFILES)
	ranlib $(LIB)

$(OFILES): $(HFILES)

clean:
	rm -f *.o core tags

clobber: clean
	rm -f $(LIB)

ci:
	ci -t/dev/null -m" " -u $(FILES) < /dev/null

tags:
	ctags $(CFILES)

lint:	$(CFILES)
	lint -I$(INCLUDE) -u $(CFILES)

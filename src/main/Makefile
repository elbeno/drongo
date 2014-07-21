#the ULTRIX_BUG is only to catch a bug in Ultrix 3.14 that fails to make a
#core dump in some cases where it should.

#compile/link flags set up for debugging
CFLAGS= -g -I../include 
# removed -DULTRIX_BUG
LDFLAGS= -g
CC=cc

#compile/link flags set up for production run
#CFLAGS= -O -I../include -DULTRIX_BUG
#LDFLAGS= -s

#compile/link flags set up for performance tuning
#CFLAGS= -p -I../include -DULTRIX_BUG
#LDFLAGS= -p

LIBDIR=	../obj
INCDIR=	../include

LIB=	\
	$(LIBDIR)/libbtree.a \
	$(LIBDIR)/libregexp.a

HDR=	\
	$(INCDIR)/btree.h \
	$(INCDIR)/externs.h \
	$(INCDIR)/io.h \
	$(INCDIR)/store.h \
	$(INCDIR)/syssym.h \
	$(INCDIR)/ubermud.h

OBJ=	\
	alloc.o \
	bltin.o \
	blttab.o \
	disp.o \
	list.o \
	main.o \
	match.o \
	misc.o \
	monitor.o \
	parse.o \
	run.o \
	store.o \
	sym.o \
	tcpio.o

SRC=	\
	alloc.c \
	bltin.c \
	blttab.c \
	disp.c \
	list.c \
	main.c \
	match.c \
	misc.c \
	monitor.c \
	parse.y \
	run.c \
	store.c \
	sym.c \
	tcpio.c

LINT=	\
	alloc.c \
	bltin.c \
	blttab.c \
	disp.c \
	list.c \
	main.c \
	match.c \
	misc.c \
	monitor.c \
	parse.c \
	run.c \
	store.c \
	sym.c \
	tcpio.c

RCS=	boot.u \
	checkserver \
	haltserver \
	README \
	sizes.c \
	startserver

all:	um sizes tags
	@echo done

um:	$(OBJ)
	gcc $(LDFLAGS) -o $@ $(OBJ) $(LIB)
	@echo WARNING - this may outdate existing data base.

sizes:	$(HDR)
	cc $(CFLAGS) $(LDFLAGS) -o $@ sizes.c
	@ $@

cleanfridge:
	rm -f fridge/chunks fridge/index fridge/inodes fridge/server_pid

lint:	$(LINT)
	lint -I$(INCDIR) $(LINT)

clean:	cleanfridge
	rm -f $(OBJ) parse.c lint.out sizes tags

clobber:	clean
	rm -f um
	
ci:
	ci -t/dev/null -m" " -u $(SRC) $(RCS) Makefile < /dev/null

tags:
	ctags $(SRC)

parse.c: $(HDR) parse.y
$(OBJ): $(HDR)

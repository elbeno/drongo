#makefile and whatnot are mine, not Spencer's

INCLUDE= ../include
# compiler to use
CC= cc

# compiler flags
#CFLAGS= -p
CFLAGS= -O -I$(INCLUDE)

LIBDIR=../obj
LIB=	$(LIBDIR)/libregexp.a

CFILES=	regexp.c regsub.c

HFILES= regmagic.h $(INCLUDE)/regexp.h

OFILES=	regexp.o regsub.o

FILES=	$(CFILES) \
	$(HFILES) \
	README \
	regexp.3 \
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

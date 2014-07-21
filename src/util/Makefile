#if using DGUX or other hosts that like the old .S_un.s_addr type format
#for structs of type in_addr, #define DGUX

#CFLAGS= -O $(DEFTHOST)
#LDFLAGS= -s
#LINTFLG=

CFLAGS= -g $(DEFTHOST)
LDFLAGS= -g
LINTFLG=

CC=	cc
#CC=	gcc

#comment or uncomment this depending on whether you need the resolver code
#or not. (does your system use a hosts table ?)
#RESOLV= -lresolv
RESOLV=

SRC=	usubmit.c \
	syncheck.y

all:	usubmit syncheck

usubmit: usubmit.o
	cc $(LDFLAGS) -o usubmit usubmit.o $(RESOLV)

syncheck: syncheck.o
	cc $(LDFLAGS) -o syncheck syncheck.o

clean:
	rm -f core syncheck.c *.o

clobber: clean
	rm -f usubmit syncheck

doc:
	@echo "that is a good one!"

ci:
	ci -t/dev/null -m" " -u $(SRC) Makefile < /dev/null

lint:
	int $(LINTFLG) usubmit.c

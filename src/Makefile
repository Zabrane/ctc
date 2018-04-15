SHELL=/bin/sh
CC=gcc
CFLAGS=-O2 -fomit-frame-pointer -Wall
ifdef DEBUG
CFLAGS += -DDEBUG=1
endif

# Use these for x86-32 and ppc32 on Linux
#LIBOBJS=main.o perfctr.o
#BENCHLIB=path-to-perfctr/usr.lib/libperfctr.a
#CPPFLAGS=-Ipath-to-perfctr/usr.lib -Ipath-to-perfctr/linux/include

# Use these for ARM on Linux w/o perfctr
LIBOBJS=
BENCHLIB=
CPPFLAGS=-DSTANDALONE

# Use these for UltraSPARC on Solaris
#LIBOBJS=main.o solaris.o
#BENCHLIB=-lcpc
#CPPFLAGS=

LIBOBJS += tak_bytecode.o

CSRC=	tak_c.c tak_bc.c tak_dtc.c tak_ctc0.c tak_ctc1.c tak_ctc2.c tak_ctc3.c tak_ctc4.c tak_ctc5.c tak_ctc6.c
OBJS=	$(CSRC:.c=.o) $(LIBOBJS)
EXES=	$(CSRC:.c=)

all:	$(EXES)

perfctr.o:	path-to-perfctr path-to-perfctr/usr.lib/libperfctr.h path-to-perfctr/linux/include/asm/perfctr.h

tak_c:	tak_c.o $(LIBOBJS)
	$(LINK.c) -o $@ $^ $(BENCHLIB)

tak_bc:	tak_bc.o $(LIBOBJS)
	$(LINK.c) -o $@ $^ $(BENCHLIB)

tak_dtc:	tak_dtc.o $(LIBOBJS)
	$(LINK.c) -o $@ $^ $(BENCHLIB)

tak_ctc0:	tak_ctc0.o $(LIBOBJS)
	$(LINK.c) -o $@ $^ $(BENCHLIB)

tak_ctc1:	tak_ctc1.o $(LIBOBJS)
	$(LINK.c) -o $@ $^ $(BENCHLIB)

tak_ctc2:	tak_ctc2.o $(LIBOBJS)
	$(LINK.c) -o $@ $^ $(BENCHLIB)

tak_ctc3:	tak_ctc3.o $(LIBOBJS)
	$(LINK.c) -o $@ $^ $(BENCHLIB)

tak_ctc4:	tak_ctc4.o $(LIBOBJS)
	$(LINK.c) -o $@ $^ $(BENCHLIB)

tak_ctc5:	tak_ctc5.o $(LIBOBJS)
	$(LINK.c) -o $@ $^ $(BENCHLIB)

tak_ctc6:	tak_ctc6.o $(LIBOBJS)
	$(LINK.c) -o $@ $^ $(BENCHLIB)

clean:
	rm -f $(OBJS) $(EXES) a.out core* ctc.aux ctc.blg ctc.dvi ctc.log

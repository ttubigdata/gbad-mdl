# Makefile
#
# GBAD 2.0
#
CC =		gcc
CFLAGS =	-Wall -O3
LDFLAGS =	-O3
# debug CFLAGS and LDFLAGS
#CFLAGS =	-g -pg -Wall -O3
#LDFLAGS =	-g -pg -O3

LDLIBS =	-lm
OBJS = 		compress.o discover.o evaluate.o extend.o graphmatch.o\
                graphops.o labels.o sgiso.o subops.o utility.o \
                gbad.o
TARGETS =	gbad

all: $(TARGETS)

gbad: main.o $(OBJS)
	$(CC) $(LDFLAGS) -o gbad main.o $(OBJS) $(LDLIBS)

%.o: %.c gbad.h
	$(CC) -c $(CFLAGS) $< -o $@

install:
	cp $(TARGETS) ../bin

clean:
	/bin/rm -f *.o $(TARGETS)


.PHONY: all clean

.SUFFIXES:

CC = gcc
CFLAGS = -Wall -Werror
LDFLAGS =
LIBS = -lpthread

INCLDIR = -I.

INCLUDE = csapp.h bibftp.h
OBJS = csapp.o


PROGS = echoserver echoclient 

all: $(PROGS)


%.o: %.c $(INCLUDE)
	$(CC) $(CFLAGS) $(INCLDIR) -c -o $@ $<


%: %.o $(OBJS)
	$(CC) -o $@ $(LDFLAGS) $^ $(LIBS)

clean:
	rm -f $(PROGS) *.o
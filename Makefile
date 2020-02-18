# http://www.cs.colby.edu/maxwell/courses/tutorials/maketutor/

CC=gcc
CFLAGS=-Wall  -std=c99 -g
LDFLAGS=
DEPS=

all: cl_hqos

cl_hqos: cl_hqos.c $(DEPS)
	$(CC) $(CFLAGS) cl_hqos.c -o cl_hqos $(LD_FLAGS)

clean:
	rm cl_hqos

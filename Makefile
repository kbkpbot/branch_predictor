# Description: Makefile for building a branch predictor.
#


LDFLAGS += 

LDLIBS   += -lzstd

CPPFLAGS1 := -O3 -Wall -Wextra -Winline -Winit-self -Wno-sequence-point\
           -Wno-unused-function -Wno-inline -fPIC -W -Wcast-qual -Wpointer-arith -Icbsl/include
CPPFLAGS := -Ofast -Icbsl/include

PROGRAMS := predictor

objects = bt9.o main.o predictor.o cbsl/src/buffer.o cbsl/src/file.o cbsl/src/flush.o cbsl/src/read.o cbsl/src/record.o cbsl/src/utils.o cbsl/src/write.o

all: $(PROGRAMS)

predictor : $(objects)
	gcc $(CPPFLAGS) $(LDFLAGS) -o $@ $^ $(LDLIBS)
	rm -f $(objects)

clean:
	rm -f $(PROGRAMS) $(objects)

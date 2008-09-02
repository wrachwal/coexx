# Makefile

UNAME := $(shell uname -o)

CXX = g++
CXXFLAGS += -g -O2 -Wall -pedantic -ansi

ifeq ($(UNAME),Solaris)
  CXXFLAGS += -D_POSIX_C_SOURCE
  LDFLAGS  += -lpthread -lrt
endif

programs = sigsel1 sigsel2

.DEFAULT: all
.PHONY: all clean

all: $(programs)

sigsel1: sigsel1.o
	$(CXX) -o $@ $^ $(LDFLAGS)

sigsel2: sigsel2.o
	$(CXX) -o $@ $^ $(LDFLAGS)

clean:
	-rm *.d *.o core *.exe $(programs)


# Makefile

UNAME := $(shell uname -o)

CXX = g++
CXXFLAGS += -g -O2 -Wall -pedantic -ansi

ifeq ($(UNAME),Solaris)
  CXXFLAGS += -D_POSIX_C_SOURCE
  LDFLAGS  += -lpthread -lrt
endif

.DEFAULT: all
.PHONY: all clean

all: sigsel

sigsel: sigsel.o
	$(CXX) -o $@ $^ $(LDFLAGS)

clean:
	-rm *.d *.o core *.exe sigsel


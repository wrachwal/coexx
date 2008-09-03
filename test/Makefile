# test/Makefile

UNAME := $(shell uname -o)

INC_DIRS = ../include ../src

CXX = g++
CXXFLAGS += -g -O2 -Wall -pedantic -ansi $(patsubst %,-I%,${INC_DIRS})

ifeq ($(UNAME),Solaris)
  CXXFLAGS += -D_POSIX_C_SOURCE
  LDFLAGS  += -lpthread -lrt
endif

VPATH = $(subst  ,:,${INC_DIRS})

programs = list cond sigsel1 sigsel2

.DEFAULT: all

.PHONY: all clean

all: $(programs)

list: list.o
	$(CXX) -o $@ $^ $(LDFLAGS)

cond: cond.o coe-os.o
	$(CXX) -o $@ $^ $(LDFLAGS)

sigsel1: sigsel1.o
	$(CXX) -o $@ $^ $(LDFLAGS)

sigsel2: sigsel2.o
	$(CXX) -o $@ $^ $(LDFLAGS)

clean:
	-rm *.d *.o core *.exe $(programs)


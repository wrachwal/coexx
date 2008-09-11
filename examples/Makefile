# $Id$

UNAME := $(shell uname -o)

inc_dirs := ../include

CXX = g++
CXX += -MMD

CXXFLAGS += -g -O2 -Wall -pedantic -ansi $(patsubst %,-I%,${inc_dirs}) \
	    -Wno-invalid-offsetof

ifeq ($(UNAME),Solaris)
  CXXFLAGS += -D_POSIX_C_SOURCE
  LDFLAGS  += -lpthread -lrt
endif

VPATH = $(subst  ,:,${inc_dirs})

programs := myhouse tick

objects := $(patsubst %.cpp,%.o,$(wildcard *.cpp))

library := ../lib/libcoe.a 

.DEFAULT: all

.PHONY: all lib clean

all: $(programs)

lib:
	$(MAKE) -C ../lib

$(library): lib

myhouse: myhouse.o $(library)
	$(CXX) -o $@ $^ $(LDFLAGS)

tick: tick.o $(library)
	$(CXX) -o $@ $^ $(LDFLAGS)

clean:
	-rm *.d *.o core *.exe $(programs)

-include $(patsubst %.o,%.d,$(objects))


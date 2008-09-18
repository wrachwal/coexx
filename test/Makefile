# $Id$

UNAME := $(shell uname -o)

inc_dirs := ../include

CXX = g++
CXX += -MMD

CXXFLAGS += -g -O2 -Wall -pedantic -ansi $(patsubst %,-I%,${inc_dirs}) \
	    -Wno-invalid-offsetof

ifeq (Solaris,$(UNAME))
  CXXFLAGS += -D_POSIX_C_SOURCE
  LDFLAGS  += -lpthread -lrt
else
ifeq (Linux,$(findstring Linux,$(UNAME)))
  LDFLAGS  += -lpthread -lrt
endif
endif

VPATH = $(subst  ,:,${inc_dirs})

programs := typeinfo list cond sigsel1 sigsel2

objects := $(patsubst %.cpp,%.o,$(wildcard *.cpp))

library := ../lib/libcoe.a 

.DEFAULT: all

.PHONY: all lib clean

all: $(programs)

lib:
	$(MAKE) -C ../lib

$(library): lib

typeinfo: typeinfo.o $(library)
	$(CXX) -o $@ $^ $(LDFLAGS)

list: list.o
	$(CXX) -o $@ $^ $(LDFLAGS)

cond: cond.o $(library)
	$(CXX) -o $@ $^ $(LDFLAGS)

cond.o: CXXFLAGS += -Wno-missing-braces

sigsel1: sigsel1.o
	$(CXX) -o $@ $^ $(LDFLAGS)

sigsel2: sigsel2.o
	$(CXX) -o $@ $^ $(LDFLAGS)

clean:
	-rm *.d *.o core *.exe $(programs)

-include $(patsubst %.o,%.d,$(objects))


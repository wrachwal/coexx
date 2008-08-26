# Makefile

INC_DIRS = ../include ../src
SRC_DIRS = ..

CXX = g++
override CXX += -MMD

override CXXFLAGS += -g -O2 -Wall -pedantic -ansi \
        $(patsubst %,-I%,${INC_DIRS})

VPATH = $(subst  ,:,${INC_DIRS} ${SRC_DIRS})

program = dynacall

.DEFAULT: $(program)

.PHONY: strip test clean

objects = \
  dynacall.o \
  typeinfo.o \
  coe--ev.o \
  coe--thread.o \
  coe-kernel.o \
  coe-kernel--dcl.o \
  coe-kernel--imp.o \
  coe-kernel--r4k.o \
  coe-kernel--s4k.o \
  coe-session.o \
  coe-session--r4s.o

$(program): $(objects)
	$(CXX) -o $@ $^ $(LDFLAGS)

strip: $(program)
	strip $(program)

test: $(program)
	./$(program)

check: $(program)
	./$(program) | diff $(program).log -

clean:
	-rm core *.o *.d $(program) $(program).exe

-include $(patsubst %.o,%.d,$(objects))


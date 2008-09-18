# $Id$

UNAME := $(shell uname -o)

inc_dirs := ../include ../src

CXX = g++
CXX += -MMD

CXXFLAGS += -g -O2 -Wall -pedantic -ansi $(patsubst %,-I%,${inc_dirs}) \
  	    -Wno-invalid-offsetof -Wno-strict-aliasing

ifeq (Solaris,$(UNAME))
  CXXFLAGS += -D_POSIX_C_SOURCE
  LDFLAGS  += -lpthread -lrt
else
ifeq (Linux,$(findstring Linux,$(UNAME)))
  LDFLAGS  += -lpthread -lrt
endif
endif

VPATH = $(subst  ,:,${inc_dirs})

library := libcoe.a

objects := $(patsubst %.cpp,%.o,$(notdir $(wildcard ../src/*.cpp)))

.DEFAULT: $(library)

.PHONY: clean realclean strip

.INTERMEDIATE: $(objects)

$(library): $(library)($(objects))

clean:
	-rm *.o *.d

realclean: clean
	-rm $(library)

strip: $(library)
	strip -x $(library)

-include $(patsubst %.o,%.d,$(objects))


# $Id$

UNAME := $(shell uname -o)

CXX = g++
CXX += -MMD

CXXFLAGS += -g -O2 -Wall -pedantic -ansi -Wno-invalid-offsetof -Wno-strict-aliasing \
	    -Iinclude

ifeq (Solaris,$(UNAME))
  CXXFLAGS += -D_POSIX_C_SOURCE
  LDLIBS   += -lpthread -lrt
else
ifeq (Linux,$(findstring Linux,$(UNAME)))
  LDLIBS   += -lpthread -lrt
endif
endif

library := lib/libcoe.a
objects := $(patsubst src/%.cpp,lib/%.o,$(wildcard src/*.cpp))

examples := $(addprefix examples/,myhouse tick)
tests    := $(addprefix test/,typeinfo list cond sigsel1 sigsel2)

objects_all := $(objects) $(patsubst %.cpp,%.o,$(wildcard examples/*.cpp test/*.cpp))

.PHONY: all examples tests strip clean realclean

.INTERMEDIATE: $(objects)

lib/%.o: src/%.cpp
	$(CXX) $(CXXFLAGS) -c -o $@ $<

$(library): $(library)($(objects))

all: $(library) examples tests

examples: $(examples)

tests: $(tests)

$(addsuffix .o,$(examples) $(tests)): %.o: %.cpp
	$(CXX) $(CXXFLAGS) -c -o $@ $<

$(examples) $(tests): %: %.o
	$(CXX) $(LDFLAGS) $< $(LDLIBS) -o $@

$(examples) $(tests): $(library)
$(examples) $(tests): LDLIBS += -Llib -lcoe

test/cond.o: CXXFLAGS += -Wno-missing-braces

strip: $(library)
	strip -x $(library)

clean:
ifeq (Cygwin,$(UNAME))
	-rm $(addsuffix .exe,$(examples) $(tests))
else
	-rm $(examples) $(tests)
endif
	-rm lib/*.o examples/*.o test/*.o
	-rm lib/*.d examples/*.d test/*.d

realclean: clean
	-rm $(library)

-include $(patsubst %.o,%.d,$(objects_all))


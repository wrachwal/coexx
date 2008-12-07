# $Id$

OS := $(shell uname -o)

ifeq (Cygwin,$(OS))
CXX = g++-4
EXEEXT = .exe
else
CXX = g++
EXEEXT =
ifeq (Solaris,$(OS))
  CXXFLAGS += -D_POSIX_C_SOURCE
  LDLIBS   += -lpthread -lrt
else
ifeq (Linux,$(findstring Linux,$(OS)))
  LDLIBS   += -lpthread -lrt
endif
endif
endif

CXX 	 += -MMD
CXXFLAGS += -g -O2 -Wall -pedantic -ansi -Iinclude

vpath %.o lib test examples

library := lib/libcoe.a
objects := $(patsubst src/%.cpp,lib/%.o,$(wildcard src/*.cpp))

examples := $(addprefix examples/,myhouse tick)
tests    := $(addprefix test/,typeinfo list cond)

objects_all := $(objects) $(patsubst %.cpp,%.o,$(wildcard examples/*.cpp test/*.cpp))

.PHONY: all examples tests clean mostlyclean

.INTERMEDIATE: $(objects)

lib/%.o: src/%.cpp 
	$(CXX) $(CXXFLAGS) -c -o $@ $<

lib/%.o: CXXFLAGS += -Wno-invalid-offsetof -Wno-strict-aliasing

$(library): $(library)($(objects))

all: $(library) examples tests

examples: $(examples)

tests: $(tests)

$(addsuffix .o,$(examples) $(tests)): %.o: %.cpp
	$(CXX) $(CXXFLAGS) -c -o $@ $<

test/list.o: CXXFLAGS += -Wno-invalid-offsetof
test/cond.o: CXXFLAGS += -Wno-missing-braces

$(examples) $(tests): %: %.o
	$(CXX) $(LDFLAGS) $< $(LDLIBS) -o $@

$(examples) $(tests): $(library)
$(examples) $(tests): LDLIBS += -Llib -lcoe

clean: mostlyclean
	-rm $(library)

mostlyclean:
	-rm $(addsuffix $(EXEEXT),$(examples) $(tests))
	-rm lib/*.o examples/*.o test/*.o
	-rm lib/*.d examples/*.d test/*.d

-include $(patsubst %.o,%.d,$(objects_all))


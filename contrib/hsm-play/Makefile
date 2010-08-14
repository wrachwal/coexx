CXX = g++ -MMD

CXXFLAGS += -g -Wall -pedantic -ansi -I../../include

OS := $(shell uname -o)

ifeq (Solaris,$(OS))
  CXXFLAGS += -D_POSIX_C_SOURCE
  LDLIBS   += -lpthread -lrt
else
ifeq (Linux,$(findstring Linux,$(OS)))
  LDLIBS   += -lpthread -lrt
endif
endif

.PHONY: all clean check brief force

PROG := hsm-play

ALL := $(PROG)

%.o: %.cpp
	$(CXX) -c $(CXXFLAGS) -o $@ $<

all: $(ALL)

../../lib/libcoe.a:
	$(MAKE) -C ../..

OBJ = $(patsubst %.cpp,%.o,$(wildcard *.cpp))

$(PROG): $(OBJ) \
    ../../lib/libcoe.a
	$(CXX) $(LDFLAGS) -o $@ $^ $(LDLIBS)

clean:
	$(RM) *.o *.d $(ALL) $(patsubst %,%.exe,$(ALL))

%.hsm: $(PROG) force
	./$(PROG) < $@ | diff $@ -

check: $(wildcard *.hsm)

brief:
	@for i in *.hsm; do \
	    echo "\n\n========== " $$i "\n"; \
	    grep '^[:$$@~]' $$i; \
	done

-include $(patsubst %.cpp,%.d,$(wildcard *.cpp))


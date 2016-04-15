# vim: noexpandtab

TARGETS += apsp bc bfs
LDLIBS += -lpthread -lrt

all:

LIBDIR=libs
OBJDIR=objs
BINDIR=bin


sources += apps/apsp/apsp.cc
sources += apps/bc/bc.cc
sources += apps/bfs/bfs.cc

sources += apps/common/graph.cc
sources += apps/roi/roi.cc

$(BINDIR)/apsp: apps/apsp/apsp.o apps/common/graph.o apps/roi/roi.o
$(BINDIR)/bc: apps/bc/bc.o apps/common/graph.o apps/roi/roi.o
$(BINDIR)/bfs: apps/bfs/bfs.o apps/common/graph.o apps/roi/roi.o

apps/roi/roi.o: CPPFLAGS += -DENABLE_PTLCALLS
apps/roi/roi.o: CPPFLAGS += -DENABLE_TIMING
apps/roi/roi.o: CPPFLAGS += -DROI_PREFIX=***


CC = gcc
CXX = g++
MD = mkdir -p
ECHO = echo

CPPFLAGS += -I./include -g -O2 -Wall -Werror -L./libs
CFLAGS +=
CXXFLAGS += --std=c++0x 

-include $(addsuffix .d,$(basename $(sources)))

.PHONY: all clean

OUTPUTS=$(addprefix $(BINDIR)/,$(TARGETS))
all: $(OUTPUTS)

clean:
	rm -f $(OUTPUTS) \
		$(addsuffix .o,$(basename $(sources))) \
		$(addsuffix .d,$(basename $(sources))) \
		*.dot $(LIBDIR)/*.a

%.d: %.c
	$(CC) $(CPPFLAGS) $(CFLAGS) -MMD -MF $@ -MP -MT $(addsuffix .o,$(basename $<)) -c $<

%.d: %.cc
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -MMD -MF $@ -MP -MT $(addsuffix .o,$(basename $<)) -c $<
	
%.o: %.c
	@$(MD) $(@D)
	$(CC) $(CPPFLAGS) $(CFLAGS) -c $< -o $@

%.o: %.cc
	@$(MD) $(@D)
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -c $< -o $@

$(BINDIR)/%: 
	@$(MD) $(@D)
	$(CXX) $(LDFLAGS) $^ -o $@ $(LDLIBS)


CXX = g++
CXXFLAGS = -g --std=c++0x -O2
CXXFLAGS += -Wall -Werror
CXXFLAGS += -I$(CRONO_DIR)/include
CXXFLAGS += -L$(CRONO_DIR)/libs

all: $(TRGS)

clean:
	rm -f $(TRGS) *.o *.dot

%.o: %.cc
	$(CXX) $(CXXFLAGS) -c $< -o $@

%: %.o
	$(CXX) $(CXXFLAGS) $^ -o $@ $(LIBS)

%.a:
	$(AR) $(ARFLAGS) $@ $^


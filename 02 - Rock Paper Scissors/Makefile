.DEFAULT_GOAL := all

CXX=g++
CXXFLAGS=--std=c++20 -Os -Wall -Wextra -Wpedantic
LD=$(CXX)
LDFLAGS=-Wl,-O1 -Wl,--as-needed
RM=rm

%.o: %.cpp
	$(CXX) -o $@ $(CXXFLAGS) -c $<

part1: part1.o
	$(LD) -o $@ $(LDFLAGS) $^

part2: part2.o
	$(LD) -o $@ $(LDFLAGS) $^

.PHONY: clean debug part1-debug part2-debug all

clean:
		$(RM) -f part1 part2 *.o

debug: CXXFLAGS += -DDEBUG -g -O0 -no-pie
debug: $(.DEFAULT_GOAL)

part1-debug: CXXFLAGS += -DDEBUG -g -O0 -no-pie
part1-debug: part1

part2-debug: CXXFLAGS += -DDEBUG -g -O0 -no-pie
part2-debug: part2

all: part1 part2

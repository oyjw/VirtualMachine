CXX=g++
CPPFLAGS=-std=c++11 -g -wall -gdwarf-2 -O0
OBJECTS:=$(patsubst %.cpp,%.o,$(wildcard *.cpp))
all:$(OBJECTS)
	$(CXX) -o vm $(CPPFLAGS) $(OBJECTS)
.PHONY:clean
clean:
	rm vm *.o

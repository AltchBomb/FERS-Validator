CXX=g++
CXXFLAGS=-Wall -Wextra -pedantic -std=c++11
LDFLAGS=-lxerces-c

SRCS=validator.cpp
OBJS=$(subst .cpp,.o,$(SRCS))

INCLUDE_DIRS=-I/opt/homebrew/Cellar/xerces-c/3.2.4_1/include
LIB_DIRS=-L/opt/homebrew/Cellar/xerces-c/3.2.4_1/lib

.PHONY: all clean

all: validator

validator: $(OBJS)
	$(CXX) $(LIB_DIRS) $(LDFLAGS) $^ -o $@

%.o: %.cpp
	$(CXX) $(INCLUDE_DIRS) $(CXXFLAGS) -c $< -o $@

clean:
	$(RM) $(OBJS) validator


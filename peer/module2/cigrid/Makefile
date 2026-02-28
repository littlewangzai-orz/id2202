# Makefile for a project written in C++.

CXX = g++
CXXFLAGS = -std=c++17 -Wall -g -DLINE_ERR -I$(CURDIR)
RM = rm -f

EXEC = cigrid
SRCS = scanner.cpp parser.cpp ir.cpp asm_ir.cpp cigrid.cpp
LIBS = libreflex.a 
OBJS = $(SRCS:.cpp=.o)

$(EXEC): %: $(OBJS)
	$(CXX) $(CXXFLAGS) -o $(EXEC) $(OBJS) $(LIBS)

%.o: %.c
	$(CXX) $(CXXFLAGS) -c $(SRCS)
	
.PHONY: clean
clean:
	$(RM) $(OBJS) $(EXEC)

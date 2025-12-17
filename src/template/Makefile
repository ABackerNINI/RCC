# Build details

CPP_COMPILER  ?= g++
CPP_STD       ?= c++17

CXX = $(CPP_COMPILER)
CXXFLAGS = -W -Wall -std=$(CPP_STD) -Wno-unused-parameter
BD = ./build

# Compile to objects

$(BD)/%.o: %.cpp
	@$(if $(wildcard $(BD)),,mkdir -p $(BD))
	$(CXX) -c -o $@ $< $(CXXFLAGS)

# Pre-Compiled Header

# for g++ PCH
%.hpp.gch: %.hpp
	$(CXX) $(CXXFLAGS) -x c++-header -o $@ $<

# for clang++ PCH
%.hpp.pch: %.hpp
	$(CXX) $(CXXFLAGS) -Xclang -emit-pch -o $@ $<

# Build Executable

pch = ""

ifeq ($(CPP_COMPILER), g++)
	pch = rcc_template.hpp.gch
else ifeq ($(CPP_COMPILER), clang++)
	pch = rcc_template.hpp.pch
endif

bin = test

all: $(pch)

rebuild: clean all

# Executable 1

obj = test.o
objbd = $(obj:%=$(BD)/%)

$(bin): $(pch) $(objbd)
	$(CXX) $(CXXFLAGS) -g -I. -o $(bin) $(objbd)

# Dependencies

test.o: rcc_template.hpp test.cpp

# Clean up

clean:
	rm -f "$(pch)" "$(bin)" $(objbd)
	rm -fd "$(BD)"

# PHONY

.PHONY: all rebuild clean

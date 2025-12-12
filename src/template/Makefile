# Build details

CXX = g++
CXXFLAGS = -W -Wall -std=$(CPP_STD) -Wno-unused-parameter
BD = ./build

# Compile to objects

$(BD)/%.o: %.cpp
	@$(if $(wildcard $(BD)),,mkdir -p $(BD))
	$(CXX) -c -o $@ $< $(CXXFLAGS)

# Pre-Compiled Header

%.h.gch: %.h
	$(CXX) -x c++-header -c -o $@ $< $(CXXFLAGS)

# Build Executable

pch = rcc_template.h.gch
bin = test

all: $(pch)

rebuild: clean all

# Executable 1

obj = test.o
objbd = $(obj:%=$(BD)/%)

$(bin): $(pch) $(objbd)
	$(CXX) -g -I. -o $(bin) $(objbd) $(CXXFLAGS)

# Dependencies

test.o: rcc_template.h test.cpp

# Clean up

clean:
	rm -f "$(pch)" "$(bin)" $(objbd)
	rm -fd "$(BD)"

# PHONY

.PHONY: all rebuild clean

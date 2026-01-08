# Build details

CPP_COMPILER  ?= g++
CPP_STD       ?= c++17

CXX = $(CPP_COMPILER)
CXXFLAGS = -Wall -Wextra -std=$(CPP_STD) -Wno-unused-parameter
BD = ./build

# Pre-Compiled Header

PCH = rcc_template.hpp
GCC_GCH_DIR = $(PCH).gch

GCH = ""

ifeq ($(CXX), g++)
	GCH = $(GCC_GCH_DIR)/rcc_template.hpp.$(CXX).$(CPP_STD).default.gch \
		  $(GCC_GCH_DIR)/rcc_template.hpp.$(CXX).$(CPP_STD).stdc++.gch
else ifeq ($(CXX), clang++)
	GCH = rcc_template.hpp.pch rcc_template.hpp.stdc++.pch
endif

# for g++ PCH
$(GCC_GCH_DIR)/%.default.gch: $(PCH)
	@mkdir -p $(GCC_GCH_DIR)
	$(CXX) $(CXXFLAGS) -x c++-header -o $@ $<
$(GCC_GCH_DIR)/%.stdc++.gch: $(PCH)
	@mkdir -p $(GCC_GCH_DIR)
	$(CXX) $(CXXFLAGS) -x c++-header -DINCLUDE_BITS_STDCPP_H -o $@ $<

# for clang++ PCH
%.hpp.pch: $(PCH)
	$(CXX) $(CXXFLAGS) -x c++-header -o $@ $<
%.hpp.stdc++.pch: $(PCH)
	$(CXX) $(CXXFLAGS) -x c++-header -DINCLUDE_BITS_STDCPP_H -o $@ $<

# Build Executable

TARGET = test

all: $(GCH)

rebuild: clean all

# Executable 1

OBJ = test.o
OBJBD = $(OBJ:%=$(BD)/%)

$(TARGET): $(PCH) $(OBJBD)
	$(CXX) $(CXXFLAGS) -g -I. -o $(TARGET) $(OBJBD)

# Compile to objects

$(BD)/%.o: %.cpp
	@$(if $(wildcard $(BD)),,mkdir -p $(BD))
	$(CXX) -c -o $@ $< $(CXXFLAGS)

# Dependencies

test.o: rcc_template.hpp test.cpp

# Clean up

clean:
	rm -f *.gch *.pch
	rm -f "$(TARGET)" $(OBJBD)
	rm -fd "$(GCC_GCH_DIR)" "$(BD)"

# PHONY

.PHONY: all rebuild clean

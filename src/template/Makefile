# Build details

CPP_COMPILER  ?= g++
CPP_STD       ?= c++17

CXX = $(CPP_COMPILER)
CXXFLAGS = -Wall -Wextra -std=$(CPP_STD) -Wno-unused-parameter
BD = ./build

# Pre-Compiled Header

GCC_GCH_DIR = rcc_template.hpp.gch

PCH = ""

ifeq ($(CPP_COMPILER), g++)
	PCH = $(GCC_GCH_DIR)/rcc_template.hpp.gch $(GCC_GCH_DIR)/rcc_template.hpp.stdc++.gch
else ifeq ($(CPP_COMPILER), clang++)
	PCH = rcc_template.hpp.pch rcc_template.hpp.stdc++.pch
endif

# for g++ PCH
$(GCC_GCH_DIR)/%.hpp.gch: %.hpp
	@mkdir -p $(GCC_GCH_DIR)
	$(CXX) $(CXXFLAGS) -x c++-header -o $@ $<
$(GCC_GCH_DIR)/%.hpp.stdc++.gch: %.hpp
	@mkdir -p $(GCC_GCH_DIR)
	$(CXX) $(CXXFLAGS) -x c++-header -DINCLUDE_BITS_STDCPP_H -o $@ $<

# for clang++ PCH
%.hpp.pch: %.hpp
	$(CXX) $(CXXFLAGS) -x c++-header -o $@ $<
%.hpp.stdc++.pch: %.hpp
	$(CXX) $(CXXFLAGS) -x c++-header -DINCLUDE_BITS_STDCPP_H -o $@ $<

# Build Executable

TARGET = test

all: $(PCH)

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

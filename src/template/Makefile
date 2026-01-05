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

gcc_gch_dir = rcc_template.hpp.gch

# for g++ PCH
$(gcc_gch_dir)/%.hpp.gch: %.hpp
	$(CXX) $(CXXFLAGS) -x c++-header -o $@ $<
$(gcc_gch_dir)/%.hpp.stdc++.gch: %.hpp
	$(CXX) $(CXXFLAGS) -x c++-header -DINCLUDE_BITS_STDCPP_H -o $@ $<

# for clang++ PCH
%.hpp.pch: %.hpp
	$(CXX) $(CXXFLAGS) -x c++-header -o $@ $<
%.hpp.stdc++.pch: %.hpp
	$(CXX) $(CXXFLAGS) -x c++-header -DINCLUDE_BITS_STDCPP_H -o $@ $<

# Build Executable

pch = ""

ifeq ($(CPP_COMPILER), g++)
	pch = create_gch_dir_for_gcc $(gcc_gch_dir)/rcc_template.hpp.gch $(gcc_gch_dir)/rcc_template.hpp.stdc++.gch
else ifeq ($(CPP_COMPILER), clang++)
	pch = rcc_template.hpp.pch rcc_template.hpp.stdc++.pch
endif

bin = test

all: $(pch)

rebuild: clean all

create_gch_dir_for_gcc:
	mkdir -p $(gcc_gch_dir)

# Executable 1

obj = test.o
objbd = $(obj:%=$(BD)/%)

$(bin): $(pch) $(objbd)
	$(CXX) $(CXXFLAGS) -g -I. -o $(bin) $(objbd)

# Dependencies

test.o: rcc_template.hpp test.cpp

# Clean up

clean:
	rm -f *.gch *.pch
	rm -f "$(bin)" $(objbd)
	rm -fd "$(gcc_gch_dir)" "$(BD)"

# PHONY

.PHONY: all rebuild clean

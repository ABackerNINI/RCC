# Build details

CXX = g++
# there should be a space before "-std"
CXXFLAGS  = -W -Wall -g -std=$(CPP_STD)     \
	"-DCPP_STD=\" -std=$(CPP_STD)\""        \
	"-DRCC_CACHE_DIR=\"$(RCC_CACHE_DIR)\""

CXXLFLAGS = -W -Wall -g -std=$(CPP_STD)

BD = ./build

# Compile to objects

$(BD)/%.o: %.cpp
	@$(if $(wildcard $(BD)),,mkdir -p $(BD))
	$(CXX) -c -o $@ $< $(CXXFLAGS)

# Build Executable

bin = rcc

all: $(bin)

rebuild: clean all

# Executable 1

obj = rcc.o
objbd = $(obj:%=$(BD)/%)

$(bin): $(objbd)
	$(CXX) -o $(bin) $(objbd) $(CXXLFLAGS)

# Dependencies

rcc.o: rcc.cpp

# Clean up

clean:
	rm -f "$(bin)" $(objbd)
	rm -fd "$(BD)"

# PHONY

.PHONY: all rebuild install clean

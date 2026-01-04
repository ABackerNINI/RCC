# BUILD DETAILS

CPP_COMPILER  ?= g++
CPP_STD       ?= c++17
RCC_CACHE_DIR ?= $(HOME)/.cache/rcc

CXX      = $(CPP_COMPILER)
CXXFLAGS = -W -Wall -std=$(CPP_STD)     			\
		   -DRCC_COMPILER=\"$(CPP_COMPILER)\"		\
		   -DRCC_CPP_STD=\"-std=$(CPP_STD)\"      	\
		   -DRCC_CACHE_DIR=\"$(RCC_CACHE_DIR)\"
LDFLAGS  = 
BUILD    = build

# TARGETS

TARGET = rcc

.PHONY: default debug release clean

default: debug

debug: CXXFLAGS += -g -DDEBUG=1
debug: $(BUILD)/debug.mode $(TARGET)

release: CXXFLAGS += -O3# -DNDEBUG=1
release: $(BUILD)/release.mode $(TARGET)

clean:
	rm -rf "$(TARGET)" "$(BUILD)"

# EXECUTABLE DETAILS

OBJS = rcc.o paths.o compiler_support.o settings.o

OBJSBD = $(OBJS:%=$(BUILD)/%)

$(TARGET): $(OBJSBD)
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LDFLAGS)

# COMPILE TO OBJECTS

$(BUILD)/%.o: %.cpp
	@mkdir -p "$(BUILD)"
	$(CXX) $(CXXFLAGS) -c -o $@ $<

# MODE CONTROL

$(BUILD)/%.mode:
	@if [ ! -f "$@" ]; then \
		if [ -d "$(BUILD)" ]; then \
			echo "Switching to $* mode..."; \
			$(MAKE) clean; \
		fi; \
		mkdir -p "$(BUILD)"; \
		touch "$@"; \
	fi

# DEPENDENCIES

SRC = $(wildcard *.h *.hpp *.c *.cpp *.cc)

$(BUILD)/depend.mk: $(SRC)
	@mkdir -p "$(BUILD)"
	@$(CXX) $(CXXFLAGS) -MM *.cpp $(LDFLAGS) | sed 's/^\(.*\).o:/$$(BUILD)\/\1.o:/' > $@

include $(BUILD)/depend.mk

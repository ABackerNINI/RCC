# BUILD DETAILS

CPP_COMPILER  ?= g++
CPP_STD       ?= c++17
RCC_CACHE_DIR ?= $(HOME)/.cache/rcc
BUILD_PCH 	  ?= TRUE

CXX      = $(CPP_COMPILER)
CXXFLAGS = -Wall -Wextra -std=$(CPP_STD) -I.. 		\
		   -DRCC_COMPILER=\"$(CPP_COMPILER)\"		\
		   -DRCC_CPP_STD=\"-std=$(CPP_STD)\"      	\
		   -DRCC_CACHE_DIR=\"$(RCC_CACHE_DIR)\"
LDFLAGS  = 
LDLIBS   = -L../libs -lfmt
BUILD    = build

PCH 	 = pch.h
PCH_GCH  = $(PCH).gch

# TARGETS

TARGET 	= rcc

DEBUG_DEP  = $(BUILD)/debug.mode $(TARGET)
RELEASE_DEP = $(BUILD)/release.mode $(TARGET)

ifeq ($(BUILD_PCH), TRUE)
DEBUG_DEP   := $(PCH_GCH)/$(PCH).$(CXX).$(CPP_STD).debug.gch $(DEBUG_DEP)
RELEASE_DEP := $(PCH_GCH)/$(PCH).$(CXX).$(CPP_STD).release.gch $(RELEASE_DEP)
else ifeq ($(CXX), clang++)
CXXFLAGS += -Wno-ignored-gch
endif

.PHONY: default debug release clean clean_pch

default: debug

debug: CXXFLAGS += -g -DDEBUG=1
debug: LDFLAGS += -g
debug: $(DEBUG_DEP)

release: CXXFLAGS += -O3# -DNDEBUG=1
release: LDFLAGS += -O3
release: $(RELEASE_DEP)

clean:
	rm -rf "$(TARGET)" "$(BUILD)"

clean_pch:
	rm -rf "$(PCH_GCH)"

# EXECUTABLE DETAILS

OBJS = rcc.o paths.o compiler_support.o settings.o utils.o

OBJSBD = $(OBJS:%=$(BUILD)/%)

$(TARGET): $(OBJSBD)
	$(CXX) $(LDFLAGS) -o $@ $^ $(LDLIBS)

# COMPILE TO OBJECTS

$(BUILD)/%.o: %.cpp
	@mkdir -p "$(BUILD)"
	$(CXX) $(CXXFLAGS) -c -o $@ -include $(PCH) $<

# TODO: optimize this
define build_pch
	@mkdir -p $(1).gch
	@echo "Building PCH for $(1) -> $(1).gch/$(notdir $(1)).$(CXX).$(CPP_STD).$(2).gch"
	$(CXX) $(CXXFLAGS) -x c++-header $(1) -o "$(1).gch/$(notdir $(1)).$(CXX).$(CPP_STD).$(2).gch"
	@echo ""
endef

$(PCH_GCH)/%.debug.gch: $(PCH)
	$(call build_pch,$<,debug)

$(PCH_GCH)/%.release.gch: $(PCH)
	$(call build_pch,$<,release)

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
	@$(CXX) $(CXXFLAGS) -MM *.cpp | sed 's/^\(.*\).o:/$$(BUILD)\/\1.o:/' > $@

include $(BUILD)/depend.mk

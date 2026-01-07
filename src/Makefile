# BUILD DETAILS

CPP_COMPILER  ?= g++
CPP_STD       ?= c++17
RCC_CACHE_DIR ?= $(HOME)/.cache/rcc

BUILD_PCH 	  ?= TRUE

CXX      = $(CPP_COMPILER)
CXXFLAGS = -Wall -Wextra -std=$(CPP_STD)     		\
		   -DRCC_COMPILER=\"$(CPP_COMPILER)\"		\
		   -DRCC_CPP_STD=\"-std=$(CPP_STD)\"      	\
		   -DRCC_CACHE_DIR=\"$(RCC_CACHE_DIR)\"
LDFLAGS  = -Llibs -lfmt
BUILD    = build

PCH 	 = pch.h
PCH_GCH  = $(PCH).gch

# TARGETS

TARGET 	= rcc

DEBUG_DEP  = $(BUILD)/debug.mode $(TARGET)
RELEASE_DEP = $(BUILD)/release.mode $(TARGET)

ifeq ($(BUILD_PCH), TRUE)
DEBUG_DEP := _build_pch_debug $(DEBUG_DEP)
RELEASE_DEP := _build_pch_release $(RELEASE_DEP)
endif

.PHONY: default debug release clean clean_pch _build_pch_debug _build_pch_release

default: debug

debug: CXXFLAGS += -g -DDEBUG=1
debug: $(DEBUG_DEP)

release: CXXFLAGS += -O3# -DNDEBUG=1
release: $(RELEASE_DEP)

clean:
	rm -rf "$(TARGET)" "$(BUILD)"

clean_pch:
	rm -rf "$(PCH_GCH)"

_build_pch_debug: $(PCH_GCH)/$(PCH).$(CPP_COMPILER).$(CPP_STD).debug.gch
_build_pch_release: $(PCH_GCH)/$(PCH).$(CPP_COMPILER).$(CPP_STD).release.gch

# EXECUTABLE DETAILS

OBJS = rcc.o paths.o compiler_support.o settings.o

OBJSBD = $(OBJS:%=$(BUILD)/%)

$(TARGET): $(OBJSBD)
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LDFLAGS)

# COMPILE TO OBJECTS

$(BUILD)/%.o: %.cpp
	@mkdir -p "$(BUILD)"
	$(CXX) $(CXXFLAGS) -c -o $@ -include $(PCH) $<

define build_pch
	@mkdir -p $(PCH_GCH)
	@echo "Building PCH for $(1)"
	$(CXX) $(CXXFLAGS) -x c++-header $(1) -o "$(PCH_GCH)/$(notdir $(1)).$(CPP_COMPILER).$(CPP_STD).$(2).gch"
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

PROJECT_ROOT := $(CURDIR)

CUR_MAKEFILE := $(lastword $(MAKEFILE_LIST))

# ======================================================================================================================
# PROJECT

PROJECT := RCC_TEMPLATE

# ======================================================================================================================
# OPTIONS

MODE := debug

CXX := g++
CXXSTD := c++17

# ======================================================================================================================
# BUILD DETAILS

CXXFLAGS = -Wall -Wextra -std=$(CXXSTD)

ifeq ($(MODE),debug)
	CXXFLAGS += -g -O0 -DDEBUG
else ifeq ($(MODE),release)
	CXXFLAGS += -flto=auto -O3 -march=native -DNDEBUG
endif

SRC := rcc_template.hpp
DIR := $(SRC).gch

SIGNATURE_ARGS := $(CXXFLAGS)
SIGNATURE := $(shell echo "a$(SIGNATURE_ARGS)b" | md5sum | cut -c1-12)

PREFIX := $(CXX).$(CXXSTD).$(MODE).$(SIGNATURE)
TARGETS := $(DIR)/$(PREFIX).default.gch $(DIR)/$(PREFIX).stdc++.gch

default: all

# ======================================================================================================================
# RULES

$(DIR)/%.default.gch: $(SRC)
	@mkdir -p $(@D)
	$(CXX) $(CXXFLAGS) -x c++-header $< -o $@

$(DIR)/%.stdc++.gch: $(SRC)
	@mkdir -p $(@D)
	$(CXX) $(CXXFLAGS) -x c++-header -DINCLUDE_BITS_STDCPP_H $< -o $@

# ======================================================================================================================
# PHONY TARGETS

all: $(TARGETS)

debug:
	@$(MAKE) -f $(CUR_MAKEFILE) MODE=debug --no-print-directory

release:
	@$(MAKE) -f $(CUR_MAKEFILE) MODE=release --no-print-directory

clean:
	rm -rf $(DIR)

.PHONY: default all debug release clean

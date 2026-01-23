PROJECT_ROOT := $(CURDIR)

CUR_MAKEFILE := $(lastword $(MAKEFILE_LIST))

# ======================================================================================================================
# PROJECT

PROJECT := RCC_TEMPLATE

# ======================================================================================================================
# OPTIONS

CXX := g++
CXXSTD := c++17

# ======================================================================================================================
# BUILD DETAILS

CXXFLAGS = -g0 -O0 -Wall -Wextra -std=$(CXXSTD)

SRC := rcc_template.hpp
DIR := $(SRC).gch

SIGNATURE_ARGS := $(CXXFLAGS)
SIGNATURE := $(shell echo "a$(SIGNATURE_ARGS)b" | md5sum | cut -c1-12)

PREFIX := $(CXX).$(CXXSTD).$(SIGNATURE)
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

clean:
	rm -rf $(DIR)

.PHONY: default all debug release clean

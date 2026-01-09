PROJECT = rcc

# OPTIONS

CONFIG 		  ?= debug

CXX 		  ?= g++
CPP_STD       ?= c++17
RCC_CACHE_DIR ?= $(HOME)/.cache/rcc

# SOURCES

SRC_DIR = src
SRCS = $(wildcard $(SRC_DIR)/*.cpp)
HDRS = $(wildcard $(SRC_DIR)/*.h)

# CONFIGURATIONS

PARAMS = $(RCC_CACHE_DIR)
PARAMS_SIGNATURE = $(shell echo "$(PARAMS)" | md5sum | cut -c1-12)
BUILD_DIR = build/$(CONFIG)/$(CXX).$(CPP_STD).$(PARAMS_SIGNATURE)
BIN_DIR = bin

CXXFLAGS = -Wall -Wextra -std=$(CPP_STD) 			\
		   -I. -I./libs/							\
		   -DRCC_COMPILER=\"$(CXX)\"		\
		   -DRCC_CPP_STD=\"-std=$(CPP_STD)\"      	\
		   -DRCC_CACHE_DIR=\"$(RCC_CACHE_DIR)\"
LDFLAGS  = -L./libs -lfmt

BINARY = $(PROJECT)

ifeq ($(CONFIG),debug)
    CXXFLAGS += -g -O0 -DDEBUG
else ifeq ($(CONFIG),release)
    CXXFLAGS += -flto=4 -O3 -march=native -DNDEBUG
else ifeq ($(CONFIG),test)
    CXXFLAGS += -g -O0 -DTEST -Igoogletest/include
    BINARY = $(PROJECT)_test
    LDFLAGS += -Lgoogletest/lib -lgtest -lgtest_main -lpthread
endif

# OBJECTS

OBJS = $(SRCS:$(SRC_DIR)/%.cpp=$(BUILD_DIR)/%.o)
DEPS = $(OBJS:.o=.d)

# MAIN TARGET

$(BIN_DIR)/$(BINARY): $(OBJS)
	@mkdir -p $(BIN_DIR)
	$(CXX) $(CXXFLAGS) $^ -o $@ $(LDFLAGS)
	$(call check_build_params)
	$(call save_build_params,$(BUILD_DIR)/build_params.txt)

# COMPILE RULE

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.cpp
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) -MMD -MP -c $< -o $@

# INCLUDE DEPENDENCIES

-include $(DEPS)

# PHONY TARGETS

.PHONY: debug release test clean all

debug:
	@$(MAKE) CONFIG=debug --no-print-directory

release:
	@$(MAKE) CONFIG=release --no-print-directory

test:
	@$(MAKE) CONFIG=test --no-print-directory

all: debug release

clean:
	rm -rf build/ bin/

# BUILD PARAMS

RED = \033[0;31m
RESET = \033[0m

define save_build_params
	@echo "PROJECT=$(PROJECT)" > "$(1)"
	@echo "CONFIG=$(CONFIG)" >> "$(1)"
	@echo "CXX=$(CXX)" >> "$(1)"
	@echo "CPP_STD=$(CPP_STD)" >> "$(1)"
	@echo "CXXFLAGS=$(CXXFLAGS)" >> "$(1)"
	@echo "RCC_CACHE_DIR=$(RCC_CACHE_DIR)" >> "$(1)"
endef

define check_build_params
	$(call save_build_params,$(BUILD_DIR)/temp.txt)

	@if [ -f $(BUILD_DIR)/build_params.txt ]; then \
		if ! diff -q "$(BUILD_DIR)/build_params.txt" "$(BUILD_DIR)/temp.txt" >/dev/null 2>&1 ; then \
			echo ""; \
			printf "$(RED)"; \
			echo "WARNING: Build params mismatch, most likely due to Makefile changes, or barely hash collision, try 'make clean' first."; \
			printf "$(RESET)"; \
			echo ""; \
			echo "Diff of build params:"; \
			diff --ignore-space-change --color --minimal "$(BUILD_DIR)/build_params.txt" "$(BUILD_DIR)/temp.txt"; \
			echo ""; \
		fi \
	fi
endef

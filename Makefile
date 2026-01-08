# 项目配置
PROJECT = rcc

CXX 		  ?= g++
CPP_STD       ?= c++17
RCC_CACHE_DIR ?= $(HOME)/.cache/rcc

# 源文件
SRC_DIR = src
SRCS = $(wildcard $(SRC_DIR)/*.cpp)
HDRS = $(wildcard $(SRC_DIR)/*.h)

# 配置管理
CONFIG ?= debug
PARAMS = $(RCC_CACHE_DIR)
PARAMS_SIGNATURE = $(shell echo "$(PARAMS)" | md5sum | cut -c1-12)
BUILD_DIR = build/$(CONFIG)/$(CXX).$(CPP_STD).$(PARAMS_SIGNATURE)
BIN_DIR = bin

CXXFLAGS = -Wall -Wextra -std=$(CPP_STD) -I. 		\
		   -DRCC_COMPILER=\"$(CPP_COMPILER)\"		\
		   -DRCC_CPP_STD=\"-std=$(CPP_STD)\"      	\
		   -DRCC_CACHE_DIR=\"$(RCC_CACHE_DIR)\"
LDFLAGS  = -L./libs -lfmt

BINARY = $(PROJECT)

# 配置特定的变量
ifeq ($(CONFIG),debug)
    CXXFLAGS += -g -O0 -DDEBUG
else ifeq ($(CONFIG),release)
    CXXFLAGS += -O3 -DNDEBUG -march=native
else ifeq ($(CONFIG),test)
    CXXFLAGS += -g -O0 -DTEST -Igoogletest/include
    BINARY = $(PROJECT)_test
    LDFLAGS += -Lgoogletest/lib -lgtest -lgtest_main -lpthread
endif

define check_build_params
	@if [ -f $(BUILD_DIR)/build_params.txt ]; then \
		echo "CONFIG=$(CONFIG)\nCXX=$(CXX)\nCPP_STD=$(CPP_STD)\nRCC_CACHE_DIR=$(RCC_CACHE_DIR)" > $(BUILD_DIR)/temp.txt; \
		if ! diff -q build/build_params.txt build/temp.txt >/dev/null 2>&1; then \
			echo "ERROR: Build params mismatch, maybe due to hash collision, or Makefile changes, try 'make clean' first"; \
			exit 1; \
		fi \
	fi
endef

define save_build_params
	@echo "CONFIG=$(CONFIG)\nCXX=$(CXX)\nCPP_STD=$(CPP_STD)\nRCC_CACHE_DIR=$(RCC_CACHE_DIR)" > $(BUILD_DIR)/build_params.txt
endef

# 对象文件
OBJS = $(SRCS:$(SRC_DIR)/%.cpp=$(BUILD_DIR)/%.o)
DEPS = $(OBJS:.o=.d)

# 主目标
$(BIN_DIR)/$(BINARY): $(OBJS)
	$(call check_build_params)
	@mkdir -p $(BIN_DIR)
	$(CXX) $(CXXFLAGS) $^ -o $@ $(LDFLAGS)
	$(call save_build_params)

# 编译规则
$(BUILD_DIR)/%.o: $(SRC_DIR)/%.cpp
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) -MMD -MP -c $< -o $@

# 包含依赖
-include $(DEPS)

# 快捷目标
.PHONY: debug release test clean all

debug:
	$(MAKE) CONFIG=debug

release:
	$(MAKE) CONFIG=release

test:
	$(MAKE) CONFIG=test

all: debug release

clean:
	rm -rf build/ bin/

# 运行目标
run: $(BIN_DIR)/$(BINARY)
	./$<

# 测试运行
test-run: test
	./bin/$(PROJECT)_test

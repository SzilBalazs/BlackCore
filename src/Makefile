CXX = g++
TARGET_FLAGS = -static -static-libgcc -static-libstdc++
ARCH=native
NAME = BlackCore
VERSION_MAJOR = 1
VERSION_MINOR = 0
OBJECT_DIR = objects
SOURCES := $(wildcard *.cpp)
OBJECTS := $(patsubst %.cpp, $(OBJECT_DIR)/%.o, $(SOURCES))

ifeq ($(OS),Windows_NT)
    uname_S := Windows
else
    uname_S := $(shell uname -s)
endif


ifeq ($(uname_S), Windows)
	SUFFIX = .exe
else
	SUFFIX =
endif

ifeq ($(ARCH), native)
	ARCH_FLAGS=-march=native
	DEFINE_FLAGS=-DNATIVE
endif

ifeq ($(ARCH), bmi2)
	ARCH_FLAGS=-mbmi2
	DEFINE_FLAGS=-DBMI2
endif

ifeq ($(ARCH), popcnt)
    ARCH_FLAGS=-mpopcnt
endif

CXXFLAGS = -std=c++20 -Ofast -flto -pthread -Wall $(DEFINE_FLAGS) $(ARCH_FLAGS)
EXE = $(NAME)_v$(VERSION_MAJOR)-$(VERSION_MINOR)$(SUFFIX)

default: $(EXE)

$(EXE): $(OBJECTS)
	@echo Linking $(NAME)
	@$(CXX) $(TARGET_FLAGS) $(CXXFLAGS) -o $@ $^
	@echo Build has finished.

$(OBJECTS): | $(OBJECT_DIR)

$(OBJECT_DIR):
	@mkdir $(OBJECT_DIR)

$(OBJECT_DIR)/%.o: %.cpp
	@echo Compiling $<
	@$(CXX) $(CXXFLAGS) -c -o $@ $<

all: clean build

build: $(EXE)

clean:
	@rm -r objects $(EXE) || true

.PHONY: all build clean

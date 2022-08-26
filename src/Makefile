CXX = g++
CXXFLAGS = -std=c++20 -O3 -march=native -flto -pthread -Wall
TARGET_FLAGS = -static -static-libgcc -static-libstdc++
NAME = BlackCore
VERSION_MAJOR = 1
VERSION_MINOR = 0
SUFFIX =

OBJECT_DIR = objects

SOURCES := $(wildcard *.cpp)
OBJECTS := $(patsubst %.cpp, $(OBJECT_DIR)/%.o, $(SOURCES))

ifeq ($(OS), Windows_NT)
	SUFFIX = .exe
endif

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

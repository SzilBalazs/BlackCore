CXX = g++
CXXFLAGS = -std=c++2a -O3 -march=native -flto=auto -pthread -Wall
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

all: build perft bench

build: $(EXE)

perft: $(EXE)
	@RESULT=$$(./$(EXE) perft) && \
	echo "$$RESULT" && \
	echo $$RESULT | grep -q "PERFT OK"

clean:
	@rm -r objects $(EXE) || true

.PHONY: all build uci perft bench clean

CXX = g++
CXXFLAGS = -std=c++20 -Ofast -march=native -flto=auto -pthread -Wall
NAME = BlackCore
VERSION = _v0
EXE = $(NAME)$(VERSION)

OBJECT_DIR = objects

SOURCES := $(wildcard *.cpp)
OBJECTS := $(patsubst %.cpp, $(OBJECT_DIR)/%.o, $(SOURCES))

default: $(EXE)

$(EXE): $(OBJECTS)
	@echo Linking $(NAME)
	@$(CXX) $(CXXFLAGS) -o $@ $^
	@echo Build has finished.

$(OBJECTS): | $(OBJECT_DIR)

$(OBJECT_DIR):
	@mkdir $(OBJECT_DIR)

$(OBJECT_DIR)/%.o: %.cpp
	@echo Compiling $<
	@$(CXX) $(CXXFLAGS) -c -o $@ $<

all: build perft bench

build: $(EXE)

uci: $(EXE)
	./$(EXE) uci

perft: $(EXE)
	@RESULT=$$(./$(EXE) perft) && \
	echo "$$RESULT" && \
	echo $$RESULT | grep -q "PERFT OK"

bench: $(EXE)
	@./$(EXE) bench

clean:
	@rm -r objects $(EXE) || true

.PHONY: all build uci perft bench clean

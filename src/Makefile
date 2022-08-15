CXX = g++
CXXFLAGS = -std=c++20 -Ofast -march=native -flto=auto -pthread -Wall
NAME = BlackCore
VERSION = _v0
TARGET = $(NAME)$(VERSION)

OBJECT_DIR = objects

SOURCES := $(wildcard *.cpp)
OBJECTS := $(patsubst %.cpp, $(OBJECT_DIR)/%.o, $(SOURCES))

default: $(TARGET)

$(TARGET): $(OBJECTS)
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

build: $(TARGET)

uci: $(TARGET)
	./$(TARGET) uci

perft: $(TARGET)
	@RESULT=$$(./$(TARGET) perft) && \
	echo "$$RESULT" && \
	echo $$RESULT | grep -q "PERFT OK"

bench: $(TARGET)
	@./$(TARGET) bench

clean:
	@rm -r objects $(TARGET) || true

.PHONY: all build uci perft bench clean

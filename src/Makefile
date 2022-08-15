CC = g++-10
CFLAGS = -std=c++20 -Ofast -march=native -flto=auto -pthread -Wall -Wno-class-memaccess
NAME = BlackCore
VERSION = _v0
TARGET = $(NAME)$(VERSION)

OBJECT_DIR = objects

SOURCES := $(wildcard *.cpp)
OBJECTS := $(patsubst %.cpp, $(OBJECT_DIR)/%.o, $(SOURCES))

default: $(TARGET)

$(TARGET): $(OBJECTS)
	@echo Linking $(NAME)
	@$(CC) $(CFLAGS) -o $@ $^
	@echo Build has finished.

$(OBJECTS): | $(OBJECT_DIR)

$(OBJECT_DIR):
	@mkdir $(OBJECT_DIR)

$(OBJECT_DIR)/%.o: %.cpp
	@echo Compiling $<
	@$(CC) $(CFLAGS) -c -o $@ $<

all: test bench

test: $(TARGET)
	@echo Starting perft test...
	@./$(TARGET) perft

bench: $(TARGET)
	@echo Starting benchmark...
	@./$(TARGET) bench

clean:
	@rm -r objects $(TARGET)

.PHONY: all test bench clean

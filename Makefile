# Compilers
CC = g++

# Compiler flags
override CXXFLAGS += -std=c++17

# Linker flags
override LDFLAGS += -flto

# Source files
SRC_FILES = \
	main.cpp \
	jsmin.cpp

# Installation prefix
PREFIX = /usr/local

TARGET = jsmin

all: $(TARGET)

$(TARGET): $(SRC_FILES)
	$(CC) $(CXXFLAGS) -o $@ $^ $(LDFLAGS)

install:
	mkdir -p $(PREFIX)/bin
	cp $(TARGET) $(PREFIX)/bin/

uninstall:
	rm -f $(PREFIX)/bin/$(TARGET)

clean:
	rm -f $(TARGET)

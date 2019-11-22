CC		= g++
TARGET	= servomotor-daemon
SRC		= main.cpp
DEPS	= ../../lib/pca9685v2.hpp

CC_FLAGS		= --std=gnu++17 -Wall -Wextra --pedantic -li2c
DEBUG_FLAGS		= 
RELEASE_FLAGS	= 

.PHONY: debug
debug: CC_FLAGS+=$(DEBUG_FLAGS)
debug: all

.PHONY: release
release: CC_FLAGS+=$(RELEASE_FLAGS)
release: all

all: $(TARGET)
$(TARGET): $(SRC) $(DEPS)
	$(CC) $(CC_FLAGS) -o $(TARGET) $(SRC)

.PHONY: clean
clean:
	rm $(TARGET)



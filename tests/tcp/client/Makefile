CC		= g++
TARGET	= tcpclient
SRC		= main.cpp
DEPS	=

CC_FLAGS		= --std=gnu++1z -Wall -Wextra --pedantic
DEBUG_FLAGS		= 
RELEASE_FLAGS	= 

.PHONY: debug
debug: CSC_FLAGS+=$(DEBUG_FLAGS)
debug: all

.PHONY: release
release: CSC_FLAGS+=$(RELEASE_FLAGS)
release: all

all: $(TARGET)
$(TARGET): $(SRC) $(DEPS)
	$(CC) $(CC_FLAGS) -o $(TARGET) $(SRC)

.PHONY: clean
clean:
	rm $(TARGET)


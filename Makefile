CFLAGS ?= -std=c11 -Wall -Wextra -Wpedantic -O2
CPPFLAGS = -D_POSIX_C_SOURCE=200809L
DEPFLAGS = -MMD -MP

TARGET = vm
SOURCES = $(wildcard src/*.c)
OBJECTS = $(patsubst src/%.c,build/%.o,$(SOURCES))

.PHONY: all clean

all: $(TARGET)

$(TARGET): $(OBJECTS)
	$(CC) $(CFLAGS) $(OBJECTS) -o $@ $(LDFLAGS)

build/%.o: src/%.c | build
	$(CC) $(CPPFLAGS) $(CFLAGS) $(DEPFLAGS) -c $< -o $@

build:
	mkdir -p build

clean:
	rm -rf build $(TARGET)

-include $(OBJECTS:.o=.d)

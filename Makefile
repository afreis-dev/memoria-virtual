CFLAGS ?= -std=c11 -Wall -Wextra -Wpedantic -O2
FRAMES ?= 128
TLB ?= on
USE_TLB = $(if $(filter off,$(TLB)),0,1)
CPPFLAGS = -D_POSIX_C_SOURCE=200809L -DFRAME_COUNT=$(FRAMES)u -DUSE_TLB=$(USE_TLB)
DEPFLAGS = -MMD -MP
BUILD_CONFIG = $(CPPFLAGS) $(CFLAGS)

TARGET = vm
SOURCES = $(wildcard src/*.c)
OBJECTS = $(patsubst src/%.c,build/%.o,$(SOURCES))

.PHONY: all clean test force

all: $(TARGET)

$(TARGET): $(OBJECTS)
	$(CC) $(CFLAGS) $(OBJECTS) -o $@ $(LDFLAGS)

build/%.o: src/%.c build/config | build
	$(CC) $(CPPFLAGS) $(CFLAGS) $(DEPFLAGS) -c $< -o $@

build/config: force | build
	@echo '$(BUILD_CONFIG)' | cmp -s - $@ || echo '$(BUILD_CONFIG)' > $@

build:
	mkdir -p build

force:

test: $(TARGET)
	FRAMES=$(FRAMES) TLB=$(TLB) sh tests/run.sh

clean:
	rm -rf build $(TARGET)

-include $(OBJECTS:.o=.d)

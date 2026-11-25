CFLAGS ?= -std=c11 -Wall -Wextra -Wpedantic -O2
FRAMES ?= 128
TLB ?= on
THREADS ?= on
SANITIZED ?=
USE_TLB = $(if $(filter off,$(TLB)),0,1)
USE_THREADS = $(if $(filter off,$(THREADS)),0,1)
CPPFLAGS = -D_POSIX_C_SOURCE=200809L -DFRAME_COUNT=$(FRAMES)u -DUSE_TLB=$(USE_TLB) -DUSE_THREADS=$(USE_THREADS) -pthread
LDLIBS = -pthread
DEPFLAGS = -MMD -MP
BUILD_CONFIG = $(CPPFLAGS) $(CFLAGS) $(LDFLAGS)
TSAN_FLAGS = -std=c11 -Wall -Wextra -Wpedantic -g -O1 -fno-omit-frame-pointer -fsanitize=thread
ASAN_FLAGS = -std=c11 -Wall -Wextra -Wpedantic -g -O1 -fno-omit-frame-pointer -fsanitize=address,undefined

TARGET = vm
SOURCES = $(wildcard src/*.c)
OBJECTS = $(patsubst src/%.c,build/%.o,$(SOURCES))

.PHONY: all clean test tsan sanitize mutantes force

all: $(TARGET)

$(TARGET): $(OBJECTS)
	$(CC) $(CFLAGS) $(OBJECTS) -o $@ $(LDFLAGS) $(LDLIBS)

build/%.o: src/%.c build/config | build
	$(CC) $(CPPFLAGS) $(CFLAGS) $(DEPFLAGS) -c $< -o $@

build/config: force | build
	@echo '$(BUILD_CONFIG)' | cmp -s - $@ || echo '$(BUILD_CONFIG)' > $@

build:
	mkdir -p build

force:

test: $(TARGET)
	FRAMES=$(FRAMES) TLB=$(TLB) THREADS=$(THREADS) SANITIZED=$(SANITIZED) sh tests/run.sh

tsan:
	$(MAKE) clean
	$(MAKE) CFLAGS="$(TSAN_FLAGS)" LDFLAGS="-fsanitize=thread" SANITIZED=thread test; status=$$?; $(MAKE) clean; exit $$status

sanitize:
	$(MAKE) clean
	$(MAKE) CFLAGS="$(ASAN_FLAGS)" LDFLAGS="-fsanitize=address,undefined" SANITIZED=address test; status=$$?; $(MAKE) clean; exit $$status

mutantes:
	sh tests/mutantes.sh

clean:
	rm -rf build $(TARGET)

-include $(OBJECTS:.o=.d)

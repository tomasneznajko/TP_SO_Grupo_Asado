# Globales
CC := gcc
CFLAGS := -Wall -Werror -fPIC
LDFLAGS := -shared
LIBRARY_NAME := ./debug/libshared.so
SOURCE_FILES := $(shell find ./src -type f -iname "*.c" -exec basename {} \;)
OBJECT_FILES := $(SOURCE_FILES:%.c=./debug/%.o)
LINK = -lpthread -lcommons -lm

### Reglas ###

build: makefolder $(LIBRARY_NAME)

debug: CFLAGS := $(CFLAGS) -g
debug: build

$(LIBRARY_NAME): $(OBJECT_FILES)
	$(CC) $(LDFLAGS) -o $@ $^ $(LINK)

./debug/%.o: ./src/%.c
	$(CC) $(CFLAGS) -c $< -o $@ $(LINK)

makefolder:
	if [ ! -d ./debug ]; then mkdir debug; fi

clean:
	rm -rf debug
	rm -rf *.out
	rm -rf *.log
#	rm -f $(LIBRARY_NAME) $(OBJECT_FILES)
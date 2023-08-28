# Globales
CC := gcc
CFLAGS := -Wall
LDFLAGS := -L../shared/debug -lshared
INCLUDE_PATH := ../shared/src
MODULE_NAME := ./debug/filesystem.out
SOURCE_FILES := $(shell find ./src -type f -iname "*.c" -exec basename {} \;)
OBJECT_FILES := $(SOURCE_FILES:%.c=./debug/%.o)
LINK = -lpthread -lcommons -lm
CONFIG_FILE := ./filesystem.config

### Reglas ###

build: makefolder makeshared build_structs $(MODULE_NAME)

debug: CFLAGS := $(CFLAGS) -g
debug: makeshared_debug build

rebuild: clean build

run: build
	$(MODULE_NAME) $(CONFIG_FILE)

valgrind: debug
	valgrind --leak-check=full --show-leak-kinds=all --log-file=valgrind.log $(MODULE_NAME) $(CONFIG_FILE)

valgrind_track: debug
	valgrind --leak-check=full --show-leak-kinds=all --log-file=valgrind.log --track-origins=yes $(MODULE_NAME) $(CONFIG_FILE)

helgrind: debug
	valgrind --tool=helgrind --log-file=valgrind.log $(MODULE_NAME) $(CONFIG_FILE)

$(MODULE_NAME): $(OBJECT_FILES)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS) -I$(INCLUDE_PATH) $(LINK)

./debug/%.o: ./src/%.c
	$(CC) $(CFLAGS) -c $< -o $@ -I$(INCLUDE_PATH) $(LINK)

makefolder:
	if [ ! -d ./debug ]; then mkdir debug; fi

makeshared:
	make -C ../shared

makeshared_debug:
	make debug -C ../shared

makefcb:
	if [ ! -d ./fcb ]; then mkdir fcb; fi

./%.dat:
	touch $@

build_structs: makefcb bloques.dat bitmap.dat

reset_structs: clean_structs build_structs

# Clean

clean: clean_structs
	rm -rf debug
	rm -rf *.out
	rm -rf *.log
	make clean -C ../shared

clean_structs:
	rm -rf fcb
	rm -f bitmap.dat
	rm -f bloques.dat

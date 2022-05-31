# GENERATED ON 2022-03-14 21:13 by utils/discover from utils/Makefile.skel

# --- PARAMETERS ---

# compilation directories
SOURCE_DIR := src
DEBUG_DIR := debug
RELEASE_DIR := release

# output binaries names
BINARY_NAME := ic4
TEST_BINARY_NAME := ic4_test

# entry points
MAIN_NAME := main.c
TEST_MAIN_NAME := test_main.c

# c compiler setup
CC := gcc
CFLAGS := -std=c11 -Wall -Wextra -Wpedantic
CDEBUG_FLAGS := -g -O0 -DDEBUG -DCHECK_ERRORLEVELS_VALIDITY -DCHECK_LOGTARGET_PTR -DCHECK_CONTEXT_CHILDS -DCHECK_UNGETTOKEN
CRELEASE_FLAGS := -Werror -O3
LDFLAGS := 

# system commands 
RM := rm -rf
MKDIR := mkdir -p

# discover script
DISCOVER_SCRIPT := utils/discover

# catalog script
CATALOG_SCRIPT := utils/catalog.py

# test discovery script
TEST_DISCOVERY_SCRIPT := utils/test_discovery.py
TEST_PREFIX := test_
TEST_MAIN_SKEL := utils/test_main.c.skel

# --- MAKE OPTIONS ---

# defaulting to debug build
ifndef BUILD
    BUILD := debug
endif

# adding debug flags
ifeq ($(BUILD), debug)
    BUILD_DIR := $(DEBUG_DIR)
    CFLAGS += $(CDEBUG_FLAGS)
else ifeq ($(BUILD), release)
    BUILD_DIR := $(RELEASE_DIR)
    CFLAGS += $(CRELEASE_FLAGS)
else
    $(error BUILD flag can only have the value "debug" or "release")
endif

# --- MAKEFILE REMAKING ---

# makefile name
MAKEFILE_NAME := $(lastword $(MAKEFILE_LIST))

# makefile skeleton name
MAKEFILE_SKEL := utils/Makefile.skel

# all targets depends on makefile components
.EXTRA_PREREQS := $(MAKEFILE_SKEL) $(DISCOVER_SCRIPT)

# discover script call
DISCOVER_ARGS :=  -source_dir $(SOURCE_DIR) -debug_dir $(DEBUG_DIR) -release_dir $(RELEASE_DIR) \
                  -binary_name $(BINARY_NAME) -test_binary_name $(TEST_BINARY_NAME) -main_name $(MAIN_NAME) -test_main_name $(TEST_MAIN_NAME)\
                  -catalog_script $(CATALOG_SCRIPT) -test_discovery_script $(TEST_DISCOVERY_SCRIPT) -test_prefix $(TEST_PREFIX) -test_main_skel $(TEST_MAIN_SKEL)

# --- SOURCE TREE CHANGES DETECTION ---

# md5 of the source tree at the generation of this file
CREATION_SRC_TREE_MD5 := 50b2b82bb95bf5aff1cfc539b4236a9b

# checking md5
ifneq ($(CREATION_SRC_TREE_MD5), $(shell find $(SOURCE_DIR) | sort | md5sum | cut -d " " -f1))
    $(shell $(RM) $(MAKEFILE_NAME)) # at the end make will detect the missing makefile and regenerate it
endif

# --- DISCOVERED SOURCES AND OBJECTS ---

# list of sources
SOURCES := $(SOURCE_DIR)/misc/context/context.c $(SOURCE_DIR)/misc/log/log.c $(SOURCE_DIR)/misc/queue.c $(SOURCE_DIR)/misc/xml.c $(SOURCE_DIR)/preprocessor/directives.c $(SOURCE_DIR)/preprocessor/lines.c $(SOURCE_DIR)/preprocessor/tokenizer.c
TEST_SOURCES := $(SOURCE_DIR)/misc/test_bookmark.c $(SOURCE_DIR)/misc/test_queue.c $(SOURCE_DIR)/preprocessor/test_directives.c $(SOURCE_DIR)/preprocessor/test_lines.c $(SOURCE_DIR)/preprocessor/test_tokenizer.c

# lists of object files needed
OBJECTS := $(BUILD_DIR)/misc/context/context.o $(BUILD_DIR)/misc/log/log.o $(BUILD_DIR)/misc/queue.o $(BUILD_DIR)/misc/xml.o $(BUILD_DIR)/preprocessor/directives.o $(BUILD_DIR)/preprocessor/lines.o $(BUILD_DIR)/preprocessor/tokenizer.o
MAIN_OBJ := $(BUILD_DIR)/main.o

TEST_OBJECTS := $(BUILD_DIR)/misc/test_bookmark.o $(BUILD_DIR)/misc/test_queue.o $(BUILD_DIR)/preprocessor/test_directives.o $(BUILD_DIR)/preprocessor/test_lines.o $(BUILD_DIR)/preprocessor/test_tokenizer.o
TEST_MAIN_OBJ := $(BUILD_DIR)/test_main.o

# list of catalogs to generate
CATALOG_FILES := $(SOURCE_DIR)/misc/context/context.cat $(SOURCE_DIR)/misc/log/log.cat $(SOURCE_DIR)/preprocessor/messages.cat
CATALOG_HEADERS := $(BUILD_DIR)/misc/context/context.cat.h $(BUILD_DIR)/misc/log/log.cat.h $(BUILD_DIR)/preprocessor/messages.cat.h

# binaries
BINARY := $(BUILD_DIR)/$(BINARY_NAME)
TEST_BINARY := $(BUILD_DIR)/$(TEST_BINARY_NAME)

# delete predefined rules
.SUFFIXES:

# --- PHONY TARGETS ---

.PHONY: all build test test_build catalogs clean

all: catalogs test build 

build: $(BINARY) 

test: test_build 
	$(TEST_BINARY) $(TEST_ARGS)

test_build: $(TEST_BINARY) 

catalogs: $(CATALOG_HEADERS)

clean: 
	$(RM) $(BUILD_DIR)

# --- LINKER TARGETS ---

$(BINARY): $(MAIN_OBJ) $(OBJECTS) | $(dir $(BINARY))
	$(CC) $(LDFLAGS) $(MAIN_OBJ) $(OBJECTS) -o $@

$(TEST_BINARY): $(TEST_MAIN_OBJ) $(OBJECTS) $(TEST_OBJECTS) | $(dir $(TEST_BINARY))
	$(CC) $(LDFLAGS) $(TEST_MAIN_OBJ) $(OBJECTS) $(TEST_OBJECTS) -o  $@

# --- COMPILE RULES ---

$(BUILD_DIR)/main.o: $(SOURCE_DIR)/main.c \
     \
     | \
    $(dir $(BUILD_DIR)/main.o)
	$(CC) $(CFLAGS)  -c $< -o $@

$(BUILD_DIR)/misc/context/context.o: $(SOURCE_DIR)/misc/context/context.c \
    $(SOURCE_DIR)/misc/bookmark.h $(SOURCE_DIR)/misc/context/context.h $(SOURCE_DIR)/misc/log/log.h $(SOURCE_DIR)/misc/log/loglevel.h \
    $(BUILD_DIR)/misc/context/context.cat.h | \
    $(dir $(BUILD_DIR)/misc/context/context.o)
	$(CC) $(CFLAGS) -I$(BUILD_DIR)/misc/context -c $< -o $@

$(BUILD_DIR)/misc/log/log.o: $(SOURCE_DIR)/misc/log/log.c \
    $(SOURCE_DIR)/misc/bookmark.h $(SOURCE_DIR)/misc/context/context.h $(SOURCE_DIR)/misc/context/contextio.h $(SOURCE_DIR)/misc/log/abortlevel.h $(SOURCE_DIR)/misc/log/log.h $(SOURCE_DIR)/misc/log/loglevel.h $(SOURCE_DIR)/misc/log/logtarget.h \
    $(BUILD_DIR)/misc/log/log.cat.h | \
    $(dir $(BUILD_DIR)/misc/log/log.o)
	$(CC) $(CFLAGS) -I$(BUILD_DIR)/misc/log -c $< -o $@

$(BUILD_DIR)/misc/queue.o: $(SOURCE_DIR)/misc/queue.c \
    $(SOURCE_DIR)/misc/queue.h \
     | \
    $(dir $(BUILD_DIR)/misc/queue.o)
	$(CC) $(CFLAGS)  -c $< -o $@

$(BUILD_DIR)/misc/xml.o: $(SOURCE_DIR)/misc/xml.c \
    $(SOURCE_DIR)/misc/xml.h \
     | \
    $(dir $(BUILD_DIR)/misc/xml.o)
	$(CC) $(CFLAGS)  -c $< -o $@

$(BUILD_DIR)/preprocessor/directives.o: $(SOURCE_DIR)/preprocessor/directives.c \
    $(SOURCE_DIR)/misc/bookmark.h $(SOURCE_DIR)/misc/charescape.h $(SOURCE_DIR)/misc/context/context.h $(SOURCE_DIR)/misc/log/log.h $(SOURCE_DIR)/misc/log/loglevel.h $(SOURCE_DIR)/misc/queue.h $(SOURCE_DIR)/preprocessor/directives.h $(SOURCE_DIR)/preprocessor/tokenizer.h \
    $(BUILD_DIR)/preprocessor/messages.cat.h | \
    $(dir $(BUILD_DIR)/preprocessor/directives.o)
	$(CC) $(CFLAGS) -I$(BUILD_DIR)/preprocessor -c $< -o $@

$(BUILD_DIR)/preprocessor/lines.o: $(SOURCE_DIR)/preprocessor/lines.c \
    $(SOURCE_DIR)/misc/bookmark.h $(SOURCE_DIR)/misc/context/context.h $(SOURCE_DIR)/misc/log/log.h $(SOURCE_DIR)/misc/log/loglevel.h $(SOURCE_DIR)/preprocessor/lines.h $(SOURCE_DIR)/preprocessor/linesco.h \
    $(BUILD_DIR)/preprocessor/messages.cat.h | \
    $(dir $(BUILD_DIR)/preprocessor/lines.o)
	$(CC) $(CFLAGS) -I$(BUILD_DIR)/preprocessor -c $< -o $@

$(BUILD_DIR)/preprocessor/tokenizer.o: $(SOURCE_DIR)/preprocessor/tokenizer.c \
    $(SOURCE_DIR)/misc/bookmark.h $(SOURCE_DIR)/misc/charescape.h $(SOURCE_DIR)/misc/context/context.h $(SOURCE_DIR)/misc/log/log.h $(SOURCE_DIR)/misc/log/loglevel.h $(SOURCE_DIR)/preprocessor/lines.h $(SOURCE_DIR)/preprocessor/tokenizer.h $(SOURCE_DIR)/preprocessor/tokenizerco.h \
    $(BUILD_DIR)/preprocessor/messages.cat.h | \
    $(dir $(BUILD_DIR)/preprocessor/tokenizer.o)
	$(CC) $(CFLAGS) -I$(BUILD_DIR)/preprocessor -c $< -o $@

# --- COMPILE TEST RULES ---

$(TEST_MAIN_OBJ): $(BUILD_DIR)/$(TEST_MAIN_NAME) | $(dir $(TEST_MAIN_OBJ))
	$(CC) $(CFLAGS)  -c $< -o $@

$(BUILD_DIR)/misc/test_bookmark.o: $(SOURCE_DIR)/misc/test_bookmark.c \
    $(SOURCE_DIR)/misc/bookmark.h \
     | \
    $(dir $(BUILD_DIR)/misc/test_bookmark.o)
	$(CC) $(CFLAGS)  -c $< -o $@

$(BUILD_DIR)/misc/test_queue.o: $(SOURCE_DIR)/misc/test_queue.c \
    $(SOURCE_DIR)/misc/queue.h \
     | \
    $(dir $(BUILD_DIR)/misc/test_queue.o)
	$(CC) $(CFLAGS)  -c $< -o $@

$(BUILD_DIR)/preprocessor/test_directives.o: $(SOURCE_DIR)/preprocessor/test_directives.c \
    $(SOURCE_DIR)/misc/bookmark.h $(SOURCE_DIR)/misc/charescape.h $(SOURCE_DIR)/misc/context/context.h $(SOURCE_DIR)/misc/log/log.h $(SOURCE_DIR)/misc/log/loglevel.h $(SOURCE_DIR)/misc/log/logtarget.h $(SOURCE_DIR)/preprocessor/directives.h $(SOURCE_DIR)/preprocessor/enum_strings.h $(SOURCE_DIR)/preprocessor/lines.h $(SOURCE_DIR)/preprocessor/linesco.h $(SOURCE_DIR)/preprocessor/pp_tok_tostring.h $(SOURCE_DIR)/preprocessor/tokenizer.h $(SOURCE_DIR)/preprocessor/tokenizerco.h \
     | \
    $(dir $(BUILD_DIR)/preprocessor/test_directives.o)
	$(CC) $(CFLAGS)  -c $< -o $@

$(BUILD_DIR)/preprocessor/test_lines.o: $(SOURCE_DIR)/preprocessor/test_lines.c \
    $(SOURCE_DIR)/misc/bookmark.h $(SOURCE_DIR)/misc/context/context.h $(SOURCE_DIR)/preprocessor/lines.h $(SOURCE_DIR)/preprocessor/linesco.h \
     | \
    $(dir $(BUILD_DIR)/preprocessor/test_lines.o)
	$(CC) $(CFLAGS)  -c $< -o $@

$(BUILD_DIR)/preprocessor/test_tokenizer.o: $(SOURCE_DIR)/preprocessor/test_tokenizer.c \
    $(SOURCE_DIR)/misc/bookmark.h $(SOURCE_DIR)/misc/charescape.h $(SOURCE_DIR)/misc/context/context.h $(SOURCE_DIR)/misc/log/log.h $(SOURCE_DIR)/misc/log/loglevel.h $(SOURCE_DIR)/misc/log/logtarget.h $(SOURCE_DIR)/preprocessor/directives.h $(SOURCE_DIR)/preprocessor/enum_strings.h $(SOURCE_DIR)/preprocessor/lines.h $(SOURCE_DIR)/preprocessor/linesco.h $(SOURCE_DIR)/preprocessor/pp_tok_tostring.h $(SOURCE_DIR)/preprocessor/tokenizer.h $(SOURCE_DIR)/preprocessor/tokenizerco.h \
     | \
    $(dir $(BUILD_DIR)/preprocessor/test_tokenizer.o)
	$(CC) $(CFLAGS)  -c $< -o $@

# --- GENERATION OF TEST_MAIN --- 

$(BUILD_DIR)/$(TEST_MAIN_NAME): $(TEST_SOURCES) $(TEST_DISCOVERY_SCRIPT) $(TEST_MAIN_SKEL) | $(dir $(BUILD_DIR)/$(TEST_MAIN_NAME))
	$(TEST_DISCOVERY_SCRIPT) $(TEST_SOURCES) --test_prefix $(TEST_PREFIX) --main_skel $(TEST_MAIN_SKEL) -o $@

# --- CATALOGS GENERATION RULES ---

$(BUILD_DIR)/misc/context/context.cat.h: $(SOURCE_DIR)/misc/context/context.cat $(CATALOG_SCRIPT) | $(dir $(BUILD_DIR)/misc/context/context.cat.h)
	$(CATALOG_SCRIPT) $< -o $@

$(BUILD_DIR)/misc/log/log.cat.h: $(SOURCE_DIR)/misc/log/log.cat $(CATALOG_SCRIPT) | $(dir $(BUILD_DIR)/misc/log/log.cat.h)
	$(CATALOG_SCRIPT) $< -o $@

$(BUILD_DIR)/preprocessor/messages.cat.h: $(SOURCE_DIR)/preprocessor/messages.cat $(CATALOG_SCRIPT) | $(dir $(BUILD_DIR)/preprocessor/messages.cat.h)
	$(CATALOG_SCRIPT) $< -o $@

# --- DIRECTORY CREATION --- 

$(BUILD_DIR)/./:
	$(MKDIR) $@

$(BUILD_DIR)/misc/:
	$(MKDIR) $@

$(BUILD_DIR)/misc/context/:
	$(MKDIR) $@

$(BUILD_DIR)/misc/log/:
	$(MKDIR) $@

$(BUILD_DIR)/preprocessor/:
	$(MKDIR) $@

$(BUILD_DIR)/:
	$(MKDIR) $@

# --- MAKEFILE REGENERATION ---

$(MAKEFILE_NAME): $(SOURCE_DIR)/$(MAIN_NAME) $(SOURCES) $(TEST_SOURCES)
	$(DISCOVER_SCRIPT) $(DISCOVER_ARGS) -mskel $(MAKEFILE_SKEL) $(MAKEFILE_NAME)
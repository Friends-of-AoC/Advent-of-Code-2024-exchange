PROJECT_ROOT = $(dir $(abspath $(lastword $(MAKEFILE_LIST))))

ifeq ($(CC),)
	CC = gcc
endif

TARGET = aoc

ifeq ($(BUILD_MODE),)
	BUILD_MODE = debug
endif
ifeq ($(BUILD_MODE),debug)
	CFLAGS += -g -Og
else ifeq ($(BUILD_MODE),run)
	CFLAGS += -Ofast -Wstrict-overflow=5
else
    $(error Build mode $(BUILD_MODE) not supported by this Makefile)
endif

CFLAGS += -Wall -Wstrict-overflow=5 -pedantic

BIN_ROOT = $(PROJECT_ROOT)bin/

BINARY = $(BIN_ROOT)$(BUILD_MODE)/

SOURCE = $(PROJECT_ROOT)src/

OBJS = $(BINARY)aoc.o $(BINARY)hash.o

ifeq ($(INTERACTIVE),)
	 INTERACTIVE = 0
endif
ifeq ($(INTERACTIVE),1)
	 OBJS += $(BINARY)interactive.o
	 CFLAGS += -DINTERACTIVE
endif

TARGET_WITH_PATH = $(BINARY)$(TARGET)

all:	$(TARGET)

$(TARGET):	$(TARGET_WITH_PATH)

$(TARGET_WITH_PATH):	$(OBJS)
	$(CC) -o $(TARGET_WITH_PATH) $^ $(LDFLAGS)
	echo FINISHED $(TARGET)

$(BINARY)%.o:	$(SOURCE)%.c
	mkdir -p `dirname $@`
	$(CC) -x c -c $(CFLAGS) -o $@ $<

clean:
	rm -fr $(BIN_ROOT)

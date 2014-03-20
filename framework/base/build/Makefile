
SOURCE_DIR  ?= $(shell cd .. && pwd)
BUILD_DIR   := $(SOURCE_DIR)/build

CXX         := g++
CC          := gcc

#MODULE=MagEvent

# Only support shared library.
#LIB_NAME := lib$(MODULE).so

LIB_INC_PATH := $(SOURCE_DIR)/inc \
                $(SOURCE_DIR)/../inc \
                $(SOURCE_DIR)/../inc/internal \
                $(SOURCE_DIR)/../agilelog/inc \
                $(SOURCE_DIR)/../agilelog/inc/pub \
                $(SOURCE_DIR)/../memory/inc

LIB_SRCS := rbtree_test.c \
            Mag_mem.c \
            Mag_rbtree.c


ifndef BINDIR
BINDIR ?= ./bin
$(shell mkdir -p $(BINDIR))
endif

ifndef OBJDIR
OBJDIR ?= ./obj
$(shell mkdir -p $(OBJDIR))
endif

CPPFLAGS += -g
CPPFLAGS += -Wno-unused-parameter -Wno-missing-field-initializers
##CPPFLAGS += -D_DEBUG_
CPPFLAGS += $(addprefix -I,$(LIB_INC_PATH))

CFLAGS += -g -DMAG_DEBUG
CFLAGS += -Wno-unused-parameter -Werror -Wno-missing-field-initializers
CFLAGS += $(addprefix -I,$(LIB_INC_PATH))

LIB_OBJS=$(patsubst %.c, $(OBJDIR)/%.o, $(LIB_SRCS))

LDFLAGS += -L../../agilelog/build/bin -lagilelog

.PHONY: all clean

all: $(BINDIR)/rbtreeTest

VPATH = $(SOURCE_DIR)/src \
        $(SOURCE_DIR)/test \
        $(SOURCE_DIR)/inc \
        $(SOURCE_DIR)/../inc \
        $(SOURCE_DIR)/../inc/internal \
        $(SOURCE_DIR)/../memory/src

$(OBJDIR)/%.o: %.c  Mag_pub_def.h
	@echo "[compiling.. $(notdir $<)]"
	@$(CC) -c -o $@ $< $(CFLAGS)
	
$(BINDIR)/rbtreeTest: $(LIB_OBJS)
	@echo "[build rbtree_test..]"
	$(CC) -o $@ $^ $(LDFLAGS)

clean:
	@echo "[clean.. $(MODULE)]"
	@-rm $(BINDIR)/*
	@-rm $(OBJDIR)/*


SOURCE_DIR  ?= $(shell cd .. && pwd)
BUILD_DIR   := $(SOURCE_DIR)/build

CXX         := g++
CC          := gcc

#MODULE=MagEvent

# Only support shared library.
#LIB_NAME := lib$(MODULE).so

LIB_INC_PATH := $(SOURCE_DIR)/inc $(SOURCE_DIR)/../inc $(SOURCE_DIR)/../base/inc $(SOURCE_DIR)/../agilelog/inc $(SOURCE_DIR)/../agilelog/inc/pub

LIB_SRCS := ooc.c \
            ooc_test.c \
            Mag_base.c


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

LDFLAGS += -lpthread -L../../agilelog/build/bin -lagilelog

.PHONY: all clean

all: $(BINDIR)/oocTest

VPATH = $(SOURCE_DIR)/src $(SOURCE_DIR)/../base/src $(SOURCE_DIR)/test $(SOURCE_DIR)/inc $(SOURCE_DIR)/../inc

$(OBJDIR)/%.o: %.c  Mag_pub_def.h
	@echo "[compiling.. $(notdir $<)]"
	@$(CC) -c -o $@ $< $(CFLAGS)
	
$(BINDIR)/oocTest: $(LIB_OBJS)
	@echo "[build oocTest..]"
	$(CC) -o $@ $^ $(LDFLAGS)

clean:
	@echo "[clean.. $(MODULE)]"
	@-rm $(BINDIR)/*
	@-rm $(OBJDIR)/*

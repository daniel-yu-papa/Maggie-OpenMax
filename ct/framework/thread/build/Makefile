
SOURCE_DIR  ?= $(shell cd .. && pwd)
BUILD_DIR   := $(SOURCE_DIR)/build

CXX         := g++
CC          := gcc

#MODULE=MagEvent

# Only support shared library.
#LIB_NAME := lib$(MODULE).so

LIB_INC_PATH := $(SOURCE_DIR)/inc $(SOURCE_DIR)/../inc $(SOURCE_DIR)/../memory/inc $(SOURCE_DIR)/../base/inc $(SOURCE_DIR)/../agilelog/inc $(SOURCE_DIR)/../agilelog/inc/pub $(SOURCE_DIR)/../event/inc

LIB_SRCS := Mag_event.c \
            Mag_base.c \
            Mag_mem.c \
            Mag_thread.c \
            Mag_threadTest.c


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

LDFLAGS += -lpthread -L/home/yujun/code/Maggie-OpenMax/hardware/marvell/ct/framework/agilelog/build/bin -lagilelog
LDFLAGS += -lrt

.PHONY: all clean

all: $(BINDIR)/threadTest 

VPATH = $(SOURCE_DIR)/src $(SOURCE_DIR)/../event/src $(SOURCE_DIR)/../base/src $(SOURCE_DIR)/../memory/src $(SOURCE_DIR)/test $(SOURCE_DIR)/inc $(SOURCE_DIR)/../inc

$(OBJDIR)/%.o: %.c  Mag_pub_def.h Mag_pub_type.h
	@echo "[compiling.. $(notdir $<)]"
	@$(CC) -c -o $@ $< $(CFLAGS)
	
$(BINDIR)/threadTest: $(LIB_OBJS)
	@echo "[build threadTest..]"
	@$(CC)  -o $@ $^ $(LDFLAGS)

clean:
	@echo "[clean.. $(MODULE)]"
	@-rm $(BINDIR)/*
	@-rm $(OBJDIR)/*


SOURCE_DIR  ?= $(shell cd .. && pwd)
BUILD_DIR   := $(SOURCE_DIR)/build

CXX         := g++
CC          := gcc

#MODULE=MagEvent

# Only support shared library.
#LIB_NAME := lib$(MODULE).so

LIB_INC_PATH := $(SOURCE_DIR)/inc \
                $(SOURCE_DIR)/../inc \
                $(SOURCE_DIR)/../base/inc \
                $(SOURCE_DIR)/../agilelog/inc \
                $(SOURCE_DIR)/../agilelog/inc/pub \
                $(SOURCE_DIR)/../event/inc \
                $(SOURCE_DIR)/../thread/inc \
                $(SOURCE_DIR)/../memory/inc \
                $(SOURCE_DIR)/../message/inc

LIB_SRCS := Mag_msg.c   \
            Mag_event.c \
            Mag_base.c  \
            Mag_looper.c \
            Mag_message.c \
            Mag_rbtree.c \
            Mag_thread.c \
            Mag_mem.c

LIB_CXX_SRCS := test_looper.cpp


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
LIB_OBJS+=$(patsubst %.cpp, $(OBJDIR)/%.o, $(LIB_CXX_SRCS))

LDFLAGS += -lpthread -L/home/yujun/code/csa_amp_1084_sdk/vendor/marvell/Maggie-OpenMax/framework/agilelog/build/bin -lagilelog
LDFLAGS += -lrt

.PHONY: all clean

all: $(BINDIR)/looperTest 

VPATH = $(SOURCE_DIR)/src \
        $(SOURCE_DIR)/../event/src \
        $(SOURCE_DIR)/../base/src \
        $(SOURCE_DIR)/../thread/src \
        $(SOURCE_DIR)/test \
        $(SOURCE_DIR)/../src \
        $(SOURCE_DIR)/../message/src \
        $(SOURCE_DIR)/../memory/src \
        $(SOURCE_DIR)/inc \
        $(SOURCE_DIR)/../inc

$(OBJDIR)/%.o: %.c  Mag_pub_def.h
	@echo "[compiling.. $(notdir $<)]"
	@$(CC) -c -o $@ $< $(CFLAGS)

$(OBJDIR)/%.o: %.cpp  Mag_pub_def.h
	@echo "[compiling.. $(notdir $<)]"
	@$(CXX) -c -o $@ $< $(CPPFLAGS)

$(BINDIR)/looperTest: $(LIB_OBJS)
	@echo "[build looperTest..]"
	$(CC) -o $@  $^ $(LDFLAGS)

clean:
	@echo "[clean.. $(MODULE)]"
	@-rm $(BINDIR)/*
	@-rm $(OBJDIR)/*

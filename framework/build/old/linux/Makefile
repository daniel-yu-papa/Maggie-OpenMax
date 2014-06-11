FRAMEWORK_SOURCE_DIR  ?= $(shell cd ../.. && pwd)
OUTPUT_DIR   := $(shell mkdir -p out && pwd)/out

CXX         := g++
CC          := gcc

MODULE := MagFramework

# Only support shared library.
LIB_NAME := lib$(MODULE).so

LIB_INC_PATH := $(FRAMEWORK_SOURCE_DIR)/inc \
                $(FRAMEWORK_SOURCE_DIR)/inc/internal \
                $(FRAMEWORK_SOURCE_DIR)/base/inc \
                $(FRAMEWORK_SOURCE_DIR)/db/inc \
                $(FRAMEWORK_SOURCE_DIR)/event/inc \
                $(FRAMEWORK_SOURCE_DIR)/hashtable/inc \
                $(FRAMEWORK_SOURCE_DIR)/looper/inc \
                $(FRAMEWORK_SOURCE_DIR)/memory/inc \
                $(FRAMEWORK_SOURCE_DIR)/message/inc \
                $(FRAMEWORK_SOURCE_DIR)/thread/inc  \
                $(FRAMEWORK_SOURCE_DIR)/ooc/inc  \
                $(FRAMEWORK_SOURCE_DIR)/streamBuffer/inc \
                $(FRAMEWORK_SOURCE_DIR)/agilelog/inc \
                $(FRAMEWORK_SOURCE_DIR)/agilelog/inc/pub

LIB_SRC_PATH := $(FRAMEWORK_SOURCE_DIR)/base/src \
                $(FRAMEWORK_SOURCE_DIR)/db/src \
                $(FRAMEWORK_SOURCE_DIR)/event/src \
                $(FRAMEWORK_SOURCE_DIR)/hashtable/src \
                $(FRAMEWORK_SOURCE_DIR)/looper/src \
                $(FRAMEWORK_SOURCE_DIR)/memory/src \
                $(FRAMEWORK_SOURCE_DIR)/message/src \
                $(FRAMEWORK_SOURCE_DIR)/thread/src  
                    
LIB_SRCS     := Mag_base.c  Mag_list.c  Mag_rbtree.c \
                Mag_minidb.c \
                Mag_event.c  \
                hashTable.c  \
                Mag_looper.c  Mag_message.c \
                Mag_mem.c  Mag_mempool.c \
                Mag_msg.c  Mag_msgQueue.c \
                Mag_thread.c

ifndef BINDIR
BINDIR ?= ./bin
$(shell mkdir -p $(BINDIR))
endif

ifndef OBJDIR
OBJDIR ?= ./obj
$(shell mkdir -p $(OBJDIR))
endif

CFLAGS += -g -DMAG_DEBUG
CFLAGS += -Wno-unused-parameter -Werror -Wno-missing-field-initializers
CFLAGS += $(addprefix -I,$(LIB_INC_PATH))
CFLAGS += -fPIC

LIB_OBJS=$(patsubst %.c, $(OBJDIR)/%.o, $(LIB_SRCS))

LDFLAGS += -lpthread -L../../agilelog/build/bin -lagilelog
LDFLAGS += -lrt

.PHONY: all clean install

all: $(BINDIR)/$(LIB_NAME)

VPATH = $(LIB_INC_PATH) \
        $(LIB_SRC_PATH)
        
$(OBJDIR)/%.o: %.c  Mag_pub_common.h  Mag_pub_def.h  Mag_pub_type.h
	@echo "[compiling.. $(notdir $<)]"
	@$(CC) -c -o $@ $< $(CFLAGS)

$(BINDIR)/$(LIB_NAME): $(LIB_OBJS)
	@echo [creating.. $(notdir $@)]
	$(CC) -shared $^ -o $@

clean:
	@echo "[clean.. $(MODULE)]"
	@-rm $(BINDIR)/*
	@-rm $(OBJDIR)/*
	@-rm -r $(OUTPUT_DIR)
    
install:
	@-mkdir $(OUTPUT_DIR)/include
	@-mkdir $(OUTPUT_DIR)/include/framework
	@-cp $(FRAMEWORK_SOURCE_DIR)/inc/LMPFramework.h $(OUTPUT_DIR)/include
	@-cp $(FRAMEWORK_SOURCE_DIR)/inc/internal/*.h $(OUTPUT_DIR)/include/framework
	@-cp $(FRAMEWORK_SOURCE_DIR)/agilelog/inc/pub/*.h $(OUTPUT_DIR)/include/framework
	@-cp $(FRAMEWORK_SOURCE_DIR)/agilelog/inc/agilelog_def.h $(OUTPUT_DIR)/include/framework
	@-cp $(FRAMEWORK_SOURCE_DIR)/base/inc/*.h $(OUTPUT_DIR)/include/framework
	@-cp $(FRAMEWORK_SOURCE_DIR)/db/inc/*.h $(OUTPUT_DIR)/include/framework
	@-cp $(FRAMEWORK_SOURCE_DIR)/event/inc/*.h $(OUTPUT_DIR)/include/framework 
	@-cp $(FRAMEWORK_SOURCE_DIR)/hashtable/inc/*.h $(OUTPUT_DIR)/include/framework
	@-cp $(FRAMEWORK_SOURCE_DIR)/looper/inc/*.h $(OUTPUT_DIR)/include/framework
	@-cp $(FRAMEWORK_SOURCE_DIR)/memory/inc/*.h $(OUTPUT_DIR)/include/framework
	@-cp $(FRAMEWORK_SOURCE_DIR)/message/inc/*.h $(OUTPUT_DIR)/include/framework
	@-cp $(FRAMEWORK_SOURCE_DIR)/thread/inc/*.h $(OUTPUT_DIR)/include/framework
    

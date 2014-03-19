LOCAL_PATH:= $(call my-dir)
include $(CLEAR_VARS)

FRAMEWORK_TOP_PATH = $(LOCAL_PATH)/../..

BASE_MODULE_FILES      := $(FRAMEWORK_TOP_PATH)/base/src/Mag_base.c \
                          $(FRAMEWORK_TOP_PATH)/base/src/Mag_list.c \
                          $(FRAMEWORK_TOP_PATH)/base/src/Mag_rbtree.c
BASE_MODULE_INC        := $(FRAMEWORK_TOP_PATH)/base/inc

DB_MODULE_FILES        := $(FRAMEWORK_TOP_PATH)/db/src/Mag_minidb.c
DB_MODULE_INC          := $(FRAMEWORK_TOP_PATH)/db/inc

EVENT_MODULE_FILES     := $(FRAMEWORK_TOP_PATH)/event/src/Mag_event.c
EVENT_MODULE_INC       := $(FRAMEWORK_TOP_PATH)/event/inc

#HASHTABLE_MODULE_FILES := $(FRAMEWORK_TOP_PATH)/hashtable/hashTable.cpp
#HASHTABLE_MODULE_INC   := $(FRAMEWORK_TOP_PATH)/hashtable

LOOPER_MODULE_FILES    := $(FRAMEWORK_TOP_PATH)/looper/src/Mag_looper.c \
                          $(FRAMEWORK_TOP_PATH)/looper/src/Mag_message.c
LOOPER_MODULE_INC      := $(FRAMEWORK_TOP_PATH)/looper/inc

MEMORY_MODULE_FILES    := $(FRAMEWORK_TOP_PATH)/memory/src/Mag_mem.c \
                          $(FRAMEWORK_TOP_PATH)/memory/src/Mag_mempool.c
MEMORY_MODULE_INC      := $(FRAMEWORK_TOP_PATH)/memory/inc
                       
OOC_MODULE_FILES       := $(FRAMEWORK_TOP_PATH)/ooc/src/ooc.c
OOC_MODULE_INC         := $(FRAMEWORK_TOP_PATH)/ooc/inc

THREAD_MODULE_FILES    := $(FRAMEWORK_TOP_PATH)/thread/src/Mag_thread.c
THREAD_MODULE_INC      := $(FRAMEWORK_TOP_PATH)/thread/inc

LOCAL_MODULE_TAGS := optional

LOCAL_MODULE := libMagFramework



LOCAL_SRC_FILES := $(BASE_MODULE_FILES)        \
                   $(DB_MODULE_FILES)          \
                   $(EVENT_MODULE_FILES)       \
                   $(LOOPER_MODULE_FILES)      \
                   $(MEMORY_MODULE_FILES)      \
                   $(OOC_MODULE_FILES)         \
                   $(THREAD_MODULE_FILES)
                   

LOCAL_C_INCLUDES += $(BASE_MODULE_INC)      \
                    $(DB_MODULE_INC)        \
                    $(EVENT_MODULE_INC)     \
                    $(HASHTABLE_MODULE_INC) \
                    $(LOOPER_MODULE_INC)    \
                    $(MEMORY_MODULE_INC)    \
                    $(OOC_MODULE_INC)       \
                    $(THREAD_MODULE_INC)

LOCAL_CFLAGS += -fpermissive -DANDROID -DANDROID_NDK

LOCAL_LDFLAGS := -lc -llog

LOCAL_MODULE_PATH:= $(LOCAL_PATH)/lib

include $(BUILD_SHARED_LIBRARY)
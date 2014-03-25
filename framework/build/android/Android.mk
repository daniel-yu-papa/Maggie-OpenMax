LOCAL_PATH := $(call my-dir)
include $(CLEAR_VARS)

LOCAL_MODULE_TAGS := optional

LOCAL_MODULE := libMagFramework

FRAMEWORK_TOP_FULL_PATH := $(LOCAL_PATH)/../..
FRAMEWORK_TOP_PATH      := ../..

BASE_MODULE_FILES      := $(FRAMEWORK_TOP_PATH)/base/src/Mag_base.c \
                          $(FRAMEWORK_TOP_PATH)/base/src/Mag_list.c \
                          $(FRAMEWORK_TOP_PATH)/base/src/Mag_rbtree.c
BASE_MODULE_INC        := $(FRAMEWORK_TOP_FULL_PATH)/base/inc

DB_MODULE_FILES        := $(FRAMEWORK_TOP_PATH)/db/src/Mag_minidb.c
DB_MODULE_INC          := $(FRAMEWORK_TOP_FULL_PATH)/db/inc

EVENT_MODULE_FILES     := $(FRAMEWORK_TOP_PATH)/event/src/Mag_event.c
EVENT_MODULE_INC       := $(FRAMEWORK_TOP_FULL_PATH)/event/inc

HASHTABLE_MODULE_FILES := $(FRAMEWORK_TOP_PATH)/hashtable/src/hashTable.c
HASHTABLE_MODULE_INC   := $(FRAMEWORK_TOP_FULL_PATH)/hashtable/inc

LOOPER_MODULE_FILES    := $(FRAMEWORK_TOP_PATH)/looper/src/Mag_looper.c \
                          $(FRAMEWORK_TOP_PATH)/looper/src/Mag_message.c
LOOPER_MODULE_INC      := $(FRAMEWORK_TOP_FULL_PATH)/looper/inc

MEMORY_MODULE_FILES    := $(FRAMEWORK_TOP_PATH)/memory/src/Mag_mem.c \
                          $(FRAMEWORK_TOP_PATH)/memory/src/Mag_mempool.c
MEMORY_MODULE_INC      := $(FRAMEWORK_TOP_FULL_PATH)/memory/inc
                       
OOC_MODULE_FILES       := $(FRAMEWORK_TOP_PATH)/ooc/src/ooc.c 
OOC_MODULE_INC         := $(FRAMEWORK_TOP_FULL_PATH)/ooc/inc

THREAD_MODULE_FILES    := $(FRAMEWORK_TOP_PATH)/thread/src/Mag_thread.c
THREAD_MODULE_INC      := $(FRAMEWORK_TOP_FULL_PATH)/thread/inc

MESSAGE_MODULE_FILES   := $(FRAMEWORK_TOP_PATH)/message/src/Mag_msgQueue.c \
                          $(FRAMEWORK_TOP_PATH)/message/src/Mag_msg.c
MESSAGE_MODULE_INC     := $(FRAMEWORK_TOP_FULL_PATH)/message/inc

COMMON_INC             := $(FRAMEWORK_TOP_FULL_PATH)/inc/internal
AGILELOG_INC           := $(FRAMEWORK_TOP_FULL_PATH)/agilelog/inc/pub \
                          $(FRAMEWORK_TOP_FULL_PATH)/agilelog/inc

LOCAL_SRC_FILES := $(BASE_MODULE_FILES)        \
                   $(DB_MODULE_FILES)          \
                   $(EVENT_MODULE_FILES)       \
                   $(LOOPER_MODULE_FILES)      \
                   $(MEMORY_MODULE_FILES)      \
                   $(OOC_MODULE_FILES)         \
                   $(THREAD_MODULE_FILES)      \
                   $(MESSAGE_MODULE_FILES)
                   

LOCAL_C_INCLUDES := $(COMMON_INC)           \
                    $(AGILELOG_INC)         \
                    $(BASE_MODULE_INC)      \
                    $(DB_MODULE_INC)        \
                    $(EVENT_MODULE_INC)     \
                    $(HASHTABLE_MODULE_INC) \
                    $(LOOPER_MODULE_INC)    \
                    $(MEMORY_MODULE_INC)    \
                    $(OOC_MODULE_INC)       \
                    $(THREAD_MODULE_INC)    \
                    $(MESSAGE_MODULE_INC)

LOCAL_CFLAGS += -DANDROID -DANDROID_NDK

#LOCAL_LDFLAGS := 

LOCAL_SHARED_LIBRARIES := \
			libagilelog

LOCAL_MODULE_PATH := $(TARGET_OUT_SHARED_LIBRARIES)/magplayer

include $(BUILD_SHARED_LIBRARY)

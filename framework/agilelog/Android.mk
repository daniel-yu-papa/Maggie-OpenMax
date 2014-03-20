LOCAL_PATH:= $(call my-dir)
include $(CLEAR_VARS)

LOCAL_MODULE_TAGS := optional

LOCAL_MODULE := libagilelog

LOCAL_SRC_FILES := src/agilelog.cpp \
                   src/agilelog_inf.cpp \
                   ../hashtable/src/hashTable.c \
                   external/tinyxml2/tinyxml2.cpp

LOCAL_C_INCLUDES += \
                    $(LOCAL_PATH)/inc \
                    $(LOCAL_PATH)/inc/pub \
                    $(LOCAL_PATH)/../hashtable/inc  \
                    $(LOCAL_PATH)/external/tinyxml2 

LOCAL_CFLAGS += -DANDROID -DANDROID_NDK

LOCAL_LDFLAGS := -lc -llog

LOCAL_MODULE_PATH:= $(LOCAL_PATH)/lib

include $(BUILD_SHARED_LIBRARY)




# Copyright 2006 The Android Open Source Project

LOCAL_PATH:= $(call my-dir)/
include $(CLEAR_VARS)

LOCAL_SRC_FILES:= agilelog_test.cpp

LOCAL_MODULE_TAGS := optional

LOCAL_CFLAGS += 

LOCAL_C_INCLUDES += $(LOCAL_PATH)/../../inc/pub \
                    $(LOCAL_PATH)/../../inc                   

LOCAL_SHARED_LIBRARIES := libagilelog

LOCAL_MODULE := agiletest

include $(BUILD_EXECUTABLE)

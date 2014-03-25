LOCAL_PATH:= $(call my-dir)
include $(CLEAR_VARS)

include vendor/marvell/Maggie-OpenMax/framework/build/android/AndroidFramework.mk

MAGPLAYER_PATH = $(LOCAL_PATH)/../..

LOCAL_SRC_FILES:= \
    tsPlayer.cpp

LOCAL_SHARED_LIBRARIES := \
    libagilelog libMagFramework libMagPlayerClient\
    libcutils libutils libbinder libgui

LOCAL_MODULE_PATH := $(TARGET_OUT_SHARED_LIBRARIES)/magplayer
   
LOCAL_MODULE:= libMagTsPlayer

LOCAL_MODULE_TAGS := optional

LOCAL_C_INCLUDES := \
    $(MAGPLAYER_FRAMEWORK_INC_PATH) \
    $(LOCAL_PATH) \
    $(MAGPLAYER_PATH)/platform/android \
    $(MAGPLAYER_PATH)/players/inc \
    $(MAGPLAYER_PATH)/client/android/inc \
    $(MAGPLAYER_PATH)/server/android/libMagPlayerService/inc \
    $(MAGPLAYER_PATH)/../omx_il/core/inc

include $(BUILD_SHARED_LIBRARY)

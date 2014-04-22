LOCAL_PATH:= $(call my-dir)
include $(CLEAR_VARS)

include vendor/marvell/Maggie-OpenMax/framework/build/android/AndroidFramework.mk

MAGPLAYER_PATH = $(LOCAL_PATH)/../..

LOCAL_SRC_FILES:= \
    src/MagPlayerClient.cpp \
    src/IMagPlayerDeathNotifier.cpp \
    src/IMagPlayerClient.cpp \
    ../../server/android/libMagPlayerService/src/IMagPlayerNotifier.cpp \
    ../../server/android/libMagPlayerService/src/IMagPlayerService.cpp \
    ../../../framework/streamBuffer/src/client/android/streamBuffer.cpp \
    ../../../framework/streamBuffer/src/client/android/streamBufferImpl.cpp \
    ../../../framework/streamBuffer/src/server/android/streamBufferUser.cpp

LOCAL_SHARED_LIBRARIES := \
    libagilelog libMagFramework \
    libcutils libutils libbinder libgui

LOCAL_MODULE_PATH := $(TARGET_OUT_SHARED_LIBRARIES)/magplayer
   
LOCAL_MODULE:= libMagPlayerClient

LOCAL_MODULE_TAGS := optional

LOCAL_C_INCLUDES := \
    $(MAGPLAYER_FRAMEWORK_INC_PATH) \
    $(LOCAL_PATH)/inc \
    $(MAGPLAYER_PATH)/includes \
    $(MAGPLAYER_PATH)/platform/android \
    $(MAGPLAYER_PATH)/server/android/libMagPlayerService/inc

include $(BUILD_SHARED_LIBRARY)

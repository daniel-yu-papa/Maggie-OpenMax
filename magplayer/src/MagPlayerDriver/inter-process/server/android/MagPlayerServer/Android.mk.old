LOCAL_PATH:= $(call my-dir)
include $(CLEAR_VARS)

include vendor/marvell/Maggie-OpenMax/framework/build/android/AndroidFramework.mk

MAGPLAYER_PATH = $(LOCAL_PATH)/../../..

LOCAL_SRC_FILES:= \
    magplayer_server.cpp

LOCAL_SHARED_LIBRARIES := \
    libbinder \
    libdl \
    liblog \
    libMagPlayerService \
    libagilelog \
    libutils

LOCAL_C_INCLUDES := \
    $(MAGPLAYER_FRAMEWORK_INC_PATH) \
    $(MAGPLAYER_PATH)/client/android/inc \
    $(MAGPLAYER_PATH)/server/android/libMagPlayerService/inc \
    $(MAGPLAYER_PATH)/players/MagPlayerDriver/android \
    $(MAGPLAYER_PATH)/players/MagPlayer \
    $(MAGPLAYER_PATH)/players/inc \
    $(MAGPLAYER_PATH)/components/includes \
    $(MAGPLAYER_PATH)/components/demuxer/ffmpeg \
    $(MAGPLAYER_PATH)/../omx_il/core/inc \
    vendor/marvell/a3ce/external/ffmpeg \
    $(MAGPLAYER_PATH)/platform/android \
    $(MAGPLAYER_PATH)/includes

LOCAL_MODULE_PATH := $(TARGET_OUT_SHARED_LIBRARIES)/magplayer

LOCAL_MODULE := MagPlayerServer
LOCAL_MODULE_TAGS := optional

include $(BUILD_EXECUTABLE)

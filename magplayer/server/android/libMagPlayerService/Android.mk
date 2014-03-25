LOCAL_PATH:= $(call my-dir)
include $(CLEAR_VARS)

include vendor/marvell/Maggie-OpenMax/framework/build/android/AndroidFramework.mk

MAGPLAYER_PATH = $(LOCAL_PATH)/../../..

LOCAL_SRC_FILES:= \
    src/IMagPlayerService.cpp  \
    src/MagPlayerService.cpp   \
    src/IMagPlayerNotifier.cpp \
    ../../../client/android/src/IMagPlayerClient.cpp \
    ../../../../framework/streamBuffer/src/client/android/streamBuffer.cpp \
    ../../../../framework/streamBuffer/src/server/android/streamBufferUser.cpp \
    ../../../../framework/streamBuffer/src/server/android/streamBufferUserImpl.cpp \
    ../../../players/MagPlayerDriver/android/MagPlayerDriver.cpp

LOCAL_SHARED_LIBRARIES := \
    libagilelog libMagFramework libMagPlayer\
    libcutils libutils libbinder libgui
 
LOCAL_MODULE_PATH := $(TARGET_OUT_SHARED_LIBRARIES)/magplayer
   
LOCAL_MODULE:= libMagPlayerService

LOCAL_MODULE_TAGS := optional

LOCAL_C_INCLUDES := \
    $(MAGPLAYER_FRAMEWORK_INC_PATH) \
    $(LOCAL_PATH)/inc \
    $(MAGPLAYER_PATH)/includes \
    $(MAGPLAYER_PATH)/platform/android \
    $(MAGPLAYER_PATH)/client/android/inc \
    $(MAGPLAYER_PATH)/server/android/libMagPlayerService/inc \
    $(MAGPLAYER_PATH)/players/MagPlayerDriver/android \
    $(MAGPLAYER_PATH)/players/MagPlayer \
    $(MAGPLAYER_PATH)/players/inc \
    $(MAGPLAYER_PATH)/components/includes \
    $(MAGPLAYER_PATH)/components/demuxer/ffmpeg \
    $(MAGPLAYER_PATH)/players/MagPlayerDriver/android \
    vendor/marvell/a3ce/external/ffmpeg

include $(BUILD_SHARED_LIBRARY)

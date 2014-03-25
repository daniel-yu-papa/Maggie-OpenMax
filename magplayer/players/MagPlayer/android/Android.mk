LOCAL_PATH:= $(call my-dir)
include $(CLEAR_VARS)

include vendor/marvell/Maggie-OpenMax/framework/build/android/AndroidFramework.mk

MAGPLAYER_PATH = $(LOCAL_PATH)/../../..

LOCAL_SRC_FILES:= \
    ../MagPlayer.cpp  \
    ../../../components/content_pipe/MagPlayer_ContentPipe.cpp \
    ../../../components/data_source/MagPlayer_DataSource.cpp \
    ../../../components/demuxer/base/MagPlayer_Demuxer_Base.cpp \
    ../../../components/demuxer/ffmpeg/MagPlayer_Demuxer_FFMPEG.cpp \
    ../../../components/mock_omx_il/MagPlayer_Mock_OMXIL.cpp \
    ../../../../framework/streamBuffer/src/client/android/streamBuffer.cpp \
    ../../../../framework/streamBuffer/src/server/android/streamBufferUser.cpp \
    ../../../../framework/streamBuffer/src/server/android/streamBufferUserImpl.cpp

LOCAL_SHARED_LIBRARIES := \
    libagilelog libMagFramework \
    libcutils libutils libbinder libgui \
    libffmpeg_avutil libffmpeg_avcodec libffmpeg_avformat

LOCAL_MODULE_PATH := $(TARGET_OUT_SHARED_LIBRARIES)/magplayer
   
LOCAL_MODULE:= libMagPlayer

LOCAL_MODULE_TAGS := optional

LOCAL_C_INCLUDES := \
    $(MAGPLAYER_FRAMEWORK_INC_PATH) \
    $(LOCAL_PATH)/../ \
    $(MAGPLAYER_PATH)/includes \
    $(MAGPLAYER_PATH)/players/inc \
    $(MAGPLAYER_PATH)/platform/android \
    $(MAGPLAYER_PATH)/components/includes \
    $(MAGPLAYER_PATH)/components/demuxer/ffmpeg \
    $(MAGPLAYER_PATH)/../omx_il/core/inc \
    vendor/marvell/a3ce/external/ffmpeg
    

include $(BUILD_SHARED_LIBRARY)

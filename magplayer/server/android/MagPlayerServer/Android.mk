LOCAL_PATH:= $(call my-dir)
include $(CLEAR_VARS)

LOCAL_SRC_FILES:= \
    magplayer_server.cpp

LOCAL_SHARED_LIBRARIES := \
    libbinder \
    libdl \
    liblog \
    libmediaplayerservice \
    libutils

LOCAL_C_INCLUDES := \
    frameworks/av/media/libmediaplayerservice \

LOCAL_MODULE := magplayerserver
LOCAL_MODULE_TAGS := optional

include $(BUILD_EXECUTABLE)
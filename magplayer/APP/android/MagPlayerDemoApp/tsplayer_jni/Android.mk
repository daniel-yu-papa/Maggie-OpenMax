LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)
LOCAL_MODULE_TAGS := optional

include vendor/marvell/Maggie-OpenMax/framework/build/android/AndroidFramework.mk

LOCAL_C_INCLUDES := \
    $(JNI_H_INCLUDE) \
    $(MAGPLAYER_FRAMEWORK_INC_PATH) \
    $(LOCAL_PATH)/include
    
LOCAL_SHARED_LIBRARIES := \
    libandroid_runtime \
    libnativehelper \
    libcutils \
    libutils \
    libbinder \
    libmedia\
    libgui \
    libMagTsPlayer


LOCAL_MODULE    := libTsPlayerJni

LOCAL_SRC_FILES := src/tsplayer_jni.cpp
LOCAL_MODULE_PATH:= $(LOCAL_PATH)/../java/libs/armeabi

include $(BUILD_SHARED_LIBRARY)

LOCAL_PATH:= $(call my-dir)/../..
include $(CLEAR_VARS)

COMMON_LOCAL_SRC_FILES := \
        ../event/src/Mag_event.c \
        ../thread/src/Mag_thread.c \
        ../base/src/Mag_list.c \
        ../base/src/Mag_base.c \
        ../memory/src/Mag_mem.c \
        src/client/android/streamBuffer.cpp \
        src/client/android/streamBufferImpl.cpp \
        src/server/android/streamBufferUser.cpp \
        src/server/android/streamBufferUserImpl.cpp

LOCAL_SRC_FILES:= \
	$(COMMON_LOCAL_SRC_FILES) \
        test/IStreamBufTest.cpp \
        test/streamBuf_service_test.cpp

LOCAL_SHARED_LIBRARIES := \
        libutils \
        libagilelog \
	libbinder

LOCAL_C_INCLUDES := \
    $(LOCAL_PATH)/inc \
    $(LOCAL_PATH)/../inc/internal \
    $(LOCAL_PATH)/test \
    $(LOCAL_PATH)/inc/android \
    $(LOCAL_PATH)/../event/inc \
    $(LOCAL_PATH)/../thread/inc \
    $(LOCAL_PATH)/../base/inc \
    $(LOCAL_PATH)/../memory/inc \
    $(LOCAL_PATH)/../hashtable/inc \
    $(LOCAL_PATH)/../inc \
    $(LOCAL_PATH)/../agilelog/inc \
    $(LOCAL_PATH)/../agilelog/inc/pub

LOCAL_MODULE:= streamBufService

LOCAL_MODULE_PATH:= $(LOCAL_PATH)/build/service/bin/
include $(BUILD_EXECUTABLE)

#############################################################
#  build client test
#############################################################


#LOCAL_SRC_FILES:= \
#        $(COMMON_LOCAL_SRC_FILES) \
#        ../test/streamBuf_client_test.cpp

#LOCAL_MODULE:= streamBufClient

#include $(BUILD_EXECUTABLE)

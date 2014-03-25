LOCAL_HOST := $(call my-dir)
include $(CLEAR_VARS)

BASE_PATH := $(LOCAL_HOST)/../..

##libagilelog.so
include $(BASE_PATH)/framework/agilelog/Android.mk

##libMagFramework.so
include $(BASE_PATH)/framework/build/android/Android.mk

##libMagPlayer.so
include $(BASE_PATH)/magplayer/players/MagPlayer/android/Android.mk

##libMagPlayerClient.so
include $(BASE_PATH)/magplayer/client/android/Android.mk

##libMagPlayerService.so
include $(BASE_PATH)/magplayer/server/android/libMagPlayerService/Android.mk

##libMagTsPlayer.so
include $(BASE_PATH)/magplayer/customers/TsPlayer/Android.mk

##MagPlayerServer
include $(BASE_PATH)/magplayer/server/android/MagPlayerServer/Android.mk
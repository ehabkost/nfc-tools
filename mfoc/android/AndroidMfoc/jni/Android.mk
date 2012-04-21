  LOCAL_PATH := $(call my-dir)

   include $(CLEAR_VARS)

   LOCAL_MODULE    := mfoc
   LOCAL_SRC_FILES := mfoc.c

   include $(BUILD_SHARED_LIBRARY)
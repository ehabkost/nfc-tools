LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_CPP_FLAGS = -DDEBUG
LOCAL_C_INCLUDES := include
LOCAL_MODULE    := mfoc
LOCAL_SRC_FILES := fake-libnfc.c fake-stdio.c mfoc.c \
	../../../src/mfoc.c ../../../src/crapto1.c ../../../src/crypto1.c ../../../src/mifare.c ../../../src/nfc-utils.c

include $(BUILD_SHARED_LIBRARY)
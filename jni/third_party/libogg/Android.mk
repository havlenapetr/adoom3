LOCAL_PATH := $(call my-dir)
include $(CLEAR_VARS)

LOCAL_MODULE := libogg

LOCAL_C_INCLUDES := \
    $(OGG_INCLUDE_DIR)

LOCAL_CFLAGS += \
    -ffast-math -fsigned-char \
    -march=armv6 -marm -mfloat-abi=softfp -mfpu=vfp

LOCAL_SRC_FILES := \
    bitwise.c \
    framing.c

include $(BUILD_STATIC_LIBRARY)

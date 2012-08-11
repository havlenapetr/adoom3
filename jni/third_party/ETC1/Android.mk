LOCAL_PATH:= $(call my-dir)

###############################################################################
# Build the ETC1 static library
#

include $(CLEAR_VARS)

ETC1_PRESENTED := true
ETC1_INCLUDE_DIR := $(LOCAL_PATH)/ETC1
LOCAL_SRC_FILES := \
    etc1.cpp

LOCAL_MODULE:= libETC1

include $(BUILD_STATIC_LIBRARY)
LOCAL_PATH := $(call my-dir)
include $(CLEAR_VARS)

LOCAL_C_INCLUDES := $(JPEG_INCLUDE_DIR)

LOCAL_MODULE := libjpeg

ifeq ($(APP_USE_PREBUILT_JPEG),true)
LOCAL_SRC_FILES := libjpeg.so
include $(PREBUILT_SHARED_LIBRARY)

include $(CLEAR_VARS)
LOCAL_C_INCLUDES := $(JPEG_INCLUDE_DIR)
LOCAL_MODULE := libjpeg_addon
LOCAL_SRC_FILES += jpeg_mem_src.cpp
include $(BUILD_STATIC_LIBRARY)
else
LOCAL_SRC_FILES := \
    jcapimin.c \
    jccoefct.c \
    jccolor.c \
    jcdctmgr.c \
    jchuff.c \
    jcinit.c \
    jcmainct.c \
    jcmarker.c \
    jcmaster.c \
    jcomapi.c \
    jcparam.c \
    jcphuff.c \
    jcprepct.c \
    jcsample.c \
    jdapimin.c \
    jdapistd.c \
    jdatadst.c \
    jdatasrc.c \
    jdcoefct.c \
    jdcolor.c \
    jddctmgr.c \
    jdhuff.c \
    jdinput.c \
    jdmainct.c \
    jdmarker.c \
    jdmaster.c \
    jdmerge.c \
    jdphuff.c \
    jdpostct.c \
    jdsample.c \
    jdtrans.c \
    jerror.c \
    jfdctflt.c \
    jfdctfst.c \
    jfdctint.c \
    jidctflt.c \
    jidctfst.c \
    jidctint.c \
    jidctred.c \
    jmemmgr.c \
    jmemnobs.c \
    jquant1.c \
    jquant2.c \
    jutils.c

LOCAL_SRC_FILES += jpeg_mem_src.cpp

include $(BUILD_STATIC_LIBRARY)
endif

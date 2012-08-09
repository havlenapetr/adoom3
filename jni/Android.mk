LOCAL_PATH:= $(call my-dir)
include $(CLEAR_VARS)

# Save base path
BASE_PATH := $(LOCAL_PATH)

# Our include paths
JPEG_INCLUDE_DIR := $(BASE_PATH)/third_party/jpeg-6b
OGG_INCLUDE_DIR := $(BASE_PATH)/third_party/libogg/include
OPENAL_INCLUDE_DIR := $(BASE_PATH)/third_party/openal/include
VORBIS_INCLUDE_DIR := $(BASE_PATH)/third_party/vorbis/include

LOCAL_CFLAGS := -ffast-math -O3 -funroll-loops -Wno-psabi \
    -D__DOOM_DLL__

ifeq ($(APP_NEON),true)
LOCAL_CFLAGS += -mfpu=neon -mfloat-abi=softfp
endif

ifeq ($(APP_DEMO),true)
LOCAL_CFLAGS += -DID_DEMO_BUILD
endif

LOCAL_C_INCLUDES := \
    $(OPENAL_INCLUDE_DIR) \
    $(LOCAL_PATH)/../idtech4/neo

LOCAL_SRC_FILES := \
    audio.cpp \
    glimp.cpp \
    sys.cpp \
    main.cpp

LOCAL_ARM_MODE := arm
LOCAL_MODULE := libdoom

# Framework library
LOCAL_STATIC_LIBRARIES := \
    libframework \
    libcompilers_dmap \
    libcompilers_aas

# Dependencies for framework library in not demo build
ifeq ($(APP_DEMO),false)
LOCAL_STATIC_LIBRARIES += \
    libcompilers_renderbump \
    libcompilers_roqvq
endif

# Other doom stuff
LOCAL_STATIC_LIBRARIES += \
    librenderer \
    libcm \
    libui \
    libsound \
    libid \
    libsys \
    libvorbis \
    libogg

ifeq ($(APP_USE_PREBUILT_JPEG),true)
LOCAL_STATIC_LIBRARIES += \
    libjpeg_addon
LOCAL_SHARED_LIBRARIES := \
    libjpeg
else
LOCAL_STATIC_LIBRARIES += \
    libjpeg
endif

#ifeq ($(APP_ENABLE_PROFILING),true)
#LOCAL_CFLAGS += -pg -DPROFILING_ENABLED
#LOCAL_STATIC_LIBRARIES += andprof
#endif

# Android native glue
LOCAL_STATIC_LIBRARIES += \
    android_native_app_glue

LOCAL_LDLIBS := -landroid -lEGL -lGLESv2 -lOpenSLES -llog -ldl

include $(BUILD_SHARED_LIBRARY)

# Call our submodules
$(call import-module,android/native_app_glue)
include $(call all-makefiles-under,$(BASE_PATH))
include $(call all-makefiles-under,$(BASE_PATH)/../idtech4)

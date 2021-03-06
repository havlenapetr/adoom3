# Copyright (C) 2012 Havlena Petr <havlenapetr@gmail.com>
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#      http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
#

LOCAL_PATH := $(call my-dir)
include $(CLEAR_VARS)

LOCAL_MODULE := libvorbis

VORBIS_INCLUDE_DIR := $(LOCAL_PATH)/include
LOCAL_C_INCLUDES := \
    $(LOCAL_PATH)/../libogg/include \
    $(LOCAL_PATH)/include \
    $(LOCAL_PATH)/lib

LOCAL_CFLAGS := $(GLOBAL_CFLAGS)

LOCAL_SRC_FILES := \
    lib/encode_file.c \
    lib/analysis.c \
    lib/registry.c \
    lib/vorbisenc.c \
    lib/bitwise.c \
    lib/framing.c \
    lib/bitrate.c  \
    lib/block.c  \
    lib/codebook.c  \
    lib/envelope.c  \
    lib/floor0.c \
    lib/floor1.c  \
    lib/info.c  \
    lib/lookup.c \
    lib/lpc.c \
    lib/lsp.c \
    lib/mapping0.c \
    lib/mdct.c \
    lib/psy.c \
    lib/res0.c \
    lib/sharedbook.c \
    lib/smallft.c \
    lib/synthesis.c \
    lib/vorbisfile.c \
    lib/window.c 

include $(BUILD_STATIC_LIBRARY)

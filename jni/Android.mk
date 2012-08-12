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

BASE_PATH := $(LOCAL_PATH)

APP_DEMO        := false
APP_ABI         := armeabi-v7a
APP_NEON        := true
APP_USE_PREBUILT_JPEG := true
APP_PLATFORM    := android-9

# build our third party libraries
include $(call all-makefiles-under,$(BASE_PATH))

# build doom3 engine and game
include $(BASE_PATH)/doom3.mk
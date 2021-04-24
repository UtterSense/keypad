#
# Copyright (C) 2008 The Android Open Source Project
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


#---------------------------------------------
#Build the Keypad shared library file here
#--------------------------------------------- 
LOCAL_PATH := $(call my-dir)
include $(CLEAR_VARS)

LOCAL_MODULE := libkeypad
#Module requires this library for Android logging utilities
LOCAL_SHARED_LIBRARIES:= liblog
LOCAL_SRC_FILES := keypad.c
LOCAL_C_INCLUDES := $(LOCAL_PATH)

include $(BUILD_SHARED_LIBRARY)


#---------------------------------------------
#Build the Keypad JNI shared library file here
#--------------------------------------------- 
#LOCAL_PATH := $(call my-dir)
include $(CLEAR_VARS)

LOCAL_MODULE := libkeypad-lib
#Module requires this library for Android logging utilities
LOCAL_SHARED_LIBRARIES:= liblog libkeypad

LOCAL_SRC_FILES := keypad-lib.c
LOCAL_C_INCLUDES := $(LOCAL_PATH)

include $(BUILD_SHARED_LIBRARY)

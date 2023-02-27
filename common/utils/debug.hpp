// Copyright 2022 Google Inc. All Rights Reserved.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

// Usage:
//   static const char* kTAG = "Your-TAG";
//   #include "debug.hpp"


#pragma once
#include <android/log.h>
#include <cassert>
#include <vector>

#ifndef DEBUG_TAG
    #define DEBUG_TAG "DebugTag"
#endif

// Android log function wrappers
#define LOGE(...) \
  ((void)__android_log_print(ANDROID_LOG_ERROR, DEBUG_TAG, __VA_ARGS__))
#define LOGI(...) \
  ((void)__android_log_print(ANDROID_LOG_INFO, DEBUG_TAG, __VA_ARGS__))
#define LOGW(...) \
  ((void)__android_log_print(ANDROID_LOG_WARN, DEBUG_TAG, __VA_ARGS__))

#define ASSERT(con, ...) \
  ((void)__android_log_assert((con), DEBUG_TAG, __VA_ARGS__))

#[[
Copyright 2022 Google Inc. All Rights Reserved.

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

     http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
]]
cmake_minimum_required(VERSION 3.4.1)

project(vk_wrapper)

add_library(${PROJECT_NAME} STATIC vulkan_wrapper.cpp)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Werror -std=c++11")
target_compile_options(${PROJECT_NAME} PUBLIC
  -DVK_USE_PLATFORM_ANDROID_KHR
  -DUSE_DEBUG_EXTENTIONS)
target_include_directories(${PROJECT_NAME} PUBLIC
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${ANDROID_NDK}/sources/third_party/vulkan/src/include)


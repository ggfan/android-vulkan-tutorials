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
cmake_minimum_required(VERSION 3.4.3)

project(vktuts)

# Integrate GameActivity, refer to
#     https://d.android.com/games/agdk/integrate-game-activity
# for the detailed instructions.
find_package(game-activity REQUIRED CONFIG)
find_package(games-memory-advice REQUIRED CONFIG)

# build Vulkan app
get_filename_component(REPO_ROOT_DIR
    ${CMAKE_SOURCE_DIR}/../../../../..  ABSOLUTE)
set(COMMON_DIR ${REPO_ROOT_DIR}/common)
set(THIRD_PARTY_DIR ${REPO_ROOT_DIR}/third_party)

add_library(${CMAKE_PROJECT_NAME} SHARED
    CreateShaderModule.cpp
    VulkanMain.cpp
    ${COMMON_DIR}/vulkan_wrapper/vulkan_wrapper.cpp
    AndroidMain.cpp
    ${COMMON_DIR}/src/GameActivitySources.cpp)

target_include_directories(${CMAKE_PROJECT_NAME} PRIVATE
    ${COMMON_DIR}/vulkan_wrapper
    ${COMMON_DIR}/src
    ${THIRD_PARTY_DIR}
    ${ANDROID_NDK}/sources/android/native_app_glue
    ${CMAKE_CURRENT_SOURCE_DIR}/shaderc/include)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} \
    -std=c++11 -Wall -Wno-unused-variable \
    -Wno-delete-non-virtual-dtor -DVK_USE_PLATFORM_ANDROID_KHR")

if (${ANDROID_ABI} STREQUAL "armeabi-v7a")
    set(CMAKE_CXX_FLAGS} "${CMAKE_CXX_FLAGS} \
        -mhard-float -D_NDK_MATH_NO_SOFTFP=1 -mfloat-abi=hard")
    set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} \
        -Wl,--no-warn-mismatch")
endif()

# requirement: prebuild shaderc with:
#  cd  ${CMAKE_CURRENT_SOURCE_DIR} && mkdir -p shaderc
#  ${ANDROID_NDK}/ndk-build NDK_PROJECT_PATH=. APP_BUILD_SCRIPT=${ANDROID_NDK}/sources/third_party/shaderc/Android.mk APP_STL:=all APP_ABI:=all APP_PLATFORM:=android-26 libshaderc_combined
add_library(shaderc STATIC IMPORTED)
set_target_properties(shaderc PROPERTIES IMPORTED_LOCATION
    ${CMAKE_CURRENT_SOURCE_DIR}/shaderc/libs/${ANDROID_STL}/${ANDROID_ABI}/libshaderc.a)

target_link_libraries(${CMAKE_PROJECT_NAME}
    game-activity::game-activity
    games-memory-advice::memory_advice
    log android shaderc)

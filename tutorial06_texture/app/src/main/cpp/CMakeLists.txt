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

# build Vulkan app
get_filename_component(REPO_ROOT_DIR
    ${CMAKE_SOURCE_DIR}/../../../../..  ABSOLUTE)
set(COMMON_DIR ${REPO_ROOT_DIR}/common)

# build the 2 dependent modules.
add_subdirectory(${COMMON_DIR}/vulkan_wrapper
        ${CMAKE_CURRENT_BINARY_DIR}/vulkan_wrapper)
add_subdirectory(${COMMON_DIR}/utils
        ${CMAKE_CURRENT_BINARY_DIR}/utils)

# import shaderc
# pre-requirement -- prebuild shaderc with:
#  cd  ${CMAKE_CURRENT_SOURCE_DIR} && mkdir -p shaderc && cd shaderc
#  ${ANDROID_NDK}/ndk-build NDK_PROJECT_PATH=. APP_BUILD_SCRIPT=${ANDROID_NDK}/sources/third_party/shaderc/Android.mk APP_STL:=all APP_ABI:=all APP_PLATFORM:=android-26 libshaderc_combined
# (note the known issue on windows: https://github.com/android/ndk/issues/1815)
add_library(shaderc STATIC IMPORTED)
set_target_properties(shaderc PROPERTIES IMPORTED_LOCATION
        ${CMAKE_CURRENT_SOURCE_DIR}/shaderc/libs/${ANDROID_STL}/${ANDROID_ABI}/libshaderc.a)

set(THIRD_PARTY_DIR )

add_library(${CMAKE_PROJECT_NAME} SHARED
    CreateShaderModule.cpp
    VulkanMain.cpp
    AndroidMain.cpp
    ${COMMON_DIR}/src/GameActivitySources.cpp)

target_include_directories(${CMAKE_PROJECT_NAME} PRIVATE
    ${COMMON_DIR}/src
    ${REPO_ROOT_DIR}/third_party
    ${CMAKE_CURRENT_SOURCE_DIR}/shaderc/include)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} \
    -std=c++11 -Wall -Wno-unused-variable \
    -Wno-delete-non-virtual-dtor")

if (${ANDROID_ABI} STREQUAL "armeabi-v7a")
    set(CMAKE_CXX_FLAGS} "${CMAKE_CXX_FLAGS} \
        -mhard-float -D_NDK_MATH_NO_SOFTFP=1 -mfloat-abi=hard")
    set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} \
        -Wl,--no-warn-mismatch")
endif()

target_link_libraries(${CMAKE_PROJECT_NAME}
    vk_wrapper vk_utils
    shaderc
    game-activity::game-activity
    log android)

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

project(vktuts)

# Integrate GameActivity, refer to
#     https://d.android.com/games/agdk/integrate-game-activity
# for the detailed instructions.
find_package(game-activity REQUIRED CONFIG)

# build vulkan app
get_filename_component(REPO_ROOT_DIR
    ${CMAKE_SOURCE_DIR}/../../../../..  ABSOLUTE)
set(COMMON_DIR ${REPO_ROOT_DIR}/common)

# build the 2 dependent modules.
add_subdirectory(${COMMON_DIR}/vulkan_wrapper
        ${CMAKE_CURRENT_BINARY_DIR}/vulkan_wrapper)
add_subdirectory(${COMMON_DIR}/utils
        ${CMAKE_CURRENT_BINARY_DIR}/utils)

add_library(${CMAKE_PROJECT_NAME} SHARED
    VulkanMain.cpp
    AndroidMain.cpp
    ${COMMON_DIR}/src/GameActivitySources.cpp)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -Wall -Werror")

target_link_libraries(${CMAKE_PROJECT_NAME}
    game-activity::game-activity
    vk_wrapper vk_utils
    log android)

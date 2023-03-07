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

#pragma  once

#define VK_NO_PROTOTYPES 1
#include <vulkan/vulkan.h>

class VkLogcat {
  public:
    VkLogcat();
    ~VkLogcat();
    void printGPUProperties(VkPhysicalDeviceProperties& properties);
    void printSurfaceCapabilities(VkSurfaceCapabilitiesKHR& cap);
    void printQueueFamilyProperties(VkQueueFamilyProperties* propertiesArrary, uint32_t count);
  private:

};
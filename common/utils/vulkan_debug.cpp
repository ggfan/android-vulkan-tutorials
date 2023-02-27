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
#include <sstream>
#include <iomanip>

#include "vulkan_debug.hpp"
#include "vulkan_utils.hpp"


VkLogcat::VkLogcat() {}
VkLogcat::~VkLogcat() {}
void VkLogcat::printGPUProperties(VkPhysicalDeviceProperties& properties) {
    LOGI("Vulkan GPU Properties:");
    LOGI("deviceName: %s; apiVersion: %d.%d.%d; driverVersion: %d.%d.%d\n"
         "\t\tvendorID: %x, deviceID: %x, deviceType: %x",
         properties.deviceName,
         VK_VERSION_MAJOR(properties.apiVersion),
         VK_VERSION_MINOR(properties.apiVersion),
         VK_VERSION_PATCH(properties.apiVersion),
         VK_VERSION_MAJOR(properties.driverVersion),
         VK_VERSION_MINOR(properties.driverVersion),
         VK_VERSION_PATCH(properties.driverVersion),
         properties.vendorID, properties.deviceID, properties.deviceType);

    std::stringstream uuid;
    uuid<<std::hex;
    for(auto& id: properties.pipelineCacheUUID) {
        uuid<<static_cast<uint32_t>(id);
    }
    LOGI("pipelineCacheUUID: %s", uuid.str().c_str());

#if 0
    //TODO a function to print it
    VkPhysicalDeviceLimits              limits;
    VkPhysicalDeviceSparseProperties    sparseProperties;
#endif
}

void VkLogcat::printSurfaceCapabilities(VkSurfaceCapabilitiesKHR &cap) {
    LOGI("Vulkan Surface Capabilities:");
    LOGI("imageCount: %u - %u\n"
         "\tcurrentExtent: %u, %u; minImageExtent: %u, %u; maxImageExtent: %u, %u\n"
         "\tmaxImageArrayLayers: %u\n"
         "\tsupportedTransforms: %0#x; currentTransform: %0#x\n"
         "\tsupportedCompositeAlpha: %0#x; supportedUsageFlags: %0#x\n",
      cap.minImageCount, cap.maxImageCount,
      cap.currentExtent.width, cap.currentExtent.height,
      cap.minImageExtent.width, cap.minImageExtent.height,
      cap.maxImageExtent.width, cap.maxImageExtent.height,
      cap.maxImageArrayLayers,
      cap.supportedTransforms, cap.currentTransform,
      cap.supportedCompositeAlpha, cap.supportedUsageFlags);
}

void VkLogcat::printQueueFamilyProperties(VkQueueFamilyProperties *propertiesArray,
                                          uint32_t count) {
    for (uint32_t idx = 0; idx < count; ++idx) {
     auto &  properties = propertiesArray[idx];
     LOGI("QueueFamilyProperties No. %d:\n"
          "\tqueueFlags: %0#x; queueCount: %d; timestampValidBits: %0#x\n"
          "\tminImagTransferGranularity: %d, %d, %d",
       idx, properties.queueFlags, properties.queueCount, properties.timestampValidBits,
       properties.minImageTransferGranularity.width,
       properties.minImageTransferGranularity.height,
       properties.minImageTransferGranularity.depth);
    }
}

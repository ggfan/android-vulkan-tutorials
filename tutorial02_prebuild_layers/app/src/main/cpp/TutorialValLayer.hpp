// Copyright 2016 Google Inc. All Rights Reserved.
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
#pragma once

#include "vulkan_wrapper.h"
#include <iostream>
#include <map>
#include <vector>

#define VULKAN_DRIVER "VulkanDriver"

/** A Helper class to manage validation layers and extensions
 * Supposed usage:
 *   1) At instance creation time, validation layer could be enabled by enable all discovered layers
 *        getLayerCount()
 *        getLayerNames()
 *   2) DbgExtension: once instance is created, call
 *        hookDbgReportExt
 *   2) at device creation time, enable all extensions available by:
 *        getExtensionCount()
 *        getExtensionNames()
 */
class LayerAndExtensions {
 public:
  LayerAndExtensions(void);
  ~LayerAndExtensions();

  uint32_t getLayerCount(void);
  const char* const* getLayerNames(void);
  bool isLayerSupported(const char* name);

  /*
   * getExtensionCount()
   *  handle: VK_NULL_HANDLE when asking for instance extensions
   *          valid VkPhysicalDevice handle for device extensions
   */
  uint32_t getExtensionCount(void* handle);
  /*
   * getExtensionNames(): return the extension names.
   * @param names holding returned extension names.
   * @param handle a valid VkPhysicalDevice, or VK_NULL_HANDLE if this is for layer extensions
   * @return true for extension returned, false otherwise.
   */
  bool getExtensionNames(std::vector<const char*>& names, void* handle);
  /*
   * isExtensionSupported(): get layer which implements give extension
   * @param extName an extension name in question
   * @param handle valid VKPhysicalDevice handle if asking for device extension in
   *               vulkan driver/implicit layers; VK_NULL_HANDLE if asks for instance extensions.
   * @param layerName memory to hold for the layer name that implements the requested extension.
   *        name could be used to enable the layer when app creating vulkan instance.
   *        Note: if the extension is supported in vulkan driver or implicitly loaded layers,
   *              string VULKAN_DRIVER is return in layerName.
   */
  bool isExtensionSupported(const char* extName, void* handle, const char** layerName);

  /*
   * Check for the supported debug report extension and its hosting layers, could be:
   *  - VK_EXT_DEBUG_UTILS_EXTENSION_NAME
   *  - VK_EXT_DEBUG_REPORT_EXTENSION_NAME
   *  - nullptr (not supported)
   *  With Validation layer release 1.2.176.1, the first 2 extension are implemented,
   *  so VK_EXT_DEBUG_UTILS_EXTENSION_NAME should be returned.
   * Return:
   *   std::pair<layerName, extName>
   *   if debug layer is implemented in driver(not in any layers), string VULKAN_DRIVER is
   *   returned as layerName, please make a note of it(as there is no layer named VULKAN_DRIVER).
   */
  std::pair<const char*, const char*> getDbgReportExtInfo(void);
  bool hookDbgReportExt(VkInstance instance);


  void printLayers(void);        // print layer names to logcat
  void printExtensions(void);    // print extension to logcat
  /**
   * printExtensions()  dump the extensions regarding to layers or implicitly implemented in this
   *     Vulkan implementation.
   * @param layerName the name of layer, or nullptr for this Vulkan implementation
   * @param device VkPhysicalDevice handle if device extensions are of interest too;
   *        or VK_NULL_HANDLE for instance extensions only.
   */
  void printExtensions(const char* layerName, VkPhysicalDevice device); // print extensions in the given layer

 private:
  VkInstance instance_;
  VkDebugReportCallbackEXT vkCallbackHandle_;

  // Global layers and extensions
  std::map<void*, std::vector<char*>> layerNames_;
  // device extension with form: physicalDevice:layer:extensions
  std::map<void*, std::map<std::string, std::vector<const char*>>> extensions_;

  // helper functions
  void initDevExtensions(void*);
};


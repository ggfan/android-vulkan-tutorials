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
#include "TutorialValLayer.hpp"
#define DEBUG_TAG "vkTutorial02"
#include "debug.hpp"
#include "vulkan_debug.hpp"
#include "vulkan_utils.hpp"

#include <cassert>

/**
 * Validation Layer name
 */
static const char* kValLayerName = "VK_LAYER_KHRONOS_validation";

/**
 * Helper function cloneString(): allocation and copy a string,
 * src is never null.
 */
static char* cloneString(const char* src) {
  char* dst = new char[strlen(src) + 1];
  strcpy(dst, src);
  return dst;
}

/**
 * Helper function AddExtNames():
 *   move extension names in the given properties array into extension cache.
 */
static bool addExtNames(std::vector<const char*>& extStore, uint32_t count,
                        VkExtensionProperties* properties) {
  if (!properties) return false;
  for (uint32_t i = 0; i < count; i++) {
    char* name = cloneString(properties[i].extensionName);
    extStore.push_back(name);
  }
  return true;
}

/**
 * Constructor: initialize (global) layer names and extensions
 */
LayerAndExtensions::LayerAndExtensions(void) {
  layerNames_.clear();
  extensions_.clear();

  // Retrieve instance extensions from the underline Vulkan implementation
  std::map<std::string, std::vector<const char*>> extMap;
  extMap.clear();

  uint32_t count = 0;
  CALL_VK(vkEnumerateInstanceExtensionProperties(nullptr, &count, nullptr));
  if (count) {
    VkExtensionProperties* prop = new VkExtensionProperties[count];
    CALL_VK(vkEnumerateInstanceExtensionProperties(nullptr, &count, prop));
    std::vector<const char*> extNames;
    addExtNames(extNames, count, prop);
    delete[] prop;

    extMap[std::string(VULKAN_DRIVER)] = extNames;
  }

  uint32_t layerCount;
  VkLayerProperties* layerProp = nullptr;
  CALL_VK(vkEnumerateInstanceLayerProperties(&layerCount, nullptr));
  if (layerCount) {
    layerProp = new VkLayerProperties[layerCount];
    CALL_VK(vkEnumerateInstanceLayerProperties(&layerCount, layerProp));
  }
  std::vector<char*> layers;
  for (uint32_t i = 0; i < layerCount; i++) {
    char* name = cloneString(layerProp[i].layerName);
    layers.push_back(name);

    CALL_VK(vkEnumerateInstanceExtensionProperties(name, &count, nullptr));
    if (count == 0) continue;

    VkExtensionProperties* prop = new VkExtensionProperties[count];
    CALL_VK(vkEnumerateInstanceExtensionProperties(name, &count, prop));
    std::vector<const char*> extNames;
    addExtNames(extNames, count, prop);
    delete[] prop;
    extMap[std::string(name)] = extNames;
  }
  // Instance layers are the available layers on the system, mapping it to VK_NULL_HANDLE
  // Other physical device layers are already enabled layers as once physical device is created
  //  could not enable/load new layers anymore(check spec for details).
  layerNames_[VK_NULL_HANDLE] = layers;
  extensions_[VK_NULL_HANDLE] = extMap;
  if (layerProp) delete[] layerProp;
}

/**
 * Report available layer names
 * @return: an array of layer name pointers, NULL if no layers are available
 */
const char* const* LayerAndExtensions::getLayerNames(void) {
  auto layers = layerNames_.find(VK_NULL_HANDLE);
  if (layers == layerNames_.end() || layers->second.empty()) {
    return nullptr;
  }

  return static_cast<char**>(layers->second.data());
}

/**
 * Report out available layers on the system + inside APK's native libs
 * @return available layers for the instance.
 */
uint32_t LayerAndExtensions::getLayerCount(void) {
  uint32_t count = 0;
  auto layers = layerNames_.find(VK_NULL_HANDLE);
  if (layers == layerNames_.end()) {
    return count;
  }

  count = layers->second.size();
  LOGI("InstLayerCount = %d", count);
  return count;
}

bool LayerAndExtensions::getExtensionNames(std::vector<const char*>& names, void* handle) {
  if (handle == VK_NULL_HANDLE) {
    // caller asks for instance extensions
    auto& exts = extensions_[VK_NULL_HANDLE];
    for(auto& e : exts) {
      names.insert(names.end(), e.second.begin(), e.second.end());
    }
    return true;
  }

  initDevExtensions(handle);
  if(extensions_.find(handle) == extensions_.end()) {
    LOGE("No extension for the physical device %p in %s", handle, __FUNCTION__);
    return false;
  }
  auto& exts = extensions_[handle];
  for(auto& e: exts) {
    names.insert(names.end(), e.second.begin(), e.second.end());
  }
  return true;
}

/**
 * Query for the available extensions, including the ones inside available
 * layers
 * @param type one of ExtensionType enums
 * @param handle either VK_NULL_HANDLE for instance extensions, or physical
 * device handles
 * @return available extensions for the queried type.
 */
uint32_t LayerAndExtensions::getExtensionCount(void* handle) {
  uint32_t count = 0;
  if (handle == VK_NULL_HANDLE) {
    // caller asks for instance extensions
    auto& exts = extensions_[VK_NULL_HANDLE];
    for(auto& e : exts) {
      count += e.second.size();
    }
    return count;
  }
  initDevExtensions(handle);
  if (extensions_.find(handle) == extensions_.end()) {
    LOGE("No extension for the physical device %p in %s", handle, __FUNCTION__);
    return 0;
  }
  auto& exts = extensions_[handle];
  for(auto& e : exts)
    count += e.second.size();

  return count;
}

bool LayerAndExtensions::isExtensionSupported(const char* extName,
    void* handle, const char** layerName) {
  if (extName == nullptr) return false;

  if (handle == VK_NULL_HANDLE) {
    // caller asks for the instance extension
    // caller asks for instance extensions
    auto& extMap = extensions_[VK_NULL_HANDLE];
    for(auto& exts: extMap) {
      for(auto& e: exts.second) {
        if(!strcmp(e, extName)) {
            if(layerName) *layerName = exts.first.c_str();
            return true;
        }
      }
    }
    return false;
  }

  initDevExtensions(handle);
  if (extensions_.find(handle) == extensions_.end()) {
    LOGE("No device extension for physical device %p in %s", handle, __FUNCTION__);
    return false;
  }
  auto &extensions = extensions_[handle];
  for (auto &exts: extensions) {
    for (auto &e: exts.second) {
      if (!strcmp(e, extName))
        return true;
    }
  }
  return false;
}

/**
 * Check whether the layer is supported. layers are common to instance and
 * devices, this simply check whether the given layer name is inside the
 * pre-built layerCache_
 * @param layerName the layer to check for supportability
 * @return true: supported, false: otherwise
 */
bool LayerAndExtensions::isLayerSupported(const char* layerName) {
  auto layers = layerNames_.find(VK_NULL_HANDLE);
  if (layers == layerNames_.end()) {
    // Internal cache is not built up, due to no layers found.
    return false;
  }

  // check for supportability
  for (auto name : layers->second) {
    if (!strcmp(name, layerName)) return true;
  }

  return false;
}

/**
 * Build up device extension cache, add to extCache_. Cache only gets built
 * once.
 * @param device VkPhysicalDevice
 */
void LayerAndExtensions::initDevExtensions(void* device) {
  auto it = extensions_.find(device);
  if (it != extensions_.end()) {
    // already in cache, no need to re-build.
    return;
  }
  VkPhysicalDevice physicalDev = reinterpret_cast<VkPhysicalDevice>(device);

  // get all enabled layers props, which have already been enabled when creating instance.
  uint32_t count = 0;
  VkLayerProperties* properties = nullptr;
  CALL_VK(vkEnumerateDeviceLayerProperties(physicalDev, &count, nullptr));
  if (count) {
    properties = new VkLayerProperties[count];
    CALL_VK(vkEnumerateDeviceLayerProperties(physicalDev, &count, properties));
  }

  // retrieve enabled layer names and their corresponding extensions
  std::map<std::string, std::vector<const char*>> extMap;
  std::vector<char*> layerNames;
  for (int i = 0; i < count; i++) {
    LOGI("layerName: %s for device %p", properties[i].layerName, physicalDev);
    char* name = cloneString(properties[i].layerName);
    layerNames.push_back(name);
    // Pull extensions supported by the layer and append to the extension list
    uint32_t extCount = 0;
    CALL_VK(vkEnumerateDeviceExtensionProperties(physicalDev, properties[i].layerName,
                                                 &extCount,nullptr));
    if (extCount) {
      std::vector<const char*> exts;
      VkExtensionProperties* extProp = new VkExtensionProperties[extCount];
      CALL_VK(vkEnumerateDeviceExtensionProperties(physicalDev, properties[i].layerName,
                                                   &extCount, extProp));
      addExtNames(exts, extCount, extProp);
      extMap[std::string(name)] = exts;
      delete[] extProp;
    }
  }
  layerNames_[physicalDev] = layerNames;
  if (properties) delete[] properties;

  // Get all implicitly supported extension properties at this physical device level
  uint32_t extCount = 0;
  CALL_VK(vkEnumerateDeviceExtensionProperties(physicalDev, nullptr, &extCount,
                                               nullptr));
  if (extCount) {
    VkExtensionProperties* extProp = new VkExtensionProperties[extCount];
    CALL_VK(vkEnumerateDeviceExtensionProperties(physicalDev, nullptr,
                                                 &extCount, extProp));
    std::vector<const char*> exts;
    addExtNames(exts, extCount, extProp);
    extMap[std::string(VULKAN_DRIVER)] = exts;
    delete[] extProp;
  }

  extensions_[physicalDev] = extMap;
}

/**
 * layer and extension printing
 */
void LayerAndExtensions::printLayers(void) {
  for (auto& layer : layerNames_) {
    LOGI("Available Layers for %p: ", layer.first);
    for (auto& name : layer.second) {
      LOGI("%s", name);
    }
  }
}

void LayerAndExtensions::printExtensions(void) {
  if (extensions_.empty()) return;
  for (auto& item : extensions_) {
    for(auto& exts: item.second) {
      LOGI("%s extensions for device %p, layer %s", item.first? "Device" : "Instance", item.first, exts.first.c_str());
      for (auto& e : exts.second) {
        LOGI("    %s", e);
      }
    }
  }
}

void LayerAndExtensions::printExtensions(const char* name, VkPhysicalDevice device) {
  auto layerNameKey = name ? std::string(name) : std::string(VULKAN_DRIVER);

  // layers are unified, must be exposed to instance already
  auto& extMap = extensions_[VK_NULL_HANDLE];
  if(extMap.find(layerNameKey) == extMap.end())
      return;

  LOGI("Instance Extensions for layer %s:", layerNameKey.c_str());
  auto& exts = extMap[layerNameKey];
  for(auto& e : exts) {
      LOGI("    %s", e);
  }

  if(device == VK_NULL_HANDLE)  return;
  initDevExtensions(device);

  extMap = extensions_[device];
  if(extMap.find(layerNameKey) == extMap.end())
    return;
  exts = extMap[layerNameKey];
  LOGI("Device extension for layer %s", layerNameKey.c_str());
  for(auto& e : exts) {
    LOGI("    %s", e);
  }
}

/**
 * Clean up caches upon exit.
 */
LayerAndExtensions::~LayerAndExtensions() {
  for (auto& layer : layerNames_) {
    for (auto& name : layer.second) {
      delete[] name;
    }
  }

  for (auto& layers : extensions_) {
    for (auto& exts : layers.second) {
      for(auto& name : exts.second)
        delete[] name;
    }
  }
}

/**
 * Debug Extension names.
 */
static const char* kDbgReportExtName = VK_EXT_DEBUG_REPORT_EXTENSION_NAME;
static const char* kDbgUtilsName = VK_EXT_DEBUG_UTILS_EXTENSION_NAME;

/*
 * Callback function for VK_EXT_DEBUG_REPORT_EXTENSION_NAME
 * This most likely are not used as the latest validation layer implemented the new
 *    VK_EXT_DEBUG_UTILS_EXTENSION_NAME.
 */
static VkBool32 VKAPI_PTR vkDebugReportCallbackEX_impl(
        VkDebugReportFlagsEXT flags, VkDebugReportObjectTypeEXT objectType,
        uint64_t object, size_t location, int32_t messageCode,
        const char* pLayerPrefix, const char* pMessage, void* pUserData) {
  // pUserData is not usable as we did not set it up when we were registering
  // the callback
  if (flags & VK_DEBUG_REPORT_INFORMATION_BIT_EXT) {
    __android_log_print(ANDROID_LOG_INFO, "Vulkan-Debug-Message: ", "%s -- %s",
                        pLayerPrefix, pMessage);
  }
  if (flags & VK_DEBUG_REPORT_WARNING_BIT_EXT) {
    __android_log_print(ANDROID_LOG_WARN, "Vulkan-Debug-Message: ", "%s -- %s",
                        pLayerPrefix, pMessage);
  }
  if (flags & VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT) {
    __android_log_print(ANDROID_LOG_WARN,
                        "Vulkan-Debug-Message-(Perf): ", "%s -- %s",
                        pLayerPrefix, pMessage);
  }
  if (flags & VK_DEBUG_REPORT_ERROR_BIT_EXT) {
    __android_log_print(ANDROID_LOG_ERROR, "Vulkan-Debug-Message: ", "%s -- %s",
                        pLayerPrefix, pMessage);
  }
  if (flags & VK_DEBUG_REPORT_DEBUG_BIT_EXT) {
    __android_log_print(ANDROID_LOG_DEBUG, "Vulkan-Debug-Message: ", "%s -- %s",
                        pLayerPrefix, pMessage);
  }

  return VK_FALSE;
}

/*
 * Callback function for VK_EXT_DEBUG_UTILS_EXTENSION_NAME
 */
VKAPI_ATTR VkBool32 VKAPI_CALL vkDebugUtilsMessengerEXT_impl(
        VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
        VkDebugUtilsMessageTypeFlagsEXT messageTypes,
        const VkDebugUtilsMessengerCallbackDataEXT* callbackData, void* userData) {
  const char validation[] = "Validation";
  const char performance[] = "Performance";
  const char error[] = "ERROR";
  const char warning[] = "WARNING";
  const char unknownType[] = "UNKNOWN_TYPE";
  const char unknownSeverity[] = "UNKNOWN_SEVERITY";
  const char* typeString = unknownType;
  const char* severityString = unknownSeverity;
  const char* messageIdName = callbackData->pMessageIdName;
  int32_t messageIdNumber = callbackData->messageIdNumber;
  const char* message = callbackData->pMessage;
  android_LogPriority priority = ANDROID_LOG_UNKNOWN;

  if (messageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT) {
    severityString = error;
    priority = ANDROID_LOG_ERROR;
  } else if (messageSeverity &
             VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT) {
    severityString = warning;
    priority = ANDROID_LOG_WARN;
  }
  if (messageTypes & VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT) {
    typeString = validation;
  } else if (messageTypes & VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT) {
    typeString = performance;
  }

  __android_log_print(priority, "AppName", "%s %s: [%s] Code %i : %s",
                      typeString, severityString, messageIdName,
                      messageIdNumber, message);

  // Returning false tells the layer not to stop when the event occurs, so
  // they see the same behavior with and without validation layers enabled.
  return VK_FALSE;
}

std::pair<const char*, const char*> LayerAndExtensions::getDbgReportExtInfo(void) {
  auto& instanceExts = extensions_[VK_NULL_HANDLE];

  for(auto const &layer: instanceExts) {
    for(auto const ext: layer.second) {
      if(!strcmp(ext, kDbgUtilsName)) {
        return std::make_pair(layer.first.c_str(), kDbgUtilsName);
      }
    }
  }
  for(auto const &layer: instanceExts) {
    for(auto const ext: layer.second) {
      if(!strcmp(ext, kDbgReportExtName)) {
        return std::make_pair(layer.first.c_str(), kDbgReportExtName);
      }
    }
  }
  return std::make_pair(nullptr, nullptr);
}

/**
 * Register our vkDebugReportCallbackEX_impl function to Vulkan so we could
 * process callbacks.
 *   - use VK_EXT_debug_utils if available, done.
 *   - use kDbgReportExtName  if available, done.
 *   - return false if none of the above 2 debug utils is available.
 * @param instance
 * @return true after the our debugging print handler is registered, false
 * otherwise.
 * (Code source: https://developer.android.com/ndk/guides/graphics/validation-layer?release=r21#debug)
 */
bool LayerAndExtensions::hookDbgReportExt(VkInstance instance) {
  if (isExtensionSupported(kDbgUtilsName,
                           VK_NULL_HANDLE, nullptr)) {
    PFN_vkCreateDebugUtilsMessengerEXT pfnCreateDebugUtilsMessengerEXT;
    PFN_vkDestroyDebugUtilsMessengerEXT pfnDestroyDebugUtilsMessengerEXT;
    pfnCreateDebugUtilsMessengerEXT =
        (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(
            instance, "vkCreateDebugUtilsMessengerEXT");
    pfnDestroyDebugUtilsMessengerEXT =
        (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(
            instance, "vkDestroyDebugUtilsMessengerEXT");

    // Create the debug messenger callback with desired settings
    if (pfnCreateDebugUtilsMessengerEXT) {
      VkDebugUtilsMessengerCreateInfoEXT messengerInfo;
      constexpr VkDebugUtilsMessageSeverityFlagsEXT kSeveritiesToLog =
          VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT |
          VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT;

      constexpr VkDebugUtilsMessageTypeFlagsEXT kMessagesToLog =
          VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
          VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
          VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;

      messengerInfo.sType =
          VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
      messengerInfo.pNext = nullptr;
      messengerInfo.flags = 0;
      messengerInfo.messageSeverity = kSeveritiesToLog;
      messengerInfo.messageType = kMessagesToLog;
      messengerInfo.pfnUserCallback =
          vkDebugUtilsMessengerEXT_impl;  // Callback example below
      messengerInfo.pUserData = nullptr;  // Custom user data passed to callback

      VkDebugUtilsMessengerEXT debugUtilsMessenger;
      CALL_VK(pfnCreateDebugUtilsMessengerEXT(instance, &messengerInfo, nullptr,
                                              &debugUtilsMessenger));
    }
    return true;
  }
  if (isExtensionSupported(kDbgReportExtName, VK_NULL_HANDLE, nullptr)) {
    if (!vkCreateDebugReportCallbackEXT) {
      vkCreateDebugReportCallbackEXT =
              (PFN_vkCreateDebugReportCallbackEXT) vkGetInstanceProcAddr(
                      instance, "vkCreateDebugReportCallbackEXT");
      vkDestroyDebugReportCallbackEXT =
              (PFN_vkDestroyDebugReportCallbackEXT) vkGetInstanceProcAddr(
                      instance, "vkDestroyDebugReportCallbackEXT");
      vkDebugReportMessageEXT =
              (PFN_vkDebugReportMessageEXT) vkGetInstanceProcAddr(
                      instance, "vkDebugReportMessageEXT");
    }

    VkDebugReportCallbackCreateInfoEXT dbgInfo = {
            .sType = VK_STRUCTURE_TYPE_DEBUG_REPORT_CREATE_INFO_EXT,
            .pNext = nullptr,
            .flags = VK_DEBUG_REPORT_WARNING_BIT_EXT |
                     VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT |
                     VK_DEBUG_REPORT_ERROR_BIT_EXT,
            .pfnCallback = vkDebugReportCallbackEX_impl,
            .pUserData = nullptr,  // no userdata we this object is not in memory
            // anymore when debug callback is happening
    };

    // Intend to keep the callback alive for the full Vulkan instance life cycle,
    // not caching the returned callback handle, not calling
    // vkDestroyDebugReportCallbackEXT either.
    VkDebugReportCallbackEXT callbackHandle;
    CALL_VK(vkCreateDebugReportCallbackEXT(instance, &dbgInfo, nullptr,
                                           &callbackHandle));
    return true;
  }
  return false;
}

//
// Created by roger on 2/21/2023.
//

#pragma once
#include "debug.hpp"
#include <cassert>

// Vulkan call wrapper
#define CALL_VK(func)                                  \
  if (VK_SUCCESS != (func)) {                          \
    LOGE("Vulkan error. File[%s], line[%d]", __FILE__, \
                        __LINE__);                     \
    assert(false);                                     \
  }


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
#include <android/log.h>
#include <memory_advice/memory_advice.h>
#include "VulkanMain.hpp"
#include <jni.h>
#include <string>

// Process the next main command.
void handle_cmd(android_app* app, int32_t cmd) {
  switch (cmd) {
    case APP_CMD_INIT_WINDOW:
      // The window is being shown, get it ready.
      InitVulkan(app);
      break;
    case APP_CMD_TERM_WINDOW:
      // The window is being hidden or closed, clean it up.
      DeleteVulkan();
      break;
    default:
      __android_log_print(ANDROID_LOG_INFO, "Vulkan Tutorials",
                          "event not handled: %d", cmd);
  }
}

JavaVM* gVM = nullptr;
JNIEXPORT jint JNI_OnLoad(JavaVM *vm, void *reserved) {
  gVM = vm;

  return JNI_VERSION_1_6;
}

volatile static MemoryAdvice_MemoryState memoryState = MemoryAdvice_MemoryState::MEMORYADVICE_STATE_OK;
volatile static MemoryAdvice_MemoryState preMemoryState = memoryState;
void  check_memory_state(void) {
    memoryState = MemoryAdvice_getMemoryState();
    auto state = memoryState;
    std::string msg;
    switch (state) {
        case MEMORYADVICE_STATE_OK:  msg = "MEMORYADVICE_STATE_OK"; break;
        case MEMORYADVICE_STATE_APPROACHING_LIMIT: msg = "MEMORYADVICE_STATE_APPROACHING_LIMIT"; break;
        case MEMORYADVICE_STATE_CRITICAL: msg = "MEMORYADVICE_STATE_CRITICAL"; break;
        default: msg = "UNKNOWN_STATE"; break;
    }

    // __android_log_print(ANDROID_LOG_INFO, "Vulkan Tutorial06", "MemoryState: %s", msg.c_str());
}


void MemoryAdviceWatcher(MemoryAdvice_MemoryState state, void *user_data) {
    // __android_log_print(ANDROID_LOG_INFO, "Vulkan Tutorial 06", "===>Watcher callback with MemoryAdvice_MemoryState = %d", state);
    struct android* app = reinterpret_cast<struct android*>(user_data);
    if (user_data == nullptr) return;
    memoryState = state;

    auto percent = MemoryAdvice_getPercentageAvailableMemory();

    // __android_log_print(ANDROID_LOG_INFO, "Vulkan Tutorial 06", "<===Watcher memory percentage: %f", percent);
}

#include <queue>
#include <cassert>
class MemoryAllocation {
const uint32_t  ALLOCATION_SIZE = (100 * 1024 * 1024);
public:
    MemoryAllocation() {
        memory = new char[ALLOCATION_SIZE];
        memset(memory, 0xCC, ALLOCATION_SIZE * sizeof(char));
        assert(memory);
    }

    ~MemoryAllocation() {
        delete [] memory;
    }
private:
    void *memory;
};
void ManageGameResources(void) {
    // GameResource states
    static int32_t state=0;  // 0: allocating state, 1: deallocating state
    auto curState = memoryState;

    static std::queue<MemoryAllocation*> memories;

    switch (state) {
        case 0:
            if (curState == MEMORYADVICE_STATE_CRITICAL) {
                state = 1;
                assert(memories.size());
            } else {
                // keeps on allocating memories
                auto block = new MemoryAllocation;
                assert(block);
                memories.push(block);
            }
            break;
        case 1:
            if (memories.empty()) {
                assert(memoryState == MemoryAdvice_MemoryState::MEMORYADVICE_STATE_OK);
                state = 0;
            } else {
                delete memories.front();
                memories.pop();
            }
            break;
        default:
            __android_log_print(ANDROID_LOG_ERROR, "Vulkan Tutorial06", "UNKNOWN_STATE in %s", __FUNCTION__ );
            break;
    }
}
void android_main(struct android_app* app) {

  // Set the callback to process system events
  app->onAppCmd = handle_cmd;

  JNIEnv *env = nullptr;
  gVM->AttachCurrentThread(&env, NULL);
  jobject javaGameActivity = env->NewGlobalRef(app->activity->javaGameActivity);
  MemoryAdvice_init(env, javaGameActivity);
  memoryState = MemoryAdvice_getMemoryState();

  auto registerResult = MemoryAdvice_registerWatcher(static_cast<uint64_t>(160), MemoryAdviceWatcher, app);
  //__android_log_print(ANDROID_LOG_INFO, "Vulkan Tutorial 06", "Watcher register result = %d", registerResult);

  // Used to poll the events in the main loop
  int events;
  android_poll_source* source;

  // Main loop
#if 0
do {
    if (ALooper_pollAll(IsVulkanReady() ? 1 : 0, nullptr,
                        &events, (void**)&source) >= 0) {
      if (source != NULL) source->process(app, source);
    }

    // render if vulkan is ready
    if (IsVulkanReady()) {
      check_memory_state();
      ManageGameResources();
      VulkanDrawFrame();
    }
  } while (app->destroyRequested == 0);
#else
    while (1) {

        // If not animating, block until we get an event;
        // If animating, don't block.
        while ((ALooper_pollAll(IsVulkanReady()? 0 : -1, NULL, &events,
                                (void **) &source)) >= 0) {
            if (source != NULL) {
                source->process(source->app, source);
            }
            if (app->destroyRequested) {
                MemoryAdvice_unregisterWatcher(MemoryAdviceWatcher);
                env->DeleteGlobalRef(javaGameActivity);
                return;
            }
        }

        // in the following code, I am dealing with 3 interfaces:
        //  Glue, GameActivity, and Framework(NDK). could I just do one?
        auto ib = android_app_swap_input_buffers(app);
        __android_log_print(ANDROID_LOG_INFO, "Vulkan Tutorial06",
                            "MotionEvent ib (%p)", ib);
        if (ib && ib->motionEventsCount) {
            __android_log_print(ANDROID_LOG_INFO, "Vulkan Tutorial06",
                                "Got MotionEvent: %d", ib->motionEventsCount);
            for (int i = 0; i < ib->motionEventsCount; i++) {
                auto event = & ib->motionEvents[i];
                __android_log_print(ANDROID_LOG_INFO, "MotionEvent", "Got raw MotionEvent list(%d): %d",
                                    i, event->action);
                auto action = event->action & AMOTION_EVENT_ACTION_MASK;
                if (action  == AMOTION_EVENT_ACTION_UP  ||
                    action  == AMOTION_EVENT_ACTION_POINTER_UP) {
                    int32_t ptrIdx = ((event->action & AMOTION_EVENT_ACTION_POINTER_INDEX_MASK)
                                     >> AMOTION_EVENT_ACTION_POINTER_INDEX_SHIFT);
                    auto x = GameActivityPointerAxes_getAxisValue(&event->pointers[ptrIdx], AMOTION_EVENT_AXIS_X);
                    auto y = GameActivityPointerAxes_getAxisValue(&event->pointers[ptrIdx], AMOTION_EVENT_AXIS_Y);
                    __android_log_print(ANDROID_LOG_INFO, "MotionEvent",
                                        "ptr: %d, coordinates:(%f, %f)", ptrIdx, x, y);
                }
            }
            android_app_clear_motion_events(ib);
        }

        if (IsVulkanReady()) {
            if (IsVulkanReady()) {
                check_memory_state();
                ManageGameResources();
                VulkanDrawFrame();
            }
        }
    }
#endif

}

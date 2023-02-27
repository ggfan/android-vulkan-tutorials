Tutorial 6 - Texture
=============================
Render a textured triangle


Description
----------
*  adding texture to triangle


Requirement
--------------
Pre-build shaderc with:
```
 mkdir -p  app/src/main/cpp/shaderc
 cd app/src/main/cpp/shaderc
 ${ndk_dir}/ndk-build NDK_PROJECT_PATH=. APP_BUILD_SCRIPT=${ANDROID_NDK}/sources/third_party/shaderc/Android.mk APP_STL:=${ANDROID_STL} APP_ABI:=all APP_PLATFORM:=${your-minSdkLevel} libshaderc_combined
```
Note that, on Windows, refer to https://github.com/android/ndk/issues/1815 for the workaround in the android.mk file.

Screenshot
------------
<img src="./Tutorial_6_Screenshot.png" height="400px">

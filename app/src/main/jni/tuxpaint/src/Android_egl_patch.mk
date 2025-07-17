LOCAL_PATH := $(call my-dir)

# Build sdl_egl_patch shared library for EGL synchronization
include $(CLEAR_VARS)

LOCAL_MODULE    := sdl_egl_patch
LOCAL_CFLAGS    := -Wall -Werror
LOCAL_LDLIBS    := -llog -lGLESv2 -lEGL -landroid
LOCAL_SRC_FILES := sdl_egl_patch.c sdl_egl_jni_bridge.c
LOCAL_C_INCLUDES := $(LOCAL_PATH)/../../../../../src/main/jni/SDL2/include

# For compatibility with libSDL2
LOCAL_SHARED_LIBRARIES := SDL2

include $(BUILD_SHARED_LIBRARY)

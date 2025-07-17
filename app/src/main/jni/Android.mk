# Include all subdirectory makefiles
include $(call all-subdir-makefiles)

# Build sdl_egl_patch directly here instead of including a separate makefile

# Build sdl_egl_patch shared library for EGL synchronization
include $(CLEAR_VARS)

LOCAL_MODULE    := sdl_egl_patch
LOCAL_CFLAGS    := -Wall -Werror
LOCAL_LDLIBS    := -llog -lGLESv2 -lEGL -landroid
LOCAL_SRC_FILES := tuxpaint/src/sdl_egl_patch.c \
                   tuxpaint/src/sdl_egl_jni_bridge.c \
                   tuxpaint/src/tuxpaint_egl_sync.c
LOCAL_C_INCLUDES := $(LOCAL_PATH)/SDL2/include

# For compatibility with libSDL2
LOCAL_SHARED_LIBRARIES := SDL2

include $(BUILD_SHARED_LIBRARY)


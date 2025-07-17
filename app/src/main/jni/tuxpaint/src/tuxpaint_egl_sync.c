/*
 * Tuxpaint EGL Context Synchronization
 * 
 * This file adds thread synchronization to SDL's EGL context management
 * to fix EGL_BAD_ACCESS errors when multiple threads attempt to access
 * the same EGL context.
 * 
 * This implementation uses function interception to wrap EGL calls with
 * mutex protection, ensuring only one thread can access the EGL context
 * at a time.
 */

#include <jni.h>
#include <android/log.h>
#include <pthread.h>
#include <dlfcn.h>
#include <EGL/egl.h>
#include <unistd.h>

#define TAG "TuxPaint-EGL-Sync"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, TAG, __VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, TAG, __VA_ARGS__)

// Global mutex for EGL context operations
static pthread_mutex_t egl_mutex = PTHREAD_MUTEX_INITIALIZER;
static int egl_mutex_initialized = 0;

// Function pointers for original EGL functions
static EGLBoolean (*real_eglMakeCurrent)(EGLDisplay dpy, EGLSurface draw, EGLSurface read, EGLContext ctx);
static EGLBoolean (*real_eglSwapBuffers)(EGLDisplay dpy, EGLSurface surface);
static EGLContext (*real_eglCreateContext)(EGLDisplay dpy, EGLConfig config, EGLContext share_context, const EGLint *attrib_list);
static EGLBoolean (*real_eglDestroyContext)(EGLDisplay dpy, EGLContext ctx);

// Thread tracking to detect when multiple threads try to use same context
static pthread_t egl_context_owner = 0;
static void* egl_context = NULL;

// Initialize the EGL mutex
static void init_egl_mutex(void) {
    if (egl_mutex_initialized) {
        LOGI("EGL mutex already initialized");
        return;
    }
    
    pthread_mutexattr_t attr;
    pthread_mutexattr_init(&attr);
    pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_RECURSIVE);
    int result = pthread_mutex_init(&egl_mutex, &attr);
    pthread_mutexattr_destroy(&attr);
    
    if (result == 0) {
        egl_mutex_initialized = 1;
        LOGI("EGL mutex initialized successfully");
    } else {
        LOGE("Failed to initialize EGL mutex, error code: %d", result);
    }
}

// JNI methods to initialize and control the EGL mutex

JNIEXPORT void JNICALL
Java_org_tuxpaint_tuxpaintActivity_initEGLMutex(JNIEnv *env, jclass clazz) {
    init_egl_mutex();
}

JNIEXPORT void JNICALL
Java_org_tuxpaint_tuxpaintActivity_lockEGLContext(JNIEnv *env, jclass clazz) {
    if (!egl_mutex_initialized) {
        LOGE("Attempting to lock uninitialized EGL mutex");
        return;
    }
    
    int result = pthread_mutex_lock(&egl_mutex);
    if (result == 0) {
        LOGI("EGL mutex locked");
        return;
    } else {
        LOGE("Failed to lock EGL mutex, error code: %d", result);
        return;
    }
}

JNIEXPORT void JNICALL
Java_org_tuxpaint_tuxpaintActivity_unlockEGLContext(JNIEnv *env, jclass clazz) {
    if (!egl_mutex_initialized) {
        LOGE("Attempting to unlock uninitialized EGL mutex");
        return;
    }
    
    int result = pthread_mutex_unlock(&egl_mutex);
    if (result == 0) {
        LOGI("EGL mutex unlocked");
        return;
    } else {
        LOGE("Failed to unlock EGL mutex, error code: %d", result);
        return;
    }
}

// Track which thread is using the EGL context
JNIEXPORT void JNICALL
Java_org_tuxpaint_tuxpaintActivity_trackEGLContext(JNIEnv *env, jclass clazz, jlong contextPtr) {
    void* context = (void*)contextPtr;
    pthread_t current_thread = pthread_self();
    
    if (context) {
        LOGI("EGL context %p is now owned by thread %lu", context, (unsigned long)current_thread);
        
        // If a different thread already owns this context, log a warning
        if (egl_context_owner != 0 && !pthread_equal(current_thread, egl_context_owner)) {
            LOGE("EGL CONTEXT VIOLATION: Thread %lu trying to use context %p previously owned by thread %lu", 
                (unsigned long)current_thread, context, (unsigned long)egl_context_owner);
        }
        
        // Update the context owner
        egl_context = context;
        egl_context_owner = current_thread;
    } else {
        // Clear ownership if no context
        egl_context = NULL;
        egl_context_owner = 0;
        LOGI("EGL context ownership cleared");
    }
}

// Utility functions to be called from C code
void tuxpaint_egl_lock(void) {
    if (!egl_mutex_initialized) {
        init_egl_mutex();
    }
    
    pthread_mutex_lock(&egl_mutex);
}

void tuxpaint_egl_unlock(void) {
    pthread_mutex_unlock(&egl_mutex);
}

void tuxpaint_check_egl_context(void* ctx) {
    if (ctx != NULL) {
        pthread_t current_thread = pthread_self();
        
        // Check if a different thread is trying to use our context
        if (ctx == egl_context && egl_context_owner != 0 && 
            !pthread_equal(current_thread, egl_context_owner)) {
            // Log a warning about potential EGL_BAD_ACCESS error
            __android_log_print(ANDROID_LOG_ERROR, "Tux-EGL-Sync", 
                              "EGL CONTEXT VIOLATION: Thread %lu trying to use context %p owned by thread %lu", 
                              (unsigned long)current_thread, ctx, (unsigned long)egl_context_owner);
        }
        
        // Update the context owner
        egl_context = ctx;
        egl_context_owner = current_thread;
        __android_log_print(ANDROID_LOG_INFO, "Tux-EGL-Sync", 
                         "EGL context %p is now owned by thread %lu", ctx, (unsigned long)current_thread);
    } else {
        // Clear the owner if no context
        egl_context = NULL;
        egl_context_owner = 0;
    }
}

void tuxpaint_release_egl_context(void* ctx) {
    tuxpaint_egl_lock();
    
    // Only clear context ownership if this is our current context
    if (ctx == egl_context) {
        egl_context = NULL;
        egl_context_owner = 0;
        __android_log_print(ANDROID_LOG_INFO, "Tux-EGL-Sync", "EGL context ownership cleared");
    }
    
    tuxpaint_egl_unlock();
}

// Wrapper functions that intercept EGL calls
EGLBoolean eglMakeCurrent(EGLDisplay dpy, EGLSurface draw, EGLSurface read, EGLContext ctx) {
    EGLBoolean result;
    
    if (!real_eglMakeCurrent) {
        // If we haven't loaded the real function yet, do it now
        real_eglMakeCurrent = dlsym(RTLD_NEXT, "eglMakeCurrent");
        if (!real_eglMakeCurrent) {
            LOGE("Failed to find original eglMakeCurrent function");
            return EGL_FALSE;
        }
    }
    
    // Lock before changing context
    tuxpaint_egl_lock();
    
    // Check if another thread is trying to use context
    tuxpaint_check_egl_context(ctx);
    
    // Call the real function
    result = real_eglMakeCurrent(dpy, draw, read, ctx);
    
    // If we're releasing the context, clear the owner
    if (ctx == EGL_NO_CONTEXT) {
        tuxpaint_release_egl_context(ctx);
    }
    
    // Unlock
    tuxpaint_egl_unlock();
    
    return result;
}

EGLBoolean eglSwapBuffers(EGLDisplay dpy, EGLSurface surface) {
    EGLBoolean result;
    
    if (!real_eglSwapBuffers) {
        real_eglSwapBuffers = dlsym(RTLD_NEXT, "eglSwapBuffers");
        if (!real_eglSwapBuffers) {
            LOGE("Failed to find original eglSwapBuffers function");
            return EGL_FALSE;
        }
    }
    
    // Lock before swapping buffers
    tuxpaint_egl_lock();
    
    // Call the real function
    result = real_eglSwapBuffers(dpy, surface);
    
    // Unlock
    tuxpaint_egl_unlock();
    
    return result;
}

EGLContext eglCreateContext(EGLDisplay dpy, EGLConfig config, EGLContext share_context, const EGLint *attrib_list) {
    EGLContext result;
    
    if (!real_eglCreateContext) {
        real_eglCreateContext = dlsym(RTLD_NEXT, "eglCreateContext");
        if (!real_eglCreateContext) {
            LOGE("Failed to find original eglCreateContext function");
            return EGL_NO_CONTEXT;
        }
    }
    
    // Lock before creating context
    tuxpaint_egl_lock();
    
    // Call the real function
    result = real_eglCreateContext(dpy, config, share_context, attrib_list);
    
    // Unlock
    tuxpaint_egl_unlock();
    
    return result;
}

EGLBoolean eglDestroyContext(EGLDisplay dpy, EGLContext ctx) {
    EGLBoolean result;
    
    if (!real_eglDestroyContext) {
        real_eglDestroyContext = dlsym(RTLD_NEXT, "eglDestroyContext");
        if (!real_eglDestroyContext) {
            LOGE("Failed to find original eglDestroyContext function");
            return EGL_FALSE;
        }
    }
    
    // Lock before destroying context
    tuxpaint_egl_lock();
    
    // Release the context
    tuxpaint_release_egl_context(ctx);
    
    // Call the real function
    result = real_eglDestroyContext(dpy, ctx);
    
    // Unlock
    tuxpaint_egl_unlock();
    
    return result;
}

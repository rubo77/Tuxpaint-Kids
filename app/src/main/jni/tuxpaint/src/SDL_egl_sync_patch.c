#include <jni.h>
#include <android/log.h>
#include <pthread.h>

/* 
 * SDL EGL Synchronization Patch
 * 
 * This file patches SDL's EGL implementation with thread synchronization
 * to prevent EGL_BAD_ACCESS errors when multiple threads attempt to access
 * the same EGL context.
 */

#define TAG "SDL-EGL-SYNC"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, TAG, __VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, TAG, __VA_ARGS__)

// Global mutex for EGL context operations
static pthread_mutex_t egl_mutex = PTHREAD_MUTEX_INITIALIZER;

// Thread tracking for the EGL context
static pthread_t egl_context_owner = 0;
static void* current_egl_context = NULL;

void SDL_EGL_LockContext(void) {
    pthread_mutex_lock(&egl_mutex);
    LOGI("SDL_EGL_LockContext: mutex locked by thread %lu", (unsigned long)pthread_self());
}

void SDL_EGL_UnlockContext(void) {
    LOGI("SDL_EGL_UnlockContext: mutex unlocked by thread %lu", (unsigned long)pthread_self());
    pthread_mutex_unlock(&egl_mutex);
}

// This function will be called from our patched SDL_egl.c
void SDL_EGL_SetContextThread(void* context) {
    if (context) {
        pthread_t current = pthread_self();
        
        // Check if a different thread is trying to use the context
        if (context == current_egl_context && egl_context_owner != 0 && 
            !pthread_equal(current, egl_context_owner)) {
            LOGE("EGL CONTEXT VIOLATION: Thread %lu trying to use context %p owned by thread %lu", 
                 (unsigned long)current, context, (unsigned long)egl_context_owner);
        }
        
        // Update the owner
        current_egl_context = context;
        egl_context_owner = current;
        LOGI("EGL context %p is now owned by thread %lu", context, (unsigned long)current);
    } else {
        // Clear the owner if no context
        current_egl_context = NULL;
        egl_context_owner = 0;
        LOGI("EGL context ownership cleared");
    }
}

// Initialize and register JNI functions
JNIEXPORT void JNICALL
Java_org_tuxpaint_tuxpaintActivity_initSDLEGLSynchronization(JNIEnv *env, jclass clazz) {
    LOGI("Initializing SDL EGL synchronization");
    
    pthread_mutexattr_t attr;
    pthread_mutexattr_init(&attr);
    pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_RECURSIVE);
    pthread_mutex_init(&egl_mutex, &attr);
    pthread_mutexattr_destroy(&attr);
    
    LOGI("SDL EGL synchronization initialized");
}

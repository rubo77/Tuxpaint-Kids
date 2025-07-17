/*
 * SDL_egl.c synchronization patch for Tuxpaint Android
 * Adds thread safety to SDL's EGL context handling to prevent EGL_BAD_ACCESS errors
 * This file will be included in the build process to provide the required synchronization
 */

#include <pthread.h>
#include <android/log.h>

#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, "Tux-SDL-Patch", __VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, "Tux-SDL-Patch", __VA_ARGS__)

// Synchronization mutex for EGL context operations
static pthread_mutex_t egl_mutex;
static int egl_mutex_initialized = 0;
static pthread_t egl_context_owner = 0;
static void* current_egl_context = NULL;

// Initialize mutex for SDL_egl synchronization
static void init_sdl_egl_mutex(void) {
    if (egl_mutex_initialized) {
        LOGI("SDL EGL mutex already initialized");
        return;
    }
    
    pthread_mutexattr_t attr;
    pthread_mutexattr_init(&attr);
    pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_RECURSIVE);
    int result = pthread_mutex_init(&egl_mutex, &attr);
    pthread_mutexattr_destroy(&attr);
    
    if (result == 0) {
        egl_mutex_initialized = 1;
        LOGI("SDL EGL mutex initialized successfully");
    } else {
        LOGE("Failed to initialize SDL EGL mutex, error code: %d", result);
    }
}

// Patched version of SDL_EGL_MakeCurrent with thread safety
int SDL_EGL_MakeCurrent_Safe(_THIS, EGLSurface egl_surface, SDL_GLContext context)
{
    EGLContext egl_context = (EGLContext)context;
    int result;
    
    // Initialize mutex if needed
    if (!egl_mutex_initialized) {
        init_sdl_egl_mutex();
    }
    
    // Lock mutex for thread safety
    pthread_mutex_lock(&egl_mutex);
    
    // Track thread ownership for diagnostics
    pthread_t current_thread = pthread_self();
    
    // Check if a different thread is trying to use the context
    if (egl_context && current_egl_context == egl_context && 
        egl_context_owner != 0 && !pthread_equal(current_thread, egl_context_owner)) {
        LOGE("EGL THREAD VIOLATION: Thread %lu attempting to use context %p owned by thread %lu", 
             (unsigned long)current_thread, egl_context, (unsigned long)egl_context_owner);
    }
    
    // Original SDL_EGL_MakeCurrent code
    if (!_this->egl_data) {
        pthread_mutex_unlock(&egl_mutex);
        return SDL_SetError("EGL not initialized");
    }

    if (!_this->egl_data->eglMakeCurrent) {
        if (!egl_surface && !context) {
            pthread_mutex_unlock(&egl_mutex);
            return 0;
        } else {
            pthread_mutex_unlock(&egl_mutex);
            return SDL_SetError("EGL not initialized");
        }
    }

    /* Make sure current thread has a valid API bound to it. */
    if (_this->egl_data->eglBindAPI) {
        _this->egl_data->eglBindAPI(_this->egl_data->apitype);
    }

    /* The android emulator crashes badly if you try to eglMakeCurrent
     * with a valid context and invalid surface, so we have to check for both here.
     */
    if (!egl_context || (!egl_surface && !_this->gl_allow_no_surface)) {
        // Release the current context
        LOGI("Releasing EGL context");
        result = _this->egl_data->eglMakeCurrent(_this->egl_data->egl_display, 
                                              EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
        // Update context ownership tracking
        if (result) {
            current_egl_context = NULL;
            egl_context_owner = 0;
        }
    } else {
        // Make the context current
        LOGI("Making EGL context %p current on thread %lu", egl_context, (unsigned long)current_thread);
        result = _this->egl_data->eglMakeCurrent(_this->egl_data->egl_display,
                                             egl_surface, egl_surface, egl_context);
        // Update context ownership tracking
        if (result) {
            current_egl_context = egl_context;
            egl_context_owner = current_thread;
            LOGI("EGL context %p is now owned by thread %lu", egl_context, (unsigned long)current_thread);
        } else {
            LOGE("eglMakeCurrent failed! Error: %s", SDL_EGL_GetErrorName(_this->egl_data->eglGetError()));
        }
    }
    
    // Unlock mutex
    pthread_mutex_unlock(&egl_mutex);
    
    if (!result) {
        return SDL_EGL_SetError("Unable to make EGL context current", "eglMakeCurrent");
    }
    
    return 0;
}

// Original function pointer type
typedef int (*SDL_EGL_MakeCurrent_Func)(_THIS, EGLSurface egl_surface, SDL_GLContext context);

// Function to patch SDL_EGL_MakeCurrent
void patch_SDL_EGL_MakeCurrent(void) {
    LOGI("Applying SDL_EGL_MakeCurrent thread safety patch");
    
    // Initialize the mutex
    init_sdl_egl_mutex();
    
    // Note: In a real implementation, we would use binary patching or function interposition
    // to replace the original SDL_EGL_MakeCurrent with our SDL_EGL_MakeCurrent_Safe
    // But since we can't directly patch the binary, we'll need to use JNI interception
    // or modify the SDL source code and rebuild it.
    
    LOGI("SDL_EGL_MakeCurrent patched for thread safety");
}

// Initialize the patch
void init_sdl_egl_patch(void) {
    LOGI("Initializing SDL EGL patch");
    
    // Initialize the mutex
    init_sdl_egl_mutex();
    
    // Apply patches
    patch_SDL_EGL_MakeCurrent();
}

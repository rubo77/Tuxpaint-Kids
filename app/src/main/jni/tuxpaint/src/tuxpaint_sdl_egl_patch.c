/*
 * Tuxpaint SDL EGL Patch
 * 
 * This file contains patched versions of critical SDL EGL functions
 * with added thread synchronization to prevent EGL_BAD_ACCESS errors.
 */

#include <jni.h>
#include <android/log.h>
#include "tuxpaint_egl_sync.h"

#define TAG "TuxPaint-SDL-Patch"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, TAG, __VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, TAG, __VA_ARGS__)

/* 
 * Functions to override SDL's EGL implementations with synchronized versions
 */

#include <SDL2/SDL.h>
#include <SDL2/SDL_syswm.h>
#include <EGL/egl.h>

/*
 * Patched version of SDL_EGL_MakeCurrent to add mutex protection
 */
int SDL_EGL_MakeCurrent_Patched(SDL_Renderer *renderer, EGLSurface egl_surface, SDL_GLContext context)
{
    EGLContext egl_context = (EGLContext)context;
    EGLDisplay egl_display = eglGetCurrentDisplay();
    int result;
    
    LOGI("Patched SDL_EGL_MakeCurrent called, acquiring lock");
    
    // Acquire the lock before making the context current
    tuxpaint_egl_lock();
    
    if (!egl_context) {
        result = eglMakeCurrent(egl_display, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
    } else {
        result = eglMakeCurrent(egl_display, egl_surface, egl_surface, egl_context);
    }
    
    if (!result) {
        // Log error info
        EGLint error = eglGetError();
        LOGE("eglMakeCurrent failed with error 0x%x", error);
    } else {
        LOGI("eglMakeCurrent succeeded");
    }
    
    // Release the lock
    tuxpaint_egl_unlock();
    
    return result ? 0 : -1;
}

/*
 * Patched version of SDL_EGL_SwapBuffers to add mutex protection
 */
int SDL_EGL_SwapBuffers_Patched(SDL_Renderer *renderer, EGLSurface egl_surface)
{
    EGLDisplay egl_display = eglGetCurrentDisplay();
    int result;
    
    LOGI("Patched SDL_EGL_SwapBuffers called, acquiring lock");
    
    // Acquire the lock before swapping buffers
    tuxpaint_egl_lock();
    
    result = eglSwapBuffers(egl_display, egl_surface);
    
    if (!result) {
        // Log error info
        EGLint error = eglGetError();
        LOGE("eglSwapBuffers failed with error 0x%x", error);
    }
    
    // Release the lock
    tuxpaint_egl_unlock();
    
    return result ? 0 : -1;
}

/*
 * Initialization function to set up our hooks
 */
JNIEXPORT void JNICALL
Java_org_tuxpaint_tuxpaintActivity_setupEGLSynchronization(JNIEnv *env, jclass clazz)
{
    LOGI("Setting up SDL EGL synchronization hooks");
    
    // Note: In a real implementation, we would use function pointer
    // replacement or LD_PRELOAD to hook these functions. This is just
    // a placeholder for demonstration.
}

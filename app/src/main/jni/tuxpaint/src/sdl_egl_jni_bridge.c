/*
 * SDL_egl JNI bridge for Tuxpaint Android
 * Provides JNI functions to initialize and manage SDL EGL context synchronization
 */

#include <jni.h>
#include <android/log.h>
#include <pthread.h>

#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, "Tux-SDL-JNI-Bridge", __VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, "Tux-SDL-JNI-Bridge", __VA_ARGS__)

// External declarations for functions in sdl_egl_patch.c
extern void init_sdl_egl_patch(void);

// Global references
static pthread_mutex_t jni_mutex = PTHREAD_MUTEX_INITIALIZER;
static int jni_initialized = 0;

// Initialize the SDL EGL patch
JNIEXPORT void JNICALL
Java_org_tuxpaint_tuxpaintActivity_initSDLEGLPatch(JNIEnv *env, jclass cls)
{
    LOGI("JNI: Initializing SDL EGL patch from Java");
    
    pthread_mutex_lock(&jni_mutex);
    
    if (!jni_initialized) {
        // Initialize the SDL EGL patch
        init_sdl_egl_patch();
        jni_initialized = 1;
        LOGI("JNI: SDL EGL patch initialized successfully");
    } else {
        LOGI("JNI: SDL EGL patch already initialized");
    }
    
    pthread_mutex_unlock(&jni_mutex);
}

// Check if the SDL EGL patch is initialized
JNIEXPORT jboolean JNICALL
Java_org_tuxpaint_tuxpaintActivity_isSDLEGLPatchInitialized(JNIEnv *env, jclass cls)
{
    return (jboolean)jni_initialized;
}

// Log the current thread ID (useful for debugging)
JNIEXPORT void JNICALL
Java_org_tuxpaint_tuxpaintActivity_logCurrentThread(JNIEnv *env, jclass cls, jstring tag)
{
    const char *tagStr = (*env)->GetStringUTFChars(env, tag, NULL);
    pthread_t thread_id = pthread_self();
    
    LOGI("%s: Current thread ID: %lu", tagStr, (unsigned long)thread_id);
    
    (*env)->ReleaseStringUTFChars(env, tag, tagStr);
}

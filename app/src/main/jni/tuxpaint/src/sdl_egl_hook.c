#include <jni.h>
#include <android/log.h>
#include <pthread.h>
#include <dlfcn.h>

#define TAG "Tux-EGL-Hook"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, TAG, __VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, TAG, __VA_ARGS__)

/* Global mutex for EGL context operations */
static pthread_mutex_t egl_mutex = PTHREAD_MUTEX_INITIALIZER;
static int egl_mutex_initialized = 0;

/* Thread tracking for the EGL context */
static pthread_t egl_context_owner = 0;
static void* current_egl_context = NULL;

/* Initialize the EGL mutex */
static void init_egl_mutex() {
    if (!egl_mutex_initialized) {
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
}

/* JNI methods for EGL context synchronization */
JNIEXPORT void JNICALL
Java_org_tuxpaint_tuxpaintActivity_initEGLMutex(JNIEnv *env, jclass clazz) {
    LOGI("Initializing EGL mutex for synchronization");
    init_egl_mutex();
}

/* This function is called from the Java side before any SDL operation that uses the EGL context */
JNIEXPORT void JNICALL
Java_org_tuxpaint_tuxpaintActivity_lockEGLContext(JNIEnv *env, jclass clazz) {
    if (!egl_mutex_initialized) {
        init_egl_mutex();
    }
    
    pthread_mutex_lock(&egl_mutex);
    LOGI("EGL mutex locked by thread %lu", (unsigned long)pthread_self());
}

/* This function is called from the Java side after any SDL operation that uses the EGL context */
JNIEXPORT void JNICALL
Java_org_tuxpaint_tuxpaintActivity_unlockEGLContext(JNIEnv *env, jclass clazz) {
    pthread_mutex_unlock(&egl_mutex);
    LOGI("EGL mutex unlocked by thread %lu", (unsigned long)pthread_self());
}

/* Function to track which thread is using the EGL context */
JNIEXPORT void JNICALL
Java_org_tuxpaint_tuxpaintActivity_trackEGLContext(JNIEnv *env, jclass clazz, jlong contextPtr) {
    void* context = (void*)contextPtr;
    
    if (context) {
        pthread_t current = pthread_self();
        
        /* Check if a different thread is trying to use the context */
        if (context == current_egl_context && egl_context_owner != 0 && 
            !pthread_equal(current, egl_context_owner)) {
            LOGE("EGL CONTEXT VIOLATION: Thread %lu trying to use context %p owned by thread %lu", 
                (unsigned long)current, context, (unsigned long)egl_context_owner);
        }
        
        /* Update the owner */
        current_egl_context = context;
        egl_context_owner = current;
        LOGI("EGL context %p is now owned by thread %lu", context, (unsigned long)current);
    } else {
        /* Clear the owner if no context */
        current_egl_context = NULL;
        egl_context_owner = 0;
        LOGI("EGL context ownership cleared");
    }
}

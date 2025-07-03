/*
 * Tuxpaint EGL Context Synchronization Header
 */

#ifndef TUXPAINT_EGL_SYNC_H
#define TUXPAINT_EGL_SYNC_H

// Functions to be called from SDL native code to synchronize EGL context access
void tuxpaint_egl_lock(void);
void tuxpaint_egl_unlock(void);

#endif /* TUXPAINT_EGL_SYNC_H */

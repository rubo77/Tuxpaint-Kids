package org.tuxpaint;

import java.io.IOException;

import org.libsdl.app.SDLActivity;

import android.content.Intent;
import android.os.Build;
import android.os.Bundle;
import android.util.Log;
import android.content.res.AssetManager;
import android.content.pm.PackageManager;
import android.Manifest;

public class tuxpaintActivity extends SDLActivity {
    private static final String TAG = "Tux Paint";
    private static AssetManager mgr;
    private static native boolean managertojni(AssetManager mgr);
    private static native void setnativelibdir(String path);

    // Lock object for OpenGL context synchronization
    private static final Object sGLLock = new Object();
    
    // Native methods for SDL EGL patch to fix EGL_BAD_ACCESS errors
    private static native void initSDLEGLPatch();
    private static native boolean isSDLEGLPatchInitialized();
    private static native void logCurrentThread(String tag);

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        Log.v(TAG, "onCreate()");

        // Initialize SDL EGL patch early to prevent EGL_BAD_ACCESS errors
        initSDLEGLContextManager();
        
        // Log the current thread ID for debugging
        logCurrentThread("onCreate");

        boolean requestPermissions = false;
        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.M && Build.VERSION.SDK_INT <= Build.VERSION_CODES.R) {
            if (this.checkSelfPermission(Manifest.permission.WRITE_EXTERNAL_STORAGE) != PackageManager.PERMISSION_GRANTED) {
                Intent intent = new Intent(this, reqpermsActivity.class);
		this.startActivity(intent);
            }
        }

        synchronized (sGLLock) {
            super.onCreate(savedInstanceState);
            mgr = getResources().getAssets();
            managertojni(mgr);
            setnativelibdir(getApplicationInfo().nativeLibraryDir + "/");
        }
    }
    
    // Synchronize OpenGL context access
    @Override
    public void onResume() {
        // Using Thread.currentThread().getName() instead of deprecated getId()
        Log.v(TAG, "onResume() - current thread: " + Thread.currentThread().getName());
        logCurrentThread("onResume");
        
        synchronized (sGLLock) {
            super.onResume();
        }
    }
    
    @Override
    public void onPause() {
        // Using Thread.currentThread().getName() instead of deprecated getId()
        Log.v(TAG, "onPause() - current thread: " + Thread.currentThread().getName());
        logCurrentThread("onPause");
        
        synchronized (sGLLock) {
            super.onPause();
        }
    }
    
    // Important: Add EGL synchronization to the main activity methods
    // to ensure OpenGL operations are properly synchronized between threads
    @Override
    public void onWindowFocusChanged(boolean hasFocus) {
        synchronized (sGLLock) {
            Log.v(TAG, "Synchronized onWindowFocusChanged: " + hasFocus);
            super.onWindowFocusChanged(hasFocus);
        }
    }
    
    @Override
    public void onLowMemory() {
        synchronized (sGLLock) {
            Log.v(TAG, "Synchronized onLowMemory");
            super.onLowMemory();
        }
    }
    
    /**
     * Initializes the SDL EGL patch to prevent EGL_BAD_ACCESS errors
     * This method sets up native code to synchronize EGL context access
     */
    private static void initSDLEGLContextManager() {
        Log.i(TAG, "Initializing SDL EGL Context Manager");
        // Initialize the patch in the native code
        initSDLEGLPatch();
        Log.i(TAG, "SDL EGL Patch initialized: " + isSDLEGLPatchInitialized());
    }

    static {
        try {
            System.loadLibrary("c++_shared");
            Log.i(TAG, "Loaded c++_shared");
        } catch (UnsatisfiedLinkError e) {
            Log.e(TAG, "Failed to load c++_shared: " + e.getMessage());
        }
        
        try {
            System.loadLibrary("tuxpaint_png");
            Log.i(TAG, "Loaded tuxpaint_png");
        } catch (UnsatisfiedLinkError e) {
            Log.e(TAG, "Failed to load tuxpaint_png: " + e.getMessage());
        }
        
        try {
            System.loadLibrary("tuxpaint_fribidi");
            Log.i(TAG, "Loaded tuxpaint_fribidi");
        } catch (UnsatisfiedLinkError e) {
            Log.e(TAG, "Failed to load tuxpaint_fribidi: " + e.getMessage());
        }
        
        try {
            System.loadLibrary("SDL2");
            Log.i(TAG, "Loaded SDL2");
        } catch (UnsatisfiedLinkError e) {
            Log.e(TAG, "Failed to load SDL2: " + e.getMessage());
        }
        
        try {
            System.loadLibrary("tp_android_assets_fopen");
            Log.i(TAG, "Loaded tp_android_assets_fopen");
        } catch (UnsatisfiedLinkError e) {
            Log.e(TAG, "Failed to load tp_android_assets_fopen: " + e.getMessage());
        }
        
        try {
            System.loadLibrary("tuxpaint_intl");
            Log.i(TAG, "Loaded tuxpaint_intl");
        } catch (UnsatisfiedLinkError e) {
            Log.e(TAG, "Failed to load tuxpaint_intl: " + e.getMessage());
        }
        
        try {
            System.loadLibrary("tuxpaint_iconv");
            Log.i(TAG, "Loaded tuxpaint_iconv");
        } catch (UnsatisfiedLinkError e) {
            Log.e(TAG, "Failed to load tuxpaint_iconv: " + e.getMessage());
        }
        
        // Load our SDL EGL patch native library
        try {
            System.loadLibrary("sdl_egl_patch");
            Log.i(TAG, "Loaded sdl_egl_patch");
        } catch (UnsatisfiedLinkError e) {
            Log.e(TAG, "Failed to load sdl_egl_patch: " + e.getMessage());
        }
        System.loadLibrary("tuxpaint_pixman");
        System.loadLibrary("tuxpaint_xml2");
        System.loadLibrary("tuxpaint_freetype");
        System.loadLibrary("tuxpaint_fontconfig");
        System.loadLibrary("tuxpaint_ffi");
        System.loadLibrary("tuxpaint_glib");
        System.loadLibrary("tuxpaint_cairo");
        System.loadLibrary("tuxpaint_harfbuzz_ng");
        System.loadLibrary("tuxpaint_pango");
        System.loadLibrary("tuxpaint_gdk_pixbuf");
        System.loadLibrary("tuxpaint_croco");
	//        System.loadLibrary("tuxpaint_rsvg");
        System.loadLibrary("SDL2_image");
        System.loadLibrary("SDL2_mixer");
        System.loadLibrary("SDL2_ttf");
        System.loadLibrary("SDL2_Pango");
	System.loadLibrary("SDL2_gfx");
        System.loadLibrary("tuxpaint");
        
        // Initialize the EGL context manager to prevent EGL_BAD_ACCESS errors
        initSDLEGLContextManager();
    }
}

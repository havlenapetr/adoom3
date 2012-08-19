/*
===========================================================================

Doom 3 GPL Source Code
Copyright (C) 1999-2011 id Software LLC, a ZeniMax Media company.
Copyright (C) 2012 Havlena Petr <havlenapetr@gmail.com>

This file is part of the Doom 3 GPL Source Code (?Doom 3 Source Code?).

Doom 3 Source Code is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Doom 3 Source Code is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with Doom 3 Source Code.  If not, see <http://www.gnu.org/licenses/>.

In addition, the Doom 3 Source Code is also subject to certain additional terms. You should have received a copy of these additional terms immediately following the terms and conditions of the GNU General Public License which accompanied the Doom 3 Source Code.  If not, please request a copy in writing from id Software at the address below.

If you have questions concerning this license or the applicable additional terms, you may contact in writing id Software LLC, c/o ZeniMax Media Inc., Suite 120, Rockville, Maryland 20850 USA.

===========================================================================
*/

#include <idlib/precompiled.h>
#include <renderer/tr_local.h>

#include <EGL/egl.h>

#include <android/native_activity.h>

#define LOG_METHOD \
    Sys_DebugPrintf("%s", __FUNCTION__);

#define LOG_METHOD_START \
    Sys_DebugPrintf("%s - start", __FUNCTION__);

#define LOG_METHOD_END \
    Sys_DebugPrintf("%s - end", __FUNCTION__);

static ANativeWindow*  window = NULL;
static EGLDisplay      display = EGL_NO_DISPLAY;
static EGLSurface      surface = EGL_NO_SURFACE;
static EGLContext      context = EGL_NO_CONTEXT;
static EGLConfig       config = NULL;

idCVar sys_videoRam("sys_videoRam", "0", CVAR_SYSTEM | CVAR_ARCHIVE | CVAR_INTEGER,
        "Texture memory on the video card (in megabytes) - 0: autodetect", 0, 512);

static
void GLimp_AndroidSetResolution(int32_t width, int32_t height) {
    cvarSystem->SetCVarBool("r_fullscreen",  true);
    cvarSystem->SetCVarInteger("r_mode", -1);

    cvarSystem->SetCVarInteger("r_customWidth", width);
    cvarSystem->SetCVarInteger("r_customHeight", height);

    float r = (float) width / (float) height;
    if (r > 1.7f) {
        cvarSystem->SetCVarInteger("r_aspectRatio", 1);    // 16:9
    } else if (r > 1.55f) {
        cvarSystem->SetCVarInteger("r_aspectRatio", 2);    // 16:10
    } else {
        cvarSystem->SetCVarInteger("r_aspectRatio", 0);    // 4:3
    }

    glConfig.vidWidth = width;
    glConfig.vidHeight = height;
}

void GLimp_AndroidInit(ANativeWindow* win) {
    assert(win);
    ANativeWindow_acquire(win);
    window = win;
}

void GLimp_AndroidQuit() {
    if(window) {
        ANativeWindow_release(window);
    }
}

void GLimp_WakeBackEnd(void *a) {
    LOG_METHOD
}

void GLimp_EnableLogging(bool log) { 
    LOG_METHOD
}

void GLimp_FrontEndSleep() {
    LOG_METHOD
}

void *GLimp_BackEndSleep() {
    LOG_METHOD
    return 0;
}

bool GLimp_SpawnRenderThread(void (*a)()) {
    LOG_METHOD
    return false;
}

void GLimp_SwapBuffers() {
    LOG_METHOD_START

    assert(display && surface);
    eglSwapBuffers(display, surface);

    LOG_METHOD_END
}

void GLimp_ActivateContext() {
    LOG_METHOD_START

    if(surface == EGL_NO_SURFACE && context == EGL_NO_CONTEXT) {
        Sys_Printf("Fetching EGL surface and context");

        surface = eglCreateWindowSurface(display, config, window, NULL);

        EGLint ctxAttrib[] = {
            EGL_CONTEXT_CLIENT_VERSION, 2,
            EGL_NONE
        };
        context = eglCreateContext(display, config, EGL_NO_CONTEXT, ctxAttrib);

        if (eglMakeCurrent(display, surface, surface, context) == EGL_FALSE) {
            Sys_Error("Unable to eglMakeCurrent");
            return;
        }
    }

    int32_t screenWidth, screenHeight;
    eglQuerySurface(display, surface, EGL_WIDTH, &screenWidth);
    eglQuerySurface(display, surface, EGL_HEIGHT, &screenHeight);
    GLimp_AndroidSetResolution(screenWidth, screenHeight);

    Sys_DebugPrintf("Surface parms: width(%i), height(%i)",
            screenWidth, screenHeight);

    LOG_METHOD_END
}

void GLimp_DeactivateContext() {
    LOG_METHOD_START

    if (display == EGL_NO_DISPLAY) {
        return;
    }

    if (context != EGL_NO_CONTEXT) {
        eglDestroyContext(display, context);
        context = EGL_NO_CONTEXT;
    }

    LOG_METHOD_END
}

static
bool GLimp_ChooseConfig(EGLDisplay* eglDisplay, EGLConfig* eglConfig, EGLint* eglNumConfig) {
    assert(eglDisplay && eglConfig && eglNumConfig);

    /*
     * Here specify the attributes of the desired configuration.
     * Below, we select an EGLConfig with at least 8 bits per color
     * component compatible with on-screen windows
     */
    EGLint attrib[] = {
        EGL_RED_SIZE,           8,    //  1,  2
        EGL_GREEN_SIZE,         8,    //  3,  4
        EGL_BLUE_SIZE,          8,    //  5,  6
        EGL_ALPHA_SIZE,         8,    //  7,  8
        EGL_DEPTH_SIZE,         24,   //  9, 10
        EGL_STENCIL_SIZE,       8,    // 11, 12
        EGL_BUFFER_SIZE,        24,   // 13, 14
        EGL_RENDERABLE_TYPE, EGL_OPENGL_ES2_BIT,    // 15, 16
        EGL_NONE,    // 17
    };
    // these match in the array
#define ATTR_RED_IDX            1
#define ATTR_GREEN_IDX          3
#define ATTR_BLUE_IDX           5
#define ATTR_ALPHA_IDX          7
#define ATTR_DEPTH_IDX          9
#define ATTR_STENCIL_IDX        11
#define ATTR_BUFFER_SIZE_IDX    13

    int colorbits, depthbits, stencilbits;
    int tcolorbits, tdepthbits, tstencilbits;
    // color, depth and stencil
    colorbits = 24;
    depthbits = 24;
    stencilbits = 8;

    for (int i = 0; i < 16; i++) {
        // 0 - default
        // 1 - minus colorbits
        // 2 - minus depthbits
        // 3 - minus stencil
        if ((i % 4) == 0 && i) {
            // one pass, reduce
            switch (i / 4) {
                case 2:
                    if (colorbits == 24)
                        colorbits = 16;
                    break;
                case 1:
                    if (depthbits == 24)
                        depthbits = 16;
                    else if (depthbits == 16)
                        depthbits = 8;
                case 3:
                    if (stencilbits == 24)
                        stencilbits = 16;
                    else if (stencilbits == 16)
                        stencilbits = 8;
            }
        }

        tcolorbits = colorbits;
        tdepthbits = depthbits;
        tstencilbits = stencilbits;

        if ((i % 4) == 3) {        // reduce colorbits
            if (tcolorbits == 24)
                tcolorbits = 16;
        }

        if ((i % 4) == 2) {        // reduce depthbits
            if (tdepthbits == 24)
                tdepthbits = 16;
            else if (tdepthbits == 16)
                tdepthbits = 8;
        }

        if ((i % 4) == 1) {        // reduce stencilbits
            if (tstencilbits == 24)
                tstencilbits = 16;
            else if (tstencilbits == 16)
                tstencilbits = 8;
            else
                tstencilbits = 0;
        }

        if (tcolorbits == 24) {
            attrib[ATTR_RED_IDX] = 8;
            attrib[ATTR_GREEN_IDX] = 8;
            attrib[ATTR_BLUE_IDX] = 8;
            attrib[ATTR_BUFFER_SIZE_IDX] = 24;
        } else {
            // must be 16 bit
            attrib[ATTR_RED_IDX] = 4;
            attrib[ATTR_GREEN_IDX] = 4;
            attrib[ATTR_BLUE_IDX] = 4;
            attrib[ATTR_BUFFER_SIZE_IDX] = 16;
        }

        attrib[ATTR_DEPTH_IDX] = tdepthbits;    // default to 24 depth
        attrib[ATTR_STENCIL_IDX] = tstencilbits;

        /* Here, the application chooses the configuration it desires. In this
         * sample, we have a very simplified selection process, where we pick
         * the first EGLConfig that matches our criteria */
        if (!eglChooseConfig(*eglDisplay, attrib, eglConfig, 1, eglNumConfig)) {
            Sys_Printf("Couldn't get a EGL config 0x%04x!", eglGetError());
            continue;
        }

        Sys_Printf("Using %d/%d/%d Color bits, %d Alpha bits, %d depth, %d stencil display.",
                   attrib[ATTR_RED_IDX], attrib[ATTR_GREEN_IDX],
                   attrib[ATTR_BLUE_IDX], attrib[ATTR_ALPHA_IDX],
                   attrib[ATTR_DEPTH_IDX],
                   attrib[ATTR_STENCIL_IDX]);

        glConfig.colorBits = tcolorbits;
        glConfig.depthBits = tdepthbits;
        glConfig.stencilBits = tstencilbits;
        return true;
    }

    return false;
}

bool GLimp_Init(glimpParms_t params) {
    LOG_METHOD_START

    if(!window) {
        Sys_Error("Can't init, because I haven't window!");
        return false;
    }

    Sys_Printf("GL parms: width(%i), height(%i)",
            params.width, params.height);

    display = eglGetDisplay(EGL_DEFAULT_DISPLAY);
    eglInitialize(display, 0, 0);

    EGLint eglNumConfig;
    if(!GLimp_ChooseConfig(&display, &config, &eglNumConfig)) {
        Sys_Error("Can't select EGL config!");
        return false;
    }

    /* EGL_NATIVE_VISUAL_ID is an attribute of the EGLConfig that is
     * guaranteed to be accepted by ANativeWindow_setBuffersGeometry().
     * As soon as we picked a EGLConfig, we can safely reconfigure the
     * ANativeWindow buffers to match, using EGL_NATIVE_VISUAL_ID. */
    int format;
    eglGetConfigAttrib(display, config, EGL_NATIVE_VISUAL_ID, &format);
    Sys_Printf("Using format(%i) for window", format);
    ANativeWindow_setBuffersGeometry(window, 0, 0, format);

    GLimp_ActivateContext();

    // print some gpu info
    Sys_Printf("GL_RENDERER: %s", glGetString(GL_RENDERER));
    Sys_Printf("GL_EXTENSIONS: %s", glGetString(GL_EXTENSIONS));

#if 0
    // draw something to show that GL is alive
    glClearColor( 0.5, 0.5, 0.7, 0 );
    glClear( GL_COLOR_BUFFER_BIT );
    GLimp_SwapBuffers();
#endif

    LOG_METHOD_END
    return true;
}

void GLimp_Shutdown() {
    LOG_METHOD_START

    GLimp_DeactivateContext();

    if (display == EGL_NO_DISPLAY) {
        return;
    }

    if (surface != EGL_NO_SURFACE) {
        eglDestroySurface(display, surface);
        surface = EGL_NO_SURFACE;
    }
    if(display != EGL_NO_DISPLAY) {
        eglTerminate(display);
        display = EGL_NO_DISPLAY;
    }

    LOG_METHOD_END
}

void GLimp_SetGamma(unsigned short red[256], unsigned short green[256], unsigned short blue[256]) {
    LOG_METHOD
}

bool GLimp_SetScreenParms(glimpParms_t parms) {
    LOG_METHOD
    return true;
}

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

#include <stdlib.h>
#include <stdio.h>
#include <assert.h>

#include <android/window.h>
#include <android_native_app_glue.h>

#if ID_PROFILING_ENABLED
#include <prof.h>
#endif

#define NO_SOUND 1

extern void GLimp_AndroidInit(ANativeWindow* win);
extern void GLimp_AndroidQuit();

extern void Posix_EarlyInit();
extern void Posix_LateInit();
extern bool Posix_AddMousePollEvent(int action, int value);
extern void Posix_QueEvent(sysEventType_t type, int value, int value2,
                    int ptrLength, void *ptr);

class idAndroidEngine {
public:
    idAndroidEngine()
        : mRunning(false),
          mExiting(false),
          mLastMotionX(0),
          mLastMotionY(0) {
    }

    void pause() {
        mRunning = false;
    }

    void resume() {
        mRunning = true;
    }

    void render() {
        if(isRunning()) {
            common->Frame();
        }
    }

    void start(ANativeWindow* win, bool debug = false) {
#if ID_PROFILING_ENABLED
        monstartup("libdoom.so");
#endif
        GLimp_AndroidInit(win);
        Posix_EarlyInit();
        common->Init(0, NULL, debug ?
                //"+devmap game/mp/d3dm1 +com_showFPS 1" : NULL);
                "+devmap testmaps/test_box +com_showFPS 1" : NULL);
        Posix_LateInit();
    }

    void stop() {
#if ID_PROFILING_ENABLED
        moncleanup();
#endif
        GLimp_AndroidQuit();
        common->Quit();
        mExiting = true;
    }

    bool isRunning() {
        return mRunning && common->IsInitialized();
    }

    bool isExiting() {
        return mExiting;
    }

    void setLastMotionPos(int32_t x, int32_t y) {
        mLastMotionX = x;
        mLastMotionY = y;
    }

    void getLastMotionPos(int32_t* x, int32_t* y) {
        *x = mLastMotionX;
        *y = mLastMotionY;
    }

private:
    bool mRunning;
    bool mExiting;
    int32_t mLastMotionX;
    int32_t mLastMotionY;
};

static
void handleCmd(struct android_app* app, int32_t cmd) {
    Sys_DebugPrintf("handleCmd start cmd(%i)", cmd);
    idAndroidEngine* engine = static_cast<idAndroidEngine*>(app->userData);

    switch (cmd) {
        case APP_CMD_SAVE_STATE:
            break;
        case APP_CMD_INIT_WINDOW:
            engine->start(app->window);
            break;
        case APP_CMD_TERM_WINDOW:
            engine->stop();
            break;
        case APP_CMD_RESUME:
        case APP_CMD_GAINED_FOCUS:
            engine->resume();
            break;
        case APP_CMD_PAUSE:
        case APP_CMD_LOST_FOCUS:
            engine->pause();
            break;
    }

    Sys_DebugPrintf("handleCmd end cmd(%i)", cmd);
}

static
int32_t handleInput(struct android_app* app, AInputEvent* event) {
    Sys_DebugPrintf("handleInput start");
    idAndroidEngine* engine = static_cast<idAndroidEngine*>(app->userData);

    switch(AInputEvent_getType(event)) {
        case AINPUT_EVENT_TYPE_MOTION:
            size_t count = AMotionEvent_getPointerCount(event);
            for(int i = 0; i < count; i++) {
                int32_t x = AMotionEvent_getX(event, i);
                int32_t y = AMotionEvent_getY(event, i);
                switch(AMotionEvent_getAction(event)) {
                    case AMOTION_EVENT_ACTION_DOWN:
                        Posix_QueEvent(SE_KEY, K_MOUSE1, true, 0, NULL);
                        if(!Posix_AddMousePollEvent(M_ACTION1, true)) {
                            return 0;
                        }
                        return 1;
                    case AMOTION_EVENT_ACTION_UP:
                        Posix_QueEvent(SE_KEY, K_MOUSE1, false, 0, NULL);
                        if(!Posix_AddMousePollEvent(M_ACTION1, false)) {
                            return 0;
                        }
                        return 1;
                   case AMOTION_EVENT_ACTION_MOVE:
                        int32_t lastX, lastY;
                        engine->getLastMotionPos(&lastX, &lastY);
                        int32_t dx = x - lastX;
                        int32_t dy = y - lastY;
                        Posix_QueEvent(SE_MOUSE, dx, dy, 0, NULL);
                        // if we overflow here, we'll get a warning,
                        // but the delta will be completely processed
                        Posix_AddMousePollEvent(M_DELTAX, dx);
                        if(!Posix_AddMousePollEvent(M_DELTAY, dy)) {
                            return 0;
                        }
                        engine->setLastMotionPos(x, y);
                        return 1;
                }
            }
            break;
    }

    Sys_DebugPrintf("handleInput stop");
    return 0;
}

/**
 * This is the main entry point of a native application that is using
 * android_native_app_glue.  It runs in its own thread, with its own
 * event loop for receiving input events and doing other things.
 */
void android_main(struct android_app* state) {
    assert(state);

    // Make sure glue isn't stripped.
    app_dummy();

    idAndroidEngine engine;
    state->userData = &engine;
    state->onAppCmd = handleCmd;
    state->onInputEvent = handleInput;
    ANativeActivity_setWindowFlags(state->activity,
            AWINDOW_FLAG_FULLSCREEN | AWINDOW_FLAG_KEEP_SCREEN_ON, 0);

#if 0
    cvarSystem->SetCVarBool("com_speeds",  true);
    cvarSystem->SetCVarBool("com_showMemoryUsage", true);
#endif
#ifdef NO_SOUND
    cvarSystem->SetCVarBool("s_noSound", true);
#else
    cvarSystem->SetCVarInteger("com_asyncSound", 0);
#endif

    while (!engine.isExiting()) {
        int ident;
        int events;
        struct android_poll_source* source;

        // If not animating, we will block forever waiting for events.
        // If animating, we loop until all events are read, then continue
        // to draw the next frame of animation.
        while ((ident=ALooper_pollAll(engine.isRunning() ? 0 : -1, NULL, &events,
                (void**)&source)) >= 0) {

            // Process this event.
            if (source != NULL) {
                source->process(state, source);
            }
        }

        engine.render();
    }
}

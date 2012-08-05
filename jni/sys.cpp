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

#include <android/log.h>

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>

//#define LOG_DEBUG 1
#define LOG_TAG "Doom"
#define LOGI(...)  __android_log_print(ANDROID_LOG_INFO,LOG_TAG,__VA_ARGS__)
#define LOGE(...)  __android_log_print(ANDROID_LOG_ERROR,LOG_TAG,__VA_ARGS__)
#ifdef LOG_DEBUG
#define LOGV(...) LOGI(__VA_ARGS__)
#else
#define LOGV(...)
#endif

#define FILL_BUFFER \
    char buffer[1024]; \
    va_list argptr; \
    va_start(argptr, fmt); \
    vsprintf(buffer, fmt, argptr); \
    va_end(argptr);

void Sys_DebugPrintf(const char *fmt, ...) {
#ifdef LOG_DEBUG
    FILL_BUFFER;
    LOGV(buffer);
#endif
}

void Sys_DebugVPrintf(const char *fmt, va_list arg) {
#ifdef LOG_DEBUG
    vprintf(fmt, arg);
#endif
}

void Sys_Printf(const char *fmt, ...) {
    FILL_BUFFER;
    LOGI(buffer);
}

void Sys_VPrintf(const char *msg, va_list arg) {
    vprintf(msg, arg);
}

void Sys_Error(const char *fmt, ...) {
    FILL_BUFFER;
    LOGE(buffer);

    exit(EXIT_FAILURE);
}

int Sys_GetVideoRam() {
    return 0;
}

const char *Sys_EXEPath(void) {
    return "/data/data/com.idsoftware.doom3/lib";
}

unsigned char Sys_MapCharForKey(int key) {
    return 0;
}

void Sys_GrabMouseCursor(bool grabit) {
}

void Sys_InitInput() {
}

void Sys_ShutdownInput() {
}

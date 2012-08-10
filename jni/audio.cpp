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
#include <sound/snd_local.h>
#include "openal_stub.h"

#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <unistd.h>

#include <SLES/OpenSLES.h>
#include <SLES/OpenSLES_Android.h>

#define CHECK_INIT(ok)  \
    if(!ok) {           \
        Sys_Printf("Can't init android audio driver[%i]", __LINE__); \
        return false;   \
    }

class AAudioBuffer {
public:
    AAudioBuffer(size_t size)
        : mData(malloc(size)),
          mSize(size) {
    }

    ~AAudioBuffer() {
        if(mData) {
            free(mData);
        }
        mSize = 0;
    }

    void* data() {
        return mData;
    }

    size_t size() {
        return mSize;
    }

private:
    void*       mData;
    size_t      mSize;
};

class idAudioHardwareAndroid : public idAudioHardware {
public:
    idAudioHardwareAndroid();
    virtual ~idAudioHardwareAndroid();

    virtual bool    Initialize(void);

    virtual bool    Lock(void **pDSLockedBuffer, ulong *dwDSLockedBufferSize) {
        return false;
    }

    virtual bool    Unlock(void *pDSLockedBuffer, dword dwDSLockedBufferSize) {
        return false;
    }

    virtual bool    GetCurrentPosition(ulong *pdwCurrentWriteCursor) {
        return false;
    }

    virtual bool    Flush() {
        // nothing here
        return true;
    }

    virtual void    Write(bool flushing);

    virtual int     GetNumberOfSpeakers() {
        return 2;
    }

    virtual int     GetMixBufferSize() {
        return MIXBUFFER_SAMPLES * 2/*m_channels*/ * GetNumberOfSpeakers();
    }

    virtual short*  GetMixBuffer() {
        if(!mInternalBuffer) {
            mInternalBuffer = new AAudioBuffer(GetMixBufferSize());
        }
        return (short *) mInternalBuffer->data();
    }

private:
    static void     PlayerCallback(SLAndroidSimpleBufferQueueItf bq, void *context);
#if 0
    AMutex                          mMutex;
    ACondition                      mCond;
#endif

    // engine interfaces
    SLObjectItf                     mEngine;
    SLEngineItf                     mIEngine;

    // output mix interface
    SLObjectItf                     mOutput;

    // buffer queue player interfaces
    SLObjectItf                     mPlayer;
    SLPlayItf                       mIPlayer;
    SLAndroidSimpleBufferQueueItf   mPlayerBuffer;

    AAudioBuffer*                   mInternalBuffer;
};

idAudioHardwareAndroid::idAudioHardwareAndroid()
    : mEngine(NULL),
      mIEngine(NULL),
      mOutput(NULL),
      mPlayer(NULL),
      mIPlayer(NULL),
      mPlayerBuffer(NULL),
      mInternalBuffer(NULL) {
}

idAudioHardwareAndroid::~idAudioHardwareAndroid() {
    if (mPlayer) {
        (*mPlayer)->Destroy(mPlayer);
        mPlayer = NULL;
        mIPlayer = NULL;
        mPlayerBuffer = NULL;
    }

    if (mOutput) {
        (*mOutput)->Destroy(mOutput);
        mOutput = NULL;
    }

    if (mEngine) {
        (*mEngine)->Destroy(mEngine);
        mEngine = NULL;
        mIEngine = NULL;
    }

    if(mInternalBuffer) {
        delete mInternalBuffer;
        mInternalBuffer = NULL;
    }
}

/* static */
void idAudioHardwareAndroid::PlayerCallback(SLAndroidSimpleBufferQueueItf bq, void *context) {
    Sys_Printf("%s", __FUNCTION__);
    assert(context);
    idAudioHardwareAndroid* driver = static_cast<idAudioHardwareAndroid*>(context);
    assert(driver);

    // enqueue another buffer
    SLresult result = (*driver->mPlayerBuffer)->Enqueue(driver->mPlayerBuffer,
            driver->mInternalBuffer->data(), driver->mInternalBuffer->size());

    // the most likely other result is SL_RESULT_BUFFER_INSUFFICIENT,
    // which for this code example would indicate a programming error
    assert(SL_RESULT_SUCCESS == result);
}

void idAudioHardwareAndroid::Write(bool flushing) {
    static bool alreadyEnqueued = false;

    assert(mInternalBuffer);
    if (alreadyEnqueued) {
        return;
	}

    // enqueue another buffer
    SLresult result = (*mPlayerBuffer)->Enqueue(mPlayerBuffer,
            mInternalBuffer->data(), mInternalBuffer->size());

    // the most likely other result is SL_RESULT_BUFFER_INSUFFICIENT,
    // which for this code example would indicate a programming error
    assert(SL_RESULT_SUCCESS == result);
    alreadyEnqueued = true;
}

bool idAudioHardwareAndroid::Initialize() {
#if 0
    CHECK_INIT(mMutex.isValid());
    CHECK_INIT(mCond.isValid());
#endif

    // create engine
    SLresult result = slCreateEngine(&mEngine, 0, NULL, 0, NULL, NULL);
    CHECK_INIT(SL_RESULT_SUCCESS == result);

    // realize the engine
    result = (*mEngine)->Realize(mEngine, SL_BOOLEAN_FALSE);
    CHECK_INIT(SL_RESULT_SUCCESS == result);

    // get the engine interface, which is needed in order to create other objects
    result = (*mEngine)->GetInterface(mEngine, SL_IID_ENGINE, &mIEngine);
    CHECK_INIT(SL_RESULT_SUCCESS == result);

    // create output mix
    const SLInterfaceID idsOutput[1] = {SL_IID_ENVIRONMENTALREVERB};
    const SLboolean reqOutput[1] = {SL_BOOLEAN_FALSE};
    result = (*mIEngine)->CreateOutputMix(mIEngine, &mOutput, 1, idsOutput, reqOutput);
    CHECK_INIT(SL_RESULT_SUCCESS == result);

    // realize the output mix
    result = (*mOutput)->Realize(mOutput, SL_BOOLEAN_FALSE);
    CHECK_INIT(SL_RESULT_SUCCESS == result);

    // configure audio source
    SLDataLocator_AndroidSimpleBufferQueue loc_bufq = {SL_DATALOCATOR_ANDROIDSIMPLEBUFFERQUEUE, 2};
    SLDataFormat_PCM format_pcm = {SL_DATAFORMAT_PCM, 1, SL_SAMPLINGRATE_8,
        SL_PCMSAMPLEFORMAT_FIXED_16, SL_PCMSAMPLEFORMAT_FIXED_16,
        SL_SPEAKER_FRONT_CENTER, SL_BYTEORDER_LITTLEENDIAN};
    SLDataSource audioSrc = {&loc_bufq, &format_pcm};

    // configure audio sink
    SLDataLocator_OutputMix loc_outmix = {SL_DATALOCATOR_OUTPUTMIX, mOutput};
    SLDataSink audioSnk = {&loc_outmix, NULL};

    // create audio player
    const SLInterfaceID idsPlayer[2] = {SL_IID_BUFFERQUEUE, SL_IID_EFFECTSEND};
    const SLboolean reqPlayer[2] = {SL_BOOLEAN_TRUE, SL_BOOLEAN_TRUE};
    result = (*mIEngine)->CreateAudioPlayer(mIEngine, &mPlayer, &audioSrc, &audioSnk,
            2, idsPlayer, reqPlayer);
    CHECK_INIT(SL_RESULT_SUCCESS == result);

    // realize the player
    result = (*mPlayer)->Realize(mPlayer, SL_BOOLEAN_FALSE);
    CHECK_INIT(SL_RESULT_SUCCESS == result);

    // get the play interface
    result = (*mPlayer)->GetInterface(mPlayer, SL_IID_PLAY, &mIPlayer);
    CHECK_INIT(SL_RESULT_SUCCESS == result);

    // get the buffer queue interface
    result = (*mPlayer)->GetInterface(mPlayer, SL_IID_BUFFERQUEUE, &mPlayerBuffer);
    CHECK_INIT(SL_RESULT_SUCCESS == result);

    // register callback on the buffer queue
    result = (*mPlayerBuffer)->RegisterCallback(mPlayerBuffer, PlayerCallback, this);
    CHECK_INIT(SL_RESULT_SUCCESS == result);

    // set the player's state to playing
    result = (*mIPlayer)->SetPlayState(mIPlayer, SL_PLAYSTATE_PLAYING);
    CHECK_INIT(SL_RESULT_SUCCESS == result);

    return true;
}

/*
===============
idAudioHardware::~idAudioHardware
===============
*/
idAudioHardware::~idAudioHardware() {
}

/* static */
idAudioHardware* idAudioHardware::Alloc() {
    return new idAudioHardwareAndroid();
}

bool Sys_LoadOpenAL() {
    return false;
}

void Sys_FreeOpenAL() {
    // nothing here
}

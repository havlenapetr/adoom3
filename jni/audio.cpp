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

    virtual bool    Flush();

    virtual void    Write(bool flushing);

    virtual int     GetNumberOfSpeakers() {
        return 2;
    }

    virtual int     GetMixBufferSize() {
        return MIXBUFFER_SAMPLES * mPcm.numChannels * GetNumberOfSpeakers();
    }

    virtual short*  GetMixBuffer() {
        Sys_DebugPrintf("GetMixBuffer - start");
        if(!mInternalBuffer) {
            mInternalBuffer = malloc(GetMixBufferSize());
        }
        Sys_DebugPrintf("GetMixBuffer - end");
        return (short *) mInternalBuffer;
    }

private:
    bool            WaitForQueue(unsigned int threshold);
    static int      GetSLSamplingRate(int rate);

    static const int                kQueueWaitTimeUs = 10000;

    // engine interfaces
    SLObjectItf                     mEngine;
    SLEngineItf                     mIEngine;

    // output mix interface
    SLObjectItf                     mOutput;
    SLDataLocator_OutputMix         mOutputMix;

    // buffer queue player interfaces
    SLObjectItf                     mPlayer;
    SLPlayItf                       mIPlayer;
    SLDataFormat_PCM                mPcm;
    SLBufferQueueItf                mBufferQueue;
    SLDataLocator_BufferQueue       mBufferQueueLoc;

    void*                           mInternalBuffer;
};

idAudioHardwareAndroid::idAudioHardwareAndroid()
    : mEngine(NULL),
      mIEngine(NULL),
      mOutput(NULL),
      mPlayer(NULL),
      mIPlayer(NULL),
      mBufferQueue(NULL),
      mInternalBuffer(NULL) {
}

idAudioHardwareAndroid::~idAudioHardwareAndroid() {
    if(mIPlayer) {
        (*mIPlayer)->SetPlayState(mIPlayer, SL_PLAYSTATE_STOPPED);
    }
    if (mPlayer) {
        (*mPlayer)->Destroy(mPlayer);
        mPlayer = NULL;
        mIPlayer = NULL;
        mBufferQueue = NULL;
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
        free(mInternalBuffer);
        mInternalBuffer = NULL;
    }
}

/* static */
int idAudioHardwareAndroid::GetSLSamplingRate(int rate) {
    switch (rate) {
		case 44100:
            return SL_SAMPLINGRATE_44_1;
	}
    return -1;
}

bool idAudioHardwareAndroid::Flush() {
    return WaitForQueue(0);
}

bool idAudioHardwareAndroid::WaitForQueue(unsigned int threshold) {
    Sys_DebugPrintf("Flush - start");
    assert(mBufferQueue);

    // Wait until the PCM data is done playing
    SLBufferQueueState state;
    do {
        if ((*mBufferQueue)->GetState(mBufferQueue, &state) != SL_RESULT_SUCCESS) {
            Sys_Printf("Can't obtain audio buffer queue state!");
            return false;
        }
        usleep(kQueueWaitTimeUs);
    } while (state.count > threshold);

    Sys_DebugPrintf("Flush - end");
    return true;
}

void idAudioHardwareAndroid::Write(bool flushing) {
    Sys_DebugPrintf("Write(%i) - start", flushing);
    assert(mBufferQueue);

    WaitForQueue(mBufferQueueLoc.numBuffers - 1);

    // enqueue another buffer
    SLresult result = (*mBufferQueue)->Enqueue(mBufferQueue,
            GetMixBuffer(), GetMixBufferSize());

    if (flushing) {
        Flush();
    }

end:
    Sys_DebugPrintf("Write - end");
}

bool idAudioHardwareAndroid::Initialize() {
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
    const SLInterfaceID idsOutput[1] = {SL_IID_VOLUME};
    const SLboolean reqOutput[1] = {SL_BOOLEAN_TRUE};
    result = (*mIEngine)->CreateOutputMix(mIEngine, &mOutput, 0, idsOutput, reqOutput);
    CHECK_INIT(SL_RESULT_SUCCESS == result);

    // realize the output mix
    result = (*mOutput)->Realize(mOutput, SL_BOOLEAN_FALSE);
    CHECK_INIT(SL_RESULT_SUCCESS == result);

    /* Setup the data source structure for the buffer queue */
    mBufferQueueLoc.locatorType = SL_DATALOCATOR_BUFFERQUEUE;
    mBufferQueueLoc.numBuffers = 4;  /* number of buffers in our buffer queue */

    /* Setup the format of the content in the buffer queue */
    mPcm.formatType = SL_DATAFORMAT_PCM;
    mPcm.numChannels = idSoundSystemLocal::s_numberOfSpeakers.GetInteger();
    mPcm.samplesPerSec = GetSLSamplingRate(PRIMARYFREQ);
    mPcm.bitsPerSample = SL_PCMSAMPLEFORMAT_FIXED_16;
    mPcm.containerSize = 16;
    mPcm.channelMask = SL_SPEAKER_FRONT_LEFT | SL_SPEAKER_FRONT_RIGHT;
    mPcm.endianness = SL_BYTEORDER_LITTLEENDIAN;

    /* Setup the data sink structure */
    mOutputMix.locatorType = SL_DATALOCATOR_OUTPUTMIX;
    mOutputMix.outputMix = mOutput;

    SLDataSource audioSource;
    audioSource.pFormat = (void *)&mPcm;
    audioSource.pLocator = (void *)&mBufferQueueLoc;

    SLDataSink audioSink;
    audioSink.pLocator = (void *)&mOutputMix;
    audioSink.pFormat = NULL;

    /* Set arrays required[] and iidArray[] for SEEK interface
     (PlayItf is implicit) */
    const SLInterfaceID iidPlayer[1] = {SL_IID_BUFFERQUEUE};
    const SLboolean reqPlayer[1] = {SL_BOOLEAN_TRUE};

    /* Create the music player */
    result = (*mIEngine)->CreateAudioPlayer(mIEngine, &mPlayer,
            &audioSource, &audioSink, 1, iidPlayer, reqPlayer);
    CHECK_INIT(SL_RESULT_SUCCESS == result);

    // realize the player
    result = (*mPlayer)->Realize(mPlayer, SL_BOOLEAN_FALSE);
    CHECK_INIT(SL_RESULT_SUCCESS == result);

    // get the play interface
    result = (*mPlayer)->GetInterface(mPlayer, SL_IID_PLAY, &mIPlayer);
    CHECK_INIT(SL_RESULT_SUCCESS == result);

    // get the buffer queue interface
    result = (*mPlayer)->GetInterface(mPlayer, SL_IID_BUFFERQUEUE, &mBufferQueue);
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

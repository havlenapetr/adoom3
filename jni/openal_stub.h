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

#ifndef _OPEN_AL_STUB_H_
#define _OPEN_AL_STUB_H_

ALenum(ALAPIENTRY *idalGetError)() = NULL;
ALvoid(ALAPIENTRY *idalGenBuffers)(ALsizei, ALuint *) = NULL;
ALboolean(ALAPIENTRY *idalIsSource)(ALuint) = NULL;
ALvoid(ALAPIENTRY *idalSourceStop)(ALuint) = NULL;
ALvoid(ALAPIENTRY *idalGetSourcei)(ALuint, ALenum, ALint *) = NULL;
ALint(ALAPIENTRY *idalGetInteger)(ALenum) = NULL;
ALCvoid(ALAPIENTRY *idalcSuspendContext)(ALCcontext *) = NULL;
ALCboolean(ALAPIENTRY *idalcMakeContextCurrent)(ALCcontext *) = NULL;
ALCvoid(ALAPIENTRY *idalcProcessContext)(ALCcontext *) = NULL;
ALCvoid(ALAPIENTRY *idalcDestroyContext)(ALCcontext *) = NULL;
ALCubyte *(ALAPIENTRY *idalcGetString)(ALCdevice *, ALCenum) = NULL;
ALvoid(ALAPIENTRY *idalBufferData)(ALuint, ALenum, ALvoid *, ALsizei, ALsizei) = NULL;
ALvoid(ALAPIENTRY *idalDeleteBuffers)(ALsizei, ALuint *) = NULL;
ALboolean(ALAPIENTRY *idalIsExtensionPresent)(ALubyte *) = NULL;
ALvoid(ALAPIENTRY *idalDeleteSources)(ALsizei, ALuint *) = NULL;
ALenum(ALAPIENTRY *idalGetEnumValue)(ALubyte *) = NULL;
ALvoid *(ALAPIENTRY *idalGetProcAddress)(ALubyte *) = NULL;
ALCcontext *(ALAPIENTRY *idalcCreateContext)(ALCdevice *, ALCint *) = NULL;
ALCdevice *(ALAPIENTRY *idalcOpenDevice)(ALubyte *) = NULL;
ALvoid(ALAPIENTRY *idalListenerfv)(ALenum, ALfloat *) = NULL;
ALvoid(ALAPIENTRY *idalSourceQueueBuffers)(ALuint, ALsizei, ALuint *) = NULL;
ALvoid(ALAPIENTRY *idalSourcei)(ALuint, ALenum, ALint) = NULL;
ALvoid(ALAPIENTRY *idalListenerf)(ALenum, ALfloat) = NULL;
ALCvoid(ALAPIENTRY *idalcCloseDevice)(ALCdevice *) = NULL;
ALboolean(ALAPIENTRY *idalIsBuffer)(ALuint) = NULL;
ALvoid(ALAPIENTRY *idalSource3f)(ALuint, ALenum, ALfloat, ALfloat, ALfloat) = NULL;
ALvoid(ALAPIENTRY *idalGenSources)(ALsizei, ALuint *) = NULL;
ALvoid(ALAPIENTRY *idalSourcef)(ALuint, ALenum, ALfloat) = NULL;
ALvoid(ALAPIENTRY *idalSourceUnqueueBuffers)(ALuint, ALsizei, ALuint *) = NULL;
ALvoid(ALAPIENTRY *idalSourcePlay)(ALuint) = NULL;

#endif

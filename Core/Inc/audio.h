/*
Copyright 2023 Piotr Wilkon
This file is part of QAM Modulator Controller.

QAM Modulator Controller is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 3 of the License, or
(at your option) any later version.

QAM Modulator Controller is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with QAM Modulator Controller.  If not, see <http://www.gnu.org/licenses/>.
*/


#ifndef INC_AUDIO_H_
#define INC_AUDIO_H_

#include <stdint.h>

//enum AudioSampleRate
//{
//	AUDIO_44100,
//	AUDIO_22050,
//	AUDIO_11025,
//};

void AudioInit(void);

void AudioStart(void (*sampleCallback)(int16_t*, uint32_t));

void AudioStop(void);

#endif /* INC_AUDIO_H_ */

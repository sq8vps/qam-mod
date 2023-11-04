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

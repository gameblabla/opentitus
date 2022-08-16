#ifndef OPL_CONFIG
#define OPL_CONFIG

#ifdef DREAMCAST
#define STEREO_SOUND 1
#define SOUND_FREQUENCY 22050
#define SOUND_SAMPLES 512
#else
#define STEREO_SOUND 1
#define SOUND_FREQUENCY 8000
#define SOUND_SAMPLES 512
#endif

#ifdef STEREO_SOUND
#define SOUND_MULTIPLY 2
#else
#define SOUND_MULTIPLY 1
#endif

#endif

// Filename: audio_load_wav.cxx
// Created by:  cary (23Sep00)
// 
////////////////////////////////////////////////////////////////////

#include <dconfig.h>
#include "audio_pool.h"
#include "config_audio.h"

Configure(audio_load_wav);

#include "audio_trait.h"

#ifdef AUDIO_USE_MIKMOD

#include "audio_mikmod_traits.h"

void AudioDestroyWav(AudioTraits::SampleClass* sample) {
  MikModSample::destroy(sample);
}

void AudioLoadWav(AudioTraits::SampleClass** sample,
		  AudioTraits::PlayingClass** state,
		  AudioTraits::PlayerClass** player,
		  AudioTraits::DeleteSampleFunc** destroy, Filename filename) {
  *sample = MikModSample::load_wav(filename);
  if (*sample == (AudioTraits::SampleClass*)0L)
    return;
  *state = ((MikModSample*)(*sample))->get_state();
  *player = MikModSamplePlayer::get_instance();
  *destroy = AudioDestroyWav;
}

#elif defined(AUDIO_USE_WIN32)

#include "audio_win_traits.h"

void EXPCL_MISC AudioDestroyWav(AudioTraits::SampleClass* sample) {
  WinSample::destroy(sample);
}

void AudioLoadWav(AudioTraits::SampleClass** sample,
		  AudioTraits::PlayingClass** state,
		  AudioTraits::PlayerClass** player,
		  AudioTraits::DeleteSampleFunc** destroy, Filename filename) {
  *sample = WinSample::load_wav(filename);
  if (*sample == (AudioTraits::SampleClass*)0L)
    return;
  *state = ((WinSample*)(*sample))->get_state();
  *player = WinPlayer::get_instance();
  *destroy = AudioDestroyWav;
}

#elif defined(AUDIO_USE_LINUX)

#include "audio_linux_traits.h"

void AudioDestroyWav(AudioTraits::SampleClass* sample) {
  LinuxSample::destroy(sample);
}

void AudioLoadWav(AudioTraits::SampleClass** sample,
		  AudioTraits::PlayingClass** state,
		  AudioTraits::PlayerClass** player,
		  AudioTraits::DeleteSampleFunc** destroy, Filename) {
  audio_cat->error() << "Linux driver does not natively support WAV."
		     << "  Try the 'st' loader." << endl;
  *sample = (AudioTraits::SampleClass*)0L;
  *state = (AudioTraits::PlayingClass*)0L;
  *player = (AudioTraits::PlayerClass*)0L;
  *destroy = AudioDestroyWav;
}

#elif defined(AUDIO_USE_NULL)

// Null driver
#include "audio_null_traits.h"

void AudioDestroyWav(AudioTraits::SampleClass* sample) {
  delete sample;
}

void AudioLoadWav(AudioTraits::SampleClass** sample,
		  AudioTraits::PlayingClass** state,
		  AudioTraits::PlayerClass** player,
		  AudioTraits::DeleteSampleFunc** destroy, Filename) {
  *sample = new NullSample();
  *state = new NullPlaying();
  *player = new NullPlayer();
  *destroy = AudioDestroyWav;
}

#else /* AUDIO_USE_NULL */

#error "unknown implementation driver"

#endif /* AUDIO_USE_NULL */

ConfigureFn(audio_load_wav) {
  AudioPool::register_sample_loader("wav", AudioLoadWav);
}

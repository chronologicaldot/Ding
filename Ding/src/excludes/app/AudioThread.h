// (c) 2019 Nicolaus Anderson

#ifndef AUDIO_THREAD_CLASS_H
#define AUDIO_THREAD_CLASS_H

#include "../soundbox/SoundBox.h"
#include <thread>
#include <mutex>
#include <atomic>

//! Audio Thread
struct AudioThread
	, public soundbox::SoundBox::Listener
{

	typedef  TSafeBuffer<soundbox::SoundBox::Sample_t>  AudioBuffer_t;

	AudioThread();
	~AudioThread();

	// Thread ----------
	void  start();
	void  stop();

	// Stream ----------
	void  openStream();
	void  closeStream();

	// Playback --------
	void  playAudio();
	void  stopAudio();

	bool  isRunning();		// Thread
	bool  isStreamOpen();	// Stream
	bool  isPlayStarted();	// Playback
	//bool  isAudioPlaying();	// SoundBox::isRunning()

	// Audio Access -------
	// Note: Bad things can happen if releaseAudio() is not called.
	// WARNING: DO NOT LOAD AUDIO VIA THIS INTERFACE
	//soundbox::SoundBox&  accessAudio();
	//void  releaseAudio();

	// Load audio buffer here
	AudioBuffer_t&  accessSharedBuffer();

	// SoundBox Direct Callbacks -----------------
	virtual void OnSoundBoxChannelEmpty( soundbox::SoundBox::u8 ) override;

	// Internal Thread Usage ONLY ----------------
	static void  loop( AudioThread& );

private:
	soundbox::SoundBox  AudioPlayer;
	AudioBuffer_t  SharedBuffer;
	std::atomic<bool>  RunSignal;		// Run Thread
	std::atomic<bool>  StreamSignal;	// Open/Close Stream
	std::atomic<bool>  StartSignal;		// Start/Stop Stream
	//std::mutex  AudioMutex;
	std::mutex  RunMutex;
	std::mutex  StreamMutex;
	std::mutex  StartMutex;
	std::thread  Thread;
};

#endif

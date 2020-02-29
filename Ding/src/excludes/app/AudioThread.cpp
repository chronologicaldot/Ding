// (c) 2019 Nicolaus Anderson

#include "AudioThread.h"
#include <lock_guard>
#include <unique_lock>
#include <chrono>

AudioThread::AudioThread()
	: AudioPlayer()
	, std::atomic<bool>  RunSignal(false)
	, std::atomic<bool>  StreamSignal(false)
	, std::atomic<bool>  StartSignal(false)
{
	AudioPlayer.setListener(this);
}

AudioThread::~AudioThread()
{
	Thread.join();
}

void
AudioThread::start() {
	RunSignal = true;
	Thread( &AudioThread::loop, std::ref(*this) );
	Thread.detach();
}

void
AudioThread::stop() {
	std::lock_guard<std::mutex>  guard(RunMutex);
	RunSignal = false;
}

void
AudioThread::openStream() {
	soundbox::SoundBox*  s = accessAudio();
	s->open();
	releaseAudio();
}

void
AudioThread::closeStream() {
	soundbox::SoundBox*  s = accessAudio();
	s->close();
	releaseAudio();
}

void
AudioThread::playAudio() {
	std::lock_guard<std::mutex>  guard(StartMutex);
	StartSignal = true;
}

void
AudioThread::pauseAudio() {
	std::lock_guard<std::mutex>  guard(StartMutex);
	StartSignal = false;
}

bool
AudioThread::isRunning() {
	std::lock_guard<std::mutex>  guard(RunMutex);
	const bool out = RunSignal;
	return out;
}

bool
AudioThread::isStreamOpen() {
	std::lock_guard<std::mutex>  guard(StreamMutex);
	const bool out = StreamSignal;
	return out;
}

bool
AudioThread::isPlayStarted() {
	std::lock_guard<std::mutex>  guard(StartMutex);
	const bool out = StartSignal;
	return out && isStreamOpen();
}

// WON'T WORK. AudioPlayer is tied up in the playback loop.
//bool
//AudioThread::isAudioPlaying() {
//	std::lock_guard(AudioMutex);
//	const bool out = AudioPlayer.isRunning();
//	return out;
//}

/*
soundbox::SoundBox&
AudioThread::accessAudio() {
	AudioMutex.lock();
	return  AudioPlayer;
}

void
AudioThread::releaseAudio() {
	AudioMutex.unlock();
}
*/

AudioBuffer_t&
AudioThread::accessSharedBuffer() {
	return  SharedBuffer;
}

void
AudioThread::OnSoundBoxChannelEmpty( soundbox::SoundBox::u8  channel ) {
	// TODO
	/*
		For this to work, there needs to be some kind of array that saves all of the channels.
		A mutex is needed to access the array because the value of the array may change
			as it's being previewed.

		NOTE: In case you've forgotten, the reason you need this is to enabled/disable
			the audio settings.
	*/
}

/*
void
AudioThread::loop( AudioThread&  audioThread ) {
	soundbox::SoundBox*  audio;

	audio = & audioThread.accessAudio();
	audio->startStream();
	audioThread.releaseAudio();

	do {
		if ( audioThread.isStarted() ) {
			audio = & audioThread.accessAudio();
			audio->start();

			std::this_thread::sleep_for( std::chrono::milliseconds(100) );

			audio = nullptr;
			audioThread.releaseAudio();
		}
		else {
			audio = & audioThread.accessAudio();
			audio->stop();
			audioThread.releaseAudio();
		}

		// Don't bottleneck processing with frequent checks
		std::this_thread::sleep_for( std::chrono::milliseconds(100) );

	} while ( audioThread.isRunning() );

	audioThread.stop();
}
*/

void
AudioThread::loop( AudioThread&  audioThread ) {

	do {
		if ( audioThread.isPlayStarted() ) {

			std::this_thread::sleep_for( std::chrono::milliseconds(100) );

		} else {

		}
		// Don't bottleneck processing with frequent checks
		std::this_thread::sleep_for( std::chrono::milliseconds(100) );

	} while ( audioThread.isRunning() );

	audioThread.stop();
}

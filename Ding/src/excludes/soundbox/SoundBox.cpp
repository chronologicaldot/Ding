// (c) 2019 Nicolaus Anderson

#include "SoundBox.h"

namespace soundbox {

SoundBox::SoundBox()
	: ready(false)
	, rtaudio()
	, apiMap()
	, currentDeviceInfo()
	, bufferFramesPerCycle(512)
	, sampleSpeed(44100)
	, outputBufferType(RTAUDIO_SINT32)
	, useInterleaved(true)
	, hogSoundDevice(false)
	, numChannels(0)
	, outputStreamName("soundbox default output")
{
	rtaudioAPIMap[RtAudio::MACOSX_CORE] = "OS-X Core Audio";
	rtaudioAPIMap[RtAudio::WINDOWS_ASIO] = "Windows ASIO";
	rtaudioAPIMap[RtAudio::WINDOWS_DS] = "Windows Direct Sound";
	rtaudioAPIMap[RtAudio::WINDOWS_WASAPI] = "Windows WASAPI";
	rtaudioAPIMap[RtAudio::UNIX_JACK] = "Jack Client";
	rtaudioAPIMap[RtAudio::LINUX_ALSA] = "Linux ALSA";
	rtaudioAPIMap[RtAudio::LINUX_PULSE] = "Linux PulseAudio";
	rtaudioAPIMap[RtAudio::LINUX_OSS] = "Linux OSS";
	rtaudioAPIMap[RtAudio::RTAUDIO_DUMMY] = "RtAudio Dummy";

	RtAudio::getCompiledApi( rtaudioAPIs );

	setOutputParameters(); // set defaults
}

SoundBox::~SoundBox()
{
	if ( rtaudio.isStreamOpen() )
		rtaudio.closeStream();

	// Buffers currently automatically cleared because they are std::vectors
}

void
SoundBox::setOutputStreamName( const std::string  name ) {
	outputStreamName = name;
}

void
SoundBox::setOutputParameters(
	PlaybackApi  audioAPIChoice,
	uint  bufferFrames,
	uint  sampleRate,
	bool  interleaveChannels,
	bool  hogAudioDevice,
	bool  scheduleRealtime,
	int  osSchedulingPriority,
	s8  numberOfChannels,
) {
	// Support for more than to channels is not yet available
	if ( numberOfChannels > 2 )
		numberOfChannels = 2;

	bufferFramesPerCycle = bufferFrames;
	sampleSpeed = sampleRate;

	outputParameters.nChannels = numberOfChannels;
	outputParameters.firstChannel = 0;

	outputOptions.flags |= (interleaveChannels ? 0 : RTAUDIO_NONINTERLEAVED);
	outputOptions.flags |= (hogAudioDevice ? RTAUDIO_HOG_DEVICE : 0);
	outputOptions.flags |= (realtimeScheduling ? RTAUDIO_SCHEDULE_REALTIME : 0);
	outputOptions.flags |= (audioAPIChoice == RtAudio::UNIX_JACK ? RTAUDIO_MINIMIZE_LATENCY : 0);
	outputOptions.numberOfBuffers = numberOfChannels;
	outputOptions.streamName = outputStreamName;
	outputOptions.priority = osSchedulingPriority;
}


bool
SoundBox::init() {
	if ( rtaudio.getDeviceCount() == 0 )
		return false;

	if ( rtaudio.isStreamOpen() )
		rtaudio.closeStream();

	outputParameters.deviceId = rtaudio.getDefaultOutputDevice();

	try {
		rtaudio.openStream( &outputParameters, NULL, 
			// TODO
		);

	} catch ( RtAudioError& e ) {
		// TODO: Set failbit error?
		return false;
	}

	return true;
}

std::string
SoundBox::getCurrentAPIAsString() {
	return rtaudioAPIMap[ rtaudio.getCurrentApi() ];
}

void
SoundBox::loadChannelBuffer( s8  channel, SoundByte_t*  buffer, size_t  bufferSize ) {
	bufferedSamplesLeft.reserve(bufferSize);

	SampleBuffer*  channelBuffer = &bufferedSamplesLeft;
	if ( channel == 1 )
		channelBuffer = &bufferedSamplesRight;

	size_t  s = 0;
	for(; s < bufferSize; ++s)
		(*channelBuffer)[s] = buffer[s];
}

SampleBuffer&
SoundBox::getChannelBuffer( s8 channel ) {
	switch( channel ) {
	case 1:
		return bufferedSamplesRight;

	default: // 0
		return bufferedSamplesLeft;
	}
}

int
SoundBox::handleAudioPlay( void* outputBuffer, void* inputBuffer, uint numBufferFrames,
					double streamTime, RtAudioStreamStatus status )
{
	if ( status ) // Underflow or Overflow when status > 0
		return 1;

	if ( outputOptions.flags & RTAUDIO_NONINTERLEAVED > 0 ) {
		switch ( outputBufferType )
		{
		case RTAUDIO_SINT8:
			return writeToS8Buffer( (s8*)outputBuffer, numBufferFrames, streamTime );

		case RTAUDIO_SINT16:
			return writeToS16Buffer( (s16*)outputBuffer, numBufferFrames, streamTime );

		case RTAUDIO_SINT24:
			return 1;

		case RTAUDIO_SINT32:
			return writeToS32Buffer( (s32*)outputBuffer, numBufferFrames, streamTime );

		case RTAUDIO_FLOAT32:
			return writeToF32Buffer( (f32*)outputBuffer, numBufferFrames, streamTime );

		case RTAUDIO_FLOAT64:
			return writeToF64Buffer( (f64*)outputBuffer, numBufferFrames, streamTime );

		default:
			return 1;
		}
	} else {
		switch ( outputBufferType )
		{
		case RTAUDIO_SINT8:
			return writeToS8BufferInterleaved( (s8*)outputBuffer, numBufferFrames, streamTime );

		case RTAUDIO_SINT16:
			return writeToS16BufferInterleaved( (s16*)outputBuffer, numBufferFrames, streamTime );

		case RTAUDIO_SINT24:
			return 1;

		case RTAUDIO_SINT32:
			return writeToS32BufferInterleaved( (s32*)outputBuffer, numBufferFrames, streamTime );

		case RTAUDIO_FLOAT32:
			return writeToF32BufferInterleaved( (f32*)outputBuffer, numBufferFrames, streamTime );

		case RTAUDIO_FLOAT64:
			return writeToF64BufferInterleaved( (f64*)outputBuffer, numBufferFrames, streamTime );

		default:
			return 1;
		}
	}
}

// Separate function but kept here because of association -----------------
int SoundBox_RtAudioCallback( void* outputBuffer, void* inputBuffer, uint numBufferFrames,
					double streamTime, RtAudioStreamStatus status, void* data )
{
	SoundBox*  soundBox = reinterpret_cast<SoundBox*>(data);
	return soundBox->handleAudioPlay( outputBuffer, inputBuffer, numBufferFrames, streamTime, status );
}

//-----------------------------------------------
int
SoundBox::writeToS8Buffer( s8*  buffer, uint  numBufferFrames, double streamTime ) {
	uint  s = 0;
	s8  c = 0;
	for (; c < numChannels; ++c) {
		for (; s < numBufferFrames )
			buffer[s] = SCALE_8bit * getSample(c, s, streamTime) / SOUND_BYTE_SCALE;
	}
	return 0;
}

int
SoundBox::writeToS16Buffer( s16*  buffer, uint  numBufferFrames, double  streamTime ) {
	uint  s = 0;
	s8  c = 0;
	for (; c < numChannels; ++c) {
		for (; s < numBufferFrames )
			buffer[s] = SCALE_16bit * getSample(c, s, streamTime) / SOUND_BYTE_SCALE;
	}
	return 0;
}

int
SoundBox::writeToS32Buffer( s32*  buffer, uint  numBufferFrames, double  streamTime ) {
	uint  s = 0;
	s8  c = 0;
	for (; c < numChannels; ++c) {
		for (s = 0; s < numBufferFrames )
			buffer[s] = SCALE_32bit * getSample(c, s, streamTime) / SOUND_BYTE_SCALE;
	}
	return 0;
}

int
SoundBox::writeToF32Buffer( f32*  buffer, uint  numBufferFrames, double  streamTime ) {
	uint  s = 0;
	s8  c = 0;
	for (; c < numChannels; ++c) {
		for (s = 0; s < numBufferFrames )
			buffer[s] = getSample(c, s, streamTime) / SOUND_BYTE_SCALE;
	}
	return 0;
}

int
SoundBox::writeToF64Buffer( f64*  buffer, uint  numBufferFrames, double  streamTime ) {
	uint  s = 0;
	s8  c = 0;
	for (; c < numChannels; ++c) {
		for (s = 0; s < numBufferFrames )
			buffer[s] = getSample(c, s, streamTime) / SOUND_BYTE_SCALE;
	}
	return 0;
}

int
SoundBox::writeToS8BufferInterleaved( s8*  buffer, uint  numBufferFrames, double streamTime ) {
	uint  s = 0;
	s8  c = 0;
	for (; s < numBufferFrames; ++s) {
		for (c = 0; c < numChannels; ++c)
			buffer[s] = SCALE_8bit * getSample(c, s, streamTime) / SOUND_BYTE_SCALE;
	}
	return 0;
}

int
SoundBox::writeToS16BufferInterleaved( s16*  buffer, uint  numBufferFrames, double  streamTime ) {
	uint  s = 0;
	s8  c = 0;
	for (; s < numBufferFrames; ++s) {
		for (c = 0; c < numChannels; ++c)
			buffer[s] = SCALE_16bit * getSample(c, s, streamTime) / SOUND_BYTE_SCALE;
	}
	return 0;
}

int
SoundBox::writeToS32BufferInterleaved( s32*  buffer, uint  numBufferFrames, double  streamTime ) {
	uint  s = 0;
	s8  c = 0;
	for (; s < numBufferFrames; ++s) {
		for (c = 0; c < numChannels; ++c)
			buffer[s] = SCALE_32bit * getSample(c, s, streamTime) / SOUND_BYTE_SCALE;
	}
	return 0;
}

int
SoundBox::writeToF32BufferInterleaved( f32*  buffer, uint  numBufferFrames, double  streamTime ) {
	uint  s = 0;
	s8  c = 0;
	for (; s < numBufferFrames; ++s) {
		for (c = 0; c < numChannels; ++c)
			buffer[s] = getSample(c, s, streamTime) / SOUND_BYTE_SCALE;
	}
	return 0;
}

int
SoundBox::writeToF64BufferInterleaved( f64*  buffer, uint  numBufferFrames, double  streamTime ) {
	uint  s = 0;
	s8  c = 0;
	for (; s < numBufferFrames; ++s) {
		for (c = 0; c < numChannels; ++c)
			buffer[s] = getSample(c, s, streamTime) / SOUND_BYTE_SCALE;
	}
	return 0;
}

SoundByte_t
SoundBox::getSample( s8  channel, uint  sampleIndex, double  streamTime ) {
	size_t  numBufferedSamples = 0;
	SampleBuffer*  buffer = &bufferedSamplesLeft;
	if ( channel == 1 ) {
		numBufferedSamples = bufferedSamplesRight.size();
		buffer = &bufferedSamplesRight;
	} else {
		numBufferedSamples = bufferedSamplesLeft.size();
	}

	if ( sampleIndex >= numBufferedSamples ) {
		/*if ( loopBuffer ) {
			sampleIndex = sampleIndex % numBufferedSamples;
		} else {*/
			return 0;
		//}
	}
	return (*buffer)[sampleIndex];
}

}

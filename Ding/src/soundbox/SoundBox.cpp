// (c) 2019 Nicolaus Anderson

#include "SoundBox.h"
#include <cstdio> // for std::puts() and std::printf()

namespace soundbox {

SampleBuffer::SampleBuffer()
	: buffer()
	, usedSamples(0)
	, looping(false)
{}

void
SampleBuffer::copyTo( SampleBuffer&  other ) {
	other.clear();
	other.reserve(buffer.size());
	for ( const SoundByte_t  s : buffer ) {
		other.push_back( s );
	}
}

void
SampleBuffer::clear() {
	buffer.clear();
	usedSamples = 0;
}

void
SampleBuffer::reserve( size_t  amount ) {
	buffer.reserve(amount);
}

void
SampleBuffer::push_back( const SoundByte_t  b ) {
	buffer.push_back(b);
}

SoundByte_t
SampleBuffer::getSample() {
	if ( usedSamples >= buffer.size() ) {
		if ( !looping )
			return 0;
		usedSamples = 0;
	}

	return buffer.at(usedSamples++);
}

void
SampleBuffer::setLooping( bool  setting ) {
	looping = setting;
	if ( looping && usedSamples == buffer.size() )
		usedSamples = 0;
}

bool
SampleBuffer::finished() {
	if ( looping )
		return false;

	return usedSamples >= buffer.size();
}

SoundBox::SoundBox()
	: ready(false)
	, rtaudio()
	, rtaudioAPIMap()
	, currentDeviceInfo()
	, bufferFramesPerCycle(512)
	, sampleRate(44100)
	, outputStreamName("soundbox default output")
	, errorType(RtAudioError::UNSPECIFIED)
	, bufferedSamplesLeft()
	, bufferedSamplesRight()
	, listener(nullptr)
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

bool
SoundBox::getDeviceCount() {
	return rtaudio.getDeviceCount();
}

bool
SoundBox::isAPISupported( PlaybackApi  api ) {
	size_t  a = 0;
	for (; a < rtaudioAPIs.size(); ++a) {
		if ( rtaudioAPIs[a] == api )
			return true;
	}
	return false;

	// TODO: Move Supported Devices code to its own function
/*
	RtAudio::DeviceInfo  info;
	unsigned  deviceCount = rtaudio.getDeviceCount();
	unsigned  d=0;

	if ( deviceCount == 0 )
		std::printf("NO AUDIO DEVICES!");

	for (; d < deviceCount; ++d) {
		info = rtaudio.getDeviceInfo(d);
		std::printf("Scanning device: %s\n", info.name.c_str());
		if ( rtaudioAPIMap[api] == info.name ) {
			return info.probed;
		}
	}
	return false;
*/
}

bool
SoundBox::isAudioFormatSupported( AudioFormat  /*format*/ ) {
	// RtAudio will automatically perform conversions.
	return true;

	// API and Device and not the same.
	// TODO: What do I do with this?
/*
	RtAudio::DeviceInfo  info = rtaudio.getDeviceInfo( rtaudio.getCurrentApi() ); // wrong
	if ( info.probed ) {
		return (info.nativeFormats & format) > 0;
	}
	return false;
*/
}

void
SoundBox::setOutputStreamName( const std::string  name ) {
	outputStreamName = name;
}

void
SoundBox::setOutputParameters(
	PlaybackApi  audioAPIChoice,
	uint  bufferFrames,
	uint  samplesRate,
	bool  interleaveChannels,
	bool  hogAudioDevice,
	bool  scheduleRealtime,
	int  osSchedulingPriority,
	u8  numberOfChannels
) {
	// Support for more than to channels is not yet available
	if ( numberOfChannels > 2 )
		numberOfChannels = 2;

	bufferFramesPerCycle = bufferFrames;
	sampleRate = samplesRate;

	outputParameters.nChannels = numberOfChannels;
	outputParameters.firstChannel = 0;

	outputOptions.flags |= (interleaveChannels ? 0 : RTAUDIO_NONINTERLEAVED);
	outputOptions.flags |= (hogAudioDevice ? RTAUDIO_HOG_DEVICE : 0);
	outputOptions.flags |= (scheduleRealtime ? RTAUDIO_SCHEDULE_REALTIME : 0);
	outputOptions.flags |= (audioAPIChoice == RtAudio::UNIX_JACK ? RTAUDIO_MINIMIZE_LATENCY : 0);
	outputOptions.numberOfBuffers = numberOfChannels;
	outputOptions.streamName = outputStreamName;
	outputOptions.priority = osSchedulingPriority;
}

bool
SoundBox::open() {
	if ( rtaudio.getDeviceCount() == 0 )
		return false;

	if ( rtaudio.isStreamOpen() )
		rtaudio.closeStream();

	outputParameters.deviceId = rtaudio.getDefaultOutputDevice();

	try {
		rtaudio.openStream( &outputParameters, NULL, AUDIO_BIT_FORMAT,
			sampleRate, &bufferFramesPerCycle, &relayAudio,
			(void*)this, &outputOptions, &relayError
		);
	} catch ( RtAudioError& e ) {
		handleError(e);
		if ( rtaudio.isStreamOpen() )
			rtaudio.closeStream();
		return false;
	}
	return true;
}

bool
SoundBox::start() {
	if ( ! rtaudio.isStreamOpen() )
		return false;

	try {
		rtaudio.startStream();

	} catch ( RtAudioError& e ) {
		handleError(e);
		return false;
	}
	return true;
}

bool
SoundBox::isRunning() {
	return rtaudio.isStreamRunning();
}

bool
SoundBox::stop() {
	if ( ! rtaudio.isStreamOpen() || ! rtaudio.isStreamRunning() )
		return true;

	try {
		rtaudio.stopStream();
	} catch ( RtAudioError&  e ) {
		handleError(e);
		return false;
	}
	return true;
}

bool
SoundBox::close() {
	if ( rtaudio.isStreamOpen() ) {
		rtaudio.closeStream();
		return true;
	}
	return false;
}

std::string
SoundBox::getCurrentAPIAsString() {
	return rtaudioAPIMap[ rtaudio.getCurrentApi() ];
}

void
SoundBox::setListener( Listener*  l ) {
	listener = l;
}

void
SoundBox::loadChannelBuffer( u8  channel, SoundByte_t*  buffer, size_t  bufferSize ) {

	SampleBuffer*  channelBuffer = &bufferedSamplesLeft;
	if ( channel == 1 )
		channelBuffer = &bufferedSamplesRight;

	channelBuffer->clear();
	channelBuffer->reserve(bufferSize);

	size_t  s = 0;
	for(; s < bufferSize; ++s)
		channelBuffer->push_back(buffer[s]);
}

SampleBuffer&
SoundBox::getChannelBuffer( u8 channel ) {
	switch( channel ) {
	case 1:
		return bufferedSamplesRight;

	default: // 0
		return bufferedSamplesLeft;
	}
}

void
SoundBox::duplicateChannel( u8  source, u8  sink ) {
	if ( source == sink ) return;
	SampleBuffer&  sourceBuffer = getChannelBuffer(source);
	SampleBuffer&  sinkBuffer = getChannelBuffer(sink);
	sourceBuffer.copyTo(sinkBuffer);
}

int
SoundBox::relayAudio( void* outputBuffer, void* inputBuffer, uint numBufferFrames,
					double streamTime, RtAudioStreamStatus status, void* data )
{
	SoundBox*  soundBox = reinterpret_cast<SoundBox*>(data);
	return soundBox->handleAudioPlay( outputBuffer, inputBuffer, numBufferFrames, streamTime, status );
}

int
SoundBox::handleAudioPlay( void* outputBuffer, void* /*inputBuffer*/, uint numBufferFrames,
					double streamTime, RtAudioStreamStatus status )
{
	//if ( status == RTAUDIO_INPUT_OVERFLOW || status == RTAUDIO_OUTPUT_UNDERFLOW )
	if ( status ) // Underflow or Overflow when status > 0
		return 1;

	if ( (outputOptions.flags & RTAUDIO_NONINTERLEAVED) > 0 ) {
		return writeToF64Buffer( (f64*)outputBuffer, numBufferFrames, streamTime );
	} else {
		return writeToF64BufferInterleaved( (f64*)outputBuffer, numBufferFrames, streamTime );
	}
}

int
SoundBox::writeToF64Buffer( f64*  buffer, uint  numBufferFrames, double  streamTime ) {
	uint  s = 0;
	u8  c = 0;
	for (; c < outputOptions.numberOfBuffers; ++c) {
		for (s = 0; s < numBufferFrames; ++s )
			buffer[s] = getSample(c, s, streamTime) / SOUND_BYTE_SCALE;
	}
	return 0;
}

int
SoundBox::writeToF64BufferInterleaved( f64*  buffer, uint  numBufferFrames, double  streamTime ) {
	uint  s = 0;
	u8  c = 0;
	const uint  buffs = outputOptions.numberOfBuffers;
	for (; s < numBufferFrames; ++s) {
		for (c = 0; c < buffs; ++c)
			buffer[s*buffs+c] = getSample(c, s, streamTime) / SOUND_BYTE_SCALE;
	}
	return 0;
}

SoundByte_t
SoundBox::getSample( u8  channel, uint  /*sampleIndex*/, double  /*streamTime*/ ) {
	SoundByte_t  sample;
	SampleBuffer*  buffer = &bufferedSamplesLeft;
	if ( channel == 1 )
		buffer = &bufferedSamplesRight;

	sample = buffer->getSample();
	if ( buffer->finished() )
		informChannelBufferIsEmpty(channel);

	return sample;
}

void
SoundBox::informChannelBufferIsEmpty( u8  channel ) {
	if ( listener )
		listener->OnSoundBoxChannelEmpty(channel);
}

void
SoundBox::relayError( RtAudioError::Type pType, const std::string& pErrorText ) {
	if ( pType == RtAudioError::WARNING ) {
		std::printf("RTAUDIO WARNING: %s\n", pErrorText.c_str());
	} else if ( pType == RtAudioError::DEBUG_WARNING ) {
		std::printf("RTAUDIO DEBUG WARNING: %s\n", pErrorText.c_str());
	} else {
		throw RtAudioError( pErrorText, pType );
	}
}

void  SoundBox::handleError( RtAudioError&  e ) {
	errorType = e.getType();

	switch ( errorType )
	{
	case RtAudioError::NO_DEVICES_FOUND:  /*!< No devices found on system. */

		break;

	case RtAudioError::INVALID_DEVICE:    /*!< An invalid device ID was specified. */

		break;

	// WARNING and DEBUG_WARNING are handled in the error callback.
	/*  Other error types:

		MEMORY_ERROR,      // An error occured during memory allocation.
		INVALID_PARAMETER, // An invalid parameter was specified to a function.
		INVALID_USE,       // The function was called incorrectly.
		DRIVER_ERROR,      // A system driver error occured.
		SYSTEM_ERROR,      // A system error occured.
		THREAD_ERROR       // A thread error occured.
	*/

	default: break;
	}
}

}

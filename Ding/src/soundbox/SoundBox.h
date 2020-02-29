// (c) 2019 Nicolaus Anderson

#ifndef SOUND_BOX_H
#define SOUND_BOX_H

#include <RtAudio.h>
#include <map>
#include <string>

namespace soundbox {

typedef  double  SoundByte_t;
static constexpr SoundByte_t  SOUND_BYTE_SCALE = 1;

//! Sample Buffer
//! Should be an interface
struct SampleBuffer {
	typedef  std::vector<SoundByte_t>  Buffer_t;

	SampleBuffer();
	void  copyTo( SampleBuffer& );
	void  clear();
	void  reserve( size_t );
	void  push_back( const SoundByte_t );
	SoundByte_t  getSample();
	void  setLooping( bool );
	bool  finished(); // Never returns true for looping

private:
	Buffer_t  buffer;
	size_t  usedSamples;
	bool  looping;
};

//! Sound Box
/*
	This class is simply a wrapper for RT Audio that uses specific settings.
	It sets up the output stream and uses 64-bit floating numbers for values.
*/
struct SoundBox {

	// Definitions ---------------------------------
	// TODO: Remove the unnecessary ones.
	typedef  unsigned  uint;
	typedef  unsigned char  u8;
	typedef  signed char  s8;
	typedef  signed short s16;
	typedef  signed long  s32;
	typedef  float  f32;
	typedef  double f64;

	typedef RtAudioFormat AudioFormat;
	const RtAudioFormat AUDIO_BIT_FORMAT = RTAUDIO_FLOAT64;

	//typedef  std::vector<SoundByte_t>  SampleBuffer;

	//! Sound Api (alias)
	typedef  RtAudio::Api  PlaybackApi;
	/*
    UNSPECIFIED,    < Search for a working compiled API.
    LINUX_ALSA,     < The Advanced Linux Sound Architecture API.
    LINUX_PULSE,    < The Linux PulseAudio API.
    LINUX_OSS,      < The Linux Open Sound System API.
    UNIX_JACK,      < The Jack Low-Latency Audio Server API.
    MACOSX_CORE,    < Macintosh OS-X Core Audio API.
    WINDOWS_WASAPI, < The Microsoft WASAPI API.
    WINDOWS_ASIO,   < The Steinberg Audio Stream I/O API.
    WINDOWS_DS,     < The Microsoft Direct Sound API.
    RTAUDIO_DUMMY   < A compilable but non-functional API.
	*/

	// Listener ------------------------------------
	//! Interface
	struct Listener {
		virtual void  OnSoundBoxChannelEmpty( u8 )=0;
	};

	// Methods -------------------------------------

	// Usage:
	// setOutputParameters() loadChannelBuffer() open() start() [isRunning()] stop() close()
	// Note:
	// loadChannelBuffer() can occur before or after open()

	SoundBox();

	~SoundBox();

	//! Check support
	bool getDeviceCount();
	bool isAPISupported( PlaybackApi );
	bool isAudioFormatSupported( AudioFormat );

	//! Set Output Stream Name
	void  setOutputStreamName( std::string );

	//! Set Output Parameters
	//! Return false if failed
	void
	setOutputParameters(
		PlaybackApi  audioAPIChoice = RtAudio::UNSPECIFIED,
		uint  bufferFrames = 512,
		uint  samplesRate = 44100,
		bool  interleaveChannels = true,
		bool  hogDevice = false,
		bool  scheduleRealtime = true,
		int  osSchedulingPriority = 1,
		u8  numberOfChannels = 2
	);

	//! Initialize the stream
	bool  open();

	//! Initialize the audio playback, return false if failed
	bool  start();

	//! Indicate if the audio is in playback
	bool  isRunning();

	//! Stop the audio playback
	bool  stop();

	//! Close the stream
	bool  close();

	//!
	std::string  getCurrentAPIAsString();

	//!
	void  setListener( Listener* );

	//! Load the Output Buffer
	/*
		Loads the output buffer for the given channel.
		When that channel is played, the audio from that buffer will be output.
	*/
	void  loadChannelBuffer( u8  channel, SoundByte_t*  buffer, size_t  bufferSize );
	
	//! Get Channel Buffer for Output
	SampleBuffer&  getChannelBuffer( u8 channel );

	//! Copy channels to each other
	void  duplicateChannel( u8  source, u8  sink );

	//! RT Audio Callback (via SoundBox_RtAudioCallback())
	static int  relayAudio( void*, void*, uint, double, RtAudioStreamStatus, void* );
	int  handleAudioPlay( void*, void*, uint, double, RtAudioStreamStatus );

	//! Error handling
	static void  relayError( RtAudioError::Type pType, const std::string& pErrorText );

protected:
	int  writeToF64Buffer( f64*, uint, double );
	int  writeToF64BufferInterleaved( f64*, uint, double );

	//! Fetch the sample, considering the given start time
	SoundByte_t  getSample( u8, uint, double );

	//! Handle error (may perform dispatching)
	void  handleError( RtAudioError& );

	//! Trigger callback for when the buffers are empty
	void  informChannelBufferIsEmpty( u8 );

private:
	bool  ready;
	RtAudio  rtaudio;
	std::map<int, std::string> rtaudioAPIMap;
	std::vector<RtAudio::Api> rtaudioAPIs;
	RtAudio::DeviceInfo  currentDeviceInfo;
	uint  bufferFramesPerCycle;
	uint  sampleRate;
	std::string  outputStreamName;

	RtAudio::StreamParameters  outputParameters;
	RtAudio::StreamOptions  outputOptions;

	RtAudioError::Type  errorType;

	// Implementation subject to change --------------
	//! Stereo buffer
	SampleBuffer  bufferedSamplesLeft;
	SampleBuffer  bufferedSamplesRight;

	Listener*  listener;
};

} // end namespace soundbox

#endif // SOUND_BOX_H

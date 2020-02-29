// (c) 2019 Nicolaus Anderson

#ifndef SOUND_BOX_H
#define SOUND_BOX_H

#include <RTAudio.h>
#include <map>
#include <string>

namespace soundbox {

//! Sound Box
/*
	This class is simply a wrapper for RT Audio that uses specific settings.
	It contains a pair of buffers - of type double - used for saving sound data.
*/
struct SoundBox {

	// Definitions ---------------------------------
	typedef  unsigned  uint;
	typedef  signed char  s8;
	typedef  signed short s16;
	typedef  signed long  s32;
	typedef  float  f32;
	typedef  double f64;

	static const s8 SCALE_8bit = 127;
	static const s16 SCALE_16bit = 32767;
	static const s32 SCALE_32bit = 2147483647;
	
	typedef  double  SoundByte_t;
	static const SoundByte_t  SOUND_BYTE_SCALE = 1;

	typedef  std::vector<SoundByte>  SampleBuffer;

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

	// Methods -------------------------------------

	SoundBox();

	~SoundBox();

	//! Set Output Stream Name
	void  setOutputStreamName( std::string );

	//! Set Output Parameters
	//! Return false if failed
	void
	setOutputParameters(
		PlaybackApi  audioAPIChoice = RtAudio::UNSPECIFIED,
		uint  bufferFrames = 512,
		uint  sampleRate = 44100,
		bool  interleaveChannels = true,
		bool  hogDevice = false,
		bool  scheduleRealtime = true
		int  osSchedulingPriority = 1
		s8  numberOfChannels = 2,
	);

	//! Initialize, return false if failed
	bool
	init();

	std::string  getCurrentAPIAsString();

	//! Load the Output Buffer
	/*
		Loads the output buffer for the given channel.
		When that channel is played, the audio from that buffer will be output.
	*/
	void  loadChannelBuffer( s8  channel, SoundByte_t*  buffer, size_t  bufferSize );
	
	//! Get Channel Buffer for Output
	SampleBuffer&  getChannelBuffer( s8 channel );

	//! RT Audio Callback (via SoundBox_RtAudioCallback())
	int  handleAudioPlay( void*, void*, uint, double, RtAudioStreamStatus );

protected:
	int  writeToS8Buffer( s8*, uint, double );
	int  writeToS16Buffer( s16*, uint, double );
	int  writeToS32Buffer( s32*, uint, double );
	int  writeToF32Buffer( f32*, uint, double );
	int  writeToF64Buffer( f64*, uint, double );

	int  writeToS8BufferInterleaved( s8*, uint, double );
	int  writeToS16BufferInterleaved( s16*, uint, double );
	int  writeToS32BufferInterleaved( s32*, uint, double );
	int  writeToF32BufferInterleaved( f32*, uint, double );
	int  writeToF64BufferInterleaved( f64*, uint, double );

	//! Fetch the sample, considering the given start time
	SoundByte_t  getSample( s8, uint, double );

private:
	bool  ready;
	RTAudio  rtaudio;
	std::map<int, std::string> rtaudioAPIMap;
	std::vector<RtAudio::Api> rtaudioAPIs;
	RtAudio::DeviceInfo  currentDeviceInfo;
	uint  bufferFramesPerCycle;
	uint  sampleRate;
	RtAudioFormat  outputBufferType;
	std::string  outputStreamName;

	RtAudio::StreamParameters  outputParameters;
	RtAudio::StreamOptions  outputOptions;

	// Implementation subject to change --------------
	//! Stereo buffer
	SampleBuffer  bufferedSamplesLeft;
	SampleBuffer  bufferedSamplesRight;
};

int  SoundBox_RtAudioCallback( void*, void*, uint, double, RtAudioStreamStatus, void* );

} // end namespace soundbox

#endif // SOUND_BOX_H

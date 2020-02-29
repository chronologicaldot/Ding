
#include <cstdlib> // For calloc() and free()
#include <stdio.h>
#include <iostream>
#include <math.h>
#include <string.h>
#include "../../rtaudio/RtAudio.h"

#define USE_INTERLEAVED 1

/*
I'm inconsistent here. Due to the fact that I must use double for creating
good sound waves but I want simple integers for output, I have to find
appropriate names for the different types of sound bytes. The fact that
output soundbytes are special means I can probably reserve "SoundByte" for
just internal use.

Furthermore, I will eventually need to check the supported types
(see rtaudio/tests/audioprobe.cpp for how to do this).
*/
#define AUDIO_BIT_FORMAT RTAUDIO_SINT32
typedef signed int		OutSoundByte; // Used for actual output
//#define OUT_SOUND_BYTE_SCALE  8388607.0
#define OUT_SOUND_BYTE_SCALE 2147483648.0
//typedef signed short	OutSoundByte;
//#define OUT_SOUND_BYTE_SCALE  32767.0

typedef double			SoundByte;

typedef unsigned int	uint;
#define PI 3.1415926535897932384626433832795028841971693993751


// Platform-dependent sleep routines.
#if defined( __WINDOWS_ASIO__ ) || defined( __WINDOWS_DS__ ) || defined( __WINDOWS_WASAPI__ )
  #include <windows.h>
  #define SLEEP( milliseconds ) Sleep( (DWORD) milliseconds ) 
#else // Unix variants
  #include <unistd.h>
  #define SLEEP( milliseconds ) usleep( (unsigned long) (milliseconds * 1000.0) )
#endif


void errorCallback( RtAudioError::Type pType, const std::string& pErrorText )
{
	if ( pType == RtAudioError::WARNING )
		printf( "\nRtAudio error: %s", pErrorText.c_str() );
	else if ( pType != RtAudioError::WARNING )
		throw( RtAudioError( pErrorText, pType ) );
}

/*
Struct used for holding the source data to be broadcast.

In future implementations, this interface needs more accessor
protection, so I must make a practice of setting the data here
using only arrays and vectors whose size is definitely known.

Also, interleaved buffers are different that non-interleaved buffers.
Having multiple channels (2 for headphones) means I have to account for
them, which means changing how the data is returned.

Finally, not all data will be filled. Here, numSamples == data array size.
This will most likely not be the case, but it might be. It all depends on
how I structure everything else.
*/
struct RawAudioSourceData
{
	// Should be based on #define from RTAUDIO_SINT32
	SoundByte* data;
	uint numSamples;
	uint nextSampleIndex; // Which sample is to be read next. Should be part of channel data in get function.
	uint nextSampleIndex2; // Which sample is to be read next. Should be part of channel data in get function.
	uint cycle;
	bool loop;
	uint numChannels; // Not supposed to be here, but needed for getSoundData().

	RawAudioSourceData()
		: data(0)
		, numSamples(0)
		, nextSampleIndex(0)
		, nextSampleIndex2(0)
		, cycle(0)
		, loop(0)
		, numChannels(0)
	{}

	~RawAudioSourceData() {
		if ( numSamples > 0 ) {
			free( data );
			numSamples = 0;
		}
	}

	void initBuffer( uint pNumSamples ) {
		data = (SoundByte*) calloc( pNumSamples, sizeof( SoundByte ) );
		numSamples = pNumSamples;
	}

/*
	void setData( SoundByte* pData, unsigned int pNumSamples ) {
		data = pData;
		numSamples = pNumSamples;
	}
*/
/*
	Consideration:

	SoundByte getSample( ChannelRead& channel ) {
		uint index = channel.nextIndex;
		if ( index >= numSamples ) {
			if ( loop )
				index -= numSamples;
			else return SoundByte(0);
		}
		SoundByte sb = data[index];
		channel.nextIndex = index + 1;
		return sb;
	}
*/
	SoundByte getSample( int channel ) {
		uint index = nextSampleIndex;
		if ( channel == 1 )
			index = nextSampleIndex2;

		if ( index >= numSamples ) {
			if ( loop )
				index -= numSamples;
			else return SoundByte(0);
		}
		SoundByte sb = data[index];
		index = index + 1;
		if ( index > numSamples || index >= cycle )
			index = 0;
		if ( channel == 1 )
			nextSampleIndex2 = index;
		else
			nextSampleIndex = index;
		return sb;
	}
};

/*
I'd like to generate a nice wave prior to feeding it into the system.
This allows me to control actual sound shape, speed up the buffering,
and only need to keep track of a reading index (as opposed to also
performing wave shape calculations).

Calculations would also slow down processing since they involve sine
waves, which require intense calculations (albeit optimized).

In the future, I would like to access a table of notes (A, B, C, D, etc.)
and their frequencies in order to make creating such sounds easy.
Such a table can merely be #defines (e.g. msec_SOUND_NOTE_FREQUENCY_A ).
*/
bool generateSinewave( RawAudioSourceData& pSrcData, int pSamplesPerCycle=200 )
{
	// Buffer must be initialized
	if ( pSrcData.numSamples == 0 ) return false;

	uint samplesPerCycle = pSamplesPerCycle;

	uint i;
	for ( i = 0; i < pSrcData.numSamples; ++i  ) {
		// I have no idea how fast this is.
		//pSrcData.data[i] = /*SOUND_BYTE_SCALE*/ * sin( (double)i * PI ); // WRONG
		//pSrcData.data[i] = sin( 0.05 * PI * i ) / 2; // WRONG
		pSrcData.data[i] = sin( PI * 2 * i / samplesPerCycle ) / 2;
	}
	pSrcData.cycle = samplesPerCycle;

	//pSrcData.loop = true;

	return true;
}

double lastSample;
double lastSample2;
double volume = 0.1;
/*
	NOTE TO SELF:
	Remember that sin() and cos() are limited in range to about 2*PI at most, and even that
	is presumptuous. Data seems to work ok for that range, but it'd be nicer having a correct
	version of sine and filling in the buffers accordingly. Since I don't have tough time
	constraints (I can take my time) for creating the audio data, a correct version of sin()
	is possible.

	Remember also to make sure that the output buffer is always cast to OutSoundByte.
*/

#if defined USE_INTERLEAVED
int getSoundData( void* outputBuffer, void* inputBuffer, uint numBufferFrames,
					double streamTime, RtAudioStreamStatus status, void* data )
{
	RawAudioSourceData* sourceData = (RawAudioSourceData*)data;
	OutSoundByte* outputSoundBuffer = (OutSoundByte*) outputBuffer;

	uint numChannels = sourceData->numChannels;
	//OutSoundByte outByte;
	for ( uint f=0; f < numBufferFrames; f++ ) {
/*
		outByte = (OutSoundByte)(OUT_SOUND_BYTE_SCALE * (sin( lastSample * PI ) + 1)/2 * volume);
		for ( uint channel=0; channel < numChannels; channel++ ) {
			*outputSoundBuffer++ = outByte;
		}
		//lastSample += 0.005 * 5;
		lastSample += 0.025;
		if ( lastSample >= 2 )
			lastSample = 0;
*/
		for ( uint channel=0; channel < numChannels; channel++ ) {
			*outputSoundBuffer++ = (OutSoundByte)( sourceData->getSample(channel) * OUT_SOUND_BYTE_SCALE );
		}
	}

	return 0;
}
#else
int getSoundData( void* outputBuffer, void* inputBuffer, uint numBufferFrames,
					double streamTime, RtAudioStreamStatus status, void* data )
{
	RawAudioSourceData* sourceData = (RawAudioSourceData*)data;
	OutSoundByte* outputSoundBuffer = (OutSoundByte*) outputBuffer;

	if ( status )
		std::cout << "\nStream underflow detected.";

	// Non-interleaved buffers. But are they the other way around??
	uint numChannels = sourceData->numChannels;
	uint i;

	//for ( uint channel=0; channel < numChannels; channel++ ) {
	//	for ( uint f=0, i=0; f < numBufferFrames; f++, i++ ) {

			/*if ( i > sourceData->numSamples ) {
				if ( sourceData->loop ) {
					i = 0;
				} else {
					break;
				}
			}*/
			//*outputSoundBuffer++ = sourceData->data[i];
			// ^ same as outputSoundBuffer[numChannels * f] = sourceData->data[i];

			//if ( i > sourceData->numSamples && ! sourceData->loop ) {
			//	break;
			//}

			//*outputSoundBuffer++ = sourceData->getSample( channel );

/*
lastSample and lastSample2 must be reset to 0 because their data type cap values are reached extremely fast.
I have to use PI in order to max resetting them a matter of using a simple integer.
*/
/*
			if ( channel == 1 ) {
				*outputSoundBuffer++ = (OutSoundByte)(OUT_SOUND_BYTE_SCALE * 0.1 * sin( lastSample2 * PI ));
				lastSample2 += 0.05;
				if ( lastSample2 >= 2 )
					lastSample2 = 0;
			} else {
				*outputSoundBuffer++ = (OutSoundByte)(OUT_SOUND_BYTE_SCALE * 0.1 * sin( lastSample * PI ));
				lastSample += 0.05;
				if ( lastSample >= 2 )
					lastSample = 0;
			}
*/
	//	}
	//}


	for ( uint f=0, i=0; f < numBufferFrames; f++, i++ ) {
		*outputSoundBuffer++ = (OutSoundByte)(OUT_SOUND_BYTE_SCALE * (sin( lastSample * PI ) + 1 )/2 * volume);
		lastSample += 0.025;
		if ( lastSample >= 2 )
			lastSample = 0;
	}

	if ( numChannels == 2 )
	for ( uint f=0, i=0; f < numBufferFrames; f++, i++ ) {
		*outputSoundBuffer++ = (OutSoundByte)(OUT_SOUND_BYTE_SCALE * sin( lastSample2 * PI ) + 1)/2 * volume);
		lastSample2 += 0.025;
		if ( lastSample2 >= 2 )
			lastSample2 = 0;
	}


	return 0; /* return == 0 continues playback.
				return == 1 drains buffers and then stops playback.
				return == 2 immediately stops playback and empties buffers. */
}
#endif

int main(int argc, char* argv[])
{
	lastSample = 0;
	lastSample2 = 0;

	int samplesPerCycle = 200;
	if ( argc > 1 )
		samplesPerCycle = atoi(argv[1]);
	if ( samplesPerCycle == 0 )
		samplesPerCycle = 200;

	//uint bufferFrames = 512;
	uint bufferFrames = samplesPerCycle;
	//uint audioDevice = 0;
	uint sampleRate = 44100; // In the future, I need to check devices for supported sample rate
	//uint sampleRate = 22050;
	RtAudio rtaudio;

	if ( rtaudio.getDeviceCount() == 0 ) {
		std::cout << "No device found. Now exiting.\n";
		return 1;
	}
	try {
		rtaudio.showWarnings(true);
	} catch ( ... ) {
		std::cout << "Display warnings not enabled!\n";
	}

	RawAudioSourceData rawData;
	rawData.initBuffer( samplesPerCycle );

	RtAudio::StreamParameters outStreamParams;
	outStreamParams.deviceId = rtaudio.getDefaultOutputDevice();
	outStreamParams.nChannels = 2; // For both speakers
	outStreamParams.firstChannel = 0;

	RtAudio::StreamOptions options;
	//options.flags = RTAUDIO_HOG_DEVICE; // I wonder what it will be like without this
	//options.flags |= RTAUDIO_SCHEDULE_REALTIME;

	options.flags = RTAUDIO_SCHEDULE_REALTIME;

#if !defined( USE_INTERLEAVED )
	options.flags |= RTAUDIO_NONINTERLEAVED;
#endif

	if ( ! generateSinewave( rawData, samplesPerCycle ) )
		return 0;
	rawData.numChannels = 2;
	rawData.loop = true;

	try {
		rtaudio.openStream( &outStreamParams, NULL, AUDIO_BIT_FORMAT, sampleRate,
							&bufferFrames, &getSoundData, (void *)&rawData, &options,
							&errorCallback );

		rtaudio.startStream();
	} catch ( RtAudioError& e ) {
		e.printMessage();
		goto cleanup;
	}

	while ( rtaudio.isStreamRunning() )
		SLEEP( 100 );

	try {
		rtaudio.stopStream();
	} catch ( RtAudioError& e ) {
		e.printMessage();
	}

cleanup:
	if ( rtaudio.isStreamOpen() )
		rtaudio.closeStream();

	return 0;
}

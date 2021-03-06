// (c) 2019 Nicolaus Anderson

#ifndef DING_WAVEFORM_H
#define DING_WAVEFORM_H

#include "ding_types.h"
#include "ding_waveform_types.h"
#include "ding_sampler.h"
#include "ding_volume_span.h"

namespace ding {

//! Waveform
/*
	This is a sound buffer modified by volume settings and aligned with a time scale from 0 to 1.
		The time values are a percentage of the total playback time of the wave.
	Intended Order of Usage:
		- construct
		- setDurationPercent()
		- setFrequency()
		- setWaveformType()
		- setSampleCount()
		- getVolumeSpan(), adjust volumes
		- getMaxSamplesProvided()
		- getSample()

	TODO:
		Remove getMaxSamplesProvided() and replace with getDuration().
*/
struct Waveform
{
	// cstor
	Waveform();

	//! dstor
	~Waveform();

	//! Set Duration of the Wave
	/*
		Sets the time that the wave will play.
	*/
	void  setDuration( play_t );

	//! Set Wave Frequency
	void  setFrequency( play_t );

	//! Set Sample Count
	/*
		Sets the number of samples returned from this waveform.
	*/
	void  setSampleCount( index_t );

	//! Set Waveform Type
	/*
		Sets the kind of waveform produced here.
	*/
	void  setWaveformType( BaseWaveform::Value );

	//! Get Volume Span
	/*
		Returns the span that controls the volumes of this waveform.
	*/
	VolumeSpan&  getVolumeSpan();

	//! Get the maximum samples provided
	/*
		Returns the total number of samples generated by this waveform before it repeats.
		It is possible that the waveform will provide samples after this value, but the
		waveform is also free to return 0.
	*/
	index_t  getMaxSamplesProvided() const;

	//! Retrieve Sample
	/*
		Returns the sample at the given time.
		The first parameter is the time that the wave has been playing.
		If the second parameter is false, volume-settings will be disabled.
	*/
	volume_t  getSample( play_t, bool );

	//! Retrieve Sample
	/*
		Returns the sample at the given index.
		The first parameter is the time that the wave has been playing.
		If the second parameter is false, volume-settings will be disabled.
	*/
	volume_t  getSampleByIndex( index_t, bool );

protected:

	void  regenerateSamples();

private:

	SampleProvider  sampler;
	VolumeSpan  volumeSpan;
	BaseWaveform::Value  waveformType;
	play_t  duration;
	index_t  sampleCount;
	play_t  frequency;
	//bool  dirty;
};

} // end namespace ding

#endif // DING_WAVEFORM_H

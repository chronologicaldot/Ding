// (c) 2019 Nicolaus Anderson

#ifndef DING_WAVE_H
#define DING_WAVE_H

#include "ding_types.h"
#include "ding_waveform_types.h"
#include "ding_io_interface.h"
#include "ding_sample_source.h"
#include "ding_volume_span.h"
#include <memory> // For shared_ptr
#include <string> // For the name

namespace ding {

//! Predeclaration for source for SampleSources
class SampleSourceProvider;

//! Wave
/*
	A simple wave source.
	It takes two sample sources: one used for sound samples and one used for volume.
	The sound samples are multiplied by a frequency, thus allowing the repetition of waves every cycle.
		It is assumed that time == 1 is a full cycle.
	The volume samples are clamped from 0 to 1.
*/
struct Wave
	: public SampleSource
{
	static constexpr const char* const IO_NAME = "wave";

	//! cstor
	Wave();

	//! dstor
	~Wave();

	//! Set Duration
	void  setDuration( play_t );

	//!
	play_t  getDuration() const;

	//! Set Sound Wave Frequency
	/*
		A value of 1 in time is considered a full cycle.
		This makes wave operations independent of the underlying primitive type
			and OS implementation of time type.
	*/
	void  setFrequency( play_t );

	//!
	play_t  getFrequency() const;

	//! Set Sound Wave Phase
	/*
		Sets the time that acts as the wave 0.
	*/
	void  setPhaseShift( play_t );

	//!
	play_t  getPhaseShift() const;

	//! Invert the Wave
	void  invertWave( bool );

	//!
	bool  isInvertedWave() const;

	//! Set Sample Source Provider
	//! For defaulting
	void  setSampleSourceProvider( SampleSourceProvider* );

	//! Set Source for Sound Samples
	void  setSampleSource( std::shared_ptr<SampleSource> );

	//! Set Volume Multiplier
	void  setVolumeMultiplier( std::shared_ptr<VolumeSpan> );

	//! Get Sample Source
	std::shared_ptr<SampleSource>  getSampleSource();

	//! Get Volume Source
	std::shared_ptr<VolumeSpan>  getVolumeMultiplier();

	//! Check for a Sample Source
	bool  hasSampleSource() const;

	//! Check for a Volume Multiplier
	bool  hasVolumeMultiplier() const;

	//! Create a Default Volume Multiplier for Itself
	void  initNewVolumeMultiplier();

	//! Get a Sample Modded by Frequency and Phase
	volume_t  getReadiedSample( play_t );

	//! Write to IO Interface
	virtual void  serialize( IOInterface& );

	//! Read from IO Interface
	virtual void  deserialize( IOInterface& );

protected:
	//! Retrieve a Sound Sample
	/*
		Takes the given number of cycles (the time value) and returns the sample volume.
	*/
	virtual volume_t  getSampleUnmuted( play_t );

	// Members ----------------------------------
public:
	std::string  Name;

protected:
	SampleSourceProvider*  sampleSourceProvider;
	std::shared_ptr<SampleSource>  sampleSource;
	std::shared_ptr<VolumeSpan>  volumeMultiplier;
	play_t  duration;
	play_t  frequency;
	play_t  phaseShift;
	bool  isInverted;
};

}

#endif // DING_WAVE_H

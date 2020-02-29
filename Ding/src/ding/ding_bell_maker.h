// (C) 2019 Nicolaus Anderson

#ifndef DING_BELL_MAKER_H
#define DING_BELL_MAKER_H

#include "ding_sample_source_provider.h"
#include "ding_wave.h"
#include "ding_io_interface.h"

namespace ding {

// ! Bell Maker
/*
	This class takes a set of waveforms and creates a bell with them.
	It also has an overall volume control.
*/
struct BellMaker
	: public IOSerializable
	, public SampleSourceProvider
{
	static constexpr const char* const IO_NAME = "bell";

	//! cstor
	BellMaker();

	//! dstor
	~BellMaker();

	//! Drop all waves and return to defaults
	void  reset();

	//! Get the Wave Count
	index_t  getWaveCount() const;

	//! Access the Wave
	/*
		Obtains the wave at the given index. If no wave exists, then one is created.
	*/
	Wave&  getWave( index_t );

	//! Set Basic Wave Source For Wave
	/*
		Creates a wave source of the given form and sets it to the wave at the given index.
	*/
	void  setBasicWaveSource( index_t, BaseWaveform::Value );

	//! Remove Wave
	void  removeWave( size_t );

	//! Access the Volume Modifier
	VolumeSpan&  getVolumeModifier();

	//! Set Samples per Second
	void  setSamplesPerSecond( index_t );

	//! Get Number of Samples Per Second
	index_t  getSamplesPerSecond() const;

	//! Get a Compiled Sample at a Playback Time
	volume_t  fetchCompiledSample( play_t );

	//! Find Longest Duration
	play_t  getMaxDuration();

	//! Load Into Buffer
	/*
		Attempts to build the waveform and put it in the given buffer.
	*/
	template<class ContainerType>
	void loadIntoContainer( ContainerType&  container, size_t  maxLoadAmount, bool  reduceVolumeByWaveCount=false ) {

		//for ( Wave&  w : waves ) {
		//	if ( ! w.getVolumeMultiplier().get() )
		//		continue;
		//	w.getVolumeMultiplier().get()->print();
		//}

		container.clear();
		unsigned  i = 0;
		//play_t  elapsed = static_cast<play_t>(container.size()) / static_cast<play_t>(samplesPerSecond);
		const play_t  duration = getMaxDuration();
		play_t  currentTime = 0; // This could be set to the streamTime from RtAudio for realtime playback applications.

		volume_t  divisor = 1;
		if ( reduceVolumeByWaveCount )
			divisor = static_cast<volume_t>( waves.size() );

		for (; (i < maxLoadAmount || maxLoadAmount == 0) && currentTime < duration; ++i ) {
			const volume_t  sample = fetchCompiledSample(currentTime);
			container.push_back( sample / divisor );
			currentTime += samplingStep;
		}
	}

	//! Write to IO Interface
	virtual void  serialize( IOInterface& );

	//! Read from IO Interface
	virtual void  deserialize( IOInterface& );

	//! Create a sample source from the given identifier
	virtual std::shared_ptr<SampleSource>
	getSampleSource( unsigned  identifier );

protected:

	list_t<Wave>  waves;
	VolumeSpan  volumeSpan;
	index_t  samplesPerSecond;
	play_t  samplingStep;
};

}

#endif // DING_BELL_H

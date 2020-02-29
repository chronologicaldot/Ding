// (c) 2019 Nicolaus Anderson

#include "ding_bell.h"

namespace ding {

Waveform::Waveform()
	: sampler()
	, volumeSpan()
	, waveformType(BaseWaveform::flat)
	, duration(1)
	, sampleCount(0)
	, frequency(44100)
{}

Waveform::~Waveform()
{}

void
Waveform::setDuration( play_t  time ) {
	duration = time;
}

void
Waveform::setFrequency( play_t  value ) {
	frequency = value;
	regenerateSamples();
}

void  Waveform::setWaveformType( BaseWaveform::Value  form ) {
	waveformType = form;
	regenerateSamples();
}

void
Waveform::setSampleCount( index_t  count ) {
	sampleCount = count;
	regenerateSamples();
}

VolumeSpan&
Waveform::getVolumeSpan() {
	return volumeSpan;
}

index_t
Waveform::getMaxSamplesProvided() const {
	return sampler.getMaxSamplesProvide();
}

volume_t
Waveform::getSample( play_t  time, bool disableVolumeSpan ) {
	//if ( dirty )
	//	regenerateSamples();

	if ( sampler.getMaxSamplesProvided() == 0 || time > duration )
		return 0;

	index_t  sampleIndex = sampler.getMaxSamplesProvided() * (time * frequency / duration);
	volume_t  sample = sampler.getSample( sampleIndex );

	if ( disableVolumeSpan )
		return sample;

	return sample * volumeSpan.getVolumeAtTime(time);
}

volume_t
Waveform::getSampleByIndex( index_t  index ) {
	//if ( dirty )
	//	regenerateSamples();

	if ( sampler.getMaxSamplesProvided() == 0 )
		return 0;

	index_t  sampleIndex = sampler.getMaxSamplesProvided() * index;
	volume_t  sample = sampler.getSample( sampleIndex );

/*
	if ( disableVolumeSpan )
		return sample;

	return sample * volumeSpan.getVolumeAtTime(time);
*/
}

void
Waveform::regenerateSamples() {
	// The sampler only produces one cycle of the wave based on the number of requested samples.
	// The larger the frequency, the fewer samples that need to be produced for a single cycle.
	sampler.generateSamples(waveformType, sampleCount / frequency);
	//dirty = false;
}


} // end namespace ding

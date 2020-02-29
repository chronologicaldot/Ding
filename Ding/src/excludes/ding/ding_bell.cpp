// (c) 2019 Nicolaus Anderson

#include "ding_bell.h"

namespace ding {


Bell::Bell()
	: waveforms()
	, volumeSpan()
	, samplesBuffer(nullptr)
	, sampleCount(0)
	, duration(0)
	, desiredSampleCount(0)
{}

Bell::~Bell()
{
}

void
Bell::setDuration( play_t  time ) {
	duration = time;

	index_t  w = 0;
	for(; w < waveforms.size(); ++w) {
		waveform[w].setDuration(duration);
	}
}

void
Bell::setDesiredSampleCount( index_t  count ) {
	desiredSampleCount = count;

	index_t  w = 0;
	for(; w < waveforms.size(); ++w) {
		waveform[w].setSamplecount(desiredSampleCount);
	}
}

Waveform&
Bell::getWaveform( index_t  index ) {
	if ( index >= waveforms.size() ) {
		waveform.push_back(Waveform{});
		waveform.last().setSampleCount(desiredSampleCount);
		return waveform.last();
	}

	return waveform[index];
}

index_t
Bell::getWaveformCount() {
	return static_cast<index_t>( waveforms.size() );
}

VolumeSpan&
Bell::getVolumeSpan() {
	return volumeSpan;
}

bool
Bell::build() {
	play_t  startTime = 0;
	play_t  timeDelta = duration / desiredSampleCount;
	index_t  w;
	while ( startTime < duration ) {

		for ( w=0; w < waveforms.size(); ++w ) {
			waveform[w].getSample(
		}

		startTime += timeDelta;
	}

	// BAD ----------------
	constructSamplesBufferOfSize( duration * samplesPerSecond * PLAYBACK_TICKS_PER_SECOND );

	index_t  s = 0;
	for(; s < sampleCount; ++s) {

		//FIXME
		/* Predicament:

			My waveforms all return samples based on a time resolution.
			Combining samples is MUCH easier if I have a strict samples-per-second for an entire project.
			Then I don't need to access points by time.
			However, I also can't speed things up, per se. Hm...

			Finally, volume settings are supposed to be by percentage of full scale, not by some "time" value.
			Not that I've done much with that. It would be easy to change.

			I suppose that, instead of time, I should be working with "full scale".

			FOLLOW THE README
		*/

	}

	return true;
}

//const volume_t*
//Bell::getDataBuffer() {
//}

index_t
Bell::getMaxSamplesProvided() const {
	return sampleCount;
}

volume_t
Bell::getSample( play_t  time, bool disableVolumeSpan ) {
	if ( sampleCount == 0 || time > duration )
		return 0;

	index_t  sampleIndex = sampleCount * time;
	volume_t  sample = sampleBuffer[sampleIndex];

	if ( disableVolumeSpan )
		return sample;

	return sample * volumeSpan.getSample(time);
}

void
Bell::destroySamplesBuffer() {
	if ( samplesBuffer ) {
		delete[] samplesBuffer;
		samplesBuffer = nullptr;
		sampleCount = 0;
	}
}

void
Bell::constructSamplesBufferOfSize( index_t  size ) {
	destroySamplesBuffer();

	sampleCount = size;
	samplesBuffer = new volume_t[sampleCount];
}

} // end namespace ding

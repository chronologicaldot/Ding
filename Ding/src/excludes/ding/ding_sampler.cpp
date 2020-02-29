// (c) 2019 Nicolaus Anderson

#include "ding_sampler.h"
#include "ding_build_waveforms.h"

namespace ding {

SampleProvider::SampleProvider()
	: repeatRule( RepeatRule::none )
	, samplesBuffer(nullptr)
	, samplesCount(0)
{
}

SampleProvider::~SampleProvider()
{
	destroySamplesBuffer();
}

void
SampleProvider::generateSamples( BaseWaveform::Value  baseform, index_t  numSamples) {
	if ( numSamples == 0 )
		return;

	switch( baseform )
	{
	case BaseWaveform::sine:
		constructSamplesBufferOfSize(numSamples / 4);
		createSineWaveFirstQuarter( samplesBuffer, sampleCount );
		repeatRule = RepeatRule::nfxfyfxy;
		break;

	case BaseWaveform::cosine:
		constructSamplesBufferOfSize(numSamples / 4);
		createCosineWaveFirstQuarter( samplesBuffer, sampleCount );
		repeatRule = RepeatRule::nfxfyfxy;
		break;

	case BaseWaveform::saw:
		constructSamplesBufferOfSize(numSamples);
		createAscendingSawWave( sampleBuffer, sampleCount );
		repeatRule = RepeatRule::repeat;

	case BaseWaveform::fsaw:
		constructSamplesBufferOfSize(numSamples);
		createAscendingSawWave( sampleBuffer, sampleCount );
		repeatRule = RepeatRule::repeat;
		break;

	case BaseWaveform::bsaw:
		constructSamplesBufferOfSize(numSamples);
		createDescendingSawWave( sampleBuffer, sampleCount );
		repeatRule = RepeatRule::repeat;
		break;

	case BaseWaveform::square:
		constructSamplesBufferOfSize(numSamples);
		createSquareWave( sampleBuffer, sampleCount );
		repeatRule = RepeatRule::repeat;
		break;

	default: break;
	}
}

void
SampleProvider::generateSamples( SampleProvider&  provider) {
	constructSamplesBufferOfSize( feed.getMaxSamplesProvided() );

	index_t  s = 0;
	while ( s < sampleCount ) {
		samplesBuffer[s] = provider.getSampleByIndex( s );
		++s;
	}
}

void
SampleProvider::generateSamples( SampleByIndexFeed&  feed) {
	constructSamplesBufferOfSize( feed.getMaxSamplesProvided() );

	index_t  s = 0;
	while ( s < sampleCount ) {
		samplesBuffer[s] = feed.getSampleByIndex( s );
		++s;
	}
}

void
SampleProvider::setRepeatRule( RepeatRule::Value  rule ) {
	repeatRule = rule;
}

RepeatRule::Value
SampleProvider::getRepeatRule() const {
	return repeatRule;
}

volume_t
SampleProvider::getSampleByIndex( index_t  index ) {
	if ( sampleCount == 0 )
		return 0;

	switch ( repeatRule )
	{
	case RepeatRule::none:
		if ( index < sampleCount )
			return samplesBuffer[index];
		break;

	case RepeatRule::repeat:
		while ( index >= sampleCount )
			index -= sampleCount;

		return samplesBuffer[index];


	case RepeatRule::mirror_x:
		while ( index >= sampleCount * 2 )
			index -= sampleCount * 2;

		if ( index < sampleCount )
			return samplesBuffer[index];

		// sampleCount - (index - sampleCount) = sampleCount*2 - index
		return samplesBuffer[sampleCount*2 - index];


	case RepeatRule::mirror_y:
		while ( index >= sampleCount * 2 )
			index -= sampleCount * 2;

		if ( index < sampleCount )
			return samplesBuffer[index];

		return - samplesBuffer[index - sampleCount];


	case RepeatRule::nfxfyfxy:
		while ( index >= sampleCount * 4 )
			index -= sampleCount * 4;

		if ( index < sampleCount ) // Positive xy
			return samplesBuffer[index];

		if ( index < sampleCount * 2 ) // Negative x, positive y
			// sampleCount - (index - sampleCount) = sampleCount*2 - index
			return samplesBuffer[sampleCount * 2 - index];

		if ( index < sampleCount * 3 ) // Positive x, negative y
			return - samplesBuffer[index - sampleCount * 3];

		// Negative x, negative y
			// sampleCount - (index - sampleCount*3) = sampleCount*4 - index
		return - samplesBuffer[sampleCount*4 - index];


	default:
		break;
	}

	return 0;
}

index_t
SampleProvider::getMaxSamplesProvided() const {
	switch( repeatRule )
	{
	case RepeatRule::mirror_x:
	case RepeatRule::mirror_y:
		return sampleCount * 2;

	case RepeatRule::nfxfyfxy:
		return sampleCount * 4;

	default: // "none", "repeat"
		return sampleCount;
	}
}

void
SampleProvider::destroySamplesBuffer() {
	if ( samplesBuffer ) {
		delete[] samplesBuffer;
		samplesBuffer = nullptr;
		sampleCount = 0;
	}
}

void
SampleProvider::constructSamplesBufferOfSize( index_t  size ) {
	destroySamplesBuffer();
	if ( size == 0 )
		return;

	samplesBuffer = new volume_t[size];
	samplesCount = size;
}


//----------------------------------------------------------------

SampleProviderFeed::SampleProviderFeed( SampleProvider&  source )
	: provider(source)
{}

volume_t
SampleProviderFeed::getSampleByIndex( index_t  index ) {
	return provider.getSampleByIndex(index);
}

index_t
SampleProviderFeed::getMaxSamplesProvided() {
	return provider.getMaxSamplesProvided();
}



} // end namespace ding

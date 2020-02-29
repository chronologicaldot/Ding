// (c) 2019 Nicolaus Anderson

#include "ding_bell_maker.h"
#include "ding_basic_wavesource.h"
#include <memory> // For std::make_shared

namespace ding {

BellMaker::BellMaker()
	: waves()
	, volumeSpan()
	, samplesPerSecond(0)
	, samplingStep(0)
{
	setSamplesPerSecond(44100);
}

BellMaker::~BellMaker()
{}

void
BellMaker::reset() {
	waves.clear();
}

index_t
BellMaker::getWaveCount() const {
	return (index_t) waves.size();
}

Wave&
BellMaker::getWave( index_t  waveIndex ) {
	if ( waveIndex >= waves.size() ) {
		waves.push_back(Wave{});
		waves.back().setSampleSourceProvider(this);
		return waves.back();
	}
	return waves[waveIndex];
}

void
BellMaker::setBasicWaveSource( index_t  waveIndex, BaseWaveform::Value  form ) {
	std::shared_ptr<SampleSource>  waveSource =  BasicWaveSource::create(form);

	if ( waveIndex < waves.size() ) {
		waves[waveIndex].setSampleSource( waveSource );
	}
}

void
BellMaker::removeWave( size_t  index ) {
	waves.erase( waves.begin() + index );
}

VolumeSpan&
BellMaker::getVolumeModifier() {
	return volumeSpan;
}

void
BellMaker::setSamplesPerSecond( index_t  sps ) {
	samplesPerSecond = sps;
	samplingStep = 1.0 / static_cast<play_t>(sps);
}

index_t
BellMaker::getSamplesPerSecond() const {
	return samplesPerSecond;
}

volume_t
BellMaker::fetchCompiledSample( play_t  time ) {
	volume_t  outputSample = 0;
	for ( Wave&  wave : waves ) {
		outputSample += wave.getSample(time);
	}
	return outputSample;
}

play_t
BellMaker::getMaxDuration() {
	play_t  t = 0;
	for ( Wave&  wave : waves ) {
		if ( wave.getDuration() > t )
			t = wave.getDuration();
	}
	return t;
}

void
BellMaker::serialize( IOInterface&  io ) {
	io.writeSection(IO_NAME);
	io.addIntAttribute("sample_rate", samplesPerSecond);
	io.endWriteAttributes();
	volumeSpan.serialize(io);
	for ( Wave&  wave : waves )
		wave.serialize(io);
	io.endWriteSection(IO_NAME);
}

void
BellMaker::deserialize( IOInterface&  io ) {
	reset();
	io.readSection(IO_NAME);
	samplesPerSecond = io.getAttributeAsInt("sample_rate");
	volumeSpan.deserialize(io);
	std::shared_ptr<SampleSource>  sampleSourcePtr;
	size_t  w = 0;
	const size_t waveCount = io.getChildNodeCount(Wave::IO_NAME);
	for (; w < waveCount; ++w ) {
		waves.push_back(Wave{});
		waves.back().setSampleSourceProvider(this);
		waves.back().deserialize(io);
	}
	io.endReadSection();
}

std::shared_ptr<SampleSource>
BellMaker::getSampleSource( unsigned  identifier ) {
	const BaseWaveform::Value  form = getWaveformFromIndex( static_cast<int>( identifier ) );
	return BasicWaveSource::create(form);
}

} // end namespace ding

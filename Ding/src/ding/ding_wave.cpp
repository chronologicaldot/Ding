// (c) 2019 Nicolaus Anderson

#include "ding_wave.h"
#include "ding_sample_source_provider.h"
#include <memory> // For std::make_shared

namespace ding {

Wave::Wave()
	: SampleSource()
	, sampleSourceProvider(nullptr)
	, sampleSource()
	, volumeMultiplier()
	, duration(1)
	, frequency(1)
	, phaseShift(0)
	, isInverted(false)
{}

Wave::~Wave()
{}

void
Wave::setDuration( play_t  time ) {
	duration = time;
}

play_t
Wave::getDuration() const {
	return duration;
}

void
Wave::setFrequency( play_t  cycles ) {
	frequency = cycles;
}

play_t
Wave::getFrequency() const {
	return frequency;
}

void
Wave::setPhaseShift( play_t  phase ) {
	phaseShift = phase;
}

play_t
Wave::getPhaseShift() const {
	return phaseShift;
}

void
Wave::invertWave( bool  setting ) {
	isInverted = setting;
}

bool
Wave::isInvertedWave() const {
	return isInverted;
}

void
Wave::setSampleSourceProvider( SampleSourceProvider*  provider ) {
	sampleSourceProvider = provider;
}

void
Wave::setSampleSource( std::shared_ptr<SampleSource>  source ) {
	sampleSource = source;
}

void
Wave::setVolumeMultiplier( std::shared_ptr<VolumeSpan>  multiplier ) {
	volumeMultiplier = multiplier;
}

std::shared_ptr<SampleSource>
Wave::getSampleSource() {
	return sampleSource;
};

std::shared_ptr<VolumeSpan>
Wave::getVolumeMultiplier() {
	return volumeMultiplier;
}

bool
Wave::hasSampleSource() const {
	return (sampleSource.get() ? true : false);
}

bool
Wave::hasVolumeMultiplier() const {
	return (volumeMultiplier.get() ? true : false);
}

void
Wave::initNewVolumeMultiplier() {
	volumeMultiplier = std::make_shared<VolumeSpan>();
}

volume_t
Wave::getReadiedSample( play_t  time ) {
	if ( time > duration || ! sampleSource.get() )
		return 0;

	return sampleSource.get()->getSample(time * frequency - phaseShift);
}

volume_t
Wave::getSampleUnmuted( play_t  time ) {
	if ( time > duration || ! volumeMultiplier.get() )
		return 0;

	volume_t  sample = getReadiedSample(time);
	volume_t  volume = volumeMultiplier.get()->getSample(time);
	volume = volume < 0 ? 0 : (volume > 1 ? 1 : volume);
	return sample * volume * (isInverted ? -1 : 1);
}

void
Wave::serialize( IOInterface&  io ) {
	io.writeSection(IO_NAME);
	io.addStringAttribute("name", Name.c_str());
	io.addDoubleAttribute("duration", duration);
	io.addDoubleAttribute("frequency", frequency);
	io.addDoubleAttribute("phase", phaseShift);
	//io.addBoolAttribute("inverted", isInverted); // Unnecessary with phase
	io.endWriteAttributes();
	if ( volumeMultiplier.get() ) {
		volumeMultiplier.get()->serialize(io);
	}
	if ( sampleSource.get() ) {
		sampleSource.get()->serialize(io);
	}
	io.endWriteSection(IO_NAME);
}

void
Wave::deserialize( IOInterface&  io ) {
	if ( ! io.readSection(IO_NAME) )
		return;
	Name = io.getAttributeAsString("name");
	duration = io.getAttributeAsDouble("duration");
	frequency = io.getAttributeAsDouble("frequency");
	phaseShift = io.getAttributeAsDouble("phase");
	//isInverted = io.getAttributeAsBool("inverted");
	volumeMultiplier = std::make_shared<VolumeSpan>();
	volumeMultiplier.get()->deserialize(io);
	if ( io.getChildNodeCount(SampleSource::IO_NAME) > 0 && sampleSourceProvider != nullptr ) {
		sampleSource = sampleSourceProvider->getSampleSource(0);
		sampleSource.get()->deserialize(io);
	}
	io.endReadSection();
}

} // end namespace ding

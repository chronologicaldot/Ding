// (c) 2019 Nicolaus Anderson

#include "ding_basic_wavesource.h"
#include "ding_wave_functions.h"

namespace ding {

BasicWaveSource::BasicWaveSource()
	: waveformType( BaseWaveform::flat )
{}

BasicWaveSource::BasicWaveSource( BaseWaveform::Value  form )
	: waveformType(form)
{}

void
BasicWaveSource::setWaveformType( BaseWaveform::Value  type ) {
	waveformType = type;
}

BaseWaveform::Value
BasicWaveSource::getWaveformType() const {
	return waveformType;
}

volume_t
BasicWaveSource::getSampleUnmuted( play_t  time ) {

	switch ( waveformType )
	{
	case BaseWaveform::sine:
		return  getSineWave(time);

	case BaseWaveform::cosine:
		return  getCosineWave(time);

	case BaseWaveform::saw:
		return  getSawWave(time);

	case BaseWaveform::fsaw:
		return  getAscendingSawWave(time);

	case BaseWaveform::bsaw:
		return  getDescendingSawWave(time);

	case BaseWaveform::square:
		return  getSquareWave(time);

	default: // flat
		return 1;
	}
}

void
BasicWaveSource::serialize( IOInterface&  io ) {
	io.writeSection(SampleSource::IO_NAME);
	io.addIntAttribute("formtype", (int)waveformType);
	io.endWriteAttributes();
	io.endWriteSection(SampleSource::IO_NAME);
}

void
BasicWaveSource::deserialize( IOInterface&  io) {
	io.readSection(SampleSource::IO_NAME);
	setWaveformType( (BaseWaveform::Value) io.getAttributeAsInt("formtype") );
	io.endReadSection();
}

} // end namespace ding

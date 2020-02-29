// (c) 2019 Nicolaus Anderson

#ifndef DING_WAVESOURCE_H
#define DING_WAVESOURCE_H

#include "ding_types.h"
#include "ding_waveform_types.h"
#include "ding_wave.h"
#include <memory> // For std::shared_ptr

namespace ding {

//! Basic Wave Source
/*
	A generator that provides different wave types depending on the setting.
	It can produce sine, cosine, saw, forward-saw, backward-saw, and square waves.
*/
struct BasicWaveSource
	: public SampleSource
{
	//! cstor
	BasicWaveSource();

	//! cstor
	BasicWaveSource( BaseWaveform::Value );

	//! Set Waveform Type
	void  setWaveformType( BaseWaveform::Value );

	//! Get the Waveform Type
	BaseWaveform::Value  getWaveformType() const;

	//! Create the Basic Wave Source in a Shared Pointer
	static std::shared_ptr<SampleSource>  create( BaseWaveform::Value  form ) {
		return std::make_shared<BasicWaveSource>(form);
	}

	//! Write to IO Interface
	virtual void  serialize( IOInterface& );

	//! Read from IO Interface
	virtual void  deserialize( IOInterface& );

protected:
	//! Returns a sample value at the given point in a cycle (1 cycle = 0->1, 1->2, etc)
	virtual volume_t  getSampleUnmuted( play_t );

private:
	BaseWaveform::Value  waveformType;
};

} // end namespace ding

#endif // DING_WAVESOURCE_H

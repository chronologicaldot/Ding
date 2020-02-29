// (c) 2019 Nicolaus Anderson

#ifndef DING_SAMPLE_SOURCE_H
#define DING_SAMPLE_SOURCE_H

#include "ding_types.h"
#include "ding_io_interface.h"
//#include <memory> // For shared_ptr

namespace ding {

//! Sample Source
/*
	A simple interface for returning samples.
*/
struct SampleSource
	: public IOSerializable
{
	static constexpr const char* const IO_NAME = "sample_source";

	SampleSource()
		: isSamplingUnmuted(true)
	{}

	//! Enabled/Disable the Sample Source
	void  setSamplingEnabled( bool  setting ) {
		isSamplingUnmuted = setting;
	}

	//! Is Sampling Enabled?
	bool  isSamplingEnabled() const {
		return isSamplingUnmuted;
	}

	//! Retrieve Sound Sample
	/*
		Should return the sample at the given playback in time.
		The zero of the given time must be the start of the wave.

		Not const so that buffers can be updated.
	*/
	volume_t  getSample( play_t  t ) {
		if ( isSamplingUnmuted )
			return getSampleUnmuted(t);

		return 0;
	}

protected:

	virtual volume_t  getSampleUnmuted( play_t ) = 0;

private:

	bool  isSamplingUnmuted;
};


//typedef  shared_ptr<SampleSource>  SampleSourcePtr;

} // end namespace ding

#endif // DING_SAMPLE_SOURCE_H

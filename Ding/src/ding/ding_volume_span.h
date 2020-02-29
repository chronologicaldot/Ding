// (c) 2019 Nicolaus Anderson

#ifndef DING_VOLUME_SPAN_H
#define DING_VOLUME_SPAN_H

#include "ding_types.h"
#include "ding_sample_source.h"
#include "ding_io_interface.h"

#define VOLUME_SPAN_SMOOTHING

#ifdef NO_VOLUME_SPAN_SMOOTHING
#undef VOLUME_SPAN_SMOOTHING
#endif

namespace ding {

//! Volume Span
/*
	This is a collection of volume settings that are sorted by time.
	A volumes at a particular time is created by checking the volume settings that
		bookend that particular time.
	By default, the volume at the time of interest is given as the interpolation of
		the volumes that bookend the time.
*/
struct VolumeSpan
	: public SampleSource
{
	static constexpr const char* const IO_NAME = "volume_span";

	//! cstor
	VolumeSpan();

	//! dstor
	~VolumeSpan();

	//! Get Volume Setting Information
	/*
		Attempts to return the Volume setting at the given index.
		If the index is out of bounds, it adds a new Volume.
		NOTE: Once this function is called, it is assumed that the Volume setting has been modified.
	*/
	Volume&  getSetting( index_t );

	//! Get const Volume Information
	/*
		Attempts to return the Volume at the given index.
		Throws an exception if the volume does not exist.
	*/
	const Volume&  getConstSettingInfo( index_t ) const;

	//! Return the number of volume modifiers herein
	index_t  getSettingCount() const;

	//! Delete all settings
	void  clearSettings();

	//! Set the volume of the Volume Setting
	/*
		This allows setting the volume of a particular setting without setting the dirty flag.
		However, if there is no volume setting at the given index, NO new setting is created.
	*/
	void  setVolumeSettingAtIndex( index_t, volume_t );

	//! Write to IO Interface
	virtual void  serialize( IOInterface& );

	//! Read from IO Interface
	virtual void  deserialize( IOInterface& );

#if 0
	void print();
#endif

	//! Use Bezier Curve Smoothing between points
	bool  SmoothingEnabled;

protected:
	//! Get the volume at the given time
	virtual volume_t  getSampleUnmuted( play_t );

	//! Sort volumes by time
	void  sortVolumeSettings();

#ifdef VOLUME_SPAN_SMOOTHING
	//! Smoothed Sample
	volume_t  getSmoothSample( play_t );

	//! Calculate Binomial Coefficients
	void  calculateCoefficients();

	//! Exponent of integer raising a double
	static double  di_pow( double, int );
#endif

	// Members ---------------------------
	list_t<Volume>  volumes;
	bool  volumesAreSorted;

#ifdef VOLUME_SPAN_SMOOTHING
	list_t<int>  binomialCoefs;
	bool  dirtyCoefs;
#endif
};

} // end namespace ding

#endif // DING_VOLUME_SPAN_H

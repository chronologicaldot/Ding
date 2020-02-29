// (c) 2019 Nicolaus Anderson

#ifndef DING_WAVEFORM_TYPES
#define DING_WAVEFORM_TYPES

namespace ding {

struct BaseWaveform {
	enum Value {
		//! Flat: A single value carried out at the same volume for all eternity
		flat = 0,

		//! Sine function
		sine,

		//! Cosine function
		cosine,

		//! Saw /\/\/\/
		saw,

		//! Saw-toothed forward/ascending /|/|/|/
		fsaw,

		//! Saw-toothed backward/descending \|\|\|
		bsaw,

		//! Square _|-|_|-|_|-
		square,

		//! NOT A VALUE - Just the number of options
		OPTIONS_COUNT
	};
};

inline BaseWaveform::Value
getWaveformFromIndex( int  typeIndex ) {
	switch( typeIndex )
	{
	case 1:
		return BaseWaveform::sine;

	case 2:
		return BaseWaveform::cosine;

	case 3:
		return BaseWaveform::saw;

	case 4:
		return BaseWaveform::fsaw;

	case 5:
		return BaseWaveform::bsaw;

	case 6:
		return BaseWaveform::square;

	default:
		return BaseWaveform::flat;
	}
}

} // end namespace ding

#endif // DING_WAVEFORM_TYPES

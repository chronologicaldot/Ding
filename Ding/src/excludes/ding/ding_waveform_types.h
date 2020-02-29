// (c) 2019 Nicolaus Anderson

#ifndef DING_WAVEFORM_TYPES
#define DING_WAVEFORM_TYPES

namespace ding {

struct BaseWaveform
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

//! Repeat Rule
/*
	This enumeration tells the constructors of the waveform how to build and repeat
	the wave pattern.
	By default ("none"), the samples end when the wave form creation function ends.
	Other types repeat the wave in cycles (the wave repeats when the samples run out)
	but perform a certain operation every time the wave cycles.
	For example, mirror_x performs two cycles - once with the samples in the forward direction
	and once in reverse.
*/
struct RepeatRule {
	enum Value {
		// No repetition - wave ends when the samples are done
		none = 0,

		// Every cycle, repeat the pattern
		repeat,

		// Perform forwards iteration then mirror across the x axis
		mirror_x,

		// Perform fowards iteration then mirror across the y axis
		mirror_y,

		// No flip, then flip x, then flip y, then flip x and y
		// Used for sine and cosine waves
		nfxfyfxy,
	};
};

} // end namespace ding

#endif // DING_WAVEFORM_TYPES

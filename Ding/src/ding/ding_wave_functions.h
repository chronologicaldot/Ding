// (c) 2019 Nicolaus Anderson

#ifndef DING_WAVE_FUNCTIONS_H
#define DING_WAVE_FUNCTIONS_H

#include "ding_types.h"

namespace ding {

//! Utility: clamp time to range 0 to 1
play_t  clampTime( play_t& );

/*
	All functions limit their time-axis range from 0 to 1
		and their volume range is limited from -1 to 1.
*/

//! Sine
volume_t  getSineWave( play_t );

//! Cosine
volume_t  getCosineWave( play_t );

//! Saw: \/\/\/
volume_t  getSawWave( play_t );

//! Saw: \/\/\/
volume_t  getSawWave( play_t );

//! Forward Saw: /|/|/|
volume_t  getAscendingSawWave( play_t );

//! Backward Saw: \|\|\|
volume_t  getDescendingSawWave( play_t );

//! Square: _|-|_|-
volume_t  getSquareWave( play_t );

} // end namespace ding

#endif

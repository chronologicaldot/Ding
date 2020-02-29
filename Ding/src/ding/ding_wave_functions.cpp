// (c) 2019 Nicolaus Anderson

#include "ding_wave_functions.h"
#include <cmath>

namespace ding {

play_t
clampTime( play_t&  time ) {
	double  f = floor( static_cast<double>(time) );
	time = time - f;
	return time;
}

volume_t
getSineWave( play_t  time ) {
	return static_cast<volume_t>( sin( (double)PI64 * 2 * clampTime(time) ) );
}

volume_t
getCosineWave( play_t  time ) {
	return static_cast<volume_t>( cos( (double)PI64 * 2 * clampTime(time) ) );
}

volume_t
getSawWave( play_t  time ) {
	time = clampTime(time);
	if ( time < 0.5 ) {
		// Y Range -1 to 1
		// Definition Points: (0,-1), (0.5,1)
		// 1-(-1) / (0.5 - 0) = 4
		// y= m(x-c) + b, c = 0, m = 4, b = -1
		return static_cast<volume_t>( time*4 - 1 );
	}
	else {
		// Range 1 to -1
		// Definition Points: (0.5,1), (1,-1)
		// -1-1 / 1-0.5 = -2/.5 = -4
		// y= m(x-c) + b, c = 0.5, m = -4, b = 1 ; -4*(time - 0.5) + 1
		return static_cast<volume_t>( 3. - time*4 );
	}
}

volume_t
getAscendingSawWave( play_t  time ) {
	// Range -1 to 1
	return static_cast<volume_t>( clampTime(time)*2 - 1 );
}

volume_t
getDescendingSawWave( play_t  time ) {
	// Range -1 to 1
	return static_cast<volume_t>( 1. - clampTime(time)*2 );
}

volume_t
getSquareWave( play_t  time ) {
	time = clampTime(time);
	if ( time < 0.5 ) {
		// Range -1 to 1
		return 1;
	}
	else {
		// Range 1 to -1
		return -1;
	}
}

} // end namespace ding

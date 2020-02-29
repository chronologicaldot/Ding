// (c) 2019 Nicolaus Anderson

#include <math.h>
#include "ding_build_waveforms.h"

namespace ding {

volume_t
getSineWave( progress_t  progress ) {
	return static_cast<volume_t>( sin( static_cast<double>(progress) ) / 2 + 0.5 );
}

volume_t
getCosineWave( progress_t  progress ) {
	return static_cast<volume_t>( cos( static_cast<double>(progress) ) / 2 + 0.5 );
}

volume_t
getSawWave( progress_t  progress ) {
	if ( progress < 0.5 ) {
		return static_cast<volume_t>( progress * 2 );
	}
	return static_cast<volume_t>(1) - progress * 2;
}

volume_t
getAscendingSawWave( progress_t  progress ) {
	return static_cast<volume_t>(1) / progress;
}

volume_t
getDescendingSawWave( progress_t  progress ) {
	return - static_cast<volume_t>(1) / progress;
}

volume_t
getSquareWave( progress_t  progress ) {
	if ( progress < 0.5 )
		return 1;

	return 0;
}


bool
createSineWaveFirstQuarter( volume_t*  buffer, index_t  buffer_size ) {
	if ( !buffer ) return false;

	index_t  i = 0;
	progress_t  angle_increment = static_cast<progress_t>( PI64 / (2 * buffer_size) );
	for (; i < buffer_size; ++i)
		buffer[i] = getSineWave(angle_increment * i);

	return true;
}

bool
createCosineWaveFirstQuarter( volume_t*  buffer, index_t  buffer_size ) {
	if ( !buffer ) return false;

	index_t  i = 0;
	progress_t  angle_increment = static_cast<progress_t>( PI64 / (2 * buffer_size) );
	for (; i < buffer_size; ++i)
		buffer[i] = getCosineWave(angle_increment * i);

	return true;
}

bool
createSawWave( volume_t*  buffer, index_t  buffer_size ) {
	if ( !buffer ) return false;

	index_t  i = 0;
	volume_t  rise = volume_t(2) / buffer_size;
	for (; i < buffer_size / 2; ++i)
		buffer[i] = rise * i;

	for (; i < buffer_size; ++i)
		buffer[i] = static_cast<volume_t>(1) - rise * i;
}

bool
createAscendingSawWave( volume_t*  buffer, index_t  buffer_size ) {
	if ( !buffer ) return false;

	index_t  i = 0;
	volume_t  rise = volume_t(1) / buffer_size;
	for (; i < buffer_size; ++i)
		buffer[i] = rise * i;

	return true;
}

bool
createDescendingSawWave( volume_t*  buffer, index_t  buffer_size ) {
	if ( !buffer ) return false;

	index_t  i = 0;
	volume_t  rise = volume_t(1) / buffer_size;
	for (; i < buffer_size; ++i)
		buffer[i] = static_cast<volume_t>(1) - rise * i;

	return true;
}

bool
createSquareWave( volume_t*  buffer, index_t  buffer_size ) {
	if ( !buffer ) return false;

	index_t  i = 0;
	for (; i < buffer_size / 2; ++i)
		buffer[i] = 1;

	for (; i < buffer_size; ++i)
		buffer[i] = 0;

	return true;
}


} // end namespace ding

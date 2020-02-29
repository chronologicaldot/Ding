// (c) 2019 Nicolaus Anderson

#ifndef DING_TYPES_H
#define DING_TYPES_H

// If std::vector is used, this must be kept
#include <vector>
#include <algorithm> // For std::sort


namespace ding {


// Guaranteed types
typedef  double  deci_t;
typedef  double  play_t; // Playback time in decimal 32-bit. Could be changed to C++11 type
typedef  double  progress_t; // Range of 0 to 1 or 0 to 2pi

//--------------------------------------------
// Types whose type is NOT guaranteed
typedef  unsigned long  playback_time_t;
typedef  double  volume_t;
typedef  unsigned  index_t;

// C++11 type definitions
template<class T>
using  list_t = std::vector<T>;

// NOTE: This is illegal:
// typedef std::vector  list_t;


//--------------------------------------------
// Constants

//! Pi
static const play_t PI64 = 3.1415926535897932384626433832795028841971693993751;

//! The number of increments of an instance of playback_time_t to reach a full second
//! (An increment being:  playback_time_t  i=0; i += 1;)
static const play_t  PLAYBACK_TICKS_PER_SECOND = 10000000;

//! Default samples per second
static const play_t  DEFAULT_SAMPLES_PER_SECOND = 24;

//--------------------------------------------
// Exceptions
struct IndexOutOfBoundsException
{
	index_t  index;
};


//--------------------------------------------
//! Volume Setting
/*
	This is used for setting the volume of either the bell or its waveforms at a particular time.
	In most cases, the volume will be interpolated between.
*/
struct Volume
{
	playback_time_t  time;
	volume_t  volume;

	Volume()
		: time(0)
		, volume(1) // 1 == full scale
	{}

	bool operator < (const Volume  rhs) const { return time < rhs.time; }
};

} // end namespace ding

#endif // DING_TYPES_H

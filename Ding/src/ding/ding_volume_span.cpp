// (c) 2019 Nicolaus Anderson

#include "ding_volume_span.h"

#ifdef VOLUME_SPAN_SMOOTHING
//#include <math.h>
#endif

#if 1
#include <cstdio>
#endif

namespace ding {

VolumeSpan::VolumeSpan()
	: SampleSource()
	, SmoothingEnabled(false)
	, volumes()
	, volumesAreSorted(false)
#ifdef VOLUME_SPAN_SMOOTHING
	, binomialCoefs()
	, dirtyCoefs(true)
#endif
{}

VolumeSpan::~VolumeSpan()
{}

Volume&
VolumeSpan::getSetting( index_t  index ) {
	volumesAreSorted = false;
	if ( index >= volumes.size() ) {
		volumes.push_back( Volume{} );
		return volumes.back();
	}
	return volumes[index];
}

const Volume&
VolumeSpan::getConstSettingInfo( index_t  index ) const {
	if ( index >= volumes.size() )
		throw IndexOutOfBoundsException{index};

	return volumes[index];
}

index_t
VolumeSpan::getSettingCount() const {
	return static_cast<index_t>( volumes.size() );
}

void
VolumeSpan::clearSettings() {
	volumes.clear();
}

void
VolumeSpan::setVolumeSettingAtIndex( index_t  index, volume_t  volume ) {
	if ( index >= volumes.size() )
		return;

	volumes[index].volume = volume;
}

#if 0
void
VolumeSpan::print() {
	std::printf("Printing volume span: \n");
	for ( Volume&  v : volumes ) {
		std::printf("vh = [time = %f, volume = %f]\n", v.time, v.volume);
	}
}
#endif

volume_t
VolumeSpan::getSampleUnmuted( play_t  time ) {
	if ( volumes.size() == 0 )
		return 1;

	if ( volumes.size() == 1 )
		return volumes[0].volume;

	if ( ! volumesAreSorted )
		sortVolumeSettings();

#ifdef VOLUME_SPAN_SMOOTHING
	if ( volumes.size() >= 3 && SmoothingEnabled )
		return getSmoothSample(time);
#endif

	// Calculate the volume by interpolating between the two volumes that bookend the
	// sample at the time of interest. Default to the first volume setting.
	volume_t  first = volumes[0].volume;
	volume_t  second = first;
	play_t  timeDelta = 0;
	play_t  timeSpan = 1; // Prevent division by zero
	volume_t  slope = 0;

		// Finding the volumes
	index_t  v = 1;
	for (; v < volumes.size(); ++v) {
		if ( v == volumes.size() - 1 && volumes[v].time < time ) // Last
		{
			first = volumes[v].volume;
			slope = 0;
			break;
		}

		if ( time <= volumes[v].time && time > volumes[v - 1].time )
		{
			first = volumes[v - 1].volume;
			second = volumes[v].volume;
			timeDelta = time - volumes[v - 1].time;
			timeSpan = volumes[v].time - volumes[v - 1].time;
			slope = (second - first) / timeSpan;
			break;
		}
	}

	return first + slope * timeDelta;
}

void
VolumeSpan::sortVolumeSettings() {
	// At the moment, we assume std::vector, but that may be changed in the future.
	std::sort( volumes.begin(), volumes.end() );
	volumesAreSorted = true;
	dirtyCoefs = true;
}

#ifdef VOLUME_SPAN_SMOOTHING
volume_t
VolumeSpan::getSmoothSample( play_t  time ) {
	if ( dirtyCoefs )
		calculateCoefficients();

	// B(t) = sum[k=0->n]( coef(k) * (1-t)^(n-k) * t^k * P(k) )
	// Where P(k) is the k_th point, t is path traversal (0->1), n is the number of points.
	// Since we cannot get the actual time value (because bezier's can overlap and may have complex roots)
	// we don't fight. The volume points define the whole curve rather than actual volume at a specific time.
	// Hence, we use volumes[k].volume == P(k).y
	const int  n = volumes.size() - 1; // This works. Don't touch it.
	const double  maxTime = volumes[n].time;
	if ( maxTime == 0 ) return 0;
	const double  t = time / maxTime;
	int  k;
	volume_t  b = 0;
	// First end, noting t^0==1
	b += volumes[0].volume * di_pow( 1-t, n );
	for (k = 1; k < n; ++k) {
		b += volumes[k].volume * binomialCoefs[k-1] * di_pow( 1-t, n-k ) * di_pow( t, k );
	}
	// Last end, noting (1-t)^(n-n)==(1-t)^0==1
	b += volumes[n].volume * di_pow( t, n );
	return b;
}

void
VolumeSpan::calculateCoefficients() {
	if ( volumes.size() < 3 ) return;

	const int  n = volumes.size() - 1;
	int  k = 1;
	int  i = 1;
	int  t = 1;

	// Range of k is 0->n inclusive
	// Only calculate the middle terms because the ends == 1
	binomialCoefs.clear();
	binomialCoefs.reserve(n-2); // Do NOT use resize()

	for (; k < n; ++k) {
		t = 1;
		for (i=1; i <= k; ++i) {
			t *= (n + 1 - i);
		}
		// Since division by integer results in zero when denominator > numerator, we save division for last
		for (i=1; i <= k; ++i) {
			t /= i;
		}
		//binomialCoefs[k-1] = t;
		binomialCoefs.push_back(t);
	}

	dirtyCoefs = false;	
}

double
VolumeSpan::di_pow( double  base, int  power ) {
	if ( power == 0 )
		return 1;
	if ( power == 1 )
		return base;
	const int  odd = power % 2;
	const int  half = power / 2; // Drops the remainder
	int  i = 1;
	double  accu = base;
	while ( i < half ) {
		accu *= base;
		++i;
	}
	accu *= accu;
	return odd == 0 ? accu : accu * base;
}

#endif

void
VolumeSpan::serialize( IOInterface&  io ) {
	io.writeSection(IO_NAME);
	io.addBoolAttribute("smooth", SmoothingEnabled);
	io.endWriteAttributes();
	for ( Volume&  v : volumes ) {
		v.serialize(io);
	}
	io.endWriteSection(IO_NAME);
}

void
VolumeSpan::deserialize( IOInterface&  io ) {
	volumes.clear();
	io.readSection(IO_NAME);
	SmoothingEnabled = io.getAttributeAsBool("smooth");
#ifdef VOLUME_SPAN_SMOOTHING
	dirtyCoefs = true;
#endif
	size_t  v = 0;
	const size_t  handleCount = io.getChildNodeCount(Volume::IO_NAME);
	for (; v < handleCount; ++v) {
		volumes.push_back(Volume{});
		volumes.back().deserialize(io);
	}
	io.endReadSection();
}

} // end namespace ding

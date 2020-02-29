// (c) 2019 Nicolaus Anderson

#ifndef DING_SAMPLE_SOURCE_PROVIDER
#define DING_SAMPLE_SOURCE_PROVIDER

#include "ding_sample_source.h"
#include <memory> // For std::shared_ptr

namespace ding {

struct SampleSourceProvider
{
	virtual std::shared_ptr<SampleSource>
	getSampleSource( unsigned  identifier ) = 0;
};

} // end namespace ding

#endif // DING_SAMPLE_SOURCE_PROVIDER

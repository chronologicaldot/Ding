// (C) 2019 Nicolaus Anderson

#ifndef DING_SAMPLE_FEED_H
#define DING_SAMPLE_FEED_H

#include "ding_types.h"

namespace ding {

//! Sample Feed
/*
	Used as a source for samples needing to be accessed by index.
*/
struct SampleByIndexFeed
{
	virtual volume_t  getSampleByIndex( index_t ) = 0;
	virtual index_t  getMaxSamplesProvided() = 0;
};

}

#endif

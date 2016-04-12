
#pragma once

#include <opencv2/core/core.hpp>
#include "Samples.h"
#include "Sampler.h"

namespace sal {

/*
\brief Class to generate samples.
Special to saliency detection algorithm:
- samples are in a small neighborhood
That is, TSamples ensures that samples are quads of locations,
but this further ensures that they are close together

ImageSaliencyDetector can use this directly,
or via a SamplePool which uses this.

Valid calling sequence:  getSample
 */
class Sampler {
public:
	explicit Sampler(
			int imageHeight,
			int imageWidth,
			int neighborhoodSize,
			int requiredSampleCount);

	virtual ~Sampler();

	// Gets a valid sample
	TSamples getSample();

	// Gets possibly invalid sample
	TSamples getCandidateSample();
	bool isSampleInImageBounds(TSamples&);

private:

	int imageHeight;
	int imageWidth;
	int neighborhoodSize;
	int halfNHood;
	int requiredSampleCount;
};

} // namespace

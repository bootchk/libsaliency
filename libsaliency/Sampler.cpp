
#include <opencv2/core/core.hpp>

#include "Sampler.h"

namespace sal {


Sampler::Sampler(
	int imageHeight,
	int imageWidth,
	int neighborhoodSize,
	int requiredSampleCount)
{
	this->imageHeight = imageHeight;
	this->imageWidth = imageWidth;
	this->neighborhoodSize =  neighborhoodSize;
	this->halfNHood = neighborhoodSize / 2;
	this->requiredSampleCount = requiredSampleCount;
}

Sampler::~Sampler() { }

TSamples Sampler::getSample() {

	TSamples result;
	do {
		result = getCandidateSample();
	}
	while (not isSampleInImageBounds(result));

	// assert result samples in bounds
	return result;
}

// Private


TSamples Sampler::getCandidateSample() {

	// TODO pass by address?
	// Allocate container for sample points
	TSamples result(COUNT_SAMPLE_POINTS);

	// Randomly select the location of the first sample
	result[0].y = rand() % imageHeight;
	result[0].x = rand() % imageWidth;

	// The other 3 samples MUST be selected in the neighborhood of the first
	result[1].y = (rand() % neighborhoodSize) + (result[0].y - halfNHood);
	result[1].x = (rand() % neighborhoodSize) + (result[0].x - halfNHood);

	result[2].y = (rand() % neighborhoodSize) + (result[0].y - halfNHood);
	result[2].x = (rand() % neighborhoodSize) + (result[0].x - halfNHood);

	result[3].y = (rand() % neighborhoodSize) + (result[0].y - halfNHood);
	result[3].x = (rand() % neighborhoodSize) + (result[0].x - halfNHood);

	// !!! Not assert that all locations are in image
	return result;
}


// Are sample locations within valid image boundaries?
bool Sampler::isSampleInImageBounds(TSamples& samples) {
	assert(samples.size() == COUNT_SAMPLE_POINTS);

	bool isValid = true;

	for (size_t i = 0; i < COUNT_SAMPLE_POINTS; ++i) {
		if (samples[i].y < 0 || samples[i].y >= imageHeight || samples[i].x < 0 || samples[i].x >= imageWidth) {
			isValid = false;
		}
	}

	return isValid;
}


} // namespace

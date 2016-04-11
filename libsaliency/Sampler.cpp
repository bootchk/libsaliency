
#include <opencv2/core/core.hpp>
#include "Sampler.h"

namespace sal {


Sampler::Sampler(
	int imageHeight,
	int imageWidth,
	int neighborhoodSize)
{
	this->imageHeight = imageHeight;
	this->imageWidth = imageWidth;
	this->neighborhoodSize =  neighborhoodSize;
	this->halfNHood = neighborhoodSize / 2;

}

Sampler::~Sampler() { }


TSamples Sampler::getSample() {
	TSamples result(4);

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

	return result;
}

} // namespace

#include <cstdio>
#include "Samples.h"
#include "Sampler.h"
#include "SamplePool.h"


SamplePool::SamplePool() {
	nextSampleIndex = 0;
}

SamplePool::~SamplePool() { }

// If you don't want to pre generate the pool, change fillWithValidSamples to just remember the params
// and make getNextSample just return sampler.getSample().

void SamplePool::fillWithValidSamples(
		int imageHeight,
		int imageWidth,
		int neighborhoodSize,
		int requiredSampleCount) {

	sal::Sampler sampler = sal::Sampler(imageHeight, imageWidth, neighborhoodSize, requiredSampleCount);

	samplePool.reserve(requiredSampleCount);

	for (int i=0; i<requiredSampleCount; i++){
			TSamples samples = sampler.getSample();
			samplePool.push_back(samples);
	}
	assert(samplePool.size() == requiredSampleCount);

	// Sorting optional
	// From experiments, this just shifts work from the iterative step to sorting step,
	// without decreasing total runtime.
	this->sort();

	nextSampleIndex = 0;
}

TSamples SamplePool::getNextSample() {
	TSamples result = samplePool[nextSampleIndex];
	nextSampleIndex++;
	// printf(" %i  %i\n", result[0].x, result[0].y);
	return result;
}


// Private

// Class that knows how to compare TSamples
struct TSamplesXGreater
{
    bool inline operator()( const TSamples& lx, const TSamples& rx ) const {
    	return lx[0].x < rx[0].x;
    }
};
struct TSamplesYGreater
{
    bool inline operator()( const TSamples& lx, const TSamples& rx ) const {
    	return lx[0].y < rx[0].y;
    }
};


void SamplePool::sort() {
	// Hides the algorithm and predicate function

	// Sort y then x, yielding 1,1  1,2  1,3  2,1, etc
	//std::stable_sort(samplePool.begin(), samplePool.end(), TSamplesYGreater());
	//std::stable_sort(samplePool.begin(), samplePool.end(), TSamplesXGreater());

	// Alternative: just sort on x, not stable
	std::sort(samplePool.begin(), samplePool.end(), TSamplesXGreater());
}

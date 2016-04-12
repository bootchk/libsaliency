
#pragma once

#include "Samples.h"	// TSample

// Wrapper around Sampler.  Uses sampler.
// The pool is sorted for better memory locality (fewer cache misses), maybe.
// This precomputes samples into a pool from which samples are generated.
// valid calling sequence:  fillWithValidSamples, getNextSample, ...


class SamplePool {

public:
	explicit SamplePool();
	virtual ~SamplePool();
	void fillWithValidSamples(
			int imageHeight,
			int imageWidth,
			int neighborhoodSize,
			int requiredSampleCount);
	TSamples getNextSample();

private:
	int nextSampleIndex;
	std::vector<TSamples> samplePool;

	void sort();
};

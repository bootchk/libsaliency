
#pragma once

#include <array>

/*
Compile time constants that limit this algorithm.
Referenced by several class definitions.
*/

/*
Max image channels this algorithm supports.

Actual channel count from source image.
Allow for future RGBAlpha or RGBDepth
Usually actual channel count is 1:grayscale or 3:color

See also definitions of Vec4f not covered by this constant
*/
//TODO channel count 5 for RGBA
#define MAX_CHANNEL_COUNT 4

/*
A sample is a pair of coords.  But this defines that we take two samples (total of four coords.)
*/
#define COUNT_SAMPLES 2

// TODO extract this from Samples.h
// #define COUNT_SAMPLE_COORDS 4


namespace sal
{

/// Used to handle special cases which can produce errors (such as numbers below 1e-15)
enum {ERROR_FLAG = -1};

// weights for each sample pair x channel
// typedef std::array<std::array<float, MAX_CHANNEL_COUNT>, COUNT_SAMPLES> SampleChannelWeights;
typedef std::array<float, COUNT_SAMPLES> SampleWeights;

/*
Packet of info.

Used in:
- densityEstimate is an image sized array of this.
- computed iteratively(for each sample) and summed to densityEstimate.

Some fields are accumulators for iterated results.
The field 'entropy' (when this is a densityEstimate) is a result,
from which saliency map is created.
*/
// TODO this could be two subclasses: each use does not use all fields
class KernelDensityInfo {
public:
	KernelDensityInfo();

	static void initClass(int channelCount);	// class method

	void init();

	/*!
	* Updates the entropy of a pixel given the iteratively-estimated distribution of
	* the distance and orientation relationships around it
	 */

	// Update calculated fields of self. Called periodically during iteration over samples.
	// void updatePixelEntropy(int channelCount);

	// Sum iterative KDI into densityEstimate KDI
	void sumOtherWeightsIntoSelf(const KernelDensityInfo& other);
	float productOfWeights();
	void sumSampleResult( const KernelDensityInfo& sampleResult);

	// Estimated entropy of a pixel in relation to its neighbors
	// Computed at least at end of sampling.
	float entropy();

	// Class var.  !!!! Must be defined in global scope elsewhere.
	static int channelCount;


	// Data

	// kernelSum and weights are intermediate contributes of sample iterations
	float kernelSum;

	SampleWeights weights;

// private:
	int sampleCount; /// Count of samples contributing to the estimate

};

}

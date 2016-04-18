
#pragma once

namespace sal
{

/// Used to handle special cases which can produce errors (such as numbers below 1e-15)
enum {ERROR_FLAG = -1};

// weights for each sample pair x channel
typedef std::array<std::array<float, 3>, 2> SampleChannelWeights;


// Packet of info.
// densityEstimate is an image sized array of this.
// Packet is computed iteratively(for each sample) and summed to densityEstimate.
class KernelDensityInfo {
public:
	KernelDensityInfo();

	void init();
	// Update calculated fields of self. Called periodically during iteration over samples.
	void updatePixelEntropy(int channelCount);

	// Sum iterative KDI into densityEstimate KDI
	void sumOtherWeightsIntoSelf(const KernelDensityInfo& other, int channelCount);

	float productOfWeights(int channelCount);

	// Data

	float kernelSum;
	float entropy;	/// The entropy info for a pixel in relation to its neighbors
	SampleChannelWeights weights;
	//float firstWeight;
	//float secondWeight;
	int sampleCount; /// The number of samples used to estimate the kernel density

};

}

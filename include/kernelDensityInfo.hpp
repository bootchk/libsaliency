
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

	/*!
		 * Updates the entropy of a pixel given the iteratively-estimated distribution of
		 * the distance and orientation relationships around it
		 */

	// Update calculated fields of self. Called periodically during iteration over samples.
	void updatePixelEntropy(int channelCount);

	// Sum iterative KDI into densityEstimate KDI
	void sumOtherWeightsIntoSelf(const KernelDensityInfo& other, int channelCount);

	float productOfWeights(int channelCount);

	void sumKernelResult(
			const KernelDensityInfo& kernelResult,
			int channelCount);

	// Data

	// kernelSum and weights are intermediate contributes of sample iterations
	float kernelSum;

	SampleChannelWeights weights;
	//float firstWeight;
	//float secondWeight;

	int sampleCount; /// Count of samples contributing to the estimate

	// This is the estimate, the result
	float entropy;	/// The entropy info for a pixel in relation to its neighbors

};

}

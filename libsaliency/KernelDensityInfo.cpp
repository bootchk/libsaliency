
#include <cmath>
#include <array>
#include <cassert>

#include "kernelDensityInfo.hpp"

/*
 * Note when this class is not nested in SaliencyDetector, you can't inline the functions.
 * Formerly was nested.
 * That might be faster?
 */
namespace sal
{

// Define class var in global scope
int KernelDensityInfo::channelCount;	// Initialized later


// Constructor calls init, but an instance can be reinitialized
KernelDensityInfo::KernelDensityInfo() {
	sampleCount = 0;
	init();
};

void KernelDensityInfo::initClass(int channelCount)
{
	// Fixed channelCount for all instances
	KernelDensityInfo::channelCount = channelCount;
}

// For an instance used and reused for sample results, clear fields reused: kernelSum and weights
void KernelDensityInfo::init() {
	kernelSum = 0.f;
	//entropy = 0.f;
	//firstWeight(0.f),
	//secondWeight(0.f),

	// Init max count of channels, even if actual channels is less
	// compiler will unroll?
	for(int i=0; i<2; i++) {
		weights[i] = 0.f;
		/*
		for(int j=0; j<MAX_CHANNEL_COUNT; j++) {
			weights[i][j] = 0.f;
		}
		*/
	}
}


// These are in the innermost loop, inline if possible.

float KernelDensityInfo::productOfWeights() {
	float result = 1.0f;
	for(int i=0; i<2; i++) {
		result *= weights[i];
		/*
		for(int j=0; j<KernelDensityInfo::channelCount; j++) {
			result *= weights[i][j];
		}
		*/
	}
	return result;
}

void KernelDensityInfo::sumOtherWeightsIntoSelf(const KernelDensityInfo& other) {
	for(int i=0; i<2; i++) {
		weights[i] += other.weights[i];
		/*
		for(int j=0; j<KernelDensityInfo::channelCount; j++) {
			weights[i][j] += other.weights[i][j];
		}
		*/
	}
}

/*
lkk: Notes about NaN, from Wiki:
NaN is an undefined result for certain operations:
1.  One operand is already NaN
2.  Indeterminate operations such as multiplication by infinity
3.  Yielding complex results such as sqrt(-1)
NaN is NOT a result of underflow or overflow.

It seems to me that you could carefully avoid NaN by thinking carefully about all earlier operations.
For example, checking for overflow and underflow (yielding infinity) earlier.
Maybe it is just more convenient to catch those 'programming errors' here.
*/
/*
void KernelDensityInfo::updatePixelEntropy(int channelCount) {
	// assert self is a densityEstimate
	if (sampleCount > 0) {

		// Product of weights across samples AND across channels
		// doesn't seem to be correct for color
		float totalWeight = 1.f;		// !!! Start with identity
		for(int i=0; i<2; i++) {
			for(int j=0; j<channelCount; j++) {
				totalWeight *= weights[i][j];
			}
		}
		// Original code:
		// float totalWeight = kernelInfo.firstWeight * kernelInfo.secondWeight;
		// IOW product between pixel pairs

		float estimation = 0.f;

		// Special case: avoid division by 0
		if (totalWeight <= 0) {
			totalWeight = static_cast<float>(sampleCount);
		}

		if (kernelSum < 0 || isnan(kernelSum)) {
			kernelSum = 0;
		}

		estimation = kernelSum / totalWeight;

		// Another special case: if the calculated values are -ve or NaNs
		if (estimation <= 1e-15) {
			entropy = ERROR_FLAG;
		} else if (isnan(estimation)) {
			entropy = ERROR_FLAG;
		} else {
			// Note estimation, and also entropy, are not functions of their own previous values.
			// They are functions of kernelSum and weights
			// I don't understand why we periodically calculate entropy
			// instead of just calculating it at the end of iteration.
			entropy = -1.0f * log2f(estimation * estimation);
		}
	}
}
*/

// Sum an iterative result to self
// Assert self is a cumulative result, i.e. a densityEstimate
void KernelDensityInfo::sumSampleResult( const KernelDensityInfo& sampleResult)
{
	kernelSum += sampleResult.kernelSum;
	//densityEstimates[row][col].firstWeight += sampleResult.firstWeight;
	//densityEstimates[row][col].secondWeight += sampleResult.secondWeight;
	sumOtherWeightsIntoSelf(sampleResult);
	this->sampleCount++;
}


// Compute final result.
// Final means done iterating.
// Wrap iterated result (V(p) in the paper)  with factors not iterated (minus log2).
// See paper:

float KernelDensityInfo::entropy() {
	// assert self is a densityEstimate accumulating from samples

	float result;
	if (sampleCount <= 0)
	{
		// This should be rare, unless sampling percentage is small.
		// If this occurs in a burst at the end of the image,
		// there is likely a bug somewhere e.g. random samples wrong or bounds wrong.
		printf("Sample count zero\n");
		result = 0;
	}
	else
	{
		// printf("Sample count non zero %i \n", sampleCount);
		float totalWeight = 1.f;		// !!! Start with identity
		for(int i=0; i<2; i++) {
			totalWeight *= weights[i];
			/*
			// Product of weights across samples AND across channels
			for(int j=0; j<KernelDensityInfo::channelCount; j++) {
				totalWeight *= weights[i][j];
			}
			*/
		}
		// Original code:
		// float totalWeight = kernelInfo.firstWeight * kernelInfo.secondWeight;
		// IOW product between pixel pairs



		// Special case: avoid division by 0
		if (totalWeight <= 0) {
			totalWeight = static_cast<float>(sampleCount);
		}

		// TODO assert(kernelSum >= 0);
		// TODO if isnan(kernelSum) is handled below also
		if (kernelSum < 0 || isnan(kernelSum)) {
			kernelSum = 0;
		}

		float ratioKernelSumToWeightSum = kernelSum / totalWeight;
		assert(ratioKernelSumToWeightSum <1);

		// Another special case: if the calculated values are -ve or NaNs
		if (ratioKernelSumToWeightSum <= 1e-15) {
			result = ERROR_FLAG;
		} else if (isnan(ratioKernelSumToWeightSum)) {
			result = ERROR_FLAG;
		} else {
			// Note result is not functions of their own previous values.
			// Is function of kernelSum and weights
			result = -1.0f * log2f(ratioKernelSumToWeightSum * ratioKernelSumToWeightSum);
		}
		// printf("Entropy %f kernel %f weight %f \n", result, kernelSum, totalWeight);
	}
	assert(("Entropy is positive or ERROR_FLAG", result>=0 or result == ERROR_FLAG));
	return result;
}

}	// namespace

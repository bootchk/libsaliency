
#include <cmath>
#include <array>

#include "constants.h"
#include "kernelDensityInfo.hpp"

/*
 * Note when this class is not nested in SaliencyDetector, you can't inline the functions.
 * Formerly was nested.
 * That might be faster?
 */
namespace sal
{

// Constructor calls init
KernelDensityInfo::KernelDensityInfo() {
	init();
};

void KernelDensityInfo::init() {
	kernelSum = 0.f;
	entropy = 0.f;
	sampleCount = 0;
	//firstWeight(0.f),
	//secondWeight(0.f),

	// Init max count of channels, even if actual channels is less
	// compiler will unroll?
	for(int i=0; i<2; i++) {
		for(int j=0; j<MAX_CHANNEL_COUNT; j++) {
			weights[i][j] = 0.f;
		}
	}
}


// These are in the innermost loop, inline if possible.

float KernelDensityInfo::productOfWeights(int channelCount) {
	float result = 1.0f;
	for(int i=0; i<2; i++) {
		for(int j=0; j<channelCount; j++) {
			result *= weights[i][j];
		}
	}
	return result;
}

void KernelDensityInfo::sumOtherWeightsIntoSelf(const KernelDensityInfo& other, int channelCount) {
	for(int i=0; i<2; i++) {
		for(int j=0; j<channelCount; j++) {
			weights[i][j] += other.weights[i][j];
		}
	}
}

/*
lkk: Notes about NaN, from Wiki:
NaN is an undefined result for certain operations:
1.  One operand is already NaN
2.  Indeterminate operations such as mulitplication by infinity
3.  Yielding complex results such as sqrt(-1)
NaN is NOT a result of underflow or overflow.

It seems to me that you could carefully avoid NaN by thinking carefully about all earlier operations.
For example, checking for overflow and underflow (yielding infinity) earlier.
Maybe it is just more convenient to catch those 'programming errors' here.
*/
void KernelDensityInfo::updatePixelEntropy(int channelCount) {
	// assert self is a densityEstimate
	if (sampleCount > 0) {

		// Product of weights across samples AND across channels
		// TODO doesn't seem to be correct for color
		float totalWeight = 1;		// !!! Start with identity
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


// Sum an iterative result to self
// Assert self is a cumulative result, i.e. a densityEstimate
void KernelDensityInfo::sumKernelResult(
		const KernelDensityInfo& kernelResult,
		int channelCount)
{
	kernelSum += kernelResult.kernelSum;
	//densityEstimates[row][col].firstWeight += kernelResult.firstWeight;
	//densityEstimates[row][col].secondWeight += kernelResult.secondWeight;
	sumOtherWeightsIntoSelf(kernelResult, channelCount);
	sampleCount++;
}

}

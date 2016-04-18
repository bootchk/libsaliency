
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

void KernelDensityInfo::updatePixelEntropy(int channelCount) {
	// assert self is a densityEstimate
	if (sampleCount > 0) {

		float totalWeight = 0;
		for(int i=0; i<2; i++) {
			for(int j=0; j<channelCount; j++) {
				totalWeight *= weights[i][j];
			}
		}
		// float totalWeight = kernelInfo.firstWeight * kernelInfo.secondWeight;
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
			entropy = -1.0f * log2f(estimation * estimation);
		}
	}
}

}

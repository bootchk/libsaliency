
#include <opencv2/core/core.hpp>

#include <kernelDensityInfo.hpp>
#include "PDFEstimate.hpp"

namespace sal {

PDFEstimate::PDFEstimate() { }
PDFEstimate::~PDFEstimate() { }


void PDFEstimate::resizeTo(cv::Size inImageSize, const int channelCount, const int neighborhoodSize )
{
	// densityEstimate per pixel, not channel
	densityEstimates.resize(inImageSize.height);
	for (int row = 0; row < inImageSize.height; ++row) {
		densityEstimates[row].resize(inImageSize.width);
	}
	// assert densityEstimates size equals inImageSize, and elements are initialized KernelDensityInfo
	// (that is what std::vector.resize() does.)

	height = inImageSize.height;
	width = inImageSize.width;
	this->channelCount = channelCount;

	// Parameter of the estimation algorithm
	int sampleCountLimit = (2 * neighborhoodSize - 1) * (2 * neighborhoodSize - 1);
}

bool PDFEstimate::isSane()
{
	assert(!densityEstimates.empty());
	assert(!densityEstimates[0].empty());
	assert(densityEstimates[0][0].sampleCount >= 0);
	return true;
}

void PDFEstimate::updateApplicableRegion(const cv::Rect& bounds, const KernelDensityInfo& kernelSum)
{
	// Require initialized densityEstimates
	assert(isSane());

	// Bounds define an aligned rect
	// Bounds are clamped to image bounds.
	// I.E. assert bounding rect contained in image
	// Also assert bounding rect is square of dimension neighborhoodSize

	// Iterate over coordinates of aligned bounding rect
	for (int row = bounds.y; row < bounds.y + bounds.height; row++) {
		//Original: for (int j = bounds.topLeft.x; j <= bounds.topRight.x; j++) {
		for (int col = bounds.x; col < bounds.x + bounds.width; col++) {
			// Assert coords are strictly in range
			// C++ array access using [] operator does not check.
			assert (row < height and col < width);
			if (densityEstimates[row][col].sampleCount < sampleCountLimit) {
				densityEstimates[row][col].sumKernelResult(kernelSum, channelCount);

				/*
				Update pixel entropy every N (= 32) sample contributions.
				Only some pixels in the region will get updates
				(they all have their own sampleCount of contributions.)

				lkk I don't understand this, must be something to do with numerical analysis losses.
				Maybe:  sums get large and lose precision?
				 */
				if (((densityEstimates[row][col].sampleCount + 1) % 32) == 0) {
					densityEstimates[row][col].updatePixelEntropy(channelCount);
				}
			}
		}
	}
}

float PDFEstimate::maxEntropy()
{
	float maxEntropy = -999;
	//float minEntropy = 999;

	for (int row = 0; row < height; row++) {
		for (int col = 0; col < width; col++) {
			if (densityEstimates[row][col].entropy > maxEntropy) {
				maxEntropy = densityEstimates[row][col].entropy;
			}
		}
	}
	return maxEntropy;
}

/*
 * Areas with 0 magnitude difference are assigned an entropy
 * value of -1. These areas are now given the value of the
 * maximum entropy estimated.
 *
 * We find the minimum entropy in tandem
 */
float PDFEstimate::minEntropy(float maxEntropy)
{
	// Find the min and set ERROR_FLAG values to a max

	// For proper visualization (lkk what does this mean?)
	float minEntropy = 999;

	for (int row = 0; row < height; row++) {
		for (int col = 0; col < width; col++) {
			if (densityEstimates[row][col].entropy == ERROR_FLAG) {
				densityEstimates[row][col].entropy = maxEntropy;
			}

			if (densityEstimates[row][col].entropy < minEntropy) {
				minEntropy = densityEstimates[row][col].entropy;
			}
		}
	}
}

void PDFEstimate::shiftValuesSoMinIsZero(float minEntropy)
{
	// Shift values so that the minimum entropy value is 0
	for (int row = 0; row < height; row++) {
		for (int col = 0; col < width; col++) {
			densityEstimates[row][col].entropy -= minEntropy;
		}
	}
}

void PDFEstimate::quantizeAndCopyTo(float maxEntropy, cv::Mat1f saliencyMap)
{
	// Fill saliencyMap
	// also adjusting maximum entropy (scaling to 8 bits, 255)

	// Note disconnect between loop nesting: for densityEstimate: column major, for saliencyMap row major
	// i.e. this is inefficient for saliencyMap, but efficiency probably not important since only done once.

	for (int row = 0; row < height; row++) {
		for (int col = 0; col < width; col++) {
			if (maxEntropy > 0) {
				saliencyMap(row, col) = (255.0 - ((densityEstimates[row][col].entropy / maxEntropy) * 255.0));
			} else {
				saliencyMap(row, col) = 0;
			}
		}
	}
}

}

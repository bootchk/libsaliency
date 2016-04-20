
#include <opencv2/core/core.hpp>

#include <kernelDensityInfo.hpp>
#include "PDFEstimate.hpp"

namespace sal {

PDFEstimate::PDFEstimate() { }
PDFEstimate::~PDFEstimate() { }


void PDFEstimate::resizeTo(cv::Size inImageSize, const int channelCount, const int neighborhoodSize )
{
	// !!! all instances have same channelCount
	KernelDensityInfo::initClass(channelCount);

	// densityEstimate per pixel, not channel
	densityEstimates.resize(inImageSize.height);
	for (int row = 0; row < inImageSize.height; ++row) {
		densityEstimates[row].resize(inImageSize.width);
	}
	// assert densityEstimates size equals inImageSize, and elements are initialized KernelDensityInfo
	// (that is what std::vector.resize() does.)

	height = inImageSize.height;
	width = inImageSize.width;
	// Other dimension: channelCount is KernelDensityInfo class var

	// Parameter of the estimation algorithm
	sampleCountLimit = (2 * neighborhoodSize - 1) * (2 * neighborhoodSize - 1);
}

bool PDFEstimate::isSane()
{
	assert(!densityEstimates.empty());
	assert(!densityEstimates[0].empty());
	assert(densityEstimates[0][0].sampleCount >= 0);
	return true;
}

void PDFEstimate::updateApplicableRegion(const cv::Rect& bounds, const KernelDensityInfo& sampleResult)
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
				densityEstimates[row][col].sumSampleResult(sampleResult);

				/*
				Update pixel entropy every N (= 32) sample contributions.
				Only some pixels in the region will get updates
				(they all have their own sampleCount of contributions.)

				lkk I don't understand this, must be something to do with numerical analysis losses.
				Maybe:  sums get large and lose precision?
				 */
				/* TEMP
				if (((densityEstimates[row][col].sampleCount + 1) % 32) == 0) {
					densityEstimates[row][col].updatePixelEntropy(channelCount);
				}
				*/
			}
		}
	}
}

void PDFEstimate::copyResultToGrayscaleImage(cv::Mat1f& saliencyMap)
{
	// This implementation doesn't use a stored entropy value
	// In original code, it was a field of KDI

	float overallmaxEntropy = maxEntropy();
	float overallminEntropy = minEntropy();
	shiftQuantizeAndCopyTo(overallminEntropy, overallmaxEntropy, saliencyMap);
}


float PDFEstimate::maxEntropy()
{
	float maxEntropy = -999;

	for (int row = 0; row < height; row++) {
		for (int col = 0; col < width; col++) {
			float value = densityEstimates[row][col].entropy();
			// See ensure assertion for entropy()
			// require (value==ERROR_FLAG which is negative) or (value is positive)
			// require value is not NaN (since then > comparison throws exception.)
			// not require value is not infinity (overflow may have occurred earlier.)
			if ( value > maxEntropy) {
				maxEntropy = value;
			}
		}
	}
	return maxEntropy;
}


// Returns least, valid entropy
float PDFEstimate::minEntropy()
{
	float minEntropy = 999;

	for (int row = 0; row < height; row++) {
		for (int col = 0; col < width; col++) {
			float value = densityEstimates[row][col].entropy();
			// See ensure assertion for entropy()
			// require (value==ERROR_FLAG which is negative) or (value is positive)
			// require value is not NaN (since then > comparison throws exception.)
			// not require value is not infinity (overflow may have occurred earlier.)
			if ( value !=ERROR_FLAG and value < minEntropy) {
				minEntropy = value;
			}
		}
	}
	assert(("Min entropy is not ERROR_FLAG", minEntropy != ERROR_FLAG));
	assert(minEntropy>=0);
	return minEntropy;
}

/*
 * Areas with 0 magnitude difference are assigned an entropy
 * value of -1. These areas are now given the value of the
 * maximum entropy estimated.
 *
 * We find the minimum entropy in tandem
 */
/*
OBSOLETE
float PDFEstimate::minEntropy(float maxEntropy)
{
	// Find the min and set ERROR_FLAG values to a max

	// For proper visualization (lkk what does this mean?)
	float minEntropy = 999;

	for (int row = 0; row < height; row++) {
		for (int col = 0; col < width; col++) {
			float value = densityEstimates[row][col].entropy();
			if (value == ERROR_FLAG) {
				value = maxEntropy;
			}

			if (value < minEntropy) {
				minEntropy = value;
			}
		}
	}
}
*/

/*
Obsolete
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

	// This requires that the entropy field has already been shifted so min is zero

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
*/

void PDFEstimate::shiftQuantizeAndCopyTo(float overallminEntropy, float overallmaxEntropy, cv::Mat1f& saliencyMap)
{
	printf("minEntropy %f maxEntropy%f \n", overallminEntropy, overallmaxEntropy);

	assert(overallmaxEntropy > overallminEntropy);
	float shiftedOverallMaxEntropy = overallmaxEntropy - overallminEntropy;

	for (int row = 0; row < height; row++) {
		for (int col = 0; col < width; col++) {
			float rawValue = densityEstimates[row][col].entropy();
			if (rawValue == ERROR_FLAG )
			{
				// magnitude difference was zero
				// Replace value with max entropy
				rawValue = overallmaxEntropy;
			}
			float shiftedValue = rawValue - overallminEntropy;

			// quantize then invert
			saliencyMap(row, col) = (255.0 - (( shiftedValue / shiftedOverallMaxEntropy) * 255.0));
		}
	}
}

}  //namespace

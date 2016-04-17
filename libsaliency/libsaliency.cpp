
#include <cstdlib>
#include <iostream>
#include <cstdio>
#include <stdexcept>
#include <cassert>
#include <ctime>
#include <cmath>
#include <array>


#include "Bounder.h"
#include "libsaliency.hpp"
#include "Gradienter.h"
#include "Smoother.h"
// #include "Sampler.h"	// Original alternative to SamplePool
#include "SamplePool.h"
#include "Quantizer.h"



namespace sal {

// Method of super class
void SaliencyDetector::postProcessSaliencyMap(cv::Mat1f& salMap, const float& sigma)
{
	Smoother smoother(sigma);

	cv::Mat1f filteredMap;

	double minV = 1, maxV = 1;
	cv::minMaxLoc(salMap, &minV, &maxV);

	smoother.smoothImage(salMap, filteredMap);
	filteredMap.copyTo(salMap);
	filteredMap.release();

	// Smoothing twice effectively gets rid of unsightly rings caused by the Gaussian
	smoother.setSigma(0.6f * sigma);
	smoother.smoothImage(salMap, filteredMap);

	// Normalize, overwriting salMap
	// lkk alternative to commented out
	cv::normalize(filteredMap, salMap, 0, 255, cv::NORM_MINMAX);
	filteredMap.release();

	/*
	 * This is the original code.
	filteredMap.copyTo(salMap);
	double maxVal = 0, minVal = 0;
	cv::minMaxLoc(salMap, &minVal, &maxVal);

	for (int i = 0; i < salMap.rows; i++) {
		for (int j = 0; j < salMap.cols; j++) {
			salMap(i, j) = ((((salMap(i, j) - static_cast<float>(minVal)) / static_cast<float>(maxVal - minVal)) * 255.0));
		}
	}
	*/
}



/// ImageSaliencyDetector subclass methods

ImageSaliencyDetector::ImageSaliencyDetector(const cv::Mat& src) {
	assert(!src.empty());
	assert(src.channels() >= 1);	// at least one channel, grayscale or more
	setSourceImage(src);

	inImageSize = srcImage.size();
	printf("Input image size %i, %i\n", inImageSize.width, inImageSize.height);

	// densityEstimate per pixel, not channel
	densityEstimates.resize(inImageSize.height);
	for (int row = 0; row < inImageSize.height; ++row) {
		densityEstimates[row].resize(inImageSize.width);
	}
	// assert densityEstimates size equals inImageSize, and elements are initialized KernelDensityInfo
	// (that is what std::vector.resize() does.)

	// !!! column major order, i.e.  [row][col] addressing
}


ImageSaliencyDetector::~ImageSaliencyDetector() { }




// For given pair of pixels, calculate weightedAttributes for each channel (pixelel)
// TODO Do channels in parallel (vector operation )
void ImageSaliencyDetector:: calculateChannelAttributes(
		Location2D first,
		Location2D second,
		AttributeVector& attributes
		)
{
	// address arithmetic
	int stride = magnitudes.channels();
	// row * rowLength + col * stride i.e. elementLength
	int firstPixelAddress = first.y * (magnitudes.cols * stride) + (first.x * stride);
	int secondPixelAddress = second.y * (magnitudes.cols * stride) + (second.x * stride);

	// To use address arithmetic, indexed pointer must be of proper type
	// i.e. compiler multiplies by sizeof(type)
	float* magnitudeData = (float*) magnitudes.data;
	float* orientationData = (float*) orientations.data;

	for(int channel=0; channel<magnitudes.channels(); channel++ ) {
		// TODO abs instead of sqrt(pow()
		float signedValue =
				magnitudeData[firstPixelAddress + channel] -
				magnitudeData[secondPixelAddress + channel] ;
		attributes[channel].weight = sqrt(pow(signedValue, 2));

		signedValue =
				orientationData[firstPixelAddress + channel] -
				orientationData[secondPixelAddress + channel] ;
		attributes[channel].angle = sqrt(pow(signedValue, 2));
	};
	// Assert attributes has angle,weight for each channel
}


std::array<float, 3> kernelChannels;

// For given sample (two pair_, calculate kernel result (gaussian function of gradient angle differences)
void ImageSaliencyDetector:: calculateKernelsForChannels(
		AttributeVector& firstPairAttributes,
		AttributeVector& secondPairAttributes,
		float aNorm,
		float angleBinWidth)
{
	for (int channel=0; channel<magnitudes.channels(); channel++) {
		float angleDifference = firstPairAttributes[channel].angle - secondPairAttributes[channel].angle;
		kernelChannels[channel] = (1.f / aNorm) * exp((pow(angleDifference, 2) / (-2.f * pow(angleBinWidth, 2))));
	}
	// assert kernelChannels holds kernel value for each channel
}


void ImageSaliencyDetector::calculateKernelSum(const TSamples& samples, KernelDensityInfo& kernelInfo) {
	assert(!srcImage.empty());
	assert(!magnitudes.empty());
	assert(!orientations.empty());
	assert(samples.size() == 4);

	int imgWidth = srcImage.cols, imgHeight = srcImage.rows;
	float sampleDistance1 = 0.f;
	float sampleDistance2 = 0.f;
	float distanceKernel = 0.f;
	//, angleKernel = 0.f;

	// Document magic numbers
	float binDimension = 10.f;
	float distanceBinWidth = sqrt(pow(imgWidth, 2) + pow(imgHeight, 2)) / binDimension;
	float angleBinWidth = 3.14159265358979323846 / binDimension;
	//float twoPI = 6.283185307;
	float dNorm = (2.5066 * distanceBinWidth);
	float aNorm = (2.5066 * angleBinWidth);

	// assert samples are in image bounds

	Location2D first  = samples[0];
	Location2D second = samples[1];
	Location2D third  = samples[2];
	Location2D fourth = samples[3];

	// First attribute: euclidean distance between two pixels of a pair
	// TODO would manhattan distance work?
	sampleDistance1 = sqrt(pow((first.y - second.y), 2) + pow((first.x - second.x), 2));
	sampleDistance2 = sqrt(pow((third.y - fourth.y), 2) + pow((third.x - fourth.x), 2));

	// Further attributes: difference in gradient direction between in pixelel (channel) of two pixels of a pair
	AttributeVector firstPairAttributes;
	AttributeVector secondPairAttributes;

	calculateChannelAttributes( first, second, firstPairAttributes);
	calculateChannelAttributes( third, fourth, secondPairAttributes);

	// !!! y first when addressing Mat using () or at()
	//sampleAngle1 = sqrt(pow(orientations(first.y, first.x) - orientations(second.y, second.x), 2));
	//sampleAngle2 = sqrt(pow(orientations(third.y, third.x) - orientations(fourth.y, fourth.x), 2));

	// Weights:  difference in magnitudes
	//sampleMag1   = sqrt(pow(magnitudes(first.y, first.x) - magnitudes(second.y, second.x), 2));
	//sampleMag2   = sqrt(pow(magnitudes(third.y, third.x) - magnitudes(fourth.y, fourth.x), 2));
	// TODO weights per channel?????

	// Statistical kernel is gaussian
	// One attribute is distance between samples
	distanceKernel = (1.f / dNorm) * exp((pow(sampleDistance1 - sampleDistance2, 2) / (-2.f * pow(distanceBinWidth, 2))));

	// One per channel, difference between angle attribute
	// TODO kernels for other channels
	// For now, only using channel 0 (say Green.)
	calculateKernelsForChannels(
			firstPairAttributes,
			secondPairAttributes,
			aNorm,
			angleBinWidth);
	// angleKernel = kernelChannels[0];

	// post results

	// Put weights into result
	for(int channel = 0; channel<magnitudes.channels(); channel++) {
		kernelInfo.weights[0][channel]  = firstPairAttributes[channel].weight;	// first sample pair
		kernelInfo.weights[1][channel]  = secondPairAttributes[channel].weight;
		// kernelInfo.secondWeight = secondPairAttributes[0].weight;
	}

	// Product of angle kernels
	float productOfKernelChannels = kernelChannels[0] * kernelChannels[1] * kernelChannels[2];

	// TODO can't we assert that some or all of these are >=0?
	// TODO check all angleKernels > 0
	if (firstPairAttributes[0].weight > 0 && secondPairAttributes[0].weight > 0 && distanceKernel > 0 ) { // && angleKernel > 0) {
		// TODO more kernels
		kernelInfo.kernelSum = (kernelInfo.productOfWeights() * distanceKernel * productOfKernelChannels); // angleKernel);
	} else {
		kernelInfo.kernelSum = 0;
	}
	// assert kernelInfo is results from this sample
}

// KernelDensityInfo methods

// Constructor calls init
ImageSaliencyDetector::KernelDensityInfo::KernelDensityInfo() {
	init();
};

void ImageSaliencyDetector::KernelDensityInfo::init() {
	kernelSum = 0.f;
	entropy = 0.f;
	sampleCount = 0;
	//firstWeight(0.f),
	//secondWeight(0.f),

	// compiler will unroll?
	for(int i=0; i<2; i++) {
		for(int j=0; j<3; j++) {
			weights[i][j] = 0.f;
		}
	}
}

float ImageSaliencyDetector::KernelDensityInfo::productOfWeights() {
	float result = 1.0f;
	for(int i=0; i<2; i++) {
		for(int j=0; j<3; j++) {
			result *= weights[i][j];
		}
	}
	return result;
}

void ImageSaliencyDetector::KernelDensityInfo::sumOtherWeightsIntoSelf(const KernelDensityInfo& other) {
	for(int i=0; i<2; i++) {
		for(int j=0; j<3; j++) {
			weights[i][j] += other.weights[i][j];
		}
	}
}

void ImageSaliencyDetector::KernelDensityInfo::updatePixelEntropy() {
	// assert self is a densityEstimate
	if (sampleCount > 0) {

		float totalWeight = 0;
		// TODO channelcount
		for(int i=0; i<2; i++) {
			for(int j=0; j<3; j++) {
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


void ImageSaliencyDetector::updateApplicableRegion(const cv::Rect& bounds, const KernelDensityInfo& kernelSum) {
	// Require initialized densityEstimates
	assert(!densityEstimates.empty());
	assert(!densityEstimates[0].empty());
	assert(densityEstimates[0][0].sampleCount >= 0);

	int sampleCountLimit = (2 * neighborhoodSize - 1) * (2 * neighborhoodSize - 1);
	int width = srcImage.cols;
	int height = srcImage.rows;

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
			assert (row < inImageSize.height and col < inImageSize.width);
			if (densityEstimates[row][col].sampleCount < sampleCountLimit) {
				sumKernelResultToDensityEstimate(kernelSum, row, col);

				// Update the pixel entropy every N (= 32) iterations
				if (((densityEstimates[row][col].sampleCount + 1) % 32) == 0) {
					densityEstimates[row][col].updatePixelEntropy();
				}
			}
		}
	}
}


void inline ImageSaliencyDetector::sumKernelResultToDensityEstimate(const KernelDensityInfo& kernelResult, int row, int col)
{
	densityEstimates[row][col].kernelSum += kernelResult.kernelSum;
	//densityEstimates[row][col].firstWeight += kernelResult.firstWeight;
	//densityEstimates[row][col].secondWeight += kernelResult.secondWeight;
	densityEstimates[row][col].sumOtherWeightsIntoSelf(kernelResult);
	densityEstimates[row][col].sampleCount++;
}


void ImageSaliencyDetector::createSaliencyMap() {
	int width = srcImage.cols;
	int height = srcImage.rows;

	// Initialize saliency map
	saliencyMap.create(height, width);

	// For proper visualization
	float maxEntropy = -999;
	float minEntropy = 999;

	for (int row = 0; row < height; row++) {
		for (int col = 0; col < width; col++) {
			if (densityEstimates[row][col].entropy > maxEntropy) {
				maxEntropy = densityEstimates[row][col].entropy;
			}
		}
	}

	/*
	 * Areas with 0 magnitude difference are assigned an entropy
	 * value of -1. These areas are now given the value of the
	 * maximum entropy estimated.
	 *
	 * We find the minimum entropy in tandem
	 */
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

	// Shift values so that the minimum entropy value is 0
	for (int row = 0; row < height; row++) {
		for (int col = 0; col < width; col++) {
			densityEstimates[row][col].entropy -= minEntropy;
		}
	}

	// Also adjust the maximum entropy
	maxEntropy -= minEntropy;

	// Fill saliencyMap (while adjusting maximum entropy)
	// Note disconnect between loop nesting: for densityEstimate: column major, for saliencyMap row major
	// i.e. this is inefficient for saliencyMap, but efficiency probably not important
	// since only done once

	for (int row = 0; row < height; row++) {
		for (int col = 0; col < width; col++) {
			if (maxEntropy > 0) {
				saliencyMap(row, col) = (255.0 - ((densityEstimates[row][col].entropy / maxEntropy) * 255.0));
			} else {
				saliencyMap(row, col) = 0;
			}
		}
	}
	// assert saliencyMap is created and filled
}


void ImageSaliencyDetector::performPostProcessing() {
	postProcessSaliencyMap(this->saliencyMap);
}


void ImageSaliencyDetector::compute() {
	clock_t tStart;

	if (srcImage.empty()) {
		throw std::logic_error("ImageSaliencyDetector: Source image is empty!");
	}

	srand(time(NULL));

	// Get gradient information
	Gradienter gradienter;
	gradienter.compute(srcImage);
	setMagnitudes(gradienter.getGradMagnitudes());
	setOrientations(gradienter.getGradOrientations());

	Quantizer quantizer;
	quantizer.quantizeMagnitudes(magnitudes);

	std::cout << "Iterating on samples.\n";


	// Perform iterative saliency detection mechanism
	int squaredNHood = neighborhoodSize * neighborhoodSize;
	int reqNumSamples = static_cast<int>(samplingPercentage * (srcImage.cols * srcImage.rows * squaredNHood));
	int counter = 0;

	// Sampler sampler(srcImage.rows, srcImage.cols, neighborhoodSize, reqNumSamples);
	tStart = clock();
    SamplePool samplePool;
    samplePool.fillWithValidSamples(srcImage.rows, srcImage.cols, neighborhoodSize, reqNumSamples);
    printf("Total(all threads) time generating samples: %.2fs\n", (double)(clock() - tStart)/CLOCKS_PER_SEC);

    tStart = clock();

    Bounder bounder = Bounder(neighborhoodSize,  cv::Rect(0, 0, srcImage.cols, srcImage.rows));

	while (counter < reqNumSamples) {
		// All uninitialized
		KernelDensityInfo kernelSum;
		cv::Rect bounds;
		TSamples samples;

		// samples = sampler.getCandidateSample();
		samples = samplePool.getNextSample();
		// assert samples from pool is in image bounds

		// lkk Note this differs from original code,
		// which seemed to update with a zero kernelSum when sample not in bounds
		// and also counted that sample.
		// if (sampler.isSampleInImageBounds(samples)) {
		kernelSum.init();
			calculateKernelSum(samples, kernelSum);
			bounds = bounder.getApplicableBounds(samples);
			updateApplicableRegion(bounds, kernelSum);
			++counter;
		//}
	}

	createSaliencyMap();
	// saliency map is same dimensions as src, but fewer channels
	assert(saliencyMap.cols == inImageSize.width and saliencyMap.rows == inImageSize.height);
	assert(saliencyMap.channels()==1);	// saliency map is grayscale
	printf("Size %i, %i\n", saliencyMap.cols, saliencyMap.rows);
	printf("Total(all threads) time iterating: %.2fs\n", (double)(clock() - tStart)/CLOCKS_PER_SEC);
}


} /* namespace sal */

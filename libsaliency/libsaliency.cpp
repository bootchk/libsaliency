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
// #include "Sampler.h"
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

	/*
	 * Smoothing another time effectively gets rid of
	 * unsightly rings caused by the Gaussian
	 */
	filteredMap.release();
	smoother.setSigma(0.6f * sigma);
	smoother.smoothImage(salMap, filteredMap);
	filteredMap.copyTo(salMap);

	//  Normalization
	// lkk alternative to commented out
	cv::normalize(filteredMap, salMap, 0, 255, cv::NORM_MINMAX);

	/*
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
	for (int i = 0; i < inImageSize.height; ++i) {
		densityEstimates[i].resize(inImageSize.width);
	}
}


ImageSaliencyDetector::~ImageSaliencyDetector() { }

// Each channel has this attribute
struct Attribute {
	float angle;
	float weight;
};

// Short container of attributes, one per channel (e.g. color)
// OpenCV requires def of inner datatype, i.e. custom type.....   typedef cv::Vec<Attribute, 4>  AttributeVector;
// std::array requires C++11
typedef std::array<Attribute, 4> AttributeVector;


// For given pair of pixels, calculate weightedAttributes for each channel (pixelel)
// TODO Do channels in parallel (vector operation )
void calculateChannelAttributes(
		Location2D first,
		Location2D second,
		cv::Mat& orientations,
		cv::Mat magnitudes,
		AttributeVector& attributes
		)
{
	// address arithmetic
	int stride = magnitudes.channels();
	// row * rowLength + col * stride i.e. elementLength
	int firstPixelAddress = first.y * (magnitudes.cols * stride) + (first.x * stride);
	int secondPixelAddress = second.y * (magnitudes.cols * stride) + (second.x * stride);

	for(int channel=0; channel<magnitudes.channels(); channel++ ) {
		// TODO abs instead of sqrt(pow()
		float unsignedValue =
				magnitudes.data[firstPixelAddress + channel] -
				magnitudes.data[secondPixelAddress + channel] ;
		attributes[channel].weight = sqrt(pow(unsignedValue, 2));

		unsignedValue =
				orientations.data[firstPixelAddress + channel] -
				orientations.data[secondPixelAddress + channel] ;
		attributes[channel].angle = sqrt(pow(unsignedValue, 2));
	};
	// Assert attributes has angle,weight for each channel
}



KernelDensityInfo ImageSaliencyDetector::calculateKernelSum(const TSamples& samples) {
	assert(!srcImage.empty());
	assert(!magnitudes.empty());
	assert(!orientations.empty());
	assert(samples.size() == 4);

	int imgWidth = srcImage.cols, imgHeight = srcImage.rows;
	float sampleDistance1 = 0.f;
	float sampleDistance2 = 0.f;
	float distanceKernel = 0.f, angleKernel = 0.f;

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

	// Second attribute: difference in gradient direction between two pixels of a pair
	// Here sqrt(pow(x)) is just getting absolute value
	// TODO For each channel
	AttributeVector firstPairAttributes;
	AttributeVector secondPairAttributes;

	calculateChannelAttributes( first, second, orientations, magnitudes, firstPairAttributes);
	calculateChannelAttributes( third, fourth, orientations, magnitudes, secondPairAttributes);

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
	float angleDifference = firstPairAttributes[0].angle - secondPairAttributes[0].angle;
	angleKernel = (1.f / aNorm) * exp((pow(angleDifference, 2) / (-2.f * pow(angleBinWidth, 2))));

	// return results
	KernelDensityInfo kernelInfo;

	kernelInfo.firstWeight  = firstPairAttributes[0].weight;
	kernelInfo.secondWeight = secondPairAttributes[0].weight;

	// TODO
	float productOfWeights = firstPairAttributes[0].weight * secondPairAttributes[0].weight;

	// TODO can't we assert that some or all of these are >=0?
	if (firstPairAttributes[0].weight > 0 && secondPairAttributes[0].weight > 0 && distanceKernel > 0 && angleKernel > 0) {
		// TODO more kernels
		kernelInfo.kernelSum = (productOfWeights * distanceKernel * angleKernel);
	} else {
		kernelInfo.kernelSum = 0;
	}
	return kernelInfo;
}





void ImageSaliencyDetector::updatePixelEntropy(KernelDensityInfo& kernelInfo) {
	if (kernelInfo.sampleCount > 0) {
		float totalWeight = kernelInfo.firstWeight * kernelInfo.secondWeight;
		float estimation = 0.f;

		// Special case: avoid division by 0
		if (totalWeight <= 0) {
			totalWeight = static_cast<float>(kernelInfo.sampleCount);
		}

		if (kernelInfo.kernelSum < 0 || isnan(kernelInfo.kernelSum)) {
			kernelInfo.kernelSum = 0;
		}

		estimation = kernelInfo.kernelSum / totalWeight;

		// Another special case: if the calculated values are -ve or NaNs
		if (estimation <= 1e-15) {
			kernelInfo.entropy = ERROR_FLAG;
		} else if (isnan(estimation)) {
			kernelInfo.entropy = ERROR_FLAG;
		} else {
			kernelInfo.entropy = -1.0f * log2f(estimation * estimation);
		}
	}
}


void ImageSaliencyDetector::updateApplicableRegion(const cv::Rect& bounds, const KernelDensityInfo& kernelSum) {
	assert(!densityEstimates.empty());
	assert(!densityEstimates[0].empty());

	int sampleCountLimit = (2 * neighborhoodSize - 1) * (2 * neighborhoodSize - 1);
	int width = srcImage.cols;
	int height = srcImage.rows;

	// Bounds define an aligned rect
	// Bounds are clamped to image bounds.
	// I.E. assert bounding rect contained in image
	// Also assert bounding rect is square of dimension neighborhoodSize

	// Iterate over coordinates of aligned bounding rect
	for (int i = bounds.y; i <= bounds.y + bounds.height; i++) {
		//for (int j = bounds.topLeft.x; j <= bounds.topRight.x; j++) {
		for (int j = bounds.x; j <= bounds.x + bounds.width; j++) {
			if (densityEstimates[i][j].sampleCount < sampleCountLimit) {
				sumKernelResultToDensityEstimate(kernelSum, i, j);

				// Update the pixel entropy every N (= 32) iterations
				if (((densityEstimates[i][j].sampleCount + 1) % 32) == 0) {
					updatePixelEntropy(densityEstimates[i][j]);
				}
			}
		}
	}
}


void inline ImageSaliencyDetector::sumKernelResultToDensityEstimate(const KernelDensityInfo& kernelResult, int x, int y)
{
	densityEstimates[x][y].kernelSum += kernelResult.kernelSum;
	densityEstimates[x][y].firstWeight += kernelResult.firstWeight;
	densityEstimates[x][y].secondWeight += kernelResult.secondWeight;
	densityEstimates[x][y].sampleCount++;
}


void ImageSaliencyDetector::createSaliencyMap() {
	int width = srcImage.cols;
	int height = srcImage.rows;

	// Initialize saliency map
	saliencyMap.create(height, width);

	// For proper visualization
	float maxEntropy = -999;
	float minEntropy = 999;

	for (int i = 0; i < height; i++) {
		for (int j = 0; j < width; j++) {
			if (densityEstimates[i][j].entropy > maxEntropy) {
				maxEntropy = densityEstimates[i][j].entropy;
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
	for (int i = 0; i < height; i++) {
		for (int j = 0; j < width; j++) {
			if (densityEstimates[i][j].entropy == ERROR_FLAG) {
				densityEstimates[i][j].entropy = maxEntropy;
			}

			if (densityEstimates[i][j].entropy < minEntropy) {
				minEntropy = densityEstimates[i][j].entropy;
			}
		}
	}

	// Shift values so that the minimum entropy value is 0
	for (int i = 0; i < height; i++) {
		for (int j = 0; j < width; j++) {
			densityEstimates[i][j].entropy -= minEntropy;
		}
	}

	// Also adjust the maximum entropy
	 maxEntropy -= minEntropy;

	for (int i = 0; i < height; i++) {
		for (int j = 0; j < width; j++) {
			if (maxEntropy > 0) {
				saliencyMap(i, j) = (255.0 - ((densityEstimates[i][j].entropy / maxEntropy) * 255.0));
			} else {
				saliencyMap(i, j) = 0;
			}
		}
	}
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
			kernelSum = calculateKernelSum(samples);
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

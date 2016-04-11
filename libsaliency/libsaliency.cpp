#include <cstdlib>
#include <iostream>
#include <cstdio>
#include <stdexcept>
#include <cassert>
#include <ctime>
#include <cmath>

#include "libsaliency.hpp"
#include "Gradienter.h"
#include "Smoother.h"
#include "Sampler.h"


namespace sal {


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


//////////////////////////////////////////////////////////////////////////////////////////
/// ImageSaliencyDetector Methods
//////////////////////////////////////////////////////////////////////////////////////////

ImageSaliencyDetector::ImageSaliencyDetector(const cv::Mat1f& src) {
	if (src.empty()) {
		throw std::invalid_argument("ImageSaliencyDetector: Source image cannot be empty!");
	}

	setSourceImage(src);

	densityEstimates.resize(srcImage.rows);
	for (int i = 0; i < srcImage.rows; ++i) {
		densityEstimates[i].resize(srcImage.cols);
	}
}


ImageSaliencyDetector::~ImageSaliencyDetector() {

}


void ImageSaliencyDetector::quantizeMagnitudes() {
	if (magnitudes.empty()) {
		throw std::logic_error("ImageSaliencyDetector: There must be magnitudes info to process!");
	}

	int width = srcImage.cols;
	int height = srcImage.rows;
	float maxMag = FLT_MIN;

	for (int i = 0; i < height; i++) {
		for (int j = 0; j < width; j++) {
			if (magnitudes(i, j) > maxMag) {
				maxMag = magnitudes(i, j);
			}
		}
	}

	// Thresholds
	float t1 = maxMag / 3.0f;
	float t2 = maxMag / 2.0f;
	float t3 = maxMag / 1.5f;

	/*
	 * This quantization differs from the original as a result of using OpenCV
	 */
	for (int i = 0; i < height; i++) {
		for (int j = 0; j < width; j++) {
			if (magnitudes(i, j) >= 0 && magnitudes(i, j) < t1) {
				magnitudes(i, j) = 0.05 * maxMag;

			} else if (magnitudes(i, j) >= t1 && magnitudes(i, j) < t2) {
				magnitudes(i, j) = 0.25 * maxMag;

			} else if (magnitudes(i, j) >= t2 && magnitudes(i, j) < t3) {
				magnitudes(i, j) = 0.75 * maxMag;

			} else if (magnitudes(i, j) >= t3){
				magnitudes(i, j) = maxMag;
			}
		}
	}
}


KernelDensityInfo ImageSaliencyDetector::calculateKernelSum(const std::vector<Location2D>& samples) {
	assert(!srcImage.empty());
	assert(!magnitudes.empty());
	assert(!orientations.empty());
	assert(samples.size() == 4);

	KernelDensityInfo kernelInfo;
	int imgWidth = srcImage.cols, imgHeight = srcImage.rows;
	float sampleDistance1 = 0.f, sampleAngle1 = 0.f, sampleMag1 = 0.f;
	float sampleDistance2 = 0.f, sampleAngle2 = 0.f, sampleMag2 = 0.f;
	float distanceKernel = 0.f, angleKernel = 0.f;
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

	sampleDistance1 = sqrt(pow((first.y - second.y), 2) + pow((first.x - second.x), 2));
	sampleDistance2 = sqrt(pow((third.y - fourth.y), 2) + pow((third.x - fourth.x), 2));


	sampleAngle1 = sqrt(pow(orientations(first.y, first.x) - orientations(second.y, second.x), 2));
	sampleAngle2 = sqrt(pow(orientations(third.y, third.x) - orientations(fourth.y, fourth.x), 2));

	sampleMag1   = sqrt(pow(magnitudes(first.y, first.x) - magnitudes(second.y, second.x), 2));
	sampleMag2   = sqrt(pow(magnitudes(third.y, third.x) - magnitudes(fourth.y, fourth.x), 2));

	kernelInfo.firstWeight  = sampleMag1;
	kernelInfo.secondWeight = sampleMag2;

	distanceKernel = (1.f / dNorm) * exp((pow(sampleDistance1 - sampleDistance2, 2) / (-2.f * pow(distanceBinWidth, 2))));
	angleKernel    = (1.f / aNorm) * exp((pow(sampleAngle1 - sampleAngle2, 2) / (-2.f * pow(angleBinWidth, 2))));

	if (sampleMag1 > 0 && sampleMag2 > 0 && distanceKernel > 0 && angleKernel > 0) {
		kernelInfo.kernelSum = (sampleMag1 * sampleMag2 * distanceKernel * angleKernel);
	} else {
		kernelInfo.kernelSum = 0;
	}


	return kernelInfo;
}


BoundingBox2D ImageSaliencyDetector::getApplicableBounds(const std::vector<Location2D>& samples) {
	BoundingBox2D bounds;
	int L = neighborhoodSize;
	int maxX = samples[0].x;
	int minX = samples[0].x;
	int maxY = samples[0].y;
	int minY = samples[0].y;
	int xDiff = 0;		// Differences between the max / min x values
	int yDiff = 0;		// Differences between the max / min y values
	int xDisp = 0;		// The disparity in the x axis for forming an M x M neighborhood
	int yDisp = 0;		// The disparity in the y axis for forming an M x M neighborhood

	// Get the maximum / minimum x and y values of the pixels
	for (int i = 1; i < 4; i++) {
		if (samples[i].x > maxX) maxX = samples[i].x;
		if (samples[i].y > maxY) maxY = samples[i].y;
		if (samples[i].x < minX) minX = samples[i].x;
		if (samples[i].y < minY) minY = samples[i].y;
	}

	// Calculate the differences between the max / min values
	xDiff = maxX - minX;
	yDiff = maxY - minY;

	// Get the x and y disparity
	xDisp = (L - xDiff) - 1;
	yDisp = (L - yDiff) - 1;

	// Calculate the applicable bounds
	bounds.topLeft.x = minX - xDisp;
	bounds.topLeft.y = minY - yDisp;

	bounds.topRight.x = maxX + xDisp;
	bounds.topRight.y = minY - yDisp;

	bounds.botLeft.x = minX - xDisp;
	bounds.botLeft.y = maxY + yDisp;

	bounds.botRight.x = maxX + xDisp;
	bounds.botRight.y = maxY + yDisp;

	return bounds;
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


void ImageSaliencyDetector::updateApplicableRegion(const BoundingBox2D& bounds, const KernelDensityInfo& kernelSum) {
	assert(!densityEstimates.empty());
	assert(!densityEstimates[0].empty());

	int sampleCountLimit = (2 * neighborhoodSize - 1) * (2 * neighborhoodSize - 1);
	int width = srcImage.cols;
	int height = srcImage.rows;

	for (int i = bounds.topLeft.y; i <= bounds.botLeft.y; i++) {
		for (int j = bounds.topLeft.x; j <= bounds.topRight.x; j++) {

			if (i >= 0 && i < height && j >= 0 && j < width) {
				if (densityEstimates[i][j].sampleCount < sampleCountLimit) {
					densityEstimates[i][j].kernelSum += kernelSum.kernelSum;
					densityEstimates[i][j].firstWeight += kernelSum.firstWeight;
					densityEstimates[i][j].secondWeight += kernelSum.secondWeight;
					densityEstimates[i][j].sampleCount++;

					/*
					 * Update the pixel entropy every N (= 32) iterations
					 */
					if (((densityEstimates[i][j].sampleCount + 1) % 32) == 0) {
						updatePixelEntropy(densityEstimates[i][j]);
					}
				}
			}
		}
	}
}


void ImageSaliencyDetector::updateSaliencyMap() {
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
	if (srcImage.empty()) {
		throw std::logic_error("ImageSaliencyDetector: Source image is empty!");
	}

	srand(time(NULL));

	// Get gradient information
	Gradienter gradienter;
	gradienter.compute(srcImage);
	setMagnitudes(gradienter.getGradMagnitudes());
	setOrientations(gradienter.getGradOrientations());

	quantizeMagnitudes();

	std::cout << "Iterating on samples.\n";

	// Perform iterative saliency detection mechanism
	int squaredNHood = neighborhoodSize * neighborhoodSize;
	int reqNumSamples = static_cast<int>(samplingPercentage * (srcImage.cols * srcImage.rows * squaredNHood));
	int counter = 0;

	Sampler sampler(srcImage.rows, srcImage.cols, neighborhoodSize);

	while (counter < reqNumSamples) {
		KernelDensityInfo kernelSum;
		BoundingBox2D bounds;
		TSamples samples;

		samples = sampler.getSample();

		// lkk Note this differs from original code,
		// which seemed to update with a zero kernelSum when sample not in bounds
		// and also counted that sample.
		if (sampler.isSampleInImageBounds(samples)) {
			kernelSum = calculateKernelSum(samples);
			bounds = getApplicableBounds(samples);
			updateApplicableRegion(bounds, kernelSum);
			++counter;
		}
	}

	updateSaliencyMap();
}




//////////////////////////////////////////////////////////////////////////////////////////
/// VideoSaliencyDetector Methods
//////////////////////////////////////////////////////////////////////////////////////////


} /* namespace sal */

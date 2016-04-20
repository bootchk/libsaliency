
//#include <cstdlib>
//#include <stdexcept>
//#include <cassert>

// For debugging
#include <iostream>
#include <cstdio>
#include <ctime>


// order is important: most basic types/classes first
#include "kernelDensityInfo.hpp"
#include "Bounder.h"
#include "PDFEstimate.hpp"
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
	 * Original code for norming
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

	pdfEstimate.resizeTo(inImageSize, srcImage.channels(), neighborhoodSize);

	// !!! column major order, i.e.  [row][col] addressing
}


ImageSaliencyDetector::~ImageSaliencyDetector() { }



// TODO class for Attributes

// For given pair of pixels, calculate weightedAttributes for each channel (pixelel)
// TODO Do channels in parallel (vector operation )
void ImageSaliencyDetector:: calculateChannelAttributes(
		Location2D first,
		Location2D second,
		AttributeVector& attributes,
		int channelCount
		)
{
	// address arithmetic
	int stride = channelCount;
	// row * rowLength + col * stride i.e. elementLength
	int firstPixelAddress = first.y * (magnitudes.cols * stride) + (first.x * stride);
	int secondPixelAddress = second.y * (magnitudes.cols * stride) + (second.x * stride);

	// To use address arithmetic, indexed pointer must be of proper type
	// i.e. compiler multiplies by sizeof(type)
	float* magnitudeData = (float*) magnitudes.data;
	float* orientationData = (float*) orientations.data;

	for(int channel=0; channel<channelCount; channel++ ) {
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


// Kernel function
inline float gaussian(float difference, float height, float width) {
	// Standard gaussian function
	// For convenience, the standard minus sign is attached to the denominator at '-2.f'
	assert(not isnan(difference));
	float result = (1.f / height) * exp( (pow(difference, 2) / (-2.f * pow(width, 2))));

	// not assert(height < 1) => not assert(result<1)

	assert(("Gaussian is a pdf", result >= 0.f));
	// assert(difference is small and width is small) => Result may not underflow, i.e. infinitely small or zero
	// But it seems to happen anyway
	assert( not isnan(result));
	return result;
}


std::array<float, MAX_CHANNEL_COUNT> kernelChannels;

// For given sample (two pair_, calculate kernel result (gaussian function of gradient angle differences)
void ImageSaliencyDetector:: calculateKernelsForChannels(
		AttributeVector& firstPairAttributes,
		AttributeVector& secondPairAttributes,
		float aNorm,
		float angleBinWidth,
		int channelCount)
{
	for (int channel=0; channel<channelCount; channel++) {
		float angleDifference = firstPairAttributes[channel].angle - secondPairAttributes[channel].angle;
		kernelChannels[channel] = gaussian( angleDifference, aNorm, angleBinWidth );
	}
	// assert kernelChannels holds kernel value for each channel
}


void ImageSaliencyDetector::calculateKernelSum(const TSamples& samples, KernelDensityInfo& kernelInfo) {
	assert(!srcImage.empty());
	assert(!magnitudes.empty());
	assert(!orientations.empty());
	assert(samples.size() == COUNT_SAMPLE_POINTS);

	int imgWidth = srcImage.cols, imgHeight = srcImage.rows;
	int channelCount = srcImage.channels();

	float sampleDistance1 = 0.f;
	float sampleDistance2 = 0.f;
	float distanceKernel = 0.f;
	//, angleKernel = 0.f;

	// TODO Document magic numbers
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

	// Difference between pixel mags and dirs, for each channel
	calculateChannelAttributes( first, second, firstPairAttributes, channelCount);
	calculateChannelAttributes( third, fourth, secondPairAttributes, channelCount);

	// !!! y first when addressing Mat using () or at()
	//sampleAngle1 = sqrt(pow(orientations(first.y, first.x) - orientations(second.y, second.x), 2));
	//sampleAngle2 = sqrt(pow(orientations(third.y, third.x) - orientations(fourth.y, fourth.x), 2));

	// Weights:  difference in magnitudes
	//sampleMag1   = sqrt(pow(magnitudes(first.y, first.x) - magnitudes(second.y, second.x), 2));
	//sampleMag2   = sqrt(pow(magnitudes(third.y, third.x) - magnitudes(fourth.y, fourth.x), 2));
	// TODO weights per channel?????

	// One attribute is distance between samples
	distanceKernel = gaussian( sampleDistance1 - sampleDistance2, dNorm, distanceBinWidth);

	// One per channel, difference between angle attribute
	// TODO kernels for other channels
	// For now, only using channel 0 (say Green.)
	calculateKernelsForChannels(
			firstPairAttributes,
			secondPairAttributes,
			aNorm,
			angleBinWidth,
			channelCount);
	// angleKernel = kernelChannels[0];

	// Product of angle kernels
	float productOfKernelChannels = kernelChannels[0];
	// Multiply by any other channels
	for (int channel=1; channel<channelCount; channel++) {
		productOfKernelChannels *= kernelChannels[channel];
	}

	// post results into sampleResult

	// Put weights into sampleResult (needed to call sampleResult.productOfWeights())
	for(int channel = 0; channel<channelCount; channel++) {
		kernelInfo.weights[0][channel]  = firstPairAttributes[channel].weight;	// first sample pair
		kernelInfo.weights[1][channel]  = secondPairAttributes[channel].weight;
		// kernelInfo.secondWeight = secondPairAttributes[0].weight;
	}

	// summand of this iteration to be accumulated into kernelSum
	float kernelTerm = kernelInfo.productOfWeights() * distanceKernel * productOfKernelChannels; // angleKernel);;

	// Summand must not be negative, but the weights can be zero so summand zero
	//
	// if (kernelTerm < 0 ) kernelTerm = 0;
	// if (firstPairAttributes[0].weight > 0 && secondPairAttributes[0].weight > 0 && distanceKernel > 0   && angleKernel > 0) {
	assert(kernelTerm >= 0.f);
	kernelInfo.kernelSum = kernelTerm;	// put kernel summand into result

	// assert kernelInfo is results from this sample
}

// KernelDensityInfo methods






void ImageSaliencyDetector::createSaliencyMap() {
	int width = srcImage.cols;
	int height = srcImage.rows;

	saliencyMap.create(height, width);
	// assert saliency map, compared to pdfEstimate:
	// - same width and height
	// - one channel
	// - lesser or equal depth (grayscale)

	// TODO allow for color saliency maps i.e. false color

	pdfEstimate.copyResultToGrayscaleImage(saliencyMap);
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

	printf("Iterating on samples\n");


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

    KernelDensityInfo sampleResult = KernelDensityInfo();
	while (counter < reqNumSamples) {
		// All uninitialized
		cv::Rect bounds;
		TSamples samples;

		// samples = sampler.getCandidateSample();
		samples = samplePool.getNextSample();
		// assert samples from pool is in image bounds

		// lkk Note this differs from original code,
		// which seemed to update with a zero sampleResult when sample not in bounds
		// and also counted that sample.
		// if (sampler.isSampleInImageBounds(samples)) {
		sampleResult.init();
		calculateKernelSum(samples, sampleResult);
		bounds = bounder.getApplicableBounds(samples);
		pdfEstimate.updateApplicableRegion(bounds, sampleResult);
		++counter;
		//}
	}

	printf("Creating saliency image\n");
	createSaliencyMap();
	// saliency map is same dimensions as src, but fewer channels
	assert(saliencyMap.cols == inImageSize.width and saliencyMap.rows == inImageSize.height);
	assert(saliencyMap.channels()==1);	// saliency map is grayscale
	printf("Size %i, %i\n", saliencyMap.cols, saliencyMap.rows);
	printf("Total(all threads) time iterating: %.2fs\n", (double)(clock() - tStart)/CLOCKS_PER_SEC);
}


} /* namespace sal */

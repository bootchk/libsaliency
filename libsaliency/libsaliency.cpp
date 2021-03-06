
//#include <cstdlib>
//#include <stdexcept>
//#include <cassert>

// For debugging
#include <iostream>
#include <cstdio>
#include <ctime>
#include <array>


// order is important: most basic types/classes first
#include "kernelDensityInfo.hpp"
#include "SaliencyMath.hpp"
#include "Channels.hpp"
#include "Bounder.h"
#include "PDFEstimate.hpp"
#include "libsaliency.hpp"
#include "Gradienter.h"
#include "Smoother.h"
// #include "Sampler.h"	// Original alternative to SamplePool
#include "SamplePool.h"
#include "Quantizer.h"





namespace sal {

void ImageSaliencyDetector::postProcessSaliencyMap(const float& sigma)
{
	Smoother smoother(sigma);

	cv::Mat1f filteredMap;

	double minV = 1, maxV = 1;
	cv::minMaxLoc(this->saliencyMap, &minV, &maxV);

	smoother.smoothImage(this->saliencyMap, filteredMap);
	filteredMap.copyTo(saliencyMap);
	filteredMap.release();

	// Smoothing twice effectively gets rid of unsightly rings caused by the Gaussian
	smoother.setSigma(0.6f * sigma);
	smoother.smoothImage(saliencyMap, filteredMap);

	// Normalize, overwriting salMap
	// lkk alternative to commented out
	cv::normalize(filteredMap, saliencyMap, 0, 255, cv::NORM_MINMAX);
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

	// Remember attributes
	this->srcSize = src.size();
	this->srcChannelCount = src.channels();

	printf("Input image size %i, %i, channels %i \n", srcSize.width, srcSize.height, src.channels());

	pdfEstimate.resizeTo(srcSize, src.channels(), neighborhoodSize);
	// !!! column major order, i.e.  [row][col] addressing

	// Pre processing ?
	/*
	std::cout << "Smoothing...\n";
	cv::Mat smoothedImage;
	Smoother smoother = Smoother(0);
	smoother.smoothImage(src, smoothedImage);
	computeGradient(smoothedImage);
	 */

	computeGradient(src);

	// The algorithm only uses gradient, not original image
	// We could src.release() but it would not free memory, since src is owned by the caller.
}


ImageSaliencyDetector::~ImageSaliencyDetector() { }


// Extract channel attributes from image into a short container (vector)
void ImageSaliencyDetector:: extractChannelAttributes(
		const Location2D first,
		Channels& firstAttributes,
		cv::Mat image,
		const int channelCount
		)
{
	// address arithmetic
	int stride = channelCount;
	// row * rowLength + col * stride i.e. elementLength
	int firstPixelAddress = first.y * (image.cols * stride) + (first.x * stride);

	// To use address arithmetic, indexed pointer must be of proper type
	// i.e. compiler multiplies by sizeof(type)
	float* attributeData = (float*) image.data;

	for(int channel=0; channel<channelCount; channel++ ) {
		// TODO abs instead of sqrt(pow()
		// firstAttribute is magnitude
		float signedValue = attributeData[firstPixelAddress + channel];
		firstAttributes[channel] = sqrt(pow(signedValue, 2));
	};
}

// TODO class for Attributes

/*
// For given pair of pixels, calculate weight and angle attributes for each channel (pixelel)
// TODO Do channels in parallel (vector operation )
void ImageSaliencyDetector:: calculateChannelAttributes(
		const Location2D first,
		const Location2D second,
		SingleAttributeVector& firstAttributes,
		SingleAttributeVector& secondAttributes,
		const int channelCount
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
		// firstAttribute is magnitude
		float signedValue =
				magnitudeData[firstPixelAddress + channel] -
				magnitudeData[secondPixelAddress + channel] ;
		firstAttributes[channel] = sqrt(pow(signedValue, 2));

		// secondAttribute is direction (sic angle)
		signedValue =
				orientationData[firstPixelAddress + channel] -
				orientationData[secondPixelAddress + channel] ;
		secondAttributes[channel] = sqrt(pow(signedValue, 2));
	};
	// Assert attributes has angle,weight for each channel
}
*/



// std::array<float, MAX_CHANNEL_COUNT> kernelChannels;

// For given sample (two pair_, calculate kernel result by channel
// Not used until I figure out how to properly use vectors of kernels
/*
void ImageSaliencyDetector:: calculateKernelsForChannels(
		AttributeVector& firstPairAttributes,
		AttributeVector& secondPairAttributes,
		float aNorm,
		float angleBinWidth,
		int channelCount)
{
	for (int channel=0; channel<channelCount; channel++) {
		kernelChannels[channel] = gaussian(
				angleBetween(firstPairAttributes[channel].angle, secondPairAttributes[channel].angle),
				aNorm, angleBinWidth );
	}
	// assert static var kernelChannels holds kernel value for each channel
}
*/


/*
float productOfKernels(AttributeVector& firstPairAttributes,
		AttributeVector& secondPairAttributes)
{
	float firstPairAttributeSum = sumChannels(firstPairAttributes);
}
*/

float ImageSaliencyDetector::calulateAngleKernel(
		const Channels& anglesFirst,
		const Channels& anglesSecond,
		const Channels& anglesThird,
		const Channels& anglesFourth,
		const float aNorm, const float angleBinWidth,
		const int channelCount)
{
	/*
	// Documentation: shows nesting when Channel ops have two operands, instead of first operand 'this'
	// A little easier to understand.
	float angleSum = sumChannels(
			angleBetweenChannels(
					angleBetweenChannels(anglesFirst, anglesSecond,
							channelCount),
					angleBetweenChannels(anglesThird, anglesFourth,
							channelCount),
					channelCount),
			channelCount);
	*/
	const Channels &firstSecondAngleBetween = anglesFirst.angleBetweenChannels( anglesSecond, channelCount);
	const Channels &thirdFourthAngleBetween = anglesThird.angleBetweenChannels( anglesFourth, channelCount);

	float angleSum = firstSecondAngleBetween.angleBetweenChannels(thirdFourthAngleBetween, channelCount).sumChannels(channelCount);

	// Result is in [0, 3 * pi]
	assert(angleSum >=0 and angleSum <= 3*LARGER_THAN_PI);
	float result = gaussian(angleSum, aNorm, angleBinWidth);
	return result;
}


float ImageSaliencyDetector::calulateChannelProductAngleKernel(
		const Channels& anglesFirst,
		const Channels& anglesSecond,
		const Channels& anglesThird,
		const Channels& anglesFourth,
		const float aNorm, const float angleBinWidth,
		const int channelCount)
{
	/*
			// Documentation: shows nesting when Channel ops have two operands, instead of first operand 'this'
			// A little easier to understand.
			float angleSum = productChannels(
			    gaussianChannels(
					angleBetweenChannels(
							angleBetweenChannels(anglesFirst, anglesSecond,
									channelCount),
							angleBetweenChannels(anglesThird, anglesFourth,
									channelCount),
							channelCount),
					channelCount)
				)   )

	 */
	const Channels& firstSecondAngleBetween = anglesFirst.angleBetweenChannels( anglesSecond, channelCount);
	const Channels& thirdFourthAngleBetween = anglesThird.angleBetweenChannels( anglesFourth, channelCount);

	const Channels angleChannels = firstSecondAngleBetween.angleBetweenChannels(thirdFourthAngleBetween, channelCount);
	const Channels angleKernels = angleChannels.gaussianChannels(channelCount, aNorm, angleBinWidth);
	// TODO TEST const Channels angleKernels = angleChannels.logCauchyChannels(channelCount, aNorm, angleBinWidth);
	// in range [ gaussian(0), gaussian(3pi) ]
	float angleKernelProduct = angleKernels.productChannels(channelCount);

	return angleKernelProduct;
}


void ImageSaliencyDetector::calculateWeights(
		float& sample1Weight, float&sample2Weight,
		Channels&firstMags,
		Channels&secondMags,
		Channels&thirdMags,
		Channels&fourthMags,
		const int countChannels)
{
	// delta first and second, then sum
	sample1Weight =
			firstMags.deltaChannels( secondMags, countChannels).sumChannels(countChannels);
	sample2Weight =
			thirdMags.deltaChannels( fourthMags, countChannels).sumChannels(countChannels);
}



void ImageSaliencyDetector::calculateKernelSum(const TSamples& samples, KernelDensityInfo& kernelInfo) {
	assert(!magnitudes.empty());
	assert(!orientations.empty());
	assert(samples.size() == COUNT_SAMPLE_POINTS);

	float sampleDistance1 = 0.f;
	float sampleDistance2 = 0.f;
	float distanceKernel = 0.f;

	// TODO Document magic numbers
	float binDimension = 10.f;
	float distanceBinWidth = sqrt(pow(srcSize.width, 2) + pow(srcSize.height, 2)) / binDimension;
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
	Channels firstAngles;
	Channels secondAngles;
	Channels thirdAngles;
	Channels fourthAngles;

	Channels firstMags;
	Channels secondMags;
	Channels thirdMags;
	Channels fourthMags;

	// Difference between pixel mags and dirs, for each channel
	extractChannelAttributes( first, firstAngles, orientations, srcChannelCount);
	extractChannelAttributes( second, secondAngles, orientations, srcChannelCount);
	extractChannelAttributes( third, thirdAngles, orientations, srcChannelCount);
	extractChannelAttributes( fourth, fourthAngles, orientations, srcChannelCount);


	extractChannelAttributes( first, firstMags, magnitudes, srcChannelCount);
	extractChannelAttributes( second, secondMags, magnitudes, srcChannelCount);
	extractChannelAttributes( third, thirdMags, magnitudes, srcChannelCount);
	extractChannelAttributes( fourth, fourthMags, magnitudes, srcChannelCount);

	//calculateChannelAttributes( third, fourth, secondPairAngles, secondPairMags, srcChannelCount);

	// !!! y first when addressing Mat using () or at()
	//sampleAngle1 = sqrt(pow(orientations(first.y, first.x) - orientations(second.y, second.x), 2));
	//sampleAngle2 = sqrt(pow(orientations(third.y, third.x) - orientations(fourth.y, fourth.x), 2));

	// Weights:  difference in magnitudes
	//sampleMag1   = sqrt(pow(magnitudes(first.y, first.x) - magnitudes(second.y, second.x), 2));
	//sampleMag2   = sqrt(pow(magnitudes(third.y, third.x) - magnitudes(fourth.y, fourth.x), 2));
	// TODO weights per channel?????

	// One attribute is distance between samples
	// distanceKernel = gaussian( sampleDistance1 - sampleDistance2, dNorm, distanceBinWidth);
	distanceKernel = proportionToNegLnCauchy( sampleDistance1 - sampleDistance2);
	// TODO temp test

	// One per channel, difference between angle attribute
	// TODO kernels for other channels
	// For now, only using channel 0 (say Green.)
	/*
	calculateKernelsForChannels(
			firstPairAttributes,
			secondPairAttributes,
			aNorm,
			angleBinWidth,
			srcChannelCount);
	*/

	/*
	float angleKernel = calulateAngleKernel(
			firstAngles, secondAngles, thirdAngles, fourthAngles,
			aNorm, angleBinWidth,
			srcChannelCount);
	*/
	float angleKernel = calulateChannelProductAngleKernel(
				firstAngles, secondAngles, thirdAngles, fourthAngles,
				aNorm, angleBinWidth,
				srcChannelCount);

	/*
	// Product of angle kernels
	float productOfKernelChannels = kernelChannels[0];
	// Multiply by any other channels
	for (int channel=1; channel<srcChannelCount; channel++) {
		productOfKernelChannels *= kernelChannels[channel];
	}
	*/

	// post results into sampleResult

	// TODO
	// Put weights into sampleResult (needed to call sampleResult.productOfWeights())
	/*
	for(int channel = 0; channel<srcChannelCount; channel++) {
		kernelInfo.weights[0][channel]  = firstPairMags[channel];	// first sample pair
		kernelInfo.weights[1][channel]  = secondPairMags[channel];
		// kernelInfo.secondWeight = secondPairAttributes[0].weight;
	}
	*/
	float sample1Weight;
	float sample2Weight;
	calculateWeights(sample1Weight, sample2Weight,
			firstMags,
			secondMags,
			thirdMags,
			fourthMags,
			srcChannelCount);
	kernelInfo.weights[0] = sample1Weight;
	kernelInfo.weights[1] = sample2Weight;

	// summand of this iteration, to be accumulated into kernelSum
	float kernelTerm = kernelInfo.productOfWeights() * distanceKernel * angleKernel;

	// Summand must not be negative, but the weights can be zero so summand zero
	//
	// if (kernelTerm < 0 ) kernelTerm = 0;
	// if (firstPairAttributes[0].weight > 0 && secondPairAttributes[0].weight > 0 && distanceKernel > 0   && angleKernel > 0) {
	assert(kernelTerm >= 0.f);
	kernelInfo.kernelSum = kernelTerm;	// put kernel summand into result

	// assert kernelInfo is results from this sample
}



void ImageSaliencyDetector::createSaliencyMap() {

	saliencyMap.create(srcSize.height, srcSize.width);
	// assert saliency map, compared to pdfEstimate:
	// - same width and height
	// - one channel
	// - lesser or equal depth (grayscale)

	// TODO allow for color saliency maps i.e. false color

	pdfEstimate.copyResultToGrayscaleImage(saliencyMap);
	// assert saliencyMap is created and filled
}


void ImageSaliencyDetector::computeGradient(cv::Mat src) {
	Gradienter gradienter;
	gradienter.compute(src);

	setMagnitudes(gradienter.getGradMagnitudes());
	setOrientations(gradienter.getGradOrientations());

	Quantizer quantizer;
	quantizer.quantizeMagnitudes(magnitudes);
}


void ImageSaliencyDetector::compute() {
	clock_t tStart;

	srand(time(NULL));

	assert(!magnitudes.empty());
	// computeGradient(srcImage);  moved to constructor

	printf("Iterating on samples\n");


	// Perform iterative saliency detection mechanism
	int squaredNHood = neighborhoodSize * neighborhoodSize;
	int reqNumSamples = static_cast<int>(samplingPercentage * (srcSize.width * srcSize.height * squaredNHood));
	int counter = 0;

	// Sampler sampler(srcSize.height, srcSize.width, neighborhoodSize, reqNumSamples);
	tStart = clock();
    SamplePool samplePool;
    samplePool.fillWithValidSamples(srcSize.height, srcSize.width, neighborhoodSize, reqNumSamples);
    printf("Total(all threads) time generating samples: %.2fs\n", (double)(clock() - tStart)/CLOCKS_PER_SEC);

    tStart = clock();

    Bounder bounder = Bounder(neighborhoodSize,  cv::Rect(0, 0, srcSize.width, srcSize.height));

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
		bounds = bounder.getRawApplicableBounds(samples);	// OR getApplicableBounds()
		pdfEstimate.updateApplicableRegion(bounds, sampleResult);
		++counter;
		//}
	}

	printf("Creating saliency image\n");
	createSaliencyMap();
	// saliency map is same dimensions as src, but fewer channels
	assert(saliencyMap.cols == srcSize.width and saliencyMap.rows == srcSize.height);
	assert(saliencyMap.channels()==1);	// saliency map is grayscale
	printf("Size %i, %i\n", saliencyMap.cols, saliencyMap.rows);
	printf("Total(all threads) time iterating: %.2fs\n", (double)(clock() - tStart)/CLOCKS_PER_SEC);
}


} /* namespace sal */

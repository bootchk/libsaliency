
#include <cstdio>
#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include "Samples.h"
#include "Bounder.h"

namespace sal {

sal::Bounder::Bounder(int neighborhoodSize, cv::Rect imageRect) {
	this->neighborhoodSize = neighborhoodSize;
	this->imageRect = imageRect;
}

sal::Bounder::~Bounder() { }

/*
 * TODO this code would grow the bounding rect uniformly around center of bounding rect
 *
cv::Size deltaSize( faces[i].width * 0.1f, faces[i].height * 0.1f ); // 0.1f = 10/100
cv::Point offset( deltaSize.width/2, deltaSize.height/2);
faces[i] += deltaSize;
faces[i] -= offset;
*/

// Compute aligned bounding rect that covers samples
// and alter it to be square
// and clip it to image bounds
cv::Rect sal::Bounder::getApplicableBounds(const TSamples& samples) {
	cv::Rect bounds;

	cv::Rect rawBounds = boundsForSample(samples);
	// since samples are in image, rawBounds are also

	// Desire a square bounding rect of dimension neighborhoodSize.
	// (to distribute the kernel result to all density estimates it is part of)
	cv::Rect cover = coveringSquareRect(rawBounds);

	// cover may not be contained in image: clip to image
	cv::Rect result = clippedRectToImage(cover);

	// result need not be square
	// result can be smaller than cover, but must be larger than 1 pixel
	// result.width >= 1

	return result;
}

// Found that expanding and clipping the rawBounds is not necessary for good results.
cv::Rect sal::Bounder::getRawApplicableBounds(const TSamples& samples) {
	cv::Rect bounds;

	cv::Rect rawBounds = boundsForSample(samples);
	// since samples are in image, rawBounds are also

	// result need not be square
	// result must be larger than 1 pixel
	// result.width >= 1

	return rawBounds;	// Test not expanding rawBounds
}

// Aligned rect that bounds given sample
cv::Rect sal::Bounder::boundsForSample(const TSamples& samples)
{
	// TODO is there an openCV function that computes bounding rect for array of points?

	// Starting bounds is just UL, LR both equal to same point, first sample
	int maxX = samples[0].x;
	int minX = samples[0].x;
	int maxY = samples[0].y;
	int minY = samples[0].y;

	// iterate over other samples[1..3], expanding bounding rect to cover them
	for (int i = 1; i < COUNT_SAMPLE_POINTS; i++) {
		// Compute upperRight
		if (samples[i].x > maxX) maxX = samples[i].x;
		if (samples[i].y > maxY) maxY = samples[i].y;
		// Compute lowerLeft
		if (samples[i].x < minX) minX = samples[i].x;
		if (samples[i].y < minY) minY = samples[i].y;
	}
	// Have two points upperLeft (minX,minY) bottomRight (maxX, maxY)
	// that define a bounding rect of samples

	// !!! Size is one larger
	cv::Size resultSize = cv::Size(maxX - minX + 1, maxY - minY + 1);
	cv::Point ul = cv::Point(minX, minY);
	cv::Rect result = cv::Rect(ul, resultSize);

	// !!! Note that result.br() is not (maxX, maxY)
	assert(resultSize.width <= neighborhoodSize and resultSize.height <= neighborhoodSize);
	return result;
}

// Square rect of constant dimension centered over given rect
cv::Rect sal::Bounder::coveringSquareRect(const cv::Rect rect)
{
	// Get delta Size: disparity or difference from wanted dimension
	cv::Size windowSize = cv::Size(neighborhoodSize, neighborhoodSize);
	cv::Size disparity = windowSize - rect.size();

	//int widthDisparity = (neighborhoodSize - size.width);	// The difference in width from neighborhoodSize
	//int heightDisparity = (neighborhoodSize - height);
	assert(disparity.width >= 0);

	// We don't divide the disparity across the rect,
	// the rect expands in the direction of the upper (minY) and left( minX)
	// TODO use openCV rect+=size to expand
	// TODO neighborhoodSize is not a size, is a dim
	/*
	 cv::Rect candidateRect = cv::Rect(
			minX - disparity.width, // x
			minY - disparity.height, // y
			neighborhoodSize, // width
			neighborhoodSize  // height
	);
	*/
	cv::Rect resultRect = rect;	// copy
	resultRect += disparity;	// expand to right and  bottom


	// TODO offset so centered
	//cv::Point offset( deltaSize.width/2, deltaSize.height/2);
	//faces[i] -= offset;
	// assert resultRect is square of dimension neighborhoodSize
	return resultRect;
}


cv::Rect sal::Bounder::clippedRectToImage(const cv::Rect rect)
{
	cv::Rect result = rect & imageRect;	// intersection

	// Note that openCV rect.br() is NOT in the rect.
	// not assert(candidateRect.contains(candidateRect.br()));
	// assert(candidateRect.br().x <= candidateRect.x + candidaterRect.width)

	// ensure result is contained in image
	// Since br() is not contained, br().x can equal rect.width
	assert(result.x>=0 and result.br().x<=imageRect.width and result.y>=0 and result.br().y<=imageRect.height);
	return result;
}

} // namespace

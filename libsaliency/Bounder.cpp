
#include <cstdio>
#include <opencv2/imgproc/imgproc.hpp>
#include "Samples.h"
#include "Bounder.h"


sal::Bounder::Bounder(int neighborhoodSize, cv::Rect imageRect) {
	this->neighborhoodSize = neighborhoodSize;
	this->imageRect = imageRect;
}

sal::Bounder::~Bounder() { }


// Compute aligned bounding rect that covers samples
// and alter it to be square
// and clip it to image bounds
cv::Rect sal::Bounder::getApplicableBounds(const TSamples& samples) {
	cv::Rect bounds;

	// TODO is there an openCV function that computes bounding rect for array of points?
	// TODO separate computation of raw bounding rect from alterations to it.

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
	// Have two points (upperLeft is minX,minY) that define a bounding rect of samples

	int width = maxX - minX + 1;
	int height = maxY - minY + 1;
	// assert width and height <= neighborhoodSize
	assert(width <= neighborhoodSize);

	// Desire a square bounding rect of dimension neighborhoodSize.
	// (to distribute the kernel result to all density estimates it is part of)

	// Get the x and y disparity: difference from wanted dimension
	int widthDisparity = (neighborhoodSize - width);	// The difference in width from neighborhoodSize
	int heightDisparity = (neighborhoodSize - height);
	assert(widthDisparity >= 0);

	// We don't divide the disparity across the rect,
	// the rect expands in the direction of the upper (minY) and left( minX)
	// TODO use openCV rect+=size to expand
	// TODO neighborhoodSize is not a size, is a dim
	cv::Rect candidateRect = cv::Rect(
			minX - widthDisparity, // x
			minY - heightDisparity, // y
			neighborhoodSize, // width
			neighborhoodSize  // height
			);
	// assert candidateRect is square of dimension neighborhoodSize

	// candidateResult may not be contained in image: clip to image
	cv::Rect finalResult = candidateRect & imageRect;	// intersection

	// Note that openCV rect.br() is NOT in the rect.
	// not assert(candidateRect.contains(candidateRect.br()));
	// assert(candidateRect.br().x <= candidateRect.x + candidaterRect.width)

	// ensure finalResult is contained in image
	// TODO this is wrong, since br().x can equal rect.width
	assert(finalResult.x>=0 and finalResult.br().x<=imageRect.width and finalResult.y>=0 and finalResult.br().y<=imageRect.height);

	// finalResult need not be square
	// finalResult can be smaller than candidateRect, but must be larger than 1 pixel
	// finalResult.width >= 1
	return finalResult;
}

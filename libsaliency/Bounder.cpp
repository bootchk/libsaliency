
#include <cstdio>
#include <opencv2/imgproc/imgproc.hpp>
#include "Samples.h"
#include "Bounder.h"


sal::Bounder::Bounder(int neighborhoodSize, cv::Rect imageRect) {
	this->neighborhoodSize = neighborhoodSize;
	this->imageRect = imageRect;
}

sal::Bounder::~Bounder() { }


cv::Rect sal::Bounder::getApplicableBounds(const TSamples& samples) {
	cv::Rect bounds;

	int maxX = samples[0].x;
	int minX = samples[0].x;
	int maxY = samples[0].y;
	int minY = samples[0].y;

	 // Get the maximum and minimum, x and y, of samples
	// Max and mins already initialized for sample[0], iterate over samples[1..3]
	for (int i = 1; i < 4; i++) {
		if (samples[i].x > maxX) maxX = samples[i].x;
		if (samples[i].y > maxY) maxY = samples[i].y;
		if (samples[i].x < minX) minX = samples[i].x;
		if (samples[i].y < minY) minY = samples[i].y;
	}
	// Have two points (upperLeft is minX,minY) that define a bounding rect of samples

	int width = maxX - minX;
	int height = maxY - minY;
	// assert width and height <= neighborhoodSize
	// Desire a square bounding rect of dimension neighborhoodSize.
	// (to distribute the kernel result to all density estimates it is part of)

	// Get the x and y disparity: difference from wanted dimension
	// lkk Why -1 ?
	//xDisp = (neighborhoodSize - width) - 1;
	//yDisp = (neighborhoodSize - height) - 1;

	int xDisp = (neighborhoodSize - width);	// The difference in width from neighborhoodSize
	int yDisp = (neighborhoodSize - height);

	// We don't divide the disparity across the rect,
	// the rect expands in the direction of the upper (minY) and left( minX)
	cv::Rect result = cv::Rect(
			minX - xDisp, // x
			minY - yDisp, // y
			neighborhoodSize, // width
			neighborhoodSize  // height
			);

	// assert result is square of dimension neighborhoodSize

	// candidateResult may not be contained in image: clip to image
	cv::Rect finalResult = result & imageRect;	// intersection

	// ensure finalResult is contained in image
	assert(finalResult.x>=0 and finalResult.br().x<imageRect.width and finalResult.y>=0 and finalResult.br().y<imageRect.height);
	// finalResult need not be square

	return finalResult;
}

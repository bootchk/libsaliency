
#include <opencv2/imgproc/imgproc.hpp>
#include "Samples.h"
#include "Bounder.h"


sal::Bounder::Bounder(int neighborhoodSize) {
	this->neighborhoodSize = neighborhoodSize;
}

sal::Bounder::~Bounder() { }


cv::Rect sal::Bounder::getApplicableBounds(const TSamples& samples) {
	cv::Rect bounds;

	int maxX = samples[0].x;
	int minX = samples[0].x;
	int maxY = samples[0].y;
	int minY = samples[0].y;
	int xDiff = 0;		// Differences between the max / min x values
	int yDiff = 0;		// Differences between the max / min y values

	int xDisp = 0;		// The disparity in the x axis for forming an M x M neighborhood
	int yDisp = 0;		// The disparity in the y axis for forming an M x M neighborhood


	 // Get the maximum and minimum, x and y, of samples
	// Max and mins already initialized for sample[0], iterate over samples[1..3]
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
	// lkk Why -1 ?
	xDisp = (neighborhoodSize - xDiff) - 1;
	yDisp = (neighborhoodSize - yDiff) - 1;


	cv::Rect result = cv::Rect(
			minX - xDisp, // x
			minY - yDisp, // y
			neighborhoodSize, // width
			neighborhoodSize  // height
			);

	// not assert result is contained in image
	return result;
}


#pragma once

#include <opencv2/core/core.hpp>

namespace sal {

/// Alias for 2D Pixel location
typedef cv::Point2i Location2D;

typedef std::vector<Location2D> TSamples;



/*
\brief Class to generate samples.

Valid calling sequence:  getSample
 */
class Sampler {
public:
	explicit Sampler(
			int imageHeight,
			int imageWidth,
			int neighborhoodSize);

	virtual ~Sampler();

	TSamples getSample();

private:

	//void calculateXAndYDerivatives(const cv::Mat1f& smoothedImg, cv::Mat1f& deltaX, cv::Mat1f& deltaY);

	int imageHeight;
	int imageWidth;
	int neighborhoodSize;
	int halfNHood;
};

} // namespace

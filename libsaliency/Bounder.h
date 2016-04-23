#pragma once

#include <opencv2/core/core.hpp>
#include "Samples.h"

namespace sal {


/*
\brief Class understanding bounding rects specialized for incremental kernel density estimation.

Its one method returns a bounding rect given a sample of four points.
The four points define a polygon (the shape is not more defined.)
Also, the returned bounding rect is a predefined size
(neighborhoodSize, the size of the graphics kernel window)
enlarged from a rect that bounds the sample points.

TODO make it also clip the bounding rect to the image.
*/
class Bounder {
public:
	explicit Bounder(int neighborhoodSize, cv::Rect imageRect);
	virtual ~Bounder();

	cv::Rect getApplicableBounds(const TSamples&);
	cv::Rect getRawApplicableBounds(const TSamples&);
	cv::Rect boundsForSample(const TSamples& samples);
	cv::Rect coveringSquareRect(const cv::Rect rect);
	cv::Rect clippedRectToImage(const cv::Rect rect);

private:
	int neighborhoodSize;
	cv::Rect imageRect;
};

} // namespace

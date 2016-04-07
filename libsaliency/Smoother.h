
#pragma once

#include <opencv2/core/core.hpp>

namespace sal {


/*
\brief Class to smooth an image.

Valid calling sequence:  compute ... getGradMagnitude
*/
class Smoother {
public:
	explicit Smoother(const float& sig = 1.20f);
	virtual ~Smoother();


	/// Smooth the input image and store the result in smoothedImg
	void smoothImage(const cv::Mat1f& src, cv::Mat1f& smoothedImg);

	float getSigma() const { return sigma; }

	int getWindowSize() const { return windowSize; }

	/// Set the sigma value (should be greater than 0)
	void setSigma(const float& value) {
		if (value > 0) {
			sigma = value;
		}
	}


private:

	/// Standard deviation for the Gaussian smoothing filter
	float sigma;

	/// The Gaussian window size based on sigma
	int windowSize;
};

} // namespace


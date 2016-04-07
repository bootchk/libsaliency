
#pragma once

#include <opencv2/core/core.hpp>

namespace sal {


/*
\brief Class to compute gradient: magnitude and direction.

Valid calling sequence:  compute ... getGradMagnitude
*/
class Gradienter {
public:
	explicit Gradienter();
	virtual ~Gradienter();

	/// Perform Canny edge detection on the given image
	void compute(const cv::Mat1f& src);

	cv::Mat1f getGradMagnitudes() const { return gradMagnitudes; }
	cv::Mat1f getGradOrientations() const { return gradOrientations; }


private:

	void calculateXAndYDerivatives(const cv::Mat1f& smoothedImg, cv::Mat1f& deltaX, cv::Mat1f& deltaY);
	void calculateGradientDirections(const cv::Mat1f& deltaX, const cv::Mat1f& deltaY);
	void calculateGradientMagnitudes(const cv::Mat1f& deltaX, const cv::Mat1f& deltaY);
	void calculateGradientDirectionsAndMagnitudes(const cv::Mat1f& deltaX, const cv::Mat1f& deltaY);

	cv::Mat1f gradMagnitudes;
	cv::Mat1f gradOrientations;	// sic direction
};

} // namespace


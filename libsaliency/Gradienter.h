
#pragma once

namespace sal {


/*
\brief Class to compute gradient: magnitude and direction.

Hides implementation: now openCL, original used de novo implementations.

Owns images it computes (magnitude and direction)

Agnostic of channels: computes gradient for each

Valid calling sequence:  compute ... getGradMagnitude
*/
class Gradienter {
public:
	explicit Gradienter();
	virtual ~Gradienter();

	void compute(const cv::Mat& src);

	cv::Mat getGradMagnitudes() const { return gradMagnitudes; }
	cv::Mat getGradOrientations() const { return gradOrientations; }


private:

	void calculateXAndYDerivatives(const cv::Mat& smoothedImg, cv::Mat& deltaX, cv::Mat& deltaY);
	void calculateGradientDirections(const cv::Mat& deltaX, const cv::Mat& deltaY);
	void calculateGradientMagnitudes(const cv::Mat& deltaX, const cv::Mat& deltaY);
	void calculateGradientDirectionsAndMagnitudes(const cv::Mat& deltaX, const cv::Mat& deltaY);

	cv::Mat gradMagnitudes;
	cv::Mat gradOrientations;	// sic direction
};

} // namespace

